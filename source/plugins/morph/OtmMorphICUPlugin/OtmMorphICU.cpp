/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "OtmMorphICU.h"
#include "core\utilities\LogWriter.h"

// for logging activate the following define 
//#define TOKENIZELOG 

/*! \brief constructor */
OtmMorphICU::OtmMorphICU(void)
{
}

/*! \brief constructor 
    \param vLanguage pointer to the language 
 */
OtmMorphICU::OtmMorphICU( const char* pszLanguage )
{
	bAvailable = false;
	strLanguage = pszLanguage;
	bAvailable = init();
}

/*! \brief destructor */
OtmMorphICU::~OtmMorphICU(void)
{
	if (NULL != pSentenceBoundary)
	{
		delete pSentenceBoundary;
	}
	if (NULL != pWordBoundary)
	{
		delete pWordBoundary;
	}
    if (NULL != pWordBoundaryForSentence) {
        delete pWordBoundaryForSentence;
    }
}

/*! \brief Not supported yet, returns all decompositions for a compound word.
	\param vTerm the section to be checked.
	\param vResult the reference to a list that receives the decompositions.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error.
 */
int OtmMorphICU::compisol( const char* pszTerm, STRINGLIST& vResult )
{
	return OtmMorph::ERROR_NO_SUPPORT;
}

/*! \brief Not supported yet, get part of speech info of the specified term
	\param vTerm the term to be checked
	\param vResult the pointer to a integer that receives the part of speech info.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error.
 */
int OtmMorphICU::getPOS( const char* pszTerm, int* piResult )
{
	return OtmMorph::ERROR_NO_SUPPORT;
}

/*! \brief Get stems of the specified term
	\param vTerm the term to be checked
	\param vResult the reference to a list that receives the stems.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error.
 */
int OtmMorphICU::stem( const wchar_t * pszTerm, vector<std::wstring>& vResult )
{
    /* This function is unsupported now.  Will be moved to hunspell plugin.
     * Just return ERROR_NO_SUPPORT for now.
     */
	
	return OtmMorph::ERROR_NO_SUPPORT;
}

/*! \brief Tokenization of an input segment.
	\param pText points to the text being tokenized
	\param vResult the reference to a vector receiving the recognized terms
 */
int OtmMorphICU::tokenizeByTerm(const wchar_t* pText, TERMLIST& vResult)
{
	if (NULL == pText)
	{
		return OtmMorph::ERROR_PARAMETAR;
	}

#ifdef TOKENIZELOG 
  LogWriter Log;
  Log.open( "MORPHICUTOKENIZE" );
  Log.writef( "tokenizeByTerm, input is \"%S\"", pText );
#endif

	pWordBoundary->setText(pText);
	getResultFromIterator(pWordBoundary, vResult);

#ifdef TOKENIZELOG 
  Log.write( "results:" );
  for (auto it = begin (vResult); it != end (vResult); ++it) 
  {
    Log.writef( "   Offset=%3ld Len=%3ld Flag=%ld", it->iStartOffset, it->iLength, it->iTermType  );
  }
  Log.close();
#endif

	return OtmMorph::SUCCESS_RETURN;
}

