/*! \file
	Description:  Generic startup code for MFC, Win32 as well as OS/2

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

//  #define STARTUP2_LOGGING  

  #undef _WINDLL

#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_DAM              // QDAM functions
#define INCL_EQF_SLIDER           // slider utility functions
#include <eqf.h>                  // General Translation Manager include file

#include <time.h>                      // C library functions: time
#include <tchar.h>
#include "locale.h"

#include "eqfstart.id"                 // our PM IDs
#include "eqfstart.h"                  // our stuff
#include "eqfstart.mri"                // for text of emergency messages
#include "eqfdrvex.h"                  // drives dialog
#include "eqfsetup.h"                  // directory names
  #include "EQFPROGR.H"                // progress indicator defines
  #include <dde.h>                     // DDE defines
#include "eqfevent.h"                  // event logging facility
#include "EQFART.H"                    // ART registry functions
#include "EQFRPT.H"                    // report handler defines


// should be removed
#include "EQFMT00.H"

#undef _WPTMIF                         // we don't care about WP I/F
#include "eqfhelp.id"                  // help resource IDs
#include "eqfhlp1.h"                   // first part of help tables
#include "eqfmsg.htb"                          // message help table

#include "eqfutmdi.h"                // MDI Utilities


/**********************************************************************/
/* statics...                                                         */
/**********************************************************************/

static HHOOK  MessageFilterHook = NULL;// message filter hook
static WNDPROC lpfnOldMDIClient = NULL;// ptr to original MDI client proc
LONG FAR PASCAL NewMDIClientProc      (HWND, UINT, WPARAM, LPARAM);
VOID PerformPendingUpdates();


       CHAR     szMsgBuf[256];                   // general purpose message buffer
static CHAR     szEqfResFile[MAX_EQF_PATH];      // name of resource file
static CHAR     EqfSystemPath[MAX_EQF_PATH];     // global system path
static CHAR     EqfSystemPropPath[MAX_EQF_PATH]; // global system properties path
static CHAR     EqfSystemMsgFile[MAX_EQF_PATH];  // global message file
static CHAR     EqfSystemHlpFile[MAX_EQF_PATH];  // global help file
static CHAR     szEqfSysLanguage[MAX_EQF_PATH];  // system language
HWND      hTwb = NULLHANDLE;           // handle of Twb main window
static HWND     hHelp;                 // IPF help hwnd
static BOOL     fReportHandler = TRUE;           // report handler enabled flag
static BOOL     fSaveOnClose = FALSE;            // save not allowed during start
static USHORT   fShutdownInProgress;             // shutdown in progress flag
static BOOL     fNotMiniMized= FALSE;                      // save not allowed when minimized
static BOOL     fInCloseProcessing = FALSE;      // in close processing flag
static BOOL     fCloseCheckDone = FALSE;         // close check has been done flag
static BOOL     fMinimize = FALSE;               // start TWB frame not minimized
static BOOL     fMFCEnabled = FALSE;             // are we dealing with MFC??

static PUSERHANDLER pUserHandler;      // ptr to user handler table
static USHORT       usUserHandler;     // number of user handlers

CHAR szMsgWindow[] = ">wnd_msg";     // identifier for message help window

static HMODULE      hNetApiMod;
static HAB          hAB;
static BOOL         fIELookAndFeel = FALSE;


/**********************************************************************/
/* Windows related function prototypes                                */
/**********************************************************************/
  DWORD CALLBACK HelpMessageFilterHook( int, WORD, LPMSG );
  BOOL FAR PASCAL         ChildEnumProc( HWND hwnd, LPARAM mp2 );


//#pragma alloc_text(EQFSTART4, TwbSwitchHelpTable, \
//                              HelpMessageFilterHook, ChildEnumProc )


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TwbInit                Intialize MAT
//------------------------------------------------------------------------------
// Function call:     TwbInit()
//------------------------------------------------------------------------------
// Description:       Start all MAT handler and create the TWB window.
//------------------------------------------------------------------------------
// Input parameter:   VOID
//------------------------------------------------------------------------------
// Returncode type:   HWND
//------------------------------------------------------------------------------
// Returncodes:       NULL   in case of errors
//                    other  handle of translation workbench frame window
//------------------------------------------------------------------------------
// Side effects:      all MAT handlers are started
//------------------------------------------------------------------------------
// Function flow:     initialize the MAT Object Manager and anchor it
//                    register MAT window classes
//                    start all other handlers
//                    bring up the MAT main window (TwbWinInit)
//                    create the help instance
//                    if initialization successful then
//                       associate the help instance
//                       re-initialize error handler with new help instance
//------------------------------------------------------------------------------

HWND TwbInit( BOOL fMFC, BOOL fMinTWB )
{
   HWND     hObjMan;                   // handle of object manager window
   BOOL fOK = TRUE;                    // internal ok flag
   PSZ      pszErrParm;                // parameter for UtlError call
   HMODULE  hmod;                      // buffer for resource module handle
   HMODULE  hResMod;

   hAB = (HAB)UtlQueryULong( QL_HAB );
   fMinimize = fMinTWB;
   fMFCEnabled = fMFC;
   setlocale( LC_COLLATE, ".OCP" );

    /******************************************************************/
    /* Load main resource module                                      */
    /******************************************************************/
    if ( DosLoadModule ( NULL, NULLHANDLE, szEqfResFile, &hmod) )
    {
       fOK = FALSE;
       pszErrParm = szEqfResFile;
       UtlError( ERROR_RESOURCE_LOAD_FAILED, MB_CANCEL, 1,
                 &pszErrParm, EQF_ERROR );
    }
    else
    {
      // set global resource module handle and our local copy
      hResMod = hmod;
      UtlSetULong( QL_HRESMOD, (ULONG)hmod );
    } /* endif */

    /******************************************************************/
    /* Preload generally used icons                                   */
    /******************************************************************/
    UtlPreloadIcons();

    /******************************************************************/
    /* Initialize the MAT Object Manager and anchor it                */
    /******************************************************************/
    if ( fOK )
    {
       fOK = TwbStartHandler( hAB, OBJECTMANAGER, OBJECTMANAGERWP, NULL );
    } /* endif */

    if ( fOK )
    {
       // check if object manager is running
       if((hObjMan=EqfQueryObjectManager()) == NULL)   // object manager anchored ?
       {
          pszErrParm = OBJECTMANAGER;
          UtlError( ERROR_DID_NOT_START, MB_CANCEL, 1, &pszErrParm, EQF_ERROR );
          fOK = FALSE;
       } /* endif */
    } /* endif */

    /******************************************************************/
    /* Register MAT window classes                                    */
    /******************************************************************/
    if ( fOK )
    {
       UtlRegisterCLB ( hAB );         // register the CLB control
    } /* endif */

    /******************************************************************/
    /* start property handler                                         */
    /******************************************************************/
    if ( fOK ) fOK = TwbStartHandler( hAB, PROPERTYHANDLER, PROPERTYHANDLERWP, NULL );

    /******************************************************************/
    /* Set directory strings and main drive                           */
    /******************************************************************/
    if ( fOK )
    {
      PPROPSYSTEM  pPropSys = GetSystemPropPtr();

      UtlSetString( QST_PRIMARYDRIVE, pPropSys->szPrimaryDrive );
      UtlSetString( QST_LANDRIVE,     pPropSys->szLanDrive );
      UtlSetString( QST_PROPDIR,      pPropSys->szPropertyPath );
      UtlSetString( QST_CONTROLDIR,   pPropSys->szCtrlPath );
      UtlSetString( QST_PROGRAMDIR,   pPropSys->szProgramPath );
      UtlSetString( QST_DICDIR,       pPropSys->szDicPath );
      UtlSetString( QST_MEMDIR,       pPropSys->szMemPath );
      UtlSetString( QST_TABLEDIR,     pPropSys->szTablePath );
      UtlSetString( QST_LISTDIR,      pPropSys->szListPath );
      UtlSetString( QST_SOURCEDIR,    pPropSys->szDirSourceDoc );
      UtlSetString( QST_SEGSOURCEDIR, pPropSys->szDirSegSourceDoc );
      UtlSetString( QST_SEGTARGETDIR, pPropSys->szDirSegTargetDoc );
      UtlSetString( QST_TARGETDIR,    pPropSys->szDirTargetDoc );
      UtlSetString( QST_DLLDIR,       pPropSys->szDllPath );
      UtlSetString( QST_MSGDIR,       pPropSys->szMsgPath );
      UtlSetString( QST_EXPORTDIR,    pPropSys->szExportPath );
      UtlSetString( QST_BACKUPDIR,    pPropSys->szBackupPath );
      UtlSetString( QST_IMPORTDIR,    pPropSys->szDirImport );
      UtlSetString( QST_COMMEMDIR,    pPropSys->szDirComMem );
      UtlSetString( QST_COMPROPDIR,   pPropSys->szDirComProp );
      UtlSetString( QST_COMDICTDIR,   pPropSys->szDirComDict );
      UtlSetString( QST_SYSTEMDIR,    pPropSys->PropHead.szPath + 3 );
      UtlSetString( QST_DIRSEGNOMATCHDIR, "SNOMATCH" );
      UtlSetString( QST_DIRSEGMTDIR, "MT" );
      UtlSetString( QST_DIRSEGRTFDIR, "RTF" );
      UtlSetString( QST_PRTPATH,      pPropSys->szPrtPath );
      if ( pPropSys->szWinPath[0] )
      {
        UtlSetString( QST_WINPATH,    pPropSys->szWinPath );
      }
      else
      {
        UtlSetString( QST_WINPATH,      WINDIR );
      } /* endif */
      if ( pPropSys->szEADataPath[0] )
      {
        UtlSetString( QST_EADATAPATH,   pPropSys->szEADataPath );
      }
      else
      {
        UtlSetString( QST_EADATAPATH,   EADATADIR );
      } /* endif */

      if ( pPropSys->szPluginPath[0] )
      {
        UtlSetString( QST_PLUGINPATH,   pPropSys->szPluginPath );
      }
      else
      {
        UtlSetString( QST_PLUGINPATH,   PLUGINDIR );
      } /* endif */

      UtlSetString( QST_ORGEQFDRIVES, pPropSys->szDriveList );

      UtlSetString( QST_TQMPROJECTPATH,    pPropSys->szTQMProjectPath );
      UtlSetString( QST_TQMREPORTPATH,     pPropSys->szTQMReportPath );
      UtlSetString( QST_TQMARCHIVEPATH,    pPropSys->szTQMArchivePath );
      UtlSetString( QST_TQMEVALUATIONPATH, pPropSys->szTQMEvalPath );
      UtlSetString( QST_TQMVENDORPATH,     pPropSys->szTQMVendorPath );

      UtlSetString( QST_MTLOGPATH,         "MTLOG" );
      UtlSetString( QST_MISCPATH,          "MISC" );
      UtlSetString( QST_LOGPATH,           "LOGS" );
      UtlSetString( QST_XLIFFPATH,         "XLIFF" );
      UtlSetString( QST_METADATAPATH,      "METADATA" );
      UtlSetString( QST_JAVAPATH,          "JAVA" );
      UtlSetString( QST_ENTITY,            "ENTITY" );
      UtlSetString( QST_REMOVEDDOCDIR,     "REMOVED" );

      if ( !pPropSys->lSmallLkupFuzzLevel )  pPropSys->lSmallLkupFuzzLevel = 3300;
      UtlSetULong( QL_SMALLLOOKUPFUZZLEVEL,    pPropSys->lSmallLkupFuzzLevel );
      if ( !pPropSys->lMediumLkupFuzzLevel ) pPropSys->lMediumLkupFuzzLevel = 3300;
      UtlSetULong( QL_MEDIUMLOOKUPFUZZLEVEL,   pPropSys->lMediumLkupFuzzLevel );
      if ( !pPropSys->lLargeLkupFuzzLevel )  pPropSys->lLargeLkupFuzzLevel = 3300;
      UtlSetULong( QL_LARGELOOKUPFUZZLEVEL,    pPropSys->lLargeLkupFuzzLevel );

      if ( !pPropSys->lSmallFuzzLevel )  pPropSys->lSmallFuzzLevel = 3300;
	  UtlSetULong( QL_SMALLFUZZLEVEL,    pPropSys->lSmallFuzzLevel );
	  if ( !pPropSys->lMediumFuzzLevel ) pPropSys->lMediumFuzzLevel = 3300;
	  UtlSetULong( QL_MEDIUMFUZZLEVEL,   pPropSys->lMediumFuzzLevel );
	  if ( !pPropSys->lLargeFuzzLevel )  pPropSys->lLargeFuzzLevel = 3300;
      UtlSetULong( QL_LARGEFUZZLEVEL,    pPropSys->lLargeFuzzLevel );

    // set SGML/DITA processing flag
    UtlSetUShort( QS_SGMLDITAPROCESSING, (USHORT)!pPropSys->fNoSgmlDitaProcessing );

    // set XSLT engine variable
    UtlSetUShort( QS_XSLTENGINE, (USHORT)pPropSys->sXSLTEngine );

    UtlSetUShort( QS_ENTITYPROCESSING, (USHORT)pPropSys->fEntityProcessing );

    UtlSetUShort( QS_MEMIMPMRKUPACTION, (USHORT)pPropSys->usMemImpMrkupAction );

    UtlSetUShort( QS_HIDETOOLBARS, (USHORT)pPropSys->usHideToolBars );

    // in this version the MT display factor is set to 100%
	  UtlSetULong( QL_MTDISPLAYFACTOR, 100 );


// GQ: following code has been disabled to
      //     allow selection of initial language settings by user
//      if ( !pPropSys->szSystemPrefLang[0])
//      {
//        SHORT sRc = 0;
//        // Use English and 850 as default... user should change their default
//        strcpy(pPropSys->szSystemPrefLang, "English(U.S.)");
//        pPropSys->ulSystemPrefCP = 850;
//
//      }
    } /* endif */


    /******************************************************************/
    /* start all other handlers                                       */
    /******************************************************************/

#ifndef _TQM
#ifndef _TLEX
#ifndef _PTM
    if ( fOK ) fOK = TwbStartHandler( hAB, FILTERHANDLER, FILTHANDLERWP, NULL );
#endif
#endif
#endif

    /******************************************************************/
    /* Register the window classes used by the generic list window    */
    /* handler                                                        */
    /******************************************************************/
    if ( fOK )
    {
      /*******************************************************************/
      /* Register generic handler window class                           */
      /*******************************************************************/
      fOK = WinRegisterClass( (HINSTANCE) hAB, GENERICHANDLER, GENERICHANDLERWP, 0L,
                              sizeof(PVOID) );

      /*******************************************************************/
      /* Register generic list window class                              */
      /*******************************************************************/
      if ( fOK )
      {
        WNDCLASS   wndclass;
        ATOM       atom;

        wndclass.style         = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc   = GENERICLISTWP;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = sizeof(PVOID);
        wndclass.hInstance     = (HINSTANCE)hAB;
        wndclass.hIcon         = NULL;   // icon is drawn by list instance
        wndclass.hCursor       = LoadCursor( NULL, IDC_ARROW );
        wndclass.hbrBackground = (HBRUSH) GetStockObject( WHITE_BRUSH );
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = GENERICLIST;

        atom = RegisterClass( &wndclass );
      } /* endif */

      /*******************************************************************/
      /* Register generic process window class                           */
      /*******************************************************************/
      if ( fOK )
      {
        WNDCLASS   wndclass;
        ATOM       atom;

        wndclass.style         = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc   = GENERICPROCESSWP;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = sizeof(PVOID);
        wndclass.hInstance     = (HINSTANCE) hAB;
        wndclass.hIcon         = NULL;   // icon is drawn by list instance
        wndclass.hCursor       = LoadCursor( NULL, IDC_ARROW );
        wndclass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1) ;
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = GENERICPROCESS;

        atom = RegisterClass( &wndclass );
      } /* endif */
    } /* endif */


    // perfom any pending updates
    if ( fOK )
    {
      PerformPendingUpdates();
    } /* endif */       


#ifdef _TLEX
    // start any TLEX list handler
#elif defined(_TQM)
    // start TQM list handler
    if ( fOK ) fOK = TwbStartListHandler( TQMLISTHANDLER,
                                          TQMListHandlerCallBack, NULL );
