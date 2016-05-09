//+----------------------------------------------------------------------------+
//|EQFTMRTV.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:        G. Queck                                                     |
//+----------------------------------------------------------------------------+
//|Description:   TM retrieve functions                                        |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+

/*--------------------------------------------------------------------------*\
|  MODULE NAME: EQFRTRV.C - Get and Extract                                  |
|                                                                            |
|  Functions:                                                                |
|  ----------                                                                |
|  TmtGet                                                                    |
|  TmtExtract                                                                |
|  CheckSimilar                                                              |
|  CheckExact                                                                |
|  CalcExactIndustry                                                         |
|  RankNewMatch                                                              |
|  GetSegByAddr                                                              |
|  TmtInfo                                                                   |
|  CheckWord                                                                 |
|  TmtGetTMPart                                                              |
|  LengthCorrectThresholds                                                   |
\*--------------------------------------------------------------------------*/

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#include <eqf.h>                  // General Translation Manager include file

#include <EQFTMI.H>               // Private header file of Translation Memory
#include <time.h>              // C time library

/*---------------------------------------------------------------------*\
|                                Tmt Get                                |
+-----------------------------------------------------------------------+
|  Function name      : TmtGet                                          |
|  Description        : Gets exact or similar matches from TM.          |
|  Function Prototype : TmtGet (ptmtg, pGetIn, pGetOut)                 |
+-----------------------------------------------------------------------+
|  Implementation  Remarks                                              |
|  -------------------------                                            |
|  Two modes of search are performed. Initially, SimilarMod is active.  |
|  In this mode each of the segments in the appropriate cluster         |
|  is checked first for similarity, and if it passes, then it is        |
|  also checked for exact match.                                        |
|  The mode of search is depended from the parameter usNumMatchesReq    |
|  passed in the GET_ structure. For this two defines are available     |
|  from the EQFTMDEF.H file: GET_EXACT and GET_EXACT_AND_FUZZY.         |
|  This defines should be bitwise 'ored' with the number of required    |
|  matches.                                                             |
|  If only the number of required matches is passed, GET_EXACT is used. |
|  Example: pGetIn->usNumMatchesReq = 3 | GET_EXACT_AND_FUZZY           |
|  If GET_EXACT is specified :                                          |
|  As soon as the first exact match is found and is a human translation |
|  subsequent segments are checked only for exact match and previous    |
|  found similar matches are deleted from the match table.              |
|  If the first exact match found is a machine translation the previous |
|  found similar matches are not deleted and it is still searched for   |
|  similar matches.                                                     |
|  If an exact match is found and it is a machine translation the       |
|  similarity level is set to the minimum exact match level.            |
|  If a similar match is found and it is a machine translation the      |
|  similarity level is set to the minimum similarity level.             |
|  If GET_EXACT_AND_FUZZY is specified :                                |         |
|  The database is searched for fuzzy and exact matches. The fuzzy and  |
|  exact matches are returned in the match table.                       |
|  The array aMatchTable contains MATCH information about the best      |
|  matches found so far.                                                |
|  An array of pointers, MatchSort,points to the elements of aMatchTable|
|  in decreasing order. The aMatchTable is updated as more segments     |
|  from the cluster are checked.                                        |
|                                                                       |
|  Hierarchy of exact matches is : same filename (and closest segment   |
|  number) same industry codes (and youngest date), closest time tag.   |
|                                                                       |
|  The bufData part of GetOut buffer is used to keep at most            |
|  numMatchesReq elements of MATCH  and then the same number of         |
|  sections of size MAX_TGT_SIZE to hold the targets. Whenever a segment|
|  gets into the best matches array, it's target replaces the target    |
|  that belongs to the worst target, and the MatchSort array is         |
|  rearranged accordingly.                                              |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  -  OK (0)    - when command ends successfully                        |
|  -  other values - if Dos operation like read/write fails then the    |
|                                                                       |
|  Function calls :                                                     |
|  ----------------                                                     |
|  GetFirstSegAddress                                                   |
|  CalcSecondaryKey                                                     |
|  ReadSegFromDisk                                                      |
|  CheckSimilar                                                         |
|  CheckExact                                                           |
|  CalcExactIndustry                                                    |
|  RankNewMatch                                                         |
\*---------------------------------------------------------------------*/
USHORT
TmtGet (PTMT_GLOBALS     ptmtg,         /* pointer to Tmt globals....*/
        PGET_IN          pGetIn,        /* pointer to input buffer...*/
        PGET_OUT         pGetOut)       /* pointer to output buffer..*/
{
  /*-------------------------------------------------------------------*/
  USHORT        rc = OK;             /* Returned rc....................*/
  PSZ           pszSource = &(pGetIn->bufData[pGetIn->usDispSource]);
                                     /* Pointer to the source..........*/
  SZSECKEY      szInputKey;          /* Sec key of input segment.......*/
  TM_ADDRESS    addrCurrent,         /* Current segment's address......*/
                addrNext;            /* Next segment's address.........*/
  BOOL          fSimilarMod = TRUE,  /* Similar mode flag .............*/
                fFirstSeg = TRUE,    /* First segment flag.............*/
                fLastSeg = FALSE,    /* Last segment flag..............*/
                fExactAndFuzzy;
  LONG          lExact,              /* Exactness level..............,,*/
                lSimLevel;           /* Similarity level...............*/
  PMATCH        pmtchCurrent = &(ptmtg->mtch);
                                     /* Match structure................*/
  PSEGMENT      psegCurrent = &(ptmtg->seg);
                                     /* Segment buffer.................*/

  TOKENENTRY    aTokenEntry[2];      /* Point to first &second words ..*/
                                     /* from which the primary key was */
                                     /* built for the input source.....*/
  BOOL          fExactMTFound=FALSE; /* TRUE if exact machine..........*/
                                     /* translation was found..........*/
  USHORT        usIndex;             /* for loop index.................*/
  /*-------------------------------------------------------------------*/
  //--- get the type of search
  //--- if fExactAndFuzzy exact and fuzzy matches are returned.
  //--- if !fExactAndFuzzy only exact matches are returned.
  fExactAndFuzzy = pGetIn->usNumMatchesReq & GET_EXACT_AND_FUZZY;
  //--- get number of required matches - at most 255 matches allowed
  pGetIn->usNumMatchesReq &= 0x0FF;

  pGetOut->usNumMatchesFound = 0;
  /* Get address of first segment in cluster...........................*/
  GetFirstSegAddress (ptmtg, pszSource, &addrNext);
  /* Register the first two words from which the primary key for the ..*/
  /* input source was built ...........................................*/
  aTokenEntry[0] = *ptmtg->pteFirstSigWord ;
  aTokenEntry[1] = *ptmtg->pteSecondSigWord ;
  if (addrNext.usBlockNumber > 0)
  {
    /* The cluster is not empty, start searching for similar matches...*/
    /* Calculate the input's secondary key.............................*/
    CalcSecondaryKey (ptmtg, (PSZ)szInputKey);
    /* If secondary key of input   segment is empty we resort to
       CheckExact mode 26/3/91 ........................................*/
    fSimilarMod = (strlen((PSZ)szInputKey) > 0 );

    //--- call function to correct the threshold values in dependency
    //--- of the number of words of the input segment
    LengthCorrectThresholds( ptmtg, (USHORT)strlen((PSZ)szInputKey), pGetIn );

    /* Scan all the segments in the cluster............................*/
    while ((!fLastSeg) && (rc == OK))
    {
      addrCurrent = addrNext;

      /* Read current segment of cluster into the segment buffer.......*/
      /* AFTER the Read, addrNext will be the addr of NEXT segment.....*/
      rc = ReadSegmentFromDisk (ptmtg, &addrNext, psegCurrent,
                                       &fFirstSeg, &fLastSeg);
      if ((rc == OK) && !psegCurrent->fLogicalDel)
      {
        /* Current segment read OK and it is not deleted...............*/
        if ( fSimilarMod )
        {
          /* check if current source similar to input source...........*/
          lSimLevel = CheckSimilar (ptmtg, psegCurrent, aTokenEntry,
                                    pGetIn, (PSZ)szInputKey, pmtchCurrent);
          if (lSimLevel > 0L)   //--- segments are similar
          {
             if ( psegCurrent->usTranslationFlag != TRANSLFLAG_NORMAL ) //similar match is a machine translation
             {
                //--- set similarity  match level to minimum similarity level
                //--- added 1, because BASE_SIMILAR means it is not similar
                lSimLevel = BASE_SIMILAR + 1;
                pmtchCurrent->lSimLevel= lSimLevel;                    /* @1FA */
             }/*endif*/
            //check for exact match
            lExact = CheckExact(pGetIn, psegCurrent, fSimilarMod,
                                        pmtchCurrent);
            if ( lExact > 0 )  //--- exact match found
            {
              //--- Prepare for exact matching
              lSimLevel = lExact;
              if ( psegCurrent->usTranslationFlag != TRANSLFLAG_NORMAL) //excat match is a machine translation
              {
                 //--- remember that a machine translation is found
                 fExactMTFound = TRUE;
                 //--- set similarity  match level to minimum exact level
                 lSimLevel = MAX_SIMILAR_VAL;                         /* @1FC */
                 pmtchCurrent->lSimLevel= lSimLevel;                  /* @1FA */
              }
              else             //--- exact match is no machine translation
              {
                 //--- stop searching for similar matches only if not alt least
                 //--- one machine translation was found
                 if ( !fExactMTFound )
                 {
                   //--- from now on search only for exact matches
                   //--- in dependency of fExactAndFuzzy
                   if ( !fExactAndFuzzy )
                   {
                     fSimilarMod = FALSE;
                     //--- pGetOut->usNumMatchesFound = 0, so that eventually found
                     //--- similar matches are removed from match table
                     //--- This is done in function RankNewMatch
                     pGetOut->usNumMatchesFound = 0;
                   }/*endif*/
                 }/*endif*/
              }/*endif lExact*/
            }/*endif similar match*/
          }/*endif similar match*/
        }
        else //--- fExactMod
        {
          lSimLevel = CheckExact(pGetIn, psegCurrent, fSimilarMod,
                                         pmtchCurrent);
          //--- excat match is a machine translation and exact        
          if ( (psegCurrent->usTranslationFlag != TRANSLFLAG_NORMAL) && lSimLevel )              
          {                                                           
             //--- set similarity  match level to minimum exact level
             lSimLevel = MAX_SIMILAR_VAL;
             pmtchCurrent->lSimLevel= lSimLevel;
           } /* endif */                                           
        } /* endif (fSimilarMod) */

        if (lSimLevel > 0L)
        {
          /* We have found a match (similar or exact)..................*/
          pmtchCurrent->addr = addrCurrent;
          pmtchCurrent->usTranslationFlag = psegCurrent->usTranslationFlag;
          pmtchCurrent->tStamp = psegCurrent->tStamp;

          (pGetOut->usNumMatchesFound)++;
          RankNewMatch (pGetIn->usNumMatchesReq,
                        pGetIn->usConvert,
                        pmtchCurrent, psegCurrent, pGetOut);
        } /* endif (fSimLevel) */
      } /* endif ReadSegment was OK && !fLogical delete*/
    } /* end while */
  } /* endif cluster not empty */

  pGetOut->usNumMatchesValid = min(pGetOut->usNumMatchesFound,
                                   pGetIn->usNumMatchesReq);

  /**************************************************************/    /*@1263A*/
  /* if an exact machine translation was found and ony exact    */    /*@1263A*/
  /* matches should be returned scan the match table and delete */    /*@1263A*/
  /* fuzzy machine translation matches                          */    /*@1263A*/
  /**************************************************************/    /*@1263A*/
  if ( fExactMTFound && !fExactAndFuzzy )                             /*@1263A*/
  {                                                                   /*@1263A*/
    for ( usIndex = 0; usIndex < pGetIn->usNumMatchesReq; usIndex++ ) /*@1263A*/
    {                                                                 /*@1263A*/
      if ( (pGetOut->amtchBest[pGetOut->ausSortedMatches[usIndex]].usTranslationFlag != TRANSLFLAG_NORMAL) /*@1263A*/
           && pGetOut->amtchBest[pGetOut->ausSortedMatches[usIndex]].lSimLevel  /*@1263A*/
           < MAX_SIMILAR_VAL )                                        /*@1263A*/
      {                                                               /*@1263A*/
        memset( &pGetOut->amtchBest[pGetOut->ausSortedMatches[usIndex]], 0, /*@1263A*/
                sizeof(MATCH) );                                      /*@1263A*/
        (pGetOut->usNumMatchesValid)--;                               /*@1263A*/
      } /* endif */                                                   /*@1263A*/
    } /* endfor */                                                    /*@1263A*/
  } /* endif */                                                       /*@1263A*/

  pGetOut->prefout.usLenOut = (sizeof (GET_OUT));

  // Set return code to OK if CLUSTER_EMPTY is returned
  if (rc == CLUSTER_EMPTY)
  {
    rc = OK;
  } /* endif */

  return rc;
} /* end of TmtGet */

