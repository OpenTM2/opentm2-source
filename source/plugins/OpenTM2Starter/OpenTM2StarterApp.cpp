//+----------------------------------------------------------------------------+
//|OpenTM2StarterApp.cpp     OTM Auto Version Up function                      |
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
//|                    during start OpenTM2                                    |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "OpenTM2StarterApp.h"

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
    // get OpenTM2 path
    HKEY hKey = NULL;
    char szOtmPath[MAX_PATH];
    memset(szOtmPath, 0x00, sizeof(szOtmPath));

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, APP_SOFTWARE_STR, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        HKEY hSubKey = NULL;
        if (RegOpenKeyEx(hKey, APP_OPENTM2_NAME_STR, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
        {
            DWORD dwType = REG_SZ;
            DWORD iSize = sizeof(szOtmPath);
            int iSuccess = RegQueryValueEx(hSubKey, KEY_DRIVE, 0, &dwType, (LPBYTE)szOtmPath, &iSize);
            if (iSuccess == ERROR_SUCCESS)
            {
                dwType = REG_SZ;
                iSize = sizeof(szOtmPath);
                strcat(szOtmPath, "\\");

                iSuccess = RegQueryValueEx(hSubKey, KEY_PATH, 0, &dwType, (LPBYTE)szOtmPath+strlen(szOtmPath), &iSize);
                if (iSuccess != ERROR_SUCCESS)
                {
                    szOtmPath[0] = 0;
                }
            }
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

    if ((NULL == szOtmPath) || (szOtmPath[0] == 0))
    {
        char strModule[MAX_PATH];
        memset(strModule, 0x00, sizeof(strModule));

        GetModuleFileName(NULL, strModule, sizeof(strModule));
        GetModuleAppPath(strModule, szOtmPath);
    }

    if ((NULL == szOtmPath) || (szOtmPath[0] == 0))
    {
        MessageBox(HWND_DESKTOP, ERROR_CANNOT_GET_REGISTRY_INFO_A_STR, APP_OPENTM2STARTER_NAME_STR, MB_ICONEXCLAMATION | MB_OK);
        return ERROR_CANNOT_GET_REGISTRY_INFO_A;
    }

    return OpenTM2StarterProps(hInstance, szOtmPath, lpCmdLine);
}
