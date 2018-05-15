//+----------------------------------------------------------------------------+
//|EQFNTEXT.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#define INCL_EQF_ASD
#include <eqf.h>                  // General Translation Manager include file

#define INCL_EQFMEM_DLGIDAS
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFMORPI.H>

static USHORT ExtractRecordV5
(
  PTMX_CLB        pTmClb,         //ptr to ctl block struct
  PTMX_RECORD     pTmRecord,
  PTMX_EXT_IN_W   pTmExtIn,
  PTMX_EXT_OUT_W  pTmExtOut,
  ULONG           ulOemCP
);

static
USHORT FillExtStructureV5
(
  PTMX_CLB    pTmClb,                  //ptr to control block
  PTMX_TARGET_RECORD pTMXTargetRecord, //ptr to tm target
  PTMX_OLD_TARGET_CLB    pTargetClb,   // ptr to current target CLB
  PSZ         pSourceString,           //ptr to source string
  PULONG      pulSourceLen,            //length of source string
  PTMX_EXT_W  pstExt,                   //extout ext struct
  ULONG       ulOemCP
);


static USHORT ExtractRecordV6
(
  PTMX_CLB        pTmClb,         //ptr to ctl block struct
  PTMX_RECORD     pTmRecord,
  PTMX_EXT_IN_W   pTmExtIn,
  PTMX_EXT_OUT_W  pTmExtOut
);


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXExtract  gets source and targets from tm data file   |
//|                                in sequential order                         |
//+----------------------------------------------------------------------------+
//|Description:       Gets source and targets in sequential order              |
//+----------------------------------------------------------------------------+
//|Function call:  TmtXGet( PTMX_CLB pTmClb,        //ptr to ctl block struct  |
//|                         PTMX_EXT_IN pTmExtIn,   //ptr to input struct      |
//|                         PTMX_EXT_OUT pTmExtOut ) //ptr to output struct    |
//+----------------------------------------------------------------------------+
//|Input parameter:   PTMX_CLB  pTmClb         control block                   |
//|                   PTMX_EXT_IN pTmExtIn     input structure                 |
//+----------------------------------------------------------------------------+
//|Output parameter:  PTMX_EXT_OUT pTmExtOut   output structure                |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|     get tm record                                                          |
//|     go to target record specified in ext in structure                      |
//|     if target record exists                                                |
//|       fill put structure in ext out structure                              |
//|       position at next target                                              |
//|         if target record exists                                            |
//|           fill usNextTarget                                                |
//|           ulTmKey is equal to same key                                     |
//|         else                                                               |
//|           set usNextTarget to 1                                            |
//|           ulTmKey is equal to next key                                     |
//|     else                                                                   |
//|       set usNextTarget to 1                                                |
//|       ulTmKey is equal to next key                                         |
//|                                                                            |
// ----------------------------------------------------------------------------+

