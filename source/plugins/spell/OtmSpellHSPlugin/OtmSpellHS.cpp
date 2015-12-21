/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#include "OtmSpellHS.h"

/*! \brief constructor  */
OtmSpellHS::OtmSpellHS(void)
{
}

/*! \brief constructor  */
OtmSpellHS::OtmSpellHS(const char* pszLanguage)
{
	strLanguage = pszLanguage;
	strDicDirectory = HunspellObjManager::getInstance()->getDirectory();
	bAvailable = init();
  if ( bAvailable )
  {
    readTermFromFile(MORPH_ABBREV_DIC);
	  readTermFromFile(MORPH_ADDENDUM_DIC);
  }
}

/*! \brief destructor */
OtmSpellHS::~OtmSpellHS(void)
{
	writeTermToFile(MORPH_ABBREV_DIC);
	writeTermToFile(MORPH_ADDENDUM_DIC);
}

/*! \brief initiate the Hunspell object and lock object. 
	\returns true means success, other value means error.
 */
bool OtmSpellHS::init()
{
	strCustomDicName = getCustomDicName(string(strDicDirectory + strLanguage).c_str());
	if (strCustomDicName.size() > 0)
	{
		strAbbrevDicName = strDicDirectory + strLanguage;
		strAbbrevDicName += "_abbrev.dic";
	}
	else
		return false;
	
	PHUNSPELLINFO tInfo = HunspellObjManager::getInstance()->getObjectByLanguage(strLanguage.c_str());
	if (NULL == tInfo)
	{
		return false;
	}
	else
	{
		pHunspell = tInfo->pHunspell;
		pLock = &tInfo->objLock;
    this->strEncoding = pHunspell->get_dic_encoding();
    this->ulCP = getConversionCP();
	}
	return true;
}

/*! \brief check if the spelling of the word is right.
	\param vWord pointer to the word that is to be checked
	\returns zero means error , other value means OK.
 */
int OtmSpellHS::spell(
		  const wchar_t* pszWord
		  )
{
	if (NULL != pszWord && true == bAvailable)
	{
		Autolock<ThreadLock> tLock(*pLock);

    UTF16ToDict( pszWord, this->szBuffer, sizeof(this->szBuffer) );

		if (strAbbrevTermList.end() != strAbbrevTermList.find(this->szBuffer))
		{
			return 1;
		}
		if (setTermList.end() != setTermList.find(this->szBuffer) || setTempTermList.end() != setTempTermList.find(this->szBuffer))
		{
			return 1;
		}
		else
			return pHunspell->spell(this->szBuffer);
	} 
	else
	{
		return 0;
	}
}

/*! \brief get a string list suggested for the word
    \param vWord pointer to the word
    \param vResult reference to a vector of string which is used to restore the suggested strings.
    \returns ERROR_NULL_OBJECT means error, positive number means the word counts.
 */
int OtmSpellHS::suggest(
			const wchar_t* pszWord, 
			vector<std::wstring>& vResult
			)
{
	if (NULL != pszWord && true == bAvailable)
	{
		Autolock<ThreadLock> tLock(*pLock);
		vResult.clear();
		char** wordList;

    UTF16ToDict( pszWord, this->szBuffer, sizeof(this->szBuffer) );

		int tWordCount = pHunspell->suggest(&wordList, this->szBuffer );
		for (int i = 0; i < tWordCount; i++)
		{
      DictToUTF16( wordList[i], this->szBufferW, sizeof(this->szBufferW)/sizeof(wchar_t) );
			vResult.push_back(this->szBufferW);
		}
		pHunspell->free_list(&wordList, tWordCount);
		return tWordCount;
	} 
	else
	{
		return ERROR_NULL_OBJECT;
	}
}

/*! \brief Get stems of the specified term
	\param vTerm the term to be checked
	\param vResult the reference to a list that receives the stems.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error.
 */
int OtmSpellHS::stem( const wchar_t * pszTerm, vector<std::wstring>& vResult )
{
	if (NULL != pszTerm && true == bAvailable)
	{
		Autolock<ThreadLock> tLock(*pLock);
		vResult.clear();
		char** wordList;

    UTF16ToDict( pszTerm, this->szBuffer, sizeof(this->szBuffer) );

		int tWordCount = pHunspell->stem(&wordList, this->szBuffer );
		for (int i = 0; i < tWordCount; i++)
		{
      DictToUTF16( wordList[i], this->szBufferW, sizeof(this->szBufferW)/sizeof(wchar_t) );
			vResult.push_back(this->szBufferW);
		}
		pHunspell->free_list(&wordList, tWordCount);
		return OtmSpell::SUCCESS_RETURN;
	} 
	else
	{
		return ERROR_NULL_OBJECT;
	}
}


