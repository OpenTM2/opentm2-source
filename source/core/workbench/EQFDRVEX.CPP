//+----------------------------------------------------------------------------+
//|EQFDRVEX.C                EQF Startup Utility functions                     |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:             Gerhard Queck (XQG at SDFVM1)                           |
//|                    Christian Michel (CHM at SDFVM1)                        |
//+----------------------------------------------------------------------------+
//|Description:        This is module contains some functions which are used   |
//|                    during initialization of EQFSTART and EQFTMCOM.         |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                   TWBSetFontCharacteristics                                |
//|                   TWBDrives              Activate drives dialog            |
//|                   TWBCheckExpirationDate Check expiration date             |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                   TWBDrivesDlgProc                                         |
//|                   TWBDrivesInit          Process WM_INITDLG of drive dlg   |
//|                   TWBDrivesCommand       Process WM_COMMAND of drive dlg   |
//|                   TWBDrivesClose         Process WM_CLOSE of drive dlg     |
//|                   fCheckForRemoteTms     Check drive for remote TMs        |
//+----------------------------------------------------------------------------+

#include <eqf.h>                  // General Translation Manager include file

//#define  EQF_COM_MAIN_SOURCE      // define controlling included data
//#define  TM_ADM                   // define controlling included data

#include <eqf.id>                 // general EQF ids
#include <eqftwb.id>              // ids for Troja workbench
#include <eqfstart.id>            // ids for EQFRES
#include <eqfserno.h>             // definition of expiration date
#include <eqfsetup.h>             // definition of EQF directories (used for
// EQFTMCOM functions only)
#include <eqfdrvex.h>             // type definitions and function prototypes
#include <time.h>                 // C time functions
#include <eqftp.h>                // TAG defines; required for EQFTMI.H
#include <eqftag.h>               // TAG defines; required for EQFTMI.H
#include <eqftm.h>                // general TM definitions
#include <eqftmi.h>               // private TM definitions
#include "commctrl.h"           // common controls
#include "eqfstart.h"           // EQFSTART defines
#include "SHLOBJ.H"             // shell functions
#include "vector"
#include "string"
#include "core\PluginManager\PluginManager.h"
#include "core\memory\MemoryFactory.h"
#include "core\utilities\LanguageFactory.H"

// prototype for non-public utility functions
VOID UtlSetString( SHORT sID, PSZ pszString );


extern HELPSUBTABLE hlpsubtblConnectDicDlg[];
extern HELPSUBTABLE hlpsubtblConnectTMDlg[];

void SetPercentValue
(
  HWND   hwndDlg,                      // handle of dialog window
  int    iControlID,                   // ID of entryfield
  int    iValue                        // percentage value (0..99)
);
UINT GetPercentValue
(
  HWND   hwndDlg,                      // handle of dialog window
  int    iControlID,                   // ID of entryfield
  BOOL   *pfOK                         // success indicator or NULL
);




