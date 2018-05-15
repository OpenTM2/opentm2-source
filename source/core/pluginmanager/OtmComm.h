//+----------------------------------------------------------------------------+
//|OtmComm.h     OTM  Plugin Manager Parser function                           |
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
#pragma once
#pragma warning(disable:4996)

#include <stdio.h>
#include <io.h>
#include <windows.h>
#include <winbase.h>
#include <process.h>
#include <CommCtrl.h>
#include <string>
#include <vector>
#include <TlHelp32.h>
#include <Shellapi.h>
#include <CommCtrl.h>
#include <sys/stat.h>

#include "curl\curl.h"
#include "curl\zlib.h"
#include "curl\unzip.h"

#include "TimeManager.h"

//#include "..\..\core\utilities\LogWriter.h"

using namespace std;

// error code defination
#define ERROR_WRONG_PARAM_A                           1001
#define ERROR_MALLOC_SIZE_A                           1002
#define ERROR_OTM_CREATE_FOLDER_A                     1003
#define ERROR_OTM_FILE_NOT_FIND_A                     1004
#define ERROR_OTM_OPEN_FILE_A                         1005
#define ERROR_OTM_READ_FILE_A                         1006
#define ERROR_OTM_REMOVE_FILE_B                       1007
#define ERROR_OTM_REMOVE_FOLDER_B                     1008
#define ERROR_EMPTY_FILE_A                            1009
#define ERROR_UNZIP_FILE_A                            1010
#define ERROR_OPEN_UNZIP_FILE_A                       1011
#define ERROR_WRITE_UNZIP_FILE_A                      1012
#define ERROR_VERSION_DIFFER_A                        1013
#define ERROR_CREATE_THREAD_A                         1014
#define ERROR_CURL_INITIAL_A                          1015
#define ERROR_CURL_SETOPT_A                           1016
#define ERROR_HTTP_DOWNLOAD_A                         1017
#define ERROR_HTTPS_DOWNLOAD_A                        1018
#define ERROR_SFTP_DOWNLOAD_A                         1019
#define ERROR_OTM_XML_PARSER_A                        1020
#define ERROR_CANNOT_FIND_KEY_C                       1021
#define ERROR_OTM_XERCESC_INITIAL_A                   1022
#define ERROR_CANNOT_GET_MODULE_NAME_A                1023
#define ERROR_HTTPS_CONNECT_A                         1024
#define ERROR_SFTP_CONNECT_A                          1025
#define ERROR_LOAD_DLL_LIBCURL_A                      1026
#define ERROR_LOAD_ENTRY_POINT_A                      1027

#define NO_NEED_START                                 999
#define VAL_HAS_NEW_VER                               998
#define VAL_NOT_HAS_NEW_VER                           997
#define VAL_NO_NEED_OPEN                              996

// define size
#define OTM_NOT_FOUND                                 -1
#define DEF_ERROR_SIZE                                -1
#define MAX_BUF_SIZE                                  2046
#define MAX_LEN                                       256
#define MAX_PROXY_ADDRESS_LEN                         50
#define MAX_PROXY_PORT_LEN                            10
#define MAX_VER_LEN                                   50
#define MAX_TYPE_LEN                                  30
#define MAX_DL_TYPE_LEN                               10
#define MAX_METHOD_LEN                                20
#define MAX_SHORT_DSCP_LEN                            256
#define MAX_LONG_DSCP_LEN                             20000
#define MAX_DATE_VAL                                  99991231
#define MAX_DATE_LEN                                  11
#define DEF_CONNECT_TIMEOUT                           5

#define VER_NEW_VAL                                   1
#define VER_SAM_VAL                                   0
#define VER_OLD_VAL                                   -1

#define PARENT_CHECKED                                2
#define PARENT_NOT_FOUND                              1
#define PARENT_NOT_CHECKED                            0

#define TYPE_INSTALLED                                1
#define TYPE_NEW_INSTALL                              2
#define TYPE_UPDATE                                   3

#define IS_FOLDER                                     10001
#define IS_FILE                                       10002
#define NOT_EXIST                                     10003
#define UNDEFINED_ERROR                               10004

