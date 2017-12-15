//+----------------------------------------------------------------------------+
//| EQFPRSNO.C                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Implements the MemFuncProcessNomatch function                 |
//|              which reads a SNOMATCH file and runs the segments against     |
//|              a NLV memory, matching segments are written to an output      |
//|              memory and word count reports are created                     |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_MORPH            // morphologic functions
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
#include "eqfwcnti.h"             // Private include file for wordcount

#include "OtmProposal.h"
#include "core\memory\MemoryFactory.h"

#ifdef _DEBUG
   // define the following to write test data to I:\\EQF\\LOGS\\NOMATCH.LOG
  // #define TEST_OUTPUT
  // #define SESSIONLOG
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  MemFuncProcessSnomatch API
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// data types 

// process phases
typedef enum _PRNOM_TASKS
{
  PRNOM_START_TASK,
  PRNOM_START_NOMATCH_TASK,
  PRNOM_PROCESS_NOMATCH_TASK,
  PRNOM_END_NOMATCH_TASK,
  PRNOM_REPORT_TASK,
  PRNOM_STOP_TASK,
  PRNOM_COMPLETED_TASK
} PRNOM_TASKS;

// values for sNomatchMode 
#define NOMATCH_WILDCARD  1
#define NOMATCH_FILE      2
#define NOMATCH_DIRECTORY 3
#define NOMATCH_FOLDER    4


// size of internal buffers
#define BUF_SIZE 16000

typedef struct _MEM_PROCNOMATCH_DATA
{
  CHAR        szNoMatchFile[MAX_LONGFILESPEC];   // fully qualified name of current NOMATCH file
  CHAR        szInMemory[MAX_LONGFILESPEC];      // fully qualified name of external input memory (encoding: UTF-16)
  CHAR        szOutMemory[MAX_LONGFILESPEC];     // name of internal output memory or fully qualified name of external output memory
  CHAR        szMemMatchReportText[MAX_LONGFILESPEC];   // fully qualified file name of the memory match count report (text format)
  CHAR        szMemMatchReportXml[MAX_LONGFILESPEC];    // fully qualified file name of the memory match count report (XML format)
  CHAR        szDupReportText[MAX_LONGFILESPEC];        // fully qualified file name of the duplicate word count report (text format)
  CHAR        szDupReportXml[MAX_LONGFILESPEC];         // fully qualified file name of the duplicate word count report (XML format)
  CHAR        szOutNomatchXml[MAX_LONGFILESPEC]; // fully qualified file name of the output nomatch file (XML format)
  CHAR        szOutNomatchExp[MAX_LONGFILESPEC]; // fully qualified file name of the output nomatch file (EXP format)
  HFILE       hfMemMatchReportText;              // file handle of memory match report file (text)
  HFILE       hfMemMatchReportXml;               // file handle of memory match report file(XML)
  HFILE       hfDupReportText;                   // file handle of duplicate word count report file (text)
  HFILE       hfDupReportXml;                    // file handle of duplicate word count report file (XML)
  FILE        *hfOutNomatchXml;                  // file handle of nomatch output file (XML)
  FILE        *hfOutNomatchExp;                  // file handle of nomatch output file (EXP)
  LONG        lOptions;                          // options specified for this function
  PRNOM_TASKS Task;                              // current task
  BYTE        bReadBuffer[16000];                // file read/write buffer (must be large enough to contain complete memory segment)
  CHAR_W      chInBufW[BUF_SIZE];                // data buffer for read of Unicode data
  CHAR        chInBuf[BUF_SIZE];                 // data buffer for read of ANSI data
  HFILE       hfIn;                              // input file
  CHAR        szNomatchSearch[MAX_LONGFILESPEC]; // fully qualified search pattern for Nomatch files
  CHAR        szCurNomatch[MAX_LONGFILESPEC];    // fully qualified name of current NOMATCH file
  SHORT       sNomatchMode;                      // mode of nomatch specification
  OBJNAME     szFolObjName;                      // buffer for folder object name
  OBJNAME     szMemObjName;                      // buffer for memory object name
  OBJNAME     szTempObjName;                     // buffer for temporary object name
  CHAR        szTempMemory[MAX_LONGFILESPEC];    // name of temporary memory
  CHAR        szBuffer[4096];                    // general purpose buffer
  OtmMemory   *pInMem;                           // input memory object
  OtmMemory   *pOutMem;                          // output memory object
  OtmMemory   *pTempMem;                         // tempory memory object
  CHAR_W      szLine[4096];                      // buffer for input line
  CHAR_W      szTempBuffer[4096];                // general purpose buffer
  CHAR_W      szSegStart[1024];                  // buffer for segment start string
  CHAR_W      szControl[1024];                   // buffer for control string
  CHAR        szControlAscii[1024];              // ASCII version of control string
  CHAR_W      szSource[4096];                    // buffer for segment source
  CHAR_W      szTarget[4096];                    // buffer for segment target
  BOOL        fUnicode;                          // TRUE = input memory is in UTF-16 format
  BOOL        fErrorStop;                        // TRUE = process stopped by an error
  WIN32_FIND_DATA FileFindData;                  // buffer for file data 
  HANDLE      hDirNomatch;                       // file find handle for nomatch files
  int         iInBufProcessed;                   // number of processed characters in chInBuf
  int         iInBufRead;                        // number characters read into chInBuf
  ULONG       ulOEMCP;                           // OEM code page for folder source language
  ULONG       ulInputCP;                         // codepage to use for import when importing non-Unicode memory
  CHAR        szCurTagTable[MAX_FILESPEC];       // name of currently loaded tag table
  PLOADEDTABLE pTagTable;                        // pointer to currently loaded tag table
  BYTE         bTokBuffer[44000];                // buffer for toknization (may contain up to 2048 tokens with alenght of 18 bytes)
  SHORT        sLanguage;                        // Morph ID of current source language 
  CHAR         szCurLanguage[MAX_LONGFILESPEC];  // current language
  ULONG        ulOemCP;                          // OEM (ASCII) codepage for current language
  PCNTIDA      pCNTIda;                          // IDA for wordcount reports calls
  USHORT       usNumOfFiles;                     // number of files to process
  CHAR        szFolName[MAX_FILESPEC];           // folder short name
  CHAR        szFolLongName[2*MAX_LONGFILESPEC]; // long folder name
  LCOUNT       lFileCount;                       // word counts of current file
  LCOUNT       lSumCount;                        // word count summary
  LCOUNT       lFileDupCount;                    // duplicate word counts of current file
  LCOUNT       lSumDupCount;                     // duplicate word count summary
  CHAR         szLanguage[MAX_LONGFILESPEC];     // memory source language
  CHAR         szDocFormat[MAX_FNAME];            // name of document markup table
  CHAR         szDocSourceLang[MAX_LANG_LENGTH];  // document source language
  CHAR         szDocTargetLang[MAX_LANG_LENGTH];  // document target language
  CHAR         szDocName[MAX_LONGFILESPEC];       // document short name
  CHAR         szDocLongName[MAX_LONGFILESPEC];   // document long name
  BYTE         bUTF8Buffer[16000];                // buffer for strings converted to UTF-8 encoding
  CHAR_W       szUTF16Buffer[16000];              // buffer for UTF16 strings
  CHAR         szCurDoc[MAX_LONGFILESPEC];        // name of currently processed document 
  BOOL         fOutNomatchPrefixWritten;          // TRUE = XML prefix has been written
  int          iCurPar;                           // current paragraph ID
  int          iCurSeg;                           // current sentence/segment ID
  long         lOutmatchSegNum;                   // segment number in output match file
  std::vector<OtmProposal *> *pvFoundProposals;    // list of found memory proposals
  CHAR         szSegSourceLang[MAX_LANG_LENGTH];  // segment source language
  CHAR         szSegTargetLang[MAX_LANG_LENGTH];  // segment target language
  CHAR         szSegMarkup[MAX_FILESPEC];         // segment markup
  CHAR         szSegDocName[MAX_LONGFILESPEC];    // segment document name
} MEM_PROCNOMATCH_DATA, *PMEM_PROCNOMATCH_DATA;



