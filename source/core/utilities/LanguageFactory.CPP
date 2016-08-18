/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqf.h"                  // General .H for EQF


// the Win32 Xerces build requires the default structure packing...
#pragma pack( push )
#pragma pack(8)

#include <iostream>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#pragma pack( pop )

XERCES_CPP_NAMESPACE_USE


#include "core\utilities\LogWriter.h"
#include "LanguageFactory.h"


// currently hard-coded language information table
LanguageFactory::LANGUAGEINFO DefaultLangTable[] = {
  // Name                ASCII  ANSI   ISO CP          ISO          BCP47  pref.  additional info
  { "Afrikaans",          850,  1252,  "ISO-8859-1",  "af-ZA",        "?", FALSE, "-ZA = South Africa ", TRUE, TRUE },
  { "Arabic",             864,  1256,  "ISO-8859-6",  "ar-EG",        "?", FALSE, "-EG = Egypt ", TRUE, TRUE },
  { "Belarusian",         866,  1251,  "ISO-8859-5",  "be-BY",        "?", FALSE, "", TRUE, TRUE },
  { "Bulgarian",          915,  1251,  "ISO-8859-5",  "bg-BG",        "?", FALSE, "", TRUE, TRUE },
  { "Catalan",              0,  1252,  "ISO-8859-1",  "ca-ES",        "?", FALSE, "", TRUE, TRUE },
  { "Chinese(simpl.)",    936,   936,  "gb2312",      "zh-CHS",       "?", FALSE, "", TRUE, TRUE },
  { "Chinese(trad.)",     950,   950,  "big5",        "zh-CHT",       "?", FALSE, "", TRUE, TRUE },
  { "Croatian",           852,  1250,  "ISO-8859-2",  "hr-HR",        "?", FALSE, "", TRUE, TRUE },
  { "Czech",              852,  1250,  "ISO-8859-2",  "cs-CZ",        "?", FALSE, "", TRUE, TRUE },
  { "Danish",             850,  1252,  "ISO-8859-1",  "da-DK",        "?", FALSE, "", TRUE, TRUE },
  { "Dutch(permissive)",  850,  1252,  "ISO-8859-1",  "nl-NL",        "?", FALSE, "", TRUE, TRUE },
  { "Dutch(restrictive)", 850,  1252,  "ISO-8859-1",  "nl-NL",        "?", TRUE,  "", TRUE, TRUE },
  { "English(U.K.)",      437,  1252,  "ISO-8859-1",  "en-GB",        "?", FALSE, "", TRUE, TRUE },
  { "English(U.S.)",      437,  1252,  "ISO-8859-1",  "en-US",        "?", FALSE, "", TRUE, TRUE },
  { "Estonian",           775,  1257,  "ISO-8859-4",  "et-EE",        "?", FALSE, "", TRUE, TRUE },
  { "Finnish",            850,  1252,  "ISO-8859-1",  "fi-FI",        "?", FALSE, "", TRUE, TRUE },
  { "French(Canadian)",   850,  1252,  "ISO-8859-1",  "fr-CA",        "?", FALSE, "", TRUE, TRUE },
  { "French(national)",   850,  1252,  "ISO-8859-1",  "fr-FR",        "?", FALSE, "", TRUE, TRUE },
  { "German(DPAnat)",     850,  1252,  "ISO-8859-1",  "de-DE",        "?", FALSE, "", TRUE, TRUE },
  { "German(reform)",     850,  1252,  "ISO-8859-1",  "de-DE",        "?", TRUE,  "", TRUE, TRUE },
  { "German(Swiss)",      850,  1252,  "ISO-8859-1",  "de-CH",        "?", FALSE, "", TRUE, TRUE },
  { "Greek",              869,  1253,  "ISO-8859-7",  "el-GR",        "?", FALSE, "", TRUE, TRUE },
  { "Hebrew",             867,  1255,  "ISO-8859-8",  "he-IL",        "?", FALSE, "", TRUE, TRUE },
  { "Hungarian",          852,  1250,  "ISO-8859-2",  "hu-HU",        "?", FALSE, "", TRUE, TRUE },
  { "Icelandic",          850,  1252,  "ISO-8859-1",  "is-IS",        "?", FALSE, "", TRUE, TRUE },
  { "Indonesian",         850,  1252,  "ISO-8859-1",  "id-ID",        "?", FALSE, "", TRUE, TRUE },
  { "Italian",            850,  1252,  "ISO-8859-1",  "it-IT",        "?", FALSE, "", TRUE, TRUE },
  { "Japanese",           932,  932,  "Shift_JIS",    "ja-JP",        "?", FALSE, "", TRUE, TRUE },
  { "Kazakh",            1251,  1251,  "ISO-8859-5",  "kk-KZ",        "?", FALSE, "", TRUE, TRUE },
  { "Korean",             949,  949,  "euc-kr",       "ko-KR",        "?", FALSE, "", TRUE, TRUE },
  { "Latvian",            775,  1257,  "ISO-8859-4",  "lv-LV",        "?", FALSE, "", TRUE, TRUE },
  { "Lithuanian",         775,  1257,  "ISO-8859-4",  "lt-LT",        "?", FALSE, "", TRUE, TRUE },
  { "Macedonian",         855,  1251,  "ISO-8859-5",  "mk-MK",        "?", FALSE, "", TRUE, TRUE },
  { "Malay",             1252,  1252,  "ISO-8859-1",  "ms-MY",        "?", FALSE, "", TRUE, TRUE },
  { "Mongolian",         1251,  1251,  "ISO-8859-5",  "mn-MN",        "?", FALSE, "", TRUE, TRUE },
  { "Norwegian(Bokmål)", 850,  1252,  "ISO-8859-1",   "nb-NO",        "?", TRUE,  "", TRUE, TRUE },
  { "Norwegian(Nynorsk)",850,  1252,  "ISO-8859-1",   "nn-NO",        "?", FALSE, "", TRUE, TRUE },
  { "Polish",            852,  1250,  "ISO-8859-2",   "pl-PL",        "?", FALSE, "", TRUE, TRUE },
  { "Portuguese(Br.New)",850,  1252,  "ISO-8859-1",   "pt-BR",        "?", TRUE,  "", TRUE, TRUE },
  { "Portuguese(Brazil)",850,  1252,  "ISO-8859-1",   "pt-BR",        "?", FALSE, "", TRUE, TRUE },
  { "Portuguese(nat.)",  850,  1252,  "ISO-8859-1",   "pt-PT",        "?", FALSE, "", TRUE, TRUE },
  { "Portuguese(nt.New)",850,  1252,  "ISO-8859-1",   "pt-PT",        "?", TRUE,  "", TRUE, TRUE },
  { "Romanian",          852,  1250,  "ISO-8859-2",   "ro-RO",        "?", FALSE, "", TRUE, TRUE },
  { "Russian",           866,  1251,  "ISO-8859-5",   "ru-RU",        "?", FALSE, "", TRUE, TRUE },
  { "Serbian (Cyrillic)",855,  1251,  "ISO-8859-5",   "Cy-sr-SP",     "?", FALSE, "", TRUE, TRUE },
  { "Serbian (Latin)",   852,  1250,  "ISO-8859-5",   "Lt-sr-SP",     "?", FALSE, "", TRUE, TRUE },
  { "Slovakian",         852,  1250,  "ISO-8859-2",   "sk-SK",        "?", FALSE, "", TRUE, TRUE },
  { "Slovene",           852,  1250,  "ISO-8859-2",   "sl-SI",        "?", FALSE, "", TRUE, TRUE },
  { "Spanish",           850,  1252,  "ISO-8859-1",   "es-ES",        "?", FALSE, "", TRUE, TRUE },
  { "Swedish",           850,  1252,  "ISO-8859-1",   "sv-SE",        "?", FALSE, "", TRUE, TRUE },
  { "Thai",              874,   874,  "ISO-8859-11",  "th-TH",        "?", FALSE, "ANSI-CP = TIS-620 ??? ", TRUE, TRUE },
  { "Turkish",           857,  1254,  "ISO-8859-9",   "tr-TR",        "?", FALSE, "", TRUE, TRUE },
  { "Ukrainian",        1252,  1252,  "ISO-8859-5",   "uk-UA",        "?", FALSE, "", TRUE, TRUE },
  { "",                    0,     0,  "",             "",              "", FALSE, "", FALSE, FALSE } };