//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBSetFontCharacteristics                                |
//+----------------------------------------------------------------------------+
//|Function call:     TWBSetFontCharacteristics()                              |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     get desktop presentation space handle                    |
//|                   get font metrics for default font                        |
//|                   set Utl long variables concerning font characteristics   |
//|                   release presentation space                               |
//+----------------------------------------------------------------------------+
VOID TWBSetFontCharacteristics( VOID )
{
  HPS           hPS;                  // desktop presentation space
  FONTMETRICS   FontMetrics;          // structure for font metrics

  /*******************************************************************/
  /* get desktop presentation space handle                           */
  /*******************************************************************/
  hPS = GETPS( HWND_DESKTOP );

  /*******************************************************************/
  /* get font metrics                                                */
  /*******************************************************************/
  GpiQueryFontMetrics( hPS, (LONG)sizeof( FontMetrics ), &FontMetrics );

  /*******************************************************************/
  /* set global variables                                            */
  /*******************************************************************/
  UtlSetULong( QL_PELSPERLINE, FontMetrics.tmHeight +
               FontMetrics.tmExternalLeading );
  UtlSetULong( QL_AVECHARWIDTH, FontMetrics.tmAveCharWidth );
  UtlSetULong( QL_CHARHEIGHT, FontMetrics.tmHeight );
  UtlSetULong( QL_EMCHARWIDTH, FontMetrics.tmAveCharWidth );

  /*******************************************************************/
  /* release desktop presentation space                              */
  /*******************************************************************/
  RELEASEPS( HWND_DESKTOP, hPS );

} /* end of TWBSetFontCharacteristics */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBDrives              Activate drives dialog            |
//+----------------------------------------------------------------------------+
//|Function call:     TWBDrives( HWND hwnd, HMODULE hResourceMod,              |
//|                              BOOL fEqfTmCom )                              |
//+----------------------------------------------------------------------------+
//|Description:       Activate the drive configuration dialog                  |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND    hwnd         handle of parent window             |
//|                   HMODULE hResourceMod Module containing dialog resources  |
//|                   BOOL    fEqfTmCom    TRUE when called by TM com code     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     fill drives dialog init structure                        |
//|                   use WinDlgBox to activate drives dialog                  |
//+----------------------------------------------------------------------------+
VOID TWBDrives
(
HWND     hwnd,                       // handle of parent window
HMODULE  hResourceMod,               // Module containing dialog resources
BOOL     fEqfTmCom                   // TRUE when called by TM com code
)
{
  DRIVES_DLG_INIT DrivesInit;
  INT_PTR           sRC;

  DrivesInit.usLength = sizeof (DRIVES_DLG_INIT);
  DrivesInit.usReserved = 0;
  DrivesInit.hResourceMod = hResourceMod;
  DrivesInit.fEqfTmCom = fEqfTmCom;

  DIALOGBOX( hwnd,
             TWBDrivesDlgProc,
             hResourceMod,
             ID_TWBDRIVES_DLG,
             MP2FROMP(&DrivesInit),
             sRC );
} /* end of function TWBDrives */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBDrivesDlgProc                                         |
//+----------------------------------------------------------------------------+
//|Function call:     TWBDrivesDlgProc( HWND hwndDlg, USHORT msg, MPARAM mp1,  |
//|                                     PARAM mp2 )                            |
//+----------------------------------------------------------------------------+
//|Description:       Dialog to allow attach or detach of secondary MAT        |
//|                   drives                                                   |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwndDlg  handle of dialog                         |
//|                   USHORT msg      type of message                          |
//|                   MPARAM mp1      first message parameter                  |
//|                   MPARAM mp2      second message parameter                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       depends on message type                                  |
//|                   normal return codes are:                                 |
//|                   TRUE = message has been processed                        |
//|                   FALSE = message has not been processed                   |
//+----------------------------------------------------------------------------+
//|Function flow:     switch msg                                               |
//|                     case WM_INITDLG:                                       |
//|                       call TWBDrivesInit to process message                |
//|                     case WM_EQF_INITIALIZE:                                |
//|                       call TWBDrivesInitialize to process message          |
//|                     case WM_COMMAND:                                       |
//|                       call TWBDrivesCommand to process message             |
//|                     case WM_CLOSE:                                         |
//|                       call TWBDrivesClose to process message               |
//|                     default:                                               |
//|                       pass message to default dialog procedure             |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK TWBDrivesDlgProc
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_TWBDRIVES_DLG, mp2 ); break;

    case WM_INITDLG:
      mResult = TWBDrivesInit( hwndDlg, mp1, mp2 );
      break;

    case WM_EQF_INITIALIZE:
      mResult = TWBDrivesInitialize( hwndDlg, mp1, mp2 );
      break;

    case WM_COMMAND:
      mResult = TWBDrivesCommand( hwndDlg, mp1, mp2 );
      break;

    case WM_CLOSE:
      mResult = TWBDrivesClose( hwndDlg, mp1, mp2 );
      break;

    default:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
} /* end of function TWBDrivesDlgProc */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBDrivesInit          Process WM_INITDLG of drive dlg   |
//+----------------------------------------------------------------------------+
//|Function call:     TWBDrivesInit( HWND hwndDlg, MPARAM mp1, MPARAM mp2 );   |
//+----------------------------------------------------------------------------+
//|Description:       Process command message of drives dialog: handle set     |
//|                   pushbutton, cancel pushbutton and drive buttons.         |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwndDlg         handle of dialog window           |
//|                   MPARAM mp1             first message parameter           |
//|                   MPARAM mp2             second message parameter          |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE (always)                                           |
//+----------------------------------------------------------------------------+
//|Function flow:     create and anchor IDA                                    |
//|                   if dialog is used by TM com code then                    |
//|                     set flag in IDA                                        |
//|                     load text strings for usage under TM com               |
//|                   endif                                                    |
//|                   get primary drive and create drive button for it         |
//|                   create buttons for secondary drives                      |
//|                   in case of errors dismiss the dialog                     |
//+----------------------------------------------------------------------------+
MRESULT TWBDrivesInit
(
HWND    hwndDlg,                    // handle of dialog window
WPARAM  mp1,                        // first parameter of WM_INITDLG
LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  MRESULT     mResult = FALSE;        // result of message processing
  PPROPSYSTEM pSysProp;               // ptr to EQF system properties
  BOOL        fOK = TRUE;             // internal OK flag
  CHAR        szTempList[MAX_DRIVELIST];       // temporary drive list
  HWND        hwndDrive;              // handle of a drive button
  PSZ         pszFrom, pszTo;         // ptrs for drive list processing
  PTWBDRIVESIDA  pIda;                // dialog IDA
  CHAR        szSecDrives[MAX_DRIVELIST];// buffer for EQF drives list

  mp1 = mp1;                          // avoid compiler warning
  mp2 = mp2;                          // avoid compiler warning

  /*******************************************************************/
  /* create and anchor IDA                                           */
  /*******************************************************************/
  fOK = UtlAlloc( (PVOID *) &pIda, 0L, (LONG) sizeof(TWBDRIVESIDA), ERROR_STORAGE );
  if ( fOK )
  {
    ANCHORDLGIDA( hwndDlg, pIda );
  } /* endif */

  /*******************************************************************/
  /* set flag indicating usage by Troja or TM Com Code               */
  /*******************************************************************/
  if ( fOK )
  {
    pIda->fEqfTmCom = ((PDRIVES_DLG_INIT) mp2)->fEqfTmCom;
    pIda->hResourceMod = ((PDRIVES_DLG_INIT) mp2)->hResourceMod;
  } /* endif */

  /*******************************************************************/
  /* get primary drive and create drive button for it                */
  /*******************************************************************/
  if ( fOK )
  {
    pSysProp = (PPROPSYSTEM) MakePropPtrFromHnd( EqfQuerySystemPropHnd());
    pIda->chPrimaryDrive = pSysProp->szPrimaryDrive[0];

    szTempList[0] = pIda->chPrimaryDrive;
    szTempList[1] = EOS;

    UtlDriveButtons( hwndDlg, szTempList,
                     ID_TODRIVEA_BTN,
                     WS_GROUP | WS_TABSTOP,
                     WS_VISIBLE,
                     GETHANDLEFROMID(hwndDlg,ID_TWBDRIVES_PRIMDRIVE_GB),
                     GETHANDLEFROMID(hwndDlg,ID_TWBDRIVES_PRIMDUMMY_PB),
                     NULLHANDLE );

    hwndDrive = WinWindowFromID( hwndDlg,
                                 IDFROMDRIVE( ID_TODRIVEA_BTN, pIda->chPrimaryDrive ));
    WinEnableWindow( hwndDrive, FALSE );
    SETDRIVE( hwndDlg,
              IDFROMDRIVE( ID_TODRIVEA_BTN, pIda->chPrimaryDrive ),
              TRUE );
  } /* endif */

  /*******************************************************************/
  /* create drive buttons for secondary drives                       */
  /*******************************************************************/
  if ( fOK )
  {
    UtlGetDriveList( (PBYTE)szTempList );
    pszFrom = szTempList;
    pszTo   = pIda->szDrives;
    while ( *pszFrom )
    {
      if ( (*pszFrom != 'A') &&
           (*pszFrom != 'B') &&
           (*pszFrom != pIda->chPrimaryDrive) )
      {
        *pszTo++ = *pszFrom;
      } /* endif */
      pszFrom++;
    } /* endwhile */
    *pszTo = EOS;
    UtlDriveButtons( hwndDlg, pIda->szDrives,
                     PID_DRIVEBUTTON_A,
                     WS_GROUP | WS_TABSTOP,
                     WS_VISIBLE,
                     GETHANDLEFROMID(hwndDlg,ID_TWBDRIVES_ADDDRIVE_GB),
                     GETHANDLEFROMID(hwndDlg,ID_TWBDRIVES_ADDDUMMY_PB),
                     NULLHANDLE );

    // set drive button states
    pszFrom = pIda->szDrives;
    UtlGetCheckedEqfDrives( szSecDrives );

    while ( *pszFrom )
    {
      if ( strchr( szSecDrives, *pszFrom ) )
      {
        SETDRIVE( hwndDlg,
                  IDFROMDRIVE( PID_DRIVEBUTTON_A, *pszFrom ),
                  TRUE );
        pIda->afDrives[*pszFrom-'A'] = TRUE;
      }
      else
      {
        pIda->afDrives[*pszFrom-'A'] = FALSE;
      } /* endif */
      pszFrom++;
    } /* endwhile */
  } /* endif */

  if ( fOK )
  {
    WinPostMsg( hwndDlg, WM_EQF_INITIALIZE, 0L, 0L );
    mResult = WinDefDlgProc( hwndDlg, WM_INITDLG, mp1, mp2);
  }
  else
  {
    WinDismissDlg( hwndDlg, FALSE );
  } /* endif */
  return( mResult );
} /* end of TWBDrivesInit */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBDrivesInitialze     Process WM_EQF_INITIALZE          |
//+----------------------------------------------------------------------------+
//|Function call:     TWBDrivesInitialze(HWND hwndDlg, MPARAM mp1, MPARAM mp2);|
//+----------------------------------------------------------------------------+
//|Description:       Process second part of dialog initialization:            |
//|                   Issue error message for inaccessible drives              |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwndDlg         handle of dialog window           |
//|                   MPARAM mp1             first message parameter           |
//|                   MPARAM mp2             second message parameter          |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE (always)                                           |
//+----------------------------------------------------------------------------+
//|Function flow:     compare valid drives list and secondary drives list      |
//|                     and issue warning message for inaccessible drives      |
//+----------------------------------------------------------------------------+
MRESULT TWBDrivesInitialize
(
HWND    hwndDlg,                    // handle of dialog window
WPARAM  mp1,                        // first parameter of WM_INITDLG
LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  MRESULT     mResult = FALSE;        // result of message processing
  BOOL        fOK = TRUE;             // internal OK flag
  PSZ         pszFrom;                // ptrs for drive list processing
  PTWBDRIVESIDA  pIda;                // dialog IDA
  CHAR        szTempString [80];      // temporary used string
  CHAR        szValidDrives[MAX_DRIVELIST]; // buffer for valid EQF drives
  PPROPSYSTEM    pPropSys;            // system properties ptr
  PSZ            pszErrParm;          // ptr to UtlError parameter string

  mp1 = mp1;                          // avoid compiler warning
  mp2 = mp2;                          // avoid compiler warning

  /*************************************************************/
  /* access dialog IDA                                         */
  /*************************************************************/
  pIda = ACCESSDLGIDA( hwndDlg, PTWBDRIVESIDA );

  /*******************************************************************/
  /* Get original and new drive list                                 */
  /*******************************************************************/
  pPropSys = GetSystemPropPtr();
  pszFrom = pPropSys->szDriveList;
  UtlGetCheckedEqfDrives( szValidDrives );

  while ( fOK && *pszFrom )
  {
    /*******************************************************/
    /* check if drive is still in drive list               */
    /*******************************************************/
    if ( strchr( szValidDrives, *pszFrom ) == NULL )
    {
      /****************************************************/
      /* drive is missing from list, display inaccessible */
      /* drive message                                    */
      /****************************************************/
      szTempString[0] = *pszFrom;
      szTempString[1] = COLON;
      szTempString[2] = EOS;
      pszErrParm      = szTempString;
      UtlError( ERROR_EQF_DRIVE_NOT_VALID,
                MB_OK, 1, &pszErrParm,
                EQF_WARNING );
    } /* endif */
    pszFrom++;
  } /* endwhile */

  return( mResult );
} /* end of TWBDrivesInitialize */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBDrivesCommand       Process WM_COMMAND of drive dlg   |
//+----------------------------------------------------------------------------+
//|Function call:     TWBDrivesCommand( HWND hwndDlg, MPARAM mp1, MPARAM mp2 );|
//+----------------------------------------------------------------------------+
//|Description:       Process command message of drives dialog: handle set     |
//|                   pushbutton, cancel pushbutton and drive buttons.         |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwndDlg         handle of dialog window           |
//|                   MPARAM mp1             first message parameter           |
//|                   MPARAM mp2             second message parameter          |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE  (always)                                           |
//+----------------------------------------------------------------------------+
//|Function flow:     switch command value                                     |
//|                     case set drives pushbutton:                            |
//|                       access IDA                                           |
//|                       build new drives list                                |
//|                       query if detached drive can still be detached        |
//|                       if ok store in new drives in system properties or    |
//|                        in OS2.INI and close dialog                         |
//|                     case cancel pushbutton or ESCAPE key:                  |
//|                       close dialog                                         |
//|                     default:                                               |
//|                       if command ID is from a drive button then            |
//|                         if drive is detached then                          |
//|                           check if drive can be detached                   |
//|                         else                                               |
//|                           try to create required directories               |
//|                         endif                                              |
//|                         if ok change drive state                           |
//|                       endif                                                |
//+----------------------------------------------------------------------------+
MRESULT TWBDrivesCommand
(
HWND   hwndDlg,                     // handle of dialog window
WPARAM mp1,                         // first message parameter
LPARAM mp2                          // second message parameter
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
  PTWBDRIVESIDA  pIda;                // dialog IDA
  CHAR           chDrive;             // currently processed drive
  BOOL           fDenyDetach;         // flag for 'do not detach drive'
  CHAR           szDrive[2];          // drive as null terminated string
  PSZ            pszErrParm;          // ptr to UtlError parameter string
  HPROP          hPropSys;            // system properties handle
  PPROPSYSTEM    pPropSys = NULL;     // system properties ptr
  PSZ            pszFrom, pszTo;      // ptrs for drive list processing
  CHAR           szOldDrives[MAX_DRIVELIST]; // buffer for old drive list
  CHAR           szNewDrives[MAX_DRIVELIST]; // buffer for new drive list
  CHAR           szChgDrives[MAX_DRIVELIST]; // buffer for changed drive list
  EQFINFO        ErrorInfo;           // error code set by property handler
  BOOL           fOK = TRUE;          // internal OK flag
  CHAR           szPath[MAX_EQF_PATH];// buffer for EQF paths
  USHORT         usRC;                // return code of Utlxxxx functions

  switch ( WMCOMMANDID( mp1, mp2 ) )
  {
    case ID_TWBDRIVES_HELP_PB:
      UtlInvokeHelp();
      break;

    case ID_TWBDRIVES_SET_PB:
      /*************************************************************/
      /* access dialog IDA                                         */
      /*************************************************************/
      pIda = ACCESSDLGIDA( hwndDlg, PTWBDRIVESIDA );

      /*************************************************************/
      /* build new drive list                                      */
      /*************************************************************/
      if ( fOK )
      {
        pszFrom = pIda->szDrives;
        pszTo   = szNewDrives;
        *pszTo++ = pIda->chPrimaryDrive;
        while ( *pszFrom )
        {
          if ( pIda->afDrives[*pszFrom-'A'] )
          {
            *pszTo++ = *pszFrom;
          } /* endif */
          pszFrom++;
        } /* endwhile */
        *pszTo = EOS;
      } /* endif */

      /*************************************************************/
      /* query if detached drives can still be detached            */
      /*************************************************************/
      if ( fOK )
      {
        pPropSys = GetSystemPropPtr();
        pszFrom = pPropSys->szDriveList;
        strcpy( szOldDrives, pszFrom );

        while ( fOK && *pszFrom )
        {
          /*******************************************************/
          /* check if drive is still in drive list               */
          /*******************************************************/
          if ( strchr( szNewDrives, *pszFrom ) == NULL )
          {
            /****************************************************/
            /* drive is to be removed, now ask if anyone might g*/
            /* problems with the remove of the drive ...        */
            /****************************************************/
            fDenyDetach = EqfSend2AllHandlers( WM_EQF_ABOUTTOREMOVEDRIVE,
                                               MP1FROMCHAR( *pszFrom ),
                                               0L );

            if ( fDenyDetach )
            {
              szDrive[0] = *pszFrom;
              szDrive[1] = EOS;
              pszErrParm = szDrive;
              UtlError( ERROR_DRIVE_IN_USE, MB_CANCEL, 1, &pszErrParm,
                        EQF_ERROR );
              fOK = FALSE;
            } /* endif */
          } /* endif */
          pszFrom++;
        } /* endwhile */
      } /* endif */

      /*************************************************************/
      /* if ok access system propertys for WRITE                   */
      /*************************************************************/
      hPropSys = EqfQuerySystemPropHnd();
      if ( !SetPropAccess( hPropSys, PROP_ACCESS_WRITE))
      {
        UtlError( ERROR_ACCESS_SYSTEMPROPERTIES, MB_CANCEL,
                  0, (PSZ *) NULP, EQF_ERROR);
        fOK = FALSE;
      } /* endif */

      /*************************************************************/
      /* if ok update system properties                            */
      /*************************************************************/
      if ( fOK )
      {
        strcpy( pPropSys->szDriveList, szNewDrives );
        SaveProperties( EqfQuerySystemPropHnd(), &ErrorInfo);
        ResetPropAccess( EqfQuerySystemPropHnd(), PROP_ACCESS_WRITE );
        UtlSetString( QST_ORGEQFDRIVES, szNewDrives );
        UtlSetString( QST_VALIDEQFDRIVES, szNewDrives );
      } /* endif */

      /*************************************************************/
      /* if ok build removed drives list and send drives-removed   */
      /* msg                                                       */
      /*************************************************************/
      if ( fOK && !pIda->fEqfTmCom)
      {
        pszFrom = szOldDrives;
        pszTo   = szChgDrives;
        while ( *pszFrom )
        {
          if ( strchr( szNewDrives, *pszFrom ) == NULL )
          {
            *pszTo++ = *pszFrom;
          } /* endif */
          pszFrom++;
        } /* endwhile */
        *pszTo = EOS;
        if ( szChgDrives[0] != EOS )
        {
          EqfSend2AllHandlers( WM_EQFN_DRIVEREMOVED,
                               MP1FROMSHORT( 0 ),
                               MP2FROMP( szChgDrives ) );
        } /* endif */
      } /* endif */

      /*************************************************************/
      /* if ok build added drives list and send drives-added msg   */
      /*************************************************************/
      if ( fOK && !pIda->fEqfTmCom)
      {
        pszFrom = szNewDrives;
        pszTo   = szChgDrives;
        while ( *pszFrom )
        {
          if ( strchr( szOldDrives, *pszFrom ) == NULL )
          {
            *pszTo++ = *pszFrom;
          } /* endif */
          pszFrom++;
        } /* endwhile */
        *pszTo = EOS;
        if ( szChgDrives[0] != EOS )
        {
          EqfSend2AllHandlers( WM_EQFN_DRIVEADDED,
                               MP1FROMSHORT( 0 ),
                               MP2FROMP( szChgDrives ) );
        } /* endif */
      } /* endif */

      /*************************************************************/
      /* if ok close dialog                                        */
      /*************************************************************/
      if ( fOK )
      {
        WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT(TRUE), 0L );
      } /* endif */
      break;

    case ID_TWBDRIVES_CANCEL_PB:
    case DID_CANCEL:
      /*************************************************************/
      /* Broadcast drive-removed message even if dialog is         */
      /* cancelled                                                 */
      /*************************************************************/

      /*************************************************************/
      /* access dialog IDA                                         */
      /*************************************************************/
      pIda = ACCESSDLGIDA( hwndDlg, PTWBDRIVESIDA );

      /*************************************************************/
      /* Get original and new drive list                           */
      /*************************************************************/
      pPropSys = GetSystemPropPtr();
      strcpy( szOldDrives, pPropSys->szDriveList );
      UtlGetCheckedEqfDrives( szNewDrives );

      /*************************************************************/
      /* build removed drives list and send drives-removed msg     */
      /*************************************************************/
      pszFrom = szOldDrives;
      pszTo   = szChgDrives;
      while ( *pszFrom )
      {
        if ( strchr( szNewDrives, *pszFrom ) == NULL )
        {
          *pszTo++ = *pszFrom;
        } /* endif */
        pszFrom++;
      } /* endwhile */
      *pszTo = EOS;
      if ( szChgDrives[0] != EOS )
      {
        EqfSend2AllHandlers( WM_EQFN_DRIVEREMOVED,
                             MP1FROMSHORT(0),
                             MP2FROMP( szChgDrives ) );
      } /* endif */



      WinPostMsg( hwndDlg, WM_CLOSE, 0L, 0L );
      break;

    default:
      pIda = ACCESSDLGIDA( hwndDlg, PTWBDRIVESIDA );

      /**************************************************************/
      /* check for drive buttons                                    */
      /**************************************************************/
      if ( ( WMCOMMANDID( mp1, mp2 ) >= PID_DRIVEBUTTON_A )  &&
           ( WMCOMMANDID( mp1, mp2 ) <= PID_DRIVEBUTTON_Z )  )
      {
        /***********************************************************/
        /* set buttons new state                                   */
        /***********************************************************/
        chDrive = DRIVEFROMID( PID_DRIVEBUTTON_A, WMCOMMANDID( mp1, mp2 ) );
        if ( pIda->afDrives[chDrive-'A'] )    // do we detach a drive ???
        {
          fDenyDetach = EqfSend2AllHandlers( WM_EQF_ABOUTTOREMOVEDRIVE,
                                             MP1FROMCHAR( chDrive ),
                                             0L );

          if ( fDenyDetach )
          {
            szDrive[0] = chDrive;
            szDrive[1] = EOS;
            pszErrParm = szDrive;
            UtlError( ERROR_DRIVE_IN_USE,
                      MB_CANCEL, 1, &pszErrParm,
                      EQF_ERROR );
          }
          else
          {
            pIda->afDrives[chDrive-'A'] = ! pIda->afDrives[chDrive-'A'];
            SETDRIVE( hwndDlg,
                      IDFROMDRIVE( PID_DRIVEBUTTON_A, chDrive ),
                      (pIda->afDrives[chDrive-'A']) );
          } /* endif */
        }
        else
        {
          pPropSys = GetSystemPropPtr();
          pszFrom = pPropSys->szDriveList;

          /********************************************************/
          /* try to create the required EQF directories           */
          /********************************************************/
          usRC = 0;
          UtlMakeEQFPath( szPath, chDrive, DIC_PATH, NULL );
          if ( !UtlDirExist( szPath ) )
          {
            usRC = UtlMkMultDir( szPath, FALSE );
          } /* endif */
          if ( !usRC )
          {
            UtlMakeEQFPath( szPath, chDrive, MEM_PATH, NULL );
            if ( !UtlDirExist( szPath) )
            {
              usRC = UtlMkMultDir( szPath, FALSE );
            } /* endif */
          } /* endif */
          if ( !usRC )
          {
            UtlMakeEQFPath( szPath, chDrive, TABLE_PATH, NULL );
            if ( !UtlDirExist( szPath) )
            {
              usRC = UtlMkMultDir( szPath, FALSE );
            } /* endif */
          } /* endif */
          if ( !usRC )
          {
            UtlMakeEQFPath( szPath, chDrive, LIST_PATH, NULL );
            if ( !UtlDirExist( szPath) )
            {
              usRC = UtlMkMultDir( szPath, FALSE );
            } /* endif */
          } /* endif */

          if ( usRC )
          {
            szDrive[0] = chDrive;
            szDrive[1] = EOS;
            pszErrParm = szDrive;
            UtlError( ERROR_EQF_DRIVE_NOT_ACCESSIBLE,
                      MB_CANCEL, 1, &pszErrParm, EQF_ERROR );
          }
          else
          {
            pIda->afDrives[chDrive-'A'] = ! pIda->afDrives[chDrive-'A'];
            SETDRIVE( hwndDlg,
                      IDFROMDRIVE( PID_DRIVEBUTTON_A, chDrive ),
                      (pIda->afDrives[chDrive-'A']) );
          } /* endif */
        } /* endif */
      } /* endif */
      break;
  } /* endswitch */

  mp2 = mp2;                          // supress 'unreferenced parameter' msg

  return( mResult );
} /* end of TWBDrivesCommand */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBDrivesClose         Process WM_CLOSE of drive dlg     |
//+----------------------------------------------------------------------------+
//|Function call:     TWBDrivesClose( HWND hwndDlg, MPARAM mp1, MPARAM mp2 );  |
//+----------------------------------------------------------------------------+
//|Description:       Process close message of drives dialog by freeing the    |
//|                   IDA storage and dismissing the dialog                    |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwndDlg         handle of dialog window           |
//|                   MPARAM mp1             first message parameter           |
//|                   MPARAM mp2             second message parameter          |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE (always)                                           |
//+----------------------------------------------------------------------------+
//|Side effects:      storage area of IDA is freed                             |
//|                   dialog is dismissed                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     free IDA                                                 |
//|                   dismiss dialog                                           |
//+----------------------------------------------------------------------------+
MRESULT TWBDrivesClose
(
HWND   hwndDlg,                     // handle of dialog window
WPARAM mp1,                         // first message parameter
LPARAM mp2                          // second message parameter
)
{
  MRESULT mResult = FALSE;            // result of message processing
  PTWBDRIVESIDA  pIda;                // dialog IDA

  mp1;                                // supress 'unreferenced parameter' msg
  mp2;                                //               "

  /*******************************************************************/
  /* free all of the allocated storage                               */
  /*******************************************************************/
  pIda = ACCESSDLGIDA( hwndDlg, PTWBDRIVESIDA );
  if ( pIda )
  {
    UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
  } /* endif */

  /*******************************************************************/
  /* get rid off Drives dialog                                       */
  /*******************************************************************/
  WinDismissDlg( hwndDlg, TRUE );

  return( mResult );
} /* end of function TWBDrivesClose */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBCheckExpirationDate Check expiration date             |
//+----------------------------------------------------------------------------+
//|Function call:     TWBCheckExpirationDate( szEqfResFile, szSysPopFile );    |
//+----------------------------------------------------------------------------+
//|Description:       Check if the current program version is expired.         |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ - pointer to EQFRES.DLL filename                     |
//|                   PSZ - pointer to path of system property file            |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     version is not expired                          |
//|                   FALSE    program version is expired                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:      the constant STR_DRIVER_EXP_DATE is in the form          |
//|                   MM/DD/YYYY                                               |
//+----------------------------------------------------------------------------+
//|Function flow:     Split program expiration date in day, month and year     |
//|                   get current date using DosGetDateTime                    |
//|                   set expired flag if expiration date has been exceeded    |
//|                    and display error message                               |
//+----------------------------------------------------------------------------+
BOOL TWBCheckExpirationDate ( PSZ pszEqfResFile, PSZ pszSysProps )
{
  BOOL fExpired = FALSE;              // password is expired flag
  ULONG   ulRC;
  LONG    lCurrentTime;              // current time
  CHAR    szTempString[40];
  CHAR    szDateFileName[MAX_PATH144];
  HFILE   hExpDateFile = NULLHANDLE;
  HMODULE hmodRes = NULLHANDLE;
  HMODULE hmodLogo = NULLHANDLE;
  USHORT  usAction = 0;
  EXP_DATE sExpDate;
  time_t tmpTime;
  struct tm *psTime;
  ULONG    ulCurrentDate = 0L;

  /*******************************************************************/
  /* assemble filename for file containing expiration date           */
  /*******************************************************************/
  {
    PSZ pszFName;

    strcpy( szDateFileName, pszSysProps );
    pszFName = UtlGetFnameFromPath( szDateFileName );
    if ( pszFName != NULL )
    {
    }
    else
    {
      pszFName = szDateFileName;
    } /* endif */
    strcpy( pszFName, EXP_DATE_FILENAME );
  }

  /*******************************************************************/
  /* Clear hidden, system and readonly flag to allow access to the   */
  /* file                                                            */
  /*******************************************************************/
  SetFileAttributes( szDateFileName, FILE_ATTRIBUTE_NORMAL );

  /* now try to open the file */
  ulRC = UtlOpen( szDateFileName,        // filename
                  &hExpDateFile,         // file handle
                  &usAction,             // action taken
                  0L,                    // create file size
                  FILE_NORMAL,           // file attributes
                  FILE_OPEN,             // open if exists, else fail
                  OPEN_SHARE_DENYWRITE | // deny write access
                  OPEN_ACCESS_READWRITE,
                  0L,                    // reserved
                  FALSE );               // display no messages

  if ( ulRC != NO_ERROR )
  {
    /* file was not found, create it */
    ulRC = UtlOpen( szDateFileName,      // filename
                    &hExpDateFile,       // file handle
                    &usAction,           // action taken
                    0L,                  // create file size
                    FILE_NORMAL,         // file attributes
                    FILE_CREATE,         // open if exists, else fail
                    OPEN_SHARE_DENYWRITE | // deny write access
                    OPEN_ACCESS_READWRITE,
                    0L,                  // reserved
                    FALSE );             // display no messages

    if ( ulRC != NO_ERROR )
    {
      /* couldn't create file, so we don't check for expiration */
      return TRUE;
    } /* endif */

    /* since file is new, preset the values */
    sExpDate.sVersion = VERSION_OFFSET;
    sExpDate.ulInstDate = 0L;
    sExpDate.ulExpDate = 0L;
  }
  else
  {
    ULONG ulBytesRead;

    /* read expiration structure */
    UtlReadL( hExpDateFile, &sExpDate, sizeof( EXP_DATE ),
             &ulBytesRead, FALSE );
    if ( ulBytesRead != sizeof( EXP_DATE ) )
    {
      /* since structure could not be read, preset the values */
      sExpDate.sVersion = VERSION_OFFSET;
      sExpDate.ulInstDate = 0L;
      sExpDate.ulExpDate = 0L;
    } /* endif */
  } /* endif */

  /* get current time */
  UtlTime( &lCurrentTime );           // get current time
  tmpTime = (time_t) lCurrentTime;
  psTime = gmtime( &tmpTime );
  ulCurrentDate = (ULONG) psTime->tm_mday +
                  (ULONG) (psTime->tm_mon + 1) * MONTH_MULTIPLICATOR +
                  (ULONG) (psTime->tm_year + 1900) * YEAR_MULTIPLICATOR;

  // load generic resource for windows ...
  DosLoadModule( NULL, NULLHANDLE, pszEqfResFile, &hmodRes );
  DosLoadModule( NULL, NULLHANDLE, EQFLOGOR_DLL, &hmodLogo );

  ulRC = LOADSTRING( NULLHANDLE, hmodLogo, SID_TM_FULL_VERSION, szTempString );

  {
    // no expiration date for OpenTM2
    fExpired = FALSE;                            // no expiration
  } /* endif */

  if ( !fExpired )
  {
    /* check version strings in logo DLL and ressource DLL */
    CHAR szLogoVersion[40];

    ulRC = LOADSTRING( NULLHANDLE, hmodLogo, SID_TM_DRIVER_LEVEL, szLogoVersion );
    if ( ulRC )
    {
      ulRC = LOADSTRING( NULLHANDLE, hmodRes, SID_LOGO_REVISION, szTempString );
    } /* endif */

    if ( ulRC )
    {
//     /***************************************************************/
//     /* this check is disabled for DBCS resource, because we have   */
//     /* to build the resource dll on a separate machine....         */
//     /***************************************************************/
//     if ( stricmp( szTempString, szLogoVersion ) != 0 )
//     {
//       fExpired = TRUE;
//     } /* endif */
    } /* endif */
  } /* endif */

  /* write expiration structure */
  {
    ULONG  ulNewOffset;
    USHORT usBytesWritten;

    UtlChgFilePtr( hExpDateFile, 0L, FILE_BEGIN, &ulNewOffset, FALSE );
    UtlWrite( hExpDateFile, &sExpDate, sizeof( EXP_DATE ),
              &usBytesWritten, FALSE );
    UtlClose( hExpDateFile, FALSE );
    SetFileAttributes( szDateFileName, FILE_ATTRIBUTE_HIDDEN |
                       FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY );
  }

  /*******************************************************************/
  /* Display '... is expired' message                                */
  /*******************************************************************/
  if ( fExpired )
  {
    USHORT   usDay;
    USHORT   usMonth;
    USHORT   usYear;
    ULONG    ulTemp;
    CHAR     szDate[20];
    PSZ      pReplAddr;

    /* get the installation date from structure */
    ulTemp = sExpDate.ulInstDate;
    usYear = (USHORT) (ulTemp / YEAR_MULTIPLICATOR);
    ulTemp %= YEAR_MULTIPLICATOR;
    usMonth = (USHORT) (ulTemp / MONTH_MULTIPLICATOR);
    ulTemp %= MONTH_MULTIPLICATOR;
    usDay = (USHORT) ulTemp;
    sprintf( szDate, "%2.2d/%2.2d/%4.4d", usMonth, usDay, usYear );
    pReplAddr = szDate;

    UtlError( ERROR_PGM_VERSION_EXPIRED, MB_CANCEL, 1, &pReplAddr, EQF_ERROR );
  } /* endif */

  DosFreeModule ( hmodLogo );
  DosFreeModule ( hmodRes );

  return( !fExpired );
} /* end of function TWBCheckExpirationDate */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     fCheckForRemoteTms     Check drive for remote TMs        |
//+----------------------------------------------------------------------------+
//|Function call:     fCheckForRemoteTms( CHAR cDrive );                       |
//+----------------------------------------------------------------------------+
//|Description:       Check if a drive contains remote TMs                     |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR    cDrive         drive letter                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    drive contains remote TMs                        |
//|                   FALSE   no remote TMs found on drive                     |
//+----------------------------------------------------------------------------+
//|Function flow:     build path to remote TMs                                 |
//|                   find first TM (UtlFindFirst)                             |
//|                   return result of UtlFindFirst                            |
//+----------------------------------------------------------------------------+
BOOL fCheckForRemoteTms (CHAR cDrive)
{
  USHORT         usSearchCount,       // how much dir entries to search
  usRc;                // return code of UtlFindFirst
  CHAR           szMemPath[MAX_EQF_PATH];    // search path for mems.
  HDIR           hSearch;             // dir handle for UtlFindFirst
  FILEFINDBUF    stFile;              // output buffer of UtlFindFirst

  /*******************************************************************/
  /* Build path for remote TMs                                       */
  /*******************************************************************/
  UtlMakeEQFPath( szMemPath, cDrive, COMMEM_PATH, NULL );
  strcat( szMemPath, BACKSLASH_STR );
  strcat( szMemPath, DEFAULT_PATTERN );

  /*******************************************************************/
  /* look for files                                                  */
  /*******************************************************************/
  usSearchCount = 1;                       // find only one file
  hSearch = HDIR_CREATE;                   // Allocate a new, unused handle

  usRc = UtlFindFirst( szMemPath, &hSearch, 0, &stFile,
                       sizeof (stFile), &usSearchCount, 0L, FALSE );
  UtlFindClose( hSearch, FALSE );

  /*******************************************************************/
  /* if nothing found yet try with dictionary                        */
  /*******************************************************************/
  if ( usRc == NO_ERROR )
  {
    /*******************************************************************/
    /* Build path for remote dicts                                     */
    /*******************************************************************/
    UtlMakeEQFPath( szMemPath, cDrive, COMDICT_PATH, NULL );
    strcat( szMemPath, BACKSLASH_STR );
    strcat( szMemPath, DEFAULT_PATTERN );

    /*******************************************************************/
    /* look for files                                                  */
    /*******************************************************************/
    usSearchCount = 1;                       // find only one file
    hSearch = HDIR_CREATE;                   // Allocate a new, unused handle

    usRc = UtlFindFirst( szMemPath, &hSearch, 0, &stFile,
                         sizeof (stFile), &usSearchCount, 0L, FALSE );
    UtlFindClose( hSearch, FALSE );
  } /* endif */

  return( usRc == NO_ERROR );
} /* end of function fCheckForRemoteTms */


