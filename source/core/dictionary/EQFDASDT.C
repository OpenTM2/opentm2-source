#define ASD_USE_COMPOUNDS_IN_AUT_LOOKUP   // activates compound handling
//+----------------------------------------------------------------------------+
//|  EQFDASDT.C  - ASD functions for automatic dictionary lookup               |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_LDB              // dictionary data encoding functions
#include <eqf.h>                  // General Translation Manager include file
#include <locale.h>

#include "EQFQDAMI.H"             // fastdam header file
#include "EQFRDICS.H"             // for CheckPropCompatibility function

#include "EQFDASDI.H"             // internal ASD services header file
#include "OtmDictionaryIF.H"
#include "eqfevent.h"             // event logging facility

// undefine the Control Block for communications -- we might have used
// the same typedef for dictionary control block
   #undef DCB

// activate the following for logging of returned term list
#ifdef _DEBUG
  #define ASDT_LOG_TERMLIST
#endif

static LONG AsdGetTransLen( PSZ_W pucTrans ); // in number of char_w'S
static PSZ_W AsdGetTransText( PSZ_W pucTrans ); // Locate actual term text
static BOOL AsdIsTranslationIdentical( PSZ_W pucTrans1, PSZ_W pucTrans2, int iLen );

static CHAR_W SearchAndEvaluateStyle( FIELDDATA *pStyleField, PSZ_W pucData, USHORT usLevel, BOOL fSearchAll );
static BOOL CheckPIDValue(PSZ_W pucProd, PSZ_W pucValues );

#ifdef ASDT_LOG_TERMLIST
  void WriteTermListToLog( PSZ pszTitle, PSZ_W pszTermList, FILE *hfLog );
#endif