#else
    // start TMgr handlers

    if ( fOK ) fOK = TwbStartListHandler( FOLDERHANDLER,
                                          FolderHandlerCallBack, NULL );
    if ( fOK ) fOK = TwbStartListHandler( FOLDERLISTHANDLER,
                                          FolderListHandlerCallBack, NULL );
    if ( fOK ) fOK = TwbStartListHandler( DICTIONARYHANDLER,
                                          DictListHandlerCallBack, NULL );
    if ( fOK ) fOK = TwbStartListHandler( TAGTABLEHANDLER,
                                          TagListHandlerCallBack, NULL );
    if ( fOK ) fOK = TwbStartListHandler( MEMORYHANDLER,
                                          MemListHandlerCallBack, NULL );
    if ( fOK ) fOK = TwbStartHandler( hAB, DOCUMENTHANDLER, DOCUMENTHANDLERWP, NULL );
    if ( fOK ) fOK = TwbStartHandler( hAB, ANALYSISHANDLER, ANALYSISHANDLERWP, NULL );
    if ( fOK ) fOK = TwbStartListHandler( LNGUPDATEHANDLER,
                                          LngListHandlerCallBack, NULL );
    if ( fOK ) fOK = TwbStartListHandler( COUNTHANDLER,
                                          WCntListHandlerCallBack, NULL );

    if ( fOK ) fOK = TwbStartListHandler( REPORTHANDLER, ReportListHandlerCallBack, NULL );

    if ( fOK ) fOK = TwbStartListHandler( LISTHANDLER, ListHandlerCallBack, NULL );
    if ( fOK ) fOK = TwbStartHandler( hAB, DDEHANDLER, DDEHANDLERWP, NULL );

#ifdef OEM_MT
    /******************************************************************/
    /* Start Machine Translation Listhandler                          */
    /* Depending on INI settings ....                                 */
    /******************************************************************/
    if ( fOK ) fOK = TwbStartListHandler( MTLISTHANDLER,
                                      MTListHandlerCallBack, NULL );
#endif

