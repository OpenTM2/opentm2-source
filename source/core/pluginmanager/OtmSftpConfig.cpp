//+----------------------------------------------------------------------------+
//|COtmSftpConfig.cpp     Auto Version Up function                             |
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
#include "OtmSftpConfig.h"

int COtmSftpConfig::ParseConfigFile(PBYTE strConfigBuffer)
{
    int nRC = NO_ERROR;
    char * strBuffer = (char *) strConfigBuffer;
    if ((NULL == strBuffer) || (strlen(strBuffer) == 0))
    {
        nRC = ERROR_EMPTY_FILE_A;
        return nRC;
    }

    char strSetense[MAX_BUF_SIZE];
    memset(strSetense, 0x00, sizeof(strSetense));

    int jInx = 0;
    int nLen = (int) strlen(strBuffer);

    BOOL bAnalysis = FALSE;
    for (int iInx = 0; iInx <= nLen; iInx++)
    {
        if ((strBuffer[iInx] != '\r') && (strBuffer[iInx] != '\n') && (strBuffer[iInx] >= 0))
        {
            strSetense[jInx] = strBuffer[iInx];
            jInx++;
        }
        else if (strBuffer[iInx] == '\r')
        {
            if (strBuffer[iInx+1] == '\n')
            {
                iInx++;
                bAnalysis = TRUE;
            }
        }
        else if (strBuffer[iInx+1] == '\n')
        {
            bAnalysis = TRUE;
        }
        else if ((iInx == nLen) || (strBuffer[iInx] < 0))
        {
            bAnalysis = TRUE;
        }

        if (bAnalysis)
        {
            strSetense[jInx] = '\0';
            /*char strMsg[MAX_BUF_SIZE];
            memset(strMsg, 0x00, sizeof(strMsg));
            sprintf(strMsg, "%d,%s", strSetense[jInx-1], strSetense);
            MessageBox(HWND_DESKTOP, strSetense, "hello 8", MB_OK);*/
            AnalysisSentense(strSetense);
            memset(strSetense, 0x00, sizeof(strSetense));
            jInx = 0;
            bAnalysis = FALSE;
        }

        if (strBuffer[iInx] < 0)
        {
            break;
        }
    }
    return nRC;
}

void COtmSftpConfig::AnalysisSentense(const char * strSetense)
{
    // skip empty setense
    if ((NULL == strSetense) || (strlen(strSetense) == 0))
    {
        return;
    }

    // skip comment
    if ('#' == strSetense[0])
    {
        return;
    }

    char strKey[MAX_BUF_SIZE];
    char strVal[MAX_BUF_SIZE];

    memset(strKey, 0x00, sizeof(strKey));
    memset(strVal, 0x00, sizeof(strVal));

    OtmGetKeyValue(strSetense, strKey, strVal, '=', FALSE);

    if ((NULL == strKey) || strlen(strKey) == 0)
    {
        return;
    }

    if (!strnicmp(strKey, KEY_SFTP_URL_AVU, strlen(KEY_SFTP_URL_AVU)))
    {
        strncpy(m_strAVUUrl, strVal, strlen(strVal));
    }
    else if (!strnicmp(strKey, KEY_SFTP_URL_PLUGINS, strlen(KEY_SFTP_URL_PLUGINS)))
    {
        strncpy(m_strPlgMgGUIUrl, strVal, strlen(strVal));
    }
    else if (!strnicmp(strKey, KEY_SFTP_USR, strlen(KEY_SFTP_USR)))
    {
        strncpy(m_strUsr, strVal, strlen(strVal));
    }
    else if (!strnicmp(strKey, KEY_SFTP_PWD, strlen(KEY_SFTP_PWD)))
    {
        strncpy(m_strPwd, strVal, strlen(strVal));
    }
    else if (!strnicmp(strKey, KEY_VER, strlen(KEY_VER)))
    {
        strncpy(m_strVer, strVal, strlen(strVal));
    }
    else if (!strnicmp(strKey, KEY_PROXY_ADDRESS, strlen(KEY_PROXY_ADDRESS)))
    {
        strncpy(m_strProxyAddress, strVal, strlen(strVal));
    }
    else if (!strnicmp(strKey, KEY_VER, strlen(KEY_VER)))
    {
        strncpy(m_strProxyPort, strVal, strlen(strVal));
    }
}

const char * COtmSftpConfig::GetAVUUrl()
{
    return m_strAVUUrl;
}

const char * COtmSftpConfig::GetPlgMgGUIUrl()
{
    return m_strPlgMgGUIUrl;
}

const char * COtmSftpConfig::GetUsr()
{
    return m_strUsr;
}

const char * COtmSftpConfig::GetPwd()
{
    return m_strPwd;
}

const char * COtmSftpConfig::GetVer()
{
    return m_strVer;
}

const char * COtmSftpConfig::GetProxyAddress()
{
    return m_strProxyAddress;
}

const char * COtmSftpConfig::GetProxyPort()
{
    return m_strProxyPort;
}

COtmSftpConfig::COtmSftpConfig(void)
{
    memset(m_strAVUUrl,        0x00, sizeof(m_strAVUUrl));
    memset(m_strPlgMgGUIUrl,   0x00, sizeof(m_strPlgMgGUIUrl));
    memset(m_strUsr,           0x00, sizeof(m_strUsr));
    memset(m_strPwd,           0x00, sizeof(m_strPwd));
    memset(m_strVer,           0x00, sizeof(m_strVer));
    memset(m_strProxyAddress,  0x00, sizeof(m_strProxyAddress));
    memset(m_strProxyPort,     0x00, sizeof(m_strProxyPort));
}

COtmSftpConfig::~COtmSftpConfig(void)
{
}
