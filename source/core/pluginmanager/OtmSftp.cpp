//+----------------------------------------------------------------------------+
//|OtmSftp.cpp     Http zip and unzip function                                 |
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
#include "OtmSftp.h"

#define RETURN_IF_NULL(fp) \
    if(NULL == fp) \
    { \
	    nRC = ERROR_LOAD_ENTRY_POINT_A; \
		return nRC; \
    }

int COtmSftp::TestConnection(const char * strUrl, PNETWORKPARAM pNetworkParam)
{
    CURL * otmCurl  = NULL;
    CURLcode ccRC   = CURLE_OK;
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
    ccRC = (*fpCurl_global_init)(CURL_GLOBAL_DEFAULT);
    if (ccRC != CURLE_OK)
    {
        nRC = ERROR_CURL_INITIAL_A;
        return nRC;
    }

    PCURL_EASY_INIT fpCurl_easy_init = (PCURL_EASY_INIT) GetProcAddress(hDllLib, "curl_easy_init");
	RETURN_IF_NULL(fpCurl_easy_init)
    otmCurl = (*fpCurl_easy_init)();
    if (!otmCurl)
    {
        nRC = ERROR_CURL_INITIAL_A;
        return nRC;
    }

    PCURL_EASY_SETOPT fpCurl_easy_setopt = (PCURL_EASY_SETOPT) GetProcAddress(hDllLib, "curl_easy_setopt");
	RETURN_IF_NULL(fpCurl_easy_setopt)
    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_URL, strUrl);
    if (ccRC != CURLE_OK)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    /*ccRC = curl_easy_setopt(otmCurl, CURLOPT_CONNECTTIMEOUT, pNetworkParam->nTimeout);
    if (ccRC != CURLE_OK)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }*/

    if ((NULL != pNetworkParam->strProxyAddress) && (strlen(pNetworkParam->strProxyAddress) != 0))
    {
        ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_PROXY, pNetworkParam->strProxyAddress);
        if (ccRC != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    if ((NULL != pNetworkParam->strProxyPort) && (strlen(pNetworkParam->strProxyPort) != 0))
    {
        ccRC = (*fpCurl_easy_setopt)(otmCurl,  CURLOPT_PROXYPORT, atoi(pNetworkParam->strProxyPort));
        if (ccRC != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_VERBOSE, 1L);
    if (CURLE_OK != ccRC)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    // for SFTP
    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
    if (CURLE_OK != ccRC)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    // set connect only
    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_CONNECT_ONLY, 1L);
    if (CURLE_OK != ccRC)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    // for SFTP
    char strUsrPwd[MAX_PATH*2];
    memset(strUsrPwd, 0x00, sizeof(strUsrPwd));
    sprintf(strUsrPwd, "%s:%s", m_strUsr, m_strPwd);
    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_USERPWD, strUsrPwd);
    if (CURLE_OK != ccRC)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    // set error info
    if (NULL != pNetworkParam->strError)
    {
        ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_ERRORBUFFER, pNetworkParam->strError);
        if (ccRC != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    PCURL_EASY_PERFORM fpCurl_easy_perform = (PCURL_EASY_PERFORM) GetProcAddress(hDllLib, "curl_easy_perform");
	if(fpCurl_easy_perform != NULL)
	{
		ccRC = (*fpCurl_easy_perform)(otmCurl);
		if (ccRC != CURLE_OK)
		{
			nRC = ERROR_SFTP_CONNECT_A;
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

int COtmSftp::DownloadFile(const char * strUrl, const char * strFile, PNETWORKPARAM pNetworkParam)
{
    return OtmDownloadData(strUrl, strFile, pNetworkParam);
}

int COtmSftp::OtmDownloadData(const char * strUrl, const char * strFile, PNETWORKPARAM pNetworkParam)
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
    ccRC = (*fpCurl_easy_perform)(otmCurl);
    if (ccRC != CURLE_OK)
    {
        nRC = ERROR_SFTP_DOWNLOAD_A;
    }
    else
    {
        if (otmDLFile.pDLFile)
        {
            fclose(otmDLFile.pDLFile); // close the local file
        }
    }

    PCURL_EASY_CLEANUP fpCurl_easy_cleanup = (PCURL_EASY_CLEANUP) GetProcAddress(hDllLib, "curl_easy_cleanup");
	if(fpCurl_easy_cleanup!=NULL)
        (*fpCurl_easy_cleanup)(otmCurl);

    PCURL_GLOBAL_CLEANUP fpCurl_global_cleanup = (PCURL_GLOBAL_CLEANUP) GetProcAddress(hDllLib, "curl_global_cleanup");
	if(fpCurl_global_cleanup!=NULL)
        (*fpCurl_global_cleanup)();

    FreeLibrary(hDllLib);

    return nRC;
}

int COtmSftp::CurlInit(CURL *& otmCurl, const char * strUrl, POTMDLFILE pDLFile, PNETWORKPARAM pNetworkParam)
{
    CURLcode ccRC  = CURLE_OK;
    int nRC        = NO_ERROR;

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
    ccRC = (*fpCurl_global_init)(CURL_GLOBAL_DEFAULT);
    if (ccRC != CURLE_OK)
    {
        nRC = ERROR_CURL_INITIAL_A;
        return nRC;
    }

    PCURL_EASY_INIT fpCurl_easy_init = (PCURL_EASY_INIT) GetProcAddress(hDllLib, "curl_easy_init");
	RETURN_IF_NULL(fpCurl_easy_init)
    otmCurl = (*fpCurl_easy_init)();
    if (!otmCurl)
    {
        nRC = ERROR_CURL_INITIAL_A;
        return nRC;
    }

    PCURL_EASY_SETOPT fpCurl_easy_setopt = (PCURL_EASY_SETOPT) GetProcAddress(hDllLib, "curl_easy_setopt");
	RETURN_IF_NULL(fpCurl_easy_setopt)
    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_URL, strUrl);
    if (ccRC != CURLE_OK)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    /*ccRC = curl_easy_setopt(otmCurl, CURLOPT_CONNECTTIMEOUT, pNetworkParam->nTimeout);
    if (ccRC != CURLE_OK)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }*/

    if ((NULL != pNetworkParam->strProxyAddress) && (strlen(pNetworkParam->strProxyAddress) != 0))
    {
        ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_PROXY, pNetworkParam->strProxyAddress);
        if (ccRC != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    if ((NULL != pNetworkParam->strProxyPort) && (strlen(pNetworkParam->strProxyPort) != 0))
    {
        ccRC = (*fpCurl_easy_setopt)(otmCurl,  CURLOPT_PROXYPORT, atoi(pNetworkParam->strProxyPort));
        if (ccRC != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_WRITEFUNCTION, OtmWritter);
    if (CURLE_OK != ccRC)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_WRITEDATA, pDLFile);
    if (CURLE_OK != ccRC)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_VERBOSE, 1L);
    if (CURLE_OK != ccRC)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    // for SFTP
    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
    if (CURLE_OK != ccRC)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    // for SFTP
    char strUsrPwd[MAX_PATH*2];
    memset(strUsrPwd, 0x00, sizeof(strUsrPwd));
    sprintf(strUsrPwd, "%s:%s", m_strUsr, m_strPwd);
    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_USERPWD, strUsrPwd);
    if (CURLE_OK != ccRC)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    // initial for progress control
    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_NOPROGRESS, 0L);
    if (CURLE_OK != ccRC)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_PROGRESSFUNCTION, OtmProcessFunc);
    if (CURLE_OK != ccRC)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_PROGRESSDATA, &m_processBar);
    if (CURLE_OK != ccRC)
    {
        nRC = ERROR_CURL_SETOPT_A;
        return nRC;
    }

    // set error info
    if (NULL != pNetworkParam->strError)
    {
        ccRC = (*fpCurl_easy_setopt)(otmCurl, CURLOPT_ERRORBUFFER, pNetworkParam->strError);
        if (ccRC != CURLE_OK)
        {
            nRC = ERROR_CURL_SETOPT_A;
            return nRC;
        }
    }

    FreeLibrary(hDllLib);

    return nRC;
}