#endif


    /******************************************************************/
    /* Start any user handler                                         */
    /******************************************************************/
    if ( fOK )
    {
      fOK = TwbStartUserHandler( hAB );
    } /* endif */

    /******************************************************************/
    /* increase the maximum number of open files                      */
    /******************************************************************/
    if ( fOK )
    {
       /**************************************************************/
       /* SetHandleCount returns number of successful opened handles */
       /**************************************************************/
       {
         int iHandles;

         iHandles = SetHandleCount( 64 );
         if ( iHandles < 64 )
         {
           UtlError( ERROR_INSUFF_RESOURCES, MB_OK, 0, NULL, EQF_ERROR );
           fOK = FALSE;
         } /* endif */
       }
    } /* endif */

    /******************************************************************/
    /* bring up the Twb Main Window                                   */
    /******************************************************************/
    if ( fOK )
    {
       hTwb = TwbWinInit( fMFC );
       fOK = ( hTwb != NULLHANDLE );
       InitUnicode();
    } /* endif */



    UtlInitError( hAB, hTwb, NULL, EqfSystemMsgFile);

    // ensure that the codepage in the system properties matchs the current default language
    {
      PPROPSYSTEM  pSysProp = GetSystemPropPtr();
      if ( pSysProp != NULL )
      {
        ULONG ulCP = GetLangOEMCP( pSysProp->szSystemPrefLang );

        if ( ulCP != pSysProp->ulSystemPrefCP )
        {
          HPROP hPropSys = EqfQuerySystemPropHnd();
          if ( SetPropAccess( hPropSys, PROP_ACCESS_WRITE))
          {
            EQFINFO ErrorInfo = 0;
            pSysProp->ulSystemPrefCP = ulCP;
            SaveProperties( EqfQuerySystemPropHnd(), &ErrorInfo);
            ResetPropAccess( EqfQuerySystemPropHnd(), PROP_ACCESS_WRITE );
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */


#ifdef _TQM
    // For TQM: activate LOGON dialog
    {
      EqfSend2Handler( TQMLISTHANDLER, WM_EQF_COMMAND,
                       MP1FROMSHORT(PID_TQM_MI_LOGON), 0L );
    } /* endif */
#endif

    LOGSYSINFO();

    return( hTwb);

} /* end of function TwbInit */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TwbWinInit             Bring up the MAT main window
//------------------------------------------------------------------------------
// Function call:     TwbWinInit();
//------------------------------------------------------------------------------
// Description:       Load resource modules and create the MAT main window.
//------------------------------------------------------------------------------
// Input parameter:   BOOL fMFC
//------------------------------------------------------------------------------
// Returncode type:   HWND
//------------------------------------------------------------------------------
// Returncodes:       NULL   in case of errors
//                    other  handle of translation workbench frame window
//------------------------------------------------------------------------------
// Side effects:      all resource modules are loaded
//------------------------------------------------------------------------------
// Function flow:     load main resource module
//                    load NETAPI DLL
//                    load remaining MAT resource modules
//                    register window class for workbench window
//                    create main window and handle errors
//                    set MAT ULONGS QL_TWBFRAME and QL_TWBCLIENT
//                    add MAT to OS2 task list
//                    return frame window handle
//------------------------------------------------------------------------------
HWND TwbWinInit( BOOL fMFC )
{
    ULONG flStyle;                     // window creation flags
    HWND hframe = NULLHANDLE;          // handle of Twb frame window
    BOOL fOK = TRUE;                   // internal OK flag
    PSZ  pszParm;                      // parameter for UtlError calls
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

    /******************************************************************/
    /* Register class for superclassed MDI class                      */
    /******************************************************************/
    if ( fOK )
    {
      WNDCLASS  wc;

      // SuperClass the MDI client window. This involves getting the info
      // of the original class and then making the necessary modifications

      GetClassInfo( NULL,"mdiclient", &wc );
      lpfnOldMDIClient = (WNDPROC)wc.lpfnWndProc;  // Save old proc
      wc.lpszClassName = MDISUPERCLASS;            // Change the class name
      wc.hInstance     = (HINSTANCE) hAB;                      // Change the instance so that
                                                   // the class gets unregistered
                                                   // when the app terminates
      wc.lpfnWndProc   = NewMDIClientProc;         // Our new proc
      wc.hbrBackground = (HBRUSH) GetStockObject(LTGRAY_BRUSH);

      // Register the puppy
      fOK = RegisterClass( &wc );
    } /* endif */

    /******************************************************************/
    /* Register class for superclassed BS_HELP buttons                */
    /******************************************************************/
    if ( fOK )
    {
      UtlRegisterEqfHelp( hAB );
    } /* endif */

    /******************************************************************/
    /* Register window class for workbench window and handle errors   */
    /******************************************************************/
    if ( fOK )
    {
      {
        WNDCLASS   wc;

        wc.style = 0L;
        wc.lpfnWndProc = TwbMainWP;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = sizeof(PVOID);
        wc.hInstance = (HINSTANCE) hAB;
        wc.hIcon = NULLHANDLE;
        wc.hIcon = (HPOINTER)UtlQueryULong( QL_TWBICON );
        wc.hCursor = LoadCursor( NULL, IDC_ARROW );
        wc.hbrBackground = (HBRUSH) (HGDIOBJ)(COLOR_WINDOW + 1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = TWBMAIN;
        fOK = RegisterClass( &wc ) != NULLHANDLE;
      }
       if ( !fOK )
       {
          pszParm = TWBMAIN;
          UtlError( ERROR_DID_NOT_START, MB_CANCEL, 1, &pszParm, EQF_ERROR );
       } /* endif */
    } /* endif */

    /******************************************************************/
    /* Create main window and handle errors                           */
    /******************************************************************/
    if ( fOK )
    {
      if ( fMFC )
      {

        HMENU hmenu;
        hmenu = LoadMenu( hResMod, MAKEINTRESOURCE(ID_TWBM_WINDOW));
        UtlSetULong( QL_TWBMENU, (ULONG)hmenu );

        /****************************************************************/
        /* The window pulldown menu is the fourth pulldown of the       */
        /* TWB actionbar, therefore we have to specify '3' as index.    */
        /*                                                         |    */
        /********************************************************* V ****/
        UtlSetULong( QL_TWBWINDOWMENU, (ULONG) GetSubMenu( hmenu, 3 ));
      }
      else
      {
        HMENU hmenu;
         flStyle = FCF_TITLEBAR | FCF_SIZEBORDER | FCF_SYSMENU | FCF_MENU |
                   FCF_MINMAX | FCF_ICON;


         {
               WinLoadString( (HAB)NULL, (HMODULE)hResMod, SID_TWBM_TITLE,
                              sizeof(szMsgBuf), szMsgBuf );
         }

         hmenu = LoadMenu( hResMod, MAKEINTRESOURCE(ID_TWBM_WINDOW));
         UtlSetULong( QL_TWBMENU, (ULONG)hmenu );

         hframe = CreateWindowEx(  WS_EX_CLIENTEDGE |
                                   WS_EX_RTLREADING |
                                  WS_EX_LEFTSCROLLBAR,
                                  TWBMAIN,
                        szMsgBuf,      //window caption
                        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        HWND_DESKTOP, //parent wnd handle
                        hmenu,
                        (HINSTANCE) hAB,               //program instance handle
                        NULL );
         if ( !hframe )
         {
            fOK = FALSE;
            pszParm = TWBMAIN;
            UtlError( ERROR_DID_NOT_START, MB_CANCEL, 1, &pszParm, EQF_ERROR );
         } /* endif */
      } /* endif */
      /******************************************************************/
      /* Set MAT ULONGS QL_TWBFRAME and QL_TWBCLIENT                    */
      /******************************************************************/
      if ( fOK )
      {
          UtlSetULong( QL_TWBFRAME, (ULONG) hframe );
         /*************************************************************/
         /* window differently anchored under MFC                     */
         /*************************************************************/
         if ( fMFC )
         {
           UtlSetULong( QL_TWBCLIENT, (ULONG) hframe );
         } /* endif */
      } /* endif */
    } /* endif */


    /******************************************************************/
    /* Return frame window handle                                     */
    /******************************************************************/
    return( hframe);
} /* end of function TwbWinInit */

__declspec(dllexport)
VOID TwbWinInitCS( CREATESTRUCT * pCS )
{
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    ULONG flStyle;                     // window creation flags

    /******************************************************************/
    /* Create main window and handle errors                           */
    /******************************************************************/
    HMENU hmenu;
    flStyle = FCF_TITLEBAR | FCF_SIZEBORDER | FCF_SYSMENU | FCF_MENU |
              FCF_MINMAX | FCF_ICON;


    {
          WinLoadString( (HAB)NULL, (HMODULE)hResMod, SID_TWBM_TITLE,
                         sizeof(szMsgBuf), szMsgBuf );
    }

    hmenu = LoadMenu( hResMod, MAKEINTRESOURCE(ID_TWBM_WINDOW));
    UtlSetULong( QL_TWBMENU, (ULONG)hmenu );


    pCS->hInstance  = (HINSTANCE) hAB;
    pCS->hMenu      = hmenu;
    pCS->hwndParent = HWND_DESKTOP;
    pCS->cy         = CW_USEDEFAULT;
    pCS->cx         = CW_USEDEFAULT;
    pCS->y          = CW_USEDEFAULT;
    pCS->x          = CW_USEDEFAULT;
    pCS->style      =  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    pCS->lpszName   = szMsgBuf;
    pCS->lpszClass  = TWBMAIN;
    pCS->dwExStyle  = 0; //WS_EX_CLIENTEDGE;

    return;
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TwbMainWP              Window procedure for TWB window
//------------------------------------------------------------------------------
// Function call:     MRESULT TwbMainWP( hwnd, msg, mp1, mp2 )
//------------------------------------------------------------------------------
// Description:       Handles all messages for the TWB window
//------------------------------------------------------------------------------
// Input parameter:   HWND    hwnd    handle of list handler object window
//                    USHORT  msg     message number
//                    MPARAM  mp1     message parameter 1
//                    MPARAM  mp2     message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       depends on message type
//                    normal return codes are:
//                    TRUE  = message has been processed
//                    FALSE = message has not been processed
//------------------------------------------------------------------------------
// Function flow:     switch message
//                      case WM_CREATE:
//                        process WM_CREATE message: call TwbCreateMsg
//                      case WM_ERASEBACKGROUND:
//                        allow erase of backgrounf by returning TRUE
//                      case WM_CLOSE:
//                        post WM_QUIT to end message processing
//                      case WM_DESTROY:
//                        free windows table
//                        free menu table
//                        free IDA
//                      case WM_EQF_SHUTDOWN:
//                        if shutdown with SAVE then
//                           get write access to system properties
//                           save workbench size and position
//                           clear restart lists
//                           save object with focus
//                           reset write access to system properties
//                        endif
//                      case WM_EQF_ACTIVATEINSTANCE:
//                        if instance gets active then
//                          if focus object is changed then
//                            reset focus flag of old object
//                            reset busy flag of new class objects
//                          endif
//                          set status of new object to FOCUS and BUSY
//                          activate help table for new object class
//                        else
//                          reset FOCUS flag of object
//                        endif
//                      case WM_EQFN_OBJECTREMOVED:
//                        if shutdown with SAVE is in progress then
//                          get write access to system properties
//                          add object to approbiate restart list
//                          reset write access to system properties
//                        endif
//                      case WM_INITMENU:
//                        call TwbInitMenu to process the message
//                      case WM_MENUEND:
//                        if menu is not HELP or WINDOWS menu then
//                          send message to active object
//                        end
//                      case WM_COMMAND:
//                        switch control/command ID
//                          case tag table menu item:
//                            if tag table window is active then
//                              set focus to tag table window
//                            else
//                              open tag table window
//                            endif
//                          case language update menu item:
//                            if language update window is active then
//                              set focus to language update window
//                            else
//                              open language update window
//                            endif
//                          case book help item:
//                            start MAT book (call TwbStartBook)
//                          case tutorial help item:
//                            start MAT tutorial (call TwbStartTutorial)
//                          case drive configuration utility:
//                            call drives dialog (TwbDrives)
//                          case help for help item:
//                            start display of help for help
//                          case server list menu item:
//                            start server list dialog
//                          case remote TM menu item:
//                            start remote TM dialog
//                          default:
//                            if command is one of the window pd items then
//                              if command is minimize or restore all then
//                                allocate buffer for window SWP array
//                                loop over all windows in list
//                                  get current window position into SWP array
//                                  change window display flag
//                                endloop
//                                set new window positions
//                                free SWP array
//                              else
//                                get handle of selected window
//                                if window is a standard window then
//                                  activate the window
//                                else
//                                  set focus to window
//                                endif
//                              endif
//                            else
//                              send command to active instance
//                            endif
//                          case WM_PAINT:
//                            erase window background
//                          case HM_QUERY_KEYS_HELP:
//                            return help ID of keys help
//                          case HM_HELPSUBITEM_NOT_FOUND:
//                            set help ID to zero
//                            loop through main help table
//                              if window with given ID found then
//                                loop through help subtable of window
//                                  if control ID found then
//                                    set help ID to help for control
//                                  endif
//                                endloop
//                                if no control has been found then
//                                  set help ID to extended help of window
//                                endif
//                              else
//                                next entry in main help table
//                            endloop
//
//                            if no help ID has been set then
//                              loop through all entries in main table
//                                loop through all entries of subtable
//                                  if topic ID or subtopic ID is ID of entry
//                                    set help ID to help for control
//                                  endif
//                                  next entry
//                                endloop
//                                next subtable
//                              endloop
//                            endif
//
//                            if a help ID has been set then
//                              post help request to help instance
//                            endif
//
//                          case HM_EXT_HELP_UNDEFINED:
//                              post help request for TWB extend help
//                               to help instance
//                          case HM_TUTORIAL:
//                          case HM_INFORM:
//                            get tutorial entry point for current help
//                            start tutorial at entry point found
//                          case HM_ERROR:
//                            display error message
//                          default:
//                            pass message to default window procedure
//------------------------------------------------------------------------------
MRESULT APIENTRY TwbMainWP
(
    HWND      hwnd,
    WINMSG    message,
    WPARAM    mp1,
    LPARAM    mp2
)
{
    PTWBMAIN_IDA  pIda;                // Ptr to instance area header
    USHORT        usFlags;             // buffer for processing flags
    POBJLST       pObject;             // pointer for object list processing
    OBJNAME       szObjName;           // buffer for object name
    USHORT        usID;                // window ID
    USHORT        usClass;             // object class
    USHORT        usMask;              // object flags mask
    PSZ           mpObj;               // message parameter casted to PSZ
    HWND          mpHwnd;              // message parameter casted to HWND
    USHORT        usI;                 // general loop index
    USHORT        usHelpResID;         // resource ID for help panel
    PSHORT        psSubTable;          // ptr into HelpSubTable;
    USHORT        usElementSize;       // element size of help subtable
    MRESULT       mResult = FALSE;     // return code of window procedure
    BOOL          fNoClose;            // do not close flag

    PHELPTABLE    pstHelpEntry;        // ptr for help table processing

    switch( message )
    {
      case WM_CREATE:
        /**************************************************************/
        /* set default action bar id for help processing              */
        /**************************************************************/
        UtlSetUShort( QS_CURMENUID, ID_TWBM_WINDOW );
        return( TwbCreateMsg( hwnd, mp1, mp2 ) );

      case WM_DDE_INITIATE:
        return EqfSend2Handler( DDEHANDLER, message, mp1, mp2 );


      case WM_CLOSE:
        /**************************************************************/
        /* check that we haven't done already the close check         */
        /**************************************************************/
        if ( !fInCloseProcessing )
        {
          fInCloseProcessing = TRUE;
          // Add for Profile Setting Manage start
          // for OpenTM2Starter start
          char strPendingConf[MAX_PATH];
          char strPluginPath[MAX_PATH];

          UtlQueryString(QST_PLUGINPATH, strPluginPath, sizeof(strPluginPath));
          UtlMakeEQFPath(strPluginPath, NULC, PLUGIN_PATH, NULL);

          memset(strPendingConf, 0x00, sizeof(strPendingConf));
          sprintf(strPendingConf, "%s\\PendingUpdates.conf", strPluginPath);
          int nIsImport = GetPrivateProfileInt("Settings", "IsImport", 0,  strPendingConf);
          // Add end
          // Modify for Profile Setting Manage start
          if (nIsImport)
          {
              fNoClose = FALSE;
              WritePrivateProfileString("Settings", "IsImport", "0", strPendingConf);
          }
          else
          {
              fNoClose = TwbCloseCheck( );
          }
          // Modify end
          if ( !fNoClose )
          {
            fCloseCheckDone = TRUE;
            WinPostMsg( hwnd, WM_QUIT, NULL, NULL);
          }
          else
          {
            fInCloseProcessing = FALSE;
          } /* endif */
        } /* endif */
        return (MRESULT)NULL;
        break;

      case WM_DESTROY:
        /**************************************************************/
        /* Free all resources                                         */
        /**************************************************************/
        pIda = ACCESSWNDIDA( hwnd, PTWBMAIN_IDA );
        if (pIda)
        {
          UtlAlloc( (PVOID *) &pIda->hwndsInMenuWindows, 0L, 0L, NOMSG );
          UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
        }

        WinHelp( hwnd, EqfSystemHlpFile, HELP_QUIT, 0L );
        break;

      case WM_EQF_SHUTDOWN:
        /**************************************************************/
        /* Prepare shutdown of workbench: take system properties in   */
        /* write access, save window position and focus object, clear */
        /* restart lists and reset write access to system properties  */
        /**************************************************************/
        usFlags = SHORTFROMMP1( mp1);
        fShutdownInProgress = usFlags;   // save shutdown type flags here
        if( usFlags & TWBSAVE)
        {
          pIda = ACCESSWNDIDA( hwnd, PTWBMAIN_IDA );

          /************************************************************/
          /* get write access to system properties                    */
          /************************************************************/
          if( !SetPropAccess( pIda->hPropSys, PROP_ACCESS_WRITE))
          {
            UtlError( ERROR_ACCESS_SYSTEMPROPERTIES, MB_CANCEL,
                      0, (PSZ *) NULP, EQF_ERROR);
            return( (MRESULT)TRUE);    // stop Twb shutdown
          } /* endif */

          /************************************************************/
          /* Save window size and position                            */
          /************************************************************/
          UtlSaveWindowPos( pIda->IdaHead.hFrame, &pIda->pPropSys->Swp);

          /************************************************************/
          /* Clear restart lists                                      */
          /************************************************************/
          *pIda->pPropSys->RestartFolderLists = NULC;
          *pIda->pPropSys->RestartFolders = NULC;
          *pIda->pPropSys->RestartMemory  = NULC;
          *pIda->pPropSys->RestartDicts   = NULC;
          *pIda->pPropSys->RestartLists   = NULC;
          *pIda->pPropSys->RestartDocs    = NULC;
          pIda->pPropSys->FocusObject[0] = NULC;

          /************************************************************/
          /* Save focus object                                        */
          /************************************************************/
          if ( (usI = EqfQueryObjectCount( clsANY )) != 0 )
          {
             UtlAlloc( (PVOID *) &pObject, 0L, (LONG) ((usI * sizeof( *pObject)) + 20),
                       ERROR_STORAGE );
             if ( pObject )
             {
                usI = EqfGetObjectList( clsANY, usI, pObject);
                for( ; usI; usI--,pObject++ )
                {
                  if ( pObject->flgs & OBJ_FOCUS )
                  {
                     EqfQueryObjectName( pObject->hwnd,
                                         pIda->pPropSys->FocusObject );
                     break;
                  } /* endif */
                } /* endfor */
             } /* endif */
          } /* endif */

          /************************************************************/
          /* Reset write access to system properties                  */
          /************************************************************/
          ResetPropAccess( EqfQuerySystemPropHnd(), PROP_ACCESS_WRITE);
        }
        return( (MRESULT)NULL);

      case WM_EQFN_SHUTDOWNCANCELED:
        /**************************************************************/
        /* Clear shutdown flag as objects being removed have not to   */
        /* be recorded anymore                                        */
        /**************************************************************/
        fShutdownInProgress = 0;       // clear shutdown flags
        return( (MRESULT)NULL);


      case WM_EQF_ACTIVATEINSTANCE:
        /************************************************************/
        /* Process activation / deactivation of objects:            */
        /*                                                          */
        /* if object gets active:                                   */
        /*   if the instance hwnd (mp2) is not to the actual one    */
        /*     the OBJ_FOCUS flag is cleared for the old instance   */
        /*   if instance class is not changed then                  */
        /*      flag OBJ_BUSY is also cleared for the old instance  */
        /* else                                                     */
        /*   the object with flag OBJ_BUSY on is searched in the    */
        /*   class given by the new object (mp2) and the flag is    */
        /*   cleared.                                               */
        /* The instance getting activated and focussed by PM is     */
        /* flagged OBJ_ACTIVE , OBJ_FOCUS and OBJ_BUSY              */
        /*                                                          */
        /* if object gets inactive:                                 */
        /*   for instance loosing the focus OBJ_FOCUS is cleared    */
        /************************************************************/
        mpHwnd = (HWND)mp2;
        pIda = ACCESSWNDIDA( hwnd, PTWBMAIN_IDA );
        if ( SHORT1FROMMP1( mp1) )      // if instance is activated ...
        {
          if( mpHwnd != pIda->hObjFocus) // is instance changed ?
          {
            /**********************************************************/
            /* Get class of new instance                              */
            /**********************************************************/
            usClass = EqfQueryObjectClass( mpHwnd);

            /**********************************************************/
            /* Process focus object                                   */
            /**********************************************************/
            if( pIda->hObjFocus)           // is there an old focus object ?
            {
              usMask = OBJ_FOCUS;
              usMask |= ( usClass == pIda->usObjClass) ? OBJ_BUSY : 0;
              EqfSetObjectStatus( pIda->hObjFocus, usMask, 0);
            } /* endif */

            /********************************************************/
            /* Reset busy flag in objects of new class              */
            /********************************************************/
            {
              HWND hwnd;
              hwnd = EqfQueryObject( NULL, usClass, OBJ_BUSY);

              if( (hwnd != NULLHANDLE) &&
                  (hwnd != mpHwnd) )
              {
                EqfSetObjectStatus( hwnd, OBJ_BUSY, 0);
              } /* endif */
            }

            /**********************************************************/
            /* Set status of new instance                             */
            /**********************************************************/
            usMask = OBJ_ACTIVE | OBJ_FOCUS | OBJ_BUSY;
            EqfSetObjectStatus( mpHwnd, usMask, usMask);

            /************************************************************/
            /* Switch to the help table for the new class               */
            /************************************************************/
            if ( usClass != pIda->usObjClass )       // a new class ???
            {
               switch ( usClass )
               {
#ifdef _TQM
//                  case clsTQMLIST :
//                     TwbSwitchHelpTable( hlpsubtblFolListTWB,
//                                         hlpsubtblFolListChangeView );
//                     break;
#else
                  case clsFOLDERLIST :
                     TwbSwitchHelpTable( hlpsubtblFolListTWB,
                                         hlpsubtblFolListChangeView );
                     break;
                  case clsFOLDER :
                     TwbSwitchHelpTable( hlpsubtblDocListTWB,
                                         hlpsubtblDocListChangeView );
                     break;
                  case clsDICTIONARY :
                     TwbSwitchHelpTable( hlpsubtblDicListTWB,
                                         hlpsubtblDicListChangeView );
                     break;
                  case clsMEMORY :
                     TwbSwitchHelpTable( hlpsubtblMemListTWB,
                                         hlpsubtblMemListChangeView );
                     break;
                  case clsTAGTABLE:
                     TwbSwitchHelpTable( hlpsubtblTagListTWB,
                                         hlpsubtblTagListChangeView );
                     break;
                  case clsLIST:
                     TwbSwitchHelpTable( hlpsubtblListNtlTWB,
                                         hlpsubtblListNTLChangeView );
                     break;
                  case clsLNGUPDATE:
                     TwbSwitchHelpTable( hlpsubtblLngUpdTWB,
                                         hlpsubtblNoHelpChangeView );
                     break;
                  case clsDOCUMENT :
                     TwbSwitchHelpTable( hlpsubtblTPMainWindowTWB,
                                         hlpsubtblNoHelpChangeView );
                     break;
                  case clsTMMAINT :
                     TwbSwitchHelpTable( hlpsubtblTMMMainWindowTWB,
                                         hlpsubtblNoHelpChangeView );
                     break;
#endif
                  default:
                     TwbSwitchHelpTable( hlpsubtblNoHelpTWB,
                                         hlpsubtblNoHelpChangeView );
                     break;
               } /* endswitch */
            } /* endif */

            /**********************************************************/
            /* Remember new object handle and class                   */
            /**********************************************************/
            pIda->hObjFocus  = mpHwnd;
            pIda->usObjClass = usClass;
          } /* endif */
        }
        else
        {
          /************************************************************/
          /* set instance status to 'not active'                      */
          /************************************************************/
          EqfSetObjectStatus( mpHwnd, OBJ_FOCUS, 0);
          if ( mpHwnd == pIda->hObjFocus )
          {
            pIda->hObjFocus = NULLHANDLE;
          } /* endif */
        } /* endif */
        return( (MRESULT)NULL);

      case WM_EQFN_OBJECTREMOVED:
        /**************************************************************/
        /* during shutdown all objects being removed are added to     */
        /* the restart lists in the system proerties                  */
        /**************************************************************/
        if( fShutdownInProgress & TWBSAVE )
        {
          usClass = SHORT1FROMMP1( mp1);
          pIda = ACCESSWNDIDA( hwnd, PTWBMAIN_IDA );

          mpObj = (PSZ) PVOIDFROMMP2(mp2);

          /************************************************************/
          /* Get write access to system properties and add object     */
          /* to restart lists                                         */
          /************************************************************/
          if( SetPropAccess( pIda->hPropSys, PROP_ACCESS_WRITE) )
          {
             switch ( usClass )
             {
                case clsMEMORY:
                   ADDTOLIST( pIda->pPropSys->RestartMemory, mpObj );
                   break;
                case clsDICTIONARY:
                   ADDTOLIST( pIda->pPropSys->RestartDicts, mpObj );
                   break;
                case clsFOLDER:
                   if (!fIELookAndFeel)
                   ADDTOLIST( pIda->pPropSys->RestartFolders, mpObj );
                   break;
                case clsFOLDERLIST:
                   ADDTOLIST( pIda->pPropSys->RestartFolderLists, mpObj );
                   break;
                case clsLIST:
                   ADDTOLIST( pIda->pPropSys->RestartLists, mpObj );
                   break;
                case clsDOCUMENT:
                   ADDTOLIST( pIda->pPropSys->RestartDocs, mpObj );
                   break;
                default:
                   break;
             } /* endswitch */
             ResetPropAccess( pIda->hPropSys, PROP_ACCESS_WRITE);
          } /* endif */
        } /* endif */
        return( (MRESULT)NULL);

      // Process activation of menus and pulldowns
      // Windows: use WM_INITMENUPOPUP, convert parameter so that TwbInitMenu can be used
      // OS/2:    use WM_INITMENU
      case WM_INITMENUPOPUP:
        /**************************************************************/
        /* log current system status                                  */
        /**************************************************************/
        LOGSYSINFO();
        /**************************************************************/
        /* Handle activation of pulldowns/popups                      */
        /**************************************************************/
        if ( !HIWORD(mp2) )                   // only for normal (=not system) menu
        {
          // under Windows we have two AABs which are toggled dynamically
          // the checks in the TWBInitMenu routine only apply for the
          // standard TWB AAB.
          // If the new AAB is active, the messages are passed on to
          // the focus window to handle the request....
          if ( GetMenu( hwnd ) != (HMENU) UtlQueryULong(QL_TWBMENU))
          {
             //--- leave it to active instance to set menu items ---
             pIda = ACCESSWNDIDA( hwnd, PTWBMAIN_IDA );
             if ( pIda->hObjFocus )
             {
               WinSendMsg( pIda->hObjFocus, WM_EQF_INITMENU, mp1, mp2);
             } /* endif */
          }
          else
          {
            TwbInitMenu( hwnd, LOWORD(mp2), mp2 );
          } /* endif */
        } /* endif */
        return( (MRESULT)NULL);

      case WM_COMMAND:
        /**************************************************************/
        /* Process command messages                                   */
        /**************************************************************/
        pIda = ACCESSWNDIDA( hwnd, PTWBMAIN_IDA );
        usID = SHORT1FROMMP1( mp1);

        /**************************************************************/
        /* Check against command values used by user handlers         */
        /**************************************************************/
        if ( usUserHandler )
        {
          USHORT i, j;                 // loop indices
          for ( i = 0; i < usUserHandler; i++ )
          {
            j = 0;
            while ( pUserHandler[i].ausCommands[j] )
            {
              if ( pUserHandler[i].ausCommands[j] == usID )
              {
                if ( pUserHandler[i].fIsListHandler )
                {
                  WinSendMsg( pUserHandler[i].hwnd, WM_EQF_COMMAND, mp1, mp2 );
                }
                else
                {
                  EqfSend2Handler( pUserHandler[i].szHandler, WM_EQF_COMMAND, mp1, mp2 );
                } /* endif */
                return( (MRESULT)NULL );   // no more processing for this command
              }
              else
              {
                j++;                   // try next command value
              } /* endif */
            } /* endwhile */
          } /* endfor */
        } /* endif */

        switch ( usID )
        {
           case PID_UTILS_MI_TAGTABLE:
             /*********************************************************/
             /* Handle request for tag table list window              */
             /*********************************************************/
              mpHwnd = EqfQueryObject( NULL, clsTAGTABLE, 0 );
              if ( mpHwnd != NULLHANDLE )
              {
                 /*****************************************************/
                 /* activate existing tag table list                  */
                 /*****************************************************/
                 WinSetFocus( HWND_DESKTOP, mpHwnd );
              }
              else
              {
                 /*****************************************************/
                 /* start a new tag table list                        */
                 /*****************************************************/
                UtlMakeEQFPath( szObjName, NULC, SYSTEM_PATH, NULL );
                strcat( szObjName, BACKSLASH_STR );
                strcat( szObjName, TAGTABLE_PROPERTIES_NAME );
                EqfSend2Handler( TAGTABLEHANDLER,
                                 WM_EQF_OPEN,
                                 MP1FROMSHORT(FALSE),
                                 MP2FROMP(szObjName) );
                UtlDispatch();                   // wait for object to come up
              } /* endif */
              return( (MRESULT)NULL);                 // return any way
              break;

           case PID_FILE_MI_SYSPROP:
             TWBSystemProps();
             break;

           case PID_FILE_MI_EXIT:
             PostMessage( hwnd, WM_CLOSE, 0, 0 );
             TWBSystemProps();
             break;

           case PID_UTILS_MI_CONNECT:
             /*********************************************************/
             /* Handle request for connect dialog                     */
             /*********************************************************/
             if ( pIda->hObjFocus )
             {
               usClass = EqfQueryObjectClass( pIda->hObjFocus );
               switch ( usClass )
               {
                  case clsMEMORY :
                  case clsDICTIONARY :
                     {
                       HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                       TWBConnect( hwnd, hResMod, usClass );
                     }
                     break;
                  case clsFOLDERLIST :
                  case clsFOLDER :
                  case clsTAGTABLE:
                  case clsLIST:
                  case clsLNGUPDATE:
                  case clsDOCUMENT :
                  default:
                     break;
               } /* endswitch */
             } /* endif */
             return( (MRESULT)NULL);                 // return any way
             break;

           case PID_UTILS_MI_NOISE:
           case PID_UTILS_MI_EXCLUSION:
           case PID_UTILS_MI_NEWTERMS:
           case PID_UTILS_MI_FOUNDTERMS:
           case PID_UTILS_MI_ABBR:
             /*********************************************************/
             /* Handle request for terminology list window            */
             /*********************************************************/

             /*********************************************************/
             /* Setup object name                                     */
             /*********************************************************/
             switch ( usID )
             {
               case PID_UTILS_MI_NOISE:
                strcpy( szObjName, NOISELISTOBJ );
                break;
               case PID_UTILS_MI_EXCLUSION:
                strcpy( szObjName, EXCLUSIONLISTOBJ );
                break;
               case PID_UTILS_MI_NEWTERMS:
                strcpy( szObjName, NEWTERMLISTOBJ );
                break;
               case PID_UTILS_MI_FOUNDTERMS:
                strcpy( szObjName, FOUNDTERMLISTOBJ );
                break;
               case PID_UTILS_MI_ABBR:
                strcpy( szObjName, ABBRLISTOBJ );
                break;
             } /* endswitch */

             /*********************************************************/
             /* Pass request to list handler                          */
             /*********************************************************/
             EqfSend2Handler( LISTHANDLER,
                              WM_EQF_OPEN,
                              MP1FROMSHORT(FALSE),
                              MP2FROMP(szObjName) );
              UtlDispatch();                   // wait for object to come up
              return( (MRESULT)NULL);                 // return any way
              break;

           case PID_UTILS_MI_LNGUPDATE:
              /********************************************************/
              /* Handle request for language update window            */
              /********************************************************/
              mpHwnd = EqfQueryObject( NULL, clsLNGUPDATE, 0 );
              if ( mpHwnd != NULLHANDLE )
              {
                 /*****************************************************/
                 /* activate existing language update window          */
                 /*****************************************************/
                 WinSetFocus( HWND_DESKTOP, mpHwnd );
              }
              else
              {
                /*****************************************************/
                /* start a language properties update list           */
                /*****************************************************/
                EqfSend2Handler( LNGUPDATEHANDLER,
                                 WM_EQF_CREATE,
                                 MP1FROMSHORT(FALSE),
                                 MP2FROMP(EMPTY_STRING) );
                UtlDispatch();                   // wait for object to come up
              } /* endif */
              return( (MRESULT)NULL);                 // return any way
              break;


           case PID_HELP_MI_PRODUCTINFO:
              /********************************************************/
              /* Handle request for the MAT book                      */
              /********************************************************/
              // TwbStartBook();
              TwbShowLogo( 0L, hwnd, szEqfResFile, EQFSTART, 0 );
              return( (MRESULT)NULL);                 // return any way
              break;


           case PID_HELP_MI_INDEX :
              WinHelp( hwnd, EqfSystemHlpFile, HELP_CONTENTS, 0L );
              return( (MRESULT)NULL);                 // return any way
              break;


           case PID_FILE_MI_EBUS:

              StartBrowser("home");
              break;



           case PID_FILE_MI_EBUS1:

              StartBrowser("docu");
              break;

           case PID_FILE_MI_TECHGUIDE:

              StartBrowser("techguide");
              break;

           case PID_FILE_MI_EBUS2:

              StartBrowser("docu");
              break;

           case PID_UTILS_MI_DRIVES:
              {
                /********************************************************/
                /* handle drive configuration request                   */
                /********************************************************/
                HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                TWBDrives( hwnd, hResMod, FALSE );
                return( (MRESULT)NULL);                 // return w/o default frame proc
              }
              break;

           case PID_HELP_FOR_HELP :
              /********************************************************/
              /* Handle help for help request                         */
              /********************************************************/

              {
                BOOL    fOK;

                fOK = WinHelp( hwnd, EqfSystemHlpFile, HELP_HELPONHELP, 0L );
#ifdef _DEBUG
                if ( !fOK )
                {
                  MessageBox( hwnd, "Call to WINHELP.EXE failed!",
                              "Assertion failed", MB_OK );
                } /* endif */
#endif
              }
              return( (MRESULT)NULL);                 // return w/o default frame proc
              break;

#if defined(_TQM)
           case PID_TQM_MI_LOGON:
           case PID_TQM_MI_LOGOFF:
           case PID_TQM_MI_USER:
           case PID_TQM_MI_EVALUATION:
           case PID_TQM_MI_VENDOR:
           case PID_TQM_MI_REPORTS:
             /*********************************************************/
             /* Pass TQM commands to TQM vendor list handler          */
             /*********************************************************/
             EqfSend2Handler( TQMLISTHANDLER, WM_EQF_COMMAND, mp1, mp2 );
             return( NULL);                 // return w/o default frame proc
             break;
#endif

           case PID_WIND_MI_MINALL :
           case PID_WIND_MI_RESTOREALL :
               EnumChildWindows( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                                 ChildEnumProc,
                                 MP2FROMSHORT( SHORT1FROMMP1(mp1) ) );
             return( (MRESULT)NULL);                 // return w/o default frame proc
             break;

           case PID_WIND_MI_TILE :
             WinSendMsg( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                         WM_MDITILE, MP1FROMSHORT(0), 0L );
             return( (MRESULT)NULL);                 // return w/o default frame proc
             break;

           case PID_WIND_MI_CASCADE :
             WinSendMsg( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                         WM_MDICASCADE, 0, 0L );
             return( (MRESULT)NULL);             // return w/o default frame proc
             break;

          default:
             /*********************************************************/
             /* Check for panel language selections                   */
             /*********************************************************/
             if( (usID >= PID_UTILS_MI_LANGFIRST) &&
                 (usID <= PID_UTILS_MI_LANGLAST) )
             {
               CHAR szNewLanguage[MAX_EQF_PATH];
               HWND hwndMenu;

               /*******************************************************/
               /* Get name of language being activated                */
               /*******************************************************/
               hwndMenu = (HWND)UtlQueryULong(QL_TWBMENU);
               GetMenuString( GetMenu( hwnd ), usID, szNewLanguage,
                              sizeof(szNewLanguage), MF_BYCOMMAND );
               /*******************************************************/
               /* Activate new language                               */
               /*******************************************************/
               if ( stricmp( szEqfSysLanguage, szNewLanguage ) != 0 )
               {
                 PSZ pszErrParm = szNewLanguage;
                 UtlError( INFO_LANG_ACTIVATED, MB_OK, 1, &pszErrParm, EQF_INFO );
               } /* endif */
               WriteStringToRegistry( APPL_Name, KEY_SYSLANGUAGE, szNewLanguage );
             }
             else
             // Windows: PID_WIND_MI_MINALL and PID_WIND_MI_RESTOREALL are
             //          processed in their own case in the code above
             //          Windows in the window list are handle by the MDI client window
             {
               /*******************************************************/
               /* send command to active instance                     */
               /*******************************************************/
               // Pass command messages to active MDI window
               {
                 HWND hwndChild;

                 hwndChild = (HWND)SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                                                 WM_MDIGETACTIVE, 0, 0L );
                 if ( IsWindow( hwndChild )  )
                 {
                    SendMessage( hwndChild, WM_COMMAND, mp1, mp2 );
                 } /* endif */

                   mResult = DefFrameProc( hwnd,
                                           (HWND)UtlQueryULong( QL_TWBCLIENT ),
                                           message, mp1, mp2 );
               }
               return( mResult );              // return w/o default frame proc
              } /* endif */

        } /* endswitch */
        return DefFrameProc( hwnd,
                             (HWND)UtlQueryULong( QL_TWBCLIENT ),
                             message, mp1, mp2 );
        break;


      case  WM_SIZE :
        /*********************************************************/
        /* size client window to reflect foolbar and Statusbar   */
        /* if necessary  - currently only toolbar ...            */
        /*********************************************************/
        {
            return DefFrameProc( hwnd,
                                 (HWND)UtlQueryULong( QL_TWBCLIENT ),
                                 message, mp1, mp2 );
        }
        break;


     case (HM_HELPSUBITEM_NOT_FOUND):
       {
         SHORT sTopic    = SHORT1FROMMP2(mp2);
         SHORT sSubTopic = SHORT2FROMMP2(mp2);

         /**************************************************************/
         /* IPF didn't find our window in its help table               */
         /* now we try to do IPF's work...                             */
         /**************************************************************/
         pstHelpEntry = htblMain;
         usHelpResID = 0;               // no help resid found so far

         LOG2SHORT( "Help request for topic %d, subtopic %d",
                    sTopic, sSubTopic );

         /**************************************************************/
         /* First try: search thru the general help table              */
         /**************************************************************/
         while ( pstHelpEntry->phstHelpSubTable && !usHelpResID )
         {
            if ( pstHelpEntry->idAppWindow == (USHORT)sTopic )
            {
               /********************************************************/
               /* window has been spotted, now try to locate the help  */
               /* for the subtopic                                     */
               /********************************************************/
               // window has been spotted, now try to locate the help
               // for the subtopic
               psSubTable = (PSHORT) pstHelpEntry->phstHelpSubTable;
               usElementSize = *psSubTable++;
               while ( *psSubTable && !usHelpResID )
               {
                  if ( *psSubTable == sSubTopic )
                  {
                     usHelpResID = *(psSubTable+1);
                     if ( !usHelpResID )
                     {
                       // no subtopic help available
                       usHelpResID = pstHelpEntry->idExtPanel;
                     } /* endif */
                  }
                  else
                  {
                     psSubTable += usElementSize;
                  } /* endif */
               } /* endwhile */

               if ( !usHelpResID )      // no subtopic help found ...
               {
                  // ... use the extended help of the window
                  usHelpResID = pstHelpEntry->idExtPanel;
               } /* endif */
               // use extended help anyway
               usHelpResID = pstHelpEntry->idExtPanel;
            }
            else
            {
               pstHelpEntry++;
            } /* endif */
         } /* endwhile */

         /**************************************************************/
         /* second try: loop thru all subtables                        */
         /**************************************************************/
         pstHelpEntry = htblMain;
         while ( pstHelpEntry->phstHelpSubTable && !usHelpResID )
         {
            psSubTable = (PSHORT) pstHelpEntry->phstHelpSubTable;
            usElementSize = *psSubTable++;
            while ( *psSubTable && !usHelpResID )
            {
               if ( (*psSubTable == sTopic) || (*psSubTable == sSubTopic) )
               {
                  usHelpResID = *(psSubTable+1);

                  // use extended help anyway
                  usHelpResID = pstHelpEntry->idExtPanel;
               }
               else
               {
                  psSubTable += usElementSize;
               } /* endif */
            } /* endwhile */
            pstHelpEntry++;             // skip to next subtable
         } /* endwhile */

         if ( usHelpResID )
         {
           LOGSHORT( "Showing help with ID %d", usHelpResID );

           if ( UtlQueryUShort( QS_CURMSGID ) == usHelpResID )
           {
             /***********************************************************/
             /* Help for a message box ==> use secondary help window    */
             /***********************************************************/
             strcpy( szMsgBuf, EqfSystemHlpFile );
             strcat( szMsgBuf, szMsgWindow );
             WinHelp( hwnd, szMsgBuf, HELP_CONTEXT, (ULONG)usHelpResID );
           }
           else
           {
             /***********************************************************/
             /* normal help ==> use primary help window                 */
             /***********************************************************/
             WinHelp( hwnd, EqfSystemHlpFile, HELP_CONTEXT, (ULONG)usHelpResID );
           } /* endif */
           return( MRFROMSHORT( TRUE ) );
         }
         else
         {
            WinHelp( hwnd, EqfSystemHlpFile, HELP_CONTEXT, (ULONG)HID_TWBM_EXTHELP );
            return( MRFROMSHORT( FALSE ) );
         } /* endif */
       }
       break;

     case WM_MENUSELECT:
       // Keep track of currently selected menu item in case of help requests
       if ( !( mp2 & MF_POPUP ) )
       {
         UtlSetUShort( QS_CURMENUITEMID, SHORT1FROMMP1(mp1) );
       }
       else
       {
         UtlSetUShort( QS_CURMENUITEMID, 0 );
       } /* endif */
       break;

     case WM_QUERYENDSESSION:
       return ( ! TwbCloseCheck());      // let user decide if he wants to close
       break;

     case WM_ENDSESSION:
       if ( SHORT1FROMMP1(mp1) )
       {
         EqfStopObjectManager( TWBCLOSE );
       } /* endif */
       break;

     default:
       break;
    } /* endswitch */

    /**************************************************************/
    /* pass message to default window procedure                   */
    /**************************************************************/
    mResult = DefFrameProc( hwnd,
                            (HWND)UtlQueryULong( QL_TWBCLIENT ),
                            message, mp1, mp2 );

    return( mResult );
} /* end of function TwbMainWP */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// name:     TwbCreateMsg           Process WM_CREATE message of TWB
//------------------------------------------------------------------------------
// Function call:     TwbCreateMsg( HWND hwnd, MPARAM mp1, MPARAM mp2 );
//------------------------------------------------------------------------------
// Description:       Process the create message for the TWB window.
//                    During creation of the TWB window all objects are
//                    restarted and the initial window size and position is
//                    set.
//------------------------------------------------------------------------------
// Input parameter:   HWND   hwnd
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       FALSE (always)
//------------------------------------------------------------------------------
// Side effects:      save objects are restarted
//                    WM_EQF_INITIALIZE is posted
//------------------------------------------------------------------------------
// Function flow:     allocate and anchor IDA
//                    fill IDA head
//                    get pointer to system properties
//                    set Utl string values
//                    set initial EQF drive list
//                    validate EQF drives (call to UtlGetCheckedEqfDrives)
//                    register TWB handler
//                    post initialization message
//                    load menu table
//                    restore window to saved window size
//                    restart all saved objects
//                    initalize the slider utility (call to fUtlInitSlider)
//                    activate focus object
//                    set final TWB window state
//------------------------------------------------------------------------------
MRESULT TwbCreateMsg( HWND hwnd, WPARAM mp1, LPARAM mp2 )
{
  MRESULT       mResult = FALSE;       // function return code
  HWND          hwndFocus;             // handle of focus window
  PTWBMAIN_IDA  pIda;                  // pointer to IDA
//CHAR          szFirstTime[MAX_FNAME];// buffer for profile first time entry
  SHORT         cxScreen, cyScreen;    // width and height of physical screen

  mp1;                                 // avoid compiler warnings
  mp2;

#ifdef STARTUP2_LOGGING
   FILE *hStartLog = NULL;
   CHAR szLogFile[MAX_EQF_PATH];
   time_t lCurTime = 0;

 //  UtlMakeEQFPath( szLogFile, NULC, SYSTEM_PATH, NULL );
   strcpy( szLogFile, "\\OTM\\LOGS" );
   UtlMkDir( szLogFile, 0L, FALSE );

   strcat( szLogFile, "\\OpenTM2Startup.LOG2" );
   hStartLog = fopen( szLogFile, "wb" );
   if ( hStartLog )
   {
      time( &lCurTime );
      fprintf( hStartLog, "------ OpenTM2 startup2:               %s", asctime( localtime( &lCurTime ) ) );
   } /* endif */
#endif

  UtlSetULong( QL_TWBCLIENT, (ULONG)hwnd );

  UtlSetUShort( QS_CURMENUID, ID_TWBM_WINDOW );
  UtlSetULong( QL_TWBFRAME, (ULONG) hwnd );
  /********************************************************************/
  /* if hTwb not set yet, use frame window -- this will happen in the */
  /* MFC case                                                         */
  /********************************************************************/
  if ( !hTwb )
  {
    hTwb = hwnd;
  } /* endif */

  /********************************************************************/
  /* Allocate buffer for IDA                                          */
  /********************************************************************/
  UtlAlloc( (PVOID *) &pIda, 0L, (LONG)sizeof(*pIda), ERROR_STORAGE );

  if( !pIda )
  {
    mResult = MRFROMSHORT(TRUE); // do not create the window
  }
  else
  {
    /************************************************************/
    /* anchor IDA                                               */
    /************************************************************/
    ANCHORWNDIDA( hwnd, pIda);


    // Create client window (under OS/2 this is done by WInCreateStdWindow)
    {
      CLIENTCREATESTRUCT stClientCreate;
      HWND               hwndClient;

      /****************************************************************/
      /* The window pulldown menu is the fourth pulldown of the       */
      /* TWB actionbar, therefore we have to specify '3' as index.    */
      /*                                                         |    */
      /*                                                         |    */
      /********************************************************* V ****/
      stClientCreate.hWindowMenu  = GetSubMenu( (HMENU)UtlQueryULong( QL_TWBMENU ), 3 );
      stClientCreate.idFirstChild = ID_FLIST_WINDOW;
      UtlSetULong( QL_TWBWINDOWMENU, (ULONG) stClientCreate.hWindowMenu );
      hwndClient = CreateWindow( MDISUPERCLASS, NULL,
                                 WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
                                 0, 0, 0, 0, hwnd,
                                 (HMENU)0xCAC,     // use magic number ...
                                 (HINSTANCE) hAB,
                                 (LPSTR)&stClientCreate );

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 1.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

      if ( fMFCEnabled )
      {
        UtlSetULong( QL_TWBCLIENT, (ULONG) hwnd );
      }
      else
      {
        UtlSetULong( QL_TWBCLIENT, (ULONG) hwndClient );
      } /* endif */

      SendMessage( hwndClient, WM_MDISETMENU,
                 (WPARAM)(HWND) UtlQueryULong( QL_TWBMENU ),
                 (LPARAM)(HWND) UtlQueryULong( QL_TWBWINDOWMENU ) );

    }
    /************************************************************/
    /* Fill-in IDA fields and address sytem properties          */
    /************************************************************/
    strcpy( pIda->IdaHead.szObjName, TWBMAIN );
    pIda->IdaHead.pszObjName = pIda->IdaHead.szObjName;
    pIda->IdaHead.hFrame = hwnd;
    pIda->hMenu = (HWND) GetMenu( hwnd );

    pIda->hPropSys = EqfQuerySystemPropHnd();
    pIda->pPropSys = GetSystemPropPtr();

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 2.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

    // do necessary startup handling for older Tmgr versions
    if ( pIda->pPropSys->lCodeVersion < CURRENT_CODE_VERSION )
    {
      // Tmgr previous code version 1 requires delete of .STx and .NLx files
      if ( pIda->pPropSys->lCodeVersion < CODE_VERSION_001 )
      {
        PSZ apszExt[5] = { "*.STU", "*.NLU", "*.STC", "*.NLC", NULL };
        int i = 0;

        while ( apszExt[i] != NULL )
        {
          char szSearch[MAX_EQF_PATH]; // buffer for search criteria

          // setup search path
          UtlMakeEQFPath( szSearch, NULC, DIC_PATH, NULL );
          strcat( szSearch, BACKSLASH_STR );
          strcat( szSearch, apszExt[i] );

          // delete all files of this type
          {
            USHORT        usCount = 1;
            HDIR          hSearch = HDIR_CREATE;
            FILEFINDBUF   stFile;
            UtlFindFirst( szSearch, &hSearch, 0, &stFile, sizeof(stFile),
                          &usCount, 0L, FALSE );
            while ( usCount == 1 )
            {
              char szFileName[MAX_EQF_PATH]; // buffer for file name

              UtlMakeEQFPath( szFileName, NULC, DIC_PATH, NULL );
              strcat( szFileName, BACKSLASH_STR );
              strcat( szFileName, RESBUFNAME(stFile) );
              UtlDelete( szFileName, 0L, FALSE );
              UtlFindNext( hSearch, &stFile, sizeof(stFile), &usCount, FALSE );
            } /* endwhile */
            UtlFindClose( hSearch, FALSE );
          }

          // continue with next type of cache list
          i++;
        } /* endwhile */
      } /* endif */

      // update code version in system properties
      if( SetPropAccess( pIda->hPropSys, PROP_ACCESS_WRITE) )
      {
        ULONG ulErrorInfo = 0;
        pIda->pPropSys->lCodeVersion = CURRENT_CODE_VERSION;
        SaveProperties( pIda->hPropSys, &ulErrorInfo );
        ResetPropAccess( pIda->hPropSys, PROP_ACCESS_WRITE);
      } /* endif */
    } /* endif */

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 3.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

    /************************************************************/
    /* force a refresh of QST_VALIDEQFDRIVES string             */
    /************************************************************/
    UtlGetCheckedEqfDrives( szMsgBuf );


#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 4.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif
    /************************************************************/
    /* Register window procedure as TWB handler                 */
    /************************************************************/
    EqfInstallHandler( TWBMAIN, hwnd, clsTWBMAIN);


    /************************************************************/
    /* Start TWB initialization                                 */
    /************************************************************/
    WinPostMsg( hwnd, WM_EQF_INITIALIZE, MP1FROMSHORT(1), NULL);

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 5.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

    /************************************************************/
    /* Correct saved window position if it is larger than the   */
    /* current desktop or if the size is too small              */
    /************************************************************/
    cxScreen = (SHORT)WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    cyScreen = (SHORT)WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );
    if ( pIda->pPropSys->Swp.cx > cxScreen )
    {
      pIda->pPropSys->Swp.cx = cxScreen - 50;
    }
    else if ( pIda->pPropSys->Swp.cx < 100 )
    {
      pIda->pPropSys->Swp.cx = 100;
    } /* endif */

    if ( pIda->pPropSys->Swp.cy > cyScreen )
    {
      pIda->pPropSys->Swp.cy = cyScreen - 50;
    }
    else if ( pIda->pPropSys->Swp.cy < (3 * (SHORT)WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR)) )
    {
      pIda->pPropSys->Swp.cy = 3 * (SHORT)WinQuerySysValue( HWND_DESKTOP,
                                                             SV_CYTITLEBAR );
    } /* endif */

    if ( pIda->pPropSys->Swp.x < 0 )
    {
      pIda->pPropSys->Swp.x = 0L;
    } /* endif */
    if ( (pIda->pPropSys->Swp.x + pIda->pPropSys->Swp.cx) > cxScreen )
    {
      pIda->pPropSys->Swp.x = max( (cxScreen - pIda->pPropSys->Swp.cx), 0 );
    } /* endif */

    if ( pIda->pPropSys->Swp.y < 0 )
    {
      pIda->pPropSys->Swp.y = 0L;
    } /* endif */
    if ( (pIda->pPropSys->Swp.y + pIda->pPropSys->Swp.cy) > cyScreen )
    {
      pIda->pPropSys->Swp.y = max( (cyScreen - pIda->pPropSys->Swp.cy), 0 );
    } /* endif */


    /************************************************************/
    /* restore to saved window size to                          */
    /* let childs correctly size and set their window positions */
    /************************************************************/

    if ( !fMinimize )
    {
      WinSetWindowPos( pIda->IdaHead.hFrame, HWND_TOP,
                       pIda->pPropSys->Swp.x,  pIda->pPropSys->Swp.y,
                       pIda->pPropSys->Swp.cx, pIda->pPropSys->Swp.cy,
                       (USHORT)(pIda->pPropSys->Swp.fs | EQF_SWP_ACTIVATE | EQF_SWP_SHOW
                                | EQF_SWP_SIZE | EQF_SWP_MOVE) );
    }
    else
    {
      WinSetWindowPos( pIda->IdaHead.hFrame, HWND_TOP,
                       pIda->pPropSys->Swp.x,  pIda->pPropSys->Swp.y,
                       pIda->pPropSys->Swp.cx, pIda->pPropSys->Swp.cy,
                       (USHORT)((pIda->pPropSys->Swp.fs & ~ (EQF_SWP_SHOW | EQF_SWP_MAXIMIZE) )
                                 | EQF_SWP_SIZE | EQF_SWP_MOVE ));
    } /* endif */

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 6.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

    /******************************************************************/
    /* Show registry dialogs                                          */
    /* (only if the selected panel language is not Japanese as there  */
    /*  is no Japanese version of Art available)                      */
    /******************************************************************/
    {
      GetStringFromRegistry( APPL_Name, KEY_SYSLANGUAGE, szEqfSysLanguage, sizeof( szEqfSysLanguage ), DEFAULT_SYSTEM_LANGUAGE);
    }
#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 6a.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

    /******************************************************************/
    /* Initialize all handlers                                        */
    /******************************************************************/
    EqfSend2AllHandlers( WM_EQF_INITIALIZE, MP1FROMSHORT(0), 0L );
#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 6b.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

    /************************************************************/
    /* restart previously saved objects (for folderlist,        */
    /* memory list, dictionary list and list list window: start */
    /* default object)                                          */
    /************************************************************/
    pIda->szObject[0] = X15;
    UtlMakeEQFPath( pIda->szObject + 1, NULC, SYSTEM_PATH, NULL );
    strcat( pIda->szObject, BACKSLASH_STR );
    strcat( pIda->szObject, DEFAULT_FOLDERLIST_NAME );
    SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                 WM_MDISETMENU,
                 (WPARAM)(HWND) UtlQueryULong( QL_TWBMENU ),
                 (LPARAM)(HWND) UtlQueryULong( QL_TWBWINDOWMENU ) );
#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 6c.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif
    DrawMenuBar( (HWND)UtlQueryULong( QL_TWBFRAME ) );
#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 6d.                         %s   [%s]", asctime( localtime( &lCurTime ) ),pIda->szObject );
  } /* endif */
#endif
    TwbRestart( FOLDERLISTHANDLER, pIda->szObject );
#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 6e.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

    // delete any folder list item under file pulldown
    DeleteMenu( GetSubMenu( (HMENU)UtlQueryULong( QL_TWBMENU ), 0 ),
                ID_FLIST_WINDOW, MF_BYCOMMAND );
#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 6f.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

    /*************************************************************/
    /* delete toolbar selection from VIEW -- it is starting at   */
    /* the 11th position wiht the SEPARATOR - keep in mind we are */
    /* zero based.                                               */
    /*************************************************************/
    {
      HMENU hwndMenu;
      hwndMenu = GetSubMenu( (HMENU)UtlQueryULong( QL_TWBMENU ), PID_TWBM_SM_VIEW );
      if ( hwndMenu )
      {
        DeleteMenu( hwndMenu, 12, MF_BYPOSITION );
        /**************************************************************/
        /* delete SEPARATOR                                           */
        /**************************************************************/
        DeleteMenu( hwndMenu, 11, MF_BYPOSITION );
      } /* endif */
    } /* endif */

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 7.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif


    pIda->szObject[0] = X15;
    UtlMakeEQFPath( pIda->szObject + 1, NULC, SYSTEM_PATH, NULL );
    strcat( pIda->szObject, BACKSLASH_STR );
    strcat( pIda->szObject, MEMORY_PROPERTIES_NAME );
    TwbRestart( MEMORYHANDLER, pIda->szObject );

    TwbRestart( FOLDERHANDLER, pIda->pPropSys->RestartFolders );

    pIda->szObject[0] = X15;
    UtlMakeEQFPath( pIda->szObject + 1, NULC, SYSTEM_PATH, NULL );
    strcat( pIda->szObject, BACKSLASH_STR );
    strcat( pIda->szObject, DICT_PROPERTIES_NAME );

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 8.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif


#ifndef _TLEX
#ifndef _TQM

    TwbRestart( DICTIONARYHANDLER, pIda->szObject );

#endif
#endif


#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 9.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

#ifndef _TQM
#ifndef _TLEX
    TwbRestart( TAGTABLEHANDLER, pIda->pPropSys->RestartTagTables );

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 10.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

    TwbRestart( LISTHANDLER, pIda->pPropSys->RestartLists );

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 11.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif
    TwbRestart( MTLISTHANDLER, pIda->pPropSys->RestartMTList );
    pIda->pPropSys->RestartMTList[0] = EOS; // clear this entry

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 12.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif
#endif
#endif


    /******************************************************************/
    /* Save only when all lists have been started                     */
    /******************************************************************/
    fSaveOnClose = TRUE;


    /******************************************************************/
    /* set final MAT window state                                     */
    /******************************************************************/
    if ( fMinimize )
    {
      ShowWindow( pIda->IdaHead.hFrame, SW_SHOWMINNOACTIVE );
    } /* endif */

    /******************************************************************/
    /* bring-up any saved translation processor session               */
    /* and remove TPRO restart objects from restart data              */
    /******************************************************************/

#ifndef _TQM
#ifndef _TLEX
    TwbRestart( DOCUMENTHANDLER,   pIda->pPropSys->RestartDocs );
#endif
#endif

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 13.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

    if( SetPropAccess( pIda->hPropSys, PROP_ACCESS_WRITE))
    {
      *pIda->pPropSys->RestartDocs    = NULC;
      ResetPropAccess( pIda->hPropSys, PROP_ACCESS_WRITE);
    } /* endif */

    /******************************************************************/
    /* Activate focus object                                          */
    /******************************************************************/
    if ( pIda->pPropSys->FocusObject[0] != NULC )
    {
       hwndFocus = EqfQueryObject( pIda->pPropSys->FocusObject, clsANY, 0 );
       if ( hwndFocus )
       {
          SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT), WM_MDIACTIVATE,
                       MP1FROMHWND(hwndFocus), 0L );
       } /* endif */
    } /* endif */

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 14.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

    // open system preferenced dialog if no default target language
    // has been specified
    {
      PPROPSYSTEM  pPropSys = GetSystemPropPtr();
      if ( !pPropSys->szSystemPrefLang[0])
      {
        TWBSystemProps();
      } /* endif */
    }


  } /* endif */

