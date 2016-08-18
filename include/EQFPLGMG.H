/*! EQF Startup Utility functions header file
	Description:       This is header file contains the function prototypes
					and type definitions used by the EQFDRVEX.C module.
	Copyright (c) 1999-2016, International Business Machines Corporation and others. All rights reserved.
*/
#ifndef CPPTEST
extern "C"
{
#endif

#pragma pack(push, TM2StructPacking, 1)
#include "eqf.id"                 // general EQF ids
#include "eqftwb.id"              // ids for Troja workbench
#include "eqfstart.id"

#pragma pack(pop, TM2StructPacking)
#ifndef CPPTEST
}
#endif

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\PlgMgXmlParser.h"

// define user message
#define WM_OTM_UPDATE                                 WM_USER + 0x700
#define WM_OTM_SET_UPDATE                             WM_USER + 0x701
#define WM_OTM_UPDATE_ONE_START                       WM_USER + 0x702
#define WM_OTM_UPDATE_ONE_END                         WM_USER + 0x703

#define AUTO_VERUP_EXE                                "OtmAutoVerUp.exe"
#define AUTO_VER_UP_CONF_FILE_NAME                    "AutoVersionUp.conf"
#define AUTO_VER_UP_SET_APP                           "Settings"
#define AUTO_VER_UP_START_FIRST_KEY                   "StartFirst"
#define LOG_PLUGIN_MGR_GUI_NAME                       "PluginManagerGUI"

// define valude
#define AUTO_VER_UP_START_FIRST_DFT                   1
#define MIN_PLUGIN_CNT                                1
#define PLUGIN_MGR_GUI_PAGE_CNT                       6

#define FIXPACK_STATE_DFT                             0

#define IS_NEW_PLUGIN                                 1
#define NOT_NEW_PLUGIN                                2
#define PLUGIN_NOT_LOADED                             3

#define TYPE_FIXPACK                                  2

#define STATE_INSTALLED                               1
#define STATE_UNINSTALL                               0

#define STATE_CB_NO                                   0
#define STATE_CB_YES                                  1
#define STATE_CB_GREY                                 2

/*****************************************************************************
 * function prototypes of exported functions                                 *
 *****************************************************************************/
__declspec(dllexport)
VOID PluginManagerMenuProps();
__declspec(dllexport)
VOID PluginManagerAutoProps();
__declspec(dllexport)
VOID AutoVerUpProps(BOOL bFromMenu);
int PluginManagerGUIStart();

/*****************************************************************************
 * type definitions                                                          *
 *****************************************************************************/
typedef struct _PLUGINMAGPROPIDA
{
   HWND  hwndPages[PLUGIN_MGR_GUI_PAGE_CNT];        // array with property pages
   HWND  hwndTabCtrl;                               // handle of tab control
} PLUGINMAGPROPIDA, *PPLUGINMAGPROPIDA;

typedef struct _DOWNLOADPARAM
{
    HWND hwndTabDlg; // tab dialog
    HWND hwndPCDlg;  // progress control dialog
    char strDLUrl[MAX_BUF_SIZE];
    char strDLFile[MAX_PATH];
    char strDLType[PLUGIN_DL_TYPE_MAX_LEN];
    char strMethod[PLUGIN_DL_METHOD_MAX_LEN];
    BOOL bRestart;
    BOOL bNeedWait;
    int nProcessedCnt;
    int nSuccessCnt;
    int nTotalCnt;
    int nLstInx;
    int nType;
    int nRC;
} DOWNLOADPARAM, *PDOWNLOADPARAM;

static CPlgMgXmlParser * gplginParse;
static COtmHttps * gOtmHttps = NULL;
static HWND ghwndDlg;
static LogWriter glogPlgMgr;
static BOOL gbLogOpened;
static char gstrConfigPath[MAX_PATH];
static HANDLE ghChkVerThread;
static HistoryWriter * gHistoryWriter;

// installed plugin UI function
INT_PTR CALLBACK PlgMgrInstalledDlgProc(HWND, WINMSG, WPARAM, LPARAM);
void AddItemToInstalledList(HWND plgMgList, int nLstInx, int nPluginInx, int nImage);
void RefreshInstalledList(HWND plgMgList);

// available plugin UI functions
INT_PTR CALLBACK PlgMgrAvailableDlgProc(HWND, WINMSG, WPARAM, LPARAM);
void InstallSelectedPlugin(HWND hwndAvbTabDlg);

// update plugin UI functions
INT_PTR CALLBACK PlgMgrUpdatesDlgProc(HWND, WINMSG, WPARAM, LPARAM);
void UpdateSelectedPlugin(HWND hwndUptTabDlg);

// setting UI function
INT_PTR CALLBACK PlgMgrSettingDlgProc(HWND, WINMSG, WPARAM, LPARAM);

