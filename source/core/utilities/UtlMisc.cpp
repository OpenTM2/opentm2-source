//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#include <shlobj.h>               // folder browse function
#include <time.h>                 // C time related functions

#define INCL_EQF_TM               // general Transl. Memory functions
#include "eqf.h"
#include "otmapi.h"
#include "eqfserno.h"
#include "eqfevent.h"
#include "eqfsetup.h"
#include "Utility.h"
#include "eqfutmdi.h"           // MDI Utilities
#include "eqffol.h"
#include <eqfutils.id>            // IDs used by dialog utilities
#include "eqfta.h"                // required for TAAdjustWhiteSpace prototype

// includes for Xalan XSLT

// the Win32 Xerces/Xalan build requires the default structure packing...
#pragma pack( push )
#pragma pack(8)
#include <iostream>
#ifdef XALANCODEDOESWORK
  #include <xercesc/util/PlatformUtils.hpp>
  #include <xercesc/util/PlatformUtils.hpp>
  #include <xalanc/XalanTransformer/XalanTransformer.hpp>
#endif
#pragma pack( pop )


#define NATIONAL_SETTINGS     "intl"

#define MAX_DIALOGS					100		// max. number of modeless dialogs
#define MAX_DISPATCH_MSGS 			30		// number of msgs processed per dispatch
static HWND hwndDlgs[MAX_DIALOGS];			// array for handles of modeless dialogs
static SHORT  sRegisteredDlgs = 0;			// number of registered dialogs
static FILE   *hFFSTFile = NULL;       // UtlAlloc log file
PEVENTTABLE pEventTable;        // ptr to event logging table

static FARPROC lpfnOldButtonProc = NULL;// ptr to original button proc
LONG FAR PASCAL EqfHelpButtonProc     (HWND, UINT, WPARAM, LPARAM);

static HANDLE hinstWFWDriver = 0;			// instance handle for Windows network driver

#ifndef ORD_MNETNETWORKENUM
	#define ORD_MNETNETWORKENUM		33
#endif
#ifndef ORD_MNETGETNETINFO
	#define ORD_MNETGETNETINFO		37
#endif
#ifndef MNM_NET_PRIMARY						// this resides in multinet.h from WFW SDK
	#define MNM_NET_PRIMARY			0x0001	// Network is primary network (Windows network)
#endif
#ifndef ORD_MNETSETNEXTTARGET
	#define ORD_MNETSETNEXTTARGET	38
#endif

/*  assume a maximum of five multinet drivers */
#define MAX_MNETS				5
static HANDLE hinstMNet[MAX_MNETS] = { NULL};
static int MNets = 0;           /* our count of networks installed */
static HANDLE primary = 0;              /* the primary network */

typedef WORD (WINAPI *LPMNETSETNEXTTARGET)( HANDLE hNetwork);
typedef WORD (WINAPI *LPMNETNETWORKENUM)( HANDLE FAR *hNetwork);
typedef WORD (WINAPI *LPMNETGETNETINFO)( HANDLE hNetwork, WORD FAR *lpwNetInfo,
        LPSTR lpszButton, WORD FAR *lpcbButton, HANDLE FAR *lphInstance);

HMENU UtlFindSubMenu( HMENU hMenu, const char *pszSubMenu );

/* function to get hinstWFWDriver handle for MNet calls */
HANDLE UtlGetWFWNetDriver()
{
        HANDLE hinst = 0;

#if defined(WIN32BIT)
   // TBD : Call to WNetGetCaps has to be converted to 32bit!
#else
        hinst = WNetGetCaps(0xffff);
#endif
        if (hinst != 0)
                hinstWFWDriver = hinst;

        return( hinstWFWDriver );
}

/* Our function to call MNetSetNextTarget */
WORD FAR PASCAL UtlSetNextTarget( HANDLE hNetwork)
{
  LPMNETSETNEXTTARGET lpMNetSetNextTarget;        // pointer to MNet function
  WORD retval = WN_BAD_VALUE;

  if (hinstWFWDriver == 0)
    hinstWFWDriver = UtlGetWFWNetDriver();

  if (hinstWFWDriver != 0)
  {
    lpMNetSetNextTarget = (LPMNETSETNEXTTARGET) GetProcAddress( (HMODULE) hinstWFWDriver,
    (LPSTR) MAKEINTRESOURCE(ORD_MNETSETNEXTTARGET) );
    if (lpMNetSetNextTarget != NULL)
      retval = (*lpMNetSetNextTarget) (hNetwork);
  }

  return (retval);
}

/*!
	Function Name:     UtlPlugIn              Plug a new entry into a list

	Description:       Links entries to a list

	Parameters:        1. PPLUG - ptr to new plug to link in list
					2. PPLUG - ptr to backward plug
					3. PPLUG - ptr to foreward plug

	Returncode type:   PPLUG

	Returncodes:       ptr to new plug

	Function flow:     set forward and backward pointers in new plug
					set forward pointer of backward plug to new plug
					set backward pointer of forawrd plug to new plug
*/
PPLUG UtlPlugIn( PPLUG pNew, PPLUG pBackward, PPLUG pForeward )
{
    pNew->Bw = pBackward;
    pNew->Fw = pForeward;
    if( pBackward )
      pBackward->Fw = pNew;
    if ( pForeward )
      pForeward->Bw = pNew;
    return( pNew );
}

/*!
	Function Name:     UtlPlugOut             Remove an entry from a list

	Description:       Unlinks an entry from a list

	Parameters:        1. PPLUG - ptr to plug to get unlinked

	Returncode type:   PPLUG

	Returncodes:       ptr to unlinked plug

	Function flow:     set backward pointer in forward plug to backward pointer
					set forward pointer in backward plug to forward pointer
*/
PPLUG UtlPlugOut( PPLUG pPlug)
{
    if( pPlug->Bw)
      pPlug->Bw->Fw = pPlug->Fw;
    if( pPlug->Fw)
      pPlug->Fw->Bw = pPlug->Bw;
    return( pPlug);
}

/*!
	Function Name:     UtlSplitFnameFromPath  Split a path at filename

	Description:       Locate and return a ptr to filename in full path

	Parameters:        1. PSZ - path

	Returncode type:   PSZ

	Returncodes:       - ptr to filename or NULP if not found

	Note on usage:     Source string is !!! modified
					The separating \ between dir path and filename gets
					replaced by a \0 to make the dir path a usable C string

	Samples:           ps = "D:\EQF\TEST.F00");
					pf = UtlSplitFnameFromPath( ps);
					will set pf to "TEST.F00" and ps to "D:\EQF"
	----------------------------------------------------------------------------+
	Function flow:     Replace last backslash with EOS and return pointer to
						next character
*/
PSZ UtlSplitFnameFromPath( PSZ path)
{
    char *p;
    if( (p = strrchr( path, BACKSLASH)) != NULL )
       *p++ = EOS;
    return( p);
}

/*!
	Function Name:     UtlGetFnameFromPath    Get the position of the file name

	Description:       Locate and return a ptr to filename in full path

	Parameters:        1. PSZ - path

	Returncode type:   PSZ

	Returncodes:       - ptr to filename or NULP if not found

	Note on usage:     Source string is not modified

	Samples:           pf = UtlGetFnameFromPath( "D:\EQF\TEST.F00");
					will set pf to "TEST.F00"

	Function flow:     Return pointer to position of last backslash plus one
*/
PSZ UtlGetFnameFromPath( PSZ path)
{
  PSZ   pszFileName = NULL;                   // ptr to begin of filename

  if( (pszFileName = strrchr( path, BACKSLASH)) != NULL )
    pszFileName++;
  else
    pszFileName = path;
  return( pszFileName );
}
/*!
	Function Name:     UtlGetDriveType   Return type of a given drive

	Description:       Under Windows: return type of given drive
					Under OS/2: return diskette for A: and B: else return
					FDISK

	Function Call:     SHORT UtlGetDriveType( CHAR chDrive );

	Parameters:        CHAR chDrive  letter of drive

	Returncode type:   SHORT

	Returncodes:       -1    drive invalid
					DRIVE_REMOVEABLE     for diskettes
					DRIVE_FIXED          for fixed disks
					DRIVE_REMOTE         for network drives
					DRIVE_CDROM          for CD ROM drives
					DRIVE_RAM            for RAM disks
*/
SHORT UtlDriveType( CHAR chDrive )
{
  SHORT   sDriveID;                    // numeric ID (index) of drive
  USHORT  usId = UtlGetTask();        // index into global variables for this task

  toupper( chDrive );

  /********************************************************************/
  /* Check for valid drive chars                                      */
  /********************************************************************/
  if ( (chDrive < 'A') || (chDrive > 'Z') )
  {
    return( -1 );
  } /* endif */

  /********************************************************************/
  /* Get index of drive                                               */
  /********************************************************************/
  sDriveID = chDrive - 'A';

  /********************************************************************/
  /* Return drive type if already set in drive type array             */
  /********************************************************************/
  if ( UtiVar[usId].sDriveType[sDriveID] != -1 )
  {
    return( UtiVar[usId].sDriveType[sDriveID] );
  } /* endif */

  /********************************************************************/
  /* Get type of drive                                                */
  /********************************************************************/
  UtiVar[usId].sDriveType[sDriveID] = (SHORT)EqfDriveType( sDriveID );

  return( UtiVar[usId].sDriveType[sDriveID] );

} /* end of function UtlDriveType */

//+----------------------------------------------------------------------------+
//External function
//+----------------------------------------------------------------------------+
//Function Name:     UtlCompareDate
//+----------------------------------------------------------------------------+
//Description:
//
//+----------------------------------------------------------------------------+
//Function Call:     SHORT UtlCompareDate *pfDateFirst,
//                                        *pfDateSecond )
//+----------------------------------------------------------------------------+
//Parameters:        - *pfDateFirst  first date to be compared
//                   - *pfDateSecond second date to be compared
//+----------------------------------------------------------------------------+
//Return Values:     LONG returns difference between *pfDateFirst and
//                        *pfDateSecond  ( *pfDateFirst - *pfDateSecond)
//+----------------------------------------------------------------------------+
//Returncode type:   LONG
//+----------------------------------------------------------------------------+
//Returncodes:       return > 0  First newer Second
//                   return = 0  First equal Second
//                   return < 0  First older Second
//+----------------------------------------------------------------------------+
//Function flow:     generate LONG from *pfDateFirst
//                   generate LONG from *pfDateSecond
//                   get difference
//+----------------------------------------------------------------------------+
LONG UtlCompareDate( FDATE *pfDateFirst, FDATE *pfDateSecond )
{
   LONG   sCompare;                             // return code
   LONG   uDayFirst;
   LONG   uDaySecond;

   uDayFirst = (pfDateFirst->year * 400) +
               (pfDateFirst->month * 31) +
               pfDateFirst->day;


   uDaySecond = (pfDateSecond->year * 400) +
                (pfDateSecond->month * 31) +
                pfDateSecond->day;

   sCompare = uDayFirst - uDaySecond;

   return( sCompare );
} /* endof UtlCompareDate */

//+----------------------------------------------------------------------------+
//External function
//+----------------------------------------------------------------------------+
//Function Name:     UtlCompareTime
//+----------------------------------------------------------------------------+
//Description:
//
//+----------------------------------------------------------------------------+
//Function Call:     SHORT UtlCompareTime *pfTimeFirst,
//                                        *pfTimeSecond )
//+----------------------------------------------------------------------------+
//Parameters:        - *pfTimeFirst  first time to be compared
//                   - *pfTimeSecond second time to be compared
//+----------------------------------------------------------------------------+
//Return Values:     LONG returns difference between *pfTimeFirst and
//                        *pfTimeSecond  ( *pfTimeFirst - *pfTimeSecond)
//+----------------------------------------------------------------------------+
//Returncode type:   LONG
//+----------------------------------------------------------------------------+
//Returncodes:       return > 0  First newer Second
//                   return = 0  First equal Second
//                   return < 0  First older Second
//+----------------------------------------------------------------------------+
//Function flow:     generate LONG from *pfTimeFirst
//                   generate LONG from *pfTimeSecond
//                   get difference
//+----------------------------------------------------------------------------+
LONG UtlCompareTime( FTIME *pfTimeFirst, FTIME *pfTimeSecond )
{
   LONG   sCompare;                             // return code
   LONG   uTimeFirst;
   LONG   uTimeSecond;

   uTimeFirst = (pfTimeFirst->hours * 3600) +
                (pfTimeFirst->minutes * 60) +
                pfTimeFirst->twosecs;

   uTimeSecond = (pfTimeSecond->hours * 3600) +
                 (pfTimeSecond->minutes * 60) +
                 pfTimeSecond->twosecs;
   sCompare =  uTimeFirst - uTimeSecond ;
   return( sCompare );
} /* endof UtlCompareTime */


BOOL UtlMNetDetect()
{
  BOOL retval = FALSE;
#if defined(WIN32BIT)
   // TBD : Call to WNetGetCaps has to be converted to 32bit!
#else
  WORD nIndex;

  static int Multinet = -1;

  if (Multinet == -1)                       /* first time, so we must determine */
  {
    nIndex = WNetGetCaps(WNNC_NET_TYPE);    /* query net version type */
    if (nIndex & WNNC_NET_MultiNet)         // is MultiNet bit set ??
       Multinet = 1;
    else
       Multinet = 0;
  } /* endif */

  if (Multinet == 1)
     retval = TRUE;
#endif
  return (retval);
}

BOOL FAR PASCAL UtlNetworkEnumAll()
{
  LPMNETNETWORKENUM lpMNetNetworkEnum;    // pointer to MNet function
  WORD err;
  HANDLE hNetwork = 0;
  char szButton[80] = "empty" ;
  WORD cbButton = 80;
  LPMNETGETNETINFO  lpMNetGetNetInfo;     // pointer to MNet function
  WORD NetInfo = 0;
  HANDLE hInstance = NULL;
  WORD errinfo;
  static BOOL fDone = FALSE;
  BOOL retval = FALSE;
  int i;

  if ( fDone )
  {
    return( TRUE );
  } /* endif */

  for ( i = 0; i < MAX_MNETS; i++)  /* initialize our MNet array */
          hinstMNet[i++] = NULL;
  i = 0;

  if (hinstWFWDriver == 0)
          hinstWFWDriver = UtlGetWFWNetDriver();

  if (hinstWFWDriver != 0 )
  {
    lpMNetNetworkEnum = (LPMNETNETWORKENUM) GetProcAddress( (HMODULE) hinstWFWDriver,
            (LPSTR) MAKEINTRESOURCE(ORD_MNETNETWORKENUM) );
    if (lpMNetNetworkEnum != NULL)
    {
      err = (*lpMNetNetworkEnum) ( &hNetwork);
      if (hNetwork != 0)
      {
        hinstMNet[i++] = hNetwork;
        lpMNetGetNetInfo = (LPMNETGETNETINFO) GetProcAddress( (HMODULE) hinstWFWDriver,
                            (LPSTR) MAKEINTRESOURCE(ORD_MNETGETNETINFO) );
        if (lpMNetGetNetInfo != NULL)
        {
          errinfo = (*lpMNetGetNetInfo) ( hNetwork, &NetInfo, szButton, &cbButton, &hInstance);
          if (errinfo == WN_SUCCESS)
          {
            if (NetInfo == MNM_NET_PRIMARY)
              primary = hNetwork;
          } /* endif */
        }
        while (err != WN_BAD_VALUE && i < MAX_MNETS )
        {
          err = (*lpMNetNetworkEnum) ( &hNetwork);
          if (hNetwork != 0)
          {
            hinstMNet[i++] = hNetwork;

            if (lpMNetGetNetInfo != NULL)
            {
              errinfo = (*lpMNetGetNetInfo) ( hNetwork, &NetInfo, szButton, &cbButton, &hInstance);
              if (errinfo == WN_SUCCESS)
              {
                if (NetInfo == MNM_NET_PRIMARY)
                  primary = hNetwork;
              }
            }
          }
        } /* while err != WN_BAD_VALUE etc */
      }
    }

    for ( i = 0; i < MAX_MNETS; i++) /* did we find some networks ? */
    {
      if (hinstMNet[i] != 0)
      {
        MNets++;
        retval = TRUE;
        fDone = TRUE;
      }
    }
  }

  return (retval);
}


USHORT UtlGetLANUserID
(
  PSZ    pszLANUserID,                 // LAN UserID - returned
  USHORT *pusUserPriv,                 // User priviliges
  USHORT usMsgHandling                 // Message handling parameter
)
{
  USHORT usRC = LANUID_NO_LAN;         // function return code
  static BOOL notfirst = FALSE;   /* the first time Windows for Workgroups gives a bogus value */

  usMsgHandling;
  // Preset the user priviliges and the LAN userid
  *pusUserPriv = USER_USER;
  if ( pszLANUserID != NULL )
  {
    strcpy( pszLANUserID, "" );
  } /* endif */

  // Handle multi net driver
  if ( UtlMNetDetect() )          /* deal with windows for workgroups multinet */
  {
    if ( UtlNetworkEnumAll() )
    {
      if (primary != 0 )
        UtlSetNextTarget(primary);
      else
        UtlSetNextTarget(hinstMNet[0]);
    }
  } /* end of Multinet detect code */

  /********************************************************************/
  /* Get LAN user ID from NETWORK entry of SYSTEM.INI                 */
  /********************************************************************/
  if ( pszLANUserID != NULL )
  {
    DWORD iUserIDLen = MAX_USERID;
    int iRC;
    static char szUser[80];

    iUserIDLen = sizeof(szUser) - 1;
    if ( GetUserName( szUser, &iUserIDLen ) )
    {
      strncpy( pszLANUserID, szUser, MAX_USERID - 1 );
      pszLANUserID[MAX_USERID-1] = EOS;
      iRC = WN_SUCCESS;
    }
    else
    {
      iRC = GetLastError();
    } /* endif */

    if ( iRC != WN_SUCCESS )
    {
      pszLANUserID[0] = NULC;
    }
    else
    {
      usRC = NO_ERROR;
    } /* endif */
  } /* endif */
  return( usRC );
}

