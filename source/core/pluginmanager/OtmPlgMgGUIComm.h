//+----------------------------------------------------------------------------+
//|EQFPLGMG.CPP     OTM  Plugin Manager Parser function                        |
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

#include "core\pluginmanager\OtmPlgMgGUIStr.h"
#include "core\pluginmanager\OtmComm.h"
#include "core\utilities\LogWriter.h"
#include "core\pluginmanager\HistoryWriter.h"

// code
// error code defination
#define ERROR_WRONG_PLUGIN_NAME_A                     2001
#define ERROR_GET_PLUGIN_PATH_A                       2002
#define ERROR_PLUGIN_NOT_FOUND_A                      2003
#define ERROR_PLUGIN_NO_ARRTI_B                       2004
#define ERROR_UPDATE_PLUGIN_B                         2005
#define ERROR_DEREGISTER_PLUGIN_B                     2006
#define ERROR_REMOVE_PLUGIN_B                         2007
#define ERROR_CANNOT_FIND_PLUGIN_B                    2008
#define ERROR_CANNOT_GET_URL_B                        2009
#define ERROR_END_ALL_TASK                            2010

// Others
#define EMPTY_STR                                     ""
#define DOWNLOAD_DIR_STR                              "Downloads"
#define PLUGIN_MGR_APP_NAME_STR                       "Plugin Manager"
#define PLUGIN_DEF_NONE_STR                           "none"

// keyword of config
#define PLUGIN_MGR_CONFIG                             "PluginManager.conf"
#define PLUGIN_MGR_FIXP_CONFIG                        "PluginManagerFixp.conf"
#define PLUGIN_MGR_FIXP_CONFIG_TMP                    "PluginManagerFixp.tmp.conf"
#define PLUGIN_MGR_CONFIG_SAMPLE                      "PluginManager.conf.sample"
#define PLUGIN_MGR_XML                                "PluginManagerInfo.xml"
#define PLUGIN_MGR_LOC_XML                            "PluginManagerInfo.xml"
#define PLUGIN_MGR_SFTP_INFO_CONF                     "SFTPInfo_PMG.conf"
#define PLUGIN_PENDING_LST                            "PluginPend.lst"
#define PLUGIN_MGR_LOC_TMP_XML                        "PluginManagerPluginsLocalTemp.xml"

#define APP_PLUGIN_MGR_NET_SET                        "Networks"
#define APP_PLUGIN_MGR_BASIC_PLUGINS                  "BasicPlugins"
#define APP_PLUGIN_MGR_NONREMOV_PLUGINS               "NonRemovablePlugins"
#define APP_PLUGIN_MGR_SETTINGS                       "Settings"
#define KEY_PLUGIN_LONG_DSCP                          "longDesc"
#define KEY_PLUGIN_MGR_URL                            "URL"
#define KEY_PLUGIN_MGR_PROXY_ADDRESS                  "ProxyAddress"
#define KEY_PLUGIN_MGR_PROXY_PORT                     "ProxyPort"
#define KEY_PLUGIN_MGR_TIMEOUT                        "Timeout"
#define KEY_PLUGIN_MGR_BP_NAME                        "Name"
#define KEY_PLUGIN_MGR_BP_MIN_CNT                     "MinCnt"
#define KEY_PLUGIN_MGR_CONF_VER                       "Ver"
#define KEY_PLUGIN_MGR_FREQUENCY                      "Frequency"
#define KEY_PLUGIN_MGR_DATE                           "Date"
#define KEY_PLUGIN_MGR_AUTO                           "Auto"
#define KEY_PLUGIN_MGR_KEEP_PKG                       "KeepPackage"
#define KEY_FIXPACKS                                  "Fixpacks"
#define KEY_FIXPACK                                   "Fixpack"
#define KEY_ATTRI_ID                                  "id"
#define KEY_FP_TYPE                                   "fptype"

// default value
#define PLUGIN_MGR_KEEP_PKG_DFT                       0
#define PLUGIN_MGR_FREQUENCY_DFT                      0
#define PLUGIN_MGR_DATE_DFT                           0
#define PLUGIN_MGR_AUTO_DFT                           0

__declspec(dllexport) 
char * OtmGetMessageFromCode(int nCode);