/*---------------------------------------------------------------------*\
|                       Tmt Extract                                     |
+-----------------------------------------------------------------------+
|  Function name      : TmtExtract                                      |
|  Description        : Extracts a segment whose address is given and   |
|                       returns the address of the next segment.        |
|  Function Prototype : TmtExtract (pTmtGlobals, pExtIn, pExtOut)       |
+-----------------------------------------------------------------------+
|  Implementation  Remarks                                              |
|  -------------------------                                            |
|  Extract is given an address of a TM segment and returns that segment |
|  and the address of next segment.                                     |
|                                                                       |
|  Extract finds the first non-deleted segment starting from the input  |
|  address by calling GetSegByAddr. The found segment is written into   |
|  ExtOut.  The address of the next segment is taken from GetSegByAddr. |
|                                                                       |
|  If the input address is {0,0,0}, it is interpreted as the address    |
|  of the first segment in the TM.                                      |
|                                                                       |
|  Note: It is valid to provide an input address of a logically deleted |
|  segment. In this case the returned segment is the first non-deleted  |
|  segment.                                                             |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  -  OK (0)    - when command ends successfully                        |
|  -  NO_SEG_FOUND - no segments in the TM starting from the input addr.|
|  -  other values - if Dos operation like read/write fails.            |
|                                                                       |
|  Function calls :                                                     |
|  ----------------                                                     |
|  GetSegByAddr                                                         |
\*---------------------------------------------------------------------*/
USHORT
TmtExtract (PTMT_GLOBALS    ptmtg,       /* pointer to Tmt globals.....*/
            PEXT_IN         pExtIn,      /* pointer to input buffer....*/
            PEXT_OUT        pExtOut)     /* pointer to output buffer...*/
{
  /*-------------------------------------------------------------------*/
  USHORT         rc;                 /* returned rc....................*/
  TM_ADDRESS     addr;               /* pointer to input address.......*/
  BOOL           fConversionMode;    // CRLF/LF conversion flag
  PSEGMENT       pWorkSeg;           // Pointer to work segment in the tmt globals
  PSEGMENT       pSegOut;            // Pointer to output segment
  /*-------------------------------------------------------------------*/
  addr = pExtIn->addr;

  // Initialize the segment length to 0
  pExtOut->segOut.usLenSegment = 0;

  if ( !addr.usBlockNumber && !addr.usDispBlockPtr)
  {
    /* Address is {Cluster N, 0, 0} First segment in cluster N required*/
    addr.usBlockNumber = ptmtg->pTmHeader->
                         ausKeyDirectory[addr.usEntryInDir];
    addr.usDispBlockPtr = BLOCK_HEADER_SIZE;
  }

  /* GetSegByAddr changes addr (input) to point to first non-deleted...*/
  /* segment.  NextAddr contains the address of the next segment.......*/
  /* That segment may be a deleted segment.............................*/

  rc = GetSegByAddr (ptmtg, &addr, &pExtOut->addrNext, &pExtOut->segOut);

  if (rc == OK )
  {
    /* return the actual address from which the segment has been read  */
    pExtOut->addr = addr;

    // Convert the source and the target of the segment as required
    if ( pExtIn->usConvert != MEM_OUTPUT_ASIS )
    {
      // Set conversion mode flag
      if ( pExtIn->usConvert == MEM_OUTPUT_CRLF )
      {
        fConversionMode = FALSE;
      }
      else
      {
        fConversionMode = TRUE;
      } /* endif */

      // Copy the entire segment to the workarea pWorkArea1 in the TMT Globals
      pWorkSeg = (PSEGMENT)ptmtg->pWorkArea1;
      pSegOut  = &pExtOut->segOut;
      memcpy( pWorkSeg, pSegOut, pSegOut->usLenSegment );

      // Convert the source segment
      pSegOut->usLenSource = usConvertCRLF(
                        &pWorkSeg->bufData[pWorkSeg->usDispSource], // input area
                        pWorkSeg->usLenSource,                      // length of input area
                        &pSegOut->bufData[pSegOut->usDispSource],   // output area
                        fConversionMode );                          // conversion mode

      // Convert the target segment
      pSegOut->usDispTarget = pSegOut->usDispSource + pSegOut->usLenSource;
      pSegOut->usLenTarget = usConvertCRLF(
                        &pWorkSeg->bufData[pWorkSeg->usDispTarget], // input area
                        pWorkSeg->usLenTarget,                      // length of input area
                        &pSegOut->bufData[pSegOut->usDispTarget],   // output area
                        fConversionMode );                          // conversion mode

      // Move "reserved bytes"
      pSegOut->usDispReserved = pSegOut->usDispTarget + pSegOut->usLenTarget;
      memcpy( &pSegOut->bufData[pSegOut->usDispReserved],
                 &pWorkSeg->bufData[pWorkSeg->usDispReserved],
                 pSegOut->usLenReserved );

      // Move "secondary key"
      pSegOut->usDispSecKey = pSegOut->usDispReserved + pSegOut->usLenReserved;
      memcpy( &pSegOut->bufData[pSegOut->usDispSecKey],
                 &pWorkSeg->bufData[pWorkSeg->usDispSecKey],
                 pSegOut->usLenSecKey );

      // Calculate new segment length
      pSegOut->usLenSegment = (sizeof( SEGMENT ) - sizeof( BUFFERIN )) +
                              pSegOut->usLenIndustry +
                              pSegOut->usLenSource +
                              pSegOut->usLenTarget +
                              pSegOut->usLenReserved +
                              pSegOut->usLenSecKey;
    } /* endif ( pExtIn->usConvert != MEM_OUTPUT_ASIS ) */
  } /* endif rc == 0 */
  // Calculate output segment length
  pExtOut->prefout.usLenOut = (sizeof( EXT_OUT ) - sizeof( SEGMENT )) +
                              pExtOut->segOut.usLenSegment;
  return rc;
}  /* end of TmtExtract               */