int OtmMorphICU::getTermType( UnicodeString text, int iStart, int iLen )
{
    // character classification flags
    BOOL       fAllCaps, fAllAlNum, fAllNumber, fAllWhiteSpace, fAllPunctuation; // TRUE = all characters are of given category
    BOOL       fContainsNumber, fContainsAlpha, fContainsNumberPunctuation; // TRUE = at least one character is of given category

    UChar c;

    if ( iLen < 1)
    {
      return( TERMTYPE_NOCOUNT | TERMTYPE_NOLOOKUP );
    }

    // Initialize processing flags
    fAllCaps = TRUE;
    fAllAlNum   = TRUE;
    fAllNumber  = TRUE;
    fAllWhiteSpace = TRUE;
    fAllPunctuation = TRUE;

    fContainsNumber = FALSE;
    fContainsAlpha  = FALSE;
    fContainsNumberPunctuation = FALSE;


    // scan the term
    for ( int i = 0; i < iLen; i++ )
    {
      c = text[iStart + i];

      if ( !iswalnum(c) ) fAllAlNum = FALSE;
      if ( !iswdigit(c) && (c != L',') && (c != L'.') && (c != L'/') && (c != L':') && (c != '-') ) fAllNumber  = FALSE;
      if ( !iswalpha(c) || iswlower(c) ) fAllCaps = FALSE;
      if ( !iswspace(c) ) fAllWhiteSpace = FALSE;
      if ( !iswpunct(c) ) fAllPunctuation = FALSE;

      if ( iswalpha(c) ) fContainsAlpha = TRUE;
      if ( iswdigit(c) ) fContainsNumber = TRUE;
      if ( (c == L',') || (c == L'.') || (c == L'/') || (c == L':') || (c == '-') ) fContainsNumberPunctuation = TRUE;
   } /* endfor */

  // evaluate term type
  if ( fAllWhiteSpace ) return( TERMTYPE_NOCOUNT | TERMTYPE_NOLOOKUP | TERMTYPE_WHITESPACE );
  if ( fAllPunctuation ) return( TERMTYPE_NOCOUNT | TERMTYPE_NOLOOKUP | TERMTYPE_PUNCTUATION );
  if ( fAllCaps ) return( TERMTYPE_ALLCAPS );
  if ( fAllNumber && fContainsNumberPunctuation  ) return( TERMTYPE_NOLOOKUP | TERMTYPE_NUMBER );
  if ( fAllNumber ) return( TERMTYPE_NOCOUNT | TERMTYPE_NOLOOKUP | TERMTYPE_NUMBER );
  if ( !fContainsAlpha && fContainsNumber ) return( TERMTYPE_NOLOOKUP );
  if ( !fContainsAlpha ) return( TERMTYPE_NOCOUNT | TERMTYPE_NOLOOKUP );
  return( 0 );
}

/*! \brief Tokenization of an input segment.
	\param vSection the section to be tokenized.
	\param vResult the reference to a list that receives the terms.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error.
 */
int OtmMorphICU::tokenizeByTerm( const char* pszSection, STRINGLIST& vResult )
{
	if (NULL == pszSection)
	{
		return OtmMorph::ERROR_PARAMETAR;
	}

#ifdef TOKENIZELOG 
  LogWriter Log;
  Log.open( "MORPHICUTOKENIZE" );
  Log.writef( "tokenizeByTerm, input is \"%s\"", pszSection );
#endif

	pWordBoundary->setText(UnicodeString(pszSection));
	getResultFromIterator(pWordBoundary, vResult);

#ifdef TOKENIZELOG 
  Log.write( "results:" );
  for (auto it = begin (vResult); it != end (vResult); ++it) 
  {
    Log.writef( "\"%s\"", it->c_str() );
  }
  Log.close();
#endif

  return OtmMorph::SUCCESS_RETURN;
}

/*! \brief initiate the class members. 
	\returns true means success, other value means error.
 */
bool OtmMorphICU::init()
{
	UErrorCode tStatus = U_ZERO_ERROR;
	pWordBoundary = BreakIterator::createWordInstance(Locale::getUS(), tStatus);
	if (U_FAILURE(tStatus))
	{
		return false;
	}
	
    setupSentenceBoundary(tStatus);
    if (U_FAILURE(tStatus))
    {
        return false;
    }

	return true;
}

// Return values for checkTrailingSpaces
#define MULTIPLE_NEWLINE_HANDLING 1
#define LIST_ITEM_HANDLING 2
#define WORD_DASH_WORD_HANDLING 3

/*! \brief get the result analyzed from the BreadIterator.
	\param pointer to the BreakIterator
	\param reference to the vector that restores the result 
 */
