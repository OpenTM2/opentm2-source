//+----------------------------------------------------------------------------+
//|OpenTM2StarterApp.cpp     OTM Auto Version Up function                      |
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
//|                    during start OpenTM2                                    |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "OpenTM2Starter.h"

int OpenTM2StarterProps(HINSTANCE hInstance, char * szOtmPath, LPTSTR lpCmdLine)
{
    int nRC = NO_ERROR;

    // initial for progress bar
    INITCOMMONCONTROLSEX otmIcc = {sizeof(otmIcc), ICC_PROGRESS_CLASS};
    InitCommonControlsEx(&otmIcc);

    ghInstance = hInstance;
    // check parameter
    if ((NULL == szOtmPath) || (0 == strlen(szOtmPath)))
    {
        nRC = ERROR_WRONG_PARAM_A;
        return nRC;
    }

    int nLen = strlen(szOtmPath);
    if (szOtmPath[nLen-1] == '\\')
    {
        szOtmPath[nLen-1] = EOS;
    }

    memset(gstrOtmPath,     0x00, sizeof(gstrOtmPath));
    memset(gstrOpenTM2Path, 0x00, sizeof(gstrOpenTM2Path));
    memset(gstrPendUptConf, 0x00, sizeof(gstrPendUptConf));
    strncpy(gstrOtmPath, szOtmPath, strlen(szOtmPath));
    sprintf(gstrOpenTM2Path, "%s\\%s\\%s", szOtmPath, OPENTM2_FOLDER_STR, APP_OPENTM2_EXE_STR);
    sprintf(gstrPendUptConf, "%s\\%s\\%s", szOtmPath, OPENTM2_PLUGIN_FOLDER_STR, PENDING_UPT_CONF);

    // judge whether the conf file exists or not
    if (OTM_NOT_FOUND == access(gstrPendUptConf, 0))
    {
        // if not exist, find sample file and rename to the config file
        char strConfigSampleFile[MAX_PATH];
        memset(strConfigSampleFile, 0x00, sizeof(strConfigSampleFile));
        sprintf(strConfigSampleFile, "%s\\%s\\%s", szOtmPath, OPENTM2_PLUGIN_FOLDER_STR, PENDING_UPT_CONF_SAMPLE);
        if (OTM_NOT_FOUND == access(strConfigSampleFile, 0))
        {
            // sample file not found, just start OpenTM2 and exist
            nRC = ERROR_FILE_NOT_FOUND;
        }
        else
        {
            rename(strConfigSampleFile, gstrPendUptConf);
        }
    }

    // Get the max waiting time and max waiting times from config file
    int nMaxWaitTime = GetPrivateProfileInt(APP_SETTINGS_STR, KEY_MAX_WAIT_TIME, DEF_WAITING_TIME, gstrPendUptConf);
    int nMaxWaitTimes = GetPrivateProfileInt(APP_SETTINGS_STR, KEY_MAX_WAIT_TIMES, DEF_WAITING_TIMES, gstrPendUptConf);

    // check whether OpenTM2 is still running, if is, wait for it end
    int nTimes = 1;
    while (IsProgramRunning(APP_OPENTM2_EXE_STR))
    {
        if (nTimes > nMaxWaitTimes)
        {
            nRC = ERROR_CANNOT_START_A;
            break;
        }

        Sleep(nMaxWaitTime);

        nTimes++;
    }

    if (nRC == ERROR_CANNOT_START_A)
    {
//        MessageBox(HWND_DESKTOP, OtmGetMessageFromCode(nRC), APP_OPENTM2STARTER_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        return nRC;
    }

    gPlginPendLst = new CPlginPendLst(szOtmPath);
    gAvuPendLst   = new CAvuPendLst(szOtmPath);

    if ((NO_PENDING == gPlginPendLst->ParseConfigFile()) &&
        (NO_PENDING == gAvuPendLst->ParseConfigFile()))
    {
        // if there is no pending, just start OpenTM2
        nRC = OtmExecuteCommand(gstrOpenTM2Path, NULL, FALSE);
        return nRC;
    }

    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_STARTER_PG_CTRL_DLG), NULL, PendingPCDlgProc, NULL);

    return nRC;
}

