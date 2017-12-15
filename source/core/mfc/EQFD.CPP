/*! EQFD.CPP
	Copyright (c) 1990-2016, International Business Machines Corporation and others. All rights reserved.
	Description:  MFC support in OpenTM2
*/

#include "eqfmfc.h"
#include "core\PluginManager\OtmPlugin.h"
#include "core\PluginManager\PluginManager.h"

// activate following define to create minidumps when program terminates abnormally
#ifdef _DEBUG
  #define CREATEMINIDUMP
#endif

// activate define to enable startup logging
//  #define STARTUP_LOGGING  

/**********************************************************************/
/* private prototypes                                                 */
/**********************************************************************/
#ifndef CPPTEST
extern "C"
{
#endif

  #define INCL_EQF_TAGTABLE         // tag table and format functions
  #define INCL_EQF_TM               // general Transl. Memory functions
  #define INCL_EQF_TP               // public translation processor functions
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_ASD              // dictionary access functions (Asd...)
  #define INCL_EQF_EDITORAPI        // editor API
  #define INCL_EQF_FOLDER           // folder list and document list functions

/**********************************************************************/
/* ensure correct packing of our structures                           */
/**********************************************************************/
#pragma pack( push, TM2StructPacking, 1 )

  #include "eqf.h"                  // General .H for EQF
  #include "eqfstart.id"            // IDs for EQFSTARR resource
  #include "eqfb.id"                // IDs for windows menu
  #include "eqffll.id"              // IDs for folder lists
  #include "eqffll00.h"             // for FLL_OBJECT_IND
  #include "eqfcolw.id"             // column width IDs
  #include "eqfdoc00.h"
  #include "eqfserno.h"
  #include "eqfwcnt.h"
  #include "EQFstart.H"
  #include "eqfdrvex.h"
  #include "eqfprogr.h"             // progress controls
  #include "EQFTMM.H"               // TMM include file
  #include "eqfutmdi.h"             // MDI Utilities
  #include "eqfmorph.h"             // for Terminating MORPH
  #include "eqfevent.h"             // event logging facility
  USHORT SetupMAT( HAB hab, CHAR chPrimaryDrive, PSZ  pszSecondaryDrives, CHAR chLanDrive );

#pragma pack( pop, TM2StructPacking )

  LRESULT TwbCreateMsg( HWND hwnd, WPARAM mp1, LPARAM mp2 );

#ifndef _TQM
  VOID    QDAMInit( VOID );
#endif
  VOID    TwbShowLogo( LONG, HWND, PSZ, USHORT, UCHAR );


#ifndef CPPTEST
}
#endif

#ifdef _TQM
//  #define TQM_LOGGING
#endif

#include "EQFD.h"

#include <dde.h>                     // DDE defines
#include "eqfmFrm.h"
#include "eqfcFrm.h"

#include "eqfclbox.h"   // base class for Column listbox
#include "eqfweb.h"     // HTML class...
#include "eqfgen99.h"
#include "eqfprc99.h"
#include "eqfdoc99.h"
#include "eqfrpt99.h"   // report handler
#include "eqftmm99.h"   // translation memory maintenance
#include "eqfdic99.h"   // dictionary lookup
#include "eqffll99.h"   // folder/subfolder handler

#include "eqf.id"
#include "EQFPLGMG.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


  #define  MINIMIZE_STR    "/TYPE=MIN"
  #define  MINIMIZE_LEN    9
  #define  LOGGROUP_STR    "/LOG="
  #define  LOGGROUP_LEN    5
  #define  DOCGROUP_STR    "DOC"
  #define  TMGROUP_STR     "TM"
  #define  DICTGROUP_STR   "DICT"
  #define  DBGROUP_STR     "DB"

SHORT GetMTLogingState();

/////////////////////////////////////////////////////////////////////////////
// CEQFDApp

BEGIN_MESSAGE_MAP(CEQFDApp, CWinApp)
        //{{AFX_MSG_MAP(CEQFDApp)
        ON_COMMAND(PID_HELP_MI_PRODUCTINFO , OnAppAbout)
        ON_COMMAND( PID_HELP_MI_INDEX, OnHelpIndex )
        ON_COMMAND( PID_HELP_FOR_HELP, OnHelpForHelp )
        ON_COMMAND( PID_FILE_MI_EBUS, OnEBus )
        ON_COMMAND( PID_FILE_MI_EBUS1, OnEBus1 )
        ON_COMMAND( PID_FILE_MI_EBUS1, OnEBus1 )
        ON_COMMAND( PID_FILE_MI_TECHGUIDE, OnTechGuide )
        ON_COMMAND( ID_HELP, OnHelp )
        ON_COMMAND( ID_CONTEXT_HELP, OnContextHelp )
        ON_COMMAND( ID_HELP_USING, OnHelpUsing )
        ON_COMMAND( ID_HELP_INDEX, OnHelpIndex )
        ON_COMMAND( ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup )

                // NOTE - the ClassWizard will add and remove mapping macros here.
                //    DO NOT EDIT what you see in these blocks of generated code!
//        ON_MESSAGE( WM_EQF_INITIALIZE, OnProcessTask )
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