// prototypes 
USHORT MemFuncPrepProcessNomatch( PFCTDATA, PSZ, PSZ, PSZ, PSZ, PSZ , PSZ, PSZ, PSZ, PSZ, LONG );
USHORT MemFuncProcessNomatchProcess( PFCTDATA pData );

USHORT MemFuncPRNNextDoc( PMEM_PROCNOMATCH_DATA pData );
USHORT MemFuncPRNDocEnd( PMEM_PROCNOMATCH_DATA pData );
USHORT MemFuncPRNDocProcess( PMEM_PROCNOMATCH_DATA pData );
USHORT MemFuncPRNDocStart( PMEM_PROCNOMATCH_DATA pData );
USHORT Cleanup( PMEM_PROCNOMATCH_DATA pData );
static USHORT StartNomatch( PMEM_PROCNOMATCH_DATA pData );    
static USHORT Initialize( PMEM_PROCNOMATCH_DATA pData );
static USHORT ProcessNomatch( PMEM_PROCNOMATCH_DATA pData );
static USHORT EndNomatch( PMEM_PROCNOMATCH_DATA pData );
static BOOL StripTag( PSZ_W pszLine, PSZ_W pszTag );
static USHORT GetLine( PMEM_PROCNOMATCH_DATA pData, PSZ_W pszLine, int iSize, BOOL fUnicode, PBOOL pfEOF  );
static USHORT FillBufferW( PMEM_PROCNOMATCH_DATA pData );
static USHORT FillBuffer( PMEM_PROCNOMATCH_DATA pData );
static USHORT WriteReport( PMEM_PROCNOMATCH_DATA pData, USHORT usID );
static void LogRC( USHORT usRC, PSZ pszFunction ); 
static BOOL WriteXMLPrefix( PMEM_PROCNOMATCH_DATA pData, FILE *hfOut, PSZ pszTargetLang );
static void EscapeChars( PSZ_W pszText, PSZ_W pszBuffer );
static BOOL WriteInUTF8( PMEM_PROCNOMATCH_DATA pData, FILE *hfOut, PSZ_W pszUTF16 );
BOOL TAGetIsoLang( PSZ pszTmLang, PSZ pszIsoLang );



// function running a SNOMATCH file against a NLV memory and creating word counts
USHORT MemFuncProcessNomatch
(
  PFCTDATA    pFuncData,              // function I/F session data
  PSZ         pszNomatch,              // fully qualified name of the SNOMATCh file
  PSZ         pszInMemory,             // name of the internal input memory
  PSZ         pszOutMemory,            // name of internal output memory (is created if it does not exist)
  PSZ         pszMemMatchReportText,   // fully qualified file name of the memory match count report (text format)
  PSZ         pszMemMatchReportXml,    // fully qualified file name of the memory match count report (XML format)
  PSZ         pszDupReportText,        // fully qualified file name of the duplicate word count report (text format)
  PSZ         pszDupReportXml,         // fully qualified file name of the duplicate word count report (XML format)
  PSZ         pszOutNomatchXml,        // fully qualified file name of the output nomatch file in nfluent XML format
  PSZ         pszOutNomatchExp,        // fully qualified file name of the output nomatch file in EXP format
  LONG        lOptions                 // options for processing
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new run or continue current one
  if ( pFuncData->fComplete )              // has last run been completed
  {
    // prepare a new process snomatch run
    pFuncData->usProgress = 0;
    usRC = MemFuncPrepProcessNomatch( pFuncData, pszNomatch, pszInMemory, pszOutMemory, pszMemMatchReportText,
                                      pszMemMatchReportXml, pszDupReportText, pszDupReportXml, pszOutNomatchXml, pszOutNomatchExp, lOptions );
    if ( usRC ) LogRC( usRC, "MemFuncPrepProcessNomatch" );
  }
  else
  {
    // continue current analysis process
    usRC = MemFuncProcessNomatchProcess( pFuncData );
    if ( usRC ) LogRC( usRC, "MemFuncProcessNomatchProcess" );
  } /* endif */

  if ( usRC ) LogRC( usRC, "MemFuncProcessNomatch" );

  return( usRC );
} /* end of function MemFuncProcessNomatch */

