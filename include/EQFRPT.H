/*! EQFRPT.H
	Description: Public include file for Counting Report handler
	Copyright (c) 1990-2013, International Business Machines Corporation and others. All rights reserved.
*/

#ifndef _OTMFUNC_H_INCLUDED
	#include "OTMFUNC.H"
#endif


typedef LONG HSESSION;



INT_PTR CALLBACK COUNTINGPROPDLGPROC (HWND, WINMSG, WPARAM, LPARAM);


MRESULT ReportListHandlerCallBack
(
  PHANDLERCOMMAREA pCommArea,
  HWND             hwnd,
  WINMSG           msg,
  WPARAM           mp1,
  LPARAM           mp2
);


INT_PTR CALLBACK RPTPROP_GENERAL_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
);


INT_PTR CALLBACK RPTPROP_PROFILE_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
);



INT_PTR CALLBACK RPTPROP_SETTINGS_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
);



INT_PTR CALLBACK RPTPROP_FACTSHEET_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
);




INT_PTR CALLBACK RPTPROP_MORE_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
);


INT_PTR CALLBACK RPTPROP_SHIPM_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
);


MRESULT RptListCallBack (PLISTCOMMAREA, HWND,WINMSG,WPARAM,LPARAM);


__declspec(dllexport)
USHORT MemFuncCreateCntReport
(
  HSESSION hSession,
  CHAR chDriveLetter,
  PSZ pszFolderName,
  PSZ pszDocuments,
  PREPORTTYPE pReportType,
  PSZ pszOutfileName,
  PSZ pszFormat,
  PSZ pszProfile,
  PREPORTSETTINGS pRepSettings,
  PFACTSHEET pFactSheet,
  USHORT usColumn,
  USHORT usCategory,
  PFINALFACTORS pFinalFactors,
  LONG lOptSecurity,
  BOOL bSingleShipment
);