void OtmMorphICU::getResultFromIterator( BreakIterator* vIterator, OtmMorph::TERMLIST& vResult, bool SentenceBoundaryCheck )
{
    CharacterIterator *tStrIter = vIterator->getText().clone();
    UnicodeString  tUnicodeStr;
    tStrIter->getText(tUnicodeStr);
    struct OtmMorph::TERMENTRY Term;
    vResult.clear();

    int32_t prev_end = 0, end, start = vIterator->first();
    int32_t tmp_end = 0;
    int32_t string_length = tUnicodeStr.length();
    int32_t value = 0;
    for (end = vIterator->next(); end != BreakIterator::DONE; end = vIterator->next()) {
        prev_end = 0;
        if (SentenceBoundaryCheck) {
            if (isBreakOnAbbreviation(tUnicodeStr, start, end)) {
                prev_end = end;
                continue;
            }
            prev_end = end;
            value = checkTrailingSpaces(tUnicodeStr, start, end);
            if (value == 0) {
                prev_end = prev_end == end ? 0 : prev_end;
            } else if (value == LIST_ITEM_HANDLING) {
                continue;
            }
        } else {
            /*
             * This for the previous check for word-word.
             */
            if (tmp_end > end) {
                tmp_end = 0;
                continue;
            }
            prev_end = end;
            checkWordBoundary(tUnicodeStr, start, end);
            if (prev_end != end) {
                tmp_end = end;
                continue;
            }
        }

        Term.iStartOffset = start; 
        Term.iLength = end-start;
        if (!SentenceBoundaryCheck) {
            Term.iTermType = getTermType( tUnicodeStr, Term.iStartOffset, Term.iLength);
        }
        vResult.push_back( Term );

        start = end;
        
        if (SentenceBoundaryCheck && value == MULTIPLE_NEWLINE_HANDLING) {
          Term.iStartOffset = start; 
          Term.iLength = prev_end-start;
          //Term.iTermType = getTermType( tUnicodeStr, Term.iStartOffset, Term.iLength);
          vResult.push_back( Term );
            start = prev_end;
            prev_end = 0;
            value = 0;
        }
        if (end == string_length) {
            break;
        }
    }

    // This is necessary just because when the end of the string is an abbreviation,
    // it is not added to the result list.  Add it now.
    if (start != prev_end && prev_end != 0) {
      Term.iStartOffset = start; 
      Term.iLength = prev_end-start;
      Term.iTermType = getTermType( tUnicodeStr, Term.iStartOffset, Term.iLength);
      vResult.push_back( Term );
    }
    delete tStrIter;
}

/*! \brief get the result analyzed from the BreadIterator.
	\param pointer to the BreakIterator
	\param reference to the vector that restores the result 
 */
void OtmMorphICU::getResultFromIterator( BreakIterator* vIterator, STRINGLIST& vResult, bool SentenceBoundaryCheck )
{
    CharacterIterator *tStrIter = vIterator->getText().clone();
    UnicodeString  tUnicodeStr;
    tStrIter->getText(tUnicodeStr);

    int32_t prev_end = 0, end, start = vIterator->first();
    int32_t string_length = tUnicodeStr.length();
    int32_t value = 0;
    for (end = vIterator->next(); end != BreakIterator::DONE; end = vIterator->next()) {
        prev_end = 0;
        if (SentenceBoundaryCheck) {
            if (isBreakOnAbbreviation(tUnicodeStr, start, end)) {
                prev_end = end;
                continue;
            }
            prev_end = end;
            value = checkTrailingSpaces(tUnicodeStr, start, end);
            if (value == 0) {
                prev_end = prev_end == end ? 0 : prev_end;
            } else if (value == LIST_ITEM_HANDLING) {
                continue;
            }
        }

        addUnicodeString(UnicodeString(tUnicodeStr, start, end-start), vResult);

        start = end;
        
        if (value == MULTIPLE_NEWLINE_HANDLING) {
            addUnicodeString(UnicodeString(tUnicodeStr, start, prev_end-start), vResult);
            start = prev_end;
            prev_end = 0;
            value = 0;
        }
        if (end == string_length) {
            break;
        }
    }

    // This is necessary just because when the end of the string is an abbreviation,
    // it is not added to the result list.  Add it now.
    if (start != prev_end && prev_end != 0) {
        addUnicodeString(UnicodeString(tUnicodeStr, start, prev_end-start), vResult);
    }
    delete tStrIter;
}

/*! \brief Tokenization of an input segment.
	\param vSection the section to be tokenized.
	\param vResult the reference to a list that receives the sentences.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error.
 */
int OtmMorphICU::tokenizeBySentence( const char* vSection, STRINGLIST& vResult )
{
    if (NULL == vSection)
    {
        return OtmMorph::ERROR_PARAMETAR;
    }

#ifdef TOKENIZELOG 
  LogWriter Log;
  Log.open( "MORPHICUTOKENIZE" );
  Log.writef( "tokenizeBySentence, input is \"%s\"", vSection );
#endif

    pSentenceBoundary->setText(UnicodeString(vSection));
    pWordBoundaryForSentence->setText(UnicodeString(vSection));

    getResultFromIterator(pSentenceBoundary, vResult, TRUE);

#ifdef TOKENIZELOG 
  Log.write( "results:" );
  for (auto it = begin (vResult); it != end (vResult); ++it) 
  {
    Log.writef( "\"%s\"", it->c_str() );
  }
  Log.close();
#endif

    return OtmMorph::SUCCESS_RETURN;
}

/*! \brief Tokenization of an input segment.
	\param vSection the section to be tokenized.
	\param vResult the reference to a list that receives the sentences.
	\returns OtmMorph::SUCCESS_RETURN means success, other value means error.
 */
