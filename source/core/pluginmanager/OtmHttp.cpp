//+----------------------------------------------------------------------------+
//|OtmHttp.cpp     Http zip and unzip function                                 |
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
//|                    during http upload, download, zip and unzip packages.   |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
#include "OtmHttp.h"

int COtmHttp::TestConnection(const char * strUrl, PNETWORKPARAM pNetworkParam)
{
    CURL * otmCurl  = NULL;
    CURLcode ccRet  = CURLE_OK;
    int nRC         = NO_ERROR;

    char strDllPath[MAX_PATH];
    memset(strDllPath, 0x00, sizeof(strDllPath));
    GetLibcurlPath(strDllPath);

    HMODULE hDllLib = LoadLibrary(strDllPath);

    if (NULL == hDllLib)
    {
        nRC = ERROR_LOAD_DLL_LIBCURL_A;
        return nRC;
    }

    PCURL_GLOBAL_INIT fpCurl_global_init = (PCURL_GLOBAL_INIT) GetProcAddress(hDllLib, "curl_global_init");
    ccRet = (*fpCurl_global_init)(CURL_GLOBAL_DEFAULT);
    if (ccRet != CURLE_OK)
    {
        nRC = ERROR_CURL_INITIAL_A;
        return nRC;
    }

    PCURL_EASY_INIT fpCurl_easy_init = (PCURL_EASY_INIT) GetProcAddress(hDllLib, "curl_easy_init");
    otmCurl = (*fpCurl_easy_init)();
    if (otmCurl == NULL)
    {
        nRC = ERROR_CURL_INITIAL_A;
        return nRC;
    }

    PCURL_EASY_SETOPT fpCurl_easy_setopt = (PCURL_EASY_SETOPT) GetProcAddress(hDllLib, "curl_easy_setopt");
//    code = curl_easy_setopt(otmCurl, CURLOPT_ERRORBUFFER, error);
    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_CONNECTTIMEOUT, pNetworkParam->nTimeout);
    if (ccRet != CURLE_OK)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_URL, strUrl);
    if (ccRet != CURLE_OK)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    if ((NULL != pNetworkParam->strProxyAddress) && (strlen(pNetworkParam->strProxyAddress) != 0))
    {
        ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_PROXY, pNetworkParam->strProxyAddress);
        if (ccRet != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    // connect only
    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_CONNECT_ONLY, 1L);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    if ((NULL != pNetworkParam->strProxyPort) && (strlen(pNetworkParam->strProxyPort) != 0))
    {
        ccRet = (*fpCurl_easy_setopt)(otmCurl,  CURLOPT_PROXYPORT, atoi(pNetworkParam->strProxyPort));
        if (ccRet != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    // set error message
    if (NULL != pNetworkParam->strError)
    {
        ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_ERRORBUFFER, pNetworkParam->strError);
        if (ccRet != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    PCURL_EASY_PERFORM fpCurl_easy_perform = (PCURL_EASY_PERFORM) GetProcAddress(hDllLib, "curl_easy_perform");
    ccRet = (*fpCurl_easy_perform)(otmCurl);
    if (ccRet != CURLE_OK)
    {
        nRC = ERROR_HTTPS_CONNECT_A;
    }

    PCURL_EASY_CLEANUP fpCurl_easy_cleanup = (PCURL_EASY_CLEANUP) GetProcAddress(hDllLib, "curl_easy_cleanup");
    (*fpCurl_easy_cleanup)(otmCurl);

    PCURL_GLOBAL_CLEANUP fpCurl_global_cleanup = (PCURL_GLOBAL_CLEANUP) GetProcAddress(hDllLib, "curl_global_cleanup");
    (*fpCurl_global_cleanup)();

    FreeLibrary(hDllLib);

    return nRC;
}

int COtmHttp::DownloadFile(const char * strUrl, const char * strFile, PNETWORKPARAM pNetworkParam)
{
    return OtmDownloadData(strUrl, strFile, pNetworkParam);
}

int COtmHttp::OtmDownloadData(const char * strUrl, const char * strFile, PNETWORKPARAM pNetworkParam)
{
    CURL * otmCurl  = NULL;
    CURLcode ccRC   = CURLE_OK;
    int nRC         = NO_ERROR;

    OTMDLFILE otmDLFile = {
        strFile,
        NULL
    };

    // do some initial for curl
    nRC = CurlInit(otmCurl, strUrl, &otmDLFile, pNetworkParam);
    if (nRC != NO_ERROR)
    {
        return nRC;
    }

//    code = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
    char strDllPath[MAX_PATH];
    memset(strDllPath, 0x00, sizeof(strDllPath));
    GetLibcurlPath(strDllPath);

    HMODULE hDllLib = LoadLibrary(strDllPath);

    if (NULL == hDllLib)
    {
        nRC = ERROR_LOAD_DLL_LIBCURL_A;
        return nRC;
    }

    // perform the download
    PCURL_EASY_PERFORM fpCurl_easy_perform = (PCURL_EASY_PERFORM) GetProcAddress(hDllLib, "curl_easy_perform");
    ccRC = (*fpCurl_easy_perform)(otmCurl);
    if (ccRC != CURLE_OK)
    {
        nRC = ERROR_HTTP_DOWNLOAD_A;
    }
    else
    {
        if (otmDLFile.pDLFile)
        {
            fclose(otmDLFile.pDLFile); // close the local file
        }
    }

    PCURL_EASY_CLEANUP fpCurl_easy_cleanup = (PCURL_EASY_CLEANUP) GetProcAddress(hDllLib, "curl_easy_cleanup");
    (*fpCurl_easy_cleanup)(otmCurl);

    PCURL_GLOBAL_CLEANUP fpCurl_global_cleanup = (PCURL_GLOBAL_CLEANUP) GetProcAddress(hDllLib, "curl_global_cleanup");
    (*fpCurl_global_cleanup)();

    FreeLibrary(hDllLib);

    return nRC;
}

int COtmHttp::CurlInit(CURL *& otmCurl, const char * strUrl, POTMDLFILE pDLFile, PNETWORKPARAM pNetworkParam)
{
    CURLcode ccRet  = CURLE_OK;
    int nRC         = NO_ERROR;

    char strDllPath[MAX_PATH];
    memset(strDllPath, 0x00, sizeof(strDllPath));
    GetLibcurlPath(strDllPath);

    HMODULE hDllLib = LoadLibrary(strDllPath);

    if (NULL == hDllLib)
    {
        nRC = ERROR_LOAD_DLL_LIBCURL_A;
        return nRC;
    }

    PCURL_GLOBAL_INIT fpCurl_global_init = (PCURL_GLOBAL_INIT) GetProcAddress(hDllLib, "curl_global_init");
    ccRet = (*fpCurl_global_init)(CURL_GLOBAL_DEFAULT);
    if (ccRet != CURLE_OK)
    {
        nRC = ERROR_CURL_INITIAL_A;
        return nRC;
    }

    PCURL_EASY_INIT fpCurl_easy_init = (PCURL_EASY_INIT) GetProcAddress(hDllLib, "curl_easy_init");
    otmCurl = (*fpCurl_easy_init)();
    if (otmCurl == NULL)
    {
        nRC = ERROR_CURL_INITIAL_A;
        return nRC;
    }

//    code = curl_easy_setopt(otmCurl, CURLOPT_ERRORBUFFER, error);
    PCURL_EASY_SETOPT fpCurl_easy_setopt = (PCURL_EASY_SETOPT) GetProcAddress(hDllLib, "curl_easy_setopt");
    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_CONNECTTIMEOUT, pNetworkParam->nTimeout);
    if (ccRet != CURLE_OK)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_URL, strUrl);
    if (ccRet != CURLE_OK)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    if ((NULL != pNetworkParam->strProxyAddress) && (strlen(pNetworkParam->strProxyAddress) != 0))
    {
        ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_PROXY, pNetworkParam->strProxyAddress);
        if (ccRet != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    if ((NULL != pNetworkParam->strProxyPort) && (strlen(pNetworkParam->strProxyPort) != 0))
    {
        ccRet = (*fpCurl_easy_setopt)(otmCurl,  CURLOPT_PROXYPORT, atoi(pNetworkParam->strProxyPort));
        if (ccRet != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_WRITEFUNCTION, OtmWritter);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_WRITEDATA, pDLFile);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

//    curl_easy_setopt(otmCurl, CURLOPT_SSL_VERIFYPEER, true);
//    curl_easy_setopt (otmCurl, CURLOPT_CAINFO, "ca_bundle.crt");
    // for ssl
    /*curl_easy_setopt(otmCurl, CURLOPT_SSL_VERIFYPEER, FALSE);
    curl_easy_setopt(otmCurl, CURLOPT_SSL_VERIFYHOST, FALSE);
    curl_easy_setopt(otmCurl, CURLOPT_USERPWD, "alstmb:43refdvc");*/

    // initial for progress control
    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_NOPROGRESS, 0L);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_PROGRESSFUNCTION, OtmProcessFunc);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_PROGRESSDATA, &m_processBar);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    FreeLibrary(hDllLib);

    return nRC;
}

