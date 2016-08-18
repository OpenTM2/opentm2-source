/*! \file
	Description: This is the startup front end for the ITM program.

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_MORPH            // morphological functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFITM.H"
#include <eqfsetup.h>             // twbinit, etc.
#include <eqfdrvex.h>             // twbsetfontcharacteristics
#include "EQFB.ID"
#include "EQFITMD.id"                  // ITM dialog include file

//------------------------------------------------------------------------------
//  Helptable include files
//     - ID files for dialogs and windows
//     - help ID file
//     - message ID file
//     - help table
//------------------------------------------------------------------------------
#include "eqfstart.h"
#include "eqfutils.id"
#include "eqfdoc01.id"
#include "eqfiana1.id"
#include "eqfmem.id"
#include "eqffol.id"
#include "eqffll.id"
#include "eqftag00.id"                 // tag handler IDs
#include <EQFTIMP.ID>
#include <EQFTEXP.ID>
#include <EQFLNG00.ID>
#include "eqfdic00.id"
#include "eqfutclb.id"
#include "EQFDIMP.ID"
#include "EQFDEX.ID"
#include "EQFTIMP.ID"
#include "EQFTEXP.ID"
#include "EQFCNT01.ID"
#include "EQFFILT.ID"
#include "EQFQDPR.ID"
#include "EQFTwb.ID"                   // IDs of Twb dialogs
#include "EQFBDLG.ID"                  // IDs of Translation Processor dialogs
#include "EQFDPROP.ID"                 // IDs of Dictionary Dialogs - 1 -
#include "EQFDDLG.ID"                  // IDs of Dictionary Dialogs - 2 -
#include "EQFRDICS.ID"                 // IDs for remote dictionary
#include "EQFLIST.ID"                  // IDs of list handler windows and dialogs
#include "EQFTMFUN.ID"                 // IDs of memory...
#include "EQFTMM.ID"                   // IDs of memory ...
#include "eqfrpt.id"                   // IDs report handler dialog
#include "eqfmt.id"                    // IDs for machine translation memory
#ifdef _WPTMIF
  #include "EQFWPSET.ID"               // IDs of WP API
#endif
#include "eqfhelp.id"                  // help resource IDs
#include "eqfhlp1.h"                   // first part of help tables
#include "eqfmsg.htb"                  // message help table

//#include "tools\common\InitPlugins.h"

HAB  hAB;                              // PM anchor block handle
  CHAR   szMsgFile[MAX_EQF_PATH];      // buffer for message file name

/**********************************************************************/
/* static variables for text strings                                  */
/**********************************************************************/
#define MAX_RC_STRING  31
CHAR chTitle[ 2*MAX_RC_STRING ];

HMODULE hITMResMod = NULLHANDLE; // as long as it is running in standalone mode
HWND hwndFrame = NULLHANDLE;       // Frame window handle

/**********************************************************************/
/* Windows related function prototypes                                */
/**********************************************************************/
  static HHOOK  MessageFilterHook = NULL;// message filter hook
  DWORD CALLBACK HelpMessageFilterHook( int, WORD, LPMSG );
  HICON  hEQFITMIcon;

  static CHAR   EqfSystemMsgFile[MAX_EQF_PATH];  // global message file
  static CHAR   EqfSystemHlpFile[MAX_EQF_PATH];  // global help file
  static CHAR   szEqfSysLanguage[MAX_EQF_PATH];  // system language
  static CHAR   szEqfResFile[MAX_EQF_PATH];      // resource

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     main
//------------------------------------------------------------------------------
// Function call:     main( argc, *argv[]);
//------------------------------------------------------------------------------
// Description:       This is the main program which parses the commandline,
//                    sets up the environment, creates the processing window
//                    and starts the work thread.
//------------------------------------------------------------------------------
// Parameters:        int argc     number of commandline parameters
//                    char *argv[] pointer to passed commandline parameters
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     init presentation manager
//                    setup internal TranslationManager/2 utilities
//                    check the input parameters
//                    if ok
//                      register the window class (with space for pointer to
//                                                 IDA)
//                      create a standard window with flags set for Titlebar,
//                        border and sysmenu.
//                      anchor the IDA and set the window position
//                      set the titlebar text
//                      dispatch all incoming messages
//                    endif
//                    destroy the message queue and terminate the application
//                    return
//------------------------------------------------------------------------------