INT_PTR CALLBACK PendingPCDlgProc
(
HWND hwndPrgCtrlDlg,
UINT msg,
WPARAM wParam,
LPARAM lParam
)
{
    HWND hwndPB;
    int nCnt;
    LRESULT mResult = NO_ERROR;

    switch (msg)
    {
    case WM_INITDIALOG:
        {
            SendMessage(hwndPrgCtrlDlg, WM_SETICON,ICON_SMALL, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_OPENTM2STARTER)));
            SendMessage(hwndPrgCtrlDlg, WM_SETICON,ICON_BIG, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_OPENTM2STARTER)));

            OtmCenterWindow(hwndPrgCtrlDlg);
            SetDlgItemText(hwndPrgCtrlDlg, IDC_STATIC_STATUS, EMPTY_STR);

            // Initialize count progress bar
            hwndPB = GetDlgItem(hwndPrgCtrlDlg, IDC_PROGRESS_BAR);
            // set the total count
            nCnt = gAvuPendLst->GetPendingCnt() * 2;
            if (0 == nCnt)
            {
                nCnt = gPlginPendLst->GetPendingCnt() * 2;
            }
            else
            {
                // if there is update for auto version upgrade, remove the update for plugin manager
            }
            SendMessage(hwndPB, PBM_SETRANGE, (WPARAM)0, (LPARAM)(MAKELPARAM(0, nCnt)));
            SendMessage(hwndPB, PBM_SETSTEP, 1, 0);
            SendMessage(hwndPB, PBM_STEPIT, 0, 0);
            SendMessage(hwndPB, PBM_SETPOS, (WPARAM)0, (LPARAM)0);

            PPENDINGPARAM pPendingParam = new PENDINGPARAM;
            pPendingParam->hwndDlg = hwndPrgCtrlDlg;
            // pDownLoadParam2->hwndPCDlg  = ghwndPrgCtrlDlg;
            if (NULL == _beginthreadex(NULL, 0, PendingPCThreadProc, pPendingParam, 0, 0))
            {
                mResult = ERROR_CREATE_THREAD_A;
            }
        }
        break;

    case WM_COMMAND:
        break;

    case WM_CLOSE:
        DestroyWindow(hwndPrgCtrlDlg);
        break;

    default:
        break;
    } /* endswitch */

    return mResult;
}

