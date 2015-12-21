//+----------------------------------------------------------------------------+
//|OtmAutoVerUp.cpp     OTM Profile Manager function                           |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2015, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:             Flora Lee                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:        This is module contains some functions which are used   |
//|                    during profile settings management                      |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
#include "OtmProfileMgrDlg.h"


OtmProfileMgrDlg::OtmProfileMgrDlg(void)
{
}


OtmProfileMgrDlg::~OtmProfileMgrDlg(void)
{
}

int OtmProfileMgrDlg::OtmProfileMgrDlgOpen(HINSTANCE & hDllInst)
{
    // start dialog box
    HWND hwnd = (HWND)UtlQueryULong(QL_TWBFRAME);
    INT_PTR nRes = DialogBoxParam(hDllInst, MAKEINTRESOURCE(IDD_DLG_PROFILE_SET_MGR), hwnd, OtmProfileMgrDlgProc, (LPARAM)this);
    return nRes;
}

// OtmProfileMgrDlg message handlers
INT_PTR CALLBACK OtmProfileMgrDlg::OtmProfileMgrDlgProc
(
HWND hwndDlg,
UINT msg,
WPARAM mp1,
LPARAM mp2
)
{
    LRESULT  mResult = FALSE;                // result value of procedure

    switch (msg)
    {
    case WM_INITDIALOG:
        {
            OtmProfileMgrDlg * pDlg = (OtmProfileMgrDlg *) mp2;
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (long) pDlg);
            pDlg->m_hwndDlg = hwndDlg;
            pDlg->OnInitDialog();
        }
        break;

    case WM_COMMAND:
        {
            OtmProfileMgrDlg * pDlg = (OtmProfileMgrDlg *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
            mResult = pDlg->OtmProfileMgrDlgCmd(mp1, mp2);
        }
        break;

    case WM_CLOSE:
        EndDialog(hwndDlg, IDCANCEL);
        break;

    default:
        break;
    }

    return mResult;
}

LRESULT OtmProfileMgrDlg::OtmProfileMgrDlgCmd
(
WPARAM mp1,
LPARAM mp2
)
{
    LRESULT mResult = TRUE;// TRUE = command is processed

    switch (HIWORD(mp1))
    {
    case EN_CHANGE:
        break;

    case BN_CLICKED:
        switch (LOWORD(mp1))
        {
        case IDOK:
            EndDialog(m_hwndDlg, IDOK);
            break;

        case IDCANCEL:
            EndDialog(m_hwndDlg, IDCANCEL);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    } /* endswitch */

    return(mResult);
}

BOOL OtmProfileMgrDlg::OnInitDialog()
{
    int nRes = NO_ERROR;
    nRes = ProfileMgrSheetLoad();
    return nRes;  // return TRUE unless you set the focus to a control
}

BOOL OtmProfileMgrDlg::ProfileMgrSheetLoad()
{
    BOOL bRes = NO_ERROR;

    RECT otmRect;
    TC_ITEM   itemTabCtrl;

    // remember adress of user area
    HINSTANCE hInst = (HINSTANCE) GetWindowLong(m_hwndDlg, GWL_HINSTANCE);
    HWND hwndTabCtrl = GetDlgItem(m_hwndDlg, IDC_TAB_MGR_SET);
    GetClientRect(hwndTabCtrl, &otmRect);
    TabCtrl_AdjustRect(hwndTabCtrl, FALSE, &otmRect);

    // leave some additional space at top
    otmRect.top += 20;
    MapWindowPoints(hwndTabCtrl, m_hwndDlg, (POINT *)&otmRect, 2);

    itemTabCtrl.mask = TCIF_TEXT | TCIF_PARAM;

    // create the appropriate TAB control and load the associated dialog
    // Installed sheet
    itemTabCtrl.pszText = TEXT(MGR_SET_TAB_NAME_STR);
    itemTabCtrl.lParam = 0;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, 0, (LPARAM)&itemTabCtrl);
    m_hwndPages[0] = CreateDialogParam(hInst,
                                       MAKEINTRESOURCE(IDD_DLG_PROFILE_MGR),
                                       m_hwndDlg,
                                       NULL,
                                       NULL);

    SetWindowPos(m_hwndPages[0], HWND_TOP,
                 otmRect.left, otmRect.top,
                 otmRect.right-otmRect.left, otmRect.bottom-otmRect.top, SWP_SHOWWINDOW);

    return bRes;
}
