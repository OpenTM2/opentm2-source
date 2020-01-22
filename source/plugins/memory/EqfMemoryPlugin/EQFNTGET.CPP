//+----------------------------------------------------------------------------+
//|EQFNTGET.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#include <eqf.h>                  // General Translation Manager include file

#define INCL_EQFMEM_DLGIDAS
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFTPI.H>               // Private header file of Standard Editor
#include <EQFMORPI.H>
#include <EQFEVENT.H>             // event logging
#include <EQFTADIT.H>             // IBMIDDOC->IBMDITA special processing functions
#include <OTMGLOBMem.h>         // Global Memory defines
#include "OtmProposal.h"
#include "core\pluginmanager\otmmarkup.h"
#include "MarkupPluginMapper.h"
#include "core\memory\MemoryFactory.h"
#include <map>

#ifdef _DEBUG
  //#define MEASURETIME
  //#define SGMLDITA_LOGGING
  //#define INLINE_TAG_REPL_LOGGING
  //#define _NTGET_LOG
  //#define MATCHLEVEL_LOGGING

  // write found matches to log file
  //#define MATCHLIST_LOG
#endif

// import logging 
#ifdef MEASURETIME
  static void GetElapsedTime( LONG64 *plTime );
#endif

#define ACTIVATE_NTMGenericDelete 

#ifdef _DEBUG
 #ifdef _NTGET_LOG
  FILE *hLog = NULLHANDLE;

  #define LOGOPEN( name ) \
    hLog = fopen( name, "ab" )

  #define LOGCLOSE() \
    if ( hLog != NULL ) fclose( hLog )

  #define LOGPRINTF0( format ) \
    if ( hLog != NULL ) fwprintf( hLog, format )

  #define LOGPRINTF1( format, v1 ) \
    if ( hLog != NULL ) fwprintf( hLog, format, v1 )

  #define LOGPRINTF2( format, v1, v2 ) \
    if ( hLog != NULL ) fwprintf( hLog, format, v1, v2 )

  #define LOGPRINTF3( format, v1, v2, v3 ) \
    if ( hLog != NULL ) fwprintf( hLog, format, v1, v2, v3 )

  #define LOGPRINTF4( format, v1, v2, v3, v4 ) \
    if ( hLog != NULL ) fwprintf( hLog, format, v1, v2, v3, v4 )
 #else
  #define LOGOPEN( a )
  #define LOGCLOSE()
  #define LOGPRINTF0( f )
  #define LOGPRINTF1( f, v1 )
  #define LOGPRINTF2( f, v1, v2 )
  #define LOGPRINTF3( f, v1, v2, v3 )
  #define LOGPRINTF4( f, v1, v2, v3, v4 )
 #endif
#else
  #define LOGOPEN( a )
  #define LOGCLOSE()
  #define LOGPRINTF0( f)
  #define LOGPRINTF1( f, v1 )
  #define LOGPRINTF2( f, v1, v2 )
  #define LOGPRINTF3( f, v1, v2, v3 )
  #define LOGPRINTF4( f, v1, v2, v3, v4 )
#endif

#ifdef MEASURETIME
  void LogWritePerfTime( FILE *hLog, PLARGE_INTEGER pliStart, PLARGE_INTEGER pliEnd, char *pszText );

  void LogWritePerfTime( FILE *hLog, PLARGE_INTEGER pliStart, PLARGE_INTEGER pliEnd, char *pszText ) 
  {
    LARGE_INTEGER liFrequency;

    QueryPerformanceFrequency( &liFrequency );

    if ( liFrequency.QuadPart != 0 )
    {
      LONGLONG ldwDiff = pliEnd->QuadPart - pliStart->QuadPart;
      DWORD dwTime = (DWORD)((ldwDiff * 1000) / liFrequency.QuadPart);
 
      if ( *pszText )
      {
        fprintf( hLog, "Time for %-20s : %ld ms (%I64d ticks)\n", pszText, dwTime, ldwDiff );
      }
      else
      {
        fprintf( hLog, "Total time                    : %ld ms (%I64d ticks)\n", dwTime, ldwDiff );
      } /* endif */
    }
  }
#endif

void NTMLogSegData( FILE *hfLog, PSZ_W pszForm, PSZ_W pszSegData );



USHORT FillMatchTable( PTMX_CLB, PSZ_W, PULONG, PTMX_TARGET_RECORD,
                PTMX_TARGET_CLB,
                PTMX_MATCH_TABLE_W, PUSHORT, BOOL, PUSHORT, PUSHORT, PUSHORT,
                ULONG, USHORT, PTMX_GET_W, PTMX_TAGTABLE_RECORD, USHORT, USHORT );

/**********************************************************************/
/* Prototypes for Static functions...                                 */
/**********************************************************************/
//For R012645 begin
static std::map<std::string,std::string> Name2FamilyMap;
static std::string& GetAndCacheFamilyName
(
  char*  pszMarkupName
);

//For R012645 end

static VOID
MakeHashValueW
(
  PULONG    pulRandom,                 // array of random numbers for hashing
  USHORT    usMaxRandom,               // maximum random numbers
  PSZ_W     pData,                     // ptr to data to be hashed
  PULONG    pulHashVal                 // resulting hash value
);


static
BOOL NTMPrepareTokens
(
  PTMX_SUBSTPROP       pSubstProp,
  PSZ_W                pData,
  PFUZZYTOK           *ppTokData,
  PUSHORT              pusTokens,
  PTMX_TAGTABLE_RECORD pTagRecord,
  SHORT                sLangID,
  ULONG                ulOemCP,
  PLOADEDTABLE         pTagTable
);

static BOOL
NTMFuzzyReplace
(
  PTMX_SUBSTPROP pSubstProp,
  PSZ_W     pSource,                   // source string
  PSZ_W     pProp,                     // proposal string
  PSZ_W     pTrans,                    // translation string
  PREPLLIST pReplPropSrc,              // list of same tokens in source and prop
  PREPLLIST pReplaceList               // list of tokens to be replaced
#ifdef INLINE_TAG_REPL_LOGGING
  , FILE *hfLog
#endif
);

#ifdef ACTIVATE_NTMGenericDelete 
static BOOL
NTMGenericDelete
(
  PTMX_SUBSTPROP pSubstProp,
  PSZ_W     pSource,                   // source string
  PSZ_W     pProp,                     // proposal string
  PSZ_W     pTrans,                    // translation string
  PREPLLIST pReplPropSrc,              // list of same tokens in source and prop
  PREPLLIST pReplaceList               // list of tokens to be replaced
#ifdef INLINE_TAG_REPL_LOGGING
  , FILE *hfLog
#endif
);
#endif

#ifdef INLINE_TAG_REPL_LOGGING
static BOOL NTMCheckTagPairs
(
  PTMX_SUBSTPROP pSubstProp,
  PREPLLIST pReplPropSrc,              // list of same tokens in source and prop
  PREPLLIST pReplaceList,               // list of tokens to be replaced
  BOOL      fRespectLFs,
  FILE      *hfLog
);
#else
static BOOL NTMCheckTagPairs
(
  PTMX_SUBSTPROP pSubstProp,
  PREPLLIST pReplPropSrc,              // list of same tokens in source and prop
  PREPLLIST pReplaceList,              // list of tokens to be replaced
  BOOL      fRespectLFs
);
#endif

static PTMX_REPLTAGPAIR
NTMFindTagPair
(
  PFUZZYTOK         pTempTok,
  PSZ_W             pTempTokData,
  PTMX_SUBSTPROP    pSubstProp,
  PTMX_REPLTAGPAIR  pCurTagPair,
  BOOL              fRespectLFs
);


static BOOL
NTMReplaceTags
(
  PTMX_SUBSTPROP pSubstProp,
  BOOL           fRespectLFs
);

static BOOL
NTMCopyTokData
(
   PSZ_W      pTempData,
   SHORT      sLen,
   PSZ_W *    ppNewData,
   PLONG      plNewLen
);


static PFUZZYTOK
NTMSplitAndAddTokens ( PTMX_SUBSTPROP,
                       PFUZZYTOK, PUSHORT, USHORT, SHORT,
                       USHORT, PSZ_W, SHORT, ULONG, PLOADEDTABLE );

USHORT NTMCompareContext
(
  PTMX_CLB pTmClb,                     // ptr to ctl block struct
  PSZ         pszMarkup,               // ptr to name markup used for segment
  PSZ_W       pszContext1,             // context of first segment
  PSZ_W       pszContext2              // context of second segment
);


//USHORT FillMatchEntryEx
//( 
//  PTMX_CLB pTmClb,
//  PTMX_SENTENCE pSentence,
//  PTMX_MATCHENTRY pMatchEntry,
//  PUSHORT pusMatchThreshold,
//  PTMX_INDEX_RECORD pIndexRecord 
//);


//typedef USHORT (APIENTRY *PFNCOMPCONTEXT)( PSZ_W, PSZ_W, PUSHORT );
typedef USHORT (__cdecl *PFNCOMPCONTEXT)( PSZ_W, PSZ_W, PUSHORT );


VOID SetAvailFlags( PTMX_GET_OUT_W pTmGetOut, USHORT usMatchesFound)
{
  // store indication if more matches are available
  if ( usMatchesFound > 1 )
  {
     USHORT   i;                                     // index value
     PTMX_MATCH_TABLE_W pstMT = &(pTmGetOut->stMatchTable[0]);

       // indicate if other exact matches are available
     pTmGetOut->fsAvailFlags |=
                   (pTmGetOut->stMatchTable[1].usMatchLevel >= EQUAL_EQUAL) ? GET_MORE_EXACTS_AVAIL : 0;
     // indicate if fuzzy matches are available
     for ( i=1; i < usMatchesFound ;i++ )
     {
       if ((pstMT+i)->usMatchLevel < EQUAL_EQUAL )
       {
         pTmGetOut->fsAvailFlags |= GET_ADDITIONAL_FUZZY_AVAIL;
         break;
       } /* endif */
     } /* endfor */
  }
}

// get match type 
OtmProposal::eMatchType GetMatchTypeFromMatchLevel( USHORT usMatchLevel )
{
  if ( usMatchLevel == 101 ) return( OtmProposal::emtExactSameDoc );; 
  if ( usMatchLevel == 100 ) return( OtmProposal::emtExact );
  if ( usMatchLevel == 102 ) return( OtmProposal::emtExactExact );
  return ( OtmProposal::emtFuzzy );
}

static OtmProposal::eProposalType GetProposalTypeFromFlag( USHORT usTranslationFlag )
{
  if ( usTranslationFlag == TRANSLFLAG_MACHINE ) return ( OtmProposal::eptMachine );
  if ( usTranslationFlag == TRANSLFLAG_GLOBMEM ) return ( OtmProposal::eptGlobalMemory );
  if ( usTranslationFlag == TRANSLFLAG_GLOBMEMSTAR ) return ( OtmProposal::eptGlobalMemoryStar );
  if ( usTranslationFlag == TRANSLFLAG_NORMAL ) return ( OtmProposal::eptManual );
  return ( OtmProposal::eptUndefined );
}


// utility to allocate pSentence and associated memory areas
USHORT NTMAllocSentenceStructure( PTMX_SENTENCE  *ppSentence );

// utility to free pSentence and associated memory areas
VOID NTMFreeSentenceStructure( PTMX_SENTENCE  pSentence );

void NTMFreeSubstProp( PTMX_SUBSTPROP pSubstProp );
void NTMMakeTagTablename( PSZ pszMarkup, PSZ pszTagTableName );

BOOL NTMAlignTags( PFUZZYTOK pTokSource, PFUZZYTOK pTokTarget, PREPLLIST *ppReplaceList );

static BOOL NTMCheckAndDeleteTagPairs
(
  PTMX_SUBSTPROP pSubstProp,
  PREPLLIST pReplaceSourceList,         // list of same tokens in source and prop
  PREPLLIST pReplaceList,              // toklist to be replaced in prop src+tgt
#ifdef INLINE_TAG_REPL_LOGGING
  BOOL      fRespectLFs,
  FILE      *hfLog                     // log file handle
#else
  BOOL      fRespectLFs
#endif
);


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXGet      gets data to the tm database                |
//+----------------------------------------------------------------------------+
//|Description:       Gets data from tm data file - exact matches and fuzzies  |
//+----------------------------------------------------------------------------+
//|Function call:  TmtXGet( PTMX_CLB pTmClb,        //ptr to ctl block struct  |
//|                         PTMX_GET_IN pTmGetIn,   //ptr to input struct      |
//|                         PTMX_GET_OUT pTmGetOut ) //ptr to output struct    |
//+----------------------------------------------------------------------------+
//|Input parameter:   PTMX_CLB  pTmClb         control block                   |
//|                   PTMX_GET_IN pTmGetIn     input structure                 |
//+----------------------------------------------------------------------------+
//|Output parameter:  PTMX_GET_OUT pTmGetOut   output structure                |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|     get id of source language, call                                        |
//|       NTMGetIdFromName, returns pointer to id                              |
//|                                                                            |
//|     using parameters from the Tm_Get structure passed                      |
//|     call TokenizeSegment with szTagTable, szSource, SourceLangId,          |
//|       TagTable record, pNormString                                         |
//|                                                                            |
//|     call HashSentence with pNormString                                     |
//|                                                                            |
//|     work through                                                           |
//|       checking if on in compact area in TM control block                   |
//|       if all hashes on                                                     |
//|         if only exact matches requested                                    |
//|           assume exact match so apply exact match test                     |
//|           if no matches found                                              |
//|             aplly the fuzzy match test                                     |
//|         else                                                               |
//|           first apply the exact match test and then the fuzzy match test   |
//|       else                                                                 |
//|         assume fuzzy match so apply fuzzy match test                       |
//|                                                                            |
// ----------------------------------------------------------------------------+
USHORT TmtXGet
(
  PTMX_CLB pTmClb,         //ptr to ctl block struct
  PTMX_GET_IN_W pTmGetIn,    //ptr to input struct
  PTMX_GET_OUT_W pTmGetOut   //ptr to output struct
)
{
  PTMX_SENTENCE pSentence = NULL;      // ptr to sentence structure
  USHORT usRc = NO_ERROR;              // return code
  USHORT usOverlaps = 0;               // compact area triple hits
  CHAR szString[MAX_EQF_PATH];         // character string
#ifdef MATCHLIST_LOG
  FILE *hfMatchListLog = NULL;
#endif

  ULONG ulStrippedParm = pTmGetIn->stTmGet.ulParm &
    ~(GET_RESPECTCRLF | GET_IGNORE_PATH | GET_NO_GENERICREPLACE | GET_ALWAYS_WITH_TAGS | GET_IGNORE_COMMENT);

#ifdef MEASURETIME
  LONG64 lDummy = 0;
  pTmClb->fTimeLogging = TRUE;
  GetElapsedTime( &lDummy );
#endif

#ifdef MATCHLEVEL_LOGGING
  FILE *hfLog = NULL;
#endif

#ifdef MATCHLIST_LOG
  {
    BOOL fNew = FALSE;
    CHAR szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szLogFile, 0L, FALSE );
    strcat( szLogFile, BACKSLASH_STR );
    strcat( szLogFile, "MATCHLIST.LOG" );
    fNew = !UtlFileExist( szLogFile );
    hfMatchListLog = fopen( szLogFile, "ab" );
    if ( hfMatchListLog )
    {
      if ( fNew ) fwrite( UNICODEFILEPREFIX, 1, 2, hfMatchListLog ); 
      fwprintf( hfMatchListLog, L"== Lookup in memory %S ==\r\n", pTmClb->stTmSign.szName );
      NTMLogSegData( hfMatchListLog, L"  LookupSource=>>>%s<<<\r\n", pTmGetIn->stTmGet.szSource );
    } /* endif */       
  }
#endif

#ifdef _NTGET_LOG
  {
    char szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath ( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szLogFile, 0L, FALSE );
    strcat( szLogFile, "\\TMTXGET.LOG" );
    
    LOGOPEN( szLogFile );
  }
#endif

  LOGPRINTF1( L"--------------\r\nLooking up:\r\n%s\r\n", pTmGetIn->stTmGet.szSource );

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lOtherTime) );
#endif

  //allocate pSentence
  usRc = NTMAllocSentenceStructure( &pSentence ); 

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lAllocTime) );
#endif

  if ( !usRc )
  {
    //build tag table path
    UtlMakeEQFPath( szString, NULC, TABLE_PATH, NULL );
    strcat( szString, BACKSLASH_STR );
    strcat( szString, pTmGetIn->stTmGet.szTagTable );
    strcat( szString, EXT_OF_FORMAT );

    //remember start of norm string
    pSentence->pNormStringStart = pSentence->pNormString;

    UTF16strcpy( pSentence->pInputString, pTmGetIn->stTmGet.szSource );
    //tokenize source segment, resuting in normalized string and
    //tag table record
    usRc = TokenizeSource( pTmClb, pSentence, szString, pTmGetIn->stTmGet.szSourceLanguage, (USHORT)pTmClb->stTmSign.bMajorVersion );

    // set the tag table ID in the tag record (this can't be done in TokenizeSource anymore)
    if ( usRc == NO_ERROR )
    {
      if ( pTmClb )
      {
        usRc = NTMGetIDFromName( pTmClb, pTmGetIn->stTmGet.szTagTable, NULL, (USHORT)TAGTABLE_KEY, &pSentence->pTagRecord->usTagTableId );
      }
      else
      {
        pSentence->pTagRecord->usTagTableId = 0;
      } /* endif */
    }
  } /* endif */

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lTokenizeTime) );
#endif

  if ( !usRc )
  {
    SHORT sRetries = MAX_RETRY_COUNT;
    do
    {

      usRc = NO_ERROR;                 // reset return code

      // Update internal buffers if database has been modified by other users
      if ( !usRc && pTmClb->fShared )
      {
        usRc = NTMCheckForUpdates( pTmClb );
      } /* endif */


      if ( !usRc )
      {
          pSentence->pNormString = pSentence->pNormStringStart;
          HashSentence( pSentence, pTmClb->stTmSign.bMajorVersion, pTmClb->stTmSign.bMinorVersion );

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lOtherTime) );
#endif

          usOverlaps = CheckCompactArea( pSentence, pTmClb );
          LOGPRINTF1( L"TmtXGet: Checked compact area, usOverlaps=%u\r\n", usOverlaps );
          if ( usOverlaps == pSentence->usActVote ) //all hash triples found
          {
#ifdef MEASURETIME
            GetElapsedTime( &(pTmClb->lOtherTime) );
#endif
            if ( ulStrippedParm & GET_EXACT ) 
            {
              LOGPRINTF0( L"TmtXGet: Calling GetExactMatch\r\n" );
              //get exact matches only
              usRc = GetExactMatch( pTmClb, pSentence, &pTmGetIn->stTmGet,
                        pTmGetOut->stMatchTable, &pTmGetOut->usNumMatchesFound, pTmGetOut );

#ifdef MEASURETIME
              GetElapsedTime( &(pTmClb->lGetExactTime) );
#endif
              //if usNumMatchesFound is zero then try for fuzzies
              if ( (pTmGetOut->usNumMatchesFound == 0) && (usRc == NO_ERROR) )
              {
                LOGPRINTF0( L"TmtXGet: No exact matches found, trying GetFuzzyMatch\r\n" );
                usRc = GetFuzzyMatch( pTmClb, pSentence, &pTmGetIn->stTmGet,
                        pTmGetOut->stMatchTable, &pTmGetOut->usNumMatchesFound );
                LOGPRINTF1( L"TmtXGet: GetFuzzyMatch returned %u matches\r\n", pTmGetOut->usNumMatchesFound );
              } /* endif */
#ifdef MEASURETIME
              GetElapsedTime( &(pTmClb->lGetFuzzyTime) );
#endif
            }
            else
            {
                LOGPRINTF0( L"TmtXGet: Calling GetFuzzyMatch\r\n" );
                usRc = GetFuzzyMatch( pTmClb, pSentence, &pTmGetIn->stTmGet,
                        pTmGetOut->stMatchTable, &pTmGetOut->usNumMatchesFound );
                LOGPRINTF1( L"TmtXGet: GetFuzzyMatch returned %u matches\r\n", pTmGetOut->usNumMatchesFound );
#ifdef MEASURETIME
                GetElapsedTime( &(pTmClb->lGetFuzzyTime) );
#endif
            } /* endif */
          }
          else
          {
            BOOL fCheckForFuzzyMatches = FALSE;

            // check if the overlap is good enough...


            // GQ 2016/04/05: new approach: (tentative fix for P403177)
            //   sentences with up to 8 (and incl) votes: always check fuzzyness
            //   sentences from 9 up to 12 votes: check fuzziness when 20% of votes are matching
            //   sentences with 12 or more votes: check fuzziness when pTmGetIn->stTmGet.usMatchThreshold (=33%) of votes are matching

            // old code:
            //  if (( usOverlaps * 100 >= pSentence->usActVote * pTmGetIn->stTmGet.usMatchThreshold ) || (pSentence->usActVote <= 8))

            // GQ 2016/05/12: disabled new approach as it causes a 12% decrease in the overall analysis performance 
            if ( pSentence->usActVote <= 8 )
            {
              fCheckForFuzzyMatches = TRUE;
            }
            //else if ( pSentence->usActVote < 18 )
            //{
            //  fCheckForFuzzyMatches = (usOverlaps * 100) >= (pSentence->usActVote * 20);
            //}
            else 
            {
              fCheckForFuzzyMatches = (usOverlaps * 100) >= (pSentence->usActVote * pTmGetIn->stTmGet.usMatchThreshold);
            } /* endif */

            if ( fCheckForFuzzyMatches )
            {
              LOGPRINTF3( L"TmtXGet: usOverlaps=%u, usActVote=%u, usMatchThreshold=%u\r\n", usOverlaps, pSentence->usActVote, pTmGetIn->stTmGet.usMatchThreshold );

              //not all triples on in compact area so fuzzy match
#ifdef MEASURETIME
            GetElapsedTime( &(pTmClb->lOtherTime) );
#endif
              LOGPRINTF0( L"TmtXGet: Calling GetFuzzyMatch\r\n" );
              usRc = GetFuzzyMatch( pTmClb, pSentence, &pTmGetIn->stTmGet, pTmGetOut->stMatchTable, &pTmGetOut->usNumMatchesFound );
              LOGPRINTF1( L"TmtXGet: GetFuzzyMatch returned %u matches\r\n", pTmGetOut->usNumMatchesFound );
#ifdef MEASURETIME
              GetElapsedTime( &(pTmClb->lGetFuzzyTime) );
#endif
            } /* endif */
          } /* endif */
      } /* endif */

      if ( pTmClb->fShared && (usRc == BTREE_IN_USE) )
      {
        UtlWait( MAX_WAIT_TIME );
        sRetries--;
      } /* endif */
    } while( pTmClb->fShared && (usRc == BTREE_IN_USE) && (sRetries > 0)); /* enddo */
  } /* endif */

  // store indication if more matches are available
  if ( pTmGetOut->usNumMatchesFound > 1 )
  {
     SetAvailFlags( pTmGetOut, pTmGetOut->usNumMatchesFound);
  }

  // if we have more than one exact match check if we have an exact match where the source
  // matchs exactly the source segment data
  if ( !usRc && (pTmGetOut->usNumMatchesFound > 1 ) && (ulStrippedParm & GET_EXACT) )
  {
     PTMX_MATCH_TABLE_W pstMatchTable = &(pTmGetOut->stMatchTable[0]);
     int iNumExactSourceMatches = 0;   // number of matches having exact the same source
     int i;                            // index values
     int iIndexOfExactMatch = 0;       // index of exact match

     for ( i = 0; i < pTmGetOut->usNumMatchesFound; i++ )
     {
       PTMX_MATCH_TABLE_W pstActMatch = &(pTmGetOut->stMatchTable[i]);
       if ( (wcscmp( pstActMatch->szSource, pTmGetIn->stTmGet.szSource ) == 0) && (pTmGetOut->stMatchTable[i].usTranslationFlag == TRANSLFLAG_NORMAL) )
       {
         iNumExactSourceMatches++;
         iIndexOfExactMatch = i;
       } /* endif */
     } /* endfor */

     // if we have exactly one exact match which exact matches the source segment we can discard
     // the other exact matches 
     if ( iNumExactSourceMatches == 1 )
     {
       for ( i = 0; i < pTmGetOut->usNumMatchesFound; i++ )
       {
         if ( i != iIndexOfExactMatch )
         {
           // remove this match
           ULONG ulLen = ( MAX_MATCHES - i - 1 ) * sizeof(TMX_MATCH_TABLE_W);
           memmove( pstMatchTable+i, pstMatchTable+1+i, ulLen );
           pTmGetOut->usNumMatchesFound--;
           if ( i < iIndexOfExactMatch )
           {
             iIndexOfExactMatch--;  // adjust index of our exact match
           } /* endif */
           i--;                    // reset i because we deleted the i-th one
         } /* endif */
       } /* endfor */
     } /* endif */
  } /* endif */

  /********************************************************************/
  /* check if we have more than one equal tgt match in our proposals  */
  /********************************************************************/
  if ( !usRc && (pTmGetOut->usNumMatchesFound > 1 ) && (ulStrippedParm & GET_EXACT ))
  {
     PTMX_MATCH_TABLE_W pstMatchTable = &(pTmGetOut->stMatchTable[0]);
     PTMX_MATCH_TABLE_W pstActMatch;
     USHORT   i, j;                     // index values

     for ( j = 0 ; j < pTmGetOut->usNumMatchesFound - 1  ;j++ )
     {
       pstActMatch = &(pTmGetOut->stMatchTable[j]);
       for ( i=j+1; i< pTmGetOut->usNumMatchesFound ; i++)
       {
         if (UtlCompIgnWhiteSpaceW(pstActMatch->szTarget, (pstMatchTable+i)->szTarget, 0) == 0L)
         {
           /*************************************************************/
           /* delete this match...                                      */
           /*************************************************************/
           ULONG ulLen = ( MAX_MATCHES - i - 1 ) * sizeof(TMX_MATCH_TABLE_W);
           memmove( pstMatchTable+i, pstMatchTable+1+i, ulLen );
           pTmGetOut->usNumMatchesFound--;
           i--;                    // reset i because we deleted the i-th one
         } /* endif */
       } /* endfor */
     } /* endfor */
  } /* endif */



  /********************************************************************/
  /* restrict number of matches found to the maximum number requested */
  /* by user                                                          */
  /********************************************************************/
  pTmGetOut->usNumMatchesFound =
    min( pTmGetOut->usNumMatchesFound, pTmGetIn->stTmGet.usRequestedMatches );


  //release memory
  NTMFreeSentenceStructure( pSentence );

  { 
    int iTemp = sizeof( TMX_GET_OUT_W);
    pTmGetOut->stPrefixOut.usLengthOutput = (USHORT)(iTemp);
  }

  pTmGetOut->stPrefixOut.usTmtXRc = usRc;

  if ( usRc ) ERREVENT2( TMTXGET_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );

  LOGCLOSE();

  // release access to any tag table loaded for segment markup
  // (may have been loaded for context handling)
  if ( pTmClb->pTagTable )
  {
    TAFreeTagTable( (PLOADEDTABLE)(pTmClb->pTagTable) );
    pTmClb->pTagTable = NULL;
  } /* endif */

#ifdef MEASURETIME
            GetElapsedTime( &(pTmClb->lOtherTime) );
#endif

#ifdef MATCHLIST_LOG
  if ( hfMatchListLog )
  {
    int i = 0;

    fwprintf( hfMatchListLog, L"Lookup complete, found %u matches\r\n", pTmGetOut->usNumMatchesFound );

    for ( i = 0; i < pTmGetOut->usNumMatchesFound; i++ )
    {
      PTMX_MATCH_TABLE_W pMatch = &(pTmGetOut->stMatchTable[i]);
      fwprintf( hfMatchListLog, L"Match %ld, MatchLevel=%u%% MFlag=%u, Origin=%S, Segment=%lu\r\n", i, 
        pMatch->usMatchLevel, pMatch->usTranslationFlag,
        (pMatch->szLongName[0] != '0') ? pMatch->szLongName : pMatch->szFileName, pMatch->ulSegmentId );
      NTMLogSegData( hfMatchListLog, L"  Source = >>>%s<<<\r\n", pMatch->szSource );
      NTMLogSegData( hfMatchListLog, L"  Target = >>>%s<<<\r\n", pMatch->szTarget );
    } /* endfor */
    fclose( hfMatchListLog );
  } /* endif */       
#endif


  return( usRc );
}




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     GetExactMatch                                            |
//+----------------------------------------------------------------------------+
//|Description:       determines the degree of exactness of a match            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_CLB pTmClb                                          |
//|                   PTMX_SENTENCE pSentence                                  |
//|                   PTMX_GET pGetIn                                          |
//|                   PTMX_MATCH_TABLE pstMatchTable                           |
//|                   PUSHORT plMatchEntries                                  |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT plMatchEntries                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|               determine which tm record to extract                         |
//|               loop through the tm records                                  |
//|                 applying the exact match criteria                          |
// ----------------------------------------------------------------------------+

