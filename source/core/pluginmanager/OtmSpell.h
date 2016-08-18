/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef OTMSPELL_H_
#define OTMSPELL_H_

#include <string>
#include <set>
#include <map>
#include <vector>
#include <stdio.h>

using namespace std;

typedef vector<string> STRINGLIST;

/*! \brief Abstract base-class for spell check objects */
class __declspec(dllexport) OtmSpell
{
public:

/*! \brief Constructors */
	OtmSpell() {};

/*! \brief Destructor */
	virtual ~OtmSpell() {};

/*! \brief Error code definition
*/
	static const int ERROR_FILE_NOT_EXIST = -6001;
	static const int ERROR_NULL_OBJECT = -6002;
	static const int ERROR_UNKNOWN = -6003;
	static const int ERROR_PARAMETAR = -6004;
	static const int ERROR_NO_SUPPORT = -6005;
	static const int SUCCESS_RETURN = 0;
	static const int MORPH_ADDENDUM_DIC = 2;
	static const int MORPH_ABBREV_DIC = 1;

/*! \brief Check if the spelling of the word is OK?
    \param vWord the word to be checked.
  	\returns 0 means success, non zero means error.
*/
	virtual int spell(
		const wchar_t* vWord
		) = 0;

/*! \brief Provide the suggestion of the given word.
    \param vWord pointer to the word.
    \param vResult reference to the string list that receives suggestions.
  	\returns ERROR_NULL_OBJECT means failed, positive number means the success.
*/
	virtual int suggest(
		const wchar_t* vWord, 
		vector<std::wstring>& vResult
		) = 0;

/*! \brief Get stems of the specified term
	\param vTerm the term to be checked
	\param vResult the reference to a list that receives the stems.
 */
	virtual int stem(const wchar_t * vTerm, vector<std::wstring> &vResult ) = 0;

/*! \brief add the term to the user permanent dictionary.
    \param vWord pointer to the word.
  	\returns ERROR_NULL_OBJECT means failed, SUCCESS_RETURN means the success.
*/
	virtual int addTerm(
		const wchar_t* vWord,
		int vDicType
		) = 0;

/*! \brief list the terms of the user dictionaries.
    \param vResult reference to the string list that receives terms of the user dictionaries.
*/
	virtual void listTerms(
		vector<std::wstring>& vResult,
		int vDicType
		) = 0;

/*! \brief list all the terms of the target language
	\param vResult reference to the vector of strings
	\param vDicType the dictionary's type
 */
	virtual void listTerms
		(
		STRINGLIST& strResult,
		int iDicType
		) = 0;

/*! \brief delete the term if it exists in the user dictionaries.
    \param vTerm pointer to the word.
*/
	virtual void deleteTerm(
		const wchar_t* vTerm,
		int vDicType
		) = 0;

/*! \brief add the term to the user temporary dictionary.
    \param vTerm pointer to the term.
  	\returns ERROR_PARAMETAR means failed, SUCCESS_RETURN means the success.
*/
	virtual int addTempTerm(
		const wchar_t* vTerm
		) = 0;

/*! \brief write all the terms in the addendum dictionary into a file
	\returns true means success
*/
	virtual bool writeTermToFile(
		int vDicType
		) = 0;

/*! \brief read all the terms in the addendum dictionary from a file
	\returns true means success.
 */
	virtual bool readTermFromFile(
		int vDicType
		) = 0;

};

#endif