#define EMPTY_STR                                     ""
#define KEY_DEFAULT                                   ""
#define LOG_NAME_OTM_COMM                             "OtmComm"
#define ORIGINAL_PATH_KEY                             "$ORIDIR$"
#define OPENTM2_DEF_PATH_KEY                          "$OPENTM2DIR$"
#define PLUGIN_DEF_PATH_KEY                           "$PLUGINDIR$"
#define OPENTM2_DEF_DL_PATH_KEY                       "$OPENTM2DLDIR$"
#define DLTYPE_ZIP                                    "zip"
#define METHOD_INSTALL                                "install"
#define METHOD_OPEN                                   "open"
#define DATE_FORMAT                                   "%04d%02d%02d"
#define REG_KEY_WOW_6432_NODE_STR                     "Wow6432Node"
#define APP_SOFTWARE_STR                              "Software"
#define KEY_PATH                                      "Path"
#define KEY_DRIVE                                     "Drive"
#define APP_NAME                                      "OpenTM2"
#define WIN_FOLDER_KEY                                "WIN"
#define LIBCURL_DLL                                   "libcurl.dll"
#define PENDING_DIR_STR                               "Downloads"

// Verb
#define VERB_OPEN                                     "open"


#define SFTP_INFO_CONF                                "SFTPInfo.conf"
#define PENDING_UPT_CONF                              "PendingUpdates.conf"
#define APP_SETTINGS_STR                              "Settings"
#define KEY_NEED_RESTART                              "NeedRestart"
#define KEY_NEED_RESTART_A                            "NeedRestartA"
#define PREFIX_FIXPACK                                "Fixpack"

#define TYPE_NEW_INSTALL_STR                          "Install"
#define TYPE_UPDATE_STR                               "Update"

enum Frequency
{
    Day,
    Week,
    Month//Manual
};

typedef struct _PROCESSBAR
{
    HWND hwndProcessBar;
    int nID;
} PROCESSBAR, * PPROCESSBAR;

typedef vector <string> OTMGRPSTING, * POTMGRPSTING;

typedef struct _OTMDOWNLOADFILE
{
    const char * strFile;
    FILE * pDLFile;

} OTMDLFILE, * POTMDLFILE;

typedef struct _COTMCOPY
{
    char strFrom[MAX_PATH];
    char strTo[MAX_PATH];

} COTMCOPY, * POTMCOPY;

typedef vector <COTMCOPY> COTMCOPIES, * PCOTMCOPIES;

typedef struct _NETWORKPARAM
{
    char strProxyAddress[MAX_PROXY_ADDRESS_LEN];
    char strProxyPort[MAX_PROXY_ADDRESS_LEN];
    char strError[MAX_BUF_SIZE];
    int nTimeout;

} NETWORKPARAM, * PNETWORKPARAM;

typedef struct _FIXPACK
{
    char strId[MAX_LEN];
    char strDLUrl[MAX_BUF_SIZE];
    char strDLType[MAX_DL_TYPE_LEN];
    char strMethod[MAX_METHOD_LEN];
    char strShortDscp[MAX_SHORT_DSCP_LEN];
    char strLongDscp[MAX_LONG_DSCP_LEN];
    char strDate[MAX_DATE_LEN];
    BOOL bRestart;
    BOOL bNeedWait;
    COTMCOPIES fixpackCopies;
} FIXPACK, * PFIXPACK;

typedef struct _MAINTHREADINFO
{
    BOOL bFixpack;
    char strName[MAX_LEN];
    char strVer[MAX_VER_LEN];
    char strFixpkId[MAX_LEN];
    int iInx;
    int jInx;
    int kInx;
} MAINTHREADINFO, *PMAINTHREADINFO;

// define function pointer for dynamic use libcurl
typedef CURLcode (*PCURL_GLOBAL_INIT)(long flags);
typedef CURL * (*PCURL_EASY_INIT)(void);
typedef CURLcode (*PCURL_EASY_SETOPT)(CURL *curl, CURLoption option, ...);
typedef CURLcode (*PCURL_EASY_PERFORM)(CURL *curl);
typedef void (*PCURL_EASY_CLEANUP)(CURL *curl);
typedef void (*PCURL_GLOBAL_CLEANUP)(void);