/*---------------------------------------------------------------------*\
|                       Tmt get part                                    |
+-----------------------------------------------------------------------+
|  Function name      : TmtGetTMPart                                    |
|  Description        : Get part of a TM                                |
|                                                                       |
|  Function Prototype : TmtGetTMPart( ptmtg, szMemPath, pGetPartIn,     |
|                                     pGetPartOut, usMsgHandling )      |
+-----------------------------------------------------------------------+
|  Implementation  Remarks                                              |
|  -------------------------                                            |
|  This function reads a block of data from the TM file. The start      |
|  position and the block size is passed in the input structure.        |
|  The read block is copied to the output stucture and the pointer      |
|  is set to the next position to be read.                              |
|  Before calling this function the TM has to opened with               |
|  usMode EXCLUSIVE_FOR_GET_PART                                        |
|  If the number of bytes to read (set in input structure               |
|  pGetPartIn->ulBytesToRead) is higher than the defined maximum        |
|  GETPART_BUFFER_SIZE, then pGetPartIn->ulBytesToRead is set to        |
|  GETPART_BUFFER_SIZE.                                                 |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  -  NO_ERROR in case of no error                                      |
|  -  other values: return codes from Dos/Utl functions                 |
|                                                                       |
\*---------------------------------------------------------------------*/
USHORT TmtGetTMPart( PTMT_GLOBALS  ptmtg,
                     PGETPART_IN   pGetPartIn,
                     PGETPART_OUT  pGetPartOut )
{
   USHORT   usRc = NO_ERROR;                //return code from DosRead
   ULONG    ulNewPointer;                   //new pointer location

   //--- set the file pointer to the value passed in pGetPartIn->ulFilePos
   usRc = UtlChgFilePtr( ptmtg->hfTM, pGetPartIn->ulFilePos, FILE_BEGIN,
                         &ulNewPointer, FALSE );

   //--- if requested bytes to read are higher than the defined maximum
   if ( pGetPartIn->ulBytesToRead > GETPART_BUFFER_SIZE )
   {
      //--- set bytes to read to defined maximum
      pGetPartIn->ulBytesToRead = GETPART_BUFFER_SIZE;
   }/*endif*/

   if ( !usRc )                             //no error until now
   {
      usRc = UtlRead( (HFILE)ptmtg->hfTM, pGetPartOut->aucOutBuffer,
                      (USHORT)pGetPartIn->ulBytesToRead,
                      (PUSHORT)&pGetPartOut->ulBytesRead, FALSE );
   }/*endif*/

   if ( !usRc )          //no error until now
   {
      //--- set new file position
      pGetPartOut->ulNextFilePos = ulNewPointer + pGetPartOut->ulBytesRead;
   }/*endif*/

   //--- set length of output structure
   pGetPartOut->prefout.usLenOut = sizeof( GETPART_OUT );

   return ( usRc );
}/* end TmtGetTMPart */

