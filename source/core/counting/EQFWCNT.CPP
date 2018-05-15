/*! \brief EQFWCNT.C - Program to count words of file
	Copyright (c) 1990-2014, International Business Machines Corporation and others. All rights reserved.
*/

// activate the following define to log wordcounts per seg in file C:\EQFWORDCOUNT.LOG"
#ifdef _DEBUG
  #define LOGWORDCOUNT

  // activate define for segment logging in duplicate word count
  // #define  WCOUNT_SEGMENT_LOGGING
#endif

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_FOLDER           // folder list and document list functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqfdde.h"               // batch mode defintions
  #include "OTMFUNC.H"            // function call interface public defines
  #include "eqffunci.h"           // function call interface private defines
#include "eqfwcnti.h"             // Private include file for wordcount
#include <process.h>              // C library for processes and threads
#include "eqfcolw.id"             // column width string IDs
#include "eqfserno.h"             // TM driver info
#include "EQFCNT01.ID"
#include "EQFWCNT.H"

#include <string>
#include <vector>
#include "core\pluginmanager\PluginManager.h"
#include "core\pluginmanager\OtmMemory.h"
#include "core\pluginmanager\OtmMemoryPlugin.h"
#include "core\memory\MemoryFactory.h"

// structure for CPP memory information
typedef struct _CNTMEMLOOKUPDATA
{
  OtmProposal      BestMatch;                    // best match found in memory lookup
  std::vector<OtmProposal *> FoundProposals;     // list of found proposals
  OtmMemory *pDocMem;                            // document memory database handle
  wchar_t szAddInfoBuffer[MAX_SEGMENT_SIZE+1];   // buffer for additional segment information
  std::vector<std::string> SearchMemoryNames;    // dynamically created list of search memory names
  std::vector<OtmMemory *> SearchMemoryDBs;      // dynamically created list of search memory databases
  OtmMemory *pDupMem;                            // memory for duplicate search
  char       szTempMemoryName[MAX_LONGFILESPEC+20]; // name of temporary memory
  char       szBuffer[2048];                     // general purpose buffer
} CNTMEMLOOKUPDATA, *PCNTMEMLOOKUPDATA;

/**********************************************************************/
/* prototypes                                                         */
/**********************************************************************/
__declspec(dllexport) int CntGetProposalClass( PSZ_W pszSegment, PSZ_W pszProposal, PLOADEDTABLE pTable, PBYTE pbBufer, PBYTE pbTokBuf, SHORT sLangID, ULONG ulOemCP, PUSHORT pusWords );
VOID EQFCNTProcess ( PCNTIDA );
static VOID FreeDoc ( PTBDOCUMENT pDoc );
static SHORT LoadCountFile ( PCNTIDA, USHORT );
static USHORT StoreCountResult ( HWND, PCNTIDA );
static USHORT StoreCountSummary ( HWND, PCNTIDA );
static USHORT SelectFileToCount ( PCNTIDA );
static VOID HandleTimer ( HWND, PCNTIDA );
static SHORT CountWords ( PCNTIDA, PSZ_W, SHORT, PULONG, PULONG, ULONG, ULONG );
static SHORT EQFWCount ( PCNTIDA );
static BOOL ActivateDocEnv ( PCNTIDA );
static VOID UnlockCountFiles( PCNTIDA );
static BOOL LockCountFiles( PCNTIDA);

MRESULT APIENTRY COUNT_WP( HWND, USHORT, WPARAM, LPARAM );
MRESULT EXPENTRY CountFrameSubProc( HWND, USHORT, WPARAM, LPARAM );
static  VOID     CountCreateWindow( PCNTIDA, HWND );
static  VOID     CreateColums( PCNTIDA, HWND);
static  HWND     CountHelpInit ( HAB, PCNTIDA );
static USHORT WCountGetMatchInfo( PCNTIDA pCNTIda, PTBSEGMENT pSeg, OtmMemory *pMem);
static SHORT WCountAddToMemory( PCNTIDA pCNTIda, PTBSEGMENT pSeg );
static SHORT WCountCreateTempMemory( PCNTIDA pCNTIda );
static SHORT WCountDeleteTempMemory( PCNTIDA pCNTIda );
static void EscapeCharacters( PSZ pszName, PSZ pszBuffer );
static SHORT WCountOpenFolderMemory( PCNTIDA pCNTIda );
static SHORT WCountCloseFolderMemory( PCNTIDA pCNTIda );
static SHORT WCountOpenDocMemory( PCNTIDA, PSZ );
static USHORT WCountGetFolMemMatchInfo( PCNTIDA pCNTIda, PTBSEGMENT pSeg );
static int WCountGetFuzzyClass( PCNTIDA  pCNTIda, PSZ_W pszSegment, OtmProposal *pProposal, PUSHORT pusWords );
static void WCountFreeMemLookupData( PCNTIDA pCNTIda );
USHORT CntRemoveTags( PLOADEDTABLE pTable, PSZ_W pszSegment, PSZ_W pszBuffer, PVOID pbTokBuf, LONG lTokBufSize, ULONG ulCP );

SHORT DoCounting ( PCNTIDA );

#ifdef WCOUNT_SEGMENT_LOGGING
  static FILE *hfLog = NULL;
#endif

// XSLT connversion located in EQFTAML.DLL
int XSLTConversion( PSZ pszXmlFile, PSZ pszHtmlFile, PSZ pszStyleSheet, BOOL fMsg, HWND hwndErrMsg );

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     EQFCNTProcess
//+----------------------------------------------------------------------------+
// Function call:     - will be passed as function address for Thread
//+----------------------------------------------------------------------------+
// Description:       This is the thread acting on all passed files.
//
//+----------------------------------------------------------------------------+
// Parameters:        PCNTIDA   pCNTIda   pointer to instance data
//+----------------------------------------------------------------------------+
// Returncode type:   VOID
//+----------------------------------------------------------------------------+
// Function flow:     initialize PM environment (necessary to displ. messages)
//                    while not fKill flag set
//                      if nothing to do
//                        sleep a while
//                      else
//                        load tag tables for QF tags and document
//                        if okay
//                          load files (source and target) to be counted
//                        endif
//                        if okay
//                          count file
//                        endif
//                        free allocated memory
//                        set status to finished, so main thread knows, he can
//                          use the data
//                      endif
//                    endwhile
//                    destroy PM environment
//+----------------------------------------------------------------------------+
VOID
EQFCNTProcess
(
PCNTIDA  pCNTIda                  // Initial Translation Memory ida
)
{
  SHORT  sRc = 0;                  // return code from EQFB functions
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  /********************************************************************/
  /* loop until killed from outside                                   */
  /********************************************************************/
  while ( ! pCNTIda->fKill )
  {
    if ( pCNTIda->usStatus == CNT_COUNTING )
    {
      /****************************************************************/
      /* now count the words                                          */
      /****************************************************************/
      sRc = DoCounting( pCNTIda );
    } /* endif */
  } /* endwhile */

  pCNTIda->fThreadActive = FALSE;      // thread is finished

} /* end of function EQFCNTProcess */

/**********************************************************************/
/* do the real counting....                                           */
/**********************************************************************/

SHORT DoCounting
(
PCNTIDA  pCNTIda                  // Count ida
)
{
  SHORT  sRc = 0;                  // return code from EQFB functions
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  /****************************************************************/
  /* reset error condition...                                     */
  /****************************************************************/
  sRc = 0;

#ifdef WCOUNT_SEGMENT_LOGGING
  if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
  {
    CHAR szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    strcat( szLogFile, "\\WCOUNTSEG.LOG" );
    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      fprintf( hfLog, "*** Counting document %s ***\n",  pCNTIda->szCountFileName );
    } /* endif */
  } /* endif */
#endif

  /****************************************************************/
  /* now count the words                                          */
  /* (no counting for MATCHCOUNT state, the results are stored in */
  /* the document property file)                                  */
  /****************************************************************/
  if ( !pCNTIda->fKill && (pCNTIda->pCnt->fOrg == TMMATCHES_STATE) )
  {
    // load document properties file and get any stored results
    HPROP         hPropDocument;       // handle to document properties
    ULONG         ulErrorInfo;         // error indicator from PRHA

    hPropDocument = OpenProperties( pCNTIda->szCountFileName,
                                    pCNTIda->pCnt->szFolderObjName,
                                    PROP_ACCESS_READ, &ulErrorInfo );
    if ( hPropDocument )
    {
      PPROPDOCUMENT pPropDocument;     // pointer to document properties
      pPropDocument = (PPROPDOCUMENT) MakePropPtrFromHnd( hPropDocument );

      if ( pPropDocument->ulTotal != 0L )
      {
        pCNTIda->lCountFile.ulWToBeTran     = pPropDocument->ulNoProps;
        pCNTIda->lCountFile.ulWTransMod     = pPropDocument->ulFuzzy;
        pCNTIda->lCountFile.ulWTransCopied  = pPropDocument->ulExactExact;
        pCNTIda->lCountFile.ulWTransScratch = pPropDocument->ulExactOne;
        pCNTIda->lCountFile.ulWMarkup       = pPropDocument->ulExactMore;
        pCNTIda->lCountFile.ulWTotal        = pPropDocument->ulTotal;
        pCNTIda->lCountFile.ulWTransSource  = pPropDocument->ulRepl;
        pCNTIda->lCountFile.ulFuzzyRepl     = pPropDocument->ulFuzzyRepl;
        pCNTIda->lCountFile.ulMachineMatch  = pPropDocument->ulMachineMatch;

        // GQ 20011125: removed the following check to fix KBT1105
        // if (pPropDocument->ulNoProps > 0xFFFF
        //  || pPropDocument->ulFuzzy > 0xFFFF
        //  || pPropDocument->ulExactExact > 0xFFFF
        //  || pPropDocument->ulExactOne > 0xFFFF
        //  || pPropDocument->ulExactMore > 0xFFFF
        //  || pPropDocument->ulTotal > 0xFFFF
        //  || pPropDocument->ulRepl > 0xFFFF
        //  || pPropDocument->ulFuzzyRepl > 0xFFFF
        //  || pPropDocument->ulMachineMatch > 0xFFFF)
        // {
        //    sprintf(szMessage, "Possibly incorrect data in %s", pCNTIda->szLongName);
        //     WinMessageBox (HWND_DESKTOP, HWND_DESKTOP, szMessage, "Warning", 0, MB_OK);
        // }
      }
      else
      {
        pCNTIda->lCountFile.ulWToBeTran     = 0L;
        pCNTIda->lCountFile.ulWTransMod     = 0L;
        pCNTIda->lCountFile.ulWTransCopied  = 0L;
        pCNTIda->lCountFile.ulWTransScratch = 0L;
        pCNTIda->lCountFile.ulWMarkup       = 0L;
        pCNTIda->lCountFile.ulWTotal        = 0L;
        pCNTIda->lCountFile.ulWTransSource  = 0L;
        pCNTIda->lCountFile.ulFuzzyRepl     = 0L;
        pCNTIda->lCountFile.ulMachineMatch  = 0L;
        sRc = FILE_NOINFO;
      } /* endif */

      CloseProperties( hPropDocument, PROP_QUIT, &ulErrorInfo );
    } /* endif */
  }
  else if ( !sRc && !pCNTIda->fKill )
  {
    if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) || (pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE) )
    {
      sRc = LoadCountFile( pCNTIda, DIRSEGSOURCEDOC_PATH );
      if ( !sRc && !pCNTIda->fKill )
      {
        sRc = LoadCountFile( pCNTIda, DIRSEGTARGETDOC_PATH );
      } /* endif */
    }
    else if ( pCNTIda->pCnt->fOrg )
    {
      sRc = LoadCountFile( pCNTIda, DIRSEGSOURCEDOC_PATH );
    }
    else
    {
      sRc = LoadCountFile( pCNTIda, DIRSEGSOURCEDOC_PATH );
      if ( !sRc && !pCNTIda->fKill )
      {
        sRc = LoadCountFile( pCNTIda, DIRSEGTARGETDOC_PATH );
      } /* endif */
    } /* endif */
    if ( !sRc && !pCNTIda->fKill )
    {
      /******************************************************************/
      /* init count structure and count the file                        */
      /******************************************************************/
      sRc = EQFWCount( pCNTIda );
    } /* endif */
  } /* endif */

  /****************************************************************/
  /* free any allocated resources                                 */
  /****************************************************************/
  if (pCNTIda->pCnt==NULL || pCNTIda->pCnt->fOrg != TMMATCHES_STATE )
  {
    FreeDoc( &pCNTIda->TBSrcDoc );
    FreeDoc( &pCNTIda->TBTgtDoc );
  } /* endif */

#ifdef WCOUNT_SEGMENT_LOGGING
  if ( hfLog )
  {
    fprintf( hfLog, "*** Counting complete ***\n" );
    fclose( hfLog );
    hfLog = NULL;
  } /* endif */