int OtmMorphICU::tokenizeBySentence( const wchar_t* vSection, TERMLIST& vResult )
{
    if (NULL == vSection)
    {
        return OtmMorph::ERROR_PARAMETAR;
    }

#ifdef TOKENIZELOG 
  LogWriter Log;
  Log.open( "MORPHICUTOKENIZE" );
  Log.writef( "tokenizeBySentence, input is \"%S\"", vSection );
#endif

    pSentenceBoundary->setText( vSection );
    pWordBoundaryForSentence->setText( vSection );

    getResultFromIterator(pSentenceBoundary, vResult, TRUE);

#ifdef TOKENIZELOG 
  Log.write( "results:" );
  for (auto it = begin (vResult); it != end (vResult); ++it) 
  {
    Log.writef( "   Offset=%3ld Len=%3ld Flag=%ld", it->iStartOffset, it->iLength, it->iTermType  );
  }
  Log.close();
#endif

    return OtmMorph::SUCCESS_RETURN;
}



/*! \brief add the string of vUnicodeString into vResult
	\param vUnicodeString reference to the UnicodeString
	\param vResult reference to the vector
 */
void OtmMorphICU::addUnicodeString(UnicodeString& vUnicodeString, STRINGLIST& vResult )
{
	char tCharBuf[256];
	UErrorCode tErrorCode = U_ZERO_ERROR;
	int32_t tLength = vUnicodeString.extract(tCharBuf, sizeof(tCharBuf)-1, NULL, tErrorCode);  
	if (tErrorCode == U_BUFFER_OVERFLOW_ERROR)
	{
		tErrorCode = U_ZERO_ERROR;
		char *tLargeCharBuf = new char[tLength + 1];
		vUnicodeString.extract(tLargeCharBuf, tLength + 1, NULL, tErrorCode);
		tLargeCharBuf[tLength] = 0;
		vResult.push_back(tLargeCharBuf);
		delete tLargeCharBuf;
	}
	else 
	{
		tCharBuf[tLength] = 0;  
		vResult.push_back(tCharBuf);
	}
}

void OtmMorphICU::loadAbbrevList( bool fUserAbbreviation ) {
    FILE* fpFilePointer;
    CHAR szName[100];

    buildAbbrevListName( fUserAbbreviation, szName );

    vector<std::wstring>*pList = ( fUserAbbreviation ) ? &userAbbrevList : &abbrevlist;
    
    pList->clear();

    if (0 == fopen_s(&fpFilePointer, szName, "r"))
    {
        char tWord[1024];
        wchar_t wWord[1024];
        while (!feof(fpFilePointer) && !ferror(fpFilePointer) && 0 != fgets(tWord, sizeof(tWord), fpFilePointer))
        {
            size_t len = strlen(tWord);
            if (len >= 2)
            {
                if (tWord[len - 1] == '\n')
                {
                    tWord[len - 1] = 0;
                }

                MultiByteToWideChar(CP_UTF8, 0, tWord, -1, wWord, 1024);
                pList->push_back(wWord);
            }
        }
        fclose(fpFilePointer);
    }
}

void OtmMorphICU::buildAbbrevListName( bool fUserAbbreviation, char *pszNameBuffer ) 
{
    UtlMakeEQFPath( pszNameBuffer, NULC, PLUGIN_PATH, NULL );
    strcat( pszNameBuffer, "\\OtmMorphICUPlugin\\AbbrevLists\\");
    strcat( pszNameBuffer, strLanguage.c_str());
    strcat( pszNameBuffer, ( fUserAbbreviation ) ? "_userabbrev.dic" : "_abbrev.dic" );
}


