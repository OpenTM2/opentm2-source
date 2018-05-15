//+----------------------------------------------------------------------------+
//|OtmHttps.cpp     Http zip and unzip function                                |
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
#include "OtmHttps.h"

#define RETURN_IF_NULL(fp) \
    if(NULL == fp) \
    { \
	    nRC = ERROR_LOAD_ENTRY_POINT_A; \
		return nRC; \
    }

int COtmHttps::TestConnection(const char * strUrl, PNETWORKPARAM pNetworkParam)
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
	RETURN_IF_NULL(fpCurl_global_init)
    ccRet = (*fpCurl_global_init)(CURL_GLOBAL_DEFAULT);
    if (ccRet != CURLE_OK)
    {
        nRC = ERROR_CURL_INITIAL_A;
        return nRC;
    }

    PCURL_EASY_INIT fpCurl_easy_init = (PCURL_EASY_INIT) GetProcAddress(hDllLib, "curl_easy_init");
	RETURN_IF_NULL(fpCurl_easy_init)
    otmCurl = (*fpCurl_easy_init)();
    if (otmCurl == NULL)
    {
        nRC = ERROR_CURL_INITIAL_A;
        return nRC;
    }

    PCURL_EASY_SETOPT fpCurl_easy_setopt = (PCURL_EASY_SETOPT) GetProcAddress(hDllLib, "curl_easy_setopt");
	RETURN_IF_NULL(fpCurl_easy_setopt)
    ccRet = fpCurl_easy_setopt(otmCurl, CURLOPT_URL, strUrl);
    if (ccRet != CURLE_OK)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    ccRet = fpCurl_easy_setopt(otmCurl, CURLOPT_CONNECTTIMEOUT, pNetworkParam->nTimeout);
    if (ccRet != CURLE_OK)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    if ((NULL != pNetworkParam->strProxyAddress) && (strlen(pNetworkParam->strProxyAddress) != 0))
    {
        ccRet = fpCurl_easy_setopt(otmCurl, CURLOPT_PROXY, pNetworkParam->strProxyAddress);
        if (ccRet != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    if ((NULL != pNetworkParam->strProxyPort) && (strlen(pNetworkParam->strProxyPort) != 0))
    {
        ccRet = fpCurl_easy_setopt(otmCurl,  CURLOPT_PROXYPORT, atoi(pNetworkParam->strProxyPort));
        if (ccRet != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    ccRet = fpCurl_easy_setopt(otmCurl, CURLOPT_VERBOSE, 1L);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    // connect only
    ccRet = fpCurl_easy_setopt(otmCurl, CURLOPT_CONNECT_ONLY, 1L);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    // for HTTPs
    // skip the verification of the server's certificate
    ccRet = fpCurl_easy_setopt(otmCurl, CURLOPT_SSL_VERIFYPEER, FALSE); // https
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    // skip the host verification
    ccRet = fpCurl_easy_setopt(otmCurl, CURLOPT_SSL_VERIFYHOST, FALSE);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    /*ccRet = curl_easy_setopt(otmCurl, CURLOPT_SSLVERSION, 3);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }*/

    // set error message
    if (NULL != pNetworkParam->strError)
    {
        ccRet = fpCurl_easy_setopt(otmCurl, CURLOPT_ERRORBUFFER, pNetworkParam->strError);
        if (ccRet != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    PCURL_EASY_PERFORM fpCurl_easy_perform = (PCURL_EASY_PERFORM) GetProcAddress(hDllLib, "curl_easy_perform");
	if(fpCurl_easy_perform != NULL)
	{
		ccRet = fpCurl_easy_perform(otmCurl);
		if (ccRet != CURLE_OK)
		{
			nRC = ERROR_HTTPS_CONNECT_A;
		}
	}

    PCURL_EASY_CLEANUP fpCurl_easy_cleanup = (PCURL_EASY_CLEANUP) GetProcAddress(hDllLib, "curl_easy_cleanup");
	if(fpCurl_easy_cleanup != NULL)
		(*fpCurl_easy_cleanup)(otmCurl);

    PCURL_GLOBAL_CLEANUP fpCurl_global_cleanup = (PCURL_GLOBAL_CLEANUP) GetProcAddress(hDllLib, "curl_global_cleanup");
	if(fpCurl_global_cleanup != NULL)
        (*fpCurl_global_cleanup)();

    FreeLibrary(hDllLib);

    return nRC;
}

int COtmHttps::DownloadFile(const char * strUrl, const char * strFile, PNETWORKPARAM pNetworkParam)
{
    return OtmDownloadData(strUrl, strFile, pNetworkParam);
}

int COtmHttps::OtmDownloadData(const char * strUrl, const char * strFile, PNETWORKPARAM pNetworkParam)
{
    CURL * otmCurl = NULL;
    CURLcode ccRet;
    int nRC = NO_ERROR;

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
	RETURN_IF_NULL(fpCurl_easy_perform)
    ccRet = (*fpCurl_easy_perform)(otmCurl);
    if (ccRet != CURLE_OK)
    {
        nRC = ERROR_HTTPS_DOWNLOAD_A;
        remove(strFile);
    }
    else
    {
        if (otmDLFile.pDLFile)
        {
            fclose(otmDLFile.pDLFile); // close the local file
        }
    }

    PCURL_EASY_CLEANUP fpCurl_easy_cleanup = (PCURL_EASY_CLEANUP) GetProcAddress(hDllLib, "curl_easy_cleanup");
	if(fpCurl_easy_cleanup != NULL)
        (*fpCurl_easy_cleanup)(otmCurl);

    PCURL_GLOBAL_CLEANUP fpCurl_global_cleanup = (PCURL_GLOBAL_CLEANUP) GetProcAddress(hDllLib, "curl_global_cleanup");
    if(fpCurl_global_cleanup != NULL)
		fpCurl_global_cleanup();

    FreeLibrary(hDllLib);

    return nRC;
}

int COtmHttps::CurlInit(CURL *& otmCurl, const char* strUrl, POTMDLFILE pDLFile, PNETWORKPARAM pNetworkParam)
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
	RETURN_IF_NULL(fpCurl_global_init)
    ccRet = (*fpCurl_global_init)(CURL_GLOBAL_DEFAULT);
    if (ccRet != CURLE_OK)
    {
        nRC = ERROR_CURL_INITIAL_A;
        return nRC;
    }

    PCURL_EASY_INIT fpCurl_easy_init = (PCURL_EASY_INIT) GetProcAddress(hDllLib, "curl_easy_init");
	RETURN_IF_NULL(fpCurl_easy_init)
    otmCurl = (*fpCurl_easy_init)();
    if (otmCurl == NULL)
    {
        nRC = ERROR_CURL_INITIAL_A;
        return nRC;
    }

    PCURL_EASY_SETOPT fpCurl_easy_setopt = (PCURL_EASY_SETOPT) GetProcAddress(hDllLib, "curl_easy_setopt");
	RETURN_IF_NULL(fpCurl_easy_setopt)
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

    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_VERBOSE, 1L);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    // for HTTPs
    /*ccRet = curl_easy_setopt(otmCurl, CURLOPT_CAINFO, "localhost.crt");
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }*/

    // for HTTPs
    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_SSL_VERIFYPEER, 0L); // https
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

    // initial for progress control
    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_NOPROGRESS, 0L);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    /*ccRet = curl_easy_setopt(otmCurl, CURLOPT_SSLVERSION, 3);
    if (CURLE_OK != ccRet)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }*/

    ccRet = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_FAILONERROR, TRUE);
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

    FreeLibrary(hDllLib);

    return nRC;
}

