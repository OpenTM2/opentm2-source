//+----------------------------------------------------------------------------+
//|EQFNTOP.C                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
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
#include <EQFEVENT.H>             // event logging

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXOpen     opens   data and index files                |
//+----------------------------------------------------------------------------+
//|Description:       Opens   qdam data and index files                        |
//+----------------------------------------------------------------------------+
//|Function call:  TmtXOpen  ( PTMX_OPEN_IN pTmOpenIn, //input struct          |
//|                            PTMX_OPEN_OUT pTmOpenOut ) //output struct      |
//+----------------------------------------------------------------------------+
//|Input parameter: PTMX_OPEN_IN pTmOpenIn     input structure                 |
//+----------------------------------------------------------------------------+
//|Output parameter: PTMX_OPEN_OUT pTmOpenOut   output structure               |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes: identical to return code in open out structure                 |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//| allocate control block     - NTM_CLB                                       |
//| allocate tag table, file name, author and compact area blocks in control   |
//|  block                                                                     |
//| open tm data file                                                          |
//| get signature record contents and store in control block                   |
//| get tag table, author, file name and compact area record from tm data file |
//|  and add to control block                                                  |
//| open index file                                                            |
//|                                                                            |
//| return open out structure                                                  |
// ----------------------------------------------------------------------------+

