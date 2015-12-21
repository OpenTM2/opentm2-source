//+----------------------------------------------------------------------------+
//|GetToolInfoApp.h     OTM  Plugin Manager Parser function                    |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
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


// parameter key
#define PARAM_CMD_UI_KEY                              "/ui"
#define PARAM_CMD_CMD_KEY                             "/cmd"
#define PARAM_CMD_DETAIL_KEY                          "/DETAILS"

int ProcessParameter(char * strParameter, PCMDPARAM pCmdParam);
void ShowErrMsg(BOOL bCmd, const char * strErrMsg);
void ShowCmdHeader();