void OtmCenterWindow(HWND hWnd)
{
    HWND hParentOrOwner;
    RECT rcParent, rcWnd;
    int x, y;
    if ((hParentOrOwner = GetParent(hWnd)) == NULL)
    {
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcParent, 0);
    }
    else
    {
        GetWindowRect(hParentOrOwner, &rcParent);
    }
    GetWindowRect(hWnd, &rcWnd);
    x = ((rcParent.right-rcParent.left) - (rcWnd.right-rcWnd.left)) / 2 + rcParent.left;
    y = ((rcParent.bottom-rcParent.top) - (rcWnd.bottom-rcWnd.top)) / 2 + rcParent.top;
    SetWindowPos(hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

unsigned int __stdcall PendingPCThreadProc(LPVOID lpParameter)
{
    int nRC = NO_ERROR;

    PPENDINGPARAM pPendingParam = (PPENDINGPARAM) lpParameter;
    HWND hwndPB = GetDlgItem(pPendingParam->hwndDlg, IDC_PROGRESS_BAR);

    BOOL bAutoVerUpt = FALSE;
    pPendingParam->bInstalled = FALSE;

    if (0 != gAvuPendLst->GetPendingCnt())
    {
        // do the update for auto version upgrade
        bAutoVerUpt = TRUE;
        nRC = AutoVerUpPendingProc(lpParameter);
    }
    else
    {
        // do the update for plugin manager
        nRC = PluginMgrPendingProc(lpParameter);
    }

    // update pending updates plugin's config file
    WritePrivateProfileString(APP_SETTINGS_STR, KEY_NEED_RESTART, "0", gstrPendUptConf);

    // remove pending update folder if it is empty
    char strLstPath[MAX_PATH];
    memset(strLstPath, 0x00, sizeof(strLstPath));

    if (bAutoVerUpt)
    {
        sprintf(strLstPath, "%s\\%s", gstrOtmPath, OPENTM2_PENDING_FOLDER_STR);
    }
    else
    {
        sprintf(strLstPath, "%s\\%s\\%s", gstrOtmPath, OPENTM2_PLUGIN_FOLDER_STR, OPENTM2_PENDING_FOLDER_STR);
    }
    // only the folder of downloads is empty, the delete will be successful
    RemoveDirectory(strLstPath);

    // if success and has fixpack, show complete dialog
    if (!nRC && !pPendingParam->bInstalled)
    {
        MessageBox(pPendingParam->hwndDlg, INFO_UPDATE_COMPLETE_STR, APP_OPENTM2STARTER_NAME_STR, MB_OK);
    }

    // always start OpenTM2
    nRC = OtmExecuteCommand(gstrOpenTM2Path, NULL, FALSE);

    SendMessage(pPendingParam->hwndDlg, WM_CLOSE, 0, 0);
    return nRC;
}

int PluginMgrPendingProc(LPVOID lpParameter)
{
    int nRC = NO_ERROR;

    char strMsg[MAX_BUF_SIZE];
    memset(strMsg, 0x00, sizeof(strMsg));

    PPENDINGPARAM pPendingParam = (PPENDINGPARAM) lpParameter;

    for (int iInx = 0; iInx < gPlginPendLst->GetPendingCnt(); iInx++)
    {
        SetDlgItemText(pPendingParam->hwndDlg, IDC_STATIC_STATUS, gPlginPendLst->GetPendingName(iInx));
        // pending update start
        SendDlgItemMessage(pPendingParam->hwndDlg, IDC_PROGRESS_BAR, PBM_SETPOS, (WPARAM) (iInx+1)*2-1, 0);

        MAINTHREADINFO mainTdInfo;
        InitMainThreadInfo(&mainTdInfo);

        nRC = PendingUpdatesPlugin(gPlginPendLst->GetPendingName(iInx), &mainTdInfo);

        if (mainTdInfo.bInstalled)
        {
            pPendingParam->bInstalled = TRUE;
        }

        const char * strType;
        if (TYPE_NEW_INSTALL == mainTdInfo.nType)
        {
            strType = TYPE_NEW_INSTALL_STR;
        }
        else
        {
            strType = TYPE_UPDATE_STR;
        }

        if (!mainTdInfo.bFixpack)
        {
            gHistoryWriter.writef("PluginManangerHistory", "%s\t%s\t%s\t%s\t%d", TimeManager::GetDateTimeStr(), strType, 
                                  mainTdInfo.strVer, mainTdInfo.strName, nRC);
        }
        else
        {
            gHistoryWriter.writef("PluginManangerHistory", "%s\t%s\t%s\t%s\t%s\t%d", TimeManager::GetDateTimeStr(), strType, 
                                  mainTdInfo.strVer, mainTdInfo.strName, mainTdInfo.strFixpkId, nRC);
        }

        if (nRC)
        {
            sprintf(strMsg, ERROR_PENDING_FAILED_C_STR, gPlginPendLst->GetPendingName(iInx), nRC);
            MessageBox(pPendingParam->hwndDlg, strMsg, APP_OPENTM2STARTER_NAME_STR, MB_OK|MB_ICONEXCLAMATION);
            continue;
        }

        // pending update end
        SendDlgItemMessage(pPendingParam->hwndDlg, IDC_PROGRESS_BAR, PBM_SETPOS, (WPARAM) (iInx+1)*2, 0);
        gPlginPendLst->SetPendingResult(iInx, nRC);
    }

    nRC = gPlginPendLst->RefreshPendingLst();
    if (nRC)
    {
        sprintf(strMsg, ERROR_PD_LIST_UPDATE_FAILD_B_STR, nRC);
        MessageBox(pPendingParam->hwndDlg, strMsg, APP_OPENTM2STARTER_NAME_STR, MB_OK|MB_ICONEXCLAMATION);
    }

    return nRC;
}

int AutoVerUpPendingProc(LPVOID lpParameter)
{
    int nRC = NO_ERROR;

    char strMsg[MAX_BUF_SIZE];
    memset(strMsg, 0x00, sizeof(strMsg));

    PPENDINGPARAM pPendingParam = (PPENDINGPARAM) lpParameter;

    for (int iInx = 0; iInx < gAvuPendLst->GetPendingCnt(); iInx++)
    {
        SetDlgItemText(pPendingParam->hwndDlg, IDC_STATIC_STATUS, gAvuPendLst->GetPendingName(iInx));
        // pending update start
        SendDlgItemMessage(pPendingParam->hwndDlg, IDC_PROGRESS_BAR, PBM_SETPOS, (WPARAM) (iInx+1)*2-1, 0);

        MAINTHREADINFO mainTdInfo;
        InitMainThreadInfo(&mainTdInfo);

        nRC = PendingUpdatesAVU(gAvuPendLst->GetPendingName(iInx), &mainTdInfo);

        if (mainTdInfo.bInstalled)
        {
            pPendingParam->bInstalled = TRUE;
        }

        if (nRC)
        {
            sprintf(strMsg, ERROR_PENDING_FAILED_C_STR, gAvuPendLst->GetPendingName(iInx), nRC);
            MessageBox(pPendingParam->hwndDlg, strMsg, APP_OPENTM2STARTER_NAME_STR, MB_OK|MB_ICONEXCLAMATION);
            continue;
        }

        // pending update end
        SendDlgItemMessage(pPendingParam->hwndDlg, IDC_PROGRESS_BAR, PBM_SETPOS, (WPARAM) (iInx+1)*2, 0);
        gAvuPendLst->SetPendingResult(iInx, nRC);
    }

    nRC = gAvuPendLst->RefreshPendingLst();
    if (nRC)
    {
        sprintf(strMsg, ERROR_PD_LIST_UPDATE_FAILD_B_STR, nRC);
        MessageBox(pPendingParam->hwndDlg, strMsg, APP_OPENTM2STARTER_NAME_STR, MB_OK|MB_ICONEXCLAMATION);
    }

    return nRC;
}

int PendingUpdatesPlugin(const char * strPendingName, PMAINTHREADINFO pMainTdInfo)
{
    int nRC = NO_ERROR;

    if ((NULL == strPendingName) || (strlen(strPendingName) == 0))
    {
        nRC = ERROR_WRONG_PARAM_A;
        return nRC;
    }

    char strXmlPath[MAX_PATH];
    memset(strXmlPath, 0x00, sizeof(strXmlPath));
    sprintf(strXmlPath, "%s\\%s\\%s", gstrOtmPath, OPENTM2_PLUGIN_FOLDER_STR, PLUGIN_MGR_LOC_XML);

    CPlgMgXmlLocParser * pPlgMgXmlLocParser = new CPlgMgXmlLocParser();
    nRC = pPlgMgXmlLocParser->XmlParser(strXmlPath);
    if (nRC)
    {
        return nRC;
    }

    const char * strUrl     = NULL;
    const char * strDLType  = NULL;
    const char * strMethod  = NULL;

#ifdef _DEBUG
    glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "start to find the pos %s", strPendingName);
#endif
    nRC = FindPluginPos(strPendingName, pPlgMgXmlLocParser, pMainTdInfo);
#ifdef _DEBUG
    glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "find the pos end(%d)", nRC);