__declspec(dllexport) 
int OtmUnCompress(const char * strSrcDir, PCOTMCOPIES pDefPathCopies, PCOTMCOPIES pOtmCopyies, const char * strDefPath, POTMGRPSTING pstrFiles);
__declspec(dllexport) 
int OtmExtractOneFile(unzFile unzipFile, char * strDestFile);
__declspec(dllexport) 
void GetRealDestDir(const char * strFileFullName, PCOTMCOPIES pDefPathCopies, PCOTMCOPIES pPluginCopyies, const char * strDefPath, char * strDestFile);
__declspec(dllexport) 
int OtmCreateFolder(char * strDestFile);
__declspec(dllexport) 
BOOL ReplaceDefaultDir(const char * strOriPath, const char * strDefPath, const char * strReplacePath, char * strNewPath);
__declspec(dllexport) 
BOOL ReplaceOriginalDir(const char * strOriPath, const char * strReplacePath, char * strNewPath);
__declspec(dllexport) 
void OtmConvertToBackSlash(char * strFileName);
__declspec(dllexport) 
long GetFileSize(const char * strFile);
__declspec(dllexport) 
void OtmGetKeyValue(const char * strParam, char * strKey, char * strVal, char cBreak, BOOL bKeepBreak);

__declspec(dllexport) 
BOOL OtmStringReplace(string & strTarget, const string & strOld, const string & strNew);
__declspec(dllexport) 
void OtmJointPath(char * strDestPath, const char * strPath1, const char * strPath2);
__declspec(dllexport) 
void OtmRemoveExtraSlash(char * strPath);
__declspec(dllexport) 
int OtmComparePath(const char * strPath1, const char * strPath2);
__declspec(dllexport) 
void OtmSplitPath(const char * strFullName, char * strPath, char * strName);
__declspec(dllexport) 
int OtmIsDirectory(const char * strPath);
__declspec(dllexport) 
int OtmDeleteAllInDir(const char * strTargetDir);
__declspec(dllexport) 
void OtmMergePath(const char * strOriPath, const char * strReplacePath, char * strMergedPath);
__declspec(dllexport) 
void OtmSplitPathToParts(const char * strPath, POTMGRPSTING pvecSegments);
__declspec(dllexport) 
int OtmFindReplacePos(POTMGRPSTING pvecOriSegments, POTMGRPSTING pvecReplaceSegments);
__declspec(dllexport) 
int OtmExecuteCommand(char * strCmd, char * strOutput, BOOL bNeedWait);
__declspec(dllexport) 
BOOL IsProgramRunning(const char * strProgram);
__declspec(dllexport) 
void RemoveDoubleQuotationMark(char * strParam);
__declspec(dllexport) 
void OtmCenterWindow(HWND hWnd);
__declspec(dllexport) 
char * OtmCtime(const time_t * timeValue);
__declspec(dllexport) 
int IsNewVersion(const char * strVerOld, const char * strVerNew);
__declspec(dllexport) 
int GetDateFromTime(int nTimeVal);
__declspec(dllexport) 
void OtmSplitDotToParts(const char * strToSplit, POTMGRPSTING pvecSegments);
__declspec(dllexport) 
void GetNameFromUrl(char * strName, const char * strUml);
__declspec(dllexport) 
BOOL IsFixpack(const char * strName);
__declspec(dllexport) 
void SplitFixpackName(const char * strFixpkName, char * strName, char * strVer, char * strId);
__declspec(dllexport) 
BOOL IsFixpackParentChked(HWND hwndDlgLst, const char * strName, int nNameInx, int nVerInx);
__declspec(dllexport) 
void InitOtmCopy(POTMCOPY pOtmCopy);
__declspec(dllexport) 
void InitFixpack(PFIXPACK pFixpack);
__declspec(dllexport) 
void InitMainThreadInfo(PMAINTHREADINFO pPluginTdInfo);
__declspec(dllexport) 
void InitNetworkParam(PNETWORKPARAM pNetworkParam);
BOOL OtmIsWow64();
__declspec(dllexport) 
void GetModuleAppPath(const char * strModule, char * strAppPath);
__declspec(dllexport) 
void GetToolAppPath(const char * strAppName, char * strAppPath);
__declspec(dllexport) 
void GetLibcurlPath(char * strPath);

