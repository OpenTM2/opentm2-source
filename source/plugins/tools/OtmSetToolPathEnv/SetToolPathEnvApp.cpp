//+----------------------------------------------------------------------------+
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

#include "stdafx.h"
#include "SetToolPathEnv.h"
#include "SetToolPathEnvApp.h"

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
    int nRC = NO_ERROR;

    CMDPARAM cmdParam;
    cmdParam.bInstall   = TRUE;
    cmdParam.bAllUsers  = TRUE;
    cmdParam.bBackup    = FALSE;
    memset(cmdParam.strInstDir, 0x00, sizeof(cmdParam.strInstDir));

    if ((NULL != lpCmdLine) && (strlen(lpCmdLine) != 0))
    {
        nRC = ProcessParameter(lpCmdLine, &cmdParam);
    }

    if (nRC)
    {
        return nRC;
    }

    cmdParam.hInstance = hInstance;

    SetToolPathEnv setToolPathEnv(&cmdParam);
    setToolPathEnv.SetPathEnvironment();

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

        if (!memicmp(strKey, PARAM_CMD_MODE_KEY, strlen(strKey)))
        {
            if (!memicmp(strVal, ALL_USERS_KEY, strlen(strVal)))
            {
                pCmdParam->bAllUsers = TRUE;
            }
            else
            {
                pCmdParam->bAllUsers = FALSE;
            }
        }
        else if (!memicmp(strKey, PARAM_CMD_INSTALL_KEY, strlen(strKey)))
        {
            pCmdParam->bInstall = atoi(strVal);
        }
        else if (!memicmp(strKey, PARAM_CMD_BACKUP_KEY, strlen(strKey)))
        {
            pCmdParam->bBackup = TRUE;
        }
        else if (!memicmp(strKey, PARAM_CMD_INSTDIR_KEY, strlen(strKey)))
        {
            if ('\"' == strVal[0])
            {
                // remove quotataion mark
                strncpy(pCmdParam->strInstDir, (strVal+1), (strlen(strVal)-2));
            }
            else
            {
                strcpy(pCmdParam->strInstDir, strVal);
            }
        }
        else
        {
            printf("Specified parameter %s not correct!\n", strParameter);
            return ERROR_WRONG_PARAM;
        }
    }

    return (NO_ERROR);
}


void OtmGetKeyValue(const char * strParam, char * strKey, char * strVal, char cBreak, BOOL bKeepBreak)
{
    if ((NULL == strParam) || (strlen(strParam) == 0))
    {
        return;
    }

    int nLen = (int) strlen(strParam);
    int nKeyInx = 0;
    int nValInx = 0;

    BOOL bValStart = FALSE;
    for (int iInx = 0; iInx <= nLen; iInx++)
    {
        if ((cBreak == strParam[iInx]) && !bValStart)
        {
            bValStart = TRUE;
            if (bKeepBreak)
            {
                strKey[nKeyInx] = strParam[iInx];
                nKeyInx++;
            }
            continue;
        }
        else if (!bValStart)
        {
            strKey[nKeyInx] = strParam[iInx];
            nKeyInx++;
            continue;
        }
        else
        {
            strVal[nValInx] = strParam[iInx];
            nValInx++;
            continue;
        }
    }

    strKey[nKeyInx] = '\0';
    strVal[nValInx] = '\0';
}