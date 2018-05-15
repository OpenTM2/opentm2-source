
//+----------------------------------------------------------------------------+
//|EQFMT01.C                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2013, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:                                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Dialog procedure for Machine Translation Properties            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.2 $ ----------- 9 Mar 2007
// GQ: - base availability of FTP on trigger file
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.2 $ ----------- 30 Nov 2004
// GQ: - fixed P020914: inconsistent MT job names in TM and on the server
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.3 $ ----------- 3 Nov 2004
// GQ: - added handling for additional FTP fields
// 
// 
// $Revision: 1.2 $ ----------- 28 Sep 2004
// GQ: - accessibility: added call to UtlInvokeHelp
//     - accessibility: reworked owner drawn button paintig
//     - accessibility: ensure that tree view controls have an initial selection
// 
// 
// $Revision: 1.1 $ ----------- 30 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.2 $ ----------- 16 Jul 2004
// GQ: - P019703 MTI&FTP-access: no access via FTP
//       added code for FTP mode
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.5 $ ----------- 10 Apr 2003
// GQ: - added handling for subject area sort order buttons
// 
// 
// $Revision: 1.4 $ ----------- 17 Mar 2003
// --RJ: removed compiler defines not needed any more and rework code to avoi
// 
//
// $Revision: 1.3 $ ----------- 26 Feb 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
//
//
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
//
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.8 $ ----------- 5 Dec 2002
// GQ: - added handling for number of words field in statistics page
//
//
// $Revision: 1.7 $ ----------- 16 Oct 2002
// GQ: - added subject preselection dialog and implemented UCD proposals
//
//
// $Revision: 1.6 $ ----------- 9 Oct 2002
// GQ: - reworked subject area handling
//     - changed code for UCD findings
//
//
// $Revision: 1.5 $ ----------- 20 Sep 2002
// GQ: - Added statistic page to MT job properties dialog
//
//
// $Revision: 1.4 $ ----------- 18 Sep 2002
// GQ: - removed conditional compilation statements around MT code
//     - removed obsolete code
//
//
// $Revision: 1.3 $ ----------- 17 Sep 2002
// GQ: - completed MT Job property dialog code
//
//
// $Revision: 1.2 $ ----------- 16 Aug 2002
// GQ: - started implementation of R007498 MT support
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.3 $ ----------- 4 Jun 2002
// RJ: P014550: Add checking if MT Subsystem is available (via the ini file)
//
//
// $Revision: 1.2 $ ----------- 3 Sep 2001
// -- RJ: get rid of compiler warnings
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
// $Revision: 1.3 $ ----------- 25 Sep 2000
// -- get rid of compiler warnings
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFMT01.CV_   1.2   15 Oct 1998 19:50:24   BUILD  $
 *
 * $Log:   K:\DATA\EQFMT01.CV_  $
 *
 *    Rev 1.2   15 Oct 1998 19:50:24   BUILD
 * - init profiles returned..
 *
 *    Rev 1.1   14 Sep 1998 15:29:26   BUILD
 * Updated Profile
 *
 *    Rev 1.0   25 Aug 1998 19:04:16   BUILD
 * Initial revision.
 *
 */
//+----------------------------------------------------------------------------+

#define INCL_EQF_DLGUTILS         // set output file dialog
#define INCL_EQF_ANALYSIS
#define INCL_EQF_FOLDER           // folder list and document list functions

#include "eqf.h"                  // general TranslationManager include file
#include "eqfhlog.h"              // defines and structures of history log file
#include "eqfmt.h"                // definition of MTPASS structure
#include "eqfmt00.h"              // generic include for MT handler
#include "eqfmt.id"               // id file
#include "commctrl.h"          // common controls
#include "eqfstart.h"

#define LONG2DATETIME(ltime, buffer) \
{ \
  UtlLongToDateString(ltime, buffer, sizeof (buffer)); \
  strcat(buffer, "  ");  \
  UtlLongToTimeString(ltime, buffer + strlen (buffer), \
                      sizeof (buffer) - strlen (buffer)); \
}


extern HELPSUBTABLE hlpsubtblAnaPropDlg[];

typedef struct _MTPROPIDA
{
  BOOL        fPropertyDlg;                      // TRUE = this is a property dialog
  HWND        hwndTabCtrl;                       // handle of tab control
  HWND        hwndPages[10];                     // support up to 10 prop pages
  CHAR        szPath[MAX_LONGFILESPEC];          // buffer for path names
  PSZ         pszSubAreaList;                    // ptr to loaded subject areas
  CHAR        szPropName[MAX_EQF_PATH];          // buffer for property file name
  PMTJOBPROP  pMTProp;                           // pointer to loaded or created MT job properties
  PANAMTDATA  pAnaData;                          // ptr to analysis<->MT interface area
  CHAR        szBuffer[1024];                    // buffer
  CHAR        szSubjectBuffer[4000];             // buffer for subject areas
  BOOL        fMQSAvail;                         // TRUE = MQ series is available
  BOOL        fDlgInit;                          // TRUE = currently processing dialog initialization 
} MTPROPIDA, *PMTPROPIDA;

BOOL    MTPropertySheetLoad( HWND hwndDlg, PMTPROPIDA pIda );
MRESULT MTPropertySheetNotification( HWND hwndDlg, WPARAM  mp1, LPARAM  mp2 );
MRESULT MTPropCommand( HWND hwndDlg, WPARAM mp1, LPARAM mp2 );
INT_PTR CALLBACK MTPROP_GENERAL_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
INT_PTR CALLBACK MTPROP_SERVER_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
INT_PTR CALLBACK MTPROP_STAT_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
BOOL MTLoadPropsFromDefaults
(
  PSZ         pszDefaults,             // full name of file containing defaults
  PMTJOBPROP  pProps                   // properties being filled
);
void MTGetOrSetCheckedItems
(
  HWND        hwndTV,                       // handle of tree view control
  HTREEITEM   hItem,                        // handle of item to start with
  PSZ         pszSubjArea,                  // area for selected subject area
  BOOL        fSetCheck                     // TRUE = set checkmarks
);
BOOL MTPropsToDefaults
(
  PSZ         pszDefaults,             // full name of file receiving defaults
  PMTJOBPROP  pProps                   // properties being filled
);
INT_PTR CALLBACK MTPROP_SUBJECT_DLGPROC
(
  HWND   hwndDlg,
  WINMSG message,
  WPARAM mp1,
  LPARAM mp2
);
void MTFillSubjectAreaList
(
  HWND hwndList,
  PSZ  pszSubject,
  PSZ  pszSelSubjects
);
// function to encrypt or decrypt the FTP password
BOOL MTEncryptPassword
( 
  PSZ         pszPassword,             // in: password or encrypted string
  PSZ         pszBuffer,               // out: encrypted string or password 
  int         iBufLen,                 // size of output buffer
  BOOL        fEncrypt                 // TRUE = encrypt, FALSE = decrypt
);





INT_PTR CALLBACK EQFMTPROPERTYDLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
   ULONG    ulTabCtrl;
   PMTPROPIDA    pIda;                      // dialog instance data area
   BOOL     fOK;                       // internal O.K. flag

   switch ( msg )
   {
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_MTPROP_DLG, mp2 ); break;

      case WM_INITDLG:
        // allocate and anchor our dialog IDA
        fOK = UtlAlloc( (PVOID *) &pIda, 0L, (ULONG)sizeof(MTPROPIDA), ERROR_STORAGE );
        if ( fOK )
        {
          fOK = ANCHORDLGIDA( hwndDlg, pIda );
          if ( !fOK )                           //no access to ida
          {
            UtlErrorHwnd( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR, hwndDlg);
          } /* endif */
        } /* endif */

        // load list of subject areas into memory
        if ( fOK )
        {
          ULONG ulBytes;
          UtlMakeEQFPath( pIda->szPath, NULC, TABLE_PATH, NULL );
          strcat( pIda->szPath, BACKSLASH_STR );
          strcat( pIda->szPath, SUBJAREALISTFILE );
          if ( UtlLoadFileL( pIda->szPath, (PVOID *)&(pIda->pszSubAreaList), &ulBytes, FALSE, FALSE ) )
          {
            pIda->pszSubAreaList[ulBytes-1] = EOS;
          } /* endif */
        } /* endif */

        // determine type of record: new dialog is called with ptr to Ana<->MT IF data area
        //                           property dialog is called with name of property file
        if ( fOK )
        {
          PSZ pszTemp = (PSZ)PVOIDFROMMP2( mp2 );

          if ( isalpha( pszTemp[0] ) && (pszTemp[1] == COLON) )
          {
            ULONG ulBytes;

            // parameter starts with drive letter, assume property dialog
            pIda->fPropertyDlg = TRUE;
            SetWindowText( hwndDlg, "MT Job Properties" );

            // store name of property file and load property file into memory
            strcpy( pIda->szPropName, pszTemp );
            if ( !UtlLoadFileL( pIda->szPropName, (PVOID *)&(pIda->pMTProp), &ulBytes, FALSE, FALSE ) )
            {
              CHAR szParm[40];
              PSZ  pszParm = szParm;
              strcpy( szParm, "MT Job " );
              Utlstrccpy( szParm + strlen(szParm), UtlGetFnameFromPath( pIda->szPropName ), DOT );
              UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszParm, EQF_ERROR );
              fOK = FALSE;
            } /* endif */
          }
          else
          {
            // parameter does not start with drive letter, assume new dialog
            pIda->fPropertyDlg = FALSE;
            SetWindowText( hwndDlg, "New MT Job" );

            // store pointer of Analysis<->MT interface in IDA
            pIda->pAnaData = (PANAMTDATA)PVOIDFROMMP2( mp2 );

            // create property file name and empty property file
            {
              LONG lTimeStamp;
              SYSTEMTIME SystemTime;             // system time

              UtlTime( &lTimeStamp );
              UtlMakeEQFPath( pIda->szPropName, NULC, DIRSEGMT_PATH, NULL );
              strcat( pIda->szPropName, BACKSLASH_STR );
              strcat( pIda->szPropName, pIda->pAnaData->szFolShortName );
              GetSystemTime( &SystemTime );
              sprintf( pIda->szPropName+strlen(pIda->szPropName), "-%4.4d%2.2d%2.2d%2.2d%2.2d%3.3d", 
                SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, 
                SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds );
              strcat( pIda->szPropName, EXT_OF_MTPROP );
              strcpy( pIda->pAnaData->szMTJobObject, pIda->szPropName );
              Utlstrccpy( pIda->pAnaData->szSegFile, pIda->szPropName, DOT );
              strcat( pIda->pAnaData->szSegFile, EXT_OF_MTSEGS );
              fOK = UtlAlloc( (PVOID *)&(pIda->pMTProp), 0L, sizeof(MTJOBPROP), ERROR_STORAGE );
              if ( fOK )
              {
                 pIda->pMTProp->lJobCreateDate = lTimeStamp;
                 strcpy( pIda->pMTProp->szMTEngineDLL, MT_ENGINE_MQS );

                 strcpy( pIda->pMTProp->szJobObject, pIda->szPropName );

                 Utlstrccpy( pIda->pMTProp->szSegFile, pIda->szPropName, DOT );
                 strcat( pIda->pMTProp->szSegFile, EXT_OF_MTSEGS );

                 Utlstrccpy( pIda->pMTProp->szZipFile, pIda->szPropName, DOT );
                 strcat( pIda->pMTProp->szZipFile, ".ZIP" );
              } /* endif */

              // check if MQ series is installed
              if ( fOK )
              {
                HMODULE hmodDll = NULLHANDLE;

                if ( DosLoadModule( NULL, 0 , MT_ENGINE_MQS, &hmodDll) != 0 )
                {
                  // MQ series is not available switch to FTP mode
                  pIda->fMQSAvail = FALSE;  
                }
                else
                {
                  pIda->fMQSAvail = TRUE;  
                  DosFreeModule( hmodDll );
                } /* endif */
              } /* endif */
            }

            // fill-in defaults or last-used values
            if ( fOK )
            {
              UtlMakeEQFPath( pIda->szPath, NULC, PROPERTY_PATH, NULL );
              strcat( pIda->szPath, BACKSLASH_STR );
              strcat( pIda->szPath, MTLASTUSED );

              if ( !MTLoadPropsFromDefaults( pIda->szPath, pIda->pMTProp ) )
              {
                UtlMakeEQFPath( pIda->szPath, NULC, PROPERTY_PATH, NULL );
                strcat( pIda->szPath, BACKSLASH_STR );
                strcat( pIda->szPath, MTDEFAULTS );

                MTLoadPropsFromDefaults( pIda->szPath, pIda->pMTProp );
              } /* endif */
            } /* endif */

            // fill in values from Ana<->MT IF data area
            if ( fOK )
            {
              strcpy( pIda->pMTProp->szFolderName, pIda->pAnaData->szFolderName );
              strcpy( pIda->pMTProp->szSourceLang, pIda->pAnaData->szSourceLang );
              strcpy( pIda->pMTProp->szTargetLang, pIda->pAnaData->szTargetLang );
            } /* endif */
          } /* endif */
        } /* endif */

        if ( !fOK )
        {
          // close dialog, FALSE means: - do not start MT job
          WinDismissDlg( hwndDlg, SHORT1FROMMP1(FALSE) );
        }
        else
        {
          MTPropertySheetLoad( hwndDlg, pIda );
        } /* endif */

        // create new MT job profile or use existing one

        mResult = DIALOGINITRETURN( mResult );
        break;


      case WM_COMMAND:
         mResult = MTPropCommand( hwndDlg, mp1, mp2 );
         break;

      case WM_NOTIFY:
         mResult = MTPropertySheetNotification( hwndDlg, mp1, mp2 );
         break;

      case WM_HELP:
         /*************************************************************/
         /* pass on a HELP_WM_HELP request                            */
         /*************************************************************/