//
// class for our SAX handler which parses the XML file containing the language settings
//
class LangParseHandler : public HandlerBase
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  LangParseHandler();
  virtual ~LangParseHandler();

  // setter functions 
  void SetLangList( std::vector<LanguageFactory::LANGUAGEINFO> *pvLangTableIn );

  // getter functions 
  bool ErrorOccured( void );
  void GetErrorText( char *pszTextBuffer, int iBufSize );

  //  handlers for the SAX DocumentHandler interface
  void startElement(const XMLCh* const name, AttributeList& attributes);
  void endElement(const XMLCh* const name );
  void characters(const XMLCh* const chars, const XMLSize_t length);

  //  handlers for the SAX ErrorHandler interface
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc );
  void fatalError(const SAXParseException& exc);

private:
  // IDs of XML elelements
  typedef enum { LANGUAGES_ELEMENT, LANGUAGE_ELEMENT, NAME_ELEMENT, CPASCII_ELEMENT, CPANSI_ELEMENT, CPNAME_ELEMENT,
                 ISO_ELEMENT, ICU_ELEMENT, PREFERRED_ELEMENT, GROUP_ELEMENT, ISSOURCELANGUAGE_ELEMENT, ISTARGETLANGUAGE_ELEMENT,
                 COMMENTS_ELEMENT, UNKNOWN_ELEMENT
               } ELEMENTID;

  typedef struct _XMLNAMETOID
  {
    CHAR_W   szName[30];                 // name of element
    LangParseHandler::ELEMENTID ID;                        // ID of element 
  } XMLNAMETOID, *PXMLNAMETOID;

  LanguageFactory::LANGUAGEINFO CurElement;
  ELEMENTID CurID;
  std::vector<ELEMENTID> vELementStack;
  std::vector<LanguageFactory::LANGUAGEINFO> *pvLangTable;
  CHAR_W szDataW[512];
  CHAR szData[512];
  char szErrorText[256];

  bool fError;                         // TRUE = parsing ended with error

  ELEMENTID GetElementID( PSZ_W pszName );
};