typedef struct _TWBCONNECTIDA
{
   USHORT  usClass;                    // Class of object being connected
   SHORT   sDlgID;                     // ID to be used for dialog
   HWND    hwndLB;                     // listbox containing connected resources
   CHAR    szBuffer[512];              // buffer for various tasks
   CHAR    szDrives[MAX_DRIVELIST];    // available drives
   CHAR    szRemProp[MAX_EQF_PATH];    // path for remote property file
   CHAR    szLocProp[MAX_EQF_PATH];    // path for local property file
   CHAR    szDataPath[MAX_EQF_PATH];   // path for data file
   CHAR    szIndexPath[MAX_EQF_PATH];  // path for index file
   CHAR    szLongName[MAX_LONGFILESPEC+10]; // buffer for long name + drive
   HWND    hwndShortNameLB;            // listbox for object short names
   std::vector<std::vector<std::string> *>*pvpvOptions;// options required for the access to shared memories
   std::vector<OtmSharedMemoryPlugin *>*pvSharedPlugins; // shared memory plugins being used
   CHAR    szObjName[MAX_LONGFILESPEC+10]; // buffer for object names
} TWBCONNECTIDA, *PTWBCONNECTIDA;



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBConnect              Activate connect dialog          |
//+----------------------------------------------------------------------------+
//|Function call:     TWBConnect( HWND hwnd, HMODULE hResourceMod );           |
//+----------------------------------------------------------------------------+
//|Description:       Activate the connect shared resources dialog             |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND    hwnd         handle of parent window             |
//|                   HMODULE hResourceMod Module containing dialog resources  |
//|                   USHORT  usClass      class of object being connected     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     use WinDlgBox to activate connect dialog                 |
//+----------------------------------------------------------------------------+
VOID TWBConnect
(
HWND     hwnd,                       // handle of parent window
HMODULE  hResourceMod,               // Module containing dialog resources
USHORT   usClass                     // class of objects being connected
)
{
  INT_PTR           sRC;

  DIALOGBOX( hwnd,
             TWBConnectDlgProc,
             hResourceMod,
             ID_CONNECT_DIC_DLG,
             MP2FROMSHORT(usClass),
             sRC );
} /* end of function TWBConnect */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBConnectDlgProc                                        |
//+----------------------------------------------------------------------------+
//|Function call:     TWBConnectDlgProc( HWND hwndDlg, USHORT msg, MPARAM mp1, |
//|                                     PARAM mp2 )                            |
//+----------------------------------------------------------------------------+
//|Description:       Dialog to allow attach or detach of secondary MAT        |
//|                   drives                                                   |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwndDlg  handle of dialog                         |
//|                   USHORT msg      type of message                          |
//|                   MPARAM mp1      first message parameter                  |
//|                   MPARAM mp2      second message parameter                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       depends on message type                                  |
//|                   normal return codes are:                                 |
//|                   TRUE = message has been processed                        |
//|                   FALSE = message has not been processed                   |
//+----------------------------------------------------------------------------+
//|Function flow:     switch msg                                               |
//|                     case WM_INITDLG:                                       |
//|                       call TWBConnectInit to process message               |
//|                     case WM_EQF_INITIALIZE:                                |
//|                       call TWBConnectInitialize to process message         |
//|                     case WM_COMMAND:                                       |
//|                       call TWBConnectCommand to process message            |
//|                     case WM_CLOSE:                                         |
//|                       call TWBConnectClose to process message              |
//|                     default:                                               |
//|                       pass message to default dialog procedure             |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK TWBConnectDlgProc
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      {
        PTWBCONNECTIDA pIda;                   // dialog IDA

        pIda = ACCESSDLGIDA( hwndDlg, PTWBCONNECTIDA );
        if ( pIda != NULL )
        {
          HANDLEQUERYID( pIda->sDlgID, mp2 );
        } /* endif */
      }
      break;


    case WM_HELP:
      {
        PTWBCONNECTIDA pIda;                   // dialog IDA
        pIda = ACCESSDLGIDA( hwndDlg, PTWBCONNECTIDA );
        if (pIda->usClass == clsMEMORY)
        {
          EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle, &hlpsubtblConnectTMDlg[0] );
        }
        else
        {
          EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle, &hlpsubtblConnectDicDlg[0] );
        } // endif
        mResult = TRUE;  // message processed
      }
      break;

    case WM_INITDLG:
      mResult = TWBConnectInit( hwndDlg, mp1, mp2 );
      break;

    case WM_EQF_INITIALIZE:
      mResult = TWBConnectInitialize( hwndDlg, mp1, mp2 );
      break;

    case WM_COMMAND:
      mResult = TWBConnectCommand( hwndDlg, WMCOMMANDID( mp1, mp2 ),
                                   WMCOMMANDCMD( mp1, mp2 ));
      break;

    case WM_CLOSE:
      mResult = TWBConnectClose( hwndDlg, mp1, mp2 );
      break;

    case DM_GETDEFID:
       // check if user pressed the ENTER key, but wants only to simulate
       // a double click
       if ( GetKeyState(VK_RETURN) & 0x8000 )
       {
          HWND hwndFocus = GetFocus();

          if ( hwndFocus == GetDlgItem( hwndDlg,  ID_CONNECT_RES_AVAILABLE_LB ) )
          {
            TWBConnectControl( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, LN_ENTER );
            mResult = TRUE;
          }
          else if ( hwndFocus == GetDlgItem( hwndDlg,  ID_CONNECT_RES_SELECTED_LB ) )
          {
            TWBConnectControl( hwndDlg, ID_CONNECT_RES_SELECTED_LB, LN_ENTER );
            mResult = TRUE;
          } /* endif */
       } /* endif */
       break;

    default:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
} /* end of function TWBConnectDlgProc */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBConnectInit          Process WM_INITDLG of drive dlg   |
//+----------------------------------------------------------------------------+
//|Function call:     TWBConnectInit( HWND hwndDlg, MPARAM mp1, MPARAM mp2 );   |
//+----------------------------------------------------------------------------+
//|Description:       Process command message of drives dialog: handle set     |
//|                   pushbutton, cancel pushbutton and drive buttons.         |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwndDlg         handle of dialog window           |
//|                   MPARAM mp1             first message parameter           |
//|                   MPARAM mp2             second message parameter          |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE (always)                                           |
//+----------------------------------------------------------------------------+
//|Function flow:     create and anchor IDA                                    |
//|                   if dialog is used by TM com code then                    |
//|                     set flag in IDA                                        |
//|                     load text strings for usage under TM com               |
//|                   endif                                                    |
//|                   get primary drive and create drive button for it         |
//|                   create buttons for secondary drives                      |
//|                   in case of errors dismiss the dialog                     |
//+----------------------------------------------------------------------------+
MRESULT TWBConnectInit
(
HWND    hwndDlg,                    // handle of dialog window
WPARAM  mp1,                        // first parameter of WM_INITDLG
LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  MRESULT     mResult = FALSE;        // result of message processing
  BOOL        fOK = TRUE;             // internal O.K. flag
  PTWBCONNECTIDA pIda;                // dialog IDA
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  mp1 = mp1;                          // avoid compiler warning
  mp2 = mp2;                          // avoid compiler warning

  /*******************************************************************/
  /* create and anchor IDA                                           */
  /*******************************************************************/
  fOK = UtlAlloc( (PVOID *) &pIda, 0L, (LONG) sizeof(TWBCONNECTIDA), ERROR_STORAGE );
  if ( fOK )
  {
    ANCHORDLGIDA( hwndDlg, pIda );
  } /* endif */

  /*******************************************************************/
  /* Create invisible listbox for list of connected resources        */
  /*******************************************************************/
  if ( fOK )
  {
    pIda->hwndLB = WinCreateWindow( hwndDlg, WC_LISTBOX, "",
                                    WS_CHILD | LBS_STANDARD,
                                    0, 0, 5, 5, hwndDlg, HWND_TOP, 1,
                                    NULL, NULL );
    pIda->hwndShortNameLB = WinCreateWindow( hwndDlg, WC_LISTBOX, "",
                                             WS_CHILD | LBS_STANDARD,
                                             0, 0, 5, 5, hwndDlg, HWND_TOP, 2,
                                             NULL, NULL );
  } /* endif */

  /*******************************************************************/
  /* load strings for TM mode of dialog                              */
  /*******************************************************************/
  if ( fOK )
  {
    pIda->usClass   = (USHORT)mp2;

    if (pIda->usClass == clsMEMORY)
    {
      /* set new window text if its the connect TM dialog */
      WinLoadString ((HAB) UtlQueryULong (QL_HAB), hResMod,
                     SID_CONNECT_TM_DLG, sizeof(pIda->szBuffer),
                     pIda->szBuffer);
      WinSetWindowText( hwndDlg, pIda->szBuffer );
      pIda->sDlgID = ID_CONNECT_TM_DLG;
      SETWINDOWID( hwndDlg, ID_CONNECT_TM_DLG );
    }
    else
    {
      pIda->sDlgID = ID_CONNECT_DIC_DLG;
    } /* endif */
  } /* endif */

  if ( pIda->usClass == clsMEMORY )
  {
    // listboxes are filled when processing the WM_EQF_INITIALIZE message
  }
  else
  {
    /*******************************************************************/
    /* get drives and scan drives for shared resources                 */
    /*******************************************************************/
    if ( fOK )
    {
      PSZ      pszDrive;                     // ptr for drive list processing
      USHORT   usCount;                      // number of files requested
      HDIR     hDirHandle;                   // DosFind routine handle
      USHORT   usRC;                         // return of Dos... functions
      FILEFINDBUF stResultBuf;               // DOS file find struct
      SHORT    sShortNameIndex;              // index of item in short name listbox

      UtlGetLANDriveList((PBYTE) pIda->szDrives ); // get list of LAN drives
      pszDrive = pIda->szDrives;        // start with first drive

      while ( *pszDrive != NULC )       // while not end of drive list ...
      {
        /***************************************************************/
        /* Setup search path and scan current drive for remote         */
        /* resources                                                   */
        /***************************************************************/
        UtlMakeEQFPath( pIda->szRemProp,
                        *pszDrive,
                        (USHORT)(( pIda->usClass == clsMEMORY ) ? MEM_PATH : DIC_PATH),
                        NULL );
        strcat( pIda->szRemProp, BACKSLASH_STR );
        strcat( pIda->szRemProp, DEFAULT_PATTERN_NAME );
        strcat( pIda->szRemProp, ( pIda->usClass == clsMEMORY ) ?
                EXT_OF_SHARED_MEMPROP :
                EXT_OF_SHARED_DICTPROP);

        usCount = 1;                              // one file per call
        hDirHandle = HDIR_CREATE;                 // use system defined dir handle
        usRC = UtlFindFirst( pIda->szRemProp, &hDirHandle, FILE_NORMAL,
                             &stResultBuf, sizeof( stResultBuf), &usCount, 0L,
                             FALSE );

        while ( (usRC == NO_ERROR) && usCount )  // while files found ...
        {
          // Get any long name from shared property file
          {
            // preset long name field with short name
            Utlstrccpy( pIda->szLongName, RESBUFNAME(stResultBuf), DOT );

            UtlMakeEQFPath( pIda->szRemProp, *pszDrive,
                            (USHORT)(( pIda->usClass == clsMEMORY ) ? MEM_PATH : DIC_PATH),
                            NULL );
            strcat( pIda->szRemProp, BACKSLASH_STR );
            strcat( pIda->szRemProp, RESBUFNAME(stResultBuf) );

            if ( pIda->usClass == clsMEMORY )
            {
              PPROP_NTM pProp = NULL;         // ptr to TM properties
              ULONG ulLen;
              if (UtlLoadFileL( pIda->szRemProp, (PVOID *)&pProp, &ulLen,
                               TRUE, FALSE ) )
              {
                if ( pProp->szLongName[0] != EOS )
                {
                  strcpy( pIda->szLongName, pProp->szLongName );
                  OEMTOANSI( pIda->szLongName );
                } /* endif */
                UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
              } /* endif */
            }
            else
            {
              PPROPDICTIONARY pProp = NULL;   // ptr to dict properties
              ULONG ulLen;
              if (UtlLoadFileL( pIda->szRemProp, (PVOID *)&pProp, &ulLen,
                               TRUE, FALSE ) )
              {
                if ( pProp->szLongName[0] != EOS )
                {
                  strcpy( pIda->szLongName, pProp->szLongName );
                  OEMTOANSI( pIda->szLongName );
                } /* endif */
                UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
              } /* endif */
            } /* endif */
          }

          /************************************************************/
          /* Cut-off extension of found file                          */
          /************************************************************/
          {
            PSZ pszExt = strchr( RESBUFNAME(stResultBuf), DOT );
            if ( pszExt != NULL )
            {
              *pszExt = NULC;
            } /* endif */
          }

          /************************************************************/
          /* Set path names for data file, index file and local       */
          /* property file                                            */
          /************************************************************/
          if (pIda->usClass == clsMEMORY)
          {
            UtlMakeEQFPath( pIda->szDataPath, *pszDrive, MEM_PATH, NULL );
            strcat( pIda->szDataPath, BACKSLASH_STR );
            strcat( pIda->szDataPath, RESBUFNAME(stResultBuf) );
            strcat( pIda->szDataPath, EXT_OF_SHARED_MEM );


            UtlMakeEQFPath( pIda->szIndexPath, *pszDrive, MEM_PATH, NULL );
            strcat( pIda->szIndexPath, BACKSLASH_STR );
            strcat( pIda->szIndexPath, RESBUFNAME(stResultBuf) );
            strcat( pIda->szIndexPath, EXT_OF_SHARED_MEMINDEX );

            UtlMakeEQFPath( pIda->szLocProp, NULC, PROPERTY_PATH, NULL );
            strcat( pIda->szLocProp, BACKSLASH_STR );
            strcat( pIda->szLocProp, RESBUFNAME(stResultBuf) );
            strcat( pIda->szLocProp, EXT_OF_MEM );
          }
          else
          {
            UtlMakeEQFPath( pIda->szDataPath, *pszDrive, DIC_PATH, NULL );
            strcat( pIda->szDataPath, BACKSLASH_STR );
            strcat( pIda->szDataPath, RESBUFNAME(stResultBuf) );
            strcat( pIda->szDataPath, EXT_OF_SHARED_DIC );

            UtlMakeEQFPath( pIda->szIndexPath, *pszDrive, DIC_PATH, NULL );
            strcat( pIda->szIndexPath, BACKSLASH_STR );
            strcat( pIda->szIndexPath, RESBUFNAME(stResultBuf) );
            strcat( pIda->szIndexPath, EXT_OF_SHARED_DICTINDEX );

            UtlMakeEQFPath( pIda->szLocProp, NULC, PROPERTY_PATH, NULL );
            strcat( pIda->szLocProp, BACKSLASH_STR );
            strcat( pIda->szLocProp, RESBUFNAME(stResultBuf) );
            strcat( pIda->szLocProp, EXT_OF_DICTPROP );
          } /* endif */


          /************************************************************/
          /* Check for existence of shared and local file and add     */
          /* current item in approbriate listbox                      */
          /************************************************************/
          if ( UtlFileExist( pIda->szDataPath ) &&
               UtlFileExist( pIda->szIndexPath ) )
          {
            SHORT sItem;               // index of inserted item

            // add item to short name listbox
            sprintf( pIda->szBuffer, "%c: %s", *pszDrive, RESBUFNAME(stResultBuf) );
            sShortNameIndex = INSERTITEMENDHWND( pIda->hwndShortNameLB,
                                                 pIda->szBuffer );

            /**********************************************************/
            /* shared components are available                        */
            /* if there is a local property file the shared resource  */
            /* is connected already                                   */
            /**********************************************************/
            sprintf( pIda->szBuffer, "%c: %s", *pszDrive, pIda->szLongName );
            if ( UtlFileExist( pIda->szLocProp ) )
            {
              /********************************************************/
              /* Resource already connected                           */
              /********************************************************/
              sItem = INSERTITEM( hwndDlg, ID_CONNECT_RES_SELECTED_LB, pIda->szBuffer );
              // set item handle to index of short name item
              SETITEMHANDLE( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sItem, sShortNameIndex );
            }
            else
            {
              /********************************************************/
              /* resource is not connected                            */
              /********************************************************/
              sItem = INSERTITEM( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, pIda->szBuffer );
              // set item handle to index of short name item
              SETITEMHANDLE( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, sItem, sShortNameIndex );
            } /* endif */
          } /* endif */

          usCount = 1;                              // one file per call
          usRC = UtlFindNext( hDirHandle, &stResultBuf, sizeof( stResultBuf),
                              &usCount, 0);
        } /* endwhile */
        // close search file handle
        if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

        pszDrive++;                     // continue with next drive

      } /* endwhile */
    } /* endif */

    /*******************************************************************/
    /* Copy items of connected/selected resources listbox              */
    /*******************************************************************/
    if ( fOK )
    {
      UtlCopyListBox( pIda->hwndLB,
                      WinWindowFromID( hwndDlg, ID_CONNECT_RES_SELECTED_LB ) );

      TWBConnectControl( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, 0 );
      TWBConnectControl( hwndDlg, ID_CONNECT_RES_SELECTED_LB, 0 );

      // adjust horizontal scrollbar
      UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB) );
      UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_CONNECT_RES_SELECTED_LB) );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    WinPostMsg( hwndDlg, WM_EQF_INITIALIZE, 0L, 0L );
    mResult = WinDefDlgProc( hwndDlg, WM_INITDLG, mp1, mp2);
  }
  else
  {
    WinDismissDlg( hwndDlg, FALSE );
  } /* endif */
  return( mResult );
} /* end of TWBConnectInit */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBConnectInitialze     Process WM_EQF_INITIALZE         |
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+
//|Description:       Process second part of dialog initialization:            |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwndDlg         handle of dialog window           |
//|                   MPARAM mp1             first message parameter           |
//|                   MPARAM mp2             second message parameter          |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE (always)                                           |
//+----------------------------------------------------------------------------+
MRESULT TWBConnectInitialize
(
HWND    hwndDlg,                    // handle of dialog window
WPARAM  mp1,                        // first parameter of WM_INITDLG
LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  MRESULT     mResult = FALSE;        // result of message processing
  PTWBCONNECTIDA pIda;                // dialog IDA

  mp1; mp2;

  pIda = ACCESSDLGIDA( hwndDlg, PTWBCONNECTIDA );

  if ( pIda->usClass == clsMEMORY )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    pIda->pvpvOptions = new (std::vector<std::vector<std::string> *> );
    pIda->pvSharedPlugins = new( std::vector<OtmSharedMemoryPlugin *> );

    // get list of plugin for shared memories
    std::vector<OtmMemoryPlugin *>vMemPlugins;
    pFactory->getMemoryPlugins( vMemPlugins, *(pIda->pvSharedPlugins) );

    // get the options for the shared memory plugins
    int i = 0;
    while( i < (int)pIda->pvSharedPlugins->size() )
    {
      std::vector<std::string> *pvOptions = new ( std::vector<std::string> );
      int iRC = pFactory->getConnectOptions( (char *)((*pIda->pvSharedPlugins)[i])->getName(), hwndDlg, pvOptions );
      if ( iRC == -1 )
      {
        // user cancelled the options dialog, so removed this plugin from the list
        pIda->pvSharedPlugins->erase( pIda->pvSharedPlugins->begin() + i );
        delete( pvOptions );
      }
      else
      {
        pIda->pvpvOptions->push_back( pvOptions );
        i++;
      } /* endif */       
    }

    // leave dialog if no (more) shared memory plugins are available
    if ( pIda->pvSharedPlugins->size() == 0 )
    {
      WinPostMsg( hwndDlg, WM_CLOSE, 0L, 0L );    
      return( FALSE );
    }

    for( int iPlugin = 0; iPlugin < (int)pIda->pvSharedPlugins->size(); iPlugin++ )
    {
      std::vector<std::string> vConnected;
      std::vector<std::string> vNotConnected;

      pFactory->listSharedMemories( (char *)(*pIda->pvSharedPlugins)[iPlugin]->getName(), (*pIda->pvpvOptions)[iPlugin], &vConnected, &vNotConnected );

      for ( std::size_t i = 0; i < vConnected.size(); i++ )
      {
        SHORT sItem = INSERTITEM( hwndDlg, ID_CONNECT_RES_SELECTED_LB, vConnected[i].c_str() );
        if ( sItem != LIT_NONE ) SETITEMHANDLE( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sItem, iPlugin );
      } /* endfor */ 

      for ( std::size_t i = 0; i < vNotConnected.size(); i++ )
      {
        SHORT sItem = INSERTITEM( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, vNotConnected[i].c_str() );
        if ( sItem != LIT_NONE ) SETITEMHANDLE( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, sItem, iPlugin );
      } /* endfor */ 
    }

    UtlCopyListBox( pIda->hwndLB, WinWindowFromID( hwndDlg, ID_CONNECT_RES_SELECTED_LB ) );

    TWBConnectControl( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, 0 );
    TWBConnectControl( hwndDlg, ID_CONNECT_RES_SELECTED_LB, 0 );

    // adjust horizontal scrollbar
    UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB) );
    UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_CONNECT_RES_SELECTED_LB) );

  } /* endif */

  // select first item of listboxes to enable correct keyboard handling
  SendDlgItemMessage( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, LB_SETSEL, TRUE, 0 );
  SendDlgItemMessage( hwndDlg, ID_CONNECT_RES_SELECTED_LB, LB_SETSEL, TRUE, 0 );


  return( mResult );

} /* end of TWBConnectInitialize */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TWBConnectCommand       Process WM_COMMAND of connect dlg|
//+----------------------------------------------------------------------------+
//|Function call:     TWBConnectCommand( HWND hwndDlg, MPARAM mp1, MPARAM mp2 )|
//+----------------------------------------------------------------------------+
//|Description:       Process command message of connect dialog: handle add,   |
//|                   remove, O.K. cancel pushbutton and LB activities.        |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwndDlg         handle of dialog window           |
//|                   MPARAM mp1             first message parameter           |
//|                   MPARAM mp2             second message parameter          |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE  (always)                                           |
//+----------------------------------------------------------------------------+
MRESULT TWBConnectCommand
(
HWND   hwndDlg,                     // handle of dialog window
SHORT sId,                          // id of button
SHORT sNotification                 // notification type
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
  PTWBCONNECTIDA pIda;                // dialog IDA
  BOOL        fOK = TRUE;             // internal O.K. flag

  sNotification;

  switch ( sId )
  {
    case ID_CONNECT_RES_HELP_PB:
      UtlInvokeHelp();
      break;

    case ID_CONNECT_RES_OK_PB:
      /*************************************************************/
      /* access dialog IDA                                         */
      /*************************************************************/
      pIda = ACCESSDLGIDA( hwndDlg, PTWBCONNECTIDA );

      /*************************************************************/
      /* Connect all resources in selected listbox which have been */
      /* added (i.e. are not connected already)                    */
      /*************************************************************/
      {
        SHORT sItem = 0;            // listbox item index
        SHORT sOrgItem;             // listbox item index
        SHORT sMaxItem = QUERYITEMCOUNT( hwndDlg, ID_CONNECT_RES_SELECTED_LB );

        while ( sItem < sMaxItem )
        {
          QUERYITEMTEXT( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sItem, pIda->szBuffer );

          /* Check item against items in original selected listbox */
          sOrgItem = SEARCHITEMHWND( pIda->hwndLB, pIda->szBuffer );
          if ( sOrgItem >= 0 )
          {
            /*******************************************************/
            /* nothing to to, resource is already connected        */
            /* remove item from original listbox                   */
            /*******************************************************/
            DELETEITEMHWND( pIda->hwndLB, sOrgItem );
          }
          else
          {
            /*******************************************************/
            /*******************************************************/
            /* connect resource: create local property file        */
            /*******************************************************/
            /*******************************************************/
            if ( pIda->usClass == clsMEMORY )
            {
              MemoryFactory *pFactory = MemoryFactory::getInstance();
              int iEntry = (int)QUERYITEMHANDLE( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sItem );
              if ( (iEntry >= 0) && (iEntry < (int)pIda->pvSharedPlugins->size()) )
              {
                int iRC = pFactory->connectToMemory( (char *)(*pIda->pvSharedPlugins)[iEntry]->getName(), hwndDlg, pIda->szBuffer, (*pIda->pvpvOptions)[iEntry] );
                fOK = (iRC == 0);
                // wlp begin
				// delete add "!((*pIda->pvSharedPlugins)[iEntry]->isLocalMemoryUsed())" temporaly because it will cause connection crash
				// because shared memory info return null
				// to get shared memory info correctly, it need further considerration.
				// wlp end

                /* Broadcast the EQFN_CREATED message                */
                if ( fOK && !((*pIda->pvSharedPlugins)[iEntry]->isLocalMemoryUsed()) )
                {
                  strcpy( pIda->szObjName, (*pIda->pvSharedPlugins)[iEntry]->getName());
                  strcat( pIda->szObjName, ":" );
                  if ( (pIda->szBuffer[1] == ':') && (pIda->szBuffer[2] == ' ') )   // is this a drive pre-fixed memory name
                  {
                    // skip drive part of the name
                    strcat( pIda->szObjName, pIda->szBuffer + 3 );
                  }
                  else
                  {
                    strcat( pIda->szObjName, pIda->szBuffer );
                  }
                  EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT( clsMEMORYDB ), MP2FROMP( pIda->szObjName ) );
                } 
              }

            }
            else
            {
              /*****************************************************/
              /* handle dictionary                                 */
              /*****************************************************/
              BOOL fOK = TRUE;                  // internal O.K. flag
              PPROPDICTIONARY pProp = NULL;             // ptr to dict properties
              ULONG ulRC;

              {
                SHORT sShortNameItem;

                // save long name of current object
                strcpy( pIda->szLongName, (pIda->usClass == clsMEMORY) ? pIda->szBuffer : pIda->szBuffer + 3 );

                // get short name of selected object
                if ( pIda->usClass != clsMEMORY )
                {
                  sShortNameItem = (SHORT)QUERYITEMHANDLE( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sItem );
                  QUERYITEMTEXTHWND( pIda->hwndShortNameLB, sShortNameItem, pIda->szBuffer );
                } /* endif */
              }

              /*******************************************************/
              /* Setup path of remote property file                  */
              /*******************************************************/
              UtlMakeEQFPath( pIda->szRemProp,
                              pIda->szBuffer[0],              // drive!!!
                              DIC_PATH, NULL );
              strcat( pIda->szRemProp, BACKSLASH_STR );
              strcat( pIda->szRemProp, pIda->szBuffer + 3 ) ; // name!!!
              strcat( pIda->szRemProp, EXT_OF_SHARED_DICTPROP );

              /*******************************************************/
              /* load property file into memory                      */
              /*******************************************************/
              fOK = UtlLoadFileL( pIda->szRemProp, (PVOID *)&pProp, &ulRC,
                                 TRUE, FALSE );

              /*****************************************************/
              /* Correct property header and drive letters for     */
              /* dictionary paths (drive letters may be assigned   */
              /* differently)                                      */
              /*****************************************************/
              if ( fOK )
              {
                pProp->szDictPath[0] = pIda->szBuffer[0];
                pProp->szIndexPath[0] = pIda->szBuffer[0];
                UtlMakeEQFPath( pProp->PropHead.szPath, NULC,
                                SYSTEM_PATH, NULL );
              } /* endif */

              /*****************************************************/
              /* Write local property file                         */
              /*****************************************************/
              if ( fOK )
              {
                UtlMakeEQFPath( pIda->szLocProp, NULC, PROPERTY_PATH, NULL );
                strcat( pIda->szLocProp, BACKSLASH_STR );
                strcat( pIda->szLocProp, pProp->PropHead.szName );

                ulRC = (ULONG)UtlWriteFileL( pIda->szLocProp, sizeof(PROPDICTIONARY),
                                     (PVOID)pProp, TRUE );
                fOK = ( ulRC == NO_ERROR );
              } /* endif */

              /*****************************************************/
              /* Broadcast the EQFN_CREATED message                */
              /*****************************************************/
              if ( fOK )
              {
                UtlMakeEQFPath( pIda->szLocProp, NULC, SYSTEM_PATH, NULL );
                strcat( pIda->szLocProp, BACKSLASH_STR );
                strcat( pIda->szLocProp, pProp->PropHead.szName );

                EqfSend2AllHandlers( WM_EQFN_CREATED,
                                     MP1FROMSHORT( clsDICTIONARY ),
                                     MP2FROMP( pIda->szLocProp ) );
              } /* endif */

              /*****************************************************/
              /* Cleanup                                           */
              /*****************************************************/
              if ( pProp ) UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
            } /* endif */
          } /* endif */

          sItem++; // continue with next item
        } /* endwhile */
      }

      /*************************************************************/
      /* Disconnect all resources which have been removed from     */
      /* the selected listbox (these are the items still           */
      /* remaining in out invisible listbox)                       */
      /*************************************************************/
      {
        SHORT sItem = 0;            // listbox item index
        SHORT sMaxItem = QUERYITEMCOUNTHWND( pIda->hwndLB );

        while ( sItem < sMaxItem )
        {
          /*********************************************************/
          /* Get item text                                         */
          /*********************************************************/
          QUERYITEMTEXTHWND( pIda->hwndLB, sItem, pIda->szBuffer );

          /*******************************************************/
          /*******************************************************/
          /* disconnect resource: delete local property file     */
          /*******************************************************/
          /*******************************************************/
          {
            SHORT sShortNameItem;

            // save long name of current object
            strcpy( pIda->szLongName, (pIda->usClass == clsMEMORY) ? pIda->szBuffer : pIda->szBuffer + 3 );

            // get short name of selected object
            if ( pIda->usClass != clsMEMORY )
            {
              sShortNameItem = (SHORT)QUERYITEMHANDLEHWND( pIda->hwndLB, sItem );
              QUERYITEMTEXTHWND( pIda->hwndShortNameLB, sShortNameItem,
                                 pIda->szBuffer );
            } /* endif */
          }

          if ( pIda->usClass == clsMEMORY )
          {
            MemoryFactory *pFactory = MemoryFactory::getInstance();
            int iEntry = (int)QUERYITEMHANDLEHWND( pIda->hwndLB, sItem );
            if ( (iEntry >= 0) && (iEntry < (int)pIda->pvSharedPlugins->size())  )
            {
              int iRC = pFactory->disconnectMemory( (char *)(*pIda->pvSharedPlugins)[iEntry]->getName(), hwndDlg, pIda->szLongName );
              fOK = (iRC == 0);

              /* Broadcast the WM_EQFN_DELETED message                */
              if ( fOK )
              {
                strcpy( pIda->szObjName, (*pIda->pvSharedPlugins)[iEntry]->getName());
                strcat( pIda->szObjName, ":" );
                if ( (pIda->szLongName[1] == ':') && (pIda->szLongName[2] == ' ') )   // is this a drive pre-fixed memory name
                {
                  // skip drive part of the name
                  strcat( pIda->szObjName, pIda->szLongName + 3 );
                }
                else
                {
                  strcat( pIda->szObjName, pIda->szLongName );
                }
                EqfSend2AllHandlers( WM_EQFN_DELETED, MP1FROMSHORT( clsMEMORYDB ), MP2FROMP( pIda->szObjName ) );
              } 
            }
          }
          else
          {
            /*****************************************************/
            /* handle dictionary                                 */
            /*****************************************************/
            BOOL fOK = TRUE;                  // internal O.K. flag


            /*******************************************************/
            /* Setup object name of dictionary                     */
            /*******************************************************/
            UtlMakeEQFPath( pIda->szLocProp, NULC, SYSTEM_PATH, NULL );
            strcat( pIda->szLocProp, BACKSLASH_STR );
            strcat( pIda->szLocProp, pIda->szBuffer + 3 ) ; // name!!!
            strcat( pIda->szLocProp, EXT_OF_DICTPROP );

            /*******************************************************/
            /* Check if dictionary is locked                       */
            /*******************************************************/
            {
              SHORT sRC;

              sRC = QUERYSYMBOL( pIda->szLocProp );
              if ( sRC != -1 )
              {
                PSZ pszErrParm = pIda->szBuffer + 3;
                UtlError( ERROR_DICT_NOT_DELETED,
                          MB_OK, 1, &pszErrParm,
                          EQF_ERROR );
                fOK = FALSE;
              } /* endif */
            }

            /*******************************************************/
            /* Setup property file name                            */
            /*******************************************************/
            UtlMakeEQFPath( pIda->szLocProp, NULC, PROPERTY_PATH, NULL );
            strcat( pIda->szLocProp, BACKSLASH_STR );
            strcat( pIda->szLocProp, pIda->szBuffer + 3 ) ; // name!!!
            strcat( pIda->szLocProp, EXT_OF_DICTPROP );

            /*****************************************************/
            /* Delete local property file                        */
            /*****************************************************/
            if ( fOK )
            {
              USHORT usRC = UtlDelete( pIda->szLocProp, 0L, TRUE );
              fOK = ( usRC == NO_ERROR );
            } /* endif */

            /*****************************************************/
            /* Broadcast the EQFN_DELETED message                */
            /*****************************************************/
            if ( fOK )
            {
              UtlMakeEQFPath( pIda->szLocProp, NULC, SYSTEM_PATH, NULL );
              strcat( pIda->szLocProp, BACKSLASH_STR );
              strcat( pIda->szLocProp, pIda->szBuffer + 3 ) ; // name!!!
              strcat( pIda->szLocProp, EXT_OF_DICTPROP );
              EqfSend2AllHandlers( WM_EQFN_DELETED,
                                   MP1FROMSHORT( clsDICTIONARY ),
                                   MP2FROMP( pIda->szLocProp ) );
            } /* endif */
          } /* endif */

          sItem++; // continue with next item
        } /* endwhile */
      }
      /*************************************************************/
      /* if ok close dialog                                        */
      /*************************************************************/
      if ( fOK )
      {
        WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT(TRUE), 0L );
      } /* endif */
      break;

    case ID_CONNECT_RES_ADD_PB:
      /*************************************************************/
      /* Add selected items from available listbox to selected     */
      /* listbox                                                   */
      /*************************************************************/
      {
        SHORT sItem;                // listbox item index

        pIda = ACCESSDLGIDA( hwndDlg, PTWBCONNECTIDA );

        sItem = QUERYNEXTSELECTION( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB,
                                    LIT_FIRST );
        while ( sItem != LIT_NONE )
        {
          /*********************************************************/
          /* Get item text                                         */
          /*********************************************************/
          QUERYITEMTEXT( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, sItem,
                         pIda->szBuffer );

          /*********************************************************/
          /* Add item to selected listbox and delete it in         */
          /* available listbox                                     */
          /*********************************************************/
          {
            LONG lHandle;
            SHORT sNewItem;
            sNewItem = INSERTITEM( hwndDlg, ID_CONNECT_RES_SELECTED_LB,
                                   pIda->szBuffer );
            lHandle = (LONG)QUERYITEMHANDLE( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, sItem );
            SETITEMHANDLE( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sNewItem, lHandle );
            DELETEITEM( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, sItem );
            // adjust horizontal scrollbar
            UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB) );
            UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_CONNECT_RES_SELECTED_LB) );
          }

          /*********************************************************/
          /* Get next item                                         */
          /*********************************************************/
          sItem = QUERYNEXTSELECTION( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB,
                                      LIT_FIRST );
        } /* endwhile */
      }
      break;

    case ID_CONNECT_RES_REMOVE_PB:
      /*************************************************************/
      /* Add selected items from selected listbox to available     */
      /* listbox                                                   */
      /*************************************************************/
      {
        SHORT sItem;                // listbox item index

        pIda = ACCESSDLGIDA( hwndDlg, PTWBCONNECTIDA );

        sItem = QUERYNEXTSELECTION( hwndDlg, ID_CONNECT_RES_SELECTED_LB,
                                    LIT_FIRST );
        while ( sItem != LIT_NONE )
        {
          /*********************************************************/
          /* Get item text                                         */
          /*********************************************************/
          QUERYITEMTEXT( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sItem,
                         pIda->szBuffer );

          /*********************************************************/
          /* Add item to available listbox and delete it in        */
          /* selected listbox                                      */
          /*********************************************************/
          {
            LONG lHandle;
            SHORT sNewItem;
            sNewItem = INSERTITEM( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB,
                                   pIda->szBuffer );
            lHandle = (LONG)QUERYITEMHANDLE( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sItem );
            SETITEMHANDLE( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, sNewItem, lHandle );
            DELETEITEM( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sItem );
            // adjust horizontal scrollbar
            UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB) );
            UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_CONNECT_RES_SELECTED_LB) );
          }

          /*********************************************************/
          /* Get next item                                         */
          /*********************************************************/
          sItem = QUERYNEXTSELECTION( hwndDlg, ID_CONNECT_RES_SELECTED_LB,
                                      LIT_FIRST );
        } /* endwhile */
      }
      break;

    case ID_CONNECT_RES_CANCEL_PB:
    case DID_CANCEL:
      WinPostMsg( hwndDlg, WM_CLOSE, 0L, 0L );
      break;

    case ID_CONNECT_RES_AVAILABLE_LB:
    case ID_CONNECT_RES_SELECTED_LB:
      mResult = TWBConnectControl( hwndDlg, sId, sNotification );
      break;
    default:
      break;
  } /* endswitch */

  return( mResult );
} /* end of TWBConnectCommand */

