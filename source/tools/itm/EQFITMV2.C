/*! \file
	Description: Visual ITM edit functions

	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/
#define INCL_EQF_MORPH
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_EDITORAPI        // editor API

  // use import DLL defines for dbcs_cp ...
  #define DLLIMPORTDBCSCP
#include <eqf.h>                  // General Translation Manager include file

#include "EQFITM.H"
#include <eqfitmd.id>             // id file for pulldowns

/**********************************************************************/
/* test output ...                                                    */
/**********************************************************************/
#ifdef ITMTEST
  FILE *fOut;                          // test output
#endif
#define MSGBOXDATALEN     30        // data len of segment to be displ.
static CHAR_W chSeg1[MSGBOXDATALEN + 1];  // start of segment 1
static CHAR_W chSeg2[MSGBOXDATALEN + 1];  // start of segment 2

static BOOL VisJoinSegData ( PTBDOCUMENT, ULONG, ULONG ); // join segment data

static BOOL VisSplitSegData ( PTBDOCUMENT, ULONG, ULONG,
                               LONG, LONG ); // split seg data

static VOID CalcInvalIndices ( PITMIDA, PULONG, PULONG,
                               ULONG, ULONG, ULONG, ULONG  );

static VOID Realign ( PITMIDA, ULONG , ULONG, ULONG, ULONG, SHORT );
static VOID VisDocDelAnchor ( PITMVISDOC, PITMVISDOC, USHORT, USHORT );
static VOID ITMDelAnchor (PITMIDA, PITMVISDOC, PITMVISDOC, ULONG, ULONG );
static VOID FindParallelRegion ( PITMIDA, ULONG, ULONG,
                                 PULONG, PULONG, PULONG, PULONG );

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncSetAnchor(PITMIDA)
//------------------------------------------------------------------------------
// Function call:     ITMFuncSetAnchor(PITMIDA)
//------------------------------------------------------------------------------
// Description:       set anchor to marked segments
//------------------------------------------------------------------------------
// Parameters:        PITMIDA pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       -
//------------------------------------------------------------------------------
// Function flow:     get srcsegnum and tgt segnum of marked segmentd
//                    reset QF_VISACT status
//                    reset crossed.out flag on src and tgt segnums
//                    reset also qStatus if it was QF_CROSSED_OUT
//                    if segment is joined, set flag userjoined
//                    if new anchor overlaps old, delete old anchor
//                    set new user anchor
//                    find next and previous anchor
//                    check whether new anchor is an overcrossing
//                    if not overcross
//                      realign inval region
//                      reset qstatus
//                    else
//                      set overcross flag and QF_OVERCROSS
//                      realign both regions affected by the new anchor
//                    endif
//                    set active segment & refresh screen
//------------------------------------------------------------------------------

VOID
ITMFuncSetAnchor
(
   PITMIDA    pITMIda
)
{
  PITMVISDOC    pstVisSrc;
  PITMVISDOC    pstVisTgt;
  PTBDOCUMENT   pSrcDoc;
  PTBDOCUMENT   pTgtDoc;
  PEQFBBLOCK    pstSrcBlock;                         // pointer to block struct
  PEQFBBLOCK    pstTgtBlock;                         // pointer to block struct
  ULONG         ulSrcSeg;
  ULONG         ulTgtSeg;
  ULONG         ulOldAnchorSeg;
  ULONG         ulSrcStart;
  ULONG         ulSrcEnd;
  ULONG         ulTgtStart;
  ULONG         ulTgtEnd;
  PTBSEGMENT    pSrcSeg;
  PTBSEGMENT    pTgtSeg;
  BOOL          fOverCross;

  pstVisSrc = &(pITMIda->stVisDocSrc);
  pstVisTgt = &(pITMIda->stVisDocTgt);
  /********************************************************************/
  /* find marked segment                                              */
  /********************************************************************/
  pSrcDoc = pstVisSrc->pDoc;
  pTgtDoc = pstVisTgt->pDoc;
  pstSrcBlock = (PEQFBBLOCK) pSrcDoc->pBlockMark;
  pstTgtBlock = (PEQFBBLOCK) pTgtDoc->pBlockMark;
  if ( pstSrcBlock && (pstSrcBlock->pDoc != NULL )
        && pstTgtBlock && (pstTgtBlock->pDoc != NULL)  )
  {
     SETCURSOR( SPTR_WAIT );
     ulSrcSeg = pstSrcBlock->ulSegNum;
     ulTgtSeg = pstTgtBlock->ulSegNum;

     EQFBFuncMarkClear( pstVisSrc->pDoc );            // remove blockmark
     EQFBFuncMarkClear( pstVisTgt->pDoc );            // remove blockmark
     /********************************************************************/
     /* guarantee that no seg has qStatus == QF_VISACT                   */
     /********************************************************************/
     VisActReset(pstVisSrc, pstVisTgt);
     /*****************************************************************/
     /* if anchor overlaps crossed out segment, undo cross-out        */
     /* if a segment is joined, keep it joined by setting .UserJoin   */
     /*****************************************************************/
     pstVisSrc->pVisState[ulSrcSeg].CrossedOut = FALSE;
     pstVisTgt->pVisState[ulTgtSeg].CrossedOut = FALSE;

     pSrcSeg = EQFBGetSegW(pstVisSrc->pDoc, ulSrcSeg);
     if ( pSrcSeg->qStatus == QF_CROSSED_OUT )
     {
       pSrcSeg->qStatus = QF_TOBE;
     } /* endif */
     if ( pSrcSeg->qStatus == QF_CROSSED_OUT_NOP )
     {
       pSrcSeg->qStatus = QF_NOP;
     } /* endif */
     if ( pSrcSeg->SegFlags.JoinStart )
     {
       pstVisSrc->pVisState[ulSrcSeg].UserJoin = TRUE;
     } /* endif */

     pTgtSeg = EQFBGetSegW(pstVisTgt->pDoc, ulTgtSeg);
     if ( pTgtSeg->qStatus == QF_CROSSED_OUT )
     {
       pTgtSeg->qStatus = QF_TOBE;
     } /* endif */
     if ( pTgtSeg->qStatus == QF_CROSSED_OUT_NOP )
     {
       pTgtSeg->qStatus = QF_NOP;
     } /* endif */
     if ( pTgtSeg->SegFlags.JoinStart )
     {
       pstVisTgt->pVisState[ulTgtSeg].UserJoin = TRUE;
     } /* endif */
     /*****************************************************************/
     /* if new anchor overlaps with old anchors, delete old anchors   */
     /*****************************************************************/
     ulOldAnchorSeg = pstVisSrc->pulAnchor[ulSrcSeg];
     if ( ulOldAnchorSeg )
     {
       ITMDelAnchor (pITMIda, pstVisSrc, pstVisTgt, ulSrcSeg, ulOldAnchorSeg);
     } /* endif */

     ulOldAnchorSeg = pstVisTgt->pulAnchor[ulTgtSeg];
     if ( ulOldAnchorSeg )
     {
       ITMDelAnchor(pITMIda, pstVisSrc, pstVisTgt, ulOldAnchorSeg, ulTgtSeg);
     } /* endif */

     /*****************************************************************/
     /* set the user anchor                                           */
     /*****************************************************************/
     pstVisSrc->pulAnchor[ulSrcSeg] = ulTgtSeg;
     pstVisTgt->pulAnchor[ulTgtSeg] = ulSrcSeg;
     pstVisSrc->pVisState[ulSrcSeg].UserAnchor = TRUE;
     pstVisTgt->pVisState[ulTgtSeg].UserAnchor = TRUE;
     /*****************************************************************/
     /* find invalid area: anchor found must not be overcross!        */
     /*****************************************************************/
     ulSrcStart = ulSrcSeg - 1;
     ulSrcEnd = ulSrcSeg + 1;
//     pSeg = EQFBGetPrevVisSeg(pstVisSrc->pDoc, &ulSrcStart);
//     pSeg = EQFBGetVisSeg(pstVisSrc->pDoc, &ulSrcEnd);

     ulSrcStart = FindNextAnchor(pstVisSrc, ulSrcStart,PREVIOUS);
     ulSrcEnd = FindNextAnchor(pstVisSrc, ulSrcEnd, NEXT);
     if ( !ulSrcEnd )              // if at end of file
     {
       ulSrcEnd = pstVisSrc->pDoc->ulMaxSeg;
     } /* endif */

     ulTgtStart = pstVisSrc->pulAnchor[ulSrcStart];
     ulTgtEnd = pstVisSrc->pulAnchor[ulSrcEnd];
     /*****************************************************************/
     /* check for overcrossing anchor                                 */
     /*****************************************************************/
     fOverCross = FALSE;
     if ( ulTgtSeg < ulTgtStart )
     {
       fOverCross = TRUE;
     } /* endif */
     if ( ulTgtSeg > ulTgtEnd )
     {
       fOverCross = TRUE;
     } /* endif */
     if ( !fOverCross )
     {
       VisAlignBlock (pITMIda, ulSrcStart, ulSrcSeg, ulSrcEnd,
                               ulTgtStart, ulTgtSeg, ulTgtEnd );
       VisSetQStatus( pstVisSrc, ulSrcStart, ulSrcEnd);
       VisSetQStatus( pstVisTgt, ulTgtStart, ulTgtEnd);

       ITMCountStatBar (pITMIda);
     }
     else
     {
       pstVisSrc->pVisState[ulSrcSeg].OverCross = TRUE;
       pstVisTgt->pVisState[ulTgtSeg].OverCross = TRUE;

       pSrcSeg->qStatus = QF_OVERCROSS;                   //set crossed out status
       VisInvalRegion(pITMIda, pstVisSrc, ulSrcSeg);

       pTgtSeg->qStatus = QF_OVERCROSS;                   //set crossed out status
       VisInvalRegion(pITMIda, pstVisTgt, ulTgtSeg);
#ifdef ITMTEST
  fOut      = fopen ( "ITMSTAT.OUT", "a" );
  fprintf (fOut, "set overcross %4d %4d ,\n", ulSrcSeg, ulTgtSeg);
  fclose( fOut      );
#endif

     } /* endif */

     /******************************************************************/
     /* set active alignment next to crossed-out segment               */
     /******************************************************************/
     VisActivateSeg(pITMIda, pstVisSrc, CURRENT, FALSE, ulSrcSeg);

     pITMIda->TBSourceDoc.Redraw |= REDRAW_ALL;        // redraw all
     pITMIda->TBTargetDoc.Redraw |= REDRAW_ALL;

     SETCURSOR( SPTR_ARROW );
     EQFBRefreshScreen( &(pITMIda->TBSourceDoc) );     // refresh the screen
     EQFBRefreshScreen( &(pITMIda->TBTargetDoc) );     // refresh the screen
  }
  else
  {
     ITMUtlError( pITMIda, ITM_MARKTWO, MB_CANCEL, 0, NULL, EQF_WARNING);
  } /* endif */

  UPDSTATUSBAR( pITMIda );
} /* end of function VOID ITMFuncSetAnchor(PITMIDA) */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncDelAnchor(PITMIDA)
//------------------------------------------------------------------------------
// Function call:     ITMFuncDelAnchor(PITMIDA)
//------------------------------------------------------------------------------
// Description:       delete an anchor
//------------------------------------------------------------------------------
// Parameters:        PITMIDA pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     get segnum of marked segment
//                    check that an anchor is marked correctly
//                    if correct marked
//                      clear blockmark
//                      reset active alignment
//                      delete anchor
//                      activate current segment
//                      refresh screen
//------------------------------------------------------------------------------