/*! \brief the language factory instance	 */
LanguageFactory* LanguageFactory::pInstance = NULL;

/*! \brief constructor	 */
LanguageFactory::LanguageFactory(void)
{
  std::vector<LANGUAGEINFO> *pvLanguageList  = new std::vector<LANGUAGEINFO>;
  this->pvoidLanguageList = (void *)pvLanguageList;
  char *pszLogFile = "LanguageFactory";

  LogWriter::registerLogFile( pszLogFile, "Logging of language name and language properties related information" );

  LogWriter *pLogWriter = new LogWriter();
  pvLogWriter = (PVOID)pLogWriter;
  //pLogWriter->open( "LanguageFactory" );
  pLogWriter->write( "Starting LanguageFactory" );

  // try to load the external language list
  char szFile[256];
  UtlMakeEQFPath( szFile, NULC, TABLE_PATH, NULL );
  strcat( szFile, "\\languages.xml" );
  int iRC = loadLanguageList( szFile );
  if ( iRC != 0 )
  {
    pLogWriter->writef( "Load of external language list file \"%s\"failed with error code %ld, switching to built-in language list", szFile, iRC );
    // load failed, used built-in list instead
    PLANGUAGEINFO pInfo = DefaultLangTable;
    pvLanguageList->clear();
    while( pInfo->szName[0] != '\0' )
    {
      pvLanguageList->push_back( *pInfo );
      pInfo++;
    }
    pLogWriter->writef( "Built-in language list contains %ld entries", pvLanguageList->size() );
  }
  else
  {
    pLogWriter->writef( "Load of external language list file \"%s\" successfull, the list contains %ld entries", szFile, pvLanguageList->size()  );
  }
}