MRESULT TWBConnectControl
(
HWND   hwndDlg,                     // dialog handle
SHORT  sId,                         // id in action
SHORT  sNotification                // notification
)
{
  MRESULT mResult = MRFROMSHORT( TRUE ); //TRUE is the default return value
  PTWBCONNECTIDA pIda;                // dialog IDA
  SHORT sItem;                        // listbox item index

  switch ( sId )
  {
    case ID_CONNECT_RES_AVAILABLE_LB:
      if ( sNotification == LN_ENTER )
      {
        pIda = ACCESSDLGIDA( hwndDlg, PTWBCONNECTIDA );

        sItem = QUERYNEXTSELECTION( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB,
                                    LIT_FIRST );
        if ( sItem >= 0 )
        {
          /*********************************************************/
          /* Get item text                                         */
          /*********************************************************/
          QUERYITEMTEXT( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, sItem,
                         pIda->szBuffer );

          /*********************************************************/
          /* Add item to available listbox and delete it in        */
          /* selected listbox                                      */
          /*********************************************************/
          {
            LONG lHandle;
            SHORT sNewItem;
            sNewItem = INSERTITEM( hwndDlg, ID_CONNECT_RES_SELECTED_LB,
                                   pIda->szBuffer );
            lHandle = (LONG)QUERYITEMHANDLE( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, sItem );
            SETITEMHANDLE( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sNewItem, lHandle );
            DELETEITEM( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, sItem );
            // adjust horizontal scrollbar
            UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB) );
            UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_CONNECT_RES_SELECTED_LB) );
          }
        } /* endif */
      }
      else
      {
        sItem = QUERYNEXTSELECTION( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB,
                                    LIT_FIRST );
        ENABLECTRL( hwndDlg, ID_CONNECT_RES_ADD_PB, (sItem >= 0) );
      } /* endif */
      break;

    case ID_CONNECT_RES_SELECTED_LB:
      if ( sNotification == LN_ENTER )
      {
        pIda = ACCESSDLGIDA( hwndDlg, PTWBCONNECTIDA );

        sItem = QUERYNEXTSELECTION( hwndDlg, ID_CONNECT_RES_SELECTED_LB,
                                    LIT_FIRST );
        if ( sItem >= 0 )
        {
          /*********************************************************/
          /* Get item text                                         */
          /*********************************************************/
          QUERYITEMTEXT( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sItem,
                         pIda->szBuffer );

          /*********************************************************/
          /* Add item to available listbox and delete it in        */
          /* selected listbox                                      */
          /*********************************************************/
          {
            LONG lHandle;
            SHORT sNewItem;
            sNewItem = INSERTITEM( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB,
                                   pIda->szBuffer );
            lHandle = (LONG)QUERYITEMHANDLE( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sItem );
            SETITEMHANDLE( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB, sNewItem, lHandle );
            DELETEITEM( hwndDlg, ID_CONNECT_RES_SELECTED_LB, sItem );
            // adjust horizontal scrollbar
            UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_CONNECT_RES_AVAILABLE_LB) );
            UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_CONNECT_RES_SELECTED_LB) );
          }
        } /* endif */
      }
      else
      {
        sItem = QUERYNEXTSELECTION( hwndDlg, ID_CONNECT_RES_SELECTED_LB,
                                    LIT_FIRST );
        ENABLECTRL( hwndDlg, ID_CONNECT_RES_REMOVE_PB, (sItem >= 0) );
      } /* endif */
      break;
  } /* endswitch */
  return( mResult );
}