USHORT UtlGetLANUserIDW
(
  PSZ_W  pszLANUserIDW,                 // LAN UserID - returned
  USHORT *pusUserPriv,                 // User priviliges
  USHORT usMsgHandling                 // Message handling parameter
)
{ USHORT     usRc = NO_ERROR;
  CHAR  szUserID[MAX_USERID];

   usRc = UtlGetLANUserID( szUserID, pusUserPriv, usMsgHandling );
   ASCII2UnicodeBuf( szUserID, pszLANUserIDW, MAX_USERID, 0L);
   return (usRc);
}

// logging functions

#ifdef _DEBUG

// write time stamp to log file
void UtlWriteTimeStamp()
{
  USHORT usTask = UtlGetTask();

  if ( UtiVar[usTask].hfUtilsLog != NULL )
  {
    FILETIME  ftSysTime;
    LARGE_INTEGER liTime;

    GetSystemTimeAsFileTime( &ftSysTime );
    memcpy( &liTime, &ftSysTime, sizeof(liTime ) );

    fprintf( UtiVar[usTask].hfUtilsLog, "%I64d ", liTime );
  } /* endif */
}

void UtlLogStart( PSZ pszLogFilePrefix )
{
  USHORT usTask = UtlGetTask();
  char szLogFile[MAX_EQF_PATH];
  FILETIME  ftSysTime;
  LARGE_INTEGER liTime;

  GetSystemTimeAsFileTime( &ftSysTime );
  memcpy( &liTime, &ftSysTime, sizeof(liTime ) );

  UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
  strcat( szLogFile, "\\" );
  sprintf( szLogFile + strlen(szLogFile), "%s-%I64d.log", pszLogFilePrefix, liTime );
  UtiVar[usTask].hfUtilsLog = fopen( szLogFile, "a" );
  UtlLogWrite( "Started logging" );
}

void UtlLogStop()
{
  USHORT usTask = UtlGetTask();

  if ( UtiVar[usTask].hfUtilsLog != NULL )
  {
    UtlLogWrite( "Stopped logging" );
    fclose( UtiVar[usTask].hfUtilsLog );
    UtiVar[usTask].hfUtilsLog = NULL;
  } /* endif */
}

void UtlLogFlush()
{
  USHORT usTask = UtlGetTask();

  if ( UtiVar[usTask].hfUtilsLog != NULL )
  {
    fflush( UtiVar[usTask].hfUtilsLog );
  } /* endif */
}

void UtlLogWriteString( PSZ pszFormat, PSZ pszString )
{
  USHORT usTask = UtlGetTask();

  if ( UtiVar[usTask].hfUtilsLog != NULL )
  {
    UtlWriteTimeStamp();
    fprintf( UtiVar[usTask].hfUtilsLog, pszFormat, pszString );
    fwrite( "\n", 1, 1, UtiVar[usTask].hfUtilsLog );
#ifdef FLUSHLOG
    UtlLogFlush();
#endif
  } /* endif */
}

void UtlLogWriteString2( PSZ pszFormat, PSZ pszString1, PSZ pszString2 )
{
  USHORT usTask = UtlGetTask();

  if ( UtiVar[usTask].hfUtilsLog != NULL )
  {
    UtlWriteTimeStamp();
    fprintf( UtiVar[usTask].hfUtilsLog, pszFormat, pszString1, pszString2 );
    fwrite( "\n", 1, 1, UtiVar[usTask].hfUtilsLog );
#ifdef FLUSHLOG
    UtlLogFlush();
#endif
  } /* endif */
}

void UtlLogWrite( PSZ pszString )
{
  USHORT usTask = UtlGetTask();

  if ( UtiVar[usTask].hfUtilsLog != NULL )
  {
    UtlWriteTimeStamp();
    fprintf( UtiVar[usTask].hfUtilsLog, "%s\n", pszString );
#ifdef FLUSHLOG
    UtlLogFlush();
#endif
  } /* endif */
}

#else

void UtlLogStart( PSZ pszLogFilePrefix ) { return; }
void UtlLogStop() { return; }
void UtlLogFlush() { return; }
void UtlLogWriteString( PSZ pszFormat, PSZ pszString ) { return; }
void UtlLogWriteString2( PSZ pszFormat, PSZ pszString1, PSZ pszString2 ) { return; }
void UtlLogWrite( PSZ pszString ) { return; }

#endif

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function Name:     UtlDispatch            Dispatch a series of messages
//+----------------------------------------------------------------------------+
// Description:       Peek/Dispatch messages until message queue is empty or
//                    WM_QUIT is encountered or MAX_DISPATCH_MSGS messages
//                    have been dispatched.
//+----------------------------------------------------------------------------+
// Function Call:     UtlDispatch()
//+----------------------------------------------------------------------------+
// Parameters:        none
//+----------------------------------------------------------------------------+
// Returncode type:   VOID
//+----------------------------------------------------------------------------+
// Function flow:     while messages to process and
//                          message count has not been exceeded
//                      if message is WM_QUIT then
//                        return
//                      endif
//                      get message
//                      dispatch message
//                      decrement dispatch message count
//                    endwhile
//+----------------------------------------------------------------------------+
VOID UtlDispatch( VOID )
{
   USHORT usMsg = MAX_DISPATCH_MSGS; // message to be processed
   HWND hwndClient =  (HWND) UtlQueryULong( QL_TWBCLIENT );
   MSG msg;                      // message queue structure

   while ( usMsg && PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
   {
     if ( msg.message == WM_QUIT)
     {
        return;                       // do not dispatch WM_QUIT, leave it to
                                      // the main message loop in EQFSTART
     } /* endif */

     if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
     {
       if ( UtlIsDialogMsg( &msg ) )
       {
         /***************************************************************/
         /* nothing to do message has been dispatched within            */
         /* UtlIsDialogMsg function                                     */
         /***************************************************************/
       }
       else
       {
         if (!hwndClient || !TranslateMDISysAccel(hwndClient, &msg) )
         {
           TranslateMessage( &msg );
           DispatchMessage( &msg );
         } /* endif */
       } /* endif */
     } /* endif */
     usMsg--;
   } /* endwhile */
}

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function Name:     UtlIsDialogMsg    Check if msg is for a modeless dialog
//+----------------------------------------------------------------------------+
// Description:       Calls Unregisters a modeless dialog (Windows)
//+----------------------------------------------------------------------------+
// Function Call:     BOOL UtlIsDialogMsg( MSG FAR *pMsg );
//+----------------------------------------------------------------------------+
// Parameters:        pMsg points to the message data being checked
//+----------------------------------------------------------------------------+
// Returncode type:   BOOL
//+----------------------------------------------------------------------------+
// Returncodes:       TRUE   message has been dispatched to dialog
//                    FALSE  message is not for a modeless dialog
//+----------------------------------------------------------------------------+
BOOL UtlIsDialogMsg( MSG FAR * pMsg )
{
  BOOL             fDispatched = FALSE;// function return value
  SHORT            sIndex;             // loop index

  /********************************************************************/
  /* Loop over all registered modeless dialogs                        */
  /********************************************************************/
  sIndex = 0;
  while ( !fDispatched && (sIndex < sRegisteredDlgs) )
  {
    fDispatched = IsDialogMessage( hwndDlgs[sIndex], pMsg );
    sIndex++;
  } /* endwhile */

  return( fDispatched );
} /* end of function UtlIsDialogMsg */

VOID UtlMenuEnableItem( SHORT sItemID )
{
  SETAABITEM( GetMenu((HWND)UtlQueryULong( QL_TWBFRAME )), sItemID, TRUE );
}

VOID UtlMenuDisableItem( SHORT sItemID )
{
  SETAABITEM( GetMenu((HWND)UtlQueryULong( QL_TWBFRAME )), sItemID, FALSE );
}

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function Name:     UtlMenuDisableAll      Disable all items of a menu
//+----------------------------------------------------------------------------+
// Description:       Disables all menu items for a given menu
//+----------------------------------------------------------------------------+
// Function Call:     UtlMenuDisableAll( hwnd, sMenuID );
//+----------------------------------------------------------------------------+
// Parameters:        'sMenuID' is the ID of the menu being set
//                    (for WM_INITMENU the menu ID is SHORT1FROMMP(mp1) )
//                    hwnd   handle of window requesting this action
//+----------------------------------------------------------------------------+
// Returncode type:   VOID
//+----------------------------------------------------------------------------+
// Samples:           case WM_INITMENU:
//                       UtlMenuDisableAll( hwnd, SHORT1FROMMP(mp1) );
//                       UtlMenuEnableItem( PID_FILE_MI_EXPORT );
//                       break;
//+----------------------------------------------------------------------------+
// Function flow:     if menu ID is not the TWB actionbar then
//                      get handle of TWB actionbar
//                      get handle of submenu with the specified ID
//                      for all items of this menu
//                        set the MIA_DISABLED attribute
//                      endfor
//                    endif
//+----------------------------------------------------------------------------+
VOID UtlMenuDisableAll( HWND hwnd, SHORT sMenuID )
{
  HWND       hwndMenu;
  LONG       lItems;
  USHORT     usI;
  ULONG      ulID;                      // id of menu items


  if ( (USHORT)sMenuID == FID_MENU )
  {
    /******************************************************************/
    /* Ignore request on workbecnh actionbar                          */
    /******************************************************************/
  }
  else
  {
      /****************************************************************/
      /* get submenu handle and disable all submenu items ...         */
      /****************************************************************/
      hwndMenu = (HWND)GetMenu((HWND)UtlQueryULong( QL_TWBFRAME ));
      hwndMenu = (HWND)GetSubMenu((HMENU)hwndMenu, sMenuID);
      /****************************************************************/
      /* check if we by chance are dealing with the system icon ...   */
      /****************************************************************/
      if ((sMenuID == 0) && IsZoomed( hwnd ) )
      {
        /**************************************************************/
        /* do nothing...                                              */
        /**************************************************************/
      }
      else
      {
        if ( hwndMenu )
        {
          lItems = GetMenuItemCount((HMENU)hwndMenu);
          for ( usI = 0; usI < lItems; usI++ )
          {
            ulID = GetMenuItemID( (HMENU)hwndMenu, usI );

            // ignore menu items created by tool plugins
            if ( (ulID < ID_TWBM_AAB_TOOLPLUGINS) || (ulID >= ID_TWBM_AAB_LAST) )
            {
              SETAABITEM( (HMENU)hwndMenu, ulID, FALSE );
            }
          } /* endfor */
        } /* endif */
      } /* endif */
  } /* endif */
}

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function Name:     UtlTime           Return current time as ULONG value
//+----------------------------------------------------------------------------+
// Description:       Gets the current time using the C library function time
//                    and does any necessar corrections.
//+----------------------------------------------------------------------------+
// Function Call:     ULONG  UtlTime( PULONG pulTime );
//+----------------------------------------------------------------------------+
// Parameters:        pulTime points to buffer for the time value and may be
//                    NULL
//+----------------------------------------------------------------------------+
// Returncode type:   ULONG
//+----------------------------------------------------------------------------+
// Returncodes:       current time as ULONG
//+----------------------------------------------------------------------------+
LONG UtlTime( PLONG plTime )
{
  LONG            lTime;             // buffer for current time

  time( (time_t*) &lTime );
  lTime -= 10800L;                    // correction: - 3 hours
  if ( plTime != NULL )
  {
    *plTime = lTime;
  } /* endif */
  return( lTime );
} /* end of function UtlTime */


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function Name:     UtlCompFDates     Compares the update times of two files
//+----------------------------------------------------------------------------+
// Description:       Compares the file update times of two files
//                    and stores the result of the compare in the supplied
//                    variable
//+----------------------------------------------------------------------------+
// Function Call:     USHORT UtlComFDates( PSZ pszFile1, PSZ pszFile2,
//                                         PSHORT psResult, BOOL fMsg );
//+----------------------------------------------------------------------------+
// Parameters:        PSZ pszFile1   fully qualified path of first file
//                    PSZ pszFile2   fully qualified path of second file
//                    PSHORT psResult address of result buffer
//+----------------------------------------------------------------------------+
// Returncode type:   USHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0     function completed successfully
//                    other DOS error return codes
//+----------------------------------------------------------------------------+
USHORT UtlCompFDates
(
  PSZ    pszFile1,                     // fully qualified path first file
  PSZ    pszFile2,                     // fully qualified path of second file
  PSHORT psResult,                     // address of result buffer
  BOOL   fMsg                          // error handling flag
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  FILESTATUS  stStatus1;               // File status information of 1st file
  FILESTATUS  stStatus2;               // File status information of 2nd file
  LONG        lResult = 0L;            // buffer for result value

  /********************************************************************/
  /* Get date and time of files                                       */
  /********************************************************************/
  usRC = UtlQPathInfo( pszFile1, 1, (PBYTE)&stStatus1, sizeof(stStatus1),
                       0L, fMsg );
  if ( usRC == NO_ERROR )
  {
    usRC = UtlQPathInfo( pszFile2, 1, (PBYTE)&stStatus2, sizeof(stStatus2),
                         0L, fMsg );
  } /* endif */

  /********************************************************************/
  /* Compare file dates                                               */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    lResult = UtlCompareDate( &(stStatus1.fdateLastWrite),
                              &(stStatus2.fdateLastWrite) );
    if ( lResult == 0L )
    {
      lResult = UtlCompareTime( &(stStatus1.ftimeLastWrite),
                                &(stStatus2.ftimeLastWrite) );
    } /* endif */
    *psResult = (SHORT)lResult;
  } /* endif */

  return usRC;
} /* end of function UtlCompFDates */

VOID UtlSetHorzScrollingForLB(HWND hLB) {
  int iNumEntries, i;
  LONG lMaxWidth = 0;
  CHAR pszStrBuf[MAX_LONGFILESPEC];
  SIZE size;
  HDC hdc;

  iNumEntries = SendMessage(hLB, LB_GETCOUNT, 0, 0);
  hdc = GetDC(hLB);
  for (i = 0; i < iNumEntries; i++) {
    SendMessage(hLB, LB_GETTEXT, (WPARAM) i, (LPARAM) (LPCTSTR) pszStrBuf);
    GetTextExtentPoint32(hdc, pszStrBuf, strlen(pszStrBuf), &size);
    if (size.cx > lMaxWidth) lMaxWidth = size.cx;
  }
  ReleaseDC(hLB, hdc);
//  SendMessage(hLB, LB_SETHORIZONTALEXTENT, (lMaxWidth + GetSystemMetrics(SM_CXVSCROLL)) * 4 /
// (GetDialogBaseUnits() & 0xffff), 0);
  SendMessage(hLB, LB_SETHORIZONTALEXTENT, lMaxWidth + GetSystemMetrics(SM_CXVSCROLL), 0);
}

/*!
// Function Name:     UtlCopyListBox         Copy list box items to a list box
// Description:       Copies the items of one listbox to another listbox.
// Function Call:     SHORT UtlCopyListBox( HWND hwndTarget, HWND hwndSource )
// Parameters:        - hwndTarget is the handle of the target listbox
//                    - hwndSource is the handle of the source listbox
// Returncode type:   SHORT
// Returncodes:       number of items copied to target listbox
// Function flow:     get number of items in source listbox
//                    for all items in source listbox do
//                      get item from source listbox
//                      insert item in target listbox
//                    endfor
//                    return number of items copied
*/
SHORT UtlCopyListBox( HWND hwndTarget, HWND hwndSource)
{
   SHORT  sNum = 0;                         //number of LB items
   SHORT  sIndex;                           //index of LB item
   PSZ    pszString = NULL;                 // buffer for item text
   BOOL   fOK = TRUE;
   #define ITEMBUFLEN 2000

   // Allocate buffer for listbox items
   fOK = UtlAlloc( (PVOID *)&pszString, 0L, (LONG)ITEMBUFLEN, ERROR_STORAGE );

   /*******************************************************************/
   /* Get number of items of source listbox                           */
   /*******************************************************************/
   if ( fOK )
   {
     sNum = QUERYITEMCOUNTHWND( hwndSource );
   } /* endif */

   /*******************************************************************/
   /* Copy listbox items                                              */
   /*******************************************************************/
   if ( fOK )
   {
    for ( sIndex = 0; sIndex < sNum; sIndex++ )
    {
       LONG lHandle;
       SHORT sTargetItem;

       //get item text and handle of source listbox
       QUERYITEMTEXTHWNDL( hwndSource, sIndex, pszString, ITEMBUFLEN );
       lHandle = (LONG) QUERYITEMHANDLEHWND( hwndSource, sIndex );

       //display item text in target listbox
       sTargetItem = INSERTITEMHWND( hwndTarget, pszString );
       SETITEMHANDLEHWND( hwndTarget, sTargetItem, lHandle );
    } /* endfor */
   } /* endif */

   return ( sNum );

} /* end of UtlCopyListBox */

BOOL UtlIsHighContrast()
{
  HIGHCONTRAST hc;
  BOOL         fHighContrast = FALSE;

  hc.cbSize = sizeof(hc);
  if (SystemParametersInfo( SPI_GETHIGHCONTRAST, sizeof( hc ), &hc, FALSE )
	  && (hc.dwFlags & HCF_HIGHCONTRASTON ))
  {
	  fHighContrast = TRUE;
  }
  return (fHighContrast);
}

