//+----------------------------------------------------------------------------+
//|OtmProfileMgrDlg.h   OTM Profile Manager dialog class                       |
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

#pragma once

#include "resource.h"

#include "ProfileSetXmlParser.h"
#include "ProfileConfXmlParser.h"
#include "OtmFileEncryptSet.h"

#define MGR_SET_TAB_LEN                                        1
#define FORMAT_XML_EXT                                         "*.xml"

#pragma once
class OtmProfileMgrDlg
{
private:
    HWND m_hwndDlg;
    HWND m_hwndPages[MGR_SET_TAB_LEN];
    int m_nMaxHistCnt;

public:
    OtmProfileMgrDlg(void);
    ~OtmProfileMgrDlg(void);
    int OtmProfileMgrDlgOpen(HINSTANCE & hDllInst);

private:
    static INT_PTR CALLBACK OtmProfileMgrDlgProc(HWND hwndDlg, UINT msg, WPARAM mp1, LPARAM mp2);
    static INT_PTR CALLBACK ProfileMgrTabFunc(HWND hwndTabDlg, UINT msg, WPARAM mp1, LPARAM mp2);
    BOOL OnInitDialog();
    LRESULT OtmProfileMgrDlgCmd(WPARAM mp1, LPARAM mp2);
    LRESULT ProfileMgrTabCmd(WPARAM mp1, LPARAM mp2);
    BOOL ProfileMgrTabLoad();
    void ProfileMgrTabInit(HWND hwndDlgSheet);
    void SetEIBtnState(HWND hwndDlgSheet);
    void OtmOpenFileDlg(HWND hwndDlgSheet);
    int ProfileSetExecute(HWND hwndDlgSheet);
    int ProfileSetExport(HWND hwndDlgSheet);
    int ProfileSetImport(HWND hwndDlgSheet);
    int GetTargetOption(HWND hwndDlgSheet, POPTIONSET pOptionSet);
    void SetProfileChkSetState(HWND hwndDlgSheet);
    void SetFileInfo(HWND hwndDlgSheet, const char * strFile, BOOL bAdd = TRUE);
    int SaveProfileMgrTabSettings(HWND hwndDlgSheet);
    void OtmAddToComboBox(HWND hWndComboBox, const char * strAddValue);
    void SetChkAllBtnState(HWND hwndTabDlg, UINT nSubItmID);
};

