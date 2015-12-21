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


// activate to debug EQFBHISTDOCSAVE function
#ifdef _DEBUG
  // #define HISTDOCSAVELOG
  //#define WORDCOUNTLOG
#endif


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
USHORT EQFBFilePrint ( PTBDOCUMENT  pDoc );
int    EQFBTagCompare( const void *, const void * );
static SHORT CountAttribs( PLOADEDTABLE, PSZ_W, SHORT, PULONG, PULONG, ULONG );/* @KIT0885A */
static USHORT EQFBSumUpWords(PTBSEGMENT, PCRITERIASUM);
static USHORT EQFBCheckCompactLF (PTBDOCUMENT,PTBSEGMENT,USHORT);
static USHORT EQFBCheckInAbbrev ( PTBSEGMENT, PSTARTSTOP, USHORT );
static VOID EQFBCompactPlusOne( PTBSEGMENT, PSTARTSTOP,
                                  USHORT, PUSHORT );
static VOID  EQFBWCntClearInfo(PTBDOCUMENT pDoc, PSZ pszFileName, PSZ pszSegmentList );
static SHORT  EQFBCheckAndAddSeg( PTBDOCUMENT, PTBSEGMENT,
                                  PUSHORT, ULONG, PTBSEGMENT,
                                  PTBSEGMENT, LONG );
static BOOL   EQFBCheckForTable( PTBSEGMENT, ULONG );
static SHORT  EQFBAddSegEx( PTBDOCUMENT, PTBSEGMENT, USHORT, LONG );
static VOID    EQFBReparseStartStop( PSTARTSTOP *, USHORT, SHORT);
static VOID    EQFBReparseHLType( PSTARTSTOP *, USHORT, SHORT, PTBSEGMENT);
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
USHORT AdjustCountInfo( PTBDOCUMENT pDoc, PSZ pszDocObjName );


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
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBFilePrint
//-----------------------------------------------------------------------------
// Function call:     EQFBFilePrint(PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       use the segment table information and print the document
//
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc        - ptr to active doc structure
//-----------------------------------------------------------------------------
// Returncode type:   USHORT usRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_OPENFILE   - error opening file
//                    ERR_WRITEFILE  - error writing file
//                    ERR_NOMEMORY   - memory allocation error
//-----------------------------------------------------------------------------
// Side effects:      calls to the following routines:
//                          UtlPrintOpen, UtlPrintLine, UtlPrintClose
//-----------------------------------------------------------------------------
// Function flow:     - open printer
//                    - get the EQF segment table information
//                    - scan through the segment table and print the text
//                      data of the segments
//                    - close printer
//-----------------------------------------------------------------------------
USHORT EQFBFilePrint
(
   PTBDOCUMENT  pDoc                     // ptr to document control structure
)
{
   HPRINT      hPrint = NULLHANDLE;    // print handle
   ULONG       ulI;                    // general loop index
   BOOL        fOK = TRUE;             // internal OK flag
   PTBSEGMENT  pSeg;                   // ptr to segment
   PSZ_W       pszLineFeed;            // position of next linefeed character
   PSZ_W       pszBuffer;              // pointer to print buffer
   PSZ_W       pszSource;              // pointer into segment data
   PSZ_W       pszTarget;              // pointer into print buffer
   ULONG       ulFilled;               // # of bytes in print buffer
   ULONG       ulLength;               // length of currently processed string
   CHAR_W      chChar;                 // temp character

   // allocate the print line buffer
   fOK = UtlAlloc((PVOID *) &pszBuffer, 0L, (LONG) (MAX_SEGMENT_SIZE+2)* sizeof(CHAR_W), ERROR_STORAGE );
   pszTarget = pszBuffer;              // start at buffer begin
   ulFilled = 0L;                       // nothing in print buffer yet


   // open the printer device context
   if ( fOK )
   {
      CHAR chTitle[ MAX_PATH144 ];
      CHAR chEqfName[ MAX_EQF_PATH ];
      CHAR chText[ 40 ];
      PSZ  pFolder, pFile;
	  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

      switch ( pDoc->docType )
      {
        case SSOURCE_DOC:
        case STARGET_DOC:
          chTitle[ 0 ] = EOS;
          strcpy( chEqfName, pDoc->szDocName );
          pFile = UtlGetFnameFromPath( chEqfName );
          if ( pFile )
          {
            /**********************************************************/
            /* chEQFName = "c:\eqf\sample1.f00\sTarget\"              */
            /**********************************************************/
            pFolder = pFile - 1;
            *pFolder = EOS;
            pFolder = UtlGetFnameFromPath( chEqfName );
            if ( pFolder )
            {
              pFolder -= 5;
              *pFolder = EOS;
              pFolder = UtlGetFnameFromPath( chEqfName );
            } /* endif */
            LOADSTRING( (HAB) UtlQueryULong( QL_HAB ),
                        hResMod, IDS_TB_PRINTFILE_PRTJOB, chText );
            sprintf( chTitle, chText, pFolder, pDoc->szDocLongName );
          } /* endif */
          break;
        case  OTHER_DOC:
          strcpy( chTitle, pDoc->szDocName );
          break;
        case TRNOTE_DOC:
          LOADSTRING( (HAB) UtlQueryULong( QL_HAB ),
                      hResMod, SID_TRNOTE_TITLE, chText );
          break;
        default :
          strcpy( chTitle, pDoc->szDocName );
          break;
      } /* endswitch */

      fOK = UtlPrintOpen( &hPrint, chTitle, NULLHANDLE );

      /****************************************************************/
      /* Print title string as header text ...                        */
      /****************************************************************/
      if ( fOK )
      {
    CHAR_W chTitleW[ 40 ];
    ASCII2Unicode( chTitle, chTitleW, pDoc->ulOemCodePage );
        if ( pDoc->ulOemCodePage == 874L )
        {
            UtlPrintSetAnsiConv( hPrint, FALSE );   // conversion disturbs Thai chars
        }

        fOK = UtlPrintLineW( hPrint, chTitleW );
              UtlPrintLineW( hPrint, EMPTY_STRINGW );
      } /* endif */
   } /* endif */


   // get the EQF segment table information
   ulI = 0;                                                      /* @KAT0028C */

   // scan through the segment table and write text data to the printer
   if ( fOK )
   {
     do {
        // get pointer to current segment
        ulI ++;                                                  /* @KAT0028A */
        pSeg = EQFBGetVisSeg(pDoc, &ulI);                        /* @KAT0028C */

        if ( pSeg && pSeg->pDataW )
        {
          CHAR_W bEnd = 0;
          CHAR_W c;
          CHAR_W chTemp;
           // start at begin of segment data
           pszSource = pSeg->pDataW;

           ulLength = UTF16strlenCHAR( pszSource );
           if (ulLength > pSeg->usLength)
           {
             bEnd = pSeg->pDataW[ pSeg->usLength ];
             pSeg->pDataW[ pSeg->usLength ] = EOS;
           } /* endif */

           // add data up to next linefeed or softlf character to print line
           pszLineFeed = pszSource;
           while ( ((c = *pszLineFeed)!= NULC) && (c != LF) && (c!=SOFTLF_CHAR) )
           {
             pszLineFeed++;
           } /* endwhile */

           while ( *pszLineFeed && fOK )
           {
              chTemp = *pszLineFeed;
              *pszLineFeed = LF;

              pszLineFeed ++;
              chChar = *pszLineFeed;   // store character
              *pszLineFeed = EOS;
              ulLength = UTF16strlenCHAR( pszSource );
              if ( ulLength + ulFilled >= MAX_SEGMENT_SIZE )
              {
                 fOK = UtlPrintLineW( hPrint, pszBuffer );
                 ulFilled = 0;
                 *pszBuffer = EOS;
                 pszTarget = pszBuffer;
              } /* endif */
              if ( fOK  )
              {
                UTF16strcat( pszTarget, pszSource );
                fOK = UtlPrintLineW( hPrint, pszBuffer );
                ulFilled = 0;
                *pszBuffer = EOS;
                pszTarget = pszBuffer;
                *(pszLineFeed-1) = chTemp;
                *pszLineFeed = chChar;        // restore original character
                pszSource = pszLineFeed ;
                while ( ((c = *pszLineFeed) != NULC) && (c != LF) && (c!=SOFTLF_CHAR) )
                {
                  pszLineFeed++;
                } /* endwhile */
              } /* endif */
           } /* endwhile*/

           if ( fOK )
           {
             ulLength = UTF16strlenCHAR( pszSource );
             if ( ulLength + ulFilled >= MAX_SEGMENT_SIZE )
             {
                fOK = UtlPrintLineW( hPrint, pszBuffer );
                ulFilled = 0L;
                *pszBuffer = EOS;
                pszTarget = pszBuffer;
             } /* endif */
             UTF16strcat( pszTarget, pszSource );
             ulFilled += ulLength;
           } /* endif */
           if (bEnd)
           {
             pSeg->pDataW[pSeg->usLength] = bEnd;
             bEnd = EOS;
           } /* endif */
        } /* endif */
     } while ( fOK && (ulI < pDoc->ulMaxSeg) ); /* enddo */
   } /* endif */

   if ( hPrint )         UtlPrintClose( hPrint );
   if ( pszBuffer )
   {
      UtlAlloc((PVOID *) &pszBuffer, 0L, 0L, NOMSG );
   } /* endif */

   return( ( fOK ) ? 0 : ERR_NOMEMORY );
} /* end of EQFBFilePrint */

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

