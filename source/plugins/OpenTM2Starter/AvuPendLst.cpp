//+----------------------------------------------------------------------------+
//|AvuPendLst.cpp     OTM Auto Version Upgrade XML Local List Parser           |
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
//|                    during autp version upgrade parser                      |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "AvuPendLst.h"

CAvuPendLst::CAvuPendLst(void)
{
    memset(m_strCfgPath, 0x00, sizeof(m_strCfgPath));
}

CAvuPendLst::CAvuPendLst(const char * strOtmPath)
{
    if ((NULL == strOtmPath) || (strlen(strOtmPath) == 0))
    {
        return;
    }

    int nLen = strlen(strOtmPath);
    memset(m_strCfgPath, 0x00, sizeof(m_strCfgPath));

    if (strOtmPath[nLen-1] == '\\')
    {
        sprintf(m_strCfgPath, "%s%s\\%s", strOtmPath, OPENTM2_PLUGIN_FOLDER_STR, AUTO_VER_UP_PENDING_LST);
    }
    else
    {
        sprintf(m_strCfgPath, "%s\\%s\\%s", strOtmPath, OPENTM2_PLUGIN_FOLDER_STR, AUTO_VER_UP_PENDING_LST);
    }
}

int CAvuPendLst::ParseConfigFile()
{
    int nRC = NO_ERROR;

    FILE * fileCfg = fopen(m_strCfgPath, "rb");
    if (NULL == fileCfg)
    {
        nRC = NO_PENDING;
        return nRC;
    }

    char strSetense[MAX_BUF_SIZE];
    memset(strSetense, 0x00, sizeof(strSetense));

    while (fgets(strSetense, MAX_BUF_SIZE, fileCfg))
    {
        int nLen = strlen(strSetense);
        if (strSetense[nLen-1] == '\n')
        {
            if (strSetense[nLen-2] == '\r')
            {
                strSetense[nLen-2] = EOS;
            }
            else
            {
                strSetense[nLen-1] = EOS;
            }
        }

        // prevent adding duplicated strings
        BOOL bFound = FALSE;
        for (int iInx = 0; iInx < (int) m_grpPendings.size(); iInx++)
        {
            if (!stricmp(strSetense, m_grpPendings[iInx].strPending))
            {
                bFound = TRUE;
                break;
            }
        }

        if (bFound)
        {
            continue;
        }

        COTMPENDING otmPending;
        // initial
        memset(otmPending.strPending, 0x00, sizeof(otmPending.strPending));
        otmPending.nRet = NO_ERROR;

        // set value
        strncpy(otmPending.strPending, strSetense, strlen(strSetense));

        m_grpPendings.push_back(otmPending);
    }

    if (m_grpPendings.size() == 0)
    {
        nRC = NO_PENDING;
    }

    fclose(fileCfg);
    return nRC;
}

int CAvuPendLst::GetPendingCnt()
{
    int nCnt = 0;
    if (!m_grpPendings.empty())
    {
        nCnt = (int) m_grpPendings.size();
    }
    return nCnt;
}

const char * CAvuPendLst::GetPendingName(int nInx)
{
    if ((nInx >= GetPendingCnt()) || (nInx < 0))
    {
        return NULL;
    }

     return m_grpPendings[nInx].strPending;
}

int CAvuPendLst::SetPendingResult(int nInx, int nRet)
{
    int nRC = NO_ERROR;

    if ((nInx >= GetPendingCnt()) || (nInx < 0))
    {
        nRC = ERROR_WRONG_INDEX_A;
        return nRC;
    }

    m_grpPendings[nInx].nRet = nRet;

    return nRC;
}

int CAvuPendLst::RefreshPendingLst()
{
    int nRC = NO_ERROR;
    int nEndCnt = 0;

    int nTotalCnt = (int) m_grpPendings.size();
    for (int iInx = 0; iInx < nTotalCnt; iInx++)
    {
        if (!m_grpPendings[iInx].nRet || (ERROR_OPEN_UNZIP_FILE_A != m_grpPendings[iInx].nRet))
        {
            nEndCnt++;
        }
    }

    if (nEndCnt == nTotalCnt)
    {
        if (remove(m_strCfgPath))
        {
            nRC = ERROR_OTM_FILE_DELETE_A;
        }
    }
    else
    {
        FILE * fileCfg = fopen(m_strCfgPath, "wb");
        for (int iInx = 0; iInx < nTotalCnt; iInx++)
        {
            fprintf(fileCfg, "%s\n", m_grpPendings[iInx].strPending);
        }

        fclose(fileCfg);
        nRC = NO_ERROR;
    }

    return nRC;
}

CAvuPendLst::~CAvuPendLst(void)
{
}