//+----------------------------------------------------------------------------+
//| AsdTranslate        Lookup all translations for all terms in a segment.    |
//+----------------------------------------------------------------------------+
//| Description:        Lookup all terms for a segment and return the terms    |
//|                     or the translation depending on your request or        |
//|                     all translations and other dictionary fields marked    |
//|                     for automatic lookup.                                  |
//|                     the mode flag controls the way AsdTranslate works:     |
//|                                                                            |
//|                       ASDT_TERMS_ONLY   only terms are added to the term   |
//|                                         list                               |
//|                       ASDT_TRANSLATIONS terms and translations are added   |
//|                                         the list                           |
//|                       ASDT_MARKED_DATA  terms, translations and marked     |
//|                                         dictionary fields are added to     |
//|                                         the list. Data of dictionary fields|
//|                                         other than the translation is      |
//|                                         prefixed by ASDT_DICT_DATA_CHAR    |
//|                                         which is 0x01.                     |
//|                       (these flags are mutual exclusive!)                  |
//|                       ASDT_SEARCH_ALL_DICTS  do not stop at first matching |
//|                                         dictionary but search all dicts    |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB     hUCB                IN     user control block handle           |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    PUCHAR   pucSegment          IN     ptr to input segment (segment must  |
//|                                        be terminated by a null character)  |
//|    USHORT   usOutBufSize        IN     size of outbuf                      |
//|    PUCHAR   pucOutBuf           OUT    buffer for terms and translations;  |
//|                                        format is:                          |
//|                                           TERM1\0TRANS11\0...TRANS1n\0\0   |
//|                                              ...                           |
//|                                           TERMm\0TRANSm1\0...TRANSmx\0\0   |
//|                                           \0                               |
//|    USHORT   usMode              IN     mode for AsdTranslate               |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_DATA_2_LRG_ASD   =  output buffer overflow      |
//|                         LX_...              =  other ASD errors            |
//|                         ASD_OVERFLOW    term buffer overflow               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
USHORT AsdTranslateW
(
   HUCB     hUCB,                      //     user control block handle
   HDCB     hDCB,                      //     dictionary control block handle
   PCHAR_W   pucSegment,                //     ptr to input segment (segment
                                       //     must be terminated by a null
                                       //     character)
   USHORT   usOutBufSize,              //     size of outbuf
   PCHAR_W   pucOutBuf,                 //     buffer for terms and translations
   USHORT   usMode                     //     mode of AsdTranslate
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
                                                                /* 2@KIT0969A */
   USHORT    usMorphRC;                // return code of Morph call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   PSZ_W     pszOrgTerm;               // ptr to current original term
   PSZ_W     pszDictTerm;              // ptr to current dictionary term
   PCHAR_W   pucDictData = NULL;       // buffer for entry data
   ULONG     ulBufSize = 0L;           // size of buffer for entry data
   ULONG     ulDataLength = 0;             // length of dictionary entry data
   ULONG     ulTermNumber;             // number of term being retrieved
   PSZ_W     pucData;                  // ptr to begin of dictionary entry data
   EQF_ATOM  Atom;                     // atom returned by Win..Atom calls
   USHORT    usDictHandle = 0;         // handle of dictionary where term was found
   PDCB      pTermDCB = NULL;          // DCB of term found
   USHORT    usTransTerms;             // number of terms in translation list
   LONG      lTransBufUsed = 0;       // number of bytes used in translation list
   LONG      lTransBufSize = usOutBufSize; // number of bytes used in translation list
   BOOL      fFirstTranslation = TRUE; // TRUE = no translations yet for term
   USHORT    usOutTerms = 0;           // number of terms in output buffer
   PSZ_W     pucOutTerms[MAX_TRANSL_TERMS+1];    // list of terms added in output
   BOOL      fFieldStart;              // TRUE = position is on a field start
   USHORT    usLevel;                  // current dictionary level
   USHORT    usField = 0;              // current dictionary field
   BOOL      fTermInBuffer = FALSE;    // TRUE = term is already in buffer
   PSZ_W     pCompound;                // ptr for compound processing
                                                                /* 4@KIT0927A */
   PSZ_W     pszStemTerm = NULL;       // ptr to current stem form term
   CHAR_W    chEndOfList = EOS;        // end of term list buffer
   CHAR_W    chFirstChar;              // buffer for first character of term
                                                                /* 2@KITxxxxA */
   LONG      lUpdTime;                 // buffer for last update time
   BOOL      fTranslate = TRUE;        // translations requested ??
   BOOL      fDictData = FALSE;        // marked dictionary data requested ??
   BOOL      fSearchAll = FALSE;       // search all dictionaries
   BOOL      fTranslationProcessed = FALSE; // TRUE = translation on this level has been processed
   LONG      lTermStylePos = 0;        // position of term style field in output buffer
   USHORT    usTransTermCount = 0 ;    // number of trans terms within a head term

   LONG      lLastHeadTermPos = 0;     // position of previous head term in output buffer
   LONG      lFirstTransTermPos = 0;   // position of first trans term for this head term
   LONG      lLastTransTermPos = 0;    // position of previous translated term in output buffer
   CHAR_W    szPIDValues[MAX_DICTPID_VALUES] ; // Selected PID values to check against  
   BOOL      fPIDRemoveLastHead = FALSE;  // TRUE = Remove previous head term for PID selection
   BOOL      fPIDRemoveLastTerm = FALSE;  // TRUE = Remove previous term for PID selection
   BOOL      fSplitTerm;               // TRUE = Term contains dash or slash
   BOOL      fPlural2Singular;         // TRUE = Try singular term not found in HUNSPELL dict.
   BOOL      fPlural2SingularUpper;    // TRUE = Try singular uppercase term not found in HUNSPELL dict.
   USHORT    usEDSuffix = 0 ;          // "ED" suffix.  1=remove "D", 2=remove "ED"
   CHAR_W    szFirstChar[2];
   BOOL      fFirstLetterUppercase = FALSE; // TRUE = Term is first in segment and 1st letter uppercase
   BOOL      fFirstLetterLowercase = FALSE; // TRUE = Processing lowercase term when 1st letter was uppercase
   USHORT    usHeadTermExact = 0 ;     // 0=No head term, 1=head term exact, 2=head term not exact

#ifdef ASDT_LOG_TERMLIST
   FILE *hfLog = NULL;
#endif


   // allocation size for AddDictData buffer
   #define ADDDICTDATASIZE 32000L
   PSZ_W     pszAddDictData = NULL;    // temporary buffer for additional dictionary data
   ULONG     ulOemCp = 0L;
   BOOL      fLookupTermsOfCompounds = (usMode & ASDT_SINGLECOMP_LOOKUP);

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;

   ulOemCp = GetLangOEMCP(NULL);       // get system pref. lang.


   DEBUGEVENT2( ASDTRANSLATE_LOC, FUNCENTRY_EVENT, 0, DICT_GROUP, NULL );

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );  // check user control block pointer
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC ); // check dictionary control block pointer
   } /* endif */
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      // check data areas
      if ( !pucSegment || !pucOutBuf || !usOutBufSize )
      {
         usNlpRC = LX_UNEXPECTED_ASD;
      } /* endif */
   } /* endif */

   //
   // set up global variables in UCB
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      if ( !pUCB->fTransUsed )
      {
         pUCB->fTransUsed = TRUE;
         pUCB->hNotInBaseAtoms  = WinCreateAtomTable( 8000, 37 );
      } /* endif */
   } /* endif */

                                                               /* 19@KITxxxxA */
   /*******************************************************************/
   /* Update atom tables if dictionary has been updated in the        */
   /* mean time                                                       */
   /*******************************************************************/
   if ( usNlpRC == LX_RC_OK_ASD )
   {
     NlpDictUpdTime( pDCB->usDictHandle,
                     &lUpdTime,
                     &usNlpRC );
     if ( (usNlpRC == LX_RC_OK_ASD) &&
          (pDCB->lUpdTime < lUpdTime) )
     {
       pDCB->lUpdTime = lUpdTime;
       WinDestroyAtomTable( pUCB->hNotInBaseAtoms );
       pUCB->hNotInBaseAtoms  = WinCreateAtomTable( 8000, 37 );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Process specified mode flag                                     */
   /*******************************************************************/
   if ( usNlpRC == LX_RC_OK_ASD )
   {
     switch ( usMode & 0x0F )
     {
       case ASDT_TERMS_ONLY:
         fTranslate = FALSE;
         fDictData  = FALSE;
         break;

        case ASDT_TRANSLATIONS:
         fTranslate = TRUE;
         fDictData  = FALSE;
         break;

        case ASDT_MARKED_DATA:
         fTranslate = TRUE;
         fDictData  = TRUE;
         break;

       default :
         usNlpRC = LX_UNEXPECTED_ASD;
         break;
     } /* endswitch */
     if ( usMode & ASDT_SEARCH_ALL_DICTS )
     {
       fSearchAll = TRUE;
     } /* endif */
   } /* endif */

   // allocate buffer for temporary dictionary data if required
   if ( fDictData && (usNlpRC == LX_RC_OK_ASD) )
   {
     if ( !UtlAlloc( (PVOID *)&pszAddDictData, 0L, ADDDICTDATASIZE * sizeof(CHAR_W),
                     NOMSG ) )
     {
       usNlpRC = LX_MEM_ALLOC_ASD;
     } /* endif */
   } /* endif */

   //
   // extract terms for dictionary lookup from input data
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {                                                           /* 14@KIT0969C */
     pUCB->usDictSearchSubType = FEXACT_EQUIV ;     // special hyphenation lookup
     DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 2, DICT_GROUP, NULL );
     usMorphRC = MorphWordRecognitionW( pDCB->sLangID, // language ID
                                     pucSegment,    // pointer to input segment
                                     hUCB,          // user control block handle
                                     hDCB,          // dict control block handle
                                     &pUCB->ulOrgTermListSize,
                                     &pUCB->pszOrgTermList,
                                     &pUCB->ulDictTermListSize,
                                     &pUCB->pszDictTermList,
                                     (fLookupTermsOfCompounds) ? 2 : TRUE, 
                                     &usNlpRC,
                                       0 );
     DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 3, DICT_GROUP, NULL );
     pUCB->usDictSearchSubType = FEXACT ;           // reset value
     if ( usMorphRC != MORPH_ASD_ERROR )
     {
       usNlpRC = AsdMorphRCToNlp( usMorphRC );
     } /* endif */
   } /* endif */

   //
   // get translations for found terms
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
     CHAR_W szFoundTerm[MAX_TERM_LEN];      // found term buffer
     
     /*****************************************************************/
     /* Start at begin of term list                                   */
     /*****************************************************************/
     pszOrgTerm    = pUCB->pszOrgTermList;
     pszDictTerm   = pUCB->pszDictTermList;

     if ( pUCB->fDictPIDSelect ) { 
        ASCII2UnicodeBuf( pUCB->szDictPIDSelect, szPIDValues, sizeof(szPIDValues)/sizeof(CHAR_W), 0L);
     }

     /*****************************************************************/
     /* Loop over all terms in list                                   */
     /*****************************************************************/
     while ( *pszOrgTerm && (usNlpRC == LX_RC_OK_ASD) )
     {
       szFoundTerm[0] = 0;             // no term found so far

       /***************************************************************/
       /* Lookup of original term in dictionary                       */
       /***************************************************************/
       //DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 0, DICT_GROUP, pszOrgTerm );
       DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 0, DICT_GROUP, NULL );
       if ( WinFindAtomW( pUCB->hNotInBaseAtoms, pszOrgTerm, ulOemCp  ) != NULLHANDLE )
       {
          usNlpRC = LX_WRD_NT_FND_ASD;      // term will not be found!
       }
       else
       {
          NlpFndEquivAsdW( pszOrgTerm, // term we are looking for
                          pDCB->usDictHandle, // dictionary handle
                          pUCB->usUser, // ASD user handle
                          pDCB->aucDummy, // matching term found
                          &ulTermNumber, // number of term found
                          &ulDataLength, // length of entry data
                          &usDictHandle, // dictionary of match
                          &usNlpRC );
          if ( ASDOK(usNlpRC) ) UTF16strcpy( szFoundTerm, pszOrgTerm );
          if ( usNlpRC == LX_WRD_NT_FND_ASD )
          {
             Atom = WinAddAtomW( pUCB->hNotInBaseAtoms, pszOrgTerm, ulOemCp  );
          } /* endif */
       } /* endif */

       /***************************************************************/
       /* Lookup of dictionary term in dictionary                     */
       /***************************************************************/
       if ( usNlpRC == LX_WRD_NT_FND_ASD )
       {
         if ( UTF16strcmp( pszOrgTerm, pszDictTerm ) == 0 )
         {
            usNlpRC = LX_WRD_NT_FND_ASD;      // term has already been looked-up
         }
         else if ( WinFindAtomW( pUCB->hNotInBaseAtoms, pszDictTerm, ulOemCp  ) != NULLHANDLE )
         {
            usNlpRC = LX_WRD_NT_FND_ASD;      // term will not be found!
         }
         else
         {
            NlpFndEquivAsdW( pszDictTerm, // term we are looking for
                            pDCB->usDictHandle, // dictionary handle
                            pUCB->usUser, // ASD user handle
                            pDCB->aucDummy, // matching term found
                            &ulTermNumber, // number of term found
                            &ulDataLength, // length of entry data
                            &usDictHandle, // dictionary of match
                            &usNlpRC );
            if ( ASDOK(usNlpRC) ) UTF16strcpy( szFoundTerm, pszDictTerm );
            if ( usNlpRC == LX_WRD_NT_FND_ASD )
            {
               Atom = WinAddAtomW( pUCB->hNotInBaseAtoms, pszDictTerm, ulOemCp  );
            } /* endif */
         } /* endif */
       } /* endif */

                                                               /* 62@KIT0927A */
       /***************************************************************/
       /* Lookup of possible stem forms in dictionary                 */
       /***************************************************************/
       if ( usNlpRC == LX_WRD_NT_FND_ASD )
       {
         /*************************************************************/
         /* Get list of stem forms for term                           */
         /* Note: The term list used for the stem list is used also   */
         /*       by the AsdGetStemForm function. Therefore           */
         /*       AsdGetStemForm must not be called while the         */
         /*       stem form list is processed!                        */
         /*************************************************************/
         DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 4, DICT_GROUP, NULL );

         usMorphRC = MorphGetStemForm( pDCB->sLangID, pszOrgTerm,
                                       &pUCB->usTermListSize, &pUCB->pszTermList,
                                        pDCB->ulOemCP);

         // convert string list unicode to ascii
        // if ( !usMorphRC )
        // {
        //   LONG lSize = pUCB->usTermListSize;
        //   LONG lUsed = 0;
        //   usMorphRC = MorphCopyStringListUnicode2ASCII( pTermListW, (LONG) usListSizeW,
        //                                           &pUCB->pszTermList, &lSize, &lUsed );
        //   pUCB->usTermListSize = (USHORT) lSize;
        //   UtlAlloc( &pTermListW, 0L, 0L, NOMSG );
        // }

         DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 5, DICT_GROUP, NULL );
         switch ( usMorphRC )
         {
           case MORPH_OK :
             pszStemTerm = pUCB->pszTermList; // use supplied stem terms
             break;
           case MORPH_NOT_FOUND :
             pszStemTerm = &chEndOfList;      // simulate end of stem term list
             break;
           default :
             usNlpRC = AsdMorphRCToNlp( usMorphRC );
             break;
         } /* endswitch */

         /*************************************************************/
         /* Loop over stem form list and try to find term in dict.    */
         /*************************************************************/
         while ( (usNlpRC == LX_WRD_NT_FND_ASD) && *pszStemTerm )
         {
           if ( UTF16strcmp( pszStemTerm, pszDictTerm ) == 0 )
           {
              usNlpRC = LX_WRD_NT_FND_ASD;      // term has already been looked-up
           }
           else if ( UTF16strcmp( pszStemTerm, pszOrgTerm ) == 0 )
           {
              usNlpRC = LX_WRD_NT_FND_ASD;      // term has already been looked-up
           }
           else if ( WinFindAtomW( pUCB->hNotInBaseAtoms, pszStemTerm, ulOemCp  ) != NULLHANDLE )
           {
              usNlpRC = LX_WRD_NT_FND_ASD;      // term will not be found!
           }
           else
           {
              NlpFndEquivAsdW( pszStemTerm, // term we are looking for
                              pDCB->usDictHandle, // dictionary handle
                              pUCB->usUser, // ASD user handle
                              pDCB->aucDummy, // matching term found
                              &ulTermNumber, // number of term found
                              &ulDataLength, // length of entry data
                              &usDictHandle, // dictionary of match
                              &usNlpRC );
              if ( ASDOK(usNlpRC) ) UTF16strcpy( szFoundTerm, pszStemTerm );
              if ( usNlpRC == LX_WRD_NT_FND_ASD )
              {
                 Atom = WinAddAtomW( pUCB->hNotInBaseAtoms, pszDictTerm, ulOemCp  );
              } /* endif */
           } /* endif */
           pszStemTerm += UTF16strlenCHAR(pszStemTerm) + 1;    // continue with next term
         } /* endwhile */
       } /* endif */

                                                               /* 80@KIT0927A */
       /***************************************************************/
       /* If term hasn't been found yet and the term is the first one */
       /* of the segment and start with a capital letter then try     */
       /* to get stem forms of term after lowercasing of the first    */
       /* character                                                   */
       /* Note: This will fix problems like                           */
       /*       German 'Wichtige' returning only 'Wichtige' as stem   */
       /*       form but not 'wichtig' which is the right form in most*/
       /*       cases.                                                */
       /***************************************************************/
       /***************************************************************/
       /* If "Term" was 1st term of segment and was found,            */
       /* then still look for "term". Usage may be for lowercase word.*/
       /* If lowercase word is not found in 1st dictionary, then      */
       /* lowercase word will not be searched for in other dictionary.*/ 
       /***************************************************************/
       if ( (usNlpRC == LX_RC_OK_ASD) &&
            (pszOrgTerm == pUCB->pszOrgTermList) )
       {
          szFirstChar[0] = *pszOrgTerm;
          szFirstChar[1] = EOS;
          UtlLowerW( szFirstChar );
          if ( *pszOrgTerm != szFirstChar[0] )
          {
             fFirstLetterUppercase = TRUE ;
          }
       }
       if ( (usNlpRC == LX_WRD_NT_FND_ASD)       &&
            (pszOrgTerm == pUCB->pszOrgTermList) )
       {
         /*************************************************************/
         /*  Convert term to lower case and check if first            */
         /*  character has been changed.                              */
         /*************************************************************/
         chFirstChar = *pszOrgTerm;
         UtlLowerW( pszOrgTerm );
         if ( *pszOrgTerm != chFirstChar )
         {
           usNlpRC = LX_WRD_NT_FND_ASD;
           /*************************************************************/
           /* Get list of stem forms for term                           */
           /* Note: The term list used for the stem list is used also   */
           /*       by the AsdGetStemForm function. Therefore           */
           /*       AsdGetStemForm must not be called while the         */
           /*       stem form list is processed!                        */
           /*************************************************************/
           usMorphRC = MorphGetStemForm( pDCB->sLangID, pszOrgTerm,
                                        &pUCB->usTermListSize,
                                        &pUCB->pszTermList, pDCB->ulOemCP );

           switch ( usMorphRC )
           {
             case MORPH_OK :
               pszStemTerm = pUCB->pszTermList; // use supplied stem terms
               break;
             case MORPH_NOT_FOUND :
               pszStemTerm = &chEndOfList;      // simulate end of stem term list
               break;
             default :
               usNlpRC = AsdMorphRCToNlp( usMorphRC );
               break;
           } /* endswitch */

           /*************************************************************/
           /* Loop over stem form list and try to find term in dict.    */
           /*************************************************************/
           while ( (usNlpRC == LX_WRD_NT_FND_ASD) && *pszStemTerm )
           {
             if ( UTF16strcmp( pszStemTerm, pszDictTerm ) == 0 )
             {
                usNlpRC = LX_WRD_NT_FND_ASD;      // term has already been looked-up
             }
             else if ( UTF16strcmp( pszStemTerm, pszOrgTerm ) == 0 )
             {
                usNlpRC = LX_WRD_NT_FND_ASD;      // term has already been looked-up
             }
             else if ( WinFindAtomW( pUCB->hNotInBaseAtoms, pszStemTerm, ulOemCp  ) != NULLHANDLE )
             {
                usNlpRC = LX_WRD_NT_FND_ASD;      // term will not be found!
             }
             else
             {
                NlpFndEquivAsdW( pszStemTerm, // term we are looking for
                                pDCB->usDictHandle, // dictionary handle
                                pUCB->usUser, // ASD user handle
                                pDCB->aucDummy, // matching term found
                                &ulTermNumber, // number of term found
                                &ulDataLength, // length of entry data
                                &usDictHandle, // dictionary of match
                                &usNlpRC );
                if ( ASDOK(usNlpRC) ) UTF16strcpy( szFoundTerm, pszStemTerm );
                if ( usNlpRC == LX_WRD_NT_FND_ASD )
                {
                   Atom = WinAddAtomW( pUCB->hNotInBaseAtoms, pszDictTerm, ulOemCp  );
                } /* endif */
             } /* endif */
             pszStemTerm += UTF16strlenCHAR(pszStemTerm) + 1;    // continue with next term
           } /* endwhile */
           *pszOrgTerm = chFirstChar;                   // restore first character
         } /* endif */
       } /* endif */

       pCompound = NULL;               // assume no compounds available
#if defined(ASD_USE_COMPOUNDS_IN_AUT_LOOKUP)
       /***************************************************************/
       /* Try to split not found term in compounds                    */
       /***************************************************************/
       if ( (usNlpRC == LX_WRD_NT_FND_ASD) && !pUCB->fNoCompounds )
       {
         DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 8, DICT_GROUP, NULL );
         usNlpRC = MorphCompIsolation( pDCB->sLangID, pszOrgTerm,
                                      &(pUCB->usCompListSize),
                                       (PVOID*) &(pUCB->pszCompList),
                                       pDCB->ulOemCP);

         DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 9, DICT_GROUP, NULL );
         switch ( usNlpRC )
         {
           case MORPH_FUNC_NOT_SUPPORTED :
             /*********************************************************/
             /* Language does not support compound separation so      */
             /* set fNoCompounds flag in user control block           */
             /*********************************************************/
             pUCB->fNoCompounds = TRUE;
             usNlpRC = LX_WRD_NT_FND_ASD;
             break;
           case MORPH_NO_MEMORY :
             usNlpRC = LX_MEM_ALLOC_ASD;       // set short on memory RC
             break;
           case MORPH_OK :
             pCompound = pUCB->pszCompList;
             usNlpRC   = LX_RC_OK_ASD;
             break;
           default :
             usNlpRC = LX_WRD_NT_FND_ASD;
             break;
         } /* endswitch */
       } /* endif */
