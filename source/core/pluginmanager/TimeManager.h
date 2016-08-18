//+----------------------------------------------------------------------------+
//|TimeManager.h     OTM Auto Version Up function                              |
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
#pragma warning(disable:4996)

#include "stdio.h"
#include "windows.h"

#define MAX_TIMESTAMP_LEN                            100

class __declspec(dllexport) TimeManager
{
public:
    static char * GetTimeStamp();
    static char * GetTimeStampStr();
    static char * GetDateStr();
    static char * GetDateTimeStr();
};
