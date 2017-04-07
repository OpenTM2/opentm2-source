//+----------------------------------------------------------------------------+
//|EQFMEMEP.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author: Markus Conrad                                                       |
//+----------------------------------------------------------------------------+
//|Description: Functions to export a TM                                       |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|  EQFMemExportStart                                                         |
//|  EQFMemExportProcess                                                       |
//|  EQFMemExportEnd                                                           |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals: CloseFile                                                        |
//|           UtlDelete                                                        |
//|           UtlAlloc                                                         |
//|           UtlDispatch                                                      |
//|           MemGetAddressOfProcessIDA                                        |
//|           MemCreateProcess                                                 |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|  MemExportStart                                                            |
//|  MemExportProcess                                                          |
//|  MemExportWriteFile                                                        |
//|  MemExportWrite                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//+----------------------------------------------------------------------------+

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_DAM

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "MemoryFactory.h"
//#include "MemoryUtil.h"


#include "EQFDDE.H"               // Batch mode definitions
#define INCL_EQFMEM_DLGIDAS       // include dialog IDA definitions
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFMEM.ID>              // Translation Memory IDs
#include <EQFTMTAG.H>             // Translation Memory SGML tags
#include "EQFMEM.ID"              // Translation Memory IDs
#include "OTMFUNC.H"            // public defines for function call interface

  #include "EQFFUNCI.H"           // private defines for function call interface


#define MEM_START_EXPORT    USER_TASK + 1
#define MEM_EXPORT_TASK     USER_TASK + 2
#define MEM_END_EXPORT      USER_TASK + 3


// activate this define to enable the export of context and addinfo data
#define EXPORT_CONTEXT_AND_ADDINFO

typedef struct _MEM_EXPORT_IDA
{
 CHAR         szMemName[MAX_LONGFILESPEC];// Memory database name without extension
 CHAR         szMemPath[2048];            // Full memory database name + path
 HTM          hMem;                       // Handle of memory database
 HFILE        hFile;                      // Handle of file the export should go to
 PMEM_IDA     pIDA;                       // Address to the memory database IDA
 HPROP        hPropMem;                   // Memory database property handle
 PPROP_NTM    pPropMem;                   // pointer to TM properties
 HPROP        hPropLast;                  // Last used property handle
 PMEM_LAST_USED pPropLast;                // Pointer to the last used properties
 BOOL         fAscii;                     // TRUE = ASCII else EQF internal format
 CHAR         szDummy[_MAX_DIR];          // string buffer
 USHORT       usEntryInDirToStop;         // The key where the export should stop
 ULONG        ulTotalBytesWritten;              // Total bytes written to extract file
 CHAR_W       szWorkArea[MEM_EXPORT_WORK_AREA]; // Work area for extract
 TMX_EXT_IN_W  stExtIn;                         // TMX_EXT_IN
 TMX_EXT_OUT_W stExtOut;                        // TMX_EXT_OUT
 ULONG        ulSegmentCounter;                 // Segment counter of progress window
 BOOL         fEOF;                             // Indicates end of file
 BOOL         fFirstExtract;                    // First extract call flag
 HWND         hProgressWindow;                  // Handle of progress indicator window
 ULONG        ulProgressPos;                        // position of progress indicator
 CHAR         acWriteBuffer[MEM_EXPORT_OUT_BUFFER]; // Export output buffer
 ULONG        ulBytesInBuffer;                      // No of bytes already in buffer
 CONTROLSIDA  ControlsIda;                      //ida of control utility
 BOOL         fBatch;                     // TRUE = we are in batch mode
 HWND         hwndErrMsg;                 // parent handle for error messages
 PDDEMEMEXP   pDDEMemExp;                 // ptr to batch memory export data
 USHORT       NextTask;                   // next task for non-DDE batch TM export
 USHORT       usRC;                       // error code in non-DDE batch mode
 USHORT       usExpMode;                  // mode for export
 CHAR         szConvArea[MEM_EXPORT_WORK_AREA]; // Work area for extract
 PSZ           pszNameList;            // pointer to list of TMs being organized
 PSZ           pszActiveName;          // points to current name in pszNameList
 USHORT        usYesToAllMode;         // yes-to-all mode for replace existing files
 ULONG         ulOemCP;                // ASCII cp of system preferences language
 ULONG         ulAnsiCP;
 // fields for external memory export methods
 LONG          lExternalExportHandle;  // handle of external memory export functions
 HMODULE       hmodMemExtExport;                 // handle of external export module/DLL
 PMEMEXPIMPINFO pstMemInfo;                        // buffer for memory information
 PMEMEXPIMPSEG  pstSegment;                        // buffer for segment data
 PFN_EXTMEMEXPORTSTART   pfnMemExpStart;         // function handling start of export
 PFN_EXTMEMEXPORTPROCESS pfnMemExpProcess;       // function handling export of a segment
 PFN_EXTMEMEXPORTEND     pfnMemExpEnd;           // function handling end of export
 BOOL         fDataCorrupted;             // TRUE = data has been corrupted during export
 CHAR_W       szBackConvArea[MEM_EXPORT_WORK_AREA]; // Work area for conversion check
 OtmMemory    *pMem;                      // pointer to memory being exported
 OtmProposal  *pProposal;                   // buffer for proposal data
 CHAR_W       szSegmentBuffer[MAX_SEGMENT_SIZE]; // Buffer for segment data
 CHAR_W       szSegmentBuffer2[MAX_SEGMENT_SIZE]; // Buffer for preprocessed segment data
 int          iComplete;                  // completion rate
 CHAR         szPlugin[MAX_LONGFILESPEC]; // name of memory plugin handling the current memory database
}
MEM_EXPORT_IDA, * PMEM_EXPORT_IDA;



int MemExpSaveAsDlg( HWND hwndParent, MEM_EXPORT_IDA *pExportIDA );

USHORT  MemExportStart(     PPROCESSCOMMAREA, HWND );
USHORT  MemExportProcess(   PMEM_EXPORT_IDA pExportIDA );
USHORT  MemExportWriteFile( PMEM_EXPORT_IDA pExportIDA );
USHORT  MemExportWrite(     PMEM_EXPORT_IDA pExportIDA, PSZ_W  pszToWrite );
USHORT  MemExportPrepareSegment( PMEM_EXPORT_IDA pExportIDA, PSZ_W pszSegmentIn, PSZ_W pszSegmentOut );

