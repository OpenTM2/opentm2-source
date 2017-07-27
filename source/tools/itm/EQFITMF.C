/*! \file
	Description: Program to find longest common subsequence

	Copyright Notice:
	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved

	The function ITMLCS corresponds to LCS (eqfbfuzz.c)
				 ITMFindMiddleSnake  to FindMiddleSnake
				 ITMSnake           to  Snake
	The only difference is, that in this file the
		PFUZZYTOK pTOkenList is a HUGE ptr;
	This is necessary because we need halloc in eqfitm.c
	   (halloc is used if pITMNOPSegs->usUsed >= 4096 )
	Changes in the function logic or error fixes have to be done
	   in both files!!
*/
//#define INITTADUMMYTAG            // initialize dummy tags
//#define INCL_EQF_FOLDER           // folder functions
#define INCL_EQF_MORPH            // morphologic functions

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_EDITORAPI        // editor API
#include <eqf.h>                  // General Translation Manager include file

#include "EQFITM.H"
#include <malloc.h>

/**********************************************************************/
/* the following structure specifies a 'snake' from the point (x,y)   */
/* to the point (u,v)                                                 */
/**********************************************************************/
typedef struct _SNAKEPTS
{
  LONG  lX;                         // x-edge of start of diagonal
  LONG  lY;                         // y-edge of start of diagonal
  LONG  lU;                         // x-edge of stop of diagonal
  LONG  lV;                         // y-edge of stop of diagonal
} SNAKEPTS, *PSNAKEPTS;

static LONG  ITMSnake ( FUZZYTOK HUGE *, FUZZYTOK HUGE *, LONG, LONG, LONG, LONG );
static USHORT ITMFindMiddleSnake( PFUZZYTOK, PFUZZYTOK, PFUZZYTOK, PFUZZYTOK,
                             LONG , LONG , PSNAKEPTS, PITMIDA );

#ifdef RAS400_ITM
static LONG   ITMCheckUniqueIdentifier(PITMIDA);
static BOOL   ITMAddToNopCnt(PITMNOPCOUNT,SHORT, SHORT,
                             PSZ_W, LONG,USHORT, BOOL);

static BOOL   IdentsAreEqual ( SHORT, SHORT, SHORT, PSZ_W, PSZ_W, LONG );
static LONG   ITMCompareIdentifierList(PITMIDA, PITMNOPCOUNT, PITMNOPCOUNT);
static VOID   ITMBuildIdentifierList ( PITMIDA, PTBDOCUMENT, PITMNOPCOUNT,
                                       PSZ_W, SHORT, ULONG );
static VOID ITMCheckSlashTok (  PTOKENENTRY ,  PFLAGOFFSLIST);
#endif

#ifdef ITMTEST
  FILE *fOut;                          // test output
#endif

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMFindMiddleSnake
//------------------------------------------------------------------------------
// Function call:     ITMFindMiddleSnake( pTokenList1,pBackList1,
//                                     pTOkenList2,pBackList2,
//                                     sLenStr1,sLenStr2,
//                                     &Midsnake)
//------------------------------------------------------------------------------
// Description:       find middle snake according E.W.Myers,
//                           using      D-band
//                    ('An O(ND) difference algorithm and its variations' by
//                     E.W.Myers)
//------------------------------------------------------------------------------
// Parameters:        PFUZZYTOK pTokenList1,        forward tokenlist of str 1
//                    PFUZZYTOK pBackList1,         backward tokenlist of str1
//                    PFUZZYTOK pTokenList2,        forward tokenlist of str 2
//                    PFUZZYTOK pBackList2,         backward tokenlist of str2
//                    LONG     lLenStr1,            number of tokens in str 1
//                    LONG     lLenStr2,            number of tokens in str2
//                    PSNAKEPTS pMidSnake           resulting middlesnake
//                    PITMIDA   pITMIda             ptr to ITM ida
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       True if at least 2 tokens match
//                    FALSE if not token match at all
//------------------------------------------------------------------------------
// Side effects:      MidSnake is filled with start (x,y) and stop (u,v) of
//                    middle snake
//------------------------------------------------------------------------------
// Function flow:     if both strings have only one token
//                      if data are equal
//                        set Midsnake, set fFound
//                      else
//                        set length of shortest edit script to 2
//                      endif
//                    else
//                      allocate space for forward and backward D-band
//                      init with -1
//                      force that string2 is the longer one
//                      init for compare loop D, sDelta, psForward, psBackward
//                      do
//                       find forward D-path
//                       if forward and backward path overlap
//                          set overlapping forward path to be the middlesnake
//                       find backward D-path
//                       if forward and backward path overlap
//                          set overlapping backword path to be the middlesnake
//                      while middlesnake not found and
//                            not already at other end of range
//                      reset longer/shorter string to original setting
//                      free allocated space
//                    endif
//                    return length of shortest edit script
//------------------------------------------------------------------------------