MRESULT TWBConnectClose
(
HWND   hwndDlg,                     // handle of dialog window
WPARAM mp1,                         // first message parameter
LPARAM mp2                          // second message parameter
)
{
  MRESULT mResult = FALSE;            // result of message processing
  PTWBCONNECTIDA  pIda;                // dialog IDA

  mp1;                                // supress 'unreferenced parameter' msg
  mp2;                                //               "

  /*******************************************************************/
  /* free all of the allocated storage                               */
  /*******************************************************************/
  pIda = ACCESSDLGIDA( hwndDlg, PTWBCONNECTIDA );
  if ( pIda )
  {
    if ( pIda->usClass == clsMEMORY )
    {
      for( int i = 0; i < (int)pIda->pvpvOptions->size(); i++ )
      {
        delete( (*pIda->pvpvOptions)[i] );
      }
      delete( pIda->pvpvOptions );
      delete( pIda->pvSharedPlugins );
    }

    WinDestroyWindow( pIda->hwndLB );          // destroy invisible listbox
    WinDestroyWindow( pIda->hwndShortNameLB ); // destroy invisible short name listbox
    UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );          // free IDA
  } /* endif */

  /*******************************************************************/
  /* get rid off Conect dialog                                       */
  /*******************************************************************/
  WinDismissDlg( hwndDlg, TRUE );

  return( mResult );
} /* end of function TWBConnectClose */

