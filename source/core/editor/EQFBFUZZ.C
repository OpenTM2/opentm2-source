//+----------------------------------------------------------------------------+
//|EQFFUZZY.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_MORPH            // morphologic functions

#include <eqf.h>                  // General Translation Manager include file

#include "EQFTPI.H"               // Translation Processor priv. include file

#define ALPHABETIC   1
#define NUMERIC      2
#define NOTALNUM     3

#ifdef _DEBUG
//  #define EQFBFUZZ_LOG
#endif

/**********************************************************************/
/* the following structure specifies a 'snake' from the point (x,y)   */
/* to the point (u,v)                                                 */
/**********************************************************************/
typedef struct _SNAKEPTS
{
  SHORT sX;                         // x-edge of start of diagonal
  SHORT sY;                         // y-edge of start of diagonal
  SHORT sU;                         // x-edge of stop of diagonal
  SHORT sV;                         // y-edge of stop of diagonal
} SNAKEPTS, *PSNAKEPTS;

typedef struct _LCSTOKEN
{
  PFUZZYTOK pTokenList;
  PFUZZYTOK pBackList;
  SHORT sStart;
  SHORT sStop;
  SHORT sTotalLen;
} LCSTOKEN, *PLSCTOKEN;

static SHORT CompFuzzyTok (   PFUZZYTOK , PFUZZYTOK,  BOOL );

/**********************************************************************/
/* prototypes for static functions ...                                */
/**********************************************************************/
static VOID MakeHashValue ( PULONG, USHORT, PSZ_W, PULONG );



static BOOL PrepareTokens ( PLOADEDTABLE, PBYTE, PBYTE, PSZ_W, SHORT, PFUZZYTOK *, ULONG );
static BOOL FuzzyReplace ( PSZ_W, PSZ_W, PSZ_W, PREPLLIST, PREPLLIST );
static VOID  TransferSource( PSZ_W, PSZ_W, PSZ_W );
static SHORT TokStrCompare ( PFUZZYTOK, PFUZZYTOK);
static PFUZZYTOK SplitTokens (PFUZZYTOK, USHORT, SHORT, USHORT, PSZ_W);
static  ULONG  ulRandom[MAX_RANDOM];   // random sequence
static SHORT Snake ( PFUZZYTOK, PFUZZYTOK, SHORT, SHORT, SHORT, SHORT, BOOL);
static SHORT FindMiddleSnake( PFUZZYTOK, PFUZZYTOK, PFUZZYTOK, PFUZZYTOK,
                             SHORT, SHORT, PSNAKEPTS, BOOL );
static VOID LCS( LCSTOKEN, LCSTOKEN , BOOL);
static BOOL EQFBTokCountDiff(PFUZZYTOK, PFUZZYTOK,
                             USHORT, USHORT, PUSHORT);
static BOOL EQFBMarkModDelIns( PFUZZYTOK, PFUZZYTOK, PFUZZYTOK*, PFUZZYTOK *,
                               USHORT, USHORT);
static void EQFBSimplifyAndCountMarks (PFUZZYTOK, PUSHORT, PUSHORT);

/**********************************************************************/
/* macro to calculate the number of tokens in the list ...            */
/* and to adjust ends of tokens                                       */
/**********************************************************************/
#define  NUMBEROFTOKENS(usLenStr, pTokenList)   \
         {                                      \
           PFUZZYTOK pTest = pTokenList;        \
           usLenStr = 0;                        \
           while ( pTest->ulHash )              \
           {                                    \
             usLenStr++;                        \
             pTest++;                           \
           } /* endwhile */                     \
         }


#ifdef EQFBFUZZ_LOG

static FILE *LogOpen()
{
  FILE *hfLog;
  CHAR szLogFile[MAX_EQF_PATH]; 

  UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
  strcat( szLogFile, "\\EQFBFUZZ.LOG" );
  hfLog = fopen( szLogFile,"a+" );
  return( hfLog );
}

static void LogClose( FILE *hfLog )
{
  if ( hfLog )
  {
    fclose( hfLog );
  } /* endif */
}

void WriteLogString( FILE *hf, PSZ_W pszString )
{
  static CHAR_W szBuffer[1024];
  PSZ_W pszTemp = szBuffer;
  
  while ( *pszString)
  {
    if ( *pszString == L'\r' )
    {
      wcscpy( pszTemp, L"<cr>" );
      pszTemp += 4;
      pszString++;
    }
    else if ( *pszString == L'\n' )
    {
      wcscpy( pszTemp, L"<lf>" );
      pszTemp += 4;
      pszString++;
    }
    else
    {
      *pszTemp++ = *pszString++;
    } /* endif */
  } /*endwhile */;
  *pszTemp = 0;
  fprintf( hf, "%S\n", szBuffer );
}

void LogToken( FILE *hfLog, PFUZZYTOK pToken )
{
  CHAR_W chTemp;
  SHORT sLen = (SHORT)(pToken->usStop  - pToken->usStart + 1);
  PSZ_W pszEnd = pToken->pData + sLen;
  chTemp = *pszEnd;
  *pszEnd = 0;
  fprintf( hfLog, "H=%10.10lu L=%3d Conn=%s Type=%-4s, Token=\"%S\"\n", pToken->ulHash, sLen, pToken->fConnected ? "Y" : "N", 
    (pToken->sType == TEXT_TOKEN) ? "Text" : "Tag", pToken->pData );
  *pszEnd = chTemp;
}

void LogReplEntry( FILE *hfLog, PREPLLIST pEntry )
{
  CHAR_W chTempSource, chTempTarget;
  int iLenSource, iLenTarget;

  iLenSource = pEntry->pSrcTok->usStop - pEntry->pSrcTok->usStart + 1;
  iLenTarget = pEntry->pTgtTok->usStop - pEntry->pTgtTok->usStart + 1;
  chTempSource = pEntry->pSrcTok->pData[iLenSource];
  chTempTarget = pEntry->pTgtTok->pData[iLenTarget];
  pEntry->pSrcTok->pData[iLenSource] = 0;
  pEntry->pTgtTok->pData[iLenTarget] = 0;
  fprintf( hfLog, "\"%S\" <=> \"%S\"\n", pEntry->pSrcTok->pData, pEntry->pTgtTok->pData ); 
  pEntry->pSrcTok->pData[iLenSource] = chTempSource;
  pEntry->pTgtTok->pData[iLenTarget] = chTempTarget;
}

void LogTokenList( PFUZZYTOK pToken, PSZ pszTitle )
{
  FILE *hfLog = LogOpen();
  if ( hfLog )
  {
    fprintf( hfLog, "%s\nTokens:\n", pszTitle );

    while ( pToken->usStop )
    {
      LogToken( hfLog, pToken++ );
    } /*endwhile */

    LogClose( hfLog );
  } /* endif */
}

