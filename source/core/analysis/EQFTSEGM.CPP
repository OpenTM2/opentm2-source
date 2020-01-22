/*! \brief EQFTSEGM.C - TextSegmentation Module
	Copyright (c) 1999-2016, International Business Machines Corporation and others. All rights reserved.
	Description: This module contains the modules needed for text segmentation
*/

// define this to force the MT output in nFluent XML format (this same define must be set in EQFIANA1.C and EQFFTAPH2.C as well)
#define NFLUENT_MT_IF


#ifdef _DEBUG
  // activate define to enable segmentation logging
  //#define TSEGM_SEGLOGGING

  // activate to measure times
  //  #define MEASUREANALYSISTIME
  //#define SESSIONLOG
#endif

// GQ 2017/10/31 activate the following define to protect the attributes of protected tags
// the protection of tag attributes had been added to fix P403787
// as the attributes of of <meta> tags sometimes have to be translated and no rules
// could be defined when to protect the attributes and when not the fix had to be
// deactivated
//#define PROTECT_ATTRIBUTES_OF_PROTECTED_TAGS

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_SLIDER           // slider utility functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TP               // public translation processor functions
#include <eqf.h>                  // General Translation Manager include file
#include "eqftmi.h"
#include "eqftag00.h"

#include "OtmProposal.h"
#include "AnalysisMemLookupData.h"
#include "core\memory\MemoryFactory.h"
#include "eqftai.h"               // Private include file for Text Analysis
#include "eqftadit.h"             // include file for SGML-DITA specific functions

#define EQFTAML_MATCHLISTFUNCTIONS
#include "eqftaml.h"

#include <eqfiana1.id>            //analysis dialog ids
#include <time.h>                 // C library for time functions
#include "EQFHLOG.H"              // defines for history log processing
#include "EQFEVENT.H"             // defines for event logging processing
  #include "OTMFUNC.H"            // public defines for function call interface
  #include "EQFFUNCI.H"           // private defines for function call interface

#include <EQFTMTAG.H>             // Translation Memory SGML tags
#include "EQFPARSE.H"
#include <OTMGLOBMem.h>         // Global Memory defines

#include "EQFMT.H"


/** List of tag groups for section processing
    Note: enlarge the value MAXNUMOFPROTECTTAGGROUPS in EQFTAI.H when new tag groups are added to the tables */
XMPTAG XmpStartTags1[] ={ { L"<xmp", 4, XMPTAGID }, { L"<screen", 7, SCREENTAGID }, { L"<msgnum", 7, MSGNUMTAGID }, { L"<codeblock", 10, CODEBLOCKTAGID }, { L"<meta", 5, METATAGID }, { L"", 0, ENDOFTAGLIST } };
XMPTAG XmpEndTags1[] ={ { L"</xmp", 5, XMPTAGID }, { L"</screen", 8, SCREENTAGID }, { L"</msgnum", 8, MSGNUMTAGID },  { L"</codeblock", 11, CODEBLOCKTAGID }, { L"</meta", 6, METATAGID }, { L"", 0, ENDOFTAGLIST } };

XMPTAG XmpStartTags2[] ={ { L"<xmp", 4, XMPTAGID }, { L"<screen", 7, SCREENTAGID }, { L"<msgnum", 7, MSGNUMTAGID }, { L"<codeblock", 10, CODEBLOCKTAGID }, { L"", 0, ENDOFTAGLIST } };
XMPTAG XmpEndTags2[] ={ { L"</xmp", 5, XMPTAGID }, { L"</screen", 8, SCREENTAGID }, { L"</msgnum", 8, MSGNUMTAGID },  { L"</codeblock", 11, CODEBLOCKTAGID }, { L"", 0, ENDOFTAGLIST } };

/** list of markup tables and the associated start/stop tag groups for section protection. No protection is done in markup tables not listed in this table */
XMPMARKUPS XmpMarkups[] = {
  { "IBMIDDOC",   XmpStartTags1, XmpEndTags1 },
  { "IBMDITA",    XmpStartTags1, XmpEndTags1 }, 
  { "IBMHTM32",   XmpStartTags2, XmpEndTags2 }, 
  { "IBMUHTM3",   XmpStartTags2, XmpEndTags2 }, 
  { "IBMXAHTM",   XmpStartTags1, XmpEndTags1 }, 
  { "IBMXHTML",   XmpStartTags1, XmpEndTags1 }, 
  { "IBMXUHTM",   XmpStartTags1, XmpEndTags1 }, 
  { "",           NULL, NULL  } };




// define to activate code for the R007097
#define R007097_PROTECT_XMP_SCREEN

// access list of allowed source and target languages
#define MTLIST_ALLOWEDTARGETLANG
#define MTLIST_ALLOWEDSOURCELANG
#include <eqfmtlst.h>

#include <eqfaprof.h>             // analysis profile functions

static CHAR chJoinString[] = "JOIN=2";   // static variable for JOIN string
static CHAR chTransString[] = "S=";     // static variable for S= string
static CHAR szCloseSeq[] = "\x0D\x0A\x1A";  // file close sequence
static CHAR szCloseSeqUnicode[] = "\x0D\x00\x0A\x00\x1A\x00"; // file close sequence for unicode files
#define UNICODECLOSESEQLEN 6

#ifdef TSEGM_SEGLOGGING
   static FILE *hSegLog = NULL;
#endif

#ifdef TSEGM_SEGLOGGING
  void LogSegData( FILE *hf, PSZ_W pszString );
#endif

#ifdef EVENTLOG
  #define EVENTLOGFILE() FILE *hfEventLog = NULL
  #define OPENEVENTLOG( name ) { CHAR ln[60]; UtlMakeEQFPath( ln, NULC, LOG_PATH, NULL ); strcat( ln, "\\" ); strcat( ln, name ); strcat( ln, ".log" ); hfEventLog = fopen( ln, "a" ); }
  #define CLOSEEVENTLOG() { if ( hfEventLog != NULL ) { fclose( hfEventLog ); hfEventLog = NULL; } }
  #define WRITEEVENTLOG( msg ) { if ( hfEventLog != NULL ) fprintf( hfEventLog, "%s\n", msg ) }
  #define WRITE2EVENTLOG( form, data ) if ( hfEventLog != NULL ) { fprintf( hfEventLog, form, data ); fputs( "\n", hfEventLog ); }
#else
  #define EVENTLOGFILE()
  #define OPENEVENTLOG( name )
  #define CLOSEEVENTLOG()
  #define WRITEEVENTLOG( msg )
  #define WRITE2EVENTLOG( form, data )
#endif


BOOL isXMPScreenStartTag( PTAINSTDATA pInD, PSZ_W pszTag, USHORT usTagLen, PSZ pszMarkup, PBOOL pfIsSelfContainedProtectTag  );
BOOL isXMPScreenEndTag( PTAINSTDATA pInD, PSZ_W pszTag, USHORT usTagLen, PSZ pszMarkup );
BOOL isSelfContainedTagEnd( PTAINSTDATA pInD, PSZ_W pszTag, USHORT usTagLen, PSZ pszMarkup );


/** Prepare the list of start and stop tags based on user selection and current markup table name 
 \param pInD pointer to analysis instance data area
 \param pszMarkup pointer to name of current markup table
*/
BOOL PrepareTags( PTAINSTDATA pInD, PSZ pszMarkup );

/*
 * The SubstitbyBlank function was originally needed for proper padding of string before
 * calling JFrost/POE API for sentence segmentation. Because OpenTM2 uses ICU, we can customize
 * the break iterator through the use of RBBI so that the string doesn't need to be "fixed"
 * before the segmentation.
 * Defining DISABLE_SUBSTITBYBLANK will disable the calling of this function.  The function is
 * still declared and defined. This is mainly for testing purposes.
 */
#define DISABLE_SUBSTITBYBLANK

static CHAR TempTokBuf[TOK_BUFFER_SIZE];// token buffer for word counting
static DOCSAVEHIST DocSaveHist;                  // history log record for saved doc

// list of markup tables which export text files and not in binary format
// for documents using this markup tables the file end sequence restoration
// is done after calling the POSTUNSEG exit of the user exit
static char szRestoreOrgEOFAfterPostUnsegMarkups[][MAX_FORMAT]=
{
  "IBMOSM00", "IBMJDK11", "IBMUHTM3", "IBMHTM32",
  ""                                   // table end delimiter - do not remove
};

#ifdef MEASUREANALYSISTIME
  void TAWriteTimeLog( PTAINPUT pTAInput );
#endif

// prototypes for nFluent format XML output functions
#ifdef NFLUENT_MT_IF
  BOOL TAOpenMTFile( PTAINPUT pTAInput, PSZ pszFileName );
  BOOL TAWriteMTSegment( PTAINPUT pTAInput, ULONG ulLength, PSZ_W pszData, ULONG ulSegNum );
  BOOL TACloseMTFile( PTAINPUT pTAInput );
#endif

USHORT
TAIsFileUnicode
(
    PSZ   pInFile,
    PBOOL  pfIsUnicode
);

BOOL TARestoreOrgEOF
(
   PSZ pszSourceFile,                  // fully qualified source file name
   PSZ pszTargetFile,                  // fully qualified target file name
   PSZ pszSourceLang,                  // document source language
   PSZ pszTargetLang                   // document target language
);
BOOL TAGetFileEndSequence
(
  HANDLE hFile,                        // file handle
  PSZ    pszEndSequence,               // callers end sequence buffer
  PBOOL  pfUnicode,                    // callers is-unicode-flag
  PSHORT psEndLength,                  // length of end sequence in bytes
  ULONG  ulCP                          // code page of document
);

BOOL TARestartAnalysisForNewFolder
(
  PTAINPUT    *ppTAInput,              // address of callers TAInput structure pointer
  PSZ         pszNewFolObjName         // points to object name of new folder
);

#define FREENODEAREA( pNodeArea )                   \
{                                                   \
  PNODEAREA  pArea, pRoot;                          \
  pRoot = pNodeArea;                                \
  if ( pRoot )                                      \
  {                                                 \
     do                                             \
     {                                              \
       pArea = pRoot;                               \
       pRoot = pArea->pNext;                        \
       UtlAlloc( (PVOID *) (PVOID *)&pArea, 0L, 0L, NOMSG );  \
     } while ( pRoot ); /* enddo */                 \
  } /* endif */                                     \
}

#define EOF_W L'\x1A'
#define CR_W  L'\r'
#define LF_W  L'\n'
/**********************************************************************/
/* table for check for translatable data                              */
/*                                                                    */
/* index is the character being tested                                */
/* (The table is modified by @JOR to indicate that segments contain.  */
/*  digits/numbers are marked as translatable)                        */
/**********************************************************************/
static CHAR isTranslatable[256] =
 // 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1,    // 00 - 0F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    // 10 - 1F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    // 20 - 2F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,    // 30 - 3F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    // 40 - 4F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,    // 50 - 5F
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    // 60 - 6F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,    // 70 - 7F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    // 80 - 8F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    // 90 - 9F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    // A0 - AF
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    // B0 - BF
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    // C0 - CF
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    // D0 - DF
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    // E0 - EF
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };  // F0 - FF

/*-------------------------------------------------------- private functions */
static BOOL EQFTextSegmentation (
                                HWND hwnd,
                                PTAINSTDATA  pInD,
                                PSZ_W *ppRest,
                                PSEGBOUNDARY  *ppSegboundaryList
                                );

static VOID   EQFTsnlcheck(
                     BOOL fAll, PSZ_W  *ppRest,
                     PTAGTABLE  pTagTable, PTOKENENTRY pTokenList,
                     PTOKENENTRYSEG pSegTokenList,
                     USHORT    uMaxLenList );

static VOID  DetermineWhitespace
(
  PTOKENENTRYSEG pSegTokenList,
  PTAG           pTag                  // pointer to tags
);
static USHORT GetSegValue(TAG Tag, SHORT sSegState);
static PSEGBOUNDARY GetEntry (SEGBOUNDARYTBL *pSegBoundaryTbl);

static BOOL
EQFSegDeterminate
     (PTAINSTDATA    pInD,             // pointer to instance data
      PSEGBOUNDARY   pFirstEntrySeg);  // pointer to first entry in segment
                                       // boundary list

static BOOL
SetSegInfo(
      PTAINSTDATA    pInD,             // pointer to instance data
      PTOKENENTRYSEG *ppSegTokenEntry,
      SEGBOUNDARYTBL * pSegBoundaryTbl,
      PTAG           pTag,
      PATTRIBUTE     pAttribute,                                /* 1@KIT1110C */
      SHORT          sSegState,
      SEGTYPE        *pSegType,
      PSEGBOUNDARY   * ppCurSegBoundary,
      USHORT           uNumTags);

BOOL EQFTolstComplex (
                     HWND  hwnd,
                     PTAINSTDATA pInD,
                     PSEGBOUNDARY   pFirstEntrySeg
                     );

static BOOL
TolstSegmentation
     (PTAINSTDATA    pInD,             // pointer to instance data
      PSZ_W          pTolstBuffer,
      SEGBOUNDARYTBL * pSegBoundaryTbl,   // pointer to segment boundary table
      PTOKENENTRYSEG pFirstSegToken,      // pointer to segmenting tokenlist
      PTOLSTCONTROL  pTolstControl,       // pointer to control info
      PSEGBOUNDARY   pFirstEntrySeg);     // pointer to first entry in segment

static BOOL
LineByLineSegmentation
     (PTAINSTDATA    pInD,             // pointer to instance data
      PSZ_W             pTolstBuffer,
      SEGBOUNDARYTBL * pSegBoundaryTbl,   // pointer to segment boundary table
      PTOKENENTRYSEG pFirstSegToken,      // pointer to segmenting tokenlist
      PSEGBOUNDARY   pFirstEntrySeg);     // pointer to first entry in segment

static VOID
AdjustTextPosition(
      PSZ_W           *ppText,
      PTOKENENTRYSEG  *ppSegTokenEntry,
      USHORT          *pusCurLength,
      USHORT          usTextLength);

static VOID
CalcLastColumnPos(
      PSZ_W    pTextEnd,
      PSZ_W    pTextStart,
      USHORT * pusColumnPos);

static VOID
CalcSegState (
      PTOKENENTRYSEG pSegTokenList,
      SHORT          *psSegState,
      PTAG           pTag,
      PSZ_W          pTextEnd,
      USHORT         uNumTags);



//static USHORT TABlockProcessing( HWND, PANALYSIS_IDA);      // process block of data
static USHORT TABlockProcessing( HWND, PTAINPUT );      // process block of data
static USHORT TAFileProcessing( HWND, PTAINPUT, PUSHORT); // process a new file
static VOID   TAListProcessing( PTAINPUT pTAInput );  // process lists
static USHORT TACreate ( PTAINPUT );                // init for textanalysis
static USHORT TATerminate (HWND, PTAINPUT);         // terminate text analysis
static USHORT TACompletion (HWND, PTAINPUT);        // notify handlers of completion
static USHORT TAOpenFiles (PTAINPUT, PSZ);          // open input source, out
                                                    //    seg source
static USHORT TACloseFiles (PTAINPUT );             // close source
                                                    //    seg source
static USHORT TACopyTemp (PTAINPUT );               // copy temp file
                                                    //      to segm source
                                                    //         to segm Target
static USHORT TASaveSegFile ( PTAINPUT );          // create segmented files
static BOOL StartTM (PTAINPUT, PTAINSTDATA);       // init translation memories
static VOID EndTM(PTAINPUT, PTAINSTDATA);          // end translation memory
static VOID TATokenProcessing ( PTAINPUT );        // fill token list with term
static VOID SubstitbyBlank( PSZ_W pBufferStart );

static BOOL
ForceSegboundary(
      SEGBOUNDARYTBL * pSegBoundaryTbl,
      PSEGBOUNDARY   * ppStartSegBoundary,
      int              iMaxSegSize);

// Function which checks if the given segmented file is in UTF16 format
BOOL TAIsUTF16SegFile
(
  PSZ    pszSegFile                    // fully qualified segmented file
);

//static BOOL
//UnSegment(
//      PSZ pSegTarget,                       // segm.target file
//      PSZ pTarget,                          // target file name
//      PTATAG pTATag,                        // pointer to tag str.
//      PBOOL  pfGoOn,                         // goon allowed
//      PSZ    apszRevMark[6],                // revision mark array
//      PEQF_BOOL  pfKill );                  // pointer to kill flag   /* @50A */
//
//
//static BOOL
//TAUnSegment
//(
//  PLOADEDTABLE pLoadedTable,           // ptr to loaded document table
//  PSZ pSegTarget,                      // segm.target file
//  PSZ pTarget,                         // target file name
//  PTATAG pTATag,                       // pointer to tag str.
//  PBOOL  pfGoOn,                       // goon allowed
//  PSZ    apszRevMark[6],               // revision mark array
//  PEQF_BOOL  pfKill                    // pointer to kill flag
//);


static BOOL
TAUnSegmentW                            // unsegment unicode file
(
  PLOADEDTABLE pLoadedTable,           // ptr to loaded document table
  PSZ pSegTarget,                      // segm.target file
  PSZ pTarget,                         // target file name
  PTATAG pTATag,                       // pointer to tag str.
  PBOOL  pfGoOn,                       // goon allowed
  PSZ    apszRevMark[6],               // revision mark array
  USHORT usTrackDocNum,                // not 0, TVT tracking document num
  PEQF_BOOL  pfKill,                   // pointer to kill flag
  PSZ    pszConversion,                // ptr to conversion or NULL
  PSZ    pszTgtLang,                   // ptr to tgt language
  BOOL   fPostUnsegW                   // TRUE = user exit uses EqfPostUnsegW
);


static VOID TASaveSegTagTok( PSEGBOUNDARY, PTAINSTDATA, USHORT, USHORT );
static USHORT EQFMergeTRNote( PTAINSTDATA, PSEGBOUNDARY, USHORT,
                              SHORT, PTAG, PSZ_W, PSZ_W, ULONG, ULONG);
static BOOL TAGetUnicodeFlagFromDocProp (  PSZ  pszDocObjName );

static PSZ_W TAAddRevMark( USHORT usOldRevMark,
                           USHORT usNewRevMark,
                           PSZ_W  pBuffer,
                           PSZ_W  apszRevMark[6],
                           BOOL   fLastCharIsLF  );

/*----------------------------------------------------------------------------*\
| EQFTAMnl : Text Analysis Main process                                        |
|                                                                              |
| purpose  : perform text analysis on a list of files with the options         |
|            specified in the input params.                                    |
| Input    : pointer to the Instance data area, which contains the activation  |
|            parameters and the pointer to the instance  private data area     |
|            which will be allocated by this module                            |
| Output   : the segmented source and target for every input files are written |
|            in the folder directory                                           |
| Pseudo Code: see low level analysis doc: LPLLD.SCR                           |
|                                                                              |
\*----------------------------------------------------------------------------*/
VOID EQFTAMnl (HWND hwnd,                   // text analysis window handle
               PTAINPUT  *ppTAInput)      // input structure for text analysis
   {
   PTAINSTDATA   pInD;                 // pointer to instance data for TA
   USHORT        usReturn;             // return values from UtlError
   BOOL          fOK = TRUE;           // Proessing status flag
   TATASK        usPhase;              // activity sequencer
   BOOL          fClosing = FALSE;     // termination in progress
   USHORT        usFilePhase = 0;          // current file processing phase
   USHORT        usSegPhase = 0;           // current segment processing phase
   USHORT        usRedCountPhase = 0;      // current redundancy counting phase
   PTAINPUT      pTAInput = *ppTAInput;

#ifdef TSEGM_SEGLOGGING
   {
     CHAR szLogFile[MAX_EQF_PATH];

     UtlMakeEQFPath( szLogFile, NULC, SYSTEM_PATH, NULL );
     strcat( szLogFile, "\\LOGS" );
     UtlMkDir( szLogFile, 0L, FALSE );

     strcat( szLogFile, "\\EQFTSEGM.LOG" );
     hSegLog = fopen( szLogFile, "ab" );
     if ( hSegLog )
     {
       fwprintf( hSegLog, L"=================================\n" );
     } /* endif */
   }
#endif

#ifdef MEASUREANALYSISTIME
   {
     CHAR szLogFile[MAX_EQF_PATH];
     FILE *hfLog;

     UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
     UtlMkDir( szLogFile, 0L, FALSE );
     strcat( szLogFile, "\\ANATIME.LOG" );
     hfLog = fopen( szLogFile, "a" );
     if ( hfLog )
     {
       fprintf( hfLog, "== Analysis Time Measurement ==\n" );
       time( &(pTAInput->lStartTime) );
       fprintf( hfLog, "Start time : %s", asctime( localtime( &(pTAInput->lStartTime) ) ) );
       fclose( hfLog );
     } /* endif */
   }
#endif

   fOK = UtlAllocHwnd( (PVOID *)&(pTAInput->pInD),     // alloc space for instance data
                   0L,
                   (LONG) (sizeof (TAINSTDATA)),
                   ERROR_STORAGE, pTAInput->hwndErrMsg );

   if ( fOK)
      {
      usPhase = EQFTA_INIT;
      pTAInput->fKill = FALSE;         // kill request
      pInD = pTAInput->pInD;           // pointer to data

      /****************************************************************/
      /* Copy to fUseLatestTMMatch to inner structure PTAINSTDATA     */
      /* as well as the fExactContextTMMatch flag.           $        */
      /* (The flags are needed by lower-level functions which have no */
      /* access to PTAINPUT structure)                                */
      /****************************************************************/
      pInD->fUseLatestTMMatch = pTAInput->fUseLatestTMMatch;
      pInD->fExactContextTMMatch = pTAInput->fExactContextTMMatch;
      pInD->fProtXmp = pTAInput->fProtXmp;
      pInD->fProtMsgNum = pTAInput->fProtMsgNum;
      pInD->fProtScreen = pTAInput->fProtScreen;
      pInD->fProtMeta = pTAInput->fProtMeta;
      pInD->fProtCodeBlock = pTAInput->fProtCodeBlock;

      while (usPhase != EQFTA_DONE)
         {
         if ( pTAInput->fKill )           // problems or
         {                             // request for stop processing
           pInD->fTerminate = TRUE;
           pTAInput->fKill = FALSE;      // close flag already processed
         }

         UtlDispatch();   // should be called only once, but as often as possible

         /*----------- requests to terminate ---------*/
         pInD = pTAInput->pInD;           // pointer to data
         if ( (((hwnd != HWND_FUNCIF) && !WinIsWindow( NULLHANDLE, hwnd)) || pInD->fTerminate) &&  //TEST MK for Non-DDE
               !fClosing )            // request not yet processed
         {
           // allow completion of TAFileProcessing (if active)
           if ( usPhase == EQFTA_FILE )
           {
             if ( usFilePhase != EQFTA_DONE )
             {
               if ( usFilePhase != EQFTA_FREE )    // already terminating???
               {
                 usFilePhase = EQFTA_ABORT;
               } /* endif */
               do
               {
                 TAFileProcessing( hwnd, pTAInput, &usFilePhase );
               } while( usFilePhase != EQFTA_DONE );
             } /* endif */
           } /* endif */

           // allow completion of TASegProc (if active)
           if ( usPhase == EQFTA_SEG2 )
           {
             if ( usSegPhase != EQFTA_DONE )
             {
               if ( usSegPhase != EQFTA_FREE )    // already terminating???
               {
                 usSegPhase = EQFTA_ABORT;
               } /* endif */
               do
               {
                 TASegProc( hwnd, pTAInput, &usSegPhase );
               } while( usSegPhase != EQFTA_DONE );
             } /* endif */
           } /* endif */

           // allow completion of TARedundCout (if active)
           if ( usPhase == EQFTA_REDUNDCOUNT )
           {
             if ( usRedCountPhase != EQFTA_DONE )
             {
               if ( usRedCountPhase != EQFTA_FREE )    // already terminating???
               {
                 usRedCountPhase = EQFTA_ABORT;
               } /* endif */
               do
               {
                 TARedundCount( hwnd, pTAInput, &usRedCountPhase );
               } while( usRedCountPhase != EQFTA_DONE );
             } /* endif */
           } /* endif */


           usPhase = EQFTA_ABORT;
           fClosing = TRUE;
         }

         switch ( usPhase )
            {
            case EQFTA_INIT:
                 usReturn = TACreate ( pTAInput );    // do init processing

                 if (usReturn == TACOMPL)
                    {
                    usPhase = EQFTA_NEXTFILE;
                    pInD->pszCurSourceFile = (PSZ) ((PBYTE)pTAInput + // point to first
                                pTAInput->stSourcefiles.ulOffset); // file
                    pInD->usCurNumSourceFile = 1;                    // and count
                    }
                 else
                    usPhase = EQFTA_ABORT;
                 break;

            case EQFTA_NEXTFILE:    // get next source file and process it
                 /*-- if there is a file  to process ---*/
                 if (pInD->usCurNumSourceFile <= pTAInput->stSourcefiles.usNumber)
                 {
                    if ( pInD->pusFileStatus[pInD->usCurNumSourceFile-1] )
                    {
                       usFilePhase = EQFTA_INIT;
                       usPhase = EQFTA_FILE;
                       pTAInput->szTempName[0] = EOS;      // no temp file yet
                    }                                           /* 4@KIT1322A */
                    else
                    {
                        pInD->usCurNumSourceFile += 1;  // increase counter
                        pInD->pszCurSourceFile +=           // point to next
                           strlen(pInD->pszCurSourceFile) + 1; // skip NULL
                       usPhase = EQFTA_NEXTFILE;
                    } /* endif */
                 }  // end if file to process
                 else
                    usPhase = EQFTA_SEG1;    // do segment processing next
                 break;


            case EQFTA_FILE:    // get next source file and process it
                 /*-- if there is a file  to process ---*/
                 TAFileProcessing( hwnd, pTAInput, &usFilePhase );
                 if ( usFilePhase == EQFTA_DONE)
                 {
                   usPhase = EQFTA_NEXTFILE;

                   // invoke PostTM exit if no further processing is
                   // done with the file
                   if ( pInD->pusFileStatus[pInD->usCurNumSourceFile-1] &&
                        !(pTAInput->fInsertTMMatches ||  // autom trans.
                         pTAInput->fInsertToTM      ||      // update TM
                         pTAInput->fInsertNewMatch  ||   // new match output
                         pTAInput->fMachTrans       ||   // machine translation
                         pTAInput->fTMMatch         ||   // TM match counting
                         pTAInput->fRedundCount     ||   // redundancy counting
                         pTAInput->fNTLProcessing ) &&  // list processing
                         (pInD->pfnEQFPostTM || pInD->pfnEQFPostTM2 || pInD->pfnEQFPostTmW) )// any of PSTTM  exits is there
                   {
                      if ( !pTAInput->fBatch )
                      {
                        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                        LOADSTRING( NULLHANDLE, hResMod, IDS_POSTTM,
                                    pTAInput->pInD->szTempDisp);

                        WinSendMsg( pTAInput->hwndProcWin,
                                    WM_EQF_UPDATESLIDER,
                                    MP1FROMSHORT(-1), // do not update pos
                                    MP2FROMP(pTAInput->pInD->szTempDisp) );
                      } /* endif */

                      fOK = TAProcessPostTM( pTAInput, pInD,
                                             pTAInput->szFileName,
                                             pTAInput->szSegTarget,
                                             FALSE );


                      if ( !fOK )
                      {
                         PSZ pszDoc;
                         EVENTLOGFILE();
 
                         pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
 
                         OPENEVENTLOG( "EQFTSEGM" );
                         WRITE2EVENTLOG( "TASegCloseFiles failed, doc is %s", pszDoc );
                         CLOSEEVENTLOG();

                         usReturn = UtlErrorHwnd( ERROR_TA_SAVE_SEGFILE,
                                             MB_YESNO,
                                             1,
                                             &pszDoc,
                                             EQF_QUERY,
                                             pTAInput->hwndErrMsg );
                         pTAInput->fKill = (usReturn == MBID_YES) ? FALSE : TRUE;
                         usPhase = EQFTA_ABORT;
                      } /* endif */
                    }                                           /* 4@KIT1322A */
                    else
                    {
                       usPhase = EQFTA_NEXTFILE;
                    } /* endif */
                    pInD->usCurNumSourceFile += 1;  // increase counter
                    pInD->pszCurSourceFile +=           // point to next
                         strlen(pInD->pszCurSourceFile) + 1; // skip NULL
                 } /* endif */
                 break;



            case EQFTA_SEG1:
                 // check if segment processing is needed
                 // if so, do initialization
                 if (pTAInput->fInsertTMMatches ||  // autom trans.
                     pTAInput->fInsertToTM      ||  // update TM
                     pTAInput->fInsertNewMatch  ||  // new match output
                     pTAInput->fMachTrans       ||  // machine translation
                     pTAInput->fTMMatch         ||   // TM match counting
                     pTAInput->fRedundCount     ||   // TM redundancy counting
                     pTAInput->fNTLProcessing       // list processing
                     )
                    {
                      if ( !pTAInput->fBatch )
                      {
                        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                        LOADSTRING( NULLHANDLE, hResMod, IDS_TMP,
                                    pTAInput->pInD->szTempDisp);

                        WinSendMsg( pTAInput->hwndProcWin,
                                    WM_EQF_UPDATESLIDER,
                                    MP1FROMSHORT(-1), // do not update pos
                                    MP2FROMP(pTAInput->pInD->szTempDisp) );
                      } /* endif */
                    usReturn = TASegCreate ( hwnd, pTAInput );  // init

                    if ( pTAInput->fMatchList )
                    {
                      TAMatchListInit( pTAInput );
                    } /* endif */

                    if (usReturn)
                    {
                       TARestartList( pTAInput );
                    }
                    if (usReturn == TACOMPL)
                    {
                       pInD->pszCurSourceFile = (PSZ) ((PBYTE)pTAInput + // point to first
                                   pTAInput->stSourcefiles.ulOffset); // file
                       pInD->usCurNumSourceFile = 1;                    // and count
                       usPhase = EQFTA_SEGNEXT;
                    }
                    else
                       usPhase = EQFTA_ABORT;
                 }
                 else
                    usPhase = EQFTA_COMPLETE;

                 break;

            case EQFTA_SEGNEXT: // get next source
                 if (pInD->usCurNumSourceFile <= pTAInput->stSourcefiles.usNumber)
                 {   // if file status is TRUE: no error
                    if (*(pInD->pusFileStatus + (pInD->usCurNumSourceFile - 1)))
                    {
                       usSegPhase = EQFTA_INIT;
                       usPhase = EQFTA_SEG2;
                    }
                    else
                    {
                      pInD->usCurNumSourceFile += 1; // increase counter
                      pInD->pszCurSourceFile +=     // point to next
                           strlen(pInD->pszCurSourceFile) + 1; // skip NULL
                      usPhase = EQFTA_SEGNEXT;
                    } /* endif */
                 }
                 else
                 {
                    usPhase = EQFTA_LIST2;
                 }  /* endif */
                 break;

            case EQFTA_SEG2: // segment processing for current source file
                 /*-- if there is a file  to process ---*/
                 TASegProc( hwnd, pTAInput, &usSegPhase );
                 if ( usSegPhase == EQFTA_DONE )
                 {
                   pInD->usCurNumSourceFile += 1; // increase counter
                   pInD->pszCurSourceFile +=     // point to next
                         strlen(pInD->pszCurSourceFile) + 1; // skip NULL
                   usPhase = EQFTA_SEGNEXT;
                 } /* endif */
                 break;

            case EQFTA_LIST2:
              if (pTAInput->fNTLProcessing)
              {
                 if ( !pTAInput->fBatch )
                 {
                   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                   LOADSTRING( NULLHANDLE, hResMod, IDS_LISTOUT,
                               pTAInput->pInD->szTempDisp);

                   WinSendMsg( pTAInput->hwndProcWin,
                               WM_EQF_UPDATESLIDER,
                               MP1FROMSHORT(-1), // do not update pos
                               MP2FROMP(pTAInput->pInD->szTempDisp) );
                 } /* endif */
                 if ( LPCompletion ( hwnd, pTAInput,(PLPDATA) pInD->pLPDATA ) )
                 {
                    usRedCountPhase = EQFTA_INIT;
                    usPhase = EQFTA_REDUNDCOUNT;
                 }
                 else
                 {
                    usPhase = EQFTA_ABORT;
                    pInD->fTerminate = TRUE;
                 }
                 LPTerminate ((PLPDATA *)&(pInD->pLPDATA));   // termination
              }
              else
                 usRedCountPhase = EQFTA_INIT;
                 usPhase = EQFTA_REDUNDCOUNT;
              break;

            case EQFTA_REDUNDCOUNT:
              if (pTAInput->fRedundCount)
              {
                 if ( !pTAInput->fBatch && (usRedCountPhase == EQFTA_INIT) )
                 {
                   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                   LOADSTRING( NULLHANDLE, hResMod, IDS_REDUNDCOUNT,
                               pTAInput->pInD->szTempDisp);

                   WinSendMsg( pTAInput->hwndProcWin,
                               WM_EQF_UPDATESLIDER,
                               MP1FROMSHORT(-1), // do not update pos
                               MP2FROMP(pTAInput->pInD->szTempDisp) );
                 } /* endif */
                 TARedundCount( hwnd, pTAInput, &usRedCountPhase );
                 if ( usRedCountPhase == EQFTA_DONE )
                 {
                   usPhase = EQFTA_COMPLETE;
                 }
                 else
                 {
                   usPhase = EQFTA_REDUNDCOUNT;
                 } /* endif */
              }
              else
                 usPhase = EQFTA_COMPLETE;
              break;

            case EQFTA_COMPLETE:
               TACompletion (hwnd, pTAInput);   // terminate text analysis

               // continue with next folder if more to follow
               if ( (pTAInput->pInD->usError == TACOMPL) && !pTAInput->fNoComplete && !pTAInput->fBatch &&
                     pTAInput->pszObjList )
               {
                 BOOL fIsEmpty = FALSE;
                 // position to next folder in list but skip empty ones
                 do
                 {
                   pTAInput->pszActiveFolder += strlen(pTAInput->pszActiveFolder) + 1;
                   if ( *pTAInput->pszActiveFolder )
                   {
                     fIsEmpty = FolIsFolderEmpty( pTAInput->pszActiveFolder );
                   } /* endif */
                 }
                 while ( *pTAInput->pszActiveFolder && fIsEmpty );

                 // show termination message if no more folders to follow
                 if ( !*pTAInput->pszActiveFolder )
                 {
                   pTAInput->pszActiveFolder = NULL;      // stop further processing
                   UtlError (ERROR_TACOMPLETE, MB_OK, 0, NULL, EQF_INFO);
                 } /* endif */
               }
               else
               {
                 pTAInput->pszActiveFolder = NULL;        // stop further processing
               } /* endif */

               if ( pTAInput->pszActiveFolder )
               {
                 BOOL fNoRemoveObject;

                 // free resources used by current analysis
                 fNoRemoveObject = pTAInput->fNoRemoveObject;
                 pTAInput->fNoRemoveObject = TRUE;
                 TATerminate( hwnd, pTAInput);
                 pTAInput->fNoRemoveObject = fNoRemoveObject;

                 // restart analysis with next folder
                 if ( TARestartAnalysisForNewFolder( &pTAInput, pTAInput->pszActiveFolder ) )
                 {
                   usPhase = EQFTA_INIT;
                   pInD = pTAInput->pInD;   // refresh pointer
                 }
                 else
                 {
                   usPhase =  EQFTA_FREE;          // free resources
                 } /* endif */
               }
               else
               {
                 usPhase =  EQFTA_FREE;          // free resources
               } /* endif */
               break;

            case EQFTA_ABORT:
               // dummy clear process results
               if ( pTAInput->fBatch )
               {
                 pTAInput->pDDEAnalysis->DDEReturn.usRc =
                                            UtlGetDDEErrorCode( pTAInput->hwndErrMsg );
               } /* endif */
               usPhase =  EQFTA_FREE;       // free resources
               break;

            case EQFTA_FREE:
               if ( pTAInput->fMatchList )
               {
                 TAMatchListTerminate( pTAInput );
               } /* endif */

               if ( pTAInput->fBalanceList )
               {
                 PSZ pszSourceFiles = (PSZ)( (PBYTE) pTAInput + pTAInput->stSourcefiles.ulOffset);
                 TABalanceList( pTAInput->szFolder, pszSourceFiles, pTAInput->stSourcefiles.usNumber );
               } /* endif */

               // free any loaded CRLF handling info
               TAFreeCRLFInfo( pInD->pCrlfLoadedInfo );


               // free resources
               TATerminate (hwnd, pTAInput);
               usPhase =  EQFTA_DONE;          // done: exit
#ifdef TSEGM_SEGLOGGING
                if ( hSegLog )
                {
                  fclose( hSegLog );
                } /* endif */
#endif
               break;

            default:
               usPhase =  EQFTA_ABORT;          // terminate task
               break;
            } /* end task selection */

         } /* endwhile usPhase != DONE */

         UtlAlloc( (PVOID *) &(pInD), 0L, 0L, NOMSG );    // free instance data space

      } /*-- endif mem alloc nok ---*/

      *ppTAInput = pTAInput;

   }  /* end of EQFTAMnl */

/*----------------------------------------------------------------------------*\
| TAFileProcessing: Perform process on a single file                           |
| purpose   : if requested perform User pre segmentation                       |
|             perform normal segmentation                                      |
|             if requested perform User post segmentation                      |
|                                                                              |
|  if there is another file to be processed in the list of files, the file     |
| Input     : pointer to the instance data area; the variable                  |
|             pszCurSourceFile must point to the name of the file              |
|             to analize                                                       |
| Output    : returns TACOMPL  if everything is OK                             |
|               else  TAERROR                                                  |
|             the segmented files are created                                  |
|  pseudocode : see Low level Analysis document LPLLD.SCR                      |
\*----------------------------------------------------------------------------*/
USHORT TAFileProcessing
(
  HWND hwnd,                           // text analysis window handle
  PTAINPUT  pTAInput,                  // input structure for text analysis
  PUSHORT   pusPhase                   // ptr to buffer for current phase
)
{
   PTAINSTDATA   pInD;                 // pointer to instance data for TA
   USHORT        usReturn = 0;         // return values from functions
   TATASK        usPhase;              // activity sequencer
   BOOL          fOK = TRUE;           // Processing status flag

   pInD = pTAInput->pInD;                   // pointer to local variables

   usPhase = (TATASK) *pusPhase;

   if ( pTAInput->fKill )           // problems or
   {                             // request for stop processing
     pInD->fTerminate = TRUE;
     pTAInput->fKill = FALSE;      // close flag already processed
     usPhase = EQFTA_ABORT;
   }

      switch ( usPhase )
         {
         case EQFTA_INIT:
              fOK = TRUE;
              pInD->ulSegsReplaced = 0L;                        /* 3@KIT1270A */
              pInD->ulReplacedWords = 0L;
              pInD->ulSegsNotReplaced = 0L;
              pInD->ulTotalSegs       = 0L;
              memset( &DocSaveHist, 0, sizeof(DocSaveHist) );

              pInD->ulNoProps     = 0L;
              pInD->ulFuzzy       = 0L;
              pInD->ulExactExact  = 0L;
              pInD->ulExactOne    = 0L;
              pInD->ulExactMore   = 0L;
              pInD->ulTotal       = 0L;
              pInD->ulRepl        = 0L;
              pInD->ulFuzzyRepl   = 0L;
              pInD->ulSegNoProps     = 0L;
              pInD->ulSegFuzzy       = 0L;
              pInD->ulSegExactExact  = 0L;
              pInD->ulSegExactOne    = 0L;
              pInD->ulSegExactMore   = 0L;
              pInD->ulSegTotal       = 0L;
              pInD->ulSegRepl        = 0L;
              pInD->ulSegFuzzyRepl   = 0L;
              pInD->ulSegMachineMatch = 0L;
              pInD->ulMachineMatch   = 0L;
              pInD->fProtectMode = FALSE;

              memset( &pInD->Total, 0, sizeof(COUNTSUMS) );
              memset( &pInD->ExactExact, 0, sizeof(COUNTSUMS) );
              memset( &pInD->Exact, 0, sizeof(COUNTSUMS) );
              memset( &pInD->Fuzzy1, 0, sizeof(COUNTSUMS) );
              memset( &pInD->Fuzzy2, 0, sizeof(COUNTSUMS) );
              memset( &pInD->Fuzzy3, 0, sizeof(COUNTSUMS) );
              memset( &pInD->NoProps, 0, sizeof(COUNTSUMS) );
              memset( &pInD->MTProps, 0, sizeof(COUNTSUMS) );
              memset( &pInD->Repl, 0, sizeof(COUNTSUMS) );
              memset( &pInD->ExactMore, 0, sizeof(COUNTSUMS) );

              /********************************************************/
              /* Get source language and format of new document       */
              /********************************************************/
              strcpy( pInD->szDocObjName, pTAInput->szFolder );
              strcat( pInD->szDocObjName, BACKSLASH_STR );
              strcat( pInD->szDocObjName, pInD->pszCurSourceFile );
              DocQueryInfo2Hwnd( pInD->szDocObjName,// document object name
                            pInD->szDocMemory,     // document TM
                            pInD->szDocFormat,     // format of document
                            pInD->szDocSourceLang, // document source language
                            pInD->szDocTargetLang, // target language needed
                            NULL,        // no long name required
                            NULL,        // no alias required
                            pTAInput->szEditName,// editor of document
                            TRUE,
                            pTAInput->hwndErrMsg );

              // check if document language is allowed for machine translation
              // when a MT job is to be created
#ifdef NFLUENT_MT_IF
#else
              if ( fOK && pTAInput->fMachTrans )
              {
                int i = 0;
                fOK = FALSE;
                while ( !fOK && (apszAllowedMTSourceLang[i] != NULL) )
                {
                  if ( stricmp( pInD->szDocSourceLang, apszAllowedMTSourceLang[i] ) == 0 )
                  {
                    fOK = TRUE;
                  } /* endif */
                  i++;
                } /* endwhile */

                if ( !fOK )
                {
                  PSZ pszErrParms[2];

                  pszErrParms[0] = pInD->szDocSourceLang;
                  pszErrParms[1] = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];

                  usReturn = UtlErrorHwnd( LANG_NOT_SUPPORTED_BY_MT,
                                           MB_CANCEL, 2, pszErrParms,
                                           EQF_WARNING, pTAInput->hwndErrMsg );
                } /* endif */
              } /* endif */

              if ( fOK && pTAInput->fMachTrans )
              {
                int i = 0;
                fOK = FALSE;
                while ( !fOK && (apszAllowedMTTargetLang[i] != NULL) )
                {
                  if ( stricmp( pInD->szDocTargetLang, apszAllowedMTTargetLang[i] ) == 0 )
                  {
                    fOK = TRUE;
                  } /* endif */
                  i++;
                } /* endwhile */

                if ( !fOK )
                {
                  PSZ pszErrParms[2];

                  pszErrParms[0] = pInD->szDocTargetLang;
                  pszErrParms[1] = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];

                  usReturn = UtlErrorHwnd( LANG_NOT_SUPPORTED_BY_MT,
                                           MB_CANCEL, 2, pszErrParms,
                                           EQF_WARNING, pTAInput->hwndErrMsg );
                } /* endif */
              } /* endif */
#endif

              /********************************************************/
              /* Activate document language if not active yet         */
              /********************************************************/
              if ( fOK && (strcmp( pInD->szCurSourceLang, pInD->szDocSourceLang ) != 0) )
              {
                /******************************************************/
                /* Deactivate language currently active               */
                /******************************************************/
                if ( pInD->szCurSourceLang[0] != EOS )
                {
                  MorphFreeLanguageID( pInD->TolstControl.sLangID );
                } /* endif */
  
                /******************************************************/
                /* Activate new language                              */
                /******************************************************/
                usReturn = MorphGetLanguageID( pInD->szDocSourceLang,
                                           &(pInD->TolstControl.sLangID) );
  
                if ( usReturn == NO_ERROR )
                {
                  pInD->TolstControl.ulOemCP = GetLangOEMCP( pInD->szDocSourceLang);
                  pInD->TolstControl.ulAnsiCP = GetLangAnsiCP( pInD->szDocSourceLang);
                  strcpy( pInD->szCurSourceLang, pInD->szDocSourceLang );
                }
                else
                {
                  PSZ pszErrParm = pInD->szDocSourceLang;
  
                  usReturn = UtlErrorHwnd( EQFRS_NOMORPH_DICT,
                                        MB_CANCEL ,
                                        1,
                                        &pszErrParm,
                                        EQF_WARNING, pTAInput->hwndErrMsg );
                  fOK = FALSE;
                } /* endif */
              } /* endif */

              /********************************************************/
              /* Load tagtable for document format if not active yet  */
              // special handling for EQFWPRO: always reload as WordPro API
              //   has problems when called more than once ...
              /********************************************************/
              if ( fOK &&
                   ( (strcmp( pInD->szCurFormat, pInD->szDocFormat ) != 0 ) ||
                     (strcmp( pInD->szCurFormat, "EQFWPRO" ) == 0 ) ) )
              {
                /******************************************************/
                /* Free tag table currently active                    */
                /******************************************************/
                if ( (pInD->szCurFormat[0] != EOS) && (pInD->pLoadedTable != NULL) )
                {
                  if ( pInD->pLoadedTable->pTagTable->szSegmentExit[0] != EOS )
                  {
                    TAFreeUserExit( pInD );
                  } /* endif */
                  TAFreeTagTable( pInD->pLoadedTable );
                  pInD->pLoadedTable = NULL;
                } /* endif */

                /******************************************************/
                /* Activate new tag table                             */
                /******************************************************/
                usReturn = TALoadTagTableHwnd( pInD->szDocFormat,
                                       &pInD->pLoadedTable,
                                       FALSE,        // load external table
                                       TRUE, pTAInput->hwndErrMsg ); // do message handling
                if ( usReturn == NO_ERROR )
                {
                  CHAR szExitName[MAX_LONGFILESPEC];
                  strcpy( pInD->szCurFormat, pInD->szDocFormat );
                  if ( MUGetUserExitFileName(  pInD->szDocFormat, NULL, szExitName, sizeof(szExitName) ) )
                  {
                    usReturn = TALoadUserExit( szExitName, pTAInput );
                    if ( usReturn != NO_ERROR )
                    {
                      fOK = FALSE;
                      pInD->szCurFormat[0] = EOS; // invalidate current format
                    } /* endif */
                  }
                  else
                  {
                    // reset all user exit function pointers
                    pInD->pfnEQFPreSeg    = NULL;
                    pInD->pfnEQFPreSeg2   = NULL;
                    pInD->pfnEQFPreSegEx  = NULL;
                    pInD->pfnEQFPostSeg   = NULL;
                    pInD->pfnEQFPostSeg2  = NULL;
                    pInD->pfnEQFPostSegEx = NULL;
                    pInD->pfnEQFPreUnSeg  = NULL;
                    pInD->pfnEQFPreUnSeg2 = NULL;
                    pInD->pfnEQFPostTM    = NULL;
                    pInD->pfnEQFPostTM2   = NULL;
                    pInD->pfnEQFCheckSeg  = NULL;     
                    pInD->pfnEQFPostSegW  = NULL;
                    pInD->pfnEQFPostSegWEx = NULL;
                    pInD->pfnEQFPostTmW   = NULL;
                    pInD->pfnEQFPreUnSegW = NULL;
                    pInD->pfnEQFCheckSegW = NULL;
                    pInD->pfnEQFCheckSegExW = NULL;
                    pInD->pfnUpdateContext = NULL;
                    pInD->pfnGetSegContext = NULL;
                  } /* endif */
                }
                else
                {
                  fOK = FALSE;
                } /* endif */
              } /* endif */

              if ( fOK )
              {
                strcpy( pTAInput->szTagTableName, pInD->szDocFormat );
                sprintf( pTAInput->szFileName,           // create full file name
                         PATHCATFILE,
                         pTAInput->szSOURCE_Path,
                         pInD->pszCurSourceFile);
                fOK = SetSegDate (pTAInput, 0L);         // reset to zero  date
				pTAInput->fSetSegDateDone = FALSE; // no document properties updated yet
                if ( !fOK )
                {
                   usReturn = pInD->usError;             // get return code
                }
                else
                {
                   PUSHORT pusTemp;
                   SHORT   sItem;                     // selected item

                   pusTemp = pInD->pusFileStatus +      // set status to false
                              (pInD->usCurNumSourceFile - 1);
                   *pusTemp = FALSE;   // set file status to failed
                   if ( hwnd != HWND_FUNCIF )
                   {
                     sItem = SEARCHITEMHWND( pTAInput->hwndToDo,
                          pTAInput->apszLongNames[pInD->usCurNumSourceFile-1] );
                     if ( sItem != LIT_NONE )
                     {
                       SELECTITEMHWND( pTAInput->hwndToDo, sItem );
                     } /* endif */
                   } /* endif */
                   usReturn = TACOMPL;                  // init processing
                } /* endif */
              } /* endif */

              if (  !fOK || (usReturn != TACOMPL) )
              {
                 usPhase = EQFTA_ABORT;
              }
              else
              {
                 /* if specified, call User exit for segmentation */
                 if (*(pInD->pLoadedTable->pTagTable->szSegmentExit))
                    usPhase = EQFTA_PRESEG;
                 else
                    usPhase = EQFTA_SEG1;  // normal segmentation
              } /* endif */
              break;

         case EQFTA_PRESEG:
              /*------ call user pre segmentation ---------------*/
              // temp file default is source file
              strcpy (pTAInput->szTempName, pTAInput->szFileName);
              fOK = TRUE;

              pTAInput->sSliderPos = 0;

              if ( !pTAInput->fBatch )
              {
                HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                LOADSTRING( NULLHANDLE, hResMod, IDS_PRESEGM,
                            pTAInput->pInD->szTempDisp);

                WinSendMsg( pTAInput->hwndProcWin,
                            WM_EQF_UPDATESLIDER,
                            MP1FROMSHORT(0),
                            MP2FROMP(pTAInput->pInD->szTempDisp) );
              } /* endif */

              // check for empty documents and skip them
              {
                WIN32_FIND_DATA FileFindData;
                HANDLE hDir;

                hDir = FindFirstFile( pTAInput->szFileName, &FileFindData );
                if ( hDir != INVALID_HANDLE_VALUE )
                {
                  if ( (FileFindData.nFileSizeHigh == 0) &&
                       (FileFindData.nFileSizeLow == 0) )
                  {
                    if ( !pTAInput->fBatch )
                    {
                      PSZ pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
                      USHORT usReturn = 0;
#ifdef SESSIONLOG
                      UtlLogWriteString( "TAFileProcessing: Error accessing file info for file %s", pTAInput->szFileName );
#endif
                      usReturn = UtlErrorHwnd( ERROR_TA_SOURCEFILE,
                                                      MB_YESNO,
                                                      1,
                                                      &pszDoc,
                                                      EQF_QUERY, pTAInput->hwndErrMsg );

                      pTAInput->fKill = (usReturn != MBID_YES);  // go to stop processing
                    } /* endif */
                    fOK = FALSE;
                  } /* endif */
                  FindClose( hDir );
                } /* endif */
              }

              if ( fOK )
              {
                BOOL fNoSegment = FALSE;


                if ( pInD->pfnEQFPreSegEx != NULL )
                {
                  fOK = pInD->pfnEQFPreSegEx(pTAInput->szTagTableName,
                                            pTAInput->szEditName,
                                            pTAInput->szProgPath,
                                            pTAInput->szFileName,
                                            pTAInput->szTempName,
                                            (PEQF_BOOL)&fNoSegment,
                                            pTAInput->hwndProcWin,
                                            &pTAInput->fKill,
                                            (LONG)pTAInput );
                }
                else if ( pInD->pfnEQFPreSeg2 == NULL )
                {
                  fOK = pInD->pfnEQFPreSeg(pTAInput->szTagTableName,
                                           pTAInput->szEditName,
                                           pTAInput->szProgPath,
                                           pTAInput->szFileName,
                                           pTAInput->szTempName,
                                           (PEQF_BOOL)&fNoSegment );
                }
                else
                {
                  fOK = pInD->pfnEQFPreSeg2(pTAInput->szTagTableName,
                                            pTAInput->szEditName,
                                            pTAInput->szProgPath,
                                            pTAInput->szFileName,
                                            pTAInput->szTempName,
                                            (PEQF_BOOL)&fNoSegment,
                                            pTAInput->hwndProcWin,
                                            &pTAInput->fKill );
                } /* endif */

#ifdef SESSIONLOG
                if ( !fOK  )
                {
                  UtlLogWriteString( "TAFileProcessing: EQFPRESEG of user exit %s failed", pInD->pLoadedTable->pTagTable->szSegmentExit );
                } /* endif */                   
#endif
                pInD->fNoSegment = (EQF_BOOL) fNoSegment;
              } /* endif */

              if (!fOK)  // user presegmentation failed
                 usPhase = EQFTA_ABORT;
              else
                 {
                 usPhase = EQFTA_PRESEG2;  // continue
                 }
              break;

         case EQFTA_PRESEG2:
              if (pInD->fNoSegment)  // user requested no standard segmentation
                 {
                 fOK = TACopyTemp(pTAInput); // copy temp file
                 if ( fOK )
                    {                              // to segmented outputs
                    usPhase = EQFTA_POSTSEG;         // go to post segm
                    }
                 else
                    usPhase = EQFTA_ABORT;
                 }
              else
                 {
                 usPhase = EQFTA_SEG1;          // start normal segm
                 if (strlen(pTAInput->szTempName) > 0)
                     strcpy (pTAInput->szFileName, pTAInput->szTempName);  // file name
                 }
              break;


         case EQFTA_SEG1:     // segmentation: open file and select
              // if an error occured during opening, the file is skipped
              // and text analysis is cancelled, if user requested so.

              if ( !pTAInput->fBatch )
              {
                HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                LOADSTRING( NULLHANDLE, hResMod, IDS_TEXTSEGM,
                            pTAInput->pInD->szTempDisp);

                WinSendMsg( pTAInput->hwndProcWin,
                            WM_EQF_UPDATESLIDER,
                            MP1FROMSHORT(-1), // do not update pos
                            MP2FROMP(pTAInput->pInD->szTempDisp) );
              } /* endif */
              fOK = TAOpenFiles(pTAInput, pTAInput->szFileName);
              if ( fOK )
                 {
                 PrepareTags( pInD, pInD->szCurFormat );  // prepare the tags lists for the section protection logic
                 usPhase = EQFTA_SEG2;  // opened: process
                 }
              else
              {
                 usPhase = EQFTA_ABORT;
#ifdef SESSIONLOG
                UtlLogWriteString( "TAFileProcessing: TAOpenFiles failed for document %s", pTAInput->szFileName );
#endif
              }

              break;

         case EQFTA_SEG2:    // segmentation: process blocks of text
              /*---- check end of file -------*/
              if ( pInD->fAll && ! pInD->pRest) // source file end process
                 {
                 usPhase =  EQFTA_SEG3;  // continue with close files
                 }
              else
                 {
                 if (TABlockProcessing( hwnd, pTAInput ))
                    usPhase =  EQFTA_SSOURCE;  // continue writing segm source
                 else
                    usPhase = EQFTA_ABORT; // kill current process
                 }
              break;

         case EQFTA_SSOURCE:   // create next block of source segm file
#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        fwprintf( hSegLog, L"Before TASaveSegFile   phase=%d\n",usPhase);
      } /* endif */
#endif
            if (TASaveSegFile ( pTAInput ))
               usPhase = EQFTA_SEG2;
            else
               usPhase = EQFTA_ABORT;
#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        fwprintf( hSegLog, L"After TASaveSegFile   phase=%d\n",usPhase);
      } /* endif */
#endif
            break;


        case EQFTA_SEG3:    // close files
            if ( TACloseFiles(pTAInput) == TACOMPL )
            {
               /*--- check if post segmentation is needed ----*/
               if (*(pInD->pLoadedTable->pTagTable->szSegmentExit))
                  usPhase = EQFTA_POSTSEG;
               else
                  usPhase = EQFTA_COMPLETE;
            }
            else
            {
               PSZ pszDoc;
               EVENTLOGFILE();

               pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];

               OPENEVENTLOG( "EQFTSEGM" );
               WRITE2EVENTLOG( "TASegCloseFiles failed, doc is %s", pszDoc );
               CLOSEEVENTLOG();
               usReturn = UtlErrorHwnd( ERROR_TA_SAVE_SEGFILE,
                                     MB_YESNO,
                                     1,
                                     &pszDoc,
                                     EQF_QUERY, pTAInput->hwndErrMsg );
               pTAInput->fKill = (usReturn == MBID_YES) ? FALSE : TRUE;
               usPhase = EQFTA_ABORT;
            } /* endif */

            break;

         case EQFTA_POSTSEG:  //--------- do post segmentation
            if ( !pTAInput->fBatch )
            {
              HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
              LOADSTRING( NULLHANDLE, hResMod, IDS_POSTSEGM,
                          pTAInput->pInD->szTempDisp);

              WinSendMsg( pTAInput->hwndProcWin,
                          WM_EQF_UPDATESLIDER,
                          MP1FROMSHORT(-1), // do not update pos
                          MP2FROMP(pTAInput->pInD->szTempDisp) );
            } /* endif */

            sprintf( pTAInput->szSegSource, PATHCATFILE,
                     pTAInput->szSEGSOURCE_Path,
                     pInD->pszCurSourceFile);

            sprintf(pTAInput->szSegTarget, PATHCATFILE,
                    pTAInput->szSEGTARGET_Path,
                    pInD->pszCurSourceFile);

            if ( pInD->pfnEQFPostSegWEx != NULL )
            {
               fOK = pInD->pfnEQFPostSegWEx( pTAInput->szTagTableName,
                                     pTAInput->szEditName,
                                     pTAInput->szProgPath,
                                     pTAInput->szSegSource,
                                     pTAInput->szSegTarget,
                                     pTAInput->TATagW,
                                     pTAInput->hwndProcWin,
                                     &pTAInput->fKill,
                                     (LONG)pTAInput );
#ifdef SESSIONLOG
              if ( !fOK  )
              {
                UtlLogWriteString( "TAFileProcessing: EQFPOSTSEGWEX of user exit %s failed", pInD->pLoadedTable->pTagTable->szSegmentExit );
              } /* endif */                   
#endif
            }
            else if ( pInD->pfnEQFPostSegW != NULL )
            {
               fOK = pInD->pfnEQFPostSegW( pTAInput->szTagTableName,
                                     pTAInput->szEditName,
                                     pTAInput->szProgPath,
                                     pTAInput->szSegSource,
                                     pTAInput->szSegTarget,
                                     pTAInput->TATagW,
                                     pTAInput->hwndProcWin,
                                     &pTAInput->fKill );
#ifdef SESSIONLOG
              if ( !fOK  )
              {
                UtlLogWriteString( "TAFileProcessing: EQFPOSTSEGW of user exit %s failed", pInD->pLoadedTable->pTagTable->szSegmentExit );
              } /* endif */                   
#endif
            }
            else
            {
              CHAR chTgtFile[MAX_EQF_PATH+1];
              CHAR chSrcFile[MAX_EQF_PATH+1];
              /******************************************************************/
              /* create temp file names -- we have to use two different output  */
              /******************************************************************/
              strcpy( chTgtFile, pTAInput->szSegTarget);
              strcat( chTgtFile, "A");
              strcpy( chSrcFile, pTAInput->szSegSource );
              strcat( chSrcFile, "B" );

              fOK = (TASegFileConvertUnicode2ASCII( pTAInput->szSegTarget,
                                                    chTgtFile,
                                                    pInD->szDocSourceLang )  ) == 0;
              fOK = (TASegFileConvertUnicode2ASCII( pTAInput->szSegSource,
                                                    chSrcFile,
                                                    pInD->szDocSourceLang )  ) == 0;
              if ( pInD->pfnEQFPostSegEx != NULL )
              {
                fOK =  pInD->pfnEQFPostSegEx(pTAInput->szTagTableName,
                                          pTAInput->szEditName,
                                          pTAInput->szProgPath,
                                          chSrcFile,
                                          chTgtFile,
                                          pTAInput->TATag,
                                          pTAInput->hwndProcWin,
                                          &pTAInput->fKill,
                                          (LONG)pTAInput );
              }
              else if ( pInD->pfnEQFPostSeg2 == NULL )
              {
                fOK =  pInD->pfnEQFPostSeg(pTAInput->szTagTableName,
                                         pTAInput->szEditName,
                                         pTAInput->szProgPath,
                                         chSrcFile,
                                         chTgtFile,
                                         pTAInput->TATag);
              }
              else
              {
                fOK =  pInD->pfnEQFPostSeg2(pTAInput->szTagTableName,
                                          pTAInput->szEditName,
                                          pTAInput->szProgPath,
                                          chSrcFile,
                                          chTgtFile,
                                          pTAInput->TATag,
                                          pTAInput->hwndProcWin,
                                          &pTAInput->fKill );
              } /* endif */
#ifdef SESSIONLOG
              if ( !fOK  )
              {
                UtlLogWriteString( "TAFileProcessing: EQFPOSTSEG of user exit %s failed", pInD->pLoadedTable->pTagTable->szSegmentExit );
              } /* endif */                   
#endif
              if (fOK )
              {
                fOK = (TASegFileConvertASCII2Unicode( chTgtFile,
                                                      pTAInput->szSegTarget,
                                                      pInD->szDocSourceLang ) ) == 0;
                if (fOK)
                {
                  fOK = (TASegFileConvertASCII2Unicode( chSrcFile,
                                                        pTAInput->szSegSource,
                                                        pInD->szDocSourceLang)) == 0;
                }
                UtlDelete( chSrcFile, 0L, NOMSG );
                UtlDelete( chTgtFile, 0L, NOMSG );
              } /* endif */
            } /* endif */

            if (fOK)
               usPhase = EQFTA_COMPLETE;    // completed
            else if (!pTAInput->fKill)
            {
               PSZ pszDoc;
               pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
               usReturn = UtlErrorHwnd( ERROR_TA_SAVE_SEGFILE,  // replace error
                                     MB_YESNO,               // with a correct err
                                     1,
                                     &pszDoc,
                                     EQF_QUERY, pTAInput->hwndErrMsg );
               pTAInput->fKill = (usReturn == MBID_YES) ? FALSE : TRUE;
               usPhase = EQFTA_ABORT;
            } /* endif */

            break;

         case EQFTA_COMPLETE:
             // perform here completion processing
             // if segment processing is not needed: set analysis date
             if (!(pTAInput->fInsertTMMatches ||
                   pTAInput->fInsertToTM      ||
                   pTAInput->fInsertNewMatch  ||
                   pTAInput->fMachTrans       ||
                   pTAInput->fTMMatch         ||   // TM match counting
                   pTAInput->fNTLProcessing ))
             {
                ULONG  ulSegDate;                // date analyzed

                UtlTime( (PLONG)&ulSegDate );
                if ( !pTAInput->fSetSegDateDone ) SetSegDate (pTAInput, ulSegDate );
             } /* endif */
             {
               PUSHORT pusTemp;
               pusTemp = pInD->pusFileStatus +      // set status to false
                         (pInD->usCurNumSourceFile - 1);
               *pusTemp = TRUE;        // set file status to OK
             }

             /************************************************************/
             /* Add analysis record to history log                       */
             /************************************************************/
             {
               ANALYSISHIST AnalysisHist;       // history record for analysis

               AnalysisHist.fAddToMem        = pTAInput->fInsertToTM;
               AnalysisHist.fAutoReplace     = pTAInput->fInsertTMMatches;
               AnalysisHist.fLatestMatch     = pTAInput->fUseLatestTMMatch;
               AnalysisHist.fAutoJoin        = pTAInput->fAutoJoin;
               strcpy( AnalysisHist.szMemory, pTAInput->pInD->szDocMemory );
               strcpy( AnalysisHist.szMarkup, pTAInput->pInD->szDocFormat );

               EQFBWriteHistLog2( pTAInput->szFolder,
                                  pTAInput->pInD->pszCurSourceFile,
                                  ANALYSIS_LOGTASK,
                                  sizeof(ANALYSISHIST), (PVOID)&AnalysisHist,
                                  TRUE, pTAInput->hwndErrMsg,
                                  pTAInput->apszLongNames[pInD->usCurNumSourceFile-1] );

               // write history log with counted source words if no phase 2
               // follows
               if (!(pTAInput->fInsertTMMatches ||
                     pTAInput->fInsertToTM      ||
                     pTAInput->fInsertNewMatch  ||
                     pTAInput->fMachTrans       ||
                     pTAInput->fTMMatch         ||   // TM match counting
                     pTAInput->fNTLProcessing ))
               {
                 EQFBWriteHistLog2( pTAInput->szFolder,
                                    pTAInput->pInD->pszCurSourceFile,
                                    AUTOMATICSUBST_LOGTASK,
                                    sizeof(DocSaveHist), (PVOID)&DocSaveHist,
                                    TRUE, pTAInput->hwndErrMsg,
                                    pTAInput->apszLongNames[pInD->usCurNumSourceFile-1] );
               } /* endif */
               if (fOK )
               {
                 //R07197: it is nec. that STARGET file is in Unicode after segm.
                 // otherwise during 1st FileRead STARGET file is conv. to Unicode with
                 // cp of Target lang-! which may cause errors - i.e. if srclng is Jap
                 // but tgt lng is German!
                 USHORT sRc = NO_ERROR;
                 CHAR chTgtFile[MAX_EQF_PATH+1];
                 /******************************************************************/
                 /* create temp file names -- we have to use two different output  */
                 /******************************************************************/
                 if ( pTAInput->szSegTarget[0] == EOS )
                 {
                    sprintf(pTAInput->szSegTarget, PATHCATFILE,
                    pTAInput->szSEGTARGET_Path,
                    pInD->pszCurSourceFile);
                 }
                 strcpy( chTgtFile, pTAInput->szSegTarget);
                 strcat( chTgtFile, "A");
                 fOK = (TASegFileConvertASCII2Unicode( pTAInput->szSegTarget,
                                                       chTgtFile,
                                                       pInD->szDocSourceLang ) ) == 0;
                 sRc = UtlCopy( chTgtFile, pTAInput->szSegTarget, 1,  0L, TRUE );
                 UtlDelete( chTgtFile, 0L, NOMSG );
               }
             }
             usPhase =  EQFTA_FREE; // go ahead with free resources
             break;

         case EQFTA_FREE:
            // free allocated resources
            // insert processed file in done listbox
            if ( (pInD->usError != TAERROR) &&
                 (pTAInput->hwndErrMsg != HWND_FUNCIF) )
            {
               INSERTITEMHWND( pTAInput->hwndDone,
                         pTAInput->apszLongNames[pInD->usCurNumSourceFile-1] );
            } /* endif */
            // delete in any case processed file from todo listbox
            if ( pInD && (pTAInput->hwndErrMsg != HWND_FUNCIF) )

            {
              SHORT   sItem;                               // selected item

              sItem = SEARCHITEMHWND( pTAInput->hwndToDo,
                         pTAInput->apszLongNames[pInD->usCurNumSourceFile-1] );

              DELETEITEMHWND( pTAInput->hwndToDo, sItem );
              if (pInD->usError == TAERROR)
              {
                pInD->usError = TA_PREV_ERROR;
              } /* endif */
            } /* endif */
            usPhase =  EQFTA_DONE;                         // done: exit
            break;


         default:
         case EQFTA_ABORT:
            pInD->usError = TAERROR;
            DelSegFiles(pTAInput);       // close and delete segmented files
                                                                /* 2@KIT1322A */
            pInD->pusFileStatus[pInD->usCurNumSourceFile - 1] = FALSE;
            usPhase =  EQFTA_FREE;       // free resources
            break;

         } /* end task selection */

      // if user requested to stop the global variable kill is set
      // so return always TACOMPL

      *pusPhase = (USHORT) usPhase;

      return (TACOMPL);

   } /*-- end of TAFileProcessing ---*/


/*----------------------------------------------------------------------------*\
|  TACreate : perform global  initialization for Text Analysis                 |
|  Input    : input parameters structure pTAInput                              |
|  Output   : dictionaries are opened, tag tables are read,                    |
|             Tolstoy is initialized, memory areas are allocated....           |
|  pseudocode : see Low level Analysis document LPLLD.SCR                      |
\*----------------------------------------------------------------------------*/
static USHORT TACreate
(
  PTAINPUT pTAInput
)
{
   PTAINSTDATA     pInD;               // pointer to instance data
   BOOL            fOK = TRUE;         // Proessing status flag
   PSZ             pszFileName;        // work pointer
   LONG            lTemp;              // temporary long integer
   PPROPFOLDER     ppropFolder;        // pointer to folder properties
   ULONG           ulErrorInfo;        // error indicator from PRHA
   HPROP           hpropFolder;        // handle of folder properties


   pInD = pTAInput->pInD;              // store pointer to pInd temporarily

   // get no generic place flag from system properties
   {
     PPROPSYSTEM pSysProp;             // ptr to EQF system properties
     pSysProp = (PPROPSYSTEM) MakePropPtrFromHnd( EqfQuerySystemPropHnd());
     pTAInput->fNoGenericReplace = pSysProp->fNoGenericMarkup;
   }

   // load CRLF handling info data
   TALoadCRLFInfo( &(pInD->pCrlfLoadedInfo) );

   // general error indicator
   pInD->usError = TACOMPL;

   // load any analysis profile
   if ( fOK )
   {
     if ( pTAInput->fProfile && (pTAInput->szProfile[0] != EOS) )
     {
       fOK = AnaProfLoadProfile( pTAInput->szProfile, &(pTAInput->hProfile), pTAInput->hwndErrMsg );
       if ( !fOK )
       {
         pTAInput->fProfile = FALSE;
       } /* endif */
     } /* endif */

     // loop over all documents to get the combined analysis flag settings
     if ( fOK )
     {
       USHORT usCount = 0;
       PSZ pszCurDoc = (PSZ) pTAInput + pTAInput->stSourcefiles.ulOffset;

       for (usCount = 0; (usCount < pTAInput->stSourcefiles.usNumber); usCount++ )
       {
         CHAR szDocObjName[MAX_EQF_PATH];
         CHAR szFormat[MAX_FILESPEC];

         // build object name (folder concatenated with file )
         sprintf( szDocObjName, "%s\\%s", pTAInput->szFolder, pszCurDoc );

         // get document markup
         szFormat[0] = EOS;
         DocQueryInfo2( szDocObjName, NULL, szFormat, NULL, NULL, NULL, NULL, NULL, FALSE );

         // apply profile
         if ( szFormat[0] != EOS )
         {
           AnaProfApplyProfile( pTAInput->hProfile, pTAInput, szFormat, TRUE );
         } /* endif */

         // point to next file name, skipping null
         pszCurDoc += strlen(pszCurDoc) + 1;
       } /* endfor */
     } /* endif */
   } /* endif */

   if (fOK) // alloc for segment boundary table
   {
      fOK = UtlAllocHwnd( (PVOID *)&(pInD->SegBoundaryTbl.pFirstEntry),
                      0L,
                      (LONG)MAX_ALLOC_SEGTOKENS * (LONG)(sizeof (SEGBOUNDARY)),
                      ERROR_STORAGE, pTAInput->hwndErrMsg );

      pInD->SegBoundaryTbl.pCurEntry = pInD->SegBoundaryTbl.pFirstEntry;
      pInD->SegBoundaryTbl.lSize =
                      (LONG)MAX_ALLOC_SEGTOKENS * (LONG)(sizeof (SEGBOUNDARY)),
      pInD->SegBoundaryTbl.usNumCurEntry = (USHORT) MAX_NUM_SEGTOKENS;
   }

   /*******************************************************************/
   /* Allocate buffer for morphologic processing                      */
   /*******************************************************************/
   if ( fOK )
   {
      fOK = UtlAllocHwnd( (PVOID *)&(pInD->TolstControl.pTextBuffer),
                      0L,
                      (LONG) BUFFERSIZE * sizeof(CHAR_W),
                      ERROR_STORAGE, pTAInput->hwndErrMsg );
   } /* endif */

   if (fOK)                               // allocate memory for Tokenlist
   {
      fOK = UtlAllocHwnd( (PVOID *)&(pInD->pTokenList),
                      0L,
                      (LONG) MAX_ALLOC_TOKENS * (LONG)(sizeof (TOKENENTRY)),
                      ERROR_STORAGE, pTAInput->hwndErrMsg );
   } /* endif */

   if (fOK)    // allocate file status array
   {
      lTemp = (LONG) max( pTAInput->stSourcefiles.usNumber * (sizeof (USHORT)),
                          MIN_ALLOC);
      fOK = UtlAllocHwnd( (PVOID *)&(pInD->pusFileStatus), 0L, lTemp, ERROR_STORAGE,
                          pTAInput->hwndErrMsg );
   } /* endif */

   if (fOK)    // allocate segmenting tokenlist
   {
      fOK = UtlAllocHwnd( (PVOID *)&(pInD->pSegTokenList),
                      0L,
                      (LONG) MAX_ALLOC_SEGTOKENS * (LONG)(sizeof (TOKENENTRYSEG)),
                      ERROR_STORAGE, pTAInput->hwndErrMsg );
   } /* endif */

   if (fOK)
   {  // if translation memory needed
      if (pTAInput->fInsertTMMatches ||
          pTAInput->fInsertToTM      ||
          pTAInput->fTMMatch         ||   // TM match counting
          pTAInput->fRedundCount     ||   // redundancy counting
          pTAInput->fMachTrans       ||
          pTAInput->fInsertNewMatch)
         fOK = StartTM (pTAInput, pInD);
   } /* endif */

   if ( fOK && pTAInput->fRedundCount )
   {
   } /* endif */

   if ( fOK )
   {
      // get program path
      UtlMakeEQFPath( pTAInput->szProgPath, NULC, PROGRAM_PATH, NULL );

      /****************************************************************/
      /* Get editor name from folder properties if editor name has    */
      /* not been set yet                                             */
      /****************************************************************/
      if ( pTAInput->szEditName[0] == EOS )
      {
        //--- open folder properties
        if( ( hpropFolder = OpenProperties  // error open properties
                             ( pTAInput->szFolderProp, NULL,
                               PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
        {
           // display error message if not already displayed
           if ( ulErrorInfo != Err_NoStorage )
           {
             pszFileName = pTAInput->szFolder;
             UtlErrorHwnd( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1,
                       &pszFileName, EQF_ERROR, pTAInput->hwndErrMsg );
           } /* endif */
           // set fOk to FALSE
           fOK = FALSE;
        }
        else
        {
           //--- get pointer to folder properties
           ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( hpropFolder );
           //--- get editor name from folder properties
           strcpy(pTAInput->szEditName, ppropFolder->szEditor); // name of editor property
           //--- close folder properties


           // clear MT fields in folder properties and in our inernal data structure
           if ( SetPropAccess( hpropFolder, PROP_ACCESS_WRITE) )
           {
             ppropFolder->fMTFieldsFilled = FALSE;
             ppropFolder->fMTReceived = FALSE;
             memset( ppropFolder->ulMTTotalWords, 0, sizeof(ppropFolder->ulMTTotalWords) );
             memset( ppropFolder->ulMTTotalSegs, 0, sizeof(ppropFolder->ulMTTotalSegs) );
             memset( ppropFolder->ulMTSendWords, 0, sizeof(ppropFolder->ulMTSendWords) );
             memset( ppropFolder->ulMTSendSegs, 0, sizeof(ppropFolder->ulMTSendSegs) );
             memset( ppropFolder->ulMTReceivedWords, 0, sizeof(ppropFolder->ulMTReceivedWords) );
             memset( ppropFolder->ulMTReceivedSegs, 0, sizeof(ppropFolder->ulMTReceivedSegs) );

             SaveProperties( hpropFolder, &ulErrorInfo );
             ResetPropAccess( hpropFolder, PROP_ACCESS_WRITE );
           } /* endif */

           memset( pTAInput->ulMTTotalWords, 0, sizeof(pTAInput->ulMTTotalWords) );
           memset( pTAInput->ulMTTotalSegs, 0, sizeof(pTAInput->ulMTTotalSegs) );
           memset( pTAInput->ulMTSendWords, 0, sizeof(pTAInput->ulMTSendWords) );
           memset( pTAInput->ulMTSendSegs, 0, sizeof(pTAInput->ulMTSendSegs) );

           CloseProperties( hpropFolder, 0, &ulErrorInfo);
        } /* endif NO ERROR */
      } /* endif */
   } /* endif */

   if (fOK && pTAInput->fNTLProcessing)
      fOK = LPInit(pTAInput, (PLPDATA *)&(pInD->pLPDATA));

   if ( fOK )
   {
    fOK = TALockFiles ( pTAInput );       // lock the files
   } /* endif */

   if ( fOK && pTAInput->fUseReplacementList )
   {
     fOK = DITALoadReplacementList( pTAInput->szReplacementList, &(pTAInput->pvReplacementList) );
   } /* endif */

   return (fOK ? TACOMPL : TAERROR);
} /* end of TACreate */


/*----------------------------------------------------------------------------*\
| TAOpenFiles: open input the source file, output the segm.source              |
| input     : pointer to the data area                                         |
| output    : files are opened ; return TRUE if everything OK                  |
\*----------------------------------------------------------------------------*/
static USHORT TAOpenFiles (PTAINPUT pTAInput, PSZ pFile)
   {
   PTAINSTDATA    pInD;
   CHAR           szFileName[MAX_EQF_PATH];  // full name of file to be opended
   USHORT         usAction;                  // used for UtlOpen
   USHORT         fOK;                       // flag for operation
   USHORT         usReturn;                  //

   pInD = pTAInput->pInD;

   // open input source file
   usReturn = UtlOpen(pFile,
                      &(pInD->hSource),      // handle to source file
                      &usAction,
                      0L,
                      FILE_NORMAL,          // Normal attribute
                      OPEN_ACTION_OPEN_IF_EXISTS,
                      OPEN_SHARE_DENYWRITE, // Deny Write access
                      0L,                   // Reserved
                      FALSE );              // no error handling
   fOK = ( !usReturn );
#ifdef SESSIONLOG
   if ( !fOK  )
   {
     UtlLogWriteString( "TAOpenFiles: UtlOpen of %s failed", pFile );
   } /* endif */                   
#endif

   if (fOK)   // get file status, length
   {

        DWORD dwSizeHigh;

      pInD->fFirstRead = TRUE;

      pInD->ulFileSize = (ULONG)GetFileSize( pInD->hSource, &dwSizeHigh );


      fOK = (USHORT)(pInD->ulFileSize > 0);
   }

    if (fOK && (*pTAInput->szSEGSOURCE_Path)) // open segmented source
       {
       sprintf( szFileName,
                PATHCATFILE,
                pTAInput->szSEGSOURCE_Path,
                pInD->pszCurSourceFile);

       usReturn = UtlBufOpen( &(pInD->CBSegSource), szFileName,
                              BUFFERSIZE, FILE_CREATE, FALSE );
       fOK = ( !usReturn );
       pInD->fFirstWrite = TRUE;
       }

   if (fOK)       // open segmented target
      {
      sprintf( szFileName,    // create full file name
               PATHCATFILE,
               pTAInput->szSEGTARGET_Path,
               pInD->pszCurSourceFile);

       usReturn = UtlBufOpen( &(pInD->CBSegTarget), szFileName,
                              BUFFERSIZE, FILE_CREATE, FALSE );
      fOK = ( !usReturn );
      }

    if (!fOK)
       {
         // opening of source or target file failed
         //pop up message box and ask user whether to continue
         BOOL fEmptyFile = FALSE;
         PSZ pszDoc;

         // check if source document is empty and use different error message for this case
         {
           FILE *hf;
           sprintf( szFileName, PATHCATFILE, pTAInput->szSOURCE_Path, pInD->pszCurSourceFile );
           hf = fopen( szFileName, "r" );
           if ( hf )
           {
             LONG len = filelength(fileno(hf));
             if ( len == 0 )
               fEmptyFile = TRUE;
             fclose( hf );
           } /* endif */
         }


       pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
#ifdef SESSIONLOG
       UtlLogWriteString( "TAOpenFiles: Open of source or target segmented file of document %s failed", pszDoc);
#endif
       usReturn = UtlErrorHwnd( fEmptyFile ? ERROR_TA_EMPTYFILE : ERROR_TA_SOURCEFILE,
                             MB_YESNO,
                             1,
                             &pszDoc,
                             EQF_QUERY, pTAInput->hwndErrMsg );

       pTAInput->fKill = (usReturn != MBID_YES);  // go to stop processing
       }
    /*----------- Init memory areas and control data --------------------*/
    if (fOK)
       {
       pInD->SegmentControl.ulSegmentcount = 1;
       pInD->ulSegNum = 1;
       pInD->SegmentControl.Seginfo = SEG;
       pInD->SegmentControl.SegType = LINGUISTIC;
       pInD->SegmentControl.sSegState = 1;
       pInD->SegmentControl.usColPos = 0;
       pInD->fSeglength = FALSE;
       pInD->uCurTextPos = 0;
       pInD->fAll = FALSE;
       pInD->pRest = NULL;
       }

   return (fOK);
   } /* end  open files  */


/*----------------------------------------------------------------------------*\
| TACloseFiles: close files: input file, segm source                           |
| input     : pointer to the data area                                         |
| output    : closes the files; return TRUE if everything OK                   |
\*----------------------------------------------------------------------------*/
static USHORT TACloseFiles (PTAINPUT pTAInput)
   {
   PTAINSTDATA    pInD;
   USHORT         fOK = TRUE;                       // flag for operation
   CHAR           szFileName[MAX_EQF_PATH];  // full name of file to be opended

   pInD = pTAInput->pInD;

   if (*(pTAInput->szSEGSOURCE_Path))
   {
   //  if ( pInD->pLoadedTable->usCharacterSet == TAGCHARSET_UNICODE )
   //  {
      // close sequence output for segm source
     fOK = ( UtlBufWriteHwnd( pInD->CBSegSource,
                        szCloseSeqUnicode, UNICODECLOSESEQLEN,
                        TRUE, pTAInput->hwndErrMsg ) == NO_ERROR );

      // close sequence output for segm target
      fOK &= ( UtlBufWriteHwnd( pInD->CBSegTarget,
                        szCloseSeqUnicode, UNICODECLOSESEQLEN,
                        TRUE, pTAInput->hwndErrMsg ) == NO_ERROR );
  //   }
  //   else
  //   {
      // close sequence output for segm source
  //    fOK = ( UtlBufWriteHwnd( pInD->CBSegSource,
  //                      szCloseSeq,
  //                      (USHORT)strlen(szCloseSeq),
  //                      TRUE, pTAInput->hwndErrMsg ) == NO_ERROR );

      // close sequence output for segm target
  //    fOK = ( UtlBufWriteHwnd( pInD->CBSegTarget,
  //                      szCloseSeq,
  //                      (USHORT)strlen(szCloseSeq),
  //                      TRUE, pTAInput->hwndErrMsg ) == NO_ERROR );

  //   } /* endif */
  }

    // to be performed also in case of errors
    TACLOSE( pInD->hSource );                    // Close Source file
    if ( pInD->CBSegSource )
    {
      UtlBufClose( pInD->CBSegSource, FALSE );
      pInD->CBSegSource = NULL;
    } /* endif */
    if ( pInD->CBSegTarget )
    {
      UtlBufClose( pInD->CBSegTarget, FALSE );
      pInD->CBSegTarget = NULL;
    } /* endif */

    // if source file different from temp file delete temp file,
    // otherwise user might get problems ....
    sprintf( szFileName,                     // create full file name
             PATHCATFILE,
             pTAInput->szSOURCE_Path,
             pInD->pszCurSourceFile);
    if ( (pTAInput->szTempName[0] != EOS) &&
         (strcmpi( pTAInput->szTempName, szFileName ) != 0) )
    {
       UtlDelete(pTAInput->szTempName, 0L, FALSE); //delete temp file
    } /* endif */

    /******************************************************************/
    /* delete the SNOMATCH file -- we have newly analyzed             */
    /******************************************************************/
    if ( fOK )
    {
      sprintf( szFileName,                     // create full file name
               PATHCATFILE,
               pTAInput->szFolderProp,
               pInD->pszCurSourceFile);
      EqfSend2Handler( MTLISTHANDLER, WM_EQFN_DELETED,
                       MP1FROMSHORT( clsMTLIST ), MP2FROMP(szFileName) );

    } /* endif */
	
    // delete the METADATA of the document
    if ( fOK )
    {
      UtlMakeEQFPath( szFileName, pTAInput->szFolderProp[0], METADATA_PATH, UtlGetFnameFromPath(pTAInput->szFolderProp) );
      strcat( szFileName, BACKSLASH_STR );
      strcat( szFileName, pInD->pszCurSourceFile );
      UtlDelete( szFileName, 0L, FALSE ); 
    } /* endif */


    return (fOK ? TACOMPL: TAERROR);
    }

/*----------------------------------------------------------------------------*\
| TACopyTemp                                                                   |
| since the user requested not to perform standard segmentation, copy the file |
| produced by the user (temp) to both segm Source and segm Target              |
| input  : pTAInput: uses SEGSOURCE_Path, SEGTARGET_Path, szTempName           |
|          pInD: uses pszCurSourceFile                                         |
| output : copies the temporary file; returns TACOMPL if OK                    |
\*----------------------------------------------------------------------------*/
static USHORT   TACopyTemp (PTAINPUT pTAInput)
   {
   PTAINSTDATA    pInD;
   CHAR           szFileName[MAX_EQF_PATH];  // full name of file to be opended
   USHORT         usDosRc;
   USHORT         usReturn;

   pInD = pTAInput->pInD;
   /*--------- Copy temp file to seg Source and Seg Target ------*/
   sprintf( szFileName,                  // create full file name
           PATHCATFILE,                 // for segmented source
           pTAInput->szSEGSOURCE_Path,
           pInD->pszCurSourceFile);

   usDosRc = UtlCopy( pTAInput->szTempName,   // copy temp file into
                      szFileName,             // ssource
                      DCPY_EXISTING,            // copy even if exist
                      0L,                       // reserved value
                      FALSE);

   if (!usDosRc)  // if ok, continue
   {
      sprintf( szFileName,                  // create full file name
              PATHCATFILE,                 // for segmented source
              pTAInput->szSEGTARGET_Path,
              pInD->pszCurSourceFile);

      usDosRc = UtlCopy( pTAInput->szTempName,   // copy temp file into
                       szFileName,               // starget
                       DCPY_EXISTING,            // copy even if exist
                       0L,                       // reserved value
                       FALSE);
   } /* endif */

   if (usDosRc)  // copying of source or target failed
   {
      PSZ pszDoc;
      DelSegFiles(pTAInput);   // delete segmented files
      //pop up message box and ask user whether to continue
      pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
#ifdef SESSIONLOG
      UtlLogWriteString( "TACopyTemp: Error copying temp file %s", pTAInput->szTempName );
#endif
      usReturn = UtlErrorHwnd( ERROR_TA_SOURCEFILE,
                            MB_YESNO,
                            1,
                            &pszDoc,
                            EQF_QUERY, pTAInput->hwndErrMsg );

      pTAInput->fKill = (usReturn != MBID_YES);  // go to stop processing
   } /* endif */

   // if source file different from temp file delete temp file,
   // otherwise user might get problems ....
   sprintf( szFileName,                     // create full file name
            PATHCATFILE,
            pTAInput->szSOURCE_Path,
            pInD->pszCurSourceFile);
   if ( strcmpi(pTAInput->szTempName, szFileName) )
   {
      UtlDelete(pTAInput->szTempName, 0L, FALSE); //delete temp file after copying
   } /* endif */

   return ((!usDosRc) ? TACOMPL: TAERROR);
   } /*--- end of TACopyTemp ---*/


/*----------------------------------------------------------------------------*\
| TABlockProcessing : process next text block of file                          |
|                                                                              |
|                                                                              |
| input  :                                                                     |
| output :                                                                     |
| pseudocode :                                                                 |
|   BEGIN                                                                      |
|         call TAReadBlock                                                     |
|         call text segmentation for current block (EQFTextSegmentation)       |
|         IF errors THEN                                                       |
|            Pop up dialog with message and ask user whether continue or not   |
|            IF user ask NOT to continue THEN                                  |
|               fKill = TRUE                                                   |
|            END                                                               |
|         ELSE                                                                 |
|            Update slider position                                            |
|         ENDIF                                                                |
|         RETURN fOK                                                           |
|   END                                                                        |
|                                                                              |
\*----------------------------------------------------------------------------*/
static USHORT TABlockProcessing( HWND hwnd, PTAINPUT pTAInput )
   {
   BOOL fOK= TRUE;                    // processing flag
   PTAINSTDATA pInD;                  // pointer to instance data
   USHORT usReturn;                   // return value
   ULONG  ulFilePosition;             // current position in source file

   pInD = pTAInput -> pInD ;  // store pointer to instance data temp.

   fOK = TAReadBlock (pTAInput);

   if (fOK)
      {
      // call text segmentation for the text block
      fOK =  EQFTextSegmentation( hwnd,
                                  pInD,
                                  &(pInD->pRest),
                                  &(pInD -> pSegBoundaryList));

      if ( !fOK )
         {
        PSZ pszDoc;
        pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
         usReturn = UtlErrorHwnd( ERROR_TA_SEGMENT,
                               MB_YESNO,
                               1,
                               &pszDoc,
                               EQF_QUERY, pTAInput->hwndErrMsg );
         pTAInput->fKill = (usReturn != MBID_YES); // do not continue
#ifdef SESSIONLOG
         UtlLogWriteString( "TABlockProcessing: EqfTextSegmentation of document %s failed", pszDoc );
#endif
         }
      else
      {
        UtlChgFilePtr( pInD->hSource, 0L, 1, &ulFilePosition, FALSE);
        pTAInput->sSliderPos = (SHORT)((LONG)(ulFilePosition * 100L / pInD->ulFileSize));

         if ( !pTAInput->fBatch )
         {
           WinSendMsg( pTAInput->hwndProcWin, WM_EQF_UPDATESLIDER,
                       MP1FROMSHORT(pTAInput->sSliderPos), NULL );
         } /* endif */
      }
   }
   return ((USHORT)fOK);
}  /* end of TABlockProcessing */

/*----------------------------------------------------------------------------*\
| TASaveSegFile : write a block of segmented data to Segm Source and Segm      |
|                 target                                                       |
| input         :                                                              |
| output        :                                                              |
\*----------------------------------------------------------------------------*/
static USHORT TASaveSegFile ( PTAINPUT pTAInput )           // create source seg file
   {
   BOOL fOK= TRUE;                     // processing flag
   USHORT usLength;                    // length of current segment
   PSEGBOUNDARY pCurSegBnd;            // segment boundary
   PTAINSTDATA pInD=(PTAINSTDATA) pTAInput->pInD; // ptr to instance data
   USHORT usReturn;                    // return from UtlError
   BOOL   fNoTranslatableInfo;         // TRUE = segment contains not translatable
                                       // information
   USHORT      usI;                    // loop index
   PTOKENENTRY pToken;                 // ptr for token list processing
   USHORT      usColPos = 0;
   CHAR_W      chTemp;

   PTAGTABLE  pTable = pInD->pLoadedTable->pTagTable;
   SHORT      sNumTags = (SHORT)pTable->uNumTags;
   PBYTE      pByte = (PBYTE)pTable;
   PTAG       pTag = OFFSETTOPOINTER(PTAG, pTable->stFixTag.uOffset );
   PATTRIBUTE pAttribute = OFFSETTOPOINTER(PATTRIBUTE, pTable->stAttribute.uOffset);

   pCurSegBnd = pInD->pSegBoundaryList->pNext;

   while ( (pCurSegBnd->pNext != NULL) && fOK )
   {
#ifdef TSEGM_SEGLOGGING
     if ( hSegLog )
     {
       fwprintf( hSegLog, L"Seg %8lu: State = %s ", pInD->SegmentControl.ulSegmentcount,
                ( pCurSegBnd->XlateState == TODO ) ? L"ToDo" : L"Nop" );
     } /* endif */
#endif
     usLength = (USHORT) ((PSZ_W) pCurSegBnd->pNext->pBoundary
                            - (PSZ_W) pCurSegBnd->pBoundary);

     TASaveSegTagTok(pCurSegBnd, pInD, usLength, usColPos);

     /*****************************************************************/
     /* For translatable segments check if segment contains data      */
     /*****************************************************************/
     if ( pCurSegBnd->XlateState == TODO )
     {
       int iTranslVar = 0;           // number of translatable variables in segment
       fNoTranslatableInfo = TRUE;

       /***************************************************************/
       /* Check if text tokens contain translatable information       */
       /***************************************************************/
       pToken = pInD->pTokenList;
       while ( (pToken->sTokenid != ENDOFLIST) && fNoTranslatableInfo )
       {
         if ( pToken->sTokenid == TEXT_TOKEN )
         {
           for ( usI = 0;
                 ( (usI < pToken->usLength) && fNoTranslatableInfo );
                 usI++)
           {
             if ( IsDBCS_CP(pInD->TolstControl.ulOemCP) &&
                  EQFIsDBCSChar(pToken->pDataStringW[usI], pInD->TolstControl.ulOemCP) )
             {
               fNoTranslatableInfo = FALSE;
             }
             // first check using Windows API
             else if ( iswalpha(*(pToken->pDataStringW+usI))
                    || iswdigit(*(pToken->pDataStringW+usI)) )
             {
               fNoTranslatableInfo = FALSE;
             }
             // when try our old apptoach using the hardcoded table which does
             // not care for the active codepage and which is not enabled for
             // UNICODE...
             // TODO: provide a UNICODE enabled table or find a different approach
             //       for this table!!!
             else if ( isTranslatable[(UCHAR)(pToken->pDataStringW[usI])] )
             {
               fNoTranslatableInfo = FALSE;
             } /* endif */
           } /* endfor */
         }
         else if ( pToken->ClassId == CLS_TRANSLVAR )
         {
           iTranslVar++;
         } /* endif */
         pToken++;
       } /* endwhile */

       // change segment type if no translatable data was found and up to one
       // translatable variable is contained in the data
       if ( fNoTranslatableInfo && (iTranslVar <= 1) )
       {
         pCurSegBnd->XlateState = NOP;
#ifdef TSEGM_SEGLOGGING
        if ( hSegLog )
        {
          fwprintf( hSegLog, L", changed to Nop (fNoTranslatableInfo)" );
        } /* endif */
#endif
       } /* endif */
     } /* endif */

     /*****************************************************************/
     /* For non-translatable segments check if segment contains       */
     /* attributes with inline text                                   */
     /*****************************************************************/
     if ( pCurSegBnd->XlateState == NOP )
     {
       /***************************************************************/
       /* Check if attributes have translatable data                  */
       /***************************************************************/
       pToken = pInD->pTokenList;
       while ( (pToken->sTokenid != ENDOFLIST) &&
               (pCurSegBnd->XlateState == NOP) )
       {
         if ( ( (pToken->sTokenid >= sNumTags) &&
                (pAttribute[pToken->sTokenid - sNumTags].BitFlags.fTranslate) ) ||
              ( (pToken->sTokenid >= 0)       &&
                (pToken->sTokenid < sNumTags) &&
                (pTag[pToken->sTokenid].BitFlags.fTranslate) ) )
         {
           pCurSegBnd->XlateState = pInD->fProtectMode ? NOP : ATTRTODO;
         } /* endif */
         pToken++;
       } /* endwhile */
     } /* endif */
#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        if ( pCurSegBnd->XlateState == ATTRTODO )
        {
          fwprintf( hSegLog, L", changed to Todo" );
        } /* endif */
      } /* endif */
#endif
     usLength = EQFMergeTRNote(pInD, pCurSegBnd, usLength, sNumTags, pTag,
                               pInD->pLoadedTable->chTrnote1TextW, pInD->pLoadedTable->chTrnote2TextW,
                               pInD->pLoadedTable->ulTRNote1Len, pInD->pLoadedTable->ulTRNote2Len);
     if ( pCurSegBnd->XlateState != NOP )
     {
       pInD->ulSegsNotReplaced++;
     } /* endif */
     pInD->ulTotalSegs++;

#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        CHAR_W chTemp = pCurSegBnd->pBoundary[usLength];
        pCurSegBnd->pBoundary[usLength] = 0;
        fwprintf( hSegLog, L"\n" );
        fwprintf( hSegLog, L"Value=\"%s\"\n", pCurSegBnd->pBoundary );
        pCurSegBnd->pBoundary[usLength] = chTemp;
      } /* endif */
#endif

     fOK =  WriteSegmentW(pTAInput,
                         usLength,
                         pCurSegBnd->pBoundary,
                         pCurSegBnd->XlateState,
                         pInD->CBSegSource,
                         pInD->SegmentControl.ulSegmentcount,
                         FALSE);
#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        fwprintf( hSegLog, L"Seg out (SRC) rc=%d\n",fOK);
      } /* endif */
#endif
     if ( !fOK )
     {
      PSZ pszDoc;
      EVENTLOGFILE();

      pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];

      OPENEVENTLOG( "EQFTSEGM" );
      WRITE2EVENTLOG( "WriteSegmentW for source file failed, doc is %s", pszDoc );
      CLOSEEVENTLOG();
     } /* endif */        

     if (fOK)
     {
        fOK =  WriteSegmentW(pTAInput,
                            usLength,
                            pCurSegBnd->pBoundary,
                            pCurSegBnd->XlateState,
                            pInD->CBSegTarget,
                            pInD->SegmentControl.ulSegmentcount,
                            FALSE);
#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        fwprintf( hSegLog, L"Seg out (TGT) rc=%d\n",fOK);
      } /* endif */
#endif

        if ( !fOK )
        {
          PSZ pszDoc;
          EVENTLOGFILE();

          pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];

          OPENEVENTLOG( "EQFTSEGM" );
          WRITE2EVENTLOG( "WriteSegmentW for target file failed, doc is %s", pszDoc );
          CLOSEEVENTLOG();
        } /* endif */        
        // if no phase 2 follows, we have to count the source words of the
        // segment
        if ( !( pTAInput->fInsertTMMatches ||    // autom trans.
                pTAInput->fInsertToTM      ||    // update TM
                pTAInput->fInsertNewMatch  ||    // new match output
                pTAInput->fTMMatch         ||   // TM match counting
                pTAInput->fRedundCount     ||   // redundancy counting
                pTAInput->fMachTrans       ||
                pTAInput->fNTLProcessing ) )     // list processing
        {
          USHORT usSrcWords  = 0;

          // count source words of segment
          {
            ULONG ulSrcWords  = 0L;
            ULONG ulSrcMarkUp = 0L;
            USHORT  usRc;

            // convert segment data to null terminated string
            chTemp = pCurSegBnd->pBoundary[usLength];
            pCurSegBnd->pBoundary[usLength] = EOS;
#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        fwprintf( hSegLog, L"Counting words...[%s]\n",pCurSegBnd->pBoundary);
      } /* endif */
#endif

            // count words
            usRc = EQFBWordCntPerSeg( pInD->pLoadedTable,
                                      (PTOKENENTRY)&TempTokBuf,
                                      pCurSegBnd->pBoundary,          //unicode string!
                                      pInD->TolstControl.sLangID,
                                      &ulSrcWords, &ulSrcMarkUp,
                                      pInD->TolstControl.ulOemCP);

#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        fwprintf( hSegLog, L"Counted words...rc=%d   words=%ld\n",usRc,ulSrcWords);
      } /* endif */
#endif
            // restore segment end character
            pCurSegBnd->pBoundary[usLength] = chTemp;

            // remember number of source words
            usSrcWords  = (USHORT)ulSrcWords;
          }

          // add source words to approbriate criteria sum
          if (usSrcWords < SIMPLE_SENT_BOUND )
          {
            DocSaveHist.NotXlated.SimpleSum.usNumSegs++;
            DocSaveHist.NotXlated.SimpleSum.ulSrcWords += (ULONG) usSrcWords;
          }
          else if (usSrcWords < MEDIUM_SENT_BOUND )
          {
            DocSaveHist.NotXlated.MediumSum.usNumSegs++;
            DocSaveHist.NotXlated.MediumSum.ulSrcWords += (ULONG) usSrcWords;
          }
          else
          {
            DocSaveHist.NotXlated.ComplexSum.usNumSegs++;
            DocSaveHist.NotXlated.ComplexSum.ulSrcWords += (ULONG) usSrcWords;
          } /* endif */
        } /* endif */
     } /* endif */
#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        fwprintf( hSegLog, L"Counted words...len=%d\n",usLength);
      } /* endif */
#endif
     if (usLength)
     {// set column for next segment
		 chTemp = pCurSegBnd->pBoundary[usLength-1];
		 if (chTemp == CR_W || chTemp == LF_W)
		 {
			 usColPos = 0;
	     }
	     else
	     { // current restriction: support only tags with COLPOSITION 1 correctly
			 usColPos = 1;
	     }
     }
     pCurSegBnd = pCurSegBnd->pNext;                 // point to next segment
     pInD->SegmentControl.ulSegmentcount ++;         // increase segment number
     } /* endwhile */

#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        fwprintf( hSegLog, L"Counted words...rc=%d\n",fOK);
      } /* endif */
#endif

   if (!fOK)
      {
        PSZ pszDoc;
        pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
      usReturn = UtlErrorHwnd( ERROR_TA_SAVE_SEGFILE,
                             MB_YESNO,
                             1,
                             &pszDoc,
                             EQF_QUERY, pTAInput->hwndErrMsg );
      pTAInput->fKill = (usReturn != MBID_YES);  // user stop req
#ifdef SESSIONLOG
      UtlLogWriteString( "TASaveSegFile: save of document %s failed", pszDoc );
#endif
      }


   return ((USHORT)fOK);
   } /* end of TASaveSegFile */

static VOID TASaveSegTagTok
(
  PSEGBOUNDARY   pActSegBnd,
  PTAINSTDATA    pInD,
  USHORT         usLength,
  USHORT         usColPos
)
{
  PSZ_W   pRest = NULL;
  CHAR_W  chTemp;
  /***************************************************************/
  /* Tokenize current segment                                    */
  /***************************************************************/
  pRest    = NULL;
  chTemp = pActSegBnd->pBoundary[usLength];
  pActSegBnd->pBoundary[usLength] = EOS;
  TATagTokenizeW( pActSegBnd->pBoundary,
                 pInD->pLoadedTable,
                 TRUE,
                 &pRest,
                 &usColPos,
                 pInD->pTokenList,
                 MAX_ALLOC_TOKENS );
  pActSegBnd->pBoundary[usLength] = chTemp;
  return;
} /* end of TASaveSegTagTok */

static USHORT EQFMergeTRNote
(
  PTAINSTDATA    pInD,
  PSEGBOUNDARY   pActSegBnd,
  USHORT         usLength,
  SHORT          sNumTags,
  PTAG           pTag,
  PSZ_W          pszTRNote1,
  PSZ_W          pszTRNote2,
  ULONG          ulTRNote1Len,
  ULONG          ulTRNote2Len
)
{
  USHORT        usTotalLen = usLength;
  BOOL          fTRNotePending = FALSE;
  PTOKENENTRY   pToken;
  SHORT         sTokID;
  PSEGBOUNDARY  pSegNext;
  USHORT        usLenNext;

  // loop until end of seg or TRNotePending
  pToken = pInD->pTokenList;
  sTokID = pToken->sTokenid;
  while ((sTokID != ENDOFLIST) && !fTRNotePending)
  {
    // check whether the word following the tag with Bitflag fTRNote is
    // the identifier that it is a "TRNOTE" comment, not another one
    if ((sTokID >= 0) && (sTokID < sNumTags)
         && pTag[sTokID].BitFlags.fTRNote)
    {
      fTRNotePending = (memicmp( pToken->pDataStringW+pToken->usLength,
                                 pszTRNote1,
                                 ulTRNote1Len ) == 0);
      if (!fTRNotePending)
      {
        fTRNotePending = (memicmp( pToken->pDataStringW+pToken->usLength,
                                   pszTRNote2,
                                   ulTRNote2Len ) == 0);
      }

    } /* endwhile */
    pToken++;
    sTokID = pToken->sTokenid;
  } /* endhwile */
  // if TRNote found in segment, loop til end of note and merge
  while (fTRNotePending)
  {
    if (sTokID == ENDOFLIST )
    {
      pSegNext = pActSegBnd->pNext;
      if ( pSegNext && pSegNext->pNext )
      {
        usLenNext = (USHORT) ( pSegNext->pNext->pBoundary - pSegNext->pBoundary);
        usTotalLen = usTotalLen + usLenNext;                // merge length
        pActSegBnd->pNext = pSegNext->pNext;    // merge by omitting one
        // no special handling for usColpos based tags inside TRNotes!!
        TASaveSegTagTok(pSegNext, pInD, usLenNext, 0);
        pToken = pInD->pTokenList;
        sTokID = pToken->sTokenid;
      }
      else
      {
        fTRNotePending = FALSE;
      } /* endif */
    } /* endif */
    while ((sTokID != ENDOFLIST) && fTRNotePending)
    {
      if ((sTokID >= 0) && (sTokID < sNumTags)
          && pTag[sTokID].BitFlags.fTRNote)
      {
        fTRNotePending = FALSE;
      } /* endif */
      pToken++;
      sTokID = pToken->sTokenid;
    } /* endhwile */
  } /* endwhile */
  return(usTotalLen);
} /* end of TASaveSegTagTok */


/*----------------------------------------------------------------------------*\
|  TATerminate:   Text Analysis         : free resources allocated             |
|  terminate process of TOLSTOY, terminate Translation memory, free user DLL   |
|  frees allocated areas                                                       |
|  Input  : pointers to instance data areas, window                            |
|  Ouput  : none, all allocated areas are freed                                |
|  Return : VOID                                                               |
\*----------------------------------------------------------------------------*/
static USHORT TATerminate (HWND hwnd, PTAINPUT pTAInput)
   {
   PTAINSTDATA   pInD;                 // pointer to instance data for TA
   BOOL          fNoRemoveObject;      // buffer for flag from TAInput

   pInD = pTAInput->pInD;
   fNoRemoveObject = pTAInput->fNoRemoveObject;

   if ( pInD )
   {
      // update folder properties with values from MT tracking fields
      if ( pTAInput->fMTValuesAvailable )
      {
        ULONG ulErrorInfo = 0;
        HPROP hpropFolder = OpenProperties( pTAInput->szFolderProp, NULL, PROP_ACCESS_READ, &ulErrorInfo );
        if ( hpropFolder != NULL )
        {
          if ( SetPropAccess( hpropFolder, PROP_ACCESS_WRITE) )
          {
            int i;
            PPROPFOLDER pProp = (PPROPFOLDER) MakePropPtrFromHnd( hpropFolder );
            for( i = 0; i < 3; i++ )
            {
              pProp->ulMTTotalWords[i] = pTAInput->ulMTTotalWords[i];
              pProp->ulMTTotalSegs[i]  = pTAInput->ulMTTotalSegs[i];
              pProp->ulMTSendWords[i]  = pTAInput->ulMTSendWords[i];
              pProp->ulMTSendSegs[i]   = pTAInput->ulMTSendSegs[i];
            } /* endfor */
            pProp->fMTFieldsFilled = TRUE;

            SaveProperties( hpropFolder, &ulErrorInfo );
            ResetPropAccess( hpropFolder, PROP_ACCESS_WRITE );
          } /* endif */
          CloseProperties( hpropFolder, 0, &ulErrorInfo);
        } /* endif NO ERROR */
      } /* endif */

      /****************************************************************/
      /* Free any active language ID                                  */
      /****************************************************************/
      if ( pInD->szCurSourceLang[0] != EOS )
      {
        MorphFreeLanguageID( pInD->TolstControl.sLangID );
      } /* endif */

      /****************************************************************/
      /* Free token list and text buffer required for morphologic     */
      /* processing                                                   */
      /****************************************************************/
      if ( pInD->TolstControl.pTokenList )
      {
        UtlAlloc( (PVOID *) &(pInD->TolstControl.pTokenList), 0L, 0L, NOMSG);
      } /* endif */

      if ( pInD->TolstControl.pTextBuffer )
      {
        UtlAlloc( (PVOID *) &(pInD->TolstControl.pTextBuffer), 0L, 0L, NOMSG);
      } /* endif */

      /****************************************************************/
      /* Free user exit                                               */
      /****************************************************************/
      TAFreeUserExit( pInD );

      if ( !pTAInput->fEndTMDone )
      {
        EndTM(pTAInput, pInD);         // end translation memory services
      } /* endif */

      if (pTAInput->fNTLProcessing)
         LPTerminate ((PLPDATA *)&(pInD->pLPDATA));// terminatio of list proc.

      TAUnlockFiles ( pTAInput );      // unlock all files to allow access

      if ( pTAInput->pvReplacementList )
      {
        DITAFreeReplacementList( &(pTAInput->pvReplacementList) );
      } /* endif */

      /*------------------- free all allocated memory ----*/
      if ( pInD->pLoadedTable )
      {
        if ( pInD->pLoadedTable->pTagTable->szSegmentExit[0] != EOS )
        {
          TAFreeUserExit( pInD );
        } /* endif */
        TAFreeTagTable( pInD->pLoadedTable );
        pInD->pLoadedTable = NULL;
      } /* endif */
      if ( pInD->pLoadedQFTable )
      {
        TAFreeTagTable( pInD->pLoadedQFTable );
        pInD->pLoadedQFTable = NULL;
      } /* endif */
      UtlAlloc( (PVOID *) &(pInD->pszSegment), 0L, 0L, NOMSG );  // for LP processing
      UtlAlloc( (PVOID *) &(pInD->pszPrevSegment), 0L, 0L, NOMSG ); // for LP processing
      UtlAlloc( (PVOID *) &(pInD->pszTag), 0L, 0L, NOMSG );  // for LP processing
      UtlAlloc( (PVOID *) &(pInD->pusFileStatus), 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *) &(pInD->pTokenList), 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *) &(pInD->pSegTokenList), 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *) &(pInD->pExclusionList), 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *) &(pInD->SegBoundaryTbl.pFirstEntry), 0L, 0L, NOMSG);

      {
        PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;
        if ( (pLookupData != NULL) && (pLookupData->pRedundCountMem != NULL) )
        {
           MemoryFactory *pFactory = MemoryFactory::getInstance();
           pFactory->closeTempMemory( pLookupData->pRedundCountMem );
           pLookupData->pRedundCountMem = NULL;
       } /* endif */
      }

     if ( pInD && pTAInput->fMachTrans )
     {
       if ( !pInD->fMTStatusSet )
       {
         // no MT status set already -> analysis not completed successfully
         MTSTATUSCHANGE StatusChange;
         StatusChange.pszMTJobObject = pTAInput->stMTData.szMTJobObject;
         StatusChange.iStatus        = MTJOB_Failed;
         strcpy( StatusChange.szStatus, "Analysis failed" );
         EqfSend2Handler( MTLISTHANDLER, WM_EQF_PROCESSTASK,
                          MP1FROMSHORT( PID_FILE_MI_MOVE ),
                          MP2FROMP( &StatusChange ) );
       } /* endif */
     } /* endif */
   } /* endif valid instance data pointer */

   if ( pTAInput->pPool ) PoolDestroy( pTAInput->pPool );
   pTAInput->pPool = NULL;
   if ( pTAInput->apszLongNames )
     UtlAlloc( (PVOID *)&(pTAInput->apszLongNames), 0L, 0L, NOMSG );
   if ( pTAInput->apszAlias )
     UtlAlloc( (PVOID *)&(pTAInput->apszAlias), 0L, 0L, NOMSG );
   if ( pTAInput->pSupportedMarkups )
     UtlAlloc( (PVOID *)&(pTAInput->pSupportedMarkups), 0L, 0L, NOMSG );
  if (pTAInput->pWSList)
      UtlAlloc( (PVOID *) &(pTAInput->pWSList), 0L, 0L, NOMSG );

   /*******************************************************************/
   /* TAINPUT area will be freed during processing of WM_DESTROY      */
   /* message of the process window                                   */
   /*******************************************************************/

   if ( pTAInput->fProfile && (pTAInput->hProfile != 0) )
   {
     AnaProfFreeProfile( pTAInput->hProfile );
   } /* endif */

   /*--- terminate TA object ----*/
   if ( !fNoRemoveObject )
   {
     EqfRemoveObject( TWBFORCE, hwnd);
   } /* endif */

   return (TRUE);
   } /* end of TATerminate */

/*----------------------------------------------------------------------------*\
| TACompletion: Text Analysis Completion  Processing                           |
| purpose: notify all handlers of good completion of text analisys             |
|          registers new properties                                            |
| input  : instance data area                                                  |                          |
| output : properties files are updated, messages to handlers are sent         |                          |
\*----------------------------------------------------------------------------*/
static USHORT TACompletion (HWND hwnd, PTAINPUT pTAInput)
   {
   PTAINSTDATA   pInD;      // pointer to instance data for TA

   hwnd = hwnd; // future use
   pInD = pTAInput->pInD;

   /* MTINTERFACE */
   // close machine translation segment file if file has been used
   if ( pTAInput->fMachTrans && pInD->fMachTransFileStarted )
   {
#ifdef NFLUENT_MT_IF
     TACloseMTFile( pTAInput );
#else
     UtlBufWriteW( pInD->pCBMachTransFile, NTM_END_TAGW, (USHORT)UTF16strlenBYTE(NTM_END_TAGW), FALSE );
     UtlBufWriteW( pInD->pCBMachTransFile, L"\x0D\x0A\x1A", 6, FALSE );
     UtlBufClose( pInD->pCBMachTransFile, FALSE );
     pInD->pCBMachTransFile = NULL;
#endif
   } /* endif */

   /* MTINTERFACE */
   // handle MT job
#ifdef NFLUENT_MT_IF
#else
   if ( pInD && pTAInput->fMachTrans )
   {
     if ( pInD->fMachTransFileUsed )
     {
       // changes status of MT job to MT_Prepared
       MTSTATUSCHANGE StatusChange;
       StatusChange.pszMTJobObject = pTAInput->stMTData.szMTJobObject;
       StatusChange.iStatus        = MTJOB_Prepared;
       StatusChange.szStatus[0]    = EOS;
       StatusChange.lNumOfWords    = pTAInput->pInD->lMTWords;
       EqfSend2Handler( MTLISTHANDLER, WM_EQF_PROCESSTASK,
                        MP1FROMSHORT( PID_FILE_MI_MOVE ),
                        MP2FROMP( &StatusChange ) );

     }
     else
     {
       // no segments written to machine translation file, set
       // corresponding status
       MTSTATUSCHANGE StatusChange;
       StatusChange.pszMTJobObject = pTAInput->stMTData.szMTJobObject;
       StatusChange.iStatus        = MTJOB_Failed;
       strcpy( StatusChange.szStatus, "No MT segments" );
       EqfSend2Handler( MTLISTHANDLER, WM_EQF_PROCESSTASK,
                        MP1FROMSHORT( PID_FILE_MI_MOVE ),
                        MP2FROMP( &StatusChange ) );

     } /* endif */
     pInD->fMTStatusSet = TRUE;
   } /* endif */
#endif

   if ( !pTAInput->fEndTMDone && pInD )
   {
     EndTM(pTAInput, pInD);         // end translation memory services
   } /* endif */

   if ( pInD )
     {
#ifdef MEASUREANALYSISTIME
          // write measured time values to log file
          TAWriteTimeLog( pTAInput );
#endif
     // completion message
     switch (pInD->usError)
        {
        case TACOMPL:
           if (!pTAInput->fNoComplete && !pTAInput->fBatch && !pTAInput->pszObjList )
           {
              UtlError (ERROR_TACOMPLETE, MB_OK, 0, NULL, EQF_INFO);
           }
           else
           {
             if ( pTAInput->fBatch && (hwnd != HWND_FUNCIF) )
             {
                 pTAInput->pDDEAnalysis->DDEReturn.usRc = UtlGetDDEErrorCode( pTAInput->hwndErrMsg );
               // pTAInput->pDDEAnalysis->DDEReturn.usRc = 0;
             } /* endif */
           } /* endif */
           break;

        case TA_PREV_ERROR:
        case TAERROR:
           // errors during analysis
           if ( !pTAInput->fBatch )
           {
             UtlError ( ERROR_TA_COMPL_ERR, MB_OK, 0, NULL, EQF_INFO); // no err
           }
           else if ( hwnd != HWND_FUNCIF )
           {
             pTAInput->pDDEAnalysis->DDEReturn.usRc = UtlGetDDEErrorCode( pTAInput->hwndErrMsg );
           } /* endif */
           break;

        default:
           break;
        } // end switch error code
     }  /*- endif pInd --*/


   return (TRUE);

   } /* end of TACompletion */


/*----------------------------------------------------------------------------*/
/* NAME: EQFTextSegmentation                                                  */
/* Purpose : this function is used to segment the current text  block         */
/*  depending on tags in the tagtable and tolstoy complex tokenization        */
/*                                                                            */
/* The caller must provide the block in a buffer and the tag table; the NLP   */
/* functions have to be already initialized.                                  */
/* The function returns a pointer to a linked list of segments                */
/* The process begins with tokenization with respect to tags, and then seg-   */
/* mentation in large text blocks based on the identified tags, procssing     */
/* of attributes of tags          and then subsequent segmentation made by    */
/* the tolstoy linguistics services. The output consist of a pointer to a     */
/* linked list of segment boundaries, in which every element has a pointer    */
/* to the actual text and a status indicator (to be translated, not to be     */
/* translated). Before exiting the function saves the status of operation     */
/* so that the next call with the next block would resume the process from    */
/* the point in which it had been interrupted.                                */
/*                                                                            */
/* input   : pInD   pointer to a structure that contains all the static data  */
/*                  this structure contains the pointers to the text to       */
/*                  analize, to the tag table, and to the work areas          */
/* output:                                                                    */
/*           ppRest p. to p. to the text to analize; at the end the pointer   */
/*                  to text is updated to the last not tokenized part         */
/*           ppSegBoundaryList pointer to the first element of the segment    */
/*                  list built by the function                                */
/*                                                                            */
/* Called by       : EQFBlockProcessing and functions external to this module */
/*                                                                            */
/* Calling hierarcy:                                                          */
/*                                                                            */
/*    EQFTextSegmentation                                                     */
/*           EQFTagTokenize                                                   */
/*           EQFTslnCheck                                                     */
/*           DetermineWhitespace                                              */
/*           EQFSegDeterminate                                                */
/*           EQFTolstComplex                                                  */
/*           CalcSegState                                                     */
/*           ForceSegBoundary                                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
static BOOL EQFTextSegmentation (
             HWND          hwnd,                // instance window handle
             PTAINSTDATA   pInD,                // pointer to control data
             PSZ_W         *ppRest,             // pointer to rest of input
             PSEGBOUNDARY  *ppSegBoundaryList   // pointer to segment boundary
                                )

   {
   BOOL fOK = TRUE;                            // processing flag
   PSEGBOUNDARY pFirstEntrySeg = NULL;         // start of segment boundary table
   PSEGBOUNDARY pCurSegBoundary = NULL;        // current segment boundary
   PTOKENENTRYSEG pSegTokenEntry;              // pointer to segmenting list
   PSZ_W          pBlockEnd = NULL;            // pointer to end of text block
   PTAG           pTag;                        // pointer to tags in tagtable
   PBYTE          pByte;                       // pointer to tagtable
   BOOL           fPrevProtectMode;            // Status at beginning of block

   /*******************************************************************/
   /* Get tag pointer                                                 */
   /*******************************************************************/
   pByte = (PBYTE) pInD->pLoadedTable->pTagTable;
   pTag = OFFSETTOPOINTER (PTAG, pInD->pLoadedTable->pTagTable->stFixTag.uOffset);

   /*******************************************************************/
   /* init pSegmentTokenList to 0                                     */
   /*******************************************************************/
   memset( pInD->pSegTokenList, 0, MAX_ALLOC_SEGTOKENS*sizeof(TOKENENTRYSEG));
   fPrevProtectMode = pInD->fProtectMode ;          /* 10-2-14 */

   // tokenize the text block
   TATagTokenizeW( pInD->szTextBuffer,              // input text
                  pInD->pLoadedTable,               // pointer to tag table
                  pInD->fAll,                       // complete text ?
                  ppRest,                           // end of tokenized text
                  &(pInD->SegmentControl.usColPos), // last column pos
                  pInD->pTokenList,                 // p token list
                  MAX_NUM_TOKENS                    // max num of tokens
                  );

   fOK = (*ppRest != pInD->szTextBuffer); // not a valid input text if it
                                          // could not be tokenized

   if (fOK)
      {
      // do tsnl processing
      EQFTsnlcheck(pInD->fAll && (*ppRest == NULL),  // more text follow ?
                   ppRest,                           // rest of text
                   pInD->pLoadedTable->pTagTable,    // tag table
                   pInD->pTokenList,                 // token list table
                   pInD->pSegTokenList,              // segment token list
                   MAX_NUM_SEGTOKENS                 // max num
                   );

      // determine the text tokens, which consist of whitespace characters only
      DetermineWhitespace( pInD->pSegTokenList, pTag );

      // initialize segment boundary table
      pInD->SegBoundaryTbl.pCurEntry = pInD->SegBoundaryTbl.pFirstEntry;
      pInD->SegBoundaryTbl.usNumCurEntry = MAX_NUM_SEGTOKENS;
      pFirstEntrySeg = GetEntry(&(pInD->SegBoundaryTbl));

      if ( !pFirstEntrySeg )
      {
         fOK = FALSE;
      }
      else
      {
         pFirstEntrySeg->pNext = NULL;
         pFirstEntrySeg->pBoundary = NULL;
         *ppSegBoundaryList = pFirstEntrySeg;

         // determine segment boundaries with respect to the tokenlist
         fOK = EQFSegDeterminate( pInD, pFirstEntrySeg);

      } /* endif */
   } /* endif */

   if ( fOK )
   {
      // determine segment boundaries with Tolstoy functions
      pInD->fProtectMode = fPrevProtectMode ;  /* Reset to status at start of block 10-2-14 */
      fOK = EQFTolstComplex ( hwnd, pInD, pFirstEntrySeg );
   } /* endif */

   if (fOK)
      { // calculate control information for next call to textsegmentation
      // get last segment boundary
      pCurSegBoundary = pFirstEntrySeg;
      while (pCurSegBoundary->pNext )
         {
         pCurSegBoundary = pCurSegBoundary->pNext;
         }

      // set pointer to end of segmented text
      pSegTokenEntry = pInD->pSegTokenList;
      if ( pSegTokenEntry->sTokenid == ENDOFLIST )
      {
        fOK = FALSE;
      }
      else
      {
        while (pSegTokenEntry->sTokenid != ENDOFLIST)
           {
           pSegTokenEntry ++;
           }
        pSegTokenEntry --;

        // if last token is whitespace, ignore as the text is part of the
        // previous token (as defined in DetermineWhiteSpace)
        if (pSegTokenEntry->sTokenid == WHITESPACE)
        {
           pSegTokenEntry--;
           // GQ: don't ignore this token if previous token is a dummy tag token!
           if (pSegTokenEntry->sTokenid == DUMMYTAG)
           {
             pSegTokenEntry++;
           } /* endif */
        } /* endif */
        pBlockEnd = pSegTokenEntry->pDataStringW + pSegTokenEntry->usLength;

      } /* endif */
    } /* endif */

    if ( fOK )
    {
      if ( (*ppRest == NULL) && pInD->fAll )    // the text block is complete
         {
         // insert a segment boundary at the end of text,
         // if there is not already one
         if (pBlockEnd > pCurSegBoundary->pBoundary )
            {
            pCurSegBoundary->pNext = GetEntry(&(pInD->SegBoundaryTbl));
            if (!pCurSegBoundary->pNext)
               {
               fOK = FALSE;
               }
            else
               {
               pCurSegBoundary = pCurSegBoundary->pNext;
               pCurSegBoundary->pBoundary = pBlockEnd;
               }
            }

         }
      else // not the whole text was tokenized
         {
         if (pCurSegBoundary->pBoundary == pFirstEntrySeg->pNext->pBoundary)
            { // no complete segment  was identified
            // display warning that segment boundary  has to be forced
            if (!pInD->fSeglength)
               {
               pInD->fSeglength = TRUE;
               }

            if (fOK)
               {
               pCurSegBoundary->pNext = GetEntry(&(pInD->SegBoundaryTbl));
               if ( !pCurSegBoundary->pNext )
                  {
                  fOK = FALSE;
                  }
               else
                  {
                  pCurSegBoundary = pCurSegBoundary->pNext;
                  pCurSegBoundary->pBoundary = pBlockEnd;
                  }
               }
            } /* endif */

         if (fOK)
            {
            // set pRest to the end of the last complete segment
            *ppRest = pCurSegBoundary->pBoundary;

            // calculate last column position
            CalcLastColumnPos( pCurSegBoundary->pBoundary, pInD->szTextBuffer,
                               &(pInD->SegmentControl.usColPos));

            // calculate segmenting state
            CalcSegState ( pInD->pSegTokenList, &(pInD->SegmentControl.sSegState),
                           pTag, pCurSegBoundary->pBoundary,
                           pInD->pLoadedTable->pTagTable->uNumTags);
            }
         }

      if (fOK)
         {
         // check for maximum size of segment boundaries
         pCurSegBoundary = pFirstEntrySeg->pNext;
         while (fOK && pCurSegBoundary->pNext != NULL)
         {
             // check segment length
              int iSegLen = pCurSegBoundary->pNext->pBoundary - pCurSegBoundary->pBoundary;
              int iMaxSegSize ;
              if ( pCurSegBoundary->XlateState == TODO )
                 iMaxSegSize = 1600 ;
              else
                 iMaxSegSize = MAX_SEGMENT_SIZE ;
              if ( iSegLen >= iMaxSegSize)
               {
               if (!pInD->fSeglength)  // user has not been informed yet about maximum
                  {                 // segment length
                  pInD->fSeglength = TRUE;
                  }

               if (fOK)
                  {
                  fOK = ForceSegboundary(&(pInD->SegBoundaryTbl), &(pCurSegBoundary), iMaxSegSize);
                  }
               }
            else
               {
               pCurSegBoundary = pCurSegBoundary->pNext;
               }
         } /* endwhile */
       }
      } /* endif */

   return (fOK);
   }

/*
.cp
************* Text Analysis Tsnlcheck *****************************************
*                                                                             *
*  check the identified tag  tokens for associated text                       *
*  (tsl or tsnl associated text )  and copy tokens to segmenting tokenlist    *
******************************************************************************/

static
VOID   EQFTsnlcheck(
           BOOL           fAll,             // more text following?
           PSZ_W          *ppRest,          // pointer to pointer to rest
                                            // of input
           PTAGTABLE      pTagTable,        // pointer to tagtable
           PTOKENENTRY    pTokenList,       // pointer to first entry in tokenlist
           PTOKENENTRYSEG pSegTokenList,    // pointer to segmenting tokenlist
           USHORT         uMaxLenList       // maximum length of new tokenlist
                   )

{
   PTOKENENTRYSEG  pSegTokenEntry;          // current entry in segment. list
   PTOKENENTRY     pTokenEntry;             // current entry in tokenlist
   PTOKENENTRY     pTokenEntryWork;
   BOOL            fOverflow = FALSE;       // does overflow occur?
   BOOL            fFound;                  // end of associated text found?
   BOOL            fComplete = TRUE;        // TSNL checking complete?
   PBYTE           pByte;                   // pointer to tagtable
   PTAG            pTag;                    // pointer to first tag in tagtable
   PTAG            pCurTag;                 // pointer to current tag
   PTOKENENTRYSEG  pTslToken = NULL;        // pointer to last valid tsl tag
   USHORT          usNumAttr;               // number of attributes after tag
   PSZ_W           pText = NULL;            // pointer to input text
   USHORT          usCurLength = 0;         // current length of text token
   USHORT          uNumTags;                // number of tags in tagtable
   BOOL            fFirstToken;             // first-token-of-associated-text flag


  pByte = (BYTE * )pTagTable;                      /* set pointer to tagtable */

  // calculate pointer to first tag
  pTag = OFFSETTOPOINTER( PTAG, pTagTable->stFixTag.uOffset);

  uNumTags = pTagTable->uNumTags;
  pSegTokenEntry = pSegTokenList;
  pTokenEntry = pTokenList;


  // if first entry in old tokenlist is text, insert dummy tag token
  if (pTokenEntry->sTokenid == TEXT_TOKEN )
  {
    pSegTokenEntry->sTokenid = DUMMYTAG;
    pSegTokenEntry->usLength = 0;
    pSegTokenEntry->pDataStringW = pTokenEntry->pDataStringW;
    pSegTokenEntry++;
  } /* endif */

   // copy first entry from old tokenlist to new one
   memcpy(pSegTokenEntry, pTokenEntry, sizeof(TOKENENTRY));
   pSegTokenEntry ++;

   // check all tag tokens in tokenlist for associated text
   while (pTokenEntry->sTokenid != ENDOFLIST && ! fOverflow)
   {
        if ( pTokenEntry->sTokenid < (SHORT) uNumTags  // token is tag
             && pTokenEntry->sTokenid >= 0)
        {
           if (pTag[pTokenEntry->sTokenid].Asstext != NOEXPL_TEXT)
           {                                      // tag has associated text
              fComplete = FALSE; // tsnl processing for current tag not complete
              fFound = FALSE;    // end of associated text not found yet
              pTslToken = pSegTokenEntry-1;       // remember current tag token

              switch (pTag[pTokenEntry->sTokenid].Asstext)
              {
                case TSNL_TEXT:
                  // search for ; and Newline in next text token

                  // calculate number of following attribute tokens
                  fFirstToken = TRUE;
                  pTokenEntryWork = pTokenEntry + 1;
                  while (pTokenEntryWork->sTokenid >= (SHORT) uNumTags)
                  {                                       // token is attribute
                     pTokenEntryWork++;
                  } /* endwhile */

                  if (pTokenEntryWork->sTokenid == TEXT_TOKEN || // next token is text
                     (pTokenEntryWork->sTokenid >= 0))     // or tag
                  {
                     // copy attribute tokens  if there is enough space

                     usNumAttr = (USHORT)(pTokenEntryWork - pTokenEntry -1);
                     if (pSegTokenEntry - pSegTokenList + usNumAttr + 5
                          >= uMaxLenList)
                     {
                         // not enough space in new tokenlist to hold attribute
                         // tokens plus newly generated text and dummy tokens
                         // and endoflist token
                        fOverflow = TRUE;
                     }
                     else  // enough space in tokenlist
                     {
                       // copy attribute tokens
                       pTokenEntry ++;
                       while (pTokenEntry->sTokenid >= (SHORT) uNumTags)
                       {
                         memcpy(pSegTokenEntry, pTokenEntry, sizeof(TOKENENTRY));
                         pSegTokenEntry ++;
                         pTokenEntry ++;
                       } /* endwhile */

                       //  loop until you reach end of list or end of line
                       //  will fix problems like: :h2.Sch&oe.naicher Stra&ss.e
                       while ( !fFound && pTokenEntry->sTokenid != ENDOFLIST
                                && !fOverflow )
                       {
                          if (pTokenEntry->sTokenid == TEXT_TOKEN)
                          {      // search for ; or NEWLINE in text
                            pText = pTokenEntry->pDataStringW;

                            // skip spaces at beginning
                            usCurLength = 0;
                            while (usCurLength < pTokenEntry->usLength &&
                                  ( *pText == ' ' || *pText == CR))
                            {
                               pText++;
                               usCurLength ++;
                            } /* endwhile */


                            // if next character is NEWLINE, skip it
                            if ( (usCurLength < pTokenEntry->usLength) &&
                                 fFirstToken &&
                                 (*pText == LF) )
                            {
                               pText++;
                               usCurLength++;
                            } /* endif */
                            fFirstToken = FALSE;

                            while (!fFound && usCurLength < pTokenEntry->usLength)
                            {
                               if (*pText == LF ||*pText == SEMICOLON)
                               {
                                  fFound = TRUE;   // end of associated text found
                               } /* endif */

                               pText++;
                               usCurLength++;
                            } /* endwhile */

                            if (!fFound)   // end of associated text not found
                            {
                              // copy text token
                              memcpy(pSegTokenEntry, pTokenEntry, sizeof(TOKENENTRY));
                              pSegTokenEntry ++;

                              pTokenEntry++;
                              // check overflow
                              if ( pSegTokenEntry-pSegTokenList>=(SHORT)uMaxLenList-2)
                              {
                                 fOverflow = TRUE;
                              } /* endif */
                            } /* endif */
                          }
                          else
                          {
                            pCurTag = pTag + pTokenEntry->sTokenid;
                            if ( (pCurTag->Tagtype == STARTNEUTRAL_TAG) ||
                                 (pCurTag->Tagtype == ENDNEUTRAL_TAG) )
                                   // next token is tag but copy it ....
                            {

                              pText = pTokenEntry->pDataStringW;
                              usCurLength = pTokenEntry->usLength;
                              // we found end since we detected a linefeed
                              if ( *(pText+usCurLength-1) == LF )
                              {
                                 fFound = TRUE;
                              }
                              else
                              {
                                 // copy token
                                 memcpy(pSegTokenEntry, pTokenEntry,
                                        sizeof(TOKENENTRY));
                                 pSegTokenEntry ++;
                                 pTokenEntry++;
                                 pTokenEntryWork = pTokenEntry; // set work entry
                                 // check overflow
                                 if ( pSegTokenEntry-pSegTokenList>=(SHORT)uMaxLenList-2)
                                 {
                                    fOverflow = TRUE;
                                 } /* endif */
                              } /* endif */
                            }
                            else
                            {
                              /****************************************/
                              /* Tag is not segment neutral,          */
                              /* end TSNL check before this tag       */
                              /****************************************/
                              fFound = TRUE;
                              pTokenEntry--;
                              usCurLength = pTokenEntry->usLength;
                              pText       = pTokenEntry->pDataStringW;
                            } /* endif */
                          } /* endif */
                       } /* endwhile */
                       if ( pTokenEntry->sTokenid == ENDOFLIST )
                       {
                          pTokenEntry--;   // will be increased at end of loop
                       } /* endif */
                     } /* endif */
                  } /* endif */
                  break;

                case TSL_TEXT:
                  /* search for Newline in the following tokens */

                  // check if current token ends with NEWLINE
                  if (*(pTokenEntry->pDataStringW + pTokenEntry->usLength -1)
                      == LF )
                  {
                     fFound = TRUE;
                     usCurLength = pTokenEntry->usLength;
                     pText = pTokenEntry->pDataStringW + usCurLength;
                     pSegTokenEntry--;
                  }
                  else    // current token does not end with NEWLINE
                  {
                     pTokenEntry++;    // check next tokens
                  } /* endif */

                  while (!fFound && pTokenEntry->sTokenid != ENDOFLIST
                         && !fOverflow )
                  {
                     /* search current token for NEWLINE                      */
                     pText = pTokenEntry->pDataStringW;
                     usCurLength = 0;
                     while (!fFound && usCurLength < pTokenEntry->usLength)
                     {
                        fFound = (*pText == LF);
                        pText++;
                        usCurLength++;
                     } /* endwhile */

                     if (!fFound)   // current token does not contain newline
                     {
                        // copy current token to new tokenlist
                        memcpy(pSegTokenEntry, pTokenEntry, sizeof(TOKENENTRY));
                        pSegTokenEntry ++;
                        pTokenEntry ++;

                        // check overflow in new tokenlist
                        if (pSegTokenEntry - pSegTokenList >= (SHORT)uMaxLenList - 2)
                        {
                          fOverflow = TRUE;
                        } /* endif */
                     } /* endif */
                  } /* endwhile */

                  if (pTokenEntry->sTokenid == ENDOFLIST)
                  {
                     pTokenEntry--;    // will be increased again at end of
                  } /* endif */        // while loop

                  break;

                default:
                  // no other types of associated text defined yet
                   break;
              } /* endswitch */


              if (fFound)  // end of associated text found in text
              {
                 // split token and insert dummy token where end of associated
                 // text was found
                 memcpy(pSegTokenEntry, pTokenEntry, sizeof(TOKENENTRY));
                 pSegTokenEntry->usLength = usCurLength;
                 pSegTokenEntry ++;

                 pSegTokenEntry->sTokenid = DUMMYTAG;
                 pSegTokenEntry->usLength = 0;
                 pSegTokenEntry->pDataStringW = pText;
                 pSegTokenEntry++;

                 if (usCurLength < pTokenEntry->usLength)
                 {
                   // end of associated text not at end of token
                   pSegTokenEntry->sTokenid = TEXT_TOKEN;
                   pSegTokenEntry->usLength = pTokenEntry->usLength - usCurLength;
                   pSegTokenEntry->pDataStringW = pText;
                   pSegTokenEntry ++;
                 } /* endif */

                 fComplete = TRUE;
              }
              else  if (fComplete)
              {
                 // dummy token has to be inserted after current tag
                 pSegTokenEntry->sTokenid = DUMMYTAG;
                 pSegTokenEntry->usLength = 0;
                 pSegTokenEntry->pDataStringW = (pTokenEntry+1)->pDataStringW;
                 pSegTokenEntry++;
              } /* endif */
           } /* endif */
        } /* endif */

      if (fOverflow)  // overflow in tsl or tsnl processing occured
      {
         pSegTokenEntry = pTslToken; // discard all tokens up to
                                     // tsnl tag token
      }
      else  // no overflow occured, copy next token to new tokenlist
      {
        pTokenEntry ++;
        memcpy(pSegTokenEntry, pTokenEntry, sizeof(TOKENENTRY));
        pSegTokenEntry ++;


        // check overflow in new tokenlist after copying

        if (pSegTokenEntry - pSegTokenList >= (SHORT)uMaxLenList - 2)
        {
           pSegTokenEntry --;
           if ((pSegTokenEntry)->sTokenid != ENDOFLIST)
           {
              fOverflow = TRUE;
           } /* endif */

        } /* endif */
      } /* endif */
   } /* endwhile */

   if (fOverflow)
   {
      pSegTokenEntry->sTokenid = ENDOFLIST;
      *ppRest = pSegTokenEntry->pDataStringW;
   }
   else if (!fComplete && !fAll)
   {
      pSegTokenEntry = pTslToken;
      pSegTokenEntry->sTokenid = ENDOFLIST;
      *ppRest = pSegTokenEntry->pDataStringW;
   } /* endif */
} /* end of EQFTsnlcheck */

/*
.cp
************* Text Analysis DetermineWhitespace *******************************
*                                                                             *
*  go through the segmenting tokenlist and check the text tokens for          *
*  whitespace characters. Add all leading whitespace characters to the        *
*  previous token, if there is one. If the whole text token                   *
*  consists of whitespace characters, the ID is changed to WHITESPACE         *
*  and the token is joined with the previous one. If there is no previous     *
*  token other than DUMMYTAG, the whitespace token is joined with the next.   *
******************************************************************************/

static
VOID  DetermineWhitespace
(
  PTOKENENTRYSEG pSegTokenList,
  PTAG           pTag                  // pointer to tags
)
{
   /* determine leading whitespace characters in text tokens and concatenate */
   /* them with previous tag token */
   PTOKENENTRYSEG pSegTokenEntry;         // pointer to segmenting tokenlist
   PSZ_W          pText;                  // pointer to input text
   USHORT         usCurLength;            // count for length of current token
   PSZ_W          pLine;                  // pointer to last LF
   CHAR_W         c;                      // temp character
   SHORT          sTokenId;               // ID of current token

   pSegTokenEntry = pSegTokenList;

   while (pSegTokenEntry->sTokenid != ENDOFLIST)  // search tokenlist
   {
      if (pSegTokenEntry->sTokenid == TEXT_TOKEN)    // check text token for whitespace
      {  // a text token can never be first token
         pText = pSegTokenEntry->pDataStringW;
         usCurLength = 0;
         pLine = NULL;

         while (( (c = *pText) == CR ||             // skip whitespace in token
                 c == LF ||
                 c == BLANK) &&
                 usCurLength < pSegTokenEntry->usLength)
         {
            if (c == LF)
            {
               pLine = pText + 1;
            } /* endif */
            pText ++;
            usCurLength++;
         } /* endwhile */

         if (usCurLength < pSegTokenEntry->usLength &&  // not the whole token
             pLine != NULL)                             // consists of whitesp.
         {
            usCurLength = (USHORT)(usCurLength - (pText - pLine));             // go back to last LF
            pText = pLine;
         } /* endif */


//       add white spaces to previous token up to the CRLF
//       sample: :LINES
//                      abd
//               :ELINES.
//       abd should have the blanks in front of it
//

         /*************************************************************/
         /* Change by XQG: Do whitespace handling only if previous    */
         /* tag is no inline tag                                      */
         /*************************************************************/
         sTokenId = (pSegTokenEntry-1)->sTokenid;
         if ( (sTokenId >= 0) &&
              ( (pTag[sTokenId].Tagtype == STARTNEUTRAL_TAG) ||
                (pTag[sTokenId].Tagtype == ENDNEUTRAL_TAG) ) )
         {
           /***********************************************************/
           /* Leave whitespace where it is ...                        */
           /***********************************************************/
         }
         else if (pSegTokenEntry->usLength == usCurLength)
         {   // text consists of whitespace characters only
            if ((pSegTokenEntry-1)->sTokenid < 0)  // = DUMMYTAG
            {  // join whitespace token with next token, if possible..
              if ( (pSegTokenEntry+1)->sTokenid != ENDOFLIST )
              {
                (pSegTokenEntry+1)->usLength =(pSegTokenEntry+1)->usLength + usCurLength;
                (pSegTokenEntry+1)->pDataStringW = pSegTokenEntry->pDataStringW;
                pSegTokenEntry->sTokenid = WHITESPACE;
                pSegTokenEntry->usLength = 0;
              } /* endif */
            }
            else
            {  // join whitespace token with previous token
               (pSegTokenEntry-1)->usLength = (pSegTokenEntry-1)->usLength + usCurLength;
                pSegTokenEntry->sTokenid = WHITESPACE;
                pSegTokenEntry->usLength = 0;
            } /* endif */
         }
         else if ((pSegTokenEntry-1)->sTokenid >= 0)
         {
            pSegTokenEntry->usLength = pSegTokenEntry->usLength - usCurLength;
            pSegTokenEntry->pDataStringW += usCurLength;
           (pSegTokenEntry-1)->usLength = (pSegTokenEntry-1)->usLength + usCurLength;
         } /* endif */

      } /* endif */
      pSegTokenEntry++;
   } /* endwhile */
} /* end of DetermineWhitespace */

/*----------------------------------------------------------------------------*/
/* NAME: EQFSegDeterminate                                                    */
/*                                                                            */
/* Determine the segment boundaries with respect to tagginag information.     */
/* Using the information in the segmenting tokenlist                          */
/* Purpose : first pass of segment information list build. It uses the        */
/* information in the token list (with segmenting information) to build       */
/* the linked list of segment boundary.                                       */
/* There are token that open a segment, token that close it and token neutral */
/* to a segment.                                                              */
/*                                                                            */
/* input    :  pInD pointer to instance data area that contains all static    */
/*                  data; what is here used is the token list with segmenting */
/*                  information, the tag table (for both the tags and the     */
/*                  attributes), the previous segmenting state (saved by the  */
/*                  routine itself)                                           */
/*                                                                            */
/*             pFirstEntrySeg pointer to the first element in the segment     */
/*                  boundary table                                            */
/*                                                                            */
/* output   :  the segment boundary table is built. it returns TRUE if the    */
/*             operation was completed succesfully                            */
/*                                                                            */
/* Called by        :  EQFTextSegmentation                                    */
/*                                                                            */
/* Calling hierarchy:                                                         */
/*        EQFSegDeterminate                                                   */
/*            SetSegInfo                                                      */
/*            GetEntry                                                                */
/*                                                                            */
/* pseudologic:                                                               */
/*                                                                            */
/* EQFSegDeterminate                                                          */
/*   BEGIN                                                                    */
/*       Initialize pointers to tag table, attribute table                    */
/*       set the segmenting state to saved segmenting state                   */
/*       set the segment pointer to the beginning of list                     */
/*       IF first token is DUMMYTAG THEN                                      */
/*          set segment information to the saved segment information          */
/*       ENDIF                                                                */
/*       Allocate a new segment boundary item                                 */
/*       Make it a TEXT segment to translate                                  */
/*       WHILE all token had been scanned and OK                              */
/*          IF token is tag THEN                                              */
/*             IF segmenting state is > 0 THEN                                */
/*                call SetSegInfo (Seg info: SEG the following text has       */
/*                                 to be segmented, NOSEG the text following  */
/*                                 has not to be segmented, SKIP if the       */
/*                                 text following has to be joined with the   */
/*                                 previous for segmentation)                 */
/*                IF current token (tag) breaks the segment  THEN             */
/*                   segmenting state = OFF                                   */
/*                ENDIF                                                       */
/*             ELSE // segmenting state is OFF                                */
/*                CASE tag segmenting attributes OF                           */
/*                   SEG_ON: increase segmenting state                        */
/*                   SEG_OFF: decrease segmenting state                       */
/*                   SEG_RESET: set segmenting state ON                       */
/*                   SEG_NEUTRAL: nothing                                     */
/*                   OTHERWISE: nothing                                       */
/*                ENDCASE                                                     */
/*                CALL SetSegState to insert segmenting information           */
/*             ENDIF // segment no active                                     */
/*          ENDIF // tag is token                                             */
/*          Get next token                                                    */
/*       ENDWHILE                                                             */
/*       IF OK THEN                                                           */
/*          IF the seginfo of the first token is SKIP THEN                    */
/*             change it to the value in segment control                      */
/*          ENDIF                                                             */
/*       ENDIF                                                                */
/*   END                                                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/
static
BOOL EQFSegDeterminate
     (PTAINSTDATA    pInD,                // pointer to instance data
      PSEGBOUNDARY   pFirstEntrySeg)      // pointer to first entry in segment
                                          // boundary list
   {
   PTOKENENTRYSEG  pSegTokenEntry;        // pointer to segmenting tokenlist
   PSEGBOUNDARY    pCurSegBoundary;       // pointer to current entry in segment
   SEGBOUNDARYTBL  * pSegBoundaryTbl;     // pointer to segment boundary table
                                          // boundary list
   SHORT           sSegState;             // segmenting state
   SEGTYPE         SegType;               // segmenting type
   PTAG            pTag;                  // pointer to tags in tagtable
   PATTRIBUTE      pAttribute;            // pointer to attributes in tagtable
   PBYTE           pByte;
   BOOL            fOK = TRUE;            // processing state
   SHORT           sTokenid;              // id of current tag token
   PTAGTABLE       pTable = pInD->pLoadedTable->pTagTable; // shortcut for tag table

   // initialize pointers to tagtable
   pByte = (PBYTE) pTable;
   pTag = OFFSETTOPOINTER(PTAG, pTable->stFixTag.uOffset);
   pAttribute = OFFSETTOPOINTER(PATTRIBUTE, pTable->stAttribute.uOffset);

   pSegTokenEntry = pInD->pSegTokenList;         // start at beginning of tokenlist
   sSegState = pInD->SegmentControl.sSegState; // init segmenting state
   SegType   = pInD->SegmentControl.SegType;   // init segmenting type
   pCurSegBoundary = pFirstEntrySeg;       // start with empty segment boundary
                                           // list
   pSegBoundaryTbl = &(pInD->SegBoundaryTbl);

   // if first token is dummy tag set seginfo to info in segmentcontrol
   if (pSegTokenEntry->sTokenid == DUMMYTAG)
      {
      pSegTokenEntry->Seginfo = pInD->SegmentControl.Seginfo;
      pSegTokenEntry->SegType = pInD->SegmentControl.SegType;
      pSegTokenEntry++;
      } /* endif */

   // set segment boundary at start of text block
   pCurSegBoundary->pNext = GetEntry(pSegBoundaryTbl);

   if (pCurSegBoundary->pNext == NULL)
      {
      fOK = FALSE;
      }
   else
      {
      pCurSegBoundary = pCurSegBoundary->pNext;
      pCurSegBoundary->pBoundary = pSegTokenEntry->pDataStringW;
      pCurSegBoundary->XlateState = pInD->fProtectMode ? NOP : TODO;
#ifdef TSEGM_SEGLOGGING
     if ( hSegLog )
     {
       CHAR_W chTemp = pSegTokenEntry->pDataStringW[pSegTokenEntry->usLength];
       pSegTokenEntry->pDataStringW[pSegTokenEntry->usLength] = 0;
       fwprintf( hSegLog, L"EqfSegDeterminate: Token=(\"%s\") = %s\n", pSegTokenEntry->pDataStringW,
         pInD->fProtectMode ? L"Nop" : L"Todo" );
       pSegTokenEntry->pDataStringW[pSegTokenEntry->usLength] = chTemp;
     } /* endif */
#endif

      pCurSegBoundary->pNext = NULL;
      }

   sTokenid = pSegTokenEntry->sTokenid;
   while ( sTokenid != ENDOFLIST && fOK )
      {
      if (sTokenid >= 0 &&         // token is tag
          sTokenid < (SHORT) pInD->pLoadedTable->pTagTable->uNumTags )
         {
#ifdef TSEGM_SEGLOGGING
     if ( hSegLog )
     {
       fwprintf( hSegLog, L"Seg=%d,%d   %d   %d\n",sTokenid,pInD->pLoadedTable->pTagTable->uNumTags,sSegState,pTag[sTokenid].Segbreak);
     } /* endif */
#endif
         if (sSegState == ON)
         {
            fOK = SetSegInfo(pInD, &pSegTokenEntry,
                             pSegBoundaryTbl,
                             pTag,
                             pAttribute,
                             sSegState, &SegType,               /* 1@KIT1110C */
                             &(pCurSegBoundary),
                             pTable->uNumTags);

            switch ( pTag[sTokenid].Segbreak )                  /* 7@KIT1110C */
            {
              case SEG_OFF :
                sSegState = OFF;
              default :
                break;
            } /* endswitch */
         }
         else if (sSegState <= 0)    // segmenting state is off
         {
                 switch (pTag[sTokenid].Segbreak)
                    {
                    case SEG_ON:      // increase segmenting state
                       sSegState ++;
                       break;

                    case SEG_OFF:     // decrease segmenting state one level
                       sSegState --;
                       break;

                    case SEG_RESET:   // reset segmenting state to on
                       sSegState = ON;
                       break;

                    case SEG_LINE:    // reset segmenting state to on
                       sSegState = ON;
                       SegType = LINEPERLINE;
                       break;

                    case SEG_NOSEGMENTING:    // no segmenting in text block
                       sSegState = ON;
                       SegType = NOSEGMENTING;
                       break;

                    case SEG_NEUTRAL: // leave segmenting state as is
                       break;

                    default:
                       break;
                    } /* endswitch */

                 fOK = SetSegInfo(pInD, &pSegTokenEntry,
                                  pSegBoundaryTbl,
                                  pTag,
                                  pAttribute,
                                  sSegState, &SegType,          /* 1@KIT1110C */
                                  &(pCurSegBoundary),
                                  pTable->uNumTags);
         } /* endifsegState < 0 */
      } /* endif token is tag */
      pSegTokenEntry->SegType = SegType;                        /* 1@KIT1110C */
      sTokenid = (++pSegTokenEntry)->sTokenid;
#ifdef TSEGM_SEGLOGGING
     if ( hSegLog )
     {
       fwprintf( hSegLog, L"Next Seg=%d,%d\n",sTokenid,pInD->pLoadedTable->pTagTable->uNumTags);
     } /* endif */
#endif
   } /* endwhile for all tokens */

   if (fOK)
      {
      // check if seginfo of first token in segmenting tokenlist is
      // SKIP, if yes change it to the value in segmentcontrol
      if (pInD->pSegTokenList->Seginfo == SKIP)
         {
         pInD->pSegTokenList->Seginfo = pInD->SegmentControl.Seginfo;
         pInD->pSegTokenList->SegType = pInD->SegmentControl.SegType;
         } /* endif */

      // set return values
      pInD->SegmentControl.sSegState = sSegState;
      pInD->SegmentControl.Seginfo = (sSegState <= 0) ? NOSEG : SEG;
      pInD->SegmentControl.SegType = SegType;
      } /* endif */
   return (fOK);
   }  /* end of EQFSegDeterminate */


/*
.cp
************* SetSegInfo ******************************************************
*                                                                             *
*  Determine segmentation information with respect to tagging information,    *
*  i.e. for each tag token specify whether the following text shall be        *
*  segmented (SEG), not segmented (NOSEG) or whether the following text shall *
*  be joined with the previous (SKIP). Also segment boundaries are determined *
*  with respect to the tagging information and the pointers to the start of   *
*  new segments are stored in a segment boundary list                         *
******************************************************************************/

static
BOOL SetSegInfo(
        PTAINSTDATA    pInD,             // pointer to instance data
        PTOKENENTRYSEG *ppSegTokenEntry,     // pointer to segmenting tokenlist
        SEGBOUNDARYTBL *pSegBoundaryTbl,     // pointer to segment boundary table
        PTAG           pTag,                 // pointer to tags
        PATTRIBUTE     pAttribute,           // pointer to attributes
        SHORT          sSegState,           // segmenting state
        SEGTYPE        *pSegType,            // segmenting type /* 1@KIT1110C */
        PSEGBOUNDARY   *ppCurSegBoundary,    // last segment boundary
        USHORT           uNumTags)             // number of tags in tagtable

   {

   SEGSTATE usSegValue;                   // segmenting value
   PTOKENENTRYSEG pSegTok;               // pointer to segmenting token
   BOOL    fSegboundary1 = FALSE;
   BOOL    fSegboundary2 = FALSE;        // shall segment boundaries be inserted?
   BOOL    fTagEndBoundary;              // shall segment boundary be
                                         // inserted at end of tag?
   TASEGINFO Seginfo1, Seginfo2;         // segmentinfo for first and
                                         //  second boundary
   SEGTYPE SegType1;
   SEGTYPE SegType2;                     // segmenttype for first and
                                         //  second boundary
   BOOL    fOK = TRUE;                   // processing flag
   PSEGBOUNDARY pCurSB;                  // pt to current segment boundary
// PSEGBOUNDARY pTempSB;                 // temporary pt to segment boundary
   SHORT   sTokenid;                     // Tokenid
   BOOL    fText;                        // is there a text token in assoc. text?
   SEGTYPE SegType;                      // segmenting type     /* 1@KIT1110A */
   BOOL    fIsSelfContainedProtectTag = FALSE; // TRUE this is a self contained tag which should be protected
   BOOL    fProtectSectionTagFound = FALSE; // TRUE the start of a protected section has been detected
   BOOL    fProtectSectionEnds = FALSE; // TRUE a protected section ends at the current tag

   pSegTok = *ppSegTokenEntry;
   pCurSB = *ppCurSegBoundary;
   SegType   = *pSegType;                                       /* 1@KIT1110A */

   memset(&SegType1, 0, sizeof(SegType1));
   memset(&SegType2, 0, sizeof(SegType2));
   memset(&Seginfo1, 0, sizeof(Seginfo1));
   memset(&Seginfo2, 0, sizeof(Seginfo2));
   // calculate segment value depending on segmenting state and tag information
#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        CHAR_W chTemp = pSegTok->pDataStringW[pSegTok->usLength];
        pSegTok->pDataStringW[pSegTok->usLength] = 0;
        fwprintf( hSegLog, L"SetSegInfo: TokenID = %d, Token=\"%s\"\n", pSegTok->sTokenid, pSegTok->pDataStringW );
        pSegTok->pDataStringW[pSegTok->usLength] = chTemp;
      } /* endif */
#endif
    usSegValue =  (SEGSTATE)GetSegValue( pTag[pSegTok->sTokenid],
                                        sSegState);


#ifdef PROTECT_ATTRIBUTES_OF_PROTECTED_TAGS
  if ( pInD->fProtectedSelfContainedTag && isSelfContainedTagEnd( pInD, pSegTok->pDataStringW, pSegTok->usLength, pInD->szCurFormat ) )
    {
      pInD->fProtectMode = FALSE;
      pInD->fProtectedSelfContainedTag = FALSE;
      fProtectSectionEnds = TRUE;
    }
    else 
#endif
    if ( (pInD->iNumOfActiveStartTags > 0) && isXMPScreenStartTag( pInD, pSegTok->pDataStringW, pSegTok->usLength, pInD->szCurFormat, &fIsSelfContainedProtectTag ) )
    {
      pInD->fProtectMode = TRUE;
      fProtectSectionTagFound = TRUE;
    }
    else if ( (pInD->iNumOfActiveEndTags > 0) && isXMPScreenEndTag( pInD, pSegTok->pDataStringW, pSegTok->usLength, pInD->szCurFormat ) )
    {
      pInD->fProtectMode = FALSE;
      fProtectSectionEnds = TRUE;
    }

#ifdef PROTECT_ATTRIBUTES_OF_PROTECTED_TAGS
    if ( fIsSelfContainedProtectTag )
    {
      pInD->fProtectedSelfContainedTag = TRUE;
      pInD->fProtectMode = TRUE;
      fProtectSectionTagFound = TRUE;
    } /* endif */
#endif

    switch (usSegValue)
      {
      case NOASSOC_NODEL_OFF_SEGOFF :
      case NOASSOC_NODEL_OFF_SEGON  :
      case NOASSOC_NODEL_ON_SEGOFF  :
      case NOASSOC_DEL_OFF_SEGOFF   :
      case NOASSOC_DEL_OFF_SEGON    :
         Seginfo1 = NOSEG;
         SegType   = SegType1 = LINGUISTIC;                     /* 1@KIT1110C */
         fSegboundary1 = FALSE;
         break;

      case NOASSOC_NODEL_OFF_NEUTRAL:
      case NOASSOC_DEL_OFF_NEUTRAL  :
         Seginfo1 = NOSEG;
         SegType1 = SegType;
         fSegboundary1 = FALSE;
         break;


      case NOASSOC_NODEL_ON_NEUTRAL :
         Seginfo1 = SKIP;
         SegType1 = SegType;
         fSegboundary1 = FALSE;
         break;

      case NOASSOC_DEL_ON_SEGOFF:
         Seginfo1 = NOSEG;
         SegType =   SegType1 = LINGUISTIC;                     /* 1@KIT1110C */
         fSegboundary1 = TRUE;
         break;

      case NOASSOC_DEL_ON_SEGON  :
      case NOASSOC_DEL_ON_RESET  :
         Seginfo1 = SEG;
         SegType = SegType1 = LINGUISTIC;                       /* 1@KIT1110C */
         fSegboundary1 = TRUE;
         break;

      case NOASSOC_DEL_ON_NEUTRAL:
         Seginfo1 = SEG;
         SegType1 = SegType;
         fSegboundary1 = TRUE;
         break;

      case NOASSOC_DEL_ON_SEGLINE:
         Seginfo1 = SEG;
         SegType = SegType1 = LINEPERLINE;                      /* 1@KIT1110C */
         fSegboundary1 = TRUE;
         break;

      case NOASSOC_DEL_ON_NOSEGMENTING:
         Seginfo1 = SEG;
         SegType = SegType1 = NOSEGMENTING;                      
         fSegboundary1 = TRUE;
         break;

      case NOASSOC_NODEL_ON_SEGON:
         Seginfo1 = SEG;
         SegType1 = SegType;
         fSegboundary1 = FALSE;
         break;

      case ASSOC_NODEL_OFF_SEGOFF:
      case ASSOC_DEL_OFF_SEGOFF:
      case ASSOC_DEL_OFF_SEGON:
         Seginfo1 = NOSEG;
         Seginfo2 = NOSEG;
         SegType1 = LINGUISTIC;
         SegType = SegType2 = LINGUISTIC;                       /* 1@KIT1110C */
         fSegboundary1 = FALSE;
         fSegboundary2 = FALSE;
         break;

      case ASSOC_DEL_OFF_NEUTRAL:
         Seginfo1 = NOSEG;
         Seginfo2 = NOSEG;
         SegType1 = SegType;
         SegType2 = SegType;
         fSegboundary1 = FALSE;
         fSegboundary2 = FALSE;
         break;

      case ASSOC_NODEL_OFF_NEUTRAL:
      case ASSOC_NODEL_ON_NEUTRAL:
         Seginfo1 = SKIP;
         Seginfo2 = SKIP;
         SegType1 = LINGUISTIC;
         SegType2 = SegType;
         fSegboundary1 = FALSE;
         fSegboundary2 = FALSE;
         break;

      case ASSOC_NODEL_ON_SEGOFF:
         Seginfo1 = NOSEG;
         Seginfo2 = SEG;
         SegType1 = LINGUISTIC;
         SegType2 = SegType;
         fSegboundary1 = FALSE;
         fSegboundary2 = FALSE;
         break;

      case ASSOC_DEL_ON_SEGOFF:
         Seginfo1 = NOSEG;
         Seginfo2 = NOSEG;
         SegType1 = LINGUISTIC;
         SegType = SegType2 = LINGUISTIC;                       /* 1@KIT1110C */
         fSegboundary1 = TRUE;
         fSegboundary2 = FALSE;
         break;

      case ASSOC_DEL_ON_SEGON:
      case ASSOC_DEL_ON_RESET:
         Seginfo1 = NOSEG;
         Seginfo2 = SEG;
         SegType1 = LINGUISTIC;
         SegType = SegType2 = LINGUISTIC;                       /* 1@KIT1110C */
         fSegboundary1 = TRUE;
         fSegboundary2 = TRUE;
         break;

      case ASSOC_DEL_ON_NEUTRAL:
         Seginfo1 = NOSEG;
         Seginfo2 = SEG;
         SegType1 = SegType;
         SegType2 = SegType;
         fSegboundary1 = TRUE;
         fSegboundary2 = TRUE;
         break;

      case ASSOC_DEL_ON_SEGLINE:
         Seginfo1 = NOSEG;
         Seginfo2 = SEG;
         SegType1 = LINGUISTIC;
         SegType = SegType2 = LINEPERLINE;                      /* 1@KIT1110C */
         fSegboundary1 = TRUE;
         fSegboundary2 = TRUE;
         break;

      case ASSOC_DEL_ON_NOSEGMENTING:
         Seginfo1 = NOSEG;
         Seginfo2 = SEG;
         SegType1 = LINGUISTIC;
         SegType = SegType2 = NOSEGMENTING;                     
         fSegboundary1 = TRUE;
         fSegboundary2 = TRUE;
         break;

      default:
         // following values should not be possible:
         // NOASSOC_NODEL_OFF_RESET
         // NOASSOC_NODEL_ON_RESET
         // NOASSOC_DEL _OFF_RESET
         // ASSOC_NODEL_OFF_SEGON
         // ASSOC_NODEL_OFF_RESET
         // ASSOC_NODEL_ON_SEGON
         // ASSOC_NODEL_ON_RESET
         // ASSOC_DEL_OFF_RESET
         Seginfo1 = NOSEG;
         Seginfo2 = NOSEG;
         SegType1 = SegType;
         SegType2 = SegType;
         fSegboundary1 = FALSE;
         fSegboundary2 = FALSE;
         break;
      } /* endswitch */

      // ensure that a segment boundary is set when a protection section starts
      if ( fProtectSectionTagFound )
      {
        fSegboundary1 = TRUE;
        Seginfo1 = NOSEG;

        // when there is already a segment boundary at the tag start, ensure that the segment state is set correctly
        if ( pSegTok->pDataStringW == pCurSB->pBoundary)
        {
          pCurSB->XlateState = NOP;
        } /* endif */
      } /* endif */

      // speciall handling at the end of the protected section
      if ( fProtectSectionEnds )
      {
         Seginfo1 = SEG;
         Seginfo2 = SEG;
         fSegboundary1 = TRUE;
         fSegboundary2 = TRUE;
      }

#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        PSZ pszSegValue = "other";
        switch (usSegValue)
        {
          case NOASSOC_NODEL_OFF_SEGOFF : pszSegValue = "NOASSOC_NODEL_OFF_SEGOFF"; break;
          case NOASSOC_NODEL_OFF_SEGON  : pszSegValue = "NOASSOC_NODEL_OFF_SEGON"; break;
          case NOASSOC_NODEL_ON_SEGOFF  : pszSegValue = "NOASSOC_NODEL_ON_SEGOFF"; break;
          case NOASSOC_DEL_OFF_SEGOFF   : pszSegValue = "NOASSOC_NODEL_ON_SEGON"; break;
          case NOASSOC_DEL_OFF_SEGON    : pszSegValue = "NOASSOC_DEL_OFF_SEGON"; break;
          case NOASSOC_NODEL_OFF_NEUTRAL: pszSegValue = "NOASSOC_NODEL_OFF_NEUTRAL"; break;
          case NOASSOC_DEL_OFF_NEUTRAL  : pszSegValue = "NOASSOC_DEL_OFF_NEUTRAL"; break;
          case NOASSOC_NODEL_ON_NEUTRAL : pszSegValue = "NOASSOC_NODEL_ON_NEUTRAL"; break;
          case NOASSOC_DEL_ON_SEGOFF:     pszSegValue = "NOASSOC_DEL_ON_SEGOFF"; break;
          case NOASSOC_DEL_ON_SEGON  :    pszSegValue = "NOASSOC_DEL_ON_SEGON"; break;
          case NOASSOC_DEL_ON_RESET  :    pszSegValue = "NOASSOC_DEL_ON_RESET"; break;
          case NOASSOC_DEL_ON_NEUTRAL:    pszSegValue = "NOASSOC_DEL_ON_NEUTRAL"; break;
          case NOASSOC_DEL_ON_SEGLINE:    pszSegValue = "NOASSOC_DEL_ON_SEGLINE"; break;
          case NOASSOC_NODEL_ON_SEGON:    pszSegValue = "NOASSOC_NODEL_ON_SEGON"; break;
          case ASSOC_NODEL_OFF_SEGOFF:    pszSegValue = "ASSOC_NODEL_OFF_SEGOFF"; break;
          case ASSOC_DEL_OFF_SEGOFF:      pszSegValue = "ASSOC_DEL_OFF_SEGOFF"; break;
          case ASSOC_DEL_OFF_SEGON:       pszSegValue = "ASSOC_DEL_OFF_SEGON"; break;
          case ASSOC_DEL_OFF_NEUTRAL:     pszSegValue = "ASSOC_DEL_OFF_NEUTRAL"; break;
          case ASSOC_NODEL_OFF_NEUTRAL:   pszSegValue = "ASSOC_NODEL_OFF_NEUTRAL"; break;
          case ASSOC_NODEL_ON_NEUTRAL:    pszSegValue = "ASSOC_NODEL_ON_NEUTRAL"; break;
          case ASSOC_NODEL_ON_SEGOFF:     pszSegValue = "ASSOC_NODEL_ON_SEGOFF"; break;
          case ASSOC_DEL_ON_SEGOFF:       pszSegValue = "ASSOC_DEL_ON_SEGOFF"; break;
          case ASSOC_DEL_ON_SEGON:        pszSegValue = "ASSOC_DEL_ON_SEGON"; break;
          case ASSOC_DEL_ON_RESET:        pszSegValue = "ASSOC_DEL_ON_RESET"; break;
          case ASSOC_DEL_ON_NEUTRAL:      pszSegValue = "ASSOC_DEL_ON_NEUTRAL"; break;
          case ASSOC_DEL_ON_SEGLINE:      pszSegValue = "ASSOC_DEL_ON_SEGLINE"; break;
        } /* endswitch */
        fwprintf( hSegLog, L"SetSegInfo: SegValue=%S\n   SegInfo1 = %s, SegInfo2 = %s,\n   SegType1 = %s, SegType2 = %s,\n   bound1 = %s, bound2 = %s\n",
          pszSegValue,
          (Seginfo1 == NOSEG) ? L"NOSEG" : ((Seginfo1 == SKIP ) ? L"SKIP" : L"SEG"),
          (Seginfo2 == NOSEG) ? L"NOSEG" : ((Seginfo2 == SKIP ) ? L"SKIP" : L"SEG"),
          (SegType1 == LINGUISTIC) ? L"LINGUISTIC" : L"LINEPERLINE",
          (SegType2 == LINGUISTIC) ? L"LINGUISTIC" : L"LINEPERLINE",
          fSegboundary1 ? L"TRUE" : L"FALSE",
          fSegboundary2 ? L"TRUE" : L"FALSE" );
      } /* endif */
#endif

   pSegTok->Seginfo = Seginfo1;
   pSegTok->SegType = SegType1;
   if (fSegboundary1 )
      {
      // insert a segment boundary before the tag, if there is not already one
      if ( pSegTok->pDataStringW != pCurSB->pBoundary)
         {
         // make entry in segment boundary table
         pCurSB->pNext = GetEntry(pSegBoundaryTbl);

         if (pCurSB->pNext == NULL)
            fOK = FALSE;
         else
            {
            pCurSB = pCurSB->pNext;
            pCurSB->pBoundary = pSegTok->pDataStringW;
            pCurSB->pNext = NULL;
            if (pTag[pSegTok->sTokenid].BitFlags.fTranslate)
               { // tag has associated text to be translated
                 pCurSB->XlateState = pInD->fProtectMode ? NOP : TODO;
               }
            else
               pCurSB->XlateState = NOP;
#ifdef TSEGM_SEGLOGGING
            if ( hSegLog )
            {
              CHAR_W chTemp = pSegTok->pDataStringW[pSegTok->usLength];
              pSegTok->pDataStringW[pSegTok->usLength] = 0;
              fwprintf( hSegLog, L"SetSegInfo1: Token=(\"%s\") = %s\n", pSegTok->pDataStringW,
                (pCurSB->XlateState == TODO) ? L"ToDo" : L"NOP" );
              pSegTok->pDataStringW[pSegTok->usLength] = chTemp;
            } /* endif */
#endif

            } /* endif */
         } /* endif pDatastring != pBoundary */

      // if tag has associated text to be translated

      if (pTag[pSegTok->sTokenid].BitFlags.fTranslate)
        pCurSB->XlateState = pInD->fProtectMode ? NOP : TODO;
      else
         pCurSB->XlateState = NOP;
#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        CHAR_W chTemp = pSegTok->pDataStringW[pSegTok->usLength];
        pSegTok->pDataStringW[pSegTok->usLength] = 0;
        fwprintf( hSegLog, L"SetSegInfo2: Token=(\"%s\") = %s\n", pSegTok->pDataStringW,
          (pCurSB->XlateState == TODO) ? L"ToDo" : L"NOP" );
        pSegTok->pDataStringW[pSegTok->usLength] = chTemp;
      } /* endif */
#endif

      if (fOK)
         {
         // if the tag does not have Associated Text to be translated,
         // skip attributes and inserts a segment boundary after them all
         fTagEndBoundary =
                  (pTag[pSegTok->sTokenid].Asstext == NOEXPL_TEXT);

         /* skip attribute tokens and set fAttrtodo */
         pSegTok++;
         sTokenid = pSegTok->sTokenid;
         while (sTokenid >= (SHORT)uNumTags)
            {     // token is attribute
            if (pAttribute[sTokenid - (SHORT)uNumTags].BitFlags.fTranslate)
            {
              pCurSB->XlateState = pInD->fProtectMode ? NOP : ATTRTODO;
#ifdef TSEGM_SEGLOGGING
              if ( hSegLog )
              {
                CHAR_W chTemp = pSegTok->pDataStringW[pSegTok->usLength];
                pSegTok->pDataStringW[pSegTok->usLength] = 0;
                fwprintf( hSegLog, L"SetSegInfo3: Token=(\"%s\") = %s\n", pSegTok->pDataStringW,
                  (pCurSB->XlateState == TODO) ? L"ToDo" : L"NOP" );
                pSegTok->pDataStringW[pSegTok->usLength] = chTemp;
              } /* endif */
#endif
            }

            sTokenid = (++pSegTok)->sTokenid;
            } /* endwhile toek is attribute */
         pSegTok--;      // reset token to last attribute

         // make entry in segment boundary table
         if (fTagEndBoundary)
            {
            pCurSB->pNext = GetEntry(pSegBoundaryTbl);
            if (pCurSB->pNext == NULL)
               fOK = FALSE;
            else
               {
               pCurSB = pCurSB->pNext;
               pCurSB->pBoundary = pSegTok->pDataStringW +
                                          pSegTok->usLength;
               pCurSB->XlateState = pInD->fProtectMode ? NOP : TODO; // default setting
#ifdef TSEGM_SEGLOGGING
              if ( hSegLog )
              {
                CHAR_W chTemp = pSegTok->pDataStringW[pSegTok->usLength];
                pSegTok->pDataStringW[pSegTok->usLength] = 0;
                fwprintf( hSegLog, L"SetSegInfo4: Token=(\"%s\") = %s\n", pSegTok->pDataStringW,
                  (pCurSB->XlateState == TODO) ? L"ToDo" : L"NOP" );
                pSegTok->pDataStringW[pSegTok->usLength] = chTemp;
              } /* endif */
#endif
               } /* endif */
            } /* endif tagEndBoundary */
         } /* endif fOK */
      } /* endif fSegBoundary1 */

   if ( usSegValue > ASSOC_NODEL_OFF_SEGOFF && fOK) // tag has Associated Text
      {
      /* skip tokens to dummy token or endoflist */
      fText = FALSE;  // no text token found within associated text yet
      sTokenid = pSegTok->sTokenid;
      while (sTokenid != ENDOFLIST &&
             sTokenid != DUMMYTAG )
      {
         // this is correct, but the information is not used
         if (sTokenid == TEXT_TOKEN )
         {
            fText = TRUE; // found text token within associated text
         } /* endif */

         if (sTokenid >= 0 &&      // token is tag
            sTokenid < (SHORT) uNumTags)
         {
            // no segmentation within associated text
            pSegTok->Seginfo = NOSEG;
         } /* endif */

         sTokenid = (++pSegTok)->sTokenid;
      } /* endwhile token ENDOFLIST, DUMMYTAG */

      // if no text token is encountered in the associated tokens, reset the
      // translateinfo to NOP, even if it is specified as translatable in
      // the tagtable (e.g :toc. may or may not have associated text), there-
      // fore it has to be specified as translatable in the tagtable
// ?? why fSegBoundary1 necessary
//      if (!fText && fSegboundary1 && pCurSB->XlateState == TODO)
      if (!fText && pCurSB->XlateState == TODO)
      {
         pCurSB->XlateState = NOP;
#ifdef TSEGM_SEGLOGGING
          if ( hSegLog )
          {
            fwprintf( hSegLog, L"SetSegInfo5: Token=(none) = %s\n",
              (pCurSB->XlateState == TODO) ? L"ToDo" : L"NOP" );
          } /* endif */
#endif
      } /* endif */

      if (sTokenid == DUMMYTAG)
      {
         pSegTok->Seginfo = Seginfo2;
         pSegTok->SegType = SegType2;
         if (fSegboundary2 )
         {
            if (pSegTok->pDataStringW != pCurSB->pBoundary)
            {
               // make entry in segment boundary table
               pCurSB->pNext = GetEntry(pSegBoundaryTbl);
               if (pCurSB->pNext == NULL)
               {
                  fOK = FALSE;
               }
               else
               {
                  pCurSB = pCurSB->pNext;
                  pCurSB->pBoundary = pSegTok->pDataStringW;
               } /* endif */
            } /* endif pDataString != pBoundary */

            if (fOK)
            {
              pCurSB->XlateState = pInD->fProtectMode ? NOP : TODO; //default setting for text
#ifdef TSEGM_SEGLOGGING
              if ( hSegLog )
              {
                CHAR_W chTemp = pSegTok->pDataStringW[pSegTok->usLength];
                pSegTok->pDataStringW[pSegTok->usLength] = 0;
                fwprintf( hSegLog, L"SetSegInfo6: Token=(\"%s\") = %s\n", pSegTok->pDataStringW,
                  (pCurSB->XlateState == TODO) ? L"ToDo" : L"NOP" );
                pSegTok->pDataStringW[pSegTok->usLength] = chTemp;
              } /* endif */
#endif
            }

         } /* endif fSegBoundary2 */
      }  /*-- endif token Dummy tag ----*/
      else  /* token is endoflist */
         pSegTok--;

      } /* endif tag has associated text */

   if (fOK)
      {
      *ppSegTokenEntry = pSegTok;
      *ppCurSegBoundary = pCurSB;
      *pSegType   = SegType;                                    /* 1@KIT1110A */
      } /* endif */

    return (fOK);
   } /* end of SetSegInfo */

/*
.cp
************* Get Segmenting Value ********************************************
*                                                                             *
*  Here the segmenting value of a tag is determined using its  attribute      *
*  info and the current segmenting state. The possible segmenting values      *
*  are listed in the enumeration SEGSTATE in eqftamnl.h                       *
******************************************************************************/

static
USHORT GetSegValue(TAG Tag,                          // tag
                   SHORT sSegState)                  //segmenting state
   {
   USHORT usSegValue;                                // segmenting value

   // calculate value for Segmenting Info
   if (Tag.Asstext == NOEXPL_TEXT)
      usSegValue = 0;
   else
      usSegValue = 1;

   usSegValue = usSegValue << 1;

   if (Tag.Tagtype != STARTNEUTRAL_TAG &&
       Tag.Tagtype != ENDNEUTRAL_TAG)
      {
      usSegValue += 1;
      }

   usSegValue = usSegValue << 1;

   if (sSegState == 1)
      {
      usSegValue += 1;
      } /* endif */

   usSegValue = usSegValue * 6;

   usSegValue = usSegValue + Tag.Segbreak;
   return (usSegValue);
} /* end of GetSegValue */

/*
.cp
****************** Get New Entry from Segmenting Boundary Table ***************/
static
PSEGBOUNDARY GetEntry (SEGBOUNDARYTBL *pSegBoundaryTbl)
{
   PSEGBOUNDARY  pEntry = NULL;        // new entry to be returned
   BOOL fOK = TRUE;                    // success indicator

   if ( pSegBoundaryTbl->usNumCurEntry == 1 )  // allocated memory all used
   {
      fOK = UtlAlloc( (PVOID *) &pEntry,
                      0L,
                      (LONG) MAX_ALLOC_SEGTOKENS * (LONG)(sizeof (SEGBOUNDARY)),
                      ERROR_STORAGE);
      if ( fOK )
      {                                      // concatenate them with old list
         pSegBoundaryTbl->usNumCurEntry = MAX_NUM_SEGTOKENS;
         pSegBoundaryTbl->pCurEntry = pEntry;
         pSegBoundaryTbl->pCurEntry++;
         pEntry->pNext = NULL;
         pEntry->pBoundary = NULL;
      } /* endif */
   }
   else                                      // get next entry from memory
   {
     pEntry = pSegBoundaryTbl->pCurEntry;
     pEntry->pNext = NULL;
     pEntry->pBoundary = NULL;
     pSegBoundaryTbl->pCurEntry++;

   } /* endif */
   pSegBoundaryTbl->usNumCurEntry --;          // decrease number of available
   return (pEntry);                            // entries
}  /* end of GetEntry */

/*----------------------------------------------------------------------------*/
/* NAME: EQFTolstComplex                                                      */
/*                                                                            */
/* Purpose : to segment further text segments determined by tag segmentation  */
/*           using natural language functions.                                */
/*           a super text segment is built skipping all SKIP token and this   */
/*           last is passed to tolstoy                                        */
/*                                                                            */
/* input    :  pInD pointer to instance data area that contains all static    */
/*                  data;                                                     */
/*                                                                            */
/*             pFirstEntrySeg pointer to the first element in the segment     */
/*                  boundary table                                            */
/*                                                                            */
/* output   :  the segment boundary table is updated;it returns TRUE if the   */
/*             operation was completed succesfully                            */
/*                                                                            */
/* Called by        :  EQFTextSegmentation                                    */
/*                                                                            */
/* Calling hierarchy:                                                         */
/*        EQFTolstComplex                                                     */
/*                                                                            */
/* pseudologic:                                                               */
/*                                                                            */
/* EQFTolstComplex                                                            */
/*   BEGIN                                                                    */
/*     fSegmenting = FALSE                                                    */
/*     set pointer to token (with segm info) list at the beginning            */
/*     WHILE current token is not ENDOFLIST                                   */
/*        WHILE Token is not TEXT and not ENDOFLIST                           */
/*           IF Token is TAG or DUMMYTAG and Seginfo != SKIP  THEN            */
/*              IF fSegmenting and some data in Tolstoy buffer THEN           */
/*                 call SubstitbyBlank to replace CR/LF with blanks           */
/*                 call TolstSegmentation  to further segment text in         */
/*                      buffer                                                */
/*                 clear buffer                                               */
/*              ENDIF                                                         */
/*              IF SegInfo = SEG THEN                                         */
/*                 fSegmenting = TRUE                                         */
/*              ENDIF                                                         */
/*           ENDIF                                                            */
/*           Get next token                                                   */
/*        ENDWHILE                                                            */
/*        IF Token is TEXT THEN                                               */
/*           IF fSegmenting THEN                                              */
/*              IF buffer is empty THEN                                       */
/*                 remember first token in working variable                   */
/*              END                                                           */
/*              Add string to the tolstoy buffer                              */
/*           ENDIF                                                            */
/*           Get next token                                                   */
/*        ENDIF                                                               */
/*     ENDWHILE                                                               */
/*     IF the tolstoy buffer is not empty THEN                                */
/*        call TolstSegmentation                                              */
/*     ENDIF                                                                  */
/*   END                                                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/
BOOL EQFTolstComplex (
      HWND           hwnd,                // handle window instance
      PTAINSTDATA    pInD,                // analysis instance data
      PSEGBOUNDARY   pFirstEntrySeg)      // pointer to segment list
{
   PTOKENENTRYSEG  pFirstTokenEntry = NULL;      // pointer to start of text string
   PTOKENENTRYSEG  pSegTokenEntry;        // pointer to start of text string

   PSEGBOUNDARY    pCurSegBoundary;       // pointer to current entry in segment
                                          // boundary list
   BOOL            fOK = TRUE;            // processing state
   BOOL            fSegmenting;           // shall follow. text be segmented
   SEGTYPE         SegType = LINGUISTIC;  // type of segmenting
   USHORT          uNumTags;              // number of tags in tagtable
   PSZ_W           pTolstTextBuffer;      // buffer for string to be passed to
                                          // tolstoy
   PSZ_W           pTolstBufferStart;     // start of tolstoy buffer
   SHORT           sTokenid;              // TokenId
   BOOL    fIsSelfContainedProtectTag = FALSE; // TRUE this is a self contained tag which should be protected

   pCurSegBoundary = pFirstEntrySeg;
   pSegTokenEntry = pInD->pSegTokenList;
   uNumTags = pInD->pLoadedTable->pTagTable->uNumTags;
   pTolstBufferStart = pInD->TolstControl.pTextBuffer;

   pTolstTextBuffer = pTolstBufferStart;
   fSegmenting = FALSE;


   /*******************************************************************/
   /* Flag bookmaster variables (tag starting with an ampersand) as   */
   /* text (using fIsText flag of SegTokenEntry)                      */
   // GQ: treat inline tags with a class of CLS_VAR in the same way
   //     as the bookmaster variables
   // GQ: the handling for tags starting with an ampersand has been
   //     removed as it caused problems with tags in IBMXMDCR
   //     It has to be ensured that all tags representing text variables
   //     are flagged correctly as CLS_VAR!
   while ( fOK && (pSegTokenEntry->sTokenid != ENDOFLIST) )
   {
     if ( (pSegTokenEntry->Seginfo == SKIP) &&
          (pSegTokenEntry->usLength != 0)   &&
          ( (pSegTokenEntry->ClassId == CLS_VAR) ||
            (pSegTokenEntry->ClassId == CLS_TRANSLVAR) ||
            (pSegTokenEntry->ClassId == CLS_WHITESPACE) ) )
     {
       pSegTokenEntry->fIsText = TRUE;
     } /* endif */
     pSegTokenEntry++;
   } /* endwhile */
   pSegTokenEntry = pInD->pSegTokenList;

   sTokenid = pSegTokenEntry->sTokenid;
   while (sTokenid != ENDOFLIST && fOK)
   {
      // get next text unit
      while ( (sTokenid != TEXT_TOKEN) && !pSegTokenEntry->fIsText && (sTokenid != ENDOFLIST) && fOK )
      {
         if ((sTokenid >= 0 && sTokenid < (SHORT)uNumTags) || (sTokenid == DUMMYTAG))
         {
            if (pSegTokenEntry->Seginfo != SKIP)
            {

              if (fSegmenting && pTolstTextBuffer != pTolstBufferStart)
              {  // prepare text block  and pass it to tolstoy
                *pTolstTextBuffer = EOS;

                if ( SegType == NOSEGMENTING )
                {
                  // nothing to do
                }
                else if ( SegType == LINEPERLINE )
                {
                  fOK = LineByLineSegmentation( pInD, pTolstBufferStart,
                                          &(pInD->SegBoundaryTbl),
                                          pFirstTokenEntry,
                                          pFirstEntrySeg);
                }
                else
                {
#ifndef DISABLE_SUBSTITBYBLANK
                  // substitute CR LF by BLANK before passing the text to TOLSTOY
                  SubstitbyBlank(pTolstBufferStart);
#endif

                  fOK = TolstSegmentation(pInD, pTolstBufferStart,
                                          &(pInD->SegBoundaryTbl),
                                          pFirstTokenEntry,
                                          &(pInD->TolstControl),
                                          pFirstEntrySeg);
                } /* endif */

                // initialize Tolstoy Buffer
                pTolstTextBuffer = pTolstBufferStart;
              } /* endif */

              fSegmenting = (pSegTokenEntry->Seginfo == SEG);
              SegType     =  pSegTokenEntry->SegType;
           } /* endif */
         } /* endif */

         if ( pInD->fProtXmp || pInD->fProtCodeBlock || pInD->fProtMeta || pInD->fProtMsgNum || pInD->fProtScreen )
         {
           if ( isXMPScreenStartTag( pInD, pSegTokenEntry->pDataStringW, pSegTokenEntry->usLength, pInD->szCurFormat, &fIsSelfContainedProtectTag  ) )
           {
             pInD->fProtectMode = TRUE;
           }
           else if ( isXMPScreenEndTag( pInD, pSegTokenEntry->pDataStringW, pSegTokenEntry->usLength, pInD->szCurFormat ) )
           {
             pInD->fProtectMode = FALSE;
           } /* endif */
         }
         sTokenid = (++pSegTokenEntry)->sTokenid;
         fOK = TAGiveCntrl(hwnd, pInD);
      } /* endwhile */

     if ( fOK && ((sTokenid == TEXT_TOKEN) || pSegTokenEntry->fIsText) )
     {
        if (fSegmenting)
        {
           if (pTolstTextBuffer == pTolstBufferStart)
           {
              pFirstTokenEntry = pSegTokenEntry; // remember first token in buffer
           } /* endif */

           /***********************************************************/
           /* Fix @01                                                 */
           /*                                                         */
           /* Fix for "inline-tag in line of itself breaks segment"   */
           /* problem:                                                */
           /*                                                         */
           /* Problem is caused by removing the inline-tag and leaving*/
           /* an empty line (which then is treated as segment break). */
           /* If the text in the Tolstoy buffer ends with CRLF and    */
           /* new text starts with CRLF (or LF) it is most likely     */
           /* that there were SKIP tags between these text blocks     */
           /* which have been removed. If this happens the CRLF at    */
           /* the end of the Tolstoy text buffer is replaced by " ,"  */
           /* which will avoid the "text break by empty line".        */
           /***********************************************************/
/*@01*/    if ( (sTokenid == TEXT_TOKEN) && (pTolstTextBuffer != pTolstBufferStart) )
/*@01*/    {
/*@01*/           if ( ( (pTolstTextBuffer[-1] == CR) ||
/*@01*/             (pTolstTextBuffer[-1] == LF) )  &&
/*@01*/           ( (pSegTokenEntry->pDataStringW[0] == CR) ||
/*@01*/             (pSegTokenEntry->pDataStringW[0] == LF) ) )
/*@01*/      {
/*@01*/        /*******************************************************/
/*@01*/        /* replace CRLF at end of Tolstoy buffer with " ,"     */
/*@01*/        /*******************************************************/
/*@01*/        if ( pTolstTextBuffer[-1] == CR )
/*@01*/        {
/*@01*/          // single CR at end of buffer
/*@01*/          pTolstTextBuffer[-1] = ' ';
/*@01*/        }
/*@01*/        else
/*@01*/        {
/*@01*/          // LF at end of buffer, now check for CRLF
/*@01*/          if ( pTolstTextBuffer > (pTolstBufferStart + 1) )
/*@01*/          {
/*@01*/            if ( pTolstTextBuffer[-2] == CR )
/*@01*/            {
/*@01*/              // buffer ends with CRLF
/*@01*/              pTolstTextBuffer[-2] = ' ';
/*@01*/              pTolstTextBuffer[-1] = ',';
/*@01*/            }
/*@01*/            else
/*@01*/            {
/*@01*/              // buffer ends with single LF
/*@01*/              pTolstTextBuffer[-1] = ' ';
/*@01*/            } /* endif */
/*@01*/          }
/*@01*/          else
/*@01*/          {
/*@01*/            // buffer contains only a single LF
/*@01*/            pTolstTextBuffer[-1] = ' ';
/*@01*/          } /* endif */
/*@01*/        } /* endif */
/*@01*/      } /* endif */
/*@01*/    } /* endif */

           // copy string to TOLSTOY string buffer
           if ( sTokenid == TEXT_TOKEN )
           {
             memcpy((PBYTE)pTolstTextBuffer, (PBYTE)pSegTokenEntry->pDataStringW,
                    pSegTokenEntry->usLength * sizeof(CHAR_W));
           }
           else
           {
             UTF16memset(pTolstTextBuffer, 'x', pSegTokenEntry->usLength);
             if ( pSegTokenEntry->ClassId == CLS_WHITESPACE )
             {
               // replace first char with space
               pTolstTextBuffer[0] = L' ';
             }
             else
             {
               // replace first char with capitalized X
               pTolstTextBuffer[0] = L'X';
             } /* endif */
           } /* endif */

           pTolstTextBuffer += pSegTokenEntry->usLength;
        } /* endif */

        sTokenid = (++pSegTokenEntry)->sTokenid;
        fOK = TAGiveCntrl(hwnd, pInD);
     } /* endif */
   } /* endwhile */

   // check if there is still some text to be segmented in the tolstoy buffer
   if (fOK && fSegmenting && pTolstTextBuffer != pTolstBufferStart)
   {
     *pTolstTextBuffer = EOS;

     if ( (pSegTokenEntry->sTokenid == ENDOFLIST) &&            /* 8@KIT1110A */
          (pSegTokenEntry != pInD->pSegTokenList) )
     {
       /***************************************************************/
       /* We are at the end of the segtoken list, use segmenting type */
       /* of last data token to determine type of tokenization        */
       /***************************************************************/
       pSegTokenEntry--;
     } /* endif */

     if ( pSegTokenEntry->SegType == NOSEGMENTING )
     {
       // nothing to do
     }
     else if ( pSegTokenEntry->SegType == LINEPERLINE )
     {
       fOK = LineByLineSegmentation( pInD, pTolstBufferStart,
                               &(pInD->SegBoundaryTbl),
                               pFirstTokenEntry,
                               pFirstEntrySeg);
     }
     else
     {
#ifndef DISABLE_SUBSTITBYBLANK
       // substitute CR LF by BLANK before passing the text to TOLSTOY
        SubstitbyBlank(pTolstBufferStart);
#endif

       fOK = TolstSegmentation( pInD, pTolstBufferStart,
                               &(pInD->SegBoundaryTbl),
                               pFirstTokenEntry,
                               &(pInD->TolstControl),
                               pFirstEntrySeg);
     } /* endif */
   } /* endif */

   return (fOK);
}

/*----------------------------------------------------------------------------*/
/* NAME: TolstSegmentation                                                    */
/*                                                                            */
/* TolsSegmentation: the purpose of this function is to break a text segment  */
/* as determined by token analysis into real linguistics segments. To do this */
/* the whole segment is sent to the National Language Services for tokeniza-  */
/* tion and then the complete token list (that contains single word or symbol */
/* tokens) is scanned to identify end of senteces. When such end is found,    */
/* a segment boundary is inserted in the segment boundary table and the poin- */
/* ters in the token list are updated.                                        */
/* input   :                                                                  */
/*           pointer to a string containing the segment to analyze            */
/*           pointer to Segment boundary table                                */
/*           pointer to the current segment boundary item                     */
/*           pointer to the current token                                     */
/*           pointer to Tolstoy control area                                  */
/* output  : some entries in the segment boundary table can be linked         */
/*           bearing to further segmentation                                  */
/*                                                                            */
/* Called by        :  EQFTolstComplex                                        */
/*                                                                            */
/* Calling hierarchy:                                                         */
/*        EQFTolstComplex                                                     */
/*                                                                            */
/* pseudologic:                                                               */
/*                                                                            */
/* TolstSegmentation                                                          */
/*   BEGIN                                                                    */
/*     Skip spaces at beginning of buffer                                     */
/*     REPEAT                                                                 */
/*        Call NlpComplex with buffer                                         */
/*        set pointer at beginning of Tolstoy token list                      */
/*        WHILE token list is not ended                                       */
/*           Call NlpNumJoined to know the number of token in joined token    */
/*           Call NlpTypToken                                                 */
/*           CASE token type OF                                               */
/*             Next sentence:                                                 */
/*                 IF not at the beginning of the list THEN                   */
/*                    Scan the segment boundary list while the current        */
/*                    boundary pointer is greater than the next segment       */
/*                    boundary pointer.                                       */
/*                    IF current text pointer != segment boundary tx.p. THEN  */
/*                       Insert a new segment boundary with the current       */
/*                       text pointer.                                        */
/*                    ENDIF                                                   */
/*                 ENDIF                                                      */
/*             Text:                                                          */
/*                 IF token is not joined token THEN                          */
/*                    get lenght of token                                     */
/*                    adjust current text pointer                             */
/*                    get length of following delimiters                      */
/*                    adjust current text pointer                             */
/*                 ELSE                                                       */
/*                    get the string lenght of joined token                   */
/*                    adjust current text pointer                             */
/*                    get length of delimiters                                */
/*                    adjust current text pointer                             */
/*                 ENDIF                                                      */
/*           ENDCASE                                                          */
/*           Get the next token from the list                                 */
/*        ENDWHILE                                                            */
/*     UNTIL requested by NlpComplex and not error                            */
/*   END                                                                      */
/*----------------------------------------------------------------------------*/
static
BOOL TolstSegmentation
     (
      PTAINSTDATA    pInD,             // pointer to instance data
      PSZ_W          pTolstBuffer,
      SEGBOUNDARYTBL * pSegBoundaryTbl,   // pointer to segment boundary table
      PTOKENENTRYSEG pFirstSegToken,      // pointer to segmenting tokenlist
      PTOLSTCONTROL  pTolstControl,        // pointer to control info
      PSEGBOUNDARY   pFirstEntrySeg)      // pointer to first entry in segment
   {
   PTOKENENTRYSEG  pCurTokenEntry;        // current token in tokenlist
   BOOL            fOK = TRUE;            // processing state
   PSZ_W           pText;                 // pointer to current text position
   USHORT          usLength;              // length of token
   USHORT          usRc;                  // return code from tolstoy calls
   PSEGBOUNDARY    pCurSegBoundary;       // pointer to current segment boundary
   PSEGBOUNDARY    pSegBoundaryWork;      // work pointer
   USHORT          usCurLength = 0;       // current length of segmenting token
   PFLAGOFFSLIST   pTerm;              // pointer for term list processing
   USHORT          usStartOffs;        // start offset of segment data


   pCurTokenEntry = pFirstSegToken;
   pCurSegBoundary = pFirstEntrySeg;
   pText = pCurTokenEntry->pDataStringW;

   // skip spaces at start of tolstoy buffer to prevent generation of empty
   // segments by tolstoy
   usLength = 0;
   while (*pTolstBuffer == BLANK )
      {
      usLength ++;
      pTolstBuffer ++;
      } /* endwhile tolstoy buffer char is blank */

   // move pText to same position in original text buffer
   AdjustTextPosition(&pText, &pCurTokenEntry, &usCurLength, usLength);
   // end of skip spaces at beginning


   /*******************************************************************/
   /* Use MorphTokenize to get a flagged token list, only the         */
   /* new sentence tokens are of interest                             */
   /*******************************************************************/
   usRc = NO_ERROR;
#ifdef TSEGM_SEGLOGGING
    if ( hSegLog )
    {
      fwprintf( hSegLog, L"TolstSegmentation Text=>>>%s<<<", pTolstBuffer );
    } /* endif */
#endif
   usRc = MorphTokenizeW( pTolstControl->sLangID,
                         pTolstBuffer,
                         &(pTolstControl->usTokenListSize),
                         (PVOID *)&(pTolstControl->pTokenList),
                         MORPH_FLAG_OFFSLIST,  pTolstControl->ulOemCP );
   if ( usRc == NO_ERROR )
   {
#ifdef TSEGM_SEGLOGGING
      if ( hSegLog )
      {
        fwprintf( hSegLog, L"\nReturned tokens:%c", ' ' );
        pTerm  = (PFLAGOFFSLIST)pTolstControl->pTokenList;
        while ( fOK && ((pTerm->usOffs != 0) ||
                         (pTerm->usLen  != 0) ||
                         (pTerm->lFlags != 0L ) ) )
        {
           fwprintf( hSegLog, L"(%u,%u,%ld),", pTerm->usOffs, pTerm->usLen, pTerm->lFlags );
           pTerm++;
        } /* endwhile */
        fwprintf( hSegLog, L"%c\n", L' ' );
      } /* endif */
#endif

     pTerm  = (PFLAGOFFSLIST)pTolstControl->pTokenList;
     usStartOffs = pTerm->usOffs;
     while ( fOK && ((pTerm->usOffs != 0) ||
                     (pTerm->usLen  != 0) ||
                     (pTerm->lFlags != 0L ) ) )
     {
       if ( pTerm->lFlags & TF_NEWSENTENCE )
       {
         /***********************************************************/
         /* Segment starts at next term                             */
         /***********************************************************/
         while( ( pTerm->lFlags & TF_NEWSENTENCE ) &&    /* 1-14-14 */
                ( pTerm->usLen != 0L ) ) {               /* 1-29-14 */
            pTerm++;
         }

         /***********************************************************/
         /* Insert segment break into input text except at beginning*/
         /***********************************************************/
         if ( pTerm->usOffs != 0 )
         {
           // search appropriate place in list
#ifdef TSEGM_SEGLOGGING
            if ( hSegLog )
            {
              fwprintf( hSegLog, L"TolstSegmentation: splitting text at=\n%s\n", pTolstBuffer + pTerm->usOffs );
              fwprintf( hSegLog, L"AdjustText for %u (%u-%u) characters (%u), pText = \"%s\"\n", (USHORT)(pTerm->usOffs - usStartOffs), (USHORT)pTerm->usOffs,(USHORT)usStartOffs,wcslen(pText), pText );
              fflush( hSegLog );
            } /* endif */
#endif
           AdjustTextPosition( &pText, &pCurTokenEntry,
                               &usCurLength, (USHORT)(pTerm->usOffs - usStartOffs) );

#ifdef TSEGM_SEGLOGGING
            if ( hSegLog )
            {
              fwprintf( hSegLog, L"TolstSegmentation: after AdjusttextPos pos=\"%s\"\n", pText );
              fflush( hSegLog );
            } /* endif */
#endif
           // do not split CRLF while inserting segment boundary
           if ( (pText[0] == LF) && ( *(pText - 1) == CR) )
           {
             // add LF to current segment
             AdjustTextPosition( &pText, &pCurTokenEntry, &usCurLength, 1 );
             pTerm->usOffs++;
           } /* endif */

           while (pCurSegBoundary->pNext != NULL &&
                  pCurSegBoundary->pNext->pBoundary < pText )
           {
              pCurSegBoundary = pCurSegBoundary->pNext;
           }

           if (pCurSegBoundary->pBoundary != pText)
           {  // no segment boundary set yet
              // insert segment boundary
             pSegBoundaryWork = pCurSegBoundary->pNext;
             pCurSegBoundary->pNext = GetEntry(pSegBoundaryTbl);

             if (pCurSegBoundary->pNext == NULL)
             {
               fOK = FALSE;
             }
             else
             {
               pCurSegBoundary = pCurSegBoundary->pNext;
               pCurSegBoundary->pBoundary = pText;
               pCurSegBoundary->XlateState = pInD->fProtectMode ? NOP : TODO;
#ifdef TSEGM_SEGLOGGING
              if ( hSegLog )
              {
                fwprintf( hSegLog, L"TolstSegmentation: State = %s\n",
                  (pCurSegBoundary->XlateState == TODO) ? L"ToDo" : L"NOP" );
              } /* endif */
#endif
               pCurSegBoundary->pNext = pSegBoundaryWork;
               }
             } /* endif no segment boundary set yet */
         } /* endif beginning of text */
         usStartOffs = pTerm->usOffs;
       } /* endif */

       if ( (pTerm->usOffs != 0) ||
            (pTerm->usLen  != 0) ||
            (pTerm->lFlags != 0L ) )
       {
         pTerm++;
       } /* endif */
     } /* endwhile */
   } /* endif */

   return (fOK);
} /* end of TolstSegmentation */

/*----------------------------------------------------------------------------*/
/* NAME: LineByLineSegmentation                                               */
/*                                                                            */
/* LinyByLineSegmentation: the purpose of this function is to break a text    */
/* segement as determined by token analysis into line segments. To do this    */
/* the input text is scanned for line feeds and when a linefeed is found,     */
/* a segment boundary is inserted in the segment boundary table and the poin- */
/* ters in the token list are updated.                                        */
/* input   :                                                                  */
/*           pointer to a string containing the segment to analyze            */
/*           pointer to Segment boundary table                                */
/*           pointer to the current segment boundary item                     */
/*           pointer to the current token                                     */
/* output  : some entries in the segment boundary table can be linked         */
/*           bearing to further segmentation                                  */
/*                                                                            */
/* Called by        :  EQFTolstComplex                                        */
/*                                                                            */
/* Calling hierarchy:                                                         */
/*        EQFTolstComplex                                                     */
/*                                                                            */
/* pseudologic:                                                               */
/*                                                                            */
/* LineByLineSegmentation                                                     */
/*   BEGIN                                                                    */
/*     REPEAT                                                                 */
/*        set pointer at beginning of Tolstoy buffer                          */
/*        WHILE not end of buffer                                             */
/*           IF current char is line feed:                                    */
/*             IF not at the beginning of the list THEN                       */
/*                Scan the segment boundary list while the current            */
/*                boundary pointer is greater than the next segment           */
/*                boundary pointer.                                           */
/*                IF current text pointer != segment boundary tx.p. THEN      */
/*                   Insert a new segment boundary with the current           */
/*                   text pointer.                                            */
/*                ENDIF                                                       */
/*             ENDIF                                                          */
/*           ELSE                                                             */
/*             continue with next character                                   */
/*                 ENDIF                                                      */
/*           ENDIF                                                            */
/*        ENDWHILE                                                            */
/*     UNTIL end reached or error                                             */
/*   END                                                                      */
/*----------------------------------------------------------------------------*/
static
BOOL LineByLineSegmentation
(
  PTAINSTDATA    pInD,             // pointer to instance data
  PSZ_W           pTolstBuffer,
  SEGBOUNDARYTBL * pSegBoundaryTbl,    // pointer to segment boundary table
  PTOKENENTRYSEG pFirstSegToken,       // pointer to segmenting tokenlist
  PSEGBOUNDARY   pFirstEntrySeg        // pointer to first entry in segment
)
{
   PTOKENENTRYSEG  pCurTokenEntry;     // current token in tokenlist
   BOOL            fOK = TRUE;         // processing state
   PSZ_W           pText;              // pointer to current text position
   USHORT          usLength;           // length of token
   PSEGBOUNDARY    pCurSegBoundary;    // pointer to current segment boundary
   PSEGBOUNDARY    pSegBoundaryWork;   // work pointer
   USHORT          usCurLength = 0;    // current length of segmenting token

#ifdef TSEGM_SEGLOGGING
    if ( hSegLog )
    {
      fwprintf( hSegLog, L"LineSegmentation Text=>>>%s<<<", pTolstBuffer );
    } /* endif */
#endif

   pCurTokenEntry          = pFirstSegToken;
   pCurSegBoundary         = pFirstEntrySeg;
   pText                   = pCurTokenEntry->pDataStringW;

   do
   {
     usLength = 0;
     while ( (*pTolstBuffer != LF) && (*pTolstBuffer != EOS) )
     {
        usLength++;
        pTolstBuffer++;
     } /* endwhile tolstoy buffer char is not end of buffer and not linefeed */

     if ( *pTolstBuffer == LF )
     {
       /***************************************************************/
       /* Skip line feed character                                    */
       /***************************************************************/
       usLength++;
       pTolstBuffer++;

       /***************************************************************/
       /* Adjust text position                                        */
       /***************************************************************/
       AdjustTextPosition(&pText, &pCurTokenEntry, &usCurLength, usLength);

       /***************************************************************/
       /***************************************************************/
       /* Insert segment break into input text                        */
       /***************************************************************/
       /***************************************************************/

       /***************************************************************/
       /* Search appropriate place in list                            */
       /***************************************************************/
       while ( (pCurSegBoundary->pNext != NULL) &&
                (pCurSegBoundary->pNext->pBoundary < pText) )
       {
         pCurSegBoundary = pCurSegBoundary->pNext;
       } /* endwhile */

       /***************************************************************/
       /* Insert segment boundary if none is set at this position     */
       /***************************************************************/
       if ( pCurSegBoundary->pBoundary != pText )
       {
         pSegBoundaryWork = pCurSegBoundary->pNext;
         pCurSegBoundary->pNext = GetEntry(pSegBoundaryTbl);

         if ( pCurSegBoundary->pNext == NULL )
         {
           fOK = FALSE;
         }
         else
         {
           pCurSegBoundary = pCurSegBoundary->pNext;
           pCurSegBoundary->pBoundary = pText;
           pCurSegBoundary->XlateState = pInD->fProtectMode ? NOP : TODO;
#ifdef TSEGM_SEGLOGGING
            if ( hSegLog )
            {
              fwprintf( hSegLog, L"LineByLineSegmentation: State = %s\n",
                (pCurSegBoundary->XlateState == TODO) ? L"ToDo" : L"NOP" );
            } /* endif */
#endif
           pCurSegBoundary->pNext = pSegBoundaryWork;
         } /* endif */
       } /* endif no segment boundary set yet */
     } /* endif */

   } while ( fOK && (*pTolstBuffer != EOS) ); /* enddo */

   return (fOK);

} /* end of function LineByLineSegmentation */

/*
.cp
********************** AdjustTextPosition **************************************
*                                                                              *
*   determine new text position in input text by advancing usTextLen           *
*   characters in text. If there are tags in the input text, they are skipped  *
*   the position in the segmenting tokenlist is adjusted, too and the current  *
*   position within the token (pusCurLength)                                   *
*******************************************************************************/

static
VOID AdjustTextPosition(
     PSZ_W           *ppText,           // pointer to text
     PTOKENENTRYSEG  *ppSegTokenEntry,  // pointer to tokenlist
     USHORT          *pusCurLength,     // length of token
     USHORT          usTextLength)      // number of text characters
                                        // to be advanced

/* synchronize text positions between Tolstoy buffer and Tokenlist           */

{
   ULONG         ulCurLength;               // current token length
   PTOKENENTRYSEG  pSegTokenEntry;            // segmenting token
   BOOL            fStop = FALSE;

   ulCurLength = *pusCurLength;
   pSegTokenEntry = *ppSegTokenEntry;

   ulCurLength += usTextLength;              // advance tokenlength

   if (ulCurLength > pSegTokenEntry->usLength) // new length exceeds token
   {
      ulCurLength -= pSegTokenEntry->usLength; // length after skipping token
      pSegTokenEntry++;

//    // skip all tokens until next text token
//    while ( (pSegTokenEntry->sTokenid != TEXT) && !pSegTokenEntry->fIsText )
//    {
//       pSegTokenEntry++;
//       if ( ((pSegTokenEntry->sTokenid == TEXT) || pSegTokenEntry->fIsText) &&
//           pSegTokenEntry->usLength < ulCurLength)
//       {
//          ulCurLength -= pSegTokenEntry->usLength; // length after skipping
//          pSegTokenEntry++; // skip text token
//       } /* endif */
//    } /* endwhile */
//
//    *ppText = pSegTokenEntry->pDataString + ulCurLength; // set new position

      /****************************************************************/
      /* loop through list until we find matching position ...        */
      /****************************************************************/
      fStop = FALSE;
      while ( (pSegTokenEntry->sTokenid != ENDOFLIST) && !fStop )
      {
        if ( (pSegTokenEntry->sTokenid == TEXT_TOKEN) ||
             (pSegTokenEntry->fIsText && (pSegTokenEntry->ClassId != CLS_WHITESPACE)) )
        {
          if ( pSegTokenEntry->usLength < ulCurLength )
          {
            ulCurLength -= pSegTokenEntry->usLength;
            pSegTokenEntry++;
          }
          else
          {
            fStop = TRUE;
          } /* endif */
        }
        else if ( pSegTokenEntry->ClassId == CLS_WHITESPACE )
        {
          // special care has to be taken for this kind of token:
          // the new sentence info splits this token as the first
          // character of the token has been replaced by blank
#ifdef TSEGM_SEGLOGGING
            if ( hSegLog )
            {
              CHAR_W chTemp = pSegTokenEntry->pDataStringW[pSegTokenEntry->usLength];
              pSegTokenEntry->pDataStringW[pSegTokenEntry->usLength] = 0;
              fwprintf( hSegLog, L"AdjustTextPosition: Skipping whitespace tag\"%s\"\n", pSegTokenEntry->pDataStringW );
              pSegTokenEntry->pDataStringW[pSegTokenEntry->usLength] = chTemp;
            } /* endif */
#endif
          if ( pSegTokenEntry->usLength < ulCurLength )
          {
            ulCurLength -= pSegTokenEntry->usLength;
            pSegTokenEntry++;
          }
          else
          {
            // stop at this token but adjust ulCurLength
            if ( ulCurLength == 1 ) ulCurLength = 0;
            fStop = TRUE;
          } /* endif */
        }
        else
        {
#ifdef TSEGM_SEGLOGGING
            if ( hSegLog )
            {
              CHAR_W chTemp = pSegTokenEntry->pDataStringW[pSegTokenEntry->usLength];
              pSegTokenEntry->pDataStringW[pSegTokenEntry->usLength] = 0;
              fwprintf( hSegLog, L"AdjustTextPosition: Skipping inline tag\"%s\"\n", pSegTokenEntry->pDataStringW );
              pSegTokenEntry->pDataStringW[pSegTokenEntry->usLength] = chTemp;
            } /* endif */
#endif
          pSegTokenEntry++;
        } /* endif */
      } /* endwhile */
      *ppText = pSegTokenEntry->pDataStringW + ulCurLength; // set new position
   }
   else
   {
      *ppText += usTextLength;  // set new text position
   } /* endif */
   *ppSegTokenEntry = pSegTokenEntry;  // pass back new token

   *pusCurLength = (USHORT)ulCurLength;        // and new position within token
} /* end of adjusttextposition */

/*
.cp
********************** CalcLastColumnPos  **************************************
*                                                                              *
*   determine last column position in the segmented input text                 *
*                                                                              *
*******************************************************************************/

static
VOID CalcLastColumnPos(PSZ_W  pTextEnd,         // pointer behind last character
                                                // of text
                       PSZ_W  pTextStart,       // pointer to start of text
                       USHORT * pusColumnPos)   // column position to be returned
{
   PSZ_W  pText;                                // text pointer
   BOOL   fFound = FALSE;                       // line end found?
   USHORT usColumnPos = 0;                      // column position

   pText = pTextEnd;

   while (pText > pTextStart && !fFound)
   {
      if (*pText == LF)
      {
         fFound = TRUE;
      }
      else
      {
         pText --;
         usColumnPos ++;
      } /* endif */
   } /* endwhile */


   if (pText != pTextEnd && fFound)
     {
        *pusColumnPos = usColumnPos - 1;
   } /* endif */
}  /* end of CalcLastColumnPos */

/*
.cp
********************* Calculate Last Segmenting State **************************
*                                                                              *
*  Determine whether segmenting state is on or off at the end of the processed *
*  text. If the segmenting state is on, the value is ok and does not have to   *
*  be changed, but if it is <=  0, it might have been 0 or 1   at the end of   *
* the processed text. E.g. " This is a processed text. It is segmented up to   *
*  last sentence boundary :fn.A boundary ..." In this case the segmenting state*
*  is off at the end of the text block because of "fn.", but at the end of the *
*  processed text after the first sentence it is still on. So in this case the *
*  segmenting state has to be set to ON   .                                    *
*******************************************************************************/

static
VOID  CalcSegState ( PTOKENENTRYSEG pSegTokenList,  // pointer to token list
                     SHORT          *psSegState,    // segmenting state
                     PTAG           pTag,           // pointer to tags
                     PSZ_W            pTextEnd,       // pointer
                     USHORT           uNumTags)

{
   PTOKENENTRYSEG  pSegTokenEntry;

   pSegTokenEntry = pSegTokenList;
   if (*psSegState <= 0)        // only do something is segmenting state is off
   {
      // get token of last segment boundary
      while ((pSegTokenEntry->sTokenid != ENDOFLIST) &&
            (pSegTokenEntry->pDataStringW < pTextEnd) )
      {
         pSegTokenEntry ++;
      } /* endwhile */


      if (pSegTokenEntry->sTokenid == ENDOFLIST)
      {
// fix by XQG, 990602
        // leave current segmenting state as-is if no tag is at current segment boundary
        // *psSegState = 1;
// end-of-fix
      }
      else if (pSegTokenEntry->pDataStringW == pTextEnd)
      {
         // segment boundary is at start of token
         // check previous token for segmenting info
         pSegTokenEntry --;
         if (pSegTokenEntry->sTokenid == TEXT_TOKEN)
         {
            *psSegState = 1;
         }
         else
         {
            // skip tokens until previous tag token
            while ((pSegTokenEntry->sTokenid != DUMMYTAG) &&
                    ((pSegTokenEntry->sTokenid < 0 ) ||
                    (pSegTokenEntry->sTokenid >= (SHORT)uNumTags)))
            {
               pSegTokenEntry --;
            } /* endwhile */

            // check segment info
            if (pTag[pSegTokenEntry->sTokenid].Segbreak == SEG_OFF)
            {
               *psSegState = OFF;
            }
            else
            {
               *psSegState = ON;
            } /* endif */
         } /* endif */
      }
      else // segment boundary is in middle of token, must have been segmented
           // by TOLSTOY
      {
         *psSegState = ON;
      } /* endif */
   } /* endif */

}  /* end of CalcSegState */

/*----------------------------------------------------------------------------*\
|   ForceSegBoundary: insert segment boundaries every MAX_SEGMANT_SIZE chars   |
|                     possibly at linebrak                                     |
|   input           : pointer to segment table                                 |
|                     pointer to current segment                               |
|                     maximum segment size                                     |
|                                                                              |
|   output          : updates the segment boundary table inserting             |
|                     segment item as necessary                                |
|   called by       : EQFTextSegmentation                                      |
|                                                                              |
\*----------------------------------------------------------------------------*/
static BOOL ForceSegboundary(
       SEGBOUNDARYTBL * pSegBoundaryTbl,    // pointer to segment boundary table
       PSEGBOUNDARY   * ppStartSegBoundary, // current entry in segment
                                            // boundary table
       int              iMaxSegSize       ) // maximum segment size     
   {
   BOOL            fOK = TRUE;            // processing state
   USHORT          usSegLength;           // new segment length
   PSZ_W           pText;                 // current text position
   PSZ_W           pEndBoundary;          // text position of next segment boundary
   PSZ_W           pWorkBoundary;         // work pointer
   PSEGBOUNDARY    pCurSegBoundary;       // current entry in segment boundary list
   PSEGBOUNDARY    pNextSegBoundary = NULL;      // next entr in segment boundary list
   XLATESTATE      OrgXLateState;         // XLATE state of original segment

   pText = (*ppStartSegBoundary)->pBoundary;
   pEndBoundary = (*ppStartSegBoundary)->pNext->pBoundary;

   // calculate new segment length
// usSegLength = (USHORT) (MAX_SEGMENT_SIZE-1);
   usSegLength = (USHORT)iMaxSegSize-1;


   pCurSegBoundary = *ppStartSegBoundary;
   OrgXLateState = pCurSegBoundary->XlateState;
   pWorkBoundary = pText;
   pText += usSegLength;

   while (pText < pEndBoundary && fOK)
   {
      // if we are at a LF we have to go one position back to ensure that the segment size
      // does not exceed exactly 2048 bytes (in case of LF pText is incremented...)
      if ( *pText == LF )
      {
        pText--;
      } /* endif */

      while (pText > pWorkBoundary && *pText != LF)
      {
         pText --;
      } /* endwhile */

      // if no LF found, try again looking for blanks
      if (pText == pWorkBoundary )
      {
         pText = pWorkBoundary + usSegLength;
         while (pText > pWorkBoundary && *pText != BLANK)
         {
           pText --;
         } /* endwhile */
      } /* endif */

      // if no LF/Space found, split at segment boundary
      if (pText == pWorkBoundary )
      {
         pText = pWorkBoundary + usSegLength;
      } /* endif */

      if (pText == pWorkBoundary )
      {
         fOK = FALSE;
      }
      else
      {
         if ( *pText == LF) pText ++;  // advance after newline

         // insert a new segment boundary after the newline
         pNextSegBoundary = pCurSegBoundary->pNext;
         pCurSegBoundary->pNext = GetEntry(pSegBoundaryTbl);

         if (pCurSegBoundary->pNext == NULL)
           {
           fOK = FALSE;
           }
         else
           {
           pCurSegBoundary = pCurSegBoundary->pNext;
           pCurSegBoundary->pBoundary = pText;
           pCurSegBoundary->XlateState = OrgXLateState;
           pCurSegBoundary->pNext = pNextSegBoundary;
           } /* endif */

         pWorkBoundary = pText;
         pText += usSegLength;
         } /* endif */
   } /* endwhile */

   *ppStartSegBoundary = pNextSegBoundary;
   return (fOK);
   } /* end of ForceSegBoundary */

/*
.cp
********************** SubstitbyBlank *****************************************/
/*----------------------------------------------------------------------------*\
|   SubstitbyBlank  : substitute CR/LF by blanks to avoid Tolstoy problems     |
|                     do substitution only if not                              |
|                       - empty line                                           |
|   input           : pointer to buffer                                        |
|                                                                              |
|   output          : updated buffer                                           |
|   called by       : EQFTextSegmentation                                      |
|                                                                              |
\*----------------------------------------------------------------------------*/
#ifdef USE_NEW_BUT_BUGGY_SUBSTITBYBLANK

// new TP6.0.1 version of function with segmentation differences to TP6.0
// version

static
VOID SubstitbyBlank(PSZ_W pBufferStart)         // pointer to text buffer
{
   PSZ_W pTextBuffer;                           // current position in buffer
   CHAR_W c;
   USHORT  usI = 0;                           // start index

   pTextBuffer = pBufferStart;
   while ((c = *pTextBuffer) != EOS)
   {
      switch ( c )
      {
         case CR:
            while ( *pTextBuffer == CR && *(pTextBuffer+1) == LF )
            {
               usI ++;
               // used to avoid more than 5 blanks which causes a segment
               // break to be inserted
               *pTextBuffer++ = ',';
               *pTextBuffer++ = ' ';
            } /* endwhile */

            if ( usI > 1 )
            {
               // empty line(s) are available - simulate a break via a forced
               // end of sentence
               pTextBuffer--;          // we've gone one too far
               *(pTextBuffer-1) = '.';
            }
            else
            {
               // cr should be replaced by blank ...
               if ( usI )
               {
                 *(pTextBuffer-1) = ' ';
                 *(pTextBuffer-2) = ' ';
                 pTextBuffer--;          // we've gone one too far
               }
               else
               {
                 *pTextBuffer = ' ';      // standalone CR detected....
               } /* endif */
            } /* endif */
            usI = 0;                   // reset index for new start
            break;
         case LF:
            *pTextBuffer = ' ';
            break;
         case '\x09':
            *pTextBuffer = ' ';         // P016147: repl. TAB by blank to find sent.
         default :
            break;
      } /* endswitch */
      pTextBuffer ++;
   } /* endwhile */

   // change blanks surrounded by blanks to dummy words
   // to avoid POE detect sentence delimiters

   pTextBuffer = pBufferStart;
   while ( (c = *pTextBuffer) != NULC )
   {
     if ( (pTextBuffer > pBufferStart) &&  // not a the beginning of the text and
          ( c == L' ') )                   // and a blank character ???
     {
         if ( (pTextBuffer[1] == L' ') &&
              (pTextBuffer[-1] == L' '))  // surrounded by blanks???
         {
           // find character to be used for dummy word
           PSZ_W   pszNextNonBlankChar = pTextBuffer;
           CHAR_W  chReplaceChar = L'a';

           while ( *pszNextNonBlankChar == L' ') pszNextNonBlankChar++;
           if ( *pszNextNonBlankChar &&
                (*pszNextNonBlankChar != L'.') &&
                (*pszNextNonBlankChar != L'!') &&
                (*pszNextNonBlankChar != L'?') )
           {
             chReplaceChar = *pszNextNonBlankChar;
           }
           else
           {
             chReplaceChar = L'A';
           } /* endif */

           while ( pTextBuffer[1] == L' ' )
           {
             *pTextBuffer++ = chReplaceChar;
             chReplaceChar = L'a';
           } /* endwhile */
           pTextBuffer--;
         } /* endif */
     } /* endif */
     pTextBuffer++;
   } /* endwhile */
} /* end of SubstitbyBlank */
#else

// old TP6.0 version of function

static
VOID SubstitbyBlank(PSZ_W pBufferStart)         // pointer to text buffer
{
   PSZ_W pTextBuffer;                           // current position in buffer
   CHAR_W c;
   USHORT  usI = 0;                           // start index

#ifdef TSEGM_SEGLOGGING
    if ( hSegLog )
    {
      fwprintf( hSegLog, L"before SubstitbyBlank:\n" );
      LogSegData( hSegLog, pBufferStart );
    } /* endif */
#endif

   pTextBuffer = pBufferStart;
   while ((c = *pTextBuffer) != EOS)
   {
      switch ( c )
      {
         case CR:
            while ( *pTextBuffer == CR && *(pTextBuffer+1) == LF )
            {
               usI ++;
               // used to avoid more than 5 blanks which causes a segment
               // break to be inserted
               *pTextBuffer++ = ',';
               *pTextBuffer++ = ' ';

               // ignore any whitespace up to following line break
               {
                 PSZ_W pszTemp = pTextBuffer;
                 while ( (*pszTemp == '\x09') || (*pszTemp == ' ') )
                 {
                   if ( (*pszTemp == '\x09') )
                   {
                     *pszTemp = ' ';   // replace tabs with blanks
                   } /* endif */               
                   pszTemp++;
                 } /* endwhile */                  
                 if ( *pszTemp == CR  )
                 {
                   // continue at end of whitespace
                   pTextBuffer = pszTemp;
                 } /* endif */                    
               }
            } /* endwhile */

            if ( usI > 1 )
            {
               // empty line(s) are available - simulate a break via a forced
               // end of sentence
               pTextBuffer--;          // we've gone one too far
               *(pTextBuffer-1) = '.';
            }
            else
            {
               // cr should be replaced by blank ...
               if ( usI )
               {
                 *(pTextBuffer-1) = ' ';
                 *(pTextBuffer-2) = ' ';
                 pTextBuffer--;          // we've gone one too far
               }
               else
               {
                 *pTextBuffer = ' ';      // standalone CR detected....
               } /* endif */
            } /* endif */
            usI = 0;                   // reset index for new start
            break;
         case LF:
            *pTextBuffer = ' ';
            break;
         case '\x09':
            *pTextBuffer = ' ';         // P016147: repl. TAB by blank to find sent.
         default :
            break;
      } /* endswitch */
      pTextBuffer ++;
   } /* endwhile */

   // change sequences of a lot of blanks in not consecutive characters
   // to avoid tolstoy segmentation

   pTextBuffer = pBufferStart;
   usI = 0;
   while ( (c = *pTextBuffer) != NULC )
   {
      if ( c == BLANK )
      {
         if ( usI > 3 )
         {
            usI = 0;
            *(pTextBuffer-3) = ',';
            *pTextBuffer = ',';
         }
         else
         {
            usI ++;
         } /* endif */
      }
      else
      {
         usI = 0;                      // reset index
      } /* endif */
      pTextBuffer++;
   } /* endwhile */

#ifdef TSEGM_SEGLOGGING
    if ( hSegLog )
    {
      fwprintf( hSegLog, L"after SubstitbyBlank:\n" );
      LogSegData( hSegLog, pBufferStart );
    } /* endif */
#endif

} /* end of SubstitbyBlank */


#endif

/*----------------------------------------------------------------------------*\
| StartTM   : open the input translation Memories; if needed opens also         |
|             the output translation memories                                  |
| input     : pointer to the input parameters structure (TAINPUTPARAM) pTAInput|
|             fields used:                                                     |
|                        stInputMemDb list of input memories names             |
|                       szOutputMemDb output memory name                       |
| output    : in the structure  PTAINSTDATA  (PTAINSTDATA) pInD                |
|             fields used:                                                     |
|                       pstInputMemDb array with handles and names input       |
|                       pstOutputMemDb array with output handles and names     |
|             returns TRUE if operation completed                              |
\*----------------------------------------------------------------------------*/
static BOOL  StartTM ( PTAINPUT         pTAInput,
                       PTAINSTDATA      pInD)

   {
   BOOL    fOK = true;                    // success indicator
   USHORT  usNumFile = 0;          // number of tm's
   PSZ     pszInMem;               // name of input tm
   SHORT   sIndex;                 // index to tm's
   
   PANALYSISMEMLOOKUPDATA pLookupData = new(ANALYSISMEMLOOKUPDATA);
   if ( pLookupData != NULL )
   {
     memset( pLookupData->szSegmentBuffer, 0, sizeof(pLookupData->szSegmentBuffer) );
     memset( pLookupData->szNameBuffer, 0, sizeof(pLookupData->szNameBuffer) );
     pLookupData->pDocMem = NULL;
   } /* end */      

    //--- open folder properties
    PPROPFOLDER  ppropFolder = NULL;    //pointer to folder properties
    ULONG        ulErrorInfo;    // error indicator from PRHA
    HPROP        hpropFolder = 0;    // handle of folder properties
    if( ( hpropFolder = OpenProperties( pTAInput->szFolderProp, NULL, PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
    {
      // display error message if not already displayed
      if ( ulErrorInfo != Err_NoStorage )
      {
        PSZ pszFileName = pTAInput->szFolder;
        UtlErrorHwnd( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1,
                  &pszFileName, EQF_ERROR, pTAInput->hwndErrMsg );
      } /* endif */
      // set fOk to FALSE
      fOK = FALSE;
    }
    else
    {
      //--- get pointer to folder properties
      ppropFolder =(PPROPFOLDER) MakePropPtrFromHnd( hpropFolder );
    }

    // load any global memory option file 
    if ( fOK )
    {
      pInD->pvGMOptList = NULL;
      if ( ppropFolder->szGlobalMemOptFile[0] != EOS )
      {
        // setup fully qualified filter file name
        UtlMakeEQFPath( pInD->szASCIIBuffer, ppropFolder->chDrive, SYSTEM_PATH, NULL );
        strcat( pInD->szASCIIBuffer, BACKSLASH_STR );
        strcat( pInD->szASCIIBuffer, ppropFolder->PropHead.szName );
        strcat( pInD->szASCIIBuffer, BACKSLASH_STR );
        strcat( pInD->szASCIIBuffer, ppropFolder->szGlobalMemOptFile );
        GlobMemLoadOptionFile( pInD->szASCIIBuffer, &(pInD->pvGMOptList) );
      }
      else
      {
        pInD->pvGMOptList = NULL;
      } /* endif */                
   }


   sIndex = 0;

   // allocate table for memory objects and proposal buffers
   if ( fOK )
   {

     for ( int i=0; i < MAX_MATCHES; i++ )
     {
       pLookupData->FoundProposals.push_back( new(OtmProposal) );
     } /* endif */        

     for ( int i=0; i < MAX_MATCHES; i++ )
     {
       pLookupData->BestProposals.push_back( new(OtmProposal) );
     } /* endif */        

     pInD->pvMemoryLookupData = (PVOID )pLookupData;
   } /* endif */      

   // pointer to first element
   pszInMem = (PSZ)((PBYTE) pTAInput + pTAInput->stInputMemDb.ulOffset);
   MemoryFactory *pFactory = MemoryFactory::getInstance();
   while (usNumFile < pTAInput->stInputMemDb.usNumber && fOK)
   {
     int iRC = 0;

     OtmMemory *pMem = pFactory->openMemory( NULL, pszInMem, NONEXCLUSIVE, &iRC );
     if ( pMem != NULL )
     {
       pMem->setGlobalMemoryOptions( pInD->pvGMOptList );
       pLookupData->InputMemoryDBs.push_back( pMem );
     }
     else
     {
       pFactory->showLastError( NULL, pszInMem, NULL, pTAInput->hwndErrMsg );
       fOK = FALSE;
     } /* endif */        

      // next element pointer
      pszInMem += strlen(pszInMem) + 1;
      usNumFile ++;
      sIndex ++;
   } /* endwhile */

   // open output and r/o memory databases only if requested
   if (fOK && (pTAInput->fInsertTMMatches ||
               pTAInput->fInsertToTM ||
               pTAInput->fTMMatch         ||   // TM match counting
               pTAInput->fRedundCount     ||   // redundancy counting
               pTAInput->fMachTrans       ||
               pTAInput->fInsertNewMatch))
   {

      while (usNumFile < pTAInput->stInputMemDb.usNumber && fOK)
      {
        int iRC = 0;

        OtmMemory *pMem = pFactory->openMemory( NULL, pszInMem, NONEXCLUSIVE, &iRC );
        if ( pMem != NULL )
        {
          pMem->setGlobalMemoryOptions( pInD->pvGMOptList );
          pLookupData->InputMemoryDBs.push_back( pMem );
        }
        else
        {
          pFactory->showLastError( NULL, pszInMem, NULL, pTAInput->hwndErrMsg );
          fOK = FALSE;
        } /* endif */        

        // next element pointer
        pszInMem += strlen(pszInMem) + 1;
        usNumFile ++;
        sIndex ++;
      } /* endwhile */
     
     
        /******************************************************************/
        /* open our R/O memories ....                                     */
        /******************************************************************/
        if ( fOK )
        {
          {
             PSZ pszToken;
             //get list of folder R/O memories from folder properies
             strcpy( pInD->szTempDisp, ppropFolder->MemTbl );

             /*********************************************************/
             /* open any pending RO memories                          */
             /*********************************************************/
             //build path to memory properties
             UtlMakeEQFPath((PSZ)pInD->szTextBuffer, NULC, MEM_PATH, NULL );
             strcat( (PSZ)pInD->szTextBuffer, "\\" );

             if ( ppropFolder->aLongMemTbl[0][0] != EOS )
             {
                int i = 0;
                sIndex = 0;
                while ( (i < MAX_NUM_OF_READONLY_MDB) && (ppropFolder->aLongMemTbl[i][0] != EOS) )
                {
                   int iRC = 0;

                   OtmMemory *pMem = pFactory->openMemory( NULL, ppropFolder->aLongMemTbl[i], READONLYACCESS, &iRC );
                   if ( pMem != NULL )
                   {
                     pMem->setGlobalMemoryOptions( pInD->pvGMOptList );
                     pLookupData->ROMemoryDBs.push_back( pMem );
                   }
                   else
                   {
                     pFactory->showLastError( NULL, pszInMem, NULL, pTAInput->hwndErrMsg );
                     fOK = FALSE;
                   } /* endif */        
                   i++;
                } /* endwhile */
             }
             else
             {
               //get first R/O memory from list
               pszToken = strtok( pInD->szTempDisp, X15_STR );
               sIndex = 0;
               while ( (pszToken != NULL) && fOK )
               {
                  int iRC = 0;

                  OtmMemory *pMem = pFactory->openMemory( NULL, pszToken, READONLYACCESS, &iRC );
                  if ( pMem != NULL )
                  {
                    pMem->setGlobalMemoryOptions( pInD->pvGMOptList );
                    pLookupData->ROMemoryDBs.push_back( pMem );
                  }
                  else
                  {
                    pFactory->showLastError( NULL, pszToken, NULL, pTAInput->hwndErrMsg );
                    fOK = FALSE;
                  } /* endif */        
                  pszToken = strtok( NULL, X15_STR );
               } /* endwhile */
             } /* endif */
          } /* endif NO ERROR */
        } /* endif */
   } /* endif */

  // close any open folder properties
  if ( hpropFolder != 0 ) CloseProperties( hpropFolder, 0, &ulErrorInfo);

   return (fOK);
} /*----------- end of StartTM ----------------------*/


/*----------------------------------------------------------------------------*\
| EndTM     : close the active Translation memories; frees allocated storage   |
|                                                                              |
| input     : pointer to the input parameters structure (TAINPUTPARAM) pTAInput|
|             fields used:                                                     |
|                       szOutputMemDb output memory name                       |
|             pointer to the input parameters structure pInD                   |
|             fields used:                                                     |
|                       pstInputMemDb pointer to list of input memories        |
|                       pGetIn        pointer to allocated area                |
|                                                                              |
| output    :                                                                  |
|             returns TRUE if operation completed                              |
\*----------------------------------------------------------------------------*/
static VOID EndTM (PTAINPUT pTAInput, PTAINSTDATA pInD)
{
   SHORT  sIndex = 0; // index to translation memories
   PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;
   MemoryFactory *pFactory = MemoryFactory::getInstance();

   if ( pLookupData != NULL )
   {
     // if valid handle close input memory databases
     for (sIndex  = 0; sIndex < (SHORT)pLookupData->InputMemoryDBs.size(); sIndex++ )
     {
       pFactory->closeMemory( pLookupData->InputMemoryDBs[sIndex] );
     }
     pLookupData->InputMemoryDBs.clear(); 
   } /* endif */      

   //--- if valid handle for output TM  close output memory database
   if ( (pLookupData != NULL) && (pLookupData->pDocMem != NULL) )
   {
      pFactory->closeMemory( pLookupData->pDocMem );
      pLookupData->pDocMem = NULL;
   } /* endif */

   /*******************************************************************/
   /* close any opened RO memory dbs                                  */
   /*******************************************************************/
   if (pLookupData != NULL)
   {
     for (sIndex  = 0; sIndex < (SHORT)pLookupData->ROMemoryDBs.size(); sIndex++ )
     {
       pFactory->closeMemory( pLookupData->ROMemoryDBs[sIndex] );
     }
     pLookupData->ROMemoryDBs.clear(); 
   } /* endif */

   // free lookup data area
   if ( pLookupData != NULL )
   {
     for ( int i=0; i < (int)pLookupData->FoundProposals.size(); i++ )
     {
       delete( pLookupData->FoundProposals[i] );
     } /* endif */        

     for ( int i=0; i < (int)pLookupData->BestProposals.size(); i++ )
     {
       delete( pLookupData->BestProposals[i] );
     } /* endif */        

     delete( pLookupData );
   } /* end */      

   pInD->pvMemoryLookupData = NULL;

   pTAInput->fEndTMDone = TRUE;        // EndTM handling has been done

} /* end of EndTm */


/*----------------------------------------------------------------------------*/
/* Unsegment Utility                                                          */
/*                                                                            */
/*   This function will remove all segmenting tags included by the text       */
/*   segmentation function                                                    */
/*                                                                            */
/* Input:                                                                     */
/*   pSegTarget :    ptr to full qualified segmented target file name         */
/*   pTarget    :    ptr to full qualified target file name                   */
/*   pTATag     :    ptr to Tag structures containing the tags for the        */
/*                   different tag classes                                    */
/*   pfGoOn     :    flag if other files are following                        */
/*                                                                            */
/* Output:                                                                    */
/*   fReturn    :    TRUE  if unsegmented file could be built                 */
/*                   FALSE otherwise                                          */
/*   pfGoOn     :    TRUE  if user wants to go on                             */
/*                   FALSE otherwise                                          */
/*   pfGoOn has only a meaning if fReturn is set to FALSE, i.e. an error      */
/*   occured.                                                                 */
/*                                                                            */
/* Assumptions: All of our TWB tags have same starting character.             */
/*----------------------------------------------------------------------------*/
BOOL EQFUnSeg( PSZ pSegTarget,                       // segm.target file
               PSZ pTarget,                          // target file name
               PTATAG pTATag,                        // pointer to tag str.
               PBOOL  pfGoOn,                        // goon allowed
               PSZ    pFolder,                       // folder name passed
               USHORT usTrackDocNum )                // not 0, TVT tracking docnum
{
  return( EQFUnSeg2( pSegTarget, pTarget, pTATag, pfGoOn, pFolder, usTrackDocNum, NULL ) );
}

BOOL EQFUnSeg2( PSZ pSegTarget,                       // segm.target file
                PSZ pTarget,                          // target file name
                PTATAG pTATag,                        // pointer to tag str.
                PBOOL  pfGoOn,                        // goon allowed
                PSZ    pFolder,                       // folder name passed
                USHORT usTrackDocNum,                 // not 0, TVT tracking docnum
                PSZ    pszConversion )                // ptr to conversion name or NULL
{
  PSZ     apszRevMark[6];

  /********************************************************************/
  /* Setup empty revision mark table                                  */
  /********************************************************************/
  apszRevMark[0] = EMPTY_STRING;
  apszRevMark[1] = EMPTY_STRING;
  apszRevMark[2] = EMPTY_STRING;
  apszRevMark[3] = EMPTY_STRING;
  apszRevMark[4] = EMPTY_STRING;
  apszRevMark[5] = EMPTY_STRING;

  /********************************************************************/
  /* Call unsegment function                                          */
  /********************************************************************/
  return( EQFUnSegRevMark2( pSegTarget, pTarget, pTATag, pfGoOn, pFolder,
                            apszRevMark, usTrackDocNum, pszConversion ) );
}



/*----------------------------------------------------------------------------*/
/* Unsegment Utility with revision mark handling                              */
/*                                                                            */
/*   This function will remove all segmenting tags included by the text       */
/*   segmentation function                                                    */
/*                                                                            */
/* Input:                                                                     */
/*   pSegTarget :    ptr to full qualified segmented target file name         */
/*   pTarget    :    ptr to full qualified target file name                   */
/*   pTATag     :    ptr to Tag structures containing the tags for the        */
/*                   different tag classes                                    */
/*   pfGoOn     :    flag if other files are following                        */
/*   apszRevMark:    array with revision mark strings                         */
/*   usTrackDocNum:  not 0, TVT tracking document num (include tracking info) */
/*                                                                            */
/* Output:                                                                    */
/*   fReturn    :    TRUE  if unsegmented file could be built                 */
/*                   FALSE otherwise                                          */
/*   pfGoOn     :    TRUE  if user wants to go on                             */
/*                   FALSE otherwise                                          */
/*   pfGoOn has only a meaning if fReturn is set to FALSE, i.e. an error      */
/*   occured.                                                                 */
/*                                                                            */
/* Assumptions: All of our TWB tags have same starting character.             */
/*----------------------------------------------------------------------------*/
BOOL EQFUnSegRevMark
(
  PSZ pSegTarget,                       // segm.target file
  PSZ pTarget,                          // target file name
  PTATAG pTATag,                        // pointer to tag str.
  PBOOL  pfGoOn,                        // goon allowed
  PSZ    pFolder,                       // folder name passed
  PSZ    apszRevMark[6],                // array with revision mark pointers
  USHORT usTrackDocNum                  // not 0, TVT tracking docnum
)
{
  return ( EQFUnSegRevMark2( pSegTarget, pTarget, pTATag, pfGoOn, pFolder,                       // folder name passed
                             apszRevMark, usTrackDocNum, NULL ) );
} /* end of function EQFUnSegRevMark */


BOOL EQFUnSegRevMark2
(
  PSZ pSegTarget,                       // segm.target file
  PSZ pTarget,                          // target file name
  PTATAG pTATag,                        // pointer to tag str.
  PBOOL  pfGoOn,                        // goon allowed
  PSZ    pFolder,                       // folder name passed
  PSZ    apszRevMark[6],                // array with revision mark pointers
  USHORT usTrackDocNum,                 // not 0, TVT tracking docnum
  PSZ    pszConversion                  // ptr to conversion name or NULL
)
{
   BOOL    fOK = TRUE;                 // success indicator
   CHAR    szEdit[MAX_FILESPEC];       // buffer for editor name
   CHAR    szFormat[MAX_FNAME];        // buffer for document format
   PSZ     pszTemp = NULL;
   PSZ     pszPath = NULL;
   HMODULE hmodSegmentDll = NULLHANDLE;
   BOOL    fFakedRTF = FALSE;
   PSZ     pszFaked;
   BOOL    fSTargetInUnicode = FALSE;
   CHAR    szTgtLang[MAX_LANG_LENGTH];
   CHAR    szSrcLang[MAX_LANG_LENGTH];
   OBJNAME       szDocObjName;       // buffer for document object name


   PFN_EQFPREUNSEG       pfnEQFPreUnSeg; 
   PFN_EQFPREUNSEG2      pfnEQFPreUnSeg2; 
   PFN_EQFPREUNSEGW      pfnEQFPreUnSegW; 
   PFN_EQFPOSTUNSEG      pfnEQFPostUnSeg; 
   PFN_EQFPOSTUNSEG2     pfnEQFPostUnSeg2; 
   PFN_EQFPOSTUNSEGW     pfnEQFPostUnSegW; 


   USHORT usRc = 0 ;
   PSZ    pFile;
   EQF_BOOL fNoUnSeg = FALSE;
   EQF_BOOL fKill = FALSE;                    // kill flag
   PLOADEDTABLE  pLoadedTable = NULL;         // ptr to in-memory copy of tag table
   pfnEQFPostUnSegW = NULL;
   pfnEQFPostUnSeg2 = NULL;
   pfnEQFPostUnSeg = NULL;
   pfnEQFPreUnSegW = NULL;
   pfnEQFPreUnSeg2 = NULL;
   pfnEQFPreUnSeg  = NULL;


   INFOEVENT2( EQFUNSEGREVMARK_LOC, FUNCENTRY_EVENT, 0, DOC_GROUP, NULL );

   // allocate storage for several buffers
   if (fOK) fOK = UtlAlloc( (PVOID *)&(pszTemp), 0L, (LONG) MAX_PATH144, ERROR_STORAGE);
   if (fOK) fOK = UtlAlloc( (PVOID *)&(pszPath), 0L, (LONG) MAX_EQF_PATH, ERROR_STORAGE);

   if ( fOK )
   {
     UtlMakeEQFPath( pszPath, NULC, PROGRAM_PATH, NULL );
   } /* endif */

   /********************************************************/
   /* Get format and editor of document                    */
   /********************************************************/
   if ( fOK )
   {
     pszFaked = strrchr(pFolder,'#');

     if (pszFaked)
     {
         *pszFaked = EOS;
         fFakedRTF = TRUE;
     } // end if

     UtlMakeEQFPath( szDocObjName, pSegTarget[0], SYSTEM_PATH, pFolder );
     strcat( szDocObjName, BACKSLASH_STR );
     strcat( szDocObjName, UtlGetFnameFromPath( pSegTarget ) );

     usRc = DocQueryInfo2( szDocObjName, // document object name
                          NULL,        // no memory needed
                          szFormat,    // format of document
                          szSrcLang,        // no source language needed
                          szTgtLang,        // no target language needed
                          NULL,        // no long name required
                          NULL,        // no alias required
                          szEdit,      // editor of document
                          TRUE );      // handle errors in function

     if (fFakedRTF)
     {
         strcpy(&szFormat[0],"EQFRTF");
     }// end if


     if ( usRc != NO_ERROR )
     {
       ERREVENT2( EQFUNSEGREVMARK_LOC, INTFUNCFAILED_EVENT, usRc, DOC_GROUP, NULL );
     } /* endif */
     fOK = (usRc == NO_ERROR );
     if  ( fOK )
     {
       fSTargetInUnicode = TAGetUnicodeFlagFromDocProp(szDocObjName );
     }
   } /* endif */

   /********************************************************/
   /* Load tagtable for document format                    */
   /********************************************************/
   if ( fOK )
   {
     INFOEVENT2( EQFUNSEGREVMARK_LOC, STATE_EVENT, 1, DOC_GROUP, NULL );
     fOK = (TALoadTagTable( szFormat, &pLoadedTable, FALSE, TRUE ) == NO_ERROR );
   } /* endif */

   /*******************************************************************/
   /* Load any user exit                                              */
   /*******************************************************************/

   if ( fOK ) {
     CHAR szExitName[MAX_LONGFILESPEC];
     if ( MUGetUserExitFileName(  szFormat, NULL, szExitName, sizeof(szExitName) ) )
     {
       INFOEVENT2( EQFUNSEGREVMARK_LOC, STATE_EVENT, 2, DOC_GROUP, NULL );
        usRc = DosLoadModule( NULL, 0 , szExitName, &hmodSegmentDll );
    
        if ( usRc == NO_ERROR )
        {
          usRc = DosGetProcAddr( hmodSegmentDll, EQFPREUNSEGW_EXIT,
                                 (PFN*) (&pfnEQFPreUnSegW));
          if ( usRc != NO_ERROR )
          {
            pfnEQFPreUnSegW = NULL;
            usRc = DosGetProcAddr( hmodSegmentDll, EQFPREUNSEG2_EXIT,
                                 (PFN*) (&pfnEQFPreUnSeg2));
            if ( usRc != NO_ERROR )
            {
              pfnEQFPreUnSeg2 = NULL;
              usRc = DosGetProcAddr( hmodSegmentDll, EQFPREUNSEG_EXIT,
                                     (PFN*) (&pfnEQFPreUnSeg));
            } /* endif */
          } /* endif */
        } /* endif */
    
        if ( usRc == NO_ERROR )
        {
          usRc = DosGetProcAddr( hmodSegmentDll, EQFPOSTUNSEGW_EXIT,
                                 (PFN*)(&pfnEQFPostUnSegW));
          if ( usRc != NO_ERROR )
          {
            pfnEQFPostUnSegW = NULL;
    
            usRc = DosGetProcAddr( hmodSegmentDll, EQFPOSTUNSEG2_EXIT,
                                   (PFN*)(&pfnEQFPostUnSeg2));
            if ( usRc != NO_ERROR )
            {
              pfnEQFPostUnSeg2 = NULL;
              usRc = DosGetProcAddr( hmodSegmentDll, EQFPOSTUNSEG_EXIT,
                                     (PFN*) (&pfnEQFPostUnSeg));
            } /* endif */
          } /* endif */
    
        } /* endif */
      }
  
      if ( usRc != NO_ERROR )
      {
        PSZ        pszParm;
  
        fOK = FALSE;
        pszParm = szExitName;
        UtlError( ERROR_CANNOT_LOAD_USER_EXIT, MB_CANCEL, 1, &pszParm,
                  EQF_ERROR);
      } /* endif */
   } /* endif */

   if (fOK && *(pLoadedTable->pTagTable->szSegmentExit) )
   {
     if ( pfnEQFPreUnSegW == NULL )
     {
       CHAR chOutFile[MAX_EQF_PATH+1];
       CHAR chUnSegFile[MAX_EQF_PATH+1];
      /******************************************************************/
      /* create temp file names -- we have to use two different output  */
      /******************************************************************/
      strcpy( chOutFile, pSegTarget);
      strcat( chOutFile, "A");
      strcpy( chUnSegFile, pSegTarget );
      strcat( chUnSegFile, "B" );

      fOK = (TASegFileConvertUnicode2ASCII( pSegTarget, chOutFile, szTgtLang )  ) == 0;
      if (fOK)
      {
        if ( pfnEQFPreUnSeg2 == NULL )
        {
          fOK = pfnEQFPreUnSeg( szFormat,
                              szEdit, pszPath,
                              &chOutFile[0], pszTemp,
                              pTATag,  &fNoUnSeg);
        }
        else
        {
          fOK = pfnEQFPreUnSeg2( szFormat,
                               szEdit, pszPath,
                               chOutFile, pszTemp,
                               pTATag, &fNoUnSeg,
                               &fKill );
        } /* endif */
      }
      if ( fOK )
      {
        if ( *pszTemp )
        {
          fOK = (TASegFileConvertASCII2Unicode( pszTemp, chUnSegFile, szTgtLang )  ) == 0;
          UtlDelete( pszTemp, 0L, NOMSG );
          strcpy( pszTemp, chUnSegFile );
        }
      }

      UtlDelete( chOutFile, 0L, NOMSG );
     }
     else
     {
       // GQ: TATAG_W array must be at least same size as TaDummyTag array (the source for pTaTag)!!!
       static TATAG_W TATagW[MAX_SEG_CLASSES];
       PTATAG_W pTATagW = TATagW;

       TATATag2Unicode( pTATag, pTATagW );

       // check if file is already in UTF16 format, if not we have to convert it before calling the user exit
       if ( !TAIsUTF16SegFile( pSegTarget ) )
       {
         CHAR szUTF16File[MAX_EQF_PATH+1];

         // create temp file name for UTF16 version of document
         strcpy( szUTF16File, pSegTarget);
         strcat( szUTF16File, "A");

         // convert to UTF16
         fOK = (TASegFileConvertASCII2Unicode( pSegTarget, szUTF16File, szTgtLang )  ) == 0;

         // from now on use converted document if successful converted
         if ( fOK )
         {
           UtlDelete( pSegTarget, 0L, NOMSG );
           UtlMove( szUTF16File, pSegTarget, 0L, FALSE );
         } /* endif */
       } /* endif */

       fOK = pfnEQFPreUnSegW( szFormat, szEdit, pszPath, pSegTarget, pszTemp, pTATagW, &fNoUnSeg, &fKill );

     } /* endif */
   } /* endif */

   if (fOK && !fNoUnSeg && !fKill )
   {  // call normal unsegmentation utility
      INFOEVENT2( EQFUNSEGREVMARK_LOC, STATE_EVENT, 3, DOC_GROUP, NULL );
      if (*pszTemp )
      {
         pFile = pszTemp;
      }
      else
      {
         pFile = pSegTarget;
      } /* endif */

      // use own unsegment utility for unicode documents
      {
        BOOL fPostUnsegW = (pfnEQFPostUnSegW != NULL);

        fOK = TAUnSegmentW( pLoadedTable,
                            pFile,
                            pTarget,                   // target file name
                            pTATag,                    // pointer to tag str.
                            pfGoOn,                    // goon allowed
                            apszRevMark,
                            usTrackDocNum,
                            &fKill,
                            pszConversion,
                            &szTgtLang[0],
                            fPostUnsegW );

        // restore original CRLF/EOF sequence as used by source file if markup is not
        // contained in our szRestoreOrgEOFAfterPostUnsegMarkups list
        {
          BOOL fFound = FALSE;
          int i = 0;

          while( !fFound && (szRestoreOrgEOFAfterPostUnsegMarkups[i][0] != EOS) )
          {
            fFound = strcmp( szRestoreOrgEOFAfterPostUnsegMarkups[i], szFormat ) == 0;
            i++;
          } /* endwhile */

          if ( !fFound )
          {
            // setup name of source of document
            CHAR szSourceFile[MAX_EQF_PATH];   // buffer for document source file
            CHAR szFolder[MAX_FILESPEC];       // buffer for folder name
            CHAR szDoc[MAX_FILESPEC];          // buffer for document name
            PSZ  pszTemp;                      // buffer for file name processing

            strcpy( szSourceFile, szDocObjName );
            pszTemp = UtlSplitFnameFromPath( szSourceFile ); // remove document name
            strcpy( szDoc, pszTemp );
            pszTemp = UtlSplitFnameFromPath( szSourceFile ); // remove folder name
            strcpy( szFolder, pszTemp );

            UtlMakeEQFPath( szSourceFile, szDocObjName[0], DIRSOURCEDOC_PATH, szFolder );
            strcat( szSourceFile, BACKSLASH_STR );
            strcat( szSourceFile, szDoc );

            // restore original EOF chars
            TARestoreOrgEOF( szSourceFile, pTarget, szSrcLang, szTgtLang );
          } /* endif */
        }

#ifdef COPYTARGETFILE
        {
          // make copy of original TARGET file
          CHAR szTemp[MAX_EQF_PATH];
          strcpy( szTemp, pTarget );
          strcat( szTemp,".after-unseg.txt" );
          UtlCopy( pTarget, szTemp, 1, 0L, FALSE);
        }
#endif
      }
   }
   else if (fOK)
   {
      INFOEVENT2( EQFUNSEGREVMARK_LOC, STATE_EVENT, 5, DOC_GROUP, NULL );
      usRc = UtlCopy(pszTemp,
                    pTarget,
                    DCPY_EXISTING,            // copy even if exist
                    0L,                       // reserved value
                    FALSE);

       if (usRc)
       {
          fOK = FALSE;
          usRc = UtlError( ERROR_TA_FILEWRITE,
                           MB_CANCEL,
                           1, &pTarget,
                           EQF_ERROR);
       } /* endif */
   } /* endif */

   if ( fOK && (pLoadedTable->pTagTable->szSegmentExit[0] != EOS) && !fKill )
   {
     INFOEVENT2( EQFUNSEGREVMARK_LOC, STATE_EVENT, 6, DOC_GROUP, NULL );
     if ( pfnEQFPostUnSegW != NULL )
     {
        fOK = pfnEQFPostUnSegW( szFormat, szEdit, pszPath, pTarget, pTATag, &fKill);
     }
     else if ( pfnEQFPostUnSeg2 == NULL )
     {
        fOK = pfnEQFPostUnSeg( szFormat, szEdit, pszPath, pTarget, pTATag);
     }
     else
     {
        fOK = pfnEQFPostUnSeg2( szFormat, szEdit, pszPath, pTarget, pTATag, &fKill);
     } /* endif */
   } /* endif */

  // restore original file end sequence if markup is in list szRestoreOrgEOFAfterPostUnsegMarkups
  if ( fOK )
  {
    BOOL fFound = FALSE;
    int i = 0;

    while( !fFound && (szRestoreOrgEOFAfterPostUnsegMarkups[i][0] != EOS) )
    {
      fFound = strcmp( szRestoreOrgEOFAfterPostUnsegMarkups[i], szFormat ) == 0;
      i++;
    } /* endwhile */

    if ( fFound )
    {
      // setup name of source of document
      CHAR szSourceFile[MAX_EQF_PATH];   // buffer for document source file
      CHAR szFolder[MAX_FILESPEC];       // buffer for folder name
      CHAR szDoc[MAX_FILESPEC];          // buffer for document name
      PSZ  pszTemp;                      // buffer for file name processing

      strcpy( szSourceFile, szDocObjName );
      pszTemp = UtlSplitFnameFromPath( szSourceFile ); // remove document name
      strcpy( szDoc, pszTemp );
      pszTemp = UtlSplitFnameFromPath( szSourceFile ); // remove folder name
      strcpy( szFolder, pszTemp );

      UtlMakeEQFPath( szSourceFile, szDocObjName[0], DIRSOURCEDOC_PATH, szFolder );
      strcat( szSourceFile, BACKSLASH_STR );
      strcat( szSourceFile, szDoc );

      // restore original EOF chars
      TARestoreOrgEOF( szSourceFile, pTarget, szSrcLang, szTgtLang );
    } /* endif */
  }

   // if source file different from temp file delete temp file,
   // otherwise user might get problems ....
   if ( pszTemp && strcmpi( pszTemp, pSegTarget ))
   {
      UtlDelete(pszTemp, 0L, FALSE); //delete temp file
   } /* endif */

   /*******************************************************************/
   /* Cleanup                                                         */
   /*******************************************************************/
   if ( hmodSegmentDll ) DosFreeModule( hmodSegmentDll );
   if ( pLoadedTable )   TAFreeTagTable( pLoadedTable );
   if ( pszPath )        UtlAlloc( (PVOID *)&(pszPath), 0L, 0L, NOMSG);
   if ( pszTemp )        UtlAlloc( (PVOID *)&(pszTemp), 0L, 0L, NOMSG);

   INFOEVENT2( EQFUNSEGREVMARK_LOC, FUNCEXIT_EVENT, (SHORT)fOK, DOC_GROUP, NULL );

   return (fOK);
}


/**********************************************************************/
/* Load user exit DLL and addresses of segmentation entry points      */
/**********************************************************************/
USHORT TALoadUserExit
(
  PSZ              pszExit,            // name of user exit DLL
  PTAINPUT         pTAInput            // input structure for text analysis
)
{
  USHORT           usRc;                    // return from dosloadmodule
  CHAR             szReplace[MAX_FILESPEC]; // replace string for UtlError
  PSZ              pszReplace;              // pointer to replace string

  PTAINSTDATA   pInD;                 // pointer to instance data for TA

  pInD = pTAInput->pInD;                   // pointer to local variables

  /********************************************************************/
  /* init segment exit procedures...                                  */
  /********************************************************************/
  pInD->pfnEQFPreSeg    = NULL;
  pInD->pfnEQFPreSeg2   = NULL;
  pInD->pfnEQFPreSegEx  = NULL;
  pInD->pfnEQFPostSeg   = NULL;
  pInD->pfnEQFPostSeg2  = NULL;
  pInD->pfnEQFPostSegEx = NULL;
  pInD->pfnEQFPreUnSeg  = NULL;
  pInD->pfnEQFPreUnSeg2 = NULL;
  pInD->pfnEQFPostTM    = NULL;
  pInD->pfnEQFPostTM2   = NULL;
  pInD->pfnEQFCheckSeg  = NULL;       // reset handle

  pInD->pfnEQFPostSegW  = NULL;
  pInD->pfnEQFPostSegWEx = NULL;
  pInD->pfnEQFPostTmW   = NULL;
  pInD->pfnEQFPreUnSegW = NULL;
  pInD->pfnEQFCheckSegW = NULL;
  pInD->pfnEQFCheckSegExW = NULL;
  pInD->pfnUpdateContext = NULL;
  pInD->pfnGetSegContext = NULL;
  pInD->pfnEQFCheckSegType  = NULL;       

  /********************************************************************/
  /* Load user exit DLL                                               */
  /********************************************************************/
  usRc = DosLoadModule( NULL, 0, pszExit, &(pInD->hmodSegmentDll) );

  /********************************************************************/
  /* Load addresses of segmentation functions                         */
  /********************************************************************/
  if ( usRc == NO_ERROR )
  {
    usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPRESEGEX_EXIT, (PFN*)(&(pInD->pfnEQFPreSegEx)));
    if ( usRc != NO_ERROR )
    {
      pInD->pfnEQFPreSegEx = NULL;
      usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPRESEG2_EXIT, (PFN*)(&(pInD->pfnEQFPreSeg2)));
      if ( usRc != NO_ERROR )
      {
        pInD->pfnEQFPreSeg2 = NULL;
        usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPRESEG_EXIT, (PFN*)(&(pInD->pfnEQFPreSeg)));
        if ( usRc != NO_ERROR ) {
           pInD->pfnEQFPreSeg = NULL;
        }
      } /* endif */
    } /* endif */


    if ( usRc == NO_ERROR )
    {
      usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPOSTSEGWEX_EXIT, (PFN*) (&(pInD->pfnEQFPostSegWEx)));
      if ( usRc != NO_ERROR )
      {
        pInD->pfnEQFPostSegWEx = NULL;
        usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPOSTSEGW_EXIT, (PFN*) (&(pInD->pfnEQFPostSegW)));
        if ( usRc != NO_ERROR )
        {
          pInD->pfnEQFPostSegW = NULL;
          usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPOSTSEG2_EXIT, (PFN*) (&(pInD->pfnEQFPostSeg2)));
          if ( usRc != NO_ERROR )
          {
            pInD->pfnEQFPostSeg2 = NULL;
            usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPOSTSEG_EXIT, (PFN*) (&(pInD->pfnEQFPostSeg)));
            if ( usRc != NO_ERROR ) {
               pInD->pfnEQFPostSeg = NULL;
            }
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    if ( usRc == NO_ERROR )
    {
      usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPREUNSEGW_EXIT, (PFN*)(&(pInD->pfnEQFPreUnSegW)));
      if ( usRc != NO_ERROR )
      {
        pInD->pfnEQFPreUnSegW = NULL;
        usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPREUNSEG2_EXIT, (PFN*) (&(pInD->pfnEQFPreUnSeg2)));
        if ( usRc != NO_ERROR )
        {
          pInD->pfnEQFPreUnSeg2 = NULL;
          usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPREUNSEG_EXIT, (PFN*) (&(pInD->pfnEQFPreUnSeg)));
          if ( usRc != NO_ERROR ) {
             pInD->pfnEQFPreUnSeg = NULL;
          }
        } /* endif */
      } /* endif */
    } /* endif */

    if ( usRc == NO_ERROR )
    {
      usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPOSTUNSEGW_EXIT, (PFN*)(&(pInD->pfnEQFPostUnSegW)));
      if ( usRc != NO_ERROR )
      {
        usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPOSTUNSEG2_EXIT, (PFN*)(&(pInD->pfnEQFPostUnSeg2)));
      } /* endif */
      if ( usRc != NO_ERROR )
      {
        pInD->pfnEQFPostUnSeg2 = NULL;
        usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFPOSTUNSEG_EXIT, (PFN*) (&(pInD->pfnEQFPostUnSeg)));
        if ( usRc != NO_ERROR ) {
           pInD->pfnEQFPostUnSeg = NULL;
        }
      } /* endif */
    } /* endif */


    if (usRc == NO_ERROR)
    {
      usRc = DosGetProcAddr( pInD->hmodSegmentDll,
                             EQFPOSTTMW_EXIT,
                             (PFN*)(&(pInD->pfnEQFPostTmW)));
      if ( usRc != NO_ERROR )
      {
        // try to load post tm exit
        // if it is not available ignore error to be upward
        // compatible
        pInD->pfnEQFPostTmW = NULL;
        usRc = DosGetProcAddr( pInD->hmodSegmentDll,
                                       EQFPOSTTM2_EXIT,
                                       (PFN*) (&(pInD->pfnEQFPostTM2)));
        if ( usRc != NO_ERROR )
        {
        // try to load post tm exit
        // if it is not available ignore error to be upward
        // compatible
          pInD->pfnEQFPostTM2 = NULL;
          usRc = DosGetProcAddr( pInD->hmodSegmentDll,
                                       EQFPOSTTM_EXIT,
                                       (PFN*) (&(pInD->pfnEQFPostTM)));


          if ( usRc == ERROR_PROC_NOT_FOUND )
          {
             usRc = NO_ERROR;
             pInD->pfnEQFPostTM = NULL;       // reset handle
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* check if CHECKSEG exit is available                            */
    /******************************************************************/
    if (usRc == NO_ERROR)
    {
       // try to load CHECKSEG Exits
       // if it is not available ignore error


       // if it is not available ignore error
       usRc = DosGetProcAddr( pInD->hmodSegmentDll, EQFCHECKSEGEXW_EXIT, (PFN*) (&(pInD->pfnEQFCheckSegExW)));
       if ( usRc != NO_ERROR )
       {
         pInD->pfnEQFCheckSegExW = NULL;       // reset handle
         
       usRc = DosGetProcAddr( pInD->hmodSegmentDll,
                              EQFCHECKSEGW_EXIT,
                              (PFN*) (&(pInD->pfnEQFCheckSegW)));

       if ( usRc != NO_ERROR )
       {
         pInD->pfnEQFCheckSegW = NULL;       // reset handle
         usRc = DosGetProcAddr( pInD->hmodSegmentDll,
                                EQFCHECKSEG_EXIT,
                                (PFN*) (&(pInD->pfnEQFCheckSeg)));
         if ( usRc == ERROR_PROC_NOT_FOUND )
         {
           usRc = NO_ERROR;
           pInD->pfnEQFCheckSeg = NULL;
         }
       } /* endif */
    } /* endif */

    /******************************************************************/
    /* check if CHECKSEGTYPE exit is available                        */
    /******************************************************************/
    if (usRc == NO_ERROR)
    {
       pInD->pfnEQFCheckSegType = NULL;       
       // try to load CHECKSEGTYPE Exit
       DosGetProcAddr( pInD->hmodSegmentDll, EQFCHECKSEGTYPE_EXIT, (PFN*) (&(pInD->pfnEQFCheckSegType)));
    } /* endif */

    } /* endif */


    // load any segment context exit
    if (usRc == NO_ERROR)
    {
       usRc = DosGetProcAddr( pInD->hmodSegmentDll,
                              EQFUPDATECONTEXT_EXIT,
                              (PFN*) (&(pInD->pfnUpdateContext)));

       if ( usRc == ERROR_PROC_NOT_FOUND )
       {
         usRc = NO_ERROR;
         pInD->pfnUpdateContext = NULL;
       }

       usRc = DosGetProcAddr( pInD->hmodSegmentDll,
                              EQFGETSEGCONTEXT_EXIT,
                              (PFN*) (&(pInD->pfnGetSegContext)));

       if ( usRc == ERROR_PROC_NOT_FOUND )
       {
         usRc = NO_ERROR;
         pInD->pfnGetSegContext = NULL;
       }

    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Handle errors                                                    */
  /********************************************************************/
  if ( usRc != NO_ERROR )
  {
    sprintf( szReplace, "%s%s",pInD->pLoadedTable->pTagTable->szSegmentExit, EXT_OF_DLL );
    pszReplace = szReplace;
    UtlErrorHwnd( ERROR_CANNOT_LOAD_USER_EXIT, MB_CANCEL, 1, &pszReplace, EQF_ERROR, pTAInput->hwndErrMsg );
  } /* endif */

  return( usRc );
} /* end of function TALoadUserExit */

/**********************************************************************/
/* Free user exit DLL                                                 */
/**********************************************************************/
USHORT TAFreeUserExit
(
  PTAINSTDATA      pInD                // ptr to analysis instance data area
)
{
  USHORT           usRC = NO_ERROR;    // function return code

  if ( pInD->hmodSegmentDll != NULLHANDLE )
  {
    usRC = DosFreeModule( pInD->hmodSegmentDll );
    pInD->hmodSegmentDll = NULLHANDLE;
    /********************************************************************/
    /* init segment exit procedures...                                  */
    /********************************************************************/
    pInD->pfnEQFPreSeg    = NULL;
    pInD->pfnEQFPreSeg2   = NULL;
    pInD->pfnEQFPreSegEx  = NULL;
    pInD->pfnEQFPostSeg   = NULL;
    pInD->pfnEQFPostSeg2  = NULL;
    pInD->pfnEQFPostSegEx = NULL;
    pInD->pfnEQFPreUnSeg  = NULL;
    pInD->pfnEQFPreUnSeg2 = NULL;
    pInD->pfnEQFPostTM    = NULL;
    pInD->pfnEQFPostTM2   = NULL;
    pInD->pfnEQFCheckSeg  = NULL;
    pInD->pfnEQFCheckSegType  = NULL;
    pInD->pfnEQFPostSegW  = NULL;
    pInD->pfnEQFPostSegWEx = NULL;
    pInD->pfnEQFPostTmW   = NULL;
    pInD->pfnEQFPreUnSegW = NULL;
    pInD->pfnEQFCheckSegW = NULL;
    pInD->pfnEQFCheckSegExW = NULL;

  } /* endif */

  return( usRC );
} /* end of function TAFreeUserExit */


BOOL TAAnalyzeFile
(
  PSZ        pszDocument,              // document object name
  HWND       hwndCaller,               // caller's window handle
  USHORT     usFlags,                  // flags for analyze process
  PSZ        pszEditor,                // name of editor or NULL
  PEQF_BOOL  *ppfKill                  // pointer to kill flag
)
{
  BOOL       fOK = TRUE;
  ULONG      ulTASize;                 // size of the TA Input area
  PSZ        pDataStart;               // pointer to var data start
  PSZ        pTempData;                // pointer to temporary buffer
  PPROPFOLDER  ppropFolder;            // pointer to folder properties
  PPROPSYSTEM  pSysProp;               // system properties
  CHAR         szFolder[MAX_EQF_PATH]; // buffer for folder name
  CHAR         szSourceLang[MAX_LANG_LENGTH]; // buffer for source language
  PTAINPUT     pTAInput;               // analysis input area
  EQFINFO      ErrorInfo;
  HPROP        hProp;

  /********************************************************************/
  /*  allocate TAInput area                                           */
  /********************************************************************/
  ulTASize = (ULONG) (sizeof( TAINPUT ) + 2 * MAX_EQF_PATH );
  fOK = UtlAlloc( (PVOID *) &pTAInput, 0L, ulTASize, ERROR_STORAGE );

  /********************************************************************/
  /* allocate stuff for long file name and alias support              */
  /********************************************************************/
  if ( fOK )
  {
    if ( fOK )
    {
      // allocate pointer array for long names
      fOK = UtlAlloc( (PVOID *)&(pTAInput->apszLongNames), 0L,
                      (LONG)max( sizeof(PSZ), MIN_ALLOC ),
                      ERROR_STORAGE );
    } /* endif */

    if ( fOK )
    {
      // allocate pointer array for alias names
      fOK = UtlAlloc( (PVOID *)&(pTAInput->apszAlias), 0L,
                      (LONG)max( sizeof(PSZ), MIN_ALLOC ),
                      ERROR_STORAGE );
    } /* endif */

    // create string pool
    if ( fOK )
    {
      pTAInput->pPool = PoolCreate( MAX_LONGFILESPEC + 10 );
      fOK = (pTAInput->pPool != NULL);
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* fill data in                                                     */
  /********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* copy source file and address of kill flag                      */
    /******************************************************************/
    *ppfKill = &pTAInput->fKill;

    pTAInput->stSourcefiles.usNumber = 1;        // we are dealing with 1 file

    pDataStart = (PSZ)(pTAInput + 1 );
    //--- copy selected file to be analyzed to analysis interface
    strcpy( pDataStart, UtlGetFnameFromPath( pszDocument ) );
    pTAInput->stSourcefiles.ulLength = strlen( pDataStart);
    pTAInput->stSourcefiles.ulOffset = pDataStart - (PSZ) pTAInput;
    pDataStart += pTAInput->stSourcefiles.ulLength + 1;
    if ( pszEditor != NULL )
    {
      strcpy( pTAInput->szEditName, pszEditor );
    }
    else
    {
      pTAInput->szEditName[0] = EOS;
    } /* endif */

    /******************************************************************/
    /* Get information from document properties                       */
    /******************************************************************/
    DocQueryInfo2( pszDocument,         // document object name
                   NULL,                // no memory needed
                   pTAInput->szTagTableName,  // format of document
                   szSourceLang,        // document source language
                   NULL,                // no target language needed
                   pTAInput->szLongBuffer, // document long name
                   pTAInput->szAliasBuffer, // alias name
                   NULL,                // no editor needed
                   TRUE );              // handle errors in function
    pTAInput->apszLongNames[0] =
       PoolAddString( pTAInput->pPool, pTAInput->szLongBuffer );
    if ( pTAInput->szAliasBuffer[0] != EOS )
    {
      pTAInput->apszAlias[0] =
         PoolAddString( pTAInput->pPool, pTAInput->szAliasBuffer );
    } /* endif */

    /******************************************************************/
    /* Copy document source language to analysis interface            */
    /******************************************************************/
    pTAInput->stMorphDict.usNumber = 1;          // we are dealing with 1 file
    strcpy( pDataStart, szSourceLang );
    pTAInput->stMorphDict.ulLength = strlen( szSourceLang );
    pTAInput->stMorphDict.ulOffset = pDataStart - (PSZ) pTAInput;
    pTAInput->fNoComplete = TRUE;     // display no completion message
    pTAInput->fNoRemoveObject = TRUE;    // do not remove document object

    /******************************************************************/
    /* fill in name of tagtable and pathes                            */
    /******************************************************************/
    if ( fOK  )
    {
      memcpy( pTAInput->TATag, TADummyTag, sizeof(TATAG) * MAX_SEG_CLASSES);
      TATATag2Unicode( pTAInput->TATag, pTAInput->TATagW );
      strcpy( szFolder, pszDocument );
      UtlSplitFnameFromPath( szFolder );

      pTempData = UtlGetFnameFromPath( szFolder );
      UtlMakeEQFPath( pTAInput->szSOURCE_Path, szFolder[0],
                      DIRSOURCEDOC_PATH, pTempData );
      UtlMakeEQFPath( pTAInput->szSEGSOURCE_Path, szFolder[0],
                      DIRSEGSOURCEDOC_PATH, pTempData );
      UtlMakeEQFPath( pTAInput->szSEGTARGET_Path, szFolder[0],
                      DIRSEGTARGETDOC_PATH, pTempData );

      /****************************************************************/
      /* get match level from folder properties                       */
      /****************************************************************/
      hProp = OpenProperties( szFolder, NULL, PROP_ACCESS_READ, &ErrorInfo );
      if ( hProp != NULL )
      {
        ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( hProp );
        pTAInput->sMatchLevel = ppropFolder->usMatchLevel;

        if ( usFlags & TA_USEFOLDERSETTINGS )                        // use options from folder properties 
        {
          // copy folder property settings
          pTAInput->fUseLatestTMMatch = ppropFolder->fUsedLatestExactMatch;
          pTAInput->fExactContextTMMatch = ppropFolder->fExactContextTMMatch;
          pTAInput->fAutoJoin = ppropFolder->fAnalAutoJoin;
          pTAInput->fIgnoreIdentical = ppropFolder->fAnalIgnoreIdentical;
          pTAInput->fIgnoreComment = ppropFolder->fAnalIgnoreComment;
          pTAInput->fIgnorePath = ppropFolder->fAnalIgnorePath;
          pTAInput->fLeadingWS = ppropFolder->fAnalLeadingWS;
          pTAInput->fTrailingWS = ppropFolder->fAnalTrailingWS;
          pTAInput->fRespectCRLF = ppropFolder->fAnalRespectCRLF;
          pTAInput->fNoAddBlank = ppropFolder->fAnalNoAddBlank;
          pTAInput->fProtXmp = ppropFolder->fSavedDlgIanaProtXmp;
          pTAInput->fProtMsgNum = ppropFolder->fSavedDlgIanaProtMsgNum;
          pTAInput->fProtMeta = ppropFolder->fSavedDlgIanaProtMeta;
          pTAInput->fProtScreen = ppropFolder->fSavedDlgIanaProtScreen;
          pTAInput->fProtCodeBlock = ppropFolder->fSavedDlgIanaProtCodeBlock;
          pTAInput->fStopAtFirstExact = ppropFolder->fStopAtFirstExact;
        } /* endif */

        CloseProperties( hProp, 0, &ErrorInfo);
      } /* endif */

      // setup folder object name
      strcpy( pTAInput->szFolder, szFolder );
      strcpy( pTAInput->szFolderProp, szFolder );
      pSysProp = (PPROPSYSTEM) MakePropPtrFromHnd( EqfQuerySystemPropHnd());
      pTAInput->szFolderProp[0] = pSysProp->szPrimaryDrive[0];

      pTAInput->fInsertTMMatches = (usFlags & TA_INSERTTMMATCH) ;  // insert TM matches into document
    } /* endif */
  } /* endif */

  if ( fOK  )
  {
   /******************************************************************/
   /* call text analysis process ...                                 */
   /******************************************************************/
   pTAInput->hwndProcWin = hwndCaller;
   pTAInput->hwndErrMsg  = HWND_DESKTOP;

   EQFTAMnl( hwndCaller, &pTAInput );

   fOK = !pTAInput->fKill;    // reset fOK if someone stopped Analysis
   if (pTAInput->pWSList)
     UtlAlloc( (PVOID *) &(pTAInput->pWSList), 0L, 0L, NOMSG );
   UtlAlloc( (PVOID *) &pTAInput, 0L, 0L, NOMSG );  // free the memory
  } /* endif */

  return fOK;
} /* end of function TAAnalyzeFile */


//----------------------------------------------------------------------------
// TaFuncAnalysisProcess: Text Analysis Of The Function Call Interface
//----------------------------------------------------------------------------
USHORT TAFuncAnalysisProcess
(
  PFCTDATA    pData                    // ptr to function interface data area
)
{
   PTAINPUT   pTAInput;                // input structure for text analysis
   PTAINSTDATA pInD;                   // pointer to instance data for TA
   USHORT     usReturn;                // return values from UtlError
   BOOL       fOK = TRUE;              // Proessing status flag
   TATASK     usPhase;                 // activity sequencer
   USHORT     usRC = NO_ERROR;         // function return code
   USHORT        usFilePhase;          // current file processing phase
   USHORT        usSegPhase;           // current segment processing phase
   USHORT        usRedCountPhase;      // current redundancy counting phase

   pTAInput = (PTAINPUT)pData->pvAnaInputData;
   pInD     = pTAInput->pInD;

   usPhase      = (TATASK)pData->usAnalysisPhase;
   usFilePhase  = pData->usAnalysisFilePhase;
   usSegPhase   = pData->usAnalysisSegPhase;
   usRedCountPhase  = pData->usAnalysisRedCountPhase;

   switch ( usPhase )
   {
     case TAFUNC_INIT:
       // reset last error message ID
       UtlSetUShort( QS_LASTERRORMSGID, 0  );


       fOK = UtlAllocHwnd( (PVOID *)&(pTAInput->pInD), 0L,
                           (LONG) (sizeof (TAINSTDATA)),
                           ERROR_STORAGE, pTAInput->hwndErrMsg );

       if ( fOK)
       {
         pData->fComplete = FALSE;
         usPhase = EQFTA_INIT;
         pTAInput->fKill = FALSE;         // kill request
         pInD = pTAInput->pInD;           // pointer to data
         pInD->fUseLatestTMMatch = pTAInput->fUseLatestTMMatch;
         pInD->fExactContextTMMatch = pTAInput->fExactContextTMMatch;
         pInD->fProtXmp = pTAInput->fProtXmp;
         pInD->fProtMsgNum = pTAInput->fProtMsgNum;           /* 4-13-14 */
         pInD->fProtScreen = pTAInput->fProtScreen;
         pInD->fProtMeta = pTAInput->fProtMeta;
         pInD->fProtCodeBlock = pTAInput->fProtCodeBlock;

         pData->usProgress = 0;

         if ( pTAInput->fInsertTMMatches ||  // autom trans.
              pTAInput->fInsertToTM      ||      // update TM
              pTAInput->fInsertNewMatch  ||   // new match output
              pTAInput->fTMMatch         ||   // TM match counting
              pTAInput->fMachTrans       ||
              pTAInput->fNTLProcessing )
         {
           pTAInput->fSecondRun = TRUE;
         }
         else
         {
           pTAInput->fSecondRun = FALSE;
         } /* endif */
       } /* endif */

       break;

      case EQFTA_INIT:
           usReturn = TACreate ( pTAInput );    // do init processing

           if (usReturn == TACOMPL)
           {
              usPhase = EQFTA_NEXTFILE;
              pInD->pszCurSourceFile = (PSZ)( (PBYTE) pTAInput + // point to first
                          pTAInput->stSourcefiles.ulOffset); // file
              pInD->usCurNumSourceFile = 1;                    // and count
           }
           else
              usPhase = EQFTA_ABORT;
           break;

      case EQFTA_NEXTFILE:    // get next source file and process it
           /*-- if there is a file  to process ---*/
           if (pInD->usCurNumSourceFile <= pTAInput->stSourcefiles.usNumber)
           {
              if ( pInD->pusFileStatus[pInD->usCurNumSourceFile-1] )
              {
                 usFilePhase = EQFTA_INIT;
                 usPhase = EQFTA_FILE;
                 pTAInput->szTempName[0] = EOS;      // no temp file yet
              }                                           /* 4@KIT1322A */
              else
              {
                  pInD->usCurNumSourceFile += 1;  // increase counter
                  pInD->pszCurSourceFile +=           // point to next
                     strlen(pInD->pszCurSourceFile) + 1; // skip NULL
                 usPhase = EQFTA_NEXTFILE;
              } /* endif */
           }  // end if file to process
           else
              usPhase = EQFTA_SEG1;    // do segment processing next
           break;


      case EQFTA_FILE:    // get next source file and process it
           /*-- if there is a file  to process ---*/
           TAFileProcessing( HWND_FUNCIF, pTAInput, &usFilePhase );

           // compute overall progress status
           {
             ULONG ulRange = (( pTAInput->fSecondRun ) ? 50 : 100);
             ULONG ulSlot  = (ulRange / pTAInput->stSourcefiles.usNumber);

             pData->usProgress = (USHORT)(ulSlot * (pInD->usCurNumSourceFile - 1));
             pData->usProgress = pData->usProgress + (USHORT)(pTAInput->sSliderPos * ulSlot / 100);
           }

           if ( usFilePhase == EQFTA_DONE)
           {
             if ( (pInD->usError != 0) && (pInD->usError != TACOMPL) )
             {
               // error conditon in analyis or in the markup table user exit
               usPhase = EQFTA_ABORT;

               usRC = UtlQueryUShort( QS_LASTERRORMSGID );
               if ( usRC == 0 )
               {
                  // no error message box has been shown, probably a user exit failed to run, show standard
                  // analyis error
                  PSZ pszParm = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
                  UtlErrorHwnd( ERROR_TA_UNDEFINEDERR, MB_CANCEL, 1, &pszParm, EQF_ERROR, pTAInput->hwndErrMsg );
                } /* endif */

             }
             else
             {
              usPhase = EQFTA_NEXTFILE;

              // invoke PostTM exit if no further processing is
              // done with the file
              if ( pInD->pusFileStatus[pInD->usCurNumSourceFile-1] &&
                    !(pTAInput->fInsertTMMatches ||  // autom trans.
                    pTAInput->fInsertToTM      ||      // update TM
                    pTAInput->fInsertNewMatch  ||   // new match output
                    pTAInput->fTMMatch         ||   // TM match counting
                    pTAInput->fMachTrans       ||
                    pTAInput->fNTLProcessing ) &&  // list processing
                    (pInD->pfnEQFPostTM || pInD->pfnEQFPostTM2 || pInD->pfnEQFPostTmW) )// any of PSTTM  exits is there
              {
                fOK = TAProcessPostTM( pTAInput, pInD,
                                        pTAInput->szFileName,
                                        pTAInput->szSegTarget,
                                        FALSE );
                  if ( !fOK )
                  {
                    PSZ pszDoc;
                  EVENTLOGFILE();

                  pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];

                  OPENEVENTLOG( "EQFTSEGM" );
                  WRITE2EVENTLOG( "TAProcessPostTM failed, doc is %s", pszDoc );
                  CLOSEEVENTLOG();
                } /* endif */        

                  if ( !fOK )
                  {
                    PSZ pszDoc;
                    pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
                    usReturn = UtlErrorHwnd( ERROR_TA_SAVE_SEGFILE, MB_YESNO, 1,
                                              &pszDoc, EQF_QUERY, HWND_FUNCIF );
                    pTAInput->fKill = TRUE;
                    usPhase = EQFTA_ABORT;
                  } /* endif */
                }                                           /* 4@KIT1322A */
                else
                {
                  usPhase = EQFTA_NEXTFILE;
                } /* endif */
                pInD->usCurNumSourceFile += 1;  // increase counter
                pTAInput->sSliderPos = 0;
                pInD->pszCurSourceFile +=           // point to next
                    strlen(pInD->pszCurSourceFile) + 1; // skip NULL
            } /* endif */
           } /* endif */
           break;


      case EQFTA_SEG1:
           // check if segment processing is needed
           // if so, do initialization
           if (pTAInput->fInsertTMMatches ||  // autom trans.
               pTAInput->fInsertToTM      ||  // update TM
               pTAInput->fInsertNewMatch  ||  // new match output
               pTAInput->fTMMatch         ||   // TM match counting
               pTAInput->fRedundCount     ||   // TM redundancy counting
               pTAInput->fMachTrans       ||
               pTAInput->fNTLProcessing )     // list processing
           {
              usReturn = TASegCreate( HWND_FUNCIF, pTAInput );  // init
              if (usReturn == TACOMPL)
              {
                 pInD->pszCurSourceFile = (PSZ)((PBYTE) pTAInput + // point to first
                             pTAInput->stSourcefiles.ulOffset); // file
                 pInD->usCurNumSourceFile = 1;                    // and count
                 usPhase = EQFTA_SEGNEXT;
              }
              else
                 usPhase = EQFTA_ABORT;
           }
           else
              usPhase = EQFTA_COMPLETE;

           break;

      case EQFTA_SEGNEXT: // get next source
           if (pInD->usCurNumSourceFile <= pTAInput->stSourcefiles.usNumber)
           {   // if file status is TRUE: no error
              if (*(pInD->pusFileStatus + (pInD->usCurNumSourceFile - 1)))
              {
                 usSegPhase = EQFTA_INIT;
                 usPhase = EQFTA_SEG2;
              }
              else
              {
                pInD->usCurNumSourceFile += 1; // increase counter
                pInD->pszCurSourceFile +=     // point to next
                     strlen(pInD->pszCurSourceFile) + 1; // skip NULL
                usPhase = EQFTA_SEGNEXT;
              } /* endif */
           }
           else
           {
              usRedCountPhase = EQFTA_INIT;
              usPhase = EQFTA_REDUNDCOUNT;
           }  /* endif */
           break;

      case EQFTA_SEG2: // segment processing for current source file
           /*-- if there is a file  to process ---*/
           TASegProc( HWND_FUNCIF, pTAInput, &usSegPhase );

           // compute overall progress status
           {
             ULONG ulRange = 50;
             ULONG ulSlot  = (ulRange / pTAInput->stSourcefiles.usNumber);

             pData->usProgress = (USHORT)(50 + (ulSlot * (pInD->usCurNumSourceFile - 1)));
             pData->usProgress = pData->usProgress + (USHORT)(pTAInput->sSliderPos * ulSlot / 100);
           }

           if ( usSegPhase == EQFTA_DONE )
           {
             pInD->usCurNumSourceFile += 1; // increase counter
             pInD->pszCurSourceFile +=     // point to next
                   strlen(pInD->pszCurSourceFile) + 1; // skip NULL
             usPhase = EQFTA_SEGNEXT;
           } /* endif */
           break;

      case EQFTA_REDUNDCOUNT:
        if (pTAInput->fRedundCount)
        {
           TARedundCount( HWND_FUNCIF, pTAInput, &usRedCountPhase );
           if ( usRedCountPhase == EQFTA_DONE )
           {
             usPhase = EQFTA_COMPLETE;
           }
           else
           {
             usPhase = EQFTA_REDUNDCOUNT;
           } /* endif */
        }
        else
           usPhase = EQFTA_COMPLETE;
        break;

      case EQFTA_COMPLETE:
         TACompletion( HWND_FUNCIF, pTAInput);   // terminate text analysis
         pData->usProgress = 100;
         usPhase =  EQFTA_FREE;          // free resources
         break;

      case EQFTA_ABORT:
         pInD->usError = TAERROR;
         usPhase =  EQFTA_FREE;       // free resources
         break;

      case EQFTA_FREE:
      #ifdef MEASUREANALYSISTIME
        TAWriteTimeLog( pTAInput );
      #endif

         // free any loaded CRLF handling info
         TAFreeCRLFInfo( pInD->pCrlfLoadedInfo );

         // free resources
         pTAInput->fNoRemoveObject = TRUE; // no object to remove
         TATerminate( NULLHANDLE, pTAInput );
         usPhase =  EQFTA_DONE;          // done: exit
         break;

      default:
         usPhase =  EQFTA_ABORT;          // terminate task
         break;
   } /* endswitch */

   pData->usAnalysisPhase = (USHORT) usPhase;
   pData->usAnalysisFilePhase = usFilePhase;
   pData->usAnalysisSegPhase = usSegPhase;
   pData->usAnalysisRedCountPhase = usRedCountPhase;

   // set error return code only when analyis has completed its cleanup
   if ( usPhase == EQFTA_DONE )
   {
     if ( pInD->usError != 0 )
     {
       usRC = UtlQueryUShort( QS_LASTERRORMSGID );
     } /* endif */
   } /* endif */

   if ( usPhase == EQFTA_DONE )
   {
     UtlAlloc( (PVOID *) &(pInD), 0L, 0L, NOMSG );    // free instance data space
     pData->fComplete = TRUE;
   } /* endif */


   return( usRC );
}  /* end of TAFuncAnalysisProcess */



static BOOL TAUnSegmentW
(
  PLOADEDTABLE pLoadedTable,           // ptr to loaded document table
  PSZ    pSegTarget,                   // segm.target file
  PSZ    pTarget,                      // target file name
  PTATAG pTATag,                       // pointer to tag str.
  PBOOL  pfGoOn,                       // goon allowed
  PSZ    apszRevMark[6],               // revision mark array
  USHORT usTrackDocNum,                // not 0, TVT tracking document num
  PEQF_BOOL  pfKill,                   // pointer to kill flag
  PSZ    pszConversion,                // PTR to conversion or NULL
  PSZ    pszTgtLang,
  BOOL   fPostUnsegW                   // TRUE = user exit uses EqfPostUnsegW
)
{
   USHORT      usRC = NO_ERROR;
   PTBDOCUMENT pDoc = NULL;
   PBUFCB      pOutFile = NULL;
   PSZ_W       pSegBuffer = NULL;
   PSZ_W       pConvBufferW = NULL;
   PSZ         pConvBuffer = NULL;
   ULONG       ulSegNum;
   PSZ_W       pszTarget = NULL;
   BOOL        fLastCharIsLF = FALSE;
   USHORT      usOldRevMarkCat = 0;
   USHORT      usNewRevMarkCat = 0;
   PTBSEGMENT  pSeg;
   PSZ_W       pRevMarkBufferW = NULL;
   PSZ_W       apszRevMarkW[6];
   ULONG       ulEmptylen = 0;

   USHORT  usConvFlag = CONVERT_USECHARSETINFO;

   pTATag;
   ulEmptylen = UTF16strlenCHAR( EMPTY_TAG );

   *pfGoOn = TRUE;

   // allocate buffer areas
   if ( !UtlAlloc( (PVOID *)&pSegBuffer, 0L, (LONG)(MAX_SEGMENT_SIZE*3), ERROR_STORAGE ) )
   {
     usRC = ERROR_NOT_ENOUGH_MEMORY;
   } /* endif */
   if ( !usRC )
   {
     if ( !UtlAlloc( (PVOID *)&pConvBufferW, 0L, (LONG)(MAX_SEGMENT_SIZE*3), ERROR_STORAGE ) )
     {
       usRC = ERROR_NOT_ENOUGH_MEMORY;
     } /* endif */
   } /* endif */

   // allocate and prepare document structure
   if ( !usRC )
   {
     pConvBuffer = (PSZ) pConvBufferW; //reuse buffer for unicode and other string

     if ( !UtlAlloc( (PVOID *)&pDoc, 0L, (LONG) sizeof(TBDOCUMENT), ERROR_STORAGE ) )
     {
       usRC = ERROR_NOT_ENOUGH_MEMORY;
     }
     else
     {
       usRC = TALoadTagTable( DEFAULT_QFTAG_TABLE, (PLOADEDTABLE *)&pDoc->pQFTagTable,
                              TRUE, TRUE );
     }
   } /* endif */

   // load segmented file
   if ( !usRC )
   {
     // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to TgtLang
     pDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pszTgtLang);
     pDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pszTgtLang);
     usRC = EQFBFileReadExW( pSegTarget, pDoc, 0L );
   } /* endif */

   // open output file
   if ( !usRC )
   {
     usRC = UtlBufOpen( &pOutFile, pTarget, 16000, FILE_CREATE, TRUE );
     if (usRC)
     {
         // file could not be opened or created, because it is already in use by powerpoint
         // error message already displayed, but possibly display a hint to kill powerpoint process
     }
   } /* endif */

   // get conversion codepage and conversion flags
   if ( !usRC )
   {
     if ( fPostUnsegW )
     {
       // user exit uses UTF16 PostUnseg exit, so we have to stay in
       // UTF16 format, it is up to the user exit to do any
       // required conversion in the exit
       usConvFlag = CONVERT_TO_UTF16;
     }
     else
     {
       // standard evaluation for codepage and conversion

       if ( pszConversion )
       {
		 USHORT  usConvCodePage = 0;         // code page for specified conversion
         UtlHandleConversionStrings( CONVCHECK_MODE, NULLHANDLE,
                                     pszConversion, &usConvCodePage, &usConvFlag );
         // RJ: 040216: usConvCodePage is not used later on!!
         // for re-activating, please check total TAUnSegmentW function!
       }
       else
       {
          // do no charset conversion if a UserExit is involved
          // The standard userexits are doing the conversion from ASCII to ANSI
          // in the PostUnseg phase
          // NOT VALID ANY MORE (020918 RJO)
          //if ( (pLoadedTable->usCharacterSet==TAGCHARSET_ANSI) &&
          //      pLoadedTable->pTagTable->szSegmentExit[0] )
          //{
          //  usConvFlag = CONVERT_TO_ASCII;
          //}
       } /* endif */

       if ( usConvFlag == CONVERT_USECHARSETINFO )
       {
         switch ( pLoadedTable->usCharacterSet )
         {
           case TAGCHARSET_ANSI:
             usConvFlag = CONVERT_TO_ANSI;
             break;
           case TAGCHARSET_UNICODE:
             usConvFlag = CONVERT_TO_UTF16;
             break;
           case TAGCHARSET_UTF8:
             usConvFlag = CONVERT_TO_UTF8;
             break;
           case TAGCHARSET_ASCII:
           default:
             usConvFlag = CONVERT_TO_ASCII;
             break;
         } /* endswitch */
       } /* endif */
     } /* endif */
   } /* endif */

   // do character set specific preprocessing
   if ( !usRC )
   {
     switch ( usConvFlag )
     {
       case CONVERT_TO_ANSI:
         break;

       case CONVERT_TO_UTF16:
         usRC = UtlBufWrite( pOutFile, UNICODEFILEPREFIX,
                             (USHORT)strlen(UNICODEFILEPREFIX), TRUE );
         break;

       case CONVERT_TO_UTF8:
         // write BOM if source file contains UTF8-BOM
         // according to David Walers all UTF8 files are written without BOM!
         //{
         //  CHAR szSourceFile[MAX_EQF_PATH];
         //  CHAR szDocName[MAX_FILESPEC];
         //  CHAR szFolName[MAX_FILESPEC];
         //  FILE *hfTest;

         //  strcpy( szSourceFile, pSegTarget );
         //  strcpy ( szDocName, UtlSplitFnameFromPath( szSourceFile ) );
         //  UtlSplitFnameFromPath( szSourceFile );
         //  strcpy ( szFolName, UtlSplitFnameFromPath( szSourceFile ) );
         //  UtlMakeEQFPath( szSourceFile, szSourceFile[0], DIRSOURCEDOC_PATH, szFolName );
         //  strcat( szSourceFile, BACKSLASH_STR );
         //  strcat( szSourceFile, szDocName );
         //
         //  hfTest = fopen( szSourceFile, "rb" );
         //  if ( hfTest )
         //  {
         //    CHAR buf[11];
         //    USHORT usLen = (USHORT)strlen(UTF8FILEPREFIX);

         //    fread( buf, 1, usLen, hfTest );
         //    fclose( hfTest );

         //    if ( memcmp( buf, UTF8FILEPREFIX, usLen ) == 0 )
         //    {
         //      usRC = UtlBufWrite( pOutFile, UTF8FILEPREFIX, usLen, TRUE );
         //    } /* endif */
         //  } /* endif */
         //}
         break;

       case CONVERT_TO_ASCII:
       default:
         break;
     } /* endswitch */
   } /* endif */

   ulSegNum = 1L;
   usOldRevMarkCat = 0;
   usNewRevMarkCat = 0;
   // convert RevMarks into UNICODE strings

  if ( !UtlAlloc( (PVOID *)&pRevMarkBufferW, 0L,
                  (LONG)(MAX_REVMARK_SIZE*6* sizeof(CHAR_W)), ERROR_STORAGE ) )
  {
     usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */
  if (!usRC)
  {
    USHORT  usOffset = 0;
    USHORT  i = 0;
    LONG    lLen = 0;
    for ( i = 0; i < 6 ; i ++ )
    {
      apszRevMarkW[i] = pRevMarkBufferW + usOffset;
      lLen = MultiByteToWideChar( pDoc->ulOemCodePage, 0, apszRevMark[i],  //input
                                 strlen(apszRevMark[i]),
                                 (LPWSTR)apszRevMarkW[i],      //output
                                  MAX_REVMARK_SIZE );

      usOffset += MAX_REVMARK_SIZE;
    }
  } /* endif */

  // add TVT tracking IDs only for UTF-8 or UTF-16 encoded files.
  // this is based on <charset> setting in TBX files.
  if ( usTrackDocNum ) {
     if ( ( pLoadedTable->usCharacterSet != CONVERT_TO_UTF8  ) &&
          ( pLoadedTable->usCharacterSet != CONVERT_TO_UTF16 ) )
     {
        usTrackDocNum = 0 ;
     }
  }


   while ( !usRC && (ulSegNum < pDoc->ulMaxSeg) && !*pfKill )                                 /* @50C */
   {
     pSeg = EQFBGetSegW( pDoc, ulSegNum );
     usOldRevMarkCat = usNewRevMarkCat;
     usNewRevMarkCat = 0;
     pszTarget = NULL;

     if ( pSeg && !pSeg->SegFlags.Joined )
     {
       // write Status state attribute to file
      /**********************************************************/
      /* if Typed = TRUE and COpied = TRUE -> szStatusAttr =2  */
      /* if Typed = TRUE and Copied =FALSE -> szStatusAttr =1  */
      /* if Typed =FALSE and Copied = TRUE -> szStatusAttr =3  */
      /**********************************************************/
      if ( pSeg->SegFlags.Typed || pSeg->SegFlags.Copied  )
      {
         if ( pSeg->SegFlags.Typed )
         {
           if ( pSeg->SegFlags.Copied )
           {
             usNewRevMarkCat = 2;
           }
           else
           {
             usNewRevMarkCat = 1;
           } /* endif */
         }
         else
         {
            usNewRevMarkCat = 3;
         } /* endif */
       } /* endif */

       pszTarget = TAAddRevMark(usOldRevMarkCat, usNewRevMarkCat, pSegBuffer,
                                apszRevMarkW, fLastCharIsLF );

       // restore original LF sequence : what about linebreaks in RevMarks?
       {
         PSZ_W pszSource = pSeg->pDataW;
         //pszTarget = pSegBuffer;
         int iCRLFLen =  UTF16strlenCHAR( pDoc->szLineBreakW );

         while ( *pszSource )
         {
           if ( *pszSource == LF )
           {
             UTF16strcpy( pszTarget, pDoc->szLineBreakW );
             pszTarget += iCRLFLen;
             pszSource++;
             fLastCharIsLF = TRUE;
           }
           else
           {
             *pszTarget++ = *pszSource++;
             fLastCharIsLF = FALSE;
           } /* endif */
         } /* endwhile */
         *pszTarget = EOS;
       }

       if ( usTrackDocNum )
       {
         if ( pSeg->qStatus==QF_XLATED || pSeg->qStatus==QF_TOBE || pSeg->qStatus==QF_CURRENT )
         {
           PSZ pszBinary ;
           char szBinaryString[65];

           // skip trailing newline characters in segment
           int iNewline = 0;
           for( int i=wcslen(pSegBuffer)-1 ; i>0 && iswspace(pSegBuffer[i]) ; --i ) {
              if ( wcschr( L"\n\r", pSegBuffer[i] ) )
                   iNewline = i;
           }
           if ( iNewline ) {
              wcscpy( pConvBufferW, &pSegBuffer[iNewline] );
              pSegBuffer[iNewline] = EOS;
           }

           // make room in pSegBuffer for tracking info start identifier
           int iLen = wcslen(pSegBuffer);
           memmove( pSegBuffer + 1, pSegBuffer, iLen*sizeof(CHAR_W) );

           // insert tracking info segment start identifier
           *pSegBuffer = L'\x200B';
           iLen++;

           // append tracking info segment end identifier
           pSegBuffer[iLen++] = L'\x2060';

           // append binary file number part of ID 
           itoa( usTrackDocNum, szBinaryString, 2 );
           pszBinary = szBinaryString ;
           while ( *pszBinary == '0' ) { pszBinary++; };
           while ( *pszBinary != '\0' )
           {
             if ( *pszBinary == '0' )
             {
               pSegBuffer[iLen++] = L'\x200C';
             }
             else 
             if ( *pszBinary == '1' )
             {
               pSegBuffer[iLen++] = L'\x200D';
             }
             pszBinary++;
           }

           // append file/segment number separator
           pSegBuffer[iLen++] = L'\x2060';

           // append binary encode segment number
           ltoa( ulSegNum, szBinaryString, 2 );
           pszBinary = szBinaryString;
           while ( *pszBinary == '0' ) { pszBinary++; };
           while ( *pszBinary != '\0' )
           {
             if ( *pszBinary == '0' )
             {
               pSegBuffer[iLen++] = L'\x200C';
             }
             else 
             if ( *pszBinary == '1' )
             {
               pSegBuffer[iLen++] = L'\x200D';
             }
             pszBinary++;
           }
           pSegBuffer[iLen] = EOS;
           if ( iNewline ) {    /* Add stripped newline characters */
              wcscat( pSegBuffer, pConvBufferW );
           }
         }
       }

       // apply any conversion and write data
       {
         ULONG ulLen;
         // only write if segment is not the :NONE tag
         if ( !UTF16strncmp(pSegBuffer, EMPTY_TAG, (USHORT)ulEmptylen) == 0 )
         {
             switch ( usConvFlag )
             {
               case CONVERT_TO_ANSI:
               { LONG lRc = NO_ERROR;
                 // convert data from unicode to ANSI codepage and set length
                 //ulLen in number of CHAR_W's which are in pszUni!
                 ulLen = UtlDirectUnicode2AnsiBuf( pSegBuffer, pConvBuffer,
                                           UTF16strlenCHAR(pSegBuffer),
                                           MAX_SEGMENT_SIZE * 3,
                             pDoc->ulAnsiCodePage, TRUE, &lRc );
                 usRC = (USHORT)lRc;
			   }
                 break;

               case CONVERT_TO_UTF8:
                 // convert data and set length
                 // convert from UTF16 to UTF8 codepage
                 WideCharToMultiByte(CP_UTF8, 0, pSegBuffer, /* wide charstr */
                                     UTF16strlenCHAR(pSegBuffer),
                                     pConvBuffer, /* output buf */
                                     MAX_SEGMENT_SIZE * 3, /* bytes in output buf */
                                     NULL, NULL );

                 // output now again in pSegBuffer, so copy again
                 ulLen = (USHORT)strlen(pConvBuffer);
                 break;

               case CONVERT_TO_UTF16:
                 UTF16strcpy(pConvBufferW, pSegBuffer );
                 ulLen = (USHORT)UTF16strlenBYTE(pConvBufferW);
                 break;

               case CONVERT_TO_ASCII:
               default:
                 // copy data to conversion buffer and set length
                 ulLen =Unicode2ASCIIBuf( pSegBuffer, pConvBuffer,
                                          UTF16strlenCHAR(pSegBuffer),
                                          MAX_SEGMENT_SIZE * 3,
                                          pDoc->ulOemCodePage);
                 break;
             } /* endswitch */

             // write segment data - ulLen is number of bytes to be written
             usRC = UtlBufWrite( pOutFile, pConvBuffer, ulLen, TRUE );
         } /* endif */
         memset(pConvBuffer, 0, MAX_SEGMENT_SIZE * 3 );
         memset( pSegBuffer, 0, MAX_SEGMENT_SIZE * 3 );
       }
     } /* endif */
     ulSegNum++;
   } /* endwhile */

   if (!usRC && (ulSegNum == pDoc->ulMaxSeg) && !*pfKill )
   {
     ULONG  ulLen = 0;
     usOldRevMarkCat = usNewRevMarkCat;
     usNewRevMarkCat = 0;
     pszTarget = TAAddRevMark(usOldRevMarkCat, usNewRevMarkCat, pSegBuffer,
                                     apszRevMarkW, fLastCharIsLF );
     if ( !UTF16strncmp(pSegBuffer, EMPTY_TAG, (USHORT)ulEmptylen) == 0 )
     {
         switch ( usConvFlag )
         //switch ( pLoadedTable->usCharacterSet )
         {
           case CONVERT_TO_ANSI:/*TAGCHARSET_ANSI:*/
           { LONG lRc = NO_ERROR;
             // convert data and set length
             // convert from unicode to ANSI codepage
             ulLen = UtlDirectUnicode2AnsiBuf( pSegBuffer, pConvBuffer,
                                           UTF16strlenCHAR(pSegBuffer),
                                           MAX_SEGMENT_SIZE * 3,
                             pDoc->ulAnsiCodePage, TRUE, &lRc );
             usRC = (USHORT)lRc;
	       }
             break;
           case CONVERT_TO_UTF8:
			 // convert data and set length
			 // convert from UTF16 to UTF8 codepage
			 WideCharToMultiByte(CP_UTF8, 0, pSegBuffer, /* wide charstr */
								 UTF16strlenCHAR(pSegBuffer),
								 pConvBuffer, /* output buf */
								 MAX_SEGMENT_SIZE * 3, /* bytes in output buf */
								 NULL, NULL );

			 // output now again in pSegBuffer, so copy again
			 ulLen = (USHORT)strlen(pConvBuffer);
			 break;

           case CONVERT_TO_UTF16:/*TAGCHARSET_UNICODE:*/
             UTF16strcpy(pConvBufferW, pSegBuffer );
             ulLen = (USHORT)UTF16strlenBYTE(pConvBufferW);
             break;

           case CONVERT_TO_ASCII:/*TAGCHARSET_ASCII:*/
           default:
             // copy data to conversion buffer and set length
             ulLen =Unicode2ASCIIBuf( pSegBuffer, pConvBuffer,
                                      UTF16strlenCHAR(pSegBuffer),
                                      MAX_SEGMENT_SIZE * 3,
                                      pDoc->ulOemCodePage);
             break;
         } /* endswitch */

         usRC = UtlBufWrite( pOutFile, pConvBuffer, ulLen, TRUE );
      } /* endif */
   }

   // cleanup
   if ( pSegBuffer ) UtlAlloc( (PVOID *)&pSegBuffer, 0L, 0L, NOMSG );
   if ( pConvBufferW ) UtlAlloc( (PVOID *)&pConvBufferW, 0L, 0L, NOMSG );
   if ( pRevMarkBufferW ) UtlAlloc( (PVOID *)&pRevMarkBufferW, 0L, 0L, NOMSG );

   TAFreeDoc((PVOID *) &pDoc);

   if (pOutFile)
      usRC = UtlBufClose( pOutFile, TRUE );

   if ( usRC )                                     // delete target file
   {                                               // in case of problems
      UtlDelete( pTarget, 0L , FALSE );
   } /* endif */


   return( usRC == NO_ERROR );
} /* end of function TAUnSegmentW */

BOOL
TAGetUnicodeFlagFromDocProp
(
  PSZ  pszDocObjName
)
{
  BOOL fOK = TRUE;
  HPROP      hPropDoc = NULL;
  ULONG      ulErrorInfo = 0L;
  PPROPDOCUMENT ppropDoc;
  BOOL    fSTargetInUnicode;

   //open document properties to get fSTargetInUnicode Flag
   if( fOK && ((hPropDoc = OpenProperties( pszDocObjName, NULL,
                                       PROP_ACCESS_READ, &ulErrorInfo))== NULL))
   {
      fOK = FALSE;
      // display error message if not already displayed
      if ( ulErrorInfo != Err_NoStorage )
      {
         UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszDocObjName, EQF_ERROR);
      } /* endif */
   }/*endif*/

     ppropDoc =(PPROPDOCUMENT) MakePropPtrFromHnd(hPropDoc );
     fSTargetInUnicode = ppropDoc->fSTargetInUnicode;
     if ( hPropDoc )
     {
       CloseProperties( hPropDoc, PROP_FILE, &ulErrorInfo);
       hPropDoc = NULL;
     } /* endif */
     return (fSTargetInUnicode);
} /* end of function   */



 /*************************************************/
 /* Add revision mark strings to output           */
 /************************************************/
static PSZ_W
TAAddRevMark
(
  USHORT   usOldRevMarkCat,
  USHORT   usNewRevMarkCat,
  PSZ_W    pSegBuffer,
  PSZ_W    apszRevMark[6],
  BOOL     fLastCharIsLF
)
{
  PSZ_W    pszTarget = NULL;
  PSZ_W      pszRevMark;

  pszTarget = pSegBuffer;

    if ( usOldRevMarkCat != usNewRevMarkCat )
    {
      /***********************************************/
      /* Get closing revision mark string            */
      /***********************************************/
      switch ( usOldRevMarkCat )
      {
        case 1 :
          pszRevMark = apszRevMark[1];
          break;
        case 2 :
          pszRevMark = apszRevMark[3];
          break;
        case 3 :
          pszRevMark = apszRevMark[5];
          break;
        case 0 :
        default :
          pszRevMark = NULL;
          break;
      } /* endswitch */

      /***********************************************/
      /* Add closing revision mark string to output  */
      /* buffer                                      */
      /***********************************************/
      if ( pszRevMark && *pszRevMark )
      {
        /*********************************************/
        /* ignore first CRLF of revision mark if     */
        /* we are at the start of a line already     */
        /*********************************************/
        if ( ( pszRevMark[0] == CR ) &&
             ( pszRevMark[1] == LF ) &&
             ( fLastCharIsLF ) )
        {
          pszRevMark += 2;
        } /* endif */

        /*********************************************/
        /* Copy revision mark to output buffer       */
        /*********************************************/
        memcpy( pszTarget, pszRevMark, UTF16strlenBYTE(pszRevMark) );
        pszTarget += UTF16strlenCHAR(pszRevMark);

      } /* endif */

      /***********************************************/
      /* Get opening revision mark string            */
      /***********************************************/
      switch ( usNewRevMarkCat )
      {
        case 1 :
          pszRevMark = apszRevMark[0];
          break;
        case 2 :
          pszRevMark = apszRevMark[2];
          break;
        case 3 :
          pszRevMark = apszRevMark[4];
          break;
        case 0 :
        default :
          pszRevMark = NULL;
          break;
      } /* endswitch */

      /***********************************************/
      /* Add opening revision mark string to output  */
      /* buffer                                      */
      /***********************************************/
      if ( pszRevMark && *pszRevMark )
      {
        /*********************************************/
        /* ignore first CRLF of revision mark if     */
        /* we are at the start of a line already     */
        /*********************************************/
        if ( ( pszRevMark[0] == CR ) &&
             ( pszRevMark[1] == LF ) &&
             ( *(pszTarget - 1) == LF ) )
        {
          pszRevMark += 2;
        } /* endif */

        /*********************************************/
        /* Copy revision mark to output buffer       */
        /*********************************************/
        memcpy( pszTarget, pszRevMark, UTF16strlenBYTE(pszRevMark) );
        pszTarget += UTF16strlenCHAR(pszRevMark);

      } /* endif */
    }  /* endif */
  return(pszTarget);
}



USHORT TASegFileConvertUnicode2ASCII( PSZ pInFile, PSZ pOutFile, PSZ pszDocLang)
{
  PTBDOCUMENT  pDoc = NULL;
  USHORT       usRc = 0;
  ULONG        i = 0;
  BOOL         fIsUnicode = FALSE;

  usRc = TAIsFileUnicode(pInFile, &fIsUnicode);

  if ( usRc != 0 )
  {
    EVENTLOGFILE();
    OPENEVENTLOG( "EQFTSEGM" );
    WRITE2EVENTLOG( "TAIsFileUnicode in TASegFileConvertUnicode2ASCII failed, file is %s", pInFile );
    CLOSEEVENTLOG();
  } /* endif */     

  // if !usRc and file is not in Unicode, conversion is not nec.
  if (!usRc && !fIsUnicode )
  { // just copy file
      usRc = UtlCopy( pInFile, pOutFile, 1,  0L, TRUE );
      if ( usRc != 0 )
      {
        EVENTLOGFILE();
        OPENEVENTLOG( "EQFTSEGM" );
        WRITE2EVENTLOG( "UtlCopy in TASegFileConvertUnicode2ASCII failed, source file is %s", pInFile );
        WRITE2EVENTLOG( "--- target file is %s", pOutFile );
        CLOSEEVENTLOG();
      } /* endif */     
  }
  else
  {   // load internal tag table
      usRc = (UtlAlloc((PVOID *) &pDoc, 0, sizeof(TBDOCUMENT), NOMSG ) == TRUE) ? 0 : ERROR_STORAGE;

      if ( !usRc )
      {
        pDoc->docType = STARGET_DOC;  // in this case X= info is maintained

        usRc = TALoadTagTable( DEFAULT_QFTAG_TABLE, (PLOADEDTABLE *)&pDoc->pQFTagTable, TRUE, FALSE );
        if ( usRc != 0 )
        {
          EVENTLOGFILE();
          OPENEVENTLOG( "EQFTSEGM" );
          WRITE2EVENTLOG( "TALoadTagTable in TASegFileConvertUnicode2ASCII failed, file is %s", pInFile );
          CLOSEEVENTLOG();
        } /* endif */     
      } /* endif */

      // read file in Unicode
      if ( !usRc )
      {
        // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to DocLang
        pDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pszDocLang);
        pDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pszDocLang);

        usRc = EQFBFileReadExW( pInFile, pDoc, 0L );
        if ( usRc != 0 )
        {
          EVENTLOGFILE();
          OPENEVENTLOG( "EQFTSEGM" );
          WRITE2EVENTLOG( "EQFBFileReadExW in TASegFileConvertUnicode2ASCII failed, file is %s", pInFile );
          CLOSEEVENTLOG();
        } /* endif */     
      }

      // get all segments in ASCII
      if ( !usRc )
      {
        for ( i=1; i< pDoc->ulMaxSeg; i++)
        {
          EQFBGetSeg( pDoc, i );
        }
      }

      // save file in ASCII
      if ( !usRc )
      {
        HFILE hOutFile;
        USHORT usOpenAction;
        USHORT  usDosRC = 0;
        usDosRC = UtlOpen( pOutFile,
                         &hOutFile,
                         &usOpenAction, 0L,
                         FILE_NORMAL,
                         FILE_TRUNCATE | FILE_CREATE,
                         OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                         0L,
                         TRUE );
        usRc = ( usDosRC ) ? ERR_OPENFILE : 0;
        if ( usRc != 0 )
        {
          EVENTLOGFILE();
          OPENEVENTLOG( "EQFTSEGM" );
          WRITE2EVENTLOG( "UtlOpen in TASegFileConvertUnicode2ASCII failed, file is %s", pOutFile );
          CLOSEEVENTLOG();
        } /* endif */     
        if (!usRc)
        {
          UtlClose( hOutFile, FALSE );
          usRc = EQFBFileWriteEx( pOutFile, pDoc, 0, SGMLFORMAT_ASCII );
          if ( usRc != 0 )
          {
            EVENTLOGFILE();
            OPENEVENTLOG( "EQFTSEGM" );
            WRITE2EVENTLOG( "EQFBFileWriteEx in TASegFileConvertUnicode2ASCII failed, file is %s", pOutFile );
            CLOSEEVENTLOG();
          } /* endif */     
        }
      }
      if ( pDoc->pQFTagTable )
      {
         TAFreeTagTable( (PLOADEDTABLE) pDoc->pQFTagTable );
      } /* endif */
      // free document resources
      SegFileFreeDoc((PVOID *) &pDoc );
  }

  return usRc;
}


USHORT TASegFileConvertASCII2Unicode( PSZ pInFile, PSZ pOutFile, PSZ pszDocLang )
{
  PTBDOCUMENT  pDoc = NULL;
  USHORT       usRc = 0;
  BOOL         fIsUnicode = FALSE;

  usRc = TAIsFileUnicode(pInFile, &fIsUnicode);
  if ( usRc != 0 )
  {
    EVENTLOGFILE();
    OPENEVENTLOG( "EQFTSEGM" );
    WRITE2EVENTLOG( "TAIsFileUnicode in TASegFileConvertASCII2Unicode failed, file is %s", pInFile );
    CLOSEEVENTLOG();
  } /* endif */     

  // If file is already Unicode, no conversion is nec., only copy of file
  // read file
  if (!usRc && fIsUnicode)
  {
      usRc = UtlCopy( pInFile, pOutFile, 1,  0L, TRUE );
      if ( usRc != 0 )
      {
        EVENTLOGFILE();
        OPENEVENTLOG( "EQFTSEGM" );
        WRITE2EVENTLOG( "UtlCopy in TASegFileConvertASCII2Unicode failed, source file is %s", pInFile );
        WRITE2EVENTLOG( "--- target file is %s", pOutFile );
        CLOSEEVENTLOG();
      } /* endif */     
  }
  else
  {      // load internal tag table
      usRc = (UtlAlloc((PVOID *) &pDoc, 0, sizeof(TBDOCUMENT), NOMSG ) == TRUE) ? 0 : ERROR_STORAGE;

      if ( !usRc )
      {
        pDoc->docType = STARGET_DOC;  // in this case X= info is maintained

        usRc = TALoadTagTable( DEFAULT_QFTAG_TABLE,
                              (PLOADEDTABLE *)&pDoc->pQFTagTable,
                              TRUE, FALSE );
        if ( usRc != 0 )
        {
          EVENTLOGFILE();
          OPENEVENTLOG( "EQFTSEGM" );
          WRITE2EVENTLOG( "TALoadTagTable in TASegFileConvertASCII2Unicode failed, file is %s", pInFile );
          CLOSEEVENTLOG();
        } /* endif */     
      } /* endif */

      // read file
      if ( !usRc )
      {
        HFILE   hOutFile;
        USHORT  usOpenAction;
        USHORT  usDosRC = 0;

        usDosRC = UtlOpen( pOutFile,
                         &hOutFile,
                         &usOpenAction, 0L,
                         FILE_NORMAL,
                         FILE_TRUNCATE | FILE_CREATE,
                         OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                         0L,
                         TRUE );
        usRc = ( usDosRC ) ? ERR_OPENFILE : 0;
        if ( usRc != 0 )
        {
          EVENTLOGFILE();
          OPENEVENTLOG( "EQFTSEGM" );
          WRITE2EVENTLOG( "UtlOpen in TASegFileConvertASCII2Unicode failed, file is %s", pInFile );
          CLOSEEVENTLOG();
        } /* endif */     
        if (!usRc)
        {
          UtlClose( hOutFile, FALSE );
          // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to DocLang
          pDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pszDocLang);
          pDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pszDocLang);

          usRc = EQFBFileReadExW( pInFile, pDoc, 0L );
          if ( usRc != 0 )
          {
            EVENTLOGFILE();
            OPENEVENTLOG( "EQFTSEGM" );
            WRITE2EVENTLOG( "EQFBFileReadExW in TASegFileConvertASCII2Unicode failed, file is %s", pInFile );
            CLOSEEVENTLOG();
          } /* endif */     
        }
      }

      // save file in Unicode
      if ( !usRc )
      {
        usRc = EQFBFileWriteEx( pOutFile, pDoc, 0, SGMLFORMAT_UNICODE );
        if ( usRc != 0 )
        {
          EVENTLOGFILE();
          OPENEVENTLOG( "EQFTSEGM" );
          WRITE2EVENTLOG( "EQFBFileWriteEx in TASegFileConvertASCII2Unicode failed, file is %s", pOutFile );
          CLOSEEVENTLOG();
        } /* endif */     
      }
      if ( pDoc->pQFTagTable )
      {
         TAFreeTagTable( (PLOADEDTABLE) pDoc->pQFTagTable );
      } /* endif */
      // free document resources
      SegFileFreeDoc((PVOID *) &pDoc );
  }
  return usRc;
}

USHORT
TAIsFileUnicode
(
    PSZ   pInFile,
    PBOOL  pfIsUnicode
)
{ PSZ     pszPrefix = UNICODEFILEPREFIX;
  int     iLen = strlen(pszPrefix);
  HFILE   hInFile;
  USHORT  usOpenAction;
  USHORT  usDosRC = 0;
  CHAR         ch[10];
  PSZ          pData = &ch[0];
  ULONG       ulBytesRead;
  USHORT       usRc = 0;


  *pfIsUnicode = FALSE;

  usDosRC = UtlOpen( pInFile,
                   &hInFile,
                   &usOpenAction, 0L,
                   FILE_NORMAL,
                   FILE_OPEN,
                   OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                   0L,
                   TRUE );
  usRc = ( usDosRC ) ? ERR_OPENFILE : 0;

  if (!usRc)
  {
    usRc = UtlReadL( hInFile, pData, 8, &ulBytesRead, FALSE );

     if ( !usRc && (memcmp( pData, pszPrefix, iLen ) == 0 ))
     {
       *pfIsUnicode = TRUE;
     }
     UtlClose( hInFile, FALSE );
  }

  return usRc;
}



BOOL
TAProcessPostTM
(
  PTAINPUT       pTAInput,
  PTAINSTDATA    pInD,
  PSZ            pszFileName,
  PSZ            pszSegTarget,
  BOOL           fTMInserted
)
{  BOOL fOK = TRUE;

   sprintf(pszFileName, PATHCATFILE,
            pTAInput->szSEGSOURCE_Path,
            pInD->pszCurSourceFile);

    sprintf(pszSegTarget, PATHCATFILE,
            pTAInput->szSEGTARGET_Path,
            pInD->pszCurSourceFile);
    if ( pInD->pfnEQFPostTmW)
    {
      ULONG ulTemp = 0;
      UtlTime( (PLONG)&ulTemp );
      SetSegDate (pTAInput, ulTemp );
      pTAInput->fSetSegDateDone = TRUE;

      fOK = pInD->pfnEQFPostTmW(pTAInput->szTagTableName,
                                       pTAInput->szEditName,
                                       pTAInput->szProgPath,
                                       pszFileName,
                                       pszSegTarget,
                                       pTAInput->TATagW,
                                       (EQF_BOOL)fTMInserted,
                                       &pInD->ulSegsReplaced,
                                       &pInD->ulSegsNotReplaced,
                                       pTAInput->hwndProcWin,
                                       (PEQF_BOOL)&pTAInput->fKill );
       if ( !fOK )
       {
          EVENTLOGFILE();
          OPENEVENTLOG( "EQFTSEGM" );
          WRITE2EVENTLOG( "TAProcessPostTM: pfnEQFPostTmW failed, doc is %s", pszFileName );
          CLOSEEVENTLOG();
       } /* endif */          

    }
    else
    {
      if ( pInD->pfnEQFPostTM || pInD->pfnEQFPostTM2 )
      {
	    ULONG ulTemp = 0;
        CHAR  chFileName[MAX_EQF_PATH + 1];
        CHAR  chSegTarget[MAX_EQF_PATH + 1];
        strcpy(chFileName, pszFileName);
        strcat( chFileName, "A");
        strcpy( chSegTarget, pszSegTarget);
        strcat( chSegTarget, "B");

        UtlTime( (PLONG)&ulTemp );
        SetSegDate (pTAInput, ulTemp );
        pTAInput->fSetSegDateDone = TRUE;
	
        fOK = (TASegFileConvertUnicode2ASCII( pszFileName,
                                              chFileName,
                                              pInD->szDocSourceLang) ) == 0;
        fOK = (TASegFileConvertUnicode2ASCII( pszSegTarget,
                                              chSegTarget,
                                              pInD->szDocTargetLang) ) == 0;
        if ( !fOK )
        {
            EVENTLOGFILE();

            OPENEVENTLOG( "EQFTSEGM" );
            WRITE2EVENTLOG( "TAProcessPostTM: TASegFileConvertUnicode2ASCII failed, doc is %s", pszFileName );
            CLOSEEVENTLOG();
        } /* endif */          

        if ( fOK && pInD->pfnEQFPostTM2 == NULL )
        {
          fOK =  pInD->pfnEQFPostTM( pTAInput->szTagTableName,
                                     pTAInput->szEditName,
                                     pTAInput->szProgPath,
                                     chFileName,
                                     chSegTarget,
                                     pTAInput->TATag,
                                     (EQF_BOOL)fTMInserted);
          if ( !fOK )
          {
              EVENTLOGFILE();
              OPENEVENTLOG( "EQFTSEGM" );
              WRITE2EVENTLOG( "TAProcessPostTM: pfnEQFPostTM failed, doc is %s", chFileName );
              CLOSEEVENTLOG();
          } /* endif */          
        }
        else if (fOK)
        {
          fOK = pInD->pfnEQFPostTM2(pTAInput->szTagTableName,
                                    pTAInput->szEditName,
                                    pTAInput->szProgPath,
                                    chFileName,
                                    chSegTarget,
                                    pTAInput->TATag,
                                    (EQF_BOOL)fTMInserted,
                                    &pInD->ulSegsReplaced,
                                    &pInD->ulSegsNotReplaced,
                                    pTAInput->hwndProcWin,
                                    (PEQF_BOOL)&pTAInput->fKill );
          if ( !fOK )
          {
              EVENTLOGFILE();
              OPENEVENTLOG( "EQFTSEGM" );
              WRITE2EVENTLOG( "TAProcessPostTM: pfnEQFPostTM2 failed, doc is %s", chFileName );
              CLOSEEVENTLOG();
          } /* endif */          
        } /* endif */
        if (fOK)
        {
          fOK = (TASegFileConvertASCII2Unicode( chFileName, pszFileName,
                                                 pInD->szDocSourceLang) ) == 0;
          if ( fOK)
          {
            fOK = ( TASegFileConvertASCII2Unicode( chSegTarget, pszSegTarget,
                                                   pInD->szDocTargetLang) ) == 0;
          }
          if ( !fOK )
          {
              EVENTLOGFILE();
              OPENEVENTLOG( "EQFTSEGM" );
              WRITE2EVENTLOG( "TAProcessPostTM: TASegFileConvertASCII2Unicode failed, doc is %s", pszFileName );
              CLOSEEVENTLOG();
          } /* endif */          
          UtlDelete( chFileName, 0L, NOMSG );
          UtlDelete( chSegTarget, 0L, NOMSG );
        }
      }  /* endif */
    }

  return(fOK);
}

BOOL TARestartAnalysisForNewFolder
(
  PTAINPUT    *ppTAInput,              // address of callers TAInput structure pointer
  PSZ         pszNewFolObjName         // points to object name of new folder
)
{
  BOOL fOK = TRUE;                     // internal OK flag
  PTAINPUT    pTAInput = *ppTAInput;   // ptr to TAINPUT structure
  LONG lOldTAInputSize;                // old size of TAINPUT structure
  HWND hwndDocsLB = NULLHANDLE;        // listbox for folder documents
  PSZ        pszDict[20];              // buffer for names of dictionaries
  PSZ        pszTM[20];                // buffer for names of input TMs
  PSZ pszAct, pszStart;                // ptr for TAINPUT processing
  PSZ        pszMorphDict = NULL;      // ptr to save morphdict name

  // free any pInD structure
  if ( pTAInput->pInD ) UtlAlloc( (PVOID *)&pTAInput->pInD, 0L, 0L, NOMSG );

  // create listbox for the documents of the folder
  hwndDocsLB = WinCreateWindow( pTAInput->hwndProcWin, WC_LISTBOX, "",
                                WS_CHILDWINDOW | LBS_STANDARD,
                                0, 0, 10, 10, pTAInput->hwndProcWin,
                                HWND_TOP, 4711, NULL, NULL );

  // remember current size of TAINPUT structure
  lOldTAInputSize = ((LONG)(pTAInput->stSourcefiles.usNumber +
                            pTAInput->stInputDict.usNumber   +
                            pTAInput->stInputMemDb.usNumber  +
                            pTAInput->stMorphDict.usNumber) *
                            (LONG)MAX_EQF_PATH ) +
                            (LONG)sizeof(TAINPUT);

  // get the documents of the folder
  if ( fOK )
  {
    EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                     MP1FROMHWND( hwndDocsLB ),
                     MP2FROMP(pszNewFolObjName) );
  } /* endif */

  // create string pool
  if ( fOK )
  {
    pTAInput->pPool = PoolCreate( 32000 );
    fOK = (pTAInput->pPool != NULL);
  } /* endif */

  // save dict and TM names and re-allocate TAINPUT
  if ( fOK )
  {
    int i;
    PSZ pszTemp;

    // save dictionaries in string pool
    pszTemp = (PSZ)pTAInput + pTAInput->stInputDict.ulOffset;
    for ( i = 0; i < (int)pTAInput->stInputDict.usNumber; i++ )
    {
      pszDict[i] = PoolAddString( pTAInput->pPool, pszTemp );
      pszTemp += strlen(pszTemp) + 1;
    } /* endfor */

    // save input TMs in string pool
    pszTemp = (PSZ)pTAInput + pTAInput->stInputMemDb.ulOffset;
    for ( i = 0; i < (int)pTAInput->stInputMemDb.usNumber; i++ )
    {
      pszTM[i] = PoolAddString( pTAInput->pPool, pszTemp );
      pszTemp += strlen(pszTemp) + 1;
    } /* endfor */

    // save morphdict in string pool
    if ( pTAInput->stMorphDict.ulOffset )
    {
      pszTemp = (PSZ)pTAInput + pTAInput->stMorphDict.ulOffset;
      pszMorphDict = PoolAddString( pTAInput->pPool, pszTemp );
    }
    else
    {
      pszMorphDict = NULL;
    } /* endif */

    // set number of source files
    pTAInput->stSourcefiles.usNumber = QUERYITEMCOUNTHWND( hwndDocsLB );

    // reduce TAInput structure
    fOK = UtlAlloc( (PVOID *)&pTAInput, lOldTAInputSize, (LONG)sizeof( TAINPUT ),
                    ERROR_STORAGE);
    if ( fOK )
    {
      *ppTAInput = pTAInput;          // set caller's ptr to TAINPUT structure
      if ( pTAInput->ppvCallersTAInput ) *(pTAInput->ppvCallersTAInput) = pTAInput;
    } /* endif */


    // re-alloc to required size
    if ( fOK )
    {
      LONG lBufferSize = ( (LONG)(pTAInput->stSourcefiles.usNumber +
                                  pTAInput->stInputMemDb.usNumber +
                                  pTAInput->stInputDict.usNumber +
                                  pTAInput->stMorphDict.usNumber) *
                            (LONG)MAX_EQF_PATH );
      fOK = UtlAlloc( (PVOID *)&pTAInput, (LONG)sizeof( TAINPUT ),
                      (LONG)sizeof(TAINPUT) + lBufferSize, ERROR_STORAGE);
      if ( fOK )
      {
        *ppTAInput = pTAInput;          // set caller's ptr to TAINPUT structure
        if ( pTAInput->ppvCallersTAInput ) *(pTAInput->ppvCallersTAInput) = pTAInput;
      } /* endif */
    } /* endif */
  } /* endif */

  // allocate new pInD area
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pTAInput->pInD, 0L, sizeof(TAINSTDATA), ERROR_STORAGE );
    if ( fOK)
    {
      pTAInput->pInD->fUseLatestTMMatch    = pTAInput->fUseLatestTMMatch;
      pTAInput->pInD->fExactContextTMMatch = pTAInput->fExactContextTMMatch;
    } /* endif */
  } /* endif */

  // fill general infos in TAINPUT structure
  if ( fOK )
  {
     PSZ pszFolder;
     CHAR chDrive;

     // clear processing flags set by previous analysis process
     pTAInput->fNoComplete  = FALSE;
     pTAInput->usLangCode   = 0;
     pTAInput->fNTLMwterm   = FALSE;
     pTAInput->fNTLcontext  = FALSE;
     pTAInput->fFTLcontext  = FALSE;
     pTAInput->fKill        = FALSE;
     pTAInput->fNTLLocked   = FALSE;
     pTAInput->fFTLLocked   = FALSE;
     pTAInput->fExcLocked   = FALSE;
     pTAInput->fExcDicLocked = FALSE;
     pTAInput->fEndTMDone = FALSE;
     pTAInput->pWSList = NULL;

     // folder object and property name
     {
       CHAR szPrimaryDrive[4];

       strcpy( pTAInput->szFolder, pszNewFolObjName );
       strcpy( pTAInput->szFolderProp, pszNewFolObjName );
       UtlQueryString( QST_PRIMARYDRIVE, szPrimaryDrive, sizeof(szPrimaryDrive ) );
       pTAInput->szFolderProp[0] = szPrimaryDrive[0];
     }

     // path names
     pszFolder =  UtlGetFnameFromPath( pTAInput->szFolder );
     chDrive = pTAInput->szFolder[0];
     UtlMakeEQFPath( pTAInput->szSOURCE_Path, chDrive, DIRSOURCEDOC_PATH, pszFolder);
     UtlMakeEQFPath( pTAInput->szSEGSOURCE_Path, chDrive, DIRSEGSOURCEDOC_PATH, pszFolder);
     UtlMakeEQFPath( pTAInput->szSEGTARGET_Path, chDrive, DIRSEGTARGETDOC_PATH, pszFolder);
     UtlMakeEQFPath( pTAInput->szSEGNEWMATCH_Path, chDrive, DIRSEGNOMATCH_PATH, pszFolder);
     UtlMakeEQFPath( pTAInput->szMACHTRANS_Path, chDrive, DIRSEGMT_PATH, pszFolder);

     // retrieve values from folder properties
     if ( fOK )
     {
        PPROPFOLDER  ppropFolder;        // pointer to folder properties
        HPROP        hpropFolder;        // folder properties handle
        ULONG        ulErrorInfo;        // error indicator from property handler

        if( ( hpropFolder = OpenProperties( pTAInput->szFolderProp, NULL,
                                             PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
        {
           // Error accessing folder properties
           if ( ulErrorInfo != Err_NoStorage )
           {
              PSZ pError = pszNewFolObjName;
              UtlError( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1,
                        &pError, EQF_ERROR );
           } /* endif */
           fOK = FALSE;
        }
        else
        {
          // Access to folder properties successful, now get some
          // data from folder properties
          ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( hpropFolder );
          pTAInput->sMatchLevel = ppropFolder->usMatchLevel;

          UtlMakeEQFPath( pTAInput->szTagTableName, NULC, TABLE_PATH, NULL);
          strcat( pTAInput->szTagTableName, BACKSLASH_STR );
          strcat( pTAInput->szTagTableName, ppropFolder->szFormat );
          strcat( pTAInput->szTagTableName, EXT_OF_FORMAT );

          if ( pTAInput->fInsertToTM || pTAInput->fInsertTMMatches ||
               pTAInput->fInsertNewMatch || pTAInput->fMachTrans )
          {
            if ( ppropFolder->szLongMemory[0] != EOS )
            {
              strcpy( pTAInput->szOutputMemDb, ppropFolder->szLongMemory );
            }
            else
            {
              strcpy( pTAInput->szOutputMemDb, ppropFolder->szMemory );
            } /* endif */
          } /* endif */

          // update process window titlebar text
          if ( pTAInput->hwndProcWin )
          {
            if ( ppropFolder->szLongName[0] )
            {
              strcpy( pTAInput->szLongBuffer, ppropFolder->szLongName );
              OEMTOANSI( pTAInput->szLongBuffer );
            }
            else
            {
              Utlstrccpy( pTAInput->szLongBuffer,
                          UtlGetFnameFromPath( pTAInput->szFolder ), DOT );
            } /* endif */
            SetWindowText( pTAInput->hwndProcWin, pTAInput->szLongBuffer );
          } /* endif */

          CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
        } /* endif */
     } /* endif */
  } /* endif */

  // fill-in document names, TM names and dictionary names
  if ( fOK )
  {
     SHORT         sIndex;               // index of files to be analyzed
     BOOL          usResponse;           // return from UtlError
     HPROP         hPropDocument;        // handle to document properties
     PPROPDOCUMENT pPropDocument;        // pointer to documnet properties
     ULONG         ulErrorInfo;          // error indicator from PRHA
     SHORT         sOrgFiles;            // original number of analysis files

     // set actual pointer to analysis IF data buffer
     pszAct = (PSZ)(pTAInput) + sizeof(TAINPUT);
     pszStart = pszAct;


     sOrgFiles = pTAInput->stSourcefiles.usNumber;

     if ( fOK )
     {
       // allocate pointer array for long names
       LONG lSize = (LONG)sizeof(PSZ) * (LONG)sOrgFiles;
       if ( lSize < MIN_ALLOC) lSize = MIN_ALLOC;
       fOK = UtlAlloc( (PVOID *)&(pTAInput->apszLongNames), 0L, lSize,
                       ERROR_STORAGE );
     } /* endif */

     if ( fOK )
     {
       // allocate pointer array for alias names
       LONG lSize = (LONG)sizeof(PSZ) * (LONG)sOrgFiles;
       if ( lSize < MIN_ALLOC) lSize = MIN_ALLOC;
       fOK = UtlAlloc( (PVOID *)&(pTAInput->apszAlias), 0L, lSize,
                       ERROR_STORAGE );
     } /* endif */

     // add document names
     if ( fOK )
     {
       SHORT sFileNum;                   // number of file in file table

       sIndex = 0;
       sFileNum = 0;
       while ( sIndex < (SHORT)pTAInput->stSourcefiles.usNumber && fOK)
       {
          QUERYITEMTEXTHWND( hwndDocsLB, sIndex, pTAInput->szLongBuffer );
          strcpy( pszAct, pTAInput->szLongBuffer );

          // open document properties
          hPropDocument = OpenProperties( pTAInput->szLongBuffer,
                                          pTAInput->szFolder,
                                          PROP_ACCESS_READ, &ulErrorInfo );
          if( hPropDocument )
          {
            pPropDocument =(PPROPDOCUMENT) MakePropPtrFromHnd( hPropDocument );

            // store long file name in string buffer
            if ( pPropDocument->szLongName[0] != EOS )
            {
              strcpy( pTAInput->szLongBuffer, pPropDocument->szLongName );
              OEMTOANSI( pTAInput->szLongBuffer );
            } /* endif */

            //--- check if file is currently in use
            sprintf( pTAInput->szTempName, "%s\\%s", pTAInput->szFolder, pszAct );

            if ( EqfQueryObject( pTAInput->szTempName, clsANY, 0 ) )
            {
               PSZ pszDocName = pTAInput->szLongBuffer;
               usResponse = UtlError( ERROR_DOC_INUSE,
                                      MB_YESNO, 1, &pszDocName,
                                      EQF_QUERY );
               fOK = (usResponse != MBID_NO);
            }
            else
            {
              usResponse = MBID_YES;
              if (pPropDocument->ulTouched )  // document has already been touched
              {
                if ( !pTAInput->fYesToAll )
                {
                  PSZ pszDocName = pTAInput->szLongBuffer;

                  usResponse = UtlError( ERROR_DOC_TOUCHED,
                                         MB_EQF_YESTOALL | MB_DEFBUTTON3,
                                         1, &pszDocName, EQF_QUERY );
                  if ( usResponse == MBID_EQF_YESTOALL )
                  {
                    pTAInput->fYesToAll = TRUE;
                    usResponse = MBID_YES;
                  } /* endif */
                } /* endif */
              } /* endif */

              switch (usResponse)
              {
                 case MBID_YES:
                    pszAct += strlen( pszAct ) + 1;
                    pTAInput->apszLongNames[sFileNum] =
                       PoolAddString( pTAInput->pPool, pTAInput->szLongBuffer );
                    if ( pPropDocument->szAlias[0] != EOS )
                    {
                      pTAInput->apszAlias[sFileNum] =
                         PoolAddString( pTAInput->pPool,
                                        pPropDocument->szAlias );
                    } /* endif */
                    sFileNum++;
                    break;

                 case MBID_NO:           // ignore the file
                    DELETEITEMHWND( hwndDocsLB, sIndex );
                    sIndex--;
                    pTAInput->stSourcefiles.usNumber--;
                    break;

                  case MBID_CANCEL:       // cancel text analysis
                    fOK = FALSE;
                    break;
              } /* endswitch */
            } /* endif */
            CloseProperties( hPropDocument, PROP_QUIT, &ulErrorInfo);
          }
          else
          {
            if ( ulErrorInfo != Err_NoStorage )
            {
               usResponse = UtlError( ERROR_OPEN_PROPERTIES,
                                      MB_CANCEL, 1, &pszAct,
                                      EQF_ERROR );
            } /* endif */
            fOK = FALSE;
          } /* endif */
          sIndex++;
       } /* endwhile */
       pTAInput->stSourcefiles.ulOffset = pszStart - (PSZ)(pTAInput);
       pTAInput->stSourcefiles.ulLength = pszAct - pszStart;
     } /* endif */

     // save selected dictionaries nd TMs to analysis interface
     if ( fOK )
     {
       pszStart = pszAct;
       for ( sIndex = 0; sIndex < (SHORT)pTAInput->stInputDict.usNumber; sIndex++)
       {
         strcpy( pszAct, pszDict[sIndex] );
         pszAct += strlen(pszAct) + 1;
       } /* endfor */
       pTAInput->stInputDict.ulOffset = pszStart - (PSZ)(pTAInput);
       pTAInput->stInputDict.ulLength = pszAct - pszStart;

       pszStart = pszAct;
       for ( sIndex = 0; sIndex < (SHORT)pTAInput->stInputMemDb.usNumber; sIndex++ )
       {
         strcpy( pszAct, pszTM[sIndex] );
         pszAct += strlen( pszAct ) + 1;
       } /* endfor */
       pTAInput->stInputMemDb.ulOffset = pszStart - (PSZ)(pTAInput);
       pTAInput->stInputMemDb.ulLength = pszAct - pszStart;
     } /* endif */

     // save morphdict to analysis interface
     if ( fOK )
     {
       pszStart = pszAct;
       if ( pszMorphDict )
       {
         strcpy( pszAct, pszMorphDict );
         pTAInput->stMorphDict.ulLength = strlen( pszStart );
         pTAInput->stMorphDict.ulOffset = pszStart - (PSZ)( pTAInput);
       }
       else
       {
         pTAInput->stInputMemDb.ulOffset = 0;
         pTAInput->stInputMemDb.ulLength = 0;
       } /* endif */
     } /* endif */


  } /* endif */

  // fill process window document listbox
  if ( fOK )
  {
    USHORT usCount;
    DELETEALLHWND( pTAInput->hwndToDo );
    DELETEALLHWND( pTAInput->hwndDone );

    for (usCount = 0; usCount < pTAInput->stSourcefiles.usNumber; usCount++)
    {
      INSERTITEMHWND( pTAInput->hwndToDo, pTAInput->apszLongNames[usCount] );
    } /* endfor */
  } /* endif */

  // cleanup
  if ( hwndDocsLB ) WinDestroyWindow( hwndDocsLB );

  return( fOK );
} /* end of function TARestartAnalysisForNewFolder */



// ensure that target file has the same EOF sequence as source file
BOOL TARestoreOrgEOF
(
   PSZ pszSourceFile,                  // fully qualified source file name
   PSZ pszTargetFile,                  // fully qualified target file name
   PSZ pszSourceLang,                  // document source language
   PSZ pszTargetLang                   // document target language
)
{

  BOOL fOK = TRUE;
  HANDLE hSourceFile = INVALID_HANDLE_VALUE; // handle of source file
  HANDLE hTargetFile = INVALID_HANDLE_VALUE; // handle of target file
  SHORT  sEndSeqLength = 0;            // length of end sequence
  CHAR   szEndSeqSource[21];           // end sequence of source file
  CHAR   szEndSeqTarget[21];           // end sequence of target file
  BOOL   fUnicode = FALSE;             // TRUE = file uses Unicode encoding
  ULONG  ulSourceCP;                   // code page of source document
  ULONG  ulTargetCP;                   // code page of target document

  // get codepage for source and target code page
  ulSourceCP = GetLangOEMCP( pszSourceLang );
  ulTargetCP = GetLangOEMCP( pszTargetLang );

  // open source file
  hSourceFile = CreateFile( pszSourceFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  fOK = ( hSourceFile != INVALID_HANDLE_VALUE );

  // get end sequence of source file
  if ( fOK )
  {
    fOK = TAGetFileEndSequence( hSourceFile, szEndSeqSource, &fUnicode, &sEndSeqLength, ulSourceCP );
  } /* endif */

  // open target file
  if ( fOK )
  {
    hTargetFile = CreateFile( pszTargetFile, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    fOK = ( hTargetFile != INVALID_HANDLE_VALUE );
  } /* endif */

  // get end sequence of target file
  if ( fOK )
  {
    fOK = TAGetFileEndSequence( hTargetFile, szEndSeqTarget, &fUnicode, &sEndSeqLength, ulTargetCP );
  } /* endif */


  // truncate target file at begin of end sequence and add end sequence
  // of source file
  if ( fOK )
  {
    char chNewEndSeq[40];        // buffer for new end sequence
    PSZ  pszNewEndSeq = chNewEndSeq; // ptr for end sequence processing
    PSZ_W  pszNewEndSeqW = (CHAR_W *)chNewEndSeq; // ptr for end sequence processing
    int  iNewLen = 0;
    DWORD dwWritten = 0;

    // build new end sequence
    {
      int iLen = strlen(szEndSeqSource);
      while ( iLen )
      {
        iLen--;
        if ( fUnicode )
        {
          switch( szEndSeqSource[iLen] )
          {
            case LF:      *pszNewEndSeqW++ = LF_W;  iNewLen += 2; break;
            case CR:      *pszNewEndSeqW++ = CR_W;  iNewLen += 2; break;
            case EOFCHAR: *pszNewEndSeqW++ = EOF_W; iNewLen += 2; break;
          } /* endswitch */
        }
        else
        {
          switch( szEndSeqSource[iLen] )
          {
            case LF:      *pszNewEndSeq++ = LF;      iNewLen += 1; break;
            case CR:      *pszNewEndSeq++ = CR;      iNewLen += 1; break;
            case EOFCHAR: *pszNewEndSeq++ = EOFCHAR; iNewLen += 1; break;
          } /* endswitch */
        } /* endif */
      } /* endwhile */
    }

    // truncate target file
    {
      SetFilePointer( hTargetFile, 0 - sEndSeqLength, NULL, FILE_END );
      SetEndOfFile( hTargetFile );
    }

    // add new end sequence to file
    if ( iNewLen )
    {
      WriteFile( hTargetFile, chNewEndSeq, iNewLen, &dwWritten, NULL );
    } /* endif */
  } /* endif */

  // cleanup
  if ( hSourceFile != INVALID_HANDLE_VALUE) CloseHandle( hSourceFile );
  if ( hTargetFile != INVALID_HANDLE_VALUE) CloseHandle( hTargetFile );

  return( fOK );
} /* end of function TARestoreOrgEOF */

// extract file end sequence
// we are only interested in the trailing EOF character and the last linebreak
// (0D0A or 0D or 0A)
BOOL TAGetFileEndSequence
(
  HANDLE hFile,                        // file handle
  PSZ    pszEndSequence,               // callers end sequence buffer
  PBOOL  pfUnicode,                    // callers is-unicode-flag
  PSHORT psEndLength,                  // length of end sequence in bytes
  ULONG  ulCP                          // code page of document
)
{
  BYTE  bBuffer[20];                   // buffer for end of file
  DWORD dwRead = 0;
  BOOL  fOK = TRUE;

  ulCP;

  *pfUnicode = FALSE;
  *psEndLength = 0;

  // get last 20 bytes of file
  if ( fOK )
  {
    // position to last 20 bytes of file
    SetFilePointer( hFile, -20, NULL, FILE_END );

    // read file end into buffer
    memset( bBuffer, 0, sizeof(bBuffer) );
    ReadFile( hFile, bBuffer, 20, &dwRead, NULL );
  } /* endif */

  // get type of file end characters
  if ( fOK )
  {
    CHAR_W chTempW;

    PSZ_W pszBufferW = (CHAR_W *)bBuffer;
    DWORD dwReadW = dwRead / 2;
    if ( dwReadW )
    {
      chTempW = pszBufferW[dwReadW-1];
      if ( (chTempW == LF_W) || (chTempW == CR_W) || (chTempW == EOF_W) )
      {
        // assume Unicode end sequence
        *pfUnicode = TRUE;
      } /* endif */
    } /* endif */

    if ( !*pfUnicode )
    {
      CHAR chTemp;

      // handling for non-unicode end sequence
      // leave last character in buffer to avoid splitting of CRLF sequences
      while ( dwRead >= 1 )
      {
        dwRead--;
        chTemp = bBuffer[dwRead];
        if ( chTemp == LF )
        {
          *psEndLength += 1;
          *pszEndSequence++ = LF;

          // check if CR is preceeding the linefeed
          if ( dwRead && (bBuffer[dwRead-1] == CR) )
      {
            dwRead--;
            *psEndLength += 1;
            *pszEndSequence++ = CR;
      } /* endif */
        }
        else if ( chTemp == CR )
      {
          *psEndLength += 1;
          *pszEndSequence++ = CR;
        }
        else if ( chTemp == EOF )
        {
          *psEndLength += 1;
          *pszEndSequence++ = EOFCHAR;
        }
        else
          {
          break; // no more EOF characters to follow
          } /* endif */
        } /* endwhile */
      *pszEndSequence = EOS;
        }
        else
        {
      // handling for unicode end sequence

      // leave last character in buffer to avoid splitting of CRLF sequences
      while ( dwReadW >= 1 )
      {
        dwReadW--;
        chTempW = pszBufferW[dwReadW];
        if ( chTempW == LF_W )
        {
          *psEndLength += 1;
          if ( *pfUnicode ) *psEndLength += 1;
          *pszEndSequence++ = LF;

          // check if CR is preceeding the linefeed
          if ( dwReadW && (pszBufferW[dwReadW-1] == CR_W) )
          {
            dwReadW--;
            *psEndLength += 1;
            if ( *pfUnicode ) *psEndLength += 1;
            *pszEndSequence++ = CR;
          } /* endif */
        }
        else if ( chTempW == CR_W )
        {
          *psEndLength += 1;
          if ( *pfUnicode ) *psEndLength += 1;
          *pszEndSequence++ = CR;
        }
        else if ( chTempW == EOF_W )
        {
          *psEndLength += 1;
          if ( *pfUnicode ) *psEndLength += 1;
          *pszEndSequence++ = EOFCHAR;
        }
        else
        {
          break; // no more EOF characters to follow
        } /* endif */
      } /* endwhile */
      *pszEndSequence = EOS;
    } /* endif */
  } /* endif */
  return( fOK );
} /* end of function TAGetFileEndSequence */

// Function which checks if the given segmented file is in UTF16 format
BOOL TAIsUTF16SegFile
(
  PSZ    pszSegFile                    // fully qualified segmented file
)
{
  BOOL   fIsUTF16 = FALSE;             // our UTF16 flag
  USHORT usRC = 0;                     // return code of called functions
  HFILE  hfSegFile = NULL;             // file handle
  USHORT usOpenAction;                 // action performed by UtlOpen
  PSZ    pszPrefix = UNICODEFILEPREFIX;

  usRC = UtlOpen( pszSegFile, &hfSegFile, &usOpenAction, 0L, FILE_NORMAL, FILE_OPEN,
                         OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, 0L, FALSE );

  if ( !usRC )
  {
    CHAR szBuffer[16];
    ULONG ulBytesRead = 0;
    int iLen = strlen(pszPrefix);

    UtlReadL( hfSegFile, szBuffer, 8, &ulBytesRead, FALSE );

    // check for UTF16 prefix or UTF16 characters at start of file
    if ( memcmp( szBuffer, pszPrefix, iLen ) == 0 )
    {
      // file starts with UTF16 BOM
      fIsUTF16 = TRUE;
    }
    else if ( (szBuffer[0] == ':') && (szBuffer[1] == EOS) )
    {
      // file starts with UTF16 character
      fIsUTF16 = TRUE;
    }
    else
    {
      // seems to be a non-UTF16 file
      fIsUTF16 = FALSE;
    } /* endif */

    // close the file
    UtlClose( hfSegFile, FALSE );
  } /* endif */

  return( fIsUTF16 );
} /* end of function TAIsUTF16SegFile */

#ifdef MEASUREANALYSISTIME
void TAWriteTimeLog( PTAINPUT pTAInput )
{
  CHAR szLogFile[MAX_EQF_PATH];
  FILE *hLog = NULL;

  UtlMakeEQFPath( szLogFile, NULC, SYSTEM_PATH, NULL );
  strcat( szLogFile, "\\LOGS" );
  UtlMkDir( szLogFile, 0L, FALSE );
  strcat( szLogFile, "\\ANATIME.LOG" );
  hLog = fopen( szLogFile, "a" );
  if ( hLog )
  {
    LONG lCurTime = 0;

    time( &lCurTime );
    fprintf( hLog, "Analysis ended at : %s", asctime( localtime( &lCurTime ) ) );
    if ( pTAInput->lStartTime )
    {
      LONG lDiff = lCurTime - pTAInput->lStartTime;
      fprintf( hLog, "Overall process time is %ld:%2.2ld:%2.2ld\n", lDiff / 3600,
               (lDiff - (lDiff / 3600 * 3600)) / 60,
               (lDiff - (lDiff / 3600 * 3600)) % 60 );
    }
    fprintf( hLog, "Other time             : %lu\n", pTAInput->ulTimeOther );
    fprintf( hLog, "GetExactTMMatch start  : %lu\n", pTAInput->ulGetExactTMMatchstart  );
    fprintf( hLog, "GetExactTMMatch TmGetW : %lu\n", pTAInput->ulTMGetW );
    fprintf( hLog, "GetExactTMMatch Repl   : %lu\n", pTAInput->ulReplMatch );
    fprintf( hLog, "GetExactTMM. CopyBest  : %lu\n", pTAInput->ulCopyBest );
    fprintf( hLog, "Memory lookup end      : %lu\n", pTAInput->ulTimeGetMatch );
    fprintf( hLog, "File read time         : %lu\n", pTAInput->ulTimeFileRead );
    fprintf( hLog, "list processing time   : %lu\n", pTAInput->ulTimeListProcessing );
    fprintf( hLog, "word counting time     : %lu\n", pTAInput->ulTimeWordCount );
    fprintf( hLog, "redundancy time        : %lu\n", pTAInput->ulTimeRedundCount );
    fprintf( hLog, "Machine match time     : %lu\n", pTAInput->ulTimeMachMatch );
    fprintf( hLog, "SNOMATCH time          : %lu\n", pTAInput->ulTimeSNOMATCH );

    fclose( hLog );
  } /* endif */
}
#endif


#ifdef TSEGM_SEGLOGGING

void LogSegData( FILE *hf, PSZ_W pszString )
{
  static CHAR_W szBuffer[1024];
  PSZ_W pszTemp = szBuffer;

  while ( *pszString)
  {
    if ( *pszString == L'\r' )
    {
      wcscpy( pszTemp, L"<cr>" );
      pszTemp += 4;
      pszString++;
    }
    else if ( *pszString == L'\n' )
    {
      wcscpy( pszTemp, L"<lf>" );
      pszTemp += 4;
      pszString++;
    }
    else
    {
      *pszTemp++ = *pszString++;
    } /* endif */

    if ( (pszTemp - szBuffer) > 60 )
    {
      *pszTemp = 0;
      fprintf( hf, "%s\n", szBuffer );
      pszTemp = szBuffer;
    } /* endif */
  } /*endwhile */;

  if ( pszTemp != szBuffer )
  {
    *pszTemp = 0;
    fwprintf( hf, L"%s\n", szBuffer );
  } /* endif */
}
#endif


BOOL isXMPMarkup( PSZ pszMarkup, PXMPTAG *ppStartTags, PXMPTAG *ppEndTags )
{
  int i = 0;
  while ( (XmpMarkups[i].szMarkup[0] != EOS) && (strcmp( XmpMarkups[i].szMarkup, pszMarkup ) != 0)  )
  {
    i++;
  } /*endwhile */
  
  *ppStartTags = XmpMarkups[i].pStartTags;
  *ppEndTags = XmpMarkups[i].pEndTags;

  return( XmpMarkups[i].szMarkup[0] != EOS );
} /* end of isXMPMarkup */

BOOL isTagNameEndChar( CHAR_W chTest )
{
  return ( iswspace(chTest) || (chTest == L'>') || (chTest == L'-') || (chTest == L'<') || (chTest == 0) || (chTest == L'/') );
}

BOOL isXMPTag( PSZ_W pszTag, USHORT usLen, PXMPTAG pTagList, int *piFoundLen )
{
  CHAR_W      chTemp;
  PSZ_W       pszTagToTest;
  BOOL        fFound = FALSE;
  int iTagLen = 0;

  chTemp = pszTag[usLen];
  pszTag[usLen] = EOS;

  // skip any "<twb--" prefix of tag
  if ( wcsnicmp( pszTag, L"<twb--", 6) == 0 )
  {
    pszTagToTest = pszTag + 6;
    iTagLen += 6;
  }
  else
  {
    pszTagToTest = pszTag;
  } /* endif */     


  while ( (pTagList->iLen != 0) && (wcsnicmp( pTagList->szTag, pszTagToTest, pTagList->iLen ) != 0)  )
  {
    pTagList++;
  } /*endwhile */

  // check if tag name really ends at end of the found tag from our list (e.g. don't treat <metadata as start tag when looking for <meta tag)
  if ( pTagList->iLen != 0 )
  {
    if ( isTagNameEndChar( pszTagToTest[pTagList->iLen] ) )
    {
      fFound = TRUE;
    } /* endif */       
    else if ( isTagNameEndChar( pszTagToTest[pTagList->iLen+1] ) )
    {
      fFound = TRUE;
    } /* endif */       
  } /* endif */     

  if ( fFound && (piFoundLen != NULL) )
  {
    *piFoundLen = iTagLen + pTagList->iLen;
  } /* endif */

  pszTag[usLen] = chTemp;

  return( fFound );

} /* end of isXMPTag */


BOOL isSelfContainedTagEnd( PTAINSTDATA pInD, PSZ_W pszTag, USHORT usTagLen, PSZ pszMarkup )
{
  BOOL        fTagEnd = FALSE;
  PSZ_W pszTagToTest;

  CHAR_W chTemp = pszTag[usTagLen];
  pszTag[usTagLen] = EOS;

  // skip any "<twb--" prefix of tag
  if ( wcsnicmp( pszTag, L"<twb--", 6) == 0 )
  {
    pszTagToTest = pszTag + 6;
  }
  else
  {
    pszTagToTest = pszTag;
  } /* endif */     

  if ( pszTagToTest[0] == L'"' ) pszTagToTest++;
  if ( (pszTagToTest[0] == L'/') && (pszTagToTest[1] == L'>') )
  {
    fTagEnd = TRUE;
  } /* endif */

  pszTag[usTagLen] = chTemp;

  return( fTagEnd );
} /* end of isSelfContainedTagEnd */


BOOL isXMPScreenStartTag( PTAINSTDATA pInD, PSZ_W pszTag, USHORT usTagLen, PSZ pszMarkup, PBOOL pfIsSelfContainedProtectTag  )
{
  BOOL        fXmpStartTag = FALSE;
  int iTagLen = 0;

  if ( pfIsSelfContainedProtectTag != NULL ) *pfIsSelfContainedProtectTag = FALSE;

  fXmpStartTag = isXMPTag( pszTag, usTagLen, pInD->aActiveStartTags, &iTagLen );

  if ( fXmpStartTag )
  {
    // check for self contained tags; i.e. tags ending with /> and having no closing tag
    static CHAR_W szBuffer[MAX_SEGMENT_SIZE+1];
    PSZ_W pszTest = szBuffer;
    PSZ_W pszTWBStart = 0 ;
    PSZ_W pszTWBEnd = 0 ;

    // make a copy of the data and remove any <twb-- and --twb> tags
    {
      PSZ_W pszSource = pszTag;
      PSZ_W pszTarget = szBuffer;
      int iLen = 0;
      while ( (iLen < MAX_SEGMENT_SIZE) && *pszSource )
      {
        if ( *pszSource == L'<' )
        {
          if ( wcsnicmp( pszSource, L"<twb--", 6 ) == 0 )
          {
            pszSource += 6;
            if ( ! pszTWBStart ) pszTWBStart = pszTarget ;
          }
          else
          {
            *pszTarget++ = *pszSource++;
            iLen++;
          } /* endif */             
        }
        else if ( *pszSource == L'-' )
        {
          if ( wcsnicmp( pszSource, L"--twb>", 6 ) == 0 )
          {
            pszSource += 6;
            if ( pszTWBStart && !pszTWBEnd ) pszTWBEnd = pszTarget ;
          }
          else
          {
            *pszTarget++ = *pszSource++;
            iLen++;
          } /* endif */             
        }
        else
        {
          *pszTarget++ = *pszSource++;
          iLen++;
        } /* endif */           
      } /* endwhile */         
      *pszTarget = 0;
    }

    // skip tag
    if ( *pszTest ) pszTest++;
    while ( *pszTest && iswalpha(*pszTest) )
    {
      pszTest++;
    } /* endwhile */       

    // skip following whitespace
    while ( *pszTest && iswspace(*pszTest) ) pszTest++;


    // skip any attributes of the tag
    while ( *pszTest && iswalpha(*pszTest) )
    {
      // skip attribue name
      while ( *pszTest && 
              (iswalnum(*pszTest) || (*pszTest == L'-') || (*pszTest == L'_')) )
      {
        pszTest++;
      } /* endwhile */       
    
      // skip any attrubute value
      if ( *pszTest == L'=' )
      {
        // skip equal sign
        pszTest++;

        // skip data
        if ( *pszTest == L'\"')
        {
          // skip values enclosed in double-quotes
          do
          {
            pszTest++;

            // skip doubled double-quotes
            if ( (pszTest[0] == L'\"') && (pszTest[1] == L'\"') )
            {
              pszTest += 2;
            } /* endif */               
          } while ( *pszTest && (*pszTest != L'\"') );
          pszTest++;
        }
        else if ( *pszTest == L'\'')
        {
          // skip values enclosed in single-quotes
          do
          {
            pszTest++;

            // skip doubled single-quotes
            if ( (pszTest[0] == L'\'') && (pszTest[1] == L'\'') )
            {
              pszTest += 2;
            } /* endif */               
          } while ( *pszTest && (*pszTest != L'\'') );
          pszTest++;
        }
        else
        {
          // skip any alphanumeric data 
          while ( *pszTest && 
                  (iswalnum(*pszTest) || (*pszTest == L'-') || (*pszTest == L'_')) )
          {
            pszTest++;
          } /* endwhile */       
        } /* endif */
      } /* endif */         

      // skip following whitespace
      while ( *pszTest && iswspace(*pszTest) ) pszTest++;

    } /* endwhile */       

    // skip following whitespace
    while (  *pszTest && iswspace(*pszTest) ) pszTest++; 

    // reset fXmpStartTag when tag end indicates a self contained tag
    if ( (pszTest[0] == L'/') && (pszTest[1] == L'>') )
    {
      fXmpStartTag = FALSE;
      if ( pfIsSelfContainedProtectTag != NULL ) *pfIsSelfContainedProtectTag = TRUE;
    } 
    else if ( pszTWBStart && pszTWBStart<pszTest && pszTest<pszTWBEnd ) 
    { 
      /* 10-20-14 */
      for( ; pszTest<pszTWBEnd && *pszTest!=L'<' ; ++pszTest ) ;
      if ( ( *pszTest == L'<' ) && ( isXMPScreenEndTag( pInD, pszTest, (USHORT)(pszTWBEnd-pszTest), pszMarkup ) ) ) 
      {
        fXmpStartTag = FALSE;
      }
    } /* endif */
                 
  } /* endif */

  return( fXmpStartTag );
} /* end of isXMPScreenStartTag */

BOOL isXMPScreenEndTag( PTAINSTDATA pInD, PSZ_W pszTag, USHORT usTagLen, PSZ pszMarkup )
{
  BOOL        fXmpEndTag = FALSE;
  int iTagLen = 0;


  pszMarkup;
  fXmpEndTag = isXMPTag( pszTag, usTagLen, pInD->aActiveEndTags, &iTagLen );

  if ( fXmpEndTag )
  {
    PSZ_W pszTest = pszTag + iTagLen;

    // find tag end character skipping any whitespace and twb tags
    CHAR_W chEndChar = 0;
    while ( *pszTest && (chEndChar == 0) )
    {
      CHAR_W chCurChar = *pszTest;
      if ( (chCurChar == L'<' ) && (wcsnicmp( pszTest, L"<twb--", 6 ) == 0) )
      {
        pszTest += 6;
      }
      else if ( (chCurChar == L'-') && (wcsnicmp( pszTest, L"--twb>", 6 ) == 0) )
      {
        pszTest += 6;
      }
      else if ( iswspace( chCurChar ) )
      {
        pszTest++;
      }
      else
      {
        chEndChar = chCurChar;
      } /* endif */
    } /* endwhile */

    if ( chEndChar != L'>' )
    {
      fXmpEndTag = FALSE;
    } /* endif */
  } /* endif */

  return( fXmpEndTag );
} /* end of isXMPScreenStartTag */

/** Prepare the list of start and stop tags based on user selection and current markup table name 
 \param pInD pointer to analysis instance data area
 \param pszMarkup pointer to name of current markup table
*/
BOOL PrepareTags( PTAINSTDATA pInD, PSZ pszMarkup )
{
//BOOL fReturn = TRUE;
  PXMPTAG pStartTags = NULL;
  PXMPTAG pEndTags = NULL;

  pInD->iNumOfActiveStartTags = 0;
  pInD->iNumOfActiveEndTags = 0;
  memset ( pInD->aActiveStartTags, 0, sizeof(pInD->aActiveStartTags) );
  memset ( pInD->aActiveEndTags, 0, sizeof(pInD->aActiveEndTags) );

  // get start list of start and stop tags for current markup table
  if ( isXMPMarkup( pszMarkup, &pStartTags, &pEndTags) )
  {
    int i = 0;
    while( pStartTags[i].szTag[0] != EOS )
    {
      if ( (pInD->fProtXmp       && (pStartTags[i].Id == XMPTAGID)) ||
           (pInD->fProtScreen    && (pStartTags[i].Id == SCREENTAGID)) ||
           (pInD->fProtMsgNum    && (pStartTags[i].Id == MSGNUMTAGID)) ||
           (pInD->fProtCodeBlock && (pStartTags[i].Id == CODEBLOCKTAGID)) ||
           (pInD->fProtMeta      && (pStartTags[i].Id == METATAGID)) )
      { 
        memcpy( pInD->aActiveStartTags + pInD->iNumOfActiveStartTags, pStartTags + i, sizeof(XMPTAG) );
        pInD->iNumOfActiveStartTags++;
      }
      i++;
    }

    i = 0;
    while( pEndTags[i].szTag[0] != EOS )
    {
      if ( (pInD->fProtXmp       && (pEndTags[i].Id == XMPTAGID)) ||
           (pInD->fProtScreen    && (pEndTags[i].Id == SCREENTAGID)) ||
           (pInD->fProtMsgNum    && (pEndTags[i].Id == MSGNUMTAGID)) ||
           (pInD->fProtCodeBlock && (pEndTags[i].Id == CODEBLOCKTAGID)) ||
           (pInD->fProtMeta      && (pEndTags[i].Id == METATAGID)) )
      { 
        memcpy( pInD->aActiveEndTags + pInD->iNumOfActiveEndTags, pEndTags + i, sizeof(XMPTAG) );
        pInD->iNumOfActiveEndTags++;
      }
      i++;
    }
  }
  return( (pInD->iNumOfActiveStartTags != 0) || (pInD->iNumOfActiveEndTags != 0) );
}
