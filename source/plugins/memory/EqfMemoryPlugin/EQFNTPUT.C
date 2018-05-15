/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#define INCL_EQF_ASD     
#include <eqf.h>                  // General Translation Manager include file

#include <process.h>              // for exit function

#define INCL_EQFMEM_DLGIDAS
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFMORPI.H>

#include <EQFEVENT.H>             // event logging


//static data
//distribution criteria for building tuples
static USHORT usTLookUp[24]= { 0,1,2, 0,1,3, 0,2,3,
                               0,0,1, 0,0,2, 0,1,1, 0,2,2, 0,0,0  };
typedef enum
 {
   REC_CLB,
   REC_SRCTAGTABLE,
   REC_TGTTAGTABLE,
   REC_NEXTTARGET,
   REC_TGTSTRING,
   REC_SRCSTRING,
   REC_FIRSTTARGET
 } RECORD_POSITION;

static
PBYTE NTRecPos(PBYTE pStart, int iType);

USHORT NTMAdjustAddDataInTgtCLB
	(
		PTMX_RECORD		   *ppTmRecordStart,
		PULONG     		   pulRecBufSize,
	    PTMX_PUT_W 		   pTmPut,
		PTMX_TARGET_CLB    *ppClb,
		PTMX_RECORD        *ppCurTmRecord,
		PTMX_TARGET_RECORD	 *ppTMXTargetRecord,
	    PULONG  		 pulLeftClbLen,
 		PBOOL 			 pfUpdate);

USHORT TMLoopAndDelTargetClb
	(
		PTMX_RECORD         pTmRecord,
		PTMX_PUT_W 			    pTmPut,
    PTMX_SENTENCE       pSentence,
    USHORT              usPutLang,
    USHORT              usPutFile,
    PBOOL               fNewerTargetExists
	);
/**********************************************************************/
/* activate this define to get a faster memory                        */
/**********************************************************************/
#define NTMFAST_TOKENIZE 1

#ifdef NTMFAST_TOKENIZE
USHORT NTMMorphTokenize
(
   SHORT    sLanguageID,               // language ID
   PSZ      pszInData,                 // pointer to input segment
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType,                // type of term list MORPH_ZTERMLIST or
                                       //    MORPH_OFFSLIST
   USHORT usVersion                    // version of TM
);

  /********************************************************************/
  /* get the substitute for the isAlNum function                      */
  /********************************************************************/
//  extern CHAR chIsText[];
  /**********************************************************************/
  /* activate optimization                                              */
  /**********************************************************************/
//  #pragma optimize("let", on )
#endif

// check for valid left length: ulLeft must be bigger or equal than ulRight
#define NTASSERTLEN(ulLeft, ulRight, iEvent)\
	if ( (ULONG)ulLeft < (ULONG)ulRight )                               \
	{                                                                   \
	  ERREVENT2( ADDTOTM_LOC, ERROR_EVENT, iEvent, TM_GROUP, NULL );    \
	  UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );   \
	  abort();                                                          \
	}