/*---------------------------------------------------------------------*\
|                       Check Similar                                   |
+-----------------------------------------------------------------------+
|  Function name      : CheckSimilar                                    |
|  Description        : Checks if input source is similar to current    |
|                       segment.                                        |
|  Function Prototype : CheckSimilar (ptmtg, pCurrentSegment, pTokenEntr|
|                                     y, pGetIn, pszInputKey, pmtch)    |
+-----------------------------------------------------------------------+
|  Implementation  Remarks                                              |
|  -------------------------                                            |
|  This function checks if two segments are similar.                    |
|  It returns a LONG number: 0 indicates that segments are not similar. |
|  When the returned value is greater then 0 it is the degree           |
|  of the similarity.                                                   |
|  The check includes  3  tests:                                        |
|    The first one checks that both segments have more or less the      |
|    same number of initials.                                           |
|                                                                       |
|    The second test is performed only if the first check is positive,  |
|    and it checks if most of the initials are the same.                |
|                                                                       |
|    The 3'rd test ensures that if the first 2 tests have passed, the   |
|    first and second significant word (according to the primary key)   |
|    from each segment are equal or one is the prefix of the other.     |
|    For comparison the length of the shorter word is used. If the      |
|    length is <=5 the the length is reduced by 1, in all other         |
|    cases the length is reduced by 2.                                  |
\*---------------------------------------------------------------------*/
LONG
CheckSimilar (PTMT_GLOBALS ptmtg,          /* pointer to Tmt globals...*/
              PSEGMENT     pCurrentSegment,/* pointer to current segmnt*/
              PTOKENENTRY  pTokenEntry,    /* points to 1'st & 2'nd word*/
              PGET_IN      pGetIn,         /* pointer to input buffer...*/
              PSZ          pszInputKey,    /* pointer to sortedkey of...*/
                                           /* input segment.............*/
              PMATCH       pmtch)         /* pointer to stMatch........*/
{
  /*-------------------------------------------------------------------*/
  LONG     lSimilar = 0L,                 /* Returned value............*/
           lLengthValue,                  /* Value of length test......*/
           lMatchValue;                   /* Value of initials test....*/

  PSZ      pszCurrentSource = &(pCurrentSegment->bufData[
                                pCurrentSegment->usDispSource]),
           pszCurrentKey = &(pCurrentSegment->bufData[pCurrentSegment->
                                                    usDispSecKey]);
  USHORT   usLenInput = (USHORT)strlen(pszInputKey),
                                          /* Length of input sec.key...*/
           usLenCurrent = (USHORT)strlen(pszCurrentKey),
                                          /* Length of current sec.key.*/
           usSameInitilas = 0;            /* number of same initials...*/
  ACHPRIMKEY    achPrimaryKey;              /*Primary key of the source*/

  /*-------------------------------------------------------------------*/
  //--- get rid of compiler warning
  pGetIn = pGetIn;
  /* Check that the input and current key are not NULL.................*/
  if ((usLenInput > 0) && (usLenCurrent > 0))
  {
    // lLengthValue = (EXTENT_SIMILAR * abs(usLenInput - usLenCurrent)) /
    //               (usLenInput + usLenCurrent);
    //--- formula changed, to be equal to match threshold formula
    lLengthValue = ( EXTENT_SIMILAR * min( usLenInput, usLenCurrent ) ) /
                     max( usLenInput, usLenCurrent );

    if ( lLengthValue >= ptmtg->lActLengthThr )
    {
      //--- The lengths of input and target segment from TM are close enough.
      //--- ptmtg->lActLengthThr is calculated in the function
      //--- LengthCorrectThresholds in dependency of the number of words in the
      //--- input segment.

      //--- Calculate the number of identical characters in both secondary keys.
      //--- The calculation is in a MergeSort-like fashion
      while ((*pszInputKey != NULC) && (*pszCurrentKey != NULC))
      {
        /* if a match was found increment counter and both pointers....*/
        if (*pszInputKey == *pszCurrentKey)
        {
          usSameInitilas++;
          pszInputKey++;
          pszCurrentKey++;
        }
        else /* increment pointer to key with the low value character*/
        {
          if (*pszInputKey > *pszCurrentKey)
            pszCurrentKey++;
          else
            pszInputKey++;
        }/* endif (*pszInputKey == *pszCurrentKey) */
      }/* endwhile */

      //--- calculate the match value of both segements
      lMatchValue = (EXTENT_SIMILAR * usSameInitilas) /
                                           max(usLenInput, usLenCurrent);

      if ( lMatchValue >= ptmtg->lActMatchThr)
      {
        //--- the match above calculated match value must be higher then
        //--- the length corrected threshold (which is calculated in the
        //--- function LengthCorrectThresholds).

        /* Find first two words from current segment primary key ......*/
        /* Call CleanSource to form TextTable and WordsTable...........*/
        CleanSource(ptmtg, pszCurrentSource);

        /* Update pointers to first & second significant words ........*/
        CalcPrimaryKey(ptmtg, achPrimaryKey);

        /* Check if one word is containd within the second word .......*/
        if ( CheckWord(  pTokenEntry, ptmtg->pteFirstSigWord  ) &&
             CheckWord(++pTokenEntry, ptmtg->pteSecondSigWord )   )
        {
          lSimilar = lMatchValue;
          pmtch->lLengthTest    = lLengthValue;
          pmtch->lSimLevel      = lSimilar;
          pmtch->lInitMatchTest = lMatchValue;
        }/* endif (CheckWord(...) ) ...................................*/
      }/* endif lMatchValue */
    } /* endif lLengthValue */
  } /* endif usLenInput */

  /* this section has been removed due to change in the policy of
     CheckSimilar. Since noise & useless words are also considered as
     part of the secondary key, empty keys imply that the segment .
     consists of just tags & noise characters Therefore if the
     secondary is empty we resort to CheckExact ...................*/
#if 0
  /* The case of empty secondary keys .............................*/
  if ((usLenInput == 0) && (usLenCurrent == 0))
  {
    /* Find first two words from current segment primary key ......*/
    /* Call CleanSource to form TextTable and WordsTable...........*/
    CleanSource(ptmtg, pszCurrentSource);

    /* Update pointers to first & second significant words ........*/
    CalcPrimaryKey(ptmtg, achPrimaryKey);

    /* Check if one word is containd within the second word .......*/
    if ( CheckWord(pTokenEntry, ptmtg->pteFirstSigWord) )
    {
      lSimilar = pGetIn->lInitMatchThr;
      pmtch->lLengthTest    = pGetIn->lLengthThr;
      pmtch->lSimLevel      = lSimilar;
      pmtch->lInitMatchTest = lSimilar;
    }/* endif (CheckWord(...) ) ...................................*/
  }/* endif ((usLenInput == 0) && (usLenCurrent == 0)) ............*/
#endif

  return (lSimilar);
} /* end of CheckSimilar */

