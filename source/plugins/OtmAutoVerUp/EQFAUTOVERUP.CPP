//+----------------------------------------------------------------------------+
//|EQFAUTOVERUP.CPP     Auto Version Up function                               |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:             Flora Lee                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:        This is module contains some functions which are used   |
//|                    during auto version up                                  |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "eqfautoverup.h"             // type definitions and function prototypes

int AutoVesrionUpProps(HINSTANCE hInstance, char * szOtmPath, LPTSTR lpCmdLine)
{
    // set path as global value
    int nRC = NO_ERROR;

    HWND hwndOpenTM2 = FindWindow(OPENTM2_APP_NAME_STR, NULL);

    AutoVerUpStart(szOtmPath);

    nRC = ProcessParameter(lpCmdLine, gpAtvIda);
    if (NO_ERROR != nRC)
    {
        MessageBox(hwndOpenTM2, OtmGetMessageFromCode(nRC), AUTO_VER_UP_NAME_STR, MB_ICONSTOP | MB_OK);
        return nRC;
    }


    // check or set the config file
    nRC = CheckConfig();
    if (nRC)
    {
        MessageBox(hwndOpenTM2, OtmGetMessageFromCode(nRC), AUTO_VER_UP_NAME_STR, MB_ICONSTOP | MB_OK);
        return nRC;
    }

    // remove needless files
    NeedlessFileCheck();

    // new version check
    nRC = CheckNewVersion(gpAtvIda->strOtmRevision);

    if (VAL_NO_NEED_OPEN == nRC)
    {
        if (!gpAtvIda->bFromMenu)
        {
            return nRC;
        }
    }
    else if (VAL_NOT_HAS_NEW_VER == nRC)
    {
        if (gpAtvIda->bFromMenu)
        {
            /*int nID = MessageBox(hwndOpenTM2, INFO_NO_NEW_VERSION_STR, AUTO_VER_UP_NAME_STR, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2);
            if (IDNO == nID)
            {
                return nRC;
            }*/
        }
        else
        {
            return nRC;
        }
    }
    else if ((nRC != NO_ERROR) && (nRC != VAL_HAS_NEW_VER) && (nRC != VAL_NO_NEED_OPEN))
    {
        //if (gpAtvIda->bFromMenu)
        //{
            MessageBox(hwndOpenTM2, OtmGetMessageFromCode(nRC), AUTO_VER_UP_NAME_STR, MB_ICONSTOP | MB_OK);
        //}
        return nRC;
    }
    else if (nRC == VAL_HAS_NEW_VER && !gpAtvIda->bFromMenu)
    {
        nRC = MessageBox(hwndOpenTM2, INFO_FOUND_NEW_VERSION_STR, AUTO_VER_UP_NAME_STR, MB_ICONQUESTION | MB_YESNO);
        if (IDNO == nRC)
        {
            nRC = NO_ERROR;
            return nRC;
        }
    }

    HWND hWndAVUDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(ID_AUTO_VERUP_DLG), NULL, AutoVerUpDlgProc, (LPARAM) gpAtvIda);

    MSG msgAutoVerUp;

    while (GetMessage(&msgAutoVerUp, NULL, NULL, NULL))
    {
        /*if (msgAutoVerUp.message == WM_KEYDOWN)
        {
            SendMessage(hWndAVUDlg, msgAutoVerUp.message, msgAutoVerUp.wParam, msgAutoVerUp.lParam);
        }
        else */if(!IsDialogMessage(hWndAVUDlg, &msgAutoVerUp))
        {
            TranslateMessage(&msgAutoVerUp);
            DispatchMessage(&msgAutoVerUp);
        }
    }

    return msgAutoVerUp.wParam;
}

void AutoVerUpStart(const char * szOtmPath)
{
    gbLogOpened = FALSE;
    if (!glogAutoVerUp.isOpen())
    {
        glogAutoVerUp.open(LOG_AUTO_VER_UP_NAME);
        gbLogOpened = TRUE;
    }

    memset(gstrgConfigFile, 0x00, sizeof(gstrgConfigFile));
    memset(gstrgOtmPath,    0x00, sizeof(gstrgOtmPath));

    strncpy(gstrgOtmPath, szOtmPath, strlen(szOtmPath));
    sprintf(gstrgConfigFile, "%s\\%s\\%s", szOtmPath, KEY_PLUGINS_PATH, AUTO_VER_UP_CONF_FILE_NAME);

    // initial for progress bar
    INITCOMMONCONTROLSEX otmIcc = {sizeof(otmIcc), ICC_PROGRESS_CLASS};
    InitCommonControlsEx(&otmIcc);

    gpAtvIda = new AUTOVERUPPROPIDA;

    if (NULL == gOtmParser)
    {
        gOtmParser = new COtmXmlParser();
    }
}

INT_PTR CALLBACK AutoVerUpDlgProc
(
HWND hwndDlg,                       /* handle of dialog window             */
UINT msg,
WPARAM mp1,
LPARAM mp2
)
{
    LRESULT  mResult = MRFROMSHORT(FALSE);                // result value of procedure
    ghwndDlg = hwndDlg;

    switch (msg)
    {
    case WM_INITDIALOG:
        {
            OtmCenterWindow(hwndDlg);
            SetForegroundWindow(hwndDlg);
            // Initial every sheet
            AutoVerUpDlgSheetLoad(hwndDlg, mp2);
        }
        break;

    case WM_COMMAND:
        mResult = AutoVerUpPropCommand(hwndDlg, mp1, mp2);
        break;

    case WM_NOTIFY:
        mResult = AutoVerUpPropPropertySheetNotification(hwndDlg, mp1, mp2);
        break;

    case WM_CLOSE:
        {
            USHORT nItem = 0;
            /***********************************************************/
            /* free all allocated pages as well as the registration    */
            /* of the modeless dialog                                  */
            /***********************************************************/
            while (ghwndPages[nItem])
            {
//              UtlUnregisterModelessDlg(pIda->hwndPages[nItem]);
                DestroyWindow(ghwndPages[nItem]);
                nItem++;
            } /* endwhile */

            if (gbLogOpened)
            {
                glogAutoVerUp.close();
            }
        }
        DestroyWindow(hwndDlg);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return TRUE;

    case WM_SIZE:
//        ResizeWholeDlg(hwndDlg, LOWORD( mp2 ), HIWORD( mp2 ));
        break;

    default:
//        mResult = DefWindowProc(hwndDlg, msg, mp1, mp2);
        break;
    } /* endswitch */

    return mResult;
} /* end of PluginManagerDlgProp */

LRESULT AutoVerUpPropPropertySheetNotification
(
HWND hwndDlg,
WPARAM  mp1,
LPARAM  mp2
)
{
    NMHDR * pNMHdr;
    ULONG       ulTabCtrl;
    LRESULT      mResult = FALSE;
    pNMHdr = (LPNMHDR)mp2;
    HWND hwndTabCtrl;
    TC_ITEM Item;

    switch (pNMHdr->code)
    {
    case TCN_SELCHANGE:
        // show the table
        hwndTabCtrl = GetDlgItem(hwndDlg, IDC_AUTO_VERUP_TAB);
        ulTabCtrl = TabCtrl_GetCurSel(hwndTabCtrl);
        memset(&Item, 0, sizeof(Item));
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem(hwndTabCtrl, ulTabCtrl, &Item);
        ulTabCtrl = Item.lParam;
        ShowWindow(ghwndPages[ulTabCtrl], SW_SHOW);
        break;
    case TCN_SELCHANGING:
        // hide the tab
        hwndTabCtrl = GetDlgItem(hwndDlg, IDC_AUTO_VERUP_TAB);
        ulTabCtrl = TabCtrl_GetCurSel(hwndTabCtrl);
        memset(&Item, 0, sizeof(Item));
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem(hwndTabCtrl, ulTabCtrl, &Item);
        ulTabCtrl = Item.lParam;

        SendMessage(hwndDlg, TCM_SETCURSEL, ulTabCtrl, 0L);
        ShowWindow(ghwndPages[ulTabCtrl], SW_HIDE);
        break;
    default:
        break;
    }
    return mResult;
} /* end of function PluginMgPropPropertySheetNotification */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TwbSysPropertySheetLoad                                  |
//+----------------------------------------------------------------------------+
//|Function call:     TwbSysPropertySheetLoad(hwndDlg, mp2);                   |
//+----------------------------------------------------------------------------+
//|Description:       handle changes on the tab page                           |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwndDlg   handle of the dialog                      |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   LRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     create any pages,                                        |
//|                   load the tabctrl text                                    |
//|                   load the (modeless) dialog, register it and position into|
//|                     tab area                                               |
//|                   return                                                   |
//+----------------------------------------------------------------------------+
BOOL AutoVerUpDlgSheetLoad(HWND hwndDlg, LPARAM lpParam)
{
    BOOL      fOK = TRUE;
    TC_ITEM   TabCtrlItem;
    USHORT    nItem = 0;
    HWND      hwndTabCtrl;
    HINSTANCE hInst;
    char strMsg[100];
    memset(strMsg, 0x00, sizeof(strMsg));

    RECT otmRect;
    // remember adress of user area
    hInst = (HINSTANCE) GetWindowLong(hwndDlg, GWL_HINSTANCE);
    hwndTabCtrl = GetDlgItem(hwndDlg, IDC_AUTO_VERUP_TAB);
    GetClientRect(hwndTabCtrl, &otmRect);
    TabCtrl_AdjustRect(hwndTabCtrl, FALSE, &otmRect);

    // leave some additional space at top
    otmRect.top += 20;
    MapWindowPoints(hwndTabCtrl, hwndDlg, (POINT *)&otmRect, 2);

    TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM;

    // create the appropriate TAB control and load the associated dialog
    // Installed sheet
    TabCtrlItem.pszText = TEXT(AVT_TAB_NAME_1_STR);
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    ghwndPages[0] = CreateDialogParam(hInst,
                                     MAKEINTRESOURCE(ID_AUTO_VERUP_LST_DLG),
                                     hwndDlg,
                                     AutoVerUpTabProc,
                                     lpParam);
    SetWindowPos(ghwndPages[0], HWND_TOP,
                 otmRect.left, otmRect.top,
                 otmRect.right-otmRect.left, otmRect.bottom-otmRect.top, SWP_SHOWWINDOW);
    nItem++;

    // Settings sheet
    TabCtrlItem.pszText = TEXT(AVT_TAB_NAME_2_STR);
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    ghwndPages[1] = CreateDialogParam(hInst,
                                     MAKEINTRESOURCE(ID_AUTO_VERUP_SET_DLG),
                                     hwndDlg,
                                     AutoVerUpSetProc,
                                     NULL);

    SetWindowPos(ghwndPages[1], HWND_TOP,
                 otmRect.left, otmRect.top,
                 otmRect.right-otmRect.left, otmRect.bottom-otmRect.top, SWP_SHOWWINDOW);
    nItem++;

    // it exists such case, if down xml failed, that mean nRC is not NO_ERROR, the setting tab should first be shown
    int nTabInx = 0;
    PAUTOVERUPPROPIDA pAtvIda = (PAUTOVERUPPROPIDA) lpParam;
    if (pAtvIda->nRC)
    {
        nTabInx = 1;
    }

    // hide all dialog pages but the first one
    if (fOK)
    {
        int iInx = 0;
        while (ghwndPages[iInx])
        {
            if (iInx == nTabInx)
            {
                TabCtrl_SetCurSel(hwndTabCtrl, iInx);
                ShowWindow(ghwndPages[iInx], SW_SHOW);
            }
            else
            {
                ShowWindow(ghwndPages[iInx], SW_HIDE);
            }
            iInx++;
        }
    }

    if (!fOK)
    {
//        POSTEQFCLOSE(hwndDlg, FALSE);
    }

    return fOK;
}

