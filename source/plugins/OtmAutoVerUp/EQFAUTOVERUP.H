//+----------------------------------------------------------------------------+
//|EQFAUTOVERUP.H     OTM Auto Version Up function                             |
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

#include "resource.h"

#include "OtmXmlParser.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Message result from standard types
#define MRFROMSHORT(s)                                ((LRESULT)(SHORT)(s))
#define MRFROMUSHORT(s)                               ((LRESULT)(USHORT)(s))
#define MRFROM2SHORT(s1, s2)                          ((LRESULT)MAKELONG(s1, s2))
#define SHORT1FROMMR(mr)                              ((USHORT)((ULONG)mr))
#define SHORT2FROMMR(mr)                              ((USHORT)((ULONG)mr >> 16))
#define WMCOMMANDID( mp1, mp2 )                       LOWORD(mp1)
#define WMCOMMANDCMD( mp1, mp2 )                      HIWORD(mp1)

#define WM_OTM_SET_UPDATE                             WM_USER + 0x701
#define WM_OTM_UPDATE_ONE_START                       WM_USER + 0x702
#define WM_OTM_UPDATE_ONE_END                         WM_USER + 0x703

#define AUTO_VER_UP_FREQUENCY_DFT                     0
#define AUTO_VER_UP_DATE_DFT                          0
#define AUTO_VER_UP_KEEP_PKG_DFT                      0
#define AUTO_VER_UP_TAL_LEN                           3
#define OTM_REVISION_LEN                              80
#define OTM_NOT_FOUND                                 -1
#define FIXPACK_STATE_DFT                             0

#define TYPE_COMP                                     1
#define TYPE_FIXPACK                                  2

#define STATE_INSTALLED                               1
#define STATE_UNINSTALL                               0

static HWND ghwndDlg;
static HWND ghwndPrgCtrlDlg;
static HWND ghwndPages[AUTO_VER_UP_TAL_LEN];
static COtmXmlParser * gOtmParser;
//static COtmHttp * gOtmHttp;
static COtmHttps * gOtmHttps;
static LogWriter glogAutoVerUp;
static BOOL gbLogOpened;

static char gstrgConfigFile[MAX_PATH];
static char gstrgOtmPath[MAX_PATH];

/*****************************************************************************
 * function prototypes of exported functions                                 *
 *****************************************************************************/
int AutoVesrionUpProps(HINSTANCE hInstance, char * szPropertyPath, LPTSTR lpCmdLine);

/*****************************************************************************
 * type definitions                                                          *
 *****************************************************************************/
typedef struct _AUTOVERUPPROPIDA
{
   HWND   hwndPages[3];              // array with property pages
   HWND   hwndTabCtrl;               // handle of tab control
   char   strOtmRevision[OTM_REVISION_LEN];
   BOOL   bFromMenu;
   int    nEncrypt;
   char   strEncryptFile[MAX_PATH];
   int    nRC;
} AUTOVERUPPROPIDA, *PAUTOVERUPPROPIDA;

static PAUTOVERUPPROPIDA gpAtvIda;

typedef struct _DOWNLOADPARAM
{
    HWND hwndTabDlg; // tab dialog
    char strDLUrl[MAX_BUF_SIZE];
    char strDLFile[MAX_PATH];
    char strDLType[MAX_DL_TYPE_LEN];
    char strMethod[MAX_METHOD_LEN];
    int nProcessedCnt;
    int nSuccessCnt;
    int nTotalCnt;
    int nLstInx;
    int nRC;
    BOOL bRestart;
    BOOL bNeedWait;
} DOWNLOADPARAM, * PDOWNLOADPARAM;

typedef struct _DOWNLOADPARAM2
{
    HWND hwndTabDlg; // tab dialog
    HWND hwndPCDlg;  // progress control dialog
    int nRC;
} DOWNLOADPARAM2, * PDOWNLOADPARAM2;

/*****************************************************************************
 * function prototypes of internal functions                                 *
 *****************************************************************************/
void AutoVerUpStart(const char * szOtmPath);
BOOL AutoVerUpDlgSheetLoad(HWND, LPARAM);
LRESULT AutoVerUpPropCommand(HWND, WPARAM, LPARAM);
LRESULT AutoVerUpLstNotify(HWND hwndDlg, WPARAM  mp1, LPARAM  mp2);
LRESULT AutoVerUpPropPropertySheetNotification(HWND, WPARAM, LPARAM);
INT_PTR CALLBACK AutoVerUpDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AutoVerUpTabProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AutoVerUpSetProc(HWND, UINT, WPARAM, LPARAM);
int GetLstTotalCnt(HWND hwndDlgLst);
int UpdateAutoVerUpLst(HWND hwndTabDlg, PAUTOVERUPPROPIDA pAtvIda, BOOL bDownload);
int UpdateSelectedComponent(HWND hwndTabDlg);
void SetInfoFromConfig(HWND hwndTabDlg);
void RefreshButtonSet(HWND hwndTabDlg);
void CheckNowRefreshUI(HWND hwndTabDlg);
int CheckNowRefresh(HWND hwndTabDlg);
void SaveInfoToConfig(HWND hwndTabDlg, BOOL bSilent);
BOOL IsSettingChanged(HWND hwndTabDlg);
int ProcessParameter(char * strParameter, PAUTOVERUPPROPIDA pAtvIda); // parameter process
void SetNextCheckTime();
int AddFixpacksToList(HWND hwndAutoVerUpLst, PAUTOVERUPPROPIDA pAtvIda, int nInx, int nLstStartInx, BOOL bNewVer);
BOOL IsFixpackInstalled(const char * strFixpack);
int SetFixpackState(const char * strFixpack, int nState, BOOL bCreate);

// download process control functions
unsigned int __stdcall DownloadPCThreadProc(LPVOID lpParameter);             // thread for downloading one by one
INT_PTR CALLBACK DownloadPCDlgProc(HWND hwndPrgCtrlDlg, UINT msg, WPARAM mp1, LPARAM mp2);
void UpdateComponentStart(HWND hwndPCDlg, PDOWNLOADPARAM pDownloadParam);
void UpdateComponentEnd(HWND hwndPCDlg, PDOWNLOADPARAM pDownloadParam);
void GetXmlDownloadStr(const char * strComponent, const char * strVer, PDOWNLOADPARAM pDownloadParam);
int CheckAndExecute(const char * strMethod, BOOL bNeedWait, const char * strUnzipFiles);
int CheckNewVersion(const char * strOtmVersion);
int HasNewVersion(const char * strOtmVersion);
int CheckConfig();
BOOL HasFixpackUninstalled(int nInx);
void NeedlessFileCheck();
void AddToNeedlessLst(const char * strFileName);
void AddToPendingLst(const char * strComp, const char * strVer);
void TestConnection(HWND hwndTabDlg);
void ResizeWholeDlg(HWND hwndDlg, SHORT sNewWidth, SHORT sNewHeight);
void CloseOpenTM2();
void GetDefPathCopies(PCOTMCOPIES pDefPathCopies);
