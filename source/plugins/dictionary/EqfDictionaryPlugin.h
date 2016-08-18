/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _EqfDictionaryPlugin_H_
#define _EqfDictionaryPlugin_H_

#include <string>
#include "core\PluginManager\OtmDictionaryPlugin.h"

class OtmDictionary;

class EqfDictionaryPlugin: public OtmDictionaryPlugin
/*! \brief This class implements the standard (EQF) dictionary plugin for OpenTM2.
*/

{
public:
/*! \brief Constructor */
	EqfDictionaryPlugin();
/*! \brief Destructor */
	~EqfDictionaryPlugin();
/*! \brief Returns the name of the plugin */
	const char* getName();
/*! \brief Returns a short plugin-Description */
	const char* getShortDescription();
/*! \brief Returns a verbose plugin-Description */
	const char* getLongDescription();
/*! \brief Returns the version of the plugin */
	const char* getVersion();
/*! \brief Returns the name of the plugin-supplier */
	const char* getSupplier();
/*! \brief Creates a dictionary */
	OtmDictionary* CreateDict();

/*! \brief Stops the plugin. 
	Terminating-function for the plugin, will be called directly before
	the DLL containing the plugin will be unloaded.\n
	The method should call PluginManager::deregisterPlugin() to tell the PluginManager
  that the plugin is not active anymore.
  Warning: THIS METHOD SHOULD BE CALLED BY THE PLUGINMANAGER ONLY!
	\param fForce, TRUE = force stop of the plugin even if functions are active, FALSE = only stop plugin when it is inactive
	\returns TRUE when successful */
	bool stopPlugin( bool fForce = false );

 
/*! \cond old wrapper stuff */
USHORT CPP_AsdBegin(USHORT usMaxDicts, PHUCB phUCB);
USHORT CPP_AsdEnd(HUCB hUCB);
USHORT CPP_AsdOpen(HUCB hUCB, USHORT usOpenFlags, USHORT usNumDicts, PSZ *ppszDicts, PHDCB phDCB, PUSHORT pusErrDict);
USHORT CPP_AsdClose(HUCB hUCB, HDCB hDCB);
/*! \endcond */

private:
	std::string name;
	std::string shortDesc;
	std::string longDesc;
	std::string version;
	std::string supplier;
};

#endif // #ifndef _EqfDictionaryPlugin_H_