//+----------------------------------------------------------------------------+
//Internal function
//+----------------------------------------------------------------------------+
//Function Name:     UtlRegisterModelessDlg   Register a modeless dialog
//+----------------------------------------------------------------------------+
//Description:       Registers a modeless dialog (Windows)
//                   Dummy function (OS/2)
//+----------------------------------------------------------------------------+
//Function Call:     BOOL UtlRegisterModelessDlg( HWND hwndDlg );
//+----------------------------------------------------------------------------+
//Parameters:        hwndDlg is the window handle of the modeless dialog
//+----------------------------------------------------------------------------+
//Returncode type:   BOOL
//+----------------------------------------------------------------------------+
//Returncodes:       TRUE   dialog has been registered
//                   FALSE  dialog could not be registered
//+----------------------------------------------------------------------------+
BOOL UtlRegisterModelessDlg( HWND hwndDlg )
{
  BOOL             fOK = TRUE;         // function return value

  if ( sRegisteredDlgs < MAX_DIALOGS )
  {
    hwndDlgs[sRegisteredDlgs] = hwndDlg;
    sRegisteredDlgs++;
  }
  else
  {
    fOK = FALSE;                       // no more room left in hwnd array
  } /* endif */

  return( fOK );
} /* end of function UtlRegisterModelessDlg */

//+----------------------------------------------------------------------------+
//Internal function
//+----------------------------------------------------------------------------+
//Function Name:     UtlUnregisterModelessDlg   Unregister a modeless dialog
//+----------------------------------------------------------------------------+
//Description:       Unregisters a modeless dialog (Windows)
//                   Dummy function (OS/2)
//+----------------------------------------------------------------------------+
//Function Call:     BOOL UtlUnregisterModelessDlg( HWND hwndDlg );
//+----------------------------------------------------------------------------+
//Parameters:        hwndDlg is the window handle of the modeless dialog
//+----------------------------------------------------------------------------+
//Returncode type:   BOOL
//+----------------------------------------------------------------------------+
//Returncodes:       TRUE   dialog has been unregistered
//                   FALSE  dialog was not found in registeres dialogs
//+----------------------------------------------------------------------------+
BOOL UtlUnregisterModelessDlg( HWND hwndDlg )
{
  BOOL             fOK = TRUE;         // function return value
  SHORT            sIndex;             // loop index

  /********************************************************************/
  /* Search dialog in array of registered dialogs                     */
  /********************************************************************/
  sIndex = 0;
  while ( sIndex < sRegisteredDlgs )
  {
    if ( hwndDlgs[sIndex] == hwndDlg )
    {
      break;                           // gotcha!, leave loop
    }
    else
    {
      sIndex++;                        // try next one
    } /* endif */
  } /* endwhile */

  /********************************************************************/
  /* Remove dialog or set return code                                 */
  /********************************************************************/
  if ( sIndex < sRegisteredDlgs )
  {
    if ( (sIndex < (sRegisteredDlgs - 1)) && (sRegisteredDlgs > 1) )
    {
      /****************************************************************/
      /* Shift all following window handles to avoid gaps in the      */
      /* array                                                        */
      /****************************************************************/
      memmove( &(hwndDlgs[sIndex]),
               &(hwndDlgs[sIndex+1]),
               (sRegisteredDlgs - sIndex - 1) * sizeof(HWND) );
    } /* endif */
    sRegisteredDlgs--;
  }
  else
  {
    fOK = FALSE;                       // dialog is not in our hwnd array
  } /* endif */

  return( fOK );
} /* end of function UtlUnregisterModelessDlg */

//+----------------------------------------------------------------------------+
//External function
//+----------------------------------------------------------------------------+
//Function Name:     UtlKeepInTWB      Ensure that a window is inside the TWB
//+----------------------------------------------------------------------------+
//Description:       Corrects the given window size+position so that it is
//                   inside the TWB.
//+----------------------------------------------------------------------------+
//Function Call:     USHORT UtlKeepInTWB( PSWP pswpWin );
//+----------------------------------------------------------------------------+
//Parameters:        pswpWin points to a SWP structure containing the
//                   size and position of the window
//+----------------------------------------------------------------------------+
//Returncode type:   BOOL
//+----------------------------------------------------------------------------+
//Returncodes:       TRUE    position has been changed
//                   FALSE   there was no need to change the window position
//+----------------------------------------------------------------------------+
BOOL UtlKeepInTWB( PSWP pswpWin )
{
  SWP              swpTWB;             // size+pos of translation workbench
  BOOL             fChanged = FALSE;   // window position changed flag

  /********************************************************************/
  /* Get TWB size/position                                            */
  /********************************************************************/
  WinQueryWindowPos( (HWND)UtlQueryULong( QL_TWBCLIENT ), &swpTWB );

  /********************************************************************/
  /* Ensure that window is not outside of the TWB                     */
  /********************************************************************/
  if ( pswpWin->x < 0L )
  {
    pswpWin->x = 0L;
    fChanged = TRUE;
  } /* endif */

  if ( (pswpWin->x + pswpWin->cx) > swpTWB.cx )
  {
    pswpWin->x = max( (swpTWB.cx - pswpWin->cx), 0 );
    fChanged = TRUE;
  } /* endif */

  if ( (pswpWin->y + pswpWin->cy) > swpTWB.cy )
  {
    pswpWin->y = swpTWB.cy - pswpWin->cy;
    fChanged = TRUE;
  } /* endif */

  if ( pswpWin->y < 0L )
  {
    pswpWin->y = min( (swpTWB.cy - pswpWin->cy), 0 );
    fChanged = TRUE;
  } /* endif */

  return( fChanged );

} /* end of function UtlKeepInTWB */

/*! UtlCheckDlgPos     Check that dialog is in visible area
	Description:       Get the current dialog position and repositions dialog
					to keep dialog in visible area of the desktop. If the
					dialog is too large at least the dialog titlebar is shown
	Function Call:     USHORT UtlCheckDlgPos( HWND hwndDlg, BOOL fShow );
	Parameters:        hwndDlg is the handle of the dialog window
					fShow if TRUE the dialog is shown and activated
	Returncode type:   USHORT    (currently not in use)
	Returncodes:       NO_ERROR
*/
USHORT UtlCheckDlgPos( HWND hwndDlg, BOOL fShow )
{
  SWP         swpDlg;                  // size+pos of dialog
  BOOL        fPosChanged = FALSE;     // TRUE id position has been changed
  LONG        cxScreen, cyScreen;      // size of physical screen

  /********************************************************************/
  /* Get dialog size and position                                     */
  /********************************************************************/
  WinQueryWindowPos( hwndDlg, &swpDlg );

  /********************************************************************/
  /* Get width and height of visible desktop area                     */
  /********************************************************************/
  cxScreen = (LONG)WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
  cyScreen = (LONG)WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

  /********************************************************************/
  /* Ensure that window is not outside of the visible desktop area    */
  /********************************************************************/
  if ( swpDlg.x < 0L )
  {
    swpDlg.x = 0L;
    fPosChanged = TRUE;
  } /* endif */

  if ( (swpDlg.x + swpDlg.cx) > cxScreen )
  {
    swpDlg.x = (SHORT)(max( (cxScreen - swpDlg.cx), 0 ));
    fPosChanged = TRUE;
  } /* endif */

  if ( swpDlg.y < 0L )
  {
    swpDlg.y = 0L;
    fPosChanged = TRUE;
  } /* endif */

  if ( (swpDlg.y + swpDlg.cy) > cyScreen )
  {
    swpDlg.y = (SHORT)(cyScreen - swpDlg.cy);
    fPosChanged = TRUE;
  } /* endif */



  /********************************************************************/
  /* Re-position dialog if necessary                                  */
  /********************************************************************/
  if ( fPosChanged || fShow )
  {
    if ( fShow )
    {
      WinSetWindowPos( hwndDlg, HWND_TOP, swpDlg.x, swpDlg.y, swpDlg.cx,
                       swpDlg.cy,
                       EQF_SWP_MOVE | EQF_SWP_SHOW | EQF_SWP_ACTIVATE );
    }
    else
    {
      WinSetWindowPos( hwndDlg, HWND_TOP, swpDlg.x, swpDlg.y, swpDlg.cx,
                       swpDlg.cy, EQF_SWP_MOVE );
    } /* endif */
  } /* endif */
  return( NO_ERROR );

} /* end of function UtlCheckDlgPos */

/*! UtlSaveWindowPos       Save a window position
	Description:       Save position of a window regardless whether it is in a
					MINIMIZED or MAXIMIZED state for restore purposes
	Parameters:        1. HWND - frame window handle
					2. PSWP - ptr to SWP struct to store position in
*/
VOID UtlSaveWindowPos( HWND hwndFrame, EQF_PSWP pSwp)
{
   WinQueryWindowPos( hwndFrame, pSwp);
}

// isolate the next file name from a comma seperated list, the list is modified
BOOL UtlGetNextFileFromCommaList
(
  PSZ  *ppszNameStart,                 // current start of file name
  PSZ  *ppszNext                       // next file name position or NULL if first call
)
{
  BOOL fFileFound = TRUE;
  PSZ  pszCurrent = *ppszNameStart;
  PSZ  pszEnd     = *ppszNext;

  // continue with next file if not the first one (end pointer is NULL for first file)
  if ( pszEnd )
  {
    pszCurrent = pszEnd;
  } /* endif */

  // skip leading blanks and seperators
  while ( (*pszCurrent == SPACE) || (*pszCurrent == COMMA) ) pszCurrent++;

  // find end of file name
  if ( *pszCurrent == DOUBLEQUOTE )
  {
    // find end of file name enclosed in double quotes
    pszCurrent++;   // skip starting double quote
    pszEnd = strchr( pszCurrent, DOUBLEQUOTE );
    if ( pszEnd == NULL )        // no ending double quote ????
    {
      // look for next comma instead
      pszEnd = strchr( pszCurrent, ',' );
      if ( pszEnd ) *pszEnd++ = EOS;
    }
    else
    {
      // terminate file name and go to next character
      *pszEnd++ = EOS;
    } /* endif */
  }
  else if ( *pszCurrent )
  {
    // search for next file name delimiter
    pszEnd = strchr( pszCurrent, ',' );
    if ( pszEnd ) *pszEnd++ = EOS;
  }
  else
  {
    fFileFound = FALSE;
  } /* endif */

  // if no delimiter found, use end of string for the end pointer
  if ( fFileFound && (pszEnd == NULL) )
  {
    pszEnd = pszCurrent + strlen(pszCurrent);
  } /* endif */

  *ppszNameStart = pszCurrent;
  *ppszNext = pszEnd;

  return( fFileFound );
} /* end of function UtlGetNextFileFromCommaList */

//
// Function UtlValidateList
//
// Converts a file name list to a pointer array with pointers to the individual
// file names in the list. The list is destroyed as hex0 is inserted to split
// the file names. A valid list contains a list of file names enclosed in ().
// The file names may be seperated by blank, comma, and/or carriage return/linefeed.
// File names containing blanks must be enclosed in double quotes.
//
// Parameters:  PSZ    pStart        pointer to start of input string
//              PSZ    *ppListIndex  pointer to listindex  - return value
//              USHORT usMaxAlloc    max pointers to be allocated
//
// Returncode type:   BOOL
//
// Returncodes:       TRUE     the passed string adheres to the list syntax
//                    FALSE    the passed string is not a list

#define LISTSTART '('
#define LISTEND   ')'

BOOL UtlValidateList
(
  PSZ pStart,                          // start of list
  PSZ **pppListIndex,                  // pointer to list array
  USHORT usMaxAlloc                    // max ptrs to be allocated
)
{
  BOOL fOK = TRUE;
  LONG lI;                             // index in list pointer array
  PSZ  pToken;                         // pointer to tokens
  PSZ  *ppListTokens = NULL;           // pointer to list token array
  USHORT usChar;                       // character to be checked

/**********************************************************************/
/* skip leading blanks ,lf and cr                                     */
/**********************************************************************/
  usChar = *(pStart);
  while ( usChar ==  LF || usChar == CR || usChar == BLANK)
  {
    pStart++;
    usChar = *(pStart);
  } /* endwhile */
  /********************************************************************/
  /* set pStart to the begin of the information, make uppercase       */
  /********************************************************************/
  // changed to have upper and lower case characters
  // strupr(pStart);
  lI = strlen(pStart);

  /********************************************************************/
  /* skip blanks at end of list                                       */
  /********************************************************************/
  while ( lI && ((pStart[lI-1] == BLANK ) || (pStart[lI-1] == CR) || (pStart[lI-1] == LF)) )
  {
    lI--;
  } /* endwhile */

  if ( !((*pStart == LISTSTART) && (*(pStart+lI-1) == LISTEND)) )
  {
    if ((*pStart == LISTSTART) || (*(pStart+lI-1) == LISTEND))
    {
      /****************************************************************/
      /* if only liststart or listend is specified, cmdline is wrong  */
      /****************************************************************/
      fOK = FALSE;
    }
    else
    {
      /****************************************************************/
      /* only one filename given, delimiter blank, linefeed           */
      /****************************************************************/
      fOK = UtlAlloc( (PVOID *)&ppListTokens, 0L, 2L * sizeof(PSZ) + MAX_LONGPATH, NOMSG );
      if ( fOK )
      {
        // blank is a valid character in long file names ands may not be used
        // delimiter
        pToken = strtok(pStart,"\n\r");
        pStart = (PSZ)ppListTokens+2*sizeof(PSZ);// point to start for filename
        strcpy( pStart, pToken );                // copy filename into space
        ppListTokens[0] = pStart;                // set pointer to it...
        *pppListIndex = ppListTokens;
      } /* endif */
    } /* endif */
  }
  else
  {
    // count the number of possible tokens in the list...
    // (we will use this value for the allocation of the token list if
    // is is larger than the supplied usMaxAlloc value
    {
      PSZ p = pStart;
      USHORT usTokens = 10;

      while ( *p != EOS )
      {
        if ( (*p == ' ') || (*p == ',') || (*p == CR) || (*p == LF) )
        {
          while ( (*p == ' ') || (*p == ',') || (*p == CR) || (*p == LF) )
          {
            p++;
          } /* endwhile */
          usTokens++;
        }
        else
        {
          p++;
        } /* endif */
      } /* endwhile */

      if ( usTokens > usMaxAlloc )
      {
        usMaxAlloc = usTokens;
      } /* endif */
    }

    /******************************************************************/
    /* allocate memory for 2000 tokens -- should be enough ...        */
    /******************************************************************/
    fOK = UtlAlloc( (PVOID *)&ppListTokens, 0L,
                    (LONG) usMaxAlloc*sizeof(PSZ), NOMSG);

    if ( fOK )
    {
      /******************************************************************/
      /* extract the pointers to list items                             */
      /******************************************************************/
      *(pStart+lI-1) = EOS;
      pStart++;                          // skip list indication

//      lI = 0;
//      pToken = strtok( pStart, " ,\n\r" );
//      ppListTokens[lI++] = pToken;
//
//      while ( pToken && (lI < usMaxAlloc) )
//      {
//        pToken = strtok( NULL, " ,\n\r" );
//        ppListTokens[lI++] = pToken;
//      } /* endwhile */


      lI = 0;

      // get all tokens in list
      pToken = pStart;
      while ( *pToken && (lI < usMaxAlloc) )
      {
        // skip leading whitespace and delimiters
        while ( (*pToken == ' ') || (*pToken == ',') || (*pToken == '\n') || (*pToken == '\r') )
        {
          pToken++;
        } /* endwhile */

        // remember start of current token and find token end
        if ( *pToken == '\"' )
        {
          // skip anything up to closing double quote
          pToken++;
          ppListTokens[lI++] = pToken;
          while ( *pToken && (*pToken != '\"') )
          {
            pToken++;
          } /* endwhile */

          // terminate token and continue with next char
          if ( *pToken ) *pToken++ = EOS;
        }
        else if ( *pToken )
        {
          ppListTokens[lI++] = pToken;

          // skip anything up to next delimiter
          while ( *pToken && (*pToken != ' ') && (*pToken != ',') && (*pToken != '\n') && (*pToken != '\r') )
          {
            pToken++;
          } /* endwhile */
          // terminate token and continue with next char
          if ( *pToken ) *pToken++ = EOS;
        } /* endif */
      } /* endwhile */
      ppListTokens[lI++] = NULL;      // terminate token list

      if ( fOK )
      {
        *pppListIndex = ppListTokens;
      } /* endif */
    } /* endif */
  } /* endif */
  return fOK;
} /* end of function UtlValidateList */

ULONG UtlFileTimeToLong(PSZ pszFileName)
{
  ULONG ulTime = 0L;
  USHORT usRC = 0;
  BOOL   fOK = FALSE;
  HANDLE hOutFile = NULL;
  USHORT usOpenAction;
  FILETIME ftFileTime;
  FILETIME ftFileTimeLocal;
  SYSTEMTIME SystemTime;
  struct tm tm_timedate;

  memset(&SystemTime, 0, sizeof(SystemTime));


  if ((usRC = UtlOpen( pszFileName,
                   &hOutFile,
                   &usOpenAction, 0L,
                   FILE_NORMAL,
                   FILE_OPEN,
                   OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                   0L,
                   FALSE ))== 0)
  {
    fOK = GetFileTime(hOutFile, NULL, NULL, &ftFileTime);

    if ( fOK )
    {
      fOK = FileTimeToLocalFileTime( &ftFileTime, &ftFileTimeLocal );
    } /* endif */

    if (fOK)
    {
      fOK = FileTimeToSystemTime( &ftFileTimeLocal ,&SystemTime );
    }
    if (fOK)
    {
      tm_timedate.tm_year = SystemTime.wYear-1900;
      tm_timedate.tm_mon  = SystemTime.wMonth-1;
      tm_timedate.tm_mday = SystemTime.wDay;
      tm_timedate.tm_hour = SystemTime.wHour;
      tm_timedate.tm_min  = SystemTime.wMinute;
      tm_timedate.tm_sec  = SystemTime.wSecond;
      tm_timedate.tm_isdst= -1;
      tm_timedate.tm_wday = 0;
      tm_timedate.tm_yday = 0;

      ulTime = (ULONG) mktime(&tm_timedate);
      ulTime -= 10800; // To become compliant with OS/2 time format

      // test to check if computed value s in-sync with original time
      {
        struct tm   *pTimeDate;    // time/date structure
        long lTime = ulTime + 10800L;
        pTimeDate = localtime( (time_t*) &lTime );
        pTimeDate->tm_year = tm_timedate.tm_year;
      }
    }
    UtlClose( hOutFile, FALSE );
  }
  return (ulTime);

}

