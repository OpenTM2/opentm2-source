/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/
#include <eqf.h>                  // General Translation Manager include file
#include "HunspellObjManager.h"

/*! \brief instance of HunspellObjManager */
HunspellObjManager* HunspellObjManager::pInstance = NULL;

/*! \brief Constructor */
HunspellObjManager::HunspellObjManager(void)
{
	getDirectory();
#ifdef _DEBUG
	hunspellObjManagerLog.open("HunspellObjManager");
#endif

 	if(readLanguageConfig() > 0)
 		hunspellObjManagerLog.writef("read language configuration file success!");
 	else
 		hunspellObjManagerLog.writef("read language configuration file failed!");
}

/*! \brief Destructor */
HunspellObjManager::~HunspellObjManager(void)
{
	map<string, PHUNSPELLINFO>::iterator tIter = mHunspellInfo.begin();
	while(tIter != mHunspellInfo.end())
	{
		PHUNSPELLINFO tInfo = tIter->second;
		if (tInfo != NULL)
		{
			delete tInfo->pHunspell;
			delete tInfo;
		}
		tIter++;
	}
}

/*! \brief get the language support list of the plugin
\param vLanguageList reference to the vector that contains all the languages supported by the HunspellObjManager class
\returns 0 means success, other value means error!
*/
int HunspellObjManager::getLanguageList(
vector<string>& vLanguageList
)
{
	vector<string>::iterator tIter = mLanguageList.begin();
	for (; tIter != mLanguageList.end(); tIter++)
	{
		vLanguageList.push_back(*tIter);
	}

	return 0;
}

/*! \brief Get configuration file from the spell check plugin directory.
*/
string HunspellObjManager::getConfigFileName()
{
	CHAR   szName[MAX_PATH144];               // string for output name
	memset(szName, 0, sizeof(szName));
	UtlMakeEQFPath ( szName, NULC, PLUGIN_PATH, NULL );
	strcat( szName, "\\OtmSpellHSPlugin\\LanguageConfig.lng" );
	strConfigFile = szName;

	hunspellObjManagerLog.writef("Get config file name: %s\n", strConfigFile.c_str());
	return strConfigFile;
}

/*! \brief Read the language name and dictionary name from the configure file.
*! \returns 1 means success, other value means error.
*/
int HunspellObjManager::readLanguageConfig()
{
	getConfigFileName();

	FILE* pfFilePointer;
	if (0 == fopen_s(&pfFilePointer, strConfigFile.c_str(), "r"))
	{

		hunspellObjManagerLog.writef("open the language configuration file %s success fully!", strConfigFile.c_str());
		char szWord[MAX_PATH144];
		memset(szWord, 0, sizeof(szWord));
		char szLanguageName[MAX_PATH144];
		char szDicName[MAX_PATH144];

		while (!feof(pfFilePointer) && !ferror(pfFilePointer) && 0 != fgets(szWord, sizeof(szWord), pfFilePointer))
		{
      if ( szWord[0] != '*' )
      {
			  memset(szLanguageName, 0, sizeof(szLanguageName));
			  memset(szDicName, 0, sizeof(szLanguageName));
			  size_t len = strlen(szWord);
			  if (len >= 10)
			  {
				  if (szWord[len - 1] == '\n')
				  {
					  szWord[len - 1] = 0;	
					  len--;
				  }
				  char* pszLanguageNameLocation = strstr(szWord, LANGUAGENAMESIG);
				  if (NULL == pszLanguageNameLocation)
				  {
					  continue;
				  }
				  pszLanguageNameLocation++;
				  pszLanguageNameLocation += strlen(LANGUAGENAMESIG);

				  char* pszDictionaryLocation = strstr(szWord, DICTIONARYNAMESIG);
				  if (NULL == pszDictionaryLocation)
				  {
					  continue;
				  }

				  memcpy(szLanguageName, pszLanguageNameLocation, pszDictionaryLocation - pszLanguageNameLocation - 1);

				  pszDictionaryLocation++;
				  pszDictionaryLocation += strlen(DICTIONARYNAMESIG);
				  strcpy(szDicName, pszDictionaryLocation);

				  if (0 < strlen(szLanguageName) && 0 < strlen(szDicName))
				  {
            mLanguageList.push_back( szLanguageName );
            _strupr( szLanguageName );
					  mLanguageNameInfo.insert(pair<string, string>(szLanguageName, szDicName));
					  hunspellObjManagerLog.writef("add language %s and dictionary name %s\n", szLanguageName, szDicName);
				  }

			  }
			  else
				  continue;
      } /* endif */         
			memset(szWord, 0, sizeof(szWord));
		}
		fclose(pfFilePointer);
	}
	else
		return -1;

	return 1;
}


