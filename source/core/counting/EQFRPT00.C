//+----------------------------------------------------------------------------+
//|EQFRPT00.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2015, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author: Michael Sekinger                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Dialog procedure to get input for Counting Report              |
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
//+----------------------------------------------------------------------------+


#define INCL_EQF_DLGUTILS         // set output file dialog
#define INCL_EQF_FOLDER           // folder list and document list functions

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_TP               // public translation processor functions


#include "eqf.h"                  // general TranslationManager include file
#include "eqfhlog.h"              // defines and structures of history log file


#include "eqfdde.h"
#include "eqfrpt.h"               // public include file for report
#include "eqfrpt.id"              // private id's for report


#include "eqftai.h"               // Private include file for Text Analysis
#include <eqfiana1.id>            // analysis dialog ids

#include "eqfrpt00.h"             // private include file for report
#include "eqfrpt03.h"             // private include file for report

#include "SHLOBJ.H"                  // folder browse function


// function prototypes
static VOID     InitCountingReportDlg    (HWND, LPARAM);
static VOID     CountingReportWM_COMMAND (HWND, WPARAM, LPARAM);
static MRESULT  CountingReportWM_CONTROL (HWND, SHORT, SHORT);

static VOID     InitCountingReport1Dlg    (HWND, LPARAM);
static VOID     CountingReport1WM_COMMAND (HWND, WPARAM, LPARAM);
static MRESULT  CountingReport1WM_CONTROL (HWND, SHORT, SHORT);

static VOID     InitCountingREPORT2Dlg    (HWND, LPARAM);
static VOID     CountingREPORT2WM_COMMAND (HWND, WPARAM, LPARAM);
static MRESULT  CountingREPORT2WM_CONTROL (HWND, SHORT, SHORT);


static VOID     InitCountingREPORT4Dlg    (HWND, LPARAM);
static VOID     CountingREPORT4WM_COMMAND (HWND, WPARAM, LPARAM);
static MRESULT  CountingREPORT4WM_CONTROL (HWND, SHORT, SHORT);

// functions for handling of format combobox
void RptFormatFillCB( HWND hwnd, int Id );
USHORT RptFormatGetSelected( HWND hwnd, int Id );
void RptFormatSelect( HWND hwnd, int Id, USHORT usFormat );

static VOID     HandleCreateButton       (HWND);
void RptFillReportOptions
( 
  HWND     hwnd,
  PRPTIDA  pRptIda,
  USHORT   usReport,
  USHORT   usOption
);

BOOL RptAdjustReportFileExtension( PSZ pszFileName, USHORT usFormat );

extern HELPSUBTABLE hlpsubtblCntRptDlg1[];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////Parameters for Final fact Sheet  //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//+----------------------------------------------------------------------------+
//|External function   RPT_ATOF                                                |
//+----------------------------------------------------------------------------+
//|Function name:                                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  MRESULT                                                   |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//+----------------------------------------------------------------------------+

BOOL RPT_ATOF_ZERO (PSZ  pszString)
{
  int    n_period = 0;
  int    n_non_null = 0;
  char   ch;

  while ((ch = *pszString++) != EOS )
  {
    if (ch == '.')  n_period++;
    else if (ch != '0')  n_non_null++;
  } // end while

  return(n_period<=1 && n_non_null==0);

}// end of function RPT_ATOF_ZERO





float RPT_ATOF
(
PSZ     pszString,
int *   Valid_Number  // pointer to string to be converted
)
{
  float   f ;

  * Valid_Number = 1;

  if (RPT_ATOF_ZERO(pszString))
  {
    return 0.0;
  }
  else
  {
    f = (float)atof(pszString);
    if (f<=0.0)
    {
      * Valid_Number = 0 ;
      return 0.0;
    }
    else
    {
      return f;
    } // end if



  } // end if



} /* end of function RPT_ATOF */



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// PROPERTY SHEET///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    HandleCreateButton                                        |
//+----------------------------------------------------------------------------+
//|Function call:    HandleCreateButton (hwnd)                                 |
//+----------------------------------------------------------------------------+
//|Description:      Handles the Count button of the dialog procedure          |
//|                  EQFREPORTDLGPROC                                          |
//+----------------------------------------------------------------------------+
//|Parameters:       HWND hwnd   dialog handle                                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  VOID                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

static
VOID HandleCreateButtonProp (HWND hwnd)
{
  PRPTIDA pRptIda;                 // pointer to instance area RPTIDA
  CHAR    szReplace[20] = {0};     // replace string from error message
  PSZ     pszReplace = szReplace;  // pointer to replace string
  HFILE   hfFileHandle;            // DOS file handle
  USHORT  usAction = 0;            // UtlOpen output
  USHORT  usRc = 0;                // return code
  USHORT  usRc1 = 0;               // returncode from strnicmp
  USHORT  usRc2 = 0;               // returncode from strnicmp
  USHORT  usResponse = 0;          // return from UtlError
  USHORT  usIndex = 0;             // index of current drive
  BOOL    fOk = TRUE;              // error indicator

  // get access to ida
  pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);

  if (fOk)
  {
    // get target language from folder properties
    strcpy( pRptIda->pRpt->szTargetLang, pRptIda->ppropFolder->szTargetLang );


    if (pRptIda->pRpt->fRptFile)   // file checkbox is selected
    {
      // isolate drive letter for error messages
      if (pRptIda->pRpt->szRptOutputFile[1] == COLON )  // filespec contains a drive
      {
        // save drive to count structured
        pRptIda->pRpt->szRptDriveLetter[0] = pRptIda->pRpt->szRptOutputFile[0];
        pRptIda->pRpt->szRptDriveLetter[1] = EOS;
      }
      else   // filespec contains no drive
      {
        // get current drive
        usIndex = UtlGetDriveList( (PBYTE)pRptIda->szDrives);

        // save drive to count structured
        pRptIda->pRpt->szRptDriveLetter[0] = pRptIda->szDrives[usIndex];
        pRptIda->pRpt->szRptDriveLetter[1] = EOS;

        // prepend drive letter and colon to output file name
        memmove (&pRptIda->pRpt->szRptOutputFile[2],
                 pRptIda->pRpt->szRptOutputFile,
                 (strlen (pRptIda->pRpt->szRptOutputFile)+1));

        pRptIda->pRpt->szRptOutputFile[0] = pRptIda->pRpt->szRptDriveLetter[0];
        pRptIda->pRpt->szRptOutputFile[1] = COLON;
      } // end if

      // convert drive letter to uppercase for error message
      pRptIda->pRpt->szRptDriveLetter[0] =
      (CHAR)toupper (pRptIda->pRpt->szRptDriveLetter[0]);

      // get system path
      UtlMakeEQFPath (pRptIda->szSysPath, NULC, SYSTEM_PATH, NULL);

      // append backslash to system path
      strcat (pRptIda->szSysPath, "\\");

      // compare if target path contains as first directory the system path
      usRc1 = (USHORT)strnicmp (pRptIda->pRpt->szRptOutputFile + 2, pRptIda->szSysPath + 2,
                        strlen(pRptIda->szSysPath) - 2);
      usRc2 = (USHORT)strnicmp (pRptIda->pRpt->szRptOutputFile + 3, pRptIda->szSysPath + 3,
                        strlen(pRptIda->szSysPath) - 2 );

      // if target path contains as first directory the system path
      if (usRc1 == 0 || usRc2 == 0)
      {
        // display error message that this is not allowed
        strcpy (pszReplace, pRptIda->szSysPath);
        UtlError (ERROR_EQF_PATH_INVALID, MB_CANCEL, 1, &pszReplace, EQF_ERROR );

        fOk = FALSE; // stop further processing
      } // end if

      if (fOk)
      {
        // check filename
        usRc = UtlOpen (pRptIda->pRpt->szRptOutputFile,  // filename
                        &hfFileHandle,                   // file handle
                        &usAction,                       // action taken by Open
                        0L,                              // file size
                        FILE_NORMAL,                     // attribute  read/write
                        OPEN_ACTION_FAIL_IF_EXISTS |     // fail if exist
                        OPEN_ACTION_CREATE_IF_NEW,
                        OPEN_ACCESS_READONLY |           // open for read only
                        OPEN_SHARE_DENYREADWRITE,        // deny any other access
                        0L,                              // reserved, must be 0
                        FALSE );                         // do no error handling

        switch (usRc)  // rc from UtlOpen
        {
          case ERROR_NETWORK_ACCESS_DENIED:
          case ERROR_PATH_NOT_FOUND :         // path does not exist
          case ERROR_FILENAME_EXCED_RANGE :   // no valid filename
            // switch to general tab if not active
            {
              HWND hwndTabCtrl = GetDlgItem( hwnd, ID_ANA_PROP_TABCTRL );
              ULONG  ulCurrent = TabCtrl_GetCurSel( hwndTabCtrl );
              if ( ulCurrent != 0 )
              {
                TabCtrl_SetCurSel( hwndTabCtrl, 0 );
                ShowWindow( pRptIda->hwndPages[ulCurrent], SW_HIDE );
                ShowWindow( pRptIda->hwndPages[0], SW_SHOW );
              } /* endif */
            }

            // set cursor to entryfiled
            SETFOCUS( hwnd, DID_RPT_FILENAME_EF );

            // display error message 
            if ( usRc == ERROR_FILENAME_EXCED_RANGE )
            {
              // display error message that filename is no valid
              pszReplace = pRptIda->pRpt->szRptOutputFile;
              UtlError (ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1,
                        &pszReplace, EQF_WARNING);
            }
            else
            {
              UtlError (ERROR_REPPATH_NOT_EXIST, MB_CANCEL, 0, (PSZ *) NULP, EQF_WARNING);
            } /* endif */

            // set cursor to entryfield
            SETFOCUS( pRptIda->hwndPages[0], DID_RPT_FILENAME_EF );

            fOk = FALSE; // stop further processing
            break;

          case ERROR_NOT_READY :              // disk not ready
            // display error message disk not ready
            pszReplace = pRptIda->pRpt->szRptDriveLetter;
            usResponse = UtlError (ERROR_NOT_READY_MSG, MB_RETRYCANCEL, 1,
                                   &pszReplace, EQF_ERROR);

            if (usResponse == MBID_RETRY)
            {
              WinPostMsg (hwnd, WM_COMMAND, MP1FROMSHORT (DID_RPT_CREATE_PB), NULL);
            } // end if

            fOk = FALSE;  // stop further processing
            break;

          case NO_ERROR :                     // filename ok
            // save filename to folder properties
            strcpy (pRptIda->ppropFolder->szRptOutputFile, pRptIda->pRpt->szRptOutputFile);

            // close file, do not handle  error
            UtlClose (hfFileHandle, FALSE);

            // delete file, do not handle  error
            UtlDelete (pRptIda->pRpt->szRptOutputFile, 0L, FALSE);
            break;

          case ERROR_FILE_EXISTS :            // file exists
          case ERROR_OPEN_FAILED :            // file exists
            // display error message that file exists
            strcpy (pRptIda->ppropFolder->szRptOutputFile, pRptIda->pRpt->szRptOutputFile);
            pszReplace = pRptIda->pRpt->szRptOutputFile;
            usResponse = UtlError (ERROR_FILE_EXISTS_ALREADY, MB_YESNO | MB_DEFBUTTON2,
                                   1, &pszReplace, EQF_QUERY);

            switch (usResponse)
            {
              case MBID_YES :   // delete existing file
                // delete output file
                usRc = UtlDelete (pRptIda->pRpt->szRptOutputFile, 0L, TRUE);
                if (usRc)
                {
                  fOk = FALSE;
                } // end if
                break;

              case MBID_NO :    // do not delete existing file
                fOk = FALSE;    // stop further processing
                break;
            } // endswitch
            break;

          default :
            // display standard error message for all cases
            // not checked for explicitly
            {
              CHAR chNum[ 10 ];
              pszReplace = chNum;
              itoa (usRc, chNum, 10);
              usResponse = UtlError (ERROR_GENERAL_DOS_ERROR_MSG, MB_CANCEL,
                                     1, &pszReplace, EQF_ERROR);
              fOk = FALSE;
            }
            break;
        } // end switch
      } // end if
    } // end if
  } // end if

  if (fOk)
  {
    // send WM_CLOSE message to destroy the dialog and start the
    // count instance
    // if fOk == FALSE the dialog is not removed
    WinPostMsg (hwnd, WM_EQF_CLOSE, MP1FROMSHORT(TRUE), NULL);
  } // end if

} // end of HandleCreateButtonProp





