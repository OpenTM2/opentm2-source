/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include "OtmMorphICUPlugin.h"
#include "OtmMorphICU.h"
#include "core\PluginManager\PluginManager.h"

// the static plugin infos
static char *pszPluginName = "OtmMorphICUPlugin";
static char *pszShortDescription = "MorphICUPlugin";
static char *pszLongDescription	= "This is the standard morphology functionality implementation using ICU";
static char *pszVersion = STR_DRIVER_LEVEL_NUMBER;
static char *pszSupplier = "International Business Machines Corporation";

/*! \brief constructor	 */
OtmMorphICUPlugin::OtmMorphICUPlugin(void)
{
	strName = pszPluginName;
	strShortDesc = pszShortDescription;
	strLongDesc = pszLongDescription;
	strVersion = pszVersion;
	strSupplier = pszSupplier;
	pluginType = OtmPlugin::eMorphType;
	usableState = OtmPlugin::eUsable;
}

/*! \brief destructor	 */
OtmMorphICUPlugin::~OtmMorphICUPlugin(void)
{
	for (map<string, OtmMorph*>::iterator tIter = mMorphMap.begin(); tIter != mMorphMap.end(); tIter++)
	{
		delete tIter->second;
	}
}

/*! \brief Returns the name of the plug-in
*/
const char* OtmMorphICUPlugin::getName()
{
	return strName.c_str();
}

/*! \brief Returns a short plug-in Description
*/
const char* OtmMorphICUPlugin::getShortDescription()
{
	return strShortDesc.c_str();
}

/*! \brief Returns a long plug-in Description
*/
const char* OtmMorphICUPlugin::getLongDescription()
{
	return strLongDesc.c_str();
}

/*! \brief Returns the version of the plug-in
*/
const char* OtmMorphICUPlugin::getVersion()
{
	return strVersion.c_str();
}

/*! \brief Returns the name of the plug-in supplier
*/
const char* OtmMorphICUPlugin::getSupplier()
{
	return strSupplier.c_str();
}

/*! \brief open the morphology object of the specified language
	\param vLanguage pointer to the language
	\returns pointer to the morphology object or NULL if failed
 */
OtmMorph* OtmMorphICUPlugin::openMorph( const char* pszLanguage )
{
	string strLanguage = string(pszLanguage);
	map<string, OtmMorph*>::iterator tIter = mMorphMap.find(strLanguage);
	if (tIter != mMorphMap.end())
	{
		return tIter->second;
	}
	else
	{
		OtmMorph* pMorphObject = new OtmMorphICU(pszLanguage);
		if (NULL == pMorphObject)
		{
			return NULL;
		}
		mMorphMap.insert(pair<string, OtmMorph*>(strLanguage, pMorphObject));
		return pMorphObject;
	}
}

/*! \brief get the morphology object's info of the specified language
	\param vName pointer to the language
	\param vInfo pointer to the structure that restores the morphology object's info
	\returns 0 means success
 */
int OtmMorphICUPlugin::getMorphInfo( const char* pszLanguage, PMORPHINFO pInfo )
{
	if (NULL == pszLanguage || NULL == pInfo)
	{
		return OtmMorphPlugin::eParameterError;
	}

	strcpy_s(pInfo->szLanguage, 50, pszLanguage);
	strcpy_s(pInfo->szDescription, 250, strLongDesc.c_str());
	strcpy_s(pInfo->szName, 50, strName.c_str());
	pInfo->bEnabled = true;

	return 0;
}

 /*! \brief get the language support list of the plugin
  \ that is supported by the OtmMorphICUPlugin class.
  \param vLanguageList reference to the vector that contains all the languages supported by the OtmMorphICUPlugin class
  \returns 0 means success, other value means error!
 */
int OtmMorphICUPlugin::getLanguageList(
	vector<string>& vLanguageList
	)
{
    // TODO: Add ICU Code to get available language list
	return 0;
}


/*! \brief check if a specific language is supported by this plugin
	\param vName pointer to the language
	\returns 1 when the language is supported and 0 if it is not supported
 */
int OtmMorphICUPlugin::isSupported(
	const char* pszLanguage
	)
{
	if (NULL == pszLanguage)
	{
		return 0;
	}

	// TODO: Add ICU Code to check if language is supported
  return 1;
}

bool OtmMorphICUPlugin::stopPlugin( bool fForce  )
{

  // TODO: check for active objects..
  bool fActiveObjects = false;

  // decline stop if we have active objects
  if ( !fForce && fActiveObjects )
  {
    return( false );
  }

  // TODO: terminate active objects, cleanup, free allocated resources

  // de-register plugin
	PluginManager *pPluginManager = PluginManager::getInstance();
	pPluginManager->deregisterPlugin( (OtmPlugin *)this );

  return( true );
}

extern "C" {
	__declspec(dllexport)
		USHORT registerPlugins()
	{
		PluginManager::eRegRc eRc = PluginManager::eSuccess;
		PluginManager *pManager = PluginManager::getInstance();
		OtmMorphICUPlugin* pMorphPlugin = new OtmMorphICUPlugin();
		eRc = pManager->registerPlugin((OtmPlugin*) pMorphPlugin);
        USHORT usRC = (USHORT) eRc;
        return usRC;
	}
}


extern "C" {
  __declspec(dllexport)
  unsigned short getPluginInfo( POTMPLUGININFO pPluginInfo )
  {
    strcpy( pPluginInfo->szName, pszPluginName );
    strcpy( pPluginInfo->szShortDescription, pszShortDescription );
    strcpy( pPluginInfo->szLongDescription, pszLongDescription );
    strcpy( pPluginInfo->szVersion, pszVersion );
    strcpy( pPluginInfo->szSupplier, pszSupplier );
    pPluginInfo->eType = OtmPlugin::eMorphType;
    strcpy( pPluginInfo->szDependencies, "" );
    pPluginInfo->iMinOpenTM2Version= -1;
    return( 0 );
  }
}
