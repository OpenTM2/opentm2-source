/*! \file
	Description: API for morphological functions

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define MORPH_BUILD_COMP_STEMS         // use compound sep. for stem forms
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file

#include <eqfmorpi.h>                  // private header file of module
#include <eqfchtbl.h>                  // character tables
#include "OTMFUNC.H"            // function call interface public defines
#include "eqffunci.h"           // function call interface private defines

#include <ctype.h>
#ifndef INCL_EQF_MORPH
#define INCL_EQF_MORPH            // morphologic functions
#endif

#include "core\spell\SpellFactory.h"
#include "core\morph\MorphFactory.h"
#include "core\utilities\LogWriter.h"
#include "cxmlwriter.h"

#define MAX_TERMSIZE        256
#define REPLY_AREA_SIZE   40000        // size of reply area
#define LEMMA_LIST_SIZE    8096        // size of lemma list buffer
#define MAX_DICT_NAME       256

// activate the next define to measure the time in MorphTokenizeW calls
//#define MEASURE_TOKENIZE_TIME


/**********************************************************************/
/* Control block for this language exit                               */
/**********************************************************************/
typedef struct _LANGCB
{
	OtmSpell* m_SpellInstance;
	OtmMorph* m_MorphInstance;
	CHAR     szLanguage[MAX_LONGFILESPEC]; // name of language
	CHAR     szSpellPlugin[MAX_LONGFILESPEC]; // name of spell plugin
	CHAR     szMorphPlugin[MAX_LONGFILESPEC]; // name of morph plugin
	CHAR     szBuffer[CCHMAXPATH];      // general use buffer
	CHAR     szLemmaList[LEMMA_LIST_SIZE];  // buffer for lemma lists
  BOOL     fNoSpellSupport;           // TRUE = no spell support for this language available 
  BOOL     fNoStemInMorph;            // TRUE = stem form reduction is not supported in morph plugin
  BOOL     fNoStemInSpell;            // TRUE = stem form reduction is not supported in spell plugin
} LANGCB, *PLANGCB;

// activate this define to do logging
  //#define ACTIVATE_LOGGING
  //#define LOG_TOKENIZE
  //#define LOG_TOKENIZEW
  //#define LOG_JVMDLL_PRELOAD

#ifdef ACTIVATE_LOGGING
  CHAR szMLogFile[MAX_EQF_PATH] = "";
#endif

USHORT TokenizeW
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ_W    pszInData,                  // IN : ptr to data being tokenized
 PULONG   pulTermListSize,            // IN/OUT:  address of variable
                                      //    containing size of term list buffer
 PVOID    *ppTermList,                // IN/OUT: address of term list pointer
 USHORT   usListType                  // IN: type of term list MORPH_ZTERMLIST,
                                     //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                     //    or MORPH_FLAG_ZTERMLIST
 );
void MorphCheckSpellInstance( PLANGCB pLangCB );
USHORT MorphWStrings2TermList( vector<std::wstring> &vStrings, PVOID *ppTermList, PULONG pulTermListSize );


/**********************************************************************/
/**********************************************************************/
/* Global variables                                                   */
/**********************************************************************/
/**********************************************************************/
//static char chDummy1[1024] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
//static PLCB IDtoLCB[256] = {      // ID to language control block table
//PLCB IDtoLCB[256] = {      // ID to language control block table
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
//};
//static char chDummy2[1024] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
PLCB* ppIdToLCB = 0;
BOOL bLCBTableAllocated = FALSE;

// array with names of languages with specific types

typedef struct _MORPHLANGTYPE
{
  CHAR        szLanguage[MAX_LANG_LENGTH];// language name
  SHORT       sType;                      // language type
} MORPHLANGTYPE, *PMORPHLANGTYPE;

static MORPHLANGTYPE LangTypes[] =
{
  { "Chinese(simpl.)", MORPH_DBCS_LANGTYPE },
  { "Chinese(trad.)",  MORPH_DBCS_LANGTYPE },
  { "Japanese",        MORPH_DBCS_LANGTYPE },
  { "Korean",          MORPH_DBCS_LANGTYPE },
  { "Hebrew",          MORPH_BIDI_H_LANGTYPE },
  { "Arabic",          MORPH_BIDI_A_LANGTYPE },
  { "Thai",            MORPH_THAI_LANGTYPE },
  { "",                0 }                   // end of array indicator
};


#if defined(OS2_20)
  #define max( a, b ) ( a > b ) ? (a) : (b)
  #define min( a, b ) ( a < b ) ? (a) : (b)
#endif

#if defined(MEASURETIME)
  #define INITTIME( pLCB )  pLCB->ulLastTime = pLCB->pInfoSeg->msecs
  #define GETTIME( pLCB, ulTime )                       \
  {                                                     \
     ulTime += pLCB->pInfoSeg->msecs - pLCB->ulLastTime;\
     pLCB->ulLastTime = pLCB->pInfoSeg->msecs;          \
  }
#endif

/**********************************************************************/
/* indicator for DBCS initialisation                                  */
/**********************************************************************/
BOOL  fInit = FALSE;

//------------------------------------------------------------------------------
// Function name:     MorphInit
//------------------------------------------------------------------------------
// Description:       Initialization of the language exit.
//------------------------------------------------------------------------------
// Function call:     MorphInit( PVOID *ppvLangCB, PSZ pszMorphDict, PSZ pszSpellPlugin, PSZ pszMorphPlugin,
//                          USHORT usCodePage, USHORT usLAngCode );
//------------------------------------------------------------------------------
// Input parameter:   PSZ        szMorphDict  name of morphologic dictionary
//                    PSZ        pszSpellPlugin name of the spell check plugin
//                    PSZ        pszMorphPlugin name of the morph plugin
//                    USHORT     usCodePage   code page to be used
//                    USHORT     usLangCode   language code (= lang identifier)
//------------------------------------------------------------------------------
// Output parameter:  PVOID      *ppvLangCB   to buffer for language CB ptr
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      control block is allocated,
//                    morphologic service is begun,
//                    morphologic dictionary is activated
//------------------------------------------------------------------------------
// Function flow:     allocate language control block
//                    fill language control block
//                    begin Nlp service
//                    set Nlp code page
//                    activate morphologic dictionary
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphInit
(
 PVOID      *ppvLangCB,               // OUT: ptr to buffer for language CB ptr
 PSZ        pszLanguage,              // IN : name of language
 PSZ        pszSpellPlugin,           // IN : name of the spell plugin
 PSZ        pszMorphPlugin,           // IN : name of the morph plug-in
 USHORT     usCodePage,               // IN : code page to be used
 USHORT     usLangCode                // IN : language code (= lang identifier)
 )
{
	PLANGCB    pLangCB;                  // ptr to allocated language CB
	USHORT     usRC = MORPH_OK;          // function return code

  usLangCode; usCodePage; 
	/********************************************************************/
	/* Allocate language control block                                  */
	/********************************************************************/
	if ( !UtlAlloc( (PVOID *)&pLangCB, 0L, (LONG)sizeof(LANGCB), NOMSG ) )
	{
		usRC = MORPH_NO_MEMORY;
	} /* endif */

	if ( usRC == MORPH_OK )
	{
		*ppvLangCB = (PVOID)pLangCB;

    strcpy( pLangCB->szLanguage, pszLanguage );
    strcpy( pLangCB->szSpellPlugin, pszSpellPlugin );
    strcpy( pLangCB->szMorphPlugin, pszMorphPlugin );

    // delay activation of the speller instance until spell functionality is requried
    // .. nothing to do here for the spell instance...

    // activate the morphologic functions
		MorphFactory* tMorphFactoryInstance = MorphFactory::getInstance();
		if (NULL == tMorphFactoryInstance)
		{
			usRC = MORPH_NO_MEMORY;
		}
		pLangCB->m_MorphInstance = tMorphFactoryInstance->getMorph( pszLanguage, pszMorphPlugin );
		if (NULL == pLangCB->m_MorphInstance)
		{
			usRC = MORPH_NO_MEMORY;
		}

		if (MORPH_OK != usRC)
		{
			UtlAlloc( (PVOID *)&pLangCB, 0L, 0L, NOMSG );
		}
	} /* endif */

	return( usRC );
} /* end of function INIT */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     Tokenize
//------------------------------------------------------------------------------
// Description:       Tokenization of an input segment. The tokenized terms
//                    are returned in form of a term list.
//------------------------------------------------------------------------------
// Function call:     Tokenize( LX_CB_P pNLPCB,
//                              PSZ pszInData,
//                              USHORT usDataLength,
//                              PUSHORT pusTermListSize, PVOID *ppTermList,
//                              USHORT usListType );
//------------------------------------------------------------------------------
// Input parameter:   LX_CB_P  pNLPCB          ptr to NLP control Block
//                    PSZ      pszInData       ptr to data being tokenized
//                    PUSHORT  pusTermListSize address of term list size var
//                    PVOID    *ppTermList     address of term list pointer
//                    USHORT   usListTyp       type of term list:
//                                             MORPH_ZTERMLIST or
//                                             MORPH_OFFSLIST
//                                             at moment only MORPH_ZTERMLIST
//                                             assumed!!!!!
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------
USHORT Tokenize
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ      pszInData,                  // IN : ptr to data being tokenized
 PUSHORT  pusTermListSize,            // IN/OUT:  address of variable
                                      //    containing size of term list buffer
 PVOID    *ppTermList,                // IN/OUT: address of term list pointer
 USHORT   usListType                  // IN: type of term list MORPH_ZTERMLIST,
                                     //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                     //    or MORPH_FLAG_ZTERMLIST
 )
{
    USHORT     usReturn = 0;             // return code
    USHORT     usTermBufUsed = 0;        // amount of space used in term buffer
    BOOL       fOffsList;                // TRUE = return a offset/length list
    LONG       lFlags;                   // flags for current term/token
    BOOL       fAllCaps, fAlNum, fNumber;// character classification flags
    BOOL       fSkip, fNewSentence,      // token processing flags
        fSingleToken;

  USHORT usCurrentPos = 0;

    fOffsList  = (usListType == MORPH_OFFSLIST) ||
        (usListType == MORPH_FLAG_OFFSLIST);

    /********************************************************************/
    /* Initialize processing flags                                      */
    /********************************************************************/
    fAllCaps = TRUE;
    fAlNum   = TRUE;
    fNumber  = TRUE;

    /********************************************************************/
    /* Work on input data                                               */
    /********************************************************************/

  PLANGCB tPLangCB = (PLANGCB)pvLangCB;
  if (NULL == tPLangCB)
  {
    return MORPH_INV_PARMS;
  }
  OtmMorph* tMorphInstance = tPLangCB->m_MorphInstance;
  if (NULL == tMorphInstance)
  {
    return MORPH_NOT_FOUND;
  }

  vector<string> tSentenceList;
  int tRet = tMorphInstance->tokenizeBySentence(pszInData, tSentenceList);
  if (0 != tRet)
  {
    return MORPH_FUNC_FAILED;
  }

  for (int j = 0; j < (int)tSentenceList.size(); j++)
  {
    if (!usReturn)
    {
      if ( (usListType == MORPH_FLAG_ZTERMLIST) ||
        (usListType == MORPH_FLAG_OFFSLIST) )
      {
        usReturn = MorphAddTermToList( (PSZ *)ppTermList,
          pusTermListSize,
          &usTermBufUsed,
          " ",
          1,
          0, // no offset possible
          TF_NEWSENTENCE,
          usListType );
      } /* endif */
    } /* endif */

    vector<string> tTermList;
    int tRet = tMorphInstance->tokenizeByTerm(tSentenceList[j].c_str(), tTermList);
    if (0 != tRet)
    {
      return MORPH_FUNC_FAILED;
    }

    for (size_t i = 0; i < tTermList.size(); i++)
    {
      fSkip = FALSE;
      fSingleToken = FALSE;
      fSkip = FALSE;
      fAllCaps = TRUE;
      fAlNum = TRUE;
      fNumber = FALSE;

      if (tTermList[i].length() < 1)
      {
        continue;
      }
      else if (tTermList[i].length() == 1)
      {

        const char* pszCurPos = tTermList[i].c_str();
        char pszNextPos = pszInData[usCurrentPos + 1];

        switch ( *pszCurPos )
        {
        case '.' :
        case '!' :
        case '?' :
          if ( pszNextPos == ' ' )
          {
            fNewSentence = TRUE;
            fSingleToken  = TRUE;
          }
          else
          {
            if ( *pszCurPos != '.' )
            {
              fSingleToken  = TRUE;
            }
            else
            {
              fAllCaps = FALSE;
              fAlNum   = FALSE;
            } /* endif */
          } /* endif */
          break;

        case ' ' :
          // Don't skip blank spaces (mow)
          //fSkip = TRUE;
          break;

        case '$' :
        case ':' :
        case '#' :
          fAllCaps = FALSE;
          fAlNum   = FALSE;
          break;

        case '/' :
        case '\\' :
        case ',' :
        case ';' :
        case '-' :
          if ( pszNextPos == ' ' )
          {
            fSingleToken  = TRUE;
          }
          else
          {
            fAllCaps = FALSE;
            fAlNum   = FALSE;
            fNumber  = FALSE;
          } /* endif */
          break;

        case '(' :
        case ')' :
        case '\"' :
          fSingleToken  = TRUE;
          break;

        default:
          BYTE bTemp = (BYTE)*pszCurPos;
          if ( !isalnum(bTemp) )
          {
            fAlNum   = FALSE;
          } /* endif */
          if ( !isdigit(bTemp) )
          {
            fNumber  = FALSE;
          } /* endif */
          if ( !isalpha(bTemp) || islower(bTemp) )
          {
            fAllCaps = FALSE;
          } /* endif */
          break;
        } /* endswitch */
      }

      for (size_t k = 0; k < tTermList[i].length(); k++)
      {
        BYTE bTemp = (BYTE)tTermList[i][k];
        if (!isalnum(bTemp) )
        {
          fAlNum = FALSE;
        }
        if (isdigit(bTemp) )
        {
          fNumber = TRUE;
        }
        if ( !isalpha(bTemp) || islower(bTemp) )
        {
          fAllCaps = FALSE;
        }
      }

      if(!fSingleToken && !fSkip)
      {
        lFlags = 0L;
        if ( fAllCaps ) lFlags |= TF_ALLCAPS;
        if ( !fAlNum )  lFlags |= TF_NOLOOKUP;
        if ( fNumber )  lFlags |= TF_NUMBER;
        strcpy( tPLangCB->szLemmaList, tTermList[i].c_str());
        usReturn = MorphAddTermToList( (PSZ *)ppTermList,
          pusTermListSize,
          &usTermBufUsed,
          tPLangCB->szLemmaList,
          (USHORT)strlen(tPLangCB->szLemmaList),
          (USHORT)(fOffsList ? usCurrentPos : 0),
          lFlags,
          usListType );
      }

      if ( fSingleToken && !usReturn )
      {
        strcpy( tPLangCB->szLemmaList, tTermList[i].c_str());
        usReturn = MorphAddTermToList( (PSZ *)ppTermList,
          pusTermListSize,
          &usTermBufUsed,
          tPLangCB->szLemmaList,
          1,
          (USHORT)(fOffsList ? usCurrentPos : 0),
          TF_NOLOOKUP,
          usListType );

      } /* endif */

      usCurrentPos += (USHORT)tTermList[i].length();

    }

  }

    /*****************************************************************/
    /* terminate the term list                                       */
    /*****************************************************************/
    if ( !usReturn )
    {
        usReturn = MorphAddTermToList( (PSZ *)ppTermList,
            pusTermListSize,
            &usTermBufUsed,
            (PSZ)EMPTY_STRING,
            0,
            0,
            0L,
            usListType );
    } /* endif */

    return (usReturn);

} /* end of function TOKENIZE */

USHORT TokenizeW
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ_W    pszInData,                  // IN : ptr to data being tokenized
 PULONG   pulTermListSize,            // IN/OUT:  address of variable
                                      //    containing size of term list buffer
 PVOID    *ppTermList,                // IN/OUT: address of term list pointer
 USHORT   usListType                  // IN: type of term list MORPH_ZTERMLIST,
                                     //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                     //    or MORPH_FLAG_ZTERMLIST
 )
{
	USHORT     usReturn = 0;             // return code
	ULONG      ulTermBufUsed = 0;        // amount of space used in term buffer
	BOOL       fOffsList;                // TRUE = return a offset/length list
#ifdef LOG_TOKENIZEW
  LogWriter Log;
#endif

	fOffsList  = (usListType == MORPH_OFFSLIST) ||
		(usListType == MORPH_FLAG_OFFSLIST);

	/********************************************************************/
	/* Work on input data                                               */
	/********************************************************************/

  PLANGCB tPLangCB = (PLANGCB)pvLangCB;
  if (NULL == tPLangCB)
  {
    return MORPH_INV_PARMS;
  }
  OtmMorph* tMorphInstance = tPLangCB->m_MorphInstance;
  if (NULL == tMorphInstance)
  {
    return MORPH_NOT_FOUND;
  }

#ifdef LOG_TOKENIZEW
        Log.open( "TOKENIZEW" );
        Log.writef( "Input string is >>>%S<<<", pszInData );
#endif

  // split text block into segments (sentences)
  OtmMorph::TERMLIST vSentenceList;
  int tRet = tMorphInstance->tokenizeBySentence( pszInData, vSentenceList );
  if (0 != tRet)
  {
    return MORPH_FUNC_FAILED;
  }

#ifdef LOG_TOKENIZEW
  Log.write( "---------------results from tokenizeBySentence----------------" );
  for (size_t j = 0; j < vSentenceList.size(); j++)
  {
    PSZ_W pszStartOfSegment = pszInData + vSentenceList[j].iStartOffset;
    wchar_t chTemp = pszStartOfSegment[vSentenceList[j].iLength];
    pszStartOfSegment[vSentenceList[j].iLength] = 0;
    Log.writef( "Segment %ld >>>%S<<<", j, pszStartOfSegment );
    pszStartOfSegment[vSentenceList[j].iLength] = chTemp;
  }
  Log.write( "--------------------------------------------------------------" );
#endif


  for (size_t j = 0; j < vSentenceList.size(); j++)
  {
    if (!usReturn)
    {
      if ( (usListType == MORPH_FLAG_ZTERMLIST) ||
        (usListType == MORPH_FLAG_OFFSLIST) )
      {
        usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList, pulTermListSize, &ulTermBufUsed, L" ", 1, 0, TF_NEWSENTENCE, usListType );
      } /* endif */
    } /* endif */

    // split segment/sentences into words
    OtmMorph::TERMLIST vTermList;
    PSZ_W pszStartOfSegment = pszInData + vSentenceList[j].iStartOffset;
    wchar_t chTemp = pszStartOfSegment[vSentenceList[j].iLength];
    pszStartOfSegment[vSentenceList[j].iLength] = 0;
    int tRet = tMorphInstance->tokenizeByTerm( pszStartOfSegment, vTermList );
#ifdef LOG_TOKENIZEW
    Log.writef( "---------tokenizeByTerm of string >>>%S<<<---------", pszStartOfSegment );
    for( size_t i = 0; i < vTermList.size(); i++ )
    {
      std::string strFlags = "";
      if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_NOLOOKUP )    strFlags += "TERMTYPE_NOLOOKUP ";
      if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_NUMBER )      strFlags += "TERMTYPE_NUMBER ";
      if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_ABBR )        strFlags += "TERMTYPE_ABBR ";
      if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_ALLCAPS )     strFlags += "TERMTYPE_ALLCAPS ";
      if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_NEWSENTENCE ) strFlags += "TERMTYPE_NEWSENTENCE ";
      if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_PREFIX )      strFlags += "TERMTYPE_PREFIX ";
      if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_COMPLEX )     strFlags += "TERMTYPE_COMPLEX ";
      if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_INITCAP )     strFlags += "TERMTYPE_INITCAP ";
      if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_WHITESPACE )  strFlags += "TERMTYPE_WHITESPACE ";
      if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_PUNCTUATION ) strFlags += "TERMTYPE_PUNCTUATION ";
      wchar_t chTemp2 = pszStartOfSegment[vTermList[i].iStartOffset+vTermList[i].iLength];
      pszStartOfSegment[vTermList[i].iStartOffset+vTermList[i].iLength] = 0;
      Log.writef( "Term %3ld: Start=%3ld, Length=%3ld, Term=\"%S\", Flags=%s", i, vTermList[i].iStartOffset, vTermList[i].iLength, 
        pszStartOfSegment + vTermList[i].iStartOffset, strFlags.c_str() );
      pszStartOfSegment[vTermList[i].iStartOffset+vTermList[i].iLength] = chTemp2;
    }
    Log.write( "--------------------------------------------------------------" );
#endif
    pszStartOfSegment[vSentenceList[j].iLength] = chTemp;


    if (0 != tRet)
    {
      return MORPH_FUNC_FAILED;
    }
    else
    {
      // convert returned term list into morph term list
      for( size_t i = 0; i < vTermList.size(); i++ )
      {
        USHORT usPos = (USHORT)(vSentenceList[j].iStartOffset + vTermList[i].iStartOffset);
        if ( (vTermList[i].iTermType & OtmMorph::TERMTYPE_WHITESPACE) != 0 )
        {
          // ignore whitespace tokens
        }
        else if ( vTermList[i].iLength == 0 ) 
        {
          // ignore empty tokens
        }
        else
        {
          usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList, pulTermListSize, &ulTermBufUsed, pszStartOfSegment + vTermList[i].iStartOffset, 
                                            (USHORT)vTermList[i].iLength, usPos, vTermList[i].iTermType, usListType );
        }
      }
    }
  }

	/*****************************************************************/
	/* terminate the term list                                       */
	/*****************************************************************/
	if ( !usReturn )
	{
		usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList, pulTermListSize, &ulTermBufUsed, L" ", 0, 0, 0L, usListType );
	} /* endif */

#ifdef LOG_TOKENIZEW
  Log.close();
