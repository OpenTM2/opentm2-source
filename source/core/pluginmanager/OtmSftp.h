//+----------------------------------------------------------------------------+
//|OtmSftp.h     OTM Auto Version Up function                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
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

class __declspec(dllexport) COtmSftp
{
public:
    COtmSftp(void);
    ~COtmSftp(void);
    int DownloadFile(const char * strUrl, const char * strFile, PNETWORKPARAM pNetworkParam);
    void SetProcessDlg(HWND hwndProcessBar, int nID);
    void SetSFTPUsr(const char * strUsr);
    void SetSFTPPwd(const char * strPwd);
    int TestConnection(const char * strUrl, PNETWORKPARAM pNetworkParam);

private:
    char m_strUsr[MAX_PATH];
    char m_strPwd[MAX_PATH];
    PROCESSBAR m_processBar;
    int OtmDownloadData(const char * strUrl, const char * strFile, PNETWORKPARAM pNetworkParam);
    int CurlInit(CURL *& otmCurl, const char * strUrl, POTMDLFILE pDLFile, PNETWORKPARAM pNetworkParam);
    static size_t OtmWritter(void * otmData, size_t nSize, size_t nMemByte, void * otmStream);
    static int OtmProcessFunc(PPROCESSBAR progressBar, double dDLTotal, double dDLNow, double dULTotal, double dULNow);
};