#endif

  /********************************************************************/
  /* set last error code                                              */
  /********************************************************************/
  pCNTIda->usRc  = sRc;
  if ( pCNTIda->pCnt != NULL )
  {
    if ( pCNTIda->pCnt->fBatch && pCNTIda->pCnt->pDDEWrdCnt )
    {
      if (sRc==FILE_NOINFO)
      {
        pCNTIda->pCnt->pDDEWrdCnt->DDEReturn.usRc = 0;
      }
      else
      {
        pCNTIda->pCnt->pDDEWrdCnt->DDEReturn.usRc = sRc;
      }
    } /* endif */
  } /* endif */
  pCNTIda->usStatus = CNT_FINISHED;          // we are ready for more work
  return sRc;
}


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     FreeDoc
//+----------------------------------------------------------------------------+
// Function call:     FreeDoc( pTBDoc );
//+----------------------------------------------------------------------------+
// Description:       Free the allocated space for the document structure
//+----------------------------------------------------------------------------+
// Parameters:        PTBDOCUMENT  pTBDoc   pointer to docum. instance data
//+----------------------------------------------------------------------------+
// Returncode type:   VOID
//+----------------------------------------------------------------------------+
// Function flow:     free buffers for input and tokens
//                    loop thru seg table and free all these buffers
//                    free segtable and undo segment
//                    return
//+----------------------------------------------------------------------------+
static
VOID FreeDoc
(
PTBDOCUMENT pDoc
)
{
  PTBSEGMENTTABLE pSegTable;          // ptr for segment table deleting
  ULONG           ulI, ulJ;           // loop counter
  PTBSEGMENT      pSegment;           // ptr for segment deleting
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
      if (pSegment->pusHLType )
      {
        UtlAlloc((PVOID *)&(pSegment->pusHLType) ,0L ,0L , NOMSG);
      } /* endif */
      if (pSegment->pContext) UtlAlloc((PVOID *)&(pSegment->pContext),0L,0L,NOMSG);
      if (pSegment->pvMetadata) UtlAlloc((PVOID *)&(pSegment->pvMetadata),0L,0L,NOMSG);
      pSegment++;
    } /* endfor */
    UtlAlloc( (PVOID *) &pSegTable->pSegments, 0L, 0L, NOMSG );
    pSegTable++;
  } /* endfor */
  pDoc->ulMaxSeg    = 0;                                            /*@1120A*/
  pDoc->ulSegTables = 0;
  UtlAlloc( (PVOID *) &pDoc->pSegTables, 0L, 0L, NOMSG );
  if (pDoc->pUndoSeg) UtlAlloc( (PVOID *) &pDoc->pUndoSeg, 0L, 0L, NOMSG );  //free storage of Undo
  if (pDoc->pUndoSegW) UtlAlloc( (PVOID *)&pDoc->pUndoSegW, 0L, 0L, NOMSG );
  if (pDoc->pContext) UtlAlloc((PVOID *)&(pDoc->pContext),0L,0L,NOMSG);
} /* end of function FreeDoc */


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     CNTInit
//+----------------------------------------------------------------------------+
// Function call:     CNTInit( pCNTIda );
//+----------------------------------------------------------------------------+
// Description:       This function will try to load the internal tag tables
//                    and start the processing thread
//+----------------------------------------------------------------------------+
// Parameters:        PCNTIDA  pCNTIda   pointer to instance data area
//+----------------------------------------------------------------------------+
// Returncode type:   VOID
//+----------------------------------------------------------------------------+
// Function flow:     set the internal tag table name and try to start the
//                    processing thread
//+----------------------------------------------------------------------------+
BOOL CNTInit
(
PCNTIDA pCNTIda
)
{
  BOOL    fOK = TRUE;                  // success indicator

  /**************************************************************/
  /* load internal table                                        */
  /**************************************************************/
  fOK = ( TALoadTagTableHwnd( QFTAG_TABLE,
                              (PLOADEDTABLE *)(&pCNTIda->TBSrcDoc.pQFTagTable),
                              TRUE, TRUE, pCNTIda->pCnt->hwndErrMsg ) == NO_ERROR );

  /**************************************************************/
  /* set the target document pointers to the same tables        */
  /**************************************************************/
  if ( fOK )
  {
    pCNTIda->TBTgtDoc.pQFTagTable = pCNTIda->TBSrcDoc.pQFTagTable;
  } /* endif */

  // allocate and prepare memory lookup area
  if ( fOK )
  {
    PCNTMEMLOOKUPDATA pMemData = new( CNTMEMLOOKUPDATA );
    pCNTIda->pvMemLookupData = (PVOID)pMemData;

    pMemData->pDocMem = NULL;
    pMemData->pDupMem = NULL; 
    
    for ( int i=0; i < 3; i++ )
    {
      pMemData->FoundProposals.push_back( new(OtmProposal) );
    } /* endif */        
  } /* end */     

  // for duplicate count only: create temporary memory
  if ( fOK && pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
  {
    fOK = (WCountCreateTempMemory( pCNTIda ) == 0);
  } /* endif */

  // for fuzzymatch count and duplicate count with memory info only: open folder mem(s)
  if ( fOK && ((((pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch)) || (pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE)) )
  {
    fOK = (WCountOpenFolderMemory( pCNTIda ) == 0);
  } /* endif */

  return( fOK );
} /* end of CNTInit */


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     LoadCountFile
//+----------------------------------------------------------------------------+
// Function call:     sRc = LoadCountFile( pCNTIda, usPath );
//+----------------------------------------------------------------------------+
// Description:       this function will create the fully qualified filename
//                    and will load the file into a document structure
//+----------------------------------------------------------------------------+
// Parameters:        PCNTIDA   pCNTIda      pointer to control block
//                    USHORT    usPath       id of path
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       DOS Error
//                    EQF Error
//+----------------------------------------------------------------------------+
// Function flow:     create path using passed parameters
//                    if file exist
//                      load it using EQFBFileRead into provided structure
//                    else
//                      display message 'file not analysed'
//                    endif
//                    return
//+----------------------------------------------------------------------------+
static SHORT
LoadCountFile
(
PCNTIDA   pCNTIda,                   // pointer to control block
USHORT    usPath
)
{
  SHORT sRc = 0;                       // success indicator
  PSZ   pszReplace;                    // temp variable for error box
  USHORT usResponse;                   // answer button of message box


  //--- build path to segmented source document directory
  UtlMakeEQFPath( pCNTIda->szCountFile,
                  pCNTIda->pCnt->szFolderObjName[0],
                  usPath,
                  pCNTIda->pCnt->szFolderName );

  //--- append filename to path
  strcat( pCNTIda->szCountFile, "\\" );
  strcat( pCNTIda->szCountFile, pCNTIda->szCountFileName );

  /**************************************************************/
  /* load file  and put them into segment structure             */
  /**************************************************************/
  if ( !pCNTIda->fKill )
  {
    if ( UtlFileExist( pCNTIda->szCountFile ) )
    {
      if ( usPath == DIRSEGSOURCEDOC_PATH )
      {
        sRc = EQFBFileRead( pCNTIda->szCountFile, &pCNTIda->TBSrcDoc);
        pCNTIda->TBSrcDoc.docType = SSOURCE_DOC;
      }
      else
      {
        sRc = EQFBFileRead( pCNTIda->szCountFile, &pCNTIda->TBTgtDoc);
        pCNTIda->TBTgtDoc.docType = STARGET_DOC;
      } /* endif */
    }
    else
    {
      /******************************************************************/
      /* display message and set error code if user wants to stop       */
      /******************************************************************/
      pszReplace = pCNTIda->szLongName;
      sRc = ERROR_WODCA_FILE_NOT_ANALYZED;
      if ( pCNTIda->pCnt->fBatch )
      {
        USHORT      usOpenAction;            // dummy for UtlOpen
        USHORT      usTempRC;                // temporary return code
        HFILE       hfSource = NULLHANDLE;   // file handle for source file
        ULONG       ulSize = 1;              // file size

        // check if this is a zero byte file
        UtlMakeEQFPath( pCNTIda->szStringBuffer, pCNTIda->pCnt->szFolderObjName[0], DIRSOURCEDOC_PATH, pCNTIda->pCnt->szFolderName );
        strcat( pCNTIda->szStringBuffer, BACKSLASH_STR );
        strcat( pCNTIda->szStringBuffer, pCNTIda->szCountFileName );

        usTempRC = UtlOpenHwnd( pCNTIda->szStringBuffer, &hfSource, &usOpenAction, 0L, FILE_NORMAL, FILE_OPEN, OPEN_ACCESS_READONLY, 0L, FALSE, HWND_FUNCIF );
        if ( !usTempRC )
        {
          usTempRC = UtlGetFileSize( hfSource, &ulSize, FALSE );
          UtlClose( hfSource, FALSE );
        } /* endif */

        if ( ulSize == 0 )
        {
          // end processing of this document but continue functions
          usResponse = MBID_YES;
          sRc = FILE_NOINFO;
        }
        else
        {
          // show error and stop process
          UtlErrorHwnd( sRc, MB_CANCEL, 1, &pszReplace, EQF_ERROR, pCNTIda->pCnt->hwndErrMsg );
          usResponse = MBID_NO;
        } /* endif */
      }
      else
      {
        usResponse = UtlError( sRc, MB_YESNO | MB_DEFBUTTON2, 1, &pszReplace, EQF_QUERY );
      } /* endif */

      pCNTIda->fKill = ( usResponse != MBID_YES );
    } /* endif */
  }
  else
  {
    sRc = ERROR_WODCA_FILE_NOT_ANALYZED;
  } /* endif */

  /*******************************************************************/
  /* let others work something...                                    */
  /*******************************************************************/
  if ( !sRc && (pCNTIda->pCnt->hwndErrMsg != HWND_FUNCIF))
  {
    UtlDispatch();
  } /* endif */
  return sRc;
} /* end of function LoadCountFile */


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFWCount
//+----------------------------------------------------------------------------+
// Function call:     EQFWCount     ( PCNTIDA     )
//+----------------------------------------------------------------------------+
// Description:       this function will set the status of the document
//                    i.e. if it is translated, how much is translated, etc.
//
//+----------------------------------------------------------------------------+
// Parameters:        PCNTIDA      pointer to controlblock
//+----------------------------------------------------------------------------+
// Returncode type:   VOID
//+----------------------------------------------------------------------------+
// Function flow:     loop thru all segments and update the sums of transl.
//                    copied and modified segments.
//                    Have in mind that you have to work with source and
//                    target segments to get correct results...
//+----------------------------------------------------------------------------+
static SHORT EQFWCount
(
  PCNTIDA      pCNTIda                 // control block
)
{
  ULONG   ulSegNum;                    // segment number
  ULONG   ulDummy  = 0;                // dummy
  SHORT   sRc = 0;                     // success indicator
  PLCOUNT plCount;                     // pointer to count array
  PTBSEGMENT  pSrcSeg;                 // ptr to segment
  PTBSEGMENT  pTgtSeg;                 // ptr to segment
  PTBDOCUMENT pSrcDoc = &pCNTIda->TBSrcDoc;  // pointer to document struct
  PTBDOCUMENT pTgtDoc = &pCNTIda->TBTgtDoc;  // pointer to document struct
  PSZ_W       pData;                   // pointer to data
  BOOL        fWhiteSpace;             // tag only white space characters

  plCount = &pCNTIda->lCountFile;      // pointer to count array

  memset( plCount, 0, sizeof(LCOUNT) ); // reset sum fields

  ulSegNum = 1;
  pSrcSeg = SegFileGetVisSeg(pSrcDoc, &ulSegNum);
  if ( !pCNTIda->pCnt->fOrg || (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) || (pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE) )
  {
    pTgtSeg = SegFileGetVisSeg(pTgtDoc, &ulSegNum);
  }
  else
  {
    pTgtSeg = pSrcSeg;
  } /* endif */

  while ( pTgtSeg && !pCNTIda->fKill && !sRc )
  {
    if (!pTgtSeg->SegFlags.NoCount )
    {
      if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
      {
        if ( (pTgtSeg->qStatus == QF_ATTR) || (pTgtSeg->qStatus == QF_TOBE) || (pTgtSeg->qStatus == QF_CURRENT) )
        {
          // get best match from folder memories
          USHORT usMatch = WCountGetFolMemMatchInfo( pCNTIda, pSrcSeg );

          // we are only interested in fuzzy matches
          if ( (usMatch > 0) && (usMatch < 100) )
          {
            // get class of current match and update counters
            PCNTMEMLOOKUPDATA pMemData = (PCNTMEMLOOKUPDATA)pCNTIda->pvMemLookupData;
            USHORT usWords = 0;
            int iClass = WCountGetFuzzyClass( pCNTIda, pSrcSeg->pDataW, &(pMemData->BestMatch), &usWords );
            if ( iClass >= 0 )
            {
              ULONG ulSegWords = 0;
              ULONG ulMarkup = 0;
              USHORT usFuzzyThreshold = 0;

              if ( usWords > 15 )
        	    {
        		    usFuzzyThreshold = (USHORT)(UtlQueryULong( QL_LARGEFUZZLEVEL ) / 100);
        	    }
        	    else if ( usWords > 4 )
        	    {
          	    usFuzzyThreshold = (USHORT)(UtlQueryULong( QL_MEDIUMFUZZLEVEL ) / 100);
        	    }
        	    else
        	    {
        		    usFuzzyThreshold = (USHORT)(UtlQueryULong( QL_SMALLFUZZLEVEL ) / 100);
        	    } /* endif */

              if ( usMatch >= usFuzzyThreshold )
              {
              CountWords( pCNTIda, pSrcSeg->pDataW, pCNTIda->pCnt->sSrcLanguage, &ulSegWords, &ulMarkup, pCNTIda->pCnt->ulSrcOemCP, ulSegNum );

              if ( iClass >= MAX_CLASS_COLS ) iClass = MAX_CLASS_COLS - 1;

              plCount->ulWClass[iClass] += ulSegWords;
              plCount->ulWTotal += ulSegWords;
            } /* endif */
          } /* endif */
        } /* endif */
        } /* endif */
      }
      else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
      {
        switch ( pTgtSeg->qStatus )
        {
          case  QF_NOP:
            /* check if string is only white space                        */
            fWhiteSpace = TRUE;
            pData = pTgtSeg->pDataW;
            while ( *pData && fWhiteSpace ) fWhiteSpace = iswspace( *pData++ );
            // if ( !fWhiteSpace ) plCount->ulWMarkup += 1;
            break;

          case  QF_ATTR:
          case  QF_TOBE:
          case  QF_CURRENT:
            {
              ULONG ulSegWords = 0;

              // count words of segment
              sRc = CountWords( pCNTIda, pSrcSeg->pDataW,
                                pCNTIda->pCnt->sSrcLanguage,
                                &ulSegWords, &(plCount->ulWMarkup), pCNTIda->pCnt->ulSrcOemCP, ulSegNum );
              plCount->ulWUntranslated += ulSegWords;
              //plCount->ulWMarkup += ulSegMarkup;

              // check memory and add words to approbriate sum
              if ( pCNTIda->pCnt->fInclMemMatch )
              {
                // get best match
                PCNTMEMLOOKUPDATA pMemData = (PCNTMEMLOOKUPDATA)pCNTIda->pvMemLookupData;
                USHORT usFolMemMatch = WCountGetFolMemMatchInfo( pCNTIda, pSrcSeg );
                USHORT usTempMatch = WCountGetMatchInfo( pCNTIda, pSrcSeg, pMemData->pDupMem );
                USHORT usMatch = usTempMatch;
                if ( usFolMemMatch > usMatch ) usMatch = usFolMemMatch;

                // store in approbriate fuzzy column
                if ( usMatch < 75 )
                {
                  plCount->ulWFuzzy[0] += ulSegWords;
                }
                else if ( usMatch < 95 )
                {
                  plCount->ulWFuzzy[1] += ulSegWords;
                }
                else if ( usMatch < 100 )
                {
                  plCount->ulWFuzzy[2] += ulSegWords;
                }
                else if ( usFolMemMatch >= 100 )
                {
                  plCount->ulWTran += ulSegWords;
                }
                else
                {
                  plCount->ulWDuplicate += ulSegWords;
                  plCount->ulWToBeTran += ulSegWords;          // add to our total field
                } /* endif */

                if ( (usFolMemMatch < 100) && (usTempMatch < 100) )
                {
                  sRc = WCountAddToMemory( pCNTIda, pSrcSeg );
                  plCount->ulWToBeTran += ulSegWords;          // add to our total field
                } /* endif */

#ifdef WCOUNT_SEGMENT_LOGGING
                if ( hfLog )
                {
                  fprintf( hfLog, "Seg: %10lu, Status: UT, Words: %4lu, TempMatch = %3u, FolMemMatch = %3u\n", pSrcSeg->ulSegNum,
                       ulSegWords, usTempMatch, usFolMemMatch );
                } /* endif */
#endif
              }
              else
              {
                // check memories to verify that there are no exact matches for this segment
                PCNTMEMLOOKUPDATA pMemData = (PCNTMEMLOOKUPDATA)pCNTIda->pvMemLookupData;
                USHORT usFolMemMatch = WCountGetFolMemMatchInfo( pCNTIda, pSrcSeg );
                USHORT usMatch = WCountGetMatchInfo( pCNTIda, pSrcSeg, pMemData->pDupMem );
                if ( (usFolMemMatch >= 100) && (pMemData->FoundProposals[0]->getType() == OtmProposal::eptManual) )
                {
                  // ignore this segment as there is an exact match available
                  plCount->ulWToBeTran += ulSegWords;
                }
                else if ( usMatch >= 100 )
                {
                  plCount->ulWDuplicate += ulSegWords;
                }
                else
                {
                  sRc = WCountAddToMemory( pCNTIda, pSrcSeg );
                  plCount->ulWToBeTran += ulSegWords;
                } /* endif */
#ifdef WCOUNT_SEGMENT_LOGGING
                if ( hfLog )
                {
                  fprintf( hfLog, "Seg: %10lu, Status: UT, Words: %4lu, TempMatch = %3u\n", pSrcSeg->ulSegNum,
                       ulSegWords, usMatch );
                } /* endif */
#endif
              } /* endif */
            }
            break;

          case  QF_XLATED:
            {
              ULONG ulSegWords = 0;
              sRc = CountWords( pCNTIda, pSrcSeg->pDataW,
                                pCNTIda->pCnt->sSrcLanguage,
                                &ulSegWords, &(plCount->ulWMarkup), pCNTIda->pCnt->ulSrcOemCP, ulSegNum  );
              plCount->ulWTran += ulSegWords;
#ifdef WCOUNT_SEGMENT_LOGGING
              if ( hfLog )
              {
                fprintf( hfLog, "Seg: %10lu, Status: XL, Words: %4lu\n", pSrcSeg->ulSegNum, ulSegWords );
              } /* endif */
#endif
            }
            break;
          default :
            break;
        } /*endswitch */
      }
      else
      {
        switch ( pTgtSeg->qStatus )
        {
          case  QF_NOP:                             // segment not counted
            /**************************************************************/
            /* check if string is only white space                        */
            /**************************************************************/
            fWhiteSpace = TRUE;
            pData = pTgtSeg->pDataW;
            while ( *pData && fWhiteSpace )
            {
              fWhiteSpace = iswspace( *pData++ );
            } /* endwhile */

            if ( !fWhiteSpace )
            {
              plCount->ulWMarkup += 1;
            } /* endif */
            break;

          case  QF_ATTR:
            sRc = CountWords( pCNTIda, pSrcSeg->pDataW,
                              pCNTIda->pCnt->sSrcLanguage,
                              &(plCount->ulWToBeTran), &(plCount->ulWMarkup), pCNTIda->pCnt->ulSrcOemCP, ulSegNum );
            /**************************************************************/
            /* the following line removed due to @KIT0885                 */
            /**************************************************************/
            //        ulSumUnTrans++;          // QF_ATTR may contain a word to be transl.
            break;
          case  QF_TOBE:
          case  QF_CURRENT:
            sRc = CountWords( pCNTIda, pSrcSeg->pDataW,
                              pCNTIda->pCnt->sSrcLanguage,
                              &(plCount->ulWToBeTran), &(plCount->ulWMarkup), pCNTIda->pCnt->ulSrcOemCP, ulSegNum );
            break;

          case  QF_XLATED:
            sRc = CountWords( pCNTIda, pSrcSeg->pDataW,
                              pCNTIda->pCnt->sSrcLanguage,
                              &(plCount->ulWTransSource), &ulDummy, pCNTIda->pCnt->ulSrcOemCP, ulSegNum  );
            if ( !sRc )
            {
              if ( pTgtSeg->SegFlags.Typed )
              {
                if ( pTgtSeg->SegFlags.Copied )
                {
                  sRc = CountWords( pCNTIda, pTgtSeg->pDataW,
                                    pCNTIda->pCnt->sTgtLanguage,
                                    &(plCount->ulWTransMod), &(plCount->ulWMarkup), pCNTIda->pCnt->ulTgtOemCP, ulSegNum );
                }
                else
                {
                  sRc = CountWords( pCNTIda, pTgtSeg->pDataW,
                                    pCNTIda->pCnt->sTgtLanguage,
                                    &(plCount->ulWTransScratch), &(plCount->ulWMarkup), pCNTIda->pCnt->ulTgtOemCP, ulSegNum );
                } /* endif */
              }
              else if ( pTgtSeg->SegFlags.Copied )
              {
                sRc = CountWords( pCNTIda, pTgtSeg->pDataW,
                                  pCNTIda->pCnt->sTgtLanguage,
                                  &(plCount->ulWTransCopied), &(plCount->ulWMarkup), pCNTIda->pCnt->ulTgtOemCP, ulSegNum );
              } /* endif */
            } /* endif */
            break;
          default :
            break;
        } /* endswitch */
      } /* endif */
    } /* endif */
    ulSegNum ++;
    pSrcSeg = SegFileGetVisSeg(pSrcDoc, &ulSegNum);
    if ( !pCNTIda->pCnt->fOrg || (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) || (pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE))
    {
      pTgtSeg = SegFileGetVisSeg(pTgtDoc, &ulSegNum);
    }
    else
    {
      pTgtSeg = pSrcSeg;
    } /* endif */

    if ( pCNTIda->pCnt->hwndErrMsg != HWND_FUNCIF )
    {
      UtlDispatch();
    } /* endif */
  } /* endwhile */

  // build totals
  if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
  {
    // do not count markup in DulicateWordCount with MatchInfo and in FuzzyMatch
    plCount->ulWMarkup = 0;
  }
  else if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
  {
    plCount->ulWTotal = plCount->ulWToBeTran;
    plCount->ulWTotal += plCount->ulWTransMod;
    plCount->ulWTotal += plCount->ulWTransCopied;
    plCount->ulWTotal += plCount->ulWTransScratch;
    // do not count markup in DulicateWordCount with MatchInfo and in FuzzyMatch
    plCount->ulWMarkup = 0;
  }
  else
  {
    plCount->ulWTotal = plCount->ulWToBeTran;
    plCount->ulWTotal += plCount->ulWTransMod;
    plCount->ulWTotal += plCount->ulWTransCopied;
    plCount->ulWTotal += plCount->ulWTransScratch;
    plCount->ulWTotal += plCount->ulWMarkup;
  } /* endif */

  return sRc;
} /* end of function EQFWCount */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     CountWords
//+----------------------------------------------------------------------------+
// Function call:     sRc = CountWords( pCNTIda, pData, sLanguageId,
//                                      pulResult, pulMarkUp );
//+----------------------------------------------------------------------------+
// Description:       This function counts the words and tags in the passed
//                    data area taking into account the used language ( POE !)
//+----------------------------------------------------------------------------+
// Parameters:        PCNTIDA      pCNTIda,      pointer to Count controlblock
//                    PSZ          pData,        pointer to data
//                    SHORT        sLanguageId,  language ID
//                    PULONG       pulResult,    result to be counted
//                    PULONG       pulMarkUp     result for markup
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       error return from POE or memory shortage
//+----------------------------------------------------------------------------+
// Function flow:     tokenize the passed string using POE
//                    loop thru list of tokens
//                     if text token
//                       decomposite it using MorphTokenize
//                     else
//                       increase markup (if last token was not markup)
//                     endif
//                     get next token
//                    endloop
//                    return success indicator
//
//+----------------------------------------------------------------------------+
static SHORT
CountWords
(
  PCNTIDA      pCNTIda,                // pointer to Count controlblock
  PSZ_W        pData,                  // pointer to data
  SHORT        sLanguageId,            // language ID
  PULONG       pulResult,              // result to be counted
  PULONG       pulMarkUp,              // result for markup
  ULONG        ulOemCP,
  ULONG        ulSegNum                // number of current segment
)
{
  BOOL          sRc = 0;               // success indicator
  PSZ           pRest;                 // ptr to start of not-processed bytes
  USHORT        usResponse;            // error return

#ifdef LOGWORDCOUNT
  ULONG ulOldWords = *pulResult;
  ULONG ulOldMarkup = *pulMarkUp;
#endif

  ulSegNum;

  sRc = EQFBWordCntPerSeg( (PLOADEDTABLE)pCNTIda->TBSrcDoc.pDocTagTable,
                           (PTOKENENTRY) pCNTIda->TBSrcDoc.pTokBuf,
                           pData, sLanguageId, pulResult, pulMarkUp, ulOemCP);
#ifdef LOGWORDCOUNT
  if ( !sRc )
  {
    CHAR szLog[MAX_EQF_PATH];
    FILE *hfLog = NULL;

    UtlMakeEQFPath( szLog, NULC, LOG_PATH, NULL );
    strcat( szLog, "\\EQFWordCount.log" );
    hfLog = fopen( szLog, "a" );
    if ( hfLog )
    {
      fprintf( hfLog, "Seg %8lu: words: %8lu  markup: %8lu\n", ulSegNum,
               *pulResult - ulOldWords, *pulMarkUp - ulOldMarkup );
     fclose( hfLog );
    } /* endif */
  } /* endif */
#endif
  if ( sRc )
  {
    //display error message: error counting file continue ? Yes, No
    pRest = pCNTIda->szLongName;
    if ( pCNTIda->pCnt->fBatch )
    {
      UtlErrorHwnd( ERROR_WODCA, MB_CANCEL, 1, &pRest, EQF_ERROR, pCNTIda->pCnt->hwndErrMsg );
      pCNTIda->fKill = TRUE;
    }
    else
    {
      usResponse =  UtlError( ERROR_WODCA, MB_YESNO | MB_DEFBUTTON2,
                              1, &pRest, EQF_QUERY );

      pCNTIda->fKill = ( usResponse != MBID_YES );
    } /* endif */
  } /* endif */
  return ((SHORT)sRc);
} /* end of function CountWords */


//+----------------------------------------------------------------------------+
// Function name: CntAbbrevFileName
//+----------------------------------------------------------------------------+
// Function call:
//
//+----------------------------------------------------------------------------+
// Description:
//
//+----------------------------------------------------------------------------+
// Input parameter:
//
//+----------------------------------------------------------------------------+
// Output parameter:
//
//+----------------------------------------------------------------------------+
// Returncode type:
//+----------------------------------------------------------------------------+
// Returncodes:
//
//+----------------------------------------------------------------------------+
// Prerequesits:
//
//+----------------------------------------------------------------------------+
// Side effects:
//
//+----------------------------------------------------------------------------+
// Samples:
//
//+----------------------------------------------------------------------------+
// Function flow:
//
//
//
//+----------------------------------------------------------------------------+

static VOID CntAbbrevFileName
(
PSZ      pszAbbrFile,
USHORT   usMax,
PSZ      pszInFile1
)
{
  ULONG            ulInFileLen;
  CHAR             chStop;
  PSZ              pszEnd;
  ULONG            ulLen;
  USHORT           usI = 0;
  USHORT           usBack = 0;
  USHORT           uCut;
  CHAR             szInFile[265];
  PSZ              pszInFile;

  strcpy(szInFile,pszInFile1);
  pszInFile = &szInFile[0];


  ulInFileLen = strlen(pszInFile);
  strcpy(pszAbbrFile,pszInFile1);
  if (ulInFileLen>usMax )
  {

    // Estimated Cut Value
    if (usMax<17 )
    {
      uCut = (USHORT) (0.3 * usMax);
    }
    else if (usMax<30 )
    {
      uCut = (USHORT) (0.4 * usMax);
    }
    else
    {
      uCut = (USHORT) (0.5 * usMax);
    } /* endif */

    // check with Backslash number
    while (usI<uCut && (usBack < 2))
    {
      if (*(pszInFile+usI ) == BACKSLASH )
      {
        usBack ++;
      } /* endif */
      usI++;
    } /* endwhile */
    chStop = pszInFile[usI];
    pszInFile[usI] = EOS;

    // remaining file name
    if (usMax < 17 )
    {
      ulLen = ulInFileLen-(usMax-(usI+1));
    }
    else
    {
      ulLen = ulInFileLen-(usMax-(usI+3));
    } /* endif */

    pszEnd = pszInFile + ulLen;

    if (usMax < 17 )
    {
      sprintf( pszAbbrFile, "%s~%s", pszInFile,pszEnd);
    }
    else
    {
      sprintf( pszAbbrFile, "%s...%s", pszInFile,pszEnd);
    } /* end if */

    // recover pszInFile
    pszInFile[usI] = chStop;


  } /* endif */

  return ;


} /* end of function CntAbbrevFileName */





//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     WriteToFile
//+----------------------------------------------------------------------------+
// Function call:     usRc = WriteToFile( pCNTIda, hwnd, usType );
//+----------------------------------------------------------------------------+
// Description:       create lines for output file and write it to file
//+----------------------------------------------------------------------------+
// Parameters:        pCNTIda       pointer to count instance IDA
//                    hwnd          window handle
//                    usType        type of line to be created
//                                  possible valuse are:
//                                  HEADER
//                                  ERROR_LINE
//                                  FILE_COUNT_LINE
//                                  SUMMARY_LINE
//+----------------------------------------------------------------------------+
// Returncode type:   USHORT
//+----------------------------------------------------------------------------+
// Returncodes:
//+----------------------------------------------------------------------------+
// Function flow:
//+----------------------------------------------------------------------------+
USHORT CntWriteToFile
(
PCNTIDA           pCNTIda,
HWND              hwnd,
USHORT            usType
)
{
  ULONG    ulBytes;                   //number of bytes written
  ULONG    ulNumBytes;                //number of bytes to be written
  USHORT   usRc = 0;                  //return code from UtlWrite
  CHAR     szReplace[10];             //replace string for error message
  PSZ      pszReplace = szReplace;    //pointer to replace string
  HAB      hab;                       // anchor block handle
  ULONG    ulWords;                   // temp variable
  USHORT   usResponse = MBID_YES;     // user wants to go on

  BOOL     fXML = pCNTIda->pCnt->fXMLOutput;

  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  if ( pCNTIda->pCnt->hwndErrMsg == HWND_FUNCIF )
  {
    hab = NULLHANDLE;
  }
  else
  {
    hab = (HAB) WinQueryAnchorBlock(hwnd);
  } /* endif */
  switch ( usType )
  {
    //-----------------------------------------------------------------------
    case HEADER :
      //--- create general header in dependency of original/translation
      //--- and document type
      if ( fXML )
      {
        LONG lTime;


        EscapeCharacters( pCNTIda->pCnt->szLongFolderName, pCNTIda->szEscapeBuffer );
        strcpy( pCNTIda->szBuffer, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<?xml-stylesheet type=\"text/xsl\" href=\"CNT2HTML.XSL\" ?>\r\n<tmwordcount>\r\n" );
        strcat( pCNTIda->szBuffer, "  <description>\r\n" );
        strcat( pCNTIda->szBuffer, "    <report type=\"WordCount\" counted=\"" );
        if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
        {
          strcat( pCNTIda->szBuffer, "Duplicates" );
        }
        else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE )
        {
          strcat( pCNTIda->szBuffer, "MemoryMatch" );
        }
        else if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
        {
          strcat( pCNTIda->szBuffer, "FuzzyMatch" );
        }
        else if ( pCNTIda->pCnt->fOrg )
        {
          strcat( pCNTIda->szBuffer, "Source" );
        }
        else
        {
          strcat( pCNTIda->szBuffer, "Target" );
        } /* endif */
        strcat( pCNTIda->szBuffer, "\" generatedDate=\"" );
        UtlTime( &lTime );
        UtlLongToDateString( lTime, pCNTIda->szBuffer + strlen(pCNTIda->szBuffer), 30 );
        strcat( pCNTIda->szBuffer, "\" generatedTime=\"" );
        UtlLongToTimeString( lTime, pCNTIda->szBuffer + strlen(pCNTIda->szBuffer), 30 );
        sprintf( pCNTIda->szBuffer + strlen(pCNTIda->szBuffer), "\" documentsSelected=\"%d\" />\r\n", pCNTIda->pCnt->usNoOfDocs );
        sprintf( pCNTIda->szBuffer + strlen(pCNTIda->szBuffer), "    <folder nameLong=\"%s\" nameShort=\"%s\" documentsAmount=\"%d\" />\r\n",
          pCNTIda->szEscapeBuffer, pCNTIda->pCnt->szFolderName, pCNTIda->pCnt->usNoOfDocs );
        sprintf( pCNTIda->szBuffer + strlen(pCNTIda->szBuffer), "    <translationManager version=\"%d\" release=\"%d\" build=\"%d\" />\r\n",
                 EQF_DRIVER_VERSION, EQF_DRIVER_RELEASE, EQF_DRIVER_SUBRELEASE );
        strcat( pCNTIda->szBuffer, "  </description>\r\n" );
        strcat( pCNTIda->szBuffer, "  <documents>\r\n" );
      }
      else
      {
        if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
        {
          strcpy( pCNTIda->szBuffer, "Word Count results - Duplicate Word Count with memory match information" );
        }
        else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE)
        {
          strcpy( pCNTIda->szBuffer, "Word Count results - Duplicate Word Count" );
        }
        else if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE)
        {
          strcpy( pCNTIda->szBuffer, "Word Count results - Fuzzy Match Word Count" );
        }
        else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE)   //--- count TM matches
        {
          WinLoadString( hab, hResMod,
                        SID_CNT00_TITLE_TMMATCHES,
                        sizeof( pCNTIda->szBuffer ),
                        pCNTIda->szBuffer );
        }
        else if ( pCNTIda->pCnt->fOrg )   //--- count originals
        {
          //--- load header text for counting originals
          WinLoadString( hab, hResMod,
                        SID_CNT00_TITLE_ORIGINAL,
                        sizeof( pCNTIda->szBuffer ),
                        pCNTIda->szBuffer );
        }
        else   //--- count translations
        {
          //--- load header text for counting translations
          WinLoadString( hab, hResMod,
                        SID_CNT00_TITLE_TRANSLATION,
                        sizeof( pCNTIda->szBuffer ),
                        pCNTIda->szBuffer );
        }/*endif*/

        //--- load folder text to output buffer
        WinLoadString( hab, hResMod,
                      SID_CNT00_FOLDER_TEXT,
                      sizeof( pCNTIda->szFileLine ),
                      pCNTIda->szFileLine );

        /*************************************************************/
        /* prepare output string                                     */
        /*************************************************************/
        if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
        {
          sprintf( pCNTIda->szStringBuffer, "%s%s%s%s%s%s",
                  CRLF_STR,
                  pCNTIda->szFileLine,
                  pCNTIda->pCnt->szDisplayName[0] ?
                    pCNTIda->pCnt->szDisplayName :
                    pCNTIda->pCnt->szLongFolderName,
                  CRLF_STR,
                  EQUAL_LINE_MEMINFODUPLICATE,
                  CRLF_STR );
        }
        else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE)
        {
          sprintf( pCNTIda->szStringBuffer, "%s%s%s%s%s%s",
                  CRLF_STR,
                  pCNTIda->szFileLine,
                  pCNTIda->pCnt->szDisplayName[0] ?
                    pCNTIda->pCnt->szDisplayName :
                    pCNTIda->pCnt->szLongFolderName,
                  CRLF_STR,
                  EQUAL_LINE_DUPLICATE,
                  CRLF_STR );
        }
        else if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE)
        {
          sprintf( pCNTIda->szStringBuffer, "%s%s%s%s%s%s",
                  CRLF_STR,
                  pCNTIda->szFileLine,
                  pCNTIda->pCnt->szDisplayName[0] ?
                    pCNTIda->pCnt->szDisplayName :
                    pCNTIda->pCnt->szLongFolderName,
                  CRLF_STR,
                  EQUAL_LINE_DUPLICATE,
                  CRLF_STR );
        }
        else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE)   //--- count TM matches
        {
          if ( pCNTIda->pCnt->fReplMatch )
          {
            sprintf( pCNTIda->szStringBuffer, "%s%s%s%s%s%s",
                    CRLF_STR,
                    pCNTIda->szFileLine,
                    pCNTIda->pCnt->szDisplayName[0] ?
                      pCNTIda->pCnt->szDisplayName :
                      pCNTIda->pCnt->szLongFolderName,
                    CRLF_STR,
                    EQUAL_LINE_TMREPLMATCH,
                    CRLF_STR );
          }
          else
          {
            sprintf( pCNTIda->szStringBuffer, "%s%s%s%s%s%s",
                    CRLF_STR,
                    pCNTIda->szFileLine,
                    pCNTIda->pCnt->szDisplayName[0] ?
                      pCNTIda->pCnt->szDisplayName :
                      pCNTIda->pCnt->szLongFolderName,
                    CRLF_STR,
                    EQUAL_LINE_TMMATCH,
                    CRLF_STR );
          } /* endif */
        }
        else
          if ( pCNTIda->pCnt->fOrg )   //--- count originals
        {
          sprintf( pCNTIda->szStringBuffer, "%s%s%s%s%s%s",
                  CRLF_STR,
                  pCNTIda->szFileLine,
                  pCNTIda->pCnt->szDisplayName[0] ?
                    pCNTIda->pCnt->szDisplayName :
                    pCNTIda->pCnt->szLongFolderName,
                  CRLF_STR,
                  EQUAL_LINE_SHORT,
                  CRLF_STR );
        }
        else   //--- count translations
        {
          sprintf( pCNTIda->szStringBuffer, "%s%s%s%s%s%s",
                  CRLF_STR,
                  pCNTIda->szFileLine,
                  pCNTIda->pCnt->szDisplayName[0] ?
                    pCNTIda->pCnt->szDisplayName :
                    pCNTIda->pCnt->szLongFolderName,
                  CRLF_STR,
                  EQUAL_LINE_LONG,
                  CRLF_STR );
        } /* endif */

        //--- append to output buffer
        strcat ( pCNTIda->szBuffer, pCNTIda->szStringBuffer );

        //--- call utility to create colum line
        CreateColums( pCNTIda, hwnd);
      } /* endif */
      break;
      //-----------------------------------------------------------------------
    case ERROR_LINE :    //--- create error line
      if ( fXML )
      {
        PSZ pszOut = pCNTIda->szBuffer;
        EscapeCharacters( pCNTIda->szLongName, pCNTIda->szEscapeBuffer );
        sprintf( pszOut, "      <document nameLong=\"%s\" nameShort=\"%s\">\r\n", pCNTIda->szEscapeBuffer, pCNTIda->szCountFileName );
        pszOut += strlen(pszOut);
        strcpy( pszOut, "        <error-info>counting failed</error-info>\r\n" );
        pszOut += strlen(pszOut);
        strcpy( pszOut, "      </document>\r\n" );
        pszOut += strlen(pszOut);
      }
      else
      {
        //--- load error string
        WinLoadString( hab, hResMod, SID_CNT00_FILE_ERROR_LINE, sizeof( pCNTIda->szErrorTextBuffer ), pCNTIda->szErrorTextBuffer );
        sprintf( pCNTIda->szBuffer, "%s%s%s", pCNTIda->szLongName, pCNTIda->szErrorTextBuffer, CRLF_STR );
      } /* endif */
      break;
      //-------------------------------------------------------------
    case LOCKED_LINE :    //--- create line : "!!!!FIle use"
      if ( fXML )
      {
        PSZ pszOut = pCNTIda->szBuffer;
        EscapeCharacters( pCNTIda->szLongName, pCNTIda->szEscapeBuffer );
        sprintf( pszOut, "      <document nameLong=\"%s\" nameShort=\"%s\">\r\n", pCNTIda->szEscapeBuffer, pCNTIda->szCountFileName );
        pszOut += strlen(pszOut);
        strcpy( pszOut, "        <error-info>document is currently in use</error-info>\r\n" );
        pszOut += strlen(pszOut);
        strcpy( pszOut, "      </document>\r\n" );
        pszOut += strlen(pszOut);
      }
      else
      {
        WinLoadString( hab, hResMod, SID_CNT00_FILE_LOCKED_LINE, sizeof( pCNTIda->szErrorTextBuffer ), pCNTIda->szErrorTextBuffer );
        sprintf( pCNTIda->szBuffer, "%s%s%s", pCNTIda->szLongName, pCNTIda->szErrorTextBuffer, CRLF_STR );
      } /* endif */
      break;
      //-----------------------------------------------------------------------
    case NOINFO_LINE :    //--- create line : "!!!! No match info"
      if ( fXML )
      {
        PSZ pszOut = pCNTIda->szBuffer;
        EscapeCharacters( pCNTIda->szLongName, pCNTIda->szEscapeBuffer );
        sprintf( pszOut, "      <document nameLong=\"%s\" nameShort=\"%s\">\r\n", pCNTIda->szEscapeBuffer, pCNTIda->szCountFileName );
        pszOut += strlen(pszOut);
        strcpy( pszOut, "        <error-info>no count info available</error-info>\r\n" );
        pszOut += strlen(pszOut);
        strcpy( pszOut, "      </document>\r\n" );
        pszOut += strlen(pszOut);
      }
      else
      {
        WinLoadString( hab, hResMod,
                      SID_CNT00_FILE_NOINFO_LINE,
                      sizeof( pCNTIda->szErrorTextBuffer ),
                      pCNTIda->szErrorTextBuffer );

        CntAbbrevFileName( pCNTIda->szShortenedName,42,&(pCNTIda->szLongName[0]));
        sprintf( pCNTIda->szBuffer, "%-42.42s%s%s",
                pCNTIda->szShortenedName,
                pCNTIda->szErrorTextBuffer,
                CRLF_STR );
      } /* endif */
      break;
      //-----------------------------------------------------------------------
    case FILE_COUNT_LINE :
      if ( fXML )
      {
        PSZ pszOut = pCNTIda->szBuffer;

        EscapeCharacters( pCNTIda->szLongName, pCNTIda->szEscapeBuffer );
        sprintf( pszOut, "    <document nameLong=\"%s\" nameShort=\"%s\">\r\n", pCNTIda->szEscapeBuffer, pCNTIda->szCountFileName );
        pszOut += strlen(pszOut);
        strcpy( pszOut, "      <wordcount>\r\n" );
        pszOut += strlen(pszOut);

        if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
        {
          sprintf( pszOut, "        <words type=\"Total\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWTotal );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-zero\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWClass[0] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-1\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWClass[1] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-2\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWClass[2] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-3\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWClass[3] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-4\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWClass[4] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-5\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWClass[5] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-6up\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWClass[6] );
          pszOut += strlen(pszOut);
        }
        else if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
        {
          sprintf( pszOut, "        <words type=\"DuplicateWords\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWDuplicate );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Fuzzy9599\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWFuzzy[2] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Fuzzy7594\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWFuzzy[1] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Fuzzy0074\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWFuzzy[0] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Total\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWTotal );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Exact\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWTran );
          pszOut += strlen(pszOut);
        }
        else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
        {
          sprintf( pszOut, "        <words type=\"Translated\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWTran );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Untranslated\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWUntranslated );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"DuplicateWords\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWDuplicate );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"ToBeTranslated\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWToBeTran );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Markup\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWMarkup  );
          pszOut += strlen(pszOut);
        }
        else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE )
        {
          sprintf( pszOut, "        <words type=\"Total\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWTotal );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"ExactExact\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWTransCopied  );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Exact-1\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWTransScratch);
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Exact-2\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWMarkup  );
          pszOut += strlen(pszOut);
          if ( pCNTIda->pCnt->fReplMatch )
          {
            sprintf( pszOut, "        <words type=\"Fuzzy\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWTransMod );
            pszOut += strlen(pszOut);
            sprintf( pszOut, "        <words type=\"Replace\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWTransSource );
            pszOut += strlen(pszOut);
            sprintf( pszOut, "        <words type=\"FuzzyRepl\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulFuzzyRepl );
            pszOut += strlen(pszOut);
          }
          else
          {
            sprintf( pszOut, "        <words type=\"Fuzzy\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWTransMod +
                     pCNTIda->lCountFile.ulWTransSource + pCNTIda->lCountFile.ulFuzzyRepl );
            pszOut += strlen(pszOut);
          } /* endif */
          sprintf( pszOut, "        <words type=\"MachMatch\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulMachineMatch  );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"NoMatch\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWToBeTran  );
          pszOut += strlen(pszOut);
        }
        else if ( pCNTIda->pCnt->fOrg )
        {
          sprintf( pszOut, "        <words type=\"Untranslated\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWToBeTran );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Markup\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWMarkup  );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Total\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWTotal  );
          pszOut += strlen(pszOut);
        }
        else
        {
          ulWords = pCNTIda->lCountFile.ulWTransMod + pCNTIda->lCountFile.ulWTransCopied + pCNTIda->lCountFile.ulWTransScratch;

          sprintf( pszOut, "        <words type=\"Translated\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWTransSource );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Untranslated\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWToBeTran  );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"TargetWords\" count=\"%lu\" />\r\n", ulWords  );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Markup\" count=\"%lu\" />\r\n", pCNTIda->lCountFile.ulWMarkup  );
          pszOut += strlen(pszOut);
        } /* endif */

        strcpy( pszOut, "      </wordcount>\r\n" );
        pszOut += strlen(pszOut);
        strcpy( pszOut, "    </document>\r\n" );
        pszOut += strlen(pszOut);
      }
      else
      {
        //--- create output line for counted file which is written to file
        //--- in dependency of original/translation and document type
        if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
        {
          CntAbbrevFileName(pCNTIda->szShortenedName,40,&(pCNTIda->szLongName[0]));

          if (strlen(pCNTIda->szLongName) > 40)
          {
            sprintf(pCNTIda->szBuffer, "%-255.255s%\r\n",pCNTIda->szLongName);
            ulNumBytes = strlen( pCNTIda->szBuffer );
            usRc = UtlWriteHwnd( pCNTIda->hfFileHandle, pCNTIda->szBuffer,
                                ulNumBytes, &ulBytes, TRUE, pCNTIda->pCnt->hwndErrMsg );
            pCNTIda->szShortenedName[0] = EOS;
          } // end if

          sprintf( pCNTIda->szBuffer, "%-40.40s%19lu%19lu%19lu%19lu%19lu%19lu%19lu%19lu",
                  pCNTIda->szShortenedName,
                  pCNTIda->lCountFile.ulWTotal,
                  pCNTIda->lCountFile.ulWClass[0],
                  pCNTIda->lCountFile.ulWClass[1],
                  pCNTIda->lCountFile.ulWClass[2],
                  pCNTIda->lCountFile.ulWClass[3],
                  pCNTIda->lCountFile.ulWClass[4],
                  pCNTIda->lCountFile.ulWClass[5],
                  pCNTIda->lCountFile.ulWClass[6] );
        }
        else if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
        {
          CntAbbrevFileName(pCNTIda->szShortenedName,40,&(pCNTIda->szLongName[0]));

          if (strlen(pCNTIda->szLongName) > 40)
          {
            sprintf(pCNTIda->szBuffer, "%-255.255s%\r\n",pCNTIda->szLongName);
            ulNumBytes = strlen( pCNTIda->szBuffer );
            usRc = UtlWriteHwnd( pCNTIda->hfFileHandle, pCNTIda->szBuffer,
                                ulNumBytes, &ulBytes, TRUE, pCNTIda->pCnt->hwndErrMsg );
            pCNTIda->szShortenedName[0] = EOS;
          } // end if

          sprintf( pCNTIda->szBuffer, "%-40.40s%9lu%19lu%19lu%19lu%19lu%19lu",
                  pCNTIda->szShortenedName,
                  pCNTIda->lCountFile.ulWDuplicate,
                  pCNTIda->lCountFile.ulWFuzzy[2],
                  pCNTIda->lCountFile.ulWFuzzy[1],
                  pCNTIda->lCountFile.ulWFuzzy[0],
                  pCNTIda->lCountFile.ulWTotal,
                  pCNTIda->lCountFile.ulWTran );
        }
        else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE)
        {
          CntAbbrevFileName(pCNTIda->szShortenedName,40,&(pCNTIda->szLongName[0]));

          if (strlen(pCNTIda->szLongName) > 40)
          {
            sprintf(pCNTIda->szBuffer, "%-255.255s%\r\n",pCNTIda->szLongName);
            ulNumBytes = strlen( pCNTIda->szBuffer );
            usRc = UtlWriteHwnd( pCNTIda->hfFileHandle, pCNTIda->szBuffer,
                                ulNumBytes, &ulBytes, TRUE, pCNTIda->pCnt->hwndErrMsg );
            pCNTIda->szShortenedName[0] = EOS;
          } // end if

          sprintf( pCNTIda->szBuffer, "%-40.40s%8lu%15lu%19lu%19lu%11lu",
                  pCNTIda->szShortenedName,
                  pCNTIda->lCountFile.ulWTran,
                  pCNTIda->lCountFile.ulWUntranslated,
                  pCNTIda->lCountFile.ulWDuplicate,
                  pCNTIda->lCountFile.ulWToBeTran,
                  pCNTIda->lCountFile.ulWMarkup );
        }
        else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE)   //--- count TM matches
        {
          CntAbbrevFileName( pCNTIda->szShortenedName,12,&(pCNTIda->szLongName[0]));

          if (strlen(pCNTIda->szLongName) > 12)
          {
            // long file name
            // max length of 80 in sprintf is too short for very long names
            sprintf(pCNTIda->szBuffer, "%-255.255s%\r\n",pCNTIda->szLongName);
            //--- get number of bytes to be written from output buffer
            ulNumBytes = strlen( pCNTIda->szBuffer );
            //--- write output buffer to file
            usRc = UtlWriteHwnd( pCNTIda->hfFileHandle, pCNTIda->szBuffer,
                                ulNumBytes, &ulBytes, TRUE, pCNTIda->pCnt->hwndErrMsg );
            pCNTIda->szShortenedName[0] = EOS;
          } // end if

          if ( pCNTIda->pCnt->fReplMatch )
          {
            sprintf( pCNTIda->szBuffer, "%-12.12s%12lu%12lu%11lu%12lu%10lu%11lu%11lu%11lu%11lu",
                    pCNTIda->szShortenedName,
                    pCNTIda->lCountFile.ulWTotal,
                    pCNTIda->lCountFile.ulWTransCopied,
                    pCNTIda->lCountFile.ulWTransScratch,
                    pCNTIda->lCountFile.ulWMarkup,
                    pCNTIda->lCountFile.ulWTransMod,
                    pCNTIda->lCountFile.ulMachineMatch,
                    pCNTIda->lCountFile.ulWToBeTran,
                    pCNTIda->lCountFile.ulWTransSource,
                    pCNTIda->lCountFile.ulFuzzyRepl );
          }
          else
          {
            sprintf( pCNTIda->szBuffer, "%-12.12s%12lu%12lu%11lu%12lu%10lu%11lu%11lu",
                    pCNTIda->szShortenedName,
                    pCNTIda->lCountFile.ulWTotal,
                    pCNTIda->lCountFile.ulWTransCopied,
                    pCNTIda->lCountFile.ulWTransScratch,
                    pCNTIda->lCountFile.ulWMarkup,
                    pCNTIda->lCountFile.ulWTransMod +
                    pCNTIda->lCountFile.ulWTransSource +    // Sum -> File
                    pCNTIda->lCountFile.ulFuzzyRepl,        // Sum -> File
                    pCNTIda->lCountFile.ulMachineMatch,        // Sum -> File
                    pCNTIda->lCountFile.ulWToBeTran );
          } /* endif */
        }
        else
          if ( pCNTIda->pCnt->fOrg )         //--- original
        {
          CntAbbrevFileName(pCNTIda->szShortenedName,40,&(pCNTIda->szLongName[0]));

          if (strlen(pCNTIda->szLongName) > 40)
          {
            // long file name
            // max length of 80 in sprintf is too short for very long names
            sprintf(pCNTIda->szBuffer, "%-255.255s%\r\n",pCNTIda->szLongName);
            //--- get number of bytes to be written from output buffer
            ulNumBytes = strlen( pCNTIda->szBuffer );
            //--- write output buffer to file
            usRc = UtlWriteHwnd( pCNTIda->hfFileHandle, pCNTIda->szBuffer,
                                ulNumBytes, &ulBytes, TRUE, pCNTIda->pCnt->hwndErrMsg );
            pCNTIda->szShortenedName[0] = EOS;
          } // end if

          sprintf( pCNTIda->szBuffer, "%-40.40s%18lu%8lu%8lu",
                  pCNTIda->szShortenedName,
                  pCNTIda->lCountFile.ulWToBeTran,
                  pCNTIda->lCountFile.ulWMarkup,
                  pCNTIda->lCountFile.ulWTotal );
        }
        else                               //--- translation
        {

          ulWords = pCNTIda->lCountFile.ulWTransMod +
                    pCNTIda->lCountFile.ulWTransCopied +
                    pCNTIda->lCountFile.ulWTransScratch;

          CntAbbrevFileName(pCNTIda->szShortenedName,40,&(pCNTIda->szLongName[0]));

          if (strlen(pCNTIda->szLongName) > 40)
          {
            // long file name
            // max length of 80 in sprintf is too short for very long names
            sprintf(pCNTIda->szBuffer, "%-255.255s%\r\n",pCNTIda->szLongName);
            //--- get number of bytes to be written from output buffer
            ulNumBytes = strlen( pCNTIda->szBuffer );
            //--- write output buffer to file
            usRc = UtlWriteHwnd( pCNTIda->hfFileHandle, pCNTIda->szBuffer,
                                ulNumBytes, &ulBytes, TRUE, pCNTIda->pCnt->hwndErrMsg );
            pCNTIda->szShortenedName[0] = EOS;
          } // end if

          sprintf( pCNTIda->szBuffer, "%-40.40s%13lu%16lu%13lu%8lu",
                  pCNTIda->szShortenedName,
                  pCNTIda->lCountFile.ulWTransSource,
                  pCNTIda->lCountFile.ulWToBeTran,
                  ulWords,
                  pCNTIda->lCountFile.ulWMarkup );
        }/*endif*/
        //--- append CRLF
        strcat( pCNTIda->szBuffer, CRLF_STR );
      } /* endif */
      break;
      //------------------------------------------------------------------------------
    case SUMMARY_LINE :
      //--- create summary line
      //--- first write equal line to buffer
      if ( fXML )
      {
        PSZ pszOut = pCNTIda->szBuffer;
        strcpy( pszOut, "  </documents>\r\n" );
        pszOut += strlen(pszOut);
        strcpy( pszOut, "  <summary>\r\n" );
        pszOut += strlen(pszOut);
        strcpy( pszOut, "    <wordcount>\r\n" );
        pszOut += strlen(pszOut);

        if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
        {
          sprintf( pszOut, "        <words type=\"Total\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWTotal );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-zero\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWClass[0] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-1\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWClass[1] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-2\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWClass[2] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-3\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWClass[3] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-4\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWClass[4] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-5\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWClass[5] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Class-6up\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWClass[6] );
          pszOut += strlen(pszOut);
        }
        else if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
        {
          sprintf( pszOut, "        <words type=\"DuplicateWords\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWDuplicate );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Fuzzy9599\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWFuzzy[2] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Fuzzy7594\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWFuzzy[1] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Fuzzy0074\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWFuzzy[0] );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Total\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWTotal );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "        <words type=\"Exact\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWTran );
          pszOut += strlen(pszOut);
        }
        else  if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
        {
          sprintf( pszOut, "      <words type=\"Translated\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWTran );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"Untranslated\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWUntranslated );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"DuplicateWords\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWDuplicate );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"ToBeTranslated\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWToBeTran  );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"Markup\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWMarkup  );
          pszOut += strlen(pszOut);
        }
        else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE )
        {
          sprintf( pszOut, "      <words type=\"Total\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWTotal );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"ExactExact\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWTransCopied  );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"Exact-1\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWTransScratch);
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"Exact-2\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWMarkup  );
          pszOut += strlen(pszOut);
          if ( pCNTIda->pCnt->fReplMatch )
          {
            sprintf( pszOut, "      <words type=\"Fuzzy\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWTransMod );
            pszOut += strlen(pszOut);
            sprintf( pszOut, "      <words type=\"Replace\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWTransSource );
            pszOut += strlen(pszOut);
            sprintf( pszOut, "      <words type=\"FuzzyRepl\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulFuzzyRepl );
            pszOut += strlen(pszOut);
          }
          else
          {
            sprintf( pszOut, "      <words type=\"Fuzzy\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWTransMod +
                     pCNTIda->lCountSum.ulWTransSource + pCNTIda->lCountSum.ulFuzzyRepl );
            pszOut += strlen(pszOut);
          } /* endif */
          sprintf( pszOut, "      <words type=\"MachMatch\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulMachineMatch  );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"NoMatch\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWToBeTran  );
          pszOut += strlen(pszOut);
        }
        else if ( pCNTIda->pCnt->fOrg )
        {
          sprintf( pszOut, "      <words type=\"Untranslated\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWToBeTran );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"Markup\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWMarkup  );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"Total\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWTotal  );
          pszOut += strlen(pszOut);
        }
        else
        {
          ulWords = pCNTIda->lCountSum.ulWTransMod + pCNTIda->lCountSum.ulWTransCopied + pCNTIda->lCountSum.ulWTransScratch;

          sprintf( pszOut, "      <words type=\"Translated\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWTransSource );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"Untranslated\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWToBeTran  );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"TargetWords\" count=\"%lu\" />\r\n", ulWords  );
          pszOut += strlen(pszOut);
          sprintf( pszOut, "      <words type=\"Markup\" count=\"%lu\" />\r\n", pCNTIda->lCountSum.ulWMarkup  );
          pszOut += strlen(pszOut);
        } /* endif */

        strcpy( pszOut, "    </wordcount>\r\n" );
        pszOut += strlen(pszOut);
        strcpy( pszOut, "  </summary>\r\n" );
        pszOut += strlen(pszOut);
        strcpy( pszOut, "</tmwordcount>\r\n" );
      }
      else
      {
        if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
        {
          strcpy( pCNTIda->szBuffer, EQUAL_LINE_MEMINFODUPLICATE );
        }
        else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE)
        {
          strcpy( pCNTIda->szBuffer, EQUAL_LINE_DUPLICATE );
        }
        else if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
        {
          strcpy( pCNTIda->szBuffer, EQUAL_LINE_FUZZYMATCH );
        }
        else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE)   //--- count TM matches
        {
          if ( pCNTIda->pCnt->fReplMatch )
          {
            strcpy( pCNTIda->szBuffer, EQUAL_LINE_TMREPLMATCH );
          }
          else
          {
            strcpy( pCNTIda->szBuffer, EQUAL_LINE_TMMATCH );
          } /* endif */
        }
        else if (  pCNTIda->pCnt->fOrg )         //original
        {
          strcpy( pCNTIda->szBuffer, EQUAL_LINE_SHORT );
        }
        else
        {
          strcpy( pCNTIda->szBuffer, EQUAL_LINE_LONG );
        } /* endif */

        //--- append CRLF
        strcat( pCNTIda->szBuffer, CRLF_STR );

        //--- get text for summary line
        WinLoadString( hab, hResMod,
                      SID_CNT00_FILE_SUM_TEXT,
                      sizeof( pCNTIda->szStringBuffer ),
                      pCNTIda->szStringBuffer );

        if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
        {
          sprintf( pCNTIda->szFileLine, "%-40.40s%19lu%19lu%19lu%19lu%19lu%19lu%19lu%19lu",
                  pCNTIda->szStringBuffer,
                  pCNTIda->lCountSum.ulWTotal,
                  pCNTIda->lCountSum.ulWClass[0],
                  pCNTIda->lCountSum.ulWClass[1],
                  pCNTIda->lCountSum.ulWClass[2],
                  pCNTIda->lCountSum.ulWClass[3],
                  pCNTIda->lCountSum.ulWClass[4],
                  pCNTIda->lCountSum.ulWClass[5],
                  pCNTIda->lCountSum.ulWClass[6] );
        }
        else if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
        {
          sprintf( pCNTIda->szFileLine, "%-40.40s%9lu%19lu%19lu%19lu%19lu%19lu",
                  pCNTIda->szStringBuffer,
                  pCNTIda->lCountSum.ulWDuplicate,
                  pCNTIda->lCountSum.ulWFuzzy[2],
                  pCNTIda->lCountSum.ulWFuzzy[1],
                  pCNTIda->lCountSum.ulWFuzzy[0],
                  pCNTIda->lCountSum.ulWTotal,
                  pCNTIda->lCountSum.ulWTran );
        }
        else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE)
        {
          sprintf( pCNTIda->szFileLine, "%-40.40s%8lu%15lu%19lu%19lu%11lu",
                  pCNTIda->szStringBuffer,
                  pCNTIda->lCountSum.ulWTran,
                  pCNTIda->lCountSum.ulWUntranslated,
                  pCNTIda->lCountSum.ulWDuplicate,
                  pCNTIda->lCountSum.ulWToBeTran,
                  pCNTIda->lCountSum.ulWMarkup );
        }
        else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE)   //--- count TM matches
        {
          if ( pCNTIda->pCnt->fReplMatch )
          {
            sprintf( pCNTIda->szFileLine, "%-12.12s%12lu%12lu%11lu%12lu%10lu%11lu%11lu%11lu%11lu",
                    pCNTIda->szStringBuffer,
                    pCNTIda->lCountSum.ulWTotal,
                    pCNTIda->lCountSum.ulWTransCopied,
                    pCNTIda->lCountSum.ulWTransScratch,
                    pCNTIda->lCountSum.ulWMarkup,
                    pCNTIda->lCountSum.ulWTransMod,
                    pCNTIda->lCountSum.ulMachineMatch,
                    pCNTIda->lCountSum.ulWToBeTran,
                    pCNTIda->lCountSum.ulWTransSource,
                    pCNTIda->lCountSum.ulFuzzyRepl );
          }
          else
          {
            sprintf( pCNTIda->szFileLine, "%-12.12s%12lu%12lu%11lu%12lu%10lu%11lu%11lu",
                    pCNTIda->szStringBuffer,
                    pCNTIda->lCountSum.ulWTotal,
                    pCNTIda->lCountSum.ulWTransCopied,
                    pCNTIda->lCountSum.ulWTransScratch,
                    pCNTIda->lCountSum.ulWMarkup,
                    pCNTIda->lCountSum.ulWTransMod +
                    pCNTIda->lCountSum.ulWTransSource +
                    pCNTIda->lCountSum.ulFuzzyRepl,
                    pCNTIda->lCountSum.ulMachineMatch,
                    pCNTIda->lCountSum.ulWToBeTran );
          } /* endif */
        }
        else  if ( pCNTIda->pCnt->fOrg )   //--- count originals
        {
          sprintf( pCNTIda->szFileLine, "%-40.40s%18ld%8ld%8ld",
                  pCNTIda->szStringBuffer,
                  pCNTIda->lCountSum.ulWToBeTran,
                  pCNTIda->lCountSum.ulWMarkup,
                  pCNTIda->lCountSum.ulWTotal );
        }
        else   //--- count translation
        {
          ulWords = pCNTIda->lCountSum.ulWTransMod +
                    pCNTIda->lCountSum.ulWTransCopied +
                    pCNTIda->lCountSum.ulWTransScratch;

          sprintf( pCNTIda->szFileLine, "%-40.40s%13lu%16lu%13lu%8lu",
                  pCNTIda->szStringBuffer,
                  pCNTIda->lCountSum.ulWTransSource,
                  pCNTIda->lCountSum.ulWToBeTran,
                  ulWords,
                  pCNTIda->lCountSum.ulWMarkup );
        }/*end if*/

        //--- append created line to output buffer
        strcat( pCNTIda->szBuffer, pCNTIda->szFileLine );
      } /* endif */

      break;
  }/*endswitch*/

  //--- get number of bytes to be written from output buffer
  ulNumBytes = strlen( pCNTIda->szBuffer );

  //--- write output buffer to file
  usRc = UtlWriteHwnd( pCNTIda->hfFileHandle, pCNTIda->szBuffer,
                       ulNumBytes, &ulBytes, TRUE, pCNTIda->pCnt->hwndErrMsg );



  if ( !usRc )    //--- no error from UtlWrite
  {
    if ( ulNumBytes != ulBytes )   //--- error disk full
    {
      /**************************************************************/
      /* no file output anymore ...                                 */
      /**************************************************************/
      pCNTIda->pCnt->fFile  = FALSE;
      //--- close output file
      UtlClose( pCNTIda->hfFileHandle, FALSE );
      pCNTIda->hfFileHandle = NULLHANDLE;

      //--- display message that disk is full, ask for continue
      pszReplace = pCNTIda->pCnt->szDriveLetter;
      usRc = ERROR_DISK_FULL;
      if ( pCNTIda->pCnt->fBatch )
      {
        UtlErrorHwnd( ERROR_WODCA_DISK_FULL, MB_CANCEL, 1, &pszReplace,
                      EQF_ERROR, pCNTIda->pCnt->hwndErrMsg );
        usResponse = MBID_NO;
        pCNTIda->pCnt->pDDEWrdCnt->DDEReturn.usRc = ERROR_WODCA_DISK_FULL;
      }
      else
      {
        usResponse = UtlError( ERROR_WODCA_DISK_FULL, MB_YESNO | MB_DEFBUTTON2,
                               1, &pszReplace, EQF_QUERY );
      } /* endif */

      //--- delete eventually created file, because it maybe filled with
      //--- incomplete date what is not very helpfull
      UtlDelete( pCNTIda->pCnt->szOutputFile, 0L, FALSE );
    }/*end if*/
  }
  else   //--- error from UtlWrite
  {
    /**************************************************************/
    /* no file output anymore ...                                 */
    /**************************************************************/
    pCNTIda->pCnt->fFile  = FALSE;
    //--- close output file
    UtlClose( pCNTIda->hfFileHandle, FALSE );
    pCNTIda->hfFileHandle = NULLHANDLE;

    //--- delete eventually created file, because it maybe filled with
    //--- incomplete date what is not very helpfull
    UtlDelete( pCNTIda->pCnt->szOutputFile, 0L, FALSE );

    //--- display message error writing to file. ask for continue
    pszReplace = pCNTIda->pCnt->szOutputFile;
    if ( pCNTIda->pCnt->fBatch )
    {
      UtlErrorHwnd( ERROR_WODCA_WRITE, MB_CANCEL, 1, &pszReplace,
                    EQF_ERROR, pCNTIda->pCnt->hwndErrMsg );
      usResponse = MBID_NO;
      if ( pCNTIda->pCnt->pDDEWrdCnt != NULL )
      {
        pCNTIda->pCnt->pDDEWrdCnt->DDEReturn.usRc = ERROR_WODCA_WRITE;
      } /* endif */
    }
    else
    {
      usResponse = UtlError( ERROR_WODCA_WRITE, MB_YESNO, 1,
                             &pszReplace, EQF_QUERY );
    } /* endif */
  }/*endif*/

  /*******************************************************************/
  /* if error happened and user didn't want to go on...              */
  /*******************************************************************/
  pCNTIda->fKill = ( usResponse != MBID_YES );

  /*******************************************************************/
  /* if error happened and user wants to go on, correct return code  */
  /*******************************************************************/
  if ( usRc && usResponse == MBID_YES )
  {
    usRc = 0;
  } /* endif */

  return( usRc );

}/*end of CntWriteToFile*/

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     Create Colums
//+----------------------------------------------------------------------------+
// Function call:     static VOID CreateColums( PCNTIDA pCNTIda, HWND hwnd )
//+----------------------------------------------------------------------------+
// Description:       creates colums for output file
//+----------------------------------------------------------------------------+
// Parameters:        pCNTIda       pointer to count IDA
//                    hwnd          window handle
//+----------------------------------------------------------------------------+
// Returncode type:   VOID
//+----------------------------------------------------------------------------+
// Function flow:
//                    load text strings depending on original or target
//
//+----------------------------------------------------------------------------+
static VOID CreateColums( PCNTIDA pCNTIda, HWND hwnd)
{
  HAB hab;
  hwnd;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  if ( pCNTIda->pCnt->hwndErrMsg == HWND_FUNCIF )
  {
    hab = NULLHANDLE;
  }
  else
  {
    hab = (HAB) WinQueryAnchorBlock(hwnd);
  } /* endif */

  if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
  {
    strcat( pCNTIda->szBuffer, STR_CNT00_FILE_DUPLICATE_MEMINFO_GML1 );
    strcat( pCNTIda->szBuffer, CRLF_STR );
    strcat( pCNTIda->szBuffer, STR_CNT00_FILE_DUPLICATE_MEMINFO_GML2 );
    strcat( pCNTIda->szBuffer, CRLF_STR );

    strcpy( pCNTIda->szLine, STR_CNT00_FILE_DUPLICATE_MEMINFO_GML3 );
  }
  else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
  {
    strcpy( pCNTIda->szLine, STR_CNT00_FILE_DUPLICATE_GML );
  }
  else if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
  {
    strcpy( pCNTIda->szLine, STR_CNT00_FILE_FUZZYMATCH_GML );
  }
  else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE)   //--- count TM matches
  {
    WinLoadString( hab, hResMod,
                   ( pCNTIda->pCnt->fReplMatch ) ?
                   SID_CNT00_FILE_TMREPLMATCH_GML :
                   SID_CNT00_FILE_TMMATCH_GML,
                   sizeof( pCNTIda->szLine ),
                   pCNTIda->szLine );
  }
  else if ( pCNTIda->pCnt->fOrg )   //--- count original
  {
    //--- get text for original colums
    WinLoadString( hab, hResMod,
                   SID_CNT00_FILE_ORIGINAL_GML,
                   sizeof( pCNTIda->szLine ),
                   pCNTIda->szLine );
  }
  else   //--- count translation
  {
    //--- get text for original colums
    WinLoadString( hab, hResMod,
                   SID_CNT00_FILE_TRANSLATION_GML,
                   sizeof( pCNTIda->szLine ),
                   pCNTIda->szLine );
  }/*endif*/

  //--- append line to output buffer
  strcat( pCNTIda->szBuffer, pCNTIda->szLine );

  //--- append CRLF
  strcat( pCNTIda->szBuffer, CRLF_STR );

  //--- append minus line
  if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
  {
    strcat( pCNTIda->szBuffer, MINUS_LINE_MEMINFODUPLICATE );
  }
  else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
  {
    strcat( pCNTIda->szBuffer, MINUS_LINE_DUPLICATE );
  }
  else if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
  {
    strcat( pCNTIda->szBuffer, MINUS_LINE_FUZZYMATCH );
  }
  else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE)   //--- count TM matches
  {
    if ( pCNTIda->pCnt->fReplMatch )
    {
      strcat( pCNTIda->szBuffer, MINUS_LINE_TMREPLMATCH );
    }
    else
    {
      strcat( pCNTIda->szBuffer, MINUS_LINE_TMMATCH );
    } /* endif */
  }
  else if ( pCNTIda->pCnt->fOrg )   //--- count original
  {
    strcat( pCNTIda->szBuffer, MINUS_LINE_SHORT );
  }
  else   //--- count translation
  {
    strcat( pCNTIda->szBuffer, MINUS_LINE_LONG );
  } /* endif */

  //--- append CRLF
  strcat( pCNTIda->szBuffer, CRLF_STR );

}/*end of CreateColums*/


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     HandleTimer
//+----------------------------------------------------------------------------+
// Function call:     _
//+----------------------------------------------------------------------------+
// Description:       this function will communicate with the Thread
//                    and give the thread the tasks to do...
//+----------------------------------------------------------------------------+
// Parameters:        HWND hwnd
//+----------------------------------------------------------------------------+
// Returncode type:   VOID
//+----------------------------------------------------------------------------+
// Function flow:     _
//+----------------------------------------------------------------------------+
static VOID
HandleTimer
(
HWND hwnd,
PCNTIDA pCNTIda
)
{
  BOOL     fStartTimer = TRUE;         // if TRUE timer will be started again
  /********************************************************************/
  /* Avoid recursive processing of WM_TIMER message                   */
  /********************************************************************/

  if ( !pCNTIda->fTimerProcessing )
  {
    WinStopTimer( WinQueryAnchorBlock( hwnd ), hwnd, CNT_TIMER_ID );
    pCNTIda->fTimerProcessing = TRUE;
    /********************************************************************/
    /* if not busy                                                      */
    /*  if error pending display error                                  */
    /*  else go to next file name if available                          */
    /* else                                                             */
    /*  ignore request                                                  */
    /* end                                                              */
    /********************************************************************/
    if ( pCNTIda->fKill )
    {
      /******************************************************************/
      /* stop timer and the whole process                               */
      /******************************************************************/
      fStartTimer = FALSE;
      WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
    }
    else
    {
      switch ( pCNTIda->usStatus )
      {
        case  CNT_COUNTING:
          /************************************************************/
          /* still counting                                           */
          /************************************************************/
          break;

        case  CNT_START:
          /**************************************************************/
          /* start the counting process                                 */
          /**************************************************************/
          pCNTIda->usRc = SelectFileToCount( pCNTIda );
          switch ( pCNTIda->usRc )
          {
            case FILE_FOUND:
              pCNTIda->usStatus = CNT_COUNTING;
              DoCounting( pCNTIda );
              break;
            case FILE_NOT_FOUND:
              pCNTIda->usStatus = CNT_STOP;
              break;
            case FILE_IN_ERROR:
            case FILE_LOCKED:
              pCNTIda->usStatus = CNT_FINISHED;
              break;
          } /* endswitch */
          break;

        case  CNT_ERROR:
          /************************************************************/
          /* error happened during count -- user wants to stop        */
          /************************************************************/
          fStartTimer = FALSE;
          WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
          break;

        case  CNT_FINISHED:
          /**************************************************************/
          /* store the result                                           */
          /**************************************************************/
          if ( ! StoreCountResult( hwnd, pCNTIda ) )
          {
            /************************************************************/
            /* count the next file                                      */
            /************************************************************/
            pCNTIda->sIndexItem ++;
            pCNTIda->usRc = SelectFileToCount(pCNTIda );
            switch ( pCNTIda->usRc )
            {
              case FILE_FOUND:
                pCNTIda->usStatus = CNT_COUNTING;
                DoCounting( pCNTIda );
                break;
              case FILE_NOT_FOUND:
                pCNTIda->usStatus = CNT_STOP;
                break;
              case FILE_LOCKED:
              case FILE_IN_ERROR:
                pCNTIda->usStatus = CNT_FINISHED;
                break;
            } /* endswitch */
          } /* endif */
          break;

        case  CNT_STOP:
          /************************************************************/
          /* stop word count                                          */
          /************************************************************/
          fStartTimer = FALSE;
          StoreCountSummary( hwnd, pCNTIda );

          /**************************************************************/
          /* close output file if open                                  */
          /**************************************************************/
          if ( pCNTIda->hfFileHandle )
          {
            UtlCloseHwnd( pCNTIda->hfFileHandle, TRUE, pCNTIda->pCnt->hwndErrMsg );
            pCNTIda->hfFileHandle = NULLHANDLE;

            if ( pCNTIda->pCnt->usFormat == HTML_FORMAT )
            {
              // convert result to HTML when requested
              int rc = 0;
              CHAR szStyleSheet[MAX_EQF_PATH];

              UtlMakeEQFPath( szStyleSheet, NULC, TABLE_PATH, NULL );
              strcat( szStyleSheet, "\\CNT2HTML.XSL" );
              UtlDelete( pCNTIda->pCnt->szBuffer, 0L, FALSE );

              strcpy( pCNTIda->pCnt->szBuffer, pCNTIda->pCnt->szOutputFile );
              strcat( pCNTIda->pCnt->szBuffer, ".new" );
              rc = XSLTConversion( pCNTIda->pCnt->szOutputFile, pCNTIda->pCnt->szBuffer, szStyleSheet, TRUE, pCNTIda->pCnt->hwndErrMsg );

              if ( rc == 0 )
              {
                UtlDelete( pCNTIda->pCnt->szOutputFile, 0L, FALSE );
                UtlMove( pCNTIda->pCnt->szBuffer, pCNTIda->pCnt->szOutputFile, 0L, FALSE );
              }
              else
              {
                UtlDelete( pCNTIda->pCnt->szBuffer, 0L, FALSE );
              } /* endif */
            }
            else if ( pCNTIda->pCnt->usFormat == XML_FORMAT )
            {
              // export stylesheet to same directory as countfile
              CHAR szStyleSheet[MAX_EQF_PATH];

              UtlMakeEQFPath( szStyleSheet, NULC, TABLE_PATH, NULL );
              strcat( szStyleSheet, "\\CNT2HTML.XSL" );
              strcpy( pCNTIda->pCnt->szBuffer, pCNTIda->pCnt->szOutputFile );
              UtlSplitFnameFromPath( pCNTIda->pCnt->szBuffer );
              strcat( pCNTIda->pCnt->szBuffer, "\\CNT2HTML.XSL" );
              UtlCopy( szStyleSheet, pCNTIda->pCnt->szBuffer, 0, 0, FALSE );
            } /* endif */

          }/*endif*/

          if ( pCNTIda->pCnt->fBatch )
          {
            WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
          } /* endif */

          break;
        default :
          break;
      } /* endswitch */
    } /* endif */

    if ( fStartTimer )
    {
      pCNTIda->fTimerProcessing = FALSE;
      WinStartTimer( WinQueryAnchorBlock(hwnd), hwnd, CNT_TIMER_ID, CNT_TIMER_VAL );
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* it is a posted timer we already working on...                  */
    /******************************************************************/
  } /* endif */

} /* end of function HandleTimer */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     SelectFileToCount
//+----------------------------------------------------------------------------+
// Function call:     _
//+----------------------------------------------------------------------------+
// Description:       set the next file to be counted..
//+----------------------------------------------------------------------------+
// Parameters:        HWND  hwnd,              window handle
//                    PCNTIDA pCNTIda          pointer to count ida
//+----------------------------------------------------------------------------+
// Returncode type:   USHORT
//+----------------------------------------------------------------------------+
// Returncodes:       FILE_FOUND      another file found to be counted          /*@1204C*/
//                    FILE_NOT_FOUND  no other file found to be counted         /*@1204A*/
//                    FILE_LOCKED     file is locked (in use)                   /*@1204A*/
//+----------------------------------------------------------------------------+
// Function flow:     get text of active item out of list box
//                    prepare listbox string and select item
//                    if no further one available, set success indicator to
//                      false.
//                    return success indicator
//+----------------------------------------------------------------------------+
static USHORT
SelectFileToCount
(
  PCNTIDA pCNTIda                      // pointer to count ida
)
{
  USHORT usRc = FILE_FOUND;  //still another file to count
  ULONG  ulItemHandle;       //itemhandle, conatins flag id file is
                             //locked and should not be counted
  SHORT  sRc;

  // get listbox item text
  pCNTIda->szLine[0] = EOS;
  // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
  //  sRc = (SHORT) QUERYITEMTEXTHWND( pCNTIda->hwndLB, pCNTIda->sIndexItem,
  //                                   pCNTIda->szLine );
  sRc = (SHORT)SendMessage( pCNTIda->hwndLB, LM_EQF_QUERYITEMTEXT,
                            (WPARAM)pCNTIda->sIndexItem, (LPARAM)pCNTIda->szLine );

  if (sRc == LB_ERR )
  {
    pCNTIda->szLine[0] = EOS;
  } /* endif */

  if ( pCNTIda->szLine[0] == EOS )
  {
    pCNTIda->szLongName[0] = EOS;
    pCNTIda->szCountFileName[0] = EOS;
  }
  else
  {
    strcpy( pCNTIda->szLongName, UtlParseX15( pCNTIda->szLine, DOCNAME ) );
    strcpy( pCNTIda->szCountFileName, UtlParseX15( pCNTIda->szLine, SHORTNAME ) );
  } /* endif */

  if ( pCNTIda->szCountFileName[0] )
  {
    /*************************************************************/
    /* a file is found, check if file is locked                  */
    /*************************************************************/
    ulItemHandle = GETITEMHANDLEHWND( pCNTIda->hwndLB,
                                      pCNTIda->sIndexItem, LONG );

    if ( ulItemHandle == ITEM_LOCKED )
    {
      /**********************************************************/
      /* file is locked; do not count it; create output line:   */
      /* "File in use"; set returncode                          */
      /**********************************************************/
      usRc = FILE_LOCKED;
      sprintf( pCNTIda->szLine, "%s%s%s%s%s%s%s%s%s%s",
               pCNTIda->szLongName, X15_STR,
               pCNTIda->szCountFileName, X15_STR,
               pCNTIda->szLocked,        X15_STR,
               X15_STR, X15_STR, X15_STR, X15_STR );
    }
    else if ( !ActivateDocEnv( pCNTIda ) )
    {
      usRc = FILE_LOCKED;
      sprintf( pCNTIda->szLine, "%s%s%s%s%s%s%s%s%s%s%s",
               pCNTIda->szLongName, X15_STR,
               pCNTIda->szCountFileName, X15_STR,
               pCNTIda->szError,         X15_STR,
               X15_STR, X15_STR, X15_STR, X15_STR, X15_STR );
    }
    else
    {
      //--- fill the word: counting... into the first colum
      sprintf( pCNTIda->szLine, "%s%s%s%s%s%s%s%s%s%s%s",
               pCNTIda->szLongName, X15_STR,
               pCNTIda->szCountFileName, X15_STR,
               pCNTIda->szCounting,      X15_STR,
               X15_STR, X15_STR, X15_STR, X15_STR, X15_STR );
    } /* endif */

    /*****************************************************************/
    /* display active item marked with the word counting... or locked*/
    /* in the result listbox and select it                           */
    /*****************************************************************/
    SETITEMTEXTHWND( pCNTIda->hwndLB, pCNTIda->sIndexItem, pCNTIda->szLine );
    SELECTITEMHWND( pCNTIda->hwndLB, pCNTIda->sIndexItem );
  }
  else
  {
    /**************************************************************/
    /* no more files to count; set return code                    */
    /**************************************************************/
    usRc = FILE_NOT_FOUND;
  } /* endif */

  return usRc;
} /* end of function SelectFileToCount */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     StoreCountResult
//+----------------------------------------------------------------------------+
// Function call:     _
//+----------------------------------------------------------------------------+
// Description:       prepare the result line to be displayed and update
//                    the total box
//+----------------------------------------------------------------------------+
// Parameters:        HWND  hwnd,                   window handle
//                    PCNTIDA  pCNTIda              control block pointer
//+----------------------------------------------------------------------------+
// Returncode type:   USHORT
//+----------------------------------------------------------------------------+
// Returncodes:       DOS RC
//+----------------------------------------------------------------------------+
// Function flow:     if error happened
//                      display error line
//                      if requested write line to file
//                    else
//                      prepare normal display
//                      if requested write line to file
//                    endif
//                    return success indicator
//+----------------------------------------------------------------------------+
static USHORT
StoreCountResult
(
HWND  hwnd,                          // window handle
PCNTIDA  pCNTIda                     // control block pointer
)
{
  USHORT usRC = 0;                     // success indicator
  LONG   ulWords;                      // temp variable

  /********************************************************************/
  /* prepare the error line if necessary                              */
  /********************************************************************/
  if ( pCNTIda->usRc  )
  {
    if ( (pCNTIda->pCnt->fOrg == TMMATCHES_STATE) &&
         ( pCNTIda->usRc == FILE_NOINFO ) )
    {
      sprintf( pCNTIda->szLine, "%s%s%s%s%s%s %s%s%s%s%s%s%s%s",
               pCNTIda->szLongName, X15_STR,
               pCNTIda->szCountFileName, X15_STR,
               pCNTIda->szNoInfo,        X15_STR,
               X15_STR, X15_STR, X15_STR, X15_STR, X15_STR, X15_STR, X15_STR, X15_STR  );
      /********************************************************************/
      /* update listbox                                                   */
      /********************************************************************/
      if ( pCNTIda->pCnt->hwndErrMsg != HWND_FUNCIF )
      {
        SETITEMTEXTHWND( pCNTIda->hwndLB, pCNTIda->sIndexItem, pCNTIda->szLine );
      } /* endif */
    }
    else  if ( pCNTIda->usRc != FILE_LOCKED )
    {
      sprintf( pCNTIda->szLine, "%s%s%s%s%s%s %s%s%s%s%s%s%s%s",
               pCNTIda->szLongName, X15_STR,
               pCNTIda->szCountFileName, X15_STR,
               pCNTIda->szError,         X15_STR,
               X15_STR, X15_STR, X15_STR, X15_STR, X15_STR, X15_STR, X15_STR, X15_STR  );
      /********************************************************************/
      /* update listbox                                                   */
      /********************************************************************/
      if ( pCNTIda->pCnt->hwndErrMsg != HWND_FUNCIF )
      {
        SETITEMTEXTHWND( pCNTIda->hwndLB, pCNTIda->sIndexItem, pCNTIda->szLine );
      } /* endif */
    } /* endif */

    /********************************************************************/
    /* update file if requested                                         */
    /********************************************************************/
    if ( pCNTIda->pCnt->fFile )
    {
      if ( (pCNTIda->pCnt->fOrg == TMMATCHES_STATE) &&
           ( pCNTIda->usRc == FILE_NOINFO ) )
      {
        usRC = CntWriteToFile( pCNTIda, hwnd, NOINFO_LINE );
      }
      else if ( pCNTIda->usRc == FILE_LOCKED )
      {
        usRC = CntWriteToFile( pCNTIda, hwnd, LOCKED_LINE );
      }
      else
      {
        usRC = CntWriteToFile( pCNTIda, hwnd, ERROR_LINE );
      } /* endif */
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* every thing okay                                               */
    /*   sum results up                                               */
    /******************************************************************/
    int i;
    pCNTIda->lCountSum.ulWTran       += pCNTIda->lCountFile.ulWTran;
    pCNTIda->lCountSum.ulWDuplicate  += pCNTIda->lCountFile.ulWDuplicate;
    pCNTIda->lCountSum.ulWToBeTran   += pCNTIda->lCountFile.ulWToBeTran;
    pCNTIda->lCountSum.ulWUntranslated += pCNTIda->lCountFile.ulWUntranslated;
    pCNTIda->lCountSum.ulWTransMod   += pCNTIda->lCountFile.ulWTransMod;
    pCNTIda->lCountSum.ulWTransCopied+= pCNTIda->lCountFile.ulWTransCopied;
    pCNTIda->lCountSum.ulWTransScratch+= pCNTIda->lCountFile.ulWTransScratch;
    pCNTIda->lCountSum.ulWMarkup  += pCNTIda->lCountFile.ulWMarkup;
    pCNTIda->lCountSum.ulWTotal   += pCNTIda->lCountFile.ulWTotal;
    pCNTIda->lCountSum.ulWTransSource += pCNTIda->lCountFile.ulWTransSource;
    pCNTIda->lCountSum.ulFuzzyRepl    += pCNTIda->lCountFile.ulFuzzyRepl;
    pCNTIda->lCountSum.ulMachineMatch += pCNTIda->lCountFile.ulMachineMatch;
    for( i = 0; i < MAX_FUZZY_COLS; i++ )
    {
      pCNTIda->lCountSum.ulWFuzzy[i] += pCNTIda->lCountFile.ulWFuzzy[i];
    } /* endfor */
    for( i = 0; i < MAX_CLASS_COLS; i++ )
    {
      pCNTIda->lCountSum.ulWClass[i] += pCNTIda->lCountFile.ulWClass[i];
    } /* endfor */

    if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
    {
        sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s",
                 pCNTIda->szLongName, X15_STR,
                 pCNTIda->szCountFileName,             X15_STR,
                 pCNTIda->lCountFile.ulWTotal, X15_STR,
                 pCNTIda->lCountFile.ulWClass[0], X15_STR,
                 pCNTIda->lCountFile.ulWClass[1], X15_STR,
                 pCNTIda->lCountFile.ulWClass[2], X15_STR,
                 pCNTIda->lCountFile.ulWClass[3], X15_STR,
                 pCNTIda->lCountFile.ulWClass[4], X15_STR,
                 pCNTIda->lCountFile.ulWClass[5], X15_STR,
                 pCNTIda->lCountFile.ulWClass[6], X15_STR );
    }
    else if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
    {
        sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s",
                 pCNTIda->szLongName, X15_STR,
                 pCNTIda->szCountFileName,             X15_STR,
                 pCNTIda->lCountFile.ulWDuplicate, X15_STR,
                 pCNTIda->lCountFile.ulWFuzzy[2], X15_STR,
                 pCNTIda->lCountFile.ulWFuzzy[1], X15_STR,
                 pCNTIda->lCountFile.ulWFuzzy[0], X15_STR,
                 pCNTIda->lCountFile.ulWTotal, X15_STR,
                 pCNTIda->lCountFile.ulWTran, X15_STR );
    }
    else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
    {
        sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%lu%s%lu%s%lu%s%lu%s",
                 pCNTIda->szLongName, X15_STR,
                 pCNTIda->szCountFileName,             X15_STR,
                 pCNTIda->lCountFile.ulWTran, X15_STR,
                 pCNTIda->lCountFile.ulWUntranslated, X15_STR,
                 pCNTIda->lCountFile.ulWDuplicate, X15_STR,
                 pCNTIda->lCountFile.ulWToBeTran, X15_STR,
                 pCNTIda->lCountFile.ulWMarkup, X15_STR );
    }
    else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE )
    {
      if ( pCNTIda->pCnt->fReplMatch )
      {
        sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s",
                 pCNTIda->szLongName,                  X15_STR,
                 pCNTIda->szCountFileName,             X15_STR,
                 pCNTIda->lCountFile.ulWTotal,         X15_STR,
                 pCNTIda->lCountFile.ulWTransCopied,   X15_STR,
                 pCNTIda->lCountFile.ulWTransScratch,  X15_STR,
                 pCNTIda->lCountFile.ulWMarkup,        X15_STR,
                 pCNTIda->lCountFile.ulWTransMod,      X15_STR,
                 pCNTIda->lCountFile.ulMachineMatch,   X15_STR,
                 pCNTIda->lCountFile.ulWToBeTran,      X15_STR,
                 pCNTIda->lCountFile.ulWTransSource,   X15_STR,
                 pCNTIda->lCountFile.ulFuzzyRepl,      X15_STR );
      }
      else
      {
        sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s",
                 pCNTIda->szLongName,                  X15_STR,
                  pCNTIda->szCountFileName,             X15_STR,
                 pCNTIda->lCountFile.ulWTotal,         X15_STR,
                 pCNTIda->lCountFile.ulWTransCopied,   X15_STR,
                 pCNTIda->lCountFile.ulWTransScratch,  X15_STR,
                 pCNTIda->lCountFile.ulWMarkup,        X15_STR,
                 pCNTIda->lCountFile.ulWTransMod +
                 pCNTIda->lCountFile.ulWTransSource +
                 pCNTIda->lCountFile.ulFuzzyRepl,      X15_STR,
                 pCNTIda->lCountFile.ulMachineMatch,   X15_STR,
                 pCNTIda->lCountFile.ulWToBeTran,      X15_STR );
      } /* endif */
    }
    else if ( pCNTIda->pCnt->fOrg )
    {
      sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%s%lu%s%lu%s%s",
               pCNTIda->szLongName, X15_STR,
               pCNTIda->szCountFileName, X15_STR,
               pCNTIda->lCountFile.ulWToBeTran, X15_STR,
               X15_STR,
               pCNTIda->lCountFile.ulWTotal, X15_STR,
               pCNTIda->lCountFile.ulWMarkup, X15_STR,
               X15_STR );
    }
    else
    {
      ulWords = pCNTIda->lCountFile.ulWTransMod +
                pCNTIda->lCountFile.ulWTransCopied +
                pCNTIda->lCountFile.ulWTransScratch;

      {
        sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%lu%s%s%lu%s%lu%s",
                 pCNTIda->szLongName, X15_STR,
                 pCNTIda->szCountFileName, X15_STR,
                 pCNTIda->lCountFile.ulWToBeTran,     X15_STR,
                 pCNTIda->lCountFile.ulWTransSource,  X15_STR,
                 X15_STR,
                 pCNTIda->lCountFile.ulWMarkup,       X15_STR,
                 ulWords,                             X15_STR );
      } /* endif */
    } /* endif */

    /********************************************************************/
    /* update listbox                                                   */
    /********************************************************************/
    if ( pCNTIda->pCnt->hwndErrMsg != HWND_FUNCIF )
    {
      SETITEMTEXTHWND( pCNTIda->hwndLB, pCNTIda->sIndexItem, pCNTIda->szLine );
    } /* endif */
    /********************************************************************/
    /* update file if requested                                         */
    /********************************************************************/
    if ( pCNTIda->pCnt->fFile )
    {
      usRC = CntWriteToFile( pCNTIda, hwnd, FILE_COUNT_LINE );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* reset last error code                                            */
  /********************************************************************/
  pCNTIda->usRc  = 0;

  return usRC;
} /* end of function StoreCountResult */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     StoreCountSummary
//+----------------------------------------------------------------------------+
// Function call:     _
//+----------------------------------------------------------------------------+
// Description:       prepare and display the summary line
//+----------------------------------------------------------------------------+
// Parameters:        HWND  hwnd,                    window handle
//                    PCNTIDA  pCNTIda               control block pointer
//+----------------------------------------------------------------------------+
// Returncode type:   USHORT
//+----------------------------------------------------------------------------+
// Returncodes:       DOS RC
//+----------------------------------------------------------------------------+
// Function flow:     prepare the summary and display it
//                    if necessary write it to the file
//                    return success indicator
//+----------------------------------------------------------------------------+

