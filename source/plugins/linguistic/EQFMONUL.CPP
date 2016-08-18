//------------------------------------------------------------------------------
// EQFMONUL.C                      Test stub for morphological language exit
//------------------------------------------------------------------------------
// Copyright Notice:
//
//           Copyright (C) 1990-2013, International Business Machines
//           Corporation and others. All rights reserved
//------------------------------------------------------------------------------
// Author:    G. Queck    QSoft Quality Software Development GmbH
//------------------------------------------------------------------------------
// Description:      This module forms a morph language exit without
//                   processing any morphological functions.
//------------------------------------------------------------------------------
// Entry Points:
//    INIT        - initialize language exit
//    TOKENIZE    - tokenize a segment
//    VERIFY      - verify spelling of a term
//    SPELLAID    - return spell aid
//    TERMADD     - add term to addenda dictionary
//    LEMMA       - reduce term to stem form (lemma)
//    ACTADD      - activate addenda dictionary
//    GETPOS      - get part-of-speech (POS) information for a term
//    TERMINATE   - terminate language exit
//    COMPISOL    - compound word isolation
//------------------------------------------------------------------------------
#include <ctype.h>
#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file
#include "core\spell\SpellFactory.h"
#include "core\morph\MorphFactory.h"

#define MAX_TERMSIZE        256
#define REPLY_AREA_SIZE   40000        // size of reply area
#define LEMMA_LIST_SIZE    8096        // size of lemma list buffer
#define MAX_DICT_NAME       256


/**********************************************************************/
/* Control block for this language exit                               */
/**********************************************************************/
typedef struct _LANGCB
{
   //---------------------- general use data areas ----------------------------
	OtmSpell* m_SpellInstance;
	OtmMorph* m_MorphInstance;
   CHAR     szBuffer[CCHMAXPATH];      // general use buffer
   CHAR     szLemmaList[LEMMA_LIST_SIZE];  // buffer for lemma lists
} LANGCB, *PLANGCB;