//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     RPTPropertySheetNotification                             |
//+----------------------------------------------------------------------------+
//|Function call:     RPTPropertySheetNotifiaction( hwndDlg, mp1, mp2);        |
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
MRESULT RptPropertySheetNotification
(
HWND hwndDlg,
WPARAM  mp1,
LPARAM  mp2
)
{
  NMHDR * pNMHdr;
  USHORT       usTabCtrl;
  MRESULT      mResult = FALSE;
  PRPTIDA     pIda;
  pNMHdr = (LPNMHDR)mp2;

  mp1;

  switch ( pNMHdr->code )
  {
    case TCN_SELCHANGE:
      pIda = ACCESSDLGIDA(hwndDlg, PRPTIDA);
      if ( pIda )
      {
        TC_ITEM Item;
        HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_ANA_PROP_TABCTRL );
        usTabCtrl = (USHORT)TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, usTabCtrl, &Item );
        usTabCtrl = (USHORT)Item.lParam;
        ShowWindow( pIda->hwndPages[ usTabCtrl ], SW_SHOW );
      } /* endif */
      break;
    case TCN_SELCHANGING:
      pIda = ACCESSDLGIDA( hwndDlg, PRPTIDA );
      if ( pIda )
      {
        /**************************************************************/
        /* Issue a direct call to the appropriate dialog proc with    */
        /* WM_COMMAND, ID_TB_PROP_SET_PB and the second parameter set */
        /* to 1L to force only consistency checking                   */
        /**************************************************************/
        TC_ITEM Item;
        PFNWP pfnWp;
        HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_ANA_PROP_TABCTRL );
        usTabCtrl = (USHORT)TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, usTabCtrl, &Item );
        usTabCtrl = (USHORT)Item.lParam;
        pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ usTabCtrl ], DWL_DLGPROC );

        mResult = pfnWp( pIda->hwndPages[usTabCtrl], WM_COMMAND,
                         PID_PB_OK, 1L);
        if ( mResult )
        {
          /************************************************************/
          /* stick on the side                                        */
          /* we have to post the request again since one of the system*/
          /* routines thinks that we still want to change the page..  */
          /************************************************************/
          WinPostMsg( hwndDlg, TCM_SETCURSEL, usTabCtrl, 0L );
        } /* endif */
        ShowWindow( pIda->hwndPages[ usTabCtrl ], SW_HIDE );
      } /* endif */
      break;
    case TTN_NEEDTEXT:
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        TOOLTIPTEXT *pToolTipText = (TOOLTIPTEXT *) mp2;
        if ( pToolTipText )
        {
          TC_ITEM Item;
          HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_ANA_PROP_TABCTRL );
          memset( &Item, 0, sizeof(Item) );
          Item.mask = TCIF_PARAM;
          TabCtrl_GetItem( hwndTabCtrl, pToolTipText->hdr.idFrom, &Item );
          switch ( (SHORT)Item.lParam )
          {
            case 0:      // first page
              LOADSTRING( hab, hResMod, IDS_RPTPROP_TTIP_GENERAL,
                          pToolTipText->szText );
              break;
            case 1:      // second page
              LOADSTRING( hab, hResMod, IDS_RPTPROP_TTIP_PROFILE,
                          pToolTipText->szText );
              break;
            case 2:      // third page
              LOADSTRING( hab, hResMod, IDS_ANAPROP_TTIP_AUTSUBST,
                          pToolTipText->szText );
              break;
            case 3:      // fourth page
              LOADSTRING( hab, hResMod, IDS_ANAPROP_TTIP_LISTS,
                          pToolTipText->szText );
            case 4:      // fifth page
              LOADSTRING( hab, hResMod, IDS_ANAPROP_TTIP_LISTS,
                          pToolTipText->szText );
            case 5:      // fifth page
              LOADSTRING( hab, hResMod, IDS_ANAPROP_TTIP_LISTS,
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
} /* end of function AnapropertySheetNotification */







//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CntPropertySheetLoad                                     |
//+----------------------------------------------------------------------------+
//|Function call:     CntPropertySheetLoad( hwndDlg, mp2 );                    |
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
BOOL CntPropertySheetLoad
(
HWND hwnd,
PRPTIDA     pRptIda
)
{
  BOOL      fOk = TRUE;
  TC_ITEM   TabCtrlItem;
  USHORT    nItem = 0;
  HWND      hwndTabCtrl;
  HINSTANCE hInst;
  CHAR      szBuffer[80];
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);


  if ( fOk )
  {
    RECT rect;

    // remember adress of user area
    hInst = GETINSTANCE( hwnd );
    hwndTabCtrl = GetDlgItem( hwnd, ID_ANA_PROP_TABCTRL );
    pRptIda->hwndTabCtrl = hwndTabCtrl;
    GetClientRect( hwndTabCtrl, &rect );
    TabCtrl_AdjustRect( hwndTabCtrl, FALSE, &rect );

    // leave some additional space at top
    rect.top += 20;
    MapWindowPoints( hwndTabCtrl, hwnd, (POINT *) &rect, 2 );


    TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM;

    // -----------------------------------------------------------------
    //
    // create the appropriate TAB control and load the associated dialog
    //
    // -----------------------------------------------------------------

    //
    // IDS_ANAPROP_TAB_GENERAL
    //

    LOADSTRING( hab, hResMod, IDS_RPTPROP_TAB_GENERAL , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pRptIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_RPTPROP_GENERAL_DLG ),
                       hwnd,
                       RPTPROP_GENERAL_DLGPROC,
                       (LPARAM)pRptIda );

    SetWindowPos( pRptIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pRptIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pRptIda->hwndPages[nItem] );
    nItem++;


    //
    // IDS_ANAPROP_TAB_PROFILE
    //

    LOADSTRING( hab, hResMod, IDS_RPTPROP_TAB_PROFILE, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pRptIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_RPTPROP_PROFILE_DLG ),
                       hwnd,
                       RPTPROP_PROFILE_DLGPROC,
                       (LPARAM)pRptIda );

    SetWindowPos( pRptIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pRptIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pRptIda->hwndPages[nItem] );
    nItem++;



    //
    // IDS_ANAPROP_TAB_SETTINGS
    //

    LOADSTRING( hab, hResMod, IDS_RPTPROP_TAB_SETTINGS, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pRptIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_RPTPROP_SETTINGS_DLG ),
                       hwnd,
                       RPTPROP_SETTINGS_DLGPROC,
                       (LPARAM)pRptIda );

    SetWindowPos( pRptIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pRptIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pRptIda->hwndPages[nItem] );
    nItem++;


    //
    // IDS_ANAPROP_TAB_FACTSHEET
    //

    LOADSTRING( hab, hResMod, IDS_RPTPROP_TAB_FACTSHEET, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pRptIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_RPTPROP_FACTSHEET_DLG ),
                       hwnd,
                       RPTPROP_FACTSHEET_DLGPROC,
                       (LPARAM)pRptIda );

    SetWindowPos( pRptIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pRptIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pRptIda->hwndPages[nItem] );
    nItem++;

    //
    // IDS_ANAPROP_TAB_MORE
    //

    LOADSTRING( hab, hResMod, IDS_RPTPROP_TAB_MORE , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pRptIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_RPTPROP_MORE_DLG ),
                       hwnd,
                       RPTPROP_MORE_DLGPROC,
                       (LPARAM)pRptIda );

    SetWindowPos( pRptIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pRptIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pRptIda->hwndPages[nItem] );
    nItem++;

    //
    // IDS_ANAPROP_TAB_SHIPM
    //

    LOADSTRING( hab, hResMod, IDS_RPTPROP_TAB_SHIPM , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pRptIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_RPTPROP_SHIPM_DLG ),
                       hwnd,
                       RPTPROP_SHIPM_DLGPROC,
                       (LPARAM)pRptIda );

    SetWindowPos( pRptIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pRptIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pRptIda->hwndPages[nItem] );
    nItem++;
  } /* endif */


  // -----------------------------------------------------------------
  //
  // hide all dialog pages but the first one
  //
  // -----------------------------------------------------------------

  if ( fOk )
  {
    int i = 1;
    while ( pRptIda->hwndPages[i] )
    {
      ShowWindow( pRptIda->hwndPages[i], SW_HIDE );
      i++;
    } /* endwhile */
  } /* endif */

  if ( !fOk )
  {
    POSTEQFCLOSE( hwnd, FALSE );
  } /* endif */

  return fOk;
}



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     RptPropCommand                                           |
//+----------------------------------------------------------------------------+
//|Function call:     AnaPropCommand( hwnd, mp1, mp2);                         |
//+----------------------------------------------------------------------------+
//|Description:       Handle WM_COMMAND message of property sheet dialog       |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwnd   handle of the dialog                         |
//|                   WPARAM  mp1    message parameter 1                       |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
MRESULT RptPropCommand
(
HWND hwnd,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
  PRPTIDA pRptIda;                         // ptr to dialog IDA
  BOOL fOk = TRUE;
  INT     nItem;

  mp2;

  switch ( WMCOMMANDID( mp1, mp2 ) )
  {

    case DID_RPT_CREATE_PB:

      pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);

      // issue command to all active dialog pages
      nItem = 0;
      while ( pRptIda->hwndPages[nItem] && fOk )
      {
        PFNWP pfnWp = (PFNWP) GetWindowLong( pRptIda->hwndPages[ nItem ],
                                             DWL_DLGPROC );

        switch ( nItem )
        {
          // general  settings
          case 0:
            fOk =  !pfnWp( pRptIda->hwndPages[nItem], WM_COMMAND,
                           PID_PB_OK, 0L);
            break;

            // profile settings
          case 1:
            fOk =  !pfnWp( pRptIda->hwndPages[nItem], WM_COMMAND,
                           PID_PB_OK, 0L);
            break;

            // profile settings
          case 2:
            fOk =  !pfnWp( pRptIda->hwndPages[nItem], WM_COMMAND,
                           PID_PB_OK, 0L);
            break;


            // profile settings
          case 3:
            fOk =  !pfnWp( pRptIda->hwndPages[nItem], WM_COMMAND,
                           PID_PB_OK, 0L);
            break;


            // profile settings
          case 4:
            fOk =  !pfnWp( pRptIda->hwndPages[nItem], WM_COMMAND,
                           PID_PB_OK, 0L);
            break;

            // profile settings
          case 5:
            fOk =  !pfnWp( pRptIda->hwndPages[nItem], WM_COMMAND,
                           PID_PB_OK, 0L);
            break;



        } /* endswitch */
        nItem++;
      } /* endwhile */



      HandleCreateButtonProp (hwnd);
      break;



    case DID_RPT_CANCEL_PB:
    case DID_CANCEL:
      POSTEQFCLOSE( hwnd, FALSE );
      break;

    case DID_RPT_HELP_PB:
      UtlInvokeHelp();
      break;

    default:
      mResult = WinDefDlgProc( hwnd, WM_COMMAND, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );
} /* end of RptPropCommand */

//|Internal function   COUNTINGPROPDLGPROC                                     |
INT_PTR CALLBACK COUNTINGPROPDLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PRPTIDA  pRptIda;                      // dialog instance data area
  BOOL     fOk = TRUE;                   // internal O.K. flag
  PRPT    pRpt;                        // Report structure
  ULONG   ulErrorInfo = 0;  // error indicator from PRHA

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_RPTPROP_DLG, mp2 ); break;

    case WM_INITDLG:


      if (UtlAlloc ((PVOID *)&pRptIda, 0L, (ULONG)sizeof (RPTIDA), ERROR_STORAGE))
      {
        if (!ANCHORDLGIDA (hwnd, pRptIda))
        {
          fOk = FALSE; // set fOk to FALSE to stop dialog

          // display error message system error
          UtlError (0, MB_CANCEL, 0, NULL, SYSTEM_ERROR);
          // post message to close dialog, do not start report instance
          WinPostMsg (hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL);
        }
      }
      else
      {
        fOk = FALSE; // set fOk to FALSE to stop dialog

        // post message to destroy dialog, do not start report instance
        WinPostMsg (hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL);
      } // end if

      if (fOk)
      {
        // save pointer to report structure passed in mp2 to IDA
        pRptIda->pRpt = (PRPT) mp2;
        pRpt = pRptIda->pRpt;

        pRpt->fInitialLoad = TRUE;


/*
        // build name of folder properties
        // get system path
        UtlMakeEQFPath (pRptIda->szFolderPropName, NULC, SYSTEM_PATH, NULP);

        // append folder name to systempath
        strcat( pRptIda->szFolderPropName, "\\" );
        strcat( pRptIda->szFolderPropName, pRptIda->pRpt->szFolderName );
*/
        strcpy(pRptIda->szFolderPropName, pRptIda->pRpt->szParentObjName);

      }// end if



      if ( !fOk )
      {
        //--- close analysis dialog, FALSE means: - do not start analysis instance
        //Close_proc( hwnd, FALSE );
      }
      else
      {
        CntPropertySheetLoad( hwnd, pRptIda );
      } /* endif */
      mResult = DIALOGINITRETURN( mResult );


      break;


    case WM_COMMAND:
      mResult = RptPropCommand( hwnd, mp1, mp2 );
      break;

    case WM_NOTIFY:

      mResult = RptPropertySheetNotification( hwnd, mp1, mp2 );
      break;

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblCntRptDlg1[0] );
      mResult = TRUE;  // message processed
      break;
    case WM_EQF_CLOSE :
      // mp1 : TRUE -> start Counting Report window
      // mp2 : not used
      pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);  // get access to IDA
      if (pRptIda)
      {
        if (mp1)  // start Counting Report window
        {
          // SAVE and close folder properties
          // return of SaveProperties !0 ==> error
          // SaveProperties displayes a error message
          if (!SaveProperties (pRptIda->hpropFolder, &ulErrorInfo))
          {
            // SaveProperties returns no error => close properties
            if (pRptIda->hpropFolder) CloseProperties (pRptIda->hpropFolder,
                                                       PROP_FILE, &ulErrorInfo);
          }
          else // SaveProp. returns error => do not start count instance
          {
            // do not start Counting Report window
            mp1 = FALSE;
          } // end if
        }
        else  // !mp1 => do not start count instance
        {
          // QUIT and close folder properties
          if (pRptIda->hpropFolder) CloseProperties (pRptIda->hpropFolder,
                                                     PROP_QUIT,  &ulErrorInfo);
        } // end if

        // free IDA and set to NULL
        UtlAlloc ((PVOID *) &pRptIda, 0L, 0L, NOMSG);
        ANCHORDLGIDA (hwnd, NULL);
      } // end if

      DelCtrlFont(hwnd, DID_RPT_DOCUMENTS_LB);

      // destroy dialog pass flag in mp1, to tell the count handler if
      // count instance should be started or not
      // mp1 == FALSE : do not start count instance
      // mp1 == TRUE  : start count instance
      WinDismissDlg (hwnd, SHORT1FROMMP1(mp1));
      break;


    default:
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
} /* end of COUNTINGPROPDLGPROC */


//|Internal function  RPTPROP_GENERAL_DLGPROC                                  |
INT_PTR CALLBACK RPTPROP_GENERAL_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PRPTIDA  pRptIda;
  PRPT     pRpt;               // pointer to report data
  PSZ      pszTmp;             // tmp pointer to string
  INT      nItem;
  BOOL     fOk = TRUE;

  SHORT sNotification= WMCOMMANDCMD( mp1,mp2 );

  switch ( msg )
  {
    case WM_INITDLG:
      //-----------------------------------
      {
        SHORT   usNumber = 0;                // number of selected documents in folder
        PSZ     pszReplace;                  // pointer to string to be replaced
        ULONG   ulErrorInfo = 0;             // error number
        BOOL    fOk = TRUE;                  // error indicator
        SHORT   reportIndex = 0;             // index of current report
        SHORT   i,j ;                        // counter

        HWND hwndLB;

        pRptIda = (PRPTIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pRptIda );
        pRpt = pRptIda->pRpt;

        if (fOk)
        {
          // build name of folder properties
          // get system path
          UtlMakeEQFPath (pRptIda->szFolderPropName, NULC, SYSTEM_PATH, (PSZ) NULP);

          // append folder name to systempath
          strcat( pRptIda->szFolderPropName, "\\" );
          strcat( pRptIda->szFolderPropName, pRptIda->pRpt->szFolderName );

          if (!pRptIda->hpropFolder)
          {
            pRptIda->hpropFolder = OpenProperties (pRptIda->szFolderPropName, NULL,
                                                   PROP_ACCESS_READ, &ulErrorInfo);
            // from handler
            if ( pRptIda->hpropFolder == NULL )
            {
              // set fOk to FALSE to stop dialog
              fOk = FALSE;

              // display error message
              pszReplace = pRptIda->szFolderPropName;
              UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszReplace, EQF_ERROR);

              // post message to close dialog, do not start count instance
              WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL );

            } // end if
          } //end if
        } // end if


        if (fOk)
        {
          // set write access to properties
          SetPropAccess (pRptIda->hpropFolder, PROP_ACCESS_WRITE);

          // get pointer to folder properties and save it to ida
          pRptIda->ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd (pRptIda->hpropFolder);

          // use old last used output path if no new one available
          if ( pRptIda->ppropFolder->szRptOutputFile[0] == EOS )
          {
            strcpy( pRptIda->ppropFolder->szRptOutputFile, pRptIda->ppropFolder->szOldRptOutputFile );
          } /* endif */


          // get number of all documents of folder
          DELETEALLHWND(pRptIda->pRpt->hwndRptHandlerLB );  // delete all documents
          EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                           MP1FROMHWND(pRptIda->pRpt->hwndRptHandlerLB),
                           (LPARAM) pRptIda->pRpt->szFolderObjName );
          pRptIda->pRpt->usAllDocuments = QUERYITEMCOUNTHWND( pRptIda->pRpt->hwndRptHandlerLB );

          // documents selected in folder
          if (EqfQueryObjectStatus (EqfQueryActiveFolderHwnd()) & OBJ_FOCUS)
          {
            pRptIda->pRpt->fFolderSelected = FALSE;
            pRptIda->pRpt->usFillMode = WM_EQF_INSERTNAMES;

            DELETEALLHWND(pRptIda->pRpt->hwndRptHandlerLB );  // delete all documents

            pRptIda->pRpt->usFillMode = WM_EQF_QUERYSELECTEDNAMES;

            // insert selected documents in listbox
            EqfSend2Handler (FOLDERHANDLER, WM_EQF_QUERYSELECTEDNAMES,
                             MP1FROMHWND(pRptIda->pRpt->hwndRptHandlerLB ),
                             (LPARAM) pRptIda->pRpt->szParentObjName);
          }
          else   // folder selected in folder list
          {
            pRptIda->pRpt->fFolderSelected = TRUE;
            pRptIda->pRpt->usFillMode = WM_EQF_INSERTNAMES;

            // insert all documents in listbox
            DELETEALLHWND(pRptIda->pRpt->hwndRptHandlerLB );  // delete all documents
            EqfSend2Handler (FOLDERHANDLER, WM_EQF_INSERTNAMES,
                             MP1FROMHWND(pRptIda->pRpt->hwndRptHandlerLB ),
                             (LPARAM) pRptIda->pRpt->szParentObjName);
          } // end if

          // get number of documents in LB
          usNumber = QUERYITEMCOUNTHWND( pRptIda->pRpt->hwndRptHandlerLB );

          // update number RPT
          pRptIda->pRpt->usSelectedDocuments = usNumber;

          // remember document list box handle in our IDA
          pRptIda->hwndDocLB = GetDlgItem( hwnd, DID_RPT_DOCUMENTS_LB );

          // show documents in listbox of dialog
          {
            SHORT   sItem;                   // listbox item index
            PSZ     pszFolName =
            UtlGetFnameFromPath( pRptIda->pRpt->szFolderObjName );



            DELETEALL(hwnd,DID_RPT_DOCUMENTS_LB);
            // fill the visible document listbox with the long names of the
            // document listbox containing the actual (the short) document names
            sItem = QUERYITEMCOUNTHWND( pRptIda->pRpt->hwndRptHandlerLB );
            while ( sItem-- )
            {
              // get the document name
              QUERYITEMTEXTHWND( pRptIda->pRpt->hwndRptHandlerLB, sItem,
                                 pRptIda->szWorkString );

              // build document object name
              UtlMakeEQFPath( pRptIda->szDocObjName,
                              pRptIda->pRpt->szFolderObjName[0],
                              SYSTEM_PATH, pszFolName );
              strcat( pRptIda->szDocObjName, BACKSLASH_STR );
              strcat( pRptIda->szDocObjName, pRptIda->szWorkString );

              // get the long name for this document
              pRptIda->szLongName[0] = EOS;
              DocQueryInfo2( pRptIda->szDocObjName, NULL, NULL, NULL, NULL,
                             pRptIda->szLongName, NULL, NULL, FALSE );

              // add long name to visible document listbox
              if ( pRptIda->szLongName[0] != EOS )
              {
                OEMTOANSI( pRptIda->szLongName );
                INSERTITEM( hwnd, DID_RPT_DOCUMENTS_LB, pRptIda->szLongName );
              }
              else
              {
                INSERTITEM( hwnd, DID_RPT_DOCUMENTS_LB, pRptIda->szWorkString );
              } /* endif */
            } /* endwhile */
          }
        } // end if


        // Document listbox
        hwndLB = GetDlgItem(hwnd, DID_RPT_DOCUMENTS_LB);
