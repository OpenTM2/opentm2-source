/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqf.h"
#include "core\PluginManager\PluginManager.h"
#include "OtmToolsLauncherPlugin.h"
#include "OtmToolsLauncherPlugin.id"

#include "string"
#include "vector"
#include "windows.h"

void executeCommand();

HINSTANCE hDll;

extern "C" __declspec(dllexport)
BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID)
{	 
   switch (ul_reason_for_call) {
      case DLL_PROCESS_ATTACH:
         hDll = (HINSTANCE)hModule;
   }
   return TRUE;
}

OtmToolsLauncherPlugin::OtmToolsLauncherPlugin()
{
    
    name        = "OtmToolsLauncherPlugin";
    shortDesc   = "ToolsLauncherPlugin";
    longDesc    = "This is a plugin to launcher OpenTM2 tools";
    version     = "1.0";
    supplier    = "International Business Machines Corporation";
    pluginType  = OtmPlugin::eToolType;
    usableState = OtmPlugin::eUsable;
}


OtmToolsLauncherPlugin::~OtmToolsLauncherPlugin()
{
}

const char* OtmToolsLauncherPlugin::getName()
{
	return name.c_str();
}

const char* OtmToolsLauncherPlugin::getShortDescription()
{
	return shortDesc.c_str();
}

const char* OtmToolsLauncherPlugin::getLongDescription()
{
	return longDesc.c_str();
}

const char* OtmToolsLauncherPlugin::getVersion()
{
	return version.c_str();
}

const char* OtmToolsLauncherPlugin::getSupplier()
{
	return supplier.c_str();
}

/*! \brief Initialize plugin 
  This method is called when the workbench window has been created.
  The tool plugin can use the plugin manager methods addMenuItem and addSubMenu to add
  own entries to the workbench toolbar.
*/
void OtmToolsLauncherPlugin::init()
{
	PluginManager *manager = PluginManager::getInstance();
    manager->addMenuItem( this, "Utilities", "GUI for command line tools", ID_TOOLSLAUNCHER_COMMAND );
}

/*! \brief Process an action bar command
  This method is called when a menu item created by the tool plugin is selected by the user.
	\param iCommandID Id of the command
*/
void OtmToolsLauncherPlugin::processCommand( int iCommandID )
{
  if ( iCommandID == ID_TOOLSLAUNCHER_COMMAND )
  {
       STARTUPINFOA si;
       memset(&si, 0, sizeof(STARTUPINFO));
       si.cb = sizeof(STARTUPINFO);
       si.hStdOutput = NULL;
       si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
       si.wShowWindow = SW_HIDE;

       PROCESS_INFORMATION pi;
       DWORD nRC = NO_ERROR;
       if (CreateProcessA(NULL, "OpenTM2ToolsLauncher.exe", NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
       {
            WaitForSingleObject(pi.hProcess, INFINITE);

            if (!GetExitCodeProcess(pi.hProcess, &nRC))
            {
                nRC = GetLastError();
            }
       }
  }
}


bool OtmToolsLauncherPlugin::stopPlugin( bool fForce  )
{

  // TODO: check for active objects..
  bool fActiveObjects = false;

  // decline stop if we have active objects
  if ( !fForce && fActiveObjects )
  {
    return( false );
  }

  // TODO: terminate active objects, cleanup, free allocated resources

  // de-register plugin
	PluginManager *pPluginManager = PluginManager::getInstance();
	pPluginManager->deregisterPlugin( (OtmPlugin *)this );

  return( true );
}

extern "C" {
__declspec(dllexport)
USHORT registerPlugins()
{
	PluginManager::eRegRc eRc = PluginManager::eSuccess;
	PluginManager *manager = PluginManager::getInstance();
	OtmToolsLauncherPlugin* plugin = new OtmToolsLauncherPlugin();
	eRc = manager->registerPlugin((OtmPlugin*) plugin);
    USHORT usRC = (USHORT) eRc;
    return usRC;
}
}

void executeCommand()
{

}