/*! \brief destructor	 */
LanguageFactory::~LanguageFactory(void)
{
  if ( pvLogWriter != NULL )
  {
    LogWriter *pLogWriter = (LogWriter *)pvLogWriter;
    pLogWriter->write( "Stopping LanguageFactory" );
    pLogWriter->close();
    delete pLogWriter;
  }
}


/*! \brief This static method returns a pointer to the LanguageFactory object.
	The first call of the method creates the LanguageFactory instance.
*/
LanguageFactory* LanguageFactory::getInstance()
{
	if ( pInstance == NULL )
	{
		pInstance = new LanguageFactory();
	}
	return pInstance;
}

/* \brief Checks if the given language is a valid target language
   \param pszLanguage OpenTM2 language name
   \returns true when the specified language is a valid source language
*/
BOOL LanguageFactory::isSourceLanguage
(
  const char *pszLanguage
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findLanguage( pszLanguage );
  if ( i < 0 )
  {
    return( FALSE );
  }

  return( (*pvLanguageList)[i].isSourceLanguage );
}

/* \brief Checks if the given language is a valid target language
   \param pszLanguage OpenTM2 language name
   \returns true when the specified language is a valid target language
*/
BOOL LanguageFactory::isTargetLanguage
(
  const char *pszLanguage
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findLanguage( pszLanguage );
  if ( i < 0 )
  {
    return( FALSE );
  }

  return( (*pvLanguageList)[i].isTargetLanguage );
}

/*! \brief Provide a list of all available languages
\param eType type of languages to be listed
\param pfnCallBack callback function to be called for each language
\param pvData caller's data pointer, is passed to callback function
\param fWithDetails TRUE = supply language details, when this flag is set, 
the pInfo parameter of the callback function is set otherwise it is NULL
\returns number of provided languages
*/
int LanguageFactory::listLanguages(
  LANGUAGETYPE eType,
	PFN_LISTLANGUAGES_CALLBACK pfnCallBack,			  
	void *pvData,
	BOOL fWithDetails
)
{
  int iLanguages = 0;
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;
  int i = 0;
  int iMax = pvLanguageList->size();
  while( i < iMax )
  {
    BOOL fAddLanguage = FALSE;
    switch( eType )
    {
      case SOURCE_LANGUAGE_TYPE:
        fAddLanguage = (*pvLanguageList)[i].isSourceLanguage;
        break;

      case TARGET_LANGUAGE_TYPE:
        fAddLanguage = (*pvLanguageList)[i].isTargetLanguage;
        break;

      case ALL_LANGUAGES_TYPE:
      default:
        fAddLanguage = TRUE;
        break;
    }

    if ( fAddLanguage )
    {
      pfnCallBack( pvData, (*pvLanguageList)[i].szName, fWithDetails ? &((*pvLanguageList)[i]) : NULL );
      iLanguages++;
    }
    i++;
  }

  return( iLanguages );
}

/* \brief Get the ISO language identifier for a OpenTM2 language name
   \param pszLanguage OpenTM2 language name
   \param pszISO buffer for the language ISO identifier
   \returns 0 when successful or an error code
*/
int LanguageFactory::getISOName
(
  const char *pszLanguage,
  char *pszISO
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findLanguage( pszLanguage );
  if ( i < 0 )
  {
    strcpy( pszISO, "??" );
    return( ERROR_LANGUAGENOTFOUND );
  }

  strcpy( pszISO, (*pvLanguageList)[i].szIsoID );

  return( 0 );
}