#define MAX_RULE_LINE 400
#define MAX_RULE_BUFFER_SIZE 200000
#define FILE_NAME_SUFFIX "-otm-rules.brk"
BreakIterator* OtmMorphICU::loadFromRules() {
    CHAR szName[100];
    memset(szName, 0, sizeof(szName));
    UtlMakeEQFPath(szName, NULC, PLUGIN_PATH, NULL);
    strcat(szName, "\\OtmMorphICUPlugin\\Rules\\");
    strcat(szName, getLanguage());
    strcat(szName, FILE_NAME_SUFFIX);

    FILE *file = fopen(szName, "rb");
    if (file == NULL) {
        return NULL;
    }

    // Loading rules from binary file
    UErrorCode status = U_ZERO_ERROR;
    // artificially set max at 2,000,000 bytes
    char *bytes2 = (char*)malloc(2000000);
    memset(bytes2, 0, 2000000);

    uint32_t length = fread(bytes2, 1, 2000000, file);

    BreakIterator *b = new RuleBasedBreakIterator((const uint8_t*)bytes2, length, status);
    if (b == NULL || U_FAILURE(status)) {
        delete b;
        return NULL;
    }
#if 0
    // This code loads the rules from text file.
    char line[MAX_RULE_LINE];
	char buffer[MAX_RULE_BUFFER_SIZE];
    line[0] = buffer[0] = 0;	

    while (fgets(line, MAX_RULE_LINE, file)) {
        if (line[0] != '#' && line[1] != 0) {
            line[strlen(line) - 1] = 0;
            strcat(buffer, line);
        }
    }
    fclose(file);

    BreakIterator* b = new RuleBasedBreakIterator(UnicodeString(buffer), UParseError(), status);
    if (b == NULL || U_FAILURE(status)) {
        delete b;
        return NULL;
    }
#endif

    return b;
}

#define LANG_ENG_1 "English"
#define LANG_ENG_2 "eng_"
const char* OtmMorphICU::getLanguage() {
    if (_strnicmp(strLanguage.c_str(), LANG_ENG_1, sizeof(LANG_ENG_1) - 1) == 0 ||
        _strnicmp(strLanguage.c_str(), LANG_ENG_2, sizeof(LANG_ENG_2) - 1) == 0) {
        return "english";
    } else {
        /*
         * Default to English for now instead of using the ICU default rules.
         * Most other languages (e.g. German) seem to follow the same sentence break
         * rules as English.  In the future, languages may need their own
         * separate sentence rules.
         */
        return "english";
    }

    return "";
}

void OtmMorphICU::setupSentenceBoundary(UErrorCode &tStatus) {
    /*
     * For most locales, the same default break iterator rules are used.
     * The current setting does not provide support for dictionary-based locales (e.g. Thai).
     */
    if (!(pSentenceBoundary = loadFromRules())) {
#ifdef TOKENIZELOG
  LogWriter Log;
  Log.open( "MORPHICUSETUP" );
  Log.writef( "Using default sentence segmentation rules." );
#endif
        pSentenceBoundary = BreakIterator::createSentenceInstance(Locale::getUS(), tStatus);
    }

    pWordBoundaryForSentence = BreakIterator::createWordInstance(Locale::getUS(), tStatus);
    if (U_FAILURE(tStatus)) {
        return;
    }

    // Read in abbreviation lists.  The "1" refers to the abbreviation list file.
    loadAbbrevList( true );   // load user abbreviation list
    loadAbbrevList( false );  // load system abbreviation list
}

#define MAX_ABREV_LENGTH 15 // Max size of the abbreviations?
bool OtmMorphICU::isBreakOnAbbreviation(UnicodeString text, int start, int end) {
	bool newline = false;
    int abbrevlist_size = abbrevlist.size();
    int userAbbrevList_size = userAbbrevList.size();
    
    if ( (abbrevlist_size <= 0) && (userAbbrevList_size  <= 0) ){
        return false;
    }

    UChar buffer[MAX_ABREV_LENGTH];

    int wb_end = 0, wb_start = 0, wb_length = 0;

    wb_end = end;
    for (;;) {
        // Find the previous word.  Could be spaces or punctuation so skip those.
        pWordBoundaryForSentence->preceding(wb_end);
        wb_start = pWordBoundaryForSentence->current();
        wb_length = wb_end - wb_start;
        
        if (wb_length >= MAX_ABREV_LENGTH || wb_length == 0 || wb_end <= start) {
            break;
        }
        
        text.extract(wb_start, wb_length, buffer);
        for (int i = 0; i < wb_length; i++) {
            if (buffer[i] == 0x000A || buffer[i] == 0x000D) {
                newline = true;
            }
        }
        if ((wb_length == 1 && (buffer[0] == ' ' || buffer[0] == '.')) || newline) {
            wb_end = wb_start;
            newline = false;
            continue;
        }
        
        // include previous punctuation
        if ((wb_length + wb_start) < text.length()) {
            if ((wb_length + 1) <= MAX_ABREV_LENGTH) {
                wb_length++;
            
                text.extract(wb_start, wb_length, buffer);
            }
        }

        // null terminate buffer
        buffer[wb_length] = 0;

        wstring wordToTest(buffer);
        
        for (int i = 0; i < abbrevlist_size; i++) {
            if (wordToTest == abbrevlist[i]) {
                return true;
            }
        }
        for (int i = 0; i < userAbbrevList_size; i++) {
            if (wordToTest == userAbbrevList[i]) {
                return true;
            }
        }
        
        if (wb_length >= 1) {
            break;
        }
        
        wb_end = wb_start;
    }

    return false;
}

