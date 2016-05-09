/*! \file
	Description: Internal functions used by EQFQDAM.C For details see description in EQFQDAM.C

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_TM               // general Transl. Memory functions
#include <eqf.h>                  // General Translation Manager include file

#include <eqfqdami.h>             // Private QDAM defines
#include <eqfcmpr.h>              // defines for compression/expand...
#include <eqfchtbl.h>             // character tables
#include <time.h>
#include "eqfevent.h"                  // event logging facility

#if defined(ASDLOGGING)
 extern FILE *hAsdLog;
 #define ASDLOG()                \
    if ( hAsdLog )               \
       fprintf( hAsdLog, "QDAM: %s, %d \n",__FILE__, __LINE__ );    \
    else                         \
       DosBeep( 1200, 200 );
#else
 #define ASDLOG()
#endif


SHORT QDAMCaseCompare
(
    PVOID pvBT,                        // pointer to tree structure
    PVOID pKey1,                       // pointer to first key
    PVOID pKey2,                       // pointer to second key
    BOOL  fIgnorePunctuation           // ignore punctuation flag
);

#define INC_READ_COUNT
#define INC_CACHED_COUNT
#define INC_CACHED_READ_COUNT
#define INC_WRITE_COUNT
#define INC_REAL_READ_COUNT
#define INC_REAL_WRITE_COUNT

static BTREEHEADRECORD header; // Static buffer for database header record

/**********************************************************************/
/* 'Magic word' for record containing locked terms                    */
/**********************************************************************/
static CHAR szLOCKEDTERMSKEY[] = "0x010x020x03LOCKEDTERMS0x010x020x030x040x050x060x070x080x09";
static CHAR_W szLOCKEDTERMSKEYW[] = L"0x010x020x03LOCKEDTERMS0x010x020x030x040x050x060x070x080x09";
static CHAR szLockRec[MAX_LOCKREC_SIZE]; // buffer for locked terms


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMUpdateList - Update In core List
//------------------------------------------------------------------------------
// Function call:     QDAMUpdateList( PBTREE, PBTREERECORD )
//
//------------------------------------------------------------------------------
// Description:       update the list of filled records in cache
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The B-tree
//                    PBTREERECORD       The buffer to be updated
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     set record pointer to DataRecList
//                    while !fFound and i < MAX_LIST
//                      if record found then
//                        set fFound
//                        set offset to new filled mark
//                      else
//                        increase index i and point to next record
//                      endif
//                    endwhile
//                    if not found yet then
//                      check for a free slot where we can insert our record
//                      - reset record pointer to start (DataRecList )
//                      - while !fFound and i<MAX_LIST
//                          if record number != 0  then
//                            increase index and record pointer
//                          else
//                            set fFound
//                            fill in the new data
//                          endif
//                        endwhile
//                    endif
//                    if not found yet then
//                      check for the slot which is filled most
//                      - reset record pointer to start
//                      - loop thru list of stored records and find record
//                        which is filled most
//                      - set pRecTemp to the selected slot
//                      - store new values for offset and record number
//                    endif
//                    return sRc
//------------------------------------------------------------------------------
VOID  QDAMUpdateList_V2
(
   PBTREE   pBTIda,
   PBTREEBUFFER_V2 pRecord
)
{
  USHORT   i;                             // index
  USHORT   usNumber;                      // number of record
  BOOL     fFound = FALSE;
  USHORT   usFilled;                      // length of slot filled
  USHORT   usMaxRec = 0;                  // record filled at most
  PRECPARAM  pRecTemp;                    // record descr. structure
  PBTREEGLOB  pBT = pBTIda->pBTree;

  usNumber = pRecord->contents.header.usNum;

  pRecTemp = pBT->DataRecList;

  i = 0;
  //  find slot in list
  while ( !fFound && i < MAX_LIST )
  {
     if ( pRecTemp->usNum != usNumber )
     {
        pRecTemp++;
        i++;
     }
     else                                     // data will fit
     {
        fFound = TRUE;
        pRecTemp->usOffset = pRecord->contents.header.usFilled;
     } /* endif */
  } /* endwhile */

  //  if not found yet check for a free slot now where we can insert our record
  if ( !fFound )
  {
     pRecTemp = pBT->DataRecList;
     i = 0;
     while ( !fFound && i < MAX_LIST )
     {
        if ( pRecTemp->usNum != 0 )
        {
           pRecTemp++;
           i++;
        }
        else
        {
           fFound = TRUE;
           // fill in the new data
           pRecTemp->usOffset = pRecord->contents.header.usFilled;
           pRecTemp->usNum = pRecord->contents.header.usNum;
        } /* endif */
     } /* endwhile */
  } /* endif */

  //  if not found yet check for the slot which is filled most
  if ( !fFound )
  {
     pRecTemp = pBT->DataRecList;
     usFilled = 0;
     for ( i=0 ; i < MAX_LIST ;i++ )
     {
        if ( pRecTemp->usOffset > usFilled)
        {
           usMaxRec = i;
           usFilled = pRecTemp->usOffset;
        } /* endif */
        pRecTemp++;                    // next record in list
     } /* endfor */
     // set pRecTemp to the selected slot
     pRecTemp = pBT->DataRecList + usMaxRec;

     if ( usFilled > pRecord->contents.header.usFilled )
     {
        pRecTemp->usOffset = pRecord->contents.header.usFilled;
        pRecTemp->usNum = pRecord->contents.header.usNum;
     } /* endif */
  } /* endif */

  return;
}