static USHORT
StoreCountSummary
(
HWND  hwnd,                          // window handle
PCNTIDA  pCNTIda                     // control block pointer
)
{
  USHORT usRc = 0;                     // success indicator
  ULONG  ulWords;                      // temp variable
  SHORT  sItem;                        // position where item is inserted
  HAB hab;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  if ( pCNTIda->pCnt->hwndErrMsg == HWND_FUNCIF )
  {
    hab = NULLHANDLE;
  }
  else
  {
    hab = (HAB) WinQueryAnchorBlock(hwnd);
  } /* endif */

  /********************************************************************/
  /* add equal line to listbox                                        */
  /********************************************************************/
  sprintf( pCNTIda->szLine, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
           SUMMARY_EQUALS, X15_STR,
           SUMMARY_EQUALS, X15_STR,
           SUMMARY_EQUALS, X15_STR,
           SUMMARY_EQUALS, X15_STR,
           SUMMARY_EQUALS, X15_STR,
           SUMMARY_EQUALS, X15_STR,
           SUMMARY_EQUALS, X15_STR,
           SUMMARY_EQUALS, X15_STR,
           SUMMARY_EQUALS, X15_STR,
           SUMMARY_EQUALS, X15_STR,
           SUMMARY_EQUALS, X15_STR );

  if ( pCNTIda->pCnt->hwndErrMsg != HWND_FUNCIF )
  {
    INSERTITEMENDHWND( pCNTIda->hwndLB, pCNTIda->szLine );
  } /* endif */

  /********************************************************************/
  /* add summary line to listbox                                      */
  /********************************************************************/
  WinLoadString( hab, hResMod,
                 SID_CNT00_TOTAL, sizeof( pCNTIda->szTotalText ),
                 pCNTIda->szTotalText);
  ulWords = pCNTIda->lCountSum.ulWTransMod +
            pCNTIda->lCountSum.ulWTransCopied +
            pCNTIda->lCountSum.ulWTransScratch;

  if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
  {
        sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s",
                 pCNTIda->szTotalText, X15_STR,
                 "",             X15_STR,
                 pCNTIda->lCountSum.ulWTotal, X15_STR,
                 pCNTIda->lCountSum.ulWClass[0], X15_STR,
                 pCNTIda->lCountSum.ulWClass[1], X15_STR,
                 pCNTIda->lCountSum.ulWClass[2], X15_STR,
                 pCNTIda->lCountSum.ulWClass[3], X15_STR,
                 pCNTIda->lCountSum.ulWClass[4], X15_STR,
                 pCNTIda->lCountSum.ulWClass[5], X15_STR,
                 pCNTIda->lCountSum.ulWClass[6], X15_STR );
  }
  else if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
  {
      sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s",
             pCNTIda->szTotalText,               X15_STR,
             "",                                 X15_STR,
                pCNTIda->lCountSum.ulWDuplicate, X15_STR,
                pCNTIda->lCountSum.ulWFuzzy[2], X15_STR,
                pCNTIda->lCountSum.ulWFuzzy[1], X15_STR,
                pCNTIda->lCountSum.ulWFuzzy[0], X15_STR,
                pCNTIda->lCountSum.ulWTotal, X15_STR,
                pCNTIda->lCountSum.ulWTran, X15_STR );
  }
  else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
  {
    sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%lu%s%lu%s%lu%s%lu%s",
             pCNTIda->szTotalText,               X15_STR,
             "",                                 X15_STR,
             pCNTIda->lCountSum.ulWTran,         X15_STR,
             pCNTIda->lCountSum.ulWUntranslated, X15_STR,
             pCNTIda->lCountSum.ulWDuplicate,    X15_STR,
             pCNTIda->lCountSum.ulWToBeTran,     X15_STR,
             pCNTIda->lCountSum.ulWMarkup,       X15_STR );
  }
  else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE )
  {
    if ( pCNTIda->pCnt->fReplMatch )
    {
      sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s",
               pCNTIda->szTotalText,               X15_STR,
               "",                                 X15_STR,
               pCNTIda->lCountSum.ulWTotal,         X15_STR,
               pCNTIda->lCountSum.ulWTransCopied,   X15_STR,
               pCNTIda->lCountSum.ulWTransScratch,  X15_STR,
               pCNTIda->lCountSum.ulWMarkup,        X15_STR,
               pCNTIda->lCountSum.ulWTransMod,      X15_STR,
               pCNTIda->lCountSum.ulMachineMatch,   X15_STR,
               pCNTIda->lCountSum.ulWToBeTran,      X15_STR,
               pCNTIda->lCountSum.ulWTransSource,   X15_STR,
               pCNTIda->lCountSum.ulFuzzyRepl,      X15_STR );
    }
    else
    {
      sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s%lu%s",
               pCNTIda->szTotalText,               X15_STR,
               "",                                 X15_STR,
               pCNTIda->lCountSum.ulWTotal,         X15_STR,
               pCNTIda->lCountSum.ulWTransCopied,   X15_STR,
               pCNTIda->lCountSum.ulWTransScratch,  X15_STR,
               pCNTIda->lCountSum.ulWMarkup,        X15_STR,
               pCNTIda->lCountSum.ulWTransMod +
               pCNTIda->lCountSum.ulWTransSource +
               pCNTIda->lCountSum.ulFuzzyRepl,      X15_STR,
               pCNTIda->lCountSum.ulMachineMatch,   X15_STR,
               pCNTIda->lCountSum.ulWToBeTran,      X15_STR );
    } /* endif */
  }
  else
  {
    sprintf( pCNTIda->szLine, "%s%s%s%s%lu%s%lu%s%lu%s%lu%s%lu%s%s",
             pCNTIda->szTotalText,               X15_STR,
             "",                                 X15_STR,
             pCNTIda->lCountSum.ulWToBeTran,      X15_STR,
             pCNTIda->lCountSum.ulWTransSource,   X15_STR,
             pCNTIda->lCountSum.ulWTotal,         X15_STR,
             pCNTIda->lCountSum.ulWMarkup,        X15_STR,
             ulWords,                             X15_STR,
             X15_STR );
  } /* endif */

  if ( pCNTIda->pCnt->hwndErrMsg != HWND_FUNCIF )
  {
    sItem = INSERTITEMENDHWND( pCNTIda->hwndLB, pCNTIda->szLine );

    /********************************************************************/
    /* scroll to bottom of listbox                                      */
    /********************************************************************/
    SELECTITEMHWND( pCNTIda->hwndLB, sItem );
  } /* endif */
  /********************************************************************/
  /* add result to file if requested                                  */
  /********************************************************************/
  if ( pCNTIda->pCnt->fFile )
  {
    usRc = CntWriteToFile( pCNTIda, hwnd, SUMMARY_LINE );
  } /* endif */
  return usRc;
} /* end of function StoreCountSummary */


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     ActivateDocEnv
//+----------------------------------------------------------------------------+
// Function call:     ActivateDocEnv( pCNTIda );
//+----------------------------------------------------------------------------+
// Description:       Activate languages and set name of document format table
//                    for current document.
//+----------------------------------------------------------------------------+
// Parameters:        PCNTIDA   pCNTIda   pointer to instance data
//+----------------------------------------------------------------------------+
// Returncode type:   BOOL
//+----------------------------------------------------------------------------+
// Returncodes:       TRUE       everything okay
//                    FALSE      errors occured
//+----------------------------------------------------------------------------+
// Function flow:     get document infor using DocQueryInfo function
//                    if failed
//                      display error message and set success indicator
//                    else
//                      get name of document format and
//                        source and target languages
//                    endif
//                    return success indicator
//+----------------------------------------------------------------------------+
static
BOOL ActivateDocEnv
(
PCNTIDA  pCNTIda                     // pointer to ida ..
)
{
  BOOL fOK = TRUE;
  PSZ             pTemp;               // temp data pointer
  USHORT          usMBID;              // return code from UtlError
  SHORT           sOldTgtLang;         // old target language ID

  /********************************************************************/
  /* Free ressources allocated for a previously processed document    */
  /********************************************************************/
  sOldTgtLang = pCNTIda->pCnt->sTgtLanguage;
  if ( pCNTIda->pCnt->sTgtLanguage != -1 )
  {
    MorphFreeLanguageID( pCNTIda->pCnt->sTgtLanguage );
    pCNTIda->pCnt->sTgtLanguage = -1;
  } /* endif */
  if ( (pCNTIda->pCnt->sSrcLanguage != -1) &&
       (pCNTIda->pCnt->sSrcLanguage != sOldTgtLang) )
  {
    MorphFreeLanguageID( pCNTIda->pCnt->sSrcLanguage );
  } /* endif */
  pCNTIda->pCnt->sSrcLanguage = -1;
  if ( pCNTIda->TBSrcDoc.pDocTagTable )
  {
    TAFreeTagTable( (PLOADEDTABLE) pCNTIda->TBSrcDoc.pDocTagTable );
    pCNTIda->TBSrcDoc.pDocTagTable = NULL;
  } /* endif */

  /********************************************************************/
  /* Get languages and format for current document                    */
  /********************************************************************/
  strcpy( pCNTIda->pCnt->szFileObjName, pCNTIda->pCnt->szFolderObjName );
  strcat( pCNTIda->pCnt->szFileObjName, BACKSLASH_STR );
  strcat( pCNTIda->pCnt->szFileObjName, pCNTIda->szCountFileName );

  if ( DocQueryInfo2Hwnd( pCNTIda->pCnt->szFileObjName,// document object name
                          pCNTIda->szNewDocMem,        // translation memory
                          pCNTIda->pCnt->szFormat,     // format of document
                          pCNTIda->pCnt->szSourceLang, // document source language
                          pCNTIda->pCnt->szTargetLang, // document target language
                          pCNTIda->szLongName,
                          NULL,
                          NULL,
                          TRUE,                        // handle errors in function
                          pCNTIda->pCnt->hwndErrMsg ) != NO_ERROR )
  {

    fOK = FALSE;
  }
  else
  {
    if ( pCNTIda->szLongName[0] == EOS )
    {
      strcpy( pCNTIda->szLongName, pCNTIda->szCountFileName );
    } /* endif */
  } /* endif */

  /**************************************************************/
  /* load document tag table if not yet done ..                 */
  /**************************************************************/
  if ( fOK )
  {
    fOK = (TALoadTagTableExHwnd( pCNTIda->pCnt->szFormat,
                                 (PLOADEDTABLE *)(&pCNTIda->TBSrcDoc.pDocTagTable),
                                 FALSE,
                                 TALOADPROTTABLEFUNC | TALOADUSEREXIT,
                                 TRUE, pCNTIda->pCnt->hwndErrMsg ) == NO_ERROR );
  } /* endif */

  /**************************************************************/
  /* set the target document pointer to the same table          */
  /**************************************************************/
  if ( fOK )
  {
    pCNTIda->TBTgtDoc.pDocTagTable = pCNTIda->TBSrcDoc.pDocTagTable;
  } /* endif */


  /******************************************************************/
  /* set source and target language IDs                             */
  /******************************************************************/
  if ( fOK )
  {
    fOK = (MorphGetLanguageID( pCNTIda->pCnt->szSourceLang,
                               &pCNTIda->pCnt->sSrcLanguage ) == MORPH_OK);
    pCNTIda->pCnt->ulSrcOemCP = GetLangOEMCP( pCNTIda->pCnt->szSourceLang);
    pCNTIda->TBSrcDoc.ulOemCodePage = pCNTIda->pCnt->ulSrcOemCP;
    /****************************************************************/
    /* display error message if linguistic support for source       */
    /* could not be loaded...                                       */
    /****************************************************************/
    if ( !fOK )
    {
      pTemp = pCNTIda->pCnt->szSourceLang;
      UtlErrorHwnd( WODCA_LING_SUPPORT, MB_CANCEL, 1, &pTemp, EQF_ERROR,
                    pCNTIda->pCnt->hwndErrMsg );
    } /* endif */

  } /* endif */

  if ( fOK && !pCNTIda->pCnt->fOrg )
  {
    /******************************************************************/
    /* Check if target language linguistic support is available       */
    /* if not display warning message and tell user that he can go on */
    /* nevertheless the result is not so good.                        */
    /* Note: Display this message only once....                       */
    /******************************************************************/
    pTemp = pCNTIda->pCnt->szTargetLang;
    fOK = (MorphGetLanguageID( pCNTIda->pCnt->szTargetLang,
                               &pCNTIda->pCnt->sTgtLanguage ) == MORPH_OK);
    pCNTIda->pCnt->ulTgtOemCP = GetLangOEMCP( pCNTIda->pCnt->szTargetLang);
    pCNTIda->TBTgtDoc.ulOemCodePage = pCNTIda->pCnt->ulTgtOemCP;
    if ( !fOK )
    {
      if ( pCNTIda->fUseSourceLng )
      {
        fOK = TRUE;
      }
      else
      {
        if ( pCNTIda->pCnt->fBatch )
        {
          fOK = TRUE;                  // continue and use source language instead
        }
        else
        {
          usMBID = UtlError( ITM_LING_SUPPORT_MISSING,
                             MB_YESNO | MB_DEFBUTTON2,
                             1, &pTemp, EQF_QUERY );
          fOK = (usMBID == MBID_YES);
        } /* endif */
      } /* endif */

      if ( fOK )
      {
        /****************************************************************/
        /* if user wants to go on we use the source language            */
        /****************************************************************/
        strcpy( pCNTIda->pCnt->szTargetLang, pCNTIda->pCnt->szSourceLang );
        pCNTIda->pCnt->sTgtLanguage = pCNTIda->pCnt->sSrcLanguage ;
        pCNTIda->pCnt->ulTgtOemCP = pCNTIda->pCnt->ulSrcOemCP;
         pCNTIda->TBTgtDoc.ulOemCodePage = pCNTIda->pCnt->ulTgtOemCP;
        /**********************************************************/
        /* set flag that source language is used for all other    */
        /* documents, too...                                      */
        /**********************************************************/
        pCNTIda->fUseSourceLng = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */

  // for fuzzy match word count and duplicate count only: open document memory
  if ( fOK && ((((pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch)) || (pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE)) )
  {
    fOK = (WCountOpenDocMemory( pCNTIda, pCNTIda->szNewDocMem ) == 0 );
  } /* endif */


  return fOK;
} /* end of function ActivateDocEnv */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     UnlockCountFiles
//+----------------------------------------------------------------------------+
// Function call:     VOID UnlockCountFiles
//+----------------------------------------------------------------------------+
// Description:       unlockes file which were locked by word cound
//+----------------------------------------------------------------------------+
// Input parameter:   HWND - handle of window
//                    pCnt - pointer to count structure
//+----------------------------------------------------------------------------+
// Returncode type:   VOID
//+----------------------------------------------------------------------------+
// Side effects:      file which were locked by word count are released
//                    the symbol is removed from the object manager
//+----------------------------------------------------------------------------+
// Function flow:
//  get filename of first item from count handler listbox
//  while ( a item is available )
//  {
//    if ( file was locked by word count )
//    {
//       release the file
//    }
//    get next filename from count handler listbox
//  }
//+----------------------------------------------------------------------------+
static
VOID UnlockCountFiles
(
  PCNTIDA  pCNTIda    //word count instance data area
)
{
  PCNT  pCnt = pCNTIda->pCnt;    //pointer to count structure
  SHORT sIndex = 0;                    //index to item in count handler listbox
  SHORT sNumOfDocs;                    // number of documents in listbox
  PSZ   pFile;

  ULONG ulItemHandle; //item handle,0:Item locked by count
                      //           !0:Item locked by another process

  sNumOfDocs =  QUERYITEMCOUNTHWND( pCNTIda->hwndLB );
  while ( sIndex < sNumOfDocs )
  {
    // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
    // QUERYITEMTEXTHWND( pCNTIda->hwndLB, sIndex, pCNTIda->szLine );
    SendMessage( pCNTIda->hwndLB, LM_EQF_QUERYITEMTEXT,
                 (WPARAM)sIndex, (LPARAM)pCNTIda->szLine );

    // build object name (folder name concatenated with filename )
    pFile = UtlParseX15( pCNTIda->szLine, SHORTNAME );
    sprintf( pCnt->szFileObjName, "%s\\%s", pCnt->szFolderObjName, pFile );

    // release file if it was locked by count
    ulItemHandle = GETITEMHANDLEHWND( pCNTIda->hwndLB, sIndex, LONG );

    if ( ulItemHandle == 0L )
    {
      REMOVESYMBOL( pCnt->szFileObjName );
    } /* endif */

    sIndex ++;
  } /* endwhile */
} /* end of function UnlockCountFiles */

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     WCntListCallBack
//+----------------------------------------------------------------------------+
// Description:       Callback function for Wordcount
//+----------------------------------------------------------------------------+
// Returncode type:   MRESULT
//+----------------------------------------------------------------------------+
// Function flow:  see Generic Handler documentation
//+----------------------------------------------------------------------------+
MRESULT WCntListCallBack
(
PLISTCOMMAREA    pCommArea,
HWND             hwnd,
WINMSG           msg,
WPARAM           mp1,
LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);

  switch ( msg )
  {
    case WM_CREATE :
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /**************************************************************/
        /* mp2 contains pointer to count structure                    */
        /**************************************************************/
        BOOL       fOK = TRUE;         // initialisation is O.K. flag
        PCNTIDA    pCNTIda = NULL;
        PCNT       pCnt;

        /**************************************************************/
        /* allocate structures used...                                */
        /**************************************************************/
        if ( fOK )
        {
          pCnt = (PCNT) mp2;

          //--- allocate storage for count instance ida
          if ( pCnt->fBatch )
          {
            fOK = UtlAllocHwnd( (PVOID *)&pCNTIda, 0L, (ULONG)sizeof( CNTIDA),
                                ERROR_STORAGE, pCnt->hwndErrMsg );
          }
          else
          {
            fOK = UtlAlloc( (PVOID *) (PVOID *)&pCNTIda, 0L, (ULONG)sizeof( CNTIDA),
                            ERROR_STORAGE );
          } /* endif */
          if ( fOK )
          {
            pCNTIda->pCnt = (PCNT) mp2;
            pCNTIda->lPelsPerLine = UtlQueryULong( QL_PELSPERLINE );
            pCNTIda->pCnt->sTgtLanguage = -1;
            pCNTIda->pCnt->sSrcLanguage = -1;
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* Load column listbox title strings                          */
        /**************************************************************/
        if ( fOK  )
        {
          if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
          {
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_DOCUMENTS, pCNTIda->szColDocuments);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DOCUMENTS_COLWIDTH, &(ColTable[0].usWidth ) );

            ColTable[0].pszTitle = pCNTIda->szColDocuments;

            //--- column : Total
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[2].usWidth ) );
            ColTable[2].pszTitle = "Total";

            //--- column : class-zero
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[2].usWidth ) );
            ColTable[3].pszTitle = "Class-zero";

            //--- column : class-1
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[4].usWidth ) );
            ColTable[4].pszTitle = "Class-1";

            //--- column : class-2
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[5].usWidth ) );
            ColTable[5].pszTitle = "Class-2";

            //--- column : class-3
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[6].usWidth ) );
            ColTable[6].pszTitle = "Class-3";

            //--- column : class-4
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[7].usWidth ) );
            ColTable[7].pszTitle = "Class-4";

            //--- column : class-5
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[7].usWidth ) );
            ColTable[8].pszTitle = "Class-5";

            //--- column : class-6
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[7].usWidth ) );
            ColTable[9].pszTitle = "Class-6up";

          }
          else if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
          {
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_DOCUMENTS, pCNTIda->szColDocuments);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DOCUMENTS_COLWIDTH, &(ColTable[0].usWidth ) );

            ColTable[0].pszTitle = pCNTIda->szColDocuments;

            //--- column : Repetitions
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[2].usWidth ) );
            ColTable[2].pszTitle = "Duplicate Words (Repetitions)";

            //--- column : 95-99% fuzzy
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[2].usWidth ) );
            ColTable[3].pszTitle = "95-99% Fuzzy";

            //--- column : 75-94% fuzzy
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[4].usWidth ) );
            ColTable[4].pszTitle = "75-94% Fuzzy";

            //--- column : 0-74% fuzzy
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[5].usWidth ) );
            ColTable[5].pszTitle = "0-74% Fuzzy";

            //--- column : total
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[6].usWidth ) );
            ColTable[6].pszTitle = "Total Untranslated";

            //--- column : 100% Match
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[7].usWidth ) );
            ColTable[7].pszTitle = "Exact matches (100% Matches)";

          }
          else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
          {
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_DOCUMENTS, pCNTIda->szColDocuments);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DOCUMENTS_COLWIDTH,
                          &(ColTable[0].usWidth ) );
            ColTable[0].pszTitle = pCNTIda->szColDocuments;

            //--- column : Translated
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[2].usWidth ) );
            ColTable[2].pszTitle = "Translated";

            //--- column : Untranslated
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[2].usWidth ) );
            ColTable[3].pszTitle = "Untranslated";

            //--- column : Duplicate Words
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[4].usWidth ) );
            ColTable[4].pszTitle = "Duplicate Words";

            //--- column : To be translated
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[5].usWidth ) );
            ColTable[5].pszTitle = "To be translated";

            //--- column : Markup
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DUPLICATE_COLWIDTH, &(ColTable[6].usWidth ) );
            ColTable[6].pszTitle = "Markup";

          }
          else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE )
          {
            //--- load strings for column headers into column listbox structure
            //--- column : Documents
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_DOCUMENTS, pCNTIda->szColDocuments);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DOCUMENTS_COLWIDTH,
                          &(ColTable[0].usWidth ) );
            ColTable[0].pszTitle = pCNTIda->szColDocuments;

            //--- column : total
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_TOTAL, pCNTIda->szColToBe);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_TMMATCH_COLWIDTH,
                          &(ColTable[2].usWidth ) );
            ColTable[2].pszTitle = pCNTIda->szColToBe;

            //--- column : exact exact
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_EXACTEXACT, pCNTIda->szColTran);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_TMMATCH_COLWIDTH,
                          &(ColTable[3].usWidth ) );
            ColTable[3].pszTitle = pCNTIda->szColTran;

            //--- column : exact
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_EXACTONE, pCNTIda->szColTotal);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_TMMATCH_COLWIDTH,
                          &(ColTable[4].usWidth ) );
            ColTable[4].pszTitle = pCNTIda->szColTotal;

            //--- column : exact more
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_EXACTMORE, pCNTIda->szColMarkup);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_TMMATCH_COLWIDTH,
                          &(ColTable[5].usWidth ) );
            ColTable[5].pszTitle = pCNTIda->szColMarkup;

            //--- column : fuzzy
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_FUZZY, pCNTIda->szColNL);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_TMMATCH_COLWIDTH,
                          &(ColTable[6].usWidth ) );
            ColTable[6].pszTitle = pCNTIda->szColNL;

            //--- column : machine match
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_MACHMATCH, pCNTIda->szColMachMatch);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_TMMATCH_COLWIDTH,
                          &(ColTable[7].usWidth ) );
            ColTable[7].pszTitle = pCNTIda->szColMachMatch;

            //--- column : no match
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_NOMATCH, pCNTIda->szColNoMatch );
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_TMMATCH_COLWIDTH,
                          &(ColTable[8].usWidth ) );
            ColTable[8].pszTitle = pCNTIda->szColNoMatch;

            if ( pCNTIda->pCnt->fReplMatch )
            {
              //--- column : replace match
              LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_REPL, pCNTIda->szColReplMatch );
              UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_TMMATCH_COLWIDTH,
                            &(ColTable[9].usWidth ) );
              ColTable[9].pszTitle = pCNTIda->szColReplMatch;

              //--- column : fuzzy replace match
              LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_FUZZYREPL,
                          pCNTIda->szColFuzzyReplMatch );
              UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_TMMATCH_COLWIDTH,
                            &(ColTable[10].usWidth ) );
              ColTable[10].pszTitle = pCNTIda->szColFuzzyReplMatch;
            } /* endif */
          }
          else
          {
            //--- load strings for column headers into column listbox structure
            //--- column : Documents
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_DOCUMENTS, pCNTIda->szColDocuments);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_DOCUMENTS_COLWIDTH,
                          &(ColTable[0].usWidth ) );

            ColTable[0].pszTitle = pCNTIda->szColDocuments;

            //--- column : untranslated words
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_TOBE_TRANSLATED, pCNTIda->szColToBe);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_TOBE_TRANSLATED_COLWIDTH,
                          &(ColTable[2].usWidth ) );
            ColTable[2].pszTitle = pCNTIda->szColToBe;

            //--- column : translated words
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_TRANSLATED, pCNTIda->szColTran);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_TRANSLATED_COLWIDTH,
                          &(ColTable[3].usWidth ) );
            ColTable[3].pszTitle = pCNTIda->szColTran;

            //--- column : total
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_TOTAL, pCNTIda->szColTotal);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_TOTAL_COLWIDTH,
                          &(ColTable[4].usWidth ) );
            ColTable[4].pszTitle = pCNTIda->szColTotal;

            //--- column : Markup
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_MARKUP, pCNTIda->szColMarkup);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_MARKUP_COLWIDTH,
                          &(ColTable[5].usWidth ) );
            ColTable[5].pszTitle = pCNTIda->szColMarkup;

            //--- column : NL words
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_NL, pCNTIda->szColNL);
            UtlLoadWidth( NULLHANDLE, hResMod, SID_CNT00_NL_COLWIDTH,
                          &(ColTable[6].usWidth ) );
            ColTable[6].pszTitle = pCNTIda->szColNL;
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* Set column listbox view lists                              */
        /* depending on selected option - count source or translation */
        /**************************************************************/
        if ( fOK  )
        {
          PSHORT psViewList;

          if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
          {
              psViewList = sFuzzyMatchGMLView;
          }
          else if ( (pCNTIda->pCnt->fOrg == DUPLICATE_STATE) && pCNTIda->pCnt->fInclMemMatch )
          {
              psViewList = sDuplicateMemInfoGMLView;
          }
          else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
          {
              psViewList = sDuplicateGMLView;
          }
          else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE )
          {
            if ( pCNTIda->pCnt->fReplMatch )
            {
              psViewList = sTMReplMatchGMLView;
            }
            else
            {
              psViewList = sTMMatchGMLView;
            } /* endif */
          }
          else if ( pCNTIda->pCnt->fOrg )
          {
            psViewList = sOrgGMLView;
          }
          else
          {
            psViewList = sTranGMLView;
          } /* endif */
          CntCLBData.psDefaultViewList  =
          CntCLBData.psDetailsViewList  =
          CntCLBData.psNameViewList     =
          CntCLBData.psSortList         =
          CntCLBData.psLastUsedViewList = psViewList;
        } /* endif */

        /****************************************************************/
        /* supply all information required to create a lng update list  */
        /****************************************************************/
        if ( fOK )
        {
          pCommArea->pUserIDA = pCNTIda;
          pCommArea->sListObjClass  = clsWORDCOUNT;
          if ( pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE )
          {
             strcpy( pCommArea->szTitle, "Word Count Results - Fuzzy Matches Per Class" );
          }
          else if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
          {
             strcpy( pCommArea->szTitle, "Word Count Results - Duplicate Words" );
          }
          else if ( pCNTIda->pCnt->fOrg == TMMATCHES_STATE )
          {
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_TITLE_TMMATCHES,
                        pCommArea->szTitle );
          }
          else if ( pCNTIda->pCnt->fOrg )   //--- count original
          {
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_TITLE_ORIGINAL,
                        pCommArea->szTitle );
          }
          else
          {
            LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_TITLE_TRANSLATION,
                        pCommArea->szTitle );
          }/*endif*/
          pCommArea->hIcon          = (HPOINTER) UtlQueryULong(QL_COUNTICON); //hiconCOUNT;
          pCommArea->fNoClose       = FALSE;
          pCommArea->sObjNameIndex  = DOCNAME;
          pCommArea->sNameIndex     = DOCNAME;
          pCommArea->sListWindowID  = ID_WORDCNT_WINDOW;
          pCommArea->sListboxID     = DID_CNT00_COUNT_LB;
          pCommArea->sPopupMenuID   = -1; // ID_CNT_POPUP;
          pCommArea->sGreyedPopupMenuID   = -1; // ID_CNT_POPUP;
          pCommArea->sNoSelPopupMenuID = -1; // ID_CNT_POPUP;
          pCommArea->pColData       = &CntCLBData;
          pCommArea->fMultipleSel   = FALSE;
          pCommArea->sDefaultAction = 0;

          pCommArea->swpSizePos.x = ((SHORT)WinQuerySysValue( HWND_DESKTOP,SV_CXSCREEN)/80) * 3 ;
          pCommArea->swpSizePos.y = 100;
          pCommArea->swpSizePos.cx = ((SHORT)WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN)/80) * 77;
          pCommArea->swpSizePos.cy = 200;
