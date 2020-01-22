//+----------------------------------------------------------------------------+
//| EQFMECLM.C                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_EDITORAPI        // for EQFWORDCOUNTPERSEGW
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#include <eqf.h>                  // General Translation Manager include file

#include <time.h>                 // C library for time functions
#include "EQFDDE.H"               // Batch mode definitions
#define INCL_EQFMEM_DLGIDAS       // include dialog IDA definitions
#include "EQFTMI.H"               // Private header file of Translation Memory
#include "EQFMEM.ID"              // PM IDs for Translation Memory
#include <EQFQDAM.H>              // Low level TM access functions
  #include "OTMFUNC.H"            // function call interface public defines
  #include "eqffunci.h"           // function call interface private defines
#include "EQFHLOG.H"              // for word count category limits

#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "core\memory\MemoryFactory.h"


#ifdef _DEBUG
  //#define CLEANMEMORYTIMELOG
  //#define CLEANMEMORYRCLOG
  #define SESSIONLOG
#endif

#ifdef CLEANMEMORYTIMELOG
  static LONG lImportStartTime = 0;
  static LONG64 lMemAccessTime = 0;
  static LONG64 lFileReadTime = 0;
  static LONG64 lOtherTime = 0;

static void GetElapsedTime( LONG64 *plTime );

#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  MemFuncCleanMemory API
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// data types 
// process phases
typedef enum _CLM_TASKS
{
  CLM_START_TASK,
  CLM_START_DOC_TASK,
  CLM_PROCESS_DOC_TASK,
  CLM_END_DOC_TASK,
  CLM_NEXT_DOC_TASK,
  CLM_START_MEM_TASK,
  CLM_PROCESS_MEM_TASK,
  CLM_END_MEM_TASK,
  CLM_STOP_TASK,
  CLM_COMPLETED_TASK
} CLM_TASKS;

// number of best matches to be kept in match table
#define CLM_MAX_MATCHES  3 

// size of input buffers
#define BUF_SIZE 8096

// best match structure, exists once per segment of the document and contains the reference for the 
// three best matches of this segment
typedef struct _CLM_MATCH
{
  USHORT      usMatchLevel;                      // level of this match
  ULONG       ulPos;                             // position of this match in the output memory
  ULONG       ulLen;                             // length of this match in the output memory
} CLM_MATCH, *PCLM_MATCH;


// segment structure, exists once per segment of the document and contains the reference for the 
// three best matches of this segment
typedef struct _CLM_SEGMENT
{
  CLM_MATCH    aBestMatch[CLM_MAX_MATCHES];     // best matches for this segment
  CLM_MATCH    BestMatchGITR;                   // best exact match for this segment when applying generic inline tag replacement
} CLM_SEGMENT, *PCLM_SEGMENT;

// document structure, exists once per document
typedef struct _CLM_DOCUMENT
{
  CHAR        szDocName[MAX_LONGFILESPEC];       // document name
  ULONG       ulSegments;                        // number of segments
  PCLM_SEGMENT pSegment;                         // pointer to segment table
} CLM_DOCUMENT, *PCLM_DOCUMENT;

typedef struct _MEM_CLEAN_MEMORY_DATA
{
  CHAR        szFolder[MAX_LONGFILESPEC];        // name of input folder
  CHAR        szInMemory[MAX_LONGFILESPEC];      // fully qualified name of external input memory (encoding: UTF-16)
  CHAR        szOutMemory[MAX_LONGFILESPEC];     // name of internal output memory or fully qualified name of external output memory
  CHAR        szTempMemory[MAX_LONGFILESPEC];    // name of tempory memory
  CHAR        szOutMemShortName[MAX_FNAME];      // shrt name of output memory
  OBJNAME     szFolObjName;                      // buffer for folder object name
  LONG        lOptions;                          // options specified for this function
  CLM_TASKS   Task;                              // current task
  CHAR        szFolSourceLang[MAX_LANG_LENGTH];  // document source language
  PSZ         pDocNameBuffer;                    // document name buffer
  USHORT      usDocuments;                       // number of documents
  CHAR        szDocFormat[MAX_FNAME];            // name of document markup table
  CHAR        szDocSourceLang[MAX_LANG_LENGTH];  // document source language
  CHAR        szDocTargetLang[MAX_LANG_LENGTH];  // document target language
  CHAR        szSourceDocName[MAX_EQF_PATH];     // buffer for source document name
  CHAR        szTempMem[MAX_LONGPATH];           // name of temporary memory
  CHAR        szOutMemPath[MAX_LONGPATH];        // fully qualified output TM name
  CHAR        szOutMemPropPath[MAX_LONGPATH];    // fully qualified TM property file
  CHAR        szFolName[MAX_FILESPEC];           // folder short name
  CHAR        szFolLongName[MAX_LONGFILESPEC];   // long folder name
  CHAR        szBuffer[4096];                    // general purpose buffer
  USHORT      usComplete;                        // current completion rate
  ULONG       ulSegments;                        // number of segments added to archive TM
  SHORT       sCurDoc;                           // number of current document (within listbox)
  CHAR        szCurDoc[MAX_FILESPEC];            // name of current document
  //TMX_PUT_IN_W stRepIn;                          // input structure for TmReplace
  //TMX_PUT_OUT_W stRepOut;                        // output structure for TmReplace
  //TMX_GET_IN_W stGetIn;                          // input structure for TmGet
  //TMX_GET_OUT_W stGetOut;                        // output structure for TmGet
  //TMX_PUT_IN_W stDelIn;                          // input structure for delete segment
  //TMX_PUT_OUT_W stDelOut;                        // output structure for delete segment
  OtmMemory   *pMem;                             // TM being worked on
  OtmMemory   *pOutMem;                          // output memory 
  BOOL        fErrorStop;                        // TRUE = process stopped by an error
  BOOL        fExternalMemory;                   // TRUE = write to external memory
  PLOADEDTABLE pLoadedQFTable;                   // ptr to loaded QF TagTable
  PTBDOCUMENT  pDoc;                             // ptr to loaded document
  OBJNAME     szDocObjName;                      // object name of current document
  PSZ         pszDocNames;                       // list of document names
  CHAR        szLongName[MAX_LONGFILESPEC];      // name of current document
  CHAR        szAlias[MAX_LONGFILESPEC];         // alias name of current document
  CHAR        szShortAlias[MAX_FILESPEC];        // short alias name of current document
  ULONG       ulSegNum;                          // number of currently active segment
  ULONG       ulAddSegNum;                       // segment number in additional table
  ULONG       ulActiveTable;                     // active segment table
  CHAR_W      szLine[4096];                      // buffer for input line
  CHAR_W      szTempBuffer[4096];                // general purpose buffer
  CHAR_W      szSegStart[1024];                  // buffer for segment start string
  CHAR_W      szControl[1024];                   // buffer for control string
  CHAR        szControlAscii[1024];              // ASCII version of control string
  CHAR_W      szSource[4096];                    // buffer for segment source
  CHAR_W      szTarget[4096];                    // buffer for segment target
  FILE        *hInFile;                          // handle of input file
  FILE        *hOutFile;                         // handle of output file
  ULONG       ulOEMCP;                           // OEM code page for folder source language
  ULONG       ulSegsCopied;                      // number of copied segments
  ULONG       ulSegsSkipped;                     // number of copied segments
  LONG64      lMemFillTime;                      // time needed to fill document memory
  LONG64      lMemOtherTime;                     // time needed to fill document memory
  LONG64      lMemLookupTime;                    // time used for memory lookup
  LONG64      lMemWriteTime;                     // time used for memory writing external/internal
  LONG64      lMemDelTime;                       // time used for delete memory segment
  LONG64      lMemSortTime;                      // time used for sort and prepare match table
  LONG64      lMemClearTime;                     // time used for memory read/write to remove unused segments 
  PCLM_DOCUMENT pDocTable;                       // document array
  int         iNumOfMatches;                     // number of matches to be kept in best match list (1 to CLM_MAX_MATCHES)
  BOOL        fUnicode;                          // TRUE = input memory is in UTF-16 format
  ULONG       ulMemHeaderSize;                   // size of header part in output memory
  BYTE        bReadBuffer[16000];                // file read/write buffer (must be large enough to contain complete memory segment)
  CHAR_W      chInBufW[BUF_SIZE];                // data buffer for read of Unicode data
  CHAR        chInBuf[BUF_SIZE];                 // data buffer for read of ANSI data
  int         iInBufProcessed;                   // number of processed characters in chInBuf
  int         iInBufRead;                        // number characters read into chInBuf
  HFILE       hfIn;                              // input file
  BOOL        fAnsi;                             // TRUE = input in ASCII format / FALSE = input in ASCII mode
  ULONG       ulInputCP;                         // codepage to use for import when importing non-Unicode memory
  BOOL        fKeepDups;                         // TRUE = keep duplicate exact and fuzzy matches in memory
  CHAR_W      szContext[4096];                   // buffer for segment context
  CHAR_W      szAddInfo[4096];                   // buffer for additional information of the segment 
  std::vector<OtmProposal *> *pvMatchList;       // pointer to a vector receiving found memory proposals
  CHAR_W      szTempSource[4096];                // buffer for preprocessed segment source
} MEM_CLEAN_MEMORY_DATA, *PMEM_CLEAN_MEMORY_DATA;



// prototypes 
USHORT MemFuncPrepCleanMemory( PFCTDATA pData, PSZ pszFolder, PSZ pszInMemory, PSZ pszOutMemory, LONG lOptions );
USHORT MemFuncCleanMemoryProcess( PFCTDATA pData );
USHORT MemFuncCLMNextDoc( PMEM_CLEAN_MEMORY_DATA pCLMData );
USHORT MemFuncCLMDocEnd( PMEM_CLEAN_MEMORY_DATA pCLMData );
USHORT MemFuncCLMDocProcess( PMEM_CLEAN_MEMORY_DATA pCLMData );
USHORT MemFuncCLMDocStart( PMEM_CLEAN_MEMORY_DATA pCLMData );
USHORT MemFuncCLMCleanup( PMEM_CLEAN_MEMORY_DATA pCLMData );
USHORT MemFuncCLMMemStart( PMEM_CLEAN_MEMORY_DATA pCLMData );    
USHORT MemFuncCLMStart( PMEM_CLEAN_MEMORY_DATA pCLMData );
USHORT MemFuncCLMMemProcess( PMEM_CLEAN_MEMORY_DATA pCLMData );
USHORT MemFuncCLMMemEnd( PMEM_CLEAN_MEMORY_DATA pCLMData );
BOOL MemStripTag( PSZ_W pszLine, PSZ_W pszTag );
USHORT MemGetLine( PMEM_CLEAN_MEMORY_DATA pCLMData, PSZ_W pszLine, int iSize, BOOL fUnicode, PBOOL pfEOF  );
static USHORT MemFillBufferW( PMEM_CLEAN_MEMORY_DATA pCLMData );
static USHORT MemFillBuffer( PMEM_CLEAN_MEMORY_DATA pCLMData );
static void   LogRC( USHORT usRC, PSZ pszFunction );