size_t COtmSftp::OtmWritter(void * otmData, size_t nSize, size_t nMemByte, void * otmStream)
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

void COtmSftp::SetProcessDlg(HWND hwndProcessBar, int nID)
{
    m_processBar.hwndProcessBar = hwndProcessBar;
    m_processBar.nID = nID;
}

int COtmSftp::OtmProcessFunc(PPROCESSBAR progressBar,
                     double dDLTotal, double dDLNow, 
                     double dULTotal, double dULNow)
{
    int nPos = (int) (dDLNow / dDLTotal * 10000);
    SendDlgItemMessage(progressBar->hwndProcessBar, progressBar->nID, PBM_SETPOS, (WPARAM) nPos, 0);
    return 0;
}

void COtmSftp::SetSFTPUsr(const char * strUsr)
{
    if ((NULL == strUsr) || (strlen(strUsr) == 0))
    {
        return;
    }

    int nLen = min(sizeof(m_strUsr), strlen(strUsr));
    strncpy(m_strUsr, strUsr, nLen);
}

void COtmSftp::SetSFTPPwd(const char * strPwd)
{
    if ((NULL == strPwd) || (strlen(strPwd) == 0))
    {
        return;
    }

    int nLen = min(sizeof(m_strPwd), strlen(strPwd));
    strncpy(m_strPwd, strPwd, nLen);
}

COtmSftp::COtmSftp(void)
{
    memset(m_strUsr, 0x00, sizeof(m_strUsr));
    memset(m_strPwd, 0x00, sizeof(m_strPwd));
}

COtmSftp::~COtmSftp(void)
{
}