// prepare the process nomatch function
USHORT MemFuncPrepProcessNomatch
(
  PFCTDATA    pFuncData,               // function I/F session data
  PSZ         pszNomatch,              // fully qualified name of the SNOMATCh file
  PSZ         pszInMemory,             // name of the internal input memory
  PSZ         pszOutMemory,            // name of internal output memory (is created if it does not exist)
  PSZ         pszMemMatchReportText,   // fully qualified file name of the memory match count report (text format)
  PSZ         pszMemMatchReportXml,    // fully qualified file name of the memory match count report (XML format)
  PSZ         pszDupReportText,        // fully qualified file name of the duplicate word count report (text format)
  PSZ         pszDupReportXml,         // fully qualified file name of the duplicate word count report (XML format)
  PSZ         pszOutNomatchXml,        // fully qualified file name of the output nomatch file in nfluent XML format
  PSZ         pszOutNomatchExp,        // fully qualified file name of the output nomatch file in EXP format
  LONG        lOptions                 // options for processing
)
{
  BOOL        fOK = TRUE;              // internal O.K. flag
  USHORT      usRC = NO_ERROR;         // function return code
  PSZ         pszParm;                 // error parameter pointer

  PMEM_PROCNOMATCH_DATA pData = NULL;   // Pointer to our data area

  fOK = UtlAllocHwnd( (PVOID *)&pData, 0L, (LONG)sizeof(MEM_PROCNOMATCH_DATA), ERROR_STORAGE, HWND_FUNCIF );

  // check NOMATCH file specificatiom and evaluate type of NOMATCH file specification used
  if ( fOK )
  {
    // the parameter is mandatory
    if ( (pszNomatch == NULL) || (*pszNomatch == EOS) )
    {
      fOK = FALSE;
      usRC = FUNC_MANDINFILE;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */

    // check type of value
    if ( fOK )
    {
      if ( (strchr( pszNomatch, '*' ) != NULL) || (strchr( pszNomatch, '?' ) != NULL) )
      {
        // value contains wild cards, use specified value as search pattern
        strcpy( pData->szNomatchSearch, pszNomatch );
        pData->sNomatchMode = NOMATCH_WILDCARD;
        sprintf( pData->szFolLongName, "SNOMATCH search pattern %s", pszNomatch );
        strcpy( pData->szFolName, "n/a" );
      }
      else if ( strchr( pszNomatch, '\\' ) != NULL )
      {
        // assume fully qualified file or directory name
        if ( UtlDirExist( pszNomatch ) )
        {
          strcpy( pData->szNomatchSearch, pszNomatch );
          strcat( pData->szNomatchSearch, "\\*.*" );
          pData->sNomatchMode = NOMATCH_DIRECTORY;
          sprintf( pData->szFolLongName, "SNOMATCH directory %s", pszNomatch );
          strcpy( pData->szFolName, "n/a" );
        }
        else if ( UtlFileExist( pszNomatch ) )
        {
          strcpy( pData->szNomatchSearch, pszNomatch );
          pData->sNomatchMode = NOMATCH_FILE;
          sprintf( pData->szFolLongName, "SNOMATCH file %s", pszNomatch );
          strcpy( pData->szFolName, "n/a" );
        }
        else
        {
          fOK = FALSE;
          usRC = FUNC_MANDINFILE;
          UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
        } /* endif */
      }
      else
      {
        // assume a folder name 
        BOOL fIsNew;

        fIsNew = !SubFolNameToObjectName( pszNomatch,  pData->szFolObjName );

        if ( fIsNew )
        {
          fOK = FALSE;
          pszParm = pszNomatch;
          usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
          UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        }
        else
        {
          strcpy( pData->szNomatchSearch, pData->szFolObjName );
          strcat( pData->szNomatchSearch, "\\SNOMATCH\\*.*" );
          pData->sNomatchMode = NOMATCH_FOLDER;
          sprintf( pData->szFolLongName, "SNOMATCH files of folder %s", pszNomatch );
          Utlstrccpy( pData->szFolName, UtlGetFnameFromPath( pData->szFolObjName ), '.' );
        } /* endif */
      } /* endif */
    } /* endif */

  } /* endif */

  // check options
  if ( fOK )
  {
    // currently are there no options to check
  } /* endif */

  // check input memory
  if ( fOK )
  {
    if ( (pszInMemory == NULL) || (*pszInMemory == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDTM;
      UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      fOK = UtlCheckIfExist( pszInMemory, TM_OBJECT );
      if ( !fOK )
      {
        PSZ pszParm = pszInMemory;
        UtlErrorHwnd( ERROR_MEMORY_NOTFOUND, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
     } /* endif */
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
    } /* endif */

    // TODO check for internl memory, create if its a new one
  } /* endif */

   // store input values in ou data structure
   if ( fOK )
   {
     strcpy( pData->szNoMatchFile, pszNomatch );
     strcpy( pData->szInMemory, pszInMemory );
     strcpy( pData->szOutMemory, pszOutMemory );
     if  ( pszMemMatchReportText != NULL )      strcpy( pData->szMemMatchReportText, pszMemMatchReportText );
     if  ( pszMemMatchReportXml != NULL )       strcpy( pData->szMemMatchReportXml, pszMemMatchReportXml );
     if  ( pszDupReportText != NULL )         strcpy( pData->szDupReportText, pszDupReportText );
     if  ( pszDupReportXml != NULL )          strcpy( pData->szDupReportXml, pszDupReportXml );
     if  ( pszOutNomatchXml != NULL )           strcpy( pData->szOutNomatchXml, pszOutNomatchXml ); 
     if  ( pszOutNomatchExp != NULL )           strcpy( pData->szOutNomatchExp, pszOutNomatchExp ); 
     pData->lOptions = lOptions;
   } /* endif */


   if ( !fOK )
   {
     if ( pData )
     {
       UtlAlloc( (PVOID *) &pData, 0L, 0L, NOMSG );
     } /* endif */
   } /* endif */

   // prepare the process
   if ( fOK )
   {
     pFuncData->pvProcessNomatch = (PVOID)pData;
     pFuncData->fComplete = FALSE;
     pData->Task = PRNOM_START_TASK;
   }
   else
   {
     usRC = UtlQueryUShort( QS_LASTERRORMSGID );
   } /* endif */

   return( usRC );

} /* end of function MemFuncPrepProcessNomatch */

// function performing the actual work 
USHORT MemFuncProcessNomatchProcess
(
  PFCTDATA    pFuncData                  // function I/F session data
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PMEM_PROCNOMATCH_DATA pData = NULL;   // Pointer to the process data area

  pData = (PMEM_PROCNOMATCH_DATA)pFuncData->pvProcessNomatch;
  UtlSetUShort( QS_LASTERRORMSGID, 0 );

  switch ( pData->Task )
  {
    case PRNOM_START_TASK:
#ifdef SESSIONLOG
      UtlLogWrite( "ProcessNomatch: Initialize" );
#endif
      usRC = Initialize( pData ); 
      if ( usRC ) LogRC( usRC, "Initialize" );
      break;
    case PRNOM_START_NOMATCH_TASK:   
      usRC = StartNomatch( pData ); 
      if ( usRC ) LogRC( usRC, "StartNomatch" );
      break;    
    case PRNOM_PROCESS_NOMATCH_TASK: 
      usRC = ProcessNomatch( pData ); 
      if ( usRC ) LogRC( usRC, "ProcessNomatch" );
      break;
    case PRNOM_END_NOMATCH_TASK:     
      usRC = EndNomatch( pData ); 
      if ( usRC ) LogRC( usRC, "EndNomatch" );
      break;
    case PRNOM_STOP_TASK:        
      usRC = Cleanup( pData ); 
      if ( usRC ) LogRC( usRC, "MemFuncPRNCleanup" );
      break;
    case PRNOM_COMPLETED_TASK:   
      break;
 } /* endswitch */

 if ( usRC )
 {
   // cleanup in case of errors
   if ( pData->Task != PRNOM_COMPLETED_TASK )
   {
     Cleanup( pData ); 
   } /* endif */
 } /* endif */

 if ( pData->Task == PRNOM_COMPLETED_TASK )
 {
   pFuncData->fComplete = TRUE;
 } /* endif */

 // free our data area when done
 if ( usRC || (pData->Task == PRNOM_COMPLETED_TASK) )
 {
   UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG ); 
 } /* endif */
 return( usRC );

} /* end of function MemFuncProcessNomatchProcess */

// ProcessNomatch start up code
static USHORT Initialize
(
  PMEM_PROCNOMATCH_DATA pData
)
{
  USHORT usRC = 0;
  BOOL fMoreFiles = TRUE;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  pData->sLanguage = -1;

  // allocate worcount data areas
  if ( !UtlAllocHwnd( (PVOID *)&(pData->pCNTIda), 0L, (ULONG)sizeof( CNTIDA), ERROR_STORAGE, HWND_FUNCIF ) )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  }
  else if ( !UtlAllocHwnd( (PVOID *)&(pData->pCNTIda->pCnt), 0L, (ULONG)sizeof(CNT), ERROR_STORAGE, HWND_FUNCIF ) )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // open input memory
  if ( !usRC )
  {
#ifdef SESSIONLOG
    UtlLogWriteString( "EqfProcessNomatch: Open of input memory %s", pData->szInMemPath );
#endif
    int iRC = 0;
    pData->pInMem = pFactory->openMemory( NULL, pData->szInMemory, 0, &iRC );
    usRC = (USHORT)iRC;

    if ( pData->pInMem == NULL )
    {
      LogRC( usRC, "factory->openMemory-InputMem" );
      pFactory->showLastError( NULL, pData->szInMemory, NULL, HWND_FUNCIF );
    } /* endif */           
  } /* endif */

  // create the temporary TM
  if ( !usRC )
  {
    pData->pInMem->getSourceLanguage( pData->szLanguage, sizeof(pData->szLanguage) );
    pData->pTempMem = pFactory->createTempMemory( "$PN", pData->szTempMemory, pData->szLanguage );
#ifdef SESSIONLOG
    UtlLogWriteString( "EqfProcessNomatch: Created temp memory %s", pData->szTempMemory );
#endif
    if ( usRC ) LogRC( usRC, "factory->createMemory" );
  } /* endif */

  // create or open the output memory
  if ( !usRC )
  {
    int iRC = 0;

    if ( UtlCheckIfExist( pData->szOutMemory, TM_OBJECT ) )
    {
      pData->pOutMem = pFactory->openMemory( NULL, pData->szOutMemory, 0, &iRC );
      usRC = (USHORT)iRC;

      if ( pData->pOutMem == NULL )
      {
        LogRC( usRC, "factory->openMemory-OutputMem" );
        pFactory->showLastError( NULL, pData->szOutMemory, NULL, HWND_FUNCIF );
      } /* endif */           
    }
    else
    {
      pData->pOutMem = pFactory->createMemory( NULL, pData->szOutMemory, "", pData->szLanguage, &iRC );
      usRC = (USHORT)iRC;

      if ( pData->pOutMem == NULL )
      {
        LogRC( usRC, "factory->createMemory-OutputMem" );
        pFactory->showLastError( NULL, pData->szOutMemory, NULL, HWND_FUNCIF );
      } /* endif */           
    } /* endif */
  } /* endif */

  // prepare buffer for found memory proposals
  if ( !usRC )
  {
    pData->pvFoundProposals = new std::vector<OtmProposal *>;
    for ( int i = 0; i < 10; i++ )
    {
      pData->pvFoundProposals->push_back( new OtmProposal );
    } /* endfor */     
  } /* end */     

  // cont number of NOMATCH files
  if ( !usRC )
  {
    pData->hDirNomatch = FindFirstFile( pData->szNomatchSearch, &pData->FileFindData );
    if ( pData->hDirNomatch != INVALID_HANDLE_VALUE )
    {
      fMoreFiles = TRUE;
      do
      {
        if ( !(pData->FileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
             !((pData->FileFindData.nFileSizeHigh == 0) && (pData->FileFindData.nFileSizeLow == 0) ) )
        {
          pData->usNumOfFiles++;
        } /*endif */
        fMoreFiles = FindNextFile( pData->hDirNomatch , &pData->FileFindData );
      } while ( fMoreFiles );
      FindClose( pData->hDirNomatch );
    } /* endif */
  } /* endif */


  // start search for NOMATCH files
  if ( !usRC )
  {
    pData->hDirNomatch = FindFirstFile( pData->szNomatchSearch, &pData->FileFindData );
    if ( pData->hDirNomatch != INVALID_HANDLE_VALUE )
    {
      fMoreFiles = TRUE;
      // skip any directories and empty files
      while ( fMoreFiles && ( (pData->FileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
                ((pData->FileFindData.nFileSizeHigh == 0) && (pData->FileFindData.nFileSizeLow == 0) ) ) )
      {
        fMoreFiles = FindNextFile( pData->hDirNomatch , &pData->FileFindData );
      } /*endwhile */
    }
    else
    {
      fMoreFiles = FALSE;
    } /* endif */


  } /* endif */

  // open report files and write report header
  if ( !usRC )
  {
    USHORT usAction = 0;

    if ( pData->szMemMatchReportText[0] != EOS )
    {
      usRC = UtlOpenHwnd( pData->szMemMatchReportText, &pData->hfMemMatchReportText, &usAction, 0L, FILE_NORMAL,
                              OPEN_ACTION_REPLACE_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW, OPEN_ACCESS_READWRITE |  OPEN_SHARE_DENYWRITE,
                              0L, TRUE, HWND_FUNCIF) ;
    if ( usRC ) LogRC( usRC, "UtlOpenHwnd-MemMatchReportText" );
    } /* endif */

    if ( !usRC && (pData->szMemMatchReportXml[0] != EOS) )
    {
      usRC = UtlOpenHwnd( pData->szMemMatchReportXml, &pData->hfMemMatchReportXml, &usAction, 0L, FILE_NORMAL,
                              OPEN_ACTION_REPLACE_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW, OPEN_ACCESS_READWRITE |  OPEN_SHARE_DENYWRITE,
                              0L, TRUE, HWND_FUNCIF) ;
    if ( usRC ) LogRC( usRC, "UtlOpenHwnd-MemMatchReportXML" );
    } /* endif */

    if ( !usRC && (pData->szDupReportText[0] != EOS) )
    {
      usRC = UtlOpenHwnd( pData->szDupReportText, &pData->hfDupReportText, &usAction, 0L, FILE_NORMAL,
                              OPEN_ACTION_REPLACE_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW, OPEN_ACCESS_READWRITE |  OPEN_SHARE_DENYWRITE,
                              0L, TRUE, HWND_FUNCIF) ;
    if ( usRC ) LogRC( usRC, "UtlOpenHwnd-DupReportText" );
    } /* endif */

    if ( !usRC && (pData->szDupReportXml[0] != EOS) )
    {
      usRC = UtlOpenHwnd( pData->szDupReportXml, &pData->hfDupReportXml, &usAction, 0L, FILE_NORMAL,
                              OPEN_ACTION_REPLACE_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW, OPEN_ACCESS_READWRITE |  OPEN_SHARE_DENYWRITE,
                              0L, TRUE, HWND_FUNCIF) ;
    if ( usRC ) LogRC( usRC, "UtlOpenHwnd-DupReportXML" );
    } /* endif */

    if ( !usRC && (pData->szOutNomatchXml[0] != EOS) )
    {
      pData->hfOutNomatchXml = fopen( pData->szOutNomatchXml, "wb" );
      if ( pData->hfOutNomatchXml == NULL )
      {
        PSZ pszParm = pData->szOutNomatchXml;
        usRC = ERROR_FILE_WRITE_ERROR;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        if ( usRC ) LogRC( usRC, "fOpen-OutNomatchXML" );
      } /* endif */         
    } /* endif */

    if ( !usRC && (pData->szOutNomatchExp[0] != EOS) )
    {
      pData->hfOutNomatchExp = fopen( pData->szOutNomatchExp, "wb" );
      if ( pData->hfOutNomatchExp == NULL )
      {
        PSZ pszParm = pData->szOutNomatchExp;
        usRC = ERROR_FILE_WRITE_ERROR;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        if ( usRC ) LogRC( usRC, "fOpen-OutNomatchExp" );
      }
      else
      {
        fwrite( UNICODEFILEPREFIX, 2, 1, pData->hfOutNomatchExp );
        fputws( L"<NTMMemoryDb>\r\n", pData->hfOutNomatchExp );
        fputws( L"<Description>\r\n", pData->hfOutNomatchExp );
        fputws( L"NOMATCH output generated by EqfProcessNomatchEX\r\n", pData->hfOutNomatchExp );
        fputws( L"</Description>\r\n", pData->hfOutNomatchExp );
        fputws( L"<Codepage>UTF16</Codepage>\r\n", pData->hfOutNomatchExp );
      } /* endif */         
    } /* endif */
     
    if ( !usRC )
    {
      usRC = WriteReport( pData, HEADER );
    } /* endif */
  } /* endif */


  // prepare next processing step
  if ( usRC )
  {
    pData->Task = PRNOM_STOP_TASK;
    pData->fErrorStop = TRUE;
  }
  else if ( fMoreFiles )
  {
    strcpy( pData->szCurNomatch, pData->szNomatchSearch );
    UtlSplitFnameFromPath( pData->szCurNomatch );
    strcat( pData->szCurNomatch, "\\" ); 
    strcat( pData->szCurNomatch, pData->FileFindData.cFileName ); 
    pData->Task = PRNOM_START_NOMATCH_TASK;
  }
  else
  {
    pData->Task = PRNOM_END_NOMATCH_TASK;
  } /* endif */

  return( usRC );
} /* end of function Initialize */


// MemoryCleanup end of input memory handling
USHORT EndNomatch
(
  PMEM_PROCNOMATCH_DATA pData
)
{
  USHORT usRC = 0;
  BOOL fMoreFiles = TRUE;

  if ( pData->hfIn )
  {
    UtlClose( pData->hfIn, FALSE );
    pData->hfIn = NULLHANDLE;

    // write reports for this nomatch file
    usRC = WriteReport( pData, FILE_COUNT_LINE );
    if ( usRC ) LogRC( usRC, "WriteReport-FileCountLine" );
  } /* endif */

  // skip any directories and empty files
  do
  {
    fMoreFiles = FindNextFile( pData->hDirNomatch, &pData->FileFindData );
  } while ( fMoreFiles &&  
            ( (pData->FileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
              ((pData->FileFindData.nFileSizeHigh == 0) && (pData->FileFindData.nFileSizeLow == 0) ) ) );
 
  if ( fMoreFiles )
  {
    strcpy( pData->szCurNomatch, pData->szNomatchSearch );
    UtlSplitFnameFromPath( pData->szCurNomatch );
    strcat( pData->szCurNomatch, "\\" ); 
    strcat( pData->szCurNomatch, pData->FileFindData.cFileName ); 
    pData->Task = PRNOM_START_NOMATCH_TASK;
  }
  else
  {
    usRC = WriteReport( pData, SUMMARY_LINE );
    if ( usRC ) LogRC( usRC, "WriteReport-SummaryLine" );

    FindClose( pData->hDirNomatch );
    pData->hDirNomatch = NULLHANDLE;
    pData->Task = PRNOM_STOP_TASK;
  } /* endif */

  return( usRC );
} /* end of function EndNomatch */



// Cleanup of process notmatch processing
USHORT Cleanup
(
  PMEM_PROCNOMATCH_DATA pData
)
{
  USHORT usRC = 0;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  if ( pData->hfIn )     UtlClose( pData->hfIn, FALSE );

  if ( pData->pInMem != NULL )
  {
    pFactory->closeMemory( pData->pInMem );
    pData->pInMem = NULL;
  } /* endif */

  if ( pData->pTempMem != NULL )
  {
    pFactory->closeTempMemory( pData->pTempMem );
    pData->pTempMem = NULL;
  } /* endif */

  if ( pData->pOutMem != NULL )
  {
    pFactory->closeMemory( pData->pOutMem );
    pData->pOutMem = NULL;
  } /* endif */

  for ( int i = 0; i < (int)pData->pvFoundProposals->size(); i++ )
  {
    delete (*pData->pvFoundProposals)[i];
  } /* endfor */         
  delete pData->pvFoundProposals;


  if ( pData->hfMemMatchReportText ) UtlClose( pData->hfMemMatchReportText, FALSE );
  if ( pData->hfMemMatchReportXml ) UtlClose( pData->hfMemMatchReportXml, FALSE );
  if ( pData->hfDupReportText ) UtlClose( pData->hfDupReportText, FALSE );
  if ( pData->hfDupReportXml ) UtlClose( pData->hfDupReportXml, FALSE );

  if ( pData->hfOutNomatchXml ) 
  {
    // end current doc (if any) 
    if ( pData->szCurDoc[0] )
    {
      BOOL fOK = TRUE;
      if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, L"      </p>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, L"    </d>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, L"   </docs>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, L"</rqt>\r\n" );
    } /* endif */

    fclose( pData->hfOutNomatchXml );
  } /* endif */     

  if ( pData->hfOutNomatchExp ) 
  {
    fputws( L"</NTMMemoryDb>\r\n", pData->hfOutNomatchExp );
    fclose( pData->hfOutNomatchExp );
  } /* endif */     


  if ( pData->sLanguage != -1) MorphFreeLanguageID( pData->sLanguage );
  if ( pData->pTagTable ) TAFreeTagTable( pData->pTagTable );

  if ( pData->pCNTIda )
  {
    if ( pData->pCNTIda->pCnt ) UtlAllocHwnd( (PVOID *)&(pData->pCNTIda->pCnt), 0L, 0L, NOMSG, HWND_FUNCIF );
    UtlAllocHwnd( (PVOID *)&(pData->pCNTIda), 0L, 0L, NOMSG, HWND_FUNCIF );
  } /* endif */

  pData->Task = PRNOM_COMPLETED_TASK;

  return( usRC );
} /* end of function Cleanup */

// Start read of SNOMATCH file
USHORT StartNomatch
(
  PMEM_PROCNOMATCH_DATA pData
)
{
  USHORT usRC = 0;
  BOOL   fEOF = FALSE;

  // set NOMATCH name in count data 
  strcpy( pData->pCNTIda->szLongName, UtlGetFnameFromPath( pData->szCurNomatch ) );

  // clear count fields on file level
  memset( &(pData->lFileCount), 0, sizeof(pData->lFileCount) );
  memset( &(pData->lFileDupCount), 0, sizeof(pData->lFileDupCount) );

  // try to get target language of document belonging to this SNOMATCH file
  {
    PSZ pszName;

    // setup document object name
    strcpy( pData->szBuffer, pData->szCurNomatch );
    pszName = UtlSplitFnameFromPath( pData->szBuffer );
    UtlSplitFnameFromPath( pData->szBuffer );
    strcpy( pData->szDocName, pszName );
    strcat ( pData->szBuffer, BACKSLASH_STR  );
    strcat ( pData->szBuffer, pData->szDocName );

    // clear document language fields
    pData->szDocSourceLang[0] = EOS;
    pData->szDocTargetLang[0] = EOS;

    // get document target languages
    DocQueryInfo2Hwnd( pData->szBuffer, NULL, NULL, pData->szDocSourceLang, pData->szDocTargetLang,
                       pData->szDocLongName, NULL, NULL, FALSE, HWND_FUNCIF ); 


  }
  

  // open input NOMATCH file
  {
    USHORT      usOpenAction;            // dummy for UtlOpen

    usRC = UtlOpenHwnd( pData->szCurNomatch, &(pData->hfIn), &usOpenAction, 0L,
                        FILE_NORMAL, FILE_OPEN, OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                        0L, TRUE, HWND_FUNCIF );
    if ( usRC ) LogRC( usRC, "UtlOpenHwnd" );

    // reset buffer handling fields
    pData->iInBufProcessed = 0;
    pData->iInBufRead = 0;
  } /* endif */

  // get first bytes of SNOMATCH file and check for UTF-16 encoding
  if ( !usRC )
  {
    ULONG ulBytesRead = 0;

    // get first two bytes and check for BOM
    BYTE chBOM[2];

    UtlReadHwnd( pData->hfIn, chBOM, 2, &ulBytesRead, TRUE, HWND_FUNCIF );

    if ( memcmp( chBOM, UNICODEFILEPREFIX, 2 ) == 0 )
    {
      pData->fUnicode = TRUE;
      usRC = FillBufferW( pData );
    }
    else
    {
      ULONG ulFilePos = 0;

      pData->fUnicode = FALSE;

      // reposition to begin of file
      UtlChgFilePtr( pData->hfIn, 0L, FILE_BEGIN, &ulFilePos, FALSE );
      usRC = FillBuffer( pData );
    } /* endif */

    GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF  );

    if ( wcsnicmp( pData->szLine, L"<ntmmemorydb>", 13 ) != 0 )
    {
      UtlErrorHwnd( ERROR_MEM_INVALID_SGML, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF);
      usRC = NO_VALID_UNICODEFORMAT;
    } /* endif */
  } /* endif */

  // read header up to first segment
  if ( !usRC )
  {
    while ( !fEOF && 
            (wcsnicmp( pData->szLine, L"<segment>", 9 ) != 0) &&
            (wcsnicmp( pData->szLine, L"</ntmmemorydb>", 14 ) != 0))
    {
      pData->szLine[0] = 0;
      GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
    } /*endwhile */
  } /* endif */

  pData->Task = ( !usRC ) ? PRNOM_PROCESS_NOMATCH_TASK : PRNOM_STOP_TASK;

  return( usRC );
} /* end of function StartNomatch */


// MemoryCleanup memory read process
USHORT ProcessNomatch
(
  PMEM_PROCNOMATCH_DATA pData
)
{
  USHORT usRC = 0;
  BOOL fMatch = FALSE;
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
      GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );

      if ( wcsnicmp( pData->szLine, L"<control>", 9 ) == 0 )
      {
        // get the control string
        GetLine( pData, pData->szControl, sizeof(pData->szControl), pData->fUnicode, &fEOF );

        if ( fEOF )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
        }
        else
        {
          // get contol string end tag and check it
          GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF  );

          // todo: check for </control>
        } /* endif */

      }
      else if ( wcsnicmp( pData->szLine, L"<source>", 8 ) == 0  )
      {
        // get the segment source
        BOOL fEnd = StripTag( pData->szLine + 8, L"</source>" );
        wcscat( pData->szSource, pData->szLine + 8 );

        while ( !fEnd && !fEOF )
        {
          GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
          fEnd = StripTag( pData->szLine, L"</source>" );
          if ( !fEnd || (pData->szLine[0] != 0) )
          {
            wcscat( pData->szSource, L"\r\n" ); 
            wcscat( pData->szSource, pData->szLine );
          } /* endif */
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
        BOOL fEnd = StripTag( pData->szLine + 8, L"</target>" );
        wcscat( pData->szTarget, pData->szLine + 8 );

        while ( !fEnd && !fEOF )
        {
          GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
          fEnd = StripTag( pData->szLine, L"</target>" );
          if ( !fEnd || (pData->szLine[0] != 0) )
          {
            wcscat( pData->szTarget, L"\r\n" ); 
            wcscat( pData->szTarget, pData->szLine );
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
    GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );

    if ( usRC )
    {
      PSZ pszParms[2];

      pszParms[0] = "n/a";
      pszParms[1] = pData->szInMemory;

      UtlErrorHwnd( ERROR_INVALID_SEGMENT, MB_CANCEL, 2, pszParms, EQF_ERROR, HWND_FUNCIF);
    } /* endif */
  } /* endif */

  // lookup segment in input memory
  if ( !usRC && pData->szControl[0] && pData->szSource[0]  )
  {
    OtmProposal SearchKey;
    ULONG ulOptions = 0;

    fMatch = FALSE;  

    WideCharToMultiByte( CP_OEMCP, 0, pData->szControl, -1, pData->szControlAscii, sizeof(pData->szControlAscii), NULL, NULL );

    SearchKey.setSegmentNum( atol( UtlParseX15( pData->szControlAscii, 0 ) ) );

    SearchKey.setSource( pData->szSource );

    strcpy( pData->pCNTIda->szCountFileName, UtlParseX15( pData->szControlAscii, 7 ) );
    SearchKey.setDocShortName( pData->pCNTIda->szCountFileName );

    strcpy( pData->pCNTIda->szLongName, UtlParseX15( pData->szControlAscii, 8 ) );
    SearchKey.setDocName( pData->pCNTIda->szLongName );

    ulOptions = GET_EXACT | GET_NO_GENERICREPLACE | GET_ALWAYS_WITH_TAGS;
    if ( pData->lOptions & RESPECTCRLF_OPT )
    {
      ulOptions |= GET_RESPECTCRLF;
    } /* endif */

    strcpy( pData->szSegMarkup, UtlParseX15( pData->szControlAscii, 6 ));
    SearchKey.setMarkup( pData->szSegMarkup );

    strcpy( pData->szSegSourceLang, UtlParseX15( pData->szControlAscii, 3 ) );
    SearchKey.setSourceLanguage( pData->szSegSourceLang );

    // use document target language when available
    if ( pData->szDocTargetLang[0] != EOS )
    {
      strcpy( pData->szSegTargetLang, pData->szDocTargetLang );

    }
    else
    {
      strcpy( pData->szSegTargetLang, UtlParseX15( pData->szControlAscii, 4 ) );
    } /* endif */
    SearchKey.setTargetLanguage( pData->szSegTargetLang );


    // only continue for valid segments
    if ( (wcslen( pData->szSource ) <= 2047) )
    {
      ULONG ulWords = 0;                         // number of words in segment
      ULONG ulMarkUp = 0;                        // mumber of tags in segment
      USHORT usInMemMatchLevel = 0;
      USHORT usTempMemMatchLevel = 0;
      BOOL fMachineMatch = FALSE;

      // get language ID and codepage
      if ( strcmp( pData->szSegSourceLang, pData->szCurLanguage ) != 0 )
      {
        usRC= MorphGetLanguageID( pData->szSegSourceLang, &pData->sLanguage );
        if ( !usRC )
        {
          strcpy( pData->szCurLanguage, pData->szSegSourceLang );
          pData->ulOemCP = GetLangOEMCP( pData->szCurLanguage );
        } /* endif */
      } /* endif */

      // load tag table for current segment
      if ( strcmp( pData->szSegMarkup, pData->szCurTagTable ) != 0 )
      {
        if ( pData->szCurTagTable[0] != EOS  )
        {
          TAFreeTagTable( pData->pTagTable );
          pData->szCurTagTable[0] = EOS;
        } /* endif */

        usRC = TALoadTagTableExHwnd( pData->szSegMarkup, &pData->pTagTable,
                                     FALSE, TALOADPROTTABLEFUNC | TALOADUSEREXIT, TRUE, HWND_FUNCIF );
        if ( usRC ) LogRC( usRC, "TALoadTagTableExHwnd" );
        if ( !usRC )
        {
          strcpy( pData->szCurTagTable, pData->szSegMarkup );
        } /* endif */
      } /* endif */

      // count words of the segment
      if ( !usRC )
      {
        usRC = EQFBWordCntPerSeg( pData->pTagTable, (PTOKENENTRY)&(pData->bTokBuffer),
                                  pData->szSource, pData->sLanguage, &ulWords, &ulMarkUp, pData->ulOemCP );
        if ( usRC ) LogRC( usRC, "EQFBWordCntPerSeg" );
      } /* endif */

#ifdef TEST_OUTPUT
      {
        FILE *hfTest = fopen( "I:\\EQF\\LOGS\\NOMATCH.LOG", "a" );
        if ( hfTest )
        {
          fprintf( hfTest, "%lu\t%lu\t%lu\n", pGetIn->stTmGet.ulSegmentId, ulWords, ulMarkUp );
          fclose( hfTest );
        } /* endif */
      }
#endif

      // lookup segment in input memory
      OtmProposal::clearAllProposals( *(pData->pvFoundProposals) );
      pData->pInMem->searchProposal( SearchKey, *(pData->pvFoundProposals), ulOptions );

      // write any matches found to the output memory
      int iNumOfProposal = OtmProposal::getNumOfProposals( *(pData->pvFoundProposals) );
      if ( iNumOfProposal != 0 )
      {
        USHORT i = 0;
        while ( i < iNumOfProposal )
        {
          pData->pOutMem->putProposal( *(*pData->pvFoundProposals)[i] );
          i++;
        } /*endwhile */

        // remember best match level
        usInMemMatchLevel = (USHORT)(*pData->pvFoundProposals)[0]->getFuzziness();
        fMachineMatch = (*pData->pvFoundProposals)[0]->getType() == OtmProposal::eptMachine;
      } /* endif */

      // lookup in temp memory to check for duplicates
      if ( usInMemMatchLevel < 100 )
      {
        OtmProposal::clearAllProposals( *pData->pvFoundProposals );

        pData->pTempMem->searchProposal( SearchKey, *pData->pvFoundProposals, ulOptions );

        if ( OtmProposal::getNumOfProposals( *pData->pvFoundProposals ) != 0 )
        {
          usTempMemMatchLevel = (USHORT)(*pData->pvFoundProposals)[0]->getFuzziness();
        } /* endif */
      } /* endif */

      // write segment to temporary memory if there is no exact match yet
      if ( (usInMemMatchLevel < 100) && (usTempMemMatchLevel < 100) )
      {
        SearchKey.setTarget( pData->szSource );
        pData->pTempMem->putProposal( SearchKey );
      } /* endif */

      // write segment to output nomatch file if there is no exact match yet
      if ( (usInMemMatchLevel < 100) && (pData->hfOutNomatchXml != NULL) )
      {
          BOOL fOK = TRUE;
          PSZ pszDoc = (pData->pCNTIda->szLongName[0] != EOS) ? pData->pCNTIda->szLongName : pData->pCNTIda->szCountFileName;  

          // write doc start if this is a new document
          if ( strcmp( pData->szCurDoc, pszDoc ) != 0 )
          {
            // end current doc (if any) 
            if ( pData->szCurDoc[0] )
            {
              BOOL fOK = TRUE;
              if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, L"      </p>\r\n" );
              if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, L"    </d>\r\n" );
            } /* endif */

            if ( !pData->fOutNomatchPrefixWritten )
            {
              WriteXMLPrefix( pData, pData->hfOutNomatchXml, pData->szSegSourceLang );
              pData->fOutNomatchPrefixWritten = TRUE;
            } /* endif */               

            strcpy( pData->szCurDoc, pszDoc );

            // write start of new document
            if ( fOK )
            {
              CHAR szIso[20];
              pData->iCurPar = pData->iCurSeg = 1;
              TAGetIsoLang( pData->szSegSourceLang, szIso );
              
              // write attributes up to document name
              swprintf ( pData->szUTF16Buffer, L"    <d lang=\"%S\" dt=\"ndoc\" url=\"", szIso );
              if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, pData->szUTF16Buffer );

              // write escaped document name
              MultiByteToWideChar( CP_ACP, 0, pData->szCurDoc, -1, (PSZ_W)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer)/sizeof(CHAR_W) );
              EscapeChars( (PSZ_W)pData->bUTF8Buffer, pData->szUTF16Buffer );
              if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, pData->szUTF16Buffer );

              // write the rest of the attributes
              swprintf ( pData->szUTF16Buffer, L"\" markup=\"%S\">\r\n", pData->szSegMarkup );
              if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, pData->szUTF16Buffer );

              swprintf ( pData->szUTF16Buffer, L"      <p pid=\"%ld\">\r\n", pData->iCurPar );
              pData->iCurPar++;
              if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, pData->szUTF16Buffer );
            } /* endif */
          } /* endif */

          // write segment data
          if ( fOK )
          {
            swprintf ( pData->szUTF16Buffer, L"        <s sid=\"%ld\" seg=\"%lu\"><t tid=\"1\" tt=\"orig\">", pData->iCurSeg, SearchKey.getSegmentNum() );
            pData->iCurSeg++;
            if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, pData->szUTF16Buffer );
            EscapeChars( pData->szSource , pData->szUTF16Buffer );
            if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, pData->szUTF16Buffer );
            if ( fOK ) fOK = WriteInUTF8( pData, pData->hfOutNomatchXml, L"</t></s>\r\n" );
          } /* endif */
      } /* endif */

      // write segment to output nomatch EXP file if there is no exact match yet
      if ( (usInMemMatchLevel < 100) && (pData->hfOutNomatchExp != NULL) )
      {
          pData->lOutmatchSegNum += 1;
          fwprintf( pData->hfOutNomatchExp, L"<Segment>%10.10ld\r\n", pData->lOutmatchSegNum );
          fputws( L"<Control>\r\n", pData->hfOutNomatchExp );
          fwprintf( pData->hfOutNomatchExp, L"%06lu%S%c%S%016lu%S%S%S%S%S%S%S%S%S%S%S%S\r\n",
              SearchKey.getSegmentNum(),
              X15_STR,
              L'0',
              X15_STR,
              0,
              X15_STR,
              pData->szSegSourceLang,
              X15_STR,
              pData->szSegTargetLang,
              X15_STR,
              L"",
              X15_STR,
              pData->szSegMarkup,
              X15_STR,
              pData->pCNTIda->szCountFileName,
              X15_STR,
              pData->pCNTIda->szLongName );
          fputws( L"</Control>\r\n", pData->hfOutNomatchExp );
          fwprintf( pData->hfOutNomatchExp, L"<Source>%s</Source>\r\n", pData->szSource );
          fwprintf( pData->hfOutNomatchExp, L"<Target>%s</Target>\r\n", L"" );
          fputws( L"</Segment>\r\n", pData->hfOutNomatchExp );
      } /* endif */

      // fill value fields for memory match count
      pData->lFileCount.ulWTotal += ulWords;
      pData->lSumCount.ulWTotal += ulWords;

      if ( fMachineMatch )
      {
        pData->lFileCount.ulMachineMatch += ulWords;
        pData->lSumCount.ulMachineMatch += ulWords;
      }
      else if ( usInMemMatchLevel > 100 )
      {
        pData->lFileCount.ulWTransCopied += ulWords;
        pData->lSumCount.ulWTransCopied += ulWords;
      }
      else if ( usInMemMatchLevel == 100 )
      {
        pData->lFileCount.ulWTransScratch += ulWords;
        pData->lSumCount.ulWTransScratch += ulWords;
      }
      else if ( usInMemMatchLevel > 0 )
      {
        pData->lFileCount.ulWTransMod += ulWords;
        pData->lSumCount.ulWTransMod += ulWords;
      }
      else 
      {
        pData->lFileCount.ulWToBeTran += ulWords;
        pData->lSumCount.ulWToBeTran += ulWords;
      } /*endif */

      // fill count fields for duplicate word count
      pData->lFileDupCount.ulWTotal += ulWords;
      pData->lSumDupCount.ulWTotal += ulWords;
      pData->lFileDupCount.ulWMarkup += ulMarkUp;
      pData->lSumDupCount.ulWMarkup += ulMarkUp;
      if ( usInMemMatchLevel >= 100 )
      {
        pData->lFileDupCount.ulWTran += ulWords;
        pData->lSumDupCount.ulWTran += ulWords;
      } 
      else if ( usTempMemMatchLevel >= 100 )
      {
        pData->lFileDupCount.ulWDuplicate += ulWords;
        pData->lSumDupCount.ulWDuplicate += ulWords;
        pData->lFileDupCount.ulWUntranslated += ulWords;
        pData->lSumDupCount.ulWUntranslated += ulWords;
      }
      else
      {
        pData->lFileDupCount.ulWToBeTran += ulWords;
        pData->lSumDupCount.ulWToBeTran += ulWords;
        pData->lFileDupCount.ulWUntranslated += ulWords;
        pData->lSumDupCount.ulWUntranslated += ulWords;
      } /* endif */
    }
    else
    {
      // ignore this segment
    } /* endif */
  } /* endif */
  
  // skip empy lines
  while ( !fEOF && (pData->szLine[0] == 0) )
  {
    GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
  } /*endwhile */

  if ( usRC )
  {
    pData->fErrorStop = TRUE;
    pData->Task = PRNOM_STOP_TASK;
  }
  else if ( wcsnicmp( pData->szLine, L"<segment>", 9 ) == 0 ) 
  {
    pData->Task = PRNOM_PROCESS_NOMATCH_TASK;
  }
  else if ( fEOF || (wcsnicmp( pData->szLine, L"</ntmmemorydb>", 14 ) == 0) )
  {
    pData->Task = PRNOM_END_NOMATCH_TASK;
  }
  else
  {
    PSZ pszParm = pData->szInMemory;
    UtlErrorHwnd( ERROR_SGML_TAG, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF);
    usRC = ERROR_SGML_TAG;
    if ( usRC ) LogRC( usRC, "ReadInputMem" );
    pData->fErrorStop = TRUE;
    pData->Task = PRNOM_STOP_TASK;
  } /* endif */

  return( usRC );

} /* end of function ProcessNomatch */