/*! UtlMakeFullPath    Combine path components
	Description:       Make a fully qualified path/file name.
	Function Call:     PSZ UtlMakeFullPath( PSZ pszPath,
											PSZ pszDrive,
											PSZ pszDir,
											PSZ pszName,
											PSZ pszExt );
	Parameters:        - pszPath is a pointer to the buffer receivng the
						created path
					- pszDrive points to the drive letter string (e.g. "E:")
						and may be NULL (if drive is contained in pszDir)
					- pszDir points to the directory path. If pszDir does
						not start with a backslash, a backslash is added.
					- pszName points to the file name
					- pszExt points to the filename extension and may be
						NULL (if filename extension is contained in pszName)
						If pszExt does not start with a dot ('.') a dot is
						added.
	Returncode type:   PSZ
	Returncodes:       ptr to create path name
	Function flow:     clear output buffer
					if drive given add drive to output buffer
					if path given add path to output buffer
					if name given add name to output buffer
					if extension given add extension delimiter (if not
						contained in extension) and extension to output buffer
*/
PSZ UtlMakeFullPath
(
  PSZ pszPath,                         // pointer to resulting filename
  PSZ pszDrive,                        // pointer to drive string
  PSZ pszDir,                          // pointer to directory
  PSZ pszName,                         // pointer to filename
  PSZ pszExt                           // pointer to extension
)
{
   *pszPath = EOS;

   if ( pszDrive )
   {
      strcat( pszPath,  pszDrive );
   } /* endif */

   if ((  pszDir )              &&     // if directory has been specified and
       ( *pszDir != BACKSLASH ) &&     // if directory has no leading backslash
       ( *pszPath != EOS  ) )          // and path is not empty
   {
      strcat( pszPath, BACKSLASH_STR );//add backslash to path
   } /* endif */

   if ( pszDir )
   {
     strcat( pszPath, pszDir );        //append passed DIR to result
   } /* endif */

   if ( (  pszName )              &&   // if name specified and
        ( *pszName != BACKSLASH ) &&   // name contains no backslash and
        ( pszPath[strlen(pszPath)-1] != BACKSLASH ) )   // path has no backslash
   {
      strcat( pszPath, BACKSLASH_STR );// add backslash to path
   } /* endif */


   if ( pszName )
   {
     strcat( pszPath,  pszName  );
   } /* endif */

   if ( pszExt && *pszExt)
   {
      if ( *pszExt != DOT )
      {
         pszPath[strlen(pszPath)+1] = EOS;
         pszPath[strlen(pszPath)]   = DOT;
      } /* endif */
      strcat( pszPath, pszExt );
   } /* endif */

   return ( pszPath );

} /* end UtlMakeFullPath */

VOID UtlLoadStringW
(
  HAB hInst,
  HMODULE hmod,
  UINT sid,
  PCHAR pszBuffer,
  PSZ_W pBufW,
  USHORT usBufWMax)
{
  hInst;
  LOADSTRINGLEN(hInst, hmod, sid, pszBuffer, usBufWMax);
  ASCII2Unicode(pszBuffer, pBufW, 0L );
}

/**********************************************************************/
/* Load and convert a column width string from a resource             */
/**********************************************************************/
USHORT UtlLoadWidth
(
  HAB         hab,                     // anchor block handle
  HMODULE     hMod,                    // resource module handle
  SHORT       sId,                     // string ID
  PUSHORT     pusWidth                 // ptr to caller's width buffer
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  USHORT      usWidth = 0;             // width from resource string
  CHAR        szWidth[20];             // width string from the resource

  hab;
  // get width string
  szWidth[0] = EOS;
  LOADSTRING( hab, hMod, sId, szWidth );

  // convert string to number
  usWidth = (USHORT)atoi( szWidth );

  // place in caller's buffer if not empty
  if ( usWidth != 0 )
  {
    *pusWidth = usWidth;
  } /* endif */

  // return to calling function
  return( usRC );
} /* end of function UtlLoadWidth */

//The call back is used to set the initial directory when using SHBrowseForFolder
int CALLBACK UtlBrowseForFolderCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{
  lp;

  switch(uMsg)
  {
    case BFFM_INITIALIZED:
        //Initial directory is set here
        {
            // WParam is TRUE since you are passing a path.
            // It would be FALSE if you were passing a pidl.
            SendMessage(hwnd,BFFM_SETSELECTION,TRUE,pData);
        }
        break;
    default:
        break;
  }
  return 0;
}

/*! UtlPreloadIcons
	Description:       Load all icons from resoure file and add icon handles
						to our long values table.
	Function call:     UtlPreloadIcons();
	Returncode type:   BOOL
	Returncodes:       TRUE (in any case)
	Prerequesits:      resource module must have been loaded
	Side effects:      none
*/
BOOL UtlPreloadIcons(void)
{
	HMODULE hResMod;
	HPOINTER hPtr;

	hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, DICTIMP_ICON );
	UtlSetULong( QL_DICTIMPICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, DICTEXP_ICON );
	UtlSetULong( QL_DICTEXPICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, FOLIMP_ICON );
	UtlSetULong( QL_FOLIMPICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, FOLEXP_ICON );
	UtlSetULong( QL_FOLEXPICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, TWB_ICON );
	UtlSetULong( QL_TWBICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, FLL_ICON );
	UtlSetULong( QL_FLLICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, FOL_ICON );
	UtlSetULong( QL_FOLICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, DOC_ICON );
	UtlSetULong( QL_DOCICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, ANA_ICON );
	UtlSetULong( QL_ANAICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, MEM_ICON );
	UtlSetULong( QL_MEMICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, TAG_ICON );
	UtlSetULong( QL_TAGICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, COUNT_ICON );
	UtlSetULong( QL_COUNTICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, TMEMIMP_ICON );
	UtlSetULong( QL_TMEMIMPICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, TMEMEXP_ICON );
	UtlSetULong( QL_TMEMEXPICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, TMEMMERGE_ICON );
	UtlSetULong( QL_TMEMMERGEICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, TMEMORG_ICON );
	UtlSetULong( QL_TMEMORGICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, MARKUPIMP_ICON );
	UtlSetULong( QL_MARKUPIMPICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, MARKUPEXP_ICON );
	UtlSetULong( QL_MARKUPEXPICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, DICTORG_ICON );
	UtlSetULong( QL_DICTORGICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, DICTLIST_ICON );
	UtlSetULong( QL_DICTLISTICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, LIST_ICON );
	UtlSetULong( QL_LISTICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, LIST_EXCL_ICON );
	UtlSetULong( QL_EXCLLISTICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, LIST_NEW_ICON );
	UtlSetULong( QL_NEWLISTICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, LIST_FOUND_ICON );
	UtlSetULong( QL_FOUNDLISTICON, (ULONG)hPtr );

	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, ID_DICTPRINT_ICON );
	UtlSetULong( QL_DICTPRINTICON, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, DICTENTRYDISP_ICON );
	UtlSetULong( QL_DICTENTRYDISPICO, (ULONG)hPtr );
	hPtr = WinLoadPointer( HWND_DESKTOP, hResMod, ID_TMM_ICON );
	UtlSetULong( QL_TMMICON, (ULONG)hPtr );
	return( TRUE );
} /* end of function UtlPreloadIcons */

//*************************************************************
//
//  EqfHelpButtonProc ()
//
//  Purpose:
//      The window function for superclassed help buttons
//
//      The EqfHelpButton class implements buttons which
//      should work like the BS_HELP buttons of OS/2
//
//  Parameters:
//      ( HWND, WORD, WORD, LONG)
//
//  Return: (LONG)
//
//*************************************************************
LONG FAR PASCAL EqfHelpButtonProc(register HWND hwnd, UINT msg,
                                 register WPARAM mp1, LPARAM mp2)
{
  MRESULT          mResult;            // function return code

  switch ( msg )
  {
    // GQ 2015/10/13: Disable help buttons until we have a running help system....
    case WM_SHOWWINDOW:
      EnableWindow( hwnd, FALSE );
      break;

	case WM_KEYDOWN:
		if ( (msg == WM_KEYDOWN) && (LOWORD(mp1) == VK_SPACE))
		{
			return( UtlInvokeHelp() );
		}
		else
		{
			break;
		} /*endif*/

    case WM_LBUTTONDOWN:
      return( UtlInvokeHelp() );

    case WM_LBUTTONUP:
      return( MRFROMSHORT(FALSE) );

  } /* endswitch */

  // call old window procedure
  mResult = CallWindowProc( (WNDPROC)lpfnOldButtonProc, hwnd, msg, mp1, mp2 );

  return( mResult );
}

MRESULT UtlInvokeHelp()
{
	HWND hTemp = NULL;
	HWND hParent = GetFocus();
	SHORT   sControlID = 0;
	SHORT   sParentID  = 0;
	CHAR    szClass[40];          // buffer for class names

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
		if ( _stricmp( szClass, WC_EQFMDIDLG ) == 0 )
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

	PostMessage( (HWND)UtlQueryULong( QL_TWBFRAME ),
				 HM_HELPSUBITEM_NOT_FOUND,
				 0,
				 MP2FROM2SHORT( sParentID, sControlID ) );
    return( MRFROMSHORT(FALSE) );
}

/******************************************************************/
/* Register class for superclassed BS_HELP buttons                */
/******************************************************************/
BOOL UtlRegisterEqfHelp( HAB hAB )
{
  BOOL fOK = TRUE;

  WNDCLASS  wc;

  // SuperClass the BUTTON window. This involves getting the info
  // of the original class and then making the necessary modifications

  GetClassInfo( NULL,"Button", &wc );
  wc.style         |= CS_GLOBALCLASS;
  lpfnOldButtonProc = (FARPROC)wc.lpfnWndProc;  // Save old proc
  wc.lpszClassName = EQFHELPBUTTON;            // Change the class name
  wc.hInstance     = (HINSTANCE)  hAB;         // Change the instance so that
                                               // the class gets unregistered
                                               // when the app terminates
  wc.lpfnWndProc   = EqfHelpButtonProc;        // Our new proc

  // Register the puppy -- don't care about return code...
  RegisterClass( &wc );

  return fOK;
}

/*! UtlConvertBuf                                            
	Description:       Converts the contents of the buffer                      
					all conversion types available in API EQFFILECONVERSIONEX
					must be supported here too                               
	Function call:     ulOutPut = UtlConvertBuf( usType,						   
												pvInString,  				   
													pvOutString,				   
													ulLen,						   
													lBufLen,					   
													ulASCIICP, ulAnsiCP 		   
												fMsg, plRc, plBytesLeft);      
	Input parameter:    usType                                               
					pvInString,  				                            
						pvOutString,				                            
						ulLen,		// # of bytes in InString				    
						lBufLen,	// # of bytes which fit in output buf!		
						ulASCIICP,	// if NULL, def.tgt.lng is used             
					ulAnsiCP    // 0 if not req.                            
					fMsg        // TRUE: display error msg                  
					plRc        // if NULL, no error return possible        
					plBytesLeft   // # of bytes at end of InString          
								//  which are not yet processed           
	Returncode type:   ULONG                                                     
	Returncodes:       # of bytes written to OutBuf                              
	Prerequesits:      InBuf filled with data in specified format               
	Side effects:      lRc = ERROR_INSUFFICIENT_BUFFER (122)                    
						if lBufLen not large enough                             
	ANSI2UTF8: if lBufLen= 0, required buffersize is returned in ulOutPut      
*/
ULONG UtlConvertBuf
(
	USHORT usType,         // all types available in EQFFILECONVERSION are allowed!
	PVOID pvInString,
	PVOID pvOutString,
	ULONG ulLen,            // # of bytes in In-Buffer
	LONG  lBufLen,			// # of bytes which fit in Out-Buffer
	ULONG ulASCIICP,        // ASCII-CP!!(if NULL & req., def.tgt.lang. OEMCP is used!)
	ULONG ulAnsiCP,
	BOOL  fMsg,				 // TRUE: displ. error msg
	PLONG plRc,               // if NULL: no error return possible
	PLONG  plBytesLeft
)
{
	ULONG ulOutPut = 0;
	ULONG ulOutPut2 = 0;
	LONG  lRc = 0;
	ULONG ulWideChars = 0;
	LONG  lBytesLeft = 0;

	if (!ulASCIICP)
	{
	   ulASCIICP = GetLangOEMCP(NULL); // get System Preferences default target lang's CP
    }

  // always use 932 when 943 is specified
  if ( ulASCIICP == 943 ) ulASCIICP = 932;


	switch(usType)
	{ case EQF_ASCII2ANSI:          // conversion via pConvTable....
	// ASCII cp as in-CP required here!
	// for DBCS ASCII cp == Ansi cp
        if (!IsDBCS_CP(ulASCIICP))
        {
	      EQFCPOemToAnsiBuff( (USHORT)ulASCIICP, (PSZ) pvInString,
                            0, (PSZ) pvOutString, (USHORT) ulLen );
	    }
	    else
	    {// it is not nec. to take special care of DBCS1st-chars at end of buffer
	          memcpy(pvOutString, pvInString, ulLen);
	    }
	    ulOutPut = ulLen;
		break;
	  case EQF_ANSI2ASCII:
	    // for DBCS, conversion not nec!!
	    //ASCII cp required here!!
	     if (!IsDBCS_CP(ulASCIICP))
         {
           EQFCPAnsiToOemBuff( 0/*not nec*/, (PSZ) pvInString,
                            (USHORT) ulASCIICP, (PSZ) pvOutString, (USHORT) ulLen );
         }
	     else
	     {
	          memcpy(pvOutString, pvInString, ulLen);
	     }
	     ulOutPut = ulLen;
	    break;
	  case EQF_ASCII2UTF8:
	    { PSZ_W pTemp;
		  //extra buffer nec. for UTF16 string!
		  if (!UtlAlloc( (PVOID *) &pTemp, 0L, (ulLen+1) * sizeof(CHAR_W), NOMSG ))
		  {
			  lRc = ERROR_STORAGE;
	      }
	      else
	      {
		      ulOutPut2= ASCII2UnicodeBufEx( (PSZ) pvInString, pTemp, ulLen, ulASCIICP,
		                                  FALSE, &lRc, &lBytesLeft);
		      if (!lRc)
              {
		         ulOutPut = Unicode2UTF8BufEx( (PSZ_W) pTemp, (PSZ) pvOutString, ulOutPut2, lBufLen,
		                                 FALSE, &lRc);
	          }
        	  UtlAlloc((PVOID *)&pTemp, 0L, 0L, NOMSG );
	      }
	    }
	    break;

	  case  EQF_UTF82ASCII:
	     { PSZ_W pTemp;
	      //extra buffer nec. for UTF16 string!
	      if (!UtlAlloc( (PVOID *) &pTemp, 0L, (ulLen+1) * sizeof(CHAR_W), NOMSG ))
	      {
			  lRc = ERROR_STORAGE;
	      }
	      else
	      {
	        ulOutPut2 = UTF82UnicodeBufEx( (PSZ) pvInString, pTemp, ulLen,
	                                     FALSE, &lRc, &lBytesLeft);

	        // lBufLen number of Bytes which fit in ASCII output buffer
            if ( !lRc)
            {
	          ulOutPut = Unicode2ASCIIBufEx(pTemp, (PSZ)pvOutString, ulOutPut2, lBufLen,
	                                  ulASCIICP, &lRc);
	        }
	        UtlAlloc( (PVOID *) &pTemp, 0L, 0L, NOMSG );
	      }
	     }
	    break;
      case  EQF_ASCII2UTF16:
         { // ulLen in number of CHAR_W's which can be in pszUni!
           ulOutPut= ASCII2UnicodeBufEx( (PSZ) pvInString, (PSZ_W) pvOutString, ulLen,
                                      ulASCIICP, FALSE, &lRc, &lBytesLeft );
           ulOutPut = ulOutPut * sizeof(CHAR_W);
	     }
         break;
	  case  EQF_UTF162ASCII:
	     ulWideChars = ulLen / sizeof(CHAR_W);
	     ulOutPut = Unicode2ASCIIBufEx( (PSZ_W)pvInString, (PSZ)pvOutString, ulWideChars,
	                                  lBufLen, ulASCIICP, &lRc );
	    break;
	  case  EQF_ANSI2UTF8:
	     { PSZ_W pTemp;
		  //extra buffer nec. for UTF16 string!
		     if (!UtlAlloc( (PVOID *) &pTemp, 0L, (ulLen+1) * sizeof(CHAR_W), NOMSG ))
		     {
				 lRc = ERROR_STORAGE;
		     }
		     else
		     {  //rc ulOutPut2 is # of char-w's
		       ulOutPut2 = UtlDirectAnsi2UnicodeBuf( (PSZ) pvInString, pTemp, ulLen,
		                                     ulAnsiCP, FALSE, &lRc, &lBytesLeft );
		       if (!lRc)
		       {
		         ulOutPut = Unicode2UTF8BufEx( (PSZ_W) pTemp, (PSZ) pvOutString,
		                                  ulOutPut2, lBufLen, FALSE, &lRc);
	           }
		       UtlAlloc( (PVOID *) &pTemp, 0L, 0L, NOMSG );
		     }
	    }
	    break;
	  case  EQF_UTF82ANSI:
	    { PSZ_W pTemp;
	  	      //extra buffer nec. for UTF16 string!
	  	      if (!UtlAlloc( (PVOID *) &pTemp, 0L, (ulLen+4) * sizeof(CHAR_W), NOMSG ))
	  	      {
				  lRc = ERROR_STORAGE;
		      }
		      else
		      {
    	  	      ulOutPut2 = UTF82UnicodeBufEx( (PSZ) pvInString, pTemp, ulLen,
	  	                                      FALSE, &lRc, &lBytesLeft);
	  	          if (!lRc)
	  	          {
	  	            ulOutPut = UtlDirectUnicode2AnsiBuf(pTemp, (PSZ)pvOutString, ulOutPut2,
	  	                                        lBufLen, ulAnsiCP, FALSE, &lRc);
		          }
	  	          UtlAlloc( (PVOID *) &pTemp, 0L, 0L, NOMSG );
		      }
	     }
	    break;
	  case  EQF_ANSI2UTF16:
	    ulOutPut = UtlDirectAnsi2UnicodeBuf( (PSZ) pvInString, (PSZ_W) pvOutString, ulLen,
	                                ulAnsiCP, FALSE, &lRc, &lBytesLeft);
	    ulOutPut = ulOutPut * sizeof(CHAR_W);

	    break;
	  case  EQF_UTF162ANSI:
	  // The lBufLen parameter in the following two functions specifies the
      //     actual size of the pAscii/pAnsi buffer in number of bytes
         ulWideChars = ulLen / sizeof(CHAR_W);
	     ulOutPut = UtlDirectUnicode2AnsiBuf( (PSZ_W) pvInString, (PSZ) pvOutString, ulWideChars,
	                                 lBufLen, ulAnsiCP, FALSE, &lRc );

	    break;
	  case  EQF_UTF82UTF16:
	    ulOutPut = UTF82UnicodeBufEx( (PSZ) pvInString, (PSZ_W) pvOutString, ulLen,
	                                         FALSE, &lRc, &lBytesLeft);
	    ulOutPut = ulOutPut * sizeof(CHAR_W);
	    break;
	  case  EQF_UTF162UTF8:
	    ulWideChars = ulLen / sizeof(CHAR_W);
	    ulOutPut = Unicode2UTF8BufEx( (PSZ_W) pvInString, (PSZ) pvOutString, ulWideChars,
	                                   lBufLen, FALSE, &lRc);
	    break;
	  default:
	    break;
    } /* endswitch */


  if (plRc)
  {
	*plRc = lRc;
  }
  if (plBytesLeft)
  {
	  *plBytesLeft = lBytesLeft;         // TRUE means could not be converted!!
  }
  if (fMsg && lRc)
  {        CHAR szTemp[10];
		   PSZ pszTemp = szTemp;
		   sprintf(szTemp, "%d", lRc);

		   UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
  }

	//return number of bytes written to output buffer
	return( ulOutPut );
}