#define MAX_TRAILING_SPACES_LENGTH_WITH_NO_NEWLINE 4 // Max number of trailing spaces to include
#define MAX_TRAILING_SPACES_LENGTH_WITH_NEWLINE 2 // Max number of trailing spaces to include when preceeded by newline
#define COLON_MARK 0x003A
#define PERIOD 0x002E
#define COMMA_MARK 0x002C
#define ASTERISK 0x002A
#define SINGLE_QUOTATION_MARK 0x0027
#define BLANK_SPACE 0x0020
#define TAB 0x0009
#define NEWLINE_1 0x000A
#define NEWLINE_2 0x000D
#define DASH 0x002D
#define FORWARD_SLASH 0x002F
#define AT_SIGN 0x0040
#define IS_WORD_CONCATENATION_SYMBOL(codepoint) ((codepoint == DASH) ||(codepoint == FORWARD_SLASH) ||(codepoint == AT_SIGN))
#define IS_NUMBER_CONCATENATION_SYMBOL(codepoint) ((codepoint == COLON_MARK))

// Special handling of newline and space characters
int OtmMorphICU::checkTrailingSpaces(UnicodeString text, int start, int &end) {
    bool isTab = false;
    int blank_count = 0;
    int newline_count = 0;
    int index = (end - 1);
    int length = text.length();
    int return_value = 0;
    
    // Check for newline before space and number item
    if ((start >= 1 && (text[start - 1] == NEWLINE_1 || text[start -1] == NEWLINE_2)) ||
        (text[start] == NEWLINE_1 || text[start] == NEWLINE_2)) {
        int tmp_index = start;
        while (tmp_index < end && (text[tmp_index] == NEWLINE_1 || text[tmp_index] == NEWLINE_2)) {
            tmp_index++;
        }
        while (tmp_index < end && text[tmp_index] == BLANK_SPACE) {
            blank_count++;
            tmp_index++;
        }
        if ((tmp_index + 3) < end && blank_count >= 8 && iswdigit(text[tmp_index]) && text[tmp_index + 1] == PERIOD && text[tmp_index + 2] == BLANK_SPACE && iswalpha(text[tmp_index + 3])) {
            end = tmp_index + 3;
            return 1;
        }
        blank_count = 0;
    }

    // Check newline first
    while (index > start && (text[index] == NEWLINE_1|| text[index] == NEWLINE_2)) {
        newline_count++;
        index--;
    }
    
    // Special handling of comments in source code
    if (newline_count) {
        int tmp_index_1 = index;
        int blank_count_1 = 0;
        while (tmp_index_1 >= start && text[tmp_index_1] == BLANK_SPACE) {
            blank_count_1++;
            tmp_index_1--;
        }
        if (text[tmp_index_1] == PERIOD) {
            tmp_index_1 = end;
            while (text[tmp_index_1] == BLANK_SPACE) {
                tmp_index_1++;
            }

            if (text[tmp_index_1] == ASTERISK) {
                end -= (newline_count);
                return return_value;
            }
        }
    }
    
    // Special handling of space newline space at end of string
    if (newline_count) {
        int tmp_index = index;
        while (tmp_index >= start && text[tmp_index] == BLANK_SPACE) {
            blank_count++;
            tmp_index--;
        }
        if (blank_count == 3) {
            tmp_index = end;
            blank_count = 0;
            while(text[tmp_index] == BLANK_SPACE) {
                blank_count++;
                tmp_index++;
            }

            if (blank_count == 6) {
                end -= (newline_count + 2);
                return return_value;
            }
        }
        blank_count = 0;
    }

    // For text. space newline spaces, then list item break after the first space
    if ((index - 3) >= 0 && text[index] == BLANK_SPACE && text[index - 1] == PERIOD && iswalpha(text[index - 2]) && iswalpha(text[index - 3])) {
        int tmp_index = end;
        while (text[tmp_index] == BLANK_SPACE) {
            tmp_index++;
        }

        if (tmp_index && iswdigit(text[tmp_index]) && text[tmp_index + 1] == PERIOD && text[tmp_index + 2] == BLANK_SPACE) {
            end -= newline_count;
            return return_value;
        }
    }

    // Check for spaces after newline
    // Include the trailing spaces if it is less than 3
    if (newline_count) {
        if (index > 0 && text[index] == BLANK_SPACE && text[index - 1] == PERIOD && text[end] == TAB) {
            end -= newline_count;
            return return_value;
        } else if (text[index] != COLON_MARK && text[end] == BLANK_SPACE || text[end] == TAB) {
            int tmp_index = end;
            while (tmp_index < length && (text[tmp_index] == BLANK_SPACE || text[tmp_index] == TAB)) {
                if (text[tmp_index] == TAB) {
                    isTab = true;
                }
                blank_count++;
                tmp_index++;
            }
            
            if (isTab) {
                if (blank_count == 1 && (index - 1) >= 0 && text[index] == BLANK_SPACE && text[index-1] == BLANK_SPACE) {
                    end -= (newline_count + 1);
                    return return_value;
                }
                int tmp_index_2 = index;
                int blank_count_2 = blank_count;
                if (blank_count_2 >= 2) {
                    blank_count_2 = 0;
                    while (tmp_index_2 > 0 && text[tmp_index_2] == BLANK_SPACE) {
                        blank_count_2++;
                        tmp_index_2--;
                    }
                    if (blank_count_2 > 1) {
                        end -= ((blank_count_2 -= 1) + newline_count);
                        return return_value;
                    }
                }
            }

            // check if the blank spaces are the end of the string
            if (blank_count <= MAX_TRAILING_SPACES_LENGTH_WITH_NEWLINE || (text[index] != PERIOD && tmp_index == length && text[tmp_index - 1] != TAB)) {
                end += blank_count;
                return return_value;
            }

            blank_count = 0;
        }
    }

    // Check quotes
    if (!newline_count && index > 0 && text[index] == BLANK_SPACE) {
        if (length > end) {
            if (text[end] == SINGLE_QUOTATION_MARK) {
                if (text.indexOf(SINGLE_QUOTATION_MARK, start, end) >= 0) {
                    // check spaces before quotation mark first
                    while (index > 0 && text[index] == BLANK_SPACE) {
                        blank_count++;
                        index--;
                    }

                    if (blank_count > MAX_TRAILING_SPACES_LENGTH_WITH_NO_NEWLINE) {
                        end -= (blank_count - 1);
                        return return_value;
                    }
                    
                    blank_count = 0;
                    
                    // skip over quotation mark
                    index = end + 1;
                    while (index < length) {
                        if (text[index] == BLANK_SPACE) {
                            blank_count++;
                        } else {
                            break;
                        }
                        index++;
                    }
                    end = index;
                    blank_count = 0;
                    index--;
                }
            }
        }
    }
    
    if (index > 0 && (text[index] == BLANK_SPACE || text[index] == TAB)) {
        // Check for space character or tab character
        while (index > start && (text[index] == BLANK_SPACE || text[index] == TAB)) {
            blank_count++;
            index--;
        }
    }

    // If the string ends with a newline and spaces, just keep the spaces
    if (!newline_count && blank_count && end == length && (text[index] == NEWLINE_1 || text[index] == NEWLINE_2)) {
        int tmp_index = index;
        while (text[tmp_index] == NEWLINE_1 || text[tmp_index] == NEWLINE_2) {
            newline_count++;
            tmp_index--;
        }
        if ((tmp_index - 1) >= 0 && text[tmp_index] == PERIOD && iswdigit(text[tmp_index - 1])) {
            return return_value;
        }
    }

    // If these are only blank spaces skip
    if ((end - start) == blank_count) {
        return return_value;
    }
    
    // Special handling for numeric list item
    if ((start == 0 && (text[start] >= 0x0030 && text[start] <= 0x0039) &&
                        text[start+1] == 0x002E && text[start+2] == BLANK_SPACE) &&
                        blank_count > MAX_TRAILING_SPACES_LENGTH_WITH_NO_NEWLINE) {
        return LIST_ITEM_HANDLING;
    }
    
    // Special handling for numeric list items not at the beginning of string
    if ( newline_count >= 4 && index >= 2 && text[index] == PERIOD &&
        (text[index - 1] >= 0x0030 && text[index - 1] <= 0x0039) && text[index - 2] == BLANK_SPACE) {
        return return_value;
    }
    
    // Special handling for ellipsis and newline
    if (newline_count && blank_count && index >= 4 &&
        text[index] == PERIOD && text[index - 1] == PERIOD && text[index -2] == PERIOD && text[index - 3] == BLANK_SPACE) {
        return return_value;
    }

    // Special handling of multiple newlines and spaces
    if (newline_count && blank_count && (text[index] == NEWLINE_1 || text[index] == NEWLINE_2)) {
        return return_value;
    }
    
    // Special handling of comma before newline
    if ( newline_count && blank_count && text[index] == COMMA_MARK) {
        return return_value;
    }

	// Return if not a sentence break
	if ( blank_count && index >= 0 && iswalpha(text[index])) {
        return return_value;
    }
    
    if (newline_count >= 4 && blank_count && text[index] != COLON) {
        end -= newline_count;
        newline_count = 0;
        return_value =  MULTIPLE_NEWLINE_HANDLING;
    }
    
    // Include the trailing spaces if they match the conditions below
    if (((text[index] < 0x0041 || text[index] > 0x005A) && text[index] != COLON_MARK) && 
        ((newline_count && blank_count > MAX_TRAILING_SPACES_LENGTH_WITH_NEWLINE) ||
         (blank_count > MAX_TRAILING_SPACES_LENGTH_WITH_NO_NEWLINE && !iswdigit(text[index])))) {
            end -= (blank_count + newline_count - 1);
            return return_value;
    }

    return return_value;
}

