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
#include "Windows.h"
#include "resource.h"
#include <string>
#include <vector>
#include "EQF.H"
#include "OtmProfileMgrStr.h"

#define OPENTM2_APP_NAME_STR                                   "OpenTM2"
#define MGR_SET_TAB_LEN                                        1

#pragma once
class OtmProfileMgrDlg
{
private:
    HWND m_hwndDlg;
    HWND m_hwndPages[MGR_SET_TAB_LEN];

public:
    OtmProfileMgrDlg(void);
    ~OtmProfileMgrDlg(void);
    int OtmProfileMgrDlgOpen(HINSTANCE & hDllInst);

private:
    static INT_PTR CALLBACK OtmProfileMgrDlgProc(HWND hwndDlg, UINT msg, WPARAM mp1, LPARAM mp2);
    BOOL OnInitDialog();
    LRESULT OtmProfileMgrDlgCmd(WPARAM mp1, LPARAM mp2);
    BOOL ProfileMgrSheetLoad();
};