#undef _WINDLL                         // we are not dealing with a DLL

int PASCAL WinMain
(
  HINSTANCE        hInstance,          // current instance
  HINSTANCE        hPrevInstance,      // previous instance
  LPSTR            lpCmdLine,          // command line
  int              nCmdShow            // show-window type (open/icon)
)
{
    BOOL fOK = TRUE;                          // success indicator
    ULONG flCreate;                    // Window creation control flags
    PITMIDA  pITMIda;                  // pointer to IDA
    PSZ      pTemp;                    // pointer for UtlError string
    USHORT   usReturn;                 // return the error
    HANDLE   hSem = INVALID_HANDLE_VALUE;

    hPrevInstance; nCmdShow;

    hAB = hInstance;

    if ( UtlIsAlreadyRunning(&hSem))
    {
      MessageBox( HWND_DESKTOP, "An instance of OpenTM2 or ITM is already running.", "OpenTM2", MB_CANCEL );
      return (FALSE);
    }


    /******************************************************************/
    /* Try to create a large message queue; in case of failures       */
    /* try a smaller one                                              */
    /******************************************************************/
    {
      SHORT sNumOfMsgs;
      for ( sNumOfMsgs = 100; sNumOfMsgs >= 10; sNumOfMsgs -= 10 )
      {
        if ( SetMessageQueue( sNumOfMsgs ) )
        {
          /************************************************************/
          /* a message queue has been created so leave the loop       */
          /************************************************************/
          break;
        } /* endif */
      } /* endfor */
      if ( sNumOfMsgs < 10 )
      {
        /**************************************************************/
        /* Fatal Error: no message queue has been created!            */
        /**************************************************************/
        return( 9 );
      } /* endif */
    }

    SetupUtils( hAB, szMsgFile );
    TWBSetFontCharacteristics();

    /******************************************************************/
    /* allocate ida                                                   */
    /******************************************************************/
    fOK = UtlAlloc( (PVOID *) &pITMIda, 0L, (LONG) sizeof( ITMIDA ), ERROR_STORAGE );

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
    /*******************************************************************/
    /* copy input parameters into Ida                                  */
    /*******************************************************************/
    if ( fOK )
    {
       pITMIda->hab = hAB;
       pITMIda->fHorizontal = FALSE;
       pITMIda->fTimer = FALSE;
       pITMIda->hwndParent = HWND_DESKTOP;
       pITMIda->usSGMLFormat = SGMLFORMAT_UNICODE;
       pITMIda->ulSGMLFormatCP = 0L;
       pITMIda->ulAnsiCP = 0L;
    } /* endif */
    /*******************************************************************/
    /* get commandline parameters...                                   */
    /*******************************************************************/
    if ( fOK )
    {
      PSZ pHWNDParent;
      int iArgc;
      strcpy( &(pITMIda->szBuffer[100]), lpCmdLine );
      pITMIda->ppArgv = (PSZ *)(&(pITMIda->szBuffer[0]));
      UtlGetArgs ( (HINSTANCE) hAB, &(pITMIda->szBuffer[100]),
                   &iArgc, pITMIda->ppArgv );
      pITMIda->usArgc = (USHORT)iArgc;
      /********************************************************************/
      /* check for hwndParent specified                                   */
      /********************************************************************/
      pHWNDParent = strstr(lpCmdLine,"/HWND=");
      if ( pHWNDParent )
      {
        pITMIda->hwndParent = (HWND) atol(pHWNDParent+6);
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* load resource dll ...                                          */
    /******************************************************************/
    if ( fOK )
    {
    // Set name of resource, help file and message file for selected language

      GetStringFromRegistry( APPL_Name, KEY_SYSLANGUAGE,
                             szEqfSysLanguage, sizeof( szEqfSysLanguage ), DEFAULT_SYSTEM_LANGUAGE );
      fOK = UtlQuerySysLangFile( szEqfSysLanguage, szEqfResFile,
                                 EqfSystemHlpFile, EqfSystemMsgFile );

    /******************************************************************/
    /* Cut off resource extension if specified (OS/2 only)            */
    /******************************************************************/
      if ( fOK )
      {
        fOK = ! DosLoadModule ( NULL, 0, szEqfResFile, &hITMResMod);
      } /* endif */
      if ( !fOK )
      {
        pTemp = szEqfResFile;
        ITMUtlError( pITMIda, ERROR_RESOURCE_LOAD_FAILED, MB_CANCEL,
                     1, &pTemp, EQF_ERROR );
      } /* endif */

      /*************************************************************/
      /* set hResMod variable to be the same as hITMResMod if noone*/
      /* else has set it right now..                               */
      /*************************************************************/
      UtlSetULong( QL_HRESMOD, (ULONG)hITMResMod );

    } /* endif */

    /******************************************************************/
    /* check time of logo display and display it or not               */
    /******************************************************************/
    if ( fOK && (pITMIda->hwndParent == HWND_DESKTOP))
    {
      SHORT sWait;
       sWait = (SHORT)GetIntFromRegistry( APPL_Name, KEY_FIRSTTIME, 9876 );

       if ( sWait == 9876 )
       {
         // no logo display time set yet, switch to indefinite wait and
         // store a set logo display time of 10 second
         sWait = -1;
         WriteIntToRegistry( APPL_Name, KEY_FIRSTTIME, 10000 );
       } /* endif */

       switch ( sWait )
       {
          case -1 :                       // indefinite wait
             TwbShowLogo( 0L, HWND_DESKTOP, szEqfResFile, EQFITM, ITM_REVISION );
             break;
          case 0  :                       // no display
             break;
          default :
             TwbShowLogo( (LONG) sWait, HWND_DESKTOP, szEqfResFile, EQFITM,
                          ITM_REVISION );
             break;
       } /* endswitch */
    } /* endif */

    /******************************************************************/
    /* Register class for superclassed BS_HELP buttons                */
    /******************************************************************/
    if ( fOK )
    {
      UtlRegisterEqfHelp( hAB );
    } /* endif */

    if ( fOK )
    {
      if ( fOK )
      {
        fOK = WinRegisterClass(               // Register window class
           (HINSTANCE) hAB,                   // Anchor block handle
           ITM_CLASS,                         // Window class name
           ITMWNDPROC,                        // Address of window procedure
           CS_SIZEREDRAW,                     // Class Style
           sizeof( PSZ ));                    // No extra window words
      } /* endif */

      if ( fOK )
      {
        fOK = WinRegisterClass(               // Register window class
           (HINSTANCE) hAB,                   // Anchor block handle
           ITM_VISCLASS,                      // Window class name
           ITMVISWNDPROC,                     // Address of window procedure
           CS_SIZEREDRAW,                     // Class Style
           sizeof( PSZ ));                    // No extra window words
      } /* endif */

      if ( fOK )
      {
        fOK = WinRegisterClass(               // Register window class
           (HINSTANCE) hAB,                   // Anchor block handle
           ITM_PROCCLASS,                     // Window class name
           ITMPROCWNDPROC,                    // Address of window procedure
           CS_SIZEREDRAW,                     // Class Style
           sizeof( PSZ ));                    // No extra window words
      } /* endif */

    if ( pITMIda->hwndParent != HWND_DESKTOP )
    {
      flCreate = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN |
                 WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZE;
    }
    else
    {
      flCreate = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN |
                 WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZE | WS_THICKFRAME;
    } /* endif */
    pITMIda->hwnd = hwndFrame =
       CreateWindow(  ITM_CLASS,
                      "",            //window caption
                      flCreate,
                      CW_USEDEFAULT,
                      CW_USEDEFAULT,
                      CW_USEDEFAULT,
                      CW_USEDEFAULT,
                      pITMIda->hwndParent,
                      LoadMenu( hITMResMod, MAKEINTRESOURCE(ID_ITM_WINDOW)),
                      (HINSTANCE) hAB,               //program instance handle
                      NULL );

     if ( hwndFrame )
     {
       hEQFITMIcon = LoadIcon( (HINSTANCE) hAB, MAKEINTRESOURCE( EQFITM_ICON ));
       SetClassLong( hwndFrame, GCL_HICON, (LONG)hEQFITMIcon );

     } /* endif */

      if ( hwndFrame )
      {
        ANCHORWNDIDA( pITMIda->hwnd, pITMIda );
        ANCHORWNDIDA( hwndFrame, pITMIda );
        // Windows: No help instance possible, we use our message filter hook

        UtlInitError( hAB, hwndFrame, pITMIda->hHelp, szMsgFile);
        UtlSetULong( QL_TWBFRAME, (ULONG) hwndFrame );
        UtlSetULong( QL_TWBCLIENT, (ULONG) pITMIda->hwnd );
       /*****************************************************************/
       /* load strings from resource                                    */
       /*****************************************************************/
        WinLoadString( hAB, hITMResMod, IDS_ITM_TITLE,
                       sizeof(chTitle), chTitle);
        WinLoadString( hAB, hITMResMod, IDS_ITM_SRCDOC,
                       sizeof( pITMIda->stSrcInfo.chType ),
                       pITMIda->stSrcInfo.chType);
        WinLoadString( hAB, hITMResMod, IDS_ITM_TGTDOC,
                       sizeof( pITMIda->stTgtInfo.chType ),
                       pITMIda->stTgtInfo.chType);

        WinLoadString( hAB, hITMResMod, IDS_ITM_STAT_SEGMENTS,
                       sizeof( pITMIda->chStatSegments ),
                       pITMIda->chStatSegments);
        WinLoadString( hAB, hITMResMod, IDS_ITM_STAT_UNALIGNED,
                       sizeof( pITMIda->chStatUnaligned ),
                       pITMIda->chStatUnaligned);
        WinLoadString( hAB, hITMResMod, IDS_ITM_STAT_IGNORED,
                       sizeof( pITMIda->chStatIgnored ),
                       pITMIda->chStatIgnored);
        WinLoadString( hAB, hITMResMod, IDS_ITM_STAT_MATCH,
                       sizeof( pITMIda->chStatMatch ),
                       pITMIda->chStatMatch);
        WinLoadString( hAB, hITMResMod, IDS_ITM_STAT_IRREGULAR,
                       sizeof( pITMIda->chStatIrregular ),
                       pITMIda->chStatIrregular);

        /****************************************************************/
        /* create the window to fillup 90% of the screen width and to   */
        /* be 500 pels in height.                                       */
        /****************************************************************/
        WinSetWindowText( hwndFrame, chTitle );
        WinSetWindowPos( hwndFrame,           // Set the size and position of
                HWND_TOP,                  // the window before showing.
                40,
                40,
                40, 40,
                EQF_SWP_SIZE | EQF_SWP_MOVE |
                EQF_SWP_ACTIVATE | EQF_SWP_MINIMIZE );

    /******************************************************************/
    /* For Windows: register help message, set hook for help messages */
    /******************************************************************/

    MessageFilterHook = SetWindowsHookEx( WH_MSGFILTER,
                                          (HOOKPROC)HelpMessageFilterHook,
                                          hInstance,
                                          (DWORD) GetWindowTask( hwndFrame ) );
    //// init plugins
    //{
    //    CHAR szPluginPath[ 256 ];
    //    UtlSetString( QST_PLUGINPATH, "PLUGINS" );
    //    UtlQueryString( QST_PLUGINPATH, szPluginPath, sizeof( szPluginPath ));
    //    UtlMakeEQFPath( szPluginPath, NULC, PLUGIN_PATH, NULL );
    //    InitializePlugins( szPluginPath );
    //}

      /****************************************************************/
      /* Get and dispatch messages from the application message queue */
      /* until WinGetMsg returns FALSE, indicating a WM_QUIT message. */
      /****************************************************************/
      {
         MSG msg;                    // message queue structure
         while ( GetMessage( &msg,   // message structure
             NULL,                   // handle of window receiving the message
             0,                      // lowest message to examine
             0))                     // highest message to examine
         {
           if ( UtlIsDialogMsg( &msg ) )
           {
             /***********************************************************/
             /* Nothing to to, message has bee dispatched inside        */
             /* UltisDialogMsg                                          */
             /***********************************************************/
           }
           else
           {
             TranslateMessage( &msg ); // Translates virtual key codes
             DispatchMessage( &msg );  // Dispatches message to window
           }
         } /* endwhile */
      }
      } /* endif */
      /******************************************************************/
      /* For Windows: Unhook help message hook                          */
      /******************************************************************/
      if ( MessageFilterHook )
      {
        UnhookWindowsHookEx( MessageFilterHook );
      } /* endif */
      /****************************************************************/
      /* free the resource - it is no longer needed ...               */
      /****************************************************************/
      if ( hITMResMod )
      {
        DosFreeModule ( hITMResMod );
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* stop any pending timer                                         */
    /******************************************************************/
    if ( pITMIda && pITMIda->fTimer )
    {
      WinStopTimer( pITMIda->hab, pITMIda->hwnd, ITM_TIMER_ID );
      pITMIda->fTimer = FALSE;
    } /* endif */

    /******************************************************************/
    /* Terminate morphologic functions                                */
    /******************************************************************/
    //MorphTerminate();

    /******************************************************************/
    /* activate the calling application ...                           */
    /******************************************************************/
    if ( pITMIda->hwndParent != HWND_DESKTOP )
    {
      SETFOCUSHWND( pITMIda->hwndParent );
    } /* endif */
    /******************************************************************/
    /* free instance of utilities                                     */
    /******************************************************************/
    if (pITMIda->usRC == ITM_COMPLETE)
    {
       usReturn = 1;  // BUTCH @@@ ONE BEER
    }
    else
    {
      usReturn = pITMIda->usRC;
    } /* end if */


    UtlAlloc( (PVOID *) &pITMIda, 0L, 0L, NOMSG );         // free resource
    UtlTerminateUtils();

    if ( hwndFrame && hEQFITMIcon )
    {
      DestroyIcon( hEQFITMIcon );
      hEQFITMIcon = NULL;
    } /* endif */

    // close our semaphore file
    if ( hSem != INVALID_HANDLE_VALUE ) CloseHandle(hSem);


    return ( usReturn );

} /* end of function main */

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
    switch ( nCode )
    {
      case MSGF_DIALOGBOX :
        if ( (lpMsg->message == WM_KEYDOWN) && (lpMsg->wParam == VK_F1) )
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
                /****************************************************/
                /* Normal child control ==> get ID of control and   */
                /* continue with control's parent                   */
                /****************************************************/
                sControlID = (SHORT)GetWindowLong( hParent, GWL_ID );
                hParent = GetParent( hParent );
              } /* endif */
            } /* endwhile */

            if ( hParent != NULL )
            {
              SendMessage( hParent, WM_EQF_QUERYID, 0, MP2FROMP(&sParentID) );
            } /* endif */
          } /* endif */

          PostMessage( hwndFrame,
                       HM_HELPSUBITEM_NOT_FOUND,
                       0,
                       MP2FROM2SHORT( sParentID, sControlID ) );
        }
        break;

      case MSGF_MENU :
        if ( (lpMsg->message == WM_KEYDOWN) && (lpMsg->wParam == VK_F1) )
        {
          SHORT   sParentID  = (SHORT)UtlQueryUShort( QS_CURMENUID );
          SHORT   sControlID = (SHORT)UtlQueryUShort( QS_CURMENUITEMID );

          PostMessage( hwndFrame,
                       HM_HELPSUBITEM_NOT_FOUND,
                       0,
                       MP2FROM2SHORT( sParentID, sControlID ) );
        }
        break;
      default:
        break;
    } /* endswitch */
  } /* endif */

  return CallNextHookEx( MessageFilterHook, nCode, wParam, (LONG)lpMsg);
}

