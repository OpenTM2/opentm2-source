/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef MORPHFACTORY_H_
#define MORPHFACTORY_H_

#include <set>
#include <string>
#include <vector>
#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMorphPlugin.h"
#include "core\PluginManager\OtmMorph.h"
#include "core\utilities\LogWriter.h"

/*! \brief   This class provides factory methods for morphology objects 
*/
class __declspec(dllexport) MorphFactory
{
public:

	/*! \brief constructor	 */
	MorphFactory(void);

	/*! \brief destructor	 */
	~MorphFactory(void);

/*! \brief This static method returns a pointer to the MorphFactory object.
	The first call of the method creates the MorphFactory instance.
*/
	static MorphFactory* getInstance();

/*! \brief Delete the MorphFactory instance if it exists.
*/
	static void close();

/* \brief get a OtmMorph object
   \param vName the language name
   \returns pointer to OtmMorph object 
*/
	OtmMorph* getMorph(
		const char* pszName,
		const char* pszPluginName = NULL
		);

/* \brief get information of the specified plug-in.
   \param vPluginName name of the plug-in
   \param vName the language name
   \param vInfo pointer to the info structure that restores info.
   \returns 0 means success.
*/
	int getMorphInfo(
		const char* pszName, 
		OtmMorphPlugin::PMORPHINFO pInfo,
		const char* pszPluginName = NULL
		);

	  /*! \brief get the language support list of the specific plugin, if the plugin name is null, the result will contain all the langauges
      \ that is supported by the MorphFactory class.
      \param vPluginName pointer to the name of the plugin
      \param vLanguageList reference to the vector that contains all the languages supported by the MorphFactory class
      \returns 0 means success, other value means error!
   */
	int getLanguageList(
		const char* pszPluginName, 
		vector<string>& vLanguageList
		);

/* \brief check if the given language is supported by any of the available Morph plugins
   \param vName the language name
   \returns TRUE if the language is supported and FALS when no plugin fo rthis language is available
*/
  bool isSupported (const char* pszName );

/*! \brief get morphology plug-in by name
	\param vPluginName pointer to the plug-in name
	\returns pointer to the OtmMorphPlugin
 */
	OtmMorphPlugin* getPlugin(const char* pszPluginName);

	/*! \brief refresh the morphology plug-in list */
	void refreshPluginList();

	/*! \brief the single MorphFactory instance	 */
	static MorphFactory* pMorphInstance;

	/*! \brief the list of morphology plug-in	 */
	vector<OtmMorphPlugin *> *pvPluginList;

	/*! \brief LogWriter object for logging
	*/
	LogWriter Log;

};

#endif