int LanguageFactory::getISOName
(
  std::string& strLanguage,
  std::string& strISO
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findLanguage( strLanguage.c_str() );
  if ( i < 0 )
  {
    strISO = "??";
    return( ERROR_LANGUAGENOTFOUND );
  }

  strISO = (*pvLanguageList)[i].szIsoID;

  return( 0 );
}

/* \brief Get the OpenTM2 language name for an ISO language identifier
   \param pszISO language ISO identifier
   \param pszLanguage buffer for OpenTM2 language name
   \returns 0 when successful or an error code
*/
int LanguageFactory::getOpenTM2NameFromISO
(
  const char *pszISO,
  char *pszLanguage
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findISO( pszISO );
  if ( i < 0 )
  {
    return( ERROR_LANGUAGENOTFOUND );
  }

  strcpy( pszLanguage, (*pvLanguageList)[i].szName );

  return( 0 );
}

int LanguageFactory::getOpenTM2NameFromISO
(
  std::string& strISO,
  std::string& strLanguage
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findISO( strISO.c_str() );
  if ( i < 0 )
  {
    return( ERROR_LANGUAGENOTFOUND );
  }

  strLanguage = (*pvLanguageList)[i].szName;

  return( 0 );
}

/* \brief Get the ICU language identifier for a OpenTM2 language name
   \param pszLanguage OpenTM2 language name
   \param pszICU buffer for the language ICU identifier
   \returns 0 when successful or an error code
*/
int LanguageFactory::getICUName
(
  const char *pszLanguage,
  char *pszICU
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findLanguage( pszLanguage );
  if ( i < 0 )
  {
    return( ERROR_LANGUAGENOTFOUND );
  }

  strcpy( pszICU, (*pvLanguageList)[i].szIcuID );

  return( 0 );
}

int LanguageFactory::getICUName
(
  std::string& strLanguage,
  std::string& strICU
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findLanguage( strLanguage.c_str() );
  if ( i < 0 )
  {
    return( ERROR_LANGUAGENOTFOUND );
  }

  strICU = (*pvLanguageList)[i].szIcuID;

  return( 0 );
}


/* \brief Get the OpenTM2 language name for a ICU language identifier
   \param pszICU ICU language identifier
   \param pszLanguage buffer for OpenTM2 language name
   \returns 0 when successful or an error code
*/
int LanguageFactory::getOpenTM2NameFromICU
(
  const char *pszICU,
  char *pszLanguage
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findICU( pszICU );
  if ( i < 0 )
  {
    return( ERROR_LANGUAGENOTFOUND );
  }

  strcpy( pszLanguage, (*pvLanguageList)[i].szName );

  return( 0 );
}

int LanguageFactory::getOpenTM2NameFromICU
(
  std::string& strICU,
  std::string& strLanguage
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findICU( strICU.c_str() );
  if ( i < 0 )
  {
    return( ERROR_LANGUAGENOTFOUND );
  }

  strLanguage = (*pvLanguageList)[i].szName;

  return( 0 );
}

/* \brief Get the OEM (ASCII) codepage for a OpenTM2 language name
   \param pszLanguage OpenTM2 language name
   \returns OEM codepage for the given language or 0 in case of errors
*/
unsigned long LanguageFactory::getOEMCP
(
  const char *pszLanguage
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findLanguage( pszLanguage );
  if ( i < 0 ) return( 0 );

  return( (*pvLanguageList)[i].lAsciiCP );
}

/* \brief Get the Windows (ANSI) codepage for a OpenTM2 language name
   \param pszLanguage OpenTM2 language name
   \returns Windows codepage for the given language or 0 in case of errors
*/
unsigned long LanguageFactory::getAnsiCP
(
  const char *pszLanguage
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findLanguage( pszLanguage );
  if ( i < 0 ) return( 0 );

  return( (*pvLanguageList)[i].lAnsiCP );
}