static USHORT FillBufferW
(
  PMEM_PROCNOMATCH_DATA pData
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

} /* end of function FillBufferW */


static USHORT FillBuffer
(
  PMEM_PROCNOMATCH_DATA pData
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

} /* end of function FillBuffer */

static USHORT ReadLineW
(
  PMEM_PROCNOMATCH_DATA pData,
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
          usRC = FillBufferW( pData );
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

} /* end of function ReadLineW */


static USHORT ReadLine
(
  PMEM_PROCNOMATCH_DATA pData,
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
          usRC = FillBuffer( pData );
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

} /* end of function ReadLine */



static USHORT GetLine( PMEM_PROCNOMATCH_DATA pData, PSZ_W pszLine, int iSize, BOOL fUnicode, PBOOL pfEOF  )
{
  USHORT usRC = 0;

  *pszLine = 0;
  if ( fUnicode )
  {
    usRC = ReadLineW( pData, pszLine, iSize );
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

    usRC = ReadLine( pData, szAsciiLine, sizeof(szAsciiLine) );
    if ( usRC == BTREE_EOF_REACHED )
    {
      *pfEOF = TRUE;
      usRC = 0;
    } /* endif */

    // the SNOMATCH file is in UTF8 encoding!
    MultiByteToWideChar( CP_UTF8 /*pData->ulInputCP*/, 0, szAsciiLine, -1, pszLine, iSize-1 );
  } /* endif */

  return( usRC );
}