#endif

       /***************************************************************/
       /* Extract translation for found term or extract translations  */
       /* for all compounds of term                                   */
       /***************************************************************/
       while ( ASDOK(usNlpRC) )
       {
         CHAR_W chEntryLevelStyle = 0;                    // style prefix on entry level, 0 = none

         /*************************************************************/
         /* If compounds are processed search current compound in     */
         /* dictionary                                                */
         /*************************************************************/
         if ( pCompound )
         {
           /***************************************************************/
           /* Lookup of original compound in dictionary                   */
           /***************************************************************/
           if ( WinFindAtomW( pUCB->hNotInBaseAtoms, pCompound, ulOemCp  ) != NULLHANDLE )
           {
              usNlpRC = LX_WRD_NT_FND_ASD;      // term will not be found!
           }
           else
           {
              NlpFndEquivAsdW( pCompound,
                              pDCB->usDictHandle, // dictionary handle
                              pUCB->usUser, // ASD user handle
                              pDCB->aucDummy, // matching term found
                              &ulTermNumber, // number of term found
                              &ulDataLength, // length of entry data
                              &usDictHandle, // dictionary of match
                              &usNlpRC );
              if ( ASDOK(usNlpRC) ) UTF16strcpy( szFoundTerm, pCompound );
              if ( usNlpRC == LX_WRD_NT_FND_ASD )
              {
                 Atom = WinAddAtomW( pUCB->hNotInBaseAtoms, pCompound, ulOemCp  );
              } /* endif */
           } /* endif */

           /***********************************************************/
           /* If not found try stem form                              */
           /***********************************************************/
           if ( usNlpRC == LX_WRD_NT_FND_ASD )
           {
             AsdGetStemForm( hUCB, hDCB, pCompound, pUCB->ucStemTerm );
             if ( UTF16strcmp( pCompound, pUCB->ucStemTerm ) == 0 )
             {
                usNlpRC = LX_WRD_NT_FND_ASD;      // term will not be found!
             }
             else if ( WinFindAtomW( pUCB->hNotInBaseAtoms,
                                    pUCB->ucStemTerm, ulOemCp  ) != NULLHANDLE )
             {
                usNlpRC = LX_WRD_NT_FND_ASD;      // term will not be found!
             }
             else
             {
                NlpFndEquivAsdW( pUCB->ucStemTerm,
                                pDCB->usDictHandle, // dictionary handle
                                pUCB->usUser, // ASD user handle
                                pDCB->aucDummy, // matching term found
                                &ulTermNumber, // number of term found
                                &ulDataLength, // length of entry data
                                &usDictHandle, // dictionary of match
                                &usNlpRC );
                if ( ASDOK(usNlpRC) ) UTF16strcpy( szFoundTerm, pUCB->ucStemTerm );
                if ( usNlpRC == LX_WRD_NT_FND_ASD )
                {
                   Atom = WinAddAtomW( pUCB->hNotInBaseAtoms, pUCB->ucStemTerm, ulOemCp  );
                } /* endif */
             } /* endif */
           } /* endif */
         } /* endif */

         // check if term has already been added to the output buffer
         if ( ASDOK(usNlpRC) && fTranslate  )
         {
            USHORT usI = 0;
            fTermInBuffer = FALSE;
            while ( usI < usOutTerms )
            {
               if ( UTF16strcmp( pucOutTerms[usI], pDCB->aucDummy ) == 0 )
               {
       //    Removed 8-15-16 P403414 (showing plurals for 1st letter uppercase term)
       //         if ( ( ! fFirstLetterLowercase ) ||
       //              ( UTF16strcmp( pucOutTerms[usI], pszOrgTerm ) == 0 ) )
       //         {
                     usI = usOutTerms + 1;  // force end of loop
                     fTermInBuffer = TRUE;
       //         } 
       //         else
       //         {
       //            usI++;            // try next term
       //         }
               }
               else
               {
                  usI++;            // try next term
               } /* endif */
            } /* endwhile */
         } /* endif */

         /***************************************************************/
         /* Retrieve data for term from all dictionaries in list (if    */
         /* requested)                                                  */
         /***************************************************************/
         fFirstTranslation = TRUE;         // no translations yet
         usHeadTermExact = 0;              // no head term yet
         chEntryLevelStyle = 0;            // style prefix on entry level, 0 = none

         while ( !fTermInBuffer && ASDOK(usNlpRC) && fTranslate)
         {

            if ( ASDOK(usNlpRC) && fTranslate  )
            {
               /************************************************************/
               /* Enlarge entry data buffer if necessary      @@ CHAR_W??             */
               /************************************************************/
               if ( ulDataLength > ulBufSize )
               {
                 if ( UtlAlloc( (PVOID *)&pucDictData, ulBufSize * sizeof(CHAR_W),
                               ulDataLength * sizeof (CHAR_W), NOMSG ) )
                 {
                   ulBufSize = ulDataLength;
                 }
                 else
                 {
                   usNlpRC = LX_MEM_ALLOC_ASD;
                 } /* endif */
               } /* endif */

               if ( ASDOK(usNlpRC))
               {
                 ULONG ulDataLenBytes = 0;
                 ulDataLenBytes = ulDataLength * sizeof(CHAR_W);
                  DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 10, DICT_GROUP, NULL );
                  NlpRetEntryAsdW( usDictHandle,        // dictionary handle
                                  pUCB->usUser,         // ASD user handle
                                  pDCB->aucDummy,       // term for this entry
                                  &ulTermNumber,        // number of term
                                  (PBYTE)pucDictData,   // entry data
                                  &ulDataLenBytes,      // data length in bytes
                                  &usDictHandle,        // dictionary of term
                                  &usNlpRC );

                ulDataLength = ulDataLenBytes / sizeof(CHAR_W);
                 DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 11, DICT_GROUP, NULL );
               } /* endif */
            } /* endif */

            /***************************************************************/
            /* Process translation of term                                 */
            /***************************************************************/
            if ( ASDOK(usNlpRC) )
            {
              CHAR_W chTargetLevelStyle = 0;                   // style prefix on target level, 0 = none

              DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 12, DICT_GROUP, NULL );
              if ( fTranslate )
              {
                //
                // search translations of term
                //

                // get DCB of dictionary containing the term
                if ( pDCB->usDictHandle != usDictHandle )
                {
                   pTermDCB = (PDCB) AsdHandleToDCB( pDCB, usDictHandle );
                }
                else
                {
                   pTermDCB = pDCB;
                } /* endif */

                /*******************************************************/
                /* Add translation(s) of term to buffer                */
                /*******************************************************/
                if ( *pucDictData == QLDB_FIRST_LEVEL )
                {
                  /*****************************************************/
                  /* Process record in QLDB format                     */
                  /*****************************************************/
                  usLevel    = 0;
                  pucData = pucDictData + QLDB_START_CTRL_INFO;
                  while ( *pucData != QLDB_END_OF_REC )
                  {
                    fFieldStart = TRUE;
                    switch ( *pucData++ )
                    {
                      case QLDB_FIRST_LEVEL :
                        {
                          CHAR_W chTempStyle = 0;
                          usLevel = 1;
                          usField = 0;
                          usTransTermCount = 0;
                          fTranslationProcessed = FALSE;

                          // search and evaluate any style field
                          chTempStyle = SearchAndEvaluateStyle( &(pTermDCB->EntryStyle), pucData, usLevel, TRUE ); 

                          if ( chTempStyle )
                          {
                            if ( chTempStyle == STYLEPREFIX_NOTALLOWED )
                            {
                              // not allowed superseeds any other style
                              chEntryLevelStyle = chTempStyle;
                            }
                            else if ( chEntryLevelStyle != STYLEPREFIX_NOTALLOWED )
                            {
                              chEntryLevelStyle = chTempStyle;
                            } /* endif */
                          } /* endif */
                          chTargetLevelStyle = 0;                      // reset style indicator on target level
                        }
                        break;
                      case QLDB_SECOND_LEVEL :
                        usLevel = 2;
                        usField = 0;
                        fTranslationProcessed = FALSE;
                        chTargetLevelStyle = 0;                      // reset style indicator on target level
                        break;
                      case QLDB_THIRD_LEVEL :
                        usLevel = 3;
                        usField = 0;
                        fTranslationProcessed = FALSE;
                        chTargetLevelStyle = 0;                      // reset style indicator on target level
                        break;
                      case QLDB_FOURTH_LEVEL :
                        usLevel = 4;
                        usField = 0;
                        fTranslationProcessed = FALSE;

                        // search and evaluate style field on curent level 
                        chTargetLevelStyle = SearchAndEvaluateStyle( &(pTermDCB->TargetStyle), pucData, usLevel, FALSE );
                        break;
                      case QLDB_ESC_CHAR:
                        pucData++;
                        fFieldStart = FALSE;
                        break;
                      case QLDB_FIELD_DELIMITER :
                        usField++;
                        switch ( *pucData )
                        {
                          case QLDB_FIRST_LEVEL :
                          case QLDB_SECOND_LEVEL :
                          case QLDB_THIRD_LEVEL :
                          case QLDB_FOURTH_LEVEL :
                          case QLDB_END_OF_REC :
                            fFieldStart = FALSE;
                            break;
                          default :
                            break;
                        } /* endswitch */
                        break;
                      default :
                        fFieldStart = FALSE;
                        break;
                    } /* endswitch */

                    if ( fFieldStart )
                    {
                      // Check for whitespace only in translation
                      {
                        PSZ_W pucTemp = pucData;
                        while ( *pucTemp != EOS )
                        {
                          if ( *pucTemp != SPACE )
                          {
                            break;
                          }
                          else
                          {
                            pucTemp++;
                          } /* endif */
                        } /* endwhile */

                        if ( *pucTemp == EOS )
                        {
                          *pucData = EOS;           // nothing to display
                        } /* endif */
                      }

                      if ( *pucData )
                      {
                        if ( (pTermDCB->TransField.usLevel == usLevel ) &&
                                  (pTermDCB->TransField.usIndex == usField) )
                        {
                          // for first translation, add head term to list
                          if ( fFirstTranslation )
                          {
                             // add head term to list of terms in output buffer
                             if ( usOutTerms < MAX_TRANSL_TERMS )
                             {
                                pucOutTerms[usOutTerms++] = pucOutBuf 
                                                            + 1 /* term length field */ 
                                                            + 1 /* style indicator */ 
                                                            + lTransBufUsed;
                             } /* endif */

                             // add the term to the output buffer
                             if ( chEntryLevelStyle == 0 )
                             {
                               chEntryLevelStyle = STYLEPREFIX_UNDEFINED;
                             } /* endif */

                             {
                               // prefix term with entry level style 
                               CHAR_W szTermBuf[MAX_TERM_LEN+1];

                               szTermBuf[0] = chEntryLevelStyle;
                               UTF16strcpy( szTermBuf + 1, pDCB->aucDummy );

                               lTermStylePos = lTransBufUsed + 1;   // remember position of entry style indicator
                               lLastHeadTermPos = lTransBufUsed ;   // remember position of head term
                               lFirstTransTermPos = 0 ;
                               lLastTransTermPos = 0 ;
                               usNlpRC =  AsdAddToTermList( szTermBuf,    //  add head term
                                                            (SHORT)UTF16strlenCHAR(szTermBuf),
                                                            &usTransTerms,
                                                            &pucOutBuf,
                                                            &lTransBufSize,
                                                            &lTransBufUsed,
                                                            FALSE,
                                                            pTermDCB->usOpenNum);
                             }
                             if ( usHeadTermExact == 0 ) {
                                if ( UTF16strcmp( pszOrgTerm, pDCB->aucDummy ) == 0 )
                                   usHeadTermExact = 1;
                                else
                                   usHeadTermExact = 2;
                             }

                             fFirstTranslation = FALSE;
                          } /* endif */

                          if ( fPIDRemoveLastTerm ) {    // If last term did not match PID, then remove it 
                             fPIDRemoveLastTerm = FALSE ;
                             memset( &pucOutBuf[lLastTransTermPos], 0, (lTransBufUsed-lLastTransTermPos)*sizeof(CHAR_W) ) ;
                             lTransBufUsed = lLastTransTermPos ;
                             usTransTermCount--;
                          }
                          lLastTransTermPos = lTransBufUsed ;  // remember position of trans term 
                          usTransTermCount++;
                          fPIDRemoveLastHead = FALSE ;
                          if ( lFirstTransTermPos == 0 ) 
                             lFirstTransTermPos = lTransBufUsed ;

                          if ( chTargetLevelStyle )
                          {
                            // prefix translation with target level style
                            CHAR_W chTemp;
                            pucData--;
                            chTemp = *pucData;
                            *pucData = chTargetLevelStyle;
                            usNlpRC =  AsdAddToTermList( pucData,
                                                        (SHORT)UTF16strlenCHAR(pucData),
                                                        &usTransTerms,
                                                        &pucOutBuf,
                                                        &lTransBufSize,
                                                        &lTransBufUsed,
                                                        FALSE,
                                                        pTermDCB->usOpenNum);
                            *pucData = chTemp;
                            pucData++;
                          }
                          else
                          {
                            usNlpRC =  AsdAddToTermList( pucData,
                                                        (SHORT)UTF16strlenCHAR(pucData),
                                                        &usTransTerms,
                                                        &pucOutBuf,
                                                        &lTransBufSize,
                                                        &lTransBufUsed,
                                                        FALSE,
                                                        pTermDCB->usOpenNum);
                          } /* endif */
                          fTranslationProcessed = TRUE;

                          // add any dictionary data from temporary buffer
                          if ( fDictData && (ASDOK(usNlpRC)) &&
                               (*pszAddDictData != EOS) )
                          {
                            PSZ_W pszTemp = pszAddDictData;
                            while ( (ASDOK(usNlpRC)) && (*pszTemp != EOS) )
                            {
                              usNlpRC =  AsdAddToTermList( pszTemp,
                                                           (SHORT)UTF16strlenCHAR(pszTemp),
                                                           &usTransTerms,
                                                           &pucOutBuf,
                                                           &lTransBufSize,
                                                           &lTransBufUsed,
                                                           FALSE,
                                                           pTermDCB->usOpenNum);
                              // continue with next data in buffer
                              pszTemp += UTF16strlenCHAR(pszTemp) + 1;
                            } /* endwhile */
                            //pszAddDictData[0] = EOS; // clear buffer
                            memset(&pszAddDictData[0], 0, ADDDICTDATASIZE * sizeof(CHAR_W));
                          } /* endif */
                        }
                        else if ( fDictData )
                        {
                          USHORT usFieldInd ;       // index of field in dict.props

                          /***********************************************/
                          /* Check for marked dictionary data            */
                          /***********************************************/
                          usFieldInd = pTermDCB->ausFirstField[usLevel-1] + usField;
                          if ( usFieldInd &&
                               pTermDCB->ausNoOfFields[usLevel-1] &&
                               pTermDCB->Prop.ProfEntry[usFieldInd].fAutLookup )
                          {
                            if ( (usLevel <= pTermDCB->TransField.usLevel) &&
                                 !fTranslationProcessed )
                            {
                              // add data to temporary dictionary data buffer as translation
                              // has not been processed yet
                              CHAR_W chTemp1, chTemp2;
                              PSZ_W  pszFreeSpace;
                              pucData--;
                              chTemp1 = *pucData;
                              *pucData = usField+1;          /* Handle field=0 */
                              pucData--;
                              chTemp2 = *pucData;
                              *pucData = ASDT_POST_DICT_DATA_CHAR; /* Data before term */

                              // find end of data in buffer for additional dictionary data
                              pszFreeSpace = pszAddDictData;
                              while ( *pszFreeSpace != EOS )
                              {
                                pszFreeSpace += UTF16strlenCHAR(pszFreeSpace) + 1;
                              } /* endwhile */

                              // add data if there is still room in our buffer
                              {
                                LONG lFreeSpace = ADDDICTDATASIZE - (pszFreeSpace - pszAddDictData);
                                LONG lAddLen    = UTF16strlenBYTE(pucData) + 1;
                                if ( lFreeSpace > (lAddLen + 1) )
                                {
                                  UTF16strcpy( pszFreeSpace, pucData );
                                  pszFreeSpace[lAddLen] = EOS;
                                } /* endif */
                              }
                              *pucData = chTemp2;
                              pucData++;
                              *pucData = chTemp1;
                              pucData++;
                            }
                            else
                            {
                              // add entry information 

                              // for first translation, add head term to list
                              if ( fFirstTranslation )
                              {
                                 // add head term to list of terms in output buffer
                                 if ( usOutTerms < MAX_TRANSL_TERMS )
                                 {
                                    pucOutTerms[usOutTerms++] = pucOutBuf +
                                                                lTransBufUsed +
                                                                1; // space for dict number!
                                 } /* endif */

                                 lLastHeadTermPos = lTransBufUsed ;   // remember position of head term
                                 lFirstTransTermPos = 0 ;
                                 lLastTransTermPos = 0 ;

                                 // add the term to the output buffer
                                 usNlpRC =  AsdAddToTermList( pDCB->aucDummy,
                                                              (SHORT)UTF16strlenCHAR(pDCB->aucDummy),
                                                              &usTransTerms,
                                                              &pucOutBuf,
                                                              &lTransBufSize,
                                                              &lTransBufUsed,
                                                              FALSE,
                                                              pTermDCB->usOpenNum);
                                 fFirstTranslation = FALSE;
                              } /* endif */

                              if ( ASDOK(usNlpRC) )
                              {
                                CHAR_W chTemp1, chTemp2;

                                pucData--;
                                chTemp1 = *pucData;
                                *pucData = usField+1;          /* Handle field=0 */
                                pucData--;
                                chTemp2 = *pucData;
                                *pucData = ASDT_POST_DICT_DATA_CHAR;   /* Data after term */

                                if ( ( pUCB->fDictPIDSelect ) && // If PID selection,
                                     ( usField == pUCB->usDictPIDSelect[pTermDCB->usOpenNum-1] ) ) { // If "NL Product" field,
                                   if ( ! CheckPIDValue( pucData, szPIDValues ) ) {  // If PID does not match,
                                      fPIDRemoveLastTerm = TRUE ; // Remove the previous term
                                      if ( lLastTransTermPos == lFirstTransTermPos ) // If no translated terms
                                         fPIDRemoveLastHead = TRUE;
                                   } else {
                                      fPIDRemoveLastHead = FALSE ; 
                                   }

                                }
                                usNlpRC =  AsdAddToTermList( pucData,
                                                             (SHORT)UTF16strlenCHAR(pucData),
                                                             &usTransTerms,
                                                             &pucOutBuf,
                                                             &lTransBufSize,
                                                             &lTransBufUsed,
                                                             FALSE,
                                                             pTermDCB->usOpenNum);
                                *pucData = chTemp2;
                                pucData++;
                                *pucData = chTemp1;
                                pucData++;
                              } /* endif */
                            } /* endif */
                          } /* endif */
                        } /* endif */
                      } /* endif */
                    } /* endif */
                  } /* endwhile */


                  // Handle remaining dictionary data, adding it to the last term
                  if ( fDictData && (ASDOK(usNlpRC)) && usTransTermCount &&
                       (*pszAddDictData != EOS) )
                  {
                    PSZ_W pszTemp = pszAddDictData;
                    while ( (ASDOK(usNlpRC)) && (*pszTemp != EOS) )
                    {
                      usNlpRC =  AsdAddToTermList( pszTemp,
                                                   (SHORT)UTF16strlenCHAR(pszTemp),
                                                   &usTransTerms,
                                                   &pucOutBuf,
                                                   &lTransBufSize,
                                                   &lTransBufUsed,
                                                   FALSE,
                                                   pTermDCB->usOpenNum);
                      // continue with next data in buffer
                      pszTemp += UTF16strlenCHAR(pszTemp) + 1;
                    } /* endwhile */
                    memset(&pszAddDictData[0], 0, ADDDICTDATASIZE * sizeof(CHAR_W));
                  } /* endif */

                  if ( fPIDRemoveLastTerm ) {    /* If last term did not match PID, then remove it */
                     fPIDRemoveLastTerm = FALSE ;
                     memset( &pucOutBuf[lLastTransTermPos], 0, (lTransBufUsed-lLastTransTermPos)*sizeof(CHAR_W) ) ;
                     lTransBufUsed = lLastTransTermPos ;
                     usTransTermCount--;
                     pucOutBuf[lTransBufUsed] = EOS ;
                     pucOutBuf[lTransBufUsed+1] = EOS ;
                     if ( lLastTransTermPos == lFirstTransTermPos ) { /* If no translated terms    */
                        fPIDRemoveLastHead = TRUE ;
                     }
                  }
                  if ( fPIDRemoveLastHead ) {    /* If head term has no trans terms, then remove head term */
              //     fPIDRemoveLastHead = FALSE ;
                     memset( &pucOutBuf[lLastHeadTermPos], 0, (lTransBufUsed-lLastHeadTermPos)*sizeof(CHAR_W) ) ;
                     lTransBufUsed = lLastHeadTermPos ;
                     pucOutBuf[lTransBufUsed] = EOS ;
                     pucOutBuf[lTransBufUsed+1] = EOS ;
                     fFirstTranslation = TRUE;
                     usTransTermCount = 0 ;
                  }
                }
                else
                {
                  /*****************************************************/
                  /* Process record in the old LDB format              */
                  /*****************************************************/
                  usNlpRC = LX_INCOMP_SIG_ASD;
                } /* endif */
                DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 13, DICT_GROUP, NULL );
              }
              else
              {
                // we only want to get the terms found back ...
                usNlpRC =  AsdAddToTermList( pszOrgTerm,
                                             (SHORT)UTF16strlenCHAR( pszOrgTerm ),
                                             &usTransTerms,
                                             &pucOutBuf,
                                             &lTransBufSize,
                                             &lTransBufUsed,
                                             FALSE,
                                             pTermDCB->usOpenNum);
              } /* endif */
            } /* endif */

            /*****************************************************/
            /* Search remaining dictionaries (if requested)      */
            /*****************************************************/
            if ( ASDOK(usNlpRC) && fSearchAll )
            {
              DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 14, DICT_GROUP, NULL );
              // loop over remaining dictionaries until found or end-of-list
              do
              {
                // position to next dictionary
                usDictHandle = DAMGetNextDict( usDictHandle );

                // try to get term from dictionary
                if ( usDictHandle && szFoundTerm[0] )
                {
                  NlpFndEquivAsdW(szFoundTerm,    // contains the term found in prev. dict
                                  usDictHandle,   // dictionary handle
                                  pUCB->usUser,   // ASD user handle
                                  pDCB->aucDummy, // matching term found
                                  &ulTermNumber,  // number of term found
                                  &ulDataLength,  // length of entry data
                                  &usDictHandle,  // dictionary of match
                                  &usNlpRC );

                  if ( ( usHeadTermExact == 1 ) &&
                       ( UTF16strcmp( pszOrgTerm, pDCB->aucDummy ) != 0 ) ) {
                     usNlpRC = LX_WRD_NT_FND_ASD;
                  } else
                  if ( ( usHeadTermExact == 2 ) &&
                       ( ! fFirstLetterUppercase ) &&
                       ( UTF16strcmp( pszOrgTerm, pDCB->aucDummy ) == 0 ) )
                  {
                     usHeadTermExact = 1;
                     memset( &pucOutBuf[lLastHeadTermPos], 0, (lTransBufUsed-lLastHeadTermPos)*sizeof(CHAR_W) ) ;
                     lTransBufUsed = lLastHeadTermPos ;
                     pucOutBuf[lTransBufUsed] = EOS ;
                     pucOutBuf[lTransBufUsed+1] = EOS ;
                     fFirstTranslation = TRUE;
                     usTransTermCount = 0 ;
                  }
                } /* endif */
              }
              while( usDictHandle && (usNlpRC == LX_WRD_NT_FND_ASD) ); /* endwhile */
              DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 15, DICT_GROUP, NULL );
            }
            else
            {
              usNlpRC = LX_WRD_NT_FND_ASD; // force end of while loop
            } /* endif */
         } /* endwhile */

         if ( ( fTranslate && !fFirstTranslation ) &&             // if terms added to buffer
              ( ( ! pUCB->fDictPIDSelect ) ||                     // no PID processing
                  ( ! fPIDRemoveLastHead ) ) )                   // Complete head not removed
         {
            // terminate list for current term
            usNlpRC =  AsdAddToTermList( EMPTY_STRINGW,
                                         0,
                                         &usTransTerms,
                                         &pucOutBuf,
                                         &lTransBufSize,
                                         &lTransBufUsed,
                                         FALSE,
                                         0);
            // adjust term style if required
            if ( chEntryLevelStyle && (chEntryLevelStyle != STYLEPREFIX_UNDEFINED) )
            {
              pucOutBuf[lTermStylePos] = chEntryLevelStyle;
            } /* endif */
         } /* endif */

         // reset any not-found return code
         if ( usNlpRC == LX_WRD_NT_FND_ASD )
         {
           usNlpRC = LX_RC_OK_ASD;
         } /* endif */

         /*************************************************************/
         /* Continue with next compound if compounds are processed    */
         /*************************************************************/
         if ( pCompound )
         {
           pCompound += UTF16strlenCHAR(pCompound) + 1;
           if ( *pCompound == EOS )
           {
             /*********************************************************/
             /* End of decomposition has been reached, try next one   */
             /*********************************************************/
             pCompound++;
             if ( *pCompound == EOS )
             {
               /*********************************************************/
               /* End of list has been reached, stop compound processing*/
               /*********************************************************/
               pCompound = NULL;
               usNlpRC   = LX_WRD_NT_FND_ASD;
             } /* endif */
           } /* endif */
         }
         else
         {
           usNlpRC = LX_WRD_NT_FND_ASD; // force end of loop
         } /* endif */
       } /* endwhile */

       /***************************************************************/
       /* Reset return code if a term was not found                   */
       /***************************************************************/
       fSplitTerm = FALSE ; 
       fPlural2Singular = FALSE ; 
       fPlural2SingularUpper = FALSE ; 
       if ( usNlpRC == LX_WRD_NT_FND_ASD )
       {
          usNlpRC = LX_RC_OK_ASD;

          /***************************************************************/
          /* Special cases when term was not found.                      */
          /***************************************************************/
          if ( ( ! fTranslate ) || 
               ( fFirstTranslation ) ) {     
             /************************************************************/
             /* Split term if it contains a possible concatenation       */
             /* character, like dash, slash, period, quote, etc.         */
             /************************************************************/
             if ( ( wcschr( pszOrgTerm+1, L'-'  ) ) ||
                  ( wcschr( pszOrgTerm+1, L'/'  ) ) ||
                  ( wcschr( pszOrgTerm+1, L'.'  ) ) ||
                  ( wcschr( pszOrgTerm+1, L'\'' ) ) ) {
                fSplitTerm = TRUE ; 
                PSZ_W  ptrTemp ;
                for( ptrTemp=pszOrgTerm+1 ; *ptrTemp ; ++ptrTemp ) {
                   if ( wcschr( L"-/.\'", *ptrTemp ) ) {
                      *ptrTemp = L'\0';
                      if ( *(ptrTemp+1) ) /* Each term must be at least 1 char */
                         ++ptrTemp;
                   }
                }
                for( ptrTemp=pszDictTerm+1 ; *ptrTemp ; ++ptrTemp ) {
                   if ( wcschr( L"-/.\'", *ptrTemp ) ) {
                      *ptrTemp = L'\0';
                      if ( *(ptrTemp+1) ) /* Each term must be at least 1 char */
                         ++ptrTemp;
                   }
                }
             }
             else
             {
                /************************************************************/
                /* If possible plural term which was not found in the       */
                /* HunSpell dictionaries, try singular version of term.     */
                /************************************************************/
                int i = wcslen(pszOrgTerm);
                if ( ( i > 4 ) &&
                     ( ! fFirstLetterUppercase ) &&
                     ( *(pszOrgTerm+i-1) == L's' ) &&
                     ( *(pszOrgTerm+i-2) != L's' ) ) {
                   fPlural2Singular = TRUE ;
                   memmove( pszOrgTerm+1, pszOrgTerm, (i-1)*sizeof(WCHAR) ) ;
                   memmove( pszDictTerm+1, pszDictTerm, (i-1)*sizeof(WCHAR) ) ;
                   pszOrgTerm++;
                   pszDictTerm++;
                   if ( fFirstLetterLowercase ) 
                      fPlural2SingularUpper = TRUE ;
                   else
                      fPlural2SingularUpper = FALSE ;
                }
                else
                if ( ( i > 4 ) &&                                /* 9-13-16 */
                     ( ! fFirstLetterUppercase ) &&
                     ( ! wcsncmp((pszOrgTerm+i-2), L"ed", 2 ) ) ) {
                   usEDSuffix = 2 ;            /* Remove 'D', stored->store */
                   memmove( pszOrgTerm+1, pszOrgTerm, (i-1)*sizeof(WCHAR) ) ;
                   memmove( pszDictTerm+1, pszDictTerm, (i-1)*sizeof(WCHAR) ) ;
                   pszOrgTerm++;
                   pszDictTerm++;
                }
                else
                if ( usEDSuffix ) {
                   --usEDSuffix ;
                   if ( usEDSuffix ) {         /* Remove "ED", constructed->contruct */
                      memmove( pszOrgTerm+1, pszOrgTerm, (i-1)*sizeof(WCHAR) ) ;
                      memmove( pszDictTerm+1, pszDictTerm, (i-1)*sizeof(WCHAR) ) ;
                      pszOrgTerm++;
                      pszDictTerm++;
                   }
                }
             }
          } else {
             usEDSuffix = 0 ;
          }
       } /* endif */

       /***************************************************************/
       /* Look for first term in segment with lowercase letter.       */
       /***************************************************************/
       fFirstLetterLowercase = FALSE;
       if ( fFirstLetterUppercase ) {
          fFirstLetterUppercase = FALSE;
          fFirstLetterLowercase = TRUE;
          *pszOrgTerm = UtlToLowerW( (CHAR_W)*pszOrgTerm );
          *pszDictTerm = UtlToLowerW( (CHAR_W)*pszDictTerm );
       }
       else

       /***************************************************************/
       /* Continue with next term                                     */
       /***************************************************************/
       if ( ( ! fSplitTerm ) &&
            ( ! fPlural2Singular ) &&
            ( usEDSuffix == 0 ) ) {
          pszOrgTerm  += UTF16strlenCHAR(pszOrgTerm) + 1;
          pszDictTerm += UTF16strlenCHAR(pszDictTerm) + 1;
       }
       else

       if ( ( fPlural2Singular ) &&
            ( fPlural2SingularUpper ) ) {
          fFirstLetterUppercase = TRUE ;
          *pszOrgTerm = UtlToUpperW( (CHAR_W)*pszOrgTerm );
          *pszDictTerm = UtlToUpperW( (CHAR_W)*pszDictTerm );
       }
     } /* endwhile */
   } /* endif */

   /*******************************************************************/
   /* Terminate translations in buffer without any error checks       */
   /*******************************************************************/
   if ( ( ! pUCB->fDictPIDSelect ) || 
        ( ( usTransTermCount ) && ( ! fPIDRemoveLastHead ) ) )
   {
      AsdAddToTermList( EMPTY_STRINGW,
                        0,
                        &usTransTerms,
                        &pucOutBuf,
                        &lTransBufSize,
                        &lTransBufUsed,
                        FALSE,
                        0 );
   }
   fPIDRemoveLastHead = FALSE ;


   if ( usNlpRC != LX_RC_OK_ASD )
   {
     ERREVENT( ASDTRANSLATE_LOC, INTFUNCFAILED_EVENT, usNlpRC );
   } /* endif */


   /*******************************************************************/
   /* Remove duplicate translations for same term from list           */
   /*******************************************************************/
   if ( usNlpRC == LX_RC_OK_ASD )
   {
     PSZ_W  pTerm;                   // ptr to start of current term

     DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 16, DICT_GROUP, NULL );