USHORT TmtXExtract
(
  PTMX_CLB pTmClb,         //ptr to ctl block struct
  PTMX_EXT_IN_W  pTmExtIn, //ptr to input struct
  PTMX_EXT_OUT_W pTmExtOut //ptr to output struct
)
{
  USHORT usRc = NO_ERROR;              //return code
  BOOL fOK;                            //success indicator
  PTMX_RECORD pTmRecord = NULL;        //pointer to tm record
  ULONG  ulRecBufSize = 0;             // current size of record buffer
  ULONG  ulLen = 0;                    //length indicator
  PTMX_SOURCE_RECORD pTMXSourceRecord; //ptr to source record
  PTMX_TARGET_RECORD pTMXTargetRecord; //ptr to target record
  ULONG ulStartKey;                    //start tm key
  ULONG ulNextKey;                     //last tm key
  BOOL  fSpecialMode = FALSE;          //special mode flag
  ULONG   ulOemCP = 0L;

  ulOemCP = GetLangOEMCP(NULL);

  //allocate 32K for tm record
  fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) (2 * TMX_REC_SIZE), NOMSG );
  ulRecBufSize = 2 * TMX_REC_SIZE;
  //allocate pString

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

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

      /********************************************************************/
      /* Set special mode flag                                            */
      /********************************************************************/
      if ( !usRc )
      {
        if ( (pTmExtIn->usConvert == MEM_OUTPUT_TAGTABLES) ||
             (pTmExtIn->usConvert == MEM_OUTPUT_AUTHORS)   ||
             (pTmExtIn->usConvert == MEM_OUTPUT_LONGNAMES) ||
             (pTmExtIn->usConvert == MEM_OUTPUT_ALLDOCS)   ||
             (pTmExtIn->usConvert == MEM_OUTPUT_DOCUMENTS) ||
             (pTmExtIn->usConvert == MEM_OUTPUT_LANGUAGES) )
        {
          fSpecialMode = TRUE;
        }
        else
        {
          fSpecialMode = FALSE;
        } /* endif */
      } /* endif */

      /********************************************************************/
      /* Special mode to get list of tag tables, authors, documents or    */
      /* languages                                                        */
      /********************************************************************/
      if ( !usRc && fSpecialMode)
      {
        PTMX_TABLE pTable = NULL;          // table containing the requested info
        ULONG     ulMaxEntries = 0;        // last + 1 entry in table

        /******************************************************************/
        /* Address table containg the requested names                     */
        /******************************************************************/
        switch ( pTmExtIn->usConvert )
        {
          case MEM_OUTPUT_TAGTABLES :
            pTable = pTmClb->pTagTables;
            ulMaxEntries = pTable->ulMaxEntries;
            break;
          case MEM_OUTPUT_AUTHORS   :
            pTable = pTmClb->pAuthors;
            ulMaxEntries = pTable->ulMaxEntries;
            break;
          case MEM_OUTPUT_DOCUMENTS :
            pTable = pTmClb->pFileNames;
            ulMaxEntries = pTable->ulMaxEntries;
            break;
          case MEM_OUTPUT_LANGUAGES :
            pTable = pTmClb->pLanguages;
            ulMaxEntries = pTable->ulMaxEntries;
            break;
          case MEM_OUTPUT_LONGNAMES :
            // different handling for long names, no setting of pTable
            ulMaxEntries = pTmClb->pLongNames->ulEntries;
            break;
          case MEM_OUTPUT_ALLDOCS :
            // different handling for long names, no setting of pTable
            pTable = pTmClb->pFileNames;
            ulMaxEntries = pTable->ulMaxEntries;
            break;
          default :
            usRc = BTREE_INVALID;
            break;
        } /* endswitch */

        /******************************************************************/
        /* Extract requested names                                        */
        /******************************************************************/
        if ( !usRc )
        {
          PTMX_TABLE_ENTRY pEntry;         // ptr to current entry
          PSZ_W   pszBuffer;               // ptr to next free space in buffer
          LONG    lLen;                    // lenght of current name
          LONG    lRoomLeft;               // space left in current buffer
          int     iteration;               // iteration counter
          PSZ     pszName;                 // ptr to current name

          /****************************************************************/
          /* Initialize our output buffers                                */
          /****************************************************************/
          memset( pTmExtOut->stTmExt.szSource, NULC,
                  sizeof(pTmExtOut->stTmExt.szSource) );
          memset( pTmExtOut->stTmExt.szTarget, NULC,
                  sizeof(pTmExtOut->stTmExt.szTarget) );

          /****************************************************************/
          /* Loop two times: first time to fill szSource, the second      */
          /* time to fill the szTarget buffer                             */
          /****************************************************************/
          iteration = 0;
          do
          {
            // set buffer and lRoomLeft variables
            if ( iteration == 0 )
            {
              // use szSource buffer
              pszBuffer = pTmExtOut->stTmExt.szSource;
              // leave room for NULC and ASCII expansion
              lRoomLeft = (sizeof(pTmExtOut->stTmExt.szSource)/sizeof(CHAR_W))/2 - 1;
            }
            else
            {
              // use szTarget buffer
              pszBuffer = pTmExtOut->stTmExt.szTarget;
              // leave room for NULC and ASCII expansion
              lRoomLeft = (sizeof(pTmExtOut->stTmExt.szTarget)/sizeof(CHAR_W))/2 - 1;
            } /* endif */

            while ( lRoomLeft && (pTmExtIn->usNextTarget < ulMaxEntries) )
            {
              if ( pTmExtIn->usConvert == MEM_OUTPUT_LONGNAMES )
              {
                pszName = pTmClb->pLongNames->
                            stTableEntry[pTmExtIn->usNextTarget].pszLongName;
              }
              else
              {
                pEntry = &pTable->stTmTableEntry;
                pEntry += pTmExtIn->usNextTarget;
                pszName = pEntry->szName;
                // check if there is a long name for the current document
                if ( pTmExtIn->usConvert == MEM_OUTPUT_ALLDOCS )
                {
                  ULONG ulEntry = 0;
                  while ( (ulEntry < pTmClb->pLongNames->ulEntries) &&
                          (pEntry->usId != pTmClb->pLongNames->
                                           stTableEntry[ulEntry].usId ) )
                  {
                    ulEntry++;
                  } /* endwhile */
                  if ( ulEntry < pTmClb->pLongNames->ulEntries )
                  {
                    pszName = pTmClb->pLongNames->stTableEntry[ulEntry].pszLongName;
                  } /* endif */
                } /* endif */
              } /* endif */
              lLen = strlen(pszName);

              if ( lRoomLeft > (lLen + 1))
              {
                ASCII2Unicode( pszName, pszBuffer, ulOemCP );
                pszBuffer += lLen;
                *pszBuffer++ = X15;
                lRoomLeft -= lLen + 1;
                pTmExtIn->usNextTarget++;
              }
              else
              {
                lRoomLeft = 0;
              } /* endif */
            } /* endwhile */

            *pszBuffer = NULC;             // terminate current buffer
            iteration++;                   // continue with next buffer
          } while ( (iteration <= 1) && (pTmExtIn->usNextTarget < ulMaxEntries) ); /* enddo */

          /****************************************************************/
          /* Set flags in output structure                                */
          /****************************************************************/
          if ( pTmExtIn->usNextTarget < ulMaxEntries )
          {
            // more names available for extract
            pTmExtOut->usNextTarget = pTmExtIn->usNextTarget;
            pTmExtOut->stTmExt.usTranslationFlag = TRANSLFLAG_MACHINE;
          }
          else
          {
            // all names have been extracted
            pTmExtOut->usNextTarget = 0;
            pTmExtOut->stTmExt.usTranslationFlag = TRANSLFLAG_NORMAL;
          } /* endif */
        } /* endif */
      } /* endif */

      /********************************************************************/
      /* Normal mode of TMExtract                                         */
      /********************************************************************/
      if ( !usRc && !fSpecialMode )
      {
        EQFNTMGetNextNumber( pTmClb->pstTmBtree, &ulStartKey, &ulNextKey);
        pTmExtOut->ulMaxEntries = (ulNextKey - ulStartKey);
        /******************************************************************/
        /* return one matching entry (if any available)                   */
        /******************************************************************/
        usRc = BTREE_NOT_FOUND;
        while ( (pTmExtIn->ulTmKey < ulNextKey) && (usRc == BTREE_NOT_FOUND) )
        {
          ulLen = ulRecBufSize;
          usRc = EQFNTMGet( pTmClb->pstTmBtree,
                            pTmExtIn->ulTmKey,
                            (PCHAR)pTmRecord,
                            &ulLen );

          // re-alloc buffer and try again if buffer overflow occured
          if ( usRc == BTREE_BUFFER_SMALL )
          {
            fOK = UtlAlloc( (PVOID *)&(pTmRecord), ulRecBufSize, ulLen, NOMSG );
            if ( fOK )
            {
              ulRecBufSize = ulLen;

              usRc = EQFNTMGet( pTmClb->pstTmBtree,
                                pTmExtIn->ulTmKey,
                                (PCHAR)pTmRecord,
                                &ulLen );
            }
            else
            {
              usRc = ERROR_NOT_ENOUGH_MEMORY;
            } /* endif */
          } /* endif */

          // adjust records of older TMs (Major Version < 4) (in-spot)
          if ( (usRc == NO_ERROR) &&
               (pTmClb->stTmSign.bMajorVersion < TM_MAJ_VERSION_4) )
          {
            PBYTE  pbTemp;             // byte pointer for bytewise access
            ULONG  ulLen;              // length of record
            ULONG ulRemaining;        // remaining (unprocessed) record length
            PTMX_TAGTABLE_RECORD pTagTable; // ptr to tag table record

            // get original length and byte pointer to start of record
            ulLen = *((PUSHORT)pTmRecord);
            ulRemaining = ulLen;
            pbTemp = (PBYTE)pTmRecord;

            // make room for larger RecordLen
            memmove ( pbTemp + 2, pbTemp, ulLen );

            // increase overall record length
            pTmRecord->ulRecordLen = ulLen + 2;
            ulRemaining = (pTmRecord->ulRecordLen - sizeof(TMX_RECORD));

            // adjust offset of source record
            pTmRecord->usSourceRecord += 2;

            // adjust offset of first target record
            // (first target offset is affected by longer record length
            // field of TM record and longer record length field of
            // source record)
            pTmRecord->usFirstTargetRecord += 4;

            // check if source record offset is  valid
            if ( pTmRecord->usSourceRecord < sizeof(TMX_RECORD) )
            {
              usRc = BTREE_CORRUPTED;
            } /* endif */

            if ( usRc == NO_ERROR )
            {
              //
              // process source record
              //

              // position to source record
              pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);

              // get original length and byte pointer to start of record
              ulLen = *((PUSHORT)pTMXSourceRecord);
              pbTemp = (PBYTE)pTMXSourceRecord;

              // make room for larger RecordLen
              memmove ( pbTemp + 2, pbTemp, ulRemaining );

              // record length is increased, adjust source offset
              pTMXSourceRecord->ulRecordLen = ulLen + 2;
              pTmRecord->ulRecordLen += 2;
              pTMXSourceRecord->usSource += 2;
              ulRemaining -= ulLen;
            } /* endif */

            if ( usRc == NO_ERROR )
            {
              //
              // process target records
              //

              // position to first target record
              pbTemp = (PBYTE)pTmRecord;
              pbTemp += pTmRecord->usFirstTargetRecord;
              pTMXTargetRecord = (PTMX_TARGET_RECORD)(pbTemp);

              // for all target records do...
              while ( ulRemaining != 0 )
              {
                // get original length and byte pointer to start of record
                ulLen = *((PUSHORT)pTMXTargetRecord);
                pbTemp = (PBYTE)pTMXTargetRecord;

                // make room for larger RecordLen
                memmove ( pbTemp + 2, pbTemp, ulRemaining );

                // record length is increased
                pTmRecord->ulRecordLen             += 2;
                pTMXTargetRecord->ulRecordLen = ulLen + 2;

                // adjust offsets
                pTMXTargetRecord->usSourceTagTable += 2;
                pTMXTargetRecord->usTargetTagTable += 2;
                pTMXTargetRecord->usTarget         += 2;
                pTMXTargetRecord->usClb            += 2;
                ulRemaining += 2;

                // correct record length of source tag table record

                // position to source tag table and get length
                pbTemp = (PBYTE)pTMXTargetRecord +
                         pTMXTargetRecord->usSourceTagTable;
                pTagTable = (PTMX_TAGTABLE_RECORD)pbTemp;
                ulLen = *((PUSHORT)pbTemp);

                // make room for larger RecordLen
                memmove ( pbTemp + 2, pbTemp, ulRemaining -
                          pTMXTargetRecord->usSourceTagTable);

                // adjust offsets and lengths
                pTmRecord->ulRecordLen             += 2;
                pTMXTargetRecord->ulRecordLen      += 2;
                pTagTable->ulRecordLen = (ULONG)ulLen + 2;
                pTagTable->usFirstTagEntry         += 2;
                pTMXTargetRecord->usTargetTagTable += 2;
                pTMXTargetRecord->usTarget         += 2;
                pTMXTargetRecord->usClb            += 2;
                ulRemaining += 2;

                // correct record length of target tag table record

                // position to target tag table and get length
                pbTemp = (PBYTE)pTMXTargetRecord +
                         pTMXTargetRecord->usTargetTagTable;
                pTagTable = (PTMX_TAGTABLE_RECORD)pbTemp;
                ulLen = *((PUSHORT)pbTemp);

                // make room for larger RecordLen
                memmove ( pbTemp + 2, pbTemp, ulRemaining -
                          pTMXTargetRecord->usTargetTagTable);

                // adjust offsets and lengths
                pTmRecord->ulRecordLen             += 2;
                pTMXTargetRecord->ulRecordLen      += 2;
                pTagTable->ulRecordLen = (ULONG)ulLen + 2;
                pTagTable->usFirstTagEntry         += 2;
                pTMXTargetRecord->usTarget         += 2;
                pTMXTargetRecord->usClb            += 2;
                ulRemaining += 2;

                // continue with next target record
                pbTemp = (PBYTE)pTMXTargetRecord;
                pbTemp += pTMXTargetRecord->ulRecordLen;
                ulRemaining -= pTMXTargetRecord->ulRecordLen;
                pTMXTargetRecord = (PTMX_TARGET_RECORD)pbTemp;
              } /* endwhile */
            } /* endif */
          } /* endif */


          if ( usRc == NO_ERROR )
          {
            if (pTmClb->stTmSign.bMajorVersion < TM_MAJ_VERSION_6)
            {
               usRc = ExtractRecordV5( pTmClb, pTmRecord, pTmExtIn, pTmExtOut, ulOemCP );
            }
            else
            {
               usRc = ExtractRecordV6( pTmClb, pTmRecord, pTmExtIn, pTmExtOut );
            }
          }
          /****************************************************************/
          /* setup new starting point (do this even in the case we are    */
          /* dealing with a corrupted TM )                                */
          /****************************************************************/
          if ( usRc != NO_ERROR )
          {
            if ( (usRc == BTREE_NOT_FOUND) || ( usRc == BTREE_CORRUPTED ) || ((pTmClb->usAccessMode & ASD_ORGANIZE) != 0) )
            {
              pTmExtIn->ulTmKey ++;
              pTmExtIn->usNextTarget = 1;
              pTmExtOut->ulTmKey = pTmExtIn->ulTmKey;
              pTmExtOut->usNextTarget = pTmExtIn->usNextTarget;
              //usRc = BTREE_NOT_FOUND;     // reset error condition
              } /* endif */
          } /* endif */
        } /* endwhile */

        /******************************************************************/
        /* set end of file condition ...                                  */
        /******************************************************************/
        if ( (usRc == BTREE_NOT_FOUND) &&  (pTmExtIn->ulTmKey == ulNextKey) )
        {
          //arrived at last tm record
          usRc = BTREE_EOF_REACHED;
        } /* endif */
      } /* endif */

      if ( (usRc == BTREE_IN_USE) && pTmClb->fShared )
      {
        UtlWait( MAX_WAIT_TIME );
        sRetries--;
      } /* endif */
    }
    while( pTmClb->fShared && (usRc == BTREE_IN_USE) && (sRetries > 0));
  } /* endif */

  pTmExtOut->stPrefixOut.usLengthOutput = sizeof( TMX_EXT_OUT );
  pTmExtOut->stPrefixOut.usTmtXRc = usRc;

  //release memory
  UtlAlloc( (PVOID *) &pTmRecord, 0L, 0L, NOMSG );

  // in organize mode only: change any error code (except BTREE_EOF_REACHED) to BTREE_CORRUPTED
  if ( (usRc != NO_ERROR ) && (usRc != BTREE_EOF_REACHED) && ((pTmClb->usAccessMode & ASD_ORGANIZE) != 0) )
  {
    usRc = BTREE_CORRUPTED;
  } /* endif */

  return( usRc );
}

