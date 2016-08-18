//+----------------------------------------------------------------------------+
//|TimeManager.cpp     OTM  Plugin Manager Parser function                     |
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

#include "TimeManager.h"

char * TimeManager::GetTimeStamp()
{
    SYSTEMTIME systemTime;
    static char timestamp[MAX_TIMESTAMP_LEN];

    ::GetLocalTime(&systemTime);

    sprintf(timestamp, "%4d-%02d-%02d %02d:%02d:%02d,%03d", systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
        systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);

    return timestamp;

}

char * TimeManager::GetTimeStampStr()
{
    SYSTEMTIME systemTime;
    static char timestamp[MAX_TIMESTAMP_LEN];

    ::GetLocalTime(&systemTime);

    sprintf(timestamp, "%4d%02d%02d%02d%02d%02d%03d", systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
        systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);

    return timestamp;
}

char * TimeManager::GetDateStr()
{
    SYSTEMTIME systemDate;
    static char strDate[MAX_TIMESTAMP_LEN];

    ::GetLocalTime(&systemDate);

    sprintf(strDate, "%4d%02d%02d", systemDate.wYear, systemDate.wMonth, systemDate.wDay);

    return strDate;
}

char * TimeManager::GetDateTimeStr()
{
    SYSTEMTIME systemDateTime;
    static char strDateTime[MAX_TIMESTAMP_LEN];

    ::GetLocalTime(&systemDateTime);

    sprintf(strDateTime, "%02d/%02d/%4d\t%02d:%02d:%02d", systemDateTime.wMonth, systemDateTime.wDay, 
            systemDateTime.wYear, systemDateTime.wHour, systemDateTime.wMinute, systemDateTime.wSecond);

    return strDateTime;
}