#ifdef STARTUP2_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "------ OpenTM2 startup2 end:           %s", asctime( localtime( &lCurTime ) ) );
    fclose( hStartLog ) ;
  } /* endif */
#endif

  return( mResult );

} /* end of function TwbCreateMsg */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TwbInitMenu            Process WM_INITMENU message of Twb
//------------------------------------------------------------------------------
// Function call:     TwbInitMenu( HWND hwnd, MPARAM mp1, MPARAM mp2 );
//------------------------------------------------------------------------------
// Description:       Process the intit menu message for the Twb actionbar.
//                    Either process the messs
//------------------------------------------------------------------------------
// Input parameter:   HWND   hwnd
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       FALSE (always)
//------------------------------------------------------------------------------
// Function flow:     switch pulldown id
//                      case help pulldown
//                        do nothing
//                      case windows pulldown
//                        delete all window entries in pulldown
//                        get list of active objects
//                        add active objects to pulldown
//                      default
//                        pass message to active object
//                        enable specific utilities
//                    endswitch
//------------------------------------------------------------------------------
MRESULT TwbInitMenuMFC( HMENU hwndMenu, WPARAM mp1, LPARAM mp2 )
{
  if ( hwndMenu )
  {
    OBJNAME       szMenuName;          // name to insert into menu
    OBJNAME       szObjName;           // name of active objects
    HWND          hwndObj;             // handle of object window
    USHORT        usWindow;            // number of window for window menu
    USHORT        usID;
    USHORT        usI;
    USHORT        usAttribute;
    POBJLST       pObject = NULL;        // pointer for object list processing
    POBJLST       pObjectStart = NULL;   // pointer for object list processing
    BOOL          fSeparator = FALSE;
    USHORT        usItems = (USHORT)GetMenuItemCount( hwndMenu );
    int           i = 0;

    PTWBMAIN_IDA  pIda = ACCESSWNDIDA( (HWND) mp1, PTWBMAIN_IDA );

    while ( i < usItems )
    {
      usID = (USHORT)GetMenuItemID( hwndMenu, i );
      if ( (usID == 0xFFFF) || (usID == 0) )   // separator
      {
        fSeparator = TRUE;
        DeleteMenu( hwndMenu, i, MF_BYPOSITION );
        usItems = (USHORT) GetMenuItemCount( hwndMenu );
      }
      else if ( fSeparator )
      {
        DeleteMenu( hwndMenu, usID, MF_BYCOMMAND );
        usItems = (USHORT)GetMenuItemCount( hwndMenu );
      }
      else
      {
        i++;
      } /* endif */
    } /* endwhile */

    AppendMenu( hwndMenu, MF_SEPARATOR, 0xFFFF, "" );

    if ( pIda->hwndsInMenuWindows )
    {
       UtlAlloc( (PVOID *) &pIda->hwndsInMenuWindows, 0L, 0L, NOMSG );
    } /* endif */
    pIda->usInMenuWindows = 0;

    usI = EqfQueryObjectCount( clsANY );
    if( !usI )
    {
       return( FALSE );
    } /* endif */

    UtlAlloc( (PVOID *) &pObjectStart,
              0L, (LONG) ((usI * sizeof( *pObject)) + 20), ERROR_STORAGE );
    if ( pObjectStart )
    {
      BOOL fClsFolder = FALSE;
      pObject = pObjectStart;
      pIda->hwndsInMenuWindows = pObject;
      pIda->usInMenuWindows = usI;

      usI = EqfGetObjectList( clsANY, usI, pObject);

      usWindow = 1;                 // start with window number 1
      usID     = PID_WIND_MI_NEXT;  // this is our number
      for ( ; usI; usI--, pObject++, usID++ )
      {
        if ( fClsFolder &&
             ((pObject->usClassID == clsFOLDER) || (pObject->usClassID == clsFOLDERLIST)))
        {
          /************************************************************/
          /* ignore entry --- already displayed                       */
          /************************************************************/
        }
        else
        {

         if ( IDFROMWINDOW( pObject->hwnd ) == FID_CLIENT )
         {
             // a standard window ...
             hwndObj = GETPARENT( pObject->hwnd );
             WinQueryWindowText( hwndObj, sizeof( szObjName), (PSZ)szObjName);
         }
         else
         {
             // a dialog window ...
             hwndObj = pObject->hwnd;
             WinQueryWindowText( hwndObj, sizeof( szObjName), (PSZ)szObjName);
             if ( szObjName[0] == EOS )
             {
               WinQueryWindowText( GetParent( hwndObj ), sizeof( szObjName), (PSZ)szObjName);
             } /* endif */
         } /* endif */
         usAttribute  = (((HWND)mp2 == pObject->hwnd) ||
                         ((HWND)mp2 == GetParent(pObject->hwnd)) ||
                         ((HWND)mp2 == GetParent(GetParent(pObject->hwnd)))) ? MF_CHECKED : 0;
         usAttribute |= WinIsWindowEnabled( pObject->hwnd) ? MF_ENABLED : 0;
         if ( usWindow <= 9 )
         {
            sprintf( szMenuName, "&%d) %s", usWindow++, szObjName );
         }
         else
         {
            strcpy( szMenuName, szObjName );
         } /* endif */
         AppendMenu( hwndMenu, usAttribute, usID, szMenuName );
        } /* endif */

        if ( fIELookAndFeel &&
             ((pObject->usClassID == clsFOLDER) || (pObject->usClassID == clsFOLDERLIST)))
        {
          /************************************************************/
          /* remember, that folder already appeared once              */
          /************************************************************/
          fClsFolder = TRUE;
        }
      } /* endfor */
    } /* endif */
  } /* endif */
  return FALSE;
}


