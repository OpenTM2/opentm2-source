/*! \file
	Description: Secondary routines, mainly for write...

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define DATATHRESHOLD  128        // compress if data longer number of bytes

#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_TM               // general Transl. Memory functions
#include <eqf.h>                  // General Translation Manager include file

#include <eqfqdami.h>             // Private QDAM defines
#include <eqfcmpr.h>              // defines for compression
#include <time.h>

#include "eqfevent.h"                  // event logging facility

#define INC_READ_COUNT
#define INC_CACHED_COUNT
#define INC_CACHED_READ_COUNT
#define INC_WRITE_COUNT
#define INC_REAL_READ_COUNT
#define INC_REAL_WRITE_COUNT

/**********************************************************************/
/* default collating sequence as returned by DosGetCollating for a    */
/* ASCII 850 code page..                                              */
/**********************************************************************/
UCHAR chDefCollate[] = {
     0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
    32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
    48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
    64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
    96,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 123, 124, 125, 126, 127,
    67,  85,  69,  65,  65,  65,  65,  67,  69,  69,  69,  73,  73,  73,  65,  65,
    69,  65,  65,  79,  79,  79,  85,  85,  89,  79,  85,  79,  36,  79, 158,  36,
    65,  73,  79,  85,  78,  78, 166, 167,  63, 169, 170, 171, 172,  33,  34,  34,
   176, 177, 178, 179, 180,  65,  65,  65, 184, 185, 186, 187, 188,  36,  36, 191,
   192, 193, 194, 195, 196, 197,  65,  65, 200, 201, 202, 203, 204, 205, 206,  36,
    68,  68,  69,  69,  69,  73,  73,  73,  73, 217, 218, 219, 220, 221,  73, 223,
    79,  83,  79,  79,  79,  79, 230, 232, 232,  85,  85,  85,  89,  89, 238, 239,
   240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 };
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMWriteRecord  Write record
//------------------------------------------------------------------------------
// Function call:     QDAMWriteRecord( PBTREE, PBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       Write the requested record either in cash ( mark it )
//                    or to disk
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The B-tree to write
//                    PBTREERECORD       The buffer to write
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//------------------------------------------------------------------------------
// Function flow:     init return code
//                    write header with open flag set if not yet done
//                    if guard flag is on then
//                      write it to disk and set sRc
//                    else
//                      set only flag
//                    endif
//                    return sRc
//------------------------------------------------------------------------------
SHORT QDAMWriteRecord_V2
(
   PBTREE       pBTIda,
   PBTREEBUFFER_V2 pBuffer
)
{
   SHORT  sRc = 0;                               // return code
   PBTREEGLOB  pBT = pBTIda->pBTree;

   DEBUGEVENT2( QDAMWRITERECORD_LOC, FUNCENTRY_EVENT, RECORDNUM(pBuffer), DB_GROUP, NULL );

   INC_WRITE_COUNT;

   /********************************************************************/
   /* write header with corruption flag -- if not yet done             */
   /********************************************************************/
   if ( pBT->fWriteHeaderPending )
   {
     sRc = QDAMWriteHeader( pBTIda );
     pBT->fWriteHeaderPending = FALSE;
   } /* endif */

   //  if guard flag is on write it to disk
   //  else set only the flag
   if ( !sRc )
   {
     if ( !pBT->fGuard )
     {
        pBuffer->fNeedToWrite = TRUE;
     }
     else
     {
        sRc = QDAMWRecordToDisk_V2(pBTIda, pBuffer);
     } /* endif */
   } /* endif */


   // For shared databases only: set internal update flag, the
   // update counter will be written once the file lock is removed
   if ( !sRc && (pBT->usOpenFlags & ASD_SHARED) )
   {
     pBT->fUpdated = TRUE;;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMWRITERECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */
   DEBUGEVENT2( QDAMWRITERECORD_LOC, FUNCEXIT_EVENT, sRc, DB_GROUP, NULL );
   return sRc;
}

SHORT QDAMWriteRecord_V3
(
   PBTREE       pBTIda,
   PBTREEBUFFER_V3 pBuffer
)
{
   SHORT  sRc = 0;                               // return code
   PBTREEGLOB  pBT = pBTIda->pBTree;

   DEBUGEVENT2( QDAMWRITERECORD_LOC, FUNCENTRY_EVENT, RECORDNUM(pBuffer), DB_GROUP, NULL );

   INC_WRITE_COUNT;

   /********************************************************************/
   /* write header with corruption flag -- if not yet done             */
   /********************************************************************/
   if ( pBT->fWriteHeaderPending )
   {
     sRc = QDAMWriteHeader( pBTIda );
     pBT->fWriteHeaderPending = FALSE;
   } /* endif */

   //  if guard flag is on write it to disk
   //  else set only the flag
   if ( !sRc )
   {
     if ( !pBT->fGuard )
     {
        pBuffer->fNeedToWrite = TRUE;
     }
     else
     {
        sRc = QDAMWRecordToDisk_V3(pBTIda, pBuffer);
     } /* endif */
   } /* endif */


   // For shared databases only: set internal update flag, the
   // update counter will be written once the file lock is removed
   if ( !sRc && (pBT->usOpenFlags & ASD_SHARED) )
   {
     pBT->fUpdated = TRUE;;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMWRITERECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */
   DEBUGEVENT2( QDAMWRITERECORD_LOC, FUNCEXIT_EVENT, sRc, DB_GROUP, NULL );
   return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMWRecordToDisk    Write record to disk
//------------------------------------------------------------------------------
// Function call:     QDAMWRecordToDisk( PBTREE, PBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       Write the requested record to disk
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The B-tree to flush
//                    PBTREERECORD       The buffer to write
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//------------------------------------------------------------------------------
// Function flow:     init return value (sRc )
//                    position in file to record to be written
//                    if okay so far then
//                      write record to disk and set sRC
//                      if okay then
//                        check if disk was full, i.e.
//                        if written bytes equal BTREE_REC_SIZE then
//                          reset fNeedToWrite flag;
//                          init buffer
//                        else
//                          set return code BTREE_DISK_FULL;
//                          set fCorrupted flag
//                        endif
//                      else
//                        set BTREE_WRITE_ERROR;
//                        set fCorrupted flag
//                      endif
//                    else
//                      set BTREE_WRITE_ERROR;
//                      set fCorrupted flag
//                    endif
//                    return sRc;
//------------------------------------------------------------------------------
SHORT QDAMWRecordToDisk_V2
(
   PBTREE     pBTIda,                     // pointer to btree structure
   PBTREEBUFFER_V2  pBuffer               // pointer to buffer to write
)
{
  SHORT sRc = 0;                          // return code
  LONG  lOffset;                          // offset in file
  ULONG ulNewOffset;                      // new file pointer position
  USHORT usBytesWritten;                  // number of bytes written
  PBTREEGLOB  pBT = pBTIda->pBTree;

  INC_REAL_WRITE_COUNT;

  DEBUGEVENT2( QDAMRECORDTODISK_LOC, FUNCENTRY_EVENT, RECORDNUM(pBuffer), DB_GROUP, NULL );

//QDAMCheckCheckSum( pBuffer, QDAMRECORDTODISK_LOC );

  if ( !sRc )
  {
    lOffset = RECORDNUM(pBuffer) * (long)BTREE_REC_SIZE_V2;

    sRc = UtlChgFilePtr( pBT->fp, lOffset, FILE_BEGIN, &ulNewOffset, FALSE);
  } /* endif */

  if ( ! sRc )
  {
    sRc = UtlWrite( pBT->fp, (PVOID) &pBuffer->contents, BTREE_REC_SIZE_V2, &usBytesWritten, FALSE );

    // check if disk is full
    if ( ! sRc )
    {
       if ( usBytesWritten == BTREE_REC_SIZE_V2 )
       {
          pBuffer->fNeedToWrite = FALSE;
       }
       else
       {
          sRc = BTREE_DISK_FULL;
          pBT->fCorrupted = TRUE;                     // indicate corruption
       } /* endif */
    }
    else
    {
       sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
       pBT->fCorrupted = TRUE;                     // indicate corruption
    } /* endif */
  }
  else
  {
    sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
    pBT->fCorrupted = TRUE;                        // indicate corruption
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMRECORDTODISK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  return sRc;
}

SHORT QDAMWRecordToDisk_V3
(
   PBTREE     pBTIda,                     // pointer to btree structure
   PBTREEBUFFER_V3  pBuffer               // pointer to buffer to write
)
{
  SHORT sRc = 0;                          // return code
  LONG  lOffset;                          // offset in file
  ULONG ulNewOffset;                      // new file pointer position
  USHORT usBytesWritten;                  // number of bytes written
  PBTREEGLOB  pBT = pBTIda->pBTree;

  INC_REAL_WRITE_COUNT;

  DEBUGEVENT2( QDAMRECORDTODISK_LOC, FUNCENTRY_EVENT, RECORDNUM(pBuffer), DB_GROUP, NULL );

//QDAMCheckCheckSum( pBuffer, QDAMRECORDTODISK_LOC );

  if ( !sRc )
  {
    lOffset = RECORDNUM(pBuffer) * (long)BTREE_REC_SIZE_V3;

    sRc = UtlChgFilePtr( pBT->fp, lOffset, FILE_BEGIN, &ulNewOffset, FALSE);
  } /* endif */

  if ( ! sRc )
  {
    sRc = UtlWrite( pBT->fp, (PVOID) &pBuffer->contents, BTREE_REC_SIZE_V3, &usBytesWritten, FALSE );

    // check if disk is full
    if ( ! sRc )
    {
       if ( usBytesWritten == BTREE_REC_SIZE_V3 )
       {
          pBuffer->fNeedToWrite = FALSE;
       }
       else
       {
          sRc = BTREE_DISK_FULL;
          pBT->fCorrupted = TRUE;                     // indicate corruption
       } /* endif */
    }
    else
    {
       sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
       pBT->fCorrupted = TRUE;                     // indicate corruption
    } /* endif */
  }
  else
  {
    sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
    pBT->fCorrupted = TRUE;                        // indicate corruption
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMRECORDTODISK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  return sRc;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMNewRecord   Add a new record to the index
//------------------------------------------------------------------------------
// Function call:     QDAMNewRecord( PBTREE, PPBTREERECORD, RECTYPE );
//
//------------------------------------------------------------------------------
// Description:         Get a new record either from the free list
//                      or create a new
//
//------------------------------------------------------------------------------
// Parameters:          PBTREE             B-tree
//                      PBTREERECORD *     the buffer returned
//                      RECTYPE            data or key record
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
//
//------------------------------------------------------------------------------
// Function flow:     if record is a datarecord
//                      if there is a free data buffer
//                        call QDAMReadRecord
//                        increase ptr to next free data buffer
//                      else
//                        read next free record and update ptr
//                      endif
//                    else
//                      if no free key buffer
//                        call QDAMAllocKeyRecords
//                      endif
//                      if ok
//                        read the free key buffer (QDAMReadRecord)
//                        update ptr to next free key buffer
//                      endif
//                    endif
//                    reset info in *ppRecord
//------------------------------------------------------------------------------
SHORT QDAMNewRecord_V2
(
   PBTREE          pBTIda,
   PBTREEBUFFER_V2  * ppRecord,
   RECTYPE         recType          // data or key record
)
{
  SHORT   sRc = 0;                         // return code
  PBTREEGLOB  pBT = pBTIda->pBTree;

  //
  // Try and use a record within the file if there are any.  This saves
  // on disk usage
  //
  *ppRecord = NULL;                       // in case of error
  if ( recType == DATAREC )
  {
     if ( pBT->usFreeDataBuffer )
     {
       sRc = QDAMReadRecord_V2( pBTIda, pBT->usFreeDataBuffer, ppRecord, FALSE );
       if ( ! sRc )
       {
         pBT->usFreeDataBuffer = NEXT( *ppRecord );
       } /* endif */
     }
     else
     {
       if ( pBT->usNextFreeRecord == 0xFFFF ) // end reached ???
       {
         sRc = BTREE_LOOKUPTABLE_TOO_SMALL;
       }
       else
       {
         pBT->usNextFreeRecord++;
         sRc = QDAMReadRecord_V2( pBTIda, pBT->usNextFreeRecord, ppRecord, TRUE );
       }
     }
  }
  else
  {
     if ( !pBT->usFreeKeyBuffer)
     {
       sRc = QDAMAllocKeyRecords( pBTIda, 5 );
     } /* endif */

     if ( !sRc )
     {
       sRc = QDAMReadRecord_V2( pBTIda, pBT->usFreeKeyBuffer, ppRecord, FALSE  );
       if ( ! sRc )
       {
         pBT->usFreeKeyBuffer = NEXT( *ppRecord );
       } /* endif */
     } /* endif */
  } /* endif */

  if ( *ppRecord )
  {
     NEXT(*ppRecord) = 0;                          // reset information
     PREV(*ppRecord) = 0;                          // reset information
  } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMNEWRECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return ( sRc );
}

SHORT QDAMNewRecord_V3
(
   PBTREE          pBTIda,
   PBTREEBUFFER_V3  * ppRecord,
   RECTYPE         recType          // data or key record
)
{
  SHORT   sRc = 0;                         // return code
  PBTREEGLOB  pBT = pBTIda->pBTree;

  //
  // Try and use a record within the file if there are any.  This saves
  // on disk usage
  //
  *ppRecord = NULL;                       // in case of error
  if ( recType == DATAREC )
  {
     if ( pBT->usFreeDataBuffer )
     {
       sRc = QDAMReadRecord_V3( pBTIda, pBT->usFreeDataBuffer, ppRecord, FALSE );
       if ( ! sRc )
       {
         pBT->usFreeDataBuffer = NEXT( *ppRecord );
       } /* endif */
     }
     else
     {
       if ( pBT->usNextFreeRecord == 0xFFFF ) // end reached ???
       {
         sRc = BTREE_LOOKUPTABLE_TOO_SMALL;
       }
       else
       {
         pBT->usNextFreeRecord++;
         sRc = QDAMReadRecord_V3( pBTIda, pBT->usNextFreeRecord, ppRecord, TRUE );
       }
     }
  }
  else
  {
     if ( !pBT->usFreeKeyBuffer)
     {
       sRc = QDAMAllocKeyRecords( pBTIda, 5 );
     } /* endif */

     if ( !sRc )
     {
       sRc = QDAMReadRecord_V3( pBTIda, pBT->usFreeKeyBuffer, ppRecord, FALSE  );
       if ( ! sRc )
       {
         pBT->usFreeKeyBuffer = NEXT( *ppRecord );
       } /* endif */
     } /* endif */
  } /* endif */

  if ( *ppRecord )
  {
     NEXT(*ppRecord) = 0;                          // reset information
     PREV(*ppRecord) = 0;                          // reset information
  } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMNEWRECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return ( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFreeRecord     Add record to free list
//------------------------------------------------------------------------------
// Function call:     QDAMFreeRecord( PBTREE, PBTREERECORD, RECTYPE );
//
//------------------------------------------------------------------------------
// Description:       add the requested record to the free list
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             B-tree
//                    PBTREERECORD       record buffer
//                    RECTYPE            data or key record
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_INVALID     invalid pointer passed
//------------------------------------------------------------------------------
// Function flow:     if pRecord is valid
//                      reinit contents ot pRecord
//                      if it is a data record
//                        add deleted record to the free list of data recs
//                        take it from the intermal data list if avail.
//                      else
//                        add deleted record to the free list of key recs
//                      endif
//                      init the header of the free record
//                      write the record (QDAMWriteRecord)
//                    else
//                      set return code = BTREE_INVALID
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMFreeRecord_V3
(
   PBTREE       pBTIda,
   PBTREEBUFFER_V3 pRecord,
   RECTYPE      recType                // data or key record
)
{
  SHORT  sRc;                          // return code
  PBTREEHEADER  pHeader;               // pointer to header
  USHORT        usRecNum;              // record number
  PBTREEGLOB  pBT = pBTIda->pBTree;

  if ( pRecord )
  {

     memset( pRecord->contents.uchData, 0, sizeof(pRecord->contents.uchData) );
     if ( recType == DATAREC)           // it is a data record
     {
       // add deleted record to the free list
       NEXT( pRecord ) = pBT->usFreeDataBuffer;
       PREV( pRecord ) = 0;
       pBT->usFreeDataBuffer = RECORDNUM( pRecord );
       usRecNum = pBT->usFreeDataBuffer;
       // get it from the internal data list if available
       QDAMFreeFromList_V3(pBT->DataRecList ,pRecord);
     }
     else
     {
       // add deleted record to the free list
       NEXT( pRecord ) = pBT->usFreeKeyBuffer;
       PREV( pRecord ) = 0;
       pBT->usFreeKeyBuffer = RECORDNUM( pRecord );
       usRecNum = pBT->usFreeKeyBuffer;

     } /* endif */
     // init this record
     pHeader = &pRecord->contents.header;
     pHeader->usNum = usRecNum;
     pHeader->usOccupied = 0;
     pHeader->usWasteSize = 0;
     pHeader->usFilled = sizeof(BTREEHEADER );
     pHeader->usLastFilled = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER );
//   pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
     sRc = QDAMWriteRecord_V3( pBTIda, pRecord);
  }
  else
  {
    sRc = BTREE_INVALID;
  }  /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMFREERECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}
SHORT QDAMFreeRecord_V2
(
   PBTREE       pBTIda,
   PBTREEBUFFER_V2 pRecord,
   RECTYPE      recType                // data or key record
)
{
  SHORT  sRc;                          // return code
  PBTREEHEADER  pHeader;               // pointer to header
  USHORT        usRecNum;              // record number
  PBTREEGLOB  pBT = pBTIda->pBTree;

  if ( pRecord )
  {

     memset( pRecord->contents.uchData, 0, sizeof(pRecord->contents.uchData) );
     if ( recType == DATAREC)           // it is a data record
     {
       // add deleted record to the free list
       NEXT( pRecord ) = pBT->usFreeDataBuffer;
       PREV( pRecord ) = 0;
       pBT->usFreeDataBuffer = RECORDNUM( pRecord );
       usRecNum = pBT->usFreeDataBuffer;
       // get it from the internal data list if available
       QDAMFreeFromList_V2(pBT->DataRecList ,pRecord);
     }
     else
     {
       // add deleted record to the free list
       NEXT( pRecord ) = pBT->usFreeKeyBuffer;
       PREV( pRecord ) = 0;
       pBT->usFreeKeyBuffer = RECORDNUM( pRecord );
       usRecNum = pBT->usFreeKeyBuffer;

     } /* endif */
     // init this record
     pHeader = &pRecord->contents.header;
     pHeader->usNum = usRecNum;
     pHeader->usOccupied = 0;
     pHeader->usWasteSize = 0;
     pHeader->usFilled = sizeof(BTREEHEADER );
     pHeader->usLastFilled = BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER );
//   pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
     sRc = QDAMWriteRecord_V2( pBTIda, pRecord);
  }
  else
  {
    sRc = BTREE_INVALID;
  }  /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMFREERECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictUpdSignLocal Write User Data
//------------------------------------------------------------------------------
// Function call:     QDAMDictUpdSignLocal( PBTREE, PCHAR, USHORT );
//
//------------------------------------------------------------------------------
// Description:       Writes the second part of the first record (user data)
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to user data
//                    USHORT                 length of user data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_INVALID     pointer invalid
//                    BTREE_USERDATA    user data too long
//                    BTREE_CORRUPTED   dictionary is corrupted
//------------------------------------------------------------------------------
// Function flow:     if tree is corrupted
//                      set sRC = BTREE_CORRUPTED
//                    else
//                      give 1K at beginning as space (UtlChgFilePtr)
//                    endif
//                    if ok then
//                      check if length of user data is correct
//                      if ok then
//                        write length of UserData to disk
//                        check if disk is full
//                        if ok
//                          write pUserData to the disk
//                          check if disk is full
//                        endif
//                      endif
//                      if ok
//                        fill rest up with zeros
//                        check if disk is full
//                      endif
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDictUpdSignLocal
(
   PBTREE pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   ULONG  ulLen                        // length of user data
)
{
  SHORT  sRc=0;                        // return code
  ULONG   ulDataLen;                   // number of bytes to be written
  USHORT  usBytesWritten;              // bytes written to disk
  PCHAR   pchBuffer;                   // pointer to buffer
  LONG    lFilePos;                    // file position to position at
  ULONG   ulNewOffset;                 // new offset
  PBTREEGLOB  pBT = pBTIda->pBTree;

  if ( pBT->fCorrupted )
  {
     sRc = BTREE_CORRUPTED;
  } /* endif */
  if ( !sRc && !pBT->fOpen )
  {
    sRc = BTREE_READONLY;
  }
  else
  {
     // let 2K at beginning as space
     lFilePos = (LONG) USERDATA_START;
     sRc = UtlChgFilePtr( pBT->fp, lFilePos, FILE_BEGIN,
                          &ulNewOffset, FALSE);
  } /* endif */

  if ( ! sRc )
  {
    if ( pBT->bRecSizeVersion == BTREE_V3 )
    {
      ulDataLen = min( ulLen, BTREE_REC_SIZE_V3 - USERDATA_START - sizeof(USHORT));
    }
    else
    {
      ulDataLen = min( ulLen, BTREE_REC_SIZE_V2 - USERDATA_START - sizeof(USHORT));
    } /* endif */
    if ( ulDataLen < ulLen )
    {
       sRc = BTREE_USERDATA;
    }
    else
    {
       if ( !pUserData  )
       {
         pUserData = "";
         ulDataLen = 0;
       } /* endif */

       /*************************************************************/
       /*  write length of userdata                                 */
       /*************************************************************/
       {
         USHORT usDataLen = (USHORT)ulDataLen;
         sRc = UtlWrite( pBT->fp, &usDataLen, sizeof(USHORT), &usBytesWritten, FALSE );
         ulDataLen = usDataLen;
       }

       // check if disk is full
       if ( ! sRc )
       {
          if (  usBytesWritten != sizeof(USHORT) )
          {
             sRc = BTREE_DISK_FULL;
          } /* endif */
       }
       else
       {
          sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
       } /* endif */
       if ( ! sRc)
       {
         /***********************************************************/
         /* write user data itselft                                 */
         /***********************************************************/
          sRc = UtlWrite( pBT->fp, pUserData, (USHORT)ulDataLen,
                          &usBytesWritten, FALSE );
          // check if disk is full
          if ( ! sRc )
          {
             if (  usBytesWritten != ulDataLen )
             {
                sRc = BTREE_DISK_FULL;
             } /* endif */
          }
          else
          {
             sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
          } /* endif */
       } /* endif */
    } /* endif */
    if ( ! sRc )
    {
       // fill rest up with zeros
       if ( pBT->bRecSizeVersion == BTREE_V3 )
       {
         ulDataLen = BTREE_REC_SIZE_V3 - USERDATA_START - sizeof(USHORT) - ulDataLen;
       }
       else
       {
         ulDataLen = BTREE_REC_SIZE_V2 - USERDATA_START - sizeof(USHORT) - ulDataLen;
       } /* endif */
       UtlAlloc( (PVOID *)&pchBuffer, 0L, (LONG) ulDataLen, NOMSG );
       if ( pchBuffer )
       {
          sRc = UtlWrite( pBT->fp, pchBuffer, (USHORT)ulDataLen, &usBytesWritten, FALSE );
          // check if disk is full
          if ( ! sRc )
          {
             if ( usBytesWritten != ulDataLen )
             {
                sRc = BTREE_DISK_FULL;
             } /* endif */
          }
          else
          {
             sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
          } /* endif */
          // free the buffer
          UtlAlloc( (PVOID *)&pchBuffer, 0L, 0L, NOMSG );
       }
       else
       {
          sRc = BTREE_NO_ROOM;
       } /* endif */
    } /* endif */
  } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMUPDSIGNLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictCreateLocal   Create local Dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDictCreateLocal( PSZ, PSZ, SHORT, PCHAR, USHORT,
//                                         PCHAR, PCHAR, PCHAR, PBTREE);
//------------------------------------------------------------------------------
// Description:       Establishes the basic parameters for searching a
//                    user dictionary.
//                    These parameters are stored in the first record of the
//                    index file so that subsequent accesses
//                    know what the index is like.
//
//                    If no server name is given (NULL pointer or EOS) than
//                    it is tried to open a local dictionary.
//                    If no collating sequence is given (NULL pointer) the
//                    default collating sequence is assumed
//------------------------------------------------------------------------------
// Parameters:        PSZ              name of the index file
//                    PSZ              name of the server
//                    SHORT            number of buffers used
//                    PCHAR            pointer to user data
//                    USHORT           length of user data
//                    PCHAR            pointer to term encoding sequence
//                    PCHAR            pointer to collating sequence
//                    PCHAR            pointer to case map file
//                    PBTREE *         pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_NO_ROOM     memory shortage
//                    BTREE_USERDATA    user data too long
//                    BTREE_OPEN_ERROR  dictionary already exists
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//------------------------------------------------------------------------------
// Function flow:     allocate space for BTREE
//                    if not possible
//                      set Rc = BTREE_NO_ROOM
//                    else
//                      open the index file which holds the BTREE
//                      if ok
//                        set initial structure heading
//                        allocate buffer space (for NUMBER_OF_BUFFERS many)
//                        if not possible
//                          set Rc = BTREE_NO_ROOM
//                        if not Rc
//                          call QDAMAllocTempAreas
//                        endif
//                        if not Rc
//                          write header to file
//                          if ok
//                            write 2nd part of first record
//                          endif
//                          if ok
//                            write out an empty root node
//                          endif
//                        endif
//                      else
//                        set Rc = BTREE_OPEN_ERROR
//                      endif
//                      if not Rc
//                        return pointer to BTree
//                      else
//                        destroy BTree
//                      endif
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDictCreateLocal
(
   PSZ    pName,                       // name of file
   SHORT  sNumberOfKey,                // number of key buffers
   PCHAR  pUserData,                   // user data
   USHORT usLen,                       // length of user data
   PCHAR  pTermTable,                  // term encoding sequence
   PCHAR  pCollating,                  // collating sequence
   PCHAR  pCaseMap,                    // case map string
   PBTREE * ppBTIda,                   // pointer to btree structure
   PNTMVITALINFO pNTMVitalInfo         // translation memory
)
{
  PBTREE  pBTIda = *ppBTIda;           // init return value
  SHORT i;
  SHORT sRc = 0;                       // return code
  USHORT  usAction;                    // used in open of file
  PBTREEGLOB pBT;                      // set work pointer to passed pointer
  BOOL   fTransMem = (pNTMVitalInfo != NULL);   // translation memory

  sNumberOfKey;                        // get rid of compiler warnings

  /********************************************************************/
  /* allocate global area first ...                                   */
  /********************************************************************/
  if ( ! UtlAlloc( (PVOID *)&pBT, 0L, (LONG) sizeof(BTREEGLOB ), NOMSG ) )
  {
     sRc = BTREE_NO_ROOM;
  }
  else
  {
    pBTIda->pBTree = pBT;          // anchor pointer

    // Try to create the index file
    sRc = UtlOpen( pName, &pBT->fp, &usAction, 0L, FILE_NORMAL, FILE_TRUNCATE | FILE_CREATE,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, FALSE);
  } /* endif */

  if ( !sRc )
  {
    // remember file name
    strcpy( pBTIda->szFileName, pName );

    // set initial structure heading
    UtlTime( &(pBT->lTime) );                                     // set open time
    pBTIda->sCurrentIndex = 0;
    pBT->usFirstNode=pBT->usFirstLeaf = 1;
    pBTIda->usCurrentRecord = 0;
    pBT->compare = QDAMKeyCompare;
    pBT->usNextFreeRecord = 1;
    pBT->usFreeKeyBuffer = 0;
    pBT->usFreeDataBuffer = 0;
    pBT->usFirstDataBuffer = 0;  // first data buffer
    pBT->fOpen  = TRUE;                          // open flag set
    strcpy(pBT->chFileName, pName);              // copy file name into struct
    pBT->fTransMem = fTransMem;
    pBT->fpDummy = NULLHANDLE;
    if ( pBT->fTransMem )
    {
      pBT->bVersion = BTREE_V2;
      pBT->bRecSizeVersion = BTREE_V3;
      pBT->usBtreeRecSize = BTREE_REC_SIZE_V3;
    }
    else
    {
      pBT->bVersion = BTREE_V2;
      pBT->bRecSizeVersion = BTREE_V2;
      pBT->usBtreeRecSize = BTREE_REC_SIZE_V2;
    } /* endif */

    /******************************************************************/
    /* do settings depending if we are dealing with a dict or a tm..  */
    /******************************************************************/
    if ( !fTransMem )
    {
      pBT->usVersion = BTREE_VERSION3;
      strcpy(pBT->chEQF,BTREE_HEADER_VALUE_V3);
      // use passed compression table if available else use default one
      if ( pTermTable )
      {
         memcpy( pBT->chEntryEncode, pTermTable, ENTRYENCODE_LEN );
         QDAMTerseInit( pBTIda, pBT->chEntryEncode );   // initialise for compression
         pBT->fTerse = TRUE;
      }
      else
      {
         pBT->fTerse = FALSE;
      } /* endif */
      /******************************************************************/
      /* support user defined collating sequence and case mapping       */
      /******************************************************************/
      if ( pCollating )
      {
         memcpy( pBT->chCollate, pCollating, COLLATE_SIZE );
      }
      else
      {
        memcpy( pBT->chCollate, chDefCollate, COLLATE_SIZE );
      } /* endif */

      if ( pCaseMap )
      {
         memcpy( pBT->chCaseMap, pCaseMap, COLLATE_SIZE );
      }
      else
      {
        /****************************************************************/
        /* fill in the characters and use the UtlLower function ...     */
        /****************************************************************/
        PBYTE  pTable;
        UCHAR  chTemp;

        pTable = pBT->chCaseMap;
        for ( i=0;i < COLLATE_SIZE; i++ )
        {
           *pTable++ = (CHAR) i;
        } /* endfor */
        chTemp = pBT->chCaseMap[ COLLATE_SIZE - 1];
        pBT->chCaseMap[ COLLATE_SIZE - 1] = EOS;
        pTable = pBT->chCaseMap;
        pTable++;
        UtlLower( (PSZ)pTable );
        pBT->chCaseMap[ COLLATE_SIZE - 1] = chTemp;
      } /* endif */
    }
    else
    {
      pBT->usVersion = NTM_VERSION2;
      strcpy(pBT->chEQF,BTREE_HEADER_VALUE_TM2);
      if ( pTermTable )
      {
         memcpy( pBT->chEntryEncode, pTermTable, ENTRYENCODE_LEN );
         QDAMTerseInit( pBTIda, pBT->chEntryEncode );   // initialise for compression
         pBT->fTerse = TRUE;
      }
      else
      {
         pBT->fTerse = FALSE;
      } /* endif */
      /******************************************************************/
      /* use the free collating sequence buffer to store our vital info */
      /* Its taken care, that the structure will not jeopardize the     */
      /* header...                                                      */
      /******************************************************************/
      memcpy( pBT->chCollate, pNTMVitalInfo, sizeof(NTMVITALINFO));
      /******************************************************************/
      /* new key compare routine ....                                   */
      /******************************************************************/
      pBT->compare = NTMKeyCompare;
    } /* endif */

    /* Allocate space for LookupTable */
    pBT->usNumberOfLookupEntries = 0;
    if ( pBT->bRecSizeVersion == BTREE_V3 )
    {
      UtlAlloc( (PVOID *)&pBT->LookupTable_V3, 0L, (LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(LOOKUPENTRY_V3), NOMSG );

      if ( pBT->LookupTable_V3 )
      {
        /* Allocate space for AccessCtrTable */
        UtlAlloc( (PVOID *)&pBT->AccessCtrTable, 0L, (LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(ACCESSCTRTABLEENTRY), NOMSG );
        if ( !pBT->AccessCtrTable )
        {
          UtlAlloc( (PVOID *)&pBT->LookupTable_V3, 0L, 0L, NOMSG );
          sRc = BTREE_NO_ROOM;
        }
        else
        {
          pBT->usNumberOfLookupEntries = MIN_NUMBER_OF_LOOKUP_ENTRIES;
        } /* endif */
      }
      else
      {
        sRc = BTREE_NO_ROOM;
      } /* endif */
    }
    else
    {
      UtlAlloc( (PVOID *)&pBT->LookupTable_V2, 0L, (LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(LOOKUPENTRY_V2), NOMSG );

      if ( pBT->LookupTable_V2 )
      {
        /* Allocate space for AccessCtrTable */
        UtlAlloc( (PVOID *)&pBT->AccessCtrTable, 0L, (LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(ACCESSCTRTABLEENTRY), NOMSG );
        if ( !pBT->AccessCtrTable )
        {
          UtlAlloc( (PVOID *)&pBT->LookupTable_V2, 0L, 0L, NOMSG );
          sRc = BTREE_NO_ROOM;
        }
        else
        {
          pBT->usNumberOfLookupEntries = MIN_NUMBER_OF_LOOKUP_ENTRIES;
        } /* endif */
      }
      else
      {
        sRc = BTREE_NO_ROOM;
      } /* endif */
    } /* endif */

    pBT->usNumberOfAllocatedBuffers = 0;

    if ( !sRc )
    {
      sRc =  QDAMAllocTempAreas( pBTIda );
    } /* endif */


    if (! sRc )
    {
      /****************************************************************/
      /* in order to initialize the area of the first record we       */
      /* write an empty record buffer to the file before writing the  */
      /* header                                                       */
      /****************************************************************/
      if ( pBT->bRecSizeVersion == BTREE_V3 )
      {
        USHORT usBytesWritten;
        PBTREEBUFFER_V3 pbuffer;
        UtlAlloc( (PVOID *)&pbuffer, 0L, (LONG) BTREE_BUFFER_V3 , NOMSG );
        if ( ! pbuffer )
        {
          sRc = BTREE_NO_ROOM;
        } /* endif */
        else
        {
          UtlWrite( pBT->fp, (PVOID)pbuffer, BTREE_REC_SIZE_V3, &usBytesWritten, FALSE );

          UtlAlloc( (PVOID *)&pbuffer, 0L, 0L , NOMSG );

          sRc = QDAMWriteHeader( pBTIda );
        } /* endif */
      }
      else
      {
        USHORT usBytesWritten;
        PBTREEBUFFER_V2 pbuffer;
        UtlAlloc( (PVOID *)&pbuffer, 0L, (LONG) BTREE_BUFFER_V2 , NOMSG );
        if ( ! pbuffer )
        {
          sRc = BTREE_NO_ROOM;
        } /* endif */
        else
        {
          UtlWrite( pBT->fp, (PVOID)pbuffer, BTREE_REC_SIZE_V2, &usBytesWritten, FALSE );

          UtlAlloc( (PVOID *)&pbuffer, 0L, 0L , NOMSG );

          sRc = QDAMWriteHeader( pBTIda );
        } /* endif */
      } /* endif */

      if (! sRc )
      {
        sRc = QDAMDictUpdSignLocal(pBTIda, pUserData, usLen );
      } /* endif */

      if ( !sRc )
      {
        /*******************************************************************/
        /* Write out an empty root node                                    */
        /*******************************************************************/
        if ( pBT->bRecSizeVersion == BTREE_V3 )
        {
          PBTREEBUFFER_V3 pRecord;
          sRc = QDAMReadRecord_V3(pBTIda, pBT->usFirstNode, &pRecord, TRUE );
          if ( !sRc )
          {
            TYPE(pRecord) = ROOT_NODE | LEAF_NODE | DATA_KEYNODE;
            sRc = QDAMWriteRecord_V3(pBTIda,pRecord);
          } /* endif */
          if ( !sRc )
          {
            sRc = QDAMAllocKeyRecords( pBTIda, 1 );
            if (! sRc )
            {
                pBT->usFirstDataBuffer = pBT->usNextFreeRecord;
            } /* endif */
          } /* endif */
        }
        else
        {
          PBTREEBUFFER_V2 pRecord;
          sRc = QDAMReadRecord_V2(pBTIda, pBT->usFirstNode, &pRecord, TRUE );
          if ( !sRc )
          {
            TYPE(pRecord) = ROOT_NODE | LEAF_NODE | DATA_KEYNODE;
            sRc = QDAMWriteRecord_V2(pBTIda,pRecord);
          } /* endif */
          if ( !sRc )
          {
            sRc = QDAMAllocKeyRecords( pBTIda, 1 );
            if (! sRc )
            {
                pBT->usFirstDataBuffer = pBT->usNextFreeRecord;
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  }
  else
  {
    switch ( sRc )
    {
      case  ERROR_INVALID_DRIVE:
        sRc = BTREE_INVALID_DRIVE;
        break;
      case  ERROR_OPEN_FAILED :
        sRc = BTREE_OPEN_FAILED;
        break;
      case  ERROR_NETWORK_ACCESS_DENIED:
        sRc = BTREE_NETWORK_ACCESS_DENIED;
        break;
      default :
        sRc = BTREE_OPEN_ERROR;
        break;
    } /* endswitch */
  } /* endif */

  if ( !sRc )
  {
     *ppBTIda = pBTIda;                          // return pointer to BTree
     /****************************************************************/
     /* add this dictionary to our list ...                          */
     /****************************************************************/
     sRc = QDAMAddDict( pName, pBTIda );
  }
  else
  {
    // leave the return code from create
    // file will not be destroyed if create failed, since then
    // filename is not set
    QDAMDestroy( pBTIda );
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMDICTCREATELOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */



  return( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictFlushLocal    Flush records
//------------------------------------------------------------------------------
// Function call:     QDAMDictFlushLocal( PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Writes all records necessary to disk and clears
//                    the buffers
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
//------------------------------------------------------------------------------
// Function flow:     if BTree is corrupted
//                      set RC = BTREE_CORRUPTED
//                    else
//                      while not rc and not all buffers written
//                        if buffer exists and needs to be written
//                          write buffer to disk
//                        endif
//                      endwhile
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDictFlushLocal
(
   PBTREE pBTIda
)
{
  int i = 0;
  SHORT sRc = 0;                                 // return code
  PBTREEGLOB  pBT = pBTIda->pBTree;

  DEBUGEVENT2( QDAMDICTFLUSHLOCAL_LOC, FUNCENTRY_EVENT, 0, DB_GROUP, NULL );

  if ( pBT->fCorrupted )
  {
     sRc = BTREE_CORRUPTED;
  }
  else
  {
    if ( pBT->bRecSizeVersion == BTREE_V3 )
    {
      if ( pBT->LookupTable_V3 != NULL  )
      {
        BOOL fRecordWritten = FALSE;
        PLOOKUPENTRY_V3 pLEntry = pBT->LookupTable_V3;
        for ( i=0; !sRc && (i < pBT->usNumberOfLookupEntries); i++ )
        {
          if ( pLEntry->pBuffer &&  (pLEntry->pBuffer)->fNeedToWrite )
          {
            sRc = QDAMWRecordToDisk_V3(pBTIda, pLEntry->pBuffer);
            fRecordWritten = TRUE;
          } /* endif */
          pLEntry++;
        } /* endfor */

        if ( (pBT->usOpenFlags & ASD_SHARED) && !pBTIda->fPhysLock && fRecordWritten )
        {
            // this condition should NEVER occur for shared resources
            // as write is only allowed if the database has been
            // previously locked
            ERREVENT2( QDAMDICTFLUSHLOCAL_LOC, INVOPERATION_EVENT, 1, DB_GROUP, NULL );
    #ifdef _DEBUG
            WinMessageBox( HWND_DESKTOP, (HWND)UtlQueryULong( QL_TWBCLIENT ),
              "Insecure write to dictionary/TM detected!\nLoc=QDAMDICTFLUSHLOCAL1\nPlease save info required to reproduce this condition.",
              "Gotcha!", 9999, MB_ERROR );
    #endif
        } /* endif */
      } /* endif */
    }
    else
    {
      if ( pBT->LookupTable_V2 != NULL  )
      {
        BOOL fRecordWritten = FALSE;
        PLOOKUPENTRY_V2 pLEntry = pBT->LookupTable_V2;
        for ( i=0; !sRc && (i < pBT->usNumberOfLookupEntries); i++ )
        {
          if ( pLEntry->pBuffer &&  (pLEntry->pBuffer)->fNeedToWrite )
          {
            sRc = QDAMWRecordToDisk_V2(pBTIda, pLEntry->pBuffer);
            fRecordWritten = TRUE;
          } /* endif */
          pLEntry++;
        } /* endfor */

        if ( (pBT->usOpenFlags & ASD_SHARED) && !pBTIda->fPhysLock && fRecordWritten )
        {
            // this condition should NEVER occur for shared resources
            // as write is only allowed if the database has been
            // previously locked
            ERREVENT2( QDAMDICTFLUSHLOCAL_LOC, INVOPERATION_EVENT, 1, DB_GROUP, NULL );
    #ifdef _DEBUG
            WinMessageBox( HWND_DESKTOP, (HWND)UtlQueryULong( QL_TWBCLIENT ),
              "Insecure write to dictionary/TM detected!\nLoc=QDAMDICTFLUSHLOCAL1\nPlease save info required to reproduce this condition.",
              "Gotcha!", 9999, MB_ERROR );
    #endif
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  if ( sRc )
  {
     ERREVENT2( QDAMDICTFLUSHLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */
  DEBUGEVENT2( QDAMDICTFLUSHLOCAL_LOC, FUNCEXIT_EVENT, 0, DB_GROUP, NULL );

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMReArrangeKRec    Rearrange key record
//------------------------------------------------------------------------------
// Function call:     QDAMReArrangeKRec( PBTREE, PBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       Rearranges the key record, i.e. deletes physically any
//                    logically deleted keys.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PBTREEBUFFER           pointer to record
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     get temp. record
//                    reset info in temp header
//                    for all key values
//                      if key exists
//                        copy key to temp buffer
//                      endif
//                    endfor
//                    copy record data back from temp to usual buffer
//------------------------------------------------------------------------------
VOID QDAMReArrangeKRec_V2
(
  PBTREE        pBTIda,
  PBTREEBUFFER_V2  pRecord
)
{
   PBTREEBUFFER_V2 pNew;
   PBTREEHEADER  pHeader;               // pointer to header
   PUSHORT     pusOffset;
   SHORT i;
   SHORT j;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // get temp record
   pNew = &pBT->BTreeTempBuffer_V2;
   pHeader = &pNew->contents.header;
   pHeader->usOccupied = 0;
   pHeader->usFilled = sizeof(BTREEHEADER );
   pHeader->usLastFilled = BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER );

   // for all key values move them into new record
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   i = 0; j = 0;
   while ( j <  (SHORT) OCCUPIED( pRecord ))
   {
      if ( *(pusOffset+i) )
      {
         QDAMCopyKeyTo_V2( pRecord, i, pNew, j, pBT->usVersion );
         i++;
         j++;
      }
      else
      {
         i++;                                // it is a deleted key
      } /* endif */
   } /* endwhile */
   // copy record data back to avoid misplacement in file
   memcpy( pRecord->contents.uchData, pNew->contents.uchData, FREE_SIZE_V2 );
   pRecord->contents.header.usFilled = pNew->contents.header.usFilled;
   pRecord->contents.header.usLastFilled = pNew->contents.header.usLastFilled;

// pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

   return;
}

VOID QDAMReArrangeKRec_V3
(
  PBTREE        pBTIda,
  PBTREEBUFFER_V3  pRecord
)
{
   PBTREEBUFFER_V3 pNew;
   PBTREEHEADER  pHeader;               // pointer to header
   PUSHORT     pusOffset;
   SHORT i;
   SHORT j;
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // get temp record
   pNew = &pBT->BTreeTempBuffer_V3;
   pHeader = &pNew->contents.header;
   pHeader->usOccupied = 0;
   pHeader->usFilled = sizeof(BTREEHEADER );
   pHeader->usLastFilled = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER );

   // for all key values move them into new record
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   i = 0; j = 0;
   while ( j <  (SHORT) OCCUPIED( pRecord ))
   {
      if ( *(pusOffset+i) )
      {
         QDAMCopyKeyTo_V3( pRecord, i, pNew, j, pBT->usVersion );
         i++;
         j++;
      }
      else
      {
         i++;                                // it is a deleted key
      } /* endif */
   } /* endwhile */
   // copy record data back to avoid misplacement in file
   memcpy( pRecord->contents.uchData, pNew->contents.uchData, FREE_SIZE_V3 );
   pRecord->contents.header.usFilled = pNew->contents.header.usFilled;
   pRecord->contents.header.usLastFilled = pNew->contents.header.usLastFilled;

// pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

   return;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMSplitNode    Split a Node
//------------------------------------------------------------------------------
// Function call:     QDAMSplitNode( PBTREE, PPBTREEBUFFER, PCHAR );
//
//------------------------------------------------------------------------------
// Description:       A node in the index is full, split the node.
//                    Insert the new key and value into the parent node.
//                    Return the node that the key belongs in.
//
//------------------------------------------------------------------------------
// Side effects:      Splitting a record will affect the parent node(s)
//                    of a record, too.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PBTREEBUFFER  *        pointer to active node
//                    PCHAR                  key passed
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
//------------------------------------------------------------------------------
// Function flow:     lock the active node-record
//                    if active node is root (it needs to be split)
//                      allocate space for new record
//                      if ok
//                        lock the new record
//                        create a new root
//                        unlock the new record
//                        if ok
//                          update first_leaf information
//                        endif
//                        if ok
//                          write header to disk
//                        endif
//                      endif
//                    endif
//                    if ok
//                      allocate space for new record
//                      if ok
//                        lock new record
//                        if a next rec (child) exists
//                          read the next (child)record
//                          if ok then
//                            reset the ptr to prev. record in child rec.
//                            write this record
//                          endif
//                        endif
//                        if ok
//                          adjust sibling pointers
//                          get key string of middle key in record
//                          if possible
//                            set indicator = TRUE
//                          else
//                            set rc = BTREE_CORRUPTED
//                          endif
//                          if ok
//                            set start no. where to split record
//                            while counter not at last key in record
//                              copy key to new record
//                            endwhile
//                            adjust count of keys
//                            Rearrange the key record
//                            insert pointer to new record into parent node
//                            decide where to add the new key
//                            add key and write this record
//                          endif
//                        endif
//                        unlock new record
//                      endif
//                    endif
//                    unlock temp record
//------------------------------------------------------------------------------
SHORT QDAMSplitNode_V3
(
   PBTREE pBTIda,                // pointer to generic structure
   PBTREEBUFFER_V3 *record,         // pointer to pointer to node
   PCHAR_W pKey                 // new key
)
{
  SHORT i,j;
  PBTREEBUFFER_V3 newRecord;
  PBTREEBUFFER_V3 child;
  PBTREEBUFFER_V3 parent = NULL;
  USHORT       usParent;               // number of parent
  PBTREEBUFFER_V3 pRecTemp;               // temporary buffer
  RECPARAM   recKey;                   // position/offset for key
  RECPARAM   recData;                  // position/offset for data
  PCHAR_W    pParentKey;               // new key to be inserted
  PUSHORT    pusOffset;                // pointer to offset table
  BOOL       fCompare;                 // indicator where to insert new key
  USHORT     usFreeKeys = 0;               // number of free keys required
  PBTREEGLOB    pBT = pBTIda->pBTree;

  SHORT sRc = 0;                       // success indicator
  memset( &recKey, 0, sizeof( recKey ) );
  memset( &recData,  0, sizeof( recData ) );

  pRecTemp = *record;
  BTREELOCKRECORD( pRecTemp );

  // if root needs to be split do it first
  if (IS_ROOT(*record))
  {
    sRc = QDAMNewRecord_V3( pBTIda, &newRecord, KEYREC );
    if ( newRecord )
    {
      BTREELOCKRECORD( newRecord );
      /* We can't simply split a root node, since only one is allowed */
      /* so a new root is created to hold the records                 */
      pBT->usFirstNode = PARENT(*record) = RECORDNUM(newRecord);
      TYPE(newRecord) = ROOT_NODE | INNER_NODE | DATA_KEYNODE;
      PARENT(newRecord) = PREV(newRecord) = NEXT(newRecord) = 0L;
      TYPE(*record) &= ~ROOT_NODE;
      recData.usNum = RECORDNUM(*record);
      recData.usOffset = 0;
      pParentKey = QDAMGetszKey_V3( *record, 0, pBT->usVersion );
      if ( pParentKey )
      {
         // store key temporarily, because record with key data is
         // not locked.
         sRc = QDAMInsertKey_V3(pBTIda, newRecord, pParentKey, recKey, recData);
         // might be freed during insert
         BTREELOCKRECORD( pRecTemp );
      }
      else
      {
        sRc = BTREE_CORRUPTED;
        ERREVENT2( QDAMSPLITNODE_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
      } /* endif */
      BTREEUNLOCKRECORD(newRecord);
      // update usFirstLeaf information
      if ( !sRc )
      {
         sRc = QDAMFirstEntry_V3( pBTIda, &newRecord );
      } /* endif */
      if ( !sRc )
      {
         sRc = QDAMWriteHeader( pBTIda );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
     // allocate space for the new record
    sRc = QDAMNewRecord_V3( pBTIda, &newRecord, KEYREC );
    if ( newRecord )
    {
      BTREELOCKRECORD(newRecord);
      if ( NEXT(*record) )
      {
        sRc = QDAMReadRecord_V3(pBTIda, NEXT(*record), &child, FALSE );
        if ( !sRc )
        {
          PREV(child) = RECORDNUM(newRecord);
          sRc = QDAMWriteRecord_V3(pBTIda, child);
        }
      }

      if ( !sRc )
      {
        /* Adjust Sibling pointers */
        NEXT(newRecord) = NEXT(*record);
        PREV(newRecord) = RECORDNUM(*record);
        NEXT(*record) = RECORDNUM(newRecord);
        PARENT(newRecord) = PARENT(*record);
        TYPE(newRecord) = (CHAR)(TYPE(*record) & ~ROOT_NODE);  // don't copy Root bit

        // Decide where to split the record
        pParentKey = QDAMGetszKey_V3( *record, (SHORT)(OCCUPIED(*record)/2), pBT->usVersion );
        if ( pParentKey )
        {
           fCompare = ((*(pBT->compare))(pBTIda, pParentKey, pKey) <= 0 ) ;
           /***********************************************************/
           /* check in which part we will lay                         */
           /***********************************************************/
           if ( fCompare )
           {
              pParentKey = QDAMGetszKey_V3(*record, (SHORT)(OCCUPIED(*record)-MINFREEKEYS), pBT->usVersion);
              fCompare = ((*(pBT->compare))(pBTIda, pParentKey, pKey) <= 0 ) ;
              if ( fCompare )
              {
                usFreeKeys = MINFREEKEYS;
              }
              else
              {
                usFreeKeys = OCCUPIED( *record )/2;
              } /* endif */
           }
           else
           {
              pParentKey = QDAMGetszKey_V3( *record,MINFREEKEYS, pBT->usVersion);
              fCompare = ((*(pBT->compare))(pBTIda, pParentKey, pKey) <= 0 ) ;
              if ( fCompare )
              {
                usFreeKeys = OCCUPIED( *record )/2;
              }
              else
              {
                usFreeKeys = OCCUPIED( *record ) -MINFREEKEYS;
              } /* endif */
           } /* endif */
        }
        else
        {
           sRc = BTREE_CORRUPTED;
           ERREVENT2( QDAMSPLITNODE_LOC, STATE_EVENT, 2, DB_GROUP, NULL );
        } /* endif */

        // leave space for at least nn further keys instead of splitting at half
        // regard the fact that IMPORT is normally in alphabetical increasing
        // order
        // in case of reverse copying ( as done in old DAM ) we are in the
        // ELSE case...
        if ( !sRc )
        {
           i = (SHORT) (OCCUPIED( *record ) - usFreeKeys);
           j = 0;
           pusOffset = (PUSHORT) (*record)->contents.uchData;
           while ( i < (SHORT) OCCUPIED( *record ))
           {
             QDAMCopyKeyTo_V3( *record, i, newRecord, j, pBT->usVersion );
             *(pusOffset+i) = 0 ;    // mark it as deleted
             j++;
             i++;
           }
           /* Adjust count of keys */
           OCCUPIED(*record) = OCCUPIED(*record) - usFreeKeys;
           OCCUPIED(newRecord) =  usFreeKeys;
           QDAMReArrangeKRec_V3( pBTIda, *record );
        /* Insert pointer to new record into the parent node */
        /* due to the construction parent MUST be the same   */
           sRc = QDAMFindParent_V3( pBTIda, *record, &usParent );
           if ( !sRc && usParent )
           {
             sRc = QDAMReadRecord_V3(pBTIda, usParent, &parent, FALSE );
           } /* endif */
        } /* endif */

        if ( !sRc )
        {
          recData.usNum = RECORDNUM( newRecord );
          recData.usOffset = 0;
          pParentKey = QDAMGetszKey_V3( newRecord,0, pBT->usVersion );
          if ( pParentKey )
          {
             sRc = QDAMInsertKey_V3(pBTIda, parent, pParentKey, recKey, recData);
          }
          else
          {
             sRc = BTREE_CORRUPTED;
             ERREVENT2( QDAMSPLITNODE_LOC, STATE_EVENT, 3, DB_GROUP, NULL );
          } /* endif */
        } /* endif */
      } /* endif */

      /* Decide whether to add the new key to the old or new record */
      if ( !sRc )
      {
//       newRecord->ulCheckSum = QDAMComputeCheckSum( newRecord );
//       (*record)->ulCheckSum = QDAMComputeCheckSum( *record );
         pParentKey = QDAMGetszKey_V3( newRecord, 0, pBT->usVersion );
         if ( pParentKey )
         {
           if ( (*(pBT->compare))(pBTIda, pParentKey, pKey) <= 0)
           {
             sRc = QDAMWriteRecord_V3( pBTIda, *record);
             *record = newRecord;                     // add to new record
           }
           else
           {
             sRc = QDAMWriteRecord_V3( pBTIda, newRecord );
           } /* endif */
         }
         else
         {
            sRc = BTREE_CORRUPTED;
            ERREVENT2( QDAMSPLITNODE_LOC, STATE_EVENT, 4, DB_GROUP, NULL );
         } /* endif */
      } /* endif */
      BTREEUNLOCKRECORD(newRecord);
    } /* endif */
  } /* endif */
  BTREEUNLOCKRECORD( pRecTemp );

   if ( sRc )
   {
     ERREVENT2( QDAMSPLITNODE_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return ( sRc );
}
SHORT QDAMSplitNode_V2
(
   PBTREE pBTIda,                // pointer to generic structure
   PBTREEBUFFER_V2 *record,         // pointer to pointer to node
   PCHAR_W pKey                 // new key
)
{
  SHORT i,j;
  PBTREEBUFFER_V2 newRecord;
  PBTREEBUFFER_V2 child;
  PBTREEBUFFER_V2 parent = NULL;
  USHORT       usParent;               // number of parent
  PBTREEBUFFER_V2 pRecTemp;               // temporary buffer
  RECPARAM   recKey;                   // position/offset for key
  RECPARAM   recData;                  // position/offset for data
  PCHAR_W    pParentKey;               // new key to be inserted
  PUSHORT    pusOffset;                // pointer to offset table
  BOOL       fCompare;                 // indicator where to insert new key
  USHORT     usFreeKeys = 0;               // number of free keys required
  PBTREEGLOB    pBT = pBTIda->pBTree;

  SHORT sRc = 0;                       // success indicator
  memset( &recKey, 0, sizeof( recKey ) );
  memset( &recData,  0, sizeof( recData ) );

  pRecTemp = *record;
  BTREELOCKRECORD( pRecTemp );

  // if root needs to be split do it first
  if (IS_ROOT(*record))
  {
    sRc = QDAMNewRecord_V2( pBTIda, &newRecord, KEYREC );
    if ( newRecord )
    {
      BTREELOCKRECORD( newRecord );
      /* We can't simply split a root node, since only one is allowed */
      /* so a new root is created to hold the records                 */
      pBT->usFirstNode = PARENT(*record) = RECORDNUM(newRecord);
      TYPE(newRecord) = ROOT_NODE | INNER_NODE | DATA_KEYNODE;
      PARENT(newRecord) = PREV(newRecord) = NEXT(newRecord) = 0L;
      TYPE(*record) &= ~ROOT_NODE;
      recData.usNum = RECORDNUM(*record);
      recData.usOffset = 0;
      pParentKey = QDAMGetszKey_V2( *record, 0, pBT->usVersion );
      if ( pParentKey )
      {
         // store key temporarily, because record with key data is
         // not locked.
         sRc = QDAMInsertKey_V2(pBTIda, newRecord, pParentKey, recKey, recData);
         // might be freed during insert
         BTREELOCKRECORD( pRecTemp );
      }
      else
      {
        sRc = BTREE_CORRUPTED;
        ERREVENT2( QDAMSPLITNODE_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
      } /* endif */
      BTREEUNLOCKRECORD(newRecord);
      // update usFirstLeaf information
      if ( !sRc )
      {
         sRc = QDAMFirstEntry_V2( pBTIda, &newRecord );
      } /* endif */
      if ( !sRc )
      {
         sRc = QDAMWriteHeader( pBTIda );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
     // allocate space for the new record
    sRc = QDAMNewRecord_V2( pBTIda, &newRecord, KEYREC );
    if ( newRecord )
    {
      BTREELOCKRECORD(newRecord);
      if ( NEXT(*record) )
      {
        sRc = QDAMReadRecord_V2(pBTIda, NEXT(*record), &child, FALSE );
        if ( !sRc )
        {
          PREV(child) = RECORDNUM(newRecord);
          sRc = QDAMWriteRecord_V2(pBTIda, child);
        }
      }

      if ( !sRc )
      {
        /* Adjust Sibling pointers */
        NEXT(newRecord) = NEXT(*record);
        PREV(newRecord) = RECORDNUM(*record);
        NEXT(*record) = RECORDNUM(newRecord);
        PARENT(newRecord) = PARENT(*record);
        TYPE(newRecord) = (CHAR)(TYPE(*record) & ~ROOT_NODE);  // don't copy Root bit

        // Decide where to split the record
        SHORT sSplitNum = (SHORT)(OCCUPIED(*record)/2);
        pParentKey = QDAMGetszKey_V2( *record, sSplitNum, pBT->usVersion );
        if ( pParentKey )
        {
           // Find the first entry for this term (case insensitive)
           PCHAR_W    pPrevKey;      
           SHORT      sTempSplit; 
           for (sTempSplit=sSplitNum-1 ; sTempSplit>0 ; sTempSplit-- ) {
              pPrevKey = QDAMGetszKey_V2( *record, sTempSplit, pBT->usVersion );
              if ( ( pPrevKey ) &&
                   ( (*(pBT->compare))(pBTIda, pParentKey, pPrevKey) == 0 ) ) 
              {
                 sSplitNum = sTempSplit;
                 pParentKey = pPrevKey ;
              }
           }

           fCompare = ((*(pBT->compare))(pBTIda, pParentKey, pKey) <= 0 ) ;
           /***********************************************************/
           /* check in which part we will lay                         */
           /***********************************************************/
           if ( fCompare )
           {
              pParentKey = QDAMGetszKey_V2(*record, (SHORT)(OCCUPIED(*record)-MINFREEKEYS), pBT->usVersion);

            PCHAR_W    pPrevKey;      
            SHORT      sTempSplit; 
            for (sTempSplit=sSplitNum-1 ; sTempSplit>0 ; sTempSplit-- ) {
               pPrevKey = QDAMGetszKey_V2( *record, sTempSplit, pBT->usVersion );
               if ( ( pPrevKey ) &&
                    ( (*(pBT->compare))(pBTIda, pParentKey, pPrevKey) == 0 ) ) 
               {
                  sSplitNum = sTempSplit;
                  pParentKey = pPrevKey ;
               }
            }
              fCompare = ((*(pBT->compare))(pBTIda, pParentKey, pKey) <= 0 ) ;
              if ( fCompare )
              {
                usFreeKeys = MINFREEKEYS;
              }
              else
              {
                usFreeKeys = sSplitNum;
              } /* endif */
           }
           else
           {
              pParentKey = QDAMGetszKey_V2( *record,MINFREEKEYS, pBT->usVersion);
              fCompare = ((*(pBT->compare))(pBTIda, pParentKey, pKey) <= 0 ) ;
              if ( fCompare )
              {
                usFreeKeys = sSplitNum;
              }
              else
              {
                usFreeKeys = OCCUPIED( *record ) -MINFREEKEYS;
              } /* endif */
           } /* endif */
        }
        else
        {
           sRc = BTREE_CORRUPTED;
           ERREVENT2( QDAMSPLITNODE_LOC, STATE_EVENT, 2, DB_GROUP, NULL );
        } /* endif */

        // leave space for at least nn further keys instead of splitting at half
        // regard the fact that IMPORT is normally in alphabetical increasing
        // order
        // in case of reverse copying ( as done in old DAM ) we are in the
        // ELSE case...
        if ( !sRc )
        {
           i = OCCUPIED( *record );
           j = usFreeKeys;
           PCHAR_W    pBaseKey, pPrevKey;      
           pBaseKey = QDAMGetszKey_V2( *record, i-j, pBT->usVersion);
           if ( pBaseKey ) {
              // Find the first entry for this term (case insensitive)
              for (j+=1 ; j<i ; j++ ) {
                 pPrevKey = QDAMGetszKey_V2( *record, i-j, pBT->usVersion );
                 if ( ( pPrevKey ) &&
                      ( (*(pBT->compare))(pBTIda, pParentKey, pPrevKey) == 0 ) ) 
                 {
                    pParentKey = pPrevKey ;  
                    usFreeKeys++;
                 } else {
                    break;
                 }
                 
              }
           }
           i = (SHORT) (OCCUPIED( *record ) - usFreeKeys);
           j = 0;
           pusOffset = (PUSHORT) (*record)->contents.uchData;
           while ( i < (SHORT) OCCUPIED( *record ))
           {
             QDAMCopyKeyTo_V2( *record, i, newRecord, j, pBT->usVersion );
             *(pusOffset+i) = 0 ;    // mark it as deleted
             j++;
             i++;
           }
           /* Adjust count of keys */
           OCCUPIED(*record) = OCCUPIED(*record) - usFreeKeys;
           OCCUPIED(newRecord) =  usFreeKeys;
           QDAMReArrangeKRec_V2( pBTIda, *record );
        /* Insert pointer to new record into the parent node */
        /* due to the construction parent MUST be the same   */
           sRc = QDAMFindParent_V2( pBTIda, *record, &usParent );
           if ( !sRc && usParent )
           {
             sRc = QDAMReadRecord_V2(pBTIda, usParent, &parent, FALSE );
           } /* endif */
        } /* endif */

        if ( !sRc )
        {
          recData.usNum = RECORDNUM( newRecord );
          recData.usOffset = 0;
          pParentKey = QDAMGetszKey_V2( newRecord,0, pBT->usVersion );
          if ( pParentKey )
          {
             sRc = QDAMInsertKey_V2(pBTIda, parent, pParentKey, recKey, recData);
          }
          else
          {
             sRc = BTREE_CORRUPTED;
             ERREVENT2( QDAMSPLITNODE_LOC, STATE_EVENT, 3, DB_GROUP, NULL );
          } /* endif */
        } /* endif */
      } /* endif */

      /* Decide whether to add the new key to the old or new record */
      if ( !sRc )
      {
//       newRecord->ulCheckSum = QDAMComputeCheckSum( newRecord );
//       (*record)->ulCheckSum = QDAMComputeCheckSum( *record );
         pParentKey = QDAMGetszKey_V2( newRecord, 0, pBT->usVersion );
         if ( pParentKey )
         {
           if ( (*(pBT->compare))(pBTIda, pParentKey, pKey) <= 0)
           {
             sRc = QDAMWriteRecord_V2( pBTIda, *record);
             *record = newRecord;                     // add to new record
           }
           else
           {
             sRc = QDAMWriteRecord_V2( pBTIda, newRecord );
           } /* endif */
         }
         else
         {
            sRc = BTREE_CORRUPTED;
            ERREVENT2( QDAMSPLITNODE_LOC, STATE_EVENT, 4, DB_GROUP, NULL );
         } /* endif */
      } /* endif */
      BTREEUNLOCKRECORD(newRecord);
    } /* endif */
  } /* endif */
  BTREEUNLOCKRECORD( pRecTemp );

   if ( sRc )
   {
     ERREVENT2( QDAMSPLITNODE_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return ( sRc );
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMInsertKey       Insert a key
//------------------------------------------------------------------------------
// Function call:     QDAMInsertKey( PBTREE, PBTREEBUFFER, PCHAR,
//                                    RECPARAM, RECPARAM );
//------------------------------------------------------------------------------
// Description:       Add a key and offset to a record. If there is no room,
//                    the record is split in half and the key is add to the
//                    appropriate half. The key being inserted must be unique.
//------------------------------------------------------------------------------
// Parameters:        PBTREE             pointer to btree structure
//                    PBTREEBUFFER       record where to insert the key
//                    PCHAR              key to be inserted
//                    RECPARAM           position/offset of the key string
//                    RECPARAM           pos.   /offset of data (if data node)
//                                               else position of child record
//
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   if key could be inserted
//                    FALSE  else
//
//------------------------------------------------------------------------------
// Side effects:      Splitting a record will affect the parent node(s) of a
//                    record.
//------------------------------------------------------------------------------
// Function flow:     lock record
//                    check if key is already there
//                    if key found
//                      set RC=BTREE_DUPLICATE_KEY
//                    else
//                      get next free position in record
//                      if node is full
//                        split node before inserting
//                      endif
//                    endif
//                    find proper location in record to insert key
//                    set new current positioon
//                    fill in key data
//                    insert reference, set data information
//                    set address of key data at correct offset
//                    write record
//                    if this is first key in record
//                      adjust above pointers
//                    endif
//                    unlock record
//
//------------------------------------------------------------------------------
SHORT QDAMInsertKey_V3
(
   PBTREE       pBTIda,
   PBTREEBUFFER_V3 pRecord,               // record where key is to be inserted
   PCHAR_W      pKey,
   RECPARAM   recKey,                  // position/offset for key
   RECPARAM   recData                  // position/offset for data
)
{
  SHORT i = 0;
  PBTREEBUFFER_V3 pTempRec;
  PCHAR_W   pCompKey = NULL;           // key to be compared with
  PCHAR_W   pOldKey;                   // old key at first position
  PCHAR_W   pNewKey;                   // new key at first position
  BOOL fFound = FALSE;
  SHORT  sKeyFound;                    // key found
  SHORT  sNearKey;                     // key found
  SHORT  sRc = 0;                      // return code
  USHORT usLastPos;
  USHORT usKeyLen = 0;                 // length of key
  USHORT usDataRec = 0;                // length of key record
  PBYTE  pData;
  PUSHORT  pusOffset = NULL;
  USHORT usCurrentRecord;              // current record
  SHORT  sCurrentIndex;                // current index
  PBTREEGLOB    pBT = pBTIda->pBTree;
   BOOL         fRecLocked = FALSE;    // TRUE if BTREELOCKRECORD has been done

  recKey;                              // get rid of compiler warnings
  //  check if key is already there -- duplicates will not be supported
  sRc = QDAMLocateKey_V3( pBTIda, pRecord, pKey, &sKeyFound, FEXACT, &sNearKey);
  BTREELOCKRECORD( pRecord );
  fRecLocked = TRUE;
  if ( !sRc )
  {
     if ( sKeyFound != -1)
     {
       sRc = BTREE_DUPLICATE_KEY;
     }
     else
     {
       i = (SHORT) OCCUPIED(pRecord);
       usKeyLen = (USHORT)((pBT->fTransMem) ? sizeof(ULONG) : (UTF16strlenBYTE(pKey) + sizeof(CHAR_W)));

       if ( pBT->usVersion >= NTM_VERSION2 )
       {
         usDataRec = usKeyLen + sizeof(USHORT) + sizeof(RECPARAM);
       }
       else
       {
         usDataRec = usKeyLen + sizeof(USHORT) + sizeof(RECPARAMOLD);
       } /* endif */
       /* If node is full. Split the node before inserting */
       if ( usDataRec+sizeof(USHORT)+pRecord->contents.header.usFilled > BTREE_REC_SIZE_V3 )
       {
         BTREEUNLOCKRECORD( pRecord );
         fRecLocked = FALSE;
         sRc = QDAMSplitNode_V3( pBTIda, &pRecord, pKey );
         if ( !sRc )
         {
           // SplitNode may have passed a new record back
           BTREELOCKRECORD( pRecord );
           fRecLocked = TRUE;

           /* if we split an inner node, the parent of the key we are */
           /* inserting must be changed                               */
           if ( !IS_LEAF( pRecord ) )
           {
             sRc = QDAMReadRecord_V3(pBTIda, recData.usNum, &pTempRec, FALSE );
             if ( ! sRc )
             {
               PARENT( pTempRec ) = RECORDNUM( pRecord );
//             pTempRec->ulCheckSum = QDAMComputeCheckSum( pTempRec );
               sRc = QDAMWriteRecord_V3( pBTIda, pTempRec );
             } /* endif */
           } /* endif */
         } /* endif */
       } /* endif */
     } /* endif */
  } /* endif */


  if ( !sRc )
  {
    i = (SHORT) OCCUPIED(pRecord);               // number of keys in record
    // Insert key at the proper location
    pusOffset = (PUSHORT) pRecord->contents.uchData;
    while ( i > 0 && !fFound )
    {
       pCompKey = QDAMGetszKey_V3( pRecord, (SHORT)(i-1), pBT->usVersion );
       if ( pCompKey )
       {
          if ( (*(pBT->compare))(pBTIda, pKey, pCompKey)  < 0 )
          {
             *(pusOffset+i) = *(pusOffset+i-1);
             i--;
          }
          else
          {
             fFound = TRUE;
          } /* endif */
       }
       else
       {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMINSERTKEY_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
       } /* endif */
    } /* endwhile */

    // set new current position
    pBTIda->sCurrentIndex = i;
    pBTIda->usCurrentRecord = RECORDNUM( pRecord );
  } /* endif */

  if ( !sRc )
  {
      CHAR chHeadTerm[128];
      // fill in key data
      usLastPos = pRecord->contents.header.usLastFilled;
      usLastPos = usLastPos - usDataRec;
      pData = pRecord->contents.uchData + usLastPos;
      // insert reference
      if ( !pBT->fTransMem && pBT->usVersion == BTREE_VERSION)
      {
         Unicode2ASCII( pKey, chHeadTerm, 0L );
         usKeyLen = (USHORT)(strlen(chHeadTerm)+1);
         pKey = (PSZ_W)&chHeadTerm[0];
      }
      *(PUSHORT) pData = usKeyLen;
      {
        PBYTE pTarget;
        if ( pBT->usVersion >= NTM_VERSION2 )
        {
          pTarget = pData + sizeof(USHORT) + sizeof(RECPARAM);
        }
        else
        {
          pTarget = pData + sizeof(USHORT) + sizeof(RECPARAMOLD);
        } /* endif */

        memcpy(pTarget, (PBYTE) pKey, usKeyLen );

      }
      // set data information
      if ( pBT->usVersion >= NTM_VERSION2 )
      {
        memcpy((PRECPARAM) (pData+sizeof(USHORT)), &recData, sizeof(RECPARAM));
      }
      else
      {
        RECPARAMOLD recDataOld;
        recDataOld.usOffset = recData.usOffset;
        recDataOld.usNum    = recData.usNum;
        recDataOld.sLen     = (SHORT)recData.ulLen;
        memcpy((PRECPARAM) (pData+sizeof(USHORT)), &recDataOld, sizeof(RECPARAMOLD));
      } /* endif */

      // set address of key data at offset i
      *(pusOffset+i) = usLastPos;

      OCCUPIED(pRecord) += 1;
      pRecord->contents.header.usLastFilled = usLastPos;
      pRecord->contents.header.usFilled += usDataRec + sizeof(USHORT);
//    pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

      sRc = QDAMWriteRecord_V3(pBTIda,pRecord);
  } /* endif */

  // If this is the first key in the record, we must adjust pointers above
  if ( !sRc )
  {
    if ((i == 0) && (!IS_ROOT(pRecord)))
    {
       pOldKey = QDAMGetszKey_V3( pRecord, 1, pBT->usVersion );
       pNewKey = QDAMGetszKey_V3( pRecord, 0, pBT->usVersion );
       if ( pOldKey && pNewKey )
       {
         if ( pCompKey && PARENT(pRecord) )
         {
            /**********************************************************/
            /* save old current record since ChangeKey will update it */
            /* (implicit call to QDAMInsertKey) and restore it ...   */
            /**********************************************************/
            usCurrentRecord = pBTIda->usCurrentRecord;
            sCurrentIndex   = pBTIda->sCurrentIndex;
            sRc = QDAMChangeKey_V3( pBTIda, PARENT(pRecord), pOldKey, pNewKey);
            pBTIda->usCurrentRecord = usCurrentRecord;
            pBTIda->sCurrentIndex   = sCurrentIndex;

         } /* endif */
       }
       else
       {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMINSERTKEY_LOC, STATE_EVENT, 10, DB_GROUP, NULL );
       } /* endif */
    } /* endif */
  } /* endif */

  if ( pRecord && fRecLocked )
  {
    BTREEUNLOCKRECORD(pRecord);
  } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMINSERTKEY_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}
SHORT QDAMInsertKey_V2
(
   PBTREE       pBTIda,
   PBTREEBUFFER_V2 pRecord,               // record where key is to be inserted
   PCHAR_W      pKey,
   RECPARAM   recKey,                  // position/offset for key
   RECPARAM   recData                  // position/offset for data
)
{
  SHORT i = 0;
  PBTREEBUFFER_V2 pTempRec;
  PCHAR_W   pCompKey = NULL;           // key to be compared with
  PCHAR_W   pOldKey;                   // old key at first position
  PCHAR_W   pNewKey;                   // new key at first position
  BOOL fFound = FALSE;
  SHORT  sKeyFound;                    // key found
  SHORT  sNearKey;                     // key found
  SHORT  sRc = 0;                      // return code
  USHORT usLastPos;
  USHORT usKeyLen = 0;                 // length of key
  USHORT usDataRec = 0;                // length of key record
  PBYTE  pData;
  PUSHORT  pusOffset = NULL;
  USHORT usCurrentRecord;              // current record
  SHORT  sCurrentIndex;                // current index
  PBTREEGLOB    pBT = pBTIda->pBTree;
   BOOL         fRecLocked = FALSE;    // TRUE if BTREELOCKRECORD has been done

  recKey;                              // get rid of compiler warnings
  //  check if key is already there -- duplicates will not be supported
  sRc = QDAMLocateKey_V2( pBTIda, pRecord, pKey, &sKeyFound, FEXACT, &sNearKey);
  BTREELOCKRECORD( pRecord );
  fRecLocked = TRUE;
  if ( !sRc )
  {
     if ( sKeyFound != -1)
     {
       sRc = BTREE_DUPLICATE_KEY;
     }
     else
     {
       i = (SHORT) OCCUPIED(pRecord);
       usKeyLen = (USHORT)((pBT->fTransMem) ? sizeof(ULONG) : (UTF16strlenBYTE(pKey) + sizeof(CHAR_W)));

       if ( pBT->usVersion >= NTM_VERSION2 )
       {
         usDataRec = usKeyLen + sizeof(USHORT) + sizeof(RECPARAM);
       }
       else
       {
         usDataRec = usKeyLen + sizeof(USHORT) + sizeof(RECPARAMOLD);
       } /* endif */
       /* If node is full. Split the node before inserting */
       if ( usDataRec+sizeof(USHORT)+pRecord->contents.header.usFilled > BTREE_REC_SIZE_V2 )
       {
         BTREEUNLOCKRECORD( pRecord );
         fRecLocked = FALSE;
         sRc = QDAMSplitNode_V2( pBTIda, &pRecord, pKey );
         if ( !sRc )
         {
           // SplitNode may have passed a new record back
           BTREELOCKRECORD( pRecord );
           fRecLocked = TRUE;

           /* if we split an inner node, the parent of the key we are */
           /* inserting must be changed                               */
           if ( !IS_LEAF( pRecord ) )
           {
             sRc = QDAMReadRecord_V2(pBTIda, recData.usNum, &pTempRec, FALSE );
             if ( ! sRc )
             {
               PARENT( pTempRec ) = RECORDNUM( pRecord );
//             pTempRec->ulCheckSum = QDAMComputeCheckSum( pTempRec );
               sRc = QDAMWriteRecord_V2( pBTIda, pTempRec );
             } /* endif */
           } /* endif */
         } /* endif */
       } /* endif */
     } /* endif */
  } /* endif */


  if ( !sRc )
  {
    i = (SHORT) OCCUPIED(pRecord);               // number of keys in record
    // Insert key at the proper location
    pusOffset = (PUSHORT) pRecord->contents.uchData;
    while ( i > 0 && !fFound )
    {
       pCompKey = QDAMGetszKey_V2( pRecord, (SHORT)(i-1), pBT->usVersion );
       if ( pCompKey )
       {
          if ( (*(pBT->compare))(pBTIda, pKey, pCompKey)  < 0 )
          {
             *(pusOffset+i) = *(pusOffset+i-1);
             i--;
          }
          else
          {
             fFound = TRUE;
          } /* endif */
       }
       else
       {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMINSERTKEY_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
       } /* endif */
    } /* endwhile */

    // set new current position
    pBTIda->sCurrentIndex = i;
    pBTIda->usCurrentRecord = RECORDNUM( pRecord );
  } /* endif */

  if ( !sRc )
  {
      CHAR chHeadTerm[128];
      // fill in key data
      usLastPos = pRecord->contents.header.usLastFilled;
      usLastPos = usLastPos - usDataRec;
      pData = pRecord->contents.uchData + usLastPos;
      // insert reference
      if ( !pBT->fTransMem && pBT->usVersion == BTREE_VERSION)
      {
         Unicode2ASCII( pKey, chHeadTerm, 0L );
         usKeyLen = (USHORT)(strlen(chHeadTerm)+1);
         pKey = (PSZ_W)&chHeadTerm[0];
      }
      *(PUSHORT) pData = usKeyLen;
      {
        PBYTE pTarget;
        if ( pBT->usVersion >= NTM_VERSION2 )
        {
          pTarget = pData + sizeof(USHORT) + sizeof(RECPARAM);
        }
        else
        {
          pTarget = pData + sizeof(USHORT) + sizeof(RECPARAMOLD);
        } /* endif */

        memcpy(pTarget, (PBYTE) pKey, usKeyLen );

      }
      // set data information
      if ( pBT->usVersion >= NTM_VERSION2 )
      {
        memcpy((PRECPARAM) (pData+sizeof(USHORT)), &recData, sizeof(RECPARAM));
      }
      else
      {
        RECPARAMOLD recDataOld;
        recDataOld.usOffset = recData.usOffset;
        recDataOld.usNum    = recData.usNum;
        recDataOld.sLen     = (SHORT)recData.ulLen;
        memcpy((PRECPARAM) (pData+sizeof(USHORT)), &recDataOld, sizeof(RECPARAMOLD));
      } /* endif */

      // set address of key data at offset i
      *(pusOffset+i) = usLastPos;

      OCCUPIED(pRecord) += 1;
      pRecord->contents.header.usLastFilled = usLastPos;
      pRecord->contents.header.usFilled += usDataRec + sizeof(USHORT);
//    pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

      sRc = QDAMWriteRecord_V2(pBTIda,pRecord);
  } /* endif */

  // If this is the first key in the record, we must adjust pointers above
  if ( !sRc )
  {
    if ((i == 0) && (!IS_ROOT(pRecord)))
    {
       pOldKey = QDAMGetszKey_V2( pRecord, 1, pBT->usVersion );
       pNewKey = QDAMGetszKey_V2( pRecord, 0, pBT->usVersion );
       if ( pOldKey && pNewKey )
       {
         if ( pCompKey && PARENT(pRecord) )
         {
            /**********************************************************/
            /* save old current record since ChangeKey will update it */
            /* (implicit call to QDAMInsertKey) and restore it ...   */
            /**********************************************************/
            usCurrentRecord = pBTIda->usCurrentRecord;
            sCurrentIndex   = pBTIda->sCurrentIndex;
            sRc = QDAMChangeKey_V2( pBTIda, PARENT(pRecord), pOldKey, pNewKey);
            pBTIda->usCurrentRecord = usCurrentRecord;
            pBTIda->sCurrentIndex   = sCurrentIndex;

         } /* endif */
       }
       else
       {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMINSERTKEY_LOC, STATE_EVENT, 10, DB_GROUP, NULL );
       } /* endif */
    } /* endif */
  } /* endif */

  if ( pRecord && fRecLocked )
  {
    BTREEUNLOCKRECORD(pRecord);
  } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMINSERTKEY_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictInsertLocal   Insert entry
//------------------------------------------------------------------------------
// Function call:     QDAMDictInsertLocal( PBTREE, PCHAR, PCHAR, USHORT );
//
//------------------------------------------------------------------------------
// Description:       Add a key and all associated data.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE          pointer to btree structure
//                    PCHAR           key to be inserted
//                    PCHAR           user data to be associated with the key
//                    USHORT          length of the user data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_USERDATA    user data too long
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_DATA_RANGE  key to long or too short
//------------------------------------------------------------------------------
// Function flow:     if dictionary corrupted then
//                      set Rc to BTREE_CORRUPTED
//                    else
//                      find the appropriate record and set Rc accordingly
//                    endif
//                    if okay then
//                      add data to buffer and set Rc
//                      if okay then
//                        insert the key and the data and set Rc
//                      endif
//                    endif
//                    return Rc
//
//------------------------------------------------------------------------------
SHORT QDAMDictInsertLocal
(
  PBTREE  pBTIda,           // pointer to binary tree struct
  PCHAR_W pKey,             // pointer to key data
  PBYTE   pData,            // pointer to user data
  ULONG   ulLen             // length of user data in bytes
)
{
   SHORT sRc = 0;                            // return code
   RECPARAM   recData;                       // offset/node of data storage
   RECPARAM   recKey;                        // offset/node of key storage
   USHORT     usKeyLen;                      // length of the key
   BOOL       fLocked = FALSE;               // file-has-been-locked flag
   PBTREEGLOB    pBT = pBTIda->pBTree;

   memset( &recKey, 0, sizeof( recKey ) );
   memset( &recData,  0, sizeof( recData ) );
   DEBUGEVENT2( QDAMDICTINSERTLOCAL_LOC, FUNCENTRY_EVENT, 0, DB_GROUP, NULL );

   if ( pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */
   if ( !sRc && !pBT->fOpen )
   {
     sRc = BTREE_READONLY;
   } /* endif */

   /*******************************************************************/
   /* check if entry is locked ....                                   */
   /*******************************************************************/
   if ( !sRc && QDAMDictLockStatus( pBTIda, pKey ) )
   {
     sRc = BTREE_ENTRY_LOCKED;
   } /* endif */

   /*******************************************************************/
   /* For shared databases: lock complete file                        */
   /*                                                                 */
   /* Note: this will also update our internal buffers and the        */
   /*       header record. No need to call QDamCheckForUpdates here.  */
   /*******************************************************************/
   if ( !sRc && (pBT->usOpenFlags & ASD_SHARED) )
   {
     sRc = QDAMPhysLock( pBTIda, TRUE, &fLocked );
   } /* endif */

   if ( pBT->bRecSizeVersion == BTREE_V3 )
   {
      PBTREEBUFFER_V3  pNode = NULL;

      if ( !sRc )
      {
        usKeyLen = (USHORT)((pBT->fTransMem) ? sizeof(ULONG) : UTF16strlenBYTE( pKey ));
        if ( (usKeyLen == 0) ||
             ((usKeyLen >= HEADTERM_SIZE * sizeof(CHAR_W))) ||
             (ulLen == 0) ||
             ((pBT->usVersion < NTM_VERSION2) && (ulLen >= MAXDATASIZE)) )
        {
          sRc = BTREE_DATA_RANGE;
        }
        else
        {
          memcpy( (PBYTE)pBTIda->chHeadTerm, (PBYTE)pKey, usKeyLen+sizeof(CHAR_W) );   // save current data
          QDAMDictUpdStatus ( pBTIda );
          sRc = QDAMFindRecord_V3( pBTIda, pKey, &pNode );
        } /* endif */
      } /* endif */

      if ( !sRc )
      {
          BTREELOCKRECORD( pNode );
          sRc = QDAMAddToBuffer_V3( pBTIda, pData, ulLen, &recData );
          if ( !sRc )
          {
            recData.ulLen = ulLen;
            sRc = QDAMInsertKey_V3 (pBTIda, pNode, pKey, recKey, recData);
          } /* endif */

          BTREEUNLOCKRECORD( pNode );

          /****************************************************************/
          /* change time to indicate modifications on dictionary...       */
          /****************************************************************/
          pBT->lTime ++;
      }
   }
   else
   {
      PBTREEBUFFER_V2  pNode = NULL;

      if ( !sRc )
      {
        usKeyLen = (USHORT)((pBT->fTransMem) ? sizeof(ULONG) : UTF16strlenBYTE( pKey ));
        if ( usKeyLen == 0 ||(  (usKeyLen >= HEADTERM_SIZE * sizeof(CHAR_W))) || ulLen == 0 || ulLen >= MAXDATASIZE )
        {
          sRc = BTREE_DATA_RANGE;
        }
        else
        {
          memcpy( (PBYTE)pBTIda->chHeadTerm, (PBYTE)pKey, usKeyLen+sizeof(CHAR_W) );   // save current data
          QDAMDictUpdStatus ( pBTIda );
          sRc = QDAMFindRecord_V2( pBTIda, pKey, &pNode );
        } /* endif */
      } /* endif */

      if ( !sRc )
      {
          BTREELOCKRECORD( pNode );
          sRc = QDAMAddToBuffer_V2( pBTIda, pData, ulLen, &recData );
          if ( !sRc )
          {
            recData.ulLen = ulLen;
            sRc = QDAMInsertKey_V2 (pBTIda, pNode, pKey, recKey, recData);
          } /* endif */

          BTREEUNLOCKRECORD( pNode );

          /****************************************************************/
          /* change time to indicate modifications on dictionary...       */
          /****************************************************************/
          pBT->lTime ++;
      }
   } /* endif */

   /*******************************************************************/
   /* For shared databases: unlock complete file                      */
   /*                                                                 */
   /* Note: this will also incement the dictionary update counter     */
   /*       if the dictionary has been modified                       */
   /*******************************************************************/
   if ( fLocked )
   {
     sRc = QDAMPhysLock( pBTIda, FALSE, NULL );
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMDICTINSERTLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return sRc;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMChangeKey      Change the key
//------------------------------------------------------------------------------
// Function call:     QDAMChangeKey( PBTREE, USHORT, PCHAR, PCHAR );
//
//------------------------------------------------------------------------------
// Description:       change the key to a new value
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    USHORT                 node to be used
//                    PCHAR                  old key value
//                    PCHAR                  new key value
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_INVALID     pointer invalid
//                    BTREE_USERDATA    user data too long
//                    BTREE_CORRUPTED   dictionary is corrupted
//
//------------------------------------------------------------------------------
// Function flow:     read record
//                    if ok
//                      lock record
//                      if ok
//                        locate leaf node that contains appropriate key
//                      endif
//                      if ok
//                        get key description and insert new key
//                      endif
//                      if ok
//                        locate leaf node with old key
//                      endif
//                      if ok
//                        delete old key
//                        rearrange key record
//                      endif
//                      unlock record
//                    endif
//
//------------------------------------------------------------------------------
SHORT QDAMChangeKey_V2
(
   PBTREE   pBTIda,                                 // ptr to tree structure
   USHORT   usNode,                                 // start node
   PCHAR_W  pOldKey,                                // find old key
   PCHAR_W  pNewKey                                 // find new key
)
{
  PBTREEBUFFER_V2 pRecord;                            // buffer for record
  PBTREEBUFFER_V2 pNewRecord = NULL;                  // buffer for new record
  SHORT i = 0;                                     // index
  SHORT sRc = 0;                                   // return code
  SHORT sNearKey;
  PUSHORT  pusOffset;                              // pointer to offset table
  RECPARAM recKey;                                 // record parameter descrip.

  sRc = QDAMReadRecord_V2( pBTIda, usNode, &pRecord, FALSE  );
  if ( !sRc )
  {
    BTREELOCKRECORD(pRecord);
    if ( !sRc )
    {
       /* Locate the Leaf node that contains the appropriate key */
       sRc = QDAMFindChild_V2( pBTIda, pNewKey, usNode, &pNewRecord );
    } /* endif */
    if ( !sRc )
    {
       // get the key description and insert the new key
       recKey.usNum = pNewRecord->contents.header.usNum;
       sRc = QDAMLocateKey_V2(pBTIda, pNewRecord, pNewKey, &i, FEXACT, &sNearKey);
       if ( !sRc && i != -1 )
       {
          recKey.usOffset = i;
          recKey.ulLen = 0;            // init length
          sRc = QDAMInsertKey_V2( pBTIda, pRecord, pNewKey, recKey, recKey);
       } /* endif */
    } /* endif */
    if ( !sRc )
    {
       sRc = QDAMLocateKey_V2( pBTIda, pRecord, pOldKey, &i, FEXACT, &sNearKey);
    } /* endif */

    if (!sRc && i!= -1)
    {
       // delete the oldkey at offset i
       pusOffset = (PUSHORT) pRecord->contents.uchData;
       *(pusOffset + i) = 0;
       OCCUPIED(pRecord) --;
       // record should be rearranged - it will be written during the insert
       // of the new key
       QDAMReArrangeKRec_V2( pBTIda, pRecord );

    }
    BTREEUNLOCKRECORD(pRecord);
  }

   if ( sRc )
   {
     ERREVENT2( QDAMCHANGEKEY_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}

SHORT QDAMChangeKey_V3
(
   PBTREE   pBTIda,                                 // ptr to tree structure
   USHORT   usNode,                                 // start node
   PCHAR_W  pOldKey,                                // find old key
   PCHAR_W  pNewKey                                 // find new key
)
{
  PBTREEBUFFER_V3 pRecord;                            // buffer for record
  PBTREEBUFFER_V3 pNewRecord = NULL;                  // buffer for new record
  SHORT i = 0;                                     // index
  SHORT sRc = 0;                                   // return code
  SHORT sNearKey;
  PUSHORT  pusOffset;                              // pointer to offset table
  RECPARAM recKey;                                 // record parameter descrip.

  sRc = QDAMReadRecord_V3( pBTIda, usNode, &pRecord, FALSE  );
  if ( !sRc )
  {
    BTREELOCKRECORD(pRecord);
    if ( !sRc )
    {
       /* Locate the Leaf node that contains the appropriate key */
       sRc = QDAMFindChild_V3( pBTIda, pNewKey, usNode, &pNewRecord );
    } /* endif */
    if ( !sRc )
    {
       // get the key description and insert the new key
       recKey.usNum = pNewRecord->contents.header.usNum;
       sRc = QDAMLocateKey_V3(pBTIda, pNewRecord, pNewKey, &i, FEXACT, &sNearKey);
       if ( !sRc && i != -1 )
       {
          recKey.usOffset = i;
          recKey.ulLen = 0;            // init length
          sRc = QDAMInsertKey_V3( pBTIda, pRecord, pNewKey, recKey, recKey);
       } /* endif */
    } /* endif */
    if ( !sRc )
    {
       sRc = QDAMLocateKey_V3( pBTIda, pRecord, pOldKey, &i, FEXACT, &sNearKey);
    } /* endif */

    if (!sRc && i!= -1)
    {
       // delete the oldkey at offset i
       pusOffset = (PUSHORT) pRecord->contents.uchData;
       *(pusOffset + i) = 0;
       OCCUPIED(pRecord) --;
       // record should be rearranged - it will be written during the insert
       // of the new key
       QDAMReArrangeKRec_V3( pBTIda, pRecord );

    }
    BTREEUNLOCKRECORD(pRecord);
  }

   if ( sRc )
   {
     ERREVENT2( QDAMCHANGEKEY_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMAddToBuffer       Add Data to buffer
//------------------------------------------------------------------------------
// Function call:     QDAMAddToBuffer( PBTREE, PCHAR, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Add the passed data to the string buffer
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key to be inserted
//                    PCHAR                  buffer for user data
//                    PRECPARAM              contains position of data stored
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
// Function flow:     if terse flag is on
//                      terse the data
//                    endif
//                    get list for currently used record type
//                    check if sufficient space is available to
//                       fit into one of these records
//                    if it fits
//                      read record
//                      fill in key data, insert reference
//                      set address of key data
//                      mark if tersed or not
//                      write record update list entry
//                    endif
//                    if record is too large or no space left in list of recs
//                       scan list and find empty slot
//                       if not found
//                         remove slot which is most filled
//                       endif
//                       get record and store start position
//                       fill it up to length or record size
//                       update list entry
//                    endif
//                    return record parameters
//
//------------------------------------------------------------------------------
SHORT  QDAMAddToBuffer_V2
(
   PBTREE  pBTIda,                     // pointer to btree struct
   PBYTE   pData,                      // pointer to data
   ULONG   ulDataLen,                  // length of data to be filled in
   PRECPARAM  precReturn               // pointer to return code
)
{
   SHORT sRc = 0;                      // success indicator
   USHORT  usFilled;                   // number of bytes filled in record
   USHORT  usMaxRec= 0;                // most filled record
   RECPARAM recStart;                  // return code where data are stored
   PRECPARAM pRecParam;                // parameter to stored list
   PRECPARAM pRecTemp ;                // parameter to stored list
   BOOL  fFit = FALSE;                 // indicator if record found
   USHORT  i;                          // index
   PBTREEHEADER  pHeader;              // record header
   PBTREEBUFFER_V2 pRecord = NULL;        // buffer record
   PBTREEBUFFER_V2 pTempRecord;           // buffer record
   CHAR     chNodeType;                // type of node
   BOOL     fTerse = FALSE;            // data are not tersed
   USHORT   usLastPos;
   ULONG    ulFullDataLen;             // length of record
   PUSHORT  pusOffset;
   PCHAR    pRecData;                  // pointer to record data
   ULONG    ulFitLen;                  // currently used length
  PBTREEGLOB    pBT = pBTIda->pBTree;
   USHORT   usLenFieldSize;            // size of length field

   memset(&recStart, 0, sizeof(recStart));
   /*******************************************************************/
   /* Enlarge pTempRecord area if it is not large enough to contain   */
   /* the data for this record                                        */
   /*******************************************************************/
#ifdef _DEBUG
   {
     CHAR szTemp[8];
     sprintf( szTemp, "%ld", ulDataLen );
     INFOEVENT2( QDAMADDTOBUFFER_LOC, STATE_EVENT, 4711, DB_GROUP, szTemp );
   }
#endif
   if ( (ulDataLen + sizeof(ULONG)) > pBT->ulTempRecSize )
   {
     if ( UtlAlloc( (PVOID *)&(pBT->pTempRecord), pBT->ulTempRecSize, ulDataLen + sizeof(ULONG), NOMSG ) )
     {
       pBT->ulTempRecSize = ulDataLen + sizeof(ULONG);
     }
     else
     {
       sRc = BTREE_NO_ROOM;
     } /* endif */
   } /* endif */

  if ( pBT->usVersion >= NTM_VERSION2 )
  {
    usLenFieldSize = sizeof(ULONG);
  }
  else
  {
    usLenFieldSize = sizeof(USHORT);
  } /* endif */

   pRecParam = pBT->DataRecList;
   /*******************************************************************/
   /* determine type of compression....                               */
   /*******************************************************************/
   switch ( pBT->fTerse )
   {
     case  BTREE_TERSE_HUFFMAN :
       fTerse = QDAMTerseData( pBTIda, pData, &ulDataLen, pBT->pTempRecord );
       break;
     default :
       fTerse = FALSE;
       break;
   } /* endswitch */

   chNodeType = DATA_NODE;

   // data should be in pBT->pTempRecord for further processing
   // copy them if not yet done.....
   if ( !fTerse )
   {
      memcpy( pBT->pTempRecord, pData, ulDataLen );
   } /* endif */

   if ( !sRc && (ulDataLen <= BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER)) )
   {
      pRecTemp = pRecParam;
      i = 0;
      //  find slot in list
      usFilled = (USHORT)(BTREE_REC_SIZE_V2 - ulDataLen - 2 * usLenFieldSize);
      while ( !fFit && i < MAX_LIST )
      {
         if ( pRecTemp->usOffset == 0 || pRecTemp->usOffset > usFilled )   // will not fit
         {
            pRecTemp++;
            i++;
         }
         else                                     // data will fit
         {
            fFit = TRUE;
         } /* endif */
      } /* endwhile */


      if ( fFit )
      {
         // store position where data will be stored
         recStart.usNum    = pRecTemp->usNum;

         // get record, copy data and write them
         sRc = QDAMReadRecord_V2( pBTIda, recStart.usNum, &pRecord, FALSE  );
         if ( !sRc )
         {
            BTREELOCKRECORD( pRecord );
            if ( TYPE( pRecord ) != chNodeType )
            {
               sRc = BTREE_CORRUPTED;
               ERREVENT2( QDAMADDTOBUFFER_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
            }
            else
            {
               // fill in key data
               usLastPos = pRecord->contents.header.usLastFilled;
               usLastPos = usLastPos - (USHORT)(ulDataLen + usLenFieldSize);
               pData = pRecord->contents.uchData + usLastPos;
               // insert reference
               if ( pBT->usVersion >= NTM_VERSION2 )
               {
                 *(PULONG) pData = ulDataLen;
               }
               else
               {
                 *(PUSHORT) pData = (USHORT)ulDataLen;
               }
               memcpy( pData+usLenFieldSize, pBT->pTempRecord, ulDataLen );

               // set address of key data at next free offset
               pusOffset = (PUSHORT) pRecord->contents.uchData;
               *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

               OCCUPIED(pRecord) += 1;
               pRecord->contents.header.usLastFilled = usLastPos;
               pRecord->contents.header.usFilled = pRecord->contents.header.usFilled + (USHORT)(ulDataLen+usLenFieldSize+sizeof(USHORT));

               // mark if tersed or not
               if ( fTerse )
               {
                 if ( pBT->usVersion >= NTM_VERSION2 )
                 {
                   *(PULONG) pData |= QDAM_TERSE_FLAGL;
                 }
                 else
                 {
                   *(PUSHORT) pData |= QDAM_TERSE_FLAG;
                 }
               } /* endif */
//             pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

               sRc = QDAMWriteRecord_V2(pBTIda, pRecord);
               // update list entry
               if ( !sRc )
               {
                  pRecTemp->usOffset = pRecord->contents.header.usFilled;
               } /* endif */
            } /* endif */
            BTREEUNLOCKRECORD( pRecord );
         } /* endif */
      } /* endif */
   } /* endif */

   //  either record is too large or no space left in list of records

   if ( !sRc && !fFit )
   {
      pRecTemp = pRecParam;
      i = 0;
      // scan list and find empty slot
      while ( (i < MAX_LIST) && !fFit )
      {
         if ( pRecTemp->usNum == 0 )     // empty slot found
         {
            fFit = TRUE;
         }
         else
         {
            pRecTemp++;                  // point to next slot
            i++;
         } /* endif */
      } /* endwhile */

      /****************************************************************/
      /* if not found remove slot which is most filled                */
      /****************************************************************/
      if ( !fFit )
      {
         pRecTemp = pRecParam;
         usFilled = 0;
         for ( i=0 ; i < MAX_LIST ;i++ )
         {
            if ( pRecTemp->usOffset > usFilled)
            {
               usMaxRec = i;
               usFilled = pRecTemp->usOffset;
            } /* endif */
            pRecTemp++;                // next record in list
         } /* endfor */
         // set pRecTemp to the selected slot
         pRecTemp = pRecParam + usMaxRec;
      } /* endif */

      // get record and store start position
      memset( pRecTemp, 0 , sizeof(RECPARAM));

      sRc = QDAMNewRecord_V2( pBTIda, &pRecord, DATAREC );
      // fill it up to length or record size
      if ( ! sRc )
      {
         TYPE( pRecord ) = chNodeType;     // indicate that it is a data node
         BTREELOCKRECORD( pRecord );
         pRecData = (PCHAR)(pBT->pTempRecord + ulDataLen);
         ulFullDataLen = ulDataLen;         // store original data len
         while ( !sRc && ulDataLen + pRecord->contents.header.usFilled  >= (BTREE_REC_SIZE_V2 - (usLenFieldSize + sizeof(SHORT))))
         {
           /***********************************************************/
           /* get a new record, anchor it and fill data from end      */
           /* new record will be predecessor of already allocated rec.*/
           /***********************************************************/
           sRc = QDAMNewRecord_V2( pBTIda, &pTempRecord, DATAREC );
           if ( !sRc )
           {
             TYPE( pTempRecord ) = chNodeType;   // data node
             NEXT( pTempRecord ) = RECORDNUM( pRecord );
             PREV( pRecord ) = RECORDNUM( pTempRecord );
             TYPE( pRecord ) = DATA_NEXTNODE;              // data node

             // fill in key data
             /*********************************************************/
             /* adjust sizes of relevant data                         */
             /*********************************************************/
             usLastPos = pRecord->contents.header.usLastFilled;
             {
               ULONG ulMax = (ULONG)(BTREE_REC_SIZE_V2 - pRecord->contents.header.usFilled );
               ulFitLen = min( ulDataLen, ulMax );
               ulFitLen -= (ULONG)(usLenFieldSize + sizeof(USHORT));
             }
             usLastPos = usLastPos - (USHORT)(ulFitLen + (ULONG)usLenFieldSize);
             pData = pRecord->contents.uchData + usLastPos;
             pRecData -= ulFitLen;
             ulDataLen -= ulFitLen;
             /*********************************************************/
             /* store length and copy data of part contained in record*/
             /*********************************************************/
             if ( pBT->usVersion >= NTM_VERSION2 )
             {
               *(PULONG) pData = ulFitLen;
             }
             else
             {
               *(PUSHORT) pData = (USHORT)ulFitLen;
             }
             memcpy( pData+usLenFieldSize, pRecData,  ulFitLen );
             // set address of key data at next free offset
             pusOffset = (PUSHORT) pRecord->contents.uchData;
             *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

             OCCUPIED(pRecord) += 1;
             pRecord->contents.header.usLastFilled = usLastPos;
             pRecord->contents.header.usFilled = pRecord->contents.header.usFilled +
                                           (USHORT)(ulFitLen + (ULONG)usLenFieldSize);
//           pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

             sRc = QDAMWriteRecord_V2(pBTIda, pRecord);
             BTREEUNLOCKRECORD( pRecord );

             /*********************************************************/
             /* toggle for adressing purposes                         */
             /* now: pRecord will be the free one for later filling   */
             /*********************************************************/
             pRecord = pTempRecord;
             BTREELOCKRECORD( pRecord );
           } /* endif */
         } /* endwhile */

         /*************************************************************/
         /* write out rest (this is the normal way in most cases      */
         /*************************************************************/
         if ( !sRc )
         {
           pHeader = &(pRecord->contents.header);
           pHeader->usParent = sizeof(BTREEHEADER );

           // store position where data will be stored
           recStart.usNum    = pHeader->usNum;

           // fill in key data
           usLastPos = pRecord->contents.header.usLastFilled;

           usLastPos = usLastPos - (USHORT)(ulDataLen + (ULONG)usLenFieldSize);
           pData = pRecord->contents.uchData + usLastPos;
           pRecData -= ulDataLen;

           // insert reference
           if ( pBT->usVersion >= NTM_VERSION2 )
           {
             *(PULONG) pData = ulFullDataLen;
           }
           else
           {
             *(PUSHORT) pData = (USHORT)ulFullDataLen;
           }
           memcpy( pData+usLenFieldSize, pRecData, ulDataLen );

           // set address of key data at next free offset
           pusOffset = (PUSHORT) pRecord->contents.uchData;
           *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

           OCCUPIED(pRecord) += 1;
           pRecord->contents.header.usLastFilled = usLastPos;
           pRecord->contents.header.usFilled = pRecord->contents.header.usFilled +
                                    (USHORT)(ulDataLen+usLenFieldSize+sizeof(USHORT));

           // mark if tersed or not
           if ( fTerse )
           {
             if ( pBT->usVersion >= NTM_VERSION2 )
             {
               *(PULONG) pData |= QDAM_TERSE_FLAGL;
             }
             else
             {
               *(PUSHORT) pData |= QDAM_TERSE_FLAG;
             }
           } /* endif */


           // write record
  //       pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
           sRc = QDAMWriteRecord_V2(pBTIda, pRecord);
           BTREEUNLOCKRECORD( pRecord );         // free the locking
           // update list entry
           if ( !sRc )
           {
              pRecTemp->usOffset = pHeader->usFilled;
              pRecTemp->usNum    = pHeader->usNum;
           } /* endif */
         } /* endif */
      } /* endif */
   } /* endif */

   //  return record parameters
   if ( !sRc )
   {
      precReturn->usNum = recStart.usNum - pBT->usFirstDataBuffer;
      precReturn->usOffset = OCCUPIED( pRecord ) - 1;
   }
   else
   {
      memset(&recStart, 0, sizeof(RECPARAM));
      precReturn = &recStart;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMADDTOBUFFER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return sRc;
}

SHORT  QDAMAddToBuffer_V3
(
   PBTREE  pBTIda,                     // pointer to btree struct
   PBYTE   pData,                      // pointer to data
   ULONG   ulDataLen,                  // length of data to be filled in
   PRECPARAM  precReturn               // pointer to return code
)
{
   SHORT sRc = 0;                      // success indicator
   USHORT  usFilled;                   // number of bytes filled in record
   USHORT  usMaxRec= 0;                // most filled record
   RECPARAM recStart;                  // return code where data are stored
   PRECPARAM pRecParam;                // parameter to stored list
   PRECPARAM pRecTemp ;                // parameter to stored list
   BOOL  fFit = FALSE;                 // indicator if record found
   USHORT  i;                          // index
   PBTREEHEADER  pHeader;              // record header
   PBTREEBUFFER_V3 pRecord = NULL;        // buffer record
   PBTREEBUFFER_V3 pTempRecord;           // buffer record
   CHAR     chNodeType;                // type of node
   BOOL     fTerse = FALSE;            // data are not tersed
   USHORT   usLastPos;
   ULONG    ulFullDataLen;             // length of record
   PUSHORT  pusOffset;
   PCHAR    pRecData;                  // pointer to record data
   ULONG    ulFitLen;                  // currently used length
  PBTREEGLOB    pBT = pBTIda->pBTree;
   USHORT   usLenFieldSize;            // size of length field

   memset(&recStart, 0, sizeof(recStart));
   /*******************************************************************/
   /* Enlarge pTempRecord area if it is not large enough to contain   */
   /* the data for this record                                        */
   /*******************************************************************/
#ifdef _DEBUG
   {
     CHAR szTemp[8];
     sprintf( szTemp, "%ld", ulDataLen );
     INFOEVENT2( QDAMADDTOBUFFER_LOC, STATE_EVENT, 4711, DB_GROUP, szTemp );
   }
#endif
   if ( (ulDataLen + sizeof(ULONG)) > pBT->ulTempRecSize )
   {
     if ( UtlAlloc( (PVOID *)&(pBT->pTempRecord), pBT->ulTempRecSize, ulDataLen + sizeof(ULONG), NOMSG ) )
     {
       pBT->ulTempRecSize = ulDataLen + sizeof(ULONG);
     }
     else
     {
       sRc = BTREE_NO_ROOM;
     } /* endif */
   } /* endif */

  if ( pBT->usVersion >= NTM_VERSION2 )
  {
    usLenFieldSize = sizeof(ULONG);
  }
  else
  {
    usLenFieldSize = sizeof(USHORT);
  } /* endif */

   pRecParam = pBT->DataRecList;
   /*******************************************************************/
   /* determine type of compression....                               */
   /*******************************************************************/
   switch ( pBT->fTerse )
   {
     case  BTREE_TERSE_HUFFMAN :
       fTerse = QDAMTerseData( pBTIda, pData, &ulDataLen, pBT->pTempRecord );
       break;
     default :
       fTerse = FALSE;
       break;
   } /* endswitch */

   chNodeType = DATA_NODE;

   // data should be in pBT->pTempRecord for further processing
   // copy them if not yet done.....
   if ( !fTerse )
   {
      memcpy( pBT->pTempRecord, pData, ulDataLen );
   } /* endif */

   if ( !sRc && (ulDataLen <= BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER)) )
   {
      pRecTemp = pRecParam;
      i = 0;
      //  find slot in list
      usFilled = (USHORT)(BTREE_REC_SIZE_V3 - ulDataLen - 2 * usLenFieldSize);
      while ( !fFit && i < MAX_LIST )
      {
         if ( pRecTemp->usOffset == 0 || pRecTemp->usOffset > usFilled )   // will not fit
         {
            pRecTemp++;
            i++;
         }
         else                                     // data will fit
         {
            fFit = TRUE;
         } /* endif */
      } /* endwhile */


      if ( fFit )
      {
         // store position where data will be stored
         recStart.usNum    = pRecTemp->usNum;

         // get record, copy data and write them
         sRc = QDAMReadRecord_V3( pBTIda, recStart.usNum, &pRecord, FALSE  );
         if ( !sRc )
         {
            BTREELOCKRECORD( pRecord );
            if ( TYPE( pRecord ) != chNodeType )
            {
               sRc = BTREE_CORRUPTED;
               ERREVENT2( QDAMADDTOBUFFER_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
            }
            else
            {
               // fill in key data
               usLastPos = pRecord->contents.header.usLastFilled;
               usLastPos = usLastPos - (USHORT)(ulDataLen + usLenFieldSize);
               pData = pRecord->contents.uchData + usLastPos;
               // insert reference
               if ( pBT->usVersion >= NTM_VERSION2 )
               {
                 *(PULONG) pData = ulDataLen;
               }
               else
               {
                 *(PUSHORT) pData = (USHORT)ulDataLen;
               }
               memcpy( pData+usLenFieldSize, pBT->pTempRecord, ulDataLen );

               // set address of key data at next free offset
               pusOffset = (PUSHORT) pRecord->contents.uchData;
               *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

               OCCUPIED(pRecord) += 1;
               pRecord->contents.header.usLastFilled = usLastPos;
               pRecord->contents.header.usFilled = pRecord->contents.header.usFilled + (USHORT)(ulDataLen+usLenFieldSize+sizeof(USHORT));

               // mark if tersed or not
               if ( fTerse )
               {
                 if ( pBT->usVersion >= NTM_VERSION2 )
                 {
                   *(PULONG) pData |= QDAM_TERSE_FLAGL;
                 }
                 else
                 {
                   *(PUSHORT) pData |= QDAM_TERSE_FLAG;
                 }
               } /* endif */
//             pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

               sRc = QDAMWriteRecord_V3(pBTIda, pRecord);
               // update list entry
               if ( !sRc )
               {
                  pRecTemp->usOffset = pRecord->contents.header.usFilled;
               } /* endif */
            } /* endif */
            BTREEUNLOCKRECORD( pRecord );
         } /* endif */
      } /* endif */
   } /* endif */

   //  either record is too large or no space left in list of records

   if ( !sRc && !fFit )
   {
      pRecTemp = pRecParam;
      i = 0;
      // scan list and find empty slot
      while ( (i < MAX_LIST) && !fFit )
      {
         if ( pRecTemp->usNum == 0 )     // empty slot found
         {
            fFit = TRUE;
         }
         else
         {
            pRecTemp++;                  // point to next slot
            i++;
         } /* endif */
      } /* endwhile */

      /****************************************************************/
      /* if not found remove slot which is most filled                */
      /****************************************************************/
      if ( !fFit )
      {
         pRecTemp = pRecParam;
         usFilled = 0;
         for ( i=0 ; i < MAX_LIST ;i++ )
         {
            if ( pRecTemp->usOffset > usFilled)
            {
               usMaxRec = i;
               usFilled = pRecTemp->usOffset;
            } /* endif */
            pRecTemp++;                // next record in list
         } /* endfor */
         // set pRecTemp to the selected slot
         pRecTemp = pRecParam + usMaxRec;
      } /* endif */

      // get record and store start position
      memset( pRecTemp, 0 , sizeof(RECPARAM));

      sRc = QDAMNewRecord_V3( pBTIda, &pRecord, DATAREC );
      // fill it up to length or record size
      if ( ! sRc )
      {
         TYPE( pRecord ) = chNodeType;     // indicate that it is a data node
         BTREELOCKRECORD( pRecord );
         pRecData = (PCHAR)(pBT->pTempRecord + ulDataLen);
         ulFullDataLen = ulDataLen;         // store original data len
         while ( !sRc && ulDataLen + pRecord->contents.header.usFilled  >= (BTREE_REC_SIZE_V3 - (usLenFieldSize + sizeof(SHORT))))
         {
           /***********************************************************/
           /* get a new record, anchor it and fill data from end      */
           /* new record will be predecessor of already allocated rec.*/
           /***********************************************************/
           sRc = QDAMNewRecord_V3( pBTIda, &pTempRecord, DATAREC );
           if ( !sRc )
           {
             TYPE( pTempRecord ) = chNodeType;   // data node
             NEXT( pTempRecord ) = RECORDNUM( pRecord );
             PREV( pRecord ) = RECORDNUM( pTempRecord );
             TYPE( pRecord ) = DATA_NEXTNODE;              // data node

             // fill in key data
             /*********************************************************/
             /* adjust sizes of relevant data                         */
             /*********************************************************/
             usLastPos = pRecord->contents.header.usLastFilled;
             {
               ULONG ulMax = (ULONG)(BTREE_REC_SIZE_V3 - pRecord->contents.header.usFilled );
               ulFitLen = min( ulDataLen, ulMax );
               ulFitLen -= (ULONG)(usLenFieldSize + sizeof(USHORT));
             }
             usLastPos = usLastPos - (USHORT)(ulFitLen + (ULONG)usLenFieldSize);
             pData = pRecord->contents.uchData + usLastPos;
             pRecData -= ulFitLen;
             ulDataLen -= ulFitLen;
             /*********************************************************/
             /* store length and copy data of part contained in record*/
             /*********************************************************/
             if ( pBT->usVersion >= NTM_VERSION2 )
             {
               *(PULONG) pData = ulFitLen;
             }
             else
             {
               *(PUSHORT) pData = (USHORT)ulFitLen;
             }
             memcpy( pData+usLenFieldSize, pRecData,  ulFitLen );
             // set address of key data at next free offset
             pusOffset = (PUSHORT) pRecord->contents.uchData;
             *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

             OCCUPIED(pRecord) += 1;
             pRecord->contents.header.usLastFilled = usLastPos;
             pRecord->contents.header.usFilled = pRecord->contents.header.usFilled +
                                           (USHORT)(ulFitLen + (ULONG)usLenFieldSize);
//           pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

             sRc = QDAMWriteRecord_V3(pBTIda, pRecord);
             BTREEUNLOCKRECORD( pRecord );

             /*********************************************************/
             /* toggle for adressing purposes                         */
             /* now: pRecord will be the free one for later filling   */
             /*********************************************************/
             pRecord = pTempRecord;
             BTREELOCKRECORD( pRecord );
           } /* endif */
         } /* endwhile */

         /*************************************************************/
         /* write out rest (this is the normal way in most cases      */
         /*************************************************************/
         if ( !sRc )
         {
           pHeader = &(pRecord->contents.header);
           pHeader->usParent = sizeof(BTREEHEADER );

           // store position where data will be stored
           recStart.usNum    = pHeader->usNum;

           // fill in key data
           usLastPos = pRecord->contents.header.usLastFilled;

           usLastPos = usLastPos - (USHORT)(ulDataLen + (ULONG)usLenFieldSize);
           pData = pRecord->contents.uchData + usLastPos;
           pRecData -= ulDataLen;

           // insert reference
           if ( pBT->usVersion >= NTM_VERSION2 )
           {
             *(PULONG) pData = ulFullDataLen;
           }
           else
           {
             *(PUSHORT) pData = (USHORT)ulFullDataLen;
           }
           memcpy( pData+usLenFieldSize, pRecData, ulDataLen );

           // set address of key data at next free offset
           pusOffset = (PUSHORT) pRecord->contents.uchData;
           *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

           OCCUPIED(pRecord) += 1;
           pRecord->contents.header.usLastFilled = usLastPos;
           pRecord->contents.header.usFilled = pRecord->contents.header.usFilled +
                                    (USHORT)(ulDataLen+usLenFieldSize+sizeof(USHORT));

           // mark if tersed or not
           if ( fTerse )
           {
             if ( pBT->usVersion >= NTM_VERSION2 )
             {
               *(PULONG) pData |= QDAM_TERSE_FLAGL;
             }
             else
             {
               *(PUSHORT) pData |= QDAM_TERSE_FLAG;
             }
           } /* endif */


           // write record
  //       pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
           sRc = QDAMWriteRecord_V3(pBTIda, pRecord);
           BTREEUNLOCKRECORD( pRecord );         // free the locking
           // update list entry
           if ( !sRc )
           {
              pRecTemp->usOffset = pHeader->usFilled;
              pRecTemp->usNum    = pHeader->usNum;
           } /* endif */
         } /* endif */
      } /* endif */
   } /* endif */

   //  return record parameters
   if ( !sRc )
   {
      precReturn->usNum = recStart.usNum - pBT->usFirstDataBuffer;
      precReturn->usOffset = OCCUPIED( pRecord ) - 1;
   }
   else
   {
      memset(&recStart, 0, sizeof(RECPARAM));
      precReturn = &recStart;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMADDTOBUFFER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDeleteDataFromBuffer    Delete Data From Buffer
//------------------------------------------------------------------------------
// Function call:     QDAMDeleteDataFromBuffer( PBTREE, RECPARAM );
//
//------------------------------------------------------------------------------
// Description:       Delete stored data (either key or data)
//
//                    Data are stored the following way
//                     USHORT      length of data
//                     USHORT      record number where associated key is stored
//                     USHORT      offset where key starts
//                     PCHAR       data following here in length of usDataLen
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    RECPARAM               position/offset of the key string
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
// Side effects:       if size of holes > MAXWASTESIZE reallocate the data
//
//------------------------------------------------------------------------------
// Function flow:     get record number to read
//                    read in requested record
//                    if okay then
//                      lock record, get offset and data length
//                      if data length > record size then
//                        get next record number
//                        as long as record number set and rc okay do
//                          read in record, set rc
//                        endwhile
//                      endif
//                      set waste size to record length
//                      mark it as deleted
//                      prepare new record
//                    endif
//                    if okay so far then
//                      write new rearranged record
//                    endif
//                    unlock record
//                    return return code
//------------------------------------------------------------------------------
SHORT QDAMDeleteDataFromBuffer_V2
(
  PBTREE  pBTIda,
  RECPARAM recParam
)
{
   SHORT sRc = 0;
   PUSHORT  pusOffset;                           // offset pointer
   SHORT   i;                                    // index
   PBTREEBUFFER_V2  pRecord;
   PBTREEBUFFER_V2  pNew;                           // temporary record
   PBTREEHEADER  pHeader;                        // pointer to header
   ULONG         ulDataLen;                      // length of data
   USHORT        usNum;                          // number of record
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // read in record
   recParam.usNum = recParam.usNum + pBT->usFirstDataBuffer;      // adjust for offset
   sRc = QDAMReadRecord_V2( pBTIda, recParam.usNum, &pRecord, FALSE  );
   if ( ! sRc )
   {
      BTREELOCKRECORD( pRecord );              // lock eecord.
      // set offset in index table to 0;
      pusOffset = (PUSHORT) pRecord->contents.uchData;

      ulDataLen = QDAMGetrecDataLen_V2( pBTIda, pRecord, recParam.usOffset );

      /****************************************************************/
      /* if more than 4k than free the next records                   */
      /* They have the following characteristic:                      */
      /*    they all contain nothing else than this record            */
      /*    they all are of type DATA_NEXTNODE                        */
      /*    they all are chained                                      */
      /****************************************************************/
      if ( ulDataLen >= BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER) )
      {
        usNum = NEXT( pRecord );
        while ( !sRc && usNum )
        {
          sRc = QDAMReadRecord_V2( pBTIda, usNum, &pNew, FALSE  );
          if ( !sRc )
          {
            if ( TYPE(pNew) != DATA_NEXTNODE )
            {
              sRc = BTREE_CORRUPTED;
              ERREVENT2( QDAMDELETEDATAFROMBUFFER_LOC, STATE_EVENT, sRc, DB_GROUP, NULL );
            }
            else
            {
              usNum = NEXT( pNew );
              sRc = QDAMFreeRecord_V2( pBTIda, pNew, DATAREC );
              ulDataLen -= (BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER) );
            } /* endif */
          } /* endif */
        } /* endwhile */
        /**************************************************************/
        /* remove the chaining information - they are not chained any */
        /* more                                                       */
        /**************************************************************/
        NEXT( pRecord ) = 0;

      } /* endif */
      pRecord->contents.header.usWasteSize = pRecord->contents.header.usWasteSize + (USHORT)
                           QDAMGetrecDataLen_V2( pBTIda, pRecord, recParam.usOffset );

      *(pusOffset + recParam.usOffset ) = 0;

      /******************************************************************/
      /* if nothing left in record free it - else adjust size ...KAT009 */
      /******************************************************************/
      if ( OCCUPIED(pRecord) == 1 )
      {
        sRc = QDAMFreeRecord_V2( pBTIda, pRecord, DATAREC );
      }
      else
      {
        /**************************************************************/
        /* do garbage collection only if not second part of a record..*/
        /* Optimal solution would be:                                 */
        /*  - do a garbage collection in any case                     */
        /*    but you have to adjust the length of the stored data    */
        /*    to take care of parts stored in other records ...       */
        /**************************************************************/
        if ( !NEXT(pRecord) && (pRecord->contents.header.usWasteSize > MAXWASTESIZE) )
        {
           // get temp record
           pNew = &pBT->BTreeTempBuffer_V2;
           pHeader = &pNew->contents.header;
           pHeader->usOccupied = 0;
           pHeader->usFilled = sizeof(BTREEHEADER );
           pHeader->usLastFilled = BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER );

           // for all data values move them into new record
           i = 0;
           while ( i < (SHORT) OCCUPIED( pRecord ))
           {
              QDAMCopyDataTo_V2( pRecord, i, pNew, i, pBT->usVersion );
              i++;
           } /* endwhile */
           // copy record data back to avoid misplacement in file
           memcpy( pRecord->contents.uchData, pNew->contents.uchData, FREE_SIZE_V2 );
           pRecord->contents.header.usFilled = pNew->contents.header.usFilled;
           pRecord->contents.header.usLastFilled = pNew->contents.header.usLastFilled;
           pRecord->contents.header.usWasteSize = 0;       // no space wasted
           QDAMUpdateList_V2( pBTIda, pRecord );
        } /* endif */

        if ( !sRc )
        {
//         pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
           sRc = QDAMWriteRecord_V2( pBTIda, pRecord );
        } /* endif */
      } /* endif */

      BTREEUNLOCKRECORD( pRecord );              // unlock previous record.
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMDELETEDATAFROMBUFFER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return ( sRc );

}

SHORT QDAMDeleteDataFromBuffer_V3
(
  PBTREE  pBTIda,
  RECPARAM recParam
)
{
   SHORT sRc = 0;
   PUSHORT  pusOffset;                           // offset pointer
   SHORT   i;                                    // index
   PBTREEBUFFER_V3  pRecord;
   PBTREEBUFFER_V3  pNew;                           // temporary record
   PBTREEHEADER  pHeader;                        // pointer to header
   ULONG         ulDataLen;                      // length of data
   USHORT        usNum;                          // number of record
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // read in record
   recParam.usNum = recParam.usNum + pBT->usFirstDataBuffer;      // adjust for offset
   sRc = QDAMReadRecord_V3( pBTIda, recParam.usNum, &pRecord, FALSE  );
   if ( ! sRc )
   {
      BTREELOCKRECORD( pRecord );              // lock eecord.
      // set offset in index table to 0;
      pusOffset = (PUSHORT) pRecord->contents.uchData;

      ulDataLen = QDAMGetrecDataLen_V3( pBTIda, pRecord, recParam.usOffset );

      /****************************************************************/
      /* if more than 4k than free the next records                   */
      /* They have the following characteristic:                      */
      /*    they all contain nothing else than this record            */
      /*    they all are of type DATA_NEXTNODE                        */
      /*    they all are chained                                      */
      /****************************************************************/
      if ( ulDataLen >= BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER) )
      {
        usNum = NEXT( pRecord );
        while ( !sRc && usNum )
        {
          sRc = QDAMReadRecord_V3( pBTIda, usNum, &pNew, FALSE  );
          if ( !sRc )
          {
            if ( TYPE(pNew) != DATA_NEXTNODE )
            {
              sRc = BTREE_CORRUPTED;
              ERREVENT2( QDAMDELETEDATAFROMBUFFER_LOC, STATE_EVENT, sRc, DB_GROUP, NULL );
            }
            else
            {
              usNum = NEXT( pNew );
              sRc = QDAMFreeRecord_V3( pBTIda, pNew, DATAREC );
              ulDataLen -= (BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER) );
            } /* endif */
          } /* endif */
        } /* endwhile */
        /**************************************************************/
        /* remove the chaining information - they are not chained any */
        /* more                                                       */
        /**************************************************************/
        NEXT( pRecord ) = 0;

      } /* endif */
      pRecord->contents.header.usWasteSize = pRecord->contents.header.usWasteSize + (USHORT)
                           QDAMGetrecDataLen_V3( pBTIda, pRecord, recParam.usOffset );

      *(pusOffset + recParam.usOffset ) = 0;

      /******************************************************************/
      /* if nothing left in record free it - else adjust size ...KAT009 */
      /******************************************************************/
      if ( OCCUPIED(pRecord) == 1 )
      {
        sRc = QDAMFreeRecord_V3( pBTIda, pRecord, DATAREC );
      }
      else
      {
        /**************************************************************/
        /* do garbage collection only if not second part of a record..*/
        /* Optimal solution would be:                                 */
        /*  - do a garbage collection in any case                     */
        /*    but you have to adjust the length of the stored data    */
        /*    to take care of parts stored in other records ...       */
        /**************************************************************/
        if ( !NEXT(pRecord) && (pRecord->contents.header.usWasteSize > MAXWASTESIZE) )
        {
           // get temp record
           pNew = &pBT->BTreeTempBuffer_V3;
           pHeader = &pNew->contents.header;
           pHeader->usOccupied = 0;
           pHeader->usFilled = sizeof(BTREEHEADER );
           pHeader->usLastFilled = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER );

           // for all data values move them into new record
           i = 0;
           while ( i < (SHORT) OCCUPIED( pRecord ))
           {
              QDAMCopyDataTo_V3( pRecord, i, pNew, i, pBT->usVersion );
              i++;
           } /* endwhile */
           // copy record data back to avoid misplacement in file
           memcpy( pRecord->contents.uchData, pNew->contents.uchData, FREE_SIZE_V3 );
           pRecord->contents.header.usFilled = pNew->contents.header.usFilled;
           pRecord->contents.header.usLastFilled = pNew->contents.header.usLastFilled;
           pRecord->contents.header.usWasteSize = 0;       // no space wasted
           QDAMUpdateList_V3( pBTIda, pRecord );
        } /* endif */

        if ( !sRc )
        {
//         pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
           sRc = QDAMWriteRecord_V3( pBTIda, pRecord );
        } /* endif */
      } /* endif */

      BTREEUNLOCKRECORD( pRecord );              // unlock previous record.
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMDELETEDATAFROMBUFFER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return ( sRc );

}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictUpdateLocal   Update Entry
//------------------------------------------------------------------------------
// Function call:     QDAMDictUpdateLocal( PBTREE, PCHAR, PCHAR, USHORT );
//
//------------------------------------------------------------------------------
// Description:       Update existing data
//
//                    Data are stored the following way:
//                     USHORT    length of data
//                     USHORT    record number where associated key is stored
//                     USHORT    offset where key starts
//                     PCHAR     data following here in length of usDataLen
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key string
//                    PCHAR                  user data
//                    USHORT                 user data length
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary corrupted
//                    BTREE_NOT_FOUND   data not found
//
//------------------------------------------------------------------------------
// Function flow:     Locate the Leaf node that contains the appropriate
//                      key
//                    find the key
//                    if found then
//                      get data and key value associated with key
//                      set new data value
//                      mark old value as deleted
//                    endif
//                    return sRc
//
//------------------------------------------------------------------------------
SHORT QDAMDictUpdateLocal
(
  PBTREE  pBTIda,                        //
  PCHAR_W pKey,                          //  key string
  PBYTE   pUserData,                     //  user data
  ULONG   ulLen                          //  user data length
)
{
   SHORT sRc = 0;                        // return code
   SHORT   i ;
   SHORT    sNearKey;                   // nearest key found
   RECPARAM      recData;               // pointer to data value
   RECPARAM      recOldData;            // pointer to old data value
   RECPARAM      recOldKey;             // pointer to old key value
   BOOL          fLocked = FALSE;       // file-has-been-locked flag
   PBTREEGLOB    pBT = NULL;            // pointer to btree

   DEBUGEVENT2( QDAMDICTUPDATELOCAL_LOC, FUNCENTRY_EVENT, 0, DB_GROUP, NULL );

   /*******************************************************************/
   /* validate passed pointer ...                                     */
   /*******************************************************************/
   CHECKPBTREE( pBTIda, sRc );

   if ( !sRc )
   {
     pBT = pBTIda->pBTree;
   } /* endif */

   if ( !sRc && pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */

   if ( !sRc && !pBT->fOpen )
   {
     sRc = BTREE_READONLY;
   } /* endif */

   /*******************************************************************/
   /* check if entry is locked ....                                   */
   /*******************************************************************/
   if ( !sRc && QDAMDictLockStatus( pBTIda, pKey ) )
   {
     sRc = BTREE_ENTRY_LOCKED;
   } /* endif */

   /*******************************************************************/
   /* For shared databases: lock complete file                        */
   /*                                                                 */
   /* Note: this will also update our internal buffers and the        */
   /*       header record. No need to call QDamCheckForUpdates here.  */
   /*******************************************************************/
   if ( !sRc && (pBT->usOpenFlags & ASD_SHARED) )
   {
     sRc = QDAMPhysLock( pBTIda, TRUE, &fLocked );
   } /* endif */

   if ( pBT->bRecSizeVersion == BTREE_V3 )
   {
      PBTREEBUFFER_V3  pRecord = NULL;        // pointer to record
      if ( !sRc )
      {
        if ( (ulLen == 0) || ((pBT->usVersion < NTM_VERSION2) && (ulLen >= MAXDATASIZE)) )
        {
          sRc = BTREE_DATA_RANGE;
        }
        else
        {
          if ( pBT->fTransMem )
          {
            memcpy( pBTIda->chHeadTerm, pKey, sizeof(ULONG));   // save data
          }
          else
          {
            UTF16strcpy( pBTIda->chHeadTerm, pKey );          // save current data
          } /* endif */
          QDAMDictUpdStatus( pBTIda );
          sRc = QDAMFindRecord_V3( pBTIda, pKey, &pRecord );
        } /* endif */
      } /* endif */

      // Locate the Leaf node that contains the appropriate key
      if ( !sRc )
      {
        //  find the key
        sRc = QDAMLocateKey_V3( pBTIda, pRecord, pKey, &i, FEXACT, &sNearKey ) ;
        if ( !sRc )
        {
            if ( i != -1)
            {
              BTREELOCKRECORD( pRecord );
              // set new current position
              pBTIda->sCurrentIndex = i;
              pBTIda->usCurrentRecord = RECORDNUM( pRecord );
              // get data value associated with key
              recOldKey = QDAMGetrecKey_V3( pRecord,i );
              recOldData = QDAMGetrecData_V3( pRecord, i, pBT->usVersion );
              //  set new data value
              if ( recOldKey.usNum && recOldData.usNum )
              {
                  sRc = QDAMAddToBuffer_V3( pBTIda, pUserData, ulLen, &recData );
                  if ( !sRc )
                  {
                    recData.ulLen = ulLen;
                    QDAMSetrecData_V3( pRecord, i, recData, pBT->usVersion );
                    sRc = QDAMWriteRecord_V3( pBTIda, pRecord );
                  } /* endif */
                /****************************************************************/
                /* change time to indicate modifications on dictionary...       */
                /****************************************************************/
                pBT->lTime ++;
              }
              else
              {
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMDICTUPDATELOCAL_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
              } /* endif */

              //  mark old value as deleted
              if ( !sRc )
              {
                  sRc = QDAMDeleteDataFromBuffer_V3( pBTIda, recOldData );
              } /* endif */
              BTREEUNLOCKRECORD( pRecord );
            }
            else
            {
              sRc = BTREE_NOT_FOUND;
              // set new current position
              pBTIda->sCurrentIndex = sNearKey;
              pBTIda->usCurrentRecord = RECORDNUM( pRecord );
            } /* endif */
        } /* endif */
    } /* endif */
   }
   else
   {
     PBTREEBUFFER_V2  pRecord = NULL;        // pointer to record
      if ( !sRc )
      {
        if ( (ulLen == 0) || ((pBT->usVersion < NTM_VERSION2) && (ulLen >= MAXDATASIZE)) )
        {
          sRc = BTREE_DATA_RANGE;
        }
        else
        {
          if ( pBT->fTransMem )
          {
            memcpy( pBTIda->chHeadTerm, pKey, sizeof(ULONG));   // save data
          }
          else
          {
            UTF16strcpy( pBTIda->chHeadTerm, pKey );          // save current data
          } /* endif */
          QDAMDictUpdStatus( pBTIda );
          sRc = QDAMFindRecord_V2( pBTIda, pKey, &pRecord );
        } /* endif */
      } /* endif */

      // Locate the Leaf node that contains the appropriate key
      if ( !sRc )
      {
        //  find the key
        sRc = QDAMLocateKey_V2( pBTIda, pRecord, pKey, &i, FEXACT, &sNearKey ) ;
        if ( !sRc )
        {
            if ( i != -1)
            {
              BTREELOCKRECORD( pRecord );
              // set new current position
              pBTIda->sCurrentIndex = i;
              pBTIda->usCurrentRecord = RECORDNUM( pRecord );
              // get data value associated with key
              recOldKey = QDAMGetrecKey_V2( pRecord,i );
              recOldData = QDAMGetrecData_V2( pRecord, i, pBT->usVersion );
              //  set new data value
              if ( recOldKey.usNum && recOldData.usNum )
              {
                  sRc = QDAMAddToBuffer_V2( pBTIda, pUserData, ulLen, &recData );
                  if ( !sRc )
                  {
                    recData.ulLen = ulLen;
                    QDAMSetrecData_V2( pRecord, i, recData, pBT->usVersion );
                    sRc = QDAMWriteRecord_V2( pBTIda, pRecord );
                  } /* endif */
                /****************************************************************/
                /* change time to indicate modifications on dictionary...       */
                /****************************************************************/
                pBT->lTime ++;
              }
              else
              {
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMDICTUPDATELOCAL_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
              } /* endif */

              //  mark old value as deleted
              if ( !sRc )
              {
                  sRc = QDAMDeleteDataFromBuffer_V2( pBTIda, recOldData );
              } /* endif */
              BTREEUNLOCKRECORD( pRecord );
            }
            else
            {
              sRc = BTREE_NOT_FOUND;
              // set new current position
              pBTIda->sCurrentIndex = sNearKey;
              pBTIda->usCurrentRecord = RECORDNUM( pRecord );
            } /* endif */
        } /* endif */
    } /* endif */
   } /* endif */


   /*******************************************************************/
   /* For shared databases: unlock complete file                      */
   /*                                                                 */
   /* Note: this will also incement the dictionary update counter     */
   /*       if the dictionary has been modified                       */
   /*******************************************************************/
   if ( fLocked )
   {
     SHORT sRc1 = QDAMPhysLock( pBTIda, FALSE, NULL );
     sRc = (sRc) ? sRc : sRc1;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMDICTUPDATELOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictCopyLocal    Copy Entries
//------------------------------------------------------------------------------
// Function call:     QDAMDictCopyLocal( PBTREE, PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Insert the current entry from the source dictionary
//                    into the target dictionary and point to the next
//                    entry in the source dictionary
//                    Source and Target have to be either local or remote
//------------------------------------------------------------------------------
// Parameters:        PBTREE       pointer to btree struct of source dict
//                    PBTREE       pointer to btree struct of target dict
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary corrupted
//                    BTREE_NOT_FOUND   invalid data
//                    BTREE_INVALID     invalid data pointer
//
//------------------------------------------------------------------------------
// Function flow:     if both tree pointers are given then
//                      store corruption flag temporarily
//                      if target is corrupted then
//                        set Rc to BTREE_CORRUPTED
//                      else
//                        reset corruption flag of source dictionary
//                        get current entry from source; set Rc appropriate
//                      endif
//                      if okay then
//                        insert dictionary entry into target dict; set Rc appr
//                      endif
//                    else
//                      set Rc to BTREE_INVALID
//                    endif
//                    return Rc;
//------------------------------------------------------------------------------
#ifdef UNUSED_CODE
SHORT QDAMDictCopyLocal
(
   PBTREE pBTSourceIda,                  // pointer of source tree structure
   PBTREE pBTTargetIda                   // pointer of target tree structure
)
{
   SHORT    sRc = 0;                  // return code
   ULONG    ulKeyLen = HEADTERM_SIZE; // key length
   ULONG    ulDataLen = MAXDATASIZE;  // data length
   PBTREEBUFFER pRecord;              // pointer to record buffer
   BOOL     fCorrupted;               // current corruption flag
   PBTREEGLOB    pBTSource = pBTSourceIda->pBTree;
   PBTREEGLOB    pBTTarget = pBTTargetIda->pBTree;

   if ( pBTSource && pBTTarget )
   {
      /*******************************************************************/
      /* For shared databases: discard all in-memory pages if database   */
      /* has been changed since last access                              */
      /*******************************************************************/
      if ( (pBTSource->usOpenFlags & ASD_SHARED) )
      {
        sRc = QDAMCheckForUpdates( pBTSourceIda );
      } /* endif */
      if ( !sRc && (pBTTarget->usOpenFlags & ASD_SHARED) )
      {
        sRc = QDAMCheckForUpdates( pBTTargetIda );
      } /* endif */

      fCorrupted = pBTSource->fCorrupted;
      if ( sRc )
      {
      }
      else if ( pBTTarget->fCorrupted )
      {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMDICTCOPYLOCAL_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
      }
      else
      {
         // reset corrupted flag for pBTSource for the moment
         //   otherwise we will have no chance in doing anything
         pBTSource->fCorrupted = FALSE;

         if ( !sRc )
         {
            // get entry from source
            sRc = QDAMDictCurrent( pBTSourceIda, (PBYTE)pBTSource->pTempKey, &ulKeyLen,
                                   pBTSource->pTempRecord, &ulDataLen);
         } /* endif */
      } /* endif */

      // if okay add it to target
      if ( !sRc )
      {
         sRc = QDAMDictInsertLocal( pBTTargetIda, pBTSource->pTempKey,
                               pBTSource->pTempRecord, ulDataLen );
      } /* endif */

      // increment position indicator
      if ( !sRc )
      {
         pBTSourceIda->sCurrentIndex++;               // point to next word
                                                      // and validate index
         sRc = QDAMReadRecord( pBTSourceIda,
                               pBTSourceIda->usCurrentRecord,&pRecord, FALSE );
         if ( !sRc )
         {
            sRc = QDAMValidateIndex( pBTSourceIda, &pRecord );
         } /* endif */
         pBTSource->fCorrupted = fCorrupted;          // restore corrupt flag
      } /* endif */
   }
   else
   {
      sRc = BTREE_INVALID;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMDICTCOPYLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return( sRc );
}
#endif

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMReduceNode  Reduce a Node
//------------------------------------------------------------------------------
// Function call:     QDAMReduceNode( PBTREE, PBTREEBUFFER );
//------------------------------------------------------------------------------
// Description:       Reduce a node so that it contains
//                    at least the minimum  number of keys
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PBTREEBUFFER           pointer to record
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary corrupted
//                    BTREE_NOT_FOUND   invalid data
//                    BTREE_INVALID     invalid data pointer
//
//------------------------------------------------------------------------------
// Function flow:     read in parent record
//                    if okay get the previous record and check if enough
//                      space left.
//                      if so then
//                         copy data and delete record from tree
//                      endif
//                    endif
//                    if okay and not shuffeled yet
//                      read in next record and check if enough space avail.
//                      if so then
//                        copy data and delete record from tree
//                      endif
//                    endif
//                    if okay and not yet shuffeled (both records are almost
//                      full) then
//                      take the prev record and take half of the keys and
//                       put them into the current one.
//                    endif
//                    return return code
//------------------------------------------------------------------------------
SHORT QDAMReduceNode_V2
(
   PBTREE pBTIda,                      // pointer to btree structure
   PBTREEBUFFER_V2 pRecord                // pointer to record
)
{
   SHORT  sRc = 0;                     // return code
   PBTREEBUFFER_V2  pParent;              // parent record
   PBTREEBUFFER_V2  pOther;               // next record
   USHORT        usOther;              // number of other key
   PUSHORT       pusOffset;            // pointer to offset table
   PCHAR_W       pKey = NULL;          // pointer to key
   PCHAR_W       pNewKey;              // pointer to new key
   BOOL          fEnoughSpace = FALSE; // enough space found
   SHORT         sKeyFound = 0;        // key found indicator
   SHORT         sNearKey;             // nearest key
   SHORT         i;                    // index
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // check if we will have enough space in the previous record (if exist)
   BTREELOCKRECORD( pRecord );

   // read in the parent
   sRc = QDAMReadRecord_V2( pBTIda, PARENT(pRecord), &pParent, FALSE  );

    if ( !sRc )
    {
       BTREELOCKRECORD(pParent);
       // allocate space for temporary keys
       UtlAlloc( (PVOID *)&pKey, 0l, (LONG) HEADTERM_SIZE, NOMSG );
       if ( !pKey)
       {
          sRc = BTREE_NO_ROOM;
       } /* endif */
    } /* endif */

    usOther = PREV( pRecord );        // get the previous

    if ( !sRc && usOther )
    {
       sRc = QDAMReadRecord_V2( pBTIda, usOther, &pOther, FALSE  );
       if ( !sRc && FILLEDUP(pRecord) + FILLEDUP(pOther) < BTREE_REC_SIZE_V2 )
       {
          fEnoughSpace = TRUE;
          // copy contents into previous node
          for ( i=0;i < (SHORT) OCCUPIED(pRecord) ; i++ )
          {
            QDAMCopyKeyTo_V2( pRecord, i, pOther, (SHORT)(i + OCCUPIED(pOther)), pBT->usVersion );
          } /* endfor */

          OCCUPIED( pOther ) = OCCUPIED( pOther ) + OCCUPIED( pRecord );
          NEXT( pOther) = NEXT( pRecord );
//        pOther->ulCheckSum = QDAMComputeCheckSum( pOther );

          sRc = QDAMWriteRecord_V2( pBTIda, pOther );

          // delete the node and the reference in the parent
          if ( !sRc )
          {
             pKey = QDAMGetszKey_V2( pRecord, 0, pBT->usVersion );
             if ( pKey )
             {
               sRc = QDAMLocateKey_V2( pBTIda, pParent, pKey, &sKeyFound, FEXACT, &sNearKey);
               BTREELOCKRECORD( pParent );
             }
             else
             {
               sRc = BTREE_CORRUPTED;
               ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 5, DB_GROUP, NULL );
             } /* endif */
          } /* endif */
          if ( !sRc )
          {
             if ( sKeyFound == -1)
             {
                sRc = BTREE_NOT_FOUND;
             }
             else
             {
                // delete the entry
                pusOffset = (PUSHORT) pParent->contents.uchData;
                *(pusOffset + sKeyFound) = 0;
                OCCUPIED(pParent) --;                       // one key removed
                QDAMReArrangeKRec_V2( pBTIda, pParent );
//              pParent->ulCheckSum = QDAMComputeCheckSum( pParent );
                sRc = QDAMWriteRecord_V2( pBTIda, pParent );
             } /* endif */
          } /* endif */

          if ( !sRc )
          {
             // free the record
             sRc = QDAMFreeRecord_V2( pBTIda, pRecord, KEYREC );
          } /* endif */
       } /* endif */
    } /* endif */

    if ( !sRc && !fEnoughSpace )
    {
       // if not yet shuffled check if in the following one (if exist)
       usOther = NEXT( pRecord );        // get the next record
       if ( usOther )
       {
          sRc = QDAMReadRecord_V2( pBTIda, usOther, &pOther, FALSE  );
          if ( !sRc && FILLEDUP(pRecord) + FILLEDUP(pOther) < BTREE_REC_SIZE_V2 )
          {
             fEnoughSpace = TRUE;
             // make space at the beginning of the node
             pusOffset = (PUSHORT) pOther->contents.uchData;
             memmove( (pusOffset+OCCUPIED(pRecord)), pusOffset, OCCUPIED(pOther) * sizeof(USHORT));

             // copy contents into next node
             for ( i=0; i < (SHORT) OCCUPIED(pRecord) ; i++ )
             {
               QDAMCopyKeyTo_V2( pRecord, i, pOther, i, pBT->usVersion );
             } /* endfor */
             OCCUPIED( pOther ) = OCCUPIED( pOther ) + OCCUPIED( pRecord );
             PREV( pOther ) = PREV( pRecord );
//           pOther->ulCheckSum = QDAMComputeCheckSum( pOther );
             sRc = QDAMWriteRecord_V2( pBTIda, pOther );

             // delete the node
             if ( !sRc )
             {
                // get the first string of this record
                pKey = QDAMGetszKey_V2( pRecord, 0, pBT->usVersion );
                if ( pKey )
                {
                  sRc = QDAMLocateKey_V2( pBTIda, pParent, pKey, &sKeyFound, FEXACT, &sNearKey);
                  BTREELOCKRECORD( pParent );
                }
                else
                {
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 10, DB_GROUP, NULL );
                } /* endif */
             } /* endif */
             if ( !sRc )
             {
                if ( sKeyFound == -1)
                {
                   sRc = BTREE_NOT_FOUND;
                }
                else
                {
                   // delete the entry
                   pusOffset = (PUSHORT) pParent->contents.uchData;
                   *(pusOffset + sKeyFound) = 0;
                   OCCUPIED(pParent) --;                       // one key removed
                   QDAMReArrangeKRec_V2( pBTIda, pParent );
//                 pParent->ulCheckSum = QDAMComputeCheckSum( pParent );
                   sRc = QDAMWriteRecord_V2( pBTIda, pParent );
                } /* endif */
             } /* endif */
             // change the key value in the root
             if ( !sRc )
             {
                pNewKey = QDAMGetszKey_V2( pOther, 0, pBT->usVersion );
                if ( pNewKey )
                {
                  sRc = QDAMChangeKey_V2( pBTIda, PARENT(pRecord), pKey, pNewKey );
                }
                else
                {
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 15, DB_GROUP, NULL );
                } /* endif */
                BTREELOCKRECORD( pParent );
             } /* endif */
             if ( !sRc )
             {
                // free the record
                sRc = QDAMFreeRecord_V2( pBTIda, pRecord, KEYREC );
             } /* endif */
          } /* endif */
       } /* endif */
    } /* endif */

    // if not yet shuffled
    if ( !sRc && !fEnoughSpace )
    {
       // if the next record exist, take half of the keys and put them
       // into the previous one, else split the previous one
       // Note: in such a case we have to change the key in the root, too.
       usOther = NEXT( pRecord );
       if ( usOther )
       {
          fEnoughSpace = TRUE;                   // found it
          sRc = QDAMReadRecord_V2( pBTIda, usOther, &pOther, FALSE  );
          if ( !sRc )
          {
             // move keys into record until both records are filled
             // up almost the same
             pKey = QDAMGetszKey_V2( pOther, 0, pBT->usVersion );
             if ( pKey )
             {
               while ( !sRc && FILLEDUP(pRecord) < FILLEDUP(pOther))
               {
                   QDAMCopyKeyTo_V2( pOther, 0, pRecord, OCCUPIED(pRecord), pBT->usVersion );
                   pusOffset = (PUSHORT) pOther->contents.uchData;
                   *pusOffset = 0 ;      // mark it as deleted
                   OCCUPIED(pOther) --;
                   QDAMReArrangeKRec_V2( pBTIda, pOther );
               } /* endwhile */
             }
             else
             {
               sRc = BTREE_CORRUPTED;
               ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 20, DB_GROUP, NULL );
             } /* endif */

             // write changed records back
             if ( !sRc )
             {
//              pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
                sRc = QDAMWriteRecord_V2( pBTIda, pRecord );
                if ( !sRc )
                {
//                 pOther->ulCheckSum = QDAMComputeCheckSum( pOther );
                   sRc = QDAMWriteRecord_V2( pBTIda, pOther );
                } /* endif */
             } /* endif */
             // change the key value in the root
             if ( !sRc )
             {
                pNewKey = QDAMGetszKey_V2( pOther, 0, pBT->usVersion );
                if ( pNewKey )
                {
                  sRc = QDAMChangeKey_V2( pBTIda, PARENT(pRecord), pKey, pNewKey );
                }
                else
                {
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 25, DB_GROUP, NULL );
                } /* endif */
             } /* endif */
          } /* endif */
       } /* endif */
    } /* endif */

    // if not yet shuffled
    if ( !sRc && !fEnoughSpace )
    {
       // if the next record exist, take half of the keys and get them
       // from the previous one,
       // Note: in such a case we have to change the key in the root, too.
       usOther = PREV( pRecord );
       if ( usOther )
       {
          fEnoughSpace = TRUE;                   // found it
          sRc = QDAMReadRecord_V2( pBTIda, usOther, &pOther, FALSE  );
             // move keys into record until both records are filled
             // up almost the same
          if ( !sRc )
          {
            pKey = QDAMGetszKey_V2( pOther, 0, pBT->usVersion );
            if ( pKey == NULL )
            {
              sRc = BTREE_CORRUPTED;
              ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 30, DB_GROUP, NULL );
            } /* endif */
          } /* endif */
          while ( !sRc && FILLEDUP(pRecord) < FILLEDUP(pOther))
          {
              // free space for first entry
              pusOffset = (PUSHORT) pRecord->contents.uchData;
              memmove( pusOffset+1, pusOffset,
                       sizeof(USHORT)*(OCCUPIED(pRecord)-1));
              QDAMCopyKeyTo_V2( pOther, (SHORT)(OCCUPIED(pOther)-1), pRecord, 0, pBT->usVersion );
              pusOffset = (PUSHORT) pOther->contents.uchData;
              *(pusOffset+OCCUPIED(pOther)-1) = 0 ;      // mark it as deleted
              OCCUPIED(pOther) --;
              QDAMReArrangeKRec_V2( pBTIda, pOther );
          } /* endwhile */
          // write changed records back
          if ( !sRc )
          {
//           pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
             sRc = QDAMWriteRecord_V2( pBTIda, pRecord );
             if ( !sRc )
             {
//              pOther->ulCheckSum = QDAMComputeCheckSum( pOther );
                sRc = QDAMWriteRecord_V2( pBTIda, pOther );
             } /* endif */
          } /* endif */
          // change the key value in the root
          if ( !sRc )
          {
             pNewKey = QDAMGetszKey_V2( pRecord, 0, pBT->usVersion );
             if ( pKey == NULL )
             {
               sRc = BTREE_CORRUPTED;
               ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 35, DB_GROUP, NULL );
             }
             else
             {
               sRc = QDAMChangeKey_V2( pBTIda, PARENT(pRecord), pKey, pNewKey );
             } /* endif */
          } /* endif */
       } /* endif */
    } /* endif */


    if ( pParent )
    {
       BTREEUNLOCKRECORD( pParent );
    } /* endif */
    if ( pRecord )
    {
       BTREEUNLOCKRECORD( pRecord );
    } /* endif */

    // free allocated memory
    UtlAlloc( (PVOID *)&pKey, 0l, (LONG) HEADTERM_SIZE, NOMSG );

   if ( sRc )
   {
     ERREVENT2( QDAMREDUCENODE_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */


    return sRc;
}

SHORT QDAMReduceNode_V3
(
   PBTREE pBTIda,                      // pointer to btree structure
   PBTREEBUFFER_V3 pRecord                // pointer to record
)
{
   SHORT  sRc = 0;                     // return code
   PBTREEBUFFER_V3  pParent;              // parent record
   PBTREEBUFFER_V3  pOther;               // next record
   USHORT        usOther;              // number of other key
   PUSHORT       pusOffset;            // pointer to offset table
   PCHAR_W       pKey = NULL;          // pointer to key
   PCHAR_W       pNewKey;              // pointer to new key
   BOOL          fEnoughSpace = FALSE; // enough space found
   SHORT         sKeyFound = 0;        // key found indicator
   SHORT         sNearKey;             // nearest key
   SHORT         i;                    // index
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // check if we will have enough space in the previous record (if exist)
   BTREELOCKRECORD( pRecord );

   // read in the parent
   sRc = QDAMReadRecord_V3( pBTIda, PARENT(pRecord), &pParent, FALSE  );

    if ( !sRc )
    {
       BTREELOCKRECORD(pParent);
       // allocate space for temporary keys
       UtlAlloc( (PVOID *)&pKey, 0l, (LONG) HEADTERM_SIZE, NOMSG );
       if ( !pKey)
       {
          sRc = BTREE_NO_ROOM;
       } /* endif */
    } /* endif */

    usOther = PREV( pRecord );        // get the previous

    if ( !sRc && usOther )
    {
       sRc = QDAMReadRecord_V3( pBTIda, usOther, &pOther, FALSE  );
       if ( !sRc && FILLEDUP(pRecord) + FILLEDUP(pOther) < BTREE_REC_SIZE_V3 )
       {
          fEnoughSpace = TRUE;
          // copy contents into previous node
          for ( i=0;i < (SHORT) OCCUPIED(pRecord) ; i++ )
          {
            QDAMCopyKeyTo_V3( pRecord, i, pOther, (SHORT)(i + OCCUPIED(pOther)), pBT->usVersion );
          } /* endfor */

          OCCUPIED( pOther ) = OCCUPIED( pOther ) + OCCUPIED( pRecord );
          NEXT( pOther) = NEXT( pRecord );
//        pOther->ulCheckSum = QDAMComputeCheckSum( pOther );

          sRc = QDAMWriteRecord_V3( pBTIda, pOther );

          // delete the node and the reference in the parent
          if ( !sRc )
          {
             pKey = QDAMGetszKey_V3( pRecord, 0, pBT->usVersion );
             if ( pKey )
             {
               sRc = QDAMLocateKey_V3( pBTIda, pParent, pKey, &sKeyFound, FEXACT, &sNearKey);
               BTREELOCKRECORD( pParent );
             }
             else
             {
               sRc = BTREE_CORRUPTED;
               ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 5, DB_GROUP, NULL );
             } /* endif */
          } /* endif */
          if ( !sRc )
          {
             if ( sKeyFound == -1)
             {
                sRc = BTREE_NOT_FOUND;
             }
             else
             {
                // delete the entry
                pusOffset = (PUSHORT) pParent->contents.uchData;
                *(pusOffset + sKeyFound) = 0;
                OCCUPIED(pParent) --;                       // one key removed
                QDAMReArrangeKRec_V3( pBTIda, pParent );
//              pParent->ulCheckSum = QDAMComputeCheckSum( pParent );
                sRc = QDAMWriteRecord_V3( pBTIda, pParent );
             } /* endif */
          } /* endif */

          if ( !sRc )
          {
             // free the record
             sRc = QDAMFreeRecord_V3( pBTIda, pRecord, KEYREC );
          } /* endif */
       } /* endif */
    } /* endif */

    if ( !sRc && !fEnoughSpace )
    {
       // if not yet shuffled check if in the following one (if exist)
       usOther = NEXT( pRecord );        // get the next record
       if ( usOther )
       {
          sRc = QDAMReadRecord_V3( pBTIda, usOther, &pOther, FALSE  );
          if ( !sRc && FILLEDUP(pRecord) + FILLEDUP(pOther) < BTREE_REC_SIZE_V3 )
          {
             fEnoughSpace = TRUE;
             // make space at the beginning of the node
             pusOffset = (PUSHORT) pOther->contents.uchData;
             memmove( (pusOffset+OCCUPIED(pRecord)), pusOffset, OCCUPIED(pOther) * sizeof(USHORT));

             // copy contents into next node
             for ( i=0; i < (SHORT) OCCUPIED(pRecord) ; i++ )
             {
               QDAMCopyKeyTo_V3( pRecord, i, pOther, i, pBT->usVersion );
             } /* endfor */
             OCCUPIED( pOther ) = OCCUPIED( pOther ) + OCCUPIED( pRecord );
             PREV( pOther ) = PREV( pRecord );
//           pOther->ulCheckSum = QDAMComputeCheckSum( pOther );
             sRc = QDAMWriteRecord_V3( pBTIda, pOther );

             // delete the node
             if ( !sRc )
             {
                // get the first string of this record
                pKey = QDAMGetszKey_V3( pRecord, 0, pBT->usVersion );
                if ( pKey )
                {
                  sRc = QDAMLocateKey_V3( pBTIda, pParent, pKey, &sKeyFound, FEXACT, &sNearKey);
                  BTREELOCKRECORD( pParent );
                }
                else
                {
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 10, DB_GROUP, NULL );
                } /* endif */
             } /* endif */
             if ( !sRc )
             {
                if ( sKeyFound == -1)
                {
                   sRc = BTREE_NOT_FOUND;
                }
                else
                {
                   // delete the entry
                   pusOffset = (PUSHORT) pParent->contents.uchData;
                   *(pusOffset + sKeyFound) = 0;
                   OCCUPIED(pParent) --;                       // one key removed
                   QDAMReArrangeKRec_V3( pBTIda, pParent );
//                 pParent->ulCheckSum = QDAMComputeCheckSum( pParent );
                   sRc = QDAMWriteRecord_V3( pBTIda, pParent );
                } /* endif */
             } /* endif */
             // change the key value in the root
             if ( !sRc )
             {
                pNewKey = QDAMGetszKey_V3( pOther, 0, pBT->usVersion );
                if ( pNewKey )
                {
                  sRc = QDAMChangeKey_V3( pBTIda, PARENT(pRecord), pKey, pNewKey );
                }
                else
                {
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 15, DB_GROUP, NULL );
                } /* endif */
                BTREELOCKRECORD( pParent );
             } /* endif */
             if ( !sRc )
             {
                // free the record
                sRc = QDAMFreeRecord_V3( pBTIda, pRecord, KEYREC );
             } /* endif */
          } /* endif */
       } /* endif */
    } /* endif */

    // if not yet shuffled
    if ( !sRc && !fEnoughSpace )
    {
       // if the next record exist, take half of the keys and put them
       // into the previous one, else split the previous one
       // Note: in such a case we have to change the key in the root, too.
       usOther = NEXT( pRecord );
       if ( usOther )
       {
          fEnoughSpace = TRUE;                   // found it
          sRc = QDAMReadRecord_V3( pBTIda, usOther, &pOther, FALSE  );
          if ( !sRc )
          {
             // move keys into record until both records are filled
             // up almost the same
             pKey = QDAMGetszKey_V3( pOther, 0, pBT->usVersion );
             if ( pKey )
             {
               while ( !sRc && FILLEDUP(pRecord) < FILLEDUP(pOther))
               {
                   QDAMCopyKeyTo_V3( pOther, 0, pRecord, OCCUPIED(pRecord), pBT->usVersion );
                   pusOffset = (PUSHORT) pOther->contents.uchData;
                   *pusOffset = 0 ;      // mark it as deleted
                   OCCUPIED(pOther) --;
                   QDAMReArrangeKRec_V3( pBTIda, pOther );
               } /* endwhile */
             }
             else
             {
               sRc = BTREE_CORRUPTED;
               ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 20, DB_GROUP, NULL );
             } /* endif */

             // write changed records back
             if ( !sRc )
             {
//              pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
                sRc = QDAMWriteRecord_V3( pBTIda, pRecord );
                if ( !sRc )
                {
//                 pOther->ulCheckSum = QDAMComputeCheckSum( pOther );
                   sRc = QDAMWriteRecord_V3( pBTIda, pOther );
                } /* endif */
             } /* endif */
             // change the key value in the root
             if ( !sRc )
             {
                pNewKey = QDAMGetszKey_V3( pOther, 0, pBT->usVersion );
                if ( pNewKey )
                {
                  sRc = QDAMChangeKey_V3( pBTIda, PARENT(pRecord), pKey, pNewKey );
                }
                else
                {
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 25, DB_GROUP, NULL );
                } /* endif */
             } /* endif */
          } /* endif */
       } /* endif */
    } /* endif */

    // if not yet shuffled
    if ( !sRc && !fEnoughSpace )
    {
       // if the next record exist, take half of the keys and get them
       // from the previous one,
       // Note: in such a case we have to change the key in the root, too.
       usOther = PREV( pRecord );
       if ( usOther )
       {
          fEnoughSpace = TRUE;                   // found it
          sRc = QDAMReadRecord_V3( pBTIda, usOther, &pOther, FALSE  );
             // move keys into record until both records are filled
             // up almost the same
          if ( !sRc )
          {
            pKey = QDAMGetszKey_V3( pOther, 0, pBT->usVersion );
            if ( pKey == NULL )
            {
              sRc = BTREE_CORRUPTED;
              ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 30, DB_GROUP, NULL );
            } /* endif */
          } /* endif */
          while ( !sRc && FILLEDUP(pRecord) < FILLEDUP(pOther))
          {
              // free space for first entry
              pusOffset = (PUSHORT) pRecord->contents.uchData;
              memmove( pusOffset+1, pusOffset,
                       sizeof(USHORT)*(OCCUPIED(pRecord)-1));
              QDAMCopyKeyTo_V3( pOther, (SHORT)(OCCUPIED(pOther)-1), pRecord, 0, pBT->usVersion );
              pusOffset = (PUSHORT) pOther->contents.uchData;
              *(pusOffset+OCCUPIED(pOther)-1) = 0 ;      // mark it as deleted
              OCCUPIED(pOther) --;
              QDAMReArrangeKRec_V3( pBTIda, pOther );
          } /* endwhile */
          // write changed records back
          if ( !sRc )
          {
//           pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
             sRc = QDAMWriteRecord_V3( pBTIda, pRecord );
             if ( !sRc )
             {
//              pOther->ulCheckSum = QDAMComputeCheckSum( pOther );
                sRc = QDAMWriteRecord_V3( pBTIda, pOther );
             } /* endif */
          } /* endif */
          // change the key value in the root
          if ( !sRc )
          {
             pNewKey = QDAMGetszKey_V3( pRecord, 0, pBT->usVersion );
             if ( pKey == NULL )
             {
               sRc = BTREE_CORRUPTED;
               ERREVENT2( QDAMREDUCENODE_LOC, STATE_EVENT, 35, DB_GROUP, NULL );
             }
             else
             {
               sRc = QDAMChangeKey_V3( pBTIda, PARENT(pRecord), pKey, pNewKey );
             } /* endif */
          } /* endif */
       } /* endif */
    } /* endif */


    if ( pParent )
    {
       BTREEUNLOCKRECORD( pParent );
    } /* endif */
    if ( pRecord )
    {
       BTREEUNLOCKRECORD( pRecord );
    } /* endif */

    // free allocated memory
    UtlAlloc( (PVOID *)&pKey, 0l, (LONG) HEADTERM_SIZE, NOMSG );

   if ( sRc )
   {
     ERREVENT2( QDAMREDUCENODE_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */


    return sRc;
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictDeleteLocal    Delete an Entry
//------------------------------------------------------------------------------
// Function call:     QDAMDictDeleteLocal( PBTREE, PCHAR );
//
//------------------------------------------------------------------------------
// Description:       Remove a key and all of its associated data from the
//                    file.
//                    If the number of keys in a record drops below a
//                    specified mark, keys will be shuffled with nearby
//                    nodes to regain a balanced status.
//                    The shuffling may cause a node to be deleted, too.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to character for the key
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary corrupted
//                    BTREE_NOT_FOUND   invalid data
//                    BTREE_INVALID     invalid data pointer
//
//------------------------------------------------------------------------------
// Function flow:     if dictionary corrupted then
//                      set Rc to BTREE_CORRUPTED
//                    else
//                      get the node where the key will be; set Rc
//                    endif
//                    if okay then
//                      reduce note if necessary and set Rc
//                      if okay then
//                        reload record again (might be moved out)
//                      endif
//                    endif
//                    if okay then
//                      locate the key and set Rc
//                      if okay then
//                        if key not found then
//                          set Rc to BTREE_NOT_FOUND;
//                          position at the nearest key
//                        else
//                          position to key found
//                          get key data and user data
//                          copy rest of key data
//                          remove one key,rearrange key record
//                          write record
//                          if parent must be changed too
//                            change key in parent node
//                          endif
//                          if ok
//                            delete data from buffer
//                          endif
//                        endif
//                      endif
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDictDeleteLocal
(
   PBTREE pBTIda,                   // pointer to generic structure
   PCHAR_W  pKey                    // pointer to key
)
{
   SHORT  sRc = 0;                     // return code
   SHORT  sKeyFound;                   // number where key is detected
   RECPARAM  recData;                  // description for data
   RECPARAM  recKey;                   // description for key record
   SHORT     i;                        // index
   SHORT     sNearKey;                 // nearest key
   PUSHORT   pusOffset;
   BOOL          fLocked = FALSE;       // file-has-been-locked flag
   PBTREEGLOB    pBT = pBTIda->pBTree;

   DEBUGEVENT2( QDAMDICTDELETELOCAL_LOC, FUNCENTRY_EVENT, 0, DB_GROUP, NULL );

   if ( pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */
   if ( !sRc && !pBT->fOpen )
   {
     sRc = BTREE_READONLY;
   } /* endif */

   /*******************************************************************/
   /* check if entry is locked ....                                   */
   /*******************************************************************/
   if ( !sRc && QDAMDictLockStatus( pBTIda, pKey ) )
   {
     sRc = BTREE_ENTRY_LOCKED;
   } /* endif */

   /*******************************************************************/
   /* For shared databases: lock complete file                        */
   /*                                                                 */
   /* Note: this will also update our internal buffers and the        */
   /*       header record. No need to call QDamCheckForUpdates here.  */
   /*******************************************************************/
   if ( !sRc && (pBT->usOpenFlags & ASD_SHARED) )
   {
     sRc = QDAMPhysLock( pBTIda, TRUE, &fLocked );
   } /* endif */

   if ( pBT->bRecSizeVersion == BTREE_V3 )
   {
      PBTREEBUFFER_V3  pRecord = NULL;       // pointer to keyboard
      if ( !sRc )
      {
        /*****************************************************************/
        /* update headterm and invalidate current records for all ...    */
        /*****************************************************************/
        if ( pBT->fTransMem )
        {
          memcpy( pBTIda->chHeadTerm, pKey,sizeof(ULONG) );  // save current data
        }
        else
        {
          UTF16strcpy( pBTIda->chHeadTerm, pKey );          // save current data
        } /* endif */
        QDAMDictUpdStatus ( pBTIda );
        // get the node where the key might be
        sRc = QDAMFindRecord_V3( pBTIda, pKey, &pRecord );
      } /* endif */

      // check if we have to reduce one node
      if ( !sRc && FILLEDUP( pRecord ) < HEADTERM_SIZE )
      {
          sRc = QDAMReduceNode_V3( pBTIda, pRecord );
          if (!sRc )
          {
            // reload the record again - might be deleted in QDAMReduceNode
            sRc = QDAMFindRecord_V3( pBTIda, pKey, &pRecord );
          } /* endif */
      } /* endif */


      // locate the key
      if ( !sRc )
      {
        sRc = QDAMLocateKey_V3( pBTIda, pRecord, pKey, &sKeyFound, FEXACT, &sNearKey);
        if ( !sRc )
        {
            BTREELOCKRECORD( pRecord );
            if ( sKeyFound == -1)
            {
              sRc = BTREE_NOT_FOUND;
              // set new current position
              pBTIda->sCurrentIndex = sNearKey;
              pBTIda->usCurrentRecord = RECORDNUM( pRecord );
            }
            else
            {
              // set new current position
              pBTIda->sCurrentIndex = sKeyFound;
              pBTIda->usCurrentRecord = RECORDNUM( pRecord );
              // delete the key data and the user data
              recData = QDAMGetrecData_V3( pRecord, sKeyFound, pBT->usVersion );
              recKey  = QDAMGetrecKey_V3( pRecord, sKeyFound );
              // copy rest of the key data
              i = (SHORT) OCCUPIED(pRecord);

              pusOffset = (PUSHORT) pRecord->contents.uchData;
              *(pusOffset+sKeyFound) = 0;
              OCCUPIED(pRecord) --;                  // one key removed
              QDAMReArrangeKRec_V3( pBTIda, pRecord );
    //        pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

              //  delete the key data
              sRc = QDAMWriteRecord_V3( pBTIda, pRecord );

              //  check if we must change the parent, too
              if  ( !sRc && sKeyFound == 0  && !IS_ROOT(pRecord) )
              {
                // there must be still one entry in the record due to our
                // construction - as done in QDAMReduceNode
                PCHAR_W  pTempKey = QDAMGetszKey_V3(pRecord, 0, pBT->usVersion);
                if ( pTempKey )
                {
                  sRc = QDAMChangeKey_V3( pBTIda, PARENT(pRecord), pKey, pTempKey );
                }
                else
                {
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMDICTDELETELOCAL_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
                } /* endif */
              } /* endif */


              // delete the user data
              if ( !sRc )
              {
                sRc = QDAMDeleteDataFromBuffer_V3( pBTIda, recData );
              } /* endif */
            } /* endif */
            BTREEUNLOCKRECORD( pRecord );
        } /* endif */
        /*****************************************************************/
        /* update update time                                            */
        /*****************************************************************/
        if ( !sRc )
        {
          /****************************************************************/
          /* change time to indicate modifications on dictionary...       */
          /****************************************************************/
          pBT->lTime ++;
        } /* endif */
      } /* endif */
   }
   else
   {
      PBTREEBUFFER_V2  pRecord = NULL;       // pointer to keyboard
      if ( !sRc )
      {
        /*****************************************************************/
        /* update headterm and invalidate current records for all ...    */
        /*****************************************************************/
        if ( pBT->fTransMem )
        {
          memcpy( pBTIda->chHeadTerm, pKey,sizeof(ULONG) );  // save current data
        }
        else
        {
          UTF16strcpy( pBTIda->chHeadTerm, pKey );          // save current data
        } /* endif */
        QDAMDictUpdStatus ( pBTIda );
        // get the node where the key might be
        sRc = QDAMFindRecord_V2( pBTIda, pKey, &pRecord );
      } /* endif */

      // check if we have to reduce one node
      if ( !sRc && FILLEDUP( pRecord ) < HEADTERM_SIZE )
      {
          sRc = QDAMReduceNode_V2( pBTIda, pRecord );
          if (!sRc )
          {
            // reload the record again - might be deleted in QDAMReduceNode
            sRc = QDAMFindRecord_V2( pBTIda, pKey, &pRecord );
          } /* endif */
      } /* endif */


      // locate the key
      if ( !sRc )
      {
        sRc = QDAMLocateKey_V2( pBTIda, pRecord, pKey, &sKeyFound, FEXACT, &sNearKey);
        if ( !sRc )
        {
            BTREELOCKRECORD( pRecord );
            if ( sKeyFound == -1)
            {
              sRc = BTREE_NOT_FOUND;
              // set new current position
              pBTIda->sCurrentIndex = sNearKey;
              pBTIda->usCurrentRecord = RECORDNUM( pRecord );
            }
            else
            {
              // set new current position
              pBTIda->sCurrentIndex = sKeyFound;
              pBTIda->usCurrentRecord = RECORDNUM( pRecord );
              // delete the key data and the user data
              recData = QDAMGetrecData_V2( pRecord, sKeyFound, pBT->usVersion );
              recKey  = QDAMGetrecKey_V2( pRecord, sKeyFound );
              // copy rest of the key data
              i = (SHORT) OCCUPIED(pRecord);

              pusOffset = (PUSHORT) pRecord->contents.uchData;
              *(pusOffset+sKeyFound) = 0;
              OCCUPIED(pRecord) --;                  // one key removed
              QDAMReArrangeKRec_V2( pBTIda, pRecord );
    //        pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

              //  delete the key data
              sRc = QDAMWriteRecord_V2( pBTIda, pRecord );

              //  check if we must change the parent, too
              if  ( !sRc && sKeyFound == 0  && !IS_ROOT(pRecord) )
              {
                // there must be still one entry in the record due to our
                // construction - as done in QDAMReduceNode
                PCHAR_W  pTempKey = QDAMGetszKey_V2(pRecord, 0, pBT->usVersion);
                if ( pTempKey )
                {
                  sRc = QDAMChangeKey_V2( pBTIda, PARENT(pRecord), pKey, pTempKey );
                }
                else
                {
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMDICTDELETELOCAL_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
                } /* endif */
              } /* endif */


              // delete the user data
              if ( !sRc )
              {
                sRc = QDAMDeleteDataFromBuffer_V2( pBTIda, recData );
              } /* endif */
            } /* endif */
            BTREEUNLOCKRECORD( pRecord );
        } /* endif */
        /*****************************************************************/
        /* update update time                                            */
        /*****************************************************************/
        if ( !sRc )
        {
          /****************************************************************/
          /* change time to indicate modifications on dictionary...       */
          /****************************************************************/
          pBT->lTime ++;
        } /* endif */
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* For shared databases: unlock complete file                      */
   /*                                                                 */
   /* Note: this will also incement the dictionary update counter     */
   /*       if the dictionary has been modified                       */
   /*******************************************************************/
   if ( fLocked )
   {
     sRc = QDAMPhysLock( pBTIda, FALSE, NULL );
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMDICTDELETELOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */


   return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMCopyKeyTo     Copy Key into new record
//------------------------------------------------------------------------------
// Function call:     QDAMCopyKeyTo( PBTREEBUFFER, SHORT, PBTREEBUFFER, SHORT);
//
//------------------------------------------------------------------------------
// Description:       Copy specified key to new record
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer from where to copy
//                    SHORT                  key to be used
//                    PBTREEBUFFER           record pointer to copy into
//                    SHORT                  key to be used
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     copy the key at the passed offset to the new location
//                    return
//------------------------------------------------------------------------------
VOID QDAMCopyKeyTo_V2
(
   PBTREEBUFFER_V2  pRecord,
   SHORT         i,
   PBTREEBUFFER_V2  pNew,
   SHORT         j,
   USHORT        usVersion             // version of database
)
{
   PUSHORT  pusOldOffset;              // offset of data
   PUSHORT  pusNewOffset;              // new data
   USHORT   usLen;                     // length of data
   USHORT   usLastPos;                 // last position filled
   USHORT   usDataOffs;                // data offset
   PUCHAR   pOldData;                  // pointer to data

   pusOldOffset = (PUSHORT) pRecord->contents.uchData;
   pusNewOffset = (PUSHORT) pNew->contents.uchData;
   usLastPos = pNew->contents.header.usLastFilled;

   usDataOffs = *(pusOldOffset+i);
   if ( usDataOffs)
   {
      pOldData = pRecord->contents.uchData + usDataOffs;
      usLen = *(PUSHORT) pOldData + sizeof(USHORT) + sizeof(RECPARAM);
      if ( usVersion >= NTM_VERSION2 )
      {
        usLen = *(PUSHORT) pOldData + sizeof(USHORT) + sizeof(RECPARAM);
      }
      else
      {
        usLen = *(PUSHORT) pOldData + sizeof(USHORT) + sizeof(RECPARAMOLD);
      } /* endif */

      usLastPos = usLastPos - usLen;
      memcpy( pNew->contents.uchData+usLastPos, pOldData, usLen );
      *(pusNewOffset+j) = usLastPos;
      pNew->contents.header.usFilled += (usLen + sizeof(USHORT));
      pNew->contents.header.usLastFilled = usLastPos;
   }
   else
   {
      *(pusNewOffset+j) = 0;
      pNew->contents.header.usFilled += sizeof(USHORT);
   } /* endif */

   return;
}

VOID QDAMCopyKeyTo_V3
(
   PBTREEBUFFER_V3  pRecord,
   SHORT         i,
   PBTREEBUFFER_V3  pNew,
   SHORT         j,
   USHORT        usVersion             // version of database
)
{
   PUSHORT  pusOldOffset;              // offset of data
   PUSHORT  pusNewOffset;              // new data
   USHORT   usLen;                     // length of data
   USHORT   usLastPos;                 // last position filled
   USHORT   usDataOffs;                // data offset
   PUCHAR   pOldData;                  // pointer to data

   pusOldOffset = (PUSHORT) pRecord->contents.uchData;
   pusNewOffset = (PUSHORT) pNew->contents.uchData;
   usLastPos = pNew->contents.header.usLastFilled;

   usDataOffs = *(pusOldOffset+i);
   if ( usDataOffs)
   {
      pOldData = pRecord->contents.uchData + usDataOffs;
      usLen = *(PUSHORT) pOldData + sizeof(USHORT) + sizeof(RECPARAM);
      if ( usVersion >= NTM_VERSION2 )
      {
        usLen = *(PUSHORT) pOldData + sizeof(USHORT) + sizeof(RECPARAM);
      }
      else
      {
        usLen = *(PUSHORT) pOldData + sizeof(USHORT) + sizeof(RECPARAMOLD);
      } /* endif */

      usLastPos = usLastPos - usLen;
      memcpy( pNew->contents.uchData+usLastPos, pOldData, usLen );
      *(pusNewOffset+j) = usLastPos;
      pNew->contents.header.usFilled += (usLen + sizeof(USHORT));
      pNew->contents.header.usLastFilled = usLastPos;
   }
   else
   {
      *(pusNewOffset+j) = 0;
      pNew->contents.header.usFilled += sizeof(USHORT);
   } /* endif */

   return;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMCopyDataTo   Copy Data into new record
//------------------------------------------------------------------------------
// Function call:     QDAMCopyDataTo( PBTREEBUFFER, SHORT, PBTREEBUFFER,SHORT);
//
//------------------------------------------------------------------------------
// Description:       Copy data to new record for reorganizing
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer from where to copy
//                    SHORT                  data to be used
//                    PBTREEBUFFER           record pointer to copy into
//                    SHORT                  data to be used
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     copy the data from the passed position to the new one
//                    return
//
//------------------------------------------------------------------------------
VOID QDAMCopyDataTo_V3
(
   PBTREEBUFFER_V3  pRecord,
   SHORT         i,
   PBTREEBUFFER_V3  pNew,
   SHORT         j,
   USHORT        usVersion             // version of BTREE
)
{
   PUSHORT  pusOldOffset;              // offset of data
   PUSHORT  pusNewOffset;              // new data
   USHORT   usLen;                     // length of data
   USHORT   usLastPos;                 // last position filled
   USHORT   usDataOffs;                // data offset
   PUCHAR   pOldData;                  // pointer to data
   USHORT   usLenFieldSize;            // size of data length field

   if ( usVersion >= NTM_VERSION2 )
   {
     usLenFieldSize = sizeof(ULONG);
   }
   else
   {
     usLenFieldSize = sizeof(USHORT);
   } /* endif */

   pusOldOffset = (PUSHORT) pRecord->contents.uchData;
   pusNewOffset = (PUSHORT) pNew->contents.uchData;
   usLastPos = pNew->contents.header.usLastFilled;

   usDataOffs = *(pusOldOffset+i);
   if ( usDataOffs)
   {
      pOldData = pRecord->contents.uchData + usDataOffs;
      if ( usVersion >= NTM_VERSION2 )
      {
        ULONG ulLen = *(PULONG) pOldData;
        if ( ulLen & QDAM_TERSE_FLAGL)
        {
          ulLen &= ~QDAM_TERSE_FLAGL;
        } /* endif */
        usLen = (USHORT)ulLen;
      }
      else
      {
        usLen = *(PUSHORT) pOldData;
        if ( usLen & QDAM_TERSE_FLAG)
        {
          usLen &= ~QDAM_TERSE_FLAG;
        } /* endif */
      }
      usLen = usLen + usLenFieldSize;       // add size of length indication

      assert( (usLastPos >= usLen) );

      usLastPos = usLastPos - usLen;
      memcpy( pNew->contents.uchData+usLastPos, pOldData, usLen );
      *(pusNewOffset+j) = usLastPos;
      pNew->contents.header.usFilled += (usLen + sizeof(USHORT));
      pNew->contents.header.usLastFilled = usLastPos;
   }
   else
   {
      *(pusNewOffset+j) = 0;
      pNew->contents.header.usFilled += sizeof(USHORT);
   } /* endif */

   return;
}

VOID QDAMCopyDataTo_V2
(
   PBTREEBUFFER_V2  pRecord,
   SHORT         i,
   PBTREEBUFFER_V2  pNew,
   SHORT         j,
   USHORT        usVersion             // version of BTREE
)
{
   PUSHORT  pusOldOffset;              // offset of data
   PUSHORT  pusNewOffset;              // new data
   USHORT   usLen;                     // length of data
   USHORT   usLastPos;                 // last position filled
   USHORT   usDataOffs;                // data offset
   PUCHAR   pOldData;                  // pointer to data
   USHORT   usLenFieldSize;            // size of data length field

   if ( usVersion >= NTM_VERSION2 )
   {
     usLenFieldSize = sizeof(ULONG);
   }
   else
   {
     usLenFieldSize = sizeof(USHORT);
   } /* endif */

   pusOldOffset = (PUSHORT) pRecord->contents.uchData;
   pusNewOffset = (PUSHORT) pNew->contents.uchData;
   usLastPos = pNew->contents.header.usLastFilled;

   usDataOffs = *(pusOldOffset+i);
   if ( usDataOffs)
   {
      pOldData = pRecord->contents.uchData + usDataOffs;
      if ( usVersion >= NTM_VERSION2 )
      {
        ULONG ulLen = *(PULONG) pOldData;
        if ( ulLen & QDAM_TERSE_FLAGL)
        {
          ulLen &= ~QDAM_TERSE_FLAGL;
        } /* endif */
        usLen = (USHORT)ulLen;
      }
      else
      {
        usLen = *(PUSHORT) pOldData;
        if ( usLen & QDAM_TERSE_FLAG)
        {
          usLen &= ~QDAM_TERSE_FLAG;
        } /* endif */
      }
      usLen = usLen + usLenFieldSize;       // add size of length indication

      assert( (usLastPos >= usLen) );

      usLastPos = usLastPos - usLen;
      memcpy( pNew->contents.uchData+usLastPos, pOldData, usLen );
      *(pusNewOffset+j) = usLastPos;
      pNew->contents.header.usFilled += (usLen + sizeof(USHORT));
      pNew->contents.header.usLastFilled = usLastPos;
   }
   else
   {
      *(pusNewOffset+j) = 0;
      pNew->contents.header.usFilled += sizeof(USHORT);
   } /* endif */

   return;
}


