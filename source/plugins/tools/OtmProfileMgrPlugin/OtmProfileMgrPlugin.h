//+----------------------------------------------------------------------------+
//|OtmProfileMgrPlugin.h  OTM Profile Manager function                         |
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
//|                    during profile settings management                      |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#pragma once

#ifndef _OtmProfileMgrPlugin_H_
#define _OtmProfileMgrPlugin_H_

#include "core\pluginmanager\OtmToolPlugin.h"
#include "core\PluginManager\PluginManager.h"
#include "OtmProfileMgrDlg.h"

class OtmProfileMgrPlugin: public OtmToolPlugin
{
public:
    // Constructor
    OtmProfileMgrPlugin();

    // Destructor
    ~OtmProfileMgrPlugin();

    // Returns the name of the plugin
    const char* getName();

    // Returns a short plugin-Description
    const char* getShortDescription();

    // Returns a verbose plugin-Description
    const char* getLongDescription();

    //  Returns the version of the plugin
    const char* getVersion();

    // brief Returns the name of the plugin-supplier
    const char* getSupplier();

    /*! \brief Stops the plugin. 
    Terminating-function for the plugin, will be called directly before
    the DLL containing the plugin will be unloaded.\n
    The method should call PluginManager::deregisterPlugin() to tell the PluginManager
    that the plugin is not active anymore.
    Warning: THIS METHOD SHOULD BE CALLED BY THE PLUGINMANAGER ONLY!
    \param fForce, TRUE = force stop of the plugin even if functions are active, FALSE = only stop plugin when it is inactive
    \returns TRUE when successful */
    bool stopPlugin( bool fForce = false );

    /*! \brief Initialize plugin 
    This method is called when the workbench window has been created.
    The tool plugin can use the plugin manager methods addMenuItem and addSubMenu to add
    own entries to the workbench toolbar.
    */
    void init();

    /*! \brief Process an action bar command
    This method is called when a menu item created by the tool plugin is selected by the user.
    \param iCommandID Id of the command
    */
    void processCommand(int iCommandID);

private:
    std::string name;
    std::string shortDesc;
    std::string longDesc;
    std::string version;
    std::string supplier;
};

#endif