int OtmMorphICU::checkWordBoundary(UnicodeString text, int start, int &end) {
    int newline_count = 0;
    int index = (end - 1);
    int length = text.length();

    if ((index + 2) < length) {
        if (iswalnum(text[index]) && IS_WORD_CONCATENATION_SYMBOL(text[index + 1]) && iswalnum(text[index + 2])) {
            // alphanumbers-symbols-alphanumbers should be treated as one word
            end += 2;
        } else if (iswdigit(text[index]) && IS_NUMBER_CONCATENATION_SYMBOL(text[index + 1]) && iswdigit(text[index + 2])) {
            // numbers in time format (e.g. 12:00) should be treated as one word
            end += 2;
        }
    }

    return 0;
}

/*! \brief add the term to the user abbreviation list.
    \param vWord pointer to the word.
  	\returns ERROR_NULL_OBJECT means failed, SUCCESS_RETURN means the success.
*/
int OtmMorphICU::addAbbreviation( const wchar_t* vWord )
{
  userAbbrevList.push_back( vWord );
  return( true );
}

/*! \brief list the terms of the user abbreviation list or the system abbreviation list.
    \param vResult reference to the string list that receives the abbreviations
    \param fUserAbbreviationList TRUE = return user abbreviationa, FALSE = return system abbreviations
*/
void OtmMorphICU::listTerms( vector<std::wstring>& vResult, bool fUserAbbreviationList )
{
  vector<std::wstring>&list = ( fUserAbbreviationList ) ? userAbbrevList : abbrevlist;
  int len = list.size();
  for (int i = 0; i < len; i++) {
		vResult.push_back( list[i] ); 
  }
}

