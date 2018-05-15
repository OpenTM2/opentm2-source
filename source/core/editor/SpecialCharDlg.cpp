//+----------------------------------------------------------------------------+
//|SpecialCharDlg.CPP     Add new character function                           |
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
//|                    during auto version up                                  |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "SpecialCharDlg.h"

SPECCHARKEYVEC vecSpecCharKey;
SPECCHARKEYVEC vecSpecCharKeyBak;

// SpecialCharDlg dialog
SpecialCharDlg::SpecialCharDlg(HWND hWndParent /*=NULL*/)
{
    m_hWndParent = hWndParent;
    memset(m_wstrSpecChar, 0x00, sizeof(m_wstrSpecChar));
    memset(m_wstrUnicode,  0x00, sizeof(m_wstrUnicode));
    memset(m_wstrName,     0x00, sizeof(m_wstrName));
    memset(m_wstrKey,      0x00, sizeof(m_wstrKey));
    wcscpy(m_wstrKey, STR_TB_VK_NONE_W);
}

SpecialCharDlg::~SpecialCharDlg()
{
}

int SpecialCharDlg::SpecialCharDlgOpen(HWND hWndParent, int nMode)
{
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

    // start dialog box
    m_hWndParent = hWndParent;
    m_nMode      = nMode;
    INT_PTR nRes = DialogBoxParam(hResMod, MAKEINTRESOURCE(IDD_DLG_SPEC_CHAR), EqfQueryTwbClient(), SpecialCharDlgProc, (LPARAM)this);
    return nRes;
}

