/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef HunspellObjManager_h__
#define HunspellObjManager_h__

#include <Windows.h>
#include "AutoLock.h"
#include "core\PluginManager\OtmSpell.h"
#include "core\utilities\LogWriter.h"

#ifdef near
#undef near
#endif
#define HUNSPELL_STATIC
#include "hunspell/hunspell.hxx"
#pragma comment(lib, "libhunspell.lib")

#ifndef LANGUAGENAMESIG
#define LANGUAGENAMESIG "LanguageName"
#endif

#ifndef DICTIONARYNAMESIG
#define DICTIONARYNAMESIG "Dictionary"
#endif


/*! \brief struct for Hunspell object */
typedef struct _HUNSPELLINFO 
{
	/*! \brief the language id	 */
	string language;
	/*! \brief the hunspell object	 */
	Hunspell* pHunspell;
	/*! \brief the lock to keep hunspell object thread-safe	 */
	ThreadLock objLock;

	/*! \brief initialization	 */
	_HUNSPELLINFO()
	{
		pHunspell = NULL;
	}
}HUNSPELLINFO, *PHUNSPELLINFO;

/*! \brief Manage the Hunspell object, since Spell Check and Morphlogy both need Hunspell object.
 */
class HunspellObjManager
{
public:
	
	/*! \brief Destructor	 */
	~HunspellObjManager(void);

	/*! \brief get instance of the HunspellObjManager class
		\returns pointer to the HunspellObjManager instance 
	 */
	static HunspellObjManager* getInstance();

	/*! \brief get Hunspell instance by the language id
		\param vLanguage pointer to the language
		\returns pointer to the HUNSPELLINFO struct
	 */
	PHUNSPELLINFO getObjectByLanguage(const char* pszLanguage);

	/*! \brief check if support for the given language is available
	\param vLanguageId pointer to the language
	\returns TRUE is spell support for the language is available, otherwise FALSE is returned
 */
  boolean isAvailable( const char* pszLanguage );

	/*! \brief get the directory of dictionaries.
		\returns a string that contains the directory.
	*/
	string getDirectory();

   /*! \brief Read the language name and dictionary name from the configure file.
       \returns 1 means success, other value means error.
   */
  int readLanguageConfig();

  /*! \brief Get configuration file from the spell check plug-in directory.
      \returns the string that contains the path of the language-support configuration file
   */
  string getConfigFileName();

	/*! \brief get the language support list of the plugin
	\param vLanguageList reference to the vector that contains all the languages supported by the HunspellObjManager class
	\returns 0 means success, other value means error!
	*/
	int getLanguageList(
	vector<string>& vLanguageList
	);

	/*! \brief Constructor	 */
	HunspellObjManager(void);

  /*! \brief get the Hunspell language name for an openTM2 language
	\param vLanguage pointer to the OpenTM2 language name
	\returns pointer to the hunspell language name
 */
  const char *getHunspellLanguageName( const char* vLanguage );

private:

	/*! \brief get dictionary name for the language	 
		\param pointer to the language
		\returns string that restores the dictionary name
	 */
	string getDicName(
		const char* pszLanguage
		);

	/*! \brief get affix name for the language	 
		\param pointer to the language
		\returns string that restores the affix name
	 */
	string getAffName(
		const char* pszLanguage
		);

 	/*! \brief list of supported languages (in mixed case as stored in the language property file) */
	vector <string> mLanguageList;
  
	/*! \brief pointer to the HunspellObjManager instance	 */
	static HunspellObjManager* pInstance;

	/*! \brief a map that restores hunspell objects info.	The key is the language name in uppercase */
	map<string, PHUNSPELLINFO> mHunspellInfo;

	/*! \brief dictionary directory	 */
	string strDicDirectory;

	/*! \brief a map that restores the language name info. The key is the language name in uppercase 	 */
	map<string, string> mLanguageNameInfo;

	/*! \brief LogWriter object for logging
	*/
	LogWriter hunspellObjManagerLog;

	/*! \brief the path of the language-support configuration file	 */
	string strConfigFile;
	
};

#endif // HunspellObjManager_h__