VOID
ITMFuncDelAnchor
(
   PITMIDA    pITMIda
)
{
  PITMVISDOC    pstVisSrc;
  PITMVISDOC    pstVisTgt;
  PTBDOCUMENT   pSrcDoc;
  PTBDOCUMENT   pTgtDoc;
  PEQFBBLOCK    pstSrcBlock;                                    // pointer to block struct
  PEQFBBLOCK    pstTgtBlock;                                    // pointer to block struct
  ULONG         ulSrcSeg = 0;
  ULONG         ulTgtSeg = 0;
  BOOL          fOK;

  pstVisSrc = &(pITMIda->stVisDocSrc);
  pstVisTgt = &(pITMIda->stVisDocTgt);
  /********************************************************************/
  /* find marked segment                                              */
  /********************************************************************/
  pSrcDoc = pstVisSrc->pDoc;
  pTgtDoc = pstVisTgt->pDoc;
  pstSrcBlock = (PEQFBBLOCK) pSrcDoc->pBlockMark;
  pstTgtBlock = (PEQFBBLOCK) pTgtDoc->pBlockMark;
  fOK = FALSE;
  if ( pstSrcBlock && (pstSrcBlock->pDoc != NULL )
        && pstTgtBlock && (pstTgtBlock->pDoc != NULL)  )
  {  // 2 segments are marked
     ulSrcSeg = pstSrcBlock->ulSegNum;
     ulTgtSeg = pstTgtBlock->ulSegNum;

     if ( pstVisSrc->pulAnchor[ulSrcSeg] == ulTgtSeg )
     {
       fOK = TRUE;
     } /* endif */
  }
  else
  {
    /******************************************************************/
    /* source segment is only marked                                  */
    /******************************************************************/
    if ( pstSrcBlock && (pstSrcBlock->pDoc != NULL) )
    {
      ulSrcSeg = pstSrcBlock->ulSegNum;
      ulTgtSeg  = pstVisSrc->pulAnchor[ulSrcSeg];
      if ( ulTgtSeg )
      {
        fOK = TRUE;
      } /* endif */
    } /* endif */
    /******************************************************************/
    /* target segment is only marked                                  */
    /******************************************************************/
    if (pstTgtBlock && (pstTgtBlock->pDoc != NULL)  )
    {
      ulTgtSeg = pstTgtBlock->ulSegNum;
      ulSrcSeg  = pstVisTgt->pulAnchor[ulTgtSeg];
      if ( ulSrcSeg )
      {
        fOK = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */
  if ( fOK )
  {
     SETCURSOR( SPTR_WAIT );
     EQFBFuncMarkClear( pstVisSrc->pDoc );            // remove blockmark
     EQFBFuncMarkClear( pstVisTgt->pDoc );                 // remove blockmark
     VisActReset(pstVisSrc, pstVisTgt);

     ITMDelAnchor (pITMIda, pstVisSrc, pstVisTgt, ulSrcSeg, ulTgtSeg);

     VisActivateSeg(pITMIda, pstVisSrc, CURRENT, FALSE, ulSrcSeg);
     pITMIda->TBSourceDoc.Redraw |= REDRAW_ALL;                  // redraw the line
     pITMIda->TBTargetDoc.Redraw |= REDRAW_ALL;
     SETCURSOR( SPTR_ARROW );
     EQFBRefreshScreen( &(pITMIda->TBSourceDoc) );     // refresh the screen
     EQFBRefreshScreen( &(pITMIda->TBTargetDoc) );     // refresh the screen

  }
  else
  {
    ITMUtlError( pITMIda, ITM_MARKANCHOR, MB_CANCEL, 0, NULL, EQF_WARNING);
  } /* endif */

  UPDSTATUSBAR( pITMIda );
} /* end of function VOID ITMFuncDelAnchor(PITMIDA) */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncDelAllUser(PITMIDA)
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       _
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   _
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------

VOID
ITMFuncDelAllUser
(
   PITMIDA    pITMIda
)
{
  pITMIda;

} /* end of function VOID ITMFuncDelAllUser(PITMIDA) */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncCrossOut(PITMIDA)
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       cross out marked segment
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     find marked segment
//                    if exact one segment is marked, call ITMCrossOut,
//                    else display error
//------------------------------------------------------------------------------

VOID
ITMFuncCrossOut
(
   PITMIDA    pITMIda
)
{
  PITMVISDOC    pstVisSrc;
  PITMVISDOC    pstVisTgt;
  PTBDOCUMENT   pSrcDoc;
  PTBDOCUMENT   pTgtDoc;
  PEQFBBLOCK    pstSrcBlock;                                    // pointer to block struct
  PEQFBBLOCK    pstTgtBlock;                                    // pointer to block struct

  pstVisSrc = &(pITMIda->stVisDocSrc);
  pstVisTgt = &(pITMIda->stVisDocTgt);
  /********************************************************************/
  /* find marked segment                                              */
  /********************************************************************/
  pSrcDoc = pstVisSrc->pDoc;
  pTgtDoc = pstVisTgt->pDoc;
  pstSrcBlock = (PEQFBBLOCK) pSrcDoc->pBlockMark;
  pstTgtBlock = (PEQFBBLOCK) pTgtDoc->pBlockMark;
  if ( pstSrcBlock && pstSrcBlock->pDoc != NULL )
  {
    if ( pstTgtBlock && pstTgtBlock->pDoc != NULL)
    {
    /******************************************************************/
    /* only one segment can be crossed-out , but two segs are marked  */
    /******************************************************************/
      ITMUtlError( pITMIda, ITM_MARKEXACTONE, MB_CANCEL, 0, NULL, EQF_WARNING);
    }
    else
    {
      ITMCrossOutBlock(pstVisSrc);
    } /* endif */
  }
  else
  {
    if ( pstTgtBlock && pstTgtBlock->pDoc != NULL)
    {
      ITMCrossOutBlock(pstVisTgt);
    }
    else
    {
      /******************************************************************/
      /* no segment is marked, hence no segment can be crossed out      */
      /******************************************************************/
      ITMUtlError( pITMIda, ITM_MARKEXACTONE, MB_CANCEL, 0, NULL, EQF_WARNING);
    } /* endif */
  } /* endif */
  UPDSTATUSBAR( pITMIda );

} /* end of function VOID ITMFuncCrossOut(PITMIDA) */
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncUndoCrossOut(PITMIDA)
//------------------------------------------------------------------------------
// Function call:     ITMFuncUndoCrossOut(PITMIDA)
//------------------------------------------------------------------------------
// Description:       undo cross out
//------------------------------------------------------------------------------
// Parameters:        PITMIDA pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     if exactly one segment is marked,
//                     call ITMUndoCrossOut
//                   else display error
//------------------------------------------------------------------------------

VOID
ITMFuncUndoCrossOut
(
   PITMIDA    pITMIda
)
{
  PITMVISDOC    pstVisSrc;
  PITMVISDOC    pstVisTgt;
  PTBDOCUMENT   pSrcDoc;
  PTBDOCUMENT   pTgtDoc;
  PEQFBBLOCK    pstSrcBlock;                                    // pointer to block struct
  PEQFBBLOCK    pstTgtBlock;                                    // pointer to block struct

  pstVisSrc = &(pITMIda->stVisDocSrc);
  pstVisTgt = &(pITMIda->stVisDocTgt);
  /********************************************************************/
  /* find marked segment                                              */
  /********************************************************************/
  pSrcDoc = pstVisSrc->pDoc;
  pTgtDoc = pstVisTgt->pDoc;
  pstSrcBlock = (PEQFBBLOCK) pSrcDoc->pBlockMark;
  pstTgtBlock = (PEQFBBLOCK) pTgtDoc->pBlockMark;
  if ( pstSrcBlock && pstSrcBlock->pDoc != NULL )
  {
    if ( pstTgtBlock && pstTgtBlock->pDoc != NULL)
    {
    /******************************************************************/
    /* only one segment can be crossed-out , but two segs are marked  */
    /******************************************************************/
      ITMUtlError( pITMIda, ITM_MARKEXACTONE, MB_CANCEL, 0, NULL, EQF_WARNING);
    }
    else
    {
      ITMUndoCrossOutBlock(pstVisSrc);
    } /* endif */
  }
  else
  {
    if ( pstTgtBlock && pstTgtBlock->pDoc != NULL)
    {
      ITMUndoCrossOutBlock(pstVisTgt);
    }
    else
    {
      /******************************************************************/
      /* no segment is marked, hence no segment can be crossed out      */
      /******************************************************************/
      ITMUtlError( pITMIda, ITM_MARKEXACTONE, MB_CANCEL, 0, NULL, EQF_WARNING);
    } /* endif */
  } /* endif */

  UPDSTATUSBAR( pITMIda );
} /* end of function VOID ITMFuncUndoCrossOut(PITMIDA) */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMCrossOutBlock
//------------------------------------------------------------------------------
// Function call:     ITMCrossOutBlock(pVisDoc)
//------------------------------------------------------------------------------
// Description:       cross out marked segment in specified visdoc
//------------------------------------------------------------------------------
// Parameters:        PITMVISDOC pVisDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     get ulSegNum of marked segment
//                    clear mark
//                    reset active alignment
//                    set flag CrossedOut and qStatus = QF_CROSSED_OUT
//                    if segment is joined, set it to userjoin
//                    if segment is part of an anchor, delete anchor
//                    realign inval region
//                    activate segment next to crossed-out segment
//                    refresh screen
//------------------------------------------------------------------------------

VOID ITMCrossOutBlock
(
 PITMVISDOC   pITMVisDoc                                 //doc with crossed seg
)
{
  ULONG       ulSegNum;
  PTBSEGMENT  pSeg;
  ULONG       ulTwinSegNum;
  PEQFBBLOCK  pstBlock;                                    // pointer to block struct
  PITMIDA     pIda;

  SETCURSOR( SPTR_WAIT );
  pIda = (PITMIDA) pITMVisDoc->pITMIda;
  pstBlock = (PEQFBBLOCK) pITMVisDoc->pDoc->pBlockMark;
  ulSegNum = pstBlock->ulSegNum;                           //segnum of blockmark

  EQFBFuncMarkClear( pITMVisDoc->pDoc );                   // remove blockmark
  VisActReset(&(pIda->stVisDocSrc),&(pIda->stVisDocTgt));

  pITMVisDoc->pVisState[ulSegNum].CrossedOut = TRUE;       //set crossed-out flag

  pSeg = EQFBGetSegW(pITMVisDoc->pDoc, ulSegNum);

  if ((pSeg->qStatus == QF_NOP) || (pSeg->qStatus == QF_NOP_ANCHOR_1)
      || (pSeg->qStatus == QF_NOP_ANCHOR_2)
      || (pSeg->qStatus == QF_NOP_ANCHOR_3)
      || (pSeg->qStatus == QF_CROSSED_OUT_NOP))
  {
    pSeg->qStatus = QF_CROSSED_OUT_NOP;            //set nopcrossedout status
  }
  else
  {
    pSeg->qStatus = QF_CROSSED_OUT;                   //set crossed out status
  } /* endif */

  /********************************************************************/
  /* if segment is joined, keep it joined                             */
  /********************************************************************/
  if ( pSeg->SegFlags.JoinStart )
  {
    pITMVisDoc->pVisState[ulSegNum].UserJoin = TRUE;
  } /* endif */

  /********************************************************************/
  /* if crossed-out seg belongs to an anchor, delete anchor           */
  /********************************************************************/
  if ( pITMVisDoc->pulAnchor[ulSegNum])
  {
    ulTwinSegNum = pITMVisDoc->pulAnchor[ulSegNum];
    if ( pITMVisDoc->pDoc->docType == VISSRC_DOC)
    {
      ITMDelAnchor (pIda,  &(pIda->stVisDocSrc), &(pIda->stVisDocTgt),
                          ulSegNum, ulTwinSegNum );
    }
    else
    {
      ITMDelAnchor( pIda, &(pIda->stVisDocSrc), &(pIda->stVisDocTgt),
                          ulTwinSegNum,  ulSegNum );
    } /* endif */
  } /* endif */

  pITMVisDoc->pVisState[ulSegNum].CrossedOut = TRUE;       //set crossed-out flag
  if ((pSeg->qStatus == QF_NOP) || (pSeg->qStatus == QF_NOP_ANCHOR_1)
      || (pSeg->qStatus == QF_NOP_ANCHOR_2)
      || (pSeg->qStatus == QF_NOP_ANCHOR_3)
      || (pSeg->qStatus == QF_CROSSED_OUT_NOP))
  {
    pSeg->qStatus = QF_CROSSED_OUT_NOP;            //set nopcrossedout status
  }
  else
  {
    pSeg->qStatus = QF_CROSSED_OUT;                   //set crossed out status
  } /* endif */

  /********************************************************************/
  /* align invalidated area again                                     */
  /********************************************************************/
  VisInvalRegion(pIda, pITMVisDoc, ulSegNum);

  VisActivateSeg(pIda, pITMVisDoc, CURRENT, FALSE, ulSegNum);

  pIda->TBSourceDoc.Redraw |= REDRAW_ALL;                  // redraw the line
  pIda->TBTargetDoc.Redraw |= REDRAW_ALL;
  SETCURSOR( SPTR_ARROW );
  EQFBRefreshScreen( &(pIda->TBSourceDoc) );     // refresh the screen
  EQFBRefreshScreen( &(pIda->TBTargetDoc) );     // refresh the screen

  UPDSTATUSBAR( pIda );

} /* end of function ITMCrossOutBlock */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMUndoCrossOutBlock
//------------------------------------------------------------------------------
// Function call:     ITMUndoCrossOutBLock(pVisDoc)
//------------------------------------------------------------------------------
// Description:       undo the cross-out of an segment
//------------------------------------------------------------------------------
// Parameters:        PITMVISDOC pVisDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     get segnum of marked segment
//                    clear mark
//                    reset active segment
//                    reset crossed-out flag and qstatus
//                    realign region
//                    activate current alignment
//                    refresh screen
//------------------------------------------------------------------------------

VOID ITMUndoCrossOutBlock
(
 PITMVISDOC   pITMVisDoc                                 //doc with crossed seg
)
{
  ULONG      ulSegNum;
  PTBSEGMENT pSeg;
  PEQFBBLOCK  pstBlock;                                    // pointer to block struct
  PITMIDA     pIda;


  SETCURSOR( SPTR_WAIT );
  pIda = (PITMIDA) pITMVisDoc->pITMIda;
  pstBlock = (PEQFBBLOCK) pITMVisDoc->pDoc->pBlockMark;
  ulSegNum = pstBlock->ulSegNum;                           //segnum of blockmark

  EQFBFuncMarkClear( pITMVisDoc->pDoc );                   // remove blockmark
  VisActReset(&(pIda->stVisDocSrc),&(pIda->stVisDocTgt));

  pITMVisDoc->pVisState[ulSegNum].CrossedOut = FALSE;       //set crossed-out flag

  /********************************************************************/
  /* if QF_CROSSSED_OUT is still set, it would not be aligned         */
  /* (see GetSegmentBlock)                                            */
  /********************************************************************/
  pSeg = EQFBGetSegW(pITMVisDoc->pDoc, ulSegNum);
  if ( pSeg->qStatus == QF_CROSSED_OUT )        // reset crossed out status
  {
    pSeg->qStatus = QF_TOBE;
  }
  else
  {
    pSeg->qStatus = QF_NOP;
  } /* endif */


  VisInvalRegion(pIda, pITMVisDoc, ulSegNum);
  /******************************************************************/
  /* set active alignment next to uncrossed   segment               */
  /******************************************************************/
  VisActivateSeg(pIda, pITMVisDoc, CURRENT, FALSE, ulSegNum);

  pIda->TBSourceDoc.Redraw |= REDRAW_ALL;                  // redraw the line
  pIda->TBTargetDoc.Redraw |= REDRAW_ALL;
  SETCURSOR( SPTR_ARROW );
  EQFBRefreshScreen( &(pIda->TBSourceDoc) );     // refresh the screen
  EQFBRefreshScreen( &(pIda->TBTargetDoc) );     // refresh the screen
  UPDSTATUSBAR( pIda );

} /* end of function ITMUndoCrossOutBlock */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     VisAlignBlock
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       the region  : in src from ulSrcStart to ulSrcEnd,
//                                  in tgt from ulTgtStart to ulTgtEnd
//                    must be aligned again.
//                    Middle is not 0 if it is a new user anchor
//------------------------------------------------------------------------------
// Parameters:        PITMIDA     pITMIda,
//                    USHORT      ulSrcStart, //src start seg of inval region
//                    USHORT      usSrcMid,   // middle point of inval (anchor)
//                    USHORT      ulSrcEnd,   // end seg of region to be aligne
//                    USHORT      ulTgtStart, //start of region to be aligned
//                    USHORT      usTgtMid,   // middle point (anchor)
//                    USHORT      ulTgtEnd    //end of region to be aligned
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     calc start & end index in alignment table to be changed
//                    find parallel region
//                    delete all added lf's in parallel region
//                    if start/end is user anchor, exclude it from aligning
//                    split segments joined because of 2:1 / 1:2
//                    if no middle point
//                       realign invalidated area
//                    else
//                       realign start to mid and mid to end
//                    endif
//                    parse align structure
//                    join segments for 2:1 / 1:2 matches
//                    fill visdoc alignment from alignment table
//                    add lf's needed for parallelization in parallel region
//          mark: start and end anchor are excluded from area
//------------------------------------------------------------------------------

VOID VisAlignBlock
(
  PITMIDA     pITMIda,
  ULONG       ulSrcStart,
  ULONG       ulSrcMid,
  ULONG       ulSrcEnd,
  ULONG       ulTgtStart,
  ULONG       ulTgtMid,
  ULONG       ulTgtEnd
)
{
  BOOL        fOK = TRUE;
  ULONG       ulFillStart;
  ULONG       ulFillEnd;
  ULONG       ulActSrc;
  ULONG       ulActTgt;
  PTBSEGMENT  pSeg;
  ULONG       ulParSrcStart = 0;
  ULONG       ulParTgtStart = 0;
  ULONG       ulParSrcEnd = 0;
  ULONG       ulParTgtEnd = 0;

   /*******************************************************************/
   /* calculate start and end index of invalidated area in alignment  */
   /* structure                                                       */
   /*******************************************************************/
   CalcInvalIndices ( pITMIda, &ulFillStart, &ulFillEnd,
                      ulSrcStart, ulSrcEnd, ulTgtStart, ulTgtEnd );
  /********************************************************************/
  /* new: exclude all anchors from invalid area!                      */
  /********************************************************************/
  FindParallelRegion (pITMIda, ulSrcStart, ulSrcEnd,
                      &ulParSrcStart, &ulParSrcEnd,
                      &ulParTgtStart, &ulParTgtEnd);
  /********************************************************************/
  /* delete added LF's due to make segs parallel                      */
  /********************************************************************/
  ITMDelLF (pITMIda, ulParSrcStart, ulParSrcEnd,
                     ulParTgtStart, ulParTgtEnd );
  /*******************************************************************/
  /* undo alignstruct-parsing and subsequent segment joining         */
  /* after this the pusNumAligned-indices are not valid any more!!   */
  /*******************************************************************/
  BuildSplitSeg(pITMIda, &ulFillStart, &ulFillEnd);

  pITMIda->Aligned.ulFillStart = ulFillStart;
  pITMIda->Aligned.ulFillIndex = ulFillStart;
  pITMIda->Aligned.ulFillEnd = ulFillEnd;

#ifdef ITMTEST
 fOut      = fopen ( "ITMSTAT.OUT", "a" );
  fprintf (fOut, "After BuildSplitSeg, for realign  \n");
  for ( usTemp1=35;usTemp1 < (pITMIda->Aligned.ulUsed);usTemp1++ )
//for ( usTemp1=10;usTemp1 < 35;usTemp1++ )
  {
    fprintf( fOut,
             "%4d Seg: %4d %4d %4d \n",
              usTemp1,
              pITMIda->Aligned.pulSrc[usTemp1],
              pITMIda->Aligned.pulTgt1[usTemp1],
              (USHORT) pITMIda->Aligned.pbType[usTemp1]);
  } /* endfor */
   fclose( fOut      );
#endif
  /********************************************************************/
  /* remember 1st src/tgt segnum which is changed for VisDocFillIndex */
  /********************************************************************/
  ulActTgt = ulTgtStart + 1;
  pSeg = EQFBGetVisSeg(&(pITMIda->TBTargetDoc), &ulActTgt);
  ulActSrc = ulSrcStart + 1;
  pSeg = EQFBGetVisSeg(&(pITMIda->TBSourceDoc), &ulActSrc);

  if  (!ulSrcMid )
  {
    Realign ( pITMIda, ulSrcStart, ulSrcEnd, ulTgtStart, ulTgtEnd, USERFREE);
  }
  else
  {
    /*******************************************************************/
    /* realign 3 disjunkt areas (usSrcMid, usTgtMid is new anchor)     */
    /* use start segments                                              */
    /*******************************************************************/

    Realign (pITMIda, ulSrcStart, ulSrcMid, ulTgtStart, ulTgtMid, USERFILL );

    fOK = AddToAlignStruct(&pITMIda->Aligned, ulSrcMid,
                        ulTgtMid, ONE_ONE, 0, USERFILL  );

    Realign (pITMIda, ulSrcMid, ulSrcEnd, ulTgtMid, ulTgtEnd,     USERFREE );
  } /* endif */

  fOK = ParseAlignStruct(pITMIda, &pITMIda->Aligned);
  if ( fOK )
  {
    BuildJoinSeg(pITMIda);
  } /* endif */
  /******************************************************************/
  /* fill index reference in both visdoc structures                 */
  /* loop thru changed area in alignment struct                     */
  /******************************************************************/
  VisDocFillIndex (pITMIda, pITMIda->Aligned.ulFillStart,
                            ulActSrc, ulActTgt);

  ITMAdjustLF ( pITMIda, ulParSrcStart, ulParSrcEnd,
                         ulParTgtStart, ulParTgtEnd );

  /********************************************************************/
  /* indicates that user manipulated the alignment                    */
  /********************************************************************/
  pITMIda->stVisDocSrc.fChanged = TRUE;
  pITMIda->stVisDocTgt.fChanged = TRUE;
#ifdef ITMTEST
 fOut      = fopen ( "ITMSTAT.OUT", "a" );
  fprintf (fOut, "End of VisAlignBlock \n");
  for ( usTemp1=35;usTemp1 < (pITMIda->Aligned.ulUsed);usTemp1++ )
//  for ( usTemp1=10;usTemp1 < 35;usTemp1++ )
  {
    fprintf( fOut,
              "%4d Seg: %4d %4d %4d \n",
              usTemp1,
              pITMIda->Aligned.pulSrc[usTemp1],
              pITMIda->Aligned.pulTgt1[usTemp1],
              (USHORT) pITMIda->Aligned.pbType[usTemp1]);
  } /* endfor */
  fclose( fOut      );
#endif
} /* end of function VisAlignBlock */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     VisInvalRegion
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       realign and readjust invalidated region
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pITMIda,
//                    PITMVISDOC   pVisDoc,
//                    USHORT       ulSegNum
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     find next previous anchor in both visdocs
//                    call VisAlignBlock to align the area
//                    set qstatus again
//                    count status bar variables
//------------------------------------------------------------------------------

VOID
VisInvalRegion
(
  PITMIDA      pITMIda,
  PITMVISDOC   pVisDoc,
  ULONG        ulSegNum
)
{
  ULONG        ulSrcStart;
  ULONG        ulSrcEnd;
  ULONG        ulTgtStart;
  ULONG        ulTgtEnd;

  /********************************************************************/
  /* guarantee that no seg has qStatus == QF_VISACT                   */
  /********************************************************************/
//VisActReset(&(pITMIda->stVisDocSrc),&(pITMIda->stVisDocTgt));

  /********************************************************************/
  /* find next/previous anchor: overcross anchors are not valid       */
  /********************************************************************/
  ulSrcStart = ulSegNum - 1;
  ulSrcEnd = ulSegNum + 1;
//  pSeg = EQFBGetPrevVisSeg(pVisDoc->pDoc, &ulSrcStart);
//  pSeg = EQFBGetVisSeg(pVisDoc->pDoc, &ulSrcEnd);
  ulSrcStart = FindNextAnchor(pVisDoc, ulSrcStart,PREVIOUS);
  ulSrcEnd = FindNextAnchor(pVisDoc, ulSrcEnd, NEXT);
  if ( !ulSrcEnd )                     //at end of file
  {
    ulSrcEnd = pVisDoc->pDoc->ulMaxSeg;
  } /* endif */

  if ( pVisDoc->pDoc->docType == VISSRC_DOC)
  {
    ulTgtStart = pVisDoc->pulAnchor[ulSrcStart];
    ulTgtEnd = pVisDoc->pulAnchor[ulSrcEnd];
  }
  else
  {
    ulTgtStart = ulSrcStart;
    ulTgtEnd = ulSrcEnd;
    ulSrcStart = pVisDoc->pulAnchor[ulSrcStart];
    ulSrcEnd = pVisDoc->pulAnchor[ulSrcEnd];
  } /* endif */

  VisAlignBlock ( pITMIda, ulSrcStart, 0, ulSrcEnd,
                           ulTgtStart, 0, ulTgtEnd );

  /********************************************************************/
  /* re-calc qstatus of all segments in changed area                  */
  /********************************************************************/
  VisSetQStatus( &(pITMIda->stVisDocSrc), ulSrcStart, ulSrcEnd );
  VisSetQStatus( &(pITMIda->stVisDocTgt), ulTgtStart, ulTgtEnd );

  ITMCountStatBar (pITMIda);
  /********************************************************************/
  /* adjust visactive segments if it is in invalidated region         */
  /* in invalidated region qStatus and alignment may be changed       */
  /********************************************************************/

} /* end of function VisInvalRegion */
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     VisJoinSeg
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       Join 2 segments
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc,
//                    USHORT      ulSegNum
//                    BOOL        fQuiet
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   success
//------------------------------------------------------------------------------
// Function flow:     copied from editor, but some parts deleted
//                    get both segments
//                    check if joined is not too long
//                    if nec, ask user to assure
//                    call VIsJoinSegData
//                    readjust the editing tables
//------------------------------------------------------------------------------
BOOL
VisJoinSeg
(
  PITMIDA     pIda,
  PITMVISDOC  pVisDoc,
  PTBDOCUMENT pDoc,
  ULONG       ulSegNum,
  BOOL        fQuiet
)
{
   PTBSEGMENT pTBStartSeg;                // pointer to segment
   PTBSEGMENT pTBJoinedSeg;               // segment to be joined
   BOOL       fOK = TRUE;                 // success indicator
   LONG       lLength;                   // length of the joined segments
   PEQFBBLOCK pstBlock = (PEQFBBLOCK) pDoc->pBlockMark;// block structure
   BOOL       fChanged;                          // temporary seg.changed flag
   ULONG      ulStartSeg;

//   ulSegNum = pDoc->TBCursor.ulSegNum;
   ulStartSeg = ulSegNum;
                                          // get first segment to be joined
   pTBStartSeg = EQFBGetSegW(pDoc, ulSegNum);
                                      // get the second one
   ulSegNum++;
   pTBJoinedSeg = EQFBGetVisSeg( pDoc, &ulSegNum );

   if (!pTBJoinedSeg || pTBJoinedSeg->ulSegNum == 0) //actseg is last seg
   {
     if ( !fQuiet )
     {
        WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if no next segment
     } /* endif */
     fOK = FALSE;
   }
   else
   {
                                      // check for length of joined segment
     pTBJoinedSeg = EQFBGetSegW( pDoc, ulSegNum ); // get real len(COMPACT!!)
     lLength = UTF16strlenCHAR(pTBStartSeg->pDataW) + UTF16strlenCHAR(pTBJoinedSeg->pDataW) + 1;

     if ( lLength >= MAX_SEGMENT_SIZE )
     {
       if ( !fQuiet )
       {
         ITMUtlError( pIda, TB_JOINEDSEGTOOLONG, MB_CANCEL,
                      0, NULL, EQF_WARNING);
       } /* endif */
       fOK = FALSE;
     }
     else
     {
      /****************************************************************/
      /* donot display msg what to join                               */
      /****************************************************************/
      if ( pIda->fParallel )                    //del LF's in both parts
      {
        ITMDocDelLF ( pDoc,pVisDoc, ulStartSeg, ulSegNum + 1);  // KBT0761: + 1
      } /* endif */

      fChanged = pDoc->EQFBFlags.workchng; // save workseg changed
      EQFBWorkSegOut( pDoc );       // reset current active segment
                                    // update the segm. identifications
      fOK = VisJoinSegData(pDoc,ulStartSeg,ulSegNum);
//      pDoc->usWorkSeg = ulStartSeg;                     //save doc seg file

      pDoc->EQFBFlags.workchng = (USHORT) fChanged; // restore workseg flag

      ulSegNum = ulStartSeg;
      // remove any remaining block mark
      if ( pstBlock && (pstBlock->pDoc == pDoc) &&
            (pstBlock->ulSegNum == ulSegNum) )
      {
         pstBlock->pDoc = NULL;             // reset block mark
      } /* endif */

      EQFBPhysCursorFromSeg(pDoc);          //position at SegOffset
      pDoc->lDBCSCursorCol =
                       pDoc->lCursorCol;      //store in DBCS column
     } /* endif */
   } /* endif */
   return (fOK);
} /* end of function VisJoinSeg */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     VisJoinSegData - join two segments
//------------------------------------------------------------------------------
// Function call:     VisJoinSegData( PTBDOCUMENT, USHORT, USHORT );
//
//------------------------------------------------------------------------------
// Description:       join two segments in the source and the target files
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance
//                    USHORT            segment number of join start
//                    USHORT            segment number of next segment
//
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE      join was successful
//                    FALSE     either memory shortage or other error
//
//------------------------------------------------------------------------------
// Function flow:     -  get the segment data for the first segment
//                    -  get the segment data for the segment to
//                       be joined
//                    -  determine the length of both
//                    -  allocate space and concatenate them together
//                    -  point to new start of segment and free the
//                       old space
//                    -  set the segment flags to indicate the status
//                       of a segment
//                    -  reparse source and target segment
//                    -  check if the tobe joined segment was already
//                       joined
//                       if so split the sources and reset the status
//                       JoinStart
//
//------------------------------------------------------------------------------

static BOOL
VisJoinSegData
(
   PTBDOCUMENT pDoc,                      // pointer to Document ida
   ULONG     ulSegNumStart,               // segment number
   ULONG     ulSegNum                           // segment number
)
{
   PTBSEGMENT pTBStartTgt;                // pointer to segment
   PTBSEGMENT pTBJoinedTgt;               // segment to be joined
   PSZ_W      pDataTgt;                   // pointer to data
   LONG       lTgtLength;                // length of the tgt.segment
   LONG       lTgtJoinLength;            // length of to be joined tgt segment
   BOOL       fOK;                        // success indicator

                                          // join the data in the source file
   pTBStartTgt  = EQFBGetSegW( pDoc, ulSegNumStart );
   pTBJoinedTgt = EQFBGetSegW( pDoc, ulSegNum );

   lTgtJoinLength = UTF16strlenCHAR(pTBJoinedTgt->pDataW);
   lTgtLength     = UTF16strlenCHAR(pTBStartTgt->pDataW) + lTgtJoinLength + 1;

   fOK = UtlAlloc( (PVOID *) &pDataTgt, 0L,
                   (LONG) max( lTgtLength * sizeof(CHAR_W), MIN_ALLOC ) , ERROR_STORAGE);

   if ( fOK )
   {
      UTF16strcpy( pDataTgt,pTBStartTgt->pDataW);  // get target data for seg start
      UTF16strcat( pDataTgt,pTBJoinedTgt->pDataW); // ...  for seg joined

      UtlAlloc( (PVOID *) &(pTBStartTgt->pDataW), 0L, 0L, NOMSG);
      pTBStartTgt->pDataW = pDataTgt;
                                          // update the segm. identifications
      pTBStartTgt->SegFlags.JoinStart = TRUE;
      pTBJoinedTgt->SegFlags.Joined = TRUE;
                                          // or with mark identification
      pTBStartTgt->SegFlags.Marked |= pTBJoinedTgt->SegFlags.Marked;

      // set modified flag (just in case something was changed)...
      pTBJoinedTgt->SegFlags.Typed  = TRUE ;
      pTBStartTgt->SegFlags.Typed  = TRUE ;

      EQFBCompSeg( pTBStartTgt );         // reparse source and tgt segment

      pDoc->flags.changed = TRUE;         // target document changed..

      // check if the tobe joined segment was already joined
      // if so split the sources and reset the status JoinStart

      if ( pTBJoinedTgt->SegFlags.JoinStart ) // split the prev. joined segs
      {
         do                                  // at least one joined seg avail
         {
            ulSegNum++;                      // point to next segment
            pTBStartTgt  = EQFBGetSegW( pDoc, ulSegNum );
            if ( pTBStartTgt && pTBStartTgt->SegFlags.Joined )
            {
               lTgtJoinLength -= UTF16strlenCHAR( pTBStartTgt->pDataW );
            } /* endif */
         } while ( pTBStartTgt && pTBStartTgt->SegFlags.Joined ); /* enddo */

         *(pTBJoinedTgt->pDataW + lTgtJoinLength) = EOS ;
         pTBJoinedTgt->SegFlags.JoinStart = FALSE;
         EQFBCompSeg( pTBJoinedTgt );        // reparse source segment
      } /* endif */
   } /* endif */

   return ( fOK );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     VisSplitSeg - split one segment
//------------------------------------------------------------------------------
// Function call:     VisSplitSeg( PTBDOCUMENT );
//------------------------------------------------------------------------------
// Description:       split previously joined segments
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT     pointer to document instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if reflow allowed:
//                    - if segment is not active or not joined then
//                         cancel request and issue a warning
//                      else:
//                         - split at last deactivated segment boundery found
//                         - display a box with 2 seg.for user reassurance
//                         - if user indicates OK then
//                              - split segment data
//                              -  if ok, save documents(ssource and starget)
//                              - send 2nd 'short' segment to services
//                              - activate this segment
//                              - save seg in TM for consistancy between
//                                TM and file
//                            endif
//                      endif
//                    endif
//
//------------------------------------------------------------------------------
BOOL
VisSplitSeg
(
  PITMIDA     pIda,
  PITMVISDOC  pVisDoc,
  PTBDOCUMENT pDoc,
  ULONG       ulStartSeg,
  BOOL        fQuiet
)
{
   PSZ_W pSeg[3];                        // pointer array to segments
   PTBSEGMENT pTBSeg;                    // pointer to segment
   PTBSEGMENT pTBFirstSeg;               // pointer to first segment
   ULONG      ulJ;                       // index
   USHORT     usMBId;                    // return code from UtlError
   ULONG      ulSegNum;                  // segment number
   BOOL       fOK = TRUE;                // success indicator
   PEQFBBLOCK  pstBlock = (PEQFBBLOCK) pDoc->pBlockMark;     // block structure
   LONG       lLenSeg1;                 // len of 1st twin seg w/o joined
   LONG       lLenSeg2;                 // len of twin( seg2+..+lastsegjoined)
   LONG       lLength;                  // length of segment


   ulSegNum = ulStartSeg;
   pTBFirstSeg = pTBSeg = EQFBGetSegW(pDoc, ulSegNum);

   if (!pTBSeg->SegFlags.JoinStart )
   {
      if ( !fQuiet )
      {
        ITMUtlError( pIda, TB_NOSEGJOINED, MB_CANCEL, 0, NULL, EQF_WARNING);
      } /* endif */
   }
   else
   {
      if (pDoc->fAutoLineWrap && pDoc->fLineWrap )
      {
        EQFBSoftLFRemove(pDoc);
      } /* endif */

      ulJ = ulSegNum + 1;
      pTBSeg = EQFBGetSegW(pDoc, ulJ);
      if ( pIda->fParallel )
      {
        ITMDocDelLF ( pDoc,pVisDoc, ulStartSeg, ulJ);
      } /* endif */
      /*************************************************************/
      /*calc original length of this segment when it was not joined*/
      /*************************************************************/

      lLenSeg1 = UTF16strlenCHAR(pTBFirstSeg->pDataW);

      /*************************************************************/
      /* scan thru joined segments and subtract the lengths        */
      /*************************************************************/
      lLenSeg2 = 0;
      while (pTBSeg && pTBSeg->SegFlags.Joined)
      {
        lLenSeg2 += UTF16strlenCHAR(pTBSeg->pDataW);
        ulJ++;
        pTBSeg = EQFBGetSegW(pDoc, ulJ);
      } /* endwhile */
      lLenSeg1 -= lLenSeg2;
      if ( !fQuiet )
      {
        /*************************************************************/
        /* fill strings to display message                           */
        /*************************************************************/
        lLength = min(MSGBOXDATALEN, lLenSeg1);
        UTF16strncpy( &chSeg1[0], pTBFirstSeg->pDataW, lLength);
        chSeg1[ lLength ] = EOS;   // set end of string
        if ( UTF16strlenCHAR(chSeg1) >= MSGBOXDATALEN )
        {                                // fill last 3 chars with '...'
           UTF16strcpy(chSeg1+MSGBOXDATALEN-4, L"...");
        } /* endif */

        pSeg[0] = chSeg1;
                                         // limit second segment
        lLength = min(MSGBOXDATALEN, lLenSeg2);
        UTF16strncpy( chSeg2, pTBFirstSeg->pDataW+lLenSeg1, lLength);
        chSeg2[ lLength ] = EOS;   // set end of string
        if ( UTF16strlenCHAR(chSeg2) >= MSGBOXDATALEN )
        {                                // fill last 3 chars with '...'
           UTF16strcpy(chSeg2+MSGBOXDATALEN-4, L"...");
        } /* endif */
        pSeg[1] = chSeg2;

        // display message for Split segment message box
        usMBId = ITMUtlErrorW( pIda, TB_JOINEDSEG, MB_YESNO | MB_DEFBUTTON2,
                              2, &pSeg[0], EQF_QUERY);
      }
      else
      {
        usMBId = MBID_YES;
      } /* endif */
      if ( usMBId == MBID_YES )                 // split the segments
      {
         EQFBWorkSegOut( pDoc );              // save the current segment
         fOK = VisSplitSegData( pDoc, ulSegNum, (ulJ-1),
                                 lLenSeg1, lLenSeg2 );
         ulSegNum = ulStartSeg;
         // remove any remaining block mark
         if ( pstBlock && (pstBlock->pDoc == pDoc) &&
               (pstBlock->ulSegNum == ulSegNum) )
         {
            pstBlock->pDoc = NULL;             // reset block mark
         } /* endif */

         if (pDoc->fAutoLineWrap && pDoc->fLineWrap )
         {
           EQFBSoftLFInsert(pDoc);
         } /* endif */
      }
      else
      {
        fOK = FALSE;
      } /* endif */
   } /* endif */
   return (fOK);
}
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     VisSplitSegData - split joined segments
//------------------------------------------------------------------------------
// Function call:     VisSplitSegData( PTBDOCUMENT, USHORT, USHORT
//                                        USHORT, USHORT );
//------------------------------------------------------------------------------
// Description:       split a previously joined segment in the
//                    source and the target files
//
//                    Logik:
//                     Suppose Seg1, Seg2 and seg3 are joined and should be
//                     splitted:
//                    Before:
//                      SegNum   Joined data         Flag
//                        Seg1   Seg1+Seg2+Seg3    JoinStart
//                        Seg2   Seg2              Joined
//                        Seg3   Seg3              Joined
//
//                    After Split:         (OLd)
//                      SegNum   Joined data         Flag
//                        Seg1   Seg1+Seg2         JoinStart
//                        Seg2   Seg2              Joined
//                        Seg3   Seg3                 -
//
//                    New Split :     (July 93)
//                      SegNum   Joined data      Flag       Length
//                        Seg1   Seg1              -        lLenSeg1
//                        Seg2   Seg2 +Seg3     JoinStart   lLenSeg2
//                        Seg3   Seg3            Joined
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance
//                    USHORT            segment number to split
//                    USHORT            segment number of last segment
//                    USHORT            length of twin seg1
//                    USHORT            len of twin ( seg2+seg3)
//
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       split segment successful
//                    FALSE      error happened
//
//------------------------------------------------------------------------------
// Function flow:     - get segments pointers for source and target
//                      for start segment and the last
//                      attached(joined) segment
//                    - determine length of remaining part
//                    - allocate buffer for new segments
//                    - if okay then
//                        reset the status flags for last attached
//                        (joined) segment
//                        copy the changed data into source and
//                         target segment
//                        reparse the source and target segment
//                        indicate that source and target document
//                         are changed
//                      endif
//
//------------------------------------------------------------------------------

static BOOL
VisSplitSegData
(
   PTBDOCUMENT pDoc,                      // pointer to Document ida
   ULONG       ulSegNum,                  // segment number
   ULONG       ulLastSegJoined,           // segment number of last joined segm
   LONG        lLenSeg1,                 // len of new seg 1 twin
   LONG        lLenSeg2                  // len of remaining joined twin segs
)
{
   PTBSEGMENT pTBStartTgt;                // pointer to segment
   PTBSEGMENT pTBStartTgt2;               // pointer to joined segment
   PSZ_W      pDataTgt;                   // pointer to data
   PSZ_W      pDataTgt2;                  // pointer to data
   BOOL       fOK = TRUE;                 // success indicator

   pTBStartTgt = EQFBGetSegW( pDoc, ulSegNum );
   pTBStartTgt2 = EQFBGetSegW( pDoc, (ulSegNum +1));

   /*******************************************************************/
   /* change Seg2  to Seg2 + Seg3                                     */
   /* only nec if a joined segment remains                            */
   /* Tgt is untranslated after split segment                         */
   /*******************************************************************/

   if ( UtlAlloc( (PVOID *) &pDataTgt2, 0L,
                  (LONG) max((lLenSeg2+1)*sizeof(CHAR_W), MIN_ALLOC), ERROR_STORAGE)
     && UtlAlloc( (PVOID *) &pDataTgt, 0L,
                  (LONG) max((lLenSeg1+1)*sizeof(CHAR_W), MIN_ALLOC), ERROR_STORAGE) )
   {

      memcpy( pDataTgt2,pTBStartTgt->pDataW+lLenSeg1, lLenSeg2*sizeof(CHAR_W));
      *(pDataTgt2+lLenSeg2) = EOS;

      memcpy( pDataTgt,pTBStartTgt->pDataW, lLenSeg1*sizeof(CHAR_W));
      *(pDataTgt+lLenSeg1) = EOS;

      UtlAlloc( (PVOID *) &(pTBStartTgt2->pDataW), 0L, 0L, NOMSG);
      pTBStartTgt2->pDataW = pDataTgt2;

      UtlAlloc( (PVOID *) &(pTBStartTgt->pDataW), 0L, 0L, NOMSG);
      pTBStartTgt->pDataW = pDataTgt;

      /**************************************************************/
      /* indicate 1st seg is not joined any more                    */
      /* 2nd is now the start of the  joined segs                   */
      /* if there remain joined segs                                */
      /**************************************************************/
      pTBStartTgt->SegFlags.JoinStart = FALSE;

      pTBStartTgt2->SegFlags.Joined = FALSE;
      if ( ulLastSegJoined > ulSegNum + 1 )
      {
        pTBStartTgt2->SegFlags.JoinStart = TRUE;
      } /* endif */

      pTBStartTgt->SegFlags.Current = TRUE;

      EQFBCompSeg( pTBStartTgt );         // reparse source and tgt segment

      EQFBCompSeg( pTBStartTgt2 );

      pTBStartTgt2->SegFlags.Typed  = FALSE ;
      pTBStartTgt->SegFlags.Typed  = FALSE ;

   }
   else
   {
     fOK = FALSE;                         // allocation went wrong
   } /* endif */

   return ( fOK );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncJoinSeg(pITMIda, pVisDoc)
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       Join 2 segments on user request
//------------------------------------------------------------------------------
// Parameters:        PITMIDA     pIda,
//                    PITMVISDOC  pVisDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     get cursor segment
//                    reset active alignment
//                    join segments (ask user to assure)
//                    if ok
//                      if segment is crossedout , delete the crossout
//                      set userjoin flag
//                      if segment was anchor, delete anchor
//                      realign region
//                      actiavte alignment
//                      refresh screen
//------------------------------------------------------------------------------

VOID
ITMFuncJoinSeg
(
  PITMIDA     pIda,
  PITMVISDOC  pVisDoc
)
{
  PTBDOCUMENT pDoc;
  ULONG       ulSegNum;
  PTBSEGMENT  pJoinedSeg;
  PTBSEGMENT  pSeg;
  BOOL        fOK;
  ULONG       ulJoinedSeg;
  ULONG       ulAnchorSeg;
  ULONG       ulAnchorJoinedSeg;

  SETCURSOR( SPTR_WAIT );
  pDoc = pVisDoc->pDoc;
  ulSegNum = pDoc->TBCursor.ulSegNum;
  VisActReset(&(pIda->stVisDocSrc),&(pIda->stVisDocTgt));
  /******************************************************************/
  /* check for overlapping with anchor                              */
  /******************************************************************/
  ulJoinedSeg = ulSegNum + 1;
  pJoinedSeg = EQFBGetVisSeg(pDoc, &ulJoinedSeg);
  ulAnchorSeg = pVisDoc->pulAnchor[ulSegNum];
  ulAnchorJoinedSeg = pVisDoc->pulAnchor[ulJoinedSeg];
  pSeg = EQFBGetSegW(pDoc, ulSegNum);
/**********************************************************************/
/* fQuiet = FALSE:          send msg if seg will become too long      */
/**********************************************************************/
  fOK = VisJoinSeg ( pIda, pVisDoc, pDoc, ulSegNum, FALSE );
  if ( pIda->fParallel )
  {
    ITMDocCountLF ( pDoc, pVisDoc, ulSegNum, ulSegNum );
  } /* endif */
  if ( fOK )                                       // segs are joined
  {
    /********************************************************************/
    /* if a segment is crossed-out, del the cross out                   */
    /********************************************************************/
    if ( pVisDoc->pVisState[ulSegNum].CrossedOut )
    {
      pVisDoc->pVisState[ulSegNum].CrossedOut = FALSE;
      pSeg->qStatus = QF_TOBE;
    } /* endif */
    if ( pVisDoc->pVisState[ulJoinedSeg].CrossedOut )
    {
      pVisDoc->pVisState[ulJoinedSeg].CrossedOut = FALSE;
      pJoinedSeg->qStatus = QF_TOBE;
    } /* endif */

    pVisDoc->pVisState[ulSegNum].UserJoin = TRUE;
    pVisDoc->pVisState[ulSegNum].UserSplit = FALSE;
    /******************************************************************/
    /* if joined seg overlaps an anchor, delete the anchor            */
    /* delete 1st anchor in joined segment, because if ulAnchorSeg    */
    /* also exist, the ITMAdjustLF during that DelAnchor loops ...    */
    /******************************************************************/
    if ( ulAnchorJoinedSeg )
    {
      if ( pDoc->docType == VISSRC_DOC )
      {
        ITMDelAnchor (pIda, &(pIda->stVisDocSrc),&(pIda->stVisDocTgt),
                             ulJoinedSeg, ulAnchorJoinedSeg);
      }
      else
      {
        ITMDelAnchor (pIda, &(pIda->stVisDocSrc),&(pIda->stVisDocTgt),
                             ulAnchorJoinedSeg, ulJoinedSeg);
      } /* endif */
    } /* endif */
    if (ulAnchorSeg)
    {
      if ( pDoc->docType == VISSRC_DOC )
      {
        ITMDelAnchor (pIda, &(pIda->stVisDocSrc),&(pIda->stVisDocTgt),
                             ulSegNum, ulAnchorSeg);
      }
      else
      {
        ITMDelAnchor (pIda, &(pIda->stVisDocSrc),&(pIda->stVisDocTgt),
                             ulAnchorSeg, ulSegNum);
      } /* endif */
    } /* endif */

    /********************************************************************/
    /* invalidate region and realign                                    */
    /********************************************************************/
    ulSegNum = pDoc->TBCursor.ulSegNum;
    VisInvalRegion(pIda, pVisDoc, ulSegNum);


  } /* endif */
  /******************************************************************/
  /* do the following whether join was succesfull or not            */
  /* set active alignment next to joined      segment               */
  /******************************************************************/
  ulSegNum = pDoc->TBCursor.ulSegNum;
  VisActivateSeg(pIda, pVisDoc, CURRENT, FALSE, ulSegNum);

  pIda->TBSourceDoc.Redraw |= REDRAW_ALL;                  // redraw the line
  pIda->TBTargetDoc.Redraw |= REDRAW_ALL;
  SETCURSOR( SPTR_ARROW );
  EQFBRefreshScreen( &(pIda->TBSourceDoc) );     // refresh the screen
  EQFBRefreshScreen( &(pIda->TBTargetDoc) );             // refresh the screen

} /* end of function ITMFuncJoinSeg */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncSplitSeg(pITMIda, pVisDoc)
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       SPLIT2 segments on user request
//------------------------------------------------------------------------------
// Parameters:        PITMIDA     pITMIda,
//                    PITMVISDOC  pVisDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     reset active alignment
//                    split segment
//                    set usersplit
//                    if segment was crossed-out, reset crossout
//                    if segment was anchor, delete anchor
//                    align again
//                    activate segment
//                    refresh screen
//------------------------------------------------------------------------------

VOID
ITMFuncSplitSeg
(
  PITMIDA     pITMIda,
  PITMVISDOC  pVisDoc
)
{
  PTBDOCUMENT pDoc;
  ULONG       ulSegNum;
  BOOL        fOK;
  ULONG       ulJoinedSeg;
  ULONG       ulAnchorSeg;
  ULONG       ulAnchorJoinedSeg;
  PTBSEGMENT  pJoinedSeg;
  PTBSEGMENT  pSeg;

  SETCURSOR( SPTR_WAIT );
  pDoc = pVisDoc->pDoc;
  ulSegNum = pDoc->TBCursor.ulSegNum;
  VisActReset(&(pITMIda->stVisDocSrc),&(pITMIda->stVisDocTgt));
  ulJoinedSeg = ulSegNum + 1;
  pJoinedSeg = EQFBGetSegW(pDoc, ulJoinedSeg);

  ulAnchorSeg = pVisDoc->pulAnchor[ulSegNum];
  ulAnchorJoinedSeg = pVisDoc->pulAnchor[ulJoinedSeg];
  pSeg = EQFBGetSegW(pDoc, ulSegNum);


  fOK = VisSplitSeg ( pITMIda, pVisDoc, pDoc, pDoc->TBCursor.ulSegNum, TRUE );

  if ( pITMIda->fParallel )
  {
    ITMDocCountLF ( pDoc, pVisDoc, ulSegNum, ulJoinedSeg );
  } /* endif */

  if ( fOK )                                    //if segs are split...
  {
    pVisDoc->pVisState[ulSegNum].UserSplit = TRUE;
    pVisDoc->pVisState[ulSegNum].UserJoin = FALSE;

    /********************************************************************/
    /* if a segment is crossed-out, del the cross out                   */
    /********************************************************************/
    if ( pVisDoc->pVisState[ulSegNum].CrossedOut )
    {
      pVisDoc->pVisState[ulSegNum].CrossedOut = FALSE;
      pSeg->qStatus = QF_TOBE;
    } /* endif */
    if ( pVisDoc->pVisState[ulJoinedSeg].CrossedOut )
    {
      pVisDoc->pVisState[ulJoinedSeg].CrossedOut = FALSE;
      pJoinedSeg->qStatus = QF_TOBE;
    } /* endif */
    /******************************************************************/
    /* if splitted segments overlap an anchor, delete anchor          */
    /******************************************************************/

    if (ulAnchorSeg)
    {
      if ( pVisDoc->pDoc->docType == VISSRC_DOC)
      {
        ITMDelAnchor (pITMIda, &(pITMIda->stVisDocSrc),&(pITMIda->stVisDocTgt),
                           ulSegNum, ulAnchorSeg);
      }
      else
      {
        ITMDelAnchor (pITMIda, &(pITMIda->stVisDocSrc),&(pITMIda->stVisDocTgt),
                           ulAnchorSeg, ulSegNum);
      } /* endif */
    } /* endif */
    if ( ulAnchorJoinedSeg )
    {
      if ( pVisDoc->pDoc->docType == VISSRC_DOC)
      {
        ITMDelAnchor (pITMIda, &(pITMIda->stVisDocSrc),&(pITMIda->stVisDocTgt),
                           ulJoinedSeg, ulAnchorJoinedSeg);
      }
      else
      {
        ITMDelAnchor (pITMIda, &(pITMIda->stVisDocSrc),&(pITMIda->stVisDocTgt),
                           ulAnchorJoinedSeg, ulJoinedSeg);
      } /* endif */
    } /* endif */

  } /* endif */
  /********************************************************************/
  /* align invalidated area again; always nec because LF's are deleted*/
  /* whether or not VisSplit is ok                                    */
  /********************************************************************/
  VisInvalRegion(pITMIda, pVisDoc, ulSegNum);
  /******************************************************************/
  /* do this whether split is ok or not                             */
  /* set active alignment next to joined      segment               */
  /******************************************************************/
  VisActivateSeg(pITMIda, pVisDoc, CURRENT, FALSE, ulSegNum);

  pITMIda->TBSourceDoc.Redraw |= REDRAW_ALL;        // redraw the line
  pITMIda->TBTargetDoc.Redraw |= REDRAW_ALL;
  SETCURSOR( SPTR_ARROW );
  EQFBRefreshScreen( &(pITMIda->TBSourceDoc) );     // refresh the screen
  EQFBRefreshScreen( &(pITMIda->TBTargetDoc) );     // refresh the screen

} /* end of function ITMFuncSplitSeg() */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     CalcInvalIndices ( &ulFillStart, &ulFillEnd)
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       find start and end index in alignment structure between
//                    which the alignment is updated
//------------------------------------------------------------------------------
// Parameters:        PITMIDA     pITMIda,
//                    PUSHORT     pulFillStart
//                    PUSHORT     pulFillEnd,
//                    USHORT      ulSrcStart,
//                    USHORT      ulSrcEnd,
//                    USHORT      ulTgtStart,
//                    USHORT      ulTgtEnd
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     find starting index so that SrcStart/TgtStart is inside
//                    find ending index so that SrcEnd/TgtEnd is inside
//------------------------------------------------------------------------------
static VOID
CalcInvalIndices
(
  PITMIDA     pITMIda,
  PULONG      pulFillStart,
  PULONG      pulFillEnd,
  ULONG       ulSrcStart,
  ULONG       ulSrcEnd,
  ULONG       ulTgtStart,
  ULONG       ulTgtEnd
)
{
  ULONG       ulFillStart;
  ULONG       ulFillEnd;
  ULONG       ulFillTemp;
  ULONG       ulI;
  ULONG       ulMaxAlign;

  /********************************************************************/
  /* to garantuee that ulI stops at end of alignment struct           */
  /********************************************************************/
  ulMaxAlign = pITMIda->Aligned.ulUsed;

  /********************************************************************/
  /* the anchor ulSrcStart/ulSrcEnd and ulTgtStart/ulTgtEnd           */
  /* are     excluded                                                 */
  /********************************************************************/
  ulI = 0;
  while ( (ulI < ulMaxAlign) && (pITMIda->Aligned.pulSrc[ulI] <= ulSrcStart ))
  {
    ulI++;
  } /* endwhile */
  ulFillStart = ulI;
  while ((ulI < ulMaxAlign) && (pITMIda->Aligned.pulSrc[ulI] < ulSrcEnd ))
  {
    ulI++;
  } /* endwhile */
  ulFillEnd = ulI - 1;

  ulI = 0;
  while ((ulI < ulMaxAlign) && (pITMIda->Aligned.pulTgt1[ulI] <= ulTgtStart ))
  {
    ulI++;
  } /* endwhile */
  ulFillTemp = ulI;
  if ( ulFillTemp < ulFillStart )
  {
    ulFillStart = ulFillTemp;
  } /* endif */
  while ((ulI < ulMaxAlign) && (pITMIda->Aligned.pulTgt1[ulI] < ulTgtEnd ))
  {
    ulI++;
  } /* endwhile */
  ulFillTemp = ulI - 1;
  if ( ulFillTemp < ulFillEnd )
  {
    ulFillEnd = ulFillTemp;
  } /* endif */
/****************************************************************/
/* if                                                           */
/* anchor is nop and hence no alignment, search for next align. */
/****************************************************************/
  *pulFillStart = ulFillStart;
  *pulFillEnd = ulFillEnd;
} /* end of function CalcInvalIndices ( &ulFillStart, &ulFillEnd) */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     Realign
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       realign from srcstart/tgtstart til srcEnd/TgtEnd
//                    (this is one segment block)
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda,
//                    USHORT     ulSrcStart,
//                    USHORT     ulSrcEnd,
//                    USHORT     ulTgtStart,
//                    USHORT     ulTgtEnd,
//                    SHORT      sFillType
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     init paragraph variables
//                    fill src segment block structure
//                    fill tgt segment block structure
//                    align both paragraphs
//                    fill alignment structure with new aligments
//------------------------------------------------------------------------------
static VOID
Realign
(
  PITMIDA    pITMIda,
  ULONG      ulSrcStart,
  ULONG      ulSrcEnd,
  ULONG      ulTgtStart,
  ULONG      ulTgtEnd,
  SHORT      sFillType
)
{
  LONG       lLen = 0;
  BOOL       fOK;
  SHORT      sNum = 0;
  ULONG      ulStart = 0;
  ULONG      ulEnd = 0;
  PTBSEGMENT pSeg;
   /********************************************************************/
   /* init used elements                                               */
   /********************************************************************/
   lLen = sizeof(SHORT) * pITMIda->itmSrcText.ulUsed;
   memset(pITMIda->itmSrcText.psPara, 0, lLen );
   lLen = sizeof(ULONG) * pITMIda->itmSrcText.ulUsed;
   memset(pITMIda->itmSrcText.pulSegStart, 0, lLen );
   memset(pITMIda->itmSrcText.pulSegEnd, 0, lLen );
   pITMIda->itmSrcText.ulUsed = 0;
   lLen = sizeof(SHORT) * pITMIda->itmTgtText.ulUsed;
   memset(pITMIda->itmTgtText.psPara, 0, lLen );
   lLen = sizeof(ULONG) * pITMIda->itmTgtText.ulUsed;
   memset(pITMIda->itmTgtText.pulSegStart, 0, lLen );
   memset(pITMIda->itmTgtText.pulSegEnd, 0, lLen );
   pITMIda->itmTgtText.ulUsed = 0;

   /*******************************************************************/
   /* exclude starts and ends                                         */
   /*******************************************************************/
   ulStart = ulSrcStart + 1;
   pSeg = EQFBGetVisSeg(&(pITMIda->TBSourceDoc), &ulStart);
   ulEnd = ulSrcEnd - 1 ;
   pSeg = EQFBGetPrevVisSeg(&(pITMIda->TBSourceDoc), &ulEnd);
   /********************************************************************/
   /* now fill the blocks ...                                          */
   /********************************************************************/
   fOK = GetSegmentBlock( pITMIda, &(pITMIda->TBSourceDoc),
                          &(pITMIda->itmSrcText),
                          ulStart, ulEnd);
   if ( fOK )
   {
     ulStart = ulTgtStart + 1;
     pSeg = EQFBGetVisSeg(&(pITMIda->TBTargetDoc), &ulStart);
     ulEnd = ulTgtEnd - 1;
     pSeg = EQFBGetPrevVisSeg(&(pITMIda->TBTargetDoc), &ulEnd);
     fOK = GetSegmentBlock( pITMIda, &(pITMIda->TBTargetDoc),
                            &(pITMIda->itmTgtText),
                            ulStart, ulEnd);
   } /* endif */
   /*********************************************************************/
   /*    prepare invalidated block                                      */
   /*********************************************************************/
   if ( fOK )
   {
     fOK = ITMSeqAlign( pITMIda->itmSrcText.psPara,
                       pITMIda->itmTgtText.psPara,
                       pITMIda->itmSrcText.ulUsed,
                       pITMIda->itmTgtText.ulUsed,
                       &pITMIda->pAlign, &sNum,
                       pITMIda->dbMean,
                       pITMIda->dbVar );
   } /* endif */

   /************************************************************/
   /* fill translation memory                                  */
   /************************************************************/
   if ( fOK && !pITMIda->fKill )
   {
    /******************************************************************/
    /* insert new alignments in alignment structure                   */
    /* evtl. auch hier gleich die reference (index) in visdoc         */
    /* anpassen                                                       */
    /******************************************************************/
    fOK = FillAlignStruct(pITMIda, pITMIda->pAlign, sNum, sFillType );
   }
   /**********************************************************/
   /* free the aligned resources                             */
   /**********************************************************/
   UtlAlloc( (PVOID *) &pITMIda->pAlign, 0L, 0L, NOMSG );

} /* end of function Realign */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMDelAnchor
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       delete an anchor (user request)
//------------------------------------------------------------------------------
// Parameters:        PITMIDA     pITMIda,
//                    PITMVISDOC  pstVisSrc
//                    PITMVISDOC  pstVisTgt
//                    USHORT      ulSrcSeg,
//                    USHORT      ulTgtSeg
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     if anchor is overcross, reset QF_OVERCROSS
//                    if not overcross,
//                      invalidate region affected by the change
//                    else
//                      invalidate both regions affected by the change
//------------------------------------------------------------------------------
static VOID
ITMDelAnchor
(
  PITMIDA     pITMIda,
  PITMVISDOC  pstVisSrc,
  PITMVISDOC  pstVisTgt,
  ULONG       ulSrcSeg,
  ULONG       ulTgtSeg
)
{
  BOOL        fOverCross;
  PTBSEGMENT  pSeg;

  if ( pstVisSrc->pVisState[ulSrcSeg].OverCross )
  {
    fOverCross = TRUE;
    /******************************************************************/
    /* reset QF_OVERCROSS                                             */
    /******************************************************************/
    pSeg = EQFBGetSegW(pstVisSrc->pDoc, ulSrcSeg);
    pSeg->qStatus = QF_TOBE;
    pSeg = EQFBGetSegW(pstVisTgt->pDoc, ulTgtSeg);
    pSeg->qStatus = QF_TOBE;
  }
  else
  {
    fOverCross = FALSE;
  } /* endif */

  pstVisSrc->pulAnchor[ulSrcSeg] = 0;
  pstVisTgt->pulAnchor[ulTgtSeg] = 0;
  pstVisSrc->pVisState[ulSrcSeg].UserAnchor = FALSE;
  pstVisTgt->pVisState[ulTgtSeg].UserAnchor = FALSE;
  pstVisSrc->pVisState[ulSrcSeg].OverCross = FALSE;
  pstVisTgt->pVisState[ulTgtSeg].OverCross = FALSE;

  if ( !fOverCross )
  {
     VisInvalRegion(pITMIda, pstVisSrc, ulSrcSeg);
   }
   else
   {
     VisInvalRegion(pITMIda, pstVisSrc, ulSrcSeg);
     VisInvalRegion(pITMIda, pstVisTgt, ulTgtSeg);
   } /* endif */


} /* end of function ITMDelAnchor */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     FindParallelRegion
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       find region in which 'parallel' lf's must be adjusted
//                    after realignment
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pITMIda,
//                    ULONG        ulSrcStart,
//                    ULONG        ulSrcEnd,
//                    PULONG       pulParSrcStart,
//                    PULONG       pulParSrcEnd,
//                    PULONG       pulParTgtStart,
//                    PULONG       pulParTgtEnd
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     find alignment previous to invalarea
//                     this is the start of parallelregion
//                    find alignments next to invalarea
//                      this is the end of parallelregion
//------------------------------------------------------------------------------
static VOID
FindParallelRegion
(
  PITMIDA      pITMIda,
  ULONG        ulSrcStart,
  ULONG        ulSrcEnd,
  PULONG       pulParSrcStart,
  PULONG       pulParSrcEnd,
  PULONG       pulParTgtStart,
  PULONG       pulParTgtEnd
)
{
  ULONG        ulIndex;

  if ( pITMIda->fParallel )
  {
    /********************************************************************/
    /* find alignment previous to invalarea start                       */
    /********************************************************************/
    ulIndex = FindCurAlignIndex(&(pITMIda->stVisDocSrc), ulSrcStart, PREVIOUS);
    *pulParSrcStart = pITMIda->Aligned.pulSrc[ulIndex];
    *pulParTgtStart = pITMIda->Aligned.pulTgt1[ulIndex];

    /********************************************************************/
    /* find alignment next to invalregion end                           */
    /********************************************************************/
    ulIndex = FindCurAlignIndex(&(pITMIda->stVisDocSrc), ulSrcEnd, NEXT);
    *pulParSrcEnd = pITMIda->Aligned.pulSrc[ulIndex];
    *pulParTgtEnd = pITMIda->Aligned.pulTgt1[ulIndex];
  } /* endif */

} /* end of function FindParallelRegion  */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     VisDocSave
//------------------------------------------------------------------------------
// Function call:     VisDocSave( PTBDOCUMENT,PSZ, PITMVISDOC)
//------------------------------------------------------------------------------
// Description:       save the current document
//                    this function is a copy of EQFBDocSave;
//                    all parts which are not required are deleted
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc ptr to document instance
//                    PSZ pszFileName
//                    PITMVISDOC  pstVisDoc
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       sRc        return codes from EQFBFileWrite
//------------------------------------------------------------------------------
// Function flow:     write file to disk
//------------------------------------------------------------------------------

SHORT
VisDocSave
(
   PTBDOCUMENT pDoc,
   PSZ         pszFileName,
   PITMVISDOC  pstVisDoc
)
{
   SHORT       sRC = 0;                // function's return code
   DOCFLAGS    EQFBFlags;              // structure of flags
   ULONG       ulStartSeg, ulEndSeg;
   ULONG       ulSegNum;
   PTBSEGMENT  pSeg;

/**********************************************************************/
/* for tests: reset qstatus of total document                         */
/**********************************************************************/
   ulStartSeg = 1;
   ulEndSeg = pDoc->ulMaxSeg;

   ulSegNum =  ulStartSeg;
   pSeg = EQFBGetSegW(pDoc, ulSegNum);

   while ( pSeg && (ulSegNum <= ulEndSeg))
   {
     switch ( pSeg->qStatus )
     {
       case  QF_NOP:
       case  QF_NOP_ANCHOR_1:
       case  QF_NOP_ANCHOR_2:
       case  QF_NOP_ANCHOR_3:
       case  QF_CROSSED_OUT_NOP:
         pSeg->qStatus = QF_NOP;
         break;
       case  QF_CROSSED_OUT:
       case  QF_ANCHOR_1:
       case  QF_ANCHOR_2:
       case  QF_ANCHOR_3:
       case  QF_VALID_11_1:
       case  QF_VALID_11_2:
       case  QF_VALID_11_3:
       case  QF_VALID_10:
       case  QF_VALID_01:
       case  QF_OVERCROSS:
       default :
         pSeg->qStatus = QF_TOBE;
         break;
     } /* endswitch */
     ulSegNum++;
     pSeg = EQFBGetSegW(pDoc, ulSegNum);
   } /* endwhile */
   /*******************************************************************/
   /* QF_CURRENT does not exist in ITM                                */
   /* tbActSeg also does not exist                                    */
   /*******************************************************************/
   memcpy( &EQFBFlags, &pDoc->EQFBFlags, sizeof( DOCFLAGS ));

   if ( pDoc )
   {
     if ( pDoc->flags.changed )       // set by VisJoinSegData
     {
        sRC = EQFBFileWrite( pszFileName, pDoc );
     } /* endif */

     memcpy( &pDoc->EQFBFlags, &EQFBFlags, sizeof( DOCFLAGS ));

   } /* endif */

   /*******************************************************************/
   /* set qstatus again                                               */
   /*******************************************************************/
   VisSetQStatus( pstVisDoc, 0, ulEndSeg);

   return ( sRC );
}




/**********************************************************************/
/* Functions for keyboard handling...                                 */
/**********************************************************************/
MRESULT HandleITMWMCharEx
(
   HWND        hwnd,                                       // window handle
   WPARAM      mp1,
   LPARAM      mp2,
   USHORT      msg
)
{
  PTBDOCUMENT  pDoc;                 // pointer to doc ida

  USHORT usAction;                    // action status
  void (*function)( PITMVISDOC );     // and function to be processed
  MRESULT   mResult = FALSE;          // return value from window proc

  USHORT   usStatus;                  // current status
  USHORT   usFunction;                // current function ID
  BOOL     fDBCSOK = TRUE;
                                      // Pointer to instance variables
  PITMVISDOC pVisDoc = ACCESSWNDIDA(hwnd, PITMVISDOC);

  pDoc = pVisDoc->pDoc;

  if ( EQFBMapKey( msg, mp1, mp2, &usFunction, &pDoc->ucState, ITM_MAPKEY) )
  {
     if ( pDoc->ulOemCodePage && (msg == WM_CHAR))
     {
        pDoc->usChar      = (USHORT)mp1;

     }
     else
     {
       pDoc->usChar      = (USHORT)mp1;
       pDoc->usDBCS2Char = 0;
       if ((pDoc->ucState & ST_CTRL) && (pDoc->ucState & ST_SHIFT)
            && ('A' <= pDoc->usChar) && (pDoc->usChar <= 'Z') )
       {
         pDoc->usChar = pDoc->usChar + 'a' - 'A';
       } /* endif */
       fDBCSOK = TRUE;
     } /* endif */

    if (fDBCSOK )     // do not process half of DBCS character
    {
       usAction     = ITMFuncTable[usFunction].usAction;
       function     = ITMFuncTable[usFunction].function;
       if ( pDoc->docType == VISTGT_DOC)
       {
         /***************************************************************/
         /* toggle doctype temporarily (to allow modifications)         */
         /***************************************************************/
         pDoc->docType = STARGET_DOC;
         usStatus = EQFBCurrentState( pDoc );
         pDoc->docType = VISTGT_DOC;
       }
       else
       {
         usStatus = EQFBCurrentState( pDoc );
       } /* endif */

       if ((usStatus & usAction) == usAction )
       {
          (*function)( pVisDoc );           // execute the function
          /****************************************************/
          /* check if it is a function where pDoc will be     */
          /* freed and therefore is invalid furthermore       */
          /****************************************************/
          if (!((usFunction == QUIT_ITMFUNC)||(usFunction == FILE_ITMFUNC) )
               && pDoc->pSegTables )  // be sure noone else closed doc..
          {
            EQFBRefreshScreen( pDoc );  // refresh the screen
          } /* endif */
       }
       else
       {
          ITMFuncNothing( pVisDoc ); // ignore keystroke
       } /* endif */

       mResult = MRFROMSHORT(TRUE);// indicate message is processed
    } /* endif */
  }
  else
  {
    // pass on message to standard window proc for further processing
    mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
  } /* endif */
  return mResult;
}


VOID ITMFuncBackspace( PITMVISDOC pVisDoc)
{
  EQFBFuncBackspace( pVisDoc->pDoc );
  return;
}


VOID ITMFuncBacktab( PITMVISDOC pVisDoc)
{
  EQFBFuncBacktab( pVisDoc->pDoc );
  return;
}


VOID ITMFuncBottomDoc( PITMVISDOC pVisDoc)
{
  EQFBFuncBottomDoc( pVisDoc->pDoc );
  return;
}


VOID ITMFuncDeleteChar( PITMVISDOC pVisDoc)
{
  EQFBFuncDeleteChar( pVisDoc->pDoc );
  return;
}


VOID ITMFuncDown( PITMVISDOC pVisDoc)
{
  EQFBFuncDown( pVisDoc->pDoc );
  return;
}


VOID ITMFuncEndLine( PITMVISDOC pVisDoc)
{
  EQFBFuncEndLine( pVisDoc->pDoc );
  return;
}


VOID ITMFuncEndSeg( PITMVISDOC pVisDoc)
{
  EQFBFuncEndSeg( pVisDoc->pDoc );
  return;
}


VOID ITMFuncDoFile( PITMVISDOC pVisDoc)
{
  PITMIDA pITMIda = (PITMIDA) pVisDoc->pITMIda;
  ITMFuncSave(pITMIda, pITMIda->hwnd, pITMIda->hab);
  return;
}


VOID ITMFuncInsToggle( PITMVISDOC pVisDoc)
{
  EQFBFuncInsToggle( pVisDoc->pDoc );
  return;
}


VOID ITMFuncJoinLine( PITMVISDOC pVisDoc)
{
  EQFBFuncJoinLine( pVisDoc->pDoc );
  return;
}


VOID ITMFuncLeft( PITMVISDOC pVisDoc)
{
  EQFBFuncLeft( pVisDoc->pDoc );
  return;
}


VOID ITMFuncNextWord( PITMVISDOC pVisDoc)
{
  EQFBFuncNextWord( pVisDoc->pDoc );
  return;
}


VOID ITMFuncNothing( PITMVISDOC pVisDoc)
{
  EQFBFuncNothing( pVisDoc->pDoc );
  return;
}


VOID ITMFuncPageDown( PITMVISDOC pVisDoc)
{
  EQFBFuncPageDown( pVisDoc->pDoc );
  return;
}


VOID ITMFuncPageUp( PITMVISDOC pVisDoc)
{
  EQFBFuncPageUp( pVisDoc->pDoc );
  return;
}


VOID ITMFuncPrevWord( PITMVISDOC pVisDoc)
{
  EQFBFuncPrevWord( pVisDoc->pDoc );
  return;
}


VOID ITMFuncDoQuit( PITMVISDOC pVisDoc)
{

  PITMIDA pITMIda = (PITMIDA) pVisDoc->pITMIda;
  ITMFuncQuit(pITMIda, pITMIda->hwnd, pITMIda->hab);
  return;
}


VOID ITMFuncRight( PITMVISDOC pVisDoc)
{
  EQFBFuncRight( pVisDoc->pDoc );
  return;
}


VOID ITMFuncSave1( PITMVISDOC pVisDoc)
{
  ITMFuncContinue((PITMIDA) pVisDoc->pITMIda);
  return;
}


VOID ITMCursorScrollDown( PITMVISDOC pVisDoc)
{
  EQFBCursorScrollDown( pVisDoc->pDoc );
  return;
}


VOID ITMFuncScrollLeft( PITMVISDOC pVisDoc)
{
  EQFBFuncScrollLeft( pVisDoc->pDoc );
  return;
}


VOID ITMFuncScrollRight( PITMVISDOC pVisDoc)
{
  EQFBFuncScrollRight( pVisDoc->pDoc );
  return;
}


VOID ITMCursorScrollUp( PITMVISDOC  pVisDoc)
{
  EQFBCursorScrollUp( pVisDoc->pDoc );
  return;
}


VOID ITMFuncSplitLine( PITMVISDOC pVisDoc)
{
  EQFBFuncSplitLine( pVisDoc->pDoc );
  return;
}


VOID ITMFuncStartLine( PITMVISDOC pVisDoc)
{
  EQFBFuncStartLine( pVisDoc->pDoc );
  return;
}


VOID ITMFuncStartSeg( PITMVISDOC pVisDoc)
{
  EQFBFuncStartSeg( pVisDoc->pDoc );
  return;
}


VOID ITMFuncTab( PITMVISDOC pVisDoc)
{
  EQFBFuncTab( pVisDoc->pDoc );
  return;
}


VOID ITMFuncTopDoc( PITMVISDOC pVisDoc)
{
  EQFBFuncTopDoc( pVisDoc->pDoc );
  return;
}


VOID ITMFuncUndo( PITMVISDOC pVisDoc)
{
  if (pVisDoc->pDoc->docType == VISTGT_DOC )
  {
    pVisDoc->pDoc->docType = STARGET_DOC;
    EQFBFuncUndo( pVisDoc->pDoc );
    pVisDoc->pDoc->docType = VISTGT_DOC;
  } /* endif */
  return;
}


VOID ITMFuncUp( PITMVISDOC pVisDoc)
{
  EQFBFuncUp( pVisDoc->pDoc );
  return;
}


VOID ITMJoinSeg( PITMVISDOC pVisDoc)
{
  ITMFuncJoinSeg((PITMIDA) pVisDoc->pITMIda, pVisDoc);
  return;
}


VOID ITMSplitSeg( PITMVISDOC pVisDoc)
{
  ITMFuncSplitSeg((PITMIDA) pVisDoc->pITMIda, pVisDoc);
  return;
}


VOID ITMMark( PITMVISDOC pVisDoc)
{
  EQFBMark( pVisDoc->pDoc );
  return;
}


VOID ITMFindMark( PITMVISDOC pVisDoc)
{
  EQFBFindMark( pVisDoc->pDoc );
  return;
}


VOID ITMClearMark( PITMVISDOC pVisDoc)
{
  EQFBClearMark( pVisDoc->pDoc );
  return;
}


VOID ITMFuncFind( PITMVISDOC pVisDoc)
{
  EQFBFuncFind( pVisDoc->pDoc );
  return;
}


VOID ITMFuncCutToClip( PITMVISDOC pVisDoc)
{
  if (pVisDoc->pDoc->docType == VISTGT_DOC )
  {
    pVisDoc->pDoc->docType = STARGET_DOC;
    EQFBFuncCutToClip( pVisDoc->pDoc );
    pVisDoc->pDoc->docType = VISTGT_DOC;
  }
  else
  {
    EQFBFuncCutToClip( pVisDoc->pDoc );
  } /* endif */
  return;
}


VOID ITMFuncCopyToClip( PITMVISDOC pVisDoc)
{
  EQFBFuncCopyToClip( pVisDoc->pDoc );
  return;
}


VOID ITMFuncPasteFromClip( PITMVISDOC pVisDoc)
{
  if (pVisDoc->pDoc->docType == VISTGT_DOC )
  {
    pVisDoc->pDoc->docType = STARGET_DOC;
    EQFBFuncPasteFromClip( pVisDoc->pDoc );
    pVisDoc->pDoc->docType = VISTGT_DOC;
  }
  else
  {
    EQFBFuncPasteFromClip( pVisDoc->pDoc );
  } /* endif */
  return;
}


VOID ITMFuncMarkNextWord( PITMVISDOC pVisDoc)
{
  EQFBFuncMarkNextWord( pVisDoc->pDoc );
  return;
}


VOID ITMFuncMarkPrevWord( PITMVISDOC pVisDoc)
{
  EQFBFuncMarkPrevWord( pVisDoc->pDoc );
  return;
}


VOID ITMFuncFonts( PITMVISDOC pVisDoc)
{
  EQFBFuncFonts( pVisDoc->pDoc );
  return;
}


VOID ITMFuncChangeToShort( PITMVISDOC pVisDoc)
{
  ITMFuncStyle((PITMIDA) pVisDoc->pITMIda, DISP_SHORTEN);
  return;
}

VOID ITMFuncChangeToHide( PITMVISDOC pVisDoc)
{
  ITMFuncStyle((PITMIDA) pVisDoc->pITMIda, DISP_HIDE);
  return;
}

VOID ITMFuncChangeToProt( PITMVISDOC pVisDoc)
{
  ITMFuncStyle((PITMIDA) pVisDoc->pITMIda, DISP_PROTECTED);
  return;
}


VOID ITMFuncChangeToUnpro( PITMVISDOC pVisDoc)
{
  ITMFuncStyle((PITMIDA) pVisDoc->pITMIda, DISP_UNPROTECTED);
  return;
}


VOID ITMFuncNextLine( PITMVISDOC pVisDoc)
{
  EQFBFuncNextLine( pVisDoc->pDoc );
  return;
}


VOID ITMFuncCharacter( PITMVISDOC pVisDoc)
{
  EQFBFuncCharacter( pVisDoc->pDoc );
  return;
}


VOID ITMFuncMarkClear( PITMVISDOC pVisDoc)
{
  EQFBFuncMarkClear( pVisDoc->pDoc );
  return;
}


VOID ITMFuncMarkSegment( PITMVISDOC pVisDoc)
{
  EQFBFuncMarkSegment( pVisDoc->pDoc );
  return;
}


VOID ITMFuncMarkLeftCUA( PITMVISDOC pVisDoc)
{
  EQFBFuncMarkLeftCUA( pVisDoc->pDoc );
  return;
}


VOID ITMFuncMarkRightCUA( PITMVISDOC pVisDoc)
{
  EQFBFuncMarkRightCUA( pVisDoc->pDoc );
  return;
}


VOID ITMFuncMarkUpCUA( PITMVISDOC pVisDoc)
{
  EQFBFuncMarkUpCUA( pVisDoc->pDoc );
  return;
}


VOID ITMFuncMarkDownCUA( PITMVISDOC pVisDoc)
{
  EQFBFuncMarkDownCUA( pVisDoc->pDoc );
  return;
}


VOID ITMAnchorNextFunc( PITMVISDOC pVisDoc)
{
  ITMFuncNextAnchor((PITMIDA) pVisDoc->pITMIda, NEXT);
  EQFBScreenData( pVisDoc->pDoc->twin );          // display screen
  EQFBScreenCursor( pVisDoc->pDoc->twin );        // update cursor and sliders
  return;
}

VOID ITMAnchorPrevFunc( PITMVISDOC pVisDoc)
{
  ITMFuncNextAnchor((PITMIDA) pVisDoc->pITMIda, PREVIOUS);
  EQFBScreenData( pVisDoc->pDoc->twin );          // display screen
  EQFBScreenCursor( pVisDoc->pDoc->twin );        // update cursor and sliders
  return;
}


VOID ITMIrregularNextFunc( PITMVISDOC pVisDoc)
{
  ITMFuncNextIrregular((PITMIDA) pVisDoc->pITMIda, NEXT);
  EQFBScreenData( pVisDoc->pDoc->twin );          // display screen
  EQFBScreenCursor( pVisDoc->pDoc->twin );        // update cursor and sliders
  return;
}

VOID ITMIrregularPrevFunc( PITMVISDOC pVisDoc)
{
  ITMFuncNextIrregular((PITMIDA) pVisDoc->pITMIda, PREVIOUS);
  EQFBScreenData( pVisDoc->pDoc->twin );          // display screen
  EQFBScreenCursor( pVisDoc->pDoc->twin );        // update cursor and sliders
  return;
}


VOID ITMFuncLineWrap( PITMVISDOC pVisDoc)
{
  EQFBFuncLineWrap( pVisDoc->pDoc );
  return;
}

VOID ITMFuncFontSize( PITMVISDOC pVisDoc)
{
  EQFBFuncFontSize( pVisDoc->pDoc );
  return;
}

VOID ITMFuncMarginAct( PITMVISDOC pVisDoc)
{
  EQFBFuncMarginAct( pVisDoc->pDoc );
  return;
}

VOID ITMFuncSpellSeg( PITMVISDOC pVisDoc)
{
  EQFBFuncSpellSeg( pVisDoc->pDoc );               //spellcheck segment
  return;
}

VOID ITMFuncSpellFile( PITMVISDOC pVisDoc)
{
  EQFBFuncSpellFile( pVisDoc->pDoc );               //spellcheck file
  return;
}

VOID ITMGotoLine( PITMVISDOC pVisDoc)
{
  EQFBGotoLine( pVisDoc->pDoc );            // invoke the GotoLine dialog ..
  return;
}

VOID ITMQueryLine( PITMVISDOC pVisDoc)
{
  EQFBQueryLine( pVisDoc->pDoc );           // find the current cursor pos.
  return;
}

VOID ITMFuncChangeToShrink( PITMVISDOC pVisDoc)
{
  ITMFuncStyle((PITMIDA) pVisDoc->pITMIda, DISP_SHRINK);
  return;
}

VOID ITMFuncChangeToCompact( PITMVISDOC pVisDoc)
{
  ITMFuncStyle((PITMIDA) pVisDoc->pITMIda, DISP_COMPACT);
  return;
}

VOID ITMFuncCaps( PITMVISDOC pVisDoc)
{
  if (pVisDoc->pDoc->sSrcLanguage == -1)
  {
    PITMIDA pITMIda;

    pITMIda = (PITMIDA) pVisDoc->pITMIda;

    MorphGetLanguageID( pITMIda->szTargetLang, &pVisDoc->pDoc->sSrcLanguage);
  }

  EQFBFuncCaps( pVisDoc->pDoc );
  return;
}

VOID ITMFuncVisibleSpace( PITMVISDOC pVisDoc)
{
  EQFBFuncVisibleSpace( pVisDoc->pDoc );
  return;
}

VOID ITMFuncHotPopUp( PITMVISDOC pVisDoc)
{
    POINT  lppt;
    GetCaretPos( &lppt );
    ITMDOCVIEWWNDPROC( ((PITMIDA)(pVisDoc->pITMIda))->hwndFocus, WM_RBUTTONDOWN,
                  0, MP2FROMLONG(*((PLONG)&lppt)));

  return;
}

VOID ITMSynchFunc( PITMVISDOC pVisDoc)
{
  ITMFuncSynch((PITMIDA) pVisDoc->pITMIda, pVisDoc );
  EQFBScreenData( pVisDoc->pDoc->twin );          // display screen
  EQFBScreenCursor( pVisDoc->pDoc->twin );        // update cursor and sliders
  return;
}

VOID ITMSynchNextFunc( PITMVISDOC pVisDoc)
{
  VisActivateSeg((PITMIDA) pVisDoc->pITMIda, pVisDoc, NEXT,
                 TRUE, pVisDoc->ulVisActSeg );
  EQFBScreenData( pVisDoc->pDoc->twin );          // display screen
  EQFBScreenCursor( pVisDoc->pDoc->twin );        // update cursor and sliders
  return;
}

VOID ITMSynchPrevFunc( PITMVISDOC pVisDoc)
{
  VisActivateSeg((PITMIDA) pVisDoc->pITMIda, pVisDoc, PREVIOUS,
                  TRUE, pVisDoc->ulVisActSeg );
  EQFBScreenData( pVisDoc->pDoc->twin );          // display screen
  EQFBScreenCursor( pVisDoc->pDoc->twin );        // update cursor and sliders
  return;
}


VOID ITMToggleFunc( PITMVISDOC pVisDoc)
{
  PITMIDA pITMIda = (PITMIDA) pVisDoc->pITMIda;
  WinPostMsg( pITMIda->hwnd, WM_COMMAND, MP1FROMSHORT( IDM_TOGGLE ), NULL );
  return;
}

VOID ITMSetAnchorFunc( PITMVISDOC pVisDoc)
{
  ITMFuncSetAnchor((PITMIDA) pVisDoc->pITMIda);
  return;
}

VOID ITMDelAnchorFunc( PITMVISDOC pVisDoc)
{
  ITMFuncDelAnchor((PITMIDA) pVisDoc->pITMIda);
  return;
}

VOID ITMCrossOutFunc( PITMVISDOC pVisDoc)
{
  ITMFuncCrossOut((PITMIDA) pVisDoc->pITMIda);
  return;
}

VOID ITMUndoCrossOutFunc( PITMVISDOC pVisDoc)
{
  ITMFuncUndoCrossOut((PITMIDA) pVisDoc->pITMIda);
  return;
}

VOID ITMAddAbbrevFunc( PITMVISDOC pVisDoc)
{
  PITMIDA pITMIda = (PITMIDA) pVisDoc->pITMIda;
  WinPostMsg( pITMIda->hwnd, WM_COMMAND, MP1FROMSHORT( IDM_ITMADDABBR ), NULL );
  return;
}

VOID ITMNextUnalignedFunc( PITMVISDOC pVisDoc)
{
  ITMFuncNextLone((PITMIDA) pVisDoc->pITMIda, NEXT);
  EQFBScreenData( pVisDoc->pDoc->twin );          // display screen
  EQFBScreenCursor( pVisDoc->pDoc->twin );        // update cursor and sliders
  return;
}

VOID ITMPrevUnalignedFunc( PITMVISDOC pVisDoc)
{
  ITMFuncNextLone((PITMIDA) pVisDoc->pITMIda, PREVIOUS);
  EQFBScreenData( pVisDoc->pDoc->twin );          // display screen
  EQFBScreenCursor( pVisDoc->pDoc->twin );        // update cursor and sliders
  return;
}