//        memcpy( &(pCommArea->swpSizePos), &(pProp->Swp), sizeof(EQF_SWP) );
          pCommArea->sItemClass     = clsWORDCOUNT;
          pCommArea->sItemPropClass = 0;
          pCommArea->asMsgsWanted[0] = WM_TIMER;
          pCommArea->asMsgsWanted[1] = 0;        // ends list of messages
        } /* endif */

        /**************************************************************/
        /* In case of errors set error return code                    */
        /**************************************************************/
        if ( !fOK )
        {
          mResult = MRFROMSHORT(DO_NOT_CREATE);
        } /* endif */
      }
      break;

    case WM_CLOSE:
      {
        PCNTIDA pCNTIda = (PCNTIDA) pCommArea->pUserIDA;
        if ( pCNTIda )
        {
          if ( pCNTIda->usStatus == CNT_COUNTING )
          {
            SHORT si;

            mResult = MRFROMSHORT( TRUE );   // = do not close right now
            pCNTIda->fKill = TRUE;
            si = 0;
            while ( (si++ < 200) && (pCNTIda->fThreadActive
                                     || pCNTIda->fHelpInitActive) )
            {
// for OS/2 only..
            } /* endwhile */
          }
          else
          {
            mResult = MRFROMSHORT( FALSE );   // = close right now
          } /* endif */
        }
        else
        {
          mResult = MRFROMSHORT( FALSE );  // = continue with close
        } /* endif */
      }
      break;

    case WM_DESTROY:
      /****************************************************************/
      /* Free all resource allocated by list instance callback        */
      /* function                                                     */
      /****************************************************************/
      {
        PCNTIDA pCNTIda = (PCNTIDA) pCommArea->pUserIDA;
        if (pCNTIda )
        {
          UnlockCountFiles( pCNTIda );

          // remove temporary memory when countig duplicates
          if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
          {
            WCountDeleteTempMemory( pCNTIda );
          } /* endif */

          // for fuzzy match count and duplicate count only: close folder mem(s)
          if ( (pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE) || ((pCNTIda->pCnt->fOrg == DUPLICATE_STATE) ) )
          {
            WCountCloseFolderMemory( pCNTIda );
          } /* endif */

          //close output file if open
          if ( pCNTIda->hfFileHandle )
          {
            UtlClose( pCNTIda->hfFileHandle, FALSE );
            pCNTIda->hfFileHandle = NULLHANDLE;
          }/*endif*/

          // free memory lookup area
          WCountFreeMemLookupData( pCNTIda );


          if ( pCNTIda->hwndDocLB != NULLHANDLE )
          {
            WinDestroyWindow( pCNTIda->hwndDocLB );
          } /* endif */
          /**************************************************************/
          /* Tell DDE handler that we are thru ...                      */
          /**************************************************************/
          if ( pCNTIda->pCnt->fBatch )
          {
            WinPostMsg( pCNTIda->pCnt->pDDEWrdCnt->hwndOwner,
                        WM_EQF_DDE_ANSWER, NULL,
                        &pCNTIda->pCnt->pDDEWrdCnt->DDEReturn );
          } /* endif */


          //--- free count instance ida
          if ( pCNTIda->TBSrcDoc.pQFTagTable )
          {
            TAFreeTagTable( (PLOADEDTABLE) pCNTIda->TBSrcDoc.pQFTagTable );
          } /* endif */
          if ( pCNTIda->TBSrcDoc.pDocTagTable )
          {
            TAFreeTagTable( (PLOADEDTABLE) pCNTIda->TBSrcDoc.pDocTagTable );
          } /* endif */
          UtlAlloc( (PVOID *) &pCNTIda->pStack, 0L, 0L, NOMSG );
          UtlAlloc( (PVOID *) &pCNTIda->pCnt, 0L, 0L, NOMSG );
          UtlAlloc( (PVOID *) &pCNTIda, 0L, 0L, NOMSG) ;
        } /* endif */
      }
      break;

    case WM_EQF_INITIALIZE:
      /****************************************************************/
      /* Fill column listbox                                          */
      /****************************************************************/
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        PCNTIDA  pCNTIda = (PCNTIDA) pCommArea->pUserIDA;
        BOOL     fOK = TRUE;

        /**************************************************************/
        /* add anchor ...                                             */
        /**************************************************************/
        pCNTIda->hwndLB = pCommArea->hwndLB;
        //--- set error message window handle when not in batch mode
        if ( !pCNTIda->pCnt->fBatch )
        {
          pCNTIda->pCnt->hwndErrMsg = hwnd;    // set error message window handle
        } /* endif */

        //--- initialize output file close flag
        pCNTIda->fClose = FALSE;

        //--- initialize index of item (document)in count listbox
        pCNTIda->sIndexItem = 0;

        //--- initialize the counting string with Counting...
        LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_COUNTING_TEXT,
                    pCNTIda->szCounting);

        //--- initialize the error string with Error...
        LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_ERROR_TEXT, pCNTIda->szError);

        //--- initialize the locked string with File in use
        LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_LOCKED_TEXT, pCNTIda->szLocked);

        //--- initialize the noinfo string with no match info
        LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_NOINFO_TEXT, pCNTIda->szNoInfo);

        if ( pCNTIda->pCnt->fFile ) //--- output checkbox is selected
        {
          //--- open output file
          //--- comment: if file exists and should not be deleted, the
          //---          word count dialog allows not to create the
          //---          instance
          USHORT usRc;
          USHORT usAction;

          usRc = UtlOpenHwnd
                 ( pCNTIda->pCnt->szOutputFile,
                   &pCNTIda->hfFileHandle,  //file handle
                   &usAction,                    //action taken by Open
                   0L,                           //file size
                   FILE_NORMAL,                  //attribute  read/write
                   OPEN_ACTION_FAIL_IF_EXISTS |  //fail if file exists
                   OPEN_ACTION_CREATE_IF_NEW,    //create file if not
                   OPEN_ACCESS_READWRITE |       //open for read+write
                   OPEN_SHARE_DENYWRITE,         //deny any other access
                   0L,                           //reserved, must be 0
                   TRUE,                         //do error handling
                   pCNTIda->pCnt->hwndErrMsg );
          if ( usRc )   //error from UtlOpen
          {
            //--- stop further processing
            fOK = FALSE;
          }
          else
          {
            //--- create heading in output file, in case of error
            //--- CntWriteToFile closes the output file
            fOK  = !CntWriteToFile( pCNTIda, hwnd, HEADER );
          }/*endif*/
        }/*endif*/

        /*********************************************************/
        /* start the Thread under OS/2                           */
        /*********************************************************/
        if ( fOK )
        {
          fOK = CNTInit( pCNTIda );
        } /* endif */

        if ( fOK )
        {
          HWND hwndDocLB;             // listbox for document names

          // create tempory listbox for document names
          hwndDocLB = WinCreateWindow( hwnd, WC_LISTBOX, "",
                                       WS_CHILDWINDOW | LBS_STANDARD,
                                       0, 0, 10, 10, hwnd,
                                       HWND_TOP, 123, NULL, NULL);

          //--- initialize summary variables
          memset( &pCNTIda->lCountSum, 0, sizeof( pCNTIda->lCountSum ));

          //--- insert selected documents in listbox
          if ( pCNTIda->pCnt->fBatch )
          {
            pCNTIda->sCountNum = UtlCopyListBox( hwndDocLB,
                                                 pCNTIda->pCnt->hwndCountHandlerLB );

          }
          else
          {
            EqfSend2Handler ( FOLDERHANDLER, pCNTIda->pCnt->usFillMode,
                              MP1FROMHWND(hwndDocLB),
                              (LPARAM) pCNTIda->pCnt->szParentObjName );
            pCNTIda->sCountNum = QUERYITEMCOUNTHWND( pCommArea->hwndLB );
          } /* endif */

          // add document long and short names to our document listbox
          {
            SHORT sIndex;                  // listbox item buffer
            SHORT sNumOfDocs;              // number of documents in listbox
            HPROP         hPropDocument;   // handle to document properties
            PPROPDOCUMENT pPropDocument;   // pointer to document properties
            ULONG         ulErrorInfo;     // error indicator from PRHA

            sIndex = 0;
            sNumOfDocs =  QUERYITEMCOUNTHWND( hwndDocLB );
            while ( sIndex < sNumOfDocs )
            {
              CHAR  szDocument[MAX_FILESPEC];

              QUERYITEMTEXTHWND( hwndDocLB, sIndex, szDocument );
              hPropDocument = OpenProperties( szDocument,
                                              pCNTIda->pCnt->szFolderObjName,
                                              PROP_ACCESS_READ, &ulErrorInfo );
              if ( hPropDocument )
              {
                pPropDocument = (PPROPDOCUMENT) MakePropPtrFromHnd( hPropDocument );

                if ( pPropDocument->szLongName[0] != EOS )
                {
                  strcpy( pCNTIda->szLongName, pPropDocument->szLongName );
                  OEMTOANSI( pCNTIda->szLongName );
                }
                else
                {
                  strcpy( pCNTIda->szLongName, szDocument );
                } /* endif */

                sprintf( pCNTIda->szLine, "%s%c%s%c\x15\x15\x15\x15\x15\x15\x15\x15\x15", pCNTIda->szLongName,
                         X15, szDocument, X15 );
                INSERTITEMHWND( pCommArea->hwndLB, pCNTIda->szLine );


                CloseProperties( hPropDocument, PROP_QUIT, &ulErrorInfo );
              } /* endif */
              sIndex ++;
            } /* endwhile */
          }

          //--- check if files are locked and lock files
          fOK = LockCountFiles( pCNTIda );
          if ( fOK )
          {
            pCNTIda->usStatus = CNT_START;
            WinStartTimer( WinQueryAnchorBlock(hwnd), hwnd,
                           CNT_TIMER_ID, CNT_TIMER_VAL );
          } /* endif */
        }/*endif*/

        if ( !fOK )
        {
          //--- destroy window
          WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
        } /* endif */

      }
      break;

    case WM_TIMER:
      HandleTimer( hwnd, (PCNTIDA) pCommArea->pUserIDA );
      break;

    case WM_EQF_BUILDITEMTEXT :
      /****************************************************************/
      /* Setup item text for the object passed in mp2 parameter       */
      /****************************************************************/
      strcpy( pCommArea->szBuffer, (char *) PVOIDFROMMP2(mp2));
      mResult = MRFROMSHORT( TRUE );
      break;

    case WM_EQF_INITMENU:
    case WM_INITMENU:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

    case WM_EQF_TOOLBAR_ENABLED:
      /****************************************************************/
      /* disable all                                                  */
      /****************************************************************/
      break;

    case WM_EQF_COMMAND:
    case WM_COMMAND:
      mResult = MRFROMSHORT( TRUE ); // default return code for COMMAND msgs
      switch ( SHORT1FROMMP1(mp1) )
      {
        case PID_SYS_SIZE:
          SendMessage( hwnd, WM_SYSCOMMAND, SC_SIZE, 0L );
          break;
        case PID_SYS_MOVE:
          SendMessage( hwnd, WM_SYSCOMMAND, SC_MOVE, 0L );
          break;
        case PID_SYS_CLOSE:
          SendMessage( hwnd, WM_SYSCOMMAND, SC_CLOSE, 0L );
          break;
        default:
          if ( msg == WM_EQF_COMMAND)
          {
            mResult = MRFROMSHORT( FALSE ); // tell twbmain that we rejected
          } /* endif */
      }
      break;

    default:
      break;
  } /* endswitch */
  return( mResult );
} /* end of function WCntListCallBack */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     LockCountFiles
//+----------------------------------------------------------------------------+
// Function call:     BOOL LockCountFiles( hwndDialog, pCnt );
//+----------------------------------------------------------------------------+
// Description:       Ckeck if count files are locked and lock count files
//+----------------------------------------------------------------------------+
// Input parameter:   hwndDialog - count dialog handle
//                    pCnt       - pointer to count structure
//+----------------------------------------------------------------------------+
// Returncode type:   BOOL
//+----------------------------------------------------------------------------+
// Returncodes:    TRUE  - no file is locked or a locked file should be skipped
//                 FALSE - a file is locked and should not skiped
//+----------------------------------------------------------------------------+
// Side effects:      sets the itemhandle to locked in the count listbox
//                    this means this file is locked and should not be counted
//+----------------------------------------------------------------------------+
// Function flow:
//   fOk = TRUE
//   while a item is available
//   {
//     if document is locked
//       display message an to check if document should be skiped
//       if document should be skiped
//         delete item from dialog list box
//         set item as locked in count handler listbox
//       else
//         set return code to FALSE
//     else
//       lock document
//       set item as unlocked in count handler listbox
//     get next document to be processed
//   }
//   return fOk
//+----------------------------------------------------------------------------+
static BOOL LockCountFiles
(
  PCNTIDA  pCNTIda    //word count instance data area
)
{
  PCNT   pCnt = pCNTIda->pCnt;        //pointer to count structure
  PSZ    pszReplace = pCNTIda->szLongName; //replace string for UtlError
  USHORT usResponse;                     //return from UtlError
  SHORT  sIndex = 0;                     //index of count handler LB
  SHORT  sRc;                            //rc from WM_EQF_QUERYSYMBOL
  BOOL   fOk = TRUE;                     //function return code
  SHORT sNumOfDocs;                   // number of documents in listbox

  sNumOfDocs =  QUERYITEMCOUNTHWND( pCNTIda->hwndLB );
  while ( fOk && (sIndex < sNumOfDocs) )
  {
    // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
    // QUERYITEMTEXTHWND( pCNTIda->hwndLB, sIndex, pCNTIda->szLine );
    SendMessage( pCNTIda->hwndLB, LM_EQF_QUERYITEMTEXT,
                 (WPARAM)sIndex, (LPARAM)pCNTIda->szLine );

    if ( pCNTIda->szLine != EOS )
    {
      // extract long and short file name
      strcpy( pCNTIda->szLongName, UtlParseX15( pCNTIda->szLine, DOCNAME ) );
      strcpy( pCNTIda->szCountFileName, UtlParseX15( pCNTIda->szLine, SHORTNAME ) );

      // build object name (folder name concatenated with filename )
      sprintf( pCnt->szFileObjName, "%s\\%s", pCnt->szFolderObjName,
               pCNTIda->szCountFileName );


      // check if symbol for this file already exists
      // i.e. file ise in use
      // if in batch mode all counted files are already locked, so
      // do not check and lock the file here
      if ( pCnt->fBatch )
      {
        // flag file as unlocked in count handler listbox
        SETITEMHANDLEHWND( pCNTIda->hwndLB, sIndex, NULL );
      }
      else
      {
        sRc = QUERYSYMBOL( pCnt->szFileObjName );
        if ( sRc != -1 )
        {
          // file is in use so display error message
          usResponse = UtlError( ERROR_WODCA_DOC_IN_USE, MB_YESNO,
                                 1, &pszReplace, EQF_QUERY );
          if ( usResponse == MBID_YES )
          {
            // flag file as locked in count handler listbox
            SETITEMHANDLEHWND( pCNTIda->hwndLB, sIndex, ITEM_LOCKED );
          }
          else
          {
            // file should not be skiped, count should not continue
            fOk = FALSE;
          } /* endif */
        }
        else
        {
          // file is not in use, lock it! now it is in use by count
          SETSYMBOL( pCnt->szFileObjName );

          // flag file as unlocked in count handler listbox
          SETITEMHANDLEHWND( pCNTIda->hwndLB, sIndex, NULL );
        } /* endif */
      } /* endif */
    } /* endif */

    sIndex ++;
  } /* endwhile */

  return fOk;
} /* end of LockCountFiles */


