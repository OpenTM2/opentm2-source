/* $MOD(EQFITML.C),COMP(ITM),PROD(TM2):  Module EQFITML.C  C                  */
//+----------------------------------------------------------------------------+
//|EQFITML.C                                                                   |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:   R.Jornitz                                                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Program for ITM visualization - LF's insert and delete         |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
#define INCL_EQF_MORPH
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_EDITORAPI        // editor API
#include <eqf.h>                  // General Translation Manager include file

#include "EQFITM.H"

/**********************************************************************/
/* test output ...                                                    */
/**********************************************************************/
#ifdef ITMTEST
  FILE *fOut;                          // test output
#endif

static VOID ITMAddLF ( ULONG ,LONG, PTBDOCUMENT, PITMVISDOC );
static VOID AddLFToBegin(PTBDOCUMENT, PITMVISDOC, LONG, ULONG  );
static LONG ITMSegCountLF ( PITMVISDOC, PTBDOCUMENT , ULONG  );
static ULONG  ITMFindBlockStart(ULONG, PTBDOCUMENT, PITMVISDOC, PITMIDA);
static LONG   ITMCountBlockLF ( ULONG, PULONG , PTBDOCUMENT,
                                PITMVISDOC, PITMIDA );
static VOID ITMCheckIndex (PALLALIGNED, PULONG, PULONG);
static USHORT IncreaseBy ( USHORT );
static VOID   ITMFind1stAlign( PITMIDA );

static PTBSEGMENT GetCurVisSeg ( PTBDOCUMENT , PULONG );
static void ITMPrepRedisp( PTBDOCUMENT pDoc );
static void ITMStringDelUpToNumLF( PSZ_W pData, LONG  lNumLF);
/**********************************************************************/
/* return TRUE if any kind of nop...                                  */
/**********************************************************************/
#define ISQFNOP( a )  \
 ((a==QF_NOP)||(a==QF_NOP_ANCHOR_1)||(a==QF_NOP_ANCHOR_2)||(a==QF_NOP_ANCHOR_3))

