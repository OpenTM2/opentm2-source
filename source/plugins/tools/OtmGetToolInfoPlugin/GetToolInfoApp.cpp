//+----------------------------------------------------------------------------+
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
//|                    during plugin manager parser                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "stdafx.h"
#include "GetToolInfo.h"
#include "GetToolInfoApp.h"

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
    int nRC = NO_ERROR;

    CMDPARAM cmdParam;
    cmdParam.bCmd = TRUE;
    cmdParam.bDetails = FALSE;

    ShowCmdHeader();

    if ((NULL != lpCmdLine) && (strlen(lpCmdLine) != 0))
    {
        nRC = ProcessParameter(lpCmdLine, &cmdParam);
    }

    if (nRC)
    {
        ShowErrMsg(cmdParam.bCmd, ERROR_WRONG_PARAM);
        return nRC;
    }

    cmdParam.hInstance = hInstance;

    GetToolInfo getToolInfo(&cmdParam);
    getToolInfo.ShowToolInfo();

    return 0;
}

int ProcessParameter(char * strParameter, PCMDPARAM pCmdParam)
{
    // initialize
    int nLen = (int) strlen(strParameter);

    char * strSegment = new char [MAX_BUF_SIZE];
    memset(strSegment, 0x00, sizeof(strSegment));

    int jInx = 0;
    BOOL bQuoStart = FALSE;                       // Quotation start mark
    for (int iInx = 0; iInx <= nLen; iInx++)
    {
        if ('\"' == strParameter[iInx])
        {
            bQuoStart = !bQuoStart;
        }

        if (((' ' != strParameter[iInx]) || bQuoStart) && (iInx != nLen))
        {
            strSegment[jInx] = strParameter[iInx];
            jInx++;
            continue;
        }
        strSegment[jInx] = '\0';

        char * strKey = new char [MAX_BUF_SIZE];
        char * strVal = new char [MAX_BUF_SIZE];
        memset(strKey, 0x00, sizeof(strKey));
        memset(strVal, 0x00, sizeof(strVal));

        OtmGetKeyValue(strSegment, strKey, strVal, ':', FALSE);
        memset(strSegment, 0x00, sizeof(strSegment));
        jInx = 0;

        if ((NULL == strKey) || (strlen(strKey) == 0))
        {
            continue;
        }

        if(!memicmp(strKey, PARAM_CMD_UI_KEY, strlen(strKey))) {
            pCmdParam->bCmd = FALSE;
        } else if(!memicmp(strKey, PARAM_CMD_CMD_KEY, strlen(strKey))) {
            pCmdParam->bCmd = TRUE;
        } else if(!memicmp(strKey, PARAM_CMD_DETAIL_KEY, strlen(strKey))) {
            pCmdParam->bDetails = TRUE;
        } else {
            printf("Specified parameter %s not correct!\n", strParameter);
//            PrintHelp();
            return ERROR_WRONG_PARAM_A;
        }
    }

    return (NO_ERROR);
}

void ShowErrMsg(BOOL bCmd, const char * strErrMsg)
{
    if (bCmd)
    {
        printf("Error: %s", strErrMsg);
    }
    else
    {
        MessageBox(HWND_DESKTOP, strErrMsg, APP_GET_TOOL_INFO_STR, MB_ICONEXCLAMATION | MB_OK);
    }
}

void ShowCmdHeader()
{
    char strLogPath[MAX_PATH];
    memset(strLogPath, 0x00, sizeof(strLogPath));
    sprintf(strLogPath, "\\%s\\%s\\", OTM_FOLDER_KEY, LOG_FOLDER_KEY);

    printf(CMD_HEAD_1, GET_TOOL_INFO_NAME, strLogPath);
    printf(LINE_BREAK_STR);
}