USHORT EQFBCharType
(
   PTBDOCUMENT pDoc,                    // ptr to active document
   PTBSEGMENT  pSeg,                    // number of active segment
   USHORT      usOffs                   // offset in active segment
)
{
   USHORT     usType= UNPROTECTED_CHAR;  // type of active character
   PSTARTSTOP  pstCurrent= NULL;         // ptr to entries of start/stop table
   static USHORT usMsg = ERROR_STORAGE;  // display message the first time
   HWND        hwndTemp;                 // temp. window handle
   PSTEQFGEN   pstEQFGen;                // pointer to generic structure
   DISPSTYLE   DispStyle;                // local display style active
   ULONG       ulInTagAbbrLen;           // length of compact inline tag
   USHORT      usStart;                  // holds pstCurrent->usStart
   USHORT      usRC = NO_ERROR;          // no error
   USEROPT* pEQFBUserOpt = get_EQFBUserOpt();

   /*******************************************************************/
   /* correct local display style if current segment is expanded      */
   /*******************************************************************/
   if (pSeg )
   {
     DispStyle = (pSeg->SegFlags.Expanded) ?
                        DISP_PROTECTED : (pDoc->DispStyle);


     if ( pSeg->pusBPET == NULL )
     {
	   usRC = TAIteratedCreateProtectTableW(pDoc, pSeg, 10);

       // issue a close request to the translation environment window
       if ( pDoc->pstEQFGen && usRC )
       {
         if ( !pDoc->fErrorProcessed )
         {
           UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR );
         } /* endif */
         pDoc->fErrorProcessed = TRUE;       // error is processed
         pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
         hwndTemp = pDoc->hwndFrame;
         UtlDispatch();                       // display error message
         pDoc = ACCESSWNDIDA( hwndTemp, PTBDOCUMENT );
         WinPostMsg( pstEQFGen->hwndTWBS, WM_CLOSE, NULL, NULL );
       } /* endif */
     } /* endif */

     if ( pDoc && !usRC)
     {
       if (pSeg->pusBPET )
       {
          // look for position in start/stop table
          pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
          while ( (pstCurrent->usType != 0) && (usOffs > pstCurrent->usStop) )
          {
             pstCurrent++;
          } /* endwhile */
          if (pstCurrent->usType != 0)
          {
             usType = pstCurrent->usType;
          }
          else
          {
             usType = ENDOFSEG_CHAR;
          } /* endif */
       }
       else
       {
          usType = UNPROTECTED_CHAR;          // use this as default
       } /* endif */
       if (pSeg->usLength <= usOffs )
       {
         usType = ENDOFSEG_CHAR;
       }
       else if ((usType != TRNOTE_CHAR) && (*(pSeg->pDataW+usOffs) == '\n' ) )
       {
         usType = LINEBREAK_CHAR;
         if ( (DispStyle == DISP_COMPACT) && (pSeg->qStatus != QF_NOP) )
         {
           /************************************************************/
           /* check whether linebreak is in a inline tag and whether   */
           /* it is hidden or not                                      */
           /************************************************************/
           usType = EQFBCheckCompactLF(pDoc,pSeg,usOffs);
         } /* endif */
         if (DispStyle == DISP_SHORTEN )
         {
           /*************************************************************/
           /* check whether linebreak is hidden or not                  */
           /*************************************************************/
           usType = EQFBCheckCompactLF(pDoc, pSeg, usOffs);
         } /* endif */
       } /* endif */

       if ( pDoc->fLineWrap && pDoc->fAutoLineWrap
            &&  (usType != ENDOFSEG_CHAR)
            &&  (*(pSeg->pDataW+usOffs) == SOFTLF_CHAR))
       {
         if (*(pSeg->pDataW+usOffs+1) == SOFTLF_CHAR)
         {
           /*************************************************************/
           /* next char is SOFTLF_CHAR too; use usType of BPET          */
           /*************************************************************/
         }
         else if ( usOffs && (*(pSeg->pDataW+usOffs-1) == SOFTLF_CHAR) )
         {
           /*************************************************************/
           /* double SOFTLF : display first one, hide 2nd one           */
           /* ( if AutoLinewrap is ON, and SOFTLF occurrs in the file,  */
           /* it has been duplicated )                                  */
           /*************************************************************/
           usType = HIDDEN_CHAR;                //prev.char is SOFTLF too
         }
         else
         {
           usType = LINEBREAK_CHAR;             // single SOFTLF is softlinefeed
         } /* endif */
       } /* endif */

       //--- correct QF_NOP depending on current display style ---
       if ( DispStyle == DISP_HIDE && (pSeg->qStatus == QF_NOP) )
       {
          switch ( usType )
          {
             case PROTECTED_CHAR:
             case UNPROTECTED_CHAR:
                usType = HIDDEN_CHAR;
                break;
             default:
                break;
          } /* endswitch */
       } /* endif */

       if (DispStyle == DISP_SHORTEN )
       {
         EQFBCompactPlusOne( pSeg, pstCurrent, usOffs, &usType);
       }
       else
       {
         //--- correct usType depending on current display style ---
         if ( usType == PROTECTED_CHAR )
         {
            switch ( DispStyle )
            {
               case DISP_HIDE:
                  usType = HIDDEN_CHAR;
                  break;
                case  DISP_COMPACT:
                  ulInTagAbbrLen = strlen (pEQFBUserOpt->szInTagAbbr);
                  if ( pSeg->qStatus != QF_NOP )       //now inline tag!!
                  {
                    usStart = pstCurrent->usStart;
                    if ( usStart == pstCurrent->usStop)
                    {
            // leave character as it is...
          }
          else
                    if ( (usStart <= usOffs) &&
                         (usOffs <= (usStart + ulInTagAbbrLen-1)) )
                    {

                      if ( (pstCurrent == (PSTARTSTOP) pSeg->pusBPET) ||
                            ((pstCurrent-1)->usType != PROTECTED_CHAR) )
                      {
                        usType = COMPACT_CHAR;
                      }
                      else
                      {
                        usType = HIDDEN_CHAR;
                      } /* endif */
                    }
                    else
                    {
                      usType = HIDDEN_CHAR;
                    } /* endif */
                  } /* endif */
                  break;
      //       case DISP_PROTECTED:
      //          usType = PROTECTED_CHAR;
      //          break;
               case DISP_UNPROTECTED:
                  usType = UNPROTECTED_CHAR;
                  break;
            } /* endswitch */
         } /* endif */
       } /* endif */

       /*****************************************************************/
       /* correct type TRNOTE_CHAR depending on current display style   */
       /*****************************************************************/
       if (usType == TRNOTE_CHAR )
       {
         usType = EQFBCheckInAbbrev( pSeg, pstCurrent, usOffs);
       } /* endif */
     } /* endif */
   }
   else
   {
     usType = UNPROTECTED_CHAR;  // should not occur!
   } /* endif */

   return( usType );
} /* end of EQFBCharType */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBCheckInAbbrev
//-----------------------------------------------------------------------------
// Function call:     EQFBCheckInAbbrev  (PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       check whether current offset is in visible part of trnote
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//                    PTBSEGMENT  pSeg
//                    PSTARTSTOP  pstCurrent
//                    USHORT      usOffs
//-----------------------------------------------------------------------------
// Returncode type:   USHORT
//-----------------------------------------------------------------------------
// Returncodes:       usType     TRNOTE_CHAR  if at begin of  tag
//                               HIDDEN_CHAR  else
//-----------------------------------------------------------------------------
// Prerequesits:      start/stop table exists
//-----------------------------------------------------------------------------
// Function flow:     if at start of trnote: return TRNOTE_CHAR
//                    else
//                      if trnote contains a linebreak: return LINEBREAK_CHAR
//                      else return HIDDEN_CHAR
//-----------------------------------------------------------------------------
static
USHORT EQFBCheckInAbbrev
(
   PTBSEGMENT  pSeg,                    // number of active segment
   PSTARTSTOP  pstCurrent,              // start-stop region of offset
   USHORT      usOffs                   // offset in active segment
)
{
   ULONG      ulLen;
   USHORT     usStart;
   USHORT     usType;
   USEROPT* pEQFBUserOpt = get_EQFBUserOpt();

   ulLen = strlen ( pEQFBUserOpt->chTRNoteAbbr);
   usStart = pstCurrent->usStart;
   if ((usStart <= usOffs) &&
       (usOffs <= (usStart + ulLen - 1))  )
   {
      usType = TRNOTE_CHAR;     // type TRNOTE remains
   }
   else
   {
     if (  (usOffs == pSeg->usLength-1) &&
            ( *(pSeg->pDataW + usOffs ) == '\n') )
     {
       usType = LINEBREAK_CHAR;
     }
     else
     {
       usType = HIDDEN_CHAR;
     } /* endif */
   } /* endif */
   /*******************************************************************/
   /* is it nec to distinguish between inline and NOP segments???     */
   /*******************************************************************/
   return( usType );
} /* end of function EQFBCheckInAbbrev */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBCheckCompactLF
//-----------------------------------------------------------------------------
// Function call:     EQFBCheckCompactLF (PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       check whether current linefeed is the 1st in block
//                    of inline tags
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//-----------------------------------------------------------------------------
// Returncode type:   USHORT
//-----------------------------------------------------------------------------
// Returncodes:       usType     LINEBREAK_CHAR  if 1st LF in tag
//                               HIDDEN_CHAR  else
//-----------------------------------------------------------------------------
// Prerequesits:      start/stop table exists
//-----------------------------------------------------------------------------
// Function flow:     _
//-----------------------------------------------------------------------------
static
USHORT EQFBCheckCompactLF
(
   PTBDOCUMENT pDoc,
   PTBSEGMENT  pSeg,                    // number of active segment
   USHORT      usOffs                   // offset in active segment
)
{
  PSTARTSTOP pstCurrent;                // ptr to entries in start/stop table
  PSTARTSTOP pstTagBlockBegin;          // ptr to entries in start stop table
  BOOL       fLineFeed = FALSE;
  USHORT     usType = LINEBREAK_CHAR;
  PSZ_W      pTemp;                     // ptr to data string in segment
  PSZ_W      pDataOffs;

  pDoc;

   pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
   while ( (pstCurrent->usType != 0)
             && (usOffs > (pstCurrent->usStop + 1)) )
   {
      pstCurrent++;
   } /* endwhile */
   if ( pstCurrent->usType == PROTECTED_CHAR )
   {
     /*****************************************************************/
     /* loop back til begin of block of inline tags                   */
     /*****************************************************************/
     pstTagBlockBegin = pstCurrent;
     while ( pstTagBlockBegin->usType == PROTECTED_CHAR
             && pstTagBlockBegin != (PSTARTSTOP)pSeg->pusBPET )
     {
       pstTagBlockBegin --;
     } /* endwhile */
     if ( pstTagBlockBegin->usType != PROTECTED_CHAR )
     {
       pstTagBlockBegin ++;    //we went 1 too far
     } /* endif */

     pDataOffs = pSeg->pDataW + usOffs;
     pTemp = pSeg->pDataW + (pstTagBlockBegin->usStart);
     /*****************************************************************/
     /* find 1st LF in this block                                     */
     /*****************************************************************/
     while ( !fLineFeed && (pTemp < pDataOffs) )
     {
       fLineFeed = ((*pTemp) == LF );
       pTemp++;
     } /* endwhile */
     if ( fLineFeed )
     {
       usType = HIDDEN_CHAR;     //current LF is not 1st in block of
     } /* endif */               //inline tags
   } /* endif */
   return( usType );
} /* end of function EQFBCheckCompactLF */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBCompactPlusOne
//-----------------------------------------------------------------------------
// Function call:     EQFBCompactPlusOne (PTBDOCUMENT)
//-----------------------------------------------------------------------------
// Description:       correct usType for Compact plus one style
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//                    PTBSEGMENT  pSeg
//                    PSTARTSTOP  pstCurrent
//                    USHORT      usOffs
//-----------------------------------------------------------------------------
// Returncode type:   USHORT
//-----------------------------------------------------------------------------
// Returncodes:       usType     PROTECTED_CHAR  at begin of  tag
//                               HIDDEN_CHAR  else
//-----------------------------------------------------------------------------
// Prerequesits:      start/stop table exists
//-----------------------------------------------------------------------------
// Function flow:     if at start o
//-----------------------------------------------------------------------------
static
VOID EQFBCompactPlusOne
(
   PTBSEGMENT  pSeg,                    // number of active segment
   PSTARTSTOP  pstCurrent,              // start-stop region of offset
   USHORT      usOffs,                           // offset in active segment
   PUSHORT     pusType
)
{
   USHORT     usStart = 0;
   USHORT     usType = *pusType;
   BOOL       fBlankFound = FALSE;
   BOOL       fCheckNec = FALSE;       // do not check type
   USHORT     usShortStart = 0;
   USHORT     usStop = 0;

   /********************************************************/
   /* display first word( blank and colon are word delimiter */
   /* and last character of tag, hide all other characters   */
   /********************************************************/
   /********************************************************/
   /* find start of protected parts                        */
   /********************************************************/
   if (pSeg->qStatus == QF_NOP )
   {
     if ((usType == PROTECTED_CHAR) || (usType == UNPROTECTED_CHAR) )
     {
       fCheckNec = TRUE;
       usStart = 0;
       usStop = pSeg->usLength-1;
     } /* endif */
   }
   else
   {
     if (usType == PROTECTED_CHAR )
     {
       if (pstCurrent->usStop - pstCurrent->usStart >= 10 )
       {
         usStart = pstCurrent->usStart;
         usStop = pstCurrent->usStop;
         fCheckNec = TRUE;
       } /* endif */
     } /* endif */
   } /* endif */
   /***************************************************************/
   /* find start of SHORTEN_CHAR: it is end of 1st word, but at   */
   /* most the tenth character                                    */
   /***************************************************************/
   if (fCheckNec )
   {
     while ( !fBlankFound )      // find start of SHORTEN_CHAR
     {
       if (usShortStart > 10 )
       {
         fBlankFound = TRUE;
       }
       else if (*(pSeg->pDataW + usStart + usShortStart) == BLANK )
       {
           fBlankFound = TRUE;
       } /* endif */
       usShortStart++;
     } /* endwhile */
     /***************************************************************/
     /* usOffs < usStart:  char is protected                        */
     /* usStart-1 <= usOffs <=  usStart+1: char is SHORTEN_CHAR("...")*/
     /* usStart+1 < usOffs: char is HIDDEN                          */
     /* last char of NOP or tag: PROTECTED                          */
     /***************************************************************/
     usShortStart = (USHORT)(usShortStart + usStart);
     // P018515: assure that tag does not become longer!

		 if (usOffs == usStop)
		 { // last char of NOP or last char of inline tag
		   usType = PROTECTED_CHAR;
		 }
		 else if (usOffs > usShortStart + 1 )
		 {
		   usType = HIDDEN_CHAR;
		 }
	     else if ((usShortStart + 1 < usStop) &&
	         ((usOffs == usShortStart + 1) || ( usOffs == usShortStart )
					 || ( usOffs == usShortStart-1 ) ))
		 {
		   usType = SHORTEN_CHAR;
		 }
		 else
		 {  // avoid UNPROTECTED_CHAR in NOP in compact+1 style!!!
		   usType = PROTECTED_CHAR;
		 } /* endif */
   } /* endif */

   *pusType = usType;
   return;
} /* end of function EQFBCompactPlusOne */

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBDiffTag
//-----------------------------------------------------------------------------
// Function call:     EQFBDiffTag(PTBDOCUMENT,USHORT,USHORT)
//-----------------------------------------------------------------------------
// Description:       The function evaluates whether the current character
//                    and the previous character belong to different tags
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   pDoc   ptr to the structure describing the
//                                         active segment
//                    USHORT        usSeg  entry number of active segment
//                    USHORT        usOffs offset of current character in
//                                         active segment
//-----------------------------------------------------------------------------
// Returncode type:   BOOL
//-----------------------------------------------------------------------------
// Returncodes:       TRUE      different tag
//                    FALSE     the same tag
//-----------------------------------------------------------------------------
// Prerequesits:      document tag table must have been loaded
//-----------------------------------------------------------------------------
// Function flow:     init return indicator to FALSE
//                    get start of start/stop table
//                    find start/stop token which contains current offset
//                    if offset is equal a start of a token
//                      previous character belongs to another tag
//                      set indicator fDifferent = TRUE
//                      force reparse
//-----------------------------------------------------------------------------

