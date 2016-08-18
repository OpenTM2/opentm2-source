/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef OtmMorph_h__
#define OtmMorph_h__
#include <windows.h>
#include "string"
#include "vector"
using namespace std;

typedef vector<string> STRINGLIST;

/*! \brief Abstract base-class for morphology objects */
class __declspec(dllexport) OtmMorph
{
public:

/*! \brief Constructors */
	OtmMorph(void) {};

/*! \brief Destructor */
	virtual ~OtmMorph(void) {};

	/*! \brief Error code definition
	*/
	static const int ERROR_FILE_NOT_EXIST = -6001;
	static const int ERROR_NULL_OBJECT = -6002;
	static const int ERROR_UNKNOWN = -6003;
	static const int ERROR_PARAMETAR = -6004;
	static const int ERROR_NO_SUPPORT = -6005;
	static const int SUCCESS_RETURN = 0;

/*! \brief Get stems of the specified term
	\param vTerm the term to be checked
	\param vResult the reference to a list that receives the stems.
 */
	virtual int stem(const wchar_t * vTerm, vector<std::wstring> &vResult ) = 0;

/*! \brief Get part of speech info of the specified term
	\param vTerm the term to be checked
	\param vResult the pointer to a integer that receives the part of speech info.
 */
	virtual int getPOS(const char* vTerm, int* vResult) = 0;

  /*! \brief Flags for the term type in term lists
 */
    static const int TERMTYPE_NOLOOKUP    = 0x00000001L; // term is no lookup term
    static const int TERMTYPE_NUMBER      = 0x00000002L; // term is date/number/time value
    static const int TERMTYPE_ABBR        = 0x00000004L; // term is an abbreviation
    static const int TERMTYPE_ALLCAPS     = 0x00000008L; // term consists of capital letters only
    static const int TERMTYPE_NEWSENTENCE = 0x00000010L; // term is a dummy term for the start of a new sentence
    static const int TERMTYPE_PREFIX      = 0x00000040L; // term is a prefix, e.g. c' (in c'est)
    static const int TERMTYPE_COMPLEX     = 0x00000080L; // term is complex token, e.g. con-taining
    static const int TERMTYPE_INITCAP     = 0x00000100L; // first letter of term is uppercase
    static const int TERMTYPE_NOCOUNT     = 0x00000200L; // do not count this term
    static const int TERMTYPE_WHITESPACE  = 0x00000400L; // term consists of whitespace only
    static const int TERMTYPE_PUNCTUATION = 0x00000800L; // punctuation: comma, semi-colon, period, exclamation mark,...


/*! \brief Term definition for term lists
 */
  struct TERMENTRY
  {
    int iStartOffset;                  // start offset of term
    int iLength;                       // length of term in number of characters
    int iTermType;                     // type of the Term 
  };

  typedef std::vector<TERMENTRY> TERMLIST;

/*! \brief Tokenization of an input segment.
	\param pText points to the text being tokenized
	\param vResult the reference to a vector receiving the recognized terms
 */
	virtual int tokenizeByTerm(const wchar_t* pText, TERMLIST& vResult) = 0;

/*! \brief Tokenization of an input segment.
	\param vSection the section to be tokenized.
	\param vResult the reference to a list that receives the terms.
 */
	virtual int tokenizeByTerm(const char* vSection, STRINGLIST& vResult) = 0;

	/*! \brief Tokenization of an input segment.
	\param vSection the section to be tokenized.
	\param vResult the reference to a list that receives the sentences.
 */
	virtual int tokenizeBySentence(const char* vSection, STRINGLIST& vResult) = 0;

	/*! \brief Tokenization of an input segment.
	\param vSection the section to be tokenized.
	\param vResult the reference to a list that receives the sentences.
 */
	virtual int tokenizeBySentence(const wchar_t* vSection, TERMLIST& vResult) = 0;

/*! \brief Returns all decompositions for a compound word.
	\param vTerm the section to be checked.
	\param vResult the reference to a list that receives the decompositions.
 */
	virtual int compisol(const char* vTerm, STRINGLIST& vResult) = 0;

/*! \brief add the term to the user abbreviation list.
    \param vWord pointer to the word.
  	\returns ERROR_NULL_OBJECT means failed, SUCCESS_RETURN means the success.
*/
	virtual int addAbbreviation(
		const wchar_t* vWord
		) = 0;

/*! \brief list the terms of the user abbreviation list or the system abbreviation list.
    \param vResult reference to the string list that receives the abbreviations
    \param fUserAbbreviationList TRUE = return user abbreviationa, FALSE = return system abbreviations
*/
	virtual void listTerms(
		vector<std::wstring>& vResult,
		bool fUserAbbreviationList
		) = 0;

/*! \brief list the terms of the user abbreviation list or the system abbreviation list.
    \param vResult reference to the vector that receives the abbreviations
    \param fUserAbbreviationList TRUE = return user abbreviations, FALSE = return system abbreviations
*/
	virtual void listTerms
		(
		STRINGLIST& strResult,
		bool fUserAbbreviationList
		) = 0;

/*! \brief clear the user abbreviation list
*/
	virtual void clearAbbreviations(
		) = 0;

  /*! \brief save the user abbreviations to disk
*/
	virtual void saveAbbreviations(
		) = 0;

};
#endif