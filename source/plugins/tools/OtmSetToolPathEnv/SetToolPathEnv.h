//+----------------------------------------------------------------------------+
//|SetToolPathEnv.h     OTM  Plugin Manager function                              |
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
//|                    during plugin manager parser                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
#pragma once

#include "stdafx.h"
#include "resource.h"
#include "OtmLogWriterEnv.h"

using namespace std;

typedef struct _CMDPARAM
{
    BOOL bAllUsers;
    BOOL bInstall;
    BOOL bBackup;
    char strInstDir[MAX_PATH];
    HINSTANCE hInstance;
} CMDPARAM, * PCMDPARAM;

class SetToolPathEnv
{
private:
    HINSTANCE m_hInstance;
    BOOL m_bInstall;                        // TRUE: install FALSE: uninstall
    BOOL m_bAllUsers;
    BOOL m_bBackup;
    char m_strInstDir[MAX_PATH];
    LogWriter m_logWritter;

public:
    int SetPathEnvironment();
    SetToolPathEnv(PCMDPARAM pCmdParam);
    SetToolPathEnv(void);
    ~SetToolPathEnv(void);

private:
    DWORD ProcessSystemPath();
    DWORD ProcessUserPath();
    DWORD GetEnvPathValue(HKEY hKey, LPDWORD pdwDataSize, LPDWORD pdwDataType, LPTSTR & lpValData);
    DWORD SetEnvPathValue(HKEY hKey, DWORD pdwDataSize, DWORD pdwDataType, LPTSTR & lpValData, BOOL bAdd);
    int BackupEnvironment(const char * strSubKey);
    char * GetTimeStampStr();
};