//         EqfDisplayContextHelp( ((LPHELPINFO) mp2)->hItemHandle,
//                                &hlpsubtblAnaPropDlg[0] );
         mResult = TRUE;  // message processed
         break;


      case WM_EQF_CLOSE:
         //--- get rid off dialog ---
         pIda = ACCESSDLGIDA(hwndDlg, PMTPROPIDA);
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

           if ( pIda->pszSubAreaList ) UtlAlloc( (PVOID *)&(pIda->pszSubAreaList), 0L, 0L, NOMSG );

           UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
         } /* endif */
         DISMISSDLG( hwndDlg, mp1 );
         break;

      case TCM_SETCURSEL:
        {
          TC_ITEM Item;
          HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_MTPROP_TABCTRL );
          pIda = ACCESSDLGIDA( hwndDlg, PMTPROPIDA);
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
} /* end of MTPROPDLGPROC */

BOOL MTPropertySheetLoad
(
  HWND hwndDlg,
  PMTPROPIDA     pIda
)
{
  BOOL      fOK = TRUE;
  TC_ITEM   TabCtrlItem;
  USHORT    nItem = 0;
  HWND      hwndTabCtrl;
  HINSTANCE hInst;
  CHAR      szBuffer[80];

  if ( fOK )
  {
    RECT rect;
    // remember adress of user area
    hInst = GETINSTANCE( hwndDlg );
    hwndTabCtrl = GetDlgItem( hwndDlg, ID_MTPROP_TABCTRL );
    pIda->hwndTabCtrl = hwndTabCtrl;
    GetClientRect( hwndTabCtrl, &rect );
    TabCtrl_AdjustRect( hwndTabCtrl, FALSE, &rect );

    // leave some additional space at top
    rect.top += 20;
    MapWindowPoints( hwndTabCtrl, hwndDlg, (POINT *) &rect, 2 );


    TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM;

    // create the appropriate TAB control and load the associated dialog
    // LOADSTRING( hab, hResMod, IDS_MTPROP_TAB_GENERAL, szBuffer );
    strcpy( szBuffer, "General" );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
      CreateDialogParam( hInst,
                         MAKEINTRESOURCE( ID_MTPROP_GENERAL_DLG ),
                         hwndDlg,
                         MTPROP_GENERAL_DLGPROC,
                         (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    //LOADSTRING( hab, hResMod, IDS_MTPROP_TAB_OTHER, szBuffer );
    strcpy( szBuffer, "Server" );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
      CreateDialogParam( hInst,
                         MAKEINTRESOURCE( ID_MTPROP_SERVER_DLG),
                         hwndDlg,
                         MTPROP_SERVER_DLGPROC,
                         (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    if ( pIda->fPropertyDlg )
    {
      strcpy( szBuffer, "Statistics" );
      TabCtrlItem.pszText = szBuffer;
      TabCtrlItem.lParam = nItem;
      SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
      pIda->hwndPages[nItem] =
        CreateDialogParam( hInst,
                           MAKEINTRESOURCE( ID_MTPROP_STAT_DLG),
                           hwndDlg,
                           MTPROP_STAT_DLGPROC,
                           (LPARAM)pIda );

      SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                    rect.left, rect.top,
                    rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
      SetFocus( pIda->hwndPages[nItem] );
      UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
      nItem++;
    } /* endif */

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

#ifndef ListView_SetCheckState
   #define ListView_SetCheckState(hwndLV, i, fCheck) \
      ListView_SetItemState(hwndLV, i, \
      INDEXTOSTATEIMAGEMASK((fCheck)+1), LVIS_STATEIMAGEMASK)
#endif


INT_PTR CALLBACK MTPROP_GENERAL_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PMTPROPIDA     pIda;

  switch ( msg )
  {
    case WM_INITDLG:
      pIda = (PMTPROPIDA)PVOIDFROMMP2( mp2 );
      ANCHORDLGIDA( hwndDlg, pIda );

      // set text limits
      SETTEXTLIMIT( hwndDlg, ID_MTPROP_EMAIL_EF, sizeof(pIda->pMTProp->szSenderEMail)-1 );
      SETTEXTLIMIT( hwndDlg, ID_MTPROP_WORDLIMIT_EF, 4 );
      SETTEXTLIMIT( hwndDlg, ID_MTPROP_SMALLFUZZY_EF, 3 );
      SETTEXTLIMIT( hwndDlg, ID_MTPROP_MEDIUMFUZZY_EF, 3 );
      SETTEXTLIMIT( hwndDlg, ID_MTPROP_LARGEFUZZY_EF, 3 );

      // disable some/all controls
      ENABLECTRL( hwndDlg, ID_MTPROP_FOLDER_EF, FALSE );
      ENABLECTRL( hwndDlg, ID_MTPROP_SOURCELANG_EF, FALSE );
      ENABLECTRL( hwndDlg, ID_MTPROP_TARGETLANG_EF, FALSE );
      if ( pIda->fPropertyDlg )
      {
        ENABLECTRL( hwndDlg, ID_MTPROP_EMAIL_EF, FALSE  );
        ENABLECTRL( hwndDlg, ID_MTPROP_WORDLIMIT_CHK, FALSE  );
        ENABLECTRL( hwndDlg, ID_MTPROP_WORDLIMIT_EF, FALSE  );
        ENABLECTRL( hwndDlg, ID_MTPROP_SMALLFUZZY_CHK, FALSE  );
        ENABLECTRL( hwndDlg, ID_MTPROP_SMALLFUZZY_EF, FALSE  );
        ENABLECTRL( hwndDlg, ID_MTPROP_MEDIUMFUZZY_EF, FALSE  );
        ENABLECTRL( hwndDlg, ID_MTPROP_LARGEFUZZY_EF, FALSE  );
        ENABLECTRL( hwndDlg, ID_MTPROP_CHANGESUBJECT_PB, FALSE );
        ENABLECTRL( hwndDlg, ID_MTPROP_SUBJAREA_LB, FALSE );
      } /* endif */

      // fill fields from properties
      {
        CHAR szNum[10];
        OEMSETTEXT( hwndDlg, ID_MTPROP_FOLDER_EF, pIda->pMTProp->szFolderName );
        SETTEXT( hwndDlg, ID_MTPROP_SOURCELANG_EF, pIda->pMTProp->szSourceLang );
        SETTEXT( hwndDlg, ID_MTPROP_TARGETLANG_EF, pIda->pMTProp->szTargetLang );
        SETTEXT( hwndDlg, ID_MTPROP_EMAIL_EF, pIda->pMTProp->szSenderEMail  );

        itoa( pIda->pMTProp->usMaxWords, szNum, 10 );
        if ( pIda->pMTProp->usMaxWords )
        {
          SETTEXT( hwndDlg, ID_MTPROP_WORDLIMIT_EF, szNum  );
        } /* endif */
        if ( pIda->pMTProp->fMaxWordsFilter )
        {
          SETCHECK_TRUE( hwndDlg, ID_MTPROP_WORDLIMIT_CHK  );
        }
        else
        {
          ENABLECTRL( hwndDlg, ID_MTPROP_WORDLIMIT_TEXT, FALSE );
          ENABLECTRL( hwndDlg, ID_MTPROP_WORDLIMIT_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_MTPROP_WORDLIMIT2_TEXT, FALSE );
        } /* endif */


        if ( pIda->pMTProp->fFuzzyFilter )
        {
          SETCHECK_TRUE( hwndDlg, ID_MTPROP_SMALLFUZZY_CHK  );
        }
        else
        {
          ENABLECTRL( hwndDlg, ID_MTPROP_SMALLFUZZY_TEXT, FALSE );
          ENABLECTRL( hwndDlg, ID_MTPROP_SMALLFUZZY_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_MTPROP_SMALLFUZZY2_TEXT, FALSE );
          ENABLECTRL( hwndDlg, ID_MTPROP_MEDIUMFUZZY_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_MTPROP_MEDIUMFUZZY2_TEXT, FALSE );
          ENABLECTRL( hwndDlg, ID_MTPROP_LARGEFUZZY_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_MTPROP_LARGEFUZZY2_TEXT, FALSE );
        } /* endif */

        itoa( pIda->pMTProp->usSmallFuzzy, szNum, 10 );
        if ( pIda->pMTProp->usSmallFuzzy )
        {
          SETTEXT( hwndDlg, ID_MTPROP_SMALLFUZZY_EF, szNum  );
        } /* endif */

        itoa( pIda->pMTProp->usMediumFuzzy, szNum, 10 );
        if ( pIda->pMTProp->usMediumFuzzy )
        {
          SETTEXT( hwndDlg, ID_MTPROP_MEDIUMFUZZY_EF, szNum  );
        } /* endif */

        itoa( pIda->pMTProp->usLargeFuzzy, szNum, 10 );
        if ( pIda->pMTProp->usLargeFuzzy )
        {
          SETTEXT( hwndDlg, ID_MTPROP_LARGEFUZZY_EF, szNum );
        } /* endif */
      }

      // fill list view control
      {
        HWND hwndList = GetDlgItem( hwndDlg, ID_MTPROP_SUBJAREA_LB );

        // switch to checkboxed style (style setting in dialog template is ignored)
        ListView_SetExtendedListViewStyleEx( hwndList, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES );

        // use selected subject areas if available subject areas list is empty
        if ( pIda->pMTProp->szAvailSubjArea[0] == EOS )
        {
          strcpy( pIda->pMTProp->szAvailSubjArea, pIda->pMTProp->szSubjArea );
        } /* endif */

        // fill and show view control or hide control and show dummy text
        if ( pIda->pMTProp->szAvailSubjArea[0] != EOS )
        {
          HIDECONTROL( hwndDlg, ID_MTPROP_NOSUBJECT_TEXT );
          SHOWCONTROL( hwndDlg, ID_MTPROP_SUBJAREA_LB );
          MTFillSubjectAreaList( hwndList, pIda->pMTProp->szAvailSubjArea,
                                           pIda->pMTProp->szSubjArea );
          ListView_SetItemState( hwndList, 0, LVIS_SELECTED, LVIS_SELECTED ); 
        }
        else
        {
          HIDECONTROL( hwndDlg, ID_MTPROP_SUBJAREA_LB );
          SHOWCONTROL( hwndDlg, ID_MTPROP_NOSUBJECT_TEXT );
        } /* endif */
      }
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case PID_PB_OK:
           /****************************************************************/
           /* if mp2 == 1L we have to validate the page, if it is 0L we    */
           /* have to copy the content of the dialog back into the struct. */
           /****************************************************************/
           if ( mp2 == 1L )
           {
             /**************************************************************/
             /* no checking for correct input necessary                    */
             /**************************************************************/
           }
           else
           {
             // get the active settings ....
             CHAR szNum[10];
             pIda = ACCESSDLGIDA( hwndDlg, PMTPROPIDA );

             QUERYTEXT( hwndDlg, ID_MTPROP_EMAIL_EF, pIda->pMTProp->szSenderEMail  );

             pIda->pMTProp->fMaxWordsFilter = QUERYCHECK( hwndDlg, ID_MTPROP_WORDLIMIT_CHK );
             QUERYTEXT( hwndDlg, ID_MTPROP_WORDLIMIT_EF, szNum  );
             pIda->pMTProp->usMaxWords = (USHORT)atoi(szNum);

             pIda->pMTProp->fFuzzyFilter = QUERYCHECK( hwndDlg, ID_MTPROP_SMALLFUZZY_CHK );
             QUERYTEXT( hwndDlg, ID_MTPROP_SMALLFUZZY_EF, szNum  );
             pIda->pMTProp->usSmallFuzzy = (USHORT)atoi(szNum);

             QUERYTEXT( hwndDlg, ID_MTPROP_MEDIUMFUZZY_EF, szNum  );
             pIda->pMTProp->usMediumFuzzy = (USHORT)atoi(szNum);

             QUERYTEXT( hwndDlg, ID_MTPROP_LARGEFUZZY_EF, szNum );
             pIda->pMTProp->usLargeFuzzy = (USHORT)atoi(szNum);

             // get list of subject area and check state
             {
               HWND hwndList = GetDlgItem( hwndDlg, ID_MTPROP_SUBJAREA_LB );
               int iMax = ListView_GetItemCount( hwndList );
               BOOL fFirst = TRUE;
               BOOL fFirstAvail = TRUE;
               int iItem = 0;
               PSZ pszSubjList = pIda->pMTProp->szSubjArea;
               PSZ pszAvailList = pIda->pMTProp->szAvailSubjArea;
               while ( iItem < iMax )
               {
                 LVITEM ListItem;
                 BOOL fChecked;

                 if ( !fFirstAvail ) *pszAvailList++ = ',';

                 memset( &ListItem, 0, sizeof(ListItem) );
                 ListItem.mask       = LVIF_TEXT;
                 ListItem.iItem      = iItem;
                 ListItem.pszText    = pszAvailList;
                 ListItem.cchTextMax = 40;
                 ListView_GetItem( hwndList, &ListItem );
                 fFirstAvail = FALSE;

                 fChecked = ListView_GetCheckState( hwndList, iItem );
                 if ( fChecked )
                 {
                   if ( !fFirst ) *pszSubjList++ = ',';

                   strcpy( pszSubjList, pszAvailList );
                   pszSubjList += strlen(pszSubjList);
                   fFirst = FALSE;
                 } /* endif */
                 pszAvailList += strlen(pszAvailList);
                 iItem++;
               } /* endwhile */
               *pszSubjList = EOS;
               *pszAvailList = EOS;
             }
           } /* endif */
           break;

        case ID_MTPROP_WORDLIMIT_CHK:
          {
            pIda = ACCESSDLGIDA( hwndDlg, PMTPROPIDA );
            if ( !pIda->fPropertyDlg )
            {
              BOOL fChecked = (BOOL)QUERYCHECK( hwndDlg, ID_MTPROP_WORDLIMIT_CHK );
              ENABLECTRL( hwndDlg, ID_MTPROP_WORDLIMIT_TEXT, fChecked  );
              ENABLECTRL( hwndDlg, ID_MTPROP_WORDLIMIT_EF, fChecked  );
              ENABLECTRL( hwndDlg, ID_MTPROP_WORDLIMIT2_TEXT, fChecked  );
            } /* endif */
          }
          break;

        case ID_MTPROP_SMALLFUZZY_CHK:
          {
            pIda = ACCESSDLGIDA( hwndDlg, PMTPROPIDA );
            if ( !pIda->fPropertyDlg )
            {
              BOOL fChecked = (BOOL)QUERYCHECK( hwndDlg, ID_MTPROP_SMALLFUZZY_CHK );
              ENABLECTRL( hwndDlg, ID_MTPROP_SMALLFUZZY_TEXT, fChecked  );
              ENABLECTRL( hwndDlg, ID_MTPROP_SMALLFUZZY_EF, fChecked  );
              ENABLECTRL( hwndDlg, ID_MTPROP_SMALLFUZZY2_TEXT, fChecked  );
              ENABLECTRL( hwndDlg, ID_MTPROP_LARGEFUZZY_EF, fChecked  );
              ENABLECTRL( hwndDlg, ID_MTPROP_LARGEFUZZY2_TEXT, fChecked  );
              ENABLECTRL( hwndDlg, ID_MTPROP_MEDIUMFUZZY_EF, fChecked  );
              ENABLECTRL( hwndDlg, ID_MTPROP_MEDIUMFUZZY2_TEXT, fChecked  );
            } /* endif */
          }
          break;

        case ID_MTPROP_CHANGESUBJECT_PB:
          {
            BOOL fOK;
            HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

            pIda = ACCESSDLGIDA( hwndDlg, PMTPROPIDA );
            DIALOGBOX( hwndDlg, MTPROP_SUBJECT_DLGPROC,
                       hResMod, ID_MTPROP_SUBJECT_DLG, pIda, fOK );

            if ( fOK )
            {
              // refresh available subject area list
              HWND hwndList = GetDlgItem( hwndDlg, ID_MTPROP_SUBJAREA_LB );
              MTFillSubjectAreaList( hwndList, pIda->pMTProp->szAvailSubjArea,
                                     pIda->pMTProp->szSubjArea );
              if ( pIda->pMTProp->szAvailSubjArea[0] != EOS )
              {
                HIDECONTROL( hwndDlg, ID_MTPROP_NOSUBJECT_TEXT );
                SHOWCONTROL( hwndDlg, ID_MTPROP_SUBJAREA_LB );
              }
              else
              {
                HIDECONTROL( hwndDlg, ID_MTPROP_SUBJAREA_LB );
                SHOWCONTROL( hwndDlg, ID_MTPROP_NOSUBJECT_TEXT );
              } /* endif */

            } /* endif */
          }
          break;

        case ID_MTPROP_SUBJECTTOP_PB:
        case ID_MTPROP_SUBJECTUP_PB:
        case ID_MTPROP_SUBJECTDN_PB:
        case ID_MTPROP_SUBJECTBOT_PB:
          {
            HWND hwndList = GetDlgItem( hwndDlg, ID_MTPROP_SUBJAREA_LB );
            // LVITEM ListItem;
            UINT uiItems = ListView_GetSelectedCount( hwndList );

            if ( uiItems )
            {
              int iSelItem = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
              if ( iSelItem >= 0 )
              {
                LVITEM ListItem;
                BOOL fChecked;
                CHAR szSubject[40];   // buffer for subject text
                int iItem;
                int iMaxItems;

                // get item data
                memset( &ListItem, 0, sizeof(ListItem) );
                ListItem.mask       = LVIF_TEXT;
                ListItem.iItem      = iSelItem;
                ListItem.pszText    = szSubject;
                ListItem.cchTextMax = sizeof(szSubject)-1;
                ListView_GetItem( hwndList, &ListItem );
                fChecked = ListView_GetCheckState( hwndList, iSelItem );

                // delete item at curren position
                ListView_DeleteItem( hwndList, iSelItem );

                // get number of remaining items
                iMaxItems = ListView_GetItemCount( hwndList );

                // compute new itemposition
                switch ( WMCOMMANDID( mp1, mp2 ) )
                {
                  case ID_MTPROP_SUBJECTTOP_PB:
                    iSelItem = 0;
                    break;
                  case ID_MTPROP_SUBJECTUP_PB:
                    if ( iSelItem > 0 ) iSelItem--;
                    break;
                  case ID_MTPROP_SUBJECTDN_PB:
                    if ( iSelItem < iMaxItems ) iSelItem++;
                    break;
                  case ID_MTPROP_SUBJECTBOT_PB:
                    iSelItem = iMaxItems;
                    break;
                } /* endswitch */

                // insert in new position
                memset( &ListItem, 0, sizeof(ListItem) );
                ListItem.mask = LVIF_TEXT;
                ListItem.pszText = szSubject;
                ListItem.iItem = iSelItem;
                iItem = ListView_InsertItem( hwndList, &ListItem );
                if ( fChecked )
                {
                  ListView_SetCheckState( hwndList, iItem, TRUE );
                } /* endif */
                ListView_SetItemState( hwndList, iItem, LVIS_SELECTED, LVIS_SELECTED ); 
                SetFocus( hwndList );
              } /* endif */
            } /* endif */
          }
          break;
        } /* endswitch */
        break;

      case WM_DRAWITEM:
        {
          // draw our picture buttons
          LPDRAWITEMSTRUCT lpDisp = (LPDRAWITEMSTRUCT)mp2;
          RECT InnerRect;
          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

          memcpy( &InnerRect, &(lpDisp->rcItem), sizeof(InnerRect) );
          InnerRect.bottom -= 4;
          InnerRect.left   += 4;
          InnerRect.right  -= 4;
          InnerRect.top    += 4;

          {
            HICON hIcon = 0;

            switch ( lpDisp->CtlID )
            {
              case ID_MTPROP_SUBJECTTOP_PB:
                hIcon = LoadIcon( hResMod, MAKEINTRESOURCE(EQFTOP_ICON) );
                break;
                
              case ID_MTPROP_SUBJECTUP_PB:
                hIcon = LoadIcon( hResMod, MAKEINTRESOURCE(EQFUP_ICON ) );
                break;
              case ID_MTPROP_SUBJECTDN_PB:
                hIcon = LoadIcon( hResMod, MAKEINTRESOURCE(EQFDOWN_ICON) );
                break;
              case ID_MTPROP_SUBJECTBOT_PB:
                hIcon = LoadIcon( hResMod, MAKEINTRESOURCE(EQFBOTTOM_ICON) );
                break;
            } /* endswitch */

            FillRect( lpDisp->hDC, &lpDisp->rcItem, GetSysColorBrush( COLOR_BTNFACE ) );

            if ( UtlIsHighContrast() )
            {
              InvertRect( lpDisp->hDC, &InnerRect );
            } /* endif */
            DrawIcon( lpDisp->hDC, 8, 4, hIcon ); 
            DestroyIcon( hIcon );
            if ( UtlIsHighContrast() )
            {
              InvertRect( lpDisp->hDC, &InnerRect );
            } /* endif */
            
            FrameRect(lpDisp->hDC, &lpDisp->rcItem, GetSysColorBrush( COLOR_BTNTEXT ) );

            // draw a focus rectangle
            if ( lpDisp->hwndItem == GetFocus() )
            {
              DrawFocusRect( lpDisp->hDC, &InnerRect );
            } /* endif */

          } /* endif */
        }
        break;

      case WM_HELP:
         /*************************************************************/
         /* pass on a HELP_WM_HELP request                            */
         /*************************************************************/
//         EqfDisplayContextHelp( ((LPHELPINFO) mp2)->hItemHandle,
//                                &hlpsubtblAnaPropGeneral[0] );
         mResult = TRUE;  // message processed
         break;

//   case WM_NOTIFY:
//     switch (((LPNMHDR) lParam)->code)
//       {
//         case
//       } /* endswitch */
//       break;
    case WM_CLOSE:
       mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
       break;

    default:
       mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return mResult;
};


INT_PTR CALLBACK MTPROP_SERVER_DLGPROC
(
   HWND hwnd,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PMTPROPIDA     pIda;

  switch ( msg )
  {
    case WM_INITDLG:
      {
        BOOL fTriggerFile = FALSE;

        pIda = (PMTPROPIDA)PVOIDFROMMP2( mp2 );

        pIda->fDlgInit = TRUE;

        ANCHORDLGIDA( hwnd, pIda );

        // check existence of trigger file
        {
           CHAR szTriggerFile[MAX_EQF_PATH];

           UtlMakeEQFPath( szTriggerFile, NULC, PROPERTY_PATH, NULL );
           strcat( szTriggerFile, "\\EQFMTTRGF.PRP" );
           fTriggerFile = UtlFileExist( szTriggerFile );
        }

        // set text limits
        SETTEXTLIMIT( hwnd, ID_MTPROP_MQSERVER_EF, sizeof(pIda->pMTProp->szMQServer)-1 );
        SETTEXTLIMIT( hwnd, ID_MTPROP_QUEUEMGR_EF, sizeof(pIda->pMTProp->szQueueMgr)-1 );
        SETTEXTLIMIT( hwnd, ID_MTPROP_INPQUEUE_EF, sizeof(pIda->pMTProp->szServerInQueue)-1 );
        SETTEXTLIMIT( hwnd, ID_MTPROP_OUTQUEUE_EF, sizeof(pIda->pMTProp->szServerOutQueue)-1 );
        SETTEXTLIMIT( hwnd, ID_MTPROP_FTPSERVER_EF, sizeof(pIda->pMTProp->szFTPServer)-1 );
        SETTEXTLIMIT( hwnd, ID_MTPROP_FTPUSER_EF, sizeof(pIda->pMTProp->szFTPUserID)-1 );
        SETTEXTLIMIT( hwnd, ID_MTPROP_FTPPASSWORD_EF, MAX_MT_NAME - 1 );

        SETTEXTLIMIT( hwnd, ID_MTPROP_FTPPORT_EF, sizeof(pIda->pMTProp->szFTPPort)-1 );
        SETTEXTLIMIT( hwnd, ID_MTPROP_FTPPROXYADDRESS, sizeof(pIda->pMTProp->szProxyAddress)-1 );
        SETTEXTLIMIT( hwnd, ID_MTPROP_FTPPROXYPORT, sizeof(pIda->pMTProp->szProxyPort)-1 );

        // disable some/all controls
        if ( pIda->fPropertyDlg )
        {
          SETCHECK( hwnd, ID_MTPROP_FTP_RB, !pIda->pMTProp->fUseFTP );
          SETCHECK( hwnd, ID_MTPROP_MQS_RB, pIda->pMTProp->fUseFTP );
          ENABLECTRL( hwnd, ID_MTPROP_MQSERVER_EF, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_QUEUEMGR_EF, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_INPQUEUE_EF, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_OUTQUEUE_EF, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_FTPSERVER_EF, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_FTPUSER_EF, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_FTPPASSWORD_EF, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_FTP_RB, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_MQS_RB, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_FTPPORT_EF, FALSE );        
          ENABLECTRL( hwnd, ID_MTPROP_FTPDIRECTACCESS_RB, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_FTPSOCKSPROXY_RB, FALSE );  
          ENABLECTRL( hwnd, ID_MTPROP_FTPPROXY_RB, FALSE );       
          ENABLECTRL( hwnd, ID_MTPROP_FTPPASSIVEMODE_CHK, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYADDRESS, FALSE );   
          ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYPORT, FALSE );      
        }
        else if ( !fTriggerFile )
        {
          // no FTP allowed
          pIda->pMTProp->fUseFTP = FALSE;
          ENABLECTRL( hwnd, ID_MTPROP_FTP_RB, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_MQS_RB, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_FTPPORT_EF, FALSE );        
          ENABLECTRL( hwnd, ID_MTPROP_FTPDIRECTACCESS_RB, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_FTPSOCKSPROXY_RB, FALSE );  
          ENABLECTRL( hwnd, ID_MTPROP_FTPPROXY_RB, FALSE );       
          ENABLECTRL( hwnd, ID_MTPROP_FTPPASSIVEMODE_CHK, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYADDRESS, FALSE );   
          ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYPORT, FALSE );      
          SETCHECK_FALSE( hwnd, ID_MTPROP_FTP_RB);
          if ( !pIda->fMQSAvail )
          {
            SETCHECK_FALSE( hwnd, ID_MTPROP_MQS_RB);
            ENABLECTRL( hwnd, ID_MTPROP_MQSERVER_EF, FALSE );
            ENABLECTRL( hwnd, ID_MTPROP_QUEUEMGR_EF, FALSE );
            ENABLECTRL( hwnd, ID_MTPROP_INPQUEUE_EF, FALSE );
            ENABLECTRL( hwnd, ID_MTPROP_OUTQUEUE_EF, FALSE );
          }
          else
          {
            SETCHECK_TRUE( hwnd, ID_MTPROP_MQS_RB);
          } /* endif */
        }
        else if ( !pIda->fMQSAvail )
        {
          pIda->pMTProp->fUseFTP = TRUE;
          SETCHECK_FALSE( hwnd, ID_MTPROP_MQS_RB);
          SETCHECK_TRUE( hwnd, ID_MTPROP_FTP_RB);
          ENABLECTRL( hwnd, ID_MTPROP_FTP_RB, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_MQS_RB, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_MQSERVER_EF, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_QUEUEMGR_EF, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_INPQUEUE_EF, FALSE );
          ENABLECTRL( hwnd, ID_MTPROP_OUTQUEUE_EF, FALSE );
        }
        else
        {
          SETCHECK( hwnd, ID_MTPROP_FTP_RB, pIda->pMTProp->fUseFTP );
          SETCHECK( hwnd, ID_MTPROP_MQS_RB, !pIda->pMTProp->fUseFTP ); 
        } /* endif */

        // fill fields from properties
        SETTEXT( hwnd, ID_MTPROP_MQSERVER_EF,  pIda->pMTProp->szMQServer );
        SETTEXT( hwnd, ID_MTPROP_QUEUEMGR_EF,  pIda->pMTProp->szQueueMgr );
        SETTEXT( hwnd, ID_MTPROP_INPQUEUE_EF,  pIda->pMTProp->szServerInQueue );
        SETTEXT( hwnd, ID_MTPROP_OUTQUEUE_EF,  pIda->pMTProp->szServerOutQueue );
        SETTEXT( hwnd, ID_MTPROP_FTPSERVER_EF, pIda->pMTProp->szFTPServer );
        SETTEXT( hwnd, ID_MTPROP_FTPUSER_EF,   pIda->pMTProp->szFTPUserID );
        {
          CHAR szBuffer[MAX_MT_NAME];
          MTEncryptPassword( pIda->pMTProp->szFTPPassword , szBuffer, sizeof(szBuffer), FALSE ); 
          SETTEXT( hwnd, ID_MTPROP_FTPPASSWORD_EF, szBuffer );
        }
        SETTEXT( hwnd, ID_MTPROP_FTPPORT_EF, pIda->pMTProp->szFTPPort );        
        switch ( pIda->pMTProp->FTPMode )
        {
          case FTPMODE_SOCKSPROXY :
            SETCHECK_TRUE( hwnd, ID_MTPROP_FTPSOCKSPROXY_RB );  
            break;
          case FTPMODE_FTPPROXY :
            SETCHECK_TRUE( hwnd, ID_MTPROP_FTPPROXY_RB );  
            break;
          default:
          case FTPMODE_DIRECTACCESS :
            SETCHECK_TRUE( hwnd, ID_MTPROP_FTPDIRECTACCESS_RB );
            break;
        } /*endswitch */
        SETCHECK( hwnd, ID_MTPROP_FTPPASSIVEMODE_CHK, pIda->pMTProp->fPassiveMode );
        SETTEXT( hwnd, ID_MTPROP_FTPPROXYADDRESS, pIda->pMTProp->szProxyAddress );   
        SETTEXT( hwnd, ID_MTPROP_FTPPROXYPORT, pIda->pMTProp->szProxyPort );      
        pIda->fDlgInit = FALSE;
      }
      break;

    case WM_COMMAND:
      {
        SHORT sId = WMCOMMANDID( mp1, mp2 );

        switch ( sId )
        {
          case PID_PB_OK:
            {
             BOOL fOK = TRUE;

             pIda = ACCESSDLGIDA( hwnd, PMTPROPIDA );
             /****************************************************************/
             /* if mp2 == 1L we have to validate the page, if it is 0L we    */
             /* have to copy the content of the dialog back into the struct. */
             /****************************************************************/
             if ( mp2 == 1L )
             {
             }
             else
             {
               if ( fOK )
               {
                 QUERYTEXT( hwnd, ID_MTPROP_MQSERVER_EF, pIda->pMTProp->szMQServer );
                 UtlStripBlanks( pIda->pMTProp->szMQServer );
                 if ( pIda->pMTProp->szMQServer[0] == EOS )
                 {
                   UtlErrorHwnd( TQM_MANDATORY_VALUE_MISSING, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd);
                   fOK = FALSE;
                   SETFOCUS( hwnd, ID_MTPROP_MQSERVER_EF );
                 } /* endif */
               } /* endif */

               if ( fOK )
               {
                 QUERYTEXT( hwnd, ID_MTPROP_QUEUEMGR_EF, pIda->pMTProp->szQueueMgr );
                 UtlStripBlanks( pIda->pMTProp->szQueueMgr );
                 if ( pIda->pMTProp->szQueueMgr[0] == EOS )
                 {
                   UtlErrorHwnd( TQM_MANDATORY_VALUE_MISSING, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd);
                   fOK = FALSE;
                   SETFOCUS( hwnd, ID_MTPROP_QUEUEMGR_EF );
                 } /* endif */
               } /* endif */
               if ( fOK )
               {
                 QUERYTEXT( hwnd, ID_MTPROP_INPQUEUE_EF, pIda->pMTProp->szServerInQueue );
                 UtlStripBlanks( pIda->pMTProp->szServerInQueue );
                 if ( pIda->pMTProp->szServerInQueue[0] == EOS )
                 {
                   UtlErrorHwnd( TQM_MANDATORY_VALUE_MISSING, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd);
                   fOK = FALSE;
                   SETFOCUS( hwnd, ID_MTPROP_INPQUEUE_EF );
                 } /* endif */
               } /* endif */

               if ( fOK )
               {
                 QUERYTEXT( hwnd, ID_MTPROP_OUTQUEUE_EF, pIda->pMTProp->szServerOutQueue );
                 UtlStripBlanks( pIda->pMTProp->szServerOutQueue );
                 if ( pIda->pMTProp->szServerOutQueue[0] == EOS )
                 {
                   UtlErrorHwnd( TQM_MANDATORY_VALUE_MISSING, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd);
                   fOK = FALSE;
                   SETFOCUS( hwnd, ID_MTPROP_OUTQUEUE_EF);
                 } /* endif */
               } /* endif */

               // FTP flag
               if ( fOK )
               {
                 pIda->pMTProp->fUseFTP = QUERYCHECK( hwnd, ID_MTPROP_FTP_RB );
               } /* endif */

               // FTP server
               if ( fOK )
               {
                 QUERYTEXT( hwnd, ID_MTPROP_FTPSERVER_EF, pIda->pMTProp->szFTPServer );
                 UtlStripBlanks( pIda->pMTProp->szFTPServer );
                 if ( pIda->pMTProp->fUseFTP && (pIda->pMTProp->szFTPServer[0] == EOS) )
                 {
                   UtlErrorHwnd( TQM_MANDATORY_VALUE_MISSING, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd);
                   fOK = FALSE;
                   SETFOCUS( hwnd, ID_MTPROP_FTPSERVER_EF);
                 } /* endif */
               } /* endif */

               // FTP port
               if ( fOK )
               {
                 CHAR szBuffer[MAX_MT_NAME];
                 QUERYTEXT( hwnd, ID_MTPROP_FTPPORT_EF, szBuffer );
                 UtlStripBlanks( szBuffer );
                 
                 // check for numeric value
                 {
                   PSZ pszPos = szBuffer;
                   while ( *pszPos && fOK )
                   {
                     if ( !isdigit( *pszPos ) )
                     {
                       UtlErrorHwnd( TQM_NO_NUMERIC_VALUE, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd);
                       fOK = FALSE;
                       SETFOCUS( hwnd, ID_MTPROP_FTPPORT_EF );
                     } /* endif */
                     pszPos++;
                   } /*endwhile */
                 }
                 if ( fOK )
                 {
                   strcpy( pIda->pMTProp->szFTPPort, szBuffer );
                 } /* endif */
               } /* endif */

               // FTP user ID
               if ( fOK )
               {
                 QUERYTEXT( hwnd, ID_MTPROP_FTPUSER_EF, pIda->pMTProp->szFTPUserID );
                 UtlStripBlanks( pIda->pMTProp->szFTPUserID );
                 if ( pIda->pMTProp->fUseFTP && (pIda->pMTProp->szFTPUserID[0] == EOS) )
                 {
                   UtlErrorHwnd( TQM_MANDATORY_VALUE_MISSING, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd);
                   fOK = FALSE;
                   SETFOCUS( hwnd, ID_MTPROP_FTPUSER_EF );
                 } /* endif */
               } /* endif */

               // FTP password
               if ( fOK )
               {
                 CHAR szBuffer[MAX_MT_NAME];
                 QUERYTEXT( hwnd, ID_MTPROP_FTPPASSWORD_EF, szBuffer );
                 UtlStripBlanks( szBuffer );
                 if ( pIda->pMTProp->fUseFTP && (szBuffer[0] == EOS) )
                 {
                   UtlErrorHwnd( TQM_MANDATORY_VALUE_MISSING, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd);
                   fOK = FALSE;
                   SETFOCUS( hwnd, ID_MTPROP_FTPPASSWORD_EF );
                 }
                 else
                 {
                   MTEncryptPassword( szBuffer, pIda->pMTProp->szFTPPassword, sizeof(pIda->pMTProp->szFTPPassword),
                                      TRUE );
                 } /* endif */
               } /* endif */

               // connection mode
               if ( fOK )
               {
                 if ( QUERYCHECK( hwnd, ID_MTPROP_FTPSOCKSPROXY_RB ) )
                 {
                   pIda->pMTProp->FTPMode = FTPMODE_SOCKSPROXY;
                 }
                 else if ( QUERYCHECK( hwnd, ID_MTPROP_FTPPROXY_RB ) )
                 {
                   pIda->pMTProp->FTPMode = FTPMODE_FTPPROXY;
                 }
                 else
                 {
                   pIda->pMTProp->FTPMode = FTPMODE_DIRECTACCESS;
                 } /* endif */
               } /* endif */

               // passive flag
               if ( fOK )
               {
                 pIda->pMTProp->fPassiveMode = QUERYCHECK( hwnd, ID_MTPROP_FTPPASSIVEMODE_CHK );
               } /* endif */

               // proxy address
               if ( fOK )
               {
                 QUERYTEXT( hwnd, ID_MTPROP_FTPPROXYADDRESS, pIda->pMTProp->szProxyAddress );
                 UtlStripBlanks( pIda->pMTProp->szProxyAddress );
                 if ( pIda->pMTProp->fUseFTP && 
                      (pIda->pMTProp->FTPMode != FTPMODE_DIRECTACCESS) &&
                      (pIda->pMTProp->szProxyAddress[0] == EOS) )
                 {
                   UtlErrorHwnd( TQM_MANDATORY_VALUE_MISSING, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd);
                   fOK = FALSE;
                   SETFOCUS( hwnd, ID_MTPROP_FTPPROXYADDRESS);
                 } /* endif */
               } /* endif */

               // proxy port
               if ( fOK )
               {
                 CHAR szBuffer[MAX_MT_NAME];
                 QUERYTEXT( hwnd, ID_MTPROP_FTPPROXYPORT, szBuffer );
                 UtlStripBlanks( szBuffer );
                 
                 // check for numeric value
                 {
                   PSZ pszPos = szBuffer;
                   while ( *pszPos && fOK )
                   {
                     if ( !isdigit( *pszPos ) )
                     {
                       UtlErrorHwnd( TQM_NO_NUMERIC_VALUE, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd);
                       fOK = FALSE;
                       SETFOCUS( hwnd, ID_MTPROP_FTPPROXYPORT );
                     } /* endif */
                     pszPos++;
                   } /*endwhile */
                 }
                 if ( fOK )
                 {
                   strcpy( pIda->pMTProp->szProxyPort, szBuffer );
                 } /* endif */
               } /* endif */

               mResult = !fOK;
             } /* endif */
            }
            break;
          case ID_MTPROP_FTP_RB:
            {
              BOOL fChecked = QUERYCHECK( hwnd, ID_MTPROP_FTP_RB );
              if ( fChecked)
              {
                SETCHECK_FALSE( hwnd, ID_MTPROP_MQS_RB );
                ENABLECTRL( hwnd, ID_MTPROP_MQSERVER_EF,    FALSE );
                ENABLECTRL( hwnd, ID_MTPROP_QUEUEMGR_EF,    FALSE );
                ENABLECTRL( hwnd, ID_MTPROP_INPQUEUE_EF,    FALSE );
                ENABLECTRL( hwnd, ID_MTPROP_OUTQUEUE_EF,    FALSE );
                ENABLECTRL( hwnd, ID_MTPROP_FTPSERVER_EF,   TRUE );
                ENABLECTRL( hwnd, ID_MTPROP_FTPUSER_EF,     TRUE );
                ENABLECTRL( hwnd, ID_MTPROP_FTPPASSWORD_EF, TRUE );
                ENABLECTRL( hwnd, ID_MTPROP_FTPPORT_EF,     TRUE );        
                ENABLECTRL( hwnd, ID_MTPROP_FTPDIRECTACCESS_RB, TRUE );
                ENABLECTRL( hwnd, ID_MTPROP_FTPSOCKSPROXY_RB, TRUE );  
                ENABLECTRL( hwnd, ID_MTPROP_FTPPROXY_RB, TRUE );       
                ENABLECTRL( hwnd, ID_MTPROP_FTPPASSIVEMODE_CHK, TRUE );
                ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYADDRESS, TRUE );   
                ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYPORT, TRUE );      
              } /* endif */
            }
            break;

          case ID_MTPROP_MQS_RB:
            {
              BOOL fChecked = QUERYCHECK( hwnd, ID_MTPROP_MQS_RB );
              if ( fChecked)
              {
                SETCHECK_FALSE( hwnd, ID_MTPROP_FTP_RB );
                ENABLECTRL( hwnd, ID_MTPROP_MQSERVER_EF,    TRUE );
                ENABLECTRL( hwnd, ID_MTPROP_QUEUEMGR_EF,    TRUE );
                ENABLECTRL( hwnd, ID_MTPROP_INPQUEUE_EF,    TRUE );
                ENABLECTRL( hwnd, ID_MTPROP_OUTQUEUE_EF,    TRUE );
                ENABLECTRL( hwnd, ID_MTPROP_FTPSERVER_EF,   FALSE );
                ENABLECTRL( hwnd, ID_MTPROP_FTPUSER_EF,     FALSE );
                ENABLECTRL( hwnd, ID_MTPROP_FTPPASSWORD_EF, FALSE );
                ENABLECTRL( hwnd, ID_MTPROP_FTPPORT_EF, FALSE );        
                ENABLECTRL( hwnd, ID_MTPROP_FTPDIRECTACCESS_RB, FALSE );
                ENABLECTRL( hwnd, ID_MTPROP_FTPSOCKSPROXY_RB, FALSE );  
                ENABLECTRL( hwnd, ID_MTPROP_FTPPROXY_RB, FALSE );       
                ENABLECTRL( hwnd, ID_MTPROP_FTPPASSIVEMODE_CHK, FALSE );
                ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYADDRESS, FALSE );   
                ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYPORT, FALSE );      
              } /* endif */
            }
            break;


          case ID_MTPROP_FTPDIRECTACCESS_RB:
            {
              BOOL fChecked = QUERYCHECK( hwnd, ID_MTPROP_FTPDIRECTACCESS_RB );
              pIda = ACCESSDLGIDA( hwnd, PMTPROPIDA );
              if ( fChecked)
              {
                SETCHECK_FALSE( hwnd, ID_MTPROP_FTPPROXY_RB );
                SETCHECK_FALSE( hwnd, ID_MTPROP_FTPSOCKSPROXY_RB );
                ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYADDRESS, FALSE );   
                ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYPORT, FALSE );      
                if ( !pIda->fDlgInit ) SETTEXT( hwnd, ID_MTPROP_FTPPROXYPORT, "" );
              } /* endif */
            }
            break;

          case ID_MTPROP_FTPPROXY_RB:
            {
              BOOL fChecked = QUERYCHECK( hwnd, ID_MTPROP_FTPPROXY_RB );
              pIda = ACCESSDLGIDA( hwnd, PMTPROPIDA );
              if ( fChecked)
              {
                SETCHECK_FALSE( hwnd, ID_MTPROP_FTPDIRECTACCESS_RB );
                SETCHECK_FALSE( hwnd, ID_MTPROP_FTPSOCKSPROXY_RB );
                if ( !pIda->fPropertyDlg )
                {
                  ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYADDRESS, TRUE );   
                  ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYPORT, TRUE );      
                  if ( !pIda->fDlgInit ) SETTEXT( hwnd, ID_MTPROP_FTPPROXYPORT, "21" );
                } /* endif */
              } /* endif */
            }
            break;

          case ID_MTPROP_FTPSOCKSPROXY_RB:
            {
              BOOL fChecked = QUERYCHECK( hwnd, ID_MTPROP_FTPSOCKSPROXY_RB );
              pIda = ACCESSDLGIDA( hwnd, PMTPROPIDA );
              if ( fChecked)
              {
                SETCHECK_FALSE( hwnd, ID_MTPROP_FTPDIRECTACCESS_RB );
                SETCHECK_FALSE( hwnd, ID_MTPROP_FTPPROXY_RB );
                if ( !pIda->fPropertyDlg )
                {
                  ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYADDRESS, TRUE );   
                  ENABLECTRL( hwnd, ID_MTPROP_FTPPROXYPORT, TRUE );      
                  if ( !pIda->fDlgInit ) SETTEXT( hwnd, ID_MTPROP_FTPPROXYPORT, "1080" );
                } /* endif */
              } /* endif */
            }
            break;

        } /* endswitch */
      }
      break;

      case WM_HELP:
         /*************************************************************/
         /* pass on a HELP_WM_HELP request                            */
         /*************************************************************/