/*---------------------------------------------------------------------*\
|                       Check Exact                                     |
+-----------------------------------------------------------------------+
|                                                                       |
|  Function name      : CheckExact                                      |
|  Description        : Checks if input source and current segment      |
|                       are exact matches .                             |
|  Function Prototype : CheckExact (pGetIn, psegCurrent, fSimilarMod,   |
|                                   pmtch)                              |
+-----------------------------------------------------------------------+
|  Implementation  Remarks                                              |
|  -------------------------                                            |
|  This function checks if two source segments match exactly, byte by   |
|  byte by ignoring CRLF and LF.                                        |
|  If the mode of work is similar (fSimilarMod = TRUE), then the        |
|  number of equal bytes in both segments is calculated. If all bytes   |
|  are equal it means that exact match has been found.                  |
|                                                                       |
|  For those sources that match exactly, a hierarchy determines which   |
|  is a better match as follows:                                        |
|  Same filename is the best match.  Matches having the same file name  |
|  are ranked further more by the distance of the seg#. The degree of   |
|  match is 400 or more.  (Used to be 4, but due to slow mode of        |
|  floating point calculation, a factor of 100 is introduced to enable  |
|  integer calculation.                                                 |
|  Second best match is if industry codes match. Within those matches   |
|  with the same ind. codes, the date is used further more to determine |
|  the rank. The degree of match is 300 or more.                        |
|                                                                       |
|  If none of the above matches, then the time stamp is used to assign  |
|  the degree of similarity. The degree is 200 or more.                 |
|  The third parameter of the function is the similarity degree         |
|  calculated previously by CheckSimilar. If this degree is less than   |
|  100, then the exact match fails immediately.                         |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  CalcExactIndustry                                                    |
\*---------------------------------------------------------------------*/
LONG
CheckExact (PGET_IN      pGetIn,           /* pointer to input segment.*/
            PSEGMENT     psegCurrent,      /* pointer to current seg...*/
            BOOL         fSimilarMod,      /* Similarity flag..........*/
            PMATCH       pmtch)            /* pointer to stMatch.......*/
{
  /*-------------------------------------------------------------------*/
  LONG      lExact = 0;                   /* returned value............*/
  USHORT    usLenEqual,                   // number of bytes equal
            usSegNumInput = pGetIn->usSegNumber,
                                          /* seg number of input seg...*/
            usSegNumCurrent = psegCurrent->usSegNumber,
                                          /* seg number of current seg.*/
            usMonthDiff;                  /* num of month between......*/
                                          /* current date & date of....*/
                                          /* current seg...............*/
  BOOL      fMatchExact;                         // Flag which indicates exact match
  /*-------------------------------------------------------------------*/

  // Check if the current source equals to the input source
  // by ignoring CRLF and LF

  fMatchExact = (UtlCompIgnWhiteSpace(
                             &pGetIn->bufData[pGetIn->usDispSource],
                             &psegCurrent->bufData[psegCurrent->usDispSource],
                             0) == 0);

  if (fSimilarMod || fMatchExact)
  {
    if ( fMatchExact )
    {
      // The sources are identical
      // Decrease the length by 1 because of the \0 at the end
//      pmtch->usNumExactBytes = usLenTmp - 1;
      pmtch->usNumExactBytes = 0;   //not used any more!?

      if ( NTMDocMatch( pGetIn->szFileName, pGetIn->szLongName,
                        psegCurrent->szFileName, psegCurrent->szLongName ) )
      {
        /* The source file names are the same --> lExact >= BASE_EXACT.*/
        lExact = MAX_EXACT_SEG_NUM_VAL -
                 (EXTENT_EXACT_SEG_NUM *
                    abs( (usSegNumInput - usSegNumCurrent))) /
                       (1+ max(usSegNumInput, usSegNumCurrent) );
      }
      else
      {
        /* filenames are not the same, check industry codes............*/
        lExact = CalcExactIndustry(pGetIn, psegCurrent);
        if (lExact == 0) {
          /* Calculate number of normalized months difference between..*/
          /* the time stamps of the Input and current segment..........*/
          /* it must not be more then EXTENT_EXACT_DATE months.........*/
          /*?????decide what to do if the date specified in the tStamp?*/
          /*?????field in the pGetIn is earlier the the segment's??????*/
          usMonthDiff = (USHORT) ((time(NULL) - psegCurrent->tStamp)/2592000L);
          if (usMonthDiff > EXTENT_EXACT_DATE)
            usMonthDiff = EXTENT_EXACT_DATE;
          lExact = MAX_EXACT_DATE_VAL - usMonthDiff;
        } /* endif (lExact == 0) */
      }/* endif (* filename is the same *) ............................*/
      /* Update lSimLevel in stMatch...................................*/
      pmtch->lSimLevel = lExact;
    }
    else
    {
      // The sources are NOT identical
      usLenEqual = 0;
      pmtch->usNumExactBytes = usLenEqual;
    } /* endif ( fMatchExact ) */
  }
  return (lExact);
} /* end of CheckExact */

/*---------------------------------------------------------------------*\
|                       Calc Exact Industry                             |
+-----------------------------------------------------------------------+
|  Function name      : CalcExactIndustry                               |
|  Description        : Calculates the degree of exact match when some  |
|                       or all industry codes match.                    |
|  Function Prototype : CalcExactIndustry (pGetIn,pchSegment)           |
+-----------------------------------------------------------------------+
|  Implementation  Remarks                                              |
|  -------------------------                                            |
|  This function calculates the degree of the match for segments that   |
|  at least one industry code is the same.                              |
|  It returns a value between 300 and 400.                              |
\*---------------------------------------------------------------------*/
LONG
CalcExactIndustry (PGET_IN  pGetIn,       /* pointer to input segment..*/
                   PSEGMENT psegCurrent)   /* pointer to current seg...*/
{
  /*-------------------------------------------------------------------*/
  LONG      lExact = 0L,                  /* returned value............*/
            lIndDiff;                     /* normalized diffrence of...*/
                                          /* length of keys............*/
  PUSHORT   pusInputIndustry = (PUSHORT) &pGetIn->bufData[pGetIn->
                                                         usDispIndustry],
                                          /*pointer to Sec key of input*/
            pusCurrentIndustry = (PUSHORT)
                     &psegCurrent->bufData[psegCurrent->usDispIndustry];
                                          /* pointer to Sec key of.....*/
                                          /* current segment...........*/
  USHORT
            usLenInputIndustry = pGetIn->usLenIndustry / sizeof(USHORT),
                                          /* Length of input ind code..*/
            usLenCurrentIndustry = psegCurrent->usLenIndustry /
                                   sizeof(USHORT) ,
                                          /* length of Current ind code*/
            usNumMatchingCodes = 0,       /* no. of matching ind codes.*/
            usIndex,
            usIndex2;
  /*-------------------------------------------------------------------*/
  /* Count the number of matching industry codes.......................*/


  for (usIndex=0; usIndex < usLenCurrentIndustry; usIndex++)
  {
    for (usIndex2=0; ( usIndex2 < usLenInputIndustry ) &&
                      !( pusInputIndustry[usIndex2] ==
                         pusCurrentIndustry[usIndex] ); usIndex2++);
    if ( pusInputIndustry[usIndex2] == pusCurrentIndustry[usIndex] )
    {
      usNumMatchingCodes++;
    }
  }
  if (usNumMatchingCodes > 0)
  {
    /* There is at least one matching industry code, continue..........*/

    lIndDiff =  (EXTENT_EXACT_IND * usNumMatchingCodes) /
                max (usLenInputIndustry, usLenCurrentIndustry);
    lExact = BASE_EXACT_IND + lIndDiff;
  }
  return lExact ;
} /* end of CalcExactIndustry */

