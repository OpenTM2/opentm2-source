/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _EqfDocumentPlugin_H_
#define _EqfDocumentPlugin_H_

#include <string>
#include "core\PluginManager\OtmDocumentPlugin.h"

class OtmDocument;

class EqfDocumentPlugin: public OtmDocumentPlugin
/*! \brief This class implements the standard document plugin (EQF) for OpenTM2.
*/

{
public:
/*! \brief Constructor
*/
	EqfDocumentPlugin();
/*! \brief Destructor
*/
	~EqfDocumentPlugin();
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

	/*! \brief Create an (empty) work-document.
	Creates a document instance and returns a pointer to it. */
	OtmDocument* createDocument();

/*! \brief Open a document.
	Creates a document instance and returns a pointer to it.
	This checks for existence and accessibility of the associated files,
	also fills the property information.
	The files are read when method OtmDocument::read() is called. */
	OtmDocument* open(char* pszDocumentName);

/*! \brief Close a document.
	This checks for unsaved changes and closes all associated files. */
	void close(OtmDocument* pDocument);

/*! \brief Deletes a document.
	This deletes the document instanceall associated files from disk. 
	The document must have been closed before calling this method. */
	bool destroy(OtmDocument* pDocument);

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
};

#endif // #ifndef _EqfDocumentPlugin_H_