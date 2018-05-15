/*! \file
	Description: Disk File Handling and Tokenization

	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_FOLDER
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_PRINT            // print functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TM               // general Transl. Memory functions
#include <eqf.h>                       // General Translation Manager include file
#include <time.h>

#include "EQFB.ID"                // Translation Processor IDs
#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFHLOG.H"            // Translation Processor priv. include file
#include "eqfdde.h"
#include "eqfrpt00.h"            // counting report
#include "eqfdoc00.h"             // private document handler include file
#include "eqfsetup.h"             // our EQF subdirectory structure

#include "OtmProposal.h"
#include "core\pluginmanager\OtmMorph.h"
#include "core\memory\MemoryFactory.h"
#include "vector"


 #define SEGMENTS_PER_ALLOC 32         // # of segments allocated per alloc
 #define ERR_TAGSAREWRONG   -15        // error rc for tagging errors
#define ADDINFO_DIR   "ADDINFO"

#define NUM_TOKENS    100              // space in token table
TOKENENTRY   astTokBuf[ NUM_TOKENS + 1 ];        // number of tokens

// activate to debug EQFBHISTDOCSAVE function
#ifdef _DEBUG
  // #define HISTDOCSAVELOG
  //#define WORDCOUNTLOG
#endif

// activate the following define to show error message EQF0055 and copy document files to temp directory in case of corrupted word count info
// this define will also reset the segment data and add an error text to the segment
//#define  SHOW_CORRUPTEDDATA_MSG


#define GETNUMBER( pszTarget, ulValue ) \
{                                   \
   ulValue = 0L;                     \
   if ( *pszTarget++ )              \
   {                                \
      while ( isdigit(*pszTarget) ) \
      {                             \
         ulValue = (ulValue * 10L) + (*pszTarget++ - '0'); \
      } /* endwhile */              \
   } /* endif */                    \
}

// text to be used for segments with corrupted data
static PSZ_W pszErrorSegText = L"ERROR! SEGMENT DATA CORRUPTED, SEGDATA=";

// structure for tag table extract (tag name + tag ID)
typedef struct _TBTAGENTRY
{
   PSZ   pszTag;                       // ptr to tag name
   SHORT sID;                          // associated tag ID
} TBTAGENTRY, *PTBTAGENTRY;

BOOL   EQFBWCntCheckSeg(PTBDOCUMENT pDoc, PTBSEGMENT pSeg );
PNODE  EQFBCreateTagTree( PTAGTABLE pTagTable );
PNODE  EQFBCreateAttrTree( PTAGTABLE pTagTable );
BOOL   EQFBAddNode ( PNODE *, PNODE, PUSHORT, PUSHORT );
int    EQFBTagCompare( const void *, const void * );
static SHORT CountAttribs( PLOADEDTABLE, PSZ_W, SHORT, PULONG, PULONG, ULONG );/* @KIT0885A */
static USHORT EQFBSumUpWords(PTBSEGMENT, PCRITERIASUM);
static VOID  EQFBWCntClearInfo(PTBDOCUMENT pDoc, PSZ pszFileName, PSZ pszSegmentList );
static SHORT  EQFBCheckAndAddSeg( PTBDOCUMENT, PTBSEGMENT,
                                  PUSHORT, ULONG, PTBSEGMENT,
                                  PTBSEGMENT, LONG );
static BOOL   EQFBCheckForTable( PTBSEGMENT, ULONG );
static SHORT  EQFBAddSegEx( PTBDOCUMENT, PTBSEGMENT, USHORT, LONG );
SHORT EQFBWordCntPerSegUsingProtTable
(
  PVOID        pVoidTable,             // ptr to loaded tag table
  PTOKENENTRY  pTokBuf,                // ptr for token table processing
  PSZ_W        pData,                  // pointer to data
  SHORT        sLanguageId,            // language ID
  PULONG       pulResult,              // result to be counted
  PULONG       pulMarkUp,               // result for markup
  ULONG        ulOemCP
);


USHORT usTablesMax = 0;                // number of tables allocated
#define MAX_TBL_INC       10           // number of tables

#define OUTBUF_SIZE 32000              // size of output buffer
// static BYTE   bOutBuf[OUTBUF_SIZE];    // static output buffer
//static USHORT usOutBufUsed = 0;        // number of bytes used in output buffer
// static PBYTE  pbOut = bOutBuf;         // current output position
//static PBYTE  pbOut = NULL;            // current output position
//static PBYTE  pbOutStart = NULL;       // start of output array

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBFileRead/EQFBFileReadEx
//-----------------------------------------------------------------------------
// Function call:     EQFBFileRead(PSZ,PTBDOCUMENT)
//                    EQFBFileReadEX(PSZ,PTBDOCUMENT,LONG)
//-----------------------------------------------------------------------------
// Description:       Read a document from disk and create a segment table
//                       for it.
//-----------------------------------------------------------------------------
// Parameters:        PSZ          pszFileName - the document's fully qualified
//                                               file name.
//                    PTBDOCUMENT  pDoc        - ptr to active doc structure
//                    LONG         lFlags      - process flags
//                       0L = normal mode of EQFBFileRead
//                       FILEREAD_FPOS pusBPET is used to store the
//                                     position of the segment data
//                                     in the input file
//-----------------------------------------------------------------------------
// Returncode type:   USHORT usRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_OPENFILE   - error opening file
//                    ERR_READFILE   - error reading file
//                    ERR_NOMEMORY   - memory allocation error
//-----------------------------------------------------------------------------
// Prerequesits:      pQFTagTable in document structure must point to a loaded
//                    tag table for QFx tags
//
//-----------------------------------------------------------------------------
// Side effects:      if pDoc->pInBuf is NULL a new input buffer is allocated.
//                    if pDoc->pTokBuf is NULL a new tokenize buffer is alloc.
//                    calls to the following routines:
//                          UtlOpen, UtlQFileInfo, UtlAlloc, UtlRead, UtlClose,
//                          EQFTagTokenize
//-----------------------------------------------------------------------------
// Function flow:     - open the document
//                    - get the document size
//                    - allocate input and token buffer if not done yet
//                    - initialize segment table
//                    - read/tokenize input until complete
//                    - convert tokens to entries in segment table
//                    - terminate the segment table and set segment table ptr
//                    - close input file
//
//-----------------------------------------------------------------------------
USHORT EQFBFileRead
(
   PSZ        pszFileName,             // ptr to fully qualified file name
   PTBDOCUMENT  pDoc                     // ptr to document control structure
)
{
    return( EQFBFileReadExW( pszFileName, pDoc, 0L ) );
} /* end of function EQFBFileRead */

USHORT EQFBFileReadExW
(
   PSZ          pszFileName,           // ptr to fully qualified file name
   PTBDOCUMENT  pDoc,                  // ptr to document control structure
   LONG         lFlags                 // process flags
)
{
   USHORT      usDosRC;                // return code of DosXXX calls
   ULONG       ulBytesToRead;          // # of bytes to read from file
   ULONG       ulBytesRead = 0;            // # of bytes read from file
   USHORT      usOpenAction;               // action performed by DosOpen
   FILESTATUS  FileStatus;                 // status information of file
   HFILE       hDocument = (HFILE)NULL;    // handle of document file
   ULONG       ulBytesInBuffer = 0L;       // # of bytes in input buffer
   ULONG       ulRemaining = 0L;           // # of bytes in document not processed
   PSZ_W       pRest = NULL;               // ptr to start of not-processed bytes
   USHORT      usColPos = 0;               // column pos used by EQFTagTokenize
   PTOKENENTRY pTok;                       // ptr for token table processing
   TBSEGMENT   ActSegment;                 // buffer for active segment
   ULONG       ulLength;                   // number of bytes to be allocated
   ULONG       ulI;                        // loop index
   USHORT      usITemp;                    // loop index temp for check
   PSZ_W       pszSource;                  // ptr to source of copy
   PSZ_W       pszTarget;                  // ptr to target of copy
   ULONG       ulCurSeg = 0;               // number of current segment
   ULONG       ulCurLine = 0;              // number of current line
   SHORT       rc = 0;                     // function return code
   enum _SEGSTATE
   {
      NOSEG,                           // no segment active
      SEGSTARTED,                      // segment start tag processed
      SEGDATA                          // segment data processed
   } SegState;                         // state of active segment
   BOOL    fTokenize = TRUE;           // TRUE = tokenize input file
   BOOL    fFirstTry = TRUE;           // TRUE = first try of tokenization
   BOOL    fMsg = TRUE;                // TRUE = report tagging errors
   USHORT  usMBCode;                   // message box (=UtlError) return code
   CHAR_W  chTest;                     // buffer for character tests
#if defined(TIMEIT)
   LONG lTimeForIO;                    // time spent in I/O operations
   LONG lTimeForTokenize;              // time spent in tokenizer
   LONG lTimeForRest;                  // time spent in rest of procedure
   LONG lLast;                         // last time
   LONG lCurrent;                      // current time
   FILE *hTimeLog;                     // handle of time log file
#endif
   PTAG        pTag;                   // pointer to structure of active tag
   PSZ         pTagNames;              // pointer to start of tagnames
   PBYTE       pByte;                  // helper pointer
   PTAGTABLE   pTagTable;              // pointer to active QF tag table
   USHORT      usValue;                // buffer for numeric values
   PSZ         pError;                 // error file name
   CHAR_W      chErrorW[MAX_LONGPATH];
   PSZ_W       pErrorW = &chErrorW[0]; // error file name (Unicode)
   PSZ_W       pTempRest = NULL;              // temp rest pointer
   ULONG       ulFilePos = 0;                 // file position of first byte in
                                              // input buffer
   BOOL        fWCntFailed = FALSE;           // wordcnt info not invalid so far
   USHORT      usLastTable = STANDARDTABLE;   // returns a value to which table
                                              // the segment has been added
   ULONG       ulNValue = 1L;                 // number in N= statement
   TBSEGMENT   tbDelimSeg;
   TBSEGMENT   tbEmptySeg;
   BOOL        fFirstRead = TRUE;             // TRUE = first read of data
   PLOADEDTABLE pLoadedTable;

   // number and array for corrupted segment list
   #define MAX_CORRUPTED_SEGMENTS 10
   int iCorruptedSegments = 0;
   ULONG aulCorruptedSegments[MAX_CORRUPTED_SEGMENTS + 1];

   SegState = NOSEG;                          // init SegState
   memset( &ActSegment, 0, sizeof(TBSEGMENT) );

   // address QF tag table names
   pLoadedTable = (PLOADEDTABLE) pDoc->pQFTagTable;
   pTagTable = pLoadedTable->pTagTable;
   pByte = (PBYTE) pTagTable;
   pTag = (PTAG) ( pByte + pTagTable->stFixTag.uOffset);
   pTagNames = (PSZ)( pByte + pTagTable->uTagNames);


#if defined(TIMEIT)
   lLast = pGlobInfoSeg->msecs;
   lTimeForRest = 0L;
   lTimeForIO   = 0L;
   lTimeForTokenize = 0L;
#endif

   // set error parameter to document name
   if ( pDoc->szDocLongName[0] != EOS )
   {
     pError = pDoc->szDocLongName;
   }
   else
   {
     pError = UtlGetFnameFromPath( pszFileName );
   } /* endif */
   ASCII2Unicode( pError, pErrorW, pDoc->ulOemCodePage );
   //
   // load the document and create the segment table
   //

#if defined(TIMEIT)
   lCurrent = pGlobInfoSeg->msecs;
   lTimeForRest += lCurrent - lLast;
   lLast = lCurrent;
#endif
   // open the document
   if ( !rc )
   {
      usDosRC = UtlOpen( pszFileName,
                         &hDocument,
                         &usOpenAction, 0L,
                         FILE_NORMAL,
                         FILE_OPEN,
                         OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                         0L,
                         TRUE);
      rc = ( usDosRC ) ? ERR_OPENFILE : 0;
   } /* endif */

   // get the document size
   if ( !rc )
   {
      usDosRC = UtlQFileInfo( hDocument,
                              1,              // info level 1
                              (PBYTE) &FileStatus,
                              sizeof(FileStatus),
                              TRUE );
      rc = ( usDosRC ) ? ERR_READFILE : 0;
      ulRemaining = FileStatus.cbFile;

      if ( ! ulRemaining )
      {
         UtlError( NO_VALID_FORMAT, MB_CANCEL, 1,
                   &pError, EQF_ERROR );
         rc = ERR_READFILE;
      } /* endif */
   } /* endif */

#if defined(TIMEIT)
   lCurrent = pGlobInfoSeg->msecs;
   lTimeForIO += lCurrent - lLast;
   lLast = lCurrent;
#endif

   // allocate input and token buffer if not done yet
   if ( !rc )
   {
      if ( !pDoc->pInBuf )
      {
         // allocate some more bytes than IO_BUFFER_SIZE to allow insertion
         // of LF at last segment (if there is none)
         // as the input buffer is used also for fuzzy tokens the size had to be
         // enlarged to 40KB as this the max size a fuzzy token table can reach
         UtlAlloc((PVOID *) &pDoc->pInBuf, 0L, (LONG)max(IO_BUFFER_SIZE+10,40000), ERROR_STORAGE );
      } /* endif */
      if ( pDoc->pInBuf && !pDoc->pTokBuf )
      {
         UtlAlloc((PVOID *) &pDoc->pTokBuf, 0L, (LONG) TOK_BUFFER_SIZE, ERROR_STORAGE );
      } /* endif */
      rc = ( pDoc->pInBuf && pDoc->pTokBuf ) ? 0 : ERR_NOMEMORY;
   } /* endif */

   // initialize read and tokenize of file
   if ( !rc )
   {
      SegState = NOSEG;                // no segment has started yet
      fTokenize = TRUE;                // try to tokenize the file
      fFirstTry = TRUE;                // first try of tokenization
                                       // fill in null segment at start
      memset( &ActSegment, 0, sizeof(TBSEGMENT) );
      memset( &tbDelimSeg, 0, sizeof(TBSEGMENT) );
      memset( &tbEmptySeg, 0, sizeof(TBSEGMENT) );

      tbDelimSeg.SegFlags.InsertAdd = FALSE;               // delimiter segment in add.table
      /****************************************************************/
      /* EmptySeg is inserted at FILEREAD to ensure that each segnum  */
      /* exists! At FILEWRITE do not write it !                       */
      /****************************************************************/
      tbEmptySeg.SegFlags.InsertAdd = FALSE;
      tbEmptySeg.SegFlags.NoWrite = TRUE;
      tbEmptySeg.qStatus = QF_NOP;

      EQFBAddSegEx(pDoc, &tbDelimSeg, ADDITIONALTABLE, (lFlags & FILEREAD_SINGLETABLE));
      EQFBAddSegEx(pDoc, &ActSegment, STANDARDTABLE, (lFlags & FILEREAD_SINGLETABLE) );

      ulCurSeg ++;
   } /* endif */

   // read/tokenize input until complete
   while ( !rc && (ulRemaining  || ulBytesInBuffer ))
   {
      // fill input buffer
      ulBytesToRead = min( ulRemaining,
                         (ULONG) (IO_BUFFER_SIZE - ulBytesInBuffer - 1));
      if ( ulBytesToRead )
      {
#if defined(TIMEIT)
   lCurrent = pGlobInfoSeg->msecs;
   lTimeForRest += lCurrent - lLast;
   lLast = lCurrent;
#endif

         if ( lFlags & FILEREAD_FPOS )
         {
           UtlChgFilePtr( hDocument, 0L, FILE_CURRENT, &ulFilePos, FALSE );
           ulFilePos -= ulBytesInBuffer;
         } /* endif */

         // if first read check for Unicode segmented file
         if ( fFirstRead )
         {
           BOOL fUnicode = FALSE;

           PSZ pszPrefix = UNICODEFILEPREFIX;
           PSZ pData = (PSZ)(pDoc->pInBuf);
           int iLen = strlen(pszPrefix);

           UtlReadL( hDocument, pData, 8, &ulBytesRead, FALSE );

           if ( memcmp( pData, pszPrefix, iLen ) == 0 )
           {
             // position right behind prefix
             ULONG ulTemp;
             UtlChgFilePtr( hDocument, iLen, FILE_BEGIN, &ulTemp, FALSE );

             // skip prefix ...
             pData += iLen;
             fUnicode = TRUE;
             ulRemaining -= iLen;
           }
           else
           {
             // position back to start of file
             ULONG ulTemp;
             UtlChgFilePtr( hDocument, 0L, FILE_BEGIN, &ulTemp, FALSE );
           } /* endif */

           // check for Unicode if not recognized yet
           if ( !fUnicode )
           {
             // very hardcoded check.... TODO: replace by more appropriate check
             if ( (pData[0] == ':') && (pData[1] == EOS) )
             {
               fUnicode = TRUE;
             } /* endif */
           } /* endif */

           pDoc->fStoreInUnicode = (EQF_BOOL)fUnicode;

           fFirstRead = FALSE;
         } /* endif */

         if ( pDoc->fStoreInUnicode )
         {
           // read Unicode data
           usDosRC = UtlReadL( hDocument,
                              pDoc->pInBuf + ulBytesInBuffer,
                              ulBytesToRead,
                              &ulBytesRead,
                              TRUE );
           ulRemaining -= (LONG) ulBytesRead;
         }
         else
         {
           // read ANSI/ASCII data and convert them into Unicode
           PSZ pConvBuffer = NULL;

           if ( UtlAlloc( (PVOID *)&pConvBuffer, 0L, IO_BUFFER_SIZE , ERROR_STORAGE ) )
           {
             ULONG  ulBytes;
             ULONG ulAnsiBytesToRead;
             ULONG ulAnsiBytesRead;

             // re-compute bytes to read
             ulAnsiBytesToRead =  min( ulRemaining, (ULONG)((IO_BUFFER_SIZE - ulBytesInBuffer - 1)/sizeof(CHAR_W)) );

             // read data into conversion buffer
             usDosRC = UtlReadL( hDocument, pConvBuffer, ulAnsiBytesToRead,
                                &ulAnsiBytesRead, TRUE );
             ulRemaining -= (LONG) ulAnsiBytesRead;
             ulBytesToRead = ulAnsiBytesToRead;
             // convert data to Unicode
             if ( usDosRC == NO_ERROR )
             {
               ulBytes = ASCII2UnicodeBuf( pConvBuffer,
                                           (LPWSTR)(pDoc->pInBuf + ulBytesInBuffer),
                                           ulAnsiBytesRead,
                                           pDoc->ulOemCodePage);
               if (ulBytes && ulAnsiBytesRead &&
                   pConvBuffer[ulAnsiBytesRead-1] &&
                   *(((PSZ_W)(pDoc->pInBuf+ulBytesInBuffer))+ulBytes-1) == EOS )
               {
                  ULONG ulCurrentPos = 0;
                  // in the middle of a DBCS character ... redo read last character
                  ulBytes--;
                  ulBytesToRead --;
                  ulRemaining ++;
                  // undo the last character read...
                  // reposition file pointer
                  UtlChgFilePtr( hDocument, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE);
                  ulCurrentPos--;
                  UtlChgFilePtr( hDocument, ulCurrentPos, FILE_BEGIN,  &ulCurrentPos, FALSE);

               }
               ulBytesRead = ulBytes*sizeof(CHAR_W);
             } /* endif */

             // cleanup
             UtlAlloc( (PVOID *)&pConvBuffer, 0L, 0L, NOMSG );
           }
           else
           {
             usDosRC = ERROR_NOT_ENOUGH_MEMORY;
           } /* endif */

         } /* endif */


#if defined(TIMEIT)
   lCurrent = pGlobInfoSeg->msecs;
   lTimeForIO += lCurrent - lLast;
   lLast = lCurrent;
#endif
         rc = ( usDosRC ) ? ERR_READFILE : 0;
         ulBytesInBuffer += ulBytesRead;
      } /* endif */

      *(pDoc->pInBuf+ulBytesInBuffer)   = EOS; // set end of data indicator
      *(pDoc->pInBuf+ulBytesInBuffer+1) = EOS;

      // tokenize data in input buffer
      if ( !rc )
      {
         if ( fTokenize )
         {
            // at end of file, remove EOF and CRLF from data
            if ( ulRemaining == 0L )
            {
               CHAR_W chEOF[4];
               chEOF[0] = '\r';
               chEOF[1] = '\n';
               chEOF[2] = '\x1A';
               chEOF[3] = 0;
               pszSource = (PSZ_W)(pDoc->pInBuf + ulBytesInBuffer) - 1;
               ulI = 3L;                // remove not more than 3 chars
               while ( ulBytesInBuffer && ulI &&
                       UTF16strchr( chEOF, *pszSource ) )
               {
                  *pszSource-- = EOS;
                  ulI--;
                  ulBytesInBuffer -= sizeof( CHAR_W );
               } /* endwhile */

               /**********************************************************/
               /* if last segment does not end with LF - add one         */
               /* have in mind to get rid of :eqf?.                      */
               /**********************************************************/
               if ( (ulBytesInBuffer > 6 * sizeof(CHAR_W)) && (*(pszSource-6) != LF ))
               {
                 memmove( pszSource-4, pszSource-5, 7 * sizeof(CHAR_W));
                 *(pszSource-5) = LF;
                 ulBytesInBuffer+=sizeof(CHAR_W);
               } /* endif */


            } /* endif */

#if defined(TIMEIT)
   lCurrent = pGlobInfoSeg->msecs;
   lTimeForRest += lCurrent - lLast;
   lLast = lCurrent;
#endif
            pTempRest = pRest;                  // save last rest ptr /* @95A */
            TAFastTokenizeW( (PSZ_W) pDoc->pInBuf, (PLOADEDTABLE) pDoc->pQFTagTable,
                            (ulRemaining == 0L), &pRest, &usColPos,
                            (PTOKENENTRY) pDoc->pTokBuf,
                            TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );

#if defined(TIMEIT)
   lCurrent = pGlobInfoSeg->msecs;
   lTimeForTokenize += lCurrent - lLast;
   lLast = lCurrent;
#endif

            pTok = (PTOKENENTRY) pDoc->pTokBuf;
            if ( fFirstTry )
            {
               // get type of linebreak used in document
               ulI = ulBytesInBuffer/sizeof(CHAR_W);  // used as loop breakout counter
               pszSource = (PSZ_W)pDoc->pInBuf;         // start at begin of buffer
               pszTarget = (PSZ_W)pDoc->szLineBreakW;    // target is linebreak array
               *pszTarget = EOS;                 // clear linebreak array
               while ( ulI )
               {
                  if ( *pszSource == LF )
                  {
                     *pszTarget++ = LF;
                     if ( *++pszSource == CR )
                     {
                        *pszTarget++ = CR;
                     } /* endif */
                     *pszTarget = EOS;
                     ulI = 0;
                  }
                  else if ( *pszSource == CR )
                  {
                     *pszTarget++ = CR;
                     switch ( *++pszSource )
                     {
                       case  LF:
                        *pszTarget++ = LF;
                         break;
                       case  CR:
                         /***********************************************/
                         /* add checking for JAVA specific sequence     */
                         /* for linebreaks: 0D0D0A                      */
                         /***********************************************/
                         *pszTarget++ = CR;
                         if ( *++pszSource == LF )
                         {
                            *pszTarget++ = LF;
                         } /* endif */
                         break;
                       default :
                         break;
                     } /* endswitch */
                     *pszTarget = EOS;
                     ulI = 0;
                  }
                  else
                  {
                     ulI--;
                     pszSource++;
                  } /* endif */
               } /* endwhile */
               if ( pDoc->szLineBreakW[0] == EOS )
               {
                 *pszTarget++ = CR;
                 *pszTarget++ = LF;
                 *pszTarget = EOS;
               } /* endif */


               // check if file contains EQF tags
               if ( pTok->sTokenid <= 0)
               {
                  fFirstTry = FALSE;
                  // wrong or missing tagging, only valid for OTHER_DOC files
                  if ( pDoc->docType != OTHER_DOC )
                  {
                     // file is not tagged or not taggged correctly !!!
                     // create aprobriate message and change call below !!!
                     UtlError( NO_VALID_FORMAT, MB_CANCEL, 1,
                               &pError, EQF_ERROR );
                     rc = ERR_TAGSAREWRONG;
                  }
                  else
                  {
                     fTokenize = FALSE;   // switch to line-by-line processing
                  } /* endif */
               } /* endif */
            } /* endif */

         } /* endif */
      } /* endif */

      fFirstTry = FALSE;

      if ( !rc )
      {
         if ( fTokenize )
         {
            // convert tokens to entries in segment table
            pTok = (PTOKENENTRY) pDoc->pTokBuf;
            usITemp = 0;
            while ( !rc && (pTok->sTokenid != ENDOFLIST) )
            {
               usITemp++;
               switch ( pTok->sTokenid )
               {
                  case QFF_TAG:
                  case QFN_TAG:
                  case QFA_TAG:
                  case QFC_TAG:
                  case QFJ_TAG:
                  case QFS_TAG:
                  case QFX_TAG:
                     // prepare a new segment
                     if ( SegState != NOSEG )
                     {
                        // tagging error !!!
                        // create aprobriate message and change call below !!!
                        if ( fMsg )
                        {
                           usMBCode = UtlError( NO_VALID_FORMAT, MB_CANCEL, 1,
                                                &pError, EQF_ERROR );
                           if ( usMBCode == MBID_YES )
                           {
                              fMsg = FALSE;      // avoid multiple messages
                           }
                           else
                           {
                              rc = ERR_TAGSAREWRONG;
                           } /* endif */
                        } /* endif */
                     }
                     else
                     {
                        memset( &ActSegment, 0, sizeof(ActSegment) );
                        switch ( pTok->sTokenid)
                        {
                           case QFF_TAG: ActSegment.qStatus = QF_TOBE; break;
                           case QFN_TAG: ActSegment.qStatus = QF_NOP; break;
                           case QFX_TAG: ActSegment.qStatus = QF_XLATED; break;
                           case QFA_TAG: ActSegment.qStatus = QF_ATTR; break;
                           case QFC_TAG: ActSegment.qStatus = QF_CURRENT; break;
                           case QFJ_TAG: ActSegment.qStatus = QF_JOINED; break;
                           case QFS_TAG: ActSegment.qStatus = QF_SPLIT; break;
                           // more to follow soon ....
                        } /* endswitch */
                        // Note: change following statement and use actual segment
                        //       number (n=xxx value)
                        ActSegment.ulSegNum = ulCurSeg;
                        SegState = SEGSTARTED;
                     } /* endif */
                     break;
                  case EQFF_TAG:
                  case EQFN_TAG:
                  case EQFA_TAG:
                  case EQFC_TAG:
                  case EQFJ_TAG:
                  case EQFS_TAG:
                  case EQFX_TAG:
                     if ( (SegState != SEGDATA) &&  (SegState != SEGSTARTED) )
                     {
                        // tagging error !!!
                        // create aprobriate message and change call below !!!
                        if ( fMsg )
                        {
                           usMBCode = UtlError( NO_VALID_FORMAT, MB_CANCEL, 1,
                                                &pError, EQF_ERROR );
                           if ( usMBCode == MBID_YES )
                           {
                              fMsg = FALSE;      // avoid multiple messages
                           }
                           else
                           {
                              rc = ERR_TAGSAREWRONG;
                           } /* endif */
                        } /* endif */
                     }
                     else
                     {
                        if ( SegState == SEGSTARTED ) // no data until now
                        {
                           ActSegment.usLength = 1;
                           // add data to current segment
                           UtlAlloc((PVOID *) &ActSegment.pDataW,
                                     0L,
                                     (LONG) MIN_ALLOC,
                                     ERROR_STORAGE );
                           if ( !ActSegment.pDataW )
                           {
                              rc = ERR_NOMEMORY;
                           } /* endif */
                        } /* endif */
                        // add the current segment to the segment table(s)
                        if ( !rc )
                        {
                          /********************************************/
                          /* plausability check                       */
                          /********************************************/
                          if ( !ActSegment.SegFlags.Joined )
                          {
                            BOOL fSegError = EQFBWCntCheckSeg(pDoc, &ActSegment );
                            fWCntFailed |= fSegError;

                            // if check failed reset segment and add segment number
                            // to list of corrupted segments
                            if ( fSegError )
                            {
#ifdef SHOW_CORRUPTEDDATA_MSG
                              PSZ_W pszNewData = NULL;
                              ULONG ulNewLen = (UTF16strlenCHAR( pszErrorSegText ) + 1 + ActSegment.usLength) * sizeof(CHAR_W) ;
                              if ( UtlAlloc((PVOID *) &pszNewData, 0L, ulNewLen, ERROR_STORAGE ) )
                              {
                                wcscpy( pszNewData, pszErrorSegText );
                                wcscat( pszNewData, ActSegment.pDataW );
                                UtlAlloc( (PVOID *)&ActSegment.pDataW, 0L, 0L, NOMSG );
                                ActSegment.pDataW = pszNewData;
                                ActSegment.usLength = (USHORT)(UTF16strlenCHAR( ActSegment.pDataW ) + 1);
                              } /* endif */
                              ActSegment.qStatus = QF_TOBE; // set status to untranslated
#endif
                              ActSegment.usSrcWords = ActSegment.usTgtWords = ActSegment.usModWords = 0;
                              if ( iCorruptedSegments < MAX_CORRUPTED_SEGMENTS )
                              {
                                // add to array
                                aulCorruptedSegments[iCorruptedSegments] = ActSegment.ulSegNum;
                              } /* endif */
                              iCorruptedSegments++;
                            } /* endif */
                          } /* endif */

                          rc = EQFBCheckAndAddSeg( pDoc, &ActSegment,
                                        &usLastTable, ulNValue,
                                        &tbDelimSeg, &tbEmptySeg ,
                                        (lFlags & FILEREAD_SINGLETABLE) );
                        } /* endif */
                        SegState = NOSEG;
                        ulCurSeg++;
                     } /* endif */
                     break;
                  case NONE_TAG:             // it is similar to text
                  case TEXT_TOKEN:
                     if ( (SegState != SEGDATA) && (SegState != SEGSTARTED) )
                     {
                        // tagging error !!!
                        // create aprobriate message and change call below !!!
                        if ( fMsg )
                        {
                           usMBCode = UtlError( NO_VALID_FORMAT, MB_CANCEL, 1,
                                                &pError, EQF_ERROR );
                           if ( usMBCode == MBID_YES )
                           {
                              fMsg = FALSE;      // avoid multiple messages
                           }
                           else
                           {
                              rc = -1;
                           } /* endif */
                        } /* endif */
                     }
                     else
                     {
                        // if file positions are requested ...
                        if ( lFlags & FILEREAD_FPOS )
                        {
                          // if this is the first text block of the segment
                          if ( SegState == SEGSTARTED )
                          {
                            ULONG ulSegPos = ulFilePos +
                                             (ULONG)((PBYTE)pTok->pDataStringW -
                                                     (PBYTE)pDoc->pInBuf) + strlen(UNICODEFILEPREFIX);
                            *((PULONG)&(ActSegment.pusBPET)) = ulSegPos;
                            ActSegment.usOrgLength = pTok->usLength;
                          }
                          else
                          {
                            ActSegment.usOrgLength = ActSegment.usOrgLength + pTok->usLength;
                          } /* endif */
                        } /* endif */

                        // reduce crlf combinations to lf
                        ulI = pTok->usLength;
                        pszSource = pszTarget = pTok->pDataStringW;
                        while ( ulI )
                        {
                           chTest = *pszTarget = *pszSource++;
                           ulI--;
                           if ( chTest == LF )
                           {
                              pszTarget++;
                              ulCurLine++;
                              if ( *pszSource == CR )
                              {
                                 pszSource++;    // skip second linebreak character
                                 if ( ulI ) ulI--;
                              } /* endif */
                           }
                           else if ( chTest == CR)
                           {
                              ulCurLine++;
                              switch ( *pszSource )
                              {
                                case  LF:
                                  pszSource++;   // skip second linebreak character
                                  if ( ulI ) ulI--;
                                  break;
                                case  CR:
                                  /************************************/
                                  /* reduce JAVA 0d0d0a sequences to  */
                                  /* one linefeed.                    */
                                  /************************************/
                                  if ( *(pszSource+1) == LF )
                                  {
                                    pszSource += 2;
                                    if ( ulI > 1 )
                                    {
                                      ulI -= 2;
                                    } /* endif */
                                  } /* endif */
                                  break;
                                default :
                                  break;
                              } /* endswitch */
                              *pszTarget++ = LF;
                           }
                           else
                           {
                              pszTarget++;
                           } /* endif */
                        } /* endwhile */
                        pTok->usLength = (USHORT)((pTok->usLength) - (pszSource - pszTarget));

                        // allocate new data area or enlarge existing one
                        if ( SegState == SEGSTARTED )
                        {
                          // this is the first text block of the segment
                          ulLength = max( pTok->usLength, MIN_ALLOC );
                          UtlAlloc((PVOID *) &ActSegment.pDataW, 0L,
                                    (LONG) (ulLength + 1)*sizeof(CHAR_W), ERROR_STORAGE );
                          ActSegment.usLength = 0;// no data in segment yet
                        }
                        else
                        {
                          // this is the 2nd or more text block of the segment
                          ulLength = max( (pTok->usLength + ActSegment.usLength),
                                          MIN_ALLOC );
                          UtlAlloc( (PVOID *)&ActSegment.pDataW,
                                    (LONG)max((ActSegment.usLength+1)*sizeof(CHAR_W),
                                              MIN_ALLOC),
                                    (LONG)(ulLength + 1)*sizeof(CHAR_W), ERROR_STORAGE );
                        } /* endif */

                        // add segment data to active segment
                        if ( ulLength >= MAX_SEGMENT_SIZE )
                        {
                          // length of segment is too large
                          if ( fMsg )
                          {
                             usMBCode = UtlError( NO_VALID_FORMAT, MB_CANCEL, 1,
                                                  &pError, EQF_ERROR );
                             fMsg = FALSE;      // avoid multiple messages
                          } /* endif */
                          rc = -1;
                          UtlAlloc( (PVOID *)&ActSegment.pDataW, 0L, 0L, NOMSG );
                        }
                        else if ( ActSegment.pDataW )
                        {
                           memcpy( ActSegment.pDataW + ActSegment.usLength,
                                   pTok->pDataStringW,
                                   pTok->usLength * sizeof(CHAR_W) );

                           ActSegment.usLength = ActSegment.usLength + pTok->usLength;
                           ActSegment.pDataW[ActSegment.usLength] = EOS;
                        }
                        else
                        {
                           rc = ERR_NOMEMORY;
                        } /* endif */
                        SegState = SEGDATA;
                     } /* endif */
                     break;
                  case CURRENT_ATTR:
                  case N_ATTR:
                  case MARK_ATTR:
                  case JOIN_ATTR:
                  case STATUS_ATTR:

                  case NOCOUNT_ATTR:

                  case COUNT_ATTR:
                     if ( SegState != SEGSTARTED )
                     {
                        // tagging error !!!
                        if ( fMsg )
                        {
                           usMBCode = UtlError( NO_VALID_FORMAT, MB_CANCEL, 1,
                                                &pError, EQF_ERROR );
                           if ( usMBCode == MBID_YES )
                           {
                              fMsg = FALSE;      // avoid multiple messages
                           }
                           else
                           {
                              rc = ERR_TAGSAREWRONG;
                           } /* endif */
                        } /* endif */
                     }
                     else
                     {
                        switch ( pTok->sTokenid )
                        {
                           case CURRENT_ATTR:
                              ActSegment.SegFlags.Current = TRUE;
                              break;
                           case N_ATTR:
                              pszTarget = pTok->pDataStringW;
                              while ( *pszTarget && (*pszTarget != '=') )
                              {
                                 pszTarget++;
                              } /* endwhile */
                              GETNUMBER( pszTarget, ulNValue );

                              ActSegment.ulSegNum = ulNValue;
                              break;
                           case MARK_ATTR:
                              ActSegment.SegFlags.Marked = TRUE;
                              pDoc->EQFBFlags.MarkedSeg = TRUE;
                              break;

                           case NOCOUNT_ATTR:
                              ActSegment.SegFlags.NoCount = TRUE;
                              break;

                           case JOIN_ATTR:
                              pszTarget = UTF16strchr( pTok->pDataStringW, L'=' );
                              GETNUMBER( pszTarget, usValue );
                              switch ( usValue )
                              {
                                 case 1:
                                    ActSegment.SegFlags.JoinStart = TRUE;
                                    break;
                                 case 2:
                                    ActSegment.SegFlags.Joined    = TRUE;
                                    break;
                              } /* endswitch */
                              break;
                           case STATUS_ATTR:
                              pszTarget = UTF16strchr( pTok->pDataStringW, L'=' );
                              GETNUMBER( pszTarget, usValue );
                              switch ( usValue )
                              {
                                 case 1:
                                    ActSegment.SegFlags.Typed = TRUE;
                                    break;
                                 case 2:
                                    ActSegment.SegFlags.Typed = TRUE;
                                    ActSegment.SegFlags.Copied = TRUE;
                                    break;
                                 case 3:
                                    ActSegment.SegFlags.Copied = TRUE;
                                    break;
                              } /* endswitch */
                              break;
                            case COUNT_ATTR:
                              pszTarget = UTF16strchr( pTok->pDataStringW, L'=' );
                              EQFBGetHexNumberW( pszTarget+1, &usValue );
                              *((PUSHORT)(&ActSegment.CountFlag)) = usValue;

                              EQFBGetHexNumberW( pszTarget+5,
                                                &(ActSegment.usSrcWords) );
                              EQFBGetHexNumberW( pszTarget+9,
                                                &(ActSegment.usTgtWords) );
                              EQFBGetHexNumberW( pszTarget+13,
                                                &(ActSegment.usModWords) );
                              // get any checksum and compare with actual checksum of count data
                              if ( pTok->usLength > 18 )
                              {
                                USHORT usActSum;           // actual checksum of count values
                                USHORT usStoredSum = 0;    // stored check sum in count attribute

                                usActSum = EQFBBuildCountCheckSum( usValue, ActSegment.usSrcWords,
                                                      ActSegment.usTgtWords, ActSegment.usModWords );
                                EQFBGetHexNumberW( pszTarget+17, &usStoredSum );
                                if ( usActSum != usStoredSum )
                                {
                                  if ( fMsg )
                                  {
                                    PSZ pszParms[2];
                                    CHAR szSegNum[10];

                                    pszParms[0] = szSegNum;
                                    pszParms[1] = pError;
                                    sprintf( szSegNum, "%lu", ActSegment.ulSegNum );
                                    UtlError( NO_VALID_FORMAT_WITH_SEG, MB_CANCEL, 2,
                                                          pszParms, EQF_ERROR );
                                    rc = ERR_TAGSAREWRONG;
                                  } /* endif */
                                } /* endif */
                              } /* endif */
                              break;
                        } /* endswitch */
                     } /* endif */
                     break;
                  default:
                     // ignore unknown tokens
                     break;
               } /* endswitch */
               pTok++;
            } /* endwhile */

            // is there still data in input buffer and something read in
            // the last time ??
            // if not we are done - may be a problem with EOF symbol
            if  ( pRest &&             // something still in buffer ???
                  ((pRest != pTempRest) || ulBytesToRead || (usITemp != 0)))   /* @95C */
            {
               // calculate number of not-processed bytes
               ulBytesInBuffer -= ((PBYTE)pRest - pDoc->pInBuf);

               // shift not-processed bytes to start of buffer
               if ( ulBytesInBuffer )
               {
                  memmove( pDoc->pInBuf, (PBYTE)pRest, ulBytesInBuffer );
               } /* endif */

               if ( lFlags & FILEREAD_FPOS )
               {
                 ulFilePos += (ULONG)((PBYTE)pRest - (PBYTE)pDoc->pInBuf);
               } /* endif */
            }
            else
            {
              /********************************************************/
              /* if too many 'NULLs' in the file, probably a EXE      */
              /********************************************************/
              if ( pRest && (ulBytesInBuffer > 5) )
              {
                // file does not seem to be a normal text file ...
                UtlError( NO_VALID_FORMAT, MB_CANCEL, 1,
                          &pError, EQF_ERROR );
                rc = ERR_TAGSAREWRONG;
              }
              else
              {
                ulBytesInBuffer = 0L;  // all data in buffer has been processed
              } /* endif */
            } /* endif */
         }
         else
         {
            // do the segmentation on line-by-line basis
            //    usI is used as loop counter
            //    pszSource points to the character currently tested
            //    pszTarget points to the starting position of the current line
            //    incomplete lines stay in the buffer

            ulI = ulBytesInBuffer / sizeof(CHAR_W);
            pszSource = pszTarget = (PSZ_W)pDoc->pInBuf;
            while ( !rc && ulI )
            {
               if ( (*pszSource == LF) || (*pszSource == CR) )
               {
                  ulCurLine++;
                  chTest = (CHAR) (( *pszSource++ == LF ) ? CR : LF);
                  ulI--;
                  ulLength = pszSource - pszTarget;
                  if ( (CHAR) *pszSource == chTest )
                  {
                     pszSource++; // skip following linebreak character
                     ulI--;
                  } /* endif */
                  memset( &ActSegment, 0, sizeof(ActSegment) );
                  UtlAlloc((PVOID *) &ActSegment.pDataW,
                            0L,
                            (LONG) max( (ulLength + 1)*sizeof(CHAR_W), MIN_ALLOC),
                            ERROR_STORAGE );
                  if ( ActSegment.pDataW )
                  {
                     memcpy( ActSegment.pDataW, pszTarget, ulLength * sizeof(CHAR_W) );
                     ActSegment.qStatus = QF_NOP;
                     *(ActSegment.pDataW + ulLength - 1) = LF; // change linebreak to LF
                     *(ActSegment.pDataW + ulLength) = EOS;    // add eos
                     ActSegment.usLength = (USHORT)ulLength;

                     rc = EQFBCheckAndAddSeg( pDoc, &ActSegment,
                                      &usLastTable, ulNValue,
                                      &tbDelimSeg, &tbEmptySeg,
                                        (lFlags & FILEREAD_SINGLETABLE) );
                     pszTarget = pszSource;      // set new line start
                     ulCurSeg++;
                  }
                  else
                  {
                     rc = ERR_NOMEMORY;
                  } /* endif */
               }
               else
               {
                  pszSource++;
                  ulI--;
               } /* endif */
            } /* endwhile */
            ulBytesInBuffer -= ((PBYTE)pszTarget - pDoc->pInBuf);
            // if nothing remains to read in, process last line
            if ( !ulRemaining && !rc )
            {
                  ulLength = UTF16strlenCHAR( pszTarget ) + 1;
                  memset( &ActSegment, 0, sizeof(ActSegment) );
                  UtlAlloc((PVOID *) &ActSegment.pDataW,
                            0L,
                            (LONG) max( (ulLength + 1)*sizeof(CHAR_W), MIN_ALLOC),
                            ERROR_STORAGE );
                  if ( ActSegment.pDataW )
                  {
                     memcpy( ActSegment.pDataW, pszTarget, ulLength * sizeof(CHAR_W));
                     ActSegment.qStatus = QF_NOP;
                     *(ActSegment.pDataW + ulLength- 1) = LF; // add linebreak
                     *(ActSegment.pDataW + ulLength) = EOS;    // add eos
                     ActSegment.usLength = (USHORT)ulLength;
                     rc = EQFBCheckAndAddSeg( pDoc, &ActSegment,
                                       &usLastTable, ulNValue,
                                       &tbDelimSeg, &tbEmptySeg,
                                        (lFlags & FILEREAD_SINGLETABLE) );
                     ulCurSeg++;
                     ulBytesInBuffer = 0L;         // reset buffer bytes
                  }
                  else
                  {
                     rc = ERR_NOMEMORY;
                  } /* endif */
            } /* endif */

            // shift not-processed bytes to start of buffer
            if ( ulBytesInBuffer )
            {
               memmove( pDoc->pInBuf, pszTarget, ulBytesInBuffer );
            } /* endif */
         } /* endif */
      } /* endif */
    } /* endwhile */

   // terminate the segment table and set segment table pointer
   if ( !rc)
   {
      pDoc->ulMaxLine= ulCurLine;    // save number of lines
   } /* endif */

   // close input file
   if ( hDocument )
   {
      UtlClose( hDocument, FALSE );
   }
   /*******************************************************************/
   /* WCnt data are inconsistent; will be cleared                     */
   /*******************************************************************/
   if (fWCntFailed )
   {
     CHAR szSegmentList[(MAX_CORRUPTED_SEGMENTS*14)+5];
     int i = 0;

     // setup list of corrupted segments
     szSegmentList[0] = EOS;
     while ( (i < MAX_CORRUPTED_SEGMENTS) && (i < iCorruptedSegments) )
     {
       CHAR szSegNumber[12];
       sprintf( szSegNumber, "#%lu", aulCorruptedSegments[i] );
       strcat( szSegmentList, szSegNumber );
       i++;
       if ( i == (iCorruptedSegments - 1) )
       {
         if ( iCorruptedSegments > 2)
         {
          strcat( szSegmentList, "," );
         } /* endif */
         strcat( szSegmentList, " and " );
       }
       else if ( i < iCorruptedSegments)
       {
         strcat( szSegmentList, ", " );
       } /* endif */
     } /*endwhile */

     if ( iCorruptedSegments >= MAX_CORRUPTED_SEGMENTS )
     {
       strcat( szSegmentList, ",++" );
     } /* endif */

     // call clear/reset function
     EQFBWCntClearInfo(pDoc, pszFileName, szSegmentList );
   } /* endif */

#if defined(TIMEIT)
   lCurrent = pGlobInfoSeg->msecs;
   lTimeForRest += lCurrent - lLast ;
   hTimeLog = fopen( "TIME.LOG", "a" );
   if ( hTimeLog )
   {
      fprintf( hTimeLog, "================ TIME LOG  ================\n" );
      fprintf( hTimeLog, "Time for I/O        : %ld [ms]\n", lTimeForIO );
      fprintf( hTimeLog, "Time for Tok        : %ld [ms]\n", lTimeForTokenize );
      fprintf( hTimeLog, "Rest of EQFBFileRead: %ld [ms]\n", lTimeForRest );
      fclose( hTimeLog );
   } /* endif */
#endif

   // load any metadata of file
   if ( !rc && ((pDoc->docType == STARGET_DOC) || (lFlags & FILEREAD_METADATA)) )
   {
     MDLoadMetaData( pDoc );
   } /* endif */


   return rc;
} /* end of EQFBFileReadExW */