/*---------------------------------------------------------------------*\
|                       Rank New Match                                  |
+-----------------------------------------------------------------------+
|  Function name      : RankNewMatch                                    |
|  Description        : Adds a new match to aMatchTable.                |
|  Function Prototype : RankNewMatch(usNumMatchesReq, pmtch, psegCurrent|
|                                     pGetOut)                          |
+-----------------------------------------------------------------------+
|  Implementation  Remarks                                              |
|  -------------------------                                            |
|  This function adds to the array aMatchTable the data of the current  |
|  This current segment has passed the tests and its MATCH              |
|  data in CurrentMatch is inserted into the array.                     |
|  If the number of elements in this                                    |
|  array is already numMatchesReq,     then its degree of similarity    |
|  is checked against the degree of the other elements. If its degree   |
|  is higher, then it replaces the worst match found so far. Otherwise  |
|  the array is unchanged. In case that the number of elements is still |
|  smaller than numMatchesReq, this segment is inserted                 |
|  automatically.                                                       |
|  In order to keep the elements of aMatchTable in decreasing order,    |
|  another array of indices, aiMatchOrder is used. Actually, only       |
|  the elements of this array are updated when the current segment is   |
|  inserted to the array of aMatchTable.                                |
|  The last four parameters of this function are needed to fill in the  |
|  fields of new element of aMatchTable.                                |
\*---------------------------------------------------------------------*/
VOID
RankNewMatch (USHORT      usNumMatchesReq,/* number of matches required*/
              USHORT      usConvert,      // Convert indicator
              PMATCH      pmtch,          /* pointer to stMatch........*/
              PSEGMENT    psegCurrent,    /* pointer to current seg....*/
              PGET_OUT    pGetOut) {      /* pointer to GET_OUT struct.*/
  /*-------------------------------------------------------------------*/
//  USHORT     usIndexMatch = pGetOut->usNumMatchesFound - 1,
                                          /* Index of new match........*/
  USHORT     usIndexOfWorst;              /* index of worst match......*/
  USHORT     usIndex;                     /* Index variable ...........*/
  USHORT     usNewEntry;                  /* where to insert new item..*/
  USHORT     usNewLength;                 /* New target length after conversion */
  BOOL       fConversionMode;             /* Conversion mode flag */
  PUSHORT    ausSortedMatches = pGetOut->ausSortedMatches;
  PMATCH     amtchBest        = pGetOut->amtchBest;
  USHORT     usStart;
  USHORT     usDel;
  /*-------------------------------------------------------------------*/
  if ( usNumMatchesReq == MAX_MATCH_TAB_ENTRIES )
  {
    usNumMatchesReq--;
  } /* endif */

  if (pGetOut->usNumMatchesFound == 1)
  {
    /* Initialize......................................................*/
    for (usIndex = 0; usIndex <= usNumMatchesReq; usIndex++)
    {
      *(ausSortedMatches++) = usIndex;
                                                                      /*0009D*/
      //--- initialize the complete match structure so that no        /*0009A*/
      //--- previous matches are used any longer                      /*0009A*/
      memset( (amtchBest++), 0, sizeof(MATCH) );                      /*0009A*/
    }
    ausSortedMatches = pGetOut->ausSortedMatches;
    amtchBest = pGetOut->amtchBest;
  }

  usIndexOfWorst = ausSortedMatches[usNumMatchesReq];

  //--- Check if found match better or equal worst then worst match  /* @8AC */
  if (pmtch->lSimLevel >= amtchBest[usIndexOfWorst].lSimLevel)       /* @8AC */
  {
    // Found match is better then worst match. Search the first match
    // which is not as good as the found match and update ausSortedMatches
    for (usIndex = 0; usIndex <= usNumMatchesReq &&                     /*@1262C*/
                      amtchBest[ausSortedMatches[usIndex]].lSimLevel >= /*@1262C*/
                      pmtch->lSimLevel;                                 /*@1262C*/
                      usIndex++)                                        /*@1262C*/
    {                                                                /* @8AA */
      //--- rank younger match higher if sim levels are equal        /* @8AA */
      if ( amtchBest[ausSortedMatches[usIndex]].lSimLevel == pmtch->lSimLevel ) /* @8AA */
      {                                                              /* @8AA */
        if ( amtchBest[ausSortedMatches[usIndex]].tStamp < pmtch->tStamp ) /* @8AA */
        {                                                            /* @8AA */
          break;                                                     /* @8AA */
        } /* endif */                                                /* @8AA */
      } /* endif */                                                  /* @8AA */
    } /* endfor */                                                   /* @8AA */

    if ( usIndex <= usNumMatchesReq )                                /*@1262A*/
    {                                                                /*@1262A*/
      /* move elements in ausSortedMatches upward and insert new entry...*/
      usNewEntry = usIndex ;
      for (usIndex = usNumMatchesReq; usIndex > usNewEntry; --usIndex)
      {
        ausSortedMatches[usIndex] = ausSortedMatches[usIndex - 1];
      }
      /* The new match is better then the worst so far...................*/
      amtchBest[usIndexOfWorst] = *pmtch ;

      // Copy/convert the target to the output array
      if ( usConvert == MEM_OUTPUT_ASIS )
      {
        // Copy without conversion
        memcpy(amtchBest[usIndexOfWorst].szTarget,
                  &psegCurrent->bufData[psegCurrent->usDispTarget],
                  psegCurrent->usLenTarget);
      }
      else
      {
        // Set conversion mode flag
        if ( usConvert == MEM_OUTPUT_CRLF )
        {
          fConversionMode = FALSE;
        }
        else
        {
          fConversionMode = TRUE;
        } /* endif */

        // Copy and convert. usMewLength is set for test purposes only.
        usNewLength = usConvertCRLF(
          &psegCurrent->bufData[psegCurrent->usDispTarget], // pointer to character input area
          psegCurrent->usLenTarget,                         // length of input area
          (PSZ)(amtchBest[usIndexOfWorst].szTarget),        // pointer to output area
          fConversionMode );                                // conversion mode
      } /* endif */

      ausSortedMatches[usNewEntry] = usIndexOfWorst;

      /**********************************************************************/
      /* check if we inserted matches with same translation by accident    */
      /**********************************************************************/
      for ( usIndex = 0; usIndex <= usNumMatchesReq; usIndex++ )
      {
        /********************************************************************/
        /* only check against the existing ones                             */
        /********************************************************************/
        if ( usIndex != usNewEntry )
        {
          /******************************************************************/
          /* Check if targets are the same ( blanks and CRLF are ignored ), *//*@1163C*/
          /* but do this only if one of the two targets is not empty.       *//*@1163A*/
          /* This is done because fStrcmpCRLF returnes TRUE if one of the   *//*@1163A*/
          /* target is empty and the other one contains only CRLF and blanks*//*@1163A*/
          /******************************************************************/
          if ( amtchBest[ausSortedMatches[usIndex]].szTarget[0] != EOS && /*@1163A*/
               amtchBest[usIndexOfWorst].szTarget[0] != EOS )             /*@1163A*/
          {                                                               /*@1163A*/
            if (UtlCompIgnWhiteSpace(
                         (PSZ)(amtchBest[ausSortedMatches[usIndex]].szTarget),
                         (PSZ)(amtchBest[usIndexOfWorst].szTarget), 0) == 0 )
            {
              /****************************************************************/
              /* they are the same, we should only use the better one         */
              /****************************************************************/
              usStart = max ( usIndex, usNewEntry );
              usDel = ausSortedMatches[usStart];
              for ( usIndex = usStart; usIndex < usNumMatchesReq; usIndex++ )
              {
                ausSortedMatches[usIndex] = ausSortedMatches[usIndex+1];
              } /* endfor */
              ausSortedMatches[usIndex] = usDel;
              memset( &amtchBest[usDel], 0, sizeof(MATCH) );
              /****************************************************************/
              /* adjust nukmber of matches, i.e.the new match dis not increase*/
              /* number of matches                                            */
              /* since we have already increased the number, we have to       */
              /* decrease it now                                              */
              /****************************************************************/
              pGetOut->usNumMatchesFound--;
              /****************************************************************/
              /* end the outer for loop                                       */
              /****************************************************************/
              usIndex++;
            } /* endif */
          } /* endif */                                                /*@1163A*/
        } /* endif */
      } /* endfor */
    } /* endif */                                                    /*@1262A*/
  } /* endif there is a worst match */
} /* end of RankNewMatch */

