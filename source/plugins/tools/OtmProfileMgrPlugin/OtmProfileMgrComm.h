//+----------------------------------------------------------------------------+
//|OtmProfileMgrComm.h     Parse profile set xml file                          |
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
//|                    during parse profile set xml file                       |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#pragma warning(disable:4996)
#pragma pack(1)
#pragma once

#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TM               // Translation Memory functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_MORPH            // morphologic functions

#include <Windows.h>
#include <string>
#include <vector>
#include <io.h>
#include <CommCtrl.h>
#include <sys/stat.h>
#include <TlHelp32.h>
#include <Shlobj.h>

#include "EQF.H"
#include "EQFTP.H"
#include "EQFTPI.H"
#include "SpecialChardlg.h"

#include "OtmProfileMgrStr.h"
// include zip
#include "curl\zip.h"

using namespace std;

#define MAX_BUF_SIZE                                            1024

#define CONF_FILE_NAME                                          "ProfileSetConf.xml"
#define DEF_PROFILE_SET_NAME                                    "ProfileSet.xml"
#define DEF_PROFILE_SET_LOG_NAME                                "ProfileSetMgr"

#define OPENTM2_APP_NAME_STR                                    "OpenTM2"
#define APP_SOFTWARE_STR                                        "Software"
#define REG_KEY_WOW_6432_NODE_STR                               "Wow6432Node"
#define KEY_PATH                                                "Path"                  // key for the OpenTM2 system path
#define KEY_DRIVE                                               "Drive"                 // key for the OpenTM2 system drive
#define OTM_APPL_STARTER_EXE                                    "OpenTM2Starter.exe"
#define VERB_OPEN                                               "open"

// key for bool value
#define TRUE_VALUE_LOWER                                        "true"
#define FALSE_VALUE_LOWER                                       "false"

// define for specified code
#define OTM_NOT_FOUND                                           -1

// define error code
#define ERROR_OTM_FILE_OPEN_A                                   80001
#define ERROR_OTM_FILE_NOT_FIND_A                               80002
#define ERROR_OTM_NO_MORE_MEMORY_A                              80003
#define ERROR_OTM_CREATE_FOLDER_A                               80004
#define ERROR_READ_EDITOR_PROP_A                                80005
#define ERROR_BACKUP_PROFILE_A                                  80006
#define ERROR_TAR_DIR_A                                         80007
#define ERROR_TAR_FILE_NAME_A                                   80008
#define ERROR_READ_SYS_PROP_A                                   80011
#define ERROR_SAVE_SYS_PROP_A                                   80012
#define ERROR_READ_FONT_INFO_A                                  80013
#define ERROR_READ_WORKBENCH_INFO_A                             80015
#define ERROR_SAVE_WORKBENCH_INFO_A                             80016
#define ERROR_READ_GLOBAL_FIND_INFO_A                           80017
#define ERROR_SAVE_GLOBAL_FIND_INFO_A                           80018
#define ERROR_READ_BATCH_LIST_INFO_A                            80019
#define ERROR_SAVE_BATCH_LIST_INFO_A                            80020
#define ERROR_READ_NFLUENT_INFO_A                               80021
#define ERROR_SAVE_NFLUENT_INFO_A                               80022
#define ERROR_READ_SHARED_MEM_ACCESS_INFO_A                     80023
#define ERROR_SAVE_SHARED_MEM_ACCESS_INFO_A                     80024
#define ERROR_READ_SHARED_MEM_CREATE_INFO_A                     80025
#define ERROR_SAVE_SHARED_MEM_CREATE_INFO_A                     80026
#define ERROR_READ_LST_LAST_USED_INFO_A                         80027
#define ERROR_SAVE_LST_LAST_USED_INFO_A                         80028
#define ERROR_OTM_XERCESC_INITIAL_A                             80030
#define ERROR_OTM_XERCESC_CREATE_A                              80031
#define ERROR_OTM_XERCESC_EXPORT_A                              80032
#define ERROR_OTM_XERCESC_MEM_A                                 80033
#define ERROR_OTM_XERCESC_DOM_A                                 80034
#define ERROR_OTM_XERCESC_UNKNOW_A                              80035
#define ERROR_READ_PROFILE_SET_FILE_A                           80036
#define ERROR_OPEN_PROFILE_SET_DIALOG_A                         80100

#define WARN_DATA_NOT_CONSISTENT                                60001

// keyword of config file
#define EOS                                                     '\0'
#define MAX_TIMESTAMP_LEN                                       100
#define DEF_MAX_HIST_CNT                                        5

typedef vector <string> STRLIST, * PSTRLIST;
typedef vector <wstring> WSTRLIST, * PWSTRLIST;

typedef struct _OPTIONSET
{
    char strTarFile[MAX_PATH];
    STRLIST lstStrDirs;
    STRLIST lstStrNames;

    int  nMaxHistCnt;
    BOOL bChkAll;
    BOOL bChkTransEditor;
    BOOL bChkWorkbench;
    BOOL bChkFldList;
    BOOL bChkLastVal;
    BOOL bExport;
    BOOL bEncrypt;
    BOOL bKeepOriFile;
} OPTIONSET, *POPTIONSET;

// define common function
BOOL IsProgramRunning(const char * strProgram);
void GetToolAppPath(const char * strAppName, char * strAppPath);
void GetModuleAppPath(const char * strModule, char * strAppPath);
BOOL OtmIsWow64();
char * OtmGetMessageFromCode(int nCode);
int OtmExecuteCommand(char * strCmd, char * strOutput, BOOL bNeedWait);
const char * GetTimeStampStr();
int CompressOneFile(zipFile zipProfile, const char * strTarFile, const char * strZipDir = NULL);
void InitSetOption(POPTIONSET pSetOption);
long GetFileSize(const char * strFile);
void OtmAddToSet(PSTRLIST pStrList, const char * strValue);