//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TmtXReplace      adds data to the tm database             
//------------------------------------------------------------------------------
// Description:       Adds new data to database or modifies existing data       
//------------------------------------------------------------------------------
// Function call:  TmtXReplace( PTMX_CLB pTmClb,    //ptr to ctl block struct   
//                             PTMX_PUT_IN pTmPutIn,  //ptr to input struct     
//                             PTMX_PUT_OUT pTmPutOut ) //ptr to output struct  
//------------------------------------------------------------------------------
// Input parameter:   PTMX_CLB  pTmClb         control block                    
//                    PTMX_PUT_IN pTmPutIn     input structure                  
//------------------------------------------------------------------------------
// Output parameter:  PTMX_PUT_OUT pTmPutOut   output structure                 
//------------------------------------------------------------------------------
// Returncode type:                                                             
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//                                                                              
//------------------------------------------------------------------------------
// Prerequesits:                                                                
//------------------------------------------------------------------------------
// Side effects:                                                                
//------------------------------------------------------------------------------
// Samples:                                                                     
//------------------------------------------------------------------------------
// Function flow:                                                               
//      get id of source language, call                                         
//        NTMGetIdFromName, returns pointer to id                               
//                                                                              
//      using parameters from the Tm_Put structure passed                       
//      call TokenizeSegment with szTagTable, szSource, SourceLangId,           
//        TagTable record, pNormString                                          
//                                                                              
//      call HashSentence with pNormString                                      
//                                                                              
//      work through ulong list of tuple hashes in sentence structure           
//        checking if on in compact area in TM control block                    
//      if all on                                                               
//        UpdateInTM( pstTmPut, pstSentence, pTmClb, pTagTable )                
//      else                                                                    
//        fill source record of tm_record (source string and tagtable struct)   
//        work through ulong hash list in pstSentence switching on              
//        respective bits  in compact area in pstTmClb                          
//        AddToTM( pszTmPut, pstTagTable, pstTmClb )                            
//        UpdateTmIndex( pstTmClb, ulHashList )                                 
//------------------------------------------------------------------------------
USHORT TmtXReplace
(
  PTMX_CLB pTmClb,         //ptr to ctl block struct
  PTMX_PUT_IN_W pTmPutIn,  //ptr to input struct
  PTMX_PUT_OUT_W pTmPutOut //ptr to output struct
)
{
  PTMX_SENTENCE  pSentence = NULL;     // ptr to sentence structure
  ULONG      ulNewKey = 0;             // sid of newly added tm record
  BOOL       fOK;                      // success indicator
  USHORT     usRc = NO_ERROR;          // return code
  USHORT     usMatchesFound;           // compact area hits
  CHAR       szString[MAX_EQF_PATH];   // character string
  BOOL        fLocked = FALSE;         // TM-database-has-been-locked flag
  BOOL         fUpdateOfIndexFailed = FALSE; // TRUE = update of index failed

  DEBUGEVENT2( TMTXREPLACE_LOC, FUNCENTRY_EVENT, 0, TM_GROUP, NULL );

  //allocate pSentence
  fOK = UtlAlloc( (PVOID *) &(pSentence), 0L, (LONG)sizeof( TMX_SENTENCE ), NOMSG );

  if ( fOK )
   fOK = UtlAlloc( (PVOID *) &(pSentence->pInputString), 0L,
                   (LONG)( MAX_SEGMENT_SIZE * sizeof(CHAR_W)), NOMSG );
  if ( fOK )
   fOK = UtlAlloc( (PVOID *) &(pSentence->pNormString), 0L,
                   (LONG)( MAX_SEGMENT_SIZE * sizeof(CHAR_W)), NOMSG );
  if ( fOK )
   fOK = UtlAlloc( (PVOID *) &(pSentence->pulVotes), 0L,
                   (LONG)(ABS_VOTES * sizeof(ULONG)), NOMSG );

  //allocate 4k for pTagRecord
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &pSentence->pTagRecord, 0L, (LONG)(TOK_SIZE*2), NOMSG);
    if ( fOK )
      pSentence->lTagAlloc = (LONG)(TOK_SIZE*2);
  } /* endif */

  //allocate 4k for pTermTokens
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &pSentence->pTermTokens, 0L, (LONG)TOK_SIZE, NOMSG );
    if ( fOK )
      pSentence->lTermAlloc = (LONG)TOK_SIZE;
  } /* endif */

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  if ( !usRc )
  {
    //build tag table path
    UtlMakeEQFPath( szString, NULC, TABLE_PATH, NULL );
    strcat( szString, BACKSLASH_STR );
    strcat( szString, pTmPutIn->stTmPut.szTagTable );
    strcat( szString, EXT_OF_FORMAT );

    //remember start of norm string
    pSentence->pNormStringStart = pSentence->pNormString;

    UTF16strcpy( pSentence->pInputString, pTmPutIn->stTmPut.szSource );

    //tokenize source segment, resulting in norm. string and tag table record
    usRc = TokenizeSource( pTmClb, pSentence, szString,
                           pTmPutIn->stTmPut.szSourceLanguage,
                           (USHORT)pTmClb->stTmSign.bMajorVersion );
    if ( strstr( szString, "OTMUTF8" ) ) {
       strcpy( pTmPutIn->stTmPut.szTagTable, "OTMUTF8" );
       pTmPutIn->stTmPut.fMarkupChanged = TRUE ;
    }
  } /* endif */

  if ( !usRc )
  {
    pSentence->pNormString = pSentence->pNormStringStart;
    HashSentence( pSentence, (USHORT)pTmClb->stTmSign.bMajorVersion, pTmClb->stTmSign.bMinorVersion );
    if ( pTmClb )  /* 4-13-15 */
    {
      usRc = NTMGetIDFromName( pTmClb, pTmPutIn->stTmPut.szTagTable, NULL, (USHORT)TAGTABLE_KEY, &pSentence->pTagRecord->usTagTableId );
    }
    else
    {
      pSentence->pTagRecord->usTagTableId = 0;
    } /* endif */
  } /* endif */

  // lock TM database
  if ( !usRc && pTmClb->fShared )
  {
    // use only two retries as locking already uses a wait loop...
    SHORT sRetries = 2;
    do
    {
      usRc = NTMLockTM( pTmClb, TRUE, &fLocked );
      if ( usRc == BTREE_IN_USE )
      {
        UtlWait( MAX_WAIT_TIME );
        sRetries--;
      } /* endif */
    }
    while( (usRc == BTREE_IN_USE) && (sRetries > 0));
  } /* endif */

  // Update internal buffers if database has been modified by other users
  if ( !usRc && pTmClb->fShared )
  {
    usRc = NTMCheckForUpdates( pTmClb );
  } /* endif */

  if ( !usRc )
  {
    usMatchesFound = CheckCompactArea( pSentence, pTmClb );
    if ( usMatchesFound == pSentence->usActVote ) //all hash triples found
    {
      //update entry in tm database
      usRc = UpdateTmRecord(pTmClb, &pTmPutIn->stTmPut, pSentence );

      //if no tm record fitted for update assume new and add to tm
      if ( usRc == ERROR_ADD_TO_TM )
      {
        usRc = AddToTm( pSentence, pTmClb, &pTmPutIn->stTmPut, &ulNewKey );
        //update index
        if ( !usRc )
        {
          usRc = UpdateTmIndex( pSentence, ulNewKey, pTmClb );
          if ( usRc ) fUpdateOfIndexFailed = TRUE;
        } /* endif */
      } /* endif */
    }
    else
    {
      //add new tm record to tm database
      usRc = AddToTm( pSentence, pTmClb, &pTmPutIn->stTmPut, &ulNewKey );

      //update index
      if ( !usRc )
      {
        usRc = UpdateTmIndex( pSentence, ulNewKey, pTmClb );
        if ( usRc ) fUpdateOfIndexFailed = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */

  // unlock TM database if database has been locked
  if ( fLocked )
  {
    NTMLockTM( pTmClb, FALSE, &fLocked );
  } /* endif */

  //release allocated memory
  UtlAlloc( (PVOID *) &pSentence->pNormStringStart, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pSentence->pInputString, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pSentence->pulVotes, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pSentence->pTagRecord, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pSentence->pTermTokens, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pSentence, 0L, 0L, NOMSG );

  pTmPutOut->stPrefixOut.usLengthOutput = sizeof( TMX_PUT_OUT );
  pTmPutOut->stPrefixOut.usTmtXRc = usRc;

  // special handling if update of index failed with BTREE_LOOKUPTABLE_TOO_SMALL:
  // delete the data record to ensure that any organize
  // of the TM can still be done without running into the
  // problem again
  if ( fUpdateOfIndexFailed && (usRc == BTREE_LOOKUPTABLE_TOO_SMALL) )
  {
    // try to delete the segment from the memory

    PTMX_PUT_OUT_W pstDelOut = NULL;
    PTMX_PUT_IN_W  pstDelIn = NULL;
    int iLen = sizeof (TMX_PUT_IN_W) + sizeof (TMX_PUT_OUT_W);
    if ( UtlAlloc( (PVOID *)&pstDelIn, 0L, (LONG)iLen, NOMSG ) )
    {
      pstDelOut = (PTMX_PUT_OUT_W)(pstDelIn+1);
    } /* endif */

    if ( pstDelIn )
    {
      UTF16strcpy( pstDelIn->stTmPut.szSource, pTmPutIn->stTmPut.szSource );
      UTF16strcpy(pstDelIn->stTmPut.szTarget, pTmPutIn->stTmPut.szTarget );
      strcpy(pstDelIn->stTmPut.szSourceLanguage, pTmPutIn->stTmPut.szSourceLanguage );
      strcpy(pstDelIn->stTmPut.szTargetLanguage, pTmPutIn->stTmPut.szTargetLanguage );
      pstDelIn->stTmPut.usTranslationFlag = pTmPutIn->stTmPut.usTranslationFlag;
      strcpy(pstDelIn->stTmPut.szFileName, pTmPutIn->stTmPut.szFileName );
      strcpy(pstDelIn->stTmPut.szLongName, pTmPutIn->stTmPut.szLongName );
      pstDelIn->stTmPut.ulSourceSegmentId = pTmPutIn->stTmPut.ulSourceSegmentId;
      strcpy(pstDelIn->stTmPut.szTagTable, pTmPutIn->stTmPut.szTagTable );
//       pstDelIn->stTmPut.lTime = pTmPutIn->stTmPut.lTargetTime;

      TmtXDelSegm( pTmClb, pstDelIn, pstDelOut );

      UtlAlloc( (PVOID *)&pstDelIn, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  if ( usRc )
  {
    ERREVENT2( TMTXREPLACE_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */
  return( usRc );
}





//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     HashSentence    Build and store hash value for terms      
//------------------------------------------------------------------------------
// Description:       Builds hash values for terms                              
//------------------------------------------------------------------------------
// Function call:     HashSentence( PTMX_SENTENCE pSentence ) //ptr sent struct 
//------------------------------------------------------------------------------
// Input parameter:   PTMX_SENTENCE pSentence     sentence structure            
//                    USHORT usVersion       version of TM                      
//------------------------------------------------------------------------------
// Output parameter:                                                            
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Prerequesits:                                                                
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Samples:                                                                     
//------------------------------------------------------------------------------
// Function flow:                                                               
//      build hash for each word in pNormString                                 
//        use adapted function in prototype                                     
//        HashTupel with pWord and length of word, returns filled out token     
//          record with offset, length and hash                                 
//                                                                              
//      build tuples out of these hash codes                                    
//        use function in prototype                                             
//        BuildVotes with token record of all words, return actual number of    
//         hash votes and ulong list of all the tuple hashes in sentence record 
//        (don't create all tuple combinations for a all tuples - rather work   
//          through all words and keep to MAX_VOTES)                            
//                                                                              
//------------------------------------------------------------------------------
VOID HashSentence
(
  PTMX_SENTENCE pSentence,          // pointer to sentence structure
  USHORT usMajVersion,               // major version of TM (req. for hash method)
  USHORT usMinVersion               // minor version of TM (req. for hash method)
)
{
  PSZ_W  pNormOffset;               // pointer to start of normalized string
  PTMX_TERM_TOKEN  pTermTokens;     // pointer to term token structure
  USHORT usCount = 0;               // counter

  pTermTokens = pSentence->pTermTokens;
  pNormOffset = pSentence->pInputString + pTermTokens->usOffset;

  while ( pTermTokens->usLength )
  {
    pTermTokens->usHash = HashTupelW( pNormOffset, pTermTokens->usLength, usMajVersion, usMinVersion );
    //max nr of hashes built
    usCount++;
    pTermTokens++;
    pNormOffset = pSentence->pInputString + pTermTokens->usOffset;
  } /* endwhile */

  //duplicate term tokens if not sufficient; this is for very short sentences
//while ( usCount < 3 )
//{
//  memcpy( pTermTokens, pTermTokens-1, sizeof( TMX_TERM_TOKEN ));
//  pTermTokens++;
//  usCount ++;
//} /* endwhile */

  /********************************************************************/
  /* if usCount < 4 add single tokens to allow matching simple        */
  /* sentences                                                        */
  /********************************************************************/
  if ( usCount < 5 )
  {
    USHORT  i;                         // index

    pTermTokens = pSentence->pTermTokens;
    for ( i=0; i < usCount; i++ )
    {
      pSentence->pulVotes[pSentence->usActVote] = pTermTokens[i].usHash;
      pSentence->usActVote++;
    } /* endfor */
  } /* endif */


  //build tuples of the term hashes
  if ( usCount >= 3 )
  {
    BuildVotes( pSentence );
  } /* endif */
}

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     HashTupel                                                 
//------------------------------------------------------------------------------
// Description:       hash the passed tupel into a hash number                  
//------------------------------------------------------------------------------
// Parameters:        PBYTE  pToken,         passed token                       
//                    USHORT usLen           length of tupel                    
//                    USHORT usVersion       version of TM                      
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       hash value                                                
//------------------------------------------------------------------------------
// Function flow:     build hash value of passed token (be careful with DBCS).  
//------------------------------------------------------------------------------

// HashtTupelW for memory version 7.1 and above
USHORT HashTupelW71
(
  PSZ_W  pToken,                       // passed token
  USHORT usLen                        // length of tupel
)
{
  USHORT usHash = 0;                   // hash value
  wchar_t c;                           // active character

  while ( usLen )
  {
    c = towlower( *pToken++ );
    if ( (c >= 'a') && (c <= 'z') )
    {
      usHash = (usHash * 131) + c-'a';
    }
    else
    {
      usHash = (usHash * 131) + c;
    } /* endif */
    usLen--;
  } /* endwhile */

  //ensure that usHash isn't zero
  if ( usHash == 0 )
  {
    usHash = 1;
  } /* endif */
  return (usHash);
} /* end of function HashTupel */

// HashtTupelW for memory versions up to and including 7.0
USHORT HashTupelW70
(
  PSZ_W  pToken,                       // passed token
  USHORT usLen                         // length of tupel
)
{
  USHORT usHash = 0;                   // hash value
  CHAR_W chHash[MAX_RANDOM];           // string to be hashed
  UCHAR   c;                           // active character

  usLen = min(usLen, MAX_RANDOM - 1);

  // get a copy of the token to be normalized and hashed
  memcpy(chHash, pToken, usLen * sizeof(CHAR_W));
  chHash[usLen] = EOS;

  pToken = UtlLowerW( chHash );

    // new approach to distinguish non-alphabetic characters
  while ( (c = (UCHAR)*pToken++) != NULC )
  {
    if ( isalpha(c) )
    {
      usHash = (usHash * 131) + c-'a';
    }
    else
    {
      usHash = (usHash * 131) + c;
    } /* endif */
  } /* endwhile */

  //ensure that usHash isn't zero
  if ( usHash == 0 )
  {
    usHash = 1;
  } /* endif */
  return (usHash);
} /* end of function HashTupelW70 */

// HashtTupelW 
USHORT HashTupelW
(
  PSZ_W  pToken,                       // passed token
  USHORT usLen,                        // length of tupel
  USHORT usMajVersion,                 // major version of TM 
  USHORT usMinVersion                  // minor version of TM 
)
{
  if ( (usMajVersion > TM_MAJ_VERSION_7) || ((usMajVersion == TM_MAJ_VERSION_7) && (usMinVersion >= TMMIN_VERSION_1)) )
  {
    return( HashTupelW71( pToken, usLen ) );
  }
  else
  {
    return( HashTupelW70( pToken, usLen ) );
  } /* endif */
}

USHORT
HashTupel
(
  PBYTE  pToken,                       // passed token
  USHORT usLen,                        // length of tupel
  USHORT usVersion                     // version of TM (req. for hash method)
)
{
  USHORT usHash = 0;                   // hash value
  CHAR   chHash[MAX_RANDOM];           // string to be hashed
  UCHAR   c;                           // active character

  usLen = min(usLen, MAX_RANDOM - 1);

  // get a copy of the token to be normalized and hashed
  memcpy(chHash, pToken, usLen);
  chHash[usLen] = EOS;

  pToken = (PBYTE)UtlLower( chHash );

  if ( usVersion == TM_VERSION_1 )
  {
    // hashing algorithm found in 'Handbook of Algorithm' by Gonnet,p.48
    while ( (c = *pToken++)!= NULC )
    {
      if ( isalpha(c) )
      {
        usHash = (usHash * 131) + c-'a';
      }
      else
      {
        usHash = (usHash * 131) + 27;
      } /* endif */
    } /* endwhile */
  }
  else
  {
    // new approach to distinguish non-alphabetic characters
    while ( (c = *pToken++)!= NULC )
    {
      if ( isalpha(c) )
      {
        usHash = (usHash * 131) + c-'a';
      }
      else
      {
        usHash = (usHash * 131) + c;
      } /* endif */
    } /* endwhile */
  } /* endif */

  //ensure that usHash isn't zero
  if ( usHash == 0 )
  {
    usHash = 1;
  } /* endif */
  return (usHash);
} /* end of function HashTupel */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     BuildVotes                                                
//------------------------------------------------------------------------------
// Description:       build a histogram of possible matching sentences/phrases  
//                    and sort it in descending order                           
//------------------------------------------------------------------------------
// Parameters:        PTMX_SENTENCE pstSentence  pointer to sentence structure  
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     if not enough tokens (at least 3) we have to simulate     
//                    a creation of at least one triple.                        
//                    Call vote routine as long as there are tokens available.  
//                    Comments indicate which triples are build for a sentence  
//                    with 7 words A B C D E F G                                
//                    PREREQ: it is ensured that sentence has at least 3 tokens 
//------------------------------------------------------------------------------
static VOID
BuildVotes
(
  PTMX_SENTENCE pSentence              // pointer to sentence structure
)
{
  PTMX_TERM_TOKEN  pTermTokens;        // pointer to active token
  USHORT           usIndex = 0;
  PTMX_TERM_TOKEN  pLastTerm;

  //run through list of tokens and build tuples
  pTermTokens = pSentence->pTermTokens;

  while ( pTermTokens->usLength && (pSentence->usActVote < ABS_VOTES) )
  {
    Vote( pTermTokens, pSentence, 0 );         // ABC, BCD, CDE, DEF, EFG
    pTermTokens++;
  } /* endwhile */
  pLastTerm = pTermTokens;
  pLastTerm --;

  pTermTokens = pSentence->pTermTokens;

  while ( pTermTokens->usLength && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote( pTermTokens, pSentence, 1 );         // ABD, BCE, CDF, DEG
    pTermTokens++;
  } /* endwhile */
  pTermTokens = pSentence->pTermTokens;

  while ( pTermTokens->usLength && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote( pTermTokens, pSentence, 2 );         // ACD, BDE, CEF, DFG
    pTermTokens++;
  } /* endwhile */

  pTermTokens = pSentence->pTermTokens;
  if( pTermTokens->usLength && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote (pTermTokens, pSentence, 4 );         // 1 * AAC
  } /* endif */


  pTermTokens  = pLastTerm - 2;
  if( pTermTokens->usLength && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote (pTermTokens, pSentence, 6 );         // 1 * EGG
  } /* endif */
  /********************************************************************/
  /* use 1st token as triple                                          */
  /********************************************************************/
  pTermTokens = pSentence->pTermTokens;
  if( pTermTokens->usLength && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote (pTermTokens, pSentence, 7 );         // 1 * AAA
  } /* endif */

  /********************************************************************/
  /* use last token as triple to ensure even distribution of token    */
  /********************************************************************/
  pTermTokens = pLastTerm;
  if( pTermTokens->usLength && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote (pTermTokens, pSentence, 7 );         // 1 * GGG
  } /* endif */

  usIndex = 0;
  pTermTokens = pSentence->pTermTokens;
  while ((usIndex < 3) && pTermTokens->usLength
         && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote (pTermTokens, pSentence, 3 );           // 3 * AAB
    if ((pLastTerm-1)->usLength
        && (pSentence->usActVote < MAX_VOTES ) )
    {
      Vote (pLastTerm-1, pSentence, 5 );         // 3 * FGG
    } /* endif */
    usIndex++;
  } /* endwhile */

} /* end of function BuildVotes */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     Vote                                                      
//------------------------------------------------------------------------------
// Description:       build the triples (3 out of 4)                            
//------------------------------------------------------------------------------
// Parameters:        PTMX_SENTENCE pstSentence ptr to sentence control struct  
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     build all possible 3 tupels out of a sequence of 4 consec 
//                    utive tokens. (base is the FLASH algorithm).              
//------------------------------------------------------------------------------
static VOID
Vote
(
  PTMX_TERM_TOKEN pTermTokens,             //ptr to term tokens
  PTMX_SENTENCE pSentence,                 //pointer to sentence structure
  USHORT usTuple                           //tuple grouping
)
{
  ULONG  ulVote;                       //actual vote
  USHORT i;                            //index values
  BOOL   fGo = FALSE;

  // create all possible 3 tupels out of a sequence of 4 consecutive tokens
  //for dealing correctly with the tail...
  if ( usTuple == 0 )
  {
    fGo = (pTermTokens+2)->usLength;
  }
  else if ((usTuple == 2) || (usTuple == 1) )
  {
    fGo = (pTermTokens+3)->usLength;
  }
  else
  {
    fGo = TRUE;
  } /* endif */

  if ( fGo )
  {
    ulVote = 0;
    for ( i = 0; i < 3; i++ )
    {
      ulVote = (ulVote*131) + pTermTokens[usTLookUp[i+3*usTuple]].usHash;
    } /* endfor */

    pSentence->pulVotes[pSentence->usActVote] = ulVote;
    pSentence->usActVote++;
  } /* endif */
} /* end of function Vote */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     CheckCompactArea                                          
//------------------------------------------------------------------------------
// Description:       calculate what percentage of triples already in tm        
//------------------------------------------------------------------------------
// Parameters:    PTMX_SENTENCE pstSentence pointer to sentence control struct  
//                PTMX_CLB pTmClb         pointer to tm control block           
//------------------------------------------------------------------------------
// Returncode type:   USHORT  number of bits in compact area on                 
//------------------------------------------------------------------------------
// Function flow:     calculate bit value of hash triple and check if set in    
//                    compact area in control block                             
//------------------------------------------------------------------------------
USHORT CheckCompactArea
(
  PTMX_SENTENCE pSentence,             // pointer to sentence structure
  PTMX_CLB  pTmClb                     // pointer to tm control block
)
{
  ULONG ulVote;                       // actual vote
  PULONG pulVotes;                    // pointer to begin of votes
  USHORT i;                           // counter
  BYTE bTuple;                        // active byte
  BYTE bRest;                         // relevant bit of byte
  USHORT usMatch;                     // number of matches
  PBYTE pByte;                        // byte pointer

  pulVotes = pSentence->pulVotes;
  usMatch = 0;
  for ( i = 0; ( i < pSentence->usActVote) ; i++, pulVotes++ )
  {
    ulVote = *pulVotes;
    ulVote = ulVote % ((LONG)(MAX_COMPACT_SIZE-1) * 8);
    bTuple = (BYTE) ulVote;
    bRest  = bTuple & 0x7;
    pByte = ((PBYTE)pTmClb->bCompact) + (ulVote >> 3);
    if ( *pByte & (1 << bRest) )
    {
      usMatch++;
    } /* endif */
  } /* endfor */
  return( usMatch );
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TokenizeTarget      Split a string into tags and words    
//------------------------------------------------------------------------------
// Description:       Tokenizes a string and stores all tags and terms found.   
//------------------------------------------------------------------------------
// Function call:     TokenizeTarget( PSZ pString,  //ptr to target string      
//                            PSZ * ppNormString, //ptr to norm string          
//                            PTMX_TAGTABLE_RECORD * ppTagRecord, //tag record  
//                            PSZ pTagTableName   // name of tag table          
//------------------------------------------------------------------------------
// Input parameter:   PSZ pString             target string                     
//                    PSZ ptagTableName       name of target tag table          
//------------------------------------------------------------------------------
// Output parameter:      PSZ * ppNormString, //ptr to norm string              
//                        PTMX_TAGTABLE_RECORD * ppTagRecord, //tag record      
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR    function completed successfully               
//                    other       error code                                    
//------------------------------------------------------------------------------
// Prerequesits:                                                                
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Samples:                                                                     
//------------------------------------------------------------------------------
// Function flow:                                                               
//     allocate storage                                                         
//     tokenize string with correct tag table                                   
//     process outputed token list                                              
//     if tag                                                                   
//       remember position of tag and add to entry structure                    
//       add length of tag to structure                                         
//       add string to structure                                                
//     else if text                                                             
//       build normalized string                                                
//                                                                              
//------------------------------------------------------------------------------
USHORT TokenizeTarget
(
   PSZ_W pString,                      // ptr to target string
   PSZ_W pNormString,                  // ptr to normalized string
   PTMX_TAGTABLE_RECORD *ppTagRecord,  // ptr to tag record structure
   PLONG pulTagAlloc,                  // size of allocated area for tag record
   PSZ pTagTableName,                  // name of tag table
   PUSHORT pusNormLen,                 // length of normalized string
   PTMX_CLB pClb                       // pointer to control block
)
{
  PVOID     pTokenList = NULL;         // ptr to token table
  BOOL      fOK;                       // success indicator
  PBYTE     pTagEntry;                 // pointer to tag entries
  PLOADEDTABLE pTable = NULL;          // pointer to tagtable
//  USHORT    usI;                     // offset
  USHORT    usFilled = 0;              // counter
  USHORT    usRc = NO_ERROR;           // returned value
  USHORT    usTagEntryLen;             // length indicator
  PTMX_TAGTABLE_RECORD pTagRecord;     // ptr to tag record structure
//   CHAR      szString[MAX_EQF_PATH];    // character string
  PSTARTSTOP pStartStop = NULL;        // ptr to start/stop table
  int        iIterations = 0;
  USHORT     usAddEntries = 0;

  pTagRecord = (*ppTagRecord);         //get contents of pointer

  //allocate 4K pTokenlist for TaTagTokenize
  fOK = UtlAlloc( (PVOID *) &(pTokenList), 0L, (LONG) TOK_SIZE, NOMSG );

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    pTagEntry = (PBYTE)pTagRecord;
    pTagEntry += sizeof(TMX_TAGTABLE_RECORD);
    RECLEN(pTagRecord) = 0;
    pTagRecord->usFirstTagEntry = (USHORT)(pTagEntry - (PBYTE)pTagRecord);

    //get id of tag table, call
    usRc = NTMGetIDFromName( pClb, pTagTableName,
                             NULL,
                             (USHORT)TAGTABLE_KEY,
                             &pTagRecord->usTagTableId  );
    if ( !usRc )
    {
      //tokenize target segment with correct tag table
      //load tag table for tokenize function

      //build tag table path
      usRc = TALoadTagTableExHwnd( pTagTableName, &pTable, FALSE,
                                   TALOADUSEREXIT | TALOADPROTTABLEFUNC |
                                   TALOADCOMPCONTEXTFUNC,
                                   FALSE, NULLHANDLE );
      if ( usRc )
      {
        usRc = ERROR_TA_ACC_TAGTABLE;
      } /* endif */
    } /* endif */

    if ( !usRc )
    {
      // build protect start/stop table for tag recognition
      usRc = TACreateProtectTableW( pString, pTable, 0,
                                   (PTOKENENTRY)pTokenList,
                                   TOK_SIZE, &pStartStop,
                                   pTable->pfnProtTable, pTable->pfnProtTableW, 0L );

      while ((iIterations < 10) && (usRc == EQFRS_AREA_TOO_SMALL))
      {
        // (re)allocate token buffer
        LONG lOldSize = (usAddEntries * sizeof(TOKENENTRY)) + (LONG)TOK_SIZE;
        LONG lNewSize = ((usAddEntries+128) * sizeof(TOKENENTRY)) + (LONG)TOK_SIZE;

        if (UtlAlloc((PVOID *) &pTokenList, lOldSize, lNewSize, NOMSG) )
        {
          usAddEntries += 128;
          iIterations++;
        }
        else
        {
          iIterations = 10;    // force end of loop
        } /* endif */
        // retry tokenization
        if (iIterations < 10 )
        {
          usRc = TACreateProtectTableW( pString, pTable, 0,
                                       (PTOKENENTRY)pTokenList,
                                       (USHORT)lNewSize, &pStartStop,
                                       pTable->pfnProtTable, pTable->pfnProtTableW, 0L );
        } /* endif */

      } /* endwhile */
    } /* endif */

    if ( !usRc )
    {
      PSTARTSTOP pEntry = pStartStop;
      while ( (pEntry->usStart != 0) ||
              (pEntry->usStop != 0)  ||
              (pEntry->usType != 0) )
      {
        switch ( pEntry->usType )
        {
          case UNPROTECTED_CHAR :
            // handle translatable text
            {
              USHORT usLength = pEntry->usStop - pEntry->usStart + 1;
              memcpy( pNormString, pString + pEntry->usStart, usLength * sizeof(CHAR_W));
              *pusNormLen = *pusNormLen + usLength;
              pNormString += usLength;
            } /* end case UNPROTECTED_CHAR */
            break;
          default :
            // handle not-translatable data
            {
              // if next start/stop-entry is a protected one ...
              if ( ((pEntry+1)->usStart != 0) &&
                   ((pEntry+1)->usType != UNPROTECTED_CHAR) )
              {
                // enlarge next entry and ignore current one
                (pEntry+1)->usStart = pEntry->usStart;
              }
              else
              {
                // add tag data
                usTagEntryLen = sizeof(TMX_TAGENTRY) +
                                (pEntry->usStop - pEntry->usStart + 1)*sizeof(CHAR_W);
                if ( ((LONG)*pulTagAlloc - (LONG)(pTagEntry - (PBYTE)pTagRecord))
                                                       <= (LONG)usTagEntryLen )
                {
                  //remember offset of pTagEntry
                  usFilled = (USHORT)(pTagEntry - (PBYTE)pTagRecord);

                  //allocate another 4k for pTagRecord
                  fOK = UtlAlloc( (PVOID *) &pTagRecord, *pulTagAlloc,
                                  *pulTagAlloc + (LONG)TOK_SIZE, NOMSG );
                  if ( fOK )
                  {
                    *pulTagAlloc += (LONG)TOK_SIZE;

                    //set new position of pTagEntry
                    pTagEntry = ((PBYTE)pTagRecord) + usFilled;
                  } /* endif */
                } /* endif */

                if ( !fOK )
                {
                  usRc = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                  ((PTMX_TAGENTRY)pTagEntry)->usOffset = pEntry->usStart;
                  ((PTMX_TAGENTRY)pTagEntry)->usTagLen =
                    (pEntry->usStop - pEntry->usStart + 1);// * sizeof(CHAR_W);
                  memcpy( &(((PTMX_TAGENTRY)pTagEntry)->bData),
                          pString + pEntry->usStart,
                          ((PTMX_TAGENTRY)pTagEntry)->usTagLen * sizeof(CHAR_W));
                  pTagEntry += usTagEntryLen;
                } /* endif */
              } /* endif */
            } /* end default */
            break;
        } /* endswitch */
        pEntry++;
      } /* endwhile */
      RECLEN(pTagRecord) = pTagEntry - (PBYTE)pTagRecord;
    } /* endif */
  } /* endif */


  //release memory
  if ( pStartStop ) UtlAlloc( (PVOID *) &pStartStop, 0L, 0L, NOMSG );
  if ( pTokenList ) UtlAlloc( (PVOID *) &pTokenList, 0L, 0L, NOMSG );

  //free tag table / decrement use count
  if ( pTable != NULL )
  {
    TAFreeTagTable( pTable );
  } /* endif */

  *ppTagRecord = pTagRecord;

  if ( usRc )
  {
    ERREVENT2( TOKENIZETARGET_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */

  return( usRc );
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     AddToTm             add a new tm record to tm database    
//------------------------------------------------------------------------------
// Description:       Add a new tm record to tm data file.                      
//------------------------------------------------------------------------------
// Function call:     AddToTm( pTmRecord,  // ptr to tm record PTMX_SENTENCE    
//                             pSentence,  // ptr to sentence structure         
//                             pTmClb,     // ptr to control block              
//                             pTmPut )    // ptr to put input structure        
//------------------------------------------------------------------------------
// Input parameter:   PTMX_RECORD pTmRecord                                     
//                    PTMX_SENTENCE pSentence                                   
//                    PTMX_CLB pTmClb                                           
//                    PTMX_PUT pTmPut                                           
//                    PULONG pulNewKey                                          
//------------------------------------------------------------------------------
// Output parameter:                                                            
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR    function completed successfully               
//                    other       error code                                    
//------------------------------------------------------------------------------
// Prerequesits:                                                                
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Samples:                                                                     
//------------------------------------------------------------------------------
// Function flow:                                                               
//      call TokenizeSegment with szTagTable, szTarget, usTargetLangId,         
//        pstTagTableRecord, pListOfWords                                       
//                                                                              
//      fill target control block structure with data from TM_PUT               
//      fill rest of target tm_record fields                                    
//                                                                              
//      add tm record to tm data file                                           
//------------------------------------------------------------------------------
USHORT AddToTm
(
  PTMX_SENTENCE pSentence,            // ptr to sentence structure
  PTMX_CLB pTmClb,                    // ptr to control block
  PTMX_PUT_W pTmPut,                  // ptr to put input structure
  PULONG pulNewKey                    // sid of newly added tm record
)
{
  PTMX_RECORD pTmRecord = NULL;           // ptr to tm record
  PTMX_TARGET_CLB pTargetClb = NULL;      // ptr to target ctl block
  PSZ_W pNormString = NULL;               // ptr to normalized string
  PTMX_TAGTABLE_RECORD pTagRecord = NULL; // ptr to tag table record
  USHORT usRc = NO_ERROR;                 // return code
  USHORT usNormLen = 0;                   // length indicator
  BOOL fOK;                               // success indicator
  LONG lTagAlloc;                         // alloc size
  USHORT usAddDataLen = 0;

  //allocate 32K for tm record
  fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );

  //allocate target control block record
  if ( fOK )
  {
    usAddDataLen = NTMComputeAddDataSize( pTmPut->szContext, pTmPut->szAddInfo );

    fOK = UtlAlloc( (PVOID *) &pTargetClb, 0L, (LONG)(sizeof(TMX_TARGET_CLB)+usAddDataLen), NOMSG );
  } /* endif */

  //allocate normalized string
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pNormString), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
  }

  //allocate 4k for pTagRecord
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pTagRecord), 0L, (LONG) TOK_SIZE, NOMSG );
    if ( fOK )
     lTagAlloc = (LONG)TOK_SIZE;
  }

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    usRc = TokenizeTarget( pTmPut->szTarget, pNormString, &pTagRecord,
                           &lTagAlloc, pTmPut->szTagTable, &usNormLen, pTmClb );
    if ( usRc == NO_ERROR )
    {
      usRc = FillClb( &pTargetClb, pTmClb, pTmPut );
      if ( usRc == NO_ERROR )
      {
        //fill tm record to add to database
        FillTmRecord ( pSentence,    // ptr to sentence struct for source info
                       pTagRecord,   // ptr to target string tag table
                       pNormString,  // ptr to target normalized string
                       usNormLen,    // length of target normalized string
                       pTmRecord,    // filled tm record returned
                       pTargetClb );

        //add new tm record to database
        *pulNewKey = NTMREQUESTNEWKEY;
        usRc = EQFNTMInsert( pTmClb->pstTmBtree, //ptr to tm structure
                             pulNewKey,          //to be allocated in funct
                             (PBYTE)pTmRecord,   //pointer to tm record
                             RECLEN(pTmRecord) );     //length
      } /* endif */
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &(pTmRecord), 0L, 0L, NOMSG);
  UtlAlloc( (PVOID *) &(pTargetClb), 0L, 0L, NOMSG);
  UtlAlloc( (PVOID *) &(pNormString), 0L, 0L, NOMSG);
  UtlAlloc( (PVOID *) &(pTagRecord), 0L, 0L, NOMSG);

  if ( usRc )
  {
    ERREVENT2( ADDTOTM_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */

  return( usRc );
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     FillTmRecord        fill complete tm record               
//------------------------------------------------------------------------------
// Description:       Fill complete tm record                                   
//------------------------------------------------------------------------------
// Function call:     FillTmRecord( pSentence,                                  
//                                  pTagRecord,                                 
//                                  pNormString,                                
//                                  usNormLen,                                  
//                                  pTmRecord )                                 
//------------------------------------------------------------------------------
// Input parameter: PTMX_SENTENCE  pSentence //ptr to sent struct for source    
//             PTMX_TAGTABLE_RECORD pTagRecord //ptr to target string tag table 
//             PSZ pNormString        // ptr to target normalized string        
//             USHORT usNormLen       // length of target normalized string     
//------------------------------------------------------------------------------
// Output parameter: PTMX_RECORD pTmRecord    // filled tm record returned      
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       NONE                                                      
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Function flow:                                                               
//      fill tm record structure                                                
//------------------------------------------------------------------------------
VOID FillTmRecord
(
  PTMX_SENTENCE  pSentence,          // ptr to sentence struct for source info
  PTMX_TAGTABLE_RECORD pTagRecord,   // ptr to target string tag table
  PSZ_W pNormString,                 // ptr to target normalized string
  USHORT usNormLen,                  // length of target normalized string
  PTMX_RECORD pTmRecord,             // filled tm record returned
  PTMX_TARGET_CLB pTargetClb         // ptr to target control block
)
{
  PTMX_SOURCE_RECORD pTMXSourceRecord;      //ptr to start of source structure
  PTMX_TARGET_RECORD pTMXTargetRecord;      //ptr to start of target structure
  PBYTE pTarget;                            //ptr to target record
  ULONG ulSrcNormLen = pSentence->usNormLen;

  //position source structure in tm record
  pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);
  //source structure offset in tm record
  pTmRecord->usSourceRecord = (USHORT)((PBYTE)pTMXSourceRecord - (PBYTE)pTmRecord);

  //set source string offset and copy source string
  pTMXSourceRecord->usSource = sizeof( TMX_SOURCE_RECORD );
//  memcpy( pTMXSourceRecord+1, pSentence->pNormString, pSentence->usNormLen *sizeof(CHAR_W));
//@@@
  ulSrcNormLen = EQFUnicode2Compress( (PBYTE)(pTMXSourceRecord+1), pSentence->pNormString, ulSrcNormLen );
  //size of source record
  RECLEN(pTMXSourceRecord) = sizeof( TMX_SOURCE_RECORD ) + ulSrcNormLen;

  //position target structure in tm record
  pTarget = (PBYTE)(pTmRecord+1);
  pTarget = pTarget + RECLEN(pTMXSourceRecord);

  //first target offset in tm record
  pTmRecord->usFirstTargetRecord = (USHORT)(pTarget - (PBYTE)pTmRecord);

  //position to start of target structure
  pTMXTargetRecord = ((PTMX_TARGET_RECORD)pTarget);

  //set source tag table offset
  pTMXTargetRecord->usSourceTagTable = sizeof(TMX_TARGET_RECORD);

  //position pointer for source tag table
  pTarget += pTMXTargetRecord->usSourceTagTable;

  //copy source tag table record to correct position
  memcpy( pTarget, pSentence->pTagRecord, RECLEN(pSentence->pTagRecord) );

  //set target tag table start offset
  pTMXTargetRecord->usTargetTagTable = (USHORT)(pTMXTargetRecord->usSourceTagTable +
                               RECLEN(pSentence->pTagRecord));
  //adjust target pointer for target tag table
  pTarget += RECLEN(pSentence->pTagRecord);

  //copy target tag table record to correct position
  memcpy( pTarget, pTagRecord, RECLEN(pTagRecord) );

  //set target string start offset
  pTMXTargetRecord->usTarget = (USHORT)(pTMXTargetRecord->usTargetTagTable +
                               RECLEN(pTagRecord));
  //adjust target pointer for target string
  pTarget += RECLEN(pTagRecord);

  //copy target string to correct position
//@@  memcpy( pTarget, pNormString, usNormLen );
  { ULONG ulTempLen = usNormLen;
    ulTempLen = EQFUnicode2Compress( pTarget, pNormString, ulTempLen );
    usNormLen = (USHORT)ulTempLen;
  }

  //set target string control block start offset
  pTMXTargetRecord->usClb = pTMXTargetRecord->usTarget + usNormLen;

  //adjust target pointer for control block
  pTarget += usNormLen;

  //copy target control block
  memcpy( pTarget, pTargetClb, TARGETCLBLEN(pTargetClb) );

  //size of target record
  RECLEN(pTMXTargetRecord) = sizeof(TMX_TARGET_RECORD) +
                                  RECLEN(pSentence->pTagRecord) +
                                  RECLEN(pTagRecord) +
                                  usNormLen +
                                  TARGETCLBLEN(pTargetClb);
  //size of entire tm record
  RECLEN(pTmRecord) = sizeof( TMX_RECORD ) +
                           RECLEN(pTMXSourceRecord) +
                           RECLEN(pTMXTargetRecord);

  //initialize subsequent empty target record
  //position at end of target record
  pTarget +=  TARGETCLBLEN(pTargetClb);
  //position at next target record
  pTMXTargetRecord = (PTMX_TARGET_RECORD)pTarget;

  RECLEN(pTMXTargetRecord) = 0;
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     FillClb             fill target control block             
//------------------------------------------------------------------------------
// Description:       Fill target control block                                 
//------------------------------------------------------------------------------
// Function call:     FillClb( PTMX_TARGET_CLB * ppTargetClb,//ptr to ctl block 
//                             PTMX_CLB pTmClb,  // ptr to tm ctl block         
//                             PTMX_PUT pTmPut ) // ptr to put input struct     
//------------------------------------------------------------------------------
// Input parameter:   PTMX_PUT pTmPut         put input structure               
//                    PTMX_CLB pTmClb         tm control block                  
//------------------------------------------------------------------------------
// Output parameter:  PTM_TARGET_CLB * ppClb  //ptr to target control block     
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
// Returncodes:       TRUE        function completed successfully               
//                    FALSE       error                                         
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Function flow:                                                               
//      get all necessary ids for strings                                       
//      allocate target control block                                           
//                                                                              
//      fill target control block structure with data from TM_PUT               
//      return ulKey                                                            
//------------------------------------------------------------------------------
USHORT FillClb
(
  PTMX_TARGET_CLB * ppTargetClb,    // ptr to target control block
  PTMX_CLB pTmClb,                  // ptr to tm control block
  PTMX_PUT_W pTmPut                 // ptr to put input structure
)
{
  USHORT  usLang;
  USHORT  usFile = 0;
  USHORT  usAuthor = 0;                 // ids
  USHORT  usRc = NO_ERROR;             // returned value
  PTMX_TARGET_CLB pTargetClb = NULL;   // ptr to target control block

  pTargetClb = *ppTargetClb;

  // replace any 0xA0 in language name to 0xFF
  // (0xA0 is 0xFF after processing by OemToAnsi)
  REPLACE_A0_BY_FF( pTmPut->szTargetLanguage );

  //get id of target language, call
  usRc = NTMGetIDFromName( pTmClb, pTmPut->szTargetLanguage, NULL, (USHORT)LANG_KEY, &usLang );

  //get id of file name, call
  if ( !usRc )
  {
    usRc = NTMGetIDFromName( pTmClb, pTmPut->szFileName, pTmPut->szLongName, (USHORT)FILE_KEY, &usFile );
  } /* endif */

  //get id of target author
  if ( !usRc )
  {
    usRc = NTMGetIDFromName( pTmClb, pTmPut->szAuthorName, NULL, (USHORT)AUTHOR_KEY, &usAuthor );
  } /* endif */

  if ( !usRc )
  {
    pTargetClb->usLangId = usLang;
    pTargetClb->bTranslationFlag = (BYTE)pTmPut->usTranslationFlag;
    //if a time is given take it else use current time
    if ( pTmPut->lTime )
    {
      pTargetClb->lTime = pTmPut->lTime;
    }
    else
    {
      UtlTime( &(pTargetClb->lTime) );
    } /* endif */
    pTargetClb->usFileId = usFile;
    pTargetClb->ulSegmId = pTmPut->ulSourceSegmentId;
    pTargetClb->usAuthorId = usAuthor;
    pTargetClb->usAddDataLen = 0;
    if ( pTmPut->szContext[0] ) NtmStoreAddData( pTargetClb, ADDDATA_CONTEXT_ID, pTmPut->szContext );
    if ( pTmPut->szAddInfo[0] ) NtmStoreAddData( pTargetClb, ADDDATA_ADDINFO_ID, pTmPut->szAddInfo );
    *ppTargetClb = pTargetClb;
  } /* endif */

  if ( usRc )
  {
    ERREVENT2( FILLCLB_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */

  return (usRc);
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     UpdateTmIndex    check if index exists else add           
//------------------------------------------------------------------------------
// Description:       Checks if triple hash entry in index file exists and if   
//                    add the new sentence id else adds a new hash entry with   
//                    sentence id                                               
//+---------------------------------------------------------------------------- 
// Function call:     UpdateTmIndex( PTMX_SENTENCE  pSentence,//sent struct     
//                                   ULONG  ulSidKey,     //tm record key       
//                                   PTMX_CLB pTmClb) //ptr to tm ctl block     
//------------------------------------------------------------------------------
// Input parameter: PTMX_SENTENCE pSentence                                     
//                  ULONG  ulSidKey                                             
//                  PTMX_CLB pTmClb                                             
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//------------------------------------------------------------------------------
// Function flow:                                                               
//   get index record for first hash in Sentence structure                      
//     extract all sids                                                         
//     loop through remaining tuple hashes in pSentence as follows              
//       QDAMDictExactLongLocal with next hash key                              
//       extract all sids returned in pData                                     
//       check for matches, remembering matches until only one left or all      
//       hashes tried                                                           
//                                                                              
//     if match(es)                                                             
//       get tm record with remaining sid(s) from QDAM using                    
//       CheckTmRecord( pTmRecord(from QDAM call), pstTmPut, pstSentence,       
//                      pTmClb )                                                
//     else                                                                     
//      fill source record of tm_record (source string and tagtable structure)  
//      call AddToTm                                                            
//------------------------------------------------------------------------------

USHORT UpdateTmIndex
(
  PTMX_SENTENCE  pSentence,            //pointer to sentence structure
  ULONG  ulSidKey,                     //tm record key
  PTMX_CLB pTmClb                      //ptr to tm control block
)
{
  USHORT   usRc = 0;                   // return code
  ULONG    ulLen;                      // length paramter
  USHORT   usIndexEntries;             // nr of entries in index record
  PULONG   pulVotes = NULL;            // pointer to votes
  USHORT   i;                          // index in for loop
  ULONG    ulKey;                      // index key
  PTMX_INDEX_RECORD pIndexRecord = NULL;  // pointer to index structure
  BOOL     fOK = FALSE;                // success indicator
  PBYTE    pIndex;                     // position pointer

  //for all votes add the index to the corresponding list
  pulVotes = pSentence->pulVotes;

  //allocate 32K for tm index record
  fOK = UtlAlloc( (PVOID *) &(pIndexRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );

#ifdef NTMTEST
{
  FILE *fOut;                          // test output
  PULONG pulTVotes = pSentence->pulVotes;
  USHORT  i;
  ULONG   ulKey;

  fOut      = fopen ( "\\NTMTEST.DBG", "a" );
  fprintf (fOut, "%20s %d\n", __FILE__, __LINE__);
  fprintf (fOut, "Input String: %s\n",pSentence->pInputString );
  fprintf (fOut, "\nCompact Bits: \n");

  for ( i = 0; i < pSentence->usActVote; i++, pulTVotes++ )
  {
      //add the match(tuple) to compact area
      ulKey = *pulTVotes % ((LONG)(MAX_COMPACT_SIZE-1) * 8);
      fprintf (fOut, "%4d %4d\n",(ulKey >> 3), (1 << (USHORT)(ulKey & 0x07)));
   } /* endfor */
   fclose( fOut      );
}
#endif

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    for ( i = 0; i < pSentence->usActVote; i++, pulVotes++ )
    {
      if ( usRc == NO_ERROR )
      {
        ulKey = (*pulVotes) & START_KEY;
        ulLen = TMX_REC_SIZE;
        memset( pIndexRecord, 0, TMX_REC_SIZE );
        usRc = EQFNTMGet( pTmClb->pstInBtree,
                          ulKey,  //index key
                          (PCHAR)pIndexRecord,   //pointer to index record
                          &ulLen );  //length

        if ( usRc == BTREE_NOT_FOUND )
        {
          //key is not in index file; add a new index entry
          pIndexRecord->usRecordLen = sizeof( TMX_INDEX_RECORD );

          pIndexRecord->stIndexEntry = NTMINDEX(pSentence->usActVote,ulSidKey);

          usRc = EQFNTMInsert( pTmClb->pstInBtree,
                               &ulKey,
                               (PBYTE)pIndexRecord,  //pointer to index
                               pIndexRecord->usRecordLen );  //length

          // if index DB is full and memory is in exclusive access we try to compact the index file
          if ( (usRc == BTREE_LOOKUPTABLE_TOO_SMALL) && (pTmClb->usAccessMode & ASD_LOCKED) )
          {
             usRc = EQFNTMOrganizeIndex( &(pTmClb->pstInBtree), pTmClb->usAccessMode, START_KEY );

             if ( usRc == NO_ERROR )
             {
               usRc = EQFNTMInsert( pTmClb->pstInBtree, &ulKey, (PBYTE)pIndexRecord, pIndexRecord->usRecordLen );
             } /* endif */
          } /* endif */

          if ( !usRc )
          {
            //add the match(tuple) to compact area
            ulKey = *pulVotes % ((LONG)(MAX_COMPACT_SIZE-1) * 8);
            *((PBYTE)pTmClb->bCompact + (ulKey >> 3)) |=
                                  1 << ((BYTE)ulKey & 0x07);
//                                  1 << (USHORT)(ulKey & 0x07);       @01M
            pTmClb->bCompactChanged = TRUE;
          } /* endif */
        }
        else
        {
          if ( usRc == NO_ERROR )
          {
            BOOL fFound = FALSE;

            //key is in index file; update index entry with new sid
            ulLen = pIndexRecord->usRecordLen;

            //calculate number of entries in index record
            usIndexEntries = (USHORT)((ulLen - sizeof(USHORT)) / sizeof(TMX_INDEX_ENTRY));
  
            //// check if SID is already contained in list..
            //{
            //  int i = (int)usIndexEntries;
            //  PULONG pulIndex = (PULONG)&(pIndexRecord->stIndexEntry); 
            //  while ( i )
            //  {
            //    if ( *pulIndex == ulNewSID )
            //    {
            //      fFound = TRUE;
            //      break;
            //    } /* endif */
            //    pulIndex++;
            //    i--;
            //  } /*endwhile */
            //}

            if ( !fFound )
            {
              if ( usIndexEntries >= (MAX_INDEX_LEN -1))
              {
                //position pointer at beginning of index record
                pIndex = (PBYTE)pIndexRecord;
                memmove( pIndex, pIndex + sizeof(ULONG), ulLen - sizeof(ULONG) );
                ulLen -= sizeof(ULONG);
                usIndexEntries--;
              }
              //only update index file if index record is not too large
              if ( usIndexEntries < (MAX_INDEX_LEN - 1))
              {
                //position pointer at beginning of index record
                pIndex = (PBYTE)pIndexRecord;

                //move pointer to end of index record
                pIndex += ulLen;

                // fill in new index entry
                *((PTMX_INDEX_ENTRY) pIndex ) =
                                    NTMINDEX(pSentence->usActVote,ulSidKey);

                //update index record size
                pIndexRecord->usRecordLen = (USHORT)(ulLen + sizeof( TMX_INDEX_ENTRY ));

                usRc = EQFNTMUpdate( pTmClb->pstInBtree,
                                    ulKey,
                                    (PBYTE)pIndexRecord,  //pointer to index
                                    pIndexRecord->usRecordLen );  //length
                // if index DB is full and memory is in exclusive access we try to compact the index file
                if ( (usRc == BTREE_LOOKUPTABLE_TOO_SMALL) && (pTmClb->usAccessMode & ASD_LOCKED) )
                {
                  usRc = EQFNTMOrganizeIndex( &(pTmClb->pstInBtree), pTmClb->usAccessMode, START_KEY );

                  if ( usRc == NO_ERROR )
                  {
                    usRc = EQFNTMUpdate( pTmClb->pstInBtree, ulKey, (PBYTE)pIndexRecord, pIndexRecord->usRecordLen ); 
                  } /* endif */
                } /* endif */
                if ( !usRc )
                {
                  //add the match(tuple) to compact area
                  ulKey = *pulVotes % ((LONG)(MAX_COMPACT_SIZE-1) * 8);
                  *((PBYTE)pTmClb->bCompact + (ulKey >> 3)) |=
                                        1 << ((BYTE)ulKey & 0x07);
                  pTmClb->bCompactChanged = TRUE;
                } /* endif */
              }
              else
              {
                usIndexEntries = usIndexEntries;

              }  /* endif */
            } /* endif */

          } /* endif */
        } /* endif */
      }
      else
      {
        //error so leave for loop
        i = pSentence->usActVote;
      } /* endif */
    } /* endfor */
  } /* endif */

  //release allocated memory
  UtlAlloc( (PVOID *) &(pIndexRecord), 0L, 0L, NOMSG);

  if ( usRc )
  {
    ERREVENT2( FILLCLB_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */

  return( usRc );
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     DetermineTmRecord outputs all possible sids               
//------------------------------------------------------------------------------
// Description:       This function returns a list of all legitimate sentence   
//                    keys                                                      
//+---------------------------------------------------------------------------- 
// Function call:  DetermineTmRecord( PTMX_CLB pTmClb,                          
//                                    PTMX_SENTENCE pSentence,                  
//                                    PULONG pulSids )                          
//------------------------------------------------------------------------------
// Input parameter: PTMX_CLB pTmClb                                             
//                  PTMX_SENTENCE pSentence                                     
//------------------------------------------------------------------------------
// Output parameter:  PULONG pulSids                                            
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//                                                                              
//------------------------------------------------------------------------------
// Function flow:                                                               
//   get index record for first triple                                          
//     get list of all sids in this index record                                
//     loop through all triple hashes built for source string                   
//       and determine valid sids eliminating those from list that no longer    
//       hold                                                                   
//------------------------------------------------------------------------------
#ifdef _DEBUG
  static BOOL  fSidLog = FALSE;
#endif

USHORT DetermineTmRecord
(
  PTMX_CLB pTmClb,                   // ptr to tm control block
  PTMX_SENTENCE pSentence,           // ptr to sentence structure
  PULONG pulSids                     // ptr to tm record
)
{
  USHORT usRc = NO_ERROR;            // return code
  ULONG  ulLen;                      // length paramter
  USHORT usSid = 0;                  // number of sentence ids found
  USHORT usPos;                      // position in pulSids
  PULONG pulVotes;                   // pointer to votes
  PULONG pulSidStart;                // po ter to votes
  USHORT i, j;                       // index in for loop
  USHORT usMaxEntries = 0;           // nr of index entries in index record
  ULONG ulKey;                       // index key
  PTMX_INDEX_RECORD pIndexRecord = NULL; // pointer to index structure
  BOOL fOK;                          // success indicator
  PTMX_INDEX_ENTRY pIndexEntry;      // pointer to index entry structure

  pulSidStart = pulSids;

  //for all votes add the index to the corresponding list
  pulVotes = pSentence->pulVotes;

  //allocate 32K for tm index record
  fOK = UtlAlloc( (PVOID *) &(pIndexRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    ulKey = (*pulVotes) & START_KEY;
    ulLen = TMX_REC_SIZE;
    memset( pIndexRecord, 0, TMX_REC_SIZE );
    usRc = EQFNTMGet( pTmClb->pstInBtree,
                      ulKey,  //index key
                      (PCHAR)pIndexRecord,   //pointer to index record
                      &ulLen );  //length

    if ( usRc == NO_ERROR )
    {
      //calculate number of index entries in index record
      ulLen = pIndexRecord->usRecordLen;
      usMaxEntries = (USHORT)((ulLen - sizeof(USHORT)) / sizeof(TMX_INDEX_ENTRY));

#ifdef _DEBUG
{
  if ( fSidLog )
  {
    FILE *stream;
    stream = fopen( "\\SIDS.LOG","a" );
        pIndexEntry = &pIndexRecord->stIndexEntry;
    fprintf(stream, "Number Entries: %d\n ", usMaxEntries );
    for (j=0 ; j<usMaxEntries;j++,pIndexEntry++ )
    {
      if (j % 10 == 0)
      {
        fprintf(stream,"\n");
      } /* endif */
      fprintf( stream, "%ul ", NTMKEY(*pIndexEntry) );
    } /* endfor */
    fclose( stream );
  } /* endif */
}
#endif

      pIndexEntry = &pIndexRecord->stIndexEntry;

      for ( j = 0; j < usMaxEntries; j++, pIndexEntry++ )
      {
        if ( NTMVOTES(*pIndexEntry) == (BYTE) pSentence->usActVote )
        {
          *pulSids = NTMKEY(*pIndexEntry);
          usSid++;
          pulSids++;
        } /* endif */
      } /* endfor */

      if ( (usSid > 1) )
      {
        //there is more than one sentence to check so go through other
        //indices until all have been tried or usSid has been reduced to 1
        pulVotes++;
        for ( i = 0; (i < pSentence->usActVote-1) && (usSid > 1);
              i++, pulVotes++ )
        {
          if ( usRc == NO_ERROR )
          {
            ulKey = (*pulVotes) & START_KEY;
            ulLen = TMX_REC_SIZE;
            memset( pIndexRecord, 0, TMX_REC_SIZE );
            usRc = EQFNTMGet( pTmClb->pstInBtree,
                              ulKey,  //index key
                              (PCHAR)pIndexRecord,   //pointer to index record
                              &ulLen );  //length

            if ( usRc == NO_ERROR )
            {
              //calculate number of index entries in index record
              ulLen = pIndexRecord->usRecordLen;
              usMaxEntries = (USHORT)((ulLen - sizeof(USHORT)) / sizeof(TMX_INDEX_ENTRY));

#ifdef _DEBUG
{
  if ( fSidLog )
  {
    FILE *stream;
    stream = fopen( "\\SIDS.LOG","a" );
        pIndexEntry = &pIndexRecord->stIndexEntry;
    fprintf(stream, "Number Entries: %d\n ", usMaxEntries );
    for (j=0 ; j<usMaxEntries;j++,pIndexEntry++ )
    {
      if (j % 10 == 0)
      {
        fprintf(stream,"\n");
      } /* endif */
      fprintf( stream, "%ul ", NTMKEY(*pIndexEntry) );
    } /* endfor */
    fclose( stream );
  } /* endif */
}
#endif

              pIndexEntry = &pIndexRecord->stIndexEntry;
              pulSids = pulSidStart;
              usPos = 0;

              //end criteria are all sentence ids in index key or only one
              //sentence id left in pulSids
              for ( j = 0; (j < usMaxEntries) && (usSid > 1);
                    j++, pIndexEntry++ )
              {
                if ( NTMVOTES(*pIndexEntry) == (BYTE) pSentence->usActVote )
                {
                  //before adding sentence id check if already in pulsids as the
                  //respective tm record need only be checked once
                  while ( (NTMKEY(*pIndexEntry) > *pulSids) && (usSid > 1) )
                  {
                    //remove sid from pulSids and decrease sid counter
                    if ( usSid > usPos )
                    {
                      ulLen = (usSid - usPos) * sizeof(ULONG);
                    }
                    else
                    {
                      ulLen = sizeof(ULONG);
                    } /* endif */
                    memmove( (PBYTE) pulSids, (PBYTE)(pulSids+1), ulLen );
                    usSid--;
                  } /* endwhile */

                  if ( NTMKEY(*pIndexEntry) == *pulSids )
                  {
                    //move on one position in pulSids
                    pulSids++;
                    usPos++;
                  } /* endif */
                } /* endif */
              } /* endfor */
            }
            else
            {
              //in case the index record doesn't exist exit function and
              //set return code to 0
              if ( usRc == BTREE_NOT_FOUND )
              {
                usRc = NO_ERROR;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endfor */
      } /* endif */
    }
    else
    {
      //in case the index record doesn't exist exit function and set return
      //code to 0
      if ( usRc == BTREE_NOT_FOUND )
      {
        usRc = NO_ERROR;
      } /* endif */
    } /* endif */
  } /* endif */

#ifdef _DEBUG
{
  if ( fSidLog )
  {
    FILE *stream;
    stream = fopen( "\\SIDS.LOG","a" );
    fprintf(stream, "\nMatching Sids: \n ");
    while ( *pulSidStart )
    {
      fprintf( stream, "%ul ", *pulSidStart );
      pulSidStart++;
    } /* endwhile */
    fclose( stream );
  } /* endif */
}
#endif
  UtlAlloc( (PVOID *) &(pIndexRecord), 0L, 0L, NOMSG );

  if ( usRc )
  {
    ERREVENT2( DETERMINETMRECORD_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */

  return( usRc );
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     UpdateTmRecord                                            
//------------------------------------------------------------------------------
// Description:       This function either adds a new target, sets the          
//                    multiple flag to true or replaces an existing target      
//                    record                                                    
//+---------------------------------------------------------------------------- 
// Function call:     UpdateTmRecord( PTMX_CLB pTmClb,                          
//                                    PTMX_PUT pTmPut,                          
//                                    PTMX_SENTENCE pSentence                   
//------------------------------------------------------------------------------
// Input parameter: PTMX_CLB                                                    
//                  PTMX_PUT                                                    
//                  PTMX_SENTENCE                                               
//------------------------------------------------------------------------------
// Output parameter:                                                            
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//                                                                              
//------------------------------------------------------------------------------
// Function flow:                                                               
//   determine all valid sentence keys for source string                        
//     for all returned sentence keys                                           
//       get tm record                                                          
//       call function with put criteria                                        
//       if record putted stop processing                                       
//       else try next sentence key                                             
//------------------------------------------------------------------------------

USHORT UpdateTmRecord
(
  PTMX_CLB      pTmClb,                //ptr to tm control block
  PTMX_PUT_W    pTmPut,                //pointer to get in data
  PTMX_SENTENCE pSentence              //ptr to sentence structure
)
{
  BOOL   fOK;                          //success indicator
  BOOL   fStop = FALSE;                //indication to leave while loop
  PULONG pulSids = NULL;               //ptr to sentence ids
  PULONG pulSidStart = NULL;           //ptr to sentence ids
  USHORT usRc = NO_ERROR;              //return code
  ULONG  ulLen;                        //length indicator
  ULONG ulKey;                         //tm record key
  PTMX_RECORD pTmRecord = NULL;        //pointer to tm record
  ULONG       ulRecBufSize = 0L;       // current size of record buffer

  //allocate 32K for tm record
  fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );
  if ( fOK )
  {
    ulRecBufSize = TMX_REC_SIZE;
  } /* endif */

  //allocate for sentence ids
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pulSids), 0L,
                    (LONG)((MAX_INDEX_LEN + 5) * sizeof(ULONG)),
                    NOMSG );
  } /* endif */

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
      while ( (*pulSids) && (usRc == NO_ERROR) && !fStop )
      {
        ulKey = *pulSids;
        ulLen = TMX_REC_SIZE;
        memset( pTmRecord, 0, ulLen );
        usRc = EQFNTMGet( pTmClb->pstTmBtree,
                          ulKey,  //tm record key
                          (PCHAR)pTmRecord,   //pointer to tm record data
                          &ulLen );    //length
        if ( usRc == BTREE_BUFFER_SMALL)
        {
          fOK = UtlAlloc( (PVOID *)&pTmRecord, ulRecBufSize, ulLen, NOMSG );
          if ( fOK )
          {
            ulRecBufSize = ulLen;
            memset( pTmRecord, 0, ulLen );

            usRc = EQFNTMGet( pTmClb->pstTmBtree,
                              ulKey,  //tm record key
                              (PCHAR)pTmRecord,   //pointer to tm record data
                              &ulLen );    //length
          }
          else
          {
            usRc = ERROR_NOT_ENOUGH_MEMORY;
          } /* endif */
        } /* endif */

        if ( usRc == NO_ERROR )
        {
          //compare tm record data with data passed in the get in structure
          usRc = ComparePutData( pTmClb, &pTmRecord, &ulRecBufSize,
                                 pTmPut, pSentence, &ulKey );

          if ( usRc == SOURCE_STRING_ERROR )
          {
            //get next tm record
            pulSids++;
            usRc = NO_ERROR;
          }
          else if ( usRc == NO_ERROR )
          {
            //new target record was added or an existing one was successfully
            //replaced so don't try other sids
            fStop = TRUE;
          } /* endif */
        } /* endif */
      } /* endwhile */

      if ( !(*pulSids) && !fStop )
      {
        //issue message that tm needs to be organized if pulsid is empty and
        //no get was successful
        usRc = ERROR_ADD_TO_TM;
      } /* endif */
    }  /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pulSidStart, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pTmRecord, 0L, 0L, NOMSG );

  if ( usRc )
  {
    ERREVENT2( UPDATETMRECORD_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */

  return( usRc );
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     ComparePutData   checks the put criteria                  
//------------------------------------------------------------------------------
// Description:       This function either adds a new target, sets the          
//                    multiple flag to true or replaces an existing target      
//                    record                                                    
//+---------------------------------------------------------------------------- 
// Function call:     ComparePutData( PTMX_CLB pTmClb,                          
//                                    PTMX_RECORD pTmRecord,                    
//                                    PTMX_PUT pTmPut,                          
//                                    PTMX_SENTENCE pSentence,                  
//                                    PULONG pulKey                             
//------------------------------------------------------------------------------
// Input parameter: PTMX_CLB pTmClb                                             
//                  PTMX_RECORD pTmRecord                                       
//                  PTMX_PUT pTmPut                                             
//                  PTMX_SENTENCE pSentence                                     
//                  PULONG pulKey                                               
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:    usRc NO_ERROR                                                
//                                                                              
//------------------------------------------------------------------------------
// Function flow:                                                               
//   if source strings are equal                                                
//     tokenize target                                                          
//     Delete old entry                                                         
//     Loop thru target records                                                 
//        loop over all target CLBs or until fStop                              
//                   if segment+file id found (exact-exact-found!)              
//                      update time field in control block                      
//                      set fUpdate= fStop=TRUE                                 
//                      update context info                                     
//                   if not fStop                                               
//                      goto next CLB                                           
//                endloop                                                       
//                if no matching CLB has been found (if not fStop)              
//                    add new CLB (ids, context, timestamp etc. )               
//        endloop                                                               
//     endloop                                                                  
//     if fupdated, update TM record                                            
//     if !fStop (all target record have been tried & none matches )            
//       add new target record to end of tm record                              
//   else                                                                       
//       source_string_error                                                    
//------------------------------------------------------------------------------
USHORT ComparePutData
(
  PTMX_CLB    pTmClb,                  // ptr to ctl block struct
  PTMX_RECORD *ppTmRecord,             // ptr to ptr of tm record data buffer
  PULONG      pulRecBufSize,           // current size of record buffer
  PTMX_PUT_W  pTmPut,                  // pointer to get in data
  PTMX_SENTENCE pSentence,             // pointer to sentence structure
  PULONG      pulKey                   // tm key
)
{
  BOOL fOK;                            //success indicator
  BOOL fStop = FALSE;                  //indicates whether to leave loop or not
  PBYTE pByte;                         //position ptr
  PBYTE pStartTarget;                  //position ptr
  PTMX_SOURCE_RECORD pTMXSourceRecord = NULL; //ptr to source record
  PTMX_TARGET_RECORD pTMXTargetRecord = NULL; //ptr to target record
  PTMX_TARGET_CLB    pClb = NULL;    //ptr to target control block
  PTMX_TAGTABLE_RECORD pTMXSourceTagTable = NULL; //ptr to source tag info
  PTMX_TAGTABLE_RECORD pTMXTargetTagTable = NULL; //ptr to tag info
  PTMX_TAGTABLE_RECORD pTagRecord = NULL;  //ptr to tag info
  ULONG ulLen = 0;                        //length indicator
  USHORT usNormLen = 0;                    //length of normalized string
  PSZ_W pString = NULL;                  //pointer to character string
  PSZ_W pNormString = NULL;              //pointer to character string
  USHORT usRc = NO_ERROR;              //returned value from function
  LONG lTagAlloc;                      //alloc size
  BOOL        fUpdate = FALSE;         // TRUE = record has been updated
  PTMX_RECORD pTmRecord = *ppTmRecord; // pointer to tm record data
  USHORT      usAuthorId;              // ID for author string
  ULONG ulLeftClbLen;

  //allocate pString
  fOK = UtlAlloc( (PVOID *) &(pString), 0L, (LONG) MAX_SEGMENT_SIZE*sizeof(CHAR_W), NOMSG );

  //allocate normalized string
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pNormString), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
  }

  //allocate 4k for pTagRecord
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pTagRecord), 0L, (LONG) TOK_SIZE, NOMSG );
    if ( fOK )
     lTagAlloc = (LONG)TOK_SIZE;
  }

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    BOOL  fStringEqual;                // indicate string equal
    USHORT  usPutLang;                 // language id of target language
    USHORT  usPutFile;                 // file id of new entry

    // replace any 0xA0 in language name to 0xFF
    // (0xA0 is 0xFF after processing by OemToAnsi)
    REPLACE_A0_BY_FF( pTmPut->szTargetLanguage );

    //get id of target language in the put structure
    if (NTMGetIDFromName( pTmClb, pTmPut->szTargetLanguage,
                          NULL, (USHORT)LANG_KEY, &usPutLang ))
    {
      usPutLang = 1;
    } /* endif */

    NTMGetIDFromName( pTmClb, pTmPut->szAuthorName, 			// get author ID
                      NULL, (USHORT)AUTHOR_KEY, &usAuthorId );

    usRc = NTMGetIDFromName( pTmClb, pTmPut->szFileName,		 // get file id
                             pTmPut->szLongName, (USHORT)FILE_KEY, &usPutFile );
    if ( !usRc )
    {
      //position at beginning of source structure in tm record
      pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);
      pByte = NTRecPos((PBYTE)(pTmRecord+1), REC_SRCSTRING);

      //calculate length of source string
      ulLen = (RECLEN(pTMXSourceRecord) - sizeof( TMX_SOURCE_RECORD ));
      //copy and compare source string
      memset( pString, 0, MAX_SEGMENT_SIZE * sizeof(CHAR_W));
      ulLen = EQFCompress2Unicode( pString, pByte, ulLen );
      fStringEqual = ! UTF16strcmp( pString, pSentence->pNormString );

      if ( fStringEqual )
      {
        BOOL fNewerTargetExists = FALSE;
        ULONG   ulLeftTgtLen;          // remaining target length

        NTASSERTLEN(RECLEN(pTmRecord), pTmRecord->usFirstTargetRecord, 4712)

        // get length of target block to work with
        //source strings equal - position at first target record
        ulLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;
        pStartTarget = NTRecPos((PBYTE)(pTmRecord+1), REC_FIRSTTARGET);
        pTMXTargetRecord = (PTMX_TARGET_RECORD)pStartTarget;

        //tokenize target string in put structure
        usRc = TokenizeTarget( pTmPut->szTarget, pNormString,
                            &pTagRecord, &lTagAlloc, pTmPut->szTagTable,
                            &usNormLen, pTmClb );

        fStop = (usRc != 0);
        //RJ: 04/01/22: P018830:
        // loop through all target records
        // delete entry if current segment has already been translated
        TMLoopAndDelTargetClb(pTmRecord, pTmPut, pSentence, usPutLang, usPutFile, &fNewerTargetExists );

        // recalc since record may have changed during delete above!
        ulLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;
        pStartTarget = NTRecPos((PBYTE)(pTmRecord+1), REC_FIRSTTARGET);
        pTMXTargetRecord = (PTMX_TARGET_RECORD)pStartTarget;

        if ( fNewerTargetExists )
        {
          fStop = TRUE;           // do not continue update loop
        } 
        else
        {
          //source strings are identical so loop through target records
          // and add new entry
          while ( fOK && ulLeftTgtLen && (RECLEN(pTMXTargetRecord) != 0) && !fStop )
          { 
            // check for valid target length
            NTASSERTLEN(ulLeftTgtLen, RECLEN(pTMXTargetRecord), 4713);
            
            // update target length
            ulLeftTgtLen -= RECLEN(pTMXTargetRecord);

            // next check the target language and target tag info
            // position at first target control block and to source tag info
            pClb = (PTMX_TARGET_CLB)NTRecPos((PBYTE)pTMXTargetRecord, REC_CLB);
            pTMXSourceTagTable = (PTMX_TAGTABLE_RECORD)NTRecPos(pStartTarget, REC_SRCTAGTABLE);

            // compare target language IDs and source tag record
            if ( (pClb->usLangId == usPutLang) &&
                (memcmp( pTMXSourceTagTable, pSentence->pTagRecord,
                          RECLEN(pTMXSourceTagTable)) == 0) )
            {
              // check if target string and target tag record are identical
              pByte = NTRecPos(pStartTarget, REC_TGTSTRING);
              ulLen = pTMXTargetRecord->usClb - pTMXTargetRecord->usTarget;
              ulLen = EQFCompress2Unicode( pString, pByte, ulLen );
              pString[ulLen] = EOS;

              pTMXTargetTagTable = (PTMX_TAGTABLE_RECORD)NTRecPos(pStartTarget, REC_TGTTAGTABLE);

              //compare target strings and target tag record
              if ( (UTF16strcmp( pString, pNormString) == 0) &&
                  (memcmp( pTMXTargetTagTable, pTagRecord,
                            RECLEN(pTMXTargetTagTable) ) == 0) )
              {  //target strings and target tag record are equal
                //position at first control block
                pClb = (PTMX_TARGET_CLB)NTRecPos(pStartTarget, REC_CLB);

                // loop over all target CLBs
                pTMXTargetRecord = (PTMX_TARGET_RECORD)pStartTarget;
                ulLeftClbLen = RECLEN(pTMXTargetRecord) - pTMXTargetRecord->usClb;
                while ( ulLeftClbLen && !fStop )
                {
                  if ( ((pClb->ulSegmId == pTmPut->ulSourceSegmentId) &&
                        (pClb->usFileId == usPutFile)) ||
                        pClb->bMultiple )
                  {
                    // either an identical segment already in record (update
                    // time if newer than existing one)
                    // or a segment with multiple flag set
                    // (clear multiple flag as we have now the same segment
                    // with a valid doc/segno information (intention is to
                    // get rid off multipe flagged target records asap))

                    if ( pClb->bMultiple )
                    {
                      pClb->bMultiple = FALSE;
                      pClb->lTime     = pTmPut->lTime;
                      pClb->ulSegmId  = pTmPut->ulSourceSegmentId;
                      pClb->usFileId  = usPutFile;
                      pClb->bTranslationFlag       = (BYTE)pTmPut->usTranslationFlag;
                      pClb->usAuthorId = usAuthorId;
                      fUpdate         = TRUE;
                      fStop           = TRUE;
                    }
                    else
                    {
                      if ( (pClb->lTime < pTmPut->lTime) || !pTmPut->lTime )
                      {
                        if ( pTmPut->lTime )
                        {
                          pClb->lTime = pTmPut->lTime;
                        }
                        else
                        {
                          UtlTime( &(pClb->lTime) );
                        } /* endif */
                        pClb->bTranslationFlag       = (BYTE)pTmPut->usTranslationFlag;
                        fUpdate         = TRUE;
                      } /* endif */
                      fStop           = TRUE;
                    } /* endif */

                    // adjust context part of target control block if necessary
                    NTMAdjustAddDataInTgtCLB( ppTmRecord, pulRecBufSize, pTmPut, &pClb, &pTmRecord, &pTMXTargetRecord, &ulLeftClbLen, &fUpdate );
                  } /* endif */

                  // continue with next target CLB
                  if ( !fStop )
                  {
                    ulLeftClbLen -= TARGETCLBLEN(pClb);
                    pClb = NEXTTARGETCLB(pClb);
                  } /* endif */
                } /* endwhile */

                // add new CLB if no matching CLB has been found
                if ( !fStop )
                {
                  // re-alloc record buffer if too small
                  USHORT usAddDataLen = NTMComputeAddDataSize( pTmPut->szContext, pTmPut->szAddInfo );
                  ULONG ulNewSize = RECLEN(pTmRecord) + sizeof(TMX_TARGET_CLB) + usAddDataLen;
                  if ( ulNewSize > *pulRecBufSize)
                  {
                    fOK = UtlAlloc( (PVOID *)ppTmRecord, *pulRecBufSize, ulNewSize, NOMSG );
                    if ( fOK )
                    {
                      *pulRecBufSize = ulNewSize;
                      pClb = (PTMX_TARGET_CLB) ADJUSTPTR( *ppTmRecord, pTmRecord, pClb );
                      pTMXTargetRecord = (PTMX_TARGET_RECORD) ADJUSTPTR( *ppTmRecord, pTmRecord, pTMXTargetRecord );
                      pTmRecord = *ppTmRecord;
                    }
                    else
                    {
                      usRc = ERROR_NOT_ENOUGH_MEMORY;
                    } /* endif */
                  } /* endif */

                  // make room at pCLB for a new CLB and adjust TM record
                  // length and target record length
                  if ( fOK )
                  {
                    ULONG ulNewClbLen = sizeof(TMX_TARGET_CLB) + usAddDataLen; 
                    memmove( (((PBYTE)pClb) + ulNewClbLen), pClb, RECLEN(pTmRecord) - ((PBYTE)pClb - (PBYTE)pTmRecord) );
                    RECLEN(pTmRecord) += ulNewClbLen;
                    RECLEN(pTMXTargetRecord) += ulNewClbLen;
                  } /* endif */

                  // fill-in new target CLB
                  if ( fOK )
                  {
                    pClb->bMultiple = FALSE;
                    if ( pTmPut->lTime )
                    {
                      pClb->lTime = pTmPut->lTime;
                    }
                    else
                    {
                      UtlTime( &(pClb->lTime) );
                    } /* endif */
                    pClb->ulSegmId   = pTmPut->ulSourceSegmentId;
                    pClb->usFileId   = usPutFile;
                    pClb->bTranslationFlag = (BYTE)pTmPut->usTranslationFlag;
                    pClb->usAuthorId = usAuthorId;
                    pClb->usLangId   = usPutLang;
                    pClb->usAddDataLen = 0;
                    NtmStoreAddData( pClb, ADDDATA_CONTEXT_ID, pTmPut->szContext );
                    NtmStoreAddData( pClb, ADDDATA_ADDINFO_ID, pTmPut->szAddInfo );
                  } /* endif */

                  fStop = TRUE;        // avoid add of a new target record at end of outer loop
                  fUpdate = TRUE;
                } /* endif */
              } /* endif */
            } /* endif */

            //position at next target
            if ( !fStop )
            {
			        pStartTarget = NTRecPos(pStartTarget, REC_NEXTTARGET);
			        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pStartTarget);
            } /* endif */
          } /* endwhile */
        } /* endif */

        // update TM record if required
        if ( fUpdate )
        {
          usRc = EQFNTMUpdate( pTmClb->pstTmBtree, *pulKey, (PBYTE)pTmRecord, RECLEN(pTmRecord) );
        } /* endif */

        if ( !fStop )
        {
          //all target records have been checked but nothing overlapped
          //so add new target record to end of tm record
          usRc = AddTmTarget( pTmClb, pTmPut, pSentence, ppTmRecord, pulRecBufSize, pulKey );
          pTmRecord = *ppTmRecord;
        } /* endif */
      }
      else
      {
        //source strings are not equal so try another sid or if all have been
        //tries add new tm record
        usRc = SOURCE_STRING_ERROR;
      } /* endif */
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pString, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pNormString, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pTagRecord, 0L, 0L, NOMSG );

  if ( usRc )
  {
    ERREVENT2( COMPAREPUTDATA_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */

  return( usRc );
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     AddTmTarget       add a new target record to end of tm    
//                                      record                                  
//------------------------------------------------------------------------------
// Description:       Adds a new target to tm record                            
//+---------------------------------------------------------------------------- 
// Function call:     AddTmTarget( PTMX_CLB pTmClb,                             
//                                 PTMX_PUT pTmPut,                             
//                                 PTMX_SENTENCE pSentence,                     
//                                 PTMX_RECORD pTmRecord,                       
//                                 PULONG pulKey )                              
//------------------------------------------------------------------------------
// Input parameter: PTMX_CLB pTmClb                                             
//                  PTMX_PUT pTmPut                                             
//                  PTMX_SENTENCE pSentence                                     
//                  PTMX_RECORD pTmRecord                                       
//                  PULONG pulKey                                               
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//                                                                              
//------------------------------------------------------------------------------
// Function flow:                                                               
//   tokenize target string                                                     
//   fill target control block structure                                        
//   fill target tm record structure add end of tm record                       
//   insert tm record in tm data fill                                           
//------------------------------------------------------------------------------
USHORT AddTmTarget(
  PTMX_CLB pTmClb,                  //ptr to ctl block struct
  PTMX_PUT_W pTmPut,                //pointer to get in data
  PTMX_SENTENCE pSentence,          //pointer to sentence structure
  PTMX_RECORD *ppTmRecord,          //pointer to tm record data pointer
  PULONG pulRecBufSize,             //ptr to current size of TM record buffer
  PULONG pulKey )                   //tm key
{
  PTMX_TARGET_CLB pTargetClb ;              // ptr to target ctl block
  PTMX_TARGET_RECORD pTargetRecord = NULL;  // ptr to target record
  PSZ_W pNormString = NULL;                 // ptr to normalized string
  PTMX_TAGTABLE_RECORD pTagRecord = NULL; // ptr to tag table record
  USHORT       usRc = NO_ERROR;           // return code
  USHORT       usNormLen = 0;             // length indicator
  BOOL         fOK;                       // success indicator
  LONG         lTagAlloc;                 // alloc size
  PBYTE        pByte;                     // position pointer
  PTMX_RECORD pTmRecord = *ppTmRecord;    //pointer to tm record data
  ULONG       ulAddDataLen = 0;

  //allocate target control block record
  ulAddDataLen = NTMComputeAddDataSize( pTmPut->szContext, pTmPut->szAddInfo );
  fOK = UtlAlloc( (PVOID *) &pTargetClb, 0L, (LONG)(sizeof(TMX_TARGET_CLB)+ulAddDataLen), NOMSG );

  //allocate normalized string
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pNormString), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
  }

  //allocate target record
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pTargetRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );
  }

  //allocate 4k for pTagRecord
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pTagRecord), 0L, (LONG) TOK_SIZE, NOMSG );
    if ( fOK )
      lTagAlloc = (LONG)TOK_SIZE;
  }

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    usRc = TokenizeTarget( pTmPut->szTarget, pNormString, &pTagRecord,
                           &lTagAlloc, pTmPut->szTagTable, &usNormLen, pTmClb );
    if ( usRc == NO_ERROR )
    {
      usRc = FillClb( &pTargetClb, pTmClb, pTmPut );
      if ( usRc == NO_ERROR )
      {
        //fill target record
        FillTargetRecord( pSentence,  //ptr to sentence structure
                          pTagRecord, //ptr to target string tag table
                          pNormString,//ptr to target normalized string
                          usNormLen,  //length of target normalized string
                          &pTargetRecord,        //filled tm record returned
                          pTargetClb );                    //tm target control block

        //check space requirements
        {
          // re-alloc record buffer if too small
          ULONG ulNewSize = RECLEN(pTmRecord) + sizeof(TMX_TARGET_RECORD) +
                            RECLEN(pTargetRecord);
          if ( ulNewSize > *pulRecBufSize)
          {
            fOK = UtlAlloc( (PVOID *)ppTmRecord, *pulRecBufSize, ulNewSize, NOMSG );
            if ( fOK )
            {
              *pulRecBufSize = ulNewSize;
              pTmRecord = *ppTmRecord;
            }
            else
            {
              usRc = ERROR_NOT_ENOUGH_MEMORY;
            } /* endif */
          } /* endif */
        }

        if ( fOK )
        {
          //postion at end on tm record
          pByte = (PBYTE)pTmRecord;
          pByte += RECLEN(pTmRecord);

          //add new target record to end
          memcpy( pByte, pTargetRecord, RECLEN(pTargetRecord) );

          //update overall length of tm record
          RECLEN(pTmRecord) += RECLEN(pTargetRecord);


          //add updated tm record to database
          usRc = EQFNTMUpdate( pTmClb->pstTmBtree,
                               *pulKey,
                               (PBYTE)pTmRecord,
                               RECLEN(pTmRecord) );
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &(pTargetClb), 0L, 0L, NOMSG);
  UtlAlloc( (PVOID *) &(pTargetRecord), 0L, 0L, NOMSG);
  UtlAlloc( (PVOID *) &(pNormString), 0L, 0L, NOMSG);
  UtlAlloc( (PVOID *) &(pTagRecord), 0L, 0L, NOMSG);

  if ( usRc )
  {
    ERREVENT2( ADDTMTARGET_LOC, ERROR_EVENT, usRc, TM_GROUP, NULL );
  } /* endif */


  return( usRc );
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     FillTargetRecord fills target record and adds to end of   
//                    tm record                                                 
//------------------------------------------------------------------------------
// Description:       Fills a tm target record                                  
//+---------------------------------------------------------------------------- 
// Function call:     FillTargetRecord( PTMX_SENTENCE pSentence,                
//                                    PTMX_TAGTABLE_RECORD pTagRecord,          
//                                    PSZ pNormString,                          
//                                    USHORT usNormLen,                         
//                                    PTMX_TARGET_RECORD pTMXTargetRecord,      
//                                    PTMX_TARGET_CLB pTargetClb )              
//------------------------------------------------------------------------------
//   Input parameter:                                                           
//     PTMX_SENTENCE pSentence,           //ptr to sentence structure           
//     PTMX_TAGTABLE_RECORD pTagRecord,   //ptr to target string tag table      
//     PSZ pNormString,                   //ptr to target normalized string     
//     USHORT usNormLen,                  //length of target normalized string  
//     PTMX_TARGET_CLB pTargetClb         //ptr to target control block         
//------------------------------------------------------------------------------
// Output parameter: PTMX_TARGET_RECORD pTMXTargetRecord //ptr to target record 
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//                                                                              
//------------------------------------------------------------------------------
// Function flow:                                                               
//   fill target tm record structure                                            
//------------------------------------------------------------------------------

VOID FillTargetRecord
(
  PTMX_SENTENCE pSentence,           //ptr to sentence structure
  PTMX_TAGTABLE_RECORD pTagRecord,   //ptr to target string tag table
  PSZ_W pNormString,                 //ptr to target normalized string
  USHORT usNormLen,                  //length of target normalized string
  PTMX_TARGET_RECORD *ppTMXTargetRecord,  //ptr to target record
  PTMX_TARGET_CLB pTargetClb         //ptr to target control block
)
{
  PBYTE pTarget;                                 //ptr to target record
  PTMX_TARGET_RECORD pTMXTargetRecord;           //ptr to target record

  pTMXTargetRecord = *ppTMXTargetRecord;

  //position to start of target structure
  pTarget = (PBYTE)pTMXTargetRecord;

  //set source tag table offset
  pTMXTargetRecord->usSourceTagTable = sizeof(TMX_TARGET_RECORD);

  //position pointer for source tag table
  pTarget += pTMXTargetRecord->usSourceTagTable;

  //copy source tag table record to correct position
  memcpy( pTarget, pSentence->pTagRecord, RECLEN(pSentence->pTagRecord) );

  //set target tag table start offset
  pTMXTargetRecord->usTargetTagTable = (USHORT)(pTMXTargetRecord->usSourceTagTable +
                                       RECLEN(pSentence->pTagRecord));
  //adjust target pointer for target tag table
  pTarget += RECLEN(pSentence->pTagRecord);

  //copy target tag table record to correct position
  memcpy( pTarget, pTagRecord, RECLEN(pTagRecord) );

  //set target string start offset
  pTMXTargetRecord->usTarget = (USHORT)(pTMXTargetRecord->usTargetTagTable +
                                       RECLEN(pTagRecord));
  //adjust target pointer for target string
  pTarget += RECLEN(pTagRecord);

  //copy target string to correct position
  { ULONG ulTempLen = usNormLen;
    ulTempLen = EQFUnicode2Compress( pTarget, pNormString, ulTempLen );
    usNormLen = (USHORT)ulTempLen;
  }
//  memcpy( pTarget, pNormString, usNormLen );

  //set target string control block start offset
  pTMXTargetRecord->usClb = pTMXTargetRecord->usTarget + usNormLen;

  //adjust target pointer for control block
  pTarget += usNormLen;

  //copy target control block
  memcpy( pTarget, pTargetClb, TARGETCLBLEN(pTargetClb) );

  //size of target record
  RECLEN(pTMXTargetRecord) = sizeof(TMX_TARGET_RECORD) +
                                  RECLEN(pSentence->pTagRecord) +
                                  RECLEN(pTagRecord) +
                                  usNormLen +
                                  TARGETCLBLEN(pTargetClb);
  *ppTMXTargetRecord = pTMXTargetRecord;
}



/**********************************************************************/
/* the following routines implement a fast tokenize independent on    */
/* language characteristics                                           */
/**********************************************************************/

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TmtXUpdSeg       update segment data                      
//------------------------------------------------------------------------------
// Description:       Updates the segment data of a specific segment            
//------------------------------------------------------------------------------
// Function call:  TmtXUpdSeg( pTmClb, pTmPutIn, ulUpdKey, usUpdTarget, usFlags 
//------------------------------------------------------------------------------
// Input parameter:   PTMX_CLB  pTmClb         control block                    
//                    PTMX_PUT_IN pTmPutIn     input structure                  
//                    ULONG       ulUpdKey     SID of record being updated      
//                    USHORT      usUpdTarget  number of target being updated   
//                    USHORT      usFlags      flags controlling the update     
//------------------------------------------------------------------------------
USHORT TmtXUpdSeg
(
  PTMX_CLB    pTmClb,      // ptr to ctl block struct
  PTMX_PUT_IN pTmPutIn,    // ptr to put input data
  ULONG       ulUpdKey,    // SID of record being updated
  USHORT      usUpdTarget, // number of target being updated
  USHORT      usFlags      // flags controlling the updated fields
)
{
  BOOL       fOK;                      // success indicator
  USHORT     usRc = NO_ERROR;          // return code
  PTMX_RECORD pTmRecord = NULL;        // pointer to tm record
  BOOL        fLocked = FALSE;         // TM-database-has-been-locked flag
  ULONG  ulLen = 0;                    //length indicator
  PBYTE pByte;                         //position ptr
  USHORT usTarget;                     //nr of target records in tm record
  PTMX_TARGET_RECORD pTMXTargetRecord; // ptr to target record
  PTMX_TARGET_CLB    pTargetClb;       // ptr to target CLB
  ULONG       ulLeftClbLen;            // remaining length of CLB area
  ULONG       ulRecBufSize = 0L;       // current size of record buffer

  //allocate 32K for tm record
  fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );
  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    ulRecBufSize = TMX_REC_SIZE;
  } /* endif */

  // lock TM database
  if ( !usRc && pTmClb->fShared )
  {
    // use only two retries as locking already uses a wait loop...
    SHORT sRetries = 2;
    do
    {
      usRc = NTMLockTM( pTmClb, TRUE, &fLocked );
      if ( usRc == BTREE_IN_USE )
      {
        UtlWait( MAX_WAIT_TIME );
        sRetries--;
      } /* endif */
    }
    while( (usRc == BTREE_IN_USE) && (sRetries > 0));
  } /* endif */

  // Update internal buffers if database has been modified by other users
  if ( !usRc && pTmClb->fShared )
  {
    usRc = NTMCheckForUpdates( pTmClb );
  } /* endif */

  // get TM record being modified and update the record
  if ( !usRc )
  {
    usRc = BTREE_NOT_FOUND;

    ulLen = TMX_REC_SIZE;
    usRc = EQFNTMGet( pTmClb->pstTmBtree, ulUpdKey, (PCHAR)pTmRecord,
                      &ulLen );

    if ( usRc == BTREE_BUFFER_SMALL)
    {
      fOK = UtlAlloc( (PVOID *)&pTmRecord, ulRecBufSize, ulLen, NOMSG );
      if ( fOK )
      {
        ulRecBufSize = ulLen;
        memset( pTmRecord, 0, ulLen );

        usRc = EQFNTMGet( pTmClb->pstTmBtree,
                          ulUpdKey,  //tm record key
                          (PCHAR)pTmRecord,   //pointer to tm record data
                          &ulLen );    //length
      }
      else
      {
        usRc = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    } /* endif */

    if ( usRc == NO_ERROR )
    {
      //check if record is empty
      if ( ( RECLEN(pTmRecord) != 0) &&
           (pTmRecord->usFirstTargetRecord < RECLEN(pTmRecord)) )
      {
        ULONG   ulLeftTgtLen;                    // remaining target length
        ulLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;

        if ( !usRc )
        {
          //find target record specified in usUpdTarget
          //move pointer to first target
          pByte = (PBYTE)(pTmRecord);
          pByte += pTmRecord->usFirstTargetRecord;
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
          pTargetClb = (PTMX_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
          ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                         pTMXTargetRecord->usClb;
          ulLeftClbLen -= TARGETCLBLEN(pTargetClb); // subtract size of current CLB
          usTarget = 1;           //initialize counter

          //loop until correct target is found
          while ( (usTarget < usUpdTarget) && ulLeftTgtLen )
          {
            // position to first target CLB
            pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
            pTargetClb = (PTMX_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
            ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                           pTMXTargetRecord->usClb;
            ulLeftClbLen -= TARGETCLBLEN(pTargetClb); // subtract size of current CLB

            // loop over all target CLBs
            while ( (usTarget < usUpdTarget) && ulLeftClbLen )
            {
              usTarget++;
              pTargetClb = NEXTTARGETCLB(pTargetClb);
              ulLeftClbLen -= TARGETCLBLEN(pTargetClb);
            } /* endwhile */

            // continue with next target if no match yet
            if ( usTarget < usUpdTarget )
            {
              // position at next target
              pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
              //move pointer to end of target
              if (ulLeftTgtLen >= RECLEN(pTMXTargetRecord))
              {
                ulLeftTgtLen -= RECLEN(pTMXTargetRecord);
              }
              else
              {
                ulLeftTgtLen = 0;
              } /* endif */
              pByte += RECLEN(pTMXTargetRecord);
              pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
              pTargetClb = (PTMX_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
              usTarget++;
            } /* endif */
          } /* endwhile */

          if ( usTarget == usUpdTarget )
          {
            //position at start of target record
            pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

            //if target record exists
            if ( ulLeftTgtLen && ( RECLEN(pTMXTargetRecord) != 0) )
            {
              // update requested fields

              // change markup/tag table if requested
              if ( (usFlags & TMUPDSEG_MARKUP) &&
                   (pTmPutIn->stTmPut.szTagTable[0] != EOS) )
              {
                PTMX_TAGTABLE_RECORD pTMXTagTableRecord;
                USHORT usNewId;
                PBYTE pByte;

                // get ID for new tag table
                usRc = NTMGetIDFromName( pTmClb, pTmPutIn->stTmPut.szTagTable,
                                         NULL,
                                         (USHORT)TAGTABLE_KEY,
                                         &usNewId );

                // update source tag table record
                pByte = (PBYTE)pTMXTargetRecord;
                pByte += pTMXTargetRecord->usSourceTagTable;
                pTMXTagTableRecord = (PTMX_TAGTABLE_RECORD)pByte;
                pTMXTagTableRecord->usTagTableId = usNewId;

                // update target tag table record
                pByte = (PBYTE)pTMXTargetRecord;
                pByte += pTMXTargetRecord->usTargetTagTable;
                pTMXTagTableRecord = (PTMX_TAGTABLE_RECORD)pByte;
                pTMXTagTableRecord->usTagTableId = usNewId;
              } /* endif */

              // update MT flag if requested
              if ( (usRc == NO_ERROR) &&
                   (usFlags & TMUPDSEG_MTFLAG) )
              {
                // set type of translation flag
                pTargetClb->bTranslationFlag = (BYTE)pTmPutIn->stTmPut.usTranslationFlag;
              } /* endif */

              // update target language if requested
              if ( (usRc == NO_ERROR) &&
                   (usFlags & TMUPDSEG_TARGLANG) )
              {
                // replace any 0xA0 in language name to 0xFF
                // (0xA0 is 0xFF after processing by OemToAnsi)
                REPLACE_A0_BY_FF( pTmPutIn->stTmPut.szTargetLanguage );

                // set target language
                usRc = NTMGetIDFromName( pTmClb, pTmPutIn->stTmPut.szTargetLanguage,
                                         NULL,
                                         (USHORT)LANG_KEY,
                                         &pTargetClb->usLangId );
              } /* endif */

              // update segment date if requested
              if ( (usRc == NO_ERROR) &&
                   (usFlags & TMUPDSEG_DATE) )
              {
                pTargetClb->lTime = pTmPutIn->stTmPut.lTime;
              } /* endif */


              // rewrite TM record
              if ( usRc == NO_ERROR )
              {
                usRc = EQFNTMUpdate( pTmClb->pstTmBtree, ulUpdKey,
                                     (PBYTE)pTmRecord, RECLEN(pTmRecord) );
              } /* endif */
            }
            else
            {
              // target not found
              usRc = BTREE_NOT_FOUND;
            } /* endif */
          }
          else
          {
            // record not found
            usRc = BTREE_NOT_FOUND;
          } /* endif */
        } /* endif */
      }
      else
      {
        // record is empty and should not be updated
        usRc = BTREE_NOT_FOUND;
      } /* endif */
    } /* endif */
  } /* endif */

  // unlock TM database if database has been locked
  if ( fLocked )
  {
    NTMLockTM( pTmClb, FALSE, &fLocked );
  } /* endif */

  //release allocated memory
  UtlAlloc( (PVOID *)&pTmRecord, 0L, 0L, NOMSG );

  return( usRc );
} /* end of function TmtXUpdSeg */


/**********************************************************************/
/* TMDelTargetClb                                                     */
/*                                                                    */
/* Delete target CLB and, if there is no more target CLB left, the    */
/* complete target record                                             */
/* Return TRUE if target record has been deleted                      */
/**********************************************************************/
BOOL TMDelTargetClb
(
  PTMX_RECORD        pTmRecord,        // ptr to TM record
  PTMX_TARGET_RECORD pTargetRecord,    // ptr to target record within TM record
  PTMX_TARGET_CLB    pTargetClb        // ptr to target control record
)
{
  BOOL fTargetRemoved = FALSE;
  PBYTE  pTmEnd;
  PBYTE  pEndTarget;
  USHORT  usTargetCLBLen;
  pTmEnd = (PBYTE)pTmRecord;
  pTmEnd += RECLEN(pTmRecord);

  // check if there are more than one CLBs in current target record
  usTargetCLBLen = TARGETCLBLEN(pTargetClb);
  if ( RECLEN(pTargetRecord) <=
       (ULONG)(pTargetRecord->usClb + usTargetCLBLen) )
  {
    // only one CLB in target record ==> delete complete target record

    //remember length of current target to be replaced
    pEndTarget = (PBYTE)pTargetRecord + RECLEN(pTargetRecord);

    //calculate new length of TM record
    RECLEN(pTmRecord) -= RECLEN(pTargetRecord);

    //remove the current target record
    memmove( pTargetRecord, pEndTarget, pTmEnd - pEndTarget );

    // remember that the complete target record has been removed
    fTargetRemoved = TRUE;

    // initialize last target record
    pTmEnd = (PBYTE)pTmRecord;
    pTmEnd += RECLEN(pTmRecord);
    pTargetRecord = (PTMX_TARGET_RECORD)pTmEnd;
    RECLEN(pTargetRecord) = 0;
  }
  else
  {
    // delete only current target CLB

    //calculate new length of tm record
    RECLEN(pTmRecord) -= usTargetCLBLen;
    RECLEN(pTargetRecord) -= usTargetCLBLen;

    //remove the current target CLB
    {
      PBYTE pNextClb = ((PBYTE)pTargetClb) + usTargetCLBLen;
      memmove( pTargetClb, pNextClb, pTmEnd - pNextClb );
    }
  } /* endif */

  return( fTargetRemoved );
} /* end of function TMDelTargetClb */

// Do a primitive on BOCU (see IBM DeveloperWorks) based compression...
static long lBoundary[6] = {   -0x02F41,   -0x041,  -0x01, 0x03f, 0x02f3f, 0x010ffff };
static int  byteCount[6] = {          3,        2,      1,     1,       2,         3 };
static long lOffset[6]   = {  0x010ffff,  0x04040,  0x080, 0x080, 0x0bfc0, 0x0ef0000 };
static long lCompress[6] = {       0x10,    0x03f,  0x07f, 0x0bf,   0x0ee,     0x0ff };




ULONG EQFUnicode2Compress( PBYTE pTarget, PSZ_W pInput, ULONG ulLenChar )
{
  ULONG ulLen = ulLenChar * sizeof(CHAR_W);


  if ( ulLen < 20 )   // don't care about too short strings
  {
    *pTarget++ = 0;
    memcpy( pTarget, pInput, ulLen );
    pTarget[ulLen] = EOS;
  }
  else
  {
    long delta;
    PBYTE pTemp;
    PSZ_W pTempIn = pInput;
    int  oldCodePoint = 0x80;
    int  i, bCount, bCountTemp, cp;


    *pTarget++ = BOCU_COMPRESS;
    pTemp = pTarget;


    for ( ulLen = 0; ulLen < ulLenChar; ulLen++ )
    {
      cp = *pInput++;
      delta = cp - oldCodePoint;
      oldCodePoint = cp;


      for (i=0;  ; ++i)   // will always break
      {
         if ( delta <= lBoundary[i])
         {
           delta += lOffset[i];
           bCountTemp = bCount = byteCount[i];

           while (--bCount >0 )
           {
             pTarget[bCount] = ((BYTE)delta & 0x0ff);
             delta = (delta & ~0xff) >> 8;
           }

           // store lead byte
           *pTarget  = (BYTE)(*pTarget  + (BYTE)delta);
           pTarget += bCountTemp;
           break;
         }
      }
    }
    ulLen = pTarget - pTemp;
//    pTarget[ulLen] = EOS;
    pTemp[ulLen] = EOS;

    // just in case we enlarged the string -- use original one...
    if (ulLen > ulLenChar * sizeof(CHAR_W))
    {
      pTarget = pTemp;
      ulLen = ulLenChar * sizeof(CHAR_W);
      *(pTarget-1) = 0;   // no compression
      memcpy( pTarget, pTempIn, ulLen );
      pTarget[ulLen] = EOS;
    }
  }

  return ulLen+1;
}

ULONG EQFCompress2Unicode( PSZ_W pOutput, PBYTE pTarget, ULONG ulLenComp )
{
    ULONG  ulLen = ulLenComp-1;
    BYTE b = *pTarget++;
    if ( b == 0 )  // no compression
    {
        memcpy( pOutput, pTarget, ulLen );
        ulLen = ulLenComp / sizeof(CHAR_W);
        pOutput[ ulLen ] = EOS;
    }
    else if (b == BOCU_COMPRESS)
    {
      PSZ_W pTemp = pOutput;
      long delta = 0;
      USHORT iLen = 0;
      int  oldCodePoint = 0x80;
      int  i, bCount;

      for ( iLen = 0; iLen < ulLen; iLen++ )
      {
        delta = (unsigned char)*pTarget++;

        for (i=0; ; i++ )
        {
          if ( delta <= lCompress[i] )
          {
            bCount = byteCount[i];

            while (--bCount >0 )
            {
              delta = (delta << 8) + (unsigned char)*pTarget++;
              iLen++;
            }

            delta = delta + oldCodePoint - lOffset[i];

            *pOutput++ = (unsigned short) delta;
            oldCodePoint = delta;

            break;
          }
        }
      }
      *pOutput = 0;
      ulLen = (pOutput - pTemp);
    }
    else
    {
        assert( 0 == 1);
    }
    return ulLen;
}


// adjust context part of
// target control block if necessary

USHORT NTMAdjustAddDataInTgtCLB
(
	PTMX_RECORD	      *ppTmRecordStart,
	PULONG     		   pulRecBufSize,
	PTMX_PUT_W 		   pTmPut,
	PTMX_TARGET_CLB    *ppClb,
	PTMX_RECORD        *ppCurTmRecord,
	PTMX_TARGET_RECORD *ppTMXTargetRecord,
	PULONG  		   pulLeftClbLen,
	PBOOL 			   pfUpdate)
{
  BOOL    fOK = TRUE;
  USHORT  usOldLen = 0;
  ULONG   ulNewLen = 0;
  USHORT  usRc = NO_ERROR;
  PTMX_RECORD  pCurTmRecord = NULL;
  PTMX_TARGET_RECORD  pTMXTargetRecord;           //ptr to target record
  PTMX_TARGET_CLB     pCurClb;
  PTMX_TARGET_CLB     pNextClb = NULL;

  pTMXTargetRecord = *ppTMXTargetRecord;
  pCurTmRecord     = *ppCurTmRecord;
  pCurClb          = *ppClb;
  usOldLen         = pCurClb->usAddDataLen;

  if ( pTmPut->szContext[0] )
  {
	  ulNewLen = NTMComputeAddDataSize( pTmPut->szContext, pTmPut->szAddInfo );
  } /* endif */

  if ( usOldLen == ulNewLen )
  {
	  // no resizing required, set update flag as data may have been changed
	  if ( ulNewLen )
	  {
		  *pfUpdate = TRUE;
	  } /* endif */
  }
  else if ( usOldLen > ulNewLen )
  {
	  // new length is smaller, reduce CLB size later when the additional data area has been re-filled but
    // remember position of next CLB
	  pNextClb = NEXTTARGETCLB(pCurClb);

  }
  else
  {
	  // new length is larger, re-alloc record and adjust pointers
	  ULONG ulDiff = ulNewLen - usOldLen;

	  // re-alloc record buffer if too small
	  ULONG ulNewSize = RECLEN(pCurTmRecord) + (ULONG)ulDiff;
	  if ( ulNewSize > *pulRecBufSize)
	  {
	    fOK = UtlAlloc( (PVOID *)ppTmRecordStart, *pulRecBufSize,
					    ulNewSize, NOMSG );
	    if ( fOK )
	    {
		    *pulRecBufSize = ulNewSize;
		    pCurClb = (PTMX_TARGET_CLB) ADJUSTPTR( *ppTmRecordStart, pCurTmRecord, pCurClb );
		    // *ppTMRecordStart + (pClb-ppCurTmRecord)
		    pTMXTargetRecord = (PTMX_TARGET_RECORD) ADJUSTPTR( *ppTmRecordStart, pCurTmRecord, pTMXTargetRecord );
		    pCurTmRecord = *ppTmRecordStart;
	    }
	    else
	    {
		    usRc = ERROR_NOT_ENOUGH_MEMORY;
	    } /* endif */
	  } /* endif */

	  // enlarge current control block and adjust TM record
	  // length and target record length
	  if ( fOK )
	  {
	    pNextClb = NEXTTARGETCLB(pCurClb);
	    memmove( ((PBYTE)pNextClb) + ulDiff, pNextClb, RECLEN(pCurTmRecord) - ((PBYTE)pNextClb - (PBYTE)pCurTmRecord) );
	    RECLEN(pCurTmRecord)  += ulDiff;
	    RECLEN(pTMXTargetRecord) += ulDiff;
	    *pulLeftClbLen += ulDiff;
	    *pfUpdate         = TRUE;
	  } /* endif */
  } /* endif */

  // update additional data
  pCurClb->usAddDataLen = 0;
  NtmStoreAddData( pCurClb, ADDDATA_CONTEXT_ID, pTmPut->szContext );
  NtmStoreAddData( pCurClb, ADDDATA_ADDINFO_ID, pTmPut->szAddInfo );

  // reduce size of CLB when the new content is smaller
  if ( usOldLen > ulNewLen )
  {
	  // new length is smaller, reduce the CLB size and adjust following data
	  ULONG ulDiff = usOldLen - ulNewLen;
	  memmove( ((PBYTE)pNextClb) - ulDiff, pNextClb, RECLEN(pCurTmRecord) - ((PBYTE)pNextClb - (PBYTE)pCurTmRecord) );
	  RECLEN(pCurTmRecord)       -= ulDiff;
	  RECLEN(pTMXTargetRecord)   -= ulDiff;
	  *pulLeftClbLen -= ulDiff;
	  *pfUpdate         = TRUE;
  } /* endif */

  *ppTMXTargetRecord = pTMXTargetRecord;
  *ppCurTmRecord = pCurTmRecord;
  *ppClb = pCurClb;
  return (usRc);
} // end of context handling

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TMLoopAndDelTargetClb                     
//------------------------------------------------------------------------------
// Description:       This function loops thru all target records and deletes   
//                    the current entry, if found                               
//+---------------------------------------------------------------------------- 
// Function call:     TMLoopAndDelTargetClb( PTMX_RECORD pTmRecord              
//                                    PTMX_PUT_W  pTmPut,                       
//                                    PTMX_SENTENCE pSentence,                  
//                                    USHORT       usPutLang ,                  
//                                    USHORT       usPutFile                    
//------------------------------------------------------------------------------
// Input parameter: PTMX_RECORD         pTmRecord,							    
//                   PTMX_PUT_W 			pTmPut,                             
//                   PTMX_SENTENCE       pSentence,                             
//                   USHORT              usPutLang,                             
//                   USHORT              usPutFile                              
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:      NO_ERROR                                                   
//------------------------------------------------------------------------------
// Function flow:                                                               
//     loop through all target records in tm record checking                    
//       if src tagtable and src tags are equal                                 
//                loop over all target CLBs or until fStop                      
//                   if lang + segment+file id found (exact-exact-found!)       
//                      if entry is older                                       
//                         delete it, fDel = TRUE                               
//                      else goon with search in next tgt CLB (control block)   
//                   else                                                       
//                      goon with search in next tgt CLB (control block)        
//                endloop                                                       
//       endif                                                                  
//       if not fDel                                                            
//          position at next target record                                      
//     endloop                                                                  
//------------------------------------------------------------------------------

USHORT TMLoopAndDelTargetClb
(
	PTMX_RECORD         pTmRecord,
	PTMX_PUT_W 			pTmPut,
	PTMX_SENTENCE       pSentence,
	USHORT              usPutLang,
  USHORT              usPutFile,
  PBOOL               pfNewerTargetExists
)
{
  USHORT 				usRc = NO_ERROR;
  PTMX_TARGET_CLB    	pClb = NULL;    //ptr to target control block
  PTMX_TAGTABLE_RECORD 	pTMXSrcTTable = NULL; //ptr to source tag info
  ULONG        			ulLeftClbLen;
  ULONG        			ulLeftTgtLen;
  BOOL         			fTgtRemoved = FALSE;
  PTMX_TARGET_RECORD  	pTMXTgtRec = NULL;
  BOOL         			fDel = FALSE;

  // preset callers flag
  *pfNewerTargetExists = FALSE;

  // loop until either end of record or one occ. found&deleted
  ulLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;

  //source strings equal, position at first target record
  pTMXTgtRec = (PTMX_TARGET_RECORD)NTRecPos((PBYTE)(pTmRecord+1), REC_FIRSTTARGET);

  while ( ulLeftTgtLen && (RECLEN(pTMXTgtRec) != 0) && !fDel )
  {
	NTASSERTLEN(ulLeftTgtLen, RECLEN(pTMXTgtRec), 4713);
	ulLeftTgtLen -= RECLEN(pTMXTgtRec);

	// pos at source tag info
	pTMXSrcTTable = (PTMX_TAGTABLE_RECORD)NTRecPos((PBYTE)pTMXTgtRec,
												   REC_SRCTAGTABLE);
	// compare source tag record
	if ( (memcmp( pTMXSrcTTable, pSentence->pTagRecord,
		  RECLEN(pTMXSrcTTable)) == 0) )
	{
		pClb = (PTMX_TARGET_CLB)NTRecPos((PBYTE)pTMXTgtRec, REC_CLB);
		ulLeftClbLen = RECLEN(pTMXTgtRec) - pTMXTgtRec->usClb;

		while ( ulLeftClbLen && !fDel )
		{
			if ( (pClb->usLangId == usPutLang) &&
			     (pClb->ulSegmId == pTmPut->ulSourceSegmentId) &&
			     (pClb->usFileId == usPutFile) && !pClb->bMultiple )
			{  	// remove target CLB and target record (if only 1 CLB)
				// as the segment is putted with a new value
				if ( (pClb->lTime < pTmPut->lTime) || !pTmPut->lTime )
				{
				  ulLeftClbLen -= TARGETCLBLEN(pClb);
				  // loop over all CLBs of this target record and remove
				  // any CLB for the current segment
				  fTgtRemoved = TMDelTargetClb( pTmRecord, pTMXTgtRec, pClb);
				  fDel = TRUE;
				}
				else
				{  // existing match is newer, continue with next one
				  ulLeftClbLen -= TARGETCLBLEN(pClb);
          *pfNewerTargetExists = TRUE;
				  pClb = NEXTTARGETCLB(pClb);
				} /* endif */
			}
			else
			{ 	// no match, try next one
			  ulLeftClbLen -= TARGETCLBLEN(pClb);
			  pClb = NEXTTARGETCLB(pClb);
			} /* endif */
		} /* endwhile */
	} /* endif */
	//position at next target
	if ( !fDel )
	{
	  pTMXTgtRec = (PTMX_TARGET_RECORD)NTRecPos((PBYTE)pTMXTgtRec, REC_NEXTTARGET);
	} /* endif */
   } /* endwhile */
   return(usRc);
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     NTRecPos                                                  
//------------------------------------------------------------------------------
// Description:       This function positions at the requested position         
//                    in the given record                                       
//+---------------------------------------------------------------------------- 
// Function call:     NTRecPos( pStart, iType)                                  
//------------------------------------------------------------------------------
// Input parameter: PBYTE pStart,                							    
//                   int  iType                                                 
//------------------------------------------------------------------------------
// Returncode type: PBYTE                                                      
//------------------------------------------------------------------------------
// Returncodes:      new position                                               
//------------------------------------------------------------------------------
// Function flow:                                                               
//     switch(Type )                                                            
//       case REC_CLB: position a first TMX_TARGET_CLB                          
//       case REC_SRCTAGTABLE: position at source TMX_TAGTABLE_RECORD           
//       case REC_TGTTAGTABLE: position at target TMX_TAGTABLE_RECORD           
//       case REC_NEXTTARGET : position at next TMX_TARGET_RECORD               
//       case REC_TGTSTRING: position at target string                          
//       case REC_SRCSTRING:  position at source string                         
//       case REC_SRCTAGTABLE: position at sourc TMX_TAGTABLE_RECORD            
//       case REC_FIRSTTARGET: position at begin of first TMX_TARGET_RECORD     
//     endswitch                                                                
//------------------------------------------------------------------------------

static
PBYTE
NTRecPos
(
	PBYTE pStart,
	int iType
)
{
	PBYTE  pNewPosition = NULL;
	PTMX_TARGET_RECORD  pTmpTgtRecord = NULL;

	switch (iType)
	{
	 	case REC_CLB:
            pTmpTgtRecord = (PTMX_TARGET_RECORD)pStart;
            pNewPosition  = pStart + pTmpTgtRecord->usClb;
	 	break;
	 	case REC_SRCTAGTABLE:
	 	    pTmpTgtRecord = (PTMX_TARGET_RECORD)pStart;
	 		pNewPosition  = pStart + pTmpTgtRecord->usSourceTagTable;
	 	break;
	 	case REC_TGTTAGTABLE:
	 	    pTmpTgtRecord = (PTMX_TARGET_RECORD)pStart;
	 		pNewPosition  = pStart + pTmpTgtRecord->usTargetTagTable;
	 	break;
	 	case REC_NEXTTARGET:
	 	    pTmpTgtRecord = (PTMX_TARGET_RECORD)pStart;
	 	    pNewPosition = pStart + RECLEN(pTmpTgtRecord);
	 	break;
	 	case REC_TGTSTRING:
	 	    pTmpTgtRecord = (PTMX_TARGET_RECORD)pStart;
	 	    pNewPosition = pStart + pTmpTgtRecord->usTarget;
        break;
        case REC_SRCSTRING:
          {
			PTMX_SOURCE_RECORD pTmpSrcRecord = (PTMX_SOURCE_RECORD)pStart;
            pNewPosition = pStart + pTmpSrcRecord->usSource;
	      }
        break;
        case REC_FIRSTTARGET:
          {
            PTMX_SOURCE_RECORD pTmpSrcRecord = (PTMX_SOURCE_RECORD)pStart;
            pNewPosition = pStart + RECLEN(pTmpSrcRecord);
	      }
          break;
	 	default:
          pNewPosition = NULL;
	 	break;
    } /* endswitch */

    return (pNewPosition);
}

