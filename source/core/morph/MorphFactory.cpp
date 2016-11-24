/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMorphPlugin.h"
#include "core\PluginManager\OtmMorph.h"
#include "core\morph\MorphFactory.h"


/*! \brief the MorphFactory instance	 */
MorphFactory* MorphFactory::pMorphInstance = NULL;

/*! \brief constructor	 */
MorphFactory::MorphFactory(void)
{
	pvPluginList = NULL;
}

/*! \brief destructor	 */
MorphFactory::~MorphFactory(void)
{
	if (NULL != pvPluginList)
	{
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			delete pvPluginList->at(i);
		}
		delete pvPluginList;
		pvPluginList = NULL;
	}
}

/*! \brief This static method returns a pointer to the MorphFactory object.
	The first call of the method creates the MorphFactory instance.
*/
MorphFactory* MorphFactory::getInstance()
{
	if (NULL == pMorphInstance)
	{
		pMorphInstance = new MorphFactory();
		pMorphInstance->refreshPluginList();
#ifdef _DEBUG
		pMorphInstance->Log.open("MorphFactory");
#endif
		pMorphInstance->Log.writef("Get instance of MorphFactory!\n");
	}
	return pMorphInstance;
}

/*! \brief refresh the morphology plug-in list */
void MorphFactory::refreshPluginList()
{
	Log.writef("Refresh plugin list\n");
	if (NULL != pvPluginList)
	{
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			delete pvPluginList->at(i);
		}
		delete pvPluginList;
		Log.writef("Delete plugin list\n");
	}

	// access plugin manager
	PluginManager* pPluginManager = PluginManager::getInstance();

	OtmMorphPlugin * pOtmPlugin = NULL;
	pvPluginList = new std::vector<OtmMorphPlugin *>;
	int i = 0;
	do
	{
		i++;
		pOtmPlugin = (OtmMorphPlugin*) pPluginManager->getPlugin(OtmPlugin::eMorphType, i );
		if ( pOtmPlugin != NULL ) pvPluginList->push_back( pOtmPlugin );
	}  while ( pOtmPlugin != NULL ); /* end */     
}

OtmMorphPlugin* MorphFactory::getPlugin( const char* pszPluginName )
{
	Log.writef("MorphFactory::getPlugin(): First, get plug-in %s\n", pszPluginName);

	if(NULL != pszPluginName)
	{
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			if (0 == strcmp(pszPluginName, pvPluginList->at(i)->getName()))
			{
				Log.writef("MorphFactory::getPlugin(): Success, get plug-in %s at %d \n", pszPluginName, i);
				return pvPluginList->at(i);
			}
		}
	}

	if (0 < pvPluginList->size())
	{
		Log.writef("MorphFactory::getPlugin(): Success, get the similar plug-in of %s at 0 \n", pszPluginName);
		return pvPluginList->at(0);
	}

	Log.writef("MorphFactory::getPlugin(): Error occured when getting plug-in %s\n", pszPluginName);
	return NULL;
}

/* \brief get a Morph checker object
   \param vName the language name
   \param vPluginName name of the plug-in
   \returns pointer to Morph object 
*/
OtmMorph* MorphFactory::getMorph(const char* pszName, const char* pszPluginName)
{

	if ( (pszPluginName == NULL) || (strcmp( pszPluginName, "" ) == 0) )
	{
		Log.writef("searching Morph for language %s\n", pszName );
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			OtmMorphPlugin* pMorphPlugin = pvPluginList->at(i);
			OtmMorph *pMorphChecker = pMorphPlugin->openMorph(pszName);
			if ( pMorphChecker != NULL )
			{
				return( pMorphChecker );
			}
		}
		Log.writef("no Morph found for language %s\n", pszName );
	}
	else
	{
		Log.writef("get Morph of %s and %s \n", pszName, pszPluginName);
		OtmMorphPlugin* pOtmMorphPlugin = getPlugin(pszPluginName);
		if (NULL != pOtmMorphPlugin)
		{
			return pOtmMorphPlugin->openMorph(pszName);
		}
		Log.writef("get Morph of %s and %s failed\n", pszName, pszPluginName);
	}

	return NULL;
}