INT_PTR CALLBACK AutoVerUpTabProc
(
HWND hwndTabDlg,
UINT msg,
WPARAM mp1,
LPARAM mp2
)
{
    LRESULT  mResult = MRFROMSHORT(FALSE);                // result value of procedure
    HWND               autoVerUpList;
    LV_COLUMN          lvCol;
    char strMsg[100];
    char strValue[MAX_PATH];

    memset(strMsg, 0x00, sizeof(strMsg));
    memset(strValue, 0x00, sizeof(strValue));

    autoVerUpList = GetDlgItem(hwndTabDlg, IDC_AUTO_VERUP_LIST);
    switch (msg)
    {
    case WM_INITDIALOG:
    {
        // reload dialog
        SetDlgItemText(hwndTabDlg, IDC_STC_LONG_DESC, EMPTY_STR);
        ListView_SetExtendedListViewStyle(autoVerUpList, LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

        // Initial column
        lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvCol.fmt = LVCFMT_LEFT;

        // component name
        lvCol.iSubItem = 0;
        lvCol.cx = 200;
        lvCol.pszText = AVT_LST_COLUMN_1_STR;
        ListView_InsertColumn(autoVerUpList, 0, &lvCol);

        // current version
        lvCol.iSubItem = 1;
        lvCol.cx = 100;
        lvCol.pszText = AVT_LST_COLUMN_2_STR;
        ListView_InsertColumn(autoVerUpList, 1, &lvCol);

        // available version
        lvCol.iSubItem = 2;
        lvCol.cx = 100;
        lvCol.pszText = AVT_LST_COLUMN_3_STR;
        ListView_InsertColumn(autoVerUpList, 2, &lvCol);

        // available date
        lvCol.iSubItem = 3;
        lvCol.cx = 130;
        lvCol.pszText = AVT_LST_COLUMN_4_STR;
        ListView_InsertColumn(autoVerUpList, 3, &lvCol);

        // short description
        lvCol.iSubItem = 4;
        lvCol.cx = 200;
        lvCol.pszText = AVT_LST_COLUMN_5_STR;
        ListView_InsertColumn(autoVerUpList, 4, &lvCol);

        // severity
        lvCol.iSubItem = 5;
        lvCol.cx = 80;
        lvCol.pszText = AVT_LST_COLUMN_6_STR;
        ListView_InsertColumn(autoVerUpList, 5, &lvCol);

        // impact on opentm2 assets
        lvCol.iSubItem = 6;
        lvCol.cx = 200;
        lvCol.pszText = AVT_LST_COLUMN_7_STR;
        ListView_InsertColumn(autoVerUpList, 6, &lvCol);

        // action to be taken after installation
        lvCol.iSubItem = 7;
        lvCol.cx = 200;
        lvCol.pszText = AVT_LST_COLUMN_8_STR;
        ListView_InsertColumn(autoVerUpList, 7, &lvCol);

        PAUTOVERUPPROPIDA pAtvIda = (PAUTOVERUPPROPIDA) mp2;
        pAtvIda->nRC = UpdateAutoVerUpLst(hwndTabDlg, pAtvIda, FALSE);

        break;
    }

    case WM_OTM_SET_UPDATE:
        UpdateAutoVerUpLst(hwndTabDlg, gpAtvIda, TRUE);
        break;

    case WM_COMMAND:
        switch (WMCOMMANDID(mp1, mp2))
        {
        case IDCANCEL:
            PostMessage(hwndTabDlg, WM_CLOSE, FALSE, 0L);
            break;
        case IDC_BTN_UPDATE:
            {
                UpdateSelectedComponent(hwndTabDlg);
            }
            break;
        }
        break;

    case WM_HELP:
        /*************************************************************/
        /* pass on a HELP_WM_HELP request                            */
        /*************************************************************/
        /*EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
        &hlpsubtblTwbSysPropGeneral[0]);*/
        mResult = TRUE;  // message processed
        break;

    case WM_CLOSE:
        PostMessage(hwndTabDlg, WM_CLOSE, FALSE, 0L);
        break;

    case WM_NOTIFY:
        AutoVerUpLstNotify(hwndTabDlg, mp1, mp2);
        break;

    default:
//        mResult = DefWindowProc(hwndDlg, msg, mp1, mp2);
        break;
    } /* endswitch */

    return mResult;
}

LRESULT AutoVerUpLstNotify(HWND hwndDlg, WPARAM  mp1, LPARAM  mp2)
{
    NMHDR * pNMHdr;
    LRESULT mResult = FALSE;

    pNMHdr = (LPNMHDR)mp2;
    switch (pNMHdr->code)
    {
    case NM_CLICK:
        {
            // when check box checked
            HWND autoVerUpList = GetDlgItem(hwndDlg, IDC_AUTO_VERUP_LIST);
            int iSelInx = -1;

            char strLstName[MAX_LEN];
            memset(strLstName, 0x00, sizeof(strLstName));

            while((iSelInx = ListView_GetNextItem(autoVerUpList, iSelInx, LVNI_SELECTED)) != -1)
            {
                // set the check state when select the item
                UINT nState = ListView_GetCheckState(autoVerUpList, iSelInx);
                ListView_SetCheckState(autoVerUpList, iSelInx, !nState);

                SetDlgItemText(hwndDlg, IDC_STC_LONG_DESC, EMPTY_STR);
                ListView_GetItemText(autoVerUpList, iSelInx, 0, strLstName, MAX_LEN);

                BOOL bFound = FALSE;
                for (int iInx = 0; iInx < gOtmParser->GetComponentsCnt(); iInx++)
                {
                    if (IsFixpack(strLstName))
                    {
                        char strId[MAX_LEN];
                        char strCompName[MAX_LEN];
                        char strCompVer[MAX_VER_LEN];

                        memset(strId,        0x00, sizeof(strId));
                        memset(strCompName,  0x00, sizeof(strCompName));
                        memset(strCompVer,   0x00, sizeof(strCompVer));

                        SplitFixpackName(strLstName, strCompName, strCompVer, strId);

                        for (int jInx = 0; jInx < gOtmParser->GetCompFixpacksCnt(iInx); jInx++)
                        {
                            if (!stricmp(strCompName, gOtmParser->GetComponentName(iInx)) &&
                                !stricmp(strCompVer,  gOtmParser->GetComponentVersion(iInx)) &&
                                !stricmp(strId,       gOtmParser->GetCompFixpackId(iInx, jInx)))
                            {
                                SetDlgItemText(hwndDlg, IDC_STC_LONG_DESC, gOtmParser->GetCompFixpackLongDscp(iInx, jInx));
                                bFound = TRUE;
                                break;
                            }
                        }

                        if (bFound)
                        {
                            break;
                        }
                    }
                    else
                    {
                        if (!stricmp(gOtmParser->GetComponentName(iInx), strLstName))
                        {
                            SetDlgItemText(hwndDlg, IDC_STC_LONG_DESC, gOtmParser->GetComponentLongDscp(iInx));
                            break;
                        }
                    }
                }
            }
        }
        break;
    default:
        break;
    }
    return mResult;
} /* end of function AutoVerUpLstNotify */

LRESULT AutoVerUpPropCommand
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
    LRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed

    switch (WMCOMMANDID(mp1, mp2))
    {
    case IDCANCEL:
        PostMessage(hwndDlg, WM_CLOSE, FALSE, 0L);
        break;

    default:
        break;
    } /* endswitch */

    return(mResult);
} /* end of PluginManagerPropCommand */

INT_PTR CALLBACK AutoVerUpSetProc
(
HWND hwndTabDlg,                       /* handle of dialog window             */
UINT msg,
WPARAM mp1,
LPARAM mp2
)
{
    LRESULT  mResult = MRFROMSHORT(FALSE);                // result value of procedure

    switch (msg)
    {
    case WM_INITDIALOG:
        SetInfoFromConfig(hwndTabDlg);
        EnableWindow(GetDlgItem(hwndTabDlg, IDC_EDT_URL), FALSE);
        break;

    case WM_COMMAND:
        switch (WMCOMMANDID(mp1, mp2))
        {
        case IDC_BTN_SAVE:
            SaveInfoToConfig(hwndTabDlg, FALSE);
            break;

        case IDC_BTN_CHECK_NOW:
            CheckNowRefreshUI(hwndTabDlg);
            break;

        case IDC_BTN_TEST_CONNECT:
            TestConnection(hwndTabDlg);
            break;

        case IDCANCEL:
            SetInfoFromConfig(hwndTabDlg);
            break;
        } /* endswitch */
        break;

    case WM_HELP:
        /*************************************************************/
        /* pass on a HELP_WM_HELP request                            */
        /*************************************************************/
        /*EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
        &hlpsubtblTwbSysPropGeneral[0]);*/
        mResult = TRUE;  // message processed
        break;

    case WM_CLOSE:
//        POSTEQFCLOSE(hwndDlg, FALSE);
        break;

    default:
//        mResult = DefWindowProc(hwndDlg, msg, mp1, mp2);
        break;
    } /* endswitch */

    return mResult;
}

