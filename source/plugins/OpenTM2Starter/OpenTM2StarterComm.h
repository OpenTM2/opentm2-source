//+----------------------------------------------------------------------------+
//|OpenTM2StarterComm.h     OTM  Plugin Manager Parser function                |
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

#include <Windows.h>
#include <process.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <CommCtrl.h>
#include <sys/stat.h>
#include <TlHelp32.h>
#include <Shellapi.h>
#include "curl\zlib.h"
#include "curl\unzip.h"
#include "OpenTM2StarterStr.h"
#include "OtmLogWriterLoc.h"
#include "HistoryWriter.h"
#include "TimeManager.h"

using namespace std;

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// error code defination
#define ERROR_CANNOT_GET_REGISTRY_INFO_A              5001
#define ERROR_WRONG_PLUGIN_NAME_A                     5002
#define ERROR_WRONG_PARAM_A                           5003
#define ERROR_OTM_XERCESC_INITIAL_A                   5004
#define ERROR_OTM_FILE_NOT_FIND_A                     5005
#define ERROR_OTM_FILE_DELETE_A                       5006
#define ERROR_EMPTY_FILE_A                            5007
#define ERROR_OTM_XML_PARSER_A                        5008
#define ERROR_UNZIP_FILE_A                            5009
#define ERROR_OPEN_UNZIP_FILE_A                       5010
#define ERROR_WRITE_UNZIP_FILE_A                      5011
#define ERROR_OTM_CREATE_FOLDER_A                     5012
#define ERROR_CREATE_THREAD_A                         5013
#define ERROR_GET_FILE_NAME_A                         5014
#define ERROR_PLUGIN_NOT_FOUND_A                      5015
#define ERROR_WRONG_INDEX_A                           5016
#define ERROR_CANNOT_START_A                          5017
#define ERROR_LOAD_DLL_ZLIBWAPI_A                     5018
#define ERROR_COMP_NAME_A                             5019
#define ERROR_DL_NOT_ZIP_A                            5020
#define ERROR_CANNOT_FIND_KEY_C                       5040

// define for specified code
#define NO_PENDING                                    -1
#define OTM_NOT_FOUND                                 -1
#define DEF_ERROR_SIZE                                -1

#define IS_FOLDER                                     10001
#define IS_FILE                                       10002
#define NOT_EXIST                                     10003
#define UNDEFINED_ERROR                               10004

#define EMPTY_STR                                     ""

#define TYPE_NEW_INSTALL                              2
#define TYPE_UPDATE                                   3

// define legth
#define MAX_BUF_SIZE                                  2046
#define MAX_LEN                                       256
#define DEF_WAITING_TIME                              3000
#define DEF_WAITING_TIMES                             1
#define DFT_KEEP_PKG                                  0
#define MAX_DL_TYPE_LEN                               10
#define MAX_METHOD_LEN                                20
#define MAX_SHORT_DSCP_LEN                            256
#define MAX_LONG_DSCP_LEN                             2046
#define MAX_DATE_LEN                                  11

// keyword of config file
#define EOS                                            '\0'
#define OPENTM2_FOLDER_STR                            "WIN"
#define OPENTM2_PLUGIN_FOLDER_STR                     "PLUGINS"
#define OPENTM2_PENDING_FOLDER_STR                    "Downloads"
#define PLUGIN_MGR_XML                                "PluginManagerInfo.xml"
#define PLUGIN_MGR_LOC_XML                            "PluginManagerInfo.xml"
#define AUTO_VER_UP_LOC_XML                           "OtmAutoVerUp.xml"
#define PLUGIN_MGR_CONFIG                             "PluginManager.conf"
#define AUTO_VER_UP_CONFIG                            "AutoVersionUp.conf"
#define PENDING_UPT_CONF                              "PendingUpdates.conf"
#define PENDING_UPT_CONF_SAMPLE                       "PendingUpdates.conf.sample"
#define PLUGIN_MGR_FIXP_CONFIG                        "PluginManagerFixp.conf"
#define DLTYPE_ZIP                                    "zip"
#define METHOD_INSTALL                                "install"
#define METHOD_OPEN                                   "open"
#define APP_OPENTM2_NAME_STR                          "OpenTM2"
#define APP_OPENTM2STARTER_NAME_STR                   "OpenTM2Starter"
#define APP_OPENTM2_EXE_STR                           "OpenTM2.exe"
#define APP_SOFTWARE_STR                              "Software"
#define APP_SETTINGS_STR                              "Settings"
#define KEY_PATH                                      "Path"                          // key for the OpenTM2 system path
#define KEY_DRIVE                                     "Drive"                         // key for the OpenTM2 system drive
#define KEY_DEFAULT                                   ""
#define KEY_NEED_RESTART                              "NeedRestart"
#define KEY_MAX_WAIT_TIME                             "MaxWaitTime"
#define KEY_MAX_WAIT_TIMES                            "MaxWaitTimes"
#define KEY_KEEP_PKG                                  "KeepPackage"
#define ORIGINAL_PATH_KEY                             "$ORIDIR$"
#define PLUGIN_DEF_PATH_KEY                           "$PLUGINDIR$"
#define OPENTM2_DEF_PATH_KEY                          "$OPENTM2DIR$"
#define OPENTM2_DEF_DL_PATH_KEY                       "$OPENTM2DLDIR$"
#define PLUGIN_DEF_NONE_STR                           "none"
#define BACKSLASH_STR                                 "\\"
#define KEY_FIXPACKS                                  "Fixpacks"
#define KEY_FIXPACK                                   "Fixpack"
#define PREFIX_FIXPACK                                "Fixpack"
#define KEY_ATTRI_ID                                  "id"