#endif

    if (nRC)
    {
        return nRC;
    }

    int iInx = pMainTdInfo->iInx;
    int jInx = pMainTdInfo->jInx;

    if (!pMainTdInfo->bFixpack)
    {
        strUrl = pPlgMgXmlLocParser->GetMainDLUrl(iInx, jInx);
        strDLType = pPlgMgXmlLocParser->GetMainDLType(iInx, jInx);
        strMethod = pPlgMgXmlLocParser->GetMainMethod(iInx, jInx);
    }
    else
    {
        int kInx = pMainTdInfo->kInx;

        strUrl = pPlgMgXmlLocParser->GetMainFixpackDLUrl(iInx, jInx, kInx);
        strDLType = pPlgMgXmlLocParser->GetMainFixpackDLType(iInx, jInx, kInx);
        strMethod = pPlgMgXmlLocParser->GetMainFixpackMethod(iInx, jInx, kInx);
    }

    char strPkgName[MAX_PATH];
    memset(strPkgName, 0x00, sizeof(strPkgName));
    GetFileNameFromUrl(strUrl, strPkgName);
    if ((NULL == strPkgName) || (strlen(strPkgName) == 0))
    {
        nRC = ERROR_GET_FILE_NAME_A;
        return nRC;
    }

    char strPkgPath[MAX_PATH];
    memset(strPkgPath, 0x00, sizeof(strPkgPath));
    sprintf(strPkgPath, "%s\\%s\\%s\\%s", gstrOtmPath, OPENTM2_PLUGIN_FOLDER_STR, OPENTM2_PENDING_FOLDER_STR, strPkgName);
#ifdef _DEBUG
    glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "find the package %s", strPkgPath);