int UpdateAutoVerUpLst(HWND hwndTabDlg, PAUTOVERUPPROPIDA pAtvIda, BOOL bDownload)
{
    char strXml[MAX_PATH];
    memset(strXml, 0x00, sizeof(strXml));
    sprintf(strXml, "%s\\WIN\\%s", gstrgOtmPath, AUTO_VER_UP_XML);

    int nRC = NO_ERROR;
    HWND hwndAutoVerUpLst = GetDlgItem(hwndTabDlg, IDC_AUTO_VERUP_LIST);

    LV_ITEM lvItem;
    lvItem.mask = LVIF_TEXT;

    ListView_DeleteAllItems(hwndAutoVerUpLst);
    int nCnt = gOtmParser->GetComponentsCnt();
    int nLstInx = 0;
    for (int iInx = 0; iInx < nCnt; iInx++)
    {
        // confirm add the new version
        char * strVerNew = gOtmParser->GetComponentVersion(iInx);
        char * strVerOld;
        if (!strnicmp(OPENTM2_APP_NAME_STR, gOtmParser->GetComponentName(iInx), strlen(OPENTM2_APP_NAME_STR)))
        {
            strVerOld = pAtvIda->strOtmRevision;
        }
        else
        {
            strVerOld = TEXT("1.0");
        }

        if (IsNewVersion(strVerOld, strVerNew) > VER_SAM_VAL)
        {
            // column 1: compomnet name
            lvItem.iItem = nLstInx;
            lvItem.iSubItem = 0;
            lvItem.pszText = gOtmParser->GetComponentName(iInx);
            ListView_InsertItem(hwndAutoVerUpLst, &lvItem);

            // column 2: current version
            lvItem.iSubItem = 1;
            lvItem.pszText = strVerOld;
            ListView_SetItem(hwndAutoVerUpLst, &lvItem);

            // column 3: available version
            lvItem.iSubItem = 2;
            lvItem.pszText = strVerNew;
            ListView_SetItem(hwndAutoVerUpLst, &lvItem);

            // column 4: available date
            lvItem.iSubItem = 3;
            lvItem.pszText = gOtmParser->GetComponentDate(iInx);
            ListView_SetItem(hwndAutoVerUpLst, &lvItem);

            // column 5: short description
            lvItem.iSubItem = 4;
            lvItem.pszText = gOtmParser->GetComponentShortDscp(iInx);
            ListView_SetItem(hwndAutoVerUpLst, &lvItem);

            // column 6: severity
            lvItem.iSubItem = 5;
            lvItem.pszText = gOtmParser->GetComponentSeverity(iInx);
            ListView_SetItem(hwndAutoVerUpLst, &lvItem);

            // column 7: impact on opentm2 assets
            lvItem.iSubItem = 6;
            lvItem.pszText = gOtmParser->GetComponentImpact(iInx);
            ListView_SetItem(hwndAutoVerUpLst, &lvItem);

            // column 8: action to be taken after insatllation
            lvItem.iSubItem = 7;
            lvItem.pszText = gOtmParser->GetComponentAfterAction(iInx);
            ListView_SetItem(hwndAutoVerUpLst, &lvItem);

            // add fixpacks info if necessary
            glogAutoVerUp.writef("%s, New %s > Old %s, add to list", gOtmParser->GetComponentName(iInx), strVerNew, strVerOld);
            nLstInx++;
            // only add fixpack info for the local version
            // nLstInx += AddFixpacksToList(hwndAutoVerUpLst, nInx, nLstInx, TRUE);
        }
        else if (IsNewVersion(strVerOld, strVerNew) == VER_SAM_VAL)
        {
            // add fixpacks info if necessary
            glogAutoVerUp.writef("%s, New %s = Old %s, add to list", gOtmParser->GetComponentName(iInx), strVerNew, strVerOld);
            nLstInx += AddFixpacksToList(hwndAutoVerUpLst, pAtvIda, iInx, nLstInx, FALSE);
        }
        else
        {
            glogAutoVerUp.writef("%s, New %s < Old %s, do nothing", gOtmParser->GetComponentName(iInx), strVerNew, strVerOld);
        }
    }

    return nRC;
}

int AddFixpacksToList(HWND hwndAutoVerUpLst, PAUTOVERUPPROPIDA pAtvIda, int nInx, int nLstStartInx, BOOL bNewVer)
{
    int nAddCnt = 0;
    int nLstInx = nLstStartInx;

    LV_ITEM lvItem;
    lvItem.mask = LVIF_TEXT;

    int nFixpacksCnt = gOtmParser->GetCompFixpacksCnt(nInx);
    for (int jInx = 0; jInx < nFixpacksCnt; jInx++)
    {
        // set compment name for fixpack
        char strName[MAX_BUF_SIZE];
        memset(strName, 0x00, sizeof(strName));
        sprintf(strName, "%s_%s_%s_%s", KEY_FIXPACK, gOtmParser->GetComponentName(nInx), 
                gOtmParser->GetComponentVersion(nInx), gOtmParser->GetCompFixpackId(nInx, jInx));

        // if it is not new version, then it should confirm whether the fixpack installed or not
        if (!bNewVer && IsFixpackInstalled(strName))
        {
            continue;
        }

        // column 1: component name
        lvItem.iItem = nLstInx;
        lvItem.iSubItem = 0;
        lvItem.pszText = strName;
        ListView_InsertItem(hwndAutoVerUpLst, &lvItem);

        char * strVerOld;
        if (!strnicmp(OPENTM2_APP_NAME_STR, gOtmParser->GetComponentName(nInx), strlen(OPENTM2_APP_NAME_STR)))
        {
            strVerOld = pAtvIda->strOtmRevision;
        }
        else
        {
            strVerOld = TEXT("1.0");
        }
        // column 2: installed version
        lvItem.iSubItem = 1;
        lvItem.pszText = strVerOld;
        ListView_SetItem(hwndAutoVerUpLst, &lvItem);

        // column 3: available version
        lvItem.iSubItem = 2;
        lvItem.pszText = gOtmParser->GetComponentVersion(nInx);
        ListView_SetItem(hwndAutoVerUpLst, &lvItem);

        // column 4: available date
        lvItem.iSubItem = 3;
        lvItem.pszText = gOtmParser->GetCompFixpackDate(nInx, jInx);
        ListView_SetItem(hwndAutoVerUpLst, &lvItem);

        // column 5: description
        lvItem.iSubItem = 4;
        lvItem.pszText = gOtmParser->GetCompFixpackShortDscp(nInx, jInx);
        ListView_SetItem(hwndAutoVerUpLst, &lvItem);

        // column 6: severity
        lvItem.iSubItem = 5;
        lvItem.pszText = gOtmParser->GetComponentSeverity(nInx);
        ListView_SetItem(hwndAutoVerUpLst, &lvItem);

        // column 7: impact on opentm2 assets
        lvItem.iSubItem = 6;
        lvItem.pszText = gOtmParser->GetComponentImpact(nInx);
        ListView_SetItem(hwndAutoVerUpLst, &lvItem);

        // column 8: action to be taken after insatllation
        lvItem.iSubItem = 7;
        lvItem.pszText = gOtmParser->GetComponentAfterAction(nInx);
        ListView_SetItem(hwndAutoVerUpLst, &lvItem);

        nLstInx++;
        nAddCnt++;

        // to list the fixpack, the installed status should be changed
        SetFixpackState(strName, STATE_UNINSTALL, TRUE);
    }

    return nAddCnt;
}

BOOL IsFixpackInstalled(const char * strFixpack)
{
    BOOL bInstalled = FALSE;
    char strFixpConf[MAX_PATH];
    memset(strFixpConf, 0x00, sizeof(strFixpConf));

    sprintf(strFixpConf, "%s\\%s\\%s", gstrgOtmPath, KEY_PLUGINS_PATH, AUTO_VER_UP_FIXP_CONF_NAME);

    if (OTM_NOT_FOUND == access(strFixpConf, 0))
    {
        return bInstalled;
    }

    char strId[MAX_LEN];
    char strCompName[MAX_LEN];
    char strCompVer[MAX_VER_LEN];

    memset(strId,        0x00, sizeof(strId));
    memset(strCompName,  0x00, sizeof(strCompName));
    memset(strCompVer,   0x00, sizeof(strCompVer));

    SplitFixpackName(strFixpack, strCompName, strCompVer, strId);

    // set app value of config file
    char strApp[MAX_LEN];
    memset(strApp,  0x00, sizeof(strApp));
    sprintf(strApp, "%s_%s", strCompName, strCompVer);

    // read the fixpack state from config file
    int nState = GetPrivateProfileInt(strApp, strId, FIXPACK_STATE_DFT, strFixpConf);

    if (nState)
    {
        bInstalled = TRUE;
    }
    else
    {
        bInstalled = FALSE;
    }

    return bInstalled;
}

int SetFixpackState(const char * strFixpack, int nState, BOOL bCreate)
{
    int nRC = NO_ERROR;

    char strFixpConf[MAX_PATH];
    memset(strFixpConf, 0x00, sizeof(strFixpConf));

    sprintf(strFixpConf, "%s\\%s\\%s", gstrgOtmPath, KEY_PLUGINS_PATH, AUTO_VER_UP_FIXP_CONF_NAME);
    // if the file not exist, create the file first
    if (OTM_NOT_FOUND == access(strFixpConf, 0) && bCreate)
    {
        FILE * pConf = fopen(strFixpConf, "wb+");
        if (NULL == pConf)
        {
            nRC = ERROR_OTM_OPEN_FILE_A;
            return nRC;
        }
        fclose(pConf);
    }

    char strId[MAX_LEN];
    char strCompName[MAX_LEN];
    char strCompVer[MAX_VER_LEN];

    memset(strId,        0x00, sizeof(strId));
    memset(strCompName,  0x00, sizeof(strCompName));
    memset(strCompVer,   0x00, sizeof(strCompVer));

    SplitFixpackName(strFixpack, strCompName, strCompVer, strId);

    // set app value of config file
    char strApp[MAX_LEN];
    memset(strApp,  0x00, sizeof(strApp));
    sprintf(strApp, "%s_%s", strCompName, strCompVer);

    char strValue[MAX_LEN];
    memset(strValue, 0x00, sizeof(strValue));
    sprintf(strValue, "%d", nState);

    // Write the status to file
    WritePrivateProfileString(strApp, strId, strValue, strFixpConf);

    return nRC;
}