//         EqfDisplayContextHelp( ((LPHELPINFO) mp2)->hItemHandle,
//                                &hlpsubtblAnaPropMisc[0] );
         mResult = TRUE;  // message processed
         break;

    default:
       mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return mResult;
};


INT_PTR CALLBACK MTPROP_STAT_DLGPROC
(
   HWND hwnd,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PMTPROPIDA     pIda;

  switch ( msg )
  {
    case WM_INITDLG:
      pIda = (PMTPROPIDA)PVOIDFROMMP2( mp2 );
      ANCHORDLGIDA( hwnd, pIda );

      // disable all controls
      ENABLECTRL( hwnd, ID_MTPROP_STATUS_EF, FALSE );
      ENABLECTRL( hwnd, ID_MTPROP_CREATED_EF, FALSE );
      ENABLECTRL( hwnd, ID_MTPROP_SEND_EF, FALSE );
      ENABLECTRL( hwnd, ID_MTPROP_TRANSLATED_EF, FALSE );
      ENABLECTRL( hwnd, ID_MTPROP_RECEIVED_EF, FALSE );
      ENABLECTRL( hwnd, ID_MTPROP_MERGED_EF, FALSE );
      ENABLECTRL( hwnd, ID_MTPROP_NUMOFWORDS_EF, FALSE );

      // fill fields from properties
      MTStatusString( pIda->pMTProp->lStatus, pIda->pMTProp->szStatus, pIda->szBuffer );
      SETTEXT( hwnd, ID_MTPROP_STATUS_EF, pIda->szBuffer );

      if ( pIda->pMTProp->lJobCreateDate )
      {
        LONG2DATETIME( pIda->pMTProp->lJobCreateDate, pIda->szBuffer );
        SETTEXT( hwnd, ID_MTPROP_CREATED_EF, pIda->szBuffer );
      } /* endif */

      if ( pIda->pMTProp->lSendDate )
      {
        LONG2DATETIME( pIda->pMTProp->lSendDate, pIda->szBuffer );
        SETTEXT( hwnd, ID_MTPROP_SEND_EF, pIda->szBuffer );
      } /* endif */

      if ( pIda->pMTProp->lTranslatedDate )
      {
        LONG2DATETIME( pIda->pMTProp->lTranslatedDate, pIda->szBuffer );
        SETTEXT( hwnd, ID_MTPROP_TRANSLATED_EF, pIda->szBuffer );
      } /* endif */

      if ( pIda->pMTProp->lReceivedDate )
      {
        LONG2DATETIME( pIda->pMTProp->lReceivedDate, pIda->szBuffer );
        SETTEXT( hwnd, ID_MTPROP_RECEIVED_EF, pIda->szBuffer );
      } /* endif */

      if ( pIda->pMTProp->lMergedDate )
      {
        LONG2DATETIME( pIda->pMTProp->lMergedDate, pIda->szBuffer );
        SETTEXT( hwnd, ID_MTPROP_MERGED_EF, pIda->szBuffer );
      } /* endif */

      if ( pIda->pMTProp->lNumOfWords )
      {
        sprintf( pIda->szBuffer, "%ld", pIda->pMTProp->lNumOfWords );
        SETTEXT( hwnd, ID_MTPROP_NUMOFWORDS_EF, pIda->szBuffer );
      } /* endif */



      break;

    case WM_COMMAND:
      {
        SHORT sId = WMCOMMANDID( mp1, mp2 );
        switch ( sId )
        {
          case PID_PB_OK:
            {
             pIda = ACCESSDLGIDA( hwnd, PMTPROPIDA );
             /****************************************************************/
             /* if mp2 == 1L we have to validate the page, if it is 0L we    */
             /* have to copy the content of the dialog back into the struct. */
             /****************************************************************/
             if ( mp2 == 1L )
             {
             }
             else
             {
             } /* endif */
            }
            break;
        } /* endswitch */
      }
      break;

      case WM_HELP:
         /*************************************************************/
         /* pass on a HELP_WM_HELP request                            */
         /*************************************************************/
//         EqfDisplayContextHelp( ((LPHELPINFO) mp2)->hItemHandle,
//                                &hlpsubtblAnaPropMisc[0] );
         mResult = TRUE;  // message processed
         break;

    default:
       mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return mResult;
}


MRESULT MTPropCommand
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
   PMTPROPIDA pIda;                         // ptr to dialog IDA
   BOOL fOK = TRUE;
   mp2;
   switch ( WMCOMMANDID( mp1, mp2 ) )
   {
      case PID_PB_HELP:
        UtlInvokeHelp();
        break;

      case PID_PB_OK:
        // get the active settings
        {
          USHORT nItem = 0;
          // access our IDA and folder properties
          pIda = ACCESSDLGIDA( hwndDlg, PMTPROPIDA);

          // disable ok, cancel and help button
          ENABLECTRL( hwndDlg, PID_PB_OK, FALSE );
          ENABLECTRL( hwndDlg, PID_PB_CANCEL, FALSE );
          ENABLECTRL( hwndDlg, PID_PB_HELP, FALSE );

          // issue command to all active dialog pages
          while ( pIda->hwndPages[nItem] && fOK )
          {
            PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ nItem ],
                                                 DWL_DLGPROC );

            switch ( nItem )
            {
              // general settings
              case 0:
                fOK =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                               PID_PB_OK, 0L);
                break;

              // server settings
              case 1:
                fOK =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                               PID_PB_OK, 0L);
                break;
            } /* endswitch */

            if ( !fOK )
            {
              // switch to page causing the error
              HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_MTPROP_TABCTRL );
              ULONG ulCurrent = TabCtrl_GetCurSel( hwndTabCtrl );
              TabCtrl_SetCurSel( hwndTabCtrl, nItem );
              ShowWindow( pIda->hwndPages[ulCurrent], SW_HIDE );
              ShowWindow( pIda->hwndPages[nItem], SW_SHOW );
            } /* endif */

            nItem++;
         } /* endwhile */

          // save properties (in case of new dialog) and last used values
          if ( fOK )
          {
            if ( !pIda->fPropertyDlg )
            {
              if ( pIda->pMTProp->fUseFTP )
              {
                strcpy( pIda->pMTProp->szMTEngineDLL, MT_ENGINE_FTP );

                // for FTP transfer we use the job name as message ID
                // as file name and message ID
                Utlstrccpy( pIda->pMTProp->szMessageID, UtlGetFnameFromPath( pIda->szPropName ), DOT );
              }
              else
              {
                strcpy( pIda->pMTProp->szMTEngineDLL, MT_ENGINE_MQS );
              } /* endif */


              UtlWriteFile( pIda->szPropName, sizeof(MTJOBPROP), pIda->pMTProp, FALSE );

              UtlMakeEQFPath( pIda->szPath, NULC, PROPERTY_PATH, NULL );
              strcat( pIda->szPath, BACKSLASH_STR );
              strcat( pIda->szPath, MTLASTUSED );

              MTPropsToDefaults( pIda->szPath, pIda->pMTProp );
            } /* endif */
          } /* endif */

          // store current values in ANA<->MT IF structure
          if ( fOK )
          {
            if ( !pIda->fPropertyDlg )
            {
              if ( pIda->pMTProp->fFuzzyFilter )
              {
                pIda->pAnaData->usLargeFuzzy  = pIda->pMTProp->usLargeFuzzy;
                pIda->pAnaData->usMediumFuzzy = pIda->pMTProp->usMediumFuzzy;
                pIda->pAnaData->usSmallFuzzy  = pIda->pMTProp->usSmallFuzzy;
              }
              else
              {
                pIda->pAnaData->usLargeFuzzy  = 0;
                pIda->pAnaData->usMediumFuzzy = 0;
                pIda->pAnaData->usSmallFuzzy  = 0;
              } /* endif */

              if ( pIda->pMTProp->fMaxWordsFilter )
              {
                pIda->pAnaData->usWordLimit   = pIda->pMTProp->usMaxWords;
              }
              else
              {
                pIda->pAnaData->usWordLimit   = 0;
              } /* endif */
            } /* endif */
          } /* endif */

          if ( fOK )
          {
             //close dialog
             WinPostMsg( hwndDlg, WM_EQF_CLOSE, MP1FROMSHORT( TRUE ), NULL );
          }
          else
          {
            // enable ok, cancel and help button
            ENABLECTRL( hwndDlg, PID_PB_OK, TRUE );
            ENABLECTRL( hwndDlg, PID_PB_CANCEL, TRUE );
            ENABLECTRL( hwndDlg, PID_PB_HELP, TRUE );
          } /* endif fOk */
        }
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
} /* end of MTPropCommand */