BOOL EQFBDiffTag
(
   PTBDOCUMENT pDoc,                    // ptr to active document
   ULONG       ulSeg,                   // number of active segment
   USHORT      usOffs                   // offset in active segment
)
{
   BOOL fDifferent = FALSE;            // same tag
   PTBSEGMENT pSeg;                   // ptr to segment
   PSTARTSTOP pstCurrent;             //ptr to current start/stop entry

   pSeg = EQFBGetSegW( pDoc, ulSeg );
   pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
   if (pstCurrent && pDoc->hwndRichEdit )
   {
      UtlAlloc((PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG);
   } /* endif */

   if ( pstCurrent )
   {
      while ((pstCurrent->usType) != 0 &&
                   (pstCurrent->usStart) < usOffs )          /* @KIT0965M */
      {
         pstCurrent++;
      } /* endwhile */
                                           //if usOffs is start of entry,
      if  (pstCurrent ->usStart == usOffs) //previous char belongs to other tag
      {
         fDifferent = TRUE;
         // force reparse
         UtlAlloc((PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG);
      } /* endif */
   } /*endif*/
   return( fDifferent);
} /* end of EQFBDiffTag */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBDiffProtectTag
//-----------------------------------------------------------------------------
// Function call:     EQFBDiffProtectTag(PTBDOCUMENT,USHORT,USHORT)
//-----------------------------------------------------------------------------
// Description:       checks whether typing allowed if cursor right of text
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc,       ptr to active document
//                    USHORT      usSeg,      number of active segment
//                    USHORT      usOffs      offset in active segment
//-----------------------------------------------------------------------------
// Returncode type:   BOOL
//-----------------------------------------------------------------------------
// Returncodes:       TRUE   typing allowed
//                    FALSE  typing not allowed
//-----------------------------------------------------------------------------
// Function flow:     init return value to true
//                    if displaystyle is not unprotected
//                      if startstop table exists
//                        find start/stop node of current offset
//                        if type of current start/stop is protected
//                        and cursor is not equal stop of token
//                          typing not allowed (return = FALSE)
//                        endif
//                      endif
//                    endif
//-----------------------------------------------------------------------------

BOOL EQFBDiffProtectTag
(
   PTBDOCUMENT pDoc,                    // ptr to active document
   ULONG       ulSeg,                   // number of active segment
   USHORT      usOffs                            // offset in active segment
)
{
   BOOL fOK = TRUE;            // typing allowed
   PTBSEGMENT pSeg;                   // ptr to segment
   PSTARTSTOP pstCurrent;             //ptr to current start/stop entry

   if ( pDoc->DispStyle != DISP_UNPROTECTED)
   {
      pSeg = EQFBGetSegW( pDoc, ulSeg );
      pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
      if (pstCurrent && pDoc->hwndRichEdit )
      {
         UtlAlloc((PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG);
      } /* endif */
      if ( pstCurrent )
      {
         while ((pstCurrent->usType) != 0 &&
                      (pstCurrent->usStart) <= usOffs )
         {
            pstCurrent++;
         } /* endwhile */

         if ( (pstCurrent->usType != 0)
             && (usOffs != (pstCurrent-1)->usStop)
             && ((pstCurrent-1)->usType == PROTECTED_CHAR) )
         {
           fOK = FALSE;
         } /* endif */
         //P016804: add the following if-clause:
         if ( (pstCurrent->usType != 0)
		      && (usOffs == (pstCurrent-1)->usStop)
		      && ((pstCurrent-1)->usType == PROTECTED_CHAR)
		      && (*(pSeg->pDataW+usOffs) == '\n') )
		 {
	        fOK = FALSE;
         } /* endif */
      } /*endif*/
   } /* endif */
   return( fOK);
} /* end of function EQFBDiffProtectTag */

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBReparse
//-----------------------------------------------------------------------------
// Function call:     EQFBReparse(PTBDOCUMENT, PTBSEGMENT,USHORT,SHORT)
//-----------------------------------------------------------------------------
// Description:       adjust all start-stop points by sDiff ,
//                    starting at the current position in the
//                    segment table
//-----------------------------------------------------------------------------
// Parameters:        PTBSEGMENT    pSeg   - ptr to segment
//                    USHORT        usOffs - current offset in segment data
//                    SHORT         sDiff  - differenz how much to shift
//                                           (sDiff = 1 : insert  char
//                                            sDiff = -1 : delete char
//                                            sDiff = 10 : insert 10 blanks
//                                                      at right end of line
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Function flow:     if at begin of segment
//                      tokenize nec.
//                    else
//                      search for current position in start/stop table
//                      if end reached and last triple is protected
//                        force a recompute
//                      else
//                        adjust all following start/stops by sDiff
//                      endif
//                      if a character was deleted
//                        remove one start/stop entry if empty
//                        adjust start/stop table
//                      endif
//                    endif
//-----------------------------------------------------------------------------

VOID EQFBReparse
(
   PTBDOCUMENT pDoc,
   PTBSEGMENT  pSeg,                    // ptr to active segment
   USHORT      usOffs,                  // current offset in segment data
   SHORT       sDiff                    // differenz how much to shift
)
{
// at begin of segment status cannot be determined -> tokenize nec.
   if ((usOffs == 0) || ((pDoc->lFontLangInfo & GCP_REORDER ) && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay))
   {
      UtlAlloc((PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG); // free old segment table
   }
   else
   {
       //search for current position in start/stop table
       EQFBReparseStartStop((PSTARTSTOP *) &pSeg->pusBPET, usOffs, sDiff);
   } /* endif */

   if (pSeg->pusHLType)
   {
 	   EQFBReparseHLType((PSTARTSTOP *) &pSeg->pusHLType, usOffs, sDiff, pSeg);
   }  /* endif */

   return;
} /* end of EQFBReparse */

VOID
EQFBReparseStartStop
(
	PSTARTSTOP *    ppstStart,
	USHORT   		usOffs,					// current offset in segment data
	SHORT			sDiff    			    // difference how much to shift
)
{
	PSTARTSTOP  pstStart =*ppstStart;
	PSTARTSTOP  pstCurrent = pstStart;
    if ( pstCurrent )
    {
          while ((pstCurrent->usType) != 0 &&
                       (pstCurrent->usStart) <= usOffs )
          {
             pstCurrent++;
          } /* endwhile */
          //  if last triple was protected, so force a recompute
          if  ( ( (pstCurrent-1)->usType == PROTECTED_CHAR) || (pstCurrent == pstStart) )
          {
             // force reparse, i.e. free memory and set return value
             UtlAlloc((PVOID *)&(pstStart) ,0L ,0L , NOMSG);
             *ppstStart = NULL;
          }
          else
          {
			    pstCurrent--;
                pstCurrent->usStop  = (USHORT)(pstCurrent->usStop + sDiff);  // adjust end position
                                                     // adjust all others
                pstCurrent++;
                while ((pstCurrent->usType) != 0)
                {
                   pstCurrent->usStart = (USHORT)(pstCurrent->usStart + sDiff);
                   pstCurrent->usStop  = (USHORT)(pstCurrent->usStop + sDiff);
                   pstCurrent ++;
                } /* endwhile */
          } /* endif */
    } /* endif */

    // if character deleted, check if one triple has become empty
    pstCurrent = pstStart;
    if ( pstCurrent && sDiff < 0 )
    {
	  while ( pstCurrent->usType )
	  {
		 if ( pstCurrent->usStart > pstCurrent->usStop )
		 {
			// current entry has to be removed
			while ( pstCurrent->usType )
			{
			   pstCurrent = (pstCurrent + 1);
			   pstCurrent++;
			} /* endwhile */
		 }
		 else
		 {
			pstCurrent ++;
		 } /* endif */
	  } /* endwhile */
    } /* endif */

	return;
}


VOID
EQFBReparseHLType
(
	PSTARTSTOP *    ppstStart,
	USHORT   		usOffs,							// current offset in segment data
	SHORT			sDiff,							// difference how much to shift
	PTBSEGMENT      pSeg
)
{   PSTARTSTOP  pstStart =*ppstStart;
	PSTARTSTOP  pstCurrent = pstStart;
	BOOL        fResetAll = FALSE;

    if ( pstCurrent )
    {
        while ((pstCurrent->usType) != 0 &&
                       (pstCurrent->usStart) <= usOffs )
        {
             pstCurrent++;
        } /* endwhile */

        if (pstCurrent != pstStart)
        {
            pstCurrent--;
            if (pstCurrent->usStop >= usOffs )
            {
				if ( pstCurrent->usStop + sDiff >= 0 )
				{
	   			  pstCurrent->usStop = (USHORT)(pstCurrent->usStop + sDiff);
			    }
			    else
			    {
					fResetAll = TRUE;
			    }
		    }
		    pstCurrent++;
	    }

    	  // adjust all others
    	  while (!fResetAll && ((pstCurrent->usType) != 0))
    	  {
    		 if (pstCurrent->usStart + sDiff >= 0)
    		 {
    		   pstCurrent->usStart = (USHORT)(pstCurrent->usStart + sDiff);
    		   pstCurrent->usStop  = (USHORT)(pstCurrent->usStop + sDiff);
    		   pstCurrent ++;
    		 }
    		 else
    		 {
    			 fResetAll = TRUE;
    		 }
    	  } /* endwhile */

	    if (fResetAll)
	    {
			UtlAlloc((PVOID *)&(pstStart) ,0L ,0L , NOMSG);
			*ppstStart = NULL;
			pSeg->SegFlags.Spellchecked = FALSE;
	    }
    } /* endif */

    // if character deleted, check if one triple has become empty
    pstCurrent = pstStart;
    if ( pstCurrent && sDiff < 0 )
    {
	  while ( pstCurrent->usType )
	  {
		 if ( pstCurrent->usStart > pstCurrent->usStop )
		 {
			// current entry has to be removed
			while ( pstCurrent->usType )
			{
			   pstCurrent = (pstCurrent + 1);
			   pstCurrent++;
			} /* endwhile */
		 }
		 else
		 {
			pstCurrent ++;
		 } /* endif */
	  } /* endwhile */
    } /* endif */

	return;
}


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

// function to correct size values in older history log records (in these records
// the usSize value was not set correctly)
void HistLogCorrectRecSizes( PHISTLOGRECORD pRecord )
{
  // only process older records ....
  if ( !pRecord->fLongNameRecord )
  {
    USHORT usTotalSize, usAddInfoSize, usNewSize;
    usTotalSize = pRecord->usSize;
    usAddInfoSize = pRecord->usAddInfoLength;
    if ( usAddInfoSize != usTotalSize - sizeof(HISTLOGRECORD) )
    {
      switch ( pRecord->Task )
      {
        case DOCIMPORT_LOGTASK :
          usNewSize = sizeof(DOCIMPORTHIST);
          break;
        case DOCIMPORT_LOGTASK2 :
          usNewSize = sizeof(DOCIMPORTHIST2); //SHIPMENT_HANDLER
          break;

        case ANALYSIS_LOGTASK :
          usNewSize = sizeof(ANALYSISHIST);
          break;
        case DOCDELETE_LOGTASK :
          usNewSize = 0;
          break;
        case AUTOMATICSUBST_LOGTASK :
        case DOCIMPNEWTARGET_LOGTASK :
        case DOCSAVE_LOGTASK  :
          usNewSize = sizeof(DOCSAVEHIST);
          break;
        case DOCEXPORT_LOGTASK :
          usNewSize = sizeof(DOCEXPORTHIST);
          break;
        case DOCPROP_LOGTASK :
          usNewSize = sizeof(DOCPROPHIST);
          break;
        case FOLPROP_LOGTASK :
          usNewSize = sizeof(FOLPROPHIST);
          break;
        case FOLPROPSHIPMENT_LOGTASK :
          usNewSize = sizeof(FOLPROPHISTSHIPMENT); //SHIPMENT_HANDLER
          break;

        case LONGNAME_LOGTASK :
          usNewSize = sizeof(LONGNAMEHIST);
          break;
        case DOCIMPNEWTARGET_LOGTASK2 :
        case DOCSAVE_LOGTASK2 :
        case DOCAPI_LOGTASK :
          usNewSize = sizeof(DOCSAVEHIST2);
          break;
        case ANALYSIS_LOGTASK3 :
        case DOCIMPNEWTARGET_LOGTASK3 :
        case DOCSAVE_LOGTASK3 :
        case DOCAPI_LOGTASK3 :
          usNewSize = sizeof(DOCSAVEHIST3);
          break;
        case VERSION_LOGTASK :
          usNewSize = sizeof(VERSIONHIST);
          break;
        default:
          usNewSize = usTotalSize - sizeof(HISTLOGRECORD);
      } /* endswitch */
      pRecord->usSize = usNewSize + sizeof(HISTLOGRECORD);
      pRecord->usAddInfoLength = usNewSize;
    } /* endif */
  } /* endif */
} /* end of function HistLogCorrectRecSizes */


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
  return( EQFBHistDocSaveEx( pszFileName, pDoc, sLogTaskID, FALSE ) );
}

SHORT EQFBHistDocSaveEx
(
   PSZ          pszFileName,             // ptr to fully qualified file name
   PTBDOCUMENT  pDoc,                  // ptr to document control structure
   SHORT        sLogTaskID,            // log task ID or 0 for no log record
   BOOL         fAdjustCountInfo        // TRUE = adjust counting information in document properties
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

  if ( !sRc && fAdjustCountInfo )
  {
    AdjustCountInfo( pDoc, szDocObjName );
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

          /************************************************************/
          /* force write of document with the updated info...         */
          /************************************************************/
          EQFBFileWriteEx( pszReadFileName, pDoc, 0, 0 );
        } /* endif */
      } /* endif */
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


// type of memory matches
typedef enum _MATCHTYPE
{
  NO_MATCH, FUZZY_MATCH, EXACTMORE_MATCH, EXACTONE_MATCH, EXACTEXACT_MATCH,
  REPL_MATCH, FUZZYREPL_MATCH, MACHINE_MATCH, GLOBMEM_MATCH
} MATCHTYPE, *PMATCHTYPE;

// data structure for adjust count information
typedef struct _ADJUSTCOUNTINFODATA
{
  // count fields
  ULONG ulNoProps;
  ULONG ulFuzzy;
  ULONG ulExactExact;
  ULONG ulExactOne;
  ULONG ulExactMore;
  ULONG ulRepl;
  ULONG ulFuzzyRepl;
  ULONG ulMachineMatch;
  ULONG ulTotal;
  ULONG ulSegNoProps;
  ULONG ulSegFuzzy;
  ULONG ulSegExactExact;
  ULONG ulSegExactOne;
  ULONG ulSegExactMore;
  ULONG ulSegTotal;
  ULONG ulSegRepl;
  ULONG ulSegFuzzyRepl;
  ULONG ulSegMachineMatch;
  COUNTSUMS Total;
  COUNTSUMS ExactExact;
  COUNTSUMS ExactOne;
  COUNTSUMS Fuzzy1;
  COUNTSUMS Fuzzy2;
  COUNTSUMS Fuzzy3;
  COUNTSUMS NoProps;
  COUNTSUMS MTProps;
  COUNTSUMS Repl;
  COUNTSUMS ExactMore;
  // memory lookup fields
  OtmProposal      SearchKey;                  // search key for memory lookup
  std::vector<OtmMemory *> MemoryDBs;            // list of memory databases
  std::vector<OtmProposal *> FoundProposals;     // list of found proposals
  std::vector<OtmProposal *> BestProposals;      // list of best proposals
  CHAR   szMemNames[MAX_NUM_OF_FOLDER_MDB+1][MAX_LONGFILESPEC]; // name of memories 
  CHAR   szBuffer[2048];                        // general purpose buffer
  CHAR   szTagTable[MAX_EQF_PATH];              // document format
  CHAR   szSourceLanguage[MAX_EQF_PATH];        // source language
  CHAR   szTargetLanguage[MAX_EQF_PATH];        // target language
  CHAR   szLongName[MAX_LONGFILESPEC];          // LongName,

} ADJUSTCOUNTINFODATA, *PADJUSTCOUNTINFODATA;

// prototypes for helper functions
USHORT ACI_UpdateDocProps( PADJUSTCOUNTINFODATA pData, PSZ pszDocObjectName );
USHORT ACI_CloseMemory( PADJUSTCOUNTINFODATA pData );
USHORT ACI_OpenMemory( PADJUSTCOUNTINFODATA pData, PSZ pszDocObjName );
USHORT ACI_PrepareSearchKey( PADJUSTCOUNTINFODATA pData, PSZ pszDocObjName );
USHORT ACI_GetmatchType( PADJUSTCOUNTINFODATA pData, PTBSEGMENT pSeg, PMATCHTYPE pMatchType );
USHORT ACI_AddCounts( PCOUNTSUMS pSum, LONG lSegs, LONG lWords );

// Adjust word count information in document properties
USHORT AdjustCountInfo( PTBDOCUMENT pDoc, PSZ pszDocObjName )
{
  PADJUSTCOUNTINFODATA pData = NULL;
  USHORT usRC = 0;

  // allocate our data area
  pData = new(ADJUSTCOUNTINFODATA);

  if ( pData == NULL )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;  
  } /* endif */     

  // open document and folder memories
  if ( usRC == 0 ) ACI_OpenMemory( pData, pszDocObjName );

  // loop over all segments and get count information
  if ( usRC == 0 )
  {
    ULONG       ulSegNum = 1;
    PTBSEGMENT pSeg = EQFBGetSegW( pDoc, ulSegNum );

    ACI_PrepareSearchKey( pData, pszDocObjName );

    while ( pSeg && (ulSegNum < pDoc->ulMaxSeg) )
    {
      // for all segments with translatable data which are not joined...
      if ( ((pSeg->qStatus == QF_XLATED) || (pSeg->qStatus == QF_TOBE ) || (pSeg->qStatus == QF_ATTR ) || (pSeg->qStatus == QF_CURRENT)) &&  
            !pSeg->SegFlags.Joined )
      {
        MATCHTYPE MatchType = NO_MATCH;
        if (pSeg->qStatus == QF_XLATED )
        { 
          MatchType = EXACTONE_MATCH;
        }
        else
        {
          ACI_GetmatchType( pData, pSeg, &MatchType );
        } /* endif */           

        switch ( MatchType )
        {
          case NO_MATCH         : 
            pData->ulNoProps += pSeg->usSrcWords; 
            pData->ulSegNoProps++; 
            ACI_AddCounts( &(pData->NoProps), 1, pSeg->usSrcWords );
            break;
          case REPL_MATCH       :
          case FUZZYREPL_MATCH  :
          case FUZZY_MATCH      : 
            {
              int iFuzziness = pData->BestProposals[0]->getFuzziness();
              pData->ulFuzzy += pSeg->usSrcWords; 
              pData->ulSegFuzzy++; 
              if( iFuzziness >= (USHORT)(FUZZY_THRESHOLD_0 * 100))
              {
                if ( iFuzziness <= (USHORT)(FUZZY_THRESHOLD_1 * 100))
                {
                  ACI_AddCounts( &(pData->Fuzzy1), 1, pSeg->usSrcWords );
                }
                else if ( iFuzziness <= (USHORT)(FUZZY_THRESHOLD_2 * 100))
                {
                  ACI_AddCounts( &(pData->Fuzzy2), 1, pSeg->usSrcWords );
                }
                else
                {
                  ACI_AddCounts( &(pData->Fuzzy3), 1, pSeg->usSrcWords );
                }  /* end if fuzzyness */
              }
              else
              {
                // treat as no match
                ACI_AddCounts( &(pData->NoProps), 1, pSeg->usSrcWords );
              } /* endif */
            }
            break;
          case EXACTMORE_MATCH  : 
            pData->ulExactMore += pSeg->usSrcWords; 
            pData->ulSegExactMore++; 
            ACI_AddCounts( &(pData->ExactMore), 1, pSeg->usSrcWords );
            break;
          case EXACTONE_MATCH   : 
            pData->ulExactOne += pSeg->usSrcWords; 
            pData->ulSegExactOne++; 
            ACI_AddCounts( &(pData->ExactOne), 1, pSeg->usSrcWords );
            break;
          case EXACTEXACT_MATCH : 
            pData->ulExactExact += pSeg->usSrcWords; 
            pData->ulSegExactExact++; 
            ACI_AddCounts( &(pData->ExactExact), 1, pSeg->usSrcWords );
            break;
          case MACHINE_MATCH    : 
            pData->ulMachineMatch += pSeg->usSrcWords; 
            pData->ulSegMachineMatch++; 
            ACI_AddCounts( &(pData->MTProps), 1, pSeg->usSrcWords );
            break;
          case GLOBMEM_MATCH    : 
            pData->ulExactOne += pSeg->usSrcWords; 
            pData->ulSegExactOne++; 
            ACI_AddCounts( &(pData->ExactMore), 1, pSeg->usSrcWords );
            break;
          default               : 
            pData->ulNoProps += pSeg->usSrcWords; 
            pData->ulSegNoProps++; 
            ACI_AddCounts( &(pData->NoProps), 1, pSeg->usSrcWords );
            break;
        } /* endswitch */           
        pData->ulTotal += pSeg->usSrcWords;
        pData->ulSegTotal++;
       } /* endif */

      ulSegNum++;
      pSeg = EQFBGetSegW(pDoc, ulSegNum);
    } /* endwhile */
  } /* endif */           

  // close all memories
  if ( pData != NULL ) ACI_CloseMemory( pData );

  // update document properties
  if ( usRC == 0 ) ACI_UpdateDocProps( pData, pszDocObjName );

  // cleanup
  if ( pData != NULL )  
  {
     for ( int i=0; i < (int)pData->FoundProposals.size(); i++ ) delete( pData->FoundProposals[i] );
     for ( int i=0; i < (int)pData->BestProposals.size(); i++ ) delete( pData->BestProposals[i] );

    delete( pData );
  }

  return( usRC );
} /* end of function AdjustCountInfo */

//
// helper functions for AdjustCountInfo
//
//

// update document properties with count info
USHORT ACI_UpdateDocProps( PADJUSTCOUNTINFODATA pData, PSZ pszDocObjName )
{
  HPROP           hPropDocument;            // handle to document properties
  PPROPDOCUMENT   pPropDocument = NULL;     // pointer to document properties
  ULONG           ulErrorInfo;              // error indicator from PRHA
  USHORT usRC = 0;

  if( (hPropDocument = OpenProperties( pszDocObjName, NULL, PROP_ACCESS_READ, &ulErrorInfo)) != NULL )
  {
    pPropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hPropDocument );
    SetPropAccess( hPropDocument, PROP_ACCESS_WRITE );

    pPropDocument->ulNoProps    = pData->ulNoProps;
    pPropDocument->ulFuzzy      = pData->ulFuzzy;
    pPropDocument->ulExactExact = pData->ulExactExact;
    pPropDocument->ulExactOne   = pData->ulExactOne;
    pPropDocument->ulExactMore  = pData->ulExactMore;
    pPropDocument->ulRepl       = pData->ulRepl;
    pPropDocument->ulFuzzyRepl  = pData->ulFuzzyRepl;
    pPropDocument->ulMachineMatch = pData->ulMachineMatch;
    pPropDocument->ulTotal      = pData->ulTotal;
    pPropDocument->ulSegNoProps    = pData->ulSegNoProps;
    pPropDocument->ulSegFuzzy      = pData->ulSegFuzzy;
    pPropDocument->ulSegExactExact = pData->ulSegExactExact;
    pPropDocument->ulSegExactOne   = pData->ulSegExactOne;
    pPropDocument->ulSegExactMore  = pData->ulSegExactMore;
    pPropDocument->ulSegTotal      = pData->ulSegTotal;
    pPropDocument->ulSegRepl       = pData->ulSegRepl;
    pPropDocument->ulSegFuzzyRepl  = pData->ulSegFuzzyRepl;
    pPropDocument->ulSegMachineMatch = pData->ulSegMachineMatch;

    memcpy( &pPropDocument->Total,      &pData->Total,    sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->ExactExact, &pData->ExactExact, sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->ExactOne,   &pData->ExactOne, sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->Fuzzy1,     &pData->Fuzzy1,   sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->Fuzzy2,     &pData->Fuzzy2,   sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->Fuzzy3,     &pData->Fuzzy3,   sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->NoProps,    &pData->NoProps,  sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->MTProps,    &pData->MTProps,  sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->Repl,       &pData->Repl,     sizeof(COUNTSUMS) );
    memcpy( &pPropDocument->ExactMore,  &pData->ExactMore, sizeof(COUNTSUMS) );

    // save and close properties
    SaveProperties( hPropDocument, &ulErrorInfo );
    ResetPropAccess( hPropDocument, PROP_ACCESS_WRITE);
    CloseProperties( hPropDocument, PROP_FILE, &ulErrorInfo);
  } /* endif */
  return( usRC );
} /* end of function ACI_UpdateDocProps */


// open the document memory and the list of folder read-only memories
USHORT ACI_OpenMemory( PADJUSTCOUNTINFODATA pData, PSZ pszDocObjName )
{
  USHORT      usRc = 0;
  HPROP       hProp;                   // handle to document properties
  ULONG       ulErrorInfo;             // error indicator from PRHA
  int         iNumOfMems = 0;
  BOOL        fOK = TRUE;

  DocQueryInfo2( pszDocObjName, pData->szMemNames[0], NULL, NULL, NULL, NULL, NULL, NULL, TRUE );
  iNumOfMems++;

  // prepare folder object name using pData->szMemNames[1] as name buffer
  {
    CHAR szDrive[3];
    strcpy( pData->szMemNames[1], pszDocObjName );
    UtlSplitFnameFromPath( pData->szMemNames[1] );
    UtlQueryString( QST_PRIMARYDRIVE, szDrive, sizeof(szDrive) );
    pData->szMemNames[1][0] = szDrive[0];
  }

  // get list of folder read-only memories
  hProp = OpenProperties( pData->szMemNames[1], NULL, PROP_ACCESS_READ, &ulErrorInfo );
  if ( hProp )
  {
    PPROPFOLDER pProp = (PPROPFOLDER)MakePropPtrFromHnd( hProp );

    // add folder R/O memories to the list
    if ( pProp->aLongMemTbl[0][0] != EOS )
    {
      int i = 0;
      while ( (i < MAX_NUM_OF_READONLY_MDB) && (pProp->aLongMemTbl[i][0] != EOS) )
      {
        strcpy( pData->szMemNames[iNumOfMems], pProp->aLongMemTbl[i] );
        iNumOfMems++;
        i++;
      } /*endwhile */
    }
    else
    {
      PSZ pszToken;

      //get first R/O memory from list
      strcpy( pData->szBuffer, pProp->MemTbl );
      pszToken = strtok( pData->szBuffer, X15_STR );
      while ( (pszToken != NULL) && fOK )
      {
        strcpy( pData->szMemNames[iNumOfMems], pszToken );
        iNumOfMems++;
        pszToken = strtok( NULL, X15_STR );
      } /* endwhile */
    } /* endif */
    CloseProperties( hProp, PROP_QUIT, &ulErrorInfo );
  } /* endif */

  // open the memories
  if ( !usRc && iNumOfMems )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    int i = 0;
    while ( !usRc && (i < iNumOfMems) )
    {
      int iRC = 0;
      OtmMemory *pMem = pFactory->openMemory( NULL, pData->szMemNames[i], READONLYACCESS, &iRC );
      if ( pMem != NULL )
      {
        pData->MemoryDBs.push_back( pMem );
      } /* endif */        
      usRc = (USHORT)iRC;
      i++;
    } /*endwhile */

    // prepare proposal vectors
    for ( int i = 0; i < 3; i++ ) pData->FoundProposals.push_back( new(OtmProposal) );
    for ( int i = 0; i < 3; i++ ) pData->BestProposals.push_back( new(OtmProposal) );

  } /* endif */

  return( usRc );
} /* end of function ACI_OpenMemory */

