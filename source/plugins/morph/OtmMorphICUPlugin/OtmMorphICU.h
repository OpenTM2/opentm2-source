/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#include "unicode/brkiter.h"
#pragma comment(lib, "icuuc.lib")
#include "unicode/rbbi.h"
#pragma comment(lib, "icuuc.lib")
#include <string>
#include <stdio.h>
#include "core\PluginManager\OtmMorph.h"

#define INCL_EQF_MORPH            // morphologic functions
#include "EQF.H"
#include "EQFSERNO.H"

/*! \brief class for morphology objects */
class OtmMorphICU : public OtmMorph
{

public:

/*! \brief constructor	 */
	OtmMorphICU(void);

/*! \brief constructor 
    \param vLanguage pointer to the language 
 */
	OtmMorphICU
		(
		const char* vLanguage
		);

/*! \brief destructor	 */
	~OtmMorphICU(void);

/*! \brief initiate the Hunspell object and lock object. 
	\returns true means success, other value means error.
 */
	bool init
		(
		);

	
/*! \brief Get stems of the specified term
	\param vTerm the term to be checked
	\param vResult the reference to a list that receives the stems.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error.
 */
	int stem
		(
		const wchar_t * pszTerm, 
		vector<std::wstring>& vResult
		);

/*! \brief Not supported yet, get part of speech info of the specified term
	\param vTerm the term to be checked
	\param vResult the pointer to a integer that receives the part of speech info.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error. 
 */
	int getPOS
		(
		const char* pszTerm, 
		int* piResult
		);

/*! \brief Tokenization of an input segment.
	\param pText points to the text being tokenized
	\param vResult the reference to a vector receiving the recognized terms
 */
	int tokenizeByTerm(const wchar_t* pText, TERMLIST& vResult);

/*! \brief Tokenization of an input segment by term.
	\param vSection the section to be tokenized.
	\param vResult the reference to a list that receives the terms.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error.
 */
	int tokenizeByTerm
		(
		const char* pszSection, 
		STRINGLIST& vResult
		);

/*! \brief Tokenization of an input segment by sentence.
	\param vSection the section to be tokenized.
	\param vResult the reference to a list that receives the sentences.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error.
 */
	int tokenizeBySentence
		(
		const char* pszSection, 
		STRINGLIST& vResult
		);

  /*! \brief Tokenization of an input segment by sentence.
	\param vSection the section to be tokenized.
	\param vResult the reference to a list that receives the sentences.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error.
 */
	int tokenizeBySentence
		(
		const wchar_t* pszSection, 
		TERMLIST& vResult
		);


/*! \brief Not supported yet, returns all decompositions for a compound word.
	\param vTerm the section to be checked.
	\param vResult the reference to a list that receives the decompositions.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error.
 */
	int compisol
		(
		const char* pszTerm, 
		STRINGLIST& vResult
		);

/*! \brief add the term to the user abbreviation list.
    \param vWord pointer to the word.
  	\returns ERROR_NULL_OBJECT means failed, SUCCESS_RETURN means the success.
*/
	int addAbbreviation(
		const wchar_t* vWord
		);

/*! \brief list the terms of the user abbreviation list or the system abbreviation list.
    \param vResult reference to the string list that receives the abbreviations
    \param fUserAbbreviationList TRUE = return user abbreviationa, FALSE = return system abbreviations
*/
	void listTerms(
		vector<std::wstring>& vResult,
		bool fUserAbbreviationList
		);

/*! \brief list the terms of the user abbreviation list or the system abbreviation list.
    \param vResult reference to the vector that receives the abbreviations
    \param fUserAbbreviationList TRUE = return user abbreviations, FALSE = return system abbreviations
*/
	void listTerms
		(
		STRINGLIST& strResult,
		bool fUserAbbreviationList
		);

/*! \brief clear the user abbreviation list
*/
	 void clearAbbreviations();

/*! \brief save the user abbreviation list to disk
*/
  void saveAbbreviations();


private:

/*! \brief get the result analyzed from the BreadIterator.
	\param pointer to the BreakIterator
	\param reference to the vector that restores the result 
    \param true if testing for sentence boundary
 */
	void getResultFromIterator(BreakIterator* vIterator, STRINGLIST& vResult, bool SentenceBoundaryCheck = false);

  /*! \brief get the result analyzed from the BreadIterator.
	\param pointer to the BreakIterator
	\param reference to the vector that restores the result 
 */
void getResultFromIterator( BreakIterator* vIterator, OtmMorph::TERMLIST& vResult, bool SentenceBoundaryCheck = false );

/*! \brief add the string of vUnicodeString into vResult
	\param vUnicodeString reference to the UnicodeString
	\param vResult reference to the vector
 */
	void addUnicodeString(UnicodeString& vUnicodeString, STRINGLIST& vResult);
	
	bool bAvailable;                 //true means available.
	
	string strLanguage;                //the language string

	BreakIterator* pWordBoundary;        //a break iterator used to do word tokenization of sections.

	BreakIterator* pSentenceBoundary;        //a break iterator used to do sentence tokenization of sections.

    BreakIterator* pWordBoundaryForSentence;  //a break iterator used by the sentence tokenization
    
    vector<std::wstring> abbrevlist; // where the abbreviation list is stored for comparison
    
    vector<std::wstring> userAbbrevList; // list of user abbreviations
    
    void loadAbbrevList( bool fUserAbbreviation ); // Load the user or system abbreviation list

    void buildAbbrevListName( bool fUserAbbreviation, char *pszNameBuffer ); // build file name of an abbreviation list


    BreakIterator* loadFromRules(); // create iterator from rule file if available

    const char* getLanguage(); // get the language prefix for the rule file to load

    void setupSentenceBoundary(UErrorCode &tStatus);
    
    bool isBreakOnAbbreviation(UnicodeString text, int start, int end);

    int checkTrailingSpaces(UnicodeString text, int start, int &end);

    int checkWordBoundary(UnicodeString text, int start, int &end);

  	int getTermType( UnicodeString text, int iStart, int iLen );

};