/*---------------------------------------------------------------------*\
|                       Get Seg By Addr                                 |
+-----------------------------------------------------------------------+
|  Function name      : GetSegByAddr                                    |
|  Description        : Finds the first non-deleted segment starting    |
|                       from input addr and on.                         |
|  Function Prototype : GetSegByAddr (ptmtg, paddrIn, paddrNext, pseg)  |
+-----------------------------------------------------------------------+
|  Implementation  Remarks                                              |
|  -------------------------                                            |
|  This function is given an input address in pAddr. It scans the TM    |
|  from this address and on until a non-deleted segment is found. Then  |
|  it returns the address of this segment in pAddr, and the address     |
|  right next to it in pNextAddr.                                       |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  -  OK (0)    - when command ends successfully                        |
|  -  NO_SEG_FOUND - no segments in the TM starting from the input addr |
|                    (because coming clusters are empty on have only    |
|                     logically deleted segments.)                      |
|  -  other values - if Dos operation like read/write fails .           |
|                                                                       |
|  Function calls :                                                     |
|  ----------------                                                     |
|  ReadSegmentFromDisk                                                  |
\*---------------------------------------------------------------------*/
USHORT
GetSegByAddr (PTMT_GLOBALS  ptmtg,        /* pointer to TmtGlobals.....*/
              PTM_ADDRESS   paddrIn,      /* pointer to input address..*/
              PTM_ADDRESS   paddrNext,    /* pointer to next address...*/
              PSEGMENT      pseg) {       /* pointer to segment........*/
  /*-------------------------------------------------------------------*/
   USHORT     rc = OK;                /* returned rc...................*/
   BOOL       fLastSeg,               /* Last seg in cluster flag......*/
              fLogicalDel = TRUE,     /* Logical deleted seg flag......*/
              fFirstCall = TRUE;      /* First call to ReadSegF... ....*/
   USHORT     usCurrentKey = paddrIn->usEntryInDir;
                                      /* Loop counter..................*/
   TM_ADDRESS  addr;                  /* temporary address.............*/
  /*-------------------------------------------------------------------*/
  /* scan all clusters until a non-deleted segment is found............*/
  memset(&addr, 0, sizeof(addr));
  while ((rc == OK) && fLogicalDel &&
         (usCurrentKey < KEY_DIR_ENTRIES_NUM))
         {
    addr = *paddrIn;
    rc = ReadSegmentFromDisk(ptmtg, paddrIn, pseg,
                             &fFirstCall, &fLastSeg);
    if (rc == OK) fLogicalDel = pseg->fLogicalDel;

    /* fLastSeg may be set in ReadSegmentFromDisk......................*/
    if ((rc == CLUSTER_EMPTY) || ((rc == 0) && fLastSeg && fLogicalDel))
    {
      /* no read problem and last segment in cluster reached...........*/
      /* compute address of the first segment in next cluster..........*/
      /* reset rc if cluster empty.....................................*/
      rc = OK ;
      while ((++usCurrentKey <  KEY_DIR_ENTRIES_NUM) &&
             (ptmtg->pTmHeader->ausKeyDirectory[usCurrentKey] == 0));

      if (usCurrentKey <  KEY_DIR_ENTRIES_NUM)
      {
        /* A non epmty cluster was found...............................*/
        paddrIn->usEntryInDir  = usCurrentKey;
        paddrIn->usBlockNumber = ptmtg->pTmHeader->
                                 ausKeyDirectory[usCurrentKey];
        paddrIn->usDispBlockPtr  = BLOCK_HEADER_SIZE;
        /* set fFirstCall TRUE to ensure that next call to.............*/
        /* ReadSegFromDisk will read a new block to BlockImage.........*/
        fFirstCall = TRUE;
      }
      else
      {
        /* next address not exist, the rest of the clusters are empty..*/
        rc = NO_SEG_FOUND;
      } /* endif non empty cluster was found */
    } /* endif Last segment in cluster */
  } /* end while */

  if (rc == OK)
  {
    /* no read problem, if fLogical delete then all segments were......*/
    /* logically deleted, otherwise, assign addresses..................*/
    if (fLogicalDel) rc = NO_SEG_FOUND ;
    else
    {
      *paddrNext = *paddrIn;
      *paddrIn = addr;
    } /* endif  (fLogicalDel) */
  } /* endif (rc == OK) */
  return (rc);
} /* end of GetSegByAddr */


/*---------------------------------------------------------------------*\
|                                Tmt Info                               |
+-----------------------------------------------------------------------+
|  Function name      : TmtInfo                                         |
|  Description        : Supply TM Information                           |
|  Function Prototype : TmtInfo (ptmtg, pInfoIn, pInfoOut)              |
+-----------------------------------------------------------------------+
|  Implementation  Remarks                                              |
|  -------------------------                                            |
|  This function supplies general information on the TM file taken from |
|  the Tm Header.                                                       |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  -  OK (0)    - when command ends successfully                        |
|  -  other values - if Dos operation like read/write fails then the    |
|                                                                       |
|  Function calls :                                                     |
|  ----------------                                                     |
\*---------------------------------------------------------------------*/
USHORT
  TmtInfo(PTMT_GLOBALS     ptmtg,         /* pointer to Tmt globals....*/
          PINFO_OUT        pInfoOut)      /* pointer to output buffer..*/
{

  PTM_HEADER    ptmh=ptmtg->pTmHeader;
  /*-------------------------------------------------------------------*/

  pInfoOut->ctmh = ptmh->ctmh;
  pInfoOut->tCreate = ptmh->tCreate;

  memcpy(pInfoOut->abABGrouping, ptmh->abABGrouping, sizeof(ABGROUP) );

  memcpy(pInfoOut->bufData, ((PUCHAR)ptmh) + ptmh->usDispExclTagList,
            ptmh->usLenExclTagList);
  pInfoOut->usDispExclTagList = 0;
  pInfoOut->usLenExclTagList = ptmh->usLenExclTagList;

  memcpy(pInfoOut->bufData + ptmh->usLenExclTagList,
            ((PUCHAR) ptmh) + ptmh->usDispExclWordList,
           ptmh->usLenExclWordList);
  pInfoOut->usDispExclWordList = ptmh->usLenExclTagList;
  pInfoOut->usLenExclWordList = ptmh->usLenExclWordList;

  pInfoOut->prefout.usLenOut = (sizeof(INFO_OUT) -
                                sizeof(pInfoOut->bufData))  +
                               pInfoOut->usLenExclWordList  +
                               pInfoOut->usLenExclTagList ;
  return(OK);
} /* end TmtInfo ......................................................*/