extern HELPSUBTABLE hlpsubtblTwbSysPropDlg[];
extern HELPSUBTABLE hlpsubtblTwbSysPropGeneral[];

// show system property dialog
VOID TWBSystemProps()
{
  INT_PTR sRc;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  DIALOGBOX( EqfQueryTwbClient(), TwbSysPropDlgProc,
             hResMod, ID_SYSPROP_DLG, NULL, sRc );

} /* end of function TWBSystemProps */

INT_PTR CALLBACK TWB_SYSPROP_ADVANCED_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
INT_PTR CALLBACK TWB_SYSPROP_MARKUP_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );


INT_PTR CALLBACK TwbSysPropDlgProc
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  ULONG    ulTabCtrl;
  BOOL     fOK;                       // internal O.K. flag
  PTWBSYSPROPIDA pIda;

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_SYSPROP_DLG, mp2 ); break;

    case WM_INITDLG:
      // allocate and anchor our dialog IDA
      fOK = UtlAlloc( (PVOID *) &pIda, 0L, (ULONG)sizeof(TWBSYSPROPIDA ), ERROR_STORAGE );
      if ( fOK )
      {
        fOK = ANCHORDLGIDA( hwndDlg, pIda );
        if ( !fOK )                           //no access to ida
        {
          UtlErrorHwnd( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR, hwndDlg);
        } /* endif */
      } /* endif */

      // get access to system properties and copy current settings
      // to the system properties copy in our IDA
      if ( fOK )
      {
        PPROPSYSTEM pPropSys;            // system properties ptr

        pPropSys = GetSystemPropPtr();
        memcpy( &(pIda->SysProps), pPropSys, sizeof(pIda->SysProps) );

        // the logo display time is not contained in system properties!
        pIda->sLogoDisplayTime = (SHORT)GetIntFromRegistry( APPL_Name, KEY_FIRSTTIME, 9876 );
        sprintf( pIda->szLogoTime, "%d", pIda->sLogoDisplayTime );
      } /* endif */

      if ( fOK )
      {
        TwbSysPropPropertySheetLoad( hwndDlg, pIda );
      } /* endif */
      mResult = DIALOGINITRETURN( mResult );
      break;


    case WM_COMMAND:
      mResult = TwbSysPropCommand( hwndDlg, mp1, mp2 );
      break;

    case WM_NOTIFY:
      mResult = TwbSysPropPropertySheetNotification( hwndDlg, mp1, mp2 );
      break;

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblTwbSysPropDlg[0] );
      mResult = TRUE;  // message processed
      break;


    case WM_EQF_CLOSE:
      //--- get rid off dialog ---
      pIda = ACCESSDLGIDA(hwndDlg, PTWBSYSPROPIDA);
      if ( pIda )
      {
        USHORT nItem = 0;
        /***********************************************************/
        /* free all allocated pages as well as the registration    */
        /* of the modeless dialog                                  */
        /***********************************************************/
        while ( pIda->hwndPages[nItem] )
        {
          UtlUnregisterModelessDlg( pIda->hwndPages[nItem] );
          DestroyWindow( pIda->hwndPages[nItem] );
          nItem++;
        } /* endwhile */
      } /* endif */
      DISMISSDLG( hwndDlg, mp1 );
      break;

    case TCM_SETCURSEL:
      {
        TC_ITEM Item;
        HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_SYSPROP_TABCTRL );

        pIda = ACCESSDLGIDA(hwndDlg, PTWBSYSPROPIDA);
        ulTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, ulTabCtrl, &Item );
        ulTabCtrl = Item.lParam;
        ShowWindow( pIda->hwndPages[ ulTabCtrl ], SW_HIDE );
        TabCtrl_SetCurSel( hwndTabCtrl, mp1 );
        ulTabCtrl = mp1;
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, ulTabCtrl, &Item );
        ulTabCtrl = Item.lParam;
        ShowWindow( pIda->hwndPages[ ulTabCtrl ], SW_SHOW );
      }
      break;

    default:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
} /* end of TWBSysPropDlgProc */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TwbSysPropertySheetLoad                                     |
//+----------------------------------------------------------------------------+
//|Function call:     TwbSysPropertySheetLoad( hwndDlg, mp2 );                    |
//+----------------------------------------------------------------------------+
//|Description:       handle changes on the tab page                           |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwndDlg   handle of the dialog                      |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     create any pages,                                        |
//|                   load the tabctrl text                                    |
//|                   load the (modeless) dialog, register it and position into|
//|                     tab area                                               |
//|                   return                                                   |
//+----------------------------------------------------------------------------+
BOOL TwbSysPropPropertySheetLoad
(
HWND hwndDlg,
PTWBSYSPROPIDA  pIda
)
{
  BOOL      fOK = TRUE;
  TC_ITEM   TabCtrlItem;
  USHORT    nItem = 0;
  HWND      hwndTabCtrl;
  HINSTANCE hInst;
  CHAR      szBuffer[80];
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  if ( fOK )
  {
    RECT rect;
    // remember adress of user area
    hInst = GETINSTANCE( hwndDlg );
    hwndTabCtrl = GetDlgItem( hwndDlg, ID_SYSPROP_TABCTRL );
    pIda->hwndTabCtrl = hwndTabCtrl;
    GetClientRect( hwndTabCtrl, &rect );
    TabCtrl_AdjustRect( hwndTabCtrl, FALSE, &rect );

    // leave some additional space at top
    rect.top += 20;
    MapWindowPoints( hwndTabCtrl, hwndDlg, (POINT *) &rect, 2 );


    TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM;

    // create the appropriate TAB control and load the associated dialog
    LOADSTRING( hab, hResMod, IDS_SYSPROP_TAB_GENERAL, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_SYSPROP_GENERAL_DLG ),
                       hwndDlg,
                       TWB_SYSPROP_GENERAL_DLGPROC,
                       (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

//    LOADSTRING( hab, hResMod, IDS_SYSPROP_TAB_GENERAL, szBuffer );
    //TabCtrlItem.pszText = "Advanced";
    //TabCtrlItem.lParam = nItem;
    //SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    //pIda->hwndPages[nItem] =
    //CreateDialogParam( hInst,
    //                   MAKEINTRESOURCE( ID_SYSPROP_ADVANCED_DLG ),
    //                   hwndDlg,
    //                   TWB_SYSPROP_ADVANCED_DLGPROC,
    //                   (LPARAM)pIda );

    //SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
    //              rect.left, rect.top,
    //              rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    //UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    //nItem++;

    // markup tab
    TabCtrlItem.pszText = "Markup related";
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_SYSPROP_MARKUP_DLG ),
                       hwndDlg,
                       TWB_SYSPROP_MARKUP_DLGPROC,
                       (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

  } /* endif */

  // hide all dialog pages but the first one
  if ( fOK )
  {
    int i = 1;
    while ( pIda->hwndPages[i] )
    {
      ShowWindow( pIda->hwndPages[i], SW_HIDE );
      i++;
    } /* endwhile */
  } /* endif */

  if ( !fOK )
  {
    POSTEQFCLOSE( hwndDlg, FALSE );
  } /* endif */

  return fOK;
}

INT_PTR CALLBACK TWB_SYSPROP_GENERAL_DLGPROC
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PTWBSYSPROPIDA  pIda;
  SHORT           sItem;

  switch ( msg )
  {
    case WM_INITDLG:
      pIda = (PTWBSYSPROPIDA)PVOIDFROMMP2( mp2 );
      ANCHORDLGIDA( hwndDlg, pIda );
      // reload system preferences
      ReloadSysProp( &(pIda->SysProps));

      SETTEXTLIMIT( hwndDlg, ID_SYSPROP_BROWSER_EF, (sizeof(pIda->SysProps.szWebBrowser)-1) );
      SETTEXT( hwndDlg, ID_SYSPROP_BROWSER_EF, pIda->SysProps.szWebBrowser );
      SETTEXT( hwndDlg, ID_SYSPROP_LOGOTIME_EF, pIda->szLogoTime );
      SETCHECK( hwndDlg, ID_SYSPROP_GENTAGGING_CHK, !pIda->SysProps.fNoGenericMarkup );
      SETCHECK( hwndDlg, ID_SYSPROP_USEIELIKELIST_CHK, pIda->SysProps.fUseIELikeListWindows );

      // handle fuzzy match levels
      {
                    SETTEXTLIMIT( hwndDlg, ID_SYSPROP_LKUP_FUZZYSMALL_EF, 3 );
        if ( !pIda->SysProps.lSmallLkupFuzzLevel ) pIda->SysProps.lSmallLkupFuzzLevel = 3300;
        SetPercentValue( hwndDlg, ID_SYSPROP_LKUP_FUZZYSMALL_EF,
                         pIda->SysProps.lSmallLkupFuzzLevel / 100L );

        SETTEXTLIMIT( hwndDlg, ID_SYSPROP_LKUP_FUZZYMEDIUM_EF, 3 );
        if ( !pIda->SysProps.lMediumLkupFuzzLevel ) pIda->SysProps.lMediumLkupFuzzLevel = 3300;
        SetPercentValue( hwndDlg, ID_SYSPROP_LKUP_FUZZYMEDIUM_EF,
                         pIda->SysProps.lMediumLkupFuzzLevel / 100L );

        SETTEXTLIMIT( hwndDlg, ID_SYSPROP_LKUP_FUZZYLARGE_EF, 3 );
        if ( !pIda->SysProps.lLargeLkupFuzzLevel ) pIda->SysProps.lLargeLkupFuzzLevel = 3300;
        SetPercentValue( hwndDlg, ID_SYSPROP_LKUP_FUZZYLARGE_EF,
                         pIda->SysProps.lLargeLkupFuzzLevel / 100L );

        SETTEXTLIMIT( hwndDlg, ID_SYSPROP_FUZZYSMALL_EF, 3 );
        if ( !pIda->SysProps.lSmallFuzzLevel ) pIda->SysProps.lSmallFuzzLevel = 3300;
        SetPercentValue( hwndDlg, ID_SYSPROP_FUZZYSMALL_EF,
                         pIda->SysProps.lSmallFuzzLevel / 100L );

        SETTEXTLIMIT( hwndDlg, ID_SYSPROP_FUZZYMEDIUM_EF, 3 );
        if ( !pIda->SysProps.lMediumFuzzLevel ) pIda->SysProps.lMediumFuzzLevel = 3300;
        SetPercentValue( hwndDlg, ID_SYSPROP_FUZZYMEDIUM_EF,
                         pIda->SysProps.lMediumFuzzLevel / 100L );

        SETTEXTLIMIT( hwndDlg, ID_SYSPROP_FUZZYLARGE_EF, 3 );
        if ( !pIda->SysProps.lLargeFuzzLevel ) pIda->SysProps.lLargeFuzzLevel = 3300;
        SetPercentValue( hwndDlg, ID_SYSPROP_FUZZYLARGE_EF,
                         pIda->SysProps.lLargeFuzzLevel / 100L );
      }

      /**************************************************************/
      /* Fill target language combobox                              */
      /**************************************************************/
      UtlFillTableLB( WinWindowFromID( hwndDlg, ID_SYSPROP_DEFTGTLANG_CB ),
                      ALL_TARGET_LANGUAGES );
      CBSEARCHSELECT( sItem, hwndDlg, ID_SYSPROP_DEFTGTLANG_CB,
                          pIda->SysProps.szSystemPrefLang );

      if ( pIda->SysProps.szSystemPrefLang[0] == EOS )
      {
        // force initial message if no language has been set yet
        PostMessage( hwndDlg, WM_EQF_PROCESSTASK, 0L, 0L );
      } /* endif */
      break;

    case  WM_EQF_PROCESSTASK:
      // show initial message when no language has been selected yet
      UtlError( WARNUNG_NO_TARGETLANG_SET, MB_OK,0, NULL, EQF_INFO );
      SETFOCUS( hwndDlg, ID_SYSPROP_DEFTGTLANG_CB );
      break;

    case  WM_VSCROLL :
      {
        USHORT usSBType = (USHORT)LOWORD(mp1);
        int iFieldID;                  // ID of field being worked with
        int iScrollbarID;              // ID of scrollbar sending notifcation
        UINT uiUpperLimit = 99;             // upper limit for this field

        // get ID of scrollbar
        {
          HWND hwndScrollbar = (HWND)mp2;
          iFieldID = ID_SYSPROP_FUZZYSMALL_EF;           // set default
          iScrollbarID = (int)GetWindowLong( hwndScrollbar, GWL_ID);
        }

        // get ID of corresponding field
        switch ( iScrollbarID )
        {
          case ID_SYSPROP_FUZZYSMALL_SB:
            iFieldID = ID_SYSPROP_FUZZYSMALL_EF;
            uiUpperLimit = 99;
            break;
          case ID_SYSPROP_FUZZYMEDIUM_SB:
            iFieldID = ID_SYSPROP_FUZZYMEDIUM_EF;
            uiUpperLimit = 99;
            break;
          case ID_SYSPROP_FUZZYLARGE_SB:
            iFieldID = ID_SYSPROP_FUZZYLARGE_EF;
            uiUpperLimit = 99;
            break;
          case ID_SYSPROP_LKUP_FUZZYSMALL_SB:
            iFieldID = ID_SYSPROP_LKUP_FUZZYSMALL_EF;
            uiUpperLimit = 50;
            break;
          case ID_SYSPROP_LKUP_FUZZYMEDIUM_SB:
            iFieldID = ID_SYSPROP_LKUP_FUZZYMEDIUM_EF;
            uiUpperLimit = 50;
            break;
          case ID_SYSPROP_LKUP_FUZZYLARGE_SB:
            iFieldID = ID_SYSPROP_LKUP_FUZZYLARGE_EF;
            uiUpperLimit = 50;
            break;
        } /* endswitch */

        pIda = ACCESSDLGIDA( hwndDlg, PTWBSYSPROPIDA );

        switch ( usSBType )
        {
          case SB_LINEUP :
            {
              UINT uiValue = GetPercentValue( hwndDlg, iFieldID, NULL );
              if ( uiValue < uiUpperLimit ) uiValue++;
              SetPercentValue( hwndDlg, iFieldID, uiValue );
            }
            break;
          case SB_LINEDOWN :
            {
              UINT uiValue = GetPercentValue( hwndDlg, iFieldID, NULL  );
              if ( uiValue > 5 ) uiValue--;
              SetPercentValue( hwndDlg, iFieldID, uiValue );
            }
            break;
        } /* switch */
      }
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case PID_PB_OK:
          {
            BOOL fOK = TRUE;

            /****************************************************************/
            /* if mp2 == 1L we have to validate the page, if it is 0L we    */
            /* have to copy the content of the dialog back into the struct. */
            /****************************************************************/


            // do checking in any case
            {
              int iID = ID_SYSPROP_FUZZYSMALL_EF;
              UINT uiMinValue = 5;
              UINT uiMaxValue = 99;

              // check handle fuzzy match levels
              do
              {
                UINT uiValue;

                // check current field
                uiValue = GetPercentValue( hwndDlg, iID, &fOK );
                if ( fOK )
                {
                  if ( (uiValue < uiMinValue) || (uiValue > uiMaxValue) )
                  {
                    fOK = FALSE;
                  } /* endif */
                } /* endif */

                // get ID of next field
                if ( fOK )
                {
                  if ( iID == ID_SYSPROP_FUZZYSMALL_EF )       iID = ID_SYSPROP_FUZZYMEDIUM_EF;
                  else if ( iID == ID_SYSPROP_FUZZYMEDIUM_EF ) iID = ID_SYSPROP_FUZZYLARGE_EF;
                  else if ( iID == ID_SYSPROP_FUZZYLARGE_EF )
                  {
                                                  iID = ID_SYSPROP_LKUP_FUZZYSMALL_EF;
                    uiMaxValue = 50;
                                    }
                              else if ( iID == ID_SYSPROP_LKUP_FUZZYSMALL_EF )  iID = ID_SYSPROP_LKUP_FUZZYMEDIUM_EF;
                                    else if ( iID == ID_SYSPROP_LKUP_FUZZYMEDIUM_EF ) iID = ID_SYSPROP_LKUP_FUZZYLARGE_EF;
                  else if ( iID == ID_SYSPROP_LKUP_FUZZYLARGE_EF )  iID = 0;
                } /* endif */

              } while ( fOK && (iID != 0) );

              // error handling
              if ( !fOK )
              {
                                        if (uiMaxValue == 50)
                                        {
                                    UtlError( ERROR_INVALID_LOOKUP_FUZZYMATCH_VALUE, MB_CANCEL, 0, NULL, EQF_ERROR );
                                  }
                                  else
                                  {
                  UtlError( ERROR_INVALID_FUZZYMATCH_VALUE, MB_CANCEL, 0, NULL, EQF_ERROR );
                                  }
                SETFOCUS( hwndDlg, iID );
                mResult = !fOK;
              } /* endif */
            }

            // update system properties fields if requested
            if ( fOK && (mp2 != 1L) )
            {
              /**************************************************************/
              /* get the active settings ....                               */
              /**************************************************************/
              pIda = ACCESSDLGIDA( hwndDlg, PTWBSYSPROPIDA );
              QUERYTEXT( hwndDlg, ID_SYSPROP_BROWSER_EF, pIda->SysProps.szWebBrowser );
              QUERYTEXT( hwndDlg, ID_SYSPROP_LOGOTIME_EF, pIda->szLogoTime );
              pIda->SysProps.fNoGenericMarkup = (EQF_BOOL)(!(BOOL)QUERYCHECK( hwndDlg, ID_SYSPROP_GENTAGGING_CHK  ));

              // check value in logo display time field
              {
                PSZ pszCurPos = pIda->szLogoTime;
                UtlStripBlanks( pIda->szLogoTime );
                if ( (*pszCurPos == '+') || (*pszCurPos == '-') )
                {
                  // skip sign of value
                  pszCurPos++;
                } /* endif */
                while ( isdigit(*pszCurPos) )
                {
                  pszCurPos++;
                } /* endwhile */
                if ( *pszCurPos != EOS )
                {
                  PSZ pszParm = pIda->szLogoTime;
                  UtlError( ERROR_INVALID_NUMERICAL_VALUE, MB_OK, 1, &pszParm, EQF_ERROR );
                  mResult = (MRESULT)TRUE; // do not leave dialog
                }
                else
                {
                  pIda->sLogoDisplayTime = (SHORT)atoi(pIda->szLogoTime);
                } /* endif */
              }

              // handle change if IE like window mode
              {
                EQF_BOOL fNewIEMode = (EQF_BOOL) QUERYCHECK( hwndDlg, ID_SYSPROP_USEIELIKELIST_CHK );
                if ( fNewIEMode != pIda->SysProps.fUseIELikeListWindows )
                {
                  // show restart-of-Tmgr-necessary message
                  UtlError( INFO_RESTART_REQUIRED, MB_OK, 0, NULL, EQF_INFO );
                } /* endif */
                pIda->SysProps.fUseIELikeListWindows = fNewIEMode;
              }

              // handle fuzzy match levels
              {
                                pIda->SysProps.lSmallLkupFuzzLevel  = GetDlgItemInt( hwndDlg, ID_SYSPROP_LKUP_FUZZYSMALL_EF,
                                                                NULL, FALSE ) * 100L;
                pIda->SysProps.lMediumLkupFuzzLevel = GetDlgItemInt( hwndDlg, ID_SYSPROP_LKUP_FUZZYMEDIUM_EF,
                                                                NULL, FALSE ) * 100L;
                pIda->SysProps.lLargeLkupFuzzLevel  = GetDlgItemInt( hwndDlg, ID_SYSPROP_LKUP_FUZZYLARGE_EF,
                                                                NULL, FALSE ) * 100L;

                pIda->SysProps.lSmallFuzzLevel  = GetDlgItemInt( hwndDlg, ID_SYSPROP_FUZZYSMALL_EF,
                                                                NULL, FALSE ) * 100L;
                pIda->SysProps.lMediumFuzzLevel = GetDlgItemInt( hwndDlg, ID_SYSPROP_FUZZYMEDIUM_EF,
                                                                NULL, FALSE ) * 100L;
                pIda->SysProps.lLargeFuzzLevel  = GetDlgItemInt( hwndDlg, ID_SYSPROP_FUZZYLARGE_EF,
                                                                NULL, FALSE ) * 100L;
              }
              /********************************************************/
              /* Get default system language                          */
              /********************************************************/
              {
                 CHAR        chLangProp[MAX_LANGUAGE_PROPERTIES];
                 CHAR        chSelLangProp[MAX_LANGUAGE_PROPERTIES];
                 QUERYTEXT( hwndDlg, ID_SYSPROP_DEFTGTLANG_CB, chLangProp );

                 // check if this is a valid language ...
                 //    if so store it and save it to the system properties
                 LanguageFactory *pLangFactory = LanguageFactory::getInstance();

                 strcpy( chSelLangProp, chLangProp );
                 if ( pLangFactory->isValidLanguage( chSelLangProp, TRUE ))
                 {
                   // before we can use this language we have to check if the language s supported
                   // by the operating system (i.e. the approbriate support has been installed)
                   fOK = UtlIsLanguageSupported( chSelLangProp, TRUE, hwndDlg );

                   if ( fOK )
                   {
                     pIda->SysProps.ulSystemPrefCP = pLangFactory->getOEMCP( chSelLangProp );
                     pIda->SysProps.ulSystemPrefCP = ADJUSTCP( pIda->SysProps.ulSystemPrefCP );
                     strcpy( pIda->SysProps.szSystemPrefLang, chSelLangProp );
                   }
        				   else
				           {
        					   PSZ pszLangProp = chLangProp;
          					 UtlErrorHwnd( ERROR_LANG_NOTSUPPORTED, MB_CANCEL, 1, &pszLangProp, EQF_ERROR, hwndDlg );
                   } /* endif */
                 }
                 else
                 {
                    // this should never happen... in such a case select English(U.S.) and 850 as default
                    strcpy(pIda->SysProps.szSystemPrefLang, "English(U.S.)");
                    pIda->SysProps.ulSystemPrefCP = 850;
                 }
              }
            } /* endif */
            if ( !fOK )  mResult = MRFROMSHORT( TRUE );
          }
          break;


        case ID_SYSPROP_BROWSE_PB:
          {
            OPENFILENAME OpenStruct;

            pIda = ACCESSDLGIDA(hwndDlg, PTWBSYSPROPIDA );

            pIda->szProgramPath[0] = EOS;

// removed as NT brings dynamic link error for following function
//        SHGetSpecialFolderPath( hwndDlg, pIda->szProgramPath,
//                                CSIDL_PROGRAMS, FALSE );

            memset( &OpenStruct, 0, sizeof(OpenStruct) );
            OpenStruct.lStructSize = sizeof(OpenStruct);
            OpenStruct.hwndOwner = hwndDlg;
            OpenStruct.lpstrFilter = "Program Files (*.COM;*.EXE)\0*.COM;*.EXE\0All Files (*.*)\0*.*\0\0";
            OpenStruct.lpstrCustomFilter = NULL;
            OpenStruct.nFilterIndex = 0;
            OpenStruct.lpstrFile = pIda->szTemp;
            OpenStruct.nMaxFile = sizeof(pIda->szTemp)-1;
            OpenStruct.lpstrFileTitle = NULL;
            OpenStruct.nMaxFileTitle = 0;
            OpenStruct.lpstrInitialDir = pIda->szProgramPath;
            OpenStruct.lpstrTitle = "Select a Web Browser";
            OpenStruct.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES |
                               OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST |
                               OFN_HIDEREADONLY;
//          OpenStruct.nFileOffset;
//          OpenStruct.nFileExtension;
//          OpenStruct.lpstrDefExt;

            if ( GetOpenFileName( &OpenStruct ) )
            {
              strcpy( pIda->szBrowser, OpenStruct.lpstrFile );
              SETTEXT( hwndDlg, ID_SYSPROP_BROWSER_EF, pIda->szBrowser );
            } /* endif */
          }
          break;
      } /* endswitch */
      break;

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblTwbSysPropGeneral[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;

    default:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
};

INT_PTR CALLBACK TWB_SYSPROP_ADVANCED_DLGPROC
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PTWBSYSPROPIDA  pIda;

  switch ( msg )
  {
    case WM_INITDLG:
      pIda = (PTWBSYSPROPIDA)PVOIDFROMMP2( mp2 );
      ANCHORDLGIDA( hwndDlg, pIda );
      if ( pIda->SysProps.sXSLTEngine == XSLT_USE_MSXML )
      {
        SETCHECK_TRUE( hwndDlg, ID_SYSPROP_MSXML_RB );
      }
      else
      {
        SETCHECK_TRUE( hwndDlg, ID_SYSPROP_XSLT4C_RB );
      } /* endif */
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case PID_PB_OK:
          {
            BOOL fOK = TRUE;

            // update system properties fields if requested
            if ( fOK && (mp2 != 1L) )
            {
              pIda = ACCESSDLGIDA( hwndDlg, PTWBSYSPROPIDA );
              if ( QUERYCHECK( hwndDlg, ID_SYSPROP_MSXML_RB ) )
              {
                pIda->SysProps.sXSLTEngine = XSLT_USE_MSXML;
              }
              else
              {
                pIda->SysProps.sXSLTEngine = XSLT_USE_XSLT4C;
              } /* endif */
              UtlSetUShort( QS_XSLTENGINE, (USHORT)pIda->SysProps.sXSLTEngine );
            } /* endif */
            if ( !fOK )  mResult = MRFROMSHORT( TRUE );
          }
          break;
      } /* endswitch */
      break;

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle, &hlpsubtblTwbSysPropGeneral[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;

    default:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
};

INT_PTR CALLBACK TWB_SYSPROP_MARKUP_DLGPROC
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PTWBSYSPROPIDA  pIda;

  switch ( msg )
  {
    case WM_INITDLG:
      pIda = (PTWBSYSPROPIDA)PVOIDFROMMP2( mp2 );
      ANCHORDLGIDA( hwndDlg, pIda );

      // set status of SGML/DITA checkbox
      SETCHECK( hwndDlg, ID_SYSPROP_SGMLDITA_CHK, !pIda->SysProps.fNoSgmlDitaProcessing );

      // set status of ENTITY processing checkbox
      SETCHECK( hwndDlg, ID_SYSPROP_ENTITY_CHK, pIda->SysProps.fEntityProcessing );

      // disable the checkboxes when no IBMMarkupTablePlugin is available
      {
        OtmPlugin *pPlugin = NULL;
        PluginManager* pPluginManager = PluginManager::getInstance();
        if ( pPluginManager  != NULL ) pPlugin = pPluginManager->getPlugin( "IBMMarkupTablePlugin" );
        if ( pPlugin == NULL )
        {
          ENABLECTRL( hwndDlg, ID_SYSPROP_SGMLDITA_CHK, FALSE );
          ENABLECTRL( hwndDlg, ID_SYSPROP_ENTITY_CHK, FALSE );
        }
      }

      {
         if ( pIda->SysProps.usMemImpMrkupAction == MEMIMP_MRKUP_ACTION_SKIP ) {
            SETCHECK( hwndDlg, ID_SYSPROP_MEMIMP_MRKUP_SKIP, TRUE );
         } else
         if ( pIda->SysProps.usMemImpMrkupAction == MEMIMP_MRKUP_ACTION_RESET ) {
            SETCHECK( hwndDlg, ID_SYSPROP_MEMIMP_MRKUP_RESET, TRUE );
         } else
            SETCHECK( hwndDlg, ID_SYSPROP_MEMIMP_MRKUP_CANCEL, TRUE );
      }
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case PID_PB_OK:
          {
            BOOL fOK = TRUE;

            // update system properties fields if requested
            if ( fOK && (mp2 != 1L) )
            {
              pIda = ACCESSDLGIDA( hwndDlg, PTWBSYSPROPIDA );

              // get value of SGML/DITA checkbox
              {
                BOOL fDitaProcessing = QUERYCHECK( hwndDlg, ID_SYSPROP_SGMLDITA_CHK );

                pIda->SysProps.fNoSgmlDitaProcessing = (EQF_BOOL)!fDitaProcessing;
                UtlSetUShort( QS_SGMLDITAPROCESSING, (USHORT)fDitaProcessing );
              }

              // get value of entity checkbox
              {
                pIda->SysProps.fEntityProcessing = QUERYCHECK( hwndDlg, ID_SYSPROP_ENTITY_CHK );
                UtlSetUShort( QS_ENTITYPROCESSING, (USHORT)pIda->SysProps.fEntityProcessing );
              }

              // get value of memory import, markup table action
              {
                USHORT usMrkupAction ;
                if ( QUERYCHECK( hwndDlg, ID_SYSPROP_MEMIMP_MRKUP_SKIP ) )
                   usMrkupAction = MEMIMP_MRKUP_ACTION_SKIP ;
                else
                if ( QUERYCHECK( hwndDlg, ID_SYSPROP_MEMIMP_MRKUP_RESET ) )
                   usMrkupAction = MEMIMP_MRKUP_ACTION_RESET ;
                else
                   usMrkupAction = MEMIMP_MRKUP_ACTION_CANCEL ;
                pIda->SysProps.usMemImpMrkupAction = usMrkupAction ;
                UtlSetUShort( QS_MEMIMPMRKUPACTION, usMrkupAction );
              }


            } /* endif */
            if ( !fOK )  mResult = MRFROMSHORT( TRUE );
          }
          break;
      } /* endswitch */
      break;

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle, &hlpsubtblTwbSysPropGeneral[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;

    default:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
};




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TwbSysPropCommand                                        |
//+----------------------------------------------------------------------------+
//|Function call:     TwbSysPropCommand( hwndDlg, mp1, mp2);                   |
//+----------------------------------------------------------------------------+
//|Description:       Handle WM_COMMAND message of property sheet dialog       |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwndDlg   handle of the dialog                      |
//|                   WPARAM  mp1    message parameter 1                       |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
MRESULT TwbSysPropCommand
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
  PTWBSYSPROPIDA pIda;                        // ptr to dialog IDA
  BOOL fOK = TRUE;
  mp2;
  switch ( WMCOMMANDID( mp1, mp2 ) )
  {
    case PID_PB_HELP:
      mResult = UtlInvokeHelp();
      break;
    case PID_PB_OK:
      {
        int nItem = 0;

        pIda = ACCESSDLGIDA(hwndDlg, PTWBSYSPROPIDA );
        // issue command to all active dialog pages
        while ( pIda->hwndPages[nItem] && fOK )
        {
          PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ nItem ],
                                               DWL_DLGPROC );

          fOK =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                         PID_PB_OK, 0L);
          nItem++;
        } /* endwhile */
      }

      // update logo display time
      if ( fOK )
      {
        WriteIntToRegistry( APPL_Name, KEY_FIRSTTIME, pIda->sLogoDisplayTime);
      } /* endif */         

      // update system properties
      if ( fOK )
      {
        HPROP hPropSys;            // system properties handle
        hPropSys = EqfQuerySystemPropHnd();
        if ( !SetPropAccess( hPropSys, PROP_ACCESS_WRITE))
        {
          UtlError( ERROR_ACCESS_SYSTEMPROPERTIES, MB_CANCEL,
                    0, (PSZ *) NULP, EQF_ERROR);
          fOK = FALSE;
        }
        else
        {
          PPROPSYSTEM pPropSys;            // system properties ptr
          EQFINFO        ErrorInfo;
          pPropSys = GetSystemPropPtr();
          memcpy( pPropSys, &(pIda->SysProps), sizeof(pIda->SysProps) );
          SaveProperties( EqfQuerySystemPropHnd(), &ErrorInfo);
          ResetPropAccess( EqfQuerySystemPropHnd(), PROP_ACCESS_WRITE );
        } /* endif */

        // update system variables
        UtlSetULong( QL_SMALLLOOKUPFUZZLEVEL,  pIda->SysProps.lSmallLkupFuzzLevel );
                    UtlSetULong( QL_MEDIUMLOOKUPFUZZLEVEL, pIda->SysProps.lMediumLkupFuzzLevel );
        UtlSetULong( QL_LARGELOOKUPFUZZLEVEL,  pIda->SysProps.lLargeLkupFuzzLevel );

        UtlSetULong( QL_SMALLFUZZLEVEL,  pIda->SysProps.lSmallFuzzLevel );
        UtlSetULong( QL_MEDIUMFUZZLEVEL, pIda->SysProps.lMediumFuzzLevel );
        UtlSetULong( QL_LARGEFUZZLEVEL,  pIda->SysProps.lLargeFuzzLevel );
      } /* endif */

      if ( fOK ) POSTEQFCLOSE( hwndDlg, TRUE );

      break;

    case PID_PB_CANCEL:
    case DID_CANCEL:
      POSTEQFCLOSE( hwndDlg, FALSE );
      break;


    default:
      mResult = WinDefDlgProc( hwndDlg, WM_COMMAND, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );
} /* end of TwbSysPropCommand */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TwbSysPropPropertySheetNotification                      |
//+----------------------------------------------------------------------------+
//|Function call:     TwbSysPropPropertySheetNotification( hwndDlg, mp1, mp2); |
//+----------------------------------------------------------------------------+
//|Description:       handle changes on the tab page                           |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwndDlg   handle of the dialog                      |
//|                   WPARAM  mp1    message parameter 1                       |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch ( pNMHdr->code )                                  |
//|                     case TCN_SELCHANGE:                                    |
//|                       activate new page                                    |
//|                     case TCN_SELCHANGING                                   |
//|                       hide the dialog                                      |
//|                   return                                                   |
//+----------------------------------------------------------------------------+
MRESULT TwbSysPropPropertySheetNotification
(
HWND hwndDlg,
WPARAM  mp1,
LPARAM  mp2
)
{
  NMHDR * pNMHdr;
  ULONG       ulTabCtrl;
  MRESULT      mResult = FALSE;
  PTWBSYSPROPIDA     pIda;
  pNMHdr = (LPNMHDR)mp2;
  mp1;
  switch ( pNMHdr->code )
  {
    case TCN_SELCHANGE:
      pIda = ACCESSDLGIDA(hwndDlg, PTWBSYSPROPIDA );
      if ( pIda )
      {
        TC_ITEM Item;
        HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_SYSPROP_TABCTRL );
        ulTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, ulTabCtrl, &Item );
        ulTabCtrl = Item.lParam;
        ShowWindow( pIda->hwndPages[ ulTabCtrl ], SW_SHOW );
      } /* endif */
      break;
    case TCN_SELCHANGING:
      pIda = ACCESSDLGIDA( hwndDlg, PTWBSYSPROPIDA );
      if ( pIda )
      {
        /**************************************************************/
        /* Issue a direct call to the appropriate dialog proc with    */
        /* WM_COMMAND, ID_TB_PROP_SET_PB and the second parameter set */
        /* to 1L to force only consistency checking                   */
        /**************************************************************/
        TC_ITEM Item;
        PFNWP pfnWp;
        HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_SYSPROP_TABCTRL );
        ulTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, ulTabCtrl, &Item );
        ulTabCtrl = Item.lParam;
        pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ ulTabCtrl ], DWL_DLGPROC );

        mResult = pfnWp( pIda->hwndPages[ulTabCtrl], WM_COMMAND,
                         PID_PB_OK, 1L);
        if ( mResult )
        {
          /************************************************************/
          /* stick on the side                                        */
          /* we have to post the request again since one of the system*/
          /* routines thinks that we still want to change the page..  */
          /************************************************************/
          WinPostMsg( hwndDlg, TCM_SETCURSEL, ulTabCtrl, 0L );
        } /* endif */
        ShowWindow( pIda->hwndPages[ ulTabCtrl ], SW_HIDE );
      } /* endif */
      break;
    case TTN_NEEDTEXT:
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        TOOLTIPTEXT *pToolTipText = (TOOLTIPTEXT *) mp2;
        if ( pToolTipText )
        {
          TC_ITEM Item;
          HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_SYSPROP_TABCTRL );

          memset( &Item, 0, sizeof(Item) );
          Item.mask = TCIF_PARAM;
          TabCtrl_GetItem( hwndTabCtrl, pToolTipText->hdr.idFrom, &Item );
          switch ( (SHORT)Item.lParam )
          {
            case 0:      // first page
              LOADSTRING( hab, hResMod, IDS_TWBSYSPROP_TTIP_GENERAL,
                          pToolTipText->szText );
              break;
          } /* endswitch */
        } /* endif */
      }
      break;
    default:
      break;
  } /* endswitch */
  return mResult;
} /* end of function TwbSysPropPropertySheetNotification */