static
USHORT ExtractRecordV6
(
  PTMX_CLB        pTmClb,         //ptr to ctl block struct
  PTMX_RECORD     pTmRecord,
  PTMX_EXT_IN_W   pTmExtIn,
  PTMX_EXT_OUT_W  pTmExtOut
)
{
  USHORT usRc = 0;
  PTMX_SOURCE_RECORD pTMXSourceRecord; //ptr to source record
  PTMX_TARGET_RECORD pTMXTargetRecord; //ptr to target record
  PTMX_TARGET_CLB    pTargetClb;       //ptr to target CLB
  ULONG       ulLeftClbLen = 0;        // remaining length of CLB area
  PBYTE pByte;                         //position ptr
  ULONG ulSourceLen = 0;              //length of source string
  USHORT usTarget;                     //nr of target records in tm record
  PSZ_W pSourceString = NULL;          //pointer to source string
  PBYTE pSource;                       //position ptr


  UtlAlloc( (PVOID *) &(pSourceString), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );

  if ( !pSourceString )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */


  if ( !usRc )
  {
    //check if record is empty
    if ( ( RECLEN(pTmRecord) != 0) &&
         (pTmRecord->usFirstTargetRecord < RECLEN(pTmRecord)) )
    {
      ULONG   ulLeftTgtLen;            // remaining target length
      /****************************************************************/
      /* get length of target block to work with                      */
      /****************************************************************/
      assert( RECLEN(pTmRecord) >= pTmRecord->usFirstTargetRecord );
      ulLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;
      //extract source string
      //position at beginning of source structure in tm record
      pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);

      //move pointer to corresponding position
      pSource = (PBYTE)(pTmRecord+1);
      pSource += pTMXSourceRecord->usSource;

      //calculate length of source string
      ulSourceLen = (RECLEN(pTMXSourceRecord) -
                            sizeof(TMX_SOURCE_RECORD) );

      //copy source string for fill matchtable
      if ( ulSourceLen < MAX_SEGMENT_SIZE * sizeof(WCHAR) )
      {
        ulSourceLen = EQFCompress2Unicode( pSourceString, pSource, ulSourceLen );
        if ( ulSourceLen < MAX_SEGMENT_SIZE ) {
////      memcpy( pSourceString, pSource, ulSourceLen );
          pSourceString[ulSourceLen] = EOS;
        } 
        else
        {
           usRc = BTREE_CORRUPTED;
        }
      }
      else
      {
        usRc = BTREE_CORRUPTED;
      } /* endif */

      if ( !usRc )
      {
        //find target record specified in ExtIn structure

        //move pointer to first target
        pByte = (PBYTE)(pTmRecord);
        pByte += pTmRecord->usFirstTargetRecord;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
        pTargetClb = (PTMX_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
        if ( pTMXTargetRecord->usClb >= RECLEN(pTMXTargetRecord) )
        {
          // target record is corrupted, continue with next TM record
          usRc = BTREE_CORRUPTED;
        }
        else
        {
          ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                         pTMXTargetRecord->usClb;
          ulLeftClbLen -= TARGETCLBLEN(pTargetClb); // subtract size of current CLB
        } /* endif */
        usTarget = 1;           //initialize counter

        //loop until correct target is found
        while ( (usTarget < pTmExtIn->usNextTarget) && ulLeftTgtLen && !usRc )
        {
          // position to first target CLB
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
          pTargetClb = (PTMX_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
          ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                         pTMXTargetRecord->usClb;

          // subtract size of current CLB
          if ( ulLeftClbLen >= TARGETCLBLEN(pTargetClb) )
          {
            ulLeftClbLen -= TARGETCLBLEN(pTargetClb); 
          }
          else
          {
            // database is corrupted
            ulLeftClbLen = 0;
            usRc = BTREE_NOT_FOUND;
          } /* endif */ 

          // loop over all target CLBs
          while ( (usTarget < pTmExtIn->usNextTarget) && ulLeftClbLen )
          {
            usTarget++;
            pTargetClb = NEXTTARGETCLB(pTargetClb);
            ulLeftClbLen -= TARGETCLBLEN(pTargetClb);
          } /* endwhile */

          // continue with next target if no match yet
          if ( usTarget < pTmExtIn->usNextTarget )
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
            ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                           pTMXTargetRecord->usClb;
            // subtract size of current CLB
            if ( ulLeftClbLen >= TARGETCLBLEN(pTargetClb) )
            {
              ulLeftClbLen -= TARGETCLBLEN(pTargetClb); 
            }
            else
            {
              // database is corrupted
              ulLeftClbLen = 0;
              usRc = BTREE_NOT_FOUND;
            } /* endif */ 
            usTarget++;
          } /* endif */
        } /* endwhile */

        if ( !usRc && (usTarget == pTmExtIn->usNextTarget) )
        {
          //position at start of target record
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

          //if target record exists
          if ( ulLeftTgtLen && ( RECLEN(pTMXTargetRecord) != 0) )
          {
            //fill out the put structure as output of the extract function
            usRc = FillExtStructure( pTmClb, pTMXTargetRecord,
                                     pTargetClb,
                                     pSourceString, &ulSourceLen,
                                     &pTmExtOut->stTmExt );
            if ( ! usRc )
            {
              //check for another target
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

              //if target exists
              if ( ((RECLEN(pTMXTargetRecord) != 0) && ulLeftTgtLen)
                          ||
                    (ulLeftClbLen != 0) )
              {
                //increase target count and leave tm record key number as before
                pTmExtOut->ulTmKey = pTmExtIn->ulTmKey;
                pTmExtOut->usNextTarget = pTmExtIn->usNextTarget + 1;
              }
              else
              {
                //no more target so get next tm record and initialize target count
                pTmExtOut->ulTmKey = pTmExtIn->ulTmKey +1;
                pTmExtOut->usNextTarget = 1;
              } /* endif */
            } /* endif */
          }
          else
          {
            //no more target so get next tm record and initialize target count
            usRc = BTREE_NOT_FOUND;
          } /* endif */
        }
        else
        {
          //no more target so get next tm record and initialize target count
          usRc = BTREE_NOT_FOUND;
        } /* endif */
      } /* endif */
    }
    else
    {
      //if record is empty, get next tm record and initialize target count
      usRc = BTREE_NOT_FOUND;
    } /* endif */

  }

  // release memory
  UtlAlloc( (PVOID *) &pSourceString, 0L, 0L, NOMSG );


  return usRc;
}

