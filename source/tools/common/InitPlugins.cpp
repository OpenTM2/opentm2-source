//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
extern "C" {
#include "eqf.h"
}
#include "core\PluginManager\OtmPlugin.h"
#include "core\PluginManager\PluginManager.h"
#include "InitPlugins.h"

// initialize the Plugin-Manager and load the available plugins
USHORT InitializePlugins( char *szPluginPath )
{
    USHORT usRC = NO_ERROR;         // function return code (add for P402974)
	PluginManager* thePluginManager = PluginManager::getInstance();
	usRC = thePluginManager->loadPluginDlls(szPluginPath);   // add for P402974
	InitTMPluginWrapper();
	InitDictPluginWrapper();
	InitMarkupPluginMapper();
	InitDocumentPluginMapper();

    return usRC;
}
