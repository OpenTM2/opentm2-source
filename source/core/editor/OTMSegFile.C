/*----------------------------------------------------------------------------\*
|                                                                              |
|                Copyright (C) 2018, International Business Machines           |
|                Corporation and others. All rights reserved                   |
|                                                                              |
|   OtmSegFile.C                                                               |
|                                                                              |
|   Segmented file related functions                                           |
|                                                                              |
\*----------------------------------------------------------------------------*/

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include <eqf.h>                  // General Translation Manager include file

/**********************************************************************/
/* Free document structure                                            */
/**********************************************************************/
VOID SegFileFreeDoc( PVOID *ppvDoc )
{
  PTBDOCUMENT *ppDoc = (PTBDOCUMENT *)ppvDoc;
  PTBDOCUMENT pDoc = *ppDoc;
  PTBSEGMENTTABLE pSegTable;          // ptr for segment table deleting
  ULONG           ulI, ulJ;           // loop counter
  PTBSEGMENT      pSegment;           // ptr for segment deleting

  if ( pDoc == NULL )
  {
    return;
  } /* endif */

  UtlAlloc( (PVOID *) &pDoc->pInBuf, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pDoc->pTokBuf, 0L, 0L, NOMSG );

  pSegTable = pDoc->pSegTables;
  for ( ulI = 0; ulI < pDoc->ulSegTables; ulI++ )
  {
     pSegment = pSegTable->pSegments;
     for ( ulJ = 0; ulJ < pSegTable->ulSegments; ulJ++ )
     {
        if ( pSegment->pData ) UtlAlloc( (PVOID *) &pSegment->pData, 0L, 0L, NOMSG );
        if ( pSegment->pDataW ) UtlAlloc( (PVOID *) &pSegment->pDataW, 0L, 0L, NOMSG );

        if (pSegment->pusHLType) UtlAlloc((PVOID*)&(pSegment->pusHLType),0L,0L,NOMSG);
        if (pSegment->pContext) UtlAlloc((PVOID *)&(pSegment->pContext),0L,0L,NOMSG);
        if (pSegment->pvMetadata) UtlAlloc((PVOID *)&(pSegment->pvMetadata),0L,0L,NOMSG);
        pSegment++;
     } /* endfor */
     UtlAlloc( (PVOID *) &pSegTable->pSegments, 0L, 0L, NOMSG );
     pSegTable++;
  } /* endfor */
  pDoc->ulSegTables = 0;
  UtlAlloc( (PVOID *) &pDoc->pSegTables, 0L, 0L, NOMSG );
  if (pDoc->pUndoSeg) UtlAlloc( (PVOID *) &pDoc->pUndoSeg, 0L, 0L, NOMSG );  //free storage of Undo
  if (pDoc->pUndoSegW) UtlAlloc( (PVOID *) &pDoc->pUndoSegW, 0L, 0L, NOMSG );
  if ( pDoc->pContext )  UtlAlloc((PVOID *) &pDoc->pContext, 0L, 0L, NOMSG );
  if ( pDoc->pSegmentBuffer )    UtlAlloc( (PVOID *)&(pDoc->pSegmentBuffer), 0L, 0L, NOMSG );
  if ( pDoc->pSegmentBufferW )   UtlAlloc( (PVOID *)&(pDoc->pSegmentBufferW), 0L, 0L, NOMSG );
  if ( pDoc->pEQFBWorkSegmentW ) UtlAlloc( (PVOID *) &(pDoc->pEQFBWorkSegmentW), 0L, 0L, NOMSG );
  if (pDoc->pContext) UtlAlloc((PVOID *)&(pDoc->pContext),0L,0L,NOMSG);
  UtlAlloc( (PVOID *) &pDoc, 0L, 0L, NOMSG );
  *ppDoc = NULL;

} /* end of function SegFileFreeDoc */