/* !brief function handling the GUI mode of the memory import */
BOOL MemGUIExportMemory
( 
  OtmMemory *pMemory, 
  PLISTCOMMAREA pCommArea,
  PSZ pszMemName, 
  PSZ pszNameList,   
  PMEM_IDA      pIDA,                      // Pointer to the Main Memory data area
  HWND hwnd
)
{
  BOOL fOK = TRUE;
  PMEM_EXPORT_IDA     pExportIDA = NULL;         // Pointer to the export IDA

  // Allocate storage for the MEM_EXPORT_IDA. The area will be freed
  // in the function  EQFMemExportEnd if the export process gets started
  // or the area will be freed in EQFMemExportStart if the export start
  // abnormally terminates. If an error occurres in this function the area
  // will be freed here.
  fOK = UtlAlloc( (PVOID *)&pExportIDA, 0L, (LONG)sizeof( MEM_EXPORT_IDA ), ERROR_STORAGE );
  if ( fOK )
  {
    int   iDlgRc = 1;

    // Fill  MEM_EXPORT_IDA with necessary values
    pExportIDA->pMem = pMemory;
    strcpy( pExportIDA->szMemName, pszMemName );
    pExportIDA->pszNameList   = pszNameList;
    pExportIDA->pszActiveName = pszNameList;
    pszNameList = NULL;
    pExportIDA->ulOemCP = GetLangOEMCP(NULL);
    pExportIDA->ulAnsiCP = GetLangAnsiCP(NULL);
    pExportIDA->pProposal = new OtmProposal;

    // Make the memory database IDA available to the export programs
    pExportIDA->pIDA = pIDA;

    iDlgRc = MemExpSaveAsDlg( QUERYACTIVEWINDOW(), pExportIDA );

    if ( iDlgRc || !WinIsWindow( (HAB)NULL, hwnd))  // WM_DESTROYed ?
    {
      fOK = FALSE;
    } /* endif */

    // If file handle not set then set usRc to FALSE
    if ( !pExportIDA->hFile )
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // If usRc post a WM_EQF_MEMEXPORT_START message
  // else clean up allocated storage.
  if ( fOK )
  {
    // build object name
    sprintf( pCommArea->szBuffer, "MEMEXP: %s", pIDA->szMemName );
    SHORT sRC = QUERYSYMBOL( pCommArea->szBuffer );
    if ( sRC == -1 )
    {
        // memory is not in use ==> lock this memory
        SETSYMBOL( pCommArea->szBuffer );
        fOK = CreateProcessWindow( pCommArea->szBuffer, MemExportCallBack, pExportIDA );
    }
    else
    {
      PSZ pTemp = pIDA->szMemName;

      UtlError( ERROR_MEM_NOT_ACCESSIBLE, MB_CANCEL, 1, &pTemp, EQF_ERROR );
      fOK = FALSE;
    } /* endif */

    if ( !fOK )
    {
      /* free IDA, otherwise it will be freed in CallBack */
      if ( pExportIDA->pProposal != NULL ) free(pExportIDA->pProposal); 
      UtlAlloc( (PVOID *)&pExportIDA, 0L, 0L, NOMSG );
    } /* endif */
  }
  else
  {
    // An error occurred  free the MEM_EXPORT_IDA area.
    if ( pExportIDA->pProposal != NULL ) free(pExportIDA->pProposal); 
    UtlAlloc( (PVOID *)&pExportIDA, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
}





// export TM using the standard save as dialog
int MemExpSaveAsDlg( HWND hwndParent, MEM_EXPORT_IDA *pIDA )
{
  BOOL fOK = TRUE;
  EQFINFO ErrorInfo;

  // init handling (borrowed from INITDLG of MemExportDlg
  pIDA->hFile = NULLHANDLE;
  UtlMakeEQFPath( pIDA->szDummy, NULC, SYSTEM_PATH, NULL );

  //--- access last used values by opening TM last used properties
  if ( fOK )
  {
     fOK = MemOpenProp(&(pIDA->hPropLast), (PVOID *)&(pIDA->pPropLast),
                       MEMORYLU_PROPERTIES_NAME, pIDA->szDummy,
                       PROP_ACCESS_WRITE, TRUE );
  } /* endif */

  if ( fOK )
  {
    strcpy( pIDA->ControlsIda.szExt, MEM_EXPORT_PATTERN_EXT );
    strcpy( pIDA->ControlsIda.szSelectedName, pIDA->szMemName );
    OEMTOANSI( pIDA->ControlsIda.szSelectedName );
  } /* endif */

  // call standard save as dialog
  {
    BOOL fDone = TRUE;
    BOOL fExtAdded = FALSE;

    OPENFILENAME OpenFileName;

    memset( &OpenFileName, 0, sizeof(OpenFileName) );
    OpenFileName.lStructSize        = sizeof(OpenFileName);
    OpenFileName.hwndOwner          = hwndParent;
    OpenFileName.hInstance          = NULLHANDLE;
    OpenFileName.lpstrFilter        = MEM_FORMAT_FILTERS_EXP;
    OpenFileName.lpstrCustomFilter  = NULL;
    OpenFileName.nMaxCustFilter     = 0;
    OpenFileName.nFilterIndex       = ( pIDA->pPropLast->usExpMode ) ?
                                              pIDA->pPropLast->usExpMode :
                                              SGMLFORMAT_ASCII;
    strcpy( pIDA->ControlsIda.szPathContent, pIDA->szMemName );
//     strcat( pIDA->ControlsIda.szPathContent, MEM_EXPORT_PATTERN_EXT ); // no default extension anymore
    OEMTOANSI( pIDA->ControlsIda.szPathContent );
    OpenFileName.lpstrFile          = pIDA->ControlsIda.szPathContent;
    OpenFileName.nMaxFile           = sizeof(pIDA->ControlsIda.szPathContent);
    OpenFileName.lpstrFileTitle     = NULL;
    OpenFileName.nMaxFileTitle      = 0;
    OpenFileName.lpstrInitialDir    = pIDA->pPropLast->szExpDirLastUsed;
    if ( pIDA->pszNameList )
    {
      sprintf( (PSZ)pIDA->szWorkArea, "Export the selected TranslationMemory databases" );
    }
    else
    {
      sprintf( (PSZ)&pIDA->szWorkArea[0], "Export TranslationMemory %s", pIDA->szMemName );
      OEMTOANSI( (PSZ)(pIDA->szWorkArea) );
    } /* endif */
    OpenFileName.lpstrTitle         = (PSZ)pIDA->szWorkArea;
    OpenFileName.Flags              = OFN_ENABLESIZING | OFN_EXPLORER |
                                      OFN_LONGNAMES | OFN_NODEREFERENCELINKS |
                                      OFN_NOTESTFILECREATE | OFN_NOCHANGEDIR |
                                      OFN_PATHMUSTEXIST;
    OpenFileName.nFileOffset        = 0;
    OpenFileName.nFileExtension     = 0;
//     OpenFileName.lpstrDefExt        = "EXP";
    OpenFileName.lpstrDefExt        = NULL;   // no default extension, we have to set the extension depending of the format
    OpenFileName.lCustData          = 0L;
    OpenFileName.lpfnHook           = NULL;
    OpenFileName.lpTemplateName     = NULL;

    do
    {
      fOK = (GetSaveFileName( &OpenFileName ) != 0 );

      if ( fOK )
      {
        pIDA->pPropLast->usExpMode = pIDA->usExpMode = (USHORT)OpenFileName.nFilterIndex;
        fExtAdded = FALSE;

        // set export file extension if none specified by user
        {
          // find file name within full path
          PSZ pszFileName = strrchr( pIDA->ControlsIda.szPathContent, '\\' );
          if ( pszFileName != NULL )
          {
            pszFileName++;               
          }
          else
          {
            pszFileName = pIDA->ControlsIda.szPathContent;
          } /* endif */

          // set default extension if none specified
          if ( strchr( pszFileName, '.' ) == NULL )
          {
            // append file extension depending on export format
            switch ( pIDA->pPropLast->usExpMode )
            {
              case MEM_FORMAT_TMX_NOCRLF:
              case MEM_FORMAT_TMX_UTF8_NOCRLF:
              case MEM_FORMAT_TMX:
              case MEM_FORMAT_TMX_UTF8:
                strcat( pIDA->ControlsIda.szPathContent, ".TMX" );
                  break;
              default:
                strcat( pIDA->ControlsIda.szPathContent, ".EXP" );
                  break;
            } /*endswitch */
            fExtAdded = TRUE;
          } /* endif */
        }
      } /* endif */

      // get overwrite confirmation (has to be done outside the dialog as the file extension
      // is not available within the SaveAs dialog
      fDone = TRUE;
      if ( fOK && UtlFileExist(pIDA->ControlsIda.szPathContent)  )
      {
        PSZ pszParms[2]; 

        pszParms[0] = pIDA->ControlsIda.szPathContent;
        if ( UtlError( FILE_EXISTS, MB_OKCANCEL | MB_DEFBUTTON2, 1, pszParms, EQF_QUERY ) != MBID_OK )
        {
          PSZ pszMemName; 

          fDone = FALSE;                   // redo SaveAs dialog

          strcpy ( pIDA->pPropLast->szExpDirLastUsed, pIDA->ControlsIda.szPathContent );
          pszMemName = UtlSplitFnameFromPath( pIDA->pPropLast->szExpDirLastUsed );
          if ( fExtAdded )
          {
            Utlstrccpy( pIDA->ControlsIda.szPathContent, pszMemName, DOT );
          }
          else
          {
            strcpy( pIDA->ControlsIda.szPathContent, pszMemName );
          } /* endif */
        } /* endif */
      } /* endif */
    } while ( !fDone );

  }

  // prepare TM export (borrowed from MemCommandhandling)
  if ( fOK )
  {
    //--- open file to export to
    USHORT usAction;
    USHORT usDosRc = UtlOpen( pIDA->ControlsIda.szPathContent, &(pIDA->hFile), &usAction,
                       0L, FILE_NORMAL, FILE_TRUNCATE | FILE_CREATE,
                       (OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE | OPEN_FLAGS_WRITE_THROUGH),
                       0L, TRUE );

    if ( usDosRc )  //--- error from UtlOpen
    {
      pIDA->hFile = NULLHANDLE;
      fOK = FALSE;
    } /* endif */

    //--- If no error from open, save last used values
    if ( fOK )
    {
      EQFINFO ErrorInfo;
      strcpy( pIDA->pPropLast->szExpDirLastUsed, pIDA->ControlsIda.szPathContent );
      UtlSplitFnameFromPath( pIDA->pPropLast->szExpDirLastUsed );
      strncpy( pIDA->pPropLast->szExpDriveLastUsed, pIDA->ControlsIda.szPathContent, 2 );
      SaveProperties( pIDA->hPropLast, &ErrorInfo );
    } /* endif */
  } /* endif */

  // cleanup
  if ( pIDA->hPropLast )
    CloseProperties( pIDA->hPropLast, PROP_QUIT, &ErrorInfo);
  if ( !fOK )
  {
    if ( pIDA->hFile )
    {
      CloseFile( &pIDA->hFile );
      pIDA->hFile = NULLHANDLE;
    } /* endif */
    if ( pIDA->pMem != NULL)
    {
      MemoryFactory *pFactory = MemoryFactory::getInstance();
      pFactory->closeMemory( pIDA->pMem );
      pIDA->pMem = NULL;
    } /* endif */
  } /* endif */


  return( fOK ? 0 : 1 );
} /* end of function MemExpSaveAsDlg */


USHORT MemFuncPrepExport
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemname,              // name of Translation Memory
  PSZ         pszOutFile,              // fully qualified name of output file
  LONG        lOptions                 // options for Translation Memory export
);
USHORT MemFuncExportProcess
(
  PFCTDATA    pData                    // function I/F session data
);

/**********************************************/
/* Callback function for Export ProcessWindow */
/**********************************************/

MRESULT MemExportCallBack
(
  PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
  HWND             hwnd,               // handle of process window
  WINMSG           message,            // message to be processed
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
)
{
  PMEM_EXPORT_IDA   pEIDA;             // pointer to instance area
  MRESULT           mResult = FALSE;   // return code for handler proc
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  switch( message)
  {
    /******************************************************************/
    /* WM_CREATE:                                                     */
    /*                                                                */
    /* Fill fields in communication area                              */
    /* Initialize data of callback function                           */
    /******************************************************************/
    case WM_CREATE :
      /**************************************************************/
      /* Anchor IDA                                                 */
      /**************************************************************/
      pEIDA               = (PMEM_EXPORT_IDA)PVOIDFROMMP2(mp2);
      pCommArea->pUserIDA = pEIDA;

      /****************************************************************/
      /* supply all information required to create the process        */
      /* window                                                       */
      /****************************************************************/
      pCommArea->sProcessWindowID = ID_MEMEXP_WINDOW;
      pCommArea->sProcessObjClass = clsMEMORYEXP;
      if ( pEIDA->fBatch )
      {
        pCommArea->Style            = PROCWIN_BATCH;
      }
      else
      {
        pCommArea->Style            = PROCWIN_TEXTSLIDER;
      } /* endif */
      pCommArea->sSliderID        = ID_MEMEXP_SLIDER;
      pCommArea->sTextID          = ID_MEMEXP_TEXT;
      LOADSTRING( NULLHANDLE, hResMod, MEM_EPTITLE, pCommArea->szBuffer );
      sprintf( pCommArea->szTitle, "%s %s", pCommArea->szBuffer,
               pEIDA->szMemName );
      LOADSTRING( NULLHANDLE, hResMod, MEM_EPLINE2, pCommArea->szBuffer );
      sprintf( pCommArea->szText, "%s %s", pCommArea->szBuffer,
               pEIDA->ControlsIda.szPathContent );
      pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_TMEMEXPICON); //hiconTMEMEXP;
      pCommArea->fNoClose         = FALSE;
      pCommArea->swpSizePos.x     = 100;
      pCommArea->swpSizePos.y     = 100;
      pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 60;
      pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 10;
      pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
      pCommArea->asMsgsWanted[1]  = 0;
      pCommArea->usComplete       = 0;
      break;


      /****************************************************************/
      /* Start processing by posting WM_EQF_PROCESSTASK               */
      /****************************************************************/
    case WM_EQF_INITIALIZE:
      WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                  MP1FROMSHORT( MEM_START_EXPORT ), 0L );
      break;

    /******************************************************************/
    /* WM_CLOSE:                                                      */
    /*                                                                */
    /* Prepare/initialize shutdown of process                         */
    /******************************************************************/
    case WM_CLOSE:
      pEIDA = (PMEM_EXPORT_IDA)pCommArea->pUserIDA;
      if ( pEIDA->hMem )
      {
         EQFMemExportEnd( pCommArea, hwnd, MP2FROMSHORT(1) );
      } /* endif */
      mResult = MRFROMSHORT( FALSE );  // = continue with close
      break;

    /******************************************************************/
    /* WM_DESTROY:                                                    */
    /*                                                                */
    /* Cleanup all resources used by the process                      */
    /******************************************************************/
    case WM_DESTROY:
      pEIDA = (PMEM_EXPORT_IDA)pCommArea->pUserIDA;

      if ( pEIDA )
      {
        if ( pEIDA->hMem )
        {
          EQFMemExportEnd( pCommArea, hwnd, MP2FROMSHORT(1) );
        } /* endif */

        if ( pEIDA->pDDEMemExp != NULL )
        {
          WinPostMsg( pEIDA->pDDEMemExp->hwndOwner, WM_EQF_DDE_ANSWER, NULL,
                      MP2FROMP(&pEIDA->pDDEMemExp->DDEReturn) );
        } /* endif */
      } /* endif */

      REMOVESYMBOL( pCommArea->szObjName );


      if ( pEIDA ) {
         if ( pEIDA->pProposal != NULL ) free(pEIDA->pProposal); 
         UtlAlloc( (PVOID *) &pEIDA, 0L, 0L, NOMSG );
      }
      pCommArea->pUserIDA = NULL;
      break;


    /******************************************************************/
    /* WM_EQF_TERMINATE:                                              */
    /*                                                                */
    /* Allow or disable termination of process                        */
    /******************************************************************/
    case WM_EQF_TERMINATE:
      mResult = MRFROMSHORT( FALSE );          // = continue with close
      break;

    /******************************************************************/
    /* WM_INITMENU:                                                   */
    /*                                                                */
    /* Enable/Disable actionbar items                                 */
    /******************************************************************/
    case WM_INITMENU:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

    case WM_EQF_PROCESSTASK:
      switch ( SHORT1FROMMP1(mp1) )
      {
        case MEM_START_EXPORT:
          EQFMemExportStart( pCommArea, hwnd );
          break;

        case MEM_EXPORT_TASK:
          EQFMemExportProcess( pCommArea, hwnd );
          break;

        case MEM_END_EXPORT:
          EQFMemExportEnd( pCommArea, hwnd, mp2 );
          break;
      } /* endswitch */
      break;
  } /* endswitch */
  return( mResult );
} /* end of function MemExportCallBack */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFMemExportStart                                        |
//+----------------------------------------------------------------------------+
//|Function call:     VOID EQFMemExportStart ( HWND hWnd, LPARAM mp2 )         |
//|                   handles the message WM_EQF_MEMEXPORT_START               |
//+----------------------------------------------------------------------------+
//|Description:       Memory database export initialization and start          |
//+----------------------------------------------------------------------------+
//|Parameters:        hwnd   dialog handle                                     |
//|                   mp2    pointer to MEM_EXPORT_IDA                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//| call MemExportStart to start export                                        |
//|                                                                            |
//| if ( error from MemExportStart )                                           |
//| {                                                                          |
//|    access export IDA MEM_EXPORT_IDA                                        |
//|                                                                            |
//|    close TM and output file                                                |
//|                                                                            |
//|    delete output file                                                      |
//|                                                                            |
//|    close analysis file if compiler option is set                           |
//|                                                                            |
//|    free storage                                                            |
//| }                                                                          |
//| else                                                                       |
//| {                                                                          |
//|    post message WM_EQF_MEMEXPORT_PROCESS to start export                   |
//|    ( mp1 contains return code from MemExportStart, which is the            |
//|      process ID )                                                          |
//| }                                                                          |
//+----------------------------------------------------------------------------+
USHORT EQFMemExportStart( PPROCESSCOMMAREA pCommArea,
                        HWND             hWnd )
{
  PMEM_EXPORT_IDA   pExportIDA;          // Pointer to the export IDA
  USHORT            usRc;                // Return code to control the process
  USHORT            usTmRc;              // Return code of Tm functions
  USHORT            usRC = NO_ERROR;          // function return code

  //--- Initialize the load process. If it failed close
  //--- the MemoryDb and the input file.
  //--- call function to  Initialize the load
  usRc = MemExportStart( pCommArea, hWnd );
  pExportIDA = (PMEM_EXPORT_IDA)pCommArea->pUserIDA;

  //--- if error starting export
  if ( !usRc )
  {
    HWND hwndErrMsg;

    //--- Get pointer to export IDA
    hwndErrMsg = pExportIDA->hwndErrMsg;

    //--- Close MemoryDb and output file.
    //--- Return codes are for testing purposes only
    if ( pExportIDA->pMem != NULL)
    {
      MemoryFactory *pFactory = MemoryFactory::getInstance();
      pFactory->closeMemory( pExportIDA->pMem );
      pExportIDA->pMem = NULL;
    } /* endif */
    CloseFile( &(pExportIDA->hFile));

    //--- delete file to which the TM should be exported
    UtlDelete( pExportIDA->ControlsIda.szPathContent, 0L, FALSE );

    //--- free export IDA
    if ( pExportIDA->pProposal != NULL ) free(pExportIDA->pProposal); 
    UtlAlloc( (PVOID *) &pExportIDA, 0L, 0L, NOMSG );
    pCommArea->pUserIDA = NULL;

    // Dismiss the slider window if it had been created
    if ( hwndErrMsg != HWND_FUNCIF )
    {
      EqfRemoveObject( TWBFORCE, hWnd );
    } /* endif */
  }
  else if ( pExportIDA->hwndErrMsg == HWND_FUNCIF )
  {
    pExportIDA->NextTask = MEM_EXPORT_TASK;
  }
  else
  {
    // Move process-ID which is stored in usRc into the
    // mp1 and issue a message WM_EQF_MEMEXPORT_PROCESS
    WinPostMsg( hWnd, WM_EQF_PROCESSTASK,
                MP1FROMSHORT( MEM_EXPORT_TASK ), NULL);
  } /* endif */
  return( usRC );
} /* end of function EQFMemExportStart */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFMemExportProcess                                      |
//+----------------------------------------------------------------------------+
//|Function call:     VOID EQFMemExportProcess ( HWND hWnd, WPARAM mp1 )       |
//+----------------------------------------------------------------------------+
//|Description:       Exports a TM                                             |
//|                   handles the message WM_EQF_MEMEXPORT_PROCESS             |
//+----------------------------------------------------------------------------+
//|Parameters:        hwnd   dialog handle                                     |
//|                   mp1    contains process ID                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//| access TM IDA                                                              |
//|                                                                            |
//| if ( access to IDA )                                                       |
//| {                                                                          |
//|    call function MemGetAddressOfProcessIDA to get the address of the       |
//|    process IDA                                                             |
//|                                                                            |
//|    If ( function returns a address )                                       |
//|    {                                                                       |
//|       call function MemExportProcess to do the export                      |
//|                                                                            |
//|       switch ( return code from function MemExportProcess )                |
//|       {                                                                    |
//|          case MEM_PROCESS_OK : // no error                                 |
//|             update slider position                                         |
//|                                                                            |
//|             dispatch waiting messages                                      |
//|                                                                            |
//|             post message WM_EQF_MEMEXPORT_PROCESS for next export step     |
//|                                                                            |
//|             break;                                                         |
//|          case MEM_PROCESS_END : // export is finished                      |
//|             set slider to 100%                                             |
//|                                                                            |
//|             if ( exported file is empty )                                  |
//|             {                                                              |
//|                delete file and display error message                       |
//|             }                                                              |
//|             else                                                           |
//|             {                                                              |
//|                close exported file and display completion message          |
//|                                                                            |
//|                post message to end the export process                      |
//|             }                                                              |
//|             break;                                                         |
//|          default : // any other error                                      |
//|             display error message                                          |
//|                                                                            |
//|             close file                                                     |
//|                                                                            |
//|             post message to end export process                             |
//|             break;                                                         |
//|       }                                                                    |
//|    }                                                                       |
//| }                                                                          |
//+----------------------------------------------------------------------------+
USHORT EQFMemExportProcess ( PPROCESSCOMMAREA  pCommArea,
                           HWND              hWnd )
{
  USHORT            usRc = TRUE;              // Return code to control a process
  PMEM_EXPORT_IDA   pExportIDA;               // Pointer to the data for the load
  PSZ               pReplString[3];           // Array of pointers to replacement strings
  CHAR              szNumber[10];             // Temporary character string
  ULONG             ulProgPos;                // new position of progress indicator
  USHORT            fOk=TRUE;                 // handle error/warning to cmd handler
  USHORT            usRC = NO_ERROR;          // function return code
  //--- Get address of _MEM_IDA
  //--- Get the address of the process IDA by means of the process handle
  pExportIDA = (PMEM_EXPORT_IDA)pCommArea->pUserIDA;

  //--- Call the memory database export function
  //--- and handle return codes
  usRc = MemExportProcess( pExportIDA );

  switch ( usRc )
  {
    //--------------------------------------------------------------------
    case MEM_PROCESS_OK:
       pCommArea->usComplete = (USHORT)pExportIDA->iComplete;

       //--- dispatch waiting messages
       if ( pExportIDA->hwndErrMsg != HWND_FUNCIF )
       {
         UtlDispatch();

         //--- Post the next WM_EQF_MEMEXPORT_PROCESS
         WinPostMsg( hWnd, WM_EQF_PROCESSTASK,
                     MP1FROMSHORT( MEM_EXPORT_TASK ), NULL );
       } /* endif */
       break;
    //--------------------------------------------------------------------
    case MEM_PROCESS_END:
       //--- Close the exported file
       //--- If ulSegmentCounter = ZERO issue the message box
       //--- "No segments have been exported to %1." otherwise
       //--- issue a message "Memory Database Export successfully completed."
       CloseFile( &(pExportIDA->hFile) );
       OEMTOANSI( pExportIDA->szMemName );
       pReplString[0] = pExportIDA->ControlsIda.szPathContent;
       pReplString[1] = pExportIDA->szMemName;

       //--- set the progress indicator to 100 percent
       pCommArea->usComplete = 100;
       if ( pExportIDA->hwndErrMsg != HWND_FUNCIF )
       {
         WinSendMsg( hWnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT( 100 ), NULL );
       } /* endif */

       if ( pExportIDA->ulSegmentCounter == 0 )
       {
         // Delete the exported file because it is empty
         // and issue error message
         UtlDelete ( pExportIDA->ControlsIda.szPathContent, 0L, FALSE );
         UtlErrorHwnd( ERROR_MEM_EXPORT_NOSEGMENTS, MB_CANCEL, 2,
                       &(pReplString[0]), EQF_WARNING, pExportIDA->hwndErrMsg );
         fOk=FALSE;
       }
       else if ( !pExportIDA->pszNameList )
       {
         pReplString[2] = _ltoa( pExportIDA->ulSegmentCounter, szNumber, 10 );
         UtlErrorHwnd( MESSAGE_MEM_EXPORT_COMPLETED, MB_OK, 3,
                   &(pReplString[0]), EQF_INFO,
                   pExportIDA->hwndErrMsg );
       } /* endif */
       ANSITOOEM( pExportIDA->szMemName );

       //--- Issue message WM_EQF_MEMEXPORT_END
       if ( pExportIDA->hwndErrMsg == HWND_FUNCIF )
       {
         pExportIDA->NextTask = MEM_END_EXPORT;
       }
       else
       {
         WinPostMsg( hWnd, WM_EQF_PROCESSTASK,
                     MP1FROMSHORT( MEM_END_EXPORT ), NULL );
       } /* endif */
       break;
    //--------------------------------------------------------------------
    default:
     //--- Close the exported file
     //--- Issue a message "Memory Database Export abnormally terminated."
     OEMTOANSI( pExportIDA->szMemName );
     pReplString[0] = pExportIDA->ControlsIda.szPathContent;
     pReplString[1] = pExportIDA->szMemName;
     CloseFile( &(pExportIDA->hFile) );
     UtlDelete( pExportIDA->ControlsIda.szPathContent, 0L, FALSE );
     UtlErrorHwnd( ERROR_MEM_EXPORT_TERMINATED, MB_CANCEL, 2,
               &(pReplString[0]), EQF_ERROR, pExportIDA->hwndErrMsg );
     ANSITOOEM( pExportIDA->szMemName );
     fOk=FALSE;

     //--- Issue message WM_EQF_MEMEXPORT_END
     if ( pExportIDA->hwndErrMsg == HWND_FUNCIF )
     {
       pExportIDA->NextTask = MEM_END_EXPORT;
     }
     else
     {
       WinPostMsg( hWnd, WM_EQF_PROCESSTASK,
                   MP1FROMSHORT( MEM_END_EXPORT ), NULL );
     } /* endif */
     break;
  } /* end switch */

   if ( !fOk )
   {
     if ( pExportIDA->fBatch )
     {
       pExportIDA->usRC = UtlGetDDEErrorCode( pExportIDA->hwndErrMsg );
       if ( pExportIDA->pDDEMemExp != NULL )
       {
         pExportIDA->pDDEMemExp->DDEReturn.usRc =  UtlGetDDEErrorCode( pExportIDA->hwndErrMsg );
       } /* endif */
     }
     else
     {
       pExportIDA->usRC = UtlQueryUShort( QS_LASTERRORMSGID );
     } /* endif */
     usRC = pExportIDA->usRC;
   } /* endif */

  return( usRC );
} /* end of function EQFMemExportProcess */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFMemExportEnd                                          |
//+----------------------------------------------------------------------------+
//|Function call:     VOID EQFMemExportEnd ( HWND   hWnd,                      |
//|                                          WPARAM mp1,                       |
//|                                          LPARAM mp2  )                     |
//+----------------------------------------------------------------------------+
//|Description:       Handle the message WM_EQF_MEMEXPORT_END                  |
//+----------------------------------------------------------------------------+
//|Parameters:        hWnd dialog                                              |
//|                   mp1  contains the process ID                             |
//|                   mp2  if mp2 not 0 then the process is forced             |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//| get access to TM IDA                                                       |
//|                                                                            |
//| if ( access available )                                                    |
//| {                                                                          |
//|    call function MemGetAddressOfProcessIDA to get the address of the       |
//|    process IDA                                                             |
//|                                                                            |
//|    If ( function returns a address )                                       |
//|    {                                                                       |
//|       if ( close was forced without completeion )                          |
//|       {                                                                    |
//|          close file and display warning messag                             |
//|       }                                                                    |
//|    }                                                                       |
//|                                                                            |
//|    If ( function returns a address )                                       |
//|    {                                                                       |
//|       do cleanup                                                           |
//|    }                                                                       |
//| }                                                                          |
//+----------------------------------------------------------------------------+
USHORT EQFMemExportEnd ( PPROCESSCOMMAREA pCommArea,
                       HWND       hWnd,
                       LPARAM     mp2  ) // if mp2 not 0 then the process
                                         //  was forced
{
  USHORT            usTmRc;              // Return code from Tm functions
  PMEM_EXPORT_IDA   pExportIDA;          // Pointer to the data for the export
  PSZ               pReplString[2];      // Array of pointers to replacement strings
  USHORT            usRC = NO_ERROR;     // function return code
  HWND              hwndErrMsg;          // handle of window for error messages

  //--- Get the address of the process IDA by means of the process handle
  pExportIDA = (PMEM_EXPORT_IDA)pCommArea->pUserIDA;
  hwndErrMsg = pExportIDA->hwndErrMsg;

  // call termination function of external memory export function 
  if ( (pExportIDA->usExpMode == MEM_FORMAT_TMX) || (pExportIDA->usExpMode == MEM_FORMAT_TMX_UTF8) ||
       (pExportIDA->usExpMode == MEM_FORMAT_TMX_NOCRLF) || (pExportIDA->usExpMode == MEM_FORMAT_TMX_UTF8_NOCRLF) )
  {
    if ( pExportIDA->pfnMemExpEnd )
    {
      pExportIDA->pfnMemExpEnd( pExportIDA->lExternalExportHandle );
    } /* endif */
    if ( pExportIDA->pstMemInfo ) UtlAlloc( (PVOID *)&pExportIDA->pstMemInfo, 0L, 0L, NOMSG );
    if ( pExportIDA->pstSegment ) UtlAlloc( (PVOID *)&pExportIDA->pstSegment, 0L, 0L, NOMSG );
    if ( pExportIDA->hmodMemExtExport ) DosFreeModule( pExportIDA->hmodMemExtExport );
  }

  //--- Check if the termination was due to a
  //--- CLOSE message. Mp2 is in that case not zero.
  if ( SHORT1FROMMP2( mp2 ) != 0 )
  {
     //--- Close the exported file
     //--- Issue a message:Export of memory database %1 to file %2 was
     //---                 forced before completion.
     OEMTOANSI( pExportIDA->szMemName );
     pReplString[0] = pExportIDA->ControlsIda.szPathContent;
     pReplString[1] = pExportIDA->szMemName;
     if ( pExportIDA->hFile ) CloseFile( &(pExportIDA->hFile) );
     pExportIDA->hFile = NULL;

     //--- delete file to which TM should be exported
     UtlDelete( pExportIDA->ControlsIda.szPathContent, 0L, FALSE );
     if ( !pExportIDA->fBatch )
     {
       UtlError( ERROR_MEM_EXPORT_TERM_FORCED, MB_CANCEL, 2, &(pReplString[0]), EQF_WARNING );
     } /* endif */
     ANSITOOEM( pExportIDA->szMemName );
  } /* endif */

  //--- Close MemoryDb and input file. Return codes are for testing purposes only
  if ( pExportIDA->pMem != NULL)
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    pFactory->closeMemory( pExportIDA->pMem );
    pExportIDA->pMem = NULL;
  } /* endif */
  if ( pExportIDA->hFile ) CloseFile( &(pExportIDA->hFile) );

  // unlock memory export object
  {
    CHAR ObjName[300]; 
    sprintf( ObjName, "MEMEXP: %s", pExportIDA->szMemName );
    REMOVESYMBOL( ObjName );
  }

  /********************************************************************/
  /* Inform DDE handler that task has been completed                  */
  /********************************************************************/
  if ( pExportIDA->pDDEMemExp != NULL )
  {
    WinPostMsg( pExportIDA->pDDEMemExp->hwndOwner, WM_EQF_DDE_ANSWER, NULL,
                MP2FROMP(&pExportIDA->pDDEMemExp->DDEReturn) );
  } /* endif */

  // continue with next TM if any
  if ( pExportIDA->pszNameList )
  {
    pExportIDA->pszActiveName += strlen(pExportIDA->pszActiveName) + 1;

    if ( *pExportIDA->pszActiveName )
    {
      BOOL fStartExport = FALSE;
      MemoryFactory *pFactory = MemoryFactory::getInstance();

      do
      {
        pFactory->splitObjName( pExportIDA->pszActiveName, pExportIDA->szPlugin, sizeof(pExportIDA->szPlugin), pExportIDA->szMemName, sizeof(pExportIDA->szMemName) );

        // build outfile name and get overwrite permission for existing files
        UtlSplitFnameFromPath( pExportIDA->ControlsIda.szPathContent );
        strcat( pExportIDA->ControlsIda.szPathContent, BACKSLASH_STR );
        strcat( pExportIDA->ControlsIda.szPathContent, pExportIDA->szMemName );
        if ( (pExportIDA->usExpMode == MEM_FORMAT_TMX) || (pExportIDA->usExpMode == MEM_FORMAT_TMX_UTF8) ||
             (pExportIDA->usExpMode == MEM_FORMAT_TMX_NOCRLF) || (pExportIDA->usExpMode == MEM_FORMAT_TMX_UTF8_NOCRLF) )
        {
          strcat( pExportIDA->ControlsIda.szPathContent, ".TMX" );
        }
        else
        {
          strcat( pExportIDA->ControlsIda.szPathContent, MEM_EXPORT_PATTERN_EXT );
        } /* endif */

        if ( UtlFileExist( pExportIDA->ControlsIda.szPathContent ) )
        {
          if ( pExportIDA->usYesToAllMode == MBID_EQF_YESTOALL )
          {
            // we are already in yes-to-all mode so do nothing and continue export
            fStartExport = TRUE;
          }
          else
          {
            USHORT usMBCode;
            PSZ pszParm = pExportIDA->ControlsIda.szPathContent;
            usMBCode = UtlError( ERROR_FILE_EXISTS_ALREADY, MB_EQF_YESTOALL, 1,
                                 &pszParm, EQF_QUERY );
            if ( usMBCode == MBID_EQF_YESTOALL )
            {
              // switch to yes-to-all mode and start export
              pExportIDA->usYesToAllMode = MBID_EQF_YESTOALL;
              fStartExport = TRUE;
            }
            else if ( usMBCode == MBID_CANCEL )
            {
              // stop all further processing
              *pExportIDA->pszActiveName = EOS;
            }
            else if ( usMBCode == MBID_NO )
            {
              // skip this file by removing it from the name list
              PSZ pszCurrent = pExportIDA->pszActiveName;
              PSZ pszNext = pszCurrent + strlen(pszCurrent) + 1;
              while ( *pszNext )
              {
                while ( *pszNext ) *pszCurrent++ = *pszNext++; // copy string
                *pszCurrent++ = *pszNext++;  // copy string enddelimiter
              } /* endwhile */
              *pszCurrent = EOS;             // add end-of-list delimiter
            }
            else
            {
              // continue export
              fStartExport = TRUE;
            } /* endif */
          } /* endif */
        }
        else
        {
          // output file does not exist, continue export
          fStartExport = TRUE;
        } /* endif */
      } while ( *pExportIDA->pszActiveName && !fStartExport );
    } /* endif */


    if ( pExportIDA->pszActiveName[0] )
    {
      // prepare export of next TM
    }
    else
    {
      // show termination message
      if ( SHORT1FROMMP2( mp2 ) == 0 )
      {
        PSZ pszTMNames = NULL;  // buffer for TM name list

        // compute size of required buffer
        PSZ pszNext = pExportIDA->pszNameList;
        int iSize = 5;
        while ( *pszNext )
        {
          int iLen = strlen(pszNext) + 1;
          iSize += iLen + 5;
          pszNext += iLen;
        } /* endwhile */

        // allocate buffer
        UtlAlloc( (PVOID *)&pszTMNames, 0L, iSize, ERROR_STORAGE );

        // fill buffer with TM names
        if ( pszTMNames )
        {
          PSZ pszNext = pExportIDA->pszNameList;
          PSZ pszCurPos = pszTMNames;
          while ( *pszNext )
          {
            int iNameLen = strlen( pszNext );
            *pszCurPos++ = '\"';
            strcpy( pszCurPos, pszNext );
            pszNext += iNameLen + 1;
            pszCurPos += iNameLen;
            *pszCurPos++ = '\"';
            if ( *pszNext )
            {
              // add delimiters for following TM name
              *pszCurPos++ = ',';
              *pszCurPos++ = ' ';
            } /* endif */
          } /* endwhile */
        } /* endif */
        UtlError ( MESSAGE_MEM_EXP_COMPLETED_MUL, MB_OK, 1, &pszTMNames, EQF_INFO );
        if ( pszTMNames ) UtlAlloc( (PVOID *)&pszTMNames, 0L, 0L, NOMSG );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( pExportIDA->pszNameList && pExportIDA->pszActiveName[0] )
  {
    BOOL fIsNew = FALSE;
    BOOL fOK = TRUE;
    int iRC = 0;

    // restart export with active TM
    pExportIDA->fFirstExtract = TRUE;
    pExportIDA->fEOF = FALSE;
    pExportIDA->ulSegmentCounter = 0L;
    pExportIDA->hFile = NULLHANDLE;

    MemoryFactory *pFactory = MemoryFactory::getInstance();
    pExportIDA->pMem = pFactory->openMemory( pExportIDA->szPlugin, pExportIDA->szMemName, EXCLUSIVE, &iRC );
    if ( pExportIDA->pMem == NULL )
    {
      pFactory->showLastError( pExportIDA->szPlugin, pExportIDA->szMemName, pExportIDA->pMem, pExportIDA->hwndErrMsg );
      fOK = FALSE;
    }

    if ( fOK )
    {
      USHORT usDosRc, usAction;


      usDosRc = UtlOpen( pExportIDA->ControlsIda.szPathContent, &(pExportIDA->hFile), &usAction,
                         0L, FILE_NORMAL, FILE_TRUNCATE | FILE_CREATE,
                         (USHORT)(OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE | OPEN_FLAGS_WRITE_THROUGH),
                         0L, TRUE );
      if ( usDosRc )
      {
        pExportIDA->hFile = NULLHANDLE;
        fOK = FALSE;
        if ( pExportIDA->pMem != NULL)
        {
          MemoryFactory *pFactory = MemoryFactory::getInstance();
          pFactory->closeMemory( pExportIDA->pMem );
          pExportIDA->pMem = NULL;
        } /* endif */
      } /* endif */
    } /* endif */

    if ( !fOK )
    {
      // stop processing
      pExportIDA->pszActiveName[0] = EOS;
    } /* endif */
  } /* endif */

  if ( pExportIDA->pszNameList && pExportIDA->pszActiveName[0] )
  {
    // GQTODO update process window title bar and outfile static
    LOADSTRING( NULLHANDLE, hResMod, MEM_EPLINE2, pCommArea->szBuffer );
    sprintf( pCommArea->szText, "%s %s", pCommArea->szBuffer, pExportIDA->ControlsIda.szPathContent );
    LOADSTRING( NULLHANDLE, hResMod, MEM_EPTITLE, pCommArea->szBuffer );
    sprintf( pCommArea->szTitle, "%s %s", pCommArea->szBuffer, pExportIDA->szMemName );

    // restart TM export
    WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_START_EXPORT ), 0L );
  }
  else
  {
    //--- Free data areas
    if ( pExportIDA->pszNameList ) UtlAlloc( (PVOID *)&pExportIDA->pszNameList, 0L, 0L, NOMSG );

    if ( (hwndErrMsg == HWND_FUNCIF) && pExportIDA->fDataCorrupted )
    {
      PSZ pszParm = "ANSI/ASCII";
      UtlErrorHwnd( ERROR_MEM_EXPORT_DATACORRUPT, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      usRC = ERROR_MEM_EXPORT_DATACORRUPT;
    } /* endif */

    if ( pExportIDA->pProposal != NULL ) free(pExportIDA->pProposal); 
    UtlAlloc( (PVOID *) &pExportIDA, 0L, 0L, NOMSG );
    pCommArea->pUserIDA = NULL;

    // Dismiss the slider window if it had been created
    if ( hwndErrMsg != HWND_FUNCIF )
    {
      EqfRemoveObject( TWBFORCE, hWnd );
    } /* endif */
  }

  return( usRC );
} /* end of function EQFMemExportEnd */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MemExportStart                                           |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT  MemExportStart( HWND hWnd, MPARAM mp2 )          |
//+----------------------------------------------------------------------------+
//|Description:       Initialize the memory database export process            |
//+----------------------------------------------------------------------------+
//|Parameters:        hWnd  dialog handle                                      |
//|                   mp2   pointer to export IDA                              |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE initalization fails. process should be terminated  |
//|                   process handle if initialization was OK                  |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//| get access to export IDA                                                   |
//|                                                                            |
//| if ( no error )                                                            |
//| {                                                                          |
//|    allocate storage for export out and export in structures                |
//|    EXT_OUT, EXT_IN                                                         |
//| }                                                                          |
//|                                                                            |
//| if ( no error from allocation )                                            |
//| {                                                                          |
//|    call function MemCreateProcess to get a process handle                  |
//|                                                                            |
//|    if ( no process handle is returned )                                    |
//|      set return code to FALSE                                              |
//| }                                                                          |
//|                                                                            |
//| if ( no error until now )                                                  |
//|    initialize slider                                                       |
//|                                                                            |
//| if ( error occurs somewhere )                                              |
//| {                                                                          |
//|    do cleanup and display error message                                    |
//| }                                                                          |
//|                                                                            |
//| return;                                                                    |
//+----------------------------------------------------------------------------+
USHORT  MemExportStart( PPROCESSCOMMAREA  pCommArea,
                        HWND              hWnd )
{
   USHORT           usRc = TRUE;          // Process control switch
   PMEM_EXPORT_IDA  pExportIDA;           // Pointer to the export IDA
   PSZ              pReplAddr[2];         // Pointer to an address list of
                                          // replacement strings

   hWnd;                                  // get rid of compiler warnings

   //--- Get pointer to export IDA
   pExportIDA = (PMEM_EXPORT_IDA)pCommArea->pUserIDA;

   // set first extract flag                                          
   pExportIDA->fFirstExtract = TRUE;

   if ( (pExportIDA->usExpMode == MEM_FORMAT_TMX) || (pExportIDA->usExpMode == MEM_FORMAT_TMX_UTF8) ||
        (pExportIDA->usExpMode == MEM_FORMAT_TMX_NOCRLF) || (pExportIDA->usExpMode == MEM_FORMAT_TMX_UTF8_NOCRLF) )
   {
     // allocate memory and segment data area
     if ( !UtlAlloc( (PVOID *)&pExportIDA->pstMemInfo, 0L, sizeof(MEMEXPIMPINFO), ERROR_STORAGE ) )
     {
       usRc = ERROR_NOT_ENOUGH_MEMORY;
     }
     else if ( !UtlAlloc( (PVOID *)&pExportIDA->pstSegment, 0L, sizeof(MEMEXPIMPSEG), ERROR_STORAGE ) )
     {
       usRc = ERROR_NOT_ENOUGH_MEMORY;
     } /* endif */

     // load external memory export module
     usRc = DosLoadModule( NULL, 0, "OTMTMXIE", &(pExportIDA->hmodMemExtExport) );

     // load entry points
     if ( !usRc )
     {
       usRc = DosGetProcAddr( pExportIDA->hmodMemExtExport, "EXTMEMEXPORTSTART", 
                             (PFN*)(&(pExportIDA->pfnMemExpStart)));
       if ( !usRc ) usRc = DosGetProcAddr( pExportIDA->hmodMemExtExport, "EXTMEMEXPORTPROCESS", 
                                           (PFN*)(&(pExportIDA->pfnMemExpProcess)));
       if ( !usRc ) usRc = DosGetProcAddr( pExportIDA->hmodMemExtExport, "EXTMEMEXPORTEND", 
                                           (PFN*)(&(pExportIDA->pfnMemExpEnd)));
     } /* endif */

     // call start entry point
     if ( !usRc )
     {
       // close output file (if open) as external export process will open the file itself...
       if ( pExportIDA->hFile ) 
       {
         UtlClose( pExportIDA->hFile, FALSE );
         pExportIDA->hFile = NULL; 
       } /* endif */

       memset( pExportIDA->pstMemInfo, 0, sizeof(MEMEXPIMPINFO) );

       // get name of first markup table
       if ( pExportIDA->pMem->getNumOfMarkupNames() >= 1 )
       {
         pExportIDA->pMem->getMarkupName( 0, pExportIDA->pstMemInfo->szFormat, sizeof(pExportIDA->pstMemInfo->szFormat) );
       } /* end */            

       strcpy( pExportIDA->pstMemInfo->szName, pExportIDA->szMemName );
       pExportIDA->pMem->getDescription( pExportIDA->pstMemInfo->szDescription, sizeof(pExportIDA->pstMemInfo->szDescription) );
       pExportIDA->pMem->getSourceLanguage( pExportIDA->pstMemInfo->szSourceLang, sizeof(pExportIDA->pstMemInfo->szSourceLang) );
       pExportIDA->pstMemInfo->fUTF16 = (pExportIDA->usExpMode == MEM_FORMAT_TMX) || (pExportIDA->usExpMode == MEM_FORMAT_TMX_NOCRLF) ;
       pExportIDA->pstMemInfo->fNoCRLF = (pExportIDA->usExpMode == MEM_FORMAT_TMX_NOCRLF) || (pExportIDA->usExpMode == MEM_FORMAT_TMX_UTF8_NOCRLF);

       usRc = pExportIDA->pfnMemExpStart( &(pExportIDA->lExternalExportHandle),  
                                           pExportIDA->ControlsIda.szPathContent,
                                           pExportIDA->pstMemInfo );
     } /* endif */

     // memory export uses usRc as an OK flag , so we have to remap our return code...
     usRc = ( !usRc ) ? TRUE : FALSE;
   }
   else
   {
    // write UNICODE prefix when exporting in Unicode mode
    if ( pExportIDA->usExpMode == MEM_SGMLFORMAT_UNICODE )
    {
      ULONG ulWritten;
      UtlWriteL( pExportIDA->hFile, UNICODEFILEPREFIX, (USHORT)strlen(UNICODEFILEPREFIX), &ulWritten, FALSE );
    } /* endif */

    if ( !usRc )
    {
      //--- Clean the storage allocations and the process handle
      //--- if the MemExportStart function failed.
      if ( pExportIDA != NULL )
      {
        //--- Issue the error message :"Initialization of export from
        //--- translation memory %1 into file %2 failed."
        OEMTOANSI( pExportIDA->szMemName );
        pReplAddr[0] = pExportIDA->szMemName;
        pReplAddr[1] = pExportIDA->ControlsIda.szPathContent;
        UtlErrorHwnd( ERROR_MEM_EXPORT_INITFAILED, MB_CANCEL, 2,
                      &pReplAddr[0], EQF_ERROR, pExportIDA->hwndErrMsg );
        ANSITOOEM( pExportIDA->szMemName );
      } /* endif */
    } /* endif */
   } /* endif */

   return usRc;
} /* end of function MemExportStart */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MemExportProcess                                         |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT MemExportProcess ( PMEM_EXPORT_IDA  pExportIDA )  |
//+----------------------------------------------------------------------------+
//|Description:       Export a segment from a TM                               |
//|                   Read a segment from the database and write it to the     |
//|                   export file                                              |
//+----------------------------------------------------------------------------+
//|Parameters:        pExportIDA pointer to export IDA                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       MEM_PROCESS_OK   OK, no error                            |
//|                   MEM_DB_ERROR     The memory database is in error         |
//|                                    the process should be stopped           |
//|                   MEM_WRITE_ERR    Export write error (probably disk full) |
//|                   MEM_PROCESS_END  No more segments to export              |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//| initialize input parameters for the TM extract                             |
//|                                                                            |
//| call function TmExtract                                                    |
//|                                                                            |
//| if ( no error from TmExtract )                                             |
//| {                                                                          |
//|    if ( cluster number is > stop cluster )                                 |
//|    {                                                                       |
//|       set EOF flag                                                         |
//|                                                                            |
//|       call function MemExportWriteFile to write end of memory tag          |
//|    }                                                                       |
//|    else                                                                    |
//|    {                                                                       |
//|       increase segment counter                                             |
//|                                                                            |
//|       call function MemExportWriteFile to write segment to file            |
//|                                                                            |
//|    }                                                                       |
//| }                                                                          |
//| else  //--- error from TmExtract                                           |
//| {                                                                          |
//|   if ( error from TmExtract is no segment found )                          |
//|   {                                                                        |
//|      set EOF flag                                                          |
//|                                                                            |
//|      call function MemExportWriteFile to write end of memory tag           |
//|   }                                                                        |
//|   else                                                                     |
//|      set rc to error                                                       |
//| }                                                                          |
//| return rc                                                                  |
//+----------------------------------------------------------------------------+
USHORT MemExportProcess ( PMEM_EXPORT_IDA  pExportIDA ) // pointer to the export
                                                        // data area
{
   USHORT        usRc = MEM_PROCESS_OK; // Return code to calling function
   int           iTmRC;               // Tm return code

   if ( pExportIDA->fFirstExtract )
   {
     iTmRC = pExportIDA->pMem->getFirstProposal( *(pExportIDA->pProposal), &(pExportIDA->iComplete) );
   }
   else
   {
     iTmRC = pExportIDA->pMem->getNextProposal( *(pExportIDA->pProposal), &(pExportIDA->iComplete) );
   } /* endif */

   if ( (iTmRC == NO_ERROR) || iTmRC == OtmMemory::INFO_ENDREACHED )
   {
     if ( iTmRC == OtmMemory::INFO_ENDREACHED )
     {
       //--- Stop Address has been reached before END_OF_TM
       pExportIDA->fEOF = TRUE;
       //--- Call the following function to write the end of memory tag
       if ( (pExportIDA->usExpMode == MEM_FORMAT_TMX) || (pExportIDA->usExpMode == MEM_FORMAT_TMX_UTF8) ||
            (pExportIDA->usExpMode == MEM_FORMAT_TMX_NOCRLF) || (pExportIDA->usExpMode == MEM_FORMAT_TMX_UTF8_NOCRLF) )
       {
         usRc = MEM_PROCESS_END;
       }
       else
       {
         usRc = MemExportWriteFile( pExportIDA );
       } /* endif */

     }
     else
     {
       //--- Increase the segment counter by one
       pExportIDA->ulSegmentCounter++;

       //--- Prepare the segment and write it to the export file
       if ( (pExportIDA->usExpMode == MEM_FORMAT_TMX) || (pExportIDA->usExpMode == MEM_FORMAT_TMX_UTF8) ||
            (pExportIDA->usExpMode == MEM_FORMAT_TMX_NOCRLF) || (pExportIDA->usExpMode == MEM_FORMAT_TMX_UTF8_NOCRLF) )
       {
         // reset first extract flag (in non-TMX mode this is done in MemExportWriteFile)
         pExportIDA->fFirstExtract = FALSE;                 

         memset( pExportIDA->pstSegment, 0, sizeof(MEMEXPIMPSEG) );

         pExportIDA->pstSegment->fValid = TRUE;
         pExportIDA->pstSegment->usTranslationFlag = 0;
         if ( pExportIDA->pProposal->getType() == OtmProposal::eptMachine ) pExportIDA->pstSegment->usTranslationFlag = 1;
         if ( pExportIDA->pProposal->getType() == OtmProposal::eptGlobalMemory ) pExportIDA->pstSegment->usTranslationFlag = 2;

         pExportIDA->pstSegment->lSegNum = pExportIDA->pProposal->getSegmentNum();
         pExportIDA->pstSegment->lTime = pExportIDA->pProposal->getUpdateTime();
         pExportIDA->pMem->getSourceLanguage( pExportIDA->pstSegment->szSourceLang, sizeof(pExportIDA->pstSegment->szSourceLang) );
         pExportIDA->pProposal->getTargetLanguage( pExportIDA->pstSegment->szTargetLang, sizeof(pExportIDA->pstSegment->szTargetLang) );
         pExportIDA->pProposal->getAuthor( pExportIDA->pstSegment->szAuthor, sizeof(pExportIDA->pstSegment->szAuthor) );
         pExportIDA->pProposal->getMarkup( pExportIDA->pstSegment->szFormat, sizeof(pExportIDA->pstSegment->szFormat) );
         pExportIDA->pProposal->getDocName( pExportIDA->pstSegment->szDocument, sizeof(pExportIDA->pstSegment->szDocument) );
         OEMTOANSI( pExportIDA->pstSegment->szDocument );
         pExportIDA->pProposal->getSource( pExportIDA->pstSegment->szSource, sizeof(pExportIDA->pstSegment->szSource) / sizeof(CHAR_W) );
         pExportIDA->pProposal->getTarget( pExportIDA->pstSegment->szTarget, sizeof(pExportIDA->pstSegment->szTarget) / sizeof(CHAR_W) );
         pExportIDA->pProposal->getContext( pExportIDA->pstSegment->szContext, sizeof(pExportIDA->pstSegment->szContext) / sizeof(CHAR_W) );
         pExportIDA->pProposal->getAddInfo( pExportIDA->pstSegment->szAddInfo, sizeof(pExportIDA->pstSegment->szAddInfo) / sizeof(CHAR_W) );

         usRc = pExportIDA->pfnMemExpProcess( pExportIDA->lExternalExportHandle, pExportIDA->pstSegment );

         // map return code to the ones used by memory export...
         switch ( usRc )
         {
           case 0 :
             usRc = MEM_PROCESS_OK;
             break;
           default:
              usRc = MEM_WRITE_ERR;
              break;
         } /*endswitch */
       }
       else
       {
         usRc = MemExportWriteFile( pExportIDA );
       } /* endif */
     } /* endif */
   }
   else  //--- An error occured in the extract call.
   {
       //--- An unrecoverable memory DB error occurred
       MemoryFactory *pFactory = MemoryFactory::getInstance();
       usRc = MEM_DB_ERROR;
       pFactory->showLastError( pExportIDA->szPlugin, pExportIDA->szMemName, pExportIDA->pMem, pExportIDA->hwndErrMsg );
       if ( pExportIDA->fBatch )
       {
         pExportIDA->usRC = UtlGetDDEErrorCode( pExportIDA->hwndErrMsg );
       } /* endif */
       if ( pExportIDA->pDDEMemExp != NULL )
       {
         pExportIDA->pDDEMemExp->DDEReturn.usRc =
             UtlGetDDEErrorCode( pExportIDA->hwndErrMsg );
       } /* endif */
   } /* endif */

   return usRc;
} /* end of function MemExportProcess  */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MemExportWriteFile                                       |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT MemExportWriteFile ( PMEM_EXPORT_IDA  pExportIDA )|
//+----------------------------------------------------------------------------+
//|Description:       prepare a segment and write it to the export file        |
//+----------------------------------------------------------------------------+
//|Parameters:        pExportIDA  pointer to export IDA                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//| MEM_PROCESS_OK   OK, no error                                              |
//| MEM_WRITE_ERR    Export write error (probably disk full)                   |
//| MEM_PROCESS_END  last segment and end of memory tag has been written       |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//| initialize variables                                                       |
//|                                                                            |
//| if ( no error )                                                            |
//| {                                                                          |
//|    if ( first segment to write )                                           |
//|       do handling for TM begin tag and write it to file                    |
//| }                                                                          |
//| if ( no error during write to file )                                       |
//| {                                                                          |
//|    if ( end of TM has been reached )                                       |
//|    {                                                                       |
//|       do handling for TM end tag and write it to file                      |
//|                                                                            |
//|       if ( no error during write )                                         |
//|          set rc to MEM_PROCESS_END                                         |
//|    }                                                                       |
//|    #ifdef MEMAN                                                            |
//|       do handling to create files for TM analysis                          |
//|    #endif                                                                  |
//|  }                                                                         |
//|  if ( no error during write )                                              |
//|     do handling for segment begin tag and write it to file                 |
//|                                                                            |
//|  if ( no error during write )                                              |
//|     do handling for segment ID and write it to file                        |
//|                                                                            |
//|  if ( no error during write )                                              |
//|     do handling for control informations and write them to file            |
//|                                                                            |
//|  if ( no error during write )                                              |
//|     do handling for source begin tag and write it to file                  |
//|                                                                            |
//|  if ( no error during write )                                              |
//|     write source segment to file                                           |
//|                                                                            |
//|    #ifdef MEMAN                                                            |
//|       do handling to create files for TM analysis                          |
//|    #endif                                                                  |
//|                                                                            |
//|  if ( no error during write )                                              |
//|     do handling for source end tag and write it to file                    |
//|                                                                            |
//|  if ( no error during write )                                              |
//|     do handling for target begin tag and write it to file                  |
//|                                                                            |
//|  if ( no error during write )                                              |
//|     write target segment to file                                           |
//|                                                                            |
//|  if ( no error during write )                                              |
//|     do handling for target end tag and write it to file                    |
//|                                                                            |
//|  if ( no error during write )                                              |
//|     do handling for segment end tag and write it to file                   |
//|                                                                            |
//|  return rc                                                                 |
//+----------------------------------------------------------------------------+
USHORT MemExportWriteFile ( PMEM_EXPORT_IDA  pExportIDA ) // pointer to the
                                                          // export data area
{
   USHORT    usRc = MEM_PROCESS_OK;   // Return code to calling function
   PSZ_W     pszWorkArea;             // Pointer to pExportIDA->szWorkArea
   CHAR_W        chMTFlag;

   /*******************************************************************/
   /* get pointer to TM_EXT structure and                             */
   /* to work area in EXPORT_IDA                                      */
   /*******************************************************************/
   pszWorkArea = pExportIDA->szWorkArea;

   /*******************************************************************/
   /* If this is the first segment to write, write first              */
   /* the memory database begin tag                                   */
   /*******************************************************************/
   if ( usRc == MEM_PROCESS_OK )
   {
     if ( pExportIDA->fFirstExtract )
     {
       pExportIDA->fFirstExtract = FALSE;

       UTF16strcpy( pszWorkArea, NTM_BEGIN_TAGW );

       usRc = MemExportWrite( pExportIDA, pszWorkArea );

      /*****************************************************************/
      /* write the description tag                                     */
      /*****************************************************************/
      if ( usRc == MEM_PROCESS_OK )
      {
        CHAR    szDescription[MAX_MEM_DESCRIPTION];   //description of memory database
        CHAR_W  szDescriptionW[MAX_MEM_DESCRIPTION];   //description of memory database


        UTF16strcpy( pszWorkArea, NTM_DESCRIPTION_BEGIN_TAGW );
        pExportIDA->pMem->getDescription( szDescription, sizeof(szDescription) );
        ASCII2Unicode( szDescription, szDescriptionW, pExportIDA->ulOemCP );
        UTF16strcat( pszWorkArea, szDescriptionW );
        UTF16strcat( pszWorkArea, NTM_DESCRIPTION_END_TAGW );
        usRc = MemExportWrite( pExportIDA, pszWorkArea );
      } /* endif */

      // write the codepage flag
      if ( usRc == MEM_PROCESS_OK )
      {
        UTF16strcpy( pszWorkArea, NTM_CODEPAGE_BEGIN_TAGW );
        switch ( pExportIDA->usExpMode )
        { 
          case MEM_SGMLFORMAT_ANSI:
            {
              SHORT sCP = (SHORT)GetOrgLangCP( NULL, FALSE );
              swprintf( pszWorkArea + wcslen(pszWorkArea), NTM_CODEPAGE_ANSI_VALUE, sCP );
            }
            break;

          case MEM_SGMLFORMAT_UNICODE:  
            UTF16strcat( pszWorkArea, NTM_CODEPAGE_UTF16_VALUE );
            break;

          case MEM_SGMLFORMAT_ASCII:
          default:

            {
              SHORT sCP = (SHORT)GetOrgLangCP( NULL, TRUE );
              swprintf( pszWorkArea + wcslen(pszWorkArea), NTM_CODEPAGE_ASCII_VALUE, sCP );
            }
            break;
        } /*endswitch */
        UTF16strcat( pszWorkArea, NTM_CODEPAGE_END_TAGW );
        usRc = MemExportWrite( pExportIDA, pszWorkArea );
      } /* endif */


     } /* endif */
   } /* endif */

   //--------------------------------------------------------------------------
   //--- If end of memory database has been reached
   //--- write end of memory tag
   if ( usRc == MEM_PROCESS_OK )
   {
     if ( pExportIDA->fEOF )
     {
       UTF16strcpy( pszWorkArea, NTM_END_TAGW );

       usRc = MemExportWrite( pExportIDA, pszWorkArea );
       if ( usRc == MEM_PROCESS_OK )
       {
            usRc = MEM_PROCESS_END;
       } /* endif */
     } /* endif */
   } /* endif */

   //--------------------------------------------------------------------------
   //--- Write Segment Begin Tag
   if ( usRc == MEM_PROCESS_OK )
   {
     UTF16strcpy( pszWorkArea, MEM_SEGMENT_BEGIN_TAGW );
     usRc = MemExportWrite( pExportIDA, pszWorkArea );
   } /* endif */

   //--------------------------------------------------------------------------
   //--- Write SegmentID
   if ( usRc == MEM_PROCESS_OK )
   {
     swprintf( pszWorkArea, L"%010ld\r\n", pExportIDA->ulSegmentCounter );
     usRc = MemExportWrite( pExportIDA, pszWorkArea );
   } /* endif */

   //--------------------------------------------------------------------------
   //--- Write Control Information
   if ( usRc == MEM_PROCESS_OK )
   {
      CHAR      szSourceLanguage[MAX_LANG_LENGTH]; //language name of source
      CHAR      szTargetLanguage[MAX_LANG_LENGTH]; //language name of target
      CHAR      szAuthorName[MAX_USERID];          //author name of target
      CHAR      szFileName[MAX_FILESPEC];          //where source comes from name+ext
      LONG_FN   szLongName;                        // name of source file (long name or EOS)
      CHAR      szTagTable[MAX_FNAME];             //tag table name
     
     /*****************************************************************/
     /* Prepare the MT flag                                           */
     /*****************************************************************/
     if ( pExportIDA->pProposal->getType() == OtmProposal::eptMachine )
     {
       chMTFlag = '1';
     }
     else if ( pExportIDA->pProposal->getType() == OtmProposal::eptGlobalMemory )
     {
       chMTFlag = '2';
     }
     else
     {
       chMTFlag = '0';
     } /* endif */

     /*****************************************************************/
     /* prepare and write the control tag                             */
     /*****************************************************************/
     pExportIDA->pProposal->getSourceLanguage( szSourceLanguage, sizeof(szSourceLanguage) );
     pExportIDA->pProposal->getTargetLanguage( szTargetLanguage, sizeof(szTargetLanguage) );
     pExportIDA->pProposal->getAuthor( szAuthorName, sizeof(szAuthorName) );
     pExportIDA->pProposal->getDocShortName( szFileName, sizeof(szFileName) );
     pExportIDA->pProposal->getDocName( szLongName, sizeof(szLongName) );
     pExportIDA->pProposal->getMarkup( szTagTable, sizeof(szTagTable) );
     sprintf( pExportIDA->szConvArea,
              "%s%06lu%s%c%s%016lu%s%s%s%s%s%s%s%s%s%s%s%s%s",
             //| |    | | | |     | | | | | | | | | | | | +- </Control>
             //| |    | | | |     | | | | | | | | | | | +- long file name
             //| |    | | | |     | | | | | | | | | | +--- X15
             //| |    | | | |     | | | | | | | | | +----- file name
             //| |    | | | |     | | | | | | | | +------- X15
             //| |    | | | |     | | | | | | | +--------- tag table
             //| |    | | | |     | | | | | | +----------- X15
             //| |    | | | |     | | | | | +------------- author
             //| |    | | | |     | | | | +--------------- X15
             //| |    | | | |     | | | +----------------- target lng
             //| |    | | | |     | | +------------------- X15
             //| |    | | | |     | +--------------------- source lng
             //| |    | | | |     +----------------------- X15
             //| |    | | | +----------------------------- time
             //| |    | | +------------------------------- X15
             //| |    | +--------------------------------- mt flag
             //| |    +----------------------------------- X15
             //| +---------------------------------------- seg id
             //+------------------------------------------ <Control>
              MEM_CONTROL_BEGIN_TAG,
              pExportIDA->pProposal->getSegmentNum(),
              X15_STR,
              chMTFlag,
              X15_STR,
              pExportIDA->pProposal->getUpdateTime(),
              X15_STR,
              szSourceLanguage,
              X15_STR,
              szTargetLanguage,
              X15_STR,
              szAuthorName,
              X15_STR,
              szTagTable,
              X15_STR,
              szFileName,
              X15_STR,
              szLongName,
              MEM_CONTROL_END_TAG );
     ASCII2Unicode( pExportIDA->szConvArea, pExportIDA->szWorkArea, pExportIDA->ulOemCP );
     usRc = MemExportWrite( pExportIDA, pExportIDA->szWorkArea );
   } /* endif */

   // write any segment context
#ifdef EXPORT_CONTEXT_AND_ADDINFO
   if ( pExportIDA->pProposal->getContextLen() != 0 )
   {
     pExportIDA->pProposal->getContext( pExportIDA->szSegmentBuffer, sizeof(pExportIDA->szSegmentBuffer) );
     if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, MEM_CONTEXT_BEGIN_TAGW );
     if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, pExportIDA->szSegmentBuffer );
     if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, MEM_CONTEXT_END_TAGW );
   } /* endif */      

   // write any segment additional information
   if ( pExportIDA->pProposal->getAddInfoLen() != 0 )
   {
     pExportIDA->pProposal->getAddInfo( pExportIDA->szSegmentBuffer, sizeof(pExportIDA->szSegmentBuffer) );
     if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, MEM_ADDINFO_BEGIN_TAGW );
     if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, pExportIDA->szSegmentBuffer );
     if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, MEM_ADDINFO_END_TAGW );
   } /* endif */      
