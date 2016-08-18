/*! \file
	Description: Do a fagued setup for MAT. Used by most of our standalone tools...

	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

/**********************************************************************/
/* Include files                                                      */
/**********************************************************************/
#include <eqf.h>                  // General Translation Manager include file
#include "eqfsetup.h"                   // MAT setup functions

/**********************************************************************/
/*  MAT Settings to use MAT utilities                                 */
/**********************************************************************/
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     SetupUtils
//------------------------------------------------------------------------------
// Function call:     SetupUtils ( HAB );
//------------------------------------------------------------------------------
// Description:       set up access to the EQF utilities, i.e. properties,
//                    error processing, etc.
//------------------------------------------------------------------------------
// Parameters:        HAB     anchor block handle
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Function flow:     set the utilities used elsewhere in the program ...
//------------------------------------------------------------------------------

BOOL SetupUtils( HAB hab, PSZ  pMsgFile )
{
  BOOL   fOK;                          // success indicator
  CHAR   szDrive[MAX_DRIVE];           // buffer for drive list
  CHAR   szLanDrive[2];                // buffer for LAN drive
  CHAR   EqfSystemMsgFile[MAX_EQF_PATH];  // global message file
  CHAR   EqfSystemHlpFile[MAX_EQF_PATH];  // global help file
  CHAR   szEqfSysLanguage[MAX_EQF_PATH];  // system language
  CHAR   szEqfResFile[MAX_EQF_PATH];      // resource

  UtlSetULong( QL_HAB, (ULONG) hab );

  UtlSetULong( QL_TWBFRAME, (ULONG) HWND_DESKTOP );
  UtlSetULong( QL_TWBCLIENT, (ULONG) HWND_DESKTOP );

  GetStringFromRegistry( APPL_Name, KEY_Drive, szDrive, sizeof( szDrive  ), "" );
  strupr( szDrive );

  GetStringFromRegistry( APPL_Name, KEY_LanDrive, szLanDrive, sizeof( szLanDrive  ), "" );
  strupr( szLanDrive );

  // Get name of current language
  GetStringFromRegistry( APPL_Name, KEY_SYSLANGUAGE, szEqfSysLanguage, sizeof( szEqfSysLanguage ), DEFAULT_SYSTEM_LANGUAGE );

  // Set name of resource, help file and message file for selected language
  fOK = UtlQuerySysLangFile( szEqfSysLanguage, szEqfResFile, EqfSystemHlpFile, EqfSystemMsgFile );

  UtlSetString( QST_PRIMARYDRIVE, szDrive );
  UtlSetString( QST_LANDRIVE,     szLanDrive );
  UtlSetString( QST_PROPDIR,      "PROPERTY" );
  UtlSetString( QST_CONTROLDIR,   "CTRL" );
  UtlSetString( QST_PROGRAMDIR,   "PGM" );
  UtlSetString( QST_DICDIR,       "DICT" );
  UtlSetString( QST_MEMDIR,       "MEM" );
  UtlSetString( QST_TABLEDIR,     "TABLE" );
  UtlSetString( QST_LISTDIR,      "LIST" );
  UtlSetString( QST_DLLDIR,       "DLL" );
  UtlSetString( QST_MSGDIR,       "MSG" );
  UtlSetString( QST_EXPORTDIR,    "EXPORT" );
  UtlSetString( QST_IMPORTDIR,    "IMPORT" );
  UtlSetString( QST_SYSTEMDIR,    "OTM" );
  UtlSetString( QST_SOURCEDIR,    "SOURCE" );
  UtlSetString( QST_TARGETDIR,    "TARGET" );
  UtlSetString( QST_SEGSOURCEDIR, "SSOURCE" );
  UtlSetString( QST_SEGTARGETDIR, "STARGET" );
  UtlSetString( QST_DIRSEGNOMATCHDIR, "SNOMATCH" );
  UtlSetString( QST_MTLOGPATH,    "MTLOG" );
  UtlSetString( QST_DIRSEGMTDIR, "MT" );
  UtlSetString( QST_DIRSEGRTFDIR, "RTF" );
  UtlSetString( QST_LOGPATH,           "LOGS" );
  UtlSetString( QST_XLIFFPATH,         "XLIFF" );
  UtlSetString( QST_METADATAPATH,      "METADATA" );
  UtlSetString( QST_JAVAPATH,          "JAVA" );
  UtlSetString( QST_ENTITY,            "ENTITY" );
  UtlSetString( QST_REMOVEDDOCDIR,     "REMOVED" );
  UtlSetString( QST_MSGFILE,       EqfSystemMsgFile );
  UtlSetString( QST_HLPFILE,       EqfSystemHlpFile );
  UtlSetString( QST_PLUGINPATH,     "PLUGINS" );

  UtlSetString( QST_RESFILE,       szEqfResFile );

  UtlInitUtils( hab );
  UtlInitError( hab, HWND_DESKTOP, NULLHANDLE, EqfSystemMsgFile );
  /********************************************************************/
  /* return message file name - if requested                          */
  /********************************************************************/
  if ( pMsgFile )
  {
    strcpy( pMsgFile, EqfSystemMsgFile );
  } /* endif */

  return fOK;
} /* end of SetupUtils */

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
   BOOL fOK = TRUE;                    // function return code
   HWND hwnd;                          // window handle of handle object window

   /*******************************************************************/
   /* Register handler window class                                   */
   /*******************************************************************/
   WinRegisterClass( (HINSTANCE) hab, pszHandler, pfnWndProc, 0L, sizeof(PVOID) );
   /*******************************************************************/
   /* Create handler object window                                    */
   /*******************************************************************/
   hwnd = WinCreateWindow( HWND_OBJECT,
                           pszHandler, pszHandler,
                           WS_OVERLAPPEDWINDOW,   // window style(s)
                           0,0,0,0,
                           HWND_DESKTOP,
                           HWND_TOP,
                           0, 0, 0 );
   fOK = ( hwnd != NULLHANDLE );

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