/*!	UtlDefHandlerProc     MAT default handler window proc    
	Description:       Default window procedure for MAT handler object windows. 
					Currently only WM_CLOSE, WM_DESTROY and WM_EQF_TERMINATE 
					are processed, all other messages are routed to          
					WinDefWindowProc.                                        
*/
MRESULT EXPENTRY UTLDEFHANDLERPROC
(
   HWND hwnd,
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT  mResult = 0L;             // result of message processing
   PIDA_HEAD pIda;                     // pointer to handler IDA

   switch ( msg )
   {
      case WM_CLOSE:
         pIda = ACCESSWNDIDA(hwnd, PIDA_HEAD);
         if (pIda)
         {
           EqfRemoveHandler( TWBFORCE, pIda->szObjName );
         }
         break;

      case WM_DESTROY:
     pIda = ACCESSWNDIDA(hwnd, PIDA_HEAD);
         UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
         break;

      case WM_EQF_TERMINATE:
         pIda = ACCESSWNDIDA(hwnd, PIDA_HEAD);
         WinDestroyWindow( pIda->hFrame);
         break;

      default:
         mResult = WinDefWindowProc( hwnd, msg, mp1, mp2 );
         break;

   } /* endswitch */
   return( mResult );
}

/*! UtlGetPidMultipleInstances                               
	Function call:     UtlGetPidMultipleInstances( PSZ, DWORD *)                
	Description:  Seeks for a named process and returns multiple instances      
				as a field of process ID's                                    
				especially for OLE/RPC Codes, it may be extended              
	Returncode type:  VOID                                                      
*/
BOOL UtlGetPidMultipleInstances( PSZ pszProcessName, DWORD *pdwPidArray )
{
   CHAR szFileName[MAX_PATH];
   PSZ  pszFileName;
   PSZ  pszProcName;
   BOOL fEqual;
   BOOL fOK = TRUE;

   DWORD dwPid;

   int i = 0;

   if (!UtlProcessWalk(PROC_TABLE_FIRST, szFileName, &dwPid))
      fOK = FALSE;

   while (dwPid != 0 && fOK)
   {
      pszFileName = szFileName;
      pszProcName = pszProcessName;

      fEqual = TRUE;

      do
      {
         if (toupper(*pszFileName++) != toupper(*pszProcName++))
         {
            fEqual = FALSE;
         }
      }
      while (*pszFileName && *pszProcName);

      if (fEqual)
      {
         pdwPidArray[i++] = dwPid;
         break;
      }

      fOK = UtlProcessWalk(PROC_TABLE_NEXT, szFileName, &dwPid);
   }

   pdwPidArray[i] = 0;

   UtlProcessWalk(PROC_TABLE_CLOSE, szFileName, &dwPid);

   return (fOK);
} // end of UtlGetPidMultipleInstances

/*! UtlKillProcess                                           
	Function call:     UtlKillProcess( DWORD )                                  
	Description:  Kills a process                                               
	Returncode type:  BOOL                                                      
*/

BOOL UtlKillProcess( DWORD dwPid )
{
   HANDLE hProcess;

   hProcess = OpenProcess(PROCESS_TERMINATE, 0, dwPid);
  return(TerminateProcess(hProcess, (unsigned)-1));
}

/*! UtlProcessWalk                                           
	Function call:     BOOL UtlProcessWalk(SHORT mode, PSZ module, DWORD *pid)  
	Description:  Walks through the process table                               
				mode                                                          
				PROC_TABLE_FIRST                                              
				PROC_TABLE_NEXT                                               
				PROC_TABLE_CLOSE                                              
				íf there are no more processes, pid is set to 0               
	Returncode type:  BOOL                                                      
	Note: Win95/98 and NT/2000 require different handling                       
																				
*/
#define PROC_TABLE_ENTRIES 512
#include <string.h>
#include <tlhelp32.h>   // Toolhelp 32


BOOL UtlProcessWalk(SHORT Mode, PSZ BaseName, DWORD *pdwPid)
{
   PSZ szTemp;

   // Win95/98 structures

   typedef BOOL (WINAPI *PROCESSWALK)
   (
    HANDLE hSnapshot,
    LPPROCESSENTRY32 lppe
   );

   typedef HANDLE (WINAPI *CREATESNAPSHOT)
   (
    DWORD dwFlags,
    DWORD th32ProcessID
   );

    HANDLE kernel;
    static HANDLE snapshot;
    PROCESSENTRY32 proc;

    CREATESNAPSHOT CreateToolhelp32Snapshot;
    PROCESSWALK Process32First;
    PROCESSWALK Process32Next = NULL;

    // Win NT structures

    typedef BOOL (WINAPI *ENUMPROCESSES)(
      DWORD * lpidProcess,  // array to receive the process identifiers
      DWORD cb,             // size of the array
      DWORD * cbNeeded      // receives the number of bytes returned
    );

    typedef BOOL (WINAPI *ENUMPROCESSMODULES)(
      HANDLE hProcess,      // handle to the process
      HMODULE * lphModule,  // array to receive the module handles
      DWORD cb,             // size of the array
      LPDWORD lpcbNeeded    // receives the number of bytes returned
    );

    typedef DWORD (WINAPI *GETMODULEFILENAME)(
      HANDLE hProcess,    // handle to the process
      HMODULE hModule,    // handle to the module
      LPTSTR lpstrFileName, // array to receive base name of module
      DWORD nSize     // size of module name array.
    );


    static HANDLE psapi;
    static ENUMPROCESSES       EnumProcesses;
    static GETMODULEFILENAME   GetModuleFileName;
    static ENUMPROCESSMODULES  EnumProcessModules;

    static DWORD process_ids[PROC_TABLE_ENTRIES];
    static DWORD num_processes;

    int success;
    static unsigned i = 0;

    *pdwPid = 0;

    switch(UtlGetOperatingSystemInfo())
    {
    case OP_WINDOWSNT:
    case OP_WINDOWS2K:
    case OP_WINXP:

       // Windows NT/2000

      if (Mode == PROC_TABLE_CLOSE)
      {
         FreeLibrary((HINSTANCE)psapi);
         return(TRUE);
      }

      if (Mode == PROC_TABLE_FIRST)
      {
         i = 0;

         psapi = LoadLibrary("PSAPI.DLL");

         if ( NULL == psapi )
            return(FALSE);

        EnumProcesses = (ENUMPROCESSES)GetProcAddress(
         (HINSTANCE)psapi, "EnumProcesses");

        GetModuleFileName = (GETMODULEFILENAME)GetProcAddress(
          (HINSTANCE)psapi, "GetModuleFileNameExA");

        EnumProcessModules = (ENUMPROCESSMODULES)GetProcAddress(
          (HINSTANCE)psapi, "EnumProcessModules");

        if (
          NULL == EnumProcesses   ||
          NULL == GetModuleFileName ||
          NULL == EnumProcessModules  )
               return(FALSE);

         success = EnumProcesses(process_ids, sizeof(process_ids), &num_processes);

         num_processes /= sizeof(process_ids[0]);

         if ( !success )
            return FALSE;
      }

      while (i < num_processes)
      {

         HANDLE process = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
            FALSE,
            process_ids[i]);

         if ( process )
         {
           HMODULE modules[PROC_TABLE_ENTRIES];
           DWORD num_modules;
           char file_name[MAX_PATH];

           EnumProcessModules(process,
                              modules,
                              sizeof(modules),
                              &num_modules);

           num_modules /= sizeof(modules[0]);

           if(GetModuleFileName(process,
                          modules[0],
                        file_name,
                        sizeof(file_name)))
           {

              *pdwPid = process_ids[i++];

              szTemp = strrchr(file_name, '\\');

              if (szTemp)
                 strcpy(BaseName, szTemp + 1);
              else
                 strcpy(BaseName, file_name);

              CloseHandle(process);

              break;
           }
           CloseHandle(process);
         } /* endif */
         i++;
      }

      if (i > num_processes)
         *pdwPid = 0L;

      return(TRUE);

      break;

    case OP_WINDOWS:

       // Windows 95/98

       if (Mode == PROC_TABLE_CLOSE)
       {
          CloseHandle(snapshot);
          return(TRUE);
       }

       if (Mode == PROC_TABLE_FIRST)
       {
          kernel = (HANDLE)GetModuleHandle(NULL);

          CreateToolhelp32Snapshot =
         (CREATESNAPSHOT)GetProcAddress((HINSTANCE)kernel, "CreateToolhelp32Snapshot");
         Process32First = (PROCESSWALK)GetProcAddress((HINSTANCE)kernel, "Process32First");
         Process32Next  = (PROCESSWALK)GetProcAddress((HINSTANCE)kernel, "Process32Next");

         if (
          NULL == CreateToolhelp32Snapshot  ||
          NULL == Process32First        ||
            NULL == Process32Next)
            return (FALSE);

          proc.dwSize = sizeof(proc);

          snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

          Process32First(snapshot, &proc);

          *pdwPid = proc.th32ProcessID;

          szTemp = strrchr(proc.szExeFile, '\\');

          if (szTemp)
             strcpy(BaseName, szTemp + 1);
          else
             strcpy(BaseName, proc.szExeFile);

       }
       else if (Mode == PROC_TABLE_NEXT)
       {
          if (!Process32Next(snapshot, &proc))
          {
             *pdwPid = 0L;
          }
          else
          {
             szTemp = strrchr(proc.szExeFile, '\\');

             if (szTemp)
                strcpy(BaseName, szTemp + 1);
             else
                strcpy(BaseName, proc.szExeFile);
          }
       }

       return (TRUE);

       break;
    default:
       return(FALSE);
    }
}

/*! UtlCopyParameter                                         
	Function call:     fOK = UtlCopyParameter(szTgt, szSrc, sizeof(szTgt), fMsg)
	Description:       The function will copy szSrc to szTgt if the size of     
					szSrc will match - otherwise it will return false and    
					display an error message (if requested)                  
					In case of errors, the target area is set to '\0'        
	Parameters:        PSZ     pszTgt    pointer to target                      
					PSZ     pszSrc    pointer to Source                      
					USHORT  usSize    size of target area                    
					BOOL    fMsg      message flag                           
	Returncode type:   BOOL                                                     
	Returncodes:       TRUE   if szSrc can be copied into szTgt                 
					FALSE  otherwise                                         
	Prerequesits:      pszTgt has to be allocated                               
	Function flow:     init success indicator                                   
					get stringlength of source string and compare it with    
						provided area.                                         
						if fit then                                            
						copy string                                          
						else                                                   
						init return values                                   
						if message requested display error message           
						endif                                                  
					return success indicator                                 
*/
BOOL
UtlCopyParameter
(
  PSZ     pszTgt,                      // pointer to target
  PSZ     pszSrc,                      // pointer to Source
  USHORT  usSize,                      // size of target area
  BOOL    fMsg                         // message flag
)
{
  BOOL  fOK = TRUE;                    // success indicator
  CHAR  chDebug[40];                   // debug info
  PSZ   pParm[1];                      // pointer array


  /********************************************************************/
  /* check for validity of pointers ...                               */
  /********************************************************************/
  if ( !pszTgt || !pszSrc )
  {
    fOK = FALSE;
    if ( fMsg )
    {
      sprintf( chDebug, "%s %d", __FILE__, __LINE__  );
      pParm[0] = chDebug;
      UtlError( ERROR_INTERNAL_PARM, MB_CANCEL, 1, pParm, EQF_ERROR );
    } /* endif */
  }
  else
  {
    if ( strlen(pszSrc) < usSize )
    {
      strcpy( pszTgt, pszSrc );
    }
    else
    {
      /******************************************************************/
      /* init return values and display error message if requested      */
      /******************************************************************/
      fOK = FALSE;
      *pszTgt = EOS;

      if ( fMsg )
      {
        UtlError( UTL_PARAM_TOO_LONG, MB_CANCEL, 1, &pszSrc, EQF_ERROR );
      } /* endif */
    } /* endif */
  } /* endif */

  return ( fOK );
} /* end of function UtlCopyParameter */

// function searching first character causing a conversion error and showing an
// approbriate error message
USHORT UtlFindAndShowConversionError
(
  PSZ    pszBuffer,
  ULONG  ulLen,
  ULONG  ulCP
)
{
  CHAR_W chBufOut[10];
  USHORT usRC = NO_ERROR;

  ULONG ulOutPut;

  // always use 932 when 943 is specified
  if ( ulCP == 943 ) ulCP = 932;

  while ( !usRC && ulLen )
  {
    ulOutPut = MultiByteToWideChar( ulCP, MB_ERR_INVALID_CHARS, pszBuffer, 1, chBufOut, 1 );

    // try conversion together with next byte, might be a DBCS character
    if ( !ulOutPut && ulLen )
    {
      ulOutPut = MultiByteToWideChar( ulCP, MB_ERR_INVALID_CHARS, pszBuffer, 2, chBufOut, 2 );
      if ( ulOutPut )
      {
        pszBuffer++;
        ulLen--;
      } /* endif */
    } /* endif */

    if ( !ulOutPut )
    {
      PSZ  pszParms[3];
      CHAR szRC[10], szCP[10], szCharacter[20];
      int iCh = (UCHAR)*pszBuffer;

      usRC = (USHORT)GetLastError();

      sprintf( szCharacter, "%d (hex %2.2X)", iCh, iCh );
      sprintf( szCP, "%lu", ulCP );
      sprintf( szRC, "%u", usRC );

      pszParms[0] = szCharacter;
      pszParms[1] = szCP;
      pszParms[2] = szRC;

      UtlError( ERROR_DATA_CONVERSION_EXINFO, MB_CANCEL, 3, pszParms, EQF_ERROR );
    }
    else
    {
      pszBuffer++;
      ulLen--;
    } /* endif */
  } /* endwhile */
  return( usRC );
} /* end of function UtlFindAndShowConversionError */

// function checking a given codepage value by doing a test conversion
BOOL UtlIsValidCP( ULONG ulCP )
{
  // do a test conversion using the supplied codepage
  CHAR_W c_W[10];
	LONG  lBytesLeft = 0L;
	LONG  lRc = 0;
	ULONG ulOutPut = 0;

  ulCP = ADJUSTCP( ulCP );

  ulOutPut = UtlDirectAnsi2UnicodeBuf( "abcdefgh", c_W, 8, ulCP, FALSE, &lRc, &lBytesLeft);

  return( (ulOutPut != 0) && (ulCP != 0) );
} /* end of function UtlIsValidCP */