#define TYPE_NEW_INSTALL_STR                          "Install"
#define TYPE_UPDATE_STR                               "Update"

// define tool  app value
#define TOOL_APP_NAME                                 ""
#define WIN_FOLDER_KEY                                "WIN"
#define ZLIBWAPI_DLL                                  "zlibwapi.dll"
#define REG_KEY_WOW_6432_NODE_STR                     "Wow6432Node"

// define function pointer for dynamic use zlibwapi
typedef unzFile (ZEXPORT *PUNZOPEN) OF((const char *));
typedef int (ZEXPORT *PUNZGETGLOBALINFO) OF((unzFile, unz_global_info *));
typedef int (ZEXPORT *PUNZGOTOFIRSTFILE) OF((unzFile));
typedef int (ZEXPORT *PUNZGETCURRENTFILEINFO) OF((unzFile, unz_file_info *, char *, uLong, void *, uLong, char *, uLong));
typedef int (ZEXPORT *PUNZGOTONEXTFILE) OF((unzFile));
typedef int (ZEXPORT *PUNZCLOSE) OF((unzFile));
typedef int (ZEXPORT *PUNZOPENCURRENTFILE) OF((unzFile file));
typedef int (ZEXPORT *PUNZREADCURRENTFILE) OF((unzFile, voidp , unsigned));
typedef int (ZEXPORT *PUNZCLOSECURRENTFILE) OF((unzFile file));

typedef struct _COTMPENDING
{
    char strPending[MAX_LEN];
    int nRet;

} COTMPENDING, * POTMPENDING;

typedef vector <string> OTMGRPSTING, * POTMGRPSTING;

typedef struct _COTMCOPY
{
    char strFrom[MAX_PATH];
    char strTo[MAX_PATH];

} COTMCOPY, * POTMCOPY;

typedef vector <COTMCOPY> COTMCOPIES, * PCOTMCOPIES;

typedef struct _CPLUGINCOPY
{
    char strFrom[MAX_PATH];
    char strTo[MAX_PATH];

} CPLUGINCOPY, * PCPLUGINCOPY;

typedef vector <CPLUGINCOPY> CPLUGINCOPIES, * PCPLUGINCOPIES;

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
    int nType;
    BOOL bFixpack;
    BOOL bInstalled;
    char strName[MAX_LEN];
    char strVer[MAX_LEN];
    char strFixpkId[MAX_LEN];
    int iInx;
    int jInx;
    int kInx;
} MAINTHREADINFO, *PMAINTHREADINFO;

__declspec(dllexport) 
int OtmUnCompress(const char * strSrcDir, PCOTMCOPIES pDefPathCopies, PCOTMCOPIES pPluginCopyies, const char * strDefPath, POTMGRPSTING strFiles);
__declspec(dllexport) 
int OtmExtractOneFile(unzFile unzipFile, char * strDestFile);
__declspec(dllexport) 
void GetRealDestDir(const char * strFileFullName, PCOTMCOPIES pDefPathCopies, PCOTMCOPIES pOtmCopies, const char * strDefPath, char * strDestFile);
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
char * OtmGetMessageFromCode(int nCode);
__declspec(dllexport) 
BOOL IsProgramRunning(const char * strProgram);
__declspec(dllexport) 
BOOL IsFixpack(const char * strName);
__declspec(dllexport) 
void SplitPluginName(const char * strName, PMAINTHREADINFO pMainTdInfo);
__declspec(dllexport) 
void InitOtmCopy(POTMCOPY pOtmCopy);
__declspec(dllexport) 
void InitFixpack(PFIXPACK pFixpack);
__declspec(dllexport) 
void InitMainThreadInfo(PMAINTHREADINFO pPluginTdInfo);

__declspec(dllexport) 
void GetZlibwapiPath(char * strPath);
__declspec(dllexport) 
void GetToolAppPath(const char * strAppName, char * strAppPath);
__declspec(dllexport) 
void GetModuleAppPath(const char * strModule, char * strAppPath);
__declspec(dllexport) 
BOOL OtmIsWow64();
__declspec(dllexport) 
void SplitFixpackName(const char * strFixpkName, char * strName, char * strVer, char * strId);
__declspec(dllexport) 
void SplitCompName(const char * strName, PMAINTHREADINFO pMainTdInfo);
