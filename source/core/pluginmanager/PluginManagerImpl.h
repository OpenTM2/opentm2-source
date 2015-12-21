/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _PLUGINMANAGERIMPL_H_
#define _PLUGINMANAGERIMPL_H_

#include <set>
#include <vector>
#include <core\utilities\LogWriter.h>

class OtmPlugin;
class PluginListener;

/*! \structure LOADEDPLUGINDLL
	\brief used to store information on a plugin DLL and the contained plugins
*/
struct LoadedPluginDLL
{
  HMODULE hMod;                        // DLL module handle
  char strDll[MAX_PATH];               // Add for P402792, dll path
  std::vector<OtmPlugin*> vPluginList; // list of plugins contained in DLL
};

/*! \typedef PLUGINSET
	\brief used to store all registered plugins
*/
typedef std::set<OtmPlugin*> PLUGINSET;

/*! \brief Implementation part of the PluginManager.
	Uses a std::set to manage the registered plugins.\n
*/
class PluginManagerImpl
{
public:

/*! \brief Creates an empty set of plugins. */
PluginManagerImpl();

/*! \brief Deletes the set of plugins */
virtual ~PluginManagerImpl();

/*! \brief Add a plugin to the set. Each plugin must have a unique name. */
PluginManager::eRegRc registerPlugin(OtmPlugin* plugin);

/*! \brief Remove a plugin from the set. */
PluginManager::eRegRc deregisterPlugin(OtmPlugin* plugin);

/*! \brief Return the number of registered plugins. */
int getPluginCount();

/*! \brief Return a pointer to the plugin with a specified name. */
OtmPlugin* getPlugin(const char* name);

/*! \brief Return a pointer to the plugin with a specified index (0-based). */
OtmPlugin* getPlugin(int iIndex);

/*! \brief Return a pointer to the first plugin with the given type. */
OtmPlugin* getPlugin(OtmPlugin::ePluginType type, int n = 0);

/*! \brief Find all DLLs in the plugin-directory (and subdirectories) and load them. */
USHORT loadPluginDlls(const char* pszPluginDir);

/*! \brief Load the specified DLL, resolve and call the register-function. */
USHORT loadPluginDll(const char* pszName);

/*! \brief Stop the given plugin and unload the plugin DLL
	\param pPlugin pointer to the plugin object
	\param fForce, TRUE = force stop of the plugin, FALSE = only stop plugin when it is inactive
	\returns TRUE when successful
*/
	bool stopPlugin( OtmPlugin* pPlugin, bool fForce = FALSE );

/*! \brief Stop the given plugin and unload the plugin DLL
	\param pszPluginName name of the plugin beiing stopped
	\param fForce, TRUE = force stop of the plugin, FALSE = only stop plugin when it is inactive
	\returns TRUE when successful
*/
	bool stopPlugin( const char *pszPluginName, bool fForce = FALSE );

/*! \brief Stops all registered plugins in forced mode and unloads the plugin DLLs
	\returns TRUE when successful
*/
	bool stopAllPlugins();

/*! \brief Add a plugin listener
	\param pPluginListener pointer to the plugin listener object
	\returns TRUE when listener has been added successfully
*/
  bool addPluginListener( PluginListener *pListener );

/*! \brief Remove a plugin listener
	\param pPluginListener pointer to the plugin listener object
	\returns TRUE when listener has been removed successfully
*/
  bool removePluginListener( PluginListener *pListener );

/*! \brief Add a menu item to the OpenTM2 actionbar
	\param pToolPlugin pointer to the tool plugin processing the menu item
	\param pszMenuName name of the menu which is receiving the new menu item
	\param pszMenuItem name of the new menu item
	\param iCommandID command to be passed to tool plugin when the user selects the new menu item
	\returns TRUE when the menu item has been added successfully
*/
  bool addMenuItem( OtmToolPlugin *pToolPlugin, char *pszMenuName, char *pszMenuItem, int iCommandID );

/*! \brief Add a sub menu item to the OpenTM2 actionbar
	\param pToolPlugin pointer to the tool plugin processing the menu item
	\param pszMenuName name of the menu which is receiving the new sub menu
	\param pszSubMenu name of the new sub menu
	\returns TRUE when the sub menu has been added successfully
*/
  bool addSubMenu( OtmToolPlugin *pToolPlugin, char *pszMenuName, char *pszSubMenu );

/*! \brief Activate all tool plugins. This method is called by the workbench once the main window has been created
*/
  void activateToolPlugins();

/*! \brief Process a tool plugin command. This method is called by the workbench when a menu item of a tool plugin is selected
	\param iCommandID ID of selected menu item
*/
  void processToolCommand( int iCommandID );

private:

/*! \brief Structure used for list of tool plugin menu items */
  typedef struct _TOOLPLUGINMENUITEM
  {
    OtmToolPlugin *pPlugin;       // tool plugin for this menu item
    int iCommandID;               // command ID to be passed tool plugin
    int iMenuItemID;              // ID used for the menu item
    CHAR szMenuName[40];          // buffer for menu name
  } TOOLPLUGINMENUITEM, *PTOOLPLUGINMENUITEM;

/*! \brief List of tool plugin menu items */
std::vector<TOOLPLUGINMENUITEM> m_ToolPluginMenuItems; 

/*! \brief ID to be used for the next menu item added to the menu bar */
int m_iNextCommandID; 

/*! \brief Notify all listeners 
	\param eNotifcation The type of notigication
  \param pszPluginName The name of the plugin
  \param eType The type of the plugin
  \param fForce TRUE when plugin is being forced
	\returns TRUE when listeners have been notified successfully
*/
  bool NotifyListeners( PluginListener::eNotifcationType eNotifcation, const char *pszPluginName, OtmPlugin::ePluginType eType, bool fForce );

  /*! \brief Find the loaded plugin DLL entry for a plugin
	\param pPlugin pointer to the plugin object
	\returns index of loaded plugin DLL entry or -1
*/
  int findPluginEntry( OtmPlugin *pPlugin );

/*! \brief Copy-Constructor is private */
PluginManagerImpl(const PluginManager&) {};

/*! \brief Assignment-operator is private */
PluginManagerImpl& operator=(const PluginManagerImpl&) {};

/*! \brief A set containing all registered plugins. */
PLUGINSET* pluginSet;

/*! \brief List of loaded plugin DLLs and plugins contained in the DLL. */
std::vector<LoadedPluginDLL> vLoadedPluginDLLs;

/*! \brief Index of currently loaded plugin DLL, -1 = no DLL load is active. */
int iCurrentlyLoadedPluginDLL;

/*! \brief Flag to allow/disallow the call of the registerPlugin()-method */
bool bRegisterAllowed;

/*! \brief List of registered plugin listeners */
std::vector<PluginListener *> m_vPluginListener;

/*! \function checked whether the path of the plugin has overed the max alled depth */
BOOL IsDepthOvered(const char * strPath);

LogWriter Log;

/*! \brief search for a sub menu with the givem name (used recursively) */
HMENU findSubMenu( HMENU hMenu, const char *pszSubMenu );

};

#endif // #ifndef _PLUGINMANAGERIMPL_H_
