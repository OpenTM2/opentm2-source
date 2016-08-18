//+----------------------------------------------------------------------------+
//| OtmRemoveTags.C                                                            |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2015, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
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
#include <eqfserno.h>             // version number

#include <time.h>                 // C library for time functions
#include "EQFDDE.H"               // Batch mode definitions
#define INCL_EQFMEM_DLGIDAS       // include dialog IDA definitions
#include "EQFTMI.H"               // Private header file of Translation Memory
#include "EQFMEM.ID"              // PM IDs for Translation Memory
#include <EQFQDAM.H>              // Low level TM access functions
  #include "OTMFUNC.H"            // function call interface public defines
  #include "eqffunci.h"           // function call interface private defines
#include "EQFHLOG.H"              // for word count category limits
#include "OtmProposal.h"
#include "core\pluginmanager\OtmMemory.h"
#include "core\memory\MemoryFactory.h"

// name of internal memory for memory type conversions
#define REMOVETAG_MEM_NAME "____REMOVETAG_TEMP_MEMORY___"

// several constants used in the following code
#define TRUE  1
#define FALSE 0
#define NULC '\0'
#define WRONG_OPT 255

// size of token buffer
#define RMT_TOKBUFSIZE 32000

// some macros ...
#define SKIPSPACE(p)  while (*p == ' ' ) p++
#define SKIPCOMMA(p)  if (*p == ',' ) p++

char szMsg[2048];                      // buffer for TranslationManager messages
char szInMem[1024];                    // name of input memory 
char szOutMem[1024];                   // name of output memory 
char szTempMemIn[1024];                // name of temporary input memory 
char szTempMemOut[1024];               // name of temporary input memory 
char szLogFile[1024];                  // name of log file
char szLine[2048];                     // buffer for a single line of the script
char szTempLine[2048];                 // temporary buffer for a single line of the script
long lOption;                          // buffer for option parameters
char chNullParm = NULC;                // empty character parameter
char szMarkup[1024];                   // markup to use for tag recognition
char szNewMarkup[1024];                // new markup to use for created segments
BOOL fSpecialRTFProcessing = FALSE;    // TRUE = do not base tag removal on markup table but use hardocded RTF tag recognition


// prototypes
static BOOL CheckMemType( PSZ pszType, PLONG plType, BOOL fInternal );
static void showHelp();


// cleanmemory API stuff
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

typedef struct _REMOVE_TAG_DATA
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
  CHAR        szMemPath[MAX_LONGPATH];           // fully qualified TM name
  CHAR        szOutMemPath[MAX_LONGPATH];        // fully qualified output TM name
  CHAR        szOutMemPropPath[MAX_LONGPATH];    // fully qualified TM property file
  CHAR        szFolName[MAX_FILESPEC];           // folder short name
  CHAR        szFolLongName[MAX_LONGFILESPEC];   // long folder name
  CHAR        szBuffer[4096];                    // general purpose buffer
  USHORT      usComplete;                        // current completion rate
  ULONG       ulSegments;                        // number of segments added to archive TM
  SHORT       sCurDoc;                           // number of current document (within listbox)
  CHAR        szCurDoc[MAX_FILESPEC];            // name of current document
  OtmMemory   *pMem;                             // memory object being worked on
  OtmMemory   *pOutMem;                          // output memory object
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
  FILE        *hfOut;                            // output file
  BOOL        fAnsi;                             // TRUE = input in ASCII format / FALSE = input in ASCII mode
  ULONG       ulInputCP;                         // codepage to use for import when importing non-Unicode memory
  CHAR        szActiveTagTable[50];              // buffer for name of currently loaded markup table
  PLOADEDTABLE pLoadedTable;                     // pointer to currently loaded markup table
  PTOKENENTRY  pTokBuf;                          // buffer for TaTagTokenize tokens
  LONG         lInMemOpt;                        // options for input memory
  LONG         lOutMemOpt;                       // options for output memory
  ULONG        ulAnsiCP;                         // ANSI codepage
  ULONG        ulOemCP;                          // ASCII codepage
  HSESSION     hSession;
} REMOVE_TAG_DATA, *PREMOVETAGDATA;



// prototypes 
USHORT MemFuncCleanMemoryProcess( PFCTDATA pData );
USHORT MemFuncCLMNextDoc( PREMOVETAGDATA pData );
USHORT MemFuncCLMDocEnd( PREMOVETAGDATA pData );
USHORT MemFuncCLMDocProcess( PREMOVETAGDATA pData );
USHORT MemFuncCLMDocStart( PREMOVETAGDATA pData );
USHORT MemFuncCLMCleanup( PREMOVETAGDATA pData );
USHORT RemoveTagStart( PREMOVETAGDATA pData );    
USHORT MemFuncCLMStart( PREMOVETAGDATA pData );
USHORT RemoveTagProcess( PREMOVETAGDATA pData );
USHORT RemoveTagEnd( PREMOVETAGDATA pData );
BOOL MemStripTag( PSZ_W pszLine, PSZ_W pszTag );
USHORT MemGetLine( PREMOVETAGDATA pData, PSZ_W pszLine, int iSize, BOOL fUnicode, PBOOL pfEOF  );
static USHORT MemFillBufferW( PREMOVETAGDATA pData );
static USHORT MemFillBuffer( PREMOVETAGDATA pData );
USHORT WriteStringToMem( PREMOVETAGDATA pData, PSZ_W pszString );
// end of cleanmemory API stuff
USHORT OpenOrCreateOutMem( PREMOVETAGDATA pData );