//      SendMessage(hwndLB,LB_SETHORIZONTALEXTENT , (WPARAM)npixel, NULL);
        UtlSetHorzScrollingForLB(hwndLB);

        if (fOk)
        {
           HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

          // set CHK "Screen"
          SETCHECK_TRUE(hwnd, DID_RPT_SCREEN_CHK);     // CHK on
          ENABLECTRL (hwnd, DID_RPT_SCREEN_CHK, FALSE);  // disable CHK

          // set TEXT of Folder
          OEMSETTEXT( hwnd, DID_RPT_FOLDERNAME_TEXT, pRptIda->pRpt->szLongFolderName );

          // set entry filed length limit
          SETTEXTLIMIT (hwnd, DID_RPT_FILENAME_EF, MAX_PATH144-1);

          // set entry filed length limit
          SETTEXTLIMIT (hwnd, DID_RPT_DESCRIPTION_EF, MAX_DESCRIPTION-1);

          // select Report and Options in dependency of last used values
          // from folder prop.

          // fill CB "Report"
          CBDELETEALL(hwnd, DID_RPT_REPORT_CB);
          while (reportIndex < MAX_REPORTS)
          {
            LOADSTRING (hab, hResMod, SID_RPT_REPORT_1 + reportIndex++,
                        pRptIda->szWorkString);
            CBINSERTITEMEND( hwnd, DID_RPT_REPORT_CB, pRptIda->szWorkString);
          } // end while


          // fill CB "FORMAT"
          RptFormatFillCB( hwnd, DID_RPT_FORMAT_CB );

          if (pRpt->usReport<0 || pRpt->usReport>MAX_REPORTS)
          {
            pRpt->usReport = 0;
            pRptIda->ppropFolder->usReport = 0;
          }//end if


          // fill OPTIONS for reports
          RptFillReportOptions( hwnd, pRptIda, pRptIda->ppropFolder->usReport, 
                                pRptIda->ppropFolder->usOption[pRptIda->ppropFolder->usReport] );

          // set last used Report
          if (pRptIda->ppropFolder->usReport<MAX_REPORTS && pRptIda->ppropFolder->usReport>=0)
          {
            CBSELECTITEM (hwnd, DID_RPT_REPORT_CB, pRptIda->ppropFolder->usReport);
          }
          else
          {
            CBSELECTITEM (hwnd, DID_RPT_REPORT_CB, 0);
          }

          // set last used Format
          RptFormatSelect(hwnd, DID_RPT_FORMAT_CB, pRptIda->ppropFolder->usFormat );

          // post a WM_COMMAND message for the selection in the combobox
          //  WinPostMsg( hwnd, WM_COMMAND, (WPARAM)DID_RPT_REPORT_CB, (LPARAM)CBN_SELCHANGE );

          // set state of CHK file
          if ( pRptIda->ppropFolder->fRptFile )             // last used value of file
          {
            // checkbox is selected
            SETCHECK_TRUE(hwnd, DID_RPT_FILE_CHK );
          } // end if

          // enable / disable PB "Select..." and EF "Filename"  and "FILE FORMAT"
          ENABLECTRL (hwnd, DID_RPT_FILESELECT_PB, pRptIda->ppropFolder->fRptFile);
          ENABLECTRL (hwnd, DID_RPT_FILENAME_EF,   pRptIda->ppropFolder->fRptFile);
          ENABLECTRL (hwnd, DID_RPT_DESCRIPTION_EF, TRUE);
          ENABLECTRL (hwnd, DID_RPT_FORMAT_CB,    pRptIda->ppropFolder->fRptFile);
          ENABLECTRL (hwnd, DID_RPT_SETTINGS_PB,
                      ( ( (pRptIda->ppropFolder->usReport>=SUMMARY_COUNTING_REPORT) &&
                          (pRptIda->ppropFolder->usReport<COMBINED_REPORT)) ||
                        (pRptIda->ppropFolder->usReport==HISTORY_REPORT)   ));
          ENABLECTRL (hwnd, DID_RPT_SETTINGS1_PB,
                      (  (pRptIda->ppropFolder->usReport>=SUMMARY_COUNTING_REPORT)&&
                         (pRptIda->ppropFolder->usReport<COMBINED_REPORT) ));


          ENABLECTRL (hwnd, DID_RPT1_COLUMN3_CHK,
                      ( pRptIda->ppropFolder->usReport==SUMMARY_COUNTING_REPORT));
          ENABLECTRL (hwnd, DID_RPT1_COLUMN4_CHK,
                      ( pRptIda->ppropFolder->usReport==SUMMARY_COUNTING_REPORT));


          if ( pRptIda->ppropFolder->szRptOutputFile[0] != EOS )
          {
            SETTEXT (hwnd, DID_RPT_DESCRIPTION_EF, pRptIda->ppropFolder->szRptDescription);
          }
          else
          {
            SETTEXT (hwnd, DID_RPT_DESCRIPTION_EF, "");
          } /* end if */


          QUERYTEXT (hwnd, DID_RPT_FILENAME_EF, pRptIda->szWorkString);

          if (pRptIda->szWorkString[0] == EOS)
          {

            if ( pRptIda->ppropFolder->szRptOutputFile[0] == EOS )
            {
              // no last used value for entry filed exists
              // build last used value from active folder object name concatenated
              // with the default extension and dislplay it in entry field
              strncpy (pRptIda->ppropFolder->szRptOutputFile, pRptIda->pRpt->szFolderObjName, 3);
              strcat( pRptIda->ppropFolder->szRptOutputFile, pRptIda->pRpt->szLongFolderName );

              RptAdjustReportFileExtension( pRptIda->ppropFolder->szRptOutputFile, pRptIda->ppropFolder->usFormat );
            } // end if
            SETTEXT (hwnd, DID_RPT_FILENAME_EF, pRptIda->ppropFolder->szRptOutputFile);
          }//end if

          /*************************************************/
          // set options of all windows to report structure
          /*************************************************/

          pRpt->usReport   = pRptIda->ppropFolder->usReport;   // selected report
          pRpt->usFormat   = pRptIda->ppropFolder->usFormat;   // selected format
          pRpt->usOptions  = pRptIda->ppropFolder->usOption[pRptIda->ppropFolder->usReport];  // selected Options
          pRpt->usOption[0]= pRptIda->ppropFolder->usOption[0]; //History
          pRpt->usOption[1]= pRptIda->ppropFolder->usOption[1]; //Counting
          pRpt->usOption[2]= pRptIda->ppropFolder->usOption[2]; //Summary
          pRpt->usOption[3]= pRptIda->ppropFolder->usOption[3]; //Summary
          pRpt->usOption[4]= pRptIda->ppropFolder->usOption[4]; //Summary
          pRpt->usOption[5]= pRptIda->ppropFolder->usOption[5]; //Summary
          pRpt->usOption1  = pRptIda->ppropFolder->usOption1;  // selected Options
          pRpt->usOption2  = pRptIda->ppropFolder->usOption2;  // selected Options
          pRpt->usOption3  = pRptIda->ppropFolder->usOption3;  // selected Options
          pRpt->usOption4  = pRptIda->ppropFolder->usOption4;  // selected Options
          pRpt->usOption5  = pRptIda->ppropFolder->usOption5;  // selected Options

          if (pRptIda->ppropFolder->usOption21 > MAX2_OPTIONS1)
          {
            pRptIda->ppropFolder->usOption21 = 0;
          }
          if (pRptIda->ppropFolder->usOption22 > MAX2_OPTIONS2)
          {
            pRptIda->ppropFolder->usOption22 = 0;
          }

          pRpt->usOption21 = pRptIda->ppropFolder->usOption21; // selected Options
          pRpt->usOption22 = pRptIda->ppropFolder->usOption22; // selected Options

          pRpt->usColumns[0] = pRptIda->ppropFolder->usColumns[0];
          pRpt->usColumns[1] = pRptIda->ppropFolder->usColumns[1];
          pRpt->usColumns[2] = pRptIda->ppropFolder->usColumns[2];
          pRpt->usColumns[3] = pRptIda->ppropFolder->usColumns[3];

          // GQ: options "plausibility check" and "lost data force new shipment" are
          //     always inactive!
          pRpt->usColumns4[0] = FALSE; // pRptIda->ppropFolder->usColumns4[0];
          pRpt->usColumns4[2] = FALSE; // pRptIda->ppropFolder->usColumns4[2];

          pRpt->usColumns4[1] = pRptIda->ppropFolder->usColumns4[1];
          pRpt->usColumns4[3] = pRptIda->ppropFolder->usColumns4[3];
          pRpt->usColumns4[4] = pRptIda->ppropFolder->usColumns4[4];
          pRpt->usStandard = pRptIda->ppropFolder->usStandard;
          pRpt->Pay_per_Standard  = pRptIda->ppropFolder->Pay_per_Standard;
          pRpt->usCurrency = pRptIda->ppropFolder->usCurrency;
          strcpy(pRpt->szCurrency,szRptCurrency[pRpt->usCurrency]);

          if ( pRptIda->ppropFolder->First_Report_Factor == 0)
          {

            // initial set of factors for final fact sheet
            pRptIda->ppropFolder->First_Report_Factor = 1;

            for (i=0;i<MAX_PAYREPORT_COLUMNS;i++)
            {
              for (j=0;j<3;j++)
              {
                pRptIda->ppropFolder->Complexity_Factor[i][j] = 1.0;
                pRptIda->ppropFolder->Pay_Factor[i][j] = 1.0;

              }  //end for
            } //end for

          } /* end if */

          // Set complexity and pay factor for final fact sheet
          // according properties
          for (i=0;i<MAX_PAYREPORT_COLUMNS;i++)
          {
            for (j=0;j<3;j++)
            {
              pRpt->Complexity_Factor[i][j] = pRptIda->ppropFolder->Complexity_Factor[i][j];
              pRpt->Pay_Factor[i][j] = pRptIda->ppropFolder->Pay_Factor[i][j];

            }  //end for
          } //end for

          SetCtrlFnt (hwnd, GetCharSet(), DID_RPT_DOCUMENTS_LB, DID_RPT_FILENAME_EF );
          WinSetFocus (HWND_DESKTOP, hwnd);  // set focus on dialog

        } // end if


      }
      break;

    case WM_COMMAND:
      //-----------------------------------
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {


        case PID_PB_OK :
          {

            pRptIda = ACCESSDLGIDA( hwnd, PRPTIDA );

            pRpt = pRptIda->pRpt;

            // get current report, store string in RPT
            pRptIda->pRpt->szReport[0] = EOS;
            CBQUERYSELECTEDITEMTEXT (pRptIda->pRpt->usReport ,hwnd,
                                     DID_RPT_REPORT_CB, pRptIda->pRpt->szReport);

            // get current format, store string in RPT
            pRptIda->pRpt->usFormat = RptFormatGetSelected( hwnd, DID_RPT_FORMAT_CB );

            // get current option, store string in RPT
            CBQUERYSELECTEDITEMTEXT (pRptIda->pRpt->usOptions, hwnd,
                                     DID_RPT_OPTIONS_SPIN, pRptIda->pRpt->szOption);

            QUERYTEXT (hwnd, DID_RPT_DESCRIPTION_EF, pRptIda->pRpt->szRptDescription);
            strcpy (pRptIda->ppropFolder->szRptDescription, pRptIda->pRpt->szRptDescription);

            // get status of file checkbox and save it to folder properties
            // and count structure
            pRptIda->ppropFolder->fRptFile = QUERYCHECK (hwnd, DID_RPT_FILE_CHK);
            pRptIda->pRpt->fRptFile = pRptIda->ppropFolder->fRptFile;

            if (pRptIda->pRpt->fRptFile)   // file checkbox is selected
            {
              // get content of file entry field
              QUERYTEXT (hwnd, DID_RPT_FILENAME_EF, pRptIda->pRpt->szRptOutputFile);
              RptAdjustReportFileExtension( pRptIda->pRpt->szRptOutputFile, pRptIda->pRpt->usFormat );
            }//end if


          }

          break;


        case DID_RPT_FILESELECT_PB :  // PB "SELECT" pressed
          //---------------------------------------------------------

          {
            OPENFILENAME ofn;
            BOOL fOk = TRUE;

            TCHAR szTitle[64];
            TCHAR szT[256];
            PSZ   pszExtension;
            BOOL  fExtensionStripped = FALSE;


            static TCHAR szFilterLoad[] = TEXT("ASCII (*.CNT)\0*.CNT\0XML (*.XML)\0*.RTF\0HTML (*.HTM)\0*.HTM;*.HTML\0\0");


            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);

            ofn.lStructSize                 = sizeof(ofn);
            ofn.hInstance                   = (HINSTANCE)UtlQueryULong(QL_HAB);
            ofn.lpstrFilter                 = szFilterLoad;
            ofn.lpstrCustomFilter           = NULL;
            ofn.nMaxCustFilter              = 0;
            switch ( pRptIda->ppropFolder->usFormat )
            {
              case ASCII         : ofn.nFilterIndex = 1; break;
              case XMLFILEFORMAT : ofn.nFilterIndex = 2; break;
              case HTML          : ofn.nFilterIndex = 3; break;
              default            : ofn.nFilterIndex = 1; break;
            } /*endswitch */
            ofn.lpstrFileTitle              = szTitle;               // output
            ofn.nMaxFileTitle               = sizeof(szTitle);
            ofn.lpstrInitialDir             = NULL;
            ofn.lpstrTitle                  = "Select File" ;
            ofn.nFileOffset                 = 0;
            ofn.nFileExtension              = 0;
            ofn.lCustData                   = 0L;
            ofn.lpfnHook                    = NULL;
            ofn.lpTemplateName              = NULL;
            ofn.hwndOwner                   = hwnd;
            ofn.lpstrFile                   = szT;
            ofn.nMaxFile                    = sizeof(szT);
            ofn.Flags                       = OFN_HIDEREADONLY |
                                              OFN_NONETWORKBUTTON ;

            ofn.lpstrDefExt                 = NULL;

            szTitle[0] = TEXT('\0');  // output
            szT[0] = TEXT('\0');      // input
            QUERYTEXT (hwnd, DID_RPT_FILENAME_EF, szT);

            // Scan Extension of Input File Name
            // in case of standard extension, eliminate the extension

            pszExtension = strrchr(szT,'.');
            if (pszExtension )
            {
              if (!stricmp(pszExtension,".CNT") ||
                  !stricmp(pszExtension,".XML") ||
                  !stricmp(pszExtension,".TXT") ||
                  !stricmp(pszExtension,".RPT") ||
                  !stricmp(pszExtension,".HTM") ||
                  !stricmp(pszExtension,".HTML") )
              {
                *(pszExtension) = EOS;
                fExtensionStripped = TRUE;
              } /* endif */
            } /* endif */

            fOk = GetSaveFileName(&ofn);

            if (fOk )
            {
              switch ( ofn.nFilterIndex )
              {
                case 1 : pRptIda->ppropFolder->usFormat = ASCII; break;
                case 2 : pRptIda->ppropFolder->usFormat = XMLFILEFORMAT; break;
                case 3 : pRptIda->ppropFolder->usFormat = HTML; break;
                default: pRptIda->ppropFolder->usFormat = ASCII; break;
              } /*endswitch */
              RptFormatSelect( hwnd, DID_RPT_FORMAT_CB, pRptIda->ppropFolder->usFormat  );

              RptAdjustReportFileExtension( szT, pRptIda->ppropFolder->usFormat );

              SETTEXT (hwnd, DID_RPT_FILENAME_EF, szT);
            } /* endif */



          }

          break;


        case DID_RPT_DOCUMENTS_LB:
          //---------------------------------------------------------
          if (sNotification == LN_ENTER || sNotification == LN_SELECT)
          {
            // deselect LB items in document LB
            DESELECTITEM (hwnd, DID_RPT_DOCUMENTS_LB, LIT_NONE);
          } // end if
          break;

        case DID_RPT_FILE_CHK:
          //---------------------------------------------------------
          if (sNotification == BN_CLICKED || sNotification == BN_DBLCLICKED)
          {
            // enable / disable PB "Select..."
            ENABLECTRL (hwnd, DID_RPT_FILESELECT_PB,
                        (BOOL) QUERYCHECK (hwnd, DID_RPT_FILE_CHK));
            // enable / disable EF "Filename"
            ENABLECTRL (hwnd, DID_RPT_FILENAME_EF,
                        (BOOL) QUERYCHECK (hwnd, DID_RPT_FILE_CHK));
            // enable / disable CB "File Format"
            ENABLECTRL (hwnd, DID_RPT_FORMAT_CB,
                        (BOOL) QUERYCHECK (hwnd, DID_RPT_FILE_CHK));
          } // end if
          break;

        case DID_RPT_REPORT_CB:
          //---------------------------------------------------------
          if (sNotification == CBN_SELCHANGE)
          {
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get selected report and save it in ida and RPT
            pRptIda->ppropFolder->usReport = CBQUERYSELECTION (hwnd, DID_RPT_REPORT_CB);
            pRpt->usReport = pRptIda->ppropFolder->usReport;

            if (pRpt->usReport<0 || pRpt->usReport>MAX_REPORTS)
            {
              pRpt->usReport = 0;
              pRptIda->ppropFolder->usReport = 0;
            }//end if

            // fill OPTIONS for reports
            RptFillReportOptions( hwnd, pRptIda, pRpt->usReport, pRptIda->ppropFolder->usOption[pRpt->usReport] ); 

            // enable/disable settings
            ENABLECTRL (hwnd, DID_RPT_SETTINGS_PB,
                        ( ((pRpt->usReport>=SUMMARY_COUNTING_REPORT)&&
                           (pRptIda->ppropFolder->usReport<COMBINED_REPORT)) ||
                          (pRptIda->ppropFolder->usReport==HISTORY_REPORT)
                        ));

            // enable/disable settings
            ENABLECTRL (hwnd, DID_RPT_SETTINGS1_PB,
                        ( (pRpt->usReport>=SUMMARY_COUNTING_REPORT)&&
                          (pRptIda->ppropFolder->usReport<COMBINED_REPORT) ));

            // check if empty folder is ok for selected report
            // -----------------------------------------------
            if ( (pRpt->usReport != HISTORY_REPORT    ) &&
                 (pRpt->usReport != CALCULATION_REPORT) &&
                 (pRpt->usReport != SUMMARY_COUNTING_REPORT) &&
                 (pRpt->fFolderSelected == TRUE       ) )
            {
              if (pRpt->usSelectedDocuments == 0)
              {
                // disable PB "Create"
                ENABLECTRL (hwnd, DID_RPT_CREATE_PB, FALSE);
                // disable SPINBUTTON
                ENABLECTRL (hwnd, DID_RPT_OPTIONS_SPIN, FALSE);
                // disable CHK "File"
                ENABLECTRL (hwnd, DID_RPT_FILE_CHK, FALSE);

                // check state of CHK "File"
                if (QUERYCHECK (hwnd, DID_RPT_FILE_CHK))
                {
                  // disable file name EF
                  ENABLECTRL (hwnd, DID_RPT_FILENAME_EF, FALSE);
                  // disable PB "Select.."
                  ENABLECTRL (hwnd, DID_RPT_FILESELECT_PB, FALSE);
                } // end if

                pszTmp = pRptIda->pRpt->szLongFolderName;

                UtlErrorHwnd (MESSAGE_RPT_EMPTY_FOLDER, MB_OK, 1,
                              &pszTmp, EQF_INFO, hwnd);
              } // end if
            }
            else
            {
              // enable PB "Create"
              ENABLECTRL (hwnd, DID_RPT_CREATE_PB, TRUE);
              // enable SPINBUTTON
              ENABLECTRL (hwnd, DID_RPT_OPTIONS_SPIN, TRUE);
              ENABLECTRL (hwnd, DID_RPT_OPTIONS_SPIN, TRUE );
              // enable CHK "File"
              ENABLECTRL (hwnd, DID_RPT_FILE_CHK, TRUE);

              // check state of CHK "File"
              if (QUERYCHECK (hwnd, DID_RPT_FILE_CHK))
              {
                // enable file name EF
                ENABLECTRL (hwnd, DID_RPT_FILENAME_EF, TRUE);
                // enable PB "Select.."
                ENABLECTRL (hwnd, DID_RPT_FILESELECT_PB, TRUE);
              } // end if
            } // end if


            //
            // reorganize all other windows
            //

            nItem = 1;
            while ( pRptIda->hwndPages[nItem] && fOk )
            {
              PFNWP pfnWp = (PFNWP) GetWindowLong( pRptIda->hwndPages[ nItem ],
                                                   DWL_DLGPROC );

              fOk =  !pfnWp( pRptIda->hwndPages[nItem], WM_INITDLG,
                             MP2FROMP(pRptIda), MP2FROMP(pRptIda) );
              nItem++;
            } /* endwhile */



          } // end if



          break;

        case DID_RPT_FORMAT_CB:
          //---------------------------------------------------------
          if (sNotification == CBN_SELCHANGE)
          {
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get selected format and save it in ida and RPT
            pRptIda->ppropFolder->usFormat = RptFormatGetSelected(hwnd, DID_RPT_FORMAT_CB);
            pRpt->usFormat = pRptIda->ppropFolder->usFormat;

            QUERYTEXT( hwnd, DID_RPT_FILENAME_EF, pRpt->szRptOutputFile );
            if ( RptAdjustReportFileExtension( pRpt->szRptOutputFile, pRpt->usFormat ) )
            {
              SETTEXT( hwnd, DID_RPT_FILENAME_EF, pRpt->szRptOutputFile );
            } /* endif */

          } // end if
          break;


        case DID_RPT_OPTIONS_SPIN:
          //---------------------------------------------------------
          if (sNotification == CBN_SELCHANGE)
          {
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get selected options and save it in ida and RPT
            pRptIda->ppropFolder->usOption[pRptIda->ppropFolder->usReport] = CBQUERYSELECTION (hwnd, DID_RPT_OPTIONS_SPIN);
            pRpt->usOptions = pRptIda->ppropFolder->usOption[pRptIda->ppropFolder->usReport];
            pRpt->usOption[pRptIda->ppropFolder->usReport] = pRptIda->ppropFolder->usOption[pRptIda->ppropFolder->usReport];

          } // end if
          break;


      } /* endswitch */
      break;

    case WM_HELP:
      //-----------------------------------
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblCntRptDlg1[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
      //-----------------------------------
      DelCtrlFont( hwnd, DID_AN_ANALYZE_LB );
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;

    default:
      //-----------------------------------
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
};