VOID  QDAMUpdateList_V3
(
   PBTREE   pBTIda,
   PBTREEBUFFER_V3 pRecord
)
{
  USHORT   i;                             // index
  USHORT   usNumber;                      // number of record
  BOOL     fFound = FALSE;
  USHORT   usFilled;                      // length of slot filled
  USHORT   usMaxRec = 0;                  // record filled at most
  PRECPARAM  pRecTemp;                    // record descr. structure
  PBTREEGLOB  pBT = pBTIda->pBTree;

  usNumber = pRecord->contents.header.usNum;

  pRecTemp = pBT->DataRecList;

  i = 0;
  //  find slot in list
  while ( !fFound && i < MAX_LIST )
  {
     if ( pRecTemp->usNum != usNumber )
     {
        pRecTemp++;
        i++;
     }
     else                                     // data will fit
     {
        fFound = TRUE;
        pRecTemp->usOffset = pRecord->contents.header.usFilled;
     } /* endif */
  } /* endwhile */

  //  if not found yet check for a free slot now where we can insert our record
  if ( !fFound )
  {
     pRecTemp = pBT->DataRecList;
     i = 0;
     while ( !fFound && i < MAX_LIST )
     {
        if ( pRecTemp->usNum != 0 )
        {
           pRecTemp++;
           i++;
        }
        else
        {
           fFound = TRUE;
           // fill in the new data
           pRecTemp->usOffset = pRecord->contents.header.usFilled;
           pRecTemp->usNum = pRecord->contents.header.usNum;
        } /* endif */
     } /* endwhile */
  } /* endif */

  //  if not found yet check for the slot which is filled most
  if ( !fFound )
  {
     pRecTemp = pBT->DataRecList;
     usFilled = 0;
     for ( i=0 ; i < MAX_LIST ;i++ )
     {
        if ( pRecTemp->usOffset > usFilled)
        {
           usMaxRec = i;
           usFilled = pRecTemp->usOffset;
        } /* endif */
        pRecTemp++;                    // next record in list
     } /* endfor */
     // set pRecTemp to the selected slot
     pRecTemp = pBT->DataRecList + usMaxRec;

     if ( usFilled > pRecord->contents.header.usFilled )
     {
        pRecTemp->usOffset = pRecord->contents.header.usFilled;
        pRecTemp->usNum = pRecord->contents.header.usNum;
     } /* endif */
  } /* endif */

  return;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFreeFromList    Free Record from List in cache
//------------------------------------------------------------------------------
// Function call:     QDAMFreeFromList( PRECPARAM, PBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       delete the record from the list maintained in cache
//
//------------------------------------------------------------------------------
// Parameters:        PRECPARAM          pointer to record parameter
//                    PBTREERECORD       The buffer to be updated
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     while !fFound and i < MAX_LIST
//                      if record found
//                        set fFound
//                        reset offset and number in record structure
//                      else
//                        increase index i and point to next record
//                      endif
//                    endwhile
//------------------------------------------------------------------------------
VOID  QDAMFreeFromList_V2
(
   PRECPARAM  pRecTemp,
   PBTREEBUFFER_V2 pRecord
)
{
  USHORT   i;                             // index
  USHORT   usNumber;                      // number of record
  BOOL     fFound = FALSE;

  usNumber = pRecord->contents.header.usNum;

  i = 0;
  //  find slot in list
  while ( !fFound && i < MAX_LIST )
  {
     if ( pRecTemp->usNum != usNumber )
     {
        pRecTemp++;
        i++;
     }
     else                                     // data record found
     {
        fFound = TRUE;
        pRecTemp->usNum = 0;
        pRecTemp->usOffset = 0;
     } /* endif */
  } /* endwhile */

  return ;
}

VOID  QDAMFreeFromList_V3
(
   PRECPARAM  pRecTemp,
   PBTREEBUFFER_V3 pRecord
)
{
  USHORT   i;                             // index
  USHORT   usNumber;                      // number of record
  BOOL     fFound = FALSE;

  usNumber = pRecord->contents.header.usNum;

  i = 0;
  //  find slot in list
  while ( !fFound && i < MAX_LIST )
  {
     if ( pRecTemp->usNum != usNumber )
     {
        pRecTemp++;
        i++;
     }
     else                                     // data record found
     {
        fFound = TRUE;
        pRecTemp->usNum = 0;
        pRecTemp->usOffset = 0;
     } /* endif */
  } /* endwhile */

  return ;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMReadRecord   Read record
//------------------------------------------------------------------------------
// Function call:     QDAMReadRecord( PBTREE, USHORT, PPBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       read the requested record either from cache or from disk
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The B-tree to read
//                    USHORT             number of buffer to read
//                    PPBTREERECORD      The buffer read
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_LOOKUPTABLE_TOO_SMALL file is too big for l.table
//                    BTREE_LOOKUPTABLE_NULL      ptr to lookup table is NULL
//                    BTREE_LOOKUPTABLE_CORRUPTED lookup table is corrupted
//------------------------------------------------------------------------------
// Side effects:      The read record routine is one of the most performance
//                    critical routines in the module.
//
//------------------------------------------------------------------------------
// Concept:
//
//   AccessCounter  Record       LookupTable       Memory
//       Table      Number
//     +-------+                  +-------+
//         0           0             NULL
//     +-------+                  +-------+       +--------------------------+
//        700          1             ptr  +------>  Buffer contains Record 1
//     +-------+                  +-------+       +--------------------------+
//         0           2             NULL
//     +-------+                  +-------+
//         .                          .
//         .                          .
//     +-------+                  +-------+       +--------------------------+
//        200          i             ptr  +------>  Buffer contains Record i
//     +-------+                  +-------+       +--------------------------+
//         .                          .
//         .                          .
//     +-------+  pBT->usNumberOf +-------+
//         0       LookupEntries     NULL
//     +-------+                  +-------+
//
//  The lookup table is a dynamically growing array of ptrs to BTREEBUFFER.
//  If the entry for record i is NULL record i isn't buffered in memory.
//  If the rec.num. of the record to read is > pBT->usNumberOfLookupEntries
//  lookup and access counter table will be resized.
//
//  The access counter table is a dynamically growing array of unsigned long.
//  On every read access of record i the entry for record i is increased by
//  ACCESSBONUSPOINTS.
//
//  From time to time (every MAX_READREC_CALLS calls to QDAMReadRecord)
//  unlocked records which aren't read very often ( accesscounter <
//  MAX_READREC_CALLS ) will be written to disk (if fNeedToWrite is set) and
//  the buffer that contains the record will be freed.
//
//------------------------------------------------------------------------------
// Function flow:
//
//     initialize return code (sRc) and pointer to record
//
//     if rec.num. >= MAX_NUMBER_OF_LOOKUP_ENTRIES (l.table would exceed 64kB)
//        or ptr. to lookup- or access-counter-table is NULL
//         set appropriate return code
//     else
//        if rec.num >= number of lookup table entries
//           resize lookup and access counter table
//           and set appropriate return code
//
//     if no error occured
//        if record is already in memory
//          set pointer to it
//        else
//          read record from disk (QDAMReadRecordFromDisk)
//            and set appropriate return code
//          initialize access counter (set it to 0)
//        endif
//
//     if no error occured
//        if #calls to QDAMReadRecord >= MAX_READREC_CALLS
//           write unlocked records with access counter < MAX_READREC_CALLS
//           to disk (if fNeedToWrite is set) and free allocated buffer
//           set pBT->ulReadRecCalls to 0
//        else
//           increment pBT->ulReadRecCalls
//
//     if no error occured
//        increase access counter of the record read by ACCESSBONUSPOINTS
//
//     return sRc
//------------------------------------------------------------------------------
SHORT  QDAMReadRecord_V2
(
   PBTREE  pBTIda,
   USHORT  usNumber,
   PBTREEBUFFER_V2 * ppReadBuffer,
   BOOL    fNewRec
)
{
  USHORT   i;
  SHORT    sRc = 0;                  // return code
  BOOL     fMemOK = FALSE;
  PBTREEGLOB    pBT = pBTIda->pBTree;
  PLOOKUPENTRY_V2  pLEntry;
  PACCESSCTRTABLEENTRY  pACTEntry;

  DEBUGEVENT2( QDAMREADRECORD_LOC, FUNCENTRY_EVENT, usNumber, DB_GROUP, NULL );

  *ppReadBuffer = NULL;
  INC_READ_COUNT;

  /********************************************************************/
  /* If Lookup-Table is too small allocate additional memory for it   */
  /********************************************************************/
  if ( usNumber >= MAX_NUMBER_OF_LOOKUP_ENTRIES )
  {
    /* There is no room for this record number in the lookup table */
    sRc = BTREE_LOOKUPTABLE_TOO_SMALL;
  }
  else if ( !pBT->LookupTable_V2 || !pBT->AccessCtrTable )
  {
    sRc = BTREE_LOOKUPTABLE_NULL;
  }
  else
  {
    if ( usNumber >= pBT->usNumberOfLookupEntries )
    {
      /* The lookup-table entry for the record to read doesn't exist */
      /* Reallocate memory for LookupTable and AccessCounterTable */
      fMemOK = UtlAlloc( (PVOID *)&pBT->LookupTable_V2,
              (LONG) pBT->usNumberOfLookupEntries * sizeof(LOOKUPENTRY_V2),
              (LONG) (usNumber + 10) * sizeof(LOOKUPENTRY_V2), NOMSG );
      if ( fMemOK==TRUE )
      {
        fMemOK = UtlAlloc( (PVOID *)&pBT->AccessCtrTable,
                (LONG) pBT->usNumberOfLookupEntries * sizeof(ACCESSCTRTABLEENTRY),
                (LONG) (usNumber + 10) * sizeof(ACCESSCTRTABLEENTRY), NOMSG );
        if ( fMemOK==TRUE )
        {
          pBT->usNumberOfLookupEntries = usNumber + 10;
        }
        else
        {
          sRc = BTREE_NO_ROOM;
        } /* endif */
      }
      else
      {
        sRc = BTREE_NO_ROOM;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    /******************************************************************/
    /* If record is in memory set ppReadBuffer                        */
    /* else call QDAMReadRecordFromDisk to read it into memory        */
    /******************************************************************/
    pLEntry = pBT->LookupTable_V2 + usNumber;

    if ( pLEntry->pBuffer )
    {
      /* Safety-Check: is rec.number = number of rec. to read ? */
      if ( (pLEntry->pBuffer)->usRecordNumber == usNumber )
      {
        /* Record is already in memory */
        *ppReadBuffer = pLEntry->pBuffer;
      }
      else
      {
        /* This should never occur ! */
        sRc = BTREE_LOOKUPTABLE_CORRUPTED;
      } /* endif */
    }
    else
    {
      /* Record isn't in memory -> read it from disk */
      sRc =  QDAMReadRecordFromDisk_V2( pBTIda, usNumber, ppReadBuffer, fNewRec );
      pACTEntry=pBT->AccessCtrTable + usNumber;
      pACTEntry->ulAccessCounter = 0L;
    } /* endif */
  } /* endif */

  /****************************************************************************/
  /* If #calls of QDAMReadRecord is greater than MAX_READREC_CALLS:           */
  /*   write unlocked records with access counter < MAX_READREC_CALLS to disk */
  /*   (if fNeedToWrite is set) and free the allocated memory                 */
  /*   set #calls of QDAMReadRecord = 0                                       */
  /* else increment #calls of QDAMReadRecord                                  */
  /****************************************************************************/
  if ( !sRc )
  {
    if ( pBT->ulReadRecCalls >= MAX_READREC_CALLS )
    {
      for ( i=0; !sRc && (i < pBT->usNumberOfLookupEntries); i++ )
      {
        pLEntry = pBT->LookupTable_V2 + i;
        pACTEntry = pBT->AccessCtrTable + i;
        if ( pLEntry->pBuffer && !((pLEntry->pBuffer)->fLocked) && (i!=usNumber)
             && (pACTEntry->ulAccessCounter<MAX_READREC_CALLS) )
        {
            /* write buffer and free allocated space */
            if ( (pLEntry->pBuffer)->fNeedToWrite )
            {
              if ( (pBT->usOpenFlags & ASD_SHARED) && !pBTIda->fPhysLock )
              {
                // this condition should NEVER occur for shared resources
                // as write is only allowed if the database has been
                // previously locked
                ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 1, DB_GROUP, NULL );
              #ifdef _DEBUG
                WinMessageBox( HWND_DESKTOP,
                   (HWND)UtlQueryULong( QL_TWBCLIENT ),
                   "Insecure write to dictionary/TM detected!\nLoc=READRECORDFROMDISK/1\nPlease save info required to reproduce this condition.",
                   "Gotcha!", 9999, MB_ERROR );
              #endif
              } /* endif */
              sRc = QDAMWRecordToDisk_V2(pBTIda, pLEntry->pBuffer);
            } /* endif */
            if ( !sRc )
            {
              UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L , NOMSG );
              pBT->usNumberOfAllocatedBuffers--;
            } /* endif */
        } /* endif */

        if ( pACTEntry->ulAccessCounter<MAX_READREC_CALLS )
        {
          pACTEntry->ulAccessCounter = 0L;
        }
        else
        {
          pACTEntry->ulAccessCounter -= MAX_READREC_CALLS;
        } /* endif */
      } /* endfor */
      pBT->ulReadRecCalls = 0L;
    }
    else
    {
      pBT->ulReadRecCalls++;
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    /* Increase the access counter for the record just read by  ACCESSBONUSPOINTS */
    pACTEntry=pBT->AccessCtrTable + usNumber;
    pACTEntry->ulAccessCounter += ACCESSBONUSPOINTS;
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMREADRECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  return ( sRc );
}

SHORT  QDAMReadRecord_V3
(
   PBTREE  pBTIda,
   USHORT  usNumber,
   PBTREEBUFFER_V3 * ppReadBuffer,
   BOOL    fNewRec
)
{
  USHORT   i;
  SHORT    sRc = 0;                  // return code
  BOOL     fMemOK = FALSE;
  PBTREEGLOB    pBT = pBTIda->pBTree;
  PLOOKUPENTRY_V3  pLEntry;
  PACCESSCTRTABLEENTRY  pACTEntry;

  DEBUGEVENT2( QDAMREADRECORD_LOC, FUNCENTRY_EVENT, usNumber, DB_GROUP, NULL );

  *ppReadBuffer = NULL;
  INC_READ_COUNT;

  /********************************************************************/
  /* If Lookup-Table is too small allocate additional memory for it   */
  /********************************************************************/
  if ( usNumber >= MAX_NUMBER_OF_LOOKUP_ENTRIES )
  {
    /* There is no room for this record number in the lookup table */
    sRc = BTREE_LOOKUPTABLE_TOO_SMALL;
  }
  else if ( !pBT->LookupTable_V3 || !pBT->AccessCtrTable )
  {
    sRc = BTREE_LOOKUPTABLE_NULL;
  }
  else
  {
    if ( usNumber >= pBT->usNumberOfLookupEntries )
    {
      /* The lookup-table entry for the record to read doesn't exist */
      /* Reallocate memory for LookupTable and AccessCounterTable */
      fMemOK = UtlAlloc( (PVOID *)&pBT->LookupTable_V3,
              (LONG) pBT->usNumberOfLookupEntries * sizeof(LOOKUPENTRY_V3),
              (LONG) (usNumber + 10) * sizeof(LOOKUPENTRY_V3), NOMSG );
      if ( fMemOK==TRUE )
      {
        fMemOK = UtlAlloc( (PVOID *)&pBT->AccessCtrTable,
                (LONG) pBT->usNumberOfLookupEntries * sizeof(ACCESSCTRTABLEENTRY),
                (LONG) (usNumber + 10) * sizeof(ACCESSCTRTABLEENTRY), NOMSG );
        if ( fMemOK==TRUE )
        {
          pBT->usNumberOfLookupEntries = usNumber + 10;
        }
        else
        {
          sRc = BTREE_NO_ROOM;
        } /* endif */
      }
      else
      {
        sRc = BTREE_NO_ROOM;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    /******************************************************************/
    /* If record is in memory set ppReadBuffer                        */
    /* else call QDAMReadRecordFromDisk to read it into memory        */
    /******************************************************************/
    pLEntry = pBT->LookupTable_V3 + usNumber;

    if ( pLEntry->pBuffer )
    {
      /* Safety-Check: is rec.number = number of rec. to read ? */
      if ( (pLEntry->pBuffer)->usRecordNumber == usNumber )
      {
        /* Record is already in memory */
        *ppReadBuffer = pLEntry->pBuffer;
      }
      else
      {
        /* This should never occur ! */
        sRc = BTREE_LOOKUPTABLE_CORRUPTED;
      } /* endif */
    }
    else
    {
      /* Record isn't in memory -> read it from disk */
      sRc =  QDAMReadRecordFromDisk_V3( pBTIda, usNumber, ppReadBuffer, fNewRec );
      pACTEntry=pBT->AccessCtrTable + usNumber;
      pACTEntry->ulAccessCounter = 0L;
    } /* endif */
  } /* endif */

  /****************************************************************************/
  /* If #calls of QDAMReadRecord is greater than MAX_READREC_CALLS:           */
  /*   write unlocked records with access counter < MAX_READREC_CALLS to disk */
  /*   (if fNeedToWrite is set) and free the allocated memory                 */
  /*   set #calls of QDAMReadRecord = 0                                       */
  /* else increment #calls of QDAMReadRecord                                  */
  /****************************************************************************/
  if ( !sRc )
  {
    if ( pBT->ulReadRecCalls >= MAX_READREC_CALLS )
    {
      for ( i=0; !sRc && (i < pBT->usNumberOfLookupEntries); i++ )
      {
        pLEntry = pBT->LookupTable_V3 + i;
        pACTEntry = pBT->AccessCtrTable + i;
        if ( pLEntry->pBuffer && !((pLEntry->pBuffer)->fLocked) && (i!=usNumber)
             && (pACTEntry->ulAccessCounter<MAX_READREC_CALLS) )
        {
            /* write buffer and free allocated space */
            if ( (pLEntry->pBuffer)->fNeedToWrite )
            {
              if ( (pBT->usOpenFlags & ASD_SHARED) && !pBTIda->fPhysLock )
              {
                // this condition should NEVER occur for shared resources
                // as write is only allowed if the database has been
                // previously locked
                ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 1, DB_GROUP, NULL );
              #ifdef _DEBUG
                WinMessageBox( HWND_DESKTOP,
                   (HWND)UtlQueryULong( QL_TWBCLIENT ),
                   "Insecure write to dictionary/TM detected!\nLoc=READRECORDFROMDISK/1\nPlease save info required to reproduce this condition.",
                   "Gotcha!", 9999, MB_ERROR );
              #endif
              } /* endif */
              sRc = QDAMWRecordToDisk_V3(pBTIda, pLEntry->pBuffer);
            } /* endif */
            if ( !sRc )
            {
              UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L , NOMSG );
              pBT->usNumberOfAllocatedBuffers--;
            } /* endif */
        } /* endif */

        if ( pACTEntry->ulAccessCounter<MAX_READREC_CALLS )
        {
          pACTEntry->ulAccessCounter = 0L;
        }
        else
        {
          pACTEntry->ulAccessCounter -= MAX_READREC_CALLS;
        } /* endif */
      } /* endfor */
      pBT->ulReadRecCalls = 0L;
    }
    else
    {
      pBT->ulReadRecCalls++;
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    /* Increase the access counter for the record just read by  ACCESSBONUSPOINTS */
    pACTEntry=pBT->AccessCtrTable + usNumber;
    pACTEntry->ulAccessCounter += ACCESSBONUSPOINTS;
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMREADRECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  return ( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMReadRecordFromDisk   Read record from disk
//------------------------------------------------------------------------------
// Function call:     QDAMReadRecordFromDisk( PBTREE, USHORT, PPBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       read the requested record from disk
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The B-tree to read
//                    USHORT             number of buffer to read
//                    PPBTREERECORD      The buffer read
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_LOOKUPTABLE_CORRUPTED lookup table is corrupted
//------------------------------------------------------------------------------
// Side effects:      The read record routine is one of the most
//                    performance critical routines in the module.
//
//------------------------------------------------------------------------------
// Function flow:     allocate memory for a new buffer and set pBuffer
//                    if still okay
//                      position in file and set sRc;
//                      if still okay then
//                        read buffer from disk, set sRc;
//                      endif
//                      if still okay then
//                        if new record then
//                          init header information
//                        endif
//                        set pointer to next buffer to be looked at
//                      else
//                        set BTREE_READ_ERROR
//                      endif
//------------------------------------------------------------------------------
SHORT QDAMReadRecordFromDisk_V2
(
   PBTREE         pBTIda,
   USHORT         usNumber,
   PBTREEBUFFER_V2 * ppReadBuffer,
   BOOL           fNewRec              // allow new records flag
)
{
  USHORT    usNumBytesRead = 0;                   // number of bytes read
  PBTREEHEADER pHeader;                          // pointer to header
  LONG     lOffset;                              // file offset to be set
  ULONG    ulNewOffset;                          // new position
  PBTREEBUFFER_V2   pBuffer = NULL;
  SHORT    sRc = 0;                             // return code
  PBTREEGLOB  pBT = pBTIda->pBTree;
  PLOOKUPENTRY_V2 pLEntry = NULL;

  DEBUGEVENT2( QDAMREADRECORDFROMDISK_LOC, FUNCENTRY_EVENT, usNumber, DB_GROUP, NULL );

  /********************************************************************/
  /* Allocate space for a new buffer and let pBuffer point to it      */
  /********************************************************************/
  if ( !pBT->LookupTable_V2 || ( usNumber >= pBT->usNumberOfLookupEntries ))
  {
    sRc = BTREE_LOOKUPTABLE_CORRUPTED;
  }
  else
  {
    pLEntry = pBT->LookupTable_V2 + usNumber;

    /* Safety-Check: is the lookup table entry (ptr. to buffer) for the
                     record to read NULL ? */
    if ( pLEntry->pBuffer )
    {
      /* ptr. isn't NULL: this should never occur */
      sRc = BTREE_LOOKUPTABLE_CORRUPTED;
    }
    else
    {
      /* Allocate memory for a buffer */
      UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, (LONG) BTREE_BUFFER_V2 , NOMSG );
      if ( pLEntry->pBuffer )
      {
        (pBT->usNumberOfAllocatedBuffers)++;
        pBuffer = pLEntry->pBuffer;
      }
      else
      {
        sRc = BTREE_NO_ROOM;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    INC_REAL_READ_COUNT;
    if ( !sRc )
    {
      lOffset = ((LONG) usNumber) * BTREE_REC_SIZE_V2;
      sRc = UtlChgFilePtr( pBT->fp, lOffset, FILE_BEGIN, &ulNewOffset, FALSE);
    } /* endif */

    // Read the record in to the buffer space
    // Mark the next buffer for future allocations
    if ( !sRc )
    {
      DEBUGEVENT2( QDAMREADRECORDFROMDISK_LOC, READ_EVENT, usNumber, DB_GROUP, NULL );
      sRc = UtlRead( pBT->fp,
                     (PVOID)&pBuffer->contents,
                     BTREE_REC_SIZE_V2, &usNumBytesRead, FALSE);
      if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

      /****************************************************************/
      /* The following check added by XQG                             */
      /* (Sometimes UtlRead terminates with sRc = 0 and usNumBytes = 0*/
      /*  if the database is locked and although we are NOT at the    */
      /*  end of the file!)                                           */
      /****************************************************************/
      if ( !sRc && !usNumBytesRead )
      {
        if ( usNumber < pBT->usNextFreeRecord )
        {
          // this is rather an error condition than an end-of-file condition!
          if ( pBT->usOpenFlags & ASD_SHARED )
          {
            sRc = BTREE_IN_USE;
          }
          else
          {
            sRc = BTREE_READ_ERROR;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    if ( !sRc )
    {
       pBuffer->usRecordNumber = usNumber;
//     pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );
       *ppReadBuffer = pBuffer;
       if ( !usNumBytesRead )
       {
         /*************************************************************/
         /* we are at the end of the file                             */
         /*************************************************************/
         if ( fNewRec )                // new record mode ???
         {
           memset (&pBuffer->contents, 0, sizeof(BTREERECORD_V2));
           // init this record
           pHeader = &(pBuffer->contents.header);
           pHeader->usOccupied = 0;
           pHeader->usNum = usNumber;
           pHeader->usFilled = sizeof(BTREEHEADER );
           pHeader->usLastFilled = BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER );
//         pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );

           sRc = QDAMWRecordToDisk_V2(pBTIda, pBuffer);
           if (! sRc )
           {
             if ( (pBT->usOpenFlags & ASD_SHARED) && !pBTIda->fPhysLock )
             {
               // this condition should NEVER occur for shared resources
               // as write is only allowed if the database has been
               // previously locked
               ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 2, DB_GROUP, NULL );
 #ifdef _DEBUG
               WinMessageBox( HWND_DESKTOP,
                              (HWND)UtlQueryULong( QL_TWBCLIENT ),
                              "Insecure write to dictionary/TM detected!\nLoc=READRECORDFROMDISK/2\nPlease save info required to reproduce this condition.",
                              "Gotcha!", 9999, MB_ERROR );
 #endif
             } /* endif */
             sRc = QDAMWriteHeader( pBTIda );
           } /* endif */
//         pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );
         }
         else
         {
           ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 4, DB_GROUP, NULL );
#ifdef _DEBUG
           WinMessageBox( HWND_DESKTOP,
                          (HWND)UtlQueryULong( QL_TWBCLIENT ),
                          "EQF9998: Write of new QDAM record w/o fNewRec set!\nLoc=READRECORDFROMDISK/3\n",
                          "Internal Error", 9998, MB_ERROR );
#endif
           sRc = BTREE_READ_ERROR;
         } /* endif */
       } /* endif */
    } /* endif */
  } /* endif */

  // For shared databases, check if database has been changed since last access
  // in case of changes force a BTREE_IN_USE return code thus triggering a retry
  // on one the outer calling levels
  if ( !sRc && (pBT->usOpenFlags & ASD_SHARED) )
  {
      LONG  lUpdCount;                 // buffer for new value of update counter

     /**********************************************************************/
     /* Get current database update count                                  */
     /**********************************************************************/
     sRc = QDAMGetUpdCounter( pBTIda, &lUpdCount, 0, 1 );

     if ( !sRc )
     {
        if ( lUpdCount != pBT->alUpdCtr[0] )
        {
           sRc = BTREE_INVALIDATED;
        } /* endif */
     } /* endif */
  } /* endif */

  /********************************************************************/
  /* Free record buffer in case of errors so record will be re-read   */
  /* during retry operations                                          */
  /********************************************************************/
  if ( sRc )
  {
    if ( (pLEntry != NULL) && (pLEntry->pBuffer != NULL) )
    {
      UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L, NOMSG );
      (pBT->usNumberOfAllocatedBuffers)--;
    } /* endif */
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */
  return sRc;
}


SHORT QDAMReadRecordFromDisk_V3
(
   PBTREE         pBTIda,
   USHORT         usNumber,
   PBTREEBUFFER_V3 * ppReadBuffer,
   BOOL           fNewRec              // allow new records flag
)
{
  USHORT    usNumBytesRead = 0;                   // number of bytes read
  PBTREEHEADER pHeader;                          // pointer to header
  LONG     lOffset;                              // file offset to be set
  ULONG    ulNewOffset;                          // new position
  PBTREEBUFFER_V3   pBuffer = NULL;
  SHORT    sRc = 0;                             // return code
  PBTREEGLOB  pBT = pBTIda->pBTree;
  PLOOKUPENTRY_V3 pLEntry = NULL;

  DEBUGEVENT2( QDAMREADRECORDFROMDISK_LOC, FUNCENTRY_EVENT, usNumber, DB_GROUP, NULL );

  /********************************************************************/
  /* Allocate space for a new buffer and let pBuffer point to it      */
  /********************************************************************/
  if ( !pBT->LookupTable_V3 || ( usNumber >= pBT->usNumberOfLookupEntries ))
  {
    sRc = BTREE_LOOKUPTABLE_CORRUPTED;
  }
  else
  {
    pLEntry = pBT->LookupTable_V3 + usNumber;

    /* Safety-Check: is the lookup table entry (ptr. to buffer) for the
                     record to read NULL ? */
    if ( pLEntry->pBuffer )
    {
      /* ptr. isn't NULL: this should never occur */
      sRc = BTREE_LOOKUPTABLE_CORRUPTED;
    }
    else
    {
      /* Allocate memory for a buffer */
      UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, (LONG) BTREE_BUFFER_V3 , NOMSG );
      if ( pLEntry->pBuffer )
      {
        (pBT->usNumberOfAllocatedBuffers)++;
        pBuffer = pLEntry->pBuffer;
      }
      else
      {
        sRc = BTREE_NO_ROOM;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    INC_REAL_READ_COUNT;
    if ( !sRc )
    {
      lOffset = ((LONG) usNumber) * BTREE_REC_SIZE_V3;
      sRc = UtlChgFilePtr( pBT->fp, lOffset, FILE_BEGIN, &ulNewOffset, FALSE);
    } /* endif */

    // Read the record in to the buffer space
    // Mark the next buffer for future allocations
    if ( !sRc )
    {
      DEBUGEVENT2( QDAMREADRECORDFROMDISK_LOC, READ_EVENT, usNumber, DB_GROUP, NULL );
      sRc = UtlRead( pBT->fp,
                     (PVOID)&pBuffer->contents,
                     BTREE_REC_SIZE_V3, &usNumBytesRead, FALSE);
      if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

      /****************************************************************/
      /* The following check added by XQG                             */
      /* (Sometimes UtlRead terminates with sRc = 0 and usNumBytes = 0*/
      /*  if the database is locked and although we are NOT at the    */
      /*  end of the file!)                                           */
      /****************************************************************/
      if ( !sRc && !usNumBytesRead )
      {
        if ( usNumber < pBT->usNextFreeRecord )
        {
          // this is rather an error condition than an end-of-file condition!
          if ( pBT->usOpenFlags & ASD_SHARED )
          {
            sRc = BTREE_IN_USE;
          }
          else
          {
            sRc = BTREE_READ_ERROR;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    if ( !sRc )
    {
       pBuffer->usRecordNumber = usNumber;
//     pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );
       *ppReadBuffer = pBuffer;
       if ( !usNumBytesRead )
       {
         /*************************************************************/
         /* we are at the end of the file                             */
         /*************************************************************/
         if ( fNewRec )                // new record mode ???
         {
           memset (&pBuffer->contents, 0, sizeof(BTREERECORD_V3));
           // init this record
           pHeader = &(pBuffer->contents.header);
           pHeader->usOccupied = 0;
           pHeader->usNum = usNumber;
           pHeader->usFilled = sizeof(BTREEHEADER );
           pHeader->usLastFilled = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER );
//         pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );

           sRc = QDAMWRecordToDisk_V3(pBTIda, pBuffer);
           if (! sRc )
           {
             if ( (pBT->usOpenFlags & ASD_SHARED) && !pBTIda->fPhysLock )
             {
               // this condition should NEVER occur for shared resources
               // as write is only allowed if the database has been
               // previously locked
               ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 2, DB_GROUP, NULL );
 #ifdef _DEBUG
               WinMessageBox( HWND_DESKTOP,
                              (HWND)UtlQueryULong( QL_TWBCLIENT ),
                              "Insecure write to dictionary/TM detected!\nLoc=READRECORDFROMDISK/2\nPlease save info required to reproduce this condition.",
                              "Gotcha!", 9999, MB_ERROR );
 #endif
             } /* endif */
             sRc = QDAMWriteHeader( pBTIda );
           } /* endif */
//         pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );
         }
         else
         {
           ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 4, DB_GROUP, NULL );
#ifdef _DEBUG
           WinMessageBox( HWND_DESKTOP,
                          (HWND)UtlQueryULong( QL_TWBCLIENT ),
                          "EQF9998: Write of new QDAM record w/o fNewRec set!\nLoc=READRECORDFROMDISK/3\n",
                          "Internal Error", 9998, MB_ERROR );
#endif
           sRc = BTREE_READ_ERROR;
         } /* endif */
       } /* endif */
    } /* endif */
  } /* endif */

  // For shared databases, check if database has been changed since last access
  // in case of changes force a BTREE_IN_USE return code thus triggering a retry
  // on one the outer calling levels
  if ( !sRc && (pBT->usOpenFlags & ASD_SHARED) )
  {
      LONG  lUpdCount;                 // buffer for new value of update counter

     /**********************************************************************/
     /* Get current database update count                                  */
     /**********************************************************************/
     sRc = QDAMGetUpdCounter( pBTIda, &lUpdCount, 0, 1 );

     if ( !sRc )
     {
        if ( lUpdCount != pBT->alUpdCtr[0] )
        {
           sRc = BTREE_INVALIDATED;
        } /* endif */
     } /* endif */
  } /* endif */

  /********************************************************************/
  /* Free record buffer in case of errors so record will be re-read   */
  /* during retry operations                                          */
  /********************************************************************/
  if ( sRc )
  {
    if ( (pLEntry != NULL) && (pLEntry->pBuffer != NULL) )
    {
      UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L, NOMSG );
      (pBT->usNumberOfAllocatedBuffers)--;
    } /* endif */
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */
  return sRc;
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMWriteHeader       Write initial header
//------------------------------------------------------------------------------
// Function call:     QDAMWriteHeader( PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Writes the first record of the (index) file so that
//                    subsequent accesses know what the index is like.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     init initial record (describing the disk)
//                    position ptr to begin of file
//                    write record to disk
//                    check if disk is full
//                    if so, set sRC = BTREE_WRITE_ERROR
//
//------------------------------------------------------------------------------
 SHORT QDAMWriteHeader
(
   PBTREE     pBTIda                   // pointer to btree structure
)
{
  SHORT  sRc=0;
  BTREEHEADRECORD HeadRecord;          // header record
  USHORT usBytesWritten;               // number of bytes written
  ULONG  ulNewOffset;                  // new offset in file
  PBTREEGLOB  pBT = pBTIda->pBTree;

  DEBUGEVENT2( QDAMWRITEHEADER_LOC, FUNCENTRY_EVENT, 0, DB_GROUP, NULL );

  // Write the initial record (describing the index) out to disk

  memset( &HeadRecord, '\0', sizeof(BTREEHEADRECORD));
  memcpy(HeadRecord.chEQF,pBT->chEQF, sizeof(pBT->chEQF));
  HeadRecord.usFirstNode = pBT->usFirstNode;
  HeadRecord.usFirstLeaf = pBT->usFirstLeaf;
  HeadRecord.usFreeKeyBuffer = pBT->usFreeKeyBuffer;
  HeadRecord.usFreeDataBuffer = pBT->usFreeDataBuffer;
  HeadRecord.usFirstDataBuffer = pBT->usFirstDataBuffer;  // first data buffer
  HeadRecord.fOpen  = (EQF_BOOL)pBT->fOpen;                    // open flag set
  // DataRecList in header is in old format (RECPARAMOLD),
  // so convert in-memory copy of list (in the RECPARAM format) to
  // the old format
  {
    int i;
    for ( i = 0; i < MAX_LIST; i++ )
    {
      HeadRecord.DataRecList[i].usOffset = pBT->DataRecList[i].usOffset;
      HeadRecord.DataRecList[i].usNum    = pBT->DataRecList[i].usNum;
      HeadRecord.DataRecList[i].sLen     = (SHORT)pBT->DataRecList[i].ulLen;
    } /* endfor */
  }
  HeadRecord.fTerse = (EQF_BOOL) pBT->fTerse;
  memcpy( HeadRecord.chCollate, pBT->chCollate, COLLATE_SIZE );
  memcpy( HeadRecord.chCaseMap, pBT->chCaseMap, COLLATE_SIZE );

  memcpy( HeadRecord.chEntryEncode, pBT->chEntryEncode, ENTRYENCODE_LEN );

  /********************************************************************/
  /* Update usNextFreeRecord field of Header and indicate that the    */
  /* field is valid by assigning BTREE_V1 to the bVersion field.      */
  /********************************************************************/
  HeadRecord.usNextFreeRecord = pBT->usNextFreeRecord;
  HeadRecord.Flags.bVersion         = BTREE_V1;


  if ( pBT->bRecSizeVersion == BTREE_V3 )
  {
    HeadRecord.Flags.f16kRec  = TRUE;
  }
  else
  {
    HeadRecord.Flags.f16kRec  = FALSE;
  } /* endif */

  sRc = UtlChgFilePtr( pBT->fp, 0L, FILE_BEGIN, &ulNewOffset, FALSE);

  if ( ! sRc )
  {
    sRc = UtlWrite( pBT->fp, (PVOID) &HeadRecord,
                    sizeof(BTREEHEADRECORD), &usBytesWritten, FALSE );
    if ( sRc )
    {
      sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
    }
    else
    {
      // check if disk is full
      if ( usBytesWritten != sizeof(BTREEHEADRECORD)  )
      {
        sRc = BTREE_DISK_FULL;
      }
      /****************************************************************/
      /* write through of header record if in GUARD mode or for       */
      /* shared databases                                             */
      /****************************************************************/
      else if ( pBT->fGuard || (pBT->usOpenFlags & ASD_SHARED) )
      {
        if ( UtlBufReset( pBT->fp, FALSE ))
        {
          sRc = BTREE_WRITE_ERROR;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  if ( sRc == NO_ERROR )
  {
    pBT->fUpdated = TRUE;
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMWRITEHEADER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */
  DEBUGEVENT2( QDAMWRITEHEADER_LOC, FUNCEXIT_EVENT, 0, DB_GROUP, NULL );

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDestroy     Destroy dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDestroy( PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Destroy the file if something went wrong during the
//                    create
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     incorrect pointer
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_CLOSE_ERROR error closing dictionary
//
//------------------------------------------------------------------------------
// Function flow:     if ptr to BTree does not exist
//                      set RC = BTREE_INVALID
//                    else
//                      if filename exists
//                        call QDAMDictClose
//                        delete the file 'filename'
//                      endif
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDestroy
(
   PBTREE  pBTIda                   // pointer to generic structure
)
{
   SHORT sRc = 0;                   // return code
   CHAR  chName[ MAX_EQF_PATH ];    // file name
   PBTREEGLOB  pBT = pBTIda->pBTree;

   if ( !pBT )
   {
     sRc = BTREE_INVALID;
   }
   else if (* (pBT->chFileName) )
   {
     //
     // QDAMDictClose frees up all of the memory associated with a B-tree,
     // we need to save the filename as we'll be deleteing the file by name
     //
     strcpy(chName, pBT->chFileName);
     sRc = QDAMDictClose( &pBTIda );
     if ( chName[0] )
     {
        UtlDelete( chName, 0L, FALSE);
     }
   }

   return ( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFindRecord      Find Record
//------------------------------------------------------------------------------
// Function call:     QDAMFindRecord( PBTREE, PCHAR, PPBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       Locates the record containing the leaf node that the
//                    given key is in or should be in if inserted.
//
//                    Returns record pointer that might contain the record,
//                    or NULL if it is unable to find it due to lack of buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key passed
//                    PPBTREEBUFFER          pointer to leaf mode
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     read record of 1st node
//                    while ok and record is not the leaf
//                      lock the record
//                      init for binary search (starting points)
//                      if lower start point < upper start point
//                        do
//                          set fFound
//                          get key string at middle position
//                          compare key found with the passed key
//                          decide whether to go left or right
//                        while not found
//                        if ok
//                          if sResult = '>'
//                            record found (sLow fits)
//                          else
//                            additional check: get key string
//                            compare keys
//                          endif
//                        endif
//                      else
//                        lower start point is position searched for
//                      endif
//                      if ok
//                        get data in data structure
//                        unlock previous record
//                        read record
//                    endwhile
//------------------------------------------------------------------------------
SHORT QDAMFindRecord_V2
(
    PBTREE   pBTIda,
    PCHAR_W  pKey,
    PBTREEBUFFER_V2 * ppRecord
)
{
  SHORT         sResult;
  SHORT         sLow;                              // Far left
  SHORT         sHigh;                             // Far right
  SHORT         sMid = 0;                          // Middle
  RECPARAM      recData;                           // data structure
  PCHAR_W       pKey2;                             // pointer to search key
  SHORT         sRc;                               // return code
  PBTREEGLOB    pBT = pBTIda->pBTree;

  memset(&recData, 0, sizeof(recData));
  sRc = QDAMReadRecord_V2( pBTIda, pBT->usFirstNode, ppRecord, FALSE  );
  while ( !sRc && !IS_LEAF( *ppRecord ))
  {
    BTREELOCKRECORD( *ppRecord );

    sLow = 0;                                    // start here
    sHigh = (SHORT) OCCUPIED( *ppRecord) -1 ;    // counting starts at zero


    while ( !sRc && (sLow <= sHigh) )
    {
      sMid = (sLow + sHigh)/2;
      pKey2 = QDAMGetszKey_V2( *ppRecord, sMid, pBT->usVersion );
      if ( pKey2 == NULL )
      {
        sRc = BTREE_CORRUPTED;
      }
      else
      {
        sResult = (*pBT->compare)(pBTIda, pKey, pKey2);
        if ( sResult < 0 )
        {
          sHigh = sMid - 1;                        // Go left
        }
        else
        {
          sLow = sMid + 1;                         // Go right
        } /* endif */
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* it is the lower one due the construction of the BTREE          */
    /*  -- according to the while loop it must be the sHIGH value     */
    /* if sHigh < 0 we should use the first one                      */
    /******************************************************************/
    if ( !sRc )
    {
      if ( sHigh == -1 )
      {
        sHigh = 0;
      } /* endif */
      recData = QDAMGetrecData_V2( *ppRecord, sHigh, pBT->usVersion );
    } /* endif */

    BTREEUNLOCKRECORD( *ppRecord );                        // unlock previous record.

    if ( !sRc )
    {
      sRc = QDAMReadRecord_V2( pBTIda, recData.usNum, ppRecord, FALSE  );
    } /* endif */
  } /* endwhile */
  return( sRc );
}

SHORT QDAMFindRecord_V3
(
    PBTREE   pBTIda,
    PCHAR_W  pKey,
    PBTREEBUFFER_V3 * ppRecord
)
{
  SHORT         sResult;
  SHORT         sLow;                              // Far left
  SHORT         sHigh;                             // Far right
  SHORT         sMid = 0;                          // Middle
  RECPARAM      recData;                           // data structure
  PCHAR_W       pKey2;                             // pointer to search key
  SHORT         sRc;                               // return code
  PBTREEGLOB    pBT = pBTIda->pBTree;

  memset(&recData, 0, sizeof(recData));
  sRc = QDAMReadRecord_V3( pBTIda, pBT->usFirstNode, ppRecord, FALSE  );
  while ( !sRc && !IS_LEAF( *ppRecord ))
  {
    BTREELOCKRECORD( *ppRecord );

    sLow = 0;                                    // start here
    sHigh = (SHORT) OCCUPIED( *ppRecord) -1 ;    // counting starts at zero


    while ( !sRc && (sLow <= sHigh) )
    {
      sMid = (sLow + sHigh)/2;
      pKey2 = QDAMGetszKey_V3( *ppRecord, sMid, pBT->usVersion );
      if ( pKey2 == NULL )
      {
        sRc = BTREE_CORRUPTED;
      }
      else
      {
        sResult = (*pBT->compare)(pBTIda, pKey, pKey2);
        if ( sResult < 0 )
        {
          sHigh = sMid - 1;                        // Go left
        }
        else
        {
          sLow = sMid + 1;                         // Go right
        } /* endif */
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* it is the lower one due the construction of the BTREE          */
    /*  -- according to the while loop it must be the sHIGH value     */
    /* if sHigh < 0 we should use the first one                      */
    /******************************************************************/
    if ( !sRc )
    {
      if ( sHigh == -1 )
      {
        sHigh = 0;
      } /* endif */
      recData = QDAMGetrecData_V3( *ppRecord, sHigh, pBT->usVersion );
    } /* endif */

    BTREEUNLOCKRECORD( *ppRecord );                        // unlock previous record.

    if ( !sRc )
    {
      sRc = QDAMReadRecord_V3( pBTIda, recData.usNum, ppRecord, FALSE  );
    } /* endif */
  } /* endwhile */
  return( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFindChild       Find Child
//------------------------------------------------------------------------------
// Function call:     QDAMFindChild ( PBTREE, PCHAR, USHORT, PPBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       Locates the record containing the childnode that the
//                    given key is in or should be in if inserted.
//
//                    Returns record pointer that might contain the record,
//                    or NULL if it is unable to find it due to lack of buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key passed
//                    PPBTREEBUFFER          pointer to leaf mode
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     read record of 1st node
//                    if    ok and record is not the leaf
//                      lock the record
//                      init for binary search (starting points)
//                      if lower start point < upper start point
//                        do
//                          set fFound
//                          get key string at middle position
//                          compare key found with the passed key
//                          decide whether to go left or right
//                        while not found
//                        if ok
//                          if sResult = '>'
//                            record found (sLow fits)
//                          else
//                            additional check: get key string
//                            compare keys
//                          endif
//                        endif
//                      else
//                        lower start point is position searched for
//                      endif
//                      if ok
//                        get data in data structure
//                        unlock previous record
//                        read record
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMFindChild_V2
(
    PBTREE pBTIda,
    PCHAR_W  pKey,
    USHORT   usNode,
    PBTREEBUFFER_V2 * ppRecord
)
{
  SHORT         sResult;
  SHORT         sLow;                              // Far left
  SHORT         sHigh;                             // Far right
  SHORT         sMid = 0;                          // Middle
  RECPARAM      recData;                           // data structure
  PCHAR_W       pKey2;                             // pointer to search key
  SHORT         sRc;                               // return code
  PBTREEGLOB    pBT = pBTIda->pBTree;

  memset(&recData, 0, sizeof(recData));
  sRc = QDAMReadRecord_V2( pBTIda, usNode, ppRecord, FALSE  );
  if ( !sRc && !IS_LEAF( *ppRecord ))
  {
    BTREELOCKRECORD( *ppRecord );

    sLow = 0;                                    // start here
    sHigh = (SHORT) OCCUPIED( *ppRecord) -1 ;    // counting starts at zero


    while ( !sRc && (sLow <= sHigh) )
    {
      sMid = (sLow + sHigh)/2;
      pKey2 = QDAMGetszKey_V2( *ppRecord, sMid, pBT->usVersion );
      if ( pKey2 == NULL )
      {
        sRc = BTREE_CORRUPTED;
      }
      else
      {
        sResult = (*pBT->compare)(pBTIda, pKey, pKey2);
        if ( sResult < 0 )
        {
          sHigh = sMid - 1;                        // Go left
        }
        else
        {
          sLow = sMid + 1;                         // Go right
        } /* endif */
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* it is the lower one due the construction of the BTREE          */
    /*  -- according to the while loop it must be the sHIGH value     */
    /* if sHigh < 0 we should use the first one                      */
    /******************************************************************/
    if ( !sRc )
    {
      if ( sHigh == -1 )
      {
        sHigh = 0;
      } /* endif */
      recData = QDAMGetrecData_V2( *ppRecord, sHigh, pBT->usVersion );
    } /* endif */

    BTREEUNLOCKRECORD( *ppRecord );    // unlock previous record.

    if ( !sRc )
    {
      sRc = QDAMReadRecord_V2( pBTIda, recData.usNum, ppRecord, FALSE  );
    } /* endif */
  } /* endif */
  return( sRc );
}

SHORT QDAMFindChild_V3
(
    PBTREE pBTIda,
    PCHAR_W  pKey,
    USHORT   usNode,
    PBTREEBUFFER_V3 * ppRecord
)
{
  SHORT         sResult;
  SHORT         sLow;                              // Far left
  SHORT         sHigh;                             // Far right
  SHORT         sMid = 0;                          // Middle
  RECPARAM      recData;                           // data structure
  PCHAR_W       pKey2;                             // pointer to search key
  SHORT         sRc;                               // return code
  PBTREEGLOB    pBT = pBTIda->pBTree;

  memset(&recData, 0, sizeof(recData));
  sRc = QDAMReadRecord_V3( pBTIda, usNode, ppRecord, FALSE  );
  if ( !sRc && !IS_LEAF( *ppRecord ))
  {
    BTREELOCKRECORD( *ppRecord );

    sLow = 0;                                    // start here
    sHigh = (SHORT) OCCUPIED( *ppRecord) -1 ;    // counting starts at zero


    while ( !sRc && (sLow <= sHigh) )
    {
      sMid = (sLow + sHigh)/2;
      pKey2 = QDAMGetszKey_V3( *ppRecord, sMid, pBT->usVersion );
      if ( pKey2 == NULL )
      {
        sRc = BTREE_CORRUPTED;
      }
      else
      {
        sResult = (*pBT->compare)(pBTIda, pKey, pKey2);
        if ( sResult < 0 )
        {
          sHigh = sMid - 1;                        // Go left
        }
        else
        {
          sLow = sMid + 1;                         // Go right
        } /* endif */
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* it is the lower one due the construction of the BTREE          */
    /*  -- according to the while loop it must be the sHIGH value     */
    /* if sHigh < 0 we should use the first one                      */
    /******************************************************************/
    if ( !sRc )
    {
      if ( sHigh == -1 )
      {
        sHigh = 0;
      } /* endif */
      recData = QDAMGetrecData_V3( *ppRecord, sHigh, pBT->usVersion );
    } /* endif */

    BTREEUNLOCKRECORD( *ppRecord );    // unlock previous record.

    if ( !sRc )
    {
      sRc = QDAMReadRecord_V3( pBTIda, recData.usNum, ppRecord, FALSE  );
    } /* endif */
  } /* endif */
  return( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFindParent       Find Parent for record
//------------------------------------------------------------------------------
// Function call:     QDAMFindParent ( PBTREE, PBTREEBUFFER, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Locates the record being the parent node to the passed
//                    record.
//
//                    Returns record number of parent node or NULL
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PBTREEBUFFER           record for which parent is wanted
//                    PUSHORT                record number of parent node
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     read record of 1st node
//                    while ok and record is not the leaf
//                      lock the record
//                      init for binary search (starting points)
//                      if lower start point < upper start point
//                        do
//                          set fFound
//                          get key string at middle position
//                          compare key found with the passed key
//                          decide whether to go left or right
//                        while not found
//                        if ok
//                          if sResult = '>'
//                            record found (sLow fits)
//                          else
//                            additional check: get key string
//                            compare keys
//                          endif
//                        endif
//                      else
//                        lower start point is position searched for
//                      endif
//                      if ok
//                        get data in data structure
//                        unlock previous record
//                        read record
//                    endwhile
//------------------------------------------------------------------------------
SHORT QDAMFindParent_V2
(
    PBTREE pBTIda,
    PBTREEBUFFER_V2  pRecord,
    PUSHORT       pusParent                      // record number of parent
)
{
  SHORT         sResult;
  SHORT         sLow;                            // Far left
  SHORT         sHigh;                           // Far right
  SHORT         sMid = 0;                        // Middle
  RECPARAM      recData;                         // data structure
  PCHAR_W       pKey2;                           // pointer to search key
  PCHAR_W       pKey ;                           // pointer to search key
  CHAR_W        chKey[HEADTERM_SIZE];            // key to be found
  SHORT         sRc;                             // return code
  PBTREEBUFFER_V2  pTempRec;                        // temp. record buffer
  USHORT        usRecNum;                        // passed record number
  PBTREEGLOB    pBT = pBTIda->pBTree;

  memset(&recData, 0, sizeof(recData));

  *pusParent = pBT->usFirstNode;                 // set parent to first node
  usRecNum = RECORDNUM( pRecord );
  pKey = QDAMGetszKey_V2( pRecord, 0, pBT->usVersion );
  if ( pKey == NULL )
  {
    sRc = BTREE_CORRUPTED;
  }
  else
  {
    if ( pBT->fTransMem )
    {
      memcpy( chKey, pKey, sizeof(ULONG));
    }
    else
    {
      UTF16strcpy( chKey, pKey );
    } /* endif */
  } /* endif */

  sRc = QDAMReadRecord_V2( pBTIda, pBT->usFirstNode, &pTempRec, FALSE  );

  while ( !sRc && !IS_LEAF( pTempRec ) && (usRecNum != RECORDNUM(pTempRec)) )
  {
    BTREELOCKRECORD( pTempRec );
    *pusParent = RECORDNUM( pTempRec );          // set parent to active node

    sLow = 0;                                    // start here
    sHigh = (SHORT) OCCUPIED( pTempRec ) -1 ;    // counting starts at zero


    while ( !sRc && (sLow <= sHigh) )
    {
      sMid = (sLow + sHigh)/2;
      pKey2 = QDAMGetszKey_V2( pTempRec, sMid, pBT->usVersion );
      if ( pKey2 == NULL )
      {
        sRc = BTREE_CORRUPTED;
      }
      else
      {
        sResult = (*pBT->compare)(pBTIda, chKey, pKey2);
        if ( sResult < 0 )
        {
          sHigh = sMid - 1;                        // Go left
        }
        else
        {
          sLow = sMid + 1;                         // Go right
        } /* endif */
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* it is the lower one due the construction of the BTREE          */
    /*  -- according to the while loop it must be the sHIGH value     */
    /* if sHigh < 0 we should use the first one                      */
    /******************************************************************/
    if ( !sRc )
    {
      if ( sHigh == -1 )
      {
        sHigh = 0;
      } /* endif */
      recData = QDAMGetrecData_V2( pTempRec, sHigh, pBT->usVersion );
    } /* endif */

    BTREEUNLOCKRECORD( pTempRec  );                        // unlock previous record.

    if ( !sRc )
    {
      sRc = QDAMReadRecord_V2( pBTIda, recData.usNum, &pTempRec, FALSE  );
    } /* endif */
  } /* endwhile */
  return( sRc );
}

SHORT QDAMFindParent_V3
(
    PBTREE pBTIda,
    PBTREEBUFFER_V3  pRecord,
    PUSHORT       pusParent                      // record number of parent
)
{
  SHORT         sResult;
  SHORT         sLow;                            // Far left
  SHORT         sHigh;                           // Far right
  SHORT         sMid = 0;                        // Middle
  RECPARAM      recData;                         // data structure
  PCHAR_W       pKey2;                           // pointer to search key
  PCHAR_W       pKey ;                           // pointer to search key
  CHAR_W        chKey[HEADTERM_SIZE];            // key to be found
  SHORT         sRc;                             // return code
  PBTREEBUFFER_V3  pTempRec;                        // temp. record buffer
  USHORT        usRecNum;                        // passed record number
  PBTREEGLOB    pBT = pBTIda->pBTree;

  memset(&recData, 0, sizeof(recData));

  *pusParent = pBT->usFirstNode;                 // set parent to first node
  usRecNum = RECORDNUM( pRecord );
  pKey = QDAMGetszKey_V3( pRecord, 0, pBT->usVersion );
  if ( pKey == NULL )
  {
    sRc = BTREE_CORRUPTED;
  }
  else
  {
    if ( pBT->fTransMem )
    {
      memcpy( chKey, pKey, sizeof(ULONG));
    }
    else
    {
      UTF16strcpy( chKey, pKey );
    } /* endif */
  } /* endif */

  sRc = QDAMReadRecord_V3( pBTIda, pBT->usFirstNode, &pTempRec, FALSE  );

  while ( !sRc && !IS_LEAF( pTempRec ) && (usRecNum != RECORDNUM(pTempRec)) )
  {
    BTREELOCKRECORD( pTempRec );
    *pusParent = RECORDNUM( pTempRec );          // set parent to active node

    sLow = 0;                                    // start here
    sHigh = (SHORT) OCCUPIED( pTempRec ) -1 ;    // counting starts at zero


    while ( !sRc && (sLow <= sHigh) )
    {
      sMid = (sLow + sHigh)/2;
      pKey2 = QDAMGetszKey_V3( pTempRec, sMid, pBT->usVersion );
      if ( pKey2 == NULL )
      {
        sRc = BTREE_CORRUPTED;
      }
      else
      {
        sResult = (*pBT->compare)(pBTIda, chKey, pKey2);
        if ( sResult < 0 )
        {
          sHigh = sMid - 1;                        // Go left
        }
        else
        {
          sLow = sMid + 1;                         // Go right
        } /* endif */
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* it is the lower one due the construction of the BTREE          */
    /*  -- according to the while loop it must be the sHIGH value     */
    /* if sHigh < 0 we should use the first one                      */
    /******************************************************************/
    if ( !sRc )
    {
      if ( sHigh == -1 )
      {
        sHigh = 0;
      } /* endif */
      recData = QDAMGetrecData_V3( pTempRec, sHigh, pBT->usVersion );
    } /* endif */

    BTREEUNLOCKRECORD( pTempRec  );                        // unlock previous record.

    if ( !sRc )
    {
      sRc = QDAMReadRecord_V3( pBTIda, recData.usNum, &pTempRec, FALSE  );
    } /* endif */
  } /* endwhile */
  return( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMLocateKey     Locate a key in passed record
//------------------------------------------------------------------------------
// Function call:     QDAMLocateKey( PBTREE, PBTREEBUFFER, PCHAR, PSHORT,
//                                    SEARCHTYPE, PSHORT );
//------------------------------------------------------------------------------
// Description:       locate a key in the passed record via a binary search
//                    Either pass back the position of the key or -1 if
//                    not found
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PBTREEBUFFER           record to be dealt with
//                    PCHAR                  key to be searched
//                    PSHORT                 located key position
//                    SEARCHTYPE             Exact, substring or equivalent
//                    PSHORT                 near position
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       -1         if key is not in record
//                    position   position of key
//
//------------------------------------------------------------------------------
// Side Effects:      record will be temporarily locked and unlocked at
//------------------------------------------------------------------------------
// Function flow:     if record exists
//                      lock record
//                      set upper/lower start points for binary search
//                      if valid area for search
//                        while key not found
//                          get key in the middle of lower/upper
//                          decide where to go on (lower/upper part)
//                          if match found
//                            if exact match is required
//                               ensure it and check prev/next
//                               (insertion is case sensitive)
//                            endif
//                          endif
//                        endwhile
//                        if ok
//                          ensure that end slots are checked
//                        endif
//                      endif
//                      unlock record
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMLocateKey_V2
(
   PBTREE pBTIda,                         // pointer to btree structure
   PBTREEBUFFER_V2 pRecord,                  // record to be dealt with
   PCHAR_W pKey,                          // key to be searched
   PSHORT  psKeyPos,                      // located key
   SEARCHTYPE  searchType,                // search type
   PSHORT  psNearPos                      // near position
)
{
  SHORT  sLow;                             // low value
  SHORT  sHigh;                            // high value
  SHORT  sResult;
  SHORT  sMid = 0;                         //
  SHORT  sRc = 0;                          // return value
  PCHAR_W  pKey2;                            // pointer to key string
  PCHAR_W  pHyphen;                            // pointer to key string
  SHORT    sCheckVariants=0;
  BOOL   fFound = FALSE;
  PBTREEGLOB    pBT = pBTIda->pBTree;
  CHAR_W  szKey[500];

  *psKeyPos = -1;                         // key not found
  if ( pRecord )
  {
    BTREELOCKRECORD( pRecord );
    sHigh = (SHORT) OCCUPIED( pRecord) -1 ;      // counting starts at zero
    sLow = 0;                                    // start here

    wcscpy( szKey, pKey ) ;
    pHyphen = wcschr(szKey, L'-') ;
    if ( pHyphen ) {
       sCheckVariants = 1 ;             // 3 variations when term contains hyphen, temporary -> 1
    }  else {
       sCheckVariants = 1 ;
    }
    while( sCheckVariants > 0 && !fFound ) {

      while ( !fFound && sLow <= sHigh )
      {
         sMid = (sLow + sHigh)/2;
         pKey2 = QDAMGetszKey_V2( pRecord, sMid, pBT->usVersion );
  
         if ( pKey2 )
         {
            sResult = (*pBT->compare)(pBTIda, szKey, pKey2);
            if ( sResult < 0 )
            {
              sHigh = sMid - 1;                        // Go left
            }
            else if (sResult > 0)
            {
              sLow = sMid+1;                           // Go right
            }
            else
            {
               fFound = TRUE;
               // if exact match is required we have to do a strcmp
               // to ensure it and probably check the previous or the
               // next one because our insertion is case insensitive
  
               /*********************************************************/
               /* checking will be done in any case to return the best  */
               /* matching substring                                    */
               /*********************************************************/
               if ( pBT->fTransMem )
               {
                 if (*((PULONG)szKey) == *((PULONG)pKey2))
                 {
                    *psKeyPos = sMid;
                 }
                 else
                 {
                    // try with previous
                    if ( sMid > sLow )
                    {
                      pKey2 = QDAMGetszKey_V2( pRecord, (SHORT)(sMid-1), pBT->usVersion );
                      if ( pKey2 == NULL )
                      {
                        sRc = BTREE_CORRUPTED;
                      }
                      else if ( *((PULONG)szKey) == *((PULONG)pKey2) )
                      {
                        *psKeyPos = sMid-1 ;
                      } /* endif */
                    } /* endif */
                    //  still not found
                    if ( !sRc && *psKeyPos == -1 && sMid < sHigh )
                    {
                      pKey2 = QDAMGetszKey_V2(  pRecord, (SHORT)(sMid+1), pBT->usVersion );
                      if ( pKey2 == NULL )
                      {
                        sRc = BTREE_CORRUPTED;
                      }
                      else if ( *((PULONG)szKey) == *((PULONG)pKey2) )
                      {
                            *psKeyPos = sMid+1 ;
                      } /* endif */
                    } /* endif */
                 } /* endif */
               }
               else
               {
                 /*******************************************************/
                 /* if dealing with exacts we have to do some more      */
                 /* explicit checking, else we have to find first pos.  */
                 /* substring match...                                  */
                 /*******************************************************/
                 enum KEYMATCH
                 {
                   EXACT_KEY,            // keys are exactly the same
                   CASEDIFF_KEY,         // case of keys is different
                   PUNCTDIFF_KEY,        // punctuation of keys differs
                   NOMATCH_KEY           // keys do not match at all
                 } BestKeyMatch = NOMATCH_KEY; // match level of best key so far
                 SHORT sBestKey = -1;    // best key found so far
  
                 /*****************************************************/
                 /* go back as long as the keys are the same ...      */
                 /*****************************************************/
  
                 /* DAW  If sMid == sLow and keys still the same, then may need to look at previous entry */
                 /*      For example, if "Submit" is first entry of this record, and "submit" is last     */
                 /*      entry of previous record, and looking for "submit".                              */
  
                 while ( sMid > sLow )
                 {
                   pKey2 = QDAMGetszKey_V2( pRecord, (SHORT)(sMid-1), pBT->usVersion );
  
                   if ( pKey2 == NULL )
                   {
                     sRc = BTREE_CORRUPTED;
                     break;
                   }
                   else if ( (*pBT->compare)(pBTIda, szKey, pKey2) == 0 )
                   {
                     sMid --;
                   }
                   else
                   {
                     break;
                   } /* endif */
                 } /* endwhile */
  
                 *psKeyPos = sMid;      // set key position
  
                 /*****************************************************/
                 /* go forward as long as the keys are the same  ..   */
                 /* and no matching sentence found                    */
                 /*****************************************************/
                 if ( (searchType == FEXACT) ||
                      (searchType == FEQUIV) )
                 {
                   *psKeyPos = -1;       // reset key position for exact search
                                         // and equivalent search
                 } /* endif */
  
                 while ( sMid <= sHigh )
                 {
                   pKey2 = QDAMGetszKey_V2( pRecord, sMid, pBT->usVersion );
                   if ( pKey2 == NULL )
                   {
                     sRc = BTREE_CORRUPTED;
                     break;
                   }
                   else if ( (*pBT->compare)(pBTIda, szKey, pKey2) == 0 )
                   {
                     if ( searchType == FEQUIV)
                     {
                       if ( UTF16strcmp( szKey, pKey2 ) == 0 )
                       {
                         // the match will not get better anymore ...
                         *psKeyPos = sMid;
                         break;
                       }
                       else if ( QDAMCaseCompare( pBTIda, szKey, pKey2, FALSE ) == 0 )
                       {
                         // match but case of characters differ
                         // so remember match if we have no better match yet and
                         // look for better ones...
                         if ( BestKeyMatch > CASEDIFF_KEY )
                         {
                           BestKeyMatch = CASEDIFF_KEY;
                           sBestKey = sMid;
                         } /* endif */
                       }
                       else if ( QDAMCaseCompare( pBTIda, szKey, pKey2, TRUE ) == 0 )
                       {
                         // match but punctuation differs
                         // so remember match if we have no other yet and
                         // look for better ones...
                         if ( BestKeyMatch == NOMATCH_KEY )
                         {
                           BestKeyMatch = PUNCTDIFF_KEY;
                           sBestKey = sMid;
                         } /* endif */
                       } /* endif */
                       sMid++;           // continue with next key
                     }
                     else if (UTF16strcmp( szKey, pKey2 ))
                     {
                        if ( QDAMCaseCompare( pBTIda, szKey, pKey2, TRUE ) == 0 )
                        {
                          // match but punctuation differs
                          // so remember match if we have no other yet and
                          // look for better ones...
                          if ( BestKeyMatch == NOMATCH_KEY )
                          {
                            BestKeyMatch = PUNCTDIFF_KEY;
                            sBestKey = sMid;
                          } /* endif */
                        } /* endif */
                       sMid ++;
                     }
                     else
                     {
                       *psKeyPos = sMid;
                       break;
                     } /* endif */
                   }
                   else
                   {
                     break;
                   } /* endif */
                 } /* endwhile */
  
                 // use best matching key if no other was found
                 if ( *psKeyPos == -1 )
                 {
                   *psKeyPos = sBestKey;
                 } /* endif */
               } /* endif */
  
               /*********************************************************/
               /* if we are checking only for substrings and didn't     */
               /* find a match yet, we will set the prev. found substrin*/
               /*********************************************************/
               if ( (*psKeyPos == -1) && (searchType == FSUBSTR)  )
               {
                  *psKeyPos = sMid;
               } /* endif */
               *psNearPos = *psKeyPos;
            } /* endif */
         }
         else
         {
            fFound = TRUE;
            sRc = BTREE_CORRUPTED;                // tree is corrupted
         } /* endif */
      } /* endwhile */
      if ( !fFound )
      {
        *psNearPos = min(sLow,(SHORT)OCCUPIED(pRecord)-1);// set nearest pos
      } /* endif */

      --sCheckVariants ;
      if ( ( ! fFound  ) &&                      // No match yet and term with hyphen
           ( pHyphen ) ) {
         sHigh = (SHORT) OCCUPIED( pRecord) -1 ; // counting restarts at zero
         sLow = 0;                               // start here
         if ( sCheckVariants == 2 ) {            // 1st hyphen variant
           wcscpy( szKey, pKey ) ;               // Replace hyphen with blank
           *pHyphen = NULL ;
         } else
         if ( sCheckVariants == 1 ) {            // 2nd hyphen variant
           wcscpy( szKey, pKey ) ;               // Remove hyphen and concatenate words
           memmove( pHyphen, pHyphen+1, (wcslen(pHyphen+1)+1)*sizeof(WCHAR) ) ;
         } else {
            sCheckVariants = 0 ;
         }
      } 
    }
    BTREEUNLOCKRECORD( pRecord );
  } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMLOCATEKEY_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}

SHORT QDAMLocateKey_V3
(
   PBTREE pBTIda,                         // pointer to btree structure
   PBTREEBUFFER_V3 pRecord,                  // record to be dealt with
   PCHAR_W pKey,                          // key to be searched
   PSHORT  psKeyPos,                      // located key
   SEARCHTYPE  searchType,                // search type
   PSHORT  psNearPos                      // near position
)
{
  SHORT  sLow;                             // low value
  SHORT  sHigh;                            // high value
  SHORT  sResult;
  SHORT  sMid = 0;                         //
  SHORT  sRc = 0;                          // return value
  PCHAR_W  pKey2;                            // pointer to key string
  BOOL   fFound = FALSE;
  PBTREEGLOB    pBT = pBTIda->pBTree;

  *psKeyPos = -1;                         // key not found
  if ( pRecord )
  {
    BTREELOCKRECORD( pRecord );
    sHigh = (SHORT) OCCUPIED( pRecord) -1 ;      // counting starts at zero
    sLow = 0;                                    // start here

    while ( !fFound && sLow <= sHigh )
    {
       sMid = (sLow + sHigh)/2;
       pKey2 = QDAMGetszKey_V3( pRecord, sMid, pBT->usVersion );

       if ( pKey2 )
       {
          sResult = (*pBT->compare)(pBTIda, pKey, pKey2);
          if ( sResult < 0 )
          {
            sHigh = sMid - 1;                        // Go left
          }
          else if (sResult > 0)
          {
            sLow = sMid+1;                           // Go right
          }
          else
          {
             fFound = TRUE;
             // if exact match is required we have to do a strcmp
             // to ensure it and probably check the previous or the
             // next one because our insertion is case insensitive

             /*********************************************************/
             /* checking will be done in any case to return the best  */
             /* matching substring                                    */
             /*********************************************************/
             if ( pBT->fTransMem )
             {
               if (*((PULONG)pKey) == *((PULONG)pKey2))
               {
                  *psKeyPos = sMid;
               }
               else
               {
                  // try with previous
                  if ( sMid > sLow )
                  {
                    pKey2 = QDAMGetszKey_V3( pRecord, (SHORT)(sMid-1), pBT->usVersion );
                    if ( pKey2 == NULL )
                    {
                      sRc = BTREE_CORRUPTED;
                    }
                    else if ( *((PULONG)pKey) == *((PULONG)pKey2) )
                    {
                      *psKeyPos = sMid-1 ;
                    } /* endif */
                  } /* endif */
                  //  still not found
                  if ( !sRc && *psKeyPos == -1 && sMid < sHigh )
                  {
                    pKey2 = QDAMGetszKey_V3(  pRecord, (SHORT)(sMid+1), pBT->usVersion );
                    if ( pKey2 == NULL )
                    {
                      sRc = BTREE_CORRUPTED;
                    }
                    else if ( *((PULONG)pKey) == *((PULONG)pKey2) )
                    {
                          *psKeyPos = sMid+1 ;
                    } /* endif */
                  } /* endif */
               } /* endif */
             }
             else
             {
               /*******************************************************/
               /* if dealing with exacts we have to do some more      */
               /* explicit checking, else we have to find first pos.  */
               /* substring match...                                  */
               /*******************************************************/
               enum KEYMATCH
               {
                 EXACT_KEY,            // keys are exactly the same
                 CASEDIFF_KEY,         // case of keys is different
                 PUNCTDIFF_KEY,        // punctuation of keys differs
                 NOMATCH_KEY           // keys do not match at all
               } BestKeyMatch = NOMATCH_KEY; // match level of best key so far
               SHORT sBestKey = -1;    // best key found so far

               /*****************************************************/
               /* go back as long as the keys are the same ...      */
               /*****************************************************/

               while ( sMid > sLow )
               {
                 pKey2 = QDAMGetszKey_V3( pRecord, (SHORT)(sMid-1), pBT->usVersion );
                 if ( pKey2 == NULL )
                 {
                   sRc = BTREE_CORRUPTED;
                   break;
                 }
                 else if ( (*pBT->compare)(pBTIda, pKey, pKey2) == 0 )
                 {
                   sMid --;
                 }
                 else
                 {
                   break;
                 } /* endif */
               } /* endwhile */

               *psKeyPos = sMid;      // set key position

               /*****************************************************/
               /* go forward as long as the keys are the same  ..   */
               /* and no matching sentence found                    */
               /*****************************************************/
               if ( (searchType == FEXACT) ||
                    (searchType == FEQUIV) )
               {
                 *psKeyPos = -1;       // reset key position for exact search
                                       // and equivalent search
               } /* endif */

               while ( sMid <= sHigh )
               {
                 pKey2 = QDAMGetszKey_V3( pRecord, sMid, pBT->usVersion );
                 if ( pKey2 == NULL )
                 {
                   sRc = BTREE_CORRUPTED;
                   break;
                 }
                 else if ( (*pBT->compare)(pBTIda, pKey, pKey2) == 0 )
                 {
                   if ( searchType == FEQUIV)
                   {
                     if ( UTF16strcmp( pKey, pKey2 ) == 0 )
                     {
                       // the match will not get better anymore ...
                       *psKeyPos = sMid;
                       break;
                     }
                     else if ( QDAMCaseCompare( pBTIda, pKey, pKey2, FALSE ) == 0 )
                     {
                       // match but case of characters differ
                       // so remember match if we have no better match yet and
                       // look for better ones...
                       if ( BestKeyMatch > CASEDIFF_KEY )
                       {
                         BestKeyMatch = CASEDIFF_KEY;
                         sBestKey = sMid;
                       } /* endif */
                     }
                     else if ( QDAMCaseCompare( pBTIda, pKey, pKey2, TRUE ) == 0 )
                     {
                       // match but punctuation differs
                       // so remember match if we have no other yet and
                       // look for better ones...
                       if ( BestKeyMatch == NOMATCH_KEY )
                       {
                         BestKeyMatch = PUNCTDIFF_KEY;
                         sBestKey = sMid;
                       } /* endif */
                     } /* endif */
                     sMid++;           // continue with next key
                   }
                   else if (UTF16strcmp( pKey, pKey2 ))
                   {
                     sMid ++;
                   }
                   else
                   {
                     *psKeyPos = sMid;
                     break;
                   } /* endif */
                 }
                 else
                 {
                   break;
                 } /* endif */
               } /* endwhile */

               // use best matching key if no other was found
               if ( *psKeyPos == -1 )
               {
                 *psKeyPos = sBestKey;
               } /* endif */
             } /* endif */

             /*********************************************************/
             /* if we are checking only for substrings and didn't     */
             /* find a match yet, we will set the prev. found substrin*/
             /*********************************************************/
             if ( (*psKeyPos == -1) && (searchType == FSUBSTR)  )
             {
                *psKeyPos = sMid;
             } /* endif */
             *psNearPos = *psKeyPos;
          } /* endif */
       }
       else
       {
          fFound = TRUE;
          sRc = BTREE_CORRUPTED;                // tree is corrupted
       } /* endif */
    } /* endwhile */
    if ( !fFound )
    {
      *psNearPos = min(sLow,(SHORT)OCCUPIED(pRecord)-1);// set nearest pos
    } /* endif */
    BTREEUNLOCKRECORD( pRecord );
  } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMLOCATEKEY_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMLocSubstr   locate the substring in the record
//------------------------------------------------------------------------------
// Function call:     QDAMLocSubStr(PBTREE, PBTREEBUFFER, PCHAR, PCHAR,
//                                  PUSHORT, PCHAR, PUSHORT);
//
//------------------------------------------------------------------------------
// Description:       Find the first key starting with the passed key and
//                    pass it back.
//                    If no error happened set this location as new
//                    current position
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PBTREEBUFFER           currently active key
//                    PCHAR                  key to be looked for
//                    PCHAR                  buffer for the key
//                    PUSHORT                on input length of buffer
//                                           on output length of filled data
//                    PCHAR                  buffer for the user data
//                    PUSHORT                on input length of buffer
//                                           on output length of filled data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//
//------------------------------------------------------------------------------
// Function flow:     locate the key with option set to FSUBSTR; set Rc
//                    if key found then
//                      check that key fulfills substring option
//                      if not okay
//                        set current position to the nearest key
//                        set Rc  = BTREE_NOT_FOUND
//                      else
//                        set current position to the found key
//                        pass back either length only or already data,
//                         depending on user request.
//                      endif
//                    else
//                      set Rc to BTREE_NOT_FOUND
//                      set current position to the nearest key
//                    endif
//                    return Rc
//------------------------------------------------------------------------------
SHORT QDAMLocSubstr_V2
(
   PBTREE        pBTIda,
   PBTREEBUFFER_V2  pRecord,
   PCHAR_W       pKey,
   PBYTE         pchBuffer,            // space for key data
   PULONG        pulLength,            // in/out length of returned key data
   PBYTE         pchUserData,          // space for user data
   PULONG        pulUserLen            // in/out length of returned user data
)
{
  SHORT  i;                            // index
  SHORT  sNearKey;                     // nearest key
  USHORT   usLen;                      // length of key
  SHORT    sRc  = 0;                   // return code
  PCHAR_W  pKey2;                      // key to be compared with
  RECPARAM recData;
  PBTREEGLOB    pBT = pBTIda->pBTree;

  sRc = QDAMLocateKey_V2(pBTIda, pRecord, pKey, &i, FSUBSTR, &sNearKey);
  if ( !sRc )
  {
     if ( i != -1 )
     {
       sNearKey = i;
     } /* endif */

     // set new current position
     pBTIda->sCurrentIndex = sNearKey;
     pBTIda->usCurrentRecord = RECORDNUM( pRecord );

     BTREELOCKRECORD( pRecord );
     //  check if the key fulfills the substring option
     if ( sNearKey != -1)
     {
       pKey2 = QDAMGetszKey_V2( pRecord, sNearKey, pBT->usVersion );
       if ( pKey2  )
       {
         if ( UTF16strnicmp(pKey, pKey2, (USHORT)UTF16strlenCHAR(pKey)) )
         {
           if ( sNearKey < (SHORT)OCCUPIED(pRecord)-1 )
           {
             sNearKey++;
           } /* endif */

           pKey2 = QDAMGetszKey_V2( pRecord, sNearKey, pBT->usVersion );
           if ( pKey2 )
           {
             if ( UTF16strnicmp(pKey, pKey2, (USHORT)UTF16strlenCHAR(pKey)) )
             {
               sRc = BTREE_NOT_FOUND;
             }
             else
             {
               pBTIda->sCurrentIndex = sNearKey;
             } /* endif */
           }
           else
           {
             sRc = BTREE_NOT_FOUND;
           } /* endif */
         } /* endif */
       }
       else
       {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMLOCSUBSTR_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
       } /* endif */

       if ( !sRc )
       {
         usLen = (USHORT)(UTF16strlenBYTE( pKey2 ) + sizeof(CHAR_W));
         if ( !pchBuffer || *pulLength == 0)
         {
            *pulLength = usLen ;  // give back length only
         }
         else
         {
            if ( *pulLength >= usLen )
            {
               *pulLength = usLen;
               memcpy( pchBuffer, pKey2, usLen );
            }
            else
            {
               ERREVENT2( QDAMLOCSUBSTR_LOC, STATE_EVENT, 2, DB_GROUP, NULL );
               sRc = BTREE_BUFFER_SMALL;
            } /* endif */
         } /* endif */
         if ( !sRc )
         {
            recData = QDAMGetrecData_V2( pRecord, i, pBT->usVersion );
            if ( *pulUserLen == 0 || ! pchUserData )
            {
               *pulUserLen = recData.ulLen;
            }
            else
            {
               sRc =  QDAMGetszData_V2( pBTIda, recData, pchUserData, pulUserLen, DATA_NODE );
             } /* endif */
         } /* endif */
       } /* endif */
     }
     else
     {
        sRc = BTREE_NOT_FOUND;
     } /* endif */
     BTREEUNLOCKRECORD( pRecord );
  } /* endif */

  return sRc;
}

SHORT QDAMLocSubstr_V3
(
   PBTREE        pBTIda,
   PBTREEBUFFER_V3  pRecord,
   PCHAR_W       pKey,
   PBYTE         pchBuffer,            // space for key data
   PULONG        pulLength,            // in/out length of returned key data
   PBYTE         pchUserData,          // space for user data
   PULONG        pulUserLen            // in/out length of returned user data
)
{
  SHORT  i;                            // index
  SHORT  sNearKey;                     // nearest key
  USHORT   usLen;                      // length of key
  SHORT    sRc  = 0;                   // return code
  PCHAR_W  pKey2;                      // key to be compared with
  RECPARAM recData;
  PBTREEGLOB    pBT = pBTIda->pBTree;

  sRc = QDAMLocateKey_V3(pBTIda, pRecord, pKey, &i, FSUBSTR, &sNearKey);
  if ( !sRc )
  {
     if ( i != -1 )
     {
       sNearKey = i;
     } /* endif */

     // set new current position
     pBTIda->sCurrentIndex = sNearKey;
     pBTIda->usCurrentRecord = RECORDNUM( pRecord );

     BTREELOCKRECORD( pRecord );
     //  check if the key fulfills the substring option
     if ( sNearKey != -1)
     {
       pKey2 = QDAMGetszKey_V3( pRecord, sNearKey, pBT->usVersion );
       if ( pKey2  )
       {
         if ( UTF16strnicmp(pKey, pKey2, (USHORT)UTF16strlenCHAR(pKey)) )
         {
           if ( sNearKey < (SHORT)OCCUPIED(pRecord)-1 )
           {
             sNearKey++;
           } /* endif */

           pKey2 = QDAMGetszKey_V3( pRecord, sNearKey, pBT->usVersion );
           if ( pKey2 )
           {
             if ( UTF16strnicmp(pKey, pKey2, (USHORT)UTF16strlenCHAR(pKey)) )
             {
               sRc = BTREE_NOT_FOUND;
             }
             else
             {
               pBTIda->sCurrentIndex = sNearKey;
             } /* endif */
           }
           else
           {
             sRc = BTREE_NOT_FOUND;
           } /* endif */
         } /* endif */
       }
       else
       {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMLOCSUBSTR_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
       } /* endif */

       if ( !sRc )
       {
         usLen = (USHORT)(UTF16strlenBYTE( pKey2 ) + sizeof(CHAR_W));
         if ( !pchBuffer || *pulLength == 0)
         {
            *pulLength = usLen ;  // give back length only
         }
         else
         {
            if ( *pulLength >= usLen )
            {
               *pulLength = usLen;
               memcpy( pchBuffer, pKey2, usLen );
            }
            else
            {
               ERREVENT2( QDAMLOCSUBSTR_LOC, STATE_EVENT, 2, DB_GROUP, NULL );
               sRc = BTREE_BUFFER_SMALL;
            } /* endif */
         } /* endif */
         if ( !sRc )
         {
            recData = QDAMGetrecData_V3( pRecord, i, pBT->usVersion );
            if ( *pulUserLen == 0 || ! pchUserData )
            {
               *pulUserLen = recData.ulLen;
            }
            else
            {
               sRc =  QDAMGetszData_V3( pBTIda, recData, pchUserData, pulUserLen, DATA_NODE );
             } /* endif */
         } /* endif */
       } /* endif */
     }
     else
     {
        sRc = BTREE_NOT_FOUND;
     } /* endif */
     BTREEUNLOCKRECORD( pRecord );
  } /* endif */

  return sRc;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetszData    Get Data String
//------------------------------------------------------------------------------
// Function call:     QDAMGetszData( PBTREE, PBTREEBUFFER, PCHAR,
//                                   RECPARAM, RECPARAM );
//------------------------------------------------------------------------------
// Description:       Get the data string for this offset.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             pointer to btree structure
//                    PBTREEBUFFER       record where to insert the key
//                    PCHAR              key to be inserted
//                    RECPARAM           position/offset of the key string
//                    RECPARAM           position/offset of data (if data node)
//                                               else position of child record
//
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   if string could be extracted
//                    FALSE  else
//
//------------------------------------------------------------------------------
// Function flow:     read record
//                    copy data
//                    if it is tersed
//                      set flag
//                      remove terse indication
//                      unterse data
//                    endif
//
//------------------------------------------------------------------------------
SHORT QDAMGetszData_V2
(
   PBTREE    pBTIda,
   RECPARAM  recDataParam,
   PBYTE     pData,
   PULONG    pulDataLen,
   CHAR      chType                             // type of record key/data
)
{
   SHORT sRc = 0;
   PBTREEBUFFER_V2 pRecord;                         // pointer to record
   PBTREEHEADER pHeader;                         // pointer to header
   PCHAR        pTempData = NULL;                // pointer to data pointer
   PCHAR        pStartData = (PCHAR)pData;       // pointer to data pointer
   ULONG        ulLen = 0;                       // length of string
   ULONG        ulTerseLen = 0;                  // length of tersed string
   BOOL         fTerse = FALSE;                  // entry tersed??
   PUSHORT      pusOffset = NULL;                // pointer to offset
   ULONG        ulFitLen;                        // free to be filled length
   USHORT       usNum;                           // record number
   ULONG        ulLZSSLen;
   PBTREEGLOB   pBT = pBTIda->pBTree;
   BOOL         fRecLocked = FALSE;    // TRUE if BTREELOCKRECORD has been done

   USHORT       usLenFieldSize;                            // size of record length field

   // get size of record length field
   if ( pBT->usVersion >= NTM_VERSION2 )
   {
     usLenFieldSize = sizeof(ULONG);
   }
   else
   {
     usLenFieldSize = sizeof(USHORT);
   } /* endif */

   recDataParam.usNum = recDataParam.usNum + pBT->usFirstDataBuffer;
   // get record and copy data
   sRc = QDAMReadRecord_V2( pBTIda, recDataParam.usNum, &pRecord, FALSE  );
   if ( !sRc )
   {
      BTREELOCKRECORD( pRecord );
      fRecLocked = TRUE;
      if ( TYPE( pRecord ) != chType )
      {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
      }
      else
      {
         pusOffset = (PUSHORT) pRecord->contents.uchData;
         pusOffset += recDataParam.usOffset;        // point to key
         pTempData = (PCHAR)(pRecord->contents.uchData + *pusOffset);
         if ( *pusOffset > pBT->usBtreeRecSize )
         {
           sRc = BTREE_CORRUPTED;
           ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 2, DB_GROUP, NULL );
         }
         else
         {
           if ( pBT->usVersion >= NTM_VERSION2 )
           {
             ulLen = *((PULONG)pTempData);
             if ( ulLen & QDAM_TERSE_FLAGL )
           {
            fTerse = TRUE;
            ulLen &= ~QDAM_TERSE_FLAGL;
            ulLZSSLen = ulLen;         // length of compressed record ....
           } /* endif */
           pTempData += sizeof(ULONG ); // get pointer to data
#ifdef _DEBUG
           {
             CHAR szTemp[8];
             sprintf( szTemp, "%ld", ulLen );
             INFOEVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, recDataParam.usNum,
                         DB_GROUP, szTemp );
           }
#endif
         }
         else
         {
           USHORT usLen = *((PUSHORT)pTempData);
           if ( usLen & QDAM_TERSE_FLAG )
           {
            fTerse = TRUE;
            usLen &= ~QDAM_TERSE_FLAG;
            ulLen = (ULONG)usLen;
            ulLZSSLen = ulLen;         // length of compressed record ....
           }
           else
           {
            ulLen = (ULONG)usLen;
           } /* endif */
           pTempData += sizeof(USHORT); // get pointer to data
         } /* endif */
           pHeader = &(pRecord->contents.header);
         } /* endif */

         /*************************************************************/
         /* check if it is a valid length                             */
         /*************************************************************/
         if ( !sRc && (ulLen == 0) )
         {
           sRc = BTREE_CORRUPTED;
           ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 3, DB_GROUP, NULL );
         } /* endif */
       } /* endif */

       if ( !sRc )
       {
         if ( *pulDataLen == 0 || !pData )
         {
            // give back only length
            if ( fTerse )
            {
              // first field contains real length
              *pulDataLen = LENGTHOFDATA(pBT,pTempData);
            }
            else
            {
               *pulDataLen = ulLen;    // give back only length
            } /* endif */
         }
         else if ( *pulDataLen >= ulLen )
         {
            ulFitLen = pBT->usBtreeRecSize - sizeof(BTREEHEADER) - *pusOffset;
            ulFitLen = min( ulLen, ulFitLen - usLenFieldSize );

            if ( fTerse )
            {
               memcpy(pData,pTempData+usLenFieldSize,ulFitLen-usLenFieldSize);
               ulTerseLen = LENGTHOFDATA(pBT,pTempData);
               /**********************************************************/
               /* adjust pointers                                        */
               /**********************************************************/
               ulLen -= ulFitLen;
               pData += (ulFitLen - usLenFieldSize);
            }
            else
            {
               memcpy( pData, pTempData, ulFitLen );
               *pulDataLen = ulLen;
               /**********************************************************/
               /* adjust pointers                                        */
               /**********************************************************/
               ulLen -= ulFitLen;
               pData += ulFitLen;
            } /* endif */


            /**********************************************************/
            /* copy as long as still data are available               */
            /**********************************************************/
            while ( !sRc && ulLen )
            {
               usNum = NEXT( pRecord );
               BTREEUNLOCKRECORD( pRecord );
               fRecLocked = FALSE;
               if ( usNum )
               {
                  sRc = QDAMReadRecord_V2( pBTIda, usNum, &pRecord, FALSE  );
                  if ( !sRc && TYPE( pRecord ) != DATA_NEXTNODE )
                  {
                    sRc = BTREE_CORRUPTED;
                    ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 3, DB_GROUP, NULL );
                  } /* endif */
                  if ( !sRc  )
                  {
                     BTREELOCKRECORD( pRecord );
                     fRecLocked = TRUE;
                     pusOffset = (PUSHORT) pRecord->contents.uchData;
                     pTempData = (PCHAR)(pRecord->contents.uchData + *pusOffset);

                     ulFitLen =  LENGTHOFDATA( pBT, pTempData );
                     ulFitLen = min( ulLen, ulFitLen );
                     pTempData += usLenFieldSize;      // get pointer to data

                     memcpy( pData, pTempData, ulFitLen );
                     ulLen -= ulFitLen;
                     pData += ulFitLen;
                  } /* endif */
               }
               else
               {
                 sRc = BTREE_CORRUPTED;
                 ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 4, DB_GROUP, NULL );
               } /* endif */
            } /* endwhile */
            if ( !sRc && fTerse )
            {
              switch ( pBT->fTerse )
              {
                case  BTREE_TERSE_HUFFMAN :
                  {
                    sRc = QDAMUnTerseData( pBTIda, (PUCHAR)pStartData, ulTerseLen,
                                           pulDataLen );
                  }
                  break;
                default :
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 5, DB_GROUP, NULL );
                  break;
              } /* endswitch */
            } /* endif */
         }
         else
         {
            sRc = BTREE_BUFFER_SMALL;
         } /* endif */

      } /* endif */
      if ( fRecLocked )
      {
        BTREEUNLOCKRECORD( pRecord );
      } /* endif */
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMGETSZDATA_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return sRc;
}
SHORT QDAMGetszData_V3
(
   PBTREE    pBTIda,
   RECPARAM  recDataParam,
   PBYTE     pData,
   PULONG    pulDataLen,
   CHAR      chType                             // type of record key/data
)
{
   SHORT sRc = 0;
   PBTREEBUFFER_V3 pRecord;                         // pointer to record
   PBTREEHEADER pHeader;                         // pointer to header
   PCHAR        pTempData = NULL;                // pointer to data pointer
   PCHAR        pStartData = (PCHAR)pData;       // pointer to data pointer
   ULONG        ulLen = 0;                       // length of string
   ULONG        ulTerseLen = 0;                  // length of tersed string
   BOOL         fTerse = FALSE;                  // entry tersed??
   PUSHORT      pusOffset = NULL;                // pointer to offset
   ULONG        ulFitLen;                        // free to be filled length
   USHORT       usNum;                           // record number
   ULONG        ulLZSSLen;
   PBTREEGLOB   pBT = pBTIda->pBTree;
   BOOL         fRecLocked = FALSE;    // TRUE if BTREELOCKRECORD has been done

   USHORT       usLenFieldSize;                            // size of record length field

   // get size of record length field
   if ( pBT->usVersion >= NTM_VERSION2 )
   {
     usLenFieldSize = sizeof(ULONG);
   }
   else
   {
     usLenFieldSize = sizeof(USHORT);
   } /* endif */

   recDataParam.usNum = recDataParam.usNum + pBT->usFirstDataBuffer;
   // get record and copy data
   sRc = QDAMReadRecord_V3( pBTIda, recDataParam.usNum, &pRecord, FALSE  );
   if ( !sRc )
   {
      BTREELOCKRECORD( pRecord );
      fRecLocked = TRUE;
      if ( TYPE( pRecord ) != chType )
      {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
      }
      else
      {
         pusOffset = (PUSHORT) pRecord->contents.uchData;
         pusOffset += recDataParam.usOffset;        // point to key
         pTempData = (PCHAR)(pRecord->contents.uchData + *pusOffset);
         if ( *pusOffset > pBT->usBtreeRecSize )
         {
           sRc = BTREE_CORRUPTED;
           ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 2, DB_GROUP, NULL );
         }
         else
         {
           if ( pBT->usVersion >= NTM_VERSION2 )
           {
             ulLen = *((PULONG)pTempData);
             if ( ulLen & QDAM_TERSE_FLAGL )
           {
            fTerse = TRUE;
            ulLen &= ~QDAM_TERSE_FLAGL;
            ulLZSSLen = ulLen;         // length of compressed record ....
           } /* endif */
           // check if length is in a normal range
           if ( ulLen > 134217728L ) // for now assume that no record exceeds a size of 128 MB )
           {
             sRc = BTREE_CORRUPTED;
           }
           pTempData += sizeof(ULONG ); // get pointer to data
#ifdef _DEBUG
           {
             CHAR szTemp[40];
             sprintf( szTemp, "%ld", ulLen );
             INFOEVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, recDataParam.usNum,
                         DB_GROUP, szTemp );
           }
#endif
         }
         else
         {
           USHORT usLen = *((PUSHORT)pTempData);
           if ( usLen & QDAM_TERSE_FLAG )
           {
            fTerse = TRUE;
            usLen &= ~QDAM_TERSE_FLAG;
            ulLen = (ULONG)usLen;
            ulLZSSLen = ulLen;         // length of compressed record ....
           }
           else
           {
            ulLen = (ULONG)usLen;
           } /* endif */
           pTempData += sizeof(USHORT); // get pointer to data
         } /* endif */
           pHeader = &(pRecord->contents.header);
         } /* endif */

         /*************************************************************/
         /* check if it is a valid length                             */
         /*************************************************************/
         if ( !sRc && (ulLen == 0) )
         {
           sRc = BTREE_CORRUPTED;
           ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 3, DB_GROUP, NULL );
         } /* endif */
       } /* endif */

       if ( !sRc )
       {
         if ( *pulDataLen == 0 || !pData )
         {
            // give back only length
            if ( fTerse )
            {
              // first field contains real length
              *pulDataLen = LENGTHOFDATA(pBT,pTempData);
            }
            else
            {
               *pulDataLen = ulLen;    // give back only length
            } /* endif */
         }
         else if ( *pulDataLen >= ulLen )
         {
            ulFitLen = pBT->usBtreeRecSize - sizeof(BTREEHEADER) - *pusOffset;
            ulFitLen = min( ulLen, ulFitLen - usLenFieldSize );

            if ( fTerse )
            {
               memcpy(pData,pTempData+usLenFieldSize,ulFitLen-usLenFieldSize);
               ulTerseLen = LENGTHOFDATA(pBT,pTempData);
               /**********************************************************/
               /* adjust pointers                                        */
               /**********************************************************/
               ulLen -= ulFitLen;
               pData += (ulFitLen - usLenFieldSize);
            }
            else
            {
               memcpy( pData, pTempData, ulFitLen );
               *pulDataLen = ulLen;
               /**********************************************************/
               /* adjust pointers                                        */
               /**********************************************************/
               ulLen -= ulFitLen;
               pData += ulFitLen;
            } /* endif */


            /**********************************************************/
            /* copy as long as still data are available               */
            /**********************************************************/
            while ( !sRc && ulLen )
            {
               usNum = NEXT( pRecord );
               BTREEUNLOCKRECORD( pRecord );
               fRecLocked = FALSE;
               if ( usNum )
               {
                  sRc = QDAMReadRecord_V3( pBTIda, usNum, &pRecord, FALSE  );
                  if ( !sRc && TYPE( pRecord ) != DATA_NEXTNODE )
                  {
                    sRc = BTREE_CORRUPTED;
                    ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 3, DB_GROUP, NULL );
                  } /* endif */
                  if ( !sRc  )
                  {
                     BTREELOCKRECORD( pRecord );
                     fRecLocked = TRUE;
                     pusOffset = (PUSHORT) pRecord->contents.uchData;
                     pTempData = (PCHAR)(pRecord->contents.uchData + *pusOffset);

                     ulFitLen =  LENGTHOFDATA( pBT, pTempData );
                     ulFitLen = min( ulLen, ulFitLen );
                     pTempData += usLenFieldSize;      // get pointer to data

                     memcpy( pData, pTempData, ulFitLen );
                     ulLen -= ulFitLen;
                     pData += ulFitLen;
                  } /* endif */
               }
               else
               {
                 sRc = BTREE_CORRUPTED;
                 ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 4, DB_GROUP, NULL );
               } /* endif */
            } /* endwhile */
            if ( !sRc && fTerse )
            {
              switch ( pBT->fTerse )
              {
                case  BTREE_TERSE_HUFFMAN :
                  {
                    sRc = QDAMUnTerseData( pBTIda, (PUCHAR)pStartData, ulTerseLen,
                                           pulDataLen );
                  }
                  break;
                default :
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 5, DB_GROUP, NULL );
                  break;
              } /* endswitch */
            } /* endif */
         }
         else
         {
            sRc = BTREE_BUFFER_SMALL;
         } /* endif */

      } /* endif */
      if ( fRecLocked )
      {
        BTREEUNLOCKRECORD( pRecord );
      } /* endif */
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMGETSZDATA_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetszKey    Get Key String
//------------------------------------------------------------------------------
// Function call:     QDAMGetszKey( PBTREEBUFFER, USHORT, PCHAR );
//
//------------------------------------------------------------------------------
// Description:       Get the key string for this offset.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           active record
//                    USHORT                 get data for this offset
//
//------------------------------------------------------------------------------
// Returncode type:   PSZ
//------------------------------------------------------------------------------
// Returncodes:       pointer to data
//
//------------------------------------------------------------------------------
// Function flow:     point to key
//                    set data pointer
//                    pass it back
//
//------------------------------------------------------------------------------
PSZ_W QDAMGetszKey_V2
(
   PBTREEBUFFER_V2  pRecord,              // active record
   USHORT  i,                          // get data term
   USHORT  usVersion                   // version of database
)
{
   PBYTE    pData, pEndOfRec;
   PUSHORT  pusOffset;
   usVersion;

   // get max pointer value

   pEndOfRec = (PBYTE)&(pRecord->contents) + BTREE_REC_SIZE_V2;

   // use record number of passed entry , read in record and pass
   // back pointer
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += i;                     // point to key
   if ( (PBYTE)pusOffset > pEndOfRec )
   {
     // offset pointer is out of range
     pData = NULL;
     ERREVENT2( QDAMGETSZKEY_LOC, INTFUNCFAILED_EVENT, 1, DB_GROUP, NULL );
   }
   else
   {
     pData = pRecord->contents.uchData + *pusOffset;
     if ( usVersion >= NTM_VERSION2 )
     {
       pData += sizeof(USHORT ) + sizeof(RECPARAM); // get pointer to data
     }
     else
     {
       pData += sizeof(USHORT ) + sizeof(RECPARAMOLD); // get pointer to data
     } /* endif */
     if ( pData > pEndOfRec )
     {
       // data pointer is out of range
       pData = NULL;
       ERREVENT2( QDAMGETSZKEY_LOC, INTFUNCFAILED_EVENT, 2, DB_GROUP, NULL );
     } /* endif */
   } /* endif */

   return ( (PSZ_W)pData );
}

PSZ_W QDAMGetszKey_V3
(
   PBTREEBUFFER_V3  pRecord,              // active record
   USHORT  i,                          // get data term
   USHORT  usVersion                   // version of database
)
{
   PBYTE    pData, pEndOfRec;
   PUSHORT  pusOffset;
   usVersion;

   // get max pointer value

   pEndOfRec = (PBYTE)&(pRecord->contents) + BTREE_REC_SIZE_V3;

   // use record number of passed entry , read in record and pass
   // back pointer
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += i;                     // point to key
   if ( (PBYTE)pusOffset > pEndOfRec )
   {
     // offset pointer is out of range
     pData = NULL;
     ERREVENT2( QDAMGETSZKEY_LOC, INTFUNCFAILED_EVENT, 1, DB_GROUP, NULL );
   }
   else
   {
     pData = pRecord->contents.uchData + *pusOffset;
     if ( usVersion >= NTM_VERSION2 )
     {
       pData += sizeof(USHORT ) + sizeof(RECPARAM); // get pointer to data
     }
     else
     {
       pData += sizeof(USHORT ) + sizeof(RECPARAMOLD); // get pointer to data
     } /* endif */
     if ( pData > pEndOfRec )
     {
       // data pointer is out of range
       pData = NULL;
       ERREVENT2( QDAMGETSZKEY_LOC, INTFUNCFAILED_EVENT, 2, DB_GROUP, NULL );
     } /* endif */
   } /* endif */

   return ( (PSZ_W)pData );
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetszKeyParam     Get Key String from param
//------------------------------------------------------------------------------
// Function call:     QDAMGetszKeyParam( PBTREE, RECPARAM, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Get the key string from passed offset.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    RECPARAM               record position
//                    PCHAR                  key to be retrieved
//                    PUSHORT                length of data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     read record
//                    if ok
//                      copy key string from passed offset to output area
//                    endif
//
//------------------------------------------------------------------------------
SHORT QDAMGetszKeyParam_V2
(
   PBTREE  pBTIda,               // pointer to btree structure
   RECPARAM  recKey,             // active record
   PCHAR_W   pKeyData,             // pointer to data
   PULONG  pulLen                // length of data
)
{
   PCHAR   pData = NULL;
   PBTREEBUFFER_V2  pRecord;              // active record
   SHORT   sRc = 0;                    // return code
   PBTREEGLOB    pBT = pBTIda->pBTree;
   USHORT  usLen;

   sRc = QDAMReadRecord_V2( pBTIda, recKey.usNum, &pRecord, FALSE  );

   /*******************************************************************/
   /* check length                                                    */
   /*******************************************************************/
   if ( !sRc )
   {
     pData = (PCHAR)(pRecord->contents.uchData + recKey.usOffset);
     usLen = *(PUSHORT) pData;

     // as data is now in Unicode the length of the key may be up to
     // HEADTERM_SIZE *2 !!!
     if ( (usLen < (HEADTERM_SIZE + HEADTERM_SIZE)) && ( *pulLen > usLen ) )
     {
       *pulLen = usLen;
       if ( pBT->usVersion >= NTM_VERSION2 )
       {
         pData += sizeof(USHORT ) + sizeof(RECPARAM);       // get pointer to data
       }
       else
       {
         pData += sizeof(USHORT ) + sizeof(RECPARAMOLD);       // get pointer to data
       } /* endif */
       memcpy( pKeyData, pData, *pulLen );
     }
     else
     {
       sRc = BTREE_CORRUPTED;
       ERREVENT2( QDAMGETSZKEYPARAM_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
       if ( pBT->usOpenFlags & ASD_SHARED )
       {
         LONG  lUpdCount;
         SHORT sTempRc = QDAMGetUpdCounter( pBTIda, &lUpdCount, 0, 1 );
         if ( !sTempRc )
         {
           if ( lUpdCount != pBT->alUpdCtr[0] )
           {
             // Corruption has ben caused by an update of the database from another user
             // switch to BTREE_IN_USE error code to allow retry of the current operation
             INFOEVENT2( QDAMGETSZKEYPARAM_LOC, REFRESH_EVENT, sRc, DB_GROUP, NULL );
             sRc = BTREE_IN_USE;
           } /* endif */
         } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMGETSZKEYPARAM_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return ( sRc );
}

SHORT QDAMGetszKeyParam_V3
(
   PBTREE  pBTIda,               // pointer to btree structure
   RECPARAM  recKey,             // active record
   PCHAR_W   pKeyData,             // pointer to data
   PULONG  pulLen                // length of data
)
{
   PCHAR   pData = NULL;
   PBTREEBUFFER_V3  pRecord;              // active record
   SHORT   sRc = 0;                    // return code
   PBTREEGLOB    pBT = pBTIda->pBTree;
   USHORT  usLen;

   sRc = QDAMReadRecord_V3( pBTIda, recKey.usNum, &pRecord, FALSE  );

   /*******************************************************************/
   /* check length                                                    */
   /*******************************************************************/
   if ( !sRc )
   {
     pData = (PCHAR)(pRecord->contents.uchData + recKey.usOffset);
     usLen = *(PUSHORT) pData;

     // as data is now in Unicode the length of the key may be up to
     // HEADTERM_SIZE *2 !!!
     if ( (usLen < (HEADTERM_SIZE + HEADTERM_SIZE)) && ( *pulLen > usLen ) )
     {
       *pulLen = usLen;
       if ( pBT->usVersion >= NTM_VERSION2 )
       {
         pData += sizeof(USHORT ) + sizeof(RECPARAM);       // get pointer to data
       }
       else
       {
         pData += sizeof(USHORT ) + sizeof(RECPARAMOLD);       // get pointer to data
       } /* endif */
       memcpy( pKeyData, pData, *pulLen );
     }
     else
     {
       sRc = BTREE_CORRUPTED;
       ERREVENT2( QDAMGETSZKEYPARAM_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
       if ( pBT->usOpenFlags & ASD_SHARED )
       {
         LONG  lUpdCount;
         SHORT sTempRc = QDAMGetUpdCounter( pBTIda, &lUpdCount, 0, 1 );
         if ( !sTempRc )
         {
           if ( lUpdCount != pBT->alUpdCtr[0] )
           {
             // Corruption has ben caused by an update of the database from another user
             // switch to BTREE_IN_USE error code to allow retry of the current operation
             INFOEVENT2( QDAMGETSZKEYPARAM_LOC, REFRESH_EVENT, sRc, DB_GROUP, NULL );
             sRc = BTREE_IN_USE;
           } /* endif */
         } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMGETSZKEYPARAM_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return ( sRc );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMKeyCompare      Generic compare function
//------------------------------------------------------------------------------
// Function call:     QDAMKeyCompare( PBTREE,PCHAR, PCHAR );
//
//------------------------------------------------------------------------------
// Description:       This is the generic compare function used
//                    for comparision
//                    It uses the Maptable provided for lower/upercase mapping
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to tree structure
//                    PCHAR                  first key
//                    PCHAR                  second key
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0   keys are equal
//                    <> keys are unequal
//
//------------------------------------------------------------------------------
// Function flow:     while character available in key1
//                      case map characters and find difference
//                      if different then
//                        go to next character touple
//                      else
//                        return collating difference
//                      endif
//                    wend
//                    get difference for last comparison and return collat.diff
//                    return difference
//------------------------------------------------------------------------------
SHORT QDAMKeyCompare
(
    PVOID pvBT,                        // pointer to tree structure
    PVOID pKey1,                       // pointer to first key
    PVOID pKey2                        // pointer to second key
)
{
  SHORT   sDiff;
  CHAR_W  c;
  PBTREE pBTIda = (PBTREE)pvBT;        // pointer to tree structure
  PBTREEGLOB    pBT = pBTIda->pBTree;
  PBYTE  pCollate = pBT->chCollate;    // pointer to collating sequence
  PSZ_W  pbKey1 = (PSZ_W) pKey1;
  PSZ_W  pbKey2 = (PSZ_W) pKey2;

  while ( (c = *pbKey1) != 0 )
  {
    /******************************************************************/
    /* ignore the following characters during matching: '/', '-', ' ' */
    /******************************************************************/
    while ( (c = *pbKey2) == ' ' || fIsPunctuation[(BYTE)c] )
    {
      pbKey2++;
    } /* endwhile */
    while ( (c = *pbKey1) == ' ' || fIsPunctuation[(BYTE)c] )
    {
      pbKey1++;
    } /* endwhile */
// @@ TODO: check collating sequence and fIsPunctuation

    sDiff = *(pCollate+(BYTE)c) - *(pCollate + (BYTE)*pbKey2);
    if ( !sDiff && *pbKey1 && *pbKey2 )
    {
      pbKey1++;
      pbKey2++;
    }
    else
    {
      return ( sDiff );
    } /* endif */
  } /* endwhile */
  return ( *(pCollate + (BYTE)c) - *(pCollate + (BYTE)*pbKey2));
}


SHORT QDAMKeyCompareNonUnicode
(
    PVOID pvBT,                        // pointer to tree structure
    PVOID pKey1,                       // pointer to first key
    PVOID pKey2                        // pointer to second key
)
{
  SHORT sDiff;
  BYTE  c;
  PBTREE pBTIda = (PBTREE)pvBT;        // pointer to tree structure
  PBTREEGLOB    pBT = pBTIda->pBTree;
  PBYTE  pCollate = pBT->chCollate;    // pointer to collating sequence
  CHAR   chHeadTerm[128];
  PBYTE  pbKey1;
  PBYTE  pbKey2 = (PBYTE) pKey2;

  pbKey1 = (PBYTE)Unicode2ASCII( (PSZ_W)pKey1, chHeadTerm, 0L );


  while ( (c = *pbKey1) != 0 )
  {
    /******************************************************************/
    /* ignore the following characters during matching: '/', '-', ' ' */
    /******************************************************************/
    while ( (c = *pbKey2) == ' ' || fIsPunctuation[c] )
    {
      pbKey2++;
    } /* endwhile */
    while ( (c = *pbKey1) == ' ' || fIsPunctuation[c] )
    {
      pbKey1++;
    } /* endwhile */

    sDiff = *(pCollate+c) - *(pCollate + *pbKey2);
    if ( !sDiff && *pbKey1 && *pbKey2 )
    {
      pbKey1++;
      pbKey2++;
    }
    else
    {
      return ( sDiff );
    } /* endif */
  } /* endwhile */
  return ( *(pCollate + c) - *(pCollate + *pbKey2));
}


/**********************************************************************/
/* Variant of QDAMKeyCompare ignoring the case of the characters      */
/**********************************************************************/
SHORT QDAMCaseCompare
(
    PVOID pvBT,                        // pointer to tree structure
    PVOID pKey1,                       // pointer to first key
    PVOID pKey2,                       // pointer to second key
    BOOL  fIgnorePunctuation           // ignore punctuation flag
)
{
  SHORT sDiff;
  BYTE  c;
  PBTREE pBTIda = (PBTREE)pvBT;        // pointer to tree structure
  PBTREEGLOB    pBT = pBTIda->pBTree;
  PBYTE  pbKey1 = (PBYTE) pKey1;
  PBYTE  pbKey2 = (PBYTE) pKey2;
  PBYTE  pMap = pBT->chCaseMap;        // pointer to mapping table

  while ( (c = *pbKey1) != 0 )
  {
    /******************************************************************/
    /* ignore the following characters during matching: '/', '-', ' ' */
    /******************************************************************/
    if ( fIgnorePunctuation )
    {
       while ( (c = *pbKey2) == ' ' || fIsPunctuation[c] )
       {
         pbKey2++;
       } /* endwhile */
       while ( (c = *pbKey1) == ' ' || fIsPunctuation[c] )
       {
         pbKey1++;
       } /* endwhile */
    } /* endif */

    sDiff = *(pMap+c) - *(pMap + *pbKey2);
    if ( !sDiff && *pbKey1 && *pbKey2 )
    {
      pbKey1++;
      pbKey2++;
    }
    else
    {
      return ( sDiff );
    } /* endif */
  } /* endwhile */
  return ( *(pMap + c) - *(pMap + *pbKey2));
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFirst    Get the first entry back
//------------------------------------------------------------------------------
// Function call:     QDAMFirst( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
//
//------------------------------------------------------------------------------
// Description:       Locate the first entry and pass back the
//                    associated information
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PRECPARAM              position of key
//                    PRECPARAM              position of key data
//                    PRECPARAM              position of data part
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//------------------------------------------------------------------------------
// Function flow:     init passed records
//                    if BTree exists
//                      set current pointer to first record of BTree
//                      if record == 0
//                        set RC = BTREE_EMPTY
//                      else
//                        read record
//                        check that record actually has values in it
//                      endif
//                    else
//                      set rc = BTREE_INVALID
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMFirst_V2
(
   PBTREE     pBTIda,
   PRECPARAM  precBTree,
   PRECPARAM  precKey,
   PRECPARAM  precData
)
{
   SHORT  sRc = 0;
   PBTREEBUFFER_V2 pRecord;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   memset( precBTree, 0, sizeof(RECPARAM));
   memset( precKey, 0, sizeof(RECPARAM));
   memset( precData, 0, sizeof(RECPARAM));

   if ( pBT )
   {
      pBTIda->usCurrentRecord = pBT->usFirstLeaf;
      pBTIda->sCurrentIndex = 0;
      if ( pBTIda->usCurrentRecord == 0)
      {
        sRc = BTREE_EMPTY;
      }
      else
      {
        sRc = QDAMReadRecord_V2( pBTIda, pBTIda->usCurrentRecord, &pRecord, FALSE  );
        if ( !sRc )
        {
          /***********************************************************************/
          /* Check that the record actually has some values in it                */
          /***********************************************************************/
          if ( ! OCCUPIED( pRecord))
          {
            sRc = BTREE_EMPTY;
          }
          else
          {
            precBTree->usOffset = pBTIda->sCurrentIndex;
            precBTree->usNum = pBTIda->usCurrentRecord;
            *precKey = QDAMGetrecKey_V2( pRecord, 0 );
            *precData = QDAMGetrecData_V2( pRecord, 0, pBT->usVersion );
          }
        }
      }
   }
   else
   {
      sRc = BTREE_INVALID;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMFIRST_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}

SHORT QDAMFirst_V3
(
   PBTREE     pBTIda,
   PRECPARAM  precBTree,
   PRECPARAM  precKey,
   PRECPARAM  precData
)
{
   SHORT  sRc = 0;
   PBTREEBUFFER_V3 pRecord;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   memset( precBTree, 0, sizeof(RECPARAM));
   memset( precKey, 0, sizeof(RECPARAM));
   memset( precData, 0, sizeof(RECPARAM));

   if ( pBT )
   {
      pBTIda->usCurrentRecord = pBT->usFirstLeaf;
      pBTIda->sCurrentIndex = 0;
      if ( pBTIda->usCurrentRecord == 0)
      {
        sRc = BTREE_EMPTY;
      }
      else
      {
        sRc = QDAMReadRecord_V3( pBTIda, pBTIda->usCurrentRecord, &pRecord, FALSE  );
        if ( !sRc )
        {
          /***********************************************************************/
          /* Check that the record actually has some values in it                */
          /***********************************************************************/
          if ( ! OCCUPIED( pRecord))
          {
            sRc = BTREE_EMPTY;
          }
          else
          {
            precBTree->usOffset = pBTIda->sCurrentIndex;
            precBTree->usNum = pBTIda->usCurrentRecord;
            *precKey = QDAMGetrecKey_V3( pRecord, 0 );
            *precData = QDAMGetrecData_V3( pRecord, 0, pBT->usVersion );
          }
        }
      }
   }
   else
   {
      sRc = BTREE_INVALID;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMFIRST_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMNext     Get the next entry back
//------------------------------------------------------------------------------
// Function call:     QDAMNext( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
//
//------------------------------------------------------------------------------
// Description:       Locate the next entry (by ascending key) and
//                    pass back the associated information
//
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PRECPARAM              position of key
//                    PRECPARAM              position of key data
//                    PRECPARAM              position of data part
//
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//------------------------------------------------------------------------------
// Function flow:     init passed areas
//                    if BTree exists
//                      if current record = 0
//                        position at first entry
//                      else
//                        read next record
//                      endif
//                      if ok
//                        check and validate the record read
//                        if ok
//                          get associated info
//                        endif
//                      endif
//                    else
//                      set rc = BTREE_INVALID
//                    endif
//
//
//------------------------------------------------------------------------------
SHORT QDAMNext_V2
(
   PBTREE     pBTIda,
   PRECPARAM  precBTree,
   PRECPARAM  precKey,
   PRECPARAM  precData
)
{
   SHORT  sRc = 0;
   PBTREEBUFFER_V2 pRecord;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   memset( precBTree, 0, sizeof(RECPARAM));
   memset( precKey, 0, sizeof(RECPARAM));
   memset( precData, 0, sizeof(RECPARAM));

   if ( pBT )
   {
      // Step to next index in the chain and read the record                     */
      // if usCurrentRecord = 0 we are at EOF
      if ( !pBTIda->usCurrentRecord )
      {
         sRc = QDAMFirstEntry_V2( pBTIda, &pRecord );
      }
      else
      {
         pBTIda->sCurrentIndex ++;
         sRc = QDAMReadRecord_V2( pBTIda, pBTIda->usCurrentRecord, &pRecord, FALSE  );
      } /* endif */


      if ( !sRc )
      {
        sRc = QDAMValidateIndex_V2( pBTIda, &pRecord );
        if ( !sRc )
        {
            precBTree->usOffset = pBTIda->sCurrentIndex;
            precBTree->usNum = pBTIda->usCurrentRecord;
            *precKey = QDAMGetrecKey_V2( pRecord, pBTIda->sCurrentIndex  );
            *precData = QDAMGetrecData_V2( pRecord, pBTIda->sCurrentIndex,
                                        pBT->usVersion );
        } /* endif */
      } /* endif */
   }
   else
   {
      sRc = BTREE_INVALID;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMNEXT_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}

SHORT QDAMNext_V3
(
   PBTREE     pBTIda,
   PRECPARAM  precBTree,
   PRECPARAM  precKey,
   PRECPARAM  precData
)
{
   SHORT  sRc = 0;
   PBTREEBUFFER_V3 pRecord;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   memset( precBTree, 0, sizeof(RECPARAM));
   memset( precKey, 0, sizeof(RECPARAM));
   memset( precData, 0, sizeof(RECPARAM));

   if ( pBT )
   {
      // Step to next index in the chain and read the record                     */
      // if usCurrentRecord = 0 we are at EOF
      if ( !pBTIda->usCurrentRecord )
      {
         sRc = QDAMFirstEntry_V3( pBTIda, &pRecord );
      }
      else
      {
         pBTIda->sCurrentIndex ++;
         sRc = QDAMReadRecord_V3( pBTIda, pBTIda->usCurrentRecord, &pRecord, FALSE  );
      } /* endif */


      if ( !sRc )
      {
        sRc = QDAMValidateIndex_V3( pBTIda, &pRecord );
        if ( !sRc )
        {
            precBTree->usOffset = pBTIda->sCurrentIndex;
            precBTree->usNum = pBTIda->usCurrentRecord;
            *precKey = QDAMGetrecKey_V3( pRecord, pBTIda->sCurrentIndex  );
            *precData = QDAMGetrecData_V3( pRecord, pBTIda->sCurrentIndex,
                                        pBT->usVersion );
        } /* endif */
      } /* endif */
   }
   else
   {
      sRc = BTREE_INVALID;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMNEXT_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMPrev   Get the prev entry back
//------------------------------------------------------------------------------
// Function call:     QDAMPrev( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
//
//------------------------------------------------------------------------------
// Description:       Locate the previous entry (by descending key)
//                    and pass back the associated
//                    information into the user provided buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PRECPARAM              position of key
//                    PRECPARAM              position of key data
//                    PRECPARAM              position of data part
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//------------------------------------------------------------------------------
// Function flow:     init variables
//                    if pointer valid then
//                     if usCurrentRecord = 0 ( EOF reached ) then
//                       position at last entry
//                     else
//                       read the record and get previous index in the chain
//                     endif
//                    endif
//                    if okay so far then
//                      validate new index
//                      if okay then
//                        fill in return values
//                      endif
//                    endif
//                    return return code
//------------------------------------------------------------------------------
SHORT QDAMPrev_V2
(
   PBTREE     pBTIda,
   PRECPARAM  precBTree,
   PRECPARAM  precKey,
   PRECPARAM  precData
)
{
   SHORT  sRc = 0;
   PBTREEBUFFER_V2 pRecord;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   memset( precBTree, 0, sizeof(RECPARAM));
   memset( precKey, 0, sizeof(RECPARAM));
   memset( precData, 0, sizeof(RECPARAM));

   if ( pBT )
   {
      // read the record and get previous index in the chain (if any)
      // if usCurrentRecord = 0 we are at EOF
      if ( !pBTIda->usCurrentRecord )
      {
         sRc = QDAMLastEntry_V2( pBTIda, &pRecord );
      }
      else
      {
         pBTIda->sCurrentIndex --;
         sRc = QDAMReadRecord_V2( pBTIda, pBTIda->usCurrentRecord, &pRecord, FALSE  );
      } /* endif */


      if ( ! sRc )
      {
         sRc = QDAMValidateIndex_V2( pBTIda, &pRecord );

        if ( !sRc )
        {
            precBTree->usOffset = pBTIda->sCurrentIndex;
            precBTree->usNum = pBTIda->usCurrentRecord;
            *precKey = QDAMGetrecKey_V2( pRecord, pBTIda->sCurrentIndex  );
            *precData = QDAMGetrecData_V2( pRecord, pBTIda->sCurrentIndex, pBT->usVersion );
        } /* endif */
      }
   }
   else
   {
      sRc = BTREE_INVALID;
   } /* endif */

  return sRc;
}

SHORT QDAMPrev_V3
(
   PBTREE     pBTIda,
   PRECPARAM  precBTree,
   PRECPARAM  precKey,
   PRECPARAM  precData
)
{
   SHORT  sRc = 0;
   PBTREEBUFFER_V3 pRecord;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   memset( precBTree, 0, sizeof(RECPARAM));
   memset( precKey, 0, sizeof(RECPARAM));
   memset( precData, 0, sizeof(RECPARAM));

   if ( pBT )
   {
      // read the record and get previous index in the chain (if any)
      // if usCurrentRecord = 0 we are at EOF
      if ( !pBTIda->usCurrentRecord )
      {
         sRc = QDAMLastEntry_V3( pBTIda, &pRecord );
      }
      else
      {
         pBTIda->sCurrentIndex --;
         sRc = QDAMReadRecord_V3( pBTIda, pBTIda->usCurrentRecord, &pRecord, FALSE  );
      } /* endif */


      if ( ! sRc )
      {
         sRc = QDAMValidateIndex_V3( pBTIda, &pRecord );

        if ( !sRc )
        {
            precBTree->usOffset = pBTIda->sCurrentIndex;
            precBTree->usNum = pBTIda->usCurrentRecord;
            *precKey = QDAMGetrecKey_V3( pRecord, pBTIda->sCurrentIndex  );
            *precData = QDAMGetrecData_V3( pRecord, pBTIda->sCurrentIndex, pBT->usVersion );
        } /* endif */
      }
   }
   else
   {
      sRc = BTREE_INVALID;
   } /* endif */

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMAllocTempAreas    Allocate Temp Areas
//------------------------------------------------------------------------------
// Function call:     QDAMAllocTempAreas( PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Allocate temp areas if necessary for key and data
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_ROOM     not enough memory
//------------------------------------------------------------------------------
// Function flow:     if no temp areas exist
//                      if no temp area for key exists
//                        allocate it
//                      endif
//                      if no temp record exists
//                        allocate it
//                      endif
//                      if not both areas allocated then
//                        set Rc to BTREE_NO_ROOM
//                      endif
//                    endif
//                    return Rc
//------------------------------------------------------------------------------

SHORT QDAMAllocTempAreas
(
   PBTREE  pBTIda
)
{
   SHORT  sRc = 0;                     // return code
   PBTREEGLOB    pBT = pBTIda->pBTree;


   if ( !pBT->pTempKey || !pBT->pTempRecord )
   {
      if ( ! pBT->pTempKey)
      {
         UtlAlloc( (PVOID *)&pBT->pTempKey, 0L, (LONG) HEADTERM_SIZE, NOMSG );
      } /* endif */
      if ( ! pBT->pTempRecord )
      {
         UtlAlloc( (PVOID *)&pBT->pTempRecord, 0L, (LONG) MAXDATASIZE, NOMSG );
         if ( pBT->pTempRecord )
         {
           pBT->ulTempRecSize = MAXDATASIZE;
         } /* endif */
      } /* endif */
      if ( !(pBT->pTempRecord && pBT->pTempKey) )
      {
         sRc = BTREE_NO_ROOM;
      } /* endif */
   } /* endif */

   return ( sRc );
}
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMValidateIndex    Find next valid position
//------------------------------------------------------------------------------
// Function call:     QDAMValidateIndex( PBTREE, PPBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       This function will return the next valid position
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PPBTREEBUFFER          pointer to selected record
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EOF_REACHED end or start of dictionary reached
//------------------------------------------------------------------------------
// Function flow:     if current index < 0 or current index > occupied then
//                      if current index < 0 then
//                        if previous record available then
//                          read in prev. record and set Rc
//                          if okay then
//                            set current index and current record to new data
//                          endif
//                        else
//                          set current record and index to 0
//                          set Rc to BTREE_EOF_REACHED
//                        endif
//                      elsif current index >= occupied then
//                        if next record available then
//                          set record to next record; current index to 0
//                          read in record and set Rc
//                        else
//                          set current record and index to 0
//                          set Rc to BTREE_EOF_REACHED
//                        endif
//                      endif
//                    endif
//                    return Rc
//
//------------------------------------------------------------------------------
SHORT QDAMValidateIndex_V2
(
   PBTREE         pBTIda,
   PBTREEBUFFER_V2 * ppRecord
)
{
   SHORT    sRc = 0;                   // set return code
   USHORT   usRecord;

   if ( pBTIda->sCurrentIndex < 0 ||
           pBTIda->sCurrentIndex >= (SHORT) OCCUPIED(*ppRecord))
   {
      if ( pBTIda->sCurrentIndex < 0 )
      {
         if ( PREV( *ppRecord ))
         {
            usRecord = PREV(*ppRecord);
            sRc = QDAMReadRecord_V2( pBTIda, usRecord, ppRecord, FALSE  );
            if (!sRc )
            {
               pBTIda->usCurrentRecord = usRecord;
               pBTIda->sCurrentIndex = (SHORT) (OCCUPIED(*ppRecord) - 1);
            } /* endif */
         }
         else
         {
            pBTIda->usCurrentRecord = 0;
            pBTIda->sCurrentIndex =  0;
            sRc = BTREE_EOF_REACHED;
         } /* endif */
      }
      else if ( pBTIda->sCurrentIndex >= (SHORT) OCCUPIED(*ppRecord))
      {
         if ( NEXT(*ppRecord) )
         {
            pBTIda->usCurrentRecord = NEXT(*ppRecord);
            sRc = QDAMReadRecord_V2( pBTIda, pBTIda->usCurrentRecord, ppRecord, FALSE  );
            pBTIda->sCurrentIndex = 0;
         }
         else
         {
            pBTIda->usCurrentRecord = 0;
            pBTIda->sCurrentIndex =  0;
            sRc = BTREE_EOF_REACHED;
         } /* endif */
      } /* endif */
   } /* endif */
   if ( sRc )
   {
     ERREVENT2( QDAMVALIDATEINDEX_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */
   return sRc;
}

SHORT QDAMValidateIndex_V3
(
   PBTREE         pBTIda,
   PBTREEBUFFER_V3 * ppRecord
)
{
   SHORT    sRc = 0;                   // set return code
   USHORT   usRecord;

   if ( pBTIda->sCurrentIndex < 0 ||
           pBTIda->sCurrentIndex >= (SHORT) OCCUPIED(*ppRecord))
   {
      if ( pBTIda->sCurrentIndex < 0 )
      {
         if ( PREV( *ppRecord ))
         {
            usRecord = PREV(*ppRecord);
            sRc = QDAMReadRecord_V3( pBTIda, usRecord, ppRecord, FALSE  );
            if (!sRc )
            {
               pBTIda->usCurrentRecord = usRecord;
               pBTIda->sCurrentIndex = (SHORT) (OCCUPIED(*ppRecord) - 1);
            } /* endif */
         }
         else
         {
            pBTIda->usCurrentRecord = 0;
            pBTIda->sCurrentIndex =  0;
            sRc = BTREE_EOF_REACHED;
         } /* endif */
      }
      else if ( pBTIda->sCurrentIndex >= (SHORT) OCCUPIED(*ppRecord))
      {
         if ( NEXT(*ppRecord) )
         {
            pBTIda->usCurrentRecord = NEXT(*ppRecord);
            sRc = QDAMReadRecord_V3( pBTIda, pBTIda->usCurrentRecord, ppRecord, FALSE  );
            pBTIda->sCurrentIndex = 0;
         }
         else
         {
            pBTIda->usCurrentRecord = 0;
            pBTIda->sCurrentIndex =  0;
            sRc = BTREE_EOF_REACHED;
         } /* endif */
      } /* endif */
   } /* endif */
   if ( sRc )
   {
     ERREVENT2( QDAMVALIDATEINDEX_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */
   return sRc;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMLastEntry      Position at last entry
//------------------------------------------------------------------------------
// Function call:     QDAMLastEntry ( PBTREE, PPBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       This function will position to the last entry in the
//                    dictionary
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PPBTREEBUFFER          pointer to selected record
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EOF_REACHED end or start of dictionary reached
//
//------------------------------------------------------------------------------
// Function flow:     read in root record
//                    if okay then
//                      while okay and record not leaf
//                        get record pointed to by last entry
//                      endwhile
//                      if okay then
//                        set current record and current index
//                      endif
//                    endif
//                    return Rc
//------------------------------------------------------------------------------
SHORT  QDAMLastEntry_V2
(
   PBTREE  pBTIda,
   PBTREEBUFFER_V2  * ppRecord               // pointer to pointer of record
)
{
   SHORT  sRc = 0;
   RECPARAM     recData;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // read in root record
   sRc = QDAMReadRecord_V2( pBTIda, pBT->usFirstNode, ppRecord, FALSE );
   if ( !sRc )
   {
      while (!sRc && !IS_LEAF(*ppRecord))
      {
         recData = QDAMGetrecData_V2( *ppRecord, (SHORT)(OCCUPIED(*ppRecord)-1), pBT->usVersion );
         sRc = QDAMReadRecord_V2( pBTIda, recData.usNum, ppRecord, FALSE  );
      } /* endwhile */
      if ( !sRc )
      {
         pBTIda->usCurrentRecord = RECORDNUM(*ppRecord);
         pBTIda->sCurrentIndex = (SHORT) (OCCUPIED(*ppRecord) - 1);
      } /* endif */
   }
   return( sRc );
}

SHORT  QDAMLastEntry_V3
(
   PBTREE  pBTIda,
   PBTREEBUFFER_V3  * ppRecord               // pointer to pointer of record
)
{
   SHORT  sRc = 0;
   RECPARAM     recData;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // read in root record
   sRc = QDAMReadRecord_V3( pBTIda, pBT->usFirstNode, ppRecord, FALSE );
   if ( !sRc )
   {
      while (!sRc && !IS_LEAF(*ppRecord))
      {
         recData = QDAMGetrecData_V3( *ppRecord, (SHORT)(OCCUPIED(*ppRecord)-1), pBT->usVersion );
         sRc = QDAMReadRecord_V3( pBTIda, recData.usNum, ppRecord, FALSE  );
      } /* endwhile */
      if ( !sRc )
      {
         pBTIda->usCurrentRecord = RECORDNUM(*ppRecord);
         pBTIda->sCurrentIndex = (SHORT) (OCCUPIED(*ppRecord) - 1);
      } /* endif */
   }
   return( sRc );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFirstEntry      Position at first entry
//------------------------------------------------------------------------------
// Function call:     QDAMFirstEntry ( PBTREE, PPBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       This function will position to the first entry in the
//                    dictionary
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PPBTREEBUFFER          pointer to selected record
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EOF_REACHED end or start of dictionary reached
//
//------------------------------------------------------------------------------
// Function flow:     read in root record and set Rc
//                    if okay then
//                      while okay and record not leaf
//                        read in record pointed to by first entry
//                      endwhile
//                      if okay then
//                        set current record and current index
//                      endif
//                    endif
//                    return Rc
//
//------------------------------------------------------------------------------
SHORT  QDAMFirstEntry_V2
(
   PBTREE  pBTIda,
   PBTREEBUFFER_V2  * ppRecord               // pointer to pointer of record
)
{
   SHORT  sRc = 0;
   RECPARAM     recData;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // read in root record
   sRc = QDAMReadRecord_V2( pBTIda, pBT->usFirstNode, ppRecord, FALSE );
   if ( !sRc )
   {
      while (!sRc && !IS_LEAF(*ppRecord))
      {
         recData = QDAMGetrecData_V2( *ppRecord, 0, pBT->usVersion);
         sRc = QDAMReadRecord_V2( pBTIda, recData.usNum, ppRecord , FALSE );
      } /* endwhile */
      if ( !sRc )
      {
         pBTIda->usCurrentRecord = RECORDNUM(*ppRecord);
         pBT->usFirstLeaf = pBTIda->usCurrentRecord;    // determine first leaf
         pBTIda->sCurrentIndex = 0;
      } /* endif */
   } /* endif */

   return (sRc);
}

SHORT  QDAMFirstEntry_V3
(
   PBTREE  pBTIda,
   PBTREEBUFFER_V3  * ppRecord               // pointer to pointer of record
)
{
   SHORT  sRc = 0;
   RECPARAM     recData;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // read in root record
   sRc = QDAMReadRecord_V3( pBTIda, pBT->usFirstNode, ppRecord, FALSE );
   if ( !sRc )
   {
      while (!sRc && !IS_LEAF(*ppRecord))
      {
         recData = QDAMGetrecData_V3( *ppRecord, 0, pBT->usVersion);
         sRc = QDAMReadRecord_V3( pBTIda, recData.usNum, ppRecord , FALSE );
      } /* endwhile */
      if ( !sRc )
      {
         pBTIda->usCurrentRecord = RECORDNUM(*ppRecord);
         pBT->usFirstLeaf = pBTIda->usCurrentRecord;    // determine first leaf
         pBTIda->sCurrentIndex = 0;
      } /* endif */
   } /* endif */

   return (sRc);
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetrecData   Get data pointer back
//------------------------------------------------------------------------------
// Function call:     QDAMGetrecData( PBTREEBUFFER, SHORT );
//
//------------------------------------------------------------------------------
// Description:       Get data record pointer back
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer
//                    SHORT                  key to be used
//
//
//------------------------------------------------------------------------------
// Returncode type:   RECPARAM
//------------------------------------------------------------------------------
// Returncodes:       filled recparam structure
//
//------------------------------------------------------------------------------
// Function flow:     use offset in table and point to data description
//                    copy it data description structure for return
//
//------------------------------------------------------------------------------
RECPARAM  QDAMGetrecData_V2
(
   PBTREEBUFFER_V2  pRecord,
   SHORT         sMid,                           // key number
   USHORT        usVersion                       // version of database
)
{
   PCHAR   pData = NULL;
   RECPARAM      recData;               // data description structure
   PUSHORT  pusOffset;

   // use record number of passed entry , read in record and pass
   // back pointer
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += sMid;                            // point to key
   pData = (PCHAR)(pRecord->contents.uchData + *pusOffset);
   pData += sizeof(USHORT );                    // get pointer to datarec

   if ( usVersion >= NTM_VERSION2 )
   {
     memcpy( &recData, (PRECPARAM) pData, sizeof(RECPARAM ) );
   }
   else
   {
     RECPARAMOLD recDataOld;

     memcpy( &recDataOld, (PRECPARAMOLD) pData, sizeof(RECPARAMOLD) );
     recData.usOffset = recDataOld.usOffset;
     recData.usNum    = recDataOld.usNum;
     recData.ulLen    = (ULONG)recDataOld.sLen;
   } /* endif */
   return ( recData );
}
RECPARAM  QDAMGetrecData_V3
(
   PBTREEBUFFER_V3  pRecord,
   SHORT         sMid,                           // key number
   USHORT        usVersion                       // version of database
)
{
   PCHAR   pData = NULL;
   RECPARAM      recData;               // data description structure
   PUSHORT  pusOffset;

   // use record number of passed entry , read in record and pass
   // back pointer
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += sMid;                            // point to key
   pData = (PCHAR)(pRecord->contents.uchData + *pusOffset);
   pData += sizeof(USHORT );                    // get pointer to datarec

   if ( usVersion >= NTM_VERSION2 )
   {
     memcpy( &recData, (PRECPARAM) pData, sizeof(RECPARAM ) );
   }
   else
   {
     RECPARAMOLD recDataOld;

     memcpy( &recDataOld, (PRECPARAMOLD) pData, sizeof(RECPARAMOLD) );
     recData.usOffset = recDataOld.usOffset;
     recData.usNum    = recDataOld.usNum;
     recData.ulLen    = (ULONG)recDataOld.sLen;
   } /* endif */
   return ( recData );
}
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetrecDataLen  get data length
//------------------------------------------------------------------------------
// Function call:     QDAMGetrecDataLen( PBTREEBUFFER, SHORT )
//
//------------------------------------------------------------------------------
// Description:       Get data length record pointer back
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer
//                    SHORT                  key to be used
//
//------------------------------------------------------------------------------
// Returncode type:   ULONG
//------------------------------------------------------------------------------
// Returncodes:       length of data
//
//------------------------------------------------------------------------------
// Function flow:     use offset in table and point to data description
//                    return length indication
//
//------------------------------------------------------------------------------
ULONG QDAMGetrecDataLen_V2
(
   PBTREE   pBTIda,
   PBTREEBUFFER_V2  pRecord,
   SHORT         sMid                            // key number
)
{
   PCHAR   pData = NULL;
   PUSHORT  pusOffset;
   PBTREEGLOB  pBT = pBTIda->pBTree;
   ULONG    ulLength;

   // use record number of passed entry , read in record and pass
   // back pointer
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += sMid;                                      // point to key
   /*******************************************************************/
   /* pusOffset should only be in the allowed range                   */
   /*******************************************************************/
   pData = (PCHAR)(pRecord->contents.uchData + *pusOffset);
   if ( pBT->usVersion >= NTM_VERSION2 )
   {
     ulLength = *(PULONG)pData;        // get length
     ulLength &= ~QDAM_TERSE_FLAGL;    // get rid off any terse flag
   }
   else
   {
     USHORT usLength = *(PUSHORT)pData;
     usLength &= ~QDAM_TERSE_FLAG;     // get rid off any terse flag
     ulLength = (ULONG)usLength;
   } /* endif */
   return( ulLength );
}

ULONG QDAMGetrecDataLen_V3
(
   PBTREE   pBTIda,
   PBTREEBUFFER_V3  pRecord,
   SHORT         sMid                            // key number
)
{
   PCHAR   pData = NULL;
   PUSHORT  pusOffset;
   PBTREEGLOB  pBT = pBTIda->pBTree;
   ULONG    ulLength;

   // use record number of passed entry , read in record and pass
   // back pointer
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += sMid;                                      // point to key
   /*******************************************************************/
   /* pusOffset should only be in the allowed range                   */
   /*******************************************************************/
   pData = (PCHAR)(pRecord->contents.uchData + *pusOffset);
   if ( pBT->usVersion >= NTM_VERSION2 )
   {
     ulLength = *(PULONG)pData;        // get length
     ulLength &= ~QDAM_TERSE_FLAGL;    // get rid off any terse flag
   }
   else
   {
     USHORT usLength = *(PUSHORT)pData;
     usLength &= ~QDAM_TERSE_FLAG;     // get rid off any terse flag
     ulLength = (ULONG)usLength;
   } /* endif */
   return( ulLength );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetrecKey     Get key record pointer
//------------------------------------------------------------------------------
// Function call:     QDAMGetrecKey( PBTREEBUFFER, SHORT );
//
//------------------------------------------------------------------------------
// Description:       Get data for key record pointer back
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer
//                    SHORT                  key to be used
//
//------------------------------------------------------------------------------
// Returncode type:   RECPARAM
//------------------------------------------------------------------------------
// Returncodes:       return filled data record
//
//------------------------------------------------------------------------------
// Function flow:     use offset in table and point to key  description
//                    copy key     description structure for return
//
//
//------------------------------------------------------------------------------
RECPARAM  QDAMGetrecKey_V2
(
   PBTREEBUFFER_V2  pRecord,
   SHORT         sMid                            // key number
)
{
   RECPARAM      recData;               // data description structure
   PUSHORT  pusOffset;

   // use record number of passed entry , read in record and pass
   // back pointer
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += sMid;                            // point to key

   recData.usNum  = pRecord->usRecordNumber;
   recData.usOffset  = *pusOffset;
   recData.ulLen = *(PUSHORT) (pRecord->contents.uchData +*pusOffset);
   return ( recData );
}

RECPARAM  QDAMGetrecKey_V3
(
   PBTREEBUFFER_V3  pRecord,
   SHORT         sMid                            // key number
)
{
   RECPARAM      recData;               // data description structure
   PUSHORT  pusOffset;

   // use record number of passed entry , read in record and pass
   // back pointer
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += sMid;                            // point to key

   recData.usNum  = pRecord->usRecordNumber;
   recData.usOffset  = *pusOffset;
   recData.ulLen = *(PUSHORT) (pRecord->contents.uchData +*pusOffset);
   return ( recData );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMSetrecData    Set Data Record Pointer
//------------------------------------------------------------------------------
// Function call:     QDAMSetrecData( PBTREEBUFFER, SHORT, RECPARAM );
//
//------------------------------------------------------------------------------
// Description:       Set data record pointer
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer
//                    SHORT                  key to be used
//                    RECPARAM               record pointer
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     get pointer to offset
//                    copy passed record structure at this place
//                    return
//------------------------------------------------------------------------------
VOID QDAMSetrecData_V2
(
   PBTREEBUFFER_V2  pRecord,
   SHORT         sMid,                           // key number
   RECPARAM      recData,                        // data pointer
   USHORT        usVersion                       // version of database
)
{
   PCHAR   pData = NULL;
   PCHAR   pEndOfRec;
   PUSHORT  pusOffset;
   usVersion;

   // get max pointer value
   pEndOfRec = (PCHAR)&(pRecord->contents) + BTREE_REC_SIZE_V2;


   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += sMid;                            // point to key

   if ( (PCHAR)pusOffset > pEndOfRec )
   {
     // offset pointer is out of range
     pData = NULL;
     ERREVENT2( QDAMSETRECDATA_LOC, INTFUNCFAILED_EVENT, 1, DB_GROUP, NULL );
   }
   else
   {
     pData = (PCHAR)(pRecord->contents.uchData + *pusOffset);
     pData += sizeof(USHORT );                    // get pointer to datarec

     if ( pData > pEndOfRec )
     {
       // data pointer is out of range
       pData = NULL;
       ERREVENT2( QDAMSETRECDATA_LOC, INTFUNCFAILED_EVENT, 2, DB_GROUP, NULL );
     }
     else
     {
       if ( usVersion >= NTM_VERSION2 )
       {
         memcpy( (PRECPARAM) pData, &recData, sizeof(RECPARAM ) );
       }
       else
       {
         RECPARAMOLD recDataOld;

         // convert to old style RECPARAMs before setting in record
         recDataOld.sLen     = (SHORT)recData.ulLen;
         recDataOld.usOffset = recData.usOffset;
         recDataOld.usNum    = recData.usNum;
         memcpy( (PRECPARAMOLD) pData, &recDataOld, sizeof(RECPARAMOLD) );
       } /* endif */
     } /* endif */

     // re-compute record checksum
//   pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
   } /* endif */

   return ;
}

VOID QDAMSetrecData_V3
(
   PBTREEBUFFER_V3  pRecord,
   SHORT         sMid,                           // key number
   RECPARAM      recData,                        // data pointer
   USHORT        usVersion                       // version of database
)
{
   PCHAR   pData = NULL;
   PCHAR   pEndOfRec;
   PUSHORT  pusOffset;
   usVersion;

   // get max pointer value
   pEndOfRec = (PCHAR)&(pRecord->contents) + BTREE_REC_SIZE_V3;


   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += sMid;                            // point to key

   if ( (PCHAR)pusOffset > pEndOfRec )
   {
     // offset pointer is out of range
     pData = NULL;
     ERREVENT2( QDAMSETRECDATA_LOC, INTFUNCFAILED_EVENT, 1, DB_GROUP, NULL );
   }
   else
   {
     pData = (PCHAR)(pRecord->contents.uchData + *pusOffset);
     pData += sizeof(USHORT );                    // get pointer to datarec

     if ( pData > pEndOfRec )
     {
       // data pointer is out of range
       pData = NULL;
       ERREVENT2( QDAMSETRECDATA_LOC, INTFUNCFAILED_EVENT, 2, DB_GROUP, NULL );
     }
     else
     {
       if ( usVersion >= NTM_VERSION2 )
       {
         memcpy( (PRECPARAM) pData, &recData, sizeof(RECPARAM ) );
       }
       else
       {
         RECPARAMOLD recDataOld;

         // convert to old style RECPARAMs before setting in record
         recDataOld.sLen     = (SHORT)recData.ulLen;
         recDataOld.usOffset = recData.usOffset;
         recDataOld.usNum    = recData.usNum;
         memcpy( (PRECPARAMOLD) pData, &recDataOld, sizeof(RECPARAMOLD) );
       } /* endif */
     } /* endif */

     // re-compute record checksum
//   pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
   } /* endif */

   return ;
}





//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMAllocKeyRecords   Allocate space for key records
//------------------------------------------------------------------------------
// Function call:     QDAMAllocKeyRecords( PBTREEBUFFER, USHORT );
//
//------------------------------------------------------------------------------
// Description:       Allocate a chunk of records for keys
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer from where to copy
//                    USHORT                 number of records to be allocated
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     allocate the number of requested records and
//                    put them into a linked list
//
//------------------------------------------------------------------------------
SHORT QDAMAllocKeyRecords
(
   PBTREE pBTIda,
   USHORT usNum
)
{
   SHORT  sRc = 0;
   PBTREEHEADER  pHeader;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   if ( pBT->bRecSizeVersion == BTREE_V3 )
   {
      PBTREEBUFFER_V3  pRecord;
      while ( usNum-- && !sRc )
      {
          pBT->usNextFreeRecord++;
          sRc = QDAMReadRecord_V3( pBTIda, pBT->usNextFreeRecord, &pRecord, TRUE  );

          if ( !sRc )
          {
            // add free record to the free list
            NEXT( pRecord ) = pBT->usFreeKeyBuffer;
            PREV( pRecord ) = 0;
            pBT->usFreeKeyBuffer = RECORDNUM( pRecord );

            // init this record
            pHeader = &pRecord->contents.header;
            pHeader->usNum = pBT->usFreeKeyBuffer;
            pHeader->usOccupied = 0;
            pHeader->usFilled = sizeof(BTREEHEADER );
            pHeader->usLastFilled = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER );

            /*************************************************************/
            /* force write                                               */
            /*************************************************************/
            sRc = QDAMWRecordToDisk_V3( pBTIda, pRecord);
          } /* endif */
      } /* endwhile */
   }
   else
   {
      PBTREEBUFFER_V2  pRecord;
      while ( usNum-- && !sRc )
      {
          pBT->usNextFreeRecord++;
          sRc = QDAMReadRecord_V2( pBTIda, pBT->usNextFreeRecord, &pRecord, TRUE  );

          if ( !sRc )
          {
            // add free record to the free list
            NEXT( pRecord ) = pBT->usFreeKeyBuffer;
            PREV( pRecord ) = 0;
            pBT->usFreeKeyBuffer = RECORDNUM( pRecord );

            // init this record
            pHeader = &pRecord->contents.header;
            pHeader->usNum = pBT->usFreeKeyBuffer;
            pHeader->usOccupied = 0;
            pHeader->usFilled = sizeof(BTREEHEADER );
            pHeader->usLastFilled = BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER );

            /*************************************************************/
            /* force write                                               */
            /*************************************************************/
            sRc = QDAMWRecordToDisk_V2( pBTIda, pRecord);
          } /* endif */
      } /* endwhile */
   } /* endif */

   return sRc;
}

//------------------------------------------------------------------------------
// Internal Function
//------------------------------------------------------------------------------
// Function name:     QDAMAddToIndexList   Add record to index list
//------------------------------------------------------------------------------
// Function call:     QDAMAddToIndexList( PBTREE, PBTREEBUFFER  );
//------------------------------------------------------------------------------
// Description:       Add index buffer to cached buffer list
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE         pointer to btree instance
//                    PBTREEBUFFER   pointer to one buffer record
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     get pointer to index cache
//                    if index to be stored already exists then
//                      begin
//                         ignore request
//                      end
//                    else
//                      begin
//                         allocate memory and fill it with buffer
//                         if okay put memory at appropriate place into list
//                         else ignore request
//                         endif
//                      end
//                    endif
//------------------------------------------------------------------------------
VOID  QDAMAddToIndexList_V2
(
   PBTREE       pBTIda,             // pointer to BTree
   PBTREEBUFFER_V2 pBuffer             // pointer to buffer
)
{
   PBTREEINDEX_V2  pIndex;             // pointer to index
   BOOL         fFound = FALSE;     // not found yet
   USHORT       usNum;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // get number of active record
   usNum = pBuffer->contents.header.usNum;
   pIndex = pBT->pIndexBuffer_V2;

   while ( pIndex && !fFound )
   {
      if (pIndex->btreeBuffer.usRecordNumber != usNum)
      {
         pIndex = pIndex->pNext;    // point to next stored index
      }
      else
      {
         fFound = TRUE;             // indicate buffer found
      } /* endif */
   } /* endwhile */


   /*******************************************************************/
   /* if found copy data into provided buffer, else try to allocate   */
   /* new buffer and copy data                                        */
   /*******************************************************************/
   if ( fFound )
   {
     INC_CACHED_COUNT;
     memcpy( &pIndex->btreeBuffer, pBuffer, sizeof(BTREEBUFFER_V2 ));
     pIndex->btreeBuffer.usRecordNumber = usNum;
   }
   else if ( pBT->usIndexBuffer < INDEX_BUFFERS )
   {
      //  allocate memory and fill it with buffer
      UtlAlloc( (PVOID *)&pIndex, 0L, (LONG) sizeof(BTREEINDEX_V2 ) , NOMSG );
      //  if okay put memory at appropriate place into list else ignore request
      if ( pIndex )
      {
         INC_CACHED_COUNT;
         if ( pBT->pIndexBuffer_V2)
         {
            pIndex->pNext = pBT->pIndexBuffer_V2;
         } /* endif */
         pBT->pIndexBuffer_V2 = pIndex;
         pBT->usIndexBuffer++;            // incr. allocated buffers
         memcpy( &pIndex->btreeBuffer, pBuffer, sizeof(BTREEBUFFER_V2));
         pIndex->btreeBuffer.usRecordNumber = usNum;
      } /* endif */
   } /* endif */
}

VOID  QDAMAddToIndexList_V3
(
   PBTREE       pBTIda,             // pointer to BTree
   PBTREEBUFFER_V3 pBuffer             // pointer to buffer
)
{
   PBTREEINDEX_V3  pIndex;             // pointer to index
   BOOL         fFound = FALSE;     // not found yet
   USHORT       usNum;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // get number of active record
   usNum = pBuffer->contents.header.usNum;
   pIndex = pBT->pIndexBuffer_V3;

   while ( pIndex && !fFound )
   {
      if (pIndex->btreeBuffer.usRecordNumber != usNum)
      {
         pIndex = pIndex->pNext;    // point to next stored index
      }
      else
      {
         fFound = TRUE;             // indicate buffer found
      } /* endif */
   } /* endwhile */


   /*******************************************************************/
   /* if found copy data into provided buffer, else try to allocate   */
   /* new buffer and copy data                                        */
   /*******************************************************************/
   if ( fFound )
   {
     INC_CACHED_COUNT;
     memcpy( &pIndex->btreeBuffer, pBuffer, sizeof(BTREEBUFFER_V3 ));
     pIndex->btreeBuffer.usRecordNumber = usNum;
   }
   else if ( pBT->usIndexBuffer < INDEX_BUFFERS )
   {
      //  allocate memory and fill it with buffer
      UtlAlloc( (PVOID *)&pIndex, 0L, (LONG) sizeof(BTREEINDEX_V3 ) , NOMSG );
      //  if okay put memory at appropriate place into list else ignore request
      if ( pIndex )
      {
         INC_CACHED_COUNT;
         if ( pBT->pIndexBuffer_V3)
         {
            pIndex->pNext = pBT->pIndexBuffer_V3;
         } /* endif */
         pBT->pIndexBuffer_V3 = pIndex;
         pBT->usIndexBuffer++;            // incr. allocated buffers
         memcpy( &pIndex->btreeBuffer, pBuffer, sizeof(BTREEBUFFER_V3));
         pIndex->btreeBuffer.usRecordNumber = usNum;
      } /* endif */
   } /* endif */
}

//------------------------------------------------------------------------------
// Internal Function
//------------------------------------------------------------------------------
// Function name:     QDAMFetchFromIndexList   get index record from list
//------------------------------------------------------------------------------
// Function call:     QDAMFetchFromIndexList( PBTREE, PBTREEBUFFER, USHORT );
//------------------------------------------------------------------------------
// Description:       get index record from internal list
//------------------------------------------------------------------------------
// Parameters:        PBTREE         pointer to btree instance
//                    PBTREEBUFFER   pointer to one buffer record
//                    USHORT         record number searched for
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       record searched for found
//                    FALSE      record searched for not in cached list
//------------------------------------------------------------------------------
// Function flow:     get pointer to index cache
//                    set found to false
//                    while not found && pIndex
//                      if found then
//                        copy address
//                        set found to True
//                      else
//                        point to next index
//                      endif
//                    endwhile
//                    return found index
//------------------------------------------------------------------------------
BOOL QDAMFetchFromIndexList_V2
(
   PBTREE       pBTIda,             // pointer to BTree
   PBTREEBUFFER_V2 pBuffer,            // pointer to buffer
   USHORT       usNumber            // number of current record
)
{
   PBTREEINDEX_V2  pIndex;             // pointer to index
   BOOL         fFound = FALSE;     // record not found yet
   PBTREEGLOB    pBT = pBTIda->pBTree;

   pIndex = pBT->pIndexBuffer_V2;

   while ( pIndex && !fFound )
   {
      if (pIndex->btreeBuffer.usRecordNumber != usNumber)
      {
         pIndex = pIndex->pNext;    // point to next stored index
      }
      else
      {
         fFound = TRUE;             // indicate buffer found
         memcpy( pBuffer, &pIndex->btreeBuffer, sizeof(BTREEBUFFER_V2));
         INC_CACHED_READ_COUNT;
      } /* endif */
   } /* endwhile */
   return fFound;
}
BOOL QDAMFetchFromIndexList_V3
(
   PBTREE       pBTIda,             // pointer to BTree
   PBTREEBUFFER_V3 pBuffer,            // pointer to buffer
   USHORT       usNumber            // number of current record
)
{
   PBTREEINDEX_V3  pIndex;             // pointer to index
   BOOL         fFound = FALSE;     // record not found yet
   PBTREEGLOB    pBT = pBTIda->pBTree;

   pIndex = pBT->pIndexBuffer_V3;

   while ( pIndex && !fFound )
   {
      if (pIndex->btreeBuffer.usRecordNumber != usNumber)
      {
         pIndex = pIndex->pNext;    // point to next stored index
      }
      else
      {
         fFound = TRUE;             // indicate buffer found
         memcpy( pBuffer, &pIndex->btreeBuffer, sizeof(BTREEBUFFER_V3));
         INC_CACHED_READ_COUNT;
      } /* endif */
   } /* endwhile */
   return fFound;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMPhysLock           Physical lock file
//------------------------------------------------------------------------------
// Function call:     QDAMPhysLock( PBTREE, BOOL, PBOOL );
//
//------------------------------------------------------------------------------
// Description:       lock or unlock the database
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The database to be locked
//                    BOOL               TRUE = LOCK, FALSE = Unlock
//                    PBOOL              ptr to locked flag (set to TRUE if
//                                       locking was successful
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
SHORT QDAMPhysLock
(
   PBTREE         pBTIda,
   BOOL           fLock,
   PBOOL          pfLocked
)
{
  SHORT    sRc = 0;                    // return code
  PBTREEGLOB  pBT = pBTIda->pBTree;
  FILELOCK  Lock;                      // range to be locked or unlocked

  if ( pfLocked != NULL ) *pfLocked = FALSE;  // set initial value
  Lock.lOffset = 0L;                   // start at begin of file
  Lock.lRange  = 0x7FFFFFFF;

  if ( fLock )
  {
    if ( !pBTIda->fPhysLock )
    {
       SHORT sRetries = MAX_RETRY_COUNT;

       // lock update counter file
       do
       {
         sRc = UtlFileLocks( pBT->fpDummy, (PFILELOCK)NULL, &Lock, FALSE );
         if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

         if ( sRc == BTREE_IN_USE )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( (sRc == BTREE_IN_USE) && (sRetries > 0));

       // lock database file (here we do not use a retry loop as the database
       // file should not be locked if the update counter file was not locked)
       if ( sRc == NO_ERROR )
       {
         sRc = UtlFileLocks( pBT->fp, (PFILELOCK)NULL, &Lock, FALSE );
         if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

         // unlock update counter file in case of errors
         if ( sRc != NO_ERROR )
         {
           UtlFileLocks( pBT->fpDummy, &Lock, (PFILELOCK)NULL, FALSE );
         } /* endif */
       } /* endif */

       if ( sRc == NO_ERROR )
       {
         pBTIda->fPhysLock = TRUE;
         if ( pfLocked != NULL ) *pfLocked = TRUE;

         // ensure we work with up-to-date data
         sRc = QDAMCheckForUpdates( pBTIda );
       } /* endif */
    } /* endif */
  }
  else
  {
     // Set update counter in case of database updates
    if ( pBT->fUpdated )
    {
      sRc = QDAMDictFlushLocal( pBTIda );
      if ( sRc == NO_ERROR )
      {
        sRc = QDAMWriteHeader( pBTIda );
      } /* endif */
      QDAMIncrUpdCounter( pBTIda, 0, NULL );
      pBT->fUpdated = FALSE;
    } /* endif */

    // unlock database file
    sRc = UtlFileLocks( pBT->fp, &Lock, (PFILELOCK)NULL, FALSE );

    // unlock update counter file
    if ( sRc == NO_ERROR )
    {
      UtlFileLocks( pBT->fpDummy, &Lock, (PFILELOCK)NULL, FALSE );
      pBTIda->fPhysLock = FALSE;
    } /* endif */
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMPHYSLOCK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
//    sRc = BTREE_IN_USE;
  } /* endif */

  return( sRc ) ;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMCheckForUpdates    Check is database has been changed
//------------------------------------------------------------------------------
// Function call:     QDAMCheckForUpdates( PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Check if the QDAm database has been modified since the
//                    last read or write operation. If a modification is
//                    detected all internal buffers are cleared thus forcing
//                    read of data from disk.
//                    Only shared databases are handled this way. For all
//                    other databases this function is a NOP
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The database to be checked for updates
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
SHORT QDAMCheckForUpdates
(
   PBTREE         pBTIda
)
{
  SHORT    sRc = 0;                             // return code
  PBTREEGLOB  pBT = pBTIda->pBTree;


  if ( pBT->usOpenFlags & ASD_SHARED )
  {
      LONG  lUpdCount;                 // buffer for new value of update counter

     /**********************************************************************/
     /* Get current database update count                                  */
     /**********************************************************************/
     sRc = QDAMGetUpdCounter( pBTIda, &lUpdCount, 0, 1 );

     if ( !sRc )
     {
      if ( lUpdCount != pBT->alUpdCtr[0] )
      {
        USHORT usNumBytesRead;          // Buffer fornumber of bytes read
        ULONG  ulNewOffset;             // new offset in file

        INFOEVENT2( QDAMCHECKFORUPDATES_LOC, REFRESH_EVENT, 0, DB_GROUP, NULL );

        /**********************************************************************/
        /* Get current header record                                          */
        /**********************************************************************/
        sRc = UtlChgFilePtr( pBT->fp, 0L, FILE_BEGIN, &ulNewOffset, FALSE);
        if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

        if ( sRc == NO_ERROR )
        {
           SHORT sRetries = 0; // MAX_RETRY_COUNT;

           do
           {
             sRc = UtlRead( pBT->fp, (PVOID)&header,
                           sizeof(BTREEHEADRECORD), &usNumBytesRead, FALSE);
             if ( sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

             if (sRc == BTREE_IN_USE )
             {
//             UtlWait( MAX_WAIT_TIME );
               sRetries--;
             } /* endif */
           } while ( (sRc == BTREE_IN_USE) && (sRetries > 0) );
        } /* endif */

        /****************************************************************/
        /* Use new update count as time of last update                   */
        /****************************************************************/
        if ( !sRc )
        {
          pBT->alUpdCtr[0] = lUpdCount;
        } /* endif */

        /****************************************************************/
        /* Refresh internal info with data from header record           */
        /****************************************************************/
        if ( !sRc )
        {
          pBT->usFirstNode        = header.usFirstNode;
          pBT->usFirstLeaf        = header.usFirstLeaf;
          pBT->usFreeKeyBuffer    = header.usFreeKeyBuffer;
          pBT->usFreeDataBuffer   = header.usFreeDataBuffer;
          pBT->usFirstDataBuffer  = header.usFirstDataBuffer;
          // DataRecList in header is in old format (RECPARAMOLD),
          // so convert it to the new format (RECPARAM)
          {
            int i;
            for ( i = 0; i < MAX_LIST; i++ )
            {
              pBT->DataRecList[i].usOffset = header.DataRecList[i].usOffset;
              pBT->DataRecList[i].usNum    = header.DataRecList[i].usNum;
              pBT->DataRecList[i].ulLen    = (ULONG)header.DataRecList[i].sLen;
            } /* endfor */
          }
          memcpy( pBT->chCollate, header.chCollate, COLLATE_SIZE );
          memcpy( pBT->chCaseMap, header.chCaseMap, COLLATE_SIZE );
          memcpy( pBT->chEntryEncode, header.chEntryEncode, ENTRYENCODE_LEN );

          // Get value for next free record
          if ( header.Flags.bVersion == BTREE_V1 )
          {
            pBT->usNextFreeRecord = header.usNextFreeRecord;
          }
          else
          {
            USHORT     usNextFreeRecord;
            ULONG      ulTemp;
            sRc = UtlGetFileSize( pBT->fp, &ulTemp, FALSE );
            if ( !sRc )
            {
              usNextFreeRecord = (USHORT)(ulTemp/pBT->usBtreeRecSize);
              if ( usNextFreeRecord != pBT->usNextFreeRecord )
              {
                INFOEVENT2( QDAMCHECKFORUPDATES_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
              } /* endif */
              pBT->usNextFreeRecord = usNextFreeRecord;
            } /* endif */
          } /* endif */
        } /* endif */

        /****************************************************************/
        /* Free all index pages                                         */
        /****************************************************************/
        if ( !sRc )
        {
          if ( pBT->bRecSizeVersion == BTREE_V3 )
          {
            PBTREEINDEX_V3  pIndexBuffer;             // temp ptr to index buffers

            while ( pBT->pIndexBuffer_V3 != NULL )
            {
              pIndexBuffer = pBT->pIndexBuffer_V3;
              pBT->pIndexBuffer_V3 = pIndexBuffer->pNext;

              UtlAlloc( (PVOID *)&pIndexBuffer, 0L, 0l, NOMSG );
            } /* endwhile */
          }
          else
          {
            PBTREEINDEX_V2  pIndexBuffer;             // temp ptr to index buffers

            while ( pBT->pIndexBuffer_V2 != NULL )
            {
              pIndexBuffer = pBT->pIndexBuffer_V2;
              pBT->pIndexBuffer_V2 = pIndexBuffer->pNext;

              UtlAlloc( (PVOID *)&pIndexBuffer, 0L, 0l, NOMSG );
            } /* endwhile */
          } /* endif */
          pBT->usIndexBuffer = 0;      // no buffers in linked list anymore
        } /* endif */

        /****************************************************************/
        /* Invalidate all data buffers                                  */
        /****************************************************************/
        /* Free allocated space for buffers */
        if ( pBT->bRecSizeVersion == BTREE_V3 )
        {
          if ( !sRc && pBT->LookupTable_V3 )
          {
            USHORT i;
            PLOOKUPENTRY_V3 pLEntry = pBT->LookupTable_V3;

            for ( i=0; i < pBT->usNumberOfLookupEntries; i++ )
            {
              if ( pLEntry->pBuffer )
              {
                UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L, NOMSG );
              } /* endif */
              pLEntry++;
            } /* endfor */
            pBT->usNumberOfAllocatedBuffers = 0;
          } /* endif */

        }
        else
        {
          if ( !sRc && pBT->LookupTable_V2 )
          {
            USHORT i;
            PLOOKUPENTRY_V2 pLEntry = pBT->LookupTable_V2;

            for ( i=0; i < pBT->usNumberOfLookupEntries; i++ )
            {
              if ( pLEntry->pBuffer )
              {
                UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L, NOMSG );
              } /* endif */
              pLEntry++;
            } /* endfor */
            pBT->usNumberOfAllocatedBuffers = 0;
          } /* endif */

        } /* endif */
        /****************************************************************/
        /* Invalidate current record and current index                  */
        /****************************************************************/
        if ( !sRc )
        {
          pBTIda->sCurrentIndex = RESET_VALUE;
          pBTIda->usCurrentRecord = 0;
        } /* endif */
      } /* endif */
     } /* endif */
  } /* endif */

  if ( sRc != 0 )
  {
    ERREVENT2( QDAMCHECKFORUPDATES_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  return( sRc ) ;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMIncrUpdCounter      Inrement database update counter
//------------------------------------------------------------------------------
// Function call:     QDAMIncrUpdCounter( PBTREE, SHORT sIndex )
//
//------------------------------------------------------------------------------
// Description:       Update one of the update counter field in the dummy
//                    /locked terms file
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    SHORT                  index of counter field
//                    PLONG                  ptr to buffer for new counte value
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     read update counter from dummy file
//                    increment update counter
//                    position ptr to begin of file
//                    write update counter to disk
//------------------------------------------------------------------------------
SHORT QDAMIncrUpdCounter
(
   PBTREE     pBTIda,                  // pointer to btree structure
   SHORT      sIndex,                  // index of update counter
   PLONG      plNewValue               // ptr to buffer for new counte value
)
{
  SHORT       sRc=0;
  USHORT      usNumBytes;              // number of bytes written or read
  ULONG       ulNewOffset;             // new offset in file
  PBTREEGLOB  pBT = pBTIda->pBTree;
  SHORT       sRetries;                // number of retries
  LONG     lNewUpdCounter;      // buffer for new update counter

  lNewUpdCounter = 0L;
  sRetries = MAX_RETRY_COUNT;
  do
  {
     // Position to requested update counter
    sRc = UtlChgFilePtr( pBT->fpDummy, (LONG)(sizeof(LONG)*sIndex),
                         FILE_BEGIN, &ulNewOffset, FALSE);
    if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

    // Read current update counter
     if ( !sRc )
     {
      sRc = UtlRead( pBT->fpDummy, (PVOID)&lNewUpdCounter, sizeof(LONG),
                     &usNumBytes, FALSE);
      if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );
     } /* endif */

     // Increment update counter
     if ( !sRc )
     {
       lNewUpdCounter++;
        pBT->alUpdCtr[sIndex] = lNewUpdCounter;;
        if ( plNewValue )
        {
          *plNewValue = pBT->alUpdCtr[sIndex];
        } /* endif */
     } /*endif */

     // Position to requested update counter
     if ( !sRc )
     {
      sRc = UtlChgFilePtr( pBT->fpDummy, (LONG)(sizeof(LONG)*sIndex),
                           FILE_BEGIN, &ulNewOffset, FALSE);
      if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );
     } /* endif */

     //  Rewrite update counter
     if ( !sRc )
     {
       sRc = UtlWrite( pBT->fpDummy, (PVOID)&lNewUpdCounter,
                       sizeof(LONG), &usNumBytes, FALSE );
       if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_WRITE_ERROR, pBT->usOpenFlags );
     } /* endif */

    if ( sRc == BTREE_IN_USE )
    {
      UtlWait( MAX_WAIT_TIME );
      sRetries--;
    } /* endif */
  } while ( (sRc == BTREE_IN_USE) && (sRetries > 0) );

  if ( sRc )
  {
    ERREVENT2( QDAMINCRUPDCOUNTER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  return sRc;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetUpdCounter       Get database update counter
//------------------------------------------------------------------------------
// Function call:     QDAMGetUpdCounter( PBTREE, PLONG, SHORT, SHORT );
//------------------------------------------------------------------------------
// Description:       Get one or more of the the database update counters
//                    from the dummy/locked terms file
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PLONG                  ptr to buffer for update counter
//                    SHORT                  index of requested update counter
//                    SHORT                  number of counters requested
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_READ_RROR   read error from disk
//
//------------------------------------------------------------------------------
// Function flow:     read update counter from dummy file
//------------------------------------------------------------------------------
 SHORT QDAMGetUpdCounter
(
   PBTREE     pBTIda,                   // pointer to btree structure
   PLONG      plUpdCount,               // ptr to buffer for update counter
   SHORT      sIndex,                   // index of requested update counter
   SHORT      sNumCounters              // number of counters requested
)
{
  SHORT  sRc=0;
  USHORT usNumBytes;                   // number of bytes written or read
  ULONG  ulNewOffset;                  // new offset in file
  PBTREEGLOB  pBT = pBTIda->pBTree;
  SHORT      sRetries;                 // number of retries

  sRetries = 0; // MAX_RETRY_COUNT;
  do
  {
     // Position to requested update counter
    sRc = UtlChgFilePtr( pBT->fpDummy, (LONG)(sizeof(LONG)*sIndex),
                         FILE_BEGIN, &ulNewOffset, FALSE);
    sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

    // Read requested update counter(s)
    if ( !sRc )
    {
      memset( plUpdCount, 0, sizeof(LONG)*sNumCounters );

      sRc = UtlRead( pBT->fpDummy, (PVOID)plUpdCount,
                     (USHORT)(sizeof(LONG) * sNumCounters),
                     &usNumBytes, FALSE);
      sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );
     } /* endif */

     if ( sRc == BTREE_IN_USE )
     {
  //   UtlWait( MAX_WAIT_TIME );
       sRetries--;
     } /* endif */

  // if ( sRetries != MAX_RETRY_COUNT )
  // {
  //   DEBUGEVENT2( QDAMGETUPDCOUNTER_LOC, WAIT_EVENT, (MAX_RETRY_COUNT - sRetries), DB_GROUP, NULL );
  // } /* endif */
  } while ( (sRc == BTREE_IN_USE) && (sRetries > 0) );

  if ( sRc )
  {
    ERREVENT2( QDAMGETUPDCOUNTER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  return sRc;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMUpdateLockRec      Update lock record of database
//------------------------------------------------------------------------------
// Function call:     QDAMUpdateLockRec( PBTREE, PSZ, BOOL );
//------------------------------------------------------------------------------
// Description:       Add a term to or remove a term from the record containing
//                    the list of locked records.
//------------------------------------------------------------------------------
// Parameters:        PBTREE     pBTIda     pointer to btree structure
//                    PSZ        pszTerm    term being added or removed
//                    BOOL       fAdd       TRUE = Add term to lock record
//                                          FALSE = remove term from lock rec.
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    other             return code os used QDAM functions
//                    BTREE_WRITE_ERROR write error to disk
//------------------------------------------------------------------------------
// Function flow:     physically lock database
//                    read lock record from database
//                    search lock term in record
//                    add or remove term to/from record
//                    rewrite lock record
//                    unlock database
//------------------------------------------------------------------------------
SHORT QDAMUpdateLockRec
(
   PBTREE     pBTIda,                  // pointer to btree structure
   PSZ_W      pszTermW,                 // term being added or removed
   BOOL       fAdd                     // TRUE = Add term to lock record
                                       // FALSE = remove term from lock record
)
{
  SHORT       sRc = NO_ERROR;          // function return code
  USHORT      usRecLen = 0;            // length of record
  PBTREEGLOB  pBT = pBTIda->pBTree;
  BOOL        fLocked = FALSE;         // database-has-been-locked flag
  PSZ         pszCurTerm = NULL;       // ptr for locked terms list processing
  FILELOCK    Lock;                    // range to be locked or unlocked
  USHORT      usNumBytes;              // number of bytes written or read
  ULONG       ulNewOffset;             // new offset in file
  SHORT       sRetries;                // number of retries
  PSZ         pszTerm;
  CHAR        chLockTerm[HEADTERM_SIZE];

  /********************************************************************/
  /* physically lock dummy file (retry in case of failures)            */
  /********************************************************************/
  Lock.lOffset = 0L;                   // start at begin of file
  Lock.lRange  = (LONG)MAX_LOCKREC_SIZE;
  sRetries = MAX_RETRY_COUNT;

  Unicode2ASCIIBuf(pszTermW, chLockTerm, (USHORT)UTF16strlenCHAR(pszTermW)+1, sizeof(chLockTerm), 0L );
  pszTerm = &chLockTerm[0];

  do
  {
    sRc = UtlFileLocks( pBT->fpDummy, (PFILELOCK)NULL, &Lock, FALSE );
    sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

    if ( sRc == BTREE_IN_USE )
    {
      UtlWait( MAX_WAIT_TIME );
      sRetries--;
    } /* endif */
  } while ( (sRc == BTREE_IN_USE) && (sRetries > 0) );

  if ( !sRc )
  {
    fLocked = TRUE;
  } /* endif */

  /********************************************************************/
  /* read lock record from dummyfile                                  */
  /********************************************************************/
  if ( sRc == NO_ERROR )
  {
   // Position to begin of locked terms list
   sRc = UtlChgFilePtr( pBT->fpDummy, (LONG)(sizeof(LONG)*MAX_UPD_CTR),
                        FILE_BEGIN, &ulNewOffset, FALSE);
   if ( sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

   // Read lock record
   if ( !sRc )
   {
    memset( szLockRec, 0, sizeof(szLockRec) );
    sRc = UtlRead( pBT->fpDummy, (PVOID)szLockRec, sizeof(szLockRec),
                   &usNumBytes, FALSE);
    if ( sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

    if ( usNumBytes == 0)
    {
      // no locked terms list yet, create an empty one
      memset( szLockRec, 0, sizeof(szLockRec) );
      usNumBytes = sizeof(LONG);
    } /* endif */
    usRecLen = usNumBytes;
   } /* endif */
  } /* endif */

  /********************************************************************/
  /* Check for and handle corrupted lock records (maybe the current   */
  /* record has been corrupted by the end-delimiter-missing bug)      */
  /********************************************************************/
  if ( sRc == NO_ERROR )
  {
    PSZ pszSource = szLockRec;          // start search at begin of locked terms
    PSZ pszEnd    = szLockRec + sizeof(szLockRec);

    /******************************************************************/
    /* Loop over all terms in list                                    */
    /******************************************************************/
    while ( (pszSource < pszEnd) && (*((PLONG)pszSource) != 0L) )
    {
      USHORT usEntryLen = (USHORT)(sizeof(LONG) + strlen( pszSource + 4 ) + 1);

      pszSource += usEntryLen;
    } /* endwhile */

    /******************************************************************/
    /* Check if the loop ended due to an out-of-range condition       */
    /******************************************************************/
    if ( pszSource >= pszEnd )
    {
      /****************************************************************/
      /* The lock record is corrupted so get rid off it               */
      /****************************************************************/
      memset( szLockRec, 0, sizeof(szLockRec) );
      usNumBytes = sizeof(LONG);
      usRecLen = usNumBytes;
    } /* endif */
  } /* endif */


  /********************************************************************/
  /* compact locked term list by removing of old entries              */
  /********************************************************************/
  if ( sRc == NO_ERROR )
  {
    PSZ pszSource = szLockRec;          // start search at begin of locked terms
    PSZ pszTarget = szLockRec;
    LONG lTime;

    /******************************************************************/
    /* Get current time - 1 day                                       */
    /******************************************************************/
    time( &lTime );
    lTime -= 86400L;                   // subtract one day (in seconds: 60 x 60 x 24)

    /******************************************************************/
    /* Copy newer terms, discard the old ones                         */
    /******************************************************************/
    while ( *((PLONG)pszSource) != 0L )
    {
      if ( *((PLONG)pszSource) >= lTime )
      {
        // term is not out-of-time, so copy it to the target area
        SHORT i;

        // just
        // copy time stamp
        for ( i = 0; i < sizeof(LONG); i++ )
        {
          *pszTarget++ = *pszSource++;
        } /* endfor */

        // copy term
        while (*pszSource != EOS)
        {
          *pszTarget++ = *pszSource++;
        } /* endwhile */
        *pszTarget++ = *pszSource++;   // copy end-of-string delimiter
      }
      else
      {
        // ignore term which is out-of-time
        pszSource += sizeof(LONG) + strlen(pszSource+sizeof(LONG)) + 1;
      } /* endif */
    } /* endwhile */
    *((PLONG)pszTarget) = 0L;          // terminate term list
    usRecLen = (USHORT)((pszTarget - szLockRec) + sizeof(LONG)); // adjust list size
  } /* endif */

  /********************************************************************/
  /* search lock term in record                                       */
  /********************************************************************/
  if ( sRc == NO_ERROR )
  {
    pszCurTerm = szLockRec;          // start search at begin of locked terms

    /******************************************************************/
    /* Look for term in locked terms list                             */
    /******************************************************************/
    while ( *((PLONG)pszCurTerm) != 0L )
    {
      if ( strcmp( pszCurTerm + sizeof(LONG), pszTerm) == 0 )
      {
        break;                         // term found in list, exit loop
      }
      else
      {
        pszCurTerm += sizeof(LONG) + strlen(pszCurTerm + sizeof(LONG)) + 1; // continue with next entry in list
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* Handle results of lookup                                       */
    /******************************************************************/
    if ( fAdd && (*((PLONG)pszCurTerm) != 0L) )
    {
      /****************************************************************/
      /* Term is already contained in lock list, set approbriate      */
      /* return code                                                  */
      /****************************************************************/
      sRc = BTREE_ENTRY_LOCKED;
    }
    else if ( !fAdd && (*((PLONG)pszCurTerm) == 0L) )
    {
      /****************************************************************/
      /* Term is not in lock list, set approbriate return code        */
      /****************************************************************/
      sRc = BTREE_LOCK_ERROR;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* add or remove term to/from record                                */
  /********************************************************************/
  if ( sRc == NO_ERROR )
  {
    if ( fAdd )
    {
      /****************************************************************/
      /* Add the term to the list of locked terms                     */
      /****************************************************************/
      USHORT usTermLength = (USHORT)strlen(pszTerm);
      USHORT usEntryLength = usTermLength + 1 + sizeof(LONG);
      if ( (usRecLen + usEntryLength + sizeof(LONG)) < sizeof(szLockRec) )
      {
        time( (PLONG)pszCurTerm );       // add term timestamp
        pszCurTerm += sizeof(LONG);
        strcpy( pszCurTerm, pszTerm );   // add term to locked term list
        pszCurTerm += usTermLength + 1;
        *((PLONG)pszCurTerm) = 0L;       // add a new end-of-list indicator
        usRecLen += usEntryLength + 1;   // adjust record length
      }
      else
      {
        sRc = BTREE_LOCK_ERROR;          // buffer limit exceeded
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* Remove term from term list                                   */
      /* pszCurTerm points to term in term list                       */
      /****************************************************************/
      PSZ pszEndOfList = pszCurTerm;
      USHORT usTermLength = (USHORT)strlen(pszTerm);

      /****************************************************************/
      /* Find end of term list                                        */
      /****************************************************************/
      while ( *((PLONG)pszEndOfList) != 0L )
      {
        pszEndOfList += sizeof(LONG) + strlen(pszEndOfList+sizeof(LONG)) + 1;
      } /* endwhile */

      /****************************************************************/
      /* Remove term from list by shifting the remaining part of the  */
      /* term list                                                    */
      /****************************************************************/
      memmove( pszCurTerm, pszCurTerm + (sizeof(LONG) + usTermLength + 1),
               (pszEndOfList - pszCurTerm) - (usTermLength + sizeof(LONG) + 1) + sizeof(LONG) );
      usRecLen -= sizeof(LONG) + usTermLength + 1;    // adjust record length
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* rewrite lock record                                              */
  /********************************************************************/
  if ( sRc == NO_ERROR )
  {
    // Position to begin of locked terms list
    sRc = UtlChgFilePtr( pBT->fpDummy, (LONG)(sizeof(LONG)*MAX_UPD_CTR),
                        FILE_BEGIN, &ulNewOffset, FALSE);
    if ( sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

    //  Rewrite update counter
    if ( !sRc )
    {
      sRc = UtlWrite( pBT->fpDummy, (PVOID)szLockRec,
                      usRecLen, &usNumBytes, FALSE );
      if ( sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );
    } /* endif */
  } /* endif */

     if ( sRc )
  {
    ERREVENT2( QDAMUPDATELOCKREC_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  /********************************************************************/
  /* unlock dummy file                                                */
  /********************************************************************/
  if ( fLocked )
  {
    UtlFileLocks( pBT->fpDummy, &Lock, (PFILELOCK)NULL, FALSE );
  } /* endif */

  return( sRc );
} /* end of function QDAMUpdateLockRec */


// Convert Dos... return codes to BTREE return codes
SHORT QDAMDosRC2BtreeRC
(
  SHORT sDosRC,                        // Dos return code
  SHORT sDefaultRC,                    // RC for default case
  USHORT usOpenFlags                   // open flags of database
)
{
   SHORT sRc;                           // converted return code

  switch ( sDosRC )
  {
     case  NO_ERROR:
       sRc = NO_ERROR;
      break;
    case  ERROR_INVALID_DRIVE:
      sRc = BTREE_INVALID_DRIVE;
      break;
    case  ERROR_OPEN_FAILED :
      sRc = BTREE_OPEN_FAILED;
      break;
    case  ERROR_NETWORK_ACCESS_DENIED:
    case  ERROR_VC_DISCONNECTED:
      sRc = BTREE_NETWORK_ACCESS_DENIED;
      break;
    case ERROR_ACCESS_DENIED:
      if ( usOpenFlags & ASD_SHARED )
      {
        // map ACCESS_DENIED to IN_USE for shared databases as
        // a locked local file returns ERROR_ACCESS_DENIED rather than
        // ERROR_SHARING_VIOLATION
        sRc = BTREE_IN_USE;
      }
      else
      {
        sRc = BTREE_ACCESS_ERROR;
      } /* endif */
      break;
    case ERROR_DRIVE_LOCKED:
    case ERROR_INVALID_ACCESS:
      sRc = BTREE_ACCESS_ERROR;
      break;
    case ERROR_FILE_NOT_FOUND:
      sRc = BTREE_FILE_NOTFOUND;
      break;
    case ERROR_LOCK_VIOLATION:
    case ERROR_SHARING_VIOLATION:
      sRc = BTREE_IN_USE;
      break;
    default :
      sRc = sDefaultRC;
      break;
  } /* endswitch */

   return( sRc );
} /* end of function QDAMDosRC2BtreeRC  */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMMatchCompound       Check if key matches compound
//------------------------------------------------------------------------------
// Description:       Check if the given key contains one of the compounds
//                    of the compound list.
//------------------------------------------------------------------------------
// Parameters:        PUCHAR pKey            key to check
//                    PUCHAR pCompound       compound(s) to look for
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE              one of the compounds is contained in
//                                      key
//                    FALSE             no match found
//------------------------------------------------------------------------------
BOOL QDAMMatchCompound
(
  PSZ_W  pKey,                         // key to check
  PSZ_W  pCompound                     // compound(s) to look for
)
{
  BOOL        fMatch = FALSE;          // function return code

  // look for first character of compound
  PSZ_W  pszTemp = pKey;
  USHORT usCompareLen  = (USHORT)UTF16strlenBYTE(pCompound);
  USHORT usCompoundLen = (USHORT)UTF16strlenCHAR(pCompound);
  USHORT usTermLen     = (USHORT)UTF16strlenCHAR(pKey);

  while ( !fMatch && (*pszTemp != EOS) )
  {
    if ( *pszTemp == *pCompound )
    {
      // check for rest of compound
      if ( (usTermLen >= usCompoundLen) &&
           (memicmp( (PBYTE)pszTemp, (PBYTE)pCompound, usCompareLen ) == 0 ) )
      {
        // got a match!
        fMatch = TRUE;
      } /* endif */
    } /* endif */
    pszTemp++;
    usTermLen--;
  } /* endwhile */

  return( fMatch );
} /* end of function QDAMMatchCompound */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMComputeCheckSum     Compute CheckSum for a record
//------------------------------------------------------------------------------
// Description:       Computes the CheckSum for a in-memory record
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER pRecord   ptr to record
//------------------------------------------------------------------------------
// Returncode type:   ULONG             the computed CheckSum
//------------------------------------------------------------------------------
ULONG QDAMComputeCheckSum_V3
(
  PBTREEBUFFER_V3     pRecord             // ptr to record
)
{
  pRecord;
  return 0;
} /* end of function QDAMComputeCheckSum */

ULONG QDAMComputeCheckSum_V2
(
  PBTREEBUFFER_V2     pRecord             // ptr to record
)
{
  pRecord;
  return 0;
} /* end of function QDAMComputeCheckSum */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMCheckCheckSum        Checks the CheckSum for a record
//------------------------------------------------------------------------------
// Description:       Check if the computed CheckSum for a record matches the
//                    CheckSum stored in the record.
//                    In _DEBUG mode a message box will popup if the CheckSum
//                    does not match.
//                    In anycase a STATE_EVENT is written to the event log
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER pRecord   ptr to record
//                    SHORT        sLocation ID for current function (_LOC id)
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE              the CheckSum matches the record
//                    FALSE             invalid CheckSum
//------------------------------------------------------------------------------
BOOL QDAMCheckCheckSum_V2
(
  PBTREEBUFFER_V2  pRecord,            // ptr to record
  SHORT            sLocation           // ID for current function (_LOC id)
)
{
  BOOL fOK = TRUE;                     // function return code
  ULONG ulCheckSum = 0L;               // buffer for CheckSum

  ulCheckSum = QDAMComputeCheckSum_V2( pRecord );

  if ( ulCheckSum != pRecord->ulCheckSum )
  {
    ERREVENT2( sLocation, CHECKSUM_EVENT, pRecord->usRecordNumber, DB_GROUP, NULL );
  } /* endif */

  return( fOK );
} /* end of function QDAMCheckCheckSum */

BOOL QDAMCheckCheckSum_V3
(
  PBTREEBUFFER_V3  pRecord,            // ptr to record
  SHORT            sLocation           // ID for current function (_LOC id)
)
{
  BOOL fOK = TRUE;                     // function return code
  ULONG ulCheckSum = 0L;               // buffer for CheckSum

  ulCheckSum = QDAMComputeCheckSum_V3( pRecord );

  if ( ulCheckSum != pRecord->ulCheckSum )
  {
    ERREVENT2( sLocation, CHECKSUM_EVENT, pRecord->usRecordNumber, DB_GROUP, NULL );
  } /* endif */

  return( fOK );
} /* end of function QDAMCheckCheckSum */
