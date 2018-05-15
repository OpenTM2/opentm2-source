//+----------------------------------------------------------------------------+
//|EQFNTDEL.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#include <eqf.h>                  // General Translation Manager include file

#define INCL_EQFMEM_DLGIDAS
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFMORPI.H>

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXDelSegm   deletes target translation in tm record    |
//+----------------------------------------------------------------------------+
//|Description:       Deletes target translation in tm record if untranslate   |
//|                   segment is initiated in translation environment          |
//+----------------------------------------------------------------------------+
//|Function call:  TmtXDelSegm( PTMX_CLB pTmClb,    //ptr to ctl block struct  |
//|                            PTMX_PUT_IN pTmDelIn,  //ptr to input struct    |
//|                            PTMX_PUT_OUT pTmDelOut ) //ptr to output struct |
//+----------------------------------------------------------------------------+
//|Input parameter:   PTMX_CLB  pTmClb         control block                   |
//|                   PTMX_PUT_IN pTmDelIn     input structure                 |
//+----------------------------------------------------------------------------+
//|Output parameter:  PTMX_PUT_OUT pTmDelOut   output structure                |
//+----------------------------------------------------------------------------+
//|Returncode type:  USHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:  Identical to return code in out structure                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|     tokenize source string in del in structure                             |
//|     build hashes and triple hashes                                         |
//|     check if hashes on in compact area                                     |
//|     if all hashes are on                                                   |
//|      find tm keys for found hashes                                         |
//|      loop through the tm keys                                              |
//|        get tm record                                                       |
//|        find and delete target record readjusting tm record length          |
// ----------------------------------------------------------------------------+
USHORT TmtXDelSegm
(
  PTMX_CLB pTmClb,         //ptr to ctl block struct
  PTMX_PUT_IN_W pTmDelIn,  //ptr to input struct
  PTMX_PUT_OUT_W pTmDelOut //ptr to output struct
)
{
  PTMX_SENTENCE pSentence = NULL;    // ptr to sentence structure
  ULONG ulKey;                         // tm record key
  BOOL fOK;                            // success indicator
  USHORT usRc = NO_ERROR;              // return code
  USHORT usMatchesFound = 0;           // compact area hits
  ULONG  ulLen = 0;                    // length indication
  PTMX_RECORD pTmRecord = NULL;        // pointer to tm record
  CHAR szString[MAX_EQF_PATH];         // character string
  PULONG pulSids = NULL;               // ptr to sentence ids
  PULONG pulSidStart = NULL;           // ptr to sentence ids
  BOOL        fLocked = FALSE;         // TM-database-has-been-locked flag
  ULONG ulRecBufSize = TMX_REC_SIZE;   // current size of record buffer

  //allocate pSentence
  fOK = UtlAlloc( (PVOID *) &(pSentence), 0L, (LONG)sizeof( TMX_SENTENCE ), NOMSG );

  if ( fOK )
   fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );

  if ( fOK )
   fOK = UtlAlloc( (PVOID *) &(pSentence->pInputString), 0L,
                   (LONG)( MAX_SEGMENT_SIZE * sizeof(CHAR_W)), NOMSG );
  if ( fOK )
   fOK = UtlAlloc( (PVOID *) &(pSentence->pNormString), 0L,
                   (LONG)( MAX_SEGMENT_SIZE * sizeof(CHAR_W)), NOMSG );
  if ( fOK )
   fOK = UtlAlloc( (PVOID *) &(pSentence->pulVotes), 0L,
                   (LONG)(ABS_VOTES * sizeof(ULONG)), NOMSG );
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &pSentence->pTagRecord, 0L, (LONG)TOK_SIZE, NOMSG);
    if ( fOK )
      pSentence->lTagAlloc = (LONG)TOK_SIZE;
  } /* endif */

  //allocate 4k for pTermTokens
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &pSentence->pTermTokens, 0L, (LONG)TOK_SIZE, NOMSG );
    if ( fOK )
      pSentence->lTermAlloc = (LONG)TOK_SIZE;
  } /* endif */

  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pulSids), 0L, (LONG)(MAX_INDEX_LEN * sizeof(ULONG)),
                    NOMSG );
    if ( fOK )
      pulSidStart = pulSids;
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
    strcat( szString, pTmDelIn->stTmPut.szTagTable );
    strcat( szString, EXT_OF_FORMAT );

    //remember start of norm string
    pSentence->pNormStringStart = pSentence->pNormString;

    UTF16strcpy( pSentence->pInputString, pTmDelIn->stTmPut.szSource );
    //tokenize source segment, resuting in normalized string and tag table record
    usRc = TokenizeSource( pTmClb, pSentence, szString,
                           pTmDelIn->stTmPut.szSourceLanguage,
                           pTmClb->stTmSign.bMajorVersion );

    // set the tag table ID in the tag record (this can't be done in TokenizeSource anymore)
    if ( usRc == NO_ERROR )
    {
      if ( pTmClb )
      {
        usRc = NTMGetIDFromName( pTmClb, pTmDelIn->stTmPut.szTagTable, NULL, (USHORT)TAGTABLE_KEY, &pSentence->pTagRecord->usTagTableId );
      }
      else
      {
        pSentence->pTagRecord->usTagTableId = 0;
      } /* endif */
    }
  } /* endif */


  // update TM databse
  if ( !usRc )
  {
    //set pNormString to beginning of string
    pSentence->pNormString = pSentence->pNormStringStart;
    HashSentence( pSentence, pTmClb->stTmSign.bMajorVersion, pTmClb->stTmSign.bMinorVersion );
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


  // update TM databse
  if ( !usRc )
  {
    usMatchesFound = CheckCompactArea( pSentence, pTmClb );
    if ( usMatchesFound == pSentence->usActVote ) //all hash triples found
    {
      usRc = DetermineTmRecord( pTmClb, pSentence, pulSids );
      if ( !usRc )
      {
        while ( *pulSids )
        {
          ulKey = *pulSids;
          ulLen = TMX_REC_SIZE;
          usRc = EQFNTMGet( pTmClb->pstTmBtree,
                            ulKey,  //tm record key
                            (PCHAR)pTmRecord,   //pointer to tm record data
                            &ulLen );  //length
          // re-alloc buffer and try again if buffer overflow occured
          if ( usRc == BTREE_BUFFER_SMALL )
          {
            fOK = UtlAlloc( (PVOID *)&(pTmRecord), ulRecBufSize, ulLen, NOMSG );
            if ( fOK )
            {
              ulRecBufSize = ulLen;

              usRc = EQFNTMGet( pTmClb->pstTmBtree,
                                ulKey,
                                (PCHAR)pTmRecord,
                                &ulLen );
            }
            else
            {
              usRc = ERROR_NOT_ENOUGH_MEMORY;
            } /* endif */
          } /* endif */

          if ( usRc == NO_ERROR )
          {

            //find target record and delete, if the target record was the
            //only target in the tm record, delete the entire record
            usRc = FindTargetAndDelete( pTmClb, pTmRecord,
                                &pTmDelIn->stTmPut, pSentence, &ulKey );
            if ( usRc == SEG_NOT_FOUND )
            {
              //get next tm record
              pulSids++;
            }
            else
            {
              //target record was found and deleted or an error occured
              //so don't try other sids
              *pulSids = 0;
            } /* endif */
          } /* endif */
        } /* endwhile */
      } /* endif */
    }
    else
    {
      usRc = SEG_NOT_FOUND;
    } /* endif */
  } /* endif */

  // unlock TM database if database has been locked
  if ( fLocked )
  {
    NTMLockTM( pTmClb, FALSE, &fLocked );
  } /* endif */


  //release memory
  UtlAlloc( (PVOID *) &pSentence->pTagRecord, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pSentence->pTermTokens, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pSentence->pNormString, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pSentence->pInputString, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pSentence->pulVotes, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pSentence, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pTmRecord, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pulSidStart, 0L, 0L, NOMSG );

  pTmDelOut->stPrefixOut.usLengthOutput = sizeof( TMX_PUT_OUT );
  pTmDelOut->stPrefixOut.usTmtXRc = usRc;
  return( usRc );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FindTargetAndDelete delete target record if it matches   |
//|                                       input data                           |
//+----------------------------------------------------------------------------+
//|Description:       Deletes a target record - function triggered in          |
//|                   translation environment when untranslate segment is      |
//|                   done                                                     |
//+----------------------------------------------------------------------------+
//|Function call:  FindTargetAnddelete( PTMX_CLB pTmClb, //ptr to ctl block    |
//|                            PTMX_RECORD pTmRecord, //ptr to tm record       |
//|                            PTMX_PUT_IN pTmDelIn->stTmPut, //input structure|
//|                            PTMX_SENTENCE pSentence, //ptr to sent structure|
//|                            PULONG pulKey ) //tm key                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PTMX_CLB  pTmClb         control block                   |
//|                   PTMX_RECORD pTmRecord    tm record                       |
//|                   PTMX_PUT_IN pTmDelIn     input structure                 |
//|                   PTMX_SENTENCE pSentence  sentence structure              |
//|                   PULONG pulKey            tm key                          |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes: NO_ERROR                                                       |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|    if source strings equal                                                 |
//|     loop through all target records in tm record                           |
//|       if target langs equal                                                |
//|         if source string tag table records are equal                       |
//|           if target strings are equal                                      |
//|             if target tag table records are equal                          |
//|               if segment id, file name and mt flag are equal               |
//|                 if multiple flag not on                                    |
//|                   delete target record and readjust total length of tm     |
//|                   record                                                   |
//|                   leave target record loop                                 |
//|                 else                                                       |
//|                   don't delete                                             |
//|                   leave target record loop                                 |
//|               else                                                         |
//|                 next target record                                         |
//|             else                                                           |
//|               next target record                                           |
//|           else                                                             |
//|             next target record                                             |
//|         else                                                               |
//|           next target record                                               |
//|       else                                                                 |
//|         next target record                                                 |
//|    else                                                                    |
//|      get next sentence key                                                 |
// ----------------------------------------------------------------------------+
USHORT FindTargetAndDelete( PTMX_CLB    pTmClb,
                            PTMX_RECORD pTmRecord,
                            PTMX_PUT_W  pTmDel,
                            PTMX_SENTENCE pSentence,
                            PULONG pulKey )
{
  BOOL fOK = FALSE;                    //success indicator
  BOOL fStop = FALSE;                  //indicates whether to leave loop or not
  PBYTE pByte;                         //position ptr
  PBYTE pStartTarget;                  //position ptr
  PTMX_SOURCE_RECORD pTMXSourceRecord = NULL; //ptr to source record
  PTMX_TARGET_RECORD pTMXTargetRecord = NULL; //ptr to target record
  PTMX_TARGET_CLB    pClb = NULL;    //ptr to target control block
  PTMX_TAGTABLE_RECORD pTMXSourceTagTable = NULL; //ptr to source tag info
  PTMX_TAGTABLE_RECORD pTMXTargetTagTable = NULL; //ptr to tag info
  PTMX_TAGTABLE_RECORD pTagRecord = NULL;  //ptr to tag info
  LONG lTagAlloc;                      //allocate length
  ULONG ulLen = 0;                    //length indicator
  USHORT usNormLen = 0;                //length of normalized string
  PSZ_W  pString = NULL;               //pointer to character string
  PSZ_W  pNormString = NULL;           //pointer to character string
  USHORT usId = 0;                     //returned id from function
  USHORT usRc = NO_ERROR;              //returned value from function

  //allocate pString
  fOK = UtlAlloc( (PVOID *) &(pString), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );

  //allocate normalized string
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pNormString), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
  } /* endif */

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
    //position at beginning of source structure in tm record
    pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);

    //move pointer to source string
    pByte = (PBYTE)(pTmRecord+1);
    pByte += pTMXSourceRecord->usSource;

    //calculate length of source string
    ulLen = (RECLEN(pTMXSourceRecord) - sizeof( TMX_SOURCE_RECORD ));

    //clear pString for later use
    memset( pString, 0, MAX_SEGMENT_SIZE * sizeof(CHAR_W) );

    //copy source string for later compare function