int UpdateSelectedComponent(HWND hwndTabDlg)
{
    int nRC = NO_ERROR;
    HWND hwndDlgLst = GetDlgItem(hwndTabDlg, IDC_AUTO_VERUP_LIST);
    char strMsg[MAX_BUF_SIZE];
    memset(strMsg, 0x00, sizeof(strMsg));

    int nCheckedCnt  = 0;
    BOOL bHasOTM     = FALSE;
    for (int nInx = 0; nInx < ListView_GetItemCount(hwndDlgLst); nInx++)
    {
        if (!ListView_GetCheckState(hwndDlgLst, nInx))
        {
            continue;
        }

        nCheckedCnt++;
        // get the name of the component
        char strName[MAX_LEN];
        memset(strName, 0x00, sizeof(strName));
        ListView_GetItemText(hwndDlgLst, nInx, 0, strName, sizeof(strName));

        if (!strnicmp(OPENTM2_APP_NAME_STR, strName, strlen(OPENTM2_APP_NAME_STR)))
        {
            bHasOTM = TRUE;
        }

        if (IsFixpack(strName))
        {
            int nChked = IsFixpackParentChked(hwndDlgLst, strName, 0, 2);
            if (PARENT_NOT_CHECKED == nChked)
            {
                MessageBox(hwndTabDlg, ERROR_FIXPACK_PARENT_NOT_CHECKED, AUTO_VER_UP_NAME_STR, MB_ICONEXCLAMATION | MB_OK);
                return nRC;
            }
        }
    }

    // if there is no item is selected, just return
    if (0 == nCheckedCnt)
    {
        return nRC;
    }

    if (bHasOTM && (nCheckedCnt > 1))
    {
        sprintf(strMsg, ERROR_NOT_INSTALL_ALONE_B_STR, OPENTM2_APP_NAME_STR);
        MessageBox(hwndTabDlg, strMsg, AUTO_VER_UP_NAME_STR, MB_ICONEXCLAMATION | MB_OK);
        nRC = ERROR_NOT_INSTALL_ALONE_B;
        return nRC;
    }

    BOOL bStart = FALSE;
    BOOL bFirst = TRUE;
    int nID = IDYES;
    while ((IDYES == nID) || (IDRETRY == nID))
    {
        if (IsProgramRunning(OTM_APPL_EXE) || IsProgramRunning(OTM_APPL_STARTER_EXE))
        {
            if (!bStart)
            {
                nID = MessageBox(hwndTabDlg, INFO_OPENTM2_OPEN_STR, AUTO_VER_UP_NAME_STR, MB_ICONEXCLAMATION | MB_YESNO);
                if (IDYES == nID)
                {
                    CloseOpenTM2();
                    bStart = TRUE;
                    continue;
                }
            }
            else
            {
                if (bFirst)
                {
                    nID = MessageBox(hwndTabDlg, INFO_OPENTM2_CLOSE_CONFIRM_STR, AUTO_VER_UP_NAME_STR, MB_YESNO);
                    if (IDYES == nID)
                    {
                        bFirst = FALSE;
                        continue;
                    }
                }
                else
                {
                    nID = MessageBox(hwndTabDlg, INFO_OPENTM2_CLOSE_RETRY_STR, AUTO_VER_UP_NAME_STR, MB_RETRYCANCEL);
                    if (IDRETRY == nID)
                    {
                        CloseOpenTM2();
                        bFirst = TRUE;
                        continue;
                    }
                }
            }
        }
        else
        {
            nID = IDOK;
            break;
        }
    }

    if (nID != IDOK)
    {
        return nRC;
    }

    // each task is divided to two parst: start and end, so the total count should multiply by 2
    nCheckedCnt = nCheckedCnt * 2;
	gOtmHttps = new COtmHttps();
    HINSTANCE hInst = (HINSTANCE) GetWindowLong(hwndTabDlg, GWL_HINSTANCE);
    ghwndPrgCtrlDlg = CreateDialogParam(hInst,
                                        MAKEINTRESOURCE(IDD_PROGRESS_CTRL_DLG),
                                        ghwndDlg,
                                        DownloadPCDlgProc,
                                        nCheckedCnt);
    ShowWindow(ghwndPrgCtrlDlg, SW_SHOW);
    EnableWindow(ghwndDlg, FALSE);

    PDOWNLOADPARAM2 pDownLoadParam2 = new DOWNLOADPARAM2;

    pDownLoadParam2->hwndTabDlg = hwndTabDlg;
    pDownLoadParam2->hwndPCDlg  = ghwndPrgCtrlDlg;
    if (NULL == _beginthreadex(NULL, 0, DownloadPCThreadProc, pDownLoadParam2, 0, 0))
    {
        nRC = ERROR_CREATE_THREAD_A;
    }

    return nRC;
}

unsigned int __stdcall DownloadPCThreadProc(LPVOID lpParameter)
{
    int nRC = NO_ERROR;

    char strMsg[MAX_BUF_SIZE];
    memset(strMsg, 0x00, sizeof(strMsg));

    PDOWNLOADPARAM2 pDownLoadParam2 = (PDOWNLOADPARAM2) lpParameter;
    HWND hwndDlgLst = GetDlgItem(pDownLoadParam2->hwndTabDlg, IDC_AUTO_VERUP_LIST);

    int nTotalCnt = GetLstTotalCnt(hwndDlgLst);
    int nProcessedCnt = 0;
    int nSuccessCnt = 0;

    for (int nInx = 0; nInx < ListView_GetItemCount(hwndDlgLst); nInx++)
    {
        if (!ListView_GetCheckState(hwndDlgLst, nInx))
        {
            continue;
        }

        PDOWNLOADPARAM pDownloadParam = new DOWNLOADPARAM;
        memset(pDownloadParam->strDLUrl,    0x00, sizeof(pDownloadParam->strDLUrl));
        memset(pDownloadParam->strDLFile,   0x00, sizeof(pDownloadParam->strDLFile));
        memset(pDownloadParam->strDLType,   0x00, sizeof(pDownloadParam->strDLType));
        memset(pDownloadParam->strMethod,   0x00, sizeof(pDownloadParam->strMethod));

        // create thread for progress control
        nProcessedCnt++;

        pDownloadParam->hwndTabDlg          = pDownLoadParam2->hwndTabDlg;
        pDownloadParam->nTotalCnt           = nTotalCnt;
        pDownloadParam->nProcessedCnt       = nProcessedCnt;
        pDownloadParam->nLstInx             = nInx;

        char strComponent[MAX_PATH];
        char strVer[MAX_VER_LEN];

        memset(strComponent, 0x00, sizeof(strComponent));
        memset(strVer,       0x00, sizeof(strVer));

        ListView_GetItemText(hwndDlgLst, nInx, 0, strComponent, MAX_PATH);
        ListView_GetItemText(hwndDlgLst, nInx, 2, strVer, MAX_VER_LEN);

        // Get download package info
        GetXmlDownloadStr(strComponent, strVer, pDownloadParam);
        string strLoad(pDownloadParam->strDLUrl);

        if (0 == strLoad.length())
        {
            SendMessage(pDownLoadParam2->hwndPCDlg, WM_OTM_UPDATE_ONE_START, (WPARAM) pDownloadParam, 0);
            sprintf(strMsg, ERROR_INSTALL_COMPONENT_STR, strComponent);
            MessageBox(pDownLoadParam2->hwndTabDlg, strMsg, AUTO_VER_UP_NAME_STR, MB_ICONSTOP | MB_OK);
            SendMessage(ghwndPrgCtrlDlg, WM_OTM_UPDATE_ONE_END, (WPARAM) pDownloadParam, 0);
            continue;
        }

        SendMessage(pDownLoadParam2->hwndPCDlg, WM_OTM_UPDATE_ONE_START, (WPARAM) pDownloadParam, 0);

        char strName[MAX_PATH];
        char strDLDir[MAX_PATH];

        memset(strName,  0x00, sizeof(strName));
        memset(strDLDir, 0x00, sizeof(strDLDir));

        // create downloads folder
        sprintf(strDLDir, "%s\\%s", gstrgOtmPath, DOWNLOAD_DIR);
        CreateDirectory(strDLDir, NULL);
        GetNameFromUrl(strName, pDownloadParam->strDLUrl);
        sprintf(pDownloadParam->strDLFile, "%s\\%s\\%s", gstrgOtmPath, DOWNLOAD_DIR, strName);

        DWORD nRC = NO_ERROR;

        // update one component
        NETWORKPARAM networkParam;
        InitNetworkParam(&networkParam);

        GetPrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_PROXY_ADDRESS, EMPTY_STR, networkParam.strProxyAddress, sizeof(networkParam.strProxyAddress), gstrgConfigFile);
        GetPrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_PROXY_PORT, EMPTY_STR, networkParam.strProxyPort, sizeof(networkParam.strProxyPort), gstrgConfigFile);
        networkParam.nTimeout = GetPrivateProfileInt(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_TIMEOUT, DEF_CONNECT_TIMEOUT, gstrgConfigFile);
   

        glogAutoVerUp.writef("Download to %s.", pDownloadParam->strDLFile);
        nRC = gOtmHttps->DownloadFile(pDownloadParam->strDLUrl, pDownloadParam->strDLFile, &networkParam);
        glogAutoVerUp.writef("Download %s end(%d), %s", pDownloadParam->strDLFile, nRC, networkParam.strError);

        if (nRC)
        {
            sprintf(strMsg, OtmGetMessageFromCode(nRC));
            MessageBox(pDownLoadParam2->hwndTabDlg, strMsg, AUTO_VER_UP_NAME_STR, MB_ICONSTOP | MB_OK);
            pDownloadParam->nRC = ERROR_END_ALL_TASK;
            SendMessage(ghwndPrgCtrlDlg, WM_OTM_UPDATE_ONE_END, (WPARAM) pDownloadParam, 0);
            return nRC;
        }


        int nKeepPkg = GetPrivateProfileInt(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_KEEP_PKG, AUTO_VER_UP_KEEP_PKG_DFT, gstrgConfigFile);
        if (!pDownloadParam->bRestart)
        {
            // check whether need unzip file
            if (!stricmp(DLTYPE_ZIP, pDownloadParam->strDLType))
            {
                // if zip mode, unzip the file first
                COTMCOPIES compCopyies;
                nRC = gOtmParser->GetCompCopiesByName(strComponent, strVer, &compCopyies);

                if (!nRC)
                {
                    OTMGRPSTING strUnzipFiles;
                    // unzip the package
                    // if successfully get copies info, unzip the package
                    COTMCOPIES defPathCopyies;
                    GetDefPathCopies(&defPathCopyies);
                    glogAutoVerUp.writef("Start unzip(%s).", pDownloadParam->strDLFile);
                    nRC = OtmUnCompress(pDownloadParam->strDLFile, &defPathCopyies, &compCopyies, gstrgOtmPath, &strUnzipFiles);
                    glogAutoVerUp.writef("End unzip(%d).", nRC);

                    if (!nRC && (!stricmp(METHOD_INSTALL, pDownloadParam->strMethod) || !stricmp(METHOD_OPEN, pDownloadParam->strMethod)))
                    {
                        // if success unzip
                        for (size_t iInx = 0; iInx < strUnzipFiles.size(); iInx++)
                        {
                            glogAutoVerUp.writef("Execute(%s).", strUnzipFiles[iInx].c_str());
                            nRC = CheckAndExecute(pDownloadParam->strMethod, pDownloadParam->bNeedWait, strUnzipFiles[iInx].c_str());
                            if (nRC)
                            {
                                nRC = ERROR_AUTO_VER_UP_A;
                                break;
                            }
                            else
                            {
                                // record to pending list
                                AddToNeedlessLst(strUnzipFiles[iInx].c_str());
                            }
                        } // end for
                    }
                    else if (!nRC)
                    {
                        // Add log info
                        for (size_t iInx = 0; iInx < strUnzipFiles.size(); iInx++)
                        {
                            glogAutoVerUp.writef("Execute(%s).", strUnzipFiles[iInx].c_str());
                        }
                    }

                    if (!nRC)
                    {
                        // if success and need to delete the package, delete the unzip file according to the setting
                        if (!nKeepPkg)
                        {
                            remove(pDownloadParam->strDLFile);
                        }
                    }
                    else
                    {
                        // if failed, report error
                        sprintf(strMsg, OtmGetMessageFromCode(nRC));
                        MessageBox(pDownLoadParam2->hwndTabDlg, strMsg, AUTO_VER_UP_NAME_STR, MB_ICONSTOP | MB_OK);
                    }
                }
                else
                {
                    glogAutoVerUp.writef("Get copy info failed.(%s).", strComponent);
                }
            }
            else
            {
                // if not unzip
                nRC = ERROR_DL_NOT_ZIP_A;
                sprintf(strMsg, OtmGetMessageFromCode(nRC));
                MessageBox(pDownLoadParam2->hwndTabDlg, strMsg, AUTO_VER_UP_NAME_STR, MB_ICONSTOP | MB_OK);
            }
        }
        else
        {
            // if need restart, modify pending config file
            glogAutoVerUp.writef("Need restart", pDownloadParam->strDLFile);
            AddToPendingLst(strComponent, strVer);
        }

        if (IsFixpack(strComponent) && !nRC && !pDownloadParam->bRestart)
        {
            nRC = SetFixpackState(strComponent, STATE_INSTALLED, FALSE);
        }

        if (!nRC)
        {
            nSuccessCnt++;
        }

        pDownloadParam->nRC         = nRC;
        pDownloadParam->nSuccessCnt = nSuccessCnt;
        SendMessage(ghwndPrgCtrlDlg, WM_OTM_UPDATE_ONE_END, (WPARAM) pDownloadParam, 0);
    }

    return nRC;
}