#endif
    OTMGRPSTING strUnzipFiles;
    strUnzipFiles.clear();

    // set check status for check box
    char strConfigPath[MAX_PATH];
    memset(strConfigPath,    0x00, sizeof(strConfigPath));
    sprintf(strConfigPath, "%s\\%s\\%s", gstrOtmPath, OPENTM2_PLUGIN_FOLDER_STR, PLUGIN_MGR_CONFIG);

    int nKeepPkg = GetPrivateProfileInt(APP_SETTINGS_STR, KEY_KEEP_PKG, DFT_KEEP_PKG, strConfigPath);

    if (!stricmp(strDLType, DLTYPE_ZIP))
    {
        char strDestPath[MAX_PATH];
        memset(strDestPath, 0x00, sizeof(strDestPath));

        // unzip the package
        if (!stricmp(strMethod, METHOD_INSTALL))
        {
            pMainTdInfo->bInstalled = TRUE;

            // zip method
            sprintf(strDestPath, "%s\\%s\\%s", gstrOtmPath, OPENTM2_PLUGIN_FOLDER_STR, OPENTM2_PENDING_FOLDER_STR);
#ifdef _DEBUG
            glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "unzip the package %s start", strPkgPath);
#endif
            nRC = OtmUnCompress(strPkgPath, NULL, NULL, strDestPath, &strUnzipFiles);
#ifdef _DEBUG
            glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "unzip the package end(%d)", nRC);
#endif

            // check whether is installer, if is, do the installation
            for (size_t iInx = 0; iInx < strUnzipFiles.size(); iInx++)
            {
                // install the package
#ifdef _DEBUG
                glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "execute command (%s)", strUnzipFiles[iInx].c_str());
#endif
                nRC = OtmExecuteCommand((char *)strUnzipFiles[iInx].c_str(), NULL, TRUE);
                // if success, delete the unzip file
                if (!nRC)
                {
                    remove(strUnzipFiles[iInx].c_str());
                }
            }
        }
        else
        {
            // copy method
            COTMCOPIES pluginCopies;
            nRC = pPlgMgXmlLocParser->GetPluginCopies(pMainTdInfo, &pluginCopies);

            if (!nRC)
            {
                sprintf(strDestPath, "%s\\%s", gstrOtmPath, OPENTM2_PLUGIN_FOLDER_STR);
                COTMCOPIES defPathCopies;
                GetDefPathCopies(&defPathCopies);
                nRC = OtmUnCompress(strPkgPath, &defPathCopies, &pluginCopies, strDestPath, &strUnzipFiles);
                // Add log info
                for (size_t iInx = 0; iInx < strUnzipFiles.size(); iInx++)
                {
#ifdef _DEBUG
                    glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "unzip file (%s)", strUnzipFiles[iInx].c_str());
#endif
                }
            }
        }
    }
    else
    {
        strUnzipFiles.push_back(strPkgPath);
    }

    if (!nRC && pMainTdInfo->bFixpack)
    {
#ifdef _DEBUG
        glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "For %s installed successfully, set install state.", strPendingName);
#endif
        SetFixpackState(pMainTdInfo, STATE_INSTALLED, FALSE);
    }

    if (!nKeepPkg)
    {
        remove(strPkgPath);
    }

    return nRC;
}

int PendingUpdatesAVU(const char * strPendingName, PMAINTHREADINFO pMainTdInfo)
{
    int nRC = NO_ERROR;

    if ((NULL == strPendingName) || (strlen(strPendingName) == 0))
    {
        nRC = ERROR_WRONG_PARAM_A;
        return nRC;
    }

    char strXmlPath[MAX_PATH];
    memset(strXmlPath, 0x00, sizeof(strXmlPath));
    sprintf(strXmlPath, "%s\\%s\\%s", gstrOtmPath, OPENTM2_PLUGIN_FOLDER_STR, AUTO_VER_UP_LOC_XML);

    COtmXmlLocParser * pOtmXmlLocParser = new COtmXmlLocParser();
    nRC = pOtmXmlLocParser->XmlParser(strXmlPath);
    if (nRC)
    {
        return nRC;
    }

    const char * strUrl     = NULL;
    const char * strDLType  = NULL;
    const char * strMethod  = NULL;

#ifdef _DEBUG
    glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "start to find the pos %s", strPendingName);
#endif
    nRC = FindAutoVerUpPos(strPendingName, pOtmXmlLocParser, pMainTdInfo);
#ifdef _DEBUG
    glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "find the pos end(%d)", nRC);
