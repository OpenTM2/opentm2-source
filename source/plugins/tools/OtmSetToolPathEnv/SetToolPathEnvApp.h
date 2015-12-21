//+----------------------------------------------------------------------------+
//|GetToolInfoApp.h     OTM  Plugin Manager Parser function                    |
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

#include "stdafx.h"

// parameter key
#define PARAM_CMD_MODE_KEY                            "/mode"
#define PARAM_CMD_INSTALL_KEY                         "/install"
#define PARAM_CMD_INSTDIR_KEY                         "/instdir"
#define PARAM_CMD_BACKUP_KEY                          "/backup"
#define ALL_USERS_KEY                                 "AllUsers"

int ProcessParameter(char * strParameter, PCMDPARAM pCmdParam);
void OtmGetKeyValue(const char * strParam, char * strKey, char * strVal, char cBreak, BOOL bKeepBreak);