MRESULT TwbInitMenu( HWND hwnd, WPARAM mp1, LPARAM mp2 )
{
    PTWBMAIN_IDA  pIda;                // pointer to IDA
    CHAR          szEngineString[200]; // MT Engine in Win.ini

   pIda = ACCESSWNDIDA( hwnd, PTWBMAIN_IDA );


   switch ( SHORT1FROMMP1( mp1) )
   {
      case PID_TWBM_SM_HELP:
         // do nothing; leave help items active ...

         // GQ 2015/10/13: disable some of the help items until out help system is working again
         UtlMenuDisableItem( PID_HELP_FOR_HELP );
         UtlMenuDisableItem( PID_HELP_MI_INDEX );
         break;

      case PID_UTILS_SM_PANELLANG:
         // do nothing; leave panel languages active ...
         break;

      // Windows: the windows pulldown is maintained by the MDI client procedure
      case PID_TWBM_SM_WINDOWS:
         break;

       default:
         /*************************************************************/
         /* remove report handler menu item from pulldown if          */
         /* report handler is not active                              */
         /*************************************************************/
         if ( (SHORT1FROMMP1( mp1) == PID_TWBM_SM_UTILITIES) &&
              !fReportHandler )
         {
           HMENU hwndMenu = GETMENU((HWND)UtlQueryULong( QL_TWBFRAME ));


           DeleteMenu( hwndMenu, PID_UTILS_MI_REPORT, MF_BYCOMMAND );
         } /* endif */



         GetStringFromRegistry( APPL_Name, KEY_MTEngine, szEngineString, sizeof(szEngineString ), "" );

         if ( (SHORT1FROMMP1( mp1) == PID_TWBM_SM_UTILITIES) &&
              !szEngineString[0] )
         {
           HMENU hwndMenu = GETMENU((HWND)UtlQueryULong( QL_TWBFRAME ));


           DeleteMenu( hwndMenu, PID_UTILS_MI_MT, MF_BYCOMMAND );
         } /* endif */

         //--- leave it to active instance to set menu items ---
         if ( pIda->hObjFocus )
         {
           WinSendMsg( pIda->hObjFocus, WM_EQF_INITMENU, mp1, mp2);
         } /* endif */

         //--- ensure that some of the items are enabled anyway ---
         UtlMenuEnableItem( PID_UTILS_MI_TAGTABLE );
         UtlMenuEnableItem( PID_UTILS_MI_LNGUPDATE );
         UtlMenuEnableItem( PID_UTILS_MI_DRIVES );
         UtlMenuEnableItem( PID_UTILS_MI_PLGINMGR);
         UtlMenuEnableItem( PID_UTILS_MI_ATOVERUP );
         UtlMenuEnableItem( PID_TERMLISTS_POPUP );
         UtlMenuEnableItem( PID_UTILS_MI_EXCLUSION );
         UtlMenuEnableItem( PID_UTILS_MI_NEWTERMS );
         UtlMenuEnableItem( PID_UTILS_MI_FOUNDTERMS );
         UtlMenuEnableItem( PID_UTILS_MI_ABBR );
         UtlMenuEnableItem( PID_UTILS_SM_PANELLANG );
         UtlMenuEnableItem( PID_FILE_MI_SYSPROP );
         UtlMenuEnableItem( PID_FILE_MI_EXIT );
         UtlMenuEnableItem( PID_UTILS_MI_MT );

         /*************************************************************/
         /* Enable all menu items required by user handlers           */
         /*************************************************************/
         if ( usUserHandler )
         {
           USHORT i, j;                 // loop indices
           for ( i = 0; i < usUserHandler; i++ )
           {
             j = 0;
             while ( pUserHandler[i].ausMenuIDs[j] )
             {
               UtlMenuEnableItem( pUserHandler[i].ausMenuIDs[j] );
               j++;
             } /* endwhile */
           } /* endfor */
         } /* endif */

         break;
   } /* endswitch */

   return( FALSE );
} /* end of function TwbInitMenu */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TwbGetCheckProfileData
//------------------------------------------------------------------------------
// Function call:     TwbGetCheckProfileData();
//------------------------------------------------------------------------------
// Description:       Get MAT profile data from OS2.INI file and check the
//                    received values.
//------------------------------------------------------------------------------
// Input parameter:   VOID
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   successful
//                    FALSE  profile entries corrupted
//------------------------------------------------------------------------------
// Function flow:     Query profile values
//                    Check values and handle errors
//                    Set system variables
//------------------------------------------------------------------------------
BOOL TwbGetCheckProfileData( PSZ pEqfSystemMsgFile, PSZ pEqfSystemPropPath,
                             PSZ pEqfResFile )
{
    CHAR   szDrive[MAX_DRIVE];         // buffer for drive list
    CHAR   szLanDrive[MAX_DRIVE];      // buffer for lan drive list
    CHAR   szSysPath[MAX_EQF_PATH];    // buffer for system path
    BOOL   fOK = TRUE;                 // internal OK flag

    // Get system drive and system path
    GetStringFromRegistry( APPL_Name, KEY_Drive, szDrive, sizeof( szDrive  ), "" );
    GetStringFromRegistry( APPL_Name, KEY_LanDrive, szLanDrive, sizeof( szLanDrive  ), "" );
    GetStringFromRegistry( APPL_Name, KEY_Path, szSysPath, sizeof( szSysPath), "" );
    sprintf( EqfSystemPath, "%s\\%s",  szDrive, szSysPath );

    // Get name of system property file
    GetStringFromRegistry( APPL_Name, KEY_SysProp, EqfSystemPropPath, sizeof( EqfSystemPropPath ), "" );

    // Get name of current language
    GetStringFromRegistry( APPL_Name, KEY_SYSLANGUAGE, szEqfSysLanguage, sizeof( szEqfSysLanguage ), DEFAULT_SYSTEM_LANGUAGE );

    // Set name of resource, help file and message file for selected language
    fOK = UtlQuerySysLangFile( szEqfSysLanguage, szEqfResFile,
                               EqfSystemHlpFile, EqfSystemMsgFile );


    /******************************************************************/
    /* Check if resources exist                                       */
    /* (The resource file is not checked here, it will be checked     */
    /*  later on while loading the resource into memory)              */
    /******************************************************************/
    if ( fOK )
    {
      fOK = UtlFileExist( EqfSystemHlpFile );
    } /* endif */

    if ( fOK && szDrive[0] && szSysPath[0] && EqfSystemPropPath[0] &&
         EqfSystemHlpFile[0] && szEqfResFile[0] )
    {
      UtlSetString( QST_MSGFILE, EqfSystemMsgFile );
      UtlSetString( QST_HLPFILE, EqfSystemHlpFile );
      UtlSetString( QST_RESFILE, szEqfResFile );
    }
    else
    {
      WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, STR_INSTALL_ERROR,
                     STR_INSTALL_ERROR_TITLE,
                     1, MB_ENTER | MB_ICONEXCLAMATION);
      fOK = FALSE;
    } /* endif */

    /******************************************************************/
    /* return strings to caller if requested...                       */
    /******************************************************************/
    if ( fOK )
    {
      if ( pEqfSystemMsgFile )
      {
        strcpy( pEqfSystemMsgFile, EqfSystemMsgFile );
      } /* endif */
      if ( pEqfSystemPropPath )
      {
        strcpy( pEqfSystemPropPath, EqfSystemPropPath );
      } /* endif */
      if ( pEqfResFile )
      {
        strcpy( pEqfResFile, szEqfResFile );
      } /* endif */
    } /* endif */

    return( fOK);

} /* end of function TwbGetCheckProfileData */



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TwbRestart             Restart objects of a MAT handler
//------------------------------------------------------------------------------
// Function call:     TwbRestart( PSZ pszHandler, PSZ pszRestartList );
//------------------------------------------------------------------------------
// Description:       Restart all objects listed in the restart list.
//                    The objects are seperated by X15.
//------------------------------------------------------------------------------
// Input parameter:   PSZ pszHandler       Name of handler for the objects
//                    PSZ pszRestartList   restart list containing object names
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Prerequesits:      The handler must have been started already.
//------------------------------------------------------------------------------
// Function flow:     create copy of restart list
//                    while not end of restart list
//                      seperate next object name
//                      start object by sending WM_EQF_OPEN to the handler
//                      dispatch messages to allow start of object
//                    endwhile
//                    free copy of restart list
//------------------------------------------------------------------------------
VOID TwbRestart
(
  PSZ pszHandler,                      // Name of handler for the objects
  PSZ pszRestartList                   // restart list containing object names
)
{
   PSZ   pszList;                      // pointer to copy of restart list
   PSZ   pszObject;                    // pointer to current object in list
   PSZ   pszNextObject;                // pointer to next object in restart list

   UtlAlloc( (PVOID *) &pszList, 0L, (LONG) max( MIN_ALLOC, strlen( pszRestartList ) + 1),
             ERROR_STORAGE );
   if ( pszList )
   {
     strcpy( pszList, pszRestartList );

     pszObject = strchr( pszList, X15 );
     while( pszObject )
     {
        /****************************************************************/
        /* seperate object name                                         */
        /****************************************************************/
        pszObject++;
        pszNextObject = strchr( pszObject, X15 );
        if ( pszNextObject )
        {
           *pszNextObject = EOS;
        } /* endif */

        /****************************************************************/
        /* open the object                                              */
        /****************************************************************/
        EqfSend2Handler( pszHandler, WM_EQF_OPEN, MP1FROMSHORT(TRUE), MP2FROMP(pszObject) );
        UtlDispatch();                   // wait for object to come up
        pszObject = pszNextObject;
     } /* endwhile */

     UtlAlloc( (PVOID *) &pszList, 0L, 0L, NOMSG );

   } /* endif */

} /* end of function TwbRestart */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TwbSwitchHelpTable
//------------------------------------------------------------------------------
// Function call:     TwbSwitchHelpTable( PHELPSUBTABLE phlpsubtblTwb,
//                                        PHELPSUBTABLE phlpsubtblChangeView );
//------------------------------------------------------------------------------
// Description:       Switch the help tables for the Twb action bar helps and
//                    the change view details help.
//------------------------------------------------------------------------------
// Input parameter:   PHELPSUBTABLE phlpsubtblTwb          Twb help table
//                    PHELPSUBTABLE phlpsubtblChangeView   table for view
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE (always)
//------------------------------------------------------------------------------
// Side effects:      The specified help tables are activated.
//------------------------------------------------------------------------------
// Function flow:     search entry for Twb help
//                    if found then
//                      replace help table found with specified help table
//                    endif
//                    search entry for view details dialog
//                    if found then
//                      replace help table found with specified help table
//                    endif
//                    activate the new help table
//------------------------------------------------------------------------------
BOOL TwbSwitchHelpTable
(
   PHELPSUBTABLE phlpsubtblTwb,        // table to be used for Twb helps
   PHELPSUBTABLE phlpsubtblChangeView  // table to be used for 'Change View' dlg
)
{
   PHELPTABLE phlptblEntry;            // pointer into main help table

   /*******************************************************************/
   /* change Twb table                                                */
   /*******************************************************************/
   phlptblEntry = htblMain;            // start at begin of main help table

   /*******************************************************************/
   /* Search help table for Twb action bar                            */
   /*******************************************************************/
   while ( phlptblEntry->idAppWindow &&                     // while not eof
           (phlptblEntry->idAppWindow != ID_TWBM_WINDOW ) ) // and not found ...

   {
      phlptblEntry++;                  // ... loop thru table
   } /* endwhile */

   /*******************************************************************/
   /* If found replace current help table with the specified one      */
   /*******************************************************************/
   if ( phlptblEntry->idAppWindow == ID_TWBM_WINDOW ) // if found ...
   {
      phlptblEntry->phstHelpSubTable = phlpsubtblTwb;// ... set new sub table
   } /* endif */

   /*******************************************************************/
   /* change Change View Dialog table                                 */
   /*******************************************************************/
   phlptblEntry = htblMain;            // start at begin of main help table

   /*******************************************************************/
   /* Search help table for select view details dialog                */
   /*******************************************************************/
   while ( phlptblEntry->idAppWindow &&                     // while not eof
           (phlptblEntry->idAppWindow != ID_SELVIEW_DLG ) ) // and not found ...

   {
      phlptblEntry++;                  // ... loop thru table
   } /* endwhile */

   /*******************************************************************/
   /* If found replace current help table with the specified one      */
   /*******************************************************************/
   if ( phlptblEntry->idAppWindow == ID_SELVIEW_DLG ) // if found ...
   {
      phlptblEntry->phstHelpSubTable = phlpsubtblChangeView;// set new table
   } /* endif */

   return( TRUE );
} /* end of function TwbSwitchHelpTable */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TwbStartHandler        Start a MAT handler
//------------------------------------------------------------------------------
// Function call:     TwbStartHandler( HAB hab, PSZ pszHandler,
//                                     PFNWP pfnWndProc );
//------------------------------------------------------------------------------
// Description:       Register handler class and create handler object window.
//------------------------------------------------------------------------------
// Input parameter:   HAB   hab              anchor block handle
//                    PSZ   pszHandler       name of handler being started
//                    PFNWP pfnWndProc       handler window procedure
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE                function completed OK
//                    FALSE               error starting handler
//------------------------------------------------------------------------------
// Side effects:      Handler window procedure gets a WM_CREATE message.
//------------------------------------------------------------------------------
// Function flow:     register handler window class
//                    if ok then
//                      create handler object window
//                    endif
//                    if ok then
//                      call UtlDispatch to allow handler startup
//                    else
//                      issue error message
//                    endif
//                    return ok flag
//------------------------------------------------------------------------------
BOOL TwbStartHandler
(
   HAB   hab,                          // anchor block handle
   PSZ   pszHandler,                   // name of handler being started
   PFNWP pfnWndProc,                   // handler window procedure
   PHWND phwnd                         // ptr to buffer for handler window handle
)
{
   BOOL fOK;                           // function return code
   HWND hwnd = NULL;                   // window handle of handle object window

   /*******************************************************************/
   /* Register handler window class                                   */
   /*******************************************************************/
   fOK = WinRegisterClass( (HINSTANCE) hab, pszHandler, pfnWndProc, 0L, sizeof(PVOID) );

   /*******************************************************************/
   /* Create handler object window                                    */
   /*******************************************************************/
   if ( fOK )
   {
      hwnd = WinCreateWindow( HWND_OBJECT,
                              pszHandler, pszHandler,
                              WS_OVERLAPPEDWINDOW,   // window style(s)
                              0,0,0,0,
                              HWND_DESKTOP,
                              HWND_TOP,
                              0, 0, 0 );
       fOK = ( hwnd != NULLHANDLE );
   } /* endif */

   /*******************************************************************/
   /* Post-processing and error handling                              */
   /*******************************************************************/
   if ( fOK )
   {
      UtlDispatch();                   //  let the started handler come up
   }
   else
   {
      UtlError( ERROR_DID_NOT_START, MB_CANCEL, 1, &pszHandler, EQF_ERROR );
   } /* endif */

   if ( fOK && phwnd )
   {
     *phwnd = hwnd;
   } /* endif */

   return ( fOK );
} /* end of function TwbStartHandler */