//    memcpy( pString, pByte, ulLen );
    ulLen = EQFCompress2Unicode( pString, pByte, ulLen );

    //compare source strings
    if ( !UTF16strcmp( pString, pSentence->pNormString ) )
    {
      ULONG   ulLeftTgtLen;            // remaining target length
      /****************************************************************/
      /* get length of target block to work with                      */
      /****************************************************************/
      assert( RECLEN(pTmRecord) >= pTmRecord->usFirstTargetRecord );
      ulLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;

      //source strings equal
      //position at first target record
      pByte = (PBYTE)(pTmRecord+1);
      pByte += RECLEN(pTMXSourceRecord);
      pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
      pStartTarget = (PBYTE)pTMXTargetRecord;

      //source strings are identical so loop through target records
      while ( ulLeftTgtLen && ( RECLEN(pTMXTargetRecord) != 0) && !fStop )
      {
        /**************************************************************/
        /* update left target length                                  */
        /**************************************************************/
        assert( ulLeftTgtLen >= RECLEN(pTMXTargetRecord) );
        ulLeftTgtLen -= RECLEN(pTMXTargetRecord);

        //next check the target language
        //position at target control block
        pByte += pTMXTargetRecord->usClb;
        pClb = (PTMX_TARGET_CLB)pByte;

        // replace any 0xA0 in language name to 0xFF
        // (0xA0 is 0xFF after processing by OemToAnsi)
        REPLACE_A0_BY_FF( pTmDel->szTargetLanguage );

        //get id of target language in the put structure
        usRc = NTMGetIDFromName( pTmClb, pTmDel->szTargetLanguage,
                                 NULL,
                                 (USHORT)LANG_KEY, &usId );
        //compare target language ids
        if ( (pClb->usLangId == usId) && !usRc )
        {
          //compare source tag table records
          //position at source tag table record
          pByte = pStartTarget;
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

          //position at source tag table
          pByte += pTMXTargetRecord->usSourceTagTable;
          pTMXSourceTagTable = (PTMX_TAGTABLE_RECORD)pByte;

          //compare tag table records
          if ( !memcmp( pTMXSourceTagTable, pSentence->pTagRecord,
                       RECLEN(pTMXSourceTagTable) ) )
          {
            //source tag tables are identical
            pByte = pStartTarget;
            pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
            pByte += pTMXTargetRecord->usTarget;

            //calculate length of target string
            ulLen = pTMXTargetRecord->usClb - pTMXTargetRecord->usTarget;

            //clear pString for later use
            memset( pString, 0, MAX_SEGMENT_SIZE * sizeof(CHAR_W));

            //copy target string for later compare function
//            memcpy( pString, pByte, ulLen );
            ulLen = EQFCompress2Unicode( pString, pByte, ulLen );

            //tokenize target string in del structure
            usRc = TokenizeTarget( pTmDel->szTarget, pNormString, &pTagRecord, &lTagAlloc, pTmDel->szTagTable, &usNormLen, pTmClb );

            if ( !usRc )
            {
              //compare target strings
              if ( !UTF16strcmp( pString, pNormString ) )
              {
                //target strings are equal so compare target tag records
                //position at target tag table record
                pByte = pStartTarget;
                pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
                pByte += pTMXTargetRecord->usTargetTagTable;
                pTMXTargetTagTable = (PTMX_TAGTABLE_RECORD)pByte;

                //compare tag table records
                if ( !memcmp( pTMXTargetTagTable, pTagRecord,
                              RECLEN(pTMXTargetTagTable) ) )
                {
                  //identical target tag table as in del structure so
                  //check segment id and file name in control block
                  pByte = pStartTarget;
                  pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

                  //position at target control block
                  pByte += pTMXTargetRecord->usClb;
                  pClb = (PTMX_TARGET_CLB)pByte;

                  //get id of filename in the put structure
                  usRc = NTMGetIDFromName( pTmClb, pTmDel->szFileName,
                                           pTmDel->szLongName,
                                           (USHORT)FILE_KEY, &usId );
                  if ( !usRc )
                  {
                    ULONG ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                                         pTMXTargetRecord->usClb;
                    while ( ulLeftClbLen && !fStop )
                    {
                      if ( (pClb->usFileId == usId) &&
                           (pClb->ulSegmId == pTmDel->ulSourceSegmentId) &&
                           (pClb->bTranslationFlag == (BYTE)pTmDel->usTranslationFlag))
                      {
                        //if segment id and filename are equal then delete
                        //target CLB and any empty target record

                        //check that multiple flag isn't on
                        //if on leave while loop as though delete was carried out
                        if ( !pClb->bMultiple || (BOOL)pTmDel->lTime )
                        {
                          TMDelTargetClb( pTmRecord, pTMXTargetRecord, pClb );

                          //add updated tm record to database
                          /**********************************************/
                          /* we usually should delete the record here   */
                          /* (if no translation is available for it)    */
                          /* BUT: since usually an untranslate is follow*/
                          /*  by a new translation, we will not remove  */
                          /* the key (only get rid of any target data)  */
                          /**********************************************/
                          usRc = EQFNTMUpdate( pTmClb->pstTmBtree,
                                               *pulKey,
                                               (PBYTE)pTmRecord,
                                               RECLEN(pTmRecord) );
                        } /* endif */

                        //leave while loop
                        fStop = TRUE;
                      }
                      else
                      {
                        // try next target CLB
                        ulLeftClbLen -= TARGETCLBLEN(pClb);
                        pClb = NEXTTARGETCLB(pClb);
                      } /* endif */
                    } /* endwhile */
                  } /* endif */
                } /* endif */
              } /* endif */
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
      } /* endwhile */
    } /* endif */

    if ( !fStop )
    {
      usRc = SEG_NOT_FOUND;
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pString, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &(pNormString), 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &(pTagRecord), 0L, 0L, NOMSG );

  return( usRc );
}