#endif

   // write segment source
   pExportIDA->pProposal->getSource( pExportIDA->szSegmentBuffer, sizeof(pExportIDA->szSegmentBuffer) );
   if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, MEM_SOURCE_BEGIN_TAGW );
   if ( usRc == MEM_PROCESS_OK ) usRc = MemExportPrepareSegment( pExportIDA, pExportIDA->szSegmentBuffer, pExportIDA->szSegmentBuffer2  );
   if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, pExportIDA->szSegmentBuffer2 );
   if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, MEM_SOURCE_END_TAGW );

   // write segment target
   pExportIDA->pProposal->getTarget( pExportIDA->szSegmentBuffer, sizeof(pExportIDA->szSegmentBuffer) );
   if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, MEM_TARGET_BEGIN_TAGW  );
   if ( usRc == MEM_PROCESS_OK ) usRc = MemExportPrepareSegment( pExportIDA, pExportIDA->szSegmentBuffer, pExportIDA->szSegmentBuffer2  );
   if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, pExportIDA->szSegmentBuffer2 );
   if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, MEM_TARGET_END_TAGW );


   //--- Write Segment end tag
   if ( usRc == MEM_PROCESS_OK ) usRc = MemExportWrite( pExportIDA, MEM_SEGMENT_END_TAGW );

  return usRc;
} /* end of function MemExportWriteFile */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MemExportWrite                                           |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT MemExportWrite( PMEM_EXPORT_IDA   pExportIDA,     |
//|                                          PSZ               pszToWrite  )   |
//+----------------------------------------------------------------------------+
//|Description:       Write an element to the export file                      |
//|                   This function writes an element to an export file,       |
//|                   handles errors and issues error messages if required.    |
//+----------------------------------------------------------------------------+
//|Parameters:        pExportIDA  Pointer to the export IDA                    |
//|                   pszToWrite  Pointer to string which has to be written    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//| MEM_PROCESS_OK  if the write was OK and all bytes have been written        |
//| MEM_WRITE_ERR   if an unrecoverable write error occurred                   |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//| if ( string fits completely intobuffer )                                   |
//| {                                                                          |
//|    copy string to buffer                                                   |
//|                                                                            |
//|    increase buffer counter                                                 |
//| }                                                                          |
//| else                                                                       |
//| {                                                                          |
//|    copy part to buffer which fits into the buffer                          |
//|                                                                            |
//|    write the buffer to file                                                |
//|                                                                            |
//|    if ( no error during write )                                            |
//|    {                                                                       |
//|       if ( not all bytes are written to file )                             |
//|          close file, display error message and set error indicator         |
//|       else                                                                 |
//|          clear buffer and copy rest of string to buffer                    |
//|    }                                                                       |
//|    else                                                                    |
//|       set error indicator                                                  |
//| }                                                                          |
//| if ( buffer has to be written )                                            |
//| {                                                                          |
//|    write buffer                                                            |
//|                                                                            |
//|    if ( no error during write )                                            |
//|    {                                                                       |
//|       if ( not all bytes are written )                                     |
//|          do cleanup, display error message and set error indicator         |
//|       else                                                                 |
//|         calculate number of bytes written and clear buffer                 |
//|     }                                                                      |
//|     else                                                                   |
//|        set error indicator                                                 |
//|  }                                                                         |
//|  return rc;                                                                |
//+----------------------------------------------------------------------------+
USHORT MemExportWrite
 (
    PMEM_EXPORT_IDA   pExportIDA,  // Pointer to the export IDA
    PSZ_W             pszToWrite   // Pointer to string which has to be written
 )
{
   USHORT        usRc =  MEM_PROCESS_OK; // Function return code
   USHORT        usDosRc;                // Dos function return code
   ULONG        ulBytesToWrite;         // Number of bytes to write
   ULONG        ulBytesWritten;         // Number of bytes written
   ULONG        ulFillBytes;            // Number of bytes to fill buffer
   PSZ           pReplString[2];                 // Array of pointers to replacement strings
   CHAR          szDrive[MAX_DRIVE];     //drive name x:EOS
   PBYTE         pBuffer = (PBYTE)pszToWrite;

   //--- Calculate length which should be written
   ulBytesToWrite = UTF16strlenBYTE( pszToWrite );

   // handle different formats
   switch ( pExportIDA->usExpMode )
   {
     case MEM_SGMLFORMAT_ANSI:
       { 
         LONG lRc = NO_ERROR;
         ULONG ulOrgChars = ulBytesToWrite / sizeof(CHAR_W);
         ulBytesToWrite = UtlDirectUnicode2AnsiBuf( pszToWrite, pExportIDA->szConvArea,
                                              (ulBytesToWrite / sizeof(CHAR_W)),
                                              sizeof(pExportIDA->szConvArea),
                                              pExportIDA->ulAnsiCP, TRUE, &lRc );
         if (lRc)
         {
		       usRc = (USHORT) lRc;
	       }

         // convert back to unicode and verify that no info is lost
         if ( (usRc == MEM_PROCESS_OK) && !pExportIDA->fDataCorrupted )
         {
           LONG lBytesLeft = 0;
           ULONG ulUnicodeChars = 0;

		       pExportIDA->szConvArea[ulBytesToWrite] = EOS;
           ulUnicodeChars = UtlDirectAnsi2UnicodeBuf( pExportIDA->szConvArea, pExportIDA->szBackConvArea,
					                                     				ulBytesToWrite, pExportIDA->ulAnsiCP, FALSE, &lRc, &lBytesLeft );
            if (lRc)
            {
		          usRc = (USHORT) lRc;
	          }
            else
            {
              // compare data
              if ( (ulOrgChars != ulUnicodeChars) || (wcsncmp( pszToWrite, pExportIDA->szBackConvArea, ulUnicodeChars ) != 0 ) )
              {
                if ( pExportIDA->hwndErrMsg != HWND_FUNCIF )
                {
                  PSZ pszParm = "ANSI";
                  USHORT usMBCode = UtlErrorHwnd( ERROR_MEM_EXPORT_DATACORRUPT, MB_OKCANCEL, 1, &pszParm, EQF_QUERY, pExportIDA->hwndErrMsg );
                  if ( usMBCode == MBID_CANCEL )
                  {
                    usRc = MEM_WRITE_ERR;
                  } /* endif */
                } /* endif */
                pExportIDA->fDataCorrupted = TRUE;
              } /* endif */
            } /* endif */
         } /* endif */
       }
       pExportIDA->szConvArea[ulBytesToWrite] = EOS;
       pBuffer = (PBYTE)pExportIDA->szConvArea;
       break;

     case MEM_SGMLFORMAT_UNICODE:   // nothing to do...
       break;

     case MEM_SGMLFORMAT_ASCII:
     default:
       {
         ULONG ulOrgChars = ulBytesToWrite / sizeof(CHAR_W);
         ulBytesToWrite = Unicode2ASCIIBuf( pszToWrite, pExportIDA->szConvArea, (ulBytesToWrite / sizeof(CHAR_W)),
                                              sizeof(pExportIDA->szConvArea), pExportIDA->ulOemCP );

         // convert back to unicode and verify that no info is lost
         if ( (usRc == MEM_PROCESS_OK) && !pExportIDA->fDataCorrupted )
         {
           ULONG ulUnicodeChars = 0;

		       pExportIDA->szConvArea[ulBytesToWrite] = EOS;
           ulUnicodeChars = ASCII2UnicodeBuf( pExportIDA->szConvArea, pExportIDA->szBackConvArea, ulBytesToWrite, pExportIDA->ulOemCP );

           // compare data
           if ( (ulOrgChars != ulUnicodeChars) || (wcsncmp( pszToWrite, pExportIDA->szBackConvArea, ulUnicodeChars ) != 0 ) )
           {
             if ( pExportIDA->hwndErrMsg != HWND_FUNCIF )
            {
               PSZ pszParm = "ASCII";
               USHORT usMBCode = UtlErrorHwnd( ERROR_MEM_EXPORT_DATACORRUPT, MB_OKCANCEL, 1, &pszParm, EQF_QUERY, pExportIDA->hwndErrMsg );
               if ( usMBCode == MBID_CANCEL )
               {
                 usRc = MEM_WRITE_ERR;
               } /* endif */
             } /* endif */
             pExportIDA->fDataCorrupted = TRUE;
           } /* endif */
         } /* endif */

         pExportIDA->szConvArea[ulBytesToWrite] = EOS;
         pBuffer = (PBYTE)pExportIDA->szConvArea;
       }
       break;
   }/*endswitch */


   //--- Now check if the string still fits completely in the buffer
   if ( ( ulBytesToWrite + pExportIDA->ulBytesInBuffer )
        <= MEM_EXPORT_OUT_BUFFER )
   {
     //--- string fits completely
     //--- copy the string to the buffer
     memcpy( &(pExportIDA->acWriteBuffer[pExportIDA->ulBytesInBuffer]),
             pBuffer, ulBytesToWrite );

     //--- increase the buffer counter by the bytes to write
     pExportIDA->ulBytesInBuffer += ulBytesToWrite;
   } /* endif */
   else
   {
     //--- string doesn't fit completely into the buffer
     //--- copy only the part to the buffer that fits
     ulFillBytes = MEM_EXPORT_OUT_BUFFER - pExportIDA->ulBytesInBuffer;
     memcpy( &(pExportIDA->acWriteBuffer[pExportIDA->ulBytesInBuffer]),
             pBuffer, ulFillBytes );

      //--- Now write the buffer because it is full
      usDosRc = UtlWriteHwnd( pExportIDA->hFile, pExportIDA->acWriteBuffer, MEM_EXPORT_OUT_BUFFER,
                              &ulBytesWritten, TRUE, pExportIDA->hwndErrMsg );


      if ( usDosRc == NO_ERROR )
      {
        //--- Check whether all bytes have been written
        if ( ulBytesWritten != MEM_EXPORT_OUT_BUFFER )
        {
           //--- Close the exported file because
           //--- Disk full condition reached
           memcpy( szDrive, pExportIDA->ControlsIda.szPathContent, 2 );
           szDrive[2] = NULC;
           pReplString[0] = szDrive;
           CloseFile( &(pExportIDA->hFile) );
           UtlErrorHwnd( ERROR_WRITING, MB_CANCEL, 1,
                     &(pReplString[0]), EQF_ERROR,
                     pExportIDA->hwndErrMsg);
           usRc = MEM_WRITE_ERR;
        }
        else
        {
          //--- All bytes have been written
          //--- increase the number of total bytes
          pExportIDA->ulTotalBytesWritten += (ULONG)MEM_EXPORT_OUT_BUFFER;

          //--- Clear the buffer and copy the rest of the string in the buffer
          memset( pExportIDA->acWriteBuffer, NULC, MEM_EXPORT_OUT_BUFFER );
          pExportIDA->ulBytesInBuffer = 0;
          memcpy( &(pExportIDA->acWriteBuffer[pExportIDA->ulBytesInBuffer]),
                  &(pBuffer[ulFillBytes]),
                  ulBytesToWrite - ulFillBytes );

          //--- increase the buffer counter by the bytes to write
          pExportIDA->ulBytesInBuffer += ( ulBytesToWrite - ulFillBytes );
        } /* endif */
      }
      else
      {
        //--- an error during the writing occurred
        usRc = MEM_WRITE_ERR;
      } /* endif */
   } /* endelse */

   //--- check if buffer has to be written ( either because it is full or
   //--- because the last thing has to be written )
   //--- and the returncode from above is still ok
   if ( ( ( pExportIDA->fEOF == TRUE ) ||
          ( pExportIDA->ulBytesInBuffer == MEM_EXPORT_OUT_BUFFER ) ) &&
        ( usRc == MEM_PROCESS_OK ) )
   {
     usDosRc = UtlWriteHwnd( pExportIDA->hFile, pExportIDA->acWriteBuffer, pExportIDA->ulBytesInBuffer,
                              &ulBytesWritten, TRUE, pExportIDA->hwndErrMsg );

     if ( usDosRc == NO_ERROR )
     {
       //--- Check whether all bytes have been written
       if ( ulBytesWritten != pExportIDA->ulBytesInBuffer )
       {
          //--- Close the exported file because
          //--- Disk full condition reached
          memcpy( szDrive, pExportIDA->ControlsIda.szPathContent, 2 );
          szDrive[2] = NULC;
          pReplString[0] = szDrive;
          CloseFile( &(pExportIDA->hFile) );
          UtlErrorHwnd( ERROR_WRITING, MB_CANCEL, 1,
                        &(pReplString[0]), EQF_ERROR, pExportIDA->hwndErrMsg );
          usRc = MEM_WRITE_ERR;
       }
       else
       {
         pExportIDA->ulTotalBytesWritten += pExportIDA->ulBytesInBuffer;

         //--- Clear the buffer
         memset( pExportIDA->acWriteBuffer, NULC, MEM_EXPORT_OUT_BUFFER );
         pExportIDA->ulBytesInBuffer = 0;
       } /* endif */
     }
     else
     {
        usRc = MEM_WRITE_ERR;
     } /* endif */
   } /* endif */

   if ( (usRc != MEM_PROCESS_OK) && (pExportIDA->pDDEMemExp) )
   {
     pExportIDA->pDDEMemExp->DDEReturn.usRc =  UtlGetDDEErrorCode( pExportIDA->hwndErrMsg );
   } /* endif */

   return usRc;
} /* end of function MemExportWrite  */
USHORT MemBatchTMExport
(
  HWND             hwndParent,         // handle of memory list window
  PDDEMEMEXP       pMemExp             // TM Export data structure
)
{
   BOOL            fOK = TRUE;         // return value
   PSZ             pszParm;            // error parameter pointer
   PMEM_EXPORT_IDA pIDA = NULL;        // Pointer to the export IDA

   /*******************************************************************/
   /* Allocate storage for the MEM_EXPORT_IDA.                        */
   /*******************************************************************/
   fOK = UtlAllocHwnd( (PVOID *)&pIDA, 0L, (LONG)sizeof(MEM_EXPORT_IDA),
                       ERROR_STORAGE, pMemExp->hwndErrMsg );

    if (fOK)
    {
        pIDA->ulOemCP = GetLangOEMCP(NULL);
        pIDA->ulAnsiCP = GetLangAnsiCP(NULL);
        pIDA->pProposal = new OtmProposal;
   }

   /*******************************************************************/
   /* Open the TM                                                     */
   /*******************************************************************/
   if ( fOK )
   {
     int iRC = 0;
     MemoryFactory *pFactory = MemoryFactory::getInstance();
     pIDA->pMem = pFactory->openMemory( NULL, pMemExp->szName, EXCLUSIVE, &iRC );
     if ( pIDA->pMem == NULL )
     {
       pFactory->showLastError( NULL, pMemExp->szName, pIDA->pMem, pMemExp->hwndErrMsg  );
       fOK = FALSE;
     }
   } /* endif */

   /*******************************************************************/
   /* Check existence of output file                                  */
   /*******************************************************************/
   if ( fOK )
   {
     strcpy( pIDA->ControlsIda.szPathContent, pMemExp->szOutFile );
     if ( UtlFileExist( pIDA->ControlsIda.szPathContent ) &&
          !pMemExp->fOverWrite )
     {
       pszParm = pIDA->ControlsIda.szPathContent;
       UtlErrorHwnd( ERROR_FILE_EXISTS_ALREADY_BATCH, MB_CANCEL, 1, &pszParm,
                     EQF_ERROR, pMemExp->hwndErrMsg );
       fOK = FALSE;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Open output file                                                */
   /*******************************************************************/
   if ( fOK )
   {
     USHORT      usDosRc;              // Return code from Dos functions
     USHORT      usAction;             // Action for DosOpen

     usDosRc = UtlOpenHwnd
       ( pIDA->ControlsIda.szPathContent, // file Name
         &(pIDA->hFile),          // file Handle (Out)
         &usAction,                // action taken
         0L,                       // file size
         FILE_NORMAL,              // normal attributes
         FILE_TRUNCATE |           // open an existing file and
         FILE_CREATE,              // truncate it or create it
                                   // if it does not exist
         (USHORT)(OPEN_ACCESS_WRITEONLY |   // deny Write to others but
         OPEN_SHARE_DENYWRITE  |   // allow write to me
         OPEN_FLAGS_WRITE_THROUGH),// write through system buffer
         0L,                       // Reserved
         TRUE,                         // display no error message
         pMemExp->hwndErrMsg );
     if ( usDosRc )  //--- error from open
     {
        //--- Reset file handle to NULL and set fOk to FALSE
        pIDA->hFile = NULLHANDLE;
        fOK = FALSE;
     }/* endif */
   } /* endif */

   /*******************************************************************/
   /* Start memory Export                                             */
   /*******************************************************************/
   if ( fOK )
   {
     OBJNAME ObjName;                  // buffer for object name
     SHORT   sRC;                      // return code of QuerySymbol function

     pIDA->fBatch     = TRUE;
     pIDA->hwndErrMsg = pMemExp->hwndErrMsg;
     pIDA->pDDEMemExp = pMemExp;
     pIDA->pIDA       = ACCESSWNDIDA(hwndParent, PMEM_IDA );
     pIDA->usEntryInDirToStop        = KEY_DIR_ENTRIES_NUM -1;
     pIDA->fAscii                    = TRUE;

     // build object name
     sprintf( ObjName, "MEMEXP: %s", pIDA->szMemName );
     sRC = QUERYSYMBOL( ObjName );
     if ( sRC == -1 )
     {
        // memory is not in use ==> lock this folder
        SETSYMBOL( ObjName );
        fOK = CreateProcessWindow( ObjName, MemExportCallBack, pIDA );
     }
     else
     {
       PSZ pTemp = pIDA->szMemName;

       UtlErrorHwnd( ERROR_MEM_NOT_ACCESSIBLE, MB_CANCEL,
                     1, &pTemp, EQF_ERROR, pMemExp->hwndErrMsg );
       fOK = FALSE;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Cleanup                                                         */
   /*******************************************************************/
   if ( !fOK )
   {
      pMemExp->DDEReturn.usRc =  UtlGetDDEErrorCode( pMemExp->hwndErrMsg );
      if ( pIDA )
      {
         if ( pIDA->pMem != NULL)
         {
           MemoryFactory *pFactory = MemoryFactory::getInstance();
           pFactory->closeMemory( pIDA->pMem );
           pIDA->pMem = NULL;
         } /* endif */
         if ( pIDA->hFile ) UtlClose( pIDA->hFile, FALSE );
         if ( pIDA->pProposal != NULL ) free(pIDA->pProposal); 
         UtlAlloc( (PVOID *) &pIDA, 0L, 0L, NOMSG );
      } /* endif */

      /****************************************************************/
      /* report end of task to DDE handler                            */
      /****************************************************************/
      WinPostMsg( pMemExp->hwndOwner, WM_EQF_DDE_ANSWER, NULL,
                  MP2FROMP(&pMemExp->DDEReturn) );
   } /* endif */

   return( pMemExp->DDEReturn.usRc );

} /* end of function MemBatchTMExport */

USHORT MemFuncExportMem
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemName,              // name of Translation Memory
  PSZ         pszOutFile,              // fully qualified name of output file
  LONG        lOptions                 // options for Translation Memory export
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // when running in COMPLETE_IN_ONE_CALL_OPT mode we use our own data area to be independent of other API calls running in parallel
  if ( lOptions & COMPLETE_IN_ONE_CALL_OPT )
  {
    PFCTDATA pPrivateData = (PFCTDATA)malloc( sizeof( FCTDATA ) );
    memset( pPrivateData, 0, sizeof( FCTDATA ) );
    pPrivateData->fComplete = TRUE;
    pPrivateData->usProgress = 0;

    usRC = MemFuncPrepExport( pPrivateData, pszMemName, pszOutFile, lOptions );
    if ( usRC == 0 )
    {
      while ( !pPrivateData->fComplete )
      {
        usRC = MemFuncExportProcess( pPrivateData );
      }
    }
    free( pPrivateData );
  }
  else
  {
    // prepare a new import or continue current one
    if ( pData->fComplete )              // has last run been completed
    {
      // prepare a new analysis run
      pData->usProgress = 0;
      usRC = MemFuncPrepExport( pData, pszMemName, pszOutFile, lOptions );
    }
    else
    {
      // continue current analysis process
      usRC = MemFuncExportProcess( pData );
    } /* endif */
  }
  return( usRC );
} /* end of function MemFuncExportMem */

// prepare the function I/F TM import
USHORT MemFuncPrepExport
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemName,              // name of Translation Memory
  PSZ         pszOutFile,              // fully qualified name of output file
  LONG        lOptions                 // options for Translation Memory export
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL            fOK = TRUE;          // internal O.K. flag
  PSZ             pszParm;             // error parameter pointer
  PMEM_EXPORT_IDA pIDA = NULL;         // Pointer to the export IDA
  PPROCESSCOMMAREA pCommArea = NULL;   // ptr to commmunication area

  UtlSetUShort( QS_LASTERRORMSGID, NO_ERROR );

  // allocate storage for the organize process communication area
  fOK = UtlAllocHwnd( (PVOID *)&pCommArea, 0L,
                     (LONG)sizeof(PROCESSCOMMAREA),
                     ERROR_STORAGE, HWND_FUNCIF );


  // allocate storage for the MEM_EXPORT_IDA
  if ( fOK )
  {
    fOK = UtlAllocHwnd( (PVOID *)&pIDA, 0L, (LONG)sizeof(MEM_EXPORT_IDA),
                        ERROR_STORAGE, HWND_FUNCIF );
  } /* endif */

 if (fOK)
 {
     pIDA->ulOemCP = GetLangOEMCP(NULL);
     pIDA->ulAnsiCP = GetLangAnsiCP(NULL);
     pIDA->pProposal = new OtmProposal;
 }
  // check if a TM has been specified
   if ( fOK )
   {
     if ( (pszMemName == NULL) || (*pszMemName == EOS) )
     {
       fOK = FALSE;
       UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL,
                     EQF_ERROR, HWND_FUNCIF );
     }
     else
     {
       strcpy( pIDA->szMemName, pszMemName );
     } /* endif */
   } /* endif */

  // open the TM
  if ( fOK )
  {
     int iRC = 0;
     MemoryFactory *pFactory = MemoryFactory::getInstance();
     pIDA->pMem = pFactory->openMemory( NULL, pszMemName, EXCLUSIVE, &iRC );
     if ( pIDA->pMem == NULL )
     {
       pFactory->showLastError( NULL, pszMemName, NULL, HWND_FUNCIF );
       fOK = FALSE;
     }
     
  } /* endif */

  // check if output file has been specified
  if ( fOK )
  {
    if ( (pszOutFile == NULL) || (*pszOutFile == EOS) )
    {
      UtlErrorHwnd( ERROR_NO_EXPORT_NAME, MB_CANCEL, 0, NULL,
                    EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // check existence of output file
  if ( fOK )
  {
    strcpy( pIDA->ControlsIda.szPathContent, pszOutFile );
    if ( UtlFileExist( pIDA->ControlsIda.szPathContent ) &&
         !(lOptions & OVERWRITE_OPT) )
    {
      pszParm = pIDA->ControlsIda.szPathContent;
      UtlErrorHwnd( ERROR_FILE_EXISTS_ALREADY_BATCH, MB_CANCEL, 1, &pszParm,
                    EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // open output file
  if ( fOK )
  {
    USHORT      usDosRc;              // Return code from Dos functions
    USHORT      usAction;             // Action for DosOpen

    usDosRc = UtlOpenHwnd
      ( pIDA->ControlsIda.szPathContent, // file Name
        &(pIDA->hFile),          // file Handle (Out)
        &usAction,                // action taken
        0L,                       // file size
        FILE_NORMAL,              // normal attributes
        FILE_TRUNCATE |           // open an existing file and
        FILE_CREATE,              // truncate it or create it
                                  // if it does not exist
        (USHORT)(OPEN_ACCESS_WRITEONLY |   // deny Write to others but
        OPEN_SHARE_DENYWRITE  |    // allow write to me
        OPEN_FLAGS_WRITE_THROUGH), // write through system buffer
        0L,                        // Reserved
        TRUE,                         // display no error message
        HWND_FUNCIF );
    if ( usDosRc )  //--- error from open
    {
       //--- Reset file handle to NULL and set fOk to FALSE
       pIDA->hFile = NULLHANDLE;
       fOK = FALSE;
    }/* endif */
  } /* endif */

  // prepare memory Export
  if ( fOK )
  {
    OBJNAME ObjName;                  // buffer for object name
    SHORT   sRC;                      // return code of QuerySymbol function

    pIDA->fBatch     = TRUE;
    pIDA->hwndErrMsg = HWND_FUNCIF;
    pIDA->pDDEMemExp = NULL;
    pIDA->pIDA       = NULL;
    pIDA->usEntryInDirToStop = KEY_DIR_ENTRIES_NUM -1;
    pIDA->fAscii     = TRUE;
    pIDA->NextTask   = MEM_START_EXPORT;

    if ( lOptions & UTF16_OPT )
    {
      pIDA->usExpMode = MEM_SGMLFORMAT_UNICODE;
    }
    else if ( lOptions & ANSI_OPT )
    {
      pIDA->usExpMode = MEM_SGMLFORMAT_ANSI;
    }
    else if ( lOptions & TMX_UTF8_OPT )
    {
      if ( lOptions & TMX_NOCRLF_OPT )
      {
        pIDA->usExpMode =  MEM_FORMAT_TMX_UTF8_NOCRLF;
      }
      else
      {
        pIDA->usExpMode =  MEM_FORMAT_TMX_UTF8;
      } /* endif */         
    }
    else if ( lOptions & TMX_UTF16_OPT )
    {
      if ( lOptions & TMX_NOCRLF_OPT )
      {
        pIDA->usExpMode =  MEM_FORMAT_TMX_NOCRLF;
      }
      else
      {
        pIDA->usExpMode =  MEM_FORMAT_TMX;
      } /* endif */         
    }
    else
    {
      pIDA->usExpMode = MEM_SGMLFORMAT_ASCII;
    } /* endif */


    // build object name
    sprintf( ObjName, "MEMEXP: %s", pIDA->szMemName );
    sRC = QUERYSYMBOL( ObjName );
    if ( sRC == -1 )
    {
       // TM is not in use ==> lock this TM
       SETSYMBOL( ObjName );
    }
    else
    {
      PSZ pTemp = pIDA->szMemName;

      UtlErrorHwnd( ERROR_MEM_NOT_ACCESSIBLE, MB_CANCEL,
                    1, &pTemp, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    } /* endif */

  } /* endif */

  // cleanup
  if ( fOK )
  {
    pData->fComplete = FALSE;
    pCommArea->pUserIDA = pIDA;
    pData->pvMemExportCommArea = pCommArea;
  }
  else
  {
     pData->fComplete = TRUE;
     usRC =  UtlGetDDEErrorCode( HWND_FUNCIF );
     if ( pIDA )
     {
        if ( pIDA->pMem != NULL)
        {
          MemoryFactory *pFactory = MemoryFactory::getInstance();
          pFactory->closeMemory( pIDA->pMem );
          pIDA->pMem = NULL;
        } /* endif */
        if ( pIDA->hFile )
        {
           UtlClose( pIDA->hFile, FALSE );
        } /* endif */
        if ( pIDA->pProposal != NULL ) free(pIDA->pProposal); 
        UtlAlloc( (PVOID *) &pIDA, 0L, 0L, NOMSG );
     } /* endif */
     if ( pCommArea) UtlAlloc( (PVOID *) &pCommArea, 0L, 0L, NOMSG );

     pData->fComplete = TRUE;
   } /* endif */

  return( usRC );
} /* end of function MemFuncPrepExport */

// prepare the function I/F TM import
USHORT MemFuncExportProcess
(
  PFCTDATA    pData                    // function I/F session data
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  PPROCESSCOMMAREA pCommArea;          // ptr to commmunication area
  PMEM_EXPORT_IDA  pIDA;               // pointer to instance area

  pCommArea = (PPROCESSCOMMAREA)pData->pvMemExportCommArea;
  pIDA = (PMEM_EXPORT_IDA)pCommArea->pUserIDA;

   switch ( pIDA->NextTask )
  {
    case MEM_START_EXPORT:
      usRC = EQFMemExportStart( pCommArea, HWND_FUNCIF );
      if ( usRC != NO_ERROR )
      {
        EQFMemExportEnd( pCommArea, HWND_FUNCIF, 0L );
        pData->fComplete = TRUE;
      } /* endif */
      break;

    case MEM_EXPORT_TASK:
      usRC = EQFMemExportProcess( pCommArea, HWND_FUNCIF );
      if ( usRC != NO_ERROR )
      {
        EQFMemExportEnd( pCommArea, HWND_FUNCIF, 0L );
        pData->fComplete = TRUE;
      } /* endif */
      break;

    case MEM_END_EXPORT:
      usRC = EQFMemExportEnd( pCommArea, HWND_FUNCIF, 0L );
      pData->fComplete = TRUE;
      break;
  } /* endswitch */
  pData->usProgress = pCommArea->usComplete;
  return( usRC );

} /* end of function MemFuncExportProcess */

USHORT  MemExportPrepareSegment( PMEM_EXPORT_IDA pExportIDA, PSZ_W pszSegmentIn, PSZ_W pszSegmentOut )
{
#ifdef FORCE_CRLF_IN_MEMORY_OUTPUT
  PSZ_W pszSource = pszSegmentIn;
  PSZ_W pszTarget = pszSegmentOut;

  pExportIDA;

  while( *pszSource )
  {
    if ( *pszSource == L'\r' )
    {
      // ignore carriage return
      pszSource++;
    }
    else if ( *pszSource == L'\n' )
    {
      // convert to CRLF
      pszSource++;
      *pszTarget++ = L'\r';
      *pszTarget++ = L'\n';
    }
    else
    {
      // copy as-is
      *pszTarget++ = *pszSource++;
    } /* endif */
  } /* endwhile */
  *pszTarget = 0;
#else
  wcscpy( pszSegmentOut, pszSegmentIn );
#endif
  return( MEM_PROCESS_OK );
}