#endif

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBPrepareFuzzyProp                                     |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBPrepareFuzzyProp( pDoc, pSource, pProp,              |
//|                                               pTrans, sLanguageId );       |
//+----------------------------------------------------------------------------+
//|Description:       Find tokens that pProp and pTrans have in common.        |
//|                   Those tokens are the ones which might be replaced        |
//|                   automatically.                                           |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT  pDoc,       pointer to document ida         |
//|                   PSZ          pSource,    pointer to source               |
//|                   PSZ          pProp,      pointer to source of proposal   |
//|                   PSZ          pTrans,     pointer to Translation          |
//|                   SHORT        sLanguageId source language Id..            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       1 (TRUE) everything okay                                 |
//|                   0(FALSE) allocation or tokenisation is not possible      |
//|                   2 not everything replaced                                |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFBPrepareFuzzyPropEx with add.param NULL          |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+
USHORT
EQFBPrepareFuzzyProp
(
  PTBDOCUMENT  pDoc,                   // pointer to document ida
  PSZ_W        pSource,                // pointer to source
  PSZ_W        pProp,                  // pointer to source of proposal
  PSZ_W        pTrans,                 // pointer to Translation
  SHORT        sLanguageId,            // source language Id..
  SHORT        sTgtLangId              // target language id..
)
{
  return EQFBPrepareFuzzyPropEx2(
               pSource,                // pointer to source
               pProp,                  // pointer to source of proposal
               pTrans,                 // pointer to Translation
               sLanguageId,            // source language Id..
               sTgtLangId,             // target language id..
               NULL,                   // fuzzy level not interesting
               pDoc->pDocTagTable,     // source document tag table
               pDoc->pDocTagTable,     // target document tag table
               pDoc->pInBuf,           // buffer for input
               pDoc->pTokBuf,          // buffer for tokens
               0L, 0L);
} /* end of function EQFBPrepareFuzzyProp */

USHORT
EQFBPrepareFuzzyPropEx
(
  PTBDOCUMENT  pDoc,                   // pointer to document ida
  PSZ_W        pSource,                // pointer to source
  PSZ_W        pProp,                  // pointer to source of proposal
  PSZ_W        pTrans,                 // pointer to Translation
  SHORT        sLanguageId,            // source language Id..
  SHORT        sTgtLangId,             // target language id..
  PUSHORT      pusFuzzy,
  ULONG        ulSrcOemCP,             // src lang cp
  ULONG        ulTgtOemCP              // tgt lang cp
)
{
  return EQFBPrepareFuzzyPropEx3(
                   pSource,            // pointer to source
                   pProp,              // pointer to source of proposal
                   pTrans,             // pointer to Translation
                   sLanguageId,        // source language Id..
                   sTgtLangId,         // target language id..
                   pusFuzzy,           // we are looking for fuzzy level
                   pDoc->pDocTagTable, // source document tag table
                   pDoc->pDocTagTable, // target document tag table
                   pDoc->pInBuf,       // buffer for input
                   pDoc->pTokBuf,       // buffer for tokens
                   ulSrcOemCP,
                   ulTgtOemCP,
                   NULL );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBPrepareFuzzyPropEx                                   |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBPrepareFuzzyPropEx( pDoc, pSource, pProp,            |
//|                                         pTrans, sLanguageId, pusFuzzy );   |
//+----------------------------------------------------------------------------+
//|Description:       Find tokens that pProp and pTrans have in common.        |
//|                   Those tokens are the ones which might be replaced        |
//|                   automatically.                                           |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT  pDoc,       pointer to document ida         |
//|                   PSZ          pSource,    pointer to source               |
//|                   PSZ          pProp,      pointer to source of proposal   |
//|                   PSZ          pTrans,     pointer to Translation          |
//|                   SHORT        sLanguageId source language Id..            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       1 (TRUE) everything okay                                 |
//|                   0(FALSE) allocation or tokenisation is not possible      |
//|                   2 not everything replaced                                |
//+----------------------------------------------------------------------------+
//|Function flow:     Prepare a token list for proposal and translation.       |
//|                   if okay then                                             |
//|                     create a list of replacement candidates                |
//|                   endif                                                    |
//|                   if okay and replacements possible then                   |
//|                     create a token list for source                         |
//|                     if okay then                                           |
//|                       determine common structure of source and proposal    |
//|                     endif                                                  |
//|                   endif                                                    |
//|                   if okay and common structure                             |
//|                     replace all common ones.                               |
//|                   endif                                                    |
//|                   free allocated resources                                 |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+
USHORT
EQFBPrepareFuzzyPropEx2
(
  PSZ_W        pSource,                // pointer to source
  PSZ_W        pProp,                  // pointer to source of proposal
  PSZ_W        pTrans,                 // pointer to Translation
  SHORT        sLanguageId,            // source language Id..
  SHORT        sTgtLangId,             // target language id..
  PUSHORT      pusFuzzy,
  PVOID        pSourceTagTable,
  PVOID        pPropTagTable,
  PBYTE        pInBuf,
  PBYTE        pTokBuf,
  ULONG        ulSrcOemCP,
  ULONG        ulTgtOemCP
)
{
  return( EQFBPrepareFuzzyPropEx3( pSource, pProp, pTrans, sLanguageId, sTgtLangId, pusFuzzy, pSourceTagTable, pPropTagTable, pInBuf, pTokBuf, ulSrcOemCP, ulTgtOemCP, NULL ) );
}

USHORT EQFBPrepareFuzzyPropEx3
(
  PSZ_W        pSource,                // pointer to source
  PSZ_W        pProp,                  // pointer to source of proposal
  PSZ_W        pTrans,                 // pointer to Translation
  SHORT        sLanguageId,            // source language Id..
  SHORT        sTgtLangId,             // target language id..
  PUSHORT      pusFuzzy,
  PVOID        pSourceTagTable,
  PVOID        pPropTagTable,
  PBYTE        pInBuf,
  PBYTE        pTokBuf,
  ULONG        ulSrcOemCP,
  ULONG        ulTgtOemCP,
  PUSHORT      pusWords
)
{
  BOOL  fOK = TRUE;                    // success indicator
  PFUZZYTOK pPropTokList = NULL;       // pointer to token list
  PFUZZYTOK pTransTokList = NULL;      // pointer to token list
  PFUZZYTOK pSourceTokList = NULL;     // pointer to token list
  PREPLLIST pReplaceList   = NULL;     // pointer to replace list
  PREPLLIST pReplPropSrc   = NULL;     // pointer to replace list
  PFUZZYTOK pTempTokList;              // pointer to token list
  USHORT    usToken;                   // number of tokens
  USHORT    usReturn = 0;
  PFUZZYTOK pTempPropList;             // temp ptr to token list
  BOOL      fReplaced = FALSE;         // true if something replaced
  PFUZZYTOK pCopyTokList1 = NULL;
  PFUZZYTOK pCopyTokList2 = NULL;
  USHORT    usPropLen = 0;
  USHORT    usLen2;
  BOOL      fConnected = FALSE;
  USHORT    k;

  /***********************************************************************/
  /* find tokens that pProp and pTrans ( source of proposal and proposal)*/
  /*  have in common - those tokens                                      */
  /* are the ones which might be replaced automatically                  */
  /***********************************************************************/
  if (!IsDBCS_CP(ulTgtOemCP))
  {
    sTgtLangId = sLanguageId;     // use Src Lang for both strings
  } /* endif */
  if ( fOK )
  {
    fOK = PrepareTokens( (PLOADEDTABLE)pPropTagTable, pInBuf, pTokBuf, pProp, sLanguageId, &pPropTokList, ulSrcOemCP );

    // fill caller's word count variable
    if ( fOK && (pusWords != NULL) )
    {
      USHORT usWords = 0;
      PFUZZYTOK pTestToken = pPropTokList;
      while ( pTestToken->usStop )
      {
        if ( pTestToken->sType == TEXT_TOKEN )
        {
          usWords++;
        } /* endif */           
        pTestToken++;
      } /*endwhile */
      *pusWords = usWords;
    } /* endif */       


#ifdef EQFBFUZZ_LOG
    LogTokenList( pPropTokList, "PropTokList" );
#endif
    if ( fOK )
    {
      if (IsDBCS_CP(ulTgtOemCP))
      {
        fOK = PrepareTokens( (PLOADEDTABLE)pPropTagTable, pInBuf, pTokBuf, pTrans, sTgtLangId, &pTransTokList, ulTgtOemCP); 

      }
      else
      {
        fOK = PrepareTokens( (PLOADEDTABLE)pPropTagTable, pInBuf, pTokBuf, pTrans, sLanguageId, &pTransTokList, ulSrcOemCP);
      } /* endif */
#ifdef EQFBFUZZ_LOG
      LogTokenList( pTransTokList, "TransTokList" );
#endif
    } /* endif */
    if (fOK )
    {
      /********************************************************************/
      /* get number of tokens in strings                                  */
      /* make a copy of tokenlist to be able to restore the sType (TEXT or*/
      /* whatever);sType is needed in FuzzyReplace; but in LCS this field */
      /* is used to store the MARK_EQUAL indicator                        */
      /********************************************************************/
      NUMBEROFTOKENS(usPropLen, pPropTokList);
      NUMBEROFTOKENS(usLen2, pTransTokList);
      fOK = UtlAlloc( (PVOID *)&pCopyTokList1,
                      0L,
                      (LONG) (usPropLen +1) * sizeof(FUZZYTOK),
                      ERROR_STORAGE );
      if (fOK )
      {
        fOK = UtlAlloc( (PVOID *)&pCopyTokList2,
                        0L,
                        (LONG) (usLen2 +1) * sizeof(FUZZYTOK),
                        ERROR_STORAGE );
      } /* endif */
      if (fOK )
      {
        pTempTokList = pPropTokList;
        for ( k = 0 ; k<=(usPropLen-1); k++ )
        {
          *(pCopyTokList1+k) = *pTempTokList;
          pTempTokList++;
        } /* endfor */

        pTempTokList = pTransTokList;
        for ( k = 0 ; k <= (usLen2-1);  k++ )
        {
          *(pCopyTokList2+k) = *pTempTokList;
          pTempTokList++;
        } /* endfor */

        usReturn = FuzzyLCSReplList( pPropTokList, pTransTokList,
                                     &pReplaceList , usPropLen,
                                     usLen2, NULL, FALSE);

        pTempTokList = pPropTokList;
        for ( k = 0 ; k<=(usPropLen-1); k++ )
        {
          fConnected = pTempTokList->fConnected;
          *pTempTokList = *(pCopyTokList1+k);
          pTempTokList->fConnected = (EQF_BOOL)fConnected;
          pTempTokList++;
        } /* endfor */

        pTempTokList = pTransTokList;
        for ( k = 0 ; k <= (usLen2-1);  k++ )
        {
          fConnected = pTempTokList->fConnected;
          *pTempTokList = *(pCopyTokList2+k);
          pTempTokList->fConnected = (EQF_BOOL)fConnected;
          pTempTokList++;
        } /* endfor */

#ifdef EQFBFUZZ_LOG
    LogTokenList( pTransTokList, "TransTokList after FuzzyLCSReplList" );
    LogTokenList( pPropTokList, "PropTokList after FuzzyLCSReplList" );
#endif

        fOK = (BOOL) usReturn;

        UtlAlloc( (PVOID *)&pCopyTokList2, 0L, 0L, NOMSG);
      } /* endif */
    } /* endif */
  } /* endif */
  /********************************************************************/
  /* if there are tokens to be automatically replaced                 */
  /********************************************************************/
  if ( fOK && pReplaceList )
  {
    /******************************************************************/
    /* find common structure in pSource and pProp and detect          */
    /* possibilities to be replaced ..                                */
    /******************************************************************/
    if ( fOK )
    {
      fOK = PrepareTokens( (PLOADEDTABLE)pSourceTagTable, pInBuf, pTokBuf, pSource, sLanguageId, &pSourceTokList, ulSrcOemCP );
#ifdef EQFBFUZZ_LOG
      LogTokenList( pSourceTokList, "SourceTokList" );
#endif
      if (fOK )
      {
        NUMBEROFTOKENS(usLen2, pSourceTokList);
        fOK = UtlAlloc( (PVOID *)&pCopyTokList2,
                        0L,
                        (LONG) (usLen2 +1) * sizeof(FUZZYTOK),
                        ERROR_STORAGE );
        if (fOK )
        {
          pTempTokList = pSourceTokList;
          for ( k = 0 ; k <= (usLen2-1);  k++ )
          {
            *(pCopyTokList2+k) = *pTempTokList;
            pTempTokList++;
          } /* endfor */

          usReturn = FuzzyLCSReplList( pPropTokList, pSourceTokList,
                                 &pReplPropSrc, usPropLen,
                                 usLen2, pusFuzzy, FALSE );
          fOK = (BOOL) usReturn;
          pTempTokList = pPropTokList;
          for ( k = 0 ; k<=(usPropLen-1); k++ )
          {
            fConnected = pTempTokList->fConnected;
            *pTempTokList = *(pCopyTokList1+k);
            pTempTokList->fConnected = (EQF_BOOL)fConnected;
            pTempTokList++;
          } /* endfor */

          pTempTokList = pSourceTokList;
          for ( k = 0 ; k <= (usLen2-1);  k++ )
          {
            fConnected = pTempTokList->fConnected;
            *pTempTokList = *(pCopyTokList2+k);
            pTempTokList->fConnected = (EQF_BOOL)fConnected;
            pTempTokList++;
          } /* endfor */
#ifdef EQFBFUZZ_LOG
    LogTokenList( pPropTokList, "PropTokList after FuzzyLCSReplList with PropSource" );
    LogTokenList( pSourceTokList, "SourceTokList after FuzzyLCSReplList with PropSource" );
#endif
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */
  UtlAlloc( (PVOID *)&pCopyTokList1, 0L, 0L, NOMSG);
  UtlAlloc( (PVOID *)&pCopyTokList2, 0L, 0L, NOMSG);
  /******************************************************************/
  /* check that we have a large portion in common ....              */
  /******************************************************************/
  if ( fOK )
  {
    if ( !pReplPropSrc )
    {
      /************************************************************/
      /* create a dummy token if empty                            */
      /************************************************************/
      fOK = UtlAlloc( (PVOID *)&pReplPropSrc, 0L,
              (LONG)( (MAX_REPL+1) * sizeof(REPLLIST) ),
              ERROR_STORAGE );
    }
    else                                                 // old part
    {
      pTempTokList = pSourceTokList;
      usToken = 0;                       // count tokens in srcstring
      while ( pTempTokList->ulHash)
      {
        usToken ++;
        pTempTokList++;
      } /* endwhile */

      if ( usToken && (2 * usReturn >= usToken))
      {
        /************************************************************/
        /* if at least half of the tokens in current active source  */
        /* is equal with the tokens in the source of proposal,      */
        /* okay, good enough ...                                    */
        /************************************************************/
      }
      else
      {
        /************************************************************/
        /* free it - not good enough                                */
        /************************************************************/
        UtlAlloc( (PVOID *)&pReplPropSrc,   0L, 0L, NOMSG );
        fOK = FALSE;
      }  /* endif */
    } /* endif */
  } /* endif */


    if ( fOK && pReplPropSrc )
    {
      /*************************************************************/
      /* insert a dummy token at the beginning of the pReplPropSrc */
      /* list to have an anchor where to start from...             */
      /*************************************************************/
      memmove( pReplPropSrc+1, pReplPropSrc, sizeof( REPLLIST ) * MAX_REPL);

      pReplPropSrc->pSrcTok = pPropTokList;
      pReplPropSrc->pTgtTok = pSourceTokList;
//      pReplPropSrc->pSrcTok --;                              /* @KIT1171D */
//      pReplPropSrc->pTgtTok --;                              /* @KIT1171D */

  /********************************************************************/
  /* printout settings into test file (if required)                   */
  /********************************************************************/
#ifdef EQFBFUZZ_LOG
  {
    FILE *fOutFile;
    PREPLLIST pReplTest = NULL;          // pointer to replace list

    fOutFile = LogOpen();

    fwprintf( fOutFile, L"pSource: %s\n", pSource );
    fwprintf( fOutFile, L"pProp: %s\n", pProp );
    fwprintf( fOutFile, L"pTrans: %s\n", pTrans);
    fprintf( fOutFile, "\nComparison between both sources ...\n" );
    pReplTest = pReplPropSrc;
    while ( pReplTest->pSrcTok)
    {
      LogReplEntry( fOutFile, pReplTest );       
      pReplTest++;
    } /* endwhile */

    fprintf( fOutFile, "\ncomparision between source and trans\n" );
    pReplTest = pReplaceList;
    while ( pReplTest->pSrcTok)
    {
      LogReplEntry( fOutFile, pReplTest );       
      pReplTest++;
    } /* endwhile */

    fprintf( fOutFile, "\n\n" );
    LogClose( fOutFile );
  }
#endif
      /******************************************************************/
      /* replace all detected ones ....                                 */
      /******************************************************************/
      fReplaced = FuzzyReplace( pSource, pProp, pTrans, pReplPropSrc, pReplaceList );

#ifdef EQFBFUZZ_LOG
      {
        FILE *fOutFile;

        fOutFile = LogOpen();

        if ( fReplaced )
        {
          fwprintf( fOutFile, L"\nStrings have been replaced:\n" );
          fwprintf( fOutFile, L"pSource: %s\n", pSource );
          fwprintf( fOutFile, L"pProp: %s\n", pProp );
          fwprintf( fOutFile, L"pTrans: %s\n", pTrans);
        }
        else
        {
          fwprintf( fOutFile, L"\nNo replacements performed\n" );
        } /* endif */
        LogClose( fOutFile );
      }
#endif
  } /* endif */

  if ( fOK && fReplaced && pSourceTokList && pPropTokList)
  {
    pTempTokList = pSourceTokList;
    pTempPropList = pPropTokList;
    while ( (pTempTokList->ulHash) && pTempTokList->fConnected
           && ((pTempPropList->ulHash) && pTempPropList->fConnected)  )
    {
      pTempTokList++;
      pTempPropList++;
    } /* endwhile */
    if (!( pTempTokList->ulHash) && !(pTempPropList->ulHash) )
    {
      usReturn = PROP_REPLACED;              // end of list, all connected
    }
    else
    {
      usReturn = PROP_REPLACED_FUZZY;            //not all replaced
    } /* endif */
  }
  else
  {
    usReturn = PROP_NOT_REPLACED;
  } /* endif */
  /********************************************************************/
  /* free resources not needed any more ....                          */
  /********************************************************************/
  UtlAlloc( (PVOID *)&pSourceTokList, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *)&pTransTokList,  0L, 0L, NOMSG );
  UtlAlloc( (PVOID *)&pPropTokList,   0L, 0L, NOMSG );
  UtlAlloc( (PVOID *)&pReplaceList,   0L, 0L, NOMSG );
  UtlAlloc( (PVOID *)&pReplPropSrc,   0L, 0L, NOMSG );


  return ( usReturn);
} /* end of function PrepareFuzzyPropEx */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     PrepareTokens                                            |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = PrepareTokens( pDoc, pString, sId, &pTokList );    |
//+----------------------------------------------------------------------------+
//|Description:       prepare the token list and build hash values for them    |
//+----------------------------------------------------------------------------+
//|Parameters:        PVOID         pTagTable      ptr to tagtable             |
//|                   PBYTE         pInBuf         temp work buffer            |
//|                   PBYTE         pTokBuf        token buffer                |
//|                   PSZ           pString,       ptr to string               |
//|                   SHORT         sLanguageId,   language ID                 |
//|                   PFUZZYTOK   * ppTransTokList resulting list of tokens    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     everything okay                                 |
//|                   FALSE    allocation went wrong                           |
//+----------------------------------------------------------------------------+
//|Function flow:     if not done yet                                          |
//|                     prepare random sequence (according to K.Wirth)         |
//|                   call EQFTagTokenize to tokenize the passed string        |
//|                   build tokenlist, i.e. convert tokens/text strings to     |
//|                      entries in start-stop table                           |
//|                      (Note: for text tokens returned we have to call       |
//|                             MorphTokenize to split them up further )       |
//|                   allocate memory for start/stop table and copy list into  |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+
static BOOL
PrepareTokens
(
  PLOADEDTABLE  pTagTable,
  PBYTE         pInBuf,
  PBYTE         pTokBuf,
  PSZ_W         pString,               // pointer to string to be tokenized
  SHORT         sLanguageId,           // language ID
  PFUZZYTOK   * ppTransTokList,         // resulting list of tokens
  ULONG         ulOemCP
)
{
  PCHAR_W     pRest;                   // ptr to start of not-processed bytes
  USHORT      usColPos = 0;            // column pos used by EQFTagTokenize
  PTOKENENTRY pTok;                    // ptr for token table processing
  USHORT      usCurOffs;               // current offset into segment
  ULONG       ulLength;                // length of start/stop table
  PFUZZYTOK   pstCurrent;              // ptr to entries of start/stop table
  CHAR_W      chTemp;                  // temp character
  BOOL        fOK = TRUE;              // success indicator
  USHORT      usRandomIndex;           // index in random sequence
  USHORT      usListSize;              // size of buffer
  PTERMLENOFFS  pTermList;             // ptr to created term list
  PTERMLENOFFS  pActTerm;              // actual term
  USHORT       usRC;                   // return code
  SHORT        sNumTags;               // number of tags       /* KIT0857A    */
  BOOL         fTag;                   // currently in tagging /* KIT0857A    */
  PFUZZYTOK    pstAct;                 // ptr start/stop table /* KIT0857A    */
  PSZ_W        pStart;

   if ( !ulRandom[0] )
   {
     /********************************************************************/
     /* random sequences, see e.g. the book of Wirth...                  */
     /********************************************************************/
     ulRandom[0] = 0xABCDEF01;
     for (usRandomIndex = 1; usRandomIndex < MAX_RANDOM; usRandomIndex++)
     {
         ulRandom[usRandomIndex] = ulRandom[usRandomIndex - 1] * 5 + 0xABCDEF01;
     }
   } /* endif */
  /********************************************************************/
  /* run TATagTokenize to find tokens ....                            */
  /********************************************************************/
  TATagTokenizeW(pString,
                 pTagTable,
                 TRUE,
                 &pRest,
                 &usColPos,
                 (PTOKENENTRY) pTokBuf,
                 TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );

  /********************************************************************/
  /* build tokenlist, i.e.                                            */
  /* convert tokens/text strings to entries in start-stop table       */
  /*                                                                  */
  /* Rational: use input buffer for temporary list ....               */
  /*           this is large enough, we can avoid any checking...     */
  /********************************************************************/
  pstCurrent = (PFUZZYTOK) pInBuf;  // use input buffer for table
  pTok = (PTOKENENTRY) pTokBuf;
  usCurOffs = 0;
  pStart = pTok->pDataStringW;
  while ( (pTok->sTokenid != ENDOFLIST) )
  {
    if ( pTok->sTokenid == TEXT_TOKEN )
    {
      usListSize = 0;
      pTermList = NULL;

      chTemp = *(pTok->pDataStringW+pTok->usLength);
      *(pTok->pDataStringW+pTok->usLength) = EOS;

      usRC = MorphTokenizeW( sLanguageId, pTok->pDataStringW,
                            &usListSize, (PVOID *)&pTermList,
                            MORPH_OFFSLIST, ulOemCP );

      *(pTok->pDataStringW+pTok->usLength) = chTemp;

      if ( pTermList )
      {
        pActTerm = pTermList;
        if ( pTermList->usLength )
        {
          while ( pActTerm->usLength )
          {
            pString = pTok->pDataStringW + pActTerm->usOffset;
            /**********************************************************/
            /* ignore the linefeeds and tabs in the matching          */
            /**********************************************************/
            if ( (*pString != LF) && (*pString != 0x09))
            {
              pstCurrent = SplitTokens(pstCurrent,
                                       (USHORT)(usCurOffs + pActTerm->usOffset),
                                       TEXT_TOKEN,
                                       pActTerm->usLength,
                                       pString);
            } /* endif */
            pActTerm++;
          } /* endwhile */
        }
        else
        {
          pstCurrent = SplitTokens(pstCurrent,
                                   usCurOffs,
                                   pTok->sTokenid,
                                   pTok->usLength,
                                   pTok->pDataStringW);

        } /* endif */
      } /* endif */
      /****************************************************************/
      /* free allocated resource ...                                  */
      /****************************************************************/
      UtlAlloc( (PVOID *)&pTermList, 0L, 0L, NOMSG );
    }
    else
    {
      pstCurrent =  SplitTokens(pstCurrent,
                               usCurOffs,
                               pTok->sTokenid,
                               pTok->usLength,
                               pTok->pDataStringW);
    } /* endif */
    /****************************************************************/
    /* adjust current offset to point to new offset in string...    */
    /****************************************************************/
    pTok++;
    usCurOffs = (USHORT)(pTok->pDataStringW - pStart);
  } /* endwhile */

  // terminate start/stop table
  memset( pstCurrent, 0, sizeof(  FUZZYTOK ));


  /********************************************************************/
  /* get number of tags ...                                           */
  /********************************************************************/
  sNumTags = (SHORT) pTagTable->pTagTable->uNumTags;
  /********************************************************************/
  /* streamline tokenlist, i.e. treat tags and their attributes as    */
  /* one unit                                                         */
  /*    pstAct -- points to active one                                */
  /*    pstCurrent  -- points to next one...                          */
  /********************************************************************/
  pstCurrent = (PFUZZYTOK) pInBuf;         // use input buffer for table
  pstAct = pstCurrent;
  /********************************************************************/
  /* set tag indication                                               */
  /********************************************************************/
  fTag = ((pstCurrent->sType < sNumTags ) &&
          (pstCurrent->sType >= 0));
  pstCurrent++;                                  // point to next entry

  while ( pstCurrent->usStop )
  {
    if ( pstCurrent->sType < sNumTags )          // tags or text;
    {
      fTag = ( pstCurrent->sType >= 0 );
      pstAct++;
      memcpy( pstAct, pstCurrent, sizeof( FUZZYTOK ));
      pstCurrent++;
    }
    else                                        // it is an attribute
    {
      /****************************************************************/
      /* if we are in a tag -- join it, i.e. update the length        */
      /****************************************************************/
      if ( fTag )
      {
        pstAct->usStop = pstCurrent->usStop;
        pstAct->ulHash += pstCurrent->ulHash;
        pstCurrent++;
      }
      else
      {
        pstAct++;
        memcpy( pstAct, pstCurrent, sizeof( FUZZYTOK ));
        pstCurrent++;
      } /* endif */
    } /* endif */
  } /* endwhile */
  /********************************************************************/
  /* put in end indication                                            */
  /********************************************************************/
  pstAct++;
  memcpy( pstAct, pstCurrent, sizeof( FUZZYTOK ));
  /********************************************************************/
  /* have to point to next one, because we use it as length parameter */
  /********************************************************************/
  pstAct++;

  /********************************************************************/
  /* allocate memory for start/stop table and copy list into it       */
  /********************************************************************/
  *ppTransTokList = NULL;              // init token list..
  ulLength = (ULONG)(((PBYTE) pstAct) - pInBuf);

  fOK = UtlAlloc( (PVOID *)ppTransTokList, 0L, ulLength, ERROR_STORAGE );
  if ( fOK )
  {
    memcpy( *ppTransTokList, pInBuf, ulLength );
  } /* endif */

  return( fOK );
} /* end of function 
  */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     SplitTokens                                              |
//+----------------------------------------------------------------------------+
//|Function call:     SplitTokens(PFUZZYTOK, USHORT, USHORT, USHORT, PSZ)      |
//+----------------------------------------------------------------------------+
//|Description:       build pstCurrent and split token if it contains          |
//|                   digits or punctuations                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   PFUZZYTOK                                                |
//+----------------------------------------------------------------------------+
//|Returncodes:       pstCurrent  points to next free position                 |
//+----------------------------------------------------------------------------+
//|Function flow:     splitting of tokens added on request,                    |
//|                   now deleted again (July 93)                              |
//+----------------------------------------------------------------------------+

static PFUZZYTOK SplitTokens
 (
  PFUZZYTOK  pstCurrent,
  USHORT     usStart,
  SHORT      sType,
  USHORT     usLength,
  PSZ_W      pString
 )
{
  CHAR_W    chTemp;
  pstCurrent->pData = pString;
  pstCurrent->usStart = usStart;
  pstCurrent->sType   = sType;
  pstCurrent->usStop  = usStart + usLength - 1;
  pstCurrent->fConnected = FALSE;
  chTemp = *(pString + usLength);
  *(pString + usLength) = EOS;
  MakeHashValue ( ulRandom, MAX_RANDOM,
                  pString , &pstCurrent->ulHash );
  *(pString + usLength) = chTemp;
  pstCurrent++;

  return (pstCurrent);
} /* end of function SplitTokens */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MakeHashValues                                           |
//+----------------------------------------------------------------------------+
//|Function call:     MakeHashValues( pulRandom, usMaxNum, pszString, pulHash);|
//+----------------------------------------------------------------------------+
//|Description:       build a quasi hash value of the passed string            |
//+----------------------------------------------------------------------------+
//|Parameters:        PULONG    pulRandom,     array of random numbers         |
//|                   USHORT    usMaxRandom,   maximum random numbers          |
//|                   PSZ       pData,         ptr to data to be hashed        |
//|                   PULONG    pulHashVal     resulting hash value            |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     go through the passed string and build hashvalue         |
//|                   of all characters up to the specified max value          |
//|                   Characters not in the alphanumeric range are ignored..   |
//|                   (Check in case of DBCS might be necessary...)            |
//+----------------------------------------------------------------------------+
static VOID
MakeHashValue
(
  PULONG    pulRandom,                 // array of random numbers for hashing
  USHORT    usMaxRandom,               // maximum random numbers
  PSZ_W     pData,                     // ptr to data to be hashed
  PULONG    pulHashVal                 // resulting hash value
)
{
  USHORT usRandomIndex = 0;
  ULONG  ulHashVal = 0;
  CHAR_W c;

  while ( ((c = *pData++) != NULC) && (usRandomIndex < usMaxRandom))
  {
    if ( isalnum((BYTE)c ) )
    {
      ulHashVal += pulRandom[usRandomIndex++] * c;
    } /* endif */
  } /* endwhile */
  /********************************************************************/
  /* normalize hashvalue, just in case it is 0 and we may look at it  */
  /* as the end of our further processing....                         */
  /********************************************************************/
  if ( ulHashVal == 0L )
  {
    ulHashVal = 1L;
  } /* endif */

  *pulHashVal = ulHashVal;

  return;
} /* end of function MakeHashValue */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FuzzyReplace                                             |
//+----------------------------------------------------------------------------+
//|Function call:     FuzzyReplace( pSource, pProp, pTrans,                    |
//|                                 pReplPropSrc, pReplaceList );              |
//+----------------------------------------------------------------------------+
//|Description:       replace found tokens in translation with original ones   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ       pSource,       source string                   |
//|                   PSZ       pProp,         proposal string                 |
//|                   PSZ       pTrans,        translation string              |
//|                   PREPLLIST pReplPropSrc,  list of same tokens in source   |
//|                   PREPLLIST pReplaceList   list of tokens to be replaced   |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pTrans is large enough to hold a total segment in length |
//|                   of MAX_SEGMENT size                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     loop thru the list of candidates                         |
//|                     if next proposal token is part of the possible repl.   |
//|                     list, we've found one match.                           |
//|                     We have to compare until the next match between Source |
//|                     and proposal is detected or the end of the list ...    |
//|                      if we found a fuzzy match, we have to replace it now..|
//|                       i.e. call the TransferSource to do a fuzzy replace   |
//|                       adjust target string and trans token list if replace-|
//|                         ment string is of different length                 |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
static BOOL
FuzzyReplace
(
  PSZ_W     pSource,                   // source string
  PSZ_W     pProp,                     // proposal string
  PSZ_W     pTrans,                    // translation string
  PREPLLIST pReplPropSrc,              // list of same tokens in source and prop
  PREPLLIST pReplaceList               // list of tokens to be replaced
)
{
  PFUZZYTOK  pSrcTok = NULL;           // source token
  PFUZZYTOK  pTgtTok = NULL;           // target token
  PFUZZYTOK  pEndSrcTok;               // search until you detect this token
  PFUZZYTOK  pEndTgtTok;               // search until you detect this token
  PFUZZYTOK  pTgtTemp;                 // target token temporary
  PFUZZYTOK  pPropTok;                 // proposal token
  USHORT     usTgtLen;                 // target length
  SHORT      sDiff;                    // difference
  PREPLLIST  pTempRepl;                // pointer to temp repl element
  BOOL       fFound;                   // match found
  CHAR_W       chSource[ MAX_WORD_LEN ]; // source word to be copied
  CHAR_W       chProp[ MAX_WORD_LEN ];   // proposal used as pattern
  CHAR_W       chTrans[ MAX_WORD_LEN ];  // translation to be replaced
  SHORT      sLen;                     // length of string
  BOOL       fReplaced = FALSE;        // nothing replaced yet
  BOOL       fFirstInLoop = TRUE;      // sep. handling 1st time
  SHORT      sLenProp;                 // length of string

  usTgtLen = (USHORT)UTF16strlenCHAR( pTrans );
  /********************************************************************/
  /* replace found tokens in translation with original ones....       */
  /********************************************************************/
  while ( pReplPropSrc->pSrcTok )
  {
    pSrcTok = pReplPropSrc->pTgtTok;
    pPropTok   = pReplPropSrc->pSrcTok;

    /******************************************************************/
    /* if next proposal token is part of the possible replacement list*/
    /* we've found one match ...                                      */
    /* we have to compare until the next match between Source and     */
    /* Proposal is detected or the end of the list ....               */
    /******************************************************************/
    pEndSrcTok = (pReplPropSrc+1)->pTgtTok;
    pEndTgtTok = (pReplPropSrc+1)->pSrcTok;

    if ( !pEndSrcTok )
    {
      pEndSrcTok = pSrcTok;
      while ( pEndSrcTok->ulHash )
      {
        pEndSrcTok++;
      } /* endwhile */
    } /* endif */

    if ( !pEndTgtTok )
    {
      pEndTgtTok = pPropTok;
      while ( pEndTgtTok->ulHash )
      {
        pEndTgtTok++;
      } /* endwhile */
    } /* endif */
    /******************************************************************/
    /* adjustment for 1st time, earlier decreasing may affect         */
    /* pEndSrcTok/pEndTgtTok, if only 1 token in segment              */
    /******************************************************************/
    if ( fFirstInLoop )                                        /* @KIT1171A */
    {                                                          /* @KIT1171A */
      fFirstInLoop = FALSE;                                    /* @KIT1171A */
      pSrcTok--;                                               /* @KIT1171A */
      pPropTok--;                                              /* @KIT1171A */
    } /* endif */                                              /* @KIT1171A */

    while ( ++pSrcTok && ++pPropTok &&
             (pSrcTok < pEndSrcTok ) && (pPropTok < pEndTgtTok) )
    {
      pTempRepl = pReplaceList;
      fFound = FALSE;
      /****************************************************************/
      /*'if'nec :otherwise a tag can be replaced by a text            */
      /****************************************************************/
      if ( ((pSrcTok->sType == TEXT_TOKEN) && (pPropTok->sType == TEXT_TOKEN))
            || ((pSrcTok->sType != TEXT_TOKEN) && (pPropTok->sType != TEXT_TOKEN))  )
      {
        while ( pTempRepl->pSrcTok && !fFound )
        {
          if ( pTempRepl->pSrcTok == pPropTok )
          {
            fFound = TRUE;
            pTgtTok = pTempRepl->pTgtTok;
          }
          else
          {
            pTempRepl++;
          } /* endif */
        } /* endwhile */
      } /* endif */

      /******************************************************************/
      /* if we found a fuzzy match, we have to replace it now....       */
      /******************************************************************/
      if ( fFound )
      {
        pSrcTok->fConnected = TRUE;
        pPropTok->fConnected = TRUE;
        /******************************************************************/
        /* we have to deal with a fuzzy replace, e.g.                     */
        /*   '1,000.00' versus '1000,00'                                  */
        /* so call TransferSource to do it...                             */
        /******************************************************************/
        sLen = (pSrcTok->usStop - pSrcTok->usStart + 1);
        if ( sLen < MAX_WORD_LEN )
        {
          memcpy( (PBYTE)chSource, (PBYTE)(pSource+pSrcTok->usStart), sLen * sizeof(CHAR_W) );
          chSource[sLen] = EOS;
        }
        else
        {
          fFound = FALSE;
        } /* endif */

        sLen = (pPropTok->usStop - pPropTok->usStart + 1);
        if ( sLen < MAX_WORD_LEN )
        {
          memcpy( (PBYTE)chProp, (PBYTE)(pProp+pPropTok->usStart), sLen * sizeof(CHAR_W) );
          chProp[sLen] = EOS;
        }
        else
        {
          fFound = FALSE;
        } /* endif */

        sLen = (pTgtTok->usStop - pTgtTok->usStart + 1);
        if ( sLen < MAX_WORD_LEN )
        {
          memcpy( (PBYTE)chTrans, (PBYTE)(pTrans+pTgtTok->usStart), sLen * sizeof(CHAR_W) );
          chTrans[sLen] = EOS;
        }
        else
        {
          fFound = FALSE;
        } /* endif */
      } /* endif */
      if (fFound )
      {
        /**************************************************************/
        /* avoid that i.e. comma is replaced by 'und'                 */
        /* punctuation is only allowed to be replaced by punctuation  */
        /**************************************************************/
        sLen = (pSrcTok->usStop - pSrcTok->usStart + 1);
        sLenProp = (pPropTok->usStop - pPropTok->usStart + 1);
        if ((sLen == 1) && (ispunct((UCHAR) chSource[0])) )
        {
          if ((sLenProp != 1) || (!ispunct((UCHAR) chProp[0])) )
          {
            fFound = FALSE;
          } /* endif */
        }
        else
        {
          if ((sLenProp == 1) && (ispunct((UCHAR) chProp[0])) )
          {
            if ((sLen != 1) || (!ispunct((UCHAR) chSource[0])) )
            {
              fFound = FALSE;
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
      if (!fFound)
      {
        pSrcTok->fConnected = FALSE;
        pPropTok->fConnected = FALSE;
      } /* endif */
      /****************************************************************/
      /* if input parameters okay and source and target different     */
      /* do a replace                                                 */
      /****************************************************************/
      if ( fFound )
      {
        // do some checking first: is target token located in target string?
        if ( pTgtTok->usStop >= usTgtLen )
        {
          // target token is out of range so igonore it
        }
        else
        {
          /**************************************************************/
          /* source and proposal different, so do a replace ....        */
          /**************************************************************/
          if ( UTF16strcmp(chSource, chProp) )
          {
            TransferSource( chSource, chProp, chTrans );
            fReplaced = TRUE;
          } /* endif */
          sLen = (SHORT)UTF16strlenCHAR( chTrans );

          /******************************************************************/
          /* adjust target string and trans token list ....                 */
          /******************************************************************/
          sDiff = sLen - (pTgtTok->usStop - pTgtTok->usStart + 1);
          if ( sDiff != 0 )
          {
            /****************************************************************/
            /* replacement string is different                              */
            /****************************************************************/
            memmove( (PBYTE)(pTrans+pTgtTok->usStop+ sDiff),
                     (PBYTE)(pTrans+pTgtTok->usStop),
                      (usTgtLen - pTgtTok->usStop + 1) * sizeof(CHAR_W) );

            usTgtLen = usTgtLen + sDiff;
            pTgtTemp = pTgtTok;
            pTgtTemp++;                       // go to the next one...
            while ( pTgtTemp->usStart )
            {
              pTgtTemp->usStart = pTgtTemp->usStart + sDiff;
              pTgtTemp->usStop  = pTgtTemp->usStop  + sDiff;
              pTgtTemp++;
            } /* endwhile */
          } /* endif */

          memcpy( (PBYTE)(pTrans+pTgtTok->usStart), (PBYTE)chTrans, sLen * sizeof(CHAR_W) );
        } /* endif */ 
      } /* endif */
    } /* endwhile */
    pReplPropSrc++;                    // point to next one
  } /* endwhile */

  return( fReplaced );
} /* end of function FuzzyReplace */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBTokListCompare                                       |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBTokListCompare(PFUZZYTOK,PFUZZYTOK)                  |
//+----------------------------------------------------------------------------+
//|Description:       Compare routine for two tokens (called by qsort)         |
//+----------------------------------------------------------------------------+
//|Parameters:        PFUZZYTOK    pElem1                                      |
//|                   PFUZZYTOK    pElem2                                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   INT                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     compare two tokens                                       |
//+----------------------------------------------------------------------------+

//int EQFBTokListCompare ( const void * pvElem1, const void *pvElem2 )
//{
//  PFUZZYTOK pElem1 = *((PFUZZYTOK *)pvElem1);
//  PFUZZYTOK pElem2 = *((PFUZZYTOK *)pvElem2);
//  ULONG ul1, ul2;
//  int   i;
//  USHORT usStart1, usStart2;                             // temp. usStart
//  PSZ  pData1, pData2;
//
//  ul1 = pElem1->ulHash;
//  ul2 = pElem2->ulHash;
//  if ( ul1 > ul2  )
//  {
//    i = 1;
//  }
//  else
//  if ( ul1 < ul2 )
//  {
//    i = -1;
//  }
//  else
//  {
//    /******************************************************************/
//    /* if hash equal compare strings w/o regard of punctuation        */
//    /* compare punctuation chars only if no alphanumerical chars exist*/
//    /******************************************************************/
//    pData1 = pElem1->pData;
//    pData2 = pElem2->pData;
//    i = TokStrCompare( pElem1, pElem2 );
//
//    if ( i == 0 )
//    {
//      /******************************************************************/
//      /* if hash is equal, return i=0 only if start equal too           */
//      /*(hash equal happens if one word is more than once in a segment) */
//      /******************************************************************/
//       usStart1 = pElem1->usStart;
//       usStart2 = pElem2->usStart;
//       if (usStart1 < usStart2 )
//       {
//         i = -1;
//       }
//       else if (usStart1 > usStart2 )
//       {
//         i = 1;
//       }
//       else
//       {
//         i = 0;
//       } /* endif */
//    } /* endif */
//  } /* endif */
//  return( i );
//}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBReplListCompare                                      |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBReplListCompare(REPLLIST,REPLLIST )                  |
//+----------------------------------------------------------------------------+
//|Description:       Compare routine for two tokens (called by qsort)         |
//+----------------------------------------------------------------------------+
//|Parameters:        PREPLLIST    pElem1                                      |
//|                   PREPLLIST    pElem2                                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   INT                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     compare two tokens                                       |
//+----------------------------------------------------------------------------+

//int EQFBReplListCompare ( const void * pElem1, const void * pElem2 )
//{
//  return( (((PREPLLIST)pElem1)->pSrcTok->usStart -
//           ((PREPLLIST)pElem2)->pSrcTok->usStart) );
//}

// isNumber
//
// helper function for TransferSource: check if string is a number
static BOOL isNumber( PSZ_W pszText )
{
  BOOL fNumber = TRUE;

  while ( *pszText && fNumber )
  {
    CHAR_W chTest = *pszText;
    if ( iswdigit(chTest) || (chTest == L'.') || (chTest == L',') || (chTest == L'+') || (chTest == L'-')  )
    {
      pszText++; // test next one
    }
    else
    {
      fNumber = FALSE;  // this is no number
    } /* endif */
  } /*endwhile */

  return( fNumber );
} /* end of function isNumber */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TransferSource                                           |
//+----------------------------------------------------------------------------+
//|Function call:     TransferSource( chSource, chProp, chTrans );             |
//+----------------------------------------------------------------------------+
//|Description:       try to find out different editing patterns               |
//|                   like e.g. in American numbers versus European numbers    |
//|                     - 1,000,000.00   versus 1000000.00 or 1,000,000.00     |
//|                   and use this during copying of the source to the target  |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ pSource,      source string                          |
//|                   PSZ pProp,        proposal string                        |
//|                   PSZ pTrans        translation string                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pTrans is large enough to hold the replacement string    |
//+----------------------------------------------------------------------------+
//|Function flow:     try to find out different editing patterns               |
//|                   (use a searching from the back technology for detecting  |
//|                    differences like in the representation of numbers )     |
//|                   if something found to be edited do it during copying     |
//+----------------------------------------------------------------------------+

static VOID
TransferSource
(
  PSZ_W pSource,                         // source string
  PSZ_W pProp,                           // proposal string
  PSZ_W pTrans                           // translation string
)
{
  CHAR_W  chEditString[ MAX_WORD_LEN ];  // edit string
  CHAR_W  chSourceString[ MAX_WORD_LEN ];// source characters
  USHORT usI = 0;                      // index into edit string
  PSZ_W    pTempTrans;
  CHAR_W   c, d;                         // temporary characters
  int      sLen = 0;
  PSZ_W    pTemp = pProp;
  USHORT   usJ = 0;

  // skip changing if tag contains text within quoted strings!- text will be
  // corrupted totally otherwise! - P019753
  sLen = UTF16strlenCHAR(pTemp);
  while (sLen)
  {
	  if (*pTemp == DOUBLEQUOTE)
	  {
		  usJ++;
      }
      pTemp++;
      sLen--;
  }

  pTemp = pTrans;
  sLen = UTF16strlenCHAR(pTemp);
  while (sLen)
  {
  	  if (*pTemp == DOUBLEQUOTE)
  	  {
  		  usJ++;
      }
      pTemp++;
      sLen--;
  }


  // GQ: avoid editing for non-number words 
  if ( isNumber(pProp) && isNumber(pTrans) )
  {
    /********************************************************************/
    /* try to find out different editing patterns                       */
    /* like e.g. in American numbers versus European numbers            */
    /*   - 1,000,000.00   versus 1000000.00 or 1,000,000.00             */
    /********************************************************************/
    if (!usJ)
    {
	    pTempTrans = pTrans;
	    UTF16strrev( pTempTrans );
	    UTF16strrev( pProp );

	    while ( *pProp && *pTempTrans )
	    {
		  if ( *pProp != *pTempTrans )
		  {
		    chSourceString[usI] = *pProp;
		    chEditString[usI++] = *pTempTrans;
		  } /* endif */
		  pProp++;
		  pTempTrans++;
	    } /* endwhile */
    }
    /********************************************************************/
    /* if something found to be edited do it during copying             */
    /********************************************************************/
  }
  else
  {
    usI = 0; // no editing
  } /* endif */


  if ( usI )
  {
	chSourceString[usI] = EOS;
	chEditString[usI] = EOS;
    while ( (c = *pSource++) != NULC )
    {
      /****************************************************************/
      /* if c is within edit sequence use the modified character for  */
      /* it                                                           */
      /****************************************************************/
      usI = 0;
      while ( ((d = chSourceString[usI]) != NULC) && (d != c) )
      {
        usI++;
      } /* endwhile */

      if ( d != c )
      {
        *pTrans ++ = c;
      }
      else
      {
        *pTrans ++ = chEditString[usI];
      } /* endif */
    } /* endwhile */

    *pTrans     = EOS;                 // finish our string
  }
  else
  {
    UTF16strcpy( pTrans, pSource );
  } /* endif */
} /* end of function TransferSource */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TokStrCompare                                            |
//+----------------------------------------------------------------------------+
//|Function call:     TokStrCompare ( PFUZZYTOK, PFUZZYTOK);                   |
//+----------------------------------------------------------------------------+
//|Description:       compare 2 strings w/o regard of punctuation              |
//+----------------------------------------------------------------------------+
//|Parameters:        PFUZZYTOK       pPropTmpList,                            |
//|                   PFUZZYTOK       pTransTmpList                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       less than 0    pPropTmpList->pData less than other       |
//|                   0              strings equal                             |
//|                   greater than 0 pPropTmpList->pData greater than other    |
//+----------------------------------------------------------------------------+
//|Function flow:     set start points                                         |
//|                   while not end of token and characters equal              |
//|                     if character is alphanumerical                         |
//|                       compare character, set rc as described above         |
//|                     endif                                                  |
//|                   endif                                                    |
//|                   if no alphanumeric char in string                        |
//|                      memcmp both data strings                              |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

static SHORT
TokStrCompare
(
  PFUZZYTOK    pPropTmpList,           // pointer to token list
  PFUZZYTOK    pTransTmpList           // pointer to token list
)
{
  SHORT        sEqual = 0;             // init: strings equal
  USHORT       usTokenLen;
  USHORT       usPropTokenLen;
  USHORT       usLen;
  PSZ_W        pPropTmpData;           // temp ptr to pPropData
  PSZ_W        pTransTmpData;          // temp ptr to pTransData
  //BYTE         c;
  BOOL         fIsAlnum;               // true if alnum in string
  CHAR_W       cW;

  /********************************************************************/
  /* set init values                                                  */
  /********************************************************************/
  usTokenLen     = ((pTransTmpList->usStop) - (pTransTmpList->usStart) + 1);
  usPropTokenLen = ((pPropTmpList->usStop) - (pPropTmpList->usStart) + 1);
  usLen = min( usTokenLen, usPropTokenLen );
  pPropTmpData = pPropTmpList->pData;
  pTransTmpData = pTransTmpList->pData;

  /********************************************************************/
  /* loop thru all characters of both tokens                          */
  /* check if all alnum characters are equal                          */
  /********************************************************************/
  fIsAlnum = FALSE;
  while ( !sEqual && usTokenLen && usPropTokenLen )
  {
    cW = *pPropTmpData++;
    if ( iswalnum( cW ) )
    {
      fIsAlnum = TRUE;

      if ( cW != *pTransTmpData  )
      {
        sEqual = (cW < *pTransTmpData) ? -1 : 1;
      } /* endif */
    } /* endif */
    pTransTmpData++;
    usPropTokenLen--;
    usTokenLen--;
  } /* endwhile */

  // if strings are equal but lengths differ check remaining characters
  if ( fIsAlnum && (sEqual == 0) )
  {
    // alphanumeric characters found, check remaining part of tokens
    if ( usTokenLen ) 
    {
      // token string not at its end, test remaining characters
      while ( !sEqual && usTokenLen )
      {
        if ( iswalnum( *pTransTmpData ) )
        {
          sEqual = -1;
        } /* endif */

        pTransTmpData++;
        usTokenLen--;
      } /*endwhile */
    }
    else if ( usPropTokenLen ) 
    {
      // token string not at its end, test remaining characters
      while ( !sEqual && usPropTokenLen )
      {
        if ( iswalnum( *pPropTmpData ) )
        {
          sEqual = 1;
        } /* endif */

        pPropTmpData++;
        usPropTokenLen--;
      } /*endwhile */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* if no alnum character in string, compare other chars too         */
  /********************************************************************/
  if ( !fIsAlnum )
  {
    usPropTokenLen = ((pPropTmpList->usStop) - (pPropTmpList->usStart) + 1);
    usTokenLen = ((pTransTmpList->usStop) - (pTransTmpList->usStart) + 1);

    if (usTokenLen >= usPropTokenLen )
    {
      sEqual =  (SHORT)(memcmp((PBYTE)pPropTmpList->pData,
                       (PBYTE)pTransTmpList->pData, usTokenLen * sizeof(CHAR_W)));
    }
    else
    {
      sEqual =  (SHORT)(memcmp((PBYTE)pPropTmpList->pData,
                        (PBYTE)pTransTmpList->pData, usPropTokenLen * sizeof(CHAR_W)));
    } /* endif */

    // if strings match at their start
    if ( (sEqual == 0) && (usPropTokenLen != usTokenLen) )
    {
      // base compare on length of tokens...
      sEqual = (usPropTokenLen < usTokenLen) ? -1 : 1;
    } /* endif */
  } /* endif */
  return( sEqual );
} /* end of function TokStrCompare */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFindDiff                                             |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       return the differences two passed segments/strings will  |
//|                   have.                                                    |
//|                   It prepares a token list and then uses an algortihm as   |
//|                   proposed in an article of Meyer which compares those     |
//|                   strings in a O(n+m) performance and returns the edit     |
//|                   string, i.e. the script how you generate the second      |
//|                   string out of the first one.                             |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT  pDoc      pointer to document               |
//|                   PSZ pString1,          first string passed               |
//|                   PSZ pString2,          second string                     |
//|                   SHORT sLanguageId,     language id to be used            |
//|                   PFUZZYTOK * ppFuzzyTok returned token list               |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    every thing okay                                 |
//|                   FALSE   else                                             |
//+----------------------------------------------------------------------------+
//|Side effects:      The returned token list has to be freed by the caller    |
//|                   Each token has an identification if it's inserted, del., |
//|                   modified or equal.                                       |
//|                    MARK_EQUAL    'E'                                       |
//|                    MARK_DELETED  'D'                                       |
//|                    MARK_INSERTED 'I'                                       |
//|                    MARK_MODIFIED 'M'                                       |
//+----------------------------------------------------------------------------+
//|Function flow:     prepare tokens for string1 and string2                   |
//|                   get number of tokens in strings                          |
//|                   allocate space for backward token list                   |
//|                   fill backward token lists (nec for finding middlesnake)  |
//|                   find Longest Common Subsequence (LCS)                    |
//|                   adjust length of tokens to avoid non-marked spaces       |
//|                   allocate space for edit string ppFuzzyTok                |
//|                   fill edit string with marked tokens                      |
//|                   simplify marks                                           |
//|                   free forward and backward token lists                    |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+
BOOL
EQFBFindDiff
(
  PTBDOCUMENT  pDoc,                   // pointer to document
  PSZ_W pString1,                      // first string passed
  PSZ_W pString2,                      // second string
  SHORT sLanguageId,                   // language id to be used
  PFUZZYTOK * ppFuzzyTok,              // returned token list
  PUSHORT     pusModWords,             // num of differences
  PUSHORT     pusTokens,
  ULONG       ulOemCP
)
{
  BOOL  fOK;
  PFUZZYTOK    pFuzzyTgt = NULL;

  fOK = EQFBFindDiffEx( pDoc->pDocTagTable,
                          pDoc->pInBuf,
                          pDoc->pTokBuf,
                          pString1,
                          pString2,
                          sLanguageId,
                          (PVOID *)ppFuzzyTok,
                          (PVOID *)&pFuzzyTgt, ulOemCP );
  /**********************************************************************/
  /*  pFuzzyTgt is not needed, so free space                            */
  /**********************************************************************/
  if (pFuzzyTgt )
  {
    UtlAlloc ( (PVOID *)&pFuzzyTgt, 0L, 0L, NOMSG );
  } /* endif */

  if ( fOK )
  {
    EQFBCountDiff( *ppFuzzyTok, pusTokens, pusModWords );
  } /* endif */

  return fOK;
}

// counts the number of tokens and the number of differences
// in a fuzzy token list created using EQFBFindDiffEx
BOOL EQFBCountDiff
(
  PVOID     pvFuzzyTok,
  PUSHORT   pusTokens,
  PUSHORT   pusDiffs
)
{
   PFUZZYTOK pTest = (PFUZZYTOK)pvFuzzyTok;
   PFUZZYTOK    pTestToken;             // pointer to token lists
   PFUZZYTOK    pToken;                 // pointer to token lists
   USHORT usLen = 0;
   USHORT usDiff = 0;
   USHORT       usPrevType;

   while ( pTest->ulHash )
   {
     if (pTest->sType == MARK_DELETED)
     {
       usLen ++;
     }
     else if (pTest->sType == MARK_INSERTED)
     {
       usLen ++;
     }
     else
     {
       usLen ++;
     }
     pTest ++;
   } /* endwhile */

   /******************************************************************/
   /* browse through marks and simplifiy:                            */
   /* MODIFIED DELETED is changed to MODIFIED                        */
   /* MODIFIED INSERTED is changed to MODIFIED MODIFIED              */
   /******************************************************************/
   usDiff = 0;
   pTestToken = (PFUZZYTOK)pvFuzzyTok;               //resulting pointers
   pToken = (PFUZZYTOK)pvFuzzyTok;                   //current pointers
   usPrevType = MARK_EQUAL;

   while ( pToken->ulHash )
   {
     switch ( pToken->sType )
     {
       case MARK_DELETED:
         if ( usPrevType == MARK_MODIFIED )
         {
           while ( pToken->sType == MARK_DELETED )
           {
             pToken++;
             usDiff++;
           } /* endwhile */
         } /* endif */
         else if ( usPrevType == MARK_DELETED )
         {
           while ( pToken->sType == MARK_DELETED )
           {
             pToken++;
             usDiff++;
           } /* endwhile */
         } /* endif */
         break;
       case MARK_INSERTED:
         if ( usPrevType == MARK_MODIFIED )
         {
           pToken->sType = MARK_MODIFIED;
         } /* endif */
         break;
       default :
         break;
     } /* endswitch */
     if ( (pToken->ulHash) && (pToken->sType != MARK_EQUAL ))
     {
       usDiff++;
     } /* endif */
     *pTestToken = *pToken;
     usPrevType = pTestToken->sType;
     pTestToken++;
     if ( pToken->ulHash ) pToken++;
   } /* endwhile */

   *pTestToken = *pToken;                  // copy end of array

   *pusDiffs = usDiff;
   *pusTokens = usLen;
   return( TRUE );
} /* end of function EQFBCountDiff */


/**********************************************************************/
/* find the differences in two strings (w/o cleanup if consecutives   */
/* delete and inserts)                                                */
/**********************************************************************/
BOOL
EQFBFindDiffEx
(
  PVOID pTable,                        // pointer to loaded tagtable
  PBYTE pInBuf,                        // pointer to input buffer
  PBYTE pTokBuf,                       // pointer to temp token buffer
  PSZ_W pString1,                      // first string passed
  PSZ_W pString2,                      // second string
  SHORT sLanguageId,                   // language id to be used for pString1
  PVOID * ppFuzzyTok,                  // returned token list of pString2
  PVOID * ppFuzzyTgt,                   // returned token list of pString1
  ULONG   ulOemCP
)
{
  BOOL fOK = TRUE;                     // success indicator
  USHORT       usLenStr1 = 0;          // length of string 1 (in num. of tokens)
  USHORT       usLenStr2 = 0;          // length of string 2 (in num. of tokens)
  PFUZZYTOK    pTokenList2 = NULL;     // pointer to token lists
  PFUZZYTOK    pTokenList1 = NULL;     // pointer to token lists
  PFUZZYTOK    pToken;                 // pointer to token lists
  PLOADEDTABLE pTagTable = (PLOADEDTABLE) pTable;

  /******************************************************************/
  /* prepare tokens for String1 and string 2                        */
  /******************************************************************/
  fOK = PrepareTokens( //pDoc,
                       pTagTable,
                       pInBuf,
                       pTokBuf,
                       pString1, sLanguageId, &pTokenList1, ulOemCP );
  if ( fOK )
  {
    fOK = PrepareTokens( // pDoc,
                         pTagTable,
                         pInBuf,
                         pTokBuf,
                         pString2, sLanguageId, &pTokenList2, ulOemCP );
  } /* endif */
  if (fOK )
  {
    /********************************************************************/
    /* get number of tokens in strings                                  */
    /********************************************************************/
    NUMBEROFTOKENS(usLenStr1, pTokenList1);
    NUMBEROFTOKENS(usLenStr2, pTokenList2);

    /******************************************************************/
    /* call LCS and compare tokens with strncmp                       */
    /******************************************************************/
    fOK = EQFBCallLCS(pTokenList1, pTokenList2,
                      usLenStr1, usLenStr2, TRUE);
  } /* endif */
  /********************************************************************/
  /* get number of tokens in strings                                  */
  /********************************************************************/
  if ( fOK )
  {
    /****************************************************************/
    /* adjust length to avoid non-marked spaces between marked tokens*/
    /****************************************************************/
    pToken = pTokenList1;                   //current pointers
    while ( pToken->ulHash )
    {
      if ( (pToken->usStop < (pToken+1)->usStart) )
      {
        pToken->usStop = (pToken+1)->usStart - 1;
      } /* endif */
      pToken++;
    } /* endwhile */

    pToken = pTokenList2;                   //current pointers
    while ( pToken->ulHash )
    {
      if ( (pToken->usStop < (pToken+1)->usStart) )
      {
        pToken->usStop = (pToken+1)->usStart - 1;
      } /* endif */
      pToken++;
    } /* endwhile */
    /****************************************************************/
    /* allocate space for fuzzy tokens ,.,,,                        */
    /* this area MUST be freed by the calling application           */
    /****************************************************************/
    fOK = EQFBMarkModDelIns( pTokenList1, pTokenList2,
                          (PFUZZYTOK *)ppFuzzyTgt, (PFUZZYTOK *)ppFuzzyTok,
                         usLenStr1, usLenStr2);

  } /* endif */
  /********************************************************************/
  /* free any allocated resources                                     */
  /********************************************************************/
  UtlAlloc( (PVOID *)&pTokenList1, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *)&pTokenList2, 0L, 0L, NOMSG );
  return( fOK );
} /* end of function EQFBFindDiffEx */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBMarkModDelIns                                        |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       convert output from LCS into fuzzytoks with              |
//|                   MARK_MODIFIED, MARK_INSERTED, MARK_DELETED               |
//+----------------------------------------------------------------------------+
//|Parameters:        PFUZZYTOK   pTokenList1,     // tokenlist from LCS       |
//|                   PFUZZYTOK   pTokenList2,     // tokenlist from LCS       |
//|                   PFUZZYTOK * ppFuzzyTgt,      // returned token of str 1  |
//|                   PFUZZYTOK * ppFuzzyTok,      // returned token of str2   |
//|                   USHORT      usLenStr1,       // # of toks in TokenList1  |
//|                   USHORT      usLenStr2        // # of toks in tokenlist2  |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    every thing okay                                 |
//|                   FALSE   else                                             |
//+----------------------------------------------------------------------------+
//|Side effects:      The returned token list has to be freed by the caller    |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate ppFuzzy1 ppFuzzy2                               |
//|                   convert output from LCS into PFUZZYTOKS with             |
//|                   MARK_MODIFIED, MARK_DELETED, MARK_INSERTED               |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+
static BOOL
EQFBMarkModDelIns
(
    PFUZZYTOK   pTokenList1,
    PFUZZYTOK   pTokenList2,
    PFUZZYTOK  * ppFuzzy1,
    PFUZZYTOK  * ppFuzzy2,
    USHORT      usLenStr1,
    USHORT      usLenStr2
)

{
  BOOL         fOK;
  PFUZZYTOK    pTestToken;             // pointer to token lists 2
  USHORT       usStringA, usStringB;   // indices into strings (A=1, B=2)
  PFUZZYTOK    pT1Token;               // pointer to token lists 1

  LONG         lTokBufLen  = (LONG)(usLenStr1 + usLenStr2 + 1);
  LONG         lTokBufSize = lTokBufLen * sizeof(FUZZYTOK);
  LONG         lTokBufUsed = 0;

  fOK = UtlAlloc( (PVOID *)ppFuzzy2, 0L, lTokBufSize, ERROR_STORAGE );
  if (fOK ) fOK = UtlAlloc( (PVOID *)ppFuzzy1, 0L, lTokBufSize, ERROR_STORAGE );

  if ( fOK )
  {
    pTestToken = *ppFuzzy2;
    pT1Token = *ppFuzzy1;
    usStringA = usStringB = 0;
    while ((usStringA < usLenStr1) && (usStringB < usLenStr2) )
    {
      if ( ((pTokenList2+usStringB)->sType == MARK_EQUAL)
           && ((pTokenList1+usStringA)->sType == MARK_EQUAL)    )
      {
         *pTestToken = *(pTokenList2+usStringB);
         pTestToken++;
         lTokBufUsed++;
         *pT1Token = *(pTokenList1+usStringA);
         pT1Token++;

         usStringA++; usStringB++;
      }
      else if ((pTokenList1+usStringA)->sType == MARK_EQUAL)
      {
        *pTestToken = *(pTokenList2+usStringB);
        pTestToken->sType = MARK_INSERTED;
        pTestToken++;
        lTokBufUsed++;
        *pT1Token = *(pTokenList2+usStringB);
        pT1Token->sType = MARK_DELETED;
        pT1Token++;

        usStringB++;
      }
      else if ((pTokenList2+usStringB)->sType == MARK_EQUAL)
      {
        *pTestToken = *(pTokenList1+usStringA);
        pTestToken->sType = MARK_DELETED;
        pTestToken++;
        lTokBufUsed++;
        *pT1Token = *(pTokenList1+usStringA);
        pT1Token->sType = MARK_INSERTED;
        pT1Token++;

        usStringA++;
      }
      else
      {
        *pTestToken = *(pTokenList2+usStringB);
        pTestToken->sType = MARK_MODIFIED;
        pTestToken++;
        lTokBufUsed++;
        *pT1Token = *(pTokenList1+usStringA);
        pT1Token->sType = MARK_MODIFIED;
        pT1Token++;

        usStringA++;
        usStringB++;
      } /* endif */
    } /* endwhile */

    while ( usStringA < usLenStr1   )
    {
      *pTestToken = *(pTokenList1+usStringA);
      pTestToken->sType = MARK_DELETED;
      pTestToken++;
      lTokBufUsed++;
      *pT1Token = *(pTokenList1+usStringA);
      pT1Token->sType = MARK_INSERTED;
      pT1Token++;

      usStringA++;
    } /* endwhile */
    while ( usStringB < usLenStr2   )
    {
      *pTestToken = *(pTokenList2+usStringB);
      pTestToken->sType = MARK_INSERTED;
      pTestToken++;
      lTokBufUsed++;

      *pT1Token = *(pTokenList2+usStringB);
      pT1Token->sType = MARK_DELETED;
      pT1Token++;
      usStringB++;
    } /* endwhile */
  } /* endif */

  return( fOK );
} /* end of function EQFBMarkModDelIns */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FindMiddleSnake                                          |
//+----------------------------------------------------------------------------+
//|Function call:     FindMiddleSnake( pTokenList1,pBackList1,                 |
//|                                    pTOkenList2,pBackList2,                 |
//|                                    sLenStr1,sLenStr2,                      |
//|                                    &Midsnake, fCompareAll)                 |
//+----------------------------------------------------------------------------+
//|Description:       find middle snake according E.W.Myers,                   |
//|                          using      D-band                                 |
//|                   ('An O(ND) difference algorithm and its variations' by   |
//|                    E.W.Myers)                                              |
//+----------------------------------------------------------------------------+
//|Parameters:        PFUZZYTOK pTokenList1,        forward tokenlist of str 1 |
//|                   PFUZZYTOK pBackList1,         backward tokenlist of str1 |
//|                   PFUZZYTOK pTokenList2,        forward tokenlist of str 2 |
//|                   PFUZZYTOK pBackList2,         backward tokenlist of str2 |
//|                   SHORT    sLenStr1,            number of tokens in str 1  |
//|                   SHORT    sLenStr2,            number of tokens in str2   |
//|                   PSNAKEPTS pMidSnake           resulting middlesnake      |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       True if at least 2 tokens match                          |
//|                   FALSE if not token match at all                          |
//+----------------------------------------------------------------------------+
//|Side effects:      MidSnake is filled with start (x,y) and stop (u,v) of    |
//|                   middle snake                                             |
//+----------------------------------------------------------------------------+
//|Function flow:     if both strings have only one token                      |
//|                     if data are equal                                      |
//|                       set Midsnake, set fFound                             |
//|                     else                                                   |
//|                       set length of shortest edit script to 2              |
//|                     endif                                                  |
//|                   else                                                     |
//|                     allocate space for forward and backward D-band         |
//|                     init with -1                                           |
//|                     force that string2 is the longer one                   |
//|                     init for compare loop D, sDelta, psForward, psBackward |
//|                     do                                                     |
//|                      find forward D-path                                   |
//|                      if forward and backward path overlap                  |
//|                         set overlapping forward path to be the middlesnake |
//|                      find backward D-path                                  |
//|                      if forward and backward path overlap                  |
//|                         set overlapping backword path to be the middlesnake|
//|                     while middlesnake not found and                        |
//|                           not already at other end of range                |
//|                     reset longer/shorter string to original setting        |
//|                     free allocated space                                   |
//|                   endif                                                    |
//|                   return length of shortest edit script                    |
//+----------------------------------------------------------------------------+

static SHORT
FindMiddleSnake
(
  PFUZZYTOK pTokenList1,
  PFUZZYTOK pBackList1,
  PFUZZYTOK pTokenList2,
  PFUZZYTOK pBackList2,
  SHORT    sLenStr1,
  SHORT    sLenStr2,
  PSNAKEPTS pMidSnake,                  // resulting middle snake
  BOOL      fCompareAll
)
{
  PSHORT       psForward;            // best path
  PSHORT       psBackward;           // best path
  SHORT        sDelta;                 // difference between two items
  SHORT        sD;                     // index
  SHORT        sK;                      // index
  BOOL         fFound= FALSE;
  PFUZZYTOK    pTestToken;
  BOOL         fOK;
  BOOL         fChanged = FALSE;
  SHORT        sY;                     // y edge of start of snake
  BOOL         fDeltaIsOdd;            // true if Delta odd,false if even
  SHORT        sSESLen = 0;
  SHORT        sMax;
  SHORT        sLag;

  /********************************************************************/
  /* separate handling if sLenStr1 = sLenStr2 = 1                     */
  /********************************************************************/
  if ( (sLenStr1 == 1) && (sLenStr2 == 1 ) )
  {
    if ( (pTokenList1[0].ulHash == pTokenList2[0].ulHash) &&
        ( pTokenList1[0].pData[0] == pTokenList2[0].pData[0] ) &&
         ( CompFuzzyTok( &(pTokenList1[0]), &(pTokenList2[0]), fCompareAll) == 0))
    {

      fFound = TRUE;
      pMidSnake->sX = 0;
      pMidSnake->sY = 0;
      pMidSnake->sV = 1;
      pMidSnake->sU = 1;
    }
    else
    {
      sSESLen = 2;
    } /* endif */
  }
  else
  {
    /********************************************************************/
    /* allocate enough space to hold rows of matrix pointers            */
    /********************************************************************/
    sMax = sLenStr1 + sLenStr2;
    if ( sMax < 0 )
    {
      sMax = 0;
    } /* endif */
    sDelta = sMax +7;
    sMax = (sMax / 2) ;
    sLag = sMax+3;
    fOK = UtlAlloc( (PVOID *)&psForward,
                    0L,
                    (LONG) 2 * sDelta * sizeof(SHORT),
                    ERROR_STORAGE );

    psBackward = psForward + sDelta ;

    if ( fOK )
    {
      /******************************************************************/
      /* put it in a sequence that the string named with '1' is the     */
      /* shorter one, this requires our algorithm                       */
      /******************************************************************/
      if (sLenStr2 < sLenStr1 )
      {
        pTestToken = pTokenList2;
        pTokenList2 = pTokenList1;
        pTokenList1 = pTestToken;
        pTestToken = pBackList2;
        pBackList2 = pBackList1;
        pBackList1 = pTestToken;
        sK = sLenStr2;
        sLenStr2 = sLenStr1;
        sLenStr1 = sK;
        fChanged = TRUE;
      } /* endif */
      memset( psForward, -1, sDelta * sizeof(SHORT));
      memset( psBackward, -1, sDelta * sizeof(SHORT));
      sDelta = sLenStr2 - sLenStr1;
      if ( sDelta & 0x01 )
      {
        fDeltaIsOdd = TRUE;
      }
      else
      {
        fDeltaIsOdd = FALSE;
      } /* endif */

      sD = -1;
      psForward [1+sLag] = 0;
      psBackward [1+sLag] = 0;
      /****************************************************************/
      /* start divide&conquer                                         */
      /****************************************************************/
      while ( !fFound && sD <= sMax )
      {
        sD++;
        sK = -sD -2;
        /**************************************************************/
        /* find forward path                                          */
        /**************************************************************/
        while ( !fFound && sK <= (sD - 2) )
        {
          sK += 2;
          if ( sK == -sD )
          {
            sY = psForward[sK+1+sLag];
          }
          else if (sK == sD )
          {
            sY = psForward[sK-1+sLag]+1;
          }
          else if (psForward[sK-1+sLag] < psForward[sK+1+sLag])
          {
            sY = psForward[sK+1+sLag];
          }
          else
          {
            sY = psForward[sK-1+sLag]+1;
          } /* endif */
          psForward[sK+sLag] = Snake(pTokenList1, pTokenList2,
                                     sLenStr1, sLenStr2, sK, sY , fCompareAll);
          if ( fDeltaIsOdd &&
                ( (1 - sD ) <= (sDelta - sK )) &&
                ((sDelta - sK ) <= (sD - 1) ) )
          {
            if  (psForward[sK+sLag] >=
                 sLenStr2 - psBackward[ sDelta - sK +sLag] )
            {
              sSESLen = (2 * sD) - 1;
              fFound = TRUE;
              if ( (sD == 1) && (sY == psForward[sK+sLag])
                   && (sY == sLenStr2) )
              {
                //last token of longer string doesn't match
                pMidSnake->sX = 0;
                pMidSnake->sY = 0;
                pMidSnake->sV = sLenStr2 - 1;
                pMidSnake->sU = sLenStr1;
              }
              else
              {
                //set middlesnake, stop loop
                pMidSnake->sX = sY - sK;
                pMidSnake->sY = sY;
                pMidSnake->sV = psForward[sK+sLag];
                pMidSnake->sU = (pMidSnake->sV ) - sK;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endwhile */
        /**************************************************************/
        /* calculate backward D-path                                  */
        /**************************************************************/
        sK = -sD -2;
        while ( !fFound && sK <=(sD-2) )
        {
          sK += 2;
          if ( sK == -sD )
          {
            sY = psBackward[sK+1+sLag];
          }
          else if (sK == sD )
          {
            sY = psBackward[sK-1+sLag]+1;
          }
          else if (psBackward[sK-1+sLag] < psBackward[sK+1+sLag])
          {
            sY = psBackward[sK+1+sLag];
          }
          else
          {
            sY = psBackward[sK-1+sLag]+1;
          } /* endif */
          psBackward[sK+sLag] = Snake(pBackList1, pBackList2,
                                     sLenStr1, sLenStr2, sK, sY , fCompareAll);
          if ( !fDeltaIsOdd &&
               ( -sD <= (sDelta -sK) ) &&
               ((sDelta -sK) <= sD)   )
          {
            if ( psForward[sDelta- sK +sLag] >=
                 sLenStr2 - psBackward[sK +sLag])
            {
              sSESLen = 2 * sD;
              //set middlesnake, stop loop
              pMidSnake->sV = sLenStr2 - sY ;
              pMidSnake->sU = (pMidSnake->sV) + sK - sDelta;
              pMidSnake->sY = sLenStr2 - psBackward[sK+sLag];
              pMidSnake->sX = (pMidSnake->sY) + sK - sDelta;
              fFound = TRUE;
            } /* endif */
          } /* endif */
        } /* endwhile */

      } /* endwhile */

      if ( fChanged )
      {
        /******************************************************************/
        /* interchange x with y and u with v                              */
        /******************************************************************/
        sK = pMidSnake->sX;
        pMidSnake->sX = pMidSnake->sY;
        pMidSnake->sY = sK;
        sK = pMidSnake->sU;
        pMidSnake->sU = pMidSnake->sV;
        pMidSnake->sV = sK;
        /******************************************************************/
        /* reset input variables to original                              */
        /******************************************************************/
        pTestToken = pTokenList2;
        pTokenList2 = pTokenList1;
        pTokenList1 = pTestToken;
        pTestToken = pBackList2;
        pBackList2 = pBackList1;
        pBackList1 = pTestToken;
        sK = sLenStr2;
        sLenStr2 = sLenStr1;
        sLenStr1 = sK;
      } /* endif */
    } /* endif */
    UtlAlloc( (PVOID *)&psForward, 0L, 0L, NOMSG );
  } /* endif */

  return ( sSESLen );
} /* end of function FindMiddleSnake */


static SHORT
CompFuzzyTok
(
  PFUZZYTOK  pFuzzy1,
  PFUZZYTOK  pFuzzy2,
  BOOL       fCompareAll
)
{
  USHORT  usTokenLen = 0;
  USHORT  usToken2Len= 0;
  SHORT   sRc = 0;

    if (fCompareAll )
    {
      usTokenLen = pFuzzy1->usStop - pFuzzy1->usStart + 1;
      usToken2Len = pFuzzy2->usStop - pFuzzy2->usStart + 1;
      if (usToken2Len > usTokenLen )
      {
        usTokenLen = usToken2Len;
      } /* endif */
      sRc = (SHORT)UTF16strncmp(pFuzzy1->pData,
                         pFuzzy2->pData,
                         usTokenLen);
    }
    else
    {
      sRc = TokStrCompare( pFuzzy1,pFuzzy2 );
    } /* endif */
    return(sRc);
 } /* end of CompFuzzyTOk */


// helper function to compare two UTF16 strings while ignoring multiple whitespaces
static long CompareTokenWS( PSZ_W pTok1, USHORT len1, PSZ_W pTok2, USHORT len2 )
{
  LONG lRc = 0;

  while ( (len1 != 0) && (len2 != 0) )
  {
    CHAR_W c = *pTok1; 
    CHAR_W d = *pTok2;

    if ( UtlIsWhiteSpaceW(c) && UtlIsWhiteSpaceW(d) )
    {
      pTok1++; len1--;
      pTok2++; len2--;

      // skip any consecutive white spaces 
      while ( UtlIsWhiteSpaceW( *pTok1 ) && (len1 != 0) ) { pTok1++; len1--; } /* endwhile */
      while ( UtlIsWhiteSpaceW( *pTok2 ) && (len2 != 0) ) { pTok2++; len2--; } /* endwhile */
    }
    else if ( c == d )
    {
      pTok1++; len1--;
      pTok2++; len2--;
    }
    else
    {
      len1 = len2 = 0;
      lRc = ( c-d );
    } /* endif */
  } /* endwhile */

  // check if both strings have been processed completely
  if ( !lRc && (  (len1 != 0) || (len2 !=0) ) )
  {
    if ( len1 != 0 )
    {
      lRc = 1;
    }
    else if ( len2 != 0 )
    {
      lRc = -1;
    } /* endif */
  } /* endif */

  return lRc;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     Snake                                                    |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       find the longest diagonal sequence, i.e. SNAKE thru      |
//|                   a matrix ....                                            |
//|                   This routine will allow a O(n+m) behaviour of finding    |
//|                   the longest sequences two string have in common..        |
//+----------------------------------------------------------------------------+
//|Parameters:        PULONG     pulStr1  ptr to first string's hash values    |
//|                   PULONG     pulStr2  ptr to second string's hash values   |
//|                   USHORT     usLen1   length of the first string           |
//|                   USHORT     usLen2   length of the second string          |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       length of the identical sequence                         |
//+----------------------------------------------------------------------------+
//|Function flow:     loop through the diagonal as long as the characters of   |
//|                   both strings are identical.                              |
//|                   Return the compounded value                              |
//+----------------------------------------------------------------------------+

static SHORT
Snake
(
  PFUZZYTOK pFuzzyStr1,
  PFUZZYTOK pFuzzyStr2,
  SHORT     sLen1,
  SHORT     sLen2,
  SHORT     sK,
  SHORT     sY,
  BOOL      fCompareAll
)
{
  SHORT  sX;
  LONG   lRc = 0;
  USHORT usTokenLen = 0;
  USHORT usToken2Len = 0;

  sY--;                                       /***********************/
  sLen1--;                                    /* we start at zero    */
  sLen2--;                                    /***********************/
  /********************************************************************/
  /* loop through the diagonal as long as the characters are identical*/
  /********************************************************************/
  sX = sY - sK;
  while ( (sX < sLen1) && (sY < sLen2) &&
               (pFuzzyStr1[sX+1].ulHash == pFuzzyStr2[sY+1].ulHash) &&
               ( lRc == 0) )
  {
    if (fCompareAll )
    {
      usTokenLen = pFuzzyStr1[sX+1].usStop - pFuzzyStr1[sX+1].usStart + 1;
      usToken2Len = pFuzzyStr2[sY+1].usStop - pFuzzyStr2[sY+1].usStart + 1;
      //if (usToken2Len > usTokenLen )
      //{
      //  usTokenLen = usToken2Len;
      //} /* endif */
      //lRc = UTF16strncmp(pFuzzyStr1[sX+1].pData,
      //                   pFuzzyStr2[sY+1].pData,
      //                   usTokenLen);
      lRc = CompareTokenWS( pFuzzyStr1[sX+1].pData, usTokenLen, pFuzzyStr2[sY+1].pData, usToken2Len );
    }
    else
    {
      lRc = TokStrCompare(&(pFuzzyStr1[sX+1]),&(pFuzzyStr2[sY+1]) );
    } /* endif */
    if ( lRc == 0 )
    {
      sX ++;
      sY ++;
    } /* endif */
  } /* endwhile */

  sY++;
/**********************************************************************/
/* return the y value of the last match                               */
/* if nothing eq, return unchanged sY;                                */
/* if all equal, sY = usLen2                                          */
/**********************************************************************/
  return ( sY );
} /* end of function Snake */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LCS                                                      |
//+----------------------------------------------------------------------------+
//|Function call:     LCS(LCSStringA,LCSStringB)                               |
//+----------------------------------------------------------------------------+
//|Description:       divide &conquer algorithm to find shortest edit script   |
//|                   (by Hirschberg )                                         |
//+----------------------------------------------------------------------------+
//|Parameters:        LCSTOKEN LCSStringA                                      |
//|                   LCSTOKEN LCSStringB                                      |
//|                   BOOL     fCompareAll                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   void                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      forward and backward tokenlist allocated and filled      |
//+----------------------------------------------------------------------------+
//|Side effects:      'equal' token are marked with type 'MARK-EQUAL' in       |
//|                   forward tokenlist                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     get number of tokens in A string and Bstring             |
//|                   if both lengths are greater 0                            |
//|                      Find Middle snake                                     |
//|                      if something equal                                    |
//|                         calculate absolute start/stop of Middle snake      |
//|                         set LCS strings for lower half of range            |
//|                         call LCS for this range                            |
//|                         set mark-equals for middlesnake                    |
//|                         set LCS strings for upper half of range            |
//|                         call LCS for this range                            |
//|                      endif                                                 |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

static VOID
LCS
(
 LCSTOKEN    LCSStringA,
 LCSTOKEN    LCSStringB,
 BOOL        fCompareAll
)
{
  SHORT     sALen;                             // # of tokens in A string
  SHORT     sBLen;                             // # of tokens in B string
  SNAKEPTS  MidSnake;
  SHORT     sI;                                // index in for loop
  SHORT     sJ;
  LCSTOKEN  LCSNextA;
  LCSTOKEN  LCSNextB;
  SHORT     sSESLen;

  sALen = LCSStringA.sStop - LCSStringA.sStart ;
  sBLen = LCSStringB.sStop - LCSStringB.sStart ;
  if ( (sALen > 0) && ( sBLen > 0) )
  {
    sSESLen = FindMiddleSnake (
            LCSStringA.pTokenList+LCSStringA.sStart,
            LCSStringA.pBackList+LCSStringA.sTotalLen-LCSStringA.sStop,
            LCSStringB.pTokenList+LCSStringB.sStart,
            LCSStringB.pBackList+LCSStringB.sTotalLen-LCSStringB.sStop,
            sALen, sBLen, &MidSnake, fCompareAll);
    /*************************************************************/
    /* if not all tokens are different...                        */
    /*************************************************************/
    if ( sSESLen < (sALen + sBLen) )
    {
      /***************************************************************/
      /* correct MidSnake by the starting lags                       */
      /***************************************************************/
      MidSnake.sX = MidSnake.sX + LCSStringA.sStart;
      MidSnake.sU = MidSnake.sU + LCSStringA.sStart;
      MidSnake.sY = MidSnake.sY + LCSStringB.sStart;
      MidSnake.sV = MidSnake.sV + LCSStringB.sStart;
      /***********************************************************/
      /* divide again                                            */
      /***********************************************************/
      LCSNextA = LCSStringA;
      if ( MidSnake.sX < LCSNextA.sStop )
      {
        LCSNextA.sStop = MidSnake.sX;
      } /* endif */
      LCSNextB = LCSStringB;
      if ( MidSnake.sY < LCSNextB.sStop )
      {
        LCSNextB.sStop = MidSnake.sY;
      } /* endif */

      LCS( LCSNextA, LCSNextB, fCompareAll);
      /***************************************************************/
      /* set the mark-equals                                         */
      /***************************************************************/
      sJ = (MidSnake.sY);
      sI = (MidSnake.sX);
      while ( (sI < MidSnake.sU) && (sJ < MidSnake.sV)  )
      {
        (LCSStringA.pTokenList + sI)->sType = MARK_EQUAL;
        (LCSStringB.pTokenList + sJ)->sType = MARK_EQUAL;
        sJ++;
        sI++;
      } /* endwhile */

      LCSNextA = LCSStringA;
      if ( MidSnake.sU > LCSNextA.sStart )
      {
        LCSNextA.sStart = MidSnake.sU;
      } /* endif */
      LCSNextB = LCSStringB;
      if ( MidSnake.sV > LCSNextB.sStart )
      {
        LCSNextB.sStart = MidSnake.sV;
      } /* endif */

      LCS( LCSNextA, LCSNextB, fCompareAll);
    } /* endif */
  } /* endif */

} /* end of function LCS */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FuzzyLCSReplList                                         |
//+----------------------------------------------------------------------------+
//|Function call:     FuzzyLCSReplList( PFUZZYTOK,PFUZZYTOK,PREPLLIST)         |
//+----------------------------------------------------------------------------+
//|Description:       Will return a list of possible tokens to be replaced     |
//|                   in a fuzzy match                                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PFUZZYTOK    pPropTokList,   pointer to token list       |
//|                   PFUZZYTOK    pTransTokList,  pointer to token list       |
//|                   PREPLLIST  * ppReplaceList   pointer to replace list     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       1 .. n  number of equal tokens                           |
//|                   0       error during allocation                          |
//+----------------------------------------------------------------------------+
//|Function flow:      set up pointer array                                    |
//|                    return success indicator                                |
//+----------------------------------------------------------------------------+

USHORT
FuzzyLCSReplList
(
  PFUZZYTOK    pTokList1,              // ptr to token list->TgtTok of ReplList
  PFUZZYTOK    pTokList2,              // ptr to token list->SrcTOk ofReplList
  PREPLLIST  * ppReplaceList,                    // pointer to replace list
  USHORT       usLenStr1,              // num of tokens in TokList1
  USHORT       usLenStr2,                        // num of tokens in TokList2
  PUSHORT      pusFuzzy,
  BOOL         fCompareAll
)
{
  PREPLLIST    pReplaceList = NULL;    // pointer to replace list
  BOOL         fOK = TRUE;             // success indicator
  USHORT       usMinEqual = 0;         // no of equal but not-connected tokens
  USHORT       usI = 0;
  USHORT       usStringA;
  USHORT       usStringB;

  /********************************************************************/
  /* call LCS and use TOkStrCompare so that tokens are compared       */
  /* without regard of punctuation                                    */
  /********************************************************************/
  fOK = EQFBCallLCS(pTokList1, pTokList2, usLenStr1, usLenStr2, fCompareAll);

  /********************************************************************/
  /* count equal and different words; nether change pTokList1/2       */
  /* since it is needed later on too                                  */
  /********************************************************************/
  if (fOK && pusFuzzy )
  {
    fOK = EQFBTokCountDiff(pTokList1, pTokList2, usLenStr1, usLenStr2, pusFuzzy);
  } /* endif */
  /*****************************************************************/
  /* build ReplaceList                                             */
  /*****************************************************************/
  if (fOK )
  {
    if ( !pReplaceList )
    {
      if ( ! UtlAlloc( (PVOID *)ppReplaceList,
                      0L, (LONG)( (MAX_REPL+1) * sizeof(REPLLIST) ),
                      ERROR_STORAGE ) )
      { /************************************************************/
        /* error happened - reset pointer to stop loop              */
        /************************************************************/
        usI = MAX_REPL;
        fOK = FALSE;
      }
      else
      {
        pReplaceList = *ppReplaceList;
      } /* endif */
    } /* endif */

    usStringA = usStringB = 0;
    while ((usStringA < usLenStr1) && (usStringB < usLenStr2) )
    {
      if ( ((pTokList2+usStringB)->sType == MARK_EQUAL)
           && ((pTokList1+usStringA)->sType == MARK_EQUAL)    )
      {
         if ( usMinEqual < MAX_REPL-1 )
         {
           pReplaceList->pSrcTok = pTokList1 + usStringA;      // store values
           pReplaceList->pTgtTok = pTokList2 + usStringB;
           (pTokList1+usStringA)->fConnected = TRUE;
           (pTokList2+usStringB)->fConnected = TRUE;
           pReplaceList++;
           usMinEqual++;
         }
         else
         {
           usMinEqual++;
         } /* endif */

         usStringA++; usStringB++;
      }
      else if ((pTokList1+usStringA)->sType == MARK_EQUAL)
      {
        usStringB++;
      }
      else if ((pTokList2+usStringB)->sType == MARK_EQUAL)
      {
        usStringA++;
      }
      else
      {
        usStringA++;
        usStringB++;
      } /* endif */
    } /* endwhile */
  } /* endif */

  if ( !fOK )
  {
    usMinEqual = 0;
  } /* endif */                          // tokens
  return usMinEqual;
} /* end of function FuzzyLCSReplList */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBCallLCS                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBCallLCS     ( PFUZZYTOK,PFUZZYTOK,PREPLLIST)         |
//+----------------------------------------------------------------------------+
//|Description:       fill structure needed in LCS and call LCS                |
//+----------------------------------------------------------------------------+
//|Parameters:        PFUZZYTOK    pPropTokList,   pointer to token list       |
//|                   PFUZZYTOK    pTransTokList,  pointer to token list       |
//|                   PREPLLIST  * ppReplaceList   pointer to replace list     |
//|                   BOOL         fCompareAll     TRUE is strncmp nec.        |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       1       success                                          |
//|                   0       error                                            |
//+----------------------------------------------------------------------------+
//|Function flow:      set up pointer array                                    |
//|                    return success indicator                                |
//+----------------------------------------------------------------------------+
BOOL EQFBCallLCS
(
  PFUZZYTOK    pTokenList1,             // ptr to token list1
  PFUZZYTOK    pTokenList2,                      // ptr to token list2
  USHORT       usLenStr1,
  USHORT       usLenStr2,
  BOOL         fCompareAll
)
{
  BOOL         fOK = TRUE;             // success indicator
  PFUZZYTOK    pTestToken;
  LCSTOKEN     LCSString1;
  LCSTOKEN     LCSString2;
  PFUZZYTOK    pBackList1;
  PFUZZYTOK    pBackList2 = NULL;
  SHORT        k;


  fOK = UtlAlloc( (PVOID *)&pBackList1,
                  0L,
                  (LONG) (usLenStr1 +1) * sizeof(FUZZYTOK),
                  ERROR_STORAGE );
  if (fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pBackList2,
                    0L,
                    (LONG) (usLenStr2 +1) * sizeof(FUZZYTOK),
                    ERROR_STORAGE );
  } /* endif */
  if ( fOK )
  {
    /******************************************************************/
    /* fill backward tokenlists                                       */
    /******************************************************************/
    pTestToken = pTokenList1;
    for ( k=(usLenStr1-1); k >= 0 ; k-- )
    {
      *(pBackList1+k) = *pTestToken;
      pTestToken++;
    } /* endfor */

    pTestToken = pTokenList2;
    for ( k = (usLenStr2-1); k >= 0 ; k-- )
    {
      *(pBackList2+k) = *pTestToken;
      pTestToken++;
    } /* endfor */

    /******************************************************************/
    /* call recursive function to find shortest edit script           */
    /******************************************************************/
    LCSString1.pTokenList = pTokenList1;
    LCSString1.pBackList = pBackList1;
    LCSString1.sStart = 0;
    LCSString1.sStop = usLenStr1;
    LCSString1.sTotalLen = usLenStr1;

    LCSString2.pTokenList = pTokenList2;
    LCSString2.pBackList = pBackList2;
    LCSString2.sStart = 0;
    LCSString2.sStop = usLenStr2;
    LCSString2.sTotalLen = usLenStr2;

    LCS ( LCSString1,LCSString2, fCompareAll);

    UtlAlloc( (PVOID *)&pBackList1, 0L, 0L, NOMSG);
    UtlAlloc( (PVOID *)&pBackList2, 0L, 0L, NOMSG);

  } /* endif */
  return fOK;
} /* end of function EQFBCallLCS */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBTokCountDiff                                         |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBTokCountDiff( PFUZZYTOK,PFUZZYTOK,USHORT, USHORT     |
//|                                     PUSHORT)                               |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//+----------------------------------------------------------------------------+
//|Parameters:        PFUZZYTOK    pTokList1   ,   pointer to token list       |
//|                   PFUZZYTOK    pTokList2       pointer to token list       |
//|                   USHORT       usLenStr1                                   |
//|                   USHORT       usLenStr2                                   |
//|                   PUSHORT      pusFuzzy                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       1       success                                          |
//|                   0       error                                            |
//+----------------------------------------------------------------------------+
//|Function flow:      set up pointer array                                    |
//|                    return success indicator                                |
//+----------------------------------------------------------------------------+
static
BOOL EQFBTokCountDiff
(
  PFUZZYTOK    pTokenList1,             // ptr to token list1
  PFUZZYTOK    pTokenList2,                      // ptr to token list2
  USHORT       usLenStr1,
  USHORT       usLenStr2,
  PUSHORT      pusFuzzy
)
{
  PFUZZYTOK pFuzzy1 = NULL;                     // ret. tokenlist of Strin2
  PFUZZYTOK pFuzzy2 = NULL;                     // ret. tokenlist of string 1
  USHORT  usDiff = 0;                           // different words in String2
  USHORT  usWords = 0;                          // total words in String2
  BOOL    fOK;

  fOK = EQFBMarkModDelIns( pTokenList1, pTokenList2,
                           &pFuzzy1, &pFuzzy2,usLenStr1, usLenStr2);

  if (fOK )
  {
    /******************************************************************/
    /* usWords : total words in combined string                       */
    /* usDiff: # of words not MARK_EQUAL in combined string           */
    /******************************************************************/
    EQFBSimplifyAndCountMarks(pFuzzy2, &usDiff, &usWords);
  } /* endif */


  UtlAlloc ((PVOID *) &pFuzzy1, 0L, 0L, NOMSG);
  UtlAlloc ((PVOID *) &pFuzzy2, 0L, 0L, NOMSG);

  if (usDiff <= usWords )
  {
    *pusFuzzy = (usWords != 0) ? ((usWords - usDiff)*100 / usWords) : 100;
  }
  else
  {
    *pusFuzzy = 0;
  } /* endif */

  return (fOK);
} /* end of function EQFBTokCountDiff */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBSimplifyAndCountMarks                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBSimplifyAndCountMarks(PFUZZYTOK                      |
//|                                             PUSHORT,                       |
//|                                     PUSHORT)                               |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//+----------------------------------------------------------------------------+
//|Parameters:        PFUZZYTOK    pTokList1   ,   pointer to token list       |
//|                   PUSHORT      pusDiff,                                    |
//|                   PUSHORT      pusWords                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       1       success                                          |
//|                   0       error                                            |
//+----------------------------------------------------------------------------+
//|Function flow:      set up pointer array                                    |
//|                    return success indicator                                |
//+----------------------------------------------------------------------------+
static void
EQFBSimplifyAndCountMarks
(
  PFUZZYTOK    pFuzzy,                     // ptr to token
  PUSHORT      pusDiff,                    // # of different words
  PUSHORT      pusWords                    // total # of words
)
{
   USHORT     usDiff = 0;
   USHORT     usWords = 0;
   PFUZZYTOK  pTestToken;
   PFUZZYTOK  pToken;
   USHORT     usPrevType;

   /******************************************************************/
   /* browse through marks and simplifiy:                            */
   /* MODIFIED DELETED is changed to MODIFIED                        */
   /* MODIFIED INSERTED is changed to MODIFIED MODIFIED              */
   /******************************************************************/
   /*******************************************************************/
   /* count all words and count all words which are not equal         */
   /*******************************************************************/
   usDiff = 0;
   pTestToken = pFuzzy;               //resulting pointers
   pToken = pFuzzy;                   //current pointers
   usPrevType = MARK_EQUAL;

   while ( pToken->ulHash )
   {
     switch ( pToken->sType )
     {
       case MARK_DELETED:
         if ( usPrevType == MARK_MODIFIED )
         {
           while ( pToken->sType == MARK_DELETED )
           {
             pToken++;
             usDiff++;
           } /* endwhile */
         } /* endif */
         if ( usPrevType == MARK_DELETED )
         {
           while ( pToken->sType == MARK_DELETED )
           {
             pToken++;
             usDiff++;
           } /* endwhile */
         } /* endif */
         break;
       case MARK_INSERTED:
         if ( usPrevType == MARK_MODIFIED )
         {
           pToken->sType = MARK_MODIFIED;
         } /* endif */
         break;
       default :
         break;
     } /* endswitch */
     if ( (pToken->ulHash) && (pToken->sType != MARK_EQUAL ))
     {
       usDiff++;
     } /* endif */
     usWords++;
     *pTestToken = *pToken;
     usPrevType = pTestToken->sType;
     pTestToken++;
     pToken++;
   } /* endwhile */

   *pTestToken = *pToken;                  // copy end of array


 *pusDiff = usDiff;
 *pusWords = usWords;
  return ;
} /* end of function EQFBSimplifyAndCountMarks */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMCompareBetweenTokens                                |
//+----------------------------------------------------------------------------+
//|Function call:     NTMCompareBetweenTokens ( PSZ_W                          |
//|                                             PSZ_W,                         |
//|                                     ULONG,                                 |
//|                                     PSZ                                    |
//|                                     SHORT )                                |
//+----------------------------------------------------------------------------+
//|Description:    compare characters between tokens                           |
//|                USE THIS FUNCTION ONLY IF ALL TOKENS ARE EQUAL!!            |
//|                i.e. TMFuzzyness returns usFuzzy = 100% but                 |
//|                UtlCompIgnWhiteSpace on total strings returns NOT-EQUAL!    |
//|EXAMPLE:  String1: "<STRONG>This is a \x0A test</STRONG>"                   |
//|          String2: "<STRONG>\x0AThis is a test</STRONG>"                    |
//|          String1 and String2 must return fStringEqual = TRUE!!             |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ_W pD1,                                               |
//|                   PSZ_W  pD2,                                              |
//|                   ULONG  ulSrcCP,                                          |
//|                   PSZ    pszMarkup,                                        |
//|                   SHORT  sLangID                                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       1       success                                          |
//|                   0       error                                            |
//+----------------------------------------------------------------------------+
//|Function flow:      set up pointer array                                    |
//|                    return success indicator                                |
//+----------------------------------------------------------------------------+

BOOL
NTMCompareBetweenTokens
(
    PSZ_W pD1,
    PSZ_W pD2,
    PSZ   pszMarkup,
    SHORT sLangID,
    ULONG ulSrcCP,
    PBOOL  pfStringEqual
)
{
    PLOADEDTABLE pTable = NULL;           // ptr to loaded tag table
    BOOL         fOK = TRUE;              // success indicator
    PFUZZYTOK    pTokenList2 = NULL;      // pointer to token lists
    PFUZZYTOK    pTokenList1 = NULL;      // pointer to token lists
    PFUZZYTOK    pstTok1;                 // pointer to token lists
    PFUZZYTOK    pstTok2;                 // pointer to token lists
    PFUZZYTOK    pstNextTok1;
    PFUZZYTOK    pstNextTok2;
    SHORT        sNumTags = 0;
    BOOL         fStringEqual = TRUE;
    CHAR_W       chW;
    PBYTE        pInBuf = NULL;
    PBYTE        pTokBuf = NULL;
    USHORT       usI = 0;
    BOOL         fTokIsTag = FALSE;
    BOOL         fTok2IsTag = FALSE;
    BOOL         fNextTokIsTag = FALSE;
    ULONG	     ulLen1 = 0;
    ULONG        ulLen2 = 0;
    PSZ_W        pTemp = NULL;

  // allocate required buffers
  fOK = UtlAlloc((PVOID *)&pInBuf, 0L, 50000L, NOMSG );
  if ( fOK ) fOK = UtlAlloc((PVOID *)&pTokBuf, 0L, (LONG)TOK_BUFFER_SIZE, NOMSG );

  // load tag table
  if ( fOK )
  {
    fOK = (TALoadTagTableExHwnd( pszMarkup, &pTable, FALSE,
                                 TALOADUSEREXIT | TALOADPROTTABLEFUNC,
                                 FALSE, NULLHANDLE ) == NO_ERROR);
  } /* endif */
  if (fOK && (pTable->pfnProtTableW || pTable->pfnProtTable))
  {
  	  // protect-unprotect cannot be detected via TATagTokenize in PrepareTokens
  	  // this is currently for EQFRTF/EQFMSWRD
  	  // so in this case, comparison between tokens cannot be provided
  	  fOK = FALSE;
  }
  if ( fOK)
  {
     /******************************************************************/
     /* prepare tokens for String1 and string 2                        */
     /******************************************************************/
     fOK = PrepareTokens( pTable,
                       pInBuf,
                       pTokBuf,
                       pD1, sLangID, &pTokenList1, ulSrcCP );
  }
  if ( fOK)
  {
     fOK = PrepareTokens( pTable,
                       pInBuf,
                       pTokBuf,
                       pD2, sLangID, &pTokenList2, ulSrcCP );
  }
  // prereq is FUzzyness= 100%, i.e. all texttokens have been MARK_EQUAL in TMFuzzyness!!
  // check whether all tokens ( text + tag) are in both lists!
  // and tag tokens are equal
  if (fOK)
  {
    sNumTags = (SHORT) pTable->pTagTable->uNumTags;
    pstTok1 = pTokenList1;
    pstTok2 = pTokenList2;

    while (fStringEqual && pstTok1->ulHash && pstTok2->ulHash)
    {
       fTokIsTag = ((pstTok1->sType < sNumTags ) && (pstTok1->sType >= 0));
       fTok2IsTag = ((pstTok2->sType < sNumTags ) && (pstTok2->sType >= 0));
       if ( fTokIsTag && fTok2IsTag )
       {
           ulLen1 = pstTok1->usStop - pstTok1->usStart+1;
           ulLen2 = pstTok2->usStop - pstTok2->usStart+1;
           if (ulLen2 > ulLen1 )
           {
             ulLen1 = ulLen2;
           } /* endif */
           fStringEqual = ( UtlCompIgnWhiteSpaceW(pstTok1->pData, pstTok2->pData,
                                                ulLen1 ) == 0);
       }
       else if ( !fTokIsTag && !fTok2IsTag )
       {
           // both are text tokens, have been compared already in TMFuzzy!
       }
       else
       {
           fStringEqual = FALSE;  // TAG token and TEXT token nether match!
       }  /* endif */

       pstTok1++;
       pstTok2++;
    } /* endwhile */

    pTemp = NULL;
	if (fStringEqual && !pstTok1->ulHash && pstTok2->ulHash)
	{ // D1 is at end, but D2 has more tokens to come!
	  pTemp = pD2;
	  usI = pstTok2->usStart;
	}
	if (fStringEqual && !pstTok2->ulHash && pstTok1->ulHash)
	{
	  usI = pstTok1->usStart;
	  pTemp = pD1;
	}
	// but if token is whitespace only, String is equal!!
	if (fStringEqual && pTemp)
	{
	   chW = *(pTemp+usI);
	   while (fStringEqual && chW)
	   {
		  if ( !((chW == BLANK) || (chW == CR) || (chW == LF)) )
		  {
			  fStringEqual = FALSE;
		  }
		  else
		  {
			 usI ++;
		  }
		  chW = *(pTemp+usI);
	   } /* endwhile */
    }

    // Now we have to take care of the spaces between the texttokens and TagTokens!
    if ( fStringEqual )
    {
      pstTok1 = pTokenList1;
      pstTok2 = pTokenList2;
      pstNextTok1 = pstTok1 + 1;
      pstNextTok2 = pstTok2 + 1;
      while ( pstTok1->ulHash && pstNextTok1->ulHash &&
              pstTok2->ulHash && pstNextTok2->ulHash )
      {
        fTokIsTag = ((pstTok1->sType < sNumTags ) &&
                     (pstTok1->sType >= 0));
        fNextTokIsTag = ((pstNextTok1->sType < sNumTags ) &&
                         (pstNextTok1->sType >= 0));

        if ( fTokIsTag != fNextTokIsTag )
        {// now we have either  TAG - TEXT or TEXT - TAG !
          if ( (pstTok1->usStop == pstNextTok1->usStart - 1) && (pstTok2->usStop < pstNextTok2->usStart - 1) )
          {  // no space between TAG - TEXT or TEXT - TAG in String D1, but something in String D2!!
             // if it is a blank - CR - LF in D2 it is ok, else not
             usI = pstTok2->usStop + 1;
             while (fStringEqual && (usI < pstNextTok2->usStart))
             {
                 chW = *(pD2+usI);
                 if ( !((chW == BLANK) || (chW == CR) || (chW == LF)) )
                 {
                     fStringEqual = FALSE;
                 }
                 else
                 {
                     usI ++;
                 }
             } /* endwhile */

          }
          else if ( (pstTok1->usStop < pstNextTok1->usStart - 1)
                 && (pstTok2->usStop == pstNextTok2->usStart - 1) )
          {
             // no space between TAG - TEXT or TEXT - TAG in String D2, but something in String D1!!
             // if it is a blank - CR - LF in D1 it is ok, else not
             usI = pstTok1->usStop + 1;
             while (fStringEqual && (usI < pstNextTok1->usStart))
             {
                 chW = *(pD1+usI);
                 if ( !((chW == BLANK) || (chW == CR) || (chW == LF)) )
                 {
                     fStringEqual = FALSE;
                 }
                 else
                 {
                     usI ++;
                 }
             } /* endwhile */
          } /* endif */
        } /* endif */
        pstTok1 = pstNextTok1;
        pstTok2 = pstNextTok2;
        pstNextTok1 = pstTok1 + 1;
        pstNextTok2 = pstTok2 + 1;
      } /* endwhile */
    } /* endif */
   } /* endif if fOK */

  if (pTokenList1) UtlAlloc( (PVOID *)&pTokenList1, 0L, 0L, NOMSG );
  if (pTokenList2) UtlAlloc( (PVOID *)&pTokenList2, 0L, 0L, NOMSG );
  if ( pInBuf )  UtlAlloc( (PVOID *) &pInBuf, 0L, 0L, NOMSG );
  if ( pTokBuf ) UtlAlloc( (PVOID *) &pTokBuf, 0L, 0L, NOMSG );
  if ( pTable )  TAFreeTagTable( pTable);
  *pfStringEqual = fStringEqual;

  return(fOK );
} /* end of function NTMCompareBetweenTokens */