/* \brief Checks if the given language name is contained in our language list
   \param pszLanguage OpenTM2 language name
   \param fAdjustLangName when TRUE the language name in the buffer is replaced by the correct language name
   \returns TRUE when the specified language is valid 
*/
BOOL LanguageFactory::isValidLanguage
(
  char *pszLanguage,
  BOOL fAdjustLangName
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findLanguage( pszLanguage );
  if ( (i >= 0) && fAdjustLangName )
  {
    strcpy( pszLanguage, (*pvLanguageList)[i].szName );
  } /* endif */

  return( i >= 0 );
}

/* \brief Get all information available for the given language
   \param pszLanguage OpenTM2 language name
   \param pInfo a pointer to a LANGUAGEINFO buffer
   \returns TRUE when the info for the specified language could be retrieved
*/
BOOL LanguageFactory::getLanguageInfo
(
  const char *pszLanguage,
  PLANGUAGEINFO pInfo
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;

  int i = this->findLanguage( pszLanguage );
  if ( i < 0 ) return( FALSE );

  memcpy( pInfo, &((*pvLanguageList)[i]), sizeof(LANGUAGEINFO) );

  return( TRUE );
}


/* \brief Find the entry for the specified OpenTM2 language name 
   \param pszLanguage OpenTM2 language name
   \returns -1 when not found or index into language table
*/
int LanguageFactory::findLanguage
(
  const char *pszLanguage
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;
  int i = 0;
  int iMax = pvLanguageList->size();
  while( i < iMax )
  {
    if ( stricmp( pszLanguage, (*pvLanguageList)[i].szName ) == 0 )
    {
      return( i );
    }
    i++;
  }

  return( -1 );
}

/* \brief Compare two ISO language names, ignoring an country specific suffix when required
   \param pszISO1 first ISO language name
   \param pszISO2 second ISO language name
   \returns -1 when not found or index into language table
*/
int LanguageFactory::compareISO
(
  const char *pszISO1, const char *pszISO2
)
{

   int iResult = stricmp( pszISO1, pszISO2 );
   if ( iResult == 0 ) return( iResult );

   // return when both ISO names have a country ID or both ISO names have none
   PSZ pszDash1 = (PSZ)strchr( pszISO1, '-' );
   PSZ pszDash2 = (PSZ)strchr( pszISO2, '-' );
   if ( ((pszDash1 == NULL) && (pszDash2 == NULL)) || ((pszDash1 != NULL) && (pszDash2 != NULL)) ) return( iResult );

   // compare ISO names without country suffix
   char szLang1[10];
   char szLang2[10];

   Utlstrccpy( szLang1, (PSZ)pszISO1, '-' );
   Utlstrccpy( szLang2, (PSZ)pszISO2, '-' );
 
   return ( stricmp( szLang1, szLang2 ) );
}

/* \brief Find the entry for the specified ISO language ID 
   \param pszLanguage OpenTM2 language name
   \returns -1 when not found or index into language table
*/
int LanguageFactory::findISO
(
  const char *pszISO
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;
  int iFound = -1;
  int i = 0;
  int iMax = pvLanguageList->size();

  while( i < iMax )
  {
    if ( compareISO( pszISO, (*pvLanguageList)[i].szIsoID ) == 0 )
    {
      if ( (*pvLanguageList)[i].fisPreferred )
      {
        return( i );
      }
      else
      {
        iFound = i;
      }
    }
    i++;
  }
  return( iFound );
}

/* \brief Find the entry for the specified ICU language ID 
   \param pszICU ICU language ID 
   \returns -1 when not found or index into language table
*/

