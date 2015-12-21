//+----------------------------------------------------------------------------+
//|  MarkupPluginMapper.H - Markup table plugin mapper functions               |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#ifndef _MARKUPPLUGINMAPPER_H_
#define _MARKUPPLUGINMAPPER_H_

#include "vector"
#include "core\pluginmanager\PluginManager.h"
#include "core\pluginmanager\OtmMarkup.h"
#include "core\pluginmanager\OtmMarkupPlugin.h"

// find a markup table object by markup table name
__declspec(dllexport)
 OtmMarkup *GetMarkupObject( char *pszMarkup, char *pszPlugin );

// get the markup table plugin providing a specific markup table 
__declspec(dllexport)
OtmMarkupPlugin *GetMarkupPlugin( char *pszMarkupName );

#endif