#endif

	return (usReturn);

} /* end of function TOKENIZEW */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     VERIFY
//------------------------------------------------------------------------------
// Description:       Verifies if the term pszTerm is spelled correctly.
//------------------------------------------------------------------------------
// Function call:     VERIFY( PVOID *ppvLangCB, PSZ pszTerm );
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//                    PSZ      pszTerm         ptr to term being verified
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK        term is spelled correctly
//                    MORPH_NOT_FOUND term not found in dictionaries
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     call NlpVerify
//                    set and return return code
//------------------------------------------------------------------------------
USHORT Verify
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ_W    pszTerm                     // IN : ptr to term being verified
 )
{
	USHORT usRC = MORPH_OK;
	PLANGCB pLangCB = (PLANGCB)pvLangCB;

	if (NULL == pLangCB)
	{
		return MORPH_INV_PARMS;
	}

  MorphCheckSpellInstance( pLangCB );

	OtmSpell* tSpellInstance = pLangCB->m_SpellInstance;
	if (NULL == tSpellInstance)
	{
		return MORPH_FUNC_NOT_SUPPORTED;
	}

	int tRet = tSpellInstance->spell(pszTerm);

	if (0 == tRet)
	{
		return MORPH_NOT_FOUND;
	}

	return( usRC );

} /* end of function VERIFY */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     SpellAid
//------------------------------------------------------------------------------
// Description:       Returns possible spelling for the term pszTerm in form
//                    of a term list (only valid list type is MORPH_ZTERMLIST).
//------------------------------------------------------------------------------
// Function call:     SpellAid( PVOID pvLangCB, PSZ pszTerm,
//                              PUSHORT pusTermListSIze, PVOID *ppTermList,
//                              USHORT usListType );
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//                    PSZ      pszTerm         ptr to mispelled term
//                    PUSHORT  pusTermListSize address of term list size var
//                    PVOID    *ppTermList     address of term list pointer
//                    USHORT   usListTyp       type of term list:
//                                             MORPH_ZTERMLIST or
//                                             MORPH_OFFSLIST
//------------------------------------------------------------------------------
// Output parameter:  PVOID      *ppvLangCB   to buffer for language CB ptr
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     call NlpSpellAid
//                    add all returned spelling to term list using
//                    terminate term list
//                    return function return code
//------------------------------------------------------------------------------
USHORT SpellAid
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ_W    pszTerm,                    // IN : ptr to mispelled term
 PUSHORT  pusTermListSize,            // IN/OUT:  address of variable
                                      //    containing size of term list buffer
 PVOID    *ppTermList,                // IN/OUT: address of term list pointer
 USHORT   usListType                  // IN: type of term list: always
                                      //     MORPH_ZTERMLIST
 )
{
	USHORT usRC = MORPH_OK;
  ULONG ulTermListSize = *pusTermListSize;

  if ( usListType != MORPH_ZTERMLIST )
  {
    return ( MORPH_FUNC_NOT_SUPPORTED );
  }

	PLANGCB pLangCB = (PLANGCB)pvLangCB;
	if (NULL == pLangCB)
	{
		return MORPH_INV_PARMS;
	}

  MorphCheckSpellInstance( pLangCB );

	OtmSpell* tSpellInstance = pLangCB->m_SpellInstance;
	if (NULL == tSpellInstance)
	{
		return MORPH_NOT_FOUND;
	}

	vector<wstring> vTermList;
	int iWordCount = tSpellInstance->suggest(pszTerm, vTermList);

	if (iWordCount <= 0)
	{
		return MORPH_NOT_FOUND;
	}

  usRC = MorphWStrings2TermList( vTermList, ppTermList, &ulTermListSize );
  *pusTermListSize = (USHORT)ulTermListSize;

	return( usRC );
} /* end of function SPELLAID */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TermAdd
//------------------------------------------------------------------------------
// Description:       Adds the given term to the addenda dictionary.
//------------------------------------------------------------------------------
// Function call:     TermAdd( PVOID pvLangCB, PSZ pszTerm );
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//                    PSZ      pszTerm         ptr to term being added
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     call NlpAddWord to add word to addenda
//                    set return code
//                    return function return code
//------------------------------------------------------------------------------
USHORT TermAdd
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ_W    pszTerm                     // IN : ptr to term being added
 )
{
	USHORT usRC = MORPH_OK;

	PLANGCB pLangCB = (PLANGCB)pvLangCB;
	if (NULL == pLangCB)
	{
		return MORPH_INV_PARMS;
	}

  MorphCheckSpellInstance( pLangCB );

	OtmSpell* tSpellInstance = pLangCB->m_SpellInstance;
	if (NULL == tSpellInstance)
	{
		return MORPH_FUNC_NOT_SUPPORTED;
	}

	int tRet = tSpellInstance->addTerm(pszTerm, ADDENDA_DICT);
	if (0 != tRet)
	{
		return MORPH_FUNC_NOT_SUPPORTED;
	}
	tSpellInstance->writeTermToFile(ADDENDA_DICT);

	return( usRC );

} /* end of function TERMADD */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     Lemma
//------------------------------------------------------------------------------
// Description:       Returns all lemmas (stem forms) for term pszTerm
//                    in form of a term list.
//------------------------------------------------------------------------------
// Function call:     Lemma( PVOID pvLangCB, PSZ pszTerm,
//                           PUSHORT pusTermListSize, PVOID *ppTermList,
//                           USHORT usListType );
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//                    PSZ      pszTerm         ptr to term
//                    PUSHORT  pusTermListSize address of term list size var
//                    PVOID    *ppTermList     address of term list pointer
//                    USHORT   usListTyp       type of term list: only
//                                             MORPH_ZTERMLIST is valid
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     call NlpMorphID
//                    add all lemmas to term list
//                    terminate term list
//                    return function return code
//------------------------------------------------------------------------------
USHORT Lemma
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ_W    pszTerm,                    // IN : ptr to term being added
 PUSHORT  pusTermListSize,            // IN/OUT:  address of variable
									                    //    containing size of term list buffer
 PVOID    *ppTermList,                // IN/OUT: address of term list pointer
 USHORT   usListType                  // IN: type of term list: always
								                      //     MORPH_ZTERMLIST
 )	
{
	PLANGCB    pLangCB;                  // ptr to allocated language CB
	USHORT     usRC = MORPH_OK;          // function return code

  usListType;

	pLangCB = (PLANGCB)pvLangCB;

	if (NULL == pLangCB)
	{
		return MORPH_INV_PARMS;
	}

  vector<std::wstring> vTermList;

  // try morph plugin 
  if ( !pLangCB->fNoStemInMorph )
  {
	  OtmMorph* tMorphInstance = pLangCB->m_MorphInstance;
	  if (NULL == tMorphInstance)
	  {
      pLangCB->fNoStemInMorph = TRUE;
		  return MORPH_NOT_FOUND;
	  }

	  int iRC = tMorphInstance->stem( pszTerm, vTermList );

	  if (iRC == OtmMorph::ERROR_NO_SUPPORT )
	  {
      pLangCB->fNoStemInMorph = TRUE;
    }
    else if ( iRC == MORPH_OK )
    {
      ULONG ulTermListSize = *pusTermListSize;
      usRC = MorphWStrings2TermList( vTermList, ppTermList, &ulTermListSize );
      *pusTermListSize = (USHORT)ulTermListSize;
      return( usRC );
	  }
    else
    {
      return( (USHORT)iRC );
    } /* endif */
  } /* endif */

  // try spell plugin
  MorphCheckSpellInstance( pLangCB );

  if ( pLangCB->fNoSpellSupport )
  {
    return( MORPH_FUNC_NOT_SUPPORTED );
  } /* endif */     

  if ( pLangCB->fNoStemInSpell )
  {
    return( MORPH_FUNC_NOT_SUPPORTED );
  } /* endif */     

  int iRC = pLangCB->m_SpellInstance->stem( pszTerm, vTermList );
	if (iRC == OtmSpell::ERROR_NO_SUPPORT )
	{
    pLangCB->fNoStemInSpell = TRUE;
  }
  else if ( iRC == MORPH_OK )
  {
    ULONG ulTermListSize = *pusTermListSize;
    usRC = MorphWStrings2TermList( vTermList, ppTermList, &ulTermListSize );
    *pusTermListSize = (USHORT)ulTermListSize;
	}
  else
  {
    usRC = (USHORT)iRC;
  }

	return( usRC );

} /* end of function LEMMA */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     GetPos
//------------------------------------------------------------------------------
// Description:       Get part-of-speech data for term pszTerm. The returned
//                    information are the ORed POS values (e.g. MORPH_VERB |
//                    MORPH_NOUN )
//------------------------------------------------------------------------------
// Function call:     GetPos( PVOID pvLangCB, PSZ pszTerm, PUSHORT pusPOSInfo )
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//                    PSZ      pszTerm         ptr to term
//                    PUSHORT  pusPOSInfo      address of POS info buffer
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     call NlpMorphID
//                    gather returned POS info
//                    return function return code
//------------------------------------------------------------------------------
USHORT GetPos
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ      pszTerm,                    // IN : ptr to term
 PUSHORT  pusPOSInfo                  // OUT: part-of-speech info for term
 )
{
	USHORT  usRC = MORPH_NOT_FOUND;

  pusPOSInfo; pszTerm; pvLangCB;

	return( usRC );

} /* end of function GETPOS */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     Terminate
//------------------------------------------------------------------------------
// Description:       Terminate the language exit. All dictionaries are closed
//                    and any allocated storage is freed.
//------------------------------------------------------------------------------
// Function call:     Terminate( PVOID pvLangCB );
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      all dictionaries are close and all memory is freed
//------------------------------------------------------------------------------
// Function flow:     deactivate main morphologic dictionary
//                    if addenda dictionary is active
//                      deactivate addenda dictionary
//                    endif
//                    end Nlp service
//                    free language control block area
//                    return function return code
//------------------------------------------------------------------------------
USHORT Terminate
(
 PVOID pvLangCB                       // IN : ptr to buffer for language CB ptr
 )
{
	PLANGCB    pLangCB;                  // ptr to allocated language CB
	USHORT     usRC = MORPH_OK;          // function return code

	pLangCB = (PLANGCB)pvLangCB;

	/********************************************************************/
	/* free memory for NLP services control block  and language CB      */
	/********************************************************************/
	UtlAlloc( (PVOID *) &pLangCB, 0L, 0L, NOMSG );

	SpellFactory::close();
	MorphFactory::close();

	return( usRC );

} /* end of function Terminate */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     Compisol
//------------------------------------------------------------------------------
// Description:       Returns all decompositions for a compound word. This
//                    function is supported only for the germanic languages.
//                    If there are mor decompositions for a term these
//                    decompositions are seperated by a EOS delimter.
//------------------------------------------------------------------------------
// Function call:     Compisol( PVOID pvLangCB, PSZ pszTerm,
//                              PUSHORT pusTermListSize, PVOID *ppTermList,
//                              USHORT usListType );
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//                    PSZ      pszTerm         ptr to mispelled term
//                    PUSHORT  pusTermListSize address of term list size var
//                    PVOID    *ppTermList     address of term list pointer
//                    USHORT   usListTyp       only MORPH_ZTERMLIST is valid
//------------------------------------------------------------------------------
// Output parameter:  PVOID      *ppvLangCB   to buffer for language CB ptr
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     call NlpEntry with LX_COMPOUND_WD_ISOL function
//                    add all returned components t term list
//                    terminate term list
//                    return function return code
//------------------------------------------------------------------------------
USHORT Compisol
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ      pszTerm,                    // IN : ptr to mispelled term
 PUSHORT  pusTermListSize,            // IN/OUT:  address of variable
									  //    containing size of term list buffer
 PVOID    *ppTermList,                // IN/OUT: address of term list pointer
 USHORT   usListType                  // IN: type of term list: always
									  //     MORPH_ZTERMLIST
 )
{
	USHORT     usRC = MORPH_OK;          // function return code

  pvLangCB; pszTerm; pusTermListSize; ppTermList; usListType;

	usRC = MORPH_NOT_FOUND;

	return( usRC );

} /* end of function COMPISOL */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     AbbrevAdd
//------------------------------------------------------------------------------
// Description:       Adds the given term to the abbrev dictionary.
//------------------------------------------------------------------------------
// Function call:     AbbrevAdd( PVOID pvLangCB, PSZ pszTerm );
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//                    PSZ      pszTerm         ptr to term being added
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     call NlpAddWord to add word to addenda
//                    set return code
//                    return function return code
//------------------------------------------------------------------------------
USHORT AbbrevAdd
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ_W    pszTerm                     // IN : ptr to term being added
 )
{
	USHORT usRC = MORPH_OK;

	PLANGCB tPLangCB = (PLANGCB)pvLangCB;
	if (NULL == tPLangCB)
	{
		return MORPH_INV_PARMS;
	}
	OtmMorph* tMorphInstance = tPLangCB->m_MorphInstance;
	if (NULL == tMorphInstance)
	{
		return MORPH_FUNC_NOT_SUPPORTED;
	}

  int tRet = tMorphInstance->addAbbreviation( pszTerm );
	if (1 != tRet)
	{
		return MORPH_FUNC_NOT_SUPPORTED;
	}
  tMorphInstance->saveAbbreviations();

	return( usRC );

} /* end of function ABBREVADD */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ListDict
//------------------------------------------------------------------------------
// Description:       Returns all the word of the dictionary according to the dictionary type
//------------------------------------------------------------------------------
// Function call:     ListDict
//                           (          
//                              PVOID pvLangCB, 
//                              USHORT usDictType, 
//                              PUSHORT pusTermListSize, 
//                              PVOID *ppTermList, 
//                              USHORT usListType
//                            ) 
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//                    PSZ      usDictType      the dictionary type
//                    PUSHORT  pusTermListSize address of term list size var
//                    PVOID    *ppTermList     address of term list pointer
//                    USHORT   usListTyp       only MORPH_ZTERMLIST is valid
//------------------------------------------------------------------------------
// Output parameter:  PVOID      *ppvLangCB   to buffer for language CB ptr
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     call readTermFromFile to get all the word of the specified dictionary
//                    add all returned terms to term list
//                    terminate term list
//                    return function return code
//------------------------------------------------------------------------------
USHORT ListDict
(
 PVOID pvLangCB, 
 USHORT usDictType, 
 PUSHORT pusTermListSize, 
 PVOID *ppTermList, 
 USHORT usListType
 )
{
	USHORT usRC = MORPH_OK;

  usListType; 

	if (ABBREV_DICT != usDictType && ABBREV_SYSTEM_DICT != usDictType &&
        ADDENDA_DICT != usDictType)
	{
		return MORPH_NOT_FOUND;
	}

	PLANGCB tPLangCB = (PLANGCB)pvLangCB;
	if (NULL == tPLangCB)
	{
		return MORPH_INV_PARMS;
	}

  vector<wstring> tTermList;

  if ( ABBREV_DICT == usDictType )
  {
    OtmMorph* tMorphInstance = tPLangCB->m_MorphInstance;
	  if (NULL == tMorphInstance)
	  {
		  return MORPH_NOT_FOUND;
	  }

    tMorphInstance->listTerms( tTermList, true ); /* User abbrev. */
  }
  else
  if ( ABBREV_SYSTEM_DICT == usDictType )
  {
    OtmMorph* tMorphInstance = tPLangCB->m_MorphInstance;
	  if (NULL == tMorphInstance)
	  {
		  return MORPH_NOT_FOUND;
	  }

    tMorphInstance->listTerms( tTermList, false ); /* System abbrev. */
  }
  else
  {
    // firstly to get the instance, incase of null
    MorphCheckSpellInstance(tPLangCB);

	  OtmSpell* tSpellInstance = tPLangCB->m_SpellInstance;
	  if (NULL == tSpellInstance)
	  {
		  return MORPH_NOT_FOUND;
	  }

	  tSpellInstance->readTermFromFile(usDictType);
	  tSpellInstance->listTerms(tTermList, usDictType);
  }

  // get size of required buffer for the term list
  ULONG ulListSize = 10;
	for (size_t i = 0; i < tTermList.size(); i++)
	{
		ulListSize += tTermList[i].length() + 1;
	}
  ulListSize = ulListSize * sizeof(CHAR_W);

	UtlAlloc((PVOID* )ppTermList, 0, ulListSize, NOMSG);
	PSZ_W pTermList = (PSZ_W)*ppTermList;
	if (NULL == pTermList)
	{
		return MORPH_NO_MEMORY;
	}
	memset(pTermList, 0, ulListSize );
	//pTermList += sizeof(PSZ_W);
	for (size_t i = 0; i < tTermList.size(); i++)
	{
		wcscpy( pTermList, tTermList[i].c_str());
		pTermList += tTermList[i].length() + 1;
	}
	*pusTermListSize = (USHORT)ulListSize;

	return( usRC );

} /* end of function LISTDICT */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     BuildDict
//------------------------------------------------------------------------------
// Description:       Returns all the word of the dictionary according to the dictionary type
//------------------------------------------------------------------------------
// Function call:     BuildDict
//                           (          
//                              PVOID pvLangCB, 
//                              USHORT usDictType, 
//                              PUSHORT pusTermListSize, 
//                              PVOID *ppTermList, 
//                              USHORT usListType
//                            ) 
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//                    PSZ      usDictType      the dictionary type
//                    PUSHORT  pusTermListSize address of term list size var
//                    PVOID    *ppTermList     address of term list pointer
//                    USHORT   usListTyp       only MORPH_ZTERMLIST is valid
//------------------------------------------------------------------------------
// Output parameter:  PVOID      *ppvLangCB   to buffer for language CB ptr
//------------------------------------------------------------------------------
// Return code type:   USHORT
//------------------------------------------------------------------------------
// Return codes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequisites:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     call writeTermFromFile to restore all the words of the specified dictionary
//                    return function return code
//------------------------------------------------------------------------------

USHORT BuildDict
( 
 PVOID pvLangCB, 
 USHORT usDictType, 
 USHORT usTermListSize, 
 PVOID pTermList, 
 USHORT usListType
 ) 
{
	USHORT usRC = MORPH_OK;

  usListType; usTermListSize; 

	if (ADDENDA_DICT != usDictType && ABBREV_DICT != usDictType)
	{
		return MORPH_NOT_FOUND;
	}

	if (NULL == pTermList)
	{
		return MORPH_OK;
	}

	PLANGCB tPLangCB = (PLANGCB)pvLangCB;
	if (NULL == tPLangCB)
	{
		return MORPH_INV_PARMS;
	}

  if ( ABBREV_DICT == usDictType )
  {
    OtmMorph* tMorphInstance = tPLangCB->m_MorphInstance;
	  if (NULL == tMorphInstance)
	  {
		  return MORPH_NOT_FOUND;
	  }

    tMorphInstance->clearAbbreviations();

    // be caution here sizeof(PSZ_W)/sizeof(wchar_t)
	  wchar_t* tPTerm = (wchar_t*) pTermList + sizeof(PSZ_W)/sizeof(wchar_t);
	  int tLen = wcslen(tPTerm);

      if(pTermList==NULL)
          tLen = 0;

	  while (tLen > 0)
	  {
		  tMorphInstance->addAbbreviation(tPTerm);
		  tPTerm += tLen;
		  tPTerm++;
		  tLen = wcslen(tPTerm);
	  }

    tMorphInstance->saveAbbreviations();
  }
  else
  {
	  OtmSpell* tSpellInstance = tPLangCB->m_SpellInstance;
	  if (NULL == tSpellInstance)
	  {
		  return MORPH_NOT_FOUND;
	  }

	  vector<wstring> tOldTermList;
	  tSpellInstance->listTerms(tOldTermList, usDictType);
	  for (size_t i = 0; i < tOldTermList.size(); i++)
	  {
		  tSpellInstance->deleteTerm(tOldTermList[i].c_str(), usDictType);
	  }
      // be caution here sizeof(PSZ_W)/sizeof(wchar_t)
	  wchar_t* tPTerm = (wchar_t*) pTermList + sizeof(PSZ_W)/sizeof(wchar_t);
	  int tLen = wcslen(tPTerm);

      if(pTermList==NULL)
          tLen = 0;

	  while (tLen > 0)
	  {
		  tSpellInstance->addTerm(tPTerm, usDictType);
		  tPTerm += tLen;
		  tPTerm++;
		  tLen = wcslen(tPTerm);
	  }


	  if( true != tSpellInstance->writeTermToFile(usDictType))
		  usRC = MORPH_FUNC_FAILED;
  }

	return usRC;
} /* end of function BUILDDICT */

//------------------------------------------------------------------------------
//   Public functions (= module entry points)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphTokenize       Split a segment into terms
//------------------------------------------------------------------------------
// Description:       Tokenizes a string and stores all terms found in a
//                    term list.
//                    :p.
//                    Depending on the type of list requested
//                    a term list is either a series of null-terminated strings
//                    which is terminated by another null value or a list
//                    of offset and length values terminated by a null entry.
//                    See the sample for the layout of a term list.
//                    :p.
//                    If the term list pointer is NULL and the term list size
//                    is zero, the term list is allocated.
//                    The term list is enlarged if not all terms fit into
//                    the current list.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphTokenize( SHORT sLanguageID, PSZ pszInData,
//                                          PUSHORT pusBufferSize,
//                                          PVOID *ppTermList,
//                                          USHORT usListType);
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    PSZ      pszInData      pointer to input segment
//                    USHORT   usListType     type of term list
//                                              MORPH_ZTERMLIST or
//                                              MORPH_OFFSLIST
//------------------------------------------------------------------------------
// Output parameter:  PUSHORT  pusBufferSize  address of variable containing
//                                               size of term list buffer
//                    PVOID    *ppTermList    address of caller's term list
//                                               pointer
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           USHORT usListSize = 0;     // allocated list size
//                    PSZ    pszList = NULL;     // ptr to list area
//                    SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );
//                    usRC = MorphTokenize( sLangID, "Mary had a little lamb",
//                                          &usListSize, &pszList,
//                                          MORPH_ZTERMLIST );
//
//                    after this call the area pointed to by pszList would
//                    contain the following term list:
//
//                    "Mary\0had\0a\0little\0lamb\0\0"
//------------------------------------------------------------------------------
// Function flow:     check input data
//                    get language control block pointer
//                    call tokenize function of language exit
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphTokenizeW
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInData,                 // pointer to input segment
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType,                 // type of term list MORPH_ZTERMLIST or
                                       //    MORPH_OFFSLIST
   ULONG    ulOemCP                    // CP of language of sLangID!!
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  PLCB      pLCB = NULL;                      // pointer to language control block
  ULONG     ulTermBufUsed = 0;         // number of bytes used in term buffer
  ULONG     ulBufferSize = *pusBufferSize;
#ifdef MEASURE_TOKENIZE_TIME
   static int iMeasureCount = 0;
   static _int64 iTotalTime = 0;
   LARGE_INTEGER iStartTime;
   LARGE_INTEGER iEndTime;
#endif

#ifdef LOG_TOKENIZE
  LogWriter Log;
#endif

#ifdef MEASURE_TOKENIZE_TIME
  QueryPerformanceCounter( &iStartTime );
#endif

  ulOemCP; 
  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pszInData == NULL)     ||
       (pusBufferSize == NULL) ||
       (ppTermList == NULL)    ||
       ((*ppTermList == NULL) && (*pusBufferSize != 0) ) )
  {
    usRC = MORPH_INV_PARMS;
  } /* endif */

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

  /********************************************************************/
  /* call language exit to tokenize the input data                    */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    if ( *pszInData != EOS )
    {
#ifdef LOG_TOKENIZE
        Log.open( "MorphTokenizeW" );
        Log.writef( "Tokenizing \"%S\"\n", pszInData );
#endif

      usRC = TokenizeW( pLCB->pvLangCB, pszInData, &ulBufferSize, ppTermList, usListType );

#ifdef LOG_TOKENIZE
      if ( usRC == MORPH_OK )
      {
        if ( usListType == MORPH_FLAG_OFFSLIST )
        {
          PSHORT psData = (PSHORT)*ppTermList;
          SHORT sStart, sLen;
          LONG lFlag;
          do
          {
            lFlag  = *(PLONG)psData;
            psData += 2;
            sLen   = *psData++;
            sStart = *psData++;
            Log.writef( "   Start=%2.2d Len=%2.2d Flag=%8.8lX\n", sStart, sLen, lFlag );
          } while ( (sStart != 0) || (sLen != 0) || (lFlag != 0) );
        }
        else if ( usListType == MORPH_OFFSLIST )
        {
          PSHORT psData = (PSHORT)*ppTermList;
          SHORT sStart, sLen;
          do
          {
            sLen   = *psData++;
            sStart = *psData++;
            Log.writef( "Start=%2.2d Len=%2.2d\n", sStart, sLen);
          } while ( (sStart != 0) || (sLen != 0) );
        } /* endif */
      }
      else
      {
        Log.writef( "   TokenizeW failed with rc=%u\n", usRC );
      } /* endif */
      Log.close();
#endif

    }
    else
    {
      usRC = MorphAddTermToList2W( (PSZ_W *)ppTermList, &ulBufferSize, &ulTermBufUsed, L" ", 0, 0, 0L, usListType );
    } /* endif */
  } /* endif */

  *pusBufferSize = (USHORT)ulBufferSize;

#ifdef MEASURE_TOKENIZE_TIME
  QueryPerformanceCounter( &iEndTime );

  iTotalTime += (iEndTime.QuadPart - iStartTime.QuadPart);
  iMeasureCount++;
  if ( iMeasureCount == 100000 )
  {
    char szMsg[60];
    sprintf( szMsg, "Time for %ld MorphTokenizeWCalls is %I64d ticks", iMeasureCount, iTotalTime );
    MessageBox( HWND_DESKTOP, szMsg, "Info", MB_OK );
  }
#endif


  return( usRC );

} /* endof MorphTokenizeW */