void SetInfoFromConfig(HWND hwndTabDlg)
{
    char strUrl[MAX_BUF_SIZE];
    memset(strUrl, 0x00, sizeof(strUrl));

    GetPrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_URL, EMPTY_STR, strUrl, sizeof(strUrl), gstrgConfigFile);
    SetDlgItemText(hwndTabDlg, IDC_EDT_URL, strUrl);

    char strProxyAddress[MAX_PROXY_ADDRESS_LEN];
    memset(strProxyAddress, 0x00, sizeof(strProxyAddress));

    GetPrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_PROXY_ADDRESS, EMPTY_STR, strProxyAddress, sizeof(strProxyAddress), gstrgConfigFile);
    SetDlgItemText(hwndTabDlg, IDC_EDT_PROXY_ADDRESS, strProxyAddress);

    char strProxyPort[MAX_PROXY_ADDRESS_LEN];
    memset(strProxyPort, 0x00, sizeof(strProxyPort));

    GetPrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_PROXY_PORT, EMPTY_STR, strProxyPort, sizeof(strProxyPort), gstrgConfigFile);
    SetDlgItemText(hwndTabDlg, IDC_EDT_PROXY_PORT, strProxyPort);

    int nTimeout = GetPrivateProfileInt(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_TIMEOUT, DEF_CONNECT_TIMEOUT, gstrgConfigFile);
    SetDlgItemInt(hwndTabDlg, IDC_EDT_TIMEOUT, nTimeout, FALSE);

    // set check frequency option
    int nFrequency = GetPrivateProfileInt(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_FREQUENCY, AUTO_VER_UP_FREQUENCY_DFT, gstrgConfigFile);
    int nCheckedID;
    switch(nFrequency)
    {
    case Day:
        nCheckedID = IDC_RADIO_DAY;
        break;
    case Week:
        nCheckedID = IDC_RADIO_WEEK;
        break;
    case Manual:
        nCheckedID = IDC_RADIO_MANUAL;
        break;
    default:
        nCheckedID = IDC_RADIO_DAY;
        break;
    }

    CheckRadioButton(hwndTabDlg, IDC_RADIO_DAY, IDC_RADIO_MANUAL, nCheckedID);

    // set title for check box
    char strTitle[MAX_BUF_SIZE];
    memset(strTitle, 0x00, sizeof(strTitle));
    sprintf(strTitle, TITLE_SET_KEEK_PKG_STR);
    SetDlgItemText(hwndTabDlg, IDC_CHK_KEEP_PKG, strTitle);

    // set check status for check box
    int nKeepPkg = GetPrivateProfileInt(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_KEEP_PKG, AUTO_VER_UP_KEEP_PKG_DFT, gstrgConfigFile);
    CheckDlgButton(hwndTabDlg, IDC_CHK_KEEP_PKG, nKeepPkg);

    // Enable button
//    RefreshButtonSet(hwndTabDlg);
}

void SaveInfoToConfig(HWND hwndTabDlg, BOOL bSilent)
{
    // set cursor to wait status
    HCURSOR hcursorHold, hcursorWait;
    hcursorWait = LoadCursor(NULL, IDC_WAIT);
    hcursorHold = ::SetCursor(hcursorWait);

    char strUrl[MAX_BUF_SIZE];
    memset(strUrl, 0x00, sizeof(strUrl));

    GetDlgItemText(hwndTabDlg, IDC_EDT_URL, strUrl, sizeof(strUrl));
    WritePrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_URL, strUrl, gstrgConfigFile);

    char strProxyAddress[MAX_PROXY_ADDRESS_LEN];
    memset(strProxyAddress, 0x00, sizeof(strProxyAddress));

    GetDlgItemText(hwndTabDlg, IDC_EDT_PROXY_ADDRESS, strProxyAddress, sizeof(strProxyAddress));
    WritePrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_PROXY_ADDRESS, strProxyAddress, gstrgConfigFile);

    char strProxyPort[MAX_PROXY_ADDRESS_LEN];
    memset(strProxyPort, 0x00, sizeof(strProxyPort));

    GetDlgItemText(hwndTabDlg, IDC_EDT_PROXY_PORT, strProxyPort, sizeof(strProxyPort));
    WritePrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_PROXY_PORT, strProxyPort, gstrgConfigFile);

    char strTimeout[MAX_BUF_SIZE];
    memset(strTimeout, 0x00, sizeof(strTimeout));

    GetDlgItemText(hwndTabDlg, IDC_EDT_TIMEOUT, strTimeout, sizeof(strTimeout));
    WritePrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_TIMEOUT, strTimeout, gstrgConfigFile);

    int nOldVal = GetPrivateProfileInt(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_FREQUENCY, AUTO_VER_UP_FREQUENCY_DFT, gstrgConfigFile);
    int nNewVal = 0;
    // when close the dialog, record the status of startup check box
    if (IsDlgButtonChecked(hwndTabDlg, IDC_RADIO_DAY))
    {
        nNewVal = 0;
        WritePrivateProfileString(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_FREQUENCY, "0", gstrgConfigFile);
    }
    else if (IsDlgButtonChecked(hwndTabDlg, IDC_RADIO_WEEK))
    {
        nNewVal = 1;
        WritePrivateProfileString(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_FREQUENCY, "1", gstrgConfigFile);
    }
    else if (IsDlgButtonChecked(hwndTabDlg, IDC_RADIO_MANUAL))
    {
        nNewVal = 2;
        WritePrivateProfileString(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_FREQUENCY, "2", gstrgConfigFile);
    }

    // set check status for check box
    if (IsDlgButtonChecked(hwndTabDlg, IDC_CHK_KEEP_PKG))
    {
        WritePrivateProfileString(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_KEEP_PKG, "1", gstrgConfigFile);
    }
    else
    {
        WritePrivateProfileString(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_KEEP_PKG, "0", gstrgConfigFile);
    }

    // Set next check time
    SetNextCheckTime();

    // Enable button
//    RefreshButtonSet(hwndTabDlg);

    // recover the cursor
    hcursorWait = LoadCursor(NULL, IDC_ARROW);
    hcursorHold = ::SetCursor(hcursorWait);

    if (bSilent)
    {
        return;
    }

    if ((nNewVal != nOldVal) && (nNewVal != 2))
    {
        int nID = MessageBox(hwndTabDlg, INFO_FREQUENCY_CHANGED_STR, AUTO_VER_UP_NAME_STR, MB_YESNO | MB_DEFBUTTON1);
        if (IDYES == nID)
        {
            CheckNowRefresh(hwndTabDlg);
        }
    }
    else
    {
        MessageBox(hwndTabDlg, INFO_SAVE_SUCCESS_STR, AUTO_VER_UP_NAME_STR, MB_OK);
    }
}