// function removed irrelevant segments from an external memory
USHORT MemFuncCleanMemory
(
  PFCTDATA         pData,              // function I/F session data
  PSZ              pszFolder,          // name of folder containing the translatable material
  PSZ              pszInMemory,        // fully qualified name of external input memory (encoding: UTF-16)
  PSZ              pszOutMemory,       // name of internal output memory or fully qualified name of external output memory
  LONG             lOptions            // options for processing
)
{
  USHORT      usRC = NO_ERROR;         // function return code

#ifdef CLEANMEMORYTIMELOG
  {
    LONG64 lDummyTime = 0;
    GetElapsedTime( &lDummyTime );
  }
#endif

  // prepare a new import or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new clean memory run
    pData->usProgress = 0;
    usRC = MemFuncPrepCleanMemory( pData, pszFolder, pszInMemory, pszOutMemory, lOptions );
    if ( usRC ) LogRC( usRC, "MemFuncPrepCleanMemory" );
  }
  else
  {
    // continue current analysis process
    usRC = MemFuncCleanMemoryProcess( pData );
    if ( usRC ) LogRC( usRC, "MemFuncCleanMemoryProcess" );
  } /* endif */

  if ( usRC ) LogRC( usRC, "EqfCleanMemory" );

  return( usRC );
} /* end of function MemFuncCleanMemory */