MRESULT MTPropertySheetNotification
(
  HWND hwndDlg,
  WPARAM  mp1,
  LPARAM  mp2
)
{
  NMHDR * pNMHdr;
  ULONG       ulTabCtrl;
  MRESULT      mResult = FALSE;
  PMTPROPIDA     pIda;
  pNMHdr = (LPNMHDR)mp2;
  mp1;
  switch ( pNMHdr->code )
  {
    case TCN_SELCHANGE:
      pIda = ACCESSDLGIDA(hwndDlg, PMTPROPIDA);
      if ( pIda )
      {
        TC_ITEM Item;
        HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_MTPROP_TABCTRL );
        ulTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, ulTabCtrl, &Item );
        ulTabCtrl = (USHORT)Item.lParam;
        ShowWindow( pIda->hwndPages[ ulTabCtrl ], SW_SHOW );
      } /* endif */
      break;
    case TCN_SELCHANGING:
      pIda = ACCESSDLGIDA( hwndDlg, PMTPROPIDA );
      if ( pIda )
      {
        /**************************************************************/
        /* Issue a direct call to the appropriate dialog proc with    */
        /* WM_COMMAND, ID_TB_PROP_SET_PB and the second parameter set */
        /* to 1L to force only consistency checking                   */
        /**************************************************************/
        TC_ITEM Item;
        PFNWP pfnWp;
        HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_MTPROP_TABCTRL );
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
        TOOLTIPTEXT *pToolTipText = (TOOLTIPTEXT *) mp2;
        if ( pToolTipText )
        {
          TC_ITEM Item;
          HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_MTPROP_TABCTRL );
          memset( &Item, 0, sizeof(Item) );
          Item.mask = TCIF_PARAM;
          TabCtrl_GetItem( hwndTabCtrl, pToolTipText->hdr.idFrom, &Item );
          switch ( (SHORT)Item.lParam )
          {
            case 0:      // first page
//            LOADSTRING( hab, hResMod, IDS_MTPROP_TTIP_GENERAL, pToolTipText->szText );
              strcpy( pToolTipText->szText, "General machine translation settings" );
              break;
            case 1:      // second page
//              LOADSTRING( hab, hResMod, IDS_MTPROP_TTIP_SERVER, pToolTipText->szText );
              strcpy( pToolTipText->szText, "Server related machine translation settings" );
              break;
          } /* endswitch */
        } /* endif */
      }
      break;
    default:
      break;
  } /* endswitch */
  return mResult;
} /* end of function MTPropertySheetNotification */