BOOL IsSettingChanged(HWND hwndTabDlg)
{
    BOOL bChanged = FALSE;

    // compare whether Url has changed
    char strNewVal[MAX_BUF_SIZE];
    char strOldVal[MAX_BUF_SIZE];
    memset(strNewVal, 0x00, sizeof(strNewVal));
    memset(strOldVal, 0x00, sizeof(strOldVal));

    GetDlgItemText(hwndTabDlg, IDC_EDT_URL, strNewVal, sizeof(strNewVal));
    GetPrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_URL, EMPTY_STR, strOldVal, sizeof(strOldVal), gstrgConfigFile);

    if (stricmp(strNewVal, strOldVal))
    {
        bChanged = TRUE;
        return bChanged;
    }

    // compare whether proxy address changed
    GetDlgItemText(hwndTabDlg, IDC_EDT_PROXY_ADDRESS, strNewVal, sizeof(strNewVal));
    GetPrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_PROXY_ADDRESS, EMPTY_STR, strOldVal, sizeof(strOldVal), gstrgConfigFile);

    if (stricmp(strNewVal, strOldVal))
    {
        bChanged = TRUE;
        return bChanged;
    }

    // compare whether proxy port changed
    GetDlgItemText(hwndTabDlg, IDC_EDT_PROXY_PORT, strNewVal, sizeof(strNewVal));
    GetPrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_PROXY_PORT, EMPTY_STR, strOldVal, sizeof(strOldVal), gstrgConfigFile);

    if (stricmp(strNewVal, strOldVal))
    {
        bChanged = TRUE;
        return bChanged;
    }

    // compare whether timeout changed
    GetDlgItemText(hwndTabDlg, IDC_EDT_TIMEOUT, strNewVal, sizeof(strNewVal));
    GetPrivateProfileString(APP_AUTO_VER_UP_NET_SET, KEY_AUTO_VER_UP_TIMEOUT, EMPTY_STR, strOldVal, sizeof(strOldVal), gstrgConfigFile);

    if (stricmp(strNewVal, strOldVal))
    {
        bChanged = TRUE;
        return bChanged;
    }

    int nNewVal, nOldVal;
    // when close the dialog, record the status of startup check box
    if (IsDlgButtonChecked(hwndTabDlg, IDC_RADIO_DAY))
    {
        nNewVal = 0;
    }
    else if (IsDlgButtonChecked(hwndTabDlg, IDC_RADIO_WEEK))
    {
        nNewVal = 1;
    }
    else if (IsDlgButtonChecked(hwndTabDlg, IDC_RADIO_MANUAL))
    {
        nNewVal = 2;
    }
    else
    {
        nNewVal = 0;
    }
    nOldVal = GetPrivateProfileInt(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_FREQUENCY, AUTO_VER_UP_FREQUENCY_DFT, gstrgConfigFile);

    if (nNewVal != nOldVal)
    {
        bChanged = TRUE;
        return bChanged;
    }

    // set check status for check box
    if (IsDlgButtonChecked(hwndTabDlg, IDC_CHK_KEEP_PKG))
    {
        nNewVal = 1;
    }
    else
    {
        nNewVal = 0;
    }
    nOldVal = GetPrivateProfileInt(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_KEEP_PKG, AUTO_VER_UP_KEEP_PKG_DFT, gstrgConfigFile);

    if (nNewVal != nOldVal)
    {
        bChanged = TRUE;
        return bChanged;
    }

    return bChanged;
}

void RefreshButtonSet(HWND hwndTabDlg)
{
    int nFrequency = GetPrivateProfileInt(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_FREQUENCY, AUTO_VER_UP_FREQUENCY_DFT, gstrgConfigFile);
    BOOL bAbled;
    switch(nFrequency)
    {
    case Day:
        // check by day
        bAbled = FALSE;
        break;
    case Week:
        // check by week
        bAbled = FALSE;
        break;
    case Manual:
        // check by manual
        bAbled = TRUE;
        break;
    default:
        bAbled = FALSE;
        break;
    }

    EnableWindow(GetDlgItem(hwndTabDlg, IDC_BTN_CHECK_NOW), bAbled);
}

void SetNextCheckTime()
{
    time_t timeNow;
    time(&timeNow);

    // get the local time
    struct tm * tmNow = localtime(&timeNow);

    int nDays = 0;

    int nFrequency = GetPrivateProfileInt(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_FREQUENCY, AUTO_VER_UP_FREQUENCY_DFT, gstrgConfigFile);
    switch(nFrequency)
    {
    case Day:
        // check by day
        nDays = 1;
        break;
    case Week:
        // check by week
        nDays = 7 - tmNow->tm_wday + 1; // regard Monday as the start date of the week
        break;
    case Manual:
        // check by manual
        glogAutoVerUp.writef("Manual, needn't set next check time.");
        return;
    default:
        glogAutoVerUp.writef("Default, needn't set next check time.");
        return;
    }

    // record to config file(
    char strNextDate[MAX_BUF_SIZE];
    memset(strNextDate, 0x00, sizeof(strNextDate));
    sprintf(strNextDate, DATE_FORMAT, (tmNow->tm_year + 1900), (tmNow->tm_mon + 1), (tmNow->tm_mday + nDays));
    WritePrivateProfileString(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_DATE, strNextDate, gstrgConfigFile);

    glogAutoVerUp.writef("Set the next check time to: %s.", strNextDate);
}

INT_PTR CALLBACK DownloadPCDlgProc
(
HWND hwndPrgCtrlDlg,
UINT msg,
WPARAM mp1,
LPARAM mp2
)
{
    LRESULT  mResult = MRFROMSHORT(FALSE);                // result value of procedure
    HWND hwndPB, hwndPBTotal;
    int nCnt;

    switch (msg)
    {
    case WM_INITDIALOG:
        OtmCenterWindow(hwndPrgCtrlDlg);
        SetDlgItemText(hwndPrgCtrlDlg, IDC_STATIC_CURRENT, EMPTY_STR);
        SetDlgItemText(hwndPrgCtrlDlg, IDC_STATIC_TOTAL, EMPTY_STR);

        // Initialize download progress bar
        hwndPB = GetDlgItem(hwndPrgCtrlDlg, IDC_PROGRESS_BAR);
        SendMessage(hwndPB, PBM_SETRANGE, (WPARAM)0, (LPARAM)(MAKELPARAM(0, 10000)));
        SendMessage(hwndPB, PBM_SETSTEP, 1, 0);
        SendMessage(hwndPB, PBM_STEPIT, 0, 0);
        SendMessage(hwndPB, PBM_SETPOS, (WPARAM)0, (LPARAM)0);

        // Initialize count progress bar
        hwndPBTotal = GetDlgItem(hwndPrgCtrlDlg, IDC_PROGRESS_BAR_TOTAL);
        nCnt = (int) mp2;
        SendMessage(hwndPBTotal, PBM_SETRANGE, (WPARAM)0, (LPARAM)(MAKELPARAM(0, nCnt)));
        SendMessage(hwndPBTotal, PBM_SETSTEP, 1, 0);
        SendMessage(hwndPBTotal, PBM_STEPIT, 0, 0);
        SendMessage(hwndPBTotal, PBM_SETPOS, (WPARAM)0, (LPARAM)0);

        gOtmHttps->SetProcessDlg(hwndPrgCtrlDlg, IDC_PROGRESS_BAR);
        break;

    case WM_COMMAND:
        break;

    case WM_OTM_UPDATE_ONE_START:
        {
            PDOWNLOADPARAM pDownloadParam = (PDOWNLOADPARAM) mp1;
            UpdateComponentStart(hwndPrgCtrlDlg, pDownloadParam);
        }
        break;

    case WM_OTM_UPDATE_ONE_END:
        {
            PDOWNLOADPARAM pDownloadParam = (PDOWNLOADPARAM) mp1;
            UpdateComponentEnd(hwndPrgCtrlDlg, pDownloadParam);
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hwndPrgCtrlDlg);
        break;

    default:
        break;
    } /* endswitch */

    return mResult;
}

void UpdateComponentStart(HWND hwndPCDlg, PDOWNLOADPARAM pDownloadParam)
{
    char strStatic[MAX_BUF_SIZE];
    char strName[MAX_PATH];

    memset(strStatic,   0x00, sizeof(strStatic));
    memset(strName,     0x00, sizeof(strName));

    GetNameFromUrl(strName, pDownloadParam->strDLUrl);
    sprintf(strStatic, DOWNLOAD_STR, strName);
    SetDlgItemText(hwndPCDlg, IDC_STATIC_CURRENT, strStatic);

    memset(strStatic,    0x00, sizeof(strStatic));
    sprintf(strStatic, PROGRESS_START_STR, pDownloadParam->nProcessedCnt, pDownloadParam->nTotalCnt);
    SetDlgItemText(hwndPCDlg, IDC_STATIC_TOTAL, strStatic);

    HWND hwndPB = GetDlgItem(ghwndPrgCtrlDlg, IDC_PROGRESS_BAR_TOTAL);
    SendMessage(hwndPB, PBM_SETPOS, (WPARAM)pDownloadParam->nProcessedCnt*2-1, (LPARAM)0);
}

void UpdateComponentEnd(HWND hwndPCDlg, PDOWNLOADPARAM pDownloadParam)
{
    HWND hwndDlgLst = GetDlgItem(pDownloadParam->hwndTabDlg, IDC_AUTO_VERUP_LIST);

    char strStatic[MAX_BUF_SIZE];
    memset(strStatic,    0x00, sizeof(strStatic));
    sprintf(strStatic, PROGRESS_END_STR, pDownloadParam->nProcessedCnt, pDownloadParam->nTotalCnt);
    SetDlgItemText(hwndPCDlg, IDC_STATIC_TOTAL, strStatic);

    HWND hwndPB = GetDlgItem(hwndPCDlg, IDC_PROGRESS_BAR_TOTAL);
    SendMessage(hwndPB, PBM_SETPOS, (WPARAM)pDownloadParam->nProcessedCnt*2, (LPARAM)0);

    // if error occurred and not necessary to continue, just quit
    if (ERROR_END_ALL_TASK == pDownloadParam->nRC)
    {
        SendMessage(hwndPCDlg, WM_CLOSE, 0, 0);
        EnableWindow(ghwndDlg, TRUE);
        MessageBox(hwndDlgLst, ERROR_UPDATE_FAILED_STR, AUTO_VER_UP_NAME_STR, MB_ICONSTOP | MB_OK);
        return;
    }

    if (pDownloadParam->nProcessedCnt < pDownloadParam->nTotalCnt)
    {
        return;
    }

    SendMessage(hwndPCDlg, WM_CLOSE, 0, 0);
    EnableWindow(ghwndDlg, TRUE);

    char strOpenTM2StarterPath[MAX_PATH];
    memset(strOpenTM2StarterPath, 0x00, sizeof(strOpenTM2StarterPath));
    sprintf(strOpenTM2StarterPath, "%s\\WIN\\%s", gstrgOtmPath, OTM_APPL_STARTER_EXE);

    if (pDownloadParam->bRestart)
    {
        int nRet = MessageBox(hwndDlgLst, INFO_NEED_RESTART_STR, AUTO_VER_UP_NAME_STR, MB_YESNO);
        if (IDYES == nRet)
        {
            // to start OpenTM2Starter, needn't wait
            OtmExecuteCommand(strOpenTM2StarterPath, NULL, FALSE);
        }
        PostMessage(ghwndDlg, WM_CLOSE, FALSE, 0L);
    }
    else
    {
        // update component list
        SendMessage(ghwndPages[0], WM_OTM_SET_UPDATE, 0, 0);
        if ((pDownloadParam->nSuccessCnt == pDownloadParam->nTotalCnt))
        {
            if (pDownloadParam->bNeedWait)
            {
                int nRet = MessageBox(hwndDlgLst, INFO_UPDATE_SUCCESS_STR, AUTO_VER_UP_NAME_STR, MB_YESNO);
                if (IDYES == nRet)
                {
                    // to start OpenTM2Starter, needn't wait
                    OtmExecuteCommand(strOpenTM2StarterPath, NULL, FALSE);
                    PostMessage(ghwndDlg, WM_CLOSE, FALSE, 0L);
                }
            }
            else
            {
                PostMessage(ghwndDlg, WM_CLOSE, FALSE, 0L);
            }
        }
        else
        {
            MessageBox(hwndDlgLst, ERROR_UPDATE_FAILED_STR, AUTO_VER_UP_NAME_STR, MB_ICONSTOP | MB_OK);
        }
    }
}