/*! UtlInitUtils           Initialization of utilities       
	Description:       General utilities initialization routine                 
	Function Call:     BOOL UtlInitUtils( HAB hab );                            
	Parameters:        - hab is the application anchor block handle             
	Prerequesits:      none                                                     
	Returncode type:   BOOL                                                     
	Returncodes:       TRUE (always)                                            
	Note on usage:     This function may only be called once by EQFSTART!       
					Do never call this function from anythere else!          
	Function flow:     query date format from OS2.INI                           
					query date seperator from OS2.INI                        
					query time format from OS2.INI                           
					query time seperator from OS2.INI                        
					query codes for AM and PM from OS2.INI                   
					load or create event list                                
*/
BOOL UtlInitUtils( HAB hab )
{
   BOOL fOK = TRUE;

   CHAR chTemp[2];
   USHORT  usId = UtlGetTask();
   SHORT i;

   hab;

   // set version info in registry
   WriteStringToRegistry( "OpenTM2", "CurVersion", STR_DRIVER_LEVEL_NUMBER );

   //--- get profile information ---
   UtiVar[usId].usDateFormat = (USHORT)WinQueryProfileInt( hab,
                                            NATIONAL_SETTINGS,
                                            "iDate",
                                            MDY_DATE_FORMAT );
   WinQueryProfileString( hab,
                          NATIONAL_SETTINGS,
                          "sDate",
                          "/",
                          chTemp,
                          2 );
   UtiVar[usId].chDateSeperator = chTemp[0];
   UtiVar[usId].usTimeFormat = (USHORT)WinQueryProfileInt( hab,
                                            NATIONAL_SETTINGS,
                                            "iTime",
                                            S_12_HOURS_TIME_FORMAT );
   WinQueryProfileString( hab,
                          NATIONAL_SETTINGS,
                          "sTime",
                          ":",
                          chTemp,
                          2 );
   UtiVar[usId].chTimeSeperator = chTemp[0];
   WinQueryProfileString( hab,
                          NATIONAL_SETTINGS,
                          "s1159",
                          "AM",
                          UtiVar[usId].szTime1159,
                          sizeof(UtiVar[usId].szTime1159 ) );
   WinQueryProfileString( hab,
                          NATIONAL_SETTINGS,
                          "s2359",
                          "PM",
                          UtiVar[usId].szTime2359,
                          sizeof(UtiVar[usId].szTime2359 ) );
/**********************************************************************/
/* init lower and upper case in table 1.= COUNTRY 2. = CODE PAGE      */
/* 0 = DEFAULT                                                        */
/**********************************************************************/
   UtlLowUpInit( );

   /*******************************************************************/
   /* Clear drive type array                                          */
   /*******************************************************************/
   for ( i = 0 ; i < 26; i++ )
   {
     UtiVar[usId].sDriveType[i] = -1;
   } /* endfor */


   // Load or create event list
   {
     CHAR szLogFile[MAX_EQF_PATH];
     ULONG ulLength;

     // Setup event log file name
     GetStringFromRegistry( APPL_Name, KEY_Drive, szLogFile, sizeof( szLogFile ), "" );
     strcat( szLogFile, "\\OTM\\PROPERTY\\" );
     strcat( szLogFile, EVENTLOGFILENAME );

     // Load event log file
     pEventTable = NULL;
     if ( UtlAlloc( (PVOID *)&pEventTable, 0L,
                    (LONG)sizeof(EVENTTABLE), NOMSG ) )
     {
       ulLength = sizeof(EVENTTABLE);
       if ( !UtlLoadFileL( szLogFile, (PVOID *)&pEventTable, &ulLength,
                          FALSE, FALSE )
                          ||
             (memcmp( pEventTable->szMagicWord, EVENTMAGICWORD, 4 ) != 0) )
       {
         // event log not loaded or old format event table, create an empty one
         memset( pEventTable, 0, sizeof(EVENTTABLE) );
         memcpy( pEventTable->szMagicWord, EVENTMAGICWORD, 4 );
         pEventTable->sVersion = EVENTTABLEVERSION;
         pEventTable->sFirstEvent = -1; // number of first event in list
         pEventTable->sLastEvent  = -1; // number of last event in list
         pEventTable->sListSize = MAX_EVENT2_ENTRIES;
       } /* endif */
     } /* endif */
   }

   // Init Object Handler and Property handler for FUNCCALL runmode
   if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
   {
     fOK = ObjHandlerInitForBatch();
     if ( fOK ) fOK = PropHandlerInitForBatch();
   } /* endif */
   // Add exit procedure for this DLL
   return( fOK );
}

/*! UtlTerminateUtils   DeInitialization of utilities        
	Description:       General utilities deinitialization routine for this      
					instance of utilities                                    
	Function Call:     BOOL UtlTerminateUtils( void );                          
	Parameters:        none                                                     
	Prerequesits:      none                                                     
	Returncode type:   BOOL                                                     
	Returncodes:       TRUE (always)                                            
	Note on usage:     This function may only be called AFTER the utilities     
					have been initialized using the UtlInitUtils call        
	Function flow:     deregister utils                                         
*/
BOOL UtlTerminateUtils( VOID )
{
  BOOL fNonDDEMode = UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE;

   // Write event list
   if ( pEventTable)
   {
     CHAR szLogFile[MAX_EQF_PATH];
     GetStringFromRegistry( APPL_Name, KEY_Drive, szLogFile, sizeof( szLogFile ), "" );
     strcat( szLogFile, "\\OTM\\PROPERTY\\" );
     strcat( szLogFile, EVENTLOGFILENAME );


     // Write event log file
     UtlWriteFile( szLogFile, sizeof(EVENTTABLE), pEventTable, FALSE );

     // free event list memory
     UtlAlloc( (PVOID *)&pEventTable, 0L, 0L, NOMSG );
   }

   // Terminate Object Handler and Property handler for FUNCCALL runmode
   if ( fNonDDEMode  )
   {
     ObjHandlerTerminateForBatch();
     PropHandlerTerminateForBatch();
   } /* endif */

  {
    WORD   usTask;
    USHORT currTask;

          _asm
            {
              mov      ax, SS
              mov      usTask, ax
            }
    for ( currTask = 0; currTask < MAX_TASK ; ++currTask )
    {
      if ( UtiVar[currTask].usTask == usTask )
      {
        /**************************************************************/
        /* free it for later usage...                                 */
        /**************************************************************/
        SHORT     sSegTable, i;
        PSEGTABLE pSegTable;
        for (  i = QST_FIRST+1; i < QST_LAST; i++ )
        {
          if ( UtiVar[currTask].pszQueryArea[i] )
          {
           UtlAlloc( (PVOID *)&(UtiVar[currTask].pszQueryArea[i]), 0L, 0L, NOMSG );
          } /* endif */
        } /* endfor */

#ifndef _TQM
        // TAFreeAllTagTableSpace();   // free tagtable space of this task
#endif
        for (sSegTable = 0; sSegTable < MAX_MEM_TABLES; sSegTable++)
        {
          pSegTable = UtiVar[currTask].SegTable + sSegTable;
          if ( pSegTable->sMaxSel != SEG_TABLE_EMPTY )
          {
            for ( i=0; i<=pSegTable->sMaxSel; i++)
            {
              free( pSegTable->pSegment[i].pSel );
              pSegTable->pSegment[i].pSel = NULL;
            }
          }
          if ( pSegTable->pSegment )
          {
            free( pSegTable->pSegment );
            pSegTable->pSegment = NULL;
          }
        }
        pSegTable->sMaxSel = SEG_TABLE_EMPTY;
        memset( &UtiVar[currTask], 0, sizeof( UTIVARS ));

        break;
      } /* endif */
    } /* endfor */
  }

   if ( hFFSTFile != NULL )
   {
     fclose( hFFSTFile );
   } /* endif */

   return( TRUE );
};

// Generic check if TM or ITM is currently running
// If not create semaphore to disable another instance of us to try to start

BOOL UtlIsAlreadyRunning
(
	HANDLE * phTM_Sem
)
{
	BOOL fRunning = FALSE;

	char szExclusiveFile[128];
	char EqfSystemPropPath[MAX_EQF_PATH]; // global system properties path
	CHAR     szDrive[MAX_DRIVE];         // buffer for drive list
    CHAR     szLanDrive[MAX_DRIVE];      // buffer for drive list
    CHAR     szSysPath[MAX_EQF_PATH];    // global system path

	/********************************************************************/
	/* build name of directory where TM is installed (e.g. c:\eqf\win)  */
	/********************************************************************/
  {
    GetStringFromRegistry( APPL_Name, KEY_Drive, szDrive, sizeof( szDrive  ), "" );
    GetStringFromRegistry( APPL_Name, KEY_LanDrive, szLanDrive, sizeof( szLanDrive  ), "" );
    GetStringFromRegistry( APPL_Name, KEY_Path, szSysPath, sizeof( szSysPath), "" );
  }

	if ( !szLanDrive[0] )
	{
	  strcpy(szLanDrive, szDrive);
	} /* endif */
	sprintf( EqfSystemPropPath, "%s\\%s",  szLanDrive, szSysPath );


	strcpy( szExclusiveFile, EqfSystemPropPath );
	strcat( szExclusiveFile, BACKSLASH_STR );
    strcat( szExclusiveFile, WINDIR );
	strcat( szExclusiveFile, "\\TMSEM.DAT" );

	*phTM_Sem = CreateFile( szExclusiveFile, GENERIC_READ | GENERIC_WRITE,
	                             0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
	                             NULL );
	if ( *phTM_Sem == INVALID_HANDLE_VALUE )
	{
	  fRunning = TRUE;
	} /* endif */

    return fRunning;
}


// helper function for UtlLoadLastUsedStrings and UtlSaveLastUsedString 

static void UtlMakeLUFileName
(
  PSZ  pszName,                 // name of last used values file (w/o path and extention)
  PSZ  pszBuffer                // buffer receiving fully qualifie 
)
{
  UtlMakeEQFPath( pszBuffer, NULC, QST_PROPDIR, NULL );
  strcat( pszBuffer, "\\" );
  strcat( pszBuffer, pszName );
  strcat( pszBuffer, ".LUV" );
  return;
}

// Utility functions for last used value handlung of comboboxes

void UtlLoadLastUsedStrings
(
  HWND        hwndDlg,                 // handle of dialog containing the combobox
  int         iID,                     // ID of the combobox
  char        *pszName                 // name of last used values file (w/o path and extention)

)
{
  PSZ_W       pszBuffer = NULL;
 
  int iMaxStringLen = 512;
  FILE *hf = NULL;

  // allocate buffer for strings
  pszBuffer = (PSZ_W)malloc( iMaxStringLen * sizeof(CHAR_W) );  
  if ( pszBuffer == NULL ) return;

  // delete all contents in the list but preserver currently active value
  QUERYTEXTHWNDLEN( GetDlgItem( hwndDlg, iID ), ((PSZ)pszBuffer), iMaxStringLen );
  SendDlgItemMessage( hwndDlg, iID, CB_RESETCONTENT, 0, 0L );
  SETTEXT( hwndDlg, iID, ((PSZ)pszBuffer) );

  // setup file name and the open file for input
  UtlMakeLUFileName( pszName, (PSZ)pszBuffer );
  hf = fopen( (PSZ)pszBuffer, "rb" );
  if ( hf == NULL ) { free( pszBuffer ); return; }

  // skip any unicode prefix
  memset( pszBuffer, 0, iMaxStringLen * sizeof(CHAR_W) );
  fread( pszBuffer, 1, 2, hf );
  if ( memcmp( pszBuffer, UNICODEFILEPREFIX, 2 ) != 0 )
  {
    // undo read
    fseek( hf, 0, SEEK_SET );
  } /* endif */     


  while ( !feof( hf ) )
  {
    memset( pszBuffer, 0, iMaxStringLen * sizeof(CHAR_W) );
    fgetws( pszBuffer, iMaxStringLen, hf );

    if ( *pszBuffer != 0 )
    {
      // remove any trailing LF and CR
      PSZ_W pszEndPos = pszBuffer + (wcslen(pszBuffer) - 1);
      if ( (pszEndPos >= pszBuffer) && (*pszEndPos == L'\n') ) *pszEndPos-- = 0;
      if ( (pszEndPos >= pszBuffer) && (*pszEndPos == L'\r') ) *pszEndPos-- = 0;

      // add non-empty lines to combobox
      if ( *pszBuffer != 0 )
      {
        SendDlgItemMessageW( hwndDlg, iID, CB_INSERTSTRING, (WPARAM) -1, (LPARAM)pszBuffer );
      } /* endif */       
    } /* endif */       
  } /* endwhile */     

  if (hf != NULL) fclose ( hf );
  free( pszBuffer );

  return;
}

void UtlSaveLastUsedString
(
  HWND        hwndDlg,                 // handle of dialog containing the combobox
  int         iID,                     // ID of the combobox
  char        *pszName,                // name of last used values file (w/o path and extention)
  int         iMaxNumOfStrings         // maximum number of strings to save
)
{
  PSZ_W       pszBuffer = NULL;
  PSZ_W       pszCurPos = NULL;
  int         iElements = 0;
  int         i = 0;
  int iMaxStringLen = 512;
  FILE *hf = NULL;

  // allocate buffer for the array of strings
  pszBuffer = (PSZ_W)malloc( iMaxStringLen * sizeof(CHAR_W) * (iMaxNumOfStrings + 1));  
  if ( pszBuffer == NULL ) return;

  // setup file name and the open file for output
  UtlMakeLUFileName( pszName, (PSZ)pszBuffer );
  hf = fopen( (char *)pszBuffer, "wb" );
  if ( hf == NULL ) { free( pszBuffer ); return; }
  fwrite( (void *)UNICODEFILEPREFIX, 1, 2, hf );

  // get strings from combobox
  GetDlgItemTextW( hwndDlg, iID, pszBuffer, iMaxStringLen );
  iElements = SendDlgItemMessage( hwndDlg, iID, CB_GETCOUNT, 0, 0L );
  pszCurPos = pszBuffer + iMaxStringLen;
  for ( i = 0; i < iElements; i++ )
  {
    SendDlgItemMessageW( hwndDlg, iID, CB_GETLBTEXT, i, (LPARAM)pszCurPos );
    pszCurPos += iMaxStringLen;
  } /* endfor */     

  // write unique strings to output file
  i = 0;
  pszCurPos = pszBuffer;
  while ( (i < iMaxNumOfStrings) && (i <= iElements) )
  {
    // check if current string is unique
    BOOL fIgnoreString = FALSE;
    int j = 0;
    PSZ_W pszTest = pszBuffer;
    while ( j < i )
    {
      if ( wcscmp( pszCurPos, pszTest ) == 0 )
      {
        fIgnoreString = TRUE;
        break;
      } /* endif */         
      j++;
      pszTest += iMaxStringLen;
    } /* endwhile */       

    // write to output file
    if ( !fIgnoreString )
    {
      fputws( pszCurPos, hf );
      fputws( L"\r\n", hf );
    } /* endif */       

    // next string
    pszCurPos += iMaxStringLen;
    i++;
  } /* endwhile */     

  // cleanup
  fclose( hf );
  free( pszBuffer ); 
  return; 
} /* end of function UtlSaveLastUsedString */

// add a menu item to a specific OpenTM2 menu bar
BOOL UtlAddMenuItemToSingleMenu( HMENU hMainMenu, const char *pszMenuName, const char *pszMenuItem, int iMenuItemID, BOOL isSubMenu )
{
  BOOL fOk = FALSE;

  HMENU hSubMenu = UtlFindSubMenu( hMainMenu, pszMenuName );
  
  if ( hSubMenu == NULL ) return( false );

//  fOk = AppendMenu( hSubMenu, MF_STRING, iMenuItemID, pszMenuItem ); 

  MENUITEMINFO mii;

  memset( &mii, 0, sizeof(MENUITEMINFO));
  mii.cbSize = sizeof(MENUITEMINFO);
  mii.fMask = MIIM_STRING | MIIM_ID;
  if ( isSubMenu ) mii.fMask = mii.fMask | MIIM_SUBMENU;
  mii.wID = iMenuItemID;
  if ( isSubMenu ) mii.hSubMenu = CreateMenu();
  mii.dwTypeData = (LPSTR)pszMenuItem;
  mii.cch = strlen(pszMenuItem);
  fOk = InsertMenuItem( hSubMenu, GetMenuItemCount( hSubMenu ), FALSE, &mii );

  if ( fOk ) DrawMenuBar( (HWND)UtlQueryULong( QL_TWBFRAME) );


  return( fOk );
}

// add a menu item to the OpenTM2 menu(s)
BOOL UtlAddMenuItem( const char *pszMenuName, const char *pszMenuItem, int iMenuItemID, BOOL isSubMenu )
{
  BOOL fOk = FALSE;

  HMENU hMainMenu1 = (HMENU)UtlQueryULong( QL_TREEVIEWMENU );
  if ( hMainMenu1 != NULL ) fOk = fOk | UtlAddMenuItemToSingleMenu( hMainMenu1, pszMenuName, pszMenuItem, iMenuItemID, isSubMenu);

  HMENU hMainMenu2 = (HMENU)UtlQueryULong( QL_TWBMENU );
  if ( (hMainMenu2 != NULL) && (hMainMenu2 != hMainMenu1)  ) fOk = fOk | UtlAddMenuItemToSingleMenu( hMainMenu2, pszMenuName, pszMenuItem, iMenuItemID, isSubMenu );

  HMENU hMainMenu3 = GetMenu( (HWND)UtlQueryULong( QL_TWBFRAME ) );
  if ( (hMainMenu3 != NULL) && (hMainMenu3 != hMainMenu1) && (hMainMenu3 != hMainMenu2) ) fOk = fOk | UtlAddMenuItemToSingleMenu( hMainMenu3, pszMenuName, pszMenuItem, iMenuItemID, isSubMenu );

  return( fOk );
}

// remove a menu item from a specific OpenTM2 menu bar
BOOL UtlRemovedMenuItemFromSingleMenu( HMENU hMainMenu, const char *pszMenuName, int iMenuItemID )
{
  BOOL fOk = FALSE;

  HMENU hSubMenu = UtlFindSubMenu( hMainMenu, pszMenuName );
  
  if ( hSubMenu == NULL ) return( false );

  fOk = DeleteMenu( hSubMenu, iMenuItemID, MF_BYCOMMAND ); 

  if ( fOk ) DrawMenuBar( (HWND)UtlQueryULong( QL_TWBFRAME) );

  return( fOk );
}

// remove a menu item from the OpenTM2 menu(s)
BOOL UtlDeleteMenuItem( const char *pszMenuName, int iMenuItemID )
{
  BOOL fOk = FALSE;

  HMENU hMainMenu1 = (HMENU)UtlQueryULong( QL_TREEVIEWMENU );
  if ( hMainMenu1 != NULL ) fOk = fOk | UtlRemovedMenuItemFromSingleMenu( hMainMenu1, pszMenuName, iMenuItemID );

  HMENU hMainMenu2 = (HMENU)UtlQueryULong( QL_TWBMENU );
  if ( (hMainMenu2 != NULL) && (hMainMenu2 != hMainMenu1)  ) fOk = fOk | UtlRemovedMenuItemFromSingleMenu( hMainMenu2, pszMenuName, iMenuItemID );

  HMENU hMainMenu3 = GetMenu( (HWND)UtlQueryULong( QL_TWBFRAME ) );
  if ( (hMainMenu3 != NULL) && (hMainMenu3 != hMainMenu1) && (hMainMenu3 != hMainMenu2) ) fOk = fOk | UtlRemovedMenuItemFromSingleMenu( hMainMenu3, pszMenuName, iMenuItemID );

  return( fOk );
}

