//+----------------------------------------------------------------------------+
//|AvuPendLst.h     OTM Auto Version Upgrade XML Local List Parser             |
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
#include "OpenTM2StarterComm.h"

#define AUTO_VER_UP_PENDING_LST                       "AutoVerUpPending.lst"

class CAvuPendLst
{
// private member
private:
    char m_strCfgPath[MAX_PATH];
    vector <COTMPENDING> m_grpPendings;

// public function
public:
    CAvuPendLst(void);
    CAvuPendLst(const char * strOtmPath);
    int ParseConfigFile();
    int GetPendingCnt();
    const char * GetPendingName(int nInx);
    int SetPendingResult(int nInx, int nRet);
    int RefreshPendingLst();
    ~CAvuPendLst(void);
};