USHORT MorphTokenize
(
   SHORT    sLanguageID,               // language ID
   PSZ      pszInData,                 // pointer to input segment
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType                 // type of term list MORPH_ZTERMLIST or
                                       //    MORPH_OFFSLIST
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  PLCB      pLCB = NULL;                      // pointer to language control block
  USHORT    usTermBufUsed = 0;         // number of bytes used in term buffer
#ifdef LOG_TOKENIZE
  LogWriter Log;
#endif

  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pszInData == NULL)     ||
       (pusBufferSize == NULL) ||
       (ppTermList == NULL)    ||
       ((*ppTermList == NULL) && (*pusBufferSize != 0) ) )
  {
    usRC = MORPH_INV_PARMS;
  } /* endif */

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

  /********************************************************************/
  /* call language exit to tokenize the input data                    */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    if ( *pszInData != EOS )
    {
#ifdef LOG_TOKENIZE
        Log.open( "MorphTokenize" );
        Log.writef( "Text: %s\n", pszInData );
#endif

      usRC = Tokenize( pLCB->pvLangCB,
                                   pszInData,
                                   pusBufferSize,
                                   ppTermList,
                                   usListType );

#ifdef LOG_TOKENIZE
      if ( usRC == MORPH_OK )
      {
        if ( usListType == MORPH_FLAG_OFFSLIST )
        {
          PSHORT psData = (PSHORT)*ppTermList;
          SHORT sStart, sLen, sFlag;
          do
          {
            sLen   = *psData++;
            sStart = *psData++;
            sFlag  = *psData++;
            Log.writef( "Start=%2.2d Len=%2.2d Flag=%4.4X\n", sStart, sLen, sFlag );
          } while ( (sStart != 0) || (sLen != 0) || (sFlag != 0) );
        }
        else if ( usListType == MORPH_OFFSLIST )
        {
          PSHORT psData = (PSHORT)*ppTermList;
          SHORT sStart, sLen;
          do
          {
            sLen   = *psData++;
            sStart = *psData++;
            Log.writef( "Start=%2.2d Len=%2.2d\n", sStart, sLen);
          } while ( (sStart != 0) || (sLen != 0) );
        } /* endif */
      }
      else
      {
        Log.writef( "Tokenize failed with rc=%u\n", usRC );
      } /* endif */
      Log.close();
#endif

    }
    else
    {
      usRC = MorphAddTermToList( (PSZ *)ppTermList,
                                 pusBufferSize,
                                 &usTermBufUsed,
                                 (PSZ)EMPTY_STRING,
                                 0,
                                 0,
                                 0L,
                                 usListType );
    } /* endif */
  } /* endif */

  return( usRC );

} /* endof MorphTokenize */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphExtractKeywords       Extract keywords
//------------------------------------------------------------------------------
// Description:       Extract all maybe keywords from the input data and
//                    returns them in form of a term list.
//                    :p.
//                    Depending on the type of list requested
//                    a term list is either a series of null-terminated strings
//                    which is terminated by another null value or a list
//                    of offset and length values terminated by a null entry.
//                    See the sample for the layout of a term list.
//                    :p.
//                    If the term list pointer is NULL and the term list size
//                    is zero, the term list is allocated.
//                    The term list is enlarged if not all terms fit into
//                    the current list.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphExtractKeywords( SHORT sLanguageID,
//                                                 PSZ pszInData,
//                                                 PUSHORT pusBufferSize,
//                                                 PVOID *ppTermList,
//                                                 USHORT usListType);
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    PSZ      pszInData      pointer to input segment
//                    USHORT   usListType     type of term list
//                                              MORPH_ZTERMLIST or
//                                              MORPH_OFFSLIST
//------------------------------------------------------------------------------
// Output parameter:  PUSHORT  pusBufferSize  address of variable containing
//                                               size of term list buffer
//                    PVOID    *ppTermList    address of caller's term list
//                                               pointer
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           USHORT usListSize = 0;     // allocated list size
//                    PSZ    pszList = NULL;     // ptr to list area
//                    SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );
//                    usRC = MorphExtractKeywords( sLangID,
//                                                 "Mary had a little lamb",
//                                                 &usListSize, &pszList,
//                                                 MORPH_ZTERMLIST );
//
//                    after this call the area pointed to by pszList would
//                    contain the following term list:
//
//                    "Mary\0lamb\0\0"
//------------------------------------------------------------------------------
// Function flow:     check input data
//                    get language control block pointer
//                    tokenize input data
//                    set output position to begin of term list
//                    loop over all terms in term list
//                       get part-of-speech data for current term
//                       if no POS info is available or POS is noun then
//                         copy term to current output position in list
//                       endif
//                    endloop
//                    terminate term list at current output position
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphExtractKeywords
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInData,                 // pointer to input segment
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType,                 // type of term list MORPH_ZTERMLIST or
                                          //    MORPH_OFFSLIST
   ULONG     ulOemCP
)
{
  USHORT      usRC = MORPH_OK;         // function return code
  PLCB        pLCB = NULL;             // pointer to language control block
  /*-- variables for offset/length type term lists  ------------------*/
  PLISTTERM   pTerm;                   // current processing position in list
  PLISTTERM   pOutTerm;                // current output position in list
  /*-- variables for zero-terminated strings type term lists ---------*/
  PSZ_W       pTermPos;                // current processing position in list
  PSZ_W       pOutPos;                 // current output position in list
  /*-- variables for the processing of a single term in the list -----*/
  PSZ_W       pszTerm = NULL;          // ptr to start of term
  ULONG       ulLen = 0;               // length of term
  ULONG       ulPOSInfo;               // buffer for POS info
  CHAR_W      chTemp;                  // buffer to store term delimiter

  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pszInData == NULL)     ||
       (pusBufferSize == NULL) ||
       (ppTermList == NULL)    ||
       ((*ppTermList == NULL) && (*pusBufferSize != 0) ) )
  {
    usRC = MORPH_INV_PARMS;
  } /* endif */

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

  /********************************************************************/
  /* tokenize the input data                                          */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphTokenizeW( sLanguageID, pszInData, pusBufferSize, ppTermList,
                          usListType, ulOemCP );
  } /* endif */

  /********************************************************************/
  /* remove all terms which are not keywords from the list            */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    /******************************************************************/
    /* start at begin of term list                                    */
    /******************************************************************/
    pOutTerm =  pTerm    = (PLISTTERM)*ppTermList;
    pOutPos  =  pTermPos = (PSZ_W)*ppTermList;

    /******************************************************************/
    /* process all term in list                                       */
    /******************************************************************/
    while ( (usRC == MORPH_OK) &&
            (((usListType == MORPH_OFFSLIST) && (pTerm->ul != 0L)) ||
             ((usListType == MORPH_ZTERMLIST) && *pTermPos) ) )
    {
       /***************************************************************/
       /* get values of current term                                  */
       /***************************************************************/
       switch ( usListType )
       {
         case MORPH_ZTERMLIST :
           ulLen    = UTF16strlenCHAR(pTermPos);
           pszTerm  = pTermPos;
           break;

         case MORPH_OFFSLIST :
           ulLen    = pTerm->OffsLen.usLength;
           pszTerm  = pszInData + pTerm->OffsLen.usOffs;
           break;
       } /* endswitch */

       /***************************************************************/
       /* terminate current term                                      */
       /***************************************************************/
       chTemp = pszTerm[ulLen];
       pszTerm[ulLen] = EOS;

       /***************************************************************/
       /* get part-of-speech info for term                            */
       /***************************************************************/
       usRC = MorphGetPOSInfo( sLanguageID, pszTerm, &ulPOSInfo, ulOemCP );

       /***************************************************************/
       /* restore term end character                                  */
       /***************************************************************/
       pszTerm[ulLen] = chTemp;

       /***************************************************************/
       /* copy term to current output position in term list if term   */
       /* might be a keyword                                          */
       /***************************************************************/
       if ( (usRC == MORPH_NOT_FOUND) ||
            ((usRC == MORPH_OK) && (ulPOSInfo & MORPH_NOUN)) )
       {
         usRC = MORPH_OK;              // reset return code

         switch ( usListType )
         {
           case MORPH_ZTERMLIST :
             while ( (*pOutPos++ = *pszTerm++) != NULC );
             break;

           case MORPH_OFFSLIST :
             pOutTerm->ul = pTerm->ul;
             pOutTerm++;
             break;
         } /* endswitch */
       } /* endif */

       /***************************************************************/
       /* continue with next term                                     */
       /***************************************************************/
       switch ( usListType )
       {
         case MORPH_ZTERMLIST :
           pTermPos += ulLen + 1;
           break;

         case MORPH_OFFSLIST :
           pTerm++;
           break;
       } /* endswitch */
    } /* endwhile */


    /******************************************************************/
    /* terminate term list at current output position                 */
    /******************************************************************/
    switch ( usListType )
    {
      case MORPH_ZTERMLIST :
        *pOutPos = EOS;
        break;

      case MORPH_OFFSLIST :
        pOutTerm->ul = 0L;
        break;
    } /* endswitch */

  } /* endif */

  return( usRC );

} /* endof MorphExtractKeywords */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphVerify
//------------------------------------------------------------------------------
// Description:       Does a spell verify on all terms in the input segment.
//                    :p.
//                    The misspelled words are returned in form of a term list.
//                    :p.
//                    A term list is either a series of null-terminated strings
//                    which is terminated by another null value.
//                    :p.
//                    See the sample for the layout of a term list.
//                    :p.
//                    If the term list pointer is NULL and the term list size
//                    is zero, the term list is allocated.
//                    The term list is enlarged if not all terms fit into
//                    the current list.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphVerify( SHORT sLanguageID,
//                                        PSZ   pszInData,
//                                        PUSHORT pusBufferSize,
//                                        PSZ *ppTermList );
//                                        USHORT usListType);
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    PSZ      pszInData      pointer to input segment
//                    USHORT   usListType     type of term list
//                                              MORPH_ZTERMLIST or
//                                              MORPH_OFFSLIST
//------------------------------------------------------------------------------
// Output parameter:  PUSHORT  pusBufferSize  address of variable containing
//                                               size of term list buffer
//                    PVOID    *ppTermList    address of caller's term list
//                                               pointer
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           USHORT usListSize = 0;     // allocated list size
//                    PSZ    pszList = NULL;     // ptr to list area
//                    SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );
//
//                    usRC = MorphVerify( sLangID, "This is a dictianary",
//                                          &usListSize, &pszList,
//                                          MORPH_ZTERMLIST );
//
//                    after this call the area pointed to by pszList would
//                    contain the following term list:
//
//                    "dictianary\0\0"
//------------------------------------------------------------------------------
// Function flow:     Check input data
//                    Get language control block pointer
//                    activate addenda dictionary if not active yet
//                    tokenize the input segment using MorphTokenize
//                    loop over all terms
//                       call verify function of language exit
//                       if term was not found
//                          add term to term list
//                       endif
//                    endloop
//                    terminate term list
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphVerify
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInData,                 // pointer to input segment
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType,                // type of term list MORPH_ZTERMLIST or
                                       //    MORPH_OFFSLIST
   ULONG     ulOemCP
)
{
   USHORT    usRC = MORPH_OK;          // function return code
   PLCB       pLCB = NULL;             // pointer to language control block
   USHORT    usListLen = 0;            // length of list in bytes
   PULONG    pTermList = NULL;         // pointer to offset/length term list
   CHAR_W    chTemp;                   // buffer for character values
   PLISTTERM pTerm;

   USHORT    usTermOffs;               // offset of currently processed term
   USHORT    usTermLength;             // length of currently processed term
   PSZ_W     pucTerm;                  // ptr to currently processed term
   ULONG     ulTermBufUsed = 0;        // space used in caller's term list buffer
   ULONG     ulBufferSize = 0;
   PSZ_W     pucNormSeg = NULL;        // buffer for preprocessed segment
   ULONG     ulSegLen;                 // data length of input segment
   PSZ_W     pszSource, pszTarget;     // pointer for segment preprocessing

  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pszInData == NULL)     ||
       (pusBufferSize == NULL) ||
       (ppTermList == NULL)    ||
       ((*ppTermList == NULL) && (*pusBufferSize != 0) ) )
  {
    usRC = MORPH_INV_PARMS;
  }
  else
  {
  ulBufferSize  = (ULONG)*pusBufferSize;
  } /* endif */

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

                                                               /* 38@KIT1167A */
  /********************************************************************/
  /* Allocate buffer for copy of input segment                        */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    ulSegLen = UTF16strlenCHAR( pszInData ) + 1;
    if ( !UtlAlloc( (PVOID *)&pucNormSeg,
                    0L,
                    (LONG)max( MIN_ALLOC, ulSegLen * sizeof(CHAR_W) ),
                    NOMSG ) )
    {
       usRC = MORPH_NO_MEMORY;        // set short on memory RC
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Copy input segment and replace dashes by blanks                  */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    pszSource = pszInData;
    pszTarget = pucNormSeg;
    while ( *pszSource )
    {
      if ( *pszSource == '-' )
      {
        *pszTarget++ = BLANK;
        *pszSource++;
      }
      else
      {
        *pszTarget++ = *pszSource++;
      } /* endif */
    } /* endwhile */
    *pszTarget = EOS;
  } /* endif */

  // use textarea based spellchecker if available else tokenize the text block first
  if ( usRC == MORPH_OK )
  {
    /********************************************************************/
    /* tokenize the preprocessed segment                                */
    /********************************************************************/
    if ( usRC == MORPH_OK )
    {
      usRC = MorphTokenizeW( sLanguageID, pucNormSeg, &usListLen,  /* 1@KIT1167C */
                            (PVOID *)&pTermList,
                            MORPH_OFFSLIST, ulOemCP );
    } /* endif */

    /********************************************************************/
    /* perform spell verify on all terms in term list                   */
    /********************************************************************/
    if ( usRC == MORPH_OK )
    {
      pTerm = (PLISTTERM)pTermList;
      while ( (usRC == MORPH_OK) && (pTerm->ul != 0L) )
      {
          // get values of current term (for easier processing)
          usTermOffs   = pTerm->OffsLen.usOffs;
          usTermLength = pTerm->OffsLen.usLength;
          pucTerm = pszInData + usTermOffs;

          // terminate current term
          chTemp = pucTerm[usTermLength];
          pucTerm[usTermLength] = EOS;

          /**************************************************************/
          /* ignore terms consisting of a letter and an apostroph       */
          /* (e.g. d', c')                                              */
          /**************************************************************/
          if ( (usTermLength == 2) &&
              iswalpha(pucTerm[0]) &&
              (pucTerm[1] == L'\'') )
          {
            /************************************************************/
            /* do not verify spelling                                   */
            /************************************************************/
            usRC = MORPH_OK;
          }
          else
          {
            /************************************************************/
            /* Verify spelling                                          */
            /************************************************************/
            usRC = Verify( pLCB->pvLangCB, pucTerm );
          } /* endif */

          if ( usRC == MORPH_FUNC_NOT_SUPPORTED )
          {
            /************************************************************/
            /* Verify is not supported by language exit, treat any      */
            /* term as valid                                            */
            /************************************************************/
            usRC = MORPH_OK;
          } /* endif */

          if ( usRC == MORPH_NOT_FOUND )
          {
            /***************************************************/
            /* add term to term list                           */
            /* parm 2,3,5 are in # of w's                      */
            /***************************************************/
            usRC = MorphAddTermToList2W( (PSZ_W *)ppTermList, &ulBufferSize, &ulTermBufUsed,
                                        pucTerm, usTermLength,
                                        pTerm->OffsLen.usOffs,
                                        0L,
                                        usListType);
            usRC = MORPH_OK;
          } /* endif */

          // restore end character of current term
          pucTerm[usTermLength] = chTemp;

        /***************************************************************/
        /* continue with next term                                     */
        /***************************************************************/
        pTerm++;
      } /* endwhile terms in list */

      /***************************************************/
      /* terminate term list                             */
      /***************************************************/
      usRC = MorphAddTermToList2W(  (PSZ_W *)ppTermList,
                                    &ulBufferSize, &ulTermBufUsed,
                                    (PSZ_W)NULL,
                                    0, 0, 0L, MORPH_OFFSLIST );

      if (usRC == MORPH_OK)
      {
        if (ulBufferSize * sizeof(CHAR_W) < 0xFFF0)
        {
          *pusBufferSize = (USHORT) ulBufferSize;
        }
        else
        {
      usRC = MORPH_NO_MEMORY;
      }
    }
    } /* endif */
  } /* endif */


  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( pucNormSeg )   UtlAlloc( (PVOID *) &pucNormSeg, 0L, 0L, NOMSG );

  return( usRC );
} /* end of function MorphVerify */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphSpellAid
//------------------------------------------------------------------------------
// Description:       Returns all candidates for the correct spelling of a
//                    mispelled word. The candidates are returned in form
//                    of a term list.
//                    :p.
//                    A term list is a series of null-terminated strings
//                    which is terminated by another null value.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphSpellAid( SHORT sLanguageID,
//                                        PSZ   pszTerm,
//                                        PUSHORT pusBufferSize,
//                                        PSZ *ppTermList );
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    PSZ      pszTerm        pointer to input term
//------------------------------------------------------------------------------
// Output parameter:  PUSHORT  pusBufferSize  address of variable containing
//                                               size of term list buffer
//                    PVOID    *ppTermList    address of caller's term list
//                                               pointer
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           USHORT usListSize = 0;     // allocated list size
//                    PSZ    pszList = NULL;     // ptr to list area
//                    SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );
//
//                    usRC = MorphSpellAid( sLangID, "dictianary",
//                                          &usListSize, &pszList );
//
//                    after this call the area pointed to by pszList would
//                    contain the following term list:
//
//                    "dictionary\0\0"
//------------------------------------------------------------------------------
// Function flow:     check input data
//                    get language control block pointer
//                    activate addenda dictionary if not active yet
//                    get proposals for mispelled word using language exit
//                    add returned terms to caller's term list
//                    terminate term list
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphSpellAid
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszTerm,                   // pointer to input term
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermListW,               // address of caller's term list pointer
   ULONG    ulOemCP
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  PLCB       pLCB = NULL;              // pointer to language control block

#ifdef ACTIVATE_LOGGING
  FILE      *hfLog = NULL;
#endif

  ulOemCP;

  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pszTerm       == NULL)    ||
       (pusBufferSize == NULL)    ||
       (ppTermListW   == NULL)    ||
       ((*ppTermListW == NULL) && (*pusBufferSize != 0) ) )
  {
    usRC = MORPH_INV_PARMS;
  } /* endif */

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

  /********************************************************************/
  /* get proposals for mispelled word                                 */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = SpellAid( pLCB->pvLangCB, pszTerm, pusBufferSize, ppTermListW, MORPH_ZTERMLIST );
  } /* endif */

#ifdef ACTIVATE_LOGGING
  if ( szMLogFile[0] == EOS )
  {
    UtlMakeEQFPath( szMLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szMLogFile, 0L, FALSE );
    strcat( szMLogFile, "\\MORPH.LOG" );
  } /* endif */

  hfLog = fopen( szMLogFile, "a" );
  if ( hfLog )
  {
    fprintf( hfLog, "MorphSpellAid, checking term \"%S\", rc=%u\n", pszTerm, usRC );
    fclose( hfLog );
  } /* endif */