// helper function to check for and remove the given tag from the end of the string
BOOL StripTag( PSZ_W pszLine, PSZ_W pszTag )
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
} /* end of function StripTag */

USHORT WriteReport
(
  PMEM_PROCNOMATCH_DATA pData,
  USHORT usID
)
{
  USHORT usRC = 0;

  pData->pCNTIda->pCnt->fBatch = TRUE;
  pData->pCNTIda->pCnt->hwndErrMsg = HWND_FUNCIF;
  strcpy( pData->pCNTIda->pCnt->szLongFolderName, pData->szFolLongName );  // folder long name
  strcpy( pData->pCNTIda->pCnt->szFolderName, pData->szFolName );          // folder short name
  pData->pCNTIda->pCnt->usNoOfDocs = pData->usNumOfFiles;

  if ( pData->hfMemMatchReportText )
  {
    pData->pCNTIda->pCnt->fXMLOutput = FALSE;
    pData->pCNTIda->pCnt->fOrg = TMMATCHES_STATE ;
    pData->pCNTIda->hfFileHandle = pData->hfMemMatchReportText;
    memcpy( &(pData->pCNTIda->lCountFile), &(pData->lFileCount), sizeof(pData->pCNTIda->lCountFile) ); 
    memcpy( &(pData->pCNTIda->lCountSum), &(pData->lSumCount), sizeof(pData->pCNTIda->lCountSum) ); 
    usRC = CntWriteToFile( pData->pCNTIda, HWND_FUNCIF, usID );
    pData->hfMemMatchReportText = pData->pCNTIda->hfFileHandle;
  } /* endif */

  if ( pData->hfMemMatchReportXml )
  {
    pData->pCNTIda->pCnt->fXMLOutput = TRUE;
    pData->pCNTIda->pCnt->fOrg = TMMATCHES_STATE ;
    pData->pCNTIda->hfFileHandle = pData->hfMemMatchReportXml;
    memcpy( &(pData->pCNTIda->lCountFile), &(pData->lFileCount), sizeof(pData->pCNTIda->lCountFile) ); 
    memcpy( &(pData->pCNTIda->lCountSum), &(pData->lSumCount), sizeof(pData->pCNTIda->lCountSum) ); 
    usRC = CntWriteToFile( pData->pCNTIda, HWND_FUNCIF, usID );
    pData->hfMemMatchReportXml = pData->pCNTIda->hfFileHandle;
  } /* endif */

  if ( pData->hfDupReportText )
  {
    pData->pCNTIda->pCnt->fXMLOutput = FALSE;
    pData->pCNTIda->pCnt->fOrg = DUPLICATE_STATE;
    pData->pCNTIda->hfFileHandle = pData->hfDupReportText ;
    memcpy( &(pData->pCNTIda->lCountFile), &(pData->lFileDupCount), sizeof(pData->pCNTIda->lCountFile) ); 
    memcpy( &(pData->pCNTIda->lCountSum), &(pData->lSumDupCount), sizeof(pData->pCNTIda->lCountSum) ); 
    usRC = CntWriteToFile( pData->pCNTIda, HWND_FUNCIF, usID );
    pData->hfDupReportText = pData->pCNTIda->hfFileHandle;
  } /* endif */

  if ( pData->hfDupReportXml )
  {
    pData->pCNTIda->pCnt->fXMLOutput = TRUE;
    pData->pCNTIda->pCnt->fOrg = DUPLICATE_STATE;
    pData->pCNTIda->hfFileHandle = pData->hfDupReportXml ;
    memcpy( &(pData->pCNTIda->lCountFile), &(pData->lFileDupCount), sizeof(pData->pCNTIda->lCountFile) ); 
    memcpy( &(pData->pCNTIda->lCountSum), &(pData->lSumDupCount), sizeof(pData->pCNTIda->lCountSum) ); 
    usRC = CntWriteToFile( pData->pCNTIda, HWND_FUNCIF, usID );
    pData->hfDupReportXml = pData->pCNTIda->hfFileHandle;
  } /* endif */

  return( usRC );
} /* end of function WriteReport */

