/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef OtmSpellHSPlugin_h__
#define OtmSpellHSPlugin_h__

#include "core\PluginManager\OtmSpellPlugin.h"

/*! \brief This class implements the standard spell check plug-in (EQF) for OpenTM2.
*/
class OtmSpellHSPlugin :
	public OtmSpellPlugin
{
public:

	/*! \brief constructor	 */
	OtmSpellHSPlugin(void);

	/*! \brief destructor	 */
	~OtmSpellHSPlugin(void);

	/*! \brief Returns the name of the plugin
	*/
	const char* getName();

	/*! \brief Returns a short plugin-Description
	*/
	const char* getShortDescription();

	/*! \brief Returns a long plugin-Description
	*/
	const char* getLongDescription();

	/*! \brief Returns the version of the plugin
	*/
	const char* getVersion();

	/*! \brief Returns the name of the plugin-supplier
	*/
	const char* getSupplier();

	/*! \brief open the spellchecking object of the specified language
		\param vName pointer to the language
		\returns pointer to the spellchecking object or NULL if failed
	 */
	OtmSpell* openSpell(
		const char* pszName
		);

	/*! \brief get the spellchecking object's info of the specified language
		\param vName pointer to the language
		\param vInfo pointer to the structure that restores the spellchecking object's info
		\returns 0 means success
	 */
	int getSpellInfo(
		const char* pszName, 
		PSPELLINFO pInfo
		);

	  /*! \brief check if a specific language is supported by this plugin
	  \param vName pointer to the language
	  \returns 1 when the language is supported and 0 if it is not supported
	*/
	int isSupported(
	  const char* pszLanguage
	  );

	 /*! \brief get the language support list of the plugin
      \ that is supported by the OtmSpellHSPlugin class.
      \param vLanguageList reference to the vector that contains all the languages supported by the OtmSpellHSPlugin class
      \returns 0 means success, other value means error!
     */
	int getLanguageList(
		vector<string>& vLanguageList
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

	/*! \brief the name of the plug-in	 */
	string strName;

	/*! \brief short description of the plug-in	 */
	string strShortDesc;

	/*! \brief 	detailed description of the plug-in */
	string strLongDesc;

	/*! \brief version of the plug-in	 */
	string strVersion;

	/*! \brief supplier of the plug-in	 */
	string strSupplier;

	/*! \brief map that restores the spell checking object's info	 */
	map<string, PSPELLINFO> mSpellInfoMap;

	/*! \brief map that restores the spell checking objects	 */
	map<string, OtmSpell*> mSpellMap;

};

#endif // OtmSpellHSPlugin_h__