#endif

    if (nRC)
    {
        return nRC;
    }

    int iInx = pMainTdInfo->iInx;
    int jInx = pMainTdInfo->jInx;

    if (!pMainTdInfo->bFixpack)
    {
        strUrl    = pOtmXmlLocParser->GetComponentDLUrl(iInx);
        strDLType = pOtmXmlLocParser->GetComponentDLType(iInx);
        strMethod = pOtmXmlLocParser->GetComponentMethod(iInx);
    }
    else
    {
        strUrl    = pOtmXmlLocParser->GetCompFixpackDLUrl(iInx, jInx);
        strDLType = pOtmXmlLocParser->GetCompFixpackDLType(iInx, jInx);
        strMethod = pOtmXmlLocParser->GetCompFixpackMethod(iInx, jInx);
    }

    char strPkgName[MAX_PATH];
    memset(strPkgName, 0x00, sizeof(strPkgName));
    GetFileNameFromUrl(strUrl, strPkgName);
    if ((NULL == strPkgName) || (strlen(strPkgName) == 0))
    {
        nRC = ERROR_GET_FILE_NAME_A;
        return nRC;
    }

    char strPkgPath[MAX_PATH];
    memset(strPkgPath, 0x00, sizeof(strPkgPath));
    sprintf(strPkgPath, "%s\\%s\\%s", gstrOtmPath, OPENTM2_PENDING_FOLDER_STR, strPkgName);
#ifdef _DEBUG
    glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "find the package %s", strPkgPath);
#endif

    // set check status for check box
    char strConfigPath[MAX_PATH];
    memset(strConfigPath,    0x00, sizeof(strConfigPath));
    sprintf(strConfigPath, "%s\\%s\\%s", gstrOtmPath, OPENTM2_PLUGIN_FOLDER_STR, AUTO_VER_UP_CONFIG);

    int nKeepPkg = GetPrivateProfileInt(APP_SETTINGS_STR, KEY_KEEP_PKG, DFT_KEEP_PKG, strConfigPath);

    if (!stricmp(strDLType, DLTYPE_ZIP))
    {
        COTMCOPIES compCopyies;
        nRC = pOtmXmlLocParser->GetCompCopiesByName(strPendingName, pMainTdInfo->strVer, &compCopyies);

        // unzip the package
        OTMGRPSTING strUnzipFiles;
        strUnzipFiles.clear();

        COTMCOPIES defPathCopyies;
        GetDefPathCopies(&defPathCopyies);
#ifdef _DEBUG
        glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "unzip the package %s start", strPkgPath);
#endif
        // unzip the package
        nRC = OtmUnCompress(strPkgPath, &defPathCopyies, &compCopyies, gstrOtmPath, &strUnzipFiles);
#ifdef _DEBUG
        glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "unzip the package end(%d)", nRC);
#endif

        if (!nRC && (!stricmp(METHOD_INSTALL, strMethod) || !stricmp(METHOD_OPEN, strMethod)))
        {
            pMainTdInfo->bInstalled = TRUE;
            // if successfully get copies info, unzip the package
            // check whether is installer, if is, do the installation
            for (int iInx = 0; iInx < (int) strUnzipFiles.size(); iInx++)
            {
                // install the package
                nRC = OtmExecuteCommand((char *)strUnzipFiles[iInx].c_str(), NULL, TRUE);
                // if success, delete the unzip file
                if (!nRC)
                {
                    remove(strUnzipFiles[iInx].c_str());
                }
            }
        }
    }
    else
    {
        // if not zip mode
        nRC = ERROR_DL_NOT_ZIP_A;
    }

    if (!nRC && pMainTdInfo->bFixpack)
    {
#ifdef _DEBUG
        glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "For %s installed successfully, set install state.", strPendingName);
#endif
        SetFixpackState(pMainTdInfo, STATE_INSTALLED, FALSE);
    }

    if (!nKeepPkg)
    {
        // whether success or not, always remove the package
        remove(strPkgPath);
    }

    return nRC;
}

int FindPluginPos(const char * strPendingName, CPlgMgXmlLocParser * pPlgMgXmlLocParser, PMAINTHREADINFO pMainTdInfo)
{
    int nRC = NO_ERROR;

#ifdef _DEBUG
    glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "split plugin name %s", strPendingName);
#endif
    SplitPluginName(strPendingName, pMainTdInfo);

    nRC = pPlgMgXmlLocParser->GetPluginPos(pMainTdInfo);