USHORT CntFuncCountProcess
(
PFCTDATA    pData                    // ptr to function call interface data
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PCNTIDA    pCNTIda = (PCNTIDA)pData->pvWordCountData;
  BOOL       fOK = TRUE;               // internal O.K. flag

  switch ( pData->usWordCountPhase )
  {
    case FCTPHASE_INIT :
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        pCNTIda->fClose = FALSE;
        pCNTIda->sIndexItem = 0;
        pCNTIda->pCnt->pszCurDoc = pCNTIda->pCnt->pDocNameBuffer;
        LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_COUNTING_TEXT, pCNTIda->szCounting);
        LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_ERROR_TEXT, pCNTIda->szError);
        LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_LOCKED_TEXT, pCNTIda->szLocked);
        LOADSTRING( NULLHANDLE, hResMod, SID_CNT00_NOINFO_TEXT, pCNTIda->szNoInfo);

        // lock folder
        if ( fOK )
        {
          if ( QUERYSYMBOL( pCNTIda->pCnt->szFolderObjName ) != -1 )
          {
            PSZ pszParm;
            fOK = FALSE;
            pszParm = pCNTIda->pCnt->szDisplayName;
            usRC = ERROR_FOLDER_LOCKED;
            UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, &pszParm,
                          EQF_ERROR, HWND_FUNCIF );
          }
          else
          {
            SETSYMBOL( pCNTIda->pCnt->szFolderObjName );
            pCNTIda->fFolderLocked = TRUE;
          } /* endif */
        } /* endif */

        if ( fOK )
        {
          if ( pCNTIda->pCnt->fFile ) //--- output checkbox is selected
          {
            USHORT usRc;
            USHORT usAction;

            usRc = UtlOpenHwnd( pCNTIda->pCnt->szOutputFile,
                                &pCNTIda->hfFileHandle,
                                &usAction, 0L, FILE_NORMAL,
                                OPEN_ACTION_FAIL_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW,
                                OPEN_ACCESS_READWRITE |  OPEN_SHARE_DENYWRITE,
                                0L, TRUE, pCNTIda->pCnt->hwndErrMsg );
            if ( usRC )
            {
              fOK = FALSE;
            }
            else
            {
              fOK  = !CntWriteToFile( pCNTIda, HWND_FUNCIF, HEADER );
            }/*endif*/
          }/*endif*/
        } /* endif */

        if ( fOK )
        {
          fOK = CNTInit( pCNTIda );
        } /* endif */

        if ( fOK )
        {
          memset( &pCNTIda->lCountSum, 0, sizeof( pCNTIda->lCountSum ));
          pCNTIda->sCountNum = pCNTIda->pCnt->usNoOfDocs;
        }/*endif*/

        if ( fOK )
        {
          pData->usWordCountPhase = FCTPHASE_NEXT;
        }
        else
        {
          pData->usWordCountPhase = FCTPHASE_ABORT;
        } /* endif */
      }
      break;

    case FCTPHASE_NEXT :
      if ( pCNTIda->sIndexItem < pCNTIda->sCountNum )
      {
        pCNTIda->sIndexItem += 1;
        strcpy( pCNTIda->szCountFileName, pCNTIda->pCnt->pszCurDoc );
        pCNTIda->pCnt->pszCurDoc += strlen(pCNTIda->pCnt->pszCurDoc) + 1;
        if ( !ActivateDocEnv( pCNTIda ) )
        {
          pData->usWordCountPhase = FCTPHASE_ABORT;
        }
        else
        {
          pData->usWordCountPhase = FCTPHASE_PROCESS;
        } /* endif */
      }
      else
      {
        StoreCountSummary( HWND_FUNCIF, pCNTIda );
        pData->usWordCountPhase = FCTPHASE_CLEANUP;
      } /* endif */
      break;

    case FCTPHASE_PROCESS :
      {
        SHORT sRC;

        sRC = DoCounting( pCNTIda );
        if ( sRC == FILE_NOINFO ) sRC = NO_ERROR;

        if ( sRC )
        {
          pData->usWordCountPhase = FCTPHASE_ABORT;
        }
        else if (!StoreCountResult( HWND_FUNCIF, pCNTIda ) )
        {
          pData->usWordCountPhase = FCTPHASE_NEXT;
        }
        else
        {
          pData->usWordCountPhase = FCTPHASE_ABORT;
        } /* endif */
      }
      break;

    case FCTPHASE_ABORT :
      pData->usWordCountPhase = FCTPHASE_CLEANUP;
      usRC = UtlQueryUShort( QS_LASTERRORMSGID );
      // break intentionally removed!

    case FCTPHASE_CLEANUP:
      // cleanup

      // remove temporary memory when countig duplicates
      if ( pCNTIda->pCnt->fOrg == DUPLICATE_STATE )
      {
        WCountDeleteTempMemory( pCNTIda );
      } /* endif */

      // for fuzzy match count and duplicate count : close folder mem(s)
      if ( fOK && (((pCNTIda->pCnt->fOrg == DUPLICATE_STATE) ) || (pCNTIda->pCnt->fOrg == FUZZYMATCH_STATE) ) )
      {
        fOK = (WCountCloseFolderMemory( pCNTIda ) == 0);
      } /* endif */

      // free memory lookup area
      WCountFreeMemLookupData( pCNTIda );

      if ( pCNTIda->hfFileHandle )
      {
        UtlClose( pCNTIda->hfFileHandle, FALSE );
        pCNTIda->hfFileHandle = NULLHANDLE;
      }/*endif*/
      if ( pCNTIda->TBSrcDoc.pQFTagTable )
      {
        TAFreeTagTable( (PLOADEDTABLE) pCNTIda->TBSrcDoc.pQFTagTable );
      } /* endif */
      if ( pCNTIda->TBSrcDoc.pDocTagTable )
      {
        TAFreeTagTable( (PLOADEDTABLE) pCNTIda->TBSrcDoc.pDocTagTable );
      } /* endif */
      if ( pCNTIda->pCnt != NULL )
      {
        if ( pCNTIda->fFolderLocked ) REMOVESYMBOL( pCNTIda->pCnt->szFolderObjName   );
        UtlAlloc( (PVOID *)&pCNTIda->pCnt->pDocNameBuffer, 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *)&pCNTIda->pCnt, 0L, 0L, NOMSG );
      } /* endif */
      UtlAlloc( (PVOID *) &pCNTIda, 0L, 0L, NOMSG) ;
      pData->usWordCountPhase = FCTPHASE_DONE;
      pData->fComplete = TRUE;
      break;
  } /* endswitch */
  return( usRC );
} /* end of function CntFuncCountProcess */

