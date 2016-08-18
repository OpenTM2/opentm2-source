/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _LanguageFactory_H_
#define _LanguageFactory_H_

#include <string>
#include <vector>


class __declspec(dllexport) LanguageFactory
/*! \brief   This class provides a language name related utility factory 

*/

{
public:

/*! \brief Error code definition
*/
  static const int ERROR_LANGUAGENOTFOUND     = 9001;  // the specified language was not found in the languages table
  static const int ERROR_MISSINGPARAMETER     = 9002;  // one or more paramters are missing

	/*! \brief constructor	 */
	LanguageFactory(void);

	/*! \brief destructor	 */
	~LanguageFactory(void);

/*! \brief The layout of the language information provided per language
*/
typedef struct _LANGUAGEINFO
{
  char szName[128];                    // language name used in Opentm2
  long lAsciiCP;                       // ASCII codepage for the language
  long lAnsiCP;                        // Ansi codepage for the language
  char szIsoCPName[40];                // ISO name of the codepage
  char szIsoID[40];                    // ISO ID for the language
  char szIcuID[40];                    // ICU/BCP47 ID for the language
  BOOL fisPreferred;                   // TRUE = this is the preferred language if several languages use the same ISO/BCP47 ID
  char szAddInfo[256];                 // additional language information or comments
  BOOL isSourceLanguage;               // TRUE = language can be used as a source language    
  BOOL isTargetLanguage;               // TRUE = language can be used as a target language    
  char szLangGroup[128];               // language group of the language (will be used as language in the memory lookup)
} LANGUAGEINFO, *PLANGUAGEINFO;

  /*! \brief type of requested languages used by listLanguages method
  */
  typedef enum _LANGUAGETYPE { SOURCE_LANGUAGE_TYPE, TARGET_LANGUAGE_TYPE, ALL_LANGUAGES_TYPE } LANGUAGETYPE;

  /*! \brief type of callback function for listLanguages method */
  typedef int (*PFN_LISTLANGUAGES_CALLBACK )(PVOID pvData, char *pszName, PLANGUAGEINFO pInfo  );

  /*! \brief Provide a list of all available languages
  \param eType type of languages to be listed
  \param pfnCallBack callback function to be called for each language
  \param pvData caller's data pointer, is passed to callback function
  \param fWithDetails TRUE = supply language details, when this flag is set, 
  the pInfo parameter of the callback function is set otherwise it is NULL
  \returns number of provided languages
  */
  int listLanguages(
    LANGUAGETYPE eType,
	  PFN_LISTLANGUAGES_CALLBACK pfnCallBack,			  
	  void *pvData,
	  BOOL fWithDetails
  );


/*! \brief This static method returns a pointer to the LanguageFactory object.
	The first call of the method creates the LanguageFactory instance.
*/
	static LanguageFactory* getInstance();

/* \brief Get the ISO language identifier for a OpenTM2 language name
   \param pszLanguage OpenTM2 language name
   \param pszISO buffer for the language ISO identifier
   \returns 0 when successful or an error code
*/
int getISOName
(
  const char *pszLanguage,
  char *pszISO
);
int getISOName
(
  std::string& strLanguage,
  std::string& strISO
);

/* \brief Get the OpenTM2 language name for an ISO language identifier
   \param pszISO language ISO identifier
   \param pszLanguage buffer for OpenTM2 language name
   \returns 0 when successful or an error code
*/
int getOpenTM2NameFromISO
(
  const char *pszISO,
  char *pszLanguage
);
int getOpenTM2NameFromISO
(
  std::string& strISO,
  std::string& strLanguage
);

/* \brief Get the ICU language identifier for a OpenTM2 language name
   \param pszLanguage OpenTM2 language name
   \param pszICU buffer for the language ICU identifier
   \returns 0 when successful or an error code
*/
int getICUName
(
  const char *pszLanguage,
  char *pszICU
);
int getICUName
(
  std::string& strLanguage,
  std::string& strICU
);

/* \brief Get the OpenTM2 language name for a ICU language identifier
   \param pszICU ICU language identifier
   \param pszLanguage buffer for OpenTM2 language name
   \returns 0 when successful or an error code
*/
int getOpenTM2NameFromICU
(
  const char *pszICU,
  char *pszLanguage
);
int getOpenTM2NameFromICU
(
  std::string& strICU,
  std::string& strLanguage
);

/* \brief Get the OEM (ASCII) codepage for a OpenTM2 language name
   \param pszLanguage OpenTM2 language name
   \returns OEM codepage for the given language or 0 in case of errors
*/
unsigned long getOEMCP
(
  const char *pszLanguage
);

/* \brief Get the Windows (ANSI) codepage for a OpenTM2 language name
   \param pszLanguage OpenTM2 language name
   \returns Windows codepage for the given language or 0 in case of errors
*/
unsigned long getAnsiCP
(
  const char *pszLanguage
);


/* \brief Checks if the given language is a valid target language
   \param pszLanguage OpenTM2 language name
   \returns true when the specified language is a valid source language
*/
BOOL isSourceLanguage
(
  const char *pszLanguage
);

/* \brief Checks if the given language is a valid target language
   \param pszLanguage OpenTM2 language name
   \returns true when the specified language is a valid target language
*/
BOOL isTargetLanguage
(
  const char *pszLanguage
);

/* \brief Checks if the given language name is contained in our language list
   \param pszLanguage OpenTM2 language name
   \param fAdjustLangName when TRUE the language name in the buffer is replaced by the correct language name
   \returns TRUE when the specified language is valid 
*/
BOOL isValidLanguage
(
  char *pszLanguage,
  BOOL fAdjustLangName
);

/* \brief Get all information available for the given language
   \param pszLanguage OpenTM2 language name
   \param pInfo a pointer to a LANGUAGEINFO buffer
   \returns TRUE when the info for the specified language could be retrieved
*/
BOOL getLanguageInfo
(
  const char *pszLanguage,
  PLANGUAGEINFO pInfo
);

private:

  /* \brief Find the entry for the specified OpenTM2 language name 
   \param pszLanguage OpenTM2 language name
   \returns -1 when not found or index into language table
*/
int findLanguage
(
  const char *pszLanguage
);

/* \brief Find the entry for the specified ISO language ID 
   \param pszISO ISO language ID 
   \returns -1 when not found or index into language table
*/
int findISO
(
  const char *pszLanguage
);
/* \brief Find the entry for the specified ICU language ID
   \param pszICU ICU language ID 
   \returns -1 when not found or index into language table
*/

int findICU
(
  const char *pszICU
);

/* \brief Compare two ISO language names, ignoring an country specific suffix when required
   \param pszISO1 first ISO language name
   \param pszISO2 second ISO language name
   \returns -1 when not found or index into language table
*/
int compareISO
(
  const char *pszISO1, const char *pszISO2
);


  /*! \brief Pointer to the instance of the MemoryFactory object (singleton).
*/
	static LanguageFactory* pInstance;

  /*! \brief Pointer to the loaded language information list
*/
  void *pvoidLanguageList;

  unsigned short loadLanguageList
  ( 
    const char *pszInFile
  );

  /*! \brief Pointer to active log writer or NULL if none is active */
  void *pvLogWriter;
};


#endif