//  close the memory DBs associated with the folder
USHORT ACI_CloseMemory( PADJUSTCOUNTINFODATA pData )
{
  USHORT       usRc = 0;
  int i = 0;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  for (i = 0; i < (int)pData->MemoryDBs.size(); i++ )
  {
    pFactory->closeMemory( pData->MemoryDBs[i] );
  }
  pData->MemoryDBs.clear(); 

  return( usRc );
} /* end of function ACI_CloseMemory */

// get document info and store it in search key
USHORT ACI_PrepareSearchKey( PADJUSTCOUNTINFODATA pData, PSZ pszDocObjName )
{
  DocQueryInfo2( pszDocObjName,
                  NULL,                        // memory
                  pData->szTagTable,           // document format
                  pData->szSourceLanguage,     // source language
                  pData->szTargetLanguage,     // target language
                  pData->szLongName,           // LongName,
                  NULL,                        // Alias
                  NULL,                        // editor
                  TRUE );

  pData->SearchKey.clear();
  pData->SearchKey.setMarkup( pData->szTagTable ); 
  pData->SearchKey.setSourceLanguage( pData->szSourceLanguage ); 
  pData->SearchKey.setTargetLanguage( pData->szTargetLanguage ); 
  pData->SearchKey.setDocName( pData->szLongName ); 
  pData->SearchKey.setDocShortName( UtlGetFnameFromPath( pszDocObjName ) ); 

  return( 0 );
}