// function to return the current DLL version
ULONG EQFGETDLLVERSION( void )
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

// set a percentage value in a dialog entryfield
void SetPercentValue
(
  HWND   hwndDlg,                      // handle of dialog window
  int    iControlID,                   // ID of entryfield
  int    iValue                        // percentage value (0..99)
)
{
  CHAR  szBuffer[10];

  sprintf( szBuffer, "%ld%%", iValue );
  SetDlgItemText( hwndDlg, iControlID, szBuffer );
}

// get the percentage value from a dialog entryfield
// only the digits of the field are retrieved
UINT GetPercentValue
(
  HWND   hwndDlg,                      // handle of dialog window
  int    iControlID,                   // ID of entryfield
  BOOL   *pfOK                         // success indicator or NULL
)
{
  CHAR szBuffer[20];
  CHAR szValue[20];
  PSZ  pszSource = szBuffer;
  PSZ  pszTarget = szValue;
  UINT uiValue = 0;

  // get text
  GetDlgItemText( hwndDlg, iControlID, szBuffer, sizeof(szBuffer) - 1 );

  // copy all digits to target string
  while ( *pszSource == BLANK ) pszSource++;
  while ( isdigit(*pszSource) ) *pszTarget++ = *pszSource++;
  *pszTarget = EOS;

  // check remaining characters if success indicator given
  if ( pfOK != NULL )
  {
    *pfOK = TRUE;
    while ( *pszSource == BLANK ) pszSource++;
    if ( (*pszSource != '%') && (*pszSource != EOS) ) *pfOK = FALSE;
    if ( *pszSource == '%' ) pszSource++;
    while ( *pszSource == BLANK ) pszSource++;
    if ( *pszSource != EOS ) *pfOK = FALSE;
  } /* endif */

  // convert string to number
  uiValue = atoi( szValue );

  return( uiValue );
}