//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBCheckAndAddSeg
//-----------------------------------------------------------------------------
// Function call:     EQFBCheckAndAddSeg(PTBDOCUMENT,PTBSEGMENT,
//                                   PUSHORT, USHORT, PTBSEGMENT, PTBSEGMENT
//-----------------------------------------------------------------------------
// Description:       Adds a new segment to the document segment tables
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc      - ptr to active document structure
//                    PTBSEGMENT   pNewSeg   - ptr to new segment
//                    PUSHORT      pusLastTable - to which table last segment
//                                                  has been added
//                    USHORT       usNValue
//                    PTBSEGMENT   pDelimSeg
//                    PTBSEGMENT   pEmptySeg
//                    BYTE         bSingleTable
//-----------------------------------------------------------------------------
// Returncode type:   SHORT sRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_NOMEMORY   - memory allocation error
//-----------------------------------------------------------------------------
// Function flow:     -
//
//-----------------------------------------------------------------------------

SHORT EQFBCheckAndAddSeg
(
   PTBDOCUMENT pDoc,
   PTBSEGMENT  pNewSeg,
   PUSHORT     pusLastTable,
   ULONG       ulNValue,
   PTBSEGMENT  pDelimSeg,
   PTBSEGMENT  pEmptySeg,
   LONG        lSingleTable
)
{
  BOOL        fIsForStandardTable = TRUE;
  SHORT       sRC = 0;                                     // function's return code
  ULONG       ulLength = 0;

  /********************************************************************/
  /* check if current segment should be added to regular table or     */
  /* to additional segmenttable                                       */
  /* segments in additional segmenttable are not handled at all in the*/
  /* standard editor                                                  */
  /********************************************************************/
  if ( !lSingleTable )
  {
    fIsForStandardTable = EQFBCheckForTable(pNewSeg, ulNValue);
  } /* endif */

  if (fIsForStandardTable )
  {
    /******************************************************************/
    /* if preceeding segments has been added to additional segtable,  */
    /* set indicator InsertAdd to TRUE, insert delimiter segment in   */
    /* additional table                                               */
    /******************************************************************/

    if (*pusLastTable == ADDITIONALTABLE )
    {
      pNewSeg->SegFlags.InsertAdd = TRUE;
      sRC = EQFBAddSegEx(pDoc, pDelimSeg,
                         ADDITIONALTABLE,
                         lSingleTable);          //add delimiter segment
    } /* endif */
    /******************************************************************/
    /* if usNValue != # of next free seg in table, insert empty segs  */
    /* to avoid missing segment numbers (XLATE!)                      */
    /******************************************************************/
    if ( !sRC && (ulNValue > pDoc->ulMaxSeg) &&
                 (pNewSeg->SegFlags.InsertAdd == TRUE ))
    {
      /****************************************************************/
      /* move indicator flag into EmptySeg !                          */
      /****************************************************************/
      pEmptySeg->SegFlags.InsertAdd = TRUE;
      pNewSeg->SegFlags.InsertAdd = FALSE;
    } /* endif */
    while ( !sRC && (ulNValue > pDoc->ulMaxSeg ))
    {
      ulLength = max( strlen(EMPTY_STRING) + 1, MIN_ALLOC );
      UtlAlloc((PVOID *) &pEmptySeg->pDataW, 0L,
                ulLength * sizeof(CHAR_W), ERROR_STORAGE );
      if ( pEmptySeg->pDataW )
      {
        UTF16strcpy(pEmptySeg->pDataW, EMPTY_STRINGW);
        pEmptySeg->usLength = (USHORT)UTF16strlenCHAR(pEmptySeg->pDataW);
        pEmptySeg->SegFlags.NoWrite = TRUE;
        sRC = EQFBAddSegEx( pDoc, pEmptySeg,
                            STANDARDTABLE, lSingleTable);
      }
      else
      {
        sRC = ERR_NOMEMORY;
      } /* endif */
      pEmptySeg->SegFlags.InsertAdd = FALSE;
    } /* endwhile */

    if ( !sRC )
    {
      pNewSeg->SegFlags.NoWrite = FALSE;
      sRC = EQFBAddSegEx( pDoc, pNewSeg,
                          STANDARDTABLE, lSingleTable );
    } /* endif */
    *pusLastTable = STANDARDTABLE;
  }
  else
  {
    pNewSeg->SegFlags.InsertAdd = TRUE;       // seg must be added at filewrite
    pNewSeg->SegFlags.NoWrite = FALSE;
    sRC = EQFBAddSegEx (pDoc, pNewSeg,
                        ADDITIONALTABLE, lSingleTable);
    *pusLastTable = ADDITIONALTABLE;
  } /* endif */

  return (sRC);
} /* endof EQFBCheckAndAddSeg */


//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBCheckForTable
//-----------------------------------------------------------------------------
// Function call:     EQFBCheckForTable (PTBDOCUMENT,PTBSEGMENT)
//-----------------------------------------------------------------------------
// Description:       Adds a new segment to the document segment tables
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc      - ptr to active document structure
//                    PTBSEGMENT   pNewSeg   - ptr to new segment
//-----------------------------------------------------------------------------
// Returncode type:   SHORT sRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_NOMEMORY   - memory allocation error
//-----------------------------------------------------------------------------
// Function flow:     if NOCOUNT attribute is set, or "N=0",
//                       add segment not in standard table
//-----------------------------------------------------------------------------
static
BOOL  EQFBCheckForTable
(
   PTBSEGMENT  pNewSeg,
   ULONG       ulNValue
)
{
  BOOL        fIsForStandardTable = TRUE;
  int     usI = 0;

  if (pNewSeg->SegFlags.NoCount || (ulNValue == 0) )
  {
    fIsForStandardTable = FALSE;
  } /* endif */

  	  // for test only

	  if (pNewSeg->pDataW)
	  {
  	    usI = UTF16strlenCHAR(pNewSeg->pDataW);
	  }
	  else if (pNewSeg->pData)
	  {
		  usI = strlen(pNewSeg->pData);
	  }
	  else
	  {
		  usI = 0;
	  }
	  if (usI == 0)
	  {
		  fIsForStandardTable = FALSE;
	  }

  return (fIsForStandardTable);

} /* endof EQFBCheckForTable  */