#ifdef ASDT_LOG_TERMLIST
     {
       CHAR szLogFileName[MAX_EQF_PATH];
       UtlMakeEQFPath( szLogFileName, NULC, LOG_PATH, NULL );
       strcat( szLogFileName, "\\ASDTTERMLIST.LOG" ); 
       hfLog = fopen( szLogFileName, "a" );
       if ( hfLog )
       {
          fwprintf( hfLog, L"Segment=\"%s\"\r\n", pucSegment );
          WriteTermListToLog( "Before reducing duplicate translations", pucOutBuf, hfLog );
       } /* endif */        
     }
#endif


     pTerm = pucOutBuf;

     while ( *pTerm != EOS)            // while not end of terms in list
     {
       PSZ_W pFirstTrans;              // ptr to first translation of term

       // skip term and position to first translation
       pFirstTrans = pTerm + UTF16strlenCHAR(pTerm) + 1;

       // check term translations
       if ( *pFirstTrans != EOS )
       {
         PSZ_W pNextTrans;
         PSZ_W pCurTrans;
//       PSZ_W pucEndOfTrans;

//       // find end of term translations
//       pucEndOfTrans = pFirstTrans; 
//       while ( *pucEndOfTrans != EOS ) pucEndOfTrans = pucEndOfTrans + AsdGetTransLen(pucEndOfTrans);

         /*------------------------------------------------------------------*/
         /*  1st run.  Sort results on dictionary indicator, style, and      */
         /*            translation.                                          */
         /*------------------------------------------------------------------*/
         {
           BOOL fSwapped = TRUE;
           while ( fSwapped )
           {
             PSZ_W pEntry1 = pFirstTrans;               /* Entry includes leading comments */
             PSZ_W pTerm1  = AsdGetTransText(pEntry1);  /* Term is actual text */
             PSZ_W pEntry2 = pEntry1 + AsdGetTransLen(pEntry1);
             PSZ_W pTerm2  = AsdGetTransText(pEntry2);
             fSwapped = FALSE;
             while ( *pTerm2 != EOS )
             {
               PSZ_W pszComp1 = pTerm1 + 1;  // skip dictionary indicator
               PSZ_W pszComp2 = pTerm2 + 1;  // skip dictionary indicator

               int iStyle1 = 0;            // style flag of term 1
               int iStyle2 = 0;            // style flag of term 2
               int result = 0;

               // sort on dictionary indicator
               result = *pTerm1 - *pTerm2; 

               // sort on style
               if ( result == 0 )
               {
                 if (  *pszComp1 ==  STYLEPREFIX_NOTALLOWED )
                 {
                   iStyle1 = -1;
                   pszComp1 += 1;             // skip style indicator
                 } 
                 else if (  *pszComp1 ==  STYLEPREFIX_PREFERRED )
                 {
                   iStyle1 = 1;
                   pszComp1 += 1;             // skip style indicator
                 } /* endif */                  
                 if (  *pszComp2 ==  STYLEPREFIX_NOTALLOWED )
                 {
                   iStyle2 = -1;
                   pszComp2 += 1;             // skip style indicator
                 } 
                 else if (  *pszComp2 ==  STYLEPREFIX_PREFERRED )
                 {
                   iStyle2 = 1;
                   pszComp2 += 1;             // skip style indicator
                 } /* endif */       

                 result = iStyle2 - iStyle1;
               } /* endif */                  

               // sort on translation
               if ( result == 0 )
               {
                 result = _wcsicoll( pszComp1, pszComp2  );
               } /* endif */                  


               
               if ( result > 0 )
               {
                 // swap terms
                 int iLen1 = AsdGetTransLen(pEntry1);
                 int iLen2 = AsdGetTransLen(pEntry2);
                 memcpy( pucDictData, pEntry1, iLen1 * sizeof(CHAR_W) );
                 memmove( pEntry1, pEntry2, iLen2 * sizeof(CHAR_W) );
                 memcpy( pEntry1 + iLen2, pucDictData, iLen1 * sizeof(CHAR_W) );
                 pEntry2 = pEntry1 + iLen2;
                 pTerm2 = AsdGetTransText(pEntry2);
                 fSwapped = TRUE;
               } /* endif */              
               pEntry1 = pEntry2;
               pTerm1 = pTerm2;
               pEntry2 = pEntry1 + AsdGetTransLen(pEntry1);
               pTerm2 = AsdGetTransText(pEntry2);
             } /* endwhile */            
           } /* endwhile */              
         }
#ifdef ASDT_LOG_TERMLIST
         if ( hfLog )
         {
           WriteTermListToLog( "Before 2nd run", pucOutBuf, hfLog );
         } /* endif */        
#endif
         /*------------------------------------------------------------------*/
         /*  2nd run.  Merge together duplicate translations.                */
         /*            Keep the dictionary number and merge together any     */
         /*            post dictionary information.                          */
         /*------------------------------------------------------------------*/
         pCurTrans = pFirstTrans;
         while ( *pCurTrans != EOS )          // Loop through all term entries.     
         {
            PSZ_W pCurTerm  = AsdGetTransText(pCurTrans);  
            int iCurTermLen = AsdGetTransLen(pCurTrans);

            /*----------------------------------------------------------------*/
            /*  For current entry, see if there is information that can be    */
            /*  merged together within this current entry.                    */
            /*----------------------------------------------------------------*/
            if ( iCurTermLen > UTF16strlenCHAR(pCurTrans)+1 ) {
              PSZ_W   pucPost1, pucPost2 ;
              USHORT  usField1, usField2 ;
              int     iUsedLen = 0;
              pucPost1  = pCurTerm + UTF16strlenCHAR( pCurTerm ) + 1 ;
              if ( pucPost1 ) {
                usField1 = pucPost1[2] ;
                pucPost2 = pucPost1 + UTF16strlenCHAR( pucPost1 ) + 1 ;
                usField2 = pucPost2[2] ;
                while ( ( pucPost2[0] != 0 ) && 
                        ( pucPost2[1] == ASDT_POST_DICT_DATA_CHAR ) ) {
                  if ( usField2 == usField1 ) {     /* Merge entry */

                    /*--------------------------------------------------*/
                    /*  Add new token at end of current information.    */
                    /*--------------------------------------------------*/
                    if ( ! wcsstr( pucPost1+3, pucPost2+3 ) ) {
                      int iLen1 = UTF16strlenCHAR( pucPost1 ) ;
                      int iLen2 = UTF16strlenCHAR( pucPost2+3 ) ;
                      int iSepLen = 2 ;
                      int iAdjust = iLen2 + iSepLen ;
                      if ( ( iLen1 < 40 ) &&              /* Max displayed text */
                           ( lTransBufUsed + iAdjust + 1 < usOutBufSize ) ) {
                         iUsedLen = (PBYTE)(pucPost1+iLen1) - (PBYTE)pucOutBuf;
                         memmove( (PBYTE)(pucPost1+iLen1+iAdjust), (PBYTE)(pucPost1+iLen1), ((lTransBufUsed+2)*sizeof(CHAR_W)) - iUsedLen );
                         wcsncpy( (pucPost1+iLen1), L"; ", iSepLen ) ;
                         iCurTermLen += iAdjust ;
                         pucPost2 += iAdjust ;
                         lTransBufUsed += iAdjust ;
                         wcsncpy( (pucPost1+iLen1+iSepLen), pucPost2+3, iLen2 ) ;  /* Merge text */
                      }
                    }
  
                    /*--------------------------------------------------*/
                    /*  Remove merged entry.                            */
                    /*--------------------------------------------------*/
                    int iLen2 = UTF16strlenCHAR( pucPost2 ) + 1 ;
                    iCurTermLen -= iLen2;
                    iUsedLen = (PBYTE)(pucPost2+iLen2) - (PBYTE)pucOutBuf;
                    if ( iUsedLen >= (int)(usOutBufSize*sizeof(CHAR_W)) ) {
                       *pucPost2 = EOS ;
                       lTransBufUsed -= iLen2;
                       break ;
                    }
                    memmove( (PBYTE)pucPost2, (PBYTE)(pucPost2+iLen2), ((lTransBufUsed+2)*sizeof(CHAR_W)) - iUsedLen );
                    lTransBufUsed -= iLen2;
                    continue;
                  }
                  pucPost2 += UTF16strlenCHAR( pucPost2 ) + 1 ;
                }
              }
            } 
  

            /*----------------------------------------------------------------*/
            /*  For each entry, check all remaining entries to identify       */
            /*  entries with the same translated term.                        */
            /*----------------------------------------------------------------*/
           pNextTrans = pCurTrans + iCurTermLen;
           while ( *pNextTrans != EOS ) {
             PSZ_W pNextTerm  = AsdGetTransText(pNextTrans); 
             int iNextTermLen = AsdGetTransLen(pNextTrans);
             PSZ_W pCurComp  = pCurTerm  + 1;  // skip dictionary indicator
             PSZ_W pNextComp = pNextTerm + 1;  // skip dictionary indicator
             CHAR_W chCurStyle = NULL ;
             CHAR_W chNextStyle = NULL ;

           /*  For each entry, check all remaining entries to identify       */
           /*  entries with the same translated term.                        */
             if ( ( *pCurComp ==  STYLEPREFIX_PREFERRED  ) ||
                  ( *pCurComp ==  STYLEPREFIX_NOTALLOWED ) ) {
                chCurStyle = *pCurComp ;
                ++pCurComp ;                  // skip style
             }
             if ( ( *pNextComp ==  STYLEPREFIX_PREFERRED  ) ||
                  ( *pNextComp ==  STYLEPREFIX_NOTALLOWED ) ) {
                chNextStyle = *pNextComp ;
                ++pNextComp ;                 // skip style
             }

             /*--------------------------------------------------------------*/
             /*  Remove duplicate entries which are exactly the same.        */
             /*--------------------------------------------------------------*/
             if ( ( iNextTermLen == iCurTermLen ) && 
                  ( AsdIsTranslationIdentical( pNextTrans, pCurTrans, iNextTermLen ) ) )  {
               int iUsedLen = 0;

               *pCurTrans |= *pNextTrans;         /* Merge dictionary number */

               // remove duplicate translation entry.
               iUsedLen = (PBYTE)(pNextTrans+iNextTermLen) - (PBYTE)pucOutBuf;
               if ( iUsedLen >= (int)(usOutBufSize*sizeof(CHAR_W)) ) {
                  *pNextTrans = EOS ;
               } else {
                  memmove( (PBYTE)pNextTrans, (PBYTE)(pNextTrans + iNextTermLen), ((lTransBufUsed+2)*sizeof(CHAR_W)) - iUsedLen );
               }
               lTransBufUsed -= iNextTermLen;
               continue;
             }
             
             /*--------------------------------------------------------------*/
             /*  Merge dictionary information for entries which have the     */
             /*  same translated term.                                       */
             /*--------------------------------------------------------------*/
             if ( ! wcscmp( pNextComp, pCurComp ) ) {
                PSZ_W   pucCurPost, pucNextPost ;
                USHORT  usCurField, usNextField ;
                int     iUsedLen = 0;
                BOOL    bGetCur = FALSE ;
                BOOL    bGetNext = FALSE ;
                BOOL    bGetCurEOF = FALSE ;

                *pCurTrans |= *pNextTrans;        /* Merge dictionary number */

                /*-----------------------------------------------------------*/
                /*  Merge the style for next entry into the current entry.   */
                /*-----------------------------------------------------------*/
                if ( ( chCurStyle  == NULL ) &&
                     ( chNextStyle != NULL ) ) {
                   iUsedLen = (PBYTE)pCurComp - (PBYTE)pucOutBuf;
                   if ( iUsedLen < (int)(usOutBufSize*sizeof(CHAR_W)) ) {
                     memmove( (PBYTE)(pCurComp+1), (PBYTE)pCurComp, ((lTransBufUsed+2)*sizeof(CHAR_W)) - iUsedLen );
                     *pCurComp = chNextStyle ;
                     lTransBufUsed += 1 ;
                     pNextTrans += 1 ;
                   }
                }

                /*-----------------------------------------------------------*/
                /*  Merge all dictionary information for this next entry     */
                /*  into the current entry.                                  */
                /*-----------------------------------------------------------*/
                pucCurPost  = pCurTerm  + UTF16strlenCHAR( pCurTerm )  + 1 ;
                usCurField = pucCurPost[2] ;
                pucNextPost = pNextTerm + UTF16strlenCHAR( pNextTerm ) + 1 ;
                usNextField = pucNextPost[2] ;

                while( ( pucNextPost[0] != 0 ) && 
                       ( pucNextPost[1] == ASDT_POST_DICT_DATA_CHAR ) ) {
                   if ( bGetCur ) {             /* Get next current field    */
                      pucCurPost  = pucCurPost + UTF16strlenCHAR( pucCurPost ) + 1 ;
                      if ( (pucCurPost[0] == 0) || (pucCurPost[1] != ASDT_POST_DICT_DATA_CHAR) ) {
                         bGetCurEOF = TRUE ;
                      } else {
                         usCurField = pucCurPost[2] ;  /* Set field number */
                      }
                   }
                   if ( bGetNext ) {            /* Get next next field */
                      pucNextPost = pucNextPost + UTF16strlenCHAR( pucNextPost ) + 1 ;
                      if ( (pucNextPost[0] == 0) || (pucNextPost[1] != ASDT_POST_DICT_DATA_CHAR) ) {
                         break ;
                      } 
                      usNextField = pucNextPost[2] ;   /* Set field number */
                   }

                   /*--------------------------------------------------------*/
                   /*  Copy this new field from the next entry to current.   */
                   /*--------------------------------------------------------*/
                   if ( ( bGetCurEOF ) ||
                        ( usNextField < usCurField ) ) {   /* Add new entry */
                      int iNextLen = UTF16strlenCHAR( pucNextPost ) + 1 ;
                      iUsedLen = (PBYTE)pucCurPost - (PBYTE)pucOutBuf;
                      if ( iUsedLen < (int)(usOutBufSize*sizeof(CHAR_W)) ) {
                         memmove( (PBYTE)(pucCurPost+iNextLen), (PBYTE)pucCurPost, ((lTransBufUsed+2)*sizeof(CHAR_W)) - iUsedLen );
                         wcsncpy( pucCurPost, pucNextPost+iNextLen, iNextLen ) ;
                         lTransBufUsed += iNextLen ;
                         pucNextPost += iNextLen ;
                         pNextTrans += iNextLen ;
                         bGetCur = TRUE ;
                         bGetNext = TRUE ;
                      }

                   } else

                   /*--------------------------------------------------------*/
                   /*  Merge the next field content into the current.        */
                   /*--------------------------------------------------------*/
                   if ( usNextField == usCurField ) {       /* Merge entries */
                      CHAR_W    szTokens[512] ;
                      PCHAR_W   ptrTok, ptrEnd, ptrTemp ;
                      BOOL  bLastToken = FALSE ;
  
                      wcsncpy( szTokens, pucNextPost+3, sizeof(szTokens)/sizeof(CHAR_W) ) ;
                      szTokens[sizeof(szTokens)/sizeof(CHAR_W)-1] = NULL ;
                      for( ptrTok=szTokens ; *ptrTok ; ) {
                         /* Get next token, separated by semicolon */
                         for( ; *ptrTok && iswspace(*ptrTok) ; ++ptrTok ) ; /* Strip leading blanks */
                         if ( ! *ptrTok ) 
                            break;
                         ptrEnd = wcschr( ptrTok, L';' ) ;
                         if ( ptrEnd ) { 
                            for( ptrTemp=ptrEnd-1 ; ptrTemp>ptrTok && iswspace(*ptrTemp) ; --ptrTemp ) ;
                            *(ptrTemp+1) = NULL ;
                         } else {
                            ptrEnd = ptrTok + wcslen(ptrTok) ;
                            bLastToken = TRUE ;
                         }
                         /*--------------------------------------------------*/
                         /*  Add new token at end of current information.    */
                         /*--------------------------------------------------*/
                         if ( ! wcsstr( pucCurPost, ptrTok ) ) {
                            int iTokLen  = UTF16strlenCHAR( ptrTok ) ;
                            int iCurLen = UTF16strlenCHAR( pucCurPost ) ;
                            int iSepLen = 2 ;
                            int iAdjust;
                            iUsedLen = (PBYTE)(pucCurPost+iCurLen) - (PBYTE)pucOutBuf;
                            if ( ( iUsedLen < (int)(usOutBufSize*sizeof(CHAR_W)) ) &&
                                 ( iCurLen < 40 ) ) {    /* Max displayed text */
                               iAdjust = iTokLen + iSepLen ;
                               memmove( (PBYTE)(pucCurPost+iCurLen+iAdjust), (PBYTE)(pucCurPost+iCurLen), ((lTransBufUsed+2)*sizeof(CHAR_W)) - iUsedLen );
                               wcsncpy( (pucCurPost+iCurLen), L"; ", iSepLen ) ;
                               wcsncpy( (pucCurPost+iCurLen+iSepLen), ptrTok, iTokLen ) ;
                               lTransBufUsed += iAdjust ;
                               pucNextPost += iAdjust ;
                               pNextTrans += iAdjust ;
                            }
                         }
                         if ( bLastToken )
                            break;
                         ptrTok = ptrEnd + 1 ;  /* Next token */
                      }
                      bGetCur = TRUE ;
                      bGetNext = TRUE ;
                   } else {            /* Skip the current entry since no corresponding next entry */
                      bGetCur = TRUE ;
                      bGetNext = FALSE ;
                   }
                }

                /*--------------------------------------------------------*/
                /*  All next entry information has been merged into the   */
                /*  current entry.  Remove the merged next entry.         */
                /*--------------------------------------------------------*/
                iUsedLen = (PBYTE)(pNextTrans+iNextTermLen) - (PBYTE)pucOutBuf;
                if ( iUsedLen >= (int)(usOutBufSize*sizeof(CHAR_W)) ) {
                   *pNextTrans = EOS ;
                } else {
                   memmove( (PBYTE)pNextTrans, (PBYTE)(pNextTrans + iNextTermLen), ((lTransBufUsed+2)*sizeof(CHAR_W)) - iUsedLen );
                }
                lTransBufUsed -= iNextTermLen;
             }
             else
             {
               // These entries do not match, so get next entry to check with current.
               pNextTrans += iNextTermLen;
             } /* endif */                
           } /* endwhile */              

           // This current entry is complete.  
           // Remove the field byte from the post data.
           PSZ_W pucPost  = pCurTrans + UTF16strlenCHAR(pCurTrans) + 1 ;
           while ( (pucPost[0] != 0) && (pucPost[1] == ASDT_POST_DICT_DATA_CHAR) )
           {
              int iUsedLen = (PBYTE)pucPost - (PBYTE)pucOutBuf;
              if ( iUsedLen >= (int)(lTransBufUsed*sizeof(CHAR_W)) ) 
                 break ;
              memmove( (PBYTE)(pucPost+2), (PBYTE)(pucPost+3), ((lTransBufUsed+2)*sizeof(CHAR_W)) - iUsedLen );
              --lTransBufUsed ;
              pucPost += UTF16strlenCHAR( pucPost ) + 1 ;
           }

           // Get the next term to check.
           pCurTrans += AsdGetTransLen(pCurTrans);
         } /* endwhile */

         // continue with next source term
         pTerm = pCurTrans + 1;
       }
       else
       {
         // no translations for this term.  Continue with next one
         pTerm = pFirstTrans + 1;
       } /* endif */
     } /* endwhile */
     DEBUGEVENT2( ASDTRANSLATE_LOC, STATE_EVENT, 17, DICT_GROUP, NULL );