USHORT GetExactMatch
(
  PTMX_CLB pTmClb,                     //ptr to ctl block struct
  PTMX_SENTENCE pSentence,             //ptr to sentence structure
  PTMX_GET_W pGetIn,                   //ptr to data in get in structure
  PTMX_MATCH_TABLE_W pstMatchTable,    //get out output structure
  PUSHORT pusMatchesFound,             //number of matches found
  PTMX_GET_OUT_W pTmGetOut             // pointer to output struct
)
{
  BOOL fOK = TRUE;                   //success indicator
  PULONG pulSids = NULL;             //ptr to sentence ids
  PULONG pulSidStart = NULL;         //ptr to sentence ids
  USHORT usRc = NO_ERROR;            //return code
  ULONG ulLen;                       //length indicator
  ULONG ulKey;                       //tm record key
  PTMX_RECORD pTmRecord = NULL;      //pointer to tm record
  USHORT   usMatchEntries = 0;         //nr of found matches
  ULONG  ulRecBufSize = 0;             // current size of record buffer

  ULONG ulStrippedParm = pGetIn->ulParm &
    ~(GET_RESPECTCRLF | GET_IGNORE_PATH | GET_NO_GENERICREPLACE | GET_ALWAYS_WITH_TAGS | GET_IGNORE_COMMENT);

  //allocate 32K for tm record
  if ( pTmClb->pvTmRecord )
  {
    pTmRecord = (PTMX_RECORD)pTmClb->pvTmRecord; 
    ulRecBufSize = pTmClb->ulRecBufSize;
//    memset( pTmRecord, 0, ulRecBufSize );
  }
  else
  {
    fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );
    pTmClb->pvTmRecord = pTmRecord;
    if ( fOK ) pTmClb->ulRecBufSize = ulRecBufSize = TMX_REC_SIZE;
  } /* endif */


  //allocate for sentence ids
  if ( fOK ) fOK = UtlAlloc( (PVOID *) &(pulSids), 0L,
                             (LONG)((MAX_INDEX_LEN + 5) * sizeof(ULONG)),
                             NOMSG );

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    pulSidStart = pulSids;
    usRc = DetermineTmRecord( pTmClb, pSentence, pulSids );
    if ( usRc == NO_ERROR )
    {
      //get tm record(s)
      while ( (*pulSids) && (usRc == NO_ERROR) )
      {
        ulKey = *pulSids;
        ulLen = TMX_REC_SIZE;
        LOGPRINTF1( L"GetExactMatch: EQFNTMGET of record %lu\r\n", ulKey );
        usRc = EQFNTMGet( pTmClb->pstTmBtree, ulKey, (PCHAR)pTmRecord, &ulLen );

        if ( usRc == BTREE_BUFFER_SMALL)
        {
          fOK = UtlAlloc( (PVOID *)&pTmRecord, ulRecBufSize, ulLen, NOMSG );
          if ( fOK )
          {
            pTmClb->ulRecBufSize = ulRecBufSize = ulLen;
            pTmClb->pvTmRecord = pTmRecord;
            memset( pTmRecord, 0, ulLen );

            usRc = EQFNTMGet( pTmClb->pstTmBtree, ulKey, (PCHAR)pTmRecord,
                              &ulLen );
          }
          else
          {
            usRc = ERROR_NOT_ENOUGH_MEMORY;
          } /* endif */
        } /* endif */
        if ( usRc == NO_ERROR )
        {
          //compare tm record data with data passed in the get in structure
          LOGPRINTF1( L"GetExactMatch: ExactTest of record %lu\r\n", ulKey );
          usRc = ExactTest( pTmClb, pTmRecord, pGetIn, pSentence,
                            pstMatchTable, &usMatchEntries, ulKey );
          LOGPRINTF1( L"GetExactMatch: ExactTest found %u matching entries\r\n", usMatchEntries );
          if ( !usRc )
          {
            //nr of matches found
            (*pusMatchesFound) = usMatchEntries;

            //get next tm record
            pulSids++;
          } /* endif */
        } /* endif */
      } /* endwhile */
      /****************************************************************/
      /* check if we are only looking for an exact match and we really*/
      /* found more than one...                                       */
    /* store the availability of additional flags for later use     */
      /****************************************************************/
      SetAvailFlags( pTmGetOut, *pusMatchesFound);

      if ( !usRc && (*pusMatchesFound > 1 ) && (ulStrippedParm & GET_EXACT) )
      {
         /*************************************************************/
         /* there is one exact exact match available -  forget about  */
         /* any 'normal' matches                                      */
         /*************************************************************/
         if ( pstMatchTable->usMatchLevel >= EQUAL_EQUAL )
         {
           if ( pstMatchTable->usMatchLevel == (EQUAL_EQUAL + 2) )
           {
             /*********************************************************/
             /* we have an exact exact ...                            */
             /*********************************************************/
             *pusMatchesFound = 1;
           }
           else
           {
             /*********************************************************/
             /* we have two or more exact one's.                      */
             /*********************************************************/
             USHORT   i;                             // index value
             for ( i=1; i< *pusMatchesFound ;i++ )
             {
               if ((pstMatchTable+i)->usMatchLevel < EQUAL_EQUAL)
               {
                 *pusMatchesFound = i;
                 break;
               } /* endif */
             } /* endfor */
           } /* endif */
         } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pulSidStart, 0L, 0L, NOMSG );

  if ( usRc )
  {
    ERREVENT2( GETEXACTMATCH_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */

  return( usRc );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ExactTest                                                |
//+----------------------------------------------------------------------------+
//|Description:       Fills the match table structure as required by the       |
//|                   getout structure                                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_CLB pTmClb                                          |
//|                   PTMX_RECORD pTmRecord                                    |
//|                   PTMX_GET pGetIn                                          |
//|                   PTMX_SENTENCE pSentence                                  |
//|                   PTMX_MATCH_TABLE pstMatchTable                           |
//|                   PUSHORT pusMatchEntries                                  |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT pusMatchEntries                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|               if source strings are equal                                  |
//|               loop through the target records in the tm record             |
//|                 if target languages are equal                              |
//|                   if source tag table records are equal                    |
//|                     equal match so fill match table                        |
//|                   else                                                     |
//|                     if source tag ids are equal                            |
//|                       equal fuzzy match with tags so fill match table      |
//|                     else                                                   |
//|                       equal fuzzy match with no tags so fill match table   |
// ----------------------------------------------------------------------------+
USHORT ExactTest
(
  PTMX_CLB pTmClb,                  //ptr to ctl block struct
  PTMX_RECORD pTmRecord,            //pointer to tm record data
  PTMX_GET_W pGetIn,                //pointer to get in data
  PTMX_SENTENCE pSentence,          //pointer to sentence structure
  PTMX_MATCH_TABLE_W pstMatchTable, //get out structure to be filled
  PUSHORT pusMatchEntries,          //nr of entries in match entry structure
  ULONG   ulKeyNum                  // number of key record
)
{
  BOOL fOK;                            //success indicator
  PBYTE pByte;                         //position ptr
  PBYTE pStartTarget;                  //position ptr
  PTMX_SOURCE_RECORD pTMXSourceRecord; //ptr to source record
  PTMX_TARGET_RECORD pTMXTargetRecord; //ptr to target record
  PTMX_TARGET_CLB    pTMXTargetClb;    //ptr to target control block
  PTMX_TAGTABLE_RECORD pTMXSourceTagTable; //ptr to source tag info
  ULONG ulLen;                        //length indicator
  PSZ_W pString = NULL;                  //pointer to character string
  USHORT usRc = NO_ERROR;              //returned value from function
  USHORT usMatchLevel;                 //pointer to ushort value
  BOOL   fStringEqual = FALSE;         // indicator for string equal
  BOOL   fEqualFound = FALSE;          // tagging equal ??
  USHORT usEqual;
  PSZ_W pContextBuffer = NULL;        //pointer to buffer for context processing
  USHORT usTargetTranslationFlag = (USHORT)-1;    // translation flag of target CLB (processed)
  PSZ pszDocName = NULL;             // document name to check for equal document names in GET_IGNORE_PATH mode

  //allocate pString and buffer for context processing (when necessary)
  fOK = UtlAlloc( (PVOID *) &(pString), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
  if ( fOK )
  {
    if ( pGetIn->szContext[0] || (pGetIn->pvGMOptList != NULL) )
    {
      fOK = UtlAlloc( (PVOID *) &(pContextBuffer), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
    } /* endif */       
  
  } /* endif */     

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    //position at beginning of source structure in tm record
    pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);

    //move pointer to corresponding position
    pByte = (PBYTE)(pTmRecord+1);
    pByte += pTMXSourceRecord->usSource;

    //calculate length of source string
    ulLen = (RECLEN(pTMXSourceRecord) - sizeof(TMX_SOURCE_RECORD));

    //copy source string for later compare function
    ulLen = EQFCompress2Unicode( pString, pByte, ulLen );

    //compare source strings
    {
      if ( pGetIn->ulParm & GET_RESPECTCRLF )
      {  // if LF different, strings are NOT EQUAL in this case!!
        fStringEqual = (UtlCompIgnSpaceW(pString, pSentence->pNormString, 0 ) == 0L );
      }
      else
      {
        fStringEqual = (UtlCompIgnWhiteSpaceW(pString, pSentence->pNormString, 0 ) == 0L );
      } /* endif */
    }

    if ( fStringEqual )
    {
      USHORT  usTgtNum = 0;            // init target number
      ULONG   ulLeftTgtLen;            // remaining target length
      USHORT  usGetLang;               // id of target language
      USHORT  usGetFile;               // id of target file
      USHORT  usAlternateGetFile;      // alternate ID of target file

      USHORT  usOldMatches = *pusMatchEntries;

      // replace any 0xA0 in language name to 0xFF
      // (0xA0 is 0xFF after processing by OemToAnsi)
      REPLACE_A0_BY_FF( pGetIn->szTargetLanguage );

      //get id of target language in the getin structure
      if (NTMGetIDFromNameEx( pTmClb, pGetIn->szTargetLanguage,
                              NULL,
                              (USHORT)LANG_KEY, &usGetLang,
                              0, NULL ))
      {
        usGetLang = 1;
      } /* endif */

      //get file name id of file name in the getin structure
      if ( NTMGetIDFromNameEx( pTmClb, pGetIn->szFileName,
                               pGetIn->szLongName,
                               (USHORT)FILE_KEY, &usGetFile,
                               0,
                               &usAlternateGetFile ))
      {
        usGetFile = 1;
      } /* endif */

      if ( pGetIn->ulParm & GET_IGNORE_PATH )
      {
        // get pointer to document name without path 
        if ( pGetIn->szLongName[0] != EOS )
        {
          pszDocName = UtlGetFnameFromPath( pGetIn->szLongName );
          if ( pszDocName == NULL )
          {
            pszDocName = pGetIn->szLongName;
          } /* endif */             
        }
        else
        {
          pszDocName = pGetIn->szFileName;
        } /* endif */           
      } /* endif */           


      /****************************************************************/
      /* get length of target block to work with                      */
      /****************************************************************/

      assert( RECLEN(pTmRecord) >= pTmRecord->usFirstTargetRecord );

      ulLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;

      //position at first target record
      pByte = (PBYTE)(pTmRecord+1);
      pByte += RECLEN(pTMXSourceRecord);
      pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
      pStartTarget = (PBYTE)pTMXTargetRecord;

      //source strings are identical so loop through target records
      while ( (usRc == NO_ERROR) && (ulLeftTgtLen && ( RECLEN(pTMXTargetRecord) != 0)) )
      {
        BOOL fTestCLB = TRUE;
        BOOL fMatchingDocName = FALSE;
        USHORT usContextRanking = 0;       // context ranking of this match


        /**************************************************************/
        /* update left target length                                  */
        /**************************************************************/
        usTgtNum++;            //  target number

        assert( ulLeftTgtLen >= RECLEN(pTMXTargetRecord) );

        ulLeftTgtLen -= RECLEN(pTMXTargetRecord);

        //next check the target language
        //position at target control block
        pByte += pTMXTargetRecord->usClb;
        pTMXTargetClb = (PTMX_TARGET_CLB)pByte;

        //compare target language group IDs
        if ( pTmClb->psLangIdToGroupTable[pTMXTargetClb->usLangId] != pTmClb->psLangIdToGroupTable[usGetLang] )
        {
          fTestCLB = FALSE;
        } 

        // check for comments when requested
        if ( pGetIn->ulParm & GET_IGNORE_COMMENT )
        {
          if ( NtmFindInAddData( pTMXTargetClb, ADDDATA_ADDINFO_ID, L"<Note" ) )
          {
            LOGPRINTF0( L"CLB with comment skipped\r\n" );
            fTestCLB = FALSE;
          } 
        } 

        // compare target table IDs
        if ( fTestCLB )
        {
          //position at source tag table record
          pByte = pStartTarget;
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

          //position at source tag table
          pByte += pTMXTargetRecord->usSourceTagTable;
          pTMXSourceTagTable = (PTMX_TAGTABLE_RECORD)pByte;

          //compare tag table records
          fStringEqual = memcmp( pTMXSourceTagTable, pSentence->pTagRecord,
                                 RECLEN(pTMXSourceTagTable) ) == 0;
          if ( !fStringEqual )
          {
            /**********************************************************/
            /* if tagging record is unequal than we have some         */
            /* (slight) differences                                   */
            /* i.e. we will create a fully qualified string and try   */
            /* another compare...                                     */
            /**********************************************************/
            //allocate pString
            if ( !pSentence->pPropString )
            {
              fOK = UtlAlloc( (PVOID *) &pSentence->pPropString, 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
            } /* endif */

            if ( fOK )
            {
              //calculate length of source string
//              USHORT  ulLenTmp;
//              usLenTmp = (USHORT)(RECLEN(pTMXSourceRecord) -
//                                  sizeof(TMX_SOURCE_RECORD));
              ULONG ulLenTmp = ulLen;       // len of pString in # of w's
              fOK = AddTagsToStringW( pString,
                                      &ulLenTmp,     // in # of w's
                                     pTMXSourceTagTable, pSentence->pPropString );
              if ( !fOK )
              {
                usRc = BTREE_CORRUPTED;
              }
              else
              {
                fStringEqual = FALSE;
                fStringEqual = (UtlCompIgnWhiteSpaceW(pSentence->pPropString,
                                              pSentence->pInputString,
                                              0 ) == 0 );
              } /* endif */
            } /* endif */
          } /* endif */

          if ( usRc == NO_ERROR )
          {
            if (fStringEqual)
            {
              fEqualFound = TRUE;
              usEqual = EQUAL_EQUAL;
            }
            else
            {
              usEqual = EQUAL_EQUAL - 3;
            } /* endif */

            //extract data from tm record
            //target with tags(TRUE)
            //compare file name and segment id in control block
            //position at control block
            pByte = pStartTarget;
            pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
            pByte += pTMXTargetRecord->usClb;
            pTMXTargetClb = (PTMX_TARGET_CLB)pByte;
            usTargetTranslationFlag = pTMXTargetClb->bTranslationFlag;

            // loop over CLBs and look for best matching entry
            {
              ULONG ulLeftClbLen;      // left CLB entries in CLB list
              PTMX_TARGET_CLB pClb;    // pointer for CLB list processing
              #define SEG_DOC_AND_CONTEXT_MATCH   8
              #define DOC_AND_CONTEXT_MATCH   7
              #define CONTEXT_MATCH           6
              #define SAME_SEG_AND_DOC_MATCH  5
              #define SAME_DOC_MATCH          4
              #define MULT_DOC_MATCH          3
              #define NORMAL_MATCH            2
              #define IGNORE_MATCH            1
              SHORT sCurMatch = 0;

              // loop over all target CLBs
              pClb = pTMXTargetClb;
              ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                             pTMXTargetRecord->usClb;
              while ( ulLeftClbLen && (sCurMatch < SAME_SEG_AND_DOC_MATCH) )
              {
                USHORT usTranslationFlag = pClb->bTranslationFlag;
                USHORT usCurContextRanking = 0;       // context ranking of this match
                BOOL fIgnoreProposal = FALSE;

                // apply global memory option file on global memory proposals
                if ( pClb->bTranslationFlag == TRANSLFLAG_GLOBMEM )
                {
                  if ( (pGetIn->pvGMOptList != NULL) && pClb->usAddDataLen  )
                  {
                    USHORT usAddDataLen = NtmGetAddData( pClb, ADDDATA_ADDINFO_ID, pContextBuffer, MAX_SEGMENT_SIZE );   
                    if ( usAddDataLen )
                    {
                      GMMEMOPT GobMemOpt = GlobMemGetFlagForProposal( pGetIn->pvGMOptList, pContextBuffer );
                      switch ( GobMemOpt )
                      {
                        case GM_SUBSTITUTE_OPT: usTranslationFlag = TRANSLFLAG_NORMAL; break;
                        case GM_HFLAG_OPT     : usTranslationFlag = TRANSLFLAG_GLOBMEM; break;
                        case GM_HFLAGSTAR_OPT : usTranslationFlag = TRANSLFLAG_GLOBMEMSTAR; break;
                        case GM_EXCLUDE_OPT   : fIgnoreProposal = TRUE; break;
                      } /* endswitch */   
                    } /* endif */                   
                  } /* endif */                 
                  if ( pClb == pTMXTargetClb )
                  {
                    usTargetTranslationFlag = usTranslationFlag;
                  } /* endif */                       
                } /* endif */                 

                // check context strings (if any)
                if ( (!fIgnoreProposal) && pGetIn->szContext[0] && pClb->usAddDataLen )
                {
                  USHORT usContextLen = NtmGetAddData( pClb, ADDDATA_CONTEXT_ID, pContextBuffer, MAX_SEGMENT_SIZE );   
                  if ( usContextLen != 0 )
                  {
                    usCurContextRanking = NTMCompareContext( pTmClb, pGetIn->szTagTable, pGetIn->szContext, pContextBuffer );
                  } /* endif */                     
                } /* endif */

                // check for matching document names
                if ( pGetIn->ulParm & GET_IGNORE_PATH )
                {
                  // we have to compare the real document names rather than comparing the document name IDs
                  PSZ pszCLBDocName = NTMFindNameForID( pTmClb, &(pClb->usFileId), (USHORT)FILE_KEY ); 
                  if ( pszCLBDocName != NULL )
                  {
                    PSZ pszName = UtlGetFnameFromPath( pszCLBDocName );
                    if ( pszName == NULL )
                    {
                      pszName = pszCLBDocName;
                    } /* endif */             
                    fMatchingDocName = stricmp( pszName, pszDocName ) == 0;
                  }
                  else
                  {
                    // could not access the document name, we have to compare the document name IDs
                    fMatchingDocName = ((pClb->usFileId == usGetFile) || (pClb->usFileId == usAlternateGetFile));
                  } /* endif */                     
                }
                else
                {
                  // we can compare the document name IDs
                  fMatchingDocName = ((pClb->usFileId == usGetFile) || (pClb->usFileId == usAlternateGetFile));
                } /* endif */


                if ( fIgnoreProposal )
                {
                  if ( sCurMatch == 0 )
                  {
                    sCurMatch = IGNORE_MATCH;
                  } /* endif */                     
                }
                else if ( usCurContextRanking == 100  )
                {
                  if ( fMatchingDocName && (pClb->ulSegmId >= (pGetIn->ulSegmentId - 1)) && (pClb->ulSegmId <= (pGetIn->ulSegmentId + 1)) )
                  {
                    if ( sCurMatch < SEG_DOC_AND_CONTEXT_MATCH )
                    {
                      sCurMatch = SEG_DOC_AND_CONTEXT_MATCH;
                      pTMXTargetClb = pClb;   // use this target CLB for match
                      usTargetTranslationFlag = usTranslationFlag;
                      usContextRanking = usCurContextRanking;
                    }
                  }
                  else if ( fMatchingDocName )
                  {
                    if ( sCurMatch < DOC_AND_CONTEXT_MATCH )
                    {
                      sCurMatch = DOC_AND_CONTEXT_MATCH;
                      pTMXTargetClb = pClb;   // use this target CLB for match
                      usTargetTranslationFlag = usTranslationFlag;
                      usContextRanking = usCurContextRanking;
                    }
                    else if ( sCurMatch == DOC_AND_CONTEXT_MATCH )
                    {
                      // we have already a match of this type so check if context ranking
                      if ( usCurContextRanking > usContextRanking )
                      {
                        pTMXTargetClb = pClb;   // use newer target CLB for match
                        usTargetTranslationFlag = usTranslationFlag;
                        usContextRanking = usCurContextRanking;
                      } 
                      // use time info to ensure that latest match is used
                      else if ( usCurContextRanking == usContextRanking )
                      {
                        // GQ 2015-04-10 New approach: If we have an exact-exact match use this one, otherwise use timestamp for the comparism
                        BOOL fExactExactNewCLB = fMatchingDocName && (pClb->ulSegmId >= (pGetIn->ulSegmentId - 1)) && (pClb->ulSegmId <= (pGetIn->ulSegmentId + 1));
                        BOOL fExactExactExistingCLB = ((pTMXTargetClb->usFileId == usGetFile) || (pTMXTargetClb->usFileId == usAlternateGetFile)) && 
                                                       (pTMXTargetClb->ulSegmId >= (pGetIn->ulSegmentId - 1)) && (pTMXTargetClb->ulSegmId <= (pGetIn->ulSegmentId + 1));
                        if ( fExactExactNewCLB && !fExactExactExistingCLB )
                        {
                          // use exact-exact CLB for match
                          pTMXTargetClb = pClb;   
                          usTargetTranslationFlag = usTranslationFlag;
                          usContextRanking = usCurContextRanking;
                        }
                        else if ( (fExactExactNewCLB == fExactExactExistingCLB) && (pClb->lTime > pTMXTargetClb->lTime) )
                        {
                          // use newer target CLB for match
                          pTMXTargetClb = pClb;   
                          usTargetTranslationFlag = usTranslationFlag;
                          usContextRanking = usCurContextRanking;
                        }
                      } /* endif */
                    } /* endif */
                  }
                  else
                  {
                    if ( sCurMatch < CONTEXT_MATCH )
                    {
                      sCurMatch = CONTEXT_MATCH;
                      pTMXTargetClb = pClb;   // use this target CLB for match
                      usTargetTranslationFlag = usTranslationFlag;
                      usContextRanking = usCurContextRanking;
                    }
                    else if ( sCurMatch == CONTEXT_MATCH )
                    {
                      // we have already a match of this type so check if context ranking
                      if ( usCurContextRanking > usContextRanking )
                      {
                        pTMXTargetClb = pClb;   // use newer target CLB for match
                        usTargetTranslationFlag = usTranslationFlag;
                        usContextRanking = usCurContextRanking;
                      } 
                      // use time info to ensure that latest match is used
                      else if ( (usCurContextRanking == usContextRanking) && (pClb->lTime > pTMXTargetClb->lTime) )
                      {
                        pTMXTargetClb = pClb;   // use newer target CLB for match
                        usTargetTranslationFlag = usTranslationFlag;
                        usContextRanking = usCurContextRanking;
                      } /* endif */
                    } /* endif */
                  } /* endif */                     
                }
                else if ( fMatchingDocName && (pClb->ulSegmId >= (pGetIn->ulSegmentId - 1)) && (pClb->ulSegmId <= (pGetIn->ulSegmentId + 1)) )
                {
                  // same segment from same document available
                  sCurMatch = SAME_SEG_AND_DOC_MATCH;
                  pTMXTargetClb = pClb;          // use this target CLB for match
                  usContextRanking = usCurContextRanking;
                  usTargetTranslationFlag = usTranslationFlag;
                }
                else if ( fMatchingDocName )
                {
                  // segment from same document available
                  if ( sCurMatch < SAME_DOC_MATCH )
                  {
                    sCurMatch = SAME_DOC_MATCH;
                    pTMXTargetClb = pClb;   // use this target CLB for match
                    usTargetTranslationFlag = usTranslationFlag;
                    usContextRanking = usCurContextRanking;
                  }
                  else if ( sCurMatch == SAME_DOC_MATCH )
                  {
                    // we have already a match of this type so
                    // use time info to ensure that latest match is used
                    if ( pClb->lTime > pTMXTargetClb->lTime )
                    {
                      pTMXTargetClb = pClb;   // use newer target CLB for match
                      usTargetTranslationFlag = usTranslationFlag;
                      usContextRanking = usCurContextRanking;
                    } /* endif */
                  } /* endif */
                }
                else if ( pClb->bMultiple )
                {
                  // multiple target segment available
                  if ( sCurMatch < MULT_DOC_MATCH )
                  {
                    // no better match yet
                    sCurMatch = MULT_DOC_MATCH;
                    pTMXTargetClb = pClb;   // use this target CLB for match
                    usTargetTranslationFlag = usTranslationFlag;
                    usContextRanking = usCurContextRanking;
                  } /* endif */
                } 
                else if ( usTranslationFlag == TRANSLFLAG_NORMAL )
                {
                  // a 'normal' memory match is available
                  if ( sCurMatch < NORMAL_MATCH )
                  {
                    // no better match yet
                    sCurMatch = NORMAL_MATCH;
                    pTMXTargetClb = pClb;   // use this target CLB for match
                    usTargetTranslationFlag = usTranslationFlag;
                    usContextRanking = usCurContextRanking;
                  } /* endif */
                } /* endif */

                // continue with next target CLB
                if ( sCurMatch < SAME_SEG_AND_DOC_MATCH )
                {
                  ulLeftClbLen -= TARGETCLBLEN(pClb);
                  if (ulLeftClbLen)
                  {
                    usTgtNum++;
                    pClb = NEXTTARGETCLB(pClb);
                  }
                } /* endif */
              } /* endwhile */


              {
                BOOL fNormalMatch = (usTargetTranslationFlag == TRANSLFLAG_NORMAL) || 
                                    (usTargetTranslationFlag == TRANSLFLAG_GLOBMEM) || 
                                    (usTargetTranslationFlag == TRANSLFLAG_GLOBMEMSTAR);
                switch ( sCurMatch )
                {
                  case IGNORE_MATCH :
                    usMatchLevel = 0;
                    break;
                  case SAME_SEG_AND_DOC_MATCH :
                    usMatchLevel = fNormalMatch ? usEqual+2 : usEqual-1;
                    break;
                  case SEG_DOC_AND_CONTEXT_MATCH :
                    usMatchLevel = fNormalMatch ? usEqual+2 : usEqual-1; // exact-exact match with matching context
                    break;
                  case DOC_AND_CONTEXT_MATCH :
                    if ( usContextRanking == 100 )
                    {
                      // GQ 2015/05/09: treat 100% context matches as normal exact matches
                      // usMatchLevel = fNormalMatch ? usEqual+2 : usEqual-1;
                      usMatchLevel = fNormalMatch ? usEqual+1 : usEqual-1;
                    }
                    else
                    {
                      usMatchLevel = fNormalMatch ? usEqual+1 : usEqual-1;
                    } /* endif */                       
                    break;
                  case CONTEXT_MATCH :
                    if ( usContextRanking == 100 )
                    {
                      // GQ 2015/05/09: treat 100% context matches as normal exact context matches
                      // usMatchLevel = fNormalMatch ? usEqual+2 : usEqual-1;
                      // GQ 2016/10/24: treat 100% context matches as normal exact matches
                      usMatchLevel = fNormalMatch ? usEqual : usEqual-1;
                    }
                    else
                    {
                      usMatchLevel = fNormalMatch ? usEqual : usEqual-1;
                    } /* endif */                       
                    break;
                  case SAME_DOC_MATCH :
                    usMatchLevel = fNormalMatch ? usEqual+1 : usEqual-1;
                    break;
                  case MULT_DOC_MATCH :
                    usMatchLevel = fNormalMatch ? usEqual+1 : usEqual-1;
                    break;
                  default :
                    usMatchLevel = fNormalMatch ? usEqual : usEqual-1;
                    break;
                } /* endswitch */
              }
            }
            pByte = pStartTarget;
            pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

            /**********************************************************/
            /* store only exact matches...                            */
            /**********************************************************/
            if ( usMatchLevel >= 100 )
            {
              ULONG ulSourceLen = ulLen;

              // use CLB translation flag is ustargetTranslationFlag is not set
              if ( usTargetTranslationFlag == (USHORT)-1 ) usTargetTranslationFlag = pTMXTargetClb->bTranslationFlag;
              FillMatchTable( pTmClb, pString, &ulSourceLen, pTMXTargetRecord,
                              pTMXTargetClb,
                              pstMatchTable, &usMatchLevel, TRUE,
                              pusMatchEntries, &pSentence->usActVote,
                              &pSentence->usActVote,
                              ulKeyNum, usTgtNum,
                              pGetIn,  pSentence->pTagRecord, usTargetTranslationFlag, usContextRanking );
            }
            else
            {
              /********************************************************/
              /* don't treat  MT matches as exact ones..              */
              /********************************************************/
            } /* endif */
          } /* endif */
        } /* endif */

        //position at next target
        pByte = pStartTarget;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
        //move pointer to end of target
        pByte += RECLEN(pTMXTargetRecord);
        //remember the end/beginning of record
        pStartTarget = pByte;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
      }  /* endwhile */

      /****************************************************************/
      /* check if we found an exact match -- if not we only use best  */
      /* fuzzy                                                        */
      /****************************************************************/
      if ( !fEqualFound )
      {
        /**************************************************************/
        /* use only the best one...                                   */
        /**************************************************************/
        if (*pusMatchEntries > usOldMatches )
        {
          *pusMatchEntries = usOldMatches + 1;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pString, 0L, 0L, NOMSG );
  if ( pContextBuffer ) UtlAlloc( (PVOID *)&pContextBuffer, 0L, 0L, NOMSG );



  if ( usRc )
  {
    ERREVENT2( EXACTTEST_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */


  return( usRc );
}

#if defined(SGMLDITA_LOGGING) || defined(INLINE_TAG_REPL_LOGGING) || defined(MATCHLIST_LOG)
  void NTMMarkCRLF( PSZ_W pszSource, PSZ_W pszTarget )
  {
    while ( *pszSource )
    {
      if ( *pszSource == L'\n' )
      {
        wcscpy( pszTarget, L"<lf>" );
        pszTarget += 4;
      }
      else if ( *pszSource == L'\r' )
      {
        wcscpy( pszTarget, L"<cr>" );
        pszTarget += 4;
      }
      else 
      {
        *pszTarget++ = *pszSource;
      } /* endif */
      pszSource++;
    } /*endwhile */
    *pszTarget = 0;
  }

  void NTMLogSegData( FILE *hfLog, PSZ_W pszForm, PSZ_W pszSegData )
  {
    static CHAR_W szSegBuf[4096];
    static CHAR_W szLineBuf[4096];
    int iLen;

    NTMMarkCRLF( pszSegData, szSegBuf );
    iLen = swprintf( szLineBuf, pszForm, szSegBuf );
    fwrite( szLineBuf, 2, iLen, hfLog ); 
  }

#endif

// removed superfluos whitespace at the beggining or end of the proposal
void NTMRemoveAdditionalWhitepace
(
  PSZ_W         pszSource, 
  PSZ_W         pszProposal 
)
{
  int           iLen;

  // only remove leading whitespace if source has none
  if ( (*pszSource != L'\n') && (*pszSource != L'\r') && (*pszSource != L' ') )
  {
    PSZ_W pszProp = pszProposal;
    PSZ_W pszTemp = pszProposal;

    while ( (*pszTemp == L'\n') || (*pszTemp == L'\r') || (*pszTemp == L' ') )
    {
      pszTemp++;
    } /*endwhile */

    if ( pszTemp != pszProp )
    {
      while ( *pszTemp )
      {
        *pszProp++ = *pszTemp++;
      } /*endwhile */
      *pszProp = 0;
    } /* endif */
  } /* endif */

  // check trailing whitespace
  iLen = wcslen( pszSource );
  if ( iLen )
  {
    iLen--;
    if  ( (pszSource[iLen] != L'\n') && (pszSource[iLen] != L'\r') && (pszSource[iLen] != L' ') )
    {
      iLen = wcslen( pszProposal );
      while ( iLen &&  ( (pszProposal[iLen-1] == L'\n') || (pszProposal[iLen-1] == L'\r') || (pszProposal[iLen-1] == L' ') ) )
      {
        iLen--;
        pszProposal[iLen] = 0;
      } /*endwhile */
    } /* endif */
  } /* endif */
} /* end of function NTMRemoveAdditionalWhitepace */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FillMatchTable                                           |
//+----------------------------------------------------------------------------+
//|Description:       Fills the match table structure as required by the       |
//|                   getout structure                                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_CLB pTmClb                                          |
//|                   PSZ pSourceString                                        |
//|                   PUSHORT pusSourceLen                                     |
//|                   PTMX_TARGET_RECORD pTMXTargetRecord                      |
//|                   PTMX_MATCH_TABLE pstMatchTable                           |
//|                   PUSHORT pusMatchLevel                                    |
//|                   BOOL fTag                                                |
//|                   PUSHORT pusMatchEntries                                  |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT pusMatchEntries                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|               determine the correct position in match table                |
//|               fill in all TMX_MATCH_TABLE field needed in the pGetOut      |
//|                structure                                                   |
//|               output number of entries in match table                      |
// ----------------------------------------------------------------------------+

USHORT FillMatchTable( PTMX_CLB pTmClb,         //ptr to ctl block struct
                   PSZ_W pSourceString,         //pointer to normalized source string
                   PULONG pulSourceLen,        //length of source string
                   PTMX_TARGET_RECORD pTMXTargetRecord, //pointer to tm target
                   PTMX_TARGET_CLB pTMXTargetClb,    //ptr to target control block
                   PTMX_MATCH_TABLE_W pstMatchTable, //getout match table struct
                   PUSHORT pusMatchLevel,       //how good is match
                   BOOL    fTag,                //target with/without subst. tags
                   PUSHORT pusMatchEntries,     //nr of entries in match table
                   PUSHORT pusMaxVotes,         //nr of tm source string votes
                   PUSHORT pusOverlaps,         //nr of overlapping triples
                   ULONG   ulKeyNum,            // key number
                   USHORT  usTgtNum,            // target number
                   PTMX_GET_W pGetIn,           // input structure
                   PTMX_TAGTABLE_RECORD pTagRecord,
                   USHORT usTranslationFlag,
                   USHORT usContextRanking )
{
  PSZ    pSrcFileName = pGetIn->szFileName;  // source file name
  PSZ    pSrcLongName = pGetIn->szLongName;  // long source file name
  PBYTE  pByte;                              //position pointer
  BOOL   fOK;                                //success indicator
  USHORT usRc = NO_ERROR;                    //return code
  ULONG  ulTargetLen;                        //length indicator
  ULONG  ulLen;                              //length indicator
  PSZ_W pString = NULL;                      //pointer to temp string
  USHORT usCurrentEntry;                     //counter
  BOOL fFound = FALSE;                       //indicates if correct pos found
  PTMX_SUBSTPROP pSubstProp = NULL;              // tag substitution array
  BOOL           fSubstAll = FALSE;
  PTMX_MATCH_TABLE_W pstMatchTableStart = pstMatchTable; //point to start of match table
  ULONG          ulSrcOemCP = 0L;
  ULONG          ulTgtOemCP = 0L;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

#ifdef SGMLDITA_LOGGING
  FILE *hfLog = NULL;
#endif
  SHORT sLangID = 0;

  pusMaxVotes;

  // force fTag flag if proposal with inline tagging is requested
  if ( pGetIn->ulParm & GET_ALWAYS_WITH_TAGS )
  {
    fTag = TRUE;
  } /* endif */

  /********************************************************************/
  /* if tagtables are different, call substitution function           */
  /********************************************************************/
  if ( !(pGetIn->ulParm & GET_NO_GENERICREPLACE) )
  {
    fOK = UtlAlloc( (PVOID*)&pSubstProp, 0L, (LONG)sizeof(TMX_SUBSTPROP), NOMSG);

    if ( !fOK )
    {
      usRc = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
      /****************************************************************/
      /* add current tagging to source and translation of proposal    */
      /****************************************************************/
      ulSrcOemCP = GetLangOEMCP( pGetIn->szSourceLanguage);
      strcpy( pSubstProp->szSourceTagTable, pGetIn->szTagTable );
      strcpy( pSubstProp->szSourceLanguage, pGetIn->szSourceLanguage);
      strcpy( pSubstProp->szTargetLanguage, pGetIn->szTargetLanguage );
      UTF16strcpy( pSubstProp->szSource, pGetIn->szSource );
      pSubstProp->pTagsSource = pTagRecord;
      pByte = (PBYTE)pTMXTargetRecord;
      pByte += pTMXTargetRecord->usSourceTagTable;

      MorphGetLanguageID( pGetIn->szSourceLanguage, &sLangID );

      pSubstProp->pTagsPropSource = (PTMX_TAGTABLE_RECORD)pByte;
      usRc = (AddTagsToStringW( pSourceString,
                              pulSourceLen,      // in # of w's
                             (PTMX_TAGTABLE_RECORD)pByte,
                             pSubstProp->szPropSource )) ? usRc : BTREE_CORRUPTED;
      if ( usRc == NO_ERROR )
      {
        PSZ_W  pTarget;

        //position at target string
        pByte = (PBYTE)pTMXTargetRecord;
        pByte += pTMXTargetRecord->usTarget;

        //calculate length of target string
        ulTargetLen = pTMXTargetRecord->usClb - pTMXTargetRecord->usTarget;

        if ( UtlAlloc( (PVOID *) &pTarget, 0L, (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG))
        {
          ulTargetLen = EQFCompress2Unicode(pTarget, pByte, ulTargetLen);
          //position at target tag record
          pByte = (PBYTE)pTMXTargetRecord;
          pByte += pTMXTargetRecord->usTargetTagTable;

          pSubstProp->pTagsPropTarget = (PTMX_TAGTABLE_RECORD)pByte;
          usRc = (AddTagsToStringW( pTarget,
                                    &ulTargetLen,     // in # of w's
                                    (PTMX_TAGTABLE_RECORD)pByte,
                                    pSubstProp->szPropTarget )) ? usRc : BTREE_CORRUPTED;
          UtlAlloc( (PVOID *) &pTarget, 0L, 0L, NOMSG );
        }
        else
        {
          usRc = ERROR_NOT_ENOUGH_MEMORY;
        }
        if ( usRc == NO_ERROR )
        {
          //fill in the markup table
          PBYTE p = ((PBYTE)pTMXTargetRecord)+pTMXTargetRecord->usTargetTagTable;
          NTMGetNameFromID( pTmClb,
                            &(((PTMX_TAGTABLE_RECORD)p)->usTagTableId),
                            (USHORT)TAGTABLE_KEY,
                            pSubstProp->szPropTagTable, NULL );
          ulTgtOemCP = GetLangOEMCP( pSubstProp->szTargetLanguage);
          // replace any 0xA0 in language name to 0xFF
          // (0xA0 is 0xFF after processing by OemToAnsi)
          REPLACE_A0_BY_FF( pSubstProp->szTargetLanguage );

          //fill in the target language
          NTMGetNameFromID( pTmClb, &pTMXTargetClb->usLangId,
                            (USHORT)LANG_KEY,
                            pSubstProp->szTargetLanguage, NULL );
        } /* endif */

        /**************************************************************/
        /* call tag substitution function (if we are not dealing with */
        /* an exact match) ...                                        */
        /**************************************************************/
        if ( ( usRc == NO_ERROR ) && (*pusMatchLevel < EQUAL_EQUAL) )
        {
          BOOL fPreprocessed = FALSE;
          PTMX_SENTENCE pMatchSource = NULL;
          PTMX_SENTENCE pMatchTarget = NULL;
          BOOL fSpecialProcessing = FALSE;
          BOOL fSkipPostprocessing = FALSE;

          if ( UtlQueryUShort( QS_SGMLDITAPROCESSING ) )
          {
            fSpecialProcessing = DITASpecialProcessingRequired( pSubstProp->szSourceTagTable, 
                                                                pSubstProp->szPropTagTable, DITAPREPAREPROPOSAL );
          } /* endif */


          if ( fSpecialProcessing )
          {
#ifdef SGMLDITA_LOGGING
            CHAR szLogFile[MAX_EQF_PATH];
            UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
            UtlMkDir( szLogFile, 0L, FALSE );
            strcat( szLogFile, "\\SGMLDITA.LOG" );
            if ( !UtlFileExist( szLogFile ) )
            {
              hfLog = fopen( szLogFile, "wb" );
              if ( hfLog )
              {
                fwrite( "\xFF\xFE", 2, 1, hfLog );            // write out BOM
                fclose( hfLog );
              } /* endif */
            } /* endif */
            hfLog = fopen( szLogFile, "ab" );
#endif

#ifdef SGMLDITA_LOGGING
            if ( hfLog )
            {
              fwprintf( hfLog, L"\r\n*** Doc=%S, Seg=%lu, preparing IBMIDDOC segment\r\n", pGetIn->szLongName, pGetIn->ulSegmentId );
              NTMLogSegData( hfLog, L"  <Segment>%s</Segment>\r\n", pGetIn->szSource );
              fwprintf( hfLog, L"before preparation\r\n" );
              NTMLogSegData( hfLog, L"  <PropSource>%s</PropSource>\r\n", pSubstProp->szPropSource );
              NTMLogSegData( hfLog, L"  <PropTarget>%s</PropTarget>\r\n", pSubstProp->szPropTarget );
            } /* endif */
#endif

            fPreprocessed = DITAPreprocessProposals( pSubstProp->szPropSource, pSubstProp->szPropTarget, pGetIn->pvReplacementList );

            // if proposal has been preprocessed, the tag tagle records have to be rebuild
            if ( fPreprocessed )
            {
              USHORT usRC = 0;
              CHAR szTagTablePath[MAX_EQF_PATH];
              ULONG ulSrcCP = GetLangOEMCP( pSubstProp->szSourceLanguage );
              ULONG ulTgtCP = GetLangOEMCP( pSubstProp->szTargetLanguage );

              usRC = NTMAllocSentenceStructure( &pMatchSource );
              if ( !usRC ) NTMAllocSentenceStructure( &pMatchTarget );
              if ( !usRC ) UTF16strcpy( pMatchSource->pInputString, pSubstProp->szPropSource  );

              UtlMakeEQFPath( szTagTablePath, NULC, TABLE_PATH, NULL );
              strcat( szTagTablePath, BACKSLASH_STR );
              strcat( szTagTablePath, pSubstProp->szPropTagTable );
              strcat( szTagTablePath, EXT_OF_FORMAT );

              if ( !usRC ) usRC = TokenizeSourceEx2( NULL, pMatchSource, szTagTablePath, pSubstProp->szSourceLanguage, TM_MAJ_VERSION, ulSrcCP, CREATEPROTTABLE_NOTRANSLVAR );
              if ( !usRC ) pSubstProp->pTagsPropSource = pMatchSource->pTagRecord;
              if ( !usRC ) UTF16strcpy( pMatchTarget->pInputString, pSubstProp->szPropTarget );
              if ( !usRC ) usRC = TokenizeSourceEx2( NULL, pMatchTarget, szTagTablePath, pSubstProp->szTargetLanguage, TM_MAJ_VERSION, ulTgtCP, CREATEPROTTABLE_NOTRANSLVAR  );
              if ( !usRC ) pSubstProp->pTagsPropTarget = pMatchTarget->pTagRecord;

              if ( fPreprocessed == 2 )     // 2 = TMO replacements have been done
              {
                // recompute fuzziness and skip postprocessing if we have an exact match
                USHORT usFuzzy = 0;
                BOOL fFuzzyOK;
                fFuzzyOK = TMFuzzyness( pGetIn->szTagTable, pSubstProp->szPropSource, pSubstProp->szSource,
                                             sLangID, &usFuzzy, ulSrcOemCP );
                if ( usFuzzy == 100 )
                {
                  fSkipPostprocessing = TRUE;
                } /* endif */

              } /* endif */
            } /* endif */

#ifdef SGMLDITA_LOGGING
            if ( hfLog )
            {
              if ( fPreprocessed )
              {
                fwprintf( hfLog, L"after preprocessing\r\n" );
                NTMLogSegData( hfLog, L"  <PropSource>%s</PropSource>\r\n", pSubstProp->szPropSource );
                NTMLogSegData( hfLog, L"  <PropTarget>%s</PropTarget>\r\n", pSubstProp->szPropTarget );
              }
              else
              {
                fwprintf( hfLog, L"no preprocessing has been performed\r\n" );
              } /* endif */
            } /* endif */
#endif
          } /* endif */

    		  fSubstAll =  NTMTagSubst( pSubstProp, ulSrcOemCP, ulTgtOemCP );

#ifdef SGMLDITA_LOGGING
          if ( hfLog && fSpecialProcessing )
          {
            if ( fSubstAll )
            {
              fwprintf( hfLog, L"all inline tagging replaced by generic inline tag replacement\r\n" );
              NTMLogSegData( hfLog, L"  <PropSource>%s</PropSource>\r\n", pSubstProp->szPropSource );
              NTMLogSegData( hfLog, L"  <PropTarget>%s</PropTarget>\r\n", pSubstProp->szPropTarget );
            } /* endif */
          } /* endif */
#endif
          if ( !fSubstAll && fSpecialProcessing ) 
          { 
            if ( !fSkipPostprocessing )
            {
              fSubstAll = DITAProcessProposal( pGetIn->szSource, pSubstProp->szPropSource, 
                                              pSubstProp->szPropTarget, pSubstProp->szPropTagTable, ulSrcOemCP  );
            } /* endif */
            if ( !fSubstAll && fPreprocessed )
            {
              // segment has been modified by preprocessing, so force a re-computation of the fuzziness
              fSubstAll = TRUE;
            } /* endif */
#ifdef SGMLDITA_LOGGING
            if ( hfLog && fSubstAll )
            {
              fwprintf( hfLog, L"after post processing\r\n" );
              NTMLogSegData( hfLog, L"  <PropSource>%s</PropSource>\r\n", pSubstProp->szPropSource );
              NTMLogSegData( hfLog, L"  <PropTarget>%s</PropTarget>\r\n", pSubstProp->szPropTarget );
            }
            else
            {
              fwprintf( hfLog, L"no postprocessing has been performed\r\n" );
            } /* endif */
#endif
          } /* endif */

          if ( pMatchSource ) NTMFreeSentenceStructure( pMatchSource );
          if ( pMatchTarget ) NTMFreeSentenceStructure( pMatchTarget );
        } /* endif */
      } /* endif */
    } /* endif */
  }
  else
  {
  } /* endif */

  /********************************************************************/
  /* if src of prop is empty ( i.e. only inline tagging), add tagging */
  /********************************************************************/
  if (*pulSourceLen == 0 )
  {
    fTag = TRUE;
  } /* endif */

  if ( usRc == NO_ERROR )
  {

    //locate correct position in matchtable array
    usCurrentEntry = 0;
    while ( !fFound && usCurrentEntry < MAX_MATCHES )
    {
      BOOL fEndOfTable = (usCurrentEntry + 1) >= MAX_MATCHES;

      int iMatchSortKey = pFactory->getProposalSortKey( GetMatchTypeFromMatchLevel( pstMatchTable->usMatchLevel ), GetProposalTypeFromFlag( pstMatchTable->usTranslationFlag ), (int)pstMatchTable->usMatchLevel , -1, 
                          pstMatchTable->usContextRanking, fEndOfTable );
      int iNewMatchSortKey = pFactory->getProposalSortKey( GetMatchTypeFromMatchLevel( *pusMatchLevel ), GetProposalTypeFromFlag( usTranslationFlag ), 
                             (int)*pusMatchLevel, -1, usContextRanking, fEndOfTable );


      if ( iMatchSortKey  > iNewMatchSortKey  )
      {
        pstMatchTable++;
        usCurrentEntry++;
      }
      else if ( iMatchSortKey == iNewMatchSortKey )
      {
        CHAR szTgtFileName[ MAX_FILESPEC ];                  // target file name
        static CHAR szTgtLongName[ MAX_LONGFILESPEC ];       // long target file name

        //fill in the target file name
        NTMGetNameFromID( pTmClb, &pTMXTargetClb->usFileId, (USHORT)FILE_KEY, szTgtFileName, szTgtLongName );

        /****************************************************************/
        /* as the matching levels are the same determine the most recent*/
        /* one, but have in mind that proposals from the same file have */
        /* higher priority                                              */
        /****************************************************************/
        if ( NTMDocMatch( szTgtFileName, szTgtLongName, pSrcFileName, pSrcLongName ) )
        {
          /**************************************************************/
          /* if compared match is from same file, check the update time */
          /* because now we deal with two matches from same source doc. */
          /**************************************************************/
          if ( (pstMatchTable->lTargetTime > pTMXTargetClb->lTime) &&
               NTMDocMatch( pstMatchTable->szFileName, pstMatchTable->szLongName,
                            pSrcFileName, pSrcLongName ) )
          {
            pstMatchTable++;
            usCurrentEntry++;
          }
          else
          {
            //found the right position
              ULONG ulLen = ( MAX_MATCHES - usCurrentEntry - 1 ) * sizeof(TMX_MATCH_TABLE_W);
              memmove( pstMatchTable+1, pstMatchTable, ulLen );
              memset( pstMatchTable, 0, sizeof(TMX_MATCH_TABLE_W) );
              fFound = TRUE;
          } /* endif */
        }
        else
        {
          /**************************************************************/
          /* if compared match is from other file, check update time    */
          /* otherwise put in our match, it's coming from same source   */
          /* document                                                   */
          /**************************************************************/
          if ( NTMDocMatch( pstMatchTable->szFileName, pstMatchTable->szLongName, pSrcFileName, pSrcLongName ) )
          {
            // match from match table is from same document, so skip to next position
            pstMatchTable++;
            usCurrentEntry++;
          }
          else if ( pstMatchTable->lTargetTime > pTMXTargetClb->lTime )
          {
            // match from match table is newer
            pstMatchTable++;
            usCurrentEntry++;
          }
          else
          {
            //found the right position
              ULONG ulLen = ( MAX_MATCHES - usCurrentEntry - 1 ) * sizeof(TMX_MATCH_TABLE_W);
              memmove( pstMatchTable+1, pstMatchTable, ulLen );
              memset( pstMatchTable, 0, sizeof(TMX_MATCH_TABLE_W) );
              fFound = TRUE;
          } /* endif */
        } /* endif */


      }
      else
      {
        //found the right position
        ULONG ulLen = ( MAX_MATCHES - usCurrentEntry - 1 ) * sizeof(TMX_MATCH_TABLE_W);
        memmove( pstMatchTable+1, pstMatchTable, ulLen );
        memset( pstMatchTable, 0, sizeof(TMX_MATCH_TABLE_W) );
        fFound = TRUE;
      } /* endif */
    } /* endwhile */

    if ( fFound )
    {
      //allocate pString
      fOK = UtlAlloc( (PVOID *) &(pString), 0L, (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );

      if ( !fOK )
      {
        usRc = ERROR_NOT_ENOUGH_MEMORY;
      }
      else
      {
        pByte = (PBYTE)pTMXTargetRecord;
        pByte += pTMXTargetRecord->usSourceTagTable;
        if (fSubstAll )
        {
          /**************************************************************/
          /* do a substitution of the tags...                           */
          /**************************************************************/
          *pulSourceLen = UTF16strlenCHAR( pSubstProp->szPropSource );
          memcpy( pstMatchTable->szSource, pSubstProp->szPropSource,
                  *pulSourceLen * sizeof(CHAR_W));
        }
        else
        {
          if ( fTag )
          {
            /************************************************************/
            /* add tags to source string                                */
            /************************************************************/
            fOK = AddTagsToStringW( pSourceString, pulSourceLen,
                                   (PTMX_TAGTABLE_RECORD)pByte,
                                   pstMatchTable->szSource );
            if ( !fOK )
            {
              usRc = BTREE_CORRUPTED;
            } /* endif */
          }
          else
          {
            //else copy normalized propsource string
            memcpy( pstMatchTable->szSource, pSourceString, *pulSourceLen * sizeof(CHAR_W));
          } /* endif */
        } /* endif */

        if ( usRc == NO_ERROR )
        {
          //position at target string
          pByte = (PBYTE)pTMXTargetRecord;
          pByte += pTMXTargetRecord->usTarget;

          //calculate length of target string
          ulTargetLen = pTMXTargetRecord->usClb - pTMXTargetRecord->usTarget;

          //copy target string for later compare function
          ulTargetLen = EQFCompress2Unicode( pString, pByte, ulTargetLen );
          // now ulTargetLen is # of w's
          //position at target tag record
          pByte = (PBYTE)pTMXTargetRecord;
          pByte += pTMXTargetRecord->usTargetTagTable;

          if (fSubstAll )
          {
            /**************************************************************/
            /* do a substitution of the replaced target string and        */
            /* adjust match level                                         */
            /**************************************************************/
            ulTargetLen = UTF16strlenCHAR( pSubstProp->szPropTarget );
            memcpy( pstMatchTable->szTarget, pSubstProp->szPropTarget,
                    ulTargetLen * sizeof(CHAR_W));

            ulLen = UTF16strlenCHAR(pSubstProp->szSource);
            *pulSourceLen = UTF16strlenCHAR(pSubstProp->szPropSource);
            if (*pulSourceLen > ulLen )
            {
              ulLen = *pulSourceLen;
            } /* endif */

            {
              BOOL fEqual;
              USHORT usFuzzy = 0;
              BOOL fFuzzynessOK;

/*Q!*/        fFuzzynessOK = TMFuzzyness( pGetIn->szTagTable,
                                          pSubstProp->szPropSource, pSubstProp->szSource,
                                          sLangID, &usFuzzy, ulSrcOemCP );

              if ( pGetIn->ulParm & GET_RESPECTCRLF )
              {
                fEqual = (UtlCompIgnSpaceW( pSubstProp->szPropSource,
                                           pSubstProp->szSource, 0 ) == 0);
              }
              else
              {
                fEqual = (usFuzzy == 100);
//                  (UtlCompIgnWhiteSpace( pSubstProp->szPropSource,
//                                                pSubstProp->szSource,
//                                                ulLen) == 0);

              } /* endif */

              // GQ 2004/08/04: do not set match level to 100 for machine translations!!
              // RJ 2004/08/16: assure that *pusMatchLevel is filled independently
              // of bMT flag set or not (P020111)
              if ( fEqual && (usFuzzy == 100) /*&& !pTMXTargetClb->bMT */)
              {
                *pusMatchLevel = (usTranslationFlag  == TRANSLFLAG_MACHINE) ? 99 : 100;
              } /* endif */
#ifdef SGMLDITA_LOGGING
              if ( hfLog )
              {
                fwprintf( hfLog, L"new match level is %u, fuzziness is %u\r\n", *pusMatchLevel, usFuzzy );
              } /* endif */
#endif
            }
          }
          else
          {
            if ( fTag )
            {
              //add tags to target string if flag set to true
              fOK = AddTagsToStringW( pString, &ulTargetLen,
                                     (PTMX_TAGTABLE_RECORD)pByte,
                                     pstMatchTable->szTarget );
              if ( !fOK )
              {
                usRc = BTREE_CORRUPTED;
              } /* endif */
            }
            else
            {
              //else copy normalized target string
              memcpy( pstMatchTable->szTarget, pString, ulTargetLen * sizeof(CHAR_W));
            } /* endif */
          } /* endif */

          if ( usRc == NO_ERROR )
          {
            //position at target control block
            // should already be done!!!
  //          pByte = (PBYTE)pTMXTargetRecord;
  //          pByte += pTMXTargetRecord->usClb;
  //          pTMXTargetClb = (PTMX_TARGET_CLB)pByte;

            //fill in the target file name
            NTMGetNameFromID( pTmClb, &pTMXTargetClb->usFileId,
                              (USHORT)FILE_KEY,
                              pstMatchTable->szFileName,
                              pstMatchTable->szLongName );
            //fill in the target author - don't care about author name....
            NTMGetNameFromID( pTmClb, &pTMXTargetClb->usAuthorId,
                              (USHORT)AUTHOR_KEY,
                              pstMatchTable->szTargetAuthor, NULL );

            //fill in the markup table
            {
              PBYTE p = ((PBYTE)pTMXTargetRecord)+pTMXTargetRecord->usTargetTagTable;
              NTMGetNameFromID( pTmClb,
                                &(((PTMX_TAGTABLE_RECORD)p)->usTagTableId),
                                (USHORT)TAGTABLE_KEY,
                                pstMatchTable->szTagTable, NULL );
            }

            //fill in the target language
            NTMGetNameFromID( pTmClb, &pTMXTargetClb->usLangId,
                              (USHORT)LANG_KEY,
                              pstMatchTable->szTargetLanguage, NULL );
            //fill in the segment id
            pstMatchTable->ulSegmentId = pTMXTargetClb->ulSegmId;
            //state whether machine translation
            pstMatchTable->usTranslationFlag = usTranslationFlag ;
            //fill in target time
            pstMatchTable->lTargetTime = pTMXTargetClb->lTime;
            //fill in match percentage
            pstMatchTable->usMatchLevel = *pusMatchLevel;
            //fill in nr of overlapping triples
            pstMatchTable->usOverlaps = *pusOverlaps;
            pstMatchTable->usContextRanking = usContextRanking;


            pstMatchTable->szContext[0] = 0;
            pstMatchTable->szAddInfo[0] = 0;
            if ( pTMXTargetClb->usAddDataLen )
            {
              NtmGetAddData( pTMXTargetClb, ADDDATA_CONTEXT_ID, pstMatchTable->szContext, sizeof(pstMatchTable->szContext) / sizeof(CHAR_W) );
              NtmGetAddData( pTMXTargetClb, ADDDATA_ADDINFO_ID, pstMatchTable->szAddInfo, sizeof(pstMatchTable->szAddInfo) / sizeof(CHAR_W) );
            } /* endif */

            /****************************************************************/
            /* fill in key and target number ...                            */
            /****************************************************************/
            pstMatchTable->ulKey = ulKeyNum;
            pstMatchTable->usTargetNum = usTgtNum;


            // check if after generic inline substitution the quality of the match
            // is still at the right place in the match table.
            while ( (pstMatchTableStart < pstMatchTable) )
            {
              PTMX_MATCH_TABLE_W pstMatchTableTemp = pstMatchTable-1;
              BOOL fEndOfTable = FALSE;
              int iMatchSortKey = pFactory->getProposalSortKey( GetMatchTypeFromMatchLevel( pstMatchTable->usMatchLevel ), GetProposalTypeFromFlag( pstMatchTable->usTranslationFlag ), 
                  (int)pstMatchTable->usMatchLevel , -1, pstMatchTable->usContextRanking, fEndOfTable );
              int iMatchSortKeyTemp = pFactory->getProposalSortKey( GetMatchTypeFromMatchLevel( pstMatchTableTemp->usMatchLevel ), GetProposalTypeFromFlag( pstMatchTableTemp->usTranslationFlag ), 
                  (int)pstMatchTableTemp->usMatchLevel, -1, pstMatchTableTemp->usContextRanking, fEndOfTable );

              if ((iMatchSortKeyTemp <  iMatchSortKey) ||
                  ((iMatchSortKeyTemp == iMatchSortKey) &&
                   (pstMatchTableTemp->lTargetTime  <   pstMatchTable->lTargetTime) &&
                    NTMDocMatch( pstMatchTable->szFileName, pstMatchTable->szLongName,
                                 pstMatchTableTemp->szFileName, pstMatchTableTemp->szLongName)) )
              {
                // interchange entries pstMatchTable moved on position pstMatchTableTemp
                TMX_MATCH_TABLE_W stMatchEntry;
                memcpy( &stMatchEntry, pstMatchTableTemp, sizeof( stMatchEntry ));
                memcpy( pstMatchTableTemp, pstMatchTable, sizeof( stMatchEntry ));
                memcpy( pstMatchTable, &stMatchEntry,     sizeof( stMatchEntry ));
                pstMatchTable = pstMatchTableTemp;
              }
              else
              {
                // leave loop
                pstMatchTableStart = pstMatchTable;
              }
            }


            //update match entry structure counter
            if ( *pusMatchEntries < MAX_MATCHES )
            {
              (*pusMatchEntries)++;
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */

      //release memory
      UtlAlloc( (PVOID *) &pString, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  if ( usRc )
  {
    ERREVENT2( FILLMATCHTABLE_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */

#ifdef SGMLDITA_LOGGING
  if ( hfLog ) fclose( hfLog );
#endif

  /********************************************************************/
  /* free any allocated resource                                      */
  /********************************************************************/
  NTMFreeSubstProp( pSubstProp );

  return( usRc );
}




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AddTagsToString                                          |
//+----------------------------------------------------------------------------+
//|Description:       Add tags to correct position of the normalized string    |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ pInString                                            |
//|                   PUSHORT ousInStringLen                                   |
//|                   PTMX_TAGTABLE_RECORD pTMXTagTable                        |
//|                   PSZ pOutString                                           |
//+----------------------------------------------------------------------------+
//|Output parameter:  PSZ pOutString                                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:     add tags at the given offsets and output string with tags|
// ----------------------------------------------------------------------------+

BOOL AddTagsToStringW( PSZ_W pInString,            //character string
                  PULONG pulInStringLen,       //length of pInString in # of w's
                  PTMX_TAGTABLE_RECORD pTMXTagTable,  //ptr to tag table record
                  PSZ_W pOutString )                    //output string
{
  PBYTE pByte;                         //record position pointer
  USHORT usStringPos, j;               //positioning counters
  PTMX_TAGENTRY pTagEntry = NULL;      //ptr to tag entries in tag table record
  ULONG  ulTagLen = 0;                 //length of all tags added to string
  ULONG  ulEnd = 0;                    //end length for for loop
  USHORT usTagRecordPos = 0;           //current length processed in tag record
  BOOL fNoTagsLeft = FALSE;            //indication of existence of tags
  ULONG  ulInputStringLen = *pulInStringLen;
  PSZ_W  pOutStart = pOutString;       // anchar start pointers
  PSZ_W  pInStart  = pInString;        //
  BOOL   fOK = TRUE;
  //position at beginning of target record
  pByte = (PBYTE)pTMXTagTable;
  pByte += pTMXTagTable->usFirstTagEntry;



  fOK = (*pulInStringLen < MAX_SEGMENT_SIZE );

  if ( fOK && ( RECLEN(pTMXTagTable) > sizeof(TMX_TAGTABLE_RECORD)) )
  {
    //there is tag info so go through all tag entries and add tags to outstring
    pTagEntry = (PTMX_TAGENTRY)pByte;

    //start of for loop
    usStringPos = 0;
    //initial positioning st beginning of first tag entry
    usTagRecordPos = sizeof(TMX_TAGTABLE_RECORD);
    //initial stop criterium for end of for loop
    ulEnd = pTagEntry->usOffset;

    while ( fOK && (*pInString || !fNoTagsLeft) )
    {
      if ( ulEnd <=  ulInputStringLen )
      {
        for ( j = usStringPos; j < ulEnd; j++, pOutString++, pInString++ )
        {
          *pOutString = *pInString;
        } /* endfor */
      }
      else
      {
        for ( j = usStringPos; j < ulEnd; j++, pOutString++, pInString++ )
        {
          if ( j < ulInputStringLen )
          {
            *pOutString = *pInString;
          }
          else
          {
            *pOutString = ' ';
            --pInString;     // we've gone 1 too far..
          } /* endif */

          if ( pOutString >=
                  pOutStart + MAX_SEGMENT_SIZE + usStringPos - ulEnd )
          {
            *pOutStart = EOS;
            fOK = FALSE;
            break;
          } /* endif */
        } /* endfor */
      } /* endif */

      if ( fOK && (j == ulEnd) && (!fNoTagsLeft) )
      {
        /**************************************************************/
        /* Add check that tag length does not jeopardize our          */
        /* output string...                                           */
        /**************************************************************/
        if ( (pOutString+pTagEntry->usTagLen)  < (pOutStart+MAX_SEGMENT_SIZE)
              && (pTagEntry->usTagLen < MAX_SEGMENT_SIZE)    )
        {
          USHORT usTagLenCHAR = pTagEntry->usTagLen;
          memcpy( pOutString, &pTagEntry->bData, usTagLenCHAR * sizeof(CHAR_W));
          pOutString += usTagLenCHAR;
          ulInputStringLen += usTagLenCHAR;
          //reset for loop start
          usStringPos = j + usTagLenCHAR;
          //update tag table record position counter
          usTagRecordPos += sizeof(TMX_TAGENTRY) + usTagLenCHAR * sizeof(CHAR_W);
          //remember length of tags added to string
          ulTagLen += pTagEntry->usTagLen;

          //move to next tag entry only if there are still tags to add to string
          if ( usTagRecordPos < RECLEN(pTMXTagTable) )
          {
            USHORT usOldOffs = pTagEntry->usOffset;
            pByte += sizeof(TMX_TAGENTRY) + pTagEntry->usTagLen * sizeof(CHAR_W);
            pTagEntry = (PTMX_TAGENTRY)pByte;
            //new end criterium for for loop
            if ( usOldOffs == pTagEntry->usOffset  )
            {
              // something went wrong: two tag entries for the same offset
              fOK = FALSE;
              memcpy( pOutStart, pInStart, *pulInStringLen * sizeof(CHAR_W));
              fNoTagsLeft = TRUE;
              pInString = pInStart + *pulInStringLen;
              ulEnd = *pulInStringLen +  ulTagLen;
            }
            else if ( pTagEntry->usOffset < ulEnd  )
            {
              // tag data seems to be corrupted, ignore the tags
              fOK = FALSE;
              memcpy( pOutStart, pInStart, *pulInStringLen * sizeof(CHAR_W));
              fNoTagsLeft = TRUE;
              pInString = pInStart + *pulInStringLen;
              ulEnd = *pulInStringLen +  ulTagLen;
            }
            else
            {
              ulEnd = pTagEntry->usOffset;
            } /* endif */
          }
          else
          {
            fNoTagsLeft = TRUE;
            //new end criterium for for loop, in this case end of string
            ulEnd = *pulInStringLen +  ulTagLen;
          } /* endif */
        }
        else
        {
          // something got corrupted -- ignore the tags ...
          fOK = FALSE;
          memcpy( pOutStart, pInStart, *pulInStringLen * sizeof(CHAR_W) );
          fNoTagsLeft = TRUE;
          pInString = pInStart + *pulInStringLen;
          ulEnd = *pulInStringLen +  ulTagLen;
        } /* endif */
      } /* endif */

      if ( pOutString >= pOutStart + MAX_SEGMENT_SIZE )
      {
        *pOutStart = EOS;
        fOK = FALSE;
        break;
      } /* endif */
    } /* endwhile */

    if ( *pOutStart )
    {
      // Add the end of string
      *pOutString = EOS;
    }
  }
  else
  {
    //there are no tags so copy character string
    if ( *pulInStringLen < MAX_SEGMENT_SIZE )
    {
      memcpy( pOutString, pInString, *pulInStringLen * sizeof(CHAR_W));
      pOutString[*pulInStringLen] = EOS;
    }
    else
    {
      fOK = FALSE;
      *pOutStart = EOS;
    } /* endif */
  } /* endif */

  return fOK;

}

BOOL AddTagsToString( PSZ pInString,            //character string
                  PULONG pulInStringLen,       //length of pInString
                  PTMX_TAGTABLE_RECORD pTMXTagTable,  //ptr to tag table record
                  PSZ pOutString )                    //output string
{
  PBYTE pByte;                         //record position pointer
  USHORT usStringPos, j;               //positioning counters
  PTMX_TAGENTRY pTagEntry = NULL;      //ptr to tag entries in tag table record
  ULONG ulTagLen = 0;                 //length of all tags added to string
  ULONG ulEnd = 0;                    //end length for for loop
  USHORT usTagRecordPos = 0;           //current length processed in tag record
  BOOL fNoTagsLeft = FALSE;            //indication of existence of tags
  ULONG ulInputStringLen = *pulInStringLen;
  PSZ    pOutStart = pOutString;       // anchar start pointers  @@1
  PSZ    pInStart  = pInString;        //                        @@1
  BOOL   fOK = TRUE;
  //position at beginning of target record
  pByte = (PBYTE)pTMXTagTable;
  pByte += pTMXTagTable->usFirstTagEntry;



  fOK = (*pulInStringLen < MAX_SEGMENT_SIZE );

  if ( fOK && ( RECLEN(pTMXTagTable) > sizeof(TMX_TAGTABLE_RECORD)) )
  {
    //there is tag info so go through all tag entries and add tags to outstring
    pTagEntry = (PTMX_TAGENTRY)pByte;

    //start of for loop
    usStringPos = 0;
    //initial positioning st beginning of first tag entry
    usTagRecordPos = sizeof(TMX_TAGTABLE_RECORD);
    //initial stop criterium for end of for loop
    ulEnd = pTagEntry->usOffset;

    while ( fOK && (*pInString || !fNoTagsLeft) )
    {
      if ( ulEnd <=  ulInputStringLen )
      {
        for ( j = usStringPos; j < ulEnd; j++, pOutString++, pInString++ )
        {
          *pOutString = *pInString;
        } /* endfor */
      }
      else
      {
        for ( j = usStringPos; j < ulEnd; j++, pOutString++, pInString++ )
        {
          if ( j < ulInputStringLen )
          {
            *pOutString = *pInString;
          }
          else
          {
            *pOutString = ' ';
            --pInString;     // we've gone 1 too far..
          } /* endif */

          if ( pOutString >=
                  pOutStart + MAX_SEGMENT_SIZE + usStringPos - ulEnd )  //@@1
          {
            *pOutStart = EOS;
            fOK = FALSE;
            break;
          } /* endif */
        } /* endfor */
      } /* endif */

      if ( fOK && (j == ulEnd) && (!fNoTagsLeft) )
      {
        /**************************************************************/
        /* Add check that tag length does not jeopardize our          */
        /* output string...                                      @@1  */
        /**************************************************************/
        if ( (pOutString+pTagEntry->usTagLen)  < (pOutStart+MAX_SEGMENT_SIZE)
              && (pTagEntry->usTagLen < MAX_SEGMENT_SIZE)    )
        {
          memcpy( pOutString, &pTagEntry->bData, pTagEntry->usTagLen );
          pOutString += pTagEntry->usTagLen;
          ulInputStringLen += pTagEntry->usTagLen;
          //reset for loop start
          usStringPos = j + pTagEntry->usTagLen;
          //update tag table record position counter
          usTagRecordPos += sizeof(TMX_TAGENTRY) + pTagEntry->usTagLen;
          //remember length of tags added to string
          ulTagLen += pTagEntry->usTagLen;

          //move to next tag entry only if there are still tags to add to string
          if ( usTagRecordPos < RECLEN(pTMXTagTable) )
          {
            pByte += sizeof(TMX_TAGENTRY) + pTagEntry->usTagLen;
            pTagEntry = (PTMX_TAGENTRY)pByte;
            //new end criterium for for loop
            if ( pTagEntry->usOffset < ulEnd  )
            {
              // tag data seems to be corrupted, ignore the tags
              fOK = FALSE;
              memcpy( pOutStart, pInStart, *pulInStringLen );
              fNoTagsLeft = TRUE;
              pInString = pInStart + *pulInStringLen;
              ulEnd = *pulInStringLen +  ulTagLen;
            }
            else
            {
              ulEnd = pTagEntry->usOffset;
            } /* endif */
          }
          else
          {
            fNoTagsLeft = TRUE;
            //new end criterium for for loop, in this case end of string
            ulEnd = *pulInStringLen +  ulTagLen;
          } /* endif */
        }
        else
        {
          // something got corrupted -- ignore the tags ...
          fOK = FALSE;
          memcpy( pOutStart, pInStart, *pulInStringLen );
          fNoTagsLeft = TRUE;
          pInString = pInStart + *pulInStringLen;
          ulEnd = *pulInStringLen +  ulTagLen;
        } /* endif */
      } /* endif */

      if ( pOutString >= pOutStart + MAX_SEGMENT_SIZE )  //@@1
      {
        *pOutStart = EOS;
        fOK = FALSE;
        break;
      } /* endif */
    } /* endwhile */
  *pOutString = EOS;
  }
  else
  {
    //there are no tags so copy character string
    if ( *pulInStringLen < MAX_SEGMENT_SIZE )
    {
      memcpy( pOutString, pInString, *pulInStringLen );
    }
    else
    {
      fOK = FALSE;
      *pOutStart = EOS;
    } /* endif */
  } /* endif */

  return fOK;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     GetFuzzyMatch                                            |
//+----------------------------------------------------------------------------+
//|Description:       Get tm record and apply fuzzy match algorithm            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_CLB pTmClb                                          |
//|                   PTMX_SENTENCE pSentence                                  |
//|                   PTMX_GET pGetIn                                          |
//|                   PTMX_MATCH_TABLE pstMatchTable                           |
//|                   PUSHORT pusMatchesFound                                  |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT pusMatchesFound                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:     determine which tm records to consider as possible       |
//|                   fuzzy candidates                                         |
//|                   for the number of matches requested                      |
//|                     get the tm record                                      |
//|                     determine the fuzzines of each target record in the tm |
//|                     record                                                 |
// ----------------------------------------------------------------------------+

USHORT GetFuzzyMatch
(
  PTMX_CLB pTmClb,                     //ptr to ctl block struct
  PTMX_SENTENCE pSentence,             //ptr to sentence structure
  PTMX_GET_W pGetIn,                   //ptr to data in get in structure
  PTMX_MATCH_TABLE_W pstMatchTable,    //get out output structure
  PUSHORT pusMatchesFound              //number of matches found
)
{
  BOOL fOK = TRUE;                     //success indicator
  USHORT usRc = NO_ERROR;              //return code
  ULONG  ulLen;                        //length indicator
  PTMX_RECORD pTmRecord = NULL;        //pointer to tm record
  PTMX_MATCHENTRY pMatchEntry = NULL;  //pointer to match entry structure
  PTMX_MATCHENTRY pMatchStart = NULL;  //pointer to match entry structure
  USHORT usMatchEntries = 0;           //nr of matches found
  USHORT usOverlaps = 0;               //nr of overlapping triples
  ULONG  ulRecBufSize = 0L;            // current size of record buffer

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lOtherTime) );
#endif

  //allocate 32K for tm record if not done yet
  if ( pTmClb->pvTmRecord )
  {
    pTmRecord = (PTMX_RECORD)pTmClb->pvTmRecord; 
    ulRecBufSize = pTmClb->ulRecBufSize;
//    memset( pTmRecord, 0, ulRecBufSize );
  }
  else
  {
    fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );
    pTmClb->pvTmRecord = pTmRecord;
    pTmClb->ulRecBufSize = ulRecBufSize = TMX_REC_SIZE;
  } /* endif */

  //allocate for match entry
  if ( fOK ) fOK = UtlAlloc( (PVOID *) &(pMatchEntry), 0L, (LONG)(ABS_VOTES * sizeof(TMX_MATCHENTRY)), NOMSG );

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    pMatchStart = pMatchEntry;

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFuzzyOtherTime) );
#endif
    //fill pMatchEntry with the tm keys with the highest frequency
    usRc = FillMatchEntry( pTmClb, pSentence, pMatchEntry, &pGetIn->usMatchThreshold );
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFuzzyFillMatchEntry) );
#endif
    if ( usRc == NO_ERROR )
    {
      //get tm record(s)
      while ( (pMatchEntry->ulKey) && (usRc == NO_ERROR)) // &&
                                                          //(*pusMatchesFound <= pGetIn->usRequestedMatches) )
      {
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFuzzyOtherTime) );
#endif
        ulLen = ulRecBufSize;

        LOGPRINTF1( L"GetFuzzyMatch: EQFNTMGET of record %lu\r\n", pMatchEntry->ulKey );

        usRc = EQFNTMGet( pTmClb->pstTmBtree, pMatchEntry->ulKey, (PCHAR)pTmRecord, &ulLen ); 

        if ( usRc == BTREE_BUFFER_SMALL)
        {
          fOK = UtlAlloc( (PVOID *)&pTmRecord, ulRecBufSize, ulLen, NOMSG );
          if ( fOK )
          {
            pTmClb->ulRecBufSize = ulRecBufSize = ulLen;
            pTmClb->pvTmRecord = pTmRecord;
            memset( pTmRecord, 0, ulLen );
            usRc = EQFNTMGet( pTmClb->pstTmBtree, pMatchEntry->ulKey, (PCHAR)pTmRecord, &ulLen );
          }
          else
          {
            usRc = ERROR_NOT_ENOUGH_MEMORY;
          } /* endif */
        } /* endif */
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFuzzyGetTime) );
#endif
        if ( usRc == NO_ERROR )
        {
          LOGPRINTF2( L"GetFuzzyMatch: MaxVotes=%u, ActVote=%u\r\nEQFNTMGET of record %lu\r\n", pMatchEntry->usMaxVotes, pSentence->usActVote );

          //compare tm record data with data passed in the get in structure
          usOverlaps = min( pMatchEntry->usMaxVotes, pSentence->usActVote );
          usRc = FuzzyTest( pTmClb, pTmRecord, pGetIn, pstMatchTable,
                            &usMatchEntries, &usOverlaps,
                            &pMatchEntry->usMaxVotes, &pSentence->usActVote,
                            pSentence, pMatchEntry->ulKey );
          if ( !usRc )
          {
            //nr of matches found
            (*pusMatchesFound) = usMatchEntries;

            //get next tm record
            pMatchEntry++;
          } /* endif */
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFuzzyTestTime) );
#endif

        } /* endif */
      } /* endwhile */

      // limit matches to number of matches requested
      if (*pusMatchesFound > pGetIn->usRequestedMatches)
      {
        // special handling for machine matches: ensure that at least one machine match is left in the truncated list
        if ( (pstMatchTable[(*pusMatchesFound)-1].usTranslationFlag == TRANSLFLAG_MACHINE) && 
             (pstMatchTable[pGetIn->usRequestedMatches-1].usMatchLevel < 100))
        {
          memcpy( pstMatchTable + (pGetIn->usRequestedMatches-1), pstMatchTable + (*pusMatchesFound - 1),  sizeof(TMX_MATCH_TABLE_W) );
        } /* endif */           

        memset( &pstMatchTable[pGetIn->usRequestedMatches], 0, sizeof( TMX_MATCH_TABLE_W ));
        *pusMatchesFound = pGetIn->usRequestedMatches;
      }

    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pMatchStart, 0L, 0L, NOMSG );

  if ( usRc )
  {
    ERREVENT2( GETFUZZYMATCH_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFuzzyOtherTime) );
#endif

  return( usRc );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FuzzyTest                                                |
//+----------------------------------------------------------------------------+
//|Description:       The fuzzy match algorithm to determine the degree of     |
//|                   fuzziness.                                               |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_CLB pTmClb                                          |
//|                   PTMX_RECORD pTmRecord                                    |
//|                   PTMX_GET pGetIn                                          |
//|                   PTMX_MATCH_TABLE pstMatchTable                           |
//|                   PUSHORT pusMatchesFound                                  |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT pusMatchesFound                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:     position at start of first target record in tm record    |
//|                   if target language                                       |
//|                     if tag table id                                        |
//|                       fill match table and add strings with tags           |
//|                     else                                                   |
//|                       fill match table and don't add tags                  |
//|                   else                                                     |
//|                     try next target record                                 |
// ----------------------------------------------------------------------------+

USHORT FuzzyTest ( PTMX_CLB pTmClb,           //ptr to control block
                   PTMX_RECORD pTmRecord,     //ptr to tm record
                   PTMX_GET_W pGetIn,         //ptr to data in get in structure
                   PTMX_MATCH_TABLE_W pstMatchTable, //get out output structure
                   PUSHORT pusMatchesFound,   //number of matches found
                   PUSHORT pusOverlaps,       //number of triple overlaps
                   PUSHORT pusTmMaxVotes,     //nr of tm source triple
                   PUSHORT pusGetMaxVotes,    //nr of get source triples
                   PTMX_SENTENCE pSentence,   //ptr to sentence structure
                   ULONG   ulKeyNum )         // record number
{
  PBYTE pByte;                         //position ptr
  PBYTE pSource;                       //position ptr
  PBYTE pStartTarget;                  //position ptr
  PTMX_TARGET_RECORD pTMXTargetRecord = NULL;     //ptr to target record
  PTMX_TAGTABLE_RECORD pTMXTargetTagTable = NULL; //ptr to target tag record
  PTMX_SOURCE_RECORD pTMXSourceRecord = NULL; //ptr to source record
  PTMX_TARGET_CLB pTMXTargetClb = NULL;       //ptr to target control block
  PSZ_W pString = NULL;                //ptr to normalized source string
  BOOL fOK = TRUE;                     //success indicator
  ULONG ulSourceLen = 0;              //length of normalized source string
  USHORT usRc = NO_ERROR;              //return code
  USHORT usFuzzy = 0;                  //fuzzy match value
  BOOL   fStringEqual;                 // strings are equal ???
  BOOL   fNormStringEqual;             // normalized strings are equal ???
  BOOL   fRespectCRLFStringEqual = 0L;

  SHORT sLangID = MorphGetLanguageID( pGetIn->szSourceLanguage, &sLangID );

  if (!pGetIn->ulSrcOemCP)
  {
    pGetIn->ulSrcOemCP = GetLangOEMCP( pGetIn->szSourceLanguage);
  }

  LOGPRINTF1( L"FuzzyTest for record %lu\r\n", ulKeyNum );

  pusGetMaxVotes;
  //allocate pString
  fOK = UtlAlloc( (PVOID *) &(pString), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    //position at beginning of source structure in tm record
    pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);

    //move pointer to corresponding position
    pSource = (PBYTE)(pTmRecord+1);
    pSource += pTMXSourceRecord->usSource;

    //calculate length of source string
    ulSourceLen = (RECLEN(pTMXSourceRecord) -
                           sizeof(TMX_SOURCE_RECORD));

    //copy source string for fill matchtable
    ulSourceLen = EQFCompress2Unicode( pString, pSource, ulSourceLen );

    LOGPRINTF1( L"FuzzyTest: <SOURCE>\r\n%s\r\n</SOURCE>\r\n", pString );

    if (pGetIn->ulParm & GET_RESPECTCRLF )   // if-else nec for P018279
    {
	    fRespectCRLFStringEqual = (UtlCompIgnSpaceW( pString, pSentence->pNormString, 0 )== 0L);

	    fStringEqual = (UtlCompIgnWhiteSpaceW( pString, pSentence->pNormString, 0 ) == 0L);
	    if (fStringEqual && !fRespectCRLFStringEqual)
	    {  // there is a LF difference!
            fStringEqual = fRespectCRLFStringEqual;
	    }
    }
    else
    {  //compare source strings
       fStringEqual = (UtlCompIgnWhiteSpaceW( pString, pSentence->pNormString, 0 ) == 0L);
    } /* endif*/

    LOGPRINTF1( L"FuzzyTest: After String compare, fStringEqual = %d\r\n", (SHORT)fStringEqual );

    /******************************************************************/
    /* if strings equal - apply equal test first ...                  */
    /* if nothing found during equal test, try with fuzzy...          */
    /******************************************************************/
    fNormStringEqual = fStringEqual;
    if ( fStringEqual )
    {
      //string are equal so apply equal test
      USHORT  usNumMatches = *pusMatchesFound;
      usRc = ExactTest( pTmClb, pTmRecord, pGetIn, pSentence, pstMatchTable, pusMatchesFound, ulKeyNum );

      fStringEqual = ( *pusMatchesFound > usNumMatches );
      LOGPRINTF1( L"FuzzyTest: After ExactTest, fStringEqual = %d\r\n", (SHORT)fStringEqual );
    } /* endif */

    if ( !fStringEqual && !usRc )
    {
      USHORT  usTgtNum = 0;            // first target number
      ULONG   ulLeftTgtLen;            // target length record...
      USHORT  usTagId;                 // tag table id
      USHORT  usTargetId;              // target language id
      BOOL    fTagTableEqual;          // TRUE if TagTable of prop == current
      USHORT  usTagThreshold;          // set dependent of fTagTableEqual
      ULONG   ulTempSrcLen;            // temp. source length

      //get id of target tag table in the get structure
      if ( NTMGetIDFromNameEx( pTmClb, pGetIn->szTagTable,
                             NULL,
                             (USHORT)TAGTABLE_KEY, &usTagId,
                             NTMGETID_NOUPDATE_OPT, NULL ))
      {
        usTagId = 1;   // set default...
      } /* endif */

      // replace any 0xA0 in language name to 0xFF
      // (0xA0 is 0xFF after processing by OemToAnsi)
      REPLACE_A0_BY_FF( pGetIn->szTargetLanguage );

        //get id of target language in the get structure

      // we have to update the memory language table to keep the language group table up-to-date...
//      if ( NTMGetIDFromNameEx( pTmClb, pGetIn->szTargetLanguage, NULL, (USHORT)LANG_KEY, &usTargetId, NTMGETID_NOUPDATE_OPT, NULL ))
      if ( NTMGetIDFromNameEx( pTmClb, pGetIn->szTargetLanguage, NULL, (USHORT)LANG_KEY, &usTargetId, 0, NULL ))
      {
        usTargetId = 1;  // set default..
      } /* endif */


      //strings are not equal, assume fuzzy
      //position at the first target record
      pByte = (PBYTE)pTmRecord;
      pByte += pTmRecord->usFirstTargetRecord;
      pTMXTargetRecord = (PTMX_TARGET_RECORD)pByte;
      pStartTarget = (PBYTE)pTMXTargetRecord;

      //loop through target records
      ulLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;

      LOGPRINTF1( L"FuzzyTest: Checking targets, ulLeftTgtLen = %lu\r\n", ulLeftTgtLen );
      while ( ulLeftTgtLen && (RECLEN(pTMXTargetRecord) != 0) )
      {
        BOOL fTestCLB = TRUE;
        USHORT usModifiedTranslationFlag = 0;

        /**************************************************************/
        /* update left target length                                  */
        /**************************************************************/
        usTgtNum ++;            // we are dealing with next target

        assert( ulLeftTgtLen >= RECLEN(pTMXTargetRecord) );

        ulLeftTgtLen -= RECLEN(pTMXTargetRecord);

        //check the target language
        //position at target control block
        pByte += pTMXTargetRecord->usClb;
        pTMXTargetClb = (PTMX_TARGET_CLB)pByte;

        //compare target language group IDs
        if ( pTmClb->psLangIdToGroupTable[pTMXTargetClb->usLangId] != pTmClb->psLangIdToGroupTable[usTargetId] )
        {
          LOGPRINTF0( L"FuzzyTest: Wrong target language!\r\n" );
          fTestCLB = FALSE;
        } 

        // check for comments when requested
        if ( pGetIn->ulParm & GET_IGNORE_COMMENT )
        {
          if ( NtmFindInAddData( pTMXTargetClb, ADDDATA_ADDINFO_ID, L"<Note" ) )
          {
            LOGPRINTF0( L"FuzzyTest: CLB with comment skipped\r\n" );
            fTestCLB = FALSE;
          } 
        } 

        // preprocessing for global memory processing
        usModifiedTranslationFlag = pTMXTargetClb->bTranslationFlag;
        if ( fTestCLB && (pTMXTargetClb->bTranslationFlag == TRANSLFLAG_GLOBMEM) && (pGetIn->pvGMOptList != NULL) && pTMXTargetClb->usAddDataLen  )
        {
          USHORT usAddDataLen = 0;
          if ( !pSentence->pPropString ) UtlAlloc( (PVOID *) &pSentence->pPropString, 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
          usAddDataLen = NtmGetAddData( pTMXTargetClb, ADDDATA_ADDINFO_ID, pSentence->pPropString, MAX_SEGMENT_SIZE );   
          if ( usAddDataLen )
          {
            GMMEMOPT GobMemOpt = GlobMemGetFlagForProposal( pGetIn->pvGMOptList, pSentence->pPropString );
            switch ( GobMemOpt )
            {
              case GM_SUBSTITUTE_OPT: usModifiedTranslationFlag  = TRANSLFLAG_NORMAL; break;
              case GM_HFLAG_OPT     : usModifiedTranslationFlag  = TRANSLFLAG_GLOBMEM; break;
              case GM_HFLAGSTAR_OPT : usModifiedTranslationFlag  = TRANSLFLAG_GLOBMEMSTAR; break;
              case GM_EXCLUDE_OPT   : fTestCLB = FALSE;; break;
            } /* endswitch */   
          } /* endif */                   
        } /* endif */                 

        LOGPRINTF1( L"FuzzyTest: fTestCLB=%s\r\n", fTestCLB ? L"Yes" : L"NO" );

        // compare target table IDs
        if ( fTestCLB )
        {
          //compare target tag table ids
          //position at target tag table record
          pByte = pStartTarget;
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

          //position at target tag table
          pByte += pTMXTargetRecord->usTargetTagTable;
          pTMXTargetTagTable = (PTMX_TAGTABLE_RECORD)pByte;

          // GQ: only compare tag tables if segment and proposal have inline tagging
          if ( (RECLEN(pTMXTargetTagTable) > sizeof(TMX_TAGTABLE_RECORD)) ||  (RECLEN(pSentence->pTagRecord) > sizeof(TMX_TAGTABLE_RECORD)) )
          {
            fTagTableEqual =(pTMXTargetTagTable->usTagTableId == usTagId);
            // For R012645 begin
            if(!fTagTableEqual)
            {
              std::string familyNameIn = GetAndCacheFamilyName(pGetIn->szTagTable);

              if( !familyNameIn.empty() )
              {
                // get markup name firstly, also with cache
                char szMarkup[MAX_FNAME];
                memset(szMarkup,0,sizeof(szMarkup));
                NTMGetNameFromID( pTmClb, &(pTMXTargetTagTable->usTagTableId), (USHORT)TAGTABLE_KEY,szMarkup, NULL );
                  
                std::string familyNameTgt = GetAndCacheFamilyName(szMarkup);
                if( !familyNameTgt.empty() && familyNameIn==familyNameTgt)
                  fTagTableEqual = TRUE;
              } 
            }
            // For R012645 end
          }
          else
          {
            fTagTableEqual = TRUE;
          } /* endif */

          /**********************************************************/
          /* TAGS_EQUAL == base 98, TAGS_UNEQUAL == base 95         */
          /**********************************************************/
          usTagThreshold = (fTagTableEqual) ? TAGS_EQUAL : TAGS_UNEQUAL;

          pByte = pStartTarget;
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
          {
            BOOL  fFuzzynessOK;

// as the strings doe not contain the inline tagging, we have
            // to add the inline tags first
            // if the tag tables are different this is omitted as the
            // inline tags are suppressed in the proposal in such a case

            //allocate pString
            if ( !pSentence->pPropString )
            {
              fOK = UtlAlloc( (PVOID *) &pSentence->pPropString, 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
            } /* endif */

            if ( fOK )
            {
              if ( fTagTableEqual )
              {
                ULONG  ulLenTmp = ulSourceLen;
                PTMX_TAGTABLE_RECORD pTMXSourceTagTable; //ptr to source tag info
                //position at source tag table record
                pByte = pStartTarget;
                pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
                pByte += pTMXTargetRecord->usSourceTagTable;
                pTMXSourceTagTable = (PTMX_TAGTABLE_RECORD)pByte;
                fOK = AddTagsToStringW( pString, &ulLenTmp,  pTMXSourceTagTable, pSentence->pPropString );
                if ( !fOK )
                {
                  usRc = BTREE_CORRUPTED;
                } /* endif */
              }
              else
              {
                UTF16strcpy( pSentence->pPropString, pString );
              } /* endif */
            } /* endif */

            fFuzzynessOK = TMFuzzyness( pGetIn->szTagTable,
//                                        pSentence->pNormString,
                                        pSentence->pInputString,
//                                        pString,
                                        pSentence->pPropString,
                                        sLangID, &usFuzzy,
                                        pGetIn->ulSrcOemCP);
            // RJ: P018279 etc: NO, must be active!
            // GQ: the following code has been disabled as the fuzziness
            // now includes any inline tagging
            if (!fTagTableEqual  )
            {
              if ( usFuzzy > 3 )
              {
                usFuzzy -= 3;
              }
              else
              {
                usFuzzy = 0;
              } /* endif */
            } /* endif */

            /**********************************************************/
            /* additional comparison if normalized strings are equal  */
            /**********************************************************/
//            if ( usFuzzy >= 100 )
            if ( fNormStringEqual )
            {
              PTMX_TAGTABLE_RECORD pTMXSourceTagTable; //ptr to source tag info
              BOOL fStringEqual = FALSE;
              //compare source tag table records
              //position at source tag table record
              pByte = pStartTarget;
              pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

              //position at source tag table
              pByte += pTMXTargetRecord->usSourceTagTable;
              pTMXSourceTagTable = (PTMX_TAGTABLE_RECORD)pByte;

              //compare tag table records
              fStringEqual = memcmp( pTMXSourceTagTable, pSentence->pTagRecord,
                                     RECLEN(pTMXSourceTagTable) ) == 0;
              if ( !fStringEqual )
              {
                /**********************************************************/
                /* if tagging record is unequal than we have some         */
                /* (slight) differences                                   */
                /* i.e. we will create a fully qualified string and try   */
                /* another compare...                                     */
                /**********************************************************/
                //allocate pString
                if ( !pSentence->pPropString )
                {
                  fOK = UtlAlloc( (PVOID *) &pSentence->pPropString, 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
                } /* endif */

                if ( fOK )
                {
                  //calculate length of source string
                  ULONG  ulLenTmp = ulSourceLen;
                  fOK = AddTagsToStringW( pString,
                                          &ulLenTmp,      // in # of w's
                                         pTMXSourceTagTable, pSentence->pPropString );
                  if ( !fOK )
                  {
                    usRc = BTREE_CORRUPTED;
                  }
                  else
                  {
                    fStringEqual = FALSE;
                    fStringEqual = (UtlCompIgnWhiteSpaceW(pSentence->pPropString,
                                                  pSentence->pInputString,
                                                  0 ) == 0 );
                    if (!fStringEqual && (usFuzzy >= 100) )
                    {
                       // compare with respect to protected/unprotected parts
					   // We can assume all Text tokens to be equal ( usFuzzy = 100),
					   // but we do not know about tag tokens
					   BOOL fTempStringEqual = FALSE;
					   BOOL fOK = FALSE;
					   fOK = NTMCompareBetweenTokens(pSentence->pPropString,
									 pSentence->pInputString,
									 pGetIn->szTagTable,
									 sLangID,
									 pGetIn->ulSrcOemCP, &fTempStringEqual);

					   if (fOK)
					   {
						 fStringEqual = fTempStringEqual;
					   }
                    }
                  } /* endif */
                } /* endif */
              } /* endif */
              /********************************************************/
              /* ensure that we are dealing with a fuzzy match        */
              /********************************************************/
              if ( !fStringEqual )
              {
                if ( usFuzzy > 3 )
                {
                  usFuzzy -= 3;
                }
                else
                {
                  usFuzzy = 0;
                } /* endif */

                usFuzzy = min( 99, usFuzzy );
              } /* endif */

            } /* endif */
          }

          //fill get output structure
          LOGPRINTF2( L"FuzzyTest: usFuzzy=%u, fTagTableEqual=%d\r\n", usFuzzy, (SHORT)fTagTableEqual );
          if ( (usModifiedTranslationFlag == TRANSLFLAG_MACHINE) && (usFuzzy < 100) )
          {
            // ignore machine fuzzy matches
          }
          else if ( usFuzzy > TM_FUZZINESS_THRESHOLD )
          {
            /********************************************************/
            /* give MT flag a little less fuzziness                 */
            /********************************************************/
            if ( usModifiedTranslationFlag == TRANSLFLAG_MACHINE )
            {
              if ( usFuzzy > 1 )
              {
                usFuzzy -= 1;
              }
              else
              {
                usFuzzy = 0;
              } /* endif */
            } /* endif */
            if (usFuzzy == 100 && (pGetIn->ulParm & GET_RESPECTCRLF) && !fRespectCRLFStringEqual )
        		{ // P018279!
    			   usFuzzy -= 1;
		        }
            ulTempSrcLen = ulSourceLen;
            FillMatchTable( pTmClb, pString, &ulTempSrcLen, pTMXTargetRecord,
                            pTMXTargetClb,
                            pstMatchTable, &usFuzzy, fTagTableEqual,
                            pusMatchesFound, pusTmMaxVotes, pusOverlaps,
                            ulKeyNum, usTgtNum,
                            pGetIn, pSentence->pTagRecord, usModifiedTranslationFlag, 0 );
          } /* endif */
        } /* endif */
        //position at next target
        pByte = pStartTarget;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
        //move pointer to end of target
        pByte += RECLEN(pTMXTargetRecord);
        //remember the beginning of next record
        pStartTarget = pByte;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
      }/* endwhile */
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pString, 0L, 0L, NOMSG );

  if ( usRc )
  {
    ERREVENT2( FUZZYTEST_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */

  return( usRc );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FillMatchEntry                                           |
//+----------------------------------------------------------------------------+
//|Description:       Determine the sentence keys for all triple hashes        |
//|                   and their frequencies and add to match table.            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_CLB pTmClb                                          |
//|                   PTMX_SENTENCE pSentence                                  |
//|                   PTMX_MATCHENTRY pMatchEntry                              |
//|                   PUSHORT pusMatchThreshold                                |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:     for all triple hashes built for the sentence segment     |
//|                     get the index record                                   |
//|                     loop through all index entries adding the sentence     |
//|                     keys and updating their respective frequency counts    |
// ----------------------------------------------------------------------------+
USHORT FillMatchEntry
( 
  PTMX_CLB pTmClb,
  PTMX_SENTENCE pSentence,
  PTMX_MATCHENTRY pMatchEntry,
  PUSHORT pusMatchThreshold
)
{
  PTMX_INDEX_RECORD pIndexRecord = NULL;      //pointer to index record
  BOOL fOK = TRUE;                            //success indicator
  PTMX_MATCHENTRY pTempMatch = NULL;          //ptr to working match entry structure
  PTMX_MATCHENTRY pTempStart = NULL;          //ptr to working match entry structure
  USHORT usRc = NO_ERROR;                     //return code
  PTMX_INDEX_ENTRY pIndexEntry = NULL;        //pointer to index entry
  PULONG pulVotes;                   // pointer to votes
  ULONG  ulLen;                      // length paramter
  USHORT i, j;                       // index in for loop
  USHORT usMaxEntries;               // nr of index entries in index record
  ULONG  lMatchEntries = 0;             // nr of entries in pTempMatch
  ULONG ulKey;                       // index key
  USHORT usMaxSentences = MAX_INDEX_LEN * 4;
  LONG   lMatchListSize = (LONG)((usMaxSentences+1) * sizeof(TMX_MATCHENTRY));


#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lOtherTime) );
#endif

  //allocate index record buffer if not done yet
  if ( pTmClb->pvIndexRecord )
  {
    pIndexRecord = (PTMX_INDEX_RECORD)pTmClb->pvIndexRecord;
    memset( pIndexRecord, 0, TMX_REC_SIZE );
  }
  else
  {
    fOK = UtlAlloc( (PVOID *) &(pIndexRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );
    pTmClb->pvIndexRecord = pIndexRecord;
  } /* endif */

  // allocate match list array if not done yet
  if ( fOK )
  {
    if ( pTmClb->pvTempMatchList )
    {
      pTempMatch = (PTMX_MATCHENTRY)pTmClb->pvTempMatchList;
      memset( pTempMatch , 0, lMatchListSize );
    }
    else
    {
      fOK = UtlAlloc( (PVOID *) &(pTempMatch), 0L, lMatchListSize, NOMSG );
      pTmClb->pvTempMatchList = pTempMatch;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    pTempStart = pTempMatch;
  }
  else
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchAllocTime) );
#endif
  if ( fOK )
  {
    pulVotes = pSentence->pulVotes;
    lMatchEntries = 0;

    for ( i = 0; i < pSentence->usActVote; i++, pulVotes++ )
    {
      ulKey = (*pulVotes) & START_KEY;
      ulLen = TMX_REC_SIZE;
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchOtherTime) );
#endif
      usRc = EQFNTMGet( pTmClb->pstInBtree, ulKey,  (PCHAR)pIndexRecord, &ulLen ); 

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchReadTime) );
#endif
      if ( usRc == NO_ERROR )
      {
        //calculate number of index entries in index record
        LOGPRINTF1( L"Processing index record %lu\r\n", ulKey );

        ulLen = pIndexRecord->usRecordLen;
        usMaxEntries = (USHORT)((ulLen - sizeof(USHORT)) / sizeof(TMX_INDEX_ENTRY));

        pIndexEntry = &pIndexRecord->stIndexEntry;
        pTempMatch = pTempStart;

        //end criteria are all sentence ids in index key or only one
        //sentence id left in pulSids
        for ( j = 0; (j < usMaxEntries) && (lMatchEntries < usMaxSentences); j++, pIndexEntry++ )
        {
          ULONG ulIndexKey = NTMKEY(*pIndexEntry);

          // reset to begin of matching table
          pTempMatch = pTempStart;
          //before adding sentence id check if already in pulsids as the
          //respective tm record need only be checked once
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchFill1Time) );
#endif
          
          // GQ: do a binary search for the key ( bsearch() cannot be used as we need the insert position) and
          //     a linear search is far too slow 
          {
            int high = lMatchEntries;
            int low = -1;
            int probe;

            while (high - low > 1)
            {
              probe = (high + low) / 2;
              if (pTempStart[probe].ulKey > ulIndexKey)
                high = probe;
              else
                low = probe;
            } /* endwhile */

            if (low == -1 )
            {
              pTempMatch = pTempStart;
            }
            else
            {
              pTempMatch = pTempStart + low;
              if ( pTempMatch->ulKey < ulIndexKey ) pTempMatch++;
            } /* endif */
          }

          // below is the old search code
          //while ( (ulIndexKey > pTempMatch->ulKey) && pTempMatch->ulKey )
          //{
          //  pTempMatch++;
          //} /* endwhile */

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchFill2Time) );
#endif
          if ( ulIndexKey == pTempMatch->ulKey )
          {
            pTempMatch->cCount++;
          }
          else if ( !pTempMatch->ulKey )
          {
            //add new match entry
            pTempMatch->ulKey = NTMKEY(*pIndexEntry);
            pTempMatch->usMaxVotes = NTMVOTES(*pIndexEntry);
            pTempMatch->cCount = 1;
            pTempMatch->usMatchVotes = pSentence->usActVote;
            lMatchEntries++;
          }
          else
          {
            //insert in the middle of match entry structure
            if ( lMatchEntries )
            {
/*Fix*/       memmove( pTempMatch+1, pTempMatch, (lMatchEntries - (pTempMatch - pTempStart) ) * sizeof(TMX_MATCHENTRY) );
            } /* enidf */
            pTempMatch->ulKey = ulIndexKey;
            pTempMatch->usMaxVotes = NTMVOTES(*pIndexEntry);
            pTempMatch->cCount = 1;
            lMatchEntries++;
          } /* endif */
        } /* endfor */

        /**************************************************************/
        /* make clean up  if we are too near at the limit ..          */
        /**************************************************************/
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchFill3Time) );
#endif
        if (lMatchEntries >= 2 * MAX_INDEX_LEN )
        {
          PTMX_MATCHENTRY pTemp1Start;

          LOGPRINTF0( L"Cleaning up...\r\n" );

          fOK = UtlAlloc( (PVOID *) &(pTemp1Start), 0L,
                         (LONG)((usMaxSentences+1)* sizeof(TMX_MATCHENTRY)), NOMSG );
          if ( fOK )
          {
            USHORT  k = 0;
            PTMX_MATCHENTRY pTemp1Match;
            pTemp1Match = pTemp1Start;
            pTempMatch = pTempStart;

            for (k=0; k<lMatchEntries ; k++ )
            {
              if ( pTempMatch->cCount > 1 )
              {
                *pTemp1Match++ = *pTempMatch++;
              }
              else
              {
                pTempMatch++;
              } /* endif */
            } /* endfor */
/*Fix*/     memset( pTemp1Match, 0, sizeof(TMX_MATCHENTRY) );

            /**********************************************************/
            /* copy area back into orignal, adjust num entries and    */
            /* free allocation                                        */
            /**********************************************************/
            memcpy( pTempStart, pTemp1Start, (usMaxSentences+1)* sizeof(TMX_MATCHENTRY));
/*Fix*/     lMatchEntries = pTemp1Match - pTemp1Start;
            UtlAlloc( (PVOID *) &pTemp1Start, 0L, 0L, NOMSG );
          } /* endif */
        }
        else
        {
        } /* endif */
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchFill4Time) );
#endif

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchFillTime) );
#endif

      } /* endif */
    } /* endfor */

    //position at start of structure
    pTempMatch = pTempStart;

    //sort the sentence keys by frequency discard those below the given
    //threshold

    CleanupTempMatch( pTempMatch, &pMatchEntry, &pSentence->usActVote, pusMatchThreshold );
  } /* endif */

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchCleanupTime) );
#endif

  //if the entry does not exist in the index file it is not to be regarded as
  //an error
  if ( usRc == BTREE_NOT_FOUND )
  {
    usRc = NO_ERROR;
  } /* endif */

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchAllocTime) );
#endif
  if ( usRc )
  {
    ERREVENT2( FILLMATCHENTRY_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */

  LOGPRINTF2( L"FillMatchEntry: lMatchEntries=%ld, rc=%u\r\n", lMatchEntries, usRc );


  return( usRc );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CleanupTempMatch                                         |
//+----------------------------------------------------------------------------+
//|Description:       Removes all entries below calculated threshold and       |
//|                   sort sentence keys by frequency.                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_MATCHENTRY pTempMatch                               |
//|                   PTMX_MATCHENTRY *ppMatchEntry                            |
//|                   PUSHORT pusActVote                                       |
//|                   PUSHORT pusMatchThreshold                                |
//+----------------------------------------------------------------------------+
//|Output parameter:  PTMX_MATCHENTRY *ppMatchEntry                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:     ignore sentence keys that fall below calculated threshold|
//|                   and sort by frequency.                                   |
// ----------------------------------------------------------------------------+

VOID CleanupTempMatch( PTMX_MATCHENTRY pTempMatch,
                       PTMX_MATCHENTRY *ppMatchEntry,
                       PUSHORT pusActVote,
                       PUSHORT pusMatchThreshold )
{
  PTMX_MATCHENTRY pTempPos = NULL;            //pointers tp pTempMatch
  PTMX_MATCHENTRY pActualPos = NULL;          //pointers to pTempMatch
  PTMX_MATCHENTRY pMatchEntry = NULL;         //pointers to pMatchEntey
  PTMX_MATCHENTRY pMatchEntryStart = NULL;    //pointers to pMatchEntey
  USHORT usThreshold;                  //clipping factor
  USHORT usEntries;                    //nr of entries in pTempMatch
  USHORT i;                            //counter
  USHORT usEntriesInList;              //nr of entries in pTempMatch
  USHORT usActVote = *pusActVote;

  pMatchEntry = *ppMatchEntry;
  pMatchEntryStart = pMatchEntry;

  // reduce act vote if MAX_VOTES is reached (the cCount value can never reach MAX_VOTES)
  if ( usActVote >= (MAX_VOTES - 3) )
  {
    usActVote = MAX_VOTES - 3;
  } /* endif */

  //calculate threshold based on number of triple hashes found for the
  //sentence

  usThreshold =  ((usActVote) * (*pusMatchThreshold)) / 100;

  LOGPRINTF3( L"Threshhold computation, usThreshold=%u, *pusActVote=%u, *pusMatchThreshold=%u\r\n", 
              usThreshold, *pusActVote, *pusMatchThreshold );

  // if threshold is no restriction, i.e. sentence too short
  // only allow for exact match
  if ((usThreshold == 0 ) || (*pusActVote <= 3))
    usThreshold = *pusActVote;

  //eliminate all entries in pTempMatch that fall below usThreshold
  //move down the list with pTempPos and insert at pActualPos if the
  //threshold criterium holds
  pActualPos = pTempMatch;
  pTempPos = pTempMatch;
  usEntries = 0;
  LOGPRINTF0( L"CleanupTempMatch, temp matches before cleaning up\r\n" );
  while ( pTempPos->cCount )
  {
    // reduce temp act vote if MAX_VOTES is reached (the cCount value can never reach MAX_VOTES)
    USHORT usTempMaxVotes = pTempPos->usMaxVotes;

    if ( usTempMaxVotes >= (MAX_VOTES - 3) )  
    {
      usTempMaxVotes = MAX_VOTES - 3;
    } /* endif */

    LOGPRINTF3( L"ulKey=%lu usMaxVotes=%u cCount=%d\r\n", pTempPos->ulKey,
                pTempPos->usMaxVotes, pTempPos->cCount );

    /******************************************************************/
    /* check that threshold criteria is fulfilled for BOTH sources    */
    /******************************************************************/
    if ( ((USHORT)pTempPos->cCount >= usThreshold ) &&
         ((usTempMaxVotes * (*pusMatchThreshold)) <= (USHORT)pTempPos->cCount * 100 ) &&
         (pTempPos->usMaxVotes != 0) )
    {
      memcpy( pActualPos, pTempPos, sizeof(TMX_MATCHENTRY) );
      pActualPos++;
      pTempPos++;
      usEntries++;
    }
    else
    {
      pTempPos++;
    } /* endif */
  } /* endwhile */

  //sort the entries above the threshold in pTempMatch so that the entries
  //with the highest frequency are at the top
  usEntriesInList = usEntries;
  qsort( pTempMatch, usEntries, sizeof(TMX_MATCHENTRY), CompCount );
  LOGPRINTF0( L"CleanupTempMatch: Ranked entries after sort are:\r\n" );
#ifdef _DEBUG
  {
    PTMX_MATCHENTRY pTest = pTempMatch;
    for ( i = 0; i < usEntries; i++, pTest++ )
    {
      LOGPRINTF3( L"ulKey=%lu usMaxVotes=%u cCount=%d\r\n", pTest->ulKey,
                  pTest->usMaxVotes, pTest->cCount );
    } /* endfor */
  }
#endif

  // limit number to really ones of interest ( the MAX_MATCHES best )
  usEntries = min( usEntries, min(MAX_MATCHES, ABS_VOTES/2-1) );

  //fill pMatchEntry with the highest ranked entries
  pTempPos = pTempMatch;
  LOGPRINTF0( L"CleanupTempMatch: Highest ranked entries returned are:\r\n" );
  for ( i = 0; i < usEntries; i++, pMatchEntry++, pTempPos++ )
  {
    memcpy( pMatchEntry, pTempPos, sizeof(TMX_MATCHENTRY ));
    LOGPRINTF3( L"ulKey=%lu usMaxVotes=%u cCount=%d\r\n", pTempPos->ulKey,
                pTempPos->usMaxVotes, pTempPos->cCount );
  } /* endfor */


  // add highest matches by using a different metric
  qsort( pTempMatch, usEntriesInList, sizeof(TMX_MATCHENTRY), CompCountVotes );
  LOGPRINTF0( L"CleanupTempMatch: Ranked entries after sort are:\r\n" );
#ifdef _DEBUG
  {
    PTMX_MATCHENTRY pTest = pTempMatch;
    for ( i = 0; i < usEntries; i++, pTest++ )
    {
      LOGPRINTF3( L"ulKey=%lu usMaxVotes=%u cCount=%d\r\n", pTest->ulKey,
                  pTest->usMaxVotes, pTest->cCount );
    } /* endfor */
  }
#endif

  // copy new possible match entry into list
  pTempPos = pTempMatch;
  for (i = 0; i < usEntries; i++, pTempPos++)
  {
    pMatchEntry = pMatchEntryStart;

    while ( memcmp( pMatchEntry, pTempPos, sizeof( TMX_MATCHENTRY) ) != 0 && pMatchEntry->cCount )
    {
      pMatchEntry++;
    }
    if (pMatchEntry->cCount == 0)
    {
      memcpy( pMatchEntry, pTempPos, sizeof( TMX_MATCHENTRY ));
    }
  }

  if ( usEntriesInList > usEntries )
  {
    // check if the following matches have same quality... if so
  // we have to use them too
    PTMX_MATCHENTRY pTest = pTempPos-1;
    while ( (pTempPos->usMaxVotes == pTest->usMaxVotes)  &&
        (pTempPos->cCount == pTest->cCount) &&
      (usEntries < min(ABS_VOTES/2-1,usEntriesInList)))
  {
      pMatchEntry = pMatchEntryStart;

      while ( memcmp( pMatchEntry, pTempPos, sizeof( TMX_MATCHENTRY) ) != 0 && pMatchEntry->cCount )
    {
        pMatchEntry++;
    }
      if (pMatchEntry->cCount == 0)
    {
        memcpy( pMatchEntry, pTempPos, sizeof( TMX_MATCHENTRY ));
    }
      // point to next entry
    pTempPos++;
    usEntries++;
  }

  }
  *ppMatchEntry = pMatchEntryStart;
}



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CompCount                                                |
//+----------------------------------------------------------------------------+
//|Description:       Compare the passed frequencies.                          |
//|                   This function is called by qsort.                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_MATCHENTRY pEntry1   pointer to first frequency     |
//|                   PTMX_MATCHENTRY pEntry2   pointer to second frequency    |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       >0   Entry1 > Entry2                                     |
//|                   =0   Entry1 = Entry2                                     |
//|                   <0   Entry1 < Entry2                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     compare the frequencies in matchentry and return the     |
//|                   comparison value                                         |
// ----------------------------------------------------------------------------+
// Note: This comparison does not fit too good under some circumstances.
//       But to be upward compatible it is still in use.
//       A new comparison function is added which adds another metric for sorting.
//       This new metric looks a lot more intuitive.
//       The result of both sort orders are merged, hence we are only getting better
//       results.
int
CompCount
(
  const void * pvEntry1,  // pointer to first frequency
  const void * pvEntry2   // pointer to second frequency
)
{
//  SHORT sRc;                           // return code
  PTMX_MATCHENTRY pEntry1 = (PTMX_MATCHENTRY)pvEntry1;
  PTMX_MATCHENTRY pEntry2 = (PTMX_MATCHENTRY)pvEntry2;

  SHORT  sCount1 = (SHORT) pEntry1->cCount ;
  SHORT  sCount2 = (SHORT) pEntry2->cCount ;

  if ( (pEntry1->usMaxVotes != 0) && (pEntry2->usMaxVotes != 0) )
  {
    sCount1 = (sCount1 << 8) / (SHORT)pEntry1->usMaxVotes;
    sCount2 = (sCount2 << 8) / (SHORT)pEntry2->usMaxVotes;
  } /* endif */     

  return ( sCount2 - sCount1 );

} /* end of function CompCount */

// Sort the possible candidates according to their maxVotes
int
CompCountVotes
(
  const void * pEntry1,  // pointer to first frequency
  const void * pEntry2   // pointer to second frequency
)
{
  int iResult;
  iResult = abs(((PTMX_MATCHENTRY)pEntry1)->usMaxVotes - ((PTMX_MATCHENTRY)pEntry1)->usMatchVotes) -
               abs(((PTMX_MATCHENTRY)pEntry2)->usMaxVotes - ((PTMX_MATCHENTRY)pEntry2)->usMatchVotes);
  if (iResult == 0)
  {
    iResult =  ((PTMX_MATCHENTRY)pEntry2)->usMaxVotes - ((PTMX_MATCHENTRY)pEntry1)->usMaxVotes;
  }
  if (iResult == 0)
  {
    iResult = ((PTMX_MATCHENTRY)pEntry2)->cCount - ((PTMX_MATCHENTRY)pEntry1)->cCount;
  }
  return iResult;

} /* end of function CompCount */



// helper function for token logging
#ifdef INLINE_TAG_REPL_LOGGING

void NTMListToken
( 
  FILE        *hfLog, 
  PSZ          pszToken,
  PFUZZYTOK    pToken 
)
{
  PSZ pszType = NULL;
  CHAR szType[6];
  int len = pToken->usStop - pToken->usStart + 1;
  CHAR_W chTemp = pToken->pData[len];
  pToken->pData[len] = 0;
  if ( pToken->sType == -1 )
  {
    pszType = "Text";
  }
  else if ( pToken->sType == -7 )
  {
    pszType = "Tag";
  }
  else 
  {
    itoa( pToken->sType, szType, 10 );
    pszType = szType;
  } /* endif */
  fwprintf( hfLog, L"%-12S : Hash=%12lu  Length=%3ld  Type=%-5S Connected=%-3s Data=\"%s\"\r\n", pszToken, 
           pToken->ulHash, len, pszType, ( pToken->fConnected ) ? L"Yes" : L"No", pToken->pData );
  pToken->pData[len] = chTemp;
} /* end of function NTMListToken */

void NTMListTokens
( 
  FILE        *hfLog, 
  PSZ          pszName,
  PFUZZYTOK    pToken 
)
{
  int i = 1;
  fwprintf( hfLog, L"\r\n%S token list\r\n", pszName ); 
  while ( pToken->ulHash )
  {
    CHAR szTokenName[20];
    sprintf( szTokenName, "Token %2ld", i ); 
    NTMListToken( hfLog, szTokenName, pToken ); 
    pToken++; i++;
  } /*endwhile */
} /* end of function NTMListTokens */

#endif



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMTagSubst                                              |
//+----------------------------------------------------------------------------+
//|Function call:     NTMTagSubst( pSubstProp );                               |
//+----------------------------------------------------------------------------+
//|Description:       GENERIC TM: substitute tags if substitution of all       |
//|                   is possible                                              |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_SUBSTPROP pSubstProp ptr to substitution struct     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       1 all tagging in proposal replaced                       |
//|                   0        nothing replaced                                |
//+----------------------------------------------------------------------------+
//|Function flow:     Prepare a token list for current src, src of prop and    |
//|                      tgt of prop                                           |
//|                   call LCS for data pair src - src of prop                 |
//|                   call LCS for data pair src of prop - tgt of prop         |
//|                   determine which tags can be replaced                     |
//|                   if so,                                                   |
//|                     if all tags can be replaced                            |
//|                       replace them                                         |
// ----------------------------------------------------------------------------+
BOOL     NTMTagSubst
(
  PTMX_SUBSTPROP pSubstProp,
  ULONG          ulSrcOemCP,
  ULONG          ulTgtOemCP
)
{
  BOOL      fOK;
  PREPLLIST pReplaceList       = NULL;
  PREPLLIST pReplaceSourceList = NULL;
  PFUZZYTOK pCopyTokList1      = NULL;
  PFUZZYTOK pCopyTokList2      = NULL;
  PFUZZYTOK pTempList;
  PBYTE     pTokBuf = NULL;
  USHORT    k;                            //index variable
  EQF_BOOL  fConnected;                          // temp variable for connection state
  BOOL      fReplaced = FALSE;
  BOOL      fDelete   = FALSE;
  SHORT     sSrcLangID = 0;
  SHORT     sTgtLangID = 0;
  PSZ_W     pInBuf;
#ifdef INLINE_TAG_REPL_LOGGING
  FILE      *hfLog = NULL;
  static CHAR_W szSegBuf[4096];
#endif
  PLOADEDTABLE pTagTable = NULL;
  BOOL      fClsTranslVarChecking = FALSE;

  // ignore LFs in inline tagging only for IBMDITA markup
  BOOL       fRespectLFs = (_stricmp( pSubstProp->szSourceTagTable, "IBMDITA" ) != 0);

  // load tag table for the proposal
  if ( TALoadTagTableExHwnd( pSubstProp->szPropTagTable, &pTagTable, FALSE, 0, FALSE, NULLHANDLE ) != 0 )
  {
    // no tag replacement possible as tag table can't be loaded
    return( FALSE );
  } /* endif */

  fClsTranslVarChecking  = ContainsClassID( pTagTable, CLS_TRANSLVAR );

  UtlAlloc((PVOID *) &pInBuf, 0L, (LONG) IO_BUFFER_SIZE * sizeof(CHAR_W), ERROR_STORAGE );
  UtlAlloc((PVOID *) &pTokBuf, 0L, (LONG) TOK_BUFFER_SIZE, ERROR_STORAGE );
  fOK = ( pInBuf && pTokBuf ) ;

#ifdef INLINE_TAG_REPL_LOGGING
  {
    BOOL fLogExists = FALSE;
    CHAR szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    strcat( szLogFile, "\\INLINETAGREPL.LOG" );
    fLogExists = UtlFileExist( szLogFile );
    hfLog = fopen( szLogFile, "ab" );
    if ( hfLog )
    {
      if ( !fLogExists )
      {
        fwrite( UNICODEFILEPREFIX, 1, 2, hfLog );
      } /* endif */         
      fwprintf( hfLog, L"*** NTMTagSubst ***\r\n" );
      NTMMarkCRLF( pSubstProp->szSource, szSegBuf );
      fwprintf( hfLog, L"<Segment>%s</Segment>\r\n", szSegBuf );
      NTMMarkCRLF( pSubstProp->szPropSource, szSegBuf );
      fwprintf( hfLog, L"<PropSource>%s</PropSource>\r\n", szSegBuf );
      NTMMarkCRLF( pSubstProp->szPropTarget, szSegBuf );
      fwprintf( hfLog, L"<PropTarget>%s</PropTarget>\r\n", szSegBuf );
    } /* endif */
  }
#endif

  /******************************************************************/
  /* prepare token structure                                        */
  /******************************************************************/
  if (fOK )
  {
    fOK = (MorphGetLanguageID(pSubstProp->szSourceLanguage, &sSrcLangID )==MORPH_OK);
  } /* endif */

  if (fOK && (MorphGetLanguageID( pSubstProp->szTargetLanguage,
              &sTgtLangID ) != MORPH_OK) )
  {
     sTgtLangID = sSrcLangID ;
  } /* endif */

  if (fOK )
  {
    fOK = NTMPrepareTokens( pSubstProp, pSubstProp->szSource,
                            (PFUZZYTOK *)&pSubstProp->pTokSource,
                            &pSubstProp->usTokenSource,
                            pSubstProp->pTagsSource, sSrcLangID, ulSrcOemCP, fClsTranslVarChecking ? pTagTable : NULL );
#ifdef INLINE_TAG_REPL_LOGGING
    if ( fOK ) NTMListTokens( hfLog, "Source", (PFUZZYTOK)pSubstProp->pTokSource );
#endif
  } /* endif */

  if ( fOK )
  {
    fOK = NTMPrepareTokens( pSubstProp, pSubstProp->szPropSource,
                            (PFUZZYTOK *)&pSubstProp->pTokPropSource,
                            &pSubstProp->usTokenPropSource,
                            pSubstProp->pTagsPropSource, sSrcLangID, ulSrcOemCP, fClsTranslVarChecking ? pTagTable : NULL  );
#ifdef INLINE_TAG_REPL_LOGGING
    if ( fOK ) NTMListTokens( hfLog, "PropSource", (PFUZZYTOK)pSubstProp->pTokPropSource );
#endif
  } /* endif */

  if ( fOK )
  { // P018276: use TgtOemCP!! (must fit to sTgtLangID)
    fOK = NTMPrepareTokens( pSubstProp, pSubstProp->szPropTarget,
                            (PFUZZYTOK *)&pSubstProp->pTokPropTarget,
                            &pSubstProp->usTokenPropTarget,
                            pSubstProp->pTagsPropTarget, sTgtLangID, ulTgtOemCP,  fClsTranslVarChecking ? pTagTable : NULL );
#ifdef INLINE_TAG_REPL_LOGGING
    if ( fOK ) NTMListTokens( hfLog, "PropTarget", (PFUZZYTOK)pSubstProp->pTokPropTarget );
#endif
  } /* endif */

  if ( fOK )
  {
    /******************************************************************/
    /* temporary store values                                         */
    /******************************************************************/
    UtlAlloc( (PVOID *) &pCopyTokList1, 0L, (LONG)(pSubstProp->usTokenPropSource+1)*sizeof(FUZZYTOK), NOMSG );
    UtlAlloc( (PVOID *) &pCopyTokList2, 0L, (LONG)(pSubstProp->usTokenPropTarget+1)*sizeof(FUZZYTOK), NOMSG );
    fOK = ( pCopyTokList1 && pCopyTokList2 );
  } /* endif */


  if ( fOK )
  {
    memcpy( pCopyTokList1, pSubstProp->pTokPropSource, pSubstProp->usTokenPropSource * sizeof(FUZZYTOK));
    memcpy( pCopyTokList2, pSubstProp->pTokPropTarget, pSubstProp->usTokenPropTarget * sizeof(FUZZYTOK));

    /******************************************************************/
    /* now do the alignments....                                      */
    /******************************************************************/
//#define USEOLDALIGN

#ifdef USEOLDALIGN
    fOK = (BOOL)FuzzyLCSReplList( (PFUZZYTOK)pSubstProp->pTokPropSource, (PFUZZYTOK)pSubstProp->pTokPropTarget, &pReplaceList,
                                  pSubstProp->usTokenPropSource, pSubstProp->usTokenPropTarget, NULL, TRUE );
#else
    fOK = NTMAlignTags( (PFUZZYTOK)pSubstProp->pTokPropSource, (PFUZZYTOK)pSubstProp->pTokPropTarget, &pReplaceList );
#endif
    /******************************************************************/
    /* restore original content, but keep fConnected...               */
    /* The original content is modified in the FuzzyLCSReplList func. */
    /******************************************************************/
    pTempList = (PFUZZYTOK) pSubstProp->pTokPropSource;
    for ( k=0; k < pSubstProp->usTokenPropSource+1; k++ )
    {
      fConnected            = pTempList->fConnected;
      *pTempList            = *(pCopyTokList1+k);
      pTempList->fConnected = fConnected;
      pTempList++;
    } /* endfor */

    pTempList = (PFUZZYTOK) pSubstProp->pTokPropTarget;
    for ( k=0; k < pSubstProp->usTokenPropTarget+1; k++ )
    {
      fConnected            = pTempList->fConnected;
      *pTempList            = *(pCopyTokList2+k);
      pTempList->fConnected = fConnected;
      pTempList++;
    } /* endfor */
  #ifdef INLINE_TAG_REPL_LOGGING
      NTMListTokens( hfLog, "Connected PropSource", (PFUZZYTOK)pSubstProp->pTokPropSource );
      NTMListTokens( hfLog, "Connected PropTarget", (PFUZZYTOK)pSubstProp->pTokPropTarget );
  #endif

    /******************************************************************/
    /* free CopyTokList2                                              */
    /******************************************************************/
    UtlAlloc( (PVOID*)&pCopyTokList2, 0L, 0L, NOMSG);
  } /* endif */

  if ( fOK && pReplaceList )
  {
    /******************************************************************/
    /* temporary store values                                         */
    /******************************************************************/
    UtlAlloc( (PVOID *) &pCopyTokList2, 0L, (LONG)(pSubstProp->usTokenSource+1)*sizeof(FUZZYTOK), NOMSG );
    fOK = ( pCopyTokList2 != NULL );
  } /* endif */

  if ( fOK && pReplaceList )
  {
    memcpy( pCopyTokList2, pSubstProp->pTokSource, pSubstProp->usTokenSource * sizeof(FUZZYTOK));
    fOK = (BOOL)FuzzyLCSReplList( (PFUZZYTOK)pSubstProp->pTokPropSource,
                                  (PFUZZYTOK)pSubstProp->pTokSource,
                                  &pReplaceSourceList,
                                  pSubstProp->usTokenPropSource,
                                  pSubstProp->usTokenSource,
                                  NULL, TRUE );
    /******************************************************************/
    /* restore original content, but keep fConnected...               */
    /******************************************************************/
    pTempList = (PFUZZYTOK) pSubstProp->pTokPropSource;
    for ( k=0; k < pSubstProp->usTokenPropSource+1; k++ )
    {
      fConnected            = pTempList->fConnected;
      *pTempList            = *(pCopyTokList1+k);
      pTempList->fConnected = fConnected;
      pTempList++;
    } /* endfor */

    pTempList = (PFUZZYTOK) pSubstProp->pTokSource;
    for ( k=0; k < pSubstProp->usTokenSource+1; k++ )
    {
      fConnected            = pTempList->fConnected;
      *pTempList            = *(pCopyTokList2+k);
      pTempList->fConnected = fConnected;
      pTempList++;
    } /* endfor */
  #ifdef INLINE_TAG_REPL_LOGGING
      if ( hfLog ) fwprintf( hfLog, L"after FuzzyLCSReplList on source and proposal source\r\n" );
      NTMListTokens( hfLog, "Connected Source", (PFUZZYTOK)pSubstProp->pTokSource );
      NTMListTokens( hfLog, "Connected PropSource", (PFUZZYTOK)pSubstProp->pTokPropSource );
  #endif

  } /* endif */

  /********************************************************************/
  /* find out which tags can be replaced by each other                */
  /********************************************************************/
  if ( fOK && pReplaceSourceList )
  {
#ifdef INLINE_TAG_REPL_LOGGING
    if ( hfLog ) fwprintf( hfLog, L"replace source list exists\r\n" );
#endif

    /*************************************************************/
    /* insert a dummy token at the beginning of the pReplaceSourcList */
    /* list to have an anchor where to start from...             */
    /*************************************************************/
    memmove( pReplaceSourceList+1, pReplaceSourceList, sizeof( REPLLIST ) * MAX_REPL);

    pReplaceSourceList->pSrcTok = (PFUZZYTOK)pSubstProp->pTokPropSource;
    pReplaceSourceList->pTgtTok = (PFUZZYTOK)pSubstProp->pTokSource;

    fReplaced = NTMFuzzyReplace ( pSubstProp, pSubstProp->szSource,
                                  pSubstProp->szPropSource,
                                  pSubstProp->szPropTarget,
                                  pReplaceSourceList, pReplaceList
#ifdef INLINE_TAG_REPL_LOGGING
                                  , hfLog
#endif
                                  );

#ifdef INLINE_TAG_REPL_LOGGING
    if ( hfLog ) fwprintf( hfLog, L"After NTMFuzzyReplace: fReplaced = %s\r\n", ( fReplaced ) ? L"True" : L"False" );
#endif

    if ( fReplaced )
    {
      /****************************************************************/
      /* check whether all tags can be replaced                       */
      /****************************************************************/
#ifdef INLINE_TAG_REPL_LOGGING
      if (NTMCheckTagPairs(pSubstProp, pReplaceSourceList, pReplaceList, fRespectLFs, hfLog ))
#else
      if (NTMCheckTagPairs(pSubstProp, pReplaceSourceList, pReplaceList, fRespectLFs ))
#endif
      {
        /**************************************************************/
        /* replace all tags possible                                  */
        /**************************************************************/
        fReplaced = NTMReplaceTags(pSubstProp, fRespectLFs );
#ifdef INLINE_TAG_REPL_LOGGING
        if ( hfLog ) 
        {
          fwprintf( hfLog, L"After NTMReplaceTags: fReplaced = %s\r\n", ( fReplaced ) ? L"True" : L"False" );
          if ( fReplaced )
          {
            NTMMarkCRLF( pSubstProp->szPropSource, szSegBuf );
            fwprintf( hfLog, L"<New PropSource>%s</PropSource>\r\n", szSegBuf );
            NTMMarkCRLF( pSubstProp->szPropTarget, szSegBuf );
            fwprintf( hfLog, L"<New PropTarget>%s</PropTarget>\r\n", szSegBuf );
          } /* endif */
        }
#endif
      }
      else
      {
        fReplaced = FALSE;
#ifdef INLINE_TAG_REPL_LOGGING
        if ( hfLog ) fwprintf( hfLog, L"After NTMCheckTagPairs: fReplaced = %s\r\n", ( fReplaced ) ? L"True" : L"False" );
#endif
      } /* endif */
    } /* endif */
  } /* endif */

  // perform tag deletion
  if ( fOK && !fReplaced )
  {
    fDelete = NTMGenericDelete ( pSubstProp, pSubstProp->szSource,
                                  pSubstProp->szPropSource,
                                  pSubstProp->szPropTarget,
                                  pReplaceSourceList, pReplaceList
#ifdef INLINE_TAG_REPL_LOGGING
                                    , hfLog
#endif
                                    );
#ifdef INLINE_TAG_REPL_LOGGING
      if ( hfLog ) fwprintf( hfLog, L"After NTMGenericDelete : fDeleted = %s\r\n", ( fDelete ) ? L"True" : L"False" );
#endif

      if ( fDelete )
      {
#ifdef INLINE_TAG_REPL_LOGGING
        // list tokens
        PREPLLIST pRepl = pReplaceSourceList;
        if ( hfLog ) fwprintf( hfLog, L"List of same tokens in source and source of proposals\r\n" );
        while ( pRepl->pSrcTok)
        {
          NTMListToken( hfLog, "SourceToken", pRepl->pSrcTok );
          NTMListToken( hfLog, "SourceOfPropToken", pRepl->pTgtTok );
          pRepl++;
        } /*endwhile */

        pRepl = pReplaceList;
        if ( hfLog ) fwprintf( hfLog, L"List of same tokens in source of proposals and target of proposal\r\n" );
        while ( pRepl->pSrcTok)
        {
          NTMListToken( hfLog, "SourcePropToken", pRepl->pSrcTok );
          NTMListToken( hfLog, "TargetPropToken", pRepl->pTgtTok );
          pRepl++;
        } /*endwhile */
#endif
#ifdef INLINE_TAG_REPL_LOGGING
        if ( hfLog ) fwprintf( hfLog, L"\nChecking and deleting tag pairs with NTMCheckAndDeleteTagPairs\r\n" );
        fReplaced = NTMCheckAndDeleteTagPairs(pSubstProp, pReplaceSourceList, pReplaceList, fRespectLFs, hfLog );
#else
        fReplaced = NTMCheckAndDeleteTagPairs(pSubstProp, pReplaceSourceList, pReplaceList, fRespectLFs );
#endif
#ifdef INLINE_TAG_REPL_LOGGING
        if ( hfLog ) fwprintf( hfLog, L"NTMCheckAndDeleteTagPairs returned %s\r\n", fReplaced ? L"TRUE" : L"FALSE" );
#endif
      } /* end */       
  } /* end */     

  // add tagging function of generic inline tag replacement
  if ( fOK && !fReplaced )
  {
    BOOL fAddTags = TRUE;
    PSZ_W pSegmentEndTags = NULL;         // ptr to segment end tagging
    PSZ_W pSegmentText = NULL;            // ptr to segment text portion

    #define TOKEN_TYPE_TAG  -7
    #define TOKEN_TYPE_TEXT -1

    // 1. check that proposal does not have any inline tagging
    if ( fAddTags )
    {
        PFUZZYTOK pToken = (PFUZZYTOK)pSubstProp->pTokPropSource; 
        while ( pToken->ulHash )
        {
          if ( pToken->sType != TOKEN_TYPE_TEXT )
          {
            fAddTags = FALSE;
          } /* endif */
          pToken++;
        } /*endwhile */
    } /* endif */

    // 2. check that source has tagging only at the beginning and at the end of the segment
    if ( fAddTags )
    {
        PFUZZYTOK pToken = (PFUZZYTOK)pSubstProp->pTokSource; 

        // handle tags at segment start
        if ( pToken->sType == TOKEN_TYPE_TAG )
        {
          while ( pToken->ulHash && (pToken->sType == TOKEN_TYPE_TAG) )
          {
            pToken++;
          } /*endwhile */
        }
        else
        {
          // no tags at segment start
          fAddTags = FALSE;
        } /* endif */

        // handle text part of segment
        if ( pToken->ulHash &&(pToken->sType == TOKEN_TYPE_TEXT) )
        {
          pSegmentText = pToken->pData;         // remember start of text portion

          while ( pToken->ulHash && (pToken->sType == TOKEN_TYPE_TEXT) )
          {
            pToken++;
          } /*endwhile */
        }
        else
        {
          // no text portion following tags
          fAddTags = FALSE;
        } /* endif */

        // handle tags at segment end
        if ( pToken->ulHash &&(pToken->sType == TOKEN_TYPE_TAG) )
        {
          pSegmentEndTags = pToken->pData;         // remember start of tagging at segment end

          while ( pToken->ulHash && (pToken->sType == TOKEN_TYPE_TAG) )
          {
            pToken++;
          } /*endwhile */

          if ( pToken->ulHash && (pToken->pData[0] != 0) )
          {
            // somethig is following tagging at segment end
            fAddTags = FALSE;
          } /* endif */
        }
        else
        {
          // no tagging at segment end
          fAddTags = FALSE;
        } /* endif */
    } /* endif */

    // 3. check that proposal text and segment text is identical
    if ( fAddTags )
    {
        PFUZZYTOK    pToken = (PFUZZYTOK)pSubstProp->pTokSource; 
        PFUZZYTOK    pPropToken = (PFUZZYTOK)pSubstProp->pTokPropSource; 
        while ( pToken->ulHash && (pToken->pData[0] != 0) && pPropToken->ulHash && (pPropToken->pData[0] != 0))
        {
          // skip any tags
          while ( (pToken->ulHash && (pToken->pData[0] != 0)) && (pToken->sType == TOKEN_TYPE_TAG) )
          {
            pToken++;
          } /*endwhile */

          if ( !pToken->ulHash || (pToken->pData[0] == 0))
          {
            // end of source segment
            fAddTags = FALSE;
          } 
          else if ( (pToken->sType == TOKEN_TYPE_TEXT) && (pPropToken->sType == TOKEN_TYPE_TEXT ) )
          {
            if ( wcsncmp( pToken->pData, pPropToken->pData, pToken->usStop - pToken->usStart + 1 ) != 0 )
            {
              // text does not match
              fAddTags = FALSE;
            } /* endif */
          }
          else
          {
            // not same number of text tokens
            fAddTags = FALSE;
          } /* endif */

          // next token
          if ( pToken->ulHash ) pToken++;
          if ( pPropToken->ulHash ) pPropToken++;
        } /*endwhile */
    } /* endif */

    // 4. check that source segment tagging is of correct type
    if ( fAddTags )
    {
        PFUZZYTOK    pToken = (PFUZZYTOK)pSubstProp->pTokSource; 

        if ( (pToken->pData[0] != L'<') || (pSegmentEndTags[0] != L'<') || (pSegmentEndTags[1] != L'/') )
        {
          // incorrect type of tagging
          fAddTags = FALSE;
        }
        else
        {
          // test characters of start and end tag 
          PSZ_W pszStartTag = pToken->pData + 1;
          PSZ_W pszEndTag = pSegmentEndTags + 2;
          while ( fAddTags && iswalpha(*pszEndTag) )
          {
            if ( *pszStartTag == *pszEndTag )
            {
              pszStartTag++;
              pszEndTag++;
            }
            else
            {
              // tag names do not match
              fAddTags = FALSE;
            } /* endif */
          } /*endwhile */
        } /* endif */
    } /* endif */


    // add the tags when every test succeeded 
    if ( fAddTags )
    {
      PFUZZYTOK pToken = (PFUZZYTOK)pSubstProp->pTokSource; 
      PSZ_W  pStartTag = pToken->pData;
      int iStartTagLen = pSegmentText - pStartTag;
      int iEndTagLen = wcslen( pSegmentEndTags );
      int iSourceTextLen = wcslen(pSubstProp->szPropSource);
      int iTargetTextLen = wcslen(pSubstProp->szPropTarget);

      if ( ( (iStartTagLen + iSourceTextLen + iEndTagLen) < MAX_SEGMENT_SIZE) &&
           ( (iStartTagLen + iTargetTextLen + iEndTagLen) < MAX_SEGMENT_SIZE) )
      {
        // change proposal source
        memmove( pSubstProp->szPropSource + iStartTagLen, pSubstProp->szPropSource, (iSourceTextLen + 1) * sizeof(CHAR_W) );
        memcpy( pSubstProp->szPropSource, pStartTag, iStartTagLen * sizeof(CHAR_W) );
        wcscpy( pSubstProp->szPropSource + wcslen(pSubstProp->szPropSource), pSegmentEndTags );

        // change proposal target
        memmove( pSubstProp->szPropTarget + iStartTagLen, pSubstProp->szPropTarget, (iTargetTextLen+1) * sizeof(CHAR_W) );
        memcpy( pSubstProp->szPropTarget, pStartTag, iStartTagLen * sizeof(CHAR_W) );
        wcscpy( pSubstProp->szPropTarget + wcslen(pSubstProp->szPropTarget), pSegmentEndTags );

        fReplaced = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* free allocated resources                                         */
  /********************************************************************/
  if ( pCopyTokList1 )
     UtlAlloc( (PVOID*)&pCopyTokList1, 0L, 0L, NOMSG);
  if ( pCopyTokList2 )
     UtlAlloc( (PVOID*)&pCopyTokList2, 0L, 0L, NOMSG);
  if ( pSubstProp->pTokSource )
     UtlAlloc( (PVOID*)&pSubstProp->pTokSource, 0L, 0L, NOMSG);
  if ( pSubstProp->pTokPropSource )
     UtlAlloc( (PVOID*)&pSubstProp->pTokPropSource, 0L, 0L, NOMSG);
  if ( pSubstProp->pTokPropTarget )
     UtlAlloc( (PVOID*)&pSubstProp->pTokPropTarget, 0L, 0L, NOMSG);
  if (pInBuf )
     UtlAlloc((PVOID *) &pInBuf, 0L, 0L, NOMSG );
  if (pTokBuf )
     UtlAlloc((PVOID *) &pTokBuf, 0L, 0L, NOMSG );
  if (pReplaceList )
    UtlAlloc((PVOID *) &pReplaceList, 0L, 0L, NOMSG);
  if (pReplaceSourceList )
    UtlAlloc((PVOID *) &pReplaceSourceList, 0L, 0L, NOMSG);

  if ( pTagTable ) TAFreeTagTable( pTagTable );


#ifdef INLINE_TAG_REPL_LOGGING
  if ( hfLog ) fclose( hfLog );
#endif

  return (fReplaced);

}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMPrepareTokens                                         |
//+----------------------------------------------------------------------------+
//|Function call:     BOOL NTMPrepareTokens                                    |
//|                   ( PTMX_SUBSTPROP       pSubstProp,                       |
//|                     PSZ                  pData,                            |
//|                     PFUZZYTOK           *ppTokData,                        |
//|                     PUSHORT              pusToken,                         |
//|                     PTMX_TAGTABLE_RECORD pTagRecord,                       |
//|                      SHORT                sLangID )                        |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_SUBSTPROP       pSubstProp,                         |
//|                   PSZ                  pData,                              |
//|                   PFUZZYTOK           *ppTokData,                          |
//|                   PUSHORT              pusToken,                           |
//|                   PTMX_TAGTABLE_RECORD pTagRecord,                         |
//|                   SHORT                sLangID                             |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
// ----------------------------------------------------------------------------+
#define TAG_TOKEN   -7

static
BOOL  isWhiteSpaceW( PSZ_W pData )
{
  PSZ_W pTemp = pData;
  CHAR_W c;
  while ( ((c = *pTemp)!= NULC) && iswspace( c ) )
  {
    pTemp++;
  } /* endwhile */
  return (*pTemp == EOS);
}


static
BOOL NTMPrepareTokens
(
  PTMX_SUBSTPROP       pSubstProp,
  PSZ_W                pData,
  PFUZZYTOK           *ppTokData,
  PUSHORT              pusToken,
  PTMX_TAGTABLE_RECORD pTagRecord,
  SHORT                sLangID,
  ULONG                ulOemCP,
  PLOADEDTABLE         pTagTable
)
{
  BOOL      fOK = TRUE;
  USHORT    usStringPos;
  USHORT    usTagRecordPos;   // in # of bytes

  PSZ_W         pTokBuf  = pSubstProp->chBuffer;
  PFUZZYTOK     pCurrent = (PFUZZYTOK)pTokBuf;
  ULONG         ulCurStop;
  BOOL          fEndReached = FALSE;
  USHORT        usRandomIndex;           // index in random sequence
  PTMX_TAGENTRY pTagEntry = NULL;      //ptr to tag entries in tag table record
  PBYTE         pByte = (PBYTE)pTagRecord;
  USHORT        usMaxToksInBuffer = (sizeof(pSubstProp->chBuffer) /
                                      sizeof(FUZZYTOK)) - 2;


  if ( !pSubstProp->ulRandom[0] )
  {
    /********************************************************************/
    /* random sequences, see e.g. the book of Wirth...                  */
    /********************************************************************/
    pSubstProp->ulRandom[0] = 0xABCDEF01;
    for (usRandomIndex = 1; usRandomIndex < MAX_RANDOM; usRandomIndex++)
    {
        pSubstProp->ulRandom[usRandomIndex] =
                   pSubstProp->ulRandom[usRandomIndex - 1] * 5 + 0xABCDEF01;
    }  /* endfor*/
  } /* endif */

  // usFirstTagEntry is in number of bytes!
  pByte += pTagRecord->usFirstTagEntry;

  //there is tag info so go through all tag entries and add tags to outstring
  pTagEntry = (PTMX_TAGENTRY)pByte;

  //start of for loop
  usStringPos = 0;
  //initial positioning st beginning of first tag entry
  // in # of bytes
  usTagRecordPos = sizeof(TMX_TAGTABLE_RECORD);

  /********************************************************************/
  /* add first entry                                                  */
  /********************************************************************/
  memset( pTokBuf, 0, sizeof( pSubstProp->chBuffer ));

  if ( usTagRecordPos < RECLEN(pTagRecord) )
  {
    if (pTagEntry->usOffset )
    {
      ulCurStop = pTagEntry->usOffset - 1;
      pCurrent = NTMSplitAndAddTokens(pSubstProp, pCurrent,
                                    &usMaxToksInBuffer,
                                    0,
                                    (SHORT)TEXT_TOKEN,
                                    (USHORT)ulCurStop,
                                    pData, sLangID, ulOemCP, pTagTable );
    } /* endif */
  }
  else
  {
    ulCurStop = UTF16strlenCHAR(pData);
    pCurrent = NTMSplitAndAddTokens(pSubstProp, pCurrent,
                                    &usMaxToksInBuffer,
                                    0,
                                    (SHORT)TEXT_TOKEN,
                                    (USHORT)ulCurStop,
                                    pData, sLangID, ulOemCP, pTagTable );
  } /* endif */



  while ( !fEndReached && (usTagRecordPos < RECLEN(pTagRecord) )
             && (usMaxToksInBuffer > 0)  )
  {
    ULONG  ulNewStart;
    /******************************************************************/
    /* fill in tag record                                             */
    /******************************************************************/
    ulNewStart = pTagEntry->usOffset;
    pCurrent = NTMSplitAndAddTokens(pSubstProp, pCurrent,
                              &usMaxToksInBuffer,
                              (USHORT)ulNewStart,
                              (SHORT)TAG_TOKEN,
                              (USHORT) (ulNewStart + pTagEntry->usTagLen-1),
                              pData, sLangID, ulOemCP, pTagTable );

    ulNewStart +=  pTagEntry->usTagLen;

  // pTagEntry->usTagLen is in # of w's, but usTagRecordPos counts bytes
  // also pByte is a byte ptr

    usTagRecordPos += sizeof(TMX_TAGENTRY)
                  + sizeof(CHAR_W) * (pTagEntry->usTagLen);

    pByte += sizeof(TMX_TAGENTRY) + sizeof(CHAR_W) * (pTagEntry->usTagLen);
    pTagEntry = (PTMX_TAGENTRY)pByte;

    if ( usTagRecordPos < RECLEN(pTagRecord) )
    {
      ulCurStop = pTagEntry->usOffset -1;
    }
    else
    {
      ulCurStop  = UTF16strlenCHAR( pData );   // we reached the end
      fEndReached = TRUE;
    } /* endif */
    /******************************************************************/
    /* split text record into word tokens                             */
    /* and fill into pCurrent struct                                  */
    /******************************************************************/
    if (usMaxToksInBuffer > 0 )
    {
      pCurrent = NTMSplitAndAddTokens(pSubstProp, pCurrent,
                                      &usMaxToksInBuffer,
                                      (USHORT)ulNewStart,
                                      (SHORT)TEXT_TOKEN,
                                      (USHORT)ulCurStop,
                                      pData, sLangID, ulOemCP, pTagTable );
    } /* endif */
  } /* endwhile */
  /********************************************************************/
  /* terminate start-stop table                                       */
  /********************************************************************/
  if (usMaxToksInBuffer > 0 )
  {
    memset(pCurrent, 0,sizeof(FUZZYTOK));

    *pusToken = (USHORT)(pCurrent - (PFUZZYTOK) pTokBuf);

    pCurrent++;

    fOK = UtlAlloc( (PVOID *)ppTokData, 0L, (PBYTE)pCurrent - (PBYTE)pTokBuf, NOMSG);
    if ( fOK )
    {
      memcpy( *ppTokData, pTokBuf, ((PBYTE)pCurrent-(PBYTE)pTokBuf));
    } /* endif */
  }
  else
  {
    fOK = FALSE;
    *ppTokData = NULL;
    *pusToken = 0;
  } /* endif */

  /********************************************************************/
  /* clean list of tokens to combine consecutive tags only intercepted*/
  /* by whitespaces returned as TEXT_TOKENS  (KBT0492)                */
  /********************************************************************/
  if ( fOK && (*pusToken > 2) )
  {
    USHORT k = 0;
    USHORT usNum = *pusToken;
    PFUZZYTOK pNew;
    CHAR_W chTemp;
    pNew = pCurrent = (PFUZZYTOK) *ppTokData;

    while ( k <= usNum )
    {
      switch ( pCurrent->sType )
      {
        case TAG_TOKEN:
          if ( k < usNum - 2 )
          {
            PFUZZYTOK pNext = pCurrent+1;
            USHORT    usTextLen = (pNext->usStop - pNext->usStart + 1);
            chTemp = *(pNext->pData + usTextLen);
            *(pNext->pData + usTextLen) = EOS;

            if ( (pNext->sType == TEXT_TOKEN) &&
                 ((pCurrent+2)->sType == TAG_TOKEN) &&
                 isWhiteSpaceW( pNext->pData ) )
            {
              /**********************************************************/
              /* combine tag - whitespace - tag sequence into one token */
              /**********************************************************/
              *(pNext->pData + usTextLen) = chTemp;
              memcpy( pNew, pCurrent, sizeof( FUZZYTOK ));

              pNext = pCurrent + 2;
              usTextLen = (pNext->usStop - pNext->usStart + 1);
              chTemp = *(pNext->pData + usTextLen);
              *(pNext->pData + usTextLen) = EOS;
              MakeHashValueW ( pSubstProp->ulRandom, MAX_RANDOM,
                              pNew->pData, &pNew->ulHash );
              *(pNext->pData + usTextLen) = chTemp;

              pNew->usStop = (pCurrent+2)->usStop;
              pNew++; pCurrent+=3; k+=3;
            }
            else
            {
              /**********************************************************/
              /* no special handling necessary                          */
              /**********************************************************/
              *(pNext->pData + usTextLen) = chTemp;
              memcpy( pNew, pCurrent, sizeof( FUZZYTOK ));
              pNew++; pCurrent++; k++;
            } /* endif */
          }
          else
          {
            /**********************************************************/
            /* no special handling necessary                          */
            /**********************************************************/
            memcpy( pNew, pCurrent, sizeof( FUZZYTOK ));
            pNew++; pCurrent++; k++;
          } /* endif */
          break;
        case TEXT_TOKEN:
        default:
          memcpy( pNew, pCurrent, sizeof( FUZZYTOK ));
          pNew++; pCurrent++; k++;
          break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /* adjust length                                                  */
    /******************************************************************/
    *pusToken = (USHORT)( pNew- (PFUZZYTOK) *ppTokData ) -1;

    while (pNew < pCurrent )
    {
      memset(pNew, 0,sizeof(FUZZYTOK));
      pNew++;
    } /* endwhile */
  } /* endif */

  return fOK;
}

// function checking text for tags which may not be used by automatic tag replacement
static EQF_BOOL ContainsNoSubstTags( PSZ_W pszText, int iLen, PLOADEDTABLE pTagTable )
{
   CHAR_W chTemp;
   PSZ_W pRest = NULL;
   USHORT usCol = 0;
   static TOKENENTRY TokenList[41];
   PTOKENENTRY pTokenEntry; 

   // return asap if no tag table is specified
   if ( pTagTable == NULL ) return( FALSE );

   // tokenize the text block to get list of tags
   memset( TokenList, 0, sizeof(TokenList) );
   chTemp = pszText[iLen];
   pszText[iLen] = 0;
   TATagTokenizeW( pszText, pTagTable, TRUE, &pRest, &usCol, TokenList, 40 );
   pszText[iLen] = chTemp;

   // skan returned tokens
   pTokenEntry = TokenList;
   while ( pTokenEntry->sTokenid != ENDOFLIST )
   {
     if ( pTokenEntry->ClassId == CLS_TRANSLVAR )
     {
       return( TRUE );
     } /* endif */    
     pTokenEntry++;
   } /* endwhile */      
  return( FALSE );
}

static PFUZZYTOK
NTMSplitAndAddTokens
(
  PTMX_SUBSTPROP     pSubstProp,
  PFUZZYTOK          pstCurrent,
  PUSHORT            pusMaxFreeToksInBuffer,
  USHORT             usStart,
  SHORT              sType,
  USHORT             usStop,
  PSZ_W              pString,
  SHORT              sLangID,
  ULONG              ulOemCP,
  PLOADEDTABLE       pTagTable
)
{
  CHAR_W        chTemp;
  USHORT        usListSize = 0;
  PTERMLENOFFS  pTermList = NULL;      // ptr to created term list
  PTERMLENOFFS  pActTerm;              // actual term
  PSZ_W         pCurWord;
  USHORT        usCurStart;
  ULONG         ulCurStop;
  USHORT        usRC;

  /********************************************************************/
  /* if token is text token, make one token for each word             */
  /********************************************************************/
  if (*pusMaxFreeToksInBuffer > 0)
  {
    if ( sType == TEXT_TOKEN )
    {
      chTemp = *(pString + (usStop+(USHORT)1));
      *(pString + (usStop+(USHORT)1)) = EOS;

      usRC = MorphTokenizeW( sLangID, pString+usStart,
                            &usListSize, (PVOID *)&pTermList,
                            MORPH_OFFSLIST, ulOemCP );

      *(pString + (usStop+(USHORT)1)) = chTemp;

      if ( pTermList )
      {
        pActTerm = pTermList;
        if ( pTermList->usLength && (*pusMaxFreeToksInBuffer > 0))
        {
          while ( pActTerm->usLength && (*pusMaxFreeToksInBuffer > 0))
          {
            pCurWord = pString + usStart + pActTerm->usOffset;
            /**********************************************************/
            /* ignore the linefeeds in the matching                   */
            /**********************************************************/
            if ( *pCurWord != LF )
            {
              usCurStart = (USHORT)(usStart + pActTerm->usOffset);
              ulCurStop = usCurStart + pActTerm->usLength - 1;

              pstCurrent->pData = pCurWord;
              pstCurrent->usStart = usCurStart;
              pstCurrent->sType   = sType;
              pstCurrent->usStop  = (USHORT)ulCurStop;
              pstCurrent->fConnected = FALSE;

              chTemp = *(pString + (ulCurStop+(USHORT)1));
              *(pString + (ulCurStop+(USHORT)1)) = EOS;
              MakeHashValueW ( pSubstProp->ulRandom, MAX_RANDOM,
                              pString + usCurStart, &pstCurrent->ulHash );
              *(pString + (ulCurStop+(USHORT)1)) = chTemp;
              pstCurrent++;
              (*pusMaxFreeToksInBuffer)--;
            } /* endif */
            pActTerm++;
          } /* endwhile */
        }
        else
        {
          pstCurrent->pData = pString+usStart;
          pstCurrent->usStart = usStart;
          pstCurrent->sType   = sType;
          pstCurrent->usStop  = usStop;
          pstCurrent->fConnected = FALSE;
          chTemp = *(pString + (usStop+(USHORT)1));
          *(pString + (usStop+(USHORT)1)) = EOS;
          MakeHashValueW ( pSubstProp->ulRandom, MAX_RANDOM,
                          pString + usStart, &pstCurrent->ulHash );
          *(pString + (usStop+(USHORT)1)) = chTemp;
          pstCurrent++;
          (*pusMaxFreeToksInBuffer)--;

        } /* endif */
      } /* endif */
      /****************************************************************/
      /* free allocated resource ...                                  */
      /****************************************************************/
      UtlAlloc( (PVOID *)&pTermList, 0L, 0L, NOMSG );
    }
    else
    {
      pstCurrent->pData = pString+usStart;
      pstCurrent->usStart = usStart;
      pstCurrent->sType   = sType;
      pstCurrent->usStop  = usStop;
      pstCurrent->fConnected = FALSE;
      pstCurrent->fNoTagSubst = ContainsNoSubstTags( pString + usStart, usStop - usStart + 1, pTagTable );
      chTemp = *(pString + (usStop+(USHORT)1));
      *(pString + (usStop+(USHORT)1)) = EOS;
      MakeHashValueW( pSubstProp->ulRandom, MAX_RANDOM,
                      pString+usStart , &pstCurrent->ulHash );
      *(pString + (usStop+(USHORT)1)) = chTemp;
      pstCurrent++;
      (*pusMaxFreeToksInBuffer)--;
    } /* endif */
  } /* endif */
  return (pstCurrent);

}

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
// ----------------------------------------------------------------------------+
static VOID
MakeHashValueW
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
  UCHAR  cSingle;

  while ( ((c = *pData++)!= NULC) && (usRandomIndex < usMaxRandom))
  {
  cSingle = (UCHAR) c;
    if ( !isspace( cSingle ) )
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
//|Function name:     NTMFuzzyReplace                                          |
//+----------------------------------------------------------------------------+
//|Function call:     NTMFuzzyReplace( pSource, pProp, pTrans,                 |
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
//|                   endwhile                                                 |
// ----------------------------------------------------------------------------+
static BOOL
NTMFuzzyReplace
(
  PTMX_SUBSTPROP pSubstProp,
  PSZ_W     pSource,                   // source string
  PSZ_W     pProp,                     // proposal string
  PSZ_W     pTrans,                    // translation string
  PREPLLIST pReplPropSrc,              // list of same tokens in source and prop
  PREPLLIST pReplaceList               // list of tokens to be replaced
#ifdef INLINE_TAG_REPL_LOGGING
  , FILE *hfLog
#endif
)
{
  PFUZZYTOK  pSrcTok;                  // source token
  PFUZZYTOK  pEndSrcTok;               // search until you detect this token
  PFUZZYTOK  pEndTgtTok;               // search until you detect this token
  PFUZZYTOK  pPropTok;                 // proposal token
  ULONG      ulTgtLen;                 // target length
  PREPLLIST  pTempRepl;                // pointer to temp repl element
  BOOL       fFound;                   // match found
  BOOL       fReplaced = FALSE;        // nothing replaced yet
  BOOL       fFirstInLoop = TRUE;      // sep. handling 1st time
  USHORT     usNumOfPairs = 0;
  BOOL       fOK = TRUE;

  pSource;
  pProp;
  ulTgtLen = UTF16strlenCHAR( pTrans );

#ifdef INLINE_TAG_REPL_LOGGING
      if ( hfLog ) fwprintf( hfLog, L"NTMFuzzyReplace\r\n" );
#endif

  pSubstProp->pTagPairs = (PTMX_REPLTAGPAIR) (pSubstProp->chBuffer);
  /********************************************************************/
  /* replace found tokens in translation with original ones....       */
  /********************************************************************/
  while ( pReplPropSrc->pSrcTok )
  {
    pSrcTok = pReplPropSrc->pTgtTok;
    pPropTok   = pReplPropSrc->pSrcTok;

#ifdef INLINE_TAG_REPL_LOGGING
      if ( hfLog )
      {
        fwprintf( hfLog, L"Testing token\r\n" );
      } /* endif */
      NTMListToken( hfLog, "SourceToken", pReplPropSrc->pSrcTok ); 
      NTMListToken( hfLog, "TargetToken", pReplPropSrc->pTgtTok ); 
#endif

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
      /* only replace of tagging done here                            */
      /****************************************************************/
      if ((pSrcTok->sType == TAG_TOKEN ) && (pPropTok->sType == TAG_TOKEN) && !pPropTok->fNoTagSubst )
      {
        while ( pTempRepl->pSrcTok && !fFound )
        {
          if ( pTempRepl->pSrcTok == pPropTok )
          {
            fFound = TRUE;
            pSubstProp->pTagPairs->pSrcTok = (PBYTE)pSrcTok;
            pSubstProp->pTagPairs->pPropTok = (PBYTE)pPropTok;
            pSubstProp->pTagPairs ++;
            usNumOfPairs ++;
          }
          else
          {
            pTempRepl++;
          } /* endif */
        } /* endwhile */
      } /* endif */

    } /* endwhile */
    pReplPropSrc++;                    // point to next one
  } /* endwhile */
  if (usNumOfPairs)
  {
    fReplaced = TRUE;
  //allocate for pairs
    fOK = UtlAlloc( (PVOID *) &(pSubstProp->pTagPairs), 0L,
                    (LONG)((usNumOfPairs+1) * sizeof(TMX_REPLTAGPAIR)), NOMSG );

    if (fOK )
    {
      //copy source string for later compare function
      memcpy( pSubstProp->pTagPairs, pSubstProp->chBuffer,
              (USHORT)( usNumOfPairs * sizeof(TMX_REPLTAGPAIR)));
    } /* endif */
  }
  else
  {
    pSubstProp->pTagPairs = NULL;
  } /* endif */
  return( fReplaced );
} /* end of function NTMFuzzyReplace */

#ifdef ACTIVATE_NTMGenericDelete 

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMGenericDelete                                         |
//+----------------------------------------------------------------------------+
//|Function call:     NTMGenericDelete( pSource, pProp, pTrans,                |
//|                                 pReplPropSrc, pReplaceList );              |
//+----------------------------------------------------------------------------+
//|Description:       delete tags                                              |
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
//|                   endwhile                                                 |
// ----------------------------------------------------------------------------+
static BOOL
NTMGenericDelete
(
  PTMX_SUBSTPROP pSubstProp,
  PSZ_W     pSource,                   // source string
  PSZ_W     pProp,                     // proposal string
  PSZ_W     pTrans,                    // translation string
  PREPLLIST pReplaceSource,            // list of same toks in src & srcofprop
  PREPLLIST pReplaceList               // eq. toks in src & tgt of prop
#ifdef INLINE_TAG_REPL_LOGGING
  , FILE *hfLog
#endif
)
{
  PFUZZYTOK  pSrcTok;                  // source token
  PFUZZYTOK  pEndSrcTok;               // search until you detect this token
  PFUZZYTOK  pEndTgtTok;               // search until you detect this token
  PFUZZYTOK  pSrcOfPropTok;            // proposal token
  ULONG      ulTgtLen;                 // target length
  PREPLLIST  pTempRepl;                // pointer to temp repl element
  BOOL       fFound;                   // match found
  BOOL       fReplaced = FALSE;        // nothing replaced yet
  BOOL       fFirstInLoop = TRUE;      // sep. handling 1st time
  USHORT     usNumOfPairs = 0;
  BOOL       fOK = TRUE;

  pProp; pSource;

  ulTgtLen = wcslen( pTrans );

  pSubstProp->pDelTagPairs = (PTMX_REPLTAGPAIR) (pSubstProp->chBuffer);
  /********************************************************************/
  /* replace found tokens in translation with original ones....       */
  /********************************************************************/
  while ( pReplaceSource->pSrcTok )
  {
    pSrcTok = pReplaceSource->pTgtTok;             // cur source
    pSrcOfPropTok   = pReplaceSource->pSrcTok;          // src of prop

    /******************************************************************/
    /* if next proposal token is part of the possible replacement list*/
    /* we've found one match ...                                      */
    /* we have to compare until the next match between Source and     */
    /* Proposal is detected or the end of the list ....               */
    /******************************************************************/
    pEndSrcTok = (pReplaceSource+1)->pTgtTok;
    pEndTgtTok = (pReplaceSource+1)->pSrcTok;

    if ( !pEndSrcTok )
    {
      pEndSrcTok = pSrcTok;
      while ( pEndSrcTok->ulHash )
      {
        pEndSrcTok++;
      } /* endwhile */
      pEndSrcTok--;
      /*****************************************************************/
      /* force that tags to be deleted at end of proposal are detected */
      /*****************************************************************/
      if (!fFirstInLoop )
      {
        pSrcTok = pEndSrcTok;
        pSrcTok--;
      } /* endif */
    } /* endif */

    if ( !pEndTgtTok )
    {
      pEndTgtTok = pSrcOfPropTok;
      while ( pEndTgtTok->ulHash )
      {
        pEndTgtTok++;
      } /* endwhile */
      pEndTgtTok--;
    } /* endif */
    /******************************************************************/
    /* adjustment for 1st time, earlier decreasing may affect         */
    /* pEndSrcTok/pEndTgtTok, if only 1 token in segment              */
    /******************************************************************/
    if ( fFirstInLoop )
    {
      fFirstInLoop = FALSE;
      pSrcTok = pEndSrcTok;   // force that they are equal
      pSrcTok--;
      pSrcOfPropTok = (PFUZZYTOK) pSubstProp->pTokPropSource;   // nec if tag at begin
      pSrcOfPropTok--;
    } /* endif */

    pSrcTok++;
    fFound = TRUE;

    while ( pSrcTok && ++pSrcOfPropTok
            && (pSrcTok == pEndSrcTok ) && (pSrcOfPropTok < pEndTgtTok)
            && fFound )
    {
      if (pSrcOfPropTok->sType == TAG_TOKEN )
      {
        pTempRepl = pReplaceList;    // eq toks in src&tgt of prop
        fFound = FALSE;
        /****************************************************************/
        /* only delete  of tagging in src/tgt of proposal checked here  */
        /****************************************************************/
        while ( pTempRepl->pSrcTok && !fFound )
        {
          // check if tagging to be deleted is identical in source and target of the proposal
          BOOL fIdentical = TRUE;
          int iSourceLen = pTempRepl->pSrcTok->usStop - pTempRepl->pSrcTok->usStart;
          int iTargetLen = pTempRepl->pTgtTok->usStop - pTempRepl->pTgtTok->usStart;
          if ( iSourceLen == iTargetLen )
          {
            fIdentical = wcsncmp( pTempRepl->pSrcTok->pData, pTempRepl->pTgtTok->pData, iSourceLen ) == 0;
          }
          else
          {
            fIdentical = FALSE;
          } /* endif */             

          if ( fIdentical && !pTempRepl->pSrcTok->fConnected && (pTempRepl->pSrcTok == pSrcOfPropTok) )
          {
            /************************************************************/
            /* tgt of proposal is eq to  src of prop   !!               */
            /************************************************************/
            fFound = TRUE;
            pSubstProp->pDelTagPairs->pSrcTok = (PBYTE)pTempRepl->pTgtTok;
            pSubstProp->pDelTagPairs->pPropTok = (PBYTE)pSrcOfPropTok;

            pSubstProp->pDelTagPairs ++;
            usNumOfPairs ++;
          }
          else
          {
            pTempRepl++;
          } /* endif */
        } /* endwhile */
      } /* endif */

    } /* endwhile */
    pReplaceSource++;                    // point to next one
  } /* endwhile */
  if (usNumOfPairs)
  {
    fReplaced = TRUE;
  //allocate for pairs
    fOK = UtlAlloc( (PVOID *) &(pSubstProp->pDelTagPairs), 0L,
                    (LONG)((usNumOfPairs+1) * sizeof(TMX_REPLTAGPAIR)), NOMSG );

    if (fOK )
    {
      //copy source string for later compare function
      memcpy( pSubstProp->pDelTagPairs, pSubstProp->chBuffer,
              (USHORT)( usNumOfPairs * sizeof(TMX_REPLTAGPAIR)));
    } /* endif */
  }
  else
  {
    pSubstProp->pDelTagPairs = NULL;
  } /* endif */
  return( fReplaced );
} /* end of function NTMGenericDelete */ 
#endif

//reset replace tag pair used flags
static void NTMResetUsedFlag( PTMX_REPLTAGPAIR pCurTagPair ) 
{
  while ( pCurTagPair )
  {
    if ( pCurTagPair->pPropTok )
    {
      pCurTagPair->fUsed = FALSE;
      pCurTagPair++;
    }
    else
    {
      pCurTagPair = NULL;
    } /* endif */
  } /*endwhile */
} /* end of function NTMResetUsedFlag */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMCheckTagPairs                                         |
//+----------------------------------------------------------------------------+
//|Function call:     NTMCheckTagPairs(pSubstProp)                             |
//+----------------------------------------------------------------------------+
//|Description:       check whether all tags in src and target of proposal     |
//|                   can be replaced                                          |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_SUBSTPROP pSubstProp                                |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      -                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
// ----------------------------------------------------------------------------+
static BOOL
NTMCheckTagPairs
(
  PTMX_SUBSTPROP pSubstProp,
  PREPLLIST pReplaceSourceList,         // list of same tokens in source and prop
  PREPLLIST pReplaceList,              // toklist to be replaced in prop src+tgt
#ifdef INLINE_TAG_REPL_LOGGING
  BOOL      fRespectLFs,
  FILE      *hfLog                     // log file handle
#else
  BOOL      fRespectLFs
#endif
)
{
  BOOL                fAllReplace = TRUE;
  PTMX_REPLTAGPAIR    pCurTagPair = pSubstProp->pTagPairs;
  PFUZZYTOK           pTempTok;
  USHORT              k;
  PREPLLIST           pReplTest = NULL;
  PREPLLIST           pReplProp = NULL;
  USHORT              usStart= 0;


  /********************************************************************/
  /* tags which are same in both srces and in propsrc-tgt, need not   */
  /* to be replaced. Hence for ease of use change sType to TEXT_TOKEN */
  /********************************************************************/

  pReplTest = pReplaceSourceList;
  pReplTest++;                                 // 1st entry is dummy
  pReplProp = pReplaceList;
  while ( pReplTest->pSrcTok)
  {
    pTempTok = pReplTest->pSrcTok;         // toks of src of prop
    if (pTempTok->sType == TAG_TOKEN )
    {
      usStart = pTempTok->usStart;
      /****************************************************************/
      /* tag in src of prop which is equal to a tag in current src    */
      /****************************************************************/
      while ( pReplProp->pSrcTok &&
             (pReplProp->pSrcTok->usStart <= usStart ))
      {
        if (pReplProp->pSrcTok == pTempTok )
        {
          /************************************************************/
          /* tag equal in all three sentence!!! no replace nec.       */
          /************************************************************/
          pTempTok->sType = TEXT_TOKEN;
          pReplTest->pTgtTok->sType = TEXT_TOKEN;
          pReplProp->pTgtTok->sType = TEXT_TOKEN;
        } /* endif */
        pReplProp++;
      } /* endwhile */

    } /* endif */
    pReplTest++;
  } /* endwhile */

#ifdef INLINE_TAG_REPL_LOGGING
  if ( hfLog )
  { 
    PREPLLIST           pRepl = pReplaceSourceList;
    fwprintf( hfLog, L"Replace source list\r\n" );
    while ( pRepl->pSrcTok)
    {
      NTMListToken( hfLog, "SourceToken", pRepl->pSrcTok );
      NTMListToken( hfLog, "TargetToken", pRepl->pTgtTok );
      pRepl++;
    } /*endwhile */

    fwprintf( hfLog, L"Replace list\r\n" );
    pRepl = pReplaceList;
    while ( pRepl->pSrcTok)
    {
      NTMListToken( hfLog, "SourceToken", pRepl->pSrcTok );
      NTMListToken( hfLog, "TargetToken", pRepl->pTgtTok );
      pRepl++;
    } /*endwhile */

  } /* endif */
#endif

  /********************************************************************/
  /* check all remaining tags in source of proposal                   */
  /********************************************************************/
  pTempTok = (PFUZZYTOK) pSubstProp->pTokPropSource;
  k = 0;
  NTMResetUsedFlag( pSubstProp->pTagPairs  );
  while ( (k<pSubstProp->usTokenPropSource) && fAllReplace )
  {
    if (pTempTok->sType == TAG_TOKEN )
    {
      pCurTagPair = NTMFindTagPair ( pTempTok,
                                     pSubstProp->szPropSource,
                                     pSubstProp, pSubstProp->pTagPairs, fRespectLFs );

      if (pCurTagPair == NULL )
      {
        pCurTagPair = NTMFindTagPair (pTempTok,
                                      pSubstProp->szPropSource,
                                      pSubstProp, pSubstProp->pDelTagPairs, fRespectLFs );
        if (pCurTagPair == NULL )
        {
          fAllReplace = FALSE;
#ifdef INLINE_TAG_REPL_LOGGING
          if ( hfLog )
          { 
            fwprintf( hfLog, L"No tag pair found for\r\n" );
            NTMListToken( hfLog, "Temp token", pTempTok );
          } /* endif */
#endif
        } /* endif */
      }
      else
      {
        if ( pCurTagPair->fUsed )
        {
          // this tag pair has been used already
          fAllReplace = FALSE;
        }
        else
        {
          pCurTagPair->fUsed = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */

    pTempTok++;
    k++;
  } /* endwhile */

  /********************************************************************/
  /* check all tags in target of proposal                             */
  /********************************************************************/
  pTempTok = (PFUZZYTOK) pSubstProp->pTokPropTarget;
  k = 0;
  NTMResetUsedFlag( pSubstProp->pTagPairs );
  while ( (k<pSubstProp->usTokenPropTarget) && fAllReplace )
  {
    if (pTempTok->sType == TAG_TOKEN )
    {
      pCurTagPair = NTMFindTagPair ( pTempTok, pSubstProp->szPropTarget, pSubstProp, pSubstProp->pTagPairs, fRespectLFs );
      if (pCurTagPair == NULL )
      {
        pCurTagPair = NTMFindTagPair (pTempTok, pSubstProp->szPropTarget, pSubstProp, pSubstProp->pDelTagPairs, fRespectLFs );
        if (pCurTagPair == NULL )
        {
#ifdef INLINE_TAG_REPL_LOGGING
          if ( hfLog )
          { 
            fwprintf( hfLog, L"No tag pair found in target\r\n" );
            NTMListToken( hfLog, "Temp token", pTempTok );
          } /* endif */
#endif
          fAllReplace = FALSE;
        } /* endif */
      }
      else
      {
        if ( pCurTagPair->fUsed )
        {
          // this tag pair has been used already
          fAllReplace = FALSE;
        }
        else
        {
          pCurTagPair->fUsed = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */

    pTempTok++;
    k++;
  } /* endwhile */


  return(fAllReplace);
} /* end of function NTMCheckTagPairs */

// NTMCheckAndDeleteTagPairs
//   check an delete tags in src and target of proposal 
static BOOL NTMCheckAndDeleteTagPairs
(
  PTMX_SUBSTPROP pSubstProp,
  PREPLLIST pReplaceSourceList,         // list of same tokens in source and prop
  PREPLLIST pDeleteList,              // toklist to be replaced in prop src+tgt
#ifdef INLINE_TAG_REPL_LOGGING
  BOOL      fRespectLFs,
  FILE      *hfLog                     // log file handle
#else
  BOOL      fRespectLFs
#endif
)
{
  BOOL                fAllReplace = TRUE;
  PFUZZYTOK           pTempTok;
  USHORT              k;
  PSZ_W               pNewData = NULL;
  PSZ_W               pTempData = NULL;
  LONG                lNewLen;
  SHORT               sLen;
  int                 iMode = 0;
  PSZ_W               pOrgText;
  USHORT              usNumOfTokens;

  fRespectLFs; 

  // process target and source of proposal
  for ( iMode = 0; iMode < 2; iMode++ )
  {
    if ( iMode == 0 )
    {
      // target mode
      pTempTok = (PFUZZYTOK) pSubstProp->pTokPropTarget;
      pOrgText = pSubstProp->szPropTarget;
      usNumOfTokens = pSubstProp->usTokenPropTarget;
    }
    else
    {
      // source mode
      pTempTok = (PFUZZYTOK) pSubstProp->pTokPropSource;
      pOrgText = pSubstProp->szPropSource;
      usNumOfTokens = pSubstProp->usTokenPropSource;
    } /* end */       

    k = 0;
    lNewLen = 0;
    pNewData = pSubstProp->chBuffer;

    // process proposal text
    while ( ( k < usNumOfTokens) && fAllReplace )
    {
      BOOL fCopyToken = TRUE;

      if ( pTempTok->sType == TAG_TOKEN )
      {
        // check if token is contained in delete pair list
        PREPLLIST pDel = pDeleteList;
        BOOL fFound = FALSE;
        while ( pDel->pSrcTok && !fFound )
        {
          PFUZZYTOK pTestTok = (iMode == 0) ? pDel->pTgtTok : pDel->pSrcTok ;
          if ( pTestTok == pTempTok )
          {
            //now check if the source proposal token is contained in the source string
            PREPLLIST pSrc = pReplaceSourceList;
            fCopyToken = FALSE;
            fFound = TRUE;
            while ( pSrc->pSrcTok && !fCopyToken )
            {
              if ( pSrc->pTgtTok == pDel->pSrcTok )
              {
                // do not delete this token
                fCopyToken = TRUE;
              } /* end */             
              pSrc++;
            } /*endwhile */
          } /* end */             
          pDel++;
        } /*endwhile */
      } /* endif */

      if ( fCopyToken )
      {
        pTempData = pOrgText + pTempTok->usStart;
        if ((pTempTok + 1)-> ulHash && (pTempTok+1)->usStart )
        {
          if ( (pTempTok->usStop < (pTempTok+1)->usStart) )
          {
            // adjust so that spaces between words are copied too       
            pTempTok->usStop = (pTempTok+1)->usStart - 1;
          } /* endif */
        }
        else
        {
          // we are at the end of the token list, ensure that we do not miss any trailing blanks
          pTempTok->usStop = (USHORT)(pTempTok->usStart + wcslen( pTempData ) - 1);
        } /* endif */
        sLen = pTempTok->usStop - pTempTok->usStart +1;
        NTMCopyTokData ( pTempData, sLen, &pNewData, &lNewLen);
      }
      else
      {
        // copy any whitepace following the skipped token
        USHORT usStop = pTempTok->usStop + 1;
        if ((pTempTok + 1)->ulHash && (pTempTok+1)->usStart )
        {
          while ( (usStop < (pTempTok+1)->usStart) )
          {
            *pNewData++ = pOrgText[usStop++];
            lNewLen++;
          } /* endwhile */
        }
        else
        {
          // we are at the end of the token list, ensure that we do not miss any trailing blanks
          USHORT usEnd = (USHORT)(pTempTok->usStart + wcslen( pTempData ) - 1);
          while ( usStop < usEnd )
          {
            *pNewData++ = pOrgText[usStop++];
            lNewLen++;
          } /* endwhile */
        } /* endif */
      } /* endif */

      pTempTok++;
      k++;
    } /* endwhile */

    if (fAllReplace )
    {
      *pNewData = EOS;
      lNewLen++;
      UTF16strcpy ( pOrgText, pSubstProp->chBuffer);
    } /* endif */
  } /* endfor */     

  return(fAllReplace);
} /* end of function NTMCheckAndDeleteTagPairs */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMReplaceTags                                           |
//+----------------------------------------------------------------------------+
//|Function call:     NTMReplaceTags  (pSubstProp)                             |
//+----------------------------------------------------------------------------+
//|Description:       build up new src and tgt of proposal with replaced       |
//|                   tagging                                                  |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_SUBSTPROP pSubstProp                                |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     replace tags in target of proposal                       |
//|                   replace tags in source of proposal                       |
// ----------------------------------------------------------------------------+
static BOOL
NTMReplaceTags
(
  PTMX_SUBSTPROP pSubstProp,
  BOOL              fRespectLFs
)
{
  BOOL                fAllReplace = TRUE;
  PTMX_REPLTAGPAIR    pCurTagPair = pSubstProp->pTagPairs;
  PFUZZYTOK           pTempTok;
  USHORT              k;
  PFUZZYTOK           pCurPairSrcTok;
  SHORT               sLen;
  PSZ_W               pTempData;
  LONG                lNewLen = 0;
  PSZ_W               pNewData;

  /********************************************************************/
  /* replace all tags in target of proposal                           */
  /********************************************************************/
  pTempTok = (PFUZZYTOK) pSubstProp->pTokPropTarget;
  k = 0;
  lNewLen = 0;
  pNewData = pSubstProp->chBuffer;
  NTMResetUsedFlag( pSubstProp->pTagPairs  );
  while ( (k<pSubstProp->usTokenPropTarget) && fAllReplace )
  {
    if (pTempTok->sType == TAG_TOKEN )
    {
      pCurTagPair = NTMFindTagPair ( pTempTok,
                                     pSubstProp->szPropTarget,
                                     pSubstProp, pSubstProp->pTagPairs, fRespectLFs );
      if (pCurTagPair != NULL )
      {
        /****************************************************************/
        /* pTempTok points to a tag token; current pCurTagPair          */
        /* contains tag from current active source segment              */
        /* which replaces tag from TranslationMemory                    */
        /****************************************************************/
        pCurTagPair->fUsed = TRUE;
        pCurPairSrcTok = (PFUZZYTOK) pCurTagPair->pSrcTok;
        pTempData = pSubstProp->szSource + pCurPairSrcTok->usStart;
        sLen = pCurPairSrcTok->usStop - pCurPairSrcTok->usStart +1;
        fAllReplace = NTMCopyTokData ( pTempData, sLen, &pNewData, &lNewLen);

        if (fAllReplace )
        {
          pCurPairSrcTok->fConnected = TRUE;
        } /* endif */

        /****************************************************************/
        /* copy "space" between tagtoken in prop and next tok in prop   */
        /****************************************************************/
        if ((pTempTok + 1)-> ulHash && (pTempTok+1)->usStart )
        {
          if ( (pTempTok->usStop + 1 < (pTempTok+1)->usStart) )
          {
            sLen = (pTempTok+1)->usStart - pTempTok->usStop - 1;
            pTempData = pSubstProp->szPropTarget + pTempTok->usStop+1;
            fAllReplace = NTMCopyTokData ( pTempData, sLen,
                                          &pNewData, &lNewLen);
          } /* endif */
        } /* endif */
      }
      else         // check whether tag should be deleted
      {
        pCurTagPair = NTMFindTagPair ( pTempTok,
                                       pSubstProp->szPropTarget,
                                       pSubstProp, pSubstProp->pDelTagPairs, fRespectLFs );
        if (pCurTagPair == NULL )
        {
          fAllReplace = FALSE;
        } /* endif */
      } /*endif */

    }
    else
    {
      /****************************************************************/
      /* pTempTok points to a text token; copy it into target         */
      /****************************************************************/
      pTempData = pSubstProp->szPropTarget + pTempTok->usStart;
      if ((pTempTok + 1)-> ulHash && (pTempTok+1)->usStart )
      {
        if ( (pTempTok->usStop < (pTempTok+1)->usStart) )
        {
          /************************************************************/
          /* adjust so that spaces between words are copied too       */
          /************************************************************/
          pTempTok->usStop = (pTempTok+1)->usStart - 1;
        } /* endif */
      }
      else
      {
        // we are at the end of the token list, ensure that we do not miss any trailing blanks
        pTempTok->usStop = (USHORT)(pTempTok->usStart + wcslen( pTempData ) - 1);
      } /* endif */
      sLen = pTempTok->usStop - pTempTok->usStart +1;
      fAllReplace = NTMCopyTokData ( pTempData, sLen, &pNewData, &lNewLen);
    } /* endif */

    pTempTok++;
    k++;
  } /* endwhile */

  if (fAllReplace )
  {
    *pNewData = EOS;
    lNewLen++;
    UTF16strcpy ( pSubstProp->szPropTarget, pSubstProp->chBuffer);
  } /* endif */

  /********************************************************************/
  /* replace all tags in source of proposal                           */
  /* build new string in chBuffer and copy it at the end in szPropSource*/
  /********************************************************************/
  pTempTok = (PFUZZYTOK) pSubstProp->pTokPropSource;
  k = 0;
  pNewData = pSubstProp->chBuffer;
  lNewLen = 0;
  NTMResetUsedFlag( pSubstProp->pTagPairs  );
  while ( (k<pSubstProp->usTokenPropSource) && fAllReplace )
  {
    if (pTempTok->sType == TAG_TOKEN )
    {
      pCurTagPair = NTMFindTagPair ( pTempTok,
                                     pSubstProp->szPropSource,
                                     pSubstProp, pSubstProp->pTagPairs, fRespectLFs );
      if (pCurTagPair != NULL )
      {
        /****************************************************************/
        /* pTempTok points to a tag token; current pCurTagPair          */
        /* contains tag from current active source segment              */
        /* which replaces tag from TranslationMemory                    */
        /****************************************************************/
        pCurTagPair->fUsed = TRUE;
        pCurPairSrcTok = (PFUZZYTOK) pCurTagPair->pSrcTok;
        pTempData = pSubstProp->szSource + pCurPairSrcTok->usStart;
        sLen = pCurPairSrcTok->usStop - pCurPairSrcTok->usStart +1;

        fAllReplace = NTMCopyTokData ( pTempData, sLen, &pNewData, &lNewLen);
        /****************************************************************/
        /* copy "space" between tagtoken in prop and next tok in prop   */
        /****************************************************************/
        if ((pTempTok + 1)-> ulHash && (pTempTok+1)->usStart )
        {
          if ( (pTempTok->usStop + 1 < (pTempTok+1)->usStart) )
          {
            sLen = (pTempTok+1)->usStart - pTempTok->usStop - 1;
            pTempData = pSubstProp->szPropSource + pTempTok->usStop+1;
            fAllReplace = NTMCopyTokData ( pTempData, sLen,
                                          &pNewData, &lNewLen);
          } /* endif */
        } /* endif */
      }
      else
      {
        pCurTagPair = NTMFindTagPair ( pTempTok,
                                       pSubstProp->szPropSource,
                                       pSubstProp, pSubstProp->pDelTagPairs, fRespectLFs );
        if (pCurTagPair == NULL )
        {
          fAllReplace = FALSE;
        } /* endif */
      } /*endif */
    }
    else
    {
      /****************************************************************/
      /* pTempTok points to a text token; copy it into target         */
      /****************************************************************/
      pTempData = pSubstProp->szPropSource + pTempTok->usStart;
      if ((pTempTok + 1)-> ulHash && (pTempTok+1)->usStart )
      {
        if ( (pTempTok->usStop < (pTempTok+1)->usStart) )
        {
          /************************************************************/
          /* adjust so that spaces between words are copied too       */
          /************************************************************/
          pTempTok->usStop = (pTempTok+1)->usStart - 1;
        } /* endif */
      }
      else
      {
        // we are at the end of the token list, ensure that we do not miss any trailing blanks
        pTempTok->usStop = (USHORT)(pTempTok->usStart + wcslen( pTempData ) - 1);
      } /* endif */
      sLen = pTempTok->usStop - pTempTok->usStart +1;
      fAllReplace = NTMCopyTokData ( pTempData, sLen, &pNewData, &lNewLen);
    } /* endif */

    pTempTok++;
    k++;
  } /* endwhile */

  if (fAllReplace )
  {
    *pNewData = EOS;
    lNewLen++;
    UTF16strcpy ( pSubstProp->szPropSource, pSubstProp->chBuffer);
    /******************************************************************/
    /* after this strcpy ALL pCurTagPair->pSrcTok and ALL pSrcTok     */
    /* which point on szPropSource are INVALID                        */
    /* Therefore replace in szPropTarget takes place before           */
    /* replace in szPropSource !!!                                    */
    /******************************************************************/
  } /* endif */
  return(fAllReplace);
} /* end of function NTMReplaceTags */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMFindTagPair                                           |
//+----------------------------------------------------------------------------+
//|Function call:     NTMFindTagPair  (pSubstProp)                             |
//+----------------------------------------------------------------------------+
//|Description:       find tag pair which has to be replaced by each other     |
//+----------------------------------------------------------------------------+
//|Parameters:        PFUZZYTOK      pTempTok,                                 |
//|                   PSZ            pTempTokData,                             |
//|                   PTMX_SUBSTPROP pSubstProp                                |
//+----------------------------------------------------------------------------+
//|Returncode type:   PTMX_REPLTAGPAIR                                         |
//+----------------------------------------------------------------------------+
//|Function flow:     find tag pair                                            |
// ----------------------------------------------------------------------------+
static PTMX_REPLTAGPAIR
NTMFindTagPair
(
  PFUZZYTOK        pTempTok,
  PSZ_W            pTempTokData,
  PTMX_SUBSTPROP   pSubstProp,
  PTMX_REPLTAGPAIR pCurTagPair,
  BOOL             fRespectLFs

)
{
  PTMX_REPLTAGPAIR  pFoundTagPair = NULL;
  BOOL              fFound = FALSE;
  SHORT             sLen;
  PFUZZYTOK         pCurPairPropTok;
  PSZ_W             pPairData;
  PSZ_W             pTempData;


  pTempData = pTempTokData + pTempTok->usStart;          // prop src or tgt
  sLen = pTempTok->usStop - pTempTok->usStart +1;
  if (pCurTagPair )
  {
    pCurPairPropTok = (PFUZZYTOK) pCurTagPair->pPropTok;   //src of proposal

    while (!fFound && pCurPairPropTok )
    {
      if ( pCurTagPair->fUsed )
      {
        // ignore this entry
      }
      else if (pCurPairPropTok == pTempTok )
      {
        fFound = TRUE;
      }
      else
      {
        pPairData = pSubstProp->szPropSource + pCurPairPropTok->usStart;
        if (sLen == (pCurPairPropTok->usStop -pCurPairPropTok->usStart + 1 ))
        {
          if ( fRespectLFs )
          {
            if ( memcmp( (PBYTE)pTempData,(PBYTE)pPairData, sLen * sizeof(CHAR_W)) == 0)
            {
              fFound = TRUE;
            } /* endif */
          }
          else
          {
            CHAR_W chTemp1 = pTempTokData[pTempTok->usStop+1];
            CHAR_W chTemp2 = pSubstProp->szPropSource[pCurPairPropTok->usStop+1];
            pTempTokData[pTempTok->usStop+1] = 0;  
            pSubstProp->szPropSource[pCurPairPropTok->usStop+1] = 0;
            fFound = (UtlCompIgnWhiteSpaceW( pTempData, pPairData, 0 ) == 0 );
            pTempTokData[pTempTok->usStop+1] = chTemp1;  
            pSubstProp->szPropSource[pCurPairPropTok->usStop+1] = chTemp2;
          } /* endif */
        } /* endif */
      } /* endif */
      if (!fFound )
      {
        pCurTagPair++;
        if (pCurTagPair )
        {
          pCurPairPropTok = (PFUZZYTOK)pCurTagPair->pPropTok;
        }
        else
        {
          pCurPairPropTok = NULL;
        } /* endif */
      }
      else
      {
        pFoundTagPair = pCurTagPair;
      } /* endif */
    } /* endwhile */

  } /* endif */
  return(pFoundTagPair);
} /* end of function NTMFindTagPair */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMCopyTokData                                           |
//+----------------------------------------------------------------------------+
//|Function call:     NTMCopyTokdata  (pSubstProp)                             |
//+----------------------------------------------------------------------------+
//|Description:       copy data into the new buffer                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ    pTempData,                                        |
//|                   SHORT  sLen,                                             |
//|                   PSZ *  ppNewData,                                        |
//|                   PSHORT pulNewLen                                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   PTMX_REPLTAGPAIR                                         |
//+----------------------------------------------------------------------------+
//|Function flow:     copy data                                                |
// ----------------------------------------------------------------------------+
static BOOL
NTMCopyTokData
(
  PSZ_W  pTempData,
  SHORT  sLen,
  PSZ_W *ppNewData,
  PLONG  plNewLen

)
{
  BOOL   fAllReplace = TRUE;
  PSZ_W  pData;

  pData = *ppNewData;

  if (*plNewLen + sLen < MAX_SEGMENT_SIZE )
  {
    memcpy((PBYTE)pData, (PBYTE)pTempData, sLen*sizeof(CHAR_W));
    pData += sLen;
    *ppNewData = pData;
    *(plNewLen) += sLen;
  }
  else
  {
    fAllReplace = FALSE;
  } /*endif */

  return (fAllReplace);
}

// function to compare the context strings of two segments
USHORT NTMCompareContext
(
  PTMX_CLB pTmClb,                     // ptr to ctl block struct
  PSZ         pszMarkup,               // ptr to name markup used for segment
  PSZ_W       pszContext1,             // context of first segment
  PSZ_W       pszContext2              // context of second segment
)
{
  USHORT usRanking = 1;              // default = weak context match

  if ( pszContext1 && *pszContext1 &&
       pszContext2 && *pszContext2 )
  {
    // load tag table if not done yet
    if ( pTmClb->pTagTable == NULL )
    {
      (TALoadTagTableExHwnd( pszMarkup, (PLOADEDTABLE *)&(pTmClb->pTagTable),
                                        FALSE,
                                        TALOADUSEREXIT | TALOADCOMPCONTEXTFUNC,
                                        FALSE, NULLHANDLE ) == NO_ERROR);
    } /* endif */

    // call compare function of user exit if available
    if ( pTmClb->pTagTable != NULL )
    {
      PLOADEDTABLE pTable = (PLOADEDTABLE)pTmClb->pTagTable;
      PFNCOMPCONTEXT pfnCompContext = (PFNCOMPCONTEXT)pTable->pfnCompareContext;

      if ( pfnCompContext )
      {
        pfnCompContext( pszContext1, pszContext2, &usRanking );
      }
      else
      {
        if ( wcsicmp( pszContext1, pszContext2 ) == 0 )
        {
          usRanking = 100;
        }
      } /* endif */
    }
    else
    {
      if ( wcsicmp( pszContext1, pszContext2 ) == 0 )
      {
        usRanking = 100;
      }
    } /* endif */
  } /* endif */

  return( usRanking );
} /* end of function NTMCompareContext */

typedef struct _NTMGETMATCHLEVELDATA
{
  CHAR             szSegmentTagTable[MAX_EQF_PATH];        // tag table name for segment markup
  CHAR             szProposalTagTable[MAX_EQF_PATH];       // tag table name for proposal markup
  CHAR_W           szSegment[8096];                        // buffer for segment data
  BYTE             bTokenBuffer[40000];                    // buffer for tokenizer in replace match detection
                                                           // this buffer has to hold token entries with a size of 18 bytes each
                                                           // theoratically a segmet could contain 2047 tokens....
  BYTE             bBuffer[40000];                         // general purpose buffer
  CHAR             szLang1[MAX_LANG_LENGTH];               // language buffer 1
  CHAR             szLang2[MAX_LANG_LENGTH];               // language buffer 2
} NTMGETMATCHLEVELDATA, *PNTMGETMATCHLEVELDATA;

// static data of NTMGetMatchLevelData
static char szLastSourceLang[MAX_LANG_LENGTH] = "";
static char szLastTargetLang[MAX_LANG_LENGTH] = "";
static ULONG ulLastSrcOemCP = 0;
static ULONG ulLastTgtOemCP = 0;               

void NTMCopyAndClean( PSZ_W pszTarget, PSZ_W pszSource )
{
  while ( *pszSource )
  {
    // ignore carriage returns 
    if ( *pszSource == L'\r' )
    {
      pszSource++;
      if ( (*pszSource != L'\n') )
      {
        *pszTarget++ = L'\n';
      } /* endif */
    }
    else
    {
      *pszTarget++ = *pszSource++;
    } /* endif */
  } /*endwhile */
  *pszTarget = 0;
}

// get match level function ( called by nonDDE API to evaluate the match level
USHORT NTMGetMatchLevel
( 
  PEQFSEGINFO      pSegment,           // pointer to segment info
  PEQFSEGINFO      pProposal,          // pointer to memory proposal info
  SHORT            *psMatchLevel,      // pointer to caller's match level field
  SHORT            *psMatchState,      // pointer to caller's match state field
  LONG             lOptions            // options to be used by the function 
)
{
  USHORT           usRC = 0;                     // function return code, 0 = OK
  PTMX_SENTENCE    pSentence = NULL;
  PTMX_SENTENCE    pMatchSentence = NULL;
  PTMX_SENTENCE    pMatchTarget = NULL;
  ULONG            ulParm = 0;                   // set here specific parameters such as RESPECT_CRLF
  BOOL             fStringEqual = FALSE;         // TRUE = strings are equal
  ULONG            ulSrcOemCP = 0;               // OEM codepage for source language
  ULONG            ulTgtOemCP = 0;               // OEM codepage for target language
  PTMX_SUBSTPROP   pSubstProp = NULL;            // data area for genreic inline tag replacment 
  SHORT            sLangID = 0;                  // morph. ID for source language
  USHORT           usFuzzy = 0;                  // buffer for computed fuziness
  BOOL             fTagTableEqual = FALSE;       // TRUE = tag tables are equal
  PNTMGETMATCHLEVELDATA pData = NULL;
  BOOL             fGenericTagReplacement = FALSE; // TRUE = apply generic inline tag replacement
  USHORT           usWords = 0;                  // number of words in segment
  BOOL             fSourceAndTargetEqual = FALSE; // TRUE = source and target of propsal are equal
  BOOL             fSourceSource = FALSE;        // TRUE = source and target language of propsal are equal


#ifdef MEASURETIME
  LARGE_INTEGER liStart;
  LARGE_INTEGER liPrepare;
  LARGE_INTEGER liTokenize;
  LARGE_INTEGER liGetLang;
  LARGE_INTEGER liMorphAct;
  LARGE_INTEGER liComp;
  LARGE_INTEGER liGetFuzzy;
  LARGE_INTEGER liGeneric;
  LARGE_INTEGER liReplace;
  DWORD dwStart, dwEnd;
#endif

#ifdef MATCHLEVEL_LOGGING
  FILE *hfLog = NULL;
#endif

#ifdef MATCHLEVEL_LOGGING
  {
    CHAR szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szLogFile, 0L, FALSE );
    strcat( szLogFile, BACKSLASH_STR );
    strcat( szLogFile, "MATCHLEVEL.LOG" );
    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      PPROPSYSTEM pSysProp;             // ptr to EQF system properties
      pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());

      fprintf( hfLog, "*** EqfGetMatchLevel ***\n" );
      fprintf( hfLog, "   segment               = \"%S\"\n", pSegment->szSource  );
      fprintf( hfLog, "   segment source lang   = \"%s\"\n", pSegment->szSourceLanguage  );
      fprintf( hfLog, "   proposal source       = \"%S\"\n", pProposal->szSource  );
      fprintf( hfLog, "   proposal target       = \"%S\"\n", pProposal->szTarget  );
      fprintf( hfLog, "   proposal source lang  = \"%s\"\n", pProposal->szSourceLanguage  );
      fprintf( hfLog, "   proposal target lang  = \"%s\"\n", pProposal->szTargetLanguage  );
      fprintf( hfLog, "   default target lang   = \"%s\" CP=%lu\n", pSysProp->szSystemPrefLang, pSysProp->ulSystemPrefCP );
    } /* endif */
  }
#endif

#ifdef MEASURETIME
  dwStart = GetTickCount();
  QueryPerformanceCounter( &liStart );
#endif

  // allocate our data area
  if ( !usRC ) usRC = (UtlAlloc( (PVOID*)&pData, 0L, (LONG)sizeof(NTMGETMATCHLEVELDATA), NOMSG)) ? 0 : ERROR_NOT_ENOUGH_MEMORY;

  // allocate sentence structures
  if ( !usRC ) usRC = NTMAllocSentenceStructure( &pSentence );
  if ( !usRC ) usRC = NTMAllocSentenceStructure( &pMatchSentence );
  if ( !usRC ) usRC = NTMAllocSentenceStructure( &pMatchTarget );

  // allocate gerenic inline replacement tag data area
  if ( !usRC ) usRC = (UtlAlloc( (PVOID*)&pSubstProp, 0L, (LONG)sizeof(TMX_SUBSTPROP), NOMSG)) ? 0 : ERROR_NOT_ENOUGH_MEMORY;

  // stop processing if languages do not match 
  if ( !usRC )
  {
    Utlstrccpy( pData->szLang1, pSegment->szSourceLanguage, '(' );
    Utlstrccpy( pData->szLang2, pProposal->szSourceLanguage, '(' );
    if ( _stricmp( pData->szLang1, pData->szLang2 ) != 0 )
    {
      *psMatchState = NONE_MATCHSTATE;
      *psMatchLevel = 0;
      return( 0 );
    } /* endif */

    Utlstrccpy( pData->szLang1, pSegment->szTargetLanguage, '(' );
    Utlstrccpy( pData->szLang2, pProposal->szTargetLanguage, '(' );
    if ( _stricmp( pData->szLang1, pData->szLang2 ) != 0 )
    {
      *psMatchState = NONE_MATCHSTATE;
      *psMatchLevel = 0;
      return( 0 );
    } /* endif */
  } /* endif */

  // check if generic inline tag replacement should be used
  if ( lOptions & NO_GENERIC_INLINETAG_REPL_OPT )
  {
    // switched off using lOptions
    fGenericTagReplacement = FALSE;
  }
  else if ( lOptions & USE_GENERIC_INLINETAG_REPL_OPT )
  {
    // switched on using lOptions
    fGenericTagReplacement = TRUE;
  }
  else
  {
    // base on setting in system properties
    PPROPSYSTEM pSysProp;             // ptr to EQF system properties
    pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
    fGenericTagReplacement = !pSysProp->fNoGenericMarkup;
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liPrepare );
#endif

  if ( strcmp( pSegment->szSourceLanguage, szLastSourceLang ) == 0 )
  {
    ulSrcOemCP = ulLastSrcOemCP;
  }
  else
  {
    ulLastSrcOemCP = ulSrcOemCP = GetLangOEMCP( pSegment->szSourceLanguage );
    strcpy( szLastSourceLang, pSegment->szSourceLanguage );
  } /* endif */

  if ( strcmp( pSegment->szTargetLanguage, pSegment->szSourceLanguage ) == 0 )
  {
    ulTgtOemCP = ulSrcOemCP;
    fSourceSource = TRUE;         // source and target languages are equal

  }
  else if ( strcmp( pSegment->szTargetLanguage, szLastTargetLang ) == 0 )
  {
    ulTgtOemCP = ulLastTgtOemCP;
  }
  else
  {
    ulLastTgtOemCP = ulTgtOemCP = GetLangOEMCP( pSegment->szTargetLanguage );
    strcpy( szLastTargetLang, pSegment->szTargetLanguage );
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liGetLang );
#endif

  fSourceAndTargetEqual = UTF16strcmp( pProposal->szSource, pProposal->szTarget ) == 0;

  // tokenize source segment and match, resulting in normalized string and tag table record
  NTMMakeTagTablename( pSegment->szMarkup, pData->szSegmentTagTable );
  NTMMakeTagTablename( pProposal->szMarkup, pData->szProposalTagTable );

  //  UTF16strcpy( pSentence->pInputString, pSegment->szSource );
  NTMCopyAndClean( pSentence->pInputString, pSegment->szSource );
  usRC = TokenizeSourceEx( NULL, pSentence, pData->szSegmentTagTable, pSegment->szSourceLanguage, TM_MAJ_VERSION, ulSrcOemCP );

  // UTF16strcpy( pMatchSentence->pInputString, pProposal->szSource );
  NTMCopyAndClean( pMatchSentence->pInputString, pProposal->szSource );
  if ( !usRC ) usRC = TokenizeSourceEx( NULL, pMatchSentence, pData->szProposalTagTable, pProposal->szSourceLanguage, TM_MAJ_VERSION, ulSrcOemCP );
  if ( !fSourceAndTargetEqual)
  {
    UTF16strcpy( pMatchTarget->pInputString, pProposal->szTarget );
    if ( !usRC ) usRC = TokenizeSourceEx( NULL, pMatchTarget, pData->szProposalTagTable, pProposal->szTargetLanguage, TM_MAJ_VERSION, ulTgtOemCP );
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liTokenize );
#endif

  if ( MorphGetLanguageID( pSegment->szSourceLanguage, &sLangID ) != 0 )
  {
    PSZ pszParm = pSegment->szSourceLanguage;
    usRC = ERROR_INV_LANGUAGE;
    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liMorphAct );
#endif


  fTagTableEqual = (_stricmp( pSegment->szMarkup, pProposal->szMarkup ) == 0);
  // For R012645 begin
  if(!fTagTableEqual)
  {
    std::string segMarkupFamily = GetAndCacheFamilyName( pSegment->szMarkup);
    if(!segMarkupFamily.empty())
    {
      std::string propMarkupFamily = GetAndCacheFamilyName(pProposal->szMarkup);
      if(!propMarkupFamily.empty() && segMarkupFamily==propMarkupFamily)
        fTagTableEqual = TRUE;
    }
  }
 // For R012645 end

  // check if strings are equal
  if ( ulParm & GET_RESPECTCRLF )   
  {
	  BOOL fRespectCRLFStringEqual = (UtlCompIgnSpaceW( pMatchSentence->pNormString, pSentence->pNormString, 0 )== 0L);
	  fStringEqual = (UtlCompIgnWhiteSpaceW( pMatchSentence->pNormString, pSentence->pNormString, 0 ) == 0L);
	  if (fStringEqual && !fRespectCRLFStringEqual)
	  {  
      // there is a LF difference!
      fStringEqual = fRespectCRLFStringEqual;
	  }
  }
  else
  { 
    fStringEqual = (UtlCompIgnWhiteSpaceW( pMatchSentence->pNormString, pSentence->pNormString, 0 ) == 0L);
  } /* endif*/

#ifdef MEASURETIME
  QueryPerformanceCounter( &liComp );
#endif

#ifdef MATCHLEVEL_LOGGING
  if ( hfLog )
  {
    fprintf( hfLog, "   fStringEqual          = %s\n", fStringEqual ? "true" : "false " );
  } /* endif */
#endif

  // compute fuzziness
  if ( !usRC )
  {
    BOOL fFuzzynessOK;
    fFuzzynessOK = TMFuzzynessEx( pSegment->szMarkup, pSegment->szSource, pProposal->szSource, sLangID, &usFuzzy, ulSrcOemCP, &usWords );
    if ( usFuzzy != 100) fStringEqual = FALSE;
#ifdef MATCHLEVEL_LOGGING
  if ( hfLog )
  {
    fprintf( hfLog, "  results of fuzziness test\n" );
    fprintf( hfLog, "   fFuzzynessOK          = %s\n", fFuzzynessOK ? "true" : "false " );
    fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
    fprintf( hfLog, "   ulSrcOemCP            = %lu\n", ulSrcOemCP );
    fprintf( hfLog, "   sLangID               = %d\n", sLangID );
  } /* endif */
#endif
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liGetFuzzy );
#endif

  // perform generic inline tag replacement
  if ( !fStringEqual && !usRC && fGenericTagReplacement )
  {
    BOOL fSubstAll = FALSE;

    // do not enter generic inline tag replacement when proposal does not contain any matches
    PTMX_TAGTABLE_RECORD pTagRecord = (PTMX_TAGTABLE_RECORD)pMatchSentence->pTagRecord;
    if ( pTagRecord->ulRecordLen > sizeof(TMX_TAGTABLE_RECORD) )
    {
      strcpy( pSubstProp->szSourceTagTable, pSegment->szMarkup );
      strcpy( pSubstProp->szSourceLanguage, pSegment->szSourceLanguage);
//      UTF16strcpy( pSubstProp->szSource, pSegment->szSource );
      NTMCopyAndClean( pSubstProp->szSource, pSegment->szSource );
      pSubstProp->pTagsSource = (PTMX_TAGTABLE_RECORD)pSentence->pTagRecord;

//      UTF16strcpy( pSubstProp->szPropSource, pProposal->szSource );
      NTMCopyAndClean( pSubstProp->szPropSource, pProposal->szSource );
      pSubstProp->pTagsPropTarget = (PTMX_TAGTABLE_RECORD)pMatchSentence->pTagRecord;

//      UTF16strcpy( pSubstProp->szPropTarget, pProposal->szTarget );
      NTMCopyAndClean( pSubstProp->szPropTarget, pProposal->szTarget );
      if ( fSourceAndTargetEqual )
      {
        pSubstProp->pTagsPropSource = (PTMX_TAGTABLE_RECORD)pMatchSentence->pTagRecord;
      }
      else
      {
        pSubstProp->pTagsPropSource = (PTMX_TAGTABLE_RECORD)pMatchTarget->pTagRecord;
      } /* endif */

      strcpy( pSubstProp->szPropTagTable, pProposal->szMarkup );
      strcpy( pSubstProp->szTargetLanguage, pProposal->szTargetLanguage );

		  fSubstAll =  NTMTagSubst( pSubstProp, ulSrcOemCP, ulTgtOemCP );
#ifdef MATCHLEVEL_LOGGING
      if ( hfLog )
      {
        fprintf( hfLog, "  generic inline tag replacement\n" );
        fprintf( hfLog, "   pSubstProp->szSource  = %S\n", pSubstProp->szSource);
        fprintf( hfLog, "   pSubstProp->szPropSource = %S\n", pSubstProp->szPropSource);
        fprintf( hfLog, "   pSubstProp->szPropTarget = %S\n", pSubstProp->szPropTarget);
        fprintf( hfLog, "   ulSrcOemCP            = %lu\n", ulSrcOemCP );
        fprintf( hfLog, "   ulTgtOemCP            = %lu\n", ulTgtOemCP );
        fprintf( hfLog, "   fSubstAll             = %s\n", fSubstAll ? "true" : "false " );
      } /* endif */
#endif

      if ( fSubstAll )
      {
        // recompute fuzziness
        BOOL fFuzzynessOK;
        fFuzzynessOK = TMFuzzyness( pSegment->szMarkup, pSegment->szSource, pSubstProp->szPropSource, sLangID, &usFuzzy, ulSrcOemCP );
#ifdef MATCHLEVEL_LOGGING
        if ( hfLog )
        {
          fprintf( hfLog, "  results of fuzziness test after generic inline tag replacement\n" ); 
          fprintf( hfLog, "   fFuzzynessOK          = %s\n", fFuzzynessOK ? "true" : "false " );
          fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
        } /* endif */
#endif
      } /* endif */
    } /* endif */
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liGeneric );
#endif

  // return result to caller
  if ( !usRC )
  {
    *psMatchLevel = (SHORT)usFuzzy;
#ifdef MATCHLEVEL_LOGGING
    if ( hfLog )
    {
      fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
      fprintf( hfLog, "   *psMatchLevel         = %d\n", *psMatchLevel );
    } /* endif */
#endif
    if ( usFuzzy == 100 )
    {
      // check type of exact match
      if ( (pSegment->lSegNumber == pProposal->lSegNumber ) && 
            (_stricmp( pSegment->szDocument, pProposal->szDocument ) == 0) )
      {
        *psMatchState = EXACTEXACT_MATCHSTATE;
      }
      else
      {
        *psMatchState = EXACT_MATCHSTATE;
      } /* endif */
#ifdef MATCHLEVEL_LOGGING
    if ( hfLog )
    {
      fprintf( hfLog, " after setting match state\n" );
      fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
      fprintf( hfLog, "   *psMatchLevel         = %d\n", *psMatchLevel );
    } /* endif */
#endif
    }
    else
    {
      // do checking for a replacement match
      USHORT usReplMatch = 0;             // buffer for replace flag
      PVOID  pvTagTable = NULL;
      SHORT  sTgtLangID = 0;               // morph. ID for target language
      USHORT usNewFuzzy = 0;

      usRC = TALoadTagTableHwnd( pSegment->szMarkup, (PLOADEDTABLE *)&pvTagTable, FALSE, TRUE, HWND_FUNCIF );

      if ( !usRC )
      {
        if ( strcmp( pProposal->szTargetLanguage, pProposal->szSourceLanguage ) == 0 )
        {
          sTgtLangID = sLangID;
        }
        else if ( MorphGetLanguageID( pProposal->szTargetLanguage, &sTgtLangID ) != 0 )
        {
          PSZ pszParm = pProposal->szTargetLanguage;
          usRC = ERROR_INV_LANGUAGE;
          UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        } /* endif */
      } /* endif */

#ifdef MATCHLEVEL_LOGGING
    if ( hfLog )
    {
      fprintf( hfLog, " before prepare Fuzzy prop\n" );
      fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
      fprintf( hfLog, "   *psMatchLevel         = %d\n", *psMatchLevel );
    } /* endif */
#endif
      // EQFBPrepareFuzzyProp modifies the target so we have to use
      // a temporary copy of the target segment
      if ( !usRC )
      {
        UTF16strcpy( pData->szSegment, pProposal->szTarget );
        usReplMatch = EQFBPrepareFuzzyPropEx2(
          pSegment->szSource,                        // pointer to source
          pProposal->szSource,                       // pointer to source of proposal
          pData->szSegment,                          // pointer to Translation
          sLangID,                                   // source language Id..
          sTgtLangID,                                // target language id..
          &usNewFuzzy,                               // we are looking for fuzzy level
          pvTagTable,                                // source document tag table
          pvTagTable,                                // target document tag table
          pData->bBuffer,                            // buffer for input
          pData->bTokenBuffer,                       // buffer for tokens
          ulSrcOemCP,
          ulTgtOemCP);
      } /* endif */

#ifdef MATCHLEVEL_LOGGING
    if ( hfLog )
    {
      fprintf( hfLog, " after prepare Fuzzy prop\n" );
      fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
      fprintf( hfLog, "   *psMatchLevel         = %d\n", *psMatchLevel );
    } /* endif */
#endif
      if ( pvTagTable ) TAFreeTagTable( (PLOADEDTABLE)pvTagTable );

      if ( usReplMatch == PROP_REPLACED )
      {
        *psMatchState = REPLACE_MATCHSTATE;  
      }
      else if ( usReplMatch == PROP_REPLACED_FUZZY )
      {
        *psMatchState = FUZZYREPLACE_MATCHSTATE;
      }
      else 
      {
        // check fuzziness
        LONG lFuzzinessThreshHold = 5000;

        if ( usWords < 5 )
        {
          lFuzzinessThreshHold = (LONG)UtlQueryULong( QL_SMALLLOOKUPFUZZLEVEL  );
        }
        else if ( usWords >= 15 )
        {
          lFuzzinessThreshHold = (LONG)UtlQueryULong( QL_LARGELOOKUPFUZZLEVEL );
        }
        else
        {
          lFuzzinessThreshHold = (LONG)UtlQueryULong( QL_MEDIUMLOOKUPFUZZLEVEL );
        } /* endif */

        if ( (usFuzzy * 100) >= lFuzzinessThreshHold )
        {
          *psMatchState = FUZZY_MATCHSTATE;
        }
        else
        {
          *psMatchState = NONE_MATCHSTATE;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

#ifdef MATCHLEVEL_LOGGING
    if ( hfLog )
    {
      fprintf( hfLog, "  returning results to caller\n" ); 
      fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
      fprintf( hfLog, "   *psMatchLevel         = %d\n", *psMatchLevel );
      fprintf( hfLog, "   *psMatchState         = %d\n", *psMatchState  );
    } /* endif */
#endif

#ifdef MEASURETIME
  QueryPerformanceCounter( &liReplace );
  dwEnd = GetTickCount();
#endif

#ifdef MEASURETIME
  {
    CHAR szLogFile[MAX_EQF_PATH];
    FILE *hfLog = NULL;

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szLogFile, 0L, FALSE );
    strcat( szLogFile, "\\MATCHLEVELTIME.LOG" );

    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      fprintf( hfLog, "NtmGetMatchLevel time log\n" );
      fprintf( hfLog, "Segment = \"%S\"\n", pSegment->szSource );
      fprintf( hfLog, "MemSeg  = \"%S\"\n", pProposal->szSource );
      LogWritePerfTime( hfLog, &liStart, &liPrepare, "Preparation" );
      LogWritePerfTime( hfLog, &liPrepare, &liGetLang, "GetLangID" );
      LogWritePerfTime( hfLog, &liGetLang, &liTokenize, "Tokenize" );
      LogWritePerfTime( hfLog, &liGetLang, &liMorphAct, "MorphAct" );
      LogWritePerfTime( hfLog, &liMorphAct, &liComp, "StringCompare" );
      LogWritePerfTime( hfLog, &liComp, &liGetFuzzy, "Fuzziness" );
      LogWritePerfTime( hfLog, &liGetFuzzy, &liGeneric, "GenericITR" );
      LogWritePerfTime( hfLog, &liGeneric, &liReplace, "ReplaceMatch" );
      LogWritePerfTime( hfLog, &liStart, &liReplace, "" );

      fprintf( hfLog, "Total time using GetTickCount : %ld ms\n", (long)(dwEnd - dwStart) );

      fclose( hfLog );
    } /* endif */
  }
#endif

  // cleanup
  NTMFreeSubstProp( pSubstProp );
  NTMFreeSentenceStructure( pSentence );
  NTMFreeSentenceStructure( pMatchSentence );
  NTMFreeSentenceStructure( pMatchTarget );
  if ( pData ) UtlAlloc( (PVOID*)&pData, 0L, 0L, NOMSG);

#ifdef MATCHLEVEL_LOGGING
  if ( hfLog )
  {
    fprintf( hfLog, "*** end of EqfGetMatchLevel, rc = %u ***\n", usRC );
    fclose( hfLog );
  } /* endif */
#endif

  return( usRC );
} /* end of function NTMGetMatchLevel */ 

// utility to allocate pSentence and associated memory areas
USHORT NTMAllocSentenceStructure
(
  PTMX_SENTENCE  *ppSentence
)
{
  USHORT usRc = 0;
  BOOL fOK = TRUE;
  PTMX_SENTENCE pSentence;

  //allocate pSentence
  fOK = UtlAlloc( (PVOID *)ppSentence, 0L, (LONG)sizeof( TMX_SENTENCE ), NOMSG );
  pSentence = *ppSentence;

  if ( fOK ) fOK = UtlAlloc( (PVOID *) &(pSentence->pInputString), 0L, (LONG)( MAX_SEGMENT_SIZE * sizeof(CHAR_W)), NOMSG );
  if ( fOK ) fOK = UtlAlloc( (PVOID *) &(pSentence->pNormString), 0L, (LONG)( MAX_SEGMENT_SIZE * sizeof(CHAR_W)), NOMSG );
  if ( fOK ) fOK = UtlAlloc( (PVOID *) &(pSentence->pulVotes), 0L, (LONG)(ABS_VOTES * sizeof(ULONG)), NOMSG );
  if ( fOK ) fOK = UtlAlloc( (PVOID *) &pSentence->pTagRecord, 0L, (LONG)(2*TOK_SIZE), NOMSG);
  if ( fOK ) fOK = UtlAlloc( (PVOID *) &pSentence->pTermTokens, 0L, (LONG)TOK_SIZE, NOMSG );

  if ( fOK )
  {
    pSentence->lTermAlloc = (LONG)TOK_SIZE;
    pSentence->lTagAlloc = (LONG)(2*TOK_SIZE);
    pSentence->pNormStringStart = pSentence->pNormString;
  } /* endif */

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  return( usRc );
} /* end of funcion NTMAllocSentenceStructure */

// utility to free pSentence and associated memory areas
VOID NTMFreeSentenceStructure
(
  PTMX_SENTENCE  pSentence
)
{
  //release memory
  if ( pSentence )
  {
    UtlAlloc( (PVOID *) &pSentence->pTermTokens, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pSentence->pTagRecord, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pSentence->pInputString, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pSentence->pNormStringStart, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pSentence->pulVotes, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pSentence->pPropString, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pSentence->pTermTokens, 0L, 0L, NOMSG ); 
    UtlAlloc( (PVOID *) &pSentence, 0L, 0L, NOMSG );
  } /* endif */
} /* end of funcion NTMFreeSentenceStructure */

// free tag substitution data area
void NTMFreeSubstProp( PTMX_SUBSTPROP pSubstProp )
{
  if ( pSubstProp )
  {
    if ( pSubstProp->pTokSource )
      UtlAlloc( (PVOID*) &pSubstProp->pTokSource, 0L, 0L, NOMSG );
    if ( pSubstProp->pTokPropSource )
      UtlAlloc( (PVOID*) &pSubstProp->pTokPropSource, 0L, 0L, NOMSG );
    if ( pSubstProp->pTokPropTarget )
      UtlAlloc( (PVOID*) &pSubstProp->pTokPropTarget, 0L, 0L, NOMSG );
    if ( pSubstProp->pTagPairs )
      UtlAlloc( (PVOID*) &pSubstProp->pTagPairs, 0L, 0L, NOMSG );
    if ( pSubstProp->pDelTagPairs )
      UtlAlloc( (PVOID*) &pSubstProp->pDelTagPairs, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID*)&pSubstProp, 0L, 0L, NOMSG);
  } /* endif */
  return;
} /* end of function NTMFreeSubstProp */


// build tag table path name
void NTMMakeTagTablename( PSZ pszMarkup, PSZ pszTagTableName )
{
  UtlMakeEQFPath( pszTagTableName, NULC, TABLE_PATH, NULL );
  strcat( pszTagTableName, BACKSLASH_STR );
  strcat( pszTagTableName, pszMarkup );
  strcat( pszTagTableName, EXT_OF_FORMAT );
  return;
}

SHORT NTMSimpleGetMatchLevel
(
  WCHAR            *pszSegmentText,    // segment text
  WCHAR            *pszProposalText,   // memory proposal text
  PSZ              pszLanguage,        // source language of segment and memory proposal
  PSZ              pszSegmentMarkup,   // markup table name of segment
  PSZ              pszProposalMarkup,  // markup table name of memory proposal
  LONG             lOptions,           // options to be used by the API call
  PVOID            *pvData            // adress of our data area pointer 

)
{
  USHORT           usRC = 0;                     
  PTMX_SENTENCE    pSentence = NULL;
  PTMX_SENTENCE    pMatchSentence = NULL;
  ULONG            ulParm = 0;                   // set here specific parameters such as RESPECT_CRLF
  BOOL             fStringEqual = FALSE;         // TRUE = strings are equal
  ULONG            ulSrcOemCP = 0;               // OEM codepage for source language
  ULONG            ulTgtOemCP = 0;               // OEM codepage for target language
  SHORT            sLangID = 0;                  // morph. ID for source language
  USHORT           usFuzzy = 0;                  // buffer for computed fuziness
  BOOL             fTagTableEqual = FALSE;       // TRUE = tag tables are equal
  PNTMGETMATCHLEVELDATA pData = NULL;
  BOOL             fGenericTagReplacement = FALSE; // TRUE = apply generic inline tag replacement
  USHORT           usWords = 0;                  // number of words in segment
  BOOL             fSourceAndTargetEqual = FALSE; // TRUE = source and target of propsal are equal
  BOOL             fSourceSource = FALSE;        // TRUE = source and target language of propsal are equal


#ifdef MEASURETIME
  LARGE_INTEGER liStart;
  LARGE_INTEGER liPrepare;
  LARGE_INTEGER liTokenize;
  LARGE_INTEGER liGetLang;
  LARGE_INTEGER liMorphAct;
  LARGE_INTEGER liComp;
  LARGE_INTEGER liGetFuzzy;
  LARGE_INTEGER liGeneric;
  LARGE_INTEGER liReplace;
  DWORD dwStart, dwEnd;
#endif
  
  pvData;

#ifdef MEASURETIME
  dwStart = GetTickCount();
  QueryPerformanceCounter( &liStart );
#endif

  // allocate our data area
  if ( !usRC ) usRC = (UtlAlloc( (PVOID*)&pData, 0L, (LONG)sizeof(NTMGETMATCHLEVELDATA), NOMSG)) ? 0 : ERROR_NOT_ENOUGH_MEMORY;

  // allocate sentence structures
  if ( !usRC ) usRC = NTMAllocSentenceStructure( &pSentence );
  if ( !usRC ) usRC = NTMAllocSentenceStructure( &pMatchSentence );

  // stop processing if languages do not match 
  if ( !usRC )
  {
    Utlstrccpy( pData->szLang1, pszLanguage, '(' );
    Utlstrccpy( pData->szLang2, pszLanguage, '(' );
  } /* endif */

  // check if generic inline tag replacement should be used
  if ( lOptions & NO_GENERIC_INLINETAG_REPL_OPT )
  {
    // switched off using lOptions
    fGenericTagReplacement = FALSE;
  }
  else if ( lOptions & USE_GENERIC_INLINETAG_REPL_OPT )
  {
    // switched on using lOptions
    fGenericTagReplacement = TRUE;
  }
  else
  {
    // base on setting in system properties
    PPROPSYSTEM pSysProp;             // ptr to EQF system properties
    pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
    fGenericTagReplacement = !pSysProp->fNoGenericMarkup;
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liPrepare );
#endif

  if ( strcmp( pszLanguage, szLastSourceLang ) == 0 )
  {
    ulSrcOemCP = ulLastSrcOemCP;
  }
  else
  {
    ulLastSrcOemCP = ulSrcOemCP = GetLangOEMCP( pszLanguage );
    strcpy( szLastSourceLang, pszLanguage );
  } /* endif */

    ulTgtOemCP = ulSrcOemCP;
    fSourceSource = TRUE;         // source and target languages are equal

#ifdef MEASURETIME
  QueryPerformanceCounter( &liGetLang );
#endif

  fSourceAndTargetEqual = TRUE;

  // tokenize source segment and match, resulting in normalized string and tag table record
  NTMMakeTagTablename( pszSegmentMarkup, pData->szSegmentTagTable );
  NTMMakeTagTablename( pszProposalMarkup, pData->szProposalTagTable );

  UTF16strcpy( pSentence->pInputString, pszSegmentText );
  usRC = TokenizeSourceEx( NULL, pSentence, pData->szSegmentTagTable, pszLanguage, TM_MAJ_VERSION, ulSrcOemCP );
  UTF16strcpy( pMatchSentence->pInputString, pszProposalText );
  if ( !usRC ) usRC = TokenizeSourceEx( NULL, pMatchSentence, pData->szProposalTagTable, pszLanguage, TM_MAJ_VERSION, ulSrcOemCP );

#ifdef MEASURETIME
  QueryPerformanceCounter( &liTokenize );
#endif

  if ( MorphGetLanguageID( pszLanguage, &sLangID ) != 0 )
  {
    PSZ pszParm = pszLanguage;
    usRC = ERROR_INV_LANGUAGE;
    UtlErrorHwnd( ERROR_INV_LANGUAGE, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liMorphAct );
#endif


  fTagTableEqual = (_stricmp( pszSegmentMarkup, pszProposalMarkup ) == 0);
  // For R012645 begin
  if(!fTagTableEqual)
  {
    std::string segMarkupFamily = GetAndCacheFamilyName(pszSegmentMarkup);
    if(!segMarkupFamily.empty())
    {
      std::string propMarkupFamily = GetAndCacheFamilyName(pszProposalMarkup);
      if(!propMarkupFamily.empty() && segMarkupFamily==propMarkupFamily)
        fTagTableEqual = TRUE;
    }
  }
  // For R012645 end

  // check if strings are equal
  if ( ulParm & GET_RESPECTCRLF )   
  {
	  BOOL fRespectCRLFStringEqual = (UtlCompIgnSpaceW( pMatchSentence->pNormString, pSentence->pNormString, 0 )== 0L);
	  fStringEqual = (UtlCompIgnWhiteSpaceW( pMatchSentence->pNormString, pSentence->pNormString, 0 ) == 0L);
	  if (fStringEqual && !fRespectCRLFStringEqual)
	  {  
      // there is a LF difference!
      fStringEqual = fRespectCRLFStringEqual;
	  }
  }
  else
  { 
    fStringEqual = (UtlCompIgnWhiteSpaceW( pMatchSentence->pNormString, pSentence->pNormString, 0 ) == 0L);
  } /* endif*/

#ifdef MEASURETIME
  QueryPerformanceCounter( &liComp );
#endif

  // compute fuzziness
  if ( !usRC )
  {
    BOOL fFuzzynessOK;
    fFuzzynessOK = TMFuzzynessEx( pszSegmentMarkup, pszSegmentText, pszProposalText, sLangID, &usFuzzy, ulSrcOemCP, &usWords );
    if ( usFuzzy != 100) fStringEqual = FALSE;
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liGetFuzzy );
#endif

  // perform generic inline tag replacement
  if ( !fStringEqual && !usRC && fGenericTagReplacement )
  {
    BOOL fSubstAll = FALSE;

    // do not enter generic inline tag replacement when proposal does not contain any matches
    PTMX_TAGTABLE_RECORD pTagRecord = (PTMX_TAGTABLE_RECORD)pMatchSentence->pTagRecord;
    if ( pTagRecord->ulRecordLen > sizeof(TMX_TAGTABLE_RECORD) )
    {
      PTMX_SUBSTPROP   pSubstProp = NULL;            // data area for genreic inline tag replacment 

      // allocate gerenic inline replacement tag data area
      if ( !usRC ) usRC = (UtlAlloc( (PVOID*)&pSubstProp, 0L, (LONG)sizeof(TMX_SUBSTPROP), NOMSG)) ? 0 : ERROR_NOT_ENOUGH_MEMORY;

      strcpy( pSubstProp->szSourceTagTable, pszSegmentMarkup );
      strcpy( pSubstProp->szSourceLanguage, pszLanguage);
      UTF16strcpy( pSubstProp->szSource, pszSegmentText );
      pSubstProp->pTagsSource = (PTMX_TAGTABLE_RECORD)pSentence->pTagRecord;

      UTF16strcpy( pSubstProp->szPropSource, pszProposalText );
      pSubstProp->pTagsPropTarget = (PTMX_TAGTABLE_RECORD)pMatchSentence->pTagRecord;

      pSubstProp->pTagsPropSource = (PTMX_TAGTABLE_RECORD)pMatchSentence->pTagRecord;

      strcpy( pSubstProp->szPropTagTable, pszProposalMarkup );
      strcpy( pSubstProp->szTargetLanguage, pszLanguage );

		  fSubstAll =  NTMTagSubst( pSubstProp, ulSrcOemCP, ulTgtOemCP );

      if ( fSubstAll )
      {
        // recompute fuzziness
        BOOL fFuzzynessOK;
        fFuzzynessOK = TMFuzzyness( pszSegmentMarkup, pSubstProp->szPropSource, pSubstProp->szSource, sLangID, &usFuzzy, ulSrcOemCP );
      } /* endif */

      NTMFreeSubstProp( pSubstProp );
    } /* endif */
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liGeneric );
#endif

  // return result to caller
  if ( !usRC )
  {
    if ( usFuzzy == 100 )
    {
    }
    else
    {
      // do checking for a replacement match
      USHORT usReplMatch = 0;             // buffer for replace flag
      PVOID  pvTagTable = NULL;
      SHORT  sTgtLangID = 0;               // morph. ID for target language
      USHORT usNewFuzzy = 0;

      usRC = TALoadTagTableHwnd( pszSegmentMarkup, (PLOADEDTABLE *)&pvTagTable, FALSE, TRUE, HWND_FUNCIF );

      if ( !usRC )
      {
        sTgtLangID = sLangID;
      } /* endif */

      // EQFBPrepareFuzzyProp modifies the target so we have to use
      // a temporary copy of the target segment
      if ( !usRC )
      {
        UTF16strcpy( pData->szSegment, pszProposalText );
        usReplMatch = EQFBPrepareFuzzyPropEx2(
          pszSegmentText,                        // pointer to source
          pszProposalText,                       // pointer to source of proposal
          pData->szSegment,                          // pointer to Translation
          sLangID,                                   // source language Id..
          sTgtLangID,                                // target language id..
          &usNewFuzzy,                               // we are looking for fuzzy level
          pvTagTable,                                // source document tag table
          pvTagTable,                                // target document tag table
          pData->bBuffer,                            // buffer for input
          pData->bTokenBuffer,                       // buffer for tokens
          ulSrcOemCP,
          ulTgtOemCP);
      } /* endif */

      if ( pvTagTable ) TAFreeTagTable( (PLOADEDTABLE)pvTagTable );

      if ( usReplMatch == PROP_REPLACED )
      {
        usFuzzy = usNewFuzzy;
      }
      else if ( usReplMatch == PROP_REPLACED_FUZZY )
      {
        usFuzzy = usNewFuzzy;
      } /* endif */
    } /* endif */
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liReplace );
  dwEnd = GetTickCount();
#endif

#ifdef MEASURETIME
  {
    CHAR szLogFile[MAX_EQF_PATH];
    FILE *hfLog = NULL;

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szLogFile, 0L, FALSE );
    strcat( szLogFile, "\\MATCHLEVELTIME.LOG" );

    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      fprintf( hfLog, "NtmSimpleGetMatchLevel time log\n" );
      fprintf( hfLog, "Segment = \"%S\"\n", pszSegmentText );
      fprintf( hfLog, "MemSeg  = \"%S\"\n", pszProposalText );
      fprintf( hfLog, "MatchLevel = %u\n", usFuzzy );
      LogWritePerfTime( hfLog, &liStart, &liPrepare, "Preparation" );
      LogWritePerfTime( hfLog, &liPrepare, &liGetLang, "GetLangID" );
      LogWritePerfTime( hfLog, &liGetLang, &liTokenize, "Tokenize" );
      LogWritePerfTime( hfLog, &liGetLang, &liMorphAct, "MorphAct" );
      LogWritePerfTime( hfLog, &liMorphAct, &liComp, "StringCompare" );
      LogWritePerfTime( hfLog, &liComp, &liGetFuzzy, "Fuzziness" );
      LogWritePerfTime( hfLog, &liGetFuzzy, &liGeneric, "GenericITR" );
      LogWritePerfTime( hfLog, &liGeneric, &liReplace, "ReplaceMatch" );
      LogWritePerfTime( hfLog, &liStart, &liReplace, "" );

      fprintf( hfLog, "Total time using GetTickCount : %ld ms\n", (long)(dwEnd - dwStart) );

      fclose( hfLog );
    } /* endif */
  }
#endif

  // cleanup
  NTMFreeSentenceStructure( pSentence );
  NTMFreeSentenceStructure( pMatchSentence );
  if ( pData ) UtlAlloc( (PVOID*)&pData, 0L, 0L, NOMSG);

  return( (usRC == 0) ? (SHORT)usFuzzy : ((SHORT)usRC * -1) );
}

SHORT NTMSimpleGetMatchLevelCleanup
(
  PVOID            *pvData             // adress of our data area pointer 
)
{
  pvData;

  return( 0 );
}

#ifdef MEASURETIME

static LARGE_INTEGER liLast = { 0 };
static LARGE_INTEGER liFrequency = { 0 };

static void GetElapsedTime( LONG64 *plTime )
{
  LARGE_INTEGER liCurrent;

  QueryPerformanceCounter( &liCurrent );

  if ( liFrequency.QuadPart == 0)
  {
    QueryPerformanceFrequency( &liFrequency );
  } /* endif */

  if ( liLast.QuadPart != 0 )
  {
    if ( liFrequency.QuadPart != 0 )
    {
      LONGLONG ldwDiff = liCurrent.QuadPart - liLast.QuadPart;
      DWORD dwTime = (DWORD)((ldwDiff * 1000000) / liFrequency.QuadPart);
      *plTime = *plTime + dwTime;
    }
  } /* endif */
  liLast.QuadPart = liCurrent.QuadPart;
} /* end of function GetElapsedTime */

#endif

// function aligning tags between source and target of a proposal
BOOL NTMAlignTags
( 
  PFUZZYTOK   pTokListSource,                     // list of proposal source token
  PFUZZYTOK   pTokListTarget,                    // list of proposal target token
  PREPLLIST   *ppReplaceList                     // adress of caller's replacement list pointer
)
{
  PREPLLIST    pReplaceList = NULL;              // pointer to replace list
  int          iSourceTags = 0;                  // number of tags in source token list
  PFUZZYTOK    pTokSource = pTokListSource;      // pointer for source token processing
  PFUZZYTOK    pTokTarget = pTokListTarget;      // pointer for target token processing
  BOOL         fOK = TRUE;                       // function return code

  // count number of tags in source of proposal
  if ( fOK )
  {
    while ( pTokSource->ulHash )
    {
      if ( pTokSource->sType == TAG_TOKEN )
      {
        iSourceTags++;
      } /* endif */
      pTokSource++; 
    } /*endwhile */
  } /* endif */

  // allocate replacement list
  if ( fOK )
  {
    int iSize = max( MAX_REPL, iSourceTags ) + 1;
    fOK = UtlAlloc( (PVOID *)ppReplaceList, 0L, iSize * sizeof(REPLLIST), ERROR_STORAGE );
    pReplaceList = *ppReplaceList;
  } /* endif */

  // look for source tags in proposal target
  if ( fOK )
  {
    PREPLLIST    pReplaceEntry = pReplaceList; 

    pTokSource = pTokListSource;      
    pTokTarget = pTokListTarget;
    while ( pTokSource->ulHash )
    {
      if ( pTokSource->sType == TAG_TOKEN )
      {
        BOOL fFound = FALSE;
        PFUZZYTOK pTargetStart = pTokTarget;
        do
        {
          if ( (pTokTarget->sType == TAG_TOKEN) && (pTokTarget->ulHash == pTokSource->ulHash) && !pTokTarget->fConnected )
          {
            pReplaceEntry->pSrcTok = pTokSource;
            pReplaceEntry->pTgtTok = pTokTarget;
            pTokSource->fConnected = TRUE;
            pTokTarget->fConnected = TRUE;
            pReplaceEntry++;
            fFound = TRUE;
          }
          else
          {
            // try next target token, wrap-around at end of list
            pTokTarget++;
            if ( !pTokTarget->ulHash )
            {
              pTokTarget = pTokListTarget;
            } /* endif */
          } /* endif */
        } while ( !fFound && (pTargetStart != pTokTarget) );  // while not found and not through list
      } /* endif */
      pTokSource++; 
    } /*endwhile */

    pTokTarget = pTokListTarget;
    pReplaceList = NULL;              // pointer to replace list
  } /* endif */

  return( fOK );
} /* end of function NTMAlignTags */

// If existed, return it, otherwise it will determine markup family name then cache it
std::string& GetAndCacheFamilyName(char* pszMarkupName)
{
  if( Name2FamilyMap.find(pszMarkupName) == Name2FamilyMap.end() )
  {
    OtmMarkup *pMarkup = GetMarkupObject( pszMarkupName, NULL);
    if( pMarkup != NULL )
      Name2FamilyMap[pszMarkupName] = pMarkup->getFamilyName();
    else
      Name2FamilyMap[pszMarkupName] = "";
  }
  return Name2FamilyMap[pszMarkupName];
}
