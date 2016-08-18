//+----------------------------------------------------------------------------+
//|OtmSftpConfig.h     OTM Auto Version Up function                            |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2013, International Business Machines          |
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

#include "core\pluginmanager\OtmComm.h"

#define KEY_SFTP_URL_AVU                              "URL_AVU"
#define KEY_SFTP_URL_PLUGINS                          "URL_PLUGINS"
#define KEY_SFTP_USR                                  "User"
#define KEY_SFTP_PWD                                  "Pwd"
#define KEY_VER                                       "Ver"
#define KEY_PROXY_ADDRESS                             "ProxyAddress"
#define KEY_PROXY_PORT                                "ProxyPort"

class __declspec(dllexport) COtmSftpConfig
{
private:
    char m_strAVUUrl[MAX_BUF_SIZE];
    char m_strPlgMgGUIUrl[MAX_BUF_SIZE];
    char m_strUsr[MAX_BUF_SIZE];
    char m_strPwd[MAX_BUF_SIZE];
    char m_strVer[MAX_LEN];
    char m_strProxyAddress[MAX_PROXY_ADDRESS_LEN];
    char m_strProxyPort[MAX_PROXY_PORT_LEN];

public:
    COtmSftpConfig(void);
    ~COtmSftpConfig(void);
    int ParseConfigFile(PBYTE strConfigBuffer);
    void AnalysisSentense(const char * strSetense);
    const char * GetAVUUrl();
    const char * GetPlgMgGUIUrl();
    const char * GetUsr();
    const char * GetPwd();
    const char * GetVer();
    const char * GetProxyAddress();
    const char * GetProxyPort();
};