/*! \brief list the terms of the user abbreviation list or the system abbreviation list.
    \param vResult reference to the vector that receives the abbreviations
    \param fUserAbbreviationList TRUE = return user abbreviations, FALSE = return system abbreviations
*/
void OtmMorphICU::listTerms( STRINGLIST& strResult, bool fUserAbbreviationList )
{
  vector<std::wstring>&list = ( fUserAbbreviationList ) ? userAbbrevList : abbrevlist;
  char tWord[1024];
  int len = list.size();
  for (int i = 0; i < len; i++) {
    WideCharToMultiByte( CP_OEMCP, 0, list[i].c_str(), -1, tWord, 1024, NULL, NULL );
		strResult.push_back( tWord ); 
  }
}

/*! \brief clear the user abbreviation list
*/
void OtmMorphICU::clearAbbreviations()
{
  userAbbrevList.clear();
}


/*! \brief save the user abbreviation list to disk
*/
void OtmMorphICU::saveAbbreviations()
{
  char szName[100];

  buildAbbrevListName( true, szName );

  FILE* tFilePointer;
	if ( 0 == fopen_s( &tFilePointer, szName, "wt"))
	{
    char tWord[1024];
    int len = userAbbrevList.size();
    for (int i = 0; i < len; i++) {
      WideCharToMultiByte( CP_UTF8, 0, userAbbrevList[i].c_str(), -1, tWord, 1024, NULL, NULL );
			fputs( tWord, tFilePointer);
			fputc('\n', tFilePointer);
    }
		fclose(tFilePointer);
	}
}
