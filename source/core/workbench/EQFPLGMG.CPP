//+----------------------------------------------------------------------------+
//|EQFPLGMG.CPP     OTM Startup Plugin Manager function                        |
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
//|                    during plugin manager                                   |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "eqfplgmg.h"             // type definitions and function prototypes
#include "core\utilities\LogWriter.h"
#include <TlHelp32.h>
#include <Shellapi.h>
#include "eqfserno.h"

// prototype for non-public utility functions
VOID UtlSetString(SHORT sID, PSZ pszString);
static char gstrPluginPath[MAX_PATH];
static char gstrAppPath[MAX_PATH];

VOID AutoVerUpProps(BOOL bFromMenu)
{
    // setting for log
    gbLogOpened = FALSE;
#ifdef _DEBUG
    if (!glogPlgMgr.isOpen())
    {
       glogPlgMgr.open(LOG_PLUGIN_MGR_GUI_NAME);
       gbLogOpened = TRUE;
    }
#endif

    // start our replicator program if it is not active yet
    if (!IsProgramRunning(AUTO_VERUP_EXE))
    {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        char szAutoVerup[MAX_LONGPATH];
        char szProgramPath[MAX_LONGPATH];

        // in the current version we use the program path!
        UtlMakeEQFPath(szProgramPath, NULC, PROGRAM_PATH, NULL);

        // build replicator exe path
        strcpy(szAutoVerup, szProgramPath);
        strcat(szAutoVerup, "\\");
        strcat(szAutoVerup, AUTO_VERUP_EXE);

        // start our replicator exe
        char szRevision[OTM_VER_LEN];
        char szCmdParam[MAX_BUF_SIZE];
        memset(szRevision, 0x00, sizeof(szRevision));
        memset(szCmdParam, 0x00, sizeof(szCmdParam));
        LOADSTRING(NULLHANDLE, hResMod, SID_LOGO_REVISION, szRevision);
        sprintf(szCmdParam, "/v:\"%s\" /fromMenu:%d", szRevision, bFromMenu);

        ShellExecute(NULL, "open", szAutoVerup, szCmdParam, szProgramPath, SW_SHOWMINNOACTIVE);
    }
    else
    {
        MessageBox(EqfQueryTwbClient(), WARNING_AVT_OPENED_STR, PLUGIN_MGR_APP_NAME_STR, MB_OK);
    }
}

VOID PluginManagerMenuProps()
{
    // wait for check thread
    int nAuto = GetPrivateProfileInt(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_AUTO, PLUGIN_MGR_AUTO_DFT, gstrConfigPath);
    if (nAuto)
    {
        // set cursor to wait status
        HCURSOR hcursorHold, hcursorWait;
        hcursorWait = LoadCursor(NULL, IDC_WAIT);
        hcursorHold = ::SetCursor(hcursorWait);

        glogPlgMgr.writef("Auto thread is opening, menu will not do any action.");

        MessageBox(EqfQueryTwbClient(), WARNING_AUTO_CHECK_RUNNING_STR, PLUGIN_MGR_APP_NAME_STR, MB_OK);

        // recover the cursor
        hcursorWait = LoadCursor(NULL,IDC_ARROW);
        hcursorHold = ::SetCursor(hcursorWait);

        glogPlgMgr.close();
        return;
    }

    int nRC = PluginManagerGUIStart();
    if (nRC)
    {
        glogPlgMgr.close();
        return;
    }

    // start dialog box
    INT_PTR sRC;
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    DIALOGBOX(EqfQueryTwbClient(), PluginManagerDlgProc, hResMod, ID_PLUGIN_MANAGER_DLG, NULL, sRC);
}

void PluginManagerAutoProps()
{
    int nRC = PluginManagerGUIStart();
    if (nRC)
    {
        glogPlgMgr.close();
        return;
    }

    ghChkVerThread = (HANDLE) _beginthreadex(NULL, 0, CheckNewVersionProc, EqfQueryTwbClient(), 0, 0);
    if (NULL == ghChkVerThread)
    {
        glogPlgMgr.close();
        return;
    }
}

int PluginManagerGUIStart()
{
    int nRC = NO_ERROR;

    gbLogOpened = FALSE;
#ifdef _DEBUG
    if (!glogPlgMgr.isOpen())
    {
        glogPlgMgr.open(LOG_PLUGIN_MGR_GUI_NAME);
        gbLogOpened = TRUE;
    }
#endif

    // Initial path of plugin
    HWND hwndMain = EqfQueryTwbClient();

    memset(gstrPluginPath,   0x00, sizeof(gstrPluginPath));
    memset(gstrAppPath,      0x00, sizeof(gstrAppPath));
    memset(gstrConfigPath,   0x00, sizeof(gstrConfigPath));

    UtlQueryString(QST_PLUGINPATH, gstrPluginPath, sizeof(gstrPluginPath));
    UtlMakeEQFPath(gstrPluginPath, NULC, PLUGIN_PATH, NULL);
    if (NULL == gstrPluginPath)
    {
        MessageBox(hwndMain, OtmGetMessageFromCode(ERROR_GET_PLUGIN_PATH_A), PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
        return ERROR_GET_PLUGIN_PATH_A;
    }

    char strModule[MAX_PATH];
    memset(strModule, 0x00, sizeof(strModule));

    GetModuleFileName(NULL, strModule, sizeof(strModule));
    GetModuleAppPath(strModule, gstrAppPath);

    sprintf(gstrConfigPath, "%s\\%s", gstrPluginPath, PLUGIN_MGR_CONFIG);

    // check or set the config file
    nRC = CheckConfig(gstrPluginPath);
    if (nRC)
    {
        MessageBox(hwndMain, OtmGetMessageFromCode(nRC), PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
        return nRC;
    }

    if (NULL == gplginParse)
    {
        gplginParse = new CPlgMgXmlParser();
    }

    if (NULL == gHistoryWriter)
    {
        gHistoryWriter = new HistoryWriter();
    }

    return nRC;
}

int CheckConfig(char * strPluginPath)
{
    int nRC = NO_ERROR;

    char strConfigPath[MAX_PATH];
    memset(strConfigPath, 0x00, sizeof(strConfigPath));
    sprintf(strConfigPath, "%s\\%s", strPluginPath, PLUGIN_MGR_CONFIG);

    // judge whether the xlm exists or not
    if (OTM_NOT_FOUND == access(strConfigPath, 0))
    {
        // if not, find sample file and rename to the config file
        char strConfigSamplePath[MAX_PATH];
        memset(strConfigSamplePath, 0x00, sizeof(strConfigSamplePath));
        sprintf(strConfigSamplePath, "%s\\%s", strPluginPath, PLUGIN_MGR_CONFIG_SAMPLE);
        if (OTM_NOT_FOUND == access(strConfigSamplePath, 0))
        {
            // if config exist, just return
            nRC = ERROR_FILE_NOT_FOUND;
            glogPlgMgr.writef("Error:%d", nRC);
            return nRC;
        }
        rename(strConfigSamplePath, strConfigPath);
    }


    return nRC;
}

INT_PTR CALLBACK PluginManagerDlgProc
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
    MRESULT  mResult = MRFROMSHORT(FALSE);                // result value of procedure
    BOOL     fOK;                                           // internal O.K. flag
    PPLUGINMAGPROPIDA pIda;

    switch (msg)
    {
    case WM_INITDLG:
        ghChkVerThread = NULL;
        OtmCenterWindow(hwndDlg);
        // allocate and anchor our dialog IDA
        fOK = UtlAlloc((PVOID *) &pIda, 0L, (ULONG)sizeof(PPLUGINMAGPROPIDA), ERROR_STORAGE);
        if (fOK)
        {
            fOK = ANCHORDLGIDA(hwndDlg, pIda);
            if (!fOK)                           //no access to ida
            {
                UtlErrorHwnd(0, MB_CANCEL, 0, NULL, SYSTEM_ERROR, hwndDlg);
            }
        }

        if (fOK)
        {
            ghwndDlg = hwndDlg;
            PluginManagerDlgSheetLoad(hwndDlg, pIda);
        } /* endif */
        mResult = DIALOGINITRETURN(mResult);
        break;


    case WM_COMMAND:
        mResult = PluginManagerPropCommand(hwndDlg, mp1, mp2);
        break;

    case WM_NOTIFY:
        mResult = PlugMgPropPropertySheetNotification(hwndDlg, mp1, mp2);
        break;

    case WM_HELP:
        /*************************************************************/
        /* pass on a HELP_WM_HELP request                            */
        /*************************************************************/
        /*EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
        &hlpsubtblTwbSysPropDlg[0]);*/
        mResult = TRUE;  // message processed
        break;

    case WM_EQF_CLOSE:
        //--- get rid off dialog ---
        pIda = ACCESSDLGIDA(hwndDlg, PPLUGINMAGPROPIDA);
        if (pIda)
        {
            USHORT nItem = 0;
            /***********************************************************/
            /* free all allocated pages as well as the registration    */
            /* of the modeless dialog                                  */
            /***********************************************************/
            while (pIda->hwndPages[nItem])
            {
                UtlUnregisterModelessDlg(pIda->hwndPages[nItem]);
                DestroyWindow(pIda->hwndPages[nItem]);
                nItem++;
            } /* endwhile */
        } /* endif */
        DISMISSDLG(hwndDlg, mp1);
        glogPlgMgr.close();
        break;

    default:
        mResult = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;
    } /* endswitch */

    return mResult;
} /* end of PluginManagerDlgProp */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     PluginManagerPropCommand                                 |
//+----------------------------------------------------------------------------+
//|Function call:     PluginManagerPropCommand(hwndDlg, mp1, mp2);             |
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
MRESULT PluginManagerPropCommand
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
    MRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
    PPLUGINMAGPROPIDA pIda;                        // ptr to dialog IDA
    BOOL fOK = TRUE;
    char strMsg[100];

    memset(strMsg, 0x00, sizeof(strMsg));
    switch (WMCOMMANDID(mp1, mp2))
    {
    case PID_PB_HELP:
        mResult = UtlInvokeHelp();
        break;
    case PID_PB_OK:
        {
            int nItem = 0;

            pIda = ACCESSDLGIDA(hwndDlg, PPLUGINMAGPROPIDA);
            // issue command to all active dialog pages
            while (pIda->hwndPages[nItem] && fOK)
            {
                PFNWP pfnWp = (PFNWP) GetWindowLong(pIda->hwndPages[ nItem ],
                    DWL_DLGPROC);

                fOK =  !pfnWp(pIda->hwndPages[nItem], WM_COMMAND, PID_PB_OK, 0L);
                nItem++;
            } /* endwhile */
        }

        if (fOK) POSTEQFCLOSE(hwndDlg, TRUE);
        break;

    case IDCANCEL:
        if (gbLogOpened)
        {
            glogPlgMgr.close();
        }
        POSTEQFCLOSE(hwndDlg, FALSE);
        break;

    default:
        mResult = WinDefDlgProc(hwndDlg, WM_COMMAND, mp1, mp2);
        break;
    } /* endswitch */

    return(mResult);
} /* end of PluginManagerPropCommand */

MRESULT PlugMgPropPropertySheetNotification
(
HWND hwndDlg,
WPARAM  mp1,
LPARAM  mp2
)
{
    NMHDR * pNMHdr;
    ULONG       ulTabCtrl;
    MRESULT      mResult = FALSE;
    PPLUGINMAGPROPIDA     pIda;
    pNMHdr = (LPNMHDR)mp2;

    switch (pNMHdr->code)
    {
    case TCN_SELCHANGE:
        pIda = ACCESSDLGIDA(hwndDlg, PPLUGINMAGPROPIDA);
        if (pIda)
        {
            TC_ITEM Item;
            HWND hwndTabCtrl = GetDlgItem(hwndDlg, IDC_PLUGM_TAB);
            ulTabCtrl = TabCtrl_GetCurSel(hwndTabCtrl);
            memset(&Item, 0, sizeof(Item));
            Item.mask = TCIF_PARAM;
            TabCtrl_GetItem(hwndTabCtrl, ulTabCtrl, &Item);
            ulTabCtrl = Item.lParam;
            ShowWindow(pIda->hwndPages[ulTabCtrl], SW_SHOW);
        } /* endif */
        break;
    case TCN_SELCHANGING:
        pIda = ACCESSDLGIDA(hwndDlg, PPLUGINMAGPROPIDA);
        if (pIda)
        {
            /**************************************************************/
            /* Issue a direct call to the appropriate dialog proc with    */
            /* WM_COMMAND, ID_TB_PROP_SET_PB and the second parameter set */
            /* to 1L to force only consistency checking                   */
            /**************************************************************/
            TC_ITEM Item;
            PFNWP pfnWp;
            HWND hwndTabCtrl = GetDlgItem(hwndDlg, IDC_PLUGM_TAB);
            ulTabCtrl = TabCtrl_GetCurSel(hwndTabCtrl);
            memset(&Item, 0, sizeof(Item));
            Item.mask = TCIF_PARAM;
            TabCtrl_GetItem(hwndTabCtrl, ulTabCtrl, &Item);
            ulTabCtrl = Item.lParam;
            pfnWp = (PFNWP) GetWindowLong(pIda->hwndPages[ ulTabCtrl ], DWL_DLGPROC);

            mResult = pfnWp(pIda->hwndPages[ulTabCtrl], WM_COMMAND, PID_PB_OK, 1L);
            if (mResult)
            {
                /************************************************************/
                /* stick on the side                                        */
                /* we have to post the request again since one of the system*/
                /* routines thinks that we still want to change the page..  */
                /************************************************************/
                WinPostMsg(hwndDlg, TCM_SETCURSEL, ulTabCtrl, 0L);
            } /* endif */
            ShowWindow(pIda->hwndPages[ulTabCtrl], SW_HIDE);
        } /* endif */
        break;
    default:
        break;
    } /* endswitch */
    return mResult;
} /* end of function PluginMgPropPropertySheetNotification */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     PluginManagerDlgSheetLoad                                |
//+----------------------------------------------------------------------------+
//|Function call:     TwbSysPropertySheetLoad(hwndDlg, mp2);                    |
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
BOOL PluginManagerDlgSheetLoad
(
HWND hwndDlg,
PPLUGINMAGPROPIDA  pIda
)
{
    BOOL      fOK = TRUE;
    TC_ITEM   TabCtrlItem;
    USHORT    nItem = 0;
    HWND      hwndTabCtrl;
    HINSTANCE hInst;

    RECT otmRect;
    // remember adress of user area
    hInst = GETINSTANCE(hwndDlg);
    hwndTabCtrl = GetDlgItem(hwndDlg, IDC_PLUGM_TAB);
    pIda->hwndTabCtrl = hwndTabCtrl;
    GetClientRect(hwndTabCtrl, &otmRect);
    TabCtrl_AdjustRect(hwndTabCtrl, FALSE, &otmRect);

    // leave some additional space at top
    otmRect.top += 20;
    MapWindowPoints(hwndTabCtrl, hwndDlg, (POINT *) &otmRect, 2);

    TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM;

    // create the appropriate TAB control and load the associated dialog
    // Installed sheet
    TabCtrlItem.pszText = TEXT(PM_TAB_NAME_1_STR);
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] = CreateDialogParam(hInst,
                                               MAKEINTRESOURCE(ID_PLUGM_INSTALLED_DLG),
                                               hwndDlg,
                                               PlgMgrInstalledDlgProc,
                                               (LPARAM)pIda);

    SetWindowPos(pIda->hwndPages[nItem], HWND_TOP,
                 otmRect.left, otmRect.top,
                 otmRect.right-otmRect.left, otmRect.bottom-otmRect.top, SWP_SHOWWINDOW);
    UtlRegisterModelessDlg(pIda->hwndPages[nItem]);
    nItem++;

    // Available sheet
    TabCtrlItem.pszText = TEXT(PM_TAB_NAME_2_STR);
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] = CreateDialogParam(hInst,
                                               MAKEINTRESOURCE(ID_PLUGM_AVAILABLE_DLG),
                                               hwndDlg,
                                               PlgMgrAvailableDlgProc,
                                               (LPARAM)pIda);

    SetWindowPos(pIda->hwndPages[nItem], HWND_TOP,
                 otmRect.left, otmRect.top,
                 otmRect.right-otmRect.left, otmRect.bottom-otmRect.top, SWP_SHOWWINDOW);
    UtlRegisterModelessDlg(pIda->hwndPages[nItem]);
    nItem++;

    // Updates sheet
    TabCtrlItem.pszText = TEXT(PM_TAB_NAME_3_STR);
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] = CreateDialogParam(hInst,
                                               MAKEINTRESOURCE(ID_PLUGM_UPDATES_DLG),
                                               hwndDlg,
                                               PlgMgrUpdatesDlgProc,
                                               (LPARAM)pIda);

    SetWindowPos(pIda->hwndPages[nItem], HWND_TOP,
                 otmRect.left, otmRect.top,
                 otmRect.right-otmRect.left, otmRect.bottom-otmRect.top, SWP_SHOWWINDOW);
    UtlRegisterModelessDlg(pIda->hwndPages[nItem]);
    nItem++;

    // Settings sheet
    TabCtrlItem.pszText = TEXT(PM_TAB_NAME_4_STR);
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] = CreateDialogParam(hInst,
                                                MAKEINTRESOURCE(ID_PLUGM_SETTINGS_DLG),
                                                hwndDlg,
                                                PlgMgrSettingDlgProc,
                                                (LPARAM)pIda);

    SetWindowPos(pIda->hwndPages[nItem], HWND_TOP,
                 otmRect.left, otmRect.top,
                 otmRect.right-otmRect.left, otmRect.bottom-otmRect.top, SWP_SHOWWINDOW);
    UtlRegisterModelessDlg(pIda->hwndPages[nItem]);
    nItem++;

    // Help sheet
    TabCtrlItem.pszText = TEXT(PM_TAB_NAME_5_STR);
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] = CreateDialogParam(hInst,
                                                MAKEINTRESOURCE(ID_PLUGM_HELP_DLG),
                                                hwndDlg,
                                                PlgMgrHelpDlgProc,
                                                (LPARAM)pIda);

    SetWindowPos(pIda->hwndPages[nItem], HWND_TOP,
                 otmRect.left, otmRect.top,
                 otmRect.right-otmRect.left, otmRect.bottom-otmRect.top, SWP_SHOWWINDOW);
    UtlRegisterModelessDlg(pIda->hwndPages[nItem]);
    nItem++;

    // hide all dialog pages but the first one
    if (fOK)
    {
        int iInx = 1;
        while (pIda->hwndPages[iInx])
        {
            ShowWindow(pIda->hwndPages[iInx], SW_HIDE);
            iInx++;
        } // endwhile
    } // endif

    if (!fOK)
    {
        POSTEQFCLOSE(hwndDlg, FALSE);
    } // endif

    return fOK;
}