void ShowTMMessage( PREMOVETAGDATA pData, HSESSION hSession, PSZ pszInfo )
{
  USHORT usRC = 0;
  EqfGetLastError( hSession, &usRC, pData->szBuffer, sizeof(pData->szBuffer) );
  printf( "ERRROR==> %s, TranslationManager returned an error:\n%s\n", pszInfo, pData->szBuffer );
}

int main
(
  int argc,
  char *argv[],
  char *envp[]
)
{
  HSESSION hSession = 0L;
  USHORT   usRC = 0;
  int fOK = TRUE;
  LONG lInType = -1;
  LONG lOutType = -1;
  PREMOVETAGDATA pData = NULL;
  BOOL fEOF = FALSE;
  LONG lSegments = 0;

  envp;

  if(argc==2 && stricmp(argv[1],"-h")==0)
  {
      showHelp();
      return 0;
  }
  // show logo
  printf( "REMOVETAGS - The tag removal tool\n\n" );

  // skip program name
  argc--;
  argv++;

  // process arguments
  szInMem[0] = '\0';
  szOutMem[0] = '\0';
  szNewMarkup[0] = '\0';
  szTempMemIn[0] = '\0';
  szTempMemOut[0] = '\0';
  szMarkup[0] = '\0';

  while ( argc )
  {
    PSZ pszParm = argv[0];
    if ( (*pszParm == '-') || (*pszParm == '/') )
    {
      if ( strnicmp( pszParm+1, "inmem=", 6 ) == 0 )
      {
        strcpy( szInMem, pszParm+7 );
      }
      else if ( strnicmp( pszParm+1, "outmem=", 7 ) == 0 )
      {
        strcpy( szOutMem, pszParm+8 );
      }
      else if ( strnicmp( pszParm+1, "outtype=", 8 ) == 0 )
      {
        fOK = CheckMemType( pszParm+9, &lOutType, TRUE );
      }
      else if ( strnicmp( pszParm+1, "intype=", 7 ) == 0 )
      {
        fOK = CheckMemType( pszParm+8, &lInType, FALSE );
      }
      else if ( strnicmp( pszParm+1, "markup=", 7 ) == 0 )
      {
        strcpy( szMarkup, pszParm+8 );
        if ( stricmp( szMarkup, "RTF"  ) == 0 )
        {
          fSpecialRTFProcessing = TRUE;
        } /* endif */           
      }
      else if ( strnicmp( pszParm+1, "newmarkup=", 10 ) == 0 )
      {
        strcpy( szNewMarkup, pszParm+11 );
      }
      else
      {
        printf( "WARNING==> unknown option \'%s\' is ignored\n", pszParm );
      } /* endif */
    }
    else
    {
      printf( "WARNING==> superfluos command line parameter \'%s\' is ignored\n", pszParm );
    } /* endif */
    argc--;
    argv++;
  } /* endwhile */

  // check for mandatory parameters
  if ( fOK )
  {
    if ( szInMem[0] == '\0' )
    {
      printf( "Error==> missing input memory specification\n" );
      fOK = FALSE;
      showHelp();
    }
    else if ( szOutMem[0] == '\0' )
    {
      printf( "Error==> missing output memory specification\n" );
      fOK = FALSE;
      showHelp();
    } 
    else if ( stricmp( szOutMem, szInMem ) == 0 )
    {
      printf( "Error==> name of output memory must be different from name of input memory\n" );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // use defaults for not supplied parameters
  if ( fOK )
  {
    if ( lInType == -1 )
    {
      printf( "Info==> no input memory format specified, using UTF16 as default\n" );
      lInType = UTF16_OPT;
    } /* endif */
    if ( lOutType == -1 )
    {
      printf( "Info==> no output memory format specified, using UTF16 as default\n" );
      lOutType = UTF16_OPT;
    } /* endif */
  } /* endif */

  // start the batch session
  if ( fOK )
  {
    usRC = EqfStartSession( &hSession );
    if ( usRC  )
    {
      fOK = FALSE;
      printf( "ERRROR==> could not start TranslationManager session, return code is %u\n", usRC );
    } /* endif */
  } /* endif */

  // allocate and prepare data area
  if ( fOK )
  {
    // allocate our data areas
    fOK = UtlAllocHwnd( (PVOID *)&pData, 0, sizeof(REMOVE_TAG_DATA), ERROR_STORAGE, HWND_FUNCIF );
    if ( fOK )
    {
      fOK = UtlAllocHwnd( (PVOID *)&(pData->pTokBuf), 0, RMT_TOKBUFSIZE, ERROR_STORAGE, HWND_FUNCIF );
    } /* endif */
    if ( !fOK )
    {
      printf( "ERRROR==> memory allocation failed, short on system storage?\n" );
    } /* endif */
  } /* endif */

  // setup  temporary memory name to use for TMX conversions
  if ( fOK )
  {
    strcpy( szTempMemIn, szOutMem );
    strcat( szTempMemIn, ".tempmemin" );
    strcpy( szTempMemOut, szOutMem );
    strcat( szTempMemOut, ".tempmemout" );
  } /* endif */

  // fill data area fields
  if ( fOK )
  {
    strcpy( pData->szInMemory, szInMem );
    strcpy( pData->szOutMemory, szOutMem );
    pData->lInMemOpt = lInType;
    pData->lOutMemOpt = lOutType;
    pData->ulOemCP = GetLangOEMCP(NULL);
    pData->ulAnsiCP = GetLangAnsiCP(NULL);
    strcpy( pData->szFolSourceLang, "English(U.S.)" );
    pData->hSession = hSession;
  } /* endif */

  // do any TMX preprocessing
  if ( fOK )
  {
    if ( lInType == TMX_OPT)
    {
      printf( "Info==> Converting TMX memory to TM format...\n" );

      {
        // avoid traps for not fully qualified file names
        if ( strchr( pData->szInMemory, BACKSLASH ) == NULL )
        {
          strcpy( pData->szInMemory, ".\\" );
          strcat( pData->szInMemory, szInMem );
        } /* endif */


        printf( "        Importing TMX memory...\n" );
        EqfDeleteMem( hSession, REMOVETAG_MEM_NAME );
        EqfCreateMem( hSession, REMOVETAG_MEM_NAME, "", NULC, pData->szFolSourceLang, 0 );

        do
        {
          usRC = EqfImportMem( hSession, REMOVETAG_MEM_NAME, pData->szInMemory, TMX_OPT | OVERWRITE_OPT );
        } while ( usRC == CONTINUE_RC );
        if ( usRC )
        {
          ShowTMMessage( pData, hSession, "Import of memory failed" );
          fOK = FALSE;
        } /* endif */
      }

      if ( fOK )
      {
        printf( "        Exporting memory in TM format...\n" );
        do
        {
          usRC = EqfExportMem( hSession, REMOVETAG_MEM_NAME, szTempMemIn, UTF16_OPT | OVERWRITE_OPT );
        } while ( usRC == CONTINUE_RC );
        if ( usRC )
        {
          ShowTMMessage( pData, hSession, "Export of memory failed" );
          fOK = FALSE;
        } /* endif */
      } /* endif */
      EqfDeleteMem( hSession, REMOVETAG_MEM_NAME );

      if ( fOK )
      {
        strcpy( pData->szInMemory, szTempMemIn );
        pData->lInMemOpt = UTF16_OPT;
      } /* endif */
    } /* endif */

    if ( lOutType == TMX_OPT)
    {
      strcpy( pData->szOutMemory, szTempMemOut );
      pData->lOutMemOpt = UTF16_OPT;
    } /* endif */
  } /* endif */

  // open input and output files
  if ( fOK )
  {
    USHORT      usOpenAction;            // dummy for UtlOpen

    // open input file
    usRC = UtlOpenHwnd( pData->szInMemory, &(pData->hfIn), &usOpenAction, 0L,
                        FILE_NORMAL, FILE_OPEN, OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                        0L, TRUE, HWND_FUNCIF );
    if ( usRC != 0 )
    {
      printf( "ERRROR==> Input memory %s failed to open.\n", pData->szInMemory );
      fOK = FALSE;
    } /* endif */

    // preprocess input file
    if ( fOK )
    {
      // skip any BOM in UTF16 memories and fill buffer
      if ( pData->lInMemOpt & UTF16_OPT )
      {
        BYTE chBOM[2];
        ULONG ulBytesRead = 0;

        UtlReadL( pData->hfIn, chBOM, 2, &ulBytesRead, TRUE );
    
        pData->fUnicode = TRUE;
        if ( memcmp( chBOM, UNICODEFILEPREFIX, 2 ) != 0 )
        {
          // reposition to begin of file
          ULONG ulFilePos = 0;
          UtlChgFilePtr( pData->hfIn, 0L, FILE_BEGIN, &ulFilePos, FALSE );
        } /* endif */

        usRC = MemFillBufferW( pData );
      }
      else
      {
        pData->fUnicode = FALSE;
        usRC = MemFillBuffer( pData );
      } /* endif */

      MemGetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF  );

      if ( wcsnicmp( pData->szLine, L"<ntmmemorydb>", 13 ) != 0 )
      {
        printf( "ERRROR==> File %s seems to be no valid memory or the input type has been specified incorrectly\n", pData->szInMemory );
        fOK = FALSE;
      } /* endif */
    } /* endif */

    // open output file or memory
    if ( fOK )
    {
      if ( pData->lOutMemOpt & CLEANMEM_INTERNAL_MEMORY_OPT )
      {
        OpenOrCreateOutMem( pData );
      }
      else
      {
        pData->hfOut = fopen( pData->szOutMemory, "wb" );
        if ( pData->hfOut == NULL )
        {
          printf( "ERRROR==> Output memory %s could not be created.\n", pData->szOutMemory );
          fOK = FALSE;
        } /* endif */

        // write BOM
        if ( fOK && (pData->lOutMemOpt & UTF16_OPT) )
        {
          fwrite( UNICODEFILEPREFIX, 1, 2, pData->hfOut );
        } /* endif */
      } /* endif */
    } /* endif */

    // read/write memory header up to first segment
    if ( fOK )
    {
      while ( !fEOF && 
              (wcsnicmp( pData->szLine, L"<segment>", 9 ) != 0) &&
              (wcsnicmp( pData->szLine, L"</ntmmemorydb>", 14 ) != 0))
      {
        if ( !(pData->lOutMemOpt & CLEANMEM_INTERNAL_MEMORY_OPT) )
        {
          WriteStringToMem( pData, pData->szLine ); 
          WriteStringToMem( pData, L"\r\n" ); 
        } /* endif */
        pData->szLine[0] = 0;
        MemGetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
      } /*endwhile */
    } /* endif */

  } /* endif */

  // process memory segments
  if ( fOK )
  {
    printf( "Info==> Removing tagging from memory segments...\n" );
    do
    {
      usRC = RemoveTagProcess( pData );
      if ( !usRC )
      {
        lSegments++;
      } /* endif */
    } while ( !usRC && (pData->Task == CLM_PROCESS_MEM_TASK) );

    if ( usRC != 0 )
    {
      ShowTMMessage( pData, hSession, "Tag removal failed" );
      fOK = FALSE;
    }
    else
    {
      printf( "Info==> Tag removal complete, %ld segments processed\n", lSegments  );
    } /* endif */

    // close files
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    if ( pData->hfIn )  UtlClose( pData->hfIn, FALSE );
    if ( pData->hfOut ) fclose( pData->hfOut );
    if ( pData->pOutMem != NULL ) pFactory->closeMemory( pData->pOutMem );
    pData->pOutMem = NULLHANDLE;
    pData->hfOut = NULL;
    pData->hfIn = 0;
  } /* endif */

  // do any TMX postprocessing
  if ( fOK )
  {
    if ( lOutType & TMX_OPT )
    {
      printf( "Info==> Converting memory to TMX format...\n" );

      {
        printf( "        Importing memory...\n" );
        EqfDeleteMem( hSession, REMOVETAG_MEM_NAME );
        EqfCreateMem( hSession, REMOVETAG_MEM_NAME, "", NULC, pData->szFolSourceLang, 0 );

        do
        {
          usRC = EqfImportMem( hSession, REMOVETAG_MEM_NAME, szTempMemOut, UTF16_OPT | OVERWRITE_OPT );
        } while ( usRC == CONTINUE_RC );
        if ( usRC )
        {
          ShowTMMessage( pData, hSession, "Import of memory failed" );
          fOK = FALSE;
        } /* endif */
      }

      if ( fOK )
      {
        printf( "        Exporting memory in TMX format...\n" );
        do
        {
          usRC = EqfExportMem( hSession, REMOVETAG_MEM_NAME, szOutMem, TMX_UTF16_OPT | OVERWRITE_OPT );
        } while ( usRC == CONTINUE_RC );
        if ( usRC )
        {
          ShowTMMessage( pData, hSession, "Export of memory failed" );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      EqfDeleteMem( hSession, REMOVETAG_MEM_NAME );
      if ( fOK )
      {
        printf( "        done!\n" );
      } /* endif */
    } /* endif */
  } /* endif */

  // show completion message
  if ( fOK )
  {
    printf( "Info==> Processing complete\n" );
  } /* endif */

  // cleanup
  if ( pData )
  {
    if ( pData->pTokBuf )
    {
      UtlAllocHwnd( (PVOID *)&(pData->pTokBuf), 0, 0, NOMSG, HWND_FUNCIF );
    } /* endif */
    UtlAllocHwnd( (PVOID *)&pData, 0, 0, NOMSG, HWND_FUNCIF );
  } /* endif */

  if ( hSession != 0L )
  {
    EqfEndSession( hSession );
  }

  if ( szTempMemIn[0] )  UtlDelete( szTempMemIn, 0, FALSE );
  if ( szTempMemOut[0] ) UtlDelete( szTempMemOut, 0, FALSE );

  return( 0 );
} /* end of main */

// check for valid memory type
static BOOL CheckMemType( PSZ pszType, PLONG plType, BOOL fInternal )
{
  BOOL fOK = TRUE;

  *plType = 0;

  if ( stricmp( pszType, "ASCII" ) == 0 )
  {
    *plType = ASCII_OPT;
  }
  else if ( stricmp( pszType, "ANSI" ) == 0 )
  {
    *plType = ANSI_OPT;
  }
  else if ( stricmp( pszType, "UTF16" ) == 0 )
  {
    *plType = UTF16_OPT;
  }
  else if ( stricmp( pszType, "TMX" ) == 0 )
  {
    *plType = TMX_OPT;
  }
  else if ( fInternal && (stricmp( pszType, "INTERNAL" ) == 0) )
  {
    *plType = CLEANMEM_INTERNAL_MEMORY_OPT;
  }
  else
  {
    fOK = FALSE;
    if ( fInternal )
    {
      printf( "ERRROR==> %s is no valid memory type, valid types are ASCII, ANSI, UTF16, TMX, and INTERNAL\n", pszType );
    }
    else
    {
      printf( "ERRROR==> %s is no valid memory type, valid types are ASCII, ANSI, UTF16, and TMX\n", pszType );
    } /* endif */
  } /* endif */

  return( fOK );
}

static void showHelp()
{
	//printf( "Error: No input memory specified\n\n" );
    printf( "OtmRemoveTags.EXE  : Inline tag removal tool\n" );
    printf( "Version               : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright             : %s\n",STR_COPYRIGHT );
    printf( "Purpose               : Remove inline tagging from segments in an external memory\n" );
    printf( "Syntax format         : OTMREMOVETAGS /INMEM=inmem /INTYPE=intype /OUTMEM=outmem /OUTTYPE=outtype [/MArkup=markup]\n" );
    printf( "Options and parameters:\n" );
    printf( "    /INMEM    the fully qualified name of the input memory\n" );
    printf( "    /INTYPE   the type of the input memory and can be \n" );
    printf( "              one of the values ASCII, ANSI, UTF16 or TMX\n" );
    printf( "    /OUTMEM   he fully qualified name of the output memory\n" );
    printf( "              name of the internal memory for /OUTTYPE=INTERNAL\n" );
    printf( "    /OUTTYPE  the type of the output memory and can be\n" );
    printf( "              one of the values ASCII, ANSI, UTF16, TMX, or INTERNAL\n" );
    printf( "    /MArkup   the markup to use for tag recognition or RTF for the\n" );
    printf( "              or the keywowrd RTF for the removal of pure RTF tags\n" );
    printf( "              if parameter is omitted the markup of the segments is used\n" );
}

// code borrowed and modified from eqfcleanmemory API 

static USHORT MemFillBufferW
(
  PREMOVETAGDATA pData
)
{
  USHORT      usRC = 0;
  PSZ_W       pTemp;
  int         iStillInBuf;
  ULONG       ulBytesRead = 0;

  if ( pData->iInBufProcessed )
  {
    // refill buffer
    pTemp = pData->chInBufW + pData->iInBufProcessed;
    iStillInBuf = pData->iInBufRead - pData->iInBufProcessed;
    memmove( pData->chInBufW, pTemp, iStillInBuf*sizeof(CHAR_W) );
    memset( &pData->chInBufW[iStillInBuf], 0, (BUF_SIZE-iStillInBuf)*sizeof(CHAR_W));
    usRC = UtlReadL( pData->hfIn, (pData->chInBufW + iStillInBuf), (BUF_SIZE - iStillInBuf)*sizeof(CHAR_W) ,
                     &ulBytesRead, TRUE );

    pData->iInBufProcessed = 0;
    pData->iInBufRead = (ulBytesRead / sizeof(CHAR_W)) + iStillInBuf;
  }
  else
  {
    // fill buffer for the first time
    usRC = UtlReadL( pData->hfIn, pData->chInBufW, BUF_SIZE * sizeof(CHAR_W), &ulBytesRead, TRUE );

    pData->iInBufRead = ulBytesRead / sizeof(CHAR_W);
  } /* endif */

  return( usRC );

} /* end of function MemFillBufferW */

static USHORT MemFillBuffer
(
  PREMOVETAGDATA pData
)
{
  USHORT      usRC = 0;
  PSZ         pTemp;
  int         iStillInBuf;
  ULONG       ulBytesRead = 0;

  if ( pData->iInBufProcessed )
  {
    // refill buffer
    pTemp = pData->chInBuf + pData->iInBufProcessed;
    iStillInBuf = pData->iInBufRead - pData->iInBufProcessed;
    memmove( pData->chInBuf, pTemp, iStillInBuf );
    memset( &pData->chInBuf[iStillInBuf], 0, (BUF_SIZE-iStillInBuf) );
    usRC = UtlReadL( pData->hfIn, (pData->chInBuf + iStillInBuf), (BUF_SIZE - iStillInBuf),
                     &ulBytesRead, TRUE );

    pData->iInBufProcessed = 0;
    pData->iInBufRead = ulBytesRead + iStillInBuf;
  }
  else
  {
    // fill buffer for the first time
    usRC = UtlReadL( pData->hfIn, pData->chInBuf, BUF_SIZE, &ulBytesRead, TRUE);

    pData->iInBufRead = ulBytesRead;
  } /* endif */

  return( usRC );

} /* end of function MemFillBuffer */


static USHORT MemReadLineW
(
  PREMOVETAGDATA pData,
  PSZ_W    pszLine,
  int      iSize
)
{
  USHORT      usRC = 0;
  PSZ_W       pLF;
  PSZ_W       pTemp;
  int         iLen = 0;

  // check if buffer can be refilled
  if ( pData->iInBufRead == BUF_SIZE )
  {
      // there is more to read from infile (except it has exactly the size
      // of BUF_SIZE)
      if ( (pData->iInBufProcessed * 2) > pData->iInBufRead )
      {
          // half of buffer parsed, so refill it
          usRC = MemFillBufferW( pData );
      } /* endif */
  } /* endif */

  pTemp = pData->chInBufW + pData->iInBufProcessed;

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

      pData->iInBufProcessed += (iLen+1);
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
  PREMOVETAGDATA pData,
  PSZ      pszLine,
  int      iSize
)
{
  USHORT      usRC = 0;
  PSZ         pLF;
  PSZ         pTemp;
  int         iLen = 0;

  // check if buffer can be refilled
  if ( pData->iInBufRead == BUF_SIZE )
  {
      // there is more to read from infile (except it has exactly the size
      // of BUF_SIZE)
      if ( (pData->iInBufProcessed * 2) > pData->iInBufRead )
      {
          // half of buffer parsed, so refill it
          usRC = MemFillBuffer( pData );
      } /* endif */
  } /* endif */

  pTemp = pData->chInBuf + pData->iInBufProcessed;

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

      pData->iInBufProcessed += (iLen+1);
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


USHORT MemGetLine( PREMOVETAGDATA pData, PSZ_W pszLine, int iSize, BOOL fUnicode, PBOOL pfEOF  )
{
  USHORT usRC = 0;

  *pszLine = 0;
  if ( fUnicode )
  {
    usRC = MemReadLineW( pData, pszLine, iSize );
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

    usRC = MemReadLine( pData, szAsciiLine, sizeof(szAsciiLine) );
    if ( usRC == BTREE_EOF_REACHED )
    {
      *pfEOF = TRUE;
      usRC = 0;
    } /* endif */

    MultiByteToWideChar( pData->ulInputCP, 0, szAsciiLine, -1, pszLine, iSize-1 );
  } /* endif */

  return( usRC );
}

// remove iline tagging of supplied text
static USHORT RemoveTags( PREMOVETAGDATA pData, PSZ_W pszData, ULONG ulCP )
{
  USHORT usRC = 0;

  if ( fSpecialRTFProcessing )
  {
    // use hard-coded RTF tag processing
    PSZ_W  pszOut = pszData;

    while ( *pszData )
    {
      if ( *pszData == L'\\' )
      {
        if ( *(pszData+1) == L'\\' )
        {
          // encoded backslash, convert to a single one
          pszData++;
          *pszOut++ = *pszData++;
        }
        if ( (*(pszData+1) == L'{') || (*(pszData+1) == L'}') )
        {
          // encoded curly brace, convert to a normal one
          pszData++;
          *pszOut++ = *pszData++;
        }
        else if ( iswalpha(*(pszData+1)) || (*(pszData+1) == L'*') )
        {
          // skip RTF tag
          pszData++;
          if ( *pszData == L'*' ) pszData++;
          if ( *pszData == L'\\' ) pszData++;
          while ( iswalpha( *pszData ) ) pszData++;
          if ( *pszData == L'-') pszData++;
          while ( iswdigit( *pszData ) ) pszData++;
          if ( *pszData == L' ') pszData++;
        }
        else
        {
          // copy backslash to target
          *pszOut++ = *pszData++;
        } /* endif */
      }
      else if ( (*pszData == L'{') || (*pszData == L'}') )
      {
        // skip curly brace
        pszData++;
      }
      else
      {
        *pszOut++ = *pszData++;
      } /* endif */
    }/* endwhile */
    *pszOut = 0;
  } 
  else
  {
    // use markup table driven tag recognition
    PBYTE  pStartStop = NULL;

    // build protect start/stop table for tag recognition
    usRC = TACreateProtectTableW( pszData, pData->pLoadedTable, 0, (PTOKENENTRY)pData->pTokBuf, RMT_TOKBUFSIZE,
                                  (PSTARTSTOP *)&pStartStop, pData->pLoadedTable->pfnProtTable, 
                                  pData->pLoadedTable->pfnProtTableW, ulCP );
    if ( !usRC )
    {
      PSTARTSTOP pEntry = (PSTARTSTOP)pStartStop;
      PSZ_W pszTarget = pszData;
      while ( (pEntry->usStart != 0) || (pEntry->usStop != 0)  || (pEntry->usType != 0) )
      {
        switch ( pEntry->usType )
        {
          case UNPROTECTED_CHAR :
            // copy translatable text
            {
              USHORT usPos = pEntry->usStart; 
              while ( usPos <= pEntry->usStop )
              {
                *pszTarget++ = pszData[usPos++];
              } /*endwhile */
            }
            break;
          default :
            // ignore not-translatable data
            break;
        } /* endswitch */
        pEntry++;
      } /* endwhile */
      *pszTarget = 0;
    } /* endif */

    UtlAlloc( (PVOID *)&pStartStop, 0L, 0L, NOMSG );
  } /* endif */     

  return( usRC );
}

// write a string to the output memory
USHORT WriteStringToMem
(
  PREMOVETAGDATA pData,
  PSZ_W          pszString
)
{
  USHORT usRC = 0;

  if ( pData->lOutMemOpt & UTF16_OPT )
  {
    fputws( pszString, pData->hfOut );
  }
  else if ( pData->lOutMemOpt & ANSI_OPT )
  {
    LONG lRC = 0;
    ULONG ulLen = UtlDirectUnicode2AnsiBuf( pszString, pData->szBuffer, wcslen(pszString),
                                           sizeof(pData->szBuffer), pData->ulAnsiCP, TRUE, &lRC );
    pData->szBuffer[ulLen] = EOS;
    fputs( pData->szBuffer, pData->hfOut );
  }
  else if ( pData->lOutMemOpt & ASCII_OPT )
  {
    ULONG ulLen = Unicode2ASCIIBuf( pszString, pData->szBuffer, wcslen(pszString),
                                           sizeof(pData->szBuffer), pData->ulOemCP );
    pData->szBuffer[ulLen] = EOS;
    fputs( pData->szBuffer, pData->hfOut );
  } /* endif */
  
  return( usRC );
}

// MemoryCleanup memory read process
USHORT RemoveTagProcess
(
  PREMOVETAGDATA pData
)
{
  USHORT usRC = 0;
  BOOL fEOF = FALSE;

  // read data until segment is complete
  if ( wcsnicmp( pData->szLine, L"<segment>", 9 ) == 0 ) 
  {
    // reset segment data
    pData->szSource[0] = 0;
    pData->szControl[0] = 0;
    pData->szTarget[0] = 0;
    wcscpy( pData->szSegStart, pData->szLine );

    while ( !usRC && !fEOF && wcsnicmp( pData->szLine, L"</segment>", 10 ) != 0 ) 
    {
      MemGetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );

      if ( wcsnicmp( pData->szLine, L"<control>", 9 ) == 0 )
      {
        // get the control string
        MemGetLine( pData, pData->szControl, sizeof(pData->szControl), pData->fUnicode, &fEOF );

        if ( fEOF )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
        }
        else
        {
          // get contol string end tag and check it
          MemGetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF  );

          // todo: check for </control>
        } /* endif */

      }
      else if ( wcsnicmp( pData->szLine, L"<source>", 8 ) == 0  )
      {
        // get the segment source
        BOOL fEnd = MemStripTag( pData->szLine + 8, L"</source>" );
        wcscat( pData->szSource, pData->szLine + 8 );

        while ( !fEnd && !fEOF )
        {
          MemGetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
          fEnd = MemStripTag( pData->szLine, L"</source>" );
          wcscat( pData->szSource, L"\r\n" ); 
          wcscat( pData->szSource, pData->szLine );
        } /*endwhile */
        if ( !fEnd )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
        } /* endif */
      }
      else if ( wcsnicmp( pData->szLine, L"<target>", 8 ) == 0 )
      {
        // get the segment target
        BOOL fEnd = MemStripTag( pData->szLine + 8, L"</target>" );
        wcscat( pData->szTarget, pData->szLine + 8 );

        while ( !fEnd && !fEOF )
        {
          MemGetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
          fEnd = MemStripTag( pData->szLine, L"</target>" );
          wcscat( pData->szTarget, L"\r\n" ); 
          wcscat( pData->szTarget, pData->szLine );
        } /*endwhile */
        if ( !fEnd )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
        } /* endif */
      } /* endif */
    } /*endwhile */

    // read next line
    MemGetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );

    if ( usRC )
    {
      PSZ pszParms[2];

      pszParms[0] = "n/a";
      pszParms[1] = pData->szInMemory;

      UtlErrorHwnd( ERROR_INVALID_SEGMENT, MB_CANCEL, 2, pszParms, EQF_ERROR, HWND_FUNCIF);
    } /* endif */
  } /* endif */

  // remove tagging of segment 
  if ( !usRC && pData->szControl[0] && (pData->szSource[0] || pData->szTarget[0]) )
  {
    PSZ pszLanguage, pszMarkup;

    // access language and markup in control record
    WideCharToMultiByte( CP_OEMCP, 0, pData->szControl, -1, pData->szControlAscii, sizeof(pData->szControlAscii), NULL, NULL );
    pszLanguage = UtlParseX15( pData->szControlAscii, 4 );
    pszMarkup = UtlParseX15( pData->szControlAscii, 6 );

    if ( !fSpecialRTFProcessing  )
    {
      if ( szMarkup[0] )
      {
        pszMarkup = szMarkup;
      } /* endif */

      // get tag table
      if ( strcmp( pData->szActiveTagTable, pszMarkup ) != 0 )
      {
        // free loaded tag table
        if ( pData->pLoadedTable )
        {
          TAFreeTagTable( pData->pLoadedTable );
          pData->pLoadedTable = NULL;
        } /* endif */

        // load document tag table
        usRC = TALoadTagTableExHwnd( pszMarkup, (PLOADEDTABLE *)&(pData->pLoadedTable), FALSE, 
                                    TALOADUSEREXIT | TALOADPROTTABLEFUNC, FALSE, NULLHANDLE );
      } /* endif */
    } /* endif */       

    // process source segment
    if ( !usRC )
    {
	  if(pData->szSource[0])
          usRC = RemoveTags( pData, pData->szSource, pData->ulOemCP );

      if ( !usRC&&pData->szTarget[0] ) 
		  RemoveTags( pData, pData->szTarget, pData->ulOemCP );
    } /* endif */
  } /* endif */

  // write to output memory 
  if ( !usRC )
  {
    // write segment
    if ( pData->lOutMemOpt & CLEANMEM_INTERNAL_MEMORY_OPT )
    {
      OtmProposal Prop;

      WideCharToMultiByte( CP_OEMCP, 0, pData->szControl, -1, pData->szControlAscii, sizeof(pData->szControlAscii), NULL, NULL );

      Prop.setSource( pData->szSource );
      Prop.setTarget( pData->szTarget );
      Prop.setDocShortName( UtlParseX15( pData->szControlAscii, 7 ) );
      Prop.setDocName( UtlParseX15( pData->szControlAscii, 8 ) );
      Prop.setSegmentNum( atol( UtlParseX15( pData->szControlAscii, 0 ) ) );
      USHORT usTranslationFlag = (USHORT)atoi( UtlParseX15( pData->szControlAscii, 1 ) );
      switch ( usTranslationFlag )
      {
        case 1: Prop.setType( OtmProposal::eptMachine ); break;
        case 2: Prop.setType( OtmProposal::eptGlobalMemory ); break;
        default: Prop.setType( OtmProposal::eptManual ); break;
      }
      Prop.setUpdateTime( atol( UtlParseX15( pData->szControlAscii, 2 ) ) );
      Prop.setMarkup( UtlParseX15( pData->szControlAscii, 6 ) );
      Prop.setSourceLanguage( UtlParseX15( pData->szControlAscii, 3 ) );
      Prop.setTargetLanguage( UtlParseX15( pData->szControlAscii, 4 ) );

      pData->pOutMem->putProposal( Prop );
    }
    else
    {
      WriteStringToMem( pData, pData->szSegStart );
      WriteStringToMem( pData, L"\r\n" ); 
      WriteStringToMem( pData, L"<Control>\r\n" );
      WriteStringToMem( pData, pData->szControl );
      WriteStringToMem( pData, L"\r\n" ); 
      WriteStringToMem( pData, L"</Control>\r\n" );
      WriteStringToMem( pData, L"<Source>" );
      WriteStringToMem( pData, pData->szSource );
      WriteStringToMem( pData, L"</Source>\r\n" );
      WriteStringToMem( pData, L"<Target>" );
      WriteStringToMem( pData, pData->szTarget );
      WriteStringToMem( pData, L"</Target>\r\n" );
      WriteStringToMem( pData, L"</Segment>\r\n" );
    } /* endif */
  } /* endif */
  
  // skip empy lines
  while ( !fEOF && (pData->szLine[0] == 0) )
  {
    MemGetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
  } /*endwhile */

  if ( usRC )
  {
    pData->fErrorStop = TRUE;
    pData->Task = CLM_STOP_TASK;
  }
  else if ( wcsnicmp( pData->szLine, L"<segment>", 9 ) == 0 ) 
  {
    pData->Task = CLM_PROCESS_MEM_TASK;
  }
  else if ( fEOF || (wcsnicmp( pData->szLine, L"</ntmmemorydb>", 14 ) == 0) )
  {
    if ( pData->lOutMemOpt & CLEANMEM_INTERNAL_MEMORY_OPT )
    {
    }
    else
    {
      WriteStringToMem( pData, L"</NTMMemoryDb>\r\n" );
    } /* endif */
    pData->Task = CLM_END_MEM_TASK;
  }
  else
  {
    PSZ pszParm = pData->szInMemory;
    UtlErrorHwnd( ERROR_SGML_TAG, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF);
    usRC = ERROR_SGML_TAG;
    pData->fErrorStop = TRUE;
    pData->Task = CLM_STOP_TASK;
  } /* endif */

  return( usRC );

} /* end of function RemoveTagProcess */

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

USHORT OpenOrCreateOutMem
(
  PREMOVETAGDATA pData
)
{
  BOOL fIsNew = FALSE;

  // checks also for shared objects on the supplied target drive
  UtlMakeEQFPath( pData->szOutMemPath, NULC, MEM_PATH, NULL );
  ObjLongToShortName( pData->szOutMemory, pData->szOutMemShortName, TM_OBJECT, &fIsNew );

  // create new memory when required
  if ( fIsNew )
  {
    EqfCreateMem( pData->hSession, pData->szOutMemory , "", NULC, pData->szFolSourceLang, 0 );
  } /* endif */

  // open the memory
  MemoryFactory *pFactory = MemoryFactory::getInstance();
  int iRC = 0;
  pData->pOutMem = pFactory->openMemory( NULL, pData->szOutMemory, NONEXCLUSIVE, &iRC );

  return( (USHORT)iRC );
}