int LanguageFactory::findICU
(
  const char *pszICU
)
{
  std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;
  int iFound = -1;
  int i = 0;
  int iMax = pvLanguageList->size();
  while( i < iMax )
  {
    if ( strcmp( pszICU, (*pvLanguageList)[i].szIcuID ) == 0 )
    {
      if ( (*pvLanguageList)[i].fisPreferred )
      {
        return( i );
      }
      else
      {
        iFound = i;
      }
    }
    i++;
  }

  return( iFound );
}


unsigned short LanguageFactory::loadLanguageList( const char *pszLangList )
{
  unsigned short usRC = 0;

  // parse the XML file containing the language properties
  if ( !usRC )
  {
    try {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch) {
        toCatch;

        return( ERROR_NOT_READY );
    }

    SAXParser* parser = new SAXParser();      // Create a SAX parser object

    // create an instance of our handler
    LangParseHandler *handler = new LangParseHandler();


    //  install our SAX handler as the document and error handler.
    parser->setDocumentHandler( handler );
    parser->setErrorHandler( handler );
    parser->setValidationSchemaFullChecking( false );
    parser->setDoSchema( false );
    parser->setLoadExternalDTD( false );
    parser->setValidationScheme( SAXParser::Val_Never );
    parser->setExitOnFirstFatalError( false );
    std::vector<LANGUAGEINFO> *pvLanguageList = (std::vector<LANGUAGEINFO> *)this->pvoidLanguageList;
    handler->SetLangList( pvLanguageList );

    try
    {
      parser->parse( pszLangList );
    }
    catch (const OutOfMemoryException& )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    catch (const XMLException& toCatch)
    {
      toCatch;

      usRC = ERROR_READ_FAULT;
    }

  } /* endif */


  return( usRC );
}


//
// implementation of SAX parser for the parsing of the language control file
//
LangParseHandler::LangParseHandler()
{
  fError = FALSE;
  pvLangTable = NULL;
}

LangParseHandler::~LangParseHandler()
{
}

void LangParseHandler::startElement(const XMLCh* const name, AttributeList& attributes)
{
    PSZ_W pszName = (PSZ_W)name;

    attributes;

    CurID = GetElementID( pszName );
    vELementStack.push_back( CurID );
    szDataW[0] = 0;
    switch ( CurID )
    {
      case LANGUAGE_ELEMENT:
        // reset element info
        memset( &(CurElement), 0, sizeof(CurElement) );
        break;
      case UNKNOWN_ELEMENT:
      default:
        break;
    } /*endswitch */
}

void LangParseHandler::endElement(const XMLCh* const name )
{
  CurID = vELementStack.back();
  vELementStack.pop_back();

  name;

  // convert collected data to ASCII
  WideCharToMultiByte( CP_OEMCP, 0, this->szDataW, -1, this->szData, sizeof(this->szData), NULL, NULL );

  switch ( CurID )
  {
    case LANGUAGE_ELEMENT:
      // add new language element to the language table
      if ( this->CurElement.szName[0] != '\0' )
      {
        this->pvLangTable->push_back( this->CurElement );
      }
      break;
    case NAME_ELEMENT:
      strncpy( this->CurElement.szName, this->szData, sizeof(this->CurElement.szName) );
      break;
    case CPASCII_ELEMENT:
      this->CurElement.lAsciiCP = atol( this->szData);
      break;
    case CPANSI_ELEMENT:
      this->CurElement.lAnsiCP = atol( this->szData);
      break;
    case CPNAME_ELEMENT:
      strncpy( this->CurElement.szIsoCPName, this->szData, sizeof(this->CurElement.szIsoCPName) );
      break;
    case ISO_ELEMENT:
      strncpy( this->CurElement.szIsoID, this->szData, sizeof(this->CurElement.szIsoID) );
      break;
    case ICU_ELEMENT:
      strncpy( this->CurElement.szIcuID, this->szData, sizeof(this->CurElement.szIcuID) );
      break;
    case PREFERRED_ELEMENT:
      this->CurElement.fisPreferred = (stricmp( this->szData, "yes" ) == 0);
      break;
    case GROUP_ELEMENT:
      strncpy( this->CurElement.szLangGroup, this->szData, sizeof(this->CurElement.szLangGroup) );
      break;
    case ISSOURCELANGUAGE_ELEMENT:
      this->CurElement.isSourceLanguage = (stricmp( this->szData, "yes" ) == 0);
      break;
    case ISTARGETLANGUAGE_ELEMENT:
      this->CurElement.isTargetLanguage = (stricmp( this->szData, "yes" ) == 0);
      break;
    case COMMENTS_ELEMENT:
      strncpy( this->CurElement.szAddInfo, this->szData, sizeof(this->CurElement.szAddInfo) );
      break;
    case UNKNOWN_ELEMENT:
    default:
      break;
  } /*endswitch */
}