/*! \brief get the language support list of the specific plugin, if the plugin name is null, the result will contain all the languages
  \ that is supported by the MorphFactory class.
  \param vPluginName pointer to the name of the plugin
  \param vLanguageList reference to the vector that contains all the languages supported by the MorphFactory class
  \returns 0 means success, other value means error!
*/
int MorphFactory::getLanguageList(
	const char* pszPluginName, 
	vector<string>& vLanguageList
	)
{
	if (NULL == pszPluginName || (strcmp(pszPluginName, "") == 0))
	{
		Log.writef("get all the languages supported by the MorphFactory class!\n");
		set<string> setLanguageList;
		for (int i = 0; i < (int)pvPluginList->size(); i++)
		{
			OtmMorphPlugin* pOtmMorphPlugin = pvPluginList->at(i);
			vector<string> vSpecficLanguageList;
			if (0 == pOtmMorphPlugin->getLanguageList(vSpecficLanguageList))
			{
				for (int j = 0; j < vSpecficLanguageList.size(); j++)
				{
					setLanguageList.insert(vSpecficLanguageList[j]);
				}
			}
		}

		for (set<string>::iterator tIter = setLanguageList.begin(); tIter != setLanguageList.end(); tIter++)
		{
			vLanguageList.push_back(*tIter);
		}
	}
	else
	{
		OtmMorphPlugin* pOtmMorphPlugin = getPlugin(pszPluginName);
		if (NULL != pOtmMorphPlugin)
		{
			return pOtmMorphPlugin->getLanguageList(vLanguageList);
		}
	}
}


/* \brief check if the given language is supported by any of the available Morph plugins
   \param vName the language name
   \returns TRUE if the language is supported and FALS when no plugin for this language is available
*/
bool MorphFactory::isSupported(const char* pszName )
{
  for (int i = 0; i < (int)pvPluginList->size(); i++)
	{
	  OtmMorphPlugin* pOtmMorphPlugin = pvPluginList->at(i);
  	if ( pOtmMorphPlugin->isSupported( pszName ) )
    {
	  Log.writef("MorphFactory::isSupported(): The language %s is supported", pszName);
      return( TRUE );
    }
  }

  Log.writef("MorphFactory::isSupported(): The language %s is not supported", pszName);
	return FALSE;
}

/* \brief get Morph info of the specified plug-in.
   \param vPluginName name of the plug-in
   \param vName the language name
   \param vInfo pointer to the Morph info struct that restores Morph info.
   \returns ERROR_PARAMETAR means error, 0 means success.
*/
int MorphFactory::getMorphInfo(const char* pszName, OtmMorphPlugin::PMORPHINFO pInfo, const char* pszPluginName)
{
	Log.writef("MorphFactory::getMorphInfo(): First, get morphology info of %s plug-in.\n", pszPluginName);
	OtmMorphPlugin* pOtmMorphPlugin = getPlugin(pszPluginName);
	if (NULL != pOtmMorphPlugin)
	{
		Log.writef("MorphFactory::getMorphInfo(): Success, get morphology info of %s plug-in.\n", pszPluginName);
		return pOtmMorphPlugin->getMorphInfo(pszName, pInfo);
	}

	Log.writef("MorphFactory::getMorphInfo(): Error, get morphology info of %s plug-in failed.\n", pszPluginName);
	return OtmMorph::ERROR_PARAMETAR;
}

/*! \brief Delete the MorphFactory instance if it exists.
*/
void MorphFactory::close()
{
	if (NULL != pMorphInstance)
	{
		delete pMorphInstance;
		pMorphInstance = NULL;
	}
}

// check if morphologic support is is availabe for the given language
BOOL isMorphSupportAvailable( PSZ pszLanguage )
{
  MorphFactory* pMorphFactoryInstance = MorphFactory::getInstance();
	if (pMorphFactoryInstance != NULL )
	{
    return ( pMorphFactoryInstance->isSupported( pszLanguage ) );
  } 
  return( FALSE );
}