//
//  function WCountGetMatchInfo
//
//  Returns the fuziness of the best match for this segment
//
static USHORT WCountGetMatchInfo
(
  PCNTIDA     pCNTIda,                 // control block
  PTBSEGMENT  pSeg,                    // segment being looked up
  OtmMemory   *pMem                    // memory object
)
{
  USHORT      usMatch = 0;
  PCNTMEMLOOKUPDATA pMemData = (PCNTMEMLOOKUPDATA)pCNTIda->pvMemLookupData;

  ULONG ulOptions = GET_EXACT;

  OtmProposal SearchKey;

  SearchKey.setDocShortName( pCNTIda->szCountFileName );
  SearchKey.setDocName( pCNTIda->szLongName );
  SearchKey.setSegmentNum( pSeg->ulSegNum );
  SearchKey.setSource( pSeg->pDataW );
  SearchKey.setMarkup( pCNTIda->pCnt->szFormat );
  SearchKey.setSourceLanguage( pCNTIda->pCnt->szSourceLang );
  SearchKey.setTargetLanguage( pCNTIda->pCnt->szTargetLang );

  OtmProposal::clearAllProposals( pMemData->FoundProposals );

  pMem->searchProposal( SearchKey, pMemData->FoundProposals, ulOptions );

  if ( OtmProposal::getNumOfProposals( pMemData->FoundProposals ) != 0 )
  {
    usMatch = (USHORT)pMemData->FoundProposals[0]->getFuzziness();
  } /* endif */

  return( usMatch );
} /* end of function WCountGetMatchInfo */