size_t COtmHttps::OtmWritter(void * otmData, size_t nSize, size_t nMemByte, void * otmStream)
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

void COtmHttps::SetProcessDlg(HWND hwndProcessBar, int nID)
{
    m_processBar.hwndProcessBar = hwndProcessBar;
    m_processBar.nID = nID;
}

int COtmHttps::OtmProcessFunc(PPROCESSBAR progressBar,
                     double dDLTotal, double dDLNow, 
                     double dULTotal, double dULNow)
{
    int nPos = (int) (dDLNow / dDLTotal * 10000);
    SendDlgItemMessage(progressBar->hwndProcessBar, progressBar->nID, PBM_SETPOS, (WPARAM) nPos, 0);
    return 0;
}

void COtmHttps::SetLocCaInfo(char * strLocCaInfo)
{
    if ((NULL == strLocCaInfo) || (strlen(strLocCaInfo) == 0))
    {
        return;
    }

    int nLen = min(sizeof(m_strLocCaInfo), strlen(strLocCaInfo));
    strncpy(m_strLocCaInfo, strLocCaInfo, nLen);
}

COtmHttps::COtmHttps(void)
{
    memset(m_strLocCaInfo, 0x00, sizeof(m_strLocCaInfo));
}

COtmHttps::~COtmHttps(void)
{
}