USHORT TmtXOpen
(
  PTMX_OPEN_IN pTmOpenIn,    //ptr to input struct
  PTMX_OPEN_OUT pTmOpenOut   //ptr to output struct
)
{
  BOOL fOK;                      //success indicator
  PTMX_CLB pTmClb = NULL;        //pointer to control block
  USHORT usRc = NO_ERROR;        //return value
  USHORT usRc1 = NO_ERROR;       //return value
  ULONG  ulLen;                  //length indicator

  DEBUGEVENT( TMTXOPEN_LOC, FUNCENTRY_EVENT, 0 );

  //allocate control block
  fOK = UtlAlloc( (PVOID *) &(pTmClb), 0L, (LONG)sizeof( TMX_CLB ), NOMSG );

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    //allocate table records
    //check access
    if ( pTmOpenIn->stTmOpen.usAccess == READONLYACCESS )
    {
      PSZ pszExt = strrchr( pTmOpenIn->stTmOpen.szDataName, DOT );
      if ( (pszExt != NULL) && (_stricmp( pszExt, EXT_OF_SHARED_MEM) == 0) )
      {
        pTmClb->usAccessMode = ASD_GUARDED | ASD_SHARED | ASD_READONLY;
        pTmClb->fShared = TRUE;
      }
      else
      {
        pTmClb->usAccessMode = ASD_GUARDED | ASD_READONLY;
      } /* endif */
    }
    else if ( pTmOpenIn->stTmOpen.usAccess != NONEXCLUSIVE )
    {
      PSZ pszExt = strrchr( pTmOpenIn->stTmOpen.szDataName, DOT );
      pTmClb->usAccessMode = ASD_GUARDED | ASD_LOCKED;
      if ( (pszExt != NULL) && (_stricmp( pszExt, EXT_OF_SHARED_MEM) == 0) )
      {
        // avoid error if open flag in header of shared TMs is set
        pTmClb->usAccessMode |= ASD_NOOPENCHECK;
      } /* endif */
    }
    else
    {
      PSZ pszExt = strrchr( pTmOpenIn->stTmOpen.szDataName, DOT );
      if ( (pszExt != NULL) && (_stricmp( pszExt, EXT_OF_SHARED_MEM) == 0) )
      {
        pTmClb->usAccessMode = ASD_GUARDED | ASD_SHARED;
        pTmClb->fShared = TRUE;
      }
      else
      {
        pTmClb->usAccessMode = ASD_GUARDED;
      } /* endif */
    } /* endif */
    if ( pTmOpenIn->stTmOpen.usAccess == FOR_ORGANIZE )
    {
        pTmClb->usAccessMode |= ASD_ORGANIZE;
    } /* endif */

    //call open function for data file
    usRc1 = EQFNTMOpen( pTmOpenIn->stTmOpen.szDataName,
                        (USHORT)(pTmClb->usAccessMode | ASD_FORCE_WRITE),
                        &pTmClb->pstTmBtree );
    if ( (usRc1 == NO_ERROR) || (usRc1 == BTREE_CORRUPTED) )
    {
      //get signature record and add to control block
      USHORT usLen = sizeof( TMX_SIGN );
      usRc = EQFNTMSign( pTmClb->pstTmBtree, (PCHAR) &(pTmClb->stTmSign), &usLen );

      // do on-spot conversion for version 6 memories
      if ( (usRc == NO_ERROR ) && (pTmClb->stTmSign.bMajorVersion == TM_MAJ_VERSION_6) )
      {
        EQFNTMClose( &pTmClb->pstTmBtree );
        MemConvertMem( pTmOpenIn->stTmOpen.szDataName );
        usRc = EQFNTMOpen( pTmOpenIn->stTmOpen.szDataName, (USHORT)(pTmClb->usAccessMode | ASD_FORCE_WRITE), &pTmClb->pstTmBtree );
        usLen = sizeof( TMX_SIGN );
        if ( !usRc1 ) usRc = EQFNTMSign( pTmClb->pstTmBtree, (PCHAR) &(pTmClb->stTmSign), &usLen );
      } /* endif */

      if ( usRc == NO_ERROR )
      {
        if ( pTmClb->stTmSign.bMajorVersion > TM_MAJ_VERSION )
        {
          usRc = ERROR_VERSION_NOT_SUPPORTED;
        }
        else if ( pTmClb->stTmSign.bMajorVersion < TM_MAJ_VERSION )
        {
          usRc = VERSION_MISMATCH;
        } /* endif */
      }
      else if ( pTmClb->usAccessMode & ASD_ORGANIZE )
      {
        // allow to continue even if signature record is corrupted
        usRc = NO_ERROR;
      } /* endif */

      if ( (usRc == NO_ERROR) || (usRc == VERSION_MISMATCH) )
      {
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc;

          ulLen =  MAX_COMPACT_SIZE-1;
          //get compact area and add to control block
          usTempRc = EQFNTMGet( pTmClb->pstTmBtree, COMPACT_KEY, (PCHAR)pTmClb->bCompact, &ulLen );

          // in organize mode allow continue if compact area is corrupted
          if ( (usTempRc != NO_ERROR) && (usTempRc != VERSION_MISMATCH) )
          {
            usTempRc = BTREE_CORRUPTED;
          } /* endif */

          if ( usTempRc == BTREE_CORRUPTED )
          {
            memset( pTmClb->bCompact, 0, sizeof(pTmClb->bCompact) );
          } /* endif */
          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */


        //get languages and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc;

          DEBUGEVENT( TMTXOPEN_LOC, STATE_EVENT, 2 );

          //call to obtain exact length of record
          ulLen = 0;
          usTempRc = NTMLoadNameTable( pTmClb, LANG_KEY,
                                       (PBYTE *)&pTmClb->pLanguages, &ulLen );

          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( pTmClb->pLanguages == NULL )
          {
            usTempRc = TM_FILE_SCREWED_UP; // cannot continue if no languages
          }
          else
          {
            usTempRc = NTMCreateLangGroupTable( pTmClb );
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get file names and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc;

          usTempRc = NTMLoadNameTable( pTmClb, FILE_KEY,
                                       (PBYTE *)&pTmClb->pFileNames,
                                       &ulLen );

          // in organize mode allow continue if file name area is corrupted
          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( usTempRc == BTREE_CORRUPTED )
          {
            if( UtlAlloc( (PVOID *)&(pTmClb->pFileNames),
                          0L, (LONG)(TMX_TABLE_SIZE), NOMSG ) )
            {
              pTmClb->pFileNames->ulAllocSize = TMX_TABLE_SIZE;
            }
            else
            {
              usTempRc = ERROR_NOT_ENOUGH_MEMORY;
            } /* endif */
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get authors and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc;

          usTempRc = NTMLoadNameTable( pTmClb, AUTHOR_KEY,
                                       (PBYTE *)&pTmClb->pAuthors, &ulLen );

          // in organize mode allow continue if author name area is corrupted
          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( usTempRc == BTREE_CORRUPTED )
          {
            if( UtlAlloc( (PVOID *)&(pTmClb->pAuthors),
                          0L, (LONG)(TMX_TABLE_SIZE), NOMSG ) )
            {
              pTmClb->pAuthors->ulAllocSize = TMX_TABLE_SIZE;
            }
            else
            {
              usTempRc = ERROR_NOT_ENOUGH_MEMORY;
            } /* endif */
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get tag tables and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc;

          usTempRc = NTMLoadNameTable( pTmClb, TAGTABLE_KEY,
                                       (PBYTE *)&pTmClb->pTagTables, &ulLen );


          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( usTempRc == BTREE_CORRUPTED )
          {
            usTempRc = TM_FILE_SCREWED_UP; // cannot continue if no tag tables
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get long document name table
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc;

          // error in memory allocations will force end of open!
          usTempRc = NTMCreateLongNameTable( pTmClb );
          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;

          // now read any long name table from database, if there is
          // none (=older TM before TOP97) write our empty one to the
          // Translation Memory
          if ( usTempRc == NO_ERROR )
          {
            usTempRc = NTMReadLongNameTable( pTmClb );
            switch ( usTempRc)
            {
              case ERROR_NOT_ENOUGH_MEMORY:
                // leave return code as is
                break;
              case NO_ERROR:
                // O.K. no problems at all
                break;
              case BTREE_NOT_FOUND :
                // no long name tabel yet,create one ...
                usTempRc = NTMWriteLongNameTable( pTmClb );
                break;
              default:
                // read of long name table failed, assume TM is corrupted
                if ( pTmOpenIn->stTmOpen.usAccess == FOR_ORGANIZE )
                {
                } /* endif */
                usTempRc = BTREE_CORRUPTED;
                break;
            } /* endswitch */
          } /* endif */

          // just for debugging purposes: check if short and long name table have the same numbe rof entries
          if ( usTempRc == NO_ERROR )
          {
            if ( pTmClb->pLongNames->ulEntries != pTmClb->pFileNames->ulMaxEntries )
            {
              // this is strange...
              int iSetBreakPointHere = 1;
            }
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */
      } /* endif */

      //add threshold to control block
      pTmClb->usThreshold = pTmOpenIn->stTmOpen.usThreshold;

      // get inital update counters
      if ( !usRc && pTmClb->fShared )
      {
        USHORT usTempRc;

        usTempRc = EQFNTMGetUpdCounter( pTmClb->pstTmBtree,
                                        pTmClb->alUpdCounter,
                                        0, MAX_UPD_COUNTERS );

        if ( usTempRc != NO_ERROR )
        {
          usRc = usTempRc;
        } /* endif */
      } /* endif */

      if ( (usRc == NO_ERROR) ||
           (usRc == BTREE_CORRUPTED) ||
           (usRc == VERSION_MISMATCH) )
      {
        //call open function for index file
        USHORT usIndexRc;
        usIndexRc = EQFNTMOpen( pTmOpenIn->stTmOpen.szIndexName,
                                pTmClb->usAccessMode, &pTmClb->pstInBtree );
        if ( usIndexRc != NO_ERROR )
        {
          usRc = usIndexRc;
        } /* endif */
      } /* endif */

      if ( usRc == NO_ERROR)
      {
        // no error during open of index, use return code of EQFNTMOPEN for
        // data file
        usRc = usRc1;
      }
      else
      {
        // open of index failed or index is corrupted so leave usRc as-is;
        // data file will be closed in cleanup code below
      } /* endif */
    }
    else
    {
      // error during open of data file, use return code of EQFNTMOPEN for
      // data file
      usRc = usRc1;
    } /* endif */

  } /* endif */

  /********************************************************************/
  /* Ensure that all required data areas have been loaded and the     */
  /* database files have been opened                                  */
  /********************************************************************/
  if ( (usRc == NO_ERROR) ||
       (usRc == BTREE_CORRUPTED) ||
       (usRc == VERSION_MISMATCH) )
  {
    if ( (pTmClb->pLanguages == NULL) ||
         (pTmClb->pAuthors   == NULL) ||
         (pTmClb->pTagTables == NULL) ||
         (pTmClb->pFileNames == NULL) ||
         (pTmClb->pstTmBtree == NULL) ||
         (pTmClb->pstInBtree == NULL) )
    {
      usRc = TM_FILE_SCREWED_UP;
    } /* endif */
  } /* endif */

  if ( (usRc != NO_ERROR) &&
       (usRc != BTREE_CORRUPTED) &&
       (usRc != VERSION_MISMATCH) )
  {
    //release allocated memory
    UtlAlloc( (PVOID *) &(pTmClb->pLanguages), 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &(pTmClb->pAuthors), 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &(pTmClb->pTagTables), 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &(pTmClb->pFileNames), 0L, 0L, NOMSG );
    if ( pTmClb->pstTmBtree != NULL ) EQFNTMClose( &pTmClb->pstTmBtree );
    if ( pTmClb->pstInBtree != NULL ) EQFNTMClose( &pTmClb->pstInBtree );
    NTMDestroyLongNameTable( pTmClb );
    UtlAlloc( (PVOID *) &pTmClb, 0L, 0L, NOMSG );
  } /* endif */

  //set out values
  pTmOpenOut->pstTmClb = pTmClb;
  pTmOpenOut->stPrefixOut.usLengthOutput = sizeof( TMX_OPEN_OUT );
  pTmOpenOut->stPrefixOut.usTmtXRc = usRc;

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMTXOPEN_LOC, ERROR_EVENT, usRc );
  } /* endif */

  DEBUGEVENT( TMTXOPEN_LOC, FUNCEXIT_EVENT, 0 );

  return( usRc );
}



