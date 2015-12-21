/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef OtmMorphICUPlugin_h__
#define OtmMorphICUPlugin_h__

#include "core\PluginManager\OtmMorphPlugin.h"
#include "map"
#include "string"
using namespace std;

/*! \brief This class implements the standard morphology plug-in (EQF) for OpenTM2.
*/
class OtmMorphICUPlugin :
	public OtmMorphPlugin
{
public:

	/*! \brief constructor	 */
	OtmMorphICUPlugin(void);

	/*! \brief destructor	 */
	~OtmMorphICUPlugin(void);

	/*! \brief Returns the name of the plug-in
	*/
	const char* getName();

	/*! \brief Returns a short plug-in Description
	*/
	const char* getShortDescription();

	/*! \brief Returns a long plug-in Description
	*/
	const char* getLongDescription();

	/*! \brief Returns the version of the plug-in
	*/
	const char* getVersion();

	/*! \brief Returns the name of the plug-in supplier
	*/
	const char* getSupplier();

	/*! \brief open the morphology object of the specified language
		\param vLanguage pointer to the language
		\returns pointer to the morphology object or NULL if failed
	 */
	OtmMorph* openMorph(
		const char* pszLanguage
		);

	/*! \brief get the morphology object's info of the specified language
		\param vName pointer to the language
		\param vInfo pointer to the structure that restores the morphology object's info
		\returns 0 means success
	 */
	int getMorphInfo(
		const char* pszName, 
		PMORPHINFO pInfo
		);

	 /*! \brief get the language support list of the plugin
      \ that is supported by the OtmMorphICUPlugin class.
      \param vLanguageList reference to the vector that contains all the languages supported by the OtmMorphICUPlugin class
      \returns 0 means success, other value means error!
     */
	int getLanguageList(
		vector<string>& vLanguageList
		);

  /*! \brief check if a specific language is supported by this plugin
	  \param vName pointer to the language
	  \returns 1 when the language is supported and 0 if it is not supported
	*/
	int isSupported(
	  const char* pszLanguage
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

	/*! \brief map that restores the morphology objects	 */
	map<string, OtmMorph*> mMorphMap;

};


#endif // OtmMorphICUPlugin_h__