//------------------------------------------------------------------------------
// Function name:     INIT
//------------------------------------------------------------------------------
// Description:       Initialization of the language exit.
//------------------------------------------------------------------------------
// Function call:     INIT( PVOID *ppvLangCB, PSZ pszMorphDict,
//                          USHORT usCodePage, USHORT usLAngCode );
//------------------------------------------------------------------------------
// Input parameter:   PSZ        szMorphDict  name of morphologic dictionary
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
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT INIT
(
  PVOID      *ppvLangCB,               // OUT: ptr to buffer for language CB ptr
  PSZ        szMorphDict,              // IN : name of morphologic dictionary
  USHORT     usCodePage,               // IN : code page to be used
  USHORT     usLangCode                // IN : language code (= lang identifier)
)
{
  PLANGCB    pLangCB;                  // ptr to allocated language CB
  USHORT     usRC = MORPH_OK;          // function return code

  /********************************************************************/
  /* Allocate language control block                                  */
  /********************************************************************/
  if ( !UtlAlloc( (PVOID *)&pLangCB, 0L, (LONG)sizeof(LANGCB), NOMSG ) )
  {
      return MORPH_NO_MEMORY;
  } /* endif */

  BOOL   fOK = TRUE;                            // success indicator
  *ppvLangCB = (PVOID)pLangCB;

  

  if ( usRC == MORPH_OK )
  {

	  char tDictName[MAX_DICT_NAME];
	  strcpy(tDictName, szMorphDict );

	  SpellFactory* tSpellFactoryInstance = SpellFactory::getInstance();
	  if (NULL == tSpellFactoryInstance)
	  {
		  usRC = MORPH_NO_MEMORY;
	  }
	  pLangCB->m_SpellInstance = tSpellFactoryInstance->getSpellChecker(tDictName);
	  if (NULL == pLangCB->m_SpellInstance)
	  {
		  usRC = MORPH_NO_MEMORY;
	  }

	  MorphFactory* tMorphFactoryInstance = MorphFactory::getInstance();
	  if (NULL == tMorphFactoryInstance)
	  {
		  usRC = MORPH_NO_MEMORY;
	  }
	  pLangCB->m_MorphInstance = tMorphFactoryInstance->getMorph(tDictName);
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
// Function name:     TOKENIZE
//------------------------------------------------------------------------------
// Description:       Tokenization of an input segment. The tokenized terms
//                    are returned in form of a term list.
//------------------------------------------------------------------------------
// Function call:     TOKENIZE( LX_CB_P pNLPCB,
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
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT TOKENIZE
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
  USHORT     usReturn = 0;             // returncode
  USHORT     usTermBufUsed = 0;        // amount of space used in term buffer
  PSZ        pTerm;                    // pointer to current token string
  PSZ        pszCurPos;                // current position in input data
  BOOL       fOffsList;                // TRUE = return a offset/length list
  LONG       lFlags;                   // flags for current term/token
  BOOL       fAllCaps, fAlNum, fNumber;// character classification flags
  BOOL       fSkip, fNewSentence,      // token processing flags
             fSingleToken, fEndOfToken;

  fOffsList  = (usListType == MORPH_OFFSLIST) ||
               (usListType == MORPH_FLAG_OFFSLIST);
  pszCurPos = pszInData;

  /********************************************************************/
  /* Always assume start of a new sentence                            */
  /********************************************************************/
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

  /********************************************************************/
  /* Initialize processing flags                                      */
  /********************************************************************/
  pTerm    = pszCurPos;
  fAllCaps = TRUE;
  fAlNum   = TRUE;
  fNumber  = TRUE;

  /********************************************************************/
  /* Work on input data                                               */
  /********************************************************************/
  while ( !usReturn && *pszCurPos )
  {
    /******************************************************************/
    /* Get type of processing required for current character          */
    /******************************************************************/
    fSkip         = FALSE;
    fNewSentence  = FALSE;
    fSingleToken  = FALSE;
    fEndOfToken   = FALSE;

    switch ( *pszCurPos )
    {
      case '.' :
      case '!' :
      case '?' :
        if ( pszCurPos[1] == ' ' )
        {
          fNewSentence = TRUE;
          fEndOfToken  = TRUE;
          fSingleToken  = TRUE;
        }
        else
        {
          if ( *pszCurPos != '.' )
          {
            fEndOfToken  = TRUE;
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
        fEndOfToken  = TRUE;
        fSkip        = TRUE;
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
        if ( pszCurPos[1] == ' ' )
        {
          fEndOfToken  = TRUE;
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
        fEndOfToken  = TRUE;
        fSingleToken  = TRUE;
        break;

      default:
        if ( !isalnum(*pszCurPos) )
        {
          fAlNum   = FALSE;
        } /* endif */
        if ( !isdigit(*pszCurPos) )
        {
          fNumber  = FALSE;
        } /* endif */
        if ( !isalpha(*pszCurPos) || islower(*pszCurPos) )
        {
          fAllCaps = FALSE;
        } /* endif */
        break;
    } /* endswitch */

    /******************************************************************/
    /* Terminate current token and start a new one if requested       */
    /******************************************************************/
    if ( fEndOfToken && (pTerm < pszCurPos) )
    {
      lFlags = 0L;
      if ( fAllCaps ) lFlags |= TF_ALLCAPS;
      if ( !fAlNum )  lFlags |= TF_NOLOOKUP;
      if ( fNumber )  lFlags |= TF_NUMBER;
      usReturn = MorphAddTermToList( (PSZ *)ppTermList,
                                     pusTermListSize,
                                     &usTermBufUsed,
                                     pTerm,
                                     pszCurPos - pTerm,
                                     (USHORT)(fOffsList ? (pTerm-pszInData) : 0),
                                     lFlags,
                                     usListType );
      pTerm    = pszCurPos;
      fAllCaps = TRUE;
      fAlNum   = TRUE;
      fNumber  = TRUE;
    } /* endif */

    /******************************************************************/
    /* Handle single character tokens                                 */
    /******************************************************************/
    if ( fSingleToken && !usReturn )
    {
      usReturn = MorphAddTermToList( (PSZ *)ppTermList,
                                     pusTermListSize,
                                     &usTermBufUsed,
                                     pszCurPos,
                                     1,
                                     (USHORT)(fOffsList ? (pszCurPos-pszInData) : 0),
                                     TF_NOLOOKUP,
                                     usListType );
      pTerm    = pszCurPos + 1;
    } /* endif */

    /******************************************************************/
    /* Handle start of a new sentence                                 */
    /******************************************************************/
    if ( fNewSentence && !usReturn )
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


    /******************************************************************/
    /* Handle skip flag                                               */
    /******************************************************************/
    if ( fSkip )
    {
      pTerm = pszCurPos + 1;
    } /* endif */

    /******************************************************************/
    /* Continue with next character                                   */
    /******************************************************************/
    if ( *pszCurPos != EOS ) pszCurPos++;

  } /* endwhile */

  /********************************************************************/
  /* Process any pending token                                        */
  /********************************************************************/
  if ( !usReturn && (pTerm <= pszCurPos) )
  {
    lFlags = 0L;
    if ( fAllCaps ) lFlags |= TF_ALLCAPS;
    if ( !fAlNum )  lFlags |= TF_NOLOOKUP;
    if ( fNumber )  lFlags |= TF_NUMBER;
    usReturn = MorphAddTermToList( (PSZ *)ppTermList,
                                   pusTermListSize,
                                   &usTermBufUsed,
                                   pTerm,
                                   (USHORT)((pszCurPos - pTerm) + 1),
                                   (USHORT)(fOffsList ? (pTerm-pszInData) : 0),
                                   lFlags,
                                   usListType );
  } /* endif */

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
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT VERIFY
(
  PVOID    pvLangCB,                   // IN : ptr to language control block
  PSZ      pszTerm                     // IN : ptr to term being verified
)
{
  USHORT usRC = MORPH_OK;

  PLANGCB tPLangCB = (PLANGCB)pvLangCB;
  if (NULL == tPLangCB)
  {
	  return MORPH_INV_PARMS;
  }
  OtmSpell* tSpellInstance = tPLangCB->m_SpellInstance;
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
// Function name:     SPELLAID
//------------------------------------------------------------------------------
// Description:       Returns possible spelling for the term pszTerm in form
//                    of a term list (only valid list type is MORPH_ZTERMLIST).
//------------------------------------------------------------------------------
// Function call:     SPELLAID( PVOID pvLangCB, PSZ pszTerm,
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
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT SPELLAID
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
  USHORT usRC = MORPH_OK;

  PLANGCB tPLangCB = (PLANGCB)pvLangCB;
  if (NULL == tPLangCB)
  {
	  return MORPH_INV_PARMS;
  }
  OtmSpell* tSpellInstance = tPLangCB->m_SpellInstance;
  if (NULL == tSpellInstance)
  {
	  return MORPH_NOT_FOUND;
  }

  vector<string> tTermList;
  int tRet = tSpellInstance->suggest(pszTerm, tTermList);
  if (tRet <= 0)
  {
	  return MORPH_NOT_FOUND;
  }

  UtlAlloc((PVOID* )ppTermList, 0, MAX_SEGMENT_SIZE, NOMSG);
  char* pTermList = (char* )*ppTermList;
  if (NULL == pTermList)
  {
	  return MORPH_NO_MEMORY;
  }
  memset(pTermList, 0, MAX_SEGMENT_SIZE);
  for (int i = 0; i < tTermList.size(); i++)
  {
	  strcpy(pTermList, tTermList[i].c_str());
	  pTermList += tTermList[i].length();
	  pTermList++;
  }
  *pusTermListSize = (USHORT)MAX_SEGMENT_SIZE;

  return( usRC );

} /* end of function SPELLAID */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TERMADD
//------------------------------------------------------------------------------
// Description:       Adds the given term to the addenda dictionary.
//------------------------------------------------------------------------------
// Function call:     TERMADD( PVOID pvLangCB, PSZ pszTerm );
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
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT TERMADD
(
  PVOID    pvLangCB,                   // IN : ptr to language control block
  PSZ      pszTerm                     // IN : ptr to term being added
)
{
	USHORT usRC = MORPH_OK;

	PLANGCB tPLangCB = (PLANGCB)pvLangCB;
	if (NULL == tPLangCB)
	{
		return MORPH_INV_PARMS;
	}
	OtmSpell* tSpellInstance = tPLangCB->m_SpellInstance;
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
// Function name:     LEMMA
//------------------------------------------------------------------------------
// Description:       Returns all lemmas (stem forms) for term pszTerm
//                    in form of a term list.
//------------------------------------------------------------------------------
// Function call:     LEMMA( PVOID pvLangCB, PSZ pszTerm,
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
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT LEMMA
(
  PVOID    pvLangCB,                   // IN : ptr to language control block
  PSZ      pszTerm,                    // IN : ptr to term being added
  PUSHORT  pusTermListSize,            // IN/OUT:  address of variable
                                       //    containing size of term list buffer
  PVOID    *ppTermList,                // IN/OUT: address of term list pointer
  USHORT   usListType                  // IN: type of term list: always
                                       //     MORPH_ZTERMLIST
)
{
  PLANGCB    pLangCB;                  // ptr to allocated language CB
  USHORT     usRC = MORPH_OK;          // function return code
  USHORT     usTermListUsed = 0;       // current size of term list

  pLangCB = (PLANGCB)pvLangCB;

  strcpy( pLangCB->szLemmaList, pszTerm );
  UtlLower( pLangCB->szLemmaList );

  if ( pLangCB->szLemmaList[strlen(pszTerm)-1] == 's' )
  {
    pLangCB->szLemmaList[strlen(pszTerm)-1] = EOS;
  } /* endif */

  /*********************************************************/
  /* Add term to term list                                 */
  /*********************************************************/
//   usRC = MorphAddTermToList( (PSZ *)ppTermList,
//                              pusTermListSize,
//                              &usTermListUsed,
//                              pLangCB->szLemmaList,
//                              (USHORT)strlen(pLangCB->szLemmaList),
//                              0,
//                              0L,
//                              usListType);

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

  vector<string> tTermList;
  usRC = tMorphInstance->stem(pszTerm, tTermList);
  
  if (MORPH_OK == usRC)
  {
	  for (int i = 0; i < tTermList.size(); i++)
	  {
		  if (tTermList[i].size() < 1)
		  {
			  continue;
		  }
		  strcpy( pLangCB->szLemmaList, tTermList[i].c_str());
		  usRC = MorphAddTermToList( (PSZ *)ppTermList,
			  pusTermListSize,
			  &usTermListUsed,
			  pLangCB->szLemmaList,
			  (USHORT)strlen(pLangCB->szLemmaList),
			  0,
			  0L,
			  usListType);
	  }
  }

  /*****************************************************************/
  /* terminate term list if term list is not empty otherwise       */
  /* change return code                                            */
  /*****************************************************************/
  if ( !usRC )
  {
    usRC = MorphAddTermToList( (PSZ *)ppTermList,
                               pusTermListSize,
                               &usTermListUsed,
                               "",
                               0,
                               0,
                               0L,
                               usListType);
  } /* endif */

  return( usRC );

} /* end of function LEMMA */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ACTADD
//------------------------------------------------------------------------------
// Description:       Activates the addenda dictionary pszAddendaDict.
//------------------------------------------------------------------------------
// Function call:     ACTADD( PVOID pvLangCB, PSZ pszAddendaDict );
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//                    PSZ      pszAddendaDict  ptr to name of addenda dict.
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     call NlpActDict
//                    if dictionary was not found then
//                      create addenda dictionary
//                      activate addenda dictionary
//                      deactivate addenda dictionary
//                      activate addenda dictionary
//                    endif
//                    return function return code
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT ACTADD
(
  PVOID    pvLangCB,                   // IN : ptr to language control block
  PSZ      pszAddendaDict              // IN : ptr to name of addenda dictionary
)
{
  USHORT     usRC = MORPH_OK;          // function return code

  return( usRC );

} /* end of function ACTADD */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     GETPOS
//------------------------------------------------------------------------------
// Description:       Get part-of-speech data for term pszTerm. The returned
//                    information are the ORed POS values (e.g. MORPH_VERB |
//                    MORPH_NOUN )
//------------------------------------------------------------------------------
// Function call:     GETPOS( PVOID pvLangCB, PSZ pszTerm, PUSHORT pusPOSInfo )
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
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT GETPOS
(
  PVOID    pvLangCB,                   // IN : ptr to language control block
  PSZ      pszTerm,                    // IN : ptr to term
  PUSHORT  pusPOSInfo                  // OUT: part-of-speech info for term
)
{
  USHORT  usRC = MORPH_NOT_FOUND;

  return( usRC );

} /* end of function GETPOS */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TERMINATE
//------------------------------------------------------------------------------
// Description:       Terminate the language exit. All dictionaries are closed
//                    and any allocated storage is freed.
//------------------------------------------------------------------------------
// Function call:     TERMINATE( PVOID pvLangCB );
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
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT TERMINATE
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

} /* end of function TERMINATE */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     COMPISOL
//------------------------------------------------------------------------------
// Description:       Returns all decompositions for a compound word. This
//                    function is supported only for the germanic languages.
//                    If there are mor decompositions for a term these
//                    decompositions are seperated by a EOS delimter.
//------------------------------------------------------------------------------
// Function call:     COMPISOL( PVOID pvLangCB, PSZ pszTerm,
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
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT COMPISOL
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

  usRC = MORPH_NOT_FOUND;

  return( usRC );

} /* end of function COMPISOL */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ABBREVADD
//------------------------------------------------------------------------------
// Description:       Adds the given term to the abbrev dictionary.
//------------------------------------------------------------------------------
// Function call:     ABBREVADD( PVOID pvLangCB, PSZ pszTerm );
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
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT ABBREVADD
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ      pszTerm                     // IN : ptr to term being added
 )
{
	USHORT usRC = MORPH_OK;

	PLANGCB tPLangCB = (PLANGCB)pvLangCB;
	if (NULL == tPLangCB)
	{
		return MORPH_INV_PARMS;
	}
	OtmSpell* tSpellInstance = tPLangCB->m_SpellInstance;
	if (NULL == tSpellInstance)
	{
		return MORPH_FUNC_NOT_SUPPORTED;
	}

	int tRet = tSpellInstance->addTerm(pszTerm, ABBREV_DICT);
	if (0 != tRet)
	{
		return MORPH_FUNC_NOT_SUPPORTED;
	}
	tSpellInstance->writeTermToFile(ABBREV_DICT);

	return( usRC );

} /* end of function ABBREVADD */


#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT LISTDICT
(
 PVOID pvLangCB, 
 USHORT usDictType, 
 PUSHORT pusTermListSize, 
 PVOID *ppTermList, 
 USHORT usListType
 )
{
	USHORT usRC = MORPH_OK;

	if (ABBREV_DICT != usDictType && ADDENDA_DICT != usDictType)
	{
		return MORPH_NOT_FOUND;
	}

	PLANGCB tPLangCB = (PLANGCB)pvLangCB;
	if (NULL == tPLangCB)
	{
		return MORPH_INV_PARMS;
	}
	OtmSpell* tSpellInstance = tPLangCB->m_SpellInstance;
	if (NULL == tSpellInstance)
	{
		return MORPH_NOT_FOUND;
	}

	tSpellInstance->readTermFromFile(usDictType);
	vector<string> tTermList;
	tSpellInstance->listTerms(tTermList, usDictType);

	UtlAlloc((PVOID* )ppTermList, 0, MAX_SEGMENT_SIZE, NOMSG);
	char* pTermList = (char* )*ppTermList;
	if (NULL == pTermList)
	{
		return MORPH_NO_MEMORY;
	}
	memset(pTermList, 0, MAX_SEGMENT_SIZE);
	for (int i = 0; i < tTermList.size(); i++)
	{
		strcpy(pTermList, tTermList[i].c_str());
		pTermList += tTermList[i].length();
		pTermList++;
	}
	*pusTermListSize = (USHORT)MAX_SEGMENT_SIZE;

	return( usRC );

} /* end of function LISTDICT */

#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT BUILDDICT
( 
 PVOID pvLangCB, 
 USHORT usDictType, 
 USHORT usTermListSize, 
 PVOID pTermList, 
 USHORT usListType
 ) 
{
	USHORT usRC = MORPH_OK;

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

	OtmSpell* tSpellInstance = tPLangCB->m_SpellInstance;
	if (NULL == tSpellInstance)
	{
		return MORPH_NOT_FOUND;
	}

	vector<string> tOldTermList;
	tSpellInstance->listTerms(tOldTermList, usDictType);
	for (int i = 0; i < tOldTermList.size(); i++)
	{
		tSpellInstance->deleteTerm(tOldTermList[i].c_str(), usDictType);
	}

	char* tPTerm = (char*) pTermList + sizeof(PSZ);
	int tLen = strlen(tPTerm);
	while (tLen > 0)
	{
		tSpellInstance->addTerm(tPTerm, usDictType);
		tPTerm += tLen;
		tPTerm++;
		tLen = strlen(tPTerm);
	}


	if( true != tSpellInstance->writeTermToFile(usDictType))
		usRC = MORPH_FUNC_FAILED;

	return usRC;
}