/*! \brief add a term to the addendum dictionary
    \param vWord pointer to the term
    \param vDicType the dictionary's type
    \returns 0 means success
 */
int OtmSpellHS::addTerm(const wchar_t* pszWord, int iDicType)
{
	if (NULL != pszWord && NULL != pHunspell && MORPH_ADDENDUM_DIC == iDicType)
	{
    UTF16ToDict( pszWord, this->szBuffer, sizeof(this->szBuffer) );
		setTermList.insert(this->szBuffer);
		return OtmSpell::SUCCESS_RETURN;
	} 
	else if (MORPH_ABBREV_DIC == iDicType)
	{
    UTF16ToDict( pszWord, this->szBuffer, sizeof(this->szBuffer) );
		strAbbrevTermList.insert(this->szBuffer);
		return OtmSpell::SUCCESS_RETURN;
	}
	else
	{
		return ERROR_NULL_OBJECT;
	}
}

/*! \brief list all the terms in the addendum dictionary
	\param vResult reference to the vector of strings
	\param vDicType the dictionary's type
 */
void OtmSpellHS::listTerms( vector<std::wstring>& vResult, int iDicType)
{
	set<string>* tSet;
  vResult.clear();

	if (MORPH_ADDENDUM_DIC == iDicType)
	{
		tSet = &setTermList;
	} 
	else
	{
		tSet = &strAbbrevTermList;
	}

	for (set<string>::iterator tIter = tSet->begin(); tIter != tSet->end(); tIter++)
	{
    DictToUTF16( tIter->c_str(), this->szBufferW, sizeof(this->szBufferW)/sizeof(wchar_t) );
		vResult.push_back(this->szBufferW);
	}
}

/*! \brief list all the terms in the addendum dictionary
	\param vResult reference to the vector of strings
	\param vDicType the dictionary's type
 */
void OtmSpellHS::listTerms( STRINGLIST& vResult, int iDicType)
{
	set<string>* tSet;
	if (MORPH_ADDENDUM_DIC == iDicType)
	{
		tSet = &setTermList;
	} 
	else
	{
		tSet = &strAbbrevTermList;
	}

	for (set<string>::iterator tIter = tSet->begin(); tIter != tSet->end(); tIter++)
	{
		vResult.push_back(*tIter);
	}
}


/*! \brief delete the term if it exists in the addendum dictionary.
	\param vTerm pointer to the term
	\param vDicType the dictionary's type
 */
void OtmSpellHS::deleteTerm( const wchar_t* pszTerm, int iDicType)
{
	if (NULL != pszTerm && MORPH_ADDENDUM_DIC == iDicType)
	{
    UTF16ToDict( pszTerm, this->szBuffer, sizeof(this->szBuffer) );
		setTermList.erase(this->szBuffer);
	}
	else if(MORPH_ABBREV_DIC == iDicType)
	{
    UTF16ToDict( pszTerm, this->szBuffer, sizeof(this->szBuffer) );
		strAbbrevTermList.erase(this->szBuffer);
	}
}

/*! \brief add a temporary term of the language , it will be deleted when the system exits.
	\param vTerm pointer to the term
	\returns 0 means success
 */
int OtmSpellHS::addTempTerm( const wchar_t* pszTerm )
{
	if (NULL == pszTerm)
	{
		return OtmSpell::ERROR_PARAMETAR;
	}
  UTF16ToDict( pszTerm, this->szBuffer, sizeof(this->szBuffer) );
	setTempTermList.insert(this->szBuffer);
	return OtmSpell::SUCCESS_RETURN;
}

/*! \brief get the name of the addendum dictionary
	\param vLanguage pointer to the language name
	\returns the name of the addendum dictionary
 */
std::string OtmSpellHS::getCustomDicName( const char* pszLanguage )
{
	if (NULL == pszLanguage)
	{
		return "";
	}

	string tStr = pszLanguage;
	return tStr + "_custom.dic";
}

/*! \brief write all the terms in the addendum dictionary into a file
	\param vDicType the dictionary's type
	\returns true means success
 */
bool OtmSpellHS::writeTermToFile(int iDicType)
{
	string strDicName = strCustomDicName;
	set<string>* tTermList = &setTermList;
	if (MORPH_ABBREV_DIC == iDicType)
	{
		strDicName = strAbbrevDicName;
		tTermList = &strAbbrevTermList;
	}

	FILE* tFilePointer;
	if (0 == fopen_s(&tFilePointer, strDicName.c_str(), "wt"))
	{
		for (set<string>::iterator tIter = tTermList->begin(); tIter != tTermList->end(); tIter++)
		{
			fputs(tIter->c_str(), tFilePointer);
			fputc('\n', tFilePointer);
		}
		fclose(tFilePointer);
	}
	else
	{
		return false;
	}

	return true;
}

