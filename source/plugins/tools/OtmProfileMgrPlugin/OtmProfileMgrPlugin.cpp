//+----------------------------------------------------------------------------+
//|OtmProfileMgrPlugin.cpp     OTM Profile Manager function                    |
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
#include "OtmProfileMgrPlugin.h"

HINSTANCE m_hDllInst;
OtmProfileMgrDlg m_dlgOtmProfileMgr;

extern "C" __declspec(dllexport)
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        m_hDllInst = (HINSTANCE)hModule;
    }
    return TRUE;
}

OtmProfileMgrPlugin::OtmProfileMgrPlugin()
{
    name        = "OtmProfileMgrPlugin";
    shortDesc   = "ProfileSettingsManagementPlugin";
    longDesc    = "This is the plugin for profile settings management";
    version     = "1.0";
    supplier    = "International Business Machines Corporation";
    pluginType  = OtmPlugin::eToolType;
    usableState = OtmPlugin::eUsable;
}

OtmProfileMgrPlugin::~OtmProfileMgrPlugin()
{
}

const char* OtmProfileMgrPlugin::getName()
{
    return name.c_str();
}

const char* OtmProfileMgrPlugin::getShortDescription()
{
    return shortDesc.c_str();
}

const char* OtmProfileMgrPlugin::getLongDescription()
{
    return longDesc.c_str();
}

const char* OtmProfileMgrPlugin::getVersion()
{
    return version.c_str();
}

const char* OtmProfileMgrPlugin::getSupplier()
{
    return supplier.c_str();
}

bool OtmProfileMgrPlugin::stopPlugin( bool fForce  )
{
    // check for active objects
    bool fActiveObjects = false;

    // decline stop if we have active objects
    if ( !fForce && fActiveObjects )
    {
        return( false );
    }

    // TODO: terminate active objects, cleanup, free allocated resources

    // de-register plugin
    PluginManager *pPluginManager = PluginManager::getInstance();
    pPluginManager->deregisterPlugin((OtmPlugin *)this);

    return( true );
}

void OtmProfileMgrPlugin::init()
{
    PluginManager *manager = PluginManager::getInstance();
    manager->addMenuItem(this, "Utilities", "Profile Settings Management...", ID_PROFILE_SET_COMMAND);
}

/*! \brief Process an action bar command
This method is called when a menu item created by the tool plugin is selected by the user.
\param iCommandID Id of the command
*/
void OtmProfileMgrPlugin::processCommand(int iCommandID)
{
    int nRet = NO_ERROR;
    if (iCommandID == ID_PROFILE_SET_COMMAND)
    {
        nRet= m_dlgOtmProfileMgr.OtmProfileMgrDlgOpen(m_hDllInst);
    }
}

extern "C" {
    __declspec(dllexport)
    USHORT registerPlugins()
    {
        PluginManager::eRegRc eRc = PluginManager::eSuccess;
        PluginManager *manager = PluginManager::getInstance();
        OtmProfileMgrPlugin* plugin = new OtmProfileMgrPlugin();
        eRc = manager->registerPlugin((OtmPlugin*) plugin);
        USHORT usRC = (USHORT) eRc;
        return usRC;
    }
}