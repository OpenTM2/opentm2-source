//+----------------------------------------------------------------------------+
//|OtmAutoVerUpComm.h     OTM  Plugin Manager Parser function                  |
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

#include "curl/curl.h"
#include "curl/zlib.h"
#include "curl/unzip.h"
#include "core\pluginmanager\OtmComm.h"
#include "OtmAutoVerUpStr.h"
#include "OtmLogWriter.h"

using namespace std;

// error code defination
#define ERROR_AUTO_VER_UP_A                           3001
#define ERROR_GET_COMP_MODULE_NAME_A                  3002
#define ERROR_NOT_INSTALL_ALONE_B                     3003
#define ERROR_CANNOT_GET_VERSION_A                    3004
#define ERROR_CANNOT_GET_REGISTRY_INFO_A              3005
#define ERROR_COMP_NAME_A                             3006
#define ERROR_DL_NOT_ZIP_A                            3007

// define for specified code
#define ERROR_END_ALL_TASK                            9002

#define APP_SOFTWARE_STR                              "Software"

// parameter key
#define PARAM_CMD_VER_KEY                             "/v"
#define PARAM_CMD_FROM_MENU_KEY                       "/fromMenu"
#define PARAM_CMD_ENCRYPT_KEY                         "/encrypt"
#define PARAM_CMD_ENCRYPT_FILE_KEY                    "/encryptFile"

// keyword of config file
#define APP_AUTO_VER_UP_NET_SET                       "Networks"
#define KEY_AUTO_VER_UP_URL                           "URL"
#define KEY_AUTO_VER_UP_PROXY_ADDRESS                 "ProxyAddress"
#define KEY_AUTO_VER_UP_PROXY_PORT                    "ProxyPort"
#define KEY_AUTO_VER_UP_TIMEOUT                       "Timeout"
#define APP_AUTO_VER_UP_SET                           "Settings"
#define KEY_AUTO_VER_UP_CONF_VER                      "Ver"
#define KEY_AUTO_VER_UP_FREQUENCY                     "Frequency"
#define KEY_AUTO_VER_UP_DATE                          "Date"
#define KEY_AUTO_VER_UP_KEEP_PKG                      "KeepPackage"

#define FOR_OPENSRC_KEY                               "forOpenSrc"
#define FOR_OPENSRC_VALUE                             "FOR_OPENSRC"

// keyword of xml
#define KEY_COMPONENT                                 "component"
#define KEY_ATTRI_NAME                                "name"
#define KEY_VERSION                                   "version"
#define KEY_DATE                                      "availabledate"
#define KEY_LONG_DSCP                                 "longdescription"
#define KEY_SHORT_DSCP                                "description"
#define KEY_SEVERITY                                  "Severity"
#define KEY_IMPACT                                    "Impact"
#define KEY_AFTERACTION                               "AfterAction"
#define KEY_INSTALL                                   "install"
#define KEY_COPY                                      "copy"
#define KEY_ATTRI_FROM                                "from"
#define KEY_ATTRI_TO                                  "to"
#define KEY_DOWNLOAD                                  "download"
#define KEY_ATTRI_DLTYPE                              "type"
#define KEY_ATTRI_METHOD                              "method"
#define KEY_ATTRI_RESTART                             "restart"
#define KEY_ATTRI_NEED_WAIT                           "needwait"
#define KEY_ARRTI_TRUE_STR                            "true"
#define KEY_FIXPACKS                                  "Fixpacks"
#define KEY_FIXPACK                                   "Fixpack"
#define KEY_ATTRI_ID                                  "id"

#define DLTYPE_ZIP                                    "zip"
#define METHOD_COPY                                   "copy"

#define AUTO_VER_UP_CONF_FILE_NAME                    "AutoVersionUp.conf"
#define AUTO_VER_UP_FIXP_CONF_NAME                    "AutoVerUpFixp.conf"
#define AUTO_VER_UP_SFTP_INFO_CONF                    "SFTPInfo_AVU.conf"
#define AUTO_VER_UP_XML                               "OtmAutoVerUp.xml"
#define AUTO_VER_UP_NEEDLESS_LST                      "AutoVerUpNeedless.lst"
#define AUTO_VER_UP_PENDING_LST                       "AutoVerUpPending.lst"
#define LOG_AUTO_VER_UP_NAME                          "AutoVerUp"
#define DOWNLOAD_DIR                                  "Downloads"
#define OTM_APPL_EXE                                  "OpenTM2.exe"
#define OTM_APPL_STARTER_EXE                          "OpenTM2Starter.exe"
#define KEY_PATH                                      "Path"                          // key for the OpenTM2 system path
#define KEY_DRIVE                                     "Drive"                         // key for the OpenTM2 system drive
#define KEY_DEFAULT                                   ""
#define KEY_PLUGINS_PATH                              "PLUGINS"
#define KEY_WIN_PATH                                  "WIN"
#define AUTO_VERSION_UP_CONFIG_SAMPLE                 "AutoVersionUp.conf.sample"

#define EOS                                           '\0'

__declspec(dllexport) 
char * OtmGetMessageFromCode(int nCode);
