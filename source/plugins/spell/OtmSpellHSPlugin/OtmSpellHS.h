/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef OtmSpellHS_h__
#define OtmSpellHS_h__

#include <Windows.h>
#include "HunspellObjManager.h"
#include <string>
#include "core\PluginManager\OtmSpell.h"

#ifdef near
#undef near
#endif
#define HUNSPELL_STATIC
#include <hunspell/hunspell.hxx>

/*! \brief class for spell check objects */
class OtmSpellHS :
	public OtmSpell
{
public:

/*! \brief constructor */
	OtmSpellHS
		(
		void
		);

/*! \brief constructor
	\param vLanguage pointer to the language
*/
	OtmSpellHS
		(
		const char* pszLanguage
		);
/*! \brief destructor */
	~OtmSpellHS
		(
		void
		);

/*! \brief initiate the Hunspell object and lock object. 
	\returns true means success, other value means error.
 */
	bool init
		(
		);

/*! \brief check if the spelling of the word is right.
	\param vWord pointer to the word that is to be checked
	\returns zero means error , other value means OK.
 */
	int spell
		(
		const wchar_t* pszWord
		);

/*! \brief get a string list suggested for the word
    \param vWord pointer to the word
    \param vResult reference to a vector of string which is used to restore the suggested strings.
    \returns ERROR_NULL_OBJECT means error, positive number means the word counts.
 */
	int suggest
		(
		const wchar_t* pszWord, 
		vector<std::wstring>& strResult
		);

/*! \brief Get stems of the specified term
	\param vTerm the term to be checked
	\param vResult the reference to a list that receives the stems.
	\returns OtmSpell::SUCCESS_RETURN means success, other value means error.
 */
	int stem
		(
		const wchar_t * pszTerm, 
		vector<std::wstring>& vResult
		);


/*! \brief add a term to the addendum dictionary
    \param vWord pointer to the term
    \param vDicType the dictionary's type
    \returns 0 means success
 */
	int addTerm
		(
		const wchar_t* pszWord,
		int iDicType
		);

/*! \brief list all the terms of the target language
	\param vResult reference to the vector of strings
	\param vDicType the dictionary's type
 */
	void listTerms
		(
		vector<std::wstring>& strResult,
		int iDicType
		);

  /*! \brief list all the terms of the target language
	\param vResult reference to the vector of strings
	\param vDicType the dictionary's type
 */
	void listTerms
		(
		STRINGLIST& strResult,
		int iDicType
		);

/*! \brief delete the term if it exists in the addendum dictionary.
	\param vTerm pointer to the term
	\param vDicType the dictionary's type
 */
	void deleteTerm
		(
		const wchar_t* pszTerm,
		int iDicType
		);

/*! \brief add a temporary term of the language , it will be deleted when the system exits.
	\param vTerm pointer to the term
	\returns 0 means success
 */
	int addTempTerm
		(
		const wchar_t* pszTerm
		);

/*! \brief write all the terms in the addendum dictionary into a file
	\param the dictionary's type
	\returns true means success
 */
	bool writeTermToFile
		(
		int iDicType
		);

/*! \brief read all the terms in the addendum dictionary from a file
	\vDicType the dictionary's type
	\returns true means success.
 */
	bool readTermFromFile
		(
		int iDicType
		);

private:

/*! \brief get the name of the addendum dictionary
	\param vLanguage pointer to the language name
	\returns the name of the addendum dictionary
 */
	string getCustomDicName
		(
		const char* pszLanguage
		);
/*! \brief get the codepage for the dictionary encoding
	  \returns codepage for the dictionary encoding
 */
	unsigned long getConversionCP
		(
		);

/*! \brief convert string from UTF-16 to dictionary encoding
	\param pszInString pointer to string in UTF-16 encoding
	\param pszOutString pointer buffer for string in dictionary encoding
	\param iSize size of output buffer in number of characters
	  \returns number of characters in output string
 */
	int UTF16ToDict
		(
      const wchar_t* pszInString,
      char *pszOutString,
      int iSize
		);

/*! \brief convert string from dictionary encoding to UTF-16 
	\param vInString pointer to string in dictionary encoding encoding
	\param vOutString pointer buffer for string in UTF-16 encoding
	\param vSize size of output buffer in number of wide characters
	  \returns number of characters in output string
 */
	int DictToUTF16
		(
      const char* pszInString,
      wchar_t *pszOutString,
      int iSize
		);

	string strCustomDicName;			//the name of the addendum dictionary
	string strAbbrevDicName;			//the name of the abbreviation dictionary
	string strDicDirectory;			  //the directory of the dictionaries
	set<string> setTempTermList;	//the vector that is used to restore the temporary addendum dictionary
	set<string> setTermList;			//the vector that is used to restore the addendum dictionary
	set<string> strAbbrevTermList;//the vector that is used to restore the abbreviation dictionary
	Hunspell* pHunspell;			    //pointer to the Hunspell instance
	ThreadLock* pLock;				    //pointer to the lock that keeps the Hunspell instance thread-safe
	string strLanguage;				    //the string that restores the language name
	string strEncoding;	  		    //the encoding used by the dictionary
  unsigned long ulCP;           //codepage for the conversion of the dictionary data to/from UTF-16
	bool bAvailable;				      //It's set to true when the functionality is available.
  char    szBuffer[2048];       // buffer for multi-byte (Ansi, ASCII, or UTF-8) strings
  wchar_t szBufferW[2048];      // buffer for widechar (Unicode UTF-16) strings
};

#endif // OtmSpellHS_h__