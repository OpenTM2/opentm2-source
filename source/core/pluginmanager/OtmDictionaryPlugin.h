/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OTMDICTIONARYPLUGIN_H_
#define _OTMDICTIONARYPLUGIN_H_

#include "OtmPlugin.h"
#include "OtmDictionary.h"

/*! \brief Abstract base-class for plugins handling dictionaries */
class __declspec(dllexport) OtmDictionaryPlugin: public OtmPlugin
{

public:

/*! \brief Constructor */
	OtmDictionaryPlugin()
		{pluginType = eDictionaryType;};

/*! \brief Destructor */
	virtual ~OtmDictionaryPlugin()
		{};

	virtual bool isUsable()
		{return OtmPlugin::isUsable();};

	virtual const char* getName() = 0;

	virtual const char* getShortDescription() = 0;

	virtual const char* getLongDescription() = 0;

	virtual const char* getVersion() = 0;

	virtual const char* getSupplier() = 0;

	virtual OtmDictionary* CreateDict() = 0;

/*! \brief Stops the plugin. 
	Terminating-function for the plugin, will be called directly before
	the DLL containing the plugin will be unloaded.\n
	The method should call PluginManager::deregisterPlugin() to tell the PluginManager
  that the plugin is not active anymore.
  Warning: THIS METHOD SHOULD BE CALLED BY THE PLUGINMANAGER ONLY!
	\param fForce, TRUE = force stop of the plugin even if functions are active, FALSE = only stop plugin when it is inactive
	\returns TRUE when successful */
	virtual bool stopPlugin( bool fForce = false ) = 0;

/*! \cond old wrapper stuff */
virtual USHORT CPP_AsdBegin(USHORT usMaxDicts, PHUCB phUCB) = 0;
virtual USHORT CPP_AsdEnd(HUCB hUCB) = 0;
virtual USHORT CPP_AsdOpen(HUCB hUCB, USHORT usOpenFlags, USHORT usNumDicts, PSZ *ppszDicts, PHDCB phDCB, PUSHORT pusErrDict) = 0;
virtual USHORT CPP_AsdClose(HUCB hUCB, HDCB hDCB) = 0;
/*! \endcond */

};

#endif // #ifndef _OTMDICTIONARYPLUGIN_H_
 