#endif

  return( usRC );

} /* end of function MorphSpellAid */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphAddToAddenda
//------------------------------------------------------------------------------
// Description:       Adds the given term to the addenda dictionary.
//                    :p.
//                    The dictionary is the addenda dictionary specified in the
//                    PROPERTY.LNG file.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphAddToAddenda( SHORT sLanguageID,
//                                              PSZ   pszTerm );
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    PSZ      pszTerm        pointer to term being added
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );
//
//                    usRC = MorphAddToAddenda( sLangID, "new" );
//
//------------------------------------------------------------------------------
// Function flow:     Check input data
//                    Get language control block pointer
//                    activate addenda dictionary if not active yet
//                    add term to addenda dictionary using language exit
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphAddToAddenda
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszTerm,                    // pointer to term being added
   ULONG    ulOemCP
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  PLCB       pLCB= NULL;               // pointer to language control block

  ulOemCP;

  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( pszTerm == NULL)
  {
    usRC = MORPH_INV_PARMS;
  } /* endif */

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

  /********************************************************************/
  /* add term to addenda dictionary                                   */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = TermAdd( pLCB->pvLangCB, pszTerm );

    if ( usRC == MORPH_OK )
    {
      /************************************************************/
      /* Force a write of the modified dictionary by closing and  */
      /* re-open of the dict which is performed by the 'refresh'  */
      /* mode of the BUILDDICT API                                */
      /************************************************************/
      usRC = BuildDict( pLCB->pvLangCB, ADDENDA_DICT, 0,
            NULL, 0 );
    }
    else if ( usRC == MORPH_FUNC_NOT_SUPPORTED )
    {
      /************************************************************/
      /* TermAdd is not supported, ignore term                    */
      /************************************************************/
      usRC = MORPH_OK;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Update/Refresh any other instance of abbreviation/addenda        */
  /* dictionary                                                       */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphRefreshDicts( pLCB, ADDENDA_DICT );
  } /* endif */
  return( usRC );

} /* end of function MorphAddToAddenda */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphAddToAbbrev
//------------------------------------------------------------------------------
// Description:       Adds the given term to the abbrev  dictionary.
//                    The dictionary was opened during the INIT (if ANY)
//------------------------------------------------------------------------------
// Function call:     usRC = MorphAddToAbbrev ( SHORT sLanguageID,
//                                              PSZ   pszTerm );
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    PSZ      pszTerm        pointer to term being added
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );
//
//                    usRC = MorphAddToAbbrev( sLangID, "U.S." );
//------------------------------------------------------------------------------
// Function flow:     Check input data
//                    Get language control block pointer
//                    activate addenda dictionary if not active yet
//                    add term to addenda dictionary using language exit
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphAddToAbbrev
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszTerm,                    // pointer to term being added
   ULONG    ulOemCP
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  PLCB       pLCB = NULL;              // pointer to language control block

  ulOemCP;

  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( pszTerm == NULL)
  {
    usRC = MORPH_INV_PARMS;
  } /* endif */

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

  /********************************************************************/
  /* add term to abbrev. dictionary                                   */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {    
    usRC = AbbrevAdd( pLCB->pvLangCB, pszTerm );

    if ( usRC == MORPH_OK )
    {
      /************************************************************/
      /* Force a write of the modified dictionary by closing and  */
      /* re-open of the dict which is performed by the 'refresh'  */
      /* mode of the BUILDDICT API                                */
      /************************************************************/
      usRC = BuildDict( pLCB->pvLangCB, ABBREV_DICT, 0,
                                    NULL, 0 );
    }
    else if ( usRC == MORPH_FUNC_NOT_SUPPORTED )
    {
      /************************************************************/
      /* AbbrevAdd is not supported, ignore term                  */
      /************************************************************/
      usRC = MORPH_OK;
    } /* endif */

  } /* endif */

  /********************************************************************/
  /* Update/Refresh any other instance of abbreviation/addenda        */
  /* dictionary                                                       */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphRefreshDicts( pLCB, ABBREV_DICT );
  } /* endif */

  return( usRC );

} /* end of function MorphAddToAbbrev */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphListDict
//------------------------------------------------------------------------------
// Description:       Lists the terms of an addenda or abbreviation dictionary.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphListDict ( sLanguageID, usDictType, &usSize,
//                                           &pTermList, MORPH_ZTERMLIST );
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    USHORT   usDictType      type of dictionary:
//                                             ADDENDA_DICT or
//                                             ABBREV_DICT
//                    PUSHORT  pusTermListSize address of term list size var
//                    PVOID    *ppTermList     address of term list pointer
//                    USHORT   usListTyp       type of term list:
//                                             MORPH_ZTERMLIST
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
USHORT MorphListDict
(
   SHORT    sLanguageID,               // language ID
   USHORT   usDictType,                // IN : type of dictionary
                                       //      ADDENDA_DICT or
                                       //      ABBREV_DICT
   PUSHORT  pusTermListSize,           // IN/OUT:  address of variable
                                       //    containing size of term list buffer
   PVOID    *ppTermList,               // IN/OUT: address of term list pointer
   USHORT   usListType                 // IN: type of term list :
                                       //     MORPH_ZTERMLIST
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  PLCB       pLCB = NULL;              // pointer to language control block

  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

  /********************************************************************/
  /* call language exit to list dictionary terms                      */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = ListDict( pLCB->pvLangCB, usDictType,
                                 pusTermListSize, ppTermList,
                                 usListType );
  } /* endif */

  return( usRC );

} /* end of function MorphListDict */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphBuildDict
//------------------------------------------------------------------------------
// Description:       Create an addenda or abbreviation dictionary from a term
//                    list.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphBuildDict ( sLanguageID, usDictType, usSize,
//                                            pTermList, MORPH_ZTERMLIST );
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    USHORT   usDictType      type of dictionary:
//                                             ADDENDA_DICT or
//                                             ABBREV_DICT
//                    USHORT   usTermListSize  term list size
//                    PVOID    pTermList       term list pointer
//                    USHORT   usListTyp       type of term list:
//                                             MORPH_ZTERMLIST or
//                                             MORPH_LARGE_ZTERMLIST
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
USHORT MorphBuildDict
(
   SHORT    sLanguageID,               // language ID
   USHORT   usDictType,                // IN : type of dictionary
                                       //      ADDENDA_DICT or
                                       //      ABBREV_DICT
   ULONG    ulTermListSize,            // IN : size of term list buffer
   PVOID    pTermList,                 // IN : address of term list pointer
   USHORT   usListType                 // IN : type of term list :
                                       //      MORPH_ZTERMLIST or
                                       //      MORPH_LARGE_ZTERMLIST
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  PLCB       pLCB= NULL;               // pointer to language control block

  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

  /********************************************************************/
  /* call language exit to list dictionary terms                      */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    // don't care if ulTermListSize is truncated, this parameter
    // is not used by the BuildDict function (end of term list
    // is triggered by an empty list entry)
    USHORT usTermListSize = (USHORT)ulTermListSize;
    usRC = BuildDict( pLCB->pvLangCB, usDictType,
                                  usTermListSize, pTermList,
                                  usListType );
  } /* endif */

  /********************************************************************/
  /* Update/Refresh any other instance of abbreviation/addenda        */
  /* dictionary                                                       */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphRefreshDicts( pLCB, usDictType );
  } /* endif */

  return( usRC );

} /* end of function MorphBuildDict */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphGetStemForm
//------------------------------------------------------------------------------
// Description:       Returns all stem (lemma) forms for a given term in form
//                    of a term list.
//                    :p.
//                    A term list is a series of null-terminated strings
//                    which is terminated by another null value.
//                    See the sample for the layout of a term list.
//                    :p.
//                    If the term list pointer is NULL and the term list size
//                    is zero, the term list is allocated.
//                    The term list is enlarged if not all terms fit into
//                    the current list.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphGetStemForm( SHORT sLanguageID,
//                                             PSZ pszInTerm,
//                                             PUSHORT pusBufferSize,
//                                             PSZ *ppTermList );
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    PSZ      pszInTerm      pointer to input term
//------------------------------------------------------------------------------
// Output parameter:  PUSHORT  pusBufferSize  address of variable containing
//                                               size of term list buffer
//                    PSZ      *ppTermList    address of caller's term list
//                                               pointer
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           USHORT usListSize = 0;     // allocated list size
//                    PSZ    pszList = NULL;     // ptr to list area
//                    SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );
//
//                    usRC = MorphGetStemForm( sLangID, "wound",
//                                             &usListSize, &pszList );
//
//                    after this call the area pointed to by pszList would
//                    contain the following term list:
//
//                    "wound\0wind\0\0"
//------------------------------------------------------------------------------
// Function flow:     get language control block pointer
//                    search term in no-lookup hash table
//                    if term not found search term in stem form cache
//                    if term not found get stem form using LEMMA function of
//                      language exit
//                    if term was found
//                      add term and term list to stem form cache
//                    else if term was not found
//                      add term to no-lookup hash table
//                    endif
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphGetStemForm
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInTermW,                // pointer to input term
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PSZ_W    *ppTermListW,               // address of caller's term list pointer
   ULONG    ulOemCP
)
{
  USHORT     usRC = MORPH_OK;          // return code of function
  PLCB       pLCB = NULL;              // pointer to language control block
  ULONG      ulLemmaListLength;        // length of lemma list
  ULONG      ulLemmaLength;            // length of a single lemma
  PSZ_W      pszLemmaList;             // pointer to list of lemmas for a term
  USHORT     usNewSize;                // new size for buffers
  PSZ_W      pszLemma;                 // pointer to lemma form of term
  PSZ_W      pNext;                    // ptr to next part in compound list
  PSZ_W      pCompound;                // ptr to current compound
  PSZ_W      pDecomposition;           // ptr to current decomposition
  enum
  {
    TERM_IN_ATOM_TABLE,
    TERM_IN_CACHE,
    TERM_FOUND,
    TERM_NOT_FOUND
  } TermState = TERM_NOT_FOUND;



  /********************************************************************/
  /* Check Term length                                                */
  /********************************************************************/

  if (UTF16strlenCHAR(pszInTermW)<=1)
  {

     usRC = MORPH_NOT_FOUND;

  } /* end if */


  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/

  if (usRC == MORPH_OK)
  {

     usRC = MorphGetLCB( sLanguageID, &pLCB );

  }/* end if */


  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if term has been looked-up by a previous call and no stem  */
  /* form was found                                                   */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    #if defined(MEASURETIME)
       INITTIME( pLCB );
       pLCB->ulStemRequests++;
    #endif

    if ( !pLCB->fNoDoNotLookup )
    {
      #if defined(MEASURETIME)
         pLCB->ulAtomRequests++;
      #endif

      if ( MorphHashSearch( pLCB->HashNoLookup, pszInTermW, NULL, NULL ) )
      {
        #if defined(MEASURETIME)
           pLCB->ulAtomFound++;
        #endif
        TermState = TERM_IN_ATOM_TABLE;
      } /* endif */

      #if defined(MEASURETIME)
         GETTIME( pLCB, pLCB->ulStemAtomTime );
      #endif

    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Lookup term in stem form cache                                   */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (TermState == TERM_NOT_FOUND) )
  {
    #if defined(MEASURETIME)
       GETTIME( pLCB, pLCB->ulStemOtherTime );
    #endif

    if ( !pLCB->fNoCaching )
    {
      #if defined(MEASURETIME)
         pLCB->ulCacheRequests++;
      #endif

      if ( MorphHashSearch( pLCB->HashCache, pszInTermW, &ulLemmaListLength, &pszLemmaList ) )
      {
        TermState = TERM_IN_CACHE;
        #if defined(MEASURETIME)
           pLCB->ulCacheFound++;
        #endif

        // Add lemmas to caller's term list
        if ( ppTermListW )              // if term list pointer is supplied ...
        {
          ulLemmaListLength = 0;
          pszLemma= pszLemmaList;
          while ( *pszLemma != EOS )
          {
            ulLemmaLength = UTF16strlenCHAR(pszLemma) + 1;
            ulLemmaListLength += ulLemmaLength;
            pszLemma += ulLemmaLength;
          } /* endwhile */
          ulLemmaListLength += 1;                          // for list end delimiter

          if ( *pusBufferSize < (ulLemmaListLength * sizeof(CHAR_W)) )
          {
            ULONG ulOldLen = *pusBufferSize;
            ULONG ulNewLen = (ulLemmaListLength * sizeof(CHAR_W));
            // allocate a larger area when allocating memory for the first time avoid too much re-allocations
            if ( (ulOldLen == 0) && (ulNewLen < 100) )
            {
              ulNewLen = 100;
            }
            if ( UtlAlloc( (PVOID *)ppTermListW, ulOldLen, ulNewLen, NOMSG ) )
            {
               *pusBufferSize = (USHORT)ulNewLen;
            }
            else
            {
               usRC = MORPH_NO_MEMORY;
            } /* endif */
          } /* endif */

          if ( usRC == MORPH_OK )
          {
            memcpy( (PVOID)*ppTermListW, pszLemmaList, ulLemmaListLength*sizeof(CHAR_W) );
          } /* endif */
        } /* endif */
      } /* endif */
      #if defined(MEASURETIME)
         GETTIME( pLCB, pLCB->ulStemCacheTime );
      #endif
    } /* endif */

  } /* endif */

  /********************************************************************/
  /* If not found, get stem form using language exit                  */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (TermState == TERM_NOT_FOUND) )
  {
    #if defined(MEASURETIME)
       pLCB->ulNlpLookups++;
    #endif

    usRC = Lemma( pLCB->pvLangCB, pszInTermW, pusBufferSize, (PVOID *)ppTermListW, MORPH_ZTERMLIST );

    if ( usRC == MORPH_FUNC_NOT_SUPPORTED )
    {
      /************************************************************/
      /* Lemma is not supported, treat as no lemma had been found */
      /************************************************************/
      usRC = MORPH_NOT_FOUND;
    } /* endif */

    #if defined(MEASURETIME)
       GETTIME( pLCB, pLCB->ulStemNlpTime );
    #endif

    TermState = ( usRC == MORPH_OK ) ? TERM_FOUND : TERM_NOT_FOUND;

#if defined(MORPH_BUILD_COMP_STEMS)
    if ( (TermState == TERM_NOT_FOUND) && pLCB->fCompSep )
    {
      /****************************************************************/
      /* Try to split term in compounds and build stem form of last   */
      /* compound                                                     */
      /****************************************************************/
      usRC = MorphCompIsolation( sLanguageID, pszInTermW,
                                 &(pLCB->usCompListSize),
                                 (PVOID *)&(pLCB->pCompList),
                                 ulOemCP);

      /****************************************************************/
      /* Go to last compound of last decomposition of term            */
      /****************************************************************/
      if ( usRC == MORPH_OK )
      {
        pNext = pLCB->pCompList;
        pCompound      = NULL;
        pDecomposition = NULL;
        while ( *pNext != EOS )
        {
          pDecomposition = pNext;
          while ( *pNext != EOS )
          {
            pCompound = pNext;
            pNext += UTF16strlenCHAR(pNext) + 1;
          } /* endwhile */
          pNext++;
        } /* endwhile */

        if ( pCompound && pDecomposition )
        {
          PVOID pTermList = NULL;
          USHORT usBufferSize = *pusBufferSize;
          UtlAlloc( &pTermList, 0L, (LONG) usBufferSize, NOMSG );

          if (pTermList)
          {
            usRC = Lemma( pLCB->pvLangCB, pCompound, &usBufferSize, (PVOID *)&pTermList, MORPH_ZTERMLIST );
            if ( usRC == MORPH_FUNC_NOT_SUPPORTED )
            {
              /************************************************************/
              /* Lemma is not supported, treat as no lemma had been found */
              /************************************************************/
              usRC = MORPH_NOT_FOUND;
            }
            else if ( usRC == MORPH_OK )
            {
              // copy first stem term to our buffer
              wcscpy( (PSZ_W)pLCB->szBuffer, (PSZ_W)pTermList );

              // Enlarge caller's term list buffer if necessary (computed size will be too large in most cases)       
              usNewSize = (USHORT)max( (UTF16strlenCHAR(pszInTermW) + UTF16strlenCHAR((PSZ_W)pLCB->szBuffer) + 1)*sizeof(CHAR_W), MIN_ALLOC );
              if ( *pusBufferSize < usNewSize )
              {
                if ( UtlAlloc( (PVOID *)ppTermListW, (LONG) *pusBufferSize, (LONG) usNewSize, NOMSG ) )
                {
                  *pusBufferSize = usNewSize;
                }
                else
                {
                  usRC = MORPH_NO_MEMORY;
                } /* endif */
              } /* endif */

              /**********************************************************/
              /* Combine compounds and stem from reduced compound to    */
              /* stem form term                                         */
              /**********************************************************/
              if ( usRC == MORPH_OK )
              {
                (*ppTermListW)[0] = EOS;
                while ( pDecomposition < pCompound )
                {
                  UTF16strcat( *ppTermListW, pDecomposition );
                  pDecomposition += UTF16strlenCHAR(pDecomposition) + 1;
                } /* endwhile */
                UTF16strcat( *ppTermListW, (PSZ_W)pLCB->szBuffer );
                (*ppTermListW)[UTF16strlenCHAR(*ppTermListW)+1] = EOS;    // term list end delimiter

                /********************************************************/
                /* Term will now be in mixed case, so lowercase the     */
                /* whole term except the first character                */
                /********************************************************/
                UtlLowerW( (PSZ_W)*ppTermListW + 1 );
                TermState = TERM_FOUND;
              } /* endif */
            } /* endif */
            UtlAlloc( &pTermList, 0L, 0L, NOMSG );
      }
        }
        else
        {
          usRC = MORPH_NOT_FOUND;
        } /* endif */
      } /* endif */

    } /* endif */
#endif

    if ( TermState == TERM_FOUND )
    {
      // Add term and term stem forms to stem form cache
      #if defined(MEASURETIME)
         GETTIME( pLCB, pLCB->ulStemOtherTime );
      #endif

      if ( !pLCB->fNoCaching )
      {
        /************************************************************/
        /* Get length of stem form list                             */
        /************************************************************/
        ulLemmaListLength = 0;
        pszLemma = *ppTermListW;
        while ( *pszLemma != EOS )
        {
          ulLemmaLength = UTF16strlenCHAR(pszLemma) + 1;
          ulLemmaListLength += ulLemmaLength;
          pszLemma += ulLemmaLength;
        } /* endwhile */
        ulLemmaListLength += 1;                // for list end delimiter

        /************************************************************/
        /* Add term and it's stem form list to cache                */
        /************************************************************/
        MorphHashAdd( pLCB->HashCache, pszInTermW,
                      ulLemmaListLength, (PSZ_W)*ppTermListW );
      } /* endif */

      #if defined(MEASURETIME)
         pLCB->ulCacheTerms++;
         GETTIME( pLCB, pLCB->ulStemCacheTime );
      #endif
    }
    else
    {
      // term was not found, add term to atom table
      #if defined(MEASURETIME)
         GETTIME( pLCB, pLCB->ulStemOtherTime );
      #endif
      if ( !pLCB->fNoDoNotLookup )
      {
        MorphHashAdd( pLCB->HashNoLookup, pszInTermW, 0, NULL );
        #if defined(MEASURETIME)
           GETTIME( pLCB, pLCB->ulStemAtomTime );
        #endif
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Return our return code to the calling function                   */
  /********************************************************************/
  #if defined(MEASURETIME)
     if ( TermState != TERM_NOT_FOUND )
     {
       pLCB->ulStemFound++;
     } /* endif */
     GETTIME( pLCB, pLCB->ulStemOtherTime );
  #endif

  if ( !usRC && ((TermState == TERM_NOT_FOUND) || (TermState == TERM_IN_ATOM_TABLE) ) )
  {
    usRC = MORPH_NOT_FOUND;
  } /* endif */

  // special handling for empty (=incorrect) entries in our hash table or from the stem form reduction function
  if ( !usRC && (*ppTermListW != NULL) && (**ppTermListW == EOS) )
  {
    usRC = MORPH_NOT_FOUND;
  } /* endif */

  return( usRC );

} /* end of function MorphGetStemForm */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphGetPOSInfo
//------------------------------------------------------------------------------
// Description:       Returns all available part-of-speech (POS) info for a
//                    given term.
//                    :p.
//                    As there may more than one POS form of the term, the
//                    POS flags are ORed together (e.g MORPH_VERB | MORPH_NOUN)
//                    :p.
//                    The following defines are used as the POS flags:
//                    :sl compact.
//                    :li.MORPH_PRONOUN
//                    :li.MORPH_VERB
//                    :li.MORPH_NOUN
//                    :li.MORPH_ADJECTIVE
//                    :li.MORPH_ADVERB
//                    :li.MORPH_PREPOSITION
//                    :li.MORPH_INTERJECTION
//                    :li.MORPH_CONJUNCTION
//                    :esl.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphGetPOSInfo( SHORT sLanguageID,
//                                            PSZ pszInTerm,
//                                            PUSHORT pulPOSInfo );
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    PSZ      pszInTerm      pointer to input term
//------------------------------------------------------------------------------
// Output parameter:  PUSHORT  pulPOSInfo     address of variable receiving
//                                               the POS info
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           USHORT ulPOSInfo;          // POS info
//                    SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );
//
//                    usRC = MorphGetPOSInfo( sLangID, "run", &ulPOSInfo );
//
//                    after this call the variable ulPOSInfo pszList would
//                    contain the following POS value:
//
//                            MORPH_NOUN | MORPH_VERB
//
//------------------------------------------------------------------------------
// Function flow:     get language control block pointer
//                    do
//                      if term is in POS no-lookup hash table
//                        set return code to MORPH_NOT_FOUND
//                      else if term is in POS cache
//                        get POS info from cache
//                      else
//                        call GETPOS function of language exit
//                      endif
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphGetPOSInfo
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInTermW,                // pointer to input term
   PULONG   pulPOSInfo,                 // address of variable receiving the
                                       //    POS info
   ULONG    ulOemCP
)
{
  USHORT     usRC = MORPH_OK;          // return code of function
  PLCB       pLCB = NULL;              // pointer to language control block

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  usRC = MorphGetLCB( sLanguageID, &pLCB );
  *pulPOSInfo = 0;

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

  /********************************************************************/
  /* Create and load POS hash table if not active yet                 */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    if ( pLCB->POSHashCache == NULL )
    {
      pLCB->POSHashCache = MorphHashCreate( 1000 );
      if ( pLCB->POSHashCache )
      {
        /******************************************************************/
        /* build name of POS cache file                                   */
        /******************************************************************/
        UtlMakeEQFPath( pLCB->szBuffer, NULC, DIC_PATH, NULL );
        strcat( pLCB->szBuffer, BACKSLASH_STR );
        Utlstrccpy( pLCB->szBuffer + strlen(pLCB->szBuffer),
                    pLCB->szMorphDict, DOT );
        strcat( pLCB->szBuffer, EXT_OF_POSCACHE );

        /******************************************************************/
        /* read POS cache from disk                                       */
        /******************************************************************/
        MorphHashRead( pLCB->POSHashCache, pLCB->szBuffer, FALSE );
      }
      else
      {
        usRC = MORPH_NO_MEMORY;
      } /* endif */
    } /* endif */

    if ( (usRC == MORPH_OK)  && (pLCB->POSHashNoLookup == NULL) )
    {
      pLCB->POSHashNoLookup = MorphHashCreate( 1000 );
      if ( pLCB->POSHashNoLookup )
      {
        /******************************************************************/
        /* build name of 'POS do not lookup' hash table                   */
        /******************************************************************/
        UtlMakeEQFPath( pLCB->szBuffer, NULC, DIC_PATH, NULL );
        strcat( pLCB->szBuffer, BACKSLASH_STR );
        Utlstrccpy( pLCB->szBuffer + strlen(pLCB->szBuffer),
                    pLCB->szMorphDict, DOT );
        strcat( pLCB->szBuffer, EXT_OF_POSNOLOOKUP );

        /******************************************************************/
        /* read term cache from disk                                      */
        /******************************************************************/
        MorphHashRead( pLCB->POSHashNoLookup, pLCB->szBuffer, FALSE );
      }
      else
      {
        usRC = MORPH_NO_MEMORY;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Check if term is already in one of our POS hash tables           */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    /******************************************************************/
    /* first look into our hash tables, if not found call stem form   */
    /* reduction for the term and look into the hash tables again     */
    /******************************************************************/
    if ( MorphHashSearch( pLCB->POSHashNoLookup, pszInTermW, NULL, NULL ) )
    {
      /**************************************************************/
      /* no lookup required;                                        */
      /**************************************************************/
      usRC = MORPH_NOT_FOUND;
    }
    else if ( MorphHashSearch( pLCB->POSHashCache, pszInTermW,
                               pulPOSInfo, NULL ) )
    {
    }
    else
    {
	    USHORT usPOSInfo;
	    usPOSInfo = (USHORT)*pulPOSInfo;

      /****************************************************************/
      /* call language exit for POS determination                     */
      /****************************************************************/
      Unicode2ASCII( pszInTermW, pLCB->szBuffer, ulOemCP );
      usRC = GetPos( pLCB->pvLangCB, pLCB->szBuffer, &usPOSInfo );
      *pulPOSInfo = usPOSInfo;
      if ( usRC == MORPH_FUNC_NOT_SUPPORTED )
      {
        /************************************************************/
        /* POS recognition is not supported, treat as no POS had    */
        /* been found                                              */
        /************************************************************/
        usRC = MORPH_NOT_FOUND;
      } /* endif */

      /**************************************************************/
      /* update POS hash tables                                     */
      /**************************************************************/
      if ( usRC == MORPH_OK )
      {
        if ( !MorphHashAdd( pLCB->POSHashCache, pszInTermW, *pulPOSInfo, NULL ) )
        {
          usRC = MORPH_NO_MEMORY;
        } /* endif */
      }
      else
      {
        if ( !MorphHashAdd( pLCB->POSHashNoLookup, pszInTermW, 0, NULL ) )
        {
          usRC = MORPH_NO_MEMORY;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Return our return code to the calling function                   */
  /********************************************************************/
  return( usRC );

} /* end of function MorphGetPOSInfo */


// a secondary thread ends
USHORT MorphThreadEnds
(
   SHORT    sLanguageID                // language ID
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  PLCB       pLCB = NULL;              // pointer to language control block

  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  return( usRC );

} /* end of function MorphThreadEnds */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphGetLanguageID           Get ID for a language
//------------------------------------------------------------------------------
// Description:       Get the unique ID for the given the language. The ID is
//                    required by subsequent Morph... calls.
//------------------------------------------------------------------------------
// Function call:     MorphGetLanguageID( PSZ pszLanguage, PSHORT psLanguageID)
//------------------------------------------------------------------------------
// Input parameter:   PSZ     pszLanguage         name of language (name used
//                                                in the PROPERTY.LNG file to
//                                                identiy the language)
//------------------------------------------------------------------------------
// Output parameter:  PSHORT  psLanguageID        address of variable receiving
//                                                the language ID
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      The dictioaries for the language are activated and the
//                    stem form caches are read from disk if this is the first
//                    call for the given language.
//------------------------------------------------------------------------------
// Samples:           SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );
//
//------------------------------------------------------------------------------
// Function flow:     search language in language table
//                    if not found
//                      allocate language control block
//                      get values for language form PROPERTY.LNG file
//                      fill language control block
//                      create hash tables
//                      read stem form cache from disk
//                      read no-lookup hash table from disk
//                      read POS cache from disk
//                      read POS no-lookup hash table from disk
//                      load language exit DLL
//                      get procedure addresses from language exit
//                      call INIT entry point of language exit
//                      add language to list of active languages
//                      set caller's language ID
//                      cleanup in case of errors
//                      return function return code
//------------------------------------------------------------------------------
USHORT MorphGetLanguageID
(
  PSZ     pszLanguage,                 // name of language
  PSHORT  psLanguageID                 // address of variable receiving the
                                       //    language ID
)
{
  USHORT   usRC = MORPH_OK;            // function returncode
  USHORT   usLangCode;                 // language code from language properties
  SHORT    sID = -1;                   // evaluated language ID
  SHORT    sI;                         // general loop index
  PLCB     pLCB = NULL;                // pointer to new language control block
  PLCB     pLCBTemp;
  PLCB*    ppLCBTemp;
  USHORT   usTid;

  {
    /******************************************************************/
    /* get Task-ID                                                    */
    /* use stack segment as differentiator to define what application */
    /* is calling this time..                                         */
    /******************************************************************/
        WORD   usTask;

        _asm
          {
            mov      ax, SS
            mov      usTask, ax
          }
     usTid = usTask;
   }

  /********************************************************************/
  /* Allocate LCB table if necessary                                  */
  /********************************************************************/
  if (bLCBTableAllocated == FALSE)
  {
    if ( !UtlAlloc( (PVOID *)&ppIdToLCB, 0L, (LONG) (sizeof(PLCB) * 256), NOMSG ) )
    {
      usRC = MORPH_NO_MEMORY;
	}
	else
	{
	  bLCBTableAllocated = TRUE;
	  memset(ppIdToLCB, 0, sizeof(PLCB) * 256);
    } /* endif */
  }
  
  /********************************************************************/
  /* Search language in currently active languages                    */
  /********************************************************************/
  ppLCBTemp = ppIdToLCB;
  for ( sI = 0; sI < 256, *ppLCBTemp != 0; sI++, ppLCBTemp++ )
  {
    pLCBTemp = *ppLCBTemp;
    if ( ( pLCBTemp->tidThread == usTid ) &&
           (stricmp( pszLanguage, pLCBTemp->szLanguage ) == 0) )
    {
      sID = sI;                        // remember ID of language
      break;                           // leave for loop
    } /* endif */
  } /* endfor */


  /********************************************************************/
  /* Allocate language control block                                  */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    if ( !UtlAlloc( (PVOID *)&pLCB, 0L, (LONG)sizeof(LCB), NOMSG ) )
    {
      usRC = MORPH_NO_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Get language properties                                          */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0) )
  {
    strcpy( pLCB->szBuffer, pszLanguage );
    strcpy(pLCB->szLanguage, pszLanguage);
    strcpy(pLCB->szMorphDict, pszLanguage);
    strcpy(pLCB->szAddendaDict, pszLanguage);
    pLCB->usCodePage = 850;
    pLCB->fCompSep = FALSE;

    SpellFactory* tSpellFactoryInstance = SpellFactory::getInstance();
    if (NULL == tSpellFactoryInstance)
    {
      usRC = MORPH_NO_MEMORY;
    }
    MorphFactory* tMorphFactoryInstance = MorphFactory::getInstance();
    if (NULL == tMorphFactoryInstance)
    {
      usRC = MORPH_NO_MEMORY;
    }
    if (MORPH_OK == usRC && tMorphFactoryInstance->isSupported(pszLanguage))
    {
    }
    else
    {
      usRC = MORPH_OK; // GQ: disabled temporarely the result of isSupported, correct code is: usRC = MORPH_NO_LANG_PROPS;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Fill language control block                                      */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    pLCB->tidThread = usTid;
  } /* endif */
  
  /********************************************************************/
  /* Activate language exit                                           */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    usRC = MorphInit( &pLCB->pvLangCB,
                             pLCB->szMorphDict,
                             pLCB->szSpellPlugin,
                             pLCB->szMorphPlugin,
                             pLCB->usCodePage,
                             usLangCode );
    if ( usRC != MORPH_OK )
    {
       pLCB->pvLangCB = NULL;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Activate stem form caches                                        */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    /******************************************************************/
    /* Create tree for caching                                        */
    /******************************************************************/
    pLCB->HashCache       = MorphHashCreate( 2000 );
    pLCB->HashNoLookup    = MorphHashCreate( 2000 );

    if ( (pLCB->HashCache == NULL) || (pLCB->HashNoLookup == NULL) )
    {
      usRC = MORPH_NO_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Load stem form chache files                                      */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    /******************************************************************/
    /* build name of stem form cache file                             */
    /******************************************************************/
    UtlMakeEQFPath( pLCB->szBuffer, NULC, DIC_PATH, NULL );
    strcat( pLCB->szBuffer, BACKSLASH_STR );
    strcat(pLCB->szBuffer, pszLanguage);
    strcat( pLCB->szBuffer, EXT_OF_STEMCACHE );

    /******************************************************************/
    /* read term cache from disk                                      */
    /******************************************************************/
    MorphHashRead( pLCB->HashCache, pLCB->szBuffer, TRUE );

    /******************************************************************/
    /* build name of 'do not lookup' hash table                       */
    /******************************************************************/
    UtlMakeEQFPath( pLCB->szBuffer, NULC, DIC_PATH, NULL );
    strcat( pLCB->szBuffer, BACKSLASH_STR );
    strcat(pLCB->szBuffer, pLCB->szLanguage);
    strcat( pLCB->szBuffer, EXT_OF_NOLOOKUP );

    /******************************************************************/
    /* read term cache from disk                                      */
    /******************************************************************/
    MorphHashRead( pLCB->HashNoLookup, pLCB->szBuffer, FALSE );
  } /* endif */

  /********************************************************************/
  /* Add language to active languages                                 */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    ppLCBTemp = ppIdToLCB;
    for ( sI = 0; sI < 256, *ppLCBTemp != 0; sI++, ppLCBTemp++ );

    if ( sI < 256 )
    {
      *ppLCBTemp = pLCB;
      sID = sI;
    }
    else
    {
      usRC = MORPH_TOO_MUCH_LANGUAGES;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Set caller's language ID                                         */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    *psLanguageID = sID;
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( usRC != MORPH_OK )
  {
    if ( pLCB )
    {
      if ( pLCB->pvLangCB )
      {
        Terminate( pLCB->pvLangCB );
      } /* endif */

      if ( pLCB->pCompList )
      {
        UtlAlloc( (PVOID *)&(pLCB->pCompList), 0L, 0L, NOMSG );
      } /* endif */

      if ( pLCB->HashCache )        MorphHashDestroy( pLCB->HashCache );
      if ( pLCB->HashNoLookup )     MorphHashDestroy( pLCB->HashNoLookup );

      UtlAlloc( (PVOID *)&pLCB, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Return function return code to caller                            */
  /********************************************************************/
  return( usRC );

} /* end of function MorphGetLanguageID */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphGetLanguageIDByPlugin           Get ID for a language
//------------------------------------------------------------------------------
// Description:       Get the unique ID for the given the language. The ID is
//                    required by subsequent Morph... calls.
//------------------------------------------------------------------------------
// Function call:     MorphGetLanguageIDByPlugin(PSZ pszSpellPlugin, PSZ pszMorphPlugin, PSZ pszLanguage, PSHORT psLanguageID)
//------------------------------------------------------------------------------
// Input parameter:   PSZ     pszLanguage         name of language (name used
//                                                in the PROPERTY.LNG file to
//                                                identiy the language)
//                    PSZ     pszSpellPlugin      name of the spell check plugin
//                    PSZ     pszMorphPlugin      name of the morph plugin
//------------------------------------------------------------------------------
// Output parameter:  PSHORT  psLanguageID        address of variable receiving
//                                                the language ID
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      The dictioaries for the language are activated and the
//                    stem form caches are read from disk if this is the first
//                    call for the given language.
//------------------------------------------------------------------------------
// Samples:           SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageIDByPlugin( "SpellPlugin", "MorphPlugin", "English(U.S.)", &sLangID );
//
//------------------------------------------------------------------------------
// Function flow:     search language in language table
//                    if not found
//                      allocate language control block
//                      get values for language form PROPERTY.LNG file
//                      fill language control block
//                      create hash tables
//                      read stem form cache from disk
//                      read no-lookup hash table from disk
//                      read POS cache from disk
//                      read POS no-lookup hash table from disk
//                      load language exit DLL
//                      get procedure addresses from language exit
//                      call INIT entry point of language exit
//                      add language to list of active languages
//                      set caller's language ID
//                      cleanup in case of errors
//                      return function return code
//------------------------------------------------------------------------------
USHORT MorphGetLanguageIDByPlugin
(
 PSZ     pszSpellPlugin,              // name of the spell plug-in
 PSZ     pszMorphPlugin,              // name of the morph plug-in
 PSZ     pszLanguage,                 // name of language
 PSHORT  psLanguageID                 // address of variable receiving the language ID
 )
{
  USHORT   usRC = MORPH_OK;            // function returncode
  USHORT   usLangCode;                 // language code from language properties
  SHORT    sID = -1;                   // evaluated language ID
  SHORT    sI;                         // general loop index
  PLCB     pLCB = NULL;                // pointer to new language control block
  PLCB     pLCBTemp;
  PLCB*    ppLCBTemp;
  USHORT   usTid;

  {
    /******************************************************************/
    /* get Task-ID                                                    */
    /* use stack segment as differentiator to define what application */
    /* is calling this time..                                         */
    /******************************************************************/
    WORD   usTask;

    _asm
    {
      mov      ax, SS
        mov      usTask, ax
    }
    usTid = usTask;
  }

  /********************************************************************/
  /* Allocate LCB table if necessary                                  */
  /********************************************************************/
  if (bLCBTableAllocated == FALSE)
  {
    if ( !UtlAlloc( (PVOID *)&ppIdToLCB, 0L, (LONG) (sizeof(PLCB) * 256), NOMSG ) )
    {
      usRC = MORPH_NO_MEMORY;
    }
    else
    {
      bLCBTableAllocated = TRUE;
      memset(ppIdToLCB, 0, sizeof(PLCB) * 256);
    } /* endif */
  }

  /********************************************************************/
  /* Search language in currently active languages                    */
  /********************************************************************/
  ppLCBTemp = ppIdToLCB;
  for ( sI = 0; sI < 256, *ppLCBTemp != 0; sI++, ppLCBTemp++ )
  {
    pLCBTemp = *ppLCBTemp;
    if ( ( pLCBTemp->tidThread == usTid ) &&
      (stricmp( pszLanguage, pLCBTemp->szLanguage ) == 0) )
    {
      if (NULL == pszSpellPlugin)
      {
        if (pLCBTemp->szSpellPlugin != NULL)
        {
          continue;
        }
      }
      else
      {
        if (stricmp(pszSpellPlugin, pLCBTemp->szSpellPlugin) != 0)
        {
          continue;
        }
      }

      if (NULL == pszMorphPlugin)
      {
        if (pLCBTemp->szMorphPlugin != NULL)
        {
          continue;
        }
      }
      else
      {
        if (stricmp(pszMorphPlugin, pLCBTemp->szMorphPlugin) != 0)
        {
          continue;
        }
      }

      sID = sI;                        // remember ID of language
      break;                           // leave for loop
    } /* endif */
  } /* endfor */


  /********************************************************************/
  /* Allocate language control block                                  */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    if ( !UtlAlloc( (PVOID *)&pLCB, 0L, (LONG)sizeof(LCB), NOMSG ) )
    {
      usRC = MORPH_NO_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Get language properties                                          */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0) )
  {
    strcpy( pLCB->szBuffer, pszLanguage );
    strcpy(pLCB->szLanguage, pszLanguage);
    strcpy(pLCB->szMorphDict, pszLanguage);
    strcpy(pLCB->szAddendaDict, pszLanguage);
    if (NULL != pszMorphPlugin)
    {
      strcpy(pLCB->szMorphPlugin, pszMorphPlugin);
    }
    if (NULL != pszSpellPlugin)
    {
      strcpy(pLCB->szSpellPlugin, pszSpellPlugin);
    }
    pLCB->usCodePage = 850;
    pLCB->fCompSep = FALSE;

    SpellFactory* tSpellFactoryInstance = SpellFactory::getInstance();
    if (NULL == tSpellFactoryInstance)
    {
      usRC = MORPH_NO_MEMORY;
    }
    MorphFactory* tMorphFactoryInstance = MorphFactory::getInstance();
    if (NULL == tMorphFactoryInstance)
    {
      usRC = MORPH_NO_MEMORY;
    }
    if (MORPH_OK == usRC && tSpellFactoryInstance->isSupported(pszLanguage) && tMorphFactoryInstance->isSupported(pszLanguage))
    {
    }
    else
    {
      usRC = MORPH_NO_LANG_PROPS;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Fill language control block                                      */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    pLCB->tidThread = usTid;
  } /* endif */

  /********************************************************************/
  /* Activate language exit                                           */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    usRC = MorphInit( &pLCB->pvLangCB,
      pLCB->szMorphDict,
      pLCB->szSpellPlugin,
      pLCB->szMorphPlugin,
      pLCB->usCodePage,
      usLangCode );
    if ( usRC != MORPH_OK )
    {
      pLCB->pvLangCB = NULL;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Activate stem form caches                                        */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    /******************************************************************/
    /* Create tree for caching                                        */
    /******************************************************************/
    pLCB->HashCache       = MorphHashCreate( 2000 );
    pLCB->HashNoLookup    = MorphHashCreate( 2000 );

    if ( (pLCB->HashCache == NULL) || (pLCB->HashNoLookup == NULL) )
    {
      usRC = MORPH_NO_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Load stem form chache files                                      */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    /******************************************************************/
    /* build name of stem form cache file                             */
    /******************************************************************/
    UtlMakeEQFPath( pLCB->szBuffer, NULC, DIC_PATH, NULL );
    strcat( pLCB->szBuffer, BACKSLASH_STR );
    strcat(pLCB->szBuffer, pszLanguage);
    strcat( pLCB->szBuffer, EXT_OF_STEMCACHE );

    /******************************************************************/
    /* read term cache from disk                                      */
    /******************************************************************/
    MorphHashRead( pLCB->HashCache, pLCB->szBuffer, TRUE );

    /******************************************************************/
    /* build name of 'do not lookup' hash table                       */
    /******************************************************************/
    UtlMakeEQFPath( pLCB->szBuffer, NULC, DIC_PATH, NULL );
    strcat( pLCB->szBuffer, BACKSLASH_STR );
    strcat(pLCB->szBuffer, pLCB->szLanguage);
    strcat( pLCB->szBuffer, EXT_OF_NOLOOKUP );

    /******************************************************************/
    /* read term cache from disk                                      */
    /******************************************************************/
    MorphHashRead( pLCB->HashNoLookup, pLCB->szBuffer, FALSE );
  } /* endif */

  /********************************************************************/
  /* Add language to active languages                                 */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    ppLCBTemp = ppIdToLCB;
    for ( sI = 0; sI < 256, *ppLCBTemp != 0; sI++, ppLCBTemp++ );

    if ( sI < 256 )
    {
      *ppLCBTemp = pLCB;
      sID = sI;
    }
    else
    {
      usRC = MORPH_TOO_MUCH_LANGUAGES;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Set caller's language ID                                         */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    *psLanguageID = sID;
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( usRC != MORPH_OK )
  {
    if ( pLCB )
    {
      if ( pLCB->pvLangCB )
      {
        Terminate( pLCB->pvLangCB );
      } /* endif */

      if ( pLCB->pCompList )
      {
        UtlAlloc( (PVOID *)&(pLCB->pCompList), 0L, 0L, NOMSG );
      } /* endif */

      if ( pLCB->HashCache )        MorphHashDestroy( pLCB->HashCache );
      if ( pLCB->HashNoLookup )     MorphHashDestroy( pLCB->HashNoLookup );

      UtlAlloc( (PVOID *)&pLCB, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Return function return code to caller                            */
  /********************************************************************/
  return( usRC );

} /* end of function MorphGetLanguageIDByPlugin */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphCheckLanguage           check if language is inst.
//------------------------------------------------------------------------------
// Description:       Check if the specified language is installed and avail.
//                    on the system and return the ID.
//                    An ID >= 0 indicates that the language is in use..
//------------------------------------------------------------------------------
// Function call:     MorphCheckLanguage( PSZ pszLanguage, PSHORT &sID )
//------------------------------------------------------------------------------
// Input parameter:   PSZ     pszLanguage         name of language (name used
//                                                in the PROPERTY.LNG file to
//                                                identiy the language)
//------------------------------------------------------------------------------
// Output parameter:  PSHORT  psLanguageID        address of variable receiving
//                                                the language ID
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE        language available and accessible
//                    FALSE       not accessible
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Samples:
//                    fOK  = MorphCheckLanguage( "English(U.S.)", &sID );
//------------------------------------------------------------------------------
// Function flow:     search language in language table
//                    if not found
//                      allocate language control block
//                      get values for language form PROPERTY.LNG file
//                      fill language control block
//                      load language exit DLL
//                      get procedure addresses from language exit
//                      call INIT entry point of language exit and set install.
//                      cleanup
//                    endif
//                    return function return code
//------------------------------------------------------------------------------
BOOL MorphCheckLanguage
(
  PSZ     pszLanguage,                 // name of language
  PSHORT  psLanguageID                 // variable receiving the Language ID
)
{
  USHORT   usRC = MORPH_OK;            // function returncode
  USHORT   usLangCode = 0;             // language code from language properties
  SHORT    sID = -1;                   // evaluated language ID
  SHORT    sI;                         // general loop index
  PLCB     pLCB = NULL;                // pointer to new language control block
  PLCB     pLCBTemp;
  PLCB*    ppLCBTemp;
  BOOL     fInstalled = FALSE;

  /********************************************************************/
  /* Allocate LCB table if necessary                                  */
  /********************************************************************/
  if (bLCBTableAllocated == FALSE)
  {
    if ( !UtlAlloc( (PVOID *)&ppIdToLCB, 0L, (LONG) (sizeof(PLCB) * 256), NOMSG ) )
    {
      usRC = MORPH_NO_MEMORY;
	}
	else
	{
	  bLCBTableAllocated = TRUE;
	  memset(ppIdToLCB, 0, sizeof(PLCB) * 256);
    } /* endif */
  }

  /********************************************************************/
  /* Search language in currently active languages                    */
  /********************************************************************/
  ppLCBTemp = ppIdToLCB;
  for ( sI = 0; sI < 256, *ppLCBTemp != 0; sI++, ppLCBTemp++ )
  {
    pLCBTemp = *ppLCBTemp;
    if ( stricmp( pszLanguage, pLCBTemp->szLanguage ) == 0 )
    {
      sID = sI;
      fInstalled = TRUE;               // language installed and used
      break;                           // leave for loop
    } /* endif */
  } /* endfor */

  /********************************************************************/
  /* Allocate language control block                                  */
  /********************************************************************/

  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    if ( !UtlAlloc( (PVOID *)&pLCB, 0L, (LONG)sizeof(LCB), NOMSG ) )
    {
      usRC = MORPH_NO_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Get language properties                                          */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0) )
  {
    strcpy( pLCB->szBuffer, pszLanguage );
    strcpy(pLCB->szLanguage, pszLanguage);
    strcpy(pLCB->szMorphDict, pszLanguage);
    strcpy(pLCB->szAddendaDict, pszLanguage);
    pLCB->usCodePage = 850;
    pLCB->fCompSep = FALSE;

    SpellFactory* tSpellFactoryInstance = SpellFactory::getInstance();
    if (NULL == tSpellFactoryInstance)
    {
      usRC = MORPH_NO_MEMORY;
    }
    MorphFactory* tMorphFactoryInstance = MorphFactory::getInstance();
    if (NULL == tMorphFactoryInstance)
    {
      usRC = MORPH_NO_MEMORY;
    }
    if (MORPH_OK == usRC && tSpellFactoryInstance->isSupported(pszLanguage) && tMorphFactoryInstance->isSupported(pszLanguage))
    {
    }
    else
    {
      usRC = MORPH_NO_LANG_PROPS;
    } /* endif */

  } /* endif */

  /********************************************************************/
  /* Activate language exit                                           */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    usRC = MorphInit( &pLCB->pvLangCB,
                             pLCB->szMorphDict,
                             pLCB->szSpellPlugin,
                             pLCB->szMorphPlugin,
                             MORPHCODEPAGE,
                             usLangCode );
    fInstalled = ( usRC == MORPH_OK );
  } /* endif */

  /********************************************************************/
  /* Cleanup -- language was not used before therefore free anything..*/
  /********************************************************************/
  if ( sID < 0 )
  {
    if ( pLCB )
    {
      if ( pLCB->pvLangCB )
      {
        Terminate( pLCB->pvLangCB );
      } /* endif */

      if ( pLCB->pCompList )
      {
        UtlAlloc( (PVOID *)&(pLCB->pCompList), 0L, 0L, NOMSG );
      } /* endif */
      UtlAlloc( (PVOID *)&pLCB, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Return function return code to caller                            */
  /********************************************************************/
  *psLanguageID = sID;              // variable receiving the Language ID

  return( fInstalled );

} /* end of function MorphCheckLanguage */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphCheckLanguageByPlugin           check if language is inst.
//------------------------------------------------------------------------------
// Description:       Check if the specified language is installed and avail.
//                    on the system and return the ID.
//                    An ID >= 0 indicates that the language is in use..
//------------------------------------------------------------------------------
// Function call:     MorphCheckLanguageByPlugin( PSZ pszSpellPlugin, PSZ pszMorphPlugin, PSZ pszLanguage, PSHORT &sID )
//------------------------------------------------------------------------------
// Input parameter:   PSZ     pszLanguage         name of language (name used
//                                                in the PROPERTY.LNG file to
//                                                identiy the language)
//                    PSZ     pszSpellPlugin      name of the spell check plugin
//                    PSZ     pszMorphPlugin      name of the morph plugin
//------------------------------------------------------------------------------
// Output parameter:  PSHORT  psLanguageID        address of variable receiving
//                                                the language ID
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE        language available and accessible
//                    FALSE       not accessible
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Samples:
//                    fOK  = MorphCheckLanguageByPlugin( "SpellPlugin", "MorphPlugin", "English(U.S.)", &sID );
//------------------------------------------------------------------------------
// Function flow:     search language in language table
//                    if not found
//                      allocate language control block
//                      get values for language form PROPERTY.LNG file
//                      fill language control block
//                      load language exit DLL
//                      get procedure addresses from language exit
//                      call INIT entry point of language exit and set install.
//                      cleanup
//                    endif
//                    return function return code
//------------------------------------------------------------------------------
BOOL MorphCheckLanguageByPlugin
(
 PSZ     pszSpellPlugin,              // the name of the spell check plugin
 PSZ     pszMorphPlugin,              // the name of the morph plugin
 PSZ     pszLanguage,                 // name of language
 PSHORT  psLanguageID                 // variable receiving the Language ID
 )
{
  USHORT   usRC = MORPH_OK;            // function returncode
  USHORT   usLangCode = 0;             // language code from language properties
  SHORT    sID = -1;                   // evaluated language ID
  SHORT    sI;                         // general loop index
  PLCB     pLCB = NULL;                // pointer to new language control block
  PLCB     pLCBTemp;
  PLCB*    ppLCBTemp;
  BOOL     fInstalled = FALSE;

  /********************************************************************/
  /* Allocate LCB table if necessary                                  */
  /********************************************************************/
  if (bLCBTableAllocated == FALSE)
  {
    if ( !UtlAlloc( (PVOID *)&ppIdToLCB, 0L, (LONG) (sizeof(PLCB) * 256), NOMSG ) )
    {
      usRC = MORPH_NO_MEMORY;
    }
    else
    {
      bLCBTableAllocated = TRUE;
      memset(ppIdToLCB, 0, sizeof(PLCB) * 256);
    } /* endif */
  }

  /********************************************************************/
  /* Search language in currently active languages                    */
  /********************************************************************/
  ppLCBTemp = ppIdToLCB;
  for ( sI = 0; sI < 256, *ppLCBTemp != 0; sI++, ppLCBTemp++ )
  {
    pLCBTemp = *ppLCBTemp;
    if ( stricmp( pszLanguage, pLCBTemp->szLanguage ) == 0 )
    {
      if (NULL == pszSpellPlugin)
      {
        if (pLCBTemp->szSpellPlugin != NULL)
        {
          continue;
        }
      }
      else
      {
        if (stricmp(pszSpellPlugin, pLCBTemp->szSpellPlugin) != 0)
        {
          continue;
        }
      }

      if (NULL == pszMorphPlugin)
      {
        if (pLCBTemp->szMorphPlugin != NULL)
        {
          continue;
        }
      }
      else
      {
        if (stricmp(pszMorphPlugin, pLCBTemp->szMorphPlugin) != 0)
        {
          continue;
        }
      }

      sID = sI;
      fInstalled = TRUE;               // language installed and used
      break;                           // leave for loop
    } /* endif */
  } /* endfor */

  /********************************************************************/
  /* Allocate language control block                                  */
  /********************************************************************/

  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    if ( !UtlAlloc( (PVOID *)&pLCB, 0L, (LONG)sizeof(LCB), NOMSG ) )
    {
      usRC = MORPH_NO_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Get language properties                                          */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0) )
  {
    strcpy( pLCB->szBuffer, pszLanguage );
    strcpy(pLCB->szLanguage, pszLanguage);
    strcpy(pLCB->szMorphDict, pszLanguage);
    strcpy(pLCB->szAddendaDict, pszLanguage);
    pLCB->usCodePage = 850;
    pLCB->fCompSep = FALSE;

    SpellFactory* tSpellFactoryInstance = SpellFactory::getInstance();
    if (NULL == tSpellFactoryInstance)
    {
      usRC = MORPH_NO_MEMORY;
    }
    MorphFactory* tMorphFactoryInstance = MorphFactory::getInstance();
    if (NULL == tMorphFactoryInstance)
    {
      usRC = MORPH_NO_MEMORY;
    }
    if (MORPH_OK == usRC && tSpellFactoryInstance->isSupported(pszLanguage) && tMorphFactoryInstance->isSupported(pszLanguage))
    {
    }
    else
    {
      usRC = MORPH_NO_LANG_PROPS;
    } /* endif */

  } /* endif */

  /********************************************************************/
  /* Activate language exit                                           */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    usRC = MorphInit( &pLCB->pvLangCB,
      pLCB->szMorphDict,
      pLCB->szSpellPlugin,
      pLCB->szMorphPlugin,
      MORPHCODEPAGE,
      usLangCode );
    fInstalled = ( usRC == MORPH_OK );
  } /* endif */

  /********************************************************************/
  /* Cleanup -- language was not used before therefore free anything..*/
  /********************************************************************/
  if ( sID < 0 )
  {
    if ( pLCB )
    {
      if ( pLCB->pvLangCB )
      {
        Terminate( pLCB->pvLangCB );
      } /* endif */

      if ( pLCB->pCompList )
      {
        UtlAlloc( (PVOID *)&(pLCB->pCompList), 0L, 0L, NOMSG );
      } /* endif */
      UtlAlloc( (PVOID *)&pLCB, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Return function return code to caller                            */
  /********************************************************************/
  *psLanguageID = sID;              // variable receiving the Language ID

  return( fInstalled );

} /* end of function MorphCheckLanguageByPlugin */



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphFreeLanguageID          End access to a language
//------------------------------------------------------------------------------
// Description:       Frees the access of a unit to a language. In the current
//                    version this function does nothing.
//------------------------------------------------------------------------------
// Function call:     MorphFreeLanguageID( SHORT sLanguageID )
//------------------------------------------------------------------------------
// Input parameter:   SHORT  sLanguageID          the language ID being freed
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     none
//------------------------------------------------------------------------------
USHORT MorphFreeLanguageID
(
  SHORT    sLanguageID                 // language ID being freed
)
{
  USHORT   usRC = MORPH_OK;            // function returncode

  sLanguageID;

  return( usRC );

} /* end of function MorphFreeLanguageID */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphTerminate      Terminate morphologic functions
//------------------------------------------------------------------------------
// Description:       Terminates morphologic processing by closing all open
//                    dictionaries, writing the stem form caches to disk, and
//                    freeing all memory allocated by Morph... functions.
//                    :p.
//                    This function should only be called during shutdown of
//                    TranslationManager/2.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphTerminate VOID );
//------------------------------------------------------------------------------
// Input parameter:   VOID
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      No more morphologic functions can be called anymore after
//                    this function call.
//------------------------------------------------------------------------------
// Samples:           usRC = MorphTerminate();
//------------------------------------------------------------------------------
// Function flow:     loop over all active languages
//                      get pointer to language control block
//                      call TERMINATE entry point of language exit
//                      write stem form cache to disk
//                      write no-lookup hash table to disk
//                      write POS cache to disk
//                      write POS no-lookup hash table to disk
//                      destroy hash tables
//                      free language control block
//                      deactivate language in language table
//                    endloop
//------------------------------------------------------------------------------
USHORT MorphTerminate()
{
  USHORT    usRC = MORPH_OK;           // function return code
  SHORT    sI;                         // general loop index
  PLCB     pLCB = NULL;                // pointer to new language control block
  PLCB*    ppLCBTemp;

  /********************************************************************/
  /* Terminate all active languages                                   */
  /*  under Windows we only terminate the active languages opened     */
  /* by this instance - otherwise it may end in a Harakiri....        */
  /********************************************************************/
    /******************************************************************/
    /* get Task-ID                                                    */
    /* use stack segment as differentiator to define what application */
    /* is calling this time..                                         */
    /******************************************************************/
   WORD   usTask;

        _asm
          {
            mov      ax, SS
            mov      usTask, ax
          }


  if ( !bLCBTableAllocated )
  {
    // no language table has been allocated, so leave termination function asap
    return( 0 );
  } /* end */     

  ppLCBTemp = ppIdToLCB;
  for ( sI = 0; sI < 256, *ppLCBTemp != 0; sI++, ppLCBTemp++ )
  {
    pLCB = *ppLCBTemp;

    if ( pLCB->tidThread == usTask )
    {
      Terminate( pLCB->pvLangCB );

      /******************************************************************/
      /* build name of stem form cache file                             */
      /******************************************************************/
      UtlMakeEQFPath( pLCB->szBuffer, NULC, DIC_PATH, NULL );
      strcat( pLCB->szBuffer, BACKSLASH_STR );
      strcat(pLCB->szBuffer, pLCB->szLanguage);
//       Utlstrccpy( pLCB->szBuffer + strlen(pLCB->szBuffer),
//                   pLCB->szMorphDict, DOT );
      strcat( pLCB->szBuffer, EXT_OF_STEMCACHE );

      /******************************************************************/
      /* write term cache to disk                                       */
      /******************************************************************/
      MorphHashWrite( pLCB->HashCache, pLCB->szBuffer );

      /******************************************************************/
      /* build name of 'do not lookup' hash table                       */
      /******************************************************************/
      UtlMakeEQFPath( pLCB->szBuffer, NULC, DIC_PATH, NULL );
      strcat( pLCB->szBuffer, BACKSLASH_STR );
      strcat(pLCB->szBuffer, pLCB->szLanguage);
//       Utlstrccpy( pLCB->szBuffer + strlen(pLCB->szBuffer),
//                   pLCB->szMorphDict, DOT );
      strcat( pLCB->szBuffer, EXT_OF_NOLOOKUP );

      /******************************************************************/
      /* write term cache to disk                                       */
      /******************************************************************/
      MorphHashWrite( pLCB->HashNoLookup, pLCB->szBuffer );

      if ( pLCB->POSHashCache != NULL )
      {
        /******************************************************************/
        /* build name of POS cache file                                   */
        /******************************************************************/
        UtlMakeEQFPath( pLCB->szBuffer, NULC, DIC_PATH, NULL );
        strcat( pLCB->szBuffer, BACKSLASH_STR );
        Utlstrccpy( pLCB->szBuffer + strlen(pLCB->szBuffer),
                    pLCB->szMorphDict, DOT );
        strcat( pLCB->szBuffer, EXT_OF_POSCACHE );

        /******************************************************************/
        /* write term cache to disk                                       */
        /******************************************************************/
        MorphHashWrite( pLCB->POSHashCache, pLCB->szBuffer );
      } /* endif */

      if ( pLCB->POSHashNoLookup != NULL )
      {
        /******************************************************************/
        /* build name of 'POS do not lookup' hash table                   */
        /******************************************************************/
        UtlMakeEQFPath( pLCB->szBuffer, NULC, DIC_PATH, NULL );
        strcat( pLCB->szBuffer, BACKSLASH_STR );
        Utlstrccpy( pLCB->szBuffer + strlen(pLCB->szBuffer),
                    pLCB->szMorphDict, DOT );
        strcat( pLCB->szBuffer, EXT_OF_POSNOLOOKUP );

        /******************************************************************/
        /* write cache to disk                                            */
        /******************************************************************/
        MorphHashWrite( pLCB->POSHashNoLookup, pLCB->szBuffer );
      } /* endif */

      /******************************************************************/
      /* destroy hash tables                                            */
      /******************************************************************/
      MorphHashDestroy( pLCB->HashCache );
      MorphHashDestroy( pLCB->HashNoLookup );
      if ( pLCB->POSHashCache )    MorphHashDestroy( pLCB->POSHashCache );
      if ( pLCB->POSHashNoLookup ) MorphHashDestroy( pLCB->POSHashNoLookup );

      /******************************************************************/
      /* free language control block                                    */
      /******************************************************************/
      UtlAlloc( (PVOID *)&pLCB, 0L, 0L, NOMSG );

      /******************************************************************/
      /* Invalidate table entry                                         */
      /******************************************************************/
      *ppLCBTemp = 0;
    } /* endif */
  } /* endfor */

  return( usRC );

} /* endof MorphTerminate */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphCompIsolation
//------------------------------------------------------------------------------
// Description:       Returns all components of a compound word.
//                    The candidates are returned in form of a MORPH_OFFSLIST
//                    of a term list.
//                    :p.
//                    A term list is a series of null-terminated strings
//                    which is terminated by another null value.
//                    :p.
//                    The function is only supported for germanic languages.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphCompIsolation( SHORT sLanguageID,
//                                               PSZ   pszTerm,
//                                               PUSHORT pusBufferSize,
//                                               PSZ *ppTermList );
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    PSZ      pszTerm        pointer to input term
//------------------------------------------------------------------------------
// Output parameter:  PUSHORT  pusBufferSize  address of variable containing
//                                               size of term list buffer
//                    PVOID    *ppTermList    address of caller's term list
//                                               pointer
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           USHORT usListSize = 0;     // allocated list size
//                    PSZ    pszList = NULL;     // ptr to list area
//                    SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageID( "Deutsch(National)",
//                                               &sLangID);
//
//                    usRC = MorphCompIsolation( sLangID, "Deckblatt",
//                                               &usListSize, &pszList );
//
//                    after this call the area pointed to by pszList would
//                    contain the following term list:
//
//                    "Deck\Blatt\0\0"
//------------------------------------------------------------------------------
// Function flow:     check input data
//                    get language control block pointer
//                    get list of components of compound using language exit
//                    add returned terms to caller's term list
//                    terminate term list
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphCompIsolation
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszTermW,                  // pointer to input term
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermListW,              // address of caller's term list pointer
   ULONG    ulOemCP
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  PLCB       pLCB = NULL;              // pointer to language control block
  PSZ_W      pszTemp;                  // pointer for term pre-processing
                                                                /* 2@KIT0911A */
  PUCHAR     pucIsText;                // pointer to is-text character table

  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pszTermW      == NULL)    ||
       (pusBufferSize == NULL)    ||
       (ppTermListW   == NULL)    ||
       ((*ppTermListW == NULL) && (*pusBufferSize != 0) ) )
  {
    usRC = MORPH_INV_PARMS;
  } /* endif */

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

  /********************************************************************/
  /* get components of compound word                                  */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    if ( pLCB->fCompSep )
    {
      /****************************************************************/
      /* Check input term                                             */
      /****************************************************************/
                                                                /* 6@KIT0911A */
      if ( UtlQueryCharTable( IS_TEXT_TABLE, &pucIsText ) != NO_ERROR )
      {
        usRC = MORPH_FUNC_FAILED;
      }
      else
      {
        pszTemp = pszTermW;
        while ( *pszTemp && pucIsText[*pszTemp] )               /* 1@KIT0911C */
        {
          pszTemp++;
        } /* endwhile */

        if ( *pszTemp != EOS )
        {

          usRC = MORPH_NOT_FOUND;
        }
        else
        {
          PSZ   pTerm = NULL;
          PVOID pTermList = NULL;
          USHORT usBufferSize = 1024;
          UtlAlloc( (PVOID *)&pTerm, 0L, MAX_SEGMENT_SIZE, NOMSG );
          UtlAlloc( &pTermList, 0L, (LONG)usBufferSize, NOMSG );
          if ( pTerm && pTermList )
          {
            Unicode2ASCII( pszTermW, pTerm, ulOemCP );
            usRC = Compisol( pLCB->pvLangCB, pTerm, &usBufferSize,
                                         &pTermList, MORPH_ZTERMLIST );
            if (usRC == MORPH_OK)
            {
              ULONG ulSize = (ULONG) *pusBufferSize;
              ULONG ulUsed = 0L;
              usRC = MorphCopyTermListASCII2Unicode( (PSZ)pTermList, usBufferSize,
                                                    (PSZ_W *)ppTermListW,
                                                     &ulSize, &ulUsed,
                                                     MORPH_ZTERMLIST, ulOemCP);
              if (usRC == MORPH_OK)
              {
                if (ulSize < 0xFFF0)
                {
                  *pusBufferSize = (USHORT) ulSize;
                }
                else
                {
                  usRC = MORPH_NO_MEMORY;
                }
              }
            }
          }
          else
          {
            usRC = MORPH_NO_MEMORY;
          }
          UtlAlloc( (PVOID *)&pTerm, 0L, 0L, NOMSG );
          UtlAlloc( &pTermList, 0L, 0L, NOMSG );
        } /* endif */
      } /* endif */                                             /* 1@KIT0911A */
    }
    else
    {
      usRC = MORPH_FUNC_NOT_SUPPORTED;
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function MorphCompIsolation */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphMultStemForm      Convert all terms to stem form
//------------------------------------------------------------------------------
// Description:       MorphMultStemForm reduces all terms in the input data
//                    to stem form.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphMultStemForm( SHORT sLanguageID,
//                                              PSZ pszInData,
//                                              USHORT usBufferSize,
//                                              PSZ    pBuffer,
//                                              PUSHORT pusOrgRC );
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    PSZ      pszInData      pointer to input segment
//                    USHORT   usBufferSize   size of buffer for stem form term
//                    PSZ      pBuffer        address of output buffer
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     Get language control block pointer
//                    tokenize the input data using MorphTokenize
//                    loop over all terms in segment
//                       get stem form of term
//                       add term's stem form to stem form segment
//                       add delimiting character up to next term to
//                         stem form segment
//                    endloop
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphMultStemForm
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInData,                 // pointer to input segment
   USHORT   usBufferSize,              // size of buffer for stem form term
   PSZ_W    pBuffer,                    // address of output buffer
   ULONG    ulOemCP
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  PLCB       pLCB = NULL;              // pointer to language control block
  USHORT    usListLen = 0;             // length of list in bytes
  PULONG    pTermList = NULL;          // pointer to offset/length term list
  USHORT    usStemListSize = 0;        // current term list size
  PSZ_W     pStemList      = NULL;     // pointer to current term list
  CHAR_W    chTemp;                    // buffer for character values
  ULONG     ulBufferUsed = 0;          // bytes used of output buffer
  PLISTTERM pTerm;                     // ptr to current term in term list

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */


  /********************************************************************/
  /* tokenize the input data                                          */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphTokenizeW( sLanguageID, pszInData, &usListLen, (PVOID*)&pTermList,
                          MORPH_OFFSLIST, ulOemCP );
  } /* endif */

  /********************************************************************/
  /* build stem form reduced term                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    pTerm          = (PLISTTERM)pTermList;
    while ( (usRC == MORPH_OK) && (pTerm->ul != 0L) )
    {
       USHORT usTermOffs, usTermLength;  // values of current term
       PSZ_W  pucTerm;                   // ptr to current term

       // get values of current term (for easier processing)
       usTermOffs   = pTerm->OffsLen.usOffs;
       usTermLength = pTerm->OffsLen.usLength;
       pucTerm = pszInData + usTermOffs;

       /*************************************************************/
       /* terminate current term                                    */
       /*************************************************************/
       chTemp = pucTerm[usTermLength];
       pucTerm[usTermLength] = EOS;

       /*************************************************************/
       /* get stem form of term                                     */
       /*************************************************************/
       usRC = MorphGetStemForm( sLanguageID, pucTerm,
                                &usStemListSize, &pStemList, ulOemCP );

       /*************************************************************/
       /* restore end character of current term                     */
       /*************************************************************/
       pucTerm[usTermLength] = chTemp;

       /*************************************************************/
       /* add term or the stem form of the term to the stem form    */
       /* segment                                                   */
       /*************************************************************/
       if ( usRC == MORPH_OK )
       {
         /***********************************************************/
         /* add first stem form of term to stem form buffer         */
         /***********************************************************/
         ULONG ulStemLen = UTF16strlenCHAR(pStemList);
         if ( (ulBufferUsed + ulStemLen + 1) > usBufferSize )
         {
           usRC = MORPH_BUFFER_OVERFLOW;
         }
         else
         {
           memcpy( pBuffer + ulBufferUsed, pStemList, ulStemLen *sizeof(CHAR_W) );
           ulBufferUsed += UTF16strlenCHAR(pStemList);
         } /* endif */
       }
       else if ( usRC == MORPH_NOT_FOUND )
       {
         /***********************************************************/
         /* add term as-is to stem form segment                     */
         /***********************************************************/
         if ( (ulBufferUsed + usTermLength + 1) > usBufferSize )
         {
           usRC = MORPH_BUFFER_OVERFLOW;
         }
         else
         {
           memcpy( pBuffer + ulBufferUsed, pucTerm, usTermLength*sizeof(CHAR_W));
           ulBufferUsed += usTermLength;
           usRC = MORPH_OK;            // reset return code
         } /* endif */
       } /* endif */

       /*************************************************************/
       /* add delimiting characters up to next term if there is     */
       /* another term and if there are delimiting characters       */
       /* between the current term and the following one            */
       /*************************************************************/
       usTermOffs   = pTerm->OffsLen.usOffs;
       usTermLength = pTerm->OffsLen.usLength;
       pucTerm = pszInData + usTermOffs;
       if ( (pTerm+1)->ul != 0L)     // if there are more terms
       {
         ULONG ulDelimLength = (pTerm+1)->OffsLen.usOffs - usTermOffs -
                                usTermLength;
         if ( ulDelimLength )
         {
           if ( (ulBufferUsed + ulDelimLength + 1) > usBufferSize )
           {
             usRC = MORPH_BUFFER_OVERFLOW;
           }
           else
           {
             memcpy( pBuffer + ulBufferUsed, pucTerm + usTermLength,
                     ulDelimLength * sizeof(CHAR_W) );
             ulBufferUsed += ulDelimLength;
           } /* endif */
         } /* endif */
       } /* endif */

       /*************************************************************/
       /* continue with next term                                   */
       /*************************************************************/
       pTerm++;
    } /* endwhile terms in list */


    /****************************************************************/
    /* terminate stem form                                          */
    /****************************************************************/
    pBuffer[ulBufferUsed++] = EOS;
  } /* endif */

  /********************************************************************/
  /* Free memory of temporary term list and work buffers              */
  /********************************************************************/
  if ( pTermList )          UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
  if ( pStemList )          UtlAlloc( (PVOID *) &pStemList, 0L, 0L, NOMSG );

  return( usRC );

} /* endof MorphMultStemForm */


//------------------------------------------------------------------------------
//   General private functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphGetLCB         Get LCB pointer for a language ID
//------------------------------------------------------------------------------
// Description:       Check a given language ID and return the pointer to the
//                    language control block identified by the language ID
//------------------------------------------------------------------------------
// Function call:     MorphGetLCB( SHORT sLanguageID, PLCB *ppLCB )
//------------------------------------------------------------------------------
// Input parameter:   SHORT     sLanguageID   language identifier
//------------------------------------------------------------------------------
// Output parameter:  PLCB      *ppLCB        address of LCB pointr
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           usRC = MorphGetLCB( sLangID, &pLCB );
//------------------------------------------------------------------------------
// Function flow:     if language ID is in [0..255] and language pointer is set
//                      set callers pointer to language control block
//                    else
//                      set return code to MORPH_INV_LANG_ID
//                    endif
//                    return return code to calling function
//------------------------------------------------------------------------------
USHORT MorphGetLCB
(
  SHORT     sLanguageID,               // language identifier
  PLCB      *ppLCB                     // address of LCB pointr
)
{
  USHORT   usRC = MORPH_OK;            // function return code
  PLCB*    ppLCBTemp = ppIdToLCB + sLanguageID;

  if ( (sLanguageID >= 0)      &&
       (sLanguageID <= 255)    &&
       (*ppLCBTemp != 0) )
  {
    usRC = MORPH_OK;
    *ppLCB = *ppLCBTemp;
  }
  else
  {
    usRC = MORPH_INV_LANG_ID;
  } /* endif */

  return( usRC );

} /* end of function MorphGetLCB */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphAddTermToList  Add a term to a term list
//------------------------------------------------------------------------------
// Description:       Adds a term to the term list and enlarges the buffer
//                    for term list if required.
//
//                    Depending on usListType either the term is added to
//                    the list or the term offset and length is added to the
//                    list.
//------------------------------------------------------------------------------
// Function call:     MorphAddTermToList( PSZ *ppList, PUSHORT usSize,
//                                        PUSHORT usUsed,
//                                        PSZ pszTerm, USHORT usLength,
//                                        USHORT usOffs, USHORT usListType );
//------------------------------------------------------------------------------
// Input parameter:   PSZ      *ppList  ptr to term list pointer
//                    PUSHORT  pusSize  ptr to allocated size of term list
//                    PUSHORT  pusUsed  ptr to used bytes in term list
//                    PSZ      pszTerm  ptr to new term being added to list
//                    USHORT   usLength length of term
//                    USHORT   usOffs   offset of term
//                    ULONG    ulFlags  flags describing the term
//                    USHORT   usListType type of list MORPH_ZTERMLIST or
//                                        MORPH_OFFSLIST
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK        function completed successfully
//                    MORPH_NO_MEMORY memory allocation failed
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      term list buffer is enlarged if necessary
//------------------------------------------------------------------------------
// Samples:           PSZ      pList      = NULL;
//                    USHORT   usListSize = 0;
//                    USHORT   usListUsed = 0;
//                    PSZ      pszTerm    = "the new term";
//
//                    MorphAddTermToList( &pList, &usListSize, &usListUsed,
//                                        pszTerm, strlen(pszTerm) );
//------------------------------------------------------------------------------
// Function flow:     if term list is too small
//                      re-allocate a larger term list and adjust *pusSize
//                    endif
//                    copy term to end of term list and adjust *pusUsed
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphAddTermToList
(
  PSZ      *ppList,                    // ptr to term list pointer
  PUSHORT  pusSize,                    // ptr to allocated size of term list
  PUSHORT  pusUsed,                    // ptr to used bytes in term list
  PSZ      pszTerm,                    // ptr to new term being added to list
  USHORT   usLength,                   // length of term
  USHORT   usOffs,                     // offset of term
  ULONG    ulFlags,                    // flags describing the term
  USHORT   usListType                  // type of list
)
{
  USHORT   usRC = MORPH_OK;            // function return code
  ULONG    ulSize = (ULONG)*pusSize;
  ULONG    ulUsed = (ULONG)*pusUsed;

  usRC = MorphAddTermToList2( ppList, &ulSize, &ulUsed, pszTerm, usLength,
                              usOffs, ulFlags, usListType );

  // information may be lost - MorphAddTermToList2 has to be used for
  // term lists which may exceed the 64k limit!

  if ( ulSize > MAX_ALLOC )
  {
    usRC = MORPH_NO_MEMORY;
  }
  else
  {
    *pusUsed = (USHORT)ulUsed;
    *pusSize = (USHORT)ulSize;
  } /* endif */

  return( usRC );
} /* endof function MorphAddTermToList */

USHORT MorphAddTermToList2
(
  PSZ      *ppList,                    // ptr to term list pointer
  PULONG   pulSize,                    // ptr to allocated size of term list
  PULONG   pulUsed,                    // ptr to used bytes in term list
  PSZ      pszTerm,                    // ptr to new term being added to list
  USHORT   usLength,                   // length of term
  USHORT   usOffs,                     // offset of term
  ULONG    ulFlags,                    // flags describing the term
  USHORT   usListType                  // type of list
)
{
  USHORT   usRC = MORPH_OK;            // function return code
  ULONG    ulDataLength = 0;           // length of data to add
  BOOL     fOK  = TRUE;                // internal OK flag

  PSZ      pList = *ppList;            // ptr to term list
  PSZ      *pAnchor = NULL;            // ptr to anchor for list

  /********************************************************************/
  /* position to last block for large term lists                      */
  /********************************************************************/
  if ( usListType == MORPH_LARGE_ZTERMLIST )
  {
    PSZ   pNextList;

    // use caller's list pointer as default anchor
    pAnchor = ppList;

    if ( pList != NULL )
    {
      pNextList = *((PSZ *)pList);

      while ( pNextList != NULL )
      {
        pAnchor = (PSZ *)pList;
        pList = pNextList;
        pNextList = *((PSZ *)pList);
      } /* endwhile */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* get required size for new entry                                  */
  /********************************************************************/
  switch ( usListType )
  {
    case MORPH_OFFSLIST :
      /******************************************************************/
      /* space for offset and length of term plus end of term list      */
      /* indicator which is a zero offset and length value              */
      /******************************************************************/
      ulDataLength = 4 * sizeof(USHORT);
      break;

    case MORPH_ZTERMLIST :
      /******************************************************************/
      /* space for term, term delimiter and end of term list indicator  */
      /******************************************************************/
      ulDataLength = usLength + 1 + 1;
      break;

    case MORPH_FLAG_OFFSLIST :
      /******************************************************************/
      /* space for flag, offset and length of term plus end of term list*/
      /* indicator which is a zero flag, offset and length value        */
      /******************************************************************/
      ulDataLength = (2 * sizeof(ULONG)) + (4 * sizeof(USHORT));
      break;

    case MORPH_FLAG_ZTERMLIST :
      /******************************************************************/
      /* space for flag, term, term delimiter and end of term list      */
      /* indicator which is a empty flag and a term delimiter           */
      /******************************************************************/
      ulDataLength = sizeof(ULONG) + usLength + 1 + sizeof(ULONG) + 1;
      break;

    case MORPH_LARGE_ZTERMLIST :
      /******************************************************************/
      /* space for term, term delimiter and end of term list indicator  */
      /******************************************************************/
      ulDataLength = usLength + 1 + 1;
      break;

    default :
      fOK = FALSE;
      break;
  } /* endswitch */

  /********************************************************************/
  /* enlarge term list if necessary                                   */
  /********************************************************************/
  if ( fOK && ((*pulUsed + ulDataLength) > *pulSize) )
  {
    LONG lNewLen = ((LONG)*pulSize) +
                    (LONG)max( TERMLIST_INCR, ulDataLength );
    LONG lMaxLen = (usListType == MORPH_LARGE_ZTERMLIST) ?
                                          MORPH_LARGE_ZTERMLIST_LEN : MAX_ALLOC;

    if ( lNewLen >= lMaxLen )
    {
      // for large termlists allocate another buffer else set error condition
      if ( usListType == MORPH_LARGE_ZTERMLIST )
      {
        lNewLen = (LONG)sizeof(PSZ) + (LONG)max( TERMLIST_INCR, ulDataLength );
        *pulSize = 0;                  // no size yet
        pAnchor  = (PSZ *)pList;       // use current list as anchor
      }
// GQ: no error anymore as we support termlist larger than 64kB
//      else
//      {
//        usRC = MORPH_NO_MEMORY;
//        UtlAlloc( (PVOID *)ppList, 0L, 0L, NOMSG );
//        *ppList = NULL;
//        fOK = FALSE;
//      } /* endif */
    } /* endif */

    if ( fOK )
    {
      fOK = UtlAlloc( (PVOID *)&pList, (LONG) *pulSize, lNewLen, NOMSG );
      if ( fOK )
      {

         // set caller's termlist pointer
         if ( usListType == MORPH_LARGE_ZTERMLIST )
         {
           *pAnchor = pList;
           if (*pulSize == 0 )         // new block???
           {
             *pulUsed = sizeof(PSZ);   // leave room for pointer to next block
           } /* endif */
           *pulSize = (USHORT)lNewLen;
         }
         else
         {
           *pulSize += max( TERMLIST_INCR, ulDataLength );
           *ppList = pList;
         } /* endif */
      }
      else
      {
         usRC = MORPH_NO_MEMORY;

         // free the term list in case of erros
         if ( usListType == MORPH_LARGE_ZTERMLIST )
         {
           pList = *ppList;
           while ( pList != NULL )
           {
             PSZ pNextList = *((PSZ *)pList);
             UtlAlloc( (PVOID *)&pList, 0L, 0L, NOMSG );
             pList = pNextList;
           } /* endwhile */
         }
         else
         {
           UtlAlloc( (PVOID *)ppList, 0L, 0L, NOMSG );
         } /* endif */
         *ppList = NULL;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* add term to term list                                            */
  /********************************************************************/
  if ( fOK )
  {
    switch ( usListType )
    {
      case MORPH_OFFSLIST :
        *((PUSHORT)(pList + *pulUsed)) = usLength;
        *pulUsed += sizeof(USHORT);
        *((PUSHORT)(pList + *pulUsed)) = usOffs;
        *pulUsed += sizeof(USHORT);
        break;

      case MORPH_LARGE_ZTERMLIST :
      case MORPH_ZTERMLIST :
        if ( usLength )
        {
          memcpy( pList + *pulUsed, pszTerm, usLength );
          *pulUsed += usLength;
        } /* endif */
        *(pList + *pulUsed) = EOS;
        *pulUsed += 1;
        break;

      case MORPH_FLAG_OFFSLIST :
        *((PULONG)(pList + *pulUsed)) = ulFlags;
        *pulUsed += sizeof(ULONG);
        *((PUSHORT)(pList + *pulUsed)) = usLength;
        *pulUsed += sizeof(USHORT);
        *((PUSHORT)(pList + *pulUsed)) = usOffs;
        *pulUsed += sizeof(USHORT);
        break;

      case MORPH_FLAG_ZTERMLIST :
        *((PULONG)(pList + *pulUsed)) = ulFlags;
        *pulUsed += sizeof(ULONG);
        if ( usLength )
        {
          memcpy( pList + *pulUsed, pszTerm, usLength );
          *pulUsed += usLength;
        } /* endif */
        *(pList + *pulUsed) = EOS;
        *pulUsed += 1;
        break;

      default :
        break;
    } /* endswitch */
  } /* endif */

  /********************************************************************/
  /* return usRC to calling function                                  */
  /********************************************************************/
  return( usRC );

} /* end of function MorphAddTermToList2 */

USHORT MorphAddTermToList2W
(
  PSZ_W    *ppList,                    // ptr to term list pointer
  PULONG   pulSize,                    // alloc. size of term list in # of w's
  PULONG   pulUsed,                    // used # of w's in term list
  PSZ_W    pszTerm,                    // ptr to new term being added to list
  USHORT   usLength,                   // length of term in # of w's
  USHORT   usOffs,                     // offset of term in # of w's???
  ULONG    ulFlags,                    // flags describing the term
  USHORT   usListType                  // type of list
)
{
  USHORT   usRC = MORPH_OK;            // function return code
  ULONG    ulDataLenBytes = 0;         // length of data to add in # of bytes
  ULONG    ulDataLen = 0;              // length of data to add in w's
  BOOL     fOK  = TRUE;                // internal OK flag

  PBYTE    pList = (PBYTE)*ppList;     // ptr to term list
  PSZ_W    *pAnchor = NULL;            // ptr to anchor for list

  /********************************************************************/
  /* position to last block for large term lists                      */
  /********************************************************************/
  if ( usListType == MORPH_LARGE_ZTERMLIST )
  {
    PSZ_W  pNextList;

    // use caller's list pointer as default anchor
    pAnchor = ppList;

    if ( pList != NULL )
    {
      pNextList = *((PSZ_W *)pList);

      while ( pNextList != NULL )
      {
        pAnchor = (PSZ_W *)pList;
        pList = (PBYTE)pNextList;
        pNextList = *((PSZ_W *)pList);
      } /* endwhile */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* get required size for new entry                                  */
  /********************************************************************/
  switch ( usListType )
  {
    case MORPH_OFFSLIST :
      /******************************************************************/
      /* space for offset and length of term plus end of term list      */
      /* indicator which is a zero offset and length value              */
      /******************************************************************/
      ulDataLenBytes = 4 * sizeof(USHORT);
      break;

    case MORPH_ZTERMLIST :
      /******************************************************************/
      /* space for term, term delimiter and end of term list indicator  */
      /******************************************************************/
      ulDataLenBytes = (usLength + 1 + 1)*sizeof(CHAR_W);
      break;

    case MORPH_FLAG_OFFSLIST :
      /******************************************************************/
      /* space for flag, offset and length of term plus end of term list*/
      /* indicator which is a zero flag, offset and length value        */
      /******************************************************************/
      ulDataLenBytes = (2 * sizeof(ULONG)) + (4 * sizeof(USHORT));
      break;

    case MORPH_FLAG_ZTERMLIST :
      /******************************************************************/
      /* space for flag, term, term delimiter and end of term list      */
      /* indicator which is a empty flag and a term delimiter           */
      /******************************************************************/
      ulDataLenBytes = sizeof(ULONG) + (usLength + 1)*sizeof(CHAR_W)
                      + sizeof(ULONG) + sizeof(CHAR_W);
      break;

    case MORPH_LARGE_ZTERMLIST :
      /******************************************************************/
      /* space for term, term delimiter and end of term list indicator  */
      /******************************************************************/
      ulDataLenBytes = (usLength + 1 + 1)*sizeof(CHAR_W);
      break;

    default :
      fOK = FALSE;
      break;
  } /* endswitch */

  /********************************************************************/
  /* enlarge term list if necessary                                   */
  /********************************************************************/
  ulDataLen = ulDataLenBytes / sizeof(CHAR_W);        // # of w's needed
  if ( fOK && ((*pulUsed + ulDataLen) > *pulSize) )
  {
    LONG lOldLen = 0L;
    LONG lNewLen = ((LONG)*pulSize) +
                    (LONG)max( TERMLIST_INCR, ulDataLen );

    LONG lMaxLen = (usListType == MORPH_LARGE_ZTERMLIST) ?
                                          MORPH_LARGE_ZTERMLIST_LEN : MAX_ALLOC;
    lNewLen = lNewLen * sizeof(CHAR_W);         //  len in # of bytes
    lOldLen = (*pulSize) * sizeof(CHAR_W);
    if ( lNewLen >= lMaxLen )
    {
      // for large termlists allocate another buffer else set error condition
      if ( usListType == MORPH_LARGE_ZTERMLIST )
      {
        lNewLen = (LONG)sizeof(PSZ_W) +
                   (LONG)sizeof(CHAR_W) * max( TERMLIST_INCR, ulDataLen );
        *pulSize = 0;                  // no size yet
        lOldLen = 0L;
        pAnchor  = (PSZ_W *)pList;       // use current list as anchor
      }
    } /* endif */

    if ( fOK )
    {
      fOK = UtlAlloc( (PVOID *)&pList, lOldLen, lNewLen, NOMSG );
      if ( fOK )
      {  // set caller's termlist pointer
         if ( usListType == MORPH_LARGE_ZTERMLIST )
         {
           *pAnchor = (PSZ_W)pList;
           if (*pulSize == 0 )         // new block???
           {
             *pulUsed = sizeof(PSZ_W)/ sizeof(CHAR_W);   // leave room for pointer to next block
           } /* endif */
           *pulSize = lNewLen / sizeof(CHAR_W);
         }
         else
         {
           *pulSize = lNewLen / sizeof(CHAR_W);  //@@+= max( TERMLIST_INCR, ulDataLength );??
           *ppList = (PSZ_W)pList;
         } /* endif */
      }
      else
      {
         usRC = MORPH_NO_MEMORY;

         // free the term list in case of erros
         if ( usListType == MORPH_LARGE_ZTERMLIST )
         {
           pList = (PBYTE)*ppList;
           while ( pList != NULL )
           {
             PSZ_W pNextList = *((PSZ_W *)pList);
             UtlAlloc( (PVOID *)&pList, 0L, 0L, NOMSG );
             pList = (PBYTE)pNextList;
           } /* endwhile */
         }
         else
         {
           UtlAlloc( (PVOID *)ppList, 0L, 0L, NOMSG );
         } /* endif */
         *ppList = NULL;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* add term to term list (pList is a PBYTE ptr                      */
  /********************************************************************/
  if ( fOK )
  {
    ULONG  ulByteUsedInList = (*pulUsed) * sizeof(CHAR_W);
    switch ( usListType )
    {
      case MORPH_OFFSLIST :
        *((PUSHORT)(pList + ulByteUsedInList)) = usLength;
        *pulUsed += sizeof(USHORT)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);

        *((PUSHORT)(pList + ulByteUsedInList)) = usOffs;
        *pulUsed += sizeof(USHORT)/sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);
        break;

      case MORPH_LARGE_ZTERMLIST :
      case MORPH_ZTERMLIST :
        if ( usLength )
        {
          memcpy( (PBYTE)(pList + ulByteUsedInList),
                  (PBYTE) pszTerm, usLength*sizeof(CHAR_W) );
          *pulUsed += usLength;
          ulByteUsedInList += usLength * sizeof(CHAR_W);
        } /* endif */
        *((PSZ_W)(pList + ((*pulUsed) * sizeof(CHAR_W)))) = EOS;        /* 10-11-16 */
        *pulUsed += 1;
        ulByteUsedInList += sizeof(CHAR_W);
        break;

      case MORPH_FLAG_OFFSLIST :
        *((PULONG)(pList + ulByteUsedInList)) = ulFlags;
        *pulUsed += sizeof(ULONG)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(ULONG);

        *((PUSHORT)(pList + ulByteUsedInList)) = usLength;
        *pulUsed += sizeof(USHORT)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);

        *((PUSHORT)(pList + ulByteUsedInList)) = usOffs;
        *pulUsed += sizeof(USHORT) / sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);
        break;

      case MORPH_FLAG_ZTERMLIST :
        *((PULONG)(pList + ulByteUsedInList)) = ulFlags;
        *pulUsed += sizeof(ULONG)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(ULONG);

        if ( usLength )
        {
          memcpy( pList + ulByteUsedInList,
                 (PBYTE)pszTerm, usLength*sizeof(CHAR_W) );
          *pulUsed += usLength;
          ulByteUsedInList += usLength * sizeof(CHAR_W);
        } /* endif */
        *((PSZ_W)(pList + ulByteUsedInList)) = EOS;     /* 10-11-16 */
        *pulUsed += 1;
        break;

      default :
        break;
    } /* endswitch */
  } /* endif */

  /********************************************************************/
  /* return usRC to calling function                                  */
  /********************************************************************/
  return( usRC );

} /* end of function MorphAddTermToList2W */


USHORT MorphCopyTermListASCII2Unicode
(
  PSZ      pList,                      // ptr to ASCII termlist
  USHORT   usListSize,                 // size of List
  PSZ_W    *ppListW,                   // ptr to term list pointer
  PULONG   pulSize,                    // ptr to allocated size of term list
  PULONG   pulUsed,                    // ptr to used bytes in term list
  USHORT   usListType,                 // type of list
  ULONG    ulOemCP
)
{
  USHORT usRc = MORPH_OK;
  ULONG i;
  PSZ_W pListW;

  if ( *pulSize < usListSize * sizeof(CHAR_W) )
  {
    UtlAlloc((PVOID *) ppListW, *pulSize, usListSize * sizeof(CHAR_W), NOMSG);
    *pulSize = usListSize * sizeof( CHAR_W );
  }


  if ( ppListW )
  {
    ULONG j;
    pListW = *ppListW;

    switch ( usListType )
    {
      case MORPH_ZTERMLIST:           // zero-terminated term list
        while ( *pList )
        {
          i = strlen( pList )+1;
          j = ASCII2UnicodeBuf( pList, pListW, i, ulOemCP );
          pList += i;
          pListW += j;
        }
        *pListW++ = EOS;
        break;
      case MORPH_FLAG_ZTERMLIST:      // zero-terminated flagged term list
      case MORPH_LARGE_ZTERMLIST:     // zero-terminated large term list
        usRc = MORPH_FUNC_NOT_SUPPORTED;  // as far as we know, this function will not be called with this flags...
        break;

      case MORPH_OFFSLIST:            // offset/length term list
      case MORPH_FLAG_OFFSLIST:       // flagged offset/length term list
        memcpy( ppListW, pList, usListSize );
        break;
    }
    *pulUsed = pListW - *ppListW;
  }
  return usRc;
}

USHORT MorphCopyTermListUnicode2ASCII
(
  PSZ_W    pListW,                      // inputptr to unicode termlist
  USHORT   usListSize,                 // size of List
  PSZ     *ppList,                     // output to ASCII term list pointer
  PULONG   pulSize,                    // ptr to allocated size of term list
  PULONG   pulUsed,                    // ptr to used bytes in term list
  USHORT   usListType,                  // type of list
  ULONG    ulOemCP
)
{
  USHORT usRc = MORPH_OK;
  ULONG  i;
  ULONG  ulLenW;
  LONG   lUsed = 0;
  PSZ pTempList = NULL;

  if ( pListW )
  {
    if ( *pulSize < usListSize  )
    {
      UtlAlloc( (PVOID *)ppList, *pulSize, usListSize , NOMSG);
      *pulSize = usListSize ;
    }

    if ( ppList )
    {
      pTempList = *ppList;
      switch ( usListType )
      {
        case MORPH_ZTERMLIST:           // zero-terminated term list
          while ( *pListW )
          {
            i = UTF16strlenCHAR( pListW )+1;
            // take care, if the new ASCII string will not fit any more -- might happen in DBCS
            if (lUsed + 2 * i > usListSize )
            {
               UtlAlloc( (PVOID *)ppList, *pulSize, *pulSize+1000 , NOMSG);
               *pulSize += 1000 ;
                pTempList = *ppList;
                pTempList += lUsed;
            }
            ulLenW = Unicode2ASCIIBuf( pListW, pTempList, i, 2*i, ulOemCP );
            pTempList += ulLenW;
            lUsed += ulLenW;
            pListW += i;
          }
          *pTempList ++ = EOS;
          lUsed++;
          break;
        case MORPH_FLAG_ZTERMLIST:      // zero-terminated flagged term list
        case MORPH_LARGE_ZTERMLIST:     // zero-terminated large term list
          usRc = MORPH_FUNC_NOT_SUPPORTED;  // as far as we know, this function will not be called with this flags...
          break;

        case MORPH_OFFSLIST:            // offset/length term list
        case MORPH_FLAG_OFFSLIST:       // flagged offset/length term list
          memcpy(pListW, ppList, usListSize );
          break;
      }
      *pulUsed = pTempList - *ppList;
    }
  }
  else
  {
    *pulUsed = 0;
  }
  return usRc;
}

USHORT MorphCopyStringListASCII2Unicode
(
  PUCHAR   pList,                      // ptr to ASCII termlist
  LONG     lListSize,                 // size of List
  PSZ_W    *ppListW,                   // ptr to term list pointer
  PLONG    plSize,                    // ptr to allocated size of term list
  PLONG    plUsed,                    // ptr to used bytes in term list
  ULONG    ulOemCP
)

{
  USHORT usRc = MORPH_OK;
  ULONG i;
  ULONG ulLenW;
  PSZ_W pTempListW;

  if ( pList )
  {

    lListSize ++;
    if ( *plSize < (LONG)(lListSize  * sizeof(CHAR_W)) )
    {
      UtlAlloc( (PVOID *)ppListW, *plSize, lListSize * sizeof(CHAR_W), NOMSG);
      *plSize = lListSize * sizeof( CHAR_W );
    }

    if ( ppListW )
    {
      pTempListW = *ppListW;

      while ( *pList )
      {
        i = strlen( (PSZ)pList );
        ulLenW =   ASCII2UnicodeBuf( (PSZ)pList, pTempListW, i, ulOemCP );
        pList += i + 1;
        pTempListW += ulLenW + 1;
      }
      *plUsed = pTempListW - *ppListW;
    }
    else
    {
      usRc = MORPH_NO_MEMORY;
      *plUsed = 0;
    }
  }
  else
  {
    *plUsed = 0;
  }

  return usRc;
}

USHORT MorphCopyStringListUnicode2ASCII
(
  PSZ_W    pListW,                   // ptr to Unicode termlist
  LONG     lListSize,                // size of List
  PSZ     *ppList,                   // ptr to term list pointer
  PLONG    plSize,                   // ptr to allocated size of term list
  PLONG    plUsed,                    // ptr to used bytes in term list
  ULONG    ulOemCP
)

{
  USHORT usRc = MORPH_OK;
  LONG  i;
  LONG lUsed = 0;
  ULONG ulLenA;
  PSZ    pTempList;


  if ( pListW )
  {

    lListSize ++;
    if ( *plSize < lListSize)
    {
      UtlAlloc( (PVOID *)ppList, *plSize, lListSize , NOMSG);
      *plSize = lListSize ;
    }

    if ( ppList )
    {
      pTempList = *ppList;

      while ( *pListW )
      {
        i = UTF16strlenCHAR( pListW );
        // take care, if the new ASCII string will not fit any more -- might happen in DBCS
        if (lUsed + 2*i > *plSize )
        {
            UtlAlloc( (PVOID *)ppList, *plSize, *plSize+1000, NOMSG);
            *plSize += 1000;
            pTempList = *ppList;
            pTempList += lUsed;
        }
        ulLenA = Unicode2ASCIIBuf( pListW, pTempList ,i, (2*i), ulOemCP);
        pListW += i + 1;
        pTempList += ulLenA + 1;
        lUsed += ulLenA + 1;
      }
    }
    else
    {
      usRc = MORPH_NO_MEMORY;
      *plUsed = 0;
    }
  }
  else
  {
     *plUsed = 0;
  }
  return usRc;
}

//------------------------------------------------------------------------------
//
//  Morphologic Hash Functions (interface to low level hash functions)
//
//  Highlights:       - using fixed number of hash buckets which are
//                      pre-allocated during hash creation
//                    - free buckets are kept in a free-bucket-list
//                    - a last recently used chain is kept
//                    - if all buckets are in use, the least recently used
//                      bucked is freed and re-used for the ew entry
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashCreate     Create the hash
//------------------------------------------------------------------------------
// Description:       Creates a hash table for the morphologic functions.
//                    The supplied parameter is the size of the hash table
//                    in number of entries.
//------------------------------------------------------------------------------
// Function call:     MorphHashCreate( USHORT usMaxElements );
//------------------------------------------------------------------------------
// Input parameter:   USHORT   usMaxElements       size of hash table
//------------------------------------------------------------------------------
// Returncode type:   PMORPHHASH
//------------------------------------------------------------------------------
// Returncodes:       NULP             in case of errors
//                                     (error has been reported already)
//                    other            pointer to morph cache
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           PMORPHASH   pHash;
//
//                    pHash = MorphHashCreate( 500 );
//------------------------------------------------------------------------------
// Function flow:     allocate hash control area
//                    initialize hash control area
//                    allocate table for hash elements
//                    add all elements to free chain
//                    initialize recently-used chain
//                    create low level hash
//                    initialize string pool
//                    cleanup in case of errors
//                    return morph cache pointer or NULL
//------------------------------------------------------------------------------
PMORPHHASH MorphHashCreate
(
  ULONG        ulMaxElements               // size of hash table
)
{
  BOOL         fOK = TRUE;             // internal O.K. flag
  PMORPHHASH   pMorphHash = NULL;      // ptr to hash control area
  PHASHENTRY   pElement;               // pointer to hash elements
  ULONG        ulI;                    // general loop index

  /********************************************************************/
  /* Allocate hash control area                                       */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pMorphHash, 0L, (LONG)sizeof(MORPHHASH), ERROR_STORAGE );
  } /* endif */

  /********************************************************************/
  /* Initialize hash control area                                     */
  /********************************************************************/
  if ( fOK )
  {
    pMorphHash->ulMaxElements = min( ulMaxElements,
                                     (MAX_SEG_SIZE / sizeof(HASHENTRY)) );
    pMorphHash->ulElements  = 0;
  } /* endif */

  /********************************************************************/
  /* Allocate table for hash elements                                 */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pMorphHash->pElements, 0L, pMorphHash->ulMaxElements * sizeof(HASHENTRY), ERROR_STORAGE );
  } /* endif */

  /********************************************************************/
  /* Add all elements to free chain                                   */
  /********************************************************************/
  if ( fOK )
  {
    pMorphHash->ulFirstFree = END_OF_ELEMENTS;
    pElement = pMorphHash->pElements;
    for ( ulI = 0; ulI < pMorphHash->ulMaxElements; ulI++ )
    {
      pElement->ulKey  = ulI;
      pElement->ulNext = pMorphHash->ulFirstFree;
      pMorphHash->ulFirstFree = ulI;
      pElement++;
    } /* endfor */
  } /* endif */

  /********************************************************************/
  /* Initialize recently-used chain                                   */
  /********************************************************************/
  if ( fOK )
  {
    pMorphHash->ulFirstUsed = END_OF_ELEMENTS;
    pMorphHash->ulLastUsed  = END_OF_ELEMENTS;
  } /* endif */

  /********************************************************************/
  /* Create low level hash                                            */
  /********************************************************************/
  if ( fOK  )
  {
    pMorphHash->pHash = (PHASH)HashCreate( sizeof(HASHENTRY),
                                    pMorphHash->ulMaxElements * 2,
                                    (PFN_HASHVALUE)MorphHashKeyValue,
                                    (PFN_HASHCOMP)MorphHashCompare,
                                    (PFN_HASHFREE)MorphHashFree,
                                    (PVOID)pMorphHash );
    fOK = pMorphHash->pHash != NULL;
  } /* endif */

  /********************************************************************/
  /* Initialize string pool                                           */
  /********************************************************************/
  if ( fOK )
  {
    pMorphHash->StringPool = PoolCreate( INITIAL_POOL_SIZE );
    fOK = pMorphHash->StringPool != NULL;
  } /* endif */

  /********************************************************************/
  /* Cleanup in case of errors                                        */
  /********************************************************************/
  if ( !fOK )
  {
    if ( pMorphHash )
    {
      if ( pMorphHash->pElements )
      {
        UtlAlloc( (PVOID *)&pMorphHash->pElements, 0L, 0L, NOMSG );
      } /* endif */
      if ( pMorphHash->pHash )
      {
        HashDestroy( pMorphHash->pHash );
      } /* endif */
      if ( pMorphHash->StringPool )
      {
        PoolDestroy( pMorphHash->StringPool );
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Return pointer to morph cash or NULL in case of erros            */
  /********************************************************************/
  return( ( fOK ) ? pMorphHash : NULL );
} /* end of function MorphHashCreate */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashDestroy
//------------------------------------------------------------------------------
// Description:       Destroy a hash created using MorphHashCreate
//------------------------------------------------------------------------------
// Function call:     MorphHashDestroy( PMORPHHASH pHash );
//------------------------------------------------------------------------------
// Input parameter:   PMORPHHASH   pHash   pointer to hash control block
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       in any case
//------------------------------------------------------------------------------
// Prerequesits:      pHash must have been created using MorphHashCreate
//------------------------------------------------------------------------------
// Side effects:      the hash table is destroyed and all memory freed
//------------------------------------------------------------------------------
// Samples:           MorphHashDestroy( pHash );
//------------------------------------------------------------------------------
// Function flow:     destroy hash table
//                    destroy string pool
//                    free element table
//                    free morph hash control area
//------------------------------------------------------------------------------
BOOL MorphHashDestroy
(
  PMORPHHASH    pMorphHash             // pointer to morph hash structure
)
{
  /********************************************************************/
  /* Destroy hash table                                               */
  /********************************************************************/
  if ( pMorphHash->pHash )
  {
    HashDestroy( pMorphHash->pHash );
  } /* endif */

  /********************************************************************/
  /* Destroy string pool                                              */
  /********************************************************************/
  if ( pMorphHash->StringPool )
  {
    PoolDestroy( pMorphHash->StringPool );
  } /* endif */

  /********************************************************************/
  /* Free element table                                               */
  /********************************************************************/
  if ( pMorphHash->pElements )
  {
    UtlAlloc( (PVOID *)&pMorphHash->pElements, 0L, 0L, NOMSG );
  } /* endif */

  /********************************************************************/
  /* Free morph hash control area                                     */
  /********************************************************************/
  UtlAlloc( (PVOID *)&pMorphHash, 0L, 0L, NOMSG );

  return( TRUE );
} /* end of function MorphHashDestroy */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashReset
//------------------------------------------------------------------------------
// Description:       Remove all entries from a morphologic hash table
//------------------------------------------------------------------------------
// Function call:     MorphHashReset( PMORPHHASH pHash );
//------------------------------------------------------------------------------
// Input parameter:   PMORPHHASH   pHash   pointer to hash control block
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       in any case
//------------------------------------------------------------------------------
// Prerequesits:      pHash must have been created using MorphHashCreate
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           MorphHashReset( pHash );
//------------------------------------------------------------------------------
// Function flow:     call low level hash reset function
//                    destroy string pool
//                    create new string pool
//------------------------------------------------------------------------------
BOOL MorphHashReset
(
  PMORPHHASH    pMorphHash             // pointer to morph hash structure
)
{
  /********************************************************************/
  /* Reset hash table                                                 */
  /********************************************************************/
  HashReset( pMorphHash->pHash );

  /********************************************************************/
  /* Destroy string pool                                              */
  /********************************************************************/
  PoolDestroy( pMorphHash->StringPool );

  /********************************************************************/
  /* Create new (=empty) string pool                                  */
  /********************************************************************/
  if ( pMorphHash->pElements )
  {
    pMorphHash->StringPool = PoolCreate( INITIAL_POOL_SIZE );
  } /* endif */

  return( TRUE );
} /* end of function MorphHashReset */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashAdd        Add an entry to the morph hash
//------------------------------------------------------------------------------
// Description:       Adds a term and it's lemma list to the morph hash table.
//                    The term and the lemma list are stored in the string
//                    pool of the morph hash.
//------------------------------------------------------------------------------
// Function call:     MorphHashAdd( PMORPHHASH pMorphHash, PSZ pszTerm,
//                                  USHORT ulLemmaListLen, PSZ pszLemmaList );
//------------------------------------------------------------------------------
// Input parameter:   PMORPHHASH  pMorphHash     pointer to hash control block
//                    PSZ         pszTerm        name of term being added
//                    USHORT      ulLemmaListLen length of lemma list in bytes
//                    PSZ         pszLemmaList   lemma list for term or NULL
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       in any case
//------------------------------------------------------------------------------
// Prerequesits:      pHash must have been created using MorphHashCreate
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           PMORPHHASH pHash;
//
//                    pHash = MorphHashCreate( 500 );
//                    MorphHashAdd( pHash, "terms", 5, "term" );
//------------------------------------------------------------------------------
// Function flow:     check if term is already in cash
//                    if all elements of hash are in use then
//                      delete oldest element in cash
//                    endif
//                    add term and term data to string pool
//                    add new element to low level hash
//                    update recently used chain
//------------------------------------------------------------------------------
BOOL MorphHashAdd
(
  PMORPHHASH   pMorphHash,             // pointer to our hash control strucuture
  PSZ_W        pszTerm,                // name of term being added
  ULONG        ulLemmaListLen,         // length of lemma list in bytes
  PSZ_W        pszLemmaList            // lemma list for term or NULL
)
{
  HASHENTRY    HashEntry;              // buffer for new hash entry
  PHASHENTRY   pHashEntry;             // pointer to found/created hash entry
  ULONG        ulEntry;                // number (index) of new entry

  /********************************************************************/
  /* Check if term is already in cash                                 */
  /********************************************************************/
  HashEntry.pszTerm = pszTerm;
  pHashEntry = (PHASHENTRY) HashSearch( pMorphHash->pHash, &HashEntry );

  /********************************************************************/
  /* Remove oldest entry from cache if all cache elements are in use  */
  /********************************************************************/
  if ( !pHashEntry )                   // if not in cache yet ...
  {
    if ( pMorphHash->ulElements == pMorphHash->ulMaxElements  )
    {
      /****************************************************************/
      /* Cache is full: remove oldest element from cache              */
      /****************************************************************/
      MorphHashDelete( pMorphHash, pMorphHash->pElements + pMorphHash->ulFirstUsed );
      pHashEntry = NULL;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Add term and lemma list to string pool                           */
  /********************************************************************/
  if ( !pHashEntry )                   // if not in cache yet ...
  {
    HashEntry.pszTerm = PoolAddStringW( pMorphHash->StringPool, pszTerm );
    HashEntry.ulListLength = ulLemmaListLen;
    if ( pszLemmaList )
    {
      HashEntry.pLemmaList = (PSZ_W) PoolAddData( pMorphHash->StringPool,
                                         (ulLemmaListLen * sizeof(CHAR_W)),
                                          pszLemmaList );
    }
    else
    {
      HashEntry.pLemmaList = NULL;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Add new element to low level hash                                */
  /********************************************************************/
  if ( !pHashEntry )                   // if not in cache yet ...
  {
    /******************************************************************/
    /* Get entry from free chain                                      */
    /******************************************************************/
    ulEntry = pMorphHash->ulFirstFree;
    pHashEntry = pMorphHash->pElements + ulEntry;
    pMorphHash->ulFirstFree = pHashEntry->ulNext;
    pMorphHash->ulElements++;

    /******************************************************************/
    /* Fill-in element data                                           */
    /******************************************************************/
    pHashEntry->pszTerm      = HashEntry.pszTerm;
    pHashEntry->ulListLength = HashEntry.ulListLength;
    pHashEntry->pLemmaList   = HashEntry.pLemmaList;

    /******************************************************************/
    /* add element to end of recently used chain                      */
    /******************************************************************/
    pHashEntry->ulNext = END_OF_ELEMENTS;
    pHashEntry->ulPrev = pMorphHash->ulLastUsed;
    if ( pMorphHash->ulLastUsed != END_OF_ELEMENTS )
    {
      pMorphHash->pElements[pMorphHash->ulLastUsed].ulNext = pHashEntry->ulKey;
    } /* endif */
    pMorphHash->ulLastUsed = ulEntry;
    if ( pMorphHash->ulFirstUsed == END_OF_ELEMENTS )
    {
      pMorphHash->ulFirstUsed = ulEntry;
    } /* endif */

    /******************************************************************/
    /* Add element to low level hash                                  */
    /******************************************************************/
    HashAdd( pMorphHash->pHash, pHashEntry );
  } /* endif */

  return( TRUE );
} /* end of function MorphHashAdd */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashDelete     Delete an entry in the morph hash
//------------------------------------------------------------------------------
// Description:       Delete an entry from the hash table. The term and
//                    lemma list stay in the string pool.
//------------------------------------------------------------------------------
// Function call:     MorphHashDelete( PMORPHHASH pMorphHash, PSZ pszTerm );
//------------------------------------------------------------------------------
// Input parameter:   PMORPHHASH  pMorphHash     pointer to hash control block
//                    PSZ         pszTerm        name of term being added
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       in any case
//------------------------------------------------------------------------------
// Prerequesits:      pHash must have been created using MorphHashCreate
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           PMORPHHASH pHash;
//
//                    pHash = MorphHashCreate( 500 );
//                    MorphHashAdd( pHash, "terms", 5, "term" );
//                    MorphHashDelete( pHash, "terms" );
//------------------------------------------------------------------------------
// Function flow:     call low level hash delete function
//------------------------------------------------------------------------------
BOOL MorphHashDelete
(
  PMORPHHASH   pMorphHash,             // pointer to our hash control strucuture
  PHASHENTRY   pEntry                  // pointer to the entry being sdeleted
)
{

  // delete the hash entry, the callback function MorphHassFree will be called (which adjusts the recently used chain and the free entries chain)
  if ( HashDelete( pMorphHash->pHash, pEntry ) )
  {
    return( TRUE ); // nothing to do any more, MorphHashFree has done the house keeping
  } /* endif */

  // this part of the function is called when HashDelete failed to delete the entry,
  // we will have to remove the entry from the MorphHash table ...
  MorphHashFree( pEntry, (PVOID)pMorphHash );

  return( TRUE );
} /* end of function MorphHashDelete */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashSearch     Search an entry in the morph hash
//------------------------------------------------------------------------------
// Description:       Searchs for an entry in the hash table. If the entry is
//                    found and ppszLemmaList is not NULL. The term lemma list
//                    pointer and the lemma list length is set.
//------------------------------------------------------------------------------
// Function call:     MorphHashSearch( PMORPHHASH pMorphHash, PSZ pszTerm,
//                                     PUSHORT pulLemmaListLen,
//                                     PSZ *ppszLemmaList );
//------------------------------------------------------------------------------
// Input parameter:   PMORPHHASH  pMorphHash     pointer to hash control block
//                    PSZ         pszTerm        name of term being added
//------------------------------------------------------------------------------
// Output parameter:  PUSHORT     pulLemmaListLen length of lemma list in bytes
//                    PSZ         *ppszLemmaList  ptr to lemma list pointer or
//                                                NULL if no lemma list is
//                                                requested
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       if term is in hash table
//                    FALSE      if term is not in hash table
//------------------------------------------------------------------------------
// Prerequesits:      pHash must have been created using MorphHashCreate
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           PMORPHHASH pHash;
//                    USHORT     usListLength;
//                    PSZ        pszList;
//
//                    pHash = MorphHashCreate( 500 );
//
//                    MorphHashAdd( pHash, "terms", 5, "term" );
//
//                    MorphHashSearch( pHash, "terms", &usListLength,
//                                     &pszList );
//                      ==> TRUE, usListLength=5, pszList="term"
//
//                    MorphHashAdd( pHash, "cars", &usListLength,
//                                     &pszList );
//                      ==> FALSE, usListLength=0, pszList=undefined
//------------------------------------------------------------------------------
// Function flow:     call the low level hash search function
//                    update recently used chain
//                    return result to caller
//------------------------------------------------------------------------------
BOOL MorphHashSearch
(
  PMORPHHASH   pMorphHash,             // pointer to our hash control strucuture
  PSZ_W        pszTerm,                // name of term being added
  PULONG       pulLemmaListLen,        // ptr to variable receiving the
                                       //      length of lemma list
  PSZ_W        *ppszLemmaList          // ptr to variable receiving the pointer
                                       //      to the lemma list
)
{
  HASHENTRY    HashEntry;              // buffer for hash entry to be searched
  PHASHENTRY   pHashEntry;             // pointer to found/created hash entry

  /********************************************************************/
  /* Search term in cache                                             */
  /********************************************************************/
  HashEntry.pszTerm = pszTerm;
  pHashEntry = (PHASHENTRY)HashSearch( pMorphHash->pHash, &HashEntry );

  /********************************************************************/
  /* if found, return data of element and update recently used chain  */
  /********************************************************************/
  if ( pHashEntry )
  {
    /******************************************************************/
    /* return data of found element                                   */
    /******************************************************************/
    if ( pulLemmaListLen )
    {
      *pulLemmaListLen = pHashEntry->ulListLength;
    } /* endif */
    if ( ppszLemmaList )
    {
      *ppszLemmaList   = pHashEntry->pLemmaList;
    } /* endif */

    /******************************************************************/
    /* remove element from recently used chain                        */
    /******************************************************************/
    if ( pHashEntry->ulPrev != END_OF_ELEMENTS )
    {
      pMorphHash->pElements[pHashEntry->ulPrev].ulNext = pHashEntry->ulNext;
    }
    else
    {
      pMorphHash->ulFirstUsed = pHashEntry->ulNext;
    } /* endif */
    if ( pHashEntry->ulNext != END_OF_ELEMENTS )
    {
      pMorphHash->pElements[pHashEntry->ulNext].ulPrev = pHashEntry->ulPrev;
    }
    else
    {
      pMorphHash->ulLastUsed = pHashEntry->ulPrev;
    } /* endif */

    /******************************************************************/
    /* add element to end of recently used chain                      */
    /******************************************************************/
    pHashEntry->ulNext = END_OF_ELEMENTS;
    pHashEntry->ulPrev = pMorphHash->ulLastUsed;
    if ( pMorphHash->ulLastUsed != END_OF_ELEMENTS )
    {
      pMorphHash->pElements[pMorphHash->ulLastUsed].ulNext = pHashEntry->ulKey;
    } /* endif */
    pMorphHash->ulLastUsed = pHashEntry->ulKey;
    if ( pMorphHash->ulFirstUsed == END_OF_ELEMENTS )
    {
      pMorphHash->ulFirstUsed = pHashEntry->ulKey;
    } /* endif */
  } /* endif */

  return( pHashEntry != NULL );
} /* end of function MorphHashSearch */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashWrite      Write contents of hash to disk
//------------------------------------------------------------------------------
// Description:       Writes the contents of a hash table to disk.
//
//                    The format of the entries written to disk is
//                    +--------?-----??-------------?---------+
// Function call:     MorphHashWrite( PMORPHHASH pMorphHash, PSZ pszFileName );
//------------------------------------------------------------------------------
//                     usRecLen  term  0  usListLength  lemma list
//                                             or
//                                           usPOS
//                    +--------?-----??-------------?---------+
//
//------------------------------------------------------------------------------
// Input parameter:   PMORPHHASH  pMorphHash     pointer to hash control block
//                    PSZ         pszFileName    name of output file
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       hash table successful written to disk
//                    FALSE      write of hash table failed
//------------------------------------------------------------------------------
// Prerequesits:      pHash must have been created using MorphHashCreate
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     open the output file
//                    write file header
//                    loop over all elements in recently used chaing
//                      write record length field
//                      write term
//                      write lemma list if any
//                    endloop
//                    close output file
//------------------------------------------------------------------------------
BOOL MorphHashWrite
(
  PMORPHHASH    pMorphHash,            // pointer to morph hash structure
  PSZ           pszFileName            // name of output file
)
{
  USHORT       usRC = MORPH_OK;        // function return code
  PBUFCB       pBufCB;                 // control block for buffered output
  BOOL         fOK   = TRUE;           // internal O.K. flag
  USHORT       usDosRC;                // return code of DosXXX calls
  PHASHENTRY   pElement;               // pointer to current hash element
  ULONG        ulElement;              // index of current hash element
  ULONG        ulRecLen;               // length of current record

  /********************************************************************/
  /* Open output file                                                 */
  /********************************************************************/
  usDosRC = UtlBufOpen( &pBufCB, pszFileName, BUFFER_SIZE, FILE_CREATE, FALSE );
  fOK = ( usDosRC == MORPH_OK );

  /********************************************************************/
  /* Loop over all elements in recently used chain and write the      */
  /* elements to disk                                                 */
  /********************************************************************/
  if ( fOK )
  {
	USHORT usTmpLen = 0;
    ulElement = pMorphHash->ulFirstUsed;
    while ( ulElement != END_OF_ELEMENTS )
    {

      pElement = pMorphHash->pElements + ulElement;

      /****************************************************************/
      /* write record length field                                                       */
      /****************************************************************/
      ulRecLen = UTF16strlenBYTE(pElement->pszTerm) + sizeof(CHAR_W) + sizeof(USHORT);
      if ( pElement->pLemmaList )
      {
        ulRecLen += pElement->ulListLength * sizeof(CHAR_W);
      } /* endif */

      usTmpLen = (USHORT)ulRecLen;
      usRC = UtlBufWrite( pBufCB, (PSZ)&usTmpLen, sizeof(USHORT), FALSE );

      /****************************************************************/
      /* write term and length of lemma list                          */
      /****************************************************************/
      usRC = UtlBufWriteW( pBufCB, pElement->pszTerm,
                          (USHORT)(UTF16strlenBYTE(pElement->pszTerm) + sizeof(CHAR_W)), FALSE );

      if ( !usRC )
      {
        usRC = UtlBufWrite( pBufCB, (PSZ)&pElement->ulListLength,
                            sizeof(USHORT), FALSE );
      } /* endif */

      /****************************************************************/
      /* write lemma list if any                                      */
      /****************************************************************/
      if ( !usRC )
      {
        if ( pElement->pLemmaList )
        {
          usRC = UtlBufWriteW( pBufCB, pElement->pLemmaList,
                              (USHORT)(pElement->ulListLength * sizeof(CHAR_W)), FALSE );
        } /* endif */
      } /* endif */
      ulElement = pElement->ulNext;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Close output file                                                */
  /********************************************************************/
  if ( pBufCB ) UtlBufClose( pBufCB, FALSE );

  return( fOK );
} /* end of function MorphHashWrite */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashRead      Build a hash table from a disk file
//------------------------------------------------------------------------------
// Description:       Fills a hash table from a disk file produced by
//                    MorphHashWrite.
//
//                    The entries read from disk are added to the hash table in
//                    the same order as they appear in the disk file.
//
//                    Incomplete entries or corrupted entries are ignored.
//------------------------------------------------------------------------------
// Function call:     MorphHashRead( PMORPHHASH pMorphHash, PSZ pszFileName );
//------------------------------------------------------------------------------
// Input parameter:   PMORPHHASH  pMorphHash     pointer to hash control block
//                    PSZ         pszFileName    name of input file
//                    BOOL        fHasLemmaList  has-a-lemma-list flag
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       hash table successful read from disk
//                    FALSE      read of hash table failed
//------------------------------------------------------------------------------
// Prerequesits:      pHash must have been created using MorphHashCreate
//                    pszFileName must be the name of a file created using
//                       MorphHashWrite
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     allocate string buffer
//                    open the input file
//                    while no error condition raised
//                      read record length
//                      read record
//                      get pointer to lemma list and length of lemma list
//                      add record data to hash table
//                    endwhile
//                    close input file and free allocated memory
//------------------------------------------------------------------------------
BOOL MorphHashRead
(
  PMORPHHASH    pMorphHash,            // pointer to morph hash structure
  PSZ           pszFileName,           // name of input file
  BOOL          fHasLemmaList          // has-a-lemma-list flag
)
{
  PBUFCB       pBufCB = NULL;          // control block for buffered output
  BOOL         fOK;                    // internal O.K. flag
  USHORT       usDosRC;                // return code of DosXXX calls
  PSZ_W        pStringBuffer;          // pointer to buffer for strings
  ULONG        ulBytesRead;            // # of bytes read from file
  USHORT       usRecLen;               // length of disk record
  ULONG        ulLemmaListLength = 0;      // length of lemma list in bytes
  PSZ_W        pszLemmaList = NULL;    // ptr to lemma list

  /********************************************************************/
  /* Allocate string buffer                                           */
  /********************************************************************/
  fOK = UtlAlloc( (PVOID *)&pStringBuffer, 0L,
                          (LONG)BUFFER_SIZE *sizeof(CHAR_W), ERROR_STORAGE );

  /********************************************************************/
  /* Open the input file                                              */
  /********************************************************************/
  if ( fOK )
  {
     usDosRC = UtlBufOpen( &pBufCB, pszFileName, BUFFER_SIZE,
                           FILE_OPEN, FALSE );
     fOK = ( usDosRC == MORPH_OK );
  } /* endif */

  /********************************************************************/
  /* Read all elements in disk file and add the elements to the hash  */
  /* table                                                            */
  /********************************************************************/
  while ( fOK )
  {
    /******************************************************************/
    /* Read record length                                             */
    /******************************************************************/
    usDosRC = UtlBufRead( pBufCB,
                          (PSZ)&usRecLen,
                          sizeof(USHORT),
                          &ulBytesRead,
                          FALSE );
    if ( (usDosRC != MORPH_OK) ||
         (ulBytesRead != sizeof(USHORT)) ||
         (usRecLen > BUFFER_SIZE) )
    {
      fOK = FALSE;
    } /* endif */

    /******************************************************************/
    /* Read record                                                    */
    /******************************************************************/
    if ( fOK )
    {
      usDosRC = UtlBufRead( pBufCB,
                            (PSZ)pStringBuffer,
                            usRecLen,
                            &ulBytesRead,
                            FALSE );
      fOK = ( usDosRC == NO_ERROR ) && ( ulBytesRead == (ULONG)usRecLen );
    } /* endif */

    /******************************************************************/
    /* Get pointer to lemma list and length of lemma list             */
    /******************************************************************/
    if ( fOK )
    {
      pszLemmaList = pStringBuffer + UTF16strlenCHAR(pStringBuffer) + 1;
      ulLemmaListLength = *((PUSHORT)pszLemmaList);
      pszLemmaList++;                  // skip lemma list length field (sizeof(USHORT))

      if ( !ulLemmaListLength || !fHasLemmaList )
      {
        pszLemmaList      = NULL;
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* Add record data to hash table                                  */
    /******************************************************************/
    if ( fOK )
    {
      fOK = MorphHashAdd( pMorphHash, pStringBuffer,
                          ulLemmaListLength, pszLemmaList );
    } /* endif */
  } /* endwhile */

  /********************************************************************/
  /* Close input file and free allocated memory                       */
  /********************************************************************/
  if ( pBufCB )         UtlBufClose( pBufCB, FALSE );
  if ( pStringBuffer )  UtlAlloc( (PVOID *)&pStringBuffer, 0L, 0L, NOMSG );

  return( fOK );
} /* end of function MorphHashRead */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashCompare    Compare two hash entries
//------------------------------------------------------------------------------
// Description:       Compare the terms of two hashentries and return the
//                    result.
//                    This function is used by the low level hash functions to
//                    compare two has entries.
//------------------------------------------------------------------------------
// Function call:     MorphHashCompare( PHASHENTRY pEntry1, PHASHENTRY pEntry2)
//------------------------------------------------------------------------------
// Input parameter:   PHASHENTRY  pEntry1     pointer to first hash entry
//                    PHASHENTRY  pEntry2     pointer to second hash entry
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       -1   if term in pEntry1 is smaller than term in pEntry2
//                     0   if term in pEntry1 is equal to term in pEntry2
//                     1   if term in pEntry1 is greater than term in pEntry2
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     return result of strcmp function
//------------------------------------------------------------------------------
SHORT MorphHashCompare
(
  PHASHENTRY pEntry1,
  PHASHENTRY pEntry2,
  PVOID      pUserPtr
)
{
  pUserPtr;                            // avoid compiler warning

  return (SHORT)(UTF16strcmp( pEntry1->pszTerm, pEntry2->pszTerm ));
} /* end of function MorphHashCompare */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashFree       Free a hash entry
//------------------------------------------------------------------------------
// Description:       Is called by the low level hash functions to free
//                    a hash element.
//                    The hash elements have been allocated in one memory
//                    block, so individual elements can't be freed. Freed
//                    elements are chained to the free element list instead.
//------------------------------------------------------------------------------
// Function call:     MorphHashFree( PHASHENTRY pEntry );
//------------------------------------------------------------------------------
// Input parameter:   PHASHENTRY pEntry     pointer to entry to be freed
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       TRUE    in any case
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     get pointer to morph hash control block
//                    remove deleted element from recently used chain
//                    add element to free list
//                    return TRUE
//------------------------------------------------------------------------------
SHORT MorphHashFree( PHASHENTRY pEntry, PVOID pUserPtr )
{
  PMORPHHASH    pMorphHash;            // pointer to morph hash structure

  /********************************************************************/
  /* Get pointer to morph hash control structure                      */
  /********************************************************************/
  pMorphHash = (PMORPHHASH)pUserPtr;

  /********************************************************************/
  /* Remove element from recently used chain                          */
  /********************************************************************/
  if ( pEntry->ulPrev != END_OF_ELEMENTS )
  {
    pMorphHash->pElements[pEntry->ulPrev].ulNext = pEntry->ulNext;
  }
  else
  {
    pMorphHash->ulFirstUsed = pEntry->ulNext;
  } /* endif */
  if ( pEntry->ulNext != END_OF_ELEMENTS )
  {
    pMorphHash->pElements[pEntry->ulNext].ulPrev = pEntry->ulPrev;
  }
  else
  {
    pMorphHash->ulLastUsed = pEntry->ulPrev;
  } /* endif */

  /********************************************************************/
  /* Add element to free list                                         */
  /********************************************************************/
  pEntry->ulNext = pMorphHash->ulFirstFree;
  pMorphHash->ulFirstFree = pEntry->ulKey;
  pMorphHash->ulElements--;

  return( TRUE );
} /* end of function MorphHashFree */



/* Two random permutations of the numbers 0..255                             */
static BYTE random1[256] = {
149, 184,  28, 173, 213,  98,  14,  87, 157,  78,  31,  62, 226, 165, 120, 143,
251, 233, 227, 122, 146,  20, 106,  92, 197, 148, 102, 241,  29, 218,  69, 228,
153,  70, 249, 204, 214, 219, 234, 195, 152, 217,  65, 254,   8,  73, 119, 230,
206, 244, 246, 248, 216,  39, 128,  55,  75, 123, 189, 155,  38, 232,  99, 116,
118, 104, 160, 190, 215,  58,  54, 242,  72,  57,   7, 202, 192,  12,  96,  43,
 95, 177, 193,  51, 127, 247,  19,  40,  34, 147, 212,  42, 203, 239, 112,  93,
208, 201, 109,  90, 176,  59, 223,  94, 133, 111, 130,   5,  67, 222,  22, 209,
131, 169,  15, 174,  79,   6,  49,  46,   3, 142,  63, 107, 154, 182, 224, 186,
168,  60, 181, 125, 207,  52, 166,  45,  41, 188, 164, 240, 185, 161, 200,  97,
255, 250,   2,  89, 158,  30,  53,  21,  44, 235,  23, 187, 198, 205,  88, 121,
162,  11,  25, 170,  50,   4, 211, 243,   0,  37,  24, 196, 144, 129,  91,  86,
 76,  26,  18, 220, 237, 210, 194, 136, 156, 100,  27, 229, 137, 101, 236, 191,
221, 159,  35, 126,  68,  71,  10, 183, 179, 110, 150,  36, 151,  85,  82, 114,
  9,  81, 124, 172,  66,   1,  83,  77, 252, 105, 145,  13,  64, 238, 163, 140,
180, 113, 178, 108,  80,  56, 225, 134, 245,  32, 103,  74,  33,  61, 199, 117,
253, 231,  48,  47,  16, 141, 115, 167, 138,  84, 135, 139,  17, 132, 175, 171
};

static BYTE random2[256] = {
247,  72, 239, 215,  34, 179,  66,  63, 229, 107, 127, 226,  96, 105, 110, 197,
 27, 241, 251,   7, 146,  44, 230, 238, 153,  86, 104, 155, 163, 130,  55, 195,
240, 149,  73,  85, 196,   8,  98, 253, 208, 212, 102,  53, 165, 124, 128,  70,
174, 111, 151,  10, 114, 125, 220, 211,  93, 106,  54,  30, 156, 250, 122,  41,
252, 170, 232, 100,  28,   6,  81, 200, 168,  91, 249,  90,  84, 219, 244,  19,
193,  38,  18,  32,  79,  71, 227,  83,  47, 213,  75,  15,  22,  21,  45, 236,
 43, 180,  76, 248,  24, 234, 205,  35,  77, 184, 235, 202,  57,  69,  29, 203,
109, 167,  89, 135, 190, 123, 141, 217, 164,  94, 255,  65,  92, 150,   3, 152,
192, 112, 218,  39,  61,  74, 214, 204, 148,  62, 103, 198, 187, 172, 166,   4,
 16,  88,  46, 224,  56,  87,  36,  99,  95, 126, 185, 201,  50,  20, 160,  67,
242, 245, 133, 101, 158, 145, 154,  78, 136, 132,  49, 225,  80, 188,  48, 237,
 40, 228,  11, 143, 231, 221,  26, 194,   0, 243, 129, 171, 108, 161,  12, 209,
 25, 182, 137,  33, 189,  97, 254,  68, 162, 176, 113,  58, 147, 246,  31,  51,
121,  23, 186,  13, 178,  64,  14,  37, 216, 142,   1, 223, 183,   5,  59,  42,
119,  17, 144, 199,  52, 131, 157, 134, 207, 117, 120,   9, 222, 233, 210, 191,
159, 116, 181, 169, 206, 175,  60, 140, 139, 173, 118, 177, 115, 138,   2,  82
};

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashKeyValue   Get hash value for an entry
//------------------------------------------------------------------------------
// Description:       Computes the hash value of an entry by 'OR'ing the
//                    bytes which make up the term of the entry. The bytes
//                    are converted using two random tables.
//                    The returned value will be in the range from
//                    0 to 65535.
//------------------------------------------------------------------------------
// Function call:     MorphHashKeyValue( usSize, pEntry );
//------------------------------------------------------------------------------
// Input parameter:   USHORT     usSize       size of hash table
//                    PHASHENTRY pEntry       point to hash entry
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       any          hash value for entry
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     while not end of term
//                      exor table1 value of character to hash1 value
//                      exor table2 value of character to hash2 value
//                      go to next character
//                    endwhile
//                    return ushort from hash1 and hash2 value
//------------------------------------------------------------------------------
USHORT MorphHashKeyValue
(
  ULONG     ulSize,                   // size of hash table
  PHASHENTRY pEntry                    // point to hash entry
)
{
  PSZ_W  pbTerm = pEntry->pszTerm;
  USHORT hash1 = 0;
  USHORT hash2 = 0;

  ulSize;                              // avoid compiler warning

  while ( *pbTerm )
  {
    // GQ 2018/05/09: The following code is not Unicode enabled, index into  random tables had to be truncated to 256 (BYTE value) 
    hash1 = random1[(BYTE)(hash1 ^ *pbTerm) ]; 
    hash2 = random2[(BYTE)(hash2 ^ *pbTerm++) ];
  }
  return (( hash1 << 8) | hash2);
} /* end of function MorphHashKeyValue */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphRefreshDicts   Refresh dicts of other tasks
//------------------------------------------------------------------------------
// Description:       Refreshes addenda and or abbreviation dicitonaries of
//                    other tasks.
//------------------------------------------------------------------------------
// Function call:     MorphRefreshDicts( pLCB, usDictType );
//------------------------------------------------------------------------------
// Input parameter:   PLCB       pLCB         pointer to current control block
//                    USHORT     usDictType   type of dictionary
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
USHORT MorphRefreshDicts
(
  PLCB       pLCB,                     // pointer to current control block
  USHORT     usDictType                // type of dictionary
)
{
  SHORT       sI;                      // loop index
  USHORT      usRC = MORPH_OK;         // function return code
  PLCB*       ppLCBTemp;

  /********************************************************************/
  /* Scan our list of active language control blocks...               */
  /********************************************************************/
  ppLCBTemp = ppIdToLCB;
  for ( sI = 0; sI < 256, *ppLCBTemp != 0; sI++, ppLCBTemp++ )
  {
    PLCB   pTempLCB;                 // pointer to language control block

    pTempLCB = *ppLCBTemp;

    if ( pTempLCB != pLCB )
    {
      /**************************************************************/
      /* Refresh addenda or abbreviation if it is the one which has */
      /* been modified                                              */
      /* Refresh is done using the refresh mode call of build       */
      /* dict                                                       */
      /**************************************************************/
      if ( usDictType == ADDENDA_DICT )
      {
        if ( strcmp( pLCB->szAddendaDict, pTempLCB->szAddendaDict ) == 0 )
        {
          pTempLCB->fAddendaDirty = TRUE;
        } /* endif */
      }
      else if ( usDictType == ABBREV_DICT )
      {
        if ( strcmp( pLCB->szMorphDict, pTempLCB->szMorphDict ) == 0 )
        {
          pTempLCB->fAbbrevDirty = TRUE;
        } /* endif */
      }
      else
      {
        /************************************************************/
        /* ignore unknown dictionary type                           */
        /************************************************************/
      } /* endif */
    } /* endif */
  } /* endfor */
  return( usRC );
} /* end of function MorphRefreshDicts */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphCheck4Refresh  Check if refresh is required
//------------------------------------------------------------------------------
// Description:       If the "dirty" flag is set for an addenda or abbreviation
//                    dictionary, the dictionary is refreshed and the
//                    dirty flag is cleared.
//------------------------------------------------------------------------------
// Function call:     MorphRefreshDicts( pLCB );
//------------------------------------------------------------------------------
// Input parameter:   PLCB       pLCB         pointer to current control block
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
USHORT MorphCheck4Refresh
(
  PLCB       pLCB                      // pointer to current control block
)
{
  USHORT      usRC = MORPH_OK;         // function return code

  if ( pLCB->fAddendaDirty )
  {
    usRC = BuildDict( pLCB->pvLangCB, ADDENDA_DICT, 0, NULL, 0 );
    pLCB->fAddendaDirty = FALSE;
  } /* endif */

  if ( (usRC == MORPH_OK) && pLCB->fAbbrevDirty )
  {
    usRC = BuildDict( pLCB->pvLangCB, ABBREV_DICT, 0, NULL, 0 );
    pLCB->fAbbrevDirty = FALSE;
  } /* endif */

  return( usRC );
} /* end of function MorphCheck4Refresh */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphGetLanguageString       Get language from ID
//------------------------------------------------------------------------------
// Description:       Get the language from given ID.
//------------------------------------------------------------------------------
// Function call:     MorphGetLanguageString(SHORT sLanguageID,PSZ pszLanguage)
//------------------------------------------------------------------------------
// Input parameter:   SHORT   sLanguageID         id of the language
//------------------------------------------------------------------------------
// Output parameter:  PSZ     pszLanguage         language name as used during
//                                                MorphGetLanguageID
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID has to be retrieved by MorphGetLanguageID
//                    pszLanguage has to be large enough to contain language
//                                name
//------------------------------------------------------------------------------
// Side effects:      NONE
//------------------------------------------------------------------------------
// Samples:           SHORT  sLangID;            // language ID
//                    CHAR   chLang[ 40 ];
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID);
//                    if (!usRC)
//                      usRC = MorphGetLanguageID( sLangID, chLang );
//
//------------------------------------------------------------------------------
// Function flow:     search id in language table
//                    if found
//                      fill pszLang
//                      return
//------------------------------------------------------------------------------
USHORT MorphGetLanguageString
(
  SHORT  sLanguageID,                  // language id
  PSZ    pszLanguage                   // name of language
)
{
  USHORT   usRC = MORPH_OK;            // function returncode
  PLCB*    ppLCBTemp = ppIdToLCB + sLanguageID;
  
  if (  (sLanguageID >= 0) && (sLanguageID < 256) )
  {
    PLCB pLCB = *ppLCBTemp;
    strcpy( pszLanguage, pLCB->szLanguage );
  }
  else
  {
    usRC = MORPH_INV_PARMS;
    *pszLanguage = EOS;
  } /* endif */

  /********************************************************************/
  /* Return function return code to caller                            */
  /********************************************************************/
  return( usRC );

} /* end of function MorphGetLanguageString */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphGetLanguageType    Get type of language
//------------------------------------------------------------------------------
// Description:       Returns type of language: DBCS, BIDI or other
//------------------------------------------------------------------------------
// Function call:     MorphGetLanguageType( PSZ pszLanguage );
//------------------------------------------------------------------------------
// Input parameter:   PSZ     pszLanguage         name of language to check
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_UNDEFINED_LANGTYPE no specific language
//                    MORPH_DBCS_LANGTYPE      a DBCS language
//                    MORPH_BIDI_LANGTYPE      a BIDI language
//------------------------------------------------------------------------------
// Prerequesits:      NONE
//------------------------------------------------------------------------------
// Side effects:      NONE
//------------------------------------------------------------------------------
// Function flow:     search language in (hard-coded) list of DBCS languages
//                    return fFound flag
//------------------------------------------------------------------------------
SHORT MorphGetLanguageType
(
  PSZ              pszLanguage         // language to be checked
)
{
  SHORT i = 0;
  SHORT sType = MORPH_UNDEFINED_LANGTYPE;

  while( (LangTypes[i].szLanguage[0] != EOS) &&
         (stricmp( LangTypes[i].szLanguage, pszLanguage) != 0) )
  {
    i++;
  } /* endwhile */

  if ( LangTypes[i].szLanguage[0] != EOS )
  {
    sType = LangTypes[i].sType;
  } /* endif */

  return( sType );
} /* end of function MorphGetLanguageType */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphFreeTermList            Free a term list
//------------------------------------------------------------------------------
// Description:       Free the memory occupied by a term list
//------------------------------------------------------------------------------
// Function call:     MorphIsDBCSLanguage( PSZ pszLanguage );
//------------------------------------------------------------------------------
// Input parameter:   PVOID   pTermList           term list pointer
//                    USHORT  usListType          type of term list
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR
//------------------------------------------------------------------------------
USHORT MorphFreeTermList
(
  PVOID       pTermList,               // term list pointer
  USHORT      usListType               // type of term list
)
{
  switch ( usListType )
  {
    case MORPH_OFFSLIST :
    case MORPH_ZTERMLIST :
    case MORPH_FLAG_OFFSLIST :
    case MORPH_FLAG_ZTERMLIST :
      // free memory block of term list
      if ( pTermList != NULL ) UtlAlloc( &pTermList, 0L, 0L, NOMSG );
      break;

    case MORPH_LARGE_ZTERMLIST :
      // free all concatenated memory blocks of the term list
      while ( pTermList != NULL )
      {
        PSZ pNextList = *((PSZ *)pTermList);
        UtlAlloc( (PVOID *)&pTermList, 0L, 0L, NOMSG );
        pTermList = pNextList;
      } /* endwhile */
      break;

    default :
      // unknown type of term list ...
      break;
  } /* endswitch */
  return( NO_ERROR );
} /* end of function MorphFreeTermList */

// activate spell instance if not done yet
void MorphCheckSpellInstance( PLANGCB pLangCB )
{
  if ( pLangCB->m_SpellInstance != NULL ) return;

  if ( pLangCB->fNoSpellSupport ) return;

  // get spell factory instance
	SpellFactory* tSpellFactoryInstance = SpellFactory::getInstance();
  if (NULL == tSpellFactoryInstance)
	{
    pLangCB->fNoSpellSupport = TRUE;
    return;
	}

  // get spell instance for this language
	pLangCB->m_SpellInstance = tSpellFactoryInstance->getSpellChecker( pLangCB->szLanguage, pLangCB->szSpellPlugin );
	if (NULL == pLangCB->m_SpellInstance)
	{
    pLangCB->fNoSpellSupport = TRUE;
    pLangCB->fNoStemInSpell = TRUE;
    return;
	}


  // check if spell plugin supports stem form reduction
}

// convert a vector of wstrings to a ZTERMlist
USHORT MorphWStrings2TermList( vector<std::wstring> &vStrings, PVOID *ppTermList, PULONG pulTermListSize )
{
  // compute length of the term list 
  ULONG ulNewLen = 4; // leave enough room for list terminator
	for (size_t i = 0; i < vStrings.size(); i++)
	{
    ulNewLen += vStrings[i].length() + 1;
	}

  // re-allocate term list when neccessary
  ULONG ulNewListSize = ulNewLen * sizeof(CHAR_W);
  if ( *pulTermListSize < ulNewListSize )
  {
    UtlAlloc( ppTermList, *pulTermListSize, ulNewListSize, NOMSG);
    *pulTermListSize = (USHORT)ulNewListSize;
	  if (NULL == *ppTermList )
	  {
		  return MORPH_NO_MEMORY;
	  }
  }

  // copy terms to the term list buffer
  PSZ_W pszCurPos = (PSZ_W)*ppTermList;
	for (size_t i = 0; i < vStrings.size(); i++)
	{
		wcscpy( pszCurPos , vStrings[i].c_str() );
		pszCurPos += vStrings[i].length() + 1;
	}
  *pszCurPos = 0;

  return( 0 );
}


// data area for spellcheck and stem from reduction functions
typedef struct _SPELLSTEMDATA
{
  char   szLine[2048];                 // buffer for input line
  CHAR_W szTermW[2048];                // current term in UTF-16 encoding
  USHORT usTermListSize;               // variable containing size of term list buffer
  PSZ_W   pTermListW;                  // term list pointer
  SHORT sLangID;                       // ID for current language 
  FILE *hfInput;                       // input file handle
  FILE *hfOutput;                      // output file handle
  ULONG ulAnsiCP;                      // ANSI codepage for give language
  CXmlWriter *pXW;                     // XML writer element
  BOOL  fTextOutput;                   // TRUE = output in text mode
} SPELLSTEMDATA, *PSPELLSTEMDATA;

unsigned short MorphFuncSpellCheckSingleTerm
(
  PSPELLSTEMDATA pData,
  char           *pszTerm
)
{
  USHORT usRC = 0;
  char szResult[20];

  // convert to UTF16
  MultiByteToWideChar( pData->ulAnsiCP, 0, pszTerm, -1, pData->szTermW, sizeof(pData->szTermW)/sizeof(CHAR_W) );

  // get language control block pointer
  PLCB       pLCB = NULL;             // pointer to language control block
  usRC = MorphGetLCB( pData->sLangID, &pLCB );
  if ( usRC != 0 ) return( usRC );

  // verify spelling
  usRC = Verify( pLCB->pvLangCB, pData->szTermW );

  // preprocess result
  if ( usRC == MORPH_NOT_FOUND )
  {
    strcpy( szResult, "Misspelled" );
  }
  else if ( usRC == MORPH_OK )
  {
    strcpy( szResult, "Correct" );
  }
  else
  {
    sprintf( szResult, "Code%u", usRC );
  }

  // add results to report
  if ( pData->fTextOutput )
  {
    fprintf( pData->hfOutput, "%S,%s\n", pData->szTermW, szResult );
  }
  else
  {
    pData->pXW->WriteStartElement( "spellCheck" );
    pData->pXW->WriteAttributeString( L"term", pData->szTermW );
    pData->pXW->WriteAttributeString( "result", szResult );
    pData->pXW->WriteEndElement(); // "spellCheck" 
  }
  return( usRC );
}


// the following two function do not fit exactly into this source file, please move to spellcheck/stemform source file once we have one...
unsigned short MorphFuncCheckSpelling
(
  char *pszLanguage,                   // name of the language being used for the spell checking
  char *pszInputTerms,                 // a comma seperated list of terms or NULL if a input file is being used
  char *pszInputFile,                  // the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTemrs is being used
  char *pszReport,                     // name of the report file receiving the results of the operation, the report is in the XML format
  long  lOptions
)
{
  PSPELLSTEMDATA pData = NULL;
  USHORT usRC = 0;

  // allocate our data area
  if ( !UtlAlloc( (PVOID *)&pData, 0, sizeof(SPELLSTEMDATA), ERROR_STORAGE ) )
  {
    return( ERROR_STORAGE );
  }

  // get language ID
  usRC = MorphGetLanguageID( pszLanguage, &pData->sLangID );

  // get language code page
  pData->ulAnsiCP = GetLangAnsiCP( pszLanguage );

  // check type of output
  if ( usRC == 0 )
  {
    if ( lOptions & TEXT_OUTPUT_OPT )
    {
      pData->fTextOutput = TRUE;
    }
  }

  // start output
  if ( usRC == 0 )
  {
    if ( pData->fTextOutput )
    {
      pData->hfOutput = fopen( pszReport, "w" );
      if ( pData->hfOutput == NULL )
      {
        usRC = ERROR_FILE_OPEN_FAILED;
      }
    }
    else
    {
      pData->pXW= new CXmlWriter( pszReport );
      pData->pXW->Formatting = CXmlWriter::Indented;
      pData->pXW->Encoding = CXmlWriter::UTF8;
      pData->pXW->Indention = 2;
      pData->pXW->WriteStartDocument();
  //    pData->pXW->WriteStylesheet( "matchlist.xsl" );
      pData->pXW->WriteStartElement( "spellReport" );
      pData->pXW->WriteStartElement( "header" );
      pData->pXW->WriteElementString( "language", pszLanguage );
      if ( (pszInputTerms != NULL) && (*pszInputTerms != '\0') )
      {
        pData->pXW->WriteElementString( "inputTerms", pszInputTerms );
      }
      else if ( (pszInputFile != NULL) && (*pszInputFile != '\0') )
      {
        pData->pXW->WriteElementString( "inputFile", pszInputFile );
      }
      else 
      {
        pData->pXW->WriteElementString( "error", "no input terms specified" );
      }

      pData->pXW->WriteEndElement(); //  "header"
      pData->pXW->WriteStartElement( "spelllist" );
    }
  }


  // work on list of terms or input file
  if ( usRC == 0 )
  {
    if ( (pszInputTerms != NULL) && (*pszInputTerms != '\0') )
    {
      PSZ pszStart = pszInputTerms;
      while ( *pszStart != '\0' )
      {
        int iLen = 0;
        PSZ pszEnd = strchr( pszStart, ',' );
        if ( pszEnd != NULL )
        {
          iLen = pszEnd - pszStart;
        }
        else
        {
          iLen = strlen( pszStart );
        }
        strncpy( pData->szLine, pszStart, iLen );
        pData->szLine[iLen] = '\0';
        UtlStripBlanks( pData->szLine );
        if ( pData->szLine[0] != '\0' ) MorphFuncSpellCheckSingleTerm( pData, pData->szLine );
        pszStart = ( pszEnd != NULL ) ? (pszStart = pszEnd + 1) : (pszStart + iLen); 
      }
    }
    else if ( (pszInputFile != NULL) && (*pszInputFile != '\0') )
    {
      pData->hfInput = fopen( pszInputFile, "r" );
      if ( pData->hfInput != NULL )
      {
        while ( !feof( pData->hfInput ) )
        {
          memset( pData->szLine, 0, sizeof(pData->szLine) );
          fgets( pData->szLine, sizeof(pData->szLine), pData->hfInput );
          int iLen = strlen(pData->szLine);
          if ( (iLen != 0) && (pData->szLine[iLen-1] == '\n') ) iLen--;
          if ( (iLen != 0) && (pData->szLine[iLen-1] == '\r') ) iLen--;
          pData->szLine[iLen] = '\0';
          UtlStripBlanks( pData->szLine );
          if ( pData->szLine[0] != '\0' ) MorphFuncSpellCheckSingleTerm( pData, pData->szLine  );
        }
      }
    }
    else
    {
      usRC = ERROR_FIELD_INPUT;
    }
  }

  // cleanup
  if ( pData != NULL )
  {
    if ( pData->pXW != NULL)
    {
      pData->pXW->WriteEndElement(); // spelllist
      pData->pXW->WriteEndElement(); // spellreport
      pData->pXW->WriteEndDocument();
      pData->pXW->Close();

      delete pData->pXW;
    }
    if ( pData->hfOutput != NULL ) fclose( pData->hfOutput );
    if ( pData->hfInput != NULL ) fclose( pData->hfInput );
    UtlAlloc( (PVOID *)&pData, 0, 0, NOMSG );
  }
 
  return( usRC );
}

unsigned short MorphFuncReduceSingleTerm
(
  PSPELLSTEMDATA pData,
  char           *pszTerm
)
{
  USHORT usRC = 0;

  // convert to UTF16
  MultiByteToWideChar( pData->ulAnsiCP, 0, pszTerm, -1, pData->szTermW, sizeof(pData->szTermW)/sizeof(CHAR_W) );

  // get stem from
  usRC = MorphGetStemForm( pData->sLangID, pData->szTermW, &(pData->usTermListSize), &(pData->pTermListW), pData->ulAnsiCP );

  // add results to report
  if ( pData->fTextOutput )
  {
    fprintf( pData->hfOutput, "%S,", pData->szTermW );
    if ( usRC == 0 )
    {
      PSZ_W pszStem = pData->pTermListW;
      int iTerms = 0;
      while( *pszStem != '\0' )
      {
        if ( iTerms != 0 ) fputs( "+", pData->hfOutput );
        fprintf( pData->hfOutput, "%S", pszStem );
        pszStem += wcslen(pszStem) + 1;
        iTerms++;
      }
    }
    fputs( "\n", pData->hfOutput );
  }
  else
  {
    pData->pXW->WriteStartElement( "stemForm" );
    pData->pXW->WriteAttributeString( L"term", pData->szTermW );
    pData->pXW->WriteStartAttribute( "rc" );
    pData->pXW->WriteInt( (int)usRC );
    pData->pXW->WriteEndAttribute();
    if ( usRC == 0 )
    {
      PSZ_W pszStem = pData->pTermListW;
      int iTerms = 0;
      while( *pszStem != '\0' )
      {
        if ( iTerms != 0 ) pData->pXW->WriteString( "," );
        pData->pXW->WriteString( pszStem );
        pszStem += wcslen(pszStem) + 1;
        iTerms++;
      }
    }
    pData->pXW->WriteEndElement(); // "stemForm"
  }

  return( usRC );
}

unsigned short MorphFuncReduceToStemForm
(
  char *pszLanguage,                   // name of the language being used for the stem form reduction
  char *pszInputTerms,                 // a comma seperated list of terms or NULL if a input file is being used
  char *pszInputFile,                  // the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTemrs is being used
  char *pszReport,                     // name of the report file receiving the results of the operation, the report is in the XML format
  long  lOptions
)
{
  PSPELLSTEMDATA pData = NULL;
  USHORT usRC = 0;


  // allocate our data area
  if ( !UtlAlloc( (PVOID *)&pData, 0, sizeof(SPELLSTEMDATA), ERROR_STORAGE ) )
  {
    return( ERROR_STORAGE );
  }

  // get language ID
  usRC = MorphGetLanguageID( pszLanguage, &pData->sLangID );

  // get language code page
  pData->ulAnsiCP = GetLangAnsiCP( pszLanguage );

  // check type of output
  if ( usRC == 0 )
  {
    if ( lOptions & TEXT_OUTPUT_OPT )
    {
      pData->fTextOutput = TRUE;
    }
  }

  // start output
  if ( usRC == 0 )
  {
    if ( pData->fTextOutput )
    {
      pData->hfOutput = fopen( pszReport, "w" );
      if ( pData->hfOutput == NULL )
      {
        usRC = ERROR_FILE_OPEN_FAILED;
      }
    }
    else
    {
      pData->pXW= new CXmlWriter( pszReport );
      pData->pXW->Formatting = CXmlWriter::Indented;
      pData->pXW->Encoding = CXmlWriter::UTF8;
      pData->pXW->Indention = 2;
      pData->pXW->WriteStartDocument();
  //    pData->pXW->WriteStylesheet( "matchlist.xsl" );
      pData->pXW->WriteStartElement( "stemReport" );
      pData->pXW->WriteStartElement( "header" );
      pData->pXW->WriteElementString( "language", pszLanguage );
      if ( (pszInputTerms != NULL) && (*pszInputTerms != '\0') )
      {
        pData->pXW->WriteElementString( "inputTerms", pszInputTerms );
      }
      else if ( (pszInputFile != NULL) && (*pszInputFile != '\0') )
      {
        pData->pXW->WriteElementString( "inputFile", pszInputFile );
      }
      else 
      {
        pData->pXW->WriteElementString( "error", "no input terms specified" );
      }

      pData->pXW->WriteEndElement(); //  "header"
      pData->pXW->WriteStartElement( "stemlist" );
    }
  }

  // work on list of terms or input file
  if ( usRC == 0 )
  {
    if ( (pszInputTerms != NULL) && (*pszInputTerms != '\0') )
    {
      PSZ pszStart = pszInputTerms;
      while ( *pszStart != '\0' )
      {
        int iLen = 0;
        PSZ pszEnd = strchr( pszStart, ',' );
        if ( pszEnd != NULL )
        {
          iLen = pszEnd - pszStart;
        }
        else
        {
          iLen = strlen( pszStart );
        }
        strncpy( pData->szLine, pszStart, iLen );
        pData->szLine[iLen] = '\0';
        UtlStripBlanks( pData->szLine );
        if ( pData->szLine[0] != '\0' ) MorphFuncReduceSingleTerm( pData, pData->szLine );
        pszStart = ( pszEnd != NULL ) ? (pszStart = pszEnd + 1) : (pszStart + iLen); 
      }
    }
    else if ( (pszInputFile != NULL) && (*pszInputFile != '\0') )
    {
      pData->hfInput = fopen( pszInputFile, "r" );
      if ( pData->hfInput != NULL )
      {
        while ( !feof( pData->hfInput ) )
        {
          memset( pData->szLine, 0, sizeof(pData->szLine) );
          fgets( pData->szLine, sizeof(pData->szLine), pData->hfInput );
          int iLen = strlen(pData->szLine);
          if ( (iLen != 0) && (pData->szLine[iLen-1] == '\n') ) iLen--;
          if ( (iLen != 0) && (pData->szLine[iLen-1] == '\r') ) iLen--;
          pData->szLine[iLen] = '\0';
          UtlStripBlanks( pData->szLine );
          if ( pData->szLine[0] != '\0' ) MorphFuncReduceSingleTerm( pData, pData->szLine );
        }
      }
    }
    else
    {
      usRC = ERROR_FIELD_INPUT;
    }
  }

  // cleanup
  if ( pData != NULL )
  {
    if ( pData->pXW != NULL)
    {
      pData->pXW->WriteEndElement(); // stemlist
      pData->pXW->WriteEndElement(); // stemreport
      pData->pXW->WriteEndDocument();
      pData->pXW->Close();

      delete pData->pXW;
    }
    if ( pData->hfOutput != NULL ) fclose( pData->hfOutput );
    if ( pData->hfInput != NULL ) fclose( pData->hfInput );
    UtlAlloc( (PVOID *)&pData, 0, 0, NOMSG );
  }
 
  return( usRC );
}

