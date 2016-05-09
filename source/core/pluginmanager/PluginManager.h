/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _PLUGINMANAGER_H_
#define _PLUGINMANAGER_H_

#include "OtmPlugin.h"
#include "OtmToolPlugin.h"
#include "PluginListener.h"

// Add for P403138 start
#define PLUGIN_MISS_MARKUP        4        // missing markup
#define PLUGIN_MISS_DICT          2        // missing dictionary
#define PLUGIN_MISS_MEMORY        1        // missing memory

#define STR_MEMORY_PLUGIN         "memory plugin"
#define STR_DICT_PLUGIN           "dictionary plugin"
#define STR_MARKUP_PLUGIN         "markup plugin"

#define COMMA_STR                 ","
// Add end

class PluginManagerImpl;

/*! \brief The PluginManager controls loading of the plugins and calling their 
	appropriate member-functions. Plugins must register with the PluginManager.
    
	The OpenTM2 PluginManager class is a singleton and represents only an
	interface.\n
	To access the PluginManager object, use the static method getInstance().
	An instance of this class will be created automatically.\n
	Implementation details can be found in the PluginManagerImpl class.
*/

class __declspec(dllexport) PluginManager
{
public:

/*! \brief This static method returns a pointer to the PluginManager object.
	The first call of the method creates the PluginManager-instance.
*/
	static PluginManager* getInstance();

/*! \enum eRegRc
	Possible returnvalues of registerPlugin()- and deregisterPlugin()-methods.
*/
	enum eRegRc
	{
		eSuccess,			/*!< plugin was successfully registered */
		eInvalidParm,		/*!< the passed parameter is invalid */
		eInvalidName,		/*!< plugin-name is invalid */
		eAlreadyRegistered,	/*!< plugin with same name was already registered before */
		eInvalidRequest,	/*!< method may only be called from within registerPlugins call */
        ePluginExpired,     /*!< plugin expired, add for P402974 */
		eUnknown,			/*!< plugin with that name was not registered before */
	};

/*! \brief Registers a plugin with the PluginMananger and adds it to the list
	of available plugins.
	\param plugin Pointer to the plugin to be registered.
	\returns Result of the call.
	\retval eSuccess plugin was successfully registered
	\retval eInvalidParm the passed parameter is invalid (e.g. NULL)
	\retval eInvalidName the plugins name is invalid (e.g. empty)
	\retval eAlreadyRegistered a plugin with the same name is already registered
	\retval eInvalidRequest method may only be called from within registerPlugins call
	\note This method can only be called during the loading-process of the plugin-DLL
	(i.e. from within the C-function registerPlugins()). This is necessary,
	because the path to the plugin-DLL has to be known for the registering-process.
*/
	eRegRc registerPlugin(OtmPlugin* plugin);

/*! \brief Deregisters a plugin (removes it from the list of available plugins).
	\param plugin Pointer to the plugin to be deregistered.
	\returns Result of the call.
	\retval eSuccess plugin was successfully deregistered
	\retval eInvalidParm the passed parameter is invalid (e.g. NULL)
	\retval eInvalidName the plugins name is invalid (e.g. empty)
	\retval eUnknown the plugin has not been registered before (is unknown)
*/
	eRegRc deregisterPlugin(OtmPlugin* plugin);

/*! \brief Returns the number of available plugins.
	\returns Number of available plugins.
	If no plugin has been added yet, the return value is 0.
*/
	int getPluginCount();

/*! \brief Returns a pointer to the plugin with the specified name.
	\param name Name of the plugin
	\returns Pointer to the plugin, NULL if no plugin with that name is registered.
*/
	OtmPlugin* getPlugin(const char* name);

/*! \brief Returns a pointer to the plugin with the specified index (0-based).
	\param iIndex Number of the plugin
	\returns Pointer to the plugin, NULL if no plugin with that index is registered.
*/
	OtmPlugin* getPlugin(int iIndex);

/*! \brief Returns a pointer to the n-th plugin with the given type.
	\param type Type of the plugin
	\param n number of the requested plugin (this can be used to fetch all plugins of a certain type)
	\returns Pointer to the plugin, NULL if no plugin with that type and/or number is registered.
*/
	OtmPlugin* getPlugin(OtmPlugin::ePluginType type, int n = 0);

/*! \brief Load all DLLs from the plugin-directory, resolve and call their register-function.
*/
	unsigned short loadPluginDlls(const char* pszPluginDir);

/*! \brief Stop the given plugin and unload the plugin DLL
	\param pPlugin pointer to the plugin object
	\param fForce, TRUE = force stop of the plugin, FALSE = only stop plugin when it is inactive
	\returns TRUE when successful
*/
	bool stopPlugin( OtmPlugin* pPlugin, bool fForce = false );

/*! \brief Stop the given plugin and unload the plugin DLL
	\param pszPluginName name of the plugin beiing stopped
	\param fForce, TRUE = force stop of the plugin, FALSE = only stop plugin when it is inactive
	\returns TRUE when successful
*/
	bool stopPlugin( const char *pszPluginName, bool fForce = false );

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

  int ValidationCheck(char * strParam);  // Add for P403138

private:

/*! \brief Constructor is private.
*/
	PluginManager() {};

/*! \brief Copy-Constructor is private.
*/
	PluginManager(const PluginManager&) {};

/*! \brief Destructor is private.
*/
	~PluginManager() {};

/*! \brief Pointer to the instance of the PluginManager object (singleton).
*/
	static PluginManager* instance;

/*! \brief Pointer to the implementation class.
*/
	PluginManagerImpl* pImpl;

};

#endif // #ifndef _PLUGINMANAGER_H_
