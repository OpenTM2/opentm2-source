/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OtmCleanupPlugin_H_
#define _OtmCleanupPlugin_H_

#include <string>
#include <vector>
#include "core\pluginmanager\OtmToolPlugin.h"


class OtmCleanupPlugin: public OtmToolPlugin
/*! \brief This class implements the standard translation memory plugin (EQF) for OpenTM2.
*/

{
public:
/*! \brief Constructor
*/
	OtmCleanupPlugin();
/*! \brief Destructor
*/
	~OtmCleanupPlugin();
/*! \brief Returns the name of the plugin
*/
	const char* getName();
/*! \brief Returns a short plugin-Description
*/
	const char* getShortDescription();
/*! \brief Returns a verbose plugin-Description
*/
	const char* getLongDescription();
/*! \brief Returns the version of the plugin
*/
	const char* getVersion();
/*! \brief Returns the name of the plugin-supplier
*/
	const char* getSupplier();

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
  void processCommand( int iCommandID );



/*! \brief Stops the plugin. 
	Terminating-function for the plugin, will be called directly before
	the DLL containing the plugin will be unloaded.\n
	The method should call PluginManager::deregisterPlugin() to tell the PluginManager
  that the plugin is not active anymore.
  Warning: THIS METHOD SHOULD BE CALLED BY THE PLUGINMANAGER ONLY!
	\param fForce, TRUE = force stop of the plugin even if functions are active, FALSE = only stop plugin when it is inactive
	\returns TRUE when successful */
	bool stopPlugin( bool fForce = false );

 
private:
	std::string name;
	std::string shortDesc;
	std::string longDesc;
	std::string version;
	std::string supplier;
	std::string strLastError;

};


#endif // #ifndef _OtmCleanupPlugin_H_