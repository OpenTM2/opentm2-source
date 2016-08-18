//+----------------------------------------------------------------------------+
//|SpecialCharDlg.H     Add new character function                             |
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
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
#pragma once

#include <vector>
#include "EQF.h"
#include "EQF.ID"
#include "EQFB.MRI"

#define MODE_NEW                                                   1
#define MODE_EDIT                                                  2

#define MAX_SPEC_CHAR_LEN                                          1
#define MAX_SPEC_CHAR_SIZE                                         2
#define MAX_BUF_SIZE                                               1024
#define MAX_SPEC_CHAR_STR_LEN                                      120

#define ERROR_NOT_ENOUGH_SIZE                                      300001

typedef struct _SPECCHARW
{
    wchar_t wstrChar[MAX_BUF_SIZE];
    wchar_t wstrCharUni[MAX_BUF_SIZE];
    wchar_t wstrCharName[MAX_BUF_SIZE];
    wchar_t wstrCharKey[MAX_BUF_SIZE];

} SPECCHARW, * PSPECCHARW;

struct SPECCHARKEY
{
    UCHAR     ucCode;                     // character code or virtual key
    UCHAR     ucState;                    // shift state
    EQF_BOOL  fChange;                    // key has been changed
    BYTE      bEditor;                    // indicat. in which editor func is valid
    wchar_t   wstrDispChar[MAX_BUF_SIZE];
};

typedef std::vector <SPECCHARKEY> SPECCHARKEYVEC;

BOOL CheckDupCharW(const wchar_t * wstrTarChar, HWND hwndDlg);
void AssignSpecCharW(HWND hwndDlg);
void AddSpecCharToFileW(const wchar_t * wstrTarChar);
void SplitSpecCharStrW(const wchar_t * wstrCharStr, PSPECCHARW pSepcChar);
BOOL CompareSpecCharW(const wchar_t * wstrCharStr1, const wchar_t * wstrCharStr2, BOOL bCmpName, BOOL bCmpKey);
void RemoveSpecCharFromVecW(const wchar_t * wstrTarChar);
void QueryInsSpecCharW(UCHAR uCode, UCHAR ucState, PSPECCHARW pSepcChar);
int AddToSpecCharVec(UCHAR ucCode, UCHAR ucState, EQF_BOOL fChange, BYTE bEditor, wchar_t * wstrDispChar);
__declspec(dllexport)
SPECCHARKEYVEC* GetSpecCharKeyVec();
void RemoveKeyFromItem(wchar_t * wstrItem);
SPECCHARKEY* QuerySpecChar(wchar_t * wstrItem);
void ReplaceSpecCharW(int nItem, wchar_t * wstrNewItem);
int  ClearSpecCharKey(wchar_t * wstrItemText, wchar_t * wstrNewItem);
void  ClearAllSpecCharKey();
void ClearSpecCharKeyVec();
void BackupSpecCharKeyVec();
void RestoreSpecCharKeyVec();

// SpecialCharDlg dialog
class SpecialCharDlg
{
private:
    HWND m_hwndDlg;
    HWND m_hWndParent;
    int  m_nMode;                // New or Edit
    wchar_t m_wstrSpecChar[MAX_SPEC_CHAR_SIZE];
    wchar_t m_wstrUnicode[MAX_BUF_SIZE];
    wchar_t m_wstrName[MAX_BUF_SIZE];
    wchar_t m_wstrKey[MAX_BUF_SIZE];

public:
    SpecialCharDlg(HWND hWndParent = NULL);   // standard constructor
    ~SpecialCharDlg();
    int SpecialCharDlgOpen(HWND hWndParent = NULL, int nMode = MODE_NEW);
    int GetSpcialChar(wchar_t * wstrChar);
    int GetCharUnicode(LPWSTR wstrCharUnicode);
    int GetCharName(LPWSTR wstrCharName);
    int GetCharKey(LPWSTR wstrCharKey);
    int SetSpcialCharStr(LPWSTR wstrCharStr);

private:
    static INT_PTR CALLBACK SpecialCharDlgProc(HWND hwndDlg, UINT msg, WPARAM mp1, LPARAM mp2);
    BOOL OnInitDialog();
    void OnBnClickedBtnOpenWinCharMap();
    LRESULT SpecialCharDlgCmd(WPARAM mp1, LPARAM mp2);
    int OtmOpenApp(LPWSTR wstrCmd, BOOL bNeedWait);
};