//+----------------------------------------------------------------------------+
//|Internal function  RPTPROP_PROFILE_DLGPROC                                  |
//+----------------------------------------------------------------------------+
//|Function name:                                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+




INT_PTR CALLBACK RPTPROP_PROFILE_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PRPTIDA  pRptIda;
  PRPT    pRpt;               // pointer to report data
  PSZ     pszTemp;             // tmp pointer to string
  CHAR    chProfile[40];
  BOOL    fOk = TRUE;
  CHAR    szProfiles[MAX_PATH144+MAX_FILESPEC]; // profile names
  int     i,j;

  switch ( msg )
  {
    case WM_INITDLG:
      {
        BOOL    fOk = TRUE;                  // error indicator
        SHORT   sItem;
        CHAR    szProfiles[MAX_PATH144+MAX_FILESPEC]; // profile names
        PSZ     pszEnd;
        CHAR    szProfile[MAX_PATH144];

        pRptIda = (PRPTIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pRptIda );
        pRpt = pRptIda->pRpt;

        if (fOk)
        {
          // FILL CB Profiles to be saved/loaded
          //   - create the path to the format information files

          UtlMakeEQFPath( szProfiles, NULC,
                          PROPERTY_PATH,NULL );
          strcat(szProfiles , BACKSLASH_STR );
          strcat(szProfiles , DEFAULT_PATTERN_NAME );
          strcat(szProfiles , "." );
          strcat(szProfiles , "R00" );

          // load the file names in the format combobox
          CBDELETEALL(hwnd,DID_RPT_PROFILE_CB);

          if ( UtlLoadFileNames( szProfiles,
                                 FILE_NORMAL,
                                 WinWindowFromID( hwnd,
                                                  DID_RPT_PROFILE_CB),
                                 NAMFMT_NODRV | NAMFMT_NODIR |
                                 NAMFMT_NOROOT | NAMFMT_NOEXT )
               == UTLERROR )
          {
            fOk = FALSE;
          } /* endif */


          // set last used Profile Name
          // --------------------------
          strcpy(szProfile,pRptIda->ppropFolder->szProfile);
          pszEnd = strrchr(szProfile,'.');
          if (pszEnd )
          {
            *pszEnd = EOS;
          } /* endif */
          if ( szProfile[0] != EOS )
          {
            sItem = CBSEARCHITEM(hwnd, DID_RPT_PROFILE_CB,
                                szProfile);
            CBSELECTITEM (hwnd, DID_RPT_PROFILE_CB,max(sItem,0) );
          } /* endif */

          // initial load of profile
          if ( pRptIda->pRpt->fInitialLoad )
          {
            if ( pRptIda->ppropFolder->szProfile[0] != EOS )
            {
              strcpy( pRpt->szProfile, pRptIda->ppropFolder->szProfile );
              SendMessage( hwnd, WM_COMMAND, DID_RPT_LOAD_PB, 0 );
            }
            else
            {
              SETTEXT( hwnd, DID_RPT_ACTPROFILE_TEXT, "- none -" );
              SETTEXT( hwnd, DID_RPT_INTPROFILENAME_TEXT, "" );
              pRptIda->pRpt->fInitialLoad = FALSE;
            } /* endif */
          } /* endif */
        } // end if
      }
      break;

    case WM_COMMAND:
      //-----------------------------------
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {

        case DID_RPT_LOAD_PB :      // PB "LOAD PROFILE" pressed
          //---------------------------------------------------------
          {
            HPROP        hpropFolder;
            PPROPFOLDER  ppropFolder;
            ULONG        ulErrorInfo;
            PSZ          pszParm;
            PSZ          pszEnd;
            INT          nItem;

            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get selected profile and save it in ida and RPT
            if ( !pRpt->fInitialLoad )
            {
              QUERYTEXT (hwnd, DID_RPT_PROFILE_CB, chProfile);
              if ( chProfile[0] == EOS )
              {
                SHORT sItem;
                CBQUERYSELECTEDITEMTEXT( sItem, hwnd, DID_RPT_PROFILE_CB, chProfile );
              } /* endif */
              pszEnd = strrchr(chProfile,'.');
              if (pszEnd )
              {
                *pszEnd = EOS;
              } /* endif */
              strcat(chProfile, ".R00");

              if (strcmp(chProfile,pRptIda->ppropFolder->szProfile ))
              {
                strcpy(pRptIda->ppropFolder->szProfile,chProfile);
                strcpy(pRpt->szProfile,chProfile);
              } // end if
            } /* endif */

            UtlMakeEQFPath( szProfiles, NULC,
                            PROPERTY_PATH,NULL );
            pszTemp = UtlSplitFnameFromPath(szProfiles);
            strcat(szProfiles , BACKSLASH_STR );
            strcat(szProfiles , pRptIda->ppropFolder->szProfile );

            //
            // open and save properties to get correct length
            //
            hpropFolder = OpenProperties(szProfiles, NULL,PROP_ACCESS_READ,&ulErrorInfo );
            if ( hpropFolder )
            {

              // set write access to properties
              SetPropAccess (hpropFolder, PROP_ACCESS_WRITE);
              SaveProperties (hpropFolder, &ulErrorInfo);
              CloseProperties(hpropFolder,PROP_FILE,&ulErrorInfo);

            }//end if

            hpropFolder = OpenProperties(szProfiles, NULL,PROP_ACCESS_READ,&ulErrorInfo );
            if ( hpropFolder )
            {
              ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd(hpropFolder);

              // set new protection state
              pRptIda->pRpt->fProtectedProfile = RptCheckProfileStamp( ppropFolder );

              /* copy profile into folder properties                     */
              pRptIda->ppropFolder->usFormat      =  ppropFolder->usFormat      ;
              pRptIda->ppropFolder->usOption[0]   =  ppropFolder->usOption[0]   ;
              pRptIda->ppropFolder->usOption[1]   =  ppropFolder->usOption[1]   ;
              pRptIda->ppropFolder->usOption[2]   =  ppropFolder->usOption[2]   ;
              pRptIda->ppropFolder->usOption[3]   =  ppropFolder->usOption[3]   ;
              pRptIda->ppropFolder->usOption[4]   =  ppropFolder->usOption[4]   ;
              pRptIda->ppropFolder->usOption[5]   =  ppropFolder->usOption[5]   ;
              pRptIda->ppropFolder->usOption1     =  ppropFolder->usOption1     ;
              pRptIda->ppropFolder->usOption2     =  ppropFolder->usOption2     ;
              pRptIda->ppropFolder->usOption3     =  ppropFolder->usOption3     ;
              pRptIda->ppropFolder->usOption4     =  ppropFolder->usOption4     ;
              pRptIda->ppropFolder->usOption5     =  ppropFolder->usOption5     ;
              pRptIda->ppropFolder->usOption21    =  ppropFolder->usOption21    ;
              pRptIda->ppropFolder->usOption22    =  ppropFolder->usOption22    ;
              pRptIda->ppropFolder->usColumns[0]  =  ppropFolder->usColumns[0]  ;
              pRptIda->ppropFolder->usColumns[1]  =  ppropFolder->usColumns[1]  ;
              pRptIda->ppropFolder->usColumns[2]  =  ppropFolder->usColumns[2]  ;
              pRptIda->ppropFolder->usColumns[3]  =  ppropFolder->usColumns[3]  ;

              // GQ: options "plausibility check" and "lost data force new shipment" are
              //     always inactive!
              pRptIda->ppropFolder->usColumns4[0]  =  FALSE; // ppropFolder->usColumns4[0] ;
              pRptIda->ppropFolder->usColumns4[2]  =  FALSE; // ppropFolder->usColumns4[2]  ;

              pRptIda->ppropFolder->usColumns4[1]  =  ppropFolder->usColumns4[1]  ;
              pRptIda->ppropFolder->usColumns4[3]  =  ppropFolder->usColumns4[3]  ;
              pRptIda->ppropFolder->usColumns4[4]  =  ppropFolder->usColumns4[4]  ;
              pRptIda->ppropFolder->usStandard    =  ppropFolder->usStandard    ;
              pRptIda->ppropFolder->Pay_per_Standard = ppropFolder->Pay_per_Standard;

              // Set complexity and pay factor for final fact sheet
              // according properties
              for (i=0;i<MAX_PAYREPORT_COLUMNS;i++)
              {
                for (j=0;j<3;j++)
                {
                  pRptIda->ppropFolder->Complexity_Factor[i][j] = ppropFolder->Complexity_Factor[i][j];
                  pRptIda->ppropFolder->Pay_Factor[i][j] = ppropFolder->Pay_Factor[i][j];

                }  //end for
              } //end for

              // during initial load of profile leave last used values of user as-is
              if ( pRpt->fInitialLoad )
              {
                // in protected profiles only the some fields are user selectable
                if (  pRptIda->pRpt->fProtectedProfile )
                {
                  //pRptIda->ppropFolder->usCurrency    =  ppropFolder->usCurrency ;
                }
                else
                {
                  pRptIda->ppropFolder->usCurrency    =  ppropFolder->usCurrency ;
                } /* endif */
              }
              else
              {
                pRptIda->ppropFolder->usCurrency    =  ppropFolder->usCurrency ;
              } /* endif */

              // set profile internam name field
              SETTEXT( hwnd, DID_RPT_INTPROFILENAME_TEXT, ppropFolder->szIntProfileName );


              CloseProperties(hpropFolder,PROP_FILE,&ulErrorInfo);

              pRptIda->pRpt->usFormat   = pRptIda->ppropFolder->usFormat;   // selected format
              pRptIda->pRpt->usOption[0]= pRptIda->ppropFolder->usOption[0]; //History
              pRptIda->pRpt->usOption[1]= pRptIda->ppropFolder->usOption[1]; //Counting
              pRptIda->pRpt->usOption[2]= pRptIda->ppropFolder->usOption[2]; //Summary
              pRptIda->pRpt->usOption[3]= pRptIda->ppropFolder->usOption[3]; //Summary
              pRptIda->pRpt->usOption[4]= pRptIda->ppropFolder->usOption[4]; //Summary
              pRptIda->pRpt->usOption[5]= pRptIda->ppropFolder->usOption[5]; //Summary
              pRptIda->pRpt->usOption1  = pRptIda->ppropFolder->usOption1;  // selected Options
              pRptIda->pRpt->usOption2  = pRptIda->ppropFolder->usOption2;  // selected Options
              pRptIda->pRpt->usOption3  = pRptIda->ppropFolder->usOption3;  // selected Options
              pRptIda->pRpt->usOption4  = pRptIda->ppropFolder->usOption4;  // selected Options
              pRptIda->pRpt->usOption5  = pRptIda->ppropFolder->usOption5;  // selected Options


              if (pRptIda->ppropFolder->usOption21 > MAX2_OPTIONS1)
              {
                pRptIda->ppropFolder->usOption21 = 0;
              }
              if (pRptIda->ppropFolder->usOption22 > MAX2_OPTIONS2)
              {
                pRptIda->ppropFolder->usOption22 = 0;
              }

              pRptIda->pRpt->usOption21 = pRptIda->ppropFolder->usOption21; // selected Options
              pRptIda->pRpt->usOption22 = pRptIda->ppropFolder->usOption22; // selected Options
              pRptIda->pRpt->usColumns[0] = pRptIda->ppropFolder->usColumns[0];
              pRptIda->pRpt->usColumns[1] = pRptIda->ppropFolder->usColumns[1];
              pRptIda->pRpt->usColumns[2] = pRptIda->ppropFolder->usColumns[2];
              pRptIda->pRpt->usColumns[3] = pRptIda->ppropFolder->usColumns[3];

              // GQ: options "plausibility check" and "lost data force new shipment" are
              //     always inactive!
              pRptIda->pRpt->usColumns4[0] = FALSE; // pRptIda->ppropFolder->usColumns4[0];
              pRptIda->pRpt->usColumns4[2] = FALSE; // pRptIda->ppropFolder->usColumns4[2];

              pRptIda->pRpt->usColumns4[1] = pRptIda->ppropFolder->usColumns4[1];
              pRptIda->pRpt->usColumns4[3] = pRptIda->ppropFolder->usColumns4[3];
              pRptIda->pRpt->usColumns4[4] = pRptIda->ppropFolder->usColumns4[4];
              pRptIda->pRpt->usStandard = pRptIda->ppropFolder->usStandard;
              pRptIda->pRpt->Pay_per_Standard  = pRptIda->ppropFolder->Pay_per_Standard;
              pRptIda->pRpt->usCurrency = pRptIda->ppropFolder->usCurrency;

              // Set complexity and pay factor for final fact sheet
              // according properties
              for (i=0;i<MAX_PAYREPORT_COLUMNS;i++)
              {
                for (j=0;j<3;j++)
                {
                  pRptIda->pRpt->Complexity_Factor[i][j] = pRptIda->ppropFolder->Complexity_Factor[i][j];
                  pRptIda->pRpt->Pay_Factor[i][j] = pRptIda->ppropFolder->Pay_Factor[i][j];
                }  //end for
              }  //end for

              // set  Format
              RptFormatSelect( hwnd, DID_RPT_FORMAT_CB, pRptIda->ppropFolder->usFormat );
              pszEnd = strrchr(szProfiles,'.');
              if (pszEnd )
              {
                *pszEnd = EOS;
              } /* endif */
              pszParm = UtlSplitFnameFromPath(szProfiles);
              if ( !pRpt->fInitialLoad )
              {
                UtlError( MESSAGE_RPT_PROFILE_LOAD , MB_OK, 1,
                          &pszParm, EQF_INFO );
              } /* endif */
            }
            else
            {
              // error
              pszEnd = strrchr(szProfiles,'.');
              if (pszEnd )
              {
                *pszEnd = EOS;
              } /* endif */
              pszParm = UtlSplitFnameFromPath(szProfiles);
              UtlError(ERROR_PROFILE_LOAD, MB_CANCEL, 1, &pszParm, EQF_ERROR);

            } /* endif */


            // set actual profile name
            if ( fOk )
            {
              CHAR szProfileName[MAX_EQF_PATH];
              Utlstrccpy( szProfileName, pRptIda->ppropFolder->szProfile, DOT );
              SETTEXT( hwnd, DID_RPT_ACTPROFILE_TEXT, szProfileName );
            } /* endif */

            pRptIda->pRpt->fInitialLoad = FALSE;


            // reorganize all other windows
            nItem = 2; // do no restart general and profile page
            while ( pRptIda->hwndPages[nItem] && fOk )
            {
              if (nItem != 1)
              {
                PFNWP pfnWp = (PFNWP) GetWindowLong( pRptIda->hwndPages[ nItem ],
                                                     DWL_DLGPROC );

                fOk =  !pfnWp( pRptIda->hwndPages[nItem], WM_INITDLG,
                               MP2FROMP(pRptIda), MP2FROMP(pRptIda));

              } // end if
              nItem++;
            } /* endwhile */
          }
          break;

        case DID_RPT_SAVE_PB :      // PB "SAVE PROFILE" pressed
          //---------------------------------------------------------

          {
            HPROP        hpropFolder;
            PPROPFOLDER  ppropFolder;
            ULONG        ulErrorInfo;
            SHORT        sItem;
            PSZ          pszEnd;
            PSZ          pszParm;
            CHAR         szProfileSave[MAX_PATH144+MAX_FILESPEC]; // profile names
            BOOL         fNew=FALSE;
            BOOL         fBeSaved=FALSE;
            CHAR         szProfileName[MAX_EQF_PATH];
            BOOL         fProtected = FALSE;

            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get selected profile and save it in ida and RPT
            QUERYTEXT (hwnd, DID_RPT_PROFILE_CB, chProfile);
            UtlStripBlanks( chProfile );
            strcpy( szProfileName, chProfile );
            pszEnd = strrchr(chProfile,'.');
            if (pszEnd )
            {
              *pszEnd = EOS;
            } /* endif */
            strcat(chProfile, ".R00");

            if (strcmp(chProfile,pRptIda->ppropFolder->szProfile ))
            {
              strcpy(pRptIda->ppropFolder->szProfile,chProfile);
              strcpy(pRpt->szProfile,chProfile);
            } // end if


            UtlMakeEQFPath( szProfiles, NULC, PROPERTY_PATH,NULL );
            pszTemp = UtlSplitFnameFromPath(szProfiles);
            strcat(szProfiles , BACKSLASH_STR );
            strcat(szProfiles , pRptIda->ppropFolder->szProfile );
            strcpy(szProfileSave,szProfiles);
            pszEnd = strrchr(szProfileSave,'.');
            if (pszEnd )
            {
              *pszEnd = EOS;
            } /* endif */

            // check if target profile is protected
            {
              hpropFolder = OpenProperties(szProfiles, NULL,PROP_ACCESS_READ,&ulErrorInfo );
              if ( hpropFolder )
              {
                ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd(hpropFolder);
                fProtected = RptCheckProfileStamp( ppropFolder );
                CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
              } /* endif */
            } /* endif */

            if ( fProtected )
            {
              PSZ pszParm = szProfileName;
              UtlError( ERROR_PROFILE_NOSAVEALLOWED, MB_CANCEL, 1, &pszParm, EQF_ERROR );
            }
            else if (strlen(pRptIda->ppropFolder->szProfile)<5 ||
                     strlen(pRptIda->ppropFolder->szProfile)>12 )
            {
              PSZ    pszParm;
              // Profile format error
              // set last used Profile Name

              pszParm = UtlSplitFnameFromPath(szProfileSave);
              pszEnd = strrchr(szProfileSave,'.');
              if (pszEnd )
              {
                *pszEnd = EOS;
              } /* endif */
              UtlError(ERROR_RPT_PROFILE_INVALID, MB_CANCEL, 1,
                       &pszParm, EQF_ERROR);
              strcpy(szProfileSave,szProfiles);
              // set cursor to entryfield
              WinSetFocus (HWND_DESKTOP, WinWindowFromID (hwnd, DID_RPT_PROFILE_CB));
            }
            else
            {
              // SaveProperties displays an error message
              // ---------------------------------------

              USHORT   usResponse;
              PSZ      pszReplace;

              hpropFolder = OpenProperties(szProfiles, NULL,PROP_ACCESS_READ,&ulErrorInfo );
              if ( hpropFolder )
              {
                ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd(hpropFolder);
                fNew = FALSE;
              }
              else
              {
                hpropFolder = CreateProperties(szProfiles,
                                               NULL,PROP_CLASS_FOLDER,&ulErrorInfo );
                ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd(hpropFolder);
                fNew = TRUE;
              } /* end if */

              // set write access to properties
              SetPropAccess (hpropFolder, PROP_ACCESS_WRITE);

              if (!fNew )
              {
                pszReplace = pRptIda->ppropFolder->szProfile;
                usResponse = UtlError (ERROR_FILE_EXISTS_ALREADY, MB_YESNO | MB_DEFBUTTON2,
                                       1, &pszReplace, EQF_QUERY);

                switch (usResponse)
                {
                  case MBID_YES :   // overwrite
                    fBeSaved=TRUE;
                    break;

                  case MBID_NO :    // do not overwrite existing file
                    // nothing to do
                    fBeSaved = FALSE;
                    break;
                } // endswitch
              }
              else
              {
                fBeSaved=TRUE;
              } // endif

              // don't overwrite the propFolder !
              if (fBeSaved == FALSE)
              {
                CloseProperties(hpropFolder,PROP_FILE,&ulErrorInfo);
                break;
              }

              ppropFolder->usFormat     =  pRptIda->ppropFolder->usFormat     ;
              ppropFolder->usOption[0]  =  pRptIda->ppropFolder->usOption[0]  ;
              ppropFolder->usOption[1]  =  pRptIda->ppropFolder->usOption[1]  ;
              ppropFolder->usOption[2]  =  pRptIda->ppropFolder->usOption[2]  ;
              ppropFolder->usOption[3]  =  pRptIda->ppropFolder->usOption[3]  ;
              ppropFolder->usOption[4]  =  pRptIda->ppropFolder->usOption[4]  ;
              ppropFolder->usOption[5]  =  pRptIda->ppropFolder->usOption[5]  ;
              ppropFolder->usOption1    =  pRptIda->ppropFolder->usOption1    ;
              ppropFolder->usOption2    =  pRptIda->ppropFolder->usOption2    ;
              ppropFolder->usOption3    =  pRptIda->ppropFolder->usOption3    ;
              ppropFolder->usOption4    =  pRptIda->ppropFolder->usOption4    ;
              ppropFolder->usOption5    =  pRptIda->ppropFolder->usOption5    ;


              if (pRptIda->ppropFolder->usOption21 > MAX2_OPTIONS1)
              {
                pRptIda->ppropFolder->usOption21 = 0;
              }
              if (pRptIda->ppropFolder->usOption22 > MAX2_OPTIONS2)
              {
                pRptIda->ppropFolder->usOption22 = 0;
              }

              ppropFolder->usOption21   =  pRptIda->ppropFolder->usOption21   ;
              ppropFolder->usOption22   =  pRptIda->ppropFolder->usOption22   ;
              ppropFolder->usColumns[0] =  pRptIda->ppropFolder->usColumns[0] ;
              ppropFolder->usColumns[1] =  pRptIda->ppropFolder->usColumns[1] ;
              ppropFolder->usColumns[2] =  pRptIda->ppropFolder->usColumns[2] ;
              ppropFolder->usColumns[3] =  pRptIda->ppropFolder->usColumns[3] ;

              // GQ: options "plausibility check" and "lost data force new shipment" are
              //     always inactive!
              ppropFolder->usColumns4[0] =  FALSE; // pRptIda->ppropFolder->usColumns4[0] ;
              ppropFolder->usColumns4[2] =  FALSE; // pRptIda->ppropFolder->usColumns4[2] ;

              ppropFolder->usColumns4[1] =  pRptIda->ppropFolder->usColumns4[1] ;
              ppropFolder->usColumns4[3] =  pRptIda->ppropFolder->usColumns4[3] ;
              ppropFolder->usColumns4[4] =  pRptIda->ppropFolder->usColumns4[4] ;
              ppropFolder->usStandard   =  pRptIda->ppropFolder->usStandard   ;
              ppropFolder->Pay_per_Standard = pRptIda->ppropFolder->Pay_per_Standard ;
              ppropFolder->usCurrency   =  pRptIda->ppropFolder->usCurrency   ;


              // Set complexity and pay factor for final fact sheet
              // according properties
              for (i=0;i<MAX_PAYREPORT_COLUMNS;i++)
              {
                for (j=0;j<3;j++)
                {
                  ppropFolder->Complexity_Factor[i][j] = pRptIda->ppropFolder->Complexity_Factor[i][j];
                  ppropFolder->Pay_Factor[i][j] = pRptIda->ppropFolder->Pay_Factor[i][j];

                }  //end for
              } //end for

              SaveProperties (hpropFolder, &ulErrorInfo);
              if ( ulErrorInfo )
              {
                CHAR szProfileName[MAX_FILESPEC];
                fBeSaved = FALSE;
                Utlstrccpy( szProfileName, UtlGetFnameFromPath( szProfileSave ), DOT );
                pszParm = szProfileName;
                UtlError( ERROR_WRITE_PROFILE , MB_OK, 1, &pszParm, EQF_ERROR );
              }
              CloseProperties(hpropFolder,PROP_FILE,&ulErrorInfo);

              // FILL CB Profiles to be saved/loaded
              //   - create the path to the format information files

              UtlMakeEQFPath( szProfiles, NULC,
                              PROPERTY_PATH,NULL );
              strcat(szProfiles , BACKSLASH_STR );
              strcat(szProfiles , DEFAULT_PATTERN_NAME );
              strcat(szProfiles , "." );
              strcat(szProfiles , "R00" );

              // load the file names in the format combobox
              CBDELETEALL(hwnd, DID_RPT_PROFILE_CB);
              UtlLoadFileNames( szProfiles,
                                FILE_NORMAL,
                                WinWindowFromID( hwnd,
                                                 DID_RPT_PROFILE_CB),
                                NAMFMT_NODRV | NAMFMT_NODIR |
                                NAMFMT_NOROOT | NAMFMT_NOEXT );

              // set last used Profile Name
              pszEnd = strrchr(szProfileSave,'.');
              if (pszEnd )
              {
                *pszEnd = EOS;
              } /* endif */
              pszParm = UtlSplitFnameFromPath(szProfileSave);
              sItem = CBSEARCHITEM(hwnd, DID_RPT_PROFILE_CB,
                                   pszParm);
              CBSELECTITEM (hwnd, DID_RPT_PROFILE_CB,max(sItem,0) );


              if (fBeSaved == TRUE)
              {
                UtlError( MESSAGE_RPT_PROFILE_SAVE , MB_OK, 1,
                          &pszParm, EQF_INFO );
              } /* endif */
            }/* end if */

          }
          break;

        case PID_PB_OK :
          {
            pRptIda = ACCESSDLGIDA( hwnd, PRPTIDA );
            pRpt = pRptIda->pRpt;
          }
          break;
      } /* endswitch */
      break;


    case WM_HELP:
      //-----------------------------------
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblCntRptDlg1[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
      //-----------------------------------
      DelCtrlFont( hwnd, DID_AN_ANALYZE_LB );
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;

    default:
      //-----------------------------------
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
};

INT_PTR CALLBACK RPTPROP_SETTINGS_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PRPTIDA  pRptIda;
  PRPT    pRpt;               // pointer to report data
  SHORT   option1Index = 0;            // index of current option
  SHORT   option2Index = 0;            // index of current option
  SHORT   option3Index = 0;            // index of current option
  SHORT   option4Index = 0;            // index of current option
  SHORT   option5Index = 0;            // index of current option
  SHORT sNotification= WMCOMMANDCMD( mp1, mp2 );

  switch ( msg )
  {
    case WM_INITDLG:
      //-----------------------------------
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        BOOL    fOk = TRUE;                  // error indicator

        pRptIda = (PRPTIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pRptIda );
        pRpt = pRptIda->pRpt;

        if (fOk)
        {
          // fill CB "Option1"
          CBDELETEALL(hwnd, DID_RPT1_OPTION1_CB);
          while (option1Index < MAX_OPTIONS1)
          {
            LOADSTRING (hab, hResMod, SID_RPT1_OPTION1_1 + option1Index++,
                        pRptIda->szWorkString);
            CBINSERTITEMEND( hwnd, DID_RPT1_OPTION1_CB, pRptIda->szWorkString);
          } // end while

          // fill CB "Option2"
          CBDELETEALL(hwnd, DID_RPT1_OPTION2_CB );
          while (option2Index < MAX_OPTIONS2)
          {
            LOADSTRING (hab, hResMod, SID_RPT1_OPTION2_1 + option2Index++,
                        pRptIda->szWorkString);
            CBINSERTITEMEND( hwnd, DID_RPT1_OPTION2_CB, pRptIda->szWorkString);
          } // end while

          // fill CB "Option3"
          CBDELETEALL(hwnd, DID_RPT1_OPTION3_CB );
          while (option3Index < MAX_OPTIONS3)
          {
            LOADSTRING (hab, hResMod, SID_RPT1_OPTION3_1 + option3Index++,
                        pRptIda->szWorkString);
            CBINSERTITEMEND( hwnd, DID_RPT1_OPTION3_CB, pRptIda->szWorkString);
          } // end while

          // fill CB "Option4" Statistics Standard or Advanced
          CBDELETEALL(hwnd, DID_RPT1_OPTION4_CB );
          while (option4Index < MAX_OPTIONS4)
          {
            LOADSTRING (hab, hResMod, SID_RPT1_OPTION4_1 + option4Index++,
                        pRptIda->szWorkString);
            CBINSERTITEMEND( hwnd, DID_RPT1_OPTION4_CB, pRptIda->szWorkString);
          } // end while


          // fill CB "Option5" Statistics Standard or Advanced
          CBDELETEALL(hwnd, DID_RPT1_OPTION5_CB );
          while (option5Index < MAX_OPTIONS5)
          {
            LOADSTRING (hab, hResMod, SID_RPT1_OPTION5_1 + option5Index++,
                        pRptIda->szWorkString);
            CBINSERTITEMEND( hwnd, DID_RPT1_OPTION5_CB, pRptIda->szWorkString);
          } // end while

          // set last used Options1,2
          if (pRptIda->ppropFolder->usOption1<MAX_OPTIONS1 && pRptIda->ppropFolder->usOption1>=0)
          {
            CBSELECTITEM (hwnd, DID_RPT1_OPTION1_CB, pRptIda->ppropFolder->usOption1);
          }
          else
          {
            CBSELECTITEM (hwnd, DID_RPT1_OPTION1_CB, 0);
          }

          if (pRptIda->ppropFolder->usOption2<MAX_OPTIONS2 && pRptIda->ppropFolder->usOption2>=0)
          {
            CBSELECTITEM (hwnd, DID_RPT1_OPTION2_CB, pRptIda->ppropFolder->usOption2);
          }
          else
          {
            CBSELECTITEM (hwnd, DID_RPT1_OPTION2_CB, 0);
          }


          if (pRptIda->ppropFolder->usOption3<MAX_OPTIONS3 && pRptIda->ppropFolder->usOption3>=0)
          {
            CBSELECTITEM (hwnd, DID_RPT1_OPTION3_CB, pRptIda->ppropFolder->usOption3);
          }
          else
          {
            CBSELECTITEM (hwnd, DID_RPT1_OPTION3_CB, 0);
          }


          if (pRptIda->ppropFolder->usOption4<MAX_OPTIONS4 && pRptIda->ppropFolder->usOption4>=0)
          {
            CBSELECTITEM (hwnd, DID_RPT1_OPTION4_CB, pRptIda->ppropFolder->usOption4);
          }
          else
          {
            CBSELECTITEM (hwnd, DID_RPT1_OPTION4_CB, 0);
          }

          if (pRptIda->ppropFolder->usOption5<MAX_OPTIONS5 && pRptIda->ppropFolder->usOption5>=0)
          {
            CBSELECTITEM (hwnd, DID_RPT1_OPTION5_CB, pRptIda->ppropFolder->usOption5);
          }
          else
          {
            CBSELECTITEM (hwnd, DID_RPT1_OPTION5_CB, 0);
          }


          // init check boxes
          SETCHECK (hwnd, DID_RPT1_COLUMN1_CHK, pRptIda->ppropFolder->usColumns[0]);
          SETCHECK (hwnd, DID_RPT1_COLUMN2_CHK, pRptIda->ppropFolder->usColumns[1]);
          SETCHECK (hwnd, DID_RPT1_COLUMN3_CHK, pRptIda->ppropFolder->usColumns[2]);
          SETCHECK (hwnd, DID_RPT1_COLUMN4_CHK, pRptIda->ppropFolder->usColumns[3]);

          // disable controls if this is a protected profile
          if ( pRpt->fProtectedProfile )
          {
            ENABLECTRL( hwnd, DID_RPT1_COLUMN1_CHK, FALSE );
            ENABLECTRL (hwnd, DID_RPT1_COLUMN2_CHK, TRUE );
            ENABLECTRL (hwnd, DID_RPT1_COLUMN3_CHK, FALSE );
            ENABLECTRL (hwnd, DID_RPT1_COLUMN4_CHK, FALSE );

            ENABLECTRL (hwnd, DID_RPT1_OPTION1_CB, FALSE );
            ENABLECTRL (hwnd, DID_RPT1_OPTION2_CB, TRUE );
            ENABLECTRL (hwnd, DID_RPT1_OPTION3_CB, FALSE );
            ENABLECTRL (hwnd, DID_RPT1_OPTION4_CB, FALSE );
            ENABLECTRL (hwnd, DID_RPT1_OPTION5_CB, FALSE );
          }
          else
          {
            ENABLECTRL (hwnd, DID_RPT1_COLUMN2_CHK, TRUE );
            ENABLECTRL (hwnd, DID_RPT1_OPTION1_CB, TRUE );
            ENABLECTRL (hwnd, DID_RPT1_OPTION2_CB, TRUE );
            ENABLECTRL (hwnd, DID_RPT1_OPTION3_CB, TRUE );

            ENABLECTRL (hwnd,DID_RPT1_COLUMN1_CHK, pRptIda->ppropFolder->usOption2 ==  Details);

            ENABLECTRL (hwnd, DID_RPT1_COLUMN3_CHK,
                        ( pRptIda->ppropFolder->usReport==SUMMARY_COUNTING_REPORT));
            ENABLECTRL (hwnd, DID_RPT1_COLUMN4_CHK,
                        ( pRptIda->ppropFolder->usReport==SUMMARY_COUNTING_REPORT));
            ENABLECTRL (hwnd, DID_RPT1_OPTION4_CB,
                        ( pRptIda->ppropFolder->usReport==SUMMARY_COUNTING_REPORT &&
                          pRptIda->ppropFolder->usColumns[2] ));
            ENABLECTRL (hwnd, DID_RPT1_OPTION5_CB,
                        ( pRptIda->ppropFolder->usReport==REDUNDANCY_REPORT ));
          } /* endif */
          WinSetFocus (HWND_DESKTOP, hwnd);  // set focus on dialog
        } // end if
      } // end case
      break;

    case WM_COMMAND:
      //-----------------------------------
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {

        case DID_RPT1_COLUMN1_CHK :
          // get access to ida
          pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
          pRpt = pRptIda->pRpt;
          // get status of file checkbox and save it to folder properties
          // and count structure
          pRptIda->ppropFolder->usColumns[0] = QUERYCHECK (hwnd, DID_RPT1_COLUMN1_CHK);
          pRptIda->pRpt->usColumns[0] = pRptIda->ppropFolder->usColumns[0];
          break;

        case DID_RPT1_COLUMN2_CHK :
          // get access to ida
          pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
          pRpt = pRptIda->pRpt;
          // get status of file checkbox and save it to folder properties
          // and count structure
          pRptIda->ppropFolder->usColumns[1] = QUERYCHECK (hwnd, DID_RPT1_COLUMN2_CHK);
          pRptIda->pRpt->usColumns[1] = pRptIda->ppropFolder->usColumns[1];
          break;

        case DID_RPT1_COLUMN3_CHK :
          // get access to ida
          pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
          pRpt = pRptIda->pRpt;
          // get status of file checkbox and save it to folder properties
          // and count structure
          pRptIda->ppropFolder->usColumns[2] = QUERYCHECK (hwnd, DID_RPT1_COLUMN3_CHK);
          pRptIda->pRpt->usColumns[2] = pRptIda->ppropFolder->usColumns[2];
          ENABLECTRL (hwnd, DID_RPT1_OPTION4_CB,
                      ( pRptIda->ppropFolder->usReport==SUMMARY_COUNTING_REPORT &&
                        pRptIda->ppropFolder->usColumns[2] ));
          break;

        case DID_RPT1_COLUMN4_CHK :
          // get access to ida
          pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
          pRpt = pRptIda->pRpt;
          // get status of file checkbox and save it to folder properties
          // and count structure
          pRptIda->ppropFolder->usColumns[3] = QUERYCHECK (hwnd, DID_RPT1_COLUMN4_CHK);
          pRptIda->pRpt->usColumns[3] = pRptIda->ppropFolder->usColumns[3];
          ENABLECTRL (hwnd, DID_RPT1_OPTION4_CB,
                      ( pRptIda->ppropFolder->usReport==SUMMARY_COUNTING_REPORT &&
                        pRptIda->ppropFolder->usColumns[2] ));
          break;

        case DID_RPT1_OPTION1_CB:
          if (sNotification == CBN_SELCHANGE)
          {
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get selected report and save it in ida and RPT
            pRptIda->ppropFolder->usOption1 = CBQUERYSELECTION (hwnd, DID_RPT1_OPTION1_CB);
            pRpt->usOption1 = pRptIda->ppropFolder->usOption1;
          }
          break;

        case DID_RPT1_OPTION2_CB:
          if (sNotification == CBN_SELCHANGE)
          {
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get selected report and save it in ida and RPT
            pRptIda->ppropFolder->usOption2 = CBQUERYSELECTION (hwnd, DID_RPT1_OPTION2_CB);
            pRpt->usOption2 = pRptIda->ppropFolder->usOption2;

            if ( pRpt->fProtectedProfile )
            {
              ENABLECTRL (hwnd,DID_RPT1_COLUMN1_CHK, FALSE );
            }
            else
            {
              ENABLECTRL (hwnd,DID_RPT1_COLUMN1_CHK, pRpt->usOption2 ==  Details);
            } /* endif */
          }
          break;

        case DID_RPT1_OPTION3_CB:
          if (sNotification == CBN_SELCHANGE)
          {
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get selected report and save it in ida and RPT
            pRptIda->ppropFolder->usOption3 = CBQUERYSELECTION (hwnd, DID_RPT1_OPTION3_CB);
            pRpt->usOption3 = pRptIda->ppropFolder->usOption3;
          }
          break;

        case DID_RPT1_OPTION4_CB:
          if (sNotification == CBN_SELCHANGE)
          {
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get selected report and save it in ida and RPT
            pRptIda->ppropFolder->usOption4 = CBQUERYSELECTION (hwnd, DID_RPT1_OPTION4_CB);
            pRpt->usOption4 = pRptIda->ppropFolder->usOption4;
          }
          break;

        case DID_RPT1_OPTION5_CB:
          if (sNotification == CBN_SELCHANGE)
          {
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get selected report and save it in ida and RPT
            pRptIda->ppropFolder->usOption5 = CBQUERYSELECTION (hwnd, DID_RPT1_OPTION5_CB);
            pRpt->usOption5 = pRptIda->ppropFolder->usOption5;
          }
          break;



        case PID_PB_OK :
          {
            pRptIda = ACCESSDLGIDA( hwnd, PRPTIDA );
            pRpt = pRptIda->pRpt;
          }
          break;
      } /* endswitch */
      break;

    case WM_HELP:
      /* pass on a HELP_WM_HELP request                            */
      EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblCntRptDlg1[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
      //-----------------------------------
      DelCtrlFont( hwnd, DID_AN_ANALYZE_LB );
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;

    default:
      //-----------------------------------
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
}

//|Internal function  RPTPROP_FACTSHEET_DLGPROC                                |
INT_PTR CALLBACK RPTPROP_FACTSHEET_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PRPTIDA  pRptIda;
  PRPT    pRpt;               // pointer to report data
  SHORT   option1Index = 0;            // index of current option
  SHORT   option2Index = 0;            // index of current option
  SHORT   option3Index = 0;            // index of current option
  float   complexity_factor;
  float   pay_factor;
  int     Valid_Number = 1;
  float   factor;
  PSZ     pData;



  SHORT sNotification= WMCOMMANDCMD( mp1, mp2 );

  switch ( msg )
  {
    case WM_INITDLG:
      //-----------------------------------
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        BOOL    fOk = TRUE;                  // error indicator

        pRptIda = (PRPTIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pRptIda );
        pRpt = pRptIda->pRpt;

        if (fOk)
        {
          // set last used values in complexity and pay factors

          complexity_factor =   pRptIda->ppropFolder->Complexity_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] ;
          pay_factor =   pRptIda->ppropFolder->Pay_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] ;

          sprintf(pRptIda->szWorkString,"%1.3f",complexity_factor);
          SETTEXT(hwnd,DID_RPT2_FACTOR1_EF, pRptIda->szWorkString);

          sprintf(pRptIda->szWorkString,"%1.3f",pay_factor);
          SETTEXT(hwnd,DID_RPT2_FACTOR2_EF, pRptIda->szWorkString);


          // set last used Pay_per_Standard

          sprintf(pRptIda->szWorkString,"%1.3f",pRptIda->ppropFolder->Pay_per_Standard);
          SETTEXT(hwnd,DID_RPT2_FACTOR3_EF, pRptIda->szWorkString);


          // fill CB "Option1"
          CBDELETEALL(hwnd, DID_RPT2_OPTION1_CB );
          while (option1Index < MAX2_OPTIONS1)
          {
            LOADSTRING (hab, hResMod, SID_RPT2_OPTION1_1 + option1Index++,
                        pRptIda->szWorkString);
            CBINSERTITEMEND( hwnd, DID_RPT2_OPTION1_CB, pRptIda->szWorkString);
          } // end while

          // fill CB "Option2"
          CBDELETEALL(hwnd, DID_RPT2_OPTION2_CB );
          while (option2Index < MAX2_OPTIONS2)
          {
            LOADSTRING (hab, hResMod, SID_RPT2_OPTION2_1 + option2Index++,
                        pRptIda->szWorkString);
            CBINSERTITEMEND( hwnd, DID_RPT2_OPTION2_CB, pRptIda->szWorkString);
          } // end while

          // fill CB "Option3"
          CBDELETEALL(hwnd, DID_RPT2_OPTION3_CB );
          while (option3Index < MAX2_OPTIONS3)
          {
            LOADSTRING (hab, hResMod, SID_RPT2_OPTION3_1 + option3Index++,
                        pRptIda->szWorkString);
            CBINSERTITEMEND( hwnd, DID_RPT2_OPTION3_CB, pRptIda->szWorkString);
          } // end while



          // fill CB "Option4" Local currencies
          CBDELETEALL(hwnd, DID_RPT2_OPTION4_CB );
          option3Index = 0;
          while (option3Index < MAX2_OPTIONS4)
          {
            CBINSERTITEMEND( hwnd, DID_RPT2_OPTION4_CB, szRptCurrency[option3Index]);
            option3Index++;

          } // end while

          // set last used Options1,2,3
          if (pRptIda->ppropFolder->usOption21<MAX2_OPTIONS1 && pRptIda->ppropFolder->usOption21>=0)
          {
            CBSELECTITEM (hwnd, DID_RPT2_OPTION1_CB, pRptIda->ppropFolder->usOption21);
          }
          else
          {
            CBSELECTITEM (hwnd, DID_RPT2_OPTION1_CB, 0);
          }


          if (pRptIda->ppropFolder->usOption22<MAX2_OPTIONS2 && pRptIda->ppropFolder->usOption22>=0)
          {
            CBSELECTITEM (hwnd, DID_RPT2_OPTION2_CB, pRptIda->ppropFolder->usOption22);
          }
          else
          {
            CBSELECTITEM (hwnd, DID_RPT2_OPTION2_CB, 0);
          }


          if ( pRptIda->ppropFolder->usStandard < MAX2_OPTIONS3 )
          {
            CBSELECTITEM (hwnd, DID_RPT2_OPTION3_CB, pRptIda->ppropFolder->usStandard);
          }
          else
          {
            CBSELECTITEM (hwnd, DID_RPT2_OPTION3_CB, 0);
          }

          if (pRptIda->ppropFolder->usCurrency<MAX2_OPTIONS4 && pRptIda->ppropFolder->usCurrency>=0)
          {
            CBSELECTITEM (hwnd, DID_RPT2_OPTION4_CB, pRptIda->ppropFolder->usCurrency);
          }
          else
          {
            CBSELECTITEM (hwnd, DID_RPT2_OPTION4_CB, 0);
          }


          // set enable statement of controls
          ENABLECTRL( hwnd, DID_RPT2_OPTION1_CB, !pRpt->fProtectedProfile );
          ENABLECTRL( hwnd, DID_RPT2_OPTION2_CB, !pRpt->fProtectedProfile );
          ENABLECTRL( hwnd, DID_RPT2_OPTION3_CB, !pRpt->fProtectedProfile );
          //ENABLECTRL( hwnd, DID_RPT2_OPTION4_CB, !pRpt->fProtectedProfile );
          ENABLECTRL( hwnd, DID_RPT2_FACTOR1_EF, !pRpt->fProtectedProfile );
          ENABLECTRL( hwnd, DID_RPT2_FACTOR2_EF, !pRpt->fProtectedProfile );
          ENABLECTRL( hwnd, DID_RPT2_FACTOR3_EF, !pRpt->fProtectedProfile );

          WinSetFocus (HWND_DESKTOP, hwnd);  // set focus on dialog

        } // end if


      }
      break;


    case WM_COMMAND:
      //-----------------------------------
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case DID_RPT2_OPTION1_CB:
          if (sNotification == CBN_SELCHANGE)
          {
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // save last setting

            // get current complexity factor and convert it
            QUERYTEXT(hwnd,DID_RPT2_FACTOR1_EF,pRptIda->szWorkString);
            factor = RPT_ATOF(pRptIda->szWorkString, &Valid_Number);
            if (Valid_Number)
            {
              pRptIda->ppropFolder->Complexity_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] = factor;
              pRpt-> Complexity_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] =
              pRptIda->ppropFolder->Complexity_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22];
            }
            else
            {
              SETFOCUS( hwnd, DID_RPT2_FACTOR1_EF);
            } // end if Valid_Number

            // get current pay factor and convert it

            if (Valid_Number)
            {
              QUERYTEXT(hwnd,DID_RPT2_FACTOR2_EF,pRptIda->szWorkString);
              factor = RPT_ATOF(pRptIda->szWorkString, &Valid_Number);
              if (Valid_Number)
              {
                pRptIda->ppropFolder->Pay_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] = factor;
                pRpt-> Pay_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] =
                pRptIda->ppropFolder->Pay_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22];
              }
              else
              {
                SETFOCUS( hwnd, DID_RPT2_FACTOR2_EF);
              } // end if Valid_Number

            } // end if Valid Number


            if (Valid_Number)
            {
              // get selected report and save it in ida and RPT
              pRptIda->ppropFolder->usOption21 = CBQUERYSELECTION (hwnd, DID_RPT2_OPTION1_CB);
              pRpt->usOption21 = pRptIda->ppropFolder->usOption21;

              // set last used values in complexity and pay factors for new setting

              complexity_factor =   pRptIda->ppropFolder->Complexity_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] ;
              pay_factor =   pRptIda->ppropFolder->Pay_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] ;

              sprintf(pRptIda->szWorkString,"%1.3f",complexity_factor);
              SETTEXT(hwnd,DID_RPT2_FACTOR1_EF, pRptIda->szWorkString);

              sprintf(pRptIda->szWorkString,"%1.3f",pay_factor);
              SETTEXT(hwnd,DID_RPT2_FACTOR2_EF, pRptIda->szWorkString);
            }
            else
            {
              if (CBQUERYSELECTION (hwnd, DID_RPT2_OPTION1_CB) != pRptIda->ppropFolder->usOption21)
              {
                CBSELECTITEM (hwnd, DID_RPT2_OPTION1_CB, pRptIda->ppropFolder->usOption21);
              }
              else
              {
                pData =  pRptIda->szWorkString;
                UtlError(WARNING_NO_NUMERICAL_VALUE,MB_OK,1,&pData,EQF_WARNING);
              } // end if
            } // end if Valid_Number
          }// end if
          break;

        case DID_RPT2_OPTION2_CB:
          if (sNotification == CBN_SELCHANGE)
          {
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // save last setting

            // get current complexity factor and convert it
            QUERYTEXT(hwnd,DID_RPT2_FACTOR1_EF,pRptIda->szWorkString);
            factor = RPT_ATOF(pRptIda->szWorkString, &Valid_Number);
            if (Valid_Number)
            {
              pRptIda->ppropFolder->Complexity_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] = factor;
              pRpt-> Complexity_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] =
              pRptIda->ppropFolder->Complexity_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22];
            }
            else
            {
              SETFOCUS( hwnd, DID_RPT2_FACTOR1_EF);
            }// end if Valid_Number

            // get current pay factor and convert it
            if (Valid_Number)
            {
              QUERYTEXT(hwnd,DID_RPT2_FACTOR2_EF,pRptIda->szWorkString);
              factor = RPT_ATOF(pRptIda->szWorkString, &Valid_Number);
              if (Valid_Number)
              {
                pRptIda->ppropFolder->Pay_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] = factor;
                pRpt-> Pay_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] =
                pRptIda->ppropFolder->Pay_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22];
              }
              else
              {
                SETFOCUS( hwnd, DID_RPT2_FACTOR2_EF);
              }  // end if Valid_Number
            }// end if Valid_Number

            if (Valid_Number)
            {
              // get selected report and save it in ida and RPT
              pRptIda->ppropFolder->usOption22 = CBQUERYSELECTION (hwnd, DID_RPT2_OPTION2_CB);

              if (pRptIda->ppropFolder->usOption22 < 0 ||  pRptIda->ppropFolder->usOption22> MAX2_OPTIONS2)
              {
                pRptIda->ppropFolder->usOption22 = 0;
              }

              pRpt->usOption22 = pRptIda->ppropFolder->usOption22;

              // set last used values in complexity and pay factors

              complexity_factor =   pRptIda->ppropFolder->Complexity_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] ;
              pay_factor =   pRptIda->ppropFolder->Pay_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] ;

              sprintf(pRptIda->szWorkString,"%1.3f",complexity_factor);
              SETTEXT(hwnd,DID_RPT2_FACTOR1_EF, pRptIda->szWorkString);

              sprintf(pRptIda->szWorkString,"%1.3f",pay_factor);
              SETTEXT(hwnd,DID_RPT2_FACTOR2_EF, pRptIda->szWorkString);
            }
            else
            {
              if (CBQUERYSELECTION (hwnd, DID_RPT2_OPTION2_CB) != pRptIda->ppropFolder->usOption22)
              {
                CBSELECTITEM (hwnd, DID_RPT2_OPTION2_CB, pRptIda->ppropFolder->usOption22);
              }
              else
              {
                pData =  pRptIda->szWorkString;
                UtlError(WARNING_NO_NUMERICAL_VALUE,MB_OK,1,&pData,EQF_WARNING);
                SETFOCUS( hwnd, DID_RPT2_FACTOR2_EF);
              } // end if
            } // end if Valid_Number

          }
          break;

        case DID_RPT2_OPTION3_CB:
          if (sNotification == CBN_SELCHANGE)
          {
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get selected report and save it in ida and RPT
            pRptIda->ppropFolder->usStandard = CBQUERYSELECTION (hwnd, DID_RPT2_OPTION3_CB);
            pRpt->usStandard = pRptIda->ppropFolder->usStandard;


          }
          break;



        case DID_RPT2_OPTION4_CB:
          if (sNotification == CBN_SELCHANGE)
          {
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get selected report and save it in ida and RPT
            pRptIda->ppropFolder->usCurrency = CBQUERYSELECTION (hwnd, DID_RPT2_OPTION4_CB);
            pRpt->usCurrency = pRptIda->ppropFolder->usCurrency;
            strcpy(pRpt->szCurrency,szRptCurrency[pRpt->usCurrency]);

          }
          break;

        case PID_PB_OK :{
            // get access to ida
            pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
            pRpt = pRptIda->pRpt;

            // get current complexity factor and convert it

            QUERYTEXT(hwnd,DID_RPT2_FACTOR1_EF,pRptIda->szWorkString);
            factor = RPT_ATOF(pRptIda->szWorkString, &Valid_Number);
            if (Valid_Number)
            {
              pRptIda->ppropFolder->Complexity_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] = factor;
              pRpt-> Complexity_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] =
              pRptIda->ppropFolder->Complexity_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22];
            }
            else
            {
              SETFOCUS( hwnd, DID_RPT2_FACTOR1_EF);
            } // end if Valid_Number


            // get current pay factor and convert it

            if (Valid_Number)
            {

              QUERYTEXT(hwnd,DID_RPT2_FACTOR2_EF,pRptIda->szWorkString);
              factor = RPT_ATOF(pRptIda->szWorkString, &Valid_Number);
              if (Valid_Number)
              {
                pRptIda->ppropFolder->Pay_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] = factor;
                pRpt-> Pay_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22] =
                pRptIda->ppropFolder->Pay_Factor[pRptIda->ppropFolder->usOption21][pRptIda->ppropFolder->usOption22];
              }
              else
              {
                SETFOCUS( hwnd, DID_RPT2_FACTOR2_EF);
              }//end if valid number
            } // end if valid_number

            if (Valid_Number)
            {

              // get Pay_per_standard factor
              QUERYTEXT(hwnd,DID_RPT2_FACTOR3_EF,pRptIda->szWorkString);
              factor = RPT_ATOF(pRptIda->szWorkString, &Valid_Number);
              if (Valid_Number)
              {
                pRptIda->ppropFolder->Pay_per_Standard = factor;
                pRpt->Pay_per_Standard  = pRptIda->ppropFolder->Pay_per_Standard;
              }
              else
              {
                SETFOCUS( hwnd, DID_RPT2_FACTOR3_EF);
              }// end if valid_number
            } // end if valid_number

            if (Valid_Number)
            {
              WinPostMsg (hwnd, WM_EQF_CLOSE, MP1FROMSHORT(TRUE), NULL);
            }
            else
            {
              pData =  pRptIda->szWorkString;
              UtlError(WARNING_NO_NUMERICAL_VALUE,MB_OK,1,&pData,EQF_WARNING);
            } // end if
          }
          break;
      } /* endswitch */
      break;

    case WM_HELP:
      /* pass on a HELP_WM_HELP request                            */
      EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblCntRptDlg1[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
      //-----------------------------------
      DelCtrlFont( hwnd, DID_AN_ANALYZE_LB );
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;

    default:
      //-----------------------------------
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
}

