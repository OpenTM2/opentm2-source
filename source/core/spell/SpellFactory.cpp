/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmSpellPlugin.h"
#include "core\PluginManager\OtmSpell.h"
#include "core\spell\SpellFactory.h"


#ifdef _DEBUG
  // activate define to enable spell logging
  //#define SPELLFACTORYLOGGING

#endif
/*! \brief the SpellFactory instance	 */
SpellFactory* SpellFactory::pSpellFactoryInstance = NULL;

/*! \brief constructor	 */
SpellFactory::SpellFactory(void)
{
	pvPluginList = NULL;
}

/*! \brief destructor	 */
SpellFactory::~SpellFactory(void)
{
	if (NULL != pvPluginList)
	{
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			delete pvPluginList->at(i);
		}
		delete pvPluginList;
	}
	pvPluginList = NULL;
}

/*! \brief This static method returns a pointer to the SpellFactory object.
	The first call of the method creates the SpellFactory instance.
*/
SpellFactory* SpellFactory::getInstance()
{
	if (NULL == pSpellFactoryInstance)
	{
		pSpellFactoryInstance = new SpellFactory();
		pSpellFactoryInstance->refreshPluginList();

#ifdef SPELLFACTORY_LOGGING
		pSpellFactoryInstance->objLog.open("SpellFactory");
		pSpellFactoryInstance->objLog.writef("Get instance of SpellFactory!\n");
#endif
	}
	return pSpellFactoryInstance;
}

void SpellFactory::refreshPluginList()
{

#ifdef SPELLFACTORY_LOGGING
	objLog.writef("Refresh plugin list\n");
#endif
	if (NULL != pvPluginList)
	{
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			delete pvPluginList->at(i);
		}
		delete pvPluginList;
#ifdef SPELLFACTORY_LOGGING
		objLog.writef("Delete plugin list\n");
#endif
	}

	// access plugin manager
	PluginManager* thePluginManager = PluginManager::getInstance();

	OtmSpellPlugin * curPlugin = NULL;
	pvPluginList = new std::vector<OtmSpellPlugin *>;
	int i = 0;
	do
	{
		i++;
		curPlugin = (OtmSpellPlugin*) thePluginManager->getPlugin(OtmPlugin::eSpellType, i );
		if ( curPlugin != NULL ) pvPluginList->push_back( curPlugin );
	}  while ( curPlugin != NULL ); /* end */     
}

/*! \brief get the language support list of the specific plugin, if the plugin name is null, the result will contain all the langauges
  \ that is supported by the SpellFactory class.
  \param vPluginName pointer to the name of the plugin
  \param vLanguageList reference to the vector that contains all the languages supported by the SpellFactory class
  \returns 0 means success, other value means error!
*/
int SpellFactory::getLanguageList(
	const char* vPluginName, 
	vector<string>& vLanguageList
	)
{
	if (NULL == vPluginName || (strcmp(vPluginName, "") == 0))
	{
#ifdef SPELLFACTORY_LOGGING
		objLog.writef("get all the langauges supported by the spellfactory class!\n");
#endif
		set<string> tLanguageList;
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			OtmSpellPlugin* pSpellPlugin = pvPluginList->at(i);
			vector<string> vSpecficLanguageList;
			if (0 == pSpellPlugin->getLanguageList(vSpecficLanguageList))
			{
				for (int j = 0; j < vSpecficLanguageList.size(); j++)
				{
					tLanguageList.insert(vSpecficLanguageList[j]);
				}
			}
		}

		for (set<string>::iterator tIter = tLanguageList.begin(); tIter != tLanguageList.end(); tIter++)
		{
			vLanguageList.push_back(*tIter);
		}
	}
	else
	{
		OtmSpellPlugin* pSpellPlugin = getPlugin(vPluginName);
		if (NULL != pSpellPlugin)
		{
			return pSpellPlugin->getLanguageList(vLanguageList);
		}
	}
}

/*! \brief get spell check plug-in by name
	\param vPluginName pointer to the plug-in name
	\returns pointer to the OtmSpellPlugin
 */
OtmSpellPlugin* SpellFactory::getPlugin( const char* pszPluginName )
{
#ifdef SPELLFACTORY_LOGGING
	objLog.writef("get plug-in %s\n", pszPluginName);
#endif
	if(pSpellFactoryInstance == NULL)
	{
		pSpellFactoryInstance = new SpellFactory();
		pSpellFactoryInstance->refreshPluginList();
	}
	if (NULL != pszPluginName)
	{
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			if (0 == strcmp(pszPluginName, pvPluginList->at(i)->getName()))
			{
				return pvPluginList->at(i);
			}
		}
	}
	if (0 < pvPluginList->size())
	{
		return pvPluginList->at(0);
	}