#ifdef ASDT_LOG_TERMLIST
     if ( hfLog )
     {
        WriteTermListToLog( "After reducing duplicate translations", pucOutBuf, hfLog );
     } /* endif */        
#endif
   } /* endif */

   // cleanup
   if ( pszAddDictData ) UtlAlloc( (PVOID *)&pszAddDictData, 0L, 0L, NOMSG );
   if ( pucDictData ) UtlAlloc( (PVOID *)&pucDictData, 0L, 0L, NOMSG );
#ifdef ASDT_LOG_TERMLIST
   if ( hfLog ) fclose( hfLog );
#endif

   DEBUGEVENT2( ASDTRANSLATE_LOC, FUNCEXIT_EVENT, 0, DICT_GROUP, NULL );

   return( usNlpRC );
} /* end of AsdTranslate */

//+----------------------------------------------------------------------------+
//| AsdGetTransLen      Get length of translation in term list                 |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PUCHAR   pucTrans            IN     pointer to term translation         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|     LONG    ulLen       length of translation plus length of add. dict data|
//+----------------------------------------------------------------------------+
static LONG AsdGetTransLen
(
  PSZ_W pucTrans                      // pointer to translation in term list
)
{
  LONG lLen = 0;                      // length of translation
  LONG lAddLen ;

  while ( (pucTrans[0] != 0) && (pucTrans[1] == ASDT_PRE_DICT_DATA_CHAR) )
  {
    lAddLen = UTF16strlenCHAR( pucTrans ) + 1;
    lLen += lAddLen;
    pucTrans += lAddLen;
  } /* endwhile */

  if ( pucTrans[0] != 0 ) {
     lAddLen = UTF16strlenCHAR( pucTrans ) + 1;       // add length of translation
     lLen += lAddLen;
     pucTrans += lAddLen;
  }

  while ( (pucTrans[0] != 0) && (pucTrans[1] == ASDT_POST_DICT_DATA_CHAR) )
  {
    lAddLen = UTF16strlenCHAR( pucTrans ) + 1;
    lLen += lAddLen;
    pucTrans += lAddLen;
  } /* endwhile */

  return( lLen );
} /* end of function AsdGetTransLen */

