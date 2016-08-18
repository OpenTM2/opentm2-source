//+----------------------------------------------------------------------------+
//|CommStr.h     OTM  Plugin Manager function                                  |
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

#define ERROR_WRONG_PARAM                             1
#define ERROR_NO_MORE_SPACE                           2
#define ERROR_CANNOT_GET_REG_INFO                     3
#define ERROR_CANNOT_GET_APP_PATH                     4
#define ERROR_GET_REG_VALUE                           5
#define ERROR_OPEN_REG_KEY                            6

#define MAX_BUF_SIZE                                  1024
#define MAX_STR_SIZE                                  8192
#define MAX_TIMESTAMP_LEN                             100

#define REG_BACKUP_CMD                                "REG EXPORT \"%s\\%s\" %s /y"
#define REG_HKLM_KEY                                  "HKLM"
#define REG_HKCU_KEY                                  "HKCU"
#define REG_BACKUP_FILE_NAME                          "OtmSetPathEnvBkp"
#define LOG_NAME                                      "OtmSetPathEnv"