// load MT job property defaults from text file
BOOL MTLoadPropsFromDefaults
(
  PSZ         pszDefaults,             // full name of file containing defaults
  PMTJOBPROP  pProps                   // properties being filled
)
{
  BOOL fFileFound = TRUE;
  BOOL fOK = TRUE;
  FILE *hInFile = NULL;
  static CHAR chLine[1024];

  hInFile = fopen( pszDefaults, "r" );
  if ( hInFile == NULL )
  {
    fOK = FALSE;
    fFileFound = FALSE;
  } /* endif */

  if ( fOK )
  {
    fgets( chLine, sizeof(chLine) - 1, hInFile );
    while ( !feof( hInFile ) )
    {
      BOOL fLineOK = TRUE;
      PSZ pszData = NULL;
      PSZ pszKey = chLine;

      if ( (chLine[0] == '*') || (chLine[0] == ';') )
      {
        fLineOK = FALSE;
      } /* endif */

      // strip any trailing CRLF
      if ( fLineOK )
      {
        int iLen = strlen(chLine);
        while ( (iLen > 0) && ((chLine[iLen-1] == CR) || (chLine[iLen-1] == LF)) )
        {
          iLen--;
          chLine[iLen] = EOS;
        } /* endwhile */
      }

      // isolate key and data
      if ( fLineOK )
      {
        pszData = strchr( pszKey, '=' );
        if ( pszData != NULL )
        {
          *pszData = EOS;
          pszData++;
          UtlStripBlanks( pszKey );
          UtlStripBlanks( pszData );
        }
        else
        {
          fLineOK = FALSE;
        } /* endif */
      } /* endif */

      if ( fLineOK )
      {
        if ( stricmp( pszKey, "MQSERVER" ) == 0 )
        {
          strncpy( pProps->szMQServer, pszData, sizeof(pProps->szMQServer) );
        }
        else if ( stricmp( pszKey, "QUEUEMGR" ) == 0 )
        {
          strncpy( pProps->szQueueMgr, pszData, sizeof(pProps->szQueueMgr) );
        }
        else if ( stricmp( pszKey, "INQUEUE" ) == 0 )
        {
          strncpy( pProps->szServerInQueue, pszData, sizeof(pProps->szServerInQueue) );
        }
        else if ( stricmp( pszKey, "OUTQUEUE" ) == 0 )
        {
          strncpy( pProps->szServerOutQueue, pszData, sizeof(pProps->szServerOutQueue) );
        }
        else if ( stricmp( pszKey, "SENDEREMAIL" ) == 0 )
        {
          strncpy( pProps->szSenderEMail, pszData, sizeof(pProps->szSenderEMail) );
        }
        else if ( stricmp( pszKey, "SUBJAREA" ) == 0 )
        {
          strncpy( pProps->szSubjArea, pszData, sizeof(pProps->szSubjArea) );
        }
        else if ( stricmp( pszKey, "AVAILSUBJAREA" ) == 0 )
        {
          strncpy( pProps->szAvailSubjArea, pszData, sizeof(pProps->szAvailSubjArea) );
        }
        else if ( stricmp( pszKey, "MAXWORDSCHECKED" ) == 0 )
        {
          pProps->fMaxWordsFilter = (*pszData == '1');
        }
        else if ( stricmp( pszKey, "MAXWORDS" ) == 0 )
        {
          pProps->usMaxWords = (USHORT)atoi( pszData );
        }
        else if ( stricmp( pszKey, "SMALLFUZZY" ) == 0 )
        {
          pProps->usSmallFuzzy = (USHORT)atoi( pszData );
        }
        else if ( stricmp( pszKey, "FUZZYCHECKED" ) == 0 )
        {
          pProps->fFuzzyFilter = (*pszData == '1');
        }
        else if ( stricmp( pszKey, "MEDIUMFUZZY" ) == 0 )
        {
          pProps->usMediumFuzzy = (USHORT)atoi( pszData );
        }
        else if ( stricmp( pszKey, "LARGEFUZZY" ) == 0 )
        {
          pProps->usLargeFuzzy = (USHORT)atoi( pszData );
        }
        else if ( stricmp( pszKey, "FTPSERVER" ) == 0 )
        {
          strncpy( pProps->szFTPServer, pszData, sizeof(pProps->szFTPServer) );
        }
        else if ( stricmp( pszKey, "FTPUSERID" ) == 0 )
        {
          strncpy( pProps->szFTPUserID, pszData, sizeof(pProps->szFTPUserID) );
        }
        else if ( stricmp( pszKey, "FTPPASSWORD" ) == 0 )
        {
          strncpy( pProps->szFTPPassword, pszData, sizeof(pProps->szFTPPassword) );
        }
        else if ( stricmp( pszKey, "USEMQS" ) == 0 )
        {
          pProps->fUseFTP = (*pszData == '1');
        }
        else if ( stricmp( pszKey, "FTPPORT" ) == 0 )
        {
          strncpy( pProps->szFTPPort, pszData, sizeof(pProps->szFTPPort)-1 );
        }
        else if ( stricmp( pszKey, "PROXYPORT" ) == 0 )
        {
          strncpy( pProps->szProxyPort, pszData, sizeof(pProps->szProxyPort)-1 );
        }
        else if ( stricmp( pszKey, "PROXYSERVER" ) == 0 )
        {
          strncpy( pProps->szProxyAddress, pszData, sizeof(pProps->szProxyAddress)-1 );
        }
        else if ( stricmp( pszKey, "CONNECTION" ) == 0 )
        {
          if ( stricmp( pszData, "FTPPROXY" ) == 0 )
          {
            pProps->FTPMode = FTPMODE_FTPPROXY;
          }
          else if ( stricmp( pszData, "SOCKSPROXY" ) == 0 )
          {
            pProps->FTPMode = FTPMODE_SOCKSPROXY;
          }
          else
          {
            pProps->FTPMode = FTPMODE_DIRECTACCESS;
          } /* endif */
        }
        else if ( stricmp( pszKey, "PASSIVEMODE" ) == 0 )
        {
          pProps->fPassiveMode = (*pszData == '1');
        }
        else
        {
          // ignore mispelled/unknown key
        } /* endif */
      } /* endif */
      fgets( chLine, sizeof(chLine) - 1, hInFile );
    } /* endwhile */

  } /* endif */

  if ( hInFile ) fclose( hInFile );

  return( fFileFound );
} /* end of function MTLoadPropsFromDefaults */