//
//  function WCountGetFolMemMatch
//
//  Returns the fuziness of the best match for this segment in the folder and document memories
//
static USHORT WCountGetFolMemMatchInfo
(
  PCNTIDA     pCNTIda,                 // control block
  PTBSEGMENT  pSeg                     // segment being looked up
)
{
  PCNTMEMLOOKUPDATA pMemData = (PCNTMEMLOOKUPDATA)pCNTIda->pvMemLookupData;
  USHORT      usMatch = 0;
  unsigned int iMemIndex = 0;

  if ( pMemData->pDocMem != NULL )
  {
    // get match for document memory
    usMatch = WCountGetMatchInfo( pCNTIda, pSeg, pMemData->pDocMem );
    if ( usMatch != 0 )
    {
      pMemData->BestMatch = *(pMemData->FoundProposals[0]);
    } /* endif */

    // skip first folder memory
    iMemIndex = 1;
  } /* endif */

  // get matches for folder memories
  while ( iMemIndex < pMemData->SearchMemoryDBs.size() )
  {
    USHORT usNewMatch = WCountGetMatchInfo( pCNTIda, pSeg, pMemData->SearchMemoryDBs[iMemIndex] );
    if ( usNewMatch > usMatch )
    {
      pMemData->BestMatch = *(pMemData->FoundProposals[0]);
      usMatch = usNewMatch;
    } /* endif */
    iMemIndex++;
  } /*endwhile */

  return( usMatch );
} /* end of function WCountGetFolMemMatchInfo */