#ifdef _DEBUG
    glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "get position %d", nRC);
#endif

    return nRC;
}

int FindAutoVerUpPos(const char * strPendingName, COtmXmlLocParser * pOtmXmlLocParser, PMAINTHREADINFO pMainTdInfo)
{
    int nRC = NO_ERROR;

#ifdef _DEBUG
    glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "split component name %s", strPendingName);
#endif
    SplitCompName(strPendingName, pMainTdInfo);

    nRC = pOtmXmlLocParser->GetComponentPos(pMainTdInfo);
#ifdef _DEBUG
    glogOpenTM2Starter.writef(LOG_OPENTM2_STARTER_NAME, "get position %d", nRC);
#endif

    return nRC;
}

void GetFileNameFromUrl(const char * strUrl, char * strFileName)
{
    if ((NULL == strUrl) || (strlen(strUrl) == 0))
    {
        return;
    }

    int nCnt = strlen(strUrl) - 1;
    int iStartInx;
    for (iStartInx = nCnt; iStartInx >= 0; iStartInx--)
    {
        if (strUrl[iStartInx] == '/')
        {
            break;
        }
    }

    if ((iStartInx >= nCnt) || (iStartInx <= 0))
    {
        return;
    }
    iStartInx++;

    memset(strFileName, 0x00, sizeof(strFileName));
    int iFileNameInx = 0;
    for (int iInx = iStartInx; iInx <= nCnt; iInx++)
    {
        strFileName[iFileNameInx] = strUrl[iInx];
        iFileNameInx++;
    }

    strFileName[iFileNameInx] = EOS;
}

int SetFixpackState(PMAINTHREADINFO pMainTdInfo, int nState, BOOL bCreate)
{
    int nRC = NO_ERROR;

    char strFixpConf[MAX_PATH];
    memset(strFixpConf, 0x00, sizeof(strFixpConf));

    sprintf(strFixpConf, "%s\\%s\\%s", gstrOtmPath, OPENTM2_PLUGIN_FOLDER_STR, PLUGIN_MGR_FIXP_CONFIG);
    // if the file not exist, create the file first
    if (OTM_NOT_FOUND == access(strFixpConf, 0) && bCreate)
    {
        FILE * pConf = fopen(strFixpConf, "wb+");
        if (NULL == pConf)
        {
            nRC = ERROR_OTM_FILE_NOT_FIND_A;
            return nRC;
        }
        fclose(pConf);
    }

    // set app value of config file
    char strApp[MAX_LEN];
    memset(strApp,  0x00, sizeof(strApp));
    sprintf(strApp, "%s_%s", pMainTdInfo->strName, pMainTdInfo->strVer);

    char strValue[MAX_LEN];
    memset(strValue, 0x00, sizeof(strValue));
    sprintf(strValue, "%d", nState);

    // Write the status to file
    WritePrivateProfileString(strApp, pMainTdInfo->strFixpkId, strValue, strFixpConf);

    return nRC;
}

void GetDefPathCopies(PCOTMCOPIES pDefPathCopies)
{
    COTMCOPY defPathCopy;
    InitOtmCopy(&defPathCopy);

    char strPluginPath[MAX_PATH];
    memset(strPluginPath, 0x00, sizeof(strPluginPath));
    sprintf(strPluginPath, "%s\\%s", gstrOtmPath, OPENTM2_PLUGIN_FOLDER_STR);

    strcpy(defPathCopy.strFrom,      PLUGIN_DEF_PATH_KEY);
    strcpy(defPathCopy.strTo,        strPluginPath);

    pDefPathCopies->push_back(defPathCopy);

    InitOtmCopy(&defPathCopy);
    strcpy(defPathCopy.strFrom,      OPENTM2_DEF_PATH_KEY);
    strcpy(defPathCopy.strTo,        gstrOtmPath);

    pDefPathCopies->push_back(defPathCopy);

    char strDLPath[MAX_PATH];
    memset(strDLPath, 0x00, sizeof(strDLPath));
    sprintf(strDLPath, "%s\\%s", gstrOtmPath, OPENTM2_PENDING_FOLDER_STR);

    InitOtmCopy(&defPathCopy);
    strcpy(defPathCopy.strFrom,      OPENTM2_DEF_DL_PATH_KEY);
    strcpy(defPathCopy.strTo,        strDLPath);

    pDefPathCopies->push_back(defPathCopy);
}