// get checked subject areas or set check (called recursively)
void MTGetOrSetCheckedItems
(
  HWND        hwndTV,                       // handle of tree view control
  HTREEITEM   hItem,                        // handle of item to start with
  PSZ         pszSubjArea,                  // area for selected subject area
  BOOL        fSetCheck                     // TRUE = set check marks
)
{
  CHAR  szSubject[50];                   // buffer for subject text
  do
  {
    // get item data
    TVITEM tvItem;                       // buffer for item data
    BOOL   fChecked = FALSE;             // TRUE = item is checked

    // get check mark for this item
    tvItem.mask = TVIF_HANDLE | TVIF_STATE | TVIF_TEXT | TVIF_CHILDREN;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;
    tvItem.cchTextMax = sizeof(szSubject);
    tvItem.pszText = szSubject;
    TreeView_GetItem( hwndTV, &tvItem);
    fChecked = (BOOL)(tvItem.state >> 12) - 1;

    // add to subect area list or set items check state
    if ( fSetCheck )
    {
      // set check mark if current subject is contained in subject area list
      PSZ pszSubject = pszSubjArea;
      BOOL fFound = FALSE;

      while ( *pszSubject && !fFound )
      {
        PSZ pszEnd = strchr( pszSubject, ',' );
        if ( pszEnd ) *pszEnd = EOS;
        if ( strcmp( pszSubject, szSubject ) == 0 )
        {
          TVITEM tvItem;

          // set check mark for this item
          memset( &tvItem, 0, sizeof(tvItem) );
          tvItem.mask      = TVIF_HANDLE | TVIF_STATE;
          tvItem.hItem     = hItem;
          tvItem.stateMask = TVIS_STATEIMAGEMASK;
          tvItem.state     = INDEXTOSTATEIMAGEMASK(2);
          TreeView_EnsureVisible( hwndTV, hItem );
          TreeView_SetItem( hwndTV, &tvItem);
          fFound = TRUE;
        } /* endif */
        if ( pszEnd )
        {
          *pszEnd = ',';
          pszSubject = pszEnd + 1;
        }
        else
        {
          pszSubject += strlen(pszSubject);
        } /* endif */
      } /* endwhile */
    }
    else
    {
      // add to subject area list if checked
      if ( fChecked )
      {
        if ( *pszSubjArea == EOS )
        {
          strcpy( pszSubjArea, szSubject );
        }
        else
        {
          PMTJOBPROP pProp = NULL;
          int iLen = strlen(pszSubjArea) + strlen(szSubject) + 2;
          if ( iLen <= sizeof(pProp->szSubjArea) )
          {
            strcat( pszSubjArea, "," );
            strcat( pszSubjArea, szSubject );
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    // handle any childs of current item
    if ( tvItem.cChildren )
    {
      HTREEITEM hChild = TreeView_GetChild( hwndTV, hItem );
      if ( hChild ) MTGetOrSetCheckedItems( hwndTV, hChild, pszSubjArea, fSetCheck );
    } /* endif */

    // continue with next sibling
    hItem = TreeView_GetNextSibling( hwndTV, hItem );
  } while ( hItem );
}

// write MT job properties to defaults text file
BOOL MTPropsToDefaults
(
  PSZ         pszDefaults,             // full name of file receiving defaults
  PMTJOBPROP  pProps                   // properties being filled
)
{
  BOOL fOK = TRUE;
  FILE *hOutFile = NULL;
  static CHAR chLine[1024];

  hOutFile = fopen( pszDefaults, "w" );
  if ( hOutFile == NULL )
  {
    fOK = FALSE;
  } /* endif */

  if ( fOK )
  {
    fprintf( hOutFile, "MQSERVER=%s\n", pProps->szMQServer );
    fprintf( hOutFile, "QUEUEMGR=%s\n", pProps->szQueueMgr );
    fprintf( hOutFile, "INQUEUE=%s\n",  pProps->szServerInQueue );
    fprintf( hOutFile, "OUTQUEUE=%s\n", pProps->szServerOutQueue );
    fprintf( hOutFile, "SUBJAREA=%s\n", pProps->szSubjArea );
    fprintf( hOutFile, "AVAILSUBJAREA=%s\n", pProps->szAvailSubjArea );
    fprintf( hOutFile, "MAXWORDSCHECKED=%d\n", pProps->fMaxWordsFilter );
    fprintf( hOutFile, "FUZZYCHECKED=%d\n", pProps->fFuzzyFilter );
    if ( pProps->szSenderEMail[0] ) fprintf( hOutFile, "SENDEREMAIL=%s\n", pProps->szSenderEMail );
    if ( pProps->usMaxWords )    fprintf( hOutFile, "MAXWORDS=%u\n",    pProps->usMaxWords );
    if ( pProps->usSmallFuzzy )  fprintf( hOutFile, "SMALLFUZZY=%u\n",  pProps->usSmallFuzzy );
    if ( pProps->usMediumFuzzy ) fprintf( hOutFile, "MEDIUMFUZZY=%u\n", pProps->usMediumFuzzy );
    if ( pProps->usLargeFuzzy )  fprintf( hOutFile, "LARGEFUZZY=%u\n",  pProps->usLargeFuzzy );
    if ( pProps->szFTPServer[0] ) fprintf( hOutFile, "FTPSERVER=%s\n", pProps->szFTPServer );
    if ( pProps->szFTPUserID[0] ) fprintf( hOutFile, "FTPUSERID=%s\n", pProps->szFTPUserID );
    if ( pProps->szFTPPassword[0] ) fprintf( hOutFile, "FTPPASSWORD=%s\n", pProps->szFTPPassword );
    if ( pProps->fUseFTP ) fprintf( hOutFile, "USEFTP=%d\n", pProps->fUseFTP );
    if ( pProps->szFTPPort[0] ) fprintf( hOutFile, "FTPPORT=%s\n", pProps->szFTPPort );
    if ( pProps->szProxyPort[0] ) fprintf( hOutFile, "PROXYPORT=%s\n", pProps->szProxyPort );
    if ( pProps->szProxyAddress[0] ) fprintf( hOutFile, "PROXYSERVER=%s\n", pProps->szProxyAddress );
    switch ( pProps->FTPMode )
    {
      case FTPMODE_FTPPROXY:   fprintf( hOutFile, "CONNECTION=FTPPROXY\n" ); break;
      case FTPMODE_SOCKSPROXY: fprintf( hOutFile, "CONNECTION=SOCKSPROXY\n" ); break;
      default:                 fprintf( hOutFile, "CONNECTION=DIRECTACCESS\n" ); break;
    } /*endswitch */
    if ( pProps->fPassiveMode ) fprintf( hOutFile, "PASSIVEMODE=%d\n", pProps->fPassiveMode );
  } /* endif */

  if ( hOutFile ) fclose( hOutFile );

  return( fOK );
} /* end of function MTPropsToDefaults */


INT_PTR CALLBACK MTPROP_SUBJECT_DLGPROC
(
  HWND   hwndDlg,
  WINMSG message,
  WPARAM mp1,
  LPARAM mp2
)
{
  PMTPROPIDA    pIda;                 // ptr to instance data area
  MRESULT       mResult = FALSE;      // dialog procedure return value

  switch ( message)
  {
    case WM_EQF_QUERYID:
      HANDLEQUERYID( ID_MTPROP_SUBJECT_DLG, mp2 );
      break;

    case WM_INITDLG:
      // anchor IDA
      pIda = (PMTPROPIDA)mp2;
      ANCHORDLGIDA( hwndDlg, pIda);

      // fill subarea tree control
      if ( pIda->pszSubAreaList )
      {
        TVINSERTSTRUCT stInsert;
        HWND hwndTV = GetDlgItem( hwndDlg, ID_MTPROP_AVAILSUBJECT_TREE );
        PSZ pszStart = pIda->pszSubAreaList;
        HTREEITEM hCurRoot = TVI_ROOT;
        HTREEITEM hCurItem = TVI_ROOT;
        int iCurIndent = 0;

        typedef struct _SUBJINSERTSTACK
        {
          HTREEITEM hRoot;
          int iIndent;
        } SUBJINSERTSTACK;

        SUBJINSERTSTACK Stack[20];
        int iStackItem = 0;

        while ( *pszStart )
        {
          PSZ pszCurEnd = pszStart;
          while ( (*pszCurEnd != CR) && (*pszCurEnd != LF) && *pszCurEnd ) pszCurEnd++;
          if ( *pszCurEnd )
          {
            int iIndent = 0;
            CHAR chTemp = *pszCurEnd;
            *pszCurEnd = EOS;
            while ( *pszStart == ' ' )
            {
              iIndent++;
              pszStart++;
            } /* endwhile */

            if ( iIndent > iCurIndent )
            {
              // push current values on stack, use current item as root item
              Stack[iStackItem].hRoot = hCurRoot;
              Stack[iStackItem].iIndent = iCurIndent;
              iStackItem++;
              hCurRoot = hCurItem;
              iCurIndent = iIndent;
            }
            else if ( iIndent < iCurIndent )
            {
              if ( iStackItem ) iStackItem--;
              while ( iStackItem && Stack[iStackItem].iIndent > iIndent ) iStackItem--;

              hCurRoot   = Stack[iStackItem].hRoot;
              iCurIndent = Stack[iStackItem].iIndent;
            } /* endif */

            memset( &stInsert, 0, sizeof(stInsert) );
            stInsert.hParent = hCurRoot;
            stInsert.hInsertAfter = TVI_LAST;
            stInsert.item.mask = TVIF_TEXT;
            stInsert.item.pszText = pszStart;
            stInsert.item.cchTextMax = 20;  // dummy value, not used when setting items
            hCurItem = TreeView_InsertItem( hwndTV, &stInsert );

            *pszCurEnd = chTemp;
            while ( (*pszCurEnd == CR) || (*pszCurEnd == LF) ) pszCurEnd++;
          } /* endif */
          pszStart = pszCurEnd;
        } /* endwhile */

         // postpone check of selected subject areas
         PostMessage( hwndDlg, WM_EQF_PROCESSTASK, 0, 0 );
      } /* endif */

      // fill selected subject area listbox
      {
        PSZ pszSubject = pIda->pMTProp->szAvailSubjArea;
        while ( *pszSubject )
        {
          PSZ pszEnd = strchr( pszSubject, ',' );
          if ( pszEnd ) *pszEnd = EOS;

          // insert current subject area
          INSERTITEM( hwndDlg, ID_MTPROP_SELECTSUBJECT_LB, pszSubject );

          // restore subject area end delimiter and go to next subject area
          if ( pszEnd )
          {
            *pszEnd = ',';
            pszSubject = pszEnd + 1;
          }
          else
          {
            pszSubject += strlen(pszSubject);
          } /* endif */
        } /* endwhile */
      }

      // disable some of the controls
      ENABLECTRL( hwndDlg, ID_MTPROP_REMOVESUBJECT_PB, FALSE );

      // position dialog window
      UtlCheckDlgPos( hwndDlg, TRUE );
      SETFOCUS( hwndDlg, ID_MTPROP_AVAILSUBJECT_TREE );

      mResult = MRFROMSHORT(TRUE);   // leave the focus where we put it

      break;

    case WM_EQF_PROCESSTASK:
      // handle request to check subject areas in tree control
      {
        HWND      hwndTV = GetDlgItem( hwndDlg, ID_MTPROP_AVAILSUBJECT_TREE );
        HTREEITEM hItem = TreeView_GetRoot( hwndTV );
        pIda = ACCESSDLGIDA(hwndDlg, PMTPROPIDA);
        MTGetOrSetCheckedItems( hwndTV, hItem, pIda->pMTProp->szAvailSubjArea, TRUE );
        TreeView_EnsureVisible( hwndTV, hItem );
      }
      break;

    case WM_COMMAND:
       switch ( WMCOMMANDID( mp1, mp2 ) )
       {
         case PID_PB_HELP:
            UtlInvokeHelp();
            break;

          case ID_MTPROP_COPYSUBJECT_PB:
            {
              HWND hwndTV = GetDlgItem( hwndDlg, ID_MTPROP_AVAILSUBJECT_TREE );
              HTREEITEM hRoot = TreeView_GetRoot( hwndTV );

              pIda = ACCESSDLGIDA(hwndDlg, PMTPROPIDA);

              // get list of checked subjectareas
              pIda->szSubjectBuffer[0] = EOS;
              MTGetOrSetCheckedItems( hwndTV, hRoot, pIda->szSubjectBuffer, FALSE );

              // add subject areas to selected subject area listbox
              {
                PSZ pszSubject = pIda->szSubjectBuffer;
                while ( *pszSubject )
                {
                  int iItem;
                  PSZ pszEnd = strchr( pszSubject, ',' );
                  if ( pszEnd ) *pszEnd = EOS;

                  // insert current subject area if not in listbox already
                  iItem = SEARCHITEM( hwndDlg, ID_MTPROP_SELECTSUBJECT_LB, pszSubject );
                  if ( iItem < 0 )
                  {
                    INSERTITEM( hwndDlg, ID_MTPROP_SELECTSUBJECT_LB, pszSubject );
                  } /* endif */

                  // restore subject area end delimiter and go to next subject area
                  if ( pszEnd )
                  {
                    *pszEnd = ',';
                    pszSubject = pszEnd + 1;
                  }
                  else
                  {
                    pszSubject += strlen(pszSubject);
                  } /* endif */
                } /* endwhile */
              }
            }
            break;
          case ID_MTPROP_REMOVESUBJECT_PB:
            {
              int iItem = LIT_FIRST;
              while ( (iItem = (int)QUERYNEXTSELECTION( hwndDlg, ID_MTPROP_SELECTSUBJECT_LB,
                                                   (SHORT)(iItem) )) != LIT_NONE )
              {
                DELETEITEM( hwndDlg, ID_MTPROP_SELECTSUBJECT_LB, iItem );
                iItem = ( iItem == 0 ) ? LIT_FIRST : iItem - 1;
              } /* endwhile */
              ENABLECTRL( hwndDlg, ID_MTPROP_REMOVESUBJECT_PB, FALSE );
            }
            break;
          case PID_PB_OK:
            {
              int iItem, iMaxItem;
              PSZ  pszSubject;

              // get IDA pointer
              pIda = ACCESSDLGIDA(hwndDlg, PMTPROPIDA);

              // fill subject area list
              pIda->pMTProp->szAvailSubjArea[0] = EOS;
              pszSubject = pIda->pMTProp->szAvailSubjArea;
              iItem = 0;
              iMaxItem = QUERYITEMCOUNT( hwndDlg, ID_MTPROP_SELECTSUBJECT_LB );
              while ( iItem < iMaxItem )
              {
                if ( pIda->pMTProp->szAvailSubjArea[0] ) *pszSubject++ = ',';
                QUERYITEMTEXTL( hwndDlg, ID_MTPROP_SELECTSUBJECT_LB,
                                iItem, pszSubject, 80 );
                pszSubject += strlen(pszSubject);
                iItem++;
              } /* endwhile */

              // close dialog
              EndDialog( hwndDlg, TRUE );
            }
            break;
          case PID_PB_CANCEL:
          case DID_CANCEL:
            EndDialog( hwndDlg, FALSE );
            break;
          case ID_MTPROP_SELECTSUBJECT_LB:
            if ( HIWORD(mp1) == LBN_SELCHANGE )
            {
              int iSelItems = SendDlgItemMessage( hwndDlg, ID_MTPROP_SELECTSUBJECT_LB,
                                                  LB_GETSELCOUNT, 0, 0 );
              ENABLECTRL( hwndDlg, ID_MTPROP_REMOVESUBJECT_PB, (iSelItems != 0) );
            } /* endif */
            break;
          default:
            mResult = WinDefDlgProc( hwndDlg, message, mp1, mp2 );
            break;
       }
       break;

    default:
      mResult = WinDefDlgProc( hwndDlg, message, mp1, mp2 );

  }
  return( mResult );
}


void MTFillSubjectAreaList
(
  HWND hwndList,
  PSZ  pszSubject,
  PSZ  pszSelSubjects
)
{
  LVITEM ListItem;
  int iNextItem = 0;

  ListView_DeleteAllItems( hwndList );

  while ( *pszSubject )
  {
    int iItem;
    PSZ pszEnd = strchr( pszSubject, ',' );
    if ( pszEnd ) *pszEnd = EOS;

    // insert current subject area
    memset( &ListItem, 0, sizeof(ListItem) );
    ListItem.mask = LVIF_TEXT;
    ListItem.pszText = pszSubject;
    ListItem.iItem = iNextItem++;
    iItem = ListView_InsertItem( hwndList, &ListItem );

    // check subject area if contained in selected subject area list
    {
      PSZ pszSelSubject = pszSelSubjects;
      BOOL fFound = FALSE;

      while ( *pszSelSubject && !fFound )
      {
        PSZ pszEnd = strchr( pszSelSubject, ',' );
        if ( pszEnd ) *pszEnd = EOS;
        if ( strcmp( pszSelSubject, pszSubject ) == 0 )
        {
          ListView_SetCheckState( hwndList, iItem, TRUE );
          fFound = TRUE;
        } /* endif */
        if ( pszEnd )
        {
          *pszEnd = ',';
          pszSelSubject = pszEnd + 1;
        }
        else
        {
          pszSelSubject += strlen(pszSelSubject);
        } /* endif */
      } /* endwhile */
    }

    // restore subject area end delimiter and go to next subject area
    if ( pszEnd )
    {
      *pszEnd = ',';
      pszSubject = pszEnd + 1;
    }
    else
    {
      pszSubject += strlen(pszSubject);
    } /* endif */
  } /* endwhile */
} /* end of function MTFillSubjectAreaList */

// function to encrypt or decrypt the FTP password
BOOL MTEncryptPassword
( 
  PSZ         pszPassword,             // in: password or encrypted string
  PSZ         pszBuffer,               // out: encrypted string or password 
  int         iBufLen,                 // size of output buffer
  BOOL        fEncrypt                 // TRUE = encrypt, FALSE = decrypt
)
{
  #define HEXTONUM( char ) (( char < 'A' ) ? ( char - '0' ) : ( char - 'A' + 10) )

  BOOL fOK = TRUE;
  PSZ pszKey = "TranslationManager, Copyright 2004"; 

  if ( fEncrypt )
  {
    BYTE bLastValue = 0;
    int iLen = strlen(pszPassword);
    int i = 0;
    for ( i = 0; ((i < iLen) && (iBufLen > 2)); i++ )
    {
      BYTE bValue = (BYTE)(pszPassword[i] + bLastValue);
      bValue = (BYTE)(bValue + (pszKey[i] * (i+1) * 3));
      bLastValue = bValue;
      sprintf( pszBuffer, "%2.2X", bValue );
      pszBuffer += 2;
      iBufLen -= 2;
    } /* endfor */
    if ( i != iLen )
    {
      // not completely encrypted
      fOK = FALSE;
    } /* endif */
    *pszBuffer = EOS;
  }
  else
  {
    int iLen = strlen(pszPassword);
    int i = 0;
    if ( (iLen % 2) != 0 )
    {
      // length must be a multiple of 2
      fOK = FALSE;
    }
    else
    {
      BYTE bLastValue = 0;
      while ( fOK && *pszPassword )
      {
        if ( isxdigit(pszPassword[0]) && isxdigit(pszPassword[1]) )
        {
          // convert to value
          BYTE bTemp = 0;
          BYTE bValue = (BYTE) HEXTONUM( toupper(pszPassword[0]) );
          bValue = bValue << 4;
          bValue |= HEXTONUM( toupper(pszPassword[1]) );
          bTemp = bValue;

          // decrypt
          bValue = (BYTE)(bValue - (pszKey[i] * (i+1) * 3));
          bValue = (BYTE)(bValue - bLastValue);
          bLastValue = bTemp;

          // store in buffer
          *pszBuffer++ = bValue;

          // next char
          pszPassword += 2;
          i++;
        }
        else
        {
          fOK = FALSE;
        } /* endif */
      } /*endwhile */
      *pszBuffer = EOS;
    } /* endif */
  } /* endif */
  return( fOK );
} /* end of function MTEncryptPassword */