size_t COtmHttp::OtmWritter(void * otmData, size_t nSize, size_t nMemByte, void * otmStream)
{
    POTMDLFILE otmTemp = (POTMDLFILE) otmStream;

    if (otmTemp && (!otmTemp->pDLFile))
    {
        // open the file and then writing
        otmTemp->pDLFile = fopen(otmTemp->strFile, "wb+");
        if(!otmTemp->pDLFile)
        {
            // failure, can't open file to write
            return (size_t) DEF_ERROR_SIZE;
        }
    }

    return fwrite(otmData, nSize, nMemByte, otmTemp->pDLFile);
}

void COtmHttp::SetProcessDlg(HWND hwndProcessBar, int nID)
{
    m_processBar.hwndProcessBar = hwndProcessBar;
    m_processBar.nID = nID;
}

int COtmHttp::OtmProcessFunc(PPROCESSBAR progressBar,
                     double dDLTotal, double dDLNow, 
                     double dULTotal, double dULNow)
{
    int nPos = (int) (dDLNow / dDLTotal * 10000);
    SendDlgItemMessage(progressBar->hwndProcessBar, progressBar->nID, PBM_SETPOS, (WPARAM) nPos, 0);
    return 0;
}

COtmHttp::COtmHttp(void)
{
}

COtmHttp::~COtmHttp(void)
{
}