static void LogRC( USHORT usRC, PSZ pszFunction )
  {
    usRC; pszFunction;
#ifdef SESSIONLOG
    if ( usRC != 0 )
    {
      char szRC[10];
      sprintf( szRC, "%u", usRC );
      UtlLogWriteString2( "EqfProcessNomatch: Return code %s returned by function %s", szRC, pszFunction );
    }
#endif
  return;
  }


// write XML prefix
static BOOL WriteXMLPrefix( PMEM_PROCNOMATCH_DATA pData, FILE *hfOut, PSZ pszTargetLang )
{
  BOOL fOK = TRUE;
  CHAR szIso[20];

  fwrite( UTF8FILEPREFIX, strlen(UTF8FILEPREFIX), 1, hfOut );
  fOK = WriteInUTF8( pData, hfOut, L"<?xml version=\'1.0\'?>\r\n" );
  TAGetIsoLang( pszTargetLang, szIso );
  swprintf( pData->szUTF16Buffer, L"<rqt action=\"trqt\" tl=\"%S\" uid=\"xyz@us.ibm.com\">\r\n", szIso );
  if ( fOK ) fOK = WriteInUTF8( pData, hfOut, pData->szUTF16Buffer );
  if ( fOK ) fOK = WriteInUTF8( pData, hfOut, L"   <docs>\r\n" );
  return( fOK );
} /* end of function WriteXMLPrefix */

