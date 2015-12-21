//+----------------------------------------------------------------------------+
//|SetToolPathEnv.CPP     OTM  Plugin Manager function                            |
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
#include "SetToolPathEnv.h"

int SetToolPathEnv::SetPathEnvironment()
{
    m_logWritter.writef(LOG_NAME, "Set path environment start.");

    DWORD dwRet = ERROR_SUCCESS;

    char strSubKey[MAX_BUF_SIZE];
    memset(strSubKey, 0x00, sizeof(strSubKey));

    // open the environment key first
    if (m_bAllUsers)
    {
        sprintf(strSubKey, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
    }
    else
    {
        sprintf(strSubKey, "Environment");
    }

    // if need backup, backup first
    if (m_bBackup && m_bInstall)
    {
        m_logWritter.writef(LOG_NAME, "Backup environment start");
        dwRet = BackupEnvironment(strSubKey);
        m_logWritter.writef(LOG_NAME, "Backup environment end(%d)", dwRet);
        if (dwRet != ERROR_SUCCESS)
        {
            return dwRet;
        }
    }

    m_logWritter.writef(LOG_NAME, "Set system path environment start.");
    dwRet = ProcessSystemPath();
    m_logWritter.writef(LOG_NAME, "Set system path environment end.(%d)", dwRet);

    m_logWritter.writef(LOG_NAME, "Set user path environment start.");
    dwRet = ProcessUserPath();
    m_logWritter.writef(LOG_NAME, "Set user path environment end.(%d)", dwRet);

    m_logWritter.writef(LOG_NAME, "Set path environment end.(%d)", dwRet);
    return dwRet;
}

DWORD SetToolPathEnv::ProcessSystemPath()
{
    DWORD dwRet = ERROR_SUCCESS;

    char strSubKey[MAX_BUF_SIZE];
    memset(strSubKey, 0x00, sizeof(strSubKey));

    HKEY hKey = NULL;

    sprintf(strSubKey, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_READ | KEY_WRITE, &hKey);
    m_logWritter.writef(LOG_NAME, "Open local machine key (%d).", dwRet);

    if (dwRet != ERROR_SUCCESS)
    {
        dwRet = ERROR_OPEN_REG_KEY;
        return dwRet;
    }

    DWORD dwDataSize;
    DWORD dwDataType;
    LPTSTR lpValData;

    // get the value of PATH key
    dwRet = GetEnvPathValue(hKey, &dwDataSize, &dwDataType, lpValData);
    m_logWritter.writef(LOG_NAME, "Get path value (%d).", dwRet);
    if (dwRet != ERROR_SUCCESS)
    {
        return dwRet;
    }

    // get the value of PATH key
    BOOL bAdd = FALSE;
    if (!m_bInstall)
    {
        bAdd = FALSE;
    }
    else
    {
        if (m_bAllUsers)
        {
            bAdd = TRUE;
        }
        else
        {
            bAdd = FALSE;
        }
    }

    dwRet = SetEnvPathValue(hKey, dwDataSize, dwDataType, lpValData, bAdd);
    m_logWritter.writef(LOG_NAME, "Set path value (%d)(%d).", bAdd, dwRet);
    if (dwRet != ERROR_SUCCESS)
    {
        return dwRet;
    }

    RegCloseKey(hKey);

    return dwRet;
}

DWORD SetToolPathEnv::ProcessUserPath()
{
    DWORD dwRet = ERROR_SUCCESS;

    char strSubKey[MAX_BUF_SIZE];
    memset(strSubKey, 0x00, sizeof(strSubKey));

    HKEY hKey = NULL;

    sprintf(strSubKey, "Environment");
    dwRet = RegOpenKeyEx(HKEY_CURRENT_USER, strSubKey, 0, KEY_READ | KEY_WRITE, &hKey);
    m_logWritter.writef(LOG_NAME, "Open the current use key (%d).", dwRet);

    if (dwRet != ERROR_SUCCESS)
    {
        dwRet = ERROR_OPEN_REG_KEY;
        return dwRet;
    }

    DWORD dwDataSize;
    DWORD dwDataType;
    LPTSTR lpValData;

    // get the value of PATH key
    dwRet = GetEnvPathValue(hKey, &dwDataSize, &dwDataType, lpValData);
    m_logWritter.writef(LOG_NAME, "Get path value (%d).", dwRet);
    if (dwRet != ERROR_SUCCESS)
    {
        return dwRet;
    }

    // get the value of PATH key
    BOOL bAdd = FALSE;
    if (!m_bInstall)
    {
        bAdd = FALSE;
    }
    else
    {
        if (m_bAllUsers)
        {
            bAdd = FALSE;
        }
        else
        {
            bAdd = TRUE;
        }
    }

    dwRet = SetEnvPathValue(hKey, dwDataSize, dwDataType, lpValData, bAdd);
    m_logWritter.writef(LOG_NAME, "Set path value (%d)(%d).", bAdd, dwRet);
    if (dwRet != ERROR_SUCCESS)
    {
        return dwRet;
    }

    RegCloseKey(hKey);

    return dwRet;
}

DWORD SetToolPathEnv::GetEnvPathValue(HKEY hKey, LPDWORD pdwDataSize, LPDWORD pdwDataType, LPTSTR & lpValData)
{
    DWORD dwRet = ERROR_SUCCESS;

    DWORD dwBuffSize = MAX_STR_SIZE;
    DWORD dwRestrictType = RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND;

    lpValData = (LPTSTR) malloc (dwBuffSize);
    memset(lpValData, 0x00, sizeof(lpValData));
    if (NULL == lpValData)
    {
        dwRet = ERROR_NO_MORE_SPACE;
        return dwRet;
    }

    *pdwDataSize = dwBuffSize;
    dwRet = RegGetValue(hKey, NULL, "PATH", dwRestrictType, pdwDataType, lpValData, pdwDataSize);

    // add for type not support start
    if (ERROR_UNSUPPORTED_TYPE == dwRet)
    {
        dwRestrictType = RRF_RT_REG_SZ;
        dwRet = RegGetValue(hKey, NULL, "PATH", dwRestrictType, pdwDataType, lpValData, pdwDataSize);
    }
    // add end

    while (ERROR_MORE_DATA == dwRet)
    {
        dwBuffSize += MAX_BUF_SIZE;
        lpValData = (LPTSTR) realloc(lpValData, dwBuffSize);
        memset(lpValData, 0x00, sizeof(lpValData));

        if (NULL == lpValData)
        {
            dwRet = ERROR_NO_MORE_SPACE;
            break;
        }

        *pdwDataSize = dwBuffSize;
        dwRet = RegGetValue(hKey, NULL, "PATH", dwRestrictType, pdwDataType, lpValData, pdwDataSize);
    }

    return dwRet;
}

DWORD SetToolPathEnv::SetEnvPathValue(HKEY hKey, DWORD dwDataSize, DWORD dwDataType, LPTSTR & lpValData, BOOL bAdd)
{
    DWORD dwRet = ERROR_SUCCESS;
    CHAR  szChar1[2] = {0} ;

    LPSTR lpSegData = NULL;
    LONG  lSegDataLen = dwDataSize + 1 ;       /* Length + NULL EOS */
    lpSegData = (LPTSTR) malloc (lSegDataLen);
    memset(lpSegData, 0x00, lSegDataLen);

    LPSTR lpPathVal = NULL;
    LONG  lPathValLen = dwDataSize + strlen(m_strInstDir) + 2 ; /* Length + ';' + NULL EOS */
    lpPathVal = (LPTSTR) malloc (lPathValLen);
    memset(lpPathVal, 0x00, lPathValLen);

    // Remove OpenTM2 path value if it already exists in PATH.
    for (DWORD iInx = 0; iInx < dwDataSize; iInx++)
    {
       // accumulate directory value
        if (';' != lpValData[iInx])
        {
            szChar1[0] = lpValData[iInx] ; 
            strcat(lpSegData, szChar1 );

            if (iInx != (dwDataSize - 1))
            {
                continue;
            }
        }

        // If the directory is not for OpenTM2, then keep it.
        if (memicmp(lpSegData, m_strInstDir, strlen(m_strInstDir)))
        {
            if (strlen(lpPathVal) == 0)
            {
                strcpy(lpPathVal, lpSegData);
            }
            else
            {
                strcat(lpPathVal, ";");
                strcat(lpPathVal, lpSegData);
            }
        } else {
           // if the directory is for OpenTM2, then skip it.
        }

        memset(lpSegData, 0x00, lSegDataLen);    /* Reset directory variable */
    }

    LPSTR lpNewVal;
    if (bAdd)
    {
       if (strlen(lpPathVal) > 0)
       {
          dwDataSize = strlen(lpPathVal) + strlen(m_strInstDir) + 1;
          lpNewVal = (LPTSTR) malloc (dwDataSize+1);
          strcpy(lpNewVal, lpPathVal);
          strcat(lpNewVal, ";");
          strcat(lpNewVal, m_strInstDir);
       } else {
          dwDataSize = strlen(m_strInstDir);
          lpNewVal = (LPTSTR) malloc (dwDataSize+1);
          strcpy(lpNewVal, m_strInstDir);
       }
    }
    else
    {
        dwDataSize = strlen(lpPathVal) ;
        lpNewVal = (LPTSTR) malloc (dwDataSize+1);
        strcpy(lpNewVal, lpPathVal);
    }

    dwDataType  = REG_EXPAND_SZ;
    dwRet = RegSetValueEx(hKey, "PATH", NULL, dwDataType, (LPBYTE)lpNewVal, dwDataSize);

    free( lpSegData ) ;
    free( lpPathVal ) ;
    free( lpNewVal ) ;

    return dwRet;
}

int SetToolPathEnv::BackupEnvironment(const char * strSubKey)
{
    int nRet = NO_ERROR;

    char strBkpFile[MAX_BUF_SIZE];
    memset(strBkpFile, 0x00, sizeof(strBkpFile));
    sprintf(strBkpFile, "%s\\%s_%s.reg", m_strInstDir, REG_BACKUP_FILE_NAME, GetTimeStampStr());

    char strCmd[MAX_BUF_SIZE];
    memset(strCmd, 0x00, sizeof(strCmd));

    // open the environment key first
    if (m_bAllUsers)
    {
        sprintf(strCmd, REG_BACKUP_CMD, REG_HKLM_KEY, strSubKey, strBkpFile);
    }
    else
    {
        sprintf(strCmd, REG_BACKUP_CMD, REG_HKCU_KEY, strSubKey, strBkpFile);
    }

    nRet = system(strCmd);

    return nRet;
}

char * SetToolPathEnv::GetTimeStampStr()
{
    SYSTEMTIME systemTime;
    static char timestamp[MAX_TIMESTAMP_LEN];

    ::GetLocalTime(&systemTime);

    sprintf(timestamp, "%4d%02d%02d%02d%02d%02d%03d", systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
        systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);

    return timestamp;
}

SetToolPathEnv::SetToolPathEnv(PCMDPARAM pCmdParam)
{
    m_hInstance = pCmdParam->hInstance;
    m_bInstall  = pCmdParam->bInstall;
    m_bAllUsers = pCmdParam->bAllUsers;
    m_bBackup   = pCmdParam->bBackup;
    memset(m_strInstDir, 0x00, sizeof(m_strInstDir));
    strcpy(m_strInstDir, pCmdParam->strInstDir);

    // add for log
    string strInstDir(m_strInstDir);
    int nPos = strInstDir.rfind("\\");
    m_logWritter.SetLogPath(strInstDir.substr(0, nPos).c_str());
}

SetToolPathEnv::SetToolPathEnv(void)
{
    m_bInstall  = TRUE;
    m_bAllUsers = TRUE;
    m_bBackup   = FALSE;
    memset(m_strInstDir, 0x00, sizeof(m_strInstDir));
}

SetToolPathEnv::~SetToolPathEnv(void)
{
}