//+----------------------------------------------------------------------------+
//| AsdGetTransText     Get translation text in term list                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PSZ_W    pucTrans            IN     pointer to term translation         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|     PSZ_W   pucTrans    Skip dict. data to find actual term text.          |
//+----------------------------------------------------------------------------+
static PSZ_W AsdGetTransText
(
  PSZ_W pucTrans                      // pointer to translation in term list
)
{
  while ( (pucTrans[0] != 0) && (pucTrans[1] == ASDT_PRE_DICT_DATA_CHAR) )
  {
    pucTrans += UTF16strlenCHAR( pucTrans ) + 1;
  } /* endwhile */
  return( (PSZ_W)pucTrans );
} /* end of function AsdGetTransText */

// compare two translation entries ignoring dictionary indicators
static BOOL AsdIsTranslationIdentical
(
  PSZ_W pucTrans1,                   // pointer to translation 1
  PSZ_W pucTrans2,                   // pointer to translation 2
  int   iLen                         // overall length of translations 1 and 2

)
{
  // skip first dictionary indicator
  if ( iLen != 0 )
  {
    pucTrans1++;
    pucTrans2++;
    iLen--;
  } /* endif */     

  while ( (iLen != 0) && (*pucTrans1 == *pucTrans2) )
  {
    BOOL fEndOfEntry = (*pucTrans1 == 0 );


    pucTrans1++;
    pucTrans2++;
    iLen--;

    // at end of entry we have to skip the following dictionary indicator as well
    if ( fEndOfEntry &&  (iLen != 0) )
    {
      pucTrans1++;
      pucTrans2++;
      iLen--;
    } /* endif */       
  } /* endwhile */     
  return( iLen == 0 );
} /* end of AsdIsTranslationIdentical */

