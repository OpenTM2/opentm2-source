//+----------------------------------------------------------------------------+
//|OtmProfileMgrComm.h     Parse profile set xml file                          |
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
//|                    during parse profile set xml file                       |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
#include "OtmProfileMgrComm.h"

BOOL IsProgramRunning(const char * strProgram)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 ProcessEntry;
        BOOL fMore = FALSE;

        ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
        fMore = Process32First(hSnapshot, &ProcessEntry);
        while (fMore)
        {
            if (stricmp(ProcessEntry.szExeFile, strProgram) == 0)
            {
                return(TRUE);
            }
            fMore = Process32Next(hSnapshot, &ProcessEntry);
        }
        CloseHandle(hSnapshot);
    }

    return(FALSE);
}

void GetToolAppPath(const char * strAppName, char * strAppPath)
{
    // get OpenTM2 properties path
    HKEY hKey = NULL;

    REGSAM regOption = NULL;
    char strSubKey[MAX_PATH];
    memset(strSubKey, 0x00, sizeof(strSubKey));

    if (OtmIsWow64())
    {
        regOption = KEY_READ | KEY_WOW64_64KEY;
        sprintf(strSubKey, "%s\\%s", APP_SOFTWARE_STR, REG_KEY_WOW_6432_NODE_STR);
    }
    else
    {
        regOption = KEY_READ | KEY_WOW64_32KEY;
        sprintf(strSubKey, "%s", APP_SOFTWARE_STR);
    }

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, regOption, &hKey) == ERROR_SUCCESS)
    {
        HKEY hSubKey = NULL;
        if (RegOpenKeyEx(hKey, strAppName, 0, regOption, &hSubKey) == ERROR_SUCCESS)
        {
            DWORD dwType = REG_SZ;
            DWORD iSize = MAX_PATH;
            int iSuccess = RegQueryValueEx(hSubKey, KEY_DRIVE, 0, &dwType, (LPBYTE)strAppPath, &iSize);
            if (iSuccess == ERROR_SUCCESS)
            {
                dwType = REG_SZ;
                iSize = MAX_PATH;
                strcat(strAppPath, "\\");
                iSuccess = RegQueryValueEx(hSubKey, KEY_PATH, 0, &dwType, (LPBYTE)(strAppPath + strlen(strAppPath)), &iSize);
                if (iSuccess != ERROR_SUCCESS)
                {
                    strAppPath[0] = 0;
                }
            }
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

    if ((NULL == strAppPath) || (strAppPath[0] == 0))
    {
        char strModule[MAX_PATH];
        memset(strModule, 0x00, sizeof(strModule));

        GetModuleFileName(NULL, strModule, sizeof(strModule));
        GetModuleAppPath(strModule, strAppPath);
    }

}

void GetModuleAppPath(const char * strModule, char * strAppPath)
{
    char strDrive[_MAX_DRIVE];
    char strDir[_MAX_DIR];
    char strFname[_MAX_FNAME];
    char strExt[_MAX_EXT];
    _splitpath(strModule, strDrive, strDir, strFname, strExt);

    // remove last path delimiter
    int nLen = strlen(strDir);
    strDir[nLen-1] = '\0';

    // find the last path delimiter
    char * strPos = strrchr(strDir, '\\');
    nLen = strPos - strDir;

    if (nLen <= 0)
    {
        return;
    }

    strDir[nLen] = '\0';
    sprintf(strAppPath, "%s\\%s", strDrive, strDir);
}

BOOL OtmIsWow64()
{
    BOOL bWow64 = FALSE;

    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");
    if (NULL != fnIsWow64Process)
    {
        fnIsWow64Process(GetCurrentProcess(), &bWow64);
    }

    return bWow64;
}

char * OtmGetMessageFromCode(int nCode)
{
    switch (nCode)
    {
    case ERROR_OTM_FILE_OPEN_A:                                // 80001
        return ERROR_OTM_FILE_OPEN_A_STR;
    case ERROR_OTM_FILE_NOT_FIND_A:                            // 80002
        return ERROR_OTM_FILE_NOT_FIND_A_STR;
    case ERROR_OTM_NO_MORE_MEMORY_A:                           // 80003
        return ERROR_OTM_NO_MORE_MEMORY_A_STR;
    case ERROR_OTM_CREATE_FOLDER_A:                            // 80004
        return ERROR_OTM_CREATE_FOLDER_A_STR;
    case ERROR_READ_EDITOR_PROP_A:                             // 80005
        return ERROR_READ_EDITOR_PROP_A_STR;
    case ERROR_BACKUP_PROFILE_A:                               // 80006
        return ERROR_BACKUP_PROFILE_A_STR;
    case ERROR_TAR_DIR_A:                                      // 80007
        return ERROR_TAR_DIR_A_STR;
    case ERROR_TAR_FILE_NAME_A:                                // 80008
        return ERROR_TAR_FILE_NAME_A_STR;
    case ERROR_READ_SYS_PROP_A:                                // 80011
        return ERROR_READ_SYS_PROP_A_STR;
    case ERROR_SAVE_SYS_PROP_A:                                // 80012
        return ERROR_SAVE_SYS_PROP_A_STR;
    case ERROR_READ_FONT_INFO_A:                               // 80013
        return ERROR_READ_FONT_INFO_A_STR;
    case ERROR_READ_WORKBENCH_INFO_A:                          // 80015
        return ERROR_READ_WORKBENCH_INFO_A_STR;
    case ERROR_SAVE_WORKBENCH_INFO_A:                          // 80016
        return ERROR_SAVE_WORKBENCH_INFO_A_STR;
    case ERROR_READ_GLOBAL_FIND_INFO_A:                        // 80017
        return ERROR_READ_GLOBAL_FIND_INFO_A_STR;
    case ERROR_SAVE_GLOBAL_FIND_INFO_A:                        // 80018
        return ERROR_SAVE_GLOBAL_FIND_INFO_A_STR;
    case ERROR_READ_BATCH_LIST_INFO_A:                         // 80019
        return ERROR_READ_BATCH_LIST_INFO_A_STR;
    case ERROR_SAVE_BATCH_LIST_INFO_A:                         // 80020
        return ERROR_SAVE_BATCH_LIST_INFO_A_STR;
    case ERROR_READ_NFLUENT_INFO_A:                            // 80021
        return ERROR_READ_NFLUENT_INFO_A_STR;
    case ERROR_SAVE_NFLUENT_INFO_A:                            // 80022
        return ERROR_SAVE_NFLUENT_INFO_A_STR;
    case ERROR_READ_SHARED_MEM_ACCESS_INFO_A:                  // 80023
        return ERROR_READ_SHARED_MEM_ACCESS_INFO_A_STR;
    case ERROR_SAVE_SHARED_MEM_ACCESS_INFO_A:                  // 80024
        return ERROR_SAVE_SHARED_MEM_ACCESS_INFO_A_STR;
    case ERROR_READ_SHARED_MEM_CREATE_INFO_A:                  // 80025
        return ERROR_READ_SHARED_MEM_CREATE_INFO_A_STR;
    case ERROR_SAVE_SHARED_MEM_CREATE_INFO_A:                  // 80026
        return ERROR_SAVE_SHARED_MEM_CREATE_INFO_A_STR;
    case ERROR_READ_LST_LAST_USED_INFO_A:                      // 80027
        return ERROR_READ_LST_LAST_USED_INFO_A_STR;
    case ERROR_SAVE_LST_LAST_USED_INFO_A:                      // 80028
        return ERROR_SAVE_LST_LAST_USED_INFO_A_STR;
    case ERROR_OTM_XERCESC_INITIAL_A:                          // 80030
        return ERROR_OTM_XERCESC_INITIAL_A_STR;
    case ERROR_OTM_XERCESC_CREATE_A:                           // 80031
        return ERROR_OTM_XERCESC_CREATE_A_STR;
    case ERROR_OTM_XERCESC_EXPORT_A:                           // 80032
        return ERROR_OTM_XERCESC_EXPORT_A_STR;
    case ERROR_OTM_XERCESC_MEM_A:                              // 80033
        return ERROR_OTM_XERCESC_MEM_A_STR;
    case ERROR_OTM_XERCESC_DOM_A:                              // 80034
        return ERROR_OTM_XERCESC_DOM_A_STR;
    case ERROR_OTM_XERCESC_UNKNOW_A:                           // 80035
        return ERROR_OTM_XERCESC_UNKNOW_A_STR;
    case ERROR_READ_PROFILE_SET_FILE_A:                        // 80036
        return ERROR_READ_PROFILE_SET_FILE_A_STR;
    default:
        return EMPTY_STR;
    }
}

int OtmExecuteCommand(char * strCmd, char * strOutput, BOOL bNeedWait)
{
    DWORD nRC = NO_ERROR;

    // if requires elevation, use ShellExecute instead of
    SHELLEXECUTEINFO otmShellInfo;

    memset(&otmShellInfo, 0x00, sizeof(otmShellInfo));
    otmShellInfo.cbSize = sizeof(otmShellInfo);
    otmShellInfo.hwnd = NULL;
    otmShellInfo.lpVerb = VERB_OPEN;
    otmShellInfo.lpFile = strCmd;
    otmShellInfo.nShow = SW_SHOWNORMAL;
    otmShellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;

    if (ShellExecuteEx(&otmShellInfo))
    {
        if (bNeedWait)
        {
            WaitForSingleObject(otmShellInfo.hProcess, INFINITE);

            if (!GetExitCodeProcess(otmShellInfo.hProcess, &nRC))
            {
                nRC = GetLastError();
            }
        }
    }
    else
    {
        nRC = GetLastError();
    }

    return nRC;
}

const char * GetTimeStampStr()
{
    SYSTEMTIME systemTime;
    static char timestamp[MAX_TIMESTAMP_LEN];

    ::GetLocalTime(&systemTime);

    sprintf(timestamp, "%4d%02d%02d%02d%02d%02d", systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
            systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

    return timestamp;
}

int CompressOneFile(zipFile zipProfile, const char * strTarFile, const char * strZipDir)
{
    int nRC = ZIP_OK;

    if (!UtlFileExist((char *)strTarFile))
    {
        return nRC;
    }

    zip_fileinfo zipProfileInfo = {0};

    // create file name in zip
    char strDrive[_MAX_DRIVE];
    char strDir[_MAX_DIR];
    char strFname[_MAX_FNAME];
    char strExt[_MAX_EXT];
    _splitpath(strTarFile, strDrive, strDir, strFname, strExt);

    char strNameInZip[MAX_PATH];
    memset(strNameInZip, 0x00, sizeof(strNameInZip));
    if ((NULL != strZipDir) && (strlen(strZipDir) != 0))
    {
        sprintf(strNameInZip, "%s\\%s%s", strZipDir, strFname, strExt);
    }
    else
    {
        sprintf(strNameInZip, "%s%s", strFname, strExt);
    }

    nRC = zipOpenNewFileInZip(zipProfile, strNameInZip, &zipProfileInfo, 
                              NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);

    if (ZIP_OK != nRC)
    {
        return nRC;
    }

    FILE * fileTar = fopen(strTarFile, "rb");
    if (NULL == fileTar)
    {
        return nRC;
    }

    BYTE bBuffer[MAX_BUF_SIZE];
    memset(bBuffer, 0x00, sizeof(bBuffer));

    size_t uReadSzie = 0;
    do
    {
        uReadSzie = fread(bBuffer, 1, sizeof(bBuffer), fileTar);
        if (uReadSzie > 0)
        {
            nRC = zipWriteInFileInZip(zipProfile, bBuffer, uReadSzie);
        }
    } while (!nRC && uReadSzie);

    return nRC;
}

void InitSetOption(POPTIONSET pSetOption)
{
    char strFilePath[MAX_PATH];
    memset(strFilePath, 0x00, sizeof(strFilePath));
    SHGetSpecialFolderPath(NULL, strFilePath, CSIDL_PERSONAL, FALSE);

    memset(pSetOption->strTarFile, 0x00, sizeof(pSetOption->strTarFile));
    sprintf(pSetOption->strTarFile, "%s\\%s", strFilePath, DEF_PROFILE_SET_NAME);

    pSetOption->lstStrDirs.clear();
    pSetOption->lstStrNames.clear();

    pSetOption->nMaxHistCnt = DEF_MAX_HIST_CNT;

    pSetOption->bChkAll = TRUE;
    pSetOption->bChkFldList = TRUE;
    pSetOption->bChkLastVal = TRUE;
    pSetOption->bChkTransEditor = TRUE;
    pSetOption->bChkWorkbench = TRUE;
    pSetOption->bExport = TRUE;                 // EXPORT is the default value
    pSetOption->bEncrypt = TRUE;
    pSetOption->bKeepOriFile = FALSE;
}

long GetFileSize(const char * strFile)
{
    // obtain file size:
    long lSize = 0;

    FILE * fileObj = fopen(strFile, "rb");
    if (!fileObj)
    {
        return lSize;
    }

    fseek(fileObj , 0, SEEK_END);
    lSize = ftell(fileObj);
    fclose(fileObj);

    return lSize;
}

void OtmAddToSet(PSTRLIST pStrList, const char * strValue)
{
    if (pStrList->size() == 0)
    {
        pStrList->push_back(strValue);
        return;
    }

    BOOL bDuplicated = FALSE;
    for (size_t iInx = 0; iInx < pStrList->size(); iInx++)
    {
        if (!stricmp((*pStrList)[iInx].c_str(), strValue))
        {
            bDuplicated = TRUE;
            break;
        }
    }

    if (!bDuplicated)
    {
        pStrList->push_back(strValue);
    }
}