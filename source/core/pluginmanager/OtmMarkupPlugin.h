//	Copyright (C) 1990-2016, International Business Machines
//	Corporation and others. All rights reserved
#ifndef _OTMMARKUPPLUGIN_H_
#define _OTMMARKUPPLUGIN_H_

#include "OtmPlugin.h"
#include "OtmMarkup.h"

/*! \brief Abstract base-class for plugins handling markup tables
*/
class __declspec(dllexport) OtmMarkupPlugin: public OtmPlugin
{

public:

/*! \brief Constructor.
*/
	OtmMarkupPlugin()
		{pluginType = eMarkupType;};

/*! \brief Destructor.
*/
	virtual ~OtmMarkupPlugin()
		{};

	virtual bool isUsable()
		{return OtmPlugin::isUsable();};

	virtual const char* getName() = 0;

	virtual const char* getShortName() = 0;

	virtual const char* getShortDescription() = 0;

	virtual const char* getLongDescription() = 0;

	virtual const char* getVersion() = 0;

	virtual const char* getPluginDirectory() = 0;

	virtual const char* getTableDirectory() = 0;

	virtual const char* getUserExitDirectory() = 0;

	virtual const char* getTablePath() = 0;

	virtual const char* getUserExitPath() = 0;

	virtual const char* getSupplier() = 0;

	virtual const int updateFiles(
       char   *pszMarkupName,
       char   *pszShortDescription,
       char   *pszLongDescription,
       char   *pszVersion,
       char   *pszTableFileName,
       char   *pszUserExitFileName,
       char   *pszFileList
    ) = 0;

	virtual const int updateInfo( 
       char   *pszMarkupName,
       char   *pszShortDescription,
       char   *pszLongDescription,
       char   *pszVersion,
       char   *pszUserExitFileName
	) = 0;

	virtual const bool deleteMarkup(
       char   *pszMarkupName
    ) = 0;

/*! \brief Get the number of markup tables provided by this plugin
	\returns Number of markup tables
*/
	virtual int getCount(
	) = 0;
	
/*! \brief Get the markup table at the given index
	\param iIndex index of the markup table in the range 0 to (getCount - 1)
	\returns Pointer to the requested markup table object or NULL when iIndex is out of range
*/
	virtual OtmMarkup* getMarkup
  (
		int iIndex
	) = 0;

/*! 	\brief Get the markup table object for a markup table
	\param pszMarkup Name of the markup table
	\returns Pointer to the requested markup table object or NULL when markup table does
	not belong to this markup table plugin
*/
	virtual OtmMarkup* getMarkup
  (
		char *pszMarkup
	) = 0;

};

#endif // #ifndef _OTMMARKUPPLUGIN_H_
 