//|Internal function  RPTPROP_MORE_DLGPROC                                     |
INT_PTR CALLBACK RPTPROP_MORE_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
){
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PRPTIDA  pRptIda;
  PRPT    pRpt;               // pointer to report data

  switch ( msg )
  {
    case WM_INITDLG:
      //-----------------------------------
      {
        BOOL    fOk = TRUE;                  // error indicator

        pRptIda = (PRPTIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pRptIda );
        pRpt = pRptIda->pRpt;

        if (fOk)
        {

          // init check boxes

          // GQ: options "plausibility check" and "lost data force new shipment" are
          //     always inactive!
          HIDECONTROL( hwnd, DID_RPT4_COLUMN1_CHK );
          HIDECONTROL( hwnd, DID_RPT4_COLUMN3_CHK );
//        SETCHECK (hwnd, DID_RPT4_COLUMN1_CHK, pRptIda->ppropFolder->usColumns4[0]);
//        SETCHECK (hwnd, DID_RPT4_COLUMN3_CHK, pRptIda->ppropFolder->usColumns4[2]);

          SETCHECK (hwnd, DID_RPT4_COLUMN2_CHK, pRptIda->ppropFolder->usColumns4[1]);


          WinSetFocus (HWND_DESKTOP, hwnd);  // set focus on dialog



        } // end if


      }
      break;

    case WM_COMMAND:
      //-----------------------------------
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {

        case DID_RPT4_COLUMN1_CHK :
          // get access to ida
          pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
          pRpt = pRptIda->pRpt;
          // get status of file checkbox and save it to folder properties
          // and count structure
          pRptIda->ppropFolder->usColumns4[0] = QUERYCHECK (hwnd, DID_RPT4_COLUMN1_CHK);
          pRptIda->pRpt->usColumns4[0] = pRptIda->ppropFolder->usColumns4[0];
          break;

        case DID_RPT4_COLUMN2_CHK :
          // get access to ida
          pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
          pRpt = pRptIda->pRpt;
          // get status of file checkbox and save it to folder properties
          // and count structure
          pRptIda->ppropFolder->usColumns4[1] = QUERYCHECK (hwnd, DID_RPT4_COLUMN2_CHK);
          pRptIda->pRpt->usColumns4[1] = pRptIda->ppropFolder->usColumns4[1];
          break;

        case DID_RPT4_COLUMN3_CHK :
          // get access to ida
          pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
          pRpt = pRptIda->pRpt;
          // get status of file checkbox and save it to folder properties
          // and count structure
          pRptIda->ppropFolder->usColumns4[2] = QUERYCHECK (hwnd, DID_RPT4_COLUMN3_CHK);
          pRptIda->pRpt->usColumns4[2] = pRptIda->ppropFolder->usColumns4[2];
          break;


        case PID_PB_OK :{

            pRptIda = ACCESSDLGIDA( hwnd, PRPTIDA );

            pRpt = pRptIda->pRpt;

          }

          break;


      } /* endswitch */
      break;

    case WM_HELP:
      //-----------------------------------
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblCntRptDlg1[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
      //-----------------------------------
      DelCtrlFont( hwnd, DID_AN_ANALYZE_LB );
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;

    default:
      //-----------------------------------
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;};






//+----------------------------------------------------------------------------+
//|Internal function  RPTPROP_SHIPM_DLGPROC                                    |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK RPTPROP_SHIPM_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PRPTIDA  pRptIda;
  PRPT    pRpt;               // pointer to report data

  switch ( msg )
  {
    case WM_INITDLG:
      //-----------------------------------
      {
        BOOL    fOk = TRUE;                  // error indicator

        pRptIda = (PRPTIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pRptIda );
        pRpt = pRptIda->pRpt;

        if (fOk)
        {
          // init check boxes
          SETCHECK (hwnd, DID_RPT5_COLUMN1_CHK, pRptIda->ppropFolder->usShipmentChk);

          // Fill combobox with all possible items
          CBDELETEALL(hwnd, DID_RPT5_OPTION1_CB);
          CBINSERTITEMEND( hwnd, DID_RPT5_OPTION1_CB , "All Shipments");
          CBINSERTITEMEND( hwnd, DID_RPT5_OPTION1_CB , "Single Shipments");

          // loop over histlog, get shipment information
          {
            BOOL fOK = TRUE;                     // O.K. flag
            int iRecNum = 0;                     // current record number
            ULONG ulLength = 0;                  // overall file length
            VARPART VarPart;

            PSZ pszDoc = "";
            FILE     *hInput = NULL;               // input file handle
            HISTLOGRECORD LogRecord;

            PSZ pszInFile = NULL;

            // build absolute path of histlog file
            strcpy (pRpt->szHistLogFile, pRpt->szFolderObjName);
            strcat (pRpt->szHistLogFile, RPT_HISTLOG_DIRECTORY);
            strcat (pRpt->szHistLogFile, HISTLOGFILE);

            pszInFile =  pRpt->szHistLogFile;


            /********************************************************************/
            /* Open input file                                                  */
            /********************************************************************/
            if ( fOK )
            {
              hInput = fopen( pszInFile, "rb" );
              if ( hInput == NULL )
              {
                fOK = FALSE;
              }
              else
              {
                ulLength = _filelength( _fileno(hInput) );
              } /* endif */
            } /* endif */


            /********************************************************************/
            /* Loop through input file                                          */
            /********************************************************************/
            if ( fOK )
            {
              LONG lRecordStart;                 // start positionof record within file
              USHORT usLongNameLength;

              while ( fOK && (ulLength != 0L) && !feof( hInput ) )
              {
                BOOL fRecord = TRUE;
                iRecNum++;

                // get fixed length record part
                lRecordStart = ftell( hInput );
                if ( fread( (PVOID)&LogRecord, sizeof(HISTLOGRECORD), 1, hInput ) != 1 )
                {
                  fOK = FALSE;
                }
                else
                {
                  ulLength -= (ULONG)sizeof(HISTLOGRECORD);
                } /* endif */

                // check if record is valid
                if ( fOK )
                {
                  if ( LogRecord.lEyeCatcher != HISTLOGEYECATCHER )
                  {
                    LONG lCurrent;
                    LONG lSkipped = 0L;

                    // re-position to start of log record
                    ulLength += (ULONG)sizeof(HISTLOGRECORD);
                    fseek( hInput, lRecordStart, SEEK_SET );

                    // skip data up to start of next valid record
                    do
                    {
                      fseek( hInput, lRecordStart, SEEK_SET );
                      fread( (PVOID)&lCurrent, sizeof(LONG), 1, hInput );
                      if ( lCurrent != HISTLOGEYECATCHER )
                      {
                        // try next byte
                        ulLength--;
                        lRecordStart++;
                        lSkipped++;
                      }
                      else
                      {
                        // position back to start of record
                        fseek( hInput, lRecordStart, SEEK_SET );
                      } /* endif */
                    } while ( (ulLength >= 2L) &&
                              (lCurrent != HISTLOGEYECATCHER) );

                    fRecord = FALSE;
                  } /* endif */
                } /* endif */

                // adjust record sizes of old records (were fill incorreclty)
                if ( fOK && fRecord )
                {
                  HistLogCorrectRecSizes( &LogRecord );
                } /* endif */

                // get document name
                if ( fOK && fRecord )
                {
                  strcpy( pRpt->szLongActualDocument, LogRecord.szDocName );
                  if ( LogRecord.fLongNameRecord )
                  {
                    USHORT usNameReadLen;
                    usLongNameLength = LogRecord.usSize - LogRecord.usAddInfoLength - sizeof(HISTLOGRECORD);
                    usNameReadLen = usLongNameLength;

                    pRpt->szLongActualDocument[0] = EOS;
                    if ( usNameReadLen >= sizeof(pRpt->szLongActualDocument) )
                    {
                      usNameReadLen = sizeof(pRpt->szLongActualDocument) - 1;
                    } /* endif */
                    if ( usNameReadLen ) fread( &(pRpt->szLongActualDocument), usNameReadLen, 1, hInput );
                    pRpt->szLongActualDocument[usNameReadLen] = EOS;
                  } /* endif */
                } /* endif */

                // get variable length record part
                if ( fOK && fRecord && (LogRecord.usAddInfoLength != 0) )
                {
                  // ensure that the additional info size is not corrupted
                  if ( LogRecord.usAddInfoLength > sizeof(VarPart) )
                  {
                    LogRecord.usAddInfoLength = sizeof(VarPart);
                  } /* endif */

                  if ( fread( (PVOID)&VarPart, LogRecord.usAddInfoLength, 1,
                              hInput ) != 1 )
                  {
                    fOK = FALSE;
                  }
                  else
                  {
                    ulLength -= (ULONG)LogRecord.usAddInfoLength;
                  } /* endif */
                } /* endif */

                // handle record contents
                if ( fOK && fRecord &&
                     ((pszDoc[0] == EOS) || (stricmp( pszDoc, LogRecord.szDocName) == 0) ) )
                {

                  // list task specific data
                  switch ( LogRecord.Task )
                  {
                    case DOCIMPORT_LOGTASK2:
                      {
                        SHORT sItem = LIT_NONE;

                        // for performance reasons we first check if the shipment is already in our shipment
                        // listbox, after that we check if th e document isselected for the report...
                        // insert in shipment listbox if not already done
                        OEMTOANSI( VarPart.DocImport2.szShipment );
                        sItem = CBSEARCHITEM( hwnd, DID_RPT5_OPTION1_CB, VarPart.DocImport2.szShipment );
                        if ( sItem < 0 )
                        {
                          // shipment not in our list, now check document name
                          OEMTOANSI( pRpt->szLongActualDocument );

                          sItem = SEARCHITEMHWND( pRptIda->hwndDocLB, pRpt->szLongActualDocument );
 
                          // if found add shipment number to our list
                          if ( sItem >= 0 )
                          {
                            CBINSERTITEMEND( hwnd, DID_RPT5_OPTION1_CB , VarPart.DocImport2.szShipment );
                          } /* endif */
                        } /* endif */
                      }
                      break;
                  } /* endswitch */
                } /* endif */
              } /* endwhile */
            } /* endif */

            /********************************************************************/
            /* Cleanup                                                          */
            /********************************************************************/
            if ( hInput )       fclose( hInput );

          } /* end of fill construction */

          CBSELECTITEM (hwnd, DID_RPT5_OPTION1_CB , 0 );

          // setup
          ENABLECTRL (hwnd, DID_RPT5_OPTION1_CB,   pRptIda->ppropFolder->usShipmentChk  );


          WinSetFocus (HWND_DESKTOP, hwnd);  // set focus on dialog



        } // end if


      }
      break;

    case WM_COMMAND:
      //-----------------------------------
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {

        case DID_RPT5_COLUMN1_CHK :
          // get access to ida
          pRptIda = ACCESSDLGIDA (hwnd, PRPTIDA);
          pRpt = pRptIda->pRpt;
          // get status of file checkbox and save it to folder properties
          // and count structure
          pRptIda->ppropFolder->usShipmentChk = QUERYCHECK (hwnd, DID_RPT5_COLUMN1_CHK);
          pRptIda->pRpt->usShipmentChk = pRptIda->ppropFolder->usShipmentChk;
          ENABLECTRL (hwnd, DID_RPT5_OPTION1_CB,  pRptIda->pRpt->usShipmentChk   );



          break;


        case PID_PB_OK :
          {

            pRptIda = ACCESSDLGIDA( hwnd, PRPTIDA );

            pRpt = pRptIda->pRpt;


            QUERYTEXT (hwnd, DID_RPT5_OPTION1_CB, pRptIda->pRpt->szShipmentChk);

          }

          break;


      } /* endswitch */
      break;

    case WM_HELP:
      //-----------------------------------
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblCntRptDlg1[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
      //-----------------------------------
      DelCtrlFont( hwnd, DID_AN_ANALYZE_LB );
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;

    default:
      //-----------------------------------
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
};


// check if profile is one of the protected ones
BOOL RPTIsProtectedProfile
(
  PSZ pszProfile
)
{
  BOOL fProtectedProfile = FALSE;

  if ( (strnicmp( pszProfile, "PII", 3 ) == 0 ) || (strnicmp( pszProfile, "PUB", 3 ) == 0 ) )
  {
    fProtectedProfile = TRUE;
  } /* endif */
  return( fProtectedProfile );
} /* end of function RPTIsProtectedProfile */


// checks stamp and checksum of a calculation profile
BOOL RptCheckProfileStamp( PPROPFOLDER pProp )
{
  BOOL fProtected = FALSE;

  LONG lCheckSum = RptCalcProfileCheckSum( pProp );

  if ( (lCheckSum == pProp->lCheckSum) && (memcmp( pProp->szProtectStamp, RPT_PROTECTSTAMP, sizeof(pProp->szProtectStamp)) == 0) )
  {
    fProtected = TRUE;
  } /* endif */

  return( fProtected );
} /* end of function RptCheckProfileStamp */


// computes the checksum for a calculation profile
LONG RptCalcProfileCheckSum( PPROPFOLDER pProp )
{
  LONG lFactor = 1;
  LONG lCheckSum = 0;
  int  iIndex = 0;
  LONG lOrgCheckSum = pProp->lCheckSum;
  PBYTE pbProp = (PBYTE)pProp;

  pProp->lCheckSum = 0;

  // get checksum for profile (wo PROPHEAD)
  for( iIndex = sizeof(PROPHEAD); iIndex < sizeof(PROPFOLDER); iIndex++ )
  {
    lCheckSum += pbProp[iIndex] * lFactor;
    lFactor++;
    if ( lFactor > 16 ) lFactor = 1;
  } /* endfor */

  pProp->lCheckSum = lOrgCheckSum;

  return( lCheckSum  );
} /* end of function RptCalcProfileCheckSum */



// fills the options combobox for the handles a change in he report type spinbutton
void RptFillReportOptions
( 
  HWND     hwnd,
  PRPTIDA  pRptIda,
  USHORT   usReport,
  USHORT   usOption
)
{
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
  int i_begin_option, i_end_option, optionIndex;

  CBDELETEALL( hwnd,DID_RPT_OPTIONS_SPIN );

  // fill OPTIONS for reports
  // as function of report
  // to be changed from Spinbutton
  if ( usReport == COMBINED_REPORT )
  {
    i_begin_option =  0;
    i_end_option   =  1;
    CBINSERTITEMEND( hwnd, DID_RPT_OPTIONS_SPIN, "Base List");
    CBINSERTITEMEND( hwnd, DID_RPT_OPTIONS_SPIN, "Detailed List");
  }
  else
  {
    if (usReport == HISTORY_REPORT)
    {
      i_begin_option = 0;
      i_end_option   =  REPORT_2_OFFSET;
    }
    else if (usReport == CALCULATION_REPORT)
    {
      i_begin_option =  REPORT_2_OFFSET;
      i_end_option   =  REPORT_3_OFFSET;
    }
    else
    {
      i_begin_option =  REPORT_3_OFFSET;
      i_end_option   =  MAX_OPTIONS-1;
    } // end if

    optionIndex = i_begin_option;
    while (optionIndex < i_end_option)
    {
      LOADSTRING( hab, hResMod, SID_RPT_OPTION_1 + optionIndex++, pRptIda->szWorkString );
      CBINSERTITEMEND( hwnd, DID_RPT_OPTIONS_SPIN, pRptIda->szWorkString);
    } // end while

    // set last used Option
    if (usReport<0 || usReport>MAX_REPORTS)
    {
      usReport = 0;
    }//end if

  } /* endif */

  // select last used option
  CBSELECTITEM(hwnd, DID_RPT_OPTIONS_SPIN, pRptIda->ppropFolder->usOption[usReport] );
  if ( usOption < i_end_option && usOption >= 0 )
  {
    CBSELECTITEM(hwnd, DID_RPT_OPTIONS_SPIN, usOption );
  }
  else
  {
    CBSELECTITEM (hwnd, DID_RPT_OPTIONS_SPIN, 0 );
  }
}

// fill format combobox with supported format names and set handle of items
void RptFormatFillCB( HWND hwnd, int id )
{
  SHORT sItem = 0;

  CBDELETEALL( hwnd, id );
    
  sItem = CBINSERTITEMEND( hwnd, id, "ASCII" );
  CBSETITEMHANDLE( hwnd, id, sItem, ASCII ); 

  sItem = CBINSERTITEMEND( hwnd, id, "HTML" );
  CBSETITEMHANDLE( hwnd, id, sItem, HTML ); 

  sItem = CBINSERTITEMEND( hwnd, id, "XML" );
  CBSETITEMHANDLE( hwnd, id, sItem, XMLFILEFORMAT ); 
}

// get selected format
USHORT RptFormatGetSelected( HWND hwnd, int id )
{
  USHORT usFormat = 0;
  SHORT sItem = 0;

  sItem = CBQUERYSELECTION( hwnd, id ); 
  if ( sItem >= 0 )
  {
    usFormat = (USHORT)CBQUERYITEMHANDLE( hwnd, id, sItem );
  } /* endif */
  return( usFormat );
}

// select given format
void RptFormatSelect( HWND hwnd, int id, USHORT usFormat )
{
  int iNumOfItems = CBQUERYITEMCOUNT( hwnd, id );
  int iSelItem = 0;
  int iItem = 0;

  for( iItem = 0; iItem < iNumOfItems; iItem++ )
  {
    USHORT usCurFormat = (USHORT)CBQUERYITEMHANDLE( hwnd, id, iItem );
    if ( usCurFormat == usFormat )
    {
      iSelItem = iItem;
    } /* endif */
  } /* endfor */
  CBSELECTITEM( hwnd, id, iSelItem );
}

// adjust the file extension of the report file depending on active format
BOOL RptAdjustReportFileExtension( PSZ pszFileName, USHORT usFormat )
{
  PSZ pszExtension;
  PSZ pszName;
  BOOL fChanged = FALSE;
  
  // find name part of file name
  pszName = strrchr( pszFileName, '\\' );
  if ( pszName == NULL ) pszName =  pszFileName;

  // find any file extension
  pszExtension = strrchr( pszFileName, '.' );

  // correct the extension
  switch ( usFormat)
  {
    case HTML :
      if ( pszExtension )
      { 
        if ( (stricmp( pszExtension, ".XML" ) == 0) ||
            (stricmp( pszExtension, ".TXT" ) == 0) ||
            (stricmp( pszExtension, ".RPT" ) == 0) ||
            (stricmp( pszExtension, ".CNT" ) == 0) )
        {
          *pszExtension = EOS;
          pszExtension = NULL;
          fChanged = TRUE;
        } /* endif */
      } /* endif */

      if ( pszExtension )
      { 
        if ( (stricmp( pszExtension, ".HTML" ) != 0) &&
             (stricmp( pszExtension, ".HTM" ) != 0) )
        {
          strcat( pszName, ".HTM" );
          fChanged = TRUE;
        } /* endif */
      }
      else
      {
        strcat( pszName, ".HTM" );
        fChanged = TRUE;
      } /* endif */
      break;

    case XMLFILEFORMAT :
      if ( pszExtension )
      { 
        if ( (stricmp( pszExtension, ".HTM" ) == 0) ||
            (stricmp( pszExtension, ".HTML" ) == 0) ||
            (stricmp( pszExtension, ".TXT" ) == 0) ||
            (stricmp( pszExtension, ".RPT" ) == 0) ||
            (stricmp( pszExtension, ".CNT" ) == 0) )
        {
          *pszExtension = EOS;
          pszExtension = NULL;
          fChanged = TRUE;
        } /* endif */
      } /* endif */

      if ( pszExtension )
      { 
        if ( stricmp( pszExtension, ".XML" ) != 0 )
        {
          strcat( pszName, ".XML" );
          fChanged = TRUE;
        } /* endif */
      }
      else
      {
        strcat( pszName, ".XML" );
        fChanged = TRUE;
      } /* endif */
      break;

    case ASCII :
    default:
      if ( pszExtension )
      { 
        if ( (stricmp( pszExtension, ".HTM" ) == 0) ||
            (stricmp( pszExtension, ".HTML" ) == 0) ||
            (stricmp( pszExtension, ".XML" ) == 0) )
        {
          *pszExtension = EOS;
          pszExtension = NULL;
          fChanged = TRUE;
        } /* endif */
      } /* endif */

      if ( !pszExtension )
      { 
        strcat( pszName, ".TXT" );
        fChanged = TRUE;
      } /* endif */
      break;
  } /*endswitch */

  return ( fChanged );
}