static USHORT
ITMFindMiddleSnake
(
  PFUZZYTOK pTokenList1,
  PFUZZYTOK pBackList1,
  PFUZZYTOK pTokenList2,
  PFUZZYTOK pBackList2,
  LONG     lLenStr1,
  LONG     lLenStr2,
  PSNAKEPTS pMidSnake,              // resulting middle snake
  PITMIDA   pITMIda
)
{
  LONG  HUGE * psForward;            // best path
  LONG  HUGE * psBackward = NULL;       // best path
  LONG         lDelta;                 // difference between two items
  LONG         lD;                     // index
  LONG         lK;                      // index
  BOOL         fFound= FALSE;
  PFUZZYTOK    pTestToken;
  BOOL         fOK = TRUE;
  BOOL         fChanged = FALSE;
  LONG         lY;                     // y edge of start of snake
  BOOL         fDeltaIsOdd;            // true if Delta odd,false if even
  USHORT       usSESLen = 0;
  LONG         lLag;
  LONG         lAllocLen;
  LONG         lLen;
  LONG         lMax;
  HWND         hwndTemp = pITMIda->hwnd;

  /********************************************************************/
  /* separate handling if sLenStr1 = sLenStr2 = 1                     */
  /********************************************************************/
  if ( (lLenStr1 == 1) && (lLenStr2 == 1 ) )
  {
    if ( (pTokenList1[0].ulHash == pTokenList2[0].ulHash) &&
        (UTF16strncmp(pTokenList1[0].pData, pTokenList2[0].pData, 1)== 0 ) )
    {
      fFound = TRUE;
      pMidSnake->lX = 0;
      pMidSnake->lY = 0;
      pMidSnake->lV = 1;
      pMidSnake->lU = 1;
    }
    else
    {
      usSESLen = 2;
    } /* endif */
  }
  else
  {
    /********************************************************************/
    /* allocate enough space to hold rows of matrix pointers            */
    /********************************************************************/

    lLen =  lLenStr1 + lLenStr2 + 7;
    lAllocLen = lLen * sizeof(LONG);
    if ( lAllocLen > 0xFFF0 )
    {
      ALLOCHUGE( psForward, LONG*, lLen, sizeof(LONG) );
      ALLOCHUGE( psBackward, LONG*, lLen, sizeof( LONG) );
      if ( (!psForward) || !psBackward )
      {
        fOK = FALSE;
      } /* endif */
    }
    else
    {
      fOK = UtlAlloc( (PVOID *)&psForward,
                      0L,
                      (LONG)  lLen * sizeof(LONG),
                      ERROR_STORAGE );
      if ( fOK  )
      {
        fOK = UtlAlloc( (PVOID *)&psBackward,
                        0L,
                        (LONG)  lLen * sizeof(LONG),
                        ERROR_STORAGE );
      } /* endif */
    } /* endif */

    lMax = (lLenStr1 + lLenStr2 ) / 2;
    lLag =  lMax + 3;

    if ( fOK )
    {
      /******************************************************************/
      /* put it in a sequence that the string named with '1' is the     */
      /* shorter one, this requires our algorithm                       */
      /******************************************************************/
      if (lLenStr2 < lLenStr1 )
      {
        pTestToken = pTokenList2;
        pTokenList2 = pTokenList1;
        pTokenList1 = pTestToken;
        pTestToken = pBackList2;
        pBackList2 = pBackList1;
        pBackList1 = pTestToken;
        lK = lLenStr2;
        lLenStr2 = lLenStr1;
        lLenStr1 = lK;
        fChanged = TRUE;
      } /* endif */
      /****************************************************************/
      /* init the pointers                                            */
      /* use for loop because memset works only on ints....           */
      /****************************************************************/
//      memset( psForward, -1l, lLen * sizeof(LONG));
//      memset( psBackward, -1l, lLen * sizeof(LONG));
      {
        LONG  i;
        LONG  HUGE * psTemp;                               // best path
        psTemp = psForward;
        for ( i=0; i<lLen ;i++ )
        {
          *psTemp++ = -1l;
        } /* endfor */
        psTemp = psBackward;
        for ( i=0; i<lLen ;i++ )
        {
          *psTemp++ = -1l;
        } /* endfor */
      }
      lDelta = lLenStr2 - lLenStr1;
      if ( lDelta & 0x01 )
      {
        fDeltaIsOdd = TRUE;
      }
      else
      {
        fDeltaIsOdd = FALSE;
      } /* endif */

      lD = -1;
      psForward [1+lLag] = 0;
      psBackward [1+lLag] = 0;
      /****************************************************************/
      /* start divide&conquer                                         */
      /****************************************************************/
      while ( !fFound && lD <= lMax )
      {
        lD++;
        lK = -lD -2;
        /**************************************************************/
        /* find forward path                                          */
        /**************************************************************/
        while ( (!pITMIda->fKill) && !fFound && lK <= (lD - 2) )
        {
          lK += 2;
          if ( lK == -lD )
          {
            lY = psForward[lK+1+lLag];
          }
          else if (lK == lD )
          {
            lY = psForward[lK-1+lLag]+1;
          }
          else if (psForward[lK-1+lLag] < psForward[lK+1+lLag])
          {
            lY = psForward[lK+1+lLag];
          }
          else
          {
            lY = psForward[lK-1+lLag]+1;
          } /* endif */
          psForward[lK+lLag] = ITMSnake(pTokenList1, pTokenList2,
                                     lLenStr1, lLenStr2, lK, lY );
          if ( fDeltaIsOdd &&
                ( (1 - lD ) <= (lDelta - lK )) &&
                ((lDelta - lK ) <= (lD - 1) ) )
          {
            if  (psForward[lK+lLag] >=
                 lLenStr2 - psBackward[ lDelta - lK +lLag] )
            {
              usSESLen = (USHORT)((2l * lD) - 1l);
              fFound = TRUE;
              if ( (lD == 1l) && (lY == psForward[lK+lLag])
                   && (lY == lLenStr2) )
              {
                //last token of longer string doesn't match
                pMidSnake->lX = 0;
                pMidSnake->lY = 0;
                pMidSnake->lV = lLenStr2 - 1l;
                pMidSnake->lU = lLenStr1;
              }
              else
              {
                //set middlesnake, stop loop
                pMidSnake->lX = lY - lK;
                pMidSnake->lY = lY;
                pMidSnake->lV = psForward[lK+lLag];
                pMidSnake->lU = (pMidSnake->lV ) - lK;
              } /* endif */
            } /* endif */
          } /* endif */

          UtlDispatch();
          pITMIda->fKill = !( (hwndTemp == HWND_FUNCIF) || (hwndTemp && WinIsWindow( (HAB) NULL, hwndTemp )));   // Test MK Non-DDE
//          ulDisps++;
        } /* endwhile */
        /**************************************************************/
        /* calculate backward D-path                                  */
        /**************************************************************/
        lK = -lD -2l;
        while ( (!pITMIda->fKill) && !fFound && lK <=(lD-2l) )
        {
          lK += 2l;
          if ( lK == -lD )
          {
            lY = psBackward[lK+1l+lLag];
          }
          else if (lK == lD )
          {
            lY = psBackward[lK-1l+lLag]+1l;
          }
          else if (psBackward[lK-1l+lLag] < psBackward[lK+1l+lLag])
          {
            lY = psBackward[lK+1+lLag];
          }
          else
          {
            lY = psBackward[lK-1+lLag]+1l;
          } /* endif */
          psBackward[lK+lLag] = ITMSnake(pBackList1, pBackList2,
                                     lLenStr1, lLenStr2, lK, lY );
          if ( !fDeltaIsOdd &&
               ( -lD <= (lDelta -lK) ) &&
               ((lDelta -lK) <= lD)   )
          {
            if ( psForward[lDelta- lK +lLag] >=
                 lLenStr2 - psBackward[lK +lLag])
            {
              usSESLen = (USHORT) (2 * lD);
              //set middlesnake, stop loop
              pMidSnake->lV = lLenStr2 - lY ;
              pMidSnake->lU = (pMidSnake->lV) + lK - lDelta;
              pMidSnake->lY = lLenStr2 - psBackward[lK+lLag];
              pMidSnake->lX = (pMidSnake->lY) + lK - lDelta;
              fFound = TRUE;
            } /* endif */
          } /* endif */

          UtlDispatch();
          pITMIda->fKill = !( (hwndTemp == HWND_FUNCIF) || (hwndTemp && WinIsWindow( (HAB) NULL, hwndTemp ))); // Test MK Non-DDE
//          ulDisps++;
        } /* endwhile */

      } /* endwhile */

      if ( (!pITMIda->fKill) && fChanged )
      {
        /******************************************************************/
        /* interchange x with y and u with v                              */
        /******************************************************************/
        lK = pMidSnake->lX;
        pMidSnake->lX = pMidSnake->lY;
        pMidSnake->lY = lK;
        lK = pMidSnake->lU;
        pMidSnake->lU = pMidSnake->lV;
        pMidSnake->lV = lK;
        /******************************************************************/
        /* reset input variables to original                              */
        /******************************************************************/
        pTestToken = pTokenList2;
        pTokenList2 = pTokenList1;
        pTokenList1 = pTestToken;
        pTestToken = pBackList2;
        pBackList2 = pBackList1;
        pBackList1 = pTestToken;
        lK = lLenStr2;
        lLenStr2 = lLenStr1;
        lLenStr1 = lK;
      } /* endif */
    } /* endif */
    /******************************************************************/
    /* freeing psForward and           - psBackward                   */
    /******************************************************************/
    if ( lAllocLen > 0xFFF0 )
    {
      if ( psForward )
      {
        FREEHUGE( psForward );
        psForward = NULL;
      } /* endif */
      if ( psBackward )
      {
        FREEHUGE( psBackward );
        psBackward = NULL;
      } /* endif */
    }
    else
    {
      UtlAlloc( (PVOID *)&psForward, 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *)&psBackward, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  return ( usSESLen );
} /* end of function ITMFindMiddleSnake */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMSnake
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       find the longest diagonal sequence, i.e. SNAKE thru
//                    a matrix ....
//                    This routine will allow a O(n+m) behaviour of finding
//                    the longest sequences two string have in common..
//------------------------------------------------------------------------------
// Parameters:        PULONG     pulStr1  ptr to first string's hash values
//                    PULONG     pulStr2  ptr to second string's hash values
//                    USHORT     usLen1   length of the first string
//                    USHORT     usLen2   length of the second string
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       length of the identical sequence
//------------------------------------------------------------------------------
// Function flow:     loop through the diagonal as long as the characters of
//                    both strings are identical.
//                    Return the compounded value
//------------------------------------------------------------------------------

static LONG
ITMSnake
(
  FUZZYTOK HUGE *pFuzzyStr1,
  FUZZYTOK HUGE *pFuzzyStr2,
  LONG      sLen1,
  LONG      sLen2,
  LONG      sK,
  LONG      sY
)
{
  LONG   sX;
//USHORT usRc = 0;

  sY--;                                       /***********************/
  sLen1--;                                    /* we start at zero    */
  sLen2--;                                    /***********************/
  /********************************************************************/
  /* loop through the diagonal as long as the characters are identical*/
  /********************************************************************/
  sX = sY - sK;
//while ( (sX < sLen1) && (sY < sLen2) &&
//             (pFuzzyStr1[sX+1].ulHash == pFuzzyStr2[sY+1].ulHash) &&
//             ( usRc == 0) )
//{
//  usDatLen = max( pFuzzyStr1[sX+1].usStop - pFuzzyStr1[sX+1].usStart + 1,
//                  pFuzzyStr2[sY+1].usStop - pFuzzyStr2[sY+1].usStart + 1 );
//  usRc = strncmp(pFuzzyStr1[sX+1].pData,
//                 pFuzzyStr2[sY+1].pData, usDatLen );
//                 pFuzzyStr1[sX+1].usStop - pFuzzyStr1[sX+1].usStart + 1);
//  if ( usRc == 0 )
//  {
//    sX ++;
//    sY ++;
//  } /* endif */
//} /* endwhile */
  while ( (sX < sLen1) && (sY < sLen2) &&
          (pFuzzyStr1[sX+1].ulHash == pFuzzyStr2[sY+1].ulHash) &&
          ItmLenComp(pFuzzyStr1[sX+1].pData, pFuzzyStr2[sY+1].pData,
           (USHORT)(pFuzzyStr1[sX+1].usStop-pFuzzyStr1[sX+1].usStart+ 1),
           (USHORT)(pFuzzyStr2[sY+1].usStop-pFuzzyStr2[sY+1].usStart+ 1)) )
//          ItmCompChars(pFuzzyStr1[sX+1].pData, pFuzzyStr2[sY+1].pData ) )
  {
    sX ++;
    sY ++;
  } /* endwhile */

  sY++;
/**********************************************************************/
/* return the y value of the last match                               */
/* if nothing eq, return unchanged sY;                                */
/* if all equal, sY = usLen2                                          */
/**********************************************************************/
  return ( sY );
} /* end of function ITMSnake */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMLCS
//------------------------------------------------------------------------------
// Function call:     ITMLCS(ITMLCSStringA,ITMLCSStringB)
//------------------------------------------------------------------------------
// Description:       divide &conquer algorithm to find shortest edit script
//                    (by Hirschberg )
//------------------------------------------------------------------------------
// Parameters:        ITMLCSTOKEN LCSStringA
//                    ITMLCSTOKEN LCSStringB
//------------------------------------------------------------------------------
// Returncode type:   void
//------------------------------------------------------------------------------
// Prerequesits:      forward and backward tokenlist allocated and filled
//------------------------------------------------------------------------------
// Side effects:      'equal' token are marked with type 'MARK-EQUAL' in
//                    forward tokenlist
//------------------------------------------------------------------------------
// Function flow:     get number of tokens in A string and Bstring
//                    if both lengths are greater 0
//                       Find Middle snake
//                       if something equal
//                          calculate absolute start/stop of Middle snake
//                          set LCS strings for lower half of range
//                          call LCS for this range
//                          set mark-equals for middlesnake
//                          set LCS strings for upper half of range
//                          call LCS for this range
//                       endif
//                    endif
//------------------------------------------------------------------------------

VOID
ITMLCS
(
 ITMLCSTOKEN    LCSStringA,
 ITMLCSTOKEN    LCSStringB,
 PITMIDA        pITMIda
)
{
  LONG      sALen;                             // # of tokens in A string
  LONG      sBLen;                             // # of tokens in B string
  SNAKEPTS  MidSnake;
  LONG      lI;                                 // index in for loop
  LONG      lJ;
  ITMLCSTOKEN  LCSNextA;
  ITMLCSTOKEN  LCSNextB;
  USHORT    usSESLen;
  FUZZYTOK HUGE *pFuzzyTokA;
  FUZZYTOK HUGE *pFuzzyTokB;
  USHORT    usNewPerc;

  sALen = LCSStringA.lStop - LCSStringA.lStart ;
  sBLen = LCSStringB.lStop - LCSStringB.lStart ;
  if ( (sALen > 0) && ( sBLen > 0) )
  {
    usSESLen = ITMFindMiddleSnake (
            (FUZZYTOK HUGE *)LCSStringA.pTokenList+LCSStringA.lStart,
            (FUZZYTOK HUGE *)LCSStringA.pBackList+LCSStringA.lTotalLen-LCSStringA.lStop,
            (FUZZYTOK HUGE *)LCSStringB.pTokenList+LCSStringB.lStart,
            (FUZZYTOK HUGE *)LCSStringB.pBackList+LCSStringB.lTotalLen-LCSStringB.lStop,
            sALen, sBLen, &MidSnake, pITMIda);
    /*************************************************************/
    /* if not all tokens are different...                        */
    /*************************************************************/
    if ( (!pITMIda->fKill) && ((LONG) usSESLen < (sALen + sBLen) ))
    {
      /***************************************************************/
      /* correct MidSnake by the starting lags                       */
      /***************************************************************/
      MidSnake.lX += LCSStringA.lStart;
      MidSnake.lU += LCSStringA.lStart;
      MidSnake.lY += LCSStringB.lStart;
      MidSnake.lV += LCSStringB.lStart;
      /***********************************************************/
      /* divide again                                            */
      /***********************************************************/
      LCSNextA = LCSStringA;
      if ( MidSnake.lX < LCSNextA.lStop )
      {
        LCSNextA.lStop = MidSnake.lX;
      } /* endif */
      LCSNextB = LCSStringB;
      if ( MidSnake.lY < LCSNextB.lStop )
      {
        LCSNextB.lStop = MidSnake.lY;
      } /* endif */

      ITMLCS( LCSNextA, LCSNextB, pITMIda);
      /***************************************************************/
      /* set the mark-equals                                         */
      /***************************************************************/
      lJ = (MidSnake.lY);
      lI = (MidSnake.lX);
      while ( (lI < MidSnake.lU) && (lJ < MidSnake.lV)  )
      {
        pFuzzyTokA = (FUZZYTOK HUGE *)LCSStringA.pTokenList + lI;
        pFuzzyTokB = (FUZZYTOK HUGE *)LCSStringB.pTokenList + lJ;

        if ((pFuzzyTokA->ulHash == pFuzzyTokB->ulHash) &&
             ItmLenComp(pFuzzyTokA->pData, pFuzzyTokB->pData,
               (USHORT)(pFuzzyTokA->usStop - pFuzzyTokA->usStart + 1),
               (USHORT)(pFuzzyTokB->usStop - pFuzzyTokB->usStart + 1)) )
        {
          pFuzzyTokA->sType = MARK_EQUAL;
          pFuzzyTokB->sType = MARK_EQUAL;

          pITMIda->usAnchorCount ++;
          usNewPerc = (SHORT)(((LONG)pITMIda->usAnchorCount*50L)
                                /pITMIda->itmSrcNop.ulUsed);
          if ( (usNewPerc != pITMIda->usOldPerc) && (usNewPerc < 25) )
          {
            pITMIda->usOldPerc = usNewPerc;
            WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
               MP1FROMSHORT(usNewPerc + (pITMIda->usStartSlider)),
               NULL );
          } /* endif */
//        ((FUZZYTOK huge *)LCSStringA.pTokenList + lI)->sType = MARK_EQUAL;
//        ((FUZZYTOK huge *)LCSStringB.pTokenList + lJ)->sType = MARK_EQUAL;

        }
        else
        {
#ifdef ITMTEST
          fOut      = fopen ( "ITMSTAT.OUT", "a" );
          fprintf (fOut, "!! snake pt is not equal\n");
          fclose( fOut      );
#endif
        } /* endif */
        lJ++;
        lI++;
      } /* endwhile */

      LCSNextA = LCSStringA;
      if ( MidSnake.lU > LCSNextA.lStart )
      {
        LCSNextA.lStart = MidSnake.lU;
      } /* endif */
      LCSNextB = LCSStringB;
      if ( MidSnake.lV > LCSNextB.lStart )
      {
        LCSNextB.lStart = MidSnake.lV;
      } /* endif */

      ITMLCS( LCSNextA, LCSNextB, pITMIda);
    } /* endif */
  } /* endif */

} /* end of function ITMLCS */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ItmCompChars
//------------------------------------------------------------------------------
// Function call:     ItmCompChars(pData1, pData2)
//------------------------------------------------------------------------------
// Description:       compare 2 strings; equal if only differences in the
//                    number of leading or ending blanks
//------------------------------------------------------------------------------
// Parameters:        PSZ    pData1
//                    PSZ    pData2
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE            if equal
//                    FALSE           if not equal
//------------------------------------------------------------------------------
// Function flow:     skip leading blanks in both strings
//                    loop thru both strings as long they are equal
//                    if equal
//                       check that in longer string only blanks are
//                       remaining
//------------------------------------------------------------------------------

// P403330 BEGIN
BOOL isSpaceOrTab(PSZ_W pData)
{
	return (*pData==BLANK || *pData=='\t');
}
// P403330 END

BOOL
ItmLenComp
(
   PSZ_W     pData1,            // data 1 string
   PSZ_W     pData2,            // to be compared with data2
   USHORT    usLen1,            // length to be compared in data1
   USHORT    usLen2             // length to be compared in data2
)
{  BOOL      fEqual = TRUE;

   /*******************************************************************/
   /* skip leading blanks                                             */
   /*******************************************************************/
   while ( usLen1 &&  isSpaceOrTab(pData1) )
   {
     pData1++;
     usLen1--;
   } /* endwhile */
   while ( usLen2 && isSpaceOrTab(pData2) )
   {
     pData2++;
     usLen2--;
   } /* endwhile */
   /*******************************************************************/
   /* compare remaining characters                                    */
   /* stops if either at end of one string or strings are not equal   */
   /*******************************************************************/
   while ( usLen1 && usLen2 && fEqual )
   {
     if ( *pData1==*pData2 || ( isSpaceOrTab(pData1) && isSpaceOrTab(pData2) ) )
     {
		// P403330 BEGIN
        if( isSpaceOrTab(pData1) )
		{
			while(usLen1>0 && isSpaceOrTab(pData1) )
			{
				pData1++;
				usLen1--;
			}

			while(usLen2>0 && isSpaceOrTab(pData2) )
			{
				pData2++;
				usLen2--;
			}

			continue;
		}
		// P403330 END

         pData1++;
         pData2++;
         usLen1--;
         usLen2--;
     }
     else
     {
       fEqual = FALSE;
     } /* endif */
   } /* endwhile */
   if ( fEqual )
   {
     if ( usLen1 )
     {
       /***************************************************************/
       /* pData2 is at eos, check that only blanks remaing in pData1  */
       /***************************************************************/
       while ( usLen1 && fEqual )
       {
         if ( isSpaceOrTab(pData1) )
         {
           pData1++;
           usLen1--;
         }
         else
         {
           fEqual = FALSE;
         } /* endif */
       } /* endwhile */

     }
     else
     {
       /***************************************************************/
       /* either pData1 or both are at eos                            */
       /* if pData2 is not at eos, check that only blanks are         */
       /* remaining in pData2                                         */
       /***************************************************************/
       while ( usLen2 && fEqual )
       {
         if ( isSpaceOrTab(pData2) )
         {
           pData2++;
           usLen2--;
         }
         else
         {
           fEqual = FALSE;
         } /* endif */
       } /* endwhile */
     } /* endif */
   } /* endif */

  return (fEqual);
} /* end of function ItmLenComp */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMCheckQuality
//------------------------------------------------------------------------------
// Function call:     ITMCheckQuality
//------------------------------------------------------------------------------
// Description:       check quality of current alignment
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pITMIda,
//                    PALLALIGNED  pAligned
//                    USHORT       usIndex
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    save this transl. with m-flag in mem.
//                    FALSE   save transl. without m-flag in memory
//------------------------------------------------------------------------------
// Function flow:     check for too different lengths of both sentences
//                    check how many none 1:1 alignments are around current
//                    check whether the unique identifiers fit in both sent.
//                    set MFlag according to trusted level
//------------------------------------------------------------------------------
BOOL
ITMCheckQuality
(
  PITMIDA  pITMIda,
  PALLALIGNED pAligned,
  ULONG    ulIndex
)
{
  BOOL   fMFlag = TRUE;
  LONG   lSrcLen = 0;
  LONG   lTgtLen = 0;
  LONG   lQuality = 100;
  ULONG  ulI = 1;
  LONG   lNumOfNone11 = 0;
  LONG   lNumOfNotUnique = 0;

#ifdef RAS400_ITM
  if ((pITMIda->sMPerCent != -1) && (pITMIda->sMPerCent != 101) )
  {
    /********************************************************************/
    /* check whether lengths differ too much !                          */
    /* if lSrcLen < 10: 0 <=lTgtLen <20 is ok                         */
    /* if lSrcLen >= 10: (1/2)*SrcLen <= TgtLen <= (1 1/2)*SrcLen is ok*/
    /********************************************************************/
    lSrcLen = UTF16strlenCHAR (pITMIda->szSourceSeg);
    lTgtLen = UTF16strlenCHAR (pITMIda->szTargetSeg);
    if ( lSrcLen < 10 )
    {
      if (lTgtLen >= 20 )
      {
        lQuality --;
      } /* endif*/
    }
    else
    {
      if ( (lSrcLen > 2*lTgtLen) ||
           ((2*lTgtLen) > (3*lSrcLen))   )
      {
        lQuality --;
      } /* endif */
    } /* endif */
    /********************************************************************/
    /* if it is not 1:1 alignment, set decrease qual. by one;          */
    /* check whether there are  NOne-1:1 alignment in the 5 alignments  */
    /* before this or in the 5 alignments after this alignment !        */
    /********************************************************************/
    if (ulIndex > 5 )
    {
      ulI = ulIndex - 5;
    } /* endif */
    while ((ulI <= ulIndex + 5) && (ulI < pAligned->ulUsed) )
    {
      if (pAligned->pbType[ulI] != ONE_ONE )
      {
        lNumOfNone11 ++;
      } /* endif */
      ulI++;
    } /* endwhile */
    if (lNumOfNone11 )
    {
      if (lQuality > lNumOfNone11 )
      {
        lQuality -= lNumOfNone11;
      }
      else
      {
        lQuality = 0;
      } /* endif */
    } /* endif */
    /********************************************************************/
    /* check whether there is a UNIQUE identifier in the sentence !     */
    /********************************************************************/
    lNumOfNotUnique = ITMCheckUniqueIdentifier(pITMIda);
    if (lQuality > lNumOfNotUnique )
    {
      lQuality -= lNumOfNotUnique;
    }
    else
    {
      lQuality = 0;
    } /* endif */
  } /* endif */
  if ( pITMIda->sMPerCent == -1 )
  {
     fMFlag = !pITMIda->fVisual;
  }
  else
  {
     if (lQuality < pITMIda->sMPerCent  )
     {
           fMFlag = TRUE;
     }
     else
     {
           fMFlag = FALSE;
     }  /* endif */
  } /* endif */
#endif
  return fMFlag;
} /* end of function ITMCheckQuality */

#ifdef RAS400_ITM
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMCheckUniqueIdentifier
//------------------------------------------------------------------------------
// Function call:     ITMCheckUniqueIdentifier
//------------------------------------------------------------------------------
// Description:       build list of identifiers in sentence
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pITMIda,
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    unique identifiers found in both sentences
//                    FALSE   differences found
//------------------------------------------------------------------------------
// Function flow:     build identifier list of source sentence
//                    build identifier list of target sentence
//                    compare both lists
//------------------------------------------------------------------------------
static LONG
ITMCheckUniqueIdentifier
(
  PITMIDA  pITMIda
)
{
  LONG         lNumOfNotUnique = 0;
  PTBDOCUMENT  pSrcDoc, pTgtDoc;
  PITMNOPCOUNT pstSrcIdentCnt, pstTgtIdentCnt;
  SHORT        sLangID;
  BOOL         fOK;
  ULONG        ulOemCP = 0L;

  pSrcDoc = &(pITMIda->TBSourceDoc);
  pTgtDoc = &(pITMIda->TBTargetDoc);

  /********************************************************************/
  /* reuse NOpCnt struct to build list of identifiers in segment      */
  /********************************************************************/
  pstSrcIdentCnt = &(pITMIda->stSrcNopCnt);
  pstTgtIdentCnt = &(pITMIda->stTgtNopCnt);

  fOK = !MorphGetLanguageID( pITMIda->szSourceLang, &sLangID );
  if ( fOK )
  {
    ulOemCP = GetLangOEMCP( pITMIda->szSourceLang);
    ITMBuildIdentifierList(pITMIda, pSrcDoc,
                           pstSrcIdentCnt, pITMIda->szSourceSeg,
                           sLangID, ulOemCP);
  } /* endif */
  fOK = !MorphGetLanguageID( pITMIda->szTargetLang, &sLangID );
  if ( fOK )
  {
    ulOemCP = GetLangOEMCP(pITMIda->szTargetLang);
    ITMBuildIdentifierList(pITMIda, pTgtDoc,
                           pstTgtIdentCnt, pITMIda->szTargetSeg,
                           sLangID, ulOemCP);
  } /* endif */
  /********************************************************************/
  /* return true if same identifiers found in both lists !            */
  /********************************************************************/
  lNumOfNotUnique = ITMCompareIdentifierList(pITMIda, pstSrcIdentCnt,
                                             pstTgtIdentCnt);
  pITMIda->stSrcNopCnt.ulUsed = 0;
  pITMIda->stTgtNopCnt.ulUsed = 0;

//UtlAlloc((PVOID *)&pITMIda->stSrcNopCnt.pusOccur, 0L, 0L, NOMSG);
//UtlAlloc((PVOID *)&pITMIda->stSrcNopCnt.psTokenID, 0L, 0L, NOMSG);
//UtlAlloc((PVOID *)&pITMIda->stSrcNopCnt.psAddInfo, 0L, 0L, NOMSG);
//UtlAlloc((PVOID *)&pITMIda->stSrcNopCnt.ppData, 0L, 0L, NOMSG);
//UtlAlloc((PVOID *)&pITMIda->stSrcNopCnt.pusLen, 0L, 0L, NOMSG);
//
//UtlAlloc((PVOID *)&pITMIda->stTgtNopCnt.pusOccur, 0L, 0L, NOMSG);
//UtlAlloc((PVOID *)&pITMIda->stTgtNopCnt.psTokenID, 0L, 0L, NOMSG);
//UtlAlloc((PVOID *)&pITMIda->stTgtNopCnt.psAddInfo, 0L, 0L, NOMSG);
//UtlAlloc((PVOID *)&pITMIda->stTgtNopCnt.ppData, 0L, 0L, NOMSG);
//UtlAlloc((PVOID *)&pITMIda->stTgtNopCnt.pusLen, 0L, 0L, NOMSG);
//
//pITMIda->stSrcNopCnt.usAlloc = 0;
//pITMIda->stTgtNopCnt.usAlloc = 0;

  return lNumOfNotUnique;

} /* end of function ITMCheckUniqueIdentifier */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMBuildIdentifierList
//------------------------------------------------------------------------------
// Function call:     ITMBuildIdentifierList
//------------------------------------------------------------------------------
// Description:       build list of identifiers in sentence
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pITMIda,
//                    PTBDOCUMENT  pDoc,
//                    PITMNOPCOUNT pstIdentCnt
//                    PSZ          pData
//------------------------------------------------------------------------------
// Returncode type:   void
//------------------------------------------------------------------------------
// Function flow:     tokenize sentence
//                    while not at end of list
//                     -if text:
//                      morphologic text tokenizing
//                      recognize "special" tags ( i.e."AS/400")
//                      if identifier found: add to list
//                     -if inline-tag
//                      add to list
//------------------------------------------------------------------------------
static VOID
ITMBuildIdentifierList
(
  PITMIDA      pITMIda,
  PTBDOCUMENT  pDoc,
  PITMNOPCOUNT pstIdentCnt,
  PSZ_W        pData,
  SHORT        sLangId,
  ULONG        ulOemCP
)
{
  PTOKENENTRY   pTok;
  PCHAR_W       pRest = NULL;
  USHORT        usColPos = 0;
  CHAR_W        chTemp;
  USHORT        usListSize;
  PFLAGOFFSLIST pTermList;
  PFLAGOFFSLIST pActTerm;
  LONG          lLength;
  USHORT        i;
  SHORT         sRc;
  BOOL          fIdentFound;
  BOOL          fFound;
  CHAR_W        szMorphData[MAX_TERM_LEN];
  BOOL          fOK = TRUE;

  TATagTokenizeW( pData,
                 ((PLOADEDTABLE)pITMIda->pLoadedTable),
                 TRUE,
                 &pRest,
                 &usColPos,
                 (PTOKENENTRY) pDoc->pTokBuf,
                 TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );

  pTok = (PTOKENENTRY) pDoc->pTokBuf;
  while ( (pTok->sTokenid != ENDOFLIST) && fOK )
  {
    if ( pTok->sTokenid == TEXT_TOKEN )
    {
      usListSize =0;
      pTermList = NULL;
      chTemp =*(pTok->pDataStringW+pTok->usLength);
      *(pTok->pDataStringW+pTok->usLength) = EOS;
      sRc = MorphTokenizeW( sLangId, pTok->pDataStringW,
                           &usListSize, (PVOID *) &pTermList,
                           MORPH_FLAG_OFFSLIST, ulOemCP);
      *(pTok->pDataStringW+pTok->usLength) = chTemp;
      if ( pTermList )
      {
        pActTerm = pTermList;
        while ( pActTerm->usLen && fOK )
        {
          fIdentFound = FALSE;
          fIdentFound = GetSpecialTok(pTok, pActTerm );
          if (!fIdentFound)
          {
            ITMCheckSlashTok(pTok, pActTerm );
          }
          if (!fIdentFound)
          {
            if ( pActTerm->lFlags & TF_NUMBER )
            {
              fIdentFound = TRUE;
            } /* endif */
//            if ( (pActTerm->lFlags & TF_ALLCAPS )
//                 && (pActTerm->usLen > 1))
//            {
//              fIdentFound = TRUE;
//            } /* endif */
//            if ( (pActTerm->lFlags & TF_ABBR )
//                 && (pActTerm->usLen > 1))
//            {
//              fIdentFound = TRUE;
//            } /* endif */
          } /* endif */

          if ( fIdentFound )
          {
            /**********************************************************/
            /* check if it is already is pstIdentCnt list             */
            /**********************************************************/
            fFound = FALSE;
            i = 0;
            while ( i < pstIdentCnt->ulUsed && !fFound )
            {
              /**********************************************************/
              /* if Tokenid equal and only part of the tag is relevant  */
              /*                     or                                 */
              /* if Tokenid equal and datastring is equal               */
              /* then both toks are recognized as equal                 */
              /**********************************************************/
              if ( IdentsAreEqual(pstIdentCnt->psTokenID[i], 0,
                      0, pstIdentCnt->ppData[i],
                      pTok->pDataStringW+(pActTerm->usOffs),
                      (LONG)(pActTerm->usLen)) )
              {
                fFound = TRUE;
              } /* endif */
              i++;
            } /* endwhile */
            /**********************************************************/
            /* add to pstIdentCnt list                                */
            /* ( use tokenid = addinfo = 0 for text tokens )          */
            /**********************************************************/
            ITMAddToNopCnt (pstIdentCnt, 0, 0,
                            pTok->pDataStringW+(pActTerm->usOffs),
                            pActTerm->usLen, i, fFound);
          } /* endif */
          pActTerm++;
        } /* endwhile */
      } /* endif */
      UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG);
    }
    else
    {
      szMorphData[0] = EOS;
      lLength = ITMGetRelLength(pITMIda->pLoadedTable,
                                 pTok->usLength,
                                 pTok->sAddInfo,
                                 pTok->sTokenid, szMorphData);
      i = 0;
      fFound = FALSE;
      while ( i < pstIdentCnt->ulUsed && !fFound )
      {
        /**********************************************************/
        /* if Tokenid equal and only part of the tag is relevant  */
        /*                     or                                 */
        /* if Tokenid equal and datastring is equal               */
        /* then both toks are recognized as equal                 */
        /**********************************************************/
        if ( IdentsAreEqual(pstIdentCnt->psTokenID[i], pTok->sTokenid,
                pTok->sAddInfo, pstIdentCnt->ppData[i],
                pTok->pDataStringW, lLength) )
        {
          fFound = TRUE;
        } /* endif */
        i++;
      } /* endwhile */
      ITMAddToNopCnt (pstIdentCnt, pTok->sTokenid, pTok->sAddInfo,
                      pTok->pDataStringW, lLength, i, fFound);
    } /* endif */
    pTok++;
  } /* endwhile */

  return;
} /* end of function ITMBuildIdentifierList */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMCompareIdentifierList
//------------------------------------------------------------------------------
// Function call:     ITMCompareIdentifierList
//------------------------------------------------------------------------------
// Description:       compare lists of identifiers whether they are equal
//------------------------------------------------------------------------------
// Parameters:        PITMIDA       pITMIda,
//                    PITMNOPCOUNT  pstSrcIdentCnt,
//                    PITMNOPCPUNT  pstTgtIdentCnt
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    all idents found in both list with eq. occurrenc.
//                    FALSE   differences found in occurrence of identifiers
//------------------------------------------------------------------------------
// Function flow:     loop thru list of identifiers in source sentence
//                       pick next identifier from src sentence
//                       while not found in list of identifiers of target
//                         compare with next identifier is target list
//                       if src identifier not found: stop loop
//------------------------------------------------------------------------------
static LONG
ITMCompareIdentifierList
(
  PITMIDA       pITMIda,
  PITMNOPCOUNT  pstSrcIdentCnt,
  PITMNOPCOUNT  pstTgtIdentCnt
)
{
  BOOL   fFoundInTgt = FALSE;
  ULONG   ulTgtIndex = 0;
  ULONG   ulSrcIndex = 0;
  PSZ_W  pSrcData;
  LONG   lSrcLen = 0;
  SHORT  sAddInfo;
  SHORT  sTokenId;
  LONG   lSrcOccur = 0;
  LONG   lTgtOccur = 0;
  LONG   lNumOfNotUnique = 0;

  pITMIda;

  while ( ulSrcIndex < pstSrcIdentCnt->ulUsed  )
  {
    ulTgtIndex = 0;
    pSrcData = pstSrcIdentCnt->ppData[ulSrcIndex];
    lSrcLen = pstSrcIdentCnt->pusLen[ulSrcIndex];
    sAddInfo = pstSrcIdentCnt->psAddInfo[ulSrcIndex];
    sTokenId = pstSrcIdentCnt->psTokenID[ulSrcIndex];
    lSrcOccur = pstSrcIdentCnt->pusOccur[ulSrcIndex];
    fFoundInTgt  = FALSE;
    while ( ulTgtIndex < pstTgtIdentCnt->ulUsed && !fFoundInTgt )
    {
      /**********************************************************/
      /* if Tokenid equal and only part of the tag is relevant  */
      /*                     or                                 */
      /* if Tokenid equal and datastring is equal               */
      /* then both toks are recognized as equal                 */
      /**********************************************************/
      if ( IdentsAreEqual(pstTgtIdentCnt->psTokenID[ulTgtIndex],
                           sTokenId,
                           sAddInfo,
                           pstTgtIdentCnt->ppData[ulTgtIndex],
                           pSrcData, lSrcLen) )
      {
        /****************************************************************/
        /* check whether occurency of tok is equal in both sentences!   */
        /****************************************************************/
        lTgtOccur = pstTgtIdentCnt->pusOccur[ulTgtIndex];
        fFoundInTgt = TRUE;
        if ( lSrcOccur >= lTgtOccur )
        {
          lNumOfNotUnique += (lSrcOccur - lTgtOccur);
        }
        else
        {
          lNumOfNotUnique += (lTgtOccur - lSrcOccur);
        } /* endif */
        pstTgtIdentCnt->pusOccur[ulTgtIndex] = 0;  // checked already
      } /* endif */
      ulTgtIndex++;
    } /* endwhile */
    if ( !fFoundInTgt )
    {
      lNumOfNotUnique += pstSrcIdentCnt->pusOccur[ulSrcIndex];
    } /* endif */
    ulSrcIndex++;
  } /* endwhile */
  /********************************************************************/
  /* add numb of unique idents left over in target                    */
  /********************************************************************/
  ulTgtIndex = 0;
  while ( ulTgtIndex < pstTgtIdentCnt->ulUsed  )
  {
    lNumOfNotUnique += pstTgtIdentCnt->pusOccur[ulTgtIndex];
    ulTgtIndex ++;
  } /* endwhile */
  return (lNumOfNotUnique);
} /* end of function ITMCompareIdentifierList */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMAddToNopCnt
//------------------------------------------------------------------------------
// Function call:     ITMAddToNopCnt
//------------------------------------------------------------------------------
// Description:       add token to list of found tokens
//------------------------------------------------------------------------------
// Parameters:
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE      token is added
//                    FALSE     token could not be added ( alloc not poss)
//------------------------------------------------------------------------------
// Function flow:     if token is not already in list
//                      add new space
//                      add new token
//                    else
//                      increase occurrence of token
//------------------------------------------------------------------------------
static BOOL
ITMAddToNopCnt
(
  PITMNOPCOUNT pstIdentCnt,
  SHORT        sId,
  SHORT        sAddInfo,
  PSZ_W        pData,
  LONG         lLength,
  USHORT       i,
  BOOL         fFound
)
{
  BOOL    fOK = TRUE;

  if ( !fFound )
  {
    fOK = NopCntAlloc(pstIdentCnt);
    if (fOK)
    {
      pstIdentCnt->psTokenID[i] = sId;
      pstIdentCnt->psAddInfo[i] = sAddInfo;
      pstIdentCnt->ppData[i] = pData;
      pstIdentCnt->pusOccur[i] = 1;
      pstIdentCnt->ulUsed ++;                  // point to next entry
      pstIdentCnt->pusLen[i] = (USHORT)lLength;
    }
  }
  else
  {
    pstIdentCnt->pusOccur[i-1]++;
  } /* endif */
  return fOK;
} /* end of function ITMAddToNopCnt */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     IdentsAreEqual
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       compares two identifiers
//------------------------------------------------------------------------------
// Parameters:
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE      identi are equal
//                    FALSE  identifiers re not equal
//------------------------------------------------------------------------------
// Function flow:     if tokenids are equal and only 1st part is relevant
//                     or
//                    if tokenids are equal and datastrings are equal
//                       return True ( tags are equal)
//                    else
//                       return FALSE
//------------------------------------------------------------------------------