static TBSEGMENT ITMShrinkSeg=            // shrinkSegment
    {
      NULL,                            // pointer to segment data
      0,                               // length of segment data
      QF_NOP,                          // status of segment  (XLATED, TOBE,..)
      0,                               // segment number
      NULL,                            // Browser Protection Elements Table
      {0},                                      // segment flags
      0,{0},0,0,0,                  // usOrgLength,CountFlag,usSrc-Tgt-Mod...Words
      0L, NULL, L"", NULL
    };
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMDelLF                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     ITMDelLF(pITMIda, usSrcStart, usSrcENd,                  |
//|                             usTgtStart, usTgtEnd)                          |
//+----------------------------------------------------------------------------+
//|Description:       delete superfluous lf's at end of segments in range      |
//|                   in both documents                                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PITMIDA     pITMIda,                                     |
//|                   USHORT      usSrcStart,                                  |
//|                   USHORT      usSrcEnd,                                    |
//|                   USHORT      usTgtStart,                                  |
//|                   USHORT      usTgtEnd                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if parallel is on                                        |
//|                      delete lfs in src doc in specified range              |
//|                      delete lfs in tgt doc in specified range              |
//+----------------------------------------------------------------------------+
VOID
ITMDelLF
(
  PITMIDA     pITMIda,
  ULONG       ulSrcStart,
  ULONG       ulSrcEnd,
  ULONG       ulTgtStart,
  ULONG       ulTgtEnd
)
{
  if ( pITMIda->fParallel )
  {
    ITMDocDelLF ( &(pITMIda->TBSourceDoc),
                  &(pITMIda->stVisDocSrc),
                  ulSrcStart, ulSrcEnd );

    ITMDocDelLF ( &(pITMIda->TBTargetDoc),
                  &(pITMIda->stVisDocTgt),
                  ulTgtStart, ulTgtEnd );
  } /* endif */
  return ;
} /* end of function ITMDelLF */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMDocDelLF                                              |
//+----------------------------------------------------------------------------+
//|Function call:     ITMDocDelLF(pDoc, pVisDoc, usStartSeg, usEndSeg)         |
//+----------------------------------------------------------------------------+
//|Description:       delete LF's in one doc added to make segments parallel   |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT    pDoc,                                     |
//|                   PITMVISDOC     pVisDoc,                                  |
//|                   USHORT         usStartSeg,                               |
//|                   USHORT         usEndSeg                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     get first seg of specified range                         |
//|                   while not at end of range                                |
//|                     if LF's has been changed at this segment               |
//|                       delete the added LF's                                |
//|                       reset length of segment                              |
//|                       reset indicator that LF's are added                  |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
VOID
ITMDocDelLF
(
  PTBDOCUMENT    pDoc,
  PITMVISDOC     pVisDoc,
  ULONG          ulStartSeg,
  ULONG          ulEndSeg
)
{
  ULONG        ulSegNum;
  PTBSEGMENT   pSeg;
  LONG         lNumLF;

  ulSegNum =  ulStartSeg;
  pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);

  while ( ulSegNum < ulEndSeg )
  {
    if ( pVisDoc->pVisState[ulSegNum].LFChanged )
    {
      /********************************************************************/
      /* adjust if ulSegnum is 1st aligned seg of file and lf's added at  */
      /* begin of segment                                                 */
      /********************************************************************/
      if ( ulSegNum == pVisDoc->ulFirstAlign )
      {
        memcpy( pSeg->pDataW,pSeg->pDataW + (pVisDoc->usAddLFatBegin),
                pSeg->usLength * sizeof(CHAR_W) );
        pSeg->usLength = (USHORT)(UTF16strlenCHAR (pSeg->pDataW));
        pVisDoc->usAddLFatBegin = 0;
      } /* endif */

      /****************************************************************/
      /* go forward the number of original LF's ( = pbLFNum )         */
      /****************************************************************/
      lNumLF = pVisDoc->pbLFNum[ulSegNum];
      ITMStringDelUpToNumLF(pSeg->pDataW, lNumLF);

      pSeg->usLength = (USHORT)(UTF16strlenCHAR(pSeg->pDataW));
      pVisDoc->pVisState[ulSegNum].LFChanged = FALSE;

      /****************************************************************/
      /* adjust pSaveSeg if nec.                                      */
      /****************************************************************/

      if ((&(pSeg->pDataW[0]) == &(pDoc->pEQFBWorkSegmentW[0])) && pDoc->pSaveSegW)
      {
        lNumLF = pVisDoc->pbLFNum[ulSegNum];
        ITMStringDelUpToNumLF(pDoc->pSaveSegW, lNumLF);
      } /* endif */
    } /* endif */
    /******************************************************************/
    /* goto next segment                                              */
    /******************************************************************/
    ulSegNum++;
    pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
  } /* endwhile */

  return ;
} /* end of function ITMDocDelLF */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMAdjustLF                                              |
//+----------------------------------------------------------------------------+
//|Function call:     ITMAdjustLF(pITMIda, ulSrcStart, ulSrcEnd,               |
//|                                        ulTgtStart, ulTgtEnd)               |
//+----------------------------------------------------------------------------+
//|Description:       add LF's as necessary to make segs parallel              |
//|                   also one empty line is required between 2 aligned segs   |
//+----------------------------------------------------------------------------+
//|Parameters:        PITMIDA     pITMIda,                                     |
//|                   ULONG       ulSrcStart                                   |
//|                   ULONG       ulSrcEnd,                                    |
//|                   ULONG       ulTgtStart                                   |
//|                   ULONG       ulTgtEnd                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if parallel is required                                  |
//|                     get alignment index of start of parallelregion         |
//|                     check that index is not a 0:1 or 1:0 alignment         |
//|                     if at begin of both files                              |
//|                        find 1st aligned segments in both files             |
//|                        if src only starts with Unaligned stuff             |
//|                          add LF's to begin of 1st aligned tgtseg           |
//|                        if tgt only starts with Unaligned stuff             |
//|                          add LF's to begin of 1st aligned srcseg           |
//|                     if area starts with unaligned stuff in both files      |
//|                        go back til begin of unaligned stuff                |
//|                        count linefeeds in this 'unaligned' blocks          |
//|                        add lf's in the shorter block                       |
//|                        set current src/tgt seg to next segment after block |
//|                     while not end of region to be parallelized             |
//|                        get alignment index                                 |
//|                        check that it not a 0:1 / 1:0 alignment             |
//|                        if current segs are aligned with another            |
//|                          count lf's of current segs                        |
//|                          add at shorter seg so that at least 2 lf's are    |
//|                           at end of each seg                               |
//|                        else                                                |
//|                          if srcseg is not aligned, count lf's of that block|
//|                          if tgtseg not aligned, count lf's of that block   |
//|                          add lf's at end of shorter garbage block, or      |
//|                          if not possible, at end of last aligned segs      |
//|                        endif                                               |
//|                        set current segs to next seg (if it was aligned)    |
//|                        or next seg after current unaligned block           |
//|                     endwhile                                               |
//|                     force recompute of start-stop tables in parallelregion |
//+----------------------------------------------------------------------------+
VOID
ITMAdjustLF
(
  PITMIDA     pITMIda,
  ULONG       ulSrcStart,
  ULONG       ulSrcEnd,
  ULONG       ulTgtStart,
  ULONG       ulTgtEnd
)
{
  PTBDOCUMENT  pSrcDoc, pTgtDoc;
  PITMVISDOC   pVisSrc, pVisTgt;
  PTBSEGMENT   pSeg = NULL;
  ULONG        ulSrcSeg = 0;
  ULONG        ulTgtSeg = 0;
  ULONG        ulSrcIndex = 0;
  ULONG        ulTgtIndex = 0;
  ULONG        ulSrcBlockStart, ulTgtBlockStart;
  LONG         lNumSrcLF, lNumTgtLF;
  ULONG        ulSrcBlockEnd = 0;
  ULONG        ulTgtBlockEnd = 0;
  ULONG        ulAlignSrc = 0;
  ULONG        ulAlignTgt = 0;
  PALLALIGNED  pAligned = NULL;
  ULONG        ulSegBlockEnd = 0;
  LONG         lMaxLF = 0;
  USHORT       usSrcLFatEnd = 0;
  USHORT       usTgtLFatEnd = 0;
  ULONG        ulSeg = 0;
  BOOL         fOK = TRUE;
  ULONG        ulOldSrc = 0;
  ULONG        ulOldTgt = 0;


  if ( pITMIda->fParallel )
  {
    ulTgtEnd;                             // to avoiiid warning
    pSrcDoc = &(pITMIda->TBSourceDoc);
    pTgtDoc = &(pITMIda->TBTargetDoc);
    pVisSrc = &(pITMIda->stVisDocSrc);
    pVisTgt = &(pITMIda->stVisDocTgt);
    pAligned = &(pITMIda->Aligned);

    /******************************************************************/
    /* to make sure that LF at begin of seg are adjusted              */
    /******************************************************************/
    if ( (ulSrcStart <= pVisSrc->ulFirstAlign )
        && (ulTgtStart <= pVisTgt->ulFirstAlign)  )
    {
      ulSrcStart = 1;
      ulTgtStart = 1;
    } /* endif */

    ulSrcIndex = pVisSrc->pulNumAligned[ulSrcStart];
    ulTgtIndex = pVisTgt->pulNumAligned[ulTgtStart];
    /******************************************************************/
    /* check that index is not a 0:1 or 1:0 alignment                 */
    /******************************************************************/
    ITMCheckIndex (pAligned, &ulSrcIndex, &ulTgtIndex);

    ulSrcSeg = ulSrcStart;
    ulTgtSeg = ulTgtStart;
    /****************************************************************/
    /* area starts with 'garbage; extra handling at begin of file   */
    /*  if only in one file garbage exists                          */
    /****************************************************************/

    if ( (ulSrcStart == 1) && (ulTgtStart == 1) )
    {
      /****************************************************************/
      /* fill 1st aligned segnum in visdoc structure                  */
      /****************************************************************/
      ITMFind1stAlign(pITMIda );

      if ( !ulSrcIndex && ulTgtIndex  )
      {
        /**************************************************************/
        /* add lf's to begin of usTgtStart                            */
        /* srcseg1: NOP                                               */
        /* srcseg2      ---- tgtseg1                                  */
        /**************************************************************/
        lNumSrcLF = ITMCountBlockLF( 1 , &ulSegBlockEnd,
                                  pSrcDoc, pVisSrc, pITMIda);
        AddLFToBegin(pTgtDoc, pVisTgt, lNumSrcLF, 1);     //cnt lf's in SrcDoc
        ulSrcSeg = ulSegBlockEnd + 1;
      } /* endif */
      if ( !ulTgtIndex && ulSrcIndex  )     // add lf's to begin of usSrcStart
      {
        /**************************************************************/
        /*                  tgtseg1 NOP                               */
        /* srcseg1     ---- tgtseg2                                   */
        /**************************************************************/
        lNumTgtLF = ITMCountBlockLF( 1 , &ulSegBlockEnd,
                                  pTgtDoc, pVisTgt, pITMIda);
        AddLFToBegin(pSrcDoc, pVisSrc, lNumTgtLF, 1);     //cnt lf's in TgtDoc
        ulTgtSeg = ulSegBlockEnd + 1;
      } /* endif */
    } /* endif */
    /********************************************************************/
    /* go thru alignment                                                */
    /********************************************************************/
    if ( !ulSrcIndex && !ulTgtIndex )     //start in garbage block
    {
      /******************************************************************/
      /* go back til begin of garbage block                             */
      /* ulSrcBLockEnd is last seg of garbage block                     */
      /******************************************************************/
      ulSrcBlockStart = ITMFindBlockStart(ulSrcSeg,pSrcDoc, pVisSrc, pITMIda);
      ulTgtBlockStart = ITMFindBlockStart(ulTgtSeg,pTgtDoc, pVisTgt, pITMIda);
      lNumSrcLF = ITMCountBlockLF(ulSrcBlockStart, &ulSrcBlockEnd,
                                   pSrcDoc, pVisSrc, pITMIda);
      lNumTgtLF = ITMCountBlockLF(ulTgtBlockStart, &ulTgtBlockEnd,
                                    pTgtDoc, pVisTgt, pITMIda);
      if ( lNumSrcLF < lNumTgtLF )
      {
        //add lf's to src
        pSeg = EQFBGetVisSeg(pSrcDoc, &ulSrcBlockEnd);
        if ( pSeg && ISQFNOP(pSeg->qStatus ) &&
             ( (pSrcDoc->DispStyle==DISP_SHRINK) ||
               (pSrcDoc->DispStyle==DISP_COMPACT)) )
        {
          /**************************************************************/
          /* add lf to begin if at begin of file, else at end of usSeg  */
          /* usSeg is then not parallel, but this is a quick and dirty  */
          /* fix; ( 6.4.94) is dirty, and wrong!!                     */
          /**************************************************************/
          ulSeg = ulSrcBlockEnd + 1;
          pSeg = EQFBGetVisSeg(pSrcDoc, &ulSeg);
          if ( ulSeg == pVisSrc->ulFirstAlign )
          {
            AddLFToBegin(pSrcDoc, pVisSrc,
                         (lNumTgtLF - lNumSrcLF ),
                          (ulSrcBlockEnd + 1));
          }
          else
          {
//            beep;                               //should not happen
//            ITMAddLF(usSeg, (lNumTgtLF - usNumSrcLF), pSrcDoc, pVisSrc);
          } /* endif */
        }
        else
        {
          ITMAddLF(ulSrcBlockEnd, (lNumTgtLF - lNumSrcLF),
                   pSrcDoc, pVisSrc);
        } /* endif */
      }
      else
      {
        pSeg = EQFBGetVisSeg(pTgtDoc, &ulTgtBlockEnd);
        if ( pSeg && ISQFNOP(pSeg->qStatus ) &&
             ( (pTgtDoc->DispStyle==DISP_SHRINK) ||
               (pTgtDoc->DispStyle==DISP_COMPACT)) )
        {
          /**************************************************************/
          /* add lf to begin if at begin of file, else at end of usSeg  */
          /* usSeg is then not parallel, but this is a quick and dirty  */
          /* fix; ( 6.4.94)                                           */
          /**************************************************************/
          ulSeg = ulTgtBlockEnd + 1;
          pSeg = EQFBGetVisSeg(pTgtDoc, &ulSeg);
          if ( ulSeg == pVisTgt->ulFirstAlign )
          {
            AddLFToBegin(pTgtDoc, pVisTgt,
                        (lNumSrcLF - lNumTgtLF ),
                        (ulTgtBlockEnd + 1));
          }
          else
          {
//            beep;                               //should not happen
//            ITMAddLF(usSeg, (usNumSrcLF - lNumTgtLF), pTgtDoc, pVisTgt);
          } /* endif */
        }
        else
        {
          ITMAddLF(ulTgtBlockEnd,
                   (lNumSrcLF - lNumTgtLF), pTgtDoc, pVisTgt);
        } /* endif */
      } /* endif */
      ulSrcSeg = ulSrcBlockEnd + 1;
      ulTgtSeg = ulTgtBlockEnd + 1;
    } /* endif */
    pSeg = EQFBGetVisSeg(pSrcDoc, &ulSrcSeg);
    pSeg = EQFBGetVisSeg(pTgtDoc, &ulTgtSeg);

    while (fOK && ( ulSrcSeg < ulSrcEnd ))
    {
      ulOldSrc = ulSrcSeg;                   // remember srcsegnum && tgtsegnum
      ulOldTgt = ulTgtSeg;
      ulSrcIndex = pVisSrc->pulNumAligned[ulSrcSeg];
      ulTgtIndex = pVisTgt->pulNumAligned[ulTgtSeg];
      ITMCheckIndex (pAligned, &ulSrcIndex, &ulTgtIndex);

      if ( ulSrcIndex && (ulSrcIndex == ulTgtIndex ))
      {
        /******************************************************************/
        /* area starts with an alignment                                  */
        /******************************************************************/
        lNumSrcLF = ITMSegCountLF(pVisSrc, pSrcDoc, ulSrcSeg);
        lNumTgtLF = ITMSegCountLF(pVisTgt, pTgtDoc, ulTgtSeg);
        ulAlignSrc = ulSrcSeg;             //remember if garbage block follows
        ulAlignTgt = ulTgtSeg;
        /**************************************************************/
        /* lMaxLF = max(2, usNumSrcLF, lNumTgtLF )                  */
        /**************************************************************/
        usSrcLFatEnd = IncreaseBy( (USHORT) pVisSrc->pbLFNum[ulSrcSeg] );
        usTgtLFatEnd = IncreaseBy( (USHORT) pVisTgt->pbLFNum[ulTgtSeg] );

        lMaxLF = max( lNumSrcLF + usSrcLFatEnd, lNumTgtLF + usTgtLFatEnd );

        if ( lNumSrcLF < lMaxLF )
        {
          ITMAddLF(ulSrcSeg,
                   (lMaxLF - lNumSrcLF), pSrcDoc, pVisSrc);
        } /* endif */

        if (lNumTgtLF < lMaxLF )
        {
          ITMAddLF(ulTgtSeg,
                   (lMaxLF - lNumTgtLF), pTgtDoc, pVisTgt);
        } /* endif */
        ulSrcSeg++;
        ulTgtSeg++;
      }
      else
      {
        /****************************************************************/
        /* area is garbage block                                        */
        /****************************************************************/
        lNumSrcLF = lNumTgtLF = 0;
        if ( !ulSrcIndex )         // in src garbage block
        {
          lNumSrcLF = ITMCountBlockLF(ulSrcSeg, &ulSrcBlockEnd,
                                       pSrcDoc, pVisSrc, pITMIda);
        } /* endif */
        if ( !ulTgtIndex )         // in tgt garbage block
        {
          lNumTgtLF = ITMCountBlockLF(ulTgtSeg, &ulTgtBlockEnd,
                                       pTgtDoc, pVisTgt, pITMIda);
        } /* endif */
        if ( !ulSrcIndex && (!ulTgtIndex) )      //garbage in both docs
        {
          /**************************************************************/
          /* add lf at shorter garbage block                            */
          /**************************************************************/
          if ( lNumSrcLF < lNumTgtLF )
          {
            pSeg = EQFBGetVisSeg(pSrcDoc, &ulSrcBlockEnd);
            if ( pSeg && ISQFNOP(pSeg->qStatus ) &&
                 ( (pSrcDoc->DispStyle==DISP_SHRINK) ||
                   (pSrcDoc->DispStyle==DISP_COMPACT)) )
            {
              ITMAddLF(ulAlignSrc,
                       (lNumTgtLF - lNumSrcLF),
                        pSrcDoc, pVisSrc);
            }
            else
            {
              ITMAddLF(ulSrcBlockEnd,
                       (lNumTgtLF - lNumSrcLF),
                        pSrcDoc, pVisSrc);
            } /* endif */
          }
          else
          {
            pSeg = EQFBGetVisSeg(pTgtDoc, &ulTgtBlockEnd);
            if ( pSeg && ISQFNOP(pSeg->qStatus ) &&
                 ( (pTgtDoc->DispStyle==DISP_SHRINK) ||
                   (pTgtDoc->DispStyle==DISP_COMPACT)) )
            {
              ITMAddLF(ulAlignTgt,
                       (lNumSrcLF - lNumTgtLF),
                       pTgtDoc, pVisTgt);
            }
            else
            {
              ITMAddLF(ulTgtBlockEnd,
                       (lNumSrcLF - lNumTgtLF),
                        pTgtDoc, pVisTgt);
            } /* endif */
          } /* endif */
          ulSrcSeg = ulSrcBlockEnd + 1;
          ulTgtSeg = ulTgtBlockEnd + 1;
        }
        else                   // garbage only in one doc
        {
          /**************************************************************/
          /* add lf to last alignseg                                    */
          /**************************************************************/
          if ( ulSrcIndex )                   // no garbage in src doc
          {
            ITMAddLF(ulAlignSrc,lNumTgtLF,pSrcDoc, pVisSrc);
            ulTgtSeg = ulTgtBlockEnd + 1;
          } /* endif */
          if ( ulTgtIndex )                   // no garbage in tgt  doc
          {
            ITMAddLF(ulAlignTgt, lNumSrcLF, pTgtDoc, pVisTgt);
            ulSrcSeg = ulSrcBlockEnd + 1;
          } /* endif */

        } /* endif */

      } /* endif */

      EQFBGetVisSeg(pSrcDoc, &ulSrcSeg);
      EQFBGetVisSeg(pTgtDoc, &ulTgtSeg);
      /****************************************************************/
      /* check here to avoid endless looping due to another error...  */
      /****************************************************************/
      if ( (ulSrcSeg == ulOldSrc) && ( ulTgtSeg == ulOldTgt) )
      {
        fOK = FALSE;
      } /* endif */
      if ( (ulSrcSeg < ulOldSrc) || (ulTgtSeg < ulOldTgt) )
      {
        fOK = FALSE;
      } /* endif */

    } /* endwhile */
    if ( !fOK )
    {
      ITMUtlError( pITMIda, ERROR_INTERNAL, MB_OK, 0, NULL, EQF_ERROR );
    } /* endif */
    /*****************************************************************/
    /* force recompute of all start-stop tables                      */
    /*****************************************************************/
    for (ulSeg = ulSrcStart ;ulSeg < ulSrcEnd  ; ulSeg++ )
    {
      pSeg = EQFBGetSegW( pSrcDoc, ulSeg );
      if ( pSeg )
      {
         UtlAlloc( (PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG); // free old segment table
         if (pSeg->pusHLType) UtlAlloc((PVOID *)&(pSeg->pusHLType),0L,0L,NOMSG);
      } /* endif */
    } /* endfor */
    for (ulSeg = ulTgtStart ;ulSeg < ulTgtEnd  ; ulSeg++ )
    {
      pSeg = EQFBGetSegW( pTgtDoc, ulSeg );
      if ( pSeg )
      {
         UtlAlloc( (PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG); // free old segment table
         if (pSeg->pusHLType) UtlAlloc((PVOID *)&(pSeg->pusHLType),0L,0L,NOMSG);
      } /* endif */
    } /* endfor */
  } /* endif */
  return ;
} /* end of function ITMAdjustLF */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMCheckIndex                                            |
//+----------------------------------------------------------------------------+
//|Function call:     ITMCheckIndex (pAligned, &pulSrcIndex, &pulTgtIndex)     |
//+----------------------------------------------------------------------------+
//|Description:       set alignment index to 0 if alignment is not a real one  |
//+----------------------------------------------------------------------------+
//|Parameters:        PALLALIGNED    pAligned,                                 |
//|                   PULONG         pulSrcIndex,                              |
//|                   PULONG         pulTgtIndex                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if Index of srcsegment alignment is not 0                |
//|                      if alignment is a 0:1 / 1:0 set index to 0            |
//|                   if index of tgtsegment alignment is not 0                |
//|                     if alignment is a 0:1/1:0 set index to 0               |
//+----------------------------------------------------------------------------+
static VOID
ITMCheckIndex
(
   PALLALIGNED    pAligned,
   PULONG         pulSrcIndex,
   PULONG         pulTgtIndex
)
{
  ULONG          ulSrcIndex ;
  ULONG          ulTgtIndex;

  ulSrcIndex = *pulSrcIndex;
  ulTgtIndex = *pulTgtIndex;
  /******************************************************************/
  /* if alignment is 0:1 or 1:0, set index to zero because          */
  /* alignment is not a real alignment                              */
  /******************************************************************/
  if ( ulSrcIndex )
  {
    if ( (pAligned->pbType[ulSrcIndex] == NUL_ONE)
          || (pAligned->pbType[ulSrcIndex] == ONE_NUL)   )
    {
      *pulSrcIndex = 0;
    } /* endif */
  } /* endif */
  if ( ulTgtIndex )
  {
    if ( (pAligned->pbType[ulTgtIndex] == NUL_ONE)
          || (pAligned->pbType[ulTgtIndex] == ONE_NUL)   )
    {
       *pulTgtIndex = 0;
    } /* endif */
  } /* endif */
  return;
} /* end of function ITMCheckIndex */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMAddLF(ulAlignTgt, usNumSrcLF, pTgtDoc, pVisTgt)       |
//+----------------------------------------------------------------------------+
//|Function call:     ITMAddLF(ulSegNum, lNumLF, pDoc, pVisDoc)               |
//+----------------------------------------------------------------------------+
//|Description:       add lNumLF many linefeeds at end of segment ulSegNum    |
//+----------------------------------------------------------------------------+
//|Parameters:        ULONG        ulSegNum                                    |
//|                   USHORT       lNumLF,                                    |
//|                   PTBDOCUMENT  pDoc,                                       |
//|                   PITMVISDOC   pVisDoc                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if lNumLF != 0 and ulSegNum != 0                        |
//|                     realloc enough space for longer segment                |
//|                     if ok, add lf's at end                                 |
//|                           set LF's changed indicator                       |
//|                           reset length of segment                          |
//|                     endif                                                  |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
static VOID
ITMAddLF
(
  ULONG        ulSegNum,                    //segnum with LF's to be added
  LONG         lNumLF,                     // add lNumLF many LF's
  PTBDOCUMENT  pDoc,                        // segnum in this document
  PITMVISDOC   pVisDoc                      // segnum in this visdoc
)
{
  PTBSEGMENT   pSeg;
  PSZ_W        pData;
  LONG         lOldLen, lNewLen;
  BOOL         fOK;
  PSZ_W        pTempData = NULL;
  LONG         lSaveNumLF = lNumLF;

  if ( lNumLF )
  {
    if ( ulSegNum && (ulSegNum < pDoc->ulMaxSeg))
    {
      pSeg = EQFBGetSegW(pDoc, ulSegNum);
      if (&(pSeg->pDataW[0]) != &(pDoc->pEQFBWorkSegmentW[0]))
      {
        lOldLen = UTF16strlenCHAR(pSeg->pDataW) + 1;
        lNewLen = lOldLen + lNumLF;
        fOK = UtlAlloc( (PVOID *)&(pSeg->pDataW), (LONG) lOldLen*sizeof(CHAR_W),    //alloc for longer seg
                      (LONG) lNewLen*sizeof(CHAR_W), ERROR_STORAGE);
      }
      else
      {
        fOK = TRUE;
        if (pDoc->pSaveSegW)
        {
          lNewLen = UTF16strlenCHAR(pDoc->pSaveSegW) + lSaveNumLF + 1;
          fOK = UtlAlloc( (PVOID *)&pTempData, 0L,    //alloc for longer seg
                      (LONG) max( lNewLen * sizeof(CHAR_W), MIN_ALLOC), ERROR_STORAGE);
        }

      }
      if ( fOK )
      {
        pData = pSeg->pDataW + UTF16strlenCHAR(pSeg->pDataW);
        while ( lNumLF )                              //add lNumLF many LF's
        {
          *pData++ =  LF;
          lNumLF--;
        } /* endwhile */
        *pData = EOS;                                  //add endofsegment

        pVisDoc->pVisState[ulSegNum].LFChanged = TRUE;
        pSeg->usLength = (USHORT)(UTF16strlenCHAR(pSeg->pDataW));

        if (pTempData  && pDoc->pSaveSegW)
        {
          UTF16strcpy(pTempData, pDoc->pSaveSegW);
          pData = pTempData + UTF16strlenCHAR(pTempData);
          while (lSaveNumLF)
          {
            *pData++ = LF;
            lSaveNumLF--;
          } /* endwhile */
          *pData = EOS;
          UtlAlloc((PVOID *) &pDoc->pSaveSegW, 0L, 0L, NOMSG);
          pDoc->pSaveSegW = pTempData;
        }

      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* ulSegNum ==0, program error...                               */
      /****************************************************************/
    } /* endif */
  } /* endif */
  return;
} /* end of function ITMAddLF */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AddLFToBegin(pVisTgt, pTgtDoc, pSrcDoc)                  |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       add LF's to begin of a segment                           |
//|                   only allowed if segment is 1st aligned segment in doc    |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT  pAddDoc,                                    |
//|                   PITMVISDOC   pVisAdd,                                    |
//|                   USHORT       lNumLF,                                    |
//|                   USHORT       ulSegNum                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if number of lf's is not 0                               |
//|                     realloc more space for segment data                    |
//|                     move segment backward to make room for lf's            |
//|                     add lf's at begin                                      |
//|                     set LF's changed indicator                             |
//|                     remember how many lf's are added at begin              |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
static VOID
AddLFToBegin
(
  PTBDOCUMENT  pAddDoc,          //where to add lf
  PITMVISDOC   pVisAdd,                //where to add lf
  LONG         lNumLF,
  ULONG        ulSegNum         // seg to which lf's are added
)
{
  LONG         lOldLen = 0;
  LONG         lNewLen = 0;
  PTBSEGMENT   pSeg = NULL;
  PSZ_W        pData = NULL;
  BOOL         fOK;
  LONG         lI = 0;
  LONG         lStartNumLF = 0;

//lNumLF = ITMCountBlockLF(ulSegNum, &ulSegBlockEnd,
//                          pCntDoc, pVisCnt, pITMIda);
  if ( lNumLF )
  {
    lStartNumLF = lNumLF;               // remember value
    /********************************************************************/
    /* add lf to begin                                                  */
    /********************************************************************/
    pSeg = EQFBGetVisSeg(pAddDoc, &ulSegNum);

    if (&(pSeg->pDataW[0]) != &(pAddDoc->pEQFBWorkSegmentW[0]))
    {
      lOldLen = UTF16strlenCHAR(pSeg->pDataW) + 1;
      lNewLen = lOldLen + lNumLF;
      fOK = UtlAlloc( (PVOID *)&(pSeg->pDataW), (LONG) lOldLen*sizeof(CHAR_W),    //alloc for longer seg
                    (LONG) lNewLen*sizeof(CHAR_W), ERROR_STORAGE);
    }
    else
    {
      fOK = TRUE;
    } /* endif */
    if ( fOK )
    {
      /******************************************************************/
      /* make room for lNumLF many LF's at begin of segment            */
      /******************************************************************/
      pData = pSeg->pDataW;
      for ( lI = lOldLen - 1 ; lI >= 0 ; lI-- )
      {
        *(pData + lI + lNumLF) = *(pData + lI);
      } /* endfor */
      /******************************************************************/
      /* add lf's at begin                                              */
      /******************************************************************/
      pData = pSeg->pDataW ;
      while ( lNumLF )                      //add lNumLF many LF's
      {
        *pData++ =  LF;
        lNumLF--;
      } /* endwhile */

      pVisAdd->pVisState[ulSegNum].LFChanged = TRUE;
      pSeg->usLength = (USHORT)(UTF16strlenCHAR(pSeg->pDataW));
      if ( ulSegNum == pVisAdd->ulFirstAlign )
      {
        pVisAdd->usAddLFatBegin = (USHORT)lStartNumLF;
      } /* endif */
    } /* endif */
  } /* endif */

  return;
} /* end of function AddLFToBegin */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMFindBlockStart                                        |
//+----------------------------------------------------------------------------+
//|Function call:     ITMFindBlockStart(ulSegNum, pDoc, pVisDoc, pITMIda)      |
//+----------------------------------------------------------------------------+
//|Description:       find start seg of block of garbage                       |
//+----------------------------------------------------------------------------+
//|Parameters:        ULONG       ulSegNum                                     |
//|                   PTBDOCUMENT pDoc,                                        |
//|                   PITMVISDOC  pVisDoc,                                     |
//|                   PITMIDA     pITMIda                                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   ULONG                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       ulSegNum which is start of block                         |
//+----------------------------------------------------------------------------+
//|Function flow:     get current visible segment                              |
//|                   loop til previous visible alignment                      |
//|                   return segment number of that segment                    |
//+----------------------------------------------------------------------------+
static
ULONG
ITMFindBlockStart
(
    ULONG       ulSegNum,
    PTBDOCUMENT pDoc,
    PITMVISDOC  pVisDoc,
    PITMIDA     pITMIda
)
{
  ULONG         ulStartSeg = ulSegNum;
  ULONG         ulIndex;
  PALLALIGNED   pAligned;
  PTBSEGMENT    pSeg;

  pAligned = &(pITMIda->Aligned);
  if ( ulSegNum != 1 )                        //find current visible seg
  {
    pSeg = GetCurVisSeg(pDoc, &ulSegNum);
//  pSeg = EQFBGetPrevVisSeg(pDoc, &ulSegNum);
//  ulSegNum++;
//  pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
  } /* endif */
  ulStartSeg = ulSegNum;
  ulIndex  = pVisDoc->pulNumAligned[ulSegNum];
  /********************************************************************/
  /* loop til previous alignment                                      */
  /********************************************************************/
  while ( ulSegNum &&
          (  !ulIndex
           || (pAligned->pbType[ulIndex] == ONE_NUL )
           || (pAligned->pbType[ulIndex] == NUL_ONE ) ) )
  {
    ulStartSeg = ulSegNum;                  //store last seg in block
    ulSegNum--;
    pSeg = EQFBGetPrevVisSeg(pDoc, &ulSegNum);
    ulIndex  = pVisDoc->pulNumAligned[ulSegNum];
  } /* endwhile */

  return (ulStartSeg);
} /* end of function ITMFindBlockStart */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     USHORT ITMCountBlockLF                                   |
//+----------------------------------------------------------------------------+
//|Function call:     ITMCountBlockLF(ulBlockStart, &ulBlockEnd, pDoc,         |
//|                                   pVisDoc, pITMIda)                        |
//+----------------------------------------------------------------------------+
//|Description:       count all lf's in garbage block                          |
//|                   garbage are all segs except 1:1,1:2, 2:1,2:2 aligned segs|
//+----------------------------------------------------------------------------+
//|Parameters:        ULONG        ulBlockStart                                |
//|                   PULONG       pulBlockEnd,                                |
//|                   PTBDOCUMENT  pDoc,                                       |
//|                   PITMVISDOC   pVisDoc,                                    |
//|                   PITMIDA      pITMIda                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       lNumLF    Number of linefeeds in block                  |
//+----------------------------------------------------------------------------+
//|Function flow:     get index of alignment of current seg                    |
//|                   while not next aligned seg is found                      |
//|                     add number of lf's of segment to total number of segs  |
//|                     get next visible segment and its alignment index       |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
static
LONG
ITMCountBlockLF
(
     ULONG        ulBlockStart,                  // visible segnum
     PULONG       pulBlockEnd,                   // visible segnum
     PTBDOCUMENT  pDoc,
     PITMVISDOC   pVisDoc,
     PITMIDA      pITMIda
)
{
  LONG            lNumLF = 0;
  ULONG           ulSegNum;
  ULONG           ulIndex;
  PTBSEGMENT      pSeg;
  PALLALIGNED     pAligned;
  BOOL            fFound = FALSE;

  pAligned = &(pITMIda->Aligned);
  ulSegNum = ulBlockStart;
  ulIndex  = pVisDoc->pulNumAligned[ulSegNum];
  /********************************************************************/
  /* loop til next alignment                                          */
  /********************************************************************/
  while ( !fFound )
  {
    lNumLF += ITMSegCountLF(pVisDoc, pDoc, ulSegNum);
    *pulBlockEnd = ulSegNum;                  //store last seg in block
    ulSegNum++;
    pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
    ulIndex  = pVisDoc->pulNumAligned[ulSegNum];

    if ( ulSegNum >= pDoc->ulMaxSeg )
    {
      fFound = TRUE;
    }
    else
    {
      if ( ulIndex )
      {
        if ( (pAligned->pbType[ulIndex] == ONE_ONE )
            || ( pAligned->pbType[ulIndex] == ONE_TWO )
            || ( pAligned->pbType[ulIndex] == TWO_ONE )
            || ( pAligned->pbType[ulIndex] == TWO_TWO ) )
        {
          fFound = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endwhile */
  return (lNumLF);
} /* end of function USHORT ITMCountBlockLF */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMSegCountLF                                            |
//+----------------------------------------------------------------------------+
//|Function call:     ITMSegCountLF(pVisDoc, pDOc, ulSegNum)                   |
//+----------------------------------------------------------------------------+
//|Description:       count all LF's in given segment                          |
//+----------------------------------------------------------------------------+
//|Parameters:        PITMVISDOC  pVisDoc                                      |
//|                   PTBDOCUMENT pDoc                                         |
//|                   ULONG       ulSegNum                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       usNumLF         Number of LF's found                     |
//+----------------------------------------------------------------------------+
//|Function flow:     get ptr to segment data                                  |
//|                   while not at end of segment                              |
//|                     if current char is lf, increase counter                |
//|                     if segment is 1st aligned segment in doc,              |
//|                      decrease counter by the number of lf's added          |
//|                      at begin of segment                                   |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
static
LONG
ITMSegCountLF
(
    PITMVISDOC   pVisDoc,
    PTBDOCUMENT  pDoc,
    ULONG        ulSegNum
)
{
  LONG  lNumLF = 0;
  PTBSEGMENT pSeg;
  PSZ_W      pData;

  if (ulSegNum < pDoc->ulMaxSeg )
  {
    pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
    pData = pSeg->pDataW;
      if (pDoc->fAutoLineWrap && pDoc->fLineWrap )
      {
        CHAR_W c;
        while ( (c=*pData) != EOS )
        {
          switch ( c )
          {
            case LF:
              lNumLF++;
              break;
            case SOFTLF_CHAR:
              if (*(pData+1) != SOFTLF_CHAR )
              {
                lNumLF++;
              } /* endif */
              break;
          } /* endswitch */
          pData++;
        } /* endwhile */
      }
      else
      {
        while ( *pData != EOS )
        {
          if ( *pData == LF )
          {
            lNumLF++;
          } /* endif */
          pData++;
        } /* endwhile */
      } /* endif */

    /********************************************************************/
    /* adjust if segment is 1st alignment of file and lf's added at     */
    /* begin of segment                                                 */
    /********************************************************************/
    if ( ulSegNum == pVisDoc->ulFirstAlign )
    {
      lNumLF -= pVisDoc->usAddLFatBegin;
    } /* endif */
  }
  else
  {
    lNumLF = 0;
  } /* endif */

  return (lNumLF);
} /* end of function ITMSegCountLF */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     VisDocFillLF                                             |
//+----------------------------------------------------------------------------+
//|Function call:     VisDocFIllLF(pITMIDa, usSrcStart, usSrcEnd,              |
//|                                         usTgtStart, usTgtEnd)              |
//+----------------------------------------------------------------------------+
//|Description:       fill structure which remembers how many lf's are         |
//|                   originally at end of each segment                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PITMIDA      pITMIda,                                    |
//|                   USHORT      usSrcStart                                   |
//|                   USHORT      usSrcEnd,                                    |
//|                   USHORT      usTgtStart                                   |
//|                   USHORT      usTgtEnd                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   void                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     count lf's in src doc                                    |
//|                   count lf's in tgt doc                                    |
//+----------------------------------------------------------------------------+
VOID
VisDocFillLF
(
  PITMIDA      pITMIda,
  ULONG        ulSrcStart,
  ULONG        ulSrcEnd,
  ULONG        ulTgtStart,
  ULONG        ulTgtEnd
)
{
  ITMDocCountLF ( &(pITMIda->TBSourceDoc),
                  &(pITMIda->stVisDocSrc),
                  ulSrcStart, ulSrcEnd );

  ITMDocCountLF ( &(pITMIda->TBTargetDoc),
                  &(pITMIda->stVisDocTgt),
                  ulTgtStart, ulTgtEnd );

} /* end of function VisDocFillLF */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMDocCountLF                                            |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       fill pVisDoc->pbLFNums in each segment in given range    |
//|                   count number of lf's     at end of segment               |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT  pDoc,                                       |
//|                   PITMVISDOC   pVisDoc,                                    |
//|                   USHORT       usStartSeg                                  |
//|                   USHORT       usEndSeg                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   void                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     get segment data ptr of startseg                         |
//|                   while not at last segment                                |
//|                     count lf's at end of seg (skip blanks)                 |
//|                     remember it in pVisDoc->pbLFNum                        |
//|                     get next segment                                       |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
VOID
ITMDocCountLF
(
  PTBDOCUMENT  pDoc,
  PITMVISDOC   pVisDoc,
  ULONG        ulStartSeg,
  ULONG        ulEndSeg
)
{
  PTBSEGMENT   pSeg;
  ULONG        ulSegNum;
  PSZ_W        pData;
  USHORT       usLFNum;

  ulSegNum =  ulStartSeg;
  pSeg = EQFBGetSegW(pDoc, ulSegNum);
  while ( ulSegNum <= ulEndSeg )
  {
    pData = pSeg->pDataW + pSeg->usLength - 1;
    usLFNum = 0;
    while ( pData && ((*pData == BLANK ) || (*pData == LF) ))
    {
      if ( *pData == LF )
      {
        usLFNum++;
      } /* endif */
      pData--;
    } /* endwhile */
    pVisDoc->pbLFNum[ulSegNum] = (BYTE) usLFNum;
    ulSegNum++;
    pSeg = EQFBGetSegW(pDoc, ulSegNum);
  } /* endwhile */

  return ;
} /* end of function ITMDocCountLF */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMFuncParallel                                          |
//+----------------------------------------------------------------------------+
//|Function call:     ITMFuncParallel(pITMIda, pVisDoc)                        |
//+----------------------------------------------------------------------------+
//|Description:       if parallel is switched on or off, this is called        |
//+----------------------------------------------------------------------------+
//|Parameters:        PITMIDA    pITMIda,                                      |
//|                   PITMVISDOC pVisDoc                                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     set hour glass                                           |
//|                   if parallel is on                                        |
//|                     delete all added linefeeds                             |
//|                     turn parallel off                                      |
//|                   else                                                     |
//|                     add lf's                                               |
//|                     turn parallel on                                       |
//|                   endif                                                    |
//|                   redraw all                                               |
//|                   position cursor at begin of current segment              |
//|                   force recompute of all start-stop tables                 |
//|                   reset mouse ptr                                          |
//+----------------------------------------------------------------------------+
VOID
ITMFuncParallel
(
   PITMIDA    pITMIda,
   PITMVISDOC pVisDoc
)
{
  PTBDOCUMENT  pDoc, pTwinDoc;

  pDoc = pVisDoc->pDoc;
  if ( pDoc->docType == VISSRC_DOC )
  {
    pTwinDoc = &(pITMIda->TBTargetDoc);
  }
  else
  {
    pTwinDoc = &(pITMIda->TBSourceDoc);
  } /* endif */

  if ( pITMIda->fParallel )
  {
    /******************************************************************/
    /* delete added linefeeds                                         */
    /******************************************************************/
    ITMDelLF (pITMIda, 1, pITMIda->TBSourceDoc.ulMaxSeg ,
                       1, pITMIda->TBTargetDoc.ulMaxSeg );

    if (pDoc->fAutoLineWrap && pDoc->fLineWrap )
    {
      EQFBSoftLFRemove(pDoc);
      EQFBSoftLFRemove(pTwinDoc);
      EQFBSoftLFInsert(pDoc);
      EQFBSoftLFInsert(pTwinDoc);
    } /* endif */

    pITMIda->fParallel = FALSE;
  }
  else
  {
    pITMIda->fParallel = TRUE;
    ITMAdjustLF ( pITMIda,
                  1, (pITMIda->TBSourceDoc.ulMaxSeg - 1),
                  1, (pITMIda->TBTargetDoc.ulMaxSeg - 1));
  } /* endif */

  pITMIda->TBSourceDoc.Redraw |= REDRAW_ALL;        // redraw all
  pITMIda->TBTargetDoc.Redraw |= REDRAW_ALL;




  EQFBGotoSeg( pTwinDoc, pTwinDoc->TBCursor.ulSegNum,
                     pTwinDoc->TBCursor.usSegOffset);
  EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum,
                     pDoc->TBCursor.usSegOffset);

  /*****************************************************************/
  /* force recompute of all start-stop tables                      */
  /*****************************************************************/
  ITMPrepRedisp( &pITMIda->TBSourceDoc );
  ITMPrepRedisp( &pITMIda->TBTargetDoc );
} /* end of function ITMFuncParallel(PITMIDA) */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     IncreaseBy                                               |
//+----------------------------------------------------------------------------+
//|Function call:     IncreaseBy(usEndLF)                                      |
//+----------------------------------------------------------------------------+
//|Description:       check how many lf's to add to assure that at least       |
//|                   2 lf's are at end of seg                                 |
//+----------------------------------------------------------------------------+
//|Parameters:        USHORT   usEndLF                                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       number of LF's that must be added at end                 |
//+----------------------------------------------------------------------------+
//|Function flow:     switch( number of lf's currently at end of seg )         |
//|                    case 0: return 2                                        |
//|                    case 1: return 1                                        |
//|                    default: return 0                                       |
//+----------------------------------------------------------------------------+
USHORT
IncreaseBy
(
  USHORT   usEndLF
)
{
  USHORT   usLFatEnd;

  /********************************************************************/
  /* assure that at end at least 2 lf's exist                         */
  /********************************************************************/
  switch ( usEndLF )
  {
    case  0:
      usLFatEnd = 2;
      break;
    case  1:
      usLFatEnd = 1;
      break;
    default :
      usLFatEnd = 0;
      break;
  } /* endswitch */
  return (usLFatEnd);
} /* end of function IncreaseBy */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMFind1stAlign(pITMIda )                                |
//+----------------------------------------------------------------------------+
//|Function call:     ITMFind1stAlign(pITMIda)                                 |
//+----------------------------------------------------------------------------+
//|Description:       find 1st aligned segment pair                            |
//|                   fill segnums in visdoc structures                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PITMIDA   pITMIda                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   void                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     start at begin of alignment index                        |
//|                   loop til 1st aligment which is not 0:1 or 1:0            |
//|                   remember this in the ulFirstAlign var of each doc        |
//+----------------------------------------------------------------------------+
static VOID
ITMFind1stAlign
(
   PITMIDA   pITMIda
)
{
  PALLALIGNED   pAligned;
  ULONG         ulIndex;

  pAligned = &(pITMIda->Aligned);
  ulIndex = 1;
  while ( ((pAligned->pbType[ulIndex] == NUL_ONE)
          || (pAligned->pbType[ulIndex] == ONE_NUL)  )
         && (ulIndex < pAligned->ulUsed) )
  {
    ulIndex++;
  }

  pITMIda->stVisDocSrc.ulFirstAlign = pAligned->pulSrc[ulIndex];
  pITMIda->stVisDocTgt.ulFirstAlign = pAligned->pulTgt1[ulIndex];
  pITMIda->stVisDocSrc.usAddLFatBegin = 0;
  pITMIda->stVisDocTgt.usAddLFatBegin = 0;

} /* end of function ITMFind1stAlign(pITMIda ) */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     GetCurVisSeg                                             |
//+----------------------------------------------------------------------------+
//|Function call:     GetCurVisSeg(pDoc, ulSegNum)                             |
//+----------------------------------------------------------------------------+
//|Description:       get ptr to current segment                               |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT  pDoc                                        |
//|                   PUSHORT      pulSegNum                                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   PTBSEGMENT                                               |
//+----------------------------------------------------------------------------+
//|Returncodes:       pSeg   ptr to current vis segment                        |
//+----------------------------------------------------------------------------+
//|Function flow:     go back til begin of joined segment if not already there |
//|                   if shrink style, handle shrinked blocks                  |
//|                    (loop backward til begin of shrinked block)             |
//|                   if shrink style, return handle to static segment         |
//+----------------------------------------------------------------------------+
static
PTBSEGMENT
GetCurVisSeg
(
  PTBDOCUMENT  pDoc,
  PULONG      pulSegNum
)
{
   PTBSEGMENT pSeg;                            // pointer to segment
   ULONG  ulOurSegNum;

   pSeg = EQFBGetSegW( pDoc,*pulSegNum );
   while (pSeg && pSeg->SegFlags.Joined)
   {
      (*pulSegNum)--;
      pSeg = EQFBGetSegW( pDoc, *pulSegNum );
   } /* endwhile */

   /*******************************************************************/
   /* if segment is shrinked, handle shrinked blocks                  */
   /*******************************************************************/
   if ( pSeg && ISQFNOP(pSeg->qStatus) &&
      ( (pDoc->DispStyle==DISP_SHRINK) || (pDoc->DispStyle==DISP_COMPACT)) )
   {
     ulOurSegNum = *pulSegNum;
     /****************************************************************/
     /* loop backward til the 1st seg of shrinked block              */
     /****************************************************************/
     while ( pSeg && ISQFNOP(pSeg->qStatus)
                  && (!pSeg->SegFlags.Joined))
     {
       ulOurSegNum --;
       pSeg = EQFBGetSegW( pDoc, ulOurSegNum);    //get segment
     } /* endwhile */

     ulOurSegNum ++;
     *pulSegNum = ulOurSegNum;                 //point to 1st shrinked
   } /* endif */
   /*******************************************************************/
   /* if display style is SHRINK or COMPACT return pointer to         */
   /* static segment.                                                 */
   /*******************************************************************/
   pSeg = &ITMShrinkSeg;
   pSeg->ulSegNum = *pulSegNum;
   pSeg->pDataW = pDoc->szOutTagLFAbbrW;
   pSeg->usLength = (USHORT)(UTF16strlenCHAR (pSeg->pDataW));

   return( pSeg );

} /* end of function GetCurVisSeg */



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     VisDocAddSoftLF                                          |
//+----------------------------------------------------------------------------+
//|Function call:     VisDocAddSoftLF(pDoc, pITMIda)                           |
//+----------------------------------------------------------------------------+
//|Description:       if autolinewrap is on, add softlf's to given doc         |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc ,                                       |
//|                   PITMIDA     pITMIda                                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if automatic linewrap is on                              |
//|                     set right margin to window width                       |
//|                     insert soft lf's in total document                     |
//|                     if parallel is ON and in both docs already soft lf'S   |
//|                      inserted:                                             |
//|                       delete parallel LF's and insert them again           |
//+----------------------------------------------------------------------------+
VOID
VisDocAddSoftLF
(
   PTBDOCUMENT pDoc,
   PITMIDA     pITMIda
)
{
  if (pDoc->fLineWrap && pDoc->fAutoLineWrap )
  {
    pDoc->sRMargin = (SHORT)(pDoc->lScrnCols);
    pDoc->pUserSettings->sRMargin = (SHORT)(pDoc->lScrnCols);

    EQFBSoftLFInsert(pDoc);
    /*****************************************************************/
    /* add parallel lf's only if both docs have already the softlf'S  */
    /******************************************************************/
    if ( pITMIda->fParallel &&
       pITMIda->TBSourceDoc.fLineWrap && pITMIda->TBSourceDoc.fAutoLineWrap &&
       pITMIda->TBTargetDoc.fLineWrap && pITMIda->TBTargetDoc.fAutoLineWrap )
    {
      /******************************************************************/
      /* delete added linefeeds and add them correctly                  */
      /******************************************************************/
      ITMDelLF (pITMIda, 1, pITMIda->TBSourceDoc.ulMaxSeg ,
                         1, pITMIda->TBTargetDoc.ulMaxSeg );

      ITMAdjustLF ( pITMIda, 1,
                    (pITMIda->TBSourceDoc.ulMaxSeg - 1),
                     1,
                    (pITMIda->TBTargetDoc.ulMaxSeg - 1));
    } /* endif */


  } /* endif */

  return ;
} /* end of function VisDocAddSoftLF */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMAutoWrap                                              |
//+----------------------------------------------------------------------------+
//|Function call:     ITMAutoWrap(pITMIda, fAutoWrap)                          |
//+----------------------------------------------------------------------------+
//|Description:       activate / deactivate autolinewrapping                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PITMIDA     pITMIda,                                     |
//|                   BOOL        fAutoWrap                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if automatic linewrap should be turned ON                |
//|                     set indicator in pDoc and add softLF's in both docs    |
//|                   else                                                     |
//|                     set indicator in pDoc to FALSE and                     |
//|                     remove softLF's in both docs                           |
//|                     if parallel is ON, adjust parallel linefeeds           |
//|                     force recompute of all start-stop tables               |
//|                     position at active segment                             |
//+----------------------------------------------------------------------------+

VOID  ITMAutoWrap
(
   PITMIDA  pITMIda,
   BOOL     fAutoWrap
)
{
  /********************************************************************/
  /* activate/deactivate wrapping                                     */
  /********************************************************************/
  if ( fAutoWrap )
  {
    pITMIda->TBSourceDoc.fLineWrap =
    pITMIda->TBSourceDoc.fAutoLineWrap = TRUE;
    pITMIda->TBTargetDoc.fLineWrap =
    pITMIda->TBTargetDoc.fAutoLineWrap = TRUE;

    VisDocAddSoftLF( &pITMIda->TBSourceDoc, pITMIda );
    VisDocAddSoftLF( &pITMIda->TBTargetDoc, pITMIda );
  }
  else
  {
    pITMIda->TBSourceDoc.fLineWrap =
    pITMIda->TBSourceDoc.fAutoLineWrap = FALSE;
    pITMIda->TBTargetDoc.fLineWrap =
    pITMIda->TBTargetDoc.fAutoLineWrap = FALSE;
    EQFBSoftLFRemove(&pITMIda->TBSourceDoc);
    EQFBSoftLFRemove(&pITMIda->TBTargetDoc);
    /*****************************************************************/
    /* adjust for parallel                                            */
    /******************************************************************/
    if ( pITMIda->fParallel )
    {
      /******************************************************************/
      /* delete added linefeeds and add them correctly                  */
      /******************************************************************/
      ITMDelLF (pITMIda, 1, pITMIda->TBSourceDoc.ulMaxSeg ,
                         1, pITMIda->TBTargetDoc.ulMaxSeg );

      ITMAdjustLF ( pITMIda, 1,
                   (pITMIda->TBSourceDoc.ulMaxSeg - 1),
                    1,
                    (pITMIda->TBTargetDoc.ulMaxSeg - 1));
    } /* endif */


  } /* endif */

  ITMPrepRedisp( &pITMIda->TBSourceDoc );
  ITMPrepRedisp( &pITMIda->TBTargetDoc );

  EQFBGotoSeg( &pITMIda->TBSourceDoc,
               pITMIda->TBSourceDoc.TBCursor.ulSegNum,
               pITMIda->TBSourceDoc.TBCursor.usSegOffset);
  EQFBGotoSeg( &pITMIda->TBTargetDoc,
               pITMIda->TBTargetDoc.TBCursor.ulSegNum,
               pITMIda->TBTargetDoc.TBCursor.usSegOffset);

}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ITMPrepRedisp                                            |
//+----------------------------------------------------------------------------+
//|Function call:     ITMPrepRedisp(PTBDOCUMENT pDoc)                          |
//+----------------------------------------------------------------------------+
//|Description:       force recompute of all start-stop tables                 |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT  pDoc                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     for all segments of doc:                                 |
//|                     free segment start-stop table                          |
//+----------------------------------------------------------------------------+

static void
ITMPrepRedisp( PTBDOCUMENT pDoc )
{
  ULONG        ulSegNum;
  PTBSEGMENT   pSeg;
  /*****************************************************************/
  /* force recompute of all start-stop tables                      */
  /*****************************************************************/
  for (ulSegNum = 1 ;ulSegNum < (pDoc->ulMaxSeg)  ; ulSegNum++ )
  {
    pSeg = EQFBGetSegW( pDoc, ulSegNum );
    if ( pSeg )
    {
       UtlAlloc( (PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG); // free old segment table
       if (pSeg->pusHLType) UtlAlloc((PVOID *)&(pSeg->pusHLType),0L,0L,NOMSG);
    } /* endif */
  } /* endfor */

}


static void
ITMStringDelUpToNumLF
(
  PSZ_W   pStartData,
  LONG    lNumLF
)
{
  PSZ_W  pData;

  pData = pStartData + UTF16strlenCHAR(pStartData) - 1; //pt to end of string
  /****************************************************************/
   /* go back while lf's or blanks                                 */
   /****************************************************************/
   while ( pData && ((*pData == BLANK ) || (*pData == LF) ))
   {
     pData--;
   } /* endwhile */
  // leave only usNumLF many linefeeds at end of segment;
  if ( !lNumLF )                // if usNumLF == 0
  {
    pData++;                     //pts one past the last character
    while ( *pData == BLANK )
    {
      pData++;
    }  /* endwhile */
  } /* endif */
  while ( lNumLF )              // if usNumLF >= 1
  {
    if ( *pData == LF )
    {
      lNumLF--;
    } /* endif */
    pData++;
  } /* endwhile */
  /****************************************************************/
  /* now pData points to new end of segment                       */
  /****************************************************************/
  *pData = EOS;
  return;
}
