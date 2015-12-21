/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OTMTOOLPLUGIN_H_
#define _OTMTOOLPLUGIN_H_

#include "OtmPlugin.h"

/*! \brief Abstract base-class for tools plugins 
*/
class __declspec(dllexport) OtmToolPlugin: public OtmPlugin
{

public:

/*! \brief Constructor.
*/
	OtmToolPlugin()
		{pluginType = eToolType;};

/*! \brief Destructor.
*/
	virtual ~OtmToolPlugin()
		{};

	virtual bool isUsable()
		{return OtmPlugin::isUsable();};

	virtual const char* getName() = 0;

	virtual const char* getShortDescription() = 0;

	virtual const char* getLongDescription() = 0;

	virtual const char* getVersion() = 0;

	virtual const char* getSupplier() = 0;

/*! \brief Initialize plugin 
  This method is called when the workbench window has been created.
  The tool plugin can use the plugin manager methods addMenuItem and addSubMenu to add
  own entries to the workbench toolbar.
*/
  virtual void init() = 0;

  /*! \brief Process an action bar command
  This method is called when a menu item created by the tool plugin is selected by the user.
	\param iCommandID Id of the command
*/
  virtual void processCommand( int iCommandID ) = 0;


/*! \brief Stops the plugin. 
	Terminating-function for the plugin, will be called directly before
	the DLL containing the plugin will be unloaded.\n
	The method should call PluginManager::deregisterPlugin() to tell the PluginManager
  that the plugin is not active anymore.
  Warning: THIS METHOD SHOULD BE CALLED BY THE PLUGINMANAGER ONLY!
	\param fForce, TRUE = force stop of the plugin even if functions are active, FALSE = only stop plugin when it is inactive
	\returns TRUE when successful */
	virtual bool stopPlugin( bool fForce = false ) = 0;

};

#endif // #ifndef _OTMTOOLPLUGIN_H_
 