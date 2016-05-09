//+----------------------------------------------------------------------------+
//|EQFMORPW.C      Word Recognition for morphological functions of TM          |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:    G. Queck    QSoft Quality Software Development GmbH              |
//+----------------------------------------------------------------------------+
//|Description:           API for morphological functions                      |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|  MorphWordRecognition    - Detect words for dictionary lookup              |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+

#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file

#include "OtmDictionaryIF.H"
#include <eqfmorpi.h>                  // private header file of module
#include <eqfchtbl.h>                  // character tables

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MorphWordRecognition   Split a segment into terms        |
//+----------------------------------------------------------------------------+
//|Description:       WordRecoginitions tokenizes a string and stores all terms|
//|                   found in a term list. WordRecognition uses the supplied  |
//|                   dictionary to detect multi-word terms.                   |
//|                   :p.                                                      |
//|                   If the switch fInclNotFoundTerms is FALSE, terms not     |
//|                   contained in the dictionary are not listed in the term   |
//|                   list.                                                    |
//|                   :p.                                                      |
//|                   A term list is series of null-terminated strings         |
//|                   which is terminated by another null value.               |
//|                   See the sample for the layout of a term list.            |
//|                   :p.                                                      |
//|                   If the term list pointer is NULL and the term list size  |
//|                   is zero, the term list is allocated.                     |
//|                   The term list is enlarged if not all terms fit into      |
//|                   the current list.                                        |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = MorphWordRecoginition( SHORT sLanguageID,         |
//|                                                 PSZ pszInData,             |
//|                                                 HUCB hUCB, HDCB hDCB,      |
//|                                                 PUSHORT pusBufferSize,     |
//|                                                 PSZ *ppTermList,           |
//|                                                 BOOL fInclNotFoundTerms,   |
//|                                                 PUSHORT pusOrgRC );        |
//+----------------------------------------------------------------------------+
//|Input parameter:   SHORT    sLanguageID    language ID                      |
//|                   PSZ      pszInData      pointer to input segment         |
//|                   HUCB     hUCB           user control block handle        |
//|                   HDCB     hDCB           dictionary control block handle  |
//|                   BOOL     fInclNotFoundTerms   TRUE = not found terms are |
//|                                             included in the term list      |
//|                                           FALSE = not found terms are      |
//|                                             omitted from the term list     |
//|                   PUSHORT  pusOrgRC       points to buffer for any return  |
//|                                           code returned from called functs.|
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT  pusOrgBufferSize  address of variable containing|
//|                                                size of term list buffer    |
//|                   PSZ      *ppOrgTermList    address of caller's term list |
//|                                                pointer for original terms  |
//|                   PUSHORT  pusDictBufferSize address of variable containing|
//|                                                size of term list buffer    |
//|                   PSZ      *ppDictTermList   address of caller's term list |
//|                                                pointer for dictionary terms|
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       MORPH_OK    function completed successfully              |
//|                   MORPH_ASD_ERROR  an ASD error occured, original ASD      |
//|                               return code is stored at location pointed to |
//|                               by pusOrgRC                                  |
//|                   other       MORPH_ error code, see EQFMORPH.H for a list |
//+----------------------------------------------------------------------------+
//|Prerequesits:      sLanguageID must have been obtained using the            |
//|                   MorphGetLanguageID function.                             |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
//|Samples:           USHORT usListSize = 0;     // allocated list size        |
//|                   PSZ    pszList = NULL;     // ptr to list area           |
//|                   SHORT  sLangID;            // language ID                |
//|                                                                            |
//|                   usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );  |
//|                                                                            |
//|                   usRC = MorphWordRecoginition( sLangID,                   |
//|                             "Regular data backup is a prerequisit for      |
//|                                system recovery", &usListSize, &pszList,    |
//|                                NULL, NULL,                                 |
//|                                FALSE );                                    |
//|                                                                            |
//|                   Assuming that the dictionary referenced by hDCB contains |
//|                   the following terms 'data', 'backup', 'data backup',     |
//|                   'system' and 'recovery', a call to MorphWordRecognition  |
//|                   would return the following term list:                    |
//|                                                                            |
//|                     "data backup\0system\0recovery\0\0"                    |
//+----------------------------------------------------------------------------+
//|Function flow:     Check input data                                         |
//|                   Get language control block pointer                       |
//|                   get copy of input segment                                |
//|                   normalize segment by reducing mulitple blanks and CRLF   |
//|                     to single blank                                        |
//|                   tokenize the normalized segment using MorphTokenize      |
//|                   allocate buffer for stem form reduced segment            |
//|                   loop over all terms in segment                           |
//|                      get stem form of term                                 |
//|                      add term's stem form to stem form segment             |
//|                      add delimiting character up to next term to           |
//|                        stem form segment                                   |
//|                   endloop                                                  |
//|                   loop over all terms in segment                           |
//|                      reduce term to stem form                              |
//|                      get all MWTs for term                                 |
//|                      if term found in dictionary                           |
//|                         loop over all MWTs of term                         |
//|                            compare MWT with data in normalized segment     |
//|                            if not matched                                  |
//|                               compare MWT with data in stem form segment   |
//|                            if matched                                      |
//|                               leave loop                                   |
//|                         endloop                                            |
//|                      if MWT found                                          |
//|                         add MWT to term list                               |
//|                         skip all subterms of MWT                           |
//|                      elsif term found or fIncludeNotFoundTerms is set      |
//|                         add term to term list                              |
//|                      end                                                   |
//|                     endloop                                                |
//|                     return function return code                            |
//+----------------------------------------------------------------------------+
USHORT MorphWordRecognition
(
   SHORT         sLanguageID,               // language ID
   PSZ           pszInData,                 // pointer to input segment
   HUCB          hUCB,                      // user control block handle
   HDCB          hDCB,                      // dictionary control block handle
   PUSHORT       pusOrgBufferSize,          // address of variable containing size of
                                            //    term list buffer
   PSZ           *ppOrgTermList,            // address of caller's term list pointer
                                            //    for original terms
   PUSHORT       pusDictBufferSize,         // address of variable containing size of
                                            //    term list buffer
   PSZ           *ppDictTermList,           // address of caller's term list pointer
                                            //    for dictionary terms
   BOOL          fInclNotFoundTerms,        // TRUE = include not found terms
                                            //                        /* 2@KIT0969A */
   PUSHORT       pusOrgRC,                  // ptr to buffer for original return code
                                            //
   USHORT        eqfMWTMode                 // Multi Word Termns
)
{
   USHORT   usRC = MORPH_OK;
   PSZ_W    pszInDataW = NULL;
   PSZ_W    pOrgTermListW = NULL;
   PSZ_W    pDictTermListW = NULL;
   ULONG    ulOrgBufferSize = 0;
   ULONG    ulDictBufferSize=0;
   ULONG    ulOemCP = 0L;

   ulOemCP = GetCPFromDCB(hDCB);

   // copy term data into unicode
   UtlAlloc( (PVOID *)&pszInDataW, 0L, MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
   if ( pszInDataW )
   {
     ASCII2Unicode( pszInData, pszInDataW, ulOemCP );


     usRC = MorphWordRecognitionW( sLanguageID, pszInDataW, hUCB, hDCB,
                                   &ulOrgBufferSize, &pOrgTermListW,
                                   &ulDictBufferSize, &pDictTermListW,
                                   fInclNotFoundTerms, pusOrgRC, eqfMWTMode);

     // prepare ASCII string
     if ( !usRC )
     {
       LONG lOrgBufferASCII = *pusOrgBufferSize;
       LONG lDictBufferASCII = *pusDictBufferSize;
       LONG lDictBufferUsed;
       usRC =  MorphCopyStringListUnicode2ASCII( pOrgTermListW, ulOrgBufferSize, ppOrgTermList,
                                                 &lOrgBufferASCII, &lDictBufferUsed, ulOemCP);
       if (!usRC )
       {
         usRC =  MorphCopyStringListUnicode2ASCII( pDictTermListW, ulDictBufferSize, ppDictTermList,
                                                   &lDictBufferASCII, &lDictBufferUsed, ulOemCP);
       }
       if ( !usRC )
       {
         *pusOrgBufferSize = (USHORT) lOrgBufferASCII;
         *pusDictBufferSize = (USHORT) lDictBufferASCII;

       }
     } /* endif */


     UtlAlloc( (PVOID *)&pszInDataW, 0L, 0L, NOMSG );
     UtlAlloc( (PVOID *)&pOrgTermListW, 0L, 0L, NOMSG );
     UtlAlloc( (PVOID *)&pDictTermListW, 0L, 0L, NOMSG );
   }
   return usRC;
}  /* endof MorphWordRecognition */



USHORT MorphWordRecognitionW
(
   SHORT         sLanguageID,               // language ID
   PSZ_W         pszInData,                 // pointer to input segment
   HUCB          hUCB,                      // user control block handle
   HDCB          hDCB,                      // dictionary control block handle
   PULONG        pulOrgBufferSize,          // address of variable containing size of
                                            //    term list buffer
   PSZ_W        *ppOrgTermList,            // address of caller's term list pointer
                                            //    for original terms
   PULONG        pulDictBufferSize,         // address of variable containing size of
                                            //    term list buffer
   PSZ_W        *ppDictTermList,           // address of caller's term list pointer
                                            //    for dictionary terms
   BOOL          fInclNotFoundTerms,        // TRUE = include not found terms
                                            //                        /* 2@KIT0969A */
   PUSHORT       pusOrgRC,                  // ptr to buffer for original return code
   USHORT        eqfMWTMode                 // Multi Word Terms
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  PLCB       pLCB = NULL;              // pointer to language control block
   //--------- variables for tokenizing of normalized segment       -----
   USHORT    usListLen = 0;            // length of list in bytes
   PULONG    pTermList = NULL;         // pointer to offset/length term list
   ULONG     ulOrgBufferUsed = 0;      // number of bytes used in caller's list
   ULONG     ulDictBufferUsed = 0;     // number of bytes used in caller's list

   CHAR_W    chTemp;                   // buffer for character values
   PSZ_W     pucNormSeg = NULL;        // buffer for normalized segment
   ULONG     ulNormSegSize = 0;        // size of normalized segment buffer
   PSZ_W     pucRedSeg = NULL;         // buffer for punctuation reduced segment
   PSZ_W     pucStemSeg = NULL;        // buffer for stem form reduced segment
   ULONG     ulStemSegSize;            // size of stem form reduced segment buffer
   ULONG     ulStemSegUsed = 0;        // bytes used of stem form segment buffer

   // variables for stem form reduction
   USHORT  usStemListSize = 0;         // current term list size
   PSZ_W   pStemList      = NULL;      // pointer to current term list
   PLISTTERM pTerm;
   PLISTTERM pNextTerm;
   PULONG    pTermInStemSegList = NULL;// pointer to offset/length term list
   PLISTTERM pTermInStemSeg;           // pointer to term in stem segm. list
   PSZ_W     pucTermInStemSeg;         // pointer to term in stem segment

   USHORT    usTermOffs;               // offset of currently processed term
   USHORT    usTermLength;             // length of currently processed term
   PSZ_W     pucTerm;                  // ptr to currently processed term
   ULONG     ulRemainLength;           // length of remaining data in segment
   ULONG     ulDelimLength;            // length of term delimiter
   //------------- variables for MWT term list processing -----
   USHORT    usMWTTerms = 0;          // number of terms in list
   LONG      lMWTBufUsed = 0;         // number of bytes used in list
   LONG      lMWTBufSize = 0;         // size of list in bytes
   PSZ_W     pucMWTBuf    = NULL;      // buffer for MWT terms
   PSZ_W     pucMWT;                   // ptr to current MWT term
   PSZ_W     pucNextMWT;               // ptr to next MWT term in list
   ULONG     ulMWTLength = 0;          // length of current MWT term
   PSZ_W     pucMWTHeadword;           // points to MWT headword term
   PSZ_W     pucIndexHeadword = NULL;  // headword for index lookup
   //------------- variables for term lookup -----
   ULONG     ulLookupLength;           // length of lookup term
   PSZ_W     pucLookupTerm = NULL;     // ptr to lookup term
                                                                /* 2@KIT0911A */
   PUCHAR    pucIsText = NULL;         // pointer to is-text character table
   CHAR     chLang[MAX_LANGUAGE_PROPERTIES];
   BOOL     fDBCSLang = FALSE;

   CHAR_W    szWord1[TERM_SIZE];
   CHAR_W    szWord2[TERM_SIZE];
   CHAR_W    szWord3[TERM_SIZE];
   ULONG     ulPos1 = 0;
   ULONG     ulPos2= 0;
   ULONG     ulPos3= 0;

   CHAR_W    szMWT[TERM_SIZE];
   PSZ_W     pszMWT   = &szMWT[0];

   BOOL      fcheck = FALSE;
   INT       iEnd = 0;
   ULONG     ulOemCP = 0L;

   ulOemCP = GetCPFromDCB(hDCB);

//   EQFMWTMODE    eqfMWTMode= EQF_MWT_START_NOUN;
//   EQFMWTMODE    eqfMWTMode= EQF_MWT_START_ADJ;
//   EQFMWTMODE    eqfMWTMode= EQF_MWT_START_NOUN_ADJ;
  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pszInData == NULL)     ||
       (pulOrgBufferSize == NULL) ||
       (ppOrgTermList == NULL)    ||
       ((*ppOrgTermList == NULL) && (*pulOrgBufferSize != 0) ) )
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
  /* Get is-text recognition table pointer and determine DBCS lang    */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    MorphGetLanguageString( sLanguageID, chLang );
    fDBCSLang = (MorphGetLanguageType( chLang ) == MORPH_DBCS_LANGTYPE);
    if ( UtlQueryCharTable( IS_TEXT_TABLE, &pucIsText ) != NO_ERROR )
    {
      usRC = MORPH_FUNC_FAILED;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* get copy of input segment and normalize the data                 */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    PSZ_W    pucCurPos;               // ptr to current position in input data

    ulNormSegSize = UTF16strlenCHAR(pszInData) + 200;

    if ( !UtlAlloc( (PVOID *)&pucNormSeg,
                    0L,
                    (LONG)ulNormSegSize * sizeof(CHAR_W),
                    NOMSG ) )
    {
       usRC = MORPH_NO_MEMORY;        // set short on memory RC
    }
    else if ( fDBCSLang )
    {
      /****************************************************************/
      /* Normalize and take care of DBCS characters                   */
      /****************************************************************/
       pucCurPos = pucNormSeg;       // start a begin of normalized segment
       chTemp = BLANK;               // remove leading white space
       while ( *pszInData )         // while not end of input segment data
       {
          if ( isdbcs1ex( (USHORT) ulOemCP, *pszInData ) == DBCS_1ST )
          {
             *pucCurPos++ = *pszInData++;          // copy current char
             *pucCurPos++ = chTemp = *pszInData++; // and next one
          }
          else if ( (*pszInData == LF) ||  // a whitespace character ??
               (*pszInData == CR) ||
               (*pszInData == BLANK) )
          {
             if ( chTemp == BLANK )    // if not the first one ...
             {
                pszInData++;                  // ... ignore it
             }
             else                              // else
             {
                *pucCurPos++ = chTemp = BLANK; // ... add it to segment
             } /* endif */
          }
          else
          {
             *pucCurPos++ = chTemp = *pszInData++; // copy current char
          } /* endif */
       } /* endwhile */
       *pucCurPos = EOS;     // terminate normalized segment
    }
    else
    {
      /****************************************************************/
      /* Normalize without taking care of DBCS characters             */
      /****************************************************************/
       pucCurPos = pucNormSeg;       // start a begin of normalized segment
       chTemp = BLANK;               // remove leading white space
       while ( *pszInData )         // while not end of input segment data
       {
          if ( (*pszInData == LF) ||  // a whitespace character ??
               (*pszInData == CR) ||
               (*pszInData == BLANK) )
          {
             if ( chTemp == BLANK )    // if not the first one ...
             {
                pszInData++;                  // ... ignore it
             }
             else                              // else
             {
                *pucCurPos++ = chTemp = BLANK; // ... add it to segment
             } /* endif */
          }
          else
          {
             *pucCurPos++ = chTemp = *pszInData++; // copy current char
          } /* endif */
       } /* endwhile */
       *pucCurPos = EOS;     // terminate normalized segment
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* get copy of normalized segment and remove punctuation and other  */
  /* stuff                                                            */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    ULONG  ulRedSegSize;            // size of punctuation reduced segment
    ulRedSegSize = UTF16strlenCHAR(pucNormSeg) + 1;

    if ( !UtlAlloc( (PVOID *)&pucRedSeg, 0L,
                    (LONG)max(MIN_ALLOC,ulRedSegSize * sizeof(CHAR_W)), NOMSG ) )
    {
       usRC = MORPH_NO_MEMORY;        // set short on memory RC
    }
    else
    {
      // we do not remove punctuation anymore (fix for KBT0214)
      UTF16strcpy( pucRedSeg, pucNormSeg );
    } /* endif */

  // punctuation is not removed anymore from the segment (fix for KBT0214)
  } /* endif */

  /********************************************************************/
  /* tokenize the reduced segment                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphTokenizeW( sLanguageID, pucRedSeg, &usListLen, (PVOID*)&pTermList,
                          MORPH_OFFSLIST, ulOemCP );
  } /* endif */

  /********************************************************************/
  /* build stem form reduced segment                                  */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    /******************************************************************/
    /* allocate buffer for segment (three times size of normalized    */
    /* segment should be enough)                                      */
    /******************************************************************/
    ulStemSegSize = ulNormSegSize * 3;
    if ( !UtlAlloc( (PVOID *)&pucStemSeg,
                    0L,
                    (LONG)ulStemSegSize*sizeof(CHAR_W),
                    NOMSG ) )
    {
       usRC = MORPH_NO_MEMORY;          // set short on memory RC
    } /* endif */

    /******************************************************************/
    /* allocate buffer for stem form term list (size = size of normal */
    /* term list)                                                     */
    /******************************************************************/
    if ( !UtlAlloc( (PVOID *)&pTermInStemSegList,
                    0L,
                    (LONG)usListLen*sizeof(CHAR_W),
                    NOMSG ) )
    {
       usRC = MORPH_NO_MEMORY;          // set short on memory RC
    } /* endif */

    if ( usRC == MORPH_OK )
    {
      pTerm          = (PLISTTERM)pTermList;
      pTermInStemSeg = (PLISTTERM)pTermInStemSegList;
      while ( (usRC == MORPH_OK) && (pTerm->ul != 0L) )
      {
         // get values of current term (for easier processing)
         usTermOffs   = pTerm->OffsLen.usOffs;
         usTermLength = pTerm->OffsLen.usLength;
         pucTerm = pucRedSeg + usTermOffs;

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
  if ( ( usRC == MORPH_NOT_FOUND ) &&
       ( wcschr(pucTerm,'-') ) ) {
     WCHAR szTemp[500] ;
     WCHAR *pChar ;
     wcscpy( szTemp, pucTerm ) ;
     pChar = wcschr(szTemp,'-');
     *pChar = NULL ;

     usRC = MorphGetStemForm( sLanguageID, szTemp,
                              &usStemListSize, &pStemList, ulOemCP );
     if ( usRC == MORPH_NOT_FOUND ) {
        wcscpy( szTemp, pucTerm ) ;
        pChar = wcschr(szTemp,'-');
        memcpy( pChar, pChar+1, (wcslen(pChar+1)+1)*sizeof(WCHAR)) ;

        usRC = MorphGetStemForm( sLanguageID, szTemp,
                                 &usStemListSize, &pStemList, ulOemCP );
     }
  }

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
           /* add first stem form of term to stem form segment        */
           /***********************************************************/
           memcpy((PBYTE) (pucStemSeg + ulStemSegUsed), (PBYTE)pStemList, UTF16strlenBYTE(pStemList) );
           pTermInStemSeg->OffsLen.usOffs   = (USHORT)ulStemSegUsed;   // # of w's
           pTermInStemSeg->OffsLen.usLength = (USHORT)UTF16strlenCHAR(pStemList);
           ulStemSegUsed += UTF16strlenCHAR(pStemList);
         }
         else if ( usRC == MORPH_NOT_FOUND )
         {
           /***********************************************************/
           /* add term as-is to stem form segment                     */
           /***********************************************************/
           memcpy( pucStemSeg + ulStemSegUsed, pucTerm,
                   usTermLength * sizeof(CHAR_W));
           pTermInStemSeg->OffsLen.usOffs   = (USHORT)ulStemSegUsed;
           pTermInStemSeg->OffsLen.usLength = usTermLength;
           ulStemSegUsed += usTermLength;       // # of w's
           usRC = MORPH_OK;            // reset return code
         } /* endif */

         /*************************************************************/
         /* add delimiting characters up to next term if there is     */
         /* another term and if there are delimiting characters       */
         /* between the current term and the following one            */
         /*************************************************************/
         usTermOffs   = pTerm->OffsLen.usOffs;    // in # of w's
         usTermLength = pTerm->OffsLen.usLength;  // in # of w's
         pucTerm = pucRedSeg + usTermOffs;
         if ( (pTerm+1)->ul != 0L)     // if there are more terms
         {
           ulDelimLength = (pTerm+1)->OffsLen.usOffs - usTermOffs -
                           usTermLength;  // in # of w's
           if ( ulDelimLength )
           {
             memcpy( (PBYTE)(pucStemSeg + ulStemSegUsed), (PBYTE)(pucTerm + usTermLength),
                     ulDelimLength * sizeof(CHAR_W) );
             ulStemSegUsed += ulDelimLength;
           } /* endif */
         } /* endif */

         /*************************************************************/
         /* continue with next term                                   */
         /*************************************************************/
         pTerm++;
         pTermInStemSeg++;
      } /* endwhile terms in list */


      /****************************************************************/
      /* terminate stem form segment                                  */
      /****************************************************************/
      pucStemSeg[ulStemSegUsed++] = EOS;
      pTermInStemSeg->ul = 0L;
    } /* endif */

  } /* endif */

  /********************************************************************/
  /* check terms against MWT terms from the index dictionary          */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
     pTerm          = (PLISTTERM)pTermList;
     pTermInStemSeg = (PLISTTERM)pTermInStemSegList;

     szWord1[0] = EOS;
     szWord2[0] = EOS;
     szWord3[0] = EOS;
     szMWT[0]   = EOS;

     while ( (usRC == MORPH_OK) && (pTerm->ul != 0L) )
     {
        // get values of current term (for easier processing)
        usTermOffs   = pTerm->OffsLen.usOffs;
        usTermLength = pTerm->OffsLen.usLength;
        pucTerm = pucNormSeg + usTermOffs;
        pucTermInStemSeg = pucStemSeg + pTermInStemSeg->OffsLen.usOffs;

        // terminate current term
        chTemp = pucTerm[usTermLength];
        pucTerm[usTermLength] = EOS;
        /*************************************************************/
        /* LOOK for MWTs                                             */
        /*                                                           */
        /* noun noun                                                 */
        /* noun noun noun                                            */
        /* adj  noun                                                 */
        /* adj  adj  noun                                            */
        /* noun of   noun                                            */
        /*************************************************************/
        if ( fInclNotFoundTerms && eqfMWTMode !=  EQF_MWT_EMPTY)
        {
           ULONG  ulPOSInfo;
           PULONG  pulPOSInfo;
           int     irun;

           pulPOSInfo = &ulPOSInfo;
           *pulPOSInfo = 0;

           usRC = MorphGetPOSInfo( sLanguageID, pucTerm, pulPOSInfo, ulOemCP );

          // preselection
          //
          if (szWord1[0] == EOS)
          {
            UTF16strcpy(szWord1, pucTerm);
            ulPos1 = *pulPOSInfo;
            iEnd = 0;
          }
          else if (szWord2[0] == EOS)
          {
            UTF16strcpy(szWord2, pucTerm);
            ulPos2 = *pulPOSInfo;

            //last Term
            pNextTerm = pTerm+1;
            if (pNextTerm->ul == 0L)
            {
              iEnd = 1;
            }
            else
            {
              iEnd = 0;
            }//end if

          }
          else if (szWord3[0] == EOS)
          {
            UTF16strcpy(szWord3, pucTerm);
            ulPos3 = *pulPOSInfo;

           //last Term
            pNextTerm = pTerm+1;
            if (pNextTerm->ul == 0L)
            {
              iEnd = 3;
            }
            else
            {
              iEnd = 2;
            }//end if
          }// end if szWords

          if (iEnd)
          {
            for (irun=1; irun<=iEnd; irun++)
            {
              if (irun == 1)//------------- first two words
              {
                if(
                    ((eqfMWTMode==EQF_MWT_START_NOUN  || eqfMWTMode==EQF_MWT_START_NOUN_ADJ)
                                  && ulPos1 & MORPH_NOUN      && ulPos2 & MORPH_NOUN ) ||

                    ((eqfMWTMode==EQF_MWT_START_ADJ || eqfMWTMode==EQF_MWT_START_NOUN_ADJ)
                                  && ulPos1 & MORPH_ADJECTIVE && ulPos2 & MORPH_NOUN )

                   )
                {
                  fcheck = TRUE;
                  swprintf( szMWT, L"%s %s", szWord1, szWord2 );
                }
                else
                {
                  fcheck = FALSE;
                }//end if

              }
              else if (irun == 3)//-------- second two words (should be the same as first two words)
              {
                if(
                    ((eqfMWTMode==EQF_MWT_START_NOUN  || eqfMWTMode==EQF_MWT_START_NOUN_ADJ)
                                  && ulPos2 & MORPH_NOUN      && ulPos3 & MORPH_NOUN ) ||

                    ((eqfMWTMode==EQF_MWT_START_ADJ || eqfMWTMode==EQF_MWT_START_NOUN_ADJ)
                                  && ulPos2 & MORPH_ADJECTIVE && ulPos3 & MORPH_NOUN )

                   )
                {
                  fcheck = TRUE;
                  swprintf( szMWT, L"%s %s",szWord2, szWord3 );
                }
                else
                {
                  fcheck = FALSE;
                }//end if

              }
              else if (irun == 2)//-------- all three words
              {
                if(
                    ((eqfMWTMode==EQF_MWT_START_NOUN  || eqfMWTMode==EQF_MWT_START_NOUN_ADJ)
                        && ulPos1 & MORPH_NOUN      && ulPos2 & MORPH_NOUN  && ulPos3 & MORPH_NOUN) ||

                    ((eqfMWTMode==EQF_MWT_START_NOUN  || eqfMWTMode==EQF_MWT_START_NOUN_ADJ)
                        && ulPos1 & MORPH_NOUN      && !UTF16strcmp(szWord2,L"of")  && ulPos3 & MORPH_NOUN) ||


                    ((eqfMWTMode==EQF_MWT_START_ADJ || eqfMWTMode==EQF_MWT_START_NOUN_ADJ)
                        && ulPos1 & MORPH_ADJECTIVE && ulPos2 & MORPH_ADJECTIVE && ulPos3 & MORPH_NOUN)

                   )
                {
                  fcheck = TRUE;
                  swprintf( szMWT, L"%s %s %s", szWord1, szWord2, szWord3 );
                }
                else
                {
                  fcheck = FALSE;
                }//end if
              }// end if irun
              if (fcheck)
              {
                //
                // check for dictionary entry
                //
                {

                  HDCB     hDCB1 ;
                  ULONG    ulTermNumber=0;
                  ULONG    ulDataLength=0;

                  usRC = AsdFndMatch ( pszMWT,              // desired term
                                       hDCB,                // dictionary control block handle
                                       hUCB,                // user control block handle
                                       pszMWT,              // matching term found
                                       &ulTermNumber,       // term number
                                       &ulDataLength,       // entry data length
                                       &hDCB1               // dictionary of match
                                    );

                  if ( usRC == LX_RC_OK_ASD )
                  {
                    usRC = MORPH_OK;
                  }
                  else
                  {
                    usRC = MORPH_NOT_FOUND;
                  } /* endif */
                }


                if (usRC == MORPH_NOT_FOUND)
                {
                  usRC = MorphAddTermToList2W( ppOrgTermList,
                                             pulOrgBufferSize,
                                             &ulOrgBufferUsed,
                                             pszMWT,
                                             (USHORT)UTF16strlenCHAR(pszMWT),
                                             0,
                                             0L,
                                             MORPH_ZTERMLIST );

                } // end if
              } //end if fcheck
            } // end for
            //
            // clear first Word
            //

            UTF16strcpy(szWord1,szWord2);
            ulPos1=ulPos2;
            UTF16strcpy(szWord2,szWord3);
            ulPos2=ulPos3;
            szWord3[0] = EOS;
            ulPos3=0;
          } // end if iEnd
        } // end if fInclNotFoundTerms

        /*************************************************************/

        usRC = MorphGetStemForm( sLanguageID, pucTerm,
                                 &usStemListSize, &pStemList, ulOemCP );

        /**************************************************************/
        /* get MWTs for current term                                  */
        /**************************************************************/
        if ( (usRC == MORPH_OK) || (usRC == MORPH_NOT_FOUND) )
        {
          pucIndexHeadword = (usRC == MORPH_OK) ? pStemList : pucTerm;
          lMWTBufUsed = 0;
          usMWTTerms = 0;
          usRC = AsdListIndex( hUCB,             // user control block handle
                               hDCB,             // dictionary control block handle
                               pucIndexHeadword, // term
                               MWT_TYPE,         // type of requested list
                               &pucMWTBuf,       // buffer for returned terms
                               &lMWTBufUsed,    // used size of buffer
                               &lMWTBufSize,    // actual size of buffer
                               &usMWTTerms,      // number of terms in buffer
                               TRUE );                     // term is already in stem form
          if ( usRC == LX_RC_OK_ASD )
          {
            usRC = MORPH_OK;
          }
          else if ( usRC == LX_WRD_NT_FND_ASD )
          {
            usRC = MORPH_NOT_FOUND;
          }
          else
          {
            if ( pusOrgRC )
            {
              *pusOrgRC = usRC;
            } /* endif */
            usRC = MORPH_ASD_ERROR;
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* restore end character of current term                      */
        /**************************************************************/
        pucTerm[usTermLength] = chTemp;
        ulLookupLength = 0;            // nothing for lookup yet

        if ( usRC == MORPH_NOT_FOUND )     // if term is not in index ...
        {
          if ( fInclNotFoundTerms )
          {
            usRC = MorphAddTermToList2W( ppOrgTermList,
                                       pulOrgBufferSize,
                                       &ulOrgBufferUsed,
                                       pucTerm,
                                       usTermLength,
                                       0,
                                       0L,
                                       MORPH_ZTERMLIST );
            if ( ppDictTermList )
            {
              usRC = MorphAddTermToList2W( ppDictTermList,
                                         pulDictBufferSize,
                                         &ulDictBufferUsed,
                                         pucTermInStemSeg,
                                         pTermInStemSeg->OffsLen.usLength,
                                         0,
                                         0L,
                                         MORPH_ZTERMLIST );
            } /* endif */
          } /* endif */

          usRC = MORPH_OK;              // ... set RC to OK
          pTerm++;                     // skip this term
          pTermInStemSeg++;
        }
        else if ( usRC == MORPH_OK )
        {
           PSZ_W   pucBestMatch;      // best matching term


           // check term against MWTs for term
           ulRemainLength = UTF16strlenCHAR( pucTerm ); // get rest length of segment
           pucMWT = pucMWTBuf; // start at begin of MWTs
           pucBestMatch = NULL;
           while ( !ulLookupLength && usMWTTerms )
           {
              // get length of MWT
              ulMWTLength = UTF16strlenCHAR( pucMWT );
              pucNextMWT  = pucMWT + ulMWTLength + 1;

              if ( ulMWTLength == 0)
              {
                // MWT list seems to be defect, leave MWT processing loop
                usMWTTerms = 0;
              } 
              else if ( (ulMWTLength <= ulRemainLength) &&
                   !memcmp( pucTerm, pucMWT, ulMWTLength *sizeof(CHAR_W)) &&
                   (!pucIsText[pucTerm[ulMWTLength]] || fDBCSLang) )
              {
                pucMWTHeadword = pucMWT;
                pucLookupTerm = pucMWT;
                ulLookupLength = ulMWTLength;
              }
              else if ( (ulMWTLength <= ulRemainLength) &&
                   !memicmp( pucTerm, pucMWT, ulMWTLength *sizeof(CHAR_W)) &&
                   (!pucIsText[pucTerm[ulMWTLength]] || fDBCSLang) )
              {
                // only use this match as best match if its length is larger than any
                // existing best match
                int iBestMatchLen = 0;
                if ( pucBestMatch )
                {
                  iBestMatchLen = UTF16strlenCHAR( pucBestMatch );
                } /* endif */
                if ( UTF16strlenCHAR( pucMWT ) > iBestMatchLen )
                {
                  pucBestMatch = pucMWT;
                } /* endif */
                pucMWT = pucNextMWT;        // test next MWT
                usMWTTerms--;
              }
              else if ( (ulMWTLength <= ulRemainLength) &&
                        !memicmp( pucTermInStemSeg, pucMWT, ulMWTLength *sizeof(CHAR_W)) &&
                        (!pucIsText[pucTermInStemSeg[ulMWTLength]] || fDBCSLang) )
              {
                pucMWTHeadword = pucMWT;
                pucLookupTerm = pucMWT;
                ulLookupLength = ulMWTLength;
              }
              else if ( ( wcschr( pucTerm, L'-' ) ) ||
                        ( wcschr( pucMWT,  L'-' ) ) ) 
              {
                // If term contains a hyphen, then check if everything else is the same. 
                // If the same, then handle it as a match.        2-17-16
                BOOL  bMatch=TRUE ;
                CHAR_W  *cKey1 = (CHAR_W*) pucTerm;
                CHAR_W  *cKey2 = (CHAR_W*) pucMWT;
                while ( bMatch && 
                        ( ( *cKey1 != NULL ) && 
                          ( *cKey2 != NULL ) ) )
                {
                  while ( wcschr( L" -", *cKey1 ) ) { cKey1++; } 
                  while ( wcschr( L" -", *cKey2 ) ) { cKey2++; }
                  if ( *cKey1 == *cKey2 ) {
                    cKey1++;
                    cKey2++;
                  } else {
                    bMatch=FALSE;
                  } /* endif */
                } /* endwhile */
                if ( ( bMatch ) &&
                     ( ( *cKey1 == NULL ) ||
                       ( ( *cKey1 == L' ' ) &&
                         ( *(cKey1+1) == NULL) ) ) &&
                     ( *cKey2 == NULL ) )
                {
                   // only use this match as best match if its length is larger than any
                   // existing best match
                   int iBestMatchLen = 0;
                   if ( pucBestMatch )
                   {
                     iBestMatchLen = UTF16strlenCHAR( pucBestMatch );
                   } /* endif */
                   if ( UTF16strlenCHAR( pucMWT ) > iBestMatchLen )
                   {
                     pucBestMatch = pucMWT;
                   } /* endif */
                   pucMWT = pucNextMWT;        // test next MWT
                   usMWTTerms--;
                } else {
                   // we have to try the next MWT
                   pucMWT = pucNextMWT;        // test next MWT
                   usMWTTerms--;
                }
              }
              else
              {
                // we have to try the next MWT
                pucMWT = pucNextMWT;        // test next MWT
                usMWTTerms--;
              } /* endif */
           } /* endwhile */

           /***********************************************************/
           /* Use best match if not term has been found               */
           /***********************************************************/
           if ( !ulLookupLength && pucBestMatch )
           {
              pucMWTHeadword = pucBestMatch;
              pucLookupTerm  = pucBestMatch;
              ulMWTLength = ulLookupLength = UTF16strlenCHAR(pucBestMatch);
           } /* endif */

           /***********************************************************/
           /* If term is not in MWT list try stem from reduced MWT    */
           /* list (if any)                                           */
           /***********************************************************/
           if ( !ulLookupLength )
           {
             /*********************************************************/
             /* Terminate current term                                */
             /*********************************************************/
             chTemp = pucTerm[usTermLength];
             pucTerm[usTermLength] = EOS;

             /*********************************************************/
             /* Get stem form reduced MWT list                        */
             /*********************************************************/
             lMWTBufUsed = 0;
             usMWTTerms = 0;
             pucBestMatch = NULL;
             usRC = AsdListIndex( hUCB,             // user control block handle
                                  hDCB,             // dictionary control block handle
                                  pucIndexHeadword, // term
                                  MWTSTEM_TYPE,     // type of requested list
                                  &pucMWTBuf,       // buffer for returned terms
                                  &lMWTBufUsed,    // used size of buffer
                                  &lMWTBufSize,    // actual size of buffer
                                  &usMWTTerms,      // number of terms in buffer
                                  TRUE );                     // term is already in stem form
             if ( usRC == LX_RC_OK_ASD )
             {
               usRC = MORPH_OK;
             }
             else if ( usRC == LX_WRD_NT_FND_ASD )
             {
               usRC = MORPH_NOT_FOUND;
             }
             else
             {
               if ( pusOrgRC )
               {
                 *pusOrgRC = usRC;
               } /* endif */
               usRC = MORPH_ASD_ERROR;
             } /* endif */

             /**************************************************************/
             /* restore end character of current term                      */
             /**************************************************************/
             pucTerm[usTermLength] = chTemp;

             /*********************************************************/
             /* Lookup term in stem form reduced MWT list             */
             /* NOTE: The check for fDBCSLang is not needed here,     */
             /*       because under DBCS we do not reduce a term to   */
             /*       its stem.                                       */
             /*********************************************************/
             if ( usRC == MORPH_OK )
             {
                ulRemainLength = UTF16strlenCHAR( pucTerm ); // get rest length of segment
                pucMWT = pucMWTBuf; // start at begin of MWTs
                while ( !ulLookupLength && usMWTTerms )
                {
                   // remember original headword of entry
                   pucMWTHeadword = pucMWT;

                   // get length of MWT
                   ulMWTLength = UTF16strlenCHAR( pucMWT );
                   pucNextMWT  = pucMWT + ulMWTLength + 1;

                   // set ptr to stem form reduced MWT
                   pucMWT = pucNextMWT;
                   if ( usMWTTerms ) usMWTTerms--;
                   ulMWTLength = UTF16strlenCHAR( pucMWT );
                   pucNextMWT  = pucMWT + ulMWTLength + 1;

                   if ( ulMWTLength == 0)
                   {
                     // MWT list seems to be defect, leave MWT processing loop
                     usMWTTerms = 0;
                   } 
                   else if ( (ulMWTLength <= ulRemainLength) &&
                        !memcmp( pucTerm, pucMWT, ulMWTLength * sizeof(CHAR_W) ) &&
                        !pucIsText[pucTerm[ulMWTLength]] )
                   {
                     pucLookupTerm = pucMWTHeadword;
                     ulLookupLength = UTF16strlenCHAR(pucMWTHeadword);
                   }
                   else if ( (ulMWTLength <= ulRemainLength) &&
                        !memicmp( pucTerm, pucMWT, ulMWTLength * sizeof(CHAR_W) ) &&
                        !pucIsText[pucTerm[ulMWTLength]] )
                   {
                     pucBestMatch = pucMWTHeadword;
                     pucMWT = pucNextMWT;        // test next MWT
                     if ( usMWTTerms ) usMWTTerms--;
                   }
                   else if ( (ulMWTLength <= ulRemainLength) &&
                             !memicmp( pucTermInStemSeg, pucMWT, ulMWTLength * sizeof(CHAR_W) ) &&
                             !pucIsText[pucTermInStemSeg[ulMWTLength]] )
                   {
                     pucLookupTerm = pucMWTHeadword;
                     ulLookupLength = UTF16strlenCHAR(pucMWTHeadword);
                   }
                   else if ( ( wcschr( pucTermInStemSeg, L'-' ) ) ||
                             ( wcschr( pucMWT, L'-' ) ) ) 
                   {
                      // If term contains a hyphen, then check if everything else is the same. 
                      // If the same, then handle it as a match.        2-17-16
                     BOOL  bMatch=TRUE ;
                     CHAR_W  *cKey1 = (CHAR_W*) pucTermInStemSeg;
                     CHAR_W  *cKey2 = (CHAR_W*) pucMWT;
                     while ( bMatch && 
                             ( ( *cKey1 != NULL ) && 
                               ( *cKey2 != NULL ) ) )
                     {
                       while ( wcschr( L" -", *cKey1 ) ) { cKey1++; } 
                       while ( wcschr( L" -", *cKey2 ) ) { cKey2++; }
                       if ( *cKey1 == *cKey2 ) {
                         cKey1++;
                         cKey2++;
                       } else {
                         bMatch=FALSE;
                       } /* endif */
                     } /* endwhile */
                     if ( ( bMatch ) &&
                          ( ( *cKey1 == NULL ) ||
                            ( ( *cKey1 == L' ' ) &&
                              ( *(cKey1+1) == NULL) ) ) &&
                          ( *cKey2 == NULL ) )
                     {
                        // only use this match as best match if its length is larger than any
                        // existing best match
                        int iBestMatchLen = 0;
                        if ( pucBestMatch )
                        {
                          iBestMatchLen = UTF16strlenCHAR( pucBestMatch );
                        } /* endif */
                        if ( UTF16strlenCHAR( pucMWT ) > iBestMatchLen )
                        {
                          pucBestMatch = pucMWTHeadword;
                        } /* endif */
                        pucMWT = pucNextMWT;        // test next MWT
                        if ( usMWTTerms ) usMWTTerms--;
                     } else {
                        // we have to try the next MWT
                        pucMWT = pucNextMWT;        // test next MWT
                        if ( usMWTTerms ) usMWTTerms--;
                     }
                   }
                   else
                   {
                     // we have to try the next MWT
                     pucMWT = pucNextMWT;        // test next MWT
                     if ( usMWTTerms ) usMWTTerms--;
                   } /* endif */
                } /* endwhile */

                /***********************************************************/
                /* Use best match if not term has been found               */
                /***********************************************************/
                 if ( !ulLookupLength && pucBestMatch )
                 {
                   pucMWTHeadword = pucBestMatch;
                   pucLookupTerm  = pucBestMatch;
                   ulMWTLength = ulLookupLength = UTF16strlenCHAR(pucBestMatch);
                 } /* endif */
             } /* endif */
           } /* endif */

           // if term is not in MWT list, add term as-is
           if ( !ulLookupLength )
           {
              usRC = MorphAddTermToList2W( ppOrgTermList,
                                         pulOrgBufferSize,
                                         &ulOrgBufferUsed,
                                         pucTerm,
                                         usTermLength,
                                         0,
                                         0L,
                                         MORPH_ZTERMLIST );
              if ( (usRC == MORPH_OK) && ppDictTermList )
              {
                usRC = MorphAddTermToList2W( ppDictTermList,
                                           pulDictBufferSize,
                                           &ulDictBufferUsed,
                                           pucTermInStemSeg,
                                           pTermInStemSeg->OffsLen.usLength,
                                           0,
                                           0L,
                                           MORPH_ZTERMLIST );
              } /* endif */
              pTerm++;                  // continue with next term
              pTermInStemSeg++;
           }
           else
           {
              // for special mode add starting term of MWT to term list
              if ( (usRC == MORPH_OK) && (fInclNotFoundTerms == 2) )
              {
                usRC = MorphAddTermToList2W( ppOrgTermList,
                                           pulOrgBufferSize,
                                           &ulOrgBufferUsed,
                                           pucTerm,
                                           usTermLength,
                                           0,
                                           0L,
                                           MORPH_ZTERMLIST );
              } /* endif */

              if ( (usRC == MORPH_OK) && ppDictTermList && (fInclNotFoundTerms == 2) )
              {
                usRC = MorphAddTermToList2W( ppDictTermList,
                                           pulDictBufferSize,
                                           &ulDictBufferUsed,
                                           pucTermInStemSeg,
                                           pTermInStemSeg->OffsLen.usLength,
                                           0,
                                           0L,
                                           MORPH_ZTERMLIST );
              } /* endif */

              /********************************************************/
              /* add MWT to caller's term list                        */
              /********************************************************/
              if ( (usRC == MORPH_OK) )
              {
                usRC = MorphAddTermToList2W( ppOrgTermList,
                                           pulOrgBufferSize,
                                           &ulOrgBufferUsed,
                                           pucLookupTerm,
                                           (USHORT)ulLookupLength,
                                           0,
                                           0L,
                                           MORPH_ZTERMLIST );
              } /* endif */

              if ( (usRC == MORPH_OK) && ppDictTermList )
              {
                usRC = MorphAddTermToList2W( ppDictTermList,
                                           pulDictBufferSize,
                                           &ulDictBufferUsed,
                                           pucLookupTerm,
                                           (USHORT)ulLookupLength,
                                           0,
                                           0L,
                                           MORPH_ZTERMLIST );
              } /* endif */

              //gs check
              if ( !(fInclNotFoundTerms == 2) && // (special mode used for dict.lookup)
                   (!fInclNotFoundTerms || (eqfMWTMode ==  EQF_MWT_EMPTY)) )
              {

                /********************************************************/
                /* skip all subterms of MWT in our private term list    */
                /********************************************************/
                while ( (pTerm->ul != 0L) &&
                       (pTerm->OffsLen.usOffs < (usTermOffs + ulMWTLength) ) )
                {
                   pTerm++;               // skip subterm
                   pTermInStemSeg++;
                } /* endwhile */

              }
              else
              {
                usRC = MORPH_OK;              // ... set RC to OK
                pTerm++;                     // skip this term
                pTermInStemSeg++;

              } // end if


           } /* endif */
        } /* endif EMPTY_STRING*/




     } /* endwhile terms in list */
  } /* endif */

  /********************************************************************/
  /* Terminate term list(s)                                           */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphAddTermToList2W( ppOrgTermList,
                               pulOrgBufferSize,
                               &ulOrgBufferUsed,
                               EMPTY_STRINGW,
                               0,
                               0,
                               0L,
                               MORPH_ZTERMLIST );
    if ( (usRC == MORPH_OK) && ppDictTermList )
    {
      usRC = MorphAddTermToList2W( ppDictTermList,
                                 pulDictBufferSize,
                                 &ulDictBufferUsed,
                                 EMPTY_STRINGW,
                                 0,
                                 0,
                                 0L,
                                 MORPH_ZTERMLIST );
    } /* endif */
  } /* endif */

                                                               /* 10@KITxxxxA */
  /********************************************************************/
  /* Free memory of temporary term list and work buffers              */
  /********************************************************************/
  if ( pTermList )          UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
  if ( pucNormSeg )         UtlAlloc( (PVOID *) &pucNormSeg, 0L, 0L, NOMSG );
  if ( pucRedSeg )          UtlAlloc( (PVOID *) &pucRedSeg, 0L, 0L, NOMSG );
  if ( pucStemSeg )         UtlAlloc( (PVOID *) &pucStemSeg, 0L, 0L, NOMSG );
  if ( pStemList )          UtlAlloc( (PVOID *) &pStemList, 0L, 0L, NOMSG );
  if ( pTermInStemSegList ) UtlAlloc( (PVOID *) &pTermInStemSegList, 0L, 0L, NOMSG );
  if ( pucMWTBuf )          UtlAlloc( (PVOID *) &pucMWTBuf, 0L, 0L, NOMSG );

  return( usRC );

} /* endof MorphWordRecognitionW */
