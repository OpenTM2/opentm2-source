/*!
 UtlMemory.c - memory functions
*/
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2017, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#include <malloc.h>

#include "eqf.h"                  // General Translation Manager include file
#define STATIC_OWNER
#include "Utility.h"
#undef STATIC_OWNER

PVOID UtlIntAlloc
(
  ULONG  ulLength,                     // length of area to allocate
  USHORT usMessageNo                   // message number for error calls
);
static
USHORT UtlSubAlloc
(
  PBYTE pSel,
  PBYTE * ppStorage,
  ULONG  ulAllocLength
);
USHORT UtlIntFree
(
  PVOID  pOldStorage,
  ULONG  ulLength
);

BOOL UtlAlloc
(
   PVOID *ppStorage,                   // pointer to allocated memory area
   LONG   lOldLength,                  // old length of storage area
   LONG   lNewLength,                  // length of area to be allocated
   USHORT usMessageNo)                 // message to be displayed when error
{
  BOOL fSuccess = TRUE;
  PVOID pTemp;
  /********************************************************************/
  /* check mode of operation                                          */
  /********************************************************************/
  if ( (lOldLength == 0L) && (lNewLength != 0L) )    // allocate new memory
  {
    {
      /******************************************************************/
      /* allocate stuff                                                 */
      /******************************************************************/
      *ppStorage = UtlIntAlloc( lNewLength, usMessageNo );
      fSuccess =( *ppStorage != NULL );
    } /* endif */
  }
  else                                 // reallocate or free
  {
    if ( lNewLength == 0L )
    {
      if ( *ppStorage )
      {
        fSuccess = (BOOL) !UtlIntFree( *ppStorage, lOldLength );
        if ( fSuccess )
        {
          *ppStorage = NULL;
        } /* endif */
      } /* endif */
    }
    else
    {
      /******************************************************************/
      /* reallocate stuff                                               */
      /******************************************************************/
      pTemp = UtlIntAlloc( lNewLength, usMessageNo );
      if ( pTemp )
      {
        memcpy( pTemp, *ppStorage, min(lNewLength,lOldLength) );
        fSuccess = (BOOL) ! UtlIntFree( *ppStorage, lOldLength );
        *ppStorage = pTemp;
      }
      else
      {
        fSuccess = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */

  return fSuccess;
} /* end of function UtlAlloc */

//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlAllocHwnd           Interface for UtlAlloc            |
//+----------------------------------------------------------------------------+
//|Description:       Call UtlAlloc and use given window handle for error      |
//|                   messages.                                                |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlAllocHwnd( PVOID  *ppMemory                      |
//|                                      LONG   lOldLength,                    |
//|                                      LONG   lNewLength,                    |
//|                                      USHORT usMsgNo,                       |
//|                                      HWND   hwndOwner );                   |
//+----------------------------------------------------------------------------+
BOOL UtlAllocHwnd
(
   PVOID *ppStorage,                   // pointer to allocated memory area
   LONG   lOldLength,                  // old length of storage area
   LONG   lNewLength,                  // length of area to be allocated
   USHORT usMessageNo,                 // message to be displayed when error
   HWND   hwnd )                       // handle for UtlError Hwnd call
{
  BOOL fOK = UtlAlloc( ppStorage, lOldLength, lNewLength, NOMSG );

  hwnd;
  usMessageNo;
  if ( !fOK && (usMessageNo != NOMSG ))
  {
    UtlErrorHwnd( usMessageNo, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd );
  } /* endif */
  return( fOK );
} /* end of UtlAllocHwnd */


PVOID UtlIntAlloc
(
  ULONG  ulLength,                     // length of area to allocate
  USHORT usMessageNo                   // message number for error calls
)
{
  PBYTE  pStorage;                     // pointer to allocated storage
#ifndef UTL_TRACEALLOC
  USHORT usRc;
  ULONG  ulRealLength;
  ULONG  ulAllocLength;

  SHORT sSegTable;
  PSEGTABLE pSegTable;                // pointer to active segment table
  PSEGENTRY pSegment;                 // pointer to active segment
  BOOL   fWrapAround;
  SHORT  sSegment;

  usMessageNo;
  
  pSegTable = NULL;
  //--- get real length of memory block ---
  ulRealLength = ulLength + sizeof(ULONG) + sizeof(ULONG);

  //--- get segment table used to store memory block ---
  for ( sSegTable = 0; sSegTable < MAX_MEM_TABLES; sSegTable++ )
  {
     pSegTable = UtiVar[UtlGetTask()].SegTable + sSegTable;
     if ( ulRealLength <= pSegTable->usMaxBlockSize )
     {
        break;                           // use this table for allocation
     } /* endif */
  } /* endfor */

  //--- get size of block actually allocated ---
  ulAllocLength = max( ulRealLength, (ULONG)pSegTable->usMinBlockSize );

  pStorage    = NULL;                 // no storage yet ...
  fWrapAround = FALSE;                // not all segments searched thru ...

  sSegment = pSegTable->sSearchSel;   // start in current memory segment

  if ( pSegTable->sMaxSel != SEG_TABLE_EMPTY )
  {
     while ( !pStorage &&             // while no block allocated and
             !fWrapAround )           // not all segments searched thru ...
     {
        // try to obtain the requested storage from the current segment
        usRc = UtlSubAlloc( pSegTable->pSegment[sSegment].pSel,
                            &pStorage, ulAllocLength);
        if ( usRc != 0 )              // if no storage is available ...
        {
           ++sSegment;                // ... continue search in next segment

           if ( sSegment > pSegTable->sMaxSel ) // at end of table ...
           {
              sSegment = 0;                     // ... continue at begin
           } /* endif */

           if ( sSegment == pSegTable->sSearchSel ) // all segments searched ???
           {
              fWrapAround = TRUE;     // set we-are-thru-flag
           } /* endif */
        }
        else
        {
           pSegTable->sSearchSel = sSegment; // here we'll start next time
           ++(pSegTable->pSegment[sSegment].usNoOfBlocks);
        } /* endif */
     } /* endwhile */
  } /* endif */

  if ( pStorage == NULL )             // no space in our segments ???
  {
     if ( (pSegTable->sMaxSel + 1 ) >= MAX_MEM_SEGMENTS )
     {
       /***************************************************************/
       /* nothing to do here                                          */
       /***************************************************************/
       int iOverFlowOfInternalTables = TRUE; // only to have a break poibnt for this condition ...
       iOverFlowOfInternalTables;
     }
     else
     {
       ULONG  ulNewAllocLen;

       /***************************************************************/
       /* check if we are dealing with boxes or if we can do a real   */
       /* alloc                                                       */
       /***************************************************************/
       if ( ulRealLength <= pSegTable->usMinBlockSize )
       {
         /***************************************************************/
         /* allocate segment pointer...                                 */
         /***************************************************************/
         if ( !pSegTable->pSegment )
         {
           pSegTable->pSegment = (PSEGENTRY) malloc( sizeof(SEGENTRY) * MAX_MEM_SEGMENTS );
           if ( pSegTable->pSegment )
           {
             memset( pSegTable->pSegment, 0, sizeof(SEGENTRY)*MAX_MEM_SEGMENTS);
           } /* endif */
         } /* endif */

         if ( pSegTable->pSegment )
         {
           PBYTE  pHlp;
           // allocate areas large enough to contain at least 128 blocks
           ULONG  ulMinSize = (ULONG)pSegTable->usMinBlockSize * 128L;

           ulNewAllocLen = max( 0xFF00L, ulMinSize );
           pHlp = pSegTable->pSegment[pSegTable->sMaxSel+1].pSel =
                             (PBYTE) malloc( ulNewAllocLen );
           if ( pHlp )
           {
             PSUBALLOC_HDR pSubAlloc;
             memset( pHlp, 0, ulNewAllocLen);
             pSubAlloc = (PSUBALLOC_HDR) pHlp;

             pSubAlloc->usSizeOfBlock = pSegTable->usMinBlockSize + 1;
             pSubAlloc->usNumOfBlocks = (USHORT)
                ((ulNewAllocLen-(ULONG)sizeof(SUBALLOC_HDR)) / (ULONG)pSubAlloc->usSizeOfBlock);
             pSubAlloc->usLastUsed = 0;
             ++(pSegTable->sMaxSel);
             pSegment = pSegTable->pSegment + pSegTable->sMaxSel;
             pSegment->usNoOfBlocks = 0;

             usRc  = UtlSubAlloc (pSegment->pSel,
                                  &pStorage, ulAllocLength);

             if ( !usRc )
             {
                // search selector for last table is always the first
                // segment, all other tables start with the current
                // segment
                if ( (sSegTable + 1) == MAX_MEM_TABLES )
                {
                   pSegTable->sSearchSel = 0;
                }
                else
                {
                   pSegTable->sSearchSel = pSegTable->sMaxSel;
                } /* endif */
                sSegment = pSegTable->sMaxSel;
                pSegment->usNoOfBlocks++;
             } /* endif */
           } /* endif */
         } /* endif */
       }
       else
       {
         pStorage = (PBYTE) malloc( ulLength + 2 * sizeof(ULONG) );
       } /* endif */
     } /* endif */
  } /* endif */

  /******************************************************************/
  /* allocate stuff                                                 */
  /******************************************************************/
#else
    pStorage = (PBYTE) malloc( ulLength + 2 * sizeof(ULONG) );
#endif
  /********************************************************************/
  /* preset head and trailer as anchor for kamikaze persons....       */
  /********************************************************************/

  if ( pStorage )
  {
    *((PULONG) pStorage ) = ulLength;
    pStorage = (PBYTE) pStorage + sizeof(ULONG);
    *( (PULONG) ((PBYTE)pStorage + ulLength)) = ulLength;
    memset( pStorage, 0, ulLength );
  }
  else 
  {
    if ( usMessageNo != NOMSG )
    {
      UtlError( usMessageNo, MB_CANCEL, 0, NULL, EQF_ERROR );
    } /* endif */
  } /* endif */
  return( pStorage );
}

/**********************************************************************/
/* do an efficient sub allocation...                                  */
/**********************************************************************/
static
USHORT UtlSubAlloc
(
  PBYTE pSel,
  PBYTE * ppStorage,
  ULONG  ulAllocLength
)
{
  USHORT usRc = 0;
  PSUBALLOC_HDR pSubAlloc = (PSUBALLOC_HDR) pSel;
  PBYTE  pData = pSel + sizeof( SUBALLOC_HDR );
  USHORT usLast;

  *ppStorage = NULL;
  ulAllocLength = 0;

  /********************************************************************/
  /* check if we can find a free slot ...                             */
  /********************************************************************/
  usLast = pSubAlloc->usLastUsed;
  pData += (ULONG)pSubAlloc->usLastUsed * (ULONG)pSubAlloc->usSizeOfBlock;

  while ( *pData && ( usLast < pSubAlloc->usNumOfBlocks - 1 ))
  {
    pData += pSubAlloc->usSizeOfBlock;
    usLast++;
  } /* endwhile */

  /********************************************************************/
  /* check if we find a free slot, otherwise wrap around              */
  /********************************************************************/
  if ( usLast == pSubAlloc->usNumOfBlocks-1 )
  {
    /******************************************************************/
    /* start at beginning ...                                         */
    /******************************************************************/
    usLast = 0;
    pData = pSel + sizeof( SUBALLOC_HDR );
    while ( *pData && ( usLast < pSubAlloc->usLastUsed ))
    {
      pData += pSubAlloc->usSizeOfBlock;
      usLast++;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* return the found slot or an error condition                      */
  /********************************************************************/
  if ( *pData )
  {
    usRc = 1;
  }
  else
  {
    *pData = 1;
    *ppStorage = pData+1;
    pSubAlloc->usLastUsed = usLast;
  } /* endif */

  return usRc;
}

USHORT UtlIntFree
(
  PVOID  pOldStorage,
  ULONG  ulLength
)
{
  USHORT usRC = 0;
  PBYTE  pStorage = (PBYTE) pOldStorage - sizeof(ULONG);
  ULONG  ulActLength = *((PULONG) pStorage );
  ULONG  ulActLength2 = *((PULONG)(((PBYTE) pOldStorage) + ulActLength));

  ulLength;
  /********************************************************************/
  /* check if someone tried to play KAMIKAZE with our memory,         */
  /* fuck him....                                                     */
  /********************************************************************/
  if ( ulActLength != ulActLength2 )
  {
    /******************************************************************/
    /* display error message - someone killed us...                   */
    /******************************************************************/
    usRC = ERROR_INTERNAL;
//    UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
  }
  else
  {
#ifndef UTL_TRACEALLOC
     ULONG  ulRealLength = ulActLength + sizeof(ULONG) + sizeof(ULONG);
     SHORT  sSegTable;
     PSEGTABLE pSegTable;                // pointer to active segment table
     PSEGENTRY pSegment;                 // pointer to active segment

     pSegment = NULL;
     pSegTable = NULL;
     //--- get segment table used to store memory block ---
     for ( sSegTable = 0; sSegTable < MAX_MEM_TABLES; sSegTable++ )
     {
        pSegTable = UtiVar[UtlGetTask()].SegTable + sSegTable;
        if ( ulRealLength <= pSegTable->usMaxBlockSize )
        {
           break;                           // use this table for allocation
        } /* endif */
     } /* endfor */

     /*****************************************************************/
     /* check if block was allocated with 'normal' alloc or with our  */
     /* special suballocation                                         */
     /*****************************************************************/
     if ( ulRealLength > pSegTable->usMinBlockSize )
     {
       free( pStorage );
     }
     else
     {
       /***************************************************************/
       /* find right table                                            */
       /***************************************************************/
       SHORT  sSegment;

       for (sSegment = 0; sSegment <= pSegTable->sMaxSel; sSegment++ )
       {
         pSegment = pSegTable->pSegment + sSegment;
         if ( (pStorage > pSegment->pSel) &&
              (pStorage < (pSegment->pSel + 0xFF00) ) )
         {
           /***********************************************************/
           /* we found it -> free area                                */
           /***********************************************************/
           *(pStorage-1) = 0;
           pSegment->usNoOfBlocks --;
           break;
         } /* endif */
       } /* endfor */
       /***************************************************************/
       /* release whole block if nothing is in there                  */
       /***************************************************************/
       if ((sSegment <= pSegTable->sMaxSel) && (pSegment->usNoOfBlocks == 0))
       {
         if ( pSegTable->sMaxSel > 1 )
         {
          free( pSegment->pSel );
           if ( sSegment < pSegTable->sMaxSel )
           {
             memmove( pSegment, pSegment+1,
                      sizeof(SEGENTRY) * (pSegTable->sMaxSel - sSegment));
           } /* endif */
           pSegTable->sMaxSel--;
           pSegTable->sSearchSel = 0;
         } /* endif */
       } /* endif */
     } /* endif */
#else
    /******************************************************************/
    /* free the momory                                                */
    /******************************************************************/
     free( pStorage );
#endif
  } /* endif */
  return usRC;
} /* end of function UtlIntFree */

/**********************************************************************/
/*  get the correct task id ...                                       */
/**********************************************************************/
USHORT UtlGetTask ( void )
{
  WORD   usTask;
  USHORT currTask;

        _asm
          {
            mov      ax, SS
            mov      usTask, ax
          }
  for ( currTask = 0; currTask < MAX_TASK ; ++currTask )
  {
    if ( UtiVar[currTask].usTask == usTask )
    {
      break;
    }
    else if ( UtiVar[currTask].usTask == 0 )
    {
      /****************************************************************/
      /* empty slot found                                             */
      /****************************************************************/
      SEGTABLE SegTable[MAX_MEM_TABLES] =  // table of segment tables
          { {  NULL, SEG_TABLE_EMPTY,     16,    16,    0 },
            {  NULL, SEG_TABLE_EMPTY,     64,    64,    0 },
            {  NULL, SEG_TABLE_EMPTY,    128,   128,    0 },
            {  NULL, SEG_TABLE_EMPTY,    256,   256,    0 },
            {  NULL, SEG_TABLE_EMPTY,    512,   512,    0 },
            {  NULL, SEG_TABLE_EMPTY,   1020,  1020,    0 },
            {  NULL, SEG_TABLE_EMPTY,   2040,  2040,    0 },
            {  NULL, SEG_TABLE_EMPTY, 0xFFFF,     1,    0 } };
      UtiVar[currTask].usTask = usTask;
      memcpy(&UtiVar[currTask].SegTable, &SegTable,
             sizeof(SEGTABLE) * MAX_MEM_TABLES );
      break;
    } /* endif */
  } /* endfor */
  return( currTask );
}