//
//  function WCountAddToMemory
//
// Check if there is an exact match for the given segment in our memory
//
static SHORT WCountAddToMemory
(
  PCNTIDA     pCNTIda,                 // control block
  PTBSEGMENT  pSeg                     // segment being added to memory
)
{
  SHORT       sRc = 0;
  PCNTMEMLOOKUPDATA pMemData = (PCNTMEMLOOKUPDATA)pCNTIda->pvMemLookupData;

  OtmProposal NewProposal;

  NewProposal.setDocShortName( pCNTIda->szCountFileName );
  NewProposal.setDocName( pCNTIda->szLongName );
  NewProposal.setSegmentNum( pSeg->ulSegNum );
  NewProposal.setSource( pSeg->pDataW );
  NewProposal.setTarget( pSeg->pDataW );
  NewProposal.setMarkup( pCNTIda->pCnt->szFormat );
  NewProposal.setSourceLanguage( pCNTIda->pCnt->szSourceLang );
  NewProposal.setTargetLanguage( pCNTIda->pCnt->szTargetLang );

  sRc = (SHORT)pMemData->pDupMem->putProposal( NewProposal );

  return( sRc );
} /* end of function WCountAddToMemory */

//
//  function WCountCreateTempMemory
//
// Create a temporary memory for the duplicate checking
//
static SHORT WCountCreateTempMemory
(
  PCNTIDA     pCNTIda                  // control block
)
{
  // create the temporary translation Memory
  int iRC = 0;
  MemoryFactory *pFactory = MemoryFactory::getInstance();    
  PCNTMEMLOOKUPDATA pMemData = (PCNTMEMLOOKUPDATA)pCNTIda->pvMemLookupData;

  // setup name of temporary memory
  strcpy( pMemData->szTempMemoryName, "$CNT-" );
  strcat( pMemData->szTempMemoryName, pCNTIda->pCnt->szLongFolderName );
  strcat( pMemData->szTempMemoryName, "-TEMPMEM" );

  // get folder source language
  FolQueryInfo( pCNTIda->pCnt->szFolderObjName, NULL, NULL, pCNTIda->szStringBuffer, NULL, FALSE );

  // delete any old duplicate memory
  pFactory->deleteMemory( NULL, pMemData->szTempMemoryName );

  // create the memory
  pMemData->pDupMem = pFactory->createMemory( NULL, pMemData->szTempMemoryName, "Temporary memory for duplicates processing", pCNTIda->szStringBuffer, &iRC );

  return( (SHORT)iRC );
} /* end of function WCountCreateTempMemory */

//
//  function WCountDeleteTempMemory
//
// Close and delete the temporary memory
//
static SHORT WCountDeleteTempMemory
(
  PCNTIDA     pCNTIda                  // control block
)
{
  SHORT       sRc = 0;
  MemoryFactory *pFactory = MemoryFactory::getInstance();    
  PCNTMEMLOOKUPDATA pMemData = (PCNTMEMLOOKUPDATA)pCNTIda->pvMemLookupData;

  if ( pMemData->pDupMem != NULL )
  {
    pFactory->closeMemory( pMemData->pDupMem );
    pFactory->deleteMemory( NULL, pMemData->szTempMemoryName );
  } /* endif */

  return( sRc );
} /* end of function WCountDeleteTempMemory */

// escape special characters for XML output
void EscapeCharacters( PSZ pszName, PSZ pszBuffer )
{
  while ( *pszName )
  {
    if ( *pszName == '&' )
    {
      strcpy( pszBuffer, "&amp;" );
      pszBuffer += strlen( pszBuffer );
    }
    else if ( *pszName == '<' )
    {
      strcpy( pszBuffer, "&lt;" );
      pszBuffer += strlen( pszBuffer );
    }
    else if ( *pszName == '>' )
    {
      strcpy( pszBuffer, "&gt;" );
      pszBuffer += strlen( pszBuffer );
    }
    else if ( *pszName == '\"' )
    {
      strcpy( pszBuffer, "&quot;" );
      pszBuffer += strlen( pszBuffer );
    }
    else
    {
      *pszBuffer++ = *pszName;
    } /* endif */
    pszName++;
  } /*endwhile */
  *pszBuffer = EOS;
}


//
//  function WCountOpenFolderMemory
//
//  open the memory DBs associated with the folder
//
static SHORT WCountOpenFolderMemory
(
  PCNTIDA     pCNTIda                  // control block
)
{
  SHORT       sRc = 0;
  HPROP       hProp;                   // handle to document properties
  ULONG       ulErrorInfo;             // error indicator from PRHA
  int         iNumOfMems = 0;
  BOOL        fOK = TRUE;
  PCNTMEMLOOKUPDATA pMemData = (PCNTMEMLOOKUPDATA)pCNTIda->pvMemLookupData;


  // get list of folder memories
  hProp = OpenProperties( pCNTIda->pCnt->szFolderObjName, NULL, PROP_ACCESS_READ, &ulErrorInfo );
  if ( hProp )
  {
    PPROPFOLDER pProp = (PPROPFOLDER) MakePropPtrFromHnd( hProp );

    // add folder main memory to list
    if ( pProp->szLongMemory[0]!= EOS )
    {
      pMemData->SearchMemoryNames.push_back( std::string( pProp->szLongMemory ) );
    }
    else
    {
      pMemData->SearchMemoryNames.push_back( std::string( pProp->szMemory ) );
    } /* endif */
    iNumOfMems++;

    // add folder R/O memories to the list
    if ( pProp->aLongMemTbl[0][0] != EOS )
    {
      int i = 0;
      while ( (i < MAX_NUM_OF_READONLY_MDB) && (pProp->aLongMemTbl[i][0] != EOS) )
      {
        pMemData->SearchMemoryNames.push_back( std::string( pProp->aLongMemTbl[i] ) );
        iNumOfMems++;
        i++;
      } /*endwhile */
    }
    else
    {
      PSZ pszToken;

      //get first R/O memory from list
      strcpy( pCNTIda->szBuffer, pProp->MemTbl );
      pszToken = strtok( pCNTIda->szBuffer, X15_STR );
      while ( (pszToken != NULL) && fOK )
      {
        pMemData->SearchMemoryNames.push_back( std::string( pszToken ) );
        iNumOfMems++;
        pszToken = strtok( NULL, X15_STR );
      } /* endwhile */
    } /* endif */

    // get stop at first exact... flag from properties
    pCNTIda->fStopAtFirstExact = pProp->fStopAtFirstExact;

    CloseProperties( hProp, PROP_QUIT, &ulErrorInfo );
  } /* endif */

  // open the memories
  if ( !sRc && iNumOfMems )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    int i = 0;
    int iRC = 0;
    while ( !sRc && (i < iNumOfMems) )
    {
      OtmMemory*pMem = pFactory->openMemory( NULL, (PSZ)pMemData->SearchMemoryNames[i].c_str(), NONEXCLUSIVE, &iRC );
      if ( pMem != NULL )
      {
        pMemData->SearchMemoryDBs.push_back( pMem );
      }
      else
      {
        sRc = (SHORT)iRC;
        pFactory->showLastError( NULL, (PSZ)pMemData->SearchMemoryNames[i].c_str(), NULL, pCNTIda->pCnt->hwndErrMsg );
      }
      i++;
    } /*endwhile */
  } /* endif */

  pCNTIda->iNumOfFolMems = iNumOfMems;

  return( sRc );
} /* end of function WCountOpenFolderMemory */

//
//  function WCountCloseFolderMemory
//
//  close the memory DBs associated with the folder
//
static SHORT WCountCloseFolderMemory
(
  PCNTIDA     pCNTIda                  // control block
)
{
  SHORT       sRc = 0;
  PCNTMEMLOOKUPDATA pMemData = (PCNTMEMLOOKUPDATA)pCNTIda->pvMemLookupData;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  for ( std::vector<OtmMemory *>::iterator it = pMemData->SearchMemoryDBs.begin(); it != pMemData->SearchMemoryDBs.end(); ++it) 
  {
    if ( *it != NULL ) pFactory->closeMemory( *it );
  } /* endfor */
  pMemData->SearchMemoryDBs.clear();

  if ( pMemData->pDocMem != NULL )
  {
    pFactory->closeMemory( pMemData->pDocMem );
    pMemData->pDocMem = NULL;
  } /* endif */
  return( sRc );
} /* end of function WCountCloseFolderMemory */

//
//  function WCountOpenDocMemory
//
//  open document memory if not open yet
//
static SHORT WCountOpenDocMemory
(
  PCNTIDA     pCNTIda,                 // control block
  PSZ         pszDocMem                // name of document memory
)
{
  SHORT       sRc = 0;
  PCNTMEMLOOKUPDATA pMemData = (PCNTMEMLOOKUPDATA)pCNTIda->pvMemLookupData;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  // ignore mmeory if is the same as the folder memory
  if ( strcmp( pMemData->SearchMemoryNames[0].c_str(), pszDocMem ) == 0 )
  {
    pszDocMem = "";
  } /* endif */

  if ( pMemData->pDocMem != NULL ) // already a document memory active ?
  {
    if ( strcmp( pCNTIda->szDocMemName, pszDocMem ) == 0 )
    {
      // same memory, so we are done
      return( 0 );
    } /* endif */

    // new name is different , de-activate current document memory
    pFactory->closeMemory( pMemData->pDocMem );
    pMemData->pDocMem = NULL;
  } /* endif */

  // open document memory
  if ( pszDocMem[0] )
  {
    int iRC = 0;
    pMemData->pDocMem = pFactory->openMemory( NULL, pszDocMem, NONEXCLUSIVE, &iRC );
    if ( pMemData->pDocMem == NULL )
    {
      sRc = (SHORT)iRC;
      pFactory->showLastError( NULL, pszDocMem, NULL, pCNTIda->pCnt->hwndErrMsg );
    } /* endif */
  } /* endif */

  return( sRc );
} /* end of function WCountOpenDocMemory */

int WCountGetFuzzyClass
(
  PCNTIDA     pCNTIda,                 // control block
  PSZ_W pszSegment,                    // segment from document
  OtmProposal *pProposal,              // memory proposal
  PUSHORT pusWords                     // output: number of words in segment
)
{
  pProposal->getSource( pCNTIda->szSegmentSource, sizeof(pCNTIda->szSegmentSource) / sizeof(CHAR_W) );
  return( CntGetProposalClass( pszSegment, pCNTIda->szSegmentSource, (PLOADEDTABLE) pCNTIda->TBSrcDoc.pDocTagTable, (PBYTE)pCNTIda->TBSrcDoc.pInBuf,
                               (PBYTE)pCNTIda->TBSrcDoc.pTokBuf, pCNTIda->pCnt->sSrcLanguage, pCNTIda->pCnt->ulSrcOemCP, pusWords ) );
} /* end of function WCountGetFuzzyClass */

//
//  function WCountFreeMemLookupData
//
//  free memory lookup data area
//
static void WCountFreeMemLookupData
(
  PCNTIDA     pCNTIda                  // control block
)
{
  if ( pCNTIda->pvMemLookupData != NULL )
  {
    PCNTMEMLOOKUPDATA pMemData = (PCNTMEMLOOKUPDATA)pCNTIda->pvMemLookupData;

    for ( int i=0; i < (int)pMemData->FoundProposals.size(); i++ )
    {
      delete( pMemData->FoundProposals[i] );
    } /* endif */        

    delete( pMemData );
    pCNTIda->pvMemLookupData = NULL;
  } /* end */     
}

// get the class of a proposal
int CntGetProposalClass( PSZ_W pszSegment, PSZ_W pszProposal, PLOADEDTABLE pTable, PBYTE pbBuffer, PBYTE pbTokBuf, SHORT sLangID, ULONG ulOemCP, PUSHORT pusWords )
{
  PFUZZYTOK    pFuzzyTgt = NULL;
  PFUZZYTOK    pFuzzyTok = NULL;       // returned token list
  USHORT       usDiff = 0;             // number of differences
  USHORT       usWords = 0;            // number of words/tokens
  BOOL         fOK = TRUE; 
  static CHAR_W szCleanedSegment[MAX_SEGMENT_SIZE+1];
  static CHAR_W szCleanedProposal[MAX_SEGMENT_SIZE+1];

  // fast exit if one or both strings are empty...
  if ( (*pszSegment == EOS) || (*pszProposal == EOS) )
  {
    return( -1 );
  } /*   endif */

  // remove inline tags from both strings
  CntRemoveTags( pTable, pszSegment, szCleanedSegment, pbTokBuf, TOK_BUFFER_SIZE, ulOemCP );
  CntRemoveTags( pTable, pszProposal, szCleanedProposal, pbTokBuf, TOK_BUFFER_SIZE, ulOemCP );

  // call function to evaluate the differences
  if ( fOK )
  {
    fOK = EQFBFindDiffEx( pTable, pbBuffer, pbTokBuf, szCleanedSegment, szCleanedProposal, sLangID, (PVOID *)&pFuzzyTok, (PVOID *)&pFuzzyTgt, ulOemCP );
  } /* endif */

  if ( fOK )
  {
    EQFBCountDiff( pFuzzyTok, &usWords, &usDiff );
  } /* endif */

  // free allocated buffers and lists
  if ( pFuzzyTgt ) UtlAlloc( (PVOID *)&pFuzzyTgt, 0L, 0L, NOMSG );
  if ( pFuzzyTok ) UtlAlloc( (PVOID *)&pFuzzyTok, 0L, 0L, NOMSG );

  if ( pusWords != NULL ) *pusWords = usWords;

  if ( usDiff > 6 ) usDiff = 6;

  return  ( fOK ? usDiff : -1 );

} /* end of function CntGetProposalClass */

// remove inline tagging from segment data and store result in supplied buffer
USHORT CntRemoveTags( PLOADEDTABLE pTable, PSZ_W pszSegment, PSZ_W pszBuffer, PVOID pbTokBuf, LONG lTokBufSize, ULONG ulCP )
{
  USHORT usRC = 0;
  PBYTE  pStartStop = NULL;

  // build protect start/stop table for tag recognition
  usRC = TACreateProtectTableW( pszSegment, pTable, 0, (PTOKENENTRY)pbTokBuf, (USHORT)lTokBufSize,
                                (PSTARTSTOP *)&pStartStop, pTable->pfnProtTable, pTable->pfnProtTableW, ulCP );
  if ( !usRC )
  {
    PSTARTSTOP pEntry = (PSTARTSTOP)pStartStop;
    PSZ_W pszTarget = pszBuffer;
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
              *pszTarget++ = pszSegment[usPos++];
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

  return( usRC );
}

