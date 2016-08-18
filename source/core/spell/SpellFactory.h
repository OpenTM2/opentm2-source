/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef SPELLFACTORY_H_
#define SPELLFACTORY_H_

#include <string>
#include <vector>
#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmSpellPlugin.h"
#include "core\PluginManager\OtmSpell.h"
#include "core\utilities\LogWriter.h"

/*! \brief   This class provides factory methods for spell check objects 
*/
class __declspec(dllexport) SpellFactory
{
public:

	/*! \brief constructor	 */
	SpellFactory(void);

	/*! \brief destructor	 */
	~SpellFactory(void);

/*! \brief This static method returns a pointer to the SpellFactory object.
	The first call of the method creates the SpellFactory instance.
*/
	static SpellFactory* getInstance();

/*! \brief Delete the SpellFactory instance if it exists.
*/
	static void close();

/* \brief get a spell checker object
   \param vName the language name
   \param vPluginName name of the plug-in
   \returns pointer to spell checker object 
*/
	OtmSpell* getSpellChecker(
		const char* pszName,
		const char* pszPluginName = NULL
		);

/* \brief get spell info of the specified plug-in.
   \param vPluginName name of the plug-in
   \param vName the language name
   \param vInfo pointer to the spell info struct that restores spell info.
   \returns ERROR_PARAMETAR means error, 0 means success.
*/
	int getSpellInfo(
		const char* pszPluginName,
		const char* pszName, 
		OtmSpellPlugin::PSPELLINFO pInfo
		);

/* \brief check if the given language is supported by any of the available spell plugins
   \param vName the language name
   \returns TRUE if the language is supported and FAILS when no plugin fo rthis language is available
*/
  bool isSupported (const char* pszName );

  /*! \brief get the language support list of the specific plugin, if the plugin name is null, the result will contain all the langauges
      \ that is supported by the SpellFactory class.
      \param vPluginName pointer to the name of the plugin
      \param vLanguageList reference to the vector that contains all the languages supported by the SpellFactory class
      \returns 0 means success, other value means error!
   */
	int getLanguageList(
		const char* pszPluginName, 
		vector<string>& vLanguageList
		);

	/*! \brief get spell check plug-in by name
		\param vPluginName pointer to the plug-in name
		\returns pointer to the OtmSpellPlugin
	 */
	OtmSpellPlugin* getPlugin(const char* pszPluginName);

	/*! \brief refresh plug-in list	 */
	void refreshPluginList();

	/*! \brief single instance of SpellFactory class	 */
	static SpellFactory* pSpellFactoryInstance;

	/*! \brief a vector of OtmSpellPlugin objects	 */
	vector<OtmSpellPlugin *> *pvPluginList;

	/*! \brief LogWriter object for logging
	*/
	LogWriter objLog;

};

#endif