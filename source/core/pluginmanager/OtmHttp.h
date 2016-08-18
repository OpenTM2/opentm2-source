//+----------------------------------------------------------------------------+
//|OtmHttp.h     OTM Auto Version Up function                                  |
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

//#define CURL_STATICLIB  //HTTP_ONLY
#pragma once

#include "core\pluginmanager\OtmComm.h"

//#if defined(_DEBUG)
//#pragma comment(lib, "libcurld.lib")
//#else
//#pragma comment(lib, "libcurl.lib")
//#endif
//#pragma comment ( lib, "ws2_32.lib" )
//#pragma comment ( lib, "winmm.lib" )
//#pragma comment ( lib, "wldap32.lib" )


class __declspec(dllexport) COtmHttp
{
public:
    COtmHttp(void);
    ~COtmHttp(void);
    int DownloadFile(const char * strUrl, const char * strFile, PNETWORKPARAM pNetworkParam);
    void SetProcessDlg(HWND hwndProcessBar, int nID);
    int TestConnection(const char * strUrl, PNETWORKPARAM pNetworkParam);

private:
    PROCESSBAR m_processBar;
    int OtmDownloadData(const char * strUrl, const char * strFile, PNETWORKPARAM pNetworkParam);
    int CurlInit(CURL *& otmCurl, const char * strUrl, POTMDLFILE pDLFile, PNETWORKPARAM pNetworkParam);
    static size_t OtmWritter(void * otmData, size_t nSize, size_t nMemByte, void * otmStream);
    static int OtmProcessFunc(PPROCESSBAR progressBar, double dDLTotal, double dDLNow, double dULTotal, double dULNow);
};