void LangParseHandler::characters(const XMLCh* const chars, const XMLSize_t length)
{
  PSZ_W pszChars = (PSZ_W)chars;

  // add data to current data buffer 
  int iCurLen = wcslen( szDataW );
  if ( (iCurLen + length + 1) < (sizeof(szData)/sizeof(CHAR_W)))
  {
    wcsncpy( szDataW + iCurLen, pszChars, length );
    szDataW[iCurLen+length] = 0;
  } /* endif */
}

void LangParseHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    this->fError = TRUE;
    sprintf( this->szErrorText, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    XMLString::release( &message );
}

void LangParseHandler::error(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    this->fError = TRUE;
    sprintf( this->szErrorText, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    XMLString::release( &message );
}

void LangParseHandler::warning(const SAXParseException& exception)
{
  exception;
}


// get the ID for a XML element
LangParseHandler::ELEMENTID LangParseHandler::GetElementID( PSZ_W pszName )
{
  int i = 0;
  ELEMENTID IDFound = UNKNOWN_ELEMENT;

static XMLNAMETOID XmlNameToID[] =
{ { L"languages",  LANGUAGES_ELEMENT },
  { L"language",   LANGUAGE_ELEMENT }, 
  { L"name",       NAME_ELEMENT }, 
  { L"cpASCII",    CPASCII_ELEMENT }, 
  { L"cpANSI",     CPANSI_ELEMENT }, 
  { L"cpName",     CPNAME_ELEMENT },
  { L"iso",        ISO_ELEMENT }, 
  { L"icu",        ICU_ELEMENT }, 
  { L"preferred",  PREFERRED_ELEMENT }, 
  { L"group",      GROUP_ELEMENT }, 
  { L"isSourceLanguage", ISSOURCELANGUAGE_ELEMENT }, 
  { L"isTargetLanguage", ISTARGETLANGUAGE_ELEMENT },
  { L"comments",   COMMENTS_ELEMENT }, 
  { L"",           UNKNOWN_ELEMENT } };

  while ( (IDFound == UNKNOWN_ELEMENT) && (XmlNameToID[i].szName[0] != 0) )
  {
    if ( _wcsicmp( pszName, XmlNameToID[i].szName ) == 0 )
    {
      IDFound = XmlNameToID[i].ID;
    }
    else
    {
      i++;
    } /* endif */
  } /*endwhile */
  return( IDFound );
} /* end of method LangParseHandler::GetElementID */

bool LangParseHandler::ErrorOccured( void )
{
  return( this->fError );
}

void LangParseHandler::GetErrorText( char *pszTextBuffer, int iBufSize )
{
  *pszTextBuffer = '\0';

  if ( this->szErrorText[0] != '\0' )
  {
    strncpy( pszTextBuffer, this->szErrorText, iBufSize );
    pszTextBuffer[iBufSize-1] = '\0';
  } /* endif */
}

void LangParseHandler::SetLangList( std::vector<LanguageFactory::LANGUAGEINFO> *pvLangTableIn )
{
  this->pvLangTable = pvLangTableIn;
}