void GetXmlDownloadStr(const char * strComponent, const char * strVer, PDOWNLOADPARAM pDownloadParam)
{
    if (NULL == strComponent)
    {
        return;
    }

    int nCompInx, nCompJnx, nFixpacksCnt;
    int nCompTotal = gOtmParser->GetComponentsCnt();
    for (nCompInx = 0; nCompInx < nCompTotal; nCompInx++)
    {
        BOOL bFound = FALSE;
        if (IsFixpack(strComponent))
        {
            char strId[MAX_LEN];
            char strCompName[MAX_LEN];
            char strCompVer[MAX_VER_LEN];

            memset(strId,        0x00, sizeof(strId));
            memset(strCompName,  0x00, sizeof(strCompName));
            memset(strCompVer,   0x00, sizeof(strCompVer));

            SplitFixpackName(strComponent, strCompName, strCompVer, strId);
            glogAutoVerUp.writef("Split (%s) = %s + %s + %s", strComponent, strCompName, strCompVer, strId);

            // check the fixpack
            nFixpacksCnt = gOtmParser->GetCompFixpacksCnt(nCompInx);
            for (nCompJnx = 0; nCompJnx < nFixpacksCnt; nCompJnx++)
            {
                char * strXmlFixpackId = gOtmParser->GetCompFixpackId(nCompInx, nCompJnx);
                if (!stricmp(strCompName,     gOtmParser->GetComponentName(nCompInx)) &&
                    !stricmp(strCompVer,      gOtmParser->GetComponentVersion(nCompInx)) &&
                    !stricmp(strXmlFixpackId, strId))
                {
                    strcpy(pDownloadParam->strDLUrl,   gOtmParser->GetCompFixpackDLUrl(nCompInx, nCompJnx));
                    strcpy(pDownloadParam->strDLType,  gOtmParser->GetCompFixpackDLType(nCompInx, nCompJnx));
                    strcpy(pDownloadParam->strMethod,  gOtmParser->GetCompFixpackMethod(nCompInx, nCompJnx));
                    pDownloadParam->bRestart  = gOtmParser->GetCompFixpackRestart(nCompInx, nCompJnx);
                    pDownloadParam->bNeedWait = gOtmParser->GetCompFixpackNeedWait(nCompInx, nCompJnx);
                    bFound = TRUE;
                    break;
                }
            }

            if (bFound)
            {
                break;
            }
        }
        else
        {
            char * strXmlCompName = gOtmParser->GetComponentName(nCompInx);
            char * strXmlCompVer  = gOtmParser->GetComponentVersion(nCompInx);

            if (!stricmp(strXmlCompName, strComponent) &&
                !stricmp(strXmlCompVer,  strVer))
            {
                strcpy(pDownloadParam->strDLUrl,   gOtmParser->GetComponentDLUrl(nCompInx));
                strcpy(pDownloadParam->strDLType,  gOtmParser->GetComponentDLType(nCompInx));
                strcpy(pDownloadParam->strMethod,  gOtmParser->GetComponentMethod(nCompInx));
                pDownloadParam->bRestart = gOtmParser->GetComponentRestart(nCompInx);
                pDownloadParam->bNeedWait = gOtmParser->GetComponentNeedWait(nCompInx);
                break;
            }
        }
    }
}

// get list total checked count
int GetLstTotalCnt(HWND hwndDlgLst)
{
    int nTotalCnt = 0;
    for (int nInx = 0; nInx < ListView_GetItemCount(hwndDlgLst); nInx++)
    {
        if (!ListView_GetCheckState(hwndDlgLst, nInx))
        {
            continue;
        }
        nTotalCnt++;
    }
    return nTotalCnt;
}

int CheckAndExecute(const char * strMethod, BOOL bNeedWait, const char * strUnzipFiles)
{
    DWORD nRC = NO_ERROR;

    BOOL bExecute = FALSE;

    if (!stricmp(METHOD_OPEN, strMethod) || !stricmp(METHOD_INSTALL, strMethod))
    {
        bExecute = TRUE;
    }

    if (bExecute)
    {
        // start our replicator exe
        nRC = OtmExecuteCommand((char *)strUnzipFiles, NULL, bNeedWait);
    }
    return nRC;
}

int CheckNewVersion(const char * strOtmVersion)
{
    int nRC = NO_ERROR;

    // new check or not
    time_t timeCheck;
    timeCheck = (time_t) GetPrivateProfileInt(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_DATE, 0, gstrgConfigFile);

    time_t timeNow;
    time(&timeNow);

    int nNow   = GetDateFromTime((int)timeNow);
    int nCheck = GetDateFromTime((int)timeCheck);

    glogAutoVerUp.writef("Compare time now=%d with check=%d", nNow, nCheck);

    char strXml[MAX_PATH];
    memset(strXml, 0x00, sizeof(strXml));
    sprintf(strXml, "%s\\PLUGINS\\%s", gstrgOtmPath, AUTO_VER_UP_XML);

    int nFrequency = GetPrivateProfileInt(APP_AUTO_VER_UP_SET, KEY_AUTO_VER_UP_FREQUENCY, AUTO_VER_UP_FREQUENCY_DFT, gstrgConfigFile);
    if ((nFrequency == Manual) || (nNow < nCheck))
    {
        glogAutoVerUp.writef("Needn't start.");
        nRC = gOtmParser->XmlParser(gstrgConfigFile, strXml, FALSE);
        if (NO_ERROR == nRC)
        {
            nRC = VAL_NO_NEED_OPEN;
        }

        if (!gpAtvIda->bFromMenu)
        {
            nRC = VAL_NO_NEED_OPEN;
            return nRC;
        }
    }
    else
    {
        SetNextCheckTime();
        glogAutoVerUp.writef("Download the xml and then do the parse.");
        nRC = gOtmParser->XmlParser(gstrgConfigFile, strXml, TRUE);
        if (NO_ERROR != nRC)
        {
            return nRC;
        }
    }

    nRC = HasNewVersion(strOtmVersion);

    return nRC;
}

int HasNewVersion(const char * strOtmVersion)
{
    int nRC = VAL_NOT_HAS_NEW_VER;
    for (int nInx = 0; nInx < (int) gOtmParser->GetComponentsCnt(); nInx++)
    {
        char * strName = gOtmParser->GetComponentName(nInx);
        // check wether the component is OpenTM2
        if (!strnicmp(OPENTM2_APP_NAME_STR, strName, strlen(OPENTM2_APP_NAME_STR)))
        {
            char * strCompVersion = gOtmParser->GetComponentVersion(nInx);
            int nRet = IsNewVersion(strOtmVersion, strCompVersion);
            if (nRet > VER_SAM_VAL)
            {
                glogAutoVerUp.writef("New version %s is larger than old %s", strCompVersion, strOtmVersion);
                nRC = VAL_HAS_NEW_VER;
                break;
            }
            else if (nRet == VER_SAM_VAL)
            {
                if (HasFixpackUninstalled(nInx))
                {
                    glogAutoVerUp.writef("Version %s has fixpack", strOtmVersion);
                    nRC = VAL_HAS_NEW_VER;
                    break;
                }
            }
        }
    }

    return nRC;
}

BOOL HasFixpackUninstalled(int nInx)
{
    BOOL bUninstall = FALSE;
    for (int jInx = 0; jInx < gOtmParser->GetCompFixpacksCnt(nInx); jInx++)
    {
        char strName[MAX_BUF_SIZE];
        memset(strName, 0x00, sizeof(strName));
        sprintf(strName, "%s_%s_%s_%s", KEY_FIXPACK, gOtmParser->GetComponentName(nInx), 
                gOtmParser->GetComponentVersion(nInx), gOtmParser->GetCompFixpackId(nInx, jInx));

        if (!IsFixpackInstalled(strName))
        {
            bUninstall = TRUE;
            break;
        }
    }

    return bUninstall;
}

int ProcessParameter(char * strParameter, PAUTOVERUPPROPIDA pAtvIda)
{
    // initialize
    memset(pAtvIda->strOtmRevision, 0x00, sizeof(pAtvIda->strOtmRevision));
    memset(pAtvIda->strEncryptFile, 0x00, sizeof(pAtvIda->strEncryptFile));
    pAtvIda->bFromMenu = FALSE;
    pAtvIda->nEncrypt  = 0;

    int nLen = (int) strlen(strParameter);

    char * strSegment = new char [MAX_BUF_SIZE];
    memset(strSegment, 0x00, sizeof(strSegment));

    int jInx = 0;
    BOOL bQuoStart = FALSE;                       // Quotation start mark
    for (int iInx = 0; iInx <= nLen; iInx++)
    {
        if ('\"' == strParameter[iInx])
        {
            bQuoStart = !bQuoStart;
        }

        if (((' ' != strParameter[iInx]) || bQuoStart) && (iInx != nLen))
        {
            strSegment[jInx] = strParameter[iInx];
            jInx++;
            continue;
        }
        strSegment[jInx] = '\0';

        char * strKey = new char [MAX_BUF_SIZE];
        char * strVal = new char [MAX_BUF_SIZE];
        memset(strKey, 0x00, sizeof(strKey));
        memset(strVal, 0x00, sizeof(strVal));

        OtmGetKeyValue(strSegment, strKey, strVal, ':', FALSE);
        memset(strSegment, 0x00, sizeof(strSegment));
        jInx = 0;

        if ((NULL == strKey) || (strlen(strKey) == 0))
        {
            continue;
        }

        if (!memicmp(strKey, PARAM_CMD_VER_KEY, strlen(strKey))) {
            // remove double quotation mark
            RemoveDoubleQuotationMark(strVal);
            if (!strnicmp(KEY_VERSION, strVal, strlen(KEY_VERSION)))
            {
                int nPos = sizeof(KEY_VERSION);
                char * strVersion = strVal + nPos;
                strcpy(pAtvIda->strOtmRevision, strVersion);
            }
            else
            {
                strcpy(pAtvIda->strOtmRevision, strVal);
            }
        } else if(!memicmp(strKey, PARAM_CMD_FROM_MENU_KEY, strlen(strKey))) {
            RemoveDoubleQuotationMark(strVal);
            pAtvIda->bFromMenu = atoi(strVal);
        } else if(!memicmp(strKey, "?", strlen(strKey))) {
            //            PrintHelp();
            return ERROR_WRONG_PARAM_A;
        } else {
//            printf("Specified parameter %s not correct!\n", strPara);
//            PrintHelp();
            return ERROR_WRONG_PARAM_A;
        }
    }

    return (NO_ERROR);
}