SHORT EQFBAddSeg( PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg )
{
  SHORT sRC = 0;
  TBSEGMENT tbSeg;

  memcpy( &tbSeg, pNewSeg, sizeof(TBSEGMENT));

  tbSeg.pDataW = NULL;      // we have to allocate the pDataW in sizeof of provided string

  if ( tbSeg.pData )
  {
    if ( UtlAlloc( (PVOID *) &tbSeg.pDataW, 0L, (strlen( pNewSeg->pData )+1)*sizeof(CHAR_W), NOMSG ) )
  {
      ASCII2Unicode( tbSeg.pData, tbSeg.pDataW, pDoc->ulOemCodePage );
      //recalc usLength nec. because # of char'w is not equal to # of ASCII's always!!
      // tbSeg.usLength MUST be # of char'w's
      tbSeg.usLength = (USHORT)UTF16strlenCHAR( tbSeg.pDataW );
      sRC = EQFBAddSegW( pDoc, &tbSeg );
  }
    else
  {
      sRC = ERR_NOMEMORY;
  }
  }
  else
  {
     sRC = EQFBAddSegW( pDoc, &tbSeg );
  }
  return( sRC );

} /* endof EQFBAddSeg */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBAddSeg
//-----------------------------------------------------------------------------
// Function call:     EQFBAddSeg(PTBDOCUMENT,PTBSEGMENT)
//-----------------------------------------------------------------------------
// Description:       Adds a new segment to the document segment tables
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc      - ptr to active document structure
//                    PTBSEGMENT   pNewSeg   - ptr to new segment
//-----------------------------------------------------------------------------
// Returncode type:   SHORT sRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_NOMEMORY   - memory allocation error
//-----------------------------------------------------------------------------
// Function flow:     - compute max segments per table if not done yet
//                    - compute max segments per table if not done yet
//                    - create segment table array if none exists
//                    - check if max number of segments is exceeded
//                    - get active (=last) segment table
//                    - add new table if current table is full
//                    - enlarge current segment table if required
//                    - add the new segment to the current segment table
//
//-----------------------------------------------------------------------------

SHORT EQFBAddSegW( PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg )
{
   SHORT     sRC = 0;

   pNewSeg->SegFlags.NoWrite = FALSE;

   sRC = EQFBAddSegEx(pDoc, pNewSeg, STANDARDTABLE, 0L);

   return( sRC );

} /* endof EQFBAddSeg */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBAddSegEx
//-----------------------------------------------------------------------------
// Function call:     EQFBAddSegEx        (PTBDOCUMENT, PTBSEGMENT)
//-----------------------------------------------------------------------------
// Description:       Adds a new segment to the additional seg   tables
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc      - ptr to active document structure
//                    PTBSEGMENT   pNewSeg   - ptr to new segment
//-----------------------------------------------------------------------------
// Returncode type:   SHORT sRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_NOMEMORY   - memory allocation error
//-----------------------------------------------------------------------------
// Function flow:     - compute max segments per table if not done yet
//                    - compute max segments per table if not done yet
//                    - create segment table array if none exists
//                    - check if max number of segments is exceeded
//                    - get active (=last) segment table
//                    - add new table if current table is full
//                    - enlarge current segment table if required
//                    - add the new segment to the current segment table
//
//-----------------------------------------------------------------------------