BOOL TwbStartListHandler( PSZ pszName, PFN_HANDLERCALLBACK pfnCallBack, PHWND phwnd )
{
   BOOL fOK = TRUE;                    // function return code
   HWND hwnd;                          // window handle of handle object window
   GENHANDLERCREATEPARMS CreateParms;

   CreateParms.usSize = sizeof(CreateParms);
   CreateParms.pfnCallBack = pfnCallBack;

   hwnd = WinCreateWindow( HWND_OBJECT,
                           GENERICHANDLER, GENERICHANDLER,
                           WS_OVERLAPPEDWINDOW,   // window style(s)
                           0,0,0,0,
                           HWND_DESKTOP,
                           HWND_TOP,
                           0,
                           (PVOID)&CreateParms,
                           NULL );
   fOK = ( hwnd != NULLHANDLE );

   /*******************************************************************/
   /* Post-processing and error handling                              */
   /*******************************************************************/
   if ( fOK )
   {
      UtlDispatch();                   //  let the started handler come up
      if ( phwnd ) *phwnd = hwnd;
   }
   else
   {
      PSZ pszParm = pszName;
      UtlError( ERROR_DID_NOT_START, MB_CANCEL, 1, &pszParm, EQF_ERROR );
   } /* endif */

   return ( fOK );
} /* end of function TwbStartListHandler */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TwbCloseCheck          Handle close fot TM/2 workbench
//------------------------------------------------------------------------------
// Function call:     TwbCloseCheck ( );
//------------------------------------------------------------------------------
// Description:       Get user confirmation for close. Close workbench or
//                    cancel shutdown
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE                Close has been canceled
//                    FALSE               close can continue
//------------------------------------------------------------------------------
// Function flow:     get user close information
//                    if close not canceled
//                      call EqfStopObjectManager to close the workbench
//                    if close was canceled
//                      call WinCancelShutdown to inform the desktop manager
//------------------------------------------------------------------------------
BOOL TwbCloseCheck( VOID )
{
  BOOL    fCanceled = TRUE;            // close canceled flag
  USHORT  usFlags;                     // TM/2 close flags

  /********************************************************************/
  /* return immediately since we've done our checking already         */
  /********************************************************************/
  if ( fCloseCheckDone )
  {
    return FALSE;        // we are ready to stop (and have been here already)
  } /* endif */


  /******************************************************************/
  /* do not save status if window is minimized                      */
  /******************************************************************/

    fNotMiniMized = !IsIconic( (HWND)UtlQueryULong( QL_TWBFRAME ) );

  /**************************************************************/
  /* Get close confirmation and save status                     */
  /**************************************************************/
#ifdef _TQM
  // TQM needs no close confirmation and saves always (if not minimized)
  if ( fSaveOnClose && fNotMiniMized )
  {
    usFlags = MBID_YES;
  }
  else
  {
    usFlags = MBID_NO;
  } /* endif */
#else
  if ( fSaveOnClose && fNotMiniMized )
  {
    usFlags = UtlError( QUERY_SAVECLOSE_TWB, MB_YESNOCANCEL, 0,
                        (PSZ *) NULP, EQF_QUERY);
  }
  else
  {
    usFlags = UtlError( QUERY_CLOSE_TWB, MB_YESNO | MB_DEFBUTTON2,
                        0, (PSZ *) NULP, EQF_QUERY);
    if (usFlags == MBID_NO)
    {
      usFlags = MBID_CANCEL;
    } /* endif */
    if (usFlags == MBID_YES)
    {
      usFlags = MBID_NO;
    } /* endif */
  } /* endif */
#endif

  /**************************************************************/
  /* if not canceled, shutdown the workbench                    */
  /* (terminating the object manager will WM_EQF_TERMINATE all  */
  /* active handlers and objects)                               */
  /**************************************************************/
  if( usFlags != MBID_CANCEL)
  {
    usFlags = (usFlags == MBID_YES) ? TWBSAVE | TWBCLOSE : TWBCLOSE;
    if( EqfStopObjectManager( usFlags) == NULLHANDLE )
    {
      fCanceled = FALSE;               // close can continue
    } /* endif */
    {
        HANDLE hMapObject = NULL;
        hMapObject = OpenFileMapping (FILE_MAP_WRITE, FALSE, EQFNDDE_SHFLAG );
        if (hMapObject)
        {
            CloseHandle(hMapObject);
        }
    }
  } /* endif */

  return( fCanceled );
}
/****************************************************************************
        HelpMessageFilterHook():
****************************************************************************/
DWORD CALLBACK HelpMessageFilterHook
(
  int              nCode,
  WORD             wParam,
  LPMSG            lpMsg
)
{
  if ( (nCode >= 0) && (lpMsg != NULL) )
  {
    if ( (lpMsg->message == WM_KEYDOWN) && (lpMsg->wParam == VK_F1) )
    {
      switch ( nCode )
      {
        case MSGF_DIALOGBOX :
          {
            HWND hTemp = NULL;
            HWND hParent = lpMsg->hwnd;
            SHORT   sControlID = 0;
            SHORT   sParentID  = 0;

            // Check for error message box first ...
            sParentID = (SHORT)UtlQueryUShort( QS_CURMSGID );

            if ( sParentID == 0 )          // not in a message box ???
            {
              CHAR szClass[40];          // buffer for class names

              /**********************************************************/
              /* First of all get handle of dialog window               */
              /* Note: This is not always the immediate parent of the   */
              /*       control receiving the message (for controls      */
              /*       consisting of several base controls; e.g.        */
              /*       comboboxes, spinbuttons, ...)                    */
              /**********************************************************/
              while ( hParent != NULL )
              {
                hTemp = hParent;

                if ( !(GetWindowLong( hParent, GWL_STYLE ) & WS_CHILD) )
                {
                  /******************************************************/
                  /* Seems to be a real parent window ==> leave the loop*/
                  /******************************************************/
                  break;
                }
                else if ( GetClassName( hParent, szClass, sizeof(szClass) ) )
                {
                  if ( stricmp( szClass, WC_EQFMDIDLG ) == 0 )
                  {
                    break;               // found a MDI dialog ==> leave loop
                  }
                  else
                  {
                    /****************************************************/
                    /* Normal child control ==> get ID of control and   */
                    /* continue with control's parent                   */
                    /****************************************************/
                    sControlID = (SHORT)GetWindowLong( hParent, GWL_ID );
                    hParent = GetParent( hParent );
                  } /* endif */
                } /* endif */
              } /* endwhile */

              if ( hParent != NULL )
              {
                SendMessage( hParent, WM_EQF_QUERYID, 0, MP2FROMP(&sParentID) );
              } /* endif */
            } /* endif */

            PostMessage( hTwb,
                         HM_HELPSUBITEM_NOT_FOUND,
                         0,
                         MP2FROM2SHORT( sParentID, sControlID ) );
          }
          break;

        case MSGF_MENU :
          {
            SHORT   sParentID  = (SHORT)UtlQueryUShort( QS_CURMENUID );
            SHORT   sControlID = (SHORT)UtlQueryUShort( QS_CURMENUITEMID );

            PostMessage( hTwb,
                         HM_HELPSUBITEM_NOT_FOUND,
                         0,
                         MP2FROM2SHORT( sParentID, sControlID ) );
          }
          break;
        default:
          break;
      } /* endswitch */
    } /* endif */
  } /* endif */

  return CallNextHookEx( MessageFilterHook, nCode, wParam, (LONG)lpMsg);
}


