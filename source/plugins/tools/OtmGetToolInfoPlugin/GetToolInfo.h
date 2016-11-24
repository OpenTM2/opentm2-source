//+----------------------------------------------------------------------------+
//|GetToolInfo.h     OTM  Plugin Manager function                              |
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

#include "..\\..\\..\\..\\include\\eqf.h"
#include "..\\..\\..\\..\\mri\\eqfstart.id"

#include "CommStr.h"
#include "resource.h"
#include "..\\..\\..\\core\\pluginmanager\\OtmComm.h"
#include "..\\..\\..\\core\\pluginmanager\\PluginManager.h"
#include <vector>
#include <string>

using namespace std;

typedef vector <string> STRINGGRP, *PSTRINGGRP;

#define OPENTM2_APP_NAME_STR                          "OpenTM2"
#define KEY_PATH                                      "Path"                          // key for the OpenTM2 system path
#define KEY_DRIVE                                     "Drive"                         // key for the OpenTM2 system drive

#define OTM_FOLDER_KEY                                "OTM"
#define PLUGIN_FOLDER_KEY                             "PLUGINS"
#define WIN_FOLDER_KEY                                "WIN"
#define RES_DLL_KEY                                   "OTMRESWE.DLL"
#define LOG_FOLDER_KEY                                "LOGS"
#define LOG_FILE_NAME                                 "OTMGetToolInfo.txt"

#define OPTION_DETAILS                                "DETAILS"

typedef struct _CMDPARAM
{
    BOOL bCmd;
    BOOL bDetails;
    HINSTANCE hInstance;
} CMDPARAM, * PCMDPARAM;

BOOL CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class GetToolInfo
{
private:
    BOOL m_bDetails;
    BOOL m_bCmd;
    HINSTANCE m_hInstance;
    BOOL m_bWow64;
    DWORD m_dwMajVer;
    char m_strAppVer[MAX_PATH];
    char m_strAppPath[MAX_PATH];
    char m_strPluginPath[MAX_PATH];
    char m_strAppMainPath[MAX_PATH];
    char m_strRptPath[MAX_PATH];
    char m_strRptFile[MAX_PATH];
    char m_strMsgFile[MAX_PATH];
    char m_strResPath[MAX_PATH];

    // checking for registry entries
    BOOL m_bHasLocMacUni;
    BOOL m_bHasLocMac;
    BOOL m_bHasCurUsr;
    BOOL m_bHasCurUsrVirUni;
    BOOL m_bHasCurUsrVir;

    char m_strLocMacUni[MAX_BUF_SIZE];
    char m_strLocMac[MAX_BUF_SIZE];
    char m_strCurUsr[MAX_BUF_SIZE];
    char m_strCurUsrVirUni[MAX_BUF_SIZE];
    char m_strCurUsrVir[MAX_BUF_SIZE];

public:
    int ShowToolInfo();
    int ShowToolInfoByUI();
    GetToolInfo(PCMDPARAM pCmdParam);
    GetToolInfo(void);
    ~GetToolInfo(void);

private:
    void Join2Path(const char * strInPath1, const char * strInPath2, char * strPathOut);
    int DoInitialize();
    const char * GetSystemName();
    const char * GetDateTimeStr();
    BOOL SetupUtils(HAB hab, PSZ  pMsgFile);
    void CopyString(char * strTar, const char * strSrc);
    BOOL OtmIsWow64();
    int GetAppVersion();
    int GetToolAppPath();
    void GetModuleAppPath(const char * strModule);
    int GetValFromReg(HKEY hKey, const char * strSubKey, const char * strKey, char * strValue);
    int GetValFromRegEx(HKEY hStartKey, const char * strSubKey, const char * strKey, LPSTR & strValue);
    void ShowCmdHeader();
    void ShowRptHeader();
    int ShowSystemInfo();
    int ShowNetworkInfo();
    int ShowEnvValue();
    int ShowRevisionInfo();
    int ShowDrivesInfo();
    int ShowMemoriesInfo();
    int ShowToolRelatedInfo();
    int ShowPluginsInfo();
    int ShowOTMRegInfo();
    int ShowOtmWinFolder();
    int ShowFileName(const char * strStartPath);
    void ShowWinErrMsg(DWORD dRet);
    void ShowErrMsg(const char * strErrMsg);
    void ShowInfoMsg(const char * strMsg);
    void EnumQueryKey(HKEY hKey, char * strParent, const char * strTarKey);
    void EnumKeyValues(HKEY hKey);
    void AddToLog(const char * strMsg);
    void AddComma(const char * strOri, char * strTar);
    void ShowDriveInfo(const char * strDrives);
    int ShowPluginFolder();
    int ShowPluginPaths(const char * strStartPath);
    int ShowPluginPath(const char * strStartPath, STRINGGRP & vecSubDir);
    int ShowFileDetails(const char * strStartPath);
    int ShowPluginInfo(const char* pszName);
    BOOL CheckDllName(const char * strFullName );
    int ScanPluginDir( const char *pszPluginDir, int iDepth );
};