static
USHORT ExtractRecordV5
(
  PTMX_CLB        pTmClb,         //ptr to ctl block struct
  PTMX_RECORD     pTmRecord,
  PTMX_EXT_IN_W   pTmExtIn,
  PTMX_EXT_OUT_W  pTmExtOut,
  ULONG           ulOemCP
)
{
  USHORT usRc = 0;
  PTMX_SOURCE_RECORD pTMXSourceRecord; //ptr to source record
  PTMX_TARGET_RECORD pTMXTargetRecord; //ptr to target record
  PTMX_OLD_TARGET_CLB pTargetClb;      //ptr to target CLB
  ULONG  ulLeftClbLen = 0;             // remaining length of CLB area
  PBYTE  pByte;                        //position ptr
  ULONG  ulSourceLen = 0;              //length of source string
  USHORT usTarget;                     //nr of target records in tm record
  PSZ    pSourceString = NULL;         //pointer to source string
  PBYTE  pSource;                      //position ptr


  UtlAlloc( (PVOID *) &(pSourceString), 0L, (LONG) MAX_SEGMENT_SIZE, NOMSG );

  if ( !pSourceString )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */


  if ( !usRc )
  {
    //check if record is empty
    if ( ( RECLEN(pTmRecord) != 0) &&
         (pTmRecord->usFirstTargetRecord < RECLEN(pTmRecord)) )
    {
      ULONG   ulLeftTgtLen;            // remaining target length
      /****************************************************************/
      /* get length of target block to work with                      */
      /****************************************************************/
      assert( RECLEN(pTmRecord) >= pTmRecord->usFirstTargetRecord );
      ulLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;
      //extract source string
      //position at beginning of source structure in tm record
      pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);

      //move pointer to corresponding position
      pSource = (PBYTE)(pTmRecord+1);
      pSource += pTMXSourceRecord->usSource;

      //calculate length of source string
      ulSourceLen = (RECLEN(pTMXSourceRecord) -
                            sizeof(TMX_SOURCE_RECORD) );

      //copy source string for fill matchtable
      if ( ulSourceLen < MAX_SEGMENT_SIZE )
      {
        //ASCII2UnicodeBuf( pSource, pSourceString, ulSourceLen );
        memcpy( pSourceString, pSource, ulSourceLen );
        pSourceString[ulSourceLen] = EOS;
      }
      else
      {
        usRc = BTREE_CORRUPTED;
      } /* endif */

      if ( !usRc )
      {
        //find target record specified in ExtIn structure

        //move pointer to first target
        pByte = (PBYTE)(pTmRecord);
        pByte += pTmRecord->usFirstTargetRecord;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
        pTargetClb = (PTMX_OLD_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
        if ( pTMXTargetRecord->usClb >= RECLEN(pTMXTargetRecord) )
        {
          // target record is corrupted, continue with next TM record
          usRc = BTREE_CORRUPTED;
        }
        else
        {
          ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                         pTMXTargetRecord->usClb;
          ulLeftClbLen -= sizeof(TMX_OLD_TARGET_CLB); // subtract size of current CLB
        } /* endif */
        usTarget = 1;           //initialize counter

        //loop until correct target is found
        while ( (usTarget < pTmExtIn->usNextTarget) && ulLeftTgtLen && !usRc )
        {
          // position to first target CLB
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
          pTargetClb = (PTMX_OLD_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
          ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                         pTMXTargetRecord->usClb;
          ulLeftClbLen -= sizeof(TMX_OLD_TARGET_CLB); // subtract size of current CLB

          // loop over all target CLBs
          while ( (usTarget < pTmExtIn->usNextTarget) && ulLeftClbLen )
          {
            usTarget++;
            pTargetClb++;
            // just to be on the save side -- avoid problems if someone corrupted our TM
            if ( ulLeftClbLen >= sizeof(TMX_OLD_TARGET_CLB) )
            {
              ulLeftClbLen -= sizeof(TMX_OLD_TARGET_CLB);
            }
            else
            {
                ulLeftClbLen = 0;
            }
          } /* endwhile */

          // continue with next target if no match yet
          if ( usTarget < pTmExtIn->usNextTarget )
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
            pTargetClb = (PTMX_OLD_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
            ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                           pTMXTargetRecord->usClb;
            ulLeftClbLen -= sizeof(TMX_OLD_TARGET_CLB); // subtract size of current CLB
            usTarget++;
          } /* endif */
        } /* endwhile */
        if ( !usRc && (usTarget == pTmExtIn->usNextTarget) )
        {
          //position at start of target record
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

          //if target record exists
          if ( ulLeftTgtLen && ( RECLEN(pTMXTargetRecord) != 0) )
          {
            //fill out the put structure as output of the extract function
            usRc = FillExtStructureV5( pTmClb, pTMXTargetRecord,
                                     pTargetClb,
                                     pSourceString, &ulSourceLen,
                                     &pTmExtOut->stTmExt, ulOemCP );
            if ( ! usRc )
            {
              //check for another target
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

              //if target exists
              if ( ((RECLEN(pTMXTargetRecord) != 0) && ulLeftTgtLen)
                          ||
                    (ulLeftClbLen != 0) )
              {
                //increase target count and leave tm record key number as before
                pTmExtOut->ulTmKey = pTmExtIn->ulTmKey;
                pTmExtOut->usNextTarget = pTmExtIn->usNextTarget + 1;
              }
              else
              {
                //no more target so get next tm record and initialize target count
                pTmExtOut->ulTmKey = pTmExtIn->ulTmKey +1;
                pTmExtOut->usNextTarget = 1;
              } /* endif */
            } /* endif */
          }
          else
          {
            //no more target so get next tm record and initialize target count
            usRc = BTREE_NOT_FOUND;
          } /* endif */
        }
        else
        {
          //no more target so get next tm record and initialize target count
          usRc = BTREE_NOT_FOUND;
        } /* endif */
      } /* endif */
    }
    else
    {
      //if record is empty, get next tm record and initialize target count
      usRc = BTREE_NOT_FOUND;
    } /* endif */

  }

  // release memory
  UtlAlloc( (PVOID *) &pSourceString, 0L, 0L, NOMSG );
  return usRc;
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FillExtStructure fills extract output structure with     |
//|                                    sequential source and target            |
//+----------------------------------------------------------------------------+
//|Description:       Fills the ext structure in sequential order              |
//+----------------------------------------------------------------------------+
//|Function call:  FillExtStructure( PTMX_CLB pTmClb, //ptr to control block   |
//|                         PTMX_TARGET_RECORD pTMXTargetRecord,               |
//|                                             //ptr to target record         |
//|                         PSZ pSourceString,  //ptr to source string         |
//|                         PUSHORT pulSourceLen, //source string length       |
//|                         PTMX_EXT pstExt ) //ptr to extract structure       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PTMX_CLB pTmClb                                          |
//|                   PTMX_TARGET_RECORD pTMXTargetRecord                      |
//|                   PSZ pSourceString                                        |
//|                   PUSHORT pulSourceLen                                     |
//+----------------------------------------------------------------------------+
//|Output parameter:  PTMX_EXT pstExt                                          |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|     position at start of target record                                     |
//|       fill extract stucture                                                |
//|     output extract structure                                               |
// ----------------------------------------------------------------------------+

USHORT FillExtStructure
(
  PTMX_CLB    pTmClb,                  //ptr to control block
  PTMX_TARGET_RECORD pTMXTargetRecord, //ptr to tm target
  PTMX_TARGET_CLB    pTargetClb,       // ptr to current target CLB
  PSZ_W pSourceString,                 //ptr to source string
  PULONG pulSourceLen,                //length of source string
  PTMX_EXT_W pstExt                    //extout ext struct
)
{
  PTMX_TARGET_CLB pTMXTargetClb = NULL;      //ptr to target control block
  PTMX_TAGTABLE_RECORD pTMXTagTableRecord = NULL;   //ptr to tag table record
  PBYTE pByte;                               //position pointer
  BOOL fOK;                                  //success indicator
  USHORT usRc = NO_ERROR;                    //return code
  ULONG  ulTargetLen = 0;                    //length indicator
  PSZ_W pTargetString = NULL;                //pointer to target string

  //allocate pString
  fOK = UtlAlloc( (PVOID *) &(pTargetString), 0L, (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    pByte = (PBYTE)pTMXTargetRecord;
    pByte += pTMXTargetRecord->usSourceTagTable;
    pTMXTagTableRecord = (PTMX_TAGTABLE_RECORD)pByte;

    //add tags to source string if there are any
    if ( RECLEN(pTMXTagTableRecord) > sizeof(TMX_TAGTABLE_RECORD) )
    {
      fOK = AddTagsToStringW( pSourceString, pulSourceLen,
                             (PTMX_TAGTABLE_RECORD)pByte, pstExt->szSource );
    }
    else
    {
      //else copy string
      memcpy( pstExt->szSource, pSourceString, *pulSourceLen * sizeof(CHAR_W) );
      pstExt->szSource[*pulSourceLen] = EOS;
    } /* endif */

    if ( fOK )
    {
      //position at target string
      pByte = (PBYTE)pTMXTargetRecord;
      pByte += pTMXTargetRecord->usTarget;

      //calculate length of target string
      ulTargetLen = pTMXTargetRecord->usClb - pTMXTargetRecord->usTarget;
      fOK = ( ulTargetLen < MAX_SEGMENT_SIZE * sizeof(WCHAR) );
    } /* endif */

    // GQ 2008/06/18 Check if target string data is valid
    if ( fOK )
    {
      BYTE b = *pByte;
      
      if ( (b != 0) && (b != BOCU_COMPRESS) )
      {
        fOK = FALSE; // invalid compression ID
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      //copy target string
        ulTargetLen = EQFCompress2Unicode( pTargetString, pByte, ulTargetLen );
        if ( ulTargetLen < MAX_SEGMENT_SIZE ) {
           pTargetString[ulTargetLen] = EOS;
        } else {
           fOK = FALSE; // invalid string length
        }

      //position at target tag record
      pByte = (PBYTE)pTMXTargetRecord;
      pByte += pTMXTargetRecord->usTargetTagTable;
      pTMXTagTableRecord = (PTMX_TAGTABLE_RECORD)pByte;

      //fill in the tag table name
      NTMGetNameFromID( pTmClb, &pTMXTagTableRecord->usTagTableId,
                        (USHORT)TAGTABLE_KEY,
                        pstExt->szTagTable, NULL );

      //add tags to target string if flag set to true
      if ( (RECLEN(pTMXTagTableRecord) > sizeof(TMX_TAGTABLE_RECORD)) )
      {
        AddTagsToStringW( pTargetString, &ulTargetLen,
                         (PTMX_TAGTABLE_RECORD)pByte, pstExt->szTarget );
      }
      else
      {
        //else copy string
        memcpy( pstExt->szTarget, pTargetString, ulTargetLen * sizeof(CHAR_W));
        pstExt->szTarget[ulTargetLen] = EOS;
      } /* endif */

      //position at target control block
      pTMXTargetClb = pTargetClb;

      // in organize mode preset author and file name field with default
      // values as here the corresponding tables may be corrupted
      if ( pTmClb->usAccessMode & ASD_ORGANIZE )
      {
        strcpy( pstExt->szFileName, OVERFLOW_NAME );
        pstExt->szLongName[0] = EOS;
        strcpy( pstExt->szAuthorName, OVERFLOW_NAME );
      } /* endif */

      //fill in the target file name
      NTMGetNameFromID( pTmClb, &pTMXTargetClb->usFileId, (USHORT)FILE_KEY,
                        pstExt->szFileName, pstExt->szLongName );
      //use overflow name if no document name available
      if ( pstExt->szFileName[0] == EOS )
      {
        strcpy( pstExt->szFileName, OVERFLOW_NAME );
      } /* endif */

      //fill in the target author
      NTMGetNameFromID( pTmClb, &pTMXTargetClb->usAuthorId, (USHORT)AUTHOR_KEY,
                        pstExt->szAuthorName, NULL );

      //fill in the target language
      NTMGetNameFromID( pTmClb, &pTMXTargetClb->usLangId, (USHORT)LANG_KEY,
                        pstExt->szTargetLanguage, NULL );

      //fill in the segment id
      pstExt->ulSourceSegmentId = pTMXTargetClb->ulSegmId;
      //state whether machine translation
      pstExt->usTranslationFlag = pTMXTargetClb->bTranslationFlag ;
      //fill in target time
      pstExt->lTargetTime = pTMXTargetClb->lTime;

      // fill in any segment context info
      pstExt->szContext[0] = 0;
      pstExt->szAddInfo[0] = 0;
      if ( pTMXTargetClb->usAddDataLen >= MAX_ADD_DATA_LEN )
      {
        // target CLB info seems to be corrupted
        fOK = FALSE;
      }
      else if ( pTMXTargetClb->usAddDataLen )
      {
        NtmGetAddData( pTMXTargetClb, ADDDATA_CONTEXT_ID, pstExt->szContext, sizeof(pstExt->szContext) / sizeof(CHAR_W) );
        NtmGetAddData( pTMXTargetClb, ADDDATA_ADDINFO_ID, pstExt->szAddInfo, sizeof(pstExt->szAddInfo) / sizeof(CHAR_W) );
      } /* endif */
    } /* endif */

    usRc = fOK ? NO_ERROR : BTREE_CORRUPTED;
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pTargetString, 0L, 0L, NOMSG );

  return( usRc );
}

static
USHORT FillExtStructureV5
(
  PTMX_CLB    pTmClb,                  //ptr to control block
  PTMX_TARGET_RECORD pTMXTargetRecord, //ptr to tm target
  PTMX_OLD_TARGET_CLB    pTargetClb,   // ptr to current target CLB
  PSZ pSourceString,                   //ptr to source string
  PULONG pulSourceLen,                //length of source string
  PTMX_EXT_W pstExt,                    //extout ext struct
  ULONG      ulOemCP
)
{
  PTMX_OLD_TARGET_CLB pTMXTargetClb = NULL;      //ptr to target control block
  PTMX_TAGTABLE_RECORD pTMXTagTableRecord = NULL;//ptr to tag table record
  PBYTE pByte;                               //position pointer
  BOOL fOK;                                  //success indicator
  USHORT usRc = NO_ERROR;                    //return code
  ULONG ulTargetLen = 0;                    //length indicator
  PSZ pTargetString = NULL;                  //pointer to target string
  PSZ pTemp;

  //allocate pString
  fOK = UtlAlloc( (PVOID *) &(pTargetString), 0L, (LONG)MAX_SEGMENT_SIZE * 2, NOMSG );

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    pTemp = pTargetString + MAX_SEGMENT_SIZE;
    pByte = (PBYTE)pTMXTargetRecord;
    pByte += pTMXTargetRecord->usSourceTagTable;
    pTMXTagTableRecord = (PTMX_TAGTABLE_RECORD)pByte;

    //add tags to source string if there are any
    if ( RECLEN(pTMXTagTableRecord) > sizeof(TMX_TAGTABLE_RECORD) )
    {
      fOK = AddTagsToString( pSourceString, pulSourceLen,
                             (PTMX_TAGTABLE_RECORD)pByte, pTemp );
      ASCII2Unicode( pTemp, pstExt->szSource, ulOemCP );
    }
    else
    {
      //else copy string
      ULONG ulLen;
      ulLen = ASCII2UnicodeBuf( pSourceString, pstExt->szSource, *pulSourceLen, ulOemCP );
      pstExt->szSource[ulLen] = EOS;
//      memcpy( pstExt->szSource, pSourceString, *pulSourceLen );
    } /* endif */

    if ( fOK )
    {
      //position at target string
      pByte = (PBYTE)pTMXTargetRecord;
      pByte += pTMXTargetRecord->usTarget;

      //calculate length of target string
      ulTargetLen = pTMXTargetRecord->usClb - pTMXTargetRecord->usTarget;
      fOK = ( ulTargetLen < MAX_SEGMENT_SIZE );
    } /* endif */

    if ( fOK )
    {
      //copy target string
      memcpy( pTargetString, pByte, ulTargetLen );
     pTargetString[ulTargetLen] = EOS;
      //position at target tag record
      pByte = (PBYTE)pTMXTargetRecord;
      pByte += pTMXTargetRecord->usTargetTagTable;
      pTMXTagTableRecord = (PTMX_TAGTABLE_RECORD)pByte;

      //fill in the tag table name
      NTMGetNameFromID( pTmClb, &pTMXTagTableRecord->usTagTableId,
                        (USHORT)TAGTABLE_KEY,
                        pstExt->szTagTable, NULL );

      //add tags to target string if flag set to true
      if ( (RECLEN(pTMXTagTableRecord) > sizeof(TMX_TAGTABLE_RECORD)) )
      {
        *pTemp = EOS;
        AddTagsToString( pTargetString, &ulTargetLen,
                         (PTMX_TAGTABLE_RECORD)pByte, pTemp );
        ASCII2Unicode( pTemp, pstExt->szTarget, ulOemCP );
      }
      else
      {
        //else copy string
//        memcpy( pstExt->szTarget, pTargetString, ulTargetLen );
        ULONG ulLen;
        ulLen = ASCII2UnicodeBuf( pTargetString, pstExt->szTarget, ulTargetLen, ulOemCP );
        pstExt->szTarget[ulLen] = EOS;
      } /* endif */

      //position at target control block
      pTMXTargetClb = pTargetClb;

      //in organize mode preset author and file name field with default
      //values as here the corresponding tables may be corrupted
      if ( pTmClb->usAccessMode & ASD_ORGANIZE )
      {
        strcpy( pstExt->szFileName, OVERFLOW_NAME );
        pstExt->szLongName[0] = EOS;
        strcpy( pstExt->szAuthorName, OVERFLOW_NAME );
      } /* endif */

      //fill in the target file name
      NTMGetNameFromID( pTmClb, &pTMXTargetClb->usFileId, (USHORT)FILE_KEY,
                        pstExt->szFileName, pstExt->szLongName );
      //use overflow name if no document name available
      if ( pstExt->szFileName[0] == EOS )
      {
        strcpy( pstExt->szFileName, OVERFLOW_NAME );
      } /* endif */

      //fill in the target author
      NTMGetNameFromID( pTmClb, &pTMXTargetClb->usAuthorId, (USHORT)AUTHOR_KEY,
                        pstExt->szAuthorName, NULL );

      //fill in the target language
      NTMGetNameFromID( pTmClb, &pTMXTargetClb->usLangId, (USHORT)LANG_KEY,
                        pstExt->szTargetLanguage, NULL );

      //fill in the segment id
      pstExt->ulSourceSegmentId = pTMXTargetClb->usSegmId;
      //state whether machine translation
      pstExt->usTranslationFlag  = pTMXTargetClb->bMT;
      //fill in target time
      pstExt->lTargetTime = pTMXTargetClb->lTime;
    } /* endif */

    usRc = fOK ? NO_ERROR : BTREE_CORRUPTED;
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pTargetString, 0L, 0L, NOMSG );

  return( usRc );
}
