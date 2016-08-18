/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OtmMarkupTablePlugin_H_
#define _OtmMarkupTablePlugin_H_

#include "core\pluginmanager\OtmMarkupPlugin.h"
#include "core\pluginmanager\OtmMarkup.h"
#include "core\utilities\LogWriter.h"

#include "OtmMarkupTable.h"

class OtmMarkup;

class OtmMarkupTablePlugin: public OtmMarkupPlugin
/*! \brief Translation memory plugin
	This plugin implements the OTM markup table plugin for OpenTM2.
*/

{
public:
/*! \brief Constructor
*/
  OtmMarkupTablePlugin();

/*! \brief Destructor
*/
	~OtmMarkupTablePlugin();

/*! \brief Returns the name of the plugin
*/
	virtual const char* getName();

/*! \brief Returns a short plugin description
*/
	virtual const char* getShortDescription();

/*! \brief Returns a short plugin name        
*/
	virtual const char* getShortName();

/*! \brief Returns a verbose plugin description
*/
	virtual const char* getLongDescription();

/*! \brief Returns plugin directory name
*/
  	virtual const char* getPluginDirectory();

/*! \brief Returns markup table TBL directory name
*/
  	virtual const char* getTableDirectory();

/*! \brief Returns markup table TBL directory path
*/
  	virtual const char* getTablePath();
    
/*! \brief Returns markup table user exit directory name
*/
  	virtual const char* getUserExitDirectory();

/*! \brief Returns markup table user exit directory path
*/
  	virtual const char* getUserExitPath();

/*! \brief Returns the version of the plugin
*/
	virtual const char* getVersion();

/*! \brief Returns the name of the plugin-supplier
*/
	virtual const char* getSupplier();

/*! \brief Returns the names of all files which make up this markup table
*/
	virtual const char* getAllFiles();

/*! \brief Returns TRUE if the markup table can be exported
*/
	virtual const bool isExportable();

/*! \brief Returns TRUE if the markup table can be imported
*/
	virtual const bool isImportable();

/*! \brief Returns TRUE if the markup table can be deleted 
*/
	virtual const bool isDeletable();

/*! \brief Returns TRUE if the markup table is protected
*/
	virtual const bool isProtected();

/*! \brief Returns TRUE if the markup table is expired
*/
	virtual const bool isExpired();

/*! \brief Returns 1 if the markup table files were updated
*/
	virtual const int updateFiles(
       char   *pszMarkupName,
       char   *pszShortDescription,
       char   *pszLongDescription,
       char   *pszVersion,
       char   *pszTableFileName,
       char   *pszUserExitFileName,
       char   *pszFileList
    );

/*! \brief Returns 1 if the markup table information was updated
*/
	virtual const int updateInfo( 
       char   *pszMarkupName,
       char   *pszShortDescription,
       char   *pszLongDescription,
       char   *pszVersion,
       char   *pszUserExitFileName
	);

/*! \brief Returns TRUE if the markup table was deleted
*/
    virtual const bool deleteMarkup(
       char   *pszMarkupName 
    );

/*! \brief Get the number of markup tables provided by this plugin
	\returns Number of markup tables
*/
	int getCount();
	
/*! \brief Get the markup table at the given index

	\param iIndex index of the markup table in the range 0 to (getCount - 1)
	\returns Pointer to the requested markup table object or NULL when iIndex is out of range
*/
	OtmMarkup* getMarkup
  (
		int iIndex
	);

	/*! \brief Get the markup table object for a markup table

	\param pszMarkup Name of the markup table
	\returns Pointer to the requested markup table object or NULL when markup table does
	Not belong to this markup table plugin
*/
	OtmMarkup* getMarkup
  (
		char *pszMarkup
	);

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
    char *pszFileList;
    char *pszLongDescription;
	char *pszName;
	char *pszShortDescription;
	char *pszShortName;
	char *pszSupplier;
	char *pszTableDirectory;
	char *pszTablePath;
	char *pszUserExitDirectory;
	char *pszUserExitPath;
	char *pszVersion;
    bool bProtected;
    bool bDeletable;
    bool bExportable;
    bool bImportable;
    bool bExpired;
    int  iMarkupCount;

  // base path of the plugin DLL
  char szBasePath[1024];

  LogWriter Log;
};



#endif // #ifndef _OtmMarkupTablePlugin_H_