/*! \brief read all the terms in the addendum dictionary from a file
	\param vDicType the dictionary's type
	\returns true means success.
 */
bool OtmSpellHS::readTermFromFile(int iDicType)
{
	string strDicName = strCustomDicName;
	set<string>* tTermList = &setTermList;
	if (MORPH_ABBREV_DIC == iDicType)
	{
		strDicName = strAbbrevDicName;
		tTermList = &strAbbrevTermList;
	}

  tTermList->clear();

	FILE* fpFilePointer;
	if (0 == fopen_s(&fpFilePointer, strDicName.c_str(), "r"))
	{
		char tWord[1024];
		while (!feof(fpFilePointer) && !ferror(fpFilePointer) && 0 != fgets(tWord, sizeof(tWord), fpFilePointer))
		{
			size_t len = strlen(tWord);
			if (len >= 2)
			{
				if (tWord[len - 1] == '\n')
				{
					tWord[len - 1] = 0;	
				}
				tTermList->insert(tWord);
			}
		}
		fclose(fpFilePointer);
	}

	return true;
}

/*! \brief get the codepage for the dictionary encoding
 */
unsigned long OtmSpellHS::getConversionCP
(
)
{
  if ( stricmp( this->strEncoding.c_str(), "KOI8.R" ) == 0 )           return( 20866 );
  if ( stricmp( this->strEncoding.c_str(), "KOI8-R" ) == 0 )           return( 20866 );
  if ( stricmp( this->strEncoding.c_str(), "ISO8859-1" ) == 0 )        return( 28591 );
  if ( stricmp( this->strEncoding.c_str(), "ISO8859-2" ) == 0 )        return( 28592 );
  if ( stricmp( this->strEncoding.c_str(), "ISO8859-3" ) == 0 )        return( 28593 );
  if ( stricmp( this->strEncoding.c_str(), "ISO8859-4" ) == 0 )        return( 28594 );
  if ( stricmp( this->strEncoding.c_str(), "ISO8859-5" ) == 0 )        return( 28595 );
  if ( stricmp( this->strEncoding.c_str(), "ISO8859-6" ) == 0 )        return( 28596 );
  if ( stricmp( this->strEncoding.c_str(), "ISO8859-7" ) == 0 )        return( 28597 );
  if ( stricmp( this->strEncoding.c_str(), "ISO8859-8" ) == 0 )        return( 28598 );
  if ( stricmp( this->strEncoding.c_str(), "ISO8859-9" ) == 0 )        return( 28599 );
  if ( stricmp( this->strEncoding.c_str(), "ISO8859-11" ) == 0 )       return( 874 );
  if ( stricmp( this->strEncoding.c_str(), "ISO8859-13" ) == 0 )       return( 28603 );
  if ( stricmp( this->strEncoding.c_str(), "ISO8859-15" ) == 0 )       return( 28605 );
  if ( stricmp( this->strEncoding.c_str(), "UTF-8" ) == 0 )            return( CP_UTF8 );
  if ( stricmp( this->strEncoding.c_str(), "microsoft-cp1251" ) == 0 ) return( 1251 );
  if ( stricmp( this->strEncoding.c_str(), "TIS620-2533" ) == 0 )      return( 874 );

  // no information for this encoding..
  // TODO: write log record

  // use UTF-8 as default
  return( CP_UTF8 );
}

/*! \brief convert string from UTF-16 to dictionary encoding
	\param pszInString pointer to string in UTF-16 encoding
	\param pszOutString pointer buffer for string in dictionary encoding
	\param iSize size of output buffer in number of characters
	  \returns number of characters in output string
 */
int OtmSpellHS::UTF16ToDict
(
  const wchar_t* pszInString,
  char *pszOutString,
  int iSize
)
{
  return( WideCharToMultiByte( this->ulCP, 0, pszInString, -1, pszOutString, iSize, NULL, NULL ) );
}

/*! \brief convert string from dictionary encoding to UTF-16 
	\param vInString pointer to string in dictionary encoding encoding
	\param vOutString pointer buffer for string in UTF-16 encoding
	\param vSize size of output buffer in number of wide characters
	  \returns number of characters in output string
 */
int OtmSpellHS::DictToUTF16
(
  const char* pszInString,
  wchar_t *pszOutString,
  int iSize
)
{
  return( MultiByteToWideChar( this->ulCP, 0, pszInString, -1, pszOutString, iSize ) );
}