// escape characters in segment data to form valid XML 
static void EscapeChars( PSZ_W pszText, PSZ_W pszBuffer )
{
  while ( *pszText )
  {
    if ( *pszText == L'\n' )
    {
      wcscpy( pszBuffer, L" " );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'\r' )
    {
    }
    else if ( *pszText == L'&' )
    {
      wcscpy( pszBuffer, L"&amp;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'<' )
    {
      wcscpy( pszBuffer, L"&lt;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'>' )
    {
      wcscpy( pszBuffer, L"&gt;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'\"' )
    {
      wcscpy( pszBuffer, L"&quot;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else
    {
      *pszBuffer++ = *pszText;
    } /* endif */
    pszText++;
  } /*endwhile */
  *pszBuffer = 0;
}

// write UTF16 string in UTF8
static BOOL WriteInUTF8( PMEM_PROCNOMATCH_DATA pData, FILE *hfOut, PSZ_W pszUTF16 )
{
  BOOL fOK = TRUE;

  // convert to UTF-8
  int iOutLen = WideCharToMultiByte( CP_UTF8, 0, pszUTF16, -1, (LPSTR)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer), NULL, NULL );

  // write UTF-8 string to output file
  fwrite( pData->bUTF8Buffer, iOutLen - 1, 1, hfOut );

  return( fOK );
}