SHORT EQFBAddSegEx
(
  PTBDOCUMENT   pDoc,
  PTBSEGMENT    pNewSeg,
  USHORT        usTableType,
  LONG          lSingleTable

)
{
   ULONG           ulOldSize;          // old size of allocated memory
   ULONG           ulNewSize;          // new size of allocated memory
   PTBSEGMENTTABLE pActSegTable = NULL;          // ptr to active segment table
   PTBSEGMENT      pSeg;               // ptr to active segment
   SHORT           sRC = 0;            // function's return code
   BOOL            fOK;                // ok flag set by UtlAlloc
   ULONG           ulSegsPerTable;
   PTBSEGMENTTABLE pSegTables;
   ULONG           ulSegTables;

   if (usTableType == STANDARDTABLE )
   {
     ulSegsPerTable = pDoc->ulSegsPerTable;
     pSegTables = pDoc->pSegTables;
     ulSegTables = pDoc->ulSegTables;
   }
   else
   {
     ulSegsPerTable = pDoc->ulAddSegsPerTable;
     pSegTables = pDoc->pAddSegTables;
     ulSegTables = pDoc->ulAddSegTables;
   } /* endif */

   // compute max segments per table if not done yet
   if ( !ulSegsPerTable )
   {
      ulSegsPerTable = (0xFF00 / sizeof(TBSEGMENT)) - 1;
   } /* endif */

   // create segment table array if none exists
   if ( !pSegTables )
   {
      fOK = UtlAlloc((PVOID *) &pSegTables, 0L,
                      (LONG) max( sizeof(TBSEGMENTTABLE), MIN_ALLOC ),
                      ERROR_STORAGE );
      if ( fOK )
      {
         ulSegTables = 1;
      }
      else
      {
         sRC = ERR_NOMEMORY;
      } /* endif */
   } /* endif */

   // check if max number of segments is exceeded
   /*******************************************************************/
   /* not necessary any more - we support more than 64k segments      */
   /*******************************************************************/
// if ( !sRC && (pDoc->usMaxSeg == 0xFFFF) )
// {
//    PSZ pTemp = pDoc->szDocName;
//    UtlError( ERROR_FILE_TOOLARGE, MB_CANCEL, 1, &pTemp, EQF_ERROR );
//    sRC = ERR_NOMEMORY;
// } /* endif */

   // get active (=last) segment table
   if ( !sRC )
   {
      pActSegTable = pSegTables + (ulSegTables - 1);
   } /* endif */

   // add new table if current table is full
   if ( !sRC )
   {
      if ( pActSegTable->ulSegments >= ulSegsPerTable )
      {
         ulOldSize = max( (sizeof( TBSEGMENTTABLE ) * ulSegTables),
                          MIN_ALLOC );
         ulNewSize = max( (sizeof( TBSEGMENTTABLE ) * (ulSegTables + 1)),
                          MIN_ALLOC );
         fOK = UtlAlloc((PVOID *) &pSegTables, ulOldSize, ulNewSize,
                         ERROR_STORAGE );
         if ( fOK )
         {
            ulSegTables++;
            pActSegTable = pSegTables + (ulSegTables - 1);
         }
         else
         {
            sRC = ERR_NOMEMORY;
         } /* endif */
      } /* endif */
   } /* endif */

   // enlarge current segment table if required
   if ( !sRC )
   {
      if ( pActSegTable->ulSegments >= pActSegTable->ulAllocSegs )
      {
         ulOldSize = sizeof(TBSEGMENT) * pActSegTable->ulAllocSegs;
         ulNewSize = sizeof(TBSEGMENT) * min( ulSegsPerTable,
                        (pActSegTable->ulAllocSegs + SEGMENTS_PER_ALLOC) );
         fOK = UtlAlloc((PVOID *) &pActSegTable->pSegments,
                        ulOldSize, ulNewSize, ERROR_STORAGE );
         if ( fOK )
         {
            pActSegTable->ulAllocSegs = min( ulSegsPerTable,
                                (pActSegTable->ulAllocSegs + SEGMENTS_PER_ALLOC) );
         }
         else
         {
            sRC = ERR_NOMEMORY;
         } /* endif */
      } /* endif */
   } /* endif */

   // finally add the new segment to the current segment table
   if ( !sRC )
   {
      pSeg = pActSegTable->pSegments + pActSegTable->ulSegments;

      if ((pNewSeg->qStatus != QF_TRNOTE_L1_1)
          && (pNewSeg->qStatus != QF_TRNOTE_L1_2)
          && (pNewSeg->qStatus != QF_TRNOTE_L2) )
      {
        /**************************************************************/
        /* as soon as pusBPET is calculated, pNewSeg can be filled    */
        /* usShrinkLen is the only field of of PTBSEGMENT which is not*/
        /* saved during a FileWrite ...                               */
        /**************************************************************/
        pNewSeg->ulShrinkLen = SHRINKLEN_NOTYETCALC;
      } /* endif */

       pNewSeg->SegFlags.Spellchecked = FALSE;

      /****************************************************************/
      /* force that ulSegNum is increasing one by one                 */
      /* only if not called from taph2(=analysis with autosubst)      */
      /****************************************************************/
      if ((usTableType == STANDARDTABLE) && !lSingleTable)
      {
        pNewSeg->ulSegNum = pDoc->ulMaxSeg;
      } /* endif */

      memcpy( pSeg, pNewSeg, sizeof(TBSEGMENT) );
      pActSegTable->ulSegments++;

      if (usTableType == STANDARDTABLE )
      {
        pDoc->ulMaxSeg++;
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* check if the specified length of the item is too long  KAT0028  */
   /*******************************************************************/
   if ( !sRC )
   {
     if ( pNewSeg->usLength >= MAX_SEGMENT_SIZE )
     {
        PSZ pTemp = pDoc->szDocName;
        if (pDoc->szDocLongName[0])
        {
            pTemp = pDoc->szDocLongName;
        }
        UtlError( NO_VALID_FORMAT, MB_CANCEL, 1, &pTemp, EQF_ERROR );
        sRC = ERR_READFILE;
     } /* endif */
   } /* endif */

   if (usTableType == STANDARDTABLE )
   {
     pDoc->ulSegsPerTable = ulSegsPerTable ;
     pDoc->pSegTables = pSegTables ;
     pDoc->ulSegTables = ulSegTables;
   }
   else
   {
     pDoc->ulAddSegsPerTable = ulSegsPerTable ;
     pDoc->pAddSegTables = pSegTables;
     pDoc->ulAddSegTables = ulSegTables;
   } /* endif */

   return( sRC );

} /* endof EQFBAddSegEx */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBWCntCheckSeg
//-----------------------------------------------------------------------------
// Function call:     EQFBWCntCheckSeg(PTBDOCUMENT,PTBSEGMENT)
//-----------------------------------------------------------------------------
// Description:       checks whether wordcnt info is invalid
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc      - ptr to active document structure
//                    PTBSEGMENT   pSeg      - ptr to new segment
//-----------------------------------------------------------------------------
// Returncode type:   SHORT sRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_NOMEMORY   - memory allocation error
//-----------------------------------------------------------------------------
// Function flow:     -
//-----------------------------------------------------------------------------

BOOL  EQFBWCntCheckSeg( PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg )
{
   ULONG          ulLen;              // length of segment
   BOOL           fFailed = FALSE;    // ok flag set by UtlAlloc

   if ( (pDoc->docType == STARGET_DOC) && pNewSeg )
   {
      if (pNewSeg->pDataW )
      {
        // fast and first plausability check
        if ((pNewSeg->usSrcWords > MAX_SEGMENT_SIZE) ||
            (pNewSeg->usModWords > MAX_SEGMENT_SIZE)  )
        {
          fFailed = TRUE;

        } /* endif */
        // GQ: use number of bytes for comparism, POE sometimes reports
        //     a word for each byte in the segment ...
        ulLen = UTF16strlenCHAR( pNewSeg->pDataW) * 2;
        if ( (pNewSeg->usTgtWords > ulLen) ||
            (pNewSeg->usModWords > pNewSeg->usSrcWords + 20 ) )
        {
          fFailed = TRUE;
        } /* endif */
      }
      else  if (pNewSeg->pData )
      {
        // fast and first plausability check
         if ((pNewSeg->usSrcWords > MAX_SEGMENT_SIZE) ||
             (pNewSeg->usModWords > MAX_SEGMENT_SIZE)  )
         {
           fFailed = TRUE;
         } /* endif */
         ulLen = strlen( pNewSeg->pData);
         if ( (pNewSeg->usTgtWords > ulLen) ||
             (pNewSeg->usModWords > pNewSeg->usSrcWords + 20 ) )
         {
           fFailed = TRUE;
         } /* endif */
      } /* endif */
   } /* endif */

   return( fFailed);

} /* endof EQFBWCntCheckSegSeg */



/**********************************************************************/
/* data area for filewrite, nec if filewrite is running in the thread */
/**********************************************************************/

typedef struct _FILEWRITEDATA
{
  ULONG     ulIndexStandard;                   // usI: standard segnumber
  ULONG     ulIndexAdditional;                 // usJ: seg# in additionaltable
  CHAR_W    szSegmentBufferW[2*MAX_SEGMENT_SIZE]; // buffer for segment data
  CHAR_W    szOutBufferW[5*MAX_SEGMENT_SIZE];  // output buffer 
  CHAR      szConvBuffer[5*MAX_SEGMENT_SIZE];  // buffer for the conversion to ASCCI/ANSI
  PTAG      pTag;                              // struct of active tag
  PSZ       pTagNames;                         // start of tagnames
  CHAR_W    szMarkAttr[ATTRIBUTE_SIZE];        // ptr to 'MARK' attribute
  CHAR_W    szCurrentAttr[ATTRIBUTE_SIZE];     // ptr to 'CURRENT' attribute
  CHAR_W    szNAttr[ATTRIBUTE_SIZE];           // for preprocessed N= attr
  CHAR_W    szJoinAttr[ATTRIBUTE_SIZE];        // for preprocessed JOIN= attr
  CHAR_W    szStatusAttr[ATTRIBUTE_SIZE];      // for preprocessed S= attr
  CHAR_W    szCountAttr[ATTRIBUTE_SIZE];       // for preprocessed X= attr
  CHAR_W    szNoCountAttr[ATTRIBUTE_SIZE];     // name of "NOCOUNT" attribute
  ULONG     ulLastTable;
  PUCHAR    pConvTable;
  CHAR      szTempName[MAX_LONGFILESPEC];      // name of temporary file
  CHAR      szOrgName[MAX_LONGFILESPEC];       // original file name
  HFILE     hOutFile;                          // output file handle
  EQF_BOOL  fAutoSave;                         // TRUE if struct of autosave
} FILEWRITEDATA, * PFILEWRITEDATA;


//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBFileWrite
//-----------------------------------------------------------------------------
// Function call:     EQFBFileWrite(PSZ,PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       use the segment table information and
//                    write the document to disk
//-----------------------------------------------------------------------------
// Parameters:        PSZ          pszFileName  the doc's fully qualified
//                                               file name
//                    PTBDOCUMENT  pDoc         ptr to active doc structure
//
//-----------------------------------------------------------------------------
// Returncode type:   USHORT usRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_OPENFILE   - error opening file
//                    ERR_WRITEFILE  - error writing file
//                    ERR_NOMEMORY   - memory allocation error
//                    WARN_NOFILE    - filename not found
//-----------------------------------------------------------------------------
// Prerequesits:      pQFTagTable in document structure must point to a
//                       loaded tag table for QFx tags
//-----------------------------------------------------------------------------
// Side effects:      calls to the following routines:
//                          UtlOpen,  UtlAlloc, UtlClose, UtlWrite
//-----------------------------------------------------------------------------
// Function flow:     - open a temp document
//                    - get the EQF segment table information
//                    - scan through the segment table and generate
//                         the QFx and EQFx tags
//                      and the text and write them to disk
//                    - if okay close temp document, erase original one
//                        and rename temp one
//-----------------------------------------------------------------------------

USHORT EQFBFileWrite
(
   PSZ        pszFileName,             // ptr to fully qualified file name
   PTBDOCUMENT  pDoc                     // ptr to document control structure
)
{
  return( EQFBFileWriteEx( pszFileName, pDoc, 0, 0 ) );
} /* end of function EQFBFileWrite */

USHORT EQFBFileWriteEx
(
   PSZ        pszFileName,             // ptr to fully qualified file name
   PTBDOCUMENT  pDoc,                  // ptr to document control structure
   SHORT        sLogTaskID,            // log task ID or 0 for no log record
   USHORT       usCPConversion         // code page conversion
)
{
   SHORT           sRC = 0;
   PFILEWRITEDATA  pFileWriteData = NULL;
   ULONG           ulCP = 0L;

   pDoc->fSaveRunning = TRUE;

   /*******************************************************************/
   /* fill pDoc->pvFileWriteData with attribute strings, alloc all nec*/
   /* load conversion table                                           */
   /*******************************************************************/
   sRC = EQFBPrepareFileWrite( pDoc, (PVOID *)&pFileWriteData, pszFileName, sLogTaskID, usCPConversion, FALSE );
   
   if ( usCPConversion == SGMLFORMAT_ASCII)
   {
     ulCP = pDoc->ulOemCodePage;
   }
   else if ( usCPConversion == SGMLFORMAT_ANSI)
   {
     ulCP = pDoc->ulOemCodePage;
   }

   // scan through the segment table and generate the QFx and EQFx tags and the text and write them to disk
   if ( !sRC )
   {
     BOOL fDone = FALSE;
     do
     {
        sRC = EQFBWriteNextSegment( pDoc, (PVOID)pFileWriteData, usCPConversion, ulCP, &fDone );
     } while ( !sRC && !fDone ); /* enddo */
   } /* endif */

   sRC = (SHORT)EQFBTerminateFileWrite( pDoc, (PVOID)pFileWriteData, (USHORT)sRC );

   /*******************************************************************/
   /* reset changed flags to 0 (in case of no error)                  */
   /*******************************************************************/
   if ( !sRC )
   {
     pDoc->flags.changed = FALSE;
   } /* endif */

   pDoc->fSaveRunning = FALSE;

   return( (USHORT)sRC );
} /* end of EQFBFileWrite */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBPrepareFileWrite
//-----------------------------------------------------------------------------
// Function call:     EQFBPrepareFileWrite(pDoc, pszFileName,
//-----------------------------------------------------------------------------
// Description:       prepare all strings nec to write a segm. file
//-----------------------------------------------------------------------------
// Parameters:        HFILE        hFile       - handle of output file
//                    PBYTE        pbData      - data to write to file
//                    USHORT       usLength    - length of data in bytes
//-----------------------------------------------------------------------------
// Returncode type:   SHORT sRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_WRITEFILE  - write to file failed
//-----------------------------------------------------------------------------
// Function flow:     - fill up buffer
//                    - write buffer to file if buffer is full
//                    - store any remaining data in buffer
//-----------------------------------------------------------------------------
USHORT EQFBPrepareFileWrite
(
  PTBDOCUMENT        pDoc,
  PVOID              *ppvFileWriteData,
  PSZ                pszFileName,
  SHORT              sLogTaskID,
  USHORT             usCPConversion,
  EQF_BOOL           fAutoSave
)
{
  USHORT            usRC = 0 ;
  PFILEWRITEDATA   pFileWriteData = NULL;
  PBYTE            pByte;                  // helper pointer
  PTAGTABLE        pTagTable;                 // pointer to active QF tag table
  USHORT           usI;
  BOOL             fExist;

  UtlAlloc((PVOID *)&pFileWriteData, 0L, (LONG)sizeof(FILEWRITEDATA), ERROR_STORAGE );
  usRC  =  pFileWriteData ? 0 : ERR_NOMEMORY;
  if (!usRC )
  {
    *ppvFileWriteData = (PVOID)pFileWriteData;
    strcpy( pFileWriteData->szOrgName, pszFileName );
    pFileWriteData->ulLastTable = STANDARDTABLE;
    pFileWriteData->pConvTable = NULL;
    pFileWriteData->fAutoSave = fAutoSave;       // true if in thread
  } /* endif */

  if (!usRC )
  {
    if ( sLogTaskID != 0 ) usRC = EQFBHistDocSave( pszFileName, pDoc, sLogTaskID );
  } /* endif */

  if ( !usRC )
  {
    pTagTable = ((PLOADEDTABLE)pDoc->pQFTagTable)->pTagTable;
    pByte = (PBYTE) (PVOID)pTagTable;
    pFileWriteData->pTag = (PTAG) ( pByte + pTagTable->stFixTag.uOffset);
    pFileWriteData->pTagNames = (PSZ)( pByte +  pTagTable-> uTagNames);
    EQFBFillWriteAttrW( pDoc->pQFTagTable, pFileWriteData->szMarkAttr, pFileWriteData->szNoCountAttr, pFileWriteData->szCurrentAttr,
                        pFileWriteData->szJoinAttr, pFileWriteData->szNAttr, pFileWriteData->szStatusAttr, pFileWriteData->szCountAttr );

  } /* endif */
  if (!usRC )
  {
    pFileWriteData->ulIndexStandard = 1;     // skip first dummy segment
    pFileWriteData->ulIndexAdditional = 1;   // index for additional segtable
  } /* endif */
  if (!usRC )
  {
    // create a temporary file name
    usI = 1;
    strcpy( pFileWriteData->szTempName, pszFileName );
    do
    {
      PSZ pTemp;
      pTemp = UtlGetFnameFromPath( pFileWriteData->szTempName );
      if ( !pTemp )
      {
        pTemp = pFileWriteData->szTempName;
      } /* endif */
      sprintf( pTemp, "TB%6.6d.$$$", usI );
      fExist = EQFBFileExists( pFileWriteData->szTempName );
      usI++;
    } while ( usI && fExist ); /* enddo */

    /*******************************************************************/
    /* check if temp file creatable....                                */
    /*******************************************************************/
    usRC = (fExist) ? ERR_OPENFILE : 0;                            /* @KIT1272A */
  } /* endif */

  // open a temp document
  if ( !usRC )
  {
    USHORT usOpenAction = 0;
    usRC = UtlOpen( pFileWriteData->szTempName, &(pFileWriteData->hOutFile), &usOpenAction, 0L,
                    FILE_NORMAL, FILE_OPEN | FILE_CREATE, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                    0L, TRUE );
    if ( !usRC && ((usCPConversion == SGMLFORMAT_UNICODE) || (usCPConversion == 0)))
    {
      ULONG  ulWritten = 0;                  // number of bytes written
      usRC = UtlWriteL( pFileWriteData->hOutFile, UNICODEFILEPREFIX, (SHORT)strlen(UNICODEFILEPREFIX), &ulWritten, TRUE );
    } /* endif */

  } /* endif */

  if (pDoc->szLineBreakW[0] == EOS)
  {
    if (pDoc->szLineBreak[0] != EOS)
    {
      ASCII2Unicode( pDoc->szLineBreak, pDoc->szLineBreakW, pDoc->ulOemCodePage );
    }
    else
    {
      UTF16strcpy(pDoc->szLineBreakW , L"\r\n" );
    } /* endif */
  } /* endif */

  return (usRC);
} /* end of EQFBPrepareFileWrite */

USHORT EQFBTerminateFileWrite( PTBDOCUMENT pDoc, PVOID pvFileWriteData, USHORT usRCIn )
{
  USHORT usRC = usRCIn;

  PFILEWRITEDATA pFileWriteData = (PFILEWRITEDATA)pvFileWriteData;

  // if okay close temp document, erase original one and rename temp one
  if ( !usRC )
  {
    usRC = UtlClose( pFileWriteData->hOutFile, TRUE );
  } /* endif */

  if ( !usRC )
  {
     UtlDelete( pFileWriteData->szOrgName, 0L, FALSE );

     if (UtlMove( pFileWriteData->szTempName, pFileWriteData->szOrgName, 0L, TRUE ))
     {
       usRC = (USHORT)ERR_WRITEFILE;
     } /* endif */
   }
   else 
   {
      UtlDelete( pFileWriteData->szTempName, 0L, FALSE );
   } /* endif */

   // reset changed flags to 0 (in case of no error)                  
   if ( !usRC ) pDoc->flags.changed = FALSE;

   // free all data area
   if ( pFileWriteData ) UtlAlloc( (PVOID *)&pFileWriteData, 0L, 0L, NOMSG );

   // write any metadata of file
   if ( !usRC && (pDoc->docType == STARGET_DOC) )
   {
     MDWriteMetaData( pDoc );
   } /* endif */

  return( usRC );
}

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBFillWriteAttr
//-----------------------------------------------------------------------------
// Function call:     EQFBFillWriteAttr
//-----------------------------------------------------------------------------
// Description:       fill the write attribute strings
//-----------------------------------------------------------------------------
// Parameters:        PVOID pQFTagTable,
//                    PSZ   pszMarkAttr,
//                    PSZ   pszNoCountAttr,
//                    PSZ   pszCurrentAttr,
//                    PSZ   pszJoinAttr,
//                    PSZ   pszNAttr,
//                    PSZ   pszStatusAttr,
//                    PSZ   pszCountAttr
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Function flow:     get attribute strings from QFTAGS.IBL
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBFillWriteAttr
//-----------------------------------------------------------------------------
// Function call:     EQFBFillWriteAttr
//-----------------------------------------------------------------------------
// Description:       fill the write attribute strings
//-----------------------------------------------------------------------------
// Parameters:        PVOID pQFTagTable,
//                    PSZ   pszMarkAttr,
//                    PSZ   pszNoCountAttr,
//                    PSZ   pszCurrentAttr,
//                    PSZ   pszJoinAttr,
//                    PSZ   pszNAttr,
//                    PSZ   pszStatusAttr,
//                    PSZ   pszCountAttr
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Function flow:     get attribute strings from QFTAGS.IBL
//-----------------------------------------------------------------------------
VOID   EQFBFillWriteAttr
(
  PVOID pQFTagTable,
  PSZ pszMarkAttr,
  PSZ pszNoCountAttr,
  PSZ pszCurrentAttr,
  PSZ pszJoinAttr,
  PSZ pszNAttr,
  PSZ pszStatusAttr,
  PSZ pszCountAttr
)
{
     PBYTE            pByte;                  // helper pointer
     PTAGTABLE        pTagTable;                 // pointer to active QF tag table
     PATTRIBUTE       pAttr;                     // start of attributes in tagtable
     PSZ              pszSource;
     PSZ              pszStart;
     PTAG             pTag;
     PSZ              pTagNames;

     pTagTable = ((PLOADEDTABLE)pQFTagTable)->pTagTable;
     pByte = (PBYTE) (PVOID)pTagTable;
     pTag = (PTAG) ( pByte + pTagTable->stFixTag.uOffset);
     pTagNames = (PSZ)( pByte +  pTagTable-> uTagNames);

     // get / preprocess attribute names
     pAttr = (PATTRIBUTE) ( pByte + pTagTable->stAttribute.uOffset);
     strcpy(pszMarkAttr,
            pTagNames + pAttr[MARK_ATTR - pTagTable->uNumTags].uStringOffs);

     strcpy(pszNoCountAttr,
            pTagNames + pAttr[NOCOUNT_ATTR - pTagTable->uNumTags].uStringOffs );

     strcpy(pszCurrentAttr,
            pTagNames + pAttr[CURRENT_ATTR - pTagTable->uNumTags].uStringOffs);
     pszSource = pTagNames + pAttr[JOIN_ATTR - pTagTable->uNumTags].uStringOffs;
     pszStart  = pszJoinAttr;
     while ( (*pszStart = *pszSource++) != NULC )
     {
        if ( *pszStart == CHAR_MULT_SUBST )
        {
           *pszStart++ = '%';
           *pszStart++ = 'd';
        }
        else
        {
           pszStart++;
        } /* endif */
     } /* endwhile */
     pszSource = pTagNames + pAttr[N_ATTR - pTagTable->uNumTags].uStringOffs;
     pszStart = pszNAttr;
     while ( (*pszStart = *pszSource++) != NULC )
     {
        if ( *pszStart == CHAR_MULT_SUBST )
        {
           *pszStart++ = '%';
           *pszStart++ = 'l';
           *pszStart++ = 'u';
        }
        else
        {
           pszStart++;
        } /* endif */
     } /* endwhile */

     pszSource = (pTagNames) +
                  pAttr[STATUS_ATTR - pTagTable->uNumTags].uStringOffs;
     pszStart = pszStatusAttr;
     while ( (*pszStart = *pszSource++) != NULC )
     {
        if ( *pszStart == CHAR_MULT_SUBST )
        {
           *pszStart++ = '%';
           *pszStart++ = 'd';
        }
        else
        {
           pszStart++;
        } /* endif */
     } /* endwhile */

     // preprocess count attribute
     pszSource = pTagNames + pAttr[COUNT_ATTR - pTagTable->uNumTags].uStringOffs;
     pszStart = pszCountAttr;
     while ( (*pszStart = *pszSource++) != NULC )
     {
        if ( *pszStart == CHAR_MULT_SUBST )
        {
           strcpy( pszStart, "%4.4X%4.4X%4.4X%4.4X%4.4X" );
           pszStart += strlen( pszStart );
        }
        else
        {
           pszStart++;
        } /* endif */
     } /* endwhile */

  } /* end of EQFBFillWriteAttr       */



VOID   EQFBFillWriteAttrW
(
  PVOID pQFTagTable,
  PSZ_W pszMarkAttr,
  PSZ_W pszNoCountAttr,
  PSZ_W pszCurrentAttr,
  PSZ_W pszJoinAttr,
  PSZ_W pszNAttr,
  PSZ_W pszStatusAttr,
  PSZ_W pszCountAttr
  )
  {
     PBYTE            pByte;                  // helper pointer
     PTAGTABLE        pTagTable;                 // pointer to active QF tag table
     PATTRIBUTE       pAttr;                     // start of attributes in tagtable
     PSZ              pszSource;
     PSZ              pszStart;
     PTAG             pTag;
     PSZ              pTagNames;
     CHAR             chTemp[256];
     PSZ              pTemp = &chTemp[0];
     ULONG            ulOemCodePage = 0L;

     // get system preferences codepage
     ulOemCodePage = GetLangOEMCP(NULL);

     pTagTable = ((PLOADEDTABLE)pQFTagTable)->pTagTable;
     pByte = (PBYTE) (PVOID)pTagTable;
     pTag = (PTAG) ( pByte + pTagTable->stFixTag.uOffset);
     pTagNames = (PSZ)( pByte +  pTagTable-> uTagNames);

     // get / preprocess attribute names
     pAttr = (PATTRIBUTE) ( pByte + pTagTable->stAttribute.uOffset);

     strcpy(pTemp, pTagNames + pAttr[MARK_ATTR - pTagTable->uNumTags].uStringOffs);
     ASCII2Unicode( pTemp, pszMarkAttr, ulOemCodePage );

     strcpy(pTemp, pTagNames + pAttr[NOCOUNT_ATTR - pTagTable->uNumTags].uStringOffs );
     ASCII2Unicode( pTemp, pszNoCountAttr, ulOemCodePage );

     strcpy(pTemp, pTagNames + pAttr[CURRENT_ATTR - pTagTable->uNumTags].uStringOffs);
     ASCII2Unicode( pTemp, pszCurrentAttr, ulOemCodePage );
     pszSource = pTagNames + pAttr[JOIN_ATTR - pTagTable->uNumTags].uStringOffs;
     pszStart  = pTemp; //pszJoinAttr;
     while ( (*pszStart = *pszSource++) != NULC )
     {
        if ( *pszStart == CHAR_MULT_SUBST )
        {
           *pszStart++ = '%';
           *pszStart++ = 'd';
        }
        else
        {
           pszStart++;
        } /* endif */
     } /* endwhile */
     ASCII2Unicode( pTemp, pszJoinAttr, ulOemCodePage );

     pszSource = pTagNames + pAttr[N_ATTR - pTagTable->uNumTags].uStringOffs;
     pszStart = pTemp; //pszNAttr;
     while ( (*pszStart = *pszSource++) != NULC )
     {
        if ( *pszStart == CHAR_MULT_SUBST )
        {
           *pszStart++ = '%';
           *pszStart++ = 'l';
           *pszStart++ = 'u';
        }
        else
        {
           pszStart++;
        } /* endif */
     } /* endwhile */
     ASCII2Unicode( pTemp, pszNAttr, ulOemCodePage );


     pszSource = (pTagNames) +
                  pAttr[STATUS_ATTR - pTagTable->uNumTags].uStringOffs;
     pszStart = pTemp; //pszStatusAttr;
     while ( (*pszStart = *pszSource++) != NULC )
     {
        if ( *pszStart == CHAR_MULT_SUBST )
        {
           *pszStart++ = '%';
           *pszStart++ = 'd';
        }
        else
        {
           pszStart++;
        } /* endif */
     } /* endwhile */
     ASCII2Unicode( pTemp, pszStatusAttr, ulOemCodePage );

     // preprocess count attribute
     pszSource = pTagNames + pAttr[COUNT_ATTR - pTagTable->uNumTags].uStringOffs;
     pszStart = pTemp; //pszCountAttr;
     while ( (*pszStart = *pszSource++) != NULC )
     {
        if ( *pszStart == CHAR_MULT_SUBST )
        {
           strcpy( pszStart, "%4.4X%4.4X%4.4X%4.4X%4.4X" );
           pszStart += strlen( pszStart );
        }
        else
        {
           pszStart++;
        } /* endif */
     } /* endwhile */
     ASCII2Unicode( pTemp, pszCountAttr, ulOemCodePage );


  } /* end of EQFBFillWriteAttrW       */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBWriteNextSegment
//-----------------------------------------------------------------------------
// Function call:     EQFBWriteNextSegment
//-----------------------------------------------------------------------------
// Description:       write next segment to segmented file
//-----------------------------------------------------------------------------
// Parameters:
//-----------------------------------------------------------------------------
// Returncode type:   USHORT  usRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_WRITEFILE  - write to file failed
//-----------------------------------------------------------------------------
// Function flow:
//-----------------------------------------------------------------------------
USHORT
EQFBWriteNextSegment
(
    PTBDOCUMENT        pDoc,
    PVOID              pvFileWriteData,
    USHORT             usCPConversion,
    ULONG              ulCP,
    PBOOL              pfDone
)
{
    PFILEWRITEDATA pFileWriteData = (PFILEWRITEDATA)pvFileWriteData;
    USHORT           usRC = 0;
    PTBSEGMENT       pSeg = NULL;
    SHORT            sToken = 0;            // ID of current token/tag
    PTAG             pTag;                       // structure of active tag
    PSZ              pTagNames;
    PSZ              pszTag;                // text of segment tag
    PSZ_W            pszSource = NULL;      // ptr to segment data
    PSZ_W            pszStart = NULL;       // start of data
    USHORT           usSegOffset = 0;
    USHORT           usBufSize;
    BOOL             fResult = TRUE;
    PSZ_W            pszOutData;           // current position in output buffer
    CHAR_W           szTagW[GML_TAG_LEN];

    if ( !usRC )
    {
      pSeg = EQFBGetFromBothTables(pDoc, &(pFileWriteData->ulIndexStandard),
                                        &(pFileWriteData->ulIndexAdditional),
                                        &(pFileWriteData->ulLastTable));
    } /* endif */

    // write start tag and attributes to file
    if ( !usRC && pSeg && pSeg->pDataW )
    {
      switch ( pSeg->qStatus )
      {
        case QF_TOBE:    sToken = QFF_TAG;  break;
        case QF_NOP:     sToken = QFN_TAG;  break;
        case QF_XLATED:  sToken = QFX_TAG;  break;
        case QF_ATTR:    sToken = QFA_TAG;  break;
        case QF_CURRENT: sToken = QFC_TAG;  break;
        case QF_JOINED:  sToken = QFJ_TAG;  break;
        case QF_SPLIT:   sToken = QFS_TAG;  break;
      } /* endswitch */

      pTag = pFileWriteData->pTag;
      pTagNames = pFileWriteData->pTagNames;
      pszTag = pTag[sToken].uTagnameOffs + pTagNames;

      // write start tag to file
      pszOutData = pFileWriteData->szOutBufferW;

      ASCII2Unicode( pszTag, &szTagW[0], pDoc->ulOemCodePage );
      swprintf( pszOutData, L"%s ", szTagW );
      pszOutData += UTF16strlenCHAR( pszOutData );

      // write segment number attribute to file
      swprintf( pszOutData, pFileWriteData->szNAttr, pSeg->ulSegNum );
      pszOutData += UTF16strlenCHAR( pszOutData );

      // write join state attribute to file
      if ( pSeg->SegFlags.JoinStart || pSeg->SegFlags.Joined )
      {
         *pszOutData++ = ' ';
         swprintf( pszOutData, pFileWriteData->szJoinAttr,
                  ( pSeg->SegFlags.JoinStart ) ? 1 : 2 );
         pszOutData += UTF16strlenCHAR( pszOutData );
      } /* endif */

      // write mark attribute to file
      if ( pSeg->SegFlags.Marked )
      {
         *pszOutData++ = ' ';
         wcscpy( pszOutData, pFileWriteData->szMarkAttr  );
         pszOutData += UTF16strlenCHAR( pszOutData );
      } /* endif */

      // write current attribute to file
      if ( pSeg->SegFlags.Current )
      {
         *pszOutData++ = ' ';
         wcscpy( pszOutData, pFileWriteData->szCurrentAttr );
         pszOutData += UTF16strlenCHAR( pszOutData );
      } /* endif */

      // write mark attribute to file
      if ( pSeg->SegFlags.NoCount )
      {
         *pszOutData++ = ' ';
         wcscpy( pszOutData, pFileWriteData->szNoCountAttr );
         pszOutData += UTF16strlenCHAR( pszOutData );
      } /* endif */

      // write Status state attribute to file
      /**********************************************************/
      /* if Typed = TRUE and COpied = TRUE -> szStatusAttr =2  */
      /* if Typed = TRUE and Copied =FALSE -> szStatusAttr =1  */
      /* if Typed =FALSE and Copied = TRUE -> szStatusAttr =3  */
      /**********************************************************/
      if ( pSeg->SegFlags.Typed || pSeg->SegFlags.Copied  )
      {
         *pszOutData++ = ' ';
         if ( pSeg->SegFlags.Typed )
         {
           if ( pSeg->SegFlags.Copied )
           {
             swprintf(pszOutData, pFileWriteData->szStatusAttr, 2 );
           }
           else
           {
             swprintf(pszOutData, pFileWriteData->szStatusAttr, 1 );
           } /* endif */
         }
         else
         {
            swprintf(pszOutData, pFileWriteData->szStatusAttr, 3 );
         } /* endif */
         pszOutData += UTF16strlenCHAR( pszOutData );
      } /* endif */

      // write count attribute to file
      if ( pDoc->docType == STARGET_DOC )
      {
         if ( (*((PUSHORT)(&pSeg->CountFlag)) == 0) &&
              (pSeg->usSrcWords == 0) &&
              (pSeg->usTgtWords == 0) &&
              (pSeg->usModWords == 0) )
         {
           // nothing to do, count data is empty
         }
         else
         {
           USHORT usCheckSum = EQFBBuildCountCheckSum( *((PUSHORT)(&pSeg->CountFlag)),
              pSeg->usSrcWords, pSeg->usTgtWords, pSeg->usModWords );
           *pszOutData++ = ' ';
           swprintf( pszOutData, pFileWriteData->szCountAttr,
                    *((PUSHORT)(&pSeg->CountFlag)),
                    pSeg->usSrcWords,
                    pSeg->usTgtWords,
                    pSeg->usModWords,
                    usCheckSum );
           pszOutData += UTF16strlenCHAR( pszOutData );
         } /* endif */
      } /* endif */

      // write tag end character to file
      if ( !usRC )
      {
        UTF16strcpy( pszOutData, TAG_END_CHAR_W );
        pszOutData += UTF16strlenCHAR( pszOutData );
      } /* endif */

      // write segment data taking into account SO/SI and line breaks
      if ( !usRC )
      {
        pszSource = pSeg->pDataW;
        pszStart  = pSeg->pDataW;

        if ( fResult )
        {
          UTF16strcpy (pFileWriteData->szSegmentBufferW, pSeg->pDataW);
          pszSource = pFileWriteData->szSegmentBufferW;
          pszStart = pFileWriteData->szSegmentBufferW;
        } /* endif */

        /**********************************************************/
        /* remove soft LF if they are in the data                 */
        /**********************************************************/
        if ( pDoc->fLineWrap && pDoc->fAutoLineWrap )
        {
          if (fResult )
          {
            usSegOffset = 0;
            EQFBBufRemoveSoftLF( pDoc->hwndRichEdit, pFileWriteData->szSegmentBufferW, &usBufSize, &usSegOffset);
          } /* endif */
        } /* endif */

        // remove any softline feeds contained in the segment data
        if ( fResult )
        {
          PSZ_W pIn = pFileWriteData->szSegmentBufferW;
          PSZ_W pOut = pFileWriteData->szSegmentBufferW;
          while ( *pIn != 0 )
          {
            if ( *pIn == SOFTLF_CHAR )
            {
              pIn++;
            }
            else
            {
              *pOut++ = *pIn++;
            } /* endif */
          } /*endwhile */
          *pOut = 0;
        } /* endif */

        /**********************************************************/
        /* add removal of SOSI in case of DBCS                    */
        /**********************************************************/
        if (IsDBCS_CP(pDoc->ulOemCodePage))
        {
          ULONG ulConvDataLen = MAX_SEGMENT_SIZE;
          if ( pszSource != pFileWriteData->szSegmentBufferW )
          {
            EQFBUtlConvertSOSI( pFileWriteData->szSegmentBufferW, &ulConvDataLen,
                                pszSource, DELETE_SOSI, pDoc->ulOemCodePage );
            pszSource = pszStart = pFileWriteData->szSegmentBufferW;
          }
          else
          {
            EQFBUtlConvertSOSI(pszSource, &ulConvDataLen, pszSource, DELETE_SOSI,
                               pDoc->ulOemCodePage);
          } /* endif */
        }


        /**********************************************************/
        /* write segment data using the original linebreaks       */
        /**********************************************************/
        while ( *pszSource && !usRC )
        {
           if ( *pszSource == LF )
           {
              // write data up to linefeed character
              if ( pszStart != pszSource )
              {
                 UTF16strncpy( pszOutData, pszStart, (USHORT)(pszSource - pszStart) );
                 pszOutData += (pszSource - pszStart);
              } /* endif */

              // write linebreak characters
              if ( !usRC )
              {
                UTF16strcpy( pszOutData, pDoc->szLineBreakW );
                pszOutData += UTF16strlenCHAR( pszOutData );

                // skip to next character
                pszSource++;
                pszStart = pszSource;
              } /* endif */
           }
           else
           {
              pszSource++;
           } /* endif */
        } /* endwhile */
      } /* endif */

      // write remaining data of segment
      if ( (pszStart != pszSource) && !usRC )
      {
         UTF16strncpy( pszOutData, pszStart, (USHORT)(pszSource - pszStart) );
         pszOutData += (pszSource - pszStart);
      } /* endif */

      // write end tag to file
      if ( !usRC )
      {
         switch ( pSeg->qStatus )
         {
           case QF_TOBE:    sToken = EQFF_TAG;  break;
           case QF_NOP:     sToken = EQFN_TAG;  break;
           case QF_XLATED:  sToken = EQFX_TAG;  break;
           case QF_ATTR:    sToken = EQFA_TAG;  break;
           case QF_CURRENT: sToken = EQFC_TAG;  break;
           case QF_JOINED:  sToken = EQFJ_TAG;  break;
           case QF_SPLIT:   sToken = EQFS_TAG;  break;
         } /* endswitch */

         pszTag = pTag[sToken].uTagnameOffs + pTagNames;
         ASCII2Unicode( pszTag, &szTagW[0], pDoc->ulOemCodePage );
         swprintf( pszOutData, L"%s%s", szTagW, TAG_END_CHAR_W );

         pszOutData += UTF16strlenCHAR( pszOutData );
      } /* endif */

      // write end tag to file
      if ( !usRC )
      {
        switch ( usCPConversion )
        {
          case SGMLFORMAT_ANSI:
            {
			        LONG   lRc = NO_ERROR;
              ULONG ulChars = UtlDirectUnicode2AnsiBuf( pFileWriteData->szOutBufferW, pFileWriteData->szConvBuffer, wcslen(pFileWriteData->szOutBufferW), sizeof(pFileWriteData->szConvBuffer),
                                                        pDoc->ulAnsiCodePage, TRUE, &lRc );

			        usRC = (USHORT)lRc;
			        if ( !usRC )
			        {
                ULONG ulWritten;
                usRC = UtlWriteL( pFileWriteData->hOutFile, pFileWriteData->szConvBuffer, ulChars, &ulWritten, TRUE );
		 	        }
            }
            break;

          case SGMLFORMAT_ASCII:
            {
              ULONG ulChars = Unicode2ASCIIBuf( pFileWriteData->szOutBufferW, pFileWriteData->szConvBuffer, wcslen(pFileWriteData->szOutBufferW), sizeof(pFileWriteData->szConvBuffer), ulCP );

              ULONG ulWritten;
              usRC = UtlWriteL( pFileWriteData->hOutFile, pFileWriteData->szConvBuffer, ulChars, &ulWritten, TRUE );
            }
            break;

          default: 
            {
              ULONG ulWritten = 0;
              usRC = UtlWriteL( pFileWriteData->hOutFile, pFileWriteData->szOutBufferW, wcslen(pFileWriteData->szOutBufferW)*sizeof(pFileWriteData->szOutBufferW[0]), &ulWritten, TRUE );
            }
            break;
        } /* endswitch */
      } /* endif */
    } /* endif */

    if ( pfDone != NULL ) *pfDone = (pFileWriteData->ulIndexStandard >= pDoc->ulMaxSeg );
 

    return (usRC);
} /* end of EQFBWriteNextSegment    */



//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBGetFromBothTables
//-----------------------------------------------------------------------------
// Function call:     EQFBGetFromBothTables
//-----------------------------------------------------------------------------
// Description:       get next segment either from standard or additional table
//                    ONLY use this during FileWrite!!
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   pDoc,
//                    PUSHORT       pusStandardIndex,
//                    PUSHORT       pusAdditionalIndex,
//                    PUSHORT       pusLastTable
//-----------------------------------------------------------------------------
// Returncode type:   PTBSEGMENT
//-----------------------------------------------------------------------------
// Returncodes:       pSeg
//-----------------------------------------------------------------------------
// Function flow:   do
//                    if last segment has been from standard table
//                      get next segment from standard table
//                    if last segment has been from additional table
//                      or current segment from standard has Flag"InsertAdd"set
//                      get next segment from additional table
//                      if it is a delimitersegment
//                        use segment from standard table
//                    update Indices for both tables
//                  until a segment is found which should be written!
//-----------------------------------------------------------------------------
PTBSEGMENT
EQFBGetFromBothTables
(
  PTBDOCUMENT   pDoc,
  PULONG        pulStandardIndex,
  PULONG        pulAdditionalIndex,
  PULONG        pulLastTable
)
{
   PTBSEGMENT   pSeg = NULL;
   BOOL         fUseAdditionalTable = FALSE;
   ULONG        ulJ = *pulAdditionalIndex;
   ULONG        ulI = *pulStandardIndex;

   do
   {
     if (*pulLastTable == STANDARDTABLE )
     {
       pSeg = EQFBGetSegEx(pDoc, ulI, STANDARDTABLE);
       if (pSeg && pSeg->SegFlags.InsertAdd )
       {
         fUseAdditionalTable = TRUE;
       }
       else
       {
         ulI++;
       } /* endif */
     } /* endif */
     if ((*pulLastTable == ADDITIONALTABLE) || fUseAdditionalTable )
     {
       pSeg = EQFBGetSegEx(pDoc, ulJ, ADDITIONALTABLE);
       if (pSeg && pSeg->SegFlags.InsertAdd )
       {
         ulJ++;                         // use seg from addtable-it is not delimiterseg
         *pulLastTable = ADDITIONALTABLE;
       }
       else                             // seg is delimitersegment !
       {
         ulJ++;                         // fetch next segment next time
         pSeg = EQFBGetSegEx(pDoc, ulI, STANDARDTABLE);
         ulI++;
         *pulLastTable = STANDARDTABLE;
       } /* endif */
     } /* endif */
   } while ( pSeg && pSeg->SegFlags.NoWrite );    /* enddo */

   *pulAdditionalIndex = ulJ;             // point to next not-yet used!
   *pulStandardIndex = ulI;               // point to next not-yet-used!
   return (pSeg);
} /* end of EQFBGetFromBothTables */




//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBFileExists
//-----------------------------------------------------------------------------
// Function call:     EQFBFileExists(PSZ)
//-----------------------------------------------------------------------------
// Description:       checks if a given file exists
//-----------------------------------------------------------------------------
// Parameters:        PSZ pszFileName    - name of file
//-----------------------------------------------------------------------------
// Returncode type:   BOOL
//-----------------------------------------------------------------------------
// Returncodes:       TRUE              - file exists
//                    FALSE             - file does not exist
//-----------------------------------------------------------------------------
// Function flow:     use DosQFileMode to check if file exists
//-----------------------------------------------------------------------------

BOOL EQFBFileExists( PSZ pszFile )
{
   USHORT  usMode = 0;
   USHORT  usDosRC;                     // return code of Dos... alias Utl...

   usDosRC = UtlQFileMode( pszFile, &usMode, 0L, FALSE );

   return( usDosRC == 0 );
} /* endof EQFBFileExists */

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBCharType
//-----------------------------------------------------------------------------
// Function call:     EQFBCharType(PTBDOCUMENT,PTBSEGMENT,USHORT)
//-----------------------------------------------------------------------------
// Description:       determine type of character under cursor
//                    The function evaluates the type of a character at a
//                     given position in the segment table of document doc.
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   pDoc   - ptr to the structure describing
//                                           the active document
//                    PTBSEGMENT    pTBSeg - entry number of active segment
//                    USHORT        usOffs - offset of current character in
//                                            active segment
//-----------------------------------------------------------------------------
// Returncode type:   USHORT
//-----------------------------------------------------------------------------
// Returncodes:       usType - PROTECTED_CHAR   : the character is display only
//                           - HIDDEN_CHAR      : the character is hidden
//                           - UNPROTECTED_CHAR : the character can be changed
//                           - LINEBREAK_CHAR   : the character is a line break
//                           - ENDOFSEG_CHAR    : the character is outside the
//                                                    active segment
//                           - COMPACT_CHAR     : the character is compact
//
//-----------------------------------------------------------------------------
// Prerequesits:      The document tag table must have been loaded.
//-----------------------------------------------------------------------------
// Side effects:      If the start/stop table does not exist for the
//                    active segment, it is created.
//-----------------------------------------------------------------------------
// Function flow:     get ptr to segment
//                    if start/stop table is NULL
//                       tokenize segment
//                       convert tokens to entries in start/stop table
//                    endif
//                    if given offset greater than length of segment
//                      set type to ENDOFSEG_CHAR
//                    else if current char is linefeed
//                      set type to LINEBREAK_CHAR
//                      correct type to HIDDEN_CHAR if LF belongs
//                        to a inline tag which is 'compact'
//                    else if start/stop table alloc error
//                      set type to UNPROTECTED_CHAR
//                    else
//                      look for current position in start/stop table
//                      if type is valid
//                        set type to type of start/stop entry
//                      else
//                        set type to ENDOFSEG_CHAR
//                      endif
//                      correct QF_NOP depending on current display style
//                      correct usType depending on current display style
//                       if Type is protected
//                    endif
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBCheckNoneTag
//-----------------------------------------------------------------------------
// Function call:     EQFBCheckNoneTag( pDoc,pString)
//-----------------------------------------------------------------------------
// Description:       if segment consists not only of none tag,
//                    the none tag will be eliminated
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc    ptr to doc instance
//                    PSZ pString         ptr to string to be checked
//-----------------------------------------------------------------------------
// Returncode type:   BOOL
//-----------------------------------------------------------------------------
// Returncodes:       TRUE if s.th changed
//                    FALSE if nothing changed
//-----------------------------------------------------------------------------
// Side effects:      passed string pString can be changed
//-----------------------------------------------------------------------------
// Function flow:
//-----------------------------------------------------------------------------

BOOL EQFBCheckNoneTag
(
    PTBDOCUMENT pDoc,               // document structure
    PSZ_W       pString             // string to be checked
)
{
    PSZ_W  pData = pString;            // pointer to data
    USHORT usEmptylen ;                //length of NONE tag
    BOOL   fNoneRemoved = FALSE;       // true if none removed
    BOOL   fSegend = FALSE;            // true if segment end is reached
    BOOL   fText = FALSE;              // true if nonwhite space found
    CHAR_W c;                          //current char of pString
    CHAR_W chFirstChar;                //1st char of empty-tag
    CHAR_W chEmptyTag[ sizeof(EMPTY_TAG) + 1 ];
    BOOL   fWHITESPfound = FALSE;      // TRUE if blank or LF found


    /******************************************************************/
    /* fill chFirstChar and chEmptyTag and usEmptylen                 */
    /******************************************************************/
    UTF16strcpy( chEmptyTag, EMPTY_TAG );
    chFirstChar = chEmptyTag[0];
    usEmptylen = (USHORT) UTF16strlenCHAR( EMPTY_TAG );
    /******************************************************************/
    /* eliminate :NONE tags                                           */
    /******************************************************************/
    while ( !fSegend )
    {
      //loop until 1st char found or end of segment
      while ( ((c = *pData)!= NULC) && (chFirstChar != c) )
      {
        pData++;
      } /* endwhile */
      if ( !(*pData) )
      {
        fSegend = TRUE;
      }
      else
      {
        /**************************************************************/
        /* check for complete match                                   */
        /**************************************************************/
        if ( UTF16strncmp(pData, EMPTY_TAG, usEmptylen) == 0 )
        {
          /************************************************************/
          /* :NONE tag found! eliminate it                            */
          /************************************************************/
          fNoneRemoved = TRUE;
          pDoc->Redraw |= REDRAW_ALL;
          memcpy(pData, pData + usEmptylen, (UTF16strlenCHAR(pData+usEmptylen)+1)*sizeof(CHAR_W));
        }
        else      //goto next character
        {
          pData++;
        } /* endif */
      } /* endif */
    } /* endwhile */
    /******************************************************************/
    /* check whether segment is empty now                             */
    /* analog to EQFBDoEmptySegCheck, but w/o user asking             */
    /******************************************************************/
    pData = pString;
    while ( !fText && ((c=*pData) != NULC))
    {
      switch ( c )
      {
        case  LF:
        case BLANK:
          fWHITESPfound = TRUE;                             /* @KIT975A */
          break;
        default :
          fText = TRUE;
          break;
      } /* endswitch */
      pData ++;
    } /* endwhile */
    /******************************************************************/
    /* if no text found  re-insert none tag                           */
    /******************************************************************/
    if ( (!fText) && (!fWHITESPfound) )                     /* @KIT975M */
    {
      pData = pString;
      memmove(pData+usEmptylen, pData, (UTF16strlenCHAR(pData)+1)*sizeof(CHAR_W));
      memcpy(pData, EMPTY_TAG, usEmptylen*sizeof(CHAR_W));
    } /* endif */

    return(fNoneRemoved);
}

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBGetHexNumber
//-----------------------------------------------------------------------------
// Function call:     EQFBGetHexNumber( pszNumber, pusValue )
//-----------------------------------------------------------------------------
// Description:       Converts a hexadecimal string (e.g. "12AF") into
//                    an USHORT value.
//-----------------------------------------------------------------------------
// Parameters:        PSZ pszNumber       ptr to hexadecimal string (4digits!)
//                    PUSHORT pusValue    ptr to buffer for result
//-----------------------------------------------------------------------------
// Returncode type:   BOOL
//-----------------------------------------------------------------------------
// Returncodes:       TRUE  if number was a valid hexadecimal number
//                    FALSE if the number is not valid
//-----------------------------------------------------------------------------
// Function flow:
//-----------------------------------------------------------------------------
BOOL EQFBGetHexNumber( PSZ pszNumber, PUSHORT pusValue )
{
  BOOL        fOK = TRUE;              // function O.K. flag
  USHORT      usDigits = 0;            // number of digits processed
  *pusValue = 0;                       // initialize result field

  while ( fOK && (usDigits < 4 ) )
  {
    *pusValue = *pusValue << 4;
    if ( isxdigit(*pszNumber) )
    {
      if ( (*pszNumber >= '0') && (*pszNumber <= '9') )
      {
        *pusValue += *pszNumber - '0';
      }
      else
      {
        *pusValue = (USHORT)((*pusValue) + toupper(*pszNumber) -'A'+10);
      } /* endif */
    }
    else
    {
      fOK = FALSE;
    } /* endif */
    usDigits++;
    pszNumber++;
  } /* endwhile */
  return( fOK );
}


BOOL EQFBGetHexNumberW( PSZ_W pszNumber, PUSHORT pusValue )
{
  BOOL        fOK = TRUE;              // function O.K. flag
  USHORT      usDigits = 0;            // number of digits processed
  *pusValue = 0;                       // initialize result field

  while ( fOK && (usDigits < 4 ) )
  {
    *pusValue = *pusValue << 4;
    if ( isxdigit(*pszNumber) )
    {
      if ( (*pszNumber >= '0') && (*pszNumber <= '9') )
      {
        *pusValue += *pszNumber - '0';
      }
      else
      {
        *pusValue = (USHORT)((*pusValue) + toupper(*pszNumber) -'A'+10);
      } /* endif */
    }
    else
    {
      fOK = FALSE;
    } /* endif */
    usDigits++;
    pszNumber++;
  } /* endwhile */
  return( fOK );
}



//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBWriteHistLog
//-----------------------------------------------------------------------------
// Description:       Writes (appends) a record to the history log file
//                    of the specified folder
//-----------------------------------------------------------------------------
// Parameters:        PSZ      pszFolObjName   folder object name
//                                             (e.g. "E:\\EQF\\SAMPLE1.F00")
//                    PSZ      pszDocName      name of document
//                                             (e.g. "DEVICE.SCR")
//                    LOGTASK  TaskId          ID of task being logged
//                    USHORT   usAddInfoLength length of additional info
//                    PVOID    pvAddInfo       ptr to data area containing
//                                             additional info
//                    BOOL     fMsg            message handling flag,
//                                             TRUE = show errors
//                    HWND     hwndErrMsg      window handle to use for error
//                                             messages
//-----------------------------------------------------------------------------
// Returncode type:   USHORT
//-----------------------------------------------------------------------------
// Returncodes:       NO_ERROR (0)  if function completed successfully
//                    other         return code of UtlDos function calls
//-----------------------------------------------------------------------------
// Function flow:
//-----------------------------------------------------------------------------
USHORT EQFBWriteHistLog
(
  PSZ      pszFolObjName,    // folder object name (e.g. "E:\\EQF\\SAMPLE1.F00")
  PSZ      pszDocName,       // name of document (e.g. "DEVICE.SCR")
  SHORT    TaskId,           // ID of task being logged
  USHORT   usAddInfoLength,  // length of additional info
  PVOID    pvAddInfo,        // ptr to data area containing additional info
  BOOL     fMsg,             // message handling flag, TRUE = show errors
  HWND     hwndErrMsg        // window handle to use for error messages
)
{
  return( EQFBWriteHistLog2( pszFolObjName, pszDocName, TaskId, usAddInfoLength,
                             pvAddInfo, fMsg, hwndErrMsg, NULL ) );
} /* end of function EQFBWriteHistLog */

USHORT EQFBWriteHistLog2
(
  PSZ      pszFolObjName,    // folder object name (e.g. "E:\\EQF\\SAMPLE1.F00")
  PSZ      pszDocName,       // short name of document (e.g. "DEVICE.SCR")
  SHORT    TaskId,           // ID of task being logged
  USHORT   usAddInfoLength,  // length of additional info
  PVOID    pvAddInfo,        // ptr to data area containing additional info
  BOOL     fMsg,             // message handling flag, TRUE = show errors
  HWND     hwndErrMsg,       // window handle to use for error messages
  PSZ      pszLongDocName    // document long name or NULL if none
)
{
  USHORT  usRC = NO_ERROR;             // function return code
  CHAR    szLogFile[MAX_EQF_PATH];     // buffer for log file name
  HFILE   hLogFile = NULLHANDLE;       // handle of log file
  HISTLOGRECORD stLogRecord;           // fixed part of history log record
  USHORT   usLongDocName = 0;          // length of long document name or zero if none
  ULONG    ulWritten;                  // number of bytes written

  // get length of long document name (if any is specified)
  if ( (pszLongDocName != NULL) && (*pszLongDocName != EOS) )
  {
    // is long name same as short name???
    if ( strcmp( pszDocName, pszLongDocName ) == 0 )
    {
      // no need for long document data as szDocName contains correct document name
      usLongDocName = 0;
    }
    else
    {
      usLongDocName = (USHORT)strlen(pszLongDocName)+1;
    } /* endif */
  } /* endif */

  // build fully qualified name of log file
  // do not use UtlMakeEQFPath to allow usage from XLATE.EXE
  // environment
  strcpy( szLogFile, pszFolObjName );
  strcat( szLogFile, BACKSLASH_STR );
  strcat( szLogFile, PROPDIR );
  strcat( szLogFile, BACKSLASH_STR );
  strcat( szLogFile, HISTLOGFILE );

  // open log file for write
  if ( usRC == NO_ERROR )
  {
    USHORT usOpenAction;               // action performed by DosOpen

    usRC = UtlOpenHwnd( szLogFile, &hLogFile, &usOpenAction, 0L,
                        FILE_NORMAL, FILE_OPEN | FILE_CREATE,
                        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                        0L, fMsg, hwndErrMsg );
  } /* endif */

  // position to end of log file
  if ( usRC == NO_ERROR )
  {
    ULONG  ulNewPos;                  // new position of file pointer

    usRC = UtlChgFilePtrHwnd( hLogFile, 0L, FILE_END, &ulNewPos,
                              fMsg, hwndErrMsg );
  } /* endif */

  // setup fixed length part of history log record
  if ( usRC == NO_ERROR )
  {
    memset( &stLogRecord, 0, sizeof(stLogRecord) );
    stLogRecord.lEyeCatcher     = HISTLOGEYECATCHER;
    stLogRecord.Task            = (UCHAR)TaskId;
    stLogRecord.fLongNameRecord = TRUE; // document name is valid or long name follows
    stLogRecord.usSize          = sizeof(HISTLOGRECORD) + usAddInfoLength + usLongDocName;
    UtlTime( &(stLogRecord.lTime) );
    strcpy( stLogRecord.szDocName, pszDocName );
    stLogRecord.usAddInfoLength = usAddInfoLength;
  } /* endif */

  // write fixed length part of history log record
  if ( usRC == NO_ERROR )
  {
    usRC = UtlWriteHwnd( hLogFile, &stLogRecord, sizeof(stLogRecord),
                         &ulWritten, fMsg, hwndErrMsg );
  } /* endif */

  // for documents with long names write long name to histlog
  if ( (usRC == NO_ERROR) && (usLongDocName != 0) )
  {
    usRC = UtlWriteHwnd( hLogFile, pszLongDocName, usLongDocName,
                         &ulWritten, fMsg, hwndErrMsg );
  } /* endif */

  // write any variable length information
  if ( (usRC == NO_ERROR) && (usAddInfoLength != 0) )
  {
     usRC = UtlWriteHwnd( hLogFile, pvAddInfo, usAddInfoLength,
                         &ulWritten, fMsg, hwndErrMsg );
  } /* endif */

  // close log file
  if ( hLogFile )
  {
    USHORT usCloseRC;

    usCloseRC = UtlCloseHwnd( hLogFile, fMsg, hwndErrMsg );

    if ( usRC == NO_ERROR )
    {
      usRC = usCloseRC;
    } /* endif */
  } /* endif */

  // return to calling function
  return( usRC );
} /* end of function EQFBWriteHistLog */

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBWordCntPerSeg
//-----------------------------------------------------------------------------
// Description:       Writes (appends) a record to the history log file
//                    of the specified folder
//-----------------------------------------------------------------------------
// Parameters:        PLOADEDTABLE pTagTable,      // ptr to loaded tag t
//                    PTOKENENTRY  pTokBuf,        // ptr for token table
//                    PSZ          pData,          // pointer to data
//                    SHORT        sLanguageId,    // language ID
//                    PULONG       pulResult,      // result to be counte
//                    PULONG       pulMarkUp       // result for markup
//-----------------------------------------------------------------------------
// Returncode type:    SHORT
//-----------------------------------------------------------------------------
// Returncodes:       NO_ERROR (0)  if function completed successfully
//                    other         return code of UtlDos function calls
//-----------------------------------------------------------------------------
// Function flow:
//-----------------------------------------------------------------------------
SHORT
EQFBWordCntPerSeg
(
  PVOID        pVoidTable,             // ptr to loaded tag table
  PTOKENENTRY  pTokBuf,                // ptr for token table processing
  PSZ_W        pData,                  // pointer to data
  SHORT        sLanguageId,            // language ID
  PULONG       pulResult,              // result to be counted
  PULONG       pulMarkUp,               // result for markup
  ULONG        ulOemCP
)
{
  BOOL          sRc = 0;               // success indicator
  PLOADEDTABLE     pTagTable;                    // ptr to loaded tag table
#ifdef WORDCOUNTLOG
  FILE *hfLog = NULL; 
  ULONG ulOrgWords = *pulResult;
#endif

  // access tag table
  pTagTable = (PLOADEDTABLE) pVoidTable;

  // try to load user exit and protect table function if not done yet and function is
  // available (checking for availability and already loaded is done in called
  // functions)
  TALoadExitIfAvailable( pTagTable );
  TALoadProtectFunctionIfAvailable( pTagTable );


  // use new protect table based function if user exit has its own protect table exit
  // prereq: user exit and protect table function have been loaded during tag table load!
  if ( (pTagTable->pfnProtTable ) || (pTagTable->pfnProtTableW ) )
  {
    sRc = (BOOL)EQFBWordCntPerSegUsingProtTable( pVoidTable, pTokBuf, pData, sLanguageId,
                                                 pulResult, pulMarkUp, ulOemCP );
  }
  else
  {
    PCHAR_W       pRest;                 // ptr to start of not-processed bytes
    USHORT        usColPos = 0;          // column pos used by EQFTagTokenize
    PTOKENENTRY   pTok;                  // ptr for token table processing
    CHAR_W        chTemp;                // temp character
    USHORT        usListSize;            // size of buffer
    PFLAGOFFSLIST pTermList = NULL;      // pt to created term list
    PFLAGOFFSLIST pActTerm;              // actual term
  //BOOL          fMarkUp = FALSE;       // no markup active       /* @KIT0885C */
    PTAG             pTag;               // pointer to tags in tagtable
    PATTRIBUTE       pAttribute;         // pointer to attributes in tagtable
    PBYTE            pByte;              // help pointer for tag table addressing
    SHORT            sNumTags;           // number of tags in tag table
    PCHAR_W          pStart;             // start of data
    PCHAR_W          pStop;              // end of data
    BOOL             fXTrans;            // translatable info ??   /* @KIT0885A */
    BOOL             fTRNote;

    // Set tag table variables
    sNumTags   = (SHORT)pTagTable->pTagTable->uNumTags;
    pByte      = (PBYTE)pTagTable->pTagTable;
    pTag       = OFFSETTOPOINTER(PTAG, pTagTable->pTagTable->stFixTag.uOffset );
    pAttribute = OFFSETTOPOINTER(PATTRIBUTE,
                                pTagTable->pTagTable->stAttribute.uOffset );

#ifdef WORDCOUNTLOG
    {
      CHAR szLog[MAX_EQF_PATH];
      
      UtlMakeEQFPath( szLog, NULC, LOG_PATH, NULL );
      strcat( szLog, "\\EQFBCOUNTWORDS.LOG" );
      hfLog = fopen( szLog, "a" );
      if ( hfLog )
      {
        fprintf( hfLog, "Counting words in segment \"%S\"\n", pData );
      }
    }
#endif

        /* end   changes XJR */                                    /* @KIT0885A */
    /********************************************************************/
    /* run TATagTokenize to find tokens ....                            */
    /********************************************************************/
    pTok = pTokBuf;
    TATagTokenizeW( pData,
                    pTagTable,
                    TRUE,
                    &pRest,
                    &usColPos,
  //                 (PTOKENENTRY) pCNTIda->TBSrcDoc.pTokBuf,
                    pTokBuf,
                    TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );

    /********************************************************************/
    /* build tokenlist, i.e.                                            */
    /* convert tokens/text strings to entries in start-stop table       */
    /*                                                                  */
    /* Rational: use input buffer for temporary list ....               */
    /*           this is large enough, we can avoid any checking...     */
    /********************************************************************/
  //  pTok = (PTOKENENTRY) pCNTIda->TBSrcDoc.pTokBuf;
    while ( (pTok->sTokenid != ENDOFLIST) )
    {
      if ( pTok->sTokenid == TEXT_TOKEN )
      {
        usListSize = 0;
        pTermList = NULL;
        chTemp = *(pTok->pDataStringW+pTok->usLength);
        *(pTok->pDataStringW+pTok->usLength) = EOS;
        sRc = MorphTokenizeW(sLanguageId, pTok->pDataStringW,
                            &usListSize, (PVOID *)&pTermList,
                            MORPH_FLAG_OFFSLIST, ulOemCP );
#ifdef WORDCOUNTLOG
        if ( hfLog )
        {
          fprintf( hfLog, "  Counting words in text block \"%S\"\n", pTok->pDataStringW );
        }
#endif
        *(pTok->pDataStringW+pTok->usLength) = chTemp;

        if ( pTermList )
        {
          pActTerm = pTermList;
          while ( pActTerm->usLen )
          {
            /************************************************************/
            /* count only if it is no noise ( .,;:?!()crlf ...)         */
            /* and and no number                                        */
            /* (but count numbers) <== if the comments *1 are deleted   */
            /************************************************************/
              if ( !(pActTerm->lFlags & OtmMorph::TERMTYPE_NEWSENTENCE ))
              {
                if ( pActTerm->lFlags & OtmMorph::TERMTYPE_NOCOUNT )
                {
  //*1            if ( pActTerm->lFlags & TF_NUMBER )
  //*1            {
  //*1              (*pulResult)++;
  //*1            } /* endif */
                }
                else
                {
#ifdef WORDCOUNTLOG
                 if ( hfLog )
                 {
                   CHAR_W chEnd = pTok->pDataStringW[pActTerm->usOffs+pActTerm->usLen];
                   pTok->pDataStringW[pActTerm->usOffs+pActTerm->usLen] = 0;
                   fprintf( hfLog, "   word %2lu : \"%S\"\n", *pulResult - ulOrgWords + 1, pTok->pDataStringW + pActTerm->usOffs );
                   pTok->pDataStringW[pActTerm->usOffs+pActTerm->usLen] = chEnd;
                 }
#endif
                  (*pulResult)++;
                } /* endif */
              } /* endif */
            pActTerm++;
          } /* endwhile */
        } /* endif */
        /****************************************************************/
        /* free allocated resource ...                                  */
        /****************************************************************/
        UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
        /****************************************************************/
        /* get next token                                               */
        /****************************************************************/
        pTok++;
      }
      else
      {
        /****************************************************************/
        /* count tags and attributes only as one markup                 */
        /****************************************************************/
        (*pulMarkUp)++;
        /* start changes XJR */                                    /* @KIT0885A */
        /****************************************************************/
        /* count translatable info separately ....                      */
        /*  i.e. loop over all tagging info, combine it and count it..  */
        /****************************************************************/
        pStart = pTok->pDataStringW;
        pStop  = pStart + pTok->usLength;

        fXTrans = FALSE;
        fTRNote = FALSE;
        while ( pTok->sTokenid >= 0 )
        {
          pStop  = pTok->pDataStringW + pTok->usLength;
          if ( !fXTrans )
          {
            fXTrans = ( ( (pTok->sTokenid >= sNumTags) &&
                        (pAttribute[pTok->sTokenid - sNumTags].BitFlags.fTranslate) ) ||
                        ( (pTok->sTokenid >= 0)       &&
                          (pTok->sTokenid < sNumTags) &&
                          (pTag[pTok->sTokenid].BitFlags.fTranslate) ) );
            fTRNote = pTag[pTok->sTokenid].BitFlags.fTRNote;
          } /* endif */
          pTok++;
        } /* endwhile */

        if ( fXTrans )
        {
          chTemp = *pStop;
          *pStop = EOS;
          sRc = CountAttribs( pTagTable, pStart, sLanguageId, pulResult, pulMarkUp, ulOemCP);
          *pStop = chTemp;
        } /* endif */

        if (fTRNote )
        {
          /**************************************************************/
          /* do not count TRNotes as translatable info                  */
          /* skip til end of TRNote, indicated by a tag with fTRNote = 1*/
          /**************************************************************/
          while ( (pTok->sTokenid != ENDOFLIST )
                  && !pTag[pTok->sTokenid].BitFlags.fTRNote )
          {
            pTok++;
          } /* endwhile */
          if (pTag[pTok->sTokenid].BitFlags.fTRNote )
          {
            pTok++;                                // skip tag at end of TRNOTE
          } /* endif */
        } /* endif */
        /* end  changes XJR */                                     /* @KIT0885A */
      } /* endif */
    } /* endwhile */
  } /* endif */

#ifdef WORDCOUNTLOG
  if ( hfLog )
  {
    fprintf( hfLog, "   total number of words in segment: %lu\n\n", *pulResult - ulOrgWords );
    fclose( hfLog );
  }
#endif

  return ((SHORT)sRc);
} /* end of function EQFBWordCntPerSeg  */

// fucntion EQFBWordCntPerSegUsingProtTable
// same functionality as EQFBWordCntPerSeg but using CreateProtectTable
// and not supporting translators notes so far
SHORT EQFBWordCntPerSegUsingProtTable
(
  PVOID        pVoidTable,             // ptr to loaded tag table
  PTOKENENTRY  pTokBuf,                // ptr for token table processing
  PSZ_W        pData,                  // pointer to data
  SHORT        sLanguageId,            // language ID
  PULONG       pulResult,              // result to be counted
  PULONG       pulMarkUp,               // result for markup
  ULONG        ulOemCP
)
{
  BOOL          sRc = 0;               // success indicator
  USHORT        usListSize;            // size of buffer
  PFLAGOFFSLIST pTermList = NULL;      // pt to created term list
  PFLAGOFFSLIST pActTerm;              // actual term
  PLOADEDTABLE     pTagTable = (PLOADEDTABLE) pVoidTable;
  PSTARTSTOP   pStartStop = NULL;
  int          iIterations = 0;
  USHORT       usAddEntries = 0;
  PTOKENENTRY  pTokenList = NULL;


  USHORT usRC = TACreateProtectTableW( pData, pVoidTable, 1, pTokBuf,
                                       ( TOK_BUFFER_SIZE / sizeof(TOKENENTRY) ),
                                       &pStartStop,  pTagTable->pfnProtTable,
                                       pTagTable->pfnProtTableW, ulOemCP );

  // retry with larger buffer if token buffer is to small
  while ((iIterations < 10) && (usRC == EQFRS_AREA_TOO_SMALL))
  {
    // (re)allocate token buffer
    LONG lOldSize = (usAddEntries * sizeof(TOKENENTRY)) + (LONG)TOK_SIZE;
    LONG lNewSize = ((usAddEntries+128) * sizeof(TOKENENTRY)) + (LONG)TOK_SIZE;

    if (UtlAlloc((PVOID *) &pTokenList, lOldSize, lNewSize, NOMSG) )
    {
      usAddEntries += 128;
      iIterations++;
    }
    else
    {
      iIterations = 10;    // force end of loop
    } /* endif */

    // retry tokenization
    if (iIterations < 10 )
    {
      usRC = TACreateProtectTableW( pData, pVoidTable, 1, (PTOKENENTRY)pTokenList,
                                    (USHORT)lNewSize,
                                    &pStartStop,  pTagTable->pfnProtTable,
                                    pTagTable->pfnProtTableW, ulOemCP );
    } /* endif */
  } /* endwhile */

  // process entries in start stop list
  if ( usRC == 0 )
  {
    PSTARTSTOP pstCurrent = (PSTARTSTOP) pStartStop;

    while ( (pstCurrent->usType != 0) && !usRC )
    {
      if ( pstCurrent->usType == UNPROTECTED_CHAR )
      {
        CHAR_W chTemp = pData[ pstCurrent->usStop+1 ];
        PSZ_W pszStart = pData + pstCurrent->usStart;

        usListSize = 0;
        pTermList = NULL;

        pData[ pstCurrent->usStop+1 ] = EOS;
        sRc = MorphTokenizeW( sLanguageId, pszStart, &usListSize, (PVOID *)&pTermList,
                              MORPH_FLAG_OFFSLIST, ulOemCP );
        pData[ pstCurrent->usStop+1 ] = chTemp;

        if ( pTermList )
        {
          pActTerm = pTermList;
          while ( pActTerm->usLen )
          {
            if ( !(pActTerm->lFlags & OtmMorph::TERMTYPE_NEWSENTENCE ))
            {
              if ( pActTerm->lFlags & OtmMorph::TERMTYPE_NOCOUNT )
              {
              }
              else
              {
                (*pulResult)++;
              } /* endif */
            } /* endif */
            pActTerm++;
          } /* endwhile */
        } /* endif */
        UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
      }
      else
      {
        // count tags and attributes only as one markup
        (*pulMarkUp)++;
      } /* endif */
      pstCurrent++;
    } /*  endwhile */
  } /* endif */

  // cleanup
  UtlAlloc( (PVOID *)&pStartStop, 0L, 0L, NOMSG );
  if (pTokenList) UtlAlloc( (PVOID *)&pTokenList, 0L, 0L, NOMSG );

  return ((SHORT)sRc);
} /* end of function EQFBWordCntPerSegUsingProtTable  */


      /* start changes XJR */                                    /* @KIT0885A */
//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     CountAttribs
//-----------------------------------------------------------------------------
// Function call:     sRc = CountAttribs( pCNTIda, pData, sLanguageId,
//                                      pulResult, pulMarkUp );
//-----------------------------------------------------------------------------
// Description:       This function counts the words in the passed tag
//                    info area taking into account the used language ( POE !)
//-----------------------------------------------------------------------------
// Parameters:        PLOADEDTABLE pTagTable     pointer to Count controlblock
//                    PSZ          pData,        pointer to data
//                    SHORT        sLanguageId,  language ID
//                    PULONG       pulResult,    result to be counted
//                    PULONG       pulMarkUp     result for markup
//-----------------------------------------------------------------------------
// Returncode type:   SHORT
//-----------------------------------------------------------------------------
// Returncodes:       error return from POE or memory shortage
//-----------------------------------------------------------------------------
// Function flow:     create start/stop table
//                    loop thru list of tokens
//                     switch over type
//                       case protected:
//                         ignore -- nothing to count
//                       default:
//                         decomposite it using MorphTokenize
//                         loop thru text tokens and count them
//                     end case
//                     get next token
//                    endloop
//                    if no translatable info found yet,
//                      increase count by one (our default)
//                    free allocated resources
//                    return success indicator
//
//-----------------------------------------------------------------------------
static SHORT
CountAttribs
(
  PLOADEDTABLE pTagTable,              // ptr to loaded tag table
  PSZ_W        pData,                  // pointer to data
  SHORT        sLanguageId,            // language ID
  PULONG       pulResult,              // result to be counted
  PULONG       pulMarkUp,               // result for markup
  ULONG        ulOemCP
)
{
  BOOL          sRc = 0;               // success indicator
  CHAR_W        chTemp;                // temp character
  PSTARTSTOP    pStartStop = NULL;     // ptr to caller's start/stop table ptr

  // variables for static token buffer
  USHORT        usTokBufferSize = 64 * sizeof( TOKENENTRY );  // number of tokens
  static TOKENENTRY TokBuffer[ 64 ];

  // variables for allocated token buffer
  PTOKENENTRY pTokBuf = NULL;
  USHORT      usTokBufEntries = 0;
  int         iIterations;

  PSTARTSTOP    pAct;                  // ptr to active start/stop table entry
  USHORT        usListSize;            // size of buffer
  PFLAGOFFSLIST pTermList = NULL;      // pt to created term list
  PFLAGOFFSLIST pActTerm;              // actual term
  ULONG         ulResult = *pulResult; // currently active words

  pulMarkUp;                           // avoid compiler warnings

  /********************************************************************/
  /* use the function TACreateProctect table and build start stop     */
  /* table.                                                           */
  /* for each of the untranslatable parts use the CountWord function  */
  /* to count successful....                                          */
  /********************************************************************/
  sRc = TACreateProtectTableW( pData,
                               pTagTable,
                               0,
                               &TokBuffer[0],
                               usTokBufferSize,
                               &pStartStop,
                               NULL, NULL, ulOemCP );    /* no special user exit needed */

  iIterations = 0;
  while ( (iIterations < 10) && (sRc == EQFRS_AREA_TOO_SMALL) )
  {
    // (re)allocate token buffer
    LONG lOldSize = usTokBufEntries * sizeof(TOKENENTRY);
    LONG lNewSize = (usTokBufEntries + 128) * sizeof(TOKENENTRY);
    if ( UtlAlloc( (PVOID *)&pTokBuf, lOldSize, lNewSize, NOMSG ) )
    {
      usTokBufEntries += 128;
      iIterations++;
    }
    else
    {
      iIterations = 10;                // force end of loop
    } /* endif */

    // retry tokenization
    if ( iIterations < 10  )
    {
      sRc = TACreateProtectTableW( pData,
                                   pTagTable,
                                   0,
                                   pTokBuf,
                                   (USHORT)(usTokBufEntries * sizeof(TOKENENTRY)),
                                   &pStartStop,
                                   NULL, NULL, ulOemCP );        /* no special user exit needed */
    } /* endif */
  } /* endwhile */

  /********************************************************************/
  /* loop thru all unprotected parts and count the translatable words */
  /********************************************************************/
  if ( !sRc )
  {
    pAct = pStartStop;
    while ( pAct->usType && !sRc)
    {
      switch ( pAct->usType )
      {

        case  PROTECTED_CHAR:
          /************************************************************/
          /* ignore the stuff -- we don't care                        */
          /************************************************************/
          break;
        default :
          /************************************************************/
          /* send the stuff for counting and increase value ....      */
          /* (recursive call)                                         */
          /************************************************************/
          chTemp = *(pData + pAct->usStop + 1);
          *(pData + pAct->usStop + 1) = EOS;
          usListSize = 0;
          pTermList = NULL;
          sRc = MorphTokenizeW(sLanguageId,
                               pData + pAct->usStart,  // pointer to data
                               &usListSize, (PVOID *)&pTermList,
                               MORPH_FLAG_OFFSLIST, ulOemCP );
          *(pData + pAct->usStop + 1) = chTemp;

          if ( pTermList )
          {
            pActTerm = pTermList;
            while ( pActTerm->usLen )
            {
              /************************************************************/
              /* count only if it is no noise ( .,;:?!()crlf ...)         */
              /* and and no number                                        */
              /* (but count numbers) <== if the comments *1 are deleted   */
              /************************************************************/
                if ( !(pActTerm->lFlags & OtmMorph::TERMTYPE_NEWSENTENCE ))
                {
                  if ( pActTerm->lFlags & OtmMorph::TERMTYPE_NOCOUNT )
                  {
                  }
                  else
                  {
                    (*pulResult)++;
                  } /* endif */
                } /* endif */
              pActTerm++;
            } /* endwhile */
          } /* endif */
          /****************************************************************/
          /* free allocated resource ...                                  */
          /****************************************************************/
          UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
          break;
      } /* endswitch */
      pAct++;
    } /* endwhile */

    /******************************************************************/
    /* if no translatable word found yet, increase the count by the   */
    /* default...                                                     */
    /******************************************************************/
    if ( !sRc && (ulResult == *pulResult ))
    {
      (*pulResult)++;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* free allocated start stop table                                  */
  /********************************************************************/
  if ( pStartStop )
  {
    UtlAlloc( (PVOID *) &pStartStop, 0L, 0L, NOMSG );
  } /* endif */

  if ( pTokBuf ) UtlAlloc( (PVOID *)&pTokBuf, 0L, 0L, NOMSG );

  return ((SHORT)sRc);
} /* end of function CountAttribs */


//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBHistDocSave
//-----------------------------------------------------------------------------
// Function call:     EQFBHistDocSave(PSZ,PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:
//-----------------------------------------------------------------------------
// Parameters:        PSZ          pszFileName - the document's fully qualified
//                                               file name.
//                    PTBDOCUMENT  pDoc        - ptr to active doc structure
//-----------------------------------------------------------------------------
// Returncode type:    SHORT usRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_OPENFILE   - error opening file      ???
//                    ERR_READFILE   - error reading file         ???
//                    ERR_NOMEMORY   - memory allocation error       ???
//-----------------------------------------------------------------------------
// Prerequesits:      pQFTagTable in document structure must point to a loaded
//                    tag table for QFx tags
//
//-----------------------------------------------------------------------------
// Side effects:      if pDoc->pInBuf is NULL a new input buffer is allocated.
//-----------------------------------------------------------------------------
// Function flow:     - open the document
//
// changed for new information in summary counting report
//    TC NEW COUNTING REPORT
//
//-----------------------------------------------------------------------------
SHORT EQFBHistDocSave
(
   PSZ        pszFileName,             // ptr to fully qualified file name
   PTBDOCUMENT  pDoc,                  // ptr to document control structure
   SHORT        sLogTaskID             // log task ID or 0 for no log record
)
{
  return( EQFBHistDocSaveEx( pszFileName, pDoc, sLogTaskID ) );
}

SHORT EQFBHistDocSaveEx
(
   PSZ          pszFileName,             // ptr to fully qualified file name
   PTBDOCUMENT  pDoc,                  // ptr to document control structure
   SHORT        sLogTaskID            // log task ID or 0 for no log record
)
{
  SHORT   sRc = 0;                     // success
  ULONG   ulSegNum = 1;
  PTBSEGMENT pSeg;
  USHORT  dummy  = 0;
//  FILE *hLog = fopen( "C:\\COUNTLOG.LOG", "w" );

  float          fuzzyness=1.;          // fuzzy number for splitting
  PDOCSAVEHIST3  pHistSave3 = NULL;     // ptr to save struct in hist.file

  BOOL          fMsg = FALSE;
  CHAR          chEqfName[ MAX_EQF_PATH ];
  PSZ           pszFolder, pFile;
  CHAR          szSourceLang[MAX_LANG_LENGTH];
  CHAR          szTargetLang[MAX_LANG_LENGTH];
  CHAR          szDocObjName[MAX_EQF_PATH];
  static CHAR   szDocLongName[MAX_LONGFILESPEC];
  SHORT         sSrcLang = -1;
  SHORT         sTgtLang = -1;
  PSZ           pFileName = NULL;
  PSZ           pTemp = NULL;
  ULONG         ulSrcOemCP = 0L;
  ULONG         ulTgtOemCP = 0L;
  // the following two fields are for debuging only
  ULONG         ulTotalSrcWords = 0;
  ULONG         ulTotalSegs = 0;

#ifdef HISTDOCSAVELOG
  FILE *hLog = NULL;
  {
    CHAR  szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szLogFile, 0L, FALSE );
    strcat( szLogFile, "\\HISTDOCSAVE.LOG" );
    hLog = fopen( szLogFile, "a" );
    if ( hLog )
    {
      fprintf( hLog, "*** HistDocSaveLog, Doc=%s\n", pszFileName );
    } /* endif */
  }
#endif

  // we use for all doc save records the DOCSAVEHIST3 format so we change
  // the log tasks accordingly
  if ( (sLogTaskID == DOCSAVE_LOGTASK )  ||
       (sLogTaskID == DOCSAVE_LOGTASK2 ) ||
       (sLogTaskID == DOCSAVE_LOGTASK3 ) )
  {
    sLogTaskID = DOCSAVE_LOGTASK3;
  }
  else if ( sLogTaskID == AUTOMATICSUBST_LOGTASK )
  {
    sLogTaskID = AUTOMATICSUBST_LOGTASK3;
  }
  else if ( sLogTaskID == DOCAPI_LOGTASK )
  {
    sLogTaskID = DOCAPI_LOGTASK3;
  }
  else if ( (sLogTaskID == DOCIMPNEWTARGET_LOGTASK2) ||
            (sLogTaskID == DOCIMPNEWTARGET_LOGTASK) )
  {
    sLogTaskID = DOCIMPNEWTARGET_LOGTASK3;
  } /* endif */

  if (! UtlAlloc((PVOID *) &pHistSave3, 0L,
                 (LONG) sizeof( DOCSAVEHIST3), ERROR_STORAGE) )
  {
     sRc = ERR_NOMEMORY;
  } /* endif */
  if ( !sRc )
  {
    /******************************************************************/
    /* example:                                                       */
    /* pDoc->szDocName = d:\eqf\sample1.f00\STARGET\dev1scr.000       */
    /* szDocObjName = d:\eqf\sample1.f00\dev1scr.000                  */
    /******************************************************************/
    strcpy( szDocObjName, pDoc->szDocName );
    pFileName = UtlSplitFnameFromPath( szDocObjName);   // ptr to filename
    pTemp = UtlGetFnameFromPath( szDocObjName);         // ptr to starget
    //copy filename at position where STARGET was !!
    strcpy( pTemp, pFileName );                         // cpy fname
    DocQueryInfo2( szDocObjName,
                   NULL,             // memory
                   NULL,             // document format
                   szSourceLang,     // source language
                   szTargetLang,     // target language
                   szDocLongName,    // LongName,
                   NULL,             // Alias
                   NULL,             // editor
                   TRUE );
    if ((MorphGetLanguageID( szTargetLang, &sTgtLang ) != MORPH_OK) )
    {
      /****************************************************************/
      /* what should happen?                                          */
      /****************************************************************/
    } /* endif */
    if ((MorphGetLanguageID( szSourceLang, &sSrcLang ) != MORPH_OK) )
    {
      /****************************************************************/
      /* what should happen?                                          */
      /****************************************************************/
    } /* endif */
    ulSrcOemCP = GetLangOEMCP(szSourceLang);
    ulTgtOemCP = GetLangOEMCP( szTargetLang);
  } /* endif */

  if (!sRc )
  {
    pSeg = EQFBGetSegW(pDoc, ulSegNum);

    while (pSeg && (ulSegNum < pDoc->ulMaxSeg) )
    {

#ifdef HISTDOCSAVELOG
    if ( hLog )
    {
      CHAR szCountFlags[10];
      CHAR szStatus[20];
      int i = 0;
      if ( pSeg->CountFlag.AnalAutoSubst ) szCountFlags[i++] = 'A';
      if ( pSeg->CountFlag.EditAutoSubst)  szCountFlags[i++] = 'E';
      if ( pSeg->CountFlag.ExactExist )    szCountFlags[i++] = 'X';
      if ( pSeg->CountFlag.FuzzyExist )    szCountFlags[i++] = 'F';
      if ( pSeg->CountFlag.MachExist )     szCountFlags[i++] = 'M';
      if ( pSeg->CountFlag.ReplExist )     szCountFlags[i++] = 'R';
      if ( pSeg->CountFlag.GlobMemExist )  szCountFlags[i++] = 'G';

      szCountFlags[i] = EOS;

      if ( pSeg->qStatus == QF_XLATED )
      {
        strcpy( szStatus, "xlated" );
      }
      else if ( pSeg->qStatus == QF_TOBE )
      {
        strcpy( szStatus, "tobe" );
      }
      else if ( pSeg->qStatus == QF_ATTR )
      {
        strcpy( szStatus, "attr" );
      }
      else if ( pSeg->qStatus == QF_CURRENT )
      {
        strcpy( szStatus, "current" );
      }
      else if ( pSeg->qStatus == QF_NOP )
      {
        strcpy( szStatus, "protected" );
      }
      else
      {
        strcpy( szStatus, "unknown" );
      }

      fprintf( hLog, "Segment %lu: SourceWords=%u Status=%s CountFlags=%s \n", pSeg->ulSegNum, pSeg->usSrcWords,
               szStatus, szCountFlags );
    } /* endif */
#endif

      if ((pSeg->qStatus == QF_XLATED)
          || (pSeg->qStatus == QF_TOBE )
          || (pSeg->qStatus == QF_ATTR )
          || (pSeg->qStatus == QF_CURRENT)  )
      {
        if (pSeg->usSrcWords == 0 )
        {
          ULONG ulSrcWords  = 0L;
          ULONG ulSrcMarkUp = 0L;
          USHORT usRc;

          PTBSEGMENT  pSrcSeg;
          if (pDoc->twin )
          {
            pSrcSeg = EQFBGetSegW(pDoc->twin, ulSegNum);

            if (pSrcSeg && pSrcSeg->pDataW
                && pDoc->pDocTagTable )
            {
              usRc = EQFBWordCntPerSeg(
                               (PLOADEDTABLE)pDoc->pDocTagTable,
                               (PTOKENENTRY) pDoc->pTokBuf,
                               pSrcSeg->pDataW,
                               sSrcLang,
                               &ulSrcWords, &ulSrcMarkUp, ulSrcOemCP);

              pSeg->usSrcWords = (USHORT) ulSrcWords;
            } /* endif */
          } /* endif */

#ifdef HISTDOCSAVELOG
        if ( hLog )
        {
          fprintf( hLog, "Recomputed word count : SourceWords=%u\n", pSeg->usSrcWords );
        } /* endif */
#endif

        } /* endif */


//        fprintf( hLog, "Seg %8ld, Status = %4d, SourceWords = %d\n", pSeg->ulSegNum, (SHORT)pSeg->qStatus, pSeg->usSrcWords );
        if ((pSeg->qStatus == QF_XLATED) && (pSeg->usTgtWords == 0 ) && pDoc->pDocTagTable )
        {
          ULONG ulTgtWords  = 0L;
          ULONG ulTgtMarkUp = 0L;
          USHORT usRc;

          if ( sTgtLang == -1 )
          {
            sTgtLang = sSrcLang ;
          } /* endif */
          usRc = EQFBWordCntPerSeg(
                           (PLOADEDTABLE)pDoc->pDocTagTable,
                           (PTOKENENTRY) pDoc->pTokBuf,
                           pSeg->pDataW,
                           sTgtLang,
                           &ulTgtWords, &ulTgtMarkUp, ulTgtOemCP);

          pSeg->usTgtWords = (USHORT) ulTgtWords;
        } /* endif */
      } /* endif */

      if (pSeg->qStatus == QF_XLATED )
      {
        ulTotalSrcWords += pSeg->usSrcWords;
        ulTotalSegs++;

        if ( sLogTaskID != AUTOMATICSUBST_LOGTASK3 )
        {
           // calculate fuzzyness - RJ: used prior to ENH: new fuzzy overlap approach
           if (pSeg->usSrcWords>0)
           {
             fuzzyness = (float)(1. - ((float)pSeg->usModWords)/pSeg->usSrcWords);
           }
           else
           {
             fuzzyness = 0.0;
           }/* end if */
        } /* endif */

        if (pSeg->CountFlag.AnalAutoSubst )
        {
          EQFBSumUpWords(pSeg, &(pHistSave3->AnalAutoSubst));
        }
        else if (pSeg->CountFlag.EditAutoSubst)
        {
          EQFBSumUpWords(pSeg, &(pHistSave3->EditAutoSubst));
        }
        else if (pSeg->CountFlag.ExactExist)
        {
          EQFBSumUpWords(pSeg, &(pHistSave3->ExactExist));
        }
        else if (pSeg->CountFlag.GlobMemExist && !pSeg->CountFlag.FuzzyExist)
        {
          EQFBSumUpWords(pSeg, &(pHistSave3->ExactExist));
        }
        else if (pSeg->CountFlag.ReplExist)
		    {
		      EQFBSumUpWords(pSeg, &(pHistSave3->ReplExist));
        }
        else if (pSeg->CountFlag.FuzzyExist)
        {
          if ( sLogTaskID != AUTOMATICSUBST_LOGTASK3 )
          {
			      if (pSeg->CountFlag.Fuzzy5070  )
			      {
				      fuzzyness = (float)0.6;   // force fuzzy is counted in 50-70 range
		          }
		          else if ( pSeg->CountFlag.Fuzzy7190 )
			      {
				      fuzzyness = (float)0.80;   // force fuzzy is counted in 71-90 range
		          }
		          else if ( pSeg->CountFlag.Fuzzy9199 )
			      {
				      fuzzyness = (float)0.95;   // force fuzzy is counted in 91-99 range
		          }
		          // if no CountFlag.Fuzzyxxx is set, either Segment translated prior TM604,
		          // or fuzzyness below 50%...(to be tested)
		          // so use old fuzzyness calculation ( ModWords / SrcWOrds )
		          if (fuzzyness <= 0.49 )
		          {
		            EQFBSumUpWords(pSeg, &(pHistSave3->NoneExist));   // no prop existed
		          }
              else if (fuzzyness<=FUZZY_THRESHOLD_1)
              {
                EQFBSumUpWords(pSeg, &(pHistSave3->FuzzyExist_1));
                EQFBSumUpWords(pSeg, &(pHistSave3->FuzzyExist));
              }
              else if  (fuzzyness<=FUZZY_THRESHOLD_2)
              {
                EQFBSumUpWords(pSeg, &(pHistSave3->FuzzyExist_2));
                EQFBSumUpWords(pSeg, &(pHistSave3->FuzzyExist));
              }
              else
              {
                EQFBSumUpWords(pSeg, &(pHistSave3->FuzzyExist_3));
                EQFBSumUpWords(pSeg, &(pHistSave3->FuzzyExist));
		          }  /* end if fuzzyness */
          } /* endif */
        } /* FuzzyExist */

#ifdef COUNT_MT_MATCHES_ONLY_WHEN_USED
        // count only used MT matches as existing MT proposals
        else if (pSeg->CountFlag.MachCopy)
#else
        // count existing MT matches (independent of usage)
        else if (pSeg->CountFlag.MachExist)
#endif
        {
          EQFBSumUpWords(pSeg, &(pHistSave3->MachExist));
        }
        else
        {
          EQFBSumUpWords(pSeg, &(pHistSave3->NoneExist));   // no prop existed
        } /* endif */

        if ( sLogTaskID != AUTOMATICSUBST_LOGTASK3 )
        {
           // Based on used
           //****************

           if (pSeg->CountFlag.AnalAutoSubst )
           {
             dummy ++;
           }
           else if (pSeg->CountFlag.EditAutoSubst)
           {
             dummy ++;
           }
           else if ( pSeg->CountFlag.ExactCopy || pSeg->CountFlag.GlobMemCopy  )
           {
              EQFBSumUpWords(pSeg, &(pHistSave3->ExactUsed));
           }
           else if (pSeg->CountFlag.ReplCopy)
		       {
		        EQFBSumUpWords(pSeg, &(pHistSave3->ReplUsed));
           }
           else if (pSeg->CountFlag.FuzzyCopy)
           {
              if (pSeg->CountFlag.Fuzzy5070  )
			        {
  				      fuzzyness = (float)0.60;   // force fuzzy is counted in 50-70 range
			        }
			        else if ( pSeg->CountFlag.Fuzzy7190 )
			        {
	  			      fuzzyness = (float)0.80;   // force fuzzy is counted in 71-90 range
			        }
			        else if ( pSeg->CountFlag.Fuzzy9199 )
			        {
				        fuzzyness = (float)0.95;   // force fuzzy is counted in 91-99 range
	            }
	            if (fuzzyness < FUZZY_THRESHOLD_0 )
			        { // with new fuzzy calc. this should not occur...
			            EQFBSumUpWords(pSeg, &(pHistSave3->NoneExist2));   // no prop existed
	            }
              else if (fuzzyness<=FUZZY_THRESHOLD_1)
              {
                 EQFBSumUpWords(pSeg, &(pHistSave3->FuzzyUsed_1));
                 EQFBSumUpWords(pSeg, &(pHistSave3->FuzzyUsed));
              }
              else if  (fuzzyness<=FUZZY_THRESHOLD_2)
              {
                 EQFBSumUpWords(pSeg, &(pHistSave3->FuzzyUsed_2));
                 EQFBSumUpWords(pSeg, &(pHistSave3->FuzzyUsed));
              }
              else
              {
                 EQFBSumUpWords(pSeg, &(pHistSave3->FuzzyUsed_3));
                 EQFBSumUpWords(pSeg, &(pHistSave3->FuzzyUsed));
              }  /* end if fuzzyness */
            } /* end if FuzzyCopy*/
            else if (pSeg->CountFlag.MachCopy)
            {
               EQFBSumUpWords(pSeg, &(pHistSave3->MachUsed));
            }
            else
            {
               EQFBSumUpWords(pSeg, &(pHistSave3->NoneExist2));   // no prop existed
            } /* endif */
          } /* endif */
       }
       else
       {
         if ( (pSeg->qStatus == QF_TOBE) || (pSeg->qStatus == QF_ATTR) || (pSeg->qStatus == QF_CURRENT)  )
         {
           if ( !pSeg->SegFlags.Joined )
           {
             ulTotalSrcWords += pSeg->usSrcWords;
             ulTotalSegs++;

             /**********************************************************/
             /* do not count joined segs at all                        */
             /**********************************************************/
             EQFBSumUpWords(pSeg, &(pHistSave3->NotXlated));   // no xlated
           } /* endif */
         } /* endif */
       } /* endif */
      ulSegNum++;
      pSeg = EQFBGetSegW(pDoc, ulSegNum);
    } /* endwhile */


    strcpy( chEqfName, pDoc->szDocName );
    pszFileName = UtlGetFnameFromPath( chEqfName );
    if ( pszFileName )
    {
      /**********************************************************/
      /* chEQFName = "c:\eqf\sample1.f00\sTarget\"              */
      /**********************************************************/
      pszFolder = pszFileName - 1;
      *pszFolder = EOS;
      pFile = UtlGetFnameFromPath(chEqfName);
      pszFolder = pFile - 1;
      *pszFolder = EOS;
      pszFolder =  &chEqfName[0];

      // fill fuzzy level fields
      pHistSave3->lLargeFuzzLevel  = (LONG)UtlQueryULong( QL_LARGELOOKUPFUZZLEVEL );
      pHistSave3->lMediumFuzzLevel = (LONG)UtlQueryULong( QL_MEDIUMLOOKUPFUZZLEVEL );
      pHistSave3->lSmallFuzzLevel  = (LONG)UtlQueryULong( QL_SMALLLOOKUPFUZZLEVEL );


      sRc = EQFBWriteHistLog2(pszFolder, pszFileName,
                              sLogTaskID, (USHORT) sizeof(DOCSAVEHIST3),
                             (PVOID) pHistSave3, fMsg, (HWND) NULLHANDLE, szDocLongName );

    }
  } /* endif */

  if (pHistSave3 ) UtlAlloc((PVOID *) &pHistSave3, 0L, 0L, NOMSG );

//  fclose( hLog );

#ifdef HISTDOCSAVELOG
    if ( hLog )
    {
      fclose ( hLog );
    } /* endif */
#endif

  return sRc;
} /* end of function EQFBHistDocSave */




//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBSumUpWords
//-----------------------------------------------------------------------------
// Function call:     EQFBSumUpWords
//-----------------------------------------------------------------------------
// Description:
//-----------------------------------------------------------------------------
// Parameters:        PTBSEGMENT  pSEg
//                    PCRITERIASUM  pCritSum
//-----------------------------------------------------------------------------
// Returncode type:   USHORT usRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//-----------------------------------------------------------------------------
// Function flow:
//
//-----------------------------------------------------------------------------
USHORT EQFBSumUpWords
(
   PTBSEGMENT   pSeg,                  // ptr to fully qualified file name
   PCRITERIASUM pCritSum                 // ptr to document control structure
)
{
  USHORT    usRc = 0;
  USHORT    usSrcWords;
  USHORT    fCount = TRUE;

  usSrcWords = pSeg->usSrcWords;

  if (pSeg->SegFlags.NoCount )
  {
    fCount = FALSE;
  } /* endif */

  if (fCount )
  {
     if (usSrcWords < SIMPLE_SENT_BOUND )
     {
       pCritSum->SimpleSum.usNumSegs++;
       pCritSum->SimpleSum.ulSrcWords += (ULONG) usSrcWords;
       pCritSum->SimpleSum.ulModWords += (ULONG) (pSeg->usModWords);
       pCritSum->SimpleSum.ulTgtWords += (ULONG) (pSeg->usTgtWords);
     }
     else if (usSrcWords < MEDIUM_SENT_BOUND )
     {
       pCritSum->MediumSum.usNumSegs++;
       pCritSum->MediumSum.ulSrcWords += (ULONG) usSrcWords;
       pCritSum->MediumSum.ulModWords += (ULONG) (pSeg->usModWords);
       pCritSum->MediumSum.ulTgtWords += (ULONG) (pSeg->usTgtWords);
     }
     else
     {
       pCritSum->ComplexSum.usNumSegs++;
       pCritSum->ComplexSum.ulSrcWords += (ULONG) usSrcWords;
       pCritSum->ComplexSum.ulModWords += (ULONG) (pSeg->usModWords);
       pCritSum->ComplexSum.ulTgtWords += (ULONG) (pSeg->usTgtWords);
     } /* endif */
  } /* endif */
  return usRc;
} /* end of function EQFBHistDocSave */


static
VOID  EQFBWCntClearInfo
(
  PTBDOCUMENT pDoc,
  PSZ         pszReadFileName,
  PSZ         pszSegmentList           // list of corrupted segments
)
{
//  ULONG         ulSegNum = 1;
//  PTBSEGMENT    pSeg;
 CHAR          chEqfName[ MAX_EQF_PATH ];
  PSZ           pszFolder;
  PSZ           pFile;
  PSZ           pszFileName;
  CHAR          chTempName[MAX_EQF_PATH + 2*MAX_FILESPEC ];
  CHAR          chFromName[MAX_EQF_PATH + 2*MAX_FILESPEC ];
  CHAR          chSpecPathName[MAX_EQF_PATH + 2*MAX_FILESPEC ];
  CHAR          szDate[12];
  struct tm     *pTimeDate;            // time/date structure
  time_t        lTime;
  CHAR          szDir[MAX_FILESPEC];
  PSZ           pszMsgTable[3];

  // GQ: no wordcount reset anymore for complete file

      ///********************************************************************/
      ///* clear CountFlags and usSrcWords/ usTgtWords / usModWords         */
      ///********************************************************************/
      //pSeg = EQFBGetSegW(pDoc, ulSegNum);

      //while (pSeg && (ulSegNum < pDoc->ulMaxSeg) )
      //{
      //  pSeg->usSrcWords = pSeg->usTgtWords = pSeg->usModWords = 0;
      //  *((PUSHORT)(&pSeg->CountFlag)) = 0;

      //  ulSegNum++;
      //  pSeg = EQFBGetSegW(pDoc, ulSegNum);
      //} /* endwhile */

    strcpy( chEqfName, pDoc->szDocName );
    pszFileName = UtlGetFnameFromPath( chEqfName );
    if ( pszFileName )
    {
      /**********************************************************/
      /* chEQFName = "c:\eqf\sample1.f00\sTarget\"              */
      /**********************************************************/
      pszFolder = pszFileName - 1;
      *pszFolder = EOS;
      pFile = UtlGetFnameFromPath(chEqfName);
      pszFolder = pFile - 1;
      *pszFolder = EOS;
      pszFolder =  &chEqfName[0];

      // GQ: no reset entry in histlog anymore

      //EQFBWriteHistLog2(pszFolder, pszFileName,
      //              HISTDATA_INVALID_LOGTASK, 0,
      //              NULL, FALSE, (HWND) NULLHANDLE, pDoc->szDocLongName );


    if (pDoc->docType == STARGET_DOC )
    {
      // write history log record for corrupted target document
      EQFBWriteHistLog2( pszFolder, pszFileName, STARGET_CORRUPT_LOGTASK, 0, NULL, FALSE, (HWND) NULLHANDLE, pDoc->szDocLongName );

#ifdef SHOW_CORRUPTEDDATA_MSG
      /********************************************************************/
      /* write additional info in extra ADDINFO subdirectory              */
      /********************************************************************/
      sprintf(chTempName, "%s\\%s", pszFolder, ADDINFO_DIR);
      UtlMkDir (chTempName, 0L, FALSE);
      szDate[0] = NULC;                  // clear buffer

      time(&lTime);
      pTimeDate = localtime( &lTime );
      if ( (lTime != 0L) && pTimeDate )   // if localtime was successful ...
      {
         sprintf(&szDate[0], "%2.2d%2.2d%2.2d%2.2d",
                      pTimeDate->tm_mon + 1,
                      pTimeDate->tm_mday,
                      pTimeDate->tm_hour,
                      pTimeDate->tm_min );

        sprintf(chTempName, "%s\\%s\\%s", pszFolder,ADDINFO_DIR, szDate);
        UtlMkDir (chTempName, 0L, FALSE);

        //copy segmented target file
        UtlQueryString( QST_SEGTARGETDIR, szDir, sizeof(szDir) );
        sprintf(chSpecPathName,  "%s\\%s",chTempName, szDir);
        UtlMkDir(chSpecPathName, 0L, FALSE);
        sprintf(chSpecPathName, "%s\\%s\\%s", chTempName, szDir, pszFileName);

        /************************************************************/
        /* copy only if file does not already exist                 */
        /************************************************************/
        if (!UtlFileExist(chSpecPathName) )
        {
          UtlCopy(pszReadFileName ,chSpecPathName, 1, 0L, FALSE);

          //copy segmented source file
          UtlQueryString( QST_SEGSOURCEDIR, szDir, sizeof(szDir) );
          sprintf(chSpecPathName, "%s\\%s",chTempName, szDir);
          UtlMkDir(chSpecPathName, 0L, FALSE);
          sprintf(chSpecPathName, "%s\\%s\\%s", chTempName, szDir, pszFileName);
          sprintf(chFromName, "%s\\%s\\%s", pszFolder, szDir, pszFileName);
          UtlCopy(chFromName, chSpecPathName, 1, 0L, FALSE);

          //copy from property directory
          UtlQueryString( QST_PROPDIR, szDir, sizeof(szDir) );
          sprintf(chSpecPathName, "%s\\%s",chTempName, szDir);
          UtlMkDir(chSpecPathName, 0L, FALSE);
          sprintf(chSpecPathName, "%s\\%s\\%s", chTempName, szDir, pszFileName);
          sprintf(chFromName, "%s\\%s\\%s", pszFolder, szDir, pszFileName);
          UtlCopy(chFromName, chSpecPathName, 1, 0L, FALSE);

          //copy file from source directory
          UtlQueryString( QST_SOURCEDIR, szDir, sizeof(szDir) );
          sprintf(chSpecPathName, "%s\\%s",chTempName, szDir);
          UtlMkDir(chSpecPathName, 0L, FALSE);
          sprintf(chSpecPathName, "%s\\%s\\%s", chTempName, szDir, pszFileName);
          sprintf(chFromName, "%s\\%s\\%s", pszFolder, szDir, pszFileName);
          UtlCopy(chFromName, chSpecPathName, 1, 0L, FALSE);
          pszMsgTable[0] = pDoc->szDocLongName[0] ? pDoc->szDocLongName : pszFileName;
          pszMsgTable[1] = &chTempName[0];
          pszMsgTable[2] = pszSegmentList;
          UtlError( ERROR_WORDCOUNT_INVALID_ADDINFO, MB_OK, 3, pszMsgTable, EQF_ERROR);
        } /* endif */
      } /* endif */
#endif

      /************************************************************/
      /* force write of document with the updated info...         */
      /************************************************************/
      EQFBFileWriteEx( pszReadFileName, pDoc, 0, 0 );
    } /* endif */
  } /* endif */
  return ;
} /* end of function EQFBWCntClearInfo */



// copy of file in eqfparse.c
// needed since not all parsers include parse-api
// so handle this function like eqfbfileread -
// since it should be called always prior to calling EQFBFIleRead to fill TBDoc-cp's

VOID EQFBParseGetCP
(
  PSZ  pszInFile,
  PULONG pulSrcOemCP,
  PULONG pulTgtOemCP,
  PULONG pulSrcAnsiCP,
  PULONG pulTgtAnsiCP
)
{
  PSZ    pszTemp = NULL;
  CHAR   szTgtLang[MAX_LANG_LENGTH];
  CHAR   szSrcLang[MAX_LANG_LENGTH];
  CHAR   szTempBuf[MAX_LONGFILESPEC];
  CHAR   abOutBuf[MAX_LONGFILESPEC];
  CHAR   szFolder[MAX_LONGFILESPEC];

  USHORT usRC = NO_ERROR;

  strcpy( abOutBuf, pszInFile );
  pszTemp  = UtlGetFnameFromPath( abOutBuf );
  pszTemp--;
  *pszTemp = EOS;
  pszTemp  = UtlGetFnameFromPath( abOutBuf );
  pszTemp--;
  *pszTemp = EOS;
  Utlstrccpy( szFolder, UtlGetFnameFromPath( abOutBuf ), DOT );

  UtlMakeEQFPath( szTempBuf, *pszInFile, SYSTEM_PATH, NULL );
  strcat( szTempBuf, BACKSLASH_STR );
  strcat( szTempBuf, szFolder );
  strcat( szTempBuf, EXT_FOLDER_MAIN );
  strcat( szTempBuf, BACKSLASH_STR );
  strcat( szTempBuf, UtlGetFnameFromPath( pszInFile ) );

  usRC = DocQueryInfo( szTempBuf,    // object name of document
                       NULL,                   // translation memory or NULL
                       NULL,                   // folder format or NULL
                       szSrcLang,               // source language or NULL
                       szTgtLang,                   // target language or NULL
                       TRUE );                 // do-message-handling flag
  if ( usRC != NO_ERROR )
  {
    // set to system preferences default
    if (pulSrcOemCP) *pulSrcOemCP = GetLangOEMCP(NULL);
    if (pulTgtOemCP) *pulTgtOemCP = GetLangOEMCP(NULL);
    if (pulSrcAnsiCP) *pulSrcAnsiCP = GetLangAnsiCP(NULL);
    if (pulTgtAnsiCP) *pulTgtAnsiCP = GetLangAnsiCP(NULL);
  }
  else
  {
    if (pulSrcOemCP) *pulSrcOemCP = GetLangOEMCP(szSrcLang);
    if (pulTgtOemCP) *pulTgtOemCP = GetLangOEMCP(szTgtLang);
    if (pulSrcAnsiCP) *pulSrcAnsiCP = GetLangAnsiCP(szSrcLang);
    if ( pulTgtAnsiCP) *pulTgtAnsiCP = GetLangAnsiCP(szTgtLang);
  }

  return;
}

USHORT usCheckSumMods[] = {345,423,2,843,65,65323,42,9,432,543,123,2,3200,342,9741,5422,12300,876,3,17,511};

// build checksum for enhanced count attribute (X=)
USHORT EQFBBuildCountCheckSum
(
  USHORT usCountFlags,
  USHORT usSrcWords,
  USHORT usTgtWords,
  USHORT usModWords
)
{
  CHAR szCountDummy[20];
  USHORT usCheckSum = 0;
  PSZ pszTemp = szCountDummy;
  int iIndex = 0;

  sprintf( szCountDummy, "%4.4X%4.4X%4.4X%4.4X", usCountFlags, usSrcWords, usTgtWords, usModWords );

  while ( *pszTemp )
  {
    usCheckSum = usCheckSum + (USHORT)(*pszTemp++ + usCheckSumMods[iIndex++]);
  } /* endwhile */

  return( usCheckSum );
} /* end of function EQFBCountCheckSum */

// helper function for EQFBFuncCharIn
BOOL
EQFBIsLFProtected
(
	PTBSEGMENT  pSeg,
	SHORT       sOffs
)
{
	PSTARTSTOP  pstCurrent;
	BOOL        fTypeIns = TRUE;

	 // look for position in start/stop table
	  pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
	  while ( (pstCurrent->usType != 0) && (sOffs > pstCurrent->usStop) )
	  {
		 pstCurrent++;
	  } /* endwhile */
	  // do we need to check TAGPROT_CHAR too?
	  if ((pstCurrent->usType == PROTECTED_CHAR) &&
		   (pstCurrent->usStart < sOffs) && (sOffs < pstCurrent->usStop))
	  { // LF in the middle of protected area - do not allow insert!!
		 fTypeIns = FALSE;
	  }
	  return (fTypeIns);
}

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     TAIteratedCreateProtectTableW
//-----------------------------------------------------------------------------
// Function call:     TAIteratedCreateProtectTableW
//-----------------------------------------------------------------------------
// Description:       Creates a table with start/stop info for protected/
//                    unprotected data in input string.
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc        document
//                    PTBSEGMENT  pSeg        segment for which prot.table
//-----------------------------------------------------------------------------
// Returncode type:   USHORT
//-----------------------------------------------------------------------------
// Returncodes:       NO_ERROR              if function completed successfully
// Returncodes:       ERROR_INVALID_DATA    if one of the input parameter is in
//                                          error
//                    ERROR_NOT_ENOUGH_MEMORY if memory allocation failed
//-----------------------------------------------------------------------------
USHORT TAIteratedCreateProtectTableW
(
	PTBDOCUMENT pDoc,
	PTBSEGMENT  pSeg,
	int         iMaxIterations
)
{
   int iIterations = 0;
   LONG lTempTokBufSize = 0;
   PBYTE pTempTokBuf = NULL;
   USHORT usColPos = 0;
   USHORT usRC = NO_ERROR;

   if (pSeg->ulSegNum >1 )
   {
	   ULONG ulPrevSeg = pSeg->ulSegNum - 1;
	   PTBSEGMENT psPrevSeg = NULL;
	   psPrevSeg = EQFBGetSegW(pDoc, ulPrevSeg);
	   if (psPrevSeg->usLength)
	   {
		   CHAR_W chTemp;
		   chTemp = psPrevSeg->pDataW[psPrevSeg->usLength-1];
		   if ( chTemp == '\r' || chTemp == '\n' )
		   {
			   usColPos = 0;
		   }
		   else
		   {
			   usColPos = 1;
		   }
	   }
   }
   usRC = TACreateProtectTableW( pSeg->pDataW,
								pDoc->pDocTagTable,
								usColPos,
								(PTOKENENTRY) pDoc->pTokBuf,
								TOK_BUFFER_SIZE,
								(PSTARTSTOP *) &(pSeg->pusBPET),
								pDoc->pfnUserExit,
								pDoc->pfnUserExitW, pDoc->ulOemCodePage);

   // dynamically allocate token buffer if standard size is too small
  while ((iIterations < iMaxIterations) && (usRC == EQFRS_AREA_TOO_SMALL))
  {
	// (re)allocate token buffer
	LONG lOldSize = lTempTokBufSize;
	LONG lNewSize = (lTempTokBufSize == 0) ?
						(TOK_BUFFER_SIZE + TOK_BUFFER_SIZE ) :
						(lTempTokBufSize + TOK_BUFFER_SIZE);

	if (UtlAlloc((PVOID *) &pTempTokBuf, lOldSize, lNewSize, NOMSG) )
	{
	  lTempTokBufSize = lNewSize;
	  iIterations++;
	}
	else
	{
	  iIterations = iMaxIterations;    // force end of loop
	} /* endif */

	// retry tokenization
	if (iIterations < iMaxIterations )
	{
	   usRC = TACreateProtectTableW( pSeg->pDataW,
									pDoc->pDocTagTable,
									usColPos,
									(PTOKENENTRY)pTempTokBuf,
									(USHORT)lTempTokBufSize ,
									(PSTARTSTOP *) &(pSeg->pusBPET),
									pDoc->pfnUserExit,
									pDoc->pfnUserExitW, pDoc->ulOemCodePage);
	} /* endif */
  } /* endwhile */

      // get rid off any temp token buffer
  if ( pTempTokBuf ) UtlAlloc((PVOID *)&pTempTokBuf, 0L, 0L, NOMSG);

  return(usRC);
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     UtlConvertSOSI
//------------------------------------------------------------------------------
// Function call: BOOL UtlConvertSOSI( PSZ     pszOutBuffer,
//                                     PUSHORT pusSizeOutBuffer,
//                                     PSZ     pszInBuffer,
//                                     USHORT  usConvert )
//------------------------------------------------------------------------------
// Description:       The SBCS/DBCS mixed string passed in the input buffer
//                    is parsed, SO/SI is inserted or deleted before/after
//                    each DBCS character sequence. The converted string is
//                    then available in the output buffer. When inserting
//                    DBCS characters the first DBCS character found is
//                    interpreted as the start of a DBCS sequence and a
//                    SO is inserted. After the last found DBCS character
//                    a SI is inserted even when it is the last character
//                    in the input buffer. The function should only be called
//                    if a  DBCS codepage is active. If the function is called
//                    and no DBCS code page is active, the input buffer is
//                    copied to the output buffer with no insert/delete
//                    of SO/SI and the size of the output buffer is returned.
//------------------------------------------------------------------------------
// Input parameter:   The function prototype, the definitions of types for
//                    parameter usConvert are available in EQFxxxxx.H.
//                    Use EQFxxxxx.LIB for linking.
//                    The runtime function is available from EQFxxxxx.DLL
//
//  pszInBuffer must not start with the second byte of a DBCS character.
//  If the last character of pszInBuffer is a DBCS character, the complete
//  DBCS character (first and second byte) must be passed.
//
//  when called with usConvert == DELETE_SOSI
//    pszOutBuffer    Pointer to output buffer where the
//                    converted string should be copied to.
//    pszInBuffer     Pointer to mixed string to be converted.
//                    Must be NULL terminated !!
//    usConvert       DELETE_SOSI, SO/SI characters should be deleted
//    Note: pusSizeOutBuffer is not needed because SO/SI are deleted, so the
//          outputbuffer can not overflow.
//
//  when called with usConvert == INSERT_SOSI
//    pszOutBuffer      Pointer to output buffer where the
//                      converted string should be copied to.
//    pusSizeOutBuffer  Size of output buffer, needed to check if the output
//                      buffer gets full, because SO/SI are inserted.
//    pszInBuffer       Pointer to mixed string to be converted.
//                      Must be NULL terminated !!
//    usConvert         INSERT_SOSI, SO/SI characters should be deleted
//------------------------------------------------------------------------------
// Output parameter:
//  when called with usConvert == DELETE_SOSI !! INSERT_SOSI
//    pszOutBuffer      Contains the converted string (NULL terminated)
//    pusSizeOutBuffer  Size of output buffer after conversion, including
//                      terminating NULL character. Contains if the
//                      outputbuffer gets full.
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   No error, string converted correctly.
//                    FALSE  Function was called with wrong value for
//                           parameter usConvert.
//                           The output buffer gets full, pusSizeOutBuffer
//                           returns 0;
//------------------------------------------------------------------------------
// Prerequesits:
//  The function should only be called when a DBCS codepage is active.
//  use initdbcs() and the global variable _dbcs_cp to check this.
//  If the function is called even when  no DBCS code page is active,
//  the input buffer is copied to the output buffer with no insert/delete
//  of SO/SI and the size of the output buffer is returned.
//
//  pszInBuffer must be a NULL terminated string.
//
//  when called with usConvert == INSERT_SOSI
//    The first byte of the input buffer must not be the second byte of a
//    DBCS character. The last DBCS character must be completely (first and
//    second byte) contained in the input buffer.
//------------------------------------------------------------------------------
// Samples:
//  #include "eqf.h"
//  #include "eqfxxxxx.h"
//  #include <dbcs.h>
//  #define BUFFER_SIZE 1024
//
//  VOID main ( INT argc, CHAR *argv[] )
//  {
//    CHAR     szIn[BUFFER_SIZE];
//    CHAR     szOut[3*BUFFER_SIZE];
//    HFILE    hf1;
//    HFILE    hf2;
//    USHORT   usActionTaken1, usActionTaken2, usOpenAction1, usOpenAction2;
//    USHORT   usOpenMode1, usOpenMode2;
//    USHORT   usBytesRead, usBytesWritten;
//    USHORT   usSizeOut;
//    USHORT   usConvert = 0;
//    BOOL     fRc = 0;
//
//    initdbcs();
//
//    if ( argv[2][0] == 'I' ) usConvert = INSERT_SOSI;
//    if ( argv[2][0] == 'D' ) usConvert = DELETE_SOSI;
//
//    usOpenAction1 = OPEN_ACTION_OPEN_IF_EXISTS |
//                    OPEN_ACTION_FAIL_IF_NEW;
//    usOpenAction2 = OPEN_ACTION_REPLACE_IF_EXISTS |
//                    OPEN_ACTION_CREATE_IF_NEW;
//    usOpenMode1   = OPEN_ACCESS_READONLY |
//                    OPEN_SHARE_DENYWRITE |
//                    OPEN_FLAGS_FAIL_ON_ERROR;
//    usOpenMode2   = OPEN_ACCESS_READWRITE |
//                    OPEN_SHARE_DENYREADWRITE |
//                    OPEN_FLAGS_FAIL_ON_ERROR;
//
//    usRc = DosOpen( argv[1], &hf1, &usActionTaken1, 0L,
//                    FILE_NORMAL, usOpenAction1, usOpenMode1, 0L);
//
//    usRc = DosOpen( "test.out", &hf2, &usActionTaken2, 0L,
//                     FILE_NORMAL, usOpenAction2, usOpenMode2, 0L);
//
//    DosRead( hf1, szIn, sizeof(szIn), &usBytesRead );
//
//    strcat( szIn, "\0" );
//    usSizeOut = sizeof(szOut);
//
//    if ( _dbcs_cp == DBCS_CP )
//    {
//      fRc = UtlConvertSOSI( szOut, &usSizeOut, szIn, usConvert );
//      if ( !fRc )
//      {
//        // do error handling here
//      }
//      DosWrite( hf2, szOut, usSizeOut, &usBytesWritten );
//    }
//    else
//    {
//      DosWrite( hf2, szIn, sizeof(szIn), &usBytesWritten );
//    } /* endif */
//    DosClose( hf1 );
//    DosClose( hf2 );
//  } /* end main */
//------------------------------------------------------------------------------
// Function flow:
//   if ( DBCS codepage is active )
//   {
//     set size of output buffer to 0
//
//     switch ( convert type )
//     {
//       case ( delete all SO and SI in input buffer )
//
//         loop through input buffer
//         while (  no end of string found )
//         {
//           switch ( current character from input buffer )
//           {
//             case ( current character is SO ) :
//             case ( current character is SI ) :
//               break; == skip this charcter, do not copy it to output buffer
//             default : //--- current character is no SO and no SI
//               copy current char. to output buffer
//               usSizeOutBuffer++;
//               break;
//           } /* endswitch */
//         } /* endwhile */
//
//         xopy last char. (==EOS) to output buffer
//         increase size of output buffer
//         return size of output buffer
//         break;
//       case ( insert SO/SI before/after dbcs character sequence ):
//         maximal output buffer size is output buffersize - SO - SI - EOS == -
//
//         loop through input buffer
//         while ( not EOS and output buffer < maximal output buffer size )
//         {
//            if ( if current character is a SBCS character )
//            {
//              if ( SO was already inserted )
//              {
//                copy SI to, and increase output buffer
//                increase size of output buffer
//                reset SO inserted flag
//              } /* endif */
//
//              copy current character to output buffer and increase
//              increase size of output buffer
//            }
//            else //--- current character is a double byte character
//            {
//              //--- no SO was inserted to output buffer until now
//              //--- this is start if a DBCS character sequence
//              if ( no SO was inserted to output buffer until now )
//              {
//                start of a DBCS character sequence is found
//                copy SO to output buffer and increase
//                increase output buffer size
//              } /* endif */
//
//              copy first byte of current dbcs character to output buffer
//              increase output buffer size
//
//              copy second byte of current dbcs character to output buffer
//              increase output buffer size
//            } /* endif */
//         } /* endwhile */
//
//         if ( output buffer < maximal output buffer size )
//         {
//            //--- if SO was inserted and the end of the input is reached,
//            //--- SI has to be inserted at the end of the DBCS sequence,
//            if ( SO was already inserted )
//            {
//              insert SI at the end of the DBCS character sequence
//            } /* endif */
//
//            xopy last char. (==EOS) to output buffer
//            increase output buffer size
//         }
//         else  //--- output buffer was exceeded
//         {
//            reset output buffer size to 0
//            return function failed
//         } /* endif */
//
//         return size of output buffer
//         break;
//       default :     //--- function was called with a wrong convert type
//         return function failed
//         break;
//     } /* endswitch */
//   }
//   else //--- no DBCS codepage is active
//   {
//      copy input buffer to output buffer
//      return size of buffer
//     /* endif */
//   return fRc;
//------------------------------------------------------------------------------
BOOL EQFBUtlConvertSOSI( PSZ_W pszOutBuffer, PULONG pulSizeOutBuffer,
                         PSZ_W pszInBuffer,  USHORT usConvert, ULONG ulOemCodePage )
{
  BOOL     fRc = TRUE;            // functions rc, set to no error
  ULONG    ulSizeOutBuffer;       // returned size of output buffer
  ULONG    ulMaxBufferSize;       // reduced output buffer size
  BOOL     fSOInserted = FALSE;   // indicates if SO is already inserted or not
  CHAR_W   ch;                    // temporary charcter

  if ( IsDBCS_CP(ulOemCodePage) )      // DBCS codepage is active
  {
    ulSizeOutBuffer = 0;          // initialize size of out buffer

    switch ( usConvert )
    {
      //------------------------------------------------------------------------
      case DELETE_SOSI :  //--- delete all SO and SI in input buffer
        //--- loop through input buffer
        while ( (ch = *pszInBuffer++) != NULC )  // while no end of string found
        {
          switch ( ch )  //--- current character from input buffer
          {
            //-----------------------------------------------------------------
            case SO : //--- current character is SO or ...
            case SI : //--- current character is SI
              break;  //--- skip this charcter, do not copy it to output buffer
            //-----------------------------------------------------------------
            default : //--- current character is no SO and no SI
              *pszOutBuffer++ = ch;   //--- copy current char. to output buffer
              ulSizeOutBuffer++;      //--- increase size of output buffer
              break;
          } /* endswitch */
        } /* endwhile */

        *pszOutBuffer = ch;      //--- xopy last char. (==EOS) to output buffer
        ulSizeOutBuffer++;       //--- increase size of output buffer

        //--- return size of output buffer
        *pulSizeOutBuffer = ulSizeOutBuffer;

        break;
      //-------------------------------------------------------------------------
      case INSERT_SOSI :  //--- insert SO/SI before/after dbcs character sequence
        //--- maximal output buffer size is output buffersize - SO - SI - EOS
        //--- ==> -3
        ulMaxBufferSize =  *pulSizeOutBuffer - 3;

        //--- loop through input buffer
        while ( ((ch = *pszInBuffer++)!= NULC)
                 && (ulSizeOutBuffer < ulMaxBufferSize) )
        {
           switch ( ch )
           {
             case SO : //--- current character is SO or ...
             case SI : //--- current character is SI
               break;  //--- skip this charcter, do not copy it to output buffer
             default :
               //--- if current character is a SBCS character
               if (!EQFIsDBCSChar(ch, ulOemCodePage ) )
               {
                 //--- if TRUE, found SBCS character indicates that this is the end
                 //--- of a DBCS character sequence, SO was already inserted
                 //--- and SI have to be inserted
                 if ( fSOInserted )
                 {
                   //--- copy SI to, and increase output buffer
                   *pszOutBuffer++ = SI;
                   //--- increase size of output buffer
                   ulSizeOutBuffer++;

                   //--- end of DBCS character sequence found, reset SO inserted flag
                   fSOInserted = FALSE;
                 } /* endif */

                 //--- copy current character to output buffer and increase
                 *pszOutBuffer++ = ch;
                 //--- increase size of output buffer
                 ulSizeOutBuffer++;
               }
               else //--- current character is a double byte character
               {
                 //--- no SO was inserted to output buffer until now
                 //--- this is start if a DBCS character sequence
                 if ( !fSOInserted )
                 {
                   //--- start of a DBCS character sequence is found
                   fSOInserted = TRUE;
                   //--- copy SO to output buffer and increase
                   *pszOutBuffer++ = SO;
                   //--- increase output buffer size
                   ulSizeOutBuffer++;
                 } /* endif */

                 //--- copy dbcs character to output buffer
                 *pszOutBuffer++ = ch;
                 //--- increase output buffer size
                 ulSizeOutBuffer++;

               } /* endif */
               break;
           } /* endswitch */
        } /* endwhile */

        if ( ulSizeOutBuffer < ulMaxBufferSize  )
        {
           //--- if SO was inserted and the end of the input is reached,
           //--- SI has to be inserted at the end of the DBCS sequence,
           if ( fSOInserted )
           {
             //--- insert SI at the end of the DBCS character sequence
             *pszOutBuffer++ = SI;
             //--- increase output buffer size
             ulSizeOutBuffer++;
           } /* endif */

           //--- EOS was found
          *pszOutBuffer = ch;  //--- xopy last char. (==EOS) to output buffer
           //--- increase output buffer size
           ulSizeOutBuffer++;
        }
        else  //--- output buffer was exceeded
        {
           ulSizeOutBuffer = 0;     //--- reset output buffer size
           fRc = FALSE;             //--- return function failed
        } /* endif */

        //--- return size of output buffer
        *pulSizeOutBuffer = ulSizeOutBuffer;
        break;
      //------------------------------------------------------------------------
      default :        //--- function was called with a wrong convert type
        fRc = FALSE;   //--- return function failed
        break;
    } /* endswitch */
  }
  else //--- no DBCS codepage is active
  {
    //--- copy input buffer to output buffer
    UTF16strcpy( pszOutBuffer, pszInBuffer );
    //--- return size of buffer
     *pulSizeOutBuffer = (UTF16strlenCHAR( pszOutBuffer ) + 1);
  } /* endif */
  return fRc;
} /* end of function EQFBUtlConvertSOSI */

PTBSEGMENT EQFBGetSeg
(
   PTBDOCUMENT pDoc,                    // ptr to active document
   ULONG       ulSeg                    // number of requested segment
)
{
  PTBSEGMENT pSeg;
  ULONG      ulLen = 0L;
  BOOL       fResult = TRUE;

  pSeg = EQFBGetSegW( pDoc, ulSeg);
  if ( pSeg )
  {
    if ( pSeg->pData )
    {
        UtlAlloc((PVOID *) &pSeg->pData, 0L, 0L, NOMSG );
    } /* endif */

    if (!pDoc->pSegmentBuffer )
    {
      fResult = UtlAlloc((PVOID *) &(pDoc->pSegmentBuffer),
                           0L, (LONG)(MAX_SEGMENT_SIZE + 1) * sizeof(CHAR),
                            ERROR_STORAGE);
    } /* endif */
    if (fResult )
    {
      Unicode2ASCII( pSeg->pDataW, pDoc->pSegmentBuffer, pDoc->ulOemCodePage );
      ulLen = strlen(pDoc->pSegmentBuffer);
      ulLen = max( (ulLen+1) * sizeof(CHAR), MIN_ALLOC );

      if ( UtlAlloc( (PVOID *) &pSeg->pData, 0L,
                   (LONG) ulLen, ERROR_STORAGE ) )
      {
         strcpy( pSeg->pData, pDoc->pSegmentBuffer );
      }
    }
  }
  return( pSeg);
} /* end of EQFBGetSeg */

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBGetSeg
//-----------------------------------------------------------------------------
// Function call:     EQFBGetSeg(PTBDOCUMENT,USHORT)
//-----------------------------------------------------------------------------
// Description:       return a pointer to the segment number'usSeg'
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc  ptr to doc structure
//                    USHORT      usSeg segment number of target segment
//-----------------------------------------------------------------------------
// Returncode type:   PTBSEGMENT
//-----------------------------------------------------------------------------
// Returncodes:       pSeg      - pointer to segment entry, Null if error
//-----------------------------------------------------------------------------
// Prerequesits:      the document structure must contain the segment table
//-----------------------------------------------------------------------------
// Function flow:     if segment number >0 and < max.number
//                      loop through segment table until pointer is found
//-----------------------------------------------------------------------------
PTBSEGMENT EQFBGetSegW
(
   PTBDOCUMENT pDoc,                    // ptr to active document
   ULONG       ulSeg                    // number of requested segment
)
{
   return( EQFBGetSegEx(pDoc, ulSeg, STANDARDTABLE) );
} /* end of EQFBGetSegW */

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBGetSegEx
//-----------------------------------------------------------------------------
// Function call:     EQFBGetSegEx(PTBDOCUMENT,USHORT, USHORT)
//-----------------------------------------------------------------------------
// Description:       return a pointer to the segment number'usSeg'
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc  ptr to doc structure
//                    USHORT      usSeg segment number of target segment
//                    USHORT      STANDARDTABLE or ADDITIONALTABLE
//-----------------------------------------------------------------------------
// Returncode type:   PTBSEGMENT
//-----------------------------------------------------------------------------
// Returncodes:       pSeg      - pointer to segment entry, Null if error
//-----------------------------------------------------------------------------
// Prerequesits:      the document structure must contain the segment table
//-----------------------------------------------------------------------------
// Function flow:     if segment number >0 and < max.number
//                      loop through segment table until pointer is found
//-----------------------------------------------------------------------------
PTBSEGMENT EQFBGetSegEx
(
   PTBDOCUMENT pDoc,                    // ptr to active document
   ULONG       ulSeg,                   // number of requested segment
   USHORT      usTypeOfTable            // type of table where to pick up seg
)
{
   PTBSEGMENTTABLE  pSegTable = NULL;      // ptr to segment table
   PTBSEGMENT       pSeg = NULL;           // ptr to segment
   ULONG            ulSegTables = 0;       // no of segment tables to process

   if ( (ulSeg > 0) &&
        ( (ulSeg < pDoc->ulMaxSeg) || (usTypeOfTable == ADDITIONALTABLE)) )
   {
     if (usTypeOfTable == STANDARDTABLE )
     {
       pSegTable = pDoc->pSegTables;
       ulSegTables =  pDoc->ulSegTables;
     }
     else
     {
       pSegTable = pDoc->pAddSegTables;
       ulSegTables =  pDoc->ulAddSegTables;
     } /* endif */

      while ( ulSegTables && ( ulSeg >= pSegTable->ulSegments) )
      {
         ulSeg -= pSegTable->ulSegments;
         ulSegTables--;
         pSegTable++;
      } /* endwhile */
      if ( ulSegTables && ( ulSeg <  pSegTable->ulSegments) )
      {
         pSeg = pSegTable->pSegments + ulSeg;
      } /* endif */
   } /* endif */

   return( pSeg );
} /* end of EQFBGetSegEx */


/**********************************************************************/
/* delete character at previous data position                         */
/**********************************************************************/

  #define SEGSOFTDELETE(p)                    \
  {                                           \
     PSZ_W pSrc = p;                          \
     PSZ_W pTgt = p-1;                        \
     while (*pSrc)                            \
     {                                        \
       *pTgt ++ = *pSrc++;                    \
     }                                        \
     *pTgt = EOS;                             \
  }                                           \

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBBufRemoveSoftLF
//------------------------------------------------------------------------------
// Function call:     EQFBBufRemoveSoftLF(HWND, PCHAR, PUSHORT, PUSHORT)
//------------------------------------------------------------------------------
// Description:       removes soft lf'S in the given data
//------------------------------------------------------------------------------
// Parameters:        HWND    hwndRichEdit
//                    PCHAR  pchData
//                    PUSHORT pusBufSize    size of returned string
//                    PUSHORT pusSegOffset  cursor offset
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     while not at end of string
//                      if current char is SOFTLF_CHAR
//                         if it is a double SOFTLF_CHAR:
//                            delete the duplicate
//                         else
//                           if at end of segment: delete SOFTLF_CHAR
//                           if in DBCS in front of a DBCS character:
//                             delete SOFTLF_CHAR
//                           else
//                             replace SOFTLF_CHAR by BLANK
//                      endwhile
//                      return resulting length
//------------------------------------------------------------------------------
VOID  EQFBBufRemoveSoftLF
(
   HWND        hwndRichEdit,
   PCHAR_W     pchData,
   PUSHORT     pusBufSize,
   PUSHORT     pusSegOffOld
)
{
  CHAR_W    ch1st;
  CHAR_W    ch2nd;
  PCHAR_W   pData;
  USHORT    usCurOffset = 0;

  if (!hwndRichEdit )
  {
    pData = pchData;
    while (*pData != EOS )
    {
      ch1st = *pData;
      if (ch1st == SOFTLF_CHAR )
      {
        ch2nd = *(pData+1);
        if ((ch2nd != EOS ) && (ch2nd == SOFTLF_CHAR ) )
        {
          SEGSOFTDELETE(pData+1);        //delete the 2nd one of double softlfs
          if (usCurOffset < *pusSegOffOld )
          {
            (*pusSegOffOld)--;
          } /* endif */
        }
        else
        {
          /***********************************************************/
          /* check whether SOFTLF has been inserted ( must be deleted*/
          /* now ) or must be replaced by a blank now                */
          /***********************************************************/
          if (ch2nd == EOS  )
          {
            *pData = EOS;  // at end of string, always delete SoftLF
          }
          else
          {
            SEGSOFTDELETE(pData+1);         // del. softLF at pos. pData
            if (usCurOffset < *pusSegOffOld )
            {
              (*pusSegOffOld)--;
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
      pData++;
      usCurOffset++;
    } /* endwhile */
    *pusBufSize = (USHORT)UTF16strlenCHAR(pchData);
  }
  else
  {
    *pusBufSize = (USHORT)UTF16strlenCHAR(pchData);
  } /* endif */
  return;
} /* end of function EQFBBufRemoveSoftLF */

// helper function to compute PE data offsets
VOID PEDataSetDataOffset( PSZ_W pszData, PULONG pulDataLen, PULONG pulDataOffs, PULONG pulCurOffs )
{
  if ( *pszData != 0 ) 
  {
    *pulDataOffs = *pulCurOffs;
    *pulDataLen = ((wcslen(pszData) + 1) * sizeof(CHAR_W));
    *pulCurOffs = *pulCurOffs  + *pulDataLen;
  } /* endif */
}

VOID WriteToMTLog( FILE *hLog, PACTSEGLOGENH2 pSegLog, PSZ_W pszSegSource, PSZ pszPropSourceDoc, PSZ_W pszMetaData, PPEDATABUFFER pPEData )
{
  if ( hLog )
  {
    PESEGLOGDATA PEData;                       

    // fill fixed length part of record
    pSegLog->ulEyeCatcher = ACTSEGLOG2_EYECATCHER;
    pSegLog->ulSegLen = 0;
    pSegLog->ulPropSourceLen = 0;
    pSegLog->ulMetaDataLen = 0;
    pSegLog->ulPEDataLen = 0;

    // prepare PE data area if PE data is available
    memset( &PEData, 0, sizeof(PEData) );
    if ( pPEData != NULL )
    {
      ULONG ulCurOffs = sizeof(PEData);

      if ( (pPEData->szCopiedMatchID[0] != 0) || (pPEData->szCopiedTarget[0] != 0) || (pPEData->szMTMatchID[0] != 0) ||
           (pPEData->szMTTarget[0] != 0) || (pPEData->szTarget[0] != 0) )
      {
        ulCurOffs = sizeof(PEData);
      } /* endif */

      PEDataSetDataOffset( pPEData->szTarget, &(PEData.ulTargetLen), &(PEData.ulTargetOffs), &ulCurOffs );
      PEDataSetDataOffset( pPEData->szMTTarget, &(PEData.ulMTTargetLen), &(PEData.ulMTTargetOffs), &ulCurOffs );
      PEDataSetDataOffset( pPEData->szMTMatchID, &(PEData.ulMTMatchIDLen), &(PEData.ulMTMatchIDOffs), &ulCurOffs );
      PEDataSetDataOffset( pPEData->szCopiedTarget, &(PEData.ulCopiedTargetLen), &(PEData.ulCopiedTargetOffs), &ulCurOffs );
      PEDataSetDataOffset( pPEData->szCopiedMatchID, &(PEData.ulCopiedMatchIDLen), &(PEData.ulCopiedMatchIDOffs), &ulCurOffs );

      if ( ulCurOffs == sizeof(PEData) )
      {
        // no PE data is available
        PEData.ulSize = 0;
        pSegLog->ulPEDataLen = 0;
      }
      else
      {
        pSegLog->ulPEDataLen = PEData.ulSize = ulCurOffs;
      }
    } /* endif */

    if ( (pszSegSource != NULL) && (*pszSegSource != 0) )
    {
      pSegLog->ulSegLen = (wcslen(pszSegSource) + 1) * sizeof(CHAR_W);
    } /* endif */       
    if ( (pszPropSourceDoc != NULL) && (*pszPropSourceDoc != 0) )
    {
      pSegLog->ulPropSourceLen = strlen(pszPropSourceDoc) + 1;
    } /* endif */       
    if ( (pszMetaData != NULL) && (*pszMetaData != 0) )
    {
      pSegLog->ulMetaDataLen = (wcslen(pszMetaData) + 1) * sizeof(CHAR_W);
    } /* endif */       
    pSegLog->ulRecordLen = sizeof(ACTSEGLOGENH2) + pSegLog->ulSegLen + pSegLog->ulPropSourceLen + pSegLog->ulMetaDataLen + pSegLog->ulPEDataLen;

    UtlTime( (LONG *)&(pSegLog->ulTimeStamp) ); 

    // proposal flags consistency check (use the copied flag as correct value)
    if ( pSegLog->PropTypeCopied.Exact && !pSegLog->PropTypeExists.Exact ) pSegLog->PropTypeExists.Exact = TRUE;
    if ( pSegLog->PropTypeCopied.Fuzzy && !pSegLog->PropTypeExists.Fuzzy ) pSegLog->PropTypeExists.Fuzzy = TRUE;
    if ( pSegLog->PropTypeCopied.Replace && !pSegLog->PropTypeExists.Replace ) pSegLog->PropTypeExists.Replace = TRUE;
    if ( pSegLog->PropTypeCopied.MT && !pSegLog->PropTypeExists.MT ) pSegLog->PropTypeExists.MT = TRUE;
    if ( pSegLog->PropTypeCopied.GlobMem && !pSegLog->PropTypeExists.GlobMem ) pSegLog->PropTypeExists.GlobMem = TRUE;
    if ( pSegLog->PropTypeCopied.GlobMemFuzzy && !pSegLog->PropTypeExists.GlobMemFuzzy ) pSegLog->PropTypeExists.GlobMemFuzzy = TRUE;

    // compute checksum
    {
      PBYTE pbTest;
      int i, iLen;

      // checksum of fixed part
      pSegLog->ulCheckSum = 0;
      pbTest = (PBYTE)&(pSegLog->ulRecordLen);
      iLen = sizeof(ACTSEGLOGENH2) - sizeof(pSegLog->ulEyeCatcher) - sizeof(pSegLog->ulCheckSum);
      i = 0;
      while ( i < iLen ) pSegLog->ulCheckSum = pSegLog->ulCheckSum + (ULONG)pbTest[i++];

      // checksum of segment source text
      if ( pSegLog->ulSegLen != 0 ) 
      {
        pbTest = (PBYTE)pszSegSource;
        iLen = (int)pSegLog->ulSegLen;
        i = 0;
        while ( i < iLen) pSegLog->ulCheckSum = pSegLog->ulCheckSum + (ULONG)pbTest[i++];
      } /* endif */         

      // checksum of proposal source document name
      if ( pSegLog->ulPropSourceLen != 0 ) 
      {
        pbTest = (PBYTE)pszPropSourceDoc;
        iLen = pSegLog->ulPropSourceLen;
        i = 0;
        while ( i < iLen) pSegLog->ulCheckSum = pSegLog->ulCheckSum + (ULONG)pbTest[i++];
      } /* endif */       

      // checksum of proposal meta data name
      if ( pSegLog->ulMetaDataLen != 0 ) 
      {
        pbTest = (PBYTE)pszMetaData;
        iLen = pSegLog->ulMetaDataLen;
        i = 0;
        while ( i < iLen) pSegLog->ulCheckSum = pSegLog->ulCheckSum + (ULONG)pbTest[i++];
      } /* endif */         
    }

    // write fixed part of record
    fwrite( pSegLog, sizeof(ACTSEGLOGENH2), 1, hLog);

    // write any segment source
    if ( pSegLog->ulSegLen != 0 ) fwrite( pszSegSource, pSegLog->ulSegLen, 1, hLog);

    // write any proposal document name
    if ( pSegLog->ulPropSourceLen != 0 ) fwrite( pszPropSourceDoc, pSegLog->ulPropSourceLen, 1, hLog);

    // write any proposal meta data
    if ( pSegLog->ulMetaDataLen != 0 ) fwrite( pszMetaData, pSegLog->ulMetaDataLen, 1, hLog);

    // write any PE data
    if ( pSegLog->ulPEDataLen != 0 ) 
    {
      fwrite( &PEData, sizeof(PEData), 1, hLog);
      if ( PEData.ulTargetLen != 0 ) fwrite( pPEData->szTarget, PEData.ulTargetLen, 1, hLog );
      if ( PEData.ulMTTargetLen != 0 ) fwrite( pPEData->szMTTarget, PEData.ulMTTargetLen, 1, hLog );
      if ( PEData.ulMTMatchIDLen != 0 ) fwrite( pPEData->szMTMatchID, PEData.ulMTMatchIDLen, 1, hLog );
      if ( PEData.ulCopiedTargetLen != 0 ) fwrite( pPEData->szCopiedTarget, PEData.ulCopiedTargetLen, 1, hLog );
      if ( PEData.ulCopiedMatchIDLen != 0 ) fwrite( pPEData->szCopiedMatchID, PEData.ulCopiedMatchIDLen, 1, hLog );
    } /* endif */
  } /* endif */     

}/* end of function WriteToMTLog */

PTBSEGMENT SegFileGetVisSeg
(
   PTBDOCUMENT pDoc,                      // pointer to Document ida
   PULONG      pulSegNum                  // pointer to segment number
)
{
   PTBSEGMENT pSeg;                       // pointer to segment
   SHORT sSign = 1;

   pSeg = EQFBGetSegW( pDoc,*pulSegNum );
   while (pSeg && pSeg->SegFlags.Joined)
   {
      (*pulSegNum)++;
      pSeg = EQFBGetSegW( pDoc, *pulSegNum );
   } /* endwhile */

   return( pSeg );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBCntAllSrcWords
//------------------------------------------------------------------------------
// Function call:     EQFBCntALlSrcWords ( PTBDOCUMENT )
//------------------------------------------------------------------------------
// Description:       this function will count all src words
//                    this is nec for the new word count utility
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Function flow:     loop thru all segments and cnt the words
//------------------------------------------------------------------------------
SHORT EQFBCntAllSrcWords
(
  PTBDOCUMENT  pSourceDoc,              // pointer to source document
  PTBDOCUMENT  pTargetDoc,              // pointer to target document
  SHORT        sSourceLang,             // language ID for source language
  ULONG        ulOemCP
)
{
   ULONG       ulSegNum;
   SHORT       sRc = 0;
   PTBSEGMENT  pSourceSeg, pTargetSeg;                     // pointer to segment
   ULONG       ulSrcWords = 0L;
   ULONG       ulSrcMarkUp = 0L;
   ULONG       ulTempSeg = 0; 

   /*****************************************************************/
   /* force count of all source words                               */
   /*****************************************************************/
   ulTempSeg = ulSegNum = 1;
   pSourceSeg = SegFileGetVisSeg( pSourceDoc, &ulTempSeg );
   pTargetSeg = SegFileGetVisSeg( pTargetDoc, &ulSegNum);
   while ( pTargetSeg && !sRc )
   {
     switch ( pTargetSeg->qStatus )
     {
       case  QF_ATTR:
       case  QF_TOBE:
       case  QF_CURRENT:
         if ((pTargetSeg->usSrcWords == 0 )&& (!pTargetSeg->SegFlags.NoCount))
         {
           ulSrcWords = 0L;
           ulSrcMarkUp = 0L;
           sRc = EQFBWordCntPerSeg(
                        (PLOADEDTABLE)pTargetDoc->pDocTagTable,
                        (PTOKENENTRY) pTargetDoc->pTokBuf,
                        pSourceSeg->pDataW,
                        sSourceLang,
                        &ulSrcWords, &ulSrcMarkUp, ulOemCP );
           if (!sRc)
           {
             pTargetSeg->usSrcWords = (USHORT) ulSrcWords;
           } /* endif */
         }
         else
         {
           pTargetSeg->usSrcWords = 0;
         } /* endif */
         break;

       case  QF_NOP:                             // segment not counted
       case  QF_XLATED:
       default :
         break;
     } /* endswitch */
     ulSegNum ++;
     ulTempSeg = ulSegNum;
     pSourceSeg = SegFileGetVisSeg( pSourceDoc, &ulTempSeg );
     pTargetSeg = SegFileGetVisSeg( pTargetDoc, &ulSegNum);
   } /* endwhile */

   return ( sRc );
}

SHORT EQFBIsNumLFEqual
(
	PSZ_W pData1,
	PSZ_W pData2,
	PSHORT psNumLF1,
	PSHORT psNumLF2
)
{
	PSZ_W  pTemp;
	USHORT usChar;
	SHORT  sLines = 0;

	//if reflow not allowe copy only if #of lines equal
										  //count # of lines in source
	   pTemp = pData1;                    //which = # lines in workseg
	   while ( (usChar = *pTemp++) != EOS )
	   {
		  if (usChar == LF)
		  {
			 sLines++;                   // #of lines in source
		  } /* endif */
	   } /* endwhile */
       if (psNumLF1)
       {
         *psNumLF1 = sLines;
       }

	   pTemp = pData2;                     // is # equal?
	   while ((usChar = *pTemp++) != EOS && (sLines >= 0) )
	   {
		  if (usChar == LF)
		  {
			 sLines--;
		  } /* endif */
	   } /* endwhile */

       if (psNumLF2 && psNumLF1)
       {
	     *psNumLF2 = *psNumLF1 - sLines;     // num of LF in Data2
       }

	return (sLines);
}

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBBufRemoveTRNote
//-----------------------------------------------------------------------------
// Function call:     EQFBBufRemoveTRNote
//-----------------------------------------------------------------------------
// Description:       This function will use TACreateProtectTable to detect
//                    and eliminate TRNOtes.
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   pDoc   - ptr to the structure
//                                           describing the active doc
//                    PSZ           pInData- pointer to input data
//                    PSZ           pOutData- pointer to output data
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Prerequesits:      The document tag table must have been loaded.
//                    Error return codes are ignored, i.e. segment will be
//                    not normalized in such cases
//-----------------------------------------------------------------------------
// Function flow:     create start stop table
//                    add character except TRNOTE into output segment
//-----------------------------------------------------------------------------

VOID EQFBBufRemoveTRNote
(
   PSZ_W        pData,                            // ptr to data
   PVOID        pDocTagTable,
   PFN          pfnUserExit,
   PFN          pfnUserExitW,
   ULONG        ulOemCP
)
{
   USHORT      usColPos = 0;           // column pos used by EQFTagTokenize
   PSTARTSTOP  pStartStop = NULL;      // start stop table
   PSTARTSTOP  pstCurrent;             // active start stop indication
   USHORT      usRc;                   // success indicator
   USHORT      usLen;                  // length to be copied
   PSZ_W       pOutData;
   int         iIterations = 0;
   USHORT      usAddEntries = 0;
   PTOKENENTRY pTokenList = NULL;

   pOutData = pData;
   usRc = TACreateProtectTableW( pData,
                                pDocTagTable,
                                usColPos,
                                astTokBuf,
                                NUM_TOKENS * sizeof(TOKENENTRY),
                                &pStartStop,
                                pfnUserExit,
                                pfnUserExitW, ulOemCP);

   /*******************************************************************/
   /* make buffer bigger if nec; donot reuse astTokBuf in this case   */
   /*******************************************************************/
   while ((iIterations < 14) && (usRc == EQFRS_AREA_TOO_SMALL))
   {
     // (re)allocate token buffer
     LONG lOldSize = (usAddEntries) * sizeof(TOKENENTRY);
     LONG lNewSize = (usAddEntries+128) * sizeof(TOKENENTRY);

     if (UtlAlloc((PVOID *) &pTokenList, lOldSize, lNewSize, NOMSG) )
     {
       usAddEntries += 128;
       iIterations++;
     }
     else
     {
       iIterations = 14;    // force end of loop
     } /* endif */
     // retry tokenization
     if (iIterations < 14 )
     {
       usRc = TACreateProtectTableW( pData,
                                    pDocTagTable,
                                    usColPos,
                                    (PTOKENENTRY)pTokenList,
                                    (USHORT)lNewSize,
                                    &pStartStop,
                                    pfnUserExit,
                                    pfnUserExitW, ulOemCP);
     } /* endif */
   } /* endwhile */

   if ( !usRc && pStartStop )
   {
     pstCurrent = pStartStop;
     while ( pstCurrent->usType )
     {
       switch ( pstCurrent->usType )
       {
         case TRNOTE_CHAR:
            /**********************************************************/
            /* ignore TRNotes text                                    */
            /**********************************************************/
           break;
         default:
           usLen = pstCurrent->usStop - pstCurrent->usStart + 1;
           memcpy( pOutData, pData + pstCurrent->usStart, usLen*sizeof(CHAR_W) );
           pOutData += usLen;
           break;
       } /* endswitch */
       pstCurrent++;
     } /* endwhile */
     *pOutData = EOS;
     UtlAlloc((PVOID *) &pStartStop, 0L, 0L, NOMSG );
   }
   else
   {
     UTF16strcpy( pOutData, pData );
   } /* endif */

   if ( pTokenList ) UtlAlloc( (PVOID *) &pTokenList, 0L, 0L, NOMSG );

   return;
} /* end of EQFBBufRemoveTRNote */

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBNormSeg
//-----------------------------------------------------------------------------
// Function call:     EQFBNormSeg(PTBDOCUMENT,PSZ,PSZ)
//-----------------------------------------------------------------------------
// Description:       This function will use EqfTagTokenize to normalize a
//                    segment. This is done in getting rid of all tags.
//                    Inline Tags of length > 1 are changed into 1 Blank
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   pDoc   - ptr to the structure
//                                           describing the active doc
//                    PSZ           pInData- pointer to input data
//                    PSZ           pOutData- pointer to output data
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Prerequesits:      The document tag table must have been loaded.
//                    Error return codes are ignored, i.e. segment will be
//                    not normalized in such cases
//-----------------------------------------------------------------------------
// Function flow:     create start stop table
//                    add any unprotected character into normalized segment..
//-----------------------------------------------------------------------------
                                                             /*   48@KIT0841A */
VOID EQFBNormSeg
(
   PTBDOCUMENT pDoc,                    // ptr to active document
   PSZ_W       pData,                   // ptr to data
   PSZ_W       pOutData                 // output data
)
{
   USHORT      usColPos = 0;           // column pos used by EQFTagTokenize
   PSTARTSTOP  pStartStop = NULL;      // start stop table
   PSTARTSTOP  pstCurrent;             // active start stop indication
   USHORT      usRc;                   // success indicator
   USHORT      usLen;                  // length to be copied
   int         iIterations = 0;
   USHORT      usAddEntries = 0;
   PTOKENENTRY pTokenList = NULL;

   usRc = TACreateProtectTableW( pData,
                                pDoc->pDocTagTable,
                                usColPos,
                                astTokBuf,
                                NUM_TOKENS * sizeof(TOKENENTRY),
                                &pStartStop,
                                pDoc->pfnUserExit,
                                pDoc->pfnUserExitW, pDoc->ulOemCodePage);
   /*******************************************************************/
   /* make buffer bigger if nec; donot reuse astTokBuf in this case   */
   /*******************************************************************/
   while ((iIterations < 14) && (usRc == EQFRS_AREA_TOO_SMALL))
   {
     // (re)allocate token buffer
     LONG lOldSize = (usAddEntries) * sizeof(TOKENENTRY);
     LONG lNewSize = ((usAddEntries+128) * sizeof(TOKENENTRY));

     if (UtlAlloc((PVOID *) &pTokenList, lOldSize, lNewSize, NOMSG) )
     {
       usAddEntries += 128;
       iIterations++;
     }
     else
     {
       iIterations = 14;    // force end of loop
     } /* endif */
     // retry tokenization
     if (iIterations < 14 )
     {
       usRc = TACreateProtectTableW( pData,
                                    pDoc->pDocTagTable,
                                    usColPos,
                                    (PTOKENENTRY)pTokenList,
                                    (USHORT)lNewSize,
                                    &pStartStop,
                                    pDoc->pfnUserExit,
                                    pDoc->pfnUserExitW, pDoc->ulOemCodePage);
     } /* endif */

   } /* endwhile */


   if ( !usRc && pStartStop )
   {
     pstCurrent = pStartStop;
     while ( pstCurrent->usType )
     {
       switch ( pstCurrent->usType )
       {
         case  UNPROTECTED_CHAR:
           usLen = pstCurrent->usStop - pstCurrent->usStart + 1;
           memcpy( pOutData, pData + pstCurrent->usStart, usLen * sizeof(CHAR_W) );
           pOutData += usLen;
           break;
         default :
           *pOutData++ = BLANK;
           break;
       } /* endswitch */
       pstCurrent++;
     } /* endwhile */
     *pOutData = EOS;
     UtlAlloc((PVOID *) &pStartStop, 0L, 0L, NOMSG );
   }
   else
   {
     UTF16strcpy( pOutData, pData );
   } /* endif */

   if ( pTokenList ) UtlAlloc( (PVOID *) &pTokenList, 0L, 0L, NOMSG );

   return;
} /* end of EQFBNormSeg */