/*! \brief search for a sub menu with the givem name (used recursively) */
#define MENU_NAME_LENGTH 1014
HMENU UtlFindSubMenu( HMENU hMenu, const char *pszSubMenu )
{
  HMENU hSubMenu = NULL;
  MENUITEMINFO MenuInfo;
  char *pszMenuName = NULL;

  if ( (hMenu == NULL) || (pszSubMenu == NULL) ) return( NULL );

  pszMenuName = (char *)malloc( MENU_NAME_LENGTH );
  if ( pszMenuName == NULL ) return( NULL );


  int iItems = GetMenuItemCount( hMenu );

  int i = 0;
  while ( (i < iItems) && (hSubMenu == NULL) )
  {
    memset( &MenuInfo, 0, sizeof(MenuInfo) );
    MenuInfo.cbSize = sizeof(MENUITEMINFO);
    MenuInfo.fMask = MIIM_SUBMENU | MIIM_STRING;
    MenuInfo.cch = MENU_NAME_LENGTH;
    MenuInfo.dwTypeData = pszMenuName;

    if ( GetMenuItemInfo( hMenu, i, TRUE, &MenuInfo ) )
    {
      if ( MenuInfo.hSubMenu != NULL )  // we are oly interested in sub menus
      {
        // get rid of accelerator marks
        char *pszSource = pszMenuName;
        char *pszTarget = pszMenuName;
        while( *pszSource )
        {
          if ( *pszSource != '&' )
          {
            *pszTarget++ = *pszSource++;
          }
          else
          {
            pszSource++;
          }
        }
        *pszTarget = '\0';

        if ( stricmp( pszSubMenu, pszMenuName ) == 0 )
        {
          free( pszMenuName );
          return( MenuInfo.hSubMenu );
        }

        hSubMenu = UtlFindSubMenu( MenuInfo.hSubMenu, pszSubMenu );
      }
    }
    else
    {
      int iRC = GetLastError();
      iRC += 1;
    }
    i++;
  }
  return( hSubMenu );
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/***                                                                ***/
/***                 Activate-a-MDI-child-window function           ***/
/***                                                                ***/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
BOOL ActivateMDIChild
(
  HWND                 hwndChild       // handle of window being activated
)
{
  /********************************************************************/
  /* Under Windows activate the MDI child window and, if it is        */
  /* minimized, restore it to its normal size                         */
  /********************************************************************/
  SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ), WM_MDIACTIVATE,
               MP1FROMHWND(hwndChild), 0L );
  if ( IsIconic(hwndChild) )
  {
    SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ), WM_MDIRESTORE,
                 MP1FROMHWND(hwndChild), 0L );
  } /* endif */
  return( TRUE );
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/***                                                                ***/
/***                 Create-a-process-window function               ***/
/***                                                                ***/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
BOOL CreateProcessWindow
(
  PSZ                  pszObjName,     // ptr to object name for process window
  PFN_PROCESSCALLBACK  pfnCallBack,    // callback function for process window
  PVOID                pvUserData      // ptr to user data (is passed to callback
                                       // function in mp2 of WM_CREATE message)
)
{
  return( CreateProcessWindow2( pszObjName, pfnCallBack, pvUserData, TRUE ) );
} /* end of function CreateProcessWindow */

BOOL CreateProcessWindow2
(
  PSZ                  pszObjName,     // ptr to object name for process window
  PFN_PROCESSCALLBACK  pfnCallBack,    // callback function for process window
  PVOID                pvUserData,     // ptr to user data (is passed to callback
                                       // function in mp2 of WM_CREATE message)
  BOOL                 fVisible        // create-a-visible-process-window flag
)
{
  HWND     hframe, hclient;            // handle of created windows
  BOOL     fOK;                        // function return code
  PROCESSCREATEPARMS CreateParms;      // process creation parameter

  CreateParms.pvUserData  = pvUserData;
  CreateParms.pfnCallBack = pfnCallBack;
  CreateParms.fVisible    = fVisible;

  {
    /*********************************************************/
    /* to avoid flickering during the creation of the MDI    */
    /* Child window we have to position it outside of the    */
    /* screen - we always reposition it to the last stored   */
    /* positions via a WinSetWindowPos function after init.  */
    /*********************************************************/
    HWND  hwndMDIClient;
    MDICREATESTRUCT mdicreate;

    mdicreate.hOwner  = (HAB)UtlQueryULong( QL_HAB );
    mdicreate.szClass = GENERICPROCESS;
    mdicreate.szTitle = pszObjName;
    mdicreate.x       = -100;
    mdicreate.y       = -100;
    mdicreate.cx      = 1;
    mdicreate.cy      = 1;
    mdicreate.style   = WS_CLIPCHILDREN;
    mdicreate.lParam  = MP2FROMP(&CreateParms);

    hwndMDIClient = (HWND)UtlQueryULong( QL_TWBCLIENT );

    hframe = hclient =
    (HWND)SendMessage( hwndMDIClient,
                       WM_MDICREATE, 0,
                       MP2FROMP((LPMDICREATESTRUCT)&mdicreate) );
  }
  if( hframe)
  {
    UtlDispatch();
    WinPostMsg( hclient, WM_EQF_INITIALIZE, NULL, NULL);
    fOK = TRUE;
  }
  else
  {
    fOK = FALSE;
  } /* endif */
  return( fOK );
} /* end of function CreateProcessWindow */

/**********************************************************************/
/* Procedure to get or refresh the commarea pointer for a generic     */
/* process window (e.g. adfter a call to UtlDispatch)                 */
/*                                                                    */
/* If NULL is returned either the list window has been destroyed or   */
/* it's data areas have been freed                                    */
/**********************************************************************/
PVOID AccessGenProcCommArea( HWND hwnd )
{
  PPROCESSCOMMAREA   pCommArea = NULL;
  PGENPROCESSINSTIDA pIda;

  pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
  if ( pIda != NULL )
  {
    pCommArea = &(pIda->CommArea);
  } /* endif */
  return( pCommArea );
} /* end of function AccessGenProcCommArea */



VOID EqfDisplayContextHelp( HWND hItemHandle, PHELPSUBTABLE pHlpTable )
{

  /********************************************************************/
  /* copy our help subtable, whose entries are SHORTs into a table as */
  /* requested  by Windows with long entries                          */
  /********************************************************************/
  LONG aItem[500];
  PLONG plItem = &aItem[0];
  PSHORT pS = (PSHORT) pHlpTable;
  pS++;                                // skip index

  memset( &aItem[0], 0, sizeof( aItem ));
  while ( *pS )
  {
    *plItem = (LONG) *pS;
    plItem++; pS++;
    *plItem = (LONG) *pS;
    plItem++; pS++;

  } /* endwhile */

  CHAR EqfSystemHlpFile[MAX_EQF_PATH];  
  UtlQueryString( QST_HLPFILE, EqfSystemHlpFile, sizeof(EqfSystemHlpFile) );

  WinHelp( hItemHandle, EqfSystemHlpFile, HELP_WM_HELP, (LONG) &aItem[0]);

}

static COLORREF acrCustomColors[16]; // array of custom colors 


// instance data area for set colors dialog
typedef struct _SETCOLORSIDA
{
  int  iCurElement;                  // index of currently active text element
  COLORREF cForeground;              // currently selected foreground color
  COLORREF cBackground;              // currently selected background color
  COLORCHOOSERDATA InOutData;        // local copy of caller's color chooser data area
} SETCOLORSIDA, *PSETCOLORSIDA;


INT_PTR CALLBACK UTLCOLORCHOOSERDLG
(
  HWND   hwnd,                        // handle of window
  WINMSG msg,                         // type of message
  WPARAM mp1,                         // first message parameter
  LPARAM mp2                          // second message parameter
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  MRESULT              mResult = FALSE;// dlg procedure return value
  PSETCOLORSIDA        pIDA;           // dialog IDA

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIDA = ACCESSDLGIDA( hwnd, PSETCOLORSIDA);
      HANDLEQUERYID( pIDA->InOutData.sID, mp2 );
      break;

    case WM_INITDLG :
      {
        // save the pointer to the IDA in the dialog reserved memory
        pIDA = (PSETCOLORSIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pIDA );

        // fill type of text listbox
        int i = 0;
        while( pIDA->InOutData.ColorSetting[i].szElement[0] != EOS )
        {
          SHORT sItem = INSERTITEMEND( hwnd, ID_SETCOLOR_ITEM_LB, pIDA->InOutData.ColorSetting[i].szElement );
          SETITEMHANDLE( hwnd, ID_SETCOLOR_ITEM_LB, sItem, i );
          i++;
        } /* endwhile */
        SELECTITEM( hwnd, ID_SETCOLOR_ITEM_LB, 0 );

        // adjust titlebar text
        if ( pIDA->InOutData.szTitle [0] != EOS ) SETTEXTHWND(hwnd, pIDA->InOutData.szTitle );

        // Keep dialog within TWB
        {
          SWP  swpDlg, swpTWB;
          WinQueryWindowPos( hwnd, &swpDlg );

          WinQueryWindowPos( (HWND)UtlQueryULong( QL_TWBCLIENT ), &swpTWB );
          if ( (swpDlg.x > 0) && ((swpDlg.x + swpDlg.cx) < swpTWB.cx) )
          {
            swpDlg.x = (swpTWB.cx - swpDlg.cx) / 2;
          } /* endif */
          if ( (swpDlg.y > 0) && ((swpDlg.y + swpDlg.cy) < swpTWB.cy) )
          {
            swpDlg.y = (swpTWB.cy - swpDlg.cy) / 2;
          } /* endif */
          WinSetWindowPos( hwnd, HWND_TOP, swpDlg.x, swpDlg.y, swpDlg.cx, swpDlg.cy, EQF_SWP_MOVE | EQF_SWP_SHOW | EQF_SWP_ACTIVATE );
        }

        // leave the focus where we put it
        mResult = MRFROMSHORT(DIALOGINITRETURN(TRUE));
      }
      break;

    case WM_COMMAND :
      {
        pIDA = ACCESSDLGIDA( hwnd, PSETCOLORSIDA);

        switch ( WMCOMMANDID( mp1, mp2 ) )
        {
          case ID_SETCOLOR_BACKGROUND_PB:
            {
              CHOOSECOLOR Colors;
              memset( &Colors, 0, sizeof(Colors) );
              Colors.lStructSize = sizeof(Colors);
              Colors.hwndOwner = hwnd;
              Colors.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
              Colors.lpCustColors = acrCustomColors;
              Colors.rgbResult = pIDA->cBackground;

              if ( ChooseColor( &Colors ) )
              {
                pIDA->cBackground = Colors.rgbResult;
                pIDA->InOutData.ColorSetting[pIDA->iCurElement].cBackground = Colors.rgbResult;
                InvalidateRect( GetDlgItem( hwnd, ID_SETCOLOR_BACKGROUND_STATIC ), NULL, TRUE );
                InvalidateRect( GetDlgItem( hwnd, ID_SETCOLOR_SAMPLE_STATIC ), NULL, TRUE );
              } /* endif */
            } 
            break;

          case ID_SETCOLOR_FOREGROUND_PB:
            {
              CHOOSECOLOR Colors;
              memset( &Colors, 0, sizeof(Colors) );
              Colors.lStructSize = sizeof(Colors);
              Colors.hwndOwner = hwnd;
              Colors.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
              Colors.lpCustColors = acrCustomColors;
              Colors.rgbResult = pIDA->cForeground;

              if ( ChooseColor( &Colors ) )
              {
                pIDA->cForeground = Colors.rgbResult;
                pIDA->InOutData.ColorSetting[pIDA->iCurElement].cForeground = Colors.rgbResult;
                InvalidateRect( GetDlgItem( hwnd, ID_SETCOLOR_FOREGROUND_STATIC ), NULL, TRUE );
                InvalidateRect( GetDlgItem( hwnd, ID_SETCOLOR_SAMPLE_STATIC ), NULL, TRUE );
              } /* endif */
            } 
            break;

          case ID_SETCOLOR_ITEM_LB:
            if ( WMCOMMANDCMD( mp1, mp2 ) == LN_SELECT )
            {
              SHORT sItem = QUERYSELECTION( hwnd, ID_SETCOLOR_ITEM_LB );
              if ( sItem != LIT_NONE )
              {
                SHORT sIndex = (SHORT)QUERYITEMHANDLE( hwnd, ID_SETCOLOR_ITEM_LB, sItem );
                pIDA->cForeground = pIDA->InOutData.ColorSetting[sIndex].cForeground;
                pIDA->cBackground = pIDA->InOutData.ColorSetting[sIndex].cBackground;
                pIDA->iCurElement = sIndex;
                InvalidateRect( GetDlgItem( hwnd, ID_SETCOLOR_FOREGROUND_STATIC ), NULL, TRUE );
                InvalidateRect( GetDlgItem( hwnd, ID_SETCOLOR_BACKGROUND_STATIC ), NULL, TRUE );
                InvalidateRect( GetDlgItem( hwnd, ID_SETCOLOR_SAMPLE_STATIC ), NULL, TRUE );
              } /* endif */
            } /* endif */
            break;

          case ID_SETCOLOR_CHANGE_PB :
            WinDismissDlg( hwnd, TRUE );
            break;

          case ID_SETCOLOR_DEFAULT_PB:
            {
              // replace current colors with their default values
              int i = 0;
              while( pIDA->InOutData.ColorSetting[i].szElement[0] != EOS )
              {
                pIDA->InOutData.ColorSetting[i].cForeground = pIDA->InOutData.ColorSetting[i].cDefaultForeground; 
                pIDA->InOutData.ColorSetting[i].cBackground = pIDA->InOutData.ColorSetting[i].cDefaultBackground;
                i++;
              } /* endwhile */

              // force a refresh by posting a LN_SELECT message for the text item listbox
              PostMessage( hwnd, WM_COMMAND, MAKEWPARAM( ID_SETCOLOR_ITEM_LB, LN_SELECT ), (LPARAM)GetDlgItem( hwnd, ID_SETCOLOR_ITEM_LB ) );
            }
            break;

          case ID_SETCOLOR_CANCEL_PB :
          case DID_CANCEL :
            WinDismissDlg( hwnd, FALSE );
            break;
        } 
      }
      break;

    case WM_CTLCOLORSTATIC:
    {
      DWORD CtrlID = GetDlgCtrlID((HWND)mp2);
      if ( CtrlID == ID_SETCOLOR_FOREGROUND_STATIC )
      {
          pIDA = ACCESSDLGIDA( hwnd, PSETCOLORSIDA );
          HDC hdcStatic = (HDC)mp1;
          SetTextColor(hdcStatic, RGB(0,0,0));
          SetBkColor(hdcStatic, pIDA->cForeground );
        return (INT_PTR)CreateSolidBrush(pIDA->cForeground);
      }
      else if ( CtrlID == ID_SETCOLOR_BACKGROUND_STATIC )
      {
          pIDA = ACCESSDLGIDA( hwnd, PSETCOLORSIDA );
          HDC hdcStatic = (HDC)mp1;
          SetTextColor(hdcStatic, RGB(0,0,0));
          SetBkColor(hdcStatic, pIDA->cBackground );
        return (INT_PTR)CreateSolidBrush(pIDA->cBackground);
      }
      else if ( CtrlID == ID_SETCOLOR_SAMPLE_STATIC )
      {
          pIDA = ACCESSDLGIDA( hwnd, PSETCOLORSIDA );
          HDC hdcStatic = (HDC)mp1;
          SetTextColor(hdcStatic, pIDA->cForeground);
          SetBkColor(hdcStatic, pIDA->cBackground );
        return (INT_PTR)CreateSolidBrush(pIDA->cBackground);
      } /* endif */
    }
    break;


    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );

}

// let the user select the color settings for a group of items
USHORT UtlColorChooserDlg
(
  PCOLORCHOOSERDATA pColorChooserData  // pointer to caller's color chooser data structure
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PSETCOLORSIDA pIDA = NULL;           // color chooser dialog IDA

  // check input parameters
  if ( pColorChooserData == NULL )
  {
    usRC = ERROR_INVALID_DATA;
  }

  // allocate dialog IDA
  if ( usRC == NO_ERROR )
  {
    if ( !UtlAlloc( (PVOID *)&pIDA, 0L, sizeof(SETCOLORSIDA), ERROR_STORAGE ) )
    {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
    }
  }

  // pop-up dialog
  if ( usRC == NO_ERROR )
  {
    BOOL fOK;
	  HMODULE hResMod;

    memcpy( &(pIDA->InOutData), pColorChooserData, sizeof(pIDA->InOutData) );

  	hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    DIALOGBOX( QUERYACTIVEWINDOW(), UTLCOLORCHOOSERDLG, hResMod, ID_SETCOLOR_DLG, pIDA, fOK );

    if ( fOK )
    {
      memcpy( pColorChooserData, &(pIDA->InOutData), sizeof(pIDA->InOutData) );
    }
    else
    {
      usRC = (USHORT)-1;   // user cancelled dialog
    } /* endif */
  }

  if ( pIDA != NULL ) UtlAlloc( (PVOID *)&pIDA, 0L, 0L, NOMSG );
  
  // return to caller
  return( usRC );

}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TAAdjustWhiteSpace                               |
//+----------------------------------------------------------------------------+
//|Description:       Adjust the whitespace of a segment; i.e. ensure          |
//|                   that the target segment has the same leading and or      |
//|                   trailing whitespace as the source segment                |
//   GQ 2004/10/21    Do not touch whitespaces adjusted by the adjust leading
//                    whitespaces part of the function when processing trailing
//                    whitespaces.
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ     pszSourceSeg  ptr to source segment data         |
//|                   PSZ     pszTargetSeg  ptr to target segment data         |
//|                   PSZ     *ppszNewTargetSeg ptr to ptr for new target seg. |
//|                                 if the ptr is NULL a new buffer            |
//|                                 is allocated (in the length of the         |
//|                                 new target segment)                        |
//|                                 if the ptr is not NULL it must             |
//|                                 point to a buffer large enough to          |
//|                                 contain a segment (MAX_SEGMENT_SIZE).      |
//|                   BOOL    fLeadingWS process leading whitespace flag       |
//|                   BOOL    fTrailingWS process trailing whitespace flag     |
//|                   PBOOL   pfChanged TRUE if target segment has been        |
//|                                 changed                                    |
//|                                 FALSE if no change is required (in this    |
//|                                 case no new target segment is allocated!)  |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL    FALSE  in case of errors (=UtlAlloc failed)      |
//|                           TRUE   everything O.K.                           |
//+----------------------------------------------------------------------------+