//+----------------------------------------------------------------------------+
//| AsdGetTranslation   Extract translations from given term data              |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HUCB     hUCB                IN     user control block handle           |
//|    HDCB     hDCB                IN     dictionary control block handle     |
//|    PUCHAR   pucTermData         IN     ptr to term data                    |
//|    USHORT   usOutBufSize        IN     size of outbuf                      |
//|    PUCHAR   pucOutBuf           OUT    buffer for translations;            |
//|                                        format is:                          |
//|                                           TRANS1\0TRANS2\0..TRANSn\0\0     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code:                              |
//|                         LX_RC_OK_ASD        =  OK                          |
//|                         LX_DATA_2_LRG_ASD   =  output buffer overflow      |
//|                         LX_...              =  other ASD errors            |
//|                         ASD_OVERFLOW    term buffer overflow               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen.                              |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//+----------------------------------------------------------------------------+

USHORT AsdGetTranslationW
(
   HUCB     hUCB,                      //     user control block handle
   HDCB     hDCB,                      //     dictionary control block handle
   PSZ_W    pucDictData,               //     ptr to term data
   USHORT   usOutBufSize,              //     size of outbuf in number of wide characters
   PSZ_W    pucOutBuf,                 //     buffer for translations
   SHORT    sListType                  //     list type (for future use)
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer
   PSZ_W     pucData;                  // ptr to begin of dictionary entry data
   USHORT    usTransBufUsed = 0;       // number of bytes used in translation list
   BOOL      fFieldStart;              // TRUE = position is on a field start
   USHORT    usLevel;                  // current dictionary level
   USHORT    usField = 0;              // current dictionary field

   pUCB = (PUCB) hUCB;         // convert handles to pointer
   pDCB = (PDCB) hDCB;

   sListType;

   //
   // check input data
   //
   CHECKUCB( pUCB, usNlpRC );  // check user control block pointer
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      CHECKDCB( pDCB, usNlpRC ); // check dictionary control block pointer
   } /* endif */
   if ( usNlpRC == LX_RC_OK_ASD )
   {
      // check data areas
      if ( !pucDictData || !pucOutBuf || !usOutBufSize )
      {
         usNlpRC = LX_UNEXPECTED_ASD;
      } /* endif */
   } /* endif */

   //
   // extract translation from input data
   //
   if ( usNlpRC == LX_RC_OK_ASD )
   {
     if ( *pucDictData == QLDB_FIRST_LEVEL )
     {
       /*****************************************************/
       /* Process record in QLDB format                     */
       /*****************************************************/
       usLevel = 0;
       pucData = pucDictData + QLDB_START_CTRL_INFO;
       while ( *pucData != QLDB_END_OF_REC )
       {
         fFieldStart = TRUE;
         switch ( *pucData++ )
         {
           case QLDB_FIRST_LEVEL :
             usLevel = 1;
             usField = 0;
             break;
           case QLDB_SECOND_LEVEL :
             usLevel = 2;
             usField = 0;
             break;
           case QLDB_THIRD_LEVEL :
             usLevel = 3;
             usField = 0;
             break;
           case QLDB_FOURTH_LEVEL :
             usLevel = 4;
             usField = 0;
             break;
           case QLDB_ESC_CHAR:
             pucData++;
             fFieldStart = FALSE;
             break;
           case QLDB_FIELD_DELIMITER :
             usField++;
             break;
           default :
             fFieldStart = FALSE;
             break;
         } /* endswitch */

         if ( fFieldStart )
         {
           if ( (pDCB->TransField.usLevel == usLevel ) &&
                (pDCB->TransField.usIndex == usField)  &&
                *pucData )
           {
             USHORT usLen = (USHORT)UTF16strlenCHAR(pucData);

             if ( (usTransBufUsed+usLen+2) < usOutBufSize )
             {
               if ( usTransBufUsed != 0 )
               {
                 UTF16strcpy( pucOutBuf + usTransBufUsed, L"; " );
                 usTransBufUsed += 2;
               } /* endif */
               UTF16strcpy( pucOutBuf + usTransBufUsed, pucData );
               usTransBufUsed = usTransBufUsed + usLen;
             } /* endif */
           } /* endif */
         } /* endif */
       } /* endwhile */
     }
     else
     {
       /*****************************************************/
       /* Process record in the old LDB format              */
       /*****************************************************/
       usNlpRC = LX_INCOMP_SIG_ASD;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Terminate translations in buffer without any error checks       */
   /*******************************************************************/
   pucOutBuf[usTransBufUsed] = 0;

   return( usNlpRC );
} /* end of AsdGetTranslationW */

//+----------------------------------------------------------------------------+
//| AsdGetStemForm         - get the stem form of a term                       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Gets the stem form of a term using the Nlp function NlpMorphID.         |
//|    If not stem form is available, the original term is copied to the       |
//|    target buffer.                                                          |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    copy input term to output term (use as default);                        |
//|    get morphological information for term;                                 |
//|    if info found then                                                      |
//|       locate first lemma of term;                                          |
//|       if lemma located then                                                |
//|          copy lemma to output term;                                        |
//|       endif;                                                               |
//|    endif;                                                                  |
//|    return Nlp return code;                                                 |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HDCB     hDCB            IN          dictionary control block handle    |
//|    HUCB     hUCB            IN          user control block handle          |
//|    PSZ      pszInTerm       IN          input term                         |
//|    PSZ      pszOutTerm      OUT         output term (reduced to stem form) |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        successful completion          |
//|                         LX_...              Nlp return codes               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    hUCB must have been created using AsdBegin.                             |
//|    hDCB must have been created using AsdOpen or AsdBuild.                  |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdGetStemForm
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pszInTerm,                 // input term
   PSZ_W    pszOutTerm                 // output term (reduced to stem form)
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PUCB      pUCB;                     // user control block pointer
   PDCB      pDCB;                     // dictionary control block pointer

   pUCB = (PUCB) hUCB;                 // convert handles to pointer
   pDCB = (PDCB) hDCB;                 // no further checks performed as this is
                                       // an internal function
   UTF16strcpy( pszOutTerm, pszInTerm );    // per default use input term as lemma

   usNlpRC = MorphGetStemForm( pDCB->sLangID, pszInTerm,
                              &pUCB->usTermListSize, &pUCB->pszTermList,
                               pDCB->ulOemCP);

   if ( usNlpRC == MORPH_OK )
   {

     UTF16strcpy( pszOutTerm, pUCB->pszTermList );
     usNlpRC = LX_RC_OK_ASD;
   }
   else
   {
     usNlpRC = AsdMorphRCToNlp( usNlpRC );
   } /* endif */

   return( LX_RC_OK_ASD );
} /* endof AsdGetStemFrom */


