//+----------------------------------------------------------------------------+
//|OtmProfileMgrDlg.cpp   OTM Profile Manager dialog class                     |
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
    OtmLogWriter::timestamp(DEF_PROFILE_SET_LOG_NAME, 0);
    OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Dialog Open.");

    HWND hwnd = (HWND)UtlQueryULong(QL_TWBFRAME);
    INT_PTR nRes = DialogBoxParam(hDllInst, MAKEINTRESOURCE(IDD_DLG_PROFILE_SET_MGR), hwnd, OtmProfileMgrDlgProc, (LPARAM)this);
    if (nRes == -1)
    {
        char strErrMsg[MAX_BUF_SIZE];
        memset(strErrMsg, 0x00, sizeof(strErrMsg));
        nRes = GetLastError();
        sprintf(strErrMsg, ERROR_OPEN_PROFILE_SET_DIALOG_A_STR, nRes);
        MessageBox(hwnd, strErrMsg, APP_TOOL_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Dialog Open failed.");
    }
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
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Initial dialog.");
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

LRESULT OtmProfileMgrDlg::OtmProfileMgrDlgCmd(WPARAM mp1, LPARAM mp2)
{
    LRESULT mResult = TRUE;// TRUE = command is processed

    switch (HIWORD(mp1))
    {
    case BN_CLICKED:
        switch (LOWORD(mp1))
        {
        case IDOK:
            if (SaveProfileMgrTabSettings(m_hwndPages[0]))
            {
                return FALSE;
            }
            EndDialog(m_hwndDlg, IDOK);
            break;

        case IDCANCEL:
            EndDialog(m_hwndDlg, IDCANCEL);
            break;

        case IDC_BTN_DLG_HELP:
            MessageBox(m_hwndDlg, INFO_HELP_NOT_AVAILABLE_STR, APP_TOOL_NAME_STR, MB_OK);
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
    nRes = ProfileMgrTabLoad();
    return nRes;  // return TRUE unless you set the focus to a control
}

BOOL OtmProfileMgrDlg::ProfileMgrTabLoad()
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
                                       ProfileMgrTabFunc,
                                       (LPARAM) this);

    SetWindowPos(m_hwndPages[0], HWND_TOP,
                 otmRect.left, otmRect.top,
                 otmRect.right-otmRect.left, otmRect.bottom-otmRect.top, SWP_SHOWWINDOW);

    return bRes;
}

INT_PTR CALLBACK OtmProfileMgrDlg::ProfileMgrTabFunc(HWND hwndTabDlg, UINT msg, WPARAM mp1, LPARAM mp2)
{
    LRESULT  mResult = FALSE;                // result value of procedure

    switch (msg)
    {
    case WM_INITDIALOG:
        {
            OtmProfileMgrDlg * pDlg = (OtmProfileMgrDlg *) mp2;
            SetWindowLongPtr(hwndTabDlg, GWLP_USERDATA, (long) pDlg);
            pDlg->ProfileMgrTabInit(hwndTabDlg);
        }
        break;

    case WM_COMMAND:
        {
            OtmProfileMgrDlg * pDlg = (OtmProfileMgrDlg *) GetWindowLongPtr(hwndTabDlg, GWLP_USERDATA);
            mResult = pDlg->ProfileMgrTabCmd(mp1, (LPARAM) hwndTabDlg);
        }
        break;

    default:
        break;
    }

    return mResult;
}

LRESULT OtmProfileMgrDlg::ProfileMgrTabCmd(WPARAM mp1, LPARAM mp2)
{
    LRESULT mResult = TRUE;// TRUE = command is processed

    HWND hwndTabDlg = HWND(mp2);

    switch (HIWORD(mp1))
    {
    case BN_CLICKED:
        switch (LOWORD(mp1))
        {
        case IDC_RADIO_EXPORT:
            SetEIBtnState(hwndTabDlg);
            break;
        case IDC_RADIO_IMPORT:
            SetEIBtnState(hwndTabDlg);
            break;
        case IDC_BTN_BROWSE:
            OtmOpenFileDlg(hwndTabDlg);
            break;
        case IDC_BTN_DONE:
            ProfileSetExecute(hwndTabDlg);
            break;
        case IDC_CHK_EI_ALL:
            SetProfileChkSetState(hwndTabDlg);
            break;
        case IDC_CHK_TRANS_EDITOR:
            SetChkAllBtnState(hwndTabDlg, IDC_CHK_TRANS_EDITOR);
            break;
        case IDC_CHK_WORKBENCH:
            SetChkAllBtnState(hwndTabDlg, IDC_CHK_WORKBENCH);
            break;
        case IDC_CHK_FLD_LIST:
            SetChkAllBtnState(hwndTabDlg, IDC_CHK_FLD_LIST);
            break;
        case IDC_CHK_LAST_USED_VALUE:
            SetChkAllBtnState(hwndTabDlg, IDC_CHK_LAST_USED_VALUE);
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

void OtmProfileMgrDlg::ProfileMgrTabInit(HWND hwndDlgSheet)
{
    // initial format combo box first
    SendDlgItemMessage(hwndDlgSheet, IDC_COMBO_FORMAT, CB_INSERTSTRING, 0, (LPARAM)FORMAT_XML_EXT);
    SendDlgItemMessage(hwndDlgSheet, IDC_COMBO_FORMAT, CB_SETCURSEL, 0, (LPARAM) 0);

    char strPluginPath[MAX_EQF_PATH];
    memset(strPluginPath, 0x00, sizeof(strPluginPath));
    UtlMakeEQFPath(strPluginPath, NULC, PLUGIN_PATH, NULL);

    CProfileConfXmlParser profileConfXmlParser(strPluginPath);

    OPTIONSET setOption;
    InitSetOption(&setOption);
    // ignore error for import config file, still open the tool even load failed
    profileConfXmlParser.LoadProfileSetConfig(&setOption);
    m_nMaxHistCnt = setOption.nMaxHistCnt;

    // set import or export
    if (setOption.bExport)
    {
        CheckDlgButton(hwndDlgSheet, IDC_RADIO_EXPORT, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hwndDlgSheet, IDC_RADIO_IMPORT, BST_CHECKED);
    }

    // check box: all
    if (setOption.bChkAll)
    {
        CheckDlgButton(hwndDlgSheet, IDC_CHK_EI_ALL, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hwndDlgSheet, IDC_CHK_EI_ALL, BST_UNCHECKED);
    }

    // check box: translation editor
    if (setOption.bChkTransEditor)
    {
        CheckDlgButton(hwndDlgSheet, IDC_CHK_TRANS_EDITOR, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hwndDlgSheet, IDC_CHK_TRANS_EDITOR, BST_UNCHECKED);
    }

    // check box: workbench
    if (setOption.bChkWorkbench)
    {
        CheckDlgButton(hwndDlgSheet, IDC_CHK_WORKBENCH, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hwndDlgSheet, IDC_CHK_WORKBENCH, BST_UNCHECKED);
    }

    // check box: folder list
    if (setOption.bChkFldList)
    {
        CheckDlgButton(hwndDlgSheet, IDC_CHK_FLD_LIST, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hwndDlgSheet, IDC_CHK_FLD_LIST, BST_UNCHECKED);
    }

    // check box: last used value
    if (setOption.bChkLastVal)
    {
        CheckDlgButton(hwndDlgSheet, IDC_CHK_LAST_USED_VALUE, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hwndDlgSheet, IDC_CHK_LAST_USED_VALUE, BST_UNCHECKED);
    }

    for (size_t iInx = 0; iInx < setOption.lstStrDirs.size(); iInx++)
    {
        const char * strValue = setOption.lstStrDirs[iInx].c_str();
        if ((NULL != strValue) && (strlen(strValue) != 0))
        {
            OtmAddToComboBox(GetDlgItem(hwndDlgSheet, IDC_COMBO_FROMTO), strValue);
        }
    }

    for (size_t iInx = 0; iInx < setOption.lstStrNames.size(); iInx++)
    {
        const char * strValue = setOption.lstStrNames[iInx].c_str();
        if ((NULL != strValue) && (strlen(strValue) != 0))
        {
            OtmAddToComboBox(GetDlgItem(hwndDlgSheet, IDC_COMBO_PROFILE_NAME), strValue);
        }
    }

    // set file info
    SetFileInfo(hwndDlgSheet, setOption.strTarFile, FALSE);

    // set button state
    SetEIBtnState(hwndDlgSheet);
}

int OtmProfileMgrDlg::SaveProfileMgrTabSettings(HWND hwndDlgSheet)
{
    int nRC = NO_ERROR;

    char strPluginPath[MAX_EQF_PATH];
    memset(strPluginPath, 0x00, sizeof(strPluginPath));
    UtlMakeEQFPath(strPluginPath, NULC, PLUGIN_PATH, NULL);

    CProfileConfXmlParser profileConfXmlParser(strPluginPath);

    OPTIONSET setOption;
    InitSetOption(&setOption);
    profileConfXmlParser.LoadProfileSetConfig(&setOption);
    nRC = GetTargetOption(hwndDlgSheet, &setOption);

    // get current settings
    UINT nChked = IsDlgButtonChecked(hwndDlgSheet, IDC_RADIO_EXPORT);

    if (BST_CHECKED == nChked)
    {
        setOption.bExport = TRUE;
    }
    else
    {
        setOption.bExport = FALSE;
    }

    nRC = profileConfXmlParser.SaveProfileSetConfig(&setOption);
    OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Save all current dialog setting (%d).", nRC);

    return nRC;
}

// Set the state of the dialog according to Export or Import
void OtmProfileMgrDlg::SetEIBtnState(HWND hwndDlgSheet)
{
    UINT nChked = IsDlgButtonChecked(hwndDlgSheet, IDC_RADIO_EXPORT);

    if (BST_CHECKED == nChked)
    {
        SetWindowText(GetDlgItem(hwndDlgSheet, IDC_STA_FROMTO), CAP_EXPORT_TO_STR);
        SetWindowText(GetDlgItem(hwndDlgSheet, IDC_STA_FRM_EI), CAP_FRM_EXPORT_STR);
        SetWindowText(GetDlgItem(hwndDlgSheet, IDC_CHK_EI_ALL), CAP_CHK_ALL_EXPORT_STR);
        SetWindowText(GetDlgItem(hwndDlgSheet, IDC_BTN_DONE),   CAP_BTN_EXPORT_STR);
    }
    else
    {
        SetWindowText(GetDlgItem(hwndDlgSheet, IDC_STA_FROMTO), CAP_IMPORT_FROM_STR);
        SetWindowText(GetDlgItem(hwndDlgSheet, IDC_STA_FRM_EI), CAP_FRM_IMPORT_STR);
        SetWindowText(GetDlgItem(hwndDlgSheet, IDC_BTN_DONE),   CAP_BTN_IMPORT_STR);
        SetWindowText(GetDlgItem(hwndDlgSheet, IDC_CHK_EI_ALL), CAP_CHK_ALL_IMPORT_STR);
    }
}

void OtmProfileMgrDlg::SetProfileChkSetState(HWND hwndDlgSheet)
{
    UINT nChked = IsDlgButtonChecked(hwndDlgSheet, IDC_CHK_EI_ALL);
    if (BST_CHECKED == nChked)
    {
        CheckDlgButton(hwndDlgSheet, IDC_CHK_TRANS_EDITOR, BST_CHECKED);
        CheckDlgButton(hwndDlgSheet, IDC_CHK_WORKBENCH, BST_CHECKED);
        CheckDlgButton(hwndDlgSheet, IDC_CHK_FLD_LIST, BST_CHECKED);
        CheckDlgButton(hwndDlgSheet, IDC_CHK_LAST_USED_VALUE, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hwndDlgSheet, IDC_CHK_TRANS_EDITOR, BST_UNCHECKED);
        CheckDlgButton(hwndDlgSheet, IDC_CHK_WORKBENCH, BST_UNCHECKED);
        CheckDlgButton(hwndDlgSheet, IDC_CHK_FLD_LIST, BST_UNCHECKED);
        CheckDlgButton(hwndDlgSheet, IDC_CHK_LAST_USED_VALUE, BST_UNCHECKED);
    }
}

void OtmProfileMgrDlg::SetChkAllBtnState(HWND hwndTabDlg, UINT nSubItmID)
{
    UINT nChked = IsDlgButtonChecked(hwndTabDlg, nSubItmID);
    if (BST_CHECKED != nChked)
    {
        CheckDlgButton(hwndTabDlg, IDC_CHK_EI_ALL, BST_UNCHECKED);
    }

    // if all sub item checked, select check all
    if ((BST_CHECKED == IsDlgButtonChecked(hwndTabDlg, IDC_CHK_TRANS_EDITOR)) &&
        (BST_CHECKED == IsDlgButtonChecked(hwndTabDlg, IDC_CHK_WORKBENCH)) &&
        (BST_CHECKED == IsDlgButtonChecked(hwndTabDlg, IDC_CHK_FLD_LIST)) &&
        (BST_CHECKED == IsDlgButtonChecked(hwndTabDlg, IDC_CHK_LAST_USED_VALUE)))
    {
        CheckDlgButton(hwndTabDlg, IDC_CHK_EI_ALL, BST_CHECKED);
    }
}

void OtmProfileMgrDlg::OtmOpenFileDlg(HWND hwndDlgSheet)
{
    OPENFILENAME openFileName;       // common dialog box structure
    char strFileName[MAX_PATH];      // buffer for file name
    char strInitDir[MAX_PATH];

    // Initialize OPENFILENAME
    ZeroMemory(&openFileName, sizeof(openFileName));
    memset(strFileName, 0x00, sizeof(strFileName));
    memset(strInitDir,  0x00, sizeof(strInitDir));

    GetWindowText(GetDlgItem(hwndDlgSheet, IDC_COMBO_FROMTO), strInitDir, sizeof(strInitDir));
    GetWindowText(GetDlgItem(hwndDlgSheet, IDC_COMBO_PROFILE_NAME), strFileName, sizeof(strFileName));

    openFileName.lStructSize     = sizeof(openFileName);
    openFileName.hwndOwner       = hwndDlgSheet;
    openFileName.lpstrFile       = strFileName;
    openFileName.nMaxFile        = MAX_PATH;
    openFileName.lpstrFilter     = "XML files (*.xml)\0*.xml\0\0";
    openFileName.lpstrInitialDir = strInitDir;
    openFileName.Flags           = OFN_EXPLORER;

    UINT nChked = IsDlgButtonChecked(hwndDlgSheet, IDC_RADIO_EXPORT);

    if (BST_CHECKED == nChked)
    {
        if (GetSaveFileName(&openFileName))
        {
            SetFileInfo(hwndDlgSheet, openFileName.lpstrFile);
        }
    }
    else
    {
        openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
        if (GetOpenFileName(&openFileName))
        {
            SetFileInfo(hwndDlgSheet, openFileName.lpstrFile);
        }
    }
}

void OtmProfileMgrDlg::SetFileInfo(HWND hwndDlgSheet, const char * strFile, BOOL bAdd)
{
    char strDrive[_MAX_DRIVE];
    char strDir[_MAX_DIR];
    char strFname[_MAX_FNAME];
    char strExt[_MAX_EXT];
    _splitpath(strFile, strDrive, strDir, strFname, strExt);

    char strValue[MAX_PATH];

    memset(strValue, 0x00, sizeof(strValue));
    sprintf(strValue, "%s%s", strDrive, strDir);
    if (bAdd)
    {
        OtmAddToComboBox(GetDlgItem(hwndDlgSheet, IDC_COMBO_FROMTO), strValue);
    }
    SetDlgItemText(hwndDlgSheet, IDC_COMBO_FROMTO, strValue);

    memset(strValue, 0x00, sizeof(strValue));
    sprintf(strValue, "%s%s", strFname, strExt);
    if (bAdd)
    {
        OtmAddToComboBox(GetDlgItem(hwndDlgSheet, IDC_COMBO_PROFILE_NAME), strValue);
    }
    SetDlgItemText(hwndDlgSheet, IDC_COMBO_PROFILE_NAME, strValue);
}

int OtmProfileMgrDlg::ProfileSetExecute(HWND hwndDlgSheet)
{
    int nRC = NO_ERROR;

    // Add current value to combo box first
    char strValue[MAX_BUF_SIZE];

    memset(strValue, 0x00, sizeof(strValue));
    GetDlgItemText(hwndDlgSheet, IDC_COMBO_FROMTO, strValue, MAX_BUF_SIZE);
    OtmAddToComboBox(GetDlgItem(hwndDlgSheet, IDC_COMBO_FROMTO), strValue);

    memset(strValue, 0x00, sizeof(strValue));
    GetDlgItemText(hwndDlgSheet, IDC_COMBO_PROFILE_NAME, strValue, MAX_BUF_SIZE);
    OtmAddToComboBox(GetDlgItem(hwndDlgSheet, IDC_COMBO_PROFILE_NAME), strValue);

    UINT nChked = IsDlgButtonChecked(hwndDlgSheet, IDC_RADIO_EXPORT);

    if (BST_CHECKED == nChked)
    {
        nRC = ProfileSetExport(hwndDlgSheet);
    }
    else
    {
        nRC = ProfileSetImport(hwndDlgSheet);
    }

    return nRC;
}

int OtmProfileMgrDlg::ProfileSetExport(HWND hwndDlgSheet)
{
    int nRC = NO_ERROR;

    OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Start to export.");

    char strPluginPath[MAX_EQF_PATH];
    memset(strPluginPath, 0x00, sizeof(strPluginPath));
    UtlMakeEQFPath(strPluginPath, NULC, PLUGIN_PATH, NULL);

    CProfileConfXmlParser profileConfXmlParser(strPluginPath);

    OPTIONSET setOption;
    InitSetOption(&setOption);
    profileConfXmlParser.LoadProfileSetConfig(&setOption);
    nRC = GetTargetOption(hwndDlgSheet, &setOption);
    OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Get target option %d.", nRC);
    if (nRC)
    {
        MessageBox(hwndDlgSheet, "Failed to get the target option", APP_TOOL_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        return nRC;
    }

    // for export, check whether file exists or not
    // check the validation of the folder
    if (OTM_NOT_FOUND != access(setOption.strTarFile, 0))
    {
        char strMsg[MAX_BUF_SIZE];
        memset(strMsg, 0x00, sizeof(strMsg));
        sprintf(strMsg, WARN_FILE_EXISTS_STR, setOption.strTarFile);
        int nID = MessageBox(hwndDlgSheet, strMsg, APP_TOOL_NAME_STR, MB_YESNO | MB_ICONEXCLAMATION);
        if (nID != IDYES)
        {
            return nRC;
        }
    }

    setOption.bExport = TRUE;

    CProfileSetXmlParser parserProfileSetXml;
    nRC = parserProfileSetXml.DoProfileExport(&setOption);
    OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Export all end (%d).", nRC);

    // Temporarily delete for the encrypt problem start
    /*
    */
    // Temporarily delete end

    if (nRC)
    {
        char * strMsg = (char *) parserProfileSetXml.GetParserErrMsg();

        if ((NULL == strMsg) || (strlen(strMsg) == 0))
        {
            strMsg = OtmGetMessageFromCode(nRC);
        }

        if ((NULL != strMsg) && (strlen(strMsg) != 0))
        {
            MessageBox(hwndDlgSheet, strMsg, APP_TOOL_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        }
        else
        {
            // unknow error
            sprintf(strMsg, ERROR_UNKNOWN_STR, nRC);
            MessageBox(hwndDlgSheet, strMsg, APP_TOOL_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        }

        // delete the wrong file
        if (!setOption.bKeepOriFile)
        {
            OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "As failed, delete the target file %s.", setOption.strTarFile);
            remove(setOption.strTarFile);
        }
    }
    else
    {
        MessageBox(hwndDlgSheet, INFO_EXPORT_FINISH_STR, APP_TOOL_NAME_STR, MB_OK);
    }

    return nRC;
}

int OtmProfileMgrDlg::ProfileSetImport(HWND hwndDlgSheet)
{
    int nRC = NO_ERROR;

    OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Start to import.");

    char strPluginPath[MAX_EQF_PATH];
    memset(strPluginPath, 0x00, sizeof(strPluginPath));
    UtlMakeEQFPath(strPluginPath, NULC, PLUGIN_PATH, NULL);

    CProfileConfXmlParser profileConfXmlParser(strPluginPath);

    OPTIONSET setOption;
    InitSetOption(&setOption);
    profileConfXmlParser.LoadProfileSetConfig(&setOption);
    nRC = GetTargetOption(hwndDlgSheet, &setOption);
    OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Get target option %d.", nRC);
    if (nRC)
    {
        MessageBox(hwndDlgSheet, "Failed to get the target option", APP_TOOL_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        return nRC;
    }
    setOption.bExport = FALSE;

    // Temporarily delete for the encrypt problem start
    /*
    */
    // Temporarily delete end

    // for import, check whether file exists or not
    if (OTM_NOT_FOUND == access(setOption.strTarFile, 0))
    {
        MessageBox(hwndDlgSheet, ERROR_FILE_NOT_EXIST_STR, APP_TOOL_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        return nRC;
    }

    // confirm the importing first
    int nID = MessageBox(hwndDlgSheet, INFO_IMPORT_CONFIRM_STR, APP_TOOL_NAME_STR, MB_YESNO);
    if (nID != IDYES)
    {
        return nRC;
    }

    CProfileSetXmlParser parserProfileSetXml;
    nRC = parserProfileSetXml.DoProfileImport(&setOption);
    OtmLogWriter::writef(DEF_PROFILE_SET_LOG_NAME, 0, "Import all end (%d).", nRC);

    // Temporarily delete for the encrypt problem start
    /*
    */
    // Temporarily delete end

    if (nRC)
    {
        char * strMsg = (char *) parserProfileSetXml.GetParserErrMsg();

        if ((NULL == strMsg) || (strlen(strMsg) == 0))
        {
            strMsg = OtmGetMessageFromCode(nRC);
        }

        if ((NULL != strMsg) && (strlen(strMsg) != 0))
        {
            MessageBox(hwndDlgSheet, strMsg, APP_TOOL_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        }
        else
        {
            // unknow error
            sprintf(strMsg, ERROR_UNKNOWN_STR, nRC);
            MessageBox(hwndDlgSheet, strMsg, APP_TOOL_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        }
    }
    else
    {
        // Save the current settings
        nRC = SaveProfileMgrTabSettings(hwndDlgSheet);
        if (nRC)
        {
            return nRC;
        }

        MessageBox(hwndDlgSheet, INFO_IMPORT_FINISH_STR, APP_TOOL_NAME_STR, MB_OK);

        // Write to config file first
        char strPendingConf[MAX_PATH];
        char strPluginPath[MAX_PATH];

        UtlQueryString(QST_PLUGINPATH, strPluginPath, sizeof(strPluginPath));
        UtlMakeEQFPath(strPluginPath, NULC, PLUGIN_PATH, NULL);

        memset(strPendingConf, 0x00, sizeof(strPendingConf));
        sprintf(strPendingConf, "%s\\PendingUpdates.conf", strPluginPath);
        WritePrivateProfileString("Settings", "IsImport", "1", strPendingConf);

        // restart opentm2
        char strWinPath[MAX_PATH];
        memset(strWinPath, 0x00, sizeof(strWinPath));
        UtlQueryString(QST_WINPATH, strWinPath, sizeof(strWinPath));
        UtlMakeEQFPath(strWinPath, NULC, WIN_PATH, NULL);

        char strOpenTM2StarterPath[MAX_PATH];
        memset(strOpenTM2StarterPath, 0x00, sizeof(strOpenTM2StarterPath));
        sprintf(strOpenTM2StarterPath, "%s\\%s", strWinPath, OTM_APPL_STARTER_EXE);

        // to start OpenTM2Starter, needn't wait
        OtmExecuteCommand(strOpenTM2StarterPath, NULL, FALSE);

        // close opentm2
        HWND hwndFrame =  (HWND)UtlQueryULong(QL_TWBFRAME);
        SendMessage(hwndFrame, WM_CLOSE, NULL, NULL);
    }

    return nRC;
}

int OtmProfileMgrDlg::GetTargetOption(HWND hwndDlgSheet, POPTIONSET pOptionSet)
{
    int nRC = NO_ERROR;

    char strTarDir[MAX_PATH];
    char strTarName[_MAX_FNAME];
    char strTarExt[_MAX_EXT];

    memset(strTarDir,  0x00, sizeof(strTarDir));
    memset(strTarName, 0x00, sizeof(strTarName));

    GetWindowText(GetDlgItem(hwndDlgSheet, IDC_COMBO_FROMTO),       strTarDir,  sizeof(strTarDir));
    GetWindowText(GetDlgItem(hwndDlgSheet, IDC_COMBO_PROFILE_NAME), strTarName, sizeof(strTarName));
    GetWindowText(GetDlgItem(hwndDlgSheet, IDC_COMBO_FORMAT),       strTarExt,  sizeof(strTarExt));

    // check folder
    if ((NULL == strTarDir) || (strlen(strTarDir) == 0))
    {
        MessageBox(hwndDlgSheet, ERROR_TAR_DIR_STR, APP_TOOL_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        return ERROR_TAR_DIR_A;
    }

    // check the validation of the folder
    if (OTM_NOT_FOUND == access(strTarDir, 0))
    {
        MessageBox(hwndDlgSheet, ERROR_TAR_DIR_STR, APP_TOOL_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        return ERROR_TAR_DIR_A;
    }

    // check file name
    if ((NULL == strTarName) || (strlen(strTarName) == 0))
    {
        MessageBox(hwndDlgSheet, ERROR_TAR_FILE_NAME_STR, APP_TOOL_NAME_STR, MB_OK | MB_ICONEXCLAMATION);
        return ERROR_TAR_FILE_NAME_A;
    }

    // check valation of the file
    char * strTmpCmp2 = strTarExt + 1;
    int nLen = strlen(strTarName);
    if (nLen < 4)
    {
        strcat(strTarName, strTmpCmp2);
    }
    else
    {
        int nPos = nLen - 4;
        char * strTmpCmp1 = strTarName + nPos;
        // check the extion name
        if (stricmp(strTmpCmp1, strTmpCmp2))
        {
            strcat(strTarName, strTmpCmp2);
        }
    }

    int nLstPos = strlen(strTarDir) - 1;
    if (strTarDir[nLstPos] != '\\')
    {
        sprintf(pOptionSet->strTarFile, "%s\\%s", strTarDir, strTarName);
    }
    else
    {
        sprintf(pOptionSet->strTarFile, "%s%s", strTarDir, strTarName);
    }

    UINT nChked = IsDlgButtonChecked(hwndDlgSheet, IDC_CHK_EI_ALL);
    if (BST_CHECKED == nChked)
    {
        pOptionSet->bChkAll = TRUE;
        pOptionSet->bChkTransEditor = TRUE;
        pOptionSet->bChkWorkbench = TRUE;
        pOptionSet->bChkFldList = TRUE;
        pOptionSet->bChkLastVal = TRUE;
    }
    else
    {
        pOptionSet->bChkAll = FALSE;

        nChked = IsDlgButtonChecked(hwndDlgSheet, IDC_CHK_TRANS_EDITOR);
        if (BST_CHECKED == nChked)
        {
            pOptionSet->bChkTransEditor = TRUE;
        }
        else
        {
            pOptionSet->bChkTransEditor = FALSE;
        }

        nChked = IsDlgButtonChecked(hwndDlgSheet, IDC_CHK_WORKBENCH);
        if (BST_CHECKED == nChked)
        {
            pOptionSet->bChkWorkbench = TRUE;
        }
        else
        {
            pOptionSet->bChkWorkbench = FALSE;
        }

        nChked = IsDlgButtonChecked(hwndDlgSheet, IDC_CHK_FLD_LIST);
        if (BST_CHECKED == nChked)
        {
            pOptionSet->bChkFldList = TRUE;
        }
        else
        {
            pOptionSet->bChkFldList = FALSE;
        }

        nChked = IsDlgButtonChecked(hwndDlgSheet, IDC_CHK_LAST_USED_VALUE);
        if (BST_CHECKED == nChked)
        {
            pOptionSet->bChkLastVal = TRUE;
        }
        else
        {
            pOptionSet->bChkLastVal = FALSE;
        }
    }

    UINT nComboxCnt, iInx;
    nComboxCnt = SendDlgItemMessage(hwndDlgSheet, IDC_COMBO_FROMTO, CB_GETCOUNT, 0, 0);
    for (iInx = 0; iInx < nComboxCnt; iInx++)
    {
        char strValue[MAX_BUF_SIZE];
        memset(strValue, 0x00, sizeof(strValue));
        SendDlgItemMessage(hwndDlgSheet, IDC_COMBO_FROMTO, CB_GETLBTEXT, iInx, (LPARAM)strValue);
        OtmAddToSet(&pOptionSet->lstStrDirs, strValue);
    }

    nComboxCnt = SendDlgItemMessage(hwndDlgSheet, IDC_COMBO_PROFILE_NAME, CB_GETCOUNT, 0, 0);
    for (iInx = 0; iInx < nComboxCnt; iInx++)
    {
        char strValue[MAX_BUF_SIZE];
        memset(strValue, 0x00, sizeof(strValue));
        SendDlgItemMessage(hwndDlgSheet, IDC_COMBO_PROFILE_NAME, CB_GETLBTEXT, iInx, (LPARAM)strValue);
        OtmAddToSet(&pOptionSet->lstStrNames, strValue);
    }

    return nRC;
}

void OtmProfileMgrDlg::OtmAddToComboBox(HWND hWndComboBox, const char * strAddValue)
{
    // Add to combobox only when not duplicated
    BOOL bDuplicated = FALSE;
    int nComboboCnt = SendMessage(hWndComboBox, CB_GETCOUNT, 0, 0);
    for (int iInx = 0; iInx < nComboboCnt; iInx++)
    {
        char strValue[MAX_BUF_SIZE];
        memset(strValue, 0x00, sizeof(strValue));
        SendMessage(hWndComboBox, CB_GETLBTEXT, iInx, (LPARAM)strValue);

        if (!stricmp(strValue, strAddValue))
        {
            bDuplicated = TRUE;
        }
    }

    if (bDuplicated)
    {
        return;
    }

    if (m_nMaxHistCnt == nComboboCnt)
    {
        // remove the first record
        SendMessage(hWndComboBox, CB_DELETESTRING, 0, NULL);
        nComboboCnt--;
    }

    SendMessage(hWndComboBox, CB_INSERTSTRING, nComboboCnt, (LPARAM)strAddValue);
}
