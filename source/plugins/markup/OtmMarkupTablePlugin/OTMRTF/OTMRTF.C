//+----------------------------------------------------------------------------+
//|OTMRTF.C                                                                  |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:         G. Queck (QSoft)                                            |
//+----------------------------------------------------------------------------+
//|Description:    RTF markup table user exit                                  |
//+----------------------------------------------------------------------------+

// GQ: - fixed P401430: EQFMSWRD TM abends when activating long code-only segments
// GQ: - fixed P400954 EQFRTF: Analysis corrupts the format of an RTF file
// INTERNAL NOTE
// 22.02.2001 bt
// A note on bidi processing:
// Bidi processing is not implemented by 100%,
// support is added for the hebrew language, the processing
// for HCW 4.03.0002 is improved; normal text docs have slight problems
// with braces.
// IMPORTANT: In some cases it's not always guaranteed that you
// get the same results on every platform (WinNT/W2K, hebrew localised
// versions); that means results look good here, but on the target
// installation there are problems vice versa
// hebrew help project files should be compiled with the language setting
// HEBREW, not ANSI

// activating the followng defines will skip the
// normal Ansi->ASCII->Unicode conversion and will
// try to convert directly to UTF16 using MultiByteToWideChar
// Note:
//    This has not been tested throuroughly and should be
//    used with care!!!
//#define RTF_CONVERT_DIRECTLY_TO_UTF16  1

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions

#ifdef WIN32BIT
  // use import DLL defines for dbcs_cp ...
  #define DLLIMPORTDBCSCP
#endif
#include <time.h>
#include <locale.h>
#include <eqf.h>                  // General Translation Manager include file

#include "OTMRTF.H"                  // internal header file
#include <tchar.h>

#ifdef __cplusplus
extern "C"
#endif
extern void   __cdecl  GetOTMTablePath( char * basename, char * OTMname ) ;

// defines for logging debug info into a file
// #define DEBUGLOG
// #undef DEBUGLOG

#ifdef DEBUGLOG
  #define LOGFILE() FILE *hLog
#else
  #define LOGFILE()
#endif


#ifdef DEBUGLOG
  #define LOGOPEN( name ) \
    { LONG lTime; \
      hLog = fopen( name, "a" ); \
      if ( hLog != NULLHANDLE ) \
      { \
        time( &lTime ); \
        fprintf( hLog, "===== Word6 Log Started %s", ctime( &lTime ) ); \
      } \
    }
#else
  #define LOGOPEN( name )
#endif

#ifdef DEBUGLOG
  #define LOGWRITE1( p1 ) { if ( hLog ) fprintf( hLog, p1 ); }
#else
  #define LOGWRITE1( p1 )
#endif

#ifdef DEBUGLOG
  #define LOGWRITE2( p1, p2 ) { if ( hLog ) fprintf( hLog, p1, p2 ); }
#else
  #define LOGWRITE2( p1, p2 )
#endif

#ifdef DEBUGLOG
  #define LOGWRITE3( p1, p2, p3 ) { if ( hLog ) fprintf( hLog, p1, p2, p3 ); }
#else
  #define LOGWRITE3( p1, p2, p3 )
#endif

#ifdef DEBUGLOG
  #define LOGWRITE4( p1, p2, p3, p4 ) { if ( hLog ) fprintf( hLog, p1, p2, p3, p4 ); }
#else
  #define LOGWRITE4( p1, p2, p3, p4 )
#endif

#ifdef DEBUGLOG
  #define LOGWRITE5( p1, p2, p3, p4, p5 ) { if ( hLog ) fprintf( hLog, p1, p2, p3, p4, p5 ); }
#else
  #define LOGWRITE5( p1, p2, p3, p4, p5 )
#endif


#ifdef DEBUGLOG
  #define LOGCLOSE() { if ( hLog ) fclose( hLog ); hLog = NULLHANDLE; }
#else
  #define LOGCLOSE()
#endif

USHORT EQFGetCPOem();
/**********************************************************************/
/* Prototype for new API entry EQFPROTTABLE                           */
/* should be incorporated in OTMAPI.H once this user exit function   */
/* gets public                                                        */
/**********************************************************************/
//__declspec(dllexport)
//BOOL EQFPROTTABLEW
//(
//  PSZ_W            pszSegment,         // ptr to input segment
//  PSTARTSTOP       *ppStartStop        // ptr to caller's start/stop table ptr
//);
//
//__declspec(dllexport)
//EQF_BOOL EQFCHECKSEGW
//(
//   PSZ_W   pszPrevSrc,                 // previous source segment
//   PSZ_W   pszSrc,                     // current source segment
//   PSZ_W   pszTgt,                     // current translation
//   PEQF_BOOL   pfChanged,              // segment changed
//   EQF_BOOL    fMsg                    // message display requested
//);

/**********************************************************************/
/* Prototypes of internal functions                                   */
/**********************************************************************/
int StringCompare( const void *psz1, const void *psz2 );
int StringICompare( const void *psz1, const void *psz2 );
int StringICompareW( register const void *psz1, register const void *psz2 );
SHORT CountBraces( PSZ_W pszText );
USHORT RTFMorphTokenize( PPARSEDATA pParseData );
CHAR_W RTFPreUnsegNextNonNeutral
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PSZ_W       pszCurPos,               // current position in current segment
  PTBDOCUMENT pTBDoc,                  // pointer to segmented target document
  ULONG       *pulSegNum,              // pointer to current Segment #
  PCHAR       pConvTable               // ptr to code conversion table
);
PSZ_W RTFPreUnsegSkipTag
(
  PSZ_W       pszSource,
  PPARSEDATA  pParseData,
  RTFTAGS     *pTagId                 // ID of skipped tag
);
BOOL IsBidiPunct
(
  CHAR_W       chTest
);
BOOL IsBidiOpenBrace
(
  CHAR_W       chTest
);
USHORT RTFMakeTempName
(
  PSZ         pszTempName,             // points to buffer for temp name
  PSZ         pszSourceName            // base name
);

// Re-fill input buffer if half of the buffer is empty
USHORT RTFBufInpReFill
(
  HFILE       hInFile,                 // file handle of input file
  PBYTE       pInBufStart,             // ptr to start of input buffer
  PBYTE       *ppInBufCurrentPos,      // ptr to current position pointer
  PUSHORT     pusBytesInBuffer,        // ptr to number of bytes in input buffer
  PLONG       plBytesToRead            // number of bytes to read from input file
);
BOOL RTFResolveUnicodeTags
(
  PSZ         pszSegFile,              // name of segment file
  PEQF_BOOL   pfKill                   // caller's kill process flag
);
BOOL RTFIsBidiCharW
(
  PPARSEDATA  pParseData,              // points to parser data structure
  CHAR_W      chTestChar,              // character being tested
  PCHAR       pConvTable               // ptr to code conversion table
);

SHORT RTFAddNewSeg
(
  PTBDOCUMENT pDoc,                    // document structure
  PTBSEGMENT  pSeg,                    // segment structure
  PSZ_W       pszSegData,              // data of segment
  PULONG      pulSegOut                // current/new segment number
);
CHAR RTFRemoveLastCharFromWriteBuffer
(
  PPARSEDATA pParseData                // ptr to parser global data structure
);

BOOL RTFIsDBCSChar( CHAR_W c, ULONG ulCP);
VOID RTFParseGetCP (   PSZ  pszInFile, PULONG pulSrcOemCP, PULONG pulTgtOemCP,
                   PULONG pulSrcAnsiCP, PULONG pulTgtAnsiCP);

static VOID
RTFCheckStartStop
(
	PSZ_W      pszSegment,
	PSTARTSTOP pStartStop,
	USHORT     usMaxEntry
);

USHORT RTFWriteInUTF16
(
  PPARSEDATA pParseData,               // ptr to parser global data structure
  HFILE      hOutFile,                 // handle of output file
  PSZ        pszData,                  // ptr to start of data
  ULONG      ulLen,                    // length of the data
  BOOL       fMsg                      // show messagebox flag
);

PSZ_W RTFSkipInlineTagsW( PSZ_W pszSource );
PSZ   RTFSkipInlineTags( PSZ pszSource );
BOOL RTFMatchTag
(
  PSZ         pszTag,                  // tag being checked for
  PSZ         pszPos,                  // current string
  int        *piLen                    // length of matching tag (may be different from tag length)
);
BOOL RTFFindTags
(
  PSZ         pszString,               // points to string being searched for tags
  PSZ         pszSearchTags,           // list of tags
  int        *piLen,                   // when found: length of found tag group
  int        *piTagIndex,              // when found: relative index of found tag in tag list
  PSZ        *ppszPos                  // when found: position of tags within string
);
BOOL RTFMatchTagW
(
  PSZ_W       pszTag,                  // tag being checked for
  PSZ_W       pszPos,                  // current string
  int        *piLen                    // length of matching tag (may be different from tag length)
);
BOOL RTFFindTagsW
(
  PSZ_W       pszString,               // points to string being searched for tags
  PSZ_W       pszSearchTags,           // list of tags
  int        *piLen,                   // when found: length of found tag group
  int        *piTagIndex,              // when found: relative index of found tag in tag list
  PSZ_W      *ppszPos                  // when found: position of tags within string
);
// remove superfluous curly braces from given string
void RTFRemoveSuperfluousBraces
(
  PSZ    pszString,
  PBYTE pbAdjustBuf1,          // ptr to a buffer with data which needs adjusting
  PBYTE pbAdjustBuf2           // ptr to a buffer with data which needs adjusting
);
void RTFRemoveSuperfluousBracesW
(
  PSZ_W  pszString
);
// adjust linefeeds in string
void RTFAdjustLinefeed
(
  PSZ    pszString
);
void RTFAdjustLinefeedW
(
  PSZ_W  pszString
);
// check for and skip a single tag or a hexadecimal encoded value
BOOL RTFSkipTag
(
  PSZ         *ppszSource
);
BOOL RTFSkipTagW
(
  PSZ_W       *ppszSource
);
// check if there is a tag directly in front of the text pointed to by pszText
BOOL RTFTagIsPreceedingTextW
(
  PSZ_W   pszText,
  PSZ_W   pszBufferStart
);
BOOL RTFTagIsPreceedingText
(
  PSZ     pszText,
  PSZ     pszBufferStart
);
// remove tags in supplied buffer, changed segment is copied to supplied buffer
BOOL RTFRemoveTags
(
  PSZ  pszInData,              // ptr to input data
  PSZ  pszOutData,             // ptr to output data
  PSZ  pszRemoveTags,          // tags to be removed
  PBYTE pbAdjustBuf1,          // ptr to a buffer with data which needs adjusting
  PBYTE pbAdjustBuf2           // ptr to a buffer with data which needs adjusting
);
BOOL RTFRemoveTagsW
(
  PSZ_W pszInData,              // ptr to input data
  PSZ_W pszOutData,             // ptr to output data
  PSZ_W pszRemoveTags           // tags to be removed
);
BOOL RTFChangeTagsW
(
  PSZ_W pszInData,              // ptr to input data
  PSZ_W pszOutData,             // ptr to output data
  PSZ_W pszChangeTags,          // tags to be changed
  PSZ_W pszNewTags              // list with new tags
);

/**********************************************************************/
/* indicator for DBCS initialisation                                  */
/**********************************************************************/
BOOL  fInit = FALSE;


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFCHECKSEG         Segment checking function
//------------------------------------------------------------------------------
// Input parameter:   PSZ pszPrevSrc      ptr to previous source segment
//                    PSZ pszSrc          ptr to current source segment
//                    PSZ pszTgt          ptr to current target segment
//                    PBOOL pfChanged     ptr to segment changed flag
//                    BOOL  fMsg          message display flag
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFCHECKSEGW
(
   PSZ_W pszPrevSrc,                     // previous source segment
   PSZ_W pszSrc,                         // current source segment
   PSZ_W pszTgt,                         // current translation
   PEQF_BOOL pfChanged,                // segment changed
   EQF_BOOL fMsg                       // message display requested
)
{
  EQF_BOOL  fContinue = TRUE;
  USHORT usMBCode;                     // message box return code

  /********************************************************************/
  /* get rid of compiler warnings - we don't need this baby here      */
  /********************************************************************/
  pszPrevSrc;
  *pfChanged = FALSE;

  /********************************************************************/
  /* check if number of curly braces has been changed                 */
  /********************************************************************/
  if ( pszSrc && pszTgt )
  {
    SHORT sSrcBraces, sTgtBraces;

    sSrcBraces = CountBraces( pszSrc );
    sTgtBraces = CountBraces( pszTgt );

    if ( sSrcBraces != sTgtBraces )
    {
      if ( fMsg )
      {
         usMBCode = UtlError( ERR_RTF_BRACEMISMATCH, MB_YESNO | MB_DEFBUTTON2,
                              0, NULL, EQF_QUERY );
         if ( usMBCode == MBID_NO )
         {
           fContinue = FALSE;
         } /* endif */
      }
      else
      {
        // Although it is quite important to notify the user about brace
        // mismatches, switch the msgbox off because it could be a real
        // hassle to confirm each occurrence interactively
        //UtlError( WARNING_RTF_BRACEMISMATCH, MB_OK, 1, NULL, EQF_WARNING );
        fContinue = FALSE;             // avoid auto-replace if brace mismatch
      } /* endif */
    } /* endif */
  } /* endif */
  return fContinue;
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFPRESEG          Presegmentation parser (old style)
//------------------------------------------------------------------------------
// Description:       Invoked from text analysis prior to segmentation.
//                    This exit is used only by the old Analysis function
//                    as shipped with the first release of TM/2.
//------------------------------------------------------------------------------
// Input parameter:   PSZ pTagTable       name of tag table
//                    PSZ pEdit           name of editor dll
//                    PSZ pProgPath       pointer to program path
//                    PSZ pSource         pointer to source file
//                    PSZ pTemp           pointer to temp file
//                    PTATAG pTATag       ta tag structure
//                    PBOOL pfNoSegment   do no further segmentation ??
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE           - success
//                    FALSE          - Dos error occured
//------------------------------------------------------------------------------
// Function flow:     call EQFPRESEG2 with slider window handle set to NULL
//------------------------------------------------------------------------------
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPRESEG
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pSource,                  // pointer to source file name
   PSZ pTempSource,              // pointer to source path
   PEQF_BOOL pfNoSegment         // no further segmenting ??
)
{
  EQF_BOOL fKill = FALSE;                 // dummy kill flag             /* @50A */

  return( EQFPRESEG2( pTagTable, pEdit, pProgPath, pSource, pTempSource,
                      pfNoSegment, NULLHANDLE, &fKill ) );                /* @50C */
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFPRESEG2          Presegmentation parser
//------------------------------------------------------------------------------
// Description:       Invoked from text analysis prior to segmentation
//------------------------------------------------------------------------------
// Input parameter:   PSZ pTagTable       name of tag table
//                    PSZ pEdit           name of editor dll
//                    PSZ pProgPath       pointer to program path
//                    PSZ pSource         pointer to source file
//                    PSZ pTemp           pointer to temp file
//                    PTATAG pTATag       ta tag structure
//                    PBOOL pfNoSegment   do no further segmentation ??
//                    HWND  hwndProcWin   handle of analysis process window
//                    PBOOL pfKill        ptr to 'kill running process' flag
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE           - success
//                    FALSE          - Dos error occured
//------------------------------------------------------------------------------
// Function flow:     setup a temporary file name for the segmented file
//                    call ParseRTF
//------------------------------------------------------------------------------
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPRESEG2
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pSource,                  // pointer to source file name
   PSZ pTempSource,              // pointer to source path
   PEQF_BOOL pfNoSegment,        // no further segmenting ??
   HWND  hwndProcWin,            // handle of analysis slider control
   PEQF_BOOL pfKill              // ptr to 'kill running process' flag/* @50A */
)
{
   PSZ pName;                    // pointer to file name

   pTagTable;                    // get rid of compiler warning
   pEdit;
   pProgPath;

   *pfNoSegment = TRUE;          // no further segmentation requested
   strcpy( pTempSource, pSource );
   pName = UtlGetFnameFromPath( pTempSource );

   /*******************************************************************/
   /* Find extension and change it into temp extension '$$$'          */
   /*******************************************************************/
   while ( *pName && *pName != DOT )
   {
      pName ++;
   } /* endwhile */

   if ( !*pName )
   {
      *pName = DOT;
   } /* endif */
   pName++;
   if ( strcmp( pName, "$$$" ) == 0 )
   {
     memset( pName, '@', 3 );
   }
   else
   {
     memset( pName, '$', 3 );
   } /* endif */
   *(pName+3) = EOS;                      // include end of string

   /*******************************************************************/
   /* Call RTF parser to do the actual work                           */
   /*******************************************************************/
   return (! ParseRTF( pSource, pTempSource, hwndProcWin, pfKill ) );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFPOSTSEG            Postsegment parser
//------------------------------------------------------------------------------
// Description:       Here we will resolve any unicode code characters
//------------------------------------------------------------------------------
// Input parameter:   PSZ pTagTable       name of tag table
//                    PSZ pEdit           name of editor dll
//                    PSZ pProgPath       pointer to program path
//                    PSZ pSegSource      pointer to source seg file
//                    PSZ pSegTarget      pointer to target seg file
//                    PTATAG pTATag       ta tag structure
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE           - success
//                    FALSE          - Dos error occured
//------------------------------------------------------------------------------
// Function flow:     currently a NOP
//------------------------------------------------------------------------------
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTSEGW
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pSegSource,               // pointer to source seg. file name
   PSZ pSegTarget,               // pointer to target seg file
   PTATAG_W pTATag,                 // ta tag structure
   HWND       hwndSlider,
   PEQF_BOOL  pfKill

)
{
   EQF_BOOL fOK = TRUE;

   pTagTable; pEdit; pProgPath; pSegTarget; pTATag; hwndSlider;

   fOK = (EQF_BOOL)RTFResolveUnicodeTags( pSegSource, pfKill );

   if ( fOK && !*pfKill )
   {
     fOK = (EQF_BOOL)RTFResolveUnicodeTags( pSegTarget, pfKill );
   } /* endif */
   return ( fOK );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFPREUNSEG           Presegment parser
//------------------------------------------------------------------------------
// Description:       Invoked from unsegment utility prior to unsegmentation
//------------------------------------------------------------------------------
// Input parameter:   PSZ pTagTable       name of tag table
//                    PSZ pEdit           name of editor dll
//                    PSZ pProgPath       pointer to program path
//                    PSZ pSegTarget      pointer to target seg file
//                    PSZ pTemp           pointer to temp file
//                    PTATAG pTATag       ta tag structure
//                    PBOOL pfNoUnseg     do no further unsegmentation ??
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE           - success
//                    FALSE          - Dos error occured
//------------------------------------------------------------------------------
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPREUNSEGW
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pSegTarget,               // pointer to target seg file
   PSZ pTemp,                    // pointer to temp file
   PTATAG_W pTATag,              // ta tag structure
   PEQF_BOOL  pfNoUnseg,         // do no further unsegmentation ??
   PEQF_BOOL  pfKill             // kill flag
)
{
   PSZ pName;

   pTagTable; pEdit; pProgPath; pSegTarget; pTATag; pTemp;
   *pfNoUnseg = FALSE;                 // do further unsegmentation

   strcpy( pTemp, pSegTarget);
   pName = UtlGetFnameFromPath( pTemp );

   // find extension and change it into temp extension '$_$'
   while ( *pName && *pName != '.')
   {
      pName ++;
   } /* endwhile */

   if ( !*pName )
   {
      *pName = '.';
   } /* endif */
   pName++;
   memcpy( pName, "$_$", 3 );
   *(pName+3) = EOS;                      // include end of string

   return (! RTFPreUnseg( pSegTarget, pTemp, pTagTable, pfKill ) );
}



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFPOSTUNSEG          Postunsegment parser
//------------------------------------------------------------------------------
// Description:       Invoked from unsegment utility after unsegmentation
//------------------------------------------------------------------------------
// Input parameter:   PSZ pTagTable       name of tag table
//                    PSZ pEdit           name of editor dll
//                    PSZ pProgPath       pointer to program path
//                    PSZ pSegTarget      pointer to target seg file
//                    PTATAG pTATag       ta tag structure
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE           - success
//                    FALSE          - Dos error occured
//------------------------------------------------------------------------------
// Function flow:     call UnSegment function
//------------------------------------------------------------------------------
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTUNSEG
(

   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pTarget,                  // pointer to target file
   PTATAG pTATag                 // ta tag structure
)
{
   pTagTable; pEdit; pProgPath; pTATag;  // avoid compiler warning 'unreferenced ...'

   return ( (EQF_BOOL)UnparseRTF( pTarget, pTagTable ) );
} /* end of function EQFBPOSTUNSEG */

// helper function toidentify alphabetic characters a..z and A..Z
BOOL RTFisAlpha( CHAR_W c)
{
  if ( (c >= L'A') && (c <= L'Z') ) return( TRUE );
  if ( (c >= L'a') && (c <= L'z') ) return( TRUE );
  return( FALSE );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFPROTTABLE          Create protecte table
//------------------------------------------------------------------------------
// Description:       Creates a start/stop-table for translatable/protected
//                    data.
//------------------------------------------------------------------------------
// Input parameter:   PSZ        pszSegment   ptr to input segment
//                    PSTARTSTOP *ppStartStop ptr to caller's start/stop table
//                               pointer
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE           - success
//                    FALSE          - Dos error occured (memory allocation!)
//------------------------------------------------------------------------------
// Function flow:     call UnSegment function
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
BOOL __cdecl /*APIENTRY*/ EQFPROTTABLEW
(
  PSZ_W            pszSegment,         // ptr to input segment
  PSTARTSTOP       *ppStartStop        // ptr to caller's start/stop table ptr
)
{
  PCTRLWORD        pCtrlWord;          // pointer to control words
  PSTARTSTOP       pStartStop = NULL;  // ptr to new start/stop table
  PSZ_W            pszWork;            // ptr for input segment processing
  PSZ_W            pszTemp;            // ptr for input segment processing
  PSZ_W            pszStartMacro;      // ptr to start of macro name
  BOOL             fOK = TRUE;         // function return code
  USHORT           usMaxEntries = 0;   // number of table entries required
  USHORT           usCurEntry = 0;     // current entry in table
  USHORT           usType;             // type of current table entry
  CHAR_W           chTemp;             // buffer for a single character
  enum
  {
    NORMAL_MODE,                       // processing normal text
    HIDDEN_MODE,                       // processing hidden text
    LINKMACRO_MODE                     // processing !xLINK macro
  } ModeStack[MAX_DEST_STACK+1];       // Stack for processing modes
  SHORT            sModeInd;           // index into mode stack
  USHORT           usParmNum = 0;      // number of current parameter
  HELPMACRO        CurrentMacro;       // buffer for current help macro data

  /********************************************************************/
  /* General initialization                                          */
  /********************************************************************/
  sModeInd = 0;
  ModeStack[sModeInd] = NORMAL_MODE;

  /*******************************************************************/
  /* get number of RTF tags if not computed by a previous call       */
  /*******************************************************************/
  if ( !usNumOfTags )
  {
    usNumOfTags = 0;                    // start with first tag
    while ( CtrlWords[usNumOfTags].szCtrlWord[0] )
    {
      usNumOfTags++;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Estimate size of start/stop table:                               */
  /*                                                                  */
  /* Any backslash and curly brace may require a switch between       */
  /* protected and translatable data. So the number of protect table  */
  /* entries required is twice the number of occurences of these      */
  /* characters in the input segment.                                 */
  /*                                                                  */
  /* New: also count "<" characters which may start an HTML tag       */
  /*                                                                  */
  /* Note: We do no specific check for DBCS characters. If the        */
  /*       input segment contains DBCS characters the actual size     */
  /*       required for the table may be smaller (as the special      */
  /*       characters may be found in the second byte of DBCS         */
  /*       characters) than the estimated size.                       */
  /********************************************************************/
  if ( fOK )
  {
    pszWork      = pszSegment;
    usMaxEntries = 4;                  // always leave some room for 'extras'
    while ( *pszWork )
    {
      switch ( *pszWork )
      {
        case BACKSLASH_W :
        case BEGIN_GROUP_W :
        case END_GROUP_W :
        case L'<' :
          usMaxEntries += 2;
          break;
        case START_MACRO_W:
          usMaxEntries += 6;
          break;
        default :
          break;
      } /* endswitch */
      pszWork++;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Allocate start/stop table                                        */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pStartStop, 0L,
                   (LONG) max( (usMaxEntries * sizeof(STARTSTOP)), MIN_ALLOC),
                   NOMSG );
  } /* endif */

  /********************************************************************/
  /* Fill start/stop table                                            */
  /********************************************************************/
  if ( fOK )
  {
    pszWork = pszSegment;              // start at begin of segment
    usCurEntry = 0;                    // start with first table entry
    usType  = UNPROTECTED_CHAR;        // assume translatable data
    pStartStop[usCurEntry].usType  = usType;
    pStartStop[usCurEntry].usStart = 0;

    while ( *pszWork )                 // while not end of input data ...
    {
      /*************************************************************/
      /* See what we have to do with the next character            */
      /*************************************************************/
      switch ( *pszWork )
      {
        case START_CTRLWORD_W:
          if ( pszWork[1] == START_CTRLWORD_W )
          {
            /**********************************************************/
            /* Double backslash: treat as tag to prevent user of      */
            /* modifying the backslash to a single one                */
            /**********************************************************/
            AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                        (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
            pszWork++;
            AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                        (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
            pszWork++;
          }
          else if ( pszWork[1] == HEXCODE_IDENTIFIER_W )
          {
            /**********************************************************/
            /* Hex encoded data: treat as tag                         */
            /**********************************************************/
            AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                        (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
            pszWork++;
            AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                        (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
            pszWork++;
            if ( iswxdigit(*pszWork) )
            {
              AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                          (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
              pszWork++;
            } /* endif */
            if ( iswxdigit(*pszWork) )
            {
              AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                          (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
              pszWork++;
            } /* endif */
          }
          else if ( pszWork[1] == BEGIN_GROUP_W )
          {
            /**********************************************************/
            /* Opening curly brace character: check if curly brace is */
            /* followed by a windows help tag                         */
            /**********************************************************/
            PHELPTAG  pTag;            // ptr to tag definition data
            PSZ_W pszCheck = pszWork + 2;// position to data following curly brace

            // ignore any whitespace
            while ( (*pszCheck == SPACE_W) ||
                    (*pszCheck == LF_W) ||
                    (*pszCheck == CR_W) )
            {
              pszCheck++;
            } /* endwhile */

            // check for Windows help tag
            pTag = HelpTag;
            while ( (pTag->Id != END_OF_TAGS) &&
                    (_wcsnicmp( pTag->szTagW,
                                pszCheck,
                                pTag->sLength ) != 0 ) )
            {
              pTag++;
            } /* endwhile */

            // handle help tags
            switch ( pTag->Id )
            {
              case BML_TAG :
              case BMC_TAG :
              case BMR_TAG :
                {
                  // add curly brace as inline tag
                  AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                              (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                  pszWork++;
                  AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                              (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                  pszWork++;

                  // add all data up to closing curly brace as inline tag (=PROTECTED)
                  while ( *pszWork &&
                          !( (pszWork[0] == START_CTRLWORD_W) &&
                             (pszWork[1] == END_GROUP_W) ) )
                  {
                    AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                    pszWork++;
                  } /* endwhile */

                  // add any closing curly brace as inline tag
                  if ( *pszWork )
                  {
                    AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                    pszWork++;
                    AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                    pszWork++;
                  } /* endif */
                }
                break;

              case BUTTON_TAG :
                {
                  // handle button tag; i.e. add first parameter (the label)
                  // as translatable data and the rest of the tag as
                  // non-translatable data (except for a KLink value as
                  // second parameter

                  // add curly brace as inline tag
                  AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                              (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                  pszWork++;
                  AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                              (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                  pszWork++;

                  // handle button tag name itself
                  {
                    USHORT usLen = pTag->sLength;
                    while ( usLen )
                    {
                      AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                  (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                      pszWork++;
                      usLen--;
                    } /* endwhile */
                  }

                  // ignore any whitespace
                  while ( (*pszWork == SPACE_W) ||
                          (*pszWork == LF_W) ||
                          (*pszWork == CR_W) )
                  {
                    AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                    pszWork++;
                  } /* endwhile */

                  // handle data of first parameter
                  while ( *pszWork &&
                          (*pszWork != COMMA_W) &&
                          !( (pszWork[0] == START_CTRLWORD_W) &&
                             (pszWork[1] == END_GROUP_W) ) )
                  {
                    AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                (SHORT)(pszWork - pszSegment), UNPROTECTED_CHAR );
                    pszWork++;
                  } /* endwhile */

                  // check second parameter which may be a KLINK macro with
                  // translatable text
                  if ( *pszWork == COMMA_W )
                  {
                    // add parameter delimiter as inline tag
                    AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                    pszWork++;

                    // ignore any whitespace
                    while ( (*pszWork == SPACE_W) ||
                            (*pszWork == LF_W) ||
                            (*pszWork == CR_W) )
                    {
                      AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                  (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                      pszWork++;
                    } /* endwhile */

                    // check for KLink parameter
                    if ( _wcsnicmp( L"KLink(", pszWork, 6 ) == 0 )
                    {
                      int i;
                      // add KLink parameter itself
                      for ( i = 0; i < 6; i++ )
                      {
                        AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                    (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                        pszWork++;
                      } /* endfor */

                      // handle leading whitespace
                      while ( (*pszWork == SPACE_W) ||
                              (*pszWork == LF_W) ||
                              (*pszWork == CR_W) )
                      {
                        AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                    (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                        pszWork++;
                      } /* endwhile */

                      // handle Klink data
                      while ( *pszWork &&
                              (*pszWork != END_PARMS_W) )
                      {
                        AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                    (SHORT)(pszWork - pszSegment), UNPROTECTED_CHAR );
                        pszWork++;
                      } /* endwhile */
                    } /* endif */
                  } /* endif */

                  // add rest of macro as inline tag
                  while ( *pszWork &&
                          !( (pszWork[0] == START_CTRLWORD_W) &&
                             (pszWork[1] == END_GROUP_W) ) )
                  {
                    AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                    pszWork++;
                  } /* endwhile */

                  // add tag end delimiter
                  if ( *pszWork )
                  {
                    AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                    pszWork++;
                    AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                    pszWork++;
                  } /* endif */
                }
                break;

              default :
                // treat curly brace as tag
                AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                            (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                pszWork++;
                AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                            (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                pszWork++;
                break;
            } /* endswitch */
          }
          else if ( pszWork[1] == END_GROUP_W )
          {
            /**********************************************************/
            /* End curly brace character: treat as tag                */
            /**********************************************************/
            AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                        (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
            pszWork++;
            AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                        (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
            pszWork++;
          }
          else if ( pszWork[1] == COLON_W )
          {
            /**********************************************************/
            /* Subindex identifier, treat as tag                      */
            /**********************************************************/
            AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                        (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
            pszWork++;
            AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                        (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
            pszWork++;
          }
          else
          {
            /*****************************************************/
            /* Look for end of following control word if any     */
            /*****************************************************/
            pszTemp = pszWork + 1;
            if ( RTFisAlpha( *pszTemp ) || (*pszTemp == COMMENT_TAG) )
            {
              /*************************************************/
              /* loop to end of control word                   */
              /*                                               */
              /* Note: Control words end at the first non-alpha*/
              /*       character with one exception:           */
              /*       Control word starting with \* are       */
              /*       followed by another \ before the        */
              /*       alphabetic characters of the control    */
              /*       word start                              */
              /*************************************************/
              if ( *pszTemp == COMMENT_TAG_W )
              {
                pszTemp++;
                /***********************************************/
                /* Handle control words starting with \*       */
                /***********************************************/
                if ( *pszTemp == START_CTRLWORD_W )
                {
                  pszTemp++;
                  while ( RTFisAlpha( *pszTemp ) )
                  {
                    pszTemp++;
                  } /* endwhile */
                } /* endif */
              }
              else
              {
                /***********************************************/
                /* Handle normal control words                 */
                /***********************************************/
                while ( RTFisAlpha( *pszTemp ) )
                {
                  pszTemp++;
                } /* endwhile */
              } /* endif */

              /**********************************************************/
              /* temporarly end segment at end of control word to allow */
              /* usage of stricmp for binary search in RTF tags         */
              /**********************************************************/
              chTemp = *pszTemp;
              *pszTemp = 0;

              /*************************************************/
              /* Look up control word in control word table    */
              /*************************************************/
              {
                CTRLWORD SearchedTag;

                if ( wcslen(pszWork) >= (sizeof(SearchedTag.szCtrlWordW) / 2) )
                {
                  pCtrlWord = NULL;
                }
                else
                {
                  if ( (pszWork[1] == L'*') && (pszWork[2] == '\\' ) )
                  {
                    UTF16strcpy( SearchedTag.szCtrlWordW, pszWork + 3 );
                  }
                  else
                  {
                    UTF16strcpy( SearchedTag.szCtrlWordW, pszWork + 1 );
                  } /* endif */

                  pCtrlWord = (PCTRLWORD) bsearch( &SearchedTag, CtrlWords, usNumOfTags,
                                      sizeof(CTRLWORD), StringICompareW );
                } /* endif */
              }

              /**********************************************************/
              /* Restore control word end delimiter                     */
              /**********************************************************/
              *pszTemp = chTemp;

              /**********************************************************/
              /* Special handling for some of the tags                  */
              /**********************************************************/
              if ( pCtrlWord )           // if control word has been recognized ...
              {
                switch ( pCtrlWord->rtfID )
                {
                  case ANNOTATION_RTFTAG:
                  case ATNAUTHOR_RTFTAG:
                  case ATNREF_RTFTAG:
                  case ATRFEND_RTFTAG:
                  case ATRFSTART_RTFTAG:
                  case BKMKEND_RTFTAG :
                  case BKMKSTART_RTFTAG :
                  case DOCVAR_RTFTAG:
// GQ 2002/12/16: removed REVISED_RTFTAG from list of protected text areas
//                as the revised text still needs translation...
//                  case REVISED_RTFTAG:
                     usType = PROTECTED_CHAR;
                     break;
                  case DELETED_RTFTAG:
                     usType = PROTECTED_CHAR;
                     ModeStack[sModeInd] = HIDDEN_MODE;
                     break;
                  case FIELD_RTFTAG :
                     usType = PROTECTED_CHAR;
                     ModeStack[sModeInd] = HIDDEN_MODE;
                     break;
                  case FLDRSLT_RTFTAG :                     // added 081200 bt
                     usType = UNPROTECTED_CHAR;
                     ModeStack[sModeInd] = NORMAL_MODE;
                     break;
                  case V_RTFTAG :        // hidden text
                    // check if hidden tag is followed by zero  which means
                    // that hidden mode ends here
                    if ( *pszTemp == '0' )
                    {
                      if ( ModeStack[sModeInd] == HIDDEN_MODE )
                      {
                        usType = UNPROTECTED_CHAR;
                        ModeStack[sModeInd] = NORMAL_MODE;
                      } /* endif */
                    }
                    else
                    {
                      usType = PROTECTED_CHAR;
                      ModeStack[sModeInd] = HIDDEN_MODE;
                    } /* endif */
                    break;
                  case PLAIN_RTFTAG :    // plain style, ends hidden text
                    if ( ModeStack[sModeInd] == HIDDEN_MODE )
                    {
                      usType = UNPROTECTED_CHAR;
                      ModeStack[sModeInd] = NORMAL_MODE;
                    } /* endif */
                    break;
                  //case SHPTXT_RTFTAG:
                  //  usType = UNPROTECTED_CHAR;
                  //  break;

                  // changed 31052001 bt

                  case XE_RTFTAG :    // index entry, has to be translatable
                  case TC_RTFTAG :    // toc entry, has to be translatable
                    ModeStack[sModeInd] = NORMAL_MODE;
                    usType = UNPROTECTED_CHAR;
                    break;
                  default :
                    break;
                } /* endswitch */
              } /* endif */

              /*************************************************/
              /* Process any control word data                 */
              /*                                               */
              /* Note: control words can be followed by a      */
              /*       numeric value which may be prefixed     */
              /*       with a minus sign                       */
              /*************************************************/
              if ( (*pszTemp == MINUS_W ) || iswdigit(*pszTemp) )
              {
                /***********************************************/
                /* skip numeric value                          */
                /***********************************************/
                do
                {
                  pszTemp++;
                } while ( iswdigit(*pszTemp) ); /* enddo */
              } /* endif */

              /*************************************************/
              /* process control word delimiter                */
              /*                                               */
              /* Note: If the control word is delimited by a   */
              /*       space the space is part of the control  */
              /*       word. Any other delimiter belongs to    */
              /*       the following data.                     */
              /*************************************************/
              if ( *pszTemp == SPACE_W )
              {
                pszTemp++;
              } /* endif */

              /**********************************************************/
              /* Add control word as protected table entry              */
              /**********************************************************/
              while ( pszWork < pszTemp )
              {
                AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                            (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                pszWork++;
              } /* endwhile */

              // special handling for factoidname RTF tag: add all following characters as protected until closing curly brace found
              if ( pCtrlWord && pCtrlWord->rtfID == FACTOIDNAME_RTFTAG )
              {
                while ( *pszWork && (*pszWork != END_GROUP_W) )
                {
                  AddToTable( pStartStop, &usCurEntry, usMaxEntries, (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                  pszWork++;
                } /* endwhile */
              } /* endif */
            }
            else
            {
              /*************************************************/
              /* Treat start of control word as text           */
              /*************************************************/
              AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                          (SHORT)(pszWork - pszSegment), usType );
              pszWork++;
            } /* endif */
          } /* endif */
          break;

        case BEGIN_GROUP_W:
          /**********************************************************/
          /* Add begin of group as protected table entry            */
          /**********************************************************/
          AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                      (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
          pszWork++;
          if ( sModeInd < MAX_DEST_STACK )
          {
            sModeInd++;
            ModeStack[sModeInd] = ModeStack[sModeInd-1];
          } /* endif */
          break;

        case END_GROUP_W:
          AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                      (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
          if ( sModeInd > 0 )
          {
            sModeInd--;
          } /* endif */
          if ( ModeStack[sModeInd] == NORMAL_MODE )
          {
            usType  = UNPROTECTED_CHAR;
          }
          else
          {
            usType  = PROTECTED_CHAR;
          } /* endif */
          pszWork++;
          break;

        case COLON_W:
          if ( wcscmp( pszWork, pszNONETAG_W ) == 0 )
          {
            int i = UTF16strlenCHAR(pszNONETAG_W);
            while ( i )
            {
              AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                          (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
              pszWork++;
              i--;
            } /* endwhile */
          }
          else
          {
            AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                        (SHORT)(pszWork - pszSegment), usType );
            pszWork++;
          } /* endif */
          break;

        case START_MACRO_W:
          /************************************************************/
          /* Check for special help macros: KLINK, ALINK, ...         */
          /************************************************************/
          {
            SHORT sI = 0;              // loop index

            // skip any inline tags between '!' and start of macro name
            pszStartMacro = RTFSkipInlineTagsW( pszWork+1 );

            // check macro name
            while ( (HelpMacro[sI].Id != END_OF_MACROS) &&
                    (_wcsnicmp( HelpMacro[sI].szMacroW,
                                pszStartMacro,
                                HelpMacro[sI].sLength ) != 0) )
            {
              sI++;
            } /* endwhile */

            if ( HelpMacro[sI].Id != END_OF_MACROS )
            {
              /********************************************************/
              /* Handle the macro                                     */
              /********************************************************/

              // add macro start character as not translatable text
              AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                          (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
              pszWork++;

              // add any inline tags between macro start character and macro name
              // as not translatable text
              while ( pszWork < pszStartMacro )
              {
                AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                            (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                pszWork++;
              } /* endwhile */

              // add macro name as not translatable text
              {
                SHORT sJ;
                for ( sJ = 0; sJ < HelpMacro[sI].sLength; sJ++ )
                {
                  AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                              (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                  pszWork++;
                } /* endfor */
              }

              // look for next non-whitespace character
              {
                PSZ_W pszTemp = pszWork;

                while ( (*pszTemp == SPACE_W) ||
                        (*pszTemp == LF_W)    ||
                        (*pszTemp == CR_W) )
                {
                  pszTemp++;
                } /* endwhile */

                // if followed by an opening curly brace: add whitespace
                // as not-translatable data
                if ( *pszTemp == BEGIN_GROUP_W )
                {
                  while ( pszWork < pszTemp )
                  {
                    AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                                (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                    pszWork++;
                  } /* endfor */
                } /* endif */
              }

              // handle macro parameter(s)
              if ( *pszWork == BEGIN_PARMS_W )
              {
                // copy data for macro to buffer of current help macro
                memcpy( &CurrentMacro, HelpMacro + sI, sizeof(CurrentMacro) );

                // handle parenthesis
                AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                            (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                pszWork++;

                // loop to first parameter value
                while ( *pszWork == SPACE_W )
                {
                  AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                              (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                  pszWork++;
                } /* endwhile */

                // prepare for macro parameters
                if ( sModeInd < MAX_DEST_STACK )
                {
                  sModeInd++;
                  ModeStack[sModeInd] = LINKMACRO_MODE;
                  usParmNum = 0;
                  if ( HelpMacro[sI].fTransParm[0] )
                  {
                    usType  = UNPROTECTED_CHAR;
                  }
                  else
                  {
                    usType  = PROTECTED_CHAR;
                  } /* endif */
                } /* endif */
              } /* endif */
            }
            else
            {
              /********************************************************/
              /* No valid or known macro, treat as normal text        */
              /********************************************************/

              //KA if ( (_isdbcs( *pszWork ) == DBCS_1ST) )
              // GQ: Check for DBCS leadbyte not required anymore as
              //     we are dealing with UTF16 characters

              // if (IsDBCSLeadByte((BYTE)*pszWork ) == TRUE) //KA
              // {
              //   AddToTable( pStartStop, &usCurEntry, usMaxEntries,
              //              (SHORT)(pszWork - pszSegment), usType );
              //  pszWork++;
              //} /* endif */

              if ( *pszWork )
              {
                AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                            (SHORT)(pszWork - pszSegment), usType );
                pszWork++;
              } /* endif */
            } /* endif */
          }
          break;

        default:
          /*****************************************************/
          /* Process 'normal' characters (but check for        */
          /* special characters within macro parameters)       */
          /*****************************************************/
          if ( ModeStack[sModeInd] == LINKMACRO_MODE )
          {
            // handle parmeter enclosed in double quotes
            if ( *pszWork == DOUBLEQUOTE_W )
            {
              // look if closing double quote is contained in segment
              PSZ_W pszEndDoubleQuote = wcschr( pszWork + 1, DOUBLEQUOTE_W );
              if ( pszEndDoubleQuote )
              {
                AddToTable( pStartStop, &usCurEntry, usMaxEntries, (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                pszWork++;
                while ( pszWork < pszEndDoubleQuote )
                {
                  AddToTable( pStartStop, &usCurEntry, usMaxEntries, (SHORT)(pszWork - pszSegment), usType );
                  pszWork++;
                } /* endwghile */
                AddToTable( pStartStop, &usCurEntry, usMaxEntries, (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                pszWork++;
              } /* endif */
            } /* endif */

            if ( *pszWork == COMMA_W  )
            {
              // end of a parameter detected ...

              // add end of macro parameter as inline-tag
              AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                          (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
              pszWork++;

              // add any whitespace as inline-tag
              while ( (*pszWork == SPACE_W) ||
                      (*pszWork == LF_W) ||
                      (*pszWork == CR_W) )
              {
                AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                            (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                pszWork++;
              } /* endwhile */

              // set new protect state depending on parameter number
              // and translatable style
              usParmNum++;
              if ( (usParmNum < 5) &&
                   CurrentMacro.fTransParm[usParmNum] )
              {
                usType = UNPROTECTED_CHAR;
              }
              else
              {
                usType = PROTECTED_CHAR;
              } /* endif */
            }
            else if ( *pszWork == END_PARMS_W  )
            {
              // end of a macro parameters detected ...
              if ( sModeInd > 0 )
              {
                sModeInd--;
              } /* endif */
              if ( ModeStack[sModeInd] == NORMAL_MODE )
              {
                usType  = UNPROTECTED_CHAR;
              }
              else
              {
                usType  = PROTECTED_CHAR;
              } /* endif */

              // add end of macro parameters as inline-tag
              AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                          (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
              pszWork++;
            }
            else
            {
              // normal character
              //KA if ( (_isdbcs( *pszWork ) == DBCS_1ST) )
              // GQ: Check for DBCS leadbyte not required anymore as
              //     we are dealing with UTF16 characters
              // if (IsDBCSLeadByte((BYTE)*pszWork ) == TRUE)  //KA
              // {
              //  AddToTable( pStartStop, &usCurEntry, usMaxEntries,
              //              (SHORT)(pszWork - pszSegment), usType );
              //   pszWork++;
              //} /* endif */

              if ( *pszWork )
              {
                AddToTable( pStartStop, &usCurEntry, usMaxEntries,
                            (SHORT)(pszWork - pszSegment), usType );
                pszWork++;
              } /* endif */
            } /* endif */
          }
          else
          {
            if ( ( (pszWork[0] == L'<') && RTFisAlpha(pszWork[1]) ) ||
                 ( (pszWork[0] == L'<') && (pszWork[1] == L'/') && RTFisAlpha(pszWork[2]) ) )
            {
              // add HTML-like tag as protected text
              while ( *pszWork && (*pszWork != L'>') )
              {
                AddToTable( pStartStop, &usCurEntry, usMaxEntries, (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                pszWork++;
              } /* endwhile */
              if ( *pszWork )
              {
                AddToTable( pStartStop, &usCurEntry, usMaxEntries, (SHORT)(pszWork - pszSegment), PROTECTED_CHAR );
                pszWork++;
              } /* endif */

            }
            else if ( *pszWork )
            {
              // normal character
              AddToTable( pStartStop, &usCurEntry, usMaxEntries, (SHORT)(pszWork - pszSegment), usType );
              pszWork++;
            } /* endif */
          } /* endif */
          break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /* terminate start/stop table                                     */
    /******************************************************************/
    usCurEntry++;
    pStartStop[usCurEntry].usStart = 0;
    pStartStop[usCurEntry].usStop  = 0;
    pStartStop[usCurEntry].usType  = 0;
  } /* endif */

// P017029: old approach: LF between 2 control words is a TEXT_TOKEN
// new approach: LF is protected and then "control word - LF - control word" is ONE TAG_TOKEN
// check whether this does not have un-wanted sideeffects
   RTFCheckStartStop(pszSegment, pStartStop, usCurEntry);


  *ppStartStop = pStartStop;

  return( fOK );
} /* end of function EQFPROTTABLEW */

static VOID
RTFCheckStartStop
(
	PSZ_W      pszSegment,
	PSTARTSTOP pStartStop,
	USHORT     usMaxEntry
)
{
   PSTARTSTOP  pstSource;             // active start stop indication
   PSTARTSTOP  pstTarget;
   PSTARTSTOP  pstSPlus1, pstSPlus2;
   BOOL        fCombined = FALSE;
   USHORT      usI = 0;

   pstSource = pStartStop;
   pstTarget = pStartStop;

   if ( pstSource && pstTarget )
   {
	  while ( pstSource->usType )
	  {
		 fCombined = FALSE;
		 if (pstSource->usType == PROTECTED_CHAR)
		 {
			 // is next one a TEXT_TOKEN with LF?
			 pstSPlus1 = pstSource + 1;
			 if ( pstSPlus1->usType &&
				  pstSPlus1->usType == UNPROTECTED_CHAR &&
				  pstSPlus1->usStop == pstSPlus1->usStart &&
				  *(pszSegment + pstSPlus1->usStart) == LF_W )
			 {
				 pstSPlus2 = pstSource + 2;
				 // is next one again a tag?
				 if (pstSPlus2->usType &&
					 pstSPlus2->usType == PROTECTED_CHAR)
				 {
					 // combine the 3 toks into one PROTECTED
					 pstTarget->usType  = PROTECTED_CHAR;
					 pstTarget->usStart = pstSource->usStart;
					 pstTarget->usStop  = pstSPlus2->usStop;
					 fCombined = TRUE;
				 } /* endif */
			 } /* endif */
		 } /* endif */
		 if (fCombined)
		 {
			 pstSource += 3;
			 pstTarget++;
			 usI++;
		 }
		 else
		 {
			 pstTarget->usType = pstSource->usType;
			 pstTarget->usStart = pstSource->usStart;
			 pstTarget->usStop  = pstSource->usStop;
			 pstTarget++;
			 usI++;
			 pstSource++;
		 }
	  } /* endwhile */
     //terminate startstop table
     while (pstTarget->usType && usI < usMaxEntry)
     {
	   pstTarget->usType = 0;
	   pstTarget->usStart = 0;
	   pstTarget->usStop  = 0;
	   pstTarget++;
	   usI++;
     }
    } /* endif */

  return;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     StringCompare          Interface to strcmp
//------------------------------------------------------------------------------
int StringCompare( register const void *psz1, register const void *psz2 )
{
  return( strcmp( (PSZ)psz1, (PSZ)psz2 ) );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     StringICompare          Interface to stricmp
//------------------------------------------------------------------------------
int StringICompare( register const void *psz1, register const void *psz2 )
{
  return( _stricmp( (PSZ)psz1, (PSZ)psz2 ) );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     StringICompareW          Interface to wcsicmp
//                    Compare the Unicode tag names of tw CTRLWORDs
//------------------------------------------------------------------------------
int StringICompareW( register const void *psz1, register const void *psz2 )
{
  PCTRLWORD pTag1 = (PCTRLWORD)psz1;
  PCTRLWORD pTag2 = (PCTRLWORD)psz2;
  return( _wcsicmp( pTag1->szCtrlWordW, pTag2->szCtrlWordW ) );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     AddToTable             Add character to a start/stop tbl
//------------------------------------------------------------------------------
// Description:       Adds given character to a start/stop table and starts
//                    new table entries if type of entry has changed.
//------------------------------------------------------------------------------
// Input parameter:   PSTARTSTOP pStartStop   ptr to start/stop table
//                    PUSHORT    pusCurEntry  ptr to number of current table
//                                            entry
//                    USHORT     usMaxEntries max number of table entries
//                    USHORT     usCurOffs    current offset of character
//                    USHORT     usType       type of character
//------------------------------------------------------------------------------
// Returncode type:   void
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------
static void AddToTable
(
  PSTARTSTOP       pStartStop,         // ptr to start/stop table
  PUSHORT          pusCurEntry,        // ptr to number of current table entry
  USHORT           usMaxEntries,       // max number of table entries
  USHORT           usCurOffs,          // current offset of character
  USHORT           usType              // type of character
)
{
  PSTARTSTOP       pCurEntry;          // ptr to current table entry

  pCurEntry = pStartStop + *pusCurEntry;

  if ( pCurEntry->usType != usType )
  {
    /******************************************************************/
    /* type of entry changes, complete current entry and start a new  */
    /* one (but only if current entry is not empty                    */
    /******************************************************************/
    if ( pCurEntry->usStart != usCurOffs )
    {
      /****************************************************************/
      /* Entry is not empty, so start a new one if there is still     */
      /* room in the start/stop table                                 */
      /****************************************************************/
      if ( (*pusCurEntry + 2) < usMaxEntries )
      {
        *pusCurEntry = *pusCurEntry + 1;
        pCurEntry++;
      } /* endif */
    } /* endif */
    pCurEntry->usStart = usCurOffs;
    pCurEntry->usStop  = usCurOffs;
    pCurEntry->usType  = usType;
  }
  else
  {
    pCurEntry->usStop  = usCurOffs;
  } /* endif */
} /* end of function AddToTable */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ParseRTF               Parse a RTF document
//------------------------------------------------------------------------------
// Function call:     ParseRTF( PSZ pszSource, PSZ pszTarget );
//------------------------------------------------------------------------------
// Description:       Parse a RTF document and add TM/2 segmentation tags.
//------------------------------------------------------------------------------
// Input parameter:   PSZ pszSource      fully qualified name of source file
//                    PSZ pszTarget      fully qualified name of target file
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR            function completed successfully
//                    other               OS/2 Dos calls error codes
//------------------------------------------------------------------------------
// Function flow:     initialize global parse data
//                    load slider utility module
//                    get source language and activate morphological functions
//                    get number of RTF tags and check if the table needs to be
//                       sorted
//                    Sort the RTF tag table if needed
//                    Setup TM/2 segmentation tag names
//                    open input file
//                    open output file
//                    get size of input file
//                    evaluate type of character conversion required for
//                      document
//                    repeat
//                       get next character from input file
//                       switch character
//                          case START_CTRLWORD
//                             if character is alphabetic
//                                get control word
//                                look up control word in control word table
//                                if PLAIN tag is encountered and a hidden
//                                  text group has been inserted then
//                                  add closing brace to output data
//                                endif
//                                if a footnote tag has been found then
//                                  set translatable state depending on
//                                    footnote character
//                                endif
//                                if a bookmark tag has been found then
//                                  set protected flag ON
//                                endif
//                                if a hidden text tag has been found then
//                                  set protected flag ON
//                                  insert opening brace if none found
//                                endif
//                                Set new translatable data flag if a RTF
//                                   destination tag is recognized and
//                                   move opening brace to new segment
//                                Terminate segment if a break tag was
//                                   encountered
//                                write translated control word to segment
//                                process control word data
//                                process control word delimiter
//                                force a SPACE delimiter after control word
//                             else
//                                look up symbol in control symbol table
//                                Write character for symbol to segment
//                          case BEGIN_GROUP:
//                             put current status on destination stack
//                             add character to segment
//                          case END_GROUP:
//                             add closing brace if a brace had been inserted
//                               for hidden text
//                             if a footnote is last element on dest. stack the
//                               add end of footnote as not translatable data
//                             endif
//                             if a footnote group ends then
//                               force a new segment
//                             endif
//                             get status from destination stack
//                             add character to segment
//                          case CR or LF
//                             -- ignore character
//                          default:
//                             if a code conversion table is active then
//                               convert character to PC code page
//                             endif
//                             add character to segment
//                    until EOF or errors encountered
//                    write last segment
//                    close input file
//                    close output file
//                    free memory allocated for term lists
//                    free slider module
//------------------------------------------------------------------------------
USHORT ParseRTF
(
  PSZ pszSource,                      // fully qualified name of source file
  PSZ pszTarget,                      // fully qualified name of target file
  HWND hwndProcWin,                   // handle of analysis process window
  PEQF_BOOL pfKill                    // ptr to 'kill running process' flag
)
{
  USHORT      usRC = NO_ERROR;        // function return code
  ULONG       ulNewPos;               // file pointer position
  USHORT      usOpenAction;           // action performed by DosOpen
  FILESTATUS  stStatus;               // File status information
  BYTE bCurrent;                      // currently processed byte
  USHORT      usI;                    // loop index
  BYTE        bTemp;                  // temporary character buffer
  USHORT      usNumOfTags = 0;        // number of tags in RTF tag table
  BOOL        fNeedSort = FALSE;      // TRUE = RTF tag table has to be sorted
  PCTRLWORD   pCtrlWord = NULL;       // pointer to control words
  PDESTSTACK  pDestStack = NULL;      // ptr into destination stack
  BYTE        abFootnoteChar[10];     // buffer for footnote character
  PSZ         pszTemp;                // temporary pointer
  BOOL        fProtected = FALSE;     // data-is-protected flag
  PPARSEDATA  pParseData = NULL;      // points to parser data structure
  //bt
  BOOL        fTagNewlineBreak = FALSE; // TRUE if a tag ends with newline and data follows
  DESTSTACK   CurEnv;                 // current destination environment
  UINT        uiCodePage;             // codepage
  //bt
  CHAR szLang[MAX_LANG_LENGTH];       // buffer for document target language
#if defined(STANDALONE)
  FILE        *hfileReport;           // report file
#endif

  BOOL fNeedUnprotected = FALSE;      //KA P012262
  BOOL fMACROBUTTONNoMacro = FALSE;     //KA P012262
  BOOL  fRemoveFontTag = FALSE;     // AOKI
  USHORT    usFontTagNo = 0;    // AOKI
  BOOL  fCheck1stFontTag = TRUE;   // AOKI
  USHORT    usFontTagLength = 0;       // AOKI
  PBYTE pCheckInBuf;                // AOKI
  #define MAX_FONT_TAG_LENGTH  30   // AOKI
  CHAR  szLastFontTag[MAX_FONT_TAG_LENGTH] = " ";    // AOKI
  CHAR  *pLastFontTag;              // AOKI
  pLastFontTag = szLastFontTag;     // AOKI

  CurEnv.fField      = FALSE;
  CurEnv.fTranslSegm = FALSE;
  CurEnv.rtfType     = NOSPEC_TYPE;
  CurEnv.rtfDest     = NOSPECIFIC_DEST;
  CurEnv.rtfCurDest  = NOSPECIFIC_DEST;

  /*******************************************************************/
  /* initialization                                                  */
  /*******************************************************************/
  UtlAlloc( (PVOID *) &pParseData, 0L, (LONG)sizeof(PARSEDATA), ERROR_STORAGE );
  if ( pParseData )
  {
    pParseData->Mode = TEXT_MODE;
    pParseData->hwndProcWin = hwndProcWin;
    pParseData->pfKill = pfKill;
    pDestStack = pParseData->DestStack;

    pParseData->fBkmkInSegment = FALSE;
    pParseData->sLangID = -1;
    pParseData->fProtectFldrslt = FALSE;

  }
  else
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  RTFParseGetCP(pszSource, &pParseData->ulSrcOemCP, &pParseData->ulTgtOemCP,
                           &pParseData->ulSrcAnsiCP, &pParseData->ulTgtAnsiCP);
  /*******************************************************************/
  /* Get source language and activate morphologic functions         */
  /*******************************************************************/
  if ( !usRC )
  {
    /*****************************************************************/
    /* Extract folder name from source file name                     */
    /*****************************************************************/
    strcpy( (PSZ)pParseData->abOutBuf, pszSource );
    pszTemp  = UtlGetFnameFromPath( (PSZ)pParseData->abOutBuf );
    pszTemp--;
    *pszTemp = EOS;
    pszTemp  = UtlGetFnameFromPath( (PSZ)pParseData->abOutBuf );
    pszTemp--;
    *pszTemp = EOS;
    Utlstrccpy( pParseData->szFolder,
                UtlGetFnameFromPath( (PSZ)pParseData->abOutBuf ), DOT );

    /*****************************************************************/
    /* Get document source and target language                       */
    /*****************************************************************/
    UtlMakeEQFPath( (PSZ)pParseData->abSegBuf, *pszSource, SYSTEM_PATH, NULL );
    strcat( (PSZ)pParseData->abSegBuf, BACKSLASH_STR );
    strcat( (PSZ)pParseData->abSegBuf, pParseData->szFolder );
    strcat( (PSZ)pParseData->abSegBuf, EXT_FOLDER_MAIN );
    strcat( (PSZ)pParseData->abSegBuf, BACKSLASH_STR );
    strcat( (PSZ)pParseData->abSegBuf, UtlGetFnameFromPath( pszSource ) );
    szLang[0] = EOS;
    usRC = DocQueryInfo( (PSZ)pParseData->abSegBuf, // object name of document
                         NULL,                   // translation memory or NULL
                         NULL,                   // folder format or NULL
                         pParseData->szLanguage,  // source language or NULL
                         szLang,                  // target language or NULL
                         TRUE );                 // do-message-handling flag

    // get target language dependend settings
    RTFGetSettings( "OTMRTF", szLang, pParseData );

    /*****************************************************************/
    /* set lang codepage                                             */
    /*****************************************************************/
    if (!RTFSetLangCodePage(szLang, &uiCodePage))

    //
    // DBCS containing input file can be assumed (szLang = target lang)
    // important for later handling in RTFHandleHexChar::_isdbcs
    //

    {
       CHAR szReplace[10];
       PSZ pszReplace;

       sprintf(szReplace, "%d", uiCodePage);

       pszReplace = szReplace;

       UtlError( WARNING_NO_LOCALE, MB_OK, 1, &pszReplace, EQF_WARNING );
    }

    /*****************************************************************/
    /* Activate morphologic functions                                */
    /*****************************************************************/
    if ( usRC == NO_ERROR )
    {
      usRC = MorphGetLanguageID( pParseData->szLanguage, &pParseData->sLangID );
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* get number of RTF tags and check if the table needs to be sorted*/
  /*                                                                 */
  /* Note: The RTF tag table as defined in the EQF_RTF.H file must   */
  /*       be sorted to allow binary search.                         */
  /*******************************************************************/
  if ( !usRC )
  {
    fNeedSort = FALSE;                  // assume RTF tag table is sorted
    usNumOfTags = 0;                    // start with first tag
    while ( CtrlWords[usNumOfTags].szCtrlWord[0] )
    {
      if ( usNumOfTags &&
           (strcmp( CtrlWords[usNumOfTags-1].szCtrlWord,
                    CtrlWords[usNumOfTags].szCtrlWord ) > 0 ) )
      {
        fNeedSort = TRUE;
      } /* endif */
      usNumOfTags++;
    } /* endwhile */
  } /* endif */

  /*******************************************************************/
  /* Sort the RTF tag table if needed                                */
  /*******************************************************************/
  if ( (usRC == NO_ERROR) && !*(pParseData->pfKill) )                   /* @50C */
  {
    if ( fNeedSort )
    {
      qsort( CtrlWords, usNumOfTags, sizeof(CTRLWORD), StringCompare );
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Setup tag names                                                 */
  /*                                                                 */
  /* Note: The '%d' parameter is required to insert the segment      */
  /*       number using the standard C library function sprintf.     */
  /*                                                                 */
  /* Warning: The approach below is NOT state-of-the-art! There      */
  /*          should be an utitity to to extract the tag names       */
  /*          dynamically out of the QF tag table.                   */
  /*******************************************************************/
  if ( (usRC == NO_ERROR) && !*(pParseData->pfKill) )                   /* @50C */
  {
    pParseData->pQFNTag   = ":QFN N=%d.";
    pParseData->pEQFNTag  = ":EQFN.";
    pParseData->pQFFTag   = ":QFF N=%d.";
    pParseData->pEQFFTag  = ":EQFF.";
    pParseData->SegType   = NO_SEGMENT;    // no segment is active yet
  } /* endif */

  /*******************************************************************/
  /* open input file                                                 */
  /*******************************************************************/
  if ( !usRC && !*(pParseData->pfKill) )                                /* @50C */
  {
    usRC = UtlOpen( pszSource,
                    &pParseData->hInFile,
                    &usOpenAction, 0L,
                    FILE_NORMAL,
                    FILE_OPEN,
                    OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                    0L,
                    TRUE );
  } /* endif */

  /*******************************************************************/
  /* open output file                                                */
  /*******************************************************************/
  if ( !usRC && !*(pParseData->pfKill) )                                /* @50C */
  {
    usRC = UtlOpen( pszTarget,
                    &pParseData->hOutFile,
                    &usOpenAction, 0L,
                    FILE_NORMAL,
                    FILE_TRUNCATE | FILE_CREATE,
                    OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                    0L,
                    TRUE );
    if ( !usRC )
    {
      // write Unicode prefix to output file
      ULONG ulBytesWritten = 0;
      UtlWriteL( pParseData->hOutFile, UNICODEFILEPREFIX,
                 (ULONG)strlen(UNICODEFILEPREFIX), &ulBytesWritten, TRUE );
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* get size of input file                                          */
  /*******************************************************************/
  if ( !usRC && !*(pParseData->pfKill) )                                /* @50C */
  {
    usRC = UtlQFileInfo( pParseData->hInFile,
                         1,
                         (PBYTE)&stStatus,
                         sizeof(FILESTATUS),
                         TRUE );
    pParseData->lTotalBytes  = stStatus.cbFile;
    pParseData->lBytesToRead = stStatus.cbFile;
  } /* endif */

  /*******************************************************************/
  /* Read first part of text to buffer to get type of code conversion*/
  /* required                                                        */
  /*                                                                 */
  /* Note: RTF documents always start with the tag \rtf1 followed by */
  /*       a tag defining the codepage used for the document.        */
  /*       These tags are \ansi or \pc or \mac. If the \ansi codepage*/
  /*       is used the chAnsiToPC code page will be used to convert  */
  /*       the ANSI characters to the PC-850 code page. Currently    */
  /*       there is no support for /mac documents nor for other      */
  /*       codepages as PC-850.                                      */
  /*******************************************************************/
  if ( !usRC && !*(pParseData->pfKill) )
  {
    /****************************************************************/
    /* Read first 512 bytes from input file                         */
    /****************************************************************/
    USHORT usRead = (USHORT)min( pParseData->lTotalBytes, 512L );
    usRC = UtlRead( pParseData->hInFile, pParseData->abInBuf,
                    usRead, &pParseData->usBytesInBuffer, TRUE );
    pParseData->abInBuf[usRead] = EOS;
    pParseData->usBytesInBuffer = 0;

    // reset Ansi->ASCII conversion tables
    pParseData->pOrgConvTable = NULL;
    pParseData->pConvTable = NULL;

    if ( usRC == NO_ERROR )
    {
      // Get type of code conversion
      if ( RTFContainsAnsiTag( (PSZ)pParseData->abInBuf ) )
      {
        UtlQueryCharTableEx( ANSI_TO_ASCII_TABLE, (PUCHAR *)&(pParseData->pConvTable),
                             (USHORT)pParseData->ulSrcOemCP );
        pParseData->pOrgConvTable = pParseData->pConvTable;
        if ( IsAPLang(szLang) )
        {
         // in a DBCS environment no codepage conversion should be done.
          pParseData->pConvTable = NULL;
        } /* endif */
      } /* endif */
    } /* endif */

    /****************************************************************/
    /* Position file read pointer back to begin of file             */
    /****************************************************************/
    UtlChgFilePtr( pParseData->hInFile,
                   0L,
                   FILE_BEGIN,
                   &ulNewPos,
                   FALSE );
  } /* endif */

  /*******************************************************************/
  /* Main loop for parsing the RTF document                          */
  /*******************************************************************/
  if ( !usRC && !*(pParseData->pfKill) )                                /* @50C */
  {
    do
    {
      /*************************************************************/
      /* Get next character from document                          */
      /*************************************************************/
      bCurrent = ParseNextChar( pParseData, &usRC );

      /*************************************************************/
      /* See what we have to do with the character                 */
      /*************************************************************/
      if ( !usRC && !*(pParseData->pfKill) )                          /* @50C */
      {
        switch ( bCurrent )
        {
          case START_CTRLWORD:
            if ( RTFGetControlWord( pParseData, &usRC, &bCurrent ) )
            {

                /***********************************************************/
                /* Special handling for unnecessary font tags.(P011402) AOKI*/
                /***********************************************************/
                /* <summary of P011402>                                    */
                /* Word J(DBCS) defines control words to specify composite */
                /* font as associated character properties. These control  */
                /* words follow the rule of associated character properties*/
                /* and understand font designation(\af).                   */
                /*  - \hich\af&<aprops>\dbch\af&<aprops>\loch<ptext>       */
                /*  - \loch\af&<aprops>\dbch\af&<aprops>\hich<ptext>       */
                /*  - \loch\af&<aprops>\hich\af&<aprops>\dbch<ptext>       */
                /*                                                         */
                /* TM uses MS Word's function to convert DOC to RTF. But,  */
                /* there are many same font tags above in the created RTF. */
                /* (Though it is MS Word's problem,) To improve translator */
                /* work, these unnecessary font tags should be removed.    */
                /*                                                         */
                /* <variable>                                              */
                /* fRemoveFontTag: TRUE - font tag should be removed.      */
                /* fCheck1stFontTag: TRUE - need to check 1st font tag.    */
                /* szLastFontTag: buffer of last font tags string.         */
                /*                                                         */
                /* <logic>                                                 */
                /*  step-1: Checking coming control word string.           */
                /*     IF coming string is same as previous one in szLastFontTag*/
                /*       THEN fRemoveFontTag=TRUE                          */
                /*       ELSE IF it is font tag string                     */
                /*              THEN copy it into szLastFontTag            */
                /*                                                         */
                /*  step-2 (1/2): It is NOT unnecessary font tag.          */
                /*       initialize to continue to normal procedure        */
                /*                                                         */
                /*  step-2 (2/2): It is just unnecessary font tag.         */
                /*       remove coming control word (font tag)             */
                /*                                                         */
                /***********************************************************/
                if (!fRemoveFontTag)
                {
                    if ( (fCheck1stFontTag) &&
                         ((strcmp(pParseData->szControlWord,"loch") == 0) ||
                          (strcmp(pParseData->szControlWord,"hich") == 0)) )

                    /***********************************************************/
                    /*  1st step:  check whether coming control word string is */
                    /*              unnecessary font tag string.               */
                    /***********************************************************/
                    {
                        if (strncmp( (PSZ)pParseData->pInBuf,pLastFontTag,strlen(szLastFontTag)) == 0)
                        {
                            fRemoveFontTag = TRUE;
                            usFontTagNo = 0;
                        }
                        else
                        {
                            pCheckInBuf = pParseData->pInBuf;
                            usFontTagLength = 0;

                            do
                            {
                                usFontTagLength++;
                                pCheckInBuf++;
                            } while ( (*pCheckInBuf != '\\') && (isalnum(*pCheckInBuf)) ); // "\af..", "\f.."

                            if (*pCheckInBuf == '\\')
                            {
                                do
                                {
                                    usFontTagLength++;
                                    pCheckInBuf++;
                                } while ( (*pCheckInBuf != '\\') && (isalnum(*pCheckInBuf)) ); // "\dbch", "\hich"

                                if (*pCheckInBuf == '\\')
                                {
                                    do
                                    {
                                        usFontTagLength++;
                                        pCheckInBuf++;
                                    } while ( (*pCheckInBuf != '\\') && (isalnum(*pCheckInBuf)) ); // "af..", "\f.."

                                    if (*pCheckInBuf == '\\')
                                    {
                                        do
                                        {
                                            usFontTagLength++;
                                            pCheckInBuf++;
                                        } while ( (*pCheckInBuf != '\\') && (isalnum(*pCheckInBuf)) ); // "\loch", "\hich", "\dbch"

                                        if (*pCheckInBuf == '\\')
                                        {
                                            do
                                            {
                                                usFontTagLength++;
                                                pCheckInBuf++;
                                            } while (isalnum(*pCheckInBuf)); // "\af..", "\f.."

                                            if (usFontTagLength < MAX_FONT_TAG_LENGTH)
                                            {
                                                strncpy( pLastFontTag, (PSZ)pParseData->pInBuf,usFontTagLength);
                                                *(pLastFontTag + usFontTagLength) = EOS;
                                                fCheck1stFontTag = FALSE;
                                                usFontTagNo = 0;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (!fCheck1stFontTag)
                    /***********************************************************/
                    /* 2nd step(1/2): control word is not unnecessary font tag.*/
                    /***********************************************************/
                    {

                      if ((strcmp(pParseData->szControlWord,"loch") != 0) &&
                          (strcmp(pParseData->szControlWord,"hich") != 0) &&
                          (strcmp(pParseData->szControlWord,"dbch") != 0) &&
                          (strcmp(pParseData->szControlWord,"af") != 0) &&
                          (strcmp(pParseData->szControlWord,"f") != 0))
                      {
                        fCheck1stFontTag = TRUE;
                      }
                      else
                      {

                        if ((usFontTagNo == 0) &&
                            ((strcmp(pParseData->szControlWord,"hich") == 0) ||
                             (strcmp(pParseData->szControlWord,"loch") == 0)) )
                        {
                          usFontTagNo = 1;
                        }

                        else if ((usFontTagNo == 1) &&
                                ((strcmp(pParseData->szControlWord,"dbch") == 0) ||
                                 (strcmp(pParseData->szControlWord,"hich") == 0)) )
                        {
                          usFontTagNo = 2;
                        }

                        else if ((usFontTagNo == 2) &&
                                ((strcmp(pParseData->szControlWord,"loch") == 0) ||
                                 (strcmp(pParseData->szControlWord,"hich") == 0) ||
                                 (strcmp(pParseData->szControlWord,"dbch") == 0)) )
                        {
                            usFontTagNo = 3;
                        }

                        if (usFontTagNo == 3)
                        {

                          usFontTagNo = 0;
                          fCheck1stFontTag = TRUE;
                        }

                      }

                    } // end if (!fCheck1stFontTag)

                }  // end if (!fRemoveFontTag)

                if (fRemoveFontTag)
                /***********************************************************/
                /* 2nd step(2/2): control word is unnecessary font tag.    */
                /***********************************************************/
                {

                  switch (usFontTagNo)
                  {
                    case 0: // 1st font tag: (\loch\af..), (\hich\af..)
                      usFontTagNo = 1;
                      break;

                    case 1: // 2nd font tag: (\hich\af..), (\dbch\af..)
                      usFontTagNo = 2;
                      break;

                    case 2: // 3rd font tag: (\loch\af..), (\hich\af..), (\dbch\af..)

                      usFontTagNo = 3;
                      break;

                  default:
                      break;

                  }

                  // skipp \afxx, \fxx
                  do
                  {
                    bCurrent = ParseNextChar( pParseData, &usRC );
                  }
                  while (!usRC && (bCurrent != START_CTRLWORD) && isalnum(bCurrent));

                  if (bCurrent != SPACE)
                      UndoChar( pParseData, bCurrent );

                  // finish ?
                  if (usFontTagNo == 3)
                  {
                      usFontTagNo = 0;
                      fRemoveFontTag = FALSE;
                      fCheck1stFontTag = TRUE;
                  }

                  continue;

                } // end if (fRemoveFontTag)

              /*************************************************/
              /* Look up control word in control word table    */
              /*************************************************/

              {
                PSZ pszControlWord = pParseData->szControlWord;
                if ( (pszControlWord[0] == '*') && (pszControlWord[1] == '\\' ) )
                {
                  pszControlWord += 2;   // skip prefix
                } /* endif */
                pCtrlWord = (PCTRLWORD) bsearch( pszControlWord ,
                                    CtrlWords, usNumOfTags,
                                    sizeof(CTRLWORD), StringICompare );
              }

              /*************************************************/
              /* End hidden text if a plain text is encounterd */
              /*************************************************/
              if ( pCtrlWord && (pCtrlWord->rtfID == PLAIN_RTFTAG) )
              {
                fProtected = FALSE;
              } /* endif */

              /********************************************************/
              /* Special handling for FOOTNOTE tags:                  */
              /********************************************************/
              /*   set translatable flag depending on                 */
              /* footnote character which prefixed the footnote       */
              /*                                                      */
              /* Normal footnote syntax is                            */
              /*   K{\footnote ...                                    */
              /*                                                      */
              /* Some RTF generating programs prefer a footnote       */
              /* syntax of                                            */
              /*   {\up K}{\footnote K                                */
              /********************************************************/
              if ( pCtrlWord && (pCtrlWord->rtfID == FOOTNOTE_RTFTAG) )
              {
                usRC = RTFHandleFootNoteTag( pParseData, &CurEnv, pCtrlWord );
              } /* endif */

              /********************************************************/
              /* Skip BIDI tagging                                    */
              /********************************************************/

              if ( pCtrlWord &&
                 pParseData->fBidi &&
                (pCtrlWord->rtfID == LTRCH_RTFTAG  ||
                 pCtrlWord->rtfID == RTLCH_RTFTAG  ||
                 pCtrlWord->rtfID == LTRPAR_RTFTAG ||
                 pCtrlWord->rtfID == RTLPAR_RTFTAG) )

              {
                // bt 310101
                // skip complete tag until next space character
                // we don't expect import files with BIDI tagging

                do
                {
                            bCurrent = ParseNextChar( pParseData, &usRC );
                }
                while (!usRC && (bCurrent == START_CTRLWORD || isalnum(bCurrent)));

                if ( bCurrent != SPACE)
                  UndoChar( pParseData, bCurrent );

                continue;
              }

              /********************************************************/
              /* Special handling for BKMK.... tags:                  */
              /********************************************************/
              /*   set fProtected flag ON                             */
              /********************************************************/
              if ( pCtrlWord &&
                   ( (pCtrlWord->rtfID == BKMKEND_RTFTAG ) ||
                     (pCtrlWord->rtfID == BKMKSTART_RTFTAG) ) )
              {
                pParseData->fBkmkInSegment = TRUE;
                fProtected = TRUE;
              } /* endif */

              /********************************************************/
              /* Special handling for hidden text flag V:             */
              /********************************************************/
              /*   set fProtect flag ON                               */
              /********************************************************/
              if ( pCtrlWord &&
                   ( pCtrlWord->rtfID == V_RTFTAG ) )
              {
                // start hidden text or end hidden text if followed by zero
                if ( bCurrent == '0' )
                {
                  fProtected = FALSE;
                }
                else
                {
                  fProtected = TRUE;
                } /* endif */
              } /* endif */

              // switch to protected text mode if deleted text is encountered
              if ( pCtrlWord && ( pCtrlWord->rtfID == DELETED_RTFTAG ) )
              {
                fProtected = FALSE;
              } /* endif */

              /********************************************************/
              /* Set new translatable data flag if a RTF              */
              /* destination tag is recognized                        */
              /********************************************************/

              if ( pCtrlWord &&
                   (pCtrlWord->Type == DEST_T || pCtrlWord->Type == FINL_T) &&
                   (pCtrlWord->rtfID != FOOTNOTE_RTFTAG))
              {

                /******************************************************/
                /* Set new translatable flag value                    */
                /******************************************************/
                CurEnv.fTranslSegm = pCtrlWord->usValue;
                fProtected = FALSE;

                if ( pCtrlWord->Type == FINL_T )
                {
                   CurEnv.fTranslSegm = TRUE;
                   fProtected = TRUE;

                   if (pCtrlWord->rtfID == FLDRSLT_RTFTAG )
                   {
                     if (pParseData->fProtectFldrslt == TRUE)
                     {
                        //
                        // Protect the contents of fldrslt if there
                        // was a \*\fldinst field instruction with
                        // special keywords (such as TOC, index)
                        //

                        CurEnv.fTranslSegm = FALSE;
                        fProtected = TRUE;
                     }
                     else
                     {
                        fProtected = FALSE;
                     }
                   }
                }

                /******************************************************/
                /* Set current destination                            */
                /******************************************************/
                CurEnv.rtfCurDest = pCtrlWord->rtfID;
                CurEnv.rtfType = pCtrlWord->Type;

                /******************************************************/
                /* Move opening curly brace to new segment            */
                /******************************************************/
                if ( pParseData->usOutBufUsed   &&
                     pParseData->pOutBuf[-1]  == BEGIN_GROUP)
                {
                  pParseData->usOutBufUsed -= 1;
                  pParseData->pOutBuf      -= 1;
                  pParseData->pSegBuf      -= 1;
                  pParseData->pCharBuf     -= 1;
                  usRC = AddToSegment( pParseData, BEGIN_GROUP,
                                       CurEnv.fTranslSegm, CH_TAG );
                } /* endif */
              } /* endif */

              /*************************************************/
              /* Terminate segment if a break tag was          */
              /* encountered                                   */
              /*************************************************/
              if ( !usRC && pCtrlWord && (pCtrlWord->Type == BREAK_T) )
              {
                usRC = RTFWriteSegment( pParseData );
                // do not reset hidden mode if a segment break is detected
                // fProtected = FALSE;
                // do not reset
              } /* endif */

              /*************************************************/
              /* Write translated control word to segment      */
              /*************************************************/
              if ( !usRC )
              {
                usRC = AddToSegment( pParseData, START_CTRLWORD,
                                     CurEnv.fTranslSegm, CH_TAG );
              } /* endif */
              usI = 0;
              while ( !usRC && pParseData->szControlWord[usI] )
              {
                usRC = AddToSegment( pParseData,
                                     pParseData->szControlWord[usI++],
                                     CurEnv.fTranslSegm, CH_TAG );
              } /* endwhile */

              /*************************************************/
              /* Process any control word data                 */
              /*                                               */
              /* Note: control words can be followed by a      */
              /*       numeric value which may be prefixed     */
              /*       with a minus sign                       */
              /*************************************************/
              if ( !usRC &&
                   ( (bCurrent == MINUS) ||
                     (bCurrent == LF)    ||
                     (bCurrent == CR)    ||
                     isdigit(bCurrent)) )
              {
                /***********************************************/
                /* add numeric value to segment                */
                /***********************************************/


                fTagNewlineBreak = FALSE;

                do
                {
                  if ( (bCurrent != LF) && (bCurrent != CR) )
                  {
                    if (!fTagNewlineBreak)
                    {
                       usRC = AddToSegment( pParseData, bCurrent,
                                         CurEnv.fTranslSegm, CH_TAG );
                    }
                    else
                    {
                           // tag that ends with \n has to be delimited
                           // added 1.17 bt

                       //usRC = AddToSegment( pParseData,
                       //              SPACE, CurEnv.fTranslSegm, CH_TAG );
                       break;
                    }

                  }
                  else
                  {        // added 1.17 bt
                    fTagNewlineBreak = TRUE;
                  }
                  bCurrent = ParseNextChar( pParseData, &usRC );
                } while (!usRC && isdigit(bCurrent)); // changed 1.17 bt
              } /* endif */

              /*************************************************/
              /* process control word delimiter                */
              /*                                               */
              /* Note: If the control word is delimited by a   */
              /*       space the space is part of the control  */
              /*       word. Any other delimiter belongs to    */
              /*       the following data.                     */
              /*************************************************/
              if ( !usRC &&
                   ((bCurrent == SPACE) ||
                    (bCurrent == LF) ||
                    (bCurrent == CR) ) )
              {
                usRC = AddToSegment( pParseData,
                                     SPACE, CurEnv.fTranslSegm, CH_TAG );
              }
              else
              {
                // special handling for unicode encoded characters without following
                // tag delimiter: check if tag is followed by a hex encoded space character
                // (\'20) as this may not be converted to a space character or the resolve unicode
                // function during POSTSEGMENTATION will misinterpret the space as part of
                // the Unicode tag and will remove the next character instead
                if ( pCtrlWord && (pCtrlWord->rtfID == U_RTFTAG) &&
                     (bCurrent == START_CTRLWORD) &&
                     (strncmp( (PSZ)pParseData->pInBuf, "\'20", 3 ) == 0) )
                {
                  int i  = 4;

                  while ( !usRC && (i != 0) )
                  {
                    usRC = AddToSegment( pParseData, bCurrent, CurEnv.fTranslSegm, CH_TAG );
                    if ( !usRC ) bCurrent = ParseNextChar( pParseData, &usRC );
                    i--;
                  } /* endwhile */
                } /* endif */

                // undo last character which does not belong to current tag
                UndoChar( pParseData, bCurrent );
              } /* endif */

              /*************************************************/
              /* Handle (=skip) data of specific destinations  */
              /* and add only a reference to the data          */
              /*************************************************/
              if ( pCtrlWord &&
                   ( (pCtrlWord->rtfID == PICT_RTFTAG)       ||
                     (pCtrlWord->rtfID == STYLESHEET_RTFTAG) ||
                     (pCtrlWord->rtfID == COLORTBL_RTFTAG)   ||
                     (pCtrlWord->rtfID == INFO_RTFTAG)       ||
                     (pCtrlWord->rtfID == OBJDATA_RTFTAG)    ||
                     (pCtrlWord->rtfID == LISTTABLE_RTFTAG)  ||
                     (pCtrlWord->rtfID == LISTOVERRIDETABLE_RTFTAG)  ||
                     (pCtrlWord->rtfID == COLORSCHEMEMAPPING_RTFTAG) ||
                     (pCtrlWord->rtfID == DATASTORE_RTFTAG) ||
                     (pCtrlWord->rtfID == THEMEDATA_RTFTAG) ||
                     (pCtrlWord->rtfID == FONTTBL_RTFTAG) ) )
              {
                usRC = RTFDataToRef( pParseData );
              } /* endif */
            }
            else if ( !usRC )
            {
              /*************************************************/
              /* Look up symbol in control symbol table        */
              /*************************************************/
              switch ( CtrlSymbols[bCurrent].SymbType )
              {
                case CHAR_SYMB:
                  /*********************************************/
                  /* Write character for symbol to segment     */
                  /*********************************************/
                  bTemp = CtrlSymbols[bCurrent].chTarget;
                  if ( !usRC && (bTemp != EOS) )
                  {
                    usRC = AddToSegment( pParseData, bTemp, CurEnv.fTranslSegm,
                                         (fProtected) ? CH_TAG :
                                                        TranslChar[bTemp] );
                  } /* endif */
                  break;

                case HEX_SYMB:
                  /*********************************************/
                  /* Get hexadezimal coded character           */
                  /*********************************************/
                  {
                    BOOL fLeaveHexCharAsIs = FALSE;
                    if ( (CurEnv.rtfCurDest == FONTTBL_RTFTAG) ||
                         (CurEnv.rtfCurDest == FCHARS_RTFTAG) ||
                         (CurEnv.rtfCurDest == LCHARS_RTFTAG) )
                    {
                      fLeaveHexCharAsIs = TRUE;
                    } /* endif */

                    usRC = RTFHandleHexChar( pParseData, CurEnv.fTranslSegm,
                                             fProtected,
                                             bCurrent, fLeaveHexCharAsIs );
                  }
                  break;

                case NO_SYMB :
                default :
                  if ( bCurrent == BEGIN_GROUP )
                  {
                    usRC = RTFCheckAndHandleHelpTag( pParseData, CurEnv.fTranslSegm,
                                                     fProtected );
                  }
                  else
                  {
                    /*********************************************/
                    /* Write unrecognized data to segment        */
                    /*********************************************/
                    if ( !usRC )
                    {
                      usRC = AddToSegment( pParseData, START_CTRLWORD,
                                           CurEnv.fTranslSegm,
                                      (fProtected) ? CH_TAG : CH_NO );
                    } /* endif */
                    if ( !usRC )
                    {
                      usRC = AddToSegment( pParseData, bCurrent, CurEnv.fTranslSegm,
                                      (fProtected) ? CH_TAG : CH_NO );
                    } /* endif */
                  } /* endif */
                  break;
              } /* endswitch */
            } /* endif */
            break;

          case BEGIN_GROUP:
            /*****************************************************/
            /* The start of a new RTF group has been encountered */
            /*****************************************************/
            if ( pDestStack < (pParseData->DestStack + MAX_DEST_STACK) )
            {
              //bt
              //if (pCtrlWord && pCtrlWord->Type == FINL_T)
              if (CurEnv.rtfType == FINL_T)
                 CurEnv.fProtected = fProtected;
              else
                 CurEnv.fProtected = FALSE;

              memcpy( pDestStack, &CurEnv, sizeof(DESTSTACK) );
              pDestStack++;
              CurEnv.rtfDest = NOSPECIFIC_DEST;
            } /* endif */
            usRC = AddToSegment( pParseData, bCurrent,
                                 CurEnv.fTranslSegm, CH_TAG );
            break;

          case END_GROUP:
            /*****************************************************/
            /* The end of a RTF control group has been           */
            /* encountered                                       */
            /*****************************************************/

            /***************+**************************************/
            /* Special handling for footnote control groups      */
            /*****************************************************/
            if ( !usRC )
            {
              if ( (pDestStack > pParseData->DestStack) &&
                   ((pDestStack-1)->rtfDest == FOOTNOTE_RTFTAG ) )
              {
                if ( (pParseData->usOutBufUsed >= 2 )         &&
                     ( (pParseData->pOutBuf[-2]  == SPACE) ||
                       (pParseData->pOutBuf[-2]  == CR)    ||
                       (pParseData->pOutBuf[-2]  == LF) ) )
                {
                  /*********************************************/
                  /* Add end of footnote as not translatable   */
                  /* data.                                     */
                  /*********************************************/
                  abFootnoteChar[0] = pParseData->pOutBuf[-2];
                  abFootnoteChar[1] = pParseData->pOutBuf[-1];
                  pParseData->usOutBufUsed -= 2;
                  pParseData->pOutBuf      -= 2;
                  pParseData->pSegBuf      -= 2;
                  pParseData->pCharBuf     -= 2;
                  pParseData->fTranslDataInSegment =
                     RTFCheckForTranslData( pParseData );
                  if ( !usRC )
                  {
                    usRC = AddToSegment( pParseData, abFootnoteChar[0],
                                         CurEnv.fTranslSegm, CH_TAG );
                  } /* endif */
                  if ( !usRC )
                  {
                    usRC = AddToSegment( pParseData, abFootnoteChar[1],
                                         CurEnv.fTranslSegm, CH_TAG );
                  } /* endif */
                  if ( !usRC )
                  {
                    usRC = AddToSegment( pParseData, bCurrent, CurEnv.fTranslSegm,
                                         CH_TAG );
                  } /* endif */
                }
                else
                {
                  usRC = AddToSegment( pParseData, bCurrent,
                                       CurEnv.fTranslSegm, CH_TAG );
                } /* endif */
              }
              else
              {
                usRC = AddToSegment( pParseData, bCurrent,
                                     CurEnv.fTranslSegm, CH_TAG );
              } /* endif */

              /********************************************************/
              /* End the current segment if we are leaving the        */
              /* footnote group                                       */
              /********************************************************/
              if ( (pDestStack > pParseData->DestStack) &&
                   ((pDestStack-1)->rtfDest != FOOTNOTE_RTFTAG ) &&
                   (CurEnv.rtfDest == FOOTNOTE_RTFTAG) )
              {
                usRC = RTFWriteSegment( pParseData );
              } /* endif */
            } /* endif */

            /*****************************************************/
            /* Restore environment which was active outside the  */
            /* group currently ended                             */
            /*****************************************************/
            if ( pDestStack > pParseData->DestStack )
            {
              // restore flag for next \fldinst field instruction if we're ending
              // a fldrslt group

              if (CurEnv.rtfCurDest == FLDRSLT_RTFTAG)
              {
                 pParseData->fProtectFldrslt = FALSE;
              }

              pDestStack--;
              memcpy( &CurEnv, pDestStack, sizeof(DESTSTACK) );

              // fProtected = FALSE; // why?
              fProtected = CurEnv.fProtected;
            } /* endif */
            break;

          case START_MACRO:
            pParseData->fMacroMayFollow = TRUE;

            /********************************************************************/
            /* Write hotspot start character to current segment                  */
            /********************************************************************/
            usRC = AddToSegment( pParseData, bCurrent, CurEnv.fTranslSegm,
                                 (fProtected) ? CH_TAG : TranslChar[bCurrent] );

            /**********************************************************/
            /* Process any following macro                            */
            /**********************************************************/
            if ( usRC == NO_ERROR )
            {
              usRC = RTFParseMacro( pParseData, &bCurrent, fProtected,
                               CurEnv.fTranslSegm, EOS );
              if ( !pParseData->fMacroMayFollow )
              {
                // macro has been found and processed, check if more macros are
                // to follow (seperated by a semicolon)
                PSZ pszTemp = (PSZ)pParseData->pInBuf;
                while ( *pszTemp == SPACE ) pszTemp++;
                if ( *pszTemp == SEMICOLON )
                {
                  pParseData->fMacroMayFollow = TRUE;
                } /* endif */
              }
            } /* endif */
            break;


          case CR:
          case LF:
            /******************************************************/
            /* ignore carriage return and linefeed                */
            /*                                                    */
            /* Note: CR and LF are inserted into RTF documents    */
            /*       at vals. These characters have    */
            /*       no other meanfixed intering as to break the document    */
            /*       into smaller data blocks.                    */
            /******************************************************/
            break;

          case SEMICOLON:
            // pParseData->fMacroMayFollow = FALSE; GQ: removed as semicolon is seperator for list of macros
            if ( (CurEnv.rtfDest == FOOTNOTE_RTFTAG) &&
                 (pParseData->bCurrFootNoteChar == 'K') )
            {
              /********************************************************/
              /* treat semicolon in keyword lists as segment break    */
              /*                                                      */
              /* write current segment and start a new one, add       */
              /* semicolon and any following whitespace character     */
              /* as not-translatable data                             */
              /********************************************************/

              // close current segment
              usRC = RTFWriteSegment( pParseData );

              // add semikolon as protected character
              if ( usRC == NO_ERROR )
              {
                usRC = RTFNormalChar( pParseData, &bCurrent, fProtected,
                                      FALSE );
              } /* endif */

              // test for following whitespace characters
              if ( usRC == NO_ERROR )
              {
                bCurrent = ParseNextChar( pParseData, &usRC );
                while ( (usRC == NO_ERROR) &&
                        ( (bCurrent == SPACE) || (bCurrent == LF) || (bCurrent == CR) ) )
                {
                  if ( bCurrent == SPACE )
                  {
                    usRC = AddToSegment( pParseData, bCurrent,
                                         FALSE, CH_TAG );
                  } /* endif */
                  if ( usRC == NO_ERROR )
                  {
                    bCurrent = ParseNextChar( pParseData, &usRC );
                  } /* endif */
                } /* endwhile */
                if ( usRC == NO_ERROR )
                {
                  UndoChar( pParseData, bCurrent );
                } /* endif */
              } /* endif */
            }
            else
            {
              usRC = RTFNormalChar( pParseData, &bCurrent, fProtected,
                                    CurEnv.fTranslSegm );
            } /* endif */
            break;

          default:
            if ( pParseData->fMacroMayFollow )     // could be start of a macro
            {
              // maybe start of a macro
              usRC = RTFParseMacro( pParseData, &bCurrent, fProtected, CurEnv.fTranslSegm, bCurrent );
              if ( pParseData->fMacroMayFollow )  // no macro found ???
              {
                // normal characters stop macro recognition
                if ( bCurrent != SPACE )
                {
                  pParseData->fMacroMayFollow = FALSE;
                } /* endif */

                // process normal character
                usRC = RTFNormalChar( pParseData, &bCurrent, fProtected,
                                      CurEnv.fTranslSegm );
              }
              else
              {
                // macro has been found and processed, check if more macros are
                // to follow (seperated by a semicolon)
                {
                  PSZ pszTemp = (PSZ)pParseData->pInBuf;
                  while ( *pszTemp == SPACE ) pszTemp++;
                  if ( *pszTemp == SEMICOLON )
                  {
                    pParseData->fMacroMayFollow = TRUE;
                  } /* endif */
                }
              } /* endif */
            }
            else
            {
              //
              // process normal character
              //

              // scan for keywords to handle fldrslt Protection
              // watch backwards on the stack

              if (CurEnv.rtfCurDest == FLDINST_RTFTAG || (pDestStack-1)->rtfCurDest == FLDINST_RTFTAG)
              {
                 register int i;

                 if (!pParseData->fProtectFldrslt)
                 {
                     for (i = 0, pParseData->fProtectFldrslt = FALSE; strlen(FldinstTagKeyWordsProtected[i]); i ++)
                     {
                        if (!_memicmp(pParseData->pInBuf-1, FldinstTagKeyWordsProtected[i],
                           strlen(FldinstTagKeyWordsProtected[i])))
                        {
                           pParseData->fProtectFldrslt = TRUE;
                           break;
                        }
                    }
                 }
              }

              if (fNeedUnprotected)                          //KA P012262
              {                                              //KA
                if (strncmp((PSZ)pParseData->pInBuf-1,"]}",2) == 0) //KA
                {                                            //KA
                     fNeedUnprotected = FALSE;               //KA
                     if (!fProtected)                        //KA
                     {                                       //KA
                         fProtected = TRUE;                  //KA
                     }                                       //KA
                }                                            //KA
                else                                         //KA
                {                                            //KA
                     fProtected = FALSE;                     //KA
                }                                            //KA
              }                                              //KA

              usRC = RTFNormalChar( pParseData, &bCurrent, fProtected,
                                    CurEnv.fTranslSegm );

              if ((!fMACROBUTTONNoMacro && !fNeedUnprotected) &&
                  (strncmp((PSZ)(pParseData->pInBuf-1),"MACROBUTTON NoMacro [",21) == 0)) //KA
              {                                              //KA
                  fMACROBUTTONNoMacro = TRUE;                //KA
              }                                              //KA
              if (fMACROBUTTONNoMacro && (bCurrent == '['))  //KA
              {                                              //KA
                  fNeedUnprotected = TRUE;                   //KA
                  fMACROBUTTONNoMacro = FALSE;               //KA
              }                                              //KA

              // reset macro may follow flag for non whitespace characters only
              if ( bCurrent != SPACE )
              {
                pParseData->fMacroMayFollow = FALSE;   // no macro to follow anymore
              } /* endif */
            } /* endif */

            break;
        } /* endswitch */
      } /* endif */
    } while ( !usRC && !*(pParseData->pfKill) ); /* enddo */

    if ( usRC == EOF_REACHED )
    {
      usRC = 0;
    } /* endif */

    if ( !usRC && !*(pParseData->pfKill) )
    {
      usRC = RTFWriteSegment( pParseData );
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Cleanup: close any open file, free allocated memory and free    */
  /*          loaded modules (DLLs)                                  */
  /*******************************************************************/
  if ( pParseData )
  {
    if ( pParseData->sLangID != -1 ) MorphFreeLanguageID( pParseData->sLangID );

    if ( pParseData->hOutFile )
    {
      UtlClose( pParseData->hOutFile, TRUE );
    } /* endif */

    if ( pParseData->hInFile )
    {
      UtlClose( pParseData->hInFile, TRUE );
    } /* endif */

    if ( pParseData->pTermList )
    {
      UtlAlloc( (PVOID *) &pParseData->pTermList, 0L, 0L, NOMSG );
    } /* endif */
    UtlAlloc( (PVOID *) &pParseData, 0L, 0L, NOMSG );
  } /* endif */


  return( usRC );
} /* end of function ParseRTF */

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/******            Subfunctions of function ParseRTF             ******/
/**********************************************************************/
/**********************************************************************/
/*********************************************************************/

/**********************************************************************/
/* RTFHandleFootNoteTag                                               */
/* Handle the footnote tag and preceeding footnote character          */
/**********************************************************************/
USHORT RTFHandleFootNoteTag
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PDESTSTACK  pCurEnv,                 // ptr to curent destination environment
  PCTRLWORD   pCtrlWord                // pointer to current control word
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  SHORT       sRemoveLen;              // length of data to be removed
  CHAR        chFootNoteChar;          // footnote character
  CHAR        abFootNoteChar[10];      // array for removed footnote characters

  // check previous data in output buffer (if available)
  if ( (pParseData->usOutBufUsed >= 3 )         &&
         (pParseData->pOutBuf[-1]  == BEGIN_GROUP) &&
         ( (pParseData->pOutBuf[-3]  == SPACE) ||
           (pParseData->pOutBuf[-3]  == CR)    ||
           (pParseData->pOutBuf[-3]  == LF)    ||
           (pParseData->pOutBuf[-3]  == END_GROUP) ) )
  {
    // this is a footnote of type "... K{\footnote ...."
    sRemoveLen = 3;
    chFootNoteChar = pParseData->pOutBuf[-2];
  }
  else if ( (pParseData->usOutBufUsed >= 3 )         &&
            (pParseData->pOutBuf[-1]  == BEGIN_GROUP) &&
            (pParseData->pOutBuf[-2]  == END_GROUP) )
  {
    // this is a footnote of type "... K}{\footnote ..."
    sRemoveLen = 3;
    chFootNoteChar = pParseData->pOutBuf[-3];
  }
  else if ( (pParseData->usOutBufUsed == 2) &&
            (pParseData->pOutBuf[-1]  == BEGIN_GROUP) )
  {
    // not enough data in output buffer to handle footnote start correctly ...
    sRemoveLen = 2;
    chFootNoteChar = pParseData->pOutBuf[-2];
  }
  else
  {
    // use last character as footnote character ans skip remove of data
    sRemoveLen = 0;
    chFootNoteChar = pParseData->bLastInputChar;
  } /* endif */

  // Remove start of footnote from current segment and write current segment
  if ( (usRC == NO_ERROR) && (sRemoveLen > 0) )
  {
    register SHORT sI;                          // loop index

    for ( sI = 0; sI < sRemoveLen; sI++ )
    {
      pParseData->usOutBufUsed--;
      pParseData->pOutBuf--;
      pParseData->pSegBuf--;
      pParseData->pCharBuf--;
      abFootNoteChar[sI] = pParseData->pOutBuf[0];
    } /* endfor */
    if ( pParseData->pFirstTagPos >= pParseData->pOutBuf )
    {
      pParseData->pFirstTagPos = NULL;
    } /* endif */
    pParseData->fTranslDataInSegment =
       RTFCheckForTranslData( pParseData );

    usRC = RTFWriteSegment( pParseData );
    pParseData->fTranslDataInSegment = FALSE;
  } /* endif */

  // Set footnote type
  switch ( chFootNoteChar )
  {
    case '+' :
    case '#' :
    case '>' : pCurEnv->fTranslSegm = FALSE; break;
    case '@' : pCurEnv->fTranslSegm = FALSE; break;
    case 'A' : pCurEnv->fTranslSegm = FALSE; break;
    default  : pCurEnv->fTranslSegm = TRUE;  break;
  } /* endswitch */

  // Add any removed data as not translatable data
  if ( (usRC == NO_ERROR) && (sRemoveLen > 0) )
  {
    if ( usRC == NO_ERROR )
    {
      register SHORT sI;
      for ( sI = sRemoveLen - 1; sI >= 0; sI-- )
      {
        usRC = AddToSegment( pParseData,
                             abFootNoteChar[sI], pCurEnv->fTranslSegm,
                             CH_TAG );
        if ( usRC != NO_ERROR )
        {
          break;
        } /* endif */
      } /* endfor */
    } /* endif */
  } /* endif */

  // setup footnote data
  if ( usRC == NO_ERROR )
  {
    pParseData->bFootNoteChar = chFootNoteChar;
    pParseData->fFootNoteChar = TRUE;
    pParseData->bCurrFootNoteChar = chFootNoteChar;

    if ( sRemoveLen == 0 )
    {
      pParseData->fTranslDataInSegment = RTFCheckForTranslData( pParseData );
    } /* endif */

    pCurEnv->fField = FALSE;
    pCurEnv->rtfDest = pCtrlWord->rtfID;
    pCurEnv->rtfCurDest = pCtrlWord->rtfID;
  } /* endif */

  return( usRC );
} /* end of RTFHandleFootNoteTag */

/**********************************************************************/
/* RTFHandleHexHar                                                    */
/* Handle the hexadecimal encoded characters                          */
/**********************************************************************/
USHORT RTFHandleHexChar
(
  PPARSEDATA  pParseData,              // points to parser data structure
  BOOL        fTranslSegm,             // segment-is-translatable flag
  BOOL        fProtected,              // data-is-protected flag
  BYTE        bHexCode,                // control code for hex-endcoded
                                       // characters
  BOOL        fLeaveAsIs               // leave the character hex-encoded flag
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL        fOK = TRUE;              // internal O.K. flag
  CHAR        chHex1 = EOS;
  CHAR        chHex2 = EOS;           // buffer for hex digits
  CHAR        ch2ndHex1 = EOS;
  CHAR        ch2ndHex2 = EOS;         // buffer for hex digits
  BYTE        bTemp;                   // buffer for hex character after conversion
  BYTE        bCurrent;                // current character from input

  // get first hex digit from input stream
  do
  {
    chHex1 = ParseNextChar( pParseData, &usRC );
  } while ( !usRC &&
            ( (chHex1 == LF) ||
              (chHex1 == CR) ) ); /* enddo */

  // get second hex digit from input stream
  if ( usRC == NO_ERROR )
  {
    do
    {
      chHex2 = ParseNextChar( pParseData, &usRC );
    } while ( !usRC &&
              ( (chHex2 == LF) ||
                (chHex2 == CR) ) ); /* enddo */
  } /* endif */

  /*********************************************/
  /* Convert hex digits to character and add   */
  /* character to segment                      */
  /*********************************************/
  if ( !usRC )
  {
    if ( isxdigit(chHex1) && isxdigit(chHex2) && !fLeaveAsIs )
    {
      /*****************************************/
      /* convert to character                  */
      /*****************************************/
      bTemp = (BYTE) HEXTONUM( toupper(chHex1) );
      bTemp = bTemp << 4;
      bTemp |= HEXTONUM( toupper(chHex2) );

      /************************************************/
      /* Check for DBCS characters:                   */
      /*                                              */
      /* If the character is the first part of a      */
      /* DBCS character suppress the conversion to    */
      /* ANSI and check for second DBCS character     */
      /************************************************/
      // KA-09212001if ( (_isdbcs(bTemp ) == DBCS_1ST) )
      if (IsDBCSLeadByteEx((USHORT)pParseData->ulSrcOemCP, (BYTE)bTemp  ) == TRUE)  // KA-09212001
      {
        /**********************************************/
        /* Check if really another hex-encoded        */
        /* character for the second byte of the DBCS  */
        /* character is following                     */
        /**********************************************/
        // skip any whitespace in front of next control data
        do
        {
          bCurrent = ParseNextChar( pParseData, &usRC );
        } while ( !usRC &&
                  ( (bCurrent == LF) ||
                    (bCurrent == CR) ) ); /* enddo */

        // check for start of a control word
        if ( (usRC == NO_ERROR) && (bCurrent != START_CTRLWORD) )
        {
          // no control word is following; undo current character
          UndoChar( pParseData, bCurrent );
          fOK = FALSE;               // no valid DBCS character
        } /* endif */

        // skip any white space in front of hex code
        if ( fOK && (usRC == NO_ERROR) )
        {
          do
          {
            bCurrent = ParseNextChar( pParseData, &usRC );
          } while ( !usRC &&
                    ( (bCurrent == LF) ||
                      (bCurrent == CR) ) ); /* enddo */
        } /* endif */

        // check for start of a hex code character
        if ( fOK && (usRC == NO_ERROR) && (bCurrent != bHexCode) )
        {
          // no hex code character is following; undo previous and current char
          UndoChar( pParseData, bCurrent );
          UndoChar( pParseData, START_CTRLWORD );
          fOK = FALSE;               // no valid DBCS character
        } /* endif */

        // skip any white space in front of first hex digit
        if ( fOK && (usRC == NO_ERROR) )
        {
          do
          {
            bCurrent = ParseNextChar( pParseData, &usRC );
          } while ( !usRC &&
                    ( (bCurrent == LF) ||
                      (bCurrent == CR) ) ); /* enddo */
        } /* endif */

        // check for first hex digit
        if ( fOK && (usRC == NO_ERROR) )
        {
          ch2ndHex1 = bCurrent;
          if ( !isxdigit(ch2ndHex1) )
          {
            // no hex digit is following; undo control code and current char
            UndoChar( pParseData, bCurrent );
            UndoChar( pParseData, bHexCode );
            UndoChar( pParseData, START_CTRLWORD );
            fOK = FALSE;               // no valid DBCS character
          } /* endif */
        } /* endif */

        // skip any white space in front of second hex digit
        if ( fOK && (usRC == NO_ERROR) )
        {
          do
          {
            bCurrent = ParseNextChar( pParseData, &usRC );
          } while ( !usRC &&
                    ( (bCurrent == LF) ||
                      (bCurrent == CR) ) ); /* enddo */
        } /* endif */

        // check for second hex digit
        if ( fOK && (usRC == NO_ERROR) )
        {
          ch2ndHex2 = bCurrent;
          if ( !isxdigit(ch2ndHex2) )
          {
            // no hex digit is following; undo control code and hex digits
            UndoChar( pParseData, bCurrent );
            UndoChar( pParseData, ch2ndHex1 );
            UndoChar( pParseData, bHexCode );
            UndoChar( pParseData, START_CTRLWORD );
            fOK = FALSE;               // no valid DBCS character
          } /* endif */
        } /* endif */

        // handle DBCS character or invalid data
        if ( fOK )
        {
          // Unicode Todo: Special handling for DBCS characters
          // write the two bytes of the DBCS character to the output file
          usRC = AddToSegment( pParseData, bTemp,
                               fTranslSegm,
                               (fProtected) ? CH_TAG : CH_YES );

          if ( !usRC )
          {
            bTemp = (BYTE) HEXTONUM( toupper(ch2ndHex1) );
            bTemp = bTemp << 4;
            bTemp |= HEXTONUM( toupper(ch2ndHex2) );

            usRC = AddToSegment( pParseData, bTemp,
                                 fTranslSegm,
                                 (fProtected) ? CH_TAG : CH_YES );
          } /* endif */
        }
        else if ( usRC == NO_ERROR )
        {
          // write first part of DBCS character in hex-encoded form to
          // avoid handling as DBCS character during export
          usRC = AddToSegment( pParseData, START_CTRLWORD, fTranslSegm, CH_TAG );
          if ( usRC == NO_ERROR )
          {
            usRC = AddToSegment( pParseData, bHexCode, fTranslSegm, CH_TAG );
          } /* endif */
          if ( usRC == NO_ERROR )
          {
            usRC = AddToSegment( pParseData, chHex1, fTranslSegm, CH_TAG );
          } /* endif */
          if ( usRC == NO_ERROR )
          {
            usRC = AddToSegment( pParseData, chHex2, fTranslSegm, CH_TAG );
          } /* endif */
        } /* endif */
      }
      else if ( bTemp < 0x10 )
      {
        // do not convert these characters
        usRC = AddToSegment( pParseData, START_CTRLWORD, fTranslSegm, CH_TAG );
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, bHexCode, fTranslSegm, CH_TAG );
        } /* endif */
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, chHex1, fTranslSegm, CH_TAG );
        } /* endif */
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, chHex2, fTranslSegm, CH_TAG );
        } /* endif */
      }
      else if ( (bTemp == 0xAE) || (bTemp == 0xA9) || (bTemp == 0x99) )
      {
        // do not convert trade Mark, Registered Trademark and Copyright symbol
        usRC = AddToSegment( pParseData, START_CTRLWORD, fTranslSegm, CH_TAG );
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, bHexCode, fTranslSegm, CH_TAG );
        } /* endif */
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, chHex1, fTranslSegm, CH_TAG );
        } /* endif */
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, chHex2, fTranslSegm, CH_TAG );
        } /* endif */
      }
      else if ( (bTemp == 0x94) || (bTemp == 0x93) || (bTemp == 0xB7) || (bTemp == 0x85) )
      {
        // typograhic double-quote, bullet and ellipsis: curently leave hex encodes as the display of this
        // characters does not work in the standard editor
        usRC = AddToSegment( pParseData, START_CTRLWORD, fTranslSegm, CH_TAG );
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, bHexCode, fTranslSegm, CH_TAG );
        } /* endif */
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, chHex1, fTranslSegm, CH_TAG );
        } /* endif */
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, chHex2, fTranslSegm, CH_TAG );
        } /* endif */
      }
      else if ( ((bTemp >= 'A') && (bTemp <= 'Z')) ||
                ((bTemp >= 'a') && (bTemp <= 'z')) )
      {
        // do not convert normal characters which are hex-encoded as
        // this characters may be symbols (e.g. from Wingdings font) and
        // will cause problems during document export in BIDI environments
        usRC = AddToSegment( pParseData, START_CTRLWORD, fTranslSegm, CH_TAG );
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, bHexCode, fTranslSegm, CH_TAG );
        } /* endif */
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, chHex1, fTranslSegm, CH_TAG );
        } /* endif */
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, chHex2, fTranslSegm, CH_TAG );
        } /* endif */
      }
      else if ( pParseData->pConvTable && (pParseData->pConvTable[bTemp] == 0) )
      {
        // GQ: leave hex encoded characters as-is if the character results in hex
        // zero after conversion

        // GQ: Unicode conversion disabled, code point created was not helpful at all
        //CHAR_W szUnicodeChar[10];
        //CHAR   szAnsiChar[3];
        //int iConverted;
        //
        //szAnsiChar[0] = (CHAR)bTemp;
        //szAnsiChar[1] = EOS;
        //iConverted = MultiByteToWideChar( CP_ACP, 0, szAnsiChar, 1, szUnicodeChar, 9 );
        //if ( iConverted == 1 )
        //{
        //  // add unicode code point of not converted character
        //  CHAR szUnicodeTag[10];
        //  PSZ  pszTemp = szUnicodeTag;
        //  sprintf( szUnicodeTag, "\\u%d", szUnicodeChar[0] );
        //  while ( (usRC == NO_ERROR) && *pszTemp )
        //  {
        //    usRC = AddToSegment( pParseData, *pszTemp, fTranslSegm, CH_TAG );
        //    pszTemp++;
        //  } /* endwhile */
        //} /* endif */
        usRC = AddToSegment( pParseData, START_CTRLWORD, fTranslSegm, CH_TAG );
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, bHexCode, fTranslSegm, CH_TAG );
        } /* endif */
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, chHex1, fTranslSegm, CH_TAG );
        } /* endif */
        if ( usRC == NO_ERROR )
        {
          usRC = AddToSegment( pParseData, chHex2, fTranslSegm, CH_TAG );
        } /* endif */
      }
      else
      {
        // handle normal characters

        /**********************************************/
        /* Convert character from ANSI (or whatever)  */
        /* to the ASCII code page using the           */
        /* active conversion table                    */
        /**********************************************/

        /**********************************************/
        /* Special handling for backslash and curly   */
        /* braces: add a backslash in front of the    */
        /* character to avoid misinterpreting of      */
        /* character as control information           */
        /**********************************************/
        switch ( bTemp )
        {
          case '\\' :
          case '{' :
          case '}' :
            usRC = AddToSegment( pParseData, '\\',
                                 fTranslSegm, CH_NO );
            if ( !usRC )
            {
              usRC = AddToSegment( pParseData, bTemp,
                                   fTranslSegm,
                                   CH_NO );
            } /* endif */
            break;
          default :
            usRC = AddToSegment( pParseData, bTemp,
                                 fTranslSegm,
                   (fProtected) ? CH_TAG : TranslChar[bTemp] );
            break;
        } /* endswitch */
      } /* endif */

    }
    else
    {
      /*****************************************/
      /* Misinterpreted control word? Write    */
      /* control word as-is ...                */
      /*****************************************/
      usRC = AddToSegment( pParseData, START_CTRLWORD,
                           fTranslSegm, CH_TAG );
      if ( !usRC )
      {
        usRC = AddToSegment( pParseData, bHexCode,
                             fTranslSegm, CH_TAG );
      } /* endif */
      if ( !usRC )
      {
        usRC = AddToSegment( pParseData, chHex1,
                             fTranslSegm, CH_TAG );
      } /* endif */
      if ( !usRC )
      {
        usRC = AddToSegment( pParseData, chHex2,
                             fTranslSegm, CH_TAG );
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function RTFHandleHexChar */

/**********************************************************************/
/* Check if output segment contains translatable data                 */
/* (as a side-effect, the pLastTagPos is set to last tag position     */
/* and pFirstTagPos is set to the first tag position at the end       */
/* of the segment)                                                    */
/**********************************************************************/
BOOL RTFCheckForTranslData
(
  PPARSEDATA  pParseData               // points to parser data structure
)
{
  register SHORT  sI;
  BOOL   fTranslData = FALSE;

  pParseData->pLastTagPos = NULL;

  /********************************************************************/
  /* Look for translatable data in segment                            */
  /********************************************************************/
  sI = 0;
  while ( sI < (SHORT)pParseData->usOutBufUsed )
  {
    if ( (pParseData->abCharBuf[sI] == CH_TAG) && !fTranslData )
    {
      pParseData->pLastTagPos = pParseData->abOutBuf + sI;
    } /* endif */

    if ( pParseData->abCharBuf[sI] == CH_YES )
    {
      fTranslData = TRUE;
      break;
    } /* endif */

    sI++;
  } /* endwhile */

  /********************************************************************/
  /* Set first tag position                                           */
  /********************************************************************/
  if ( pParseData->usOutBufUsed != 0 )
  {
    sI = pParseData->usOutBufUsed - 1;
    pParseData->pFirstTagPos = NULL;
    while ( (sI >= (SHORT)0) &&
            ( (pParseData->abCharBuf[sI] == CH_TAG) ||
              (pParseData->abCharBuf[sI] == CH_NO) ) )
    {
      if ( pParseData->abCharBuf[sI] == CH_TAG )
      {
        pParseData->pFirstTagPos = pParseData->abOutBuf + sI;
      } /* endif */
      sI--;
    } /* endwhile */
  } /* endif */

  return( fTranslData );

} /* end of function RTFCheckForTranslData */

/**********************************************************************/
/* RTFNormalChar                                                      */
/*                                                                    */
/* Process a nomral (i.e. no control charcter)                        */
/**********************************************************************/
USHORT RTFNormalChar
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PBYTE       pbCurrent,               // ptr to buffer for current character
  BOOL        fProtected,              // current protection state
  BOOL        fTranslSegm              // current segement state
)
{
  USHORT      usRC = NO_ERROR;        // function return code

  //KA if ( _isdbcs(*pbCurrent ) == DBCS_1ST )
  if (IsDBCSLeadByteEx(pParseData->ulSrcOemCP, (BYTE)*pbCurrent ) == TRUE)  //KA
  {
    /********************************************************/
    /* treat this byte and the next byte as one unit and add*/
    /* these bytes as text                                  */
    /********************************************************/
    usRC = AddToSegment( pParseData, *pbCurrent, fTranslSegm,
                         (fProtected) ? CH_TAG : TranslChar[*pbCurrent] );
    *pbCurrent = ParseNextChar( pParseData, &usRC );

    if ( !usRC && !*(pParseData->pfKill) )
    {
      usRC = AddToSegment( pParseData, *pbCurrent, fTranslSegm,
                          (fProtected) ? CH_TAG : TranslChar[*pbCurrent] );
    } /* endif */
  }
  else
  {
    BYTE bTemp;                        // buffer for converted byte

    bTemp = *pbCurrent;

    if ( *pbCurrent != SPACE )
    {
      pParseData->bLastInputChar = *pbCurrent;
      if ( pParseData->fFootNoteChar &&
           (*pbCurrent == pParseData->bFootNoteChar) )
      {
        /****************************************************/
        /* Lets have a look at the next character...        */
        /****************************************************/
        BYTE bNext = ParseNextChar( pParseData, &usRC );

        if ( !usRC )
        {
          UndoChar( pParseData, bNext );
        } /* endif */
        if ( !usRC )
        {
          if ( !isalpha(bNext) )
          {
            usRC = AddToSegment( pParseData, bTemp, fTranslSegm,
                                 CH_TAG );
          }
          else
          {
            usRC = AddToSegment( pParseData, bTemp, fTranslSegm,
                           (fProtected) ? CH_TAG : TranslChar[bTemp] );
          } /* endif */
        } /* endif */
        pParseData->fFootNoteChar = FALSE;
      }
      else
      {
        usRC = AddToSegment( pParseData, bTemp, fTranslSegm,
                             (fProtected) ? CH_TAG : TranslChar[bTemp] );
      } /* endif */
    }
    else
    {
      usRC = AddToSegment( pParseData, bTemp, fTranslSegm,
                           (fProtected) ? CH_TAG : TranslChar[bTemp] );
    } /* endif */
  } /* endif */
  return( usRC );
} /* end of function RTFNormalChar */

/**********************************************************************/
/* RTFParseMacro                                                           */
/*                                                                    */
/* Process a windows help macro started with the hotspot character !  */
/**********************************************************************/
USHORT RTFParseMacro
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PBYTE       pbCurrent,               // ptr to buffer for current character
  BOOL        fProtected,              // current protection state
  BOOL        fTranslSegm,             // current segement state
  BYTE        bFirstChar               // EOS or current byte if already read
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  register SHORT sI;                   // general loop index
  PHELPMACRO  pMacro;                  // ptr to macro definition data
  BYTE        bCurrent;                // currently processed character
  SHORT       sParmNum;                // number of currently processed parm
  pbCurrent;

  /********************************************************************/
  /* Check macro name against list of known/processed macros          */
  /********************************************************************/
  pMacro = HelpMacro;                // start at begin of macro definitions
  if ( bFirstChar != EOS )
  {
    while ( (pMacro->Id != END_OF_MACROS) &&
            !( (pMacro->szMacro[0] == bFirstChar) &&
               (_strnicmp( pMacro->szMacro+1, (PSZ)pParseData->pInBuf, pMacro->sLength-1 ) == 0) ) )
    {
      pMacro++;
    } /* endwhile */
  }
  else
  {
    while ( (pMacro->Id != END_OF_MACROS) &&
            (_strnicmp( pMacro->szMacro,(PSZ)pParseData->pInBuf, pMacro->sLength ) != 0 ) )
    {
      pMacro++;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Handle macro (if any)                                            */
  /********************************************************************/
  if ( (usRC == NO_ERROR) && (pMacro->Id != END_OF_MACROS) )
  {
    pParseData->fMacroMayFollow = FALSE; // no macro will follow anymore

    /****************************************************************/
    /* Add name of macro as not translatable data                   */
    /****************************************************************/
    sI = pMacro->sLength;
    if ( bFirstChar != EOS)
    {
      usRC = AddToSegment( pParseData, bFirstChar, fTranslSegm, CH_TAG );
      sI--;
    } /* endif */
    while ( (usRC == NO_ERROR) && (sI != 0) )
    {
      bCurrent = ParseNextChar( pParseData, &usRC );
      if ( usRC == NO_ERROR )
      {
        usRC = AddToSegment( pParseData, bCurrent, fTranslSegm, CH_TAG );
      } /* endif */
      sI--;
    } /* endwhile */

    usRC = RTFParseSkipAnyWhiteSpace( pParseData, &bCurrent, fTranslSegm, CH_TAG );

    /****************************************************************/
    /* Check for start of macro parameters                          */
    /****************************************************************/
    if ( usRC == NO_ERROR )
    {
      if ( bCurrent == BEGIN_PARMS )
      {
        // add begin of parameters to current segment
        usRC = AddToSegment( pParseData, bCurrent, fTranslSegm, CH_TAG );

        // handle macro parameters until closing parenthesis found
        sParmNum = 0;
        while ( (usRC == NO_ERROR) && (bCurrent != END_PARMS) )
        {
          usRC = RTFParseMacroParm( pParseData, &bCurrent, fProtected,
                                    fTranslSegm, CH_TAG, pMacro, sParmNum );

          // handle macro parameter delimiter
          if ( (usRC == NO_ERROR) && (bCurrent == COMMA) )
          {
            // add macro parameter dlimiter as not-translatable data
            sParmNum++;
            usRC = AddToSegment( pParseData, bCurrent, fTranslSegm, CH_TAG );
          } /* endif */
        } /* endwhile */

        if ( usRC == NO_ERROR )
        {
          // add closing parenthesis to current segment
          usRC = AddToSegment( pParseData, bCurrent, fTranslSegm, CH_TAG );
        } /* endif */
      }
      else
      {
        // put push current character back
        UndoChar( pParseData, bCurrent );
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function RTFParseMacro */

/**********************************************************************/
/* RTFGetControlWord                                                  */
/*                                                                    */
/* Checks if a control word follows and stores any control word in    */
/* szControlWord buffer of the ParsData structure                     */
/* The function returns TRUE if a valid control word was detected and */
/* FALSE if there was no or no valid control word                     */
/**********************************************************************/
BOOL RTFGetControlWord
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PUSHORT     pusRC,                   // ptr to buffer for return code
  PBYTE       pbCurrent                // ptr to buffer for current char
)
{
  register BYTE bCurrent;              // buffer for currently processed char
  register SHORT sI;                   // index for control word buffer
  BOOL        fCtrlWord = FALSE;       // valid-control-word-detected flag

  /********************************************************************/
  /* get first char of maybe control word                             */
  /********************************************************************/
  do
  {
    bCurrent = ParseNextChar( pParseData, pusRC );
  } while ( !*pusRC && ((bCurrent == LF) || (bCurrent == CR)) ); /* enddo */

  if ( !*pusRC )
  {
    /***************************************************/
    /* check for control word (starting with an        */
    /* alphabetic character)                           */
    /***************************************************/
    if ( isalpha(bCurrent) || (bCurrent == COMMENT_TAG) )
    {
      /*************************************************/
      /* get control word                              */
      /*                                               */
      /* Note: Control words end at the first non-alpha*/
      /*       character with one exception:           */
      /*       Control words starting with \* are      */
      /*       followed by another \ before the        */
      /*       alphabetic characters of the control    */
      /*       word start                              */
      /*************************************************/
      sI = 0;
      if ( bCurrent == COMMENT_TAG )
      {
        /***********************************************/
        /* Handle control words starting with \*       */
        /***********************************************/
        bCurrent = ParseNextChar( pParseData, pusRC );

        if ( bCurrent == START_CTRLWORD )
        {
          pParseData->szControlWord[sI++] = COMMENT_TAG;
          pParseData->szControlWord[sI++] = bCurrent;
          bCurrent = ParseNextChar( pParseData, pusRC );

          while ( !*pusRC && isalpha(bCurrent) )
          {
            pParseData->szControlWord[sI++] = bCurrent;
            bCurrent = ParseNextChar( pParseData, pusRC );
          } /* endwhile */
        }
        else
        {
          pParseData->szControlWord[sI++] = COMMENT_TAG;
        } /* endif */
      }
      else
      {
        /***********************************************/
        /* Handle normal control words                 */
        /***********************************************/
        while ( !*pusRC && isalpha(bCurrent) )
        {
          pParseData->szControlWord[sI++] = bCurrent;
          bCurrent = ParseNextChar( pParseData, pusRC );
        } /* endwhile */
      } /* endif */
      pParseData->szControlWord[sI] = EOS;
      if ( !*pusRC )
      {
        fCtrlWord = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */

  *pbCurrent = bCurrent;               // pass last char to calling function

  return( fCtrlWord );
} /* end of function RTFGetControlWord */

/**********************************************************************/
/* Function RTFCheckAndHandleHelpTag                                  */
/*                                                                    */
/* Check for Windows help tags (BML,BMR,BMC) and process these tags   */
/*                                                                    */
/* Note: this procedure is called after a \{ character has been       */
/*       detected                                                     */
/**********************************************************************/
USHORT RTFCheckAndHandleHelpTag
(
  PPARSEDATA  pParseData,              // points to parser data structure
  BOOL        fTranslSegm,             // current-segment-is-translatable flag
  BOOL        fProtected               // current-data-is-protected flag
)
{
  BYTE bCurrent;                       // buffer for current char
  USHORT usRC = 0;                     // function return code
  register SHORT sI;                            // loop index

  // check for Windows help tag
  sI = 0;
  while ( (HelpTag[sI].Id != END_OF_TAGS) &&
          (_strnicmp( HelpTag[sI].szTag,
                     (PSZ)pParseData->pInBuf,
                     HelpTag[sI].sLength ) != 0 ) )
  {
    sI++;
  } /* endwhile */

  // handle help tags
  switch ( HelpTag[sI].Id )
  {
    case BML_TAG :
    case BMC_TAG :
    case BMR_TAG :
      {
        // write curly brace to output segment
        if ( !usRC )
        {
          usRC = AddToSegment( pParseData, START_CTRLWORD,
                               fTranslSegm, CH_TAG );
        } /* endif */

        if ( !usRC )
        {
          usRC = AddToSegment( pParseData, BEGIN_GROUP,
                               fTranslSegm, CH_TAG );
        } /* endif */

        // add all data up to closing curly brace as inline tag (=PROTECTED),
        // skip any nested groups within the help tag (e.g. \*\bookmark)
        {
          BYTE bLast = EOS;            // last character processed
          SHORT sNesting = 0;          // current curly brace nesting


          bCurrent = ParseNextChar( pParseData, &usRC );
          while ( !usRC && !((bCurrent == END_GROUP) && (sNesting <= 0)) )
          {
            bLast = bCurrent;

            if ( bCurrent == BEGIN_GROUP )
            {
              sNesting++;
            }
            else if ( bCurrent == END_GROUP )
            {
              sNesting--;
            } /* endif */
            usRC = AddToSegment( pParseData, bCurrent,
                                 fTranslSegm, CH_TAG );
            if ( !usRC )
            {
              bCurrent = ParseNextChar( pParseData, &usRC );
            } /* endif */
          } /* endwhile */

          // Undo last character if closing curly brace was not prefixed
          // by a backslash (START_CTRLWORD)
          if ( bLast != START_CTRLWORD )
          {
            UndoChar( pParseData, bCurrent );
            bCurrent = EOS;
          } /* endif */
        }

        // add any closing curly brace as inline tag
        if ( !usRC && (bCurrent == END_GROUP) )
        {
          usRC = AddToSegment( pParseData, END_GROUP,
                               fTranslSegm, CH_TAG );
        } /* endif */
      }
      break;

    case BUTTON_TAG :
      {
        /****************************************************************/
        /* Handle button tag:                                           */
        /*                                                              */
        /* Check if second parameter is KLink, if KLink is detected     */
        /* add terms enclosed in parenthesis as translatable segments.  */
        /****************************************************************/
        // write curly brace to output segment
        if ( !usRC )
        {
          usRC = AddToSegment( pParseData, START_CTRLWORD, fTranslSegm, CH_TAG );
        } /* endif */

        if ( !usRC )
        {
          usRC = AddToSegment( pParseData, BEGIN_GROUP, fTranslSegm, CH_TAG );
        } /* endif */

        // add tag name as inline-tag
        bCurrent = ParseNextChar( pParseData, &usRC );
        while ( !usRC &&                         // no error and
                (bCurrent != BLANK) &&           // not end of macro name and
                (bCurrent != COMMA) &&           // not end of parameter and
                !((bCurrent == START_CTRLWORD) && // not end of help macro
                 (pParseData->pInBuf[0] == END_GROUP)) )
        {
          usRC = AddToSegment( pParseData, bCurrent,
                               fTranslSegm, CH_TAG );
          if ( !usRC )
          {
            bCurrent = ParseNextChar( pParseData, &usRC );
          } /* endif */
        } /* endwhile */

        // add tag name delimiter as inline-tag
        if ( !usRC && (bCurrent == BLANK) )
        {
          usRC = AddToSegment( pParseData, bCurrent,
                               fTranslSegm, CH_TAG );
          if ( !usRC )
          {
            bCurrent = ParseNextChar( pParseData, &usRC );
          } /* endif */
        } /* endif */

        // add all data up to start of second parameter as translatable data
        while ( !usRC &&                         // no error and
                (bCurrent != COMMA) &&           // not end of parameter and
                !((bCurrent == START_CTRLWORD) && // not end of help macro
                 (pParseData->pInBuf[0] == END_GROUP)) )
        {
          if ( bCurrent == START_CTRLWORD )
          {
              bCurrent = ParseNextChar( pParseData, &usRC );
              if ( bCurrent == '\'' )
              {
                usRC = RTFHandleHexChar( pParseData, fTranslSegm, FALSE,
                                        bCurrent,
                                        FALSE );
              }
              else
              {
                UndoChar( pParseData, bCurrent );
                usRC = AddToSegment( pParseData, START_CTRLWORD,
                                    fTranslSegm,
                                    (fProtected) ? CH_TAG :
                                                    TranslChar[START_CTRLWORD] );
              } /* endif */
            }
            else
            {
                    usRC = AddToSegment( pParseData, bCurrent,
                                        fTranslSegm,
                                        (fProtected) ? CH_TAG :
                                                        TranslChar[bCurrent] );
            } /* endif */

            if ( !usRC )
            {
              bCurrent = ParseNextChar( pParseData, &usRC );
            } /* endif */
        } /* endwhile */

        // handle any macro within button tag
        if ( !usRC && (bCurrent == COMMA) )
        {
          // add parameter delimiter as non-translatable data
          usRC = AddToSegment( pParseData, bCurrent,
                               fTranslSegm, CH_TAG );

          // Skip any leading whitespace
          while ( (usRC == NO_ERROR) &&
                  (pParseData->pInBuf[0] == SPACE) )
          {
            bCurrent = ParseNextChar( pParseData, &usRC );
            if ( usRC == NO_ERROR )
            {
              usRC = AddToSegment( pParseData, bCurrent, fTranslSegm, CH_TAG );
            } /* endif */
          } /* endwhile */

          // call macro processing function
          if ( usRC == NO_ERROR )
          {
            usRC = RTFParseMacro( pParseData, &bCurrent, fProtected,
                             fTranslSegm, EOS );
          } /* endif */
        } /* endif */

        // add all data up to closing curly brace as inline tag (=PROTECTED)
        if ( !usRC ) bCurrent = ParseNextChar( pParseData, &usRC );
        while ( !usRC &&
                !((bCurrent == START_CTRLWORD) && // not end of help macro
                 (pParseData->pInBuf[0] == END_GROUP)) )
        {
          usRC = AddToSegment( pParseData, bCurrent,
                               fTranslSegm, CH_TAG );
          if ( !usRC )
          {
            bCurrent = ParseNextChar( pParseData, &usRC );
          } /* endif */
        } /* endwhile */

        // add closing sequence as inline tag
        if ( !usRC )
        {
          usRC = AddToSegment( pParseData, bCurrent, fTranslSegm, CH_TAG );
        } /* endif */
        if ( !usRC )
        {
          bCurrent = ParseNextChar( pParseData, &usRC );
        } /* endif */
        if ( !usRC )
        {
          usRC = AddToSegment( pParseData, bCurrent, fTranslSegm, CH_TAG );
        } /* endif */
      }
      break;

    default :
      {
        // write curly brace to output segment
        if ( !usRC )
        {
          usRC = AddToSegment( pParseData, START_CTRLWORD,
                               fTranslSegm,
                               (fProtected) ? CH_TAG : CH_NO );
        } /* endif */

        if ( !usRC )
        {
          usRC = AddToSegment( pParseData, BEGIN_GROUP,
                               fTranslSegm,
                               (fProtected) ? CH_TAG : CH_NO );
        } /* endif */
      }
      break;
  } /* endswitch */

  return( usRC );
} /* end of function RTFCheckAndHandleHelpTag */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     AddToSegment           Add data to current segment
//------------------------------------------------------------------------------
// Function call:     AddToSegment( PPARSEDATA pParseData, BYTE bAddByte,
//                                  BOOL fTranslSeg, BOOL fTranslData );
//------------------------------------------------------------------------------
// Description:       Add one byte to the current segment and, if the segment
//                    buffer is full, write the segment to the output file and
//                    start a new one.
//------------------------------------------------------------------------------
// Input parameter:   PPARSEDATA pParseData    ptr to parser global data struct
//                    BYTE       bAddByte      byte being added to segment
//                    BOOL       fTranslSeg    data is for a translatable segm.
//                    BOOL       fTranslData   data itself is translatable
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR            function completed successfully
//                    other               OS/2 Dos calls error codes
//------------------------------------------------------------------------------
// Prerequesits:      Output file must be open.
//------------------------------------------------------------------------------
// Function flow:     Get new segment type
//                    End current segment if a segment is open and the data
//                       belongs to different type of segment
//                    Start a new segment if none is active
//                    if output buffer is full then
//                      split segment at last morphologic segment break or at
//                      last linefeed (for segments containing not
//                      translatable data) and write segment
//                    endif
//                    Move not translatable data at the begin of segment
//                       with translatable data to a segment of its own
//                    Append new byte to segment buffer
//                    Add CR/LF in translatable segments if line width is
//                       exceeded
//------------------------------------------------------------------------------

USHORT AddToSegment
(
  PPARSEDATA pParseData,               // ptr to parser global data structure
  register BYTE bAddByte,                // byte being added to segment
  BOOL       fTranslSeg,              // data is for a translatable segment
  CHARTYPE   CharType                 // type of current character (bAddByte)
)
{
  USHORT usRC = 0;                    // internal return code
  USHORT usRest;                      // rest of segment
  PBYTE  pbTemp = NULL;               // ptr for segment data processing
  SEGTYPES NewSegType;                // new type for segment
  PFLAGOFFSLIST pTerm;                // pointer for term list processing
  USHORT usLastOffs;                  // last segment break offset
  SEGTYPES TempSegType = (SEGTYPES)0; // buffer for current segment type

  /*******************************************************************/
  /* Get new segment type                                            */
  /*******************************************************************/
  NewSegType = ( fTranslSeg ) ? TRANSL_SEGMENT : PROTECT_SEGMENT;

  /*******************************************************************/
  /* End current segment if a segment is open and the data belongs   */
  /* to different type of segment (however treat segments            */
  /* with a style of TRANSL_SEGMENT and containing no translatable   */
  /* data as PROTECT_SEGMENT)                                        */
  /*******************************************************************/
  if ( (NewSegType == PROTECT_SEGMENT) &&
       (pParseData->SegType == TRANSL_SEGMENT) &&
       !pParseData->fTranslDataInSegment )
  {
    // modify type of segment to PROTECT_SEGMENT
    pParseData->SegType = PROTECT_SEGMENT;
  }
  else if ( (pParseData->SegType != NO_SEGMENT) &&
            (pParseData->SegType != NewSegType) )

  {
    usRC = RTFWriteSegment( pParseData );
  } /* endif */

  /*******************************************************************/
  /* Start a new segment if no segment is active                     */
  /*******************************************************************/
  if ( !usRC && (pParseData->SegType == NO_SEGMENT) )
  {
    pParseData->pOutBuf = pParseData->abOutBuf;
    pParseData->pLastTagPos = NULL;
    pParseData->pSegBuf = pParseData->abSegBuf;
    pParseData->pCharBuf = pParseData->abCharBuf;
    pParseData->usOutBufUsed = 0;
    pParseData->usLinePos    = 0;
    pParseData->SegType = NewSegType;
    pParseData->fTranslDataInSegment = FALSE;
  } /* endif */

  // try to remove superfluos tags if segment buffer is full
  if ( !usRC && (pParseData->usOutBufUsed == MAX_SEG_SIZE) &&
       (pParseData->SegType == TRANSL_SEGMENT) &&
       pParseData->fTranslDataInSegment )
  {
    BOOL fSegChanged = FALSE;
    CHAR chTemp = NULC;

    // ensure null terminated string
    chTemp = pParseData->abOutBuf[pParseData->usOutBufUsed];
    pParseData->abOutBuf[pParseData->usOutBufUsed] = EOS;

    // tag removal
    fSegChanged = RTFRemoveTags( (PSZ)pParseData->abOutBuf,
                                 pParseData->szRemoveTagsBuffer,
                                 pParseData->chRemoveTags,
                                 pParseData->abSegBuf,
                                 pParseData->abCharBuf );

    // restore original end character
    pParseData->abOutBuf[pParseData->usOutBufUsed] = chTemp;

    // use changed segment data if tags have been removed
    if ( fSegChanged )
    {
      int iLen = strlen( pParseData->szRemoveTagsBuffer );
      pParseData->usOutBufUsed = (USHORT)iLen;
      memcpy( pParseData->abOutBuf, pParseData->szRemoveTagsBuffer, iLen );
      pParseData->pOutBuf = pParseData->abOutBuf + iLen;
      pParseData->pSegBuf = pParseData->abSegBuf + iLen;
      pParseData->pCharBuf = pParseData->abCharBuf + iLen;
      pParseData->fTranslDataInSegment = RTFCheckForTranslData( pParseData );
    } /* endif */

  } /* endif */

  /*******************************************************************/
  /* Force a new segment if segment buffer is full                   */
  /*******************************************************************/
  if ( !usRC && (pParseData->usOutBufUsed == MAX_SEG_SIZE) )
  {
    if ( (pParseData->SegType == TRANSL_SEGMENT) &&
         pParseData->fTranslDataInSegment )
    {
      /***************************************************************/
      /* For translatable segments use morphologic functions to find */
      /* the correct place to split the segment                      */
      /***************************************************************/
      usRC = RTFMorphTokenize( pParseData );

      if ( usRC == NO_ERROR )
      {
        /*************************************************************/
        /* Look for last segment break in segment buffer             */
        /*************************************************************/
        pTerm  = (PFLAGOFFSLIST)pParseData->pTermList;
        usLastOffs = pTerm->usOffs;
        while ( (pTerm->usOffs != 0) ||
                (pTerm->usLen  != 0) ||
                (pTerm->lFlags != 0L ) )
        {
          if ( pTerm->lFlags & TF_NEWSENTENCE )
          {
            /***********************************************************/
            /* Segment starts at next term                             */
            /***********************************************************/
            pTerm++;
            if ( pTerm->usOffs != 0 )
            {
              usLastOffs = pTerm->usOffs;
            } /* endif */
          } /* endif */
          if ( (pTerm->usOffs != 0) ||
               (pTerm->usLen  != 0) ||
               (pTerm->lFlags != 0L ) )
          {
            pTerm++;
          } /* endif */
        } /* endwhile */

        /*************************************************************/
        /* Set segment break position                                */
        /*************************************************************/
        if ( usLastOffs != 0 )
        {
          pbTemp = pParseData->abOutBuf + usLastOffs - 1;
        }
        else
        {
          /**************************************************************/
          /* Try to split at last whitespace character                  */
          /**************************************************************/
          pbTemp = pParseData->abOutBuf + pParseData->usOutBufUsed;
          while ( (pbTemp > pParseData->abOutBuf) && !isspace(*pbTemp) )
          {
            pbTemp--;
          } /* endwhile */
        } /* endif */
      } /* endif */
    }
    else
    {
      // Look for the right place (= whitespace or start of tag) to split
      // data in segment buffer
      BOOL fDone = FALSE;

      pbTemp = pParseData->abOutBuf + pParseData->usOutBufUsed;
      while ( (pbTemp > pParseData->abOutBuf) && !fDone )
      {
        if ( isspace(*pbTemp) )
        {
          // o.k. we can split the segment here
          fDone = TRUE;
        }
        else  if ( *pbTemp == START_CTRLWORD )
        {
          // check previous character to ensure that we do not
          // split double backslash
          pbTemp--;
          if ( *pbTemp == START_CTRLWORD )
          {
            // a double backslash, split in front of starting backslash
            pbTemp--;
            fDone = TRUE;
          }
          else
          {
            // a single backslash split here
            fDone = TRUE;
          } /* endif */
        }
        else
        {
          pbTemp--;                    // try previous character
        } /* endif */
      } /* endwhile */

    } /* endif */

    /*****************************************************************/
    /* if no possible segment break has been spotted,write segment s */
    /* as-is otherwise split segment at found location               */
    /*****************************************************************/
    if ( pbTemp <= pParseData->abOutBuf )
    {
      /**************************************************************/
      /* No possible break location spotted ...                     */
      /**************************************************************/
      usRC = RTFWriteSegment( pParseData );
      if ( !usRC )
      {
        pParseData->pOutBuf = pParseData->abOutBuf;
        pParseData->pSegBuf = pParseData->abSegBuf;
        pParseData->pCharBuf = pParseData->abCharBuf;
        pParseData->usOutBufUsed = 0;
        pParseData->usLinePos    = 0;
        pParseData->SegType = NewSegType;
        pParseData->fTranslDataInSegment = FALSE;
        pParseData->pFirstTagPos = NULL;
      } /* endif */
    }
    else
    {
      /**************************************************************/
      /* Split segment at found position                            */
      /**************************************************************/
      pbTemp++;

      usRest = (USHORT)(pParseData->usOutBufUsed - (pbTemp - pParseData->abOutBuf));

      if ( usRest )
      {
        memcpy( pParseData->abTempBuf, pbTemp, usRest );
        memcpy( pParseData->abTempSegBuf,
                pParseData->abSegBuf + (pbTemp - pParseData->abOutBuf),
                usRest );
        memcpy( pParseData->abTempCharBuf,
                pParseData->abCharBuf + (pbTemp - pParseData->abOutBuf),
                usRest );
        pParseData->usOutBufUsed = (USHORT)(pParseData->usOutBufUsed  - usRest);
        TempSegType = pParseData->SegType;
        pParseData->pFirstTagPos = NULL;
      } /* endif */

      usRC = RTFWriteSegment( pParseData );

      if ( !usRC )
      {
        pParseData->pOutBuf = pParseData->abOutBuf;
        pParseData->pLastTagPos = NULL;
        pParseData->pSegBuf = pParseData->abSegBuf;
        pParseData->pCharBuf = pParseData->abCharBuf;
        pParseData->usOutBufUsed = 0;
        pParseData->usLinePos    = 0;
        pParseData->SegType = NewSegType;
        pParseData->fTranslDataInSegment = FALSE;
      } /* endif */

      if ( !usRC && usRest )
      {
        memcpy( pParseData->abOutBuf, pParseData->abTempBuf, usRest );
        memcpy( pParseData->abSegBuf, pParseData->abTempSegBuf, usRest );
        memcpy( pParseData->abCharBuf, pParseData->abTempCharBuf, usRest );
        pParseData->usOutBufUsed = usRest;
        pParseData->pOutBuf = pParseData->abOutBuf + usRest;
        pParseData->pSegBuf = pParseData->abSegBuf + usRest;
        pParseData->pCharBuf = pParseData->abCharBuf + usRest;
        pParseData->SegType = TempSegType;
        pParseData->fTranslDataInSegment = RTFCheckForTranslData( pParseData );
      } /* endif */
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* If segment is for translatable data and segment is not empty    */
  /* but contains tags already and the new data byte is translatable */
  /* data, write segment and start a new one                         */
  /*                                                                 */
  /* Note: This handling ensures that RTF tags at the start of a     */
  /*       segment are stored in a segment of their own.             */
  /*******************************************************************/
  if ( !usRC )
  {
    if ( (pParseData->SegType == TRANSL_SEGMENT) &&
         (pParseData->usOutBufUsed != 0)         &&
         (!pParseData->fTranslDataInSegment)     &&
          (CharType == CH_YES) )
    {
      /****************************************************************/
      /* Compute new last tag position                                */
      /****************************************************************/
      {
        SHORT sI;
        BOOL  fOtherChars = FALSE;

        pParseData->pLastTagPos = NULL;

        /********************************************************************/
        /* Look for translatable data in segment                            */
        /********************************************************************/
        sI = 0;
        while ( sI < (SHORT)pParseData->usOutBufUsed )
        {
          if ( (pParseData->abCharBuf[sI] == CH_TAG) ||
               ((pParseData->abOutBuf[sI] == SPACE) && !fOtherChars) )
          {
            pParseData->pLastTagPos = pParseData->abOutBuf + sI;
          } /* endif */

          if ( (pParseData->abCharBuf[sI] == CH_NO) &&
               (pParseData->abOutBuf[sI] != SPACE) )
          {
            fOtherChars = TRUE;
          } /* endif */

          if ( pParseData->abCharBuf[sI] == CH_YES )
          {
            break;
          } /* endif */

          sI++;
        } /* endwhile */
      }

      /****************************************************************/
      /* Split segment if last tag position has been detected         */
      /****************************************************************/
      if ( pParseData->pLastTagPos != NULL )
      {
        pbTemp = pParseData->pLastTagPos + 1;
        usRest = (USHORT)(pParseData->usOutBufUsed - (pbTemp - pParseData->abOutBuf));

        if ( usRest )
        {
          memcpy( pParseData->abTempBuf, pbTemp, usRest );
          memcpy( pParseData->abTempSegBuf,
                  pParseData->abSegBuf + (pbTemp - pParseData->abOutBuf),
                  usRest );
          memcpy( pParseData->abTempCharBuf,
                  pParseData->abCharBuf + (pbTemp - pParseData->abOutBuf),
                  usRest );
          pParseData->usOutBufUsed = (USHORT)(pParseData->usOutBufUsed - usRest);
          TempSegType = pParseData->SegType;
        } /* endif */
        pParseData->pFirstTagPos = NULL;

        usRC = RTFWriteSegment( pParseData );
        if ( !usRC )
        {
          pParseData->pOutBuf = pParseData->abOutBuf;
          pParseData->pLastTagPos = NULL;
          pParseData->pSegBuf = pParseData->abSegBuf;
          pParseData->pCharBuf = pParseData->abCharBuf;
          pParseData->usOutBufUsed = 0;
          pParseData->usLinePos    = 0;
          pParseData->SegType = NewSegType;
          pParseData->fTranslDataInSegment = FALSE;
        } /* endif */

        if ( !usRC && usRest )
        {
          memcpy( pParseData->abOutBuf, pParseData->abTempBuf, usRest );
          memcpy( pParseData->abSegBuf, pParseData->abTempSegBuf, usRest );
          memcpy( pParseData->abCharBuf, pParseData->abTempCharBuf, usRest );
          pParseData->usOutBufUsed = usRest;
          pParseData->pOutBuf = pParseData->abOutBuf + usRest;
          pParseData->pSegBuf = pParseData->abSegBuf + usRest;
          pParseData->pCharBuf = pParseData->abCharBuf + usRest;
          pParseData->SegType = TempSegType;
          pParseData->fTranslDataInSegment = RTFCheckForTranslData( pParseData );
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Append new byte to segment buffer                               */
  /*******************************************************************/
  if ( !usRC )
  {
    if ( CharType == CH_TAG )
    {
      pParseData->pLastTagPos = pParseData->pOutBuf;
      if ( pParseData->pFirstTagPos == NULL)
      {
        pParseData->pFirstTagPos = pParseData->pOutBuf;
      } /* endif */
    } /* endif */
    *(pParseData->pOutBuf)++ = bAddByte;
    if ( CharType == CH_YES )
    {
      *(pParseData->pSegBuf)++ = bAddByte;
    }
    else
    {
      *(pParseData->pSegBuf)++ = chSegChar[bAddByte];
    } /* endif */
    *(pParseData->pCharBuf)++ = (BYTE)CharType;
    pParseData->usOutBufUsed++;
    pParseData->usLinePos++;
    if ( CharType == CH_YES )
    {
      pParseData->fTranslDataInSegment = TRUE;
      pParseData->pFirstTagPos = NULL;
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function AddToSegment */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ParseNextChar       Retrieve the next input character
//------------------------------------------------------------------------------
// Function call:     ParseNextChar( PPARSEDATA pParseData, PUSHORT pusRC );
//------------------------------------------------------------------------------
// Description:       Returns the next character from the input file or from
//                    the UNDO buffer and checks end-of-file condition.
//------------------------------------------------------------------------------
// Input parameter:   PPARSEDATA pParseData  ptr to parser global data structure
//                    PUSHORT    pusRC      return code (set in case of errors)
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR            function completed successfully
//                    other               OS/2 Dos calls error codes
//------------------------------------------------------------------------------
// Prerequesits:      Input file must be open.
//------------------------------------------------------------------------------
// Side effects:      Input buffer use count and bytes to read count are
//                    changed.
//------------------------------------------------------------------------------
// Function flow:     if UNDO buffer contains data then
//                       get character from UNDO buffer
//                    else
//                       if input buffer is empty read next data from input
//                          and check end-of-file condition
//                       if input buffer contains data and data is not eof
//                          return next byte from buffer
//                       else
//                          set return code to EOF_REACHED.
//                    endif
//                    update slider position if completion ratio has changed
//------------------------------------------------------------------------------
BYTE  ParseNextChar
(
  PPARSEDATA pParseData,               // ptr to parser global data structure
  PUSHORT    pusRC                    // return code (set in case of errors)
)
{
  register BYTE   bNextByte = 0;      // byte being returned
  USHORT usBytesToRead;               // number of bytes to read from file
  USHORT usRC;                        // return code of called functions
  USHORT usComplete;                  // completion ratio

  usComplete = pParseData->usPercentComplete;

  /*******************************************************************/
  /* Check if next character is to be taken from UNDO buffer or from */
  /* input buffer                                                    */
  /*******************************************************************/
  if ( pParseData->usUndoBufUsed )
  {
    /*****************************************************************/
    /* UNDO buffer is not empty: get character from this buffer      */
    /*****************************************************************/
    bNextByte = pParseData->abUndoBuf[--pParseData->usUndoBufUsed];
  }
  else
  {
    /*****************************************************************/
    /* Get character from input buffer                               */
    /*****************************************************************/

    /****************************************************************/
    /* Re-fill input buffer if half of the buffer is empty          */
    /*                                                              */
    /* Note: This is done to ensure that the buffer contains enough */
    /*       data to check for macros and tags using strcmp instead */
    /*       of reading the name of the macro or tag character for  */
    /*       character.                                             */
    /****************************************************************/
    if ( !pParseData->usBytesInBuffer && !pParseData->lBytesToRead )
    {
        /***********************************************************/
        /* Error: file has been processed completely               */
        /***********************************************************/
        *pusRC = EOF_REACHED;
    }
    else if ( (pParseData->usBytesInBuffer < (INBUF_SIZE / 2)) &&
               pParseData->lBytesToRead )
    {
      USHORT usRead;                 // number of bytes actually read

      // copy data in buffer to begin of buffer
      memmove( pParseData->abInBuf, pParseData->pInBuf,
               pParseData->usBytesInBuffer );

      // fill second half of buffer
      usBytesToRead = (USHORT)INBUF_SIZE - pParseData->usBytesInBuffer;
      usBytesToRead = (USHORT)min( (LONG)usBytesToRead,
                                   pParseData->lBytesToRead );
      usRC = UtlRead( pParseData->hInFile,
                      pParseData->abInBuf + pParseData->usBytesInBuffer,
                      usBytesToRead, &usRead, TRUE );
      if ( !usRC )
      {
        pParseData->pInBuf          = pParseData->abInBuf;
        pParseData->lBytesToRead    -= usRead;
        pParseData->usBytesInBuffer = (USHORT)(pParseData->usBytesInBuffer + usRead);

        /************************************************************/
        /* Special handling for RTF files from Japan:               */
        /* Some of these files contained 0x00 at several places.    */
        /* This character is misinterpreted later on in the         */
        /* tokenizer as end of input data.                          */
        /* The 0x00 character is changed to a SPACE character.      */
        /************************************************************/
        {
          USHORT usI;

          for ( usI = 0; usI < pParseData->usBytesInBuffer; usI++ )
          {
            if ( pParseData->abInBuf[usI] == EOS )
            {
              pParseData->abInBuf[usI] = SPACE;
            } /* endif */
          } /* endfor */
        } /* end "Special handling for ..." */
      }
      else
      {
        *pusRC = usRC;
      } /* endif */
    } /* endif */

    /*****************************************************************/
    /* Get character from input buffer                               */
    /*****************************************************************/
    if ( pParseData->usBytesInBuffer )
    {
      bNextByte = *(pParseData->pInBuf)++;
      pParseData->usBytesInBuffer--;
      if ( bNextByte == END_OF_FILE )
      {
        *pusRC = EOF_REACHED;
        usComplete = 100;           // we are through ...
      }
      else
      {
        ULONG ulComplete;
        ULONG ulDone = (ULONG)pParseData->lTotalBytes -
                       (ULONG)pParseData->lBytesToRead -
                       (ULONG)pParseData->usBytesInBuffer;
        // avoid overflow if ulDone contains large numbers
        if ( ulDone <= (ULONG)((ULONG)0XFFFFFFFF / (ULONG)100) )
        {
          ulComplete = ulDone * (ULONG)100 / (ULONG)pParseData->lTotalBytes;
        }
        else
        {
          ulComplete = ulDone / (ULONG)pParseData->lTotalBytes * (ULONG)100;
        } /* endif */
        usComplete = (USHORT)ulComplete;
      } /* endif */
    }
    else
    {
      usComplete = 100;               // we are through ...
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Update slider if slider complete percentage has changed         */
  /*******************************************************************/
  if ( (pParseData->hwndProcWin != NULLHANDLE) &&
       (usComplete != pParseData->usPercentComplete) )
  {
    WinSendMsg( pParseData->hwndProcWin, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(usComplete), NULL );
    UtlDispatch();
  } /* endif */

  return( bNextByte );
} /* end of function ParseNextChar */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UndoChar            Put character back into input buffer
//------------------------------------------------------------------------------
// Function call:     UndoChar( PPARSEDATA pParseData, BYTE bChar );
//------------------------------------------------------------------------------
// Description:       Puts given character into undo buffer, from there it is
//                    retrieved with the next ParseNextChar call
//------------------------------------------------------------------------------
// Input parameter:   PPARSEDATA pParseData   ptr to parser global data struct.
//                    BYTE       bChar       character put back to input buffer
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if undo buffer is not full then
//                       add byte to undo buffer
//                    endif
//------------------------------------------------------------------------------
VOID UndoChar
(
  PPARSEDATA pParseData,               // ptr to parser global data structure
  BYTE       bChar                    // character put back to input buffer
)
{
  if ( pParseData->usUndoBufUsed < MAX_SEG_SIZE )
  {
    pParseData->abUndoBuf[pParseData->usUndoBufUsed++] = bChar;
  } /* endif */
} /* end of function UndoChar */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     WriteSegment
//------------------------------------------------------------------------------
// Function call:     WriteSegment( PPARSEDATA pParseData );
//------------------------------------------------------------------------------
// Description:       Writes the segment contained in the segment buffer to
//                    disk using PutSegment
//------------------------------------------------------------------------------
// Input parameter:   PPARSEDATA pParseData   ptr to parser global data struct.
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR            function completed successfully
//                    other               OS/2 Dos calls error codes
//------------------------------------------------------------------------------
// Function flow:     change segment type if segment contains no translatable
//                      data
//                    if segment contains translatable data then
//                      use morphologic tokenization to look for segment breaks
//                      split segment at sentence borders
//                    endif
//                    move trailing curly braces to begin of next segment
//                    write current segment
//                    write trailing data segment
//                    start a new segment
//------------------------------------------------------------------------------
USHORT RTFWriteSegment
(
  PPARSEDATA pParseData                // ptr to parser global data structure
)
{
  USHORT usRC = 0;                    // internal return code
  PFLAGOFFSLIST pTerm;                // pointer for term list processing
  USHORT usStartOffs;                 // start offset of segment data
  SHORT  sTrailLen = 0;               // length of segment trailer

  /*******************************************************************/
  /* Exit immediately if segment is empty                            */
  /*******************************************************************/
  if ( pParseData->usOutBufUsed == 0)
  {
    return( usRC );
  } /* endif */

  /*****************************************************************/
  /* Change segment type if segments does not contain translatable */
  /* information                                                   */
  /*****************************************************************/
  if ( (pParseData->SegType == TRANSL_SEGMENT) &&
       !pParseData->fTranslDataInSegment )
  {
    pParseData->SegType = PROTECT_SEGMENT;
  } /* endif */

  /********************************************************************/
  /* Remove temporarly tagging info from end of segment               */
  /********************************************************************/
  if ( (pParseData->SegType == TRANSL_SEGMENT) &&
       (pParseData->pFirstTagPos != NULL) )
  {
    /**************************************************************/
    /* Split segment at first tag position                        */
    /**************************************************************/
    PBYTE pbTemp = pParseData->pFirstTagPos;
    sTrailLen = (SHORT)(pParseData->usOutBufUsed - (pbTemp - pParseData->abOutBuf));

    if ( sTrailLen > 0 )
    {
      memcpy( pParseData->abTempBuf2, pbTemp, sTrailLen );
      memcpy( pParseData->abTempSegBuf2,
              pParseData->abSegBuf + (pbTemp - pParseData->abOutBuf),
              sTrailLen );
      pParseData->usOutBufUsed = (USHORT)(pParseData->usOutBufUsed  - sTrailLen);
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Check for segments breaks within segment using morphologic      */
  /* functions                                                       */
  /*******************************************************************/
  if ( pParseData->SegType == TRANSL_SEGMENT )
  {
    usRC = RTFMorphTokenize( pParseData );
    if ( usRC == NO_ERROR )
    {
      pTerm  = (PFLAGOFFSLIST)pParseData->pTermList;
      usStartOffs = pTerm->usOffs;
      while ( (pTerm->usOffs != 0) ||
              (pTerm->usLen  != 0) ||
              (pTerm->lFlags != 0L ) )
      {
        if ( pTerm->lFlags & TF_NEWSENTENCE )
        {
          /***********************************************************/
          /* Segment starts at next term                             */
          /***********************************************************/
          pTerm++;
          if ( pTerm->usOffs != 0 )
          {
            /***********************************************************/
            /* Segment break found within data, write segment up to    */
            /* segment break                                          */
            /***********************************************************/
            usRC = PutSegment( pParseData,
                               FALSE,
                               (PSZ)(pParseData->abOutBuf + usStartOffs),
                               (USHORT)(pTerm->usOffs - usStartOffs) );
            usStartOffs = pTerm->usOffs;
          } /* endif */
        } /* endif */
        if ( (pTerm->usOffs != 0) ||
             (pTerm->usLen  != 0) ||
             (pTerm->lFlags != 0L ) )
        {
          pTerm++;
        } /* endif */
      } /* endwhile */

      /***************************************************************/
      /* Shift remaining data to begin of segment buffer             */
      /***************************************************************/
      if ( usStartOffs != 0 )
      {
        memmove( pParseData->abOutBuf, pParseData->abOutBuf + usStartOffs,
                 pParseData->usOutBufUsed - usStartOffs );
        pParseData->usOutBufUsed = (USHORT)(pParseData->usOutBufUsed - usStartOffs);
      } /* endif */
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* write data of segment to output file                            */
  /*******************************************************************/
  if ( !usRC && pParseData->usOutBufUsed )
  {
    usRC = PutSegment( pParseData,
                       (pParseData->SegType == PROTECT_SEGMENT),
                       (PSZ)pParseData->abOutBuf,
                       pParseData->usOutBufUsed );
  } /* endif */

  /*******************************************************************/
  /* Handle trailing data of segment                                 */
  /*******************************************************************/
  if ( !usRC && (sTrailLen > 0) )
  {
    /*******************************************************************/
    /* write data of segment trailer to output file                    */
    /*******************************************************************/
    pParseData->SegType = PROTECT_SEGMENT;
    usRC = PutSegment( pParseData,
                       TRUE,
                       (PSZ)pParseData->abTempBuf2,
                       sTrailLen );
  } /* endif */

  /*******************************************************************/
  /* start new segment                                               */
  /*******************************************************************/
  if ( !usRC )
  {
    pParseData->pOutBuf = pParseData->abOutBuf;
    pParseData->pLastTagPos = NULL;
    pParseData->pFirstTagPos = NULL;
    pParseData->pSegBuf = pParseData->abSegBuf;
    pParseData->pCharBuf = pParseData->abCharBuf;
    pParseData->usOutBufUsed = 0;
    pParseData->usLinePos    = 0;
    pParseData->fTranslDataInSegment = FALSE;
    pParseData->SegType = NO_SEGMENT;
  } /* endif */

  pParseData->fBkmkInSegment = FALSE;

  return( usRC );
} /* end of function RTFWriteSegment */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     PutSegment
//------------------------------------------------------------------------------
// Function call:     PutSegment( PSZ PPARSEDATA pParseData );
//------------------------------------------------------------------------------
// Description:       Physically writes a segment to disk.
//------------------------------------------------------------------------------
// Input parameter:   PPARSEDATA pParseData   ptr to parser global data struct.
//                    BOOL       fProtected  protected segment flag
//                    PSZ        pszSegment  ptr to start of segment
//                    USHORT     usLen       length of segment
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR            function completed successfully
//                    other               OS/2 Dos calls error codes
//------------------------------------------------------------------------------
// Function flow:     write segment start tag
//                    write segment data
//                    write CRLF
//                    write segment end tag
//------------------------------------------------------------------------------
USHORT PutSegment
(
  PPARSEDATA pParseData,                // ptr to parser global data structure
  BOOL       fProtected,               // protected segment flag
  PSZ        pszSegment,               // ptr to start of segment
  USHORT     usLen                     // length of segment
)
{
  PSZ        pszEndTag;                // ptr to string of segment end tag
  USHORT     usRC = 0;                 // internal return code
  USHORT     usRemaining;              // remaining length of segment
  USHORT     usWriteLen;               // length of data being written
  PSZ        pszRemaining;             // ptr to remaining segment data

  // do tag cleanup for translatable segments
  if ( !fProtected )
  {
    BOOL fSegChanged = FALSE;
    CHAR chTemp = NULC;

    // ensure null terminated string
    chTemp = pszSegment[usLen];
    pszSegment[usLen] = EOS;

    // tag removal
    fSegChanged = RTFRemoveTags( pszSegment, pParseData->szRemoveTagsBuffer,
                                 pParseData->chRemoveTags, NULL, NULL );

    // restore string end delimiter
    pszSegment[usLen] = chTemp;

    // use changed segment if tags have been removed
    if ( fSegChanged )
    {
      pszSegment = pParseData->szRemoveTagsBuffer;
      usLen = (USHORT)strlen( pszSegment );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Write segment start                                              */
  /********************************************************************/
  pParseData->ulSegNum++;
  sprintf( (PSZ)pParseData->abTagBuf, (fProtected) ? pParseData->pQFNTag :
                                               pParseData->pQFFTag,
                                               pParseData->ulSegNum );
  usRC = RTFWriteInUTF16( pParseData, pParseData->hOutFile,
                          (PSZ)pParseData->abTagBuf, (ULONG)strlen((PSZ)pParseData->abTagBuf),
                          TRUE );

  /********************************************************************/
  /* Write segment data, insert CRLF in translatable segments if      */
  /* line length is too long                                          */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    if ( fProtected )
    {
      /****************************************************************/
      /* Write protected segments wihtout caring for line length      */
      /****************************************************************/
      usRC = RTFWriteInUTF16( pParseData, pParseData->hOutFile,
                              pszSegment, (ULONG)usLen, TRUE );
    }
    else
    {
      ULONG ulSegLength = usLen;

      usRemaining = usLen;
      pszRemaining = pszSegment;
      while ( (usRC == NO_ERROR) && usRemaining )
      {
        if ( (usRemaining > 75) && ((ulSegLength + 2) < MAX_SEG_SIZE) )
        {
          /************************************************************/
          /* look for a place to insert CRLF                          */
          /************************************************************/
          usWriteLen = 65;             // set starting point
          while ( (usWriteLen < usRemaining) &&
                  (pszRemaining[usWriteLen] != SPACE) )
          {
            usWriteLen++;
          } /* endwhile */

          /************************************************************/
          /* Check if found position is near the end of the data      */
          /************************************************************/
          if ( (pszRemaining[usWriteLen] == SPACE) &&
               ((usRemaining - usWriteLen) > 3) )
          {
            usWriteLen++;              // insert CRLF right behind space
          }
          else
          {
            usWriteLen = usRemaining;
          } /* endif */
        }
        else
        {
          /************************************************************/
          /* Write rest of segment without addition CRLFs             */
          /************************************************************/
          usWriteLen = usRemaining;
        } /* endif */
        usRC = RTFWriteInUTF16( pParseData, pParseData->hOutFile, pszRemaining,
                         (ULONG)usWriteLen, TRUE );
        usRemaining  = usRemaining - usWriteLen;
        pszRemaining += usWriteLen;

        /**************************************************************/
        /* Insert CRLF if not complete                                */
        /**************************************************************/
        if ( (usRC == NO_ERROR) && usRemaining )
        {
          usRC = RTFWriteInUTF16( pParseData, pParseData->hOutFile,
                                  CRLF_STR, (ULONG)strlen(CRLF_STR), TRUE );
          ulSegLength += 2;            // length of segment has been increased
        } /* endif */
      } /* endwhile */
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Write CRLF combination to end of segment                        */
  /*******************************************************************/
  if ( usRC == NO_ERROR )
  {
    usRC = RTFWriteInUTF16( pParseData, pParseData->hOutFile,
                            CRLF_STR, (ULONG)strlen(CRLF_STR), TRUE );
  } /* endif */

  /*******************************************************************/
  /* Write segment end tag to output file                            */
  /*******************************************************************/
  if ( usRC == NO_ERROR )
  {
    if ( fProtected )
    {
      pszEndTag = pParseData->pEQFNTag;
    }
    else
    {
       pszEndTag = pParseData->pEQFFTag;
    } /* endif */
    usRC = RTFWriteInUTF16( pParseData, pParseData->hOutFile, pszEndTag,
                            (ULONG)strlen(pszEndTag), TRUE );
  } /* endif */

  /********************************************************************/
  /* Return to caller                                                 */
  /********************************************************************/
  return( usRC );

} /* end of function PutSegment */


// converts the given data to UTF16 and writes the result to the given output file
USHORT RTFWriteInUTF16
(
  PPARSEDATA pParseData,               // ptr to parser global data structure
  HFILE      hOutFile,                 // handle of output file
  PSZ        pszData,                  // ptr to start of data
  ULONG      ulLen,                    // length of the data
  BOOL       fMsg                      // show messagebox flag
)
{
  USHORT usRC = NO_ERROR;
  ULONG  ulBytesWritten = 0;
  ULONG  ulCharsConverted = 0;
  ULONG  ulBytesToWrite = 0;

  // convert the data to UTF16
  if ( pParseData->pConvTable )
  {
    LONG lRC = 0;

    ulCharsConverted = UtlDirectAnsi2UnicodeBuf( pszData, pParseData->UTF16Buffer, ulLen,
                                              pParseData->ulSrcAnsiCP, TRUE, &lRC, NULL );
    usRC = (USHORT)lRC;
    ulBytesToWrite = ulCharsConverted * sizeof(CHAR_W);
  }
  else
  {
    ulCharsConverted = ASCII2UnicodeBuf( pszData, pParseData->UTF16Buffer,
                                         ulLen, pParseData->ulSrcOemCP );
    ulBytesToWrite = ulCharsConverted * sizeof(CHAR_W);
  } /* endif */

  // write converted data to output file
  usRC = UtlWriteL( hOutFile, pParseData->UTF16Buffer, ulBytesToWrite, &ulBytesWritten, fMsg );

  if ( !usRC && (ulBytesWritten != ulBytesToWrite) )
  {
    usRC = ERROR_DISK_FULL;
  } /* endif */

  return( usRC );
} /* end of function RTFWriteInUTF16 */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UnparseRTF
//------------------------------------------------------------------------------
// Function call:     UnparseRTF( PSZ pszInFile );
//------------------------------------------------------------------------------
// Description:       Postprocessing of exported documents after the
//                    segmentation tags (:QFx) have been removed from the
//                    document by the text unsegmentation utility
//                    Removes inserted blanks from the document, converts
//                    documents to ANSI code page and decodes special
//                    characters.
//------------------------------------------------------------------------------
// Input parameter:   PSZ  pszInFile        name of input file
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE        no errors during processing
//                    FALSE       errors occured
//------------------------------------------------------------------------------
// Side effects:      Input file is overwritten with the converted file.
//------------------------------------------------------------------------------
// Function flow:     copy input file name to tempfile name buffer
//                    replace extension of tempfile
//                    initialize global parser data structure
//                    open input file
//                    open output file
//                    get size of input file
//                    check type of code conversion is required
//                    while ok and not all bytes of input file processed
//                       fill input buffer
//                       while there are bytes in input buffer
//                         get current byte from buffer
//                         convert byte to ANSI code page
//                         if processing of tag names is active then
//                            if byte is alphanumeric then
//                               write byte to output file
//                            elsif byte is SPACE then
//                               end tag name processing state
//                            elsif current byte is START_CTRLWORD then
//                               begin tag name processing state
//                               write byte to output file
//                            else
//                               write byte to output file
//                               end tag name processing state
//                            endif
//                         else
//                            switch current byte
//                               case START_CTRLWORD
//                                  begin tag name processing state
//                                  write byte to output file
//                               default:
//                                  if current byte belongs to a field then
//                                     write byte to putput file
//                                  elsif byte is a special character then
//                                     write control symbol for character
//                                  elsif byte is greater 127 then
//                                     decode byte and write it to output file
//                                  else
//                                     write byte to putput file
//                                  endif
//                            endswitch
//                         endif
//                      endwhile
//                    endwhile
//                    write any pending data in output buffer
//                    close output file
//                    delete output file in case of errors
//                    close input file
//                    delete input file and rename tempfile to input file
//------------------------------------------------------------------------------
BOOL UnparseRTF ( PSZ pszInFile, PSZ pszTagTable  )
{
  CHAR        szTempFile[CCHMAXPATH]; // buffer for temporary file name
  USHORT      usRC = 0;               // function return code
  BYTE        bCurrent;               // currently processed byte
  PBYTE       pTemp;                  // temporary character pointer
  USHORT      usTagLen = 0;           // length of current tag
  PDESTSTACK  pDestStack = NULL;      // ptr into destination stack
  BOOL        fField = FALSE;         // TRUE = processing field data
  BOOL        fField_F_TAG = FALSE;   // flag for dbcs processing in F_TAG field
  BOOL        fisDBCS = FALSE;        // TRUE = last character was first DBCS char
  PCHAR       pConvTable = NULL;      // ptr to code conversion table
  enum { INTAG, OUTTAG } State = OUTTAG;
  PPARSEDATA  pParseData = NULL;       // points to parser data structure
  BOOL        fIsTagChar;              // TRUE = current character is part of a tag
  PCTRLWORD   pCtrlWord;               // pointer to control words
#ifndef _WINDOWS
  DOSVALUE    usCodePage;             // country code
  DOSVALUE    sCountryRc;             // return value
#endif
  BOOL        fInMacro = FALSE;        // TRUE = we are processing macro parms
  HFILE       hfSource = NULLHANDLE;   // handle of document source file
  USHORT      usNumOfTags = 0;         // number of RTF tags
  CHAR szObjName[MAX_EQF_PATH];        // buffer for document object name
  CHAR szLang[MAX_LANG_LENGTH];        // buffer for document target language
  UINT        uiCodePage;              // codepage

  UtlAlloc( (PVOID *) &pParseData, 0L, (LONG)sizeof(PARSEDATA), ERROR_STORAGE );
  if ( pParseData )
  {
    State = OUTTAG;
    pParseData->usLinePos = 0;
    RTFParseGetCP(pszInFile, &pParseData->ulSrcOemCP, &pParseData->ulTgtOemCP,
                           &pParseData->ulSrcAnsiCP, &pParseData->ulTgtAnsiCP);
  }
  else
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  /********************************************************************/
  /* Setup name of source file and open it                            */
  /* (we need the document source for picture data)                   */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    CHAR szFolder[MAX_FILESPEC];       // buffer for folder name
    CHAR szDoc[MAX_FILESPEC];          // buffer for document name
    PSZ  pszTemp;                      // buffer for file name processing
    USHORT usOpenAction;           // action performed by DosOpen

    // isolate document and folder name
    strcpy( szTempFile, pszInFile );
    pszTemp = UtlSplitFnameFromPath( szTempFile ); // remove document name
    strcpy( szDoc, pszTemp );
    pszTemp = UtlSplitFnameFromPath( szTempFile ); // remove sub directory
    pszTemp = UtlSplitFnameFromPath( szTempFile ); // remove folder name
    strcpy( szFolder, pszTemp );

    // setup name of source of document
    UtlMakeEQFPath( szTempFile, *pszInFile, DIRSOURCEDOC_PATH, szFolder );
    strcat( szTempFile, BACKSLASH_STR );
    strcat( szTempFile, szDoc );

    // open document source file
    usRC = UtlOpen( szTempFile, &hfSource, &usOpenAction, 0L, FILE_NORMAL,
                    FILE_OPEN, OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                    0L, TRUE );
  } /* endif */

  // setup temporary file name
  if ( usRC == NO_ERROR )
  {
    RTFMakeTempName( szTempFile, pszInFile );
  } /* endif */

  // initialization
  if ( !usRC )
  {
    memset( pParseData, 0, sizeof(PARSEDATA) );
    pParseData->pOutBuf = pParseData->abOutBuf;
    pDestStack = pParseData->DestStack;
    fField = FALSE;
    fField_F_TAG = FALSE;
    pParseData->fBidiOffMode = FALSE;
  } /* endif */


  // get language dependent settings
  if ( !usRC )
  {
    CHAR szFolder[MAX_FILESPEC];      // buffer for folder name
    CHAR szDoc[MAX_FILESPEC];         // buffer for document name
    PSZ  pszTemp;                     // ptr to for processing of path name

    // setup document object name
    strcpy( szObjName, pszInFile );
    pszTemp = UtlSplitFnameFromPath( szObjName );   // remove document name
    strcpy( szDoc, pszTemp );
    pszTemp = UtlSplitFnameFromPath( szObjName );   // remove STARGET dir
    pszTemp = UtlSplitFnameFromPath( szObjName );   // remove folder name
    strcpy( szFolder, pszTemp );
    UtlMakeEQFPath( szObjName, *pszInFile, SYSTEM_PATH, szFolder );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, szDoc );

    // get document target language
    szLang[0] = EOS;
    DocQueryInfo( szObjName, NULL, NULL, NULL, szLang, FALSE );


    /*******************************************************************/
    /* set lang codepage                                               */
    /*******************************************************************/
    if (!RTFSetLangCodePage(szLang, &uiCodePage))
    {
       CHAR szReplace[10];
       PSZ pszReplace;

       sprintf(szReplace, "%d", uiCodePage);

       pszReplace = szReplace;

       UtlError( WARNING_NO_LOCALE, MB_OK, 1, &pszReplace, EQF_WARNING );
    }

    // get language dependent settings
    RTFGetSettings( pszTagTable, szLang, pParseData );
  } /* endif */


  /*******************************************************************/
  /* get number of RTF tags and sort table if necessary              */
  /*                                                                 */
  /* Note: The RTF tag table as defined in the EQF_RTF.H file must   */
  /*       be sorted to allow binary search.                         */
  /*******************************************************************/
  if ( !usRC )
  {
    BOOL fNeedSort = FALSE;             // assume RTF tag table is sorted
    usNumOfTags = 0;                    // start with first tag
    while ( CtrlWords[usNumOfTags].szCtrlWord[0] )
    {
      if ( usNumOfTags &&
           (StringCompare( &(CtrlWords[usNumOfTags-1]),
                           &(CtrlWords[usNumOfTags]) ) > 0 ) )
      {
        fNeedSort = TRUE;
      } /* endif */
      usNumOfTags++;
    } /* endwhile */

    if ( fNeedSort )
    {
      qsort( CtrlWords, usNumOfTags, sizeof(CTRLWORD), StringCompare );
    } /* endif */
  } /* endif */


  // do any necessary BIDI RTL/LTR processing
  if ( !usRC && pParseData->fBidi )
  {
    if ( pParseData->usCodePage )
    {
      UtlQueryCharTableEx( ASCII_TO_ANSI_TABLE, (PUCHAR *)&pConvTable,
                           pParseData->usCodePage );
    }
    else
    {
      UtlQueryCharTableForDocConv( pszInFile, &pConvTable, pParseData->ulTgtOemCP );
    } /* endif */

  } /* endif */

  /*******************************************************************/
  /* open input file                                                 */
  /*******************************************************************/
  if ( !usRC )
  {
     USHORT      usOpenAction;           // action performed by DosOpen
     usRC = UtlOpen( pszInFile,
                     &pParseData->hInFile,
                     &usOpenAction, 0L,
                     FILE_NORMAL,
                     FILE_OPEN,
                     OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                     0L,
                     TRUE );
  } /* endif */

  /*******************************************************************/
  /* open output file                                                */
  /*******************************************************************/
  if ( !usRC )
  {
     USHORT      usOpenAction;           // action performed by DosOpen
     usRC = UtlOpen( szTempFile,
                     &pParseData->hOutFile,
                     &usOpenAction, 0L,
                     FILE_NORMAL,
                     FILE_TRUNCATE | FILE_CREATE,
                     OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                     0L,
                     TRUE );
  } /* endif */

  /*******************************************************************/
  /* get size of input file                                          */
  /*******************************************************************/
  if ( !usRC )
  {
     FILESTATUS stStatus;              // File status information
     usRC = UtlQFileInfo( pParseData->hInFile,
                          1,
                          (PBYTE)&stStatus,
                          sizeof(FILESTATUS),
                          TRUE );
     pParseData->lBytesToRead = stStatus.cbFile;
  } /* endif */

  /*******************************************************************/
  /* Read first part of text to buffer to get type of code conversion*/
  /* required                                                        */
  /*******************************************************************/
  if ( !usRC )
  {
    /****************************************************************/
    /* Read first 512 bytes from input file                         */
    /****************************************************************/
    usRC = UtlRead( pParseData->hInFile, pParseData->abInBuf,
                    512, &pParseData->usBytesInBuffer, TRUE );
    pParseData->usBytesInBuffer = 0;
    pParseData->abInBuf[512] = EOS;

    /**************************************************************/
    /* Get type of code conversion                                */
    /**************************************************************/
    if ( IsAPLang(szLang) )
    {
      pConvTable = NULL;           // no code conversion in DBCS environment
    }
    else if ( RTFContainsAnsiTag( (PSZ)pParseData->abInBuf ) )
    {
#if defined(R004203_CONVERSIONCP)
      if ( pParseData->usCodePage )
      {
        UtlQueryCharTableEx( ASCII_TO_ANSI_TABLE, (PUCHAR *)&pConvTable,
                             pParseData->usCodePage );
      }
      else
      {
        UtlQueryCharTableForDocConv( pszInFile, &pConvTable, pParseData->ulTgtOemCP );
      } /* endif */
#elif defined(_TP51)
      UtlQueryCharTableEx( ASCII_TO_ANSI_TABLE, &pConvTable,
                           pParseData->usCodePage );
#endif
    }
    else
    {
      pConvTable = NULL;           // no code conversion table available
    } /* endif */

  } /* endif */

  /****************************************************************/
  /* Position file read pointer back to begin of file             */
  /****************************************************************/
  if ( !usRC )
  {
    ULONG     ulNewPos;                // file pointer position

    UtlChgFilePtr( pParseData->hInFile, 0L, FILE_BEGIN, &ulNewPos, FALSE );
  } /* endif */

  // initialize last char prop

  //btxxx
  pParseData->fWasExtTagChar = 0;

  /*******************************************************************/
  /* Mainloop of parser                                              */
  /*******************************************************************/
  while ( !usRC && pParseData->lBytesToRead )
  {
    // read first block of data
    pParseData->usBytesInBuffer = 0;
    usRC = RTFBufInpReFill( pParseData->hInFile,
                            pParseData->abInBuf, &pParseData->pInBuf,
                            &pParseData->usBytesInBuffer,
                            &pParseData->lBytesToRead );

    // Process data in input buffer
    while ( !usRC && pParseData->usBytesInBuffer )

    {
      // re-fill input buffer when half full to ensure that there is
      // always enough data available to look ahead
      usRC = RTFBufInpReFill( pParseData->hInFile,
                              pParseData->abInBuf, &pParseData->pInBuf,
                              &pParseData->usBytesInBuffer,
                              &pParseData->lBytesToRead );

      /**************************************************************/
      /* Get current character                                      */
      /**************************************************************/
      if ( !usRC )
      {
        BOOL fIgnoreCharacter = FALSE;

        bCurrent = *pParseData->pInBuf++;

        fIsTagChar = FALSE;

        pParseData->usBytesInBuffer--;

        /**************************************************************/
        /* Convert character using selected conversion table          */
        /**************************************************************/
        if ( pConvTable )
        {
          bCurrent = pConvTable[bCurrent];
        } /* endif */

        /**************************************************************/
        /* Process character depending on current processing state    */
        /**************************************************************/
        if ( fisDBCS )
        {
          fisDBCS = FALSE;
          if ( fField && !fField_F_TAG)
          {
            /******************************************************/
            /* No encoding of DBCS characters inside fields and   */
            /* similar groups                                     */
            /******************************************************/
            usRC = RTFWriteBuffered( pParseData, bCurrent, fIsTagChar );

          }
          else
          {
            sprintf( (PSZ)pParseData->abTempBuf, "\\\'%2.2x", bCurrent );
            pTemp = pParseData->abTempBuf;
            while ( !usRC && *pTemp )
            {
              usRC = RTFWriteBuffered( pParseData, *pTemp++, fIsTagChar );
            } /* endwhile */
          } /* endif */
          fisDBCS = FALSE;
        }
        else
        {
          /**************************************************************/
          /* For INTAG_STATE add current character to tag name          */
          /**************************************************************/
          if ( State == INTAG )
          {
            if ( isalpha( bCurrent ) )
            {
              if ( usTagLen < MAX_CTRL_WORD )
              {
                pParseData->szControlWord[usTagLen++] = bCurrent;
              } /* endif */
            }
            else if ( bCurrent == START_CTRLWORD )
            {
              State = INTAG;
            }
            else
            {
              State = OUTTAG;
            } /* endif */

            if ( (State == OUTTAG) || (bCurrent == START_CTRLWORD) )
            {
              pParseData->szControlWord[usTagLen] = EOS;

              /*************************************************/
              /* Look up control word in control word table    */
              /*************************************************/
              {
                PSZ pszControlWord = pParseData->szControlWord;
                if ( (pszControlWord[0] == '*') && (pszControlWord[1] == '\\' ) )
                {
                  pszControlWord += 2;   // skip prefix
                } /* endif */
                pCtrlWord = (PCTRLWORD) bsearch( pszControlWord,
                                     CtrlWords, usNumOfTags,
                                     sizeof(CTRLWORD), StringICompare );
              }

              if ( pCtrlWord )
              {
                fField_F_TAG = FALSE;

                switch ( pCtrlWord->rtfID )
                {
                  case FIELD_RTFTAG :
                  case BKMKSTART_RTFTAG :
                  case BKMKEND_RTFTAG :
                    fField = TRUE;
                    break;
                  case PICT_RTFTAG :
                  case STYLESHEET_RTFTAG :
                  case FONTTBL_RTFTAG :
                  case COLORTBL_RTFTAG :
                  case INFO_RTFTAG :
                  case OBJDATA_RTFTAG :
                  case LISTTABLE_RTFTAG :
                  case LISTOVERRIDETABLE_RTFTAG:
                  case COLORSCHEMEMAPPING_RTFTAG:
                  case DATASTORE_RTFTAG:
                  case THEMEDATA_RTFTAG:
                    // Replace any reference by the actual data but process any
                    // control word data first
                    if ( (bCurrent == '-') || (bCurrent == '+') )
                    {
                      bCurrent = *pParseData->pInBuf++;
                      pParseData->usBytesInBuffer--;
                    } /* endif */
                    while ( isdigit(bCurrent) && pParseData->usBytesInBuffer)
                    {
                      bCurrent = *pParseData->pInBuf++;
                      pParseData->usBytesInBuffer--;
                    } /* endwhile */

                    usRC = RTFRefToData( pParseData, hfSource, &bCurrent,
                                         (SHORT)pCtrlWord->rtfID );
                    break;

                  case F_RTFTAG:
                   if ( pParseData->fBidi && (pParseData->sAddFontNo != 0) )
                   {
                     if ( (bCurrent == PLACEHOLDER2 ) &&
                          (*pParseData->pInBuf == PLACEHOLDER3) )
                     {
                       // skip placeholder in input buffer
                       bCurrent = *pParseData->pInBuf++;
                       pParseData->usBytesInBuffer--;
                       bCurrent = *pParseData->pInBuf++;
                       pParseData->usBytesInBuffer--;

                       // insert number of additional font
                       {
                         CHAR szFontNo[10];
                         PSZ  pszTemp;

                         sprintf( szFontNo, "%d", pParseData->sAddFontNo );
                         pszTemp = szFontNo;
                         while ( !usRC && (*pszTemp != EOS) )
                         {
                           usRC = RTFWriteBuffered( pParseData, *pszTemp, TRUE );
                           pszTemp++;
                         } /* endwhile */
                       }
                     } /* endif */
                   } /* endif */
                   fField_F_TAG = TRUE;
                   break;

                  case RTF_RTFTAG:
                   if ( pParseData->fBidi && (pParseData->chAddHeader[0] != EOS) )
                   {
                     PSZ pszTemp;

                     // skip any control word data
                     while ( isdigit(bCurrent) )
                     {
                       usRC = RTFWriteBuffered( pParseData, bCurrent, TRUE );
                       bCurrent = *pParseData->pInBuf++;
                       fIsTagChar = FALSE;
                       pParseData->usBytesInBuffer--;
                     } /* endwhile */

                     // write additional header information
                     pszTemp = pParseData->chAddHeader;
                     while ( !usRC && (*pszTemp != EOS) )
                     {
                       usRC = RTFWriteBuffered( pParseData, *pszTemp, TRUE );
                       pszTemp++;
                     } /* endwhile */
                   } /* endif */
                   break;

                 case PAR_RTFTAG:
                  // in BIDI mode: if additional paragraph data ...
                  if ( pParseData->fBidi && !pParseData->fBidiOffMode &&
                       (pParseData->chParTags[0] != EOS) )
                  {
                    // ... write additional paragraph data to output buffer
                    int i = 0;
                    while ( !usRC && (pParseData->chParTags[i] != EOS) )
                    {
                      usRC = RTFWriteBuffered( pParseData,
                                            pParseData->chParTags[i], TRUE );
                      i++;
                    } /* endwhile */
                  } /* endif */
                  break;

                case PARD_RTFTAG:
                  // insert additional paragraph tags right behind
                  // the pard tag
                  if ( pParseData->fBidi && !pParseData->fBidiOffMode &&
                       (pParseData->chParTags[0] != EOS) )
                  {
                    // ... write additional paragraph data to output buffer
                    int i = 0;
                    while ( !usRC && (pParseData->chParTags[i] != EOS) )
                    {
                      usRC = RTFWriteBuffered( pParseData,
                                            pParseData->chParTags[i],
                                            TRUE );
                      i++;
                    } /* endwhile */
                  } /* endif */
                  break;

                case U_RTFTAG:
                    // 200101 bt
                    // if the character followed by \u<number> is a whitespace
                    // character, do a conversion to hexadecimal notation
                    // otherwise Word traps
                    // Note: the \u tag is a unicode descriptor

                    // skip any control word data
                    while ( isdigit(bCurrent) || bCurrent == '-' )
                    {
                      usRC = RTFWriteBuffered( pParseData, bCurrent, TRUE );
                      bCurrent = *pParseData->pInBuf++;
                      fIsTagChar = FALSE;
                      pParseData->usBytesInBuffer--;
                    } /* endwhile */

                    if ( isspace(bCurrent) )
                    {
                      char szTemp[10];
                      PSZ pszTemp;

                      pszTemp = szTemp;

                      // write additional header information
                      sprintf(pszTemp, "\\'%2.2x", bCurrent);
                      while ( !usRC && (*pszTemp != EOS) )
                      {
                        usRC = RTFWriteBuffered( pParseData, *pszTemp, TRUE );
                        pszTemp++;
                      } /* endwhile */

                      bCurrent = *pParseData->pInBuf++;
                      pParseData->usBytesInBuffer--;
                    }
                    break;

                  case TROWD_RTFTAG:
                  case ROW_RTFTAG:
                    if (pParseData->fBidi && !pParseData->fBidiOffMode )
                    {
                      // 240401 bt
                      // if we have got BIDI processing, insert a \rtlrow command after
                      // \row or \trowd tags to reverse the column order
                      // skip any control word data

                      CHAR szInsertTag[] = "\\rtlrow";
                      PSZ pszInsertTag = szInsertTag;

                      while (*pszInsertTag)
                      {
                        usRC = RTFWriteBuffered( pParseData, *pszInsertTag, TRUE );
                        pszInsertTag++;
                      }
                    }
                    break;

                  case BIDIOFF_METATAG:
                    // remove tag from output buffer, should not be in exported file
                    {
                      char chRemoved = EOS;
                      do
                      {
                        chRemoved = RTFRemoveLastCharFromWriteBuffer( pParseData );
                      } while( (chRemoved != EOS) && (chRemoved != START_CTRLWORD) );
                      pParseData->fBidiOffMode = TRUE;
                      if ( bCurrent == SPACE) fIgnoreCharacter = TRUE;
                    }
                    break;

                  case BIDION_METATAG:
                    // remove tag from output buffer, should not be in exported file
                    {
                      char chRemoved = EOS;
                      do
                      {
                        chRemoved = RTFRemoveLastCharFromWriteBuffer( pParseData );
                      } while( (chRemoved != EOS) && (chRemoved != START_CTRLWORD) );
                      pParseData->fBidiOffMode = FALSE;
                      if ( bCurrent == SPACE) fIgnoreCharacter = TRUE;
                    }
                    break;

                } /* endswitch */
              } /* endif */

              usTagLen = 0;
            } /* endif */

            //bt
            if ( (State == INTAG) && (bCurrent != SPACE) )   // changed 1.17 bt
            {
               fIsTagChar = TRUE;
            } /* endif */
          } /* endif */

          /**************************************************************/
          /* Process current character                                  */
          /**************************************************************/
          if ( !fIgnoreCharacter )
          {
            switch ( bCurrent )
            {
              case START_CTRLWORD:
                if ( pParseData->pInBuf[0] == BEGIN_GROUP )
                {
                  // maybe start of a help macro, inhibit CRLF insertion
                  pParseData->fNoLineSplit = TRUE;
                  pParseData->fMacroMayFollow = TRUE;
                }
                else if ( pParseData->pInBuf[0] == END_GROUP )
                {
                  // maybe end of a help macro, enable CRLF insertion
                  pParseData->fNoLineSplit = FALSE;
                } /* endif */

                // insert CRLF if a help macro is starting (to avoid CRLF in
                // help macro when the line limit of 220 characters is exceeded)
                // a help macro starts with \{ followed by two or more characters
                if ( (pParseData->pInBuf[0] == BEGIN_GROUP) &&
                     isalpha(pParseData->pInBuf[1])         &&
                     isalpha(pParseData->pInBuf[2]) )
                {
                  pParseData->fForceNewLine = TRUE;
                } /* endif */

                // handle tags starting with \*\ correctly
                if ( (pParseData->pInBuf[0] == '*') &&
                     (pParseData->pInBuf[1] == START_CTRLWORD) )
                {
                  State = INTAG;
                  usTagLen = 0;
                  fIsTagChar = TRUE;
                  usRC = RTFWriteBuffered( pParseData, bCurrent, TRUE );
                  bCurrent = *pParseData->pInBuf++;
                  pParseData->usBytesInBuffer--;
                  usRC = RTFWriteBuffered( pParseData, bCurrent, TRUE );
                  pParseData->szControlWord[usTagLen++] = bCurrent;
                  bCurrent = *pParseData->pInBuf++;
                  pParseData->usBytesInBuffer--;
                  usRC = RTFWriteBuffered( pParseData, bCurrent, TRUE );
                  pParseData->szControlWord[usTagLen++] = bCurrent;
                }
                else if ( (pParseData->pInBuf[0] == '\'') &&   // hexadeciml encode digits?
                          isxdigit(pParseData->pInBuf[1]) &&
                          isxdigit(pParseData->pInBuf[2]) )

                {
                  // write hexadecimal encoded character to output buffer
                  usRC = RTFWriteBuffered( pParseData, bCurrent, TRUE );
                  bCurrent = *pParseData->pInBuf++;
                  pParseData->usBytesInBuffer--;
                  usRC = RTFWriteBuffered( pParseData, bCurrent, TRUE );
                  bCurrent = *pParseData->pInBuf++;
                  pParseData->usBytesInBuffer--;
                  usRC = RTFWriteBuffered( pParseData, bCurrent, TRUE );
                  bCurrent = *pParseData->pInBuf++;
                  pParseData->usBytesInBuffer--;
                  usRC = RTFWriteBuffered( pParseData, bCurrent, TRUE );
                  fIsTagChar = FALSE;
                  State = OUTTAG;
                }
                else
                {
                  // write backslash to output buffer
                  State = INTAG;
                  usTagLen = 0;
                  fIsTagChar = TRUE;
                  usRC = RTFWriteBuffered( pParseData, bCurrent, fIsTagChar );
                } /* endif */
                break;

              case BEGIN_GROUP:
                if ( pDestStack < (pParseData->DestStack + MAX_DEST_STACK) )
                {
                  pDestStack->fField = fField;
                  pDestStack++;
                } /* endif */
                usRC = RTFWriteBuffered( pParseData, bCurrent, fIsTagChar );
                break;

              case END_GROUP:
                if ( pDestStack > pParseData->DestStack )
                {
                  pDestStack--;
                  fField = pDestStack->fField;
                } /* endif */
                usRC = RTFWriteBuffered( pParseData, bCurrent, fIsTagChar );
                break;

              case END_PARMS:
                usRC = RTFWriteBuffered( pParseData, bCurrent, fIsTagChar );
                fInMacro = FALSE;
                break;

              case START_MACRO:
                // check if the name of a macro is following which
                // requires special handling
                {
                  SHORT sI = 0;              // loop index

                  while ( (HelpMacro[sI].Id != END_OF_MACROS) &&
                          (_strnicmp( HelpMacro[sI].szMacro,
                                     (PSZ)pParseData->pInBuf,
                                     HelpMacro[sI].sLength ) != 0) )
                  {
                    sI++;
                  } /* endwhile */
                  if ( HelpMacro[sI].Id != END_OF_MACROS )
                  {
                    fInMacro = TRUE;
                  }
                  else
                  {
                    pParseData->fMacroMayFollow = TRUE;
                  } /* endif */
                  usRC = RTFWriteBuffered( pParseData, bCurrent, fIsTagChar );
                }
                break;

              default:
                //KA if ( _isdbcs( bCurrent ) == DBCS_1ST )
                if (IsDBCSLeadByteEx(pParseData->ulTgtOemCP, (BYTE)bCurrent) == TRUE) //KA
                {
                  /********************************************************/
                  /* Handle first byte of a DBCS character                */
                  /********************************************************/
                  pParseData->fMacroMayFollow = FALSE;
                  if ( fField && !fField_F_TAG)
                  {
                    /******************************************************/
                    /* No encoding of DBCS characters inside fields and   */
                    /* similar groups                                     */
                    /******************************************************/
                    usRC = RTFWriteBuffered( pParseData, bCurrent, fIsTagChar );
                  }
                  else
                  {
                    sprintf( (PSZ)pParseData->abTempBuf, "\\\'%2.2x", bCurrent );
                    pTemp = pParseData->abTempBuf;
                    while ( !usRC && *pTemp )
                    {
                      usRC = RTFWriteBuffered( pParseData, *pTemp++, fIsTagChar );
                    } /* endwhile */
                  } /* endif */
                  fisDBCS = TRUE;             // prepare for following 2nd DBCS char
                }
                else if ( fField && !fField_F_TAG)
                {
                  /***************************************************/
                  /* No handling for special characters inside       */
                  /* fonttables, fields, stylesheets and bookmarks   */
                  /***************************************************/
                  usRC = RTFWriteBuffered( pParseData, bCurrent, fIsTagChar );

                }
                else if ( fInMacro && (bCurrent == '\'') )
                {
                  /***************************************************/
                  /* Do not convert to \rquote tag but hex encode    */
                  /* instead                                         */
                  /***************************************************/
                  sprintf( (PSZ)pParseData->abTempBuf, "\\\'%2.2x", bCurrent );
                  pTemp = pParseData->abTempBuf;
                  while ( !usRC && *pTemp )
                  {
                    usRC = RTFWriteBuffered( pParseData, *pTemp++, fIsTagChar );
                  } /* endwhile */
                }
                else if ( fInMacro && (bCurrent == '\"' ) )
                {
                  /***************************************************/
                  /* Do not convert to double quote but write as-is  */
                  /***************************************************/
                  usRC = RTFWriteBuffered( pParseData, bCurrent, fIsTagChar );
                }
                else if ( SpecialChars[bCurrent] && !IsAPLang(szLang))
                {
                  /***************************************************/
                  /* Replace special characters with their control   */
                  /* word, if not DBCS                               */
                  /***************************************************/
                  pParseData->fMacroMayFollow = FALSE;
                  pTemp = (PBYTE)SpecialChars[bCurrent];
                  while ( !usRC && *pTemp )
                  {
                    usRC = RTFWriteBuffered( pParseData, *pTemp++, fIsTagChar );
                  } /* endwhile */
                }
                else if ( bCurrent > 127 )
                {
                  /***************************************************/
                  /* Encode characters requiring 8 data bits         */
                  /***************************************************/
                  pParseData->fMacroMayFollow = FALSE;
                  sprintf( (PSZ)pParseData->abTempBuf, "\\\'%2.2x", bCurrent );
                  pTemp = (PBYTE)pParseData->abTempBuf;
                  while ( !usRC && *pTemp )
                  {
                    usRC = RTFWriteBuffered( pParseData, *pTemp++, fIsTagChar );
                  } /* endwhile */
                }
                else
                {
                  /***************************************************/
                  /* Write character as-is                           */
                  /***************************************************/
                  if ( (State != INTAG) &&
                       (bCurrent != SPACE) &&
                       (bCurrent != COMMA) &&
                       (bCurrent != SEMICOLON) &&
                       pParseData->fMacroMayFollow )
                  {
                    SHORT sI = 0;              // loop index

                    while ( (HelpMacro[sI].Id != END_OF_MACROS) &&
                            !( (HelpMacro[sI].szMacro[0] == bCurrent) &&
                               (_strnicmp( HelpMacro[sI].szMacro + 1,
                                       (PSZ)pParseData->pInBuf,
                                       HelpMacro[sI].sLength -1 ) == 0) ) )
                    {
                      sI++;
                    } /* endwhile */
                    if ( HelpMacro[sI].Id != END_OF_MACROS )
                    {
                      fInMacro = TRUE;
                    } /* endif */


                    // if no help macro, check for help tags
                    if ( !fInMacro )
                    {
                      PHELPTAG pTag = HelpTag;
                      while ( (pTag->Id != END_OF_TAGS) &&
                              !( (pTag->szTag[0] == bCurrent) &&
                                 (_strnicmp( pTag->szTag + 1,
                                         (PSZ)pParseData->pInBuf,
                                         pTag->sLength -1 ) == 0) ) )
                      {
                        pTag++;
                      } /* endwhile */

                      if ( pTag->Id != END_OF_TAGS )
                      {
                        fInMacro = TRUE;
                      } /* endif */
                    } /* endif */
                  } /* endif */

                  usRC = RTFWriteBuffered( pParseData, bCurrent, fIsTagChar );
                  if ( (State != INTAG) &&
                       (bCurrent != COMMA) &&
                       (bCurrent != SEMICOLON) &&
                       (bCurrent != SPACE) )
                  {
                    pParseData->fMacroMayFollow = FALSE;
                  } /* endif */
                } /* endif */
                break;
            } /* endswitch */
          } /* endif !fIgnoreCharacter */
        } /* endif fisDBCS */

        // save information about last char
        // added 1.17 bt
        // bt

        if (!pParseData->fWasExtTagChar)
        {
           pParseData->fWasExtTagChar = fIsTagChar;
        }

        if (!isalnum(bCurrent))
           pParseData->fWasExtTagChar = FALSE;

      } /* endif */
    } /* endwhile */
  } /* endwhile */

  // cleanup
  if ( pParseData )
  {
    if ( pParseData->hOutFile )
    {
      // Write any pending data
      if ( !usRC && pParseData->usOutBufUsed )
      {
        USHORT usBytesWritten;         // number of bytes written to file
        usRC = UtlWrite( pParseData->hOutFile,
                         pParseData->abOutBuf,
                         pParseData->usOutBufUsed,
                         &usBytesWritten,
                         TRUE );
      } /* endif */

      // close output file
      UtlClose( pParseData->hOutFile, TRUE );

      // delete output file in case of erros
      if ( usRC )
      {
        UtlDelete( szTempFile, 0L, FALSE );
      } /* endif */

    } /* endif */

    if ( pParseData->hInFile )
    {
      UtlClose( pParseData->hInFile, TRUE );
    } /* endif */

    //
    // free reference memory for ParseChangeFontSection(), ParseDefChgFontSection()
    //

    if ( pParseData->pChangeFontData )
    {
      UtlAlloc( (PVOID *)&pParseData->pChangeFontData, 0L, 0L, NOMSG );
    }

    if ( pParseData->pDefChgFontData )
    {
      UtlAlloc( (PVOID *)&pParseData->pDefChgFontData, 0L, 0L, NOMSG );
    }

    UtlAlloc( (PVOID *) &pParseData, 0L, 0L, NOMSG );
  } /* endif */
  if ( hfSource != NULLHANDLE ) UtlClose( hfSource, FALSE );

  // Delete old file and rename temp file
  if ( !usRC )
  {
    UtlDelete( pszInFile, 0L, FALSE );
    usRC = UtlMove( szTempFile, pszInFile, 0L, FALSE );
  } /* endif */

  return( usRC == 0 );
} /* end of function UnparseRTF */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     RTFWriteBuffered
//------------------------------------------------------------------------------
// Function call:     RTFWriteBuffered( PPARSEDATA pParseData, BYTE bAddByte );
//------------------------------------------------------------------------------
// Description:       Adds the given character to the output buffer. If the
//                    buffer is full its contents is written to the output
//                    file.
//------------------------------------------------------------------------------
// Input parameter:   PPARSEDATA pParseData   ptr to parser global data struct.
//                    BYTE       bAddByte    byte being written to output
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR         function completed OK
//                    other            OS/2 Dos call error codes
//------------------------------------------------------------------------------
// Side effects:      Linefeed and carriage return characters in the input
//                    data are ignored. CRLF is added in fixed intervals to
//                    the output stream
//------------------------------------------------------------------------------
// Samples:           usRC = RTFWriteBuffered( pParseData, 'B' );
//------------------------------------------------------------------------------
// Function flow:     if output buffer is full then
//                      write output buffer
//                      reset output pointer and buffer used count
//                    endif
//                    if current line pos exceeds 220 and
//                       current character is SPACE or START_CTRLWORD then
//                       add CRLF to output buffer
//                    endif
//                    add byte to output buffer
//------------------------------------------------------------------------------
USHORT RTFWriteBuffered
(
  PPARSEDATA pParseData,               // ptr to parser global data structure
  register BYTE bAddByte,                // byte being written to output
  BOOL       fIsTagChar               // char-belongs-to-a-tag flag
)
{
  USHORT usRC = 0;                    // internal return code
  USHORT usBytesWritten;              // number of bytes written to output file

  BYTE   bOrgLastOutput = pParseData->bLastOutput;

  fIsTagChar;

  /*******************************************************************/
  /* Remove CR/LF from input                                         */
  /* insert spaces to mark them instead.                             */
  /*                                                                 */
  /* Note: CR and LF are inserted in fixed intervals into the        */
  /*       output file.                                              */
  /*******************************************************************/
  {
    switch ( bAddByte )
    {
      case LF :
      case CR :
        if ( (pParseData->bLastOutput != SPACE)       &&           /* 1@KAT0014C */
             (pParseData->bLastOutput != BEGIN_GROUP) &&           /* 2@KAT0014A */
             (pParseData->bLastOutput != END_GROUP) )
        {
          pParseData->bLastOutput = LF;
        } /* endif */
        return( NO_ERROR );
        break;

      case SPACE:
        pParseData->bLastOutput = bAddByte;
        break;

      case START_CTRLWORD:
      case BEGIN_GROUP:
      case END_GROUP:
        pParseData->bLastOutput = bAddByte;
        break;

      default :
        if ( pParseData->bLastOutput == LF )
        {
          pParseData->fByteWasLF = TRUE;                          //KBT0840
          RTFWriteBuffered( pParseData, SPACE, FALSE );
        } /* endif */
        else
        {
          pParseData->fByteWasLF = FALSE;                         //KBT0840
        }
        pParseData->bLastOutput = bAddByte;
        break;
    } /* endswitch */
  }

  /*******************************************************************/
  /* Force a write if output buffer is nearly full (always leave     */
  /* enough room to add CR/LF to the buffer)                         */
  /*******************************************************************/
  if ( (pParseData->usOutBufUsed + 3) >= MAX_SEG_SIZE )
  {
    PSZ pszSearchString;

  // Fix KBT 1022
    // in order to prevent tremendous and unefficient implementing lookaheads
  // scan the output buffer for occurrences of
  // <CR><LF><SPACE><#|$|>|*|+|K|A|!|@>\footnote<...>
  // within this sequence, the <SPACE> char has to be deleted
  // otherwise, the new HCW help compiler tosses a warning and topic captions
  // in the HLP are containing garbage characters
  // added 22032001 bt

    {
      pszSearchString = (PSZ)pParseData->abOutBuf; // added 22032001 bt

      while (*pszSearchString != EOS)
      {
        if (*pszSearchString == CR &&
          *(pszSearchString + 1) == LF &&
          *(pszSearchString + 2) == SPACE)
        {
          if (IsCharTopicPrefix(*(pszSearchString + 3)))
          {
            if (!strncmp(&pszSearchString[4], "{\\footnote", 10))                   //
            {
              // shift the substring on to the left to make the SPACE vanish    //
              memmove( pszSearchString + 2, pszSearchString + 3, strlen(pszSearchString - 3));
              pParseData->usOutBufUsed--;
            }
          }
        }
        pszSearchString++;
      } /* endwhile */
    }

    usRC = UtlWrite( pParseData->hOutFile,
                       pParseData->abOutBuf,
                       pParseData->usOutBufUsed,
                       &usBytesWritten,
                       TRUE );

    /* reset outpuffer used count and output buffer pointer          */
    if ( !usRC )
    {
      pParseData->pOutBuf = pParseData->abOutBuf;
      pParseData->usOutBufUsed = 0;
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Insert carriage return and linefeed characters if forced        */
  /* by fForces or on request                                                     */
  /* Part 1: insert CRLF before the start of new tags if no space     */
  /*         character was found before (in part 2)                   */
  /*******************************************************************/
  if ( !usRC && pParseData->fForceNewLine && (pParseData->usLinePos != 0) )
  {
    *(pParseData->pOutBuf)++ = CR;
    pParseData->usOutBufUsed++;
    *(pParseData->pOutBuf)++ = LF;
    pParseData->usOutBufUsed++;
    pParseData->usLinePos = 0;
  } /* endif */
  pParseData->fForceNewLine = FALSE;


  /*******************************************************************/
  /* Insert carriage return and linefeed characters in fixed         */
  /* intervalls or on request                                                     */
  /* Part 1: insert CRLF before the start of new tags if no space     */
  /*         character was found before (in part 2)                   */
  /*******************************************************************/
  if ( !usRC  )
  {
    if ( pParseData->fNoLineSplit && (pParseData->usLinePos > 250) )
    {
      // enable insertion of CRLF as max line length will be exceeded
      pParseData->fNoLineSplit = FALSE;
    } /* endif */

    // insert CRLF if linepos > 220 and line split is allowed and
    // we are at a space or the start of a tag
    if ( (pParseData->usLinePos > 220) && !pParseData->fNoLineSplit )
    {
      BOOL fSplitHere = FALSE;

      // split at start of new control words but not within double backslashes
      if ( (bAddByte == START_CTRLWORD) && (bOrgLastOutput != START_CTRLWORD) )
      {
        fSplitHere = TRUE;
      }
      // or at blanks which do not belong to a tag ...
      // ... unfortunately blanks following tags have not set the fIsTagChar flag
      // so scan back to the start of the tag or to the previous space
      else if ( bAddByte == SPACE )
      {
        int iRemain = (int)pParseData->usOutBufUsed;
        PBYTE pTest = pParseData->pOutBuf;
        do
        {
          pTest--;
          iRemain--;
        } while ( (iRemain > 0) && isalnum(*pTest) );

        if ( *pTest != START_CTRLWORD )
        {
          fSplitHere = TRUE;
        } /* endif */
      } /* endif */

      if ( fSplitHere )
      {
         *(pParseData->pOutBuf)++ = CR;
         pParseData->usOutBufUsed++;
         *(pParseData->pOutBuf)++ = LF;
         pParseData->usOutBufUsed++;

         pParseData->usLinePos = 0;
      } /* endif */
    } /* endif */
  } /* endif */


  /* Add byte to output buffer                                       */
  if ( !usRC )
  {
    // don't add space character after CR/LF and in between
    // KBT0840
    if (!(pParseData->fByteWasLF && bAddByte == SPACE))
    {
       *(pParseData->pOutBuf)++ = bAddByte;
       pParseData->usOutBufUsed++;
       pParseData->usLinePos++;
    }
  } /* endif */

  /*******************************************************************/
  /* Insert carriage return and linefeed characters in fixed         */
  /* intervalls                                                      */
  /* Part 2: insert CRLF following the current character              */
  /*******************************************************************/
  if ( !usRC  )
  {
    if ( (pParseData->usLinePos > 200) && (bAddByte == SPACE) &&
         !pParseData->fNoLineSplit )
    {
      *(pParseData->pOutBuf)++ = CR;
      pParseData->usOutBufUsed++;
      *(pParseData->pOutBuf)++ = LF;
      pParseData->usOutBufUsed++;

      pParseData->usLinePos = 0;

    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function RTFWriteBuffered */

CHAR RTFRemoveLastCharFromWriteBuffer
(
  PPARSEDATA pParseData                // ptr to parser global data structure
)
{
  CHAR chRemoved = EOS;
  if ( pParseData->usOutBufUsed )
  {
     pParseData->pOutBuf--;
     pParseData->usOutBufUsed--;
     chRemoved = *(pParseData->pOutBuf);
     if ( pParseData->usLinePos ) pParseData->usLinePos--;
  } /* endif */
  return( chRemoved );
} /* end of function RTFRemoveLastCharFromWriteBuffer */


//------------------------------------------------------------------------------
//  RTFPreUnseg      - does the pre-unsegmentation for RTF documents
//------------------------------------------------------------------------------
//  Description:
//     Replaces the LFs in translatable segments with SPACE.
//------------------------------------------------------------------------------
//  Arguments:
//   PSZ       pszSource             - fully qualified name of source file
//   PSZ       pszTarget             - fully qualified name of target file
//   PBOOL     pfKill                - kill flag
//------------------------------------------------------------------------------
//  Returns:
//   USHORT usRC      0              - success
//                    other          - Dos error code
//------------------------------------------------------------------------------
//  Prereqs:
//   None.
//------------------------------------------------------------------------------
//  SideEffects:
//   None.
//------------------------------------------------------------------------------
static SHORT RTFPreUnseg
(
   PSZ pszSource,                      // fully qualified name of source file
   PSZ pszTarget,                      // fully qualified name of target file
   PSZ pszTagTable,                    // name of tag table
   PEQF_BOOL pfKill                    // pointer to kill flag
)
{
   SHORT       sRc = 0;                // function return code
   PTBSEGMENT  pSeg;                   // pointer to segments
   PTBDOCUMENT pTBDoc = NULL;          // pointer to segmented target document
   PTBDOCUMENT pTBSrcDoc = NULL;       // pointer to segmented source document
   PTBDOCUMENT pTBNewDoc = NULL;       // pointer to new segmented target document
   PPARSEDATA  pParseData = NULL;      // points to parser data structure
   ULONG       ulSegNum;               // segment number
   PSZ_W       pszTempSeg = NULL;      // buffer for segment data
   PCHAR       pConvTable = NULL;      // ptr to code conversion table
   CHAR szObjName[MAX_EQF_PATH];       // buffer for document object name
   CHAR szLang[MAX_LANG_LENGTH];       // buffer for document target language
   //BOOL        fFirstTranslatableSeg = TRUE;
   unsigned int uiCodePage;            // Codepage for lang setting
   BOOL        fChangeSeg;             // allow BIDI changes
   short       sUniChar;
   BOOL        fLastCharIsDBCS = FALSE;// TRUE = last character was a DBCS char
   BOOL        fCurCharIsDBCS = FALSE; // TRUE = current character is a DBCS char
   BOOL        fLastCharIsDBCSPunct = FALSE;// TRUE = last character was DBCS punctuation
   BOOL        fCurCharIsDBCSPunct = FALSE; // TRUE = current character is DBCS punctuation
   BOOL        fDBCSLang = FALSE;      // TRUE = tagret languag eis a DBCS language

   enum _DirectionMode
   {
     LTR_MODE,
     RTL_MODE,
     UNDEFINED_MODE
   } DirectionMode = UNDEFINED_MODE;   // current character direction

   //
   // since the help compiler HCW.EXE 4.03.0002 doesn't support
   // character styles combined with tagging extensions,
   // the tagging has to be written out in reverse order
   // sample:
   // \ul\cf11 \rtlch\f8\fs20 (works fine with the old HCW but not with 4.03.0002)
   // it has to be rewritten as:
   // \rtlch\f8\fs20\ul\cf11 -or-
   // \ul\cf11 \rtlch\f8\fs20\ul\cf11 (we decide to take this variant)
   //

   static CHAR_W szBidiRecordCharStyleStack[MAX_BIDI_REC_STACKDEPTH][BIDIRECORDCHARSTYLE_LEN+1];
   USHORT usBidiRecordStackLevel = 0; // we begin in the middle cause we don't track the start

   LOGFILE();                          // defines a log file
   LOGOPEN( "C:\\OTMRTF.LOG" );
   LOGWRITE2( "PreUnseg for document %s\n", pszSource );

   /*******************************************************************/
   /* allocate TBDOC structure(s)                                     */
   /*******************************************************************/
   if ( ! UtlAlloc( (PVOID *)&pTBDoc, 0L, (LONG) sizeof( TBDOCUMENT ), ERROR_STORAGE ) )
   {
     sRc = ERR_NOMEMORY;
   } /* endif */
   if ( !sRc && !*pfKill )
   {
     if ( ! UtlAlloc( (PVOID *)&pTBSrcDoc, 0L, (LONG) sizeof( TBDOCUMENT ), ERROR_STORAGE ) )
     {
       sRc = ERR_NOMEMORY;
     } /* endif */
   } /* endif */
   if ( !sRc && !*pfKill )
   {
     if ( !UtlAlloc( (PVOID *) &pParseData, 0L, (LONG)sizeof(PARSEDATA), ERROR_STORAGE ) )
     {
       sRc = ERR_NOMEMORY;
     } /* endif */
   } /* endif */

   if ( !sRc && !*pfKill )
   {
     if ( ! UtlAlloc( (PVOID *)&pszTempSeg, 0L, (LONG)10*MAX_SEGMENT_SIZE*sizeof(CHAR_W), ERROR_STORAGE ) )
     {
       sRc = ERR_NOMEMORY;
     } /* endif */
   } /* endif */

   if ( !sRc && !*pfKill )
   {
     if ( ! UtlAlloc( (PVOID *) &pTBNewDoc, 0L, (LONG) sizeof( TBDOCUMENT ), ERROR_STORAGE ) )
     {
       sRc = ERR_NOMEMORY;
     } /* endif */
   } /* endif */

   if ( !sRc && !*pfKill )
   {
     if ( ! UtlAlloc( (PVOID *) &pTBNewDoc->pInBuf, 0L, (LONG) IO_BUFFER_SIZE, ERROR_STORAGE ) )
     {
       sRc = ERR_NOMEMORY;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* reset bidi character markers                                    */
   /*******************************************************************/

   pParseData->pszBidiRecordCharStyle = szBidiRecordCharStyleStack[usBidiRecordStackLevel];
   memset(pParseData->pszBidiRecordCharStyle, 0, BIDIRECORDCHARSTYLE_LEN);
   pParseData->fSkipOldBidiTag = FALSE;

   /*******************************************************************/
   /* load tag table                                                  */
   /*******************************************************************/
   if ( !sRc && !*pfKill )
   {
     sRc = (SHORT)TALoadTagTable( DEFAULT_QFTAG_TABLE,
                                  (PLOADEDTABLE *) &pTBDoc->pQFTagTable,
                                  TRUE, FALSE );
     if ( !sRc )
     {
       pTBSrcDoc->pQFTagTable = pTBDoc->pQFTagTable;
     } /* endif */
     if ( !sRc )
     {
       sRc = (SHORT)TALoadTagTable( DEFAULT_QFTAG_TABLE,
                                    (PLOADEDTABLE *) &pTBNewDoc->pQFTagTable,
                                    TRUE, FALSE );
     } /* endif */
     if ( !sRc )
     {
      sRc = (SHORT)TALoadTagTable( pszTagTable,
                                   (PLOADEDTABLE *) &pTBNewDoc->pDocTagTable,
                                   FALSE, FALSE );
     } /* endif */
   } /* endif */

   /*****************************************************************/
   /* add a dummy segment for the beginning                         */
   /*****************************************************************/
   if ( !sRc && !*pfKill )
   {
     TBSEGMENT ActSegment;
     memset( &ActSegment, 0, sizeof(TBSEGMENT) );
     EQFBAddSeg( pTBNewDoc, &ActSegment );
   } /* endif */

   /*******************************************************************/
   /* Get document target language and check if a specific handling   */
   /* is required                                                     */
   /*******************************************************************/
   if ( !sRc && !*pfKill )
   {
     CHAR szFolder[MAX_FILESPEC];      // buffer for folder name
     CHAR szDoc[MAX_FILESPEC];         // buffer for document name
     PSZ  pszTemp;                     // ptr to for processing of path name

     // setup document object name
     strcpy( szObjName, pszSource );
     pszTemp = UtlSplitFnameFromPath( szObjName );   // remove document name
     strcpy( szDoc, pszTemp );
     pszTemp = UtlSplitFnameFromPath( szObjName );   // remove STARGET dir
     pszTemp = UtlSplitFnameFromPath( szObjName );   // remove folder name
     strcpy( szFolder, pszTemp );
     UtlMakeEQFPath( szObjName, *pszSource, SYSTEM_PATH, szFolder );
     strcat( szObjName, BACKSLASH_STR );
     strcat( szObjName, szDoc );

     // get document target language
     szLang[0] = EOS;
     DocQueryInfo( szObjName,          // object name of document
                         NULL,         // translation memory or NULL
                         NULL,         // folder format or NULL
                         NULL,         // source language or NULL
                         szLang,       // target language or NULL
                         FALSE );      // message-handling flag
     LOGWRITE2( "Document target language is \"%s\"\n", szLang );


     // set document short name in our document structures
     strcpy(  pTBSrcDoc->szDocName, szDoc );
     strcpy(  pTBNewDoc->szDocName, szDoc );

     // check if target language is a DBCS language
     fDBCSLang = (MorphGetLanguageType( szLang ) == MORPH_DBCS_LANGTYPE);

     //
     // Set target language locale to enable _isdbcs recognition
     //

     if (!RTFSetLangCodePage(szLang, &uiCodePage))
     {
        CHAR szReplace[10];
        PSZ pszReplace;

        sprintf(szReplace, "%d", uiCodePage);

        pszReplace = szReplace;

        UtlError( WARNING_NO_LOCALE, MB_OK, 1, &pszReplace, EQF_WARNING );
     }

     // get language dependent settings
     RTFGetSettings( pszTagTable, szLang, pParseData );

     pParseData->fIsFontTag = FALSE;

     // free memory of ChangeFontData / DefChgFontData because nothing
     // is done with it right now
     if ( pParseData->pChangeFontData )
     {
        UtlAlloc( (PVOID *)&pParseData->pChangeFontData, 0L, 0L, NOMSG );
     }

     if ( pParseData->pDefChgFontData )
     {
        UtlAlloc( (PVOID *)&pParseData->pDefChgFontData, 0L, 0L, NOMSG );
     }

     // set fDBCSSpaceForLF flag depending on target language
//     {
//       int i = 0;
//
//       fDBCSSpaceForLF = FALSE;          // set default
//       while ( SpecialLang[i].szLang[0] != EOS  )
//       {
//         if ( _stricmp( SpecialLang[i].szLang, szLang ) == 0 )
//         {
//           fDBCSSpaceForLF = (SpecialLang[i].ulFlags & DBCS_BLANK_FOR_LINEFEED) != 0L;
//           fBidi = (SpecialLang[i].ulFlags & INSERT_BIDI_TAGS) != 0L;
//         } /* endif */
//         i++;
//       } /* endwhile */
//       LOGWRITE2( "fDBCSSpaceForLF is %d\n", ((SHORT)fDBCSSpaceForLF) );
//
//     }

   } /* endif */

   /*******************************************************************/
   /* load the segmented target file                                  */
   /*******************************************************************/
   if ( !sRc && !*pfKill )
   {
     RTFParseGetCP(pszSource, &pParseData->ulSrcOemCP, &pParseData->ulTgtOemCP,
                           &pParseData->ulSrcAnsiCP, &pParseData->ulTgtAnsiCP);
     //EQFBFileRead assumes ulOem/AnsiCodePage to be set in TBDoc
     pTBDoc->ulOemCodePage = pParseData->ulTgtOemCP;
     pTBDoc->ulAnsiCodePage = pParseData->ulTgtAnsiCP;
     sRc = EQFBFileRead( pszSource, pTBDoc );
   } /* endif */

   /*******************************************************************/
   /* load the segmented source file                                  */
   /*******************************************************************/
   if ( !sRc && !*pfKill )
   {
     CHAR szSourceDoc[MAX_EQF_PATH];   // buffer for path name of source file
     CHAR szFolder[MAX_FILESPEC];      // buffer for folder name
     CHAR szDoc[MAX_FILESPEC];         // buffer for document name
     PSZ  pszTemp;                     // ptr to for processing of path name

     // setup name of segmented source file
     strcpy( szSourceDoc, pszSource );
     pszTemp = UtlSplitFnameFromPath( szSourceDoc );   // remove document name
     strcpy( szDoc, pszTemp );
     pszTemp = UtlSplitFnameFromPath( szSourceDoc );   // remove STARGET dir
     pszTemp = UtlSplitFnameFromPath( szSourceDoc );   // remove folder name
     strcpy( szFolder, pszTemp );
     UtlMakeEQFPath( szSourceDoc, *pszSource, DIRSEGSOURCEDOC_PATH, szFolder );
     strcat( szSourceDoc, BACKSLASH_STR );
     strcat( szSourceDoc, szDoc );

     //EQFBFileRead assumes CP set in TBSrcDoc
     pTBSrcDoc->ulOemCodePage = pParseData->ulSrcOemCP;
     pTBSrcDoc->ulAnsiCodePage = pParseData->ulSrcAnsiCP;
     sRc = EQFBFileRead( szSourceDoc, pTBSrcDoc );
   } /* endif */


    // for loop over all segs and check curly braces
    if ( !sRc && !*pfKill )
    {
      PTBSEGMENT pSourceSeg, pTargetSeg;

      ulSegNum = 1;
      pTargetSeg = EQFBGetSegW( pTBDoc, ulSegNum );
      while ( pTargetSeg )
      {
        EQF_BOOL fSegOK;
        EQF_BOOL fChanged;

        pSourceSeg = EQFBGetSegW( pTBSrcDoc, ulSegNum );

        if ( pTargetSeg->SegFlags.Joined && !pTargetSeg->SegFlags.JoinStart )
        {
          // ignore joined segments
        }
        else
        {
          fSegOK = EQFCHECKSEGW( NULL, pSourceSeg->pDataW, pTargetSeg->pDataW, &fChanged, FALSE );

          if ( !fSegOK )
          {
            USHORT usMBCode;
            CHAR szSegNumber[10];
            PSZ  pszParm = szSegNumber;
            sprintf( szSegNumber, "%lu", pSourceSeg->ulSegNum );
            usMBCode = UtlError( ERR_RTF_BRACEMISMATCHERR, MB_YESNO | MB_DEFBUTTON2,
                                  1, &pszParm, EQF_QUERY );
            if ( usMBCode != MBID_YES )
            {
              sRc = ERR_RTF_BRACEMISMATCHERR;
            } /* endif */
          } /* endif */
        } /* endif */

        // next segment
        ulSegNum++;
        pTargetSeg = EQFBGetSegW( pTBDoc, ulSegNum );
      } /* endwhile */
    } /* endif */

    // get type of code conversion (only required for checking
    // in bidi mode)
    if ( IsAPLang(szLang) )
    {
      pConvTable = NULL;           // no code conversion in DBCS environment
    }
    else
    {
#if defined(R004203_CONVERSIONCP)
      if ( pParseData->usCodePage )
      {
        UtlQueryCharTableEx( ASCII_TO_ANSI_TABLE, (PUCHAR *)&pConvTable,
                             pParseData->usCodePage );
      }
      else
      {
         UtlQueryCharTableForDocConv( pszSource, &pConvTable, pParseData->ulTgtOemCP );
      } /* endif */
#elif defined(_TP51)
      UtlQueryCharTableEx( ASCII_TO_ANSI_TABLE, &pConvTable,
                           pParseData->usCodePage );
#endif
    } /* endif */

   /*******************************************************************/
   /* Create output file (necessary as EQFBFileWrite tries to delete  */
   /* this file wih fMsg set to TRUE)                                 */
   /*******************************************************************/
   if ( !sRc && !*pfKill )
   {
     HFILE       hFile;                // handle of output file
     USHORT      usOpenAction;         // action performed by DosOpen
     USHORT      usDosRC;              // return code of Dos calls

     usDosRC = UtlOpen( pszTarget,
                        &hFile,
                        &usOpenAction, 0L,
                        FILE_NORMAL,
                        FILE_OPEN | FILE_CREATE,
                        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                        0L,
                        TRUE );
     if ( usDosRC )
     {
        sRc = ERR_OPENFILE;
     }
     else
     {
       UtlClose( hFile, FALSE );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* scan the file and change segments                               */
   /*******************************************************************/
   if ( !sRc && !*pfKill )
   {
     ULONG  ulOutSegNum = 1;

     ulSegNum = 1;
     pSeg = EQFBGetSegW(pTBDoc, ulSegNum);

     if ( pParseData->fBidi )
     {
       DirectionMode = UNDEFINED_MODE;
     } /* endif */

     while ( pSeg && !sRc && !*pfKill )
     {
       PSZ_W pszSegData = NULL;        // ptr to active segment data

       /***************************************************************/
       /* Process data of translatable segments                       */
       /***************************************************************/
     if (pSeg->pDataW != NULL)
     {
         PSZ_W     pszSource = pSeg->pDataW;
         PSZ_W     pszTarget = pszTempSeg;
         CHAR_W    chLastChar = 0;
         BOOL      fInTag = FALSE;

         // only if we've got a translated segment
         // add rtl/ltr tags

         if ( pSeg->qStatus == QF_XLATED /* && !fFirstTranslatableSeg */)
           fChangeSeg = TRUE;
         else
           fChangeSeg = FALSE;

         //if ( pSeg->qStatus != QF_NOP )
         //{
         //  fFirstTranslatableSeg = FALSE;
         //} /* endif */

         // always assume undefined direction at the start of the segment
         if ( pParseData->fBidi )
         {
           DirectionMode = UNDEFINED_MODE;
         } /* endif */

         while ( *pszSource != 0 )
         {
           // update last character DBCS flags only if current character
           // does not start a tag or is a line break
           if ( *pszSource != START_CTRLWORD_W )
           {
             // use current character flag as last character flag
             fLastCharIsDBCS = fCurCharIsDBCS;
             fLastCharIsDBCSPunct = fCurCharIsDBCSPunct;

             // reset current character flag when current character is no line break and
             // no neutral character
             if ( (*pszSource != CR_W) && (*pszSource != LF_W) &&
                  (*pszSource != BEGIN_GROUP_W) && (*pszSource != END_GROUP_W) )
             {
               fCurCharIsDBCS = FALSE;
               fCurCharIsDBCSPunct = FALSE;
             } /* endif */
           } /* endif */


       //
       // turn off recording of preceding character styles by default
       // to prevent RTFPreUnsegNextNonNeutral from recording (it's
       // only a lookahead function
       //

       pParseData->fBidiRecordCharStyle = FALSE;

       if ( *pszSource == START_CTRLWORD_W)
       {
          PSZ_W pszNext;
          RTFTAGS TagId;               // ID of skipped tag

          // for hex encoded trademark character and DBCS environment only:
          // add unicode codepoint for trademark character
          if ( fDBCSLang && (pszSource[1] == '\'') && (pszSource[2] == 'a') && (pszSource[3] == 'e') )
          {
            pszSource += 3;             // only skip 3 characters, last character is skipped at end of loop...
            UTF16strcpy( pszTarget, L"\\uc1\\u174\\\'ae" );
            pParseData->iUCStatus = 1;
            pszTarget += UTF16strlenCHAR( pszTarget );
            fChangeSeg = TRUE;
          }
          else
          {


            // skip RTF tags, allow recording preceding bidi char styles
            pParseData->fBidiRecordCharStyle = TRUE;
            pszNext = RTFPreUnsegSkipTag( pszSource, pParseData, &TagId );

            if ( pszNext != pszSource && !pParseData->fSkipOldBidiTag)
            {
              while ( pszSource < pszNext )
              {
                *pszTarget++ = *pszSource++;
              }
              pszSource = pszNext - 1;  // go one character back, as pszSource is incremented at end of loop
              chLastChar = *pszSource;  // remember last character of tag
              fInTag = (chLastChar != SPACE_W );                // we are at the end of a tag

              // handle any metatags and set bidi-off mode
              if ( TagId == BIDIOFF_METATAG )
              {
                pParseData->fBidiOffMode = TRUE;
                if ( pParseData->chBIDIOFFTagsW[0] )
                {
                  UTF16strcpy( pszTarget, pParseData->chBIDIOFFTagsW );
                  pszTarget += UTF16strlenCHAR(pszTarget);
                  *pszTarget++ = SPACE_W;
                  fChangeSeg = TRUE;
                } /* endif */
              }
              else if ( TagId == BIDION_METATAG )
              {
                pParseData->fBidiOffMode = FALSE;
                if ( pParseData->chBIDIONTagsW[0] )
                {
                  UTF16strcpy( pszTarget, pParseData->chBIDIONTagsW );
                  pszTarget += UTF16strlenCHAR(pszTarget);
                  *pszTarget++ = SPACE_W;
                  fChangeSeg = TRUE;
                } /* endif */
              } /* endif */
            }
            else
            {
              // we have to skip any \ltrch or \rtlch sequences
              // normally they shouldn't be found here

              do
              {
                *pszSource++;
              }
              while ( *pszSource == START_CTRLWORD || iswalnum(*pszSource) );

              if (*pszSource == SPACE_W ) pszSource++;

              fChangeSeg = TRUE;             // GQ: we have changed this segment ...

              continue;
            }
          } /* endif */
         }
         else if ( *pszSource == CR_W )
         {
           // Always ignore any carriage return in the data
         }
         else if ( *pszSource == LF_W )
         {
           BOOL fWasInTag = fInTag;

           LOGWRITE1( "<LF>\n" );

           fInTag = FALSE;
           /*********************************************************/
           /* Handle LF (linefeed)                                  */
           /*********************************************************/
           if ( chLastChar == BACKSLASH_W)
           {
             /*******************************************************/
             /* Leave LF as the LF may split special characters     */
             /* (e.g. the curly brace character which is coded      */
             /* as backslash curly brace)                           */
             /*******************************************************/
             *pszTarget++ = *pszSource;
             chLastChar = LF_W;
           }
           else if ( chLastChar == SPACE_W )
           {
             // Ignore LF as it is prefixed with a SPACE character
           }
           else if ( fWasInTag )
           {
             // convert LF to space as it is required as tag delimiter
             *pszTarget++ = SPACE_W;
             chLastChar = SPACE_W;
           }
           else if ( (pszSource[1] == SPACE_W) || (pszSource[1] == LF_W) )
           {
             // ignore LF as next line starts already has a blank or a LF
           }
           else if ( pszSource[1] == 0 )
           {
               /*******************************************************/
               /* We detected a line feed at the end of the current   */
               /* segment. This is normally inserted by the           */
               /* segmentation exit and can be removed without        */
               /* problems. If the segment ended however with         */
               /* <period><blank><lf>, the blank may have been        */
               /* removed by TPRO and the <lf> has to be treated as   */
               /* a blank                                             */
               /* We have to look at the source segment to determine  */
               /* the correct action to be performed                  */
               /*******************************************************/
               PTBSEGMENT  pSourceSeg; // pointer to source segment
               USHORT      usSegLen;   // length of segment

               pSourceSeg = EQFBGetSegW( pTBSrcDoc, ulSegNum );

               if ( pSourceSeg )
               {
                 usSegLen = (USHORT)UTF16strlenCHAR( pSourceSeg->pDataW );
                 if ( (usSegLen >= 2) &&
                      (pSourceSeg->pDataW[usSegLen-1] == LF_W) &&
                      (pSourceSeg->pDataW[usSegLen-2] == SPACE_W) )
                 {
                   // there was a blank right before the LF so
                   // replace the LF by blank if nt enclosed in DBCS characters
                   CHAR_W chNextChar;

                   // get next character
                   chNextChar = *(pszSource + 1);
                   if ( !chNextChar )
                   {
                     // take first character of next segment
                     PTBSEGMENT  pNextSeg;
                     pNextSeg = EQFBGetSegW( pTBDoc, ulSegNum+1 );
                     if ( pNextSeg && pNextSeg->pDataW )
                     {
                       // point to data of new segment
                       chNextChar = *(pNextSeg->pDataW);
                     } /* endif */
                   } /* endif */

                   // check for surrounding DBCS characters or DBCS punctuation
                   if ( pParseData->fDBCSBlankForLF )
                   {
                     // always replace linebreak by space without DBCS character checks
                     chLastChar = *pszTarget++ = SPACE_W;
                   }
                   else if ( (fLastCharIsDBCS && RTFIsDBCSChar( chNextChar, pTBDoc->ulOemCodePage )) ||
                             fLastCharIsDBCSPunct )
                   {
                     // ignore LF
                   }
                   else
                   {
                     chLastChar = *pszTarget++ = SPACE_W;
                   } /* endif */
                 } /* endif */
               }
               else
               {
                 // ignore linefeed, source segment could not be read
               } /* endif */
             }
             else
             {
               // Replace LF with a SPACE character if not enclosed in DBCS
               // characters
               CHAR_W chNextChar;

               // get next character
               chNextChar = *(pszSource + 1);
               if ( !chNextChar )
               {
                 // take first character of next segment
                 PTBSEGMENT  pNextSeg;
                 pNextSeg = EQFBGetSegW( pTBDoc, ulSegNum+1 );
                 if ( pNextSeg && pNextSeg->pDataW )
                 {
                   // point to data of new segment
                   chNextChar = *(pNextSeg->pDataW);
                 } /* endif */
               } /* endif */

               // check for surrounding DBCS characters or preceeding DBCS punctuation
               if ( pParseData->fDBCSBlankForLF )
               {
                 // always replace linebreak by space without DBCS character checks
                 chLastChar = *pszTarget++ = SPACE_W;
               }
               else if ( (fLastCharIsDBCS && RTFIsDBCSChar( chNextChar, pTBDoc->ulOemCodePage )) ||
                          fLastCharIsDBCSPunct )
               {
                 // ignore LF
               }
               else
               {
                 chLastChar = *pszTarget++ = SPACE_W;
               } /* endif */
             } /* endif */
           }
           else
           {
             BOOL fIsBidiChar = FALSE;

             // set fInTag flag
             if ( fInTag )
             {
               // we stay inside tag mode as long there are alphabetic or numbers
               if ( RTFisAlpha(*pszSource) || iswdigit(*pszSource) )
               {
                 // OK, we are still within the tag name/data
               }
               else
               {
                 fInTag = FALSE;
               } /* endif */
             }
             else if ( RTFisAlpha(*pszSource) )
             {
               if ( chLastChar == BACKSLASH_W )
               {
                 fInTag = TRUE;
               } /* endif */
             }
             else
             {
               fInTag = FALSE;
             } /* endif */

             LOGWRITE3( "<%c(0x%2.2X)>", *pszSource, ((UCHAR)*pszSource) );
             if ( pParseData->fBidi )
             {
               if ( !pParseData->fBidiOffMode )
               {
                fIsBidiChar =RTFIsBidiCharW( pParseData, *pszSource, pConvTable );
                if ( fIsBidiChar )
                {
                  // handle left-to-right characters
                  LOGWRITE2( "BIDI!RTL=%d", DirectionMode == RTL_MODE );

                  // if there was a non-bidi font before, overwrite!
                  if ( DirectionMode != RTL_MODE || pParseData->fIsFontTag)
                  {
                    if ( pParseData->chRTLTagsW[0] && fChangeSeg && !pParseData->fHiddenMode )
                    {
                        LOGWRITE1( " Inserting RTL tags\n" );
                        UTF16strcpy( pszTarget, pParseData->chRTLTagsW );
                        pszTarget += UTF16strlenCHAR(pszTarget);
                        UTF16strcpy( pszTarget, pParseData->pszBidiRecordCharStyle );
                        pszTarget += UTF16strlenCHAR(pszTarget);
                        *pszTarget++ = SPACE_W;
                    }
                    DirectionMode = RTL_MODE;

                    // reset marker
                    pParseData->fIsFontTag = FALSE;
                  } /* endif */
                }
                else if ( *pszSource == BEGIN_GROUP_W )
                {
                  if (*(pszSource - 1) != START_CTRLWORD_W)
                  {
                    pParseData->fHiddenMode = FALSE;
                    usBidiRecordStackLevel += 1;

                    if (usBidiRecordStackLevel == 0)
                    {
                      memset(szBidiRecordCharStyleStack[usBidiRecordStackLevel], 0, BIDIRECORDCHARSTYLE_LEN);
                    }
                    else
                    {
                      // a beginning group ... { ... } inherits
                      // all attributes from outside

                      memcpy(szBidiRecordCharStyleStack[usBidiRecordStackLevel],
                        szBidiRecordCharStyleStack[usBidiRecordStackLevel - 1], BIDIRECORDCHARSTYLE_LEN);
                    }
                    pParseData->pszBidiRecordCharStyle =
                      szBidiRecordCharStyleStack[usBidiRecordStackLevel];
                  }

                  // treat as neutral characters ...
                }
                else if ( *pszSource == END_GROUP_W )
                {
                  if (*(pszSource - 1) != START_CTRLWORD_W)
                  {
                      pParseData->fHiddenMode = FALSE;
                      memset(szBidiRecordCharStyleStack[usBidiRecordStackLevel], 0, BIDIRECORDCHARSTYLE_LEN);
                      if ( usBidiRecordStackLevel != 0 ) usBidiRecordStackLevel -= 1;
                      pParseData->pszBidiRecordCharStyle =
                        szBidiRecordCharStyleStack[usBidiRecordStackLevel];
                    }

                    // treat as neutral characters, but reset
                    // current direction mode as we do not know
                    // which direction was active outside the group
                    DirectionMode = UNDEFINED_MODE;
                }
                else if ( *pszSource == SPACE_W )
                {
                  CHAR_W chNextNN = RTFPreUnsegNextNonNeutral( pParseData, pszSource,
                                                                pTBDoc,
                                                                &ulSegNum, pConvTable );
                  if ( RTFIsBidiCharW( pParseData, chNextNN, pConvTable ) &&
                      DirectionMode != RTL_MODE || pParseData->fIsFontTag)
                  {
                    if ( pParseData->chRTLTags[0] && fChangeSeg && !pParseData->fHiddenMode )
                    {
                      LOGWRITE1( " Inserting RTL tags\n" );
                      UTF16strcpy( pszTarget, pParseData->chRTLTagsW );
                      pszTarget += UTF16strlenCHAR( pszTarget );
                      UTF16strcpy( pszTarget, pParseData->pszBidiRecordCharStyle );
                      pszTarget += UTF16strlenCHAR( pszTarget );
                      *pszTarget++ = SPACE_W;
                    }
                    DirectionMode = RTL_MODE;
                  } /* endif */

                  pParseData->fIsFontTag = FALSE; // no more needed
                }
                // added bt 15012001 ...
                else if ( IsBidiOpenBrace(*pszSource) )
                {
                  // we need a lookahead to see what's going on

                  CHAR_W chNextNN  =
                  RTFPreUnsegNextNonNeutral( pParseData, pszSource, pTBDoc,
                                              &ulSegNum, pConvTable );
                  if ( RTFIsBidiCharW( pParseData, chNextNN, pConvTable ) &&
                              (DirectionMode != LTR_MODE) )
                  {
                    if ( pParseData->chLTRTagsW[0] && fChangeSeg && !pParseData->fHiddenMode )
                    {
                        LOGWRITE1( " Inserting LTR tags\n" );
                        UTF16strcpy( pszTarget, pParseData->chLTRTagsW );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        UTF16strcpy( pszTarget, pParseData->pszBidiRecordCharStyle );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        *pszTarget++ = SPACE;
                    }
                    DirectionMode = LTR_MODE;
                  } /* endif */
                }
                // ... bt
                else if ( RTFisAlpha(*pszSource) )
                {
                        // handle right-to-left characters
                  if ( (DirectionMode != LTR_MODE) && !fInTag )
                  {
                    if (UTF16strlenCHAR(pParseData->chLTRTagsW) && fChangeSeg && !pParseData->fHiddenMode )
                    {
                        LOGWRITE1( " Inserting LTR tags\n" );
                        UTF16strcpy( pszTarget, pParseData->chLTRTagsW );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        UTF16strcpy( pszTarget, pParseData->pszBidiRecordCharStyle );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        *pszTarget++ = SPACE_W;
                    }
                    DirectionMode = LTR_MODE;
                  } /* endif */
                }
                else if ( iswdigit(*pszSource) )
                {
                  // handle numeric characters
                  if ( DirectionMode == LTR_MODE )
                  {
                    // nothing to do, we will stay in LTR mode
                  }
                  else if ( DirectionMode == RTL_MODE )
                  {
                    // nothing to do, we will stay in RTL mode
                  }
                  else
                  {
                    // we are in undefined mode so look what is following
                    CHAR_W chNextNN =
                      RTFPreUnsegNextNonNeutral( pParseData, pszSource, pTBDoc,
                                                  &ulSegNum, pConvTable );
                    if ( RTFIsBidiCharW( pParseData, chNextNN, pConvTable ) )
                    {
                      if ( pParseData->chRTLTagsW[0] && fChangeSeg && !pParseData->fHiddenMode )
                      {
                        LOGWRITE1( " Inserting RTL tags\n" );
                        UTF16strcpy( pszTarget, pParseData->chRTLTagsW );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        UTF16strcpy( pszTarget, pParseData->pszBidiRecordCharStyle );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        *pszTarget++ = SPACE_W;
                      }
                      DirectionMode = RTL_MODE;
                    }
                    else
                    {
                      if ( pParseData->chLTRTagsW[0] && fChangeSeg && !pParseData->fHiddenMode )
                      {
                        LOGWRITE1( " Inserting LTR tags\n" );
                        UTF16strcpy( pszTarget, pParseData->chLTRTagsW );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        UTF16strcpy( pszTarget, pParseData->pszBidiRecordCharStyle );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        *pszTarget++ = SPACE_W;
                      }
                      DirectionMode = LTR_MODE;
                    } /* endif */
                  } /* endif */
                }
                // added bt 15012001 ...
                else if ( !IsBidiPunct(*pszSource) )
                {
                  // skip all other stuff
                }
                // ... bt
                else
                {
                  // handle neutral characters
                  if ( DirectionMode == LTR_MODE )
                  {
                    // lets have a look at the the next non-neutral char
                    CHAR_W chNextNN =
                    RTFPreUnsegNextNonNeutral( pParseData, pszSource, pTBDoc,
                                                &ulSegNum, pConvTable );

                    // GQ: modified to handle punction in dependency of surrounding
                    //     text. Stay in LTR mode if punctuation is followed
                    //     by non-BIDI text
  //                   if ( RTFIsBidiCharW( pParseData, chNextNN, pConvTable ) )
                    // GQ: disabled change again as no feedback about sideeffects
                    // so far
                    if ( RTFIsBidiCharW( pParseData, chNextNN, pConvTable ) ||
                          IsBidiPunct(*pszSource) )
                    {
                      // neutrals are followed by BIDI stuff, so switch to RTL mode
                      if ( pParseData->chRTLTagsW[0] && fChangeSeg && !pParseData->fHiddenMode )
                      {
                        LOGWRITE1( " Inserting RTL tags\n" );
                        UTF16strcpy( pszTarget, pParseData->chRTLTagsW );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        UTF16strcpy( pszTarget, pParseData->pszBidiRecordCharStyle );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        *pszTarget++ = SPACE_W;
                      }
                      DirectionMode = RTL_MODE;
                    } /* endif */
                  }
                  else if ( DirectionMode == RTL_MODE )
                  {
                    // nothing to do, we will stay in RTL mode
                  }
                  else
                  {
                    // we are in undefined mode so look what is following
                    CHAR_W chNextNN =
                      RTFPreUnsegNextNonNeutral( pParseData, pszSource, pTBDoc,
                                                  &ulSegNum, pConvTable );
                    if ( RTFIsBidiCharW( pParseData, chNextNN, pConvTable ) )
                    {
                      if ( pParseData->chRTLTagsW[0] && fChangeSeg && !pParseData->fHiddenMode )
                      {
                        LOGWRITE1( " Inserting RTL tags\n" );
                        UTF16strcpy( pszTarget, pParseData->chRTLTagsW );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        UTF16strcpy( pszTarget, pParseData->pszBidiRecordCharStyle );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        *pszTarget++ = SPACE_W;
                      }
                      DirectionMode = RTL_MODE;
                    }
                    else if ( RTFisAlpha(chNextNN) )
                    {
                      if ( pParseData->chLTRTagsW[0] && fChangeSeg && !pParseData->fHiddenMode )
                      {
                        LOGWRITE1( " Inserting LTR tags\n" );
                        UTF16strcpy( pszTarget, pParseData->chLTRTagsW );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        UTF16strcpy( pszTarget, pParseData->pszBidiRecordCharStyle );
                        pszTarget += UTF16strlenCHAR( pszTarget );
                        *pszTarget++ = SPACE;
                      }
                      DirectionMode = LTR_MODE;
                    } /* endif */
                  } /* endif */
                } /* endif */
               }
               else
               {
                 // do no bidi handling, but handle curly braces
                 if ( *pszSource == BEGIN_GROUP_W )
                 {
                   if (*(pszSource - 1) != START_CTRLWORD_W)
                   {
                    pParseData->fHiddenMode = FALSE;
                    usBidiRecordStackLevel += 1;

                    if (usBidiRecordStackLevel == 0)
                    {
                      memset(szBidiRecordCharStyleStack[usBidiRecordStackLevel], 0, BIDIRECORDCHARSTYLE_LEN);
                    }
                    else
                    {
                      // a beginning group ... { ... } inherits
                      // all attributes from outside
                      memcpy(szBidiRecordCharStyleStack[usBidiRecordStackLevel],
                        szBidiRecordCharStyleStack[usBidiRecordStackLevel - 1], BIDIRECORDCHARSTYLE_LEN);
                    }
                    pParseData->pszBidiRecordCharStyle =
                      szBidiRecordCharStyleStack[usBidiRecordStackLevel];
                   }
                 }
                 else if ( *pszSource == END_GROUP_W )
                 {
                   if (*(pszSource - 1) != START_CTRLWORD_W)
                   {
                      pParseData->fHiddenMode = FALSE;
                      memset(szBidiRecordCharStyleStack[usBidiRecordStackLevel], 0, BIDIRECORDCHARSTYLE_LEN);
                      /* if ( usBidiRecordStackLevel != 0 )*/ usBidiRecordStackLevel -= 1;
                      pParseData->pszBidiRecordCharStyle =
                        szBidiRecordCharStyleStack[usBidiRecordStackLevel];
                   }
                   DirectionMode = UNDEFINED_MODE;
                 }
               } /* endif */
             } /* endif fBidi mode */

             // handle current character
             {
               // check if this a Unicode character with its own RTF tag
               int i = 0;
               BOOL fFound = FALSE;

               while ( SpecialCharTags[i].szTag[0] && !fFound )
               {
                 if ( SpecialCharTags[i].chValue == *pszSource )
                 {
                   fFound = TRUE;
                 }
                 else
                 {
                   i++;
                 } /* endif */
               } /* endwhile */

               if ( fFound )
               {
                 // insert tag instead of character
                 *pszTarget++ = L'\\';
                 UTF16strcpy( pszTarget, SpecialCharTags[i].szTag );
                 pszTarget += UTF16strlenCHAR( pszTarget );
                 *pszTarget++ = SPACE_W;
                 pParseData->iUCStatus = 0;          //Aoki
                 fChangeSeg = TRUE;
               }
               // BIDI chars: add as-is without unicode handling
               else if ( fIsBidiChar )
               {
                 chLastChar = *pszTarget++ = *pszSource;
                 pParseData->iUCStatus = 0;          //Aoki
               }
               // check if character is in ASCII range, if not we have to add it
               // as Unicode value
               else if ( iswascii( *pszSource ) )
               {
                 // no unicode coding required
                 chLastChar = *pszTarget++ = *pszSource;
                 pParseData->iUCStatus = 0;          //Aoki
               }
               else
               {
                 LONG lRc = 0;
                 UCHAR ucReplaceChars[10];                                          //Aoki
                 CHAR_W chUnicodeString[2];

                 chUnicodeString[0] = *pszSource;
                 chUnicodeString[1] = 0;

                 memset( ucReplaceChars, 0, sizeof(ucReplaceChars) );              // GQ

                 // if pConvTable is NULL the target is ASCII else Ansi
                 if ( pConvTable )
                 {
                   UtlDirectUnicode2AnsiBuf( chUnicodeString, (PSZ)ucReplaceChars, 1,
                                             sizeof(ucReplaceChars),
                                             pTBDoc->ulAnsiCodePage, FALSE, &lRc );
                 }
                 else
                 {
                   Unicode2ASCII( chUnicodeString, (PSZ)ucReplaceChars, pTBDoc->ulOemCodePage );
                 } /* endif */

                 /* END TEST MK */
                 // add \uc tag if not active yet
                 if (ucReplaceChars[1] == 0)             //Aoki
                 {                                       //Aoki
                   if ( pParseData->iUCStatus != 1 )
                   {
                     UTF16strcpy( pszTarget, L"\\uc1 " );
                     pParseData->iUCStatus = 1;
                     pszTarget += UTF16strlenCHAR( pszTarget );
                   } /* endif */
                 }                                       //Aoki
                 else                                    //Aoki
                 {                                       //Aoki
                   if ( pParseData->iUCStatus != 2 )
                   {
                     UTF16strcpy( pszTarget, L"\\uc2 " );  //Aoki
                     pParseData->iUCStatus = 2;
                     pszTarget += UTF16strlenCHAR( pszTarget );
                   } /* endif */
                   fCurCharIsDBCS = TRUE;
                   // check for DBCS punctuation
                   if ( ((ucReplaceChars[0] == 0x81) && (ucReplaceChars[1] == 0x41)) ||
                        ((ucReplaceChars[0] == 0x81) && (ucReplaceChars[1] == 0x42)) )
                   {
                     fCurCharIsDBCSPunct = TRUE;
                   } /* endif */
                 } /* endif */


                 // encode as unicode character and add replacement character
                 sUniChar = (short) *pszSource;         // MK
                 if (ucReplaceChars[1] == 0)                                       //Aoki
                 {                                                                 //Aoki
                   // write replacement char in hex notation (otherwise the blank
                   // following the \u tag will be handled as replacement char
                   // in UnparseRTF
                   pszTarget += swprintf( pszTarget, L"\\u%li\\\'%2.2x", sUniChar,   //MK
                                            ucReplaceChars[0] );                     //GQ
                 }                                                                 //Aoki
                 else                                                              //Aoki
                 {                                                                 //Aoki
                   //pszTarget += swprintf( pszTarget, L"\\u%li\\'%x",          //Aoki
                   //                         *pszSource,ucReplaceChars[0],ucReplaceChars[1]); //Aoki
                   pszTarget += swprintf( pszTarget, L"\\u%li\\'%x\\'%x",                 //MK
                                            sUniChar,ucReplaceChars[0],ucReplaceChars[1]);//MK

                 }                                                                 //Aoki
                 chLastChar = *pszSource;
                 fChangeSeg = TRUE; // segment data has been changed

               } /* endif */
             } /* endif */
           } /* endif */
           pszSource++;
         } /* endwhile */
         *pszTarget = EOS;

         if ( pSeg->qStatus == QF_NOP )
         {
           // remove any trailing LF character
           USHORT usLen = (USHORT)UTF16strlenCHAR(pSeg->pDataW);
           if ( (usLen != 0) && (pSeg->pDataW[usLen-1] == LF_W) )
           {
             pSeg->pDataW[usLen-1] = 0;
           } /* endif */
         } /* endif */

         // set active segment pointer
         pszSegData = fChangeSeg ? pszTempSeg : pSeg->pDataW;

         // apply any tag changes
         if ( pParseData->chChangeFromTagsW[0] )
         {
           BOOL fSegChanged = FALSE;

           fSegChanged = RTFChangeTagsW( pszSegData,
                                         pParseData->szChangeTagsBufferW,
                                         pParseData->chChangeFromTagsW,
                                         pParseData->chChangeToTagsW );
           if ( fSegChanged )
           {
             pszSegData = pParseData->szChangeTagsBufferW;
           } /* endif */
         } /* endif */

         // if the changed segment exceeds the segment limit (either by additional
         // BIDI tags, Unicode encoding or change tag activity)
         // we have to split it into smaller ones
         // otherwise write it as-is to new document
         {
           ULONG ulSegLenChar = UTF16strlenCHAR( pszSegData );

           if ( (ulSegLenChar + 1) >= MAX_SEGMENT_SIZE )
           {
             PSZ_W pszSegStart = pszSegData;
             BOOL fDone = FALSE;

             do
             {
               PSZ_W pszCurPos = pszSegStart;    // ptr for split position search
               ULONG ulPos = 0;                  // current lenth of searched data
               PSZ_W pszSplitHere = NULL;        // pointer to segment split position

               // scan data and find possible split positions
               pszCurPos = pszSegStart;
               while ( *pszCurPos && ((ulPos + 1) < MAX_SEGMENT_SIZE) )
               {
                 if ( iswspace(*pszCurPos) )
                 {
                   // use space as split position and continue with next character
                   pszSplitHere = pszCurPos;
                   pszCurPos++;
                   ulPos++;
                 }
                 else if ( *pszCurPos == START_CTRLWORD_W )
                 {
                   // use start of control word as split position
                   pszSplitHere = pszCurPos;
                   pszCurPos++;
                   ulPos++;

                   // avoid splitting at double backslash or control words
                   // starting with "\*\"
                   if ( *pszCurPos == START_CTRLWORD_W )
                   {
                    pszCurPos++;
                    ulPos++;
                   }
                   else if ( (*pszCurPos == L'*') && (*(pszCurPos+1) == START_CTRLWORD_W) )
                   {
                    pszCurPos += 2;
                    ulPos += 2;
                   } /* endif */
                 }
                 else
                 {
                   // try next character
                   pszCurPos++;
                   ulPos++;
                 } /* endif */
               } /*endwhile */

               // end of segment data or segment size limit exceeded
               if ( *pszCurPos )
               {
                 //
                 // segment size limit exceeded
                 //
                 CHAR_W chTemp;

                 if ( pszSplitHere == NULL )
                 {
                   // use current position for split as not split position has been found
                   pszSplitHere = pszCurPos;
                 } /* endif */

                 // terminate segment data
                 chTemp = *pszSplitHere;
                 *pszSplitHere = 0;

                 // write segment
                 sRc = RTFAddNewSeg( pTBNewDoc, pSeg, pszSegStart, &ulOutSegNum );

                 // continue with remaining data
                 *pszSplitHere = chTemp;
                 pszSegStart = pszSplitHere;
               }
               else
               {
                 // end of segment data reached, write rest of segment
                 sRc = RTFAddNewSeg( pTBNewDoc, pSeg, pszSegStart, &ulOutSegNum );
                 fDone = TRUE;
               } /* endif */
             } while ( !sRc && !fDone );
           }
           else
           {
             // add segment as-is to new document
             sRc = RTFAddNewSeg( pTBNewDoc, pSeg, pszSegData, &ulOutSegNum );
           } /* endif */
         }

       } /* endif */

       // get next segment but ignore joined ones...
       do
       {
         ulSegNum++;                         // point to next segment
         pParseData->iUCStatus = 0;          //Aoki
         pSeg = EQFBGetSegW(pTBDoc, ulSegNum);
       } while ( pSeg && (pSeg->SegFlags.Joined && !pSeg->SegFlags.JoinStart) );

     } /* endwhile */
   } /* endif */

   /*******************************************************************/
   /* write the new segmented target file out                         */
   /*******************************************************************/
   if ( !sRc && !*pfKill )
   {
     sRc = EQFBFileWrite( pszTarget, pTBNewDoc );
   } /* endif */

   /*******************************************************************/
   /* free allocated memory                                           */
   /*******************************************************************/
   if ( pTBDoc )
   {
     TAFreeTagTable( (PLOADEDTABLE) pTBDoc->pQFTagTable );
     RTFFreeDoc( (PVOID *)&pTBDoc );
   } /* endif */

   if ( pTBNewDoc )
   {
     TAFreeTagTable( (PLOADEDTABLE) pTBNewDoc->pQFTagTable );
     TAFreeTagTable( (PLOADEDTABLE) pTBNewDoc->pDocTagTable );
     RTFFreeDoc( (PVOID *)&pTBNewDoc );
   } /* endif */

   if ( pTBSrcDoc )
   {
     RTFFreeDoc( (PVOID *)&pTBSrcDoc );
   } /* endif */
   if (  pszTempSeg ) UtlAlloc( (PVOID *)&pszTempSeg, 0L, 0L, NOMSG );
   if (  pParseData ) UtlAlloc( (PVOID *)&pParseData, 0L, 0L, NOMSG );

   LOGWRITE1( "\n----------------------------------\n" );
   LOGCLOSE();

   return( sRc );
} /* endof RTFPreUnseg */

static VOID RTFFreeDoc
(
  PVOID * ppDoc
)
{
   PTBSEGMENTTABLE pSegTable;          // ptr for segment table deleting
   ULONG           ulI, ulJ;           // loop counter
   PTBSEGMENT      pSegment;           // ptr for segment deleting
   PTBDOCUMENT     pDoc = (PTBDOCUMENT) *ppDoc;
   /* Free up the document space */

   UtlAlloc( (PVOID *) &pDoc->pInBuf, 0L, 0L, NOMSG );
   UtlAlloc( (PVOID *) &pDoc->pTokBuf, 0L, 0L, NOMSG );

   pSegTable = pDoc->pSegTables;
   for ( ulI = 0; ulI < pDoc->ulSegTables; ulI++ )
   {
      pSegment = pSegTable->pSegments;
      for ( ulJ = 0; ulJ < pSegTable->ulSegments; ulJ++ )
      {
         if ( pSegment->pData )
         {
            UtlAlloc( (PVOID *) &pSegment->pData, 0L, 0L, NOMSG );
         } /* endif */
         if ( pSegment->pDataW )
         {
            UtlAlloc( (PVOID *) &pSegment->pDataW, 0L, 0L, NOMSG );
         } /* endif */
         if ( pSegment->pusBPET )
         {
            UtlAlloc( (PVOID *) &pSegment->pusBPET, 0L, 0L, NOMSG );
         } /* endif */
         if ( pSegment->pvMetadata )
         {
            UtlAlloc( (PVOID *) &pSegment->pvMetadata, 0L, 0L, NOMSG );
         } /* endif */
         pSegment++;
      } /* endfor */
      UtlAlloc( (PVOID *) &pSegTable->pSegments, 0L, 0L, NOMSG );
      pSegTable++;
   } /* endfor */
   pDoc->ulSegTables = 0;
   UtlAlloc( (PVOID *) &pDoc->pSegTables, 0L, 0L, NOMSG );
   UtlAlloc( (PVOID *) &pDoc->pUndoSeg, 0L, 0L, NOMSG );  //free storage of Undo

   UtlAlloc( (PVOID *) &pDoc, 0L, 0L, NOMSG );
   /*******************************************************************/
   /* pass back new value for pDoc...                                 */
   /*******************************************************************/
   *ppDoc = pDoc;
} /* end of function RTFFreeDoc */

/**********************************************************************/
/* Function to count number of curly braces in text strings           */
/**********************************************************************/
SHORT CountBraces( PSZ_W pszText )
{
  SHORT sBraces = 0;

  while ( *pszText != 0 )
  {
    //KA if ( _isdbcs(*pszText) == DBCS_1ST )
    // GQ: Check is not required anymore ...
    //if (IsDBCSLeadByte((BYTE)*pszText ) == TRUE) //KA
    //{
    //  pszText++;                       // skip 1st DBCS character, 2nd character
    //                                   // will be skipped at the end of the loop
    //} else
    if ( *pszText == BEGIN_GROUP_W )
    {
      sBraces++;
    }
    else if ( *pszText == END_GROUP_W )
    {
      sBraces--;
    }
    else if ( (*pszText == START_CTRLWORD_W) &&
              ( (*(pszText+1) == BEGIN_GROUP_W) ||
                (*(pszText+1) == END_GROUP_W) ) )
    {
      /****************************************************************/
      /* handle curly brace as character and skip backslash, curly    */
      /* brace will be skipped at the end of this loop                */
      /****************************************************************/
      pszText++;
    } /* endif */
    if ( *pszText != 0 ) pszText++;
  } /* endwhile */
  return( sBraces );
}

/**********************************************************************/
/* RTFMorphTokenize                                                   */
/*                                                                    */
/* Build a flagged term list for the data in the abOutBuf buffer.     */
/* The abCharBuf must be filled for correct tag detection.            */
/**********************************************************************/
USHORT RTFMorphTokenize
(
  PPARSEDATA  pParseData               // ptr to parser data area
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  /********************************************************************/
  /* Terminate segment data                                           */
  /********************************************************************/
  pParseData->abSegBuf[pParseData->usOutBufUsed] = EOS;

  /****************************************************************/
  /* Ignore current contents of segment buffer and build up a     */
  /* new one containing only the non-tag data                     */
  /****************************************************************/
  {
    PSZ       pszTarget = (PSZ)pParseData->abSegBuf;  // target pointer
    USHORT    usI = 0;                 // loop counter

    while ( usI < pParseData->usOutBufUsed )
    {
      if ( pParseData->abCharBuf[usI] != CH_TAG )
      {
        *pszTarget++ = pParseData->abOutBuf[usI];
      } /* endif */
      usI++;
    } /* endwhile */
    *pszTarget++ = EOS;
  }

  /********************************************************************/
  /* Mask leading spaces (or POE will assume a segment break...)      */
  /********************************************************************/
  {
    PSZ  pszTemp = (PSZ)pParseData->abSegBuf;
    while ( (*pszTemp != EOS) && (*pszTemp == SPACE) )
    {
      *pszTemp++ = 'x';
    } /* endwhile */
  }

  /********************************************************************/
  /* Tokenize the data                                                */
  /********************************************************************/
  usRC = MorphTokenize( pParseData->sLangID,
                        (PSZ)pParseData->abSegBuf,
                        &(pParseData->usTermListSize),
                        (PVOID *)&(pParseData->pTermList),
                        MORPH_FLAG_OFFSLIST );

  /****************************************************************/
  /* Adjust offsets in returned termlist to actual data layout    */
  /****************************************************************/
  if ( usRC == NO_ERROR )
  {
    USHORT usCurOffs = 0;          // actual offset into data area
    USHORT usDelta   = 0;          // delta between segment buffer and data
                                   //   buffer offsets
    PFLAGOFFSLIST pTerm  = (PFLAGOFFSLIST)pParseData->pTermList;

    pTerm  = (PFLAGOFFSLIST)pParseData->pTermList;
    while ( (pTerm->usOffs != 0) ||
            (pTerm->usLen  != 0) ||
            (pTerm->lFlags != 0L ) )
    {
      /****************************************************************/
      /* Process data up to begin of current term                     */
      /****************************************************************/
      pTerm->usOffs = (USHORT)(pTerm->usOffs + usDelta);        // adjust start offset of term
      while ( usCurOffs < pTerm->usOffs )
      {
        // if there is tag data at the current position ...
        if ( pParseData->abCharBuf[usCurOffs] == CH_TAG )
        {
          pTerm->usOffs += 1;          // adjust term offset
          usDelta++;                   // and delta
        } /* endif */
        usCurOffs++;
      } /* endwhile */

      /****************************************************************/
      /* Adjust length of current term                                */
      /****************************************************************/
      // while we are inside the data of the current term ...
      while (usCurOffs < (pTerm->usOffs + pTerm->usLen) )
      {
        // if there is tag data at the current position ...
        if ( pParseData->abCharBuf[usCurOffs] == CH_TAG )
        {
          pTerm->usLen += 1;           // ... adjust length of term and
          usDelta++;                   // delta
        } /* endif */
        usCurOffs++;
      } /* endwhile */

      /****************************************************************/
      /* Continue with next term                                      */
      /****************************************************************/
      pTerm++;
    } /* endwhile */
  } /* endif */

  return( usRC );
} /* end of function RTFMorphTokenize */

/**********************************************************************/
/* IsBidiPunct                                                        */
/*                                                                    */
/* checks for stopping chars                                          */
/**********************************************************************/
BOOL IsBidiPunct( CHAR_W chTest)
{
  return (chTest == L'.' ||
      chTest == L';' ||
      chTest == L':' ||
      chTest == L',');
}
/**********************************************************************/
/* IsBidiPunct                                                        */
/*                                                                    */
/* checks for stopping chars                                          */
/**********************************************************************/
BOOL IsBidiOpenBrace( CHAR_W chTest)
{
  return (chTest == L'(' || chTest == L'[');
}
/**********************************************************************/
/* RTFContainsAnsiTag                                                 */
/*                                                                    */
/* Looks for \ANSI tag in the supplied string.                        */
/**********************************************************************/
BOOL RTFContainsAnsiTag( PSZ pszString )
{
  PSZ     pszTag;
  PSZ     pszAnsiTag = ANSI_TAG;
  SHORT   sAnsiTagLen = (SHORT)strlen(pszAnsiTag);
  BOOL    fAnsiTagFound = FALSE;

  pszTag = strchr( pszString, START_CTRLWORD );
  while ( (pszTag != NULL) && !fAnsiTagFound )
  {
    if ( _strnicmp( pszTag, pszAnsiTag, sAnsiTagLen ) == 0 )
    {
      fAnsiTagFound = TRUE;
    }
    else
    {
      pszTag = strchr( pszTag + 1, START_CTRLWORD );
    } /* endif */
  } /* endwhile */

  return( fAnsiTagFound );
} /* end of fucntion RTFContainsAnsiTag */

/**********************************************************************/
/* RTFDataToRef                                                       */
/*                                                                    */
/* Skip data up to end of current destination and write a reference   */
/* to the data into the output segment                                */
/*                                                                    */
/* The reference consists of 3 numbers which are separated            */
/* by a comma:                                                        */
/*                                                                    */
/* {\xxxx 234,4096,7869}                                              */
/*                                                                    */
/*                   +--->  (ULONG) checksum of first 20 bytes of data*/
/*              +-------->  (ULONG) length of data                    */
/*         +------------->  (ULONG) offset of data in source file     */
/**********************************************************************/
USHORT RTFDataToRef
(
  PPARSEDATA  pParseData               // points to parser data structure
)
{
  ULONG       ulPicOffs;               // offset to pict data in the document
  ULONG       ulPicLength = 0L;        // length of pict data
  ULONG       ulCheckSum = 0L;         // checksum over the first 20 bytes
  SHORT       sBytesToCheck = 20;      // number of bytes to be used for checksum
  USHORT      usRC = NO_ERROR;         // function return code
  register BYTE        bCurrent;       // current byte from input buffer
  SHORT       sNesting = 0;            // current nesting of curly braces
  BOOL        fEndOfPic = FALSE;       // end-of-picture-data flag

  /********************************************************************/
  /* Remember start position of picture data                          */
  /* The start position is the difference between the total bytes of  */
  /* the document and the bytes still left to read plus the bytes     */
  /* in the input buffer minus any characters in the undo buffer      */
  /********************************************************************/
  ulPicOffs = (ULONG)pParseData->lTotalBytes - (ULONG)pParseData->lBytesToRead -
              (ULONG)pParseData->usBytesInBuffer -
              (ULONG)pParseData->usUndoBufUsed;

  /********************************************************************/
  /* Skip picture data up to closing curly brace                      */
  /********************************************************************/
  do
  {
    // get next character from document
    bCurrent = ParseNextChar( pParseData, &usRC );

    // handle current character
    if ( !usRC && !*(pParseData->pfKill) )
    {
      // handle checksum
      if ( sBytesToCheck )
      {
        ulCheckSum += bCurrent;
        sBytesToCheck--;
      } /* endif */

      switch ( bCurrent )
      {
        case START_CTRLWORD:
          /************************************************************/
          /* Check for the curly brace characters so we do not        */
          /* misinterpret them as real curly braces                   */
          /************************************************************/
          bCurrent = ParseNextChar( pParseData, &usRC );
          if ( (bCurrent == BEGIN_GROUP) || (bCurrent == END_GROUP) )
          {
            ulPicLength += 2L;
          }
          else
          {
            UndoChar( pParseData, bCurrent );
            ulPicLength++;
          } /* endif */
          break;

        case BEGIN_GROUP:
          sNesting++;
          ulPicLength++;
          break;

        case END_GROUP:
          if ( sNesting == 0 )
          {
            // end of picture data detected
            fEndOfPic = TRUE;
            UndoChar( pParseData, bCurrent );
          }
          else
          {
            sNesting--;
            ulPicLength++;
          } /* endif */
          break;

        default:
          //KA if ( _isdbcs(bCurrent ) == DBCS_1ST )
          if (IsDBCSLeadByteEx(pParseData->ulSrcOemCP, (BYTE)bCurrent ) == TRUE) //KA
          {
            // treat this byte and the next byte as one unit
            bCurrent = ParseNextChar( pParseData, &usRC );
            ulPicLength += 2L;
          }
          else
          {
            // skip a single byte
            ulPicLength++;
          } /* endif */
          break;
      } /* endswitch */
    } /* endif */
  } while ( !fEndOfPic && !usRC && !*(pParseData->pfKill) ); /* enddo */

  // insert picture reference
  if ( !usRC && !*(pParseData->pfKill) )
  {
    CHAR szPicRef[32];
    PSZ pszPicRef = szPicRef;

    sprintf( szPicRef, " %lu,%lu,%lu", ulPicOffs, ulPicLength, ulCheckSum );

    while ( *pszPicRef && !usRC && !*(pParseData->pfKill) )
    {
      usRC = AddToSegment( pParseData, *pszPicRef++, FALSE, CH_TAG );
    } /* endwhile */
  } /* endif */

  /* no longer necessary - check for this in RTFRefToData disabled
  if (ulPicLength == 0)
  {
    UtlError( ERR_RTF_CANTDETECTPICSIZE, MB_OK | MB_DEFBUTTON1,
              0, NULL, EQF_ERROR );
  }
  */

  return( usRC );
} /* end of function RTFDataToRef */

/**********************************************************************/
/* RTFRefToData                                                       */
/*                                                                    */
/* Resolve a reference and insert the actual data from the            */
/* source file.                                                       */
/**********************************************************************/

#define PICBUFSIZE            4096              // block size for reading
#define RESTOREPICBUFSIZE      256              // for block saving of previous block
                                                // must be <= PICBUFSIZE
#define MAX_RTF_FONTTBL_ENTRY  256              // should be enough mem for a single font definition entry

USHORT RTFRefToData
(
  PPARSEDATA  pParseData,              // parser data structure
  HFILE       hfSource,                // file handle of source file
  PBYTE       pbCurrent,               // ptr to caller's character buffer
  SHORT       sObjType                 // type of referenced object
)
{
  ULONG       ulPicOffs = 0L;          // offset of picture
  ULONG       ulPicLen = 0L;           // picture length
  ULONG       ulCheckSum = 0L;         // checksum of first 20 bytes of pic data
  BOOL        fOK = TRUE;              // internal O.K. flag
  USHORT      usRC = NO_ERROR;         // function return code
  PBYTE       pbOldInBufPtr;           // buffer for input buffer pointer
  USHORT      usOldBytesInBuffer;      // buffer for bytes in buffer counter
  BOOL        fFirstDataBlock = TRUE;  // first-data-block-of-picture flag
  BYTE        bOldCurrent;             // buffer for current byte
  PBYTE       pBuffer = NULL;          // ptr to buffer for picture data

  BOOL fReplaceFontOneToOne = FALSE;
  BOOL fReplaceDefaultFont = FALSE;
  BOOL fFontTagFound = FALSE;

  enum
  {
    WAITFORSTART,
    WAITFORFONTTAG,
    WAITFORNUMBER,
    COPY_REMAININGFONTDEF
  } WaitState = WAITFORSTART;          // current wait state

  SHORT sOldBlockBytesLeft = 0;

  BOOL fFontFamFound = FALSE;          // used to mark right order for scanning font families

  SHORT sFontNumber = 0;               // number of current font
  SHORT sLastFont = 0;                 // number of last font

  USHORT usBytesRead = 0;
  signed short sNewFontEntryPos = 0;                    // SHORT important here
  SHORT  sFontFamEntryPos = 0;

  LOGFILE();                           // defines a log file

  LOGOPEN( "C:\\OTMRTF.LOG" );

  LOGWRITE2( "RefToData Processing for object type %d\n", sObjType );



  // remember current input buffer pointer
  pbOldInBufPtr = pParseData->pInBuf;
  usOldBytesInBuffer = pParseData->usBytesInBuffer;
  bOldCurrent = *pbCurrent;

  // go back one byte as current byte is already first byte of offset
  pParseData->pInBuf--;
  pParseData->usBytesInBuffer++;

  // skip any whitespace
  while ( (*pParseData->pInBuf == SPACE) && pParseData->usBytesInBuffer )
  {
    pParseData->pInBuf++;
    pParseData->usBytesInBuffer--;
  } /* endwhile */

  // get picture offset
  if ( fOK )
    {
    while ( isdigit(*pParseData->pInBuf) && pParseData->usBytesInBuffer )
    {
      ulPicOffs = (ulPicOffs * 10L) + (ULONG)(*pParseData->pInBuf - '0');
      pParseData->pInBuf++;
      pParseData->usBytesInBuffer--;
    } /* endwhile */
  } /* endif */

  // skip delimiter
  if ( fOK )
  {
    if ( (*pParseData->pInBuf != COMMA) || !pParseData->usBytesInBuffer )
    {
      fOK = FALSE;                     // pic reference is in error
    }
    else
    {
      pParseData->pInBuf++;
      pParseData->usBytesInBuffer--;
    } /* endif */
  } /* endif */

  // get picture data length
  if ( fOK )
  {
    while ( isdigit(*pParseData->pInBuf) && pParseData->usBytesInBuffer )
    {
      ulPicLen = (ulPicLen * 10L) + (ULONG)(*pParseData->pInBuf - '0');
      pParseData->pInBuf++;
      pParseData->usBytesInBuffer--;
    } /* endwhile */
  } /* endif */

  // skip delimiter
  if ( fOK )
  {
    if ( (*pParseData->pInBuf != COMMA) || !pParseData->usBytesInBuffer )
    {
      fOK = FALSE;                     // pic reference is in error
    }
    else
    {
      pParseData->pInBuf++;
      pParseData->usBytesInBuffer--;
    } /* endif */
  } /* endif */

  // get checksum
  if ( fOK )
  {
    while ( isdigit(*pParseData->pInBuf) && pParseData->usBytesInBuffer )
    {
      ulCheckSum = (ulCheckSum * 10L) + (ULONG)(*pParseData->pInBuf - '0');
      pParseData->pInBuf++;
      pParseData->usBytesInBuffer--;
    } /* endwhile */
  } /* endif */

  // set caller's bCurrent to current character
  if ( fOK && pParseData->usBytesInBuffer )
  {

    *pbCurrent = *pParseData->pInBuf++;
    pParseData->usBytesInBuffer--;
  } /* endif */

  LOGWRITE2( "  ulPicOffs  = %ld\n", ulPicOffs );
  LOGWRITE2( "  ulPicLen   = %ld\n", ulPicLen );
  LOGWRITE2( "  ulCheckSum = %ld\n", ulCheckSum );

  // check some values
  if ( fOK )
  {
    if ( (ulCheckSum == 0L) || (ulPicOffs == 0L) )
    {
      // seems to be no valid picture reference
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // position to picture data
  if ( fOK )
  {
    ULONG ulOffs;

    fOK = UtlChgFilePtr( hfSource, ulPicOffs, FILE_BEGIN,
                         &ulOffs, FALSE) == NO_ERROR;
  } /* endif */

  // allocate buffer for picture data
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pBuffer, 0L, (LONG)PICBUFSIZE, ERROR_STORAGE );
  } /* endif */

  // flush output buffer and write CRLF to output file
  if ( fOK )
  {
    USHORT usBytesWritten;
    if ( pParseData->usOutBufUsed )
    {
      usRC = UtlWrite( pParseData->hOutFile, pParseData->abOutBuf,
                       pParseData->usOutBufUsed, &usBytesWritten, TRUE );
    } /* endif */

    if ( !usRC )
    {
      usRC = UtlWrite( pParseData->hOutFile, "\r\n", 2,
                       &usBytesWritten, TRUE );
    } /* endif */

    // reset outpuffer used count and output buffer pointer
    if ( !usRC )
    {
      pParseData->pOutBuf = pParseData->abOutBuf;
      pParseData->usOutBufUsed = 0;
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* write it out                                                     */
  /********************************************************************/
  while ( ulPicLen && fOK )
  {
    USHORT usBytesToRead = (USHORT) min( (ULONG)PICBUFSIZE, ulPicLen );
    SHORT  sFSubstEndPos;
    CHAR pszRestoreBuffer[RESTOREPICBUFSIZE];

    //
    // keep the last RESTOREPICBUFSIZE bytes of previous data block
    // important for fonttable conversion handling
    //

    if (!fFirstDataBlock)
    {
       memcpy(pszRestoreBuffer, &pBuffer[usBytesRead - RESTOREPICBUFSIZE], RESTOREPICBUFSIZE);
    }

    fOK = (UtlRead( hfSource, pBuffer,
                    usBytesToRead, &usBytesRead, TRUE ) == NO_ERROR );


    if ( fOK )
    {
      ulPicLen -= (ULONG)usBytesRead;
    }

    LOGWRITE2( "  writing block, remaining = %ld\n", ulPicLen );

    // compare checksum if this is the first data block of the picture
    // if ( fOK && fFirstDataBlock )
    // {
    //   SHORT i;
    //   ULONG ulDataCheckSum = 0L;
    //
    //   for ( i = 0; i < 20; i++ )
    //   {
    //     ulDataCheckSum += (BYTE)pBuffer[i];
    //   }
    //
    //   fOK = (ulDataCheckSum == ulCheckSum);
    //   fFirstDataBlock = FALSE;
    // }

    //
    // add picture data of current block to output file
    // special handling for fonttable: keep track of font
    // numbers in use and add any additional font definition
    // stored in chAddFont field of ParseData structure
    //
    if ( fOK )
    {
      int i, j, k, l;
      int sLevel = 0; // current nesting level inside font definition

      USHORT usFontFamilyListPos;
      CHAR chFontTabEntry[MAX_RTF_FONTTBL_ENTRY];

      // font table specific processing
      if ( sObjType == FONTTBL_RTFTAG )
      {
        for ( i = 0; ( (i < (int)usBytesRead) && (usRC == NO_ERROR) ); i++ )
        {
          // check for \fnnn tag and keep track of last used font number

          if (sOldBlockBytesLeft)                   // added 300501 bt
            WaitState = COPY_REMAININGFONTDEF;

          switch ( WaitState )
          {
            case WAITFORSTART :
              WaitState = (pBuffer[i] == BACKSLASH) ? WAITFORFONTTAG
                                                    : WAITFORSTART;
              break;
            case WAITFORFONTTAG :
              if (toupper(pBuffer[i]) == 'F')
              {
                 if (!fFontTagFound)
                 {
                    //
                    // strange but we could have a \n char here: {\n\fxyz\...
                    //
                    // note: i could be negative here
                    //

                    for (sNewFontEntryPos = (short)(i - 1); ; sNewFontEntryPos --)
                    {
                        if (sNewFontEntryPos >= 0 && pBuffer[sNewFontEntryPos] == '{')
                           break;
                        if (sNewFontEntryPos < 0 && !fFirstDataBlock && pszRestoreBuffer[RESTOREPICBUFSIZE + sNewFontEntryPos] == '{')
                        {
                           // block change has happened

                           sNewFontEntryPos += PICBUFSIZE;   // added 181200 bt
                           break;
                        }
                        if (sNewFontEntryPos < 0 && fFirstDataBlock)
                        {
                           // special case if there's only one single font defined
                           // -it's not enclosed in {} braces
                           // added 300501 bt

                           sNewFontEntryPos ++;
                           break;
                        }
                    }

                    WaitState = WAITFORNUMBER;
                 }

                 if (fFontTagFound && !fFontFamFound)
                 {
                    // the font family tag follows immediately after the font number tag
                    // and is a necessary tag
                    // look comment above ...
                    for (sFontFamEntryPos = (short)(i - 1); ; sFontFamEntryPos--)
                    {
                      if (sFontFamEntryPos > 0 && pBuffer[sFontFamEntryPos] == '\\')
                         break;
                      if (sFontFamEntryPos <= 0 && !fFirstDataBlock && pszRestoreBuffer[RESTOREPICBUFSIZE + sFontFamEntryPos] == '\\')
                         break;
                      if (sFontFamEntryPos < 0 && fFirstDataBlock)
                      {
                         sFontFamEntryPos++;
                         break;
                      }
                    }
                    fFontFamFound = TRUE;
                 }
              }
              else
              {
                 WaitState = WAITFORSTART;
              }

              break;
            case WAITFORNUMBER :
              if ( isdigit(pBuffer[i]) )
              {
                sFontNumber = (sFontNumber * 10) + (pBuffer[i] - '0');
                fFontTagFound = TRUE;
              }
              else
              {
                if ( sFontNumber != 0)
                {
                  sLastFont = sFontNumber;
                  sFontNumber = 0;
                } /* endif */

                if (pBuffer[i] == '\\')
                   WaitState = WAITFORFONTTAG;
                else
                   WaitState = WAITFORSTART;

              } /* endif */
              break;

            case COPY_REMAININGFONTDEF :
              // write out remaining bytes of last font definition
              // - we had a block change before
              // added 300501 bt
              usRC = RTFWriteBuffered( pParseData, pBuffer[i], TRUE );
              sOldBlockBytesLeft --;

              if (!sOldBlockBytesLeft)
                WaitState = WAITFORSTART;

              break;

          } /* endswitch */

          //
          // font substitution (CHANGEFONT / DEFCHGFONT section)
          //
          // check for ';' which points to the end of a single font
          // definition entry of the table
          // there are two possibilities in RTF:
          //
          // \f<num>(...)<fontname to replace>{\*\falt <fontname>};
          // \f<num>(...)<fontname to replace>;
          //
          // Note: careful with the blocks...
          //       in large fonttbl sections it could happen that
          //       a matching sign isn't in the current block read
          //
          // And:  think of a possible \n char:  (...)}\n;(...)
          //

          if ( (pBuffer[i] == '{') && fFontTagFound )
          {
            // we have groups inside the font definition
            sLevel++;
          }
          else if ( (pBuffer[i] == '}') && fFontTagFound )
          {
            // we leave a inside groups of the font definition
            if ( sLevel ) sLevel--;
          }
          else if ( (pBuffer[i] == ';') && (sLevel == 0) )
          {
              if (i > 0 && pBuffer[i-1] == '\n' ||
                 i == 0 && !fFirstDataBlock && pszRestoreBuffer[RESTOREPICBUFSIZE] == '\n')
              {
                 // set offset
                 k = 1;
              }
              else
              {
                 k = 0;
              }

              if (i > 0 + k && pBuffer[i-(1+k)] == '}' ||
                  i <= 0 + k && !fFirstDataBlock && pszRestoreBuffer[RESTOREPICBUFSIZE - k] == '}')
              {
                 // go backwards and skip {\*\falt ...}

                 for (sFSubstEndPos = (short)(i - 1); sFSubstEndPos >= 0; sFSubstEndPos --)
                 {
                    if (sFSubstEndPos >= 0)
                    {
                       if (pBuffer[sFSubstEndPos] == '{')
                          break;
                    }
                    else  // do the same if the matching char is in the old block
                    {
                       if (!fFirstDataBlock)
                       {
                          if (pszRestoreBuffer[RESTOREPICBUFSIZE + sFSubstEndPos] == '{')
                             break;
                       }
                    }
                 } /* end for */
              }
              else
              {
                 sFSubstEndPos = (short)i;
              }

              // check if we have to substitute with an exact match (CHANGEFONT section)
              for (j=0, fReplaceFontOneToOne=FALSE; j < pParseData->usChangeFontTabEntries; j++)
              {
                 // do some patchwork if we've got an over-border spec
                 for (k = 0, l = sFSubstEndPos; k < pParseData->ChangeFontTab[j].usFontNameLen; k++, l++)
                 {
                    if ((SHORT)(l - pParseData->ChangeFontTab[j].usFontNameLen) >= 0) // changed to (SHORT) 300501 bt
                    {
                       chFontTabEntry[k] = pBuffer[l - pParseData->ChangeFontTab[j].usFontNameLen] ;
                    }
                    else
                    {
                       if (!fFirstDataBlock)
                          chFontTabEntry[k] = pszRestoreBuffer[l - pParseData->ChangeFontTab[j].usFontNameLen] ;
                    }
                 }

                 if (!memcmp((void *)pParseData->ChangeFontTab[j].pszFontName,
                             (void *)&chFontTabEntry,
                             pParseData->ChangeFontTab[j].usFontNameLen))
                 {
                    fReplaceFontOneToOne = TRUE;
                    break;
                 }
              }

              //
              // prepare substitution string
              //
              if (fReplaceFontOneToOne)
              {
                 sprintf(chFontTabEntry, "{\\f%d", sLastFont);

                 // ignore 4 chars at beginning
                 strncat(chFontTabEntry, &pParseData->ChangeFontTab[j].pszSubstFontRTFSpec[4],
                         pParseData->ChangeFontTab[j].usSubstFontRTFSpecLen - 3);
              }
              else
              {
                 //
                 // we haven't found an exact match
                 // now check of we have to substitute
                 // a default font (DEFCHGFONT section)
                 //
                 for (j=0, fReplaceDefaultFont=FALSE; DefChgFntTag2Txt[j].FontFamilyTag != RTF_FF_NULL; j++)
                 {
                    // do some patchwork if we've got an over-border spec
                    for (k = 0, l = sFontFamEntryPos + 2; k < (int)strlen(DefChgFntTag2Txt[j].szFontFamily); k++, l++)
                    {
                       if (l < PICBUFSIZE)
                       {
                          chFontTabEntry[k] = pBuffer[l] ;
                       }
                       else
                       {
                          if (!fFirstDataBlock)
                             chFontTabEntry[k] = pszRestoreBuffer[RESTOREPICBUFSIZE - (PICBUFSIZE - l)] ;
                       }
                    }

                    if (!memcmp((void *)DefChgFntTag2Txt[j].szFontFamily,
                                (void *)&chFontTabEntry,
                                strlen(DefChgFntTag2Txt[j].szFontFamily)))
                    {
                       // we found the same font family string

                       usFontFamilyListPos = (USHORT)j;

                       // now check if we have a setting for this family

                       for (j=0, fReplaceDefaultFont=FALSE; j < pParseData->usDefChgFontTabEntries; j++)
                       {
                          if (pParseData->DefChgFontTab[j].FontFamilyTag ==
                              DefChgFntTag2Txt[usFontFamilyListPos].FontFamilyTag)
                          {
                             sprintf(chFontTabEntry, "{\\f%d", sLastFont);

                             // ignore 4 chars at beginning

                             strncat(chFontTabEntry, &pParseData->DefChgFontTab[j].pszSubstFontRTFSpec[4],
                                     pParseData->DefChgFontTab[j].usSubstFontRTFSpecLen - 3);

                             fReplaceDefaultFont = TRUE;

                             break;
                          }
                       }

                       break;
                    }
                 }

                 if (!fReplaceDefaultFont && !fReplaceFontOneToOne)
                 {
                    //
                    // nothing at all found -
                    // keep original
                    //

                    if ((SHORT)(i - sNewFontEntryPos) < 0)    // changed to (SHORT) 300501 bt
                    {
                       // backward block change has happened

                       memcpy((void *)chFontTabEntry,
                              (void *)&pszRestoreBuffer[RESTOREPICBUFSIZE - (PICBUFSIZE - sNewFontEntryPos)],
                              PICBUFSIZE - sNewFontEntryPos);
                       chFontTabEntry[PICBUFSIZE - sNewFontEntryPos] = EOS;
                       strncat(chFontTabEntry, (PSZ)pBuffer, i + 2) ;
                    }
                    else
                    {
                       if ( (int)strlen((PSZ)(&pBuffer[sNewFontEntryPos])) >= (i - sNewFontEntryPos + 2))
                       {
                         sOldBlockBytesLeft = 0;

                         memcpy((void *)chFontTabEntry, (void *)&pBuffer[sNewFontEntryPos], i - sNewFontEntryPos + 2);
                         chFontTabEntry[i - sNewFontEntryPos + 2] = EOS;
                       }
                       else
                       {
                         // two possibilities here:
                         // this is the first and only font defined in the whole fonttbl
                         // defined without enclosing braces {}
                         // -or-
                         // forward block change

                         if (pBuffer[sNewFontEntryPos] == '{')
                         {
                           // forward block change has happened
                           // (pBuffer doesn't hold font definition completely)

                           // store offset for the next loop entry

                           sOldBlockBytesLeft = (SHORT)(i - sNewFontEntryPos + 2 - strlen((PSZ)(&pBuffer[sNewFontEntryPos])));
                         }
                         else
                         {
                           // first and only font
                           // pBuffer[sNewFontEntryPos] should be here '\\'

                           sOldBlockBytesLeft = 0;
                         }

                         memcpy((void *)chFontTabEntry, (void *)&pBuffer[sNewFontEntryPos], strlen((PSZ)(&pBuffer[sNewFontEntryPos])));
                         chFontTabEntry[strlen((PSZ)(&pBuffer[sNewFontEntryPos]))] = EOS;
                       }
                    }
                 }
              }

              //
              // write back (asynchronous to block read)
              //

              for (k = 0; k < (int)strlen(chFontTabEntry) && usRC == NO_ERROR; k++ )
              {
                  usRC = RTFWriteBuffered( pParseData, chFontTabEntry[k], TRUE );
              } /* endfor */


              // and again...

              fFontTagFound = FALSE;
              fFontFamFound = FALSE;

          }
        } /* endfor */
      }
      else
      {
        // directly write data to output file
        USHORT usBytesWritten;
        usRC = UtlWrite( pParseData->hOutFile, pBuffer,
                         usBytesRead, &usBytesWritten, TRUE );
        fOK = (usRC == NO_ERROR);
      } /* endif */

      fOK = (usRC == NO_ERROR);
    } /* endif */

    fFirstDataBlock = FALSE;      // changed 300501 bt

  } /* endwhile */


  // add additional font to font tables (the closing curly brace of
  // the font table has not been written yet...)
  if ( fOK && (sObjType == FONTTBL_RTFTAG) && (pParseData->chAddFont[0] != EOS) )
  {
    PBYTE pbTemp;
    PSZ pszPlaceHolder;

    // write additional font data up to place holder
    pszPlaceHolder = strstr( pParseData->chAddFont, PLACEHOLDER );
    if ( pszPlaceHolder != NULL )
    {
      *pszPlaceHolder = EOS;
    } /* endif */
    pbTemp = (PBYTE)pParseData->chAddFont;
    while ( !usRC && (*pbTemp != EOS) )
    {
      usRC = RTFWriteBuffered( pParseData, *pbTemp, TRUE );
      pbTemp++;
    } /* endwhile */

    // write font number
    if ( !usRC && (pszPlaceHolder != NULL) )
    {
      CHAR szFontNo[10];
      sprintf( szFontNo, "f%d", sLastFont + 1);
      pbTemp = (PBYTE)szFontNo;
      while ( !usRC && (*pbTemp != EOS) )
      {
        usRC = RTFWriteBuffered( pParseData, *pbTemp, TRUE );
        pbTemp++;
      } /* endwhile */
      pParseData->sAddFontNo = sLastFont + 1;
    } /* endif */

    // write remaining data of font definition
    if ( !usRC && (pszPlaceHolder != NULL) )
    {
      pbTemp = (PBYTE)(pszPlaceHolder + strlen(PLACEHOLDER));
      while ( !usRC && (*pbTemp != EOS) )
      {
        usRC = RTFWriteBuffered( pParseData, *pbTemp, TRUE );
        pbTemp++;
      } /* endwhile */
    } /* endif */
  } /* endif */

  // write CRLF to output file
  if ( sObjType != FONTTBL_RTFTAG )
  {
    USHORT usBytesWritten;
    usRC = UtlWrite( pParseData->hOutFile, "\r\n", 2,
                     &usBytesWritten, TRUE );
    pParseData->usLinePos = 0;
  } /* endif */

  // restore old input buffer pointer in case of errors
  if ( !fOK )
  {
    pParseData->pInBuf = pbOldInBufPtr;
    pParseData->usBytesInBuffer = usOldBytesInBuffer;
    *pbCurrent = bOldCurrent;
  } /* endif */

  // cleanup
  if ( pBuffer != NULL ) UtlAlloc( (PVOID *)&pBuffer, 0L, 0L, NOMSG );

  LOGCLOSE();

  return( usRC );
} /* end of RTFRefToData */


/**********************************************************************/
/* Table used by function RTFGetSettings to check the settings file.  */
/* The table has to be terminated by an empty entry. The values in    */
/* the value list are seperated by a comma. The function uses the     */
/* index of the specified value in the value list as new setting for  */
/* a specific keyword (e.g. "NO,YES", "NO" sets 0, "YES" sets 1).     */
/**********************************************************************/
typedef struct _SETTINGKEYWORD
{
  SHORT       sID;                     // symbolic identifier for keyword
  CHAR        szKey[20];               // keyword of switch
  SHORT       sLen;                    // length of keyword
  SHORT       sValueType;              // type of value
  CHAR        szValues[80];            // list of possible values
} SETTINGKEYWORD, *PSETTINGKEYWORD;

enum
{
  BIDI_ID,
  ADDHEADER_ID,
  ADDFONT_ID,
  RTLCHARS_ID,
  RTLTAGS_ID,
  LTRTAGS_ID,
  PARTAGS_ID,
  DBCSBLANKFORLF_ID,
  CODEPAGE_ID,
  CHANGEFONT_ID,
  DEFCHGFONT_ID,
  BIDIOFF_ID,
  BIDION_ID,
  REMOVE_ID,
  CHANGE_ID
} KEYWORDIDS;

// types for values
enum
{
  CODERANGE_VALUE,                     // a range of code points
  FROMLIST_VALUE,                      // a value from the szValues list
  STRING_VALUE,                        // a string enclosed in double quotes
  NUM_VALUE                            // a numeric value
} VALUETYPES;

SETTINGKEYWORD SettingsKeywords[] =
{
  { BIDI_ID,           "BIDI=",            5, FROMLIST_VALUE, "NO,YES" },
  { ADDFONT_ID,        "ADDFONT=",         8, STRING_VALUE,   "" },
  { ADDHEADER_ID,      "ADDHEADER=",      10, STRING_VALUE,   "" },
  { RTLCHARS_ID,       "RTLCHARS=",        9, CODERANGE_VALUE,"" },
  { RTLTAGS_ID,        "RTLTAGS=",         8, STRING_VALUE,   "" },
  { LTRTAGS_ID,        "LTRTAGS=",         8, STRING_VALUE,   "" },
  { PARTAGS_ID,        "PARTAGS=",         8, STRING_VALUE,   "" },
  { DBCSBLANKFORLF_ID, "DBCSBLANKFORLF=", 15, FROMLIST_VALUE, "NO,YES" },
  { CODEPAGE_ID,       "CODEPAGE=",        9, NUM_VALUE,      "" },
  { CHANGEFONT_ID,     "CHANGEFONT=",     11, STRING_VALUE,   "" },
  { DEFCHGFONT_ID,     "DEFCHGFONT=",     11, STRING_VALUE,   "" },
  { BIDIOFF_ID,        "BIDIOFF=",         8, STRING_VALUE,   "" },
  { BIDION_ID,         "BIDION=",          7, STRING_VALUE,   "" },
  { REMOVE_ID,         "REMOVE=",          7, STRING_VALUE,   "" },
  { CHANGE_ID,         "CHANGETAG=",      10, STRING_VALUE,   "" },
  { 0,                 "",                 0, 0,              "" }
};

/**********************************************************************/
/*  RTFGetSettings                                                    */
/*                                                                    */
/* Get any settings file (name of settings file is name of tag table  */
/* with an extension of .CHR) and set the flags in the passed         */
/* flags structure                                                    */
/**********************************************************************/
USHORT RTFGetSettings
(
  PSZ         pszTagTable,             // name of tag table
  PSZ         pszTargetLang,           // name of target language
  PPARSEDATA  pParseData               // parser data structure
)
{
  // private data area
  typedef struct _GETSETTINGSDATA
  {
    CHAR chInBuf[8096];                // input buffer
    CHAR szLine[1024];                 // buffer for current line
    CHAR szSettingsFile[MAX_EQF_PATH]; // path name of settings file
    CHAR szValue[4096];                // buffer for string values
    CHAR achCodes[256];                // buffer for code range values
  } GETSETTINGSDATA, *PGETSETTINGSDATA;

  PGETSETTINGSDATA pData = NULL;       // pointer to private data
  HFILE hfSettings = NULLHANDLE;       // file handle of settings file
  BOOL        fOK = TRUE;              // internal O.K. flag
  LONG        lBytesToRead = 0L;       // number of bytes to read from file
  USHORT      usBytesInBuffer = 0;     // number of bytes in buffer
  BOOL        fActiveSection = FALSE;  // no active language section yet
  PSZ         pszBegConCatStr = NULL;  // line position of concatenated string
  BOOL        fStringConCat = FALSE;   // used to recognize string concatenation
  USHORT      usValue = 0;             // buffer for numeric values
  PSETTINGKEYWORD pKeyWord = NULL;     // ptr for keyword processing
  LONG        lRemoveTagsLen = 0;      // current length of remove tags list
  PSZ         pszRemoveTagsPos;        // current position within remove tag list
  PSZ_W       pszRemoveTagsPosW;       // current position within remove tag list (UTF16)
  LONG        lChangeFromTagsLen = 0;  // current length of changefrom tags list
  PSZ         pszChangeFromTagsPos;    // current position changefrom tag list
  PSZ_W       pszChangeFromTagsPosW;   // current position changefrom tag list (UTF16)
  LONG        lChangeToTagsLen = 0;    // current length of changeto tags list
  PSZ         pszChangeToTagsPos;      // current position changeto tag list
  PSZ_W       pszChangeToTagsPosW;     // current position changeto tag list (UTF16)

  // reset values
  pParseData->fBidi = FALSE;

  // set pointers for tag lists
  pszRemoveTagsPos      = pParseData->chRemoveTags;
  pszRemoveTagsPosW     = pParseData->chRemoveTagsW;
  pszChangeFromTagsPos  = pParseData->chChangeFromTags;
  pszChangeFromTagsPosW = pParseData->chChangeFromTagsW;
  pszChangeToTagsPos    = pParseData->chChangeToTags;
  pszChangeToTagsPosW   = pParseData->chChangeToTagsW;

  // allocate our private data structure
  fOK = UtlAlloc( (PVOID *)&pData, 0L, sizeof(GETSETTINGSDATA), NOMSG );

  // setup name of settings file and open settings file
  if ( fOK )
  {
    USHORT usActionTaken;              // action performed by file open

    UtlMakeEQFPath( pData->szSettingsFile, NULC, TABLE_PATH, NULL );
    GetOTMTablePath( pData->szSettingsFile, pData->szLine ) ;
    strcpy( pData->szSettingsFile, pData->szLine ) ;
    Utlstrccpy( pData->szSettingsFile + strlen(pData->szSettingsFile),
                pszTagTable, DOT );
    strcat( pData->szSettingsFile, EXT_OF_FORMATSETTINGS );
    fOK = UtlOpen( pData->szSettingsFile, &hfSettings, &usActionTaken, 0L,
             FILE_NORMAL, FILE_OPEN,
             OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
             0L, FALSE ) == NO_ERROR;
  } /* endif */

  // get size of input file
  if ( fOK )
  {
    FILESTATUS  stStatus;              // File status information

    fOK = UtlQFileInfo( hfSettings,  1, (PBYTE)&stStatus,
                        sizeof(FILESTATUS),  FALSE ) == NO_ERROR;
    lBytesToRead = stStatus.cbFile;
  } /* endif */

  // read and process until done
  while( fOK && ((lBytesToRead != 0L)|| (usBytesInBuffer != 0)) )
  {
    // fill input buffer
    if ( lBytesToRead && (usBytesInBuffer < sizeof(pData->chInBuf)) )
    {
      USHORT usBytesRead;              // number of bytes read from file
      USHORT usBytesToRead;

      usBytesToRead = (USHORT)min( ((LONG)sizeof(pData->chInBuf)-(LONG)usBytesInBuffer),
                                   lBytesToRead );
      fOK = UtlRead( hfSettings, pData->chInBuf + usBytesInBuffer,
                     usBytesToRead,
                     &usBytesRead, FALSE ) == NO_ERROR;
      usBytesInBuffer = usBytesInBuffer + usBytesRead;
      lBytesToRead -= (LONG)usBytesRead;
    } /* endif */

    // extract next line from input buffer
    if ( fOK )
    {
      USHORT usPos = 0;

      // copy bytes to line buffer until EOL detected
      while ( (usPos < usBytesInBuffer) &&
              (pData->chInBuf[usPos] != CR) &&
              (pData->chInBuf[usPos] != LF ) )
      {
        pData->szLine[usPos] = pData->chInBuf[usPos];
        usPos++;
      } /* endwhile */

      // terminate line buffer and skip EOL characters
      pData->szLine[usPos] = EOS;
      while ( (usPos < usBytesInBuffer) &&
              ( (pData->chInBuf[usPos] == CR) ||
                (pData->chInBuf[usPos] == LF ) ) )
      {
        usPos++;
      } /* endwhile */

      // adjust data in input buffer
      memmove( pData->chInBuf, pData->chInBuf + usPos, usBytesInBuffer - usPos );
      usBytesInBuffer = usBytesInBuffer - usPos;
    } /* endif */

    // process line
    if ( fOK )
    {
      switch ( pData->szLine[0] )
      {
        case '*':
          // ignore comments
          break;

        case '[':
          // check start of language specific section
          {
            PSZ pszEndDelim;           // ptr to language end delimiter

            fActiveSection = FALSE;    // reset any active language section

            pszEndDelim = strchr( pData->szLine, ']' );
            if ( pszEndDelim != NULL )
            {
              *pszEndDelim = EOS;
              if ( (_stricmp( pData->szLine + 1, pszTargetLang ) == 0 ) ||
                   (_stricmp( pData->szLine + 1, "all" ) == 0 ) )
              {
                fActiveSection = TRUE;
              } /* endif */
            } /* endif */
          }
          break;

        default:
          // check for keywords if in active language section
          if ( fActiveSection )
          {
            // if there's string concatenation, keep the current keyword

            if (!fStringConCat)
            {
               pKeyWord = SettingsKeywords;

               while ( (pKeyWord->szKey[0] != EOS) &&
                       (_strnicmp( pKeyWord->szKey, pData->szLine, pKeyWord->sLen ) != 0) )
               {
                  pKeyWord++;
               } /* endwhile */
            }
            else
            {
               // first char must be empty

               if (isspace(pData->szLine[0]))
               {
                  pszBegConCatStr = strchr( pData->szLine, DOUBLEQUOTE );
               }
            }

            // check supplied value if keyword found and do any settings
            if ( pKeyWord->szKey[0] != EOS)
            {
              PSZ pszTemp;
              PSZ pszSpecifiedValue;
              PSZ pszValue;
              SHORT sValueIndex = 0;
              BOOL fDone = FALSE;

              if (fStringConCat)
              {
                 pszSpecifiedValue = pszBegConCatStr ;
              }
              else
              {
                 pszSpecifiedValue = pData->szLine + pKeyWord->sLen;
              }

              // preprocess keyword value
              switch ( pKeyWord->sValueType )
              {
                case CODERANGE_VALUE :
                 // setup character recognition array
                 {
                   BOOL fNoError = TRUE;
                   PSZ  pszTemp = pszSpecifiedValue;

                   memset( pData->achCodes, 0, sizeof(pData->achCodes) );
                   while ( *pszTemp == BLANK ) pszTemp++;
                   while ( fNoError && *pszTemp )
                   {
                     if ( (pszTemp[0] == '0') &&
                          (pszTemp[1] == 'x') &&
                          isxdigit(pszTemp[2]) &&
                          isxdigit(pszTemp[3]) )
                     {
                       // get hex value and set flag
                       BYTE bHex1 = (BYTE) HEXTONUM( toupper(pszTemp[2]) );
                       bHex1 = bHex1 << 4;
                       bHex1 |= HEXTONUM( toupper(pszTemp[3]) );
                       pData->achCodes[bHex1] = 1;

                       // skip any whitespace
                       pszTemp += 4;
                       while ( *pszTemp == BLANK ) pszTemp++;

                       // check for range character
                       if ( *pszTemp == '-' )
                       {
                         // skip character and following whitespace
                         pszTemp++;
                         while ( *pszTemp == BLANK ) pszTemp++;

                         // get second hex character and set range
                         if ( (pszTemp[0] == '0') &&
                              (pszTemp[1] == 'x') &&
                              isxdigit(pszTemp[2]) &&
                              isxdigit(pszTemp[3]) )
                         {
                           // get second hex value
                           BYTE bHex2 = (BYTE) HEXTONUM( toupper(pszTemp[2]) );
                           bHex2 = bHex2 << 4;
                           bHex2 |= HEXTONUM( toupper(pszTemp[3]) );
                           pszTemp += 4;

                           // swap if not in correct order
                           if ( bHex2 < bHex1 )
                           {
                             BYTE bTemp = bHex1;
                             bHex1 = bHex2;
                             bHex2 = bTemp;
                           } /* endif */

                           // set flags in character array
                           memset( pData->achCodes + bHex1, 1, bHex2 - bHex1 + 1 );
                         }
                         else
                         {
                           fNoError = FALSE; // second hex char corrupt
                         } /* endif */

                         // skip trailing whitespace
                         while ( *pszTemp == BLANK ) pszTemp++;
                       } /* endif */

                       // skip sepearator and trailing whitespace
                       if ( *pszTemp == ',' ) pszTemp++;
                       while ( *pszTemp == BLANK ) pszTemp++;
                     }
                     else
                     {
                       fNoError = FALSE;
                     } /* endif */

                     while ( *pszTemp == BLANK ) pszTemp++;
                   } /* endwhile */
                   fDone = fNoError;
                 }
                 break;

                case STRING_VALUE :
                 // extract string enclosed in double-quotes
                 {
                   PSZ pszStart = strchr( pszSpecifiedValue, DOUBLEQUOTE );
                   if ( pszStart != NULL )
                   {
                     PSZ pszEnd = strchr( pszStart + 1, DOUBLEQUOTE );
                     if ( pszEnd != NULL )
                     {
                       *pszEnd = EOS;

                       if (fStringConCat)
                          strcat( pData->szValue, pszStart + 1 );
                       else
                          strcpy( pData->szValue, pszStart + 1 );

                       if (strchr(pszEnd + 1, '\\'))
                       {
                          fStringConCat = TRUE;
                          fDone = FALSE;
                       }
                       else
                       {
                          fStringConCat = FALSE;
                          fDone = TRUE;
                       }
                     } /* endif */
                   } /* endif */
                 }
                 break;

                case FROMLIST_VALUE :
                  {
                     // truncate value at first non-alphanumeric character
                     pszTemp = pszSpecifiedValue;
                     while ( isalnum(*pszTemp) ) pszTemp++;
                     *pszTemp = EOS;

                     // compare specified value with possible values
                     pszValue = pKeyWord->szValues;

                     while ( (*pszValue != EOS) && !fDone )
                     {
                       CHAR chTemp;    // buffer for value end character

                       // isolate current value of possible value list
                       pszTemp = pszValue;
                       while ( (*pszTemp != ',') && (*pszTemp != EOS) ) pszTemp++;
                       chTemp = *pszTemp;
                       *pszTemp = EOS;

                       // compare against specified value
                       if ( _stricmp( pszValue, pszSpecifiedValue ) == 0 )
                       {
                         fDone = TRUE;
                       } /* endif */

                       // continue with next value (if any)
                       *pszTemp = chTemp;
                       if ( !fDone && (chTemp != EOS) )
                       {
                         pszValue = pszTemp + 1;
                         sValueIndex += 1;
                       }
                       else
                       {
                         pszValue = pszTemp;
                       } /* endif */
                     } /* endwhile */
                  }
                  break;

                case NUM_VALUE :
                 // get numeric value
                 {
                   PSZ  pszTemp = pszSpecifiedValue;

                   usValue = 0;
                   while ( *pszTemp == BLANK ) pszTemp++;
                   while ( isdigit(*pszTemp) )
                   {
                     usValue = (usValue * 10) + (*pszTemp - '0');
                     pszTemp++;
                   } /* endwhile */
                   fDone = (usValue != 0);
                 }
                 break;
              } /* endswitch */

              // set flags or values if valid
              if ( fDone )
              {
                switch ( pKeyWord->sID )
                {
                 case BIDI_ID :
                  pParseData->fBidi = sValueIndex;
                  break;
                 case DBCSBLANKFORLF_ID :
                  pParseData->fDBCSBlankForLF = sValueIndex;
                  break;
                 case RTLCHARS_ID :
                  memcpy( pParseData->achIsBidiChar, pData->achCodes,
                          sizeof(pParseData->achIsBidiChar) );
                  break;
                 case ADDHEADER_ID :
                  strcpy( pParseData->chAddHeader, pData->szValue );
                  MultiByteToWideChar( pParseData->ulTgtOemCP, 0L, pData->szValue, -1,
                                       pParseData->chAddHeaderW,
                                       sizeof(pParseData->chAddHeaderW) / sizeof(CHAR_W) );
                  break;
                 case ADDFONT_ID :
                  strcpy( pParseData->chAddFont, pData->szValue );
                  MultiByteToWideChar( pParseData->ulTgtOemCP, 0L, pData->szValue, -1,
                                       pParseData->chAddFontW,
                                       sizeof(pParseData->chAddFontW) / sizeof(CHAR_W) );
                  break;
                 case RTLTAGS_ID :
                  strcpy( pParseData->chRTLTags, pData->szValue );
                  MultiByteToWideChar( pParseData->ulTgtOemCP, 0L, pData->szValue, -1,
                                       pParseData->chRTLTagsW,
                                       sizeof(pParseData->chRTLTagsW) / sizeof(CHAR_W) );
                  break;
                 case LTRTAGS_ID :
                  strcpy( pParseData->chLTRTags, pData->szValue );
                  MultiByteToWideChar( pParseData->ulTgtOemCP, 0L, pData->szValue, -1,
                                       pParseData->chLTRTagsW,
                                       sizeof(pParseData->chLTRTagsW) / sizeof(CHAR_W) );
                  break;
                 case PARTAGS_ID :
                  strcpy( pParseData->chParTags, pData->szValue );
                  MultiByteToWideChar( pParseData->ulTgtOemCP, 0L, pData->szValue, -1,
                                       pParseData->chParTagsW,
                                       sizeof(pParseData->chParTagsW) / sizeof(CHAR_W) );
                  break;
                 case CODEPAGE_ID :
                  pParseData->usCodePage = usValue;
                  break;
                 case CHANGEFONT_ID :
                  ParseChangeFontSection ( pData->szValue, pParseData );
                  break;
                 case DEFCHGFONT_ID :
                  ParseDefChgFontSection ( pData->szValue, pParseData );
                  break;
                 case BIDION_ID :
                  strcpy( pParseData->chBIDIONTags, pData->szValue );
                  MultiByteToWideChar( pParseData->ulTgtOemCP, 0L, pData->szValue, -1,
                                       pParseData->chBIDIONTagsW,
                                       sizeof(pParseData->chBIDIONTagsW) / sizeof(CHAR_W) );
                  break;
                 case BIDIOFF_ID :
                  strcpy( pParseData->chBIDIOFFTags, pData->szValue );
                  MultiByteToWideChar( pParseData->ulTgtOemCP, 0L, pData->szValue, -1,
                                       pParseData->chBIDIOFFTagsW,
                                       sizeof(pParseData->chBIDIOFFTagsW) / sizeof(CHAR_W) );
                  break;
                 case REMOVE_ID :
                   {
                     // add tags to remove tag list
                     LONG lAddLen = strlen( pData->szValue ) + 1;
                     if ( (lRemoveTagsLen + lAddLen ) < (sizeof(pParseData->chRemoveTags) - 1 ) )
                     {
                       strcpy( pszRemoveTagsPos, pData->szValue );
                       MultiByteToWideChar( pParseData->ulTgtOemCP, 0L, pData->szValue, -1,
                                            pszRemoveTagsPosW,
                                            (sizeof(pParseData->chRemoveTagsW) / sizeof(CHAR_W)) - lRemoveTagsLen );
                       pszRemoveTagsPos += lAddLen;
                       lRemoveTagsLen += lAddLen;
                       pszRemoveTagsPosW += UTF16strlenCHAR(pszRemoveTagsPosW) + 1;
                     } /* endif */
                   }
                  break;
                 case CHANGE_ID :
                   {
                     PSZ pszChangeToTag;
                     PSZ pszTagDelim;
                     CHAR chTemp = EOS;
                     LONG lChangeFromLen, lChangeToLen;

                     // find changefrom/changeto tag delimiter
                     pszTagDelim = strchr( pData->szValue, '=' );
                     if ( pszTagDelim != NULL )
                     {
                       chTemp = *pszTagDelim;
                       *pszTagDelim = EOS;
                       pszChangeToTag = pszTagDelim + 1;
                     }
                     else
                     {
                       pszChangeToTag = "";
                     } /* endif */

                     // add tags to changefrom tag list and changeto tag list
                     lChangeFromLen = strlen(pData->szValue) + 1;
                     lChangeToLen = strlen(pszChangeToTag) +1 ;

                     if ( ((lChangeFromTagsLen + lChangeFromLen) < (sizeof(pParseData->chChangeFromTags) - 1 )) &&
                          ((lChangeToTagsLen + lChangeToLen ) < (sizeof(pParseData->chChangeToTags) - 1 )) )
                     {
                       strcpy( pszChangeFromTagsPos, pData->szValue );
                       MultiByteToWideChar( pParseData->ulTgtOemCP, 0L, pData->szValue, -1,
                                            pszChangeFromTagsPosW,
                                            (sizeof(pParseData->chChangeFromTagsW) / sizeof(CHAR_W)) - lChangeFromTagsLen );
                       pszChangeFromTagsPos += lChangeFromLen;
                       lChangeFromTagsLen += lChangeFromLen;
                       pszChangeFromTagsPosW += UTF16strlenCHAR(pszChangeFromTagsPosW) + 1;

                       strcpy( pszChangeToTagsPos, pszChangeToTag );
                       MultiByteToWideChar( pParseData->ulTgtOemCP, 0L, pszChangeToTag, -1,
                                            pszChangeToTagsPosW,
                                            (sizeof(pParseData->chChangeToTagsW) / sizeof(CHAR_W)) - lChangeToTagsLen );
                       pszChangeToTagsPos += lChangeToLen;
                       lChangeToTagsLen += lChangeToLen;
                       pszChangeToTagsPosW += UTF16strlenCHAR(pszChangeToTagsPosW) + 1;
                     } /* endif */

                     if ( pszTagDelim != NULL )
                     {
                       *pszTagDelim = chTemp;
                     } /* endif */
                   }
                  break;
                 default:
                  break;
                } /* endswitch */
              } /* endif */
           }
          } /* endif */
          break;
      } /* endswitch */
    } /* endif */
  } /* endwhile */

  // terminate remove tag lists
  *pszRemoveTagsPos = 0;
  *pszRemoveTagsPosW = 0;

  // cleanup
  if ( hfSettings ) UtlClose( hfSettings, FALSE );
  if ( pData )      UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );

  // return to caller
  return( NO_ERROR );
} /* end of function RTFGetSettings */

/**********************************************************************/
/*  ParseChangeFontSection                                            */
/*  (logically part of RTFGetSettings)                                */
/* Parses the CHANGEFONT section, stores the entries in the           */
/* CHANGEFONTTAB struct.                                              */
/* Note: Additional memory is allocated and must be freed             */
/**********************************************************************/
BOOL ParseChangeFontSection(PSZ pszInputFileChangeFontSection, PPARSEDATA pParseData)
{
    PSZ pszChangeFontSection = NULL;
    PSZ pszCFSection = NULL;
    PSZ pszEndOfEntry = NULL;
    PSZ pszTemp = NULL;

    BOOL fHasEqual;

    BOOL fOK = TRUE;

    int i;

    // allocate for reference from CHANGEFONTTAB

    pParseData->pChangeFontData = NULL;

    if (pszInputFileChangeFontSection)
    {
        // allocate our private data structure
        fOK = UtlAlloc( (PVOID *)&pParseData->pChangeFontData, 0L, (LONG)strlen(pszInputFileChangeFontSection) + 1L, NOMSG );
    }

    if (fOK)
    {
        strcpy((char *) pParseData->pChangeFontData, pszInputFileChangeFontSection) ;
        pszChangeFontSection = (char *) pParseData->pChangeFontData ;
        pszCFSection = (char *) pParseData->pChangeFontData ;
    }

    // scan thru section

    fHasEqual = FALSE;
    pszEndOfEntry = strchr(pszCFSection, ';');

    i = 0;

    while (pszEndOfEntry && fOK && *pszCFSection && i < MAX_CHFTTAB_ENTRIES)
    {
        switch (*pszCFSection++)
        {
        case '[' :
            pParseData->ChangeFontTab[i].pszFontName = pszCFSection;
            pszTemp = strchr(pszCFSection, ']');

            if (!pszTemp || pszTemp > pszEndOfEntry)
                fOK = FALSE;     // missing ']'
            else
                pParseData->ChangeFontTab[i].usFontNameLen = (USHORT)(pszTemp - pszCFSection);

            break;
        case ']' :
            break;
        case '=' :
            fHasEqual = TRUE ;
            break;
        case '{' :
            pParseData->ChangeFontTab[i].pszSubstFontRTFSpec = pszCFSection;
            pParseData->ChangeFontTab[i].usSubstFontRTFSpecLen = (USHORT)((pszEndOfEntry + 1) - pszCFSection);

            break;
        case ';' :
            if (!fHasEqual)
                fOK = FALSE;

            fHasEqual = FALSE;

            pszEndOfEntry = strchr(pszCFSection + 1, ';');

            if (fOK)
               i ++ ;

            break;
        default:
            // overread spaces and newlines and anything else
            break;
        }
    }

    if (fOK)
    {
        pParseData->usChangeFontTabEntries = (USHORT)i;
    }
    else
    {
        pParseData->usChangeFontTabEntries = 0;
    }

    return(fOK);
} /* end of function ParseChangeFontSection */

/**********************************************************************/
/*  ParseDefChgFontSection                                            */
/*  (logically part of RTFGetSettings)                                */
/* Parses the DEFCHGFONT section, stores the entries in the           */
/* DEFCHGFONTTAB struct.                                              */
/* Note: Additional memory is allocated and must be freed             */
/**********************************************************************/
BOOL ParseDefChgFontSection(PSZ pszInputFileDefChgFontSection, PPARSEDATA pParseData)
{
    PSZ pszDefChgFontSection = NULL;
    PSZ pszDCFSection = NULL;
    PSZ pszEndOfEntry = NULL;
    PSZ pszTemp = NULL;

    BOOL fHasEqual;

    BOOL fOK = TRUE;

    int i, j;

    // allocate for reference from DEFCHGFONTTAB

    pParseData->pDefChgFontData = NULL;

    if (pszInputFileDefChgFontSection)
    {
        // allocate our private data structure
        fOK = UtlAlloc( (PVOID *)&pParseData->pDefChgFontData, 0L, (LONG)strlen(pszInputFileDefChgFontSection) + 1L, NOMSG );
    }

    if (fOK)
    {
        strcpy((char *) pParseData->pDefChgFontData, pszInputFileDefChgFontSection) ;
        pszDefChgFontSection = (char *) pParseData->pDefChgFontData ;
        pszDCFSection = (char *) pParseData->pDefChgFontData ;
    }

    // scan thru section

    fHasEqual = FALSE;
    pszEndOfEntry = strchr(pszDCFSection, ';');

    i = 0;

    while (pszEndOfEntry && fOK && *pszDCFSection && i < MAX_DEFCHFT_ENTRIES)
    {
        switch (*pszDCFSection++)
        {
        case '[' :
            pszTemp = strchr(pszDCFSection, ']');

            if (!pszTemp || pszTemp > pszEndOfEntry)
            {
                fOK = FALSE;     // missing ']'
            }
            else
            {
               //
               // set the right font family tag
               //

               pParseData->DefChgFontTab[i].FontFamilyTag = RTF_FF_NULL;

               for (j = 0; DefChgFntTag2Txt[j].FontFamilyTag != RTF_FF_NULL; j++)
               {
                  if (!memcmp((void *)pszDCFSection, (void *)DefChgFntTag2Txt[j].szFontFamily, pszTemp - pszDCFSection))
                  {
                     pParseData->DefChgFontTab[i].FontFamilyTag = DefChgFntTag2Txt[j].FontFamilyTag;
                     break;
                  }
               }
            }
            break;
        case ']' :
            break;
        case '=' :
            fHasEqual = TRUE ;
            break;
        case '{' :
            pParseData->DefChgFontTab[i].pszSubstFontRTFSpec = pszDCFSection;
            pParseData->DefChgFontTab[i].usSubstFontRTFSpecLen = (USHORT)((pszEndOfEntry + 1) - pszDCFSection);

            break;
        case ';' :
            if (!fHasEqual)
                fOK = FALSE;

            fHasEqual = FALSE;

            pszEndOfEntry = strchr(pszDCFSection + 1, ';');

            if (fOK)
               i ++ ;

            break;
        default:
            // overread spaces and newlines and anything else
            break;
        }
    }

    if (fOK)
    {
        pParseData->usDefChgFontTabEntries = (USHORT)i;
    }
    else
    {
        pParseData->usDefChgFontTabEntries = 0;
    }

    return(fOK);
} /* end of function ParseDefChgFontSection */

#define EQFSHOW_FNAME      "EQFSHOW.RTF"

/**********************************************************************/
/* Dummy tag for export                                               */
/**********************************************************************/
TATAG TADummyTag [] = { { ":qfa n=%u.", ":eqfa." },     // attribute
                        { ":qff n=%u.", ":eqff." },     // to be translated
                        { ":qfn n=%u.", ":eqfn." },     // no operation
                        { ":qfx n=%u s=%i.", ":eqfx." },// translated
                        { ":qfc n=%u.", ":eqfc." },     // current element
                        { ":qfj.",      ":eqfj." },     // joined segments
                        { ":qfs.",      ":eqfs." },     // splitted segments
                        { ":none.", "" },
                        { ":qfmark.", "" },
                        { "", "" } };



//------------------------------------------------------------------------------
//  EQFSHOW           - show document in WYSIWYG
//------------------------------------------------------------------------------
//  Description:
//     show the translated file WYSIWYG
//------------------------------------------------------------------------------
//    Flow:    get all segments and display it in wysiwyg
//             free allocated buffers
//------------------------------------------------------------------------------
//  Arguments:
//   LONG     lInfo,
//   HWND     handle
//------------------------------------------------------------------------------
//  Returns:
//   EQF_BOOL         TRUE           - success
//                    FALSE          - Dos error code
//------------------------------------------------------------------------------
//  Prereqs:
//------------------------------------------------------------------------------
//  SideEffects:
//   The show translation window is displayed in WYSIWYG
//------------------------------------------------------------------------------
#ifdef _WINDOWS

__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFSHOW( LONG lInfo, HWND hwndParent )
{
  USHORT usRC = 0;
  PTBDOCUMENT pDoc = (PTBDOCUMENT) lInfo;

  CHAR   chSystemPath[MAX_EQF_PATH];
  CHAR   chTempName[MAX_EQF_PATH];
  CHAR   chFileName[MAX_EQF_PATH];
  CHAR   chOutPut[MAX_EQF_PATH];
  PSZ    pFolder;
  PSZ    pFileName;
  BOOL   fChanged;


  strcpy( chTempName, pDoc->szDocName );
  pFileName = UtlSplitFnameFromPath( chTempName );
  UtlSplitFnameFromPath( chTempName );
  pFolder = UtlSplitFnameFromPath( chTempName );


  /******************************************************************/
  /* create temp file names -- we have to use two different output  */
  /* files, because otherwise our RTF unsegment will fail...        */
  /******************************************************************/
  UtlMakeEQFPath( chSystemPath, chTempName[0], DIRTARGETDOC_PATH, pFolder );
  sprintf( chFileName, "%s\\%s", chSystemPath, pFileName );
  sprintf( chOutPut, "%s\\%s", chSystemPath, EQFSHOW_FNAME );

  /********************************************************************/
  /* save original target document - but keep change flags ..         */
  /********************************************************************/
  if ( pDoc->flags.changed ||
       ((pDoc->docType == STARGET_DOC ) &&
        (pDoc->EQFBFlags.workchng || pDoc->fFuzzyCopied ||
            (pDoc->pTBSeg && pDoc->pTBSeg->SegFlags.UnTrans) ) ) )
  {
    fChanged = pDoc->flags.changed;
    usRC = EQFBFileWrite( pDoc->szDocName, pDoc );
    pDoc->flags.changed = (USHORT)fChanged;
  } /* endif */

  if ( !usRC )
  {
    /**********************************************************/
    /* unsegment and export the document                      */
    /**********************************************************/
    BOOL fGoOn = TRUE;
    usRC = (USHORT)!EQFUnSeg( pDoc->szDocName,
                      chFileName,
                      TADummyTag, &fGoOn,
                      pFolder, 0 );
    if ( !usRC )
    {
      /****************************************************************/
      /* rename to file with default extension (.RTF) to allow for    */
      /* use of HTML control                                          */
      /****************************************************************/
      usRC = UtlCopy( chFileName, chOutPut, 1, 0L, FALSE );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* force display in HTML control -- but have file mode switched to  */
  /* R/O, otherwise user is able to modify text                       */
  /********************************************************************/
  if ( !usRC )
  {
    UtlSetFileMode(chOutPut, FILE_READONLY, 0L, FALSE);
    WinSendMsg( GETPARENT(hwndParent), WM_EQF_SHOWHTML, "Translation Preview", chOutPut );
  } /* endif */

  /********************************************************************/
  /* free allocated resources ...                                     */
  /********************************************************************/
  UtlDelete( chFileName, 0L, FALSE );
  UtlDelete( chOutPut, 0L, FALSE );

  return (usRC == 0);
}

#endif

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     RTFParseSkipAnyWhiteSpace                                |
//+----------------------------------------------------------------------------+
//|Description:       Copys any whitespace to current segment                  |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|  PPARSEDATA  pParseData   points to parser data structure                  |
//|  PBYTE       pbCurrent    ptr to buffer for current character              |
//|  BOOL        fTranslSegm  current segement state                           |
//|  CHARTYPE    CharType     type to bne used for current character           |
//+----------------------------------------------------------------------------+
//|Returns:      USHORT       return code of called functions                  |
//+----------------------------------------------------------------------------+
USHORT RTFParseSkipAnyWhiteSpace
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PBYTE       pbCurrent,               // ptr to buffer for current character
  BOOL        fTranslSegm,             // current segement state
  CHARTYPE    CharType                 // type to bne used for current character
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  *pbCurrent  = ParseNextChar( pParseData, &usRC );
  while ( (usRC == NO_ERROR) &&
          ( (*pbCurrent == SPACE) || (*pbCurrent == LF) || (*pbCurrent == CR) ) )
  {
    usRC = AddToSegment( pParseData, *pbCurrent, fTranslSegm, CharType );
    if ( usRC == NO_ERROR )
    {
      *pbCurrent = ParseNextChar( pParseData, &usRC );
    } /* endif */
  } /* endwhile */
  return( usRC );
} /* end of function RTFParseSkipAnyWhiteSpace */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     RTFParseMacroParm                                        |
//+----------------------------------------------------------------------------+
//|Description:       Process a single macro paramter during presegmentation   |
//|                   and handle any macros used as macro parameter            |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|      PPARSEDATA  pParseData   points to parser data structure              |
//|      PBYTE       pbCurrent    ptr to buffer for current character          |
//|      BOOL        fTranslSegm  current segement state                       |
//|      CHARTYPE    CharType     type to bne used for current character       |
//|      PHELPMACRO  pMacro       ptr to macro definition data                 |
//|      SHORT       sParmNum     number of current macro parameter            |
//+----------------------------------------------------------------------------+
//|Returns:                                                                    |
//|      USHORT      return code of called functions                           |
//+----------------------------------------------------------------------------+
USHORT RTFParseMacroParm
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PBYTE       pbCurrent,               // ptr to buffer for current character
  BOOL        fProtected,              // current protection state
  BOOL        fTranslSegm,             // current segement state
  CHARTYPE    CharType,                // type to bne used for current character
  PHELPMACRO  pMacro,                  // ptr to macro definition data
  SHORT       sParmNum                 // number of current macro parameter
)
{
  USHORT usRC = RTFParseSkipAnyWhiteSpace( pParseData, pbCurrent,
                                           fTranslSegm, CH_TAG );

  CharType;

  // handle any macro in parameter
  if ( isalpha( *pbCurrent ) )
  {
    // remember current input position and data
    PBYTE pbTempPtr = pParseData->pInBuf;
    BYTE  bCurrChar = *pbCurrent;

    // resolve any macro used as parameter
    usRC = RTFParseMacro( pParseData, pbCurrent, fProtected,
                          fTranslSegm, *pbCurrent );

    // continue to next character if macro has been processed
    if ( usRC == NO_ERROR )
    {
      if ( (pbTempPtr != pParseData->pInBuf) ||
           (bCurrChar != *pbCurrent) )
      {
        usRC = RTFParseSkipAnyWhiteSpace( pParseData, pbCurrent,
                                          fTranslSegm, CH_TAG );
      } /* endif */
    } /* endif */
  } /* endif */

  while ( (usRC == NO_ERROR) &&
          (*pbCurrent != COMMA) &&
          (*pbCurrent != END_PARMS) )
  {
    switch ( *pbCurrent )
    {
      case SEMICOLON :
        // add semicolon and delimiting whitespace as not-translatable data
        do
        {
          usRC = AddToSegment( pParseData, *pbCurrent, fTranslSegm, CH_TAG );
          if ( !usRC )
          {
            *pbCurrent = ParseNextChar( pParseData, &usRC );
          } /* endif */
        } while ( (usRC == NO_ERROR) && (*pbCurrent == SPACE) ); /* enddo */
        break;

      case START_CTRLWORD :
        // Check for hexadecimal encoded characters or tags
        *pbCurrent = ParseNextChar( pParseData, &usRC );
        if ( *pbCurrent == '\'' )
        {
          usRC = RTFHandleHexChar( pParseData, fTranslSegm, FALSE,
                                   *pbCurrent,
                                   FALSE );
        }
        else
        {
          UndoChar( pParseData, *pbCurrent );
          *pbCurrent = START_CTRLWORD;

          // add control word to segment as non-translatable data
          do
          {
            usRC = AddToSegment( pParseData, *pbCurrent, fTranslSegm, CH_TAG );
            if ( !usRC )
            {
              *pbCurrent = ParseNextChar( pParseData, &usRC );
            } /* endif */
          } while ( (usRC == NO_ERROR) && isalpha(*pbCurrent) ); /* enddo */

          if ( (*pbCurrent == MINUS ) || isdigit(*pbCurrent) )
          {
            do
            {
              usRC = AddToSegment( pParseData, *pbCurrent, fTranslSegm,
                                   CH_TAG );
              if ( !usRC )
              {
                *pbCurrent = ParseNextChar( pParseData, &usRC );
              } /* endif */
            } while ( (usRC == NO_ERROR) && isdigit(*pbCurrent) ); /* enddo */
          } /* endif */

          if ( (usRC == NO_ERROR) && (*pbCurrent == SPACE) )
          {
            usRC = AddToSegment( pParseData, *pbCurrent, fTranslSegm, CH_TAG );
            if ( usRC == NO_ERROR )
            {
              *pbCurrent = ParseNextChar( pParseData, &usRC );
            } /* endif */
          } /* endif */
        } /* endif */
        break;

      default :
        // handle parameter data
        if ( pMacro->fExtraSeg )
        {
          // force a new segment for translatable data
          usRC = RTFWriteSegment( pParseData );
        } /* endif */

        if ( usRC == NO_ERROR )
        {
          if ( *pbCurrent == '\"' )
          {
            // handle SAS-style parameter (enclosed in double quotes)
            usRC = AddToSegment( pParseData, *pbCurrent, fTranslSegm, CH_TAG );
            if ( usRC == NO_ERROR ) *pbCurrent = ParseNextChar( pParseData, &usRC );
            do
            {
              if ( *pbCurrent == SEMICOLON )
              {
                if ( (usRC == NO_ERROR) && pMacro->fExtraSeg )
                {
                  // write segment containing parameter data
                  usRC = RTFWriteSegment( pParseData );
                } /* endif */

                usRC = AddToSegment( pParseData, *pbCurrent,
                                     fTranslSegm, CH_TAG );


                if ( (usRC == NO_ERROR) && pMacro->fExtraSeg )
                {
                  // write segment containing parameter delimiter
                  usRC = RTFWriteSegment( pParseData );
                } /* endif */

              }
              else if ( (*pbCurrent == START_CTRLWORD) &&
                   (pParseData->pInBuf[0] == '\'') )
              {
                *pbCurrent = ParseNextChar( pParseData, &usRC );
                if ( (sParmNum < 5) &&  pMacro->fTransParm[sParmNum] )
                {
                  usRC = RTFHandleHexChar( pParseData, fTranslSegm, TRUE,
                                           *pbCurrent,
                                           FALSE );
                 }
                 else
                 {
                  usRC = RTFHandleHexChar( pParseData, fTranslSegm, FALSE,
                                           *pbCurrent,
                                           FALSE );
                 } /* endif */
              }
              else
              {
                if ( (sParmNum < 5) &&  pMacro->fTransParm[sParmNum] )
                {
                  usRC = RTFNormalChar( pParseData, &*pbCurrent, FALSE,
                                        TRUE );
                }
                else
                {
                  usRC = RTFNormalChar( pParseData, &*pbCurrent, TRUE,
                                        fTranslSegm );
                } /* endif */
              } /* endif */
              if ( !usRC )
              {
                *pbCurrent = ParseNextChar( pParseData, &usRC );
              } /* endif */
            }  while ( (usRC == NO_ERROR)           &&
                       (*pbCurrent != '\"')            &&
                       !((*pbCurrent == START_CTRLWORD) &&
                         (pParseData->pInBuf[0] != '\'')) );  /* enddo */
            if ( (usRC == NO_ERROR) && pMacro->fExtraSeg )
            {
              // write segment containing parameter data
              usRC = RTFWriteSegment( pParseData );
            } /* endif */
            if ( (usRC == NO_ERROR) && (*pbCurrent == '\"') )
            {
              usRC = AddToSegment( pParseData, *pbCurrent, fTranslSegm, CH_TAG );
              *pbCurrent = ParseNextChar( pParseData, &usRC );
            } /* endif */
          }
          else
          {
            do
            {
              if ( (*pbCurrent == START_CTRLWORD) &&
                   (pParseData->pInBuf[0] == '\'') )
              {
                *pbCurrent = ParseNextChar( pParseData, &usRC );
                if ( (sParmNum < 5) &&  pMacro->fTransParm[sParmNum] )
                {
                  usRC = RTFHandleHexChar( pParseData, fTranslSegm, TRUE,
                                           *pbCurrent,
                                           FALSE );
                 }
                 else
                 {
                  usRC = RTFHandleHexChar( pParseData, fTranslSegm, FALSE,
                                           *pbCurrent,
                                           FALSE );
                 } /* endif */
               }
               else
               {
                if ( (sParmNum < 5) &&  pMacro->fTransParm[sParmNum] )
                {
                  usRC = RTFNormalChar( pParseData, &*pbCurrent, FALSE,
                                        TRUE );
                }
                else
                {
                  // handle non-translatable macro parameter (be sure to avoid split of
                  // segment by specifying type of currently active segment)
                  BOOL fNewTranslSeg = (pParseData->SegType == TRANSL_SEGMENT);
                  usRC = RTFNormalChar( pParseData, &*pbCurrent, TRUE,
                                        fNewTranslSeg );
                } /* endif */
              } /* endif */
              if ( !usRC )
              {
                *pbCurrent = ParseNextChar( pParseData, &usRC );
              } /* endif */
            }  while ( (usRC == NO_ERROR)           &&
                      (*pbCurrent != SEMICOLON)      &&
                      (*pbCurrent != COMMA)          &&
                      !((*pbCurrent == START_CTRLWORD) &&
                        (pParseData->pInBuf[0] != '\'')) &&
                      (*pbCurrent != END_PARMS) ); /* enddo */
          } /* endif */
        } /* endif */

        if ( (usRC == NO_ERROR) && pMacro->fExtraSeg )
        {
          // write segment containing parameter data
          usRC = RTFWriteSegment( pParseData );
        } /* endif */
        break;
    } /* endswitch */
  } /* endwhile */
  return( usRC );
} /* end of function RTFParseMacroParm */

// looks for the next non-neutral character in a segmented document
// but stops at next non-translatable segment
CHAR_W RTFPreUnsegNextNonNeutral
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PSZ_W       pszCurSegPos,            // current position in current segment
  PTBDOCUMENT pTBDoc,                  // pointer to segmented target document
  ULONG       *pulSegNum,              // pointer to current segment #
  PCHAR       pConvTable               // ptr to code conversion table
)
{
  PTBSEGMENT  pSeg;
  CHAR_W      chNextNN = 0;            // function return code
  PSZ_W       pszCurPos;

  ULONG       ulSegNum;

  ulSegNum = *pulSegNum;
  pszCurPos = pszCurSegPos;

  while ( *pszCurPos && !chNextNN )
  {
    if ( RTFisAlpha(*pszCurPos) )
    {
      chNextNN = *pszCurPos;         // use alpha character as next-NN
    }
    else if ( *pszCurPos == START_CTRLWORD_W )
    {
      // skip any RTF tag
      pszCurPos = RTFPreUnsegSkipTag( pszCurPos, pParseData, NULL );
    }
    else
    {
      // check for BIDI characters
      if ( RTFIsBidiCharW( pParseData, *pszCurPos, pConvTable ) )
      {
        chNextNN = *pszCurPos;
      }
      else
      {
        pszCurPos++;                 // try next character

        // added bt 120101
        if ( (*pszCurPos == 0) && (pulSegNum != NULL) )
        {
          // get the next segment and look what's going on
          // get segment only if we are in segmentation
          // (ulSegNum != NULL)

          pSeg = EQFBGetSegW( pTBDoc, ++ulSegNum);

          if (pSeg && pSeg->qStatus == QF_XLATED && pSeg->pDataW)
          {
             // point to data of new segment
             pszCurPos = pSeg->pDataW;
          }
        }
      } /* endif */
    } /* endif */
  } /* endwhile */

  return( chNextNN );
} /* end of function RTFPreUnsegNextNonNeutral */

// skip RTF tags during pre-unsegmenation
PSZ_W RTFPreUnsegSkipTag
(
  PSZ_W       pszSource,
  PPARSEDATA  pParseData,
  RTFTAGS     *pTagId                 // ID of skipped tag
)
{
  PSZ_W       pszTagStart;

  pParseData->fSkipOldBidiTag = FALSE;
  if ( pTagId != NULL ) *pTagId = NOSPECIFIC_DEST;

  // skip RTF tags
  if ( *pszSource == START_CTRLWORD_W )
  {
    pszSource++;
    if ( *pszSource == BEGIN_GROUP_W )
    {
      /**********************************************************/
      /* Opening curly brace character: check if curly brace is */
      /* followed by a windows help tag                         */
      /**********************************************************/
      PHELPTAG  pTag;            // ptr to tag definition data
      PSZ_W pszCheck = pszSource + 1;// position to data following curly brace

      // ignore any whitespace
      while ( (*pszCheck == SPACE_W) ||
              (*pszCheck == LF_W) ||
              (*pszCheck == CR_W) )
      {
        pszCheck++;
      } /* endwhile */

      // check for Windows help tag
      pTag = HelpTag;
      while ( (pTag->Id != END_OF_TAGS) &&
              (_wcsnicmp( pTag->szTagW,
                          pszCheck,
                          pTag->sLength ) != 0 ) )
      {
        pTag++;
      } /* endwhile */

      // handle help tag
      switch ( pTag->Id )
      {
        case BML_TAG :
        case BMC_TAG :
        case BMR_TAG :
        case BUTTON_TAG :
          {
            // skip help tag name
            pszSource = pszCheck + pTag->sLength;

            // skip data up to closing \}
            while ( (pszSource[0] != 0) && (pszSource[1] != 0))
            {
              if ( (pszSource[0] == START_CTRLWORD_W) &&
                   (pszSource[1] == END_GROUP_W) )
              {
                pszSource += 2;
                break;
              }
              else
              {
                pszSource++;
              } /* endif */
            } /* endwhile */
          }
          break;

        default :
          // do nothing
          break;
      } /* endswitch */
    }
    else if ( RTFisAlpha( *pszSource ) || (*pszSource == COMMENT_TAG_W) )
    {
      // skip tag characters
      RTFTAGS TagId = NOSPECIFIC_DEST;

      pszTagStart = pszSource;

      if ( *pszSource == COMMENT_TAG_W )
      {
        pszSource++;
        if ( *pszSource == START_CTRLWORD_W )
        {
          pszSource++;
          while ( RTFisAlpha( *pszSource ) ) pszSource++;
        } /* endif */
      }
      else
      {
        while ( RTFisAlpha( *pszSource ) ) pszSource++;
      } /* endif */

      // get type of tag
      {
        PCTRLWORD pCtrlWord;
        CHAR_W    chTemp;

        // get number of RTF tags if not computed by a previous call
        if ( !usNumOfTags )
        {
          usNumOfTags = 0;                    // start with first tag
          while ( CtrlWords[usNumOfTags].szCtrlWord[0] )
          {
            usNumOfTags++;
          } /* endwhile */
        } /* endif */

        chTemp = *pszSource;
        *pszSource = 0;
        {
          CTRLWORD SearchedTag;
          if ( (pszTagStart[0] == L'*') && (pszTagStart[1] == '\\' ) )
          {
            UTF16strcpy( SearchedTag.szCtrlWordW, pszTagStart + 2 );
          }
          else
          {
            UTF16strcpy( SearchedTag.szCtrlWordW, pszTagStart );
          } /* endif */
          pCtrlWord = (PCTRLWORD) bsearch( &SearchedTag, CtrlWords, usNumOfTags,
                               sizeof(CTRLWORD), StringICompareW );
        }

        // Restore control word end delimiter                     */
        *pszSource = chTemp;

        if ( pCtrlWord )
        {
          TagId = pCtrlWord->rtfID;
          if ( pTagId != NULL ) *pTagId = TagId;
        } /* endif */
      }

      // skip any numeric value prefix
      if ( (*pszSource == L'-') || (*pszSource == L'+') )
      {
        pszSource++;
      } /* endif */

      // skip any numeric value
      while ( iswdigit( *pszSource ) ) pszSource++;

      // skip any tag delimiter
      if ( *pszSource == SPACE_W ) pszSource++;

      // special handling for our reference tags: skip data up to closing
      // curly brace
      if ( (TagId == PICT_RTFTAG)        ||
           (TagId == STYLESHEET_RTFTAG)  ||
           (TagId == COLORTBL_RTFTAG)    ||
           (TagId == INFO_RTFTAG)        ||
           (TagId == OBJDATA_RTFTAG)     ||
           (TagId == LISTTABLE_RTFTAG)   ||
           (TagId == LISTOVERRIDETABLE_RTFTAG) ||
           (TagId == LEVELNUMBERS_RTFTAG)||
           (TagId == LEVELTEXT_RTFTAG)   ||
           (TagId == COLORSCHEMEMAPPING_RTFTAG)   ||
           (TagId == DATASTORE_RTFTAG)   ||
           (TagId == THEMEDATA_RTFTAG)   ||
           (TagId == LISTNAME_RTFTAG)    ||
           (TagId == PNTXTA_RTFTAG)      ||   // added to leave as-is
           (TagId == PNTXTB_RTFTAG)      ||   // added to leave as-is
           (TagId == FONTTBL_RTFTAG) )
      {
        while ( *pszSource && (*pszSource != END_GROUP_W) && (*pszSource != BEGIN_GROUP_W) )
        {
          pszSource++;
        } /* endwhile */
      } /* endif */

      // remember that we have font tag \f<xyz>
      // this is important for our bidi processing -
      // we have to overwrite the current font if
      // bidi chars are following
      //
      // bt 15012001
      if ( TagId == F_RTFTAG )
      {
        pParseData->fIsFontTag = TRUE;
      }

      if ( TagId == V_RTFTAG )
      {
        // set fHiddenMode to prevent inserion of BIDI tags
        // within hidden text (which in help files contains links)
        pParseData->fHiddenMode = TRUE;
      } /* endif */

      if ( pParseData->fBidi &&
           ( TagId == PARD_RTFTAG  ||
             TagId == SECTD_RTFTAG ||
             TagId == PLAIN_RTFTAG ) )
      {
        // empty current record buffer

        memset(pParseData->pszBidiRecordCharStyle, 0, BIDIRECORDCHARSTYLE_LEN);
        pParseData->fHiddenMode = FALSE;
      }

      // record character style markers
      // because of the Help compiler
      // see comment in function RTFPreUnseg
      if (  pParseData->fBidi &&
         ( TagId == ULDB_RTFTAG ||
           TagId == ULD_RTFTAG  ||
           TagId == UL_RTFTAG   ||
           TagId == CF_RTFTAG   ||
           TagId == B_RTFTAG    ||
           TagId == I_RTFTAG) )
      {
        // recording

            USHORT usIndex = 0;

        if (pParseData->fBidiRecordCharStyle)
        {
           while ( iswalnum(pszTagStart[usIndex++]));
           wcsncat( pParseData->pszBidiRecordCharStyle, pszTagStart-1, usIndex);
        }
      }

      if ( pParseData->fBidi &&
             ( TagId == LTRCH_RTFTAG  ||
               TagId == RTLCH_RTFTAG  ||
               TagId == LTRPAR_RTFTAG ||
               TagId == RTLPAR_RTFTAG ||
               TagId == QL_RTFTAG) )     // GQ: Added \ql tag as candidate for removal
      {
        //
        // an encounter of \lrtch or \rtlch during
        // unsegmentation means that these
        // tags were in the translation memory -
        // they have to be deleted and to be reset
        //

        pParseData->fSkipOldBidiTag = TRUE;
      }

    }
    else if ( (*pszSource == L'\'') &&
              iswxdigit(pszSource[1]) &&
              iswxdigit(pszSource[2]) )
    {
      // skip hexadecimal encoded character
      pszSource += 3;
    } /* endif */
  } /* endif */
  return( pszSource );
} /* end of function RTFPreUnsegSkipTag */

// create a temporary name by replacing the file extension
USHORT RTFMakeTempName
(
  PSZ         pszTempName,             // points to buffer for temp name
  PSZ         pszSourceName            // base name
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PSZ         pName;                   // points to start of name

  strcpy( pszTempName, pszSourceName );
  pName = UtlGetFnameFromPath( pszTempName );
  while ( *pName && *pName != '.' )
  {
     pName ++;
  } /* endwhile */

  if ( !*pName )
  {
     *pName = '.';
  } /* endif */
  pName++;

  if ( strcmp( pName, "$$$" ) == 0 )
  {
    memset( pName, '@', 3 );
  }
  else
  {
    memset( pName, '$', 3 );
  } /* endif */
  *(pName+3) = EOS;                      // include end of string

  return( usRC );
} /* end of function RTFMakeTempName */

// Re-fill input buffer if half of the buffer is empty
USHORT RTFBufInpReFill
(
  HFILE       hInFile,                 // file handle of input file
  PBYTE       pInBufStart,             // ptr to start of input buffer
  PBYTE       *ppInBufCurrentPos,      // ptr to current position pointer
  PUSHORT     pusBytesInBuffer,        // ptr to number of bytes in input buffer
  PLONG       plBytesToRead            // number of bytes to read from input file
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  if ( *pusBytesInBuffer < (INBUF_SIZE / 2) )
  {
    USHORT usRead;                 // number of bytes actually read
    USHORT usBytesToRead;          // bytes to read from input file

    // copy data in buffer to begin of buffer
    if ( *pusBytesInBuffer != 0 )
    {
      memmove( pInBufStart, *ppInBufCurrentPos, *pusBytesInBuffer );
    } /* endif */
    *ppInBufCurrentPos = pInBufStart;
    pInBufStart[*pusBytesInBuffer] = EOS;

    if ( *plBytesToRead != 0L )
    {
      // fill second half of buffer
      usBytesToRead = (USHORT)INBUF_SIZE - 1 - *pusBytesInBuffer;
      usBytesToRead = (USHORT)min( (LONG)usBytesToRead, *plBytesToRead );
      usRC = UtlRead( hInFile, pInBufStart + *pusBytesInBuffer,
                      usBytesToRead, &usRead, TRUE );
      if ( !usRC )
      {
        *plBytesToRead     -= usRead;
        *pusBytesInBuffer  = *pusBytesInBuffer + usRead;
      } /* endif */
    } /* endif */
  } /* endif */
  return( usRC );
} /* end of function RTFBufInpReFill */


/**********************************************************************/
/* RTFSetLangCodePage                                                 */
/**********************************************************************/
BOOL RTFSetLangCodePage(char *pszLang, UINT *puiCodePage)
{
   LANGCPARRAY szLangCPArray[] =
   {
      { "Chinese(simpl.)", 936 }, // codepage corrected 16022001 bt
      { "Chinese(trad.)", 950 },
      { "Japanese", 932 },
      { "Korean", 949 },
    { "Thai", 874 },
      { "", 0 }
   };

   char szLocale[10];
   short sI;

   *puiCodePage = 0;

   if (!pszLang)
      return (FALSE);

   // set standard (1252) codepage for non AP languages

   if (!IsAPLang(pszLang))
   {
      *puiCodePage = 1252;
      return (BOOL)setlocale(LC_CTYPE, ".1252");
   }

   for (sI = 0; strlen(szLangCPArray[sI].szTmLang) > 0; sI ++)
   {
      if (!strcmp(szLangCPArray[sI].szTmLang, pszLang))
      {
         sprintf(szLocale, ".%d", szLangCPArray[sI].uiCodePage);
         if (setlocale(LC_CTYPE, szLocale))
         {
            return TRUE;
         }
     *puiCodePage = szLangCPArray[sI].uiCodePage; // changed 16022001 bt
      }
   }

   return (FALSE);
} /* end of RTFSetLangCodePage */

/************************************************************************/
/* IsCharTopicPrefix                              */
/*                                    */
/* Checks if char belongs to help topic prefixes              */
/************************************************************************/
BOOL IsCharTopicPrefix(char TestChar)
{
  char szTopicPrefixSet[] = "#$>*+KA!@";

  if (strchr(szTopicPrefixSet, (int) TestChar))
    return (TRUE);
  else
    return (FALSE);
}


//////////////////////////////////////////////////////////////////////////////
///
///  Unicode specific functions
//////////////////////////////////////////////////////////////////////////////


SHORT RTFGetNumW( PSZ_W *ppszNum );
void RTFSkipNextTagOrCharW( PSZ_W *ppszPos );



// load an already segmented file and convert coded unicode characters
// to unicode
BOOL RTFResolveUnicodeTags
(
  PSZ         pszSegFile,              // name of segment file
  PEQF_BOOL   pfKill                   // caller's kill process flag
)
{
  BOOL fOK = TRUE;
  BOOL fFileChanged = FALSE;           // TRUE = segmented file has been changed
  PTBDOCUMENT pTBDoc = NULL;           // ptr to loaded document
  PSZ_W  pszTempSeg = NULL;            // ptr to buffer for temporary segment data
  SHORT  sUCStack[40];                 // stack for current UC values
  SHORT  sStackPos = 0;
  static CHAR_W szTag[1000];           // buffer for tag names
  PPARSEDATA  pParseData = NULL;       // points to parser data structure

  fOK = UtlAlloc( (PVOID *) &pParseData, 0L, (LONG)sizeof(PARSEDATA), ERROR_STORAGE );

  // allocate segment buffer
  fOK = UtlAlloc( (PVOID *)&pszTempSeg, 0L,
                  (LONG)(MAX_SEGMENT_SIZE * sizeof(CHAR_W) * 3), TRUE );

  if ( fOK )
  {
    CHAR szLang[MAX_LANG_LENGTH];       // buffer for document target language
    USHORT usRC = 0;

    // get document object namee
    strcpy( (PSZ)pParseData->abOutBuf, pszSegFile );
    UtlSplitFnameFromPath( (PSZ)pParseData->abOutBuf );
    UtlSplitFnameFromPath( (PSZ)pParseData->abOutBuf );
    strcat( (PSZ)pParseData->abOutBuf, BACKSLASH_STR );
    strcat( (PSZ)pParseData->abOutBuf, UtlGetFnameFromPath( pszSegFile ) );

    szLang[0] = EOS;
    usRC = DocQueryInfo( (PSZ)pParseData->abOutBuf, // object name of document
                         NULL,                   // translation memory or NULL
                         NULL,                   // folder format or NULL
                         pParseData->szLanguage,  // source language or NULL
                         szLang,                  // target language or NULL
                         TRUE );                 // do-message-handling flag

    // get target language dependend settings
    RTFGetSettings( "OTMRTF", szLang, pParseData );
  } /* endif */

  // allocate TBDOC structure
  if ( fOK && !*pfKill )
  {
    fOK = UtlAlloc( (PVOID *)&pTBDoc, 0L, (LONG) sizeof( TBDOCUMENT ), TRUE );
  } /* endif */

  // load tag table
  if ( fOK && !*pfKill )
  {
    if ( (SHORT)TALoadTagTable( DEFAULT_QFTAG_TABLE,
                                (PLOADEDTABLE *) &pTBDoc->pQFTagTable,
                                TRUE, FALSE ) != 0 )
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // load the segmented file
  if ( fOK && !*pfKill)
  {
     RTFParseGetCP(pszSegFile, &pTBDoc->ulOemCodePage, NULL,
                           &pTBDoc->ulAnsiCodePage, NULL);

    fOK = ( EQFBFileRead( pszSegFile, pTBDoc ) == 0);
  } /* endif */

  // scan the file and change segments
  if ( fOK && !*pfKill )
  {
     ULONG ulSegNum = 1;
     SHORT sUC = 1;               // current \UC value

     PTBSEGMENT pSeg = EQFBGetSegW( pTBDoc, ulSegNum );

     while ( pSeg && fOK && !*pfKill )
     {
       BOOL fSegChanged = FALSE;

       // look for and handle unicode tags in segment data
       if ( fOK )
       {
         PSZ_W pszPos = pSeg->pDataW;
         PSZ_W pszOut = pszTempSeg;
         while ( *pszPos )
         {
           switch ( *pszPos )
           {
             case L'{':
               *pszOut++ = *pszPos++;
               if ( sStackPos < 99)
               {
                 sUCStack[sStackPos] = sUC;
                 sStackPos++;
               } /* endif */
               break;

             case L'}':
               *pszOut++ = *pszPos++;
               if ( sStackPos > 0)
               {
                 sStackPos--;
                 sUC = sUCStack[sStackPos];
               } /* endif */
               break;

             case L'\\':
               {
                 PSZ_W pszTemp = pszPos + 1;
                 if ( (*pszTemp == L'\\') || (*pszTemp == L'{') || (*pszTemp == L'}') )
                 {
                   // encoded character, copy to out segment
                   *pszOut++ = *pszPos++;
                   *pszOut++ = *pszPos++;
                 }
                 else if ( RTFisAlpha(*pszTemp) )
                 {
                   // get following tag
                   PSZ_W pszTag = szTag;
                   while ( RTFisAlpha(*pszTemp) )
                   {
                     *pszTag++ = *pszTemp++;
                   } /* endwhile */
                   *pszTag = EOS;

                   // check for and handle unicode tags
                   if ( wcscmp( szTag, L"uc" ) == 0 )
                   {
                     // set new UC value
                     sUC = RTFGetNumW( &pszTemp );

                     // copy tag to out segment
                     while ( pszPos < pszTemp ) *pszOut++ = *pszPos++;
                   }
                   else if ( wcscmp( szTag, L"u" ) == 0 )
                   {
                     // insert coded unicode character
                     SHORT sUnicode = RTFGetNumW( &pszTemp );
                     *pszOut++ = sUnicode;

                     // skip tag delimiter (if any)
                     if ( *pszTemp == L' ' ) pszTemp++;

                     // skip following sUC characters or tags
                     if ( sUC )
                     {
                       RTFSkipNextTagOrCharW( &pszTemp );
                     } /* endif */

                     // continue scan at current position
                     pszPos = pszTemp;

                     // set segment change flag
                     fSegChanged = TRUE;

                   }
                   else
                   {
                     // look for special tags representing characters
                     int i = 0;
                     BOOL fFound = FALSE;

                     while ( SpecialCharTags[i].szTag[0] && !fFound )
                     {
                       if ( wcscmp( szTag, SpecialCharTags[i].szTag ) == 0 )
                       {
                         fFound = TRUE;
                       }
                       else
                       {
                         i++;
                       } /* endif */
                     } /* endwhile */

                     if ( fFound )
                     {
                       // replace tag with its unicode character
                       *pszOut++ = SpecialCharTags[i].chValue;

                     // set segment change flag
                     fSegChanged = TRUE;

                     // skip tag delimiter (if any)
                     if ( *pszTemp == L' ' ) pszTemp++;

                     // continue scan at current position
                     pszPos = pszTemp;


                     }
                     else
                     {
                       // copy not processed tag to out segment
                       while ( pszPos < pszTemp ) *pszOut++ = *pszPos++;
                     } /* endif */
                   } /* endif */
                 }
                 else
                 {
                   // treat as normal text
                   *pszOut++ = *pszPos++;
                 } /* endif */
               }
               break;

             default:
               *pszOut++ = *pszPos++;
               break;
           } /* endswitch */
         } /* endwhile */
         *pszOut = 0;
       } /* endif */

       // second step: do tag cleanup in translatable segments
       // GQ: tag removal has been moved to segmentation step (ParsseRTF->WriteSegment)
       //if ( fOK && (pSeg->qStatus == QF_TOBE) )
       //{
       //  fSegChanged = RTFRemoveTagsW( ( fSegChanged ) ? pszTempSeg : pSeg->pDataW, pszTempSeg,
       //                                  pParseData->chRemoveTagsW );
       //} /* endif */

       // re-alloc segment data if segment data has grown
       if ( fOK && fSegChanged )
       {
         int iNewLen = UTF16strlenCHAR( pszTempSeg) + 1;
         if ( fOK && fSegChanged && (iNewLen > (int)pSeg->usLength) )
         {
           fOK = UtlAlloc( (PVOID *)&(pSeg->pDataW),
                           (LONG)(pSeg->usLength*sizeof(CHAR_W)),
                           (LONG)(iNewLen*sizeof(CHAR_W)), TRUE );
           if ( fOK )
           {
             pSeg->usLength = (USHORT)iNewLen;
           } /* endif */
         } /* endif */
       } /* endif */

       // copy segment data to segment buffer
       if ( fOK && fSegChanged )
       {
         UTF16strcpy( pSeg->pDataW, pszTempSeg );
         fFileChanged = TRUE;
       } /* endif */

       // Continue with next segment
       ulSegNum++;                         // point to next segment
       pSeg = EQFBGetSegW( pTBDoc, ulSegNum );
     } /* endwhile */
   } /* endif */

   // write the segmented file out (write file always to ensure that the
   // segmented files are stored in Unicode)
   if ( fOK && !*pfKill )
   {

     fOK = ( EQFBFileWrite( pszSegFile, pTBDoc ) == 0);
   } /* endif */

   // free allocated memory
   if ( pTBDoc )
   {
     TAFreeTagTable( (PLOADEDTABLE) pTBDoc->pQFTagTable );
     RTFFreeDoc( (PVOID *)&pTBDoc );
   } /* endif */

   if (  pszTempSeg ) UtlAlloc( (PVOID *)&pszTempSeg, 0L, 0L, NOMSG );
   if (  pParseData ) UtlAlloc( (PVOID *)&pParseData, 0L, 0L, NOMSG );

   return( fOK );
} /* end of function RTFResolveUnicodeTags */


// get follwing short number (if any)
SHORT RTFGetNumW( PSZ_W *ppszNum )
{
  PSZ_W pszEnd;
  CHAR_W chEndChar;

  SHORT sResult = 0;

  // find end of number
  pszEnd = *ppszNum;
  if ( *pszEnd == L'-' ) pszEnd++;
  while ( iswdigit( *pszEnd ) ) pszEnd++;

  // terminate number string
  chEndChar = *pszEnd;
  *pszEnd = 0;

  // convert number string
  sResult = (SHORT)_wtoi(*ppszNum);

  // restore end of number and set caller's pointer
  *pszEnd = chEndChar;
  *ppszNum = pszEnd;

  return( sResult );
} /* end of function RTFGetNumW */

// skip the next character or tag
void RTFSkipNextTagOrCharW( PSZ_W *ppszPos )
{
  PSZ_W pszPos = *ppszPos;

  if ( *pszPos == L'\\' )
  {
    pszPos++;
    if ( (*pszPos == L'\\') || (*pszPos == L'{') || (*pszPos == L'}') )
    {
      pszPos++;
    }
    else if ( *pszPos == '\'' )
    {
      // skip hexadecimal encoded character
      pszPos++;
      if ( iswxdigit( *pszPos ) ) pszPos++;
      if ( iswxdigit( *pszPos ) ) pszPos++;
    }
    else
    {
      // skip a tag
      while ( RTFisAlpha( *pszPos ) ) pszPos++;
      if ( *pszPos == '-' ) pszPos++;
      while ( iswdigit( *pszPos ) ) pszPos++;
      if ( *pszPos == ' ' ) pszPos++;
    } /* endif */
  }
  else
  {
    // skip a single character
    pszPos++;
  } /* endif */

  *ppszPos = pszPos;
} /* end of function RTFSkipNextTagOrCharW */

// check if a UTF16 character is a BIDI character
BOOL RTFIsBidiCharW
(
  PPARSEDATA  pParseData,              // points to parser data structure
  CHAR_W      chTestChar,              // character being tested
  PCHAR       pConvTable               // ptr to code conversion table
)
{
  BOOL fIsBidiChar = FALSE;
  UCHAR ucTest;

  // should be replaced by a more sophisticated test!!!
  //GQTODO!
  CHAR_W szTestBuffer[10];
  CHAR   szResult[10];

  // copy to test buffer and convert to ASCII ...
  szTestBuffer[0] = chTestChar;
  szTestBuffer[1] = 0;
  Unicode2ASCII( szTestBuffer, szResult, pParseData->ulTgtOemCP );

  ucTest = (UCHAR)szResult[0];
  if ( pConvTable ) ucTest = pConvTable[ucTest];
  fIsBidiChar = pParseData->achIsBidiChar[ucTest];

  return ( fIsBidiChar );
} /* end of function RTFIsBidiCharW */

//USHORT EQFGetCPOem()
//{
//  TCHAR        cp [6];
//  USHORT   usCP;

//// 720->864 is fix for Arabic
//// for Greek special setting, 869 must be used instead of 737

//  GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTCODEPAGE, cp, sizeof(cp));
//  usCP = (USHORT)_ttol (cp);
//  if (usCP == 720)
//  {
//     usCP = 864;
//  }
//  else if ((usCP == 737L) && (GetOEMCP() == 869) && (GetKBCodePage() == 869 ) )
//  {
//    usCP = 869;
//  }
//  return usCP;
//}

SHORT RTFAddNewSeg
(
  PTBDOCUMENT pDoc,                    // document structure
  PTBSEGMENT  pSeg,                    // segment structure
  PSZ_W       pszSegData,              // data of segment
  PULONG      pulSegOut                // current/new segment number
)
{
  SHORT sRc = 0;

  TBSEGMENT   ActSegment;

  memcpy( &ActSegment, pSeg, sizeof(TBSEGMENT) );
  ActSegment.usLength = (USHORT)UTF16strlenCHAR( pszSegData );
  ActSegment.SegFlags.Joined = 0;
  ActSegment.SegFlags.JoinStart = 0;

  sRc = (SHORT)!UtlAlloc( (PVOID *) &ActSegment.pDataW,
                           0L, (LONG)((ActSegment.usLength+1)*sizeof(CHAR_W)), ERROR_STORAGE );
  if ( !sRc )
  {
    UTF16strcpy( ActSegment.pDataW, pszSegData );
    ActSegment.ulSegNum = *pulSegOut++;
    sRc = EQFBAddSegW( pDoc, &ActSegment );
  } /* endif */

  return sRc;

} /* end of function RTFAddNewSeg */

// code borrowed form EQFUTILS.C as function had not been exported by EQFDLL.DLL
BOOL RTFIsDBCSChar( CHAR_W c, ULONG ulCP)
{
    CHAR_W chW[2];
    CHAR        ch[5];

    chW[0] = c; chW[1] = EOS;
    memset (ch, 0, sizeof(ch));

    return (Unicode2ASCIIBuf( chW, ch, 1, sizeof(ch), ulCP ) == 2 );
} /* endof function RTFIsDBCSChar */

// rtf does not compile with eqfparse.obj, so it is needed here too
VOID RTFParseGetCP
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
  BYTE   abTempBuf[MAX_LONGFILESPEC];
  BYTE   abOutBuf[MAX_LONGFILESPEC];
  CHAR   szFolder[MAX_LONGFILESPEC];

  USHORT usRC = NO_ERROR;

  strcpy( (PSZ)abOutBuf, pszInFile );
  pszTemp  = UtlGetFnameFromPath( (PSZ)abOutBuf );
  pszTemp--;
  *pszTemp = EOS;
  pszTemp  = UtlGetFnameFromPath( (PSZ)abOutBuf );
  pszTemp--;
  *pszTemp = EOS;
  Utlstrccpy( szFolder, UtlGetFnameFromPath( (PSZ)abOutBuf ), DOT );

  UtlMakeEQFPath( (PSZ)abTempBuf, *pszInFile, SYSTEM_PATH, NULL );
  strcat( (PSZ)abTempBuf, BACKSLASH_STR );
  strcat( (PSZ)abTempBuf, szFolder );
  strcat( (PSZ)abTempBuf, EXT_FOLDER_MAIN );
  strcat( (PSZ)abTempBuf, BACKSLASH_STR );
  strcat( (PSZ)abTempBuf, UtlGetFnameFromPath( pszInFile ) );

  usRC = DocQueryInfo( (PSZ)abTempBuf,    // object name of document
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

// skip any RTF inline tags in the give string
// the function returns a pointer to the first non-inline tag character
PSZ_W RTFSkipInlineTagsW
(
  PSZ_W       pszSource
)
{
  BOOL        fEndOfTags = FALSE;

  // skip RTF inline tagging
  do
  {
    PSZ_W pszTestStart = pszSource;
    if ( *pszSource == START_CTRLWORD_W )
    {
      // skip tag or hexdicmal encoded character
      fEndOfTags = !RTFSkipTagW( &pszSource );
    }
    else if ( *pszSource == BEGIN_GROUP_W )
    {
      pszSource++;
    }
    else if ( *pszSource == END_GROUP_W )
    {
      pszSource++;
    }
    else
    {
      fEndOfTags = TRUE;
    } /* endif */

    // if end of tags exceeded, set pointer to test start position
    if ( fEndOfTags )
    {
      pszSource = pszTestStart;
    } /* endif */
  } while ( !fEndOfTags );

  return( pszSource );
} /* end of function RTFSkipInlineTagsW */

// check for and skip a single tag or a hexadecimal encoded value
BOOL RTFSkipTag
(
  PSZ        *ppszSource
)
{
  PSZ  pszSource = *ppszSource;
  BOOL  fIsTag = TRUE;

  if ( *pszSource == START_CTRLWORD )
  {
    pszSource++;
    if ( isalpha( *pszSource ) || (*pszSource == COMMENT_TAG) )
    {
      if ( *pszSource == COMMENT_TAG )
      {
        pszSource++;
        if ( *pszSource == START_CTRLWORD )
        {
          pszSource++;
          while ( isalpha( *pszSource ) ) pszSource++;
        } /* endif */
      }
      else
      {
        while ( isalpha( *pszSource ) ) pszSource++;
      } /* endif */

      // skip any numeric value prefix
      if ( (*pszSource == '-') || (*pszSource == '+') )
      {
        pszSource++;
      } /* endif */

      // skip any numeric value
      while ( isdigit( *pszSource ) ) pszSource++;

      // skip any tag delimiter
      if ( *pszSource == SPACE ) pszSource++;
    }
    else if ( (*pszSource == '\'') &&
              isxdigit(pszSource[1]) &&
              isxdigit(pszSource[2]) )
    {
      // skip hexadecimal encoded character
      pszSource += 3;
    }
    else
    {
      fIsTag = FALSE;
    } /* endif */
  }
  else
  {
    fIsTag = FALSE;
  } /* endif */

  if ( fIsTag )
  {
    *ppszSource = pszSource;
  } /* endif */

  return( fIsTag );
} /* end of function RTFSkipTag */

BOOL RTFSkipTagW
(
  PSZ_W       *ppszSource
)
{
  PSZ_W pszSource = *ppszSource;
  BOOL  fIsTag = TRUE;

  if ( *pszSource == START_CTRLWORD_W )
  {
    pszSource++;
    if ( RTFisAlpha( *pszSource ) || (*pszSource == COMMENT_TAG_W) )
    {
      if ( *pszSource == COMMENT_TAG_W )
      {
        pszSource++;
        if ( *pszSource == START_CTRLWORD_W )
        {
          pszSource++;
          while ( RTFisAlpha( *pszSource ) ) pszSource++;
        } /* endif */
      }
      else
      {
        while ( RTFisAlpha( *pszSource ) ) pszSource++;
      } /* endif */

      // skip any numeric value prefix
      if ( (*pszSource == L'-') || (*pszSource == L'+') )
      {
        pszSource++;
      } /* endif */

      // skip any numeric value
      while ( iswdigit( *pszSource ) ) pszSource++;

      // skip any tag delimiter
      if ( *pszSource == SPACE_W ) pszSource++;
    }
    else if ( (*pszSource == L'\'') &&
              iswxdigit(pszSource[1]) &&
              iswxdigit(pszSource[2]) )
    {
      // skip hexadecimal encoded character
      pszSource += 3;
    }
    else
    {
      fIsTag = FALSE;
    } /* endif */
  }
  else
  {
    fIsTag = FALSE;
  } /* endif */

  if ( fIsTag )
  {
    *ppszSource = pszSource;
  } /* endif */

  return( fIsTag );
} /* end of function RTFSkipTagW */


// skip any RTF inline tags in the give string
// the function returns a pointer to the first non-inline tag character
PSZ RTFSkipInlineTags
(
  PSZ       pszSource
)
{
  BOOL        fEndOfTags = FALSE;

  // skip RTF inline tagging
  do
  {
    PSZ pszTestStart = pszSource;
    if ( *pszSource == START_CTRLWORD_W )
    {
      pszSource++;
      if ( RTFisAlpha( *pszSource ) || (*pszSource == COMMENT_TAG) )
      {
        if ( *pszSource == COMMENT_TAG )
        {
          pszSource++;
          if ( *pszSource == START_CTRLWORD )
          {
            pszSource++;
            while ( isalpha( *pszSource ) ) pszSource++;
          } /* endif */
        }
        else
        {
          while ( isalpha( *pszSource ) ) pszSource++;
        } /* endif */

        // skip any numeric value prefix
        if ( (*pszSource == '-') || (*pszSource == '+') )
        {
          pszSource++;
        } /* endif */

        // skip any numeric value
        while ( iswdigit( *pszSource ) ) pszSource++;

        // skip any tag delimiter
        if ( *pszSource == SPACE ) pszSource++;
      }
      else if ( (*pszSource == '\'') &&
                isxdigit(pszSource[1]) &&
                isxdigit(pszSource[2]) )
      {
        // skip hexadecimal encoded character
        pszSource += 3;
      }
      else
      {
        fEndOfTags = TRUE;
      } /* endif */
    }
    else if ( *pszSource == BEGIN_GROUP )
    {
      pszSource++;
    }
    else if ( *pszSource == END_GROUP )
    {
      pszSource++;
    }
    else
    {
      fEndOfTags = TRUE;
    } /* endif */

    // if end of tags exceeded, set pointer to test start position
    if ( fEndOfTags )
    {
      pszSource = pszTestStart;
    } /* endif */
  } while ( !fEndOfTags );

  return( pszSource );
} /* end of function RTFSkipInlineTags */

// RTFFindTags / RTFFindTagsW
//
// Find a tag or tag group within the supplied string from a list of tags/tag groups
//
// The list of tags/tag groups is a list of null-terminated strings. The end of the list
// is indicated by an empty string.
//
// The tags can contain the wildcard character '?' which stands for one or more numeric digits
//
BOOL RTFFindTags
(
  PSZ         pszString,               // points to string being searched for tags
  PSZ         pszSearchTags,           // list of tags
  int        *piLen,                   // when found: length of found tag group
  int        *piTagIndex,              // when found: relative index of found tag in tag list
  PSZ        *ppszPos                  // when found: position of tags within string
)
{
  PSZ        pszCurPos = pszString;    // current position within strin
  PSZ        pszTag;                   // currently tested tag
  int        iTagIndex = 0;            // index of currently tested tag
  BOOL       fFound = FALSE;           // TRUE = tag has been found
  int        iLen = 0;                 // length of matching string

  // loop through string checking first character of tag strings
  while ( !fFound && *pszCurPos )
  {
    // test current character against first character of tags
    pszTag = pszSearchTags;
    iTagIndex = 0;
    while ( !fFound && *pszTag )
    {
      if ( *pszTag == *pszCurPos )
      {
        fFound = RTFMatchTag( pszTag, pszCurPos, &iLen );
      } /* endif */

      // try next tag if no match
      if ( !fFound )
      {
        pszTag = pszTag + strlen( pszTag) + 1;
        iTagIndex++;
      } /* endif */
    } /*endwhile */

    // continue with next character if no match so far
    if ( !fFound )
    {
      pszCurPos++;
    } /* endif */
  } /*endwhile */

  // return found values if found
  if ( fFound )
  {
    *piLen = iLen;
    *piTagIndex = iTagIndex;
    *ppszPos = pszCurPos;
  } /* endif */

  return( fFound );
} /* end of function RTFFindTags */

BOOL RTFFindTagsW
(
  PSZ_W       pszString,               // points to string being searched for tags
  PSZ_W       pszSearchTags,           // list of tags
  int        *piLen,                   // when found: length of found tag group
  int        *piTagIndex,              // when found: relative index of found tag in tag list
  PSZ_W      *ppszPos                  // when found: position of tags within string
)
{
  PSZ_W      pszCurPos = pszString;    // current position within strin
  PSZ_W      pszTag;                   // currently tested tag
  int        iTagIndex = 0;            // index of currently tested tag
  BOOL       fFound = FALSE;           // TRUE = tag has been found
  int        iLen = 0;                 // length of matching string

  // loop through string checking first character of tag strings
  while ( !fFound && *pszCurPos )
  {
    // test current character against first character of tags
    pszTag = pszSearchTags;
    iTagIndex = 0;
    while ( !fFound && *pszTag )
    {
      if ( *pszTag == *pszCurPos )
      {
        fFound = RTFMatchTagW( pszTag, pszCurPos, &iLen );
      } /* endif */

      // try next tag if no match
      if ( !fFound )
      {
        pszTag = pszTag + UTF16strlenCHAR( pszTag) + 1;
        iTagIndex++;
      } /* endif */
    } /*endwhile */

    // continue with next character if no match so far
    if ( !fFound )
    {
      pszCurPos++;
    } /* endif */
  } /*endwhile */

  // return found values if found
  if ( fFound )
  {
    *piLen = iLen;
    *piTagIndex = iTagIndex;
    *ppszPos = pszCurPos;
  } /* endif */

  return( fFound );
} /* end of function RTFFindTagsW */



// RTFMatchTag / RTFMatchTagW
//
// Check if the given tag matches the supplied string.
//
// The tag can contain the wildcard character '?' which stands for one or more numeric digits
//
BOOL RTFMatchTag
(
  PSZ         pszTag,                  // tag being checked for
  PSZ         pszPos,                  // current string
  int        *piLen                    // length of matching tag (may be different from tag length)
)
{
  BOOL       fMatch = TRUE;            // TRUE = tag characters match string
  int        iLen = 0;                 // length of matching string

  while ( fMatch && *pszTag )
  {
    if ( *pszTag == '?' )
    {
      if ( isdigit( *pszPos ) )
      {
        // skip all digits in source
        while ( isdigit( *pszPos ) )
        {
          pszPos++;
          iLen++;
        } /*endwhile */

        // skip wildcard character
        pszTag++;
      }
      else
      {
        // strings do not match
        fMatch = FALSE;
      } /* endif */
    }
    else if ( toupper(*pszTag) == toupper(*pszPos) )
    {
      pszTag++;
      pszPos++;
      iLen++;
    }
    else if ( *pszPos == '\n' )
    {
      // ignore linefeeds
      pszPos++;
      iLen++;
    }
    else
    {
      fMatch = FALSE;
    } /* endif */
  } /*endwhile */

  if ( fMatch )
  {
    *piLen = iLen;
  } /* endif */

  return( fMatch );
} /* end of function RTFMatchTag */

BOOL RTFMatchTagW
(
  PSZ_W       pszTag,                  // tag being checked for
  PSZ_W       pszPos,                  // current string
  int        *piLen                    // length of matching tag (may be different from tag length)
)
{
  BOOL       fMatch = TRUE;            // TRUE = tag characters match string
  int        iLen = 0;                 // length of matching string

  while ( fMatch && *pszTag )
  {
    if ( *pszTag == L'?' )
    {
      if ( iswdigit( *pszPos ) )
      {
        // skip all digits in source
        while ( iswdigit( *pszPos ) )
        {
          pszPos++;
          iLen++;
        } /*endwhile */

        // skip wildcard character
        pszTag++;
      }
      else
      {
        // strings do not match
        fMatch = FALSE;
      } /* endif */
    }
    else if ( towupper(*pszTag) == towupper(*pszPos) )
    {
      pszTag++;
      pszPos++;
      iLen++;
    }
    else if ( *pszPos == L'\n' )
    {
      // ignore linefeeds
      pszPos++;
      iLen++;
    }
    else
    {
      fMatch = FALSE;
    } /* endif */
  } /*endwhile */

  if ( fMatch )
  {
    *piLen = iLen;
  } /* endif */

  return( fMatch );
} /* end of function RTFMatchTagW */

// remove superfluous curly braces from given string
void RTFRemoveSuperfluousBraces
(
  PSZ    pszString,
  PBYTE  pbAdjustBuf1,          // ptr to a buffer with data which needs adjusting
  PBYTE  pbAdjustBuf2           // ptr to a buffer with data which needs adjusting

)
{
  PSZ    pszOpenBrace = NULL;
  PSZ    pszPos = pszString;
  while ( *pszPos )
  {
    if ( *pszPos == '{' )
    {
      // rememember position of opening brace
      pszOpenBrace = pszPos;
      pszPos++;
    }
    else if ( *pszPos == '}' )
    {
      // closing curly brace, check if braces can be removed
      if ( pszOpenBrace )
      {
        // check if there is a tag right before the opening brace..
        PSZ pszTest = pszOpenBrace - 1; 
        BOOL fStillInTag = TRUE;
        while ( fStillInTag && (pszTest > pszString) )
        {
          if ( isalnum(*pszTest) || (*pszTest == '*') )
          {
            pszTest--;
          }
          else
          {
            fStillInTag = FALSE;
          } /* endif */
        } /* endwhile */           

        if ( (*pszTest != '\\') && (pszTest > pszString) )
        {
        // braces can be removed

        // remove opening brace (move one char more as in remaining string for EOS)
        int iLen = strlen( pszOpenBrace );
        memmove( pszOpenBrace, pszOpenBrace + 1, iLen );

        if ( pbAdjustBuf1 )
        {
          int iPos = pszOpenBrace - pszString;
          memmove( pbAdjustBuf1 + iPos, pbAdjustBuf1 + iPos + 1, iLen - 1 );
        } /* endif */

        if ( pbAdjustBuf2 )
        {
          int iPos = pszOpenBrace - pszString;
          memmove( pbAdjustBuf2 + iPos, pbAdjustBuf2 + iPos + 1, iLen - 1 );
        } /* endif */

        // adjust current pos
        pszPos--;

        // remove closing brace (move one char more as in remaining string for EOS)
        iLen = strlen( pszPos );
        memmove( pszPos, pszPos + 1, iLen );
        pszOpenBrace = NULL;

        if ( pbAdjustBuf1 )
        {
          int iPos = pszPos - pszString;
          memmove( pbAdjustBuf1 + iPos, pbAdjustBuf1 + iPos + 1, iLen - 1 );
        } /* endif */

        if ( pbAdjustBuf2 )
        {
          int iPos = pszPos - pszString;
          memmove( pbAdjustBuf2 + iPos, pbAdjustBuf2 + iPos + 1, iLen - 1 );
        } /* endif */

      }
      else
      {
        pszPos++;
      } /* endif */
      }
      else
      {
        pszPos++;
      } /* endif */
    }
    else if ( *pszPos == '\\' )
    {
      // maybe tags detected, ignore curly brace if not hex encoded character
      if ( pszPos[1] != '\'' )
      {
        pszOpenBrace = NULL;
      } /* endif */

      pszPos++;

      // check for following curly brace (which is to be treated as a character
      // if preceeded by a backslash)
      if ( (*pszPos == '{') || (*pszPos == '}') )
      {
        pszPos++;
      } /* endif */
    }
    else
    {
      // continue with next char
      pszPos++;
    } /* endif */
  } /*endwhile */
} /* end of function RTFRemoveSuperfluosBraces */

// remove superfluous curly braces from given string
void RTFRemoveSuperfluousBracesW
(
  PSZ_W  pszString
)
{
  PSZ_W  pszOpenBrace = NULL;
  PSZ_W  pszPos = pszString;
  while ( *pszPos )
  {
    if ( *pszPos == L'{' )
    {
      // rememember position of opening brace
      pszOpenBrace = pszPos;
      pszPos++;
    }
    else if ( *pszPos == L'}' )
    {
      // closing curly brace, check if braces can be removed
      if ( pszOpenBrace )
      {
        // check if there is a tag right before the opening brace..
        PSZ_W pszTest = pszOpenBrace - 1;
        BOOL fStillInTag = TRUE;
        while ( fStillInTag && (pszTest > pszString) )
        {
          fStillInTag = iswalnum(*pszTest) || (*pszTest == L'*');
          pszTest--;
        } /* endwhile */           

        if ( (*pszTest != L'\\') && (pszTest > pszString) )
        {
        // braces can be removed

        // remove opening brace (move one char more as in remaining string for EOS)
        memmove( pszOpenBrace, pszOpenBrace + 1, UTF16strlenBYTE( pszOpenBrace ) );

        // adjust current pos
        pszPos--;

        // remove closing brace (move one char more as in remaining string for EOS)
        memmove( pszPos, pszPos + 1, UTF16strlenBYTE( pszPos ) );
        pszOpenBrace = NULL;
      }
      else
      {
        pszPos++;
      } /* endif */
    }
      else
      {
        pszPos++;
      } /* endif */
    }
    else if ( *pszPos == L'\\' )
    {
      // maybe tags detected, ignore curly brace if not hex encoded character
      if ( pszPos[1] != L'\'' )
      {
        pszOpenBrace = NULL;
      } /* endif */

      pszPos++;

      // check for following curly brace (which is to be treated as a character
      // if preceeded by a backslash)
      if ( (*pszPos == L'{') || (*pszPos == L'}') )
      {
        pszPos++;
      } /* endif */
    }
    else
    {
      // continue with next char
      pszPos++;
    } /* endif */
  } /*endwhile */
} /* end of function RTFRemoveSuperfluosBracesW */

// adjust linefeeds in string; i.e. force a linefeed after 75 characters
void RTFAdjustLinefeed
(
  PSZ    pszString
)
{
  // first step: remove all linefeeds contained in string
  {
    PSZ    pszSource;
    PSZ    pszTarget;

    pszSource = pszTarget = pszString;
    while ( *pszSource )
    {
      if ( *pszSource != '\n' )
      {
        *pszTarget++ = *pszSource;
      } /* endif */
      pszSource++;
    } /*endwhile */
    *pszTarget = *pszSource;
  }

  // second step: insert linefeed in fixed intervals
  {
    PSZ  pszPos = pszString;
    PSZ  pszLFPos = NULL;
    int iColPos = 0;

    while ( *pszPos )
    {
      // check current character
      if ( *pszPos == '\\' )
      {
        // remember as possible linefeed position
        pszLFPos = pszPos;

        // skip any tag
        if ( RTFSkipTag( &pszPos ) )
        {
          // adjust current position within line
          iColPos += (pszPos - pszLFPos);
        }
        else
        {
          // no tag, check for other characters
          pszPos++;
          iColPos++;
          if ( (*pszPos == '\\') || (*pszPos == '}') || (*pszPos == '{') )
          {
            // skip encoded character
            iColPos++;
            pszPos++;
          } /* endif */
        } /* endif */
      }
      else if ( *pszPos == ' ' )
      {
        // remember as possible linefeed position
        pszLFPos = pszPos;
        iColPos++;
        pszPos++;
      }
      else
      {
        // continue with next character
        iColPos++;
        pszPos++;
      } /* endif */

      // insert linefeed if necessary and possible
      if ( (iColPos > 75) && (pszLFPos != NULL) )
      {
        memmove( pszLFPos + 1, pszLFPos, strlen( pszLFPos ) + 2 );
        *pszLFPos = '\n';
        iColPos = 0;
        pszLFPos = NULL;
        pszPos++;
      } /* endif */
    } /*endwhile */

    // re-add linefeed at end of segment
    if ( iColPos != 0 )
    {
      *pszPos++ = '\n';
      *pszPos = 0;
    } /* endif */
  }
} /* end of function RTFAdjustLinefeed */

// adjust linefeeds in string; i.e. force a linefeed after 75 characters
void RTFAdjustLinefeedW
(
  PSZ_W  pszString
)
{
  // first step: remove all linefeeds contained in string
  {
    PSZ_W  pszSource;
    PSZ_W  pszTarget;

    pszSource = pszTarget = pszString;
    while ( *pszSource )
    {
      if ( *pszSource != L'\n' )
      {
        *pszTarget++ = *pszSource;
      } /* endif */
      pszSource++;
    } /*endwhile */
    *pszTarget = *pszSource;
  }

  // second step: insert linefeed in fixed intervals
  {
    PSZ_W  pszPos = pszString;
    PSZ_W  pszLFPos = NULL;
    int iColPos = 0;

    while ( *pszPos )
    {
      // check current character
      if ( *pszPos == L'\\' )
      {
        // remember as possible linefeed position
        pszLFPos = pszPos;

        // skip any tag
        if ( RTFSkipTagW( &pszPos ) )
        {
          // adjust current position within line
          iColPos += (pszPos - pszLFPos);
        }
        else
        {
          // no tag, check for other characters
          pszPos++;
          iColPos++;
          if ( (*pszPos == L'\\') || (*pszPos == L'}') || (*pszPos == L'{') )
          {
            // skip encoded character
            iColPos++;
            pszPos++;
          } /* endif */
        } /* endif */
      }
      else if ( *pszPos == L' ' )
      {
        // remember as possible linefeed position
        pszLFPos = pszPos;
        iColPos++;
        pszPos++;
      }
      else
      {
        // continue with next character
        iColPos++;
        pszPos++;
      } /* endif */

      // insert linefeed if necessary and possible
      if ( (iColPos > 75) && (pszLFPos != NULL) )
      {
        memmove( pszLFPos + 1, pszLFPos, UTF16strlenBYTE( pszLFPos ) + 2 );
        *pszLFPos = L'\n';
        iColPos = 0;
        pszLFPos = NULL;
        pszPos++;
      } /* endif */
    } /*endwhile */

    // re-add linefeed at end of segment
    if ( iColPos != 0 )
    {
      *pszPos++ = L'\n';
      *pszPos = 0;
    } /* endif */
  }
} /* end of function RTFAdjustLinefeedW */

// check if there is a tag directly in front of the text pointed to by pszText
BOOL RTFTagIsPreceedingTextW
(
  PSZ_W   pszText,
  PSZ_W   pszBufferStart
)
{
  BOOL    fTagBeforeText = TRUE;
  PSZ_W   pszTest = pszText - 1;
  BOOL    fNumericValue = FALSE;
  BOOL    fNameChars = FALSE;

  // skip preceeding digits (which may be part of a tag
  while ( (pszTest > pszBufferStart) && iswdigit( *pszTest ) )
  {
    pszTest--;
    fNumericValue = TRUE;
  } /*endwhile */

  // skip sign of numerical value
  if ( (pszTest > pszBufferStart) && fNumericValue  &&
       ( (*pszTest == L'-') || (*pszTest == L'+') ) )
  {
    pszTest--;
  } /* endif */

  // skip characters of tag name
  while ( (pszTest > pszBufferStart) && RTFisAlpha( *pszTest ) )
  {
    pszTest--;
    fNameChars = TRUE;
  } /*endwhile */

  // handle tag starting with \*\ correctly
  if ( (pszTest > pszBufferStart) && fNameChars  &&
       (*pszTest == L'\\') && (pszTest[-1] == L'*') )
  {
    pszTest--;
    if ( pszTest > pszBufferStart) pszTest--;
  } /* endif */

  // if we are at backslash and tag name characters had been skipped
  // we are at the start of a tag
  if ( (*pszTest == L'\\') && fNameChars )
  {
    // last check: check that preceeding character is no backslash itself
    if ( pszTest > pszBufferStart )
    {
      pszTest--;
      fTagBeforeText = !(*pszTest == L'\\');
   }
    else
    {
      // assume tag
      fTagBeforeText = TRUE;
    } /* endif */
  }
  else
  {
    fTagBeforeText = FALSE;
  } /* endif */

  return( fTagBeforeText );
} /* end of function RTFTagIsPreceedingTextW */

// check if there is a tag directly in front of the text pointed to by pszText
BOOL RTFTagIsPreceedingText
(
  PSZ     pszText,
  PSZ     pszBufferStart
)
{
  BOOL    fTagBeforeText = TRUE;
  PSZ     pszTest = pszText - 1;
  BOOL    fNumericValue = FALSE;
  BOOL    fNameChars = FALSE;

  // skip preceeding digits (which may be part of a tag
  while ( (pszTest > pszBufferStart) && isdigit( *pszTest ) )
  {
    pszTest--;
    fNumericValue = TRUE;
  } /*endwhile */

  // skip sign of numerical value
  if ( (pszTest > pszBufferStart) && fNumericValue  &&
       ( (*pszTest == '-') || (*pszTest == '+') ) )
  {
    pszTest--;
  } /* endif */

  // skip characters of tag name
  while ( (pszTest > pszBufferStart) && isalpha( *pszTest ) )
  {
    pszTest--;
    fNameChars = TRUE;
  } /*endwhile */

  // handle tag starting with \*\ correctly
  if ( (pszTest > pszBufferStart) && fNameChars  &&
       (*pszTest == '\\') && (pszTest[-1] == '*') )
  {
    pszTest--;
    if ( pszTest > pszBufferStart) pszTest--;
  } /* endif */

  // if we are at backslash and tag name characters had been skipped
  // we are at the start of a tag
  if ( (*pszTest == '\\') && fNameChars )
  {
    // last check: check that preceeding character is no backslash itself
    if ( pszTest > pszBufferStart )
    {
      pszTest--;
      fTagBeforeText = !(*pszTest == '\\');
   }
    else
    {
      // assume tag
      fTagBeforeText = TRUE;
    } /* endif */
  }
  else
  {
    fTagBeforeText = FALSE;
  } /* endif */

  return( fTagBeforeText );
} /* end of function RTFTagIsPreceedingText */

// remove tags in supplied buffer, changed segment is copied to supplied buffer
BOOL RTFRemoveTags
(
  PSZ   pszInData,              // ptr to input data
  PSZ   pszOutData,             // ptr to output data
  PSZ   pszRemoveTags,          // tags to be removed
  PBYTE pbAdjustBuf1,           // ptr to a buffer with data which needs adjusting
  PBYTE pbAdjustBuf2            // ptr to a buffer with data which needs adjusting
)
{
  PSZ   pszSource = pszInData;
  PSZ   pszTarget = pszOutData; // ptr to target
  BOOL fFound = FALSE;          // TRUE = tags have been found
  int  iLen = 0;                // length of found tags
  int  iTagIndex = 0;           // index of found tags
  PSZ   pszPos = NULL;          // position of found tags
  BOOL fTagsRemoved = FALSE;    // TRUE = tags have been removed
  PBYTE pbAdj1Source, pbAdj1Target; // ptr for adjust1 buffer processing
  PBYTE pbAdj2Source, pbAdj2Target; // ptr for adjust2 buffer processing

  pbAdj1Source = pbAdj1Target = pbAdjustBuf1;
  pbAdj2Source = pbAdj2Target = pbAdjustBuf2;

  do
  {
    fFound = RTFFindTags( pszSource, pszRemoveTags, &iLen, &iTagIndex, &pszPos );
    if ( fFound )
    {
      BOOL fBlankFollowing, fTagPreceeding, fTagAtEnd;

      fTagAtEnd = RTFTagIsPreceedingText( pszPos + iLen, pszPos );
      fBlankFollowing = (pszPos[iLen] == ' ');

      // copy data up to tags to target area
      while ( pszSource < pszPos )
      {
        *pszTarget++ = *pszSource++;
        if ( pbAdjustBuf1 ) *pbAdj1Target++ = *pbAdj1Source++;
        if ( pbAdjustBuf2 ) *pbAdj2Target++ = *pbAdj2Source++;
      } /*endwhile */

      // check if there is another tag preceeding the tags being removed
      fTagPreceeding = RTFTagIsPreceedingText( pszTarget, pszOutData );

      // skip tags
      pszSource = pszPos + iLen;
      if ( pbAdjustBuf1 ) pbAdj1Source += iLen;
      if ( pbAdjustBuf2 ) pbAdj2Source += iLen;

      // special handling for blanks following removed tags:
      // if there is a tag at the end of the removed data and a blank is
      // following but the removed text is not directly preceeded by a tag
      // remove blank as well as it is the tag end delimter of the removed tags
      if ( !fTagPreceeding && fTagAtEnd && fBlankFollowing )
      {
        pszSource++;
        if ( pbAdjustBuf1 ) pbAdj1Source++;
        if ( pbAdjustBuf2 ) pbAdj2Source++;
      } /* endif */

      // remember that segment has been changed
      fTagsRemoved = TRUE;
    } /* endif */
  } while ( fFound );

  // copy rest of source to target
  while ( *pszSource )
  {
    *pszTarget++ = *pszSource++;
    if ( pbAdjustBuf1 )
    {
      *pbAdj1Target++ = *pbAdj1Source++;
    } /* endif */

    if ( pbAdjustBuf2 )
    {
      *pbAdj2Target++ = *pbAdj2Source++;
    } /* endif */
  } /*endwhile */
  *pszTarget = 0;
  if ( pbAdjustBuf1 ) *pbAdj1Target = 0;
  if ( pbAdjustBuf2 ) *pbAdj2Target = 0;

  // remove orphaned curly braces from segment if tags had been removed
  if ( fTagsRemoved )
  {
    RTFRemoveSuperfluousBraces( pszOutData, pbAdjustBuf1, pbAdjustBuf2 );
  } /*endif */

  // re-arrange linefeeds to avoid nearly empty lines

  // GQ: line-feed re-arrange diasbled, seems to be not necessary in RtfParse
  //if ( fTagsRemoved )
  //{
  //  RTFAdjustLinefeed( pszOutData );
  //} /* endif */

  return( fTagsRemoved );

} /* end of function RTFRemoveTags */

BOOL RTFRemoveTagsW
(
  PSZ_W pszInData,              // ptr to input data
  PSZ_W pszOutData,             // ptr to output data
  PSZ_W pszRemoveTags           // tags to be removed
)
{
  PSZ_W pszSource = pszInData;
  PSZ_W pszTarget = pszOutData; // ptr to target
  BOOL fFound = FALSE;          // TRUE = tags have been found
  int  iLen = 0;                // length of found tags
  int  iTagIndex = 0;           // index of found tags
  PSZ_W pszPos = NULL;          // position of found tags
  BOOL fTagsRemoved = FALSE;    // TRUE = tags have been removed

  do
  {
    fFound = RTFFindTagsW( pszSource, pszRemoveTags, &iLen, &iTagIndex, &pszPos );
    if ( fFound )
    {
      BOOL fBlankFollowing, fTagPreceeding, fTagAtEnd;

      fTagPreceeding = RTFTagIsPreceedingTextW( pszTarget, pszOutData );  // use output buffer to check for preceeding tags!
      fTagAtEnd = RTFTagIsPreceedingTextW( pszPos + iLen, pszPos );
      fBlankFollowing = (pszPos[iLen] == L' ');

      // copy data up to tags to target area
      while ( pszSource < pszPos ) *pszTarget++ = *pszSource++;

      // skip tags
      pszSource = pszPos + iLen;

      // special handling for blanks following removed tags:
      // if there is a tag at the end of the removed data and a blank is
      // following but the removed text is not directly preceeded by a tag
      // remove blank as well as it is the tag end delimter of the removed tags
      if ( !fTagPreceeding && fTagAtEnd && fBlankFollowing )
      {
        pszSource++;
      } /* endif */

      // remember that segment has been changed
      fTagsRemoved = TRUE;
    } /* endif */
  } while ( fFound );

  // copy rest of source to target
  while ( *pszSource ) *pszTarget++ = *pszSource++;
  *pszTarget = 0;

  // remove orphaned curly braces from segment if tags had been removed
  if ( fTagsRemoved )
  {
    RTFRemoveSuperfluousBracesW( pszOutData );
  } /*endif */

  // re-arrange linefeeds to avoid nearly empty lines
  if ( fTagsRemoved )
  {
    RTFAdjustLinefeedW( pszOutData );
  } /* endif */

  return( fTagsRemoved );

} /* end of function RTFRemoveTagsW */

BOOL RTFChangeTagsW
(
  PSZ_W pszInData,              // ptr to input data
  PSZ_W pszOutData,             // ptr to output data
  PSZ_W pszChangeFromTags,      // tags to be changed
  PSZ_W pszChangeToTags         // list with new tags
)
{
  PSZ_W pszSource = pszInData;
  PSZ_W pszTarget = pszOutData; // ptr to target
  BOOL fFound = FALSE;          // TRUE = tags have been found
  int  iLen = 0;                // length of found tags
  int  iTagIndex = 0;           // index of found tags
  PSZ_W pszPos = NULL;          // position of found tags
  BOOL fTagsChanged = FALSE;    // TRUE = tags have been removed

  do
  {
    iTagIndex = 0;
    fFound = RTFFindTagsW( pszSource, pszChangeFromTags, &iLen, &iTagIndex, &pszPos );
    if ( fFound )
    {
      BOOL fBlankFollowing, fTagPreceeding, fTagAtEnd, fEndDifferent;
      PSZ_W pszChangeTo;

      fTagPreceeding = RTFTagIsPreceedingTextW( pszPos, pszSource );
      fTagAtEnd = RTFTagIsPreceedingTextW( pszPos + iLen, pszPos );
      fBlankFollowing = (pszPos[iLen] == L' ');

      // find change-to tag
      pszChangeTo = pszChangeToTags;
      while ( iTagIndex )
      {
        pszChangeTo += UTF16strlenCHAR( pszChangeTo ) + 1;
        iTagIndex--;
      } /*endwhile */

      // check if changefrom and changtotags are different concerning blanks at the end
      {
        int iChangeToLen = UTF16strlenCHAR( pszChangeTo );

        fEndDifferent = FALSE;

        if ( iChangeToLen )
        {
          if ( pszChangeTo[iChangeToLen-1] == L' ' )
          {
            fEndDifferent = (pszPos[iLen-1] != L' ');
          }
          else
          {
            fEndDifferent = (pszPos[iLen-1] == L' ');
          } /* endif */
        }
        else
        {
          fEndDifferent = TRUE;
        } /* endif */
      }

      // copy data up to tags to target area
      while ( pszSource < pszPos ) *pszTarget++ = *pszSource++;

      // skip tags
      pszSource = pszPos + iLen;

      // add change-to tags
      {
        while ( *pszChangeTo ) *pszTarget++ = *pszChangeTo++;
      }

      // special handling for blanks following removed tags:
      // if there is a tag at the end of the removed data and a blank is
      // following but the removed text is not directly preceeded by a tag
      // remove blank as well as it is the tag end delimter of the removed tags
      if ( fEndDifferent && !fTagPreceeding && fTagAtEnd && fBlankFollowing )
      {
        pszSource++;
      } /* endif */

      // remember that segment has been changed
      fTagsChanged = TRUE;
    } /* endif */
  } while ( fFound );

  // copy rest of source to target
  while ( *pszSource ) *pszTarget++ = *pszSource++;
  *pszTarget = 0;

  return( fTagsChanged );

} /* end of function RTFChangeTagsW */

// the following code has not been completed yet
#ifdef CHANGERTFTAGS

// buffer sizes for function RTFProcessFile
#define RTFPROCESSFILEBUFSIZE 32000

// work area for function RTFProcessFile
typedef struct _RTFPROCESSFILEWORKAREA
{
  // input buffer
  CHAR        achInputBuffer[RTFPROCESSFILEBUFSIZE];
  ULONG       ulCharsInBuffer;
} RTFPROCESSFILEWORKAREA, *PRTFPROCESSFILEWORKAREA;

// run through a RTF document and perform the tag changes
USHORT RTFProcessTags
(
  PSZ         pszInFile,               // name of input file
  PSZ         pszOutFile,              // name of output file
  ULONG       ulProcessType            // type of processing
)
{
  USHORT      usRC = 0;                // function return code
  BOOL        fWriteToOutput = TRUE;   // TRUE = write current data to output file
  PRTFPROCESSFILEWORKAREA pData = NULL;// pointer toour work area

  // allocate our work area
  if ( !UtlAlloc( (PVOID*)&pData, 0L, sizeof(RTFPROCESSFILEWORKAREA), ERROR_STORAGE ) )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // open input file
  if ( !usRC )
  {
    usRC = UtlOpen( pszSource, &hInFile, &usOpenAction, 0L, FILE_NORMAL, FILE_OPEN,
                    OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, 0L, TRUE );
  } /* endif */

  // open/create output file

  // process input while not end of file
  while ( !usRC && ulBytesToProcess )
  {
    // refill input buffer
    if ( (ulBytesInBuffer < (RTFPROCESSFILEBUFSIZE / 2)) && ulBytesToRead )
    {
      // move not processed data to begin of buffer
      if ( ulBytesInBuffer )
      {
        ULONG ulProcessed = pCurPos - pData->achInputBuffer;
        memmove( pData->achInputBuffer, pData->achInputBuffer + ulProcessed, ulBytesInBuffer - ulProcessed );
        pCurPos = pData->achInputBuffer;
        ulBytesInBuffer -= ulProcessed;
      } /* endif */

      if ( )
      {
      } /* endif */

      move...
      read...
      adjust...
    } /* endif */

    // look for start of next tag or curly brace
    while ( pData->achInBuf[notag )
    {
      if ( fWriteToOutput )
      {
        // write output buffer if full
        if ( ulBytesToWrite  == sizeof( pData->achOutBuf ) )
        {
          ULONG ulWritten = 0;
          usRC = UtlWriteL( hOutFile, pData->achOutBuf, ulBytesTowrite, &ulWritten, TRUE );
          ulBytesToWrite = 0;
        } /* endif */

        // add to output...
        pData->achOutBuf[ulBytesToWrite++] =
      }
      next char
    } /* endwhile */
  } /* endwhile */

  // close files and cleanup
  if ( hInFile ) UtlClose( hinFile, FALSE );
  if ( hOutFile )
  {
    ULONG ulWritten = 0;
    if ( ulBytesToWrite ) UtlWriteL( hOutFile, pData->achOutBuf, ulBytesTowrite, &ulWritten, TRUE );
    UtlClose( hOutFile, FALSE );
  } /* endif */
  if ( pData ) UtlAlloc( (PVOID*)&pData, 0L, 0L, NOMSG );
  return( usRC );

#endif
