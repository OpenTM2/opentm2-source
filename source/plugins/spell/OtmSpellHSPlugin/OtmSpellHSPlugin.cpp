/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include "OtmSpellHSPlugin.h"
#include "OtmSpellHS.h"
#include "core\PluginManager\PluginManager.h"

// the static plugin infos
static char *pszPluginName = "OtmSpellHSCheckPlugin";
static char *pszShortDescription = "SpellCheckPlugin";
static char *pszLongDescription	= "This is the standard Spell checking implementation";
static char *pszVersion = "1.0";
static char *pszSupplier = "International Business Machines Corporation";


/*! \brief constructor	 */
OtmSpellHSPlugin::OtmSpellHSPlugin(void)
{
	strName = pszPluginName;
	strShortDesc = pszShortDescription;
	strLongDesc = pszLongDescription;
	strVersion = pszVersion;
	strSupplier = pszSupplier;
	pluginType = OtmPlugin::eSpellType;
	usableState = OtmPlugin::eUsable;
}

/*! \brief destructor	 */
OtmSpellHSPlugin::~OtmSpellHSPlugin(void)
{
	for (map<string, PSPELLINFO>::iterator tIter = mSpellInfoMap.begin(); tIter != mSpellInfoMap.end(); tIter++)
	{
		delete tIter->second;
	}

	for (map<string, OtmSpell*>::iterator tIter = mSpellMap.begin(); tIter != mSpellMap.end(); tIter++)
	{
		delete tIter->second;
	}
}

/*! \brief Returns the name of the plugin
*/
const char* OtmSpellHSPlugin::getName()
{
	return strName.c_str();
}

/*! \brief Returns a short plugin-Description
*/
const char* OtmSpellHSPlugin::getShortDescription()
{
	return strShortDesc.c_str();
}

/*! \brief Returns a long plugin-Description
*/
const char* OtmSpellHSPlugin::getLongDescription()
{
	return strLongDesc.c_str();
}

/*! \brief Returns the version of the plugin
*/
const char* OtmSpellHSPlugin::getVersion()
{
	return strVersion.c_str();
}

/*! \brief Returns the name of the plugin-supplier
*/
const char* OtmSpellHSPlugin::getSupplier()
{
	return strSupplier.c_str();
}

/*! \brief open the spellchecking object of the specified language
	\param vName pointer to the language
	\returns pointer to the spellchecking object or NULL if failed
 */
OtmSpell* OtmSpellHSPlugin::openSpell(
	const char* pszName
	)
{
  string strLanguage = HunspellObjManager::getInstance()->getHunspellLanguageName(pszName);
	map<string, OtmSpell*>::iterator tIter = mSpellMap.find(strLanguage);
	if (tIter != mSpellMap.end())
	{
		return tIter->second;
	}
	else
	{
			OtmSpell* pSpellObject = new OtmSpellHS(pszName);
			if (NULL == pSpellObject)
			{
				return NULL;
			}
			mSpellMap.insert(pair<string, OtmSpell*>(strLanguage, pSpellObject));
			PSPELLINFO pSpellInfo = new SPELLINFO;
			strcpy_s(pSpellInfo->szName, 50, pszName);
			strcpy_s(pSpellInfo->szLanguage, 50, pszName);
			pSpellInfo->bEnabled = true;
			mSpellInfoMap.insert(pair<string, PSPELLINFO>(strLanguage, pSpellInfo));
			return pSpellObject;
	}
}

/*! \brief get the spellchecking object's info of the specified language
	\param vName pointer to the language
	\param vInfo pointer to the structure that restores the spellchecking object's info
	\returns 0 means success
 */
int OtmSpellHSPlugin::getSpellInfo(
	const char* pszLanguage, 
	PSPELLINFO pInfo
	)
{
	if (NULL == pszLanguage)
	{
		return -1;
	}
	string strLanguage = pszLanguage;
	map<string, PSPELLINFO>::iterator tIter = mSpellInfoMap.find(strLanguage);
	if (tIter != mSpellInfoMap.end())
	{
		pInfo = tIter->second;
		memcpy( pInfo, tIter->second, sizeof(OtmSpellPlugin::SPELLINFO) );
		return 0;
	}

	return OtmSpellPlugin::eNotFound;
}

/*! \brief check if a specific language is supported by this plugin
	\param vName pointer to the language
	\returns 1 when the language is supported and 0 if it is not supported
 */
int OtmSpellHSPlugin::isSupported(
	const char* pszLanguage
	)
{
	if (NULL == pszLanguage)
	{
		return 0;
	}

  return( HunspellObjManager::getInstance()->isAvailable( pszLanguage ) );
}

 /*! \brief get the language support list of the plugin
  \ that is supported by the OtmSpellHSPlugin class.
  \param vLanguageList reference to the vector that contains all the languages supported by the OtmSpellHSPlugin class
  \returns 0 means success, other value means error!
 */
int OtmSpellHSPlugin::getLanguageList(
	vector<string>& vLanguageList
	)
{
	return HunspellObjManager::getInstance()->getLanguageList(vLanguageList);
}

bool OtmSpellHSPlugin::stopPlugin( bool fForce  )
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
		OtmSpellHSPlugin* pSpellPlugin = new OtmSpellHSPlugin();
		eRc = pManager->registerPlugin((OtmPlugin*) pSpellPlugin);
		if (eRc == PluginManager::eSuccess)
		{
			//printf("OtmSpellHSPlugin registered successfully.\n");
		}
		else
		{
			//printf("OtmSpellHSPlugin NOT registered!\n");
		}

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
    pPluginInfo->eType = OtmPlugin::eSpellType;
    strcpy( pPluginInfo->szDependencies, "" );
    pPluginInfo->iMinOpenTM2Version= -1;
    return( 0 );
  }
}