#ifdef SPELLFACTORY_LOGGING
	objLog.writef("get plug-in %s\n", pszPluginName);
#endif
	return NULL;
}

/* \brief get a spell checker object
   \param vName the language name
   \param vPluginName name of the plug-in
   \returns pointer to spell checker object 
*/
OtmSpell* SpellFactory::getSpellChecker(const char* pszName, const char* pszPluginName)
{
	if ( (pszPluginName == NULL) || (strcmp( pszPluginName, "" ) == 0) )
	{
#ifdef SPELLFACTORY_LOGGING
		objLog.writef("searching spell checker for language %s\n", pszName );
#endif
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			OtmSpellPlugin* pSpellPlugin = pvPluginList->at(i);
			OtmSpell *tSpellChecker = pSpellPlugin->openSpell(pszName);
			if ( tSpellChecker != NULL )
			{
				return( tSpellChecker );
			}
		}
#ifdef SPELLFACTORY_LOGGING
		objLog.writef("no spell checker found for language %s\n", pszName );
#endif
	}
	else
	{
#ifdef SPELLFACTORY_LOGGING
		objLog.writef("get spell checker of %s and %s \n", pszName, pszPluginName);
#endif
		OtmSpellPlugin* pSpellPlugin = getPlugin(pszPluginName);
		if (NULL != pSpellPlugin)
		{
			return pSpellPlugin->openSpell(pszName);
		}
#ifdef SPELLFACTORY_LOGGING
		objLog.writef("get spell checker of %s and %s failed\n", pszName, pszPluginName);
#endif
	}

	return NULL;
}

/* \brief check if the given language is supported by any of the available spell plugins
   \param vName the language name
   \returns TRUE if the language is supported and FALS when no plugin for this language is available
*/
bool SpellFactory::isSupported(const char* pszName )
{
  for (int i = 0; i < (int)pvPluginList->size(); i++)
	{
	  OtmSpellPlugin* pSpellPlugin = pvPluginList->at(i);
  	if ( pSpellPlugin->isSupported( pszName ) )
    {
#ifdef SPELLFACTORY_LOGGING
	  objLog.writef("SpellFactory::isSupported(): The language %s is supported", pszName);
#endif
      return( TRUE );
    }
  }

#ifdef SPELLFACTORY_LOGGING
  objLog.writef("SpellFactory::isSupported(): The language %s is not supported", pszName);
#endif
	return FALSE;
}

/* \brief get spell info of the specified plug-in.
   \param vPluginName name of the plug-in
   \param vName the language name
   \param vInfo pointer to the spell info struct that restores spell info.
   \returns ERROR_PARAMETAR means error, 0 means success.
*/
int SpellFactory::getSpellInfo( const char* pszPluginName, const char* pszName, OtmSpellPlugin::PSPELLINFO pInfo )
{
#ifdef SPELLFACTORY_LOGGING
	objLog.writef("get spell info of %s and %s \n", pszName, pszPluginName);
#endif
	OtmSpellPlugin* tSpellPlugin = getPlugin(pszPluginName);
	if (NULL != tSpellPlugin)
	{
		return tSpellPlugin->getSpellInfo(pszName, pInfo);
	}

#ifdef SPELLFACTORY_LOGGING
	objLog.writef("get spell info of %s and %s failed\n", pszName, pszPluginName);
#endif
	return OtmSpell::ERROR_PARAMETAR;
}

/*! \brief Delete the SpellFactory instance if it exists.
*/
void SpellFactory::close()
{
	if (NULL != pSpellFactoryInstance)
	{
		delete pSpellFactoryInstance;
		pSpellFactoryInstance = NULL;
	}
#ifdef SPELLFACTORY_LOGGING
	objLog.close();
#endif
}

// check if a spellchecker is availabe for the given language
BOOL isSpellCheckerAvailable( PSZ pszLanguage )
{
  SpellFactory* pSpellFactoryInstance = SpellFactory::getInstance();
	if (pSpellFactoryInstance != NULL )
	{
    return ( pSpellFactoryInstance->isSupported( pszLanguage ) );
  } 
  return( FALSE );
}