INT_PTR CALLBACK PlgMgrInstalledDlgProc
(
HWND hwndInsTabDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
    MRESULT  mResult = MRFROMSHORT(FALSE);                // result value of procedure
    PPLUGINMAGPROPIDA  pIda;
    HWND               plgMgList;
    LV_COLUMN          lvCol;
    char strMsg[MAX_BUF_SIZE];

    memset(strMsg, 0x00, sizeof(strMsg));
    plgMgList = GetDlgItem(hwndInsTabDlg, IDC_PLUGINMGR_LIST);
//    ListView_SetExtendedListViewStyle(plgMgList, LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES);
    ListView_SetExtendedListViewStyle(plgMgList, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    switch (msg)
    {
    case WM_INITDLG:
        pIda = (PPLUGINMAGPROPIDA)PVOIDFROMMP2(mp2);
        ANCHORDLGIDA(hwndInsTabDlg, pIda);

        //Initial list view control
        InitialOtmListViewCtrl(plgMgList);

        SetDlgItemText(hwndInsTabDlg, IDC_STC_LONG_DESC, EMPTY_STRING);
        // Initial column
        lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvCol.fmt = LVCFMT_LEFT;

        // column 1: Plugin
        lvCol.iSubItem = 0;
        lvCol.cx = 200;
        lvCol.pszText = INSTALL_LST_COLUMN_1_STR;
        ListView_InsertColumn(plgMgList, 0, &lvCol);

        // column 2: Type
        lvCol.iSubItem = 1;
        lvCol.cx = 150;
        lvCol.pszText = INSTALL_LST_COLUMN_2_STR;
        ListView_InsertColumn(plgMgList, 1, &lvCol);

        // column 3: Installed Version
        lvCol.iSubItem = 2;
        lvCol.cx = 100;
        lvCol.pszText = INSTALL_LST_COLUMN_3_STR;
        ListView_InsertColumn(plgMgList, 2, &lvCol);

        // column 4: Has Update
        lvCol.iSubItem = 3;
        lvCol.cx = 100;
        lvCol.pszText = INSTALL_LST_COLUMN_4_STR;
        ListView_InsertColumn(plgMgList, 3, &lvCol);

        // column 5: short description
        lvCol.iSubItem = 4;
        lvCol.cx = 300;
        lvCol.pszText = INSTALL_LST_COLUMN_5_STR;
        ListView_InsertColumn(plgMgList, 4, &lvCol);

        RefreshInstalledList(plgMgList);
        break;

    case WM_OTM_UPDATE:
        RefreshInstalledList(plgMgList);
        break;

    case WM_OTM_SET_UPDATE:
        RefreshInstalledList(plgMgList);
        break;

    case WM_NOTIFY:
        PluginListNotify(hwndInsTabDlg, mp1, mp2, TRUE);
        break;

    case WM_COMMAND:
        switch (WMCOMMANDID(mp1, mp2))
        {
        case PID_PB_OK:
            {
                /****************************************************************/
                /* if mp2 == 1L we have to validate the page, if it is 0L we    */
                /* have to copy the content of the dialog back into the struct. */
                /****************************************************************/
                break;
            }
        case IDCANCEL:
            {
                POSTEQFCLOSE(hwndInsTabDlg, FALSE);
                break;
            }
        case IDC_REMOVE_BTN:
            RemoveSelectedPlugin(hwndInsTabDlg);
            break;
        default:
            break;
        }
    case WM_HELP:
        /*************************************************************/
        /* pass on a HELP_WM_HELP request                            */
        /*************************************************************/
        /*EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
        &hlpsubtblTwbSysPropGeneral[0]);*/
        mResult = TRUE;  // message processed
        break;

    case WM_CLOSE:
        POSTEQFCLOSE(hwndInsTabDlg, FALSE);
        break;

    default:
//        mResult = WinDefDlgProc(hwndInstallDlg, msg, mp1, mp2);
        break;
    } /* endswitch */

    return mResult;
}

INT_PTR CALLBACK PlgMgrAvailableDlgProc
(
HWND hwndAvbTabDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
    MRESULT  mResult = MRFROMSHORT(FALSE);                // result value of procedure
    PPLUGINMAGPROPIDA  pIda;
    HWND               plgMgList;
    LV_COLUMN          lvCol;
    LV_ITEM            lvItem;
    char strMsg[100];

    memset(strMsg, 0x00, sizeof(strMsg));
    plgMgList = GetDlgItem(hwndAvbTabDlg, IDC_PLUGINMGR_LIST);
    ListView_SetExtendedListViewStyle(plgMgList, LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    switch (msg)
    {
    case WM_INITDLG:
        pIda = (PPLUGINMAGPROPIDA)PVOIDFROMMP2(mp2);
        ANCHORDLGIDA(hwndAvbTabDlg, pIda);

        SetDlgItemText(hwndAvbTabDlg, IDC_STC_LONG_DESC, EMPTY_STR);
        // Initial column
        lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvCol.fmt = LVCFMT_LEFT;

        // column 1: plugin name
        lvCol.iSubItem = 0;
        lvCol.cx = 180;
        lvCol.pszText = AVAILBLE_LST_COLUMN_1_STR;
        ListView_InsertColumn(plgMgList, 0, &lvCol);

        // column 2: plugin type
        lvCol.iSubItem = 1;
        lvCol.cx = 80;
        lvCol.pszText = AVAILBLE_LST_COLUMN_2_STR;
        ListView_InsertColumn(plgMgList, 1, &lvCol);

        // column 3: available version
        lvCol.iSubItem = 2;
        lvCol.cx = 100;
        lvCol.pszText = AVAILBLE_LST_COLUMN_3_STR;
        ListView_InsertColumn(plgMgList, 2, &lvCol);

        // column 4: available date
        lvCol.iSubItem = 3;
        lvCol.cx = 100;
        lvCol.pszText = AVAILBLE_LST_COLUMN_4_STR;
        ListView_InsertColumn(plgMgList, 3, &lvCol);

        // column 5: short description
        lvCol.iSubItem = 4;
        lvCol.cx = 200;
        lvCol.pszText = AVAILBLE_LST_COLUMN_5_STR;
        ListView_InsertColumn(plgMgList, 4, &lvCol);

        // column 6: severity
        lvCol.iSubItem = 5;
        lvCol.cx = 80;
        lvCol.pszText = AVAILBLE_LST_COLUMN_6_STR;
        ListView_InsertColumn(plgMgList, 5, &lvCol);

        // column 7: impact on opentm2 assets
        lvCol.iSubItem = 6;
        lvCol.cx = 200;
        lvCol.pszText = AVAILBLE_LST_COLUMN_7_STR;
        ListView_InsertColumn(plgMgList, 6, &lvCol);

        // column 8: action to be taken after installation
        lvCol.iSubItem = 7;
        lvCol.cx = 200;
        lvCol.pszText = AVAILBLE_LST_COLUMN_8_STR;
        ListView_InsertColumn(plgMgList, 7, &lvCol);

        lvItem.mask = LVIF_TEXT;
        SplitPlugins(plgMgList, TYPE_NEW_INSTALL);
        break;

    case WM_OTM_UPDATE:
    case WM_OTM_SET_UPDATE:
        SplitPlugins(plgMgList, TYPE_NEW_INSTALL);
        break;

    case WM_NOTIFY:
        PluginListNotify(hwndAvbTabDlg, mp1, mp2, FALSE);
        break;

    case WM_COMMAND:
        switch (WMCOMMANDID(mp1, mp2))
        {
        case PID_PB_OK:
            {
                break;
            }
        case IDCANCEL:
            {
                POSTEQFCLOSE(hwndAvbTabDlg, FALSE);
                break;
            }
        case IDC_INSTALL_BTN:
            {
                InstallSelectedPlugin(hwndAvbTabDlg);
                break;
            }
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
        POSTEQFCLOSE(hwndAvbTabDlg, FALSE);
        break;

    default:
//        mResult = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;
    } /* endswitch */

    return mResult;
}

INT_PTR CALLBACK PlgMgrUpdatesDlgProc
(
HWND hwndUptTabDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
    MRESULT  mResult = MRFROMSHORT(FALSE);                // result value of procedure
    PPLUGINMAGPROPIDA  pIda;
    HWND               plgMgList;
    LV_COLUMN          lvCol;
    plgMgList = GetDlgItem(hwndUptTabDlg, IDC_PLUGINMGR_LIST);
    ListView_SetExtendedListViewStyle(plgMgList, LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    switch (msg)
    {
    case WM_INITDLG:
        pIda = (PPLUGINMAGPROPIDA)PVOIDFROMMP2(mp2);
        ANCHORDLGIDA(hwndUptTabDlg, pIda);

        SetDlgItemText(hwndUptTabDlg, IDC_STC_LONG_DESC, EMPTY_STRING);
        // Initial column
        lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvCol.fmt = LVCFMT_LEFT;

        // column 1: plugin name
        lvCol.iSubItem = 0;
        lvCol.cx = 180;
        lvCol.pszText = UPDATE_LST_COLUMN_1_STR;
        ListView_InsertColumn(plgMgList, 0, &lvCol);

        // column 2: plugin type
        lvCol.iSubItem = 1;
        lvCol.cx = 80;
        lvCol.pszText = UPDATE_LST_COLUMN_2_STR;
        ListView_InsertColumn(plgMgList, 1, &lvCol);

        // column 3: insatlled version
        lvCol.iSubItem = 2;
        lvCol.cx = 100;
        lvCol.pszText = UPDATE_LST_COLUMN_3_STR;
        ListView_InsertColumn(plgMgList, 2, &lvCol);

        // column 4: available version
        lvCol.iSubItem = 3;
        lvCol.cx = 100;
        lvCol.pszText = UPDATE_LST_COLUMN_4_STR;
        ListView_InsertColumn(plgMgList, 3, &lvCol);

        // column 5: available date
        lvCol.iSubItem = 4;
        lvCol.cx = 100;
        lvCol.pszText = UPDATE_LST_COLUMN_5_STR;
        ListView_InsertColumn(plgMgList, 4, &lvCol);

        // column 6: short description
        lvCol.iSubItem = 5;
        lvCol.cx = 200;
        lvCol.pszText = UPDATE_LST_COLUMN_6_STR;
        ListView_InsertColumn(plgMgList, 5, &lvCol);

        // column 7: severity
        lvCol.iSubItem = 6;
        lvCol.cx = 80;
        lvCol.pszText = UPDATE_LST_COLUMN_7_STR;
        ListView_InsertColumn(plgMgList, 6, &lvCol);

        // column 8: Impact on OpenTM2 Assets
        lvCol.iSubItem = 7;
        lvCol.cx = 200;
        lvCol.pszText = UPDATE_LST_COLUMN_8_STR;
        ListView_InsertColumn(plgMgList, 7, &lvCol);

        // column 9: Action to be Taken After Installation
        lvCol.iSubItem = 8;
        lvCol.cx = 200;
        lvCol.pszText = UPDATE_LST_COLUMN_9_STR;
        ListView_InsertColumn(plgMgList, 8, &lvCol);

        SplitPlugins(plgMgList, TYPE_UPDATE);
        break;

    case WM_OTM_UPDATE:
    case WM_OTM_SET_UPDATE:
        SplitPlugins(plgMgList, TYPE_UPDATE);
        break;

    case WM_NOTIFY:
        PluginListNotify(hwndUptTabDlg, mp1, mp2, FALSE);
        break;

    case WM_COMMAND:
        switch (WMCOMMANDID(mp1, mp2))
        {
        case PID_PB_OK:
            /****************************************************************/
            /* if mp2 == 1L we have to validate the page, if it is 0L we    */
            /* have to copy the content of the dialog back into the struct. */
            /****************************************************************/
            break;
        case IDCANCEL:
            {
                POSTEQFCLOSE(hwndUptTabDlg, FALSE);
                break;
            }
        case IDC_UPDATE_BTN:
            {
                UpdateSelectedPlugin(hwndUptTabDlg);
                break;
            }
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
        POSTEQFCLOSE(hwndUptTabDlg, FALSE);
        break;

    default:
//        mResult = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;
    } /* endswitch */

    return mResult;
}

INT_PTR CALLBACK PlgMgrSettingDlgProc
(
HWND hwndTabDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
    MRESULT  mResult = MRFROMSHORT(FALSE);                // result value of procedure
    PPLUGINMAGPROPIDA  pIda;

    switch (msg)
    {
    case WM_INITDLG:
        pIda = (PPLUGINMAGPROPIDA)PVOIDFROMMP2(mp2);
        ANCHORDLGIDA(hwndTabDlg, pIda);
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
            CheckNowRefresh(hwndTabDlg);
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
        POSTEQFCLOSE(hwndTabDlg, FALSE);
        break;

    default:
//        mResult = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;
    } /* endswitch */

    return mResult;
}

INT_PTR CALLBACK PlgMgrHelpDlgProc
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
    MRESULT  mResult = MRFROMSHORT(FALSE);                // result value of procedure
    PPLUGINMAGPROPIDA  pIda;

    switch (msg)
    {
    case WM_INITDLG:
        pIda = (PPLUGINMAGPROPIDA)PVOIDFROMMP2(mp2);
        ANCHORDLGIDA(hwndDlg, pIda);
        break;

    case WM_COMMAND:
        switch (WMCOMMANDID(mp1, mp2))
        {
        case PID_PB_OK:
            break;
        case IDCANCEL:
            {
                POSTEQFCLOSE(hwndDlg, FALSE);
                break;
            }
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
        POSTEQFCLOSE(hwndDlg, FALSE);
        break;

    default:
        mResult = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;
    } /* endswitch */

    return mResult;
}

LRESULT PluginListNotify(HWND hwndDlg, WPARAM mp1, LPARAM mp2, int nType)
{
    NMHDR * pNMHdr;
    LRESULT mResult = FALSE;

    pNMHdr = (LPNMHDR)mp2;
    switch (pNMHdr->code)
    {
    case NM_CLICK:
        {
            SetListViewCtrlChkState(hwndDlg, nType);
            ShowLongDscp(hwndDlg, nType);
        } 
        break;
    default:
        break;
    }
    return mResult;
} /* end of function AutoVerUpLstNotify */

int OtmParseXml(BOOL bDownload)
{
    int nRC = NO_ERROR;

    char strXml[MAX_PATH];
    memset(strXml,           0x00, sizeof(strXml));

    sprintf(strXml, "%s\\%s", gstrPluginPath, PLUGIN_MGR_XML);
    glogPlgMgr.writef("Start parse:%s", strXml);
    nRC = gplginParse->XmlParser(gstrConfigPath, strXml, bDownload);
    glogPlgMgr.writef("End parse:%s", strXml);

    return nRC;
}

void RefreshInstalledList(HWND plgMgList)
{
    glogPlgMgr.writef("Refresh installed list");
    ListView_DeleteAllItems(plgMgList);
    PluginManager*     thePluginManager;
    OtmPlugin*         otmPlugin;
    int nInx, nCnt;

    thePluginManager = PluginManager::getInstance();
    nCnt = thePluginManager->getPluginCount();

    int nLstInx = 0;
    // first add removable plugin
    for (nInx = 0; nInx < nCnt; nInx++)
    {
        otmPlugin = thePluginManager->getPlugin(nInx);
        if (!IsRemovablePlugin(otmPlugin->getName()))
        {
            continue;
        }

        AddItemToInstalledList(plgMgList, nLstInx, nInx, STATE_CB_NO);

        nLstInx++;
    }

    //  add non-removable plugin
    for (nInx = 0; nInx < nCnt; nInx++)
    {
        otmPlugin = thePluginManager->getPlugin(nInx);
        if (IsRemovablePlugin(otmPlugin->getName()))
        {
            continue;
        }

        AddItemToInstalledList(plgMgList, nLstInx, nInx, STATE_CB_GREY);

        nLstInx++;
    }
}

void AddItemToInstalledList(HWND plgMgList, int nLstInx, int nPluginInx, int nImage)
{
    PluginManager*     thePluginManager;
    OtmPlugin*         otmPlugin;
    OtmPlugin::ePluginType typePlugin;

    thePluginManager = PluginManager::getInstance();

    otmPlugin = thePluginManager->getPlugin(nPluginInx);

    LV_ITEM            lvItem;
    lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvItem.iItem = nLstInx;

    // column 1: Plugin name
    lvItem.iSubItem = 0;
    lvItem.iImage = nImage;
    lvItem.pszText = (LPSTR) otmPlugin->getName();
    ListView_InsertItem(plgMgList, &lvItem);

    // column 2: type
    lvItem.iSubItem = 1;
    typePlugin = otmPlugin->getType();
    switch(typePlugin)
    {
    case OtmPlugin::eTranslationMemoryType:
        lvItem.pszText = PLUGIN_TYPE_MEMORY_STR;
        break;
    case OtmPlugin::eDictionaryType:
        lvItem.pszText = PLUGIN_TYPE_DICT_STR;
        break;
    case OtmPlugin::eMarkupType:
        lvItem.pszText = PLUGIN_TYPE_MARKUP_STR;
        break;
    case OtmPlugin::eSharedTranslationMemoryType:
        lvItem.pszText = PLUGIN_TYPE_SHARED_MEMORY_STR;
        break;
    case OtmPlugin::eDocumentType:
        lvItem.pszText = PLUGIN_TYPE_DOCUMENT_STR;
        break;
    case OtmPlugin::eSpellType:
        lvItem.pszText = PLUGIN_TYPE_SPELL_STR;
        break;
    case OtmPlugin::eMorphType:
        lvItem.pszText = PLUGIN_TYPE_MORGH_STR;
        break;
    case OtmPlugin::eToolType:
        lvItem.pszText = PLUGIN_TYPE_TOOL_STR;
        break;
    default:
        lvItem.pszText = PLUGIN_TYPE_UNDEFINED_STR;
        break;
    }
    ListView_SetItem(plgMgList, &lvItem);

    // column 3: installed version
    lvItem.iSubItem = 2;
    lvItem.pszText = (LPSTR) otmPlugin->getVersion();
    ListView_SetItem(plgMgList, &lvItem);

    // column 4: has update
    lvItem.iSubItem = 3;
    if (HasUpdate(otmPlugin->getName(), otmPlugin->getVersion()))
    {
        lvItem.pszText = YES_STR;
    }
    else
    {
        lvItem.pszText = EMPTY_STR;
    }
    ListView_SetItem(plgMgList, &lvItem);

    // column 5: short description
    lvItem.iSubItem = 4;
    lvItem.pszText = (LPSTR) otmPlugin->getShortDescription();
    ListView_SetItem(plgMgList, &lvItem);
}

void SplitPlugins(HWND hwndPlgMgrLst, int nType)
{
    glogPlgMgr.writef("Split plugins");
    LV_ITEM            lvItem;
    lvItem.mask = LVIF_TEXT;
    int nAvailableInx, nUpdatesInx;

    ListView_DeleteAllItems(hwndPlgMgrLst);
    nAvailableInx = nUpdatesInx = 0;

    for (int iInx = 0; iInx < gplginParse->GetPluginsCnt(); iInx++)
    {
        char * strPluginName = gplginParse->GetPluginName(iInx);
        if (NULL == strPluginName)
        {
            continue;
        }

        glogPlgMgr.writef("Checking %s", strPluginName);
        if ((isNewPlugin(strPluginName) == IS_NEW_PLUGIN) && (TYPE_NEW_INSTALL == nType))
        {
            for (int jInx = 0; jInx < gplginParse->GetPluginMainCnt(iInx); jInx++)
            {
                glogPlgMgr.writef("Plugin %s is new.", strPluginName);
                // new plugin
                lvItem.iItem = nAvailableInx;

                // column 1: plugin
                lvItem.iSubItem = 0;
                lvItem.pszText = strPluginName;
                ListView_InsertItem(hwndPlgMgrLst, &lvItem);

                // column 2: Type
                lvItem.iSubItem = 1;
                lvItem.pszText = gplginParse->GetPluginType(iInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 3: available version
                lvItem.iSubItem = 2;
                lvItem.pszText = gplginParse->GetMainVersion(iInx, jInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 4: available date
                lvItem.iSubItem = 3;
                lvItem.pszText = gplginParse->GetMainDate(iInx, jInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 5: short description
                lvItem.iSubItem = 4;
                lvItem.pszText = gplginParse->GetPluginDscp(iInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 6: severity
                lvItem.iSubItem = 5;
                lvItem.pszText = gplginParse->GetPluginSeverity(iInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 7: impact on opentm2 assets
                lvItem.iSubItem = 6;
                lvItem.pszText = gplginParse->GetPluginImpact(iInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 8: action to be taken after installation
                lvItem.iSubItem = 7;
                lvItem.pszText = gplginParse->GetPluginAfterAction(iInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);
            }
        }
        else if ((isNewPlugin(strPluginName) == NOT_NEW_PLUGIN) && (TYPE_UPDATE == nType))
        {
            for (int jInx = 0; jInx < gplginParse->GetPluginMainCnt(iInx); jInx++)
            {
                // update plugin
                char * strVerNew = gplginParse->GetMainVersion(iInx, jInx);
                char * strVerOld = GetLocalPluginVer(strPluginName);

                if (IsNewVersion(strVerOld, strVerNew) < VER_SAM_VAL)
                {
                    glogPlgMgr.writef("%s, New %s < Old %s", strPluginName, strVerNew, strVerOld);
                    continue;
                }
                else if (IsNewVersion(strVerOld, strVerNew) == VER_SAM_VAL)
                {
                    // check whether the version is same as local, if it is, add fixpack
                    glogPlgMgr.writef("%s, New %s = Old %s", strPluginName, strVerNew, strVerOld);
                    glogPlgMgr.writef("Only the same version need add fixpack info");
                    nUpdatesInx += AddFixpacksToList(hwndPlgMgrLst, iInx, jInx, nUpdatesInx, FALSE);
                    continue;
                }

                glogPlgMgr.writef("Plugin %s is update.", strPluginName);
                lvItem.iItem = nUpdatesInx;

                // column 1: plugin
                lvItem.iSubItem = 0;
                lvItem.pszText = strPluginName;
                ListView_InsertItem(hwndPlgMgrLst, &lvItem);

                // column 2 type
                lvItem.iSubItem = 1;
                lvItem.pszText = gplginParse->GetPluginType(iInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 3: installed version
                lvItem.iSubItem = 2;
                lvItem.pszText = strVerOld;
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 4: available version
                lvItem.iSubItem = 3;
                lvItem.pszText = strVerNew;
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 5: available date
                lvItem.iSubItem = 4;
                lvItem.pszText = gplginParse->GetMainDate(iInx, jInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 6: short description
                lvItem.iSubItem = 5;
                lvItem.pszText = gplginParse->GetPluginDscp(iInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 7: severity
                lvItem.iSubItem = 6;
                lvItem.pszText = gplginParse->GetPluginSeverity(iInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 8: impact on opentm2 assets
                lvItem.iSubItem = 7;
                lvItem.pszText = gplginParse->GetPluginImpact(iInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                // column 9: action to be taken after installation
                lvItem.iSubItem = 8;
                lvItem.pszText = gplginParse->GetPluginAfterAction(iInx);
                ListView_SetItem(hwndPlgMgrLst, &lvItem);

                glogPlgMgr.writef("%s, New %s > Old %s", strPluginName, strVerNew, strVerOld);
                nUpdatesInx++;
//              nUpdatesInx += AddFixpacksToList(hwndPlgMgrLst, iInx, jInx, nUpdatesInx, FALSE);
            }
        }
    }
}

int isNewPlugin(char * strPluginName)
{
    int nRC = IS_NEW_PLUGIN;
    PluginManager*     thePluginManager;
    OtmPlugin*         otmPlugin;
    int nCnt, nInx;

    thePluginManager = PluginManager::getInstance();
    nCnt = thePluginManager->getPluginCount();
    for (nInx = 0; nInx < nCnt; nInx++)
    {
        otmPlugin = thePluginManager->getPlugin(nInx);
        if (!stricmp(strPluginName, otmPlugin->getName()))
        {
            nRC = NOT_NEW_PLUGIN;
            break;
        }
    }

    if (nRC == IS_NEW_PLUGIN)
    {
        char strDllName[MAX_PATH];
        memset(strDllName, 0x00, sizeof(strDllName));
        sprintf(strDllName, "%s.dll", strPluginName);

        if (IfPluginExists(gstrPluginPath, strDllName, strPluginName))
        {
            glogPlgMgr.writef("Plugin %s is existed, but not loaded.", strPluginName);
            nRC = PLUGIN_NOT_LOADED;
        }
    }

    return nRC;
}

int AddFixpacksToList(HWND hwndPlgMgrLst, int iInx, int jInx, int nLstStartInx, BOOL bNewVer)
{
    int nAddCnt = 0;
    int nLstInx = nLstStartInx;
    int nCnt = gplginParse->GetMainFixpacksCnt(iInx, jInx);

    LV_ITEM            lvItem;
    lvItem.mask = LVIF_TEXT;

    for (int kInx= 0; kInx < nCnt; kInx++)
    {
        // set compment name for fixpack
        char strName[MAX_BUF_SIZE];
        memset(strName, 0x00, sizeof(strName));
        sprintf(strName, "%s_%s_%s_%s", KEY_FIXPACK, gplginParse->GetPluginName(iInx), 
                gplginParse->GetMainVersion(iInx, jInx), gplginParse->GetMainFixpackId(iInx, jInx, kInx));

        // if it is not new version, then it should confirm whether the fixpack installed or not
        if (!bNewVer && IsFixpackInstalled(strName))
        {
            glogPlgMgr.writef("For %s not new version but installed, skip.", strName);
            continue;
        }

        // column 1: name
        lvItem.iItem = nLstInx;
        lvItem.iSubItem = 0;
        lvItem.pszText = strName;
        ListView_InsertItem(hwndPlgMgrLst, &lvItem);

        if (bNewVer)
        {
            // new
            // column 2: type
            lvItem.iSubItem = 1;
            lvItem.pszText = gplginParse->GetPluginType(iInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 3: available version
            lvItem.iSubItem = 2;
            lvItem.pszText = gplginParse->GetMainVersion(iInx, jInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 4: available date
            lvItem.iSubItem = 3;
            lvItem.pszText = gplginParse->GetMainFixpackDate(iInx, jInx, kInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 5: short description
            lvItem.iSubItem = 4;
            lvItem.pszText = gplginParse->GetMainFixpackShortDscp(iInx, jInx, kInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 6: severity
            lvItem.iSubItem = 5;
            lvItem.pszText = gplginParse->GetPluginSeverity(iInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 7: impact on opentm2 assets
            lvItem.iSubItem = 6;
            lvItem.pszText = gplginParse->GetPluginImpact(iInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 8: action to be taken after installation
            lvItem.iSubItem = 7;
            lvItem.pszText = gplginParse->GetPluginAfterAction(iInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);
        }
        else
        {
            // update
            // column 2: type
            lvItem.iSubItem = 1;
            lvItem.pszText = gplginParse->GetPluginType(iInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 3: installed version
            lvItem.iSubItem = 2;
            lvItem.pszText = GetLocalPluginVer(gplginParse->GetPluginName(iInx));
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 4: available version
            lvItem.iSubItem = 3;
            lvItem.pszText = gplginParse->GetMainVersion(iInx, jInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 5: available date
            lvItem.iSubItem = 4;
            lvItem.pszText = gplginParse->GetMainFixpackDate(iInx, jInx, kInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 6, short description
            lvItem.iSubItem = 5;
            lvItem.pszText = gplginParse->GetMainFixpackShortDscp(iInx, jInx, kInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 7: severity
            lvItem.iSubItem = 6;
            lvItem.pszText = gplginParse->GetPluginSeverity(iInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 8: impact on opentm2 assets
            lvItem.iSubItem = 7;
            lvItem.pszText = gplginParse->GetPluginImpact(iInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);

            // column 9: action to be taken after installation
            lvItem.iSubItem = 8;
            lvItem.pszText = gplginParse->GetPluginAfterAction(iInx);
            ListView_SetItem(hwndPlgMgrLst, &lvItem);
        }

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

    sprintf(strFixpConf, "%s\\%s", gstrPluginPath, PLUGIN_MGR_FIXP_CONFIG);

    if (OTM_NOT_FOUND == access(strFixpConf, 0))
    {
        return bInstalled;
    }

    char strId[MAX_LEN];
    char strPluginName[MAX_LEN];
    char strPluginVer[MAX_VER_LEN];

    memset(strId,          0x00, sizeof(strId));
    memset(strPluginName,  0x00, sizeof(strPluginName));
    memset(strPluginVer,   0x00, sizeof(strPluginVer));

    SplitFixpackName(strFixpack, strPluginName, strPluginVer, strId);

    // set app value of config file
    char strApp[MAX_LEN];
    memset(strApp,  0x00, sizeof(strApp));
    sprintf(strApp, "%s_%s", strPluginName, strPluginVer);

    // read the fixpack state from config file
    int nState = GetPrivateProfileInt(strApp, strId, FIXPACK_STATE_DFT, strFixpConf);

    if (nState)
    {
        glogPlgMgr.writef("For fixpack %s_%s is installed.", strApp, strId);
        bInstalled = TRUE;
    }
    else
    {
        glogPlgMgr.writef("For fixpack %s_%s is not installed.", strApp, strId);
        bInstalled = FALSE;
    }

    return bInstalled;
}

int SetFixpackState(const char * strFixpack, int nState, BOOL bCreate)
{
    int nRC = NO_ERROR;

    char strFixpConf[MAX_PATH];
    memset(strFixpConf, 0x00, sizeof(strFixpConf));

    sprintf(strFixpConf, "%s\\%s", gstrPluginPath, PLUGIN_MGR_FIXP_CONFIG);
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
    char strPluginName[MAX_LEN];
    char strPluginVer[MAX_VER_LEN];

    memset(strId,          0x00, sizeof(strId));
    memset(strPluginName,  0x00, sizeof(strPluginName));
    memset(strPluginVer,   0x00, sizeof(strPluginVer));

    SplitFixpackName(strFixpack, strPluginName, strPluginVer, strId);

    // set app value of config file
    char strApp[MAX_LEN];
    memset(strApp,  0x00, sizeof(strApp));
    sprintf(strApp, "%s_%s", strPluginName, strPluginVer);

    char strValue[MAX_LEN];
    memset(strValue, 0x00, sizeof(strValue));
    sprintf(strValue, "%d", nState);

    // Write the status to file
    WritePrivateProfileString(strApp, strId, strValue, strFixpConf);

    return nRC;
}

void ClearFixpackState(const char * strPlugin)
{
    char strFixpConf[MAX_PATH];
    memset(strFixpConf, 0x00, sizeof(strFixpConf));
    sprintf(strFixpConf, "%s\\%s", gstrPluginPath, PLUGIN_MGR_FIXP_CONFIG);

    // if the file not exist, create the file first
    if (OTM_NOT_FOUND == access(strFixpConf, 0))
    {
        return;
    }

    char strFixpConfTmp[MAX_PATH];
    memset(strFixpConfTmp, 0x00, sizeof(strFixpConfTmp));
    sprintf(strFixpConfTmp, "%s\\%s", gstrPluginPath, PLUGIN_MGR_FIXP_CONFIG_TMP);

    FILE * fileConf = fopen(strFixpConf, "r");
    if (NULL == fileConf)
    {
        return;
    }

    FILE * fileConfTmp = fopen(strFixpConfTmp, "w+");
    if (NULL == fileConfTmp)
    {
        fclose(fileConf);
        return;
    }

    char strSetense[MAX_BUF_SIZE];
    memset(strSetense, 0x00, sizeof(strSetense));

    BOOL bSkip = FALSE;
    int nLine = 0;

    while (fgets(strSetense, MAX_BUF_SIZE, fileConf))
    {
        if ('[' == strSetense[0])
        {
            char strKey[MAX_BUF_SIZE];
            memset(strKey, 0x00, sizeof(strKey));
            sprintf(strKey, "[%s]", strPlugin);
            if (strnicmp(strKey, strSetense, strlen(strKey)))
            {
                bSkip = TRUE;
            }
            else
            {
                bSkip = FALSE;
                fprintf(fileConfTmp, "%s", strSetense);
                nLine++;
            }
        }
        else
        {
            if (!bSkip)
            {
                fprintf(fileConfTmp, "%s", strSetense);
                nLine++;
            }
        }

    }

    fclose(fileConf);
    fclose(fileConfTmp);

    remove(strFixpConf);
    if (nLine > 0)
    {
        rename(strFixpConfTmp, strFixpConf);
    }
    else
    {
        remove(strFixpConfTmp);
    }
}

char * GetLocalPluginVer(char * strPluginName)
{
    PluginManager*     thePluginManager;
    OtmPlugin*         otmPlugin;
    int nCnt, nInx;
    char * strPluginVer = EMPTY_STR;

    thePluginManager = PluginManager::getInstance();
    nCnt = thePluginManager->getPluginCount();
    for (nInx = 0; nInx < nCnt; nInx++)
    {
        otmPlugin = thePluginManager->getPlugin(nInx);
        if (!stricmp(strPluginName, otmPlugin->getName()))
        {
            strPluginVer = (char *) otmPlugin->getVersion();
            break;
        }
    }

    return strPluginVer;
}

void RemoveSelectedPlugin(HWND hwndInsTabDlg)
{
    // cofirm the delete first
    int nID = MessageBox(hwndInsTabDlg, WARNING_DELETE_CONFIRM_STR, PLUGIN_MGR_APP_NAME_STR, MB_YESNO | MB_WARNING | MB_DEFBUTTON2);
    if (nID != IDYES)
    {
        return;
    }

    // set cursor to wait status
    HCURSOR hcursorHold, hcursorWait;
    hcursorWait = LoadCursor(NULL,IDC_WAIT);
    hcursorHold = ::SetCursor(hcursorWait);

    HWND hwndDlgLst = GetDlgItem(hwndInsTabDlg, IDC_PLUGINMGR_LIST);
    if (!RemoveItemChk(hwndDlgLst))
    {
        return;
    }

    BOOL bSuccess = TRUE;
    int nTotal = 0;
    for (int nInx = 0; nInx < ListView_GetItemCount(hwndDlgLst); nInx++)
    {
        if (Otm_ListView_GetCheckState(hwndDlgLst, nInx) != STATE_CB_YES)
        {
            continue;
        }
        nTotal++;

        // get name from list
        char strPluginName[MAX_PATH];
        memset(strPluginName, 0x00, sizeof(strPluginName));
        ListView_GetItemText(hwndDlgLst, nInx, 0, strPluginName, sizeof(strPluginName));

        int nRC = RemovePlugin(hwndInsTabDlg, strPluginName);
        if (!nRC)
        {
            ListView_DeleteItem(hwndDlgLst, nInx);
            nInx--;
        }
        else
        {
            bSuccess = FALSE;
        }
    }
    PPLUGINMAGPROPIDA pIda;
    pIda = ACCESSDLGIDA(hwndInsTabDlg, PPLUGINMAGPROPIDA);
    SendMessage(pIda->hwndPages[1], WM_OTM_UPDATE, 0, 0);
    SendMessage(pIda->hwndPages[2], WM_OTM_UPDATE, 0, 0);

    if (0 == nTotal)
    {
        MessageBox(hwndInsTabDlg, WARNING_NO_ITEM_SELECTED, PLUGIN_MGR_APP_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
    }
    else if (bSuccess)
    {
        MessageBox(hwndInsTabDlg, INFO_TASK_END_SUCCESS_STR, PLUGIN_MGR_APP_NAME_STR, MB_OK);
    }
    else
    {
        MessageBox(hwndInsTabDlg, INFO_TASK_END_FAIL_STR, PLUGIN_MGR_APP_NAME_STR, MB_OK);
    }

    // recover the cursor
    hcursorWait = LoadCursor(NULL,IDC_ARROW);
    hcursorHold = ::SetCursor(hcursorWait);
}

int RemovePlugin(HWND hwndInsTabDlg, const char * strPluginName)
{
    PluginManager*     thePluginManager;
    OtmPlugin*         otmPlugin;
    int                nRC = NO_ERROR;

    thePluginManager = PluginManager::getInstance();
    otmPlugin = thePluginManager->getPlugin(strPluginName);

    char strVer[MAX_VER_LEN];
    memset(strVer, 0x00, sizeof(strVer));
    strcpy(strVer, otmPlugin->getVersion());
    nRC = RemoveOnePlugin(hwndInsTabDlg, strPluginName);
    gHistoryWriter->writef("PluginManangerHistory", "%s\tRemove\t%s\t%s\t%d", TimeManager::GetDateTimeStr(), strVer, strPluginName, nRC);

    return nRC;
}

int RemoveOnePlugin(HWND hwndTabDlg, string strPluginName)
{
    int nRC = NO_ERROR;
    nRC = RemoveWithConfig(hwndTabDlg, strPluginName);
    if (!nRC)
    {
        return nRC;
    }
    else if (ERROR_CANNOT_FIND_KEY_C == nRC)
    {
        nRC = RemoveAlone(hwndTabDlg, strPluginName.c_str());
    }

    return nRC;
}

int RemoveWithConfig(HWND hwndTabDlg, string strPluginName)
{
    int nRC = NO_ERROR;
    char strXml[MAX_PATH];
    char strMsg[MAX_BUF_SIZE];

    memset(strXml,   0x00, sizeof(strXml));
    memset(strMsg,   0x00, sizeof(strMsg));

    sprintf(strXml, "%s\\%s", gstrPluginPath, PLUGIN_MGR_LOC_XML);

    OTMGRPSTING grpPluginPath;
    glogPlgMgr.writef("Start get plugin %s path from xml %s.", strPluginName.c_str(), strXml);
    COTMCOPIES defPathCopyies;
    GetDefPathCopies(&defPathCopyies);
    nRC = gplginParse->GetPluginPaths(strXml, strPluginName.c_str(), &defPathCopyies, &grpPluginPath);
    glogPlgMgr.writef("End get plugin path from xml %d.", nRC);
    if (!nRC)
    {
        nRC = RemoveOneWithConfig(hwndTabDlg, strPluginName.c_str(), &grpPluginPath);
        glogPlgMgr.writef("Remove with config %d.", nRC);
    }
    else if (ERROR_OTM_FILE_NOT_FIND_A== nRC)
    {
        return nRC;
    }
    else if (ERROR_EMPTY_FILE_A == nRC)
    {
        sprintf(strMsg, ERROR_EMPTY_LOC_CONFIG_STR, strXml);
        MessageBox(hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
    }
    else if (ERROR_CANNOT_FIND_KEY_C == nRC)
    {
//        sprintf(strMsg, OtmGetMessageFromCode(ERROR_CANNOT_FIND_KEY_C), strXml, KEY_PLUGIN);
//        MessageBox(hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
    }
    else if (ERROR_PLUGIN_NO_ARRTI_B == nRC)
    {
        sprintf(strMsg, ERROR_CANNOT_FIND_ATTRI_STR, strPluginName.c_str());
        MessageBox(hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
    }
    else if (ERROR_REMOVE_PLUGIN_B == nRC)
    {
        sprintf(strMsg, OtmGetMessageFromCode(ERROR_REMOVE_PLUGIN_B), strPluginName.c_str());
        MessageBox(hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
    }
    else if (ERROR_DEREGISTER_PLUGIN_B == nRC)
    {
        // do nothing, error message has been displayed in sub function
    }
    else
    {
        sprintf(strMsg, OtmGetMessageFromCode(ERROR_REMOVE_PLUGIN_B), strPluginName.c_str(), nRC);
        MessageBox(hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
    }

    return nRC;
}

int RemoveOneWithConfig(HWND hwndTabDlg, const char * strPluginName, POTMGRPSTING pGrpPluginPath)
{
    int nRC = NO_ERROR;
    char strMsg[MAX_BUF_SIZE];
    memset(strMsg, 0x00, sizeof(strMsg));

    glogPlgMgr.writef("Start to remove plugin %s.", strPluginName);

    // first deregister plugin
    // deregister the plugin and remove it
    glogPlgMgr.writef("Start to deregister plugin %s.", strPluginName);
    nRC = DeregisterOtmPlugin(strPluginName);
    glogPlgMgr.writef("End to deregister plugin %d.", nRC);
    if (nRC)
    {
        sprintf(strMsg, OtmGetMessageFromCode(ERROR_DEREGISTER_PLUGIN_B), strPluginName);
        MessageBox(hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
        return nRC;
    }

    // delete one plugin's all related moudles one by one
    for (int nInx = 0; nInx < (int) pGrpPluginPath->size(); nInx++)
    {
        glogPlgMgr.writef("Removing %s.....", (*pGrpPluginPath)[nInx].c_str());
        nRC = OtmIsDirectory((*pGrpPluginPath)[nInx].c_str());
        if (IS_FOLDER == nRC)
        {
            nRC = OtmDeleteAllInDir((*pGrpPluginPath)[nInx].c_str());
            glogPlgMgr.writef("Has removed all in folder %s(%d).", (*pGrpPluginPath)[nInx].c_str(), nRC);
        }
        else if (IS_FILE == nRC)
        {
            if (!remove((*pGrpPluginPath)[nInx].c_str()))
            {
                nRC = NO_ERROR;
            }
            else
            {
                nRC = ERROR_OTM_REMOVE_FILE_B;
            }
            glogPlgMgr.writef("Remove file %s(%d).", (*pGrpPluginPath)[nInx].c_str(), nRC);
        }
        else if (NOT_EXIST == nRC)
        {
            nRC = ERROR_OTM_FILE_NOT_FIND_A;
            glogPlgMgr.writef("Not exist %s(%d).", (*pGrpPluginPath)[nInx].c_str(), nRC);
            break;
        }
        else
        {
            glogPlgMgr.writef("Remove failed %s(%d).", (*pGrpPluginPath)[nInx].c_str(), nRC);
            nRC = ERROR_OTM_REMOVE_FOLDER_B;
            break;
        }
    }
    return nRC;
}

int DeregisterOtmPlugin(const char * strPluginName)
{
    int nRC = ERROR_CANNOT_FIND_PLUGIN_B;

    PluginManager*     thePluginManager;
    thePluginManager = PluginManager::getInstance();

    // deregister the plugin first
    for (int nPluginInx = 0; nPluginInx < thePluginManager->getPluginCount(); nPluginInx++)
    {
        OtmPlugin* otmPlugin;
        otmPlugin = thePluginManager->getPlugin(nPluginInx);

        if (!stricmp(strPluginName, otmPlugin->getName()))
        {
            nRC = NO_ERROR;
            glogPlgMgr.writef("Start stop plugin %s.", otmPlugin->getName());
            if (!thePluginManager->stopPlugin((OtmPlugin*) otmPlugin, FALSE))
            {
                glogPlgMgr.writef("End stop plugin %s failed.", strPluginName);
                nRC = ERROR_DEREGISTER_PLUGIN_B;
            }
            break;
        }
    }

    return nRC;
}

int RemoveAlone(HWND hwndTabDlg, const char * strPluginName)
{
    int nRC = NO_ERROR;
    string strDllName;
    char strMsg[MAX_BUF_SIZE];
    memset(strMsg, 0x00, sizeof(strMsg));

    nRC = DeregisterOtmPlugin(strPluginName);
    if (nRC)
    {
        sprintf(strMsg, OtmGetMessageFromCode(ERROR_DEREGISTER_PLUGIN_B), strPluginName);
        MessageBox(hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
        return nRC;
    }

    // remove dll first
    string strFileSpec(gstrPluginPath);
    USHORT usDosRc;
    USHORT usCount = 1;
    HDIR hDir = HDIR_CREATE;
    FILEFINDBUF ffb;

    strFileSpec += "\\*.dll";
    usDosRc = UtlFindFirst((PSZ) strFileSpec.c_str(), &hDir, FILE_NORMAL, &ffb, sizeof(ffb), &usCount, 0L, FALSE);

    strDllName = strPluginName;
    strDllName += ".DLL";
    string strDll;
    while (usCount == 1)
    {
        if (!strnicmp(strDllName.c_str(), ffb.cFileName, strlen(ffb.cFileName)))
        {
            strDll = gstrPluginPath;
            strDll += '\\';
            strDll += ffb.cFileName;
            glogPlgMgr.writef("Delete dll:%s", strDll.c_str());
            if (!DeleteFile(strDll.c_str()))
            {
                nRC = ERROR_REMOVE_PLUGIN_B;
                sprintf(strMsg, OtmGetMessageFromCode(nRC), strPluginName);
                MessageBox(hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
                break;
            }
        }
        usDosRc = UtlFindNext(hDir, &ffb, sizeof(ffb), &usCount, FALSE);
    }
    if (hDir != 0)
    {
        UtlFindClose(hDir, FALSE);
    }
    if (NO_ERROR != nRC)
    {
        return nRC;
    }

    // now search and remove all subdirectories
    hDir = HDIR_CREATE;
    usCount = 1;
    strFileSpec = gstrPluginPath;
    strFileSpec += "\\*";
    usDosRc = UtlFindFirst((PSZ) strFileSpec.c_str(), &hDir, FILE_DIRECTORY, &ffb, sizeof(ffb), &usCount, 0L, FALSE);

    while (usCount == 1)
    {
        if (ffb.cFileName[0] != '.')
        {
            if (!stricmp(strPluginName, ffb.cFileName))
            {
                std::string strSubdir(gstrPluginPath);
                strSubdir += '\\';
                strSubdir += ffb.cFileName;

                nRC = OtmDeleteAllInDir(strSubdir.c_str());
                if (nRC)
                {
                    sprintf(strMsg, OtmGetMessageFromCode(nRC), strSubdir.c_str());
                    MessageBox(hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
                    break;
                }
            }
        }
        usDosRc = UtlFindNext(hDir, &ffb, sizeof(ffb), &usCount, FALSE);
    }
    if (hDir != 0)
    {
        UtlFindClose(hDir, FALSE);
    }

    return nRC;
}

void split(std::string instr, std::vector<std::string>& res, char sep)
{
	std::size_t iLast = 0;
	std::size_t idx = instr.find(sep,iLast);
	while(idx != std::string::npos)
	{
		std::string temp = instr.substr(iLast,idx-iLast);
		//std::cout<<temp<<std::endl;
		res.push_back(temp);
		// skip space
		idx++;
		while( isspace(instr[idx]) )
			idx++;
		// update idx loop
		iLast = idx;
		idx = instr.find(sep,iLast);
	}
	std::string temp = instr.substr(iLast);
	//std::cout<<temp<<std::endl;
	res.push_back(temp);
	
}


// WLP P403853
bool isForCurOtm(const char* pPlugginName, std::string &msg)
{
	//std::string strOtmVersion = gplginParse->GetOtmVersion(iInx);
	const char* pOtmVersion = gplginParse->GetOtmVersionByName(pPlugginName);

	// if there isn't otm version, it means for any otm version
	if (pOtmVersion==NULL || strlen(pOtmVersion)==0)
		return true;

	// check otm version string here
	std::string curOtmVersion = STR_DRIVER_LEVEL_NUMBER;
	std::string strOtmVersion = std::string(pOtmVersion);
	msg = strOtmVersion;

	// 1.4.2+
	std::size_t iLen = strOtmVersion.length();
	if(strOtmVersion[iLen-1] == '+')
	{
		msg = std::string("equal or above ")+strOtmVersion.substr(0,iLen-1);
		return ( curOtmVersion >= strOtmVersion.substr(0,iLen-1) );
	}
	// -1.4.2
	else if(strOtmVersion[0] == '-')
	{
		msg = std::string("equal or below ")+strOtmVersion.substr(1,iLen-1);
		return ( curOtmVersion <= strOtmVersion.substr(1,iLen-1) );
	}
	// 1.4.2, 1.4.3, 1.4.5
	else if(strOtmVersion.find(',') != std::string::npos)
	{
		msg = std::string("[ ")+strOtmVersion+"]";
		std::vector<std::string> vers;
		split(strOtmVersion,vers,',');
		for(auto iter=vers.begin(); iter!=vers.end(); iter++)
		{
			if(*iter == curOtmVersion)
				return true;
		}
	}
	// 1.3.1-1.4.2
	else if (strOtmVersion.find('-')!=std::string::npos)
	{
		std::vector<std::string> vers;
		split(strOtmVersion,vers,'-');
		if(vers.size() == 2)
		{
			msg = std::string("in range ")+vers[0]+" and "+vers[1];
			return ( (curOtmVersion >= vers[0]) && (curOtmVersion<=vers[1]) );
		}
	}
	// 1.4.2
	else
	{
		return (curOtmVersion == strOtmVersion);
	}

	return false;
}

void InstallSelectedPlugin(HWND hwndAvbTabDlg)
{
    int nRC = NO_ERROR;
    HWND hwndDlgLst = GetDlgItem(hwndAvbTabDlg, IDC_PLUGINMGR_LIST);

    int nTotal = 0;
    for (int nInx = 0; nInx < ListView_GetItemCount(hwndDlgLst); nInx++)
    {
        if (!ListView_GetCheckState(hwndDlgLst, nInx))
        {
            continue;
        }
		
        // get the name of the plugin
        char strLstName[PLUGIN_NAME_MAX_LEN];
        memset(strLstName, 0x00, sizeof(strLstName));
        ListView_GetItemText(hwndDlgLst, nInx, 0, strLstName, sizeof(strLstName));


		// WLP P403853
		std::string msg;
		if (!isForCurOtm(strLstName,msg) )
		{
			 std::string errorMsg("The plugin is for OTM ");
			 errorMsg += msg;
			 errorMsg += ", yours is ";
			 errorMsg += STR_DRIVER_LEVEL_NUMBER;

			 MessageBox(hwndAvbTabDlg, errorMsg.c_str(), PLUGIN_MGR_APP_NAME_STR, MB_ICONEXCLAMATION | MB_OK);
             return;
		}


        // if fixpackage, just + 1
        if (IsFixpack(strLstName))
        {
            int nChked = IsFixpackParentChked(hwndDlgLst, strLstName, 0, 2); // name index is 0, version index is 2
            if (PARENT_NOT_CHECKED == nChked)
            {
                MessageBox(hwndAvbTabDlg, ERROR_FIXPACK_PARENT_NOT_CHECKED, PLUGIN_MGR_APP_NAME_STR, MB_ICONEXCLAMATION | MB_OK);
                return;
            }
            nTotal++;
            continue;
        }

        // get one component's count
        nTotal += gplginParse->GetPluginCntByName(strLstName);
    }

    // if there is no item is selected, just return
    if (0 == nTotal)
    {
        MessageBox(hwndAvbTabDlg, WARNING_NO_ITEM_SELECTED, PLUGIN_MGR_APP_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    // each task is divided to two parst: start and end, so the total count should multiply by 2
    nTotal = nTotal * 2;
	if(gOtmHttps != NULL)
		delete gOtmHttps;
    gOtmHttps = new COtmHttps();
    HINSTANCE hInst = (HINSTANCE) GetWindowLong(hwndAvbTabDlg, GWL_HINSTANCE);
    HWND ghwndPrgCtrlDlg;
    ghwndPrgCtrlDlg = CreateDialogParam(hInst,
                                        MAKEINTRESOURCE(IDD_PROGRESS_CTRL_DLG),
                                        hwndAvbTabDlg,
                                        DownloadPCDlgProc,
                                        nTotal);
    ShowWindow(ghwndPrgCtrlDlg, SW_SHOW);
    EnableWindow(ghwndDlg, FALSE);

    PDOWNLOADPARAM pDownloadParam = new DOWNLOADPARAM;

    pDownloadParam->hwndTabDlg = hwndAvbTabDlg;
    pDownloadParam->hwndPCDlg  = ghwndPrgCtrlDlg;
    pDownloadParam->nType      = TYPE_NEW_INSTALL;
    if (NULL == _beginthreadex(NULL, 0, DownloadPCThreadProc, pDownloadParam, 0, 0))
    {
        nRC = ERROR_CREATE_THREAD_A;
    }
}


void UpdateSelectedPlugin(HWND hwndUptTabDlg)
{
    int nRC = NO_ERROR;
    HWND hwndDlgLst = GetDlgItem(hwndUptTabDlg, IDC_PLUGINMGR_LIST);

    int nTotal = 0;
    for (int nInx = 0; nInx < ListView_GetItemCount(hwndDlgLst); nInx++)
    {
        if (!ListView_GetCheckState(hwndDlgLst, nInx))
        {
            continue;
        }


        // get the name of the plugin
        char strLstName[PLUGIN_NAME_MAX_LEN];
        memset(strLstName, 0x00, sizeof(strLstName));
        ListView_GetItemText(hwndDlgLst, nInx, 0, strLstName, sizeof(strLstName));


		// WLP P403853
		std::string msg;
		if ( !isForCurOtm(strLstName,msg) )
		{
			 std::string errorMsg("The plugin is for OTM ");
			 errorMsg += msg;
			 errorMsg += ", yours is ";
			 errorMsg += STR_DRIVER_LEVEL_NUMBER;

			 MessageBox(hwndUptTabDlg, errorMsg.c_str(), PLUGIN_MGR_APP_NAME_STR, MB_ICONEXCLAMATION | MB_OK);
             return;
		}


        // if fixpackage, just + 1
        if (IsFixpack(strLstName))
        {
            int nChked = IsFixpackParentChked(hwndDlgLst, strLstName, 0, 3);
            if (PARENT_NOT_CHECKED == nChked)
            {
                MessageBox(hwndUptTabDlg, ERROR_FIXPACK_PARENT_NOT_CHECKED, PLUGIN_MGR_APP_NAME_STR, MB_ICONEXCLAMATION | MB_OK);
                return;
            }
            nTotal++;
            continue;
        }

        // get one component's count
        nTotal += gplginParse->GetPluginCntByName(strLstName);
    }

    // if there is no item is selected, just return
    if (0 == nTotal)
    {
        MessageBox(hwndUptTabDlg, WARNING_NO_ITEM_SELECTED, PLUGIN_MGR_APP_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    // each task is divided to two parst: start and end, so the total count should multiply by 2
    nTotal = nTotal * 2;
	if(gOtmHttps != NULL)
		delete gOtmHttps;
    gOtmHttps = new COtmHttps();
    HINSTANCE hInst = (HINSTANCE) GetWindowLong(hwndUptTabDlg, GWL_HINSTANCE);
    HWND ghwndPrgCtrlDlg;
    ghwndPrgCtrlDlg = CreateDialogParam(hInst,
                                        MAKEINTRESOURCE(IDD_PROGRESS_CTRL_DLG),
                                        hwndUptTabDlg,
                                        DownloadPCDlgProc,
                                        nTotal);
    ShowWindow(ghwndPrgCtrlDlg, SW_SHOW);
    EnableWindow(ghwndDlg, FALSE);

    PDOWNLOADPARAM pDownloadParam = new DOWNLOADPARAM;

    pDownloadParam->hwndTabDlg = hwndUptTabDlg;
    pDownloadParam->hwndPCDlg  = ghwndPrgCtrlDlg;
    pDownloadParam->nType      = TYPE_UPDATE;
    if (NULL == _beginthreadex(NULL, 0, DownloadPCThreadProc, pDownloadParam, 0, 0))
    {
        nRC = ERROR_CREATE_THREAD_A;
    }
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
            DownloadStart(hwndPrgCtrlDlg, pDownloadParam);
        }
        break;

    case WM_OTM_UPDATE_ONE_END:
        {
            PDOWNLOADPARAM pDownloadParam = (PDOWNLOADPARAM) mp1;
            DownloadEnd(hwndPrgCtrlDlg, pDownloadParam);
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

// Dowload, intall and update process control
unsigned int __stdcall DownloadPCThreadProc(LPVOID lpParameter)
{
    int nRC            = NO_ERROR;
    int nProcessedCnt  = 0;
    int nSuccessCnt    = 0;

    PDOWNLOADPARAM pDownloadParam = (PDOWNLOADPARAM) lpParameter;
    HWND hwndDlgLst = GetDlgItem(pDownloadParam->hwndTabDlg, IDC_PLUGINMGR_LIST);

    int nTotalCnt = GetLstTotalCnt(hwndDlgLst);
    char strMsg[MAX_BUF_SIZE];
    memset(strMsg, 0x00, sizeof(strMsg));

    char strPreNdRestarPlugin[MAX_LEN];
    memset(strPreNdRestarPlugin, 0x00, sizeof(strPreNdRestarPlugin));

    for (int nInx = 0; nInx < ListView_GetItemCount(hwndDlgLst); nInx++)
    {
        if (!ListView_GetCheckState(hwndDlgLst, nInx))
        {
            // if item not checked, just continue
            continue;
        }

        nProcessedCnt++;

        memset(pDownloadParam->strDLUrl,    0x00, sizeof(pDownloadParam->strDLUrl));
        memset(pDownloadParam->strDLFile,   0x00, sizeof(pDownloadParam->strDLFile));
        memset(pDownloadParam->strDLType,   0x00, sizeof(pDownloadParam->strDLType));
        memset(pDownloadParam->strMethod, 0x00, sizeof(pDownloadParam->strMethod));
        pDownloadParam->bRestart  = FALSE;
        pDownloadParam->bNeedWait = TRUE;

        char strLstPluginName[MAX_PATH];
        memset(strLstPluginName, 0x00, sizeof(strLstPluginName));
        ListView_GetItemText(hwndDlgLst, nInx, 0, strLstPluginName, sizeof(strLstPluginName));

        // download dll
        pDownloadParam->nProcessedCnt  = nProcessedCnt;
        pDownloadParam->nSuccessCnt    = nSuccessCnt;
        pDownloadParam->nTotalCnt      = nTotalCnt;
        pDownloadParam->nLstInx        = nInx;

        MAINTHREADINFO mainTdInfo;
        InitMainThreadInfo(&mainTdInfo);

        // According to the name from list to get the plugin's real name, version and position info
        nRC = GetPluginInfoForLst(hwndDlgLst, pDownloadParam->nType, nInx, &mainTdInfo);
        if (NO_ERROR != nRC)
        {
            // if download failed, show errror message and continue next
            SendMessage(pDownloadParam->hwndPCDlg, WM_OTM_UPDATE_ONE_START, (WPARAM) pDownloadParam, 0);
            sprintf(strMsg, OtmGetMessageFromCode(nRC), mainTdInfo.strName);
            MessageBox(pDownloadParam->hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
            pDownloadParam->nRC = nRC;
            EndOneTask(pDownloadParam, &mainTdInfo);
            continue;
        }

        // Get download info
        GetXmlDownloadInfo(&mainTdInfo, pDownloadParam);

        // set check whether need restart
        if (!mainTdInfo.bFixpack && pDownloadParam->bRestart)
        {
            // if not fixpack and need restart, set name of pre need restart plugin
            strcpy(strPreNdRestarPlugin, strLstPluginName);
        }
        else if (mainTdInfo.bFixpack)
        {
            // if the plugin name is same as previous need restart plugin, set its following fixpack to restart too
            if (!stricmp(mainTdInfo.strName, strPreNdRestarPlugin))
            {
                pDownloadParam->bRestart = TRUE;
            }
        }

        // show task start
        SendMessage(pDownloadParam->hwndPCDlg, WM_OTM_UPDATE_ONE_START, (WPARAM) pDownloadParam, 0);

        if (TYPE_UPDATE == pDownloadParam->nType)
        {
            if (!mainTdInfo.bFixpack && !pDownloadParam->bRestart)
            // if not plugin and not need restart, the plugin must deregister first
            {
                // first deresgier plugin
                glogPlgMgr.writef("Not fixpack and needn't restart, remove plugin %s first.", strLstPluginName);
                nRC = RemoveOnePlugin(pDownloadParam->hwndTabDlg, strLstPluginName);
                glogPlgMgr.writef("End deregister plugin %s.(%d)", strLstPluginName, nRC);
            }
            else if (mainTdInfo.bFixpack && !pDownloadParam->bRestart)
            {
                // first, stop and deregister the plugin
                glogPlgMgr.writef("For fixpack, need deregister plugin %s first.", mainTdInfo.strName);
                nRC = DeregisterOtmPlugin(mainTdInfo.strName);
                glogPlgMgr.writef("End deregister plugin %s.(%d)", mainTdInfo.strName, nRC);
            }

            // if cannot remove the plugin and it's not plugin not existed error
            if ((NO_ERROR != nRC) && (ERROR_CANNOT_FIND_PLUGIN_B != nRC))
            {
                // if remove plugin failed, show error message and continue next
                sprintf(strMsg, OtmGetMessageFromCode(nRC), strLstPluginName);
                MessageBox(pDownloadParam->hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_OK);
                pDownloadParam->nRC = nRC;
                EndOneTask(pDownloadParam, &mainTdInfo);
                continue;
            }
        }
        else
        {
            if (!mainTdInfo.bFixpack)
            {
                // if new install plugin, check whether need show dependency info
                char strDepnsStr[MAX_BUF_SIZE];
                nRC = GetDepnsStr(&mainTdInfo, strDepnsStr);
                if (NO_ERROR != nRC)
                {
                    // if get dependency failed, show error message and continue next
                    sprintf(strMsg, OtmGetMessageFromCode(nRC), strLstPluginName);
                    MessageBox(pDownloadParam->hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
                    pDownloadParam->nRC = nRC;
                    EndOneTask(pDownloadParam, &mainTdInfo);
                    continue;
                }

                if ((NULL != strDepnsStr) && (strlen(strDepnsStr) != 0))
                {
                    // show depedency warning message
                    sprintf(strMsg, WARNING_TRD_PATH_SFW_STR, strLstPluginName, strDepnsStr);
                    MessageBox(pDownloadParam->hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_OK | MB_WARNING);
                }
            }
        }

        // get the name of download plugin dll
        nRC = GetPluginDllPath(pDownloadParam->strDLUrl, pDownloadParam->bRestart, pDownloadParam->strDLFile);
        if (nRC)
        {
            // if failed to get DLL path, show error message and continue next
            sprintf(strMsg, OtmGetMessageFromCode(nRC));
            MessageBox(pDownloadParam->hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
            pDownloadParam->nRC = nRC;
            EndOneTask(pDownloadParam, &mainTdInfo);
            continue;
        }

        // Download the file from the network
        nRC = OtmDownloadFile(pDownloadParam);
        if (nRC)
        {
            // if failed to download package, show error message and return
            sprintf(strMsg, OtmGetMessageFromCode(nRC));
            MessageBox(pDownloadParam->hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
            pDownloadParam->nRC = ERROR_END_ALL_TASK;
            EndOneTask(pDownloadParam, &mainTdInfo);
            return nRC;
        }
        else if (!nRC && !pDownloadParam->bRestart)
        {
            // check whether need unzip file
            OTMGRPSTING strUnzipFiles;
            strUnzipFiles.clear();
            if (!stricmp(DLTYPE_ZIP, pDownloadParam->strDLType))
            {
                COTMCOPIES pluginCopyies;
                nRC = gplginParse->GetPluginCopies(&mainTdInfo, &pluginCopyies);

                if (!nRC)
                {
                    glogPlgMgr.writef("Start unzip(%s).", pDownloadParam->strDLFile);
                    COTMCOPIES defPathCopyies;
                    GetDefPathCopies(&defPathCopyies);
                    nRC = OtmUnCompress(pDownloadParam->strDLFile, &defPathCopyies, &pluginCopyies, gstrPluginPath, &strUnzipFiles);
                    glogPlgMgr.writef("End unzip(%d).", nRC);
                }
                else
                {
                    glogPlgMgr.writef("Get copy info failed.(%s).", strLstPluginName);
                }
            }

            if (!nRC)
            {
                // if success, delete the unzip file
                int nKeepPkg = GetPrivateProfileInt(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_KEEP_PKG, PLUGIN_MGR_KEEP_PKG_DFT, gstrConfigPath);
                if (!nKeepPkg)
                {
                    remove(pDownloadParam->strDLFile);
                }

                // check whether is installer, if is, do the installation
                if (!stricmp(METHOD_INSTALL, pDownloadParam->strMethod))
                {
                    for (size_t iInx = 0; iInx < strUnzipFiles.size(); iInx++)
                    {
                        glogPlgMgr.writef("Execute command unzip(%s).", strUnzipFiles[iInx].c_str());
                        nRC = OtmExecuteCommand((char *)strUnzipFiles[iInx].c_str(), NULL, TRUE);
                        glogPlgMgr.writef("Execute command end(%d).", nRC);
                        if (!nRC)
                        {
                            remove(strUnzipFiles[iInx].c_str());
                        }
                    }
                }
                else
                {
                    // Add log info
                    for (size_t iInx = 0; iInx < strUnzipFiles.size(); iInx++)
                    {
                        glogPlgMgr.writef("Unzip file (%s).", strUnzipFiles[iInx].c_str());
                    }
                }

                if (mainTdInfo.bFixpack)
                {
                    // if success to install the fixpack, set the install status
                    glogPlgMgr.writef("For %s installed successfully, set install state.", strLstPluginName);
                    SetFixpackState(strLstPluginName, STATE_INSTALLED, FALSE);
                }
            }
            else
            {
                sprintf(strMsg, OtmGetMessageFromCode(nRC));
                MessageBox(pDownloadParam->hwndTabDlg, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
                glogPlgMgr.writef("Download failed(%s).", pDownloadParam->strDLFile);
            }
        }
        else if (!nRC && pDownloadParam->bRestart)
        {
            // if need restart, modify pending config file
            glogPlgMgr.writef("Need restart", pDownloadParam->strDLFile);
            UpdatePendUptFiles(&mainTdInfo, pDownloadParam->nType);
        }

        pDownloadParam->nRC = nRC;
        if (!nRC)
        {
            nSuccessCnt++;
            pDownloadParam->nSuccessCnt = nSuccessCnt;

            // if not fixpack, clear the all the fixpack status
            if (!mainTdInfo.bFixpack)
            {
                ClearFixpackState(strLstPluginName);
            }
        }
        EndOneTask(pDownloadParam, &mainTdInfo);
    }

    return nRC;
}

int OtmDownloadFile(PDOWNLOADPARAM pDownloadParam)
{
    int nRC = NO_ERROR;
    glogPlgMgr.writef("Download %s start.", pDownloadParam->strDLUrl);

    // update one component
    NETWORKPARAM networkParam;
    InitNetworkParam(&networkParam);

    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_PROXY_ADDRESS, EMPTY_STR, networkParam.strProxyAddress, sizeof(networkParam.strProxyAddress), gstrConfigPath);
    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_PROXY_PORT, EMPTY_STR, networkParam.strProxyPort, sizeof(networkParam.strProxyPort), gstrConfigPath);
    networkParam.nTimeout = GetPrivateProfileInt(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_TIMEOUT, DEF_CONNECT_TIMEOUT, gstrConfigPath);

    nRC = gOtmHttps->DownloadFile(pDownloadParam->strDLUrl, pDownloadParam->strDLFile, &networkParam);

   
    if (nRC)
    {
        // if failed to download package, show error message and return
        return nRC;
    }

    // decrypt the file

    return nRC;
}

void EndOneTask(PDOWNLOADPARAM pDownloadParam, PMAINTHREADINFO pMainTdInfo)
{
    // Add to history log
    SendMessage(pDownloadParam->hwndPCDlg, WM_OTM_UPDATE_ONE_END, (WPARAM) pDownloadParam, 0);

    if (pDownloadParam->bRestart)
    {
        return;
    }

    const char * strType;
    if (TYPE_UPDATE == pDownloadParam->nType)
    {
        strType = TYPE_NEW_INSTALL_STR;
    }
    else
    {
        strType = TYPE_UPDATE_STR;
    }

    if (!pMainTdInfo->bFixpack)
    {
        gHistoryWriter->writef("PluginManangerHistory", "%s\t%s\t%s\t%s\t%d", TimeManager::GetDateTimeStr(), strType, 
                              pMainTdInfo->strVer, pMainTdInfo->strName, pDownloadParam->nRC);
    }
    else
    {
        gHistoryWriter->writef("PluginManangerHistory", "%s\t%s\t%s\t%s\t%s\t%d", TimeManager::GetDateTimeStr(), strType, 
                              pMainTdInfo->strVer, pMainTdInfo->strName, pMainTdInfo->strFixpkId, pDownloadParam->nRC);
    }
}

void ShowLongDscp(HWND hwndDlg, int nType)
{
    int nRC = NO_ERROR;

    HWND plginMgrList = GetDlgItem(hwndDlg, IDC_PLUGINMGR_LIST);
    int iSelInx = -1;

    while((iSelInx = ListView_GetNextItem(plginMgrList, iSelInx, LVNI_SELECTED)) != -1)
    {
        MAINTHREADINFO mainTdInfo;
        InitMainThreadInfo(&mainTdInfo);

        // According to the name from list to get the plugin's real name, version and position info
        nRC = GetPluginInfoForLst(plginMgrList, nType, iSelInx, &mainTdInfo);

        int iInx = mainTdInfo.iInx;
        int jInx = mainTdInfo.jInx;

        if (mainTdInfo.bFixpack)
        {
            // for fixpack info, always read from XML file
            int kInx = mainTdInfo.kInx;
            SetDlgItemText(hwndDlg, IDC_STC_LONG_DESC, gplginParse->GetMainFixpackLongDscp(iInx, jInx, kInx));
            return;
        }
        else
        {
            // get corresponding plugin's long description first
            int nPluginInx, nPluginTotalCnt;

            // find the plugin in PluginManager class
            PluginManager*  thePluginManager;
            OtmPlugin*      otmPlugin  = NULL;

            thePluginManager       = PluginManager::getInstance();
            nPluginTotalCnt        = thePluginManager->getPluginCount();

            for (nPluginInx = 0; nPluginInx < nPluginTotalCnt; nPluginInx++)
            {
                otmPlugin = thePluginManager->getPlugin(nPluginInx);
                char * strLocalPlginNam = (char *) otmPlugin->getName();
                if (!stricmp(strLocalPlginNam, mainTdInfo.strName))
                {
                    SetDlgItemText(hwndDlg, IDC_STC_LONG_DESC, otmPlugin->getLongDescription());
                    return;
                }
            }

            // if not found, that mean the long description should be found from the xml file
            SetDlgItemText(hwndDlg, IDC_STC_LONG_DESC, gplginParse->GetPluginLongDscp(iInx));
        }
    }
}

void SetInfoFromConfig(HWND hwndTabDlg)
{
    char strConfigPath[MAX_PATH];
    memset(strConfigPath,    0x00, sizeof(strConfigPath));
    sprintf(strConfigPath, "%s\\%s", gstrPluginPath, PLUGIN_MGR_CONFIG);

    char strUrl[MAX_BUF_SIZE];
    memset(strUrl, 0x00, sizeof(strUrl));

    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_URL, EMPTY_STR, strUrl, sizeof(strUrl), strConfigPath);
    SetDlgItemText(hwndTabDlg, IDC_EDT_URL, strUrl);

    char strProxyAddress[MAX_PROXY_ADDRESS_LEN];
    memset(strProxyAddress, 0x00, sizeof(strProxyAddress));

    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_PROXY_ADDRESS, EMPTY_STR, strProxyAddress, sizeof(strProxyAddress), strConfigPath);
    SetDlgItemText(hwndTabDlg, IDC_EDT_PROXY_ADDRESS, strProxyAddress);

    char strProxyPort[MAX_PROXY_ADDRESS_LEN];
    memset(strProxyPort, 0x00, sizeof(strProxyPort));

    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_PROXY_PORT, EMPTY_STR, strProxyPort, sizeof(strProxyPort), strConfigPath);
    SetDlgItemText(hwndTabDlg, IDC_EDT_PROXY_PORT, strProxyPort);

    int nTimeout = GetPrivateProfileInt(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_TIMEOUT, DEF_CONNECT_TIMEOUT, strConfigPath);
    SetDlgItemInt(hwndTabDlg, IDC_EDT_TIMEOUT, nTimeout, FALSE);

    // set check frequency option
    int nFrequency = ::GetPrivateProfileInt(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_FREQUENCY, PLUGIN_MGR_FREQUENCY_DFT, strConfigPath);
    int nCheckedID;
    switch(nFrequency)
    {
    case Day:
        nCheckedID = IDC_RADIO_DAY;
        break;
    case Week:
        nCheckedID = IDC_RADIO_WEEK;
        break;
	case Month://Manual:
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
    int nKeepPkg = GetPrivateProfileInt(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_KEEP_PKG, PLUGIN_MGR_KEEP_PKG_DFT, strConfigPath);
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

    char strConfigPath[MAX_PATH];
    memset(strConfigPath,    0x00, sizeof(strConfigPath));
    sprintf(strConfigPath, "%s\\%s", gstrPluginPath, PLUGIN_MGR_CONFIG);

    char strUrl[MAX_BUF_SIZE];
    memset(strUrl, 0x00, sizeof(strUrl));

    GetDlgItemText(hwndTabDlg, IDC_EDT_URL, strUrl, sizeof(strUrl));
    WritePrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_URL, strUrl, strConfigPath);

    char strProxyAddress[MAX_PROXY_ADDRESS_LEN];
    memset(strProxyAddress, 0x00, sizeof(strProxyAddress));

    GetDlgItemText(hwndTabDlg, IDC_EDT_PROXY_ADDRESS, strProxyAddress, sizeof(strProxyAddress));
    WritePrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_PROXY_ADDRESS, strProxyAddress, strConfigPath);

    char strProxyPort[MAX_PROXY_ADDRESS_LEN];
    memset(strProxyPort, 0x00, sizeof(strProxyPort));

    GetDlgItemText(hwndTabDlg, IDC_EDT_PROXY_PORT, strProxyPort, sizeof(strProxyPort));
    WritePrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_PROXY_PORT, strProxyPort, strConfigPath);

    char strTimeout[MAX_BUF_SIZE];
    memset(strTimeout, 0x00, sizeof(strTimeout));

    GetDlgItemText(hwndTabDlg, IDC_EDT_TIMEOUT, strTimeout, sizeof(strTimeout));
    WritePrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_TIMEOUT, strTimeout, strConfigPath);

    int nOldVal = GetPrivateProfileInt(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_FREQUENCY, PLUGIN_MGR_FREQUENCY_DFT, strConfigPath);
    int nNewVal = 0;
    // when close the dialog, record the status of startup check box
    if (IsDlgButtonChecked(hwndTabDlg, IDC_RADIO_DAY))
    {
        nNewVal = 0;
        WritePrivateProfileString(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_FREQUENCY, "0", strConfigPath);
    }
    else if (IsDlgButtonChecked(hwndTabDlg, IDC_RADIO_WEEK))
    {
        nNewVal = 1;
        WritePrivateProfileString(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_FREQUENCY, "1", strConfigPath);
    }
    else if (IsDlgButtonChecked(hwndTabDlg, IDC_RADIO_MANUAL))
    {
        nNewVal = 2;
        WritePrivateProfileString(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_FREQUENCY, "2", strConfigPath);
    }

    // set check status for check box
    if (IsDlgButtonChecked(hwndTabDlg, IDC_CHK_KEEP_PKG))
    {
        WritePrivateProfileString(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_KEEP_PKG, "1", strConfigPath);
    }
    else
    {
        WritePrivateProfileString(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_KEEP_PKG, "0", strConfigPath);
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
        int nID = MessageBox(hwndTabDlg, INFO_FREQUENCY_CHANGED_STR, PLUGIN_MGR_APP_NAME_STR, MB_YESNO | MB_DEFBUTTON1);
        if (IDYES == nID)
        {
            CheckNowRefresh(hwndTabDlg);
        }
    }
    else
    {
        MessageBox(hwndTabDlg, INFO_SAVE_SUCCESS_STR, PLUGIN_MGR_APP_NAME_STR, MB_OK);
    }
}

BOOL IsSettingChanged(HWND hwndTabDlg)
{
    BOOL bChanged = FALSE;

    char strConfigPath[MAX_PATH];
    memset(strConfigPath,    0x00, sizeof(strConfigPath));
    sprintf(strConfigPath, "%s\\%s", gstrPluginPath, PLUGIN_MGR_CONFIG);

    // compare whether Url has changed
    char strNewVal[MAX_BUF_SIZE];
    char strOldVal[MAX_BUF_SIZE];
    memset(strNewVal, 0x00, sizeof(strNewVal));
    memset(strOldVal, 0x00, sizeof(strOldVal));

    GetDlgItemText(hwndTabDlg, IDC_EDT_URL, strNewVal, sizeof(strNewVal));
    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_URL, EMPTY_STR, strOldVal, sizeof(strOldVal), strConfigPath);

    if (stricmp(strNewVal, strOldVal))
    {
        bChanged = TRUE;
        return bChanged;
    }

    // compare whether proxy address changed
    GetDlgItemText(hwndTabDlg, IDC_EDT_PROXY_ADDRESS, strNewVal, sizeof(strNewVal));
    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_PROXY_ADDRESS, EMPTY_STR, strOldVal, sizeof(strOldVal), strConfigPath);

    if (stricmp(strNewVal, strOldVal))
    {
        bChanged = TRUE;
        return bChanged;
    }

    // compare whether proxy port changed
    GetDlgItemText(hwndTabDlg, IDC_EDT_PROXY_PORT, strNewVal, sizeof(strNewVal));
    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_PROXY_PORT, EMPTY_STR, strOldVal, sizeof(strOldVal), strConfigPath);

    if (stricmp(strNewVal, strOldVal))
    {
        bChanged = TRUE;
        return bChanged;
    }

    // compare whether timeout changed
    GetDlgItemText(hwndTabDlg, IDC_EDT_TIMEOUT, strNewVal, sizeof(strNewVal));
    GetPrivateProfileString(APP_PLUGIN_MGR_NET_SET, KEY_PLUGIN_MGR_TIMEOUT, EMPTY_STR, strOldVal, sizeof(strOldVal), strConfigPath);

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
    nOldVal = GetPrivateProfileInt(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_FREQUENCY, PLUGIN_MGR_FREQUENCY_DFT, strConfigPath);

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
    nOldVal = GetPrivateProfileInt(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_KEEP_PKG, PLUGIN_MGR_KEEP_PKG_DFT, strConfigPath);

    if (nNewVal != nOldVal)
    {
        bChanged = TRUE;
        return bChanged;
    }

    return bChanged;
}

void DownloadStart(HWND hwndPCDlg, PDOWNLOADPARAM pDownloadParam)
{
    char strStatic[MAX_BUF_SIZE];
    char strName[MAX_PATH];

    memset(strStatic,   0x00, sizeof(strStatic));
    memset(strName,     0x00, sizeof(strName));

    GetNameFromUrl(strName, pDownloadParam->strDLUrl);
    glogPlgMgr.writef("Check %s, %s", strName, pDownloadParam->strDLUrl);
    sprintf(strStatic, DOWNLOAD_STR, strName);
    SetDlgItemText(hwndPCDlg, IDC_STATIC_CURRENT, strStatic);

    memset(strStatic,    0x00, sizeof(strStatic));
    sprintf(strStatic, PROGRESS_START_STR, pDownloadParam->nProcessedCnt, pDownloadParam->nTotalCnt);
    SetDlgItemText(hwndPCDlg, IDC_STATIC_TOTAL, strStatic);

    HWND hwndPBTotal = GetDlgItem(hwndPCDlg, IDC_PROGRESS_BAR_TOTAL);
    SendMessage(hwndPBTotal, PBM_SETPOS, (WPARAM)pDownloadParam->nProcessedCnt*2-1, (LPARAM)0);
}

void DownloadEnd(HWND hwndPCDlg, PDOWNLOADPARAM pDownloadParam)
{
    char strStatic[MAX_BUF_SIZE];
    memset(strStatic,   0x00, sizeof(strStatic));
    sprintf(strStatic, PROGRESS_END_STR, pDownloadParam->nProcessedCnt, pDownloadParam->nTotalCnt);
    SetDlgItemText(hwndPCDlg, IDC_STATIC_TOTAL, strStatic);

    HWND hwndPBTotal = GetDlgItem(hwndPCDlg, IDC_PROGRESS_BAR_TOTAL);
    SendMessage(hwndPBTotal, PBM_SETPOS, (WPARAM)pDownloadParam->nProcessedCnt*2, (LPARAM)0);

    PluginManager* thePluginManager = PluginManager::getInstance();
    // if error occurred and not necessary to continue, just quit
    if (ERROR_END_ALL_TASK == pDownloadParam->nRC)
    {
        // Update all lists
        thePluginManager->loadPluginDlls(gstrPluginPath);
        thePluginManager->activateToolPlugins();  // Add for P402792
        RefreshAllList(pDownloadParam->hwndTabDlg, FALSE);

        SendMessage(hwndPCDlg, WM_CLOSE, 0, 0);
        EnableWindow(ghwndDlg, TRUE);

        return;
    }

    if (pDownloadParam->nProcessedCnt < pDownloadParam->nTotalCnt)
    {
        return;
    }

    // Update all lists
    thePluginManager->loadPluginDlls(gstrPluginPath);
    thePluginManager->activateToolPlugins();  // Add for P402792
    RefreshAllList(pDownloadParam->hwndTabDlg, FALSE);

    SendMessage(hwndPCDlg, WM_CLOSE, 0, 0);
    EnableWindow(ghwndDlg, TRUE);

    // show restart request if necessary
    char strPendUptConf[MAX_PATH];

    memset(strPendUptConf, 0x00, sizeof(strPendUptConf));
    sprintf(strPendUptConf, "%s\\%s", gstrPluginPath, PENDING_UPT_CONF);

    // Get the max waiting time and max waiting times from config file
    int bRestart = GetPrivateProfileInt(APP_SETTINGS_STR, KEY_NEED_RESTART, 0, strPendUptConf);

    if (bRestart)
    {
        MessageBox(pDownloadParam->hwndTabDlg, WARNING_PEND_UPT_STR, PLUGIN_MGR_APP_NAME_STR, MB_OK | MB_WARNING);
    }
    else
    {
        if (pDownloadParam->nSuccessCnt == pDownloadParam->nTotalCnt)
        {
            MessageBox(pDownloadParam->hwndTabDlg, INFO_TASK_END_SUCCESS_STR, PLUGIN_MGR_APP_NAME_STR, MB_OK);
        }
        else
        {
            MessageBox(pDownloadParam->hwndTabDlg, INFO_TASK_END_FAIL_STR, PLUGIN_MGR_APP_NAME_STR, MB_OK);
        }
    }
}

int GetPluginDllPath(const char * strDLUrl, BOOL bRestart, char * strDLFile)
{
    int nRC = NO_ERROR;

    string strUrl(strDLUrl);
    unsigned nPos = strUrl.rfind("/");
    if (std::string::npos == nPos)
    {
        nRC = ERROR_CANNOT_GET_MODULE_NAME_A;
    }
    else
    {
        string strDLName;
        strDLName = strUrl.substr(nPos+1);

        if (NULL != strDLFile)
        {
            char strDir[MAX_PATH];
            memset(strDir, 0x00, sizeof(strDir));

            if (bRestart)
            {
                // if need install or update the plugin need restart, use pending dir
                sprintf(strDir, "%s\\%s\\", gstrPluginPath, PENDING_DIR_STR);
            }
            else
            {
                // if nothing special, use download dir
                sprintf(strDir, "%s\\%s\\", gstrPluginPath, DOWNLOAD_DIR_STR);
            }

            glogPlgMgr.writef("Create folder %s.", strDir);
            OtmCreateFolder(strDir);
            sprintf(strDLFile, "%s%s", strDir, strDLName.c_str());
        }
    }

    return nRC;
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

BOOL RemoveItemChk(HWND hwndDlgLst)
{
    BOOL bRemoved = TRUE;

    char strConfigPath[MAX_PATH];
    memset(strConfigPath,    0x00, sizeof(strConfigPath));
    sprintf(strConfigPath, "%s\\%s", gstrPluginPath, PLUGIN_MGR_CONFIG);

    char strBasicPlugins[MAX_BUF_SIZE];
    memset(strBasicPlugins, 0x00, sizeof(strBasicPlugins));

    int nMinCnt = GetPrivateProfileInt(APP_PLUGIN_MGR_BASIC_PLUGINS, KEY_PLUGIN_MGR_BP_MIN_CNT, MIN_PLUGIN_CNT_DEF, strConfigPath);
    if (MIN_PLUGIN_CNT_DEF == nMinCnt)
    {
        // if no plugin is basically required, just return.
        return bRemoved;
    }
    GetPrivateProfileString(APP_PLUGIN_MGR_BASIC_PLUGINS, KEY_PLUGIN_MGR_BP_NAME, EMPTY_STR, strBasicPlugins, sizeof(strBasicPlugins), strConfigPath);

    OTMGRPSTING grpStrBasicPlugins;
    SplitMultiPlugins(strBasicPlugins, &grpStrBasicPlugins);

    char strMsg[MAX_BUF_SIZE];
    memset(strMsg, 0x00, sizeof(strMsg));
    vector <int> lstnInx;
    for (int iInx = 0; iInx < ListView_GetItemCount(hwndDlgLst); iInx++)
    {
        // if selected, continued
        if (ListView_GetCheckState(hwndDlgLst, iInx))
        {
            continue;
        }

        // compare with unselected item
        char strPluginName[MAX_PATH];
        memset(strPluginName, 0x00, sizeof(strPluginName));
        ListView_GetItemText(hwndDlgLst, iInx, 0, strPluginName, MAX_PATH);

        for (int jInx = 0; jInx < (int) grpStrBasicPlugins.size(); jInx++)
        {
            // if total count already larger than minimum count, just break.
            if ((int) lstnInx.size() > nMinCnt)
            {
                break;
            }

            if (!stricmp(strPluginName, grpStrBasicPlugins[jInx].c_str()))
            {
                lstnInx.push_back(iInx);
            }
        }
    }

    // if the count of the plugins equals the minimum count, disable these items
    int nLeftCnt = (int) lstnInx.size();
    if (nLeftCnt < nMinCnt)
    {
        sprintf(strMsg, ERROR_REMOVE_CRITICAL_PLUGIN_STR, strBasicPlugins);
        MessageBox(hwndDlgLst, strMsg, PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
        bRemoved = FALSE;
    }

    return bRemoved;
}

void SplitMultiPlugins(char * strPlugins, POTMGRPSTING pGrpStrBP)
{
    if (NULL == strPlugins)
    {
        return;
    }

    char strPlugin[PLUGIN_NAME_MAX_LEN];

    int nPInx = 0;
    int nLen = (int) strlen(strPlugins);
    for (int iInx = 0; iInx < nLen; iInx++)
    {
        if (iInx == (nLen - 1))
        {
            strPlugin[nPInx] = strPlugins[iInx];
            nPInx++;
        }
        else if (',' != strPlugins[iInx])
        {
            strPlugin[nPInx] = strPlugins[iInx];
            nPInx++;
            continue;
        }

        strPlugin[nPInx] = '\0';
        pGrpStrBP->push_back(strPlugin);
        memset(strPlugin, 0x00, sizeof(strPlugin));
        nPInx = 0;
    }
}

int GetDepnsStr(PMAINTHREADINFO pMainTdInfo, char * strDepnsStr)
{
    int nRC = NO_ERROR;

    memset(strDepnsStr, 0x00, sizeof(strDepnsStr));
    if (pMainTdInfo->bFixpack)
    {
        return nRC;
    }

    int iInx = pMainTdInfo->iInx;
    for (int jInx = 0; jInx < gplginParse->GetPluginDepnsCnt(iInx); jInx++)
    {
        char * strTemp = new char [MAX_BUF_SIZE];
        memset(strTemp, 0x00, sizeof(strTemp));
        
        char * strDepnName = gplginParse->GetPluginDepnName(iInx, jInx);
        if ((NULL == strDepnName) || (strlen(strDepnName) == 0) || !stricmp(strDepnName, PLUGIN_DEF_NONE_STR))
        {
            break;
        }

        sprintf(strTemp, DEPN_STR, strDepnName, gplginParse->GetPluginDepnDL(iInx, jInx));
        if (0 != jInx)
        {
            sprintf(strDepnsStr, "%s\n%s", strDepnsStr, strTemp);
        }
        else
        {
            sprintf(strDepnsStr, "%s", strTemp);
        }
        delete strTemp;
    }

    return nRC;
}

void RefreshAllList(HWND hwndTabDlg, BOOL bShowMsg)
{
    PPLUGINMAGPROPIDA pIda;
    pIda = ACCESSDLGIDA(hwndTabDlg, PPLUGINMAGPROPIDA);

    // refresh three list
    SendMessage(pIda->hwndPages[0], WM_OTM_UPDATE, 1, 0);
    SendMessage(pIda->hwndPages[1], WM_OTM_UPDATE, 1, 0);
    SendMessage(pIda->hwndPages[2], WM_OTM_UPDATE, 1, 0);

    if (!bShowMsg)
    {
        return;
    }

    int nRC = HasNewVersion();
    if (VAL_HAS_NEW_VER == nRC)
    {
        MessageBox(hwndTabDlg, INFO_NEW_VERSION_FOUND_STR, PLUGIN_MGR_APP_NAME_STR, MB_OK);
    }
    else
    {
        MessageBox(hwndTabDlg, INFO_NO_NEW_FOUND_STR, PLUGIN_MGR_APP_NAME_STR, MB_OK);
    }
}
/*
void RefreshButtonSet(HWND hwndTabDlg)
{
    int nFrequency = GetPrivateProfileInt(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_FREQUENCY, PLUGIN_MGR_FREQUENCY_DFT, gstrConfigPath);
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
*/
void SetNextCheckTime()
{
    time_t timeNow;
    time(&timeNow);

    // get the local time
    struct tm * tmNow = localtime(&timeNow);

    int nDays = 0;

    int nFrequency = GetPrivateProfileInt(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_FREQUENCY, PLUGIN_MGR_FREQUENCY_DFT, gstrConfigPath);
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
    case Month:
        // check by manual
        //glogPlgMgr.writef("Manual, needn't set next check time.");
        //return;
		tmNow->tm_mon = (tmNow->tm_mon + 1)%12;
		if (tmNow->tm_mon == 0)
		{
			tmNow->tm_year++;
		}

		break;
    default:
        glogPlgMgr.writef("Default, needn't set next check time.");
        return;
    }

    // record to config file
    char strNextDate[MAX_BUF_SIZE];
    memset(strNextDate, 0x00, sizeof(strNextDate));
    sprintf(strNextDate, DATE_FORMAT, (tmNow->tm_year + 1900), (tmNow->tm_mon + 1), (tmNow->tm_mday + nDays));
    WritePrivateProfileString(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_DATE, strNextDate, gstrConfigPath);

    glogPlgMgr.writef("Set the next check time to: %s.", strNextDate);
}

void UpdatePendUptFiles(PMAINTHREADINFO pMainTdInfo, int nType)
{
    char strPendUptConf[MAX_PATH];
    char strPendUptLst[MAX_PATH];

    memset(strPendUptConf, 0x00, sizeof(strPendUptConf));
    memset(strPendUptLst,  0x00, sizeof(strPendUptLst));

    sprintf(strPendUptConf, "%s\\%s", gstrPluginPath, PENDING_UPT_CONF);
    sprintf(strPendUptLst,  "%s\\%s\\%s", gstrPluginPath, PENDING_DIR_STR, PLUGIN_PENDING_LST);

    // Record need reatart info to the config file
    WritePrivateProfileString(APP_SETTINGS_STR, KEY_NEED_RESTART, "1", strPendUptConf);

    // Record plugin name into update list
    FILE * filePendUptLst = fopen(strPendUptLst, "a+");
    if (!pMainTdInfo->bFixpack)
    {
        fprintf(filePendUptLst, "%s_%d_%s\n", pMainTdInfo->strName, nType, pMainTdInfo->strVer);
    }
    else
    {
        fprintf(filePendUptLst, "%s_%d_%s_%s\n", pMainTdInfo->strName, nType, pMainTdInfo->strVer, pMainTdInfo->strFixpkId);
    }
    fclose(filePendUptLst);
}

// Dowload, intall and update process control
unsigned int __stdcall CheckNewVersionProc(LPVOID lpParameter)
{
    int nRC = NO_ERROR;

    WritePrivateProfileString(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_AUTO, "1", gstrConfigPath);
    time_t timeCheck;
    timeCheck = (time_t) GetPrivateProfileInt(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_DATE, PLUGIN_MGR_DATE_DFT, gstrConfigPath);

    time_t timeNow;
    time(&timeNow);

    int nNow   = GetDateFromTime((int)timeNow);
    int nCheck = GetDateFromTime((int)timeCheck);

    glogPlgMgr.writef("Compare time now=%d with check=%d", nNow, nCheck);

    int nFrequency = GetPrivateProfileInt(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_FREQUENCY, PLUGIN_MGR_FREQUENCY_DFT, gstrConfigPath);
    //if ((nFrequency == Manual) || (nNow < nCheck))
	if ( nNow < nCheck )
    {
        glogPlgMgr.writef("Needn't start:%d", nRC);
        WritePrivateProfileString(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_AUTO, "0", gstrConfigPath);
        nRC = OtmParseXml(FALSE);

        if (nRC)
        {
            glogPlgMgr.writef("Parse XML with no download failed:%d", nRC);
            glogPlgMgr.close();
            return nRC;
        }
        return nRC;
    }
    else
    {
        // set next check time first
        SetNextCheckTime();
    }

    nRC = OtmParseXml(TRUE);
    if (nRC)
    {
        MessageBox(EqfQueryTwbClient(), OtmGetMessageFromCode(nRC), PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
        glogPlgMgr.writef("Parse XML with download failed:%d", nRC);
        WritePrivateProfileString(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_AUTO, "0", gstrConfigPath);
        glogPlgMgr.close();
        return nRC;
    }

    glogPlgMgr.writef("Parse successfully:%d", nRC);

    nRC = HasNewVersion();
    if (VAL_HAS_NEW_VER == nRC)
    {
        glogPlgMgr.writef("New vesrion detected.");
        // show main plugin manager GUI dialog
        HWND hwndMain = (HWND) lpParameter;
        int nRet = MessageBox(hwndMain, INFO_NEW_VERSION_STR, PLUGIN_MGR_APP_NAME_STR, MB_YESNO);
        if (IDYES == nRet)
        {
            INT_PTR sRC;
            HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            DIALOGBOX(hwndMain, PluginManagerDlgProc, hResMod, ID_PLUGIN_MANAGER_DLG, NULL, sRC);
            nRC = VAL_NO_NEED_OPEN;
        }
    }

    WritePrivateProfileString(APP_PLUGIN_MGR_SETTINGS, KEY_PLUGIN_MGR_AUTO, "0", gstrConfigPath);

    return nRC;
}

int HasNewVersion()
{
    int nRC = NO_ERROR;

    int nCnt = gplginParse->GetPluginsCnt();
    glogPlgMgr.writef("Plugin Info Xml Cnt %d.", nCnt);
    for (int iInx = 0; iInx < nCnt; iInx++)
    {
        char * strPluginName = gplginParse->GetPluginName(iInx);
        glogPlgMgr.writef("Plugin name %s.", strPluginName);
        int nRC = isNewPlugin(strPluginName);
        if (nRC == IS_NEW_PLUGIN)
        {
            nRC = VAL_HAS_NEW_VER;
            return nRC;
        }
        else if (nRC == PLUGIN_NOT_LOADED)
        {
            continue;
        }

        for (int jInx = 0; jInx < gplginParse->GetPluginMainCnt(iInx); jInx++)
        {
            char * strVerXml = gplginParse->GetMainVersion(iInx, jInx);
            char * strVerLoc = GetLocalPluginVer(strPluginName);
            glogPlgMgr.writef("Compare version %s, %s.", strVerLoc, strVerXml);
            // if null is local version, it menas the plugin is new
            int bNewVer = IsNewVersion(strVerLoc, strVerXml);
            if (bNewVer > VER_SAM_VAL)
            {
                nRC = VAL_HAS_NEW_VER;
                return nRC;
            }
            else if (VER_SAM_VAL == bNewVer)
            {
                // check whether there is fixpack uninstalled
                if (HasFixpackUninstalled(iInx, jInx))
                {
                    nRC = VAL_HAS_NEW_VER;
                    return nRC;
                }
            }
        }
    }

    return nRC;
}

BOOL HasUpdate(const char * strPluginLoc, const char * strVerLoc)
{
    BOOL bHasUpdate = FALSE;

    int nCnt = gplginParse->GetPluginsCnt();
    glogPlgMgr.writef("Check whether plugin has update %s.", strPluginLoc);
    for (int iInx = 0; iInx < nCnt; iInx++)
    {
        char * strPluginName = gplginParse->GetPluginName(iInx);
        if (!stricmp(strPluginName, strPluginLoc))
        {
            for (int jInx = 0; jInx < gplginParse->GetPluginMainCnt(iInx); jInx++)
            {
                int nCmp = IsNewVersion(strVerLoc, gplginParse->GetMainVersion(iInx, jInx));
                if (nCmp > VER_SAM_VAL)
                {
                    bHasUpdate = TRUE;
                    return bHasUpdate;
                }
                else if (nCmp == 0)
                {
                    glogPlgMgr.writef("Check whether plugin has fixpack need to be updated %s.", strPluginLoc);
                    if ((gplginParse->GetMainFixpacksCnt(iInx, jInx) > 0) &&
                        HasFixpackUninstalled(iInx, jInx))
                    {
                        bHasUpdate = TRUE;
                    }

                    return bHasUpdate;
                }
                else
                {
                    return bHasUpdate;
                }
            }
        }
    }

    return bHasUpdate;
}

void CheckNowRefresh(HWND hwndTabDlg)
{
    if (IsSettingChanged(hwndTabDlg))
    {
        int nID = MessageBox(hwndTabDlg, INFO_SETTING_CHANGED_STR, PLUGIN_MGR_APP_NAME_STR, MB_YESNO | MB_DEFBUTTON1);
        if (IDNO == nID)
        {
            return;
        }
        else
        {
            SaveInfoToConfig(hwndTabDlg, TRUE);
        }
    }

    int nRC;
    // set cursor to wait status
    HCURSOR hcursorHold, hcursorWait;
    hcursorWait = LoadCursor(NULL, IDC_WAIT);
    hcursorHold = ::SetCursor(hcursorWait);

    glogPlgMgr.writef("Start to reget xml file.");

    nRC = OtmParseXml(TRUE);
    if (nRC)
    {
        MessageBox(hwndTabDlg, OtmGetMessageFromCode(nRC), PLUGIN_MGR_APP_NAME_STR, MB_CANCEL | MB_ERROR);
        glogPlgMgr.writef("Refresh parse XML failed:%d", nRC);
        return;
    }

    glogPlgMgr.writef("Refresh parse successfully:%d", nRC);

    // update component list
    RefreshAllList(hwndTabDlg, TRUE);

    // recover the cursor
    hcursorWait = LoadCursor(NULL,IDC_ARROW);
    hcursorHold = ::SetCursor(hcursorWait);
}

BOOL IsRemovablePlugin(const char * strPluginName)
{
    BOOL bRemovable = TRUE;

    if ((NULL == strPluginName) && (strlen(strPluginName) == 0))
    {
        return bRemovable;
    }

    char strNonRemovePlugin[MAX_BUF_SIZE];
    memset(strNonRemovePlugin, 0x00, sizeof(strNonRemovePlugin));

    GetPrivateProfileString(APP_PLUGIN_MGR_NONREMOV_PLUGINS, KEY_PLUGIN_MGR_BP_NAME, EMPTY_STR, strNonRemovePlugin, sizeof(strNonRemovePlugin), gstrConfigPath);

    if ((NULL == strNonRemovePlugin) || (strlen(strNonRemovePlugin) == 0))
    {
        return bRemovable;
    }

    OTMGRPSTING grpStrNonRemovPlugins;
    SplitMultiPlugins(strNonRemovePlugin, &grpStrNonRemovPlugins);

    for (int iInx = 0; iInx < (int) grpStrNonRemovPlugins.size(); iInx++)
    {
        if (!stricmp(strPluginName, grpStrNonRemovPlugins[iInx].c_str()))
        {
            bRemovable = FALSE;
            break;
        }
    }

    return bRemovable;
}

void TestConnection(HWND hwndTabDlg)
{
    if (IsSettingChanged(hwndTabDlg))
    {
        int nID = MessageBox(hwndTabDlg, INFO_SETTING_CHANGED_STR, PLUGIN_MGR_APP_NAME_STR, MB_YESNO | MB_DEFBUTTON1);
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

    int nRC = gplginParse->TestConnection(gstrConfigPath);
    if (!nRC)
    {
        MessageBox(hwndTabDlg, INFO_CONNECT_SUCCESS_STR, PLUGIN_MGR_APP_NAME_STR, MB_OK);
    }
    else
    {
        MessageBox(hwndTabDlg, OtmGetMessageFromCode(nRC), PLUGIN_MGR_APP_NAME_STR, MB_ICONSTOP | MB_OK);
    }

    // recover the cursor
    hcursorWait = LoadCursor(NULL, IDC_ARROW);
    hcursorHold = ::SetCursor(hcursorWait);
}

BOOL HasFixpackUninstalled(int iInx, int jInx)
{
    BOOL bUninstall = FALSE;
    for (int kInx = 0; kInx < gplginParse->GetMainFixpacksCnt(iInx, jInx); kInx++)
    {
        char strName[MAX_BUF_SIZE];
        memset(strName, 0x00, sizeof(strName));
        sprintf(strName, "%s_%s_%s_%s", KEY_FIXPACK, gplginParse->GetPluginName(iInx), 
                gplginParse->GetMainVersion(iInx, jInx), gplginParse->GetMainFixpackId(iInx, jInx, kInx));

        glogPlgMgr.writef("Check whether the fixpack %s has installed.", strName);
        if (!IsFixpackInstalled(strName))
        {
            bUninstall = TRUE;
            break;
        }
    }

    return bUninstall;
}

void InitialOtmListViewCtrl(HWND hwndListView)
{
    HIMAGELIST hImgListSm;
    HICON hicon;
    HINSTANCE hInst = GETINSTANCE(hwndListView);

    hImgListSm = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK, 1, 0);

    hicon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_CB_NO));
    ImageList_AddIcon(hImgListSm, hicon);

    hicon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_CB_YES));
    ImageList_AddIcon(hImgListSm, hicon);

    hicon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_CB_GREY));
    ImageList_AddIcon(hImgListSm, hicon);

    ListView_SetImageList(hwndListView, hImgListSm, LVSIL_SMALL);
}

void SetListViewCtrlChkState(HWND hwndDlg, int nType)
{
    HWND plginMgrList = GetDlgItem(hwndDlg, IDC_PLUGINMGR_LIST);
    int iSelInx = -1;

    while((iSelInx = ListView_GetNextItem(plginMgrList, iSelInx, LVNI_SELECTED)) != -1)
    {
        if (TYPE_INSTALLED == nType)
        {
            LV_ITEM lvItem;
            lvItem.iItem = iSelInx;
            lvItem.iSubItem = 0;
            lvItem.mask = LVIF_IMAGE;
            ListView_GetItem(plginMgrList, &lvItem);
            if (lvItem.iImage == STATE_CB_YES)
            {
                lvItem.iImage = STATE_CB_NO;
            }
            else if (lvItem.iImage == STATE_CB_NO)
            {
                lvItem.iImage = STATE_CB_YES;
            }
            ListView_SetItem(plginMgrList, &lvItem);
        }
        else
        {
            // set the check state when select the item
            UINT nState = ListView_GetCheckState(plginMgrList, iSelInx);
            ListView_SetCheckState(plginMgrList, iSelInx, !nState);
        }
    }
}

int Otm_ListView_GetCheckState(HWND hwndListView, int nInx)
{
    LV_ITEM lvItem;
    lvItem.iItem = nInx;
    lvItem.iSubItem = 0;
    lvItem.mask = LVIF_IMAGE;
    ListView_GetItem(hwndListView, &lvItem);
    return lvItem.iImage;
}

int GetPluginInfoForLst(HWND hwndDlgLst, int nType, int nLstInx, PMAINTHREADINFO pMainTdInfo)
{
    int nRC = NO_ERROR;

    char strLstItemName[PLUGIN_NAME_MAX_LEN];
    memset(strLstItemName, 0x00, sizeof(strLstItemName));
    ListView_GetItemText(hwndDlgLst, nLstInx, 0, strLstItemName, sizeof(strLstItemName));

    if (TYPE_UPDATE == nType)
    {
        // if it is update, get the version info from 4th column
        ListView_GetItemText(hwndDlgLst, nLstInx, 3, pMainTdInfo->strVer, sizeof(pMainTdInfo->strVer));
    }
    else
    {
        // if it is update, get the version info from 3rd column
        ListView_GetItemText(hwndDlgLst, nLstInx, 2, pMainTdInfo->strVer, sizeof(pMainTdInfo->strVer));
    }

    if (!IsFixpack(strLstItemName))
    {
        pMainTdInfo->bFixpack = FALSE;
        strcpy(pMainTdInfo->strName, strLstItemName);
    }
    else
    {
        pMainTdInfo->bFixpack = TRUE;
        SplitFixpackName(strLstItemName, pMainTdInfo->strName, pMainTdInfo->strVer, pMainTdInfo->strFixpkId);
    }

    nRC = gplginParse->GetPluginPos(pMainTdInfo);

    return nRC;
}

void GetXmlDownloadInfo(PMAINTHREADINFO pMainTdInfo, PDOWNLOADPARAM pDLParam)
{
    int iInx = pMainTdInfo->iInx;
    int jInx = pMainTdInfo->jInx;

    if (!pMainTdInfo->bFixpack)
    {
        if (NULL != pDLParam->strDLUrl)
        {
            strcpy(pDLParam->strDLUrl, gplginParse->GetMainDLUrl(iInx, jInx));
        }

        if (NULL != pDLParam->strDLType)
        {
            strcpy(pDLParam->strDLType, gplginParse->GetMainDLType(iInx, jInx));
        }

        if (NULL != pDLParam->strMethod)
        {
            strcpy(pDLParam->strMethod, gplginParse->GetMainMethod(iInx, jInx));
        }

        pDLParam->bRestart = gplginParse->GetMainNeedRestart(iInx, jInx);

        pDLParam->bNeedWait = gplginParse->GetMainNeedWait(iInx, jInx);
    }
    else
    {
        int kInx = pMainTdInfo->kInx;

        if (NULL != pDLParam->strDLUrl)
        {
            strcpy(pDLParam->strDLUrl, gplginParse->GetMainFixpackDLUrl(iInx, jInx, kInx));
        }

        if (NULL != pDLParam->strDLType)
        {
            strcpy(pDLParam->strDLType, gplginParse->GetMainFixpackDLType(iInx, jInx, kInx));
        }

        if (NULL != pDLParam->strMethod)
        {
            strcpy(pDLParam->strMethod, gplginParse->GetMainFixpackMethod(iInx, jInx, kInx));
        }

        pDLParam->bRestart = gplginParse->GetMainFixpackNeedRestart(iInx, jInx, kInx);
    }
}

void GetDefPathCopies(PCOTMCOPIES pDefPathCopies)
{
    COTMCOPY defPathCopy;
    InitOtmCopy(&defPathCopy);

    strcpy(defPathCopy.strFrom,      PLUGIN_DEF_PATH_KEY);
    strcpy(defPathCopy.strTo,        gstrPluginPath);

    pDefPathCopies->push_back(defPathCopy);

    InitOtmCopy(&defPathCopy);
    strcpy(defPathCopy.strFrom,      OPENTM2_DEF_PATH_KEY);
    strcpy(defPathCopy.strTo,        gstrAppPath);

    pDefPathCopies->push_back(defPathCopy);

    char strDLPath[MAX_PATH];
    memset(strDLPath, 0x00, sizeof(strDLPath));
    sprintf(strDLPath, "%s\\%s", gstrAppPath, PENDING_DIR_STR);

    InitOtmCopy(&defPathCopy);
    strcpy(defPathCopy.strFrom,      OPENTM2_DEF_DL_PATH_KEY);
    strcpy(defPathCopy.strTo,        strDLPath);

    pDefPathCopies->push_back(defPathCopy);
}

BOOL IfPluginExists(const char * strStartDir, const char * strPluginDllName, const char * strPluginName)
{
    BOOL bExist = FALSE;

    WIN32_FIND_DATA otmFindData;
    ZeroMemory(&otmFindData, sizeof(WIN32_FIND_DATA));

    HANDLE hFindHandle;
    BOOL bFinished = FALSE;

    char strSearchPath[MAX_PATH];
    char strTmpTgtDir[MAX_PATH];
    memset(strSearchPath, 0x00, sizeof(strSearchPath));
    memset(strTmpTgtDir,  0x00, sizeof(strTmpTgtDir));

    strcpy(strSearchPath, strStartDir);
    if (strSearchPath[strlen(strSearchPath) - 1] != '\\')
    {
       strcat(strSearchPath, "\\");
    }
    strcpy(strTmpTgtDir, strSearchPath);
    strcat(strSearchPath, "*");

    hFindHandle = FindFirstFile(strSearchPath, &otmFindData);
    if (NULL == hFindHandle)
    {
        bFinished = TRUE;
        return bExist;
    }

    while (!bFinished)
    {
        if (otmFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (otmFindData.cFileName[0] != '.')
            {
                char strNewStartDir[MAX_PATH];
                memset(strNewStartDir, 0x00, sizeof(strNewStartDir));
                sprintf(strNewStartDir, "%s%s", strTmpTgtDir, otmFindData.cFileName);
                bExist = IfPluginExists(strNewStartDir, strPluginDllName, strPluginName);
                if (bExist)
                {
                    bFinished = TRUE;
                    break;
                }
            }
        }
        else
        {
            if (!stricmp(otmFindData.cFileName, strPluginDllName))
            {
                bExist = TRUE;
                bFinished = TRUE;
                break;
            }
        }

        if (!FindNextFile(hFindHandle, &otmFindData))
        {
            bFinished = TRUE;
        }
    }

    FindClose(hFindHandle);

    return bExist;
}