/*---------------------------------------------------------------------*\
|                                Check Word                             |
+-----------------------------------------------------------------------+
|  Function name      : TmtInfo                                         |
|  Description        : Check for equality betwen two words             |
|  Function Prototype : TmtInfo (pteFirst, pteSecond )                  |
+-----------------------------------------------------------------------+
|  Implementation  Remarks                                              |
|  -------------------------                                            |
|  This function is given an input of 2 pointers to TokenEntry structure|
|  s, Each structure points to a word. The words are compared for       |
|  complete or partial eqfuality(one word can be the prefix of the other|
|  word.                                                                |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  -  TRUE        - One word is equal or is the prefix of the other word|
|  -  FALSE       - The two word are not equal                          |
|                                                                       |
|  Function calls :                                                     |
|  ----------------                                                     |
\*---------------------------------------------------------------------*/
BOOL
  CheckWord( PTOKENENTRY  pteFirst,
             PTOKENENTRY  pteSecond )
{
  USHORT        usIndex,
                usMinLength;
  BOOL          rc = TRUE;
  /*-------------------------------------------------------------------*/
  //--- get the length of the shortest word
  usMinLength = min(pteFirst->usLength, pteSecond->usLength);

  //--- check if the length is <= 5
  //--- This is done to find more fuzzy matches
  //--- example:
  //---   starts and stops started tasks
  //---   starting and stoping started tasks (now found as fuzzy match)

  if ( usMinLength <= 5 )
  {
      usMinLength -= 1;
  }
  else
  {
     usMinLength -= 2;
  }/*endif*/

  for(usIndex=0; rc && (usIndex < usMinLength); usIndex++)
    rc=(tolower(pteFirst->pDataString[usIndex]) ==
        tolower(pteSecond->pDataString[usIndex]) );

  return rc ;
}/*end CheckWord ......................................................*/

/*---------------------------------------------------------------------*\
|                       Length Correct Thresholds                       |
+-----------------------------------------------------------------------+
|  Function name      : LengthCorrectThresholds                         |
|  Description        : increase thresholds in dependecy of number      |
|                       of words                                        |
|  Function Prototype : LengthCorrectThresholds( ptmtg, usLenInput,     |
|                                                pGetIn             )   |
+-----------------------------------------------------------------------+
|  Implementation  Remarks                                              |
|  -------------------------                                            |
|   This function increases the length and match thresholds             |
|   values in dependency of the number of words in the input segment.   |
|   For increasing the match threshold the differences from the         |
|   initial thresholds and the maximal threshold are taken and          |
|   increased by a percentage value.                                    |
|   The initial and maximal thresholds and the percentage values are    |
|   defined in the file EQFTMGET.H                                      |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|                                                                       |
|  Function calls :                                                     |
|  ----------------                                                     |
|                                                                       |
\*---------------------------------------------------------------------*/
VOID
LengthCorrectThresholds( PTMT_GLOBALS ptmtg,      //pointer to TM globals
                         USHORT       usLenInput, //number of words in inp. seg.
                         PGET_IN      pGetIn )    //pointer to input buffer
{
   LONG lDifferenceMatchThr;   //difference initial to maximal match threshold
   LONG lDifferenceLengthThr;  //difference initial to maximal length threshold

   //--- calculate the difference from initial match threshold to the
   //--- maximum match threshold
   lDifferenceMatchThr = EXTENT_SIMILAR - pGetIn->lInitMatchThr;

   //--- calculate the difference from initial length threshold to the
   //--- maximum length threshold
   lDifferenceLengthThr = MAX_LENGTH_THR - pGetIn->lLengthThr;

   if ( usLenInput <= LENGTH_SHORT_VALUE )
   {  //--- the input segment is a short segment
      if ( usLenInput <= LENGTH_SHORTER_VALUE )
      {  //--- the input segment is a shorter segment
         //--- set match threshold for shorter segments
         ptmtg->lActMatchThr = SHORTER_MATCH_THR;
      }
      else
      {
         //--- increase match value by a small percentage
         ptmtg->lActMatchThr =
               pGetIn->lInitMatchThr +
               ( ( lDifferenceMatchThr * MATCH_PERCENTAGE_SHORT_VALUE ) / 100 );
      }/*endif*/

      //--- increase length value by a small percentage
      ptmtg->lActLengthThr =
            pGetIn->lLengthThr +
            ( ( lDifferenceLengthThr * LENGTH_PERCENTAGE_SHORT_VALUE ) / 100 );
   }
   else
   {  //--- the input segment is NO!  short segment
      if ( usLenInput <= LENGTH_MEDIUM_VALUE )    //medium segment
      {  //--- the input segment is a medium segment

         //--- increase match value by a medium percentage
         ptmtg->lActMatchThr =
            pGetIn->lInitMatchThr +
            ( ( lDifferenceMatchThr * MATCH_PERCENTAGE_MEDIUM_VALUE ) / 100 );
         //--- increase length value by a medium percentage
         ptmtg->lActLengthThr =
            pGetIn->lLengthThr +
            ( ( lDifferenceLengthThr * LENGTH_PERCENTAGE_MEDIUM_VALUE ) / 100 );
      }
      else
      {  //--- the input segment is NO! medium segment
         if ( usLenInput <= LENGTH_LONG_VALUE )
         {  //--- the input segment is a long segment

            //--- increase match value by a greate percentage
            ptmtg->lActMatchThr =
               pGetIn->lInitMatchThr +
               ( ( lDifferenceMatchThr * MATCH_PERCENTAGE_LONG_VALUE ) / 100 );
            //--- increase length value by a greate percentage
            ptmtg->lActLengthThr =
               pGetIn->lLengthThr +
               ( ( lDifferenceLengthThr * LENGTH_PERCENTAGE_LONG_VALUE ) / 100 );
         }
         else
         {  //--- the input segment is a longer segment
            //--- increase match value by a greater percentage
            ptmtg->lActMatchThr =
               pGetIn->lInitMatchThr +
               ( ( lDifferenceMatchThr * MATCH_PERCENTAGE_LONGER_VALUE ) / 100 );
            //--- increase length value by a greate percentage
            ptmtg->lActLengthThr =
               pGetIn->lLengthThr +
               ( ( lDifferenceLengthThr * LENGTH_PERCENTAGE_LONGER_VALUE ) / 100 );
         }/*endif*/
      }/*endif*/
   }/*endif*/
}/* end LengthCorrectThresholds .............................................*/