// prepare the lean memory function
USHORT MemFuncPrepCleanMemory
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolder,               // name of folder containing the translatable material
  PSZ         pszInMemory,             // fully qualified name of external input memory (encoding: UTF-16)
  PSZ         pszOutMemory,            // name of internal output memory or fully qualified name of external output memory
  LONG        lOptions                 // options for processing
)
{
  BOOL        fOK = TRUE;              // internal O.K. flag
  USHORT      usRC = NO_ERROR;         // function return code
  PSZ         pszParm;                 // error parameter pointer

  PMEM_CLEAN_MEMORY_DATA pCLMData = NULL;   // Pointer to the load IDA

  fOK = UtlAllocHwnd( (PVOID *)&pCLMData, 0L, (LONG)sizeof(MEM_CLEAN_MEMORY_DATA), ERROR_STORAGE, HWND_FUNCIF );

  // check folder
  if ( fOK )
  {
    if ( (pszFolder == NULL) || (*pszFolder == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      BOOL fIsNew;

      fIsNew = !SubFolNameToObjectName( pszFolder,  pCLMData->szFolObjName );

      if ( fIsNew )
      {
        fOK = FALSE;
        pszParm = pszFolder;
        usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    } /* endif */
  } /* endif */

  // check options
  if ( fOK )
  {
    if ( (lOptions & CLEANMEM_EXTERNAL_MEMORY_OPT) && (lOptions & CLEANMEM_INTERNAL_MEMORY_OPT) )
    {
      fOK = FALSE;
      usRC = WRONG_OPTIONS_RC;
      UtlErrorHwnd( EQFRS_INVALID_CMD, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else if ( (lOptions & CLEANMEM_INTERNAL_MEMORY_OPT) )
    {
      pCLMData->fExternalMemory = FALSE;
    }
    else
    {
      // external memory or none
      pCLMData->fExternalMemory = TRUE;
    } /* endif */

    if ( fOK )
    {
      // merge option valid for internal memories only
      if ( pCLMData->fExternalMemory && (lOptions & CLEANMEM_MERGE_OPT) )
      {
        fOK = FALSE;
        usRC = WRONG_OPTIONS_RC;
        UtlErrorHwnd( EQFRS_INVALID_CMD, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      // merge option and overwrite option may not be specified together
      if ( (lOptions & OVERWRITE_OPT) && (lOptions & CLEANMEM_MERGE_OPT) )
      {
        fOK = FALSE;
        usRC = WRONG_OPTIONS_RC;
        UtlErrorHwnd( EQFRS_INVALID_CMD, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    } /* endif */

    // get any impot memory format options
    if ( lOptions & ASCII_OPT )
    {
      pCLMData->fAnsi = FALSE;
    }
    else if ( lOptions & ANSI_OPT )
    {
      pCLMData->fAnsi = TRUE;
    } /* endif */

    // get codepage to be used for import
    {
      if ( pCLMData->fAnsi )
      {
          pCLMData->ulInputCP = GetLangAnsiCP(NULL);
      }
      else
      {
          pCLMData->ulInputCP = GetLangOEMCP(NULL);
      } /* endif */
    }

    // set duplicate option
    pCLMData->fKeepDups = (lOptions & CLEANMEM_KEEP_DUPS_OPT) != 0;
  } /* endif */


  // check input memory
  if ( fOK )
  {
    if ( (pszInMemory == NULL) || (*pszInMemory == EOS) )
    {
      fOK = FALSE;
      usRC = TMT_MANDCMDLINE;
      UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else if ( !UtlFileExist( pszInMemory ) )
    {
       pszParm = pszInMemory;
       UtlErrorHwnd( FILE_NOT_EXISTS, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
       fOK = FALSE;
    } /* endif */
  } /* endif */

  // check output memory
  if ( fOK )
  {
    if ( (pszOutMemory == NULL) || (*pszOutMemory == EOS) )
    {
      fOK = FALSE;
      usRC = TMT_MANDCMDLINE;
      UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else if ( (lOptions & CLEANMEM_EXTERNAL_MEMORY_OPT) && !(lOptions & OVERWRITE_OPT) && UtlFileExist( pszOutMemory ) )
    {
       pszParm = pszOutMemory;
       UtlErrorHwnd( DDE_FILE_NO_OVERWRITE, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
       fOK = FALSE;
    } /* endif */
  } /* endif */

   // store input values in set options for import
   if ( fOK )
   {
     strcpy( pCLMData->szFolder, pszFolder );
     strcpy( pCLMData->szInMemory, pszInMemory );
     strcpy( pCLMData->szOutMemory, pszOutMemory );
     pCLMData->lOptions = lOptions;
   } /* endif */


   if ( !fOK )
   {
     if ( pCLMData )
     {
       UtlAlloc( (PVOID *) &pCLMData, 0L, 0L, NOMSG );
     } /* endif */
   } /* endif */

   // prepare the process
   if ( fOK )
   {
     pData->pvCleanMemory = (PVOID)pCLMData;
     pData->fComplete = FALSE;
     pCLMData->Task = CLM_START_TASK;
   }
   else
   {
     usRC = UtlQueryUShort( QS_LASTERRORMSGID );
   } /* endif */

   return( usRC );

} /* end of function MemFuncPrepCleanMemory */

// function performaing the actual work of the CleanMemory function
USHORT MemFuncCleanMemoryProcess
(
  PFCTDATA    pData                    // function I/F session data
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PMEM_CLEAN_MEMORY_DATA pCLMData = NULL;   // Pointer to the load IDA

  pCLMData = (PMEM_CLEAN_MEMORY_DATA)pData->pvCleanMemory;
  UtlSetUShort( QS_LASTERRORMSGID, 0 );

  switch ( pCLMData->Task )
  {
    case CLM_START_TASK:
      usRC = MemFuncCLMStart( pCLMData ); 
      if ( usRC ) LogRC( usRC, "MemFuncCLMStart" );
      break;
    case CLM_NEXT_DOC_TASK:    
      usRC = MemFuncCLMNextDoc( pCLMData ); 
      if ( usRC ) LogRC( usRC, "MemFuncCLMNextDoc" );
      break;
    case CLM_START_DOC_TASK:   
      usRC = MemFuncCLMDocStart( pCLMData ); 
      if ( usRC ) LogRC( usRC, "MemFuncCLMDocStart" );
      break;
    case CLM_PROCESS_DOC_TASK: 
      usRC = MemFuncCLMDocProcess( pCLMData ); 
      if ( usRC ) LogRC( usRC, "MemFuncCLMDocProcess" );
      break;
    case CLM_END_DOC_TASK:     
      usRC = MemFuncCLMDocEnd( pCLMData ); 
      if ( usRC ) LogRC( usRC, "MemFuncCLMDocEnd" );
      break; 
    case CLM_START_MEM_TASK:   
      usRC = MemFuncCLMMemStart( pCLMData ); 
      if ( usRC ) LogRC( usRC, "MemFuncCLMMemStart" );
      break;    
    case CLM_PROCESS_MEM_TASK: 
      usRC = MemFuncCLMMemProcess( pCLMData ); 
      if ( usRC ) LogRC( usRC, "MemFuncCLMMemProcess" );
      break;
    case CLM_END_MEM_TASK:     
      usRC = MemFuncCLMMemEnd( pCLMData ); 
      if ( usRC ) LogRC( usRC, "MemFuncCLMMemEnd" );
      break;
    case CLM_STOP_TASK:        
      usRC = MemFuncCLMCleanup( pCLMData ); 
      if ( usRC ) LogRC( usRC, "MemFuncCLMCleanup" );
      break;
    case CLM_COMPLETED_TASK:   
      break;
 } /* endswitch */

 if ( usRC )
 {
   // break here
//   int i = 5;
 } /* endif */

 if ( pCLMData->Task == CLM_COMPLETED_TASK )
 {
   pData->fComplete = TRUE;
   UtlAlloc( (PVOID *)&pCLMData, 0L, 0L, NOMSG ); 
 } /* endif */

 return( usRC );

} /* end of function MemFuncImportProcess */


// MemoryCleanup start up code
USHORT MemFuncCLMStart
(
  PMEM_CLEAN_MEMORY_DATA pCLMData
)
{
  USHORT usRC = 0;
  BOOL fOK = TRUE;

#ifdef CLEANMEMORYTIMELOG
  GetElapsedTime( &(pCLMData->lMemOtherTime) );
#endif

  // get folder source language
  FolQueryInfo( pCLMData->szFolObjName, NULL, NULL, pCLMData->szFolSourceLang, NULL, FALSE );
  pCLMData->ulOEMCP = GetLangOEMCP( pCLMData->szFolSourceLang );

  // set number of matches to be kept per segment
  if ( pCLMData->lOptions & CLEANMEM_BESTMATCH_OPT )
  {
    pCLMData->iNumOfMatches = 1; 
  }
  else
  {
    pCLMData->iNumOfMatches = 3; 
  } /* endif */

  // get number of documents 
  if ( fOK )
  {
    pCLMData->usDocuments = LoadDocumentNames( pCLMData->szFolObjName, HWND_FUNCIF,
                                               LOADDOCNAMES_INCLSUBFOLDERS,
                                               (PSZ)&(pCLMData->pszDocNames ));
  } /* endif */

  // allocate document array
  if ( fOK )
  {  
    LONG lSize = pCLMData->usDocuments * sizeof(CLM_DOCUMENT);
    fOK = UtlAllocHwnd( (PVOID *)&(pCLMData->pDocTable), 0L, lSize, ERROR_STORAGE, HWND_FUNCIF );
  } /* endif */

  // create the temporary TM
  MemoryFactory *pFactory = MemoryFactory::getInstance();
  pCLMData->pMem = pFactory->createTempMemory( "$CM", pCLMData->szTempMem, pCLMData->szFolSourceLang );
#ifdef SESSIONLOG
  UtlLogWriteString( "EqfCleanMemory: Created temp memory %s", pCLMData->szTempMem );
#endif
  if ( usRC ) LogRC( usRC, "TMCreateTempMemHwnd" );

  // load QF tag table
  if ( !usRC )
  {
    USHORT usRC;

    usRC = TALoadTagTableHwnd( QFTAG_TABLE, &(pCLMData->pLoadedQFTable), TRUE, TRUE, HWND_FUNCIF );
    if ( usRC ) LogRC( usRC, "TALoadTagTableHwnd" );
  } /* endif */

  // prepare next processing step
  if ( !usRC )
  {
    pCLMData->sCurDoc  = -1;
    pCLMData->Task = CLM_NEXT_DOC_TASK;
  }
  else
  {
    pCLMData->Task = CLM_STOP_TASK;
    pCLMData->fErrorStop = TRUE;
  } /* endif */

  return( usRC );
} /* end of function MemFuncImportProcess */

// MemoryCleanup go to next document
USHORT MemFuncCLMNextDoc
(
  PMEM_CLEAN_MEMORY_DATA pCLMData
)
{
  USHORT usRC = 0;

  // Position to next document, check if document has analyzed
  pCLMData->sCurDoc++;
  if ( pCLMData->sCurDoc < pCLMData->usDocuments )
  {
    pCLMData->Task = CLM_START_DOC_TASK;
  }
  else
  {
    pCLMData->Task = CLM_START_MEM_TASK;
  } /* endif */

  return( usRC );
} /* end of function MemFuncCLMNextDoc */

// MemoryCleanup start new document
USHORT MemFuncCLMDocStart
(
  PMEM_CLEAN_MEMORY_DATA pCLMData
)
{
  USHORT usRC = 0;

  // get name of active document
  {
    int i = pCLMData->sCurDoc;
    PSZ pszCurDoc = pCLMData->pszDocNames;
    while ( i )
    {
      pszCurDoc += strlen(pszCurDoc) + 1;
      i--;
    } /* endwhile */
    strcpy( pCLMData->szCurDoc, pszCurDoc );
  }

  strcpy( pCLMData->szDocObjName, pCLMData->szFolObjName );
  strcat( pCLMData->szDocObjName, BACKSLASH_STR );
  strcat( pCLMData->szDocObjName, pCLMData->szCurDoc );

  // get document info
  usRC = DocQueryInfo2Hwnd( pCLMData->szDocObjName, NULL, pCLMData->szDocFormat, pCLMData->szDocSourceLang,
                            pCLMData->szDocTargetLang, pCLMData->szLongName, pCLMData->szAlias,  
                            NULL, TRUE, HWND_FUNCIF );
  if ( usRC ) LogRC( usRC, "DocQueryInfo2Hwnd" );

  // Check existence of segmented source document (may not be there if document has not been analyzed yet)
  strcpy( pCLMData->szFolName, UtlGetFnameFromPath( pCLMData->szFolObjName ) );
  UtlMakeEQFPath( pCLMData->szSourceDocName, pCLMData->szFolObjName[0],
                  DIRSEGSOURCEDOC_PATH, pCLMData->szFolName );
  strcat( pCLMData->szSourceDocName, BACKSLASH_STR );
  strcat( pCLMData->szSourceDocName, pCLMData->szCurDoc );

  if ( !UtlFileExist(pCLMData->szSourceDocName) )
  {
    USHORT      usOpenAction;            // dummy for UtlOpen
    USHORT      usTempRC;                // temporary return code
    HFILE       hfSource = NULLHANDLE;   // file handle for source file
    ULONG       ulSize = 1;              // file size  

    // check if this is a zero byte file
    UtlMakeEQFPath( pCLMData->szSourceDocName, pCLMData->szFolObjName[0], DIRSOURCEDOC_PATH, pCLMData->szFolName );
    strcat( pCLMData->szSourceDocName, BACKSLASH_STR );
    strcat( pCLMData->szSourceDocName, pCLMData->szCurDoc );

    usTempRC = UtlOpenHwnd( pCLMData->szSourceDocName, &hfSource, &usOpenAction, 0L, FILE_NORMAL, FILE_OPEN, OPEN_ACCESS_READONLY, 0L, FALSE, HWND_FUNCIF );
    if ( !usTempRC )
    {
      usTempRC = UtlGetFileSize( hfSource, &ulSize, FALSE );
      UtlClose( hfSource, FALSE );
    } /* endif */

    if ( ulSize == 0 )
    {
      // end processing of this document
      pCLMData->Task = CLM_END_DOC_TASK;
      return( 0 );
    }
    else
    {
      // stop process
      PSZ pszParm = pCLMData->szLongName;
      UtlErrorHwnd( ERROR_NOT_ANALYZED, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF);
      usRC = ERROR_NOT_ANALYZED;
    } /* endif */
  } /* endif */

  // handle any document alias
  if ( !usRC )
  {
    if ( UtlIsLongFileName( pCLMData->szAlias ) )
    {
      UtlLongToShortName( pCLMData->szAlias, pCLMData->szShortAlias );
    }
    else
    {
      strcpy( pCLMData->szShortAlias, pCLMData->szAlias );
      pCLMData->szAlias[0] = EOS;
    } /* endif */
  } /* endif */

  // allocate structure for document
  if ( !usRC )
  {
    if ( !UtlAllocHwnd( (PVOID *) &pCLMData->pDoc, 0L, sizeof(TBDOCUMENT), ERROR_STORAGE, HWND_FUNCIF ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // load document tag table
  if ( !usRC )
  {
    usRC = TALoadTagTableExHwnd( pCLMData->szDocFormat, (PLOADEDTABLE *)&(pCLMData->pDoc->pDocTagTable),
                                 FALSE, TALOADUSEREXIT | TALOADGETSEGCONTEXTFUNC, TRUE, HWND_FUNCIF );
    if ( usRC ) LogRC( usRC, "TALoadTagTableExHwnd" );
  } /* endif */

  // load segmented source document
  if ( !usRC )
  {
    pCLMData->pDoc->pQFTagTable = pCLMData->pLoadedQFTable;
    UtlMakeEQFPath( pCLMData->szSourceDocName, pCLMData->szFolObjName[0],
                    DIRSEGSOURCEDOC_PATH, pCLMData->szFolName );
    strcat( pCLMData->szSourceDocName, BACKSLASH_STR );
    strcat( pCLMData->szSourceDocName, pCLMData->szCurDoc );

    // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to SrcLang
    pCLMData->pDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pCLMData->szDocSourceLang);
    pCLMData->pDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pCLMData->szDocSourceLang);

    UtlSetUShort( QS_LASTERRORMSGID, 0 );
    usRC = EQFBFileReadExW( pCLMData->szSourceDocName, pCLMData->pDoc, 0 );
    if ( usRC )
    {
      // get original return code of error message box
      USHORT usOrgRC = UtlQueryUShort( QS_LASTERRORMSGID);
      if ( usOrgRC ) usRC = usOrgRC;
      LogRC( usRC, "EQFBFileReadExW" );
    } /* end */       
  } /* endif */

  // allocate document segment table
  if ( !usRC )
  {
    LONG lSize = sizeof(CLM_SEGMENT) * (pCLMData->pDoc->ulMaxSeg + 2);
    PCLM_DOCUMENT pDoc = pCLMData->pDocTable + pCLMData->sCurDoc;
    strcpy( pDoc->szDocName, pCLMData->szCurDoc );
    pDoc->ulSegments = pCLMData->pDoc->ulMaxSeg;
    if ( !UtlAllocHwnd( (PVOID *)&(pDoc->pSegment), 0L, lSize, ERROR_STORAGE, HWND_FUNCIF ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // determine next processing step
  if ( !usRC )
  {
    pCLMData->ulSegNum = 1;
    pCLMData->ulAddSegNum = 1;
    pCLMData->ulActiveTable = STANDARDTABLE;
    pCLMData->Task = CLM_PROCESS_DOC_TASK;
  }
  else
  {
    pCLMData->fErrorStop = TRUE;
    pCLMData->Task = CLM_STOP_TASK;
  } /* endif */

  return( usRC );
} /* end of function MemFuncCLMDocStart */


// MemoryCleanup process document
USHORT MemFuncCLMDocProcess
(
  PMEM_CLEAN_MEMORY_DATA pCLMData
)
{
  int  i = 20;           // segment counter
  PTBSEGMENT pSeg;       // pointer to segment
  USHORT usRC = 0;       // return code 

  // process next 20 segments 
  while ( !usRC && (i > 0) && (  pCLMData->ulSegNum <= pCLMData->pDoc->ulMaxSeg) )
  {
    // get segment pointer
    ULONG ulCurrentSegNum = pCLMData->ulSegNum;
    pSeg = EQFBGetFromBothTables( pCLMData->pDoc, &(pCLMData->ulSegNum), &(pCLMData->ulAddSegNum), 
                                  &(pCLMData->ulActiveTable));

    // write segment to TM
    if ( pSeg != NULL ) 
    {
      // for source/source memory write all translatable segments
      if ( pSeg->qStatus != QF_NOP )
      {
        OtmProposal prop;

        prop.setSource( pSeg->pDataW );
        prop.setTarget( pSeg->pDataW );

        // we use the index of the document as document short name
        sprintf( pCLMData->szBuffer, "NUM%d", pCLMData->sCurDoc );
        prop.setDocShortName( pCLMData->szBuffer );

        prop.setDocName( pCLMData->szLongName );

        prop.setSegmentNum( ulCurrentSegNum );
        prop.setMarkup( pCLMData->szDocFormat );
        prop.setSourceLanguage( pCLMData->szDocSourceLang );
        prop.setTargetLanguage( pCLMData->szDocTargetLang );

        // handle any segment context
        {
          PLOADEDTABLE pTable = (PLOADEDTABLE)pCLMData->pDoc->pDocTagTable;
          if ( pTable->pfnGetSegContext )
          {
            // ulSegnum is already positioned on next segment!!!
            PTBSEGMENT pPrevSeg = EQFBGetSegW( pCLMData->pDoc, ulCurrentSegNum - 1 );
            PTBSEGMENT pNextSeg = EQFBGetSegW( pCLMData->pDoc, ulCurrentSegNum + 1);
            PSZ_W pszPrevSegData = ( pPrevSeg ) ? pPrevSeg->pDataW : NULL;
            PSZ_W pszNextSegData = ( pNextSeg ) ? pNextSeg->pDataW : NULL;

            (pTable->pfnGetSegContext)( pSeg->pDataW, pszPrevSegData, pszNextSegData, pCLMData->szContext, (LONG)pCLMData->pDoc, ulCurrentSegNum  );
            prop.setContext( pCLMData->szContext );
          } /* endif */
        }


        usRC = (USHORT)pCLMData->pMem->putProposal( prop );

        if ( usRC == NO_ERROR )
        {
          pCLMData->ulSegments++;
        } /* endif */
      } /* endif */
    } /* endif */

    // continue with next segment
    i--;
  } /* endwhile */

  // update slider position 
  if ( !usRC )
  {
    LONG  lPosInDoc, lSlotPerDoc, lPosOfDoc;

    lPosInDoc = (LONG)pCLMData->ulSegNum * 100L / (LONG)pCLMData->pDoc->ulMaxSeg;
    lSlotPerDoc = 100L / (LONG)pCLMData->usDocuments;
    lPosOfDoc   = (LONG)pCLMData->sCurDoc * 100L / (LONG)pCLMData->usDocuments;
    pCLMData->usComplete = (USHORT)(lPosOfDoc + (lPosInDoc * lSlotPerDoc / 100L));
  } /* endif */

  // set next processing step
  if ( usRC )
  {
    pCLMData->Task = CLM_STOP_TASK;
    pCLMData->fErrorStop = TRUE;
  }
  else if ( pCLMData->ulSegNum > pCLMData->pDoc->ulMaxSeg )
  {
    pCLMData->Task = CLM_END_DOC_TASK;
  }
  else
  {
    pCLMData->Task = CLM_PROCESS_DOC_TASK;
  } /* endif */

  return( usRC );
} /* end of function MemFuncCLMDocProcess */

// MemoryCleanup end document
USHORT MemFuncCLMDocEnd
(
  PMEM_CLEAN_MEMORY_DATA pCLMData
)
{
  USHORT usRC = 0;

  // free document
  if ( pCLMData->pDoc )
  {
    if ( pCLMData->pDoc->pDocTagTable ) TAFreeTagTable( (PLOADEDTABLE)pCLMData->pDoc->pDocTagTable );
    SegFileFreeDoc( (PVOID *) &(pCLMData->pDoc) );
    pCLMData->pDoc = NULL;
  } /* endif */

  pCLMData->Task = ( !usRC ) ? CLM_NEXT_DOC_TASK : CLM_STOP_TASK;

#ifdef CLEANMEMORYTIMELOG
  GetElapsedTime( &(pCLMData->lMemFillTime) );
#endif

  return( usRC );
} /* end of function MemFuncCLMDocEnd */

// MemoryCleanup start read of input memory
USHORT MemFuncCLMMemStart
(
  PMEM_CLEAN_MEMORY_DATA pCLMData
)
{
  USHORT usRC = 0;
  BOOL   fEOF = FALSE;

  MemoryFactory *pFactory = MemoryFactory::getInstance();


  // open input memory
  {
    USHORT      usOpenAction;            // dummy for UtlOpen

    usRC = UtlOpenHwnd( pCLMData->szInMemory, &(pCLMData->hfIn), &usOpenAction, 0L,
                        FILE_NORMAL, FILE_OPEN, OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                        0L, TRUE, HWND_FUNCIF );
    if ( usRC ) LogRC( usRC, "UtlOpenHwnd" );

//  pCLMData->hInFile = fopen( pCLMData->szInMemory, "rb" );
  }

  //if ( pCLMData->hInFile == NULL )
  //{
  //  PSZ pszParm = pCLMData->szInMemory;
  //  UtlErrorHwnd( FILE_NOT_EXISTS, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF);
  //  usRC = FILE_NOT_EXISTS;
  //} /* endif */

  // open intermediate output memory
  if ( !usRC )
  {
    // get temporary file name for the memory file
    UtlMakeEQFPath( pCLMData->szBuffer, NULC, SYSTEM_PATH, NULL );
    strcpy( pCLMData->szTempMemory, _tempnam( pCLMData->szBuffer, "clmmem" ) );

    if ( pCLMData->fKeepDups )
    {
      pCLMData->hOutFile = fopen( pCLMData->szOutMemory, "wb" );
    }
    else
    {
      pCLMData->hOutFile = fopen( pCLMData->szTempMemory, "wb" );
    } /* endif */
    if ( pCLMData->hOutFile == NULL )
    {
      PSZ pszParm = pCLMData->szOutMemory;
      UtlErrorHwnd( ERROR_FILE_WRITE_ERROR, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF);
      usRC = ERROR_FILE_WRITE_ERROR;
    }
    else
    {
      fwrite( UNICODEFILEPREFIX, 1, 2, pCLMData->hOutFile );
    } /* endif */
  } /* endif */

  // create/open internal memory if output to internal memory is requested
  if ( !usRC )
  {
    if ( !pCLMData->fExternalMemory )
    {
      MemoryFactory *pFactory = MemoryFactory::getInstance();
      BOOL fMerge = FALSE;

      if ( pFactory->exists( NULL, pCLMData->szOutMemory ) )      
      {
        if ( pCLMData->lOptions & OVERWRITE_OPT )
        {
          pFactory->deleteMemory( NULL, pCLMData->szOutMemory );
        }
        else if ( !(pCLMData->lOptions & CLEANMEM_MERGE_OPT) )
        {
          PSZ pszParm = pCLMData->szOutMemory;
          usRC = ERROR_MEM_NAME_INVALID;
          UtlErrorHwnd( ERROR_MEM_NAME_INVALID, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        }
        else
        {
          fMerge = TRUE;
        } /* endif */
      } /* endif */
 
      // open/create the output TM
      if ( !usRC )
      {
        int iRC = 0;

        if ( fMerge )
        {
#ifdef SESSIONLOG
          UtlLogWriteString( "EqfCleanMemory: Open of memory %s", pCLMData->szOutMemory );
#endif
          pCLMData->pOutMem = pFactory->openMemory( NULL, pCLMData->szOutMemory, EXCLUSIVE, &iRC );
          if ( usRC ) LogRC( (USHORT)iRC, "pFactory->openMemory" );
        }
        else
        {
#ifdef SESSIONLOG
          UtlLogWriteString( "EqfCleanMemory: Create of memory %s", pCLMData->szOutMemory );
#endif
          pCLMData->pOutMem = pFactory->createMemory( NULL, pCLMData->szOutMemory, "", pCLMData->szFolSourceLang, &iRC );
          if ( usRC ) LogRC( (USHORT)iRC, "pFactory->createMemory" );
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  // get first bytes of external memory and check for UTF-16 encoding
  if ( !usRC )
  {
    ULONG ulBytesRead = 0;

    // get first two bytes and check for BOM
    BYTE chBOM[2];

    UtlReadHwnd( pCLMData->hfIn, chBOM, 2, &ulBytesRead, TRUE, HWND_FUNCIF );
//    fread( chBOM, 1, 2, pCLMData->hInFile );

    if ( memcmp( chBOM, UNICODEFILEPREFIX, 2 ) == 0 )
    {
      pCLMData->fUnicode = TRUE;
      usRC = MemFillBufferW( pCLMData );
    }
    else
    {
      ULONG ulFilePos = 0;

      pCLMData->fUnicode = FALSE;

      //// re-open file in text mode
      //fclose( pCLMData->hInFile );
      //pCLMData->hInFile = fopen( pCLMData->szInMemory, "r" );

      // reposition to begin of file
      UtlChgFilePtr( pCLMData->hfIn, 0L, FILE_BEGIN, &ulFilePos, FALSE );
      usRC = MemFillBuffer( pCLMData );
    } /* endif */

    MemGetLine( pCLMData, pCLMData->szLine, sizeof(pCLMData->szLine), pCLMData->fUnicode, &fEOF  );

    if ( wcsnicmp( pCLMData->szLine, L"<ntmmemorydb>", 13 ) != 0 )
    {
      UtlErrorHwnd( ERROR_MEM_INVALID_SGML, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF);
      usRC = NO_VALID_UNICODEFORMAT;
    } /* endif */
  } /* endif */

  // read/write memory header up to first segment
  if ( !usRC )
  {
    while ( !fEOF && 
            (wcsnicmp( pCLMData->szLine, L"<segment>", 9 ) != 0) &&
            (wcsnicmp( pCLMData->szLine, L"</ntmmemorydb>", 14 ) != 0))
    {
      fputws( pCLMData->szLine, pCLMData->hOutFile ); 
      fputws( L"\r\n", pCLMData->hOutFile ); 
      pCLMData->szLine[0] = 0;
      MemGetLine( pCLMData, pCLMData->szLine, sizeof(pCLMData->szLine), pCLMData->fUnicode, &fEOF );
    } /*endwhile */

    // remember size of memory header in output file
    pCLMData->ulMemHeaderSize = ftell( pCLMData->hOutFile );
  } /* endif */

  // allocate vector and proposal object for the memory lookup
  if ( !usRC )
  {
    pCLMData->pvMatchList = new std::vector<OtmProposal *>;
    for ( int i = 0; i < 10; i++ )
    {
      pCLMData->pvMatchList->push_back( new OtmProposal );
    } /* endfor */     

  }

  pCLMData->Task = ( !usRC ) ? CLM_PROCESS_MEM_TASK : CLM_STOP_TASK;

  return( usRC );
} /* end of function MemFuncCLMMemStart */

// utility to strip CRs in the data
void CLMCopyWithoutCR( PSZ_W pszTarget, PSZ_W pszSource )
{
  while ( *pszSource )
  {
    if ( *pszSource != L'\r' )
    {
      *pszTarget++ = *pszSource++;
    }
    else
    {
      pszSource++;
    } /* endif */
  } /*endwhile */
  *pszTarget = 0;
}

// for debug purposes
static long lMemFuncCLMMemProcessCounter = 0;

// MemoryCleanup memory read process
USHORT MemFuncCLMMemProcess
(
  PMEM_CLEAN_MEMORY_DATA pCLMData
)
{
  USHORT usRC = 0;
  BOOL fMatch = FALSE;
  BOOL fEOF = FALSE;
  BOOL fExactExists = FALSE;
  USHORT usBestMatchLevel = 0;
  USHORT usBestGITRMatchLevel = 0;

  PCLM_MATCH  apMatchSegments[MAX_MATCHES*2];         // table of best matches for which this segment is a match
  int iMatchSegments = 0;

  // read data until segment is complete
  if ( wcsnicmp( pCLMData->szLine, L"<segment>", 9 ) == 0 ) 
  {
    // reset segment data
    pCLMData->szSource[0] = 0;
    pCLMData->szControl[0] = 0;
    pCLMData->szTarget[0] = 0;
    pCLMData->szContext[0] = 0;
    pCLMData->szAddInfo[0] = 0;
    
    wcscpy( pCLMData->szSegStart, pCLMData->szLine );

    while ( !usRC && !fEOF && wcsnicmp( pCLMData->szLine, L"</segment>", 10 ) != 0 ) 
    {
      MemGetLine( pCLMData, pCLMData->szLine, sizeof(pCLMData->szLine), pCLMData->fUnicode, &fEOF );

      if ( wcsnicmp( pCLMData->szLine, L"<control>", 9 ) == 0 )
      {
        // get the control string
        MemGetLine( pCLMData, pCLMData->szControl, sizeof(pCLMData->szControl), pCLMData->fUnicode, &fEOF );

        if ( fEOF )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
        }
        else
        {
          // get contol string end tag and check it
          MemGetLine( pCLMData, pCLMData->szLine, sizeof(pCLMData->szLine), pCLMData->fUnicode, &fEOF  );

          // todo: check for </control>
        } /* endif */

      }
      else if ( wcsnicmp( pCLMData->szLine, L"<source>", 8 ) == 0  )
      {
        // get the segment source
        BOOL fEnd = MemStripTag( pCLMData->szLine + 8, L"</source>" );
        wcscat( pCLMData->szSource, pCLMData->szLine + 8 );

        while ( !fEnd && !fEOF )
        {
          wcscat( pCLMData->szSource, L"\n" ); 
          MemGetLine( pCLMData, pCLMData->szLine, sizeof(pCLMData->szLine), pCLMData->fUnicode, &fEOF );
          fEnd = MemStripTag( pCLMData->szLine, L"</source>" );
          if ( !fEnd || (pCLMData->szLine[0] != 0) )
          {
            wcscat( pCLMData->szSource, pCLMData->szLine );
          } /* endif */
        } /*endwhile */
        if ( !fEnd )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
        }
        else
        {
          // clear line as data has been processed
          pCLMData->szLine[0] = 0;
        } /* endif */
      }
      else if ( wcsnicmp( pCLMData->szLine, L"<target>", 8 ) == 0 )
      {
        // get the segment target
        BOOL fEnd = MemStripTag( pCLMData->szLine + 8, L"</target>" );
        wcscat( pCLMData->szTarget, pCLMData->szLine + 8 );

        while ( !fEnd && !fEOF )
        {
          wcscat( pCLMData->szTarget, L"\n" ); 
          MemGetLine( pCLMData, pCLMData->szLine, sizeof(pCLMData->szLine), pCLMData->fUnicode, &fEOF );
          fEnd = MemStripTag( pCLMData->szLine, L"</target>" );
          if ( !fEnd || (pCLMData->szLine[0] != 0) )
          {
            wcscat( pCLMData->szTarget, pCLMData->szLine );
          } /* endif */
        } /*endwhile */
        if ( !fEnd )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
        }
        else
        {
          // clear line as data has been processed
          pCLMData->szLine[0] = 0;
        } /* endif */
      } 
      else if ( wcsnicmp( pCLMData->szLine, L"<context>", 9 ) == 0 )
      {
        // get the segment context
        BOOL fEnd = MemStripTag( pCLMData->szLine + 9, L"</context>" );
        wcscat( pCLMData->szContext, pCLMData->szLine + 9 );

        while ( !fEnd && !fEOF )
        {
          wcscat( pCLMData->szContext, L"\n" ); 
          MemGetLine( pCLMData, pCLMData->szLine, sizeof(pCLMData->szLine), pCLMData->fUnicode, &fEOF );
          fEnd = MemStripTag( pCLMData->szLine, L"</context>" );
          if ( !fEnd || (pCLMData->szLine[0] != 0) )
          {
            wcscat( pCLMData->szContext, pCLMData->szLine );
          } /* endif */
        } /*endwhile */
        if ( !fEnd )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
        } /* endif */
      } 
      else if ( wcsnicmp( pCLMData->szLine, L"<adddata>", 9 ) == 0 )
      {
        // get the segment context
        BOOL fEnd = MemStripTag( pCLMData->szLine + 9, L"</adddata>" );
        wcscat( pCLMData->szAddInfo, pCLMData->szLine + 9 );

        while ( !fEnd && !fEOF )
        {
          wcscat( pCLMData->szAddInfo, L"\n" ); 
          MemGetLine( pCLMData, pCLMData->szLine, sizeof(pCLMData->szLine), pCLMData->fUnicode, &fEOF );
          fEnd = MemStripTag( pCLMData->szLine, L"</adddata>" );
          if ( !fEnd || (pCLMData->szLine[0] != 0) )
          {
            wcscat( pCLMData->szAddInfo, pCLMData->szLine );
          } /* endif */
        } /*endwhile */
        if ( !fEnd )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
        } /* endif */
      } /* endif */
    } /*endwhile */

    // read next line
    MemGetLine( pCLMData, pCLMData->szLine, sizeof(pCLMData->szLine), pCLMData->fUnicode, &fEOF );

    if ( usRC )
    {
      PSZ pszParms[2];

      pszParms[0] = "n/a";
      pszParms[1] = pCLMData->szInMemory;

      UtlErrorHwnd( ERROR_INVALID_SEGMENT, MB_CANCEL, 2, pszParms, EQF_ERROR, HWND_FUNCIF);
    } /* endif */
  } /* endif */

  // lookup segment in document memory
  if ( !usRC && pCLMData->szControl[0] && pCLMData->szSource[0] && pCLMData->szTarget[0] )
  {
    OtmProposal searchKey;


    fMatch = FALSE;  

    WideCharToMultiByte( CP_OEMCP, 0, pCLMData->szControl, -1, pCLMData->szControlAscii, sizeof(pCLMData->szControlAscii), NULL, NULL );

    OtmProposal::clearAllProposals( *(pCLMData->pvMatchList) );

    searchKey.setSegmentNum( atol( UtlParseX15( pCLMData->szControlAscii, 0 ) ) );

    searchKey.setContext( pCLMData->szContext );
    searchKey.setAddInfo( pCLMData->szAddInfo );
    searchKey.setDocShortName( UtlParseX15( pCLMData->szControlAscii, 7 ) );
    searchKey.setDocName( UtlParseX15( pCLMData->szControlAscii, 8 ) );

    searchKey.setMarkup( UtlParseX15( pCLMData->szControlAscii, 6 ));
    searchKey.setSourceLanguage( UtlParseX15( pCLMData->szControlAscii, 3 ) );
    searchKey.setTargetLanguage( UtlParseX15( pCLMData->szControlAscii, 4 ));

    // check some sizes before we continue
    if ( (wcslen( pCLMData->szSource ) <= 2047) || (wcslen( pCLMData->szTarget ) <= 2047 ) )
    {
#ifdef CLEANMEMORYTIMELOG
      GetElapsedTime( &(pCLMData->lMemOtherTime) );
#endif

      CLMCopyWithoutCR( pCLMData->szTempSource, pCLMData->szSource );
      searchKey.setSource( pCLMData->szTempSource );

      pCLMData->pMem->searchProposal( searchKey, *(pCLMData->pvMatchList), GET_EXACT | GET_NO_GENERICREPLACE | GET_ALWAYS_WITH_TAGS );

      // retry with generic inline tag replacement if no 100% match found
      int iNumMatches = OtmProposal::getNumOfProposals( *(pCLMData->pvMatchList) );
      if ( iNumMatches > 0 )
      {
        USHORT usMatch = (USHORT)(*pCLMData->pvMatchList)[0]->getFuzziness();
        if ( usMatch < 100 )
        {
          iNumMatches = 0;
        }
      }
      if ( iNumMatches == 0 )
      {
        pCLMData->pMem->searchProposal( searchKey, *(pCLMData->pvMatchList), GET_EXACT | GET_ALWAYS_WITH_TAGS );
      }

#ifdef CLEANMEMORYTIMELOG
      GetElapsedTime( &(pCLMData->lMemLookupTime) );
#endif
    }
    else
    {
      // ignore this segment
    } /* endif */

    int iNumMatches = OtmProposal::getNumOfProposals( *(pCLMData->pvMatchList) );

    if ( !usRC && (iNumMatches != 0) )
    {
      int i = 0;
      static EQFSEGINFO Segment;
      static EQFSEGINFO Proposal;

      usBestGITRMatchLevel = 0;
      usBestMatchLevel = (USHORT)(*pCLMData->pvMatchList)[0]->getFuzziness();

      while ( i < iNumMatches )
      {
        // check if we have already a better match for this segment
        CHAR          szShortNameBuf[MAX_FILESPEC+1];
        OtmProposal   *pProp = (*pCLMData->pvMatchList)[i];
        ULONG         ulSegNum = pProp->getSegmentNum();
        pProp->getDocShortName( szShortNameBuf, sizeof(szShortNameBuf) );
        int           iDocNum = atoi( szShortNameBuf + 3 ); 
        USHORT        usCurMatchLevel = (USHORT)pProp->getFuzziness();
        PCLM_DOCUMENT pDoc = pCLMData->pDocTable + iDocNum;
        PCLM_SEGMENT  pSeg = pDoc->pSegment + ulSegNum;
        int iMatch = 0;

        // special match pre-processing: increase match level if source and proposal match exactly
        if ( usCurMatchLevel >= 100 )
        {
          pProp->getSource( Segment.szSource, sizeof(Segment.szSource)/sizeof(Segment.szSource[0]) );
          searchKey.getSource( Segment.szTarget, sizeof(Segment.szTarget)/sizeof(Segment.szTarget[0]) );
          if ( wcscmp( Segment.szSource, Segment.szTarget ) == 0 )
          {
            usCurMatchLevel++;
          } /* endif */
        } /* endif */

        // for keep-dups only: check if there is already an exact match for this segment
        if ( pCLMData->fKeepDups )
        {
          iMatch = 0;
          while ( iMatch < pCLMData->iNumOfMatches )
          {
            if  ( pSeg->aBestMatch[iMatch].usMatchLevel >= 100 )
            {
              fExactExists = TRUE;
            } /* endif */
            iMatch++;
          } /*endwhile */
        } /* endif */

        // check if this match is better than the ones contained in the table
        iMatch = 0;
        while ( (iMatch < pCLMData->iNumOfMatches) &&  (pSeg->aBestMatch[iMatch].usMatchLevel >= usCurMatchLevel) )
        {
          iMatch++;
        } /*endwhile */

        if ( (usCurMatchLevel >= 30) && (iMatch < pCLMData->iNumOfMatches) )
        {
          // found the corrrect place to insert this proposal

          // make room for this proposal
          int iLen = (pCLMData->iNumOfMatches - 1) - iMatch;
          if ( iLen )
          {
            memmove( &(pSeg->aBestMatch[iMatch + 1]), &(pSeg->aBestMatch[iMatch]), iLen * sizeof(CLM_MATCH) );
          } /*endif */

          // insert this proposal
          pSeg->aBestMatch[iMatch].usMatchLevel = usCurMatchLevel;
          pSeg->aBestMatch[iMatch].ulLen = 0;
          pSeg->aBestMatch[iMatch].ulPos = 0;
          apMatchSegments[iMatchSegments++] = &(pSeg->aBestMatch[iMatch]);
          fMatch = TRUE;
        } /* endif */

        // if still no exact match try the same for our match table with inline tag generic replacement matches
        if ( (usCurMatchLevel < 101) && (pSeg->BestMatchGITR.usMatchLevel < 101) )
        {
          USHORT usNewMatchLevel = 0;
          SHORT sMatchState = 0;

          memset( &Segment, 0, sizeof(Segment) );
          pProp->getSource( Segment.szSource, sizeof(Segment.szSource)/sizeof(Segment.szSource[0]) );
          pProp->getDocName( Segment.szDocument, sizeof(Segment.szDocument) );
          pProp->getMarkup( Segment.szMarkup, sizeof(Segment.szMarkup) );
          pProp->getSourceLanguage( Segment.szSourceLanguage, sizeof(Segment.szSourceLanguage) );
          pProp->getTargetLanguage( Segment.szTargetLanguage, sizeof(Segment.szTargetLanguage) );
          Segment.lSegNumber = (LONG)pProp->getSegmentNum();

          memset( &Proposal, 0, sizeof(Proposal) );
          searchKey.getSource( Proposal.szSource, sizeof(Proposal.szSource)/sizeof(Proposal.szSource[0]) );
          wcscpy( Proposal.szTarget, pCLMData->szTarget );
          searchKey.getDocName( Proposal.szDocument, sizeof(Proposal.szDocument) );
          searchKey.getMarkup( Proposal.szMarkup, sizeof(Proposal.szMarkup) );
          searchKey.getSourceLanguage( Proposal.szSourceLanguage, sizeof(Proposal.szSourceLanguage) );
          searchKey.getTargetLanguage( Proposal.szTargetLanguage, sizeof(Proposal.szTargetLanguage) );
          Proposal.lSegNumber = (LONG)searchKey.getSegmentNum();
          
          NTMGetMatchLevel( &Segment, &Proposal, (SHORT *)&usNewMatchLevel, &sMatchState, USE_GENERIC_INLINETAG_REPL_OPT );

          if ( usNewMatchLevel > usBestGITRMatchLevel )
          {
            usBestGITRMatchLevel = usNewMatchLevel;
          } /* endif */             

          if ( usNewMatchLevel > usCurMatchLevel )
          {
            // only exact matches are of interest here
            if ( (usNewMatchLevel >= 100) && (pSeg->BestMatchGITR.usMatchLevel == 0) )
            {
              // remember new exact match
              pSeg->BestMatchGITR.usMatchLevel = usNewMatchLevel;
              fMatch = TRUE;
              apMatchSegments[iMatchSegments++] = &(pSeg->BestMatchGITR);
            } /* endif */
          } /* endif */
        } /* endif */
        i++;
      } /*endwhile */
    } 

    if ( fMatch )
    {
      pCLMData->ulSegsCopied++;
    }
    else
    {
      pCLMData->ulSegsSkipped++;
    } /* endif */
  } /* endif */

  // for keep-duplicates option only: 
  // keep segment if is a fuzzy match and no exact matchs found so far or if it is an
  // exact match
  if ( !usRC && pCLMData->fKeepDups )
  {
    if ( fMatch )
    {
      if ( (usBestMatchLevel < 100) && (usBestGITRMatchLevel < 100) && fExactExists )
      {
        fMatch = FALSE;
      } /* endif */
    } 
    else if ( (usBestMatchLevel >= 100) || (usBestGITRMatchLevel >= 100) )
    {
      // always keep exact matches
      fMatch = TRUE;
    } 
    else if ( !fExactExists && (usBestMatchLevel > 30) )
    {
      // keep fuzy matches as no exact match exists so far
      fMatch = TRUE;
    } /* endif */
  } /* endif */

  // write to intermediate memory if match
  if ( !usRC && fMatch )
  {
#ifdef CLEANMEMORYTIMELOG
    GetElapsedTime( &(pCLMData->lMemOtherTime) );
#endif

    {
      if ( pCLMData->fKeepDups && !pCLMData->fExternalMemory )
      {
        // write to internal memory as keep-dups has no intermediate memory
        OtmProposal prop;

        WideCharToMultiByte( CP_OEMCP, 0, pCLMData->szControl, -1, pCLMData->szControlAscii, sizeof(pCLMData->szControlAscii), NULL, NULL );

        prop.setSource( pCLMData->szSource);
        prop.setTarget( pCLMData->szTarget );
        prop.setContext( pCLMData->szContext );
        prop.setAddInfo( pCLMData->szAddInfo );
        prop.setDocShortName( UtlParseX15( pCLMData->szControlAscii, 7 ) );
        prop.setDocName( UtlParseX15( pCLMData->szControlAscii, 8 ) );
        prop.setSegmentNum( atol( UtlParseX15( pCLMData->szControlAscii, 0 ) ) );
        USHORT usTranslationFlag = (USHORT)atoi( UtlParseX15( pCLMData->szControlAscii, 1 ) );
        if ( usTranslationFlag == TRANSLFLAG_NORMAL )
        {
          prop.setType( OtmProposal::eptManual );
        }
        else if ( usTranslationFlag == TRANSLFLAG_MACHINE )
        {
          prop.setType( OtmProposal::eptMachine );
        }
        else if ( usTranslationFlag == TRANSLFLAG_GLOBMEM )
        {
          prop.setType( OtmProposal::eptGlobalMemory );
        }
        else if ( usTranslationFlag == TRANSLFLAG_GLOBMEMSTAR )
        {
          prop.setType( OtmProposal::eptGlobalMemoryStar );
        }
        else 
        {
          prop.setType( OtmProposal::eptUndefined );
        } /* endif */
        prop.setUpdateTime( atol( UtlParseX15( pCLMData->szControlAscii, 2 ) ) );
        prop.setMarkup( UtlParseX15( pCLMData->szControlAscii, 6 ) );
        prop.setSourceLanguage( UtlParseX15( pCLMData->szControlAscii, 3 ) );
        prop.setTargetLanguage( UtlParseX15( pCLMData->szControlAscii, 4 ) );
        
        usRC = (USHORT)pCLMData->pOutMem->putProposal( prop );
        if ( usRC ) LogRC( usRC, "putProposal" );
      }
      else
      {
        LONG lPos = ftell( pCLMData->hOutFile );
        LONG lLen = 0;
        int i = 0;

        // write segment
        fputws( pCLMData->szSegStart, pCLMData->hOutFile );
        fputws( L"\r\n", pCLMData->hOutFile ); 
        fputws( L"<Control>\r\n", pCLMData->hOutFile );
        fputws( pCLMData->szControl, pCLMData->hOutFile );
        fputws( L"\r\n", pCLMData->hOutFile ); 
        fputws( L"</Control>\r\n", pCLMData->hOutFile );
        if ( pCLMData->szContext[0] != 0 )
        {
          fputws( L"<Context>", pCLMData->hOutFile );
          fputws( pCLMData->szContext, pCLMData->hOutFile );
          fputws( L"</Context>\r\n", pCLMData->hOutFile );
        } /* endif */           
        if ( pCLMData->szAddInfo[0] != 0 )
        {
          fputws( L"<AddData>", pCLMData->hOutFile );
          fputws( pCLMData->szAddInfo, pCLMData->hOutFile );
          fputws( L"</AddData>\r\n", pCLMData->hOutFile );
        } /* endif */           
        fputws( L"<Source>", pCLMData->hOutFile );
        fputws( pCLMData->szSource, pCLMData->hOutFile );
        fputws( L"</Source>\r\n", pCLMData->hOutFile );
        fputws( L"<Target>", pCLMData->hOutFile );
        fputws( pCLMData->szTarget, pCLMData->hOutFile );
        fputws( L"</Target>\r\n", pCLMData->hOutFile );
        fputws( L"</Segment>\r\n", pCLMData->hOutFile );
        lLen = ftell( pCLMData->hOutFile ) - lPos;

        // update all match fields for this segment with actual position and length
        for( i = 0; i < iMatchSegments; i++ )
        {
          apMatchSegments[i]->ulPos = lPos;
          apMatchSegments[i]->ulLen = lLen;
        } /* endfor */
      } /* endif */
    }

#ifdef CLEANMEMORYTIMELOG
    GetElapsedTime( &(pCLMData->lMemWriteTime) );
#endif
  } /* endif */
  
  // skip empy lines
  while ( !fEOF && (pCLMData->szLine[0] == 0) )
  {
    MemGetLine( pCLMData, pCLMData->szLine, sizeof(pCLMData->szLine), pCLMData->fUnicode, &fEOF );
  } /*endwhile */

  if ( usRC )
  {
    pCLMData->fErrorStop = TRUE;
    pCLMData->Task = CLM_STOP_TASK;
  }
  else if ( wcsnicmp( pCLMData->szLine, L"<segment>", 9 ) == 0 ) 
  {
    pCLMData->Task = CLM_PROCESS_MEM_TASK;
  }
  else if ( fEOF || (wcsnicmp( pCLMData->szLine, L"</ntmmemorydb>", 14 ) == 0) )
  {
    if ( pCLMData->fExternalMemory || !pCLMData->fKeepDups )
    {
      fputws( L"</NTMMemoryDb>\r\n", pCLMData->hOutFile );
    } /* endif */
    pCLMData->Task = CLM_END_MEM_TASK;
  }
  else
  {
    PSZ pszParm = pCLMData->szInMemory;
    UtlErrorHwnd( ERROR_SGML_TAG, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF);
    usRC = ERROR_SGML_TAG;
    pCLMData->fErrorStop = TRUE;
    pCLMData->Task = CLM_STOP_TASK;
  } /* endif */


  // increment our debug counter
  lMemFuncCLMMemProcessCounter++;

  return( usRC );

} /* end of function MemFuncCLMMemProcess */

int MemFuncCompMatch( const void *pv1, const void *pv2 )
{
  PCLM_MATCH pMatch1 = (PCLM_MATCH)pv1;
  PCLM_MATCH pMatch2 = (PCLM_MATCH)pv2;
  return( pMatch1->ulPos - pMatch2->ulPos );
}

// MemoryCleanup end of input memory handling
USHORT MemFuncCLMMemEnd
(
  PMEM_CLEAN_MEMORY_DATA pCLMData
)
{
  USHORT usRC = 0;
  PCLM_MATCH pMatchTable = NULL;                 // table with the best matches for each segment
  LONG  lMatches = 0;                            // number of matches in the match table

  if ( pCLMData->hfIn )     UtlClose( pCLMData->hfIn, FALSE );
  if ( pCLMData->hInFile )  fclose( pCLMData->hInFile );
  if ( pCLMData->hOutFile ) fclose( pCLMData->hOutFile );

#ifdef CLEANMEMORYTIMELOG
  GetElapsedTime( &(pCLMData->lMemOtherTime) );
#endif

  // create table containing the best matches for each segment
  if ( !pCLMData->fKeepDups )
  {
    // count number of segment in all document tables
    LONG lSegments = 0;
    int iDocument;
    for( iDocument = 0; iDocument < pCLMData->usDocuments; iDocument++ )
    {
      lSegments += pCLMData->pDocTable[iDocument].ulSegments;
    } /* endfor */

    // allocate match table for all segments
    lSegments *= pCLMData->iNumOfMatches;
    UtlAlloc( (PVOID *)&pMatchTable, 0, lSegments * sizeof(CLM_MATCH), NOMSG );

    // fill the table with best matches
    for( iDocument = 0; iDocument < pCLMData->usDocuments; iDocument++ )
    {
      PCLM_DOCUMENT pDoc = pCLMData->pDocTable + iDocument;
      ULONG ulSegment;
      for( ulSegment = 0; ulSegment < pDoc->ulSegments; ulSegment++ )
      {
        PCLM_SEGMENT pSeg = pDoc->pSegment + ulSegment;

        if ( pSeg->aBestMatch[0].usMatchLevel >= 100 )
        {
          memcpy( pMatchTable + lMatches++, &(pSeg->aBestMatch[0]), sizeof(CLM_MATCH) );
        }
        else if ( pSeg->BestMatchGITR.usMatchLevel >= 100 )
        {
          memcpy( pMatchTable + lMatches++, &(pSeg->BestMatchGITR), sizeof(CLM_MATCH) );
        }
        else 
        {
          int i = 0;
          while ( (i < pCLMData->iNumOfMatches) && (pSeg->aBestMatch[i].usMatchLevel != 0) )
          {
            memcpy( pMatchTable + lMatches++, &(pSeg->aBestMatch[i]), sizeof(CLM_MATCH) );
            i++;
          } /*endwhile */
        } /* endif */
      } /* endfor */
    } /* endfor */
  }

  // sort table on position values
  if ( !pCLMData->fKeepDups && pMatchTable )
  {
    qsort( pMatchTable, lMatches, sizeof(CLM_MATCH), MemFuncCompMatch );
  } /* endif */

#ifdef CLEANMEMORYTIMELOG
  GetElapsedTime( &(pCLMData->lMemSortTime) );
#endif

  // rename old output memory, open old output memory as input, open new output memory for write
  if ( !pCLMData->fKeepDups && pMatchTable )
  {
    // open intermediate output memory as input file
    if ( !usRC )
    {
      pCLMData->hInFile = fopen( pCLMData->szTempMemory, "rb" );
      if ( pCLMData->hInFile == NULL )
      {
        PSZ pszParm = pCLMData->szTempMemory;
        UtlErrorHwnd( FILE_NOT_EXISTS, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF);
        usRC = FILE_NOT_EXISTS;
      } /* endif */
    } /* endif */

    // open final output memory for output
    if ( !usRC && pCLMData->fExternalMemory )
    {
      pCLMData->hOutFile = fopen( pCLMData->szOutMemory, "wb" );
      if ( pCLMData->hInFile == NULL )
      {
        PSZ pszParm = pCLMData->szOutMemory;
        UtlErrorHwnd( FILE_NOT_EXISTS, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF);
        usRC = FILE_NOT_EXISTS;
      } /* endif */
    } /* endif */

    // copy header part to new output memory
    if ( !usRC  && pCLMData->fExternalMemory )
    {
      if ( fread( pCLMData->bReadBuffer, pCLMData->ulMemHeaderSize, 1, pCLMData->hInFile) != 1 )
      {
        PSZ pszParm = pCLMData->szTempMemory;
        UtlErrorHwnd( ERROR_FILE_READ_ERROR, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF);
        usRC = FILE_NOT_EXISTS;
      } /* endif */

      if ( !usRC )
      {
        fwrite( pCLMData->bReadBuffer, pCLMData->ulMemHeaderSize, 1, pCLMData->hOutFile );
      } /* endif */
    } /* endif */
  } /* endif */

  // extract all maching segments and write them to the output memory
  if ( !pCLMData->fKeepDups && pMatchTable )
  {
    long i = 0;
    long lUniqueMatches = 0;
    ULONG ulLastPos = 0;
    for( i = 0; (i < lMatches) && !usRC; i++ )
    {
      if ( pMatchTable[i].ulPos != ulLastPos )
      {
        lUniqueMatches++;
        ulLastPos = pMatchTable[i].ulPos;

        fseek( pCLMData->hInFile, pMatchTable[i].ulPos, SEEK_SET );

        if ( fread( pCLMData->bReadBuffer, pMatchTable[i].ulLen, 1, pCLMData->hInFile) != 1 )
        {
          PSZ pszParm = pCLMData->szTempMemory;
          UtlErrorHwnd( ERROR_FILE_READ_ERROR, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF);
          usRC = FILE_NOT_EXISTS;
        } /* endif */

        if ( !usRC )
        {
          if ( pCLMData->fExternalMemory )
          {
            fwrite( pCLMData->bReadBuffer, pMatchTable[i].ulLen, 1, pCLMData->hOutFile );
          }
          else
          {
            PSZ_W pszSource = NULL;
            PSZ_W pszTarget = NULL;
            PSZ_W pszEndSource = NULL;
            PSZ_W pszEndTarget = NULL;
            PSZ_W pszContext = NULL;
            PSZ_W pszEndContext = NULL;
            PSZ_W pszAddInfo = NULL;
            PSZ_W pszEndAddInfo = NULL;

            // reduce CRLF to LF (internally all memory operations expect LF as line end delimiter)
            {
              PSZ_W pszSource = (PSZ_W)pCLMData->bReadBuffer;
              PSZ_W pszTarget = pszSource;

              pCLMData->bReadBuffer[pMatchTable[i].ulLen] = 0;

              while ( *pszSource )
              {
                // ignore carriage returns 
                if ( *pszSource == L'\r' )
                {
                  pszSource++;
                  if ( (*pszSource != L'\n') )
                  {
                    *pszTarget++ = L'\n';
                  } /* endif */
                }
                else
                {
                  *pszTarget++ = *pszSource++;
                } /* endif */
              } /*endwhile */
              *pszTarget = 0;

            }

            // split segment in buffer into control, context, addinfo, source, and target string
            {
              PSZ_W pszControl = wcsstr( (PSZ_W)pCLMData->bReadBuffer, L"<Control>" );
              PSZ_W pszControlEnd = wcsstr( pszControl, L"</Control>" );
              if ( pszControl && pszControlEnd )
              {
                pszControl += 9; // skip tag

                while ( (*pszControl == L'\r') || (*pszControl == L'\n')  )
                {
                  pszControl++;
                } /*endwhile */

                while ( (pszControlEnd[-1] == L'\r') || (pszControlEnd[-1] == L'\n')  )
                {
                  pszControlEnd--;
                } /*endwhile */

                *pszControlEnd++ = 0;

                wcscpy( pCLMData->szControl, pszControl );
              } /* endif */

              if ( pszControlEnd ) pszContext = wcsstr( pszControlEnd, L"<Context>" );
              if ( pszContext )    pszEndContext = wcsstr( pszContext, L"</Context>" );

              if ( pszControlEnd ) pszAddInfo = wcsstr( pszControlEnd, L"<AddData>" );
              if ( pszAddInfo )    pszEndAddInfo = wcsstr( pszAddInfo, L"</AddData>" );


              if ( pszControlEnd ) pszSource = wcsstr( pszControlEnd, L"<Source>" );
              if ( pszSource)      pszEndSource = wcsstr( pszSource, L"</Source>" );
              if ( pszEndSource )  pszTarget = wcsstr( pszEndSource, L"<Target>" );
              if ( pszTarget )     pszEndTarget = wcsstr( pszTarget, L"</Target>" );

              if ( pszSource && pszEndSource && pszTarget && pszEndTarget )
              {
                pszSource += 8; // skip tag 
                pszTarget += 8; // skip tag
                *pszEndSource = 0;
                *pszEndTarget = 0;
              } /* endif */

              if ( pszContext && pszEndContext )
              {
                pszContext += 9; // skip tag 
                *pszEndContext = 0;
              }
              else
              {
                pszContext = NULL;
              } /* endif */

              if ( pszAddInfo && pszEndAddInfo )
              {
                pszAddInfo += 9; // skip tag 
                *pszEndAddInfo = 0;
              }
              else
              {
                pszAddInfo = NULL;
              } /* endif */
            }

            // write segment to internal memory
            {
              OtmProposal prop;

              WideCharToMultiByte( CP_OEMCP, 0, pCLMData->szControl, -1, pCLMData->szControlAscii, sizeof(pCLMData->szControlAscii), NULL, NULL );

              prop.setSource( pszSource );
              prop.setTarget( pszTarget );
              if ( pszContext ) prop.setContext( pszContext ); 
              if ( pszAddInfo ) prop.setAddInfo( pszAddInfo ); 

              prop.setDocShortName( UtlParseX15( pCLMData->szControlAscii, 7 ) );
              prop.setDocName( UtlParseX15( pCLMData->szControlAscii, 8 ) );
              prop.setSegmentNum( atol( UtlParseX15( pCLMData->szControlAscii, 0 ) ) );
              USHORT usTranslationFlag = (USHORT)atoi( UtlParseX15( pCLMData->szControlAscii, 1 ) );
              if ( usTranslationFlag == TRANSLFLAG_NORMAL )
              {
                prop.setType( OtmProposal::eptManual );
              }
              else if ( usTranslationFlag == TRANSLFLAG_MACHINE )
              {
                prop.setType( OtmProposal::eptMachine );
              }
              else if ( usTranslationFlag == TRANSLFLAG_GLOBMEM )
              {
                prop.setType( OtmProposal::eptGlobalMemory );
              }
              else if ( usTranslationFlag == TRANSLFLAG_GLOBMEMSTAR )
              {
                prop.setType( OtmProposal::eptGlobalMemoryStar );
              }
              else 
              {
                prop.setType( OtmProposal::eptUndefined );
              } /* endif */
              prop.setUpdateTime( atol( UtlParseX15( pCLMData->szControlAscii, 2 ) ) );
              prop.setMarkup( UtlParseX15( pCLMData->szControlAscii, 6 ) );
              prop.setSourceLanguage( UtlParseX15( pCLMData->szControlAscii, 3 ) );
              prop.setTargetLanguage( UtlParseX15( pCLMData->szControlAscii, 4 ) );

              usRC = (USHORT)pCLMData->pOutMem->putProposal( prop );
              if ( usRC ) LogRC( usRC, "putProposal" );
            }
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endfor */
    if ( pCLMData->fExternalMemory )
    {
      fputws( L"</NTMMemoryDb>\r\n", pCLMData->hOutFile );
    } /* endif */
  } /* endif */
  
  if ( pCLMData->hInFile ) 
  {
    fclose( pCLMData->hInFile );
    pCLMData->hInFile = NULL;
  } /* endif */

  if ( pCLMData->hOutFile ) 
  {
    fclose( pCLMData->hOutFile );
    pCLMData->hOutFile = NULL;
  } /* endif */

  if ( !pCLMData->fKeepDups )
  {
    UtlDelete( pCLMData->szTempMemory, 0L, NOMSG );
  } /* endif */

  if ( pCLMData->pOutMem != NULL )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    pFactory->closeMemory( pCLMData->pOutMem ); 
    pCLMData->pOutMem = NULL;
  } /* endif */

#ifdef CLEANMEMORYTIMELOG
  GetElapsedTime( &(pCLMData->lMemClearTime) );
#endif


  if ( pMatchTable ) UtlAlloc( (PVOID *)&pMatchTable, 0, 0, NOMSG );

  pCLMData->Task = CLM_STOP_TASK;

  return( usRC );
} /* end of function MemFuncCLMMemEnd */


// MemoryCleanup cleanup code
USHORT MemFuncCLMCleanup
(
  PMEM_CLEAN_MEMORY_DATA pCLMData
)
{
  USHORT usRC = 0;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  ULONG ulTest = pCLMData->ulSegsCopied++;
  ulTest = pCLMData->ulSegsSkipped++;

  if ( pCLMData->pMem != NULL )
  {
    pFactory->closeTempMemory( pCLMData->pMem ); 
    pCLMData->pMem = NULL;
  } /* endif */

  if ( pCLMData->pOutMem != NULL )
  {
    pFactory->closeMemory( pCLMData->pOutMem ); 
    pCLMData->pOutMem = NULL;
  } /* endif */

  if ( pCLMData->pDoc )
  {
    if ( pCLMData->pDoc->pDocTagTable ) TAFreeTagTable( (PLOADEDTABLE)pCLMData->pDoc->pDocTagTable  );
    SegFileFreeDoc( (PVOID *)&(pCLMData->pDoc) );
    pCLMData->pDoc = NULL;
  } /* endif */

  if ( pCLMData->pDocTable )
  {
    USHORT usI = 0;
    for( usI = 0; usI < pCLMData->usDocuments; usI++ )
    {
      if ( pCLMData->pDocTable[usI].pSegment ) 
      {
        UtlAlloc( (PVOID *)&(pCLMData->pDocTable[usI].pSegment), 0, 0, NOMSG );
      } /* endif */
    } /* endfor */
    UtlAlloc( (PVOID *)&(pCLMData->pDocTable), 0, 0, NOMSG );
  } /* endif */

  if ( pCLMData->hInFile )  fclose( pCLMData->hInFile );
  if ( pCLMData->hOutFile ) fclose( pCLMData->hOutFile );

  if ( pCLMData->pLoadedQFTable ) TAFreeTagTable( pCLMData->pLoadedQFTable );

  // free vector and proposal objects
  if ( pCLMData->pvMatchList != NULL )
  {
    for ( int i = 0; i < (int)pCLMData->pvMatchList->size(); i++ )
    {
      delete (*pCLMData->pvMatchList)[i];
    } /* endfor */         
    delete( pCLMData->pvMatchList );
  }


  pCLMData->Task = CLM_COMPLETED_TASK;

#ifdef CLEANMEMORYTIMELOG
  {
    FILE *hfLog = NULL;
    CHAR szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szLogFile, 0L, NOMSG );
    strcat( szLogFile, "\\EQFCLEANMEMORYTIME.LOG" );
    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      fprintf( hfLog, "**** EqfCleanMemory log ****\n" );
      fprintf( hfLog, "Document memory setup time %I64d [ms]\n", pCLMData->lMemFillTime );
      fprintf( hfLog, "Memory lookup time         %I64d [ms]\n", pCLMData->lMemLookupTime );
      fprintf( hfLog, "Memory output time         %I64d [ms]\n", pCLMData->lMemWriteTime );
      fprintf( hfLog, "Memory segment delete time %I64d [ms]\n", pCLMData->lMemDelTime );
      fprintf( hfLog, "Matchtable create and sort %I64d [ms]\n", pCLMData->lMemSortTime );
      fprintf( hfLog, "Read/Write Memory          %I64d [ms]\n", pCLMData->lMemClearTime );
      fprintf( hfLog, "Other times                %I64d [ms]\n", pCLMData->lMemOtherTime );
      fclose( hfLog );
    } /* endif */
  }
#endif


  return( usRC );
} /* end of function MemFuncCLMCleanup */


// helper function to check for and remove the given tag from the end of the string
BOOL MemStripTag( PSZ_W pszLine, PSZ_W pszTag )
{
  BOOL fFound = FALSE;
  PSZ_W pszTest = NULL;

  int  iLen = wcslen( pszLine );
  int  iTagLen = wcslen( pszTag );

  if ( iLen >= iTagLen )
  {
    pszTest = pszLine + (iLen - iTagLen);
  } /* endif */

  if ( pszTest )
  {
    fFound = ( wcsnicmp( pszTest, pszTag, iTagLen ) == 0 );

    if ( fFound )
    {
      *pszTest = 0;
    } /* endif */
  } /* endif */

  return( fFound );
} /* end of function MemStripTag */

static USHORT MemFillBufferW
(
  PMEM_CLEAN_MEMORY_DATA pCLMData
)
{
  USHORT      usRC = 0;
  PSZ_W       pTemp;
  int         iStillInBuf;
  ULONG       ulBytesRead = 0;

  if ( pCLMData->iInBufProcessed )
  {
    // refill buffer
    pTemp = pCLMData->chInBufW + pCLMData->iInBufProcessed;
    iStillInBuf = pCLMData->iInBufRead - pCLMData->iInBufProcessed;
    memmove( pCLMData->chInBufW, pTemp, iStillInBuf*sizeof(CHAR_W) );
    memset( &pCLMData->chInBufW[iStillInBuf], 0, (BUF_SIZE-iStillInBuf)*sizeof(CHAR_W));
    usRC = UtlReadL( pCLMData->hfIn, (pCLMData->chInBufW + iStillInBuf), (BUF_SIZE - iStillInBuf)*sizeof(CHAR_W) ,
                     &ulBytesRead, TRUE );

    pCLMData->iInBufProcessed = 0;
    pCLMData->iInBufRead = (ulBytesRead / sizeof(CHAR_W)) + iStillInBuf;
  }
  else
  {
    // fill buffer for the first time
    usRC = UtlReadL( pCLMData->hfIn, pCLMData->chInBufW, BUF_SIZE * sizeof(CHAR_W), &ulBytesRead, TRUE );

    pCLMData->iInBufRead = ulBytesRead / sizeof(CHAR_W);
  } /* endif */

  return( usRC );

} /* end of function MemFillBufferW */

static USHORT MemFillBuffer
(
  PMEM_CLEAN_MEMORY_DATA pCLMData
)
{
  USHORT      usRC = 0;
  PSZ         pTemp;
  int         iStillInBuf;
  ULONG       ulBytesRead = 0;

  if ( pCLMData->iInBufProcessed )
  {
    // refill buffer
    pTemp = pCLMData->chInBuf + pCLMData->iInBufProcessed;
    iStillInBuf = pCLMData->iInBufRead - pCLMData->iInBufProcessed;
    memmove( pCLMData->chInBuf, pTemp, iStillInBuf );
    memset( &pCLMData->chInBuf[iStillInBuf], 0, (BUF_SIZE-iStillInBuf) );
    usRC = UtlReadL( pCLMData->hfIn, (pCLMData->chInBuf + iStillInBuf), (BUF_SIZE - iStillInBuf),
                     &ulBytesRead, TRUE );

    pCLMData->iInBufProcessed = 0;
    pCLMData->iInBufRead = ulBytesRead + iStillInBuf;
  }
  else
  {
    // fill buffer for the first time
    usRC = UtlReadL( pCLMData->hfIn, pCLMData->chInBuf, BUF_SIZE, &ulBytesRead, TRUE);

    pCLMData->iInBufRead = ulBytesRead;
  } /* endif */

  return( usRC );

} /* end of function MemFillBuffer */


static USHORT MemReadLineW
(
  PMEM_CLEAN_MEMORY_DATA pCLMData,
  PSZ_W    pszLine,
  int      iSize
)
{
  USHORT      usRC = 0;
  PSZ_W       pLF;
  PSZ_W       pTemp;
  int         iLen = 0;

  // check if buffer can be refilled
  if ( pCLMData->iInBufRead == BUF_SIZE )
  {
      // there is more to read from infile (except it has exactly the size
      // of BUF_SIZE)
      if ( (pCLMData->iInBufProcessed * 2) > pCLMData->iInBufRead )
      {
          // half of buffer parsed, so refill it
          usRC = MemFillBufferW( pCLMData );
      } /* endif */
  } /* endif */

  pTemp = pCLMData->chInBufW + pCLMData->iInBufProcessed;

  // get one line out of buffer
  pLF = wcschr( pTemp, L'\n' );
  if ( pLF )
  {
      iLen = pLF - pTemp;
      if ( iLen > iSize )
      {
        iLen = iSize - 1;
      } /* endif */

      memcpy( pszLine, pTemp, iLen * sizeof(CHAR_W) );
      pszLine[iLen] = 0;
      if ( (iLen > 1) &&  pszLine[iLen-1] == L'\r' )
      {
          pszLine[iLen-1] = 0;
      } /* endif */

      pCLMData->iInBufProcessed += (iLen+1);
  }
  else
  {
      /******************************************************************/
      /* EOF reached...                                                 */
      /******************************************************************/
      usRC = BTREE_EOF_REACHED;
  } /* endif */

  return( usRC );

} /* end of function MemReadLineW */


static USHORT MemReadLine
(
  PMEM_CLEAN_MEMORY_DATA pCLMData,
  PSZ      pszLine,
  int      iSize
)
{
  USHORT      usRC = 0;
  PSZ         pLF;
  PSZ         pTemp;
  int         iLen = 0;

  // check if buffer can be refilled
  if ( pCLMData->iInBufRead == BUF_SIZE )
  {
      // there is more to read from infile (except it has exactly the size
      // of BUF_SIZE)
      if ( (pCLMData->iInBufProcessed * 2) > pCLMData->iInBufRead )
      {
          // half of buffer parsed, so refill it
          usRC = MemFillBuffer( pCLMData );
      } /* endif */
  } /* endif */

  pTemp = pCLMData->chInBuf + pCLMData->iInBufProcessed;

  // get one line out of buffer
  pLF = strchr( pTemp, '\n' );
  if ( pLF )
  {
      iLen = pLF - pTemp;
      if ( iLen > iSize )
      {
        iLen = iSize - 1;
      } /* endif */

      memcpy( pszLine, pTemp, iLen );
      pszLine[iLen] = 0;
      if ( (iLen > 1) &&  pszLine[iLen-1] == '\r' )
      {
          pszLine[iLen-1] = 0;
      } /* endif */

      pCLMData->iInBufProcessed += (iLen+1);
  }
  else
  {
      /******************************************************************/
      /* EOF reached...                                                 */
      /******************************************************************/
      usRC = BTREE_EOF_REACHED;
  } /* endif */

  return( usRC );

} /* end of function MemReadLine */


USHORT MemGetLine( PMEM_CLEAN_MEMORY_DATA pCLMData, PSZ_W pszLine, int iSize, BOOL fUnicode, PBOOL pfEOF  )
{
  USHORT usRC = 0;

  *pszLine = 0;
  if ( fUnicode )
  {
    usRC = MemReadLineW( pCLMData, pszLine, iSize );
    if ( usRC == BTREE_EOF_REACHED )
    {
      *pfEOF = TRUE;
      usRC = 0;
    } /* endif */
  }
  else
  {
    static CHAR szAsciiLine[8096];  

    szAsciiLine[0] = EOS;

    usRC = MemReadLine( pCLMData, szAsciiLine, sizeof(szAsciiLine) );
    if ( usRC == BTREE_EOF_REACHED )
    {
      *pfEOF = TRUE;
      usRC = 0;
    } /* endif */

    MultiByteToWideChar( pCLMData->ulInputCP, 0, szAsciiLine, -1, pszLine, iSize-1 );
  } /* endif */

  return( usRC );
}


#ifdef CLEANMEMORYTIMELOG

static LARGE_INTEGER liLast = { 0 };
static LARGE_INTEGER liFrequency = { 0 };

static void GetElapsedTime( LONG64 *plTime )
{
  LARGE_INTEGER liCurrent;

  QueryPerformanceCounter( &liCurrent );

  if ( liFrequency.QuadPart == 0)
  {
    QueryPerformanceFrequency( &liFrequency );
  } /* endif */

  if ( liLast.QuadPart != 0 )
  {
    if ( liFrequency.QuadPart != 0 )
    {
      LONGLONG ldwDiff = liCurrent.QuadPart - liLast.QuadPart;
      DWORD dwTime = (DWORD)((ldwDiff * 1000000) / liFrequency.QuadPart);
      *plTime = *plTime + dwTime;
    }
  } /* endif */
  liLast.QuadPart = liCurrent.QuadPart;
} /* end of function GetElapsedTime */

#endif

static void LogRC( USHORT usRC, PSZ pszFunction )
  {
    usRC; pszFunction;
#ifdef CLEANMEMORYRCLOG
    if ( usRC != 0 )
    {
      FILE *hfLog = NULL;
      CHAR szLogFile[MAX_EQF_PATH];

      UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
      UtlMkDir( szLogFile, 0L, NOMSG );
      strcat( szLogFile, "\\CLMRC.LOG" );

      hfLog = fopen( szLogFile, "a" );
      if ( hfLog )
      {
        LONG lCurTime = 0;  
        time( &lCurTime );
        fprintf( hfLog, "%sReturn code %u returned by function %s\n", asctime( localtime( &lCurTime ) ), usRC, pszFunction );
        fclose( hfLog );
      } /* endif */
    } /* endif */
#endif
#ifdef SESSIONLOG
    if ( usRC != 0 )
    {
      char szRC[10];
      sprintf( szRC, "%u", usRC );
      UtlLogWriteString2( "EqfCleanMemory: Return code %s returned by function %s", szRC, pszFunction );
    }
#endif
  return;
  }