// SpecialCharDlg message handlers
INT_PTR CALLBACK SpecialCharDlg::SpecialCharDlgProc
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
            SpecialCharDlg * pDlg = (SpecialCharDlg *) mp2;
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (long) pDlg);
            pDlg->m_hwndDlg = hwndDlg;
            pDlg->OnInitDialog();
        }
        break;

    case WM_COMMAND:
        {
            SpecialCharDlg * pDlg = (SpecialCharDlg *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
            mResult = pDlg->SpecialCharDlgCmd(mp1, mp2);
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

LRESULT SpecialCharDlg::SpecialCharDlgCmd
(
WPARAM mp1,
LPARAM mp2
)
{
    LRESULT mResult = TRUE;// TRUE = command is processed

    switch (HIWORD(mp1))
    {
    case EN_CHANGE:
        if (LOWORD(mp1) == IDC_EDT_CHAR)
        {
            HWND hwndEditChar = GetDlgItem(m_hwndDlg, IDC_EDT_CHAR);
            int nCnt = GetWindowText(hwndEditChar, m_wstrSpecChar, MAX_SPEC_CHAR_SIZE);
            if (0 == nCnt)
            {
                break;
            }
            m_wstrSpecChar[nCnt] = L'\0';
            wsprintf(m_wstrUnicode, L"x%X", (long)m_wstrSpecChar[0]);
            SetWindowText(GetDlgItem(m_hwndDlg, IDC_EDT_UNICODE), m_wstrUnicode);
        }
        else if (LOWORD(mp1) == IDC_EDT_CHARNAME)
        {
            HWND hwndEditChar = GetDlgItem(m_hwndDlg, IDC_EDT_CHARNAME);
            GetWindowText(hwndEditChar, m_wstrName, MAX_SPEC_CHAR_SIZE);
        }
        break;

    case BN_CLICKED:
        switch (LOWORD(mp1))
        {
        case IDC_BTN_OPEN_WIN_CHAR_MAP:
            OnBnClickedBtnOpenWinCharMap();
            break;

        case IDOK:
            {
                HWND hwndEdit = GetDlgItem(m_hwndDlg, IDC_EDT_CHAR);
                int nCntChar = GetWindowText(hwndEdit, m_wstrSpecChar, MAX_SPEC_CHAR_SIZE);

                hwndEdit = GetDlgItem(m_hwndDlg, IDC_EDT_UNICODE);
                int nCntUnicode = GetWindowText(hwndEdit, m_wstrUnicode, MAX_BUF_SIZE);

                hwndEdit = GetDlgItem(m_hwndDlg, IDC_EDT_CHARNAME);
                int nCntName = GetWindowText(hwndEdit, m_wstrName, MAX_BUF_SIZE);

                if ((0 == nCntChar) || (0 == nCntUnicode))
                {
                    MessageBoxA(m_hwndDlg, STR_ERR_EMPTY_STR, STR_SPEC_CHAR_DLG_TITLE, MB_OK | MB_ICONEXCLAMATION);
                    break;
                }

                m_wstrSpecChar[nCntChar] = L'\0';
                m_wstrUnicode[nCntUnicode] = L'\0';
                m_wstrName[nCntName] = L'\0';
            }
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

void SpecialCharDlg::OnBnClickedBtnOpenWinCharMap()
{
    // Open Windows CharMpa
    OtmOpenApp(L"C:\\windows\\system32\\charmap.exe", FALSE);
}

BOOL SpecialCharDlg::OnInitDialog()
{
    //OtmCenterWindow(hwndDlg);
    //SetForegroundWindow(hwndDlg);
    HWND hwndEdit = GetDlgItem(m_hwndDlg, IDC_EDT_CHAR);
    SendMessage(hwndEdit, EM_SETLIMITTEXT, MAX_SPEC_CHAR_LEN, NULL);
    SendMessage(GetDlgItem(m_hwndDlg, IDC_EDT_UNICODE), EM_SETREADONLY, TRUE, 0);

    // set the value for the edit box
    SetDlgItemText(m_hwndDlg, IDC_EDT_CHAR,     m_wstrSpecChar);
    SetDlgItemText(m_hwndDlg, IDC_EDT_UNICODE,  m_wstrUnicode);
    SetDlgItemText(m_hwndDlg, IDC_EDT_CHARNAME, m_wstrName);

    if (MODE_NEW == m_nMode)
    {
        SetWindowTextA(m_hwndDlg, STR_SPEC_CHAR_DLG_TITLE);
        SetDlgItemTextA(m_hwndDlg, IDOK, STR_BTN_IDOK_ADD);
        EnableWindow(GetDlgItem(m_hwndDlg, IDC_EDT_CHAR), TRUE);
    }
    else
    {
        SetWindowTextA(m_hwndDlg, STR_SPEC_CHAR_DLG_TITLE_E);
        SetDlgItemTextA(m_hwndDlg, IDOK, STR_BTN_IDOK_EDIT);
        EnableWindow(GetDlgItem(m_hwndDlg, IDC_EDT_CHAR), FALSE);
    }

    return TRUE;  // return TRUE unless you set the focus to a control
}

int SpecialCharDlg::OtmOpenApp(LPWSTR wstrCmd, BOOL bNeedWait)
{
    DWORD nRC = NO_ERROR;

    // if requires elevation, use ShellExecute instead of
    SHELLEXECUTEINFO otmShellInfo;

    memset(&otmShellInfo, 0x00, sizeof(otmShellInfo));
    otmShellInfo.cbSize = sizeof(otmShellInfo);
    otmShellInfo.hwnd = m_hwndDlg;
    otmShellInfo.lpVerb = L"open";
    otmShellInfo.lpFile = wstrCmd;
    otmShellInfo.nShow = SW_SHOWNORMAL;
    otmShellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;

    if (ShellExecuteEx(&otmShellInfo))
    {
        /*HWND hwndCM = ::FindWindow(NULL, L"Character Map");
        if (NULL != hwndCM)
        {
            RECT dlgRect;
            GetWindowRect(m_hwndDlg, &dlgRect);
            RECT dlgRectCM;
            GetWindowRect(hwndCM, &dlgRectCM);
            SetWindowPos(hwndCM, HWND_TOP, dlgRect.left, dlgRect.top, dlgRectCM.right - dlgRectCM.left, dlgRectCM.bottom - dlgRectCM.top, NULL);
            ShowWindow(hwndCM, SW_SHOW);
        }

        if (bNeedWait)
        {
            WaitForSingleObject(otmShellInfo.hProcess, INFINITE);
            if (!GetExitCodeProcess(otmShellInfo.hProcess, &nRC))
            {
                nRC = GetLastError();
            }
        }*/
    }
    else
    {
        nRC = GetLastError();
    }

    return nRC;
}

int SpecialCharDlg::GetSpcialChar(LPWSTR wstrChar)
{
    int nRes = NO_ERROR;

    if (NULL == wstrChar)
    {
        return ERROR_NOT_ENOUGH_SIZE;
    }

    wcsncpy(wstrChar, m_wstrSpecChar, wcslen(m_wstrSpecChar));

    return nRes;
}

int SpecialCharDlg::GetCharUnicode(LPWSTR wstrCharUnicode)
{
    int nRes = NO_ERROR;

    if (NULL == wstrCharUnicode)
    {
        return ERROR_NOT_ENOUGH_SIZE;
    }

    wcsncpy(wstrCharUnicode, m_wstrUnicode, wcslen(m_wstrUnicode));

    return nRes;
}

int SpecialCharDlg::GetCharName(LPWSTR wstrCharName)
{
    int nRes = NO_ERROR;

    if (NULL == wstrCharName)
    {
        return ERROR_NOT_ENOUGH_SIZE;
    }

    wcsncpy(wstrCharName, m_wstrName, wcslen(m_wstrName));

    return nRes;
}

int SpecialCharDlg::GetCharKey(LPWSTR wstrCharKey)
{
    int nRes = NO_ERROR;

    if (NULL == wstrCharKey)
    {
        return ERROR_NOT_ENOUGH_SIZE;
    }

    wcsncpy(wstrCharKey, m_wstrKey, wcslen(m_wstrKey));

    return nRes;
}

int SpecialCharDlg::SetSpcialCharStr(LPWSTR wstrCharStr)
{
    int nItem = LB_ERR;
    if ((NULL == wstrCharStr) || (wcslen(wstrCharStr) == 0))
    {
        return nItem;
    }

    SPECCHARW specChar;

    memset(specChar.wstrChar,      0x00, sizeof(specChar.wstrChar));
    memset(specChar.wstrCharUni,   0x00, sizeof(specChar.wstrCharUni));
    memset(specChar.wstrCharName,  0x00, sizeof(specChar.wstrCharName));
    memset(specChar.wstrCharKey,   0x00, sizeof(specChar.wstrCharKey));

    SplitSpecCharStrW(wstrCharStr, &specChar);

    for (size_t iInx = 0; iInx < vecSpecCharKey.size(); iInx++)
    {
        SPECCHARW specCharItem;

        memset(specCharItem.wstrChar,      0x00, sizeof(specCharItem.wstrChar));
        memset(specCharItem.wstrCharUni,   0x00, sizeof(specCharItem.wstrCharUni));
        memset(specCharItem.wstrCharName,  0x00, sizeof(specCharItem.wstrCharName));
        memset(specCharItem.wstrCharKey,   0x00, sizeof(specCharItem.wstrCharKey));

        SplitSpecCharStrW(vecSpecCharKey[iInx].wstrDispChar, &specCharItem);

        if (wcsicmp(specCharItem.wstrCharUni, specChar.wstrCharUni) == 0)
        {
            wcscpy(specChar.wstrChar, specCharItem.wstrChar);
            nItem = iInx;
            break;
        }
    }

    if (LB_ERR == nItem)
    {
        return nItem;
    }

    if ((NULL != specChar.wstrChar) && (wcslen(specChar.wstrChar) != 0))
    {
        wcscpy(m_wstrSpecChar, specChar.wstrChar);
    }

    if ((NULL != specChar.wstrCharUni) && (wcslen(specChar.wstrCharUni) != 0))
    {
        wcscpy(m_wstrUnicode, specChar.wstrCharUni);
    }

    if ((NULL != specChar.wstrCharName) && (wcslen(specChar.wstrCharName) != 0))
    {
        wcscpy(m_wstrName, specChar.wstrCharName);
    }

    if ((NULL != specChar.wstrCharKey) && (wcslen(specChar.wstrCharKey) != 0))
    {
        wcscpy(m_wstrKey, specChar.wstrCharKey);
    }

    return nItem;
}

void RemoveSpecCharFromVecW(const wchar_t * wstrTarChar)
{
    if (vecSpecCharKey.empty())
    {
        return;
    }

    size_t nDelInx;
    for (nDelInx = 0; nDelInx < vecSpecCharKey.size(); nDelInx++)
    {
        SPECCHARW specCharItem;

        memset(specCharItem.wstrChar,      0x00, sizeof(specCharItem.wstrChar));
        memset(specCharItem.wstrCharUni,   0x00, sizeof(specCharItem.wstrCharUni));
        memset(specCharItem.wstrCharName,  0x00, sizeof(specCharItem.wstrCharName));
        memset(specCharItem.wstrCharKey,   0x00, sizeof(specCharItem.wstrCharKey));

        SplitSpecCharStrW(wstrTarChar, &specCharItem);

        SPECCHARW specCharVec;

        memset(specCharVec.wstrChar,      0x00, sizeof(specCharVec.wstrChar));
        memset(specCharVec.wstrCharUni,   0x00, sizeof(specCharVec.wstrCharUni));
        memset(specCharVec.wstrCharName,  0x00, sizeof(specCharVec.wstrCharName));
        memset(specCharVec.wstrCharKey,   0x00, sizeof(specCharVec.wstrCharKey));

        SplitSpecCharStrW(vecSpecCharKey[nDelInx].wstrDispChar, &specCharVec);

        if (wcsicmp(specCharVec.wstrCharUni, specCharItem.wstrCharUni) == 0)
        {
            break;
        }
    }

    if (nDelInx < vecSpecCharKey.size())
    {
        vecSpecCharKey.erase(vecSpecCharKey.begin() + nDelInx);
    }
}

BOOL CompareSpecCharW(const wchar_t * wstrCharStr1, const wchar_t * wstrCharStr2, BOOL bCmpName, BOOL bCmpKey)
{
    BOOL bDuplicate = TRUE;

    SPECCHARW specChar1;
    SPECCHARW specChar2;

    memset(specChar1.wstrChar,      0x00, sizeof(specChar1.wstrChar));
    memset(specChar1.wstrCharUni,   0x00, sizeof(specChar1.wstrCharUni));
    memset(specChar1.wstrCharName,  0x00, sizeof(specChar1.wstrCharName));
    memset(specChar1.wstrCharKey,   0x00, sizeof(specChar1.wstrCharKey));
    memset(specChar2.wstrChar,      0x00, sizeof(specChar2.wstrChar));
    memset(specChar2.wstrCharUni,   0x00, sizeof(specChar2.wstrCharUni));
    memset(specChar2.wstrCharName,  0x00, sizeof(specChar2.wstrCharName));
    memset(specChar2.wstrCharKey,   0x00, sizeof(specChar2.wstrCharKey));

    SplitSpecCharStrW(wstrCharStr1, &specChar1);
    SplitSpecCharStrW(wstrCharStr2, &specChar2);

    // compare char
    /*if ((NULL == specChar1.wstrChar) || (wcslen(specChar1.wstrChar) == 0) ||
        (NULL == specChar2.wstrChar) || (wcslen(specChar2.wstrChar) == 0))
    {
        bDuplicate = FALSE;
        return bDuplicate;
    }

    if (wcsicmp(specChar1.wstrChar, specChar2.wstrChar) != 0)
    {
        bDuplicate = FALSE;
        return bDuplicate;
    }*/

    // compare unicode
    if ((NULL == specChar1.wstrCharUni) || (wcslen(specChar1.wstrCharUni) == 0) ||
        (NULL == specChar2.wstrCharUni) || (wcslen(specChar2.wstrCharUni) == 0))
    {
        return bDuplicate;
    }

    if (wcsicmp(specChar1.wstrCharUni, specChar2.wstrCharUni) != 0)
    {
        bDuplicate = FALSE;
        return bDuplicate;
    }

    if (!bCmpName && !bCmpKey)
    {
        return bDuplicate;
    }

    if (bCmpName)
    {
        // compare name
        if ((NULL == specChar1.wstrCharName) || (wcslen(specChar1.wstrCharName) == 0) ||
            (NULL == specChar2.wstrCharName) || (wcslen(specChar2.wstrCharName) == 0))
        {
            bDuplicate = FALSE;
            return bDuplicate;
        }

        if (wcsicmp(specChar1.wstrCharUni, specChar2.wstrCharUni) != 0)
        {
            bDuplicate = FALSE;
            return bDuplicate;
        }
    }

    if (!bCmpKey)
    {
        bDuplicate = TRUE;
        return bDuplicate;
    }
    else
    {
        // compare key
        if ((NULL == specChar1.wstrCharKey) || (wcslen(specChar1.wstrCharKey) == 0) ||
            (NULL == specChar2.wstrCharKey) || (wcslen(specChar2.wstrCharKey) == 0))
        {
            bDuplicate = FALSE;
            return bDuplicate;
        }

        if (wcsicmp(specChar1.wstrCharUni, specChar2.wstrCharUni) != 0)
        {
            bDuplicate = FALSE;
            return bDuplicate;
        }
    }

    return bDuplicate;
}

void SplitSpecCharStrW(const wchar_t * wstrCharStr, PSPECCHARW pSepcChar)
{
    // check whether char is empty
    if ((NULL == wstrCharStr) || (wcslen(wstrCharStr) == 0))
    {
        return;
    }

    // check whether the start string is "Insert character"
    if (wcsnicmp(wstrCharStr, STR_TITLE_INSERT_CHAR_W, wcslen(STR_TITLE_INSERT_CHAR_W)) != 0)
    {
        return;
    }

    wchar_t wstrTemp[MAX_BUF_SIZE];
    memset(wstrTemp, 0x00, sizeof(wstrTemp));
    wcsncpy(wstrTemp, wstrCharStr, wcslen(wstrCharStr));

    wchar_t * wstrMove = wstrTemp;
    wstrMove += wcslen(STR_TITLE_INSERT_CHAR_W) + 1; // ignoe : and space

    const wchar_t * wstrPos = wcsstr(wstrMove, L"\t");

    size_t nLen;
    // set key first
    if ((NULL != wstrPos) && (wcslen(wstrPos) != 0))
    {
        wstrPos++; // skip tab
        nLen = wcslen(wstrPos);
        wcsncpy(pSepcChar->wstrCharKey, wstrPos, nLen);
    }

    nLen = wcslen(wstrMove)-wcslen(wstrPos);
    wstrMove[nLen-1] = L'\0';
    wstrPos = wcsstr(wstrMove, L" ");
    if ((NULL == wstrPos) || (wcslen(wstrPos) == 0))
    {
        wcsncpy(pSepcChar->wstrChar, wstrMove, nLen);
        return;
    }

    // get character
    nLen = wcslen(wstrMove)-wcslen(wstrPos);
    wcsncpy(pSepcChar->wstrChar, wstrMove, nLen);

    wstrMove += nLen + 1;
    wstrPos = wcsstr(wstrMove, L" ");
    if ((NULL == wstrPos) || wcslen(wstrPos) == 0)
    {
        wcscpy(pSepcChar->wstrCharUni, wstrMove);
        return;
    }

    nLen = wcslen(wstrMove)-wcslen(wstrPos);
    wcsncpy(pSepcChar->wstrCharUni, wstrMove, nLen);
    wcscpy(pSepcChar->wstrCharName, wstrPos+1);
}

void QueryInsSpecCharW(UCHAR uCode, UCHAR ucState, PSPECCHARW pSepcChar)
{
    if ((NULC == uCode) || (NULC == ucState))
    {
        return;
    }

    if (vecSpecCharKey.empty())
    {
        return;
    }

    for (size_t iInx = 0; iInx < vecSpecCharKey.size(); iInx++)
    {
        if ((uCode   == vecSpecCharKey[iInx].ucCode) &&
            (ucState == vecSpecCharKey[iInx].ucState))
        {
            memset(pSepcChar->wstrChar,      0x00, sizeof(pSepcChar->wstrChar));
            memset(pSepcChar->wstrCharUni,   0x00, sizeof(pSepcChar->wstrCharUni));
            memset(pSepcChar->wstrCharName,  0x00, sizeof(pSepcChar->wstrCharName));
            memset(pSepcChar->wstrCharKey,   0x00, sizeof(pSepcChar->wstrCharKey));

            SplitSpecCharStrW(vecSpecCharKey[iInx].wstrDispChar, pSepcChar);
            break;
        }
    }
}

int AddToSpecCharVec(
    UCHAR     ucCode,                     // character code or virtual key
    UCHAR     ucState,                    // shift state
    EQF_BOOL  fChange,                    // key has been changed
    BYTE      bEditor, 
    wchar_t * wstrDispChar)
{
    int nItem = LB_ERR;

    SPECCHARKEY SpecCharKey;
    SpecCharKey.ucCode = ucCode;
    SpecCharKey.ucState = ucState;
    SpecCharKey.fChange = fChange;
    SpecCharKey.bEditor = bEditor;
    memset(SpecCharKey.wstrDispChar, 0x00, sizeof(SpecCharKey.wstrDispChar));
    wcsncpy(SpecCharKey.wstrDispChar, wstrDispChar, wcslen(wstrDispChar));

    vecSpecCharKey.push_back(SpecCharKey);
    vecSpecCharKeyBak.push_back(SpecCharKey);

    nItem = vecSpecCharKey.size() - 1;

    return nItem;
}

SPECCHARKEYVEC* GetSpecCharKeyVec()
{
    return &vecSpecCharKey;
}

void ClearSpecCharKeyVec()
{
    vecSpecCharKey.clear();
}

void ClearSpecCharKeyBakVec()
{
    vecSpecCharKeyBak.clear();
}

void BackupSpecCharKeyVec()
{
    if (vecSpecCharKey.empty())
    {
        return;
    }

    vecSpecCharKeyBak.clear();
    for (size_t iInx = 0; iInx < vecSpecCharKey.size(); iInx++)
    {
        SPECCHARKEY specCharKey;
        specCharKey.ucCode   = vecSpecCharKey[iInx].ucCode;
        specCharKey.ucState  = vecSpecCharKey[iInx].ucState;
        specCharKey.bEditor  = vecSpecCharKey[iInx].bEditor;
        specCharKey.fChange  = vecSpecCharKey[iInx].fChange;
        wcscpy(specCharKey.wstrDispChar, vecSpecCharKey[iInx].wstrDispChar);
        vecSpecCharKeyBak.push_back(specCharKey);
    }
}

void RestoreSpecCharKeyVec()
{
    if (vecSpecCharKey.empty() || vecSpecCharKeyBak.empty())
    {
        return;
    }

    if (vecSpecCharKey.size() != vecSpecCharKeyBak.size())
    {
        return;
    }

    for (size_t iInx = 0; iInx < vecSpecCharKeyBak.size(); iInx++)
    {
        vecSpecCharKey[iInx].ucCode   = vecSpecCharKeyBak[iInx].ucCode;
        vecSpecCharKey[iInx].ucState  = vecSpecCharKeyBak[iInx].ucState;
        vecSpecCharKey[iInx].bEditor  = vecSpecCharKeyBak[iInx].bEditor;
        vecSpecCharKey[iInx].fChange  = vecSpecCharKeyBak[iInx].fChange;
        wcscpy(vecSpecCharKey[iInx].wstrDispChar, vecSpecCharKeyBak[iInx].wstrDispChar);
    }
}

SPECCHARKEY * QuerySpecChar(wchar_t * wstrItem)
{
    if (vecSpecCharKey.empty())
    {
        return NULL;
    }

    for (size_t iInx = 0; iInx < vecSpecCharKey.size(); iInx++)
    {
        /*if (wcsicmp(wstrItem, vecSpecCharKey[iInx].wstrDispChar) == 0)
        {
        return &vecSpecCharKey[iInx];
        }*/
        SPECCHARW specCharItem;

        memset(specCharItem.wstrChar,      0x00, sizeof(specCharItem.wstrChar));
        memset(specCharItem.wstrCharUni,   0x00, sizeof(specCharItem.wstrCharUni));
        memset(specCharItem.wstrCharName,  0x00, sizeof(specCharItem.wstrCharName));
        memset(specCharItem.wstrCharKey,   0x00, sizeof(specCharItem.wstrCharKey));

        SplitSpecCharStrW(wstrItem, &specCharItem);

        SPECCHARW specCharVec;

        memset(specCharVec.wstrChar,      0x00, sizeof(specCharVec.wstrChar));
        memset(specCharVec.wstrCharUni,   0x00, sizeof(specCharVec.wstrCharUni));
        memset(specCharVec.wstrCharName,  0x00, sizeof(specCharVec.wstrCharName));
        memset(specCharVec.wstrCharKey,   0x00, sizeof(specCharVec.wstrCharKey));

        SplitSpecCharStrW(vecSpecCharKey[iInx].wstrDispChar, &specCharVec);

        if (wcsicmp(specCharVec.wstrCharUni, specCharItem.wstrCharUni) == 0)
        {
            wcscpy(wstrItem, vecSpecCharKey[iInx].wstrDispChar); // copy again for ? char
            return &vecSpecCharKey[iInx];
        }
    }

    return NULL;
}

void RemoveKeyFromItem(wchar_t * wstrItem)
{
    wchar_t * wstrPos = wcsstr(wstrItem,  L"\t");
    int nLen = wcslen(wstrItem) - wcslen(wstrPos);
    wstrItem[nLen] = L'\0';
}

void ReplaceSpecCharW(int nItem, wchar_t * wstrNewItem)
{
    if (vecSpecCharKey.empty())
    {
        return;
    }

    if (nItem >= (int) vecSpecCharKey.size())
    {
        return;
    }

    wcscpy(vecSpecCharKey[nItem].wstrDispChar, wstrNewItem);
}

int  ClearSpecCharKey(wchar_t * wstrItemText, wchar_t * wstrNewItem)
{
    int nItem = LB_ERR;
    if (vecSpecCharKey.empty())
    {
        return nItem;
    }

    if (wcsnicmp(wstrItemText, STR_TITLE_INSERT_CHAR_W, wcslen(STR_TITLE_INSERT_CHAR_W)) != 0)
    {
        return nItem;
    }

    SPECCHARW specCharItem;

    memset(specCharItem.wstrChar,      0x00, sizeof(specCharItem.wstrChar));
    memset(specCharItem.wstrCharUni,   0x00, sizeof(specCharItem.wstrCharUni));
    memset(specCharItem.wstrCharName,  0x00, sizeof(specCharItem.wstrCharName));
    memset(specCharItem.wstrCharKey,   0x00, sizeof(specCharItem.wstrCharKey));

    SplitSpecCharStrW(wstrItemText, &specCharItem);

    for (size_t iInx = 0; iInx < vecSpecCharKey.size(); iInx++)
    {
        SPECCHARW specCharVec;

        memset(specCharVec.wstrChar,      0x00, sizeof(specCharVec.wstrChar));
        memset(specCharVec.wstrCharUni,   0x00, sizeof(specCharVec.wstrCharUni));
        memset(specCharVec.wstrCharName,  0x00, sizeof(specCharVec.wstrCharName));
        memset(specCharVec.wstrCharKey,   0x00, sizeof(specCharVec.wstrCharKey));

        SplitSpecCharStrW(vecSpecCharKey[iInx].wstrDispChar, &specCharVec);

        if (wcsicmp(specCharVec.wstrCharUni, specCharItem.wstrCharUni) == 0)
        {
            vecSpecCharKey[iInx].ucState = NULC;
            vecSpecCharKey[iInx].ucState = NULC;
            vecSpecCharKey[iInx].fChange = TRUE;
            RemoveKeyFromItem(vecSpecCharKey[iInx].wstrDispChar);
            wsprintf(vecSpecCharKey[iInx].wstrDispChar, L"%s\t%s", vecSpecCharKey[iInx].wstrDispChar, STR_TB_VK_NONE_W);
            wcscpy(wstrNewItem, vecSpecCharKey[iInx].wstrDispChar);
            nItem = iInx;
            break;
        }
    }

    return nItem;
}

void  ClearAllSpecCharKey()
{
    if (vecSpecCharKey.empty())
    {
        return;
    }

    for (size_t iInx = 0; iInx < vecSpecCharKey.size(); iInx++)
    {
        vecSpecCharKey[iInx].ucState = NULC;
        vecSpecCharKey[iInx].ucState = NULC;
        vecSpecCharKey[iInx].fChange = TRUE;
        RemoveKeyFromItem(vecSpecCharKey[iInx].wstrDispChar);
        wsprintf(vecSpecCharKey[iInx].wstrDispChar, L"%s\t%s", vecSpecCharKey[iInx].wstrDispChar, STR_TB_VK_NONE_W);
    }
}