#include "dbghelp.h"

// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);


/////////////////////////////////////////////////////////////////////////////
// CEQFDApp construction

CEQFDApp::CEQFDApp()
{
        // TODO: add construction code here,
        // Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CEQFDApp object

CEQFDApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CEQFDApp initialization


//VOID CEQFDApp::A3( WPARAM mp1, LPARAM mp2 )
//{
//  AfxMessageBox( "in CChildFrame" );
//}


BOOL CEQFDApp::PreTranslateMessage( MSG * pMsg )
{
  if( UtlIsDialogMsg( pMsg ) )
    return TRUE;
  else if ( TranslateMDISysAccel(m_pMainWnd->m_hWnd, pMsg) )
    return TRUE;
  else
    return CWinApp::PreTranslateMessage( pMsg );
}


BOOL CEQFDApp::InitInstance()
{
  CHAR szEqfResFile[ 256 ];
  CHAR EqfSystemPropPath[MAX_EQF_PATH]; // global system properties path
  CHAR EqfSystemMsgFile[MAX_EQF_PATH];  // global message file
  BOOL fMinimize = FALSE;        // start TWB frame not minimized
  SHORT sLogGroups[30] = { -1 };   // group of events to be logged
  SHORT sNoOfGroups    = 0;        // number of groups

#ifdef STARTUP_LOGGING
   FILE *hStartLog = NULL;
   CHAR szLogFile[MAX_EQF_PATH];
   time_t lCurTime = 0;

 //  UtlMakeEQFPath( szLogFile, NULC, SYSTEM_PATH, NULL );
   strcpy( szLogFile, "\\OTM\\LOGS" );
   UtlMkDir( szLogFile, 0L, FALSE );

   strcat( szLogFile, "\\OpenTM2Startup.LOG" );
   hStartLog = fopen( szLogFile, "wb" );
   if ( hStartLog )
   {
      time( &lCurTime );
      fprintf( hStartLog, "------ OpenTM2 startup:               %s", asctime( localtime( &lCurTime ) ) );
   } /* endif */
#endif

  // check if OpenTM has been installed already
  BOOL fInstalled = FALSE;
  HKEY hKey = NULL;
  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
  {
    HKEY hSubKey = NULL;
    if ( RegOpenKeyEx( hKey, "APPL_Name", 0, KEY_READ, &hSubKey) == ERROR_SUCCESS )
    {
      fInstalled = TRUE;
    }
    else
    {
      RegCloseKey( hSubKey );
    } /* endif */
    RegCloseKey( hKey );
  } /* endif */
  if ( !fInstalled )
  {
    strcpy( EqfSystemPropPath, m_pszHelpFilePath );
    UtlSplitFnameFromPath( EqfSystemPropPath );

    // check if OTM is placed in the correct directory
    if ( stricmp( EqfSystemPropPath + 1, ":\\OTM\\WIN" ) != 0 )
    {
      CHAR szBuffer[600];

      sprintf( szBuffer, "Setup Error\n\nThe OpenTM2 files are not placed in the correct location.\n\n\
Current location is %s\n\nthe OTM folder should reside in the root directory of the drive \
(%c:\\OTM)", EqfSystemPropPath, EqfSystemPropPath[0] );
      AfxMessageBox( szBuffer );
      return( FALSE );
    } /* endif */

    // do a on-the-fly installation
    SetupMAT( (HAB)0, m_pszHelpFilePath[0], "", ' ' );
  } /* endif */

#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 1.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

  COleMessageFilter *pFilter;

  m_hTM_TQMSem = INVALID_HANDLE_VALUE;

#ifndef _TQM

#ifdef USENEWRTFEDIT
  HMODULE hModRichEd = LoadLibrary("Msftedit.dll");     // Load the RichEdit DLL
#else
  HMODULE hModRichEd = LoadLibrary("RichEd20.Dll");     // Load the RichEdit DLL
#endif
  if ( hModRichEd )
  {
/**********************************************************************/
/* DllGetVersion is not externalized by RichEd20 - so try the old     */
/* fashioned approach                                                 */
/**********************************************************************/
//
//  DLLGETVERSIONPROC pDllGetVersion;
//  BOOL  fOK = TRUE;
//
//  pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress( hModRichEd, "DllGetVersion" );
//  if ( pDllGetVersion )
//  {
//    DLLVERSIONINFO dvi;
//    memset( &dvi, 0, sizeof( DLLVERSIONINFO ));
//    dvi.cbSize = sizeof( DLLVERSIONINFO );
//    if SUCCEEDED((*pDllGetVersion)( &dvi ))
//    {
//      if ( dvi.dwMajorVersion >= 3 )
//      {
//        int i = 0;
//      }
//      else
//      {
//        fOK = FALSE;
//        UtlError(ERROR_NEWER_RICHED_NEEDED, MB_CANCEL, 0, NULL, EQF_ERROR);
//      } /* endif */
//    }
//    else
//    {
//      fOK = FALSE;
//    } /* endif */
//  }
//  else
//  {
//    fOK = FALSE;
//  } /* endif */

    CHAR  chBuffer[ 512 ];
    BOOL  fOK;

#ifdef USENEWRTFEDIT
    fOK = GetFileVersionInfo( "Msftedit.dll", 0, sizeof( chBuffer ), &chBuffer[0] );
#else
    fOK = GetFileVersionInfo( "RichEd20.dll", 0, sizeof( chBuffer ), &chBuffer[0] );
#endif
    if ( fOK )
    {
      VS_FIXEDFILEINFO *pvsFileInfo;
      UINT  uLen;
      VerQueryValue( chBuffer, "\\", (PVOID *)&pvsFileInfo, &uLen );
      /****************************************************************/
      /* RichEd V.3.0 has fileversion 0x0005001e                      */
      /****************************************************************/
      fOK = ( pvsFileInfo->dwFileVersionMS >= 0x0005001e );
    } /* endif */

    if (!fOK )
    {
      FreeLibrary( hModRichEd );
    } /* endif */
  }
  else
  {
#ifdef USENEWRTFEDIT
    PSZ pData = "RichEdit 4.1";
#else
    PSZ pData = RICHEDIT_CLASS;
#endif
    UtlError(ERROR_DID_NOT_START, MB_CANCEL, 1, &pData, EQF_ERROR);
  } /* endif */

#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 2.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif


  // install exception handler
 	::SetUnhandledExceptionFilter( TopLevelFilter );

  // initialize MFC version of rich edit control
  AfxInitRichEdit();

#endif

  // ActiveX Control Containment component
  AfxEnableControlContainer();

  // Initialize OLE libraries
  if (!AfxOleInit())
  {
    AfxMessageBox("OLE_INIT_FAILED");
    return FALSE;
  }

#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 3.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif


  // Disable "system busy" popup when using OLE automation to
  // invoke MS Word during document validation export.   9-22-16
  pFilter = AfxOleGetMessageFilter();
  ASSERT_VALID(pFilter);
  pFilter->EnableBusyDialog(FALSE);
  pFilter->EnableNotRespondingDialog(FALSE);
//pFilter->SetMessagePendingDelay(120000);


  // Standard initialization
  // If you are not using these features and wish to reduce the size
  //  of your final executable, you should remove from the following
  //  the specific initialization routines you do not need.

//#ifdef _AFXDLL
//  Enable3dControls();                     // Call this when using MFC in a shared DLL
//#else
//  Enable3dControlsStatic();       // Call this when linking to MFC statically
//#endif

  // store path of EXE in system property path
  strcpy( EqfSystemPropPath, m_pszHelpFilePath );
  UtlSplitFnameFromPath( EqfSystemPropPath );

#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 4.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif




  // check if TQM/(and TM - P018362) is already running, if this so
  // show message and return
#ifdef _TQM
  {
    char szExclusiveFile[128];

    strcpy( szExclusiveFile, EqfSystemPropPath );
	strcat( szExclusiveFile, "\\TQMSEM.DAT" );
	m_hTM_TQMSem = CreateFile( szExclusiveFile, GENERIC_READ | GENERIC_WRITE,
                                 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                 NULL );
    if ( m_hTM_TQMSem == INVALID_HANDLE_VALUE )
    {
      AfxMessageBox( "TQM is already in use. Please wait!" );
      return( FALSE );
    } /* endif */
  }
#else
  if ( UtlIsAlreadyRunning(&m_hTM_TQMSem))
  {
    // Check if there is an OpenTM2 window already
    HWND hwndTMgr = FindWindow( TWBMAIN, NULL );
    if ( hwndTMgr )
    {
      SetForegroundWindow( hwndTMgr );
    }
    else
    {
     MessageBox( HWND_DESKTOP, "An instance of OpenTM2 or OtmItm is already running.", "OpenTM2", MB_CANCEL );    /* 5-30-14 */
    } /* endif */
    return( FALSE );
  }
#endif


  /******************************************************************/
  /* hadle command line switches                                    */
  /******************************************************************/
  if ( m_lpCmdLine && *m_lpCmdLine )
  {
    PSZ pszNextOpt = strchr( m_lpCmdLine, '/' );
    while ( pszNextOpt != NULL )
    {
      if (strnicmp( pszNextOpt, MINIMIZE_STR, MINIMIZE_LEN ) == 0 )
      {
        fMinimize = TRUE;
      }
      else if (strnicmp( pszNextOpt, LOGGROUP_STR, LOGGROUP_LEN ) == 0 )
      {
        PSZ pszGroup = pszNextOpt + LOGGROUP_LEN;
        if ( strnicmp( pszGroup, DOCGROUP_STR, strlen(DOCGROUP_STR) ) == 0 )
        {
          sLogGroups[sNoOfGroups++] = DOC_GROUP;
          sLogGroups[sNoOfGroups]   = -1;
        }
        else if ( strnicmp( pszGroup, TMGROUP_STR, strlen(TMGROUP_STR) ) == 0 )
        {
          sLogGroups[sNoOfGroups++] = TM_GROUP;
          sLogGroups[sNoOfGroups]   = -1;
        }
        else if ( strnicmp( pszGroup, DICTGROUP_STR, strlen(DICTGROUP_STR) ) == 0 )
        {
          sLogGroups[sNoOfGroups++] = DICT_GROUP;
          sLogGroups[sNoOfGroups]   = -1;
        }
        else if ( strnicmp( pszGroup, DBGROUP_STR, strlen(DBGROUP_STR) ) == 0 )
        {
          sLogGroups[sNoOfGroups++] = DB_GROUP;
          sLogGroups[sNoOfGroups]   = -1;
        } /* endif */
      } /* endif */
      pszNextOpt = strchr( pszNextOpt + 1, '/' );
    } /* endwhile */
  } /* endif */

  HINSTANCE hInstance = AfxGetInstanceHandle();

  // check if already an instance is running



  UtlSetULong( QL_HAB, (ULONG) hInstance );   // set QL_HAB
  UtlSetUShort( QS_RUNMODE, TWB_RUNMODE );
  UtlSetUShort( QS_GUIMODE, MFC_GUIMODE );

  UtlInitUtils( hInstance );

  UtlAddEventGroup( GENERAL_GROUP );
  int i = 0;
  while ( sLogGroups[i] != -1 )
  {
    UtlAddEventGroup( sLogGroups[i++] );
  } /* endwhile */

  INFOEVENT2( TWBSTART_LOC, NA_EVENT, 0, GENERAL_GROUP, NULL );


#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 5.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif


#ifndef _TQM
  QDAMInit();
#endif

#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 6.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif


  if ( TwbGetCheckProfileData(EqfSystemMsgFile,EqfSystemPropPath,szEqfResFile) )
  {
    BOOL fContinue = TRUE;

    TwbInit( TRUE, fMinimize );  // MFC enabled
    HMODULE hMod = (HMODULE) UtlQueryULong( QL_HRESMOD );
    AfxSetResourceHandle( hMod );

    {
      CHAR szPreReqFile[MAX_EQF_PATH];

      UtlMakeEQFPath( szPreReqFile, NULC, REPHANDLERPREREQPATH, NULL );
      strcat( szPreReqFile, BACKSLASH_STR );
      strcat( szPreReqFile, REPHANDLERPREREQFILE );

      fReportHandler = UtlFileExist( szPreReqFile );
    } /* endif */

    TWBSetFontCharacteristics( );

    RegisterProgressControl( hInstance );

    /******************************************************************/
    /* Register call for MDI dialogs                                  */
    /******************************************************************/
    RegisterMDIDialog( hInstance );


#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 6.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif


    /******************************************************************/
    /* check expiration date                                          */
    /******************************************************************/
#ifndef _TQM
    fContinue = TWBCheckExpirationDate( szEqfResFile, EqfSystemPropPath );
    if ( !fContinue )
    {
      return ( FALSE );
    } /* endif */
#endif

#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 7.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif


    /******************************************************************/
    /* check time of logo display and display it or not               */
    /******************************************************************/
#ifdef _TQM
    // currently no LOGO under TQM...
#else
    if ( fContinue )
    {
      SHORT sWait = (SHORT)GetIntFromRegistry( APPL_Name, KEY_FIRSTTIME, 9876 );
      if ( sWait == 9876 )
      {
        // no logo display time set yet, switch to indefinite wait and
        // set logo display time to 10 second
        sWait = -1;
        WriteIntToRegistry( APPL_Name, KEY_FIRSTTIME, 10000 );
      } /* endif */

      /***************************************************************/
      /* disable logo popup if asked for minimized startup           */
      /***************************************************************/
      if ( fMinimize )
      {
        sWait = 0;
      } /* endif */

      switch ( sWait )
      {
         case -1 :                       // indefinite wait
            TwbShowLogo( 0L, HWND_DESKTOP, szEqfResFile, EQFSTART, 0 );
            break;
         case 0  :                       // no display
            break;
         default :
            TwbShowLogo( (LONG) sWait, HWND_DESKTOP, szEqfResFile,EQFSTART,0);
            break;
      } /* endswitch */
      AutoVerUpProps(FALSE);
    } /* endif */
#endif
  }
  else
  {
    return FALSE;
  } /* endif */


#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 8.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

  // cv
  // register custom clipboard format
  // set format id to global buffer
  UINT uiFormat = RegisterClipboardFormat(CF_DOCLIST);
  if (!uiFormat)
  {
    UtlError( ERROR_REGISTER_CF, MB_OK, 0, NULL, EQF_ERROR );
  }
  UtlSetUShort(QS_CLIPBOARDFORMAT,(USHORT)uiFormat);

	// initialize the Plugin-Manager and load the available plugins
	{
		CHAR szPluginPath[ 256 ];
		UtlQueryString( QST_PLUGINPATH, szPluginPath, sizeof( szPluginPath ));
		UtlMakeEQFPath( szPluginPath, NULC, PLUGIN_PATH, NULL );

		PluginManager* thePluginManager = PluginManager::getInstance();
		thePluginManager->loadPluginDlls(szPluginPath);
        // Add for P403138
        char strParam[MAX_BUF_SIZE];
        memset(strParam, 0x00, sizeof(strParam));

        if (thePluginManager->ValidationCheck(strParam))
        {
            PSZ pszParam = strParam;
            UtlError(ERROR_PLUGIN_INVALID, MB_OK , 1, &pszParam, EQF_WARNING);
        }
        // Add end


#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 9.                          %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

		InitTMPluginWrapper();

#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 10.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

		InitDictPluginWrapper();

#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 11.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

		InitMarkupPluginMapper();

#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 12.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

		InitDocumentPluginMapper();

#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 13.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif


	}


  // Register the application's document templates.  Document templates
  //  serve as the connection between documents, frame windows and views.
  AddAvailTemplates();
#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 14.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

  // create main MDI Frame window
  CMainFrame* pMainFrame = new CMainFrame;
  if (!pMainFrame->LoadFrame(ID_TWBM_WINDOW))
  {
    PSZ p = szEqfResFile;
    UtlError( ERROR_RESOURCE_LOAD_FAILED, MB_CANCEL, 1, &p, EQF_ERROR );
    return FALSE;
  }
  else
  {
    UtlInitError( hInstance, pMainFrame->m_hWnd, NULL, EqfSystemMsgFile);
  } /* endif */
  m_pMainWnd = pMainFrame;

  // set MT logging flag
  UtlSetUShort( QS_MTLOGGING, GetMTLogingState() );
#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 15.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

  /******************************************************************/
  /* For Windows: register help message, set hook for help messages */
  /******************************************************************/
  TwbSetMessageFilterHook( hInstance, m_pMainWnd->m_hWnd );

  TwbCreateMsg( pMainFrame->m_hWnd, NULL, NULL );
#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 16.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

  // activate any tool plugins modifying the workbench action bar
  {
		PluginManager* thePluginManager = PluginManager::getInstance();
    thePluginManager->activateToolPlugins();
  }
#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 17.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

  PluginManagerAutoProps();
#ifdef STARTUP_LOGGING
  if ( hStartLog )
  {
     time( &lCurTime );
     fprintf( hStartLog, "---  Step 18.                         %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */
#endif

#ifdef STARTUP_LOGGING
   {
      time( &lCurTime );
      fprintf( hStartLog, "------ OpenTM2 startup end:           %s", asctime( localtime( &lCurTime ) ) );
     fclose( hStartLog ) ;
   }
#endif

  return TRUE;
}

// for OpenTM2Starter start
int OtmExecuteCommand(char * strCmd, char * strOutput, BOOL bNeedWait)
{
    DWORD nRC = NO_ERROR;

    STARTUPINFO si;
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    if ((strOutput != NULL) && (strlen(strOutput) != 0))
    {
        SECURITY_ATTRIBUTES StdErrSA = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
        si.hStdOutput = CreateFile(strOutput, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, 
                                   &StdErrSA, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;
    } else {
        si.hStdOutput = NULL;
        si.dwFlags = STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;
    }

    PROCESS_INFORMATION pi;

    if (CreateProcess(NULL, strCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        if (bNeedWait)
        {
            WaitForSingleObject(pi.hProcess, INFINITE);

            if (!GetExitCodeProcess(pi.hProcess, &nRC))
            {
                nRC = GetLastError();
            }
        }
    }
    else
    {
        nRC = GetLastError();
    }

    if ((strOutput != NULL) && (strlen(strOutput) != 0)) {
        CloseHandle(si.hStdOutput);
    }

    return nRC;
}
// for OpenTM2Starter  end

/**********************************************************************/
/* free all resources and clean up                                    */
/**********************************************************************/
int CEQFDApp::ExitInstance()
{
  // stop all plugins
  PluginManager* thePluginManager = PluginManager::getInstance();
  if ( thePluginManager != NULL ) thePluginManager->stopAllPlugins();

  // for OpenTM2Starter start
  char strPendingConf[MAX_PATH];
  CHAR szPluginPath[MAX_PATH];

  UtlQueryString( QST_PLUGINPATH, szPluginPath, sizeof( szPluginPath ));
  UtlMakeEQFPath( szPluginPath, NULC, PLUGIN_PATH, NULL );

  memset(strPendingConf, 0x00, sizeof(strPendingConf));
  sprintf(strPendingConf, "%s\\PendingUpdates.conf", szPluginPath);
  int nNeedStart = GetPrivateProfileInt("Settings", "NeedRestart", 0, strPendingConf);

  if (nNeedStart)
  {
      const char * strMsg = "Warning: A plugin installation will be completed the next time OpenTM2 is started. Do you want to restart OpenTM2 now?";
      int nRet = MessageBox(m_pMainWnd->m_hWnd, strMsg, APPL_Name, MB_YESNO | MB_WARNING);
      if (IDYES == nRet)
      {
          char strOpenTM2StarterPath[MAX_PATH];
          CHAR szWinPath[MAX_PATH];
          UtlQueryString( QST_WINPATH, szWinPath, sizeof( szWinPath ));
          UtlMakeEQFPath( szWinPath, NULC, WIN_PATH, NULL );
          sprintf(strOpenTM2StarterPath, "%s\\OpenTM2Starter.exe", szWinPath);
          OtmExecuteCommand(strOpenTM2StarterPath, NULL, FALSE);
      }
  }
  // for OpenTM2Starter end

  /******************************************************************/
  /* free instance of utilities                                     */
  /******************************************************************/
  UtlTerminateUtils();

#ifdef _TQM
  if ( hTQMLog )
  {
    struct tm *newtime;
    time_t aclock;

    time( &aclock );   // Get time in seconds
    newtime = localtime( &aclock );   // Convert time to struct tm form

    fprintf( hTQMLog, "*** Ended TQM Log %s\n", asctime(newtime)  );
    fclose( hTQMLog );
    hTQMLog = NULL;
  }
#endif

  // close our semaphore file
  if ( m_hTM_TQMSem != INVALID_HANDLE_VALUE ) CloseHandle(m_hTM_TQMSem);

  return 0;
}

/**********************************************************************/
/* Start all application templates  -- new templates have to be added */
/* at the end.                                                        */
/**********************************************************************/
BOOL CEQFDApp::AddAvailTemplates()
{

  /********************************************************************/
  /* Template ID=1:  Generic List windows                             */
  /********************************************************************/
  AddDocTemplate( new CMultiDocTemplate( ID_TWBM_WINDOW,
                                         RUNTIME_CLASS(CGenListDoc),
                                         RUNTIME_CLASS(CChildFrame),
                                         RUNTIME_CLASS(CGenListView)));
  /********************************************************************/
  /* Template ID=2:  Generic Process Windows                          */
  /********************************************************************/
  AddDocTemplate( new CMultiDocTemplate( ID_FOLDER_WINDOW,
                                         RUNTIME_CLASS(CGenProcDoc),
                                         RUNTIME_CLASS(CChildFrame),
                                         RUNTIME_CLASS(CGenProcView)));

  /********************************************************************/
  /* Template ID=3:  Translation Environment                          */
  /********************************************************************/
  AddDocTemplate( new CMultiDocTemplate( ID_TP_MAIN_WINDOW,
                                         RUNTIME_CLASS(CGenTEnvDoc),
                                         RUNTIME_CLASS(CChildFrame),
                                         RUNTIME_CLASS(CGenTEnvView)));

  /********************************************************************/
  /* Template ID=4:  Translation Memory Maintenance                   */
  /********************************************************************/
  AddDocTemplate( new CMultiDocTemplate( ID_TMM_WINDOW,
                                         RUNTIME_CLASS(CGenTMMDoc),
                                         RUNTIME_CLASS(CChildFrame),
                                         RUNTIME_CLASS(CGenTMMView)));

  /********************************************************************/
  /* Template ID=5:  WordCount                                        */
  /********************************************************************/
  AddDocTemplate( new CMultiDocTemplate( ID_WORDCNT_WINDOW,
                                         RUNTIME_CLASS(CRPTProcDoc),
                                         RUNTIME_CLASS(CChildFrame),
                                         RUNTIME_CLASS(CRPTProcView)));

  /********************************************************************/
  /* Template ID=6:  MDI Dictionary Lookup                            */
  /********************************************************************/
  AddDocTemplate( new CMultiDocTemplate( ID_DICTIONARY_WINDOW,
                                         RUNTIME_CLASS(CDICProcDoc),
                                         RUNTIME_CLASS(CChildFrame),
                                         RUNTIME_CLASS(CDICProcView)));

  /********************************************************************/
  /* Template ID=7:  Folder/SubFolder List                            */
  /********************************************************************/
  AddDocTemplate( new CMultiDocTemplate( ID_FOLSUBFOLDER_WINDOW,
                                         RUNTIME_CLASS(CFLLProcDoc),
                                         RUNTIME_CLASS(CChildFrame),
                                         RUNTIME_CLASS(CFLLListView)));

  /********************************************************************/
  /* all others to follow @@@                                         */
  /********************************************************************/
  return TRUE;
}

// App command to run the dialog
void CEQFDApp::OnAppAbout()
{
  CHAR szEqfResFile[ 256 ];
  UtlQueryString( QST_RESFILE, szEqfResFile, sizeof( szEqfResFile ));
  TwbShowLogo( 0L,
               (HWND)UtlQueryULong( QL_TWBCLIENT ),
               szEqfResFile,
               0, 0 );
}



void CEQFDApp::OnHelp()
{
  AfxMessageBox( "Help called ");
}

void CEQFDApp::OnHelpUsing()
{
  AfxMessageBox( "Help Using called ");
}

void CEQFDApp::OnContextHelp()
{
  AfxMessageBox( "Help Context called ");
}


void CEQFDApp::OnHelpIndex()
{
  EqfDisplayHelpIndex();
}

void CEQFDApp::OnHelpForHelp()
{
  EqfDisplayHelpForHelp();
}




// Start the homepage
// ------------------


void CEQFDApp::OnEBus()
{



    StartBrowser("home");


}



// Start the Online Docu
// ---------------------


void CEQFDApp::OnEBus1()
{


    StartBrowser("docu");

}

void CEQFDApp::OnTechGuide()
{


    StartBrowser("techguide");

}

void CEQFDApp::OnEBus2()
{


    StartBrowser("docu");

}





//HWND CEQFDApp::OnProcessTask(WPARAM mp1, LPARAM mp2)
//{
//  MDICREATESTRUCT *pMDICreate;
//  pMDICreate = (MDICREATESTRUCT *) mp2;
//
//        // TODO: Add your message handler code here
//  if (pMDICreate )
//  {
//    OnFileNew();
//  } /* endif */
//  return NULL;
//}


HWND CEQFDApp::CreateList( MDICREATESTRUCT *pMDICreate, SHORT sTempl)
{
  POSITION curTemplatePos =GetFirstDocTemplatePosition();
  HWND hwnd = NULL;

  /********************************************************************/
  /* store pointer                                                    */
  /********************************************************************/
  pMDI = pMDICreate;

  /********************************************************************/
  /* determine type of list to be created                             */
  /********************************************************************/
  strcpy( chTitle, pMDICreate->szTitle);

  /********************************************************************/
  /* try to find matching template                                    */
  /********************************************************************/
  CDocTemplate* curTemplate = GetNextDocTemplate( curTemplatePos );
  for (int i=1;(i<sTempl) && curTemplate ;i++ )
  {
    curTemplate = GetNextDocTemplate( curTemplatePos );
  } /* endfor */
  if ( curTemplate )
  {
    CDocument *pDoc = curTemplate->OpenDocumentFile( NULL );
    if ( pDoc )
    {
      POSITION pos = pDoc->GetFirstViewPosition();
      CView* pView = (CView*)pDoc->GetNextView( pos );
      /****************************************************************/
      /* store object name                                            */
      /****************************************************************/
      ((CEQFDoc*)pDoc)->SetObjName( (char *)pMDICreate->szTitle );

      hwnd = pView->m_hWnd;
    }
    else
    {
      hwnd = NULL;
    } /* endif */
  }
  else
  {
    AfxMessageBox("Not yet implemented" );
  } /* endif */
  return hwnd;
}

PVOID CEQFDApp::CreateShowWnd( HWND hwndParent, PSZ pTextFile )
{
  CWebBrowser2 * pCWeb = new CWebBrowser2;
  if ( pCWeb )
  {
    CRect rect;
    CWnd *pCWnd   = (CWnd::FromHandle(hwndParent));
    pCWnd->GetClientRect( &rect );
    pCWeb->Create( "", "PreView", WS_VISIBLE | WS_MAXIMIZE, rect,
                   pCWnd, 4711, NULL);
    if ( pCWeb->m_hWnd )
    {
      /****************************************************************/
      /* load file                                                    */
      /****************************************************************/
      int i = 14;     // navNoHistory | navNoWriteToCache | navNoReadFromCache,
      pCWeb->Navigate( pTextFile,
                       (VARIANT *) &i,
                       NULL, NULL, NULL );
    }
    else
    {
      delete pCWeb;
      pCWeb = NULL;
    } /* endif */
  }
  else
  {
    delete pCWeb;
    pCWeb = NULL;
  } /* endif */
  return pCWeb;
}

// navigate to given file
VOID CEQFDApp::ShowWndNavigate( PVOID pCWeb, PSZ pszFile )
{
  if ( pCWeb )
  {
    int i = 14;     // navNoHistory | navNoWriteToCache | navNoReadFromCache,
    ((CWebBrowser2 *)pCWeb)->Navigate( pszFile, (VARIANT *) &i, NULL, NULL, NULL );
    ((CWebBrowser2 *)pCWeb)->Refresh();
  }
}

VOID CEQFDApp::DestroyShowWnd( PVOID pCWeb )
{
  /********************************************************************/
  /* free our Web control...                                          */
  /********************************************************************/
  if ( pCWeb )
  {
    CWebBrowser2 * pCWeb2 = (CWebBrowser2 *)pCWeb;
    delete pCWeb2;
    pCWeb = NULL;
  }
}

VOID CEQFDApp::SizeShowWnd( HWND hwnd, PVOID pCWeb )
{
  /********************************************************************/
  /* size our Web control...                                          */
  /********************************************************************/
  if ( pCWeb )
  {
    CRect rect;
    CWnd * pCWnd = CWnd::FromHandle(hwnd);

    pCWnd->GetClientRect( &rect );
    ((CWebBrowser2 *)pCWeb)->MoveWindow( &rect, TRUE );
  } /* endif */
}

  /********************************************************************/
  /* refresh our Web control...                                       */
  /********************************************************************/
VOID CEQFDApp::RefreshShowWnd( PVOID pCWeb )
{
  if ( pCWeb )
  {
    ((CWebBrowser2 *)pCWeb)->Refresh();
    /*****************************************************************/
    /* select small font ...                                         */
    /*****************************************************************/
    SetFontSize( pCWeb, 1L );
  } /* endif */
}

VOID CEQFDApp::SetFontSize( PVOID pCWeb, LONG lSel )
{
  if ( pCWeb )
  {
    /****************************************************************/
    /* set font ..                                                  */
    /****************************************************************/
    COleVariant vaFontScale( lSel );
    ((CWebBrowser2 *)pCWeb)->ExecWB( OLECMDID_ZOOM,
                                     OLECMDEXECOPT_DONTPROMPTUSER,
                                     &vaFontScale, NULL );
  } /* endif */
}


LONG CEQFDApp::TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
	LONG retval = EXCEPTION_CONTINUE_SEARCH;
  static char szMessage [2048];
  pExceptionInfo;

  // force write of event log
  UtlEvent( GENERAL_GROUP, TWBEXCEPTION_LOC, INVOPERATION_EVENT, 0 );

#ifdef CREATEMINIDUMP
	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old
	// (e.g. Windows 2000)
	HMODULE hDll = NULL;
	char szDbgHelpPath[_MAX_PATH];
  static char szScratch [2048];


	if (GetModuleFileName( NULL, szDbgHelpPath, _MAX_PATH ))
	{
		char *pSlash = _tcsrchr( szDbgHelpPath, '\\' );
		if (pSlash)
		{
			_tcscpy( pSlash+1, "DBGHELP.DLL" );
			hDll = ::LoadLibrary( szDbgHelpPath );
		}
	}

	if (hDll==NULL)
	{
		// load any version we can
		hDll = ::LoadLibrary( "DBGHELP.DLL" );
	}

	LPCTSTR szResult = NULL;

	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if (pDump)
		{
			char szDumpPath[_MAX_PATH];

			// work out a good place for the dump file
      UtlMakeEQFPath( szDumpPath, NULC, SYSTEM_PATH, NULL );
      strcat( szDumpPath, "\\OpenTM2-" );
      strcat( szDumpPath, STR_DRIVER_LEVEL_NUMBER );
      strcat( szDumpPath, ".dmp" );

			{
				// create the file
				HANDLE hFile = ::CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
											FILE_ATTRIBUTE_NORMAL, NULL );

				if (hFile!=INVALID_HANDLE_VALUE)
				{
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

					ExInfo.ThreadId = ::GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;

					// write the dump
					BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
					if (bOK)
					{
            sprintf( szScratch, "Please send the files '%s' and '%c:\\EQF\\PROPERTY\\EQFEVENT.LOG' to the OpenTM2 development team for problem evaluation.", szDumpPath, szDumpPath[0] );
						szResult = szScratch;
						retval = EXCEPTION_EXECUTE_HANDLER;
					}
					else
					{
						sprintf( szScratch, "No dump created (Failed to save dump file to '%s' (error %d))", szDumpPath, GetLastError() );
						szResult = szScratch;
					}
					::CloseHandle(hFile);
				}
				else
				{
					sprintf( szScratch, "No dump created (Failed to create dump file '%s' (error %d))", szDumpPath, GetLastError() );
					szResult = szScratch;
				}
			}
		}
		else
		{
			szResult = "No dump created (DBGHELP.DLL too old)";
		}
	}
	else
	{
		szResult = "No dump created (DBGHELP.DLL not found)";
	}

	if (szResult)
  {
    strcpy( szMessage, "EQF9999: A fatal error has been encountered.\n\nOpenTM2 ended abnormally.\n\n" );
    strcat( szMessage, szResult );
		::MessageBox( NULL, szMessage, "Abnormal program termination", MB_OK );
  } /* endif */
#else
    strcpy( szMessage, "EQF9999: A fatal error has been encountered.\n\nOpenTM2 ended abnormally." );
		::MessageBox( NULL, szMessage, "Abnormal program termination", MB_OK );
#endif
	return retval;
}

// function to return the current DLL version
ULONG EQFGETEXEVERSION( void )
{
  BYTE abVersion[4];
  ULONG ulVersion;

  abVersion[0] = EQF_DRIVER_VERSION;
  abVersion[1] = EQF_DRIVER_RELEASE;
  abVersion[2] = EQF_DRIVER_SUBRELEASE;
  abVersion[3] = EQF_DRIVER_BUILD;

  ulVersion = *((ULONG *)&abVersion);

  return( ulVersion );
}

// check if MT logging is required
SHORT GetMTLogingState()
{
  SHORT sMTLogState = 0;

  FILE *hfTrigger = NULL;
  CHAR szBuffer[512];

  // setup trigger file name
  UtlMakeEQFPath( szBuffer, NULC, PROPERTY_PATH, NULL );
  strcat( szBuffer, "\\EQFNFLUENT.TRG" );

  // loop over trigger file lines
  hfTrigger = fopen( szBuffer, "r" );
  if ( hfTrigger )
  {
    fgets( szBuffer, sizeof(szBuffer), hfTrigger );
    while( !feof( hfTrigger ) && (sMTLogState == 0) )
    {
      PSZ pszLine = (PSZ)szBuffer;
      if ( *pszLine != '*' )
      {
        if ( _strnicmp( pszLine, "MTLOGGING", 9 ) == 0 )
        {
          sMTLogState = 1;
        } /* endif */
      } /* endif */
      fgets( (PSZ)szBuffer, sizeof(szBuffer), hfTrigger );
    } /*endwhile */
    fclose( hfTrigger );
  } /* endif */
  return( sMTLogState );
}

/////////////////////////////////////////////////////////////////////////////
// CEQFDApp comman