#define TAISWHITESPACE( c ) \
    ( (c == LF) || (c == CR) || (c == SPACE) || (c == 0x09) )

static BOOL TACheckForWS
(
	CHAR_W c,
	PSZ_W  pWSList
)
{
	BOOL fIsWS = FALSE;

	fIsWS = TAISWHITESPACE( c ) ;

	if (pWSList && !fIsWS)
	{
		while (*pWSList && !fIsWS)
		{
			if (c == *pWSList)
			{
				fIsWS = TRUE;
		    }
		    else
		    {
				pWSList++;
		    }
	    } /* endwhile */
    } /* endif */


	return(fIsWS);
}


BOOL TAAdjustWhiteSpace
(
  PSZ_W         pszSourceSeg,            // ptr to source segment data
  PSZ_W         pszTargetSeg,            // ptr to target segment
  PSZ_W         *ppszNewTargetSeg,       // ptr to ptr of output buffer
  BOOL        fLeadingWS,              // process leading whitespace flag
  BOOL        fTrailingWS,             // process trailing whitespace flag
  PBOOL       pfChanged,                // TRUE if target has been changed
  PSZ_W       pWSList
)
{
  BOOL        fOK = TRUE;              // function return code
  PSZ_W         pszTargetWhiteSpace=NULL;// points to last whitespace in target
  PSZ_W         pszSourceWhiteSpace=NULL;// points to last whitespace in source
  PSZ_W         pszTempSegW = NULL;
  int         iAdjustLen = 0;          // length of adjusted leading whitespace

  // initialize callers change flag
  *pfChanged = FALSE;

  // allocate buffer for temporary segment
  fOK = UtlAlloc( (PVOID *)&pszTempSegW, 0L, (2L*(LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W)),
                           ERROR_STORAGE );

  // handle leading whitespace
  if ( fOK && fLeadingWS )
  {
    CHAR_W chFirstNonWSSource;
    CHAR_W chFirstNonWSTarget;
    PSZ_W pszNonWSSource = pszSourceSeg;
    PSZ_W pszNonWSTarget = pszTargetSeg;


    // position to first non-whitespace character in source and target
    while ( TACheckForWS(*pszNonWSSource, pWSList) ) pszNonWSSource++;
    while ( TACheckForWS(*pszNonWSTarget, pWSList) ) pszNonWSTarget++;

    // temporarly terminate segment at found locations
    chFirstNonWSSource = *pszNonWSSource;
    *pszNonWSSource = EOS;
    chFirstNonWSTarget = *pszNonWSTarget;
    *pszNonWSTarget = EOS;

    // compare leading whitespace and adjust if necessary
    if ( UTF16strcmp( pszSourceSeg, pszTargetSeg ) != 0 )
    {
      // copy whitespace of source segment to our temp seg buffer
      UTF16strcpy( pszTempSegW, pszSourceSeg );

      iAdjustLen = UTF16strlenCHAR( pszTempSegW );

      // restore end of leading whitespace
      *pszNonWSSource = chFirstNonWSSource;
      *pszNonWSTarget = chFirstNonWSTarget;

      // copy data of target segment to our temp seg buffer
      UTF16strcat( pszTempSegW, pszNonWSTarget );

      // set caller's changed flag
      *pfChanged = TRUE;
    }
    else
    {
      iAdjustLen = UTF16strlenCHAR( pszTargetSeg );

      // restore end of leading whitespace
      *pszNonWSSource = chFirstNonWSSource;
      *pszNonWSTarget = chFirstNonWSTarget;
    } /* endif */
  } /* endif */

  // if nothing has been changed copy complete target segment to
  // our temp seg buffer
  if ( fOK && !*pfChanged )
  {
    UTF16strcpy( pszTempSegW, pszTargetSeg );
  } /* endif */

  // locate trailing whitespace in target segment
  if ( fOK && fTrailingWS )
  {
    int    iLen;

    iLen = UTF16strlenCHAR(pszTempSegW);
    if ( iLen > iAdjustLen )
    {
      pszTargetWhiteSpace = pszTempSegW + iLen - 1;
      while ( (iLen > iAdjustLen) && TACheckForWS(*pszTargetWhiteSpace, pWSList) )
      {
        pszTargetWhiteSpace--;
        iLen--;
      } /* endwhile */
      pszTargetWhiteSpace++;             // point to first byte of whitespace
    } /* endif */
  } /* endif */

  // locate trailing whitespace in source segment
  if ( fOK && fTrailingWS )
  {
    int      iLen;

    iLen = UTF16strlenCHAR(pszSourceSeg);
    if ( iLen > iAdjustLen )
    {
      pszSourceWhiteSpace = pszSourceSeg + iLen - 1;
      while ( (iLen > iAdjustLen) && TACheckForWS(*pszSourceWhiteSpace, pWSList) )
      {
        pszSourceWhiteSpace--;
        iLen--;
      } /* endwhile */
      pszSourceWhiteSpace++;             // point to first byte of whitespace
    } /* endif */
  } /* endif */

  // setup new target if trailing whitespace differs
  if ( fOK && fTrailingWS && pszSourceWhiteSpace && pszTargetWhiteSpace &&
       UTF16strcmp( pszSourceWhiteSpace, pszTargetWhiteSpace ) != 0 )
  {
    // build new target segment
    *pszTargetWhiteSpace = EOS;
    UTF16strcat( pszTempSegW, pszSourceWhiteSpace );

    // set caller's changed flag
    *pfChanged = TRUE;
  } /* endif */

  // return modified temp segment to caller
  if ( fOK && *pfChanged )
  {
    LONG lNewLen = UTF16strlenCHAR(pszTempSegW) + 1;

    // if changed segment exceeds maximum segment size...
    if ( lNewLen > MAX_SEGMENT_SIZE )
    {
      // .. ignore the changes
      *pfChanged = FALSE;
    }
    else
    {
      if ( *ppszNewTargetSeg == NULL )
      {
        LONG lNewLen = UTF16strlenCHAR(pszTempSegW) + 1;
        // GQ: allocate segment at least large enough for the :NONE. tag!
        fOK = UtlAlloc( (PVOID *)ppszNewTargetSeg, 0L,
                        max( lNewLen * sizeof(CHAR_W), (sizeof(EMPTY_TAG)+4)), ERROR_STORAGE );
      } /* endif */

      if ( fOK )
      {
        UTF16strcpy( *ppszNewTargetSeg, pszTempSegW );
      } /* endif */
    } /* endif */
  } /* endif */

  // cleanup
  if ( pszTempSegW ) UtlAlloc( (PVOID *)&pszTempSegW, 0L, 0L, NOMSG );

  return( fOK );
} /* end of function TAAdjustWhiteSpace */

//+----------------------------------------------------------------------------+
// Function name:  UtlMakeFNameAndPath                                          
//+----------------------------------------------------------------------------+
// Description: Parses a string and returns the drive and directories           
//              in second parameter and filename plus extension in the          
//              third parameter                                                 
//+----------------------------------------------------------------------------+
// Parameters: PSZ file path                 IN                                 
//             PSZ drive and directories     OUT                                
//             PSZ filename plus extension   OUT                                
//+----------------------------------------------------------------------------+
// Returncode type:  VOID                                                       
//+----------------------------------------------------------------------------+
// Returncodes:  -                                                              
//+----------------------------------------------------------------------------+
// Prerequesites: none                                                          
//+----------------------------------------------------------------------------+
// Side effects: none                                                           
//+----------------------------------------------------------------------------+
// Function call:  UtlMakeFNameAndPath( szFullPath, szPath, szPatternName );    
//+----------------------------------------------------------------------------+
// Samples:                                                                     
//   UtlMakeFNameAndPath is called as follows:                                  
//                                                                              
//   //get path and pattern from pControlsIda->szPathContent                    
//   UtlMakeFNameAndPath( pControlsIda->szPathContent,   //input string         
//                        pControlsIda->szDummy,         //path without fname   
//                        pControlsIda->szPatternName);  //fname + ext          
//+----------------------------------------------------------------------------+
// Function flow:                                                               
//   separate out path from fully qualified file path                           
//   if there is path information then                                          
//     add backslash to path                                                    
//     save path and filename information                                       
//   else                                                                       
//     set path string to NULL                                                  
//     save filename information (endif)                                        
//+----------------------------------------------------------------------------+

VOID UtlMakeFNameAndPath( PSZ pszFullPath,    //in- fully qualified pathname
                          PSZ pszPath,        //out- drive and directories
                          PSZ pszFileName )   //out- filename plus extension
{
  PSZ   pszPointer;
  CHAR  szPath[MAX_LONGPATH];

  strcpy( szPath, pszFullPath );
  pszPointer = strrchr( szPath, BACKSLASH );
  if ( pszPointer++ )
  {
    strcpy( pszFileName, pszPointer);
    *(pszPointer) = EOS;
    strcpy( pszPath, szPath );
  }
  else
  {
    strcpy( pszFileName, szPath );
    strcpy( pszPath, EMPTY_STRING );
  } /* endif */
}  /* end UtlMakeFNameAndPath */

// convert XML file to HTML using XSLT
int XSLTConversion
(
  PSZ         pszXmlFile,                        // name of file containing the XML data
  PSZ         pszHtmlFile,                       // name of file receiving the HTML output
  PSZ         pszStyleSheet,                     // stylesheet to be used for conversion
  BOOL        fMsg,                              // TRUE = show error messages
  HWND        hwndErrMsg                         // window to use as parent for error message
)
{
  int iRC = 0;

  fMsg; hwndErrMsg;

#ifdef XALANCODEDOESWORK
  // call Xalan converter to do the conversion
  try
  {
	  XALAN_USING_XERCES(XMLPlatformUtils)
  	XALAN_USING_XALAN(XalanTransformer)

  	// Call the static initializer for Xerces.
	  XMLPlatformUtils::Initialize();

  	// Initialize Xalan.
	  XalanTransformer::initialize();

		{
			// Create a XalanTransformer.
			XalanTransformer theXalanTransformer;

			// Do the transform.
			int iResult = theXalanTransformer.transform( pszXmlFile, pszStyleSheet, pszHtmlFile );

			if( iResult != 0 )
			{
  			const char *pszError = theXalanTransformer.getLastError();
        PSZ pszParms[3];

        pszParms[0] = (PSZ)pszError;
        pszParms[1] = pszXmlFile;
        pszParms[2] = pszStyleSheet;
        if ( fMsg ) UtlErrorHwnd( ERROR_XML_CONVERSION, MB_CANCEL, 3, pszParms, EQF_ERROR, hwndErrMsg );
        iRC = ERROR_XML_CONVERSION;
			}
		}

		// Terminate Xalan...
		XalanTransformer::terminate();

		// Terminate Xerces...
		XMLPlatformUtils::Terminate();

		// Clean up the ICU, if it's integrated...
		XalanTransformer::ICUCleanUp();
	}
	catch(...)
	{
    PSZ pszParms[3];
    PSZ pszError = "Unknown error in XML conversion";

    pszParms[0] = pszError;
    pszParms[1] = pszXmlFile;
    pszParms[2] = pszStyleSheet;
    if ( fMsg ) UtlErrorHwnd( ERROR_XML_CONVERSION, MB_CANCEL, 3, pszParms, EQF_ERROR, hwndErrMsg );
    iRC = ERROR_XML_CONVERSION;
	}
#else
  {
    // use external conversion as there is no Xalan version available working with the Xerces 3.1.1 version
    static CHAR szCommand[1024];

    UtlMakeEQFPath( szCommand, NULC, SYSTEM_PATH, NULL );
    strcat( szCommand, "\\WIN\\XalanTransform \"" );
    strcat( szCommand, pszXmlFile );
    strcat( szCommand, "\" \"" );
    strcat( szCommand, pszStyleSheet );
    strcat( szCommand, "\" \"" );
    strcat( szCommand, pszHtmlFile );
    strcat( szCommand, "\"" );

    system( szCommand );
  }
#endif

  return iRC;
} /* end of XSLTConversion */


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/***                                                                ***/
/***                 Create-a-list-window function                  ***/
/***                                                                ***/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
BOOL CreateListWindow
(
  PSZ                  pszObjName,     // ptr to object name for process window
  PFN_LISTCALLBACK     pfnCallBack,    // callback function for list window
  PVOID                pvUserData,     // ptr to user data (is passed to callback
                                       // function in mp2 of WM_CREATE message)
  BOOL                 fRestart        // TRUE = restarted by TWB
)
{
  HWND     hframe, hclient;            // handle of created windows
  BOOL     fOK;                        // function return code
  LISTCREATEPARMS CreateParms;         // list creation parameter

  CreateParms.pvUserData  = pvUserData;
  CreateParms.pfnCallBack = pfnCallBack;
  CreateParms.fRestart    = fRestart;
  {
    /*********************************************************/
    /* to avoid flickering during the creation of the MDI    */
    /* Child window we have to position it outside of the    */
    /* screen - we always reposition it to the last stored   */
    /* positions via a WinSetWindowPos function after init.  */
    /*********************************************************/
    HWND  hwndMDIClient;
    MDICREATESTRUCT mdicreate;

    mdicreate.hOwner  = (HAB)UtlQueryULong( QL_HAB );
    mdicreate.szClass = GENERICLIST;
    mdicreate.szTitle = pszObjName;
    mdicreate.x       = -100;
    mdicreate.y       = -100;
    mdicreate.cx      = 1;
    mdicreate.cy      = 1;
    mdicreate.style   = WS_CLIPCHILDREN;
    mdicreate.lParam  = MP2FROMP(&CreateParms);

    hwndMDIClient = (HWND)UtlQueryULong( QL_TWBCLIENT );

    hframe = hclient =
    (HWND)SendMessage( hwndMDIClient,
                       WM_MDICREATE, 0,
                       MP2FROMP((LPMDICREATESTRUCT)&mdicreate) );
  }
  if( hframe)
  {
    WinPostMsg( hclient, WM_EQF_INITIALIZE, NULL, NULL);
    fOK = TRUE;
  }
  else
  {
    fOK = FALSE;
  } /* endif */
  return( fOK );
} /* end of function CreateListWindow */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ListOfFiles                                              |
//+----------------------------------------------------------------------------+
//|Function call:     ListOfFiles( pDDEClient, pCurDir, pFile, ppListIndex );  |
//+----------------------------------------------------------------------------+
//|Description:       read in the file and try to extract pointers to filenames|
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT pDDEClient  ptr to DDE instance structure     |
//|                   PSZ     pCurDir        ptr to current directory          |
//|                   PSZ     pFile          pointer to file name              |
//|                   PSZ     * ppListIndex  pointer to array of files         |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     file could be read in and pointers extracted    |
//|                   FALSE    something went wrong - error message  displayed |
//+----------------------------------------------------------------------------+
//|Side Effects:      the allocated memory will not be freed, since only ptrs  |
//|                   to it will be set up...                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     update file name to be fully qualified                   |
//|                   try to load the specified file                           |
//|                   if okay                                                  |
//|                     split it up into pointers                              |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
BOOL UtlListOfFiles
(
  PSZ **pppCurDirIndex,             // pointer to curdir array
  PSZ pFile,                        // name of list file
  PSZ  **pppListIndex               // pointer to listindex array
)
{
  PSZ  pData;                          // pointer to data
  BOOL fOK = TRUE;                     // success indicator
  ULONG   ulSize = 0;                      // size of the file
  USHORT usChar;                       // character to be checked
  CHAR   szBuf[MAX_LONGPATH+MAX_FILESPEC]; // temp buffer
  CHAR   szOutBuf[ MAX_EQF_PATH ];

  /********************************************************************/
  /* ensure, that the list  file is fully qualified -- if not do it   */
  /********************************************************************/
  pData = UtlGetFnameFromPath( pFile );
  if ( !pData && (pppCurDirIndex != NULL))
  {
    fOK = UtlInsertCurDir(pFile, pppCurDirIndex, szOutBuf);
    if ( fOK )
    {
      pFile = szOutBuf;
      strcpy( szBuf, pFile );
    } /* endif */
  }
  else
  {
    strcpy( szBuf, pFile );
  } /* endif */

  /********************************************************************/
  /* load the file - limited to files up to 64k                       */
  /********************************************************************/
  if ( fOK )
  {
    pData = NULL;
    fOK = UtlLoadFileL( szBuf, (PVOID *)&pData, &ulSize, FALSE, FALSE );
  } /* endif */

  if ( fOK )
  {
    /******************************************************************/
    /* get rid of the file ending symbols ....                        */
    /******************************************************************/
    if ( pData[ulSize-1] == EOFCHAR )
    {
      ulSize--;
    } /* endif */
    usChar = pData[ulSize-1];
    while ( usChar ==  LF || usChar == CR || usChar == BLANK)
    {
      ulSize--;
      usChar = pData[ulSize-1];
    } /* endwhile */

    pData[ulSize] = EOS;

    fOK = UtlValidateList( pData, pppListIndex, MAX_LIST_FILES );
  } /* endif */

  return ( fOK );
} /* end of function UtlListOfFiles */