static BOOL
IdentsAreEqual
(
   SHORT    sTokenID,
   SHORT    sRefTokenID,
   SHORT    sAddInfo,
   PSZ_W    pData,
   PSZ_W    pRefData,
   LONG     lRefLen
)
{
  BOOL      fEqual = FALSE;

  if ( (sTokenID == 0) && (sAddInfo == 0) )
  {
    fEqual = ItmCompChars(pData, pRefData, lRefLen);
  }
  else
  {
    /********************************************************************/
    /* compare with respect to the TAG_ITM_PART flag set in the sAddInfo*/
    /********************************************************************/

    if ( (sTokenID == sRefTokenID &&
            ((sAddInfo & TAG_ITM_PART) ||
             ItmCompChars(pData, pRefData, lRefLen) )  ))
    {
      fEqual = TRUE;
    } /* endif */
  } /* endif */
  return (fEqual);
} /* end of function IdentsAreEqual */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMCheckSlashTok
//------------------------------------------------------------------------------
// Function call:     ITMCheckSlashTok(pTok,pActTerm)
//------------------------------------------------------------------------------
// Description:       set fFound and Flags for special words such as
//                    AS/400, F1 - F22, A4, ( first = uppercase, followed by
//                    numbers)
//------------------------------------------------------------------------------
// Parameters:        PTOKENENTRY  pTok
//                    PFLAGOFFSLIST pActTerm
//------------------------------------------------------------------------------
// Returncode type:   -
//
//------------------------------------------------------------------------------
// Function flow:     compare whether tok is special word
//------------------------------------------------------------------------------
VOID
ITMCheckSlashTok
(
   PTOKENENTRY    pTok,
   PFLAGOFFSLIST  pActTerm
)
{
  PSZ_W      pData;
  USHORT   usLen = 0;
  PFLAGOFFSLIST pNextActTerm;

  pData = pTok->pDataStringW + (pActTerm->usOffs);
  usLen = pActTerm->usLen;
  if ((usLen == 1) && (*pData == DELSLASH))
  {
       pNextActTerm = pActTerm;
       pNextActTerm ++;                                 // reset to avoid re-usage!
       if ((pNextActTerm->lFlags & TF_NUMBER)
           && (pNextActTerm->usOffs == pActTerm->usOffs + usLen))
       {
          pNextActTerm->lFlags = 0L;
       } /* endif */
  } /* endif */

  return;
} /* end of function static ITMCheckSlashTok(PToKENENTRY, PFLAGOFFSLIST) */
#endif