// Enumeration callback procedure for TWB child windows
BOOL FAR PASCAL ChildEnumProc
(
  HWND             hwnd,               // handle of child window
  LPARAM           mp2                 // command value
)
{
  HWND  hwndMDIClient = (HWND) UtlQueryULong(QL_TWBCLIENT);

  /********************************************************************/
  /* pass on only messages for our MDI Childs - not for our listboxes */
  /* etc...                                                           */
  /********************************************************************/
  if ( hwndMDIClient == GetParent(hwnd) )
  {
    LONG           lStyle;             // buffer for window style

    /******************************************************************/
    /* Check for MDI window with dialog frame (e.g. LOOKUP dialog)    */
    /* and do ignore this type of MDI window                          */
    /******************************************************************/
    lStyle = GetWindowLong( hwnd, GWL_EXSTYLE );
    if ( lStyle & WS_EX_DLGMODALFRAME )
    {
      /****************************************************************/
      /* Ignore dialog type MDI windows                               */
      /****************************************************************/
    }
    else
    {
      /****************************************************************/
      /* Minimize or restore window                                   */
      /****************************************************************/
      switch ( SHORT1FROMMP2(mp2) )
      {
        case PID_WIND_MI_MINALL :
          /**************************************************************/
          /* Skip the icon title windows -- the owner of the icon-title */
          /* Note: an MID Client window does not have an owner          */
          /*       the icon-title, which under windows is a separately  */
          /*       defined window has an owner - the MDI document client*/
          /**************************************************************/
          if ((!IsIconic(hwnd)) && (!GetWindow( hwnd, GW_OWNER )) )
          {
            ShowWindow( hwnd, SW_MINIMIZE );
          } /* endif */
          break;
        case PID_WIND_MI_RESTOREALL :
          if ( (IsIconic(hwnd) || IsZoomed(hwnd)) )
          {
            SendMessage( hwndMDIClient, WM_MDIRESTORE,
                         MP1FROMHWND(hwnd), 0L );
          } /* endif */
          break;
      } /* endswitch */
    } /* endif */
  } /* endif */

  return ( TRUE );

} /* end of function ChildEnumProc */


//*************************************************************
//
//  MyNewClientProc ()
//
//  Purpose:
//      The window function for superclassed client window
//
//  Parameters:
//      ( HWND, WORD, WORD, LONG)
//
//  Return: (LONG)
//
//*************************************************************
LONG FAR PASCAL NewMDIClientProc (register HWND hwnd, UINT msg,
                                 register WPARAM mp1, LPARAM mp2)
{
static HBITMAP hbm=NULL;
static BOOL bSizeChanged=FALSE;

  switch (msg)
  {
    case WM_CREATE:
      /****************************************************************/
      /* Load any background bitmap specified in the configuration    */
      /* file                                                         */
      /****************************************************************/
      GetStringFromRegistry( APPL_Name, KEY_BG_BITMAP, szMsgBuf, sizeof(szMsgBuf), "NO" );
      if ( stricmp( szMsgBuf, "YES" ) == 0 )
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        hbm = LoadBitmap( hResMod, MAKEINTRESOURCE(ID_BACKGROUND_BMP) );
      } /* endif */
      break;

    case WM_DESTROY:        // clean up
      if ( hbm )
         DeleteObject( hbm );
      break;

    case WM_SIZE:
      {
        static WORD cxClient=0, cyClient=0;

        // if the app is just starting up, save the window
        // dimensions and get out

        if ((cxClient==0) && (cyClient==0))
        {
          cxClient = LOWORD(mp2);
          cyClient = HIWORD(mp2);
          break;
        }

        // if the size hasn't changed, break and pass to default

        if ((LOWORD(mp2) == cxClient) && (HIWORD(mp2) == cyClient))
            break;

        // window size has changed so save new dimensions and force
        // entire background to redraw, including icon backgrounds

        cxClient = LOWORD(mp2);
        cyClient = HIWORD(mp2);

        RedrawWindow(hwnd, NULL, NULL,
            RDW_INVALIDATE | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN );

        return 0;
      }

    case WM_ERASEBKGND:
    {
      HBITMAP hbmOld;
      BITMAP bm;
      HBRUSH hbr, hbrOld;
      RECT rc;
      HDC hdcMem, hdc = (HDC)mp1;

      // If there is no bitmap, for whatever reason, let the
      // default proc erase the background normally.

      if (!hbm)
          break;

      // Fill client area with background brush.
      // This isn't necessary if the bitmap takes up the
      // entire window.

      GetClientRect( (HWND)UtlQueryULong( QL_TWBCLIENT ), &rc );
      if ((hbr = (HBRUSH)GetClassLong(hwnd, GCL_HBRBACKGROUND)) != NULL)
      {
        hbrOld = (HBRUSH) SelectObject(hdc, hbr);
        PatBlt(hdc, 0, 0, rc.right, rc.bottom, PATCOPY);
        if (hbrOld)
          SelectObject(hdc, hbrOld);
      }

      // Blt bitmap to the center of the window.
      // Could use StretchBlt to size the bitmap to
      // the window, but it's slow.

      GetObject(hbm, sizeof(BITMAP), &bm);
      hdcMem = CreateCompatibleDC(hdc);
      hbmOld = (HBITMAP) SelectObject(hdcMem, hbm);

      BitBlt(hdc,
          (rc.right-bm.bmWidth)>>1,   // centered
          (rc.bottom-bm.bmHeight)>>1,
          bm.bmWidth,
          bm.bmHeight,
          hdcMem,
          0, 0,
          SRCCOPY);

      // clean up

      if (hbmOld)
          SelectObject(hdcMem, hbmOld);
      DeleteDC(hdcMem);

      return 1;
    }

    case WM_RBUTTONDOWN:
      /* Draw the "floating" popup in the app's client area */
      {
        RECT rc;
        POINT pt;
        GetClientRect( hwnd, (LPRECT)&rc);
        pt.x = LOWORD(mp2);
        pt.y = HIWORD(mp2);
        if (PtInRect ((LPRECT)&rc, pt))
        {
          HandlePopupMenu( hwnd, pt, ID_TWB_POPUP );
          return 0;
        } /* endif */
      }
      break;

    default:
      break;
  } /* endswitch */

  // call old window procedure
  return CallWindowProc( (WNDPROC) lpfnOldMDIClient, hwnd, msg, mp1, mp2 );
}


/**********************************************************************/
/* Log file routines                                                  */
/**********************************************************************/
#ifdef _DEBUG

CHAR    szLogLine[512];                // buffer for text written to log file
static BOOL fLogging = FALSE;

void Log( PSZ pszString )
{
  WriteLog( pszString );
}

void LogString( PSZ pszFormat, PSZ pszString )
{
  sprintf( szLogLine, pszFormat, pszString );
  WriteLog( szLogLine );
}

void LogShort( PSZ pszFormat, SHORT sValue )
{
  sprintf( szLogLine, pszFormat, sValue );
  WriteLog( szLogLine );
}

void Log2Short( PSZ pszFormat, SHORT sValue1, SHORT sValue2 )
{
  sprintf( szLogLine, pszFormat, sValue1, sValue2 );
  WriteLog( szLogLine );
}

void LogLong( PSZ pszFormat, LONG lValue )
{
  sprintf( szLogLine, pszFormat, lValue );
  WriteLog( szLogLine );
}

