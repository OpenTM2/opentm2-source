/*! \file
	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OTMDOCUMENTPLUGIN_H_
#define _OTMDOCUMENTPLUGIN_H_

#include "OtmPlugin.h"
#include "OtmDocument.h"

/*! \brief Abstract base-class for plugins handling documents */
class __declspec(dllexport) OtmDocumentPlugin: public OtmPlugin
{

public:

/*! \brief Constructor */
	OtmDocumentPlugin()
		{pluginType = eDocumentType;};

/*! \brief Destructor */
	virtual ~OtmDocumentPlugin()
		{};

/*! \brief Returns the "usable"-state of the plugin-object. */
	virtual bool isUsable()
		{return OtmPlugin::isUsable();};

/*! \brief Create an (empty) work-document.
	Creates a document instance and returns a pointer to it. */
	virtual OtmDocument* createDocument() = 0;

/*! \brief Open a document.
	Creates a document instance and returns a pointer to it.
	This checks for existence and accessibility of the associated files,
	also fills the property information.
	The files are read when method OtmDocument::read() is called. */
	virtual OtmDocument* open(char* pszDocumentName) = 0;

/*! \brief Close a document.
	This checks for unsaved changes and closes all associated files. */
	virtual void close(OtmDocument* pDocument) = 0;

/*! \brief Deletes a document.
	This deletes the document instanceall associated files from disk. 
	The document must have been closed before calling this method. */
	virtual bool destroy(OtmDocument* pDocument) = 0;

};

#endif // #ifndef _OTMDOCUMENTPLUGIN_H_
 
