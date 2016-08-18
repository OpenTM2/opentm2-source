//+----------------------------------------------------------------------------+
//|OtmAutoVerUp.cpp     OTM Auto Version Up function                           |
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

#include "OtmAutoVerUp.h"

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
    if ((NULL == lpCmdLine) || (strlen(lpCmdLine) == 0))
    {
        MessageBox(HWND_DESKTOP, ERROR_CANNOT_GET_VERSION_A_STR, AUTO_VER_UP_NAME_STR, MB_ICONEXCLAMATION | MB_OK);
        return ERROR_CANNOT_GET_VERSION_A;
    }

    // get OpenTM2 properties path
    char szOtmPath[MAX_PATH];
    memset(szOtmPath, 0x00, sizeof(szOtmPath));

    GetToolAppPath(OPENTM2_APP_NAME_STR, szOtmPath);

    if ((NULL == szOtmPath) || (szOtmPath[0] == 0))
    {
        MessageBox(HWND_DESKTOP, ERROR_CANNOT_GET_REGISTRY_INFO_A_STR, AUTO_VER_UP_NAME_STR, MB_ICONEXCLAMATION | MB_OK);
        return ERROR_CANNOT_GET_REGISTRY_INFO_A;
    }

    return AutoVesrionUpProps(hInstance, szOtmPath, lpCmdLine);
}