/*! \brief get instance of the HunspellObjManager class
	\returns pointer to the HunspellObjManager instance 
 */
HunspellObjManager* HunspellObjManager::getInstance()
{
	if (NULL == pInstance)
	{
		pInstance = new HunspellObjManager();
	}

	return pInstance;
}

/*! \brief check if support for the given language is available
	\param vLanguageId pointer to the language
	\returns TRUE is spell support for the language is available, otherwise FALSE is returend
 */
boolean HunspellObjManager::isAvailable( const char* pszLanguage )
{
  string strHunspellLanguage = getHunspellLanguageName( pszLanguage );
  if ( strHunspellLanguage.length() == 0 )
	{
		return FALSE;
	}
  string strDicName = getDicName( strHunspellLanguage.c_str() );
	string strDictFile = strDicDirectory + strDicName;
	
  OFSTRUCT OpenBuff;

  return( OpenFile( strDictFile.c_str(), &OpenBuff, OF_EXIST ) != -1);
}

/*! \brief get the Hunspell language name for an openTM2 language
	\param vLanguage pointer to the OpenTM2 language name
	\returns pointer to the hunspell language name
 */
const char *HunspellObjManager::getHunspellLanguageName( const char* vLanguage )
{
 	if (NULL == vLanguage)
	{
		return "";
	}
  char szLanguage[80];
  strcpy( szLanguage, vLanguage );
  _strupr( szLanguage );
	map<string, string>::iterator tNameIter = mLanguageNameInfo.find(szLanguage);
	if (mLanguageNameInfo.end() == tNameIter)
  {
    return "";
  }
  return( tNameIter->second.c_str() );
}

/*! \brief get Hunspell instance by the language id
	\param vLanguageId pointer to the language
	\returns pointer to the HUNSPELLINFO struct
 */
PHUNSPELLINFO HunspellObjManager::getObjectByLanguage( const char* vLanguage )
{
	if (NULL == vLanguage)
	{
		return NULL;
	}

  char szLanguage[80];
  strcpy( szLanguage, vLanguage );
  _strupr( szLanguage );

	map<string, PHUNSPELLINFO>::iterator tIter = mHunspellInfo.find(szLanguage);
	if (tIter != mHunspellInfo.end())
	{
		return tIter->second;
	}
	else 
	{
		map<string, string>::iterator tNameIter = mLanguageNameInfo.find(szLanguage);
		if (mLanguageNameInfo.end() == tNameIter)
		{		
      return NULL;
		}
  	string strLangaugeRealName = tNameIter->second;
		string strLanguage = strDicDirectory + strLangaugeRealName;
		string strDicName = getDicName(strLanguage.c_str());
		string strAffName = getAffName(strLanguage.c_str());
		Hunspell* pHunspell = new Hunspell(strAffName.c_str(), strDicName.c_str());
		if (NULL == pHunspell)
		{
			return NULL;
		}
		PHUNSPELLINFO tInfo = new HUNSPELLINFO;
		tInfo->pHunspell = pHunspell;
		tInfo->language = szLanguage;
		mHunspellInfo.insert(pair<string, PHUNSPELLINFO>(szLanguage, tInfo));
		return tInfo;
	}
}

/*! \brief get dictionary name for the language	 
	\param pointer to the language
	\returns string that restores the dictionary name
 */
string HunspellObjManager::getDicName( const char* pszLanguage )
{
	if (NULL == pszLanguage)
	{
		return "";
	}

	string tStr = pszLanguage;
	return tStr + ".dic";
}

/*! \brief get affix name for the language	 
	\param pointer to the language
	\returns string that restores the affix name
 */
string HunspellObjManager::getAffName( const char* pszLanguage )
{
	if (NULL == pszLanguage)
	{
		return "";
	}

	string tStr = pszLanguage;
	return tStr + ".aff";
}

/*! \brief get the directory of dictionaries.
    \returns a string that contains the directory.
 */
string HunspellObjManager::getDirectory()
{
	CHAR   szName[MAX_PATH144];               // string for output name
	memset(szName, 0, sizeof(szName));
	UtlMakeEQFPath ( szName, NULC, PLUGIN_PATH, NULL );
	strcat( szName, "\\OtmSpellHSPlugin\\DICT\\" );
	strDicDirectory = szName;

	hunspellObjManagerLog.writef("Get dictionary file path: %s\n", strDicDirectory.c_str());
	return strDicDirectory;
}