//+----------------------------------------------------------------------------+
//| AsdAddToTermList        - add a new term to a term list                    |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Add a new term to a term list and enlarge term list if current term     |
//|    list size is exceeded.                                                  |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PUCHAR    pucTerm             IN      term being added                  |
//|    USHORT    usTermLength        IN      length of term                    |
//|    PUSHORT   pusTerms            IN/OUT  number of terms in term list      |
//|    PUCHAR    *ppucTermList       IN/OUT  ptr to term list                  |
//|    PUSHORT   pusSize             IN/OUT  current size of term list         |
//|    PUSHORT   pusUsed             IN/OUT  used bytes in term list           |
//|    BOOL      fReallocAllowed     IN      TRUE = reallocation of term list  |
//|                                          is allowed.                       |
//|    USHORT    usDictHandle        IN      dictionary handle where term found|
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        function return code                               |
//|                         LX_RC_OK_ASD        function completed OK          |
//|                         LX_MEM_ALLOC_ASD    memory allocation failed       |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT AsdAddToTermList
(
   PSZ_W    pucTerm,                   // term being added
   USHORT   usTermLength,              // length of current term (in CHAR_Ws)
   PUSHORT  pusTerms,                  // number of terms in term list
   PSZ_W    *ppucTermList,             // ptr to term list
   PLONG    plSize,                    // current size of term list
   PLONG    plUsed,                    // used bytes in term list
   BOOL     fReallocAllowed,           // reallocation is allowed flag
   USHORT   usDictHandle               // dictionary handle
)
{
   USHORT        usNlpRC = LX_RC_OK_ASD; // return code of Nlp call(s)
   PSZ_W         pucNewEntry;          // ptr to new term entry
   BOOL          fOK = TRUE;           // return code of UtlAlloc function

   //
   // enlarge term list if required and allowed
   //
   if ( (*plUsed + (LONG)usTermLength + 2L) >= *plSize )
   {
      if ( fReallocAllowed )
      {
         USHORT usIncrement = max( (usTermLength + 2), TERM_BUFFER_INCR );

         fOK = UtlAlloc( (PVOID *)ppucTermList, *plSize * sizeof(CHAR_W),
                         (*plSize + usIncrement)*sizeof(CHAR_W), NOMSG );
         if ( fOK )
         {
             *plSize += (LONG)usIncrement;
         }
         else
         {
             usNlpRC = LX_MEM_ALLOC_ASD;
         } /* endif */
      }
      else
      {
         usNlpRC = LX_DATA_2_LRG_ASD;
      } /* endif */
   } /* endif */

   //
   // add new entry to list
   //
   if ( ASDOK(usNlpRC) )
   {
      pucNewEntry = *ppucTermList + *plUsed;
      /****************************************************************/
      /* insert dictionary handle if set                              */
      /****************************************************************/
      if ( usDictHandle )
      {
        // we use bit flags for the dictionary indicator now
        USHORT usIndicator = 0x00000001 << (usDictHandle - 1);
        *pucNewEntry++ = usIndicator;
        *plUsed += 1;
      } /* endif */
      /****************************************************************/
      /* insert term                                                  */
      /****************************************************************/
      if ( usTermLength )
      {
         memcpy( (PBYTE)pucNewEntry, (PBYTE)pucTerm, usTermLength * sizeof(CHAR_W));
      } /* endif */
      pucNewEntry[usTermLength] = EOS;
      *plUsed += (LONG)(usTermLength + 1);
      *pusTerms += 1;
   } /* endif */

   return( usNlpRC );
} /* end of AsdAddToTermList */

//
// search style field on current QLDB level and evaluate its contents
//
static CHAR_W SearchAndEvaluateStyle( FIELDDATA *pStyleField, PSZ_W pucData, USHORT usLevel, BOOL fSearchAll )
{
  USHORT usField = 0;                // we start at the begin of the current level
  BOOL   fLevelDone = FALSE;
  BOOL   fTreeDone = FALSE;
  int    iLen = 0;
  CHAR_W chStyle =  0;               // function return code

  while ( (fSearchAll || !fLevelDone) && !fTreeDone )
  {
    switch ( *pucData )
    {
      case QLDB_FIRST_LEVEL :
        usLevel = 1;
        usField = 0;
        fLevelDone = TRUE;
        pucData++;
        break;
      case QLDB_SECOND_LEVEL :
        usLevel = 2;
        usField = 0;
        fLevelDone = TRUE;
        pucData++;
        break;
      case QLDB_THIRD_LEVEL :
        usLevel = 3;
        usField = 0;
        fLevelDone = TRUE;
        pucData++;
        break;
      case QLDB_FOURTH_LEVEL :
        usLevel = 4;
        usField = 0;
        fLevelDone = TRUE;
        pucData++;
        break;
      case QLDB_END_OF_REC :
        fTreeDone = TRUE;
        fLevelDone = TRUE;
        pucData++;
        break;
      case QLDB_FIELD_DELIMITER :
        usField++;
        pucData++;
        break;
      default:
        // should be a field...
        iLen = UTF16strlenCHAR(pucData);
        if ( (usField == pStyleField->usIndex) && (usLevel == pStyleField->usLevel) )
        {
          PSZ_W pszStyleList;
          
          CHAR_W chFoundStyle = 0;

          // evaluate style field
          pszStyleList = STYLEVALUE_NOTALLOWED;
          while ( *pszStyleList && (chFoundStyle == 0) )
          {
            if ( UTF16stricmp( pucData, pszStyleList ) == 0 )
            {
              chFoundStyle = STYLEPREFIX_NOTALLOWED;
            }
            else
            {
              pszStyleList += UTF16strlenCHAR( pszStyleList ) + 1; 
            } /* endif */
          } /*endwhile */

          if ( !chFoundStyle )
          {
            pszStyleList = STYLEVALUE_PREFERRED;
            while ( *pszStyleList && (chFoundStyle == 0) )
            {
              if ( UTF16stricmp( pucData, pszStyleList ) == 0 )
              {
                chFoundStyle = STYLEPREFIX_PREFERRED;
              }
              else
              {
                pszStyleList += UTF16strlenCHAR( pszStyleList ) + 1; 
              } /* endif */
            } /*endwhile */
          } /* endif */

          // handle any style found
          if ( chFoundStyle )
          {
            if ( fSearchAll )
            {
              // combine found style with current style
              if ( !chStyle )
              {
                // no style yet, use found one
                chStyle = chFoundStyle;
              }
              else if ( chStyle == STYLEPREFIX_PREFERRED )
              {
                // use new style as any negative style will overwrite a positive style  
                chStyle = chFoundStyle;
              } /* endif */
            }
            else
            {
              chStyle = chFoundStyle;
            } /* endif */
          } /* endif */


          // we are through with this level...
          fLevelDone = TRUE;
        } /* endif */

        // skip field data 
        pucData += iLen;

        break;
    } /*endswitch */
  } /*endwhile */

  return( chStyle );
} /* end of function SearchAndEvaluateStyle */


//
// search NL Product field for selected PID values
//
static BOOL CheckPIDValue( PSZ_W pszProducts, PSZ_W pszValues ) 
{
  CHAR_W  szValues[MAX_DICTPID_VALUES] ;
  CHAR_W  *ptr ; 
  BOOL    fPIDMatch ;

  if ( *pszProducts ) {
     fPIDMatch = FALSE ;
     wcscpy( szValues, pszValues ) ;
     for( ptr = wcstok( szValues, L";" ); ptr ;
          ptr = wcstok( NULL, L";" ) ) { 
        if ( wcsstr( pszProducts, ptr ) ) {
           fPIDMatch = TRUE ;
           break;
        }
     }
  } else {
     fPIDMatch = TRUE ;  /* If no product info, then match */
  }

  return( fPIDMatch );
} /* end of function CheckPIDValue */

#ifdef ASDT_LOG_TERMLIST
  void WriteTermToLog( PSZ_W pszTerm, FILE *hfLog )
  {
    USHORT usDict = *pszTerm++;

    fwprintf( hfLog, L"[%u]", usDict );
    if ( pszTerm[0] == ASDT_PRE_DICT_DATA_CHAR )
    {
      fwprintf( hfLog, L"[PREDICTDATA]" );
      pszTerm++;
    } /* endif */       
    if ( pszTerm[0] == ASDT_POST_DICT_DATA_CHAR )
    {
      fwprintf( hfLog, L"[POSTDICTDATA]" );
      pszTerm++;
    } /* endif */       
    if ( pszTerm[0] == STYLEPREFIX_PREFERRED )
    {
      fwprintf( hfLog, L"[+]" );
      pszTerm++;
    } /* endif */       
    else if ( pszTerm[0] == STYLEPREFIX_NOTALLOWED )
    {
      fwprintf( hfLog, L"[-]" );
      pszTerm++;
    } 
    else if ( pszTerm[0] == STYLEPREFIX_UNDEFINED )
    {
      fwprintf( hfLog, L"[U]" ) ;
      pszTerm++;
    } /* endif */       
    fwprintf( hfLog, L"%s\n", pszTerm );
  }

  void WriteTermListToLog( PSZ pszTitle, PSZ_W pszTermList, FILE *hfLog )
  {
    PSZ_W pszTerm = pszTermList;

    fwprintf( hfLog, L"-----------------%S-------------------------\n", pszTitle );

    while ( *pszTerm != 0 )
    {
      fwprintf( hfLog, L"[HW]" );
      while ( *pszTerm != 0  )
      {
        WriteTermToLog( pszTerm, hfLog );
      
        pszTerm += UTF16strlenCHAR( pszTerm ) + 1; 
      } /* endwhile */         
      pszTerm++;
    } /* endwhile */       
  }
#endif