// get the available proposals for the segment and evaluate the type of matches
USHORT ACI_GetmatchType( PADJUSTCOUNTINFODATA pData, PTBSEGMENT pSeg, PMATCHTYPE pMatchType )
{
  USHORT       usRC = 0;
  int i = 0;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  *pMatchType = NO_MATCH;

  pData->SearchKey.setSource( pSeg->pDataW );

  OtmProposal::clearAllProposals( pData->BestProposals );
  while ( !usRC && (i < (int)pData->MemoryDBs.size()) && (pData->MemoryDBs[i] != NULL) )
  {
    OtmProposal::clearAllProposals( pData->FoundProposals );
    pData->MemoryDBs[i]->searchProposal( pData->SearchKey, pData->FoundProposals, GET_EXACT );
    pFactory->copyBestMatches( pData->FoundProposals, pData->BestProposals, 3 );
    i++;
  } /* endwhile */
  
  // evaluate match type
  int iProposals = OtmProposal::getNumOfProposals( pData->BestProposals );
  if ( iProposals == 0  )
  {
    *pMatchType = NO_MATCH;
  }
  else if ( pData->BestProposals[0]->getType() == OtmProposal::eptMachine )
  {
    *pMatchType = MACHINE_MATCH;
  }
  else if ( (pData->BestProposals[0]->getType() == OtmProposal::eptGlobalMemory) ||
            (pData->BestProposals[0]->getType() == OtmProposal::eptGlobalMemoryStar) )
  {
    *pMatchType = GLOBMEM_MATCH;
  }
  else if ( !pData->BestProposals[0]->isExactMatch() )
  {
    *pMatchType = FUZZY_MATCH;
  }
  else if ( iProposals == 1 ) 
  {
    // only one exact proposal
    if ( pData->BestProposals[0]->getMatchType() == OtmProposal::emtExactExact )
    {
      *pMatchType = EXACTEXACT_MATCH;
    }
    else
    {
      *pMatchType = EXACTONE_MATCH;
    } /* endif */       
  }
  else
  {
    // several exact proposals
    if ( pData->BestProposals[0]->getMatchType() == OtmProposal::emtExactExact )
    {
      *pMatchType = EXACTEXACT_MATCH;
    }
    else
    {
      *pMatchType = EXACTMORE_MATCH;
    } /* endif */       
  } /* endif */     
  return( usRC );
} /* end of function ACI_GetmatchType */

USHORT ACI_AddCounts( PCOUNTSUMS pSum, LONG lSegs, LONG lWords )
{
  LONG lAbsWords = abs( lWords );
  if ( lAbsWords < SIMPLE_SENT_BOUND )
  {
    pSum->ulSimpleSegs  += lSegs;
    pSum->ulSimpleWords += lWords;
  }
  else if ( lAbsWords < MEDIUM_SENT_BOUND )
  {
    pSum->ulMediumSegs  += lSegs;
    pSum->ulMediumWords += lWords;
  }
  else
  {
    pSum->ulComplexSegs  += lSegs;
    pSum->ulComplexWords += lWords;
  } /* endif */
  return( NO_ERROR );
} /* end of function TAAddCounts */