/**********************************************************************/
/* init the handler environment for a standalone external utility     */
/**********************************************************************/
BOOL TWBInit
(
  HAB hAB
)
{
  BOOL fOK = TRUE;
  HWND     hObjMan;                    // handle of object manager window
  PSZ      pszErrParm;
  /******************************************************************/
  /* Initialize the MAT Object Manager and anchor it,               */
  /* but first of all check if we are already running ....          */
  /******************************************************************/
  if ( (hObjMan=EqfQueryObjectManager()) == NULL )
  {
    fOK = TwbStartHandler( hAB, (PSZ)OBJECTMANAGER, OBJECTMANAGERWP, NULL );
  } /* endif */

  if ( fOK )
  {
     // check if object manager is running
     if( (hObjMan=EqfQueryObjectManager()) == NULL)   // object manager anchored ?
     {
        pszErrParm = OBJECTMANAGER;
        UtlError( ERROR_DID_NOT_START, MB_CANCEL, 1, &pszErrParm, EQF_ERROR );
        fOK = FALSE;
     } /* endif */
  } /* endif */

  /******************************************************************/
  /* start all other handlers                                       */
  /******************************************************************/
  if ( fOK ) fOK = TwbStartHandler( hAB, PROPERTYHANDLER,
                                    PROPERTYHANDLERWP, NULL );

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
      wndclass.hInstance     = (HINSTANCE) hAB;
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
      wndclass.hbrBackground = (HBRUSH) GetStockObject( WHITE_BRUSH );
      wndclass.lpszMenuName  = NULL;
      wndclass.lpszClassName = GENERICPROCESS;

      atom = RegisterClass( &wndclass );
    } /* endif */
  } /* endif */

  return fOK;
}
