//+----------------------------------------------------------------------------+
//|OpenTM2Starter.h     OTM Auto Version Up function                           |
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
//|                    during auto version up                                  |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#pragma once

#define LOG_OPENTM2_STARTER_NAME                      "OpenTM2Starter"

#define STATE_INSTALLED                               1
#define STATE_UNINSTALL                               0

#include "resource.h"
#include "OpenTM2StarterComm.h"
#include "PlginPendLst.h"
#include "AvuPendLst.h"
#include "PlgMgXmlLocParser.h"
#include "OtmXmlLocParser.h"

static HINSTANCE ghInstance;
static char gstrOtmPath[MAX_PATH];
static char gstrOpenTM2Path[MAX_PATH];
static char gstrPendUptConf[MAX_PATH];
static CPlginPendLst * gPlginPendLst;
static CAvuPendLst   * gAvuPendLst;

static HistoryWriter gHistoryWriter;
static LogWriter glogOpenTM2Starter;
static BOOL gbLogOpened;

typedef struct _PENDINGPARAM
{
    HWND hwndDlg; // dialog
    BOOL bInstalled;
} PENDINGPARAM, * PPENDINGPARAM;

/*****************************************************************************
 * function prototypes of exported functions                                 *
 *****************************************************************************/
int OpenTM2StarterProps(HINSTANCE hInstance, char * szPropertyPath, LPTSTR lpCmdLine);
INT_PTR CALLBACK PendingPCDlgProc(HWND hwndPrgCtrlDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void OtmCenterWindow(HWND hWnd);
unsigned int __stdcall PendingPCThreadProc(LPVOID lpParameter);
void GetFileNameFromUrl(const char * strUrl, char * strFileName);
int SetFixpackState(PMAINTHREADINFO pMainTdInfo, int nState, BOOL bCreate);
int FindPluginPos(const char * strPendingName, CPlgMgXmlLocParser * pPlgMgXmlLocParser, PMAINTHREADINFO pMainTdInfo);
int FindAutoVerUpPos(const char * strPendingName, COtmXmlLocParser * pOtmXmlLocParser, PMAINTHREADINFO pMainTdInfo);
void GetDefPathCopies(PCOTMCOPIES pDefPathCopies);
int PluginMgrPendingProc(LPVOID lpParameter);
int AutoVerUpPendingProc(LPVOID lpParameter);
int PendingUpdatesPlugin(const char * strPendingName, PMAINTHREADINFO pMainTdInfo);
int PendingUpdatesAVU(const char * strPendingName, PMAINTHREADINFO pMainTdInfo);