// help UI function
INT_PTR CALLBACK PlgMgrHelpDlgProc(HWND, WINMSG, WPARAM, LPARAM);

// UI function
BOOL PluginManagerDlgSheetLoad(HWND, PPLUGINMAGPROPIDA);
MRESULT PluginManagerDlgInit (HWND, WPARAM, LPARAM);
MRESULT PluginManagerPropCommand(HWND, WPARAM, LPARAM);
MRESULT PlugMgPropPropertySheetNotification(HWND, WPARAM, LPARAM);
INT_PTR CALLBACK PluginManagerDlgProc(HWND, WINMSG, WPARAM, LPARAM);
LRESULT PluginListNotify(HWND hwndDlg, WPARAM mp1, LPARAM mp2, int nType);
void ShowLongDscp(HWND hwndDlg, int nType);
void InitialOtmListViewCtrl(HWND hwndListView);
void SetListViewCtrlChkState(HWND hwndDlg, int nType);
int Otm_ListView_GetCheckState(HWND hwndListView, int nInx);
int AddFixpacksToList(HWND hwndPlgMgrLst, int iInx, int jInx, int nLstInx, BOOL bNewVer);

// remove plugin functions
BOOL HasUpdate(const char * strPluginLoc, const char * strVerLoc);
void RemoveSelectedPlugin(HWND hwndInsTabDlg);
int RemovePlugin(HWND hwndInsTabDlg, const char * strPluginName);
int RemoveWithConfig(HWND hwndInsTabDlg, string strPluginName);
int RemoveOneWithConfig(HWND hwndTabDlg, const char * strPluginName, POTMGRPSTING pGrpPluginPath);
int RemoveAlone(HWND hwndTabDlg, const char * strPluginName);
int DeregisterOtmPlugin(const char * strPluginName);
int RemoveOnePlugin(HWND hwndTabDlg, string strPluginName);

// plugin corresponding functions
int isNewPlugin(char * strPluginName);
char * GetLocalPluginVer(char * strPluginName);
int OtmParseXml(BOOL bDownload);
void SplitPlugins(HWND hwndPlgMgrLst, int nType);
int HasNewVersion();
BOOL HasFixpackUninstalled(int iInx, int jInx);

// method for config file
int CheckConfig(char * strPluginPath);
void SetInfoFromConfig(HWND hwndTabDlg);
void SaveInfoToConfig(HWND hwndTabDlg, BOOL bSilent);
BOOL IsSettingChanged(HWND hwndTabDlg);
void RefreshButtonSet(HWND hwndTabDlg);
void SetNextCheckTime();
void CheckNowRefresh(HWND hwndTabDlg);
BOOL IsFixpackInstalled(const char * strFixpack);
int SetFixpackState(const char * strFixpack, int nState, BOOL bCreate);
void ClearFixpackState(const char * strPlugin);

// method for download progress control
INT_PTR CALLBACK DownloadPCDlgProc(HWND, WINMSG, WPARAM, LPARAM);
unsigned int __stdcall DownloadPCThreadProc(LPVOID lpParameter);
unsigned int __stdcall CheckNewVersionProc(LPVOID lpParameter);
void DownloadStart(HWND hwndTabDlg, PDOWNLOADPARAM pDownloadParam);
void DownloadEnd(HWND hwndTabDlg, PDOWNLOADPARAM pDownloadParam);
int GetPluginDllPath(const char * strDLUrl, BOOL bRestart, char * strDLFile);
void GetXmlDownloadInfo(PMAINTHREADINFO pPluginTdInfo, PDOWNLOADPARAM pDLParam);
int GetLstTotalCnt(HWND hwndDlgLst);
BOOL RemoveItemChk(HWND hwndDlgLst);
void SplitMultiPlugins(char * strBasicPlugins, POTMGRPSTING pGrpStrBP);
void EndOneTask(PDOWNLOADPARAM pDownloadParam, PMAINTHREADINFO pMainTdInfo);
int GetDepnsStr(PMAINTHREADINFO pPluginTdInfo, char * strDepnsStr);
void RefreshAllList(HWND hwndTabDlg, BOOL bShowMsg);
void UpdatePendUptFiles(PMAINTHREADINFO pMainTdInfo, int nType);
BOOL IsRemovablePlugin(const char * strPluginName);
void TestConnection(HWND hwndTabDlg);
int OtmDownloadFile(PDOWNLOADPARAM pDownloadParam);

int GetPluginInfoForLst(HWND hwndDlgLst, int nType, int nLstInx, PMAINTHREADINFO pPluginTdInfo);
void GetDefPathCopies(PCOTMCOPIES pDefPathCopies);
BOOL IfPluginExists(const char * strStartDir, const char * strPluginDllName, const char * strPluginName);