int CheckConfig()
{
    int nRC = NO_ERROR;

    // judge whether the conf file exists or not
    if (OTM_NOT_FOUND == access(gstrgConfigFile, 0))
    {
        // if not exist, find sample file and rename to the config file
        char strConfigSampleFile[MAX_PATH];
        memset(strConfigSampleFile, 0x00, sizeof(strConfigSampleFile));
        sprintf(strConfigSampleFile, "%s\\%s\\%s", gstrgOtmPath, KEY_PLUGINS_PATH, AUTO_VERSION_UP_CONFIG_SAMPLE);
        if (OTM_NOT_FOUND == access(strConfigSampleFile, 0))
        {
            // sample file not found, exist
            nRC = ERROR_FILE_NOT_FOUND;
            return nRC;
        }
        rename(strConfigSampleFile, gstrgConfigFile);
    }


    return nRC;
}

void CheckNowRefreshUI(HWND hwndTabDlg)
{
    if (IsSettingChanged(hwndTabDlg))
    {
        int nID = MessageBox(hwndTabDlg, INFO_SETTING_CHANGED_STR, AUTO_VER_UP_NAME_STR, MB_YESNO | MB_DEFBUTTON1);
        if (IDNO == nID)
        {
            return;
        }
        else
        {
            SaveInfoToConfig(hwndTabDlg, TRUE);
        }
    }

    CheckNowRefresh(hwndTabDlg);
}

int CheckNowRefresh(HWND hwndTabDlg)
{
    int nRC;
    // set cursor to wait status
    HCURSOR hcursorHold, hcursorWait;
    hcursorWait = LoadCursor(NULL, IDC_WAIT);
    hcursorHold = ::SetCursor(hcursorWait);

    glogAutoVerUp.writef("Start to reget xml file.");

    char strXml[MAX_PATH];
    memset(strXml, 0x00, sizeof(strXml));
    sprintf(strXml, "%s\\PLUGINS\\%s", gstrgOtmPath, AUTO_VER_UP_XML);

    nRC = gOtmParser->XmlParser(gstrgConfigFile, strXml, TRUE);
    if (nRC)
    {
        MessageBox(hwndTabDlg, OtmGetMessageFromCode(nRC), AUTO_VER_UP_NAME_STR, MB_ICONSTOP | MB_OK);
        glogAutoVerUp.writef("Refresh parse XML failed:%d", nRC);
        return nRC;
    }

    glogAutoVerUp.writef("Refresh parse successfully:%d", nRC);

    // update component list
    SendMessage(ghwndPages[0], WM_OTM_SET_UPDATE, 0, 0);

    nRC = HasNewVersion(gpAtvIda->strOtmRevision);
    if (VAL_HAS_NEW_VER == nRC)
    {
        MessageBox(hwndTabDlg, INFO_NEW_VERSION_FOUND_STR, AUTO_VER_UP_NAME_STR, MB_OK);
    }
    else
    {
        MessageBox(hwndTabDlg, INFO_NO_NEW_FOUND_STR, AUTO_VER_UP_NAME_STR, MB_OK);
    }

    // recover the cursor
    hcursorWait = LoadCursor(NULL,IDC_ARROW);
    hcursorHold = ::SetCursor(hcursorWait);

    return nRC;
}

void NeedlessFileCheck()
{
    char strNeedlessLst[MAX_PATH];

    memset(strNeedlessLst, 0x00, sizeof(strNeedlessLst));
    sprintf(strNeedlessLst, "%s\\%s\\%s", gstrgOtmPath, DOWNLOAD_DIR, AUTO_VER_UP_NEEDLESS_LST);

    FILE * fileLst = fopen(strNeedlessLst, "rb");
    if (NULL == fileLst)
    {
        return;
    }

    char strSetense[MAX_BUF_SIZE];
    memset(strSetense, 0x00, sizeof(strSetense));

    while (fgets(strSetense, MAX_BUF_SIZE, fileLst))
    {
        int nLen = strlen(strSetense);
        if (strSetense[nLen-1] == '\n')
        {
            if (strSetense[nLen-2] == '\r')
            {
                strSetense[nLen-2] = EOS;
            }
            else
            {
                strSetense[nLen-1] = EOS;
            }
        }

        remove(strSetense);
    }

    fclose(fileLst);
    remove(strNeedlessLst);
}

void AddToNeedlessLst(const char * strFileName)
{
    char strPendingLst[MAX_PATH];

    memset(strPendingLst, 0x00, sizeof(strPendingLst));
    sprintf(strPendingLst, "%s\\%s\\%s", gstrgOtmPath, DOWNLOAD_DIR, AUTO_VER_UP_NEEDLESS_LST);

    FILE * fileLst = fopen(strPendingLst, "ab+");
    if (NULL == fileLst)
    {
        return;
    }

    fprintf(fileLst, "%s\n", strFileName);

    fclose(fileLst);
}

void AddToPendingLst(const char * strComp, const char * strVer)
{
    char strPendUptConf[MAX_PATH];
    char strPendingLst[MAX_PATH];

    memset(strPendUptConf, 0x00, sizeof(strPendUptConf));
    memset(strPendingLst,  0x00, sizeof(strPendingLst));

    sprintf(strPendUptConf, "%s\\%s\\%s", gstrgOtmPath, KEY_PLUGINS_PATH, PENDING_UPT_CONF);
    sprintf(strPendingLst,  "%s\\%s\\%s", gstrgOtmPath, KEY_PLUGINS_PATH, AUTO_VER_UP_PENDING_LST);

    glogAutoVerUp.writef("Writing to pending list %s start", strPendingLst);

    // Record need reatart info to the config file
    WritePrivateProfileString(APP_SETTINGS_STR, KEY_NEED_RESTART_A, "1", strPendUptConf);

    // Record plugin name into update list
    FILE * filePendUptLst = fopen(strPendingLst, "a+");
    if (!IsFixpack(strComp))
    {
        fprintf(filePendUptLst, "%s_%s\n", strComp, strVer);
    }
    else
    {
        fprintf(filePendUptLst, "%s\n", strComp);
    }

    fclose(filePendUptLst);

    glogAutoVerUp.writef("Writing to pending list %s end", strPendingLst);
}

void TestConnection(HWND hwndTabDlg)
{
    if (IsSettingChanged(hwndTabDlg))
    {
        int nID = MessageBox(hwndTabDlg, INFO_SETTING_CHANGED_STR, AUTO_VER_UP_NAME_STR, MB_YESNO | MB_DEFBUTTON1);
        if (IDNO == nID)
        {
            return;
        }
        else
        {
            SaveInfoToConfig(hwndTabDlg, TRUE);
        }
    }

    // set cursor to wait status
    HCURSOR hcursorHold, hcursorWait;
    hcursorWait = LoadCursor(NULL, IDC_WAIT);
    hcursorHold = ::SetCursor(hcursorWait);

    int nRC = gOtmParser->TestConnection(gstrgConfigFile);
    if (!nRC)
    {
        MessageBox(hwndTabDlg, INFO_CONNECT_SUCCESS_STR, AUTO_VER_UP_NAME_STR, MB_OK);
    }
    else
    {
        MessageBox(hwndTabDlg, OtmGetMessageFromCode(nRC), AUTO_VER_UP_NAME_STR, MB_ICONSTOP | MB_OK);
    }

    // recover the cursor
    hcursorWait = LoadCursor(NULL, IDC_ARROW);
    hcursorHold = ::SetCursor(hcursorWait);
}

void ResizeWholeDlg(HWND hwndDlg, SHORT sNewWidth, SHORT sNewHeight)
{
    HWND  hwndCtrl;
    RECT  rectCtrl;
    POINT ptCtrl;

    hwndCtrl = GetDlgItem(hwndDlg, IDC_AUTO_VERUP_TAB);
    GetWindowRect(hwndCtrl, &rectCtrl);

    ptCtrl.x = rectCtrl.left;
    ptCtrl.y = rectCtrl.top;
    ScreenToClient(hwndDlg, &ptCtrl);

    int nNewWeight = rectCtrl.right - rectCtrl.left;
    int nNewHeight   = rectCtrl.bottom - rectCtrl.top;

    glogAutoVerUp.writef("Move: %d, %d, %d, %d", rectCtrl.left, rectCtrl.top, nNewWeight, nNewHeight);
    MoveWindow(hwndCtrl, 0, 0, nNewWeight, nNewHeight, TRUE);
}

void CloseOpenTM2()
{
    HWND hwndOpenTM2 = FindWindow(OPENTM2_APP_NAME_STR, NULL);
    SetForegroundWindow(hwndOpenTM2);
    SendMessage(hwndOpenTM2, WM_CLOSE, 0, 0);
}

void GetDefPathCopies(PCOTMCOPIES pDefPathCopies)
{
    COTMCOPY defPathCopy;
    InitOtmCopy(&defPathCopy);

    char strPluginPath[MAX_PATH];
    memset(strPluginPath, 0x00, sizeof(strPluginPath));
    sprintf(strPluginPath, "%s\\%s", gstrgOtmPath, KEY_PLUGINS_PATH);

    strcpy(defPathCopy.strFrom,      PLUGIN_DEF_PATH_KEY);
    strcpy(defPathCopy.strTo,        strPluginPath);

    pDefPathCopies->push_back(defPathCopy);

    strcpy(defPathCopy.strFrom,      OPENTM2_DEF_PATH_KEY);
    strcpy(defPathCopy.strTo,        gstrgOtmPath);

    pDefPathCopies->push_back(defPathCopy);

    char strDLPath[MAX_PATH];
    memset(strDLPath, 0x00, sizeof(strDLPath));
    sprintf(strDLPath, "%s\\%s", gstrgOtmPath, PENDING_DIR_STR);

    InitOtmCopy(&defPathCopy);
    strcpy(defPathCopy.strFrom,      OPENTM2_DEF_DL_PATH_KEY);
    strcpy(defPathCopy.strTo,        strDLPath);

    pDefPathCopies->push_back(defPathCopy);
}