void WriteLog( PSZ pszString )
{
  FILE *hLog;

  if ( fLogging )
  {
    /********************************************************************/
    /* Open logfile in append mode                                      */
    /********************************************************************/
    hLog = fopen( "\\EQF.LOG", "a" );

    if ( hLog != NULL )
    {
      CHAR   szDateTime[20];             // buffer for date or time string

      /******************************************************************/
      /* Prefix line with date and time                                 */
      /******************************************************************/
      _strdate( szDateTime );
      fputs( szDateTime, hLog );
      fputc( ' ', hLog );
      _strtime( szDateTime );
      fputs( szDateTime, hLog );
      fputc( ' ', hLog );
      fputc( ' ', hLog );

      /******************************************************************/
      /* Write text line to log                                         */
      /******************************************************************/
      fputs( pszString, hLog );
      fputc( '\n', hLog );

      /******************************************************************/
      /* Close logfile                                                  */
      /******************************************************************/
      fclose( hLog );
    } /* endif */
  } /* endif */
}
/**********************************************************************/
/* logging of system info ..                                          */
/**********************************************************************/
VOID LogSysInfo()
{
}
#else
// Dummy log functions for non-debug version
void Log( PSZ pszString ) { }
void LogString( PSZ pszFormat, PSZ pszString ) {}
void LogShort( PSZ pszFormat, SHORT sValue ) {}
void Log2Short( PSZ pszFormat, SHORT sValue1, SHORT sValue2 ) {}
void LogLong( PSZ pszFormat, LONG lValue ) {}
void WriteLog( PSZ pszString ) {}
VOID LogSysInfo() {}
#endif


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TwbStartUserHandler    Start any user handler
//------------------------------------------------------------------------------
// Function call:     TwbStartUserHandler( hAB );
//------------------------------------------------------------------------------
// Description:       Reads the file containing the list of user handles
//                    and starts all handlers found
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE                always
//------------------------------------------------------------------------------
BOOL TwbStartUserHandler( HAB hab )
{
  BOOL             fOK = TRUE;         // internal O.K. flag
  PHANDLERCONTROL  pHndlrCtrl = NULL;  // ptr to in-memory copy of handler file
  ULONG            ulBytes = 0;        // size of loaded handler control file
  USHORT           usEntries = 0;      // number of entries in table

  /********************************************************************/
  /* Load user handler control file into memory                       */
  /********************************************************************/
  if ( fOK )
  {
    CHAR   szFileName[MAX_EQF_PATH];

    strcpy( szFileName, EqfSystemPath );
    strcat( szFileName, BACKSLASH_STR );
    strcat( szFileName, PROPDIR );
    strcat( szFileName, BACKSLASH_STR );
    strcat( szFileName, USERHANDLERTABLE );

    fOK = UtlLoadFileL( szFileName, (PVOID *)&pHndlrCtrl, &ulBytes, FALSE, FALSE );
  } /* endif */

  /********************************************************************/
  /* Check size of loaded file (must be a multiple of the size of a   */
  /* handler control record)                                          */
  /********************************************************************/
  if ( fOK )
  {
    usEntries = (USHORT)(ulBytes / sizeof(HANDLERCONTROL));
    if ( (usEntries * sizeof(HANDLERCONTROL)) != ulBytes )
    {
      fOK = FALSE;                     // file seems to be damaged ...
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* allocate user handler control table area                         */
  /********************************************************************/
  if ( fOK  && usEntries )
  {
    fOK = UtlAlloc( (PVOID *) &pUserHandler, 0L,
                    (LONG)(sizeof(USERHANDLER)*usEntries), NOMSG );
    usUserHandler = 0;                 // no handler in user handler table yet
  } /* endif */

  /********************************************************************/
  /* Process the entries in the handler control table                 */
  /********************************************************************/
  if ( fOK )
  {
    PHANDLERCONTROL pEntry = pHndlrCtrl; // ptr to current entry
    PBYTE           pTemp;             // ptr for checksum computation
    USHORT          usCheckSum;        // buffer for checksum
    SHORT           i;                 // loop index
    HMODULE         hModule;           // buffer for module handle
    PFN             pfnFunction = NULL;       // ptr to user handle function
    HWND            hwnd = NULL;              // handler window handle

    while ( usEntries )
    {
      fOK = TRUE;                      // reset O.K. flag
      hModule = NULLHANDLE;            // reset module handle

      /****************************************************************/
      /* Check checksum of entry                                      */
      /****************************************************************/
      pTemp = (PBYTE)pEntry;           // start at begin of entry
      usCheckSum = 0;                  // initialize checksum buffer
      for ( i = sizeof(USHORT); i < sizeof(HANDLERCONTROL); i++ )
      {
        usCheckSum = usCheckSum + pTemp[i];
      } /* endfor */
      if ( usCheckSum != pEntry->usCheckSum )
      {
        fOK = FALSE;                   // entry is corrupted, ignore it
      } /* endif */

      /****************************************************************/
      /* Load user handler DLL                                        */
      /****************************************************************/
      if ( fOK )
      {
        CHAR   szModuleName[MAX_FILESPEC]; // buffer for module name
        USHORT usRC;                   // return code of DosLoadModule

        strcpy( szModuleName, pEntry->szModule );
        strcat( szModuleName, EXT_OF_DLL );
        usRC = DosLoadModule( NULL, 0, szModuleName, &hModule );
        if ( usRC != NO_ERROR )
        {
          fOK = FALSE;                 // DLL fails to load
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Get Address of handler function or handler callback routine  */
      /****************************************************************/
      if ( fOK )
      {
        USHORT usRC;                   // return code of DosGetProcAddr

        usRC = DosGetProcAddr( hModule, pEntry->szFunction, &pfnFunction );
        if ( usRC != NO_ERROR )
        {
          fOK = FALSE;                 // function address not found
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Start the handler                                            */
      /****************************************************************/
      if ( fOK )
      {
        if ( pEntry->fIsListHandler )
        {
          fOK = TwbStartListHandler( pEntry->szHandler,
                                     (PFN_HANDLERCALLBACK)pfnFunction,
                                     &hwnd );
        }
        else
        {
          fOK = TwbStartHandler( hab, pEntry->szHandler,
                                 (PFNWP)pfnFunction, &hwnd );
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Add to global user handler table if successful               */
      /****************************************************************/
      if ( fOK )
      {
        pUserHandler[usUserHandler].hModule = hModule;
        pUserHandler[usUserHandler].hwnd    = hwnd;
        pUserHandler[usUserHandler].fIsListHandler = pEntry->fIsListHandler;
        strcpy( pUserHandler[usUserHandler].szHandler, pEntry->szHandler );
        memcpy( pUserHandler[usUserHandler].ausCommands, pEntry->ausCommands,
                sizeof(pUserHandler[usUserHandler].ausCommands) );
        memcpy( pUserHandler[usUserHandler].ausMenuIDs, pEntry->ausMenuIDs,
                sizeof(pUserHandler[usUserHandler].ausMenuIDs) );
        usUserHandler++;
      } /* endif */

      /****************************************************************/
      /* Cleanup of user handler if something wrent wrong             */
      /****************************************************************/
      if ( !fOK )
      {
        if ( hModule ) DosFreeModule ( hModule );
      } /* endif */

      /****************************************************************/
      /* Prepare for next entry                                       */
      /****************************************************************/
      usEntries--;
      pEntry++;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( pHndlrCtrl != NULL )  UtlAlloc( (PVOID *) &pHndlrCtrl, 0L, 0L, NOMSG );

  return( TRUE );                      // return TRUE in any case

} /* end of function */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TwbFreeResources
//------------------------------------------------------------------------------
// Function call:     TwbFreeResources();
//------------------------------------------------------------------------------
// Description:       free user handler and other resources..
//------------------------------------------------------------------------------
// Parameters:        VOID
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     free any user handler
//                    free the hResMod
//------------------------------------------------------------------------------
VOID TwbFreeResources( VOID )
{
  /******************************************************************/
  /* Free any user handler modules                                  */
  /******************************************************************/
  if ( usUserHandler )
  {
    USHORT i;                                  // loop index

    for ( i = 0; i < usUserHandler; i++ )
    {
      DosFreeModule( pUserHandler[i].hModule );
    } /* endfor */
  } /* endif */
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
  DosFreeModule ( hResMod );

} /* end of function TwbFreeResources */

//------------------------------------------------------------------------------
// Internal function
// External function
//------------------------------------------------------------------------------
// Function name:     TwbSetMessageFilterHook
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       _
//------------------------------------------------------------------------------
// Input parameter:   _
// Parameters:        _
//------------------------------------------------------------------------------
// Output parameter:  _
//------------------------------------------------------------------------------
// Returncode type:   _
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Prerequesits:      _
//------------------------------------------------------------------------------
// Side effects:      _
//------------------------------------------------------------------------------
// Samples:           _
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------
VOID TwbSetMessageFilterHook( HINSTANCE hInstance, HWND hFrame )
{
    MessageFilterHook = SetWindowsHookEx( WH_MSGFILTER,
                                          (HOOKPROC)HelpMessageFilterHook,
                                          hInstance,
                                          GetWindowThreadProcessId( hFrame, NULL) );

} /* end of function TwbSetMessageFilterHook */


void
TwbUnSetMessageFilterHook()
{
  if ( MessageFilterHook )
  {
    UnhookWindowsHookEx( MessageFilterHook );
  } /* endif */
}

VOID EqfDisplayHelpIndex()
{
  WinHelp( (HWND)UtlQueryULong( QL_TWBCLIENT ),
           EqfSystemHlpFile, HELP_CONTENTS, 0L );
}

VOID EqfDisplayHelpForHelp()
{
  WinHelp( (HWND)UtlQueryULong( QL_TWBCLIENT ),
           EqfSystemHlpFile, HELP_HELPONHELP, 0L );
}

/**********************************************************************/
/* Set IELookAndFeel usage or not ..                                  */
/**********************************************************************/
VOID EqfIELookAndFeel (BOOL fIE)
{
  fIELookAndFeel = fIE;
}

//------------------------------------------------------------------------------
// Function name: StartBrowser
//------------------------------------------------------------------------------
// Function call:
//
//------------------------------------------------------------------------------
// Description:
//
//------------------------------------------------------------------------------
// Input parameter:
//
//------------------------------------------------------------------------------
// Output parameter:
//
//------------------------------------------------------------------------------
// Returncode type:
//------------------------------------------------------------------------------
// Returncodes:
//
//------------------------------------------------------------------------------
// Prerequesits:
//
//------------------------------------------------------------------------------
// Side effects:
//
//------------------------------------------------------------------------------
// Samples:
//
//------------------------------------------------------------------------------
// Function flow:
//
//               fOk = WinExec("eqfcmd.exe", SW_SHOW); for 16bit apps
//
//
//------------------------------------------------------------------------------
BOOL StartBrowser(PSZ content)
{
  char  CmdLine[512];
  CHAR  szTMTitle[100];
  HINSTANCE hi;


  strcpy(CmdLine,"");

  if ( !stricmp(content,"home") )
  {
      GetStringFromRegistry( APPL_Name, "Homepage",  szTMTitle, sizeof(szTMTitle ), "http://www.opentm2.org/" );
      strcat(CmdLine,szTMTitle);

  }
  else if ( !stricmp(content,"docu") )
  {
    UtlMakeEQFPath( CmdLine, NULC, SYSTEM_PATH, NULL );
    strcat( CmdLine, "\\doc\\Opentm2TranslatorsReference.pdf" );
      //GetStringFromRegistry( APPL_Name, "Documentation", szTMTitle, sizeof(szTMTitle ), "http://www.beo-doc.de/opentm2wiki/index.php/Main_Page" );
      //strcat(CmdLine,szTMTitle);
  }
  else if ( !stricmp(content,"techguide") )
  {
    UtlMakeEQFPath( CmdLine, NULC, SYSTEM_PATH, NULL );
    strcat( CmdLine, "\\doc\\OpenTM2TechnicalReference.pdf" );
  }
  else
  {
      strcat(CmdLine,content);
  } // end if

  hi = ShellExecute( NULL,"open",
                     CmdLine,NULL,NULL,SW_SHOW);
  return ((int) hi >= 32);
} // end of function start session

// perform any pending updates on startup
// 12-15-15 DAW  This should not longer be needed.  Function is in markup table plugin constructor.
VOID PerformPendingUpdates()
{
  FILEFINDBUF stFile;              // Output buffer of UtlFindFirst
  USHORT usCount;                  // For UtlFindFirst
  HDIR hSearch = HDIR_CREATE;      // Directory handle for UtlFindFirst
  USHORT usRC;
  CHAR szSearch[MAX_EQF_PATH];     // buffer for search pattern
  CHAR szSource[MAX_EQF_PATH];     // buffer for source file
  CHAR szTarget[MAX_EQF_PATH];     // buffer for target file
  BOOL fUserExitsMoved = FALSE;    // TRUE = user exits have been moved

  //
  // copy any user exits
  //

  // setup search path
  UtlMakeEQFPath( szSearch, NULC, SYSTEM_PATH, NULL );
  strcat( szSearch, BACKSLASH_STR );
  strcat( szSearch, PENDINGEXITS_DIR );
  strcat( szSearch, BACKSLASH_STR );
  strcat( szSearch, DEFAULT_PATTERN_NAME );
  strcat( szSearch, EXT_OF_DLL );

  // loop over all user exit DLLs in  'pending user exit copy' directory
  memset( &stFile, 0, sizeof(stFile) );
  usCount = 1;
  usRC = UtlFindFirst( szSearch, &hSearch, 0, &stFile, sizeof(stFile), &usCount, 0L, FALSE );
  while ( (usRC == NO_ERROR) && usCount )
  {
    // setup source and target path names
    UtlMakeEQFPath( szSource, NULC, SYSTEM_PATH, NULL );
    strcat( szSource, BACKSLASH_STR );
    strcat( szSource, PENDINGEXITS_DIR );
    strcat( szSource, BACKSLASH_STR );
    strcat( szSource, RESBUFNAME(stFile) );
    UtlMakeEQFPath( szTarget, NULC, WIN_PATH, NULL );
    strcat( szTarget, BACKSLASH_STR );
    strcat( szTarget, RESBUFNAME(stFile) );

    // move user exit
    UtlDelete( szTarget, 0L, FALSE );
    UtlMove( szSource, szTarget, 0L, FALSE );
    fUserExitsMoved = TRUE;

    // next user exit DLL
    usRC = UtlFindNext( hSearch, &stFile, sizeof(stFile), &usCount, FALSE );
  } /* endwhile */
  if ( hSearch != HDIR_CREATE ) UtlFindClose( hSearch, FALSE );

  // reset folder-is-disabled flags in folder properties
  if ( fUserExitsMoved )
  {
    // loop over all folder properties and reset flag if required
    hSearch = HDIR_CREATE;      
    UtlMakeEQFPath( szSearch, NULC, PROPERTY_PATH, NULL );
    strcat( szSearch, BACKSLASH_STR );
    strcat( szSearch, DEFAULT_PATTERN_NAME );
    strcat( szSearch, EXT_FOLDER_MAIN );
    memset( &stFile, 0, sizeof(stFile) );
    usCount = 1;
    usRC = UtlFindFirst( szSearch, &hSearch, 0, &stFile, sizeof(stFile), &usCount, 0L, FALSE );
    while ( (usRC == NO_ERROR) && usCount )
    {
      PPROPFOLDER pProp = NULL;
      ULONG       ulSize = 0;

      UtlMakeEQFPath( szSource, NULC, PROPERTY_PATH, NULL );
      strcat( szSource, BACKSLASH_STR );
      strcat( szSource, RESBUFNAME(stFile) );
      if ( UtlLoadFileL( szSource, (PVOID *)&pProp, &ulSize, FALSE, FALSE ) )
      {
        if ( pProp->fDisabled_UserExitRefresh )
        {
          pProp->fDisabled_UserExitRefresh = FALSE;
          UtlWriteFileL( szSource, ulSize, pProp, FALSE );
        } /* endif */           
        UtlAlloc( (PVOID *)&pProp, 0, 0, NOMSG );
      } /* endif */     

      // next property file
      usRC = UtlFindNext( hSearch, &stFile, sizeof(stFile), &usCount, FALSE );
    } /* endwhile */
    if ( hSearch != HDIR_CREATE ) UtlFindClose( hSearch, FALSE );
  } /* endif */     
} /* end of function PerformPendingUpdates */
