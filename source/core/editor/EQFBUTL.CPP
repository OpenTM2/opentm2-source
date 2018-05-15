/*! \file
	Description: Utility functions used in the Translation processor

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_FOLDER           // folder list and document list functions
#include <eqf.h>                  // General Translation Manager include file

#include "core\spell\SpellFactory.h"

#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFTAI.H"               // Text Analysis private defines

#include <eqfdoc00.h>             // for document handler defines
#include "SpecialChardlg.h"       // Add for R012027


static BOOL EQFBScanLF ( PTBDOCUMENT pDoc,               //pointer to doc instance
                         PUSHORT pusSegOffOld,           //old segment offset
                         PTBSEGMENT pSeg,
                         USHORT usPos);                  //starting segment offset
static BOOL EQFBInsertLF ( PTBDOCUMENT pDoc,             //pointer to doc instance
                           PTBSEGMENT pSeg,              //pointer to Segment
                           PUSHORT pusSegOffOld,         //old cursor segoffset
                           PULONG  pulPos,               //starting col pos
                           USHORT usSegOffset );         //starting seg offset

 static BOOL EQFBDBCSLineWrap ( PTBDOCUMENT pDoc,                                      //pointer to Doc instance
                                SHORT       sSegOffset,
                                LONG        lPos,
                                PSHORT       sSegOffOld );


BOOL  EQFBSegSoftLFInsert ( PTBDOCUMENT, PTBSEGMENT, USHORT );

/**********************************************************************/
/* delete character at previous data position                         */
/**********************************************************************/

  #define SEGSOFTDELETE(p)                    \
  {                                           \
     PSZ_W pSrc = p;                          \
     PSZ_W pTgt = p-1;                        \
     while (*pSrc)                            \
     {                                        \
       *pTgt ++ = *pSrc++;                    \
     }                                        \
     *pTgt = EOS;                             \
  }                                           \

static BOOL EQFBCheckAndAddSoftLF (PTBDOCUMENT, PTBSEGMENT, PUSHORT, PBOOL);


USHORT   EQFBMakeHLType(PTBSEGMENT pSeg, USHORT usTagCount);


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBUtlSpell
//------------------------------------------------------------------------------
// Function call:     EQFBUtlSpell( pstEQFGen )
//------------------------------------------------------------------------------
// Description:       get target language and check if it is installed
//------------------------------------------------------------------------------
// Parameters:        PSTEQFGEN  pstEQFGen  pointer to generic struct...
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     spellchecking could be activated
//                    FALSE    no spellchecking available
//------------------------------------------------------------------------------
// Function flow:     get target language
//                    extract the target language properties and try to find
//                    the language support file in the DPATH
//------------------------------------------------------------------------------

BOOL  EQFBUtlSpell
(
   PSTEQFGEN   pstEQFGen
)
{
   PDOCUMENT_IDA  pIda;                         // pointer to document struct.

   pIda    = (PDOCUMENT_IDA) pstEQFGen->pDoc;

   return ( EQFBCheckSpellLang( pIda->szDocTargetLang ) );
} /* end of function EQFBUtlSpell */


BOOL EQFBCheckSpellLang ( PSZ pszLang )        // check avail. of spell check
{
  SpellFactory* pSpellFactoryInstance = SpellFactory::getInstance();
	if (NULL == pSpellFactoryInstance)
	{
		return( FALSE );
  } 

  return ( pSpellFactoryInstance->isSupported( pszLang ) );
}

BOOL
EQFBCheckTags
(
   PTBDOCUMENT pDoc                                        // ptr to active document
)
{
  BOOL fEqual = TRUE;                  // success indicator
  PTBSEGMENT  pSrcSeg;                    // pointer to active segment
  PTBSEGMENT  pTgtSeg;                    // pointer to active segment
  USHORT      usRC = 0;
  PSTARTSTOP  pstSrcCurrent;
  PSTARTSTOP  pstTgtCurrent;
  BOOL        fFound = FALSE;
  USHORT      usTagCount = 0;
  USHORT      usColPos = 0;
  ULONG       ulLen = 0;
  ULONG       ulLenSrc = 0;


  pSrcSeg = EQFBGetSegW(pDoc->twin, pDoc->ulWorkSeg);
  pTgtSeg = EQFBGetSegW(pDoc, pDoc->ulWorkSeg);
  if (pDoc->hwndRichEdit )
  {
    if (pSrcSeg->pusBPET )
    {
      UtlAlloc((PVOID *)&(pSrcSeg->pusBPET), 0L, 0L, NOMSG );   // free start/stop
    } /* endif */
    if (pTgtSeg->pusBPET )
    {
      UtlAlloc((PVOID *)&(pTgtSeg->pusBPET), 0L, 0L, NOMSG );   // free start/stop
    } /* endif */
  } /* endif */
  if (pSrcSeg->pusBPET == NULL )
  {
    usRC = TACreateProtectTableW(pSrcSeg->pDataW,
                                pDoc->pDocTagTable,
                                usColPos,
                                (PTOKENENTRY) pDoc->pTokBuf,
                                TOK_BUFFER_SIZE,
                                (PSTARTSTOP * ) &(pSrcSeg->pusBPET),
                                pDoc->pfnUserExit,
                                pDoc->pfnUserExitW, pDoc->twin->ulOemCodePage);
  } /* endif */
  if (pTgtSeg->pusBPET == NULL )
  {
    usRC = TACreateProtectTableW(pTgtSeg->pDataW,
                                pDoc->pDocTagTable,
                                usColPos,
                                (PTOKENENTRY) pDoc->pTokBuf,
                                TOK_BUFFER_SIZE,
                                (PSTARTSTOP * ) &(pTgtSeg->pusBPET),
                                pDoc->pfnUserExit,
                                pDoc->pfnUserExitW, pDoc->ulOemCodePage);
  } /* endif */

  if (!usRC )
  {
    pstSrcCurrent = (PSTARTSTOP) pSrcSeg->pusBPET;
    while (pstSrcCurrent->usType != 0 )
    {
      if (pstSrcCurrent->usType == PROTECTED_CHAR )
      {
        pstTgtCurrent = (PSTARTSTOP) pTgtSeg->pusBPET;
        fFound = FALSE;
        while ( pstTgtCurrent->usType && !fFound)
        {
          if (pstTgtCurrent->usType == PROTECTED_CHAR )
          {
            ulLenSrc = pstSrcCurrent->usStop - pstSrcCurrent->usStart+1;
            ulLen = pstTgtCurrent->usStop - pstTgtCurrent->usStart+1;
            if (ulLenSrc > ulLen )
            {
              ulLen = ulLenSrc;
            } /* endif */
            if (UtlCompIgnWhiteSpaceW(pSrcSeg->pDataW + pstSrcCurrent->usStart,
                        pTgtSeg->pDataW + pstTgtCurrent->usStart,
                        ulLen ) == 0 )
            {
              /********************************************************/
              /* mark it as equal via type "UNPROTECTED_CHAR"         */
              /********************************************************/
              pstTgtCurrent->usType = UNPROTECTED_CHAR;
              fFound = TRUE;
            } /* endif */
          } /* endif */
          pstTgtCurrent++;
        } /* endwhile */
        if (!fFound )
        {
          fEqual = FALSE;         // src tag has no equvalent in tgt!
        } /* endif */
      } /* endif */
      pstSrcCurrent ++;
    } /* endwhile */
    /******************************************************************/
    /* check how many tags are left as unequal!!                      */
    /******************************************************************/
    pstTgtCurrent = (PSTARTSTOP) pTgtSeg->pusBPET;
    while ( pstTgtCurrent->usType )
    {
      if (pstTgtCurrent->usType == PROTECTED_CHAR  )
      {
        usTagCount ++;                // count how many tgt tags are left
        fEqual = FALSE;
      } /* endif */
      pstTgtCurrent ++;
    } /* endwhile */

    /********************************************************************/
    /* mark tag which is different                                      */
    /********************************************************************/
    if (!fEqual && usTagCount )
    {
      /********************************************************************/
      /* fill pusHLTable                                                  */
      /********************************************************************/
      usRC = EQFBMakeHLType( pTgtSeg, usTagCount);
    } /* endif */
    if (pTgtSeg->pusBPET )
    {
      UtlAlloc((PVOID *)&(pTgtSeg->pusBPET), 0L, 0L, NOMSG );   // free start/stop
    } /* endif */
  } /* endif */

  return( fEqual );
} /* end of function EQFBCheckTags */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBMakeHLType
//------------------------------------------------------------------------------
// Function call:     EQFBMakeHLType( PTBDOCUMENT , PTBSEGMENT, USHORT );
//------------------------------------------------------------------------------
// Description:       fill pusHLType with start-stop table for highlighting
//------------------------------------------------------------------------------
// Function flow:     count number of different tags
//                    allocate start-stop table for different tags
//                    fill start-stops for TAG_HIGHLIGHT parts
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    - pointer to document instance data
//                    PTBSEGMENT
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       usRC  - NO_ERROR if ok
//                    ERROR_NOT_ENOUGH_MEMORY from UtlAlloc
//------------------------------------------------------------------------------
USHORT
EQFBMakeHLType
(
      PTBSEGMENT  pSeg,
      USHORT      usTagCount
)
{
   USHORT      usRC = NO_ERROR;                         // success indicator
   PSTARTSTOP  pHLCurrent;
   PSTARTSTOP  pBPETCurrent;
   USHORT      usMSCount = 0;
   USHORT      usNewHLSize = 0;
   PUSHORT     pusNewHL;
   PSTARTSTOP  pHLOld = NULL;

   if (usTagCount )
   {
     usTagCount++;                     // count NULL element at end of start-stop table
     /*****************************************************************/
     /* count how many spellchecking highlights exist already         */
     /* old TAG_HIGHLIGHTS are deleted                                */
     /*****************************************************************/
     pHLOld = (PSTARTSTOP ) pSeg->pusHLType;
     if (pHLOld )
     {
       while (pHLOld->usType )
       {
         if (pHLOld->usType == MISSPELLED_HIGHLIGHT)
         {
           usMSCount++;
         } /* endif */
         pHLOld++;
       } /* endwhile */
     } /* endif */

     usNewHLSize = usTagCount + usMSCount;
     usNewHLSize +=2;

     if ( !UtlAlloc( (PVOID*) &(pusNewHL), 0L,
                    (LONG) max( (usNewHLSize * sizeof(STARTSTOP)), MIN_ALLOC),
                    NOMSG ))
     {
       usRC = ERROR_NOT_ENOUGH_MEMORY;
     } /* endif */

     if ( usRC == NO_ERROR )
     {
       pHLOld = (PSTARTSTOP ) pSeg->pusHLType;
       pBPETCurrent = (PSTARTSTOP ) pSeg->pusBPET;
       pHLCurrent = (PSTARTSTOP ) pusNewHL;
       if (!pHLOld )                  // no misspelled highlight!
       {
         while ( pBPETCurrent ->usType )
         {
           if (pBPETCurrent->usType == PROTECTED_CHAR )
           {
             pHLCurrent->usStart = pBPETCurrent->usStart;
             pHLCurrent->usStop  = pBPETCurrent->usStop;
             pHLCurrent->usType  = TAG_HIGHLIGHT;
             pHLCurrent++;
             usNewHLSize --;
           } /* endif */
           pBPETCurrent ++;
         } /* endwhile */
       }
       else
       {
         /***************************************************************/
         /* get first protected part and first misspelled part          */
         /***************************************************************/
         while (pBPETCurrent->usType  &&
                (pBPETCurrent->usType != PROTECTED_CHAR) )
         {
           pBPETCurrent ++;
         } /* endwhile */
         while (pHLOld->usType && (pHLOld->usType != MISSPELLED_HIGHLIGHT))
         {
           pHLOld ++;
         } /* endwhile */
         /***************************************************************/
         /* merge TAG_HIGHLIGHTS and MISSPELLED_HIGHLIGHTS into new     */
         /* HLType structure                                            */
         /***************************************************************/
         usNewHLSize --;
         while (usNewHLSize && ((pHLOld->usType) || (pBPETCurrent->usType)) )
         {
           if (((pHLOld->usType) && (pHLOld->usStart < pBPETCurrent->usStart) )
               || (pBPETCurrent->usType == 0) )
           {
             pHLCurrent->usStart = pHLOld->usStart;
             pHLCurrent->usStop  = pHLOld->usStop;
             pHLCurrent->usType  = MISSPELLED_HIGHLIGHT;
             pHLOld ++;
             while (pHLOld->usType &&
                      (pHLOld->usType != MISSPELLED_HIGHLIGHT))
             {
               pHLOld ++;                        // next misspelled part
             } /* endwhile */
           }
           else
           {
             pHLCurrent->usStart = pBPETCurrent->usStart;
             pHLCurrent->usStop  = pBPETCurrent->usStop;
             pHLCurrent->usType  = TAG_HIGHLIGHT;
             pBPETCurrent ++;
             while (pBPETCurrent->usType  &&
                    (pBPETCurrent->usType != PROTECTED_CHAR) )
             {
               pBPETCurrent ++;                  // next highlighted tag
             } /* endwhile */
           } /* endif */
           pHLCurrent ++;
           usNewHLSize --;
         } /* endwhile */
       } /* endif */
       /******************************************************************/
       /* terminate start/stop table                                     */
       /******************************************************************/
       pHLCurrent->usStart = 0;
       pHLCurrent->usStop  = 0;
       pHLCurrent->usType  = 0;
       pHLCurrent++;

       if (pSeg->pusHLType )
       {
         UtlAlloc((PVOID*) &(pSeg->pusHLType), 0L, 0L, NOMSG);
       } /* endif */
       pSeg->pusHLType =  pusNewHL;
     } /* endif */
   } /* endif */

   /********************************************************************/
   /* Return start/stop list to caller or cleanup                      */
   /********************************************************************/
   if ( usRC != NO_ERROR )
   {
     if ( pSeg->pusHLType )
     {
       pSeg->SegFlags.Spellchecked = FALSE;
       UtlAlloc( (PVOID *)&pSeg->pusHLType, 0L, 0L, NOMSG );
     } /* endif */
   } /* endif */

   return (usRC);
}  /* end of EQFBMakeHLType */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBDoTagCheck
//------------------------------------------------------------------------------
// Function call:     EQFBDoTagCheck( PTBDOCUMENT );
//------------------------------------------------------------------------------
// Description:       check if tags are equal or not
//
//------------------------------------------------------------------------------
// Function flow:     check if the tags are the same
//                    if tags are not the same
//                      display an error message and let user decide
//                    endif
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    - pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE  - proceed with save
//                    FALSE - user wants to work on this seg again
//------------------------------------------------------------------------------

BOOL EQFBDoTagCheck
(
   PTBDOCUMENT  pDoc                        // pointer to document ida
)
{
   BOOL        fOK = TRUE;                     // success indicator
   USHORT      usResult;                       // return value from UtlError
   PTBSEGMENT  pSeg;

   // check that tags are the same
   fOK = EQFBCheckTags( pDoc );


   if ( !fOK )
   {
     pDoc->Redraw |= REDRAW_ALL;                             // redraw all
//     UtlDispatch();

     if (pDoc->hwndRichEdit )
     {
       BOOL fWorkChng = pDoc->EQFBFlags.workchng;
       pSeg = EQFBGetSegW( pDoc, pDoc->ulWorkSeg );
       EQFBSetWorkSegRTF( pDoc, pDoc->ulWorkSeg, pSeg->pDataW );
       LockWindowUpdate( NULL );
       pDoc->EQFBFlags.workchng = (USHORT)fWorkChng;
     }
     else
     {
       EQFBRefreshScreen(pDoc);
     } /* endif */
     usResult = UtlError( TB_TAGCHANGED, MB_YESNO | MB_DEFBUTTON2,
                              0, NULL, EQF_QUERY);

     fOK = ( usResult == MBID_YES );
     if (usResult == MBID_YES )
     {
       pSeg = EQFBGetSegW( pDoc, pDoc->ulWorkSeg );

       if ( pSeg->pusHLType != NULL )
       {
         UtlAlloc( (PVOID *) &(pSeg->pusHLType), 0L, 0L, NOMSG) ;
         pSeg->SegFlags.Spellchecked = FALSE;
       } /* endif */
     } /* endif */
   } /* endif */

   return (fOK);
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBMisspelledHLType
//------------------------------------------------------------------------------
// Function call:     EQFBMisspelledHLType( PTBDOCUMENT , PTBSEGMENT, USHORT )
//------------------------------------------------------------------------------
// Description:       fill pusHLType with start-stop table for
//                    misspelled words
//------------------------------------------------------------------------------
// Function flow:     count number of misspelled words
//                    allocate start-stop table for different tags
//                    fill start-stops for MISSPELLED_HIGHLIGHT parts
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    - pointer to document instance data
//                    PTBSEGMENT
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       usRC  - NO_ERROR if ok
//                    ERROR_NOT_ENOUGH_MEMORY from UtlAlloc
//------------------------------------------------------------------------------

USHORT
EQFBMisspelledHLType
(
      PTBDOCUMENT pDoc,
      PTBSEGMENT  pSeg,
      PSZ_W       pMisTermList
)
{
   USHORT      usRC = NO_ERROR;                         // success indicator
   PSTARTSTOP  pHLCurrent;
   USHORT      usMSCount = 0;         // number of misspelled words
   SHORT       sCurOffset;
   SHORT       sWordLen;
   PSZ_W       pCurWord;
   SHORT       sSegOffset = 0;

   pCurWord = pMisTermList;

   while (*pCurWord && (usMSCount < MAX_SEGMENT_SIZE) )
   {
     usMSCount++;
     pCurWord += UTF16strlenCHAR(pCurWord) + 1;
   } /* endwhile */


   if (usMSCount )
   {
     usMSCount++;        // count NULL element at end of start-stop table
     if (pSeg && pSeg->pusHLType )
     {
       UtlAlloc((PVOID*) &(pSeg->pusHLType), 0L, 0L, NOMSG);
     } /* endif */

     if ( !UtlAlloc( (PVOID*) &(pSeg->pusHLType), 0L,
                    (LONG) max( (usMSCount * sizeof(STARTSTOP)), MIN_ALLOC),
                    NOMSG ))
     {
       usRC = ERROR_NOT_ENOUGH_MEMORY;
     } /* endif */

     if ( usRC == NO_ERROR )
     {
       pHLCurrent = (PSTARTSTOP ) pSeg->pusHLType;

       pCurWord = pMisTermList;
       sCurOffset = 0;
       while ( usMSCount && *pCurWord )
       {
         // determine position of misspelled word in segment
         sSegOffset = EQFBPosGotoSeg(pDoc,
                                     pSeg->ulSegNum,
                                     pCurWord, sCurOffset);
         sWordLen = (SHORT)UTF16strlenCHAR(pCurWord);

         if (sWordLen )
         {
           pHLCurrent->usStart = sSegOffset;
           pHLCurrent->usStop  = sSegOffset + sWordLen -1;
           pHLCurrent->usType  = MISSPELLED_HIGHLIGHT;
           pHLCurrent++;
         } /* endif */
         usMSCount--;
         pCurWord += sWordLen + 1;
         sCurOffset = sSegOffset + sWordLen;
       } /* endwhile */

       /******************************************************************/
       /* terminate start/stop table                                     */
       /******************************************************************/
       pHLCurrent->usStart = 0;
       pHLCurrent->usStop  = 0;
       pHLCurrent->usType  = 0;
       pHLCurrent++;
       pSeg->SegFlags.Spellchecked = TRUE;
     } /* endif */
   }
   else
   {
     if (!pSeg->pusHLType )
     {
       UtlAlloc((PVOID*) &(pSeg->pusHLType), 0L,
                (LONG) max( sizeof(STARTSTOP), MIN_ALLOC),
                NOMSG);
     } /* endif */
     if ( pSeg->pusHLType )
     {
       ((PSTARTSTOP)pSeg->pusHLType)->usStart = 0;
       ((PSTARTSTOP)pSeg->pusHLType)->usStop  = 0;
       ((PSTARTSTOP)pSeg->pusHLType)->usType  = 0;
       pSeg->SegFlags.Spellchecked = TRUE;
     } /* endif */
   } /* endif */

   /********************************************************************/
   /* Return start/stop list to caller or cleanup                      */
   /********************************************************************/
   if ( usRC != NO_ERROR )
   {
     if ( pSeg->pusHLType )
     {
       pSeg->SegFlags.Spellchecked = FALSE;
       UtlAlloc( (PVOID *)&pSeg->pusHLType, 0L, 0L, NOMSG );
     } /* endif */
   } /* endif */

   return (usRC);
}  /* end of EQFBMisspelledHLType */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFindLine
//------------------------------------------------------------------------------
// Function call:     EQFBFindLine( pDoc, ulLine );
//------------------------------------------------------------------------------
// Description:       Position at the specified line
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc   pointer to document
//                    ULONG        ulLine line number to position at
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    positionned at the specified line
//                    FALSE   specified line could not be reached
//------------------------------------------------------------------------------
// Function flow:     scan through the segments until specified line is reached
//                    if found
//                      check if not at end of a segment, if so pos at next one
//                      goto found segment and position at start of this line
//                       (EQFBGotoSeg will skip protected characters)
//                    endif
//                    return found
//------------------------------------------------------------------------------
BOOL
EQFBFindLine
(
  PTBDOCUMENT  pDoc,                   // pointer to document ida
  ULONG        ulLine                  // line number to search for
)
{
  BOOL  fFound = FALSE;                // positioned at line number
  ULONG    ulSeg = 1;                  // start with first segment
  ULONG    ulActLine = 1;              // active line
  PTBSEGMENT  pSeg;                    // pointer to segment data
  PSZ_W    pData;                      // pointer to data
  DISPSTYLE OldDispStyle;              // store original display style

  OldDispStyle = pDoc->DispStyle;
  pDoc->DispStyle = DISP_PROTECTED;
  /********************************************************************/
  /* find the requested line and segment ..                           */
  /* do not use EQFBGetVisSeg because in Shrink/Compact style         */
  /* the original segments are needed to count the lines,             */
  /* not the shrinked block of NOPs                                   */
  /********************************************************************/
  pSeg = EQFBGetVisSeg( pDoc, &ulSeg );          // get segment
  pData = pSeg->pDataW;
  fFound = (ulActLine >= ulLine);

  while ( ! fFound )
  {
    switch ( *pData )
    {
      case  SOFTLF_CHAR:
        if ( pDoc->fAutoLineWrap )
        {
          ulActLine ++;
          pData++;
          fFound = (ulActLine >= ulLine);
        }
        else
        {
          pData++;
        } /* endif */
        break;
      case  LF:
        ulActLine ++;
        pData++;
        fFound = (ulActLine >= ulLine);
        break;
      case  EOS:
        ulSeg++;
        pSeg = EQFBGetVisSeg( pDoc, &ulSeg );    // get next segment
        if ( pSeg )
        {
          pData = pSeg->pDataW;
        }
        else
        {
          fFound = TRUE;
        } /* endif */
        break;
      default :
        pData++;
        break;
    } /* endswitch */
  } /* endwhile */

  pDoc->DispStyle = OldDispStyle;
  /********************************************************************/
  /* if we found the line, we will position at the requested segm..   */
  /********************************************************************/
  if ( ulActLine >= ulLine )
  {
    /******************************************************************/
    /* check that we are not at the end of a segment ..               */
    /******************************************************************/
    if ( !*pData )
    {
      ULONG       ulSegTemp = ulSeg;
      ulSeg++;
      pSeg = EQFBGetVisSeg( pDoc, &ulSeg );    // get next segment
      if ( pSeg )
      {
        EQFBGotoSeg( pDoc, ulSeg, 0 );
        EQFBFuncStartLine( pDoc );
      }
      else
      {
        ulSeg = ulSegTemp;
        pSeg = EQFBGetVisSeg( pDoc, &ulSeg );          // get next segment
        if ( !pSeg )
        {
          ulSeg = ulSegTemp;
          pSeg = EQFBGetPrevVisSeg( pDoc, &ulSeg );    // get prev segment
        } /* endif */
        if ( pSeg )
        {
          EQFBGotoSeg( pDoc, ulSeg, 0 );
          EQFBFuncStartLine( pDoc );
        }
        else
        {
          fFound = FALSE;
        } /* endif */
      } /* endif */
    }
    else
    {
      EQFBGotoSeg( pDoc, ulSeg, (USHORT)((pData - pSeg->pDataW)));
      EQFBFuncStartLine( pDoc );
    } /* endif */
  }
  else
  {
    fFound = FALSE;
  } /* endif */


  return ( fFound );
} /* end of function EQFBFindLine */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBQueryLine
//------------------------------------------------------------------------------
// Function call:     EQFBQueryLine( pDoc )
//------------------------------------------------------------------------------
// Description:       find the current cursor line and the position in the
//                    file
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc   pointer to document instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     find the lines up to the cursor segment
//                    increment this number with the LF within cursor segment
//                    loop through the rest to find total  number
//                    display message with current position and total number
//
//------------------------------------------------------------------------------

VOID
EQFBQueryLine
(
  PTBDOCUMENT   pDoc                   // pointer to document ida
)
{
  ULONG    ulSeg = 1;                  // start with first segment
  ULONG    ulActLine = 1;              // active line
  USHORT   usOffset = 0;
  ULONG    ulCrsSeg;                   // cursor segment
  USHORT   usCrsOffset;                // cursor offset
  ULONG    ulTotalLines;               // number of total lines2
  PTBSEGMENT  pSeg;                    // pointer to segment data
  PSZ_W    pData;                      // pointer to data
  PSZ      pResult[3];                 // result buffer
  CHAR     chLines[20];                // array for conversions...
  DISPSTYLE OldDispStyle;              // store original display style

  OldDispStyle = pDoc->DispStyle;
  pDoc->DispStyle = DISP_PROTECTED;
  /********************************************************************/
  /* find cursor segment                                              */
  /********************************************************************/
  ulCrsSeg = pDoc->TBCursor.ulSegNum;
  usCrsOffset = pDoc->TBCursor.usSegOffset;

  /********************************************************************/
  /* find lines up to cursor segment                                  */
  /********************************************************************/
  pSeg = EQFBGetVisSeg( pDoc, &ulSeg );          // get segment
  pData = pSeg->pDataW;

  while ( ulCrsSeg > ulSeg)
  {
    switch ( *pData )
    {
      case  LF:
        ulActLine ++;
        pData++;
        break;
      case  EOS:
        ulSeg++;
        pSeg = EQFBGetVisSeg( pDoc, &ulSeg );    // get next segment
        if ( pSeg )
        {
          pData = pSeg->pDataW;
        }
        else
        {
          pData = NULL;              // should never happen,but one never know
          ulSeg = ulCrsSeg;
        } /* endif */
        break;
      default :
        pData++;
        break;
    } /* endswitch */
  } /* endwhile */

  /********************************************************************/
  /* scan cursor segment up to the specified position                 */
  /********************************************************************/
  if ( pData )
  {
    while ( *pData && (usOffset < usCrsOffset) )
    {
      if ( *pData == LF )
      {
        ulActLine++;
      } /* endif */
      usOffset++;
      pData++;
    } /* endwhile */
  } /* endif */

/**********************************************************************/
/* now scan through to find total lines                               */
/**********************************************************************/
  ulTotalLines = ulActLine;
  while ( pSeg )
  {
    switch ( *pData )
    {
      case  LF:
        ulTotalLines ++;
        pData++;
        break;
      case  EOS:
        ulSeg++;
        pSeg = EQFBGetVisSeg( pDoc, &ulSeg );    // get next segment
        if ( pSeg )
        {
          pData = pSeg->pDataW;
        }
        else
        {
          pData = NULL;              // we have find the end
        } /* endif */
        break;
      default :
        pData++;
        break;
    } /* endswitch */

  } /* endwhile */
  pDoc->DispStyle = OldDispStyle;
  ulTotalLines--;                      // we've gone one too far ..

  /********************************************************************/
  /* display the information message                                  */
  /********************************************************************/
  pResult[0] = chLines;
  pResult[1] = &chLines[10];
  ulActLine = min( ulActLine, ulTotalLines );
  ltoa( ulActLine, chLines, 10 );
  ltoa( ulTotalLines, chLines+10, 10 );
  UtlError( TB_LINEINFILE, MB_CANCEL, 2 , &pResult[0], EQF_INFO );
} /* end of function EQFBQueryLine */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBQueryActLine
//------------------------------------------------------------------------------
// Function call:     EQFBQueryActLine(pDoc)
//------------------------------------------------------------------------------
// Description:       find the current cursor line and the position in the
//                    file
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc   pointer to document instance
//------------------------------------------------------------------------------
// Returncode type:   ULONG
//------------------------------------------------------------------------------
// Function flow:     find the lines up to the cursor segment
//                    increment this number with the LF within cursor segment
//------------------------------------------------------------------------------

ULONG
EQFBQueryActLine
(
  PTBDOCUMENT   pDoc,                   // pointer to document ida
  ULONG         ulCrsSeg,               // cursor segment
  USHORT        usCrsOffset             // offset in segment
)
{
  ULONG    ulSeg = 1;                  // start with first segment
  ULONG    ulActLine = 1;              // active line
  USHORT   usOffset = 0;
  PTBSEGMENT  pSeg;                    // pointer to segment data
  PSZ_W    pData;                      // pointer to data

  /********************************************************************/
  /* find lines up to cursor segment                                  */
  /********************************************************************/
  pSeg = EQFBGetVisSeg( pDoc, &ulSeg );                    // get segment
  if (pSeg )
  {
    pData = pSeg->pDataW;

    while ( ulCrsSeg > ulSeg)
    {
      switch ( *pData )
      {
        case SOFTLF_CHAR:
          if (*(pData+1) != SOFTLF_CHAR )
          {
            ulActLine++;
            pData++;
          }
          else
          {
            /************************************************************/
            /* skip duplicated softlf which is not linefeed             */
            /************************************************************/
            pData += 2;
          } /* endif */
          break;
        case  LF:
          ulActLine ++;
          pData++;
          break;
        case  EOS:
          ulSeg++;
          pSeg = EQFBGetVisSeg( pDoc, &ulSeg );    // get next segment
          if ( pSeg )
          {
            pData = pSeg->pDataW;
          }
          else
          {
            pData = NULL;              // should never happen,but one never know
            ulSeg = ulCrsSeg;
          } /* endif */
          break;
        default :
          pData++;
          break;
      } /* endswitch */
    } /* endwhile */

    /********************************************************************/
    /* scan cursor segment up to the specified position                 */
    /********************************************************************/
    if ( pData )
    {
      while ( *pData && (usOffset < usCrsOffset) )
      {
        if ( *pData == LF )
        {
          ulActLine++;
        } /* endif */
        if ((*pData == SOFTLF_CHAR ) )
        {
          if (*(pData +1) == SOFTLF_CHAR )
          {
            usOffset += 2;        //duplicated softlf
            pData += 2;
          }
          else
          {
            ulActLine++;          // softlf is linefeed
            usOffset++;
            pData++;
          } /* endif */
        }
        else
        {
          usOffset++;
          pData++;
        } /* endif */
      } /* endwhile */
    } /* endif */
  } /* endif */
  return( ulActLine);

} /* end of function EQFBQueryActLine */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBGetUserSettings
//------------------------------------------------------------------------------
// Function call:     EQFBGetUserSettings( pDoc )
//------------------------------------------------------------------------------
// Description:       get a pointer to the user settings
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc  pointer to document structure
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     set the pointer to the user options ....
//------------------------------------------------------------------------------

VOID EQFBGetUserSettings
(
  PTBDOCUMENT pDoc
)
{
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
  pDoc->pUserSettings = pEQFBUserOpt;
} /* end of function EQFBGetUserSettings */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFileInsertSOSI
//------------------------------------------------------------------------------
// Function call:     EQFBFileInsertSOSI(pDoc)
//------------------------------------------------------------------------------
// Description:       insert SOSI in all xlated segments
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if dbcs codepage
//                      get ptr to 1st segment
//                      while not end of segments
//                         switch (status of segment)
//                           case QF_NOP,QF_ATTR,QF_TOBE:
//                            break
//                           case QF_CURRENT:
//                           case QF_XLATED;
//                            insert SOSI
//                            break
//                          endswitch
//                          get next segment
//                      endwhile
//                    else
//                      beep
//                    endif
//------------------------------------------------------------------------------

VOID EQFBFileInsertSOSI
(
  PTBDOCUMENT pDoc
)
{
  ULONG  ulSegNum;                         //segment number
  PTBSEGMENT pSeg;                               // ptr to segment

  if (IsDBCS_CP(pDoc->ulOemCodePage))
  {
    ulSegNum = 1;
    pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
    while ( pSeg )
    {
      switch ( pSeg->qStatus )
      {
//      case  QF_NOP:                             // segment not counted
//      case  QF_TOBE:
//      case  QF_ATTR:
//        break;
        case  QF_CURRENT:
        case  QF_XLATED:
          EQFBConvertSOSI(pDoc,pSeg, INSERT_SOSI);
          break;
        default :
          break;
      } /* endswitch */
      ulSegNum ++;
      pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
    } /* endwhile */

    if (pDoc->fLineWrap && pDoc->fAutoLineWrap )
    {
      if (pDoc->lScrnCols )
      {
        EQFBFuncMarginAct(pDoc);
        EQFBFuncMarginAct(pDoc);
      } /* endif */
      if (pDoc->fAutoSpellCheck && pDoc->pvSpellData )
	  { // force that thread recalcs pusHLType of screen
	     PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
	     pSpellData->TBFirstLine.ulSegNum = 0;
	     pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
	  }
    } /* endif */

    EQFBCurSegFromCursor(pDoc);
    EQFBScrnLinesFromSeg ( pDoc,                  // pointer to doc ida
                           0,                     // starting row
                           pDoc->lScrnRows,      // number of rows
                                                  // starting segment
                         (pDoc->TBRowOffset+1));
//  EQFBScrnLinesFromSeg ( pDoc,                  // pointer to doc ida
//                        pDoc->sCursorRow,       // starting row
//                        pDoc->usScrnRows -
//                            pDoc->sCursorRow,   // number of rows
//                        &(pDoc->TBCursor) );    // seg num start row
  }
  else
  {
    WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if not dbcs
  } /* endif */
} /* end of function EQFBFileInsertSOSI */
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFileDeleteSOSI
//------------------------------------------------------------------------------
// Function call:     EQFBFileDeleteSOSI(pDoc)
//------------------------------------------------------------------------------
// Description:       Delete SOSI from within the file
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if dbcs codepage
//                      get ptr to 1st segment
//                      while not end of segments
//                         switch (status of segment)
//                           case QF_NOP,QF_ATTR,QF_TOBE:
//                            break
//                           case QF_CURRENT:
//                           case QF_XLATED;
//                            delete SOSI
//                            break
//                          endswitch
//                          get next segment
//                      endwhile
//                    else
//                      beep
//                    endif
//------------------------------------------------------------------------------

VOID EQFBFileDeleteSOSI
(
  PTBDOCUMENT pDoc
)
{
  ULONG  ulSegNum;                         //segment number
  PTBSEGMENT pSeg;                         // ptr to segment

  if (IsDBCS_CP(pDoc->ulOemCodePage))
  {
    ulSegNum = 1;
    pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
    while ( pSeg )
    {
      switch ( pSeg->qStatus )
      {
//      case  QF_NOP:                             // segment not counted
//      case  QF_TOBE:
//      case  QF_ATTR:
//        break;
        case  QF_CURRENT:
        case  QF_XLATED:
          EQFBConvertSOSI(pDoc,pSeg, DELETE_SOSI);
          break;
        default :
          break;
      } /* endswitch */
      ulSegNum ++;
      pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
    } /* endwhile */

    if (pDoc->fLineWrap && pDoc->fAutoLineWrap )
    {
      if (pDoc->lScrnCols )
      {
        EQFBFuncMarginAct(pDoc);
        EQFBFuncMarginAct(pDoc);
      } /* endif */
      if (pDoc->fAutoSpellCheck && pDoc->pvSpellData )
	  { // force that thread recalcs pusHLType of screen
	    PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
	    pSpellData->TBFirstLine.ulSegNum = 0;
	    pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
	  }
    } /* endif */

    EQFBCurSegFromCursor(pDoc);
    EQFBScrnLinesFromSeg ( pDoc,                  // pointer to doc ida
                           0,                     // starting row
                           pDoc->lScrnRows,      // number of rows
                                                  // starting segment
                         (pDoc->TBRowOffset+1));
  }
  else
  {
    WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if not dbcs
  } /* endif */
} /* end of function EQFBFileDeleteSOSI */
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBSegDeleteSOSI
//------------------------------------------------------------------------------
// Function call:     EQFBSegDeleteSOSI(pDoc)
//------------------------------------------------------------------------------
// Description:       delete SOSI in current segment
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if dbcs codepage
//                      delete all SOSI
//                    else
//                      beep
//                    endif
//------------------------------------------------------------------------------

VOID EQFBSegDeleteSOSI
(
  PTBDOCUMENT pDoc
)
{
  if (IsDBCS_CP(pDoc->ulOemCodePage))
  {
    EQFBConvertSOSI(pDoc,pDoc->pTBSeg, DELETE_SOSI);
    EQFBCurSegFromCursor(pDoc);
    EQFBScrnLinesFromSeg ( pDoc,                  // pointer to doc ida
                           0,                     // starting row
                           pDoc->lScrnRows,      // number of rows
                                                  // starting segment
                         (pDoc->TBRowOffset+1));
  }
  else
  {
    WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if not dbcs
  } /* endif */
} /* end of function EQFBSegDeleteSOSI */
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBSegInsertSOSI
//------------------------------------------------------------------------------
// Function call:     EQFBSegInsertSOSI(pDoc)
//------------------------------------------------------------------------------
// Description:       insert SOSI in current segment
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if dbcs codepage
//                      insert all SOSI
//                    else
//                      beep
//                    endif
//------------------------------------------------------------------------------

VOID EQFBSegInsertSOSI
(
  PTBDOCUMENT pDoc
)
{

  if (IsDBCS_CP(pDoc->ulOemCodePage))
  {
    EQFBConvertSOSI(pDoc,pDoc->pTBSeg, INSERT_SOSI);
    EQFBCurSegFromCursor(pDoc);
    EQFBScrnLinesFromSeg ( pDoc,                  // pointer to doc ida
                           0,                     // starting row
                           pDoc->lScrnRows,      // number of rows
                                                  // starting segment
                         (pDoc->TBRowOffset+1));
  }
  else
  {
    WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if not dbcs
  } /* endif */
} /* end of function EQFBSegInsertSOSI */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBConvertSOSI
//------------------------------------------------------------------------------
// Function call:     EQFBConvertSOSI(PTBDOCUMENT,PTBSEGMENT,USHORT)
//------------------------------------------------------------------------------
// Description:       insert/delete(acc. to usConvert) SOSI in given segment
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//                    PTBSEGMENT pSeg
//                    USHORT usConvert
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   conversion is ok
//                    FALSE  something went wrong
//------------------------------------------------------------------------------
// Function flow:     if Buffer in pDoc is not yet allocated
//                      allocate buffer in MAX_SEGMENT_SIZE +1
//                    call utility to convert data of given segment
//                      and return result in buffer
//                    if ok so far
//                      allocate new space for pData of given segment
//                      copy data from buffer into pSeg->pData
//                      adjust length of segment
//                      free start/stop table of segment
//                    endif
//                    return success indicator
//------------------------------------------------------------------------------

BOOL EQFBConvertSOSI
(
 PTBDOCUMENT pDoc,
 PTBSEGMENT pSeg,
 USHORT usConvert
)
{
  ULONG  ulConvDataLen;                //length of converted data string
  ULONG  ulLen;                        //length to alloc for new pSeg-pData
  BOOL   fResult = TRUE;               //success indicator
  PSZ_W  pTmpBuf = NULL;               // pointer to temp buffer

  if (!pDoc->pSegmentBufferW )
  {
    fResult = UtlAlloc((PVOID *)&(pDoc->pSegmentBufferW), 0L,
                       (LONG)(MAX_SEGMENT_SIZE+1)*sizeof(CHAR_W),
                       ERROR_STORAGE);
  } /* endif */

  if ( fResult )
  {
    ulConvDataLen = MAX_SEGMENT_SIZE;
    fResult = EQFBUtlConvertSOSI(pDoc->pSegmentBufferW,
                                 &ulConvDataLen, pSeg->pDataW,
                                 usConvert, pDoc->ulOemCodePage );


    if ( fResult )
    {
      if ( (pSeg->ulSegNum != pDoc->ulWorkSeg) )
      {
        ulLen = max( ulConvDataLen, MIN_ALLOC ); // get rid of warning
                                                 // in UtlAlloc
        pTmpBuf = pSeg->pDataW;                   // get pointer to data
        fResult = UtlAlloc((PVOID *) &(pSeg->pDataW),
                 0L, (LONG) (ulLen*sizeof(CHAR_W)), ERROR_STORAGE );
      }
      else
      {
        pDoc->EQFBFlags.workchng = TRUE;
      } /* endif */
      if ( fResult )
      {
        UTF16strcpy( pSeg->pDataW, pDoc->pSegmentBufferW );
        pSeg->usLength = (USHORT)(ulConvDataLen - 1);
        UtlAlloc((PVOID *)&(pSeg->pusBPET), 0L, 0L, NOMSG );          // free start/stop
        if ( pTmpBuf )
        {
          UtlAlloc((PVOID *)&pTmpBuf, 0L, 0L, NOMSG );                // data space
        } /* endif */

        if (pSeg->pusHLType )
        {
          pSeg->SegFlags.Spellchecked = FALSE;
          UtlAlloc((PVOID *)&(pSeg->pusHLType), 0L, 0L, NOMSG);
        } /* endif */
        pDoc->Redraw |= REDRAW_ALL;                          // redraw all
      }
      else
      {
        fResult = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */
  return (fResult);
} /* end of function EQFBConvertSOSI */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBAddFuzzyMatch
//------------------------------------------------------------------------------
// Function call:     EQFBAddFuzzyMatch(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       display fuzzy match in TM window
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     call EQFTRANSSEG with EQFF_EXACTANDFUZZY mode
//------------------------------------------------------------------------------

VOID EQFBAddFuzzyMatch
(
 PTBDOCUMENT pDoc
)
{
   SHORT   sMatchFound;                          // number of matches
   USHORT  usRc;
   PSZ     pData;                                // pointer to error text
   PSZ_W   pszContext;

  /********************************************************************/
  /* call to EQFBTRansSeg                                             */
  /********************************************************************/
   pszContext = EQFBGetContext( pDoc, pDoc->pTBSeg, pDoc->pTBSeg->ulSegNum );
   usRc = EQFTRANSSEG3W(pDoc->pTBSeg->pDataW,     // pointer to seg data
                        pszContext, pDoc->pTBSeg->pvMetadata,
                        pDoc->pTBSeg->ulSegNum,  // segment number
                        TRUE,                    // foreground
                        EQFF_EXACTANDFUZZY,      // no automatic mode
                        &sMatchFound);

  if (usRc != EQFRC_OK )           // && usRc != EQFRC_SEG_NOT_FOUND)
  {
     pData = EQFERRINS();              // get error message
     UtlDispatch();
     UtlError( EQFERRID(), MB_CANCEL, 1, &pData, EQF_ERROR );
  } /* endif */
} /* end of function EQFBAddFuzzyMatch */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncDispAllExactProps - display all exact proposals
//------------------------------------------------------------------------------
// Function call:     EQFBFuncDispAllExactProps( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:        This function will display all exact proposals
//
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:       primitive function but necessary to allow calling
//                     logic within translation editor
//
//------------------------------------------------------------------------------
// Function flow:      if not in postedit mode
//                        Call services (EQFTRANSSEG with EQFF_ALLEXACTONES)
//                     else
//                        beep
//                     endif
//
//------------------------------------------------------------------------------
VOID  EQFBFuncDispAllExactProps( PTBDOCUMENT pDoc )
{
   if ( !pDoc->EQFBFlags.PostEdit )
   {
     SHORT   sMatchFound;                          // number of matches
     USHORT  usRc;
     PSZ     pData;                                // pointer to error text
     PSZ_W   pszContext;
     /********************************************************************/
     /* call to EQFBTRansSeg                                             */
     /********************************************************************/
     pszContext = EQFBGetContext( pDoc, pDoc->pTBSeg, pDoc->pTBSeg->ulSegNum );

     usRc = EQFTRANSSEG3W(pDoc->pTBSeg->pDataW,     // pointer to seg data
                          pszContext, pDoc->pTBSeg->pvMetadata,
                          pDoc->pTBSeg->ulSegNum,  // segment number
                          TRUE,                    // foreground
                          EQFF_ALLEXACTONES | EQFF_MOREPROPINDIC ,      // no automatic mode
                          &sMatchFound);

     if (usRc != EQFRC_OK)
     {
       pData = EQFERRINS();              // get error message
       UtlDispatch();
       UtlError( EQFERRID(), MB_CANCEL, 1, &pData, EQF_ERROR );
     } /* endif */
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if no transl.mem.wnd
   } /* endif */

}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBCheckEndOfSeg
//------------------------------------------------------------------------------
// Function call:     EQFBCheckEndOfSeg(pDoc,pSeg,usSegOffset)
//------------------------------------------------------------------------------
// Description:       Check the end-of segment flag
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//                    PTBSEGMENT  pSeg            ptr to segment
//                    USHORT      usSegOffset     offset in segment
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------
VOID
EQFBCheckEndOfSeg
(
   PTBDOCUMENT  pDoc,
   PTBSEGMENT   pSeg,
   USHORT       usSegOffset
)
{
  PSZ_W     pData;
  USHORT    usType;
  DISPSTYLE DispStyle;

  pDoc->EQFBFlags.EndOfSeg = FALSE;

  DispStyle = (pSeg->SegFlags.Expanded) ?
                 DISP_PROTECTED : (pDoc->DispStyle);

  if ( (DispStyle == DISP_COMPACT) &&
       ((usType= EQFBCharType(pDoc,pSeg,usSegOffset)) == COMPACT_CHAR) )
  {
     usType = HIDDEN_CHAR;
     while (usType == HIDDEN_CHAR  )
     {
       usSegOffset++;
       usType = EQFBCharType(pDoc,pSeg, usSegOffset);
     } /* endwhile */
     if ( usType == ENDOFSEG_CHAR )
     {
        pDoc->EQFBFlags.EndOfSeg = TRUE;
     } /* endif */
     EQFBScreenCursorType( pDoc );

  }
  else
  {
     pData = pSeg->pDataW + usSegOffset;
     if (*pData != '\n')                //set EndOfSeg flag
     {
//        if ( _dbcs_cp == DBCS_CP && isdbcs1(*pData) == DBCS_1ST )
//        {
//           pDoc->EQFBFlags.EndOfSeg = (*(pData+2) == EOS);
//        }
//        else
//        {
           pDoc->EQFBFlags.EndOfSeg = (*(pData+1) == EOS);
//        } /* endif */
        EQFBScreenCursorType( pDoc );
     } /* endif */
  } /* endif */
  return;
} /* end of function EQFBCheckEndOfSeg */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFindWord
//------------------------------------------------------------------------------
// Function call:     EQFBFindWOrd(PSZ,SHORT,USHORT,PUSHORT,PUSHORT)
//------------------------------------------------------------------------------
// Description:       find the current word
//------------------------------------------------------------------------------
// Parameters:        PSZ       pData,                   //ptr to data string
//                    SHORT     sLanguageId,             //language ID
//                    USHORT    usCursorOffset,          //cursor offset
//                    PUSHORT   pusStart,                //start of word
//                    PUSHORT   pusEnd                   //end of word
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       fFOund        True if found, FALSE if not on a valid term
//------------------------------------------------------------------------------
// Function flow:     morph.tokenize data string
//                    if tokenlist exists
//                      while not at end of list or found
//                        goto next token
//                      endwhile
//                      if cursor between 2 tokens
//                         no valid current word found
//                      if current token is punctuation or number
//                         no valid current word found
//                      if word found
//                         set start and end of word
//                    endif
//                    return found indicator
//------------------------------------------------------------------------------

BOOL
EQFBFindWord
(
  PSZ_W     pData,                             //ptr to data string
  SHORT     sLanguageId,                       //language ID
  USHORT    usCursorOffset,                    //cursor offset
  PUSHORT   pusStart,                          //start of word
  PUSHORT   pusEnd,                             //end of word
  ULONG     ulOemCP,
  BOOL      fFindAllWords                       // TRUE if called from FuncWordDel
)
{
  SHORT          sRc;
  USHORT         usListSize = 0;
  PFLAGOFFSLIST  pTermList = NULL;
  PFLAGOFFSLIST  pActTerm;
  BOOL           fFound = FALSE;
  BOOL           fCursorOnDelimiter= FALSE;
  PSZ_W          pNormData;            // pointer to normalized string
  PSZ_W          pTemp;                // temp pointer...

  if ( UtlAlloc( (PVOID *)&pNormData, 0L, (LONG) MAX_SEGMENT_SIZE*sizeof(CHAR_W), ERROR_STORAGE) )
  {
     UTF16strcpy( pNormData, pData );
     pTemp = pNormData;
     while ( *pTemp )
     {
        if ( *pTemp == LF )
        {
           *pTemp = BLANK;
        } /* endif */
        pTemp++;
     } /* endwhile */

     sRc = MorphTokenizeW(sLanguageId, pNormData,
                         &usListSize, (PVOID *) &pTermList,
                         MORPH_FLAG_OFFSLIST, ulOemCP);
     if ( pTermList )
     {
       pActTerm = pTermList;
       while ( pActTerm->usLen && !fFound )
       {
         if ( !(pActTerm->lFlags & TF_NEWSENTENCE ))
         {
           if ( (pActTerm->usOffs <= usCursorOffset)
              && ( (pActTerm->usOffs + pActTerm->usLen) > usCursorOffset) )
           {
             fFound = TRUE;
           }
           else if (pActTerm->usOffs > usCursorOffset )
           {
             fFound = TRUE;
             fCursorOnDelimiter = TRUE;
           } /* endif */
         } /* endif */
         pActTerm++;
       } /* endwhile */
       /******************************************************************/
       /* make correction due to end of while loop                       */
       /******************************************************************/
       if ( fCursorOnDelimiter )
       {
         fFound = FALSE;
       } /* endif */

       pActTerm--;

       if ( fFound )
       {
		 if (!fFindAllWords )
		 {
			 /**************************************************************/
			 /* noise (numbers,.;:?! crlf ...) are no valid words          */
			 /* for usage in dictionary lookup                             */
			 /**************************************************************/
			 if ( !(pActTerm->lFlags & TF_NEWSENTENCE ))
			 {
			   if ( pActTerm->lFlags & TF_NOLOOKUP )
			   {
				 fFound = FALSE;               // it is a noise
			   } /* endif */
			 }
			 else
			 {
			   fFound = FALSE;               // dummy token are not valid
			 } /* endif */
	     }
         if ( pActTerm->usLen )
         {
           *pusStart = pActTerm->usOffs;
           *pusEnd = (pActTerm->usOffs) + (pActTerm->usLen);
         } /* endif */
       } /* endif */
       UtlAlloc( (PVOID *)&pTermList, 0L, 0L, NOMSG );
     } /* endif */

    /******************************************************************/
    /* free unused resource...                                        */
    /******************************************************************/
    UtlAlloc( (PVOID *)&pNormData, 0L, 0L, NOMSG );
  } /* endif */
  return (fFound);
} /* end of function EQFBFindWord */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncChangeFocusLine
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
EQFBFuncChangeFocusLine
(
  SHORT  sNewFocus
)
{
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
  pEQFBUserOpt->sFocusLine = sNewFocus;
} /* end of function EQFBFuncChangeFocusLine */

SHORT EQFBFuncGetFocusLine()
{
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
  return( pEQFBUserOpt->sFocusLine );
} /* end of function EQFBFuncChangeFocusLine */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBGetColors
//------------------------------------------------------------------------------
// Function call:     EQFBGetColors(pTextTypeTable)
//------------------------------------------------------------------------------
// Description:       get a pointer to the colors table
//------------------------------------------------------------------------------
// Parameters:        PEXTTYPETABLE pTextTypeTable
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     set the pointer to the colors table ....
//------------------------------------------------------------------------------

VOID EQFBGetColors
(
  PTEXTTYPETABLE  pTextType
)
{
  pTextType = get_TextTypeTable();
} /* end of function EQFBGetColors */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBGETREQSEG
//------------------------------------------------------------------------------
// Function call:     EQFBGETREQSEG(LONG, PULONG, PCHAR, PUSHORT)
//------------------------------------------------------------------------------
// Description:       Fill the allocated buffer with datastring of segment
//                    with the given segment number
//------------------------------------------------------------------------------
// Parameters:        LONG      lInfo,
//                    PUSHORT   pulSegNum,
//                    PCHAR     pchData,
//                    PUSHORT   pusBufSize
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       EQFRC_OK  everything okay
//                    EQFRS_ENTRY_NOT_AVAIL  out of range
//------------------------------------------------------------------------------
// Function flow:     if direction is zero:
//                       get current cursor segment number
//                    if segment number
//                      get current segment
//                      if previous or next searched
//                        find next/previous segment which is not joined
//                        copy segment data to given output buffer
//                        set data length and segment number for return purpose
//                    return error code
//------------------------------------------------------------------------------
USHORT EQFBGetReqSeg
(
  PTBDOCUMENT pDoc,
  SHORT     sDirection,                   // Next, Previous , Current
  PULONG    pulSegNum,
  PCHAR_W   pchData,
  PUSHORT   pusBufSize,
  PUSHORT   pusStatus
)
{
  USHORT        usRc;                            // return code
  PTBSEGMENT    pSeg;
  USHORT        usSegOffset;

  if (sDirection == 0 )
  {
    EQFBCurSegFromCursor(pDoc);
  //if (*pulSegNum == pDoc->TBCursor.ulSegNum)
  //} /* endif */
    *pulSegNum = pDoc->TBCursor.ulSegNum;
  } /* endif */

  if (*pulSegNum )
  {
    pSeg = EQFBGetSegW( pDoc,*pulSegNum );
    if (sDirection )                 // next = 1, previous = -1, current= 0
    {
      while (pSeg && pSeg->SegFlags.Joined)
      {
         (*pulSegNum)+= sDirection;
         pSeg = EQFBGetSegW( pDoc, *pulSegNum );
      } /* endwhile */
    } /* endif */
    if (pSeg && (pSeg->usLength <= *pusBufSize ))
    {
      UTF16strcpy(pchData, pSeg->pDataW);
      *pusBufSize = pSeg->usLength;
      if (pDoc->fLineWrap && pDoc->fAutoLineWrap )
      {
        /**************************************************************/
        /* remove soft lf's in copy of data                           */
        /**************************************************************/
          usSegOffset = 0;
          EQFBBufRemoveSoftLF(pDoc->hwndRichEdit, pchData, pusBufSize, &usSegOffset);
      } /* endif */

      *pulSegNum = pSeg->ulSegNum;
      *pusStatus = pSeg->qStatus;
      usRc = EQFRC_OK;
    }
    else
    {
      usRc = EQFRS_ENTRY_NOT_AVAIL;     // out of range
    } /* endif */
  }
  else
  {
    usRc = EQFRS_ENTRY_NOT_AVAIL;     // out of range
  } /* endif */
  return usRc;

}                                      // end 'EQFBGETReqSeg'

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncSHowTrans
//------------------------------------------------------------------------------
// Function call:     EQFBFuncShowTrans(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       call EQFSHOW in user exit
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   pDOc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     call EQFSHOW in user exit
//------------------------------------------------------------------------------

VOID EQFBFuncShowTrans
(
  PTBDOCUMENT     pDoc
)
{
  if ( pDoc->pfnShowTrans )
  {
    pDoc->pfnShowTrans( (LONG) pDoc, pDoc->hwndFrame );
  }
  else
  {
    EQFBFuncNothing( pDoc );
  } /* endif */
} /* end of function EQFBFuncShowTrans */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncDoLineWrap - wrap line at cursor position
//------------------------------------------------------------------------------
// Function call:     EQFBFuncDoLineWrap( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     insert linefeed at begin of current word
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//                      -start at begin of line (if it is same segment)
//                        else start at begin of segment
//                      -in case of DBCS
//                        call EQFBDBCSLineWRap
//                       else
//                        scan thru line and find split position; remember
//                        blank positions;
//                        replace the last one of consecutive blanks with a LF
//                        replace the following lf by a blank if line is not
//                        too long
//                        find next LF and replace it by a blank if following
//                        line is not too long; do not delete ending LF's
//                       endif
//                      -reset TBCursor, CursorCol,CursorROw to old pos
//                      -if LF was inserted
//                         recalc TBRowOffset table
//                         set redraw flag
//                         set DBCS CursorCol
//                       endif
//------------------------------------------------------------------------------
 VOID EQFBFuncDoLineWrap
 (
    PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
    LONG   lPos;                           //Column position on screen
    SHORT  sSegOffOld;                     //old TBCursor Segoffset
    PTBROWOFFSET pTBRow;                   // pointer to row structure
    SHORT  sSegOffset;                     //seg offset where to start
    BOOL   fLFInserted = FALSE;            //true if LF was inserted
    LONG   lStartCol;
    SHORT  sWordPos;                       // col of word begin
    PSZ_W  pData, pTemp;                   // pointer to data
    CHAR_W c, c0;
    LONG   lCurRow, lCurCol;               // cursor row and column
    LONG   lSideScroll;                    // remember start sidescroll
    CHAR_W cActLinefeed;                   // hard or soft linefeed


    if ( pDoc->hwndRichEdit )
    {
      /****************************************************************/
      /* no linewrapping necessary in case of RichEdit control        */
      /****************************************************************/
      return;
    } /* endif */




   /*******************************************************************/
   /* remember old settings ....                                      */
   /*******************************************************************/
   sSegOffOld = (SHORT)pDoc->TBCursor.usSegOffset;
   lCurRow = pDoc->lCursorRow;
   lCurCol = pDoc->lCursorCol;
   lSideScroll = pDoc->lSideScroll;                        /* KAT0008 A */
   /*******************************************************************/
   /* start at begin of line (if it is same segment)                  */
   /* else start at begin of segment                                  */
   /*******************************************************************/
   pTBRow = pDoc->TBRowOffset+pDoc->lCursorRow +1; // screen - row offset
   if ( pTBRow->ulSegNum == pDoc->ulWorkSeg )
   {
     sSegOffset = (SHORT)pTBRow->usSegOffset;
     lPos = 0;
   }
   else
   {
      sSegOffset = 0;
      pDoc->TBCursor.usSegOffset = 0;
      EQFBPhysCursorFromSeg(pDoc);               //set CursorCol/Row to
                                                 //begin of segment
      lPos = pDoc->lCursorCol + pDoc->lSideScroll;
   } /* endif */


   /*******************************************************************/
   /* linewrapping dependant of SBCS or DBCS mode                     */
   /*******************************************************************/
   if ( IsDBCS_CP(pDoc->ulOemCodePage))
   {
     fLFInserted = EQFBDBCSLineWrap(pDoc, sSegOffset, lPos, &sSegOffOld);
   }
   else
   {
     /*******************************************************************/
     /* usStartCol is nec if segment doesn't start in first col         */
     /*******************************************************************/
     sWordPos = sSegOffset;
     lStartCol = lPos;
     pData = pDoc->pEQFBWorkSegmentW + sSegOffset;
     /*****************************************************************/
     /* decide whether soft or hard lf's are inserted                 */
     /*****************************************************************/
     cActLinefeed = LF;
     if (pDoc->fAutoLineWrap )
     {
       cActLinefeed = SOFTLF_CHAR;
     } /* endif */
     /*******************************************************************/
     /* stop scanning the segment if end of segment or                  */
     /*  if sPos greater than RMargin as well as sWordPos not in first  */
     /*  column (except if sWordPos at begin of segment (== 0) )        */
     /* (linewrap is done if >RMargin, but only if not total line is    */
     /* wrapped around)                                                 */
     /*******************************************************************/
     //scan segment from beginning until sPos >= RMArgin
     c0 = BLANK;   // assures that it is not SoftLF_CHAR at start of seg
     while ( ((c=*pData)!= NULC) && (!ISLF(c, *(pData+1), c0)) &&
             !(( (lPos >= pDoc->sRMargin) /*|| EQFBRelCurPos( pDoc, sPos, pDoc->sRMargin )*/) &&
             ((sWordPos > (sSegOffset + lStartCol - lPos)) ||
              (lStartCol > 0))  ))
     {
       if ( (c == BLANK) &&
          ( EQFBCharType(pDoc,pDoc->pTBSeg,sSegOffset) == UNPROTECTED_CHAR))
       {
           //if BLANK unprotected, it is word delimiter
           sWordPos = sSegOffset;
       } /* endif */
       lPos ++;
       sSegOffset ++;                 //point to next char
       pData++;
       c0 = c;
     } /* endwhile */
     pTemp = pData;
     /***************************************************************/
     /* no blank found yet - try to find the first blank in the     */
     /* segment ...                                                 */
     /***************************************************************/
     if ( sWordPos == 0 )
     {
       BOOL fFound = FALSE;
       pTemp = pDoc->pEQFBWorkSegmentW+sWordPos;
       while ( !fFound )
       {
//         c = pDoc->EQFBWorkSegment[sWordPos];
         switch ( *pTemp )
         {
           case  BLANK:
             if ( EQFBCharType(pDoc,pDoc->pTBSeg,sWordPos) == UNPROTECTED_CHAR )
             {
               fFound = TRUE;
             }
             else
             {
               sWordPos++;
               pTemp++;
             } /* endif */
             break;
           case SOFTLF_CHAR:
             if (*(pTemp+1) != SOFTLF_CHAR )
             {
               fFound = TRUE;
             } /* endif */
           case EOS:
           case  LF:
             fFound = TRUE;
             break;
           default :
             sWordPos++;
             pTemp++;
             break;
         } /* endswitch */
       } /* endwhile */
     } /* endif */
     /*****************************************************************/
     /* if not end of segment, then linewrap has to be done           */
     /*****************************************************************/
     c = *pTemp;
     if (sWordPos )
     {
       c0 = *(pTemp-1);
     }
     else
     {
       c0 = BLANK;
     } /* endif */
     if ( c && !ISLF(c, *(pTemp+1), c0) )
     {
       /***************************************************************/
       /* skip consecutive blanks and replace the last one by a LF    */
       /***************************************************************/
       pTemp = pDoc->pEQFBWorkSegmentW + sWordPos;
       while ( ((c=*pTemp) == BLANK) || ISLF(c, *(pTemp+1), c0) )
       {
         sWordPos++;
         pTemp++;
         c0 = c;
       } /* endwhile */

       if ( sWordPos > 0 )
       {
         sWordPos--;
         pTemp--;
         if (sWordPos > 0 )
         {
           c0= *(pTemp-1);
         }
         else
         {
           c0 = BLANK;
         } /* endif */
         /***************************************************************/
         /* if we are on a LF than splitting not necessary because only */
         /* blanks are overlapping....                                  */
         /***************************************************************/
         if ( !ISLF(*pTemp, *(pTemp+1), c0 ) )
         {
//           pDoc->EQFBWorkSegment[sWordPos] = cActLinefeed;
           if (cActLinefeed != SOFTLF_CHAR )
           {
             *pTemp = cActLinefeed;
           }
           else
           {
             /*********************************************************/
             /* NEW: blank is followed by inserted SOFTLF             */
             /*********************************************************/
             USHORT  sCurPos = 0;
             sCurPos = (USHORT)(pTemp - (pDoc->pEQFBWorkSegmentW));
             EQFBWorkRight(pDoc, sCurPos, 1);
             if (sCurPos <= sSegOffOld )
             {
               sSegOffOld++;
             } /* endif */
             pTemp++;
             *pTemp = cActLinefeed;
             pDoc->pTBSeg->usLength ++;
           } /* endif */
           fLFInserted = TRUE;                       //set rc
           /***************************************************************/
           /* try to find next LF and replace it by a blank if line is    */
           /* less than the right margin specified...                     */
           /***************************************************************/
           pData = pTemp + 1;

         } /* endif */
       } /* endif */
     } /*endif*/
     /***************************************************************/
     /* try to find next LF and replace it by a blank if following  */
     /* line is not too long...                                     */
     /***************************************************************/
     if (fLFInserted)
     {
       c0 = *(pData-1);
       while ( ((c=*pData)!= NULC) && !ISLF(c, *(pData+1), c0) )
       {
         sSegOffset++;
         pData++;
         c0 = c;
       } /* endwhile */
       /***********************************************************/
       /* check if the character following the LF is part of a    */
       /* column based tag - if so we have to leave the LF in     */
       /* to avoid overtyping                                     */
       /***********************************************************/
       if ( c == LF )
       {
         /***************************************************************/
         /* erase only lf's which are not in front of tags              */
         /* further (planned) enhancement: check whether tag is column  */
         /* based; if it is not column based, lf is deleted             */
         /***************************************************************/
         c = (TACheckColumnPos ( pData+1,
                                 pDoc->pDocTagTable,
                                 (PTOKENENTRY) pDoc->pTokBuf,
                                 TOK_BUFFER_SIZE,
                                 pDoc->pfnUserExit )) ? 0 : LF ;
       } /* endif */
       /***********************************************************/
       /* now find the following LF and check if intermediate LF  */
       /* might be replaced by a BLANK                            */
       /* This approach ensures, that ending LFs will not be      */
       /* optimized away...                                       */
       /***********************************************************/
       if ( ISLF(c, *(pData+1), c0) )
       {
         pTemp = pData++;              // remember blank LF position
         c0 = c;
         while ( ((c=*pData)!= NULC) && !ISLF(c, *(pData+1), c0) )
         {
           sSegOffset++;
           pData++;
           c0=c;
         } /* endwhile */
         if ( ISLF(c, *(pData+1), c0)
              && (sSegOffset < sWordPos + pDoc->sRMargin)  /*&& !EQFBRelCurPos( pDoc, sSegOffset, (SHORT)(sWordPos+ pDoc->sRMargin ))*/)
         {
           pData = pTemp;
           /***********************************************************/
           /* replace softLF by blank which is deleted below          */
           /* ( to assure that softlf is deleted! )                   */
           /***********************************************************/
           if (cActLinefeed != SOFTLF_CHAR )
           {
             *pData ++ = BLANK;
           }
           else
           {
             *pData = BLANK;
             /*********************************************************/
             /* if not in DBCS language increase Data ptr to avoid    */
             /* removal of possible substituted lf (KBT0845)          */
             /*********************************************************/
             if ( !IsDBCS_CP(pDoc->ulOemCodePage) && (sSegOffset > 0) && (*(pData-1) != BLANK)  )
             {
               pData++;
             } /* endif */
           } /* endif */
           /*********************************************************/
           /* skip all further blanks and copy rest together...     */
           /*********************************************************/
           if ( *pData == BLANK )
           {
             pTemp = pData;
             while ( ((c=*pTemp)!= NULC) && (c==BLANK) )
             {
               pTemp++;
             } /* endwhile */
             while ( (*pData++ = *pTemp++) != NULC );
             EQFBCompSeg( pDoc->pTBSeg );
           } /* endif */
         } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */
   /*******************************************************************/
   /* do adjustments when changes....                                 */
   /*******************************************************************/
   pDoc->TBCursor.usSegOffset = (SHORT)sSegOffOld; // reset to old cursor
   if ( fLFInserted )
   {
      pDoc->lSideScroll = 0;                     // reset side scroll
      UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusBPET) ,0L ,0L , NOMSG);    /* @KIT1332A */

       if (pDoc->pTBSeg->pusHLType )
       {
         pDoc->pTBSeg->SegFlags.Spellchecked = FALSE;
         UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusHLType) ,0L ,0L , NOMSG);

       } /* endif */

      EQFBScrnLinesFromSeg ( pDoc,                  // pointer to doc ida
                             0,                     // starting row
                             pDoc->lScrnRows,       // number of rows
                                                    // starting segment
                             (pDoc->TBRowOffset+1));
      EQFBPhysCursorFromSeg( pDoc );                // get new cursor position
      pDoc->Redraw |= REDRAW_ALL;                   // redraw the whole screen

      pDoc->lDBCSCursorCol = pDoc->lCursorCol;          //set to actual Col
   }
   else
   {
//    EQFBPhysCursorFromSeg( pDoc );                       // reset old crscol/row
      pDoc->lCursorRow = lCurRow;
      pDoc->lCursorCol = lCurCol;
      pDoc->lSideScroll = lSideScroll;                         /* KAT0008 A */
   } /* endif */

   return;
 }



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBDBCSLineWrap - wrap current seg if DBCS
//------------------------------------------------------------------------------
// Function call:     EQFBDBCSLineWrap( PTBDOCUMENT );
//------------------------------------------------------------------------------
// Description  :     wrap if DBCS
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     scan segment til end or right of right margin
//                    if linewrap should be done
//                      insert linefeed
//                      delete blanks at beginning of next line
//                    if linefeed has been inserted
//                      check next lf and delete or substitute it by a blank
//                    return indicator whether linewrap happened
//------------------------------------------------------------------------------
 static BOOL
 EQFBDBCSLineWrap
 (
    PTBDOCUMENT pDoc,                                      //pointer to Doc instance
    SHORT       sSegOffset,
    LONG        lPos,
    PSHORT      psSegOffOld
 )
 {
   PSZ_W  pData;
   CHAR_W c, c0;
   SHORT  sWordPos;
   LONG   lStartCol;
   BOOL   fInWord = FALSE;
   BOOL   fLFInserted = FALSE;
   SHORT  sSegOldDiff;
   SHORT  sIndex;
   SHORT  sTempOffset;
   PSZ_W  pTemp;
   CHAR_W cActLinefeed;
   BOOL   fDelLF = FALSE;


   sWordPos = sSegOffset;
   lStartCol = lPos;
   pData = pDoc->pEQFBWorkSegmentW + sSegOffset;
     /*****************************************************************/
     /* decide whether soft or hard lf's are inserted                 */
     /*****************************************************************/
     cActLinefeed = LF;
     if (pDoc->fAutoLineWrap )
     {
       cActLinefeed = SOFTLF_CHAR;
     } /* endif */

    if (sSegOffset )
    {
      c0 = *(pData-1);
    }
    else
    {
      c0 = BLANK;
    } /* endif */
   /*******************************************************************/
   /* stop scanning the segment if end of segment or                  */
   /*  if sPos greater than RMargin as well as sWordPos not in first  */
   /*  column (except if sWordPos at begin of segment (== 0) )        */
   /* (linewrap is done if >RMargin, but only if not total line is    */
   /* wrapped around)                                                 */
   /*******************************************************************/
   while ( ((c=*pData)!= NULC) && !ISLF(c, *(pData+1), c0) &&
         !( ((lPos >= pDoc->sRMargin) /*|| EQFBRelCurPos( pDoc, sPos, pDoc->sRMargin  )*/) &&
           ((sWordPos > (sSegOffset + lStartCol - lPos)) ||    /* @G3C */
            (lStartCol > 0))  ))                                    /* @G3A */
   {
       if ( EQFIsDBCSChar( c, pDoc->ulOemCodePage ))
       {
           sWordPos = sSegOffset;
           fInWord = FALSE;
           lPos +=2;                // every DBCS char takes 2 display positions
           sSegOffset++;
           pData++;
           c0 = c;
       }
       else
       {
         if ( c == BLANK )
         {
           if ( EQFBCharType(pDoc,pDoc->pTBSeg,sSegOffset) == UNPROTECTED_CHAR  )
           {
             //if BLANK unprotected, it is word delimiter
             sWordPos = sSegOffset;
             fInWord = FALSE;
           } /* endif */
         }
         else
         {
           if ( !fInWord )
           {
             fInWord = TRUE;
             sWordPos = sSegOffset;
           } /* endif */
         } /* endif */
         lPos ++;
         sSegOffset ++;                 //point to next char
         pData++;
         c0 = c;
       } /* endif */
   } /* endwhile */

   /*****************************************************************/
   /* if not end of segment, then linewrap has to be done           */
   /*****************************************************************/

   if (sWordPos )
   {
     c0 = *(pData-1);
   }
   else
   {
     c0 = BLANK;
   } /* endif */
   if ( c && !ISLF(c, *(pData+1), c0) )
   {
     /*****************************************************************/
     /* force that softLF is always inserted, and not replace of a    */
     /* blank is done!                                                */
     /*****************************************************************/
     if ((cActLinefeed != SOFTLF_CHAR)
         && sWordPos && (pDoc->pEQFBWorkSegmentW[sWordPos-1] == BLANK) )
     {
       pDoc->pEQFBWorkSegmentW[sWordPos-1] = cActLinefeed;
       fLFInserted = TRUE;
     }
     else
     {
       //make room for LF
       EQFBWorkRight(pDoc,sWordPos,1);
       if ( sWordPos <= *psSegOffOld)
       {
        (*psSegOffOld)++;               //adjust if WorkSegment changed
       } /* endif */
       pDoc->pEQFBWorkSegmentW[sWordPos] = cActLinefeed;
       fLFInserted = TRUE;                                 //set rc
       pDoc->pTBSeg->usLength ++;                // increase length

       EQFBReparse(pDoc, pDoc->pTBSeg,sWordPos,1);
       pData++;
       sWordPos++;                               // err fix! is needed!(06/96)
       sSegOffset++;
     } /* endif */
     /***************************************************************/
     /* delete blanks at beginning of next line                     */
     /* if next line would be empty, delete LF too                  */
     /* (to ensure that no empty lines are inserted if there is     */
     /* only one long word in previous line which cannot be wrapped)*/
     /***************************************************************/
     sIndex = sWordPos;
     sSegOldDiff = 0;
     while ( pDoc->pEQFBWorkSegmentW[sIndex] == BLANK )
     {
       if ( sIndex < *psSegOffOld )
       {
         sSegOldDiff++;
       } /* endif */
       if ( sSegOffset > sIndex )
       {
         sSegOffset --;
         pData --;
         lPos--;
       } /* endif */
       sIndex ++;
     } /* endwhile */
     pData = pDoc->pEQFBWorkSegmentW+sIndex;
     c = *pData;
     if (sIndex )
     {
       c0 = *(pData-1);
     } /* endif */
     if ( ISLF(c, *(pData+1), c0) )
     {
       if ( sIndex < *psSegOffOld )
       {
         sSegOldDiff++;
       } /* endif */
       sIndex ++;
     } /* endif */
     sIndex = sIndex - sWordPos;                       //set to difference
     if ( sIndex )
     {
       *psSegOffOld = *psSegOffOld - sSegOldDiff;
       EQFBWorkLeft(pDoc,sWordPos,sIndex);
       pDoc->pTBSeg->usLength = pDoc->pTBSeg->usLength - sIndex;       // decrease length
       EQFBReparse(pDoc, pDoc->pTBSeg,sWordPos,(SHORT)(-sIndex));
     } /* endif */
   } /*endif*/

   /***************************************************************/
   /* try to find next LF and delete  it            if following  */
   /* line is not too long...                                     */
   /***************************************************************/
   if (fLFInserted)
   {
     pData = &pDoc->pEQFBWorkSegmentW[sSegOffset];
     if (sSegOffset )
     {
       c0 = *(pData-1);
     }
     else
     {
       c0 = BLANK;
     } /* endif */
     while ( ((c= *pData)!= NULC) && !ISLF(c, *(pData+1), c0) )
     {
       sSegOffset++;
       pData++;
       c0 = c;
     } /* endwhile */
     /***********************************************************/
     /* check if the character following the LF is part of a    */
     /* column based tag - if so we have to leave the LF in     */
     /* to avoid overtyping                                     */
     /***********************************************************/
     if ( c == LF )
     {
       /***************************************************************/
       /* erase only lf's which are not in front of tags              */
       /* further (planned) enhancement: check whether tag is column  */
       /* based; if it is not column based, lf is deleted             */
       /***************************************************************/
       c = (TACheckColumnPos ( pData+1,
                               pDoc->pDocTagTable,
                               (PTOKENENTRY) pDoc->pTokBuf,
                               TOK_BUFFER_SIZE,
                               pDoc->pfnUserExit )) ? 0 : LF ;
     } /* endif */
     /***********************************************************/
     /* now find the following LF and check if intermediate LF  */
     /* might be replaced by a BLANK                            */
     /* This approach ensures, that ending LFs will not be      */
     /* optimized away...                                       */
     /***********************************************************/
     if ( ISLF(c, *(pData+1), c0) )
     {
       c0 = *pData;
       pTemp = pData++;                // remember blank LF position
       while ( ((c=*pData)!= NULC) && !ISLF(c, *(pData+1), c0) )
       {
         sSegOffset++;
         pData++;
         c0 = c;
       } /* endwhile */
       c0 = *(pData-1);
       if ( ISLF(c, *(pData+1), c0) &&
            (sSegOffset < sWordPos + pDoc->sRMargin)  /*&& !EQFBRelCurPos( pDoc, sSegOffset, (SHORT)(sWordPos+pDoc->sRMargin )) */)
       {
         /***********************************************************/
         /* do not substitute LF by a blank between 2 DBCS chars    */
         /* or if a blank is preceeding the lf - pData points to LF */
         /***********************************************************/
         pData = pTemp;
         sTempOffset = (USHORT)(pTemp - &pDoc->pEQFBWorkSegmentW[0]);
         c = *(pTemp+1);
         fDelLF = (*(pTemp - 1) == BLANK );
         if (!fDelLF && EQFIsDBCSChar(c, pDoc->ulOemCodePage)
              && EQFIsDBCSChar(*(pTemp-1), pDoc->ulOemCodePage) )
         {
             fDelLF = TRUE;
         }
         if (fDelLF )
         {
           pTemp = pData + 1;          // skip LF to which pData points
         }
         else
         {
           *pData++ = BLANK;
           /***********************************************************/
           /* pData points to pos after replaced LF!                  */
           /***********************************************************/
           pTemp = pData;
         } /* endif */
         /*********************************************************/
         /* skip all further blanks and copy rest together...     */
         /*********************************************************/
         if ( *pTemp == BLANK )
         {
           while ( ((c=*pTemp)!= NULC) && ((c==BLANK)!= NULC) )
           {
             pTemp++;
           } /* endwhile */
         } /* endif */
         if (pData != pTemp )
         {
           while ( (*pData++ = *pTemp++) != NULC );
           EQFBCompSeg( pDoc->pTBSeg );
         } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */

   return(fLFInserted);
 }


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncLineWrap - wrap lines for total active segment
//------------------------------------------------------------------------------
// Function call:     EQFBFuncLineWrap( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     do reflow for total active segment
//                    reset cursor to begin of segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if not reflow or margin out of range
//                      beep
//                      return
//                    endif
//                      - update UNDO buffer
//                      - set sCursorCol/Row to begin of segment
//                      - scan segment for all linefeeds and delete all
//                          superfluous linefeeds (EQFBScanLF)
//                      - scan total segment and insert LF where nec
//                        (EQFBInsertLF)
//                      - position at segment and stored old cursor position
//                      - reset old block mark and worksegment change flag
//
//------------------------------------------------------------------------------
 VOID EQFBFuncLineWrap
 (
    PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
    USHORT usSegOffOld;                    //TBCursor.SegOffset at begin
    ULONG  ulPos;                          // col position


    // in case of new RTF Control, do nothing
    if (pDoc->hwndRichEdit)
      return;

    if ( pDoc->fAutoLineWrap || (! pDoc->EQFBFlags.Reflow) || pDoc->sRMargin > 999 )   // @KWT0028M
    {
       EQFBFuncNothing( pDoc );
       return;                // Not nice but very efficient here !!!!
    } /* endif */

    if ( pDoc->pUndoSegW )
    {                                      //update undo buffer
      UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
      pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
      pDoc->fUndoState = FALSE;            //function is not a typing one
    } /* endif */
    usSegOffOld = pDoc->TBCursor.usSegOffset;
    pDoc->TBCursor.usSegOffset = 0;

    EQFBPhysCursorFromSeg(pDoc);          //set CursorCol/Row to
                                          //begin of segment

    /******************************************************************/
    /* delete all superfluous linefeeds                               */
    /******************************************************************/
   EQFBScanLF(pDoc,&usSegOffOld,pDoc->pTBSeg,pDoc->TBCursor.usSegOffset);

   /*******************************************************************/
   /* scan segment from begin and insert LF where nec                 */
   /* adjust old Cursor segment offset (usSegOffOld)                  */
   /* the 'TRUE' indicates, that total segment has to be scanned      */
   /*******************************************************************/
   ulPos = pDoc->lCursorCol + pDoc->lSideScroll;
   EQFBInsertLF(pDoc,pDoc->pTBSeg,&usSegOffOld,
                &ulPos,pDoc->TBCursor.usSegOffset );

   // position at segment and stored old cursor position
   EQFBGotoSeg( pDoc, pDoc->ulWorkSeg, usSegOffOld );

   // reset any old block mark in same segment
   EQFBFuncResetMarkInSeg( pDoc );

   pDoc->EQFBFlags.workchng = TRUE;    //work segment changed
   pDoc->pTBSeg->SegFlags.Typed = TRUE;

   MTLogStartEditing( pDoc );

   pDoc->ActSegLog.usNumTyped ++;

   return;
 }
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBScanLF
//------------------------------------------------------------------------------
// Function call:     EQFBScanLF(PTBDOCUMENT,PSZ,PUSHORT)
//------------------------------------------------------------------------------
// Description:        scan segment for \n;
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to doc instance
//                    PUSHORT pusSegOffOld old segment offset
//                    PTBSEGMENT pSeg   pointer to segment
//                    USHORT usPos      starting offset in segment
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    if LF was inserted
//                    FALSE   if LF cannot be inserted
//------------------------------------------------------------------------------
// Side effects:      pDoc->EQFBWOrkSegment may be changed
//------------------------------------------------------------------------------
// Function flow:      scan segment for LF;
//                     if next char after LF not blank and not EOS,
//                     then delete leading blanks and change \n into blank
//                     adjust usSegOffOld
//+                    if LF between 2 DBCS chars, del.LF and don't replace it
//------------------------------------------------------------------------------
static
BOOL EQFBScanLF
(
  PTBDOCUMENT pDoc,                       //pointer to doc instance
  PUSHORT pusSegOffOld,                   //old segment offset
  PTBSEGMENT pSeg,
  USHORT usPos                            // starting offset in segment
)
{
  PSZ_W  pData;
  PSZ_W  pDataOld;                        //sstores input of pData
  CHAR_W c,c2;                            //character of segment
  USHORT usRc = FALSE;
  USHORT usType_Char;                     //rc from EQFBCharType
  USHORT usType_NextChar;                 //rc from EQFBCharType,next char
  DISPSTYLE DispStyle;
  BOOL fChanged = FALSE;                  //return true if s.th. changed

  DispStyle = pDoc->DispStyle;            // store old dispstyle
  pDoc->DispStyle = (pDoc->DispStyle == DISP_UNPROTECTED) ?
                 DISP_PROTECTED : (pDoc->DispStyle);
  pData = pDoc->pEQFBWorkSegmentW + usPos;
  pDataOld = pData;
  usType_NextChar = EQFBCharType(pDoc,pDoc->pTBSeg,usPos);
  while ( (c = *pData) != NULC)         // while not at end of segment
  {
     usType_Char = usType_NextChar;
     usType_NextChar = EQFBCharType(pDoc,pDoc->pTBSeg,(USHORT)(usPos+1));

     if ((usType_Char == LINEBREAK_CHAR)
          && (c2=*(pData+1))!= NULC )   // if at lf and next is not eos
     {
       BOOL fColumnBased;              // indicator if found tag is column based
       /***************************************************************/
       /* erase only lf's which are not in front of tags              */
       /* further (planned) enhancement: check whether tag is column  */
       /* based; if it is not column based, lf is deleted             */
       /***************************************************************/
       switch ( usType_NextChar )
       {
         case PROTECTED_CHAR:
         case HIDDEN_CHAR:
         case COMPACT_CHAR:
           fColumnBased = TACheckColumnPos ( pDoc->pTBSeg->pDataW+usPos+1,
                                             pDoc->pDocTagTable,
                                             (PTOKENENTRY) pDoc->pTokBuf,
                                             TOK_BUFFER_SIZE,
                                             pDoc->pfnUserExit );
           break;
         default :
           fColumnBased = FALSE;
           break;
       } /* endswitch */

       if ( (c2 != BLANK) && !fColumnBased )
       {
          // delete blanks leading \n
          while (*(pData-1) == BLANK && usPos >0)
          {
             EQFBWorkLeft(pDoc,usPos,1);
             if ( usPos <= *(pusSegOffOld))
             {
               (*pusSegOffOld)--;       //adjust if WorkSegment changed
             } /* endif */
             fChanged = TRUE;
             pSeg->usLength --;         // decrease length
             EQFBReparse(pDoc, pSeg,(USHORT)(usPos-1),-1);
             pData--;
             usPos--;
          } /* endwhile */
                                  //change \n into blank
          if (usPos >0)
          {
            if ( usRc )
            {
              EQFBWorkLeft(pDoc,usPos,1);   //delete \n
              if ( usPos <= *(pusSegOffOld))
              {
                 (*pusSegOffOld)--;       //adjust if WorkSegment changed
              } /* endif */
              fChanged = TRUE;
              pSeg->usLength --;         // decrease length
              EQFBReparse(pDoc, pSeg,(USHORT)(usPos-1),-1);
              pData--;
              usPos--;
              usRc = FALSE;
            }
            else
            {
              *pData = BLANK;              //replace \n with blank
              fChanged = TRUE;                                   /* @KIT1332A */
            } /* endif */
          }
          else                    // \n is 1st char in segment
          {
             EQFBWorkLeft(pDoc,0,1);         //delete \n
             if ( usPos <= *(pusSegOffOld))
             {
               (*pusSegOffOld)--;       //adjust if WorkSegment changed
             } /* endif */
             fChanged = TRUE;
             pSeg->usLength --;         // decrease length
             EQFBReparse(pDoc, pSeg,(USHORT)(usPos-1),-1);
             pData--;
             usPos--;
          } /* endif */
       } /* endif */
     } /* endif */
     pData ++;                               // point to next character
     usPos ++;
  } /* endwhile */


  pDoc->DispStyle = DispStyle;               // restore old dispstyle
  return(fChanged);
} /* end of function  */

//------------------------------------------------------------------------------
// Internal function                                      /* KWT0027A */
//------------------------------------------------------------------------------
// Function name:     TACheckColumnPos
//------------------------------------------------------------------------------
// Description:       function will check if tag following the LF is column
//                    based.
//                    Return indicator if tag is column based of not.
//                    If we are dealing with a provided user exit for creating
//                    a tag table we assume that tag is column based.
//------------------------------------------------------------------------------
// Parameters:        PSZ           pData,            ptr to data
//                    PVOID         pVoidTable,       pointer to tag table
//                    PTOKENENTRY   pTokBuffer,       buffer used for tokens
//                    USHORT        usTokBufferSize,  token buffer size
//                    PVOID         pvUserExit        ptr to user exit function
//
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   column based tag found
//                    FALSE  no column based tag found
//------------------------------------------------------------------------------
// Function flow:     check input parameters
//                    if okay,
//                      tokenize passed data and check if starting tag
//                      is columnbased.
//------------------------------------------------------------------------------

BOOL TACheckColumnPos
(
  PSZ_W         pData,                 // ptr to data
  PVOID         pVoidTable,            // pointer to tag table
  PTOKENENTRY   pTokBuffer,            // buffer used for tokens
  USHORT        usTokBufferSize,       // token buffer size
  PFN           pvUserExit             // ptr to user exit function
)
{
  BOOL  fColumnBased = FALSE;          // tag data are not column based
  PLOADEDTABLE pTagTable;              // pointer to tag table

  pTagTable = (PLOADEDTABLE) pVoidTable;

  /********************************************************************/
  /* check input parameters                                           */
  /********************************************************************/

  if ( !pData || !pTagTable || !pTokBuffer || (usTokBufferSize==0)
       || pvUserExit )
  {
    /******************************************************************/
    /* to be on the secure side                                       */
    /******************************************************************/
//    fColumnBased = TRUE;
    fColumnBased = FALSE;
  } /* endif */
  else
  /********************************************************************/
  /* tokenize passed data and check if first token is a tag which     */
  /* is column based                                                  */
  /********************************************************************/
  {
    SHORT sNumTags = (SHORT) pTagTable->pTagTable->uNumTags;
    PBYTE pByte = (PBYTE) pTagTable->pTagTable;
    PTAG  pTag = OFFSETTOPOINTER(PTAG, pTagTable->pTagTable->stFixTag.uOffset);
    PSZ_W  pRest = NULL;
    USHORT usColPos = 0;

    TATagTokenizeW( pData,
                   pTagTable,
                   TRUE,
                   &pRest,
                   &usColPos,
                   pTokBuffer,
                   (USHORT)(usTokBufferSize / sizeof( TOKENENTRY )));

    if ( (pTokBuffer->sTokenid < sNumTags) && (pTokBuffer->sTokenid >= 0)
         && pTag[pTokBuffer->sTokenid].usPosition )
    {
        fColumnBased = TRUE;                              // ROB decided so
//      fColumnBased = (pTag[pTokBuffer->sTokenid].usPosition == 1);
    } /* endif */
  } /* endif */

  return fColumnBased;
} /* end of function TACheckColumnPos */
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBInsertLF
//------------------------------------------------------------------------------
// Function call:     EQFBInsertLF(PTBDOCUMENT,PTBSEGMENT,PUSHORT,
//                                 PUSHORT,USHORT )
//------------------------------------------------------------------------------
// Description:       insert all nec linefeeds
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to doc instance
//                    PTBSEGMENT pSeg   pointer to segment
//                    PUSHORT pusSegOffOld old segment offset
//                    PUSHORT pusPos    starting col position
//                    USHORT usSegOffset starting seg offset
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    if LF was inserted
//                    FALSE   if LF cannot be inserted
//------------------------------------------------------------------------------
// Side effects:      pDoc->EQFBWorkSegment may be changed
//------------------------------------------------------------------------------
// Function flow:     while not end of segment
//                      while(not right of RMargin and not total line wrapped
//                            and not end of segment )
//                        if current char is DBCS
//                          cur char is word begin,word delimiter
//                          increase variables by 2
//                        else
//                          switch char
//                            case LF: set variables to begin of line
//                            case BLANK:if unprotected, set as word delimiter
//                            default: set indicator to be inside a word
//                          endswitch
//                          increase pointers by 1
//                        endif
//                      endwhile
//                      if not end of segment(then linewrap has to be done)
//                        shift to the right one position
//                        adjust old segment offset
//                        insert LF and set return indicator
//                        adjust length of segment, reparse segment
//                        adjust pointers and variables(due to EQFBWorkRight)
//                        delete blanks at beginning of next line
//                      endif
//                    endwhile
//------------------------------------------------------------------------------
static
BOOL EQFBInsertLF
(
  PTBDOCUMENT pDoc,                       //pointer to doc instance
  PTBSEGMENT pSeg,                        //pointer to EQFBWorkSegment
  PUSHORT pusSegOffOld,                   //old segment offset
  PULONG  pulPos,                         //starting col pos
  USHORT usSegOffset                      //starting seg offset
)
{
    PSZ_W  pData;                         //pointer to worksegment
    BOOL   fInWord = FALSE;               //TRUE if in word
    SHORT  sWordPos = 0;                  //column of word begin
    CHAR_W c;                             //character of segment
    ULONG  ulStartCol;                    //stores current cursor col.
    SHORT sIndex;                         //index for while loop
    SHORT sSegOldDiff = 0;                //how much to change usSegOffOld
    ULONG ulOurPos;                      //=usPOs, to avoid *pusPos
    BOOL fLFInserted = FALSE;             //return true if LF inserted

   /*******************************************************************/
   /* usStartCol is nec if segment doesn't start in first col         */
   /*******************************************************************/
   ulOurPos = *pulPos;
   ulStartCol = *pulPos;
   pData = pDoc->pEQFBWorkSegmentW + usSegOffset;
   while ( (c = *pData) != NULC )                // while not end of segment
   {
   /*******************************************************************/
   /* stop scanning the segment if end of segment or                  */
   /*  if usPos greater than RMargin as well as sWordPos not in first */
   /*  column (except if sWordPos at begin of segment (== 0) )        */
   /* (linewrap is done if >RMargin, but only if not total line is    */
   /* wrapped around)                                                 */
   /*******************************************************************/
     //scan segment from beginning until usPos >= RMArgin
     while ( (c=*pData)!= NULC &&
           !( (((SHORT)ulOurPos >= pDoc->sRMargin)   /* || EQFBRelCurPos( pDoc, usOurPos, pDoc->sRMargin )*/) &&
             ((sWordPos > (SHORT)(usSegOffset + ulStartCol - ulOurPos)) ||
              (ulStartCol > 0))  ))
     {
       if ( IsDBCS_CP(pDoc->ulOemCodePage) && EQFIsDBCSChar(c, pDoc->ulOemCodePage) )
       {
         sWordPos = usSegOffset;        //current char is word begin
         fInWord = FALSE;               //DBCS char is word delimiter
         ulOurPos +=2;                  // every DBCS Char is taking 2 display positions
         usSegOffset ++;
         pData ++;
       }
       else
       {
         /*************************************************************/
         /* soft linebreaks are all deleted in ScanLF!                */
         /*************************************************************/
         switch ( c )
         {
           case  LF:
             ulOurPos =0;
             fInWord = FALSE;
             sWordPos = usSegOffset;
             ulStartCol = 0;
             break;
           case  BLANK:
             if ( EQFBCharType(pDoc,pDoc->pTBSeg,usSegOffset)
                                           == UNPROTECTED_CHAR  )
             {
               //if BLANK unprotected, it is word delimiter
               sWordPos = usSegOffset;
               fInWord = FALSE;
             } /* endif */
             ulOurPos ++;
             break;
           default :
             if ( !fInWord )
             {
               fInWord = TRUE;
               sWordPos = usSegOffset;
             } /* endif */
             ulOurPos++;
             break;
         } /* endswitch */
         usSegOffset ++;                 //point to next char
         pData++;
       } /* endif */
     } /* endwhile */
     /*****************************************************************/
     /* if not end of segment, then linewrap has to be done           */
     /*****************************************************************/
     if ( c )
     {
       //make room for LF
       EQFBWorkRight(pDoc,sWordPos,1);
       if ( sWordPos <= (SHORT)(*pusSegOffOld) )
       {
        (*pusSegOffOld)++;               //adjust if WorkSegment changed
       } /* endif */

       pDoc->pEQFBWorkSegmentW[sWordPos] = LF;
       fLFInserted = TRUE;                 //set rc
       pSeg->usLength ++;                            // increase length
       EQFBReparse(pDoc, pSeg,sWordPos,1);
       /***************************************************************/
       /* adjust pointers and variables (due to EQFBWorkRight)        */
       /* so that they stay on same character                         */
       /* do not reset fInWord !  , c is not changed                  */
       /***************************************************************/
       pData++;
       usSegOffset++;
       sWordPos++;
       ulOurPos = usSegOffset - sWordPos;
       ulStartCol = 0;
       /***************************************************************/
       /* delete blanks at beginning of next line                     */
       /* if next line would be empty, delete LF too                  */
       /* (to ensure that no empty lines are inserted if there is     */
       /* only one long word in previous line which cannot be wrapped)*/
       /***************************************************************/
       sIndex = sWordPos;
       sSegOldDiff = 0;
       while ( pDoc->pEQFBWorkSegmentW[sIndex] == BLANK )
       {
         if ( sIndex < (SHORT)(*pusSegOffOld) )
         {
           sSegOldDiff++;
         } /* endif */
         if ( (SHORT) usSegOffset > sIndex )
         {
           usSegOffset --;
           pData --;
           ulOurPos--;
         } /* endif */
         sIndex ++;
       } /* endwhile */
       /***************************************************************/
       /* only hard LF can exist after ScanLF                         */
       /***************************************************************/
       if ( pDoc->pEQFBWorkSegmentW[sIndex] == LF )
       {
         if ( sIndex < (SHORT)(*pusSegOffOld) )
         {
           sSegOldDiff++;
         } /* endif */
         sIndex ++;
       } /* endif */
       sIndex = sIndex - sWordPos;                                 //set to difference
       if ( sIndex )
       {
         *pusSegOffOld = *pusSegOffOld - sSegOldDiff;
         EQFBWorkLeft(pDoc,sWordPos,sIndex);
         pSeg->usLength = pSeg->usLength - sIndex;                         // decrease length
         EQFBReparse(pDoc, pSeg,sWordPos,(SHORT)(-sIndex));
       } /* endif */
     } /* endif */
   } /* endwhile */
   *pulPos = ulOurPos;                     //return the new position
  return(fLFInserted);
} /* end of function  */




//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBSoftLFRemove
//------------------------------------------------------------------------------
// Function call:     EQFBSoftLFRemove(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       remove soft LFs in all segments
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     while not at end of file
//                      get segment
//                      call EQFBBufRemoveSoftLF
//                      goto next segment
//                    endhwile
//------------------------------------------------------------------------------

VOID  EQFBSoftLFRemove
(
   PTBDOCUMENT  pDoc
)
{
   DISPSTYLE   pOldStyle;
   ULONG       ulSegNum;
   PTBSEGMENT  pSeg;
   PSZ_W       pData;
   USHORT      usBufSize;
   USHORT      usSegOffset;
   SHORT       sSign= 1;

   if ( !pDoc->hwndRichEdit )
   {
     pOldStyle = pDoc->DispStyle;
     pDoc->DispStyle = DISP_PROTECTED;
     ulSegNum = 1;
     while (ulSegNum < pDoc->ulMaxSeg )
     {
       /*****************************************************************/
       /* remove softlf's from not visible joined segs too              */
       /*****************************************************************/
       pSeg = EQFBGetSegW(pDoc, ulSegNum);
       /*****************************************************************/
       /* if segment is shrinked, handle shrinked blocks                */
       /*****************************************************************/
       if (pSeg && ISQFNOP(pSeg->qStatus) &&
            (    (pDoc->DispStyle == DISP_SHRINK)
              || (pDoc->DispStyle == DISP_COMPACT) ) )
       {
         pSeg = EQFBGetShrinkSeg(pDoc, &ulSegNum, sSign);
       } /* endif */
       if (pSeg )
       {
         pData = pSeg->pDataW;
         usSegOffset = 0;
         EQFBBufRemoveSoftLF ( pDoc->hwndRichEdit, pData, &usBufSize , &usSegOffset);
         pSeg->usLength = usBufSize;
         UtlAlloc((PVOID *)&(pSeg->pusBPET), 0L, 0L, NOMSG );          // free start/stop

         if (pSeg->pusHLType )
         {
           pSeg->SegFlags.Spellchecked = FALSE;
           UtlAlloc((PVOID *)&(pSeg->pusHLType) ,0L ,0L , NOMSG);
         } /* endif */

       } /* endif */
       ulSegNum++;
     } /* endwhile */
     pDoc->DispStyle = pOldStyle;
   } /* endif */
   return;
} /* end of function EQFBSoftLFRemove */



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBSegSoftLFInsert
//------------------------------------------------------------------------------
// Function call:     EQFBSegSoftLFInsert (pDoc, pSeg, usCurOffset)
//------------------------------------------------------------------------------
// Description:       insert soft LFs in segmetn at given position
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc,
//                    PTBSEGMENT   pSeg,
//                    USHORT       usCurOffset
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     success
//                    FALSE    no success
//------------------------------------------------------------------------------
// Function flow:     alloc buffer if nec
//                    if new length is not over max.segment size:
//                      copy data and insert SOFTLF
//                      free old data
//------------------------------------------------------------------------------

BOOL  EQFBSegSoftLFInsert
(
   PTBDOCUMENT  pDoc,
   PTBSEGMENT   pSeg,
   USHORT       usCurOffset
)
{
  USHORT usConvDataLen;                //length of converted data string
  USHORT usLen;                        //length to alloc for new pSeg-pData
  BOOL   fResult = TRUE;               //success indicator
  PSZ_W  pTmpBuf = NULL;               // pointer to temp buffer
  PSZ_W  pSrc, pTgt;

  if (!pDoc->hwndRichEdit )
  {
    if (!pDoc->pSegmentBufferW )
    {
      fResult = UtlAlloc((PVOID *)&(pDoc->pSegmentBufferW),
                         0L, (LONG)(MAX_SEGMENT_SIZE+1)*sizeof(CHAR_W),
                         ERROR_STORAGE);
    } /* endif */
    if ( fResult )
    {
      usConvDataLen = pSeg->usLength + 1;
      if (usConvDataLen+1 > MAX_SEGMENT_SIZE)
      {
        fResult = FALSE;
      }
      else
      {
        UTF16strcpy( pDoc->pSegmentBufferW, pSeg->pDataW );
        pSrc = pDoc->pSegmentBufferW + usCurOffset;
        pTgt = pSrc + 1;
        if (usConvDataLen > usCurOffset +1 )
        {
          memmove(pTgt, pSrc,(usConvDataLen - usCurOffset + 1)*sizeof(CHAR_W));
          *(pDoc->pSegmentBufferW + usCurOffset ) = SOFTLF_CHAR;    //add SOFTLF at usCurOffset
          usConvDataLen ++;
        }
        else
        {
          if (usConvDataLen == usCurOffset +1 )
          {
            *(pDoc->pSegmentBufferW + usCurOffset ) = SOFTLF_CHAR;
            *(pDoc->pSegmentBufferW + usCurOffset +1) = EOS;
            usConvDataLen++;
          } /* endif */
        } /* endif */
      } /* endif */

      if ( fResult )
      {
        if ( (pSeg->ulSegNum != pDoc->ulWorkSeg)  ||
              (&pDoc->pEQFBWorkSegmentW[0] != pSeg->pDataW)  )   //@@
        {
          usLen = max( usConvDataLen+1, MIN_ALLOC ); // get rid of warning
                                                   // in UtlAlloc
          pTmpBuf = pSeg->pDataW;                   // get pointer to data
          fResult = UtlAlloc((PVOID *) &(pSeg->pDataW),
                   0L, (LONG) (usLen * sizeof(CHAR_W)), ERROR_STORAGE );
        }
        else
        {
  //        pDoc->EQFBFlags.workchng = TRUE;     // donot displ.EQF600 "WorkSegChanged"
        } /* endif */
        if ( fResult )
        {
          UTF16strcpy( pSeg->pDataW, pDoc->pSegmentBufferW );
          pSeg->usLength = (USHORT)UTF16strlenCHAR(pSeg->pDataW);
          UtlAlloc((PVOID *)&(pSeg->pusBPET), 0L, 0L, NOMSG );          // free start/stop

          if (pSeg->pusHLType )
          {
            pSeg->SegFlags.Spellchecked = FALSE;
            UtlAlloc((PVOID *)&(pSeg->pusHLType) ,0L ,0L , NOMSG);
          } /* endif */

          if ( pTmpBuf )
          {
            UtlAlloc((PVOID *)&pTmpBuf, 0L, 0L, NOMSG );                // data space
          } /* endif */
          pDoc->Redraw |= REDRAW_ALL;                          // redraw all
	    }
	    else
	    {  // restore old pDataW ptr
			pSeg->pDataW = pTmpBuf;              // P018040: reset if alloc failed
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */
  return(fResult);
} /* end of function EQFBSegSoftLFInsert */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBSoftLFInsert
//------------------------------------------------------------------------------
// Function call:     EQFBSoftLFInsert(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       insert soft LFs in all segments
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     while not at end of file
//                      get segment and segment data
//                      set column where this segment starts
//                      insert softLF in this segment and
//                      return column where this segment ends
//                    endwhile
//                    update screen
//------------------------------------------------------------------------------
VOID  EQFBSoftLFInsert
(
   PTBDOCUMENT  pDoc
)
{
  if ( !pDoc->hwndRichEdit )
  {
   ULONG      ulSegNum;                         //segment number
   PTBSEGMENT pSeg;                                        // ptr to segment
   LONG       lCol = 0;
   PSZ_W      pData;
   LONG       lSegStartCol;
   USHORT     usSegOffOld = 0;
   SHORT      sSign = 1;

   pSeg = NULL;
   ulSegNum = 1;
   lCol = 0;
   while ( (ulSegNum < pDoc->ulMaxSeg) )
   {
     /*****************************************************************/
     /* insert softlf's in   not visible joined segs too              */
     /*****************************************************************/
     pSeg = EQFBGetSegW(pDoc, ulSegNum);
     /*****************************************************************/
     /* if segment is shrinked, handle shrinked blocks                */
     /*****************************************************************/
     if (pSeg && ISQFNOP(pSeg->qStatus) &&
          (    (pDoc->DispStyle == DISP_SHRINK)
            || (pDoc->DispStyle == DISP_COMPACT) )  )
     {
       pSeg = EQFBGetShrinkSeg(pDoc, &ulSegNum, sSign);
     } /* endif */
     if (pSeg )
     {
       lSegStartCol = lCol;
       pData = pSeg->pDataW;
       if ( pData )
       {
         EQFBBufAddSoftLF(pDoc,pSeg, pData, lSegStartCol, &lCol,&usSegOffOld );
       } /* endif */
     } /* endif */
     ulSegNum ++;
   } /* endwhile */

   EQFBCurSegFromCursor(pDoc);
   EQFBScrnLinesFromSeg ( pDoc,                  // pointer to doc ida
                          0,                     // starting row
                          pDoc->lScrnRows,      // number of rows
                                                 // starting segment
                        (pDoc->TBRowOffset+1));
  } /* endif */
  return;
} /* end of function EQFBSoftLFInsert */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBBufAddSoftLF
//------------------------------------------------------------------------------
// Function call:     EQFBBufAddSoftLF(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       insert soft LFs given segment
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    pDoc,
//                    PTBSEGMENT     pSeg,
//                    PSZ            pData,
//                    SHORT          sSegStartCol,
//                    PSHORT         psSegEndCol,
//                    PUSHORT        pusSegOffOld
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     while not at end of segment
//                      get current character
//                      if DBCS und character is 1st of DBCS
//                        if right of right margin: insert SOFTLF
//                        goto next character
//                      else
//                       switch (character)
//                        case LF:  reset column to column 0
//                        case BLANK: if unprotected, remember this position
//                        case SOFTLF_CHAR: duplicate it
//                        default: goon
//                       if right of right margin: insert softLF
//                          if possible
//                       increase data pointer
//                     endif
//                    endwhile
//------------------------------------------------------------------------------

BOOL
EQFBBufAddSoftLF
(
   PTBDOCUMENT    pDoc,
   PTBSEGMENT     pSeg,
   PSZ_W          pData,
   LONG           lSegStartCol,
   PLONG          plSegEndCol,
   PUSHORT        pusSegOffOld
 )
{
  BOOL     fResult = TRUE;

  if ( !pDoc->hwndRichEdit )
  {
    USHORT   usCurOffset = 0;
    LONG     lCol;
    USHORT   usSoftSegOffset = 0;
    BOOL     fSoftLFInserted = FALSE;
    PSZ_W    pDataStart;
    CHAR_W   ch;
    BOOL     fInsert = TRUE;
    USHORT   usOldLen = 0;
    /********************************************************************/
    /* if fNew, usSoftSegOffset is a valid position in a new line and if*/
    /* nec, SoftLF can be set at this position                          */
    /********************************************************************/
    BOOL     fNew = TRUE;
    BOOL     fDuplSoftLF = FALSE;

    pDataStart = pData;
    lCol = lSegStartCol;
    while (*pData && fResult)          // P018040: stop loop if fResult =FALSE
    {
       ch = *pData;
       if (IsDBCS_CP(pDoc->ulOemCodePage) && EQFIsDBCSChar(ch, pDoc->ulOemCodePage) )
       {
         if ((lCol+1) >= pDoc->sRMargin )
         {
           /*************************************************************/
           /* insert SoftLF in front of current DBCS character          */
           /*************************************************************/
           usOldLen = pSeg->usLength;
           fSoftLFInserted = EQFBCheckAndAddSoftLF(pDoc, pSeg,
                                                   &usCurOffset, &fResult);
           /*************************************************************/
           /* set pDataStart and pData, since a new buffer has been     */
           /* allocated in EQFBCheckAndAddSoftLF for pSeg->pData        */
           /*************************************************************/
           if ( fSoftLFInserted )
           {
             pDataStart = pSeg->pDataW;
             pData = pDataStart + usCurOffset;     //1st pos after softlf
           } /* endif */
         } /* endif */
         if (fSoftLFInserted )
         {
           fNew = FALSE;
           if ((usOldLen != pSeg->usLength) && (usCurOffset < *pusSegOffOld ))
           {
             (*pusSegOffOld)++;
           } /* endif */
           lCol = 0;
           fSoftLFInserted = FALSE;
         }
         else
         {
           usSoftSegOffset = usCurOffset;
           fInsert = TRUE;
           fNew = TRUE;
         } /* endif */
         lCol+=2;                            // every DBCS char needs 2 display positions
         usCurOffset++;                      // goto next character
         pData++;
       }
       else
       {
         switch ( ch )
         {
           case LF:
             lCol = 0;
             fNew = FALSE;
             break;
           case BLANK:
             if (EQFBCharType(pDoc, pSeg, usCurOffset) ==
                                UNPROTECTED_CHAR    )
             {
               /*********************************************************/
               /* allow this position for SoftLF only if duplicated     */
               /* softlf is not preceeding this position or following   */
               /* on next position                                      */
               /*********************************************************/
               fDuplSoftLF = FALSE;
               if (usCurOffset > 1 )
               {
                 if ( (*(pData-1) == SOFTLF_CHAR) &&
                      (*(pData-2) == SOFTLF_CHAR))
                 {
                   fDuplSoftLF = TRUE;
                 } /* endif */
               } /* endif */
               if (*(pData + 1) == SOFTLF_CHAR )
               {
                 fDuplSoftLF = TRUE;
               } /* endif */
               if (!fDuplSoftLF )
               {
                 usSoftSegOffset = usCurOffset;
                 fInsert = FALSE;
                 fNew = TRUE;
               } /* endif */
             }
             lCol++;
             break;
           case SOFTLF_CHAR:
             usCurOffset++;                               // insert softLF after
             pData++;                                        // the one found
             usOldLen = pSeg->usLength;
             fResult = EQFBSegSoftLFInsert ( pDoc, pSeg, usCurOffset);
             pDataStart = pSeg->pDataW;                    // point to inserted
             pData = pDataStart + usCurOffset;            // softlf
             if (fResult && (usOldLen != pSeg->usLength)
                         && (usCurOffset < *pusSegOffOld ))
             {
               (*pusSegOffOld)++;
             } /* endif */
             lCol++;
             /***********************************************************/
             /* don't use usSoftSegOffset because duplicated softlf     */
             /* would be duplicated again                               */
             /***********************************************************/
             fNew = FALSE;
             break;
           default:
             if (EQFBCharType(pDoc, pSeg, usCurOffset) != HIDDEN_CHAR)
             {
               lCol++;
             } /* endif */
         } /* endswitch */
         if (((lCol >= pDoc->sRMargin)/* || EQFBRelCurPos( pDoc, sCol, pDoc->sRMargin )*/) && fNew)
         {
           /*************************************************************/
           /* fInsert is FALSE if fNew is TRUE except if at start of seg*/
           /*************************************************************/
           usOldLen = pSeg->usLength;
           fInsert = TRUE;  // NEW: SoftLF always inserted
           fSoftLFInserted = EQFBCheckAndAddSoftLF(pDoc, pSeg,
                                                   &usSoftSegOffset, &fResult);
           if (fSoftLFInserted )
           {
             if ((usOldLen != pSeg->usLength) &&
                  ( usSoftSegOffset < *pusSegOffOld ))
             {
               (*pusSegOffOld)++;
             } /* endif */
             pDataStart = pSeg->pDataW;
             pData = pDataStart + usSoftSegOffset;
             usCurOffset = usSoftSegOffset;         //1st pos after softlf
             fNew = FALSE;
             lCol = 0;
             fSoftLFInserted = FALSE;
           } /* endif */
           usCurOffset ++;
           pData ++;
           lCol ++;
         }
         else
         {
           usCurOffset++;                      // goto next character
           pData++;
         } /* endif */
       } /* endif */
    } /* endwhile */
    if (pDoc->EQFBFlags.PostEdit && pDoc->pUserSettings->fSegBound )
    {
      lCol++;                                   // 1col for Postedit SegEndsign
    } /* endif */
    *plSegEndCol = lCol;
  } /* endif */

  return(fResult);
} /* end of function EQFBBufAddSoftLF                                 */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBCheckAndAddSoftLF
//------------------------------------------------------------------------------
// Function call:     EQFBCheckAndAddSoftLF
//------------------------------------------------------------------------------
// Description:       insert soft LFs at given position
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    pDoc,
//                    PTBSEGMENT     pSeg,
//                    PSZ            pData,
//                    BOOL           fInsert,
//                    PUSHORT        pusCurOffset
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if segment if NOP: add softLF at end of segment
//                    else
//                      if fInsert: insert SOFTLF
//                      else replace BLANK by SOFTLF( take care that no blank
//                         is following the SOFTLF)
//                    endwhile
//------------------------------------------------------------------------------
static BOOL
EQFBCheckAndAddSoftLF
(
    PTBDOCUMENT    pDoc,
    PTBSEGMENT     pSeg,
    PUSHORT        pusCurOffset,
    PBOOL          pfOK
 )
{
  BOOL        fSoftLFInserted = FALSE;
  BOOL        fOK = TRUE;

  if ( !pDoc->hwndRichEdit )
  {
    PTBSEGMENT  pNopSeg;
    USHORT      usLen;
    USHORT      usCurOffset;
    usCurOffset = *pusCurOffset;


    if (ISQFNOP(pSeg->qStatus ) )
    {
      if (pDoc->DispStyle == DISP_COMPACT || pDoc->DispStyle == DISP_SHRINK )
      {
        pNopSeg = EQFBGetSegW(pDoc, pSeg->ulSegNum);
        /**************************************************************/
        /* add softlf at end of NOP                                   */
        /**************************************************************/
        fOK = EQFBSegSoftLFInsert(pDoc, pNopSeg, pNopSeg->usLength);
        fSoftLFInserted = fOK;
      }
      else
      {
        //* if already a LF inside the NOP following the split position, do not add Softlf            
        usLen = pSeg->usLength - 1;
        USHORT usI = usCurOffset;
        while ( (usI <= usLen) && (pSeg->pDataW[usI] != LF) ) usI++;
        if ( usI > usLen )
        {
          fOK = EQFBSegSoftLFInsert(pDoc, pSeg, pSeg->usLength);
          fSoftLFInserted = fOK;
          *pusCurOffset = pSeg->usLength;
        }
        else
        {
          *pusCurOffset = usI;
        } /* endif */
      } /* endif */
      
    }
    else
    {
      if (*(pSeg->pDataW + usCurOffset) == BLANK )
      {
        usCurOffset ++;                       // add softlf AFTER Blank
        fOK = EQFBSegSoftLFInsert(pDoc, pSeg, usCurOffset);
        fSoftLFInserted = fOK;
        usCurOffset ++;
      }
      else
      {
        fOK = EQFBSegSoftLFInsert(pDoc, pSeg, usCurOffset);
        fSoftLFInserted = fOK;
        usCurOffset ++;                              // 1st pos after added softlf
      } /* endif */
      *pusCurOffset = usCurOffset;                   // 1st pos after added softlf
    } /* endif */
  } /* endif */
  //P018040:   fOK FALSE is something went wrong in SegSoftLFInsert and loops should
  // be stopped! (fOK = TRUE is success or nothing done!)
  *pfOK = fOK;
  return(fSoftLFInserted);
} /* end of function EQFBCheckAndAddSoftLF */
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ActTransEnv
//------------------------------------------------------------------------------
// Function call:     fOK = ActTransEnv( pTBDoc );
//------------------------------------------------------------------------------
// Description:       activate the translation environment for the passed
//                    document structure
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT      -- pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE      -- success
//                    FALSE     -- failure (allocation or activation error)
//------------------------------------------------------------------------------
// Function flow:     try to activate passed on object
//                      -- invoke EQF_XDOCACT and run the TM and dict. request
//                    return success
//------------------------------------------------------------------------------
BOOL ActTransEnv( PTBDOCUMENT pDoc )
{
  PSTEQFGEN pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen;
  BOOL   fOK = TRUE;
  PSZ    pDocObj;

  /********************************************************************/
  /* generate object file name and try to activate this object...     */
  /********************************************************************/
  if (UtlAlloc( (PVOID *)&pDocObj, 0L, (LONG) MAX_PATH144, ERROR_STORAGE) )
  {
    PSZ pFileName, pObjName;
    BYTE chTempFileEnd, chTempObjEnd;

    chTempObjEnd = EOS;                  // init;
    pFileName = UtlGetFnameFromPath( pDoc->szDocName );
    if (pFileName)
    {
      PTBDOCUMENT pTempDoc;
      chTempFileEnd = *(pFileName-1);
      *(pFileName-1) = EOS;

      pObjName = UtlGetFnameFromPath( pDoc->szDocName );
      if (pObjName)
      {
        chTempObjEnd = *pObjName;
        *pObjName = EOS;
      } /* endif */
      sprintf( pDocObj, "%s%s", pDoc->szDocName, pFileName );
      if ( EQF_XDOCACT( pstEQFGen, pDocObj ) == 0 )
      {
        PTBDOCUMENT *ppDoc = (PTBDOCUMENT *) &(pstEQFGen->ucbUserArea[0]);
        pTempDoc = pDoc;
        do
        {
          pTempDoc->fTransEnvAct = FALSE;
          pTempDoc = pTempDoc->next;
        } while ( pTempDoc != pDoc ); /* enddo */
        pDoc->fTransEnvAct = TRUE;
        *ppDoc = pDoc;                // anchor our new document...
        ((PSTEQFGEN)pDoc->pstEQFGen)->hwndEditorTgt = pDoc->hwndClient;
        // activate document window
        WinShowWindow( pDoc->hwndFrame, TRUE );

        PostMessage(((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS, WM_EQF_SETFOCUS,
                      0,
                      MP2FROMHWND(((PSTEQFGEN)pDoc->pstEQFGen)->hwndEditorTgt));

        *(pFileName-1) = chTempFileEnd;
        *pObjName      = chTempObjEnd;
        /**************************************************************/
        /* reload active TRNote                                       */
        /**************************************************************/
        {
          PTBDOCUMENT pDocCurrent = pDoc->next;
          while ((pDocCurrent->docType != TRNOTE_DOC) && (pDocCurrent != pDoc ))
          {
            pDocCurrent = pDocCurrent->next;
          } /* endwhile */
          if (pDocCurrent->docType == TRNOTE_DOC)
          {
            if ( pDocCurrent->hwndFrame &&
                 WinIsWindowVisible(pDocCurrent->hwndFrame) )
            {
              EQFBRemoveDoc ( pDocCurrent );
              EQFBFuncOpenTRNote( pDoc );
            }
            else
            {
              EQFBRemoveDoc ( pDocCurrent );
            } /* endif */
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* activate MT logging (using the segmented source file name  */
        /**************************************************************/
        if ( pDoc && pDoc->twin )
        {
          PSZ pszMarkup = "";
          PLOADEDTABLE pTable = (PLOADEDTABLE)pDoc->twin->pDocTagTable;
          if ( pTable )
          {
            pszMarkup = pTable->szName;
          } /* endif */
          ActivateMTLog( pDoc->twin->szDocName, pszMarkup );
        } /* endif */
        /****************************************************************/
        /* reload the active segment...                                 */
        /****************************************************************/
        if ( !pDoc->EQFBFlags.PostEdit )
        {
          ULONG  ulSegNum = pDoc->tbActSeg.ulSegNum;
          PTBSEGMENT pTBSourceSeg = EQFBGetVisSeg(pDoc->twin, &ulSegNum);
          USHORT usMatchFound;
          if ( pTBSourceSeg )
          {
             PSZ_W pszContext = EQFBGetContext( pDoc, pTBSourceSeg, pTBSourceSeg->ulSegNum );
             fOK = !EQFTRANSSEG3W( pTBSourceSeg->pDataW,     // pointer to seg data
                                   pszContext, pTBSourceSeg->pvMetadata,
                                    pTBSourceSeg->ulSegNum,  // segment number
                                    TRUE,                    // mode of operation
                                    FALSE,                   // no automatic mode
                                    (PSHORT)&usMatchFound);
          } /* endif */
        }
        else
        {
          EQFCLEAR (EQFF_NOPROPWND | EQFF_NODICTWND | EQFF_NOSEGPROPWND );
        } /* endif */
      }
      else
      {
        fOK = FALSE;
        UtlError( TB_ACTNEWTENV, MB_CANCEL, 0, NULL, EQF_ERROR);
      } /* endif */
    }
    else
    {
      fOK = FALSE;
      UtlError( TB_ACTNEWTENV, MB_CANCEL, 0, NULL, EQF_ERROR);
    } /* endif */
    UtlAlloc( (PVOID *)&pDocObj, 0L, 0L, NOMSG );
  }
  else
  {
    fOK = FALSE;
  } /* endif */
  return fOK;

}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBTenvStart
//------------------------------------------------------------------------------
// Function call:     usRc = EQFBTenvStart(pTBDoc, pNewDoc, pstEQFGen);
//------------------------------------------------------------------------------
// Description:       start the selected translation environment
//                    Note: pTBDoc may be NULL (last active doc removed)
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT     -- ptr to loaded document struct
//                    PSZ             -- ptr to newly selected document
//                    PSTEQFGEN       -- generic structure
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       0      --  success
//                    TM/2 rc
//------------------------------------------------------------------------------
// Function flow:     check if selected document already part of ring
//                    if so  -- activate it only (EQF_XDOCACT)
//                      else -- invoke EQFBFuncStart
//                    activate the Translation editor window
//                    return success
//------------------------------------------------------------------------------
USHORT
EQFBTenvStart
(
  PTBDOCUMENT pDoc,
  PSZ         pOutDocObjName,
  PSTEQFGEN   pstEQFGen
)
{
  BOOL   fFound = FALSE;               // document already in ring
  USHORT usRc = 0;                     // success indicator
  PSZ pFileName;                       // pointer to file name
  PSZ pFolder;                         // pointer to folder
  PTBDOCUMENT pDocStart = pDoc;        // document pointer
  PSZ pSegTarget = NULL, pSegSource, pTempDocObj;

  if ( UtlAlloc( (PVOID *)&pSegTarget, 0L, 3L*MAX_LONGPATH, NOMSG ) )
  {
    pSegSource = pSegTarget+ MAX_LONGPATH;
    pTempDocObj = pSegTarget+ 2* MAX_LONGPATH;

    strcpy( pTempDocObj, pOutDocObjName );
    pFileName = UtlSplitFnameFromPath( pTempDocObj );
    pFolder = UtlGetFnameFromPath( pTempDocObj );

    /********************************************************/
    /* create segmented target path...                      */
    /********************************************************/
    if (pFileName && pFolder )
    {
      UtlMakeEQFPath( pSegTarget, pTempDocObj[0],
                      DIRSEGTARGETDOC_PATH, pFolder );
      strcat( pSegTarget, BACKSLASH_STR );
      strcat( pSegTarget, pFileName );
      if ( pDoc )
      {
        do
        {
          if (pDoc->docType == STARGET_DOC)
          {
            fFound = (strcmp( pSegTarget, pDoc->szDocName ) == 0);
          } /* endif */
          if ( !fFound )
          {
            pDoc = pDoc->next;
          } /* endif */
        } while ( (pDoc != pDocStart) && !fFound ); /* enddo */
      } /* endif */

      usRc = EQF_XDOCACT( pstEQFGen, pOutDocObjName );
      if ( usRc )
      {
        /**************************************************************/
        /* activate the one we started with ...                       */
        /**************************************************************/
        EQF_XDOCREMOVE( pstEQFGen, pOutDocObjName );
        if ( pDoc )
        {
          pDocStart = pDoc;
          fFound = FALSE;
          do
          {
            if (pDoc->twin)
            {
              fFound = TRUE;
            }
            else
            {
              pDoc = pDoc->next;
            } /* endif */
          } while ( (pDoc != pDocStart) && !fFound ); /* enddo */

          ActTransEnv( pDoc );
        } /* endif */
      } /* endif */
    }
    else
    {
      usRc = ERROR_STORAGE;
    } /* endif */
    if ( !usRc && !fFound )
    {
      /******************************************************/
      /* load document                                      */
      /******************************************************/
      UtlMakeEQFPath( pSegSource, pTempDocObj[0],
                      DIRSEGSOURCEDOC_PATH, pFolder );
      strcat( pSegSource, BACKSLASH_STR );
      strcat( pSegSource, pFileName );
      /**************************************************************/
      /* get tagtable for this document                             */
      /**************************************************************/
      DocQueryInfo2( pOutDocObjName,        // document object name
                     NULL,                  // document translation memory
                     (PSZ)pstEQFGen->szTagTable, // format of document
                     NULL,                  // document source language
                     NULL,                  // document target language
                     NULL,                  // long name of document
                     NULL,                  // document alias name
                     NULL,                  // document editor
                     TRUE );

      usRc = (USHORT) !EQFBFuncStart( pSegSource, pSegTarget,
                             pstEQFGen,
                             TRUE, pDoc );
      if ( !usRc && !pDoc )
      {
        PTBDOCUMENT *ppDoc = (PTBDOCUMENT *) &(pstEQFGen->ucbUserArea[0]);
        pDoc = pDocStart = *ppDoc;
      } /* endif */

      if ( !usRc )
      {
        UtlDispatch();   // ensure that posted messages are worked on...
        fFound = FALSE;
        do
        {
          if (pDoc->docType == STARGET_DOC)
          {
            fFound = (strcmp( pSegTarget, pDoc->szDocName ) == 0);
          } /* endif */
          if ( !fFound )
          {
            pDoc = pDoc->next;
          } /* endif */
        } while ( (pDoc != pDocStart) && !fFound ); /* enddo */
      } /* endif */
    } /* endif */
    /********************************************************/
    /* user wants to go to this document                    */
    /********************************************************/
    if ( !usRc)
    {
      pDoc->Redraw |= REDRAW_ALL;
      // activate document window
      WinShowWindow( pDoc->hwndFrame, TRUE );

      PostMessage(((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS, WM_EQF_SETFOCUS,
                   0, MP2FROMHWND(pDoc->hwndFrame));
    } /* endif */
    UtlAlloc( (PVOID *)&pSegTarget, 0L, 0L, NOMSG );
  }
  else
  {
    usRc = ERROR_STORAGE;
  } /* endif */
  return usRc;
} /* end of function EQFBTenvStart */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ActivateMTLog
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       _
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     check for existance of EQFMTLOG file in the prop.direct.
//                    if exist open file in the STARGET dir.
//                    close any previously opened MTLOG files
//  Modification by GQ: use the new MTLOG directory for the log file and
//                      keep the original short name
//------------------------------------------------------------------------------

VOID ActivateMTLog( PSZ pDocName, PSZ pszTagTable )
{
  CHAR  chFile[MAX_EQF_PATH];
  CHAR  szFolShortName[MAX_FILESPEC];

  // check for MT logging trigger file

  // GQ: always write MT log info..

  //UtlMakeEQFPath ( chFile, NULC, PROPERTY_PATH, NULL );
  //strcat( chFile, BACKSLASH_STR );
  //strcat( chFile, "EQFMTLOG" );
  //if ( UtlFileExist( chFile ))

  if ( UtlQueryUShort( QS_MTLOGGING ) != 0 )
  {
    BOOL fLogExists = FALSE;
    FILE* hMTLog = get_hMTLog();
	
    // extract folder short name from given document name
    strcpy( chFile, pDocName );
    UtlSplitFnameFromPath( chFile );             // remove document name
    UtlSplitFnameFromPath( chFile );             // remove document path (SSOURCE)
    strcpy( szFolShortName, UtlGetFnameFromPath( chFile ) ); // save folder short name

    // get path to MTLOG directory
    UtlMakeEQFPath ( chFile, pDocName[0], MTLOG_PATH, szFolShortName );

    // create log directory if it does not exists
    if ( !UtlDirExist( chFile ) )
    {
      UtlMkDir( chFile, 0L, FALSE );
    } /* endif */

    // setup complete log file name
    strcat( chFile, BACKSLASH_STR );
    strcat( chFile, UtlGetFnameFromPath( pDocName ) );

    // close any open log file
    if ( hMTLog ) fclose( hMTLog );

    fLogExists = UtlFileExist( chFile );

    hMTLog = fopen( chFile, "ab" );
    set_hMTLog( hMTLog );

    // write dummy record (segment number 0) containing markup table name to log file
    // if log file is new
    if ( hMTLog && !fLogExists && pszTagTable && (*pszTagTable != EOS) )
    {
      ACTSEGLOGOLD SegLog;

      // we store the markup table right behind the segment number within the seg log record
      // the size of the record is large enough to contain a markup table name (up to 8 chars)
      PSZ       pszMarkup = (PSZ)&SegLog + sizeof(ULONG);

      memset( &SegLog, 0, sizeof(SegLog) );
      strcpy( pszMarkup, pszTagTable );
      fwrite( &SegLog, sizeof(SegLog), 1, hMTLog );
    } /* endif */
  }
} /* end of function ActivateMTLog */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBGetFontPtrs                                          |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBGetFontPtrs                                          |
//+----------------------------------------------------------------------------+
//|Description:       get a pointer to font specs                              |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     set the pointer to the user options ....                 |
//+----------------------------------------------------------------------------+
COLORREF * EQFBGetColorRGB ( )
{
  return (&COLORRGBTABLE[0]);
} /* end of function EQFBGetColorRGB     */

PUSHORT EQFBGetFontTypesInGroup ( )
{
  return (&usFontTypesInGroup[0]);
} /* end of function EQFBGetFontTypesInGroup     */

CHARFORMAT2 * EQFBGetFontSpecs()
{
  return (get_aszFontExtSpecs());
} /* end of function EQFBGetFontSpecs            */

//get default font specs

CHARFORMAT2 * EQFBGetDefFontSpecs()
{
  /********************************************************************/
  /* build complete CHARFORMAT2 with default settings                 */
  /********************************************************************/
  return (get_aszFontExtSpecs());
} /* end of function EQFBGetDefFontSpecs            */


VOID EQFBSetCharFormat2
(
   CHARFORMAT2 *pCharFormat,
   PSZ          pszFaceName,
   BYTE         bCharSet,
   BYTE         bPitchAndFamily,
   LONG         lHeight,
   SHORT        sStyle,
   COLORREF     crTextColor,
   COLORREF     crBackColor,
   SHORT        sUnderline,
   SHORT        sStrikeOut
)
{
  memset(pCharFormat, 0, sizeof(CHARFORMAT2));
  pCharFormat->dwMask = (DWORD)0;
  pCharFormat->dwEffects = (DWORD) 0;
  pCharFormat->cbSize = sizeof(CHARFORMAT2);
  if (pszFaceName )
  {
    strcpy(pCharFormat->szFaceName, pszFaceName);
    pCharFormat->bCharSet = bCharSet;
    pCharFormat->bPitchAndFamily = bPitchAndFamily;
  } /* endif */
  if (lHeight )
  {
    pCharFormat->yHeight = lHeight;
  } /* endif */
  if (sStyle )
  {
    switch ( sStyle )
    {
      case STYLE_STANDARD:
        //standard
        break;
      case STYLE_ITALIC:   //italic
        pCharFormat->dwEffects |= CFE_ITALIC;
        break;
      case STYLE_BOLD: //bold
        pCharFormat->dwEffects |= CFE_BOLD;
        break;
      case STYLE_ITALBOLD: // bold & italic
        pCharFormat->dwEffects |= CFE_ITALIC | CFE_BOLD;
        break;
    } /* endswitch */
  } /* endif */
  if (crTextColor )
  {
    pCharFormat->crTextColor = crTextColor;
  } /* endif */
  if (crBackColor )
  {
    pCharFormat->crBackColor = crBackColor;
  } /* endif */
  if (sUnderline )
  {
    switch ( sUnderline )
    {
      case UNDERLINE_NOSPEC:
        //not specified
        break;
      case UNDERLINE_NONE:   //no underline
        pCharFormat->bUnderlineType |= CFU_UNDERLINENONE;
        break;
      case UNDERLINE_YES: //underline
        pCharFormat->bUnderlineType |= CFU_UNDERLINE;
        pCharFormat->dwEffects |= CFE_UNDERLINE;
        break;
    } /* endswitch */
  } /* endif */
  if (sStrikeOut == STRIKEOUT )
  {
    pCharFormat->dwEffects |= CFE_STRIKEOUT;
  } /* endif */
  pCharFormat->dwMask |= CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_ITALIC | CFM_BOLD;
  pCharFormat->dwMask |= CFM_COLOR | CFM_BACKCOLOR | CFM_UNDERLINETYPE | CFM_STRIKEOUT;

  return;
} /* end of function EQFBSetCharFormat2          */


VOID EQFBResetAllFonts
(
   PTBDOCUMENT  pDoc
)
{
  PTBDOCUMENT  pDocStart;             // ptr to current pdoc
  pDocStart = pDoc ;                     // pointer to active document

  // scan through all documents and issue a reset together with a
  // forced redraw of the screen
  do
  {
    if ( pDoc->hwndRichEdit )
    {
      WinPostMsg( pDoc->hwndClient, WM_EQF_FONTCHANGED, 0L, 0L );
    }
    else
    {
      pDoc->Redraw |= REDRAW_ALL;
      EQFBRefreshScreen( pDoc );  // refresh the screen
    } /* endif */
    pDoc = pDoc->next;
  } while ( pDoc != pDocStart  ); /* enddo */
  EQFBWriteProfile(pDoc);

  if ( pDoc->pstEQFGen )
  {
    PSTEQFGEN     pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
    PDOCUMENT_IDA pIda      = (PDOCUMENT_IDA) pstEQFGen->pDoc;
    SendMessage(pIda->tbDevDictionary.tbDoc.hwndClient, WM_EQF_FONTCHANGED,0L,0L);
    SendMessage(pIda->tbDevProposal.tbDoc.hwndClient, WM_EQF_FONTCHANGED,0L,0L);
    SendMessage(pIda->tbDevSource.tbDoc.hwndClient, WM_EQF_FONTCHANGED,0L,0L);
  } /* endif */
  return;
}

// EQFBGetContext
//
// returns the context information for this segment, if none has been
// stored yet, the EQFGETSEGCONTEXT user exit function is called to
// retrieve the segment context information
PSZ_W EQFBGetContext
(
  PTBDOCUMENT pDoc,
  PTBSEGMENT pSeg,
  ULONG ulSegNum
)
{
  PSZ_W pszContext = NULL;

  if ( pSeg->pContext )
  {
    // return existing context for this segment
    pszContext = pSeg->pContext;
  }
  else if ( pSeg->SegFlags.NoContextAvailable )
  {
    // nothing to do: context had been requested for this segment, but no context is avaialble
  }
  else if ( pDoc->pfnGetSegContext != NULL )
  {
    BOOL   fOK = TRUE;
    PSZ_W  pPrevSegData = NULL;
    PSZ_W  pNextSegData = NULL;

    // allocate context buffer if not done yet
    if ( !pDoc->pContext )
    {
      fOK = UtlAlloc( (PVOID *)&(pDoc->pContext), 0L,
                      (MAX_SEGMENT_SIZE * sizeof(CHAR_W)) + 10, NOMSG );
    } /* endif */

    // clear context buffer
    if ( fOK )
    {
      memset( pDoc->pContext, 0, MAX_SEGMENT_SIZE * sizeof(CHAR_W) );
    } /* endif */

    // get pointers to previous and next segment
    if ( fOK )
    {
      PTBSEGMENT pTBSeg;

      pTBSeg = EQFBGetSegW( pDoc, (ULONG)(ulSegNum-1) );
      pPrevSegData = (pTBSeg) ? pTBSeg->pDataW : NULL;

      pTBSeg = EQFBGetSegW( pDoc, (ULONG)(ulSegNum+1) );
      pNextSegData = (pTBSeg) ? pTBSeg->pDataW : NULL;
    } /* endif */

    // call user exit function to supply the context information
    if ( fOK )
    {
      pDoc->ulContextSeg = ulSegNum;
      (pDoc->pfnGetSegContext)( pSeg->pDataW, pPrevSegData, pNextSegData, pDoc->pContext, (LONG)pDoc, ulSegNum );
      if ( *(pDoc->pContext) == 0 )
      {
        fOK = FALSE;                   // no further processing required
        pSeg->SegFlags.NoContextAvailable = TRUE;
      } /* endif */
    } /* endif */

    // allocate segment context buffer
    if ( fOK )
    {
      int iLen = UTF16strlenCHAR( pDoc->pContext ) + 1;
      fOK = UtlAlloc( (PVOID *)&(pSeg->pContext), 0L, (iLen * sizeof(CHAR_W)), NOMSG );
    } /* endif */

    // store context in segment context buffer
    if ( fOK )
    {
      UTF16strcpy( pSeg->pContext, pDoc->pContext );
    } /* endif */

    // set returned context pointer
    if ( fOK )
    {
      pszContext = pSeg->pContext;
    } /* endif */
  } /* endif */

  return( pszContext );
} /* end of function EQFBGetContext */


VOID EQFBFuncLRM
(
   PTBDOCUMENT  pDoc
)
{
  /// simulate a WM_CHAR message with LRM marker ( i.e. Unicode 200E )
  // input character at current cursor position
  // check WIn 2000 and NOn-Unicode systems!!

  pDoc->usChar = 0x200E;
  EQFBFuncCharacter(pDoc);

  return;

}

VOID EQFBFuncRLM
(
   PTBDOCUMENT  pDoc
)
{
  /// send a WM_CHAR message with RLM marker ( i.e. Unicode 200F )
  //

  pDoc->usChar = 0x200F;
  EQFBFuncCharacter(pDoc);

  return;
}
// MT log helper function: handle start of edit actions
VOID MTLogStartEditing( PTBDOCUMENT pDoc )
{
  ULONG ulCurrent = GetTickCount();

  if ( pDoc->ActSegLog.ulChoiceTime == 0 )  // no user activity yet
  {
    // stop choice time
    pDoc->ActSegLog.ulChoiceTime = ulCurrent - pDoc->ActSegLog.ulTotalTime;
  } /* endif */            

  if ( pDoc->ActSegLog.AddFlags.ComputeThinkTime )  // user think time not computed yet
  {
    // compute think time 
    pDoc->ActSegLog.ulThinkTime = ulCurrent - pDoc->ActSegLog.ulTime;
    pDoc->ActSegLog.AddFlags.ComputeThinkTime = FALSE;
  } /* endif */            

}

// MT log helper function: handle copy of a proposal
VOID MTLogProposalCopied( PTBDOCUMENT pDoc )
{
  ULONG ulCurrent = GetTickCount();

  // choice time ends here
  if ( pDoc->ActSegLog.ulChoiceTime == 0 )  // no user activity yet
  {
    pDoc->ActSegLog.ulChoiceTime = ulCurrent - pDoc->ActSegLog.ulTotalTime;
  } /* endif */            

  // (re)start processing time
  pDoc->ActSegLog.ulTime = ulCurrent;

  // (re)set "think time" computation flag
  pDoc->ActSegLog.AddFlags.ComputeThinkTime = TRUE;

}

// MT log helper function: undo the copy of a proposal
VOID MTLogUndoProposalCopy( PTBDOCUMENT pDoc )
{
  // (re)set "think time" computation flag
  pDoc->ActSegLog.AddFlags.ComputeThinkTime = FALSE;
}

//-----------------------------------------------------------------------------
//                                                                            
//      Profile routines                                                      
//                                                                            
//-----------------------------------------------------------------------------

// layout of profile file:
//  +------ block 1 ------+
//    USHORT usSize
//    USHORT usType
//    BYTE bData[usSize]
//  +---------------------+
//  +------ block 2 ------+
//    USHORT usSize
//    USHORT usType
//    BYTE bData[usSize]
//  +---------------------+
//        ....
//  +------ block n ------+
//    USHORT usSize
//    USHORT usType
//    BYTE bData[usSize]
//  +---------------------+
//        ....
//  +----- last block ----+
//    USHORT usSize = 0
//    USHORT usType =
//                PROF_EOF
//  +---------------------+

#define PROF_CURVERSION  3             // profile version stored in profile

// types of profile data
typedef enum _PROFDATATYPE
{
   PROF_KEYS = 1,                      // key assignment information
   PROF_FONTCOLS,                      // font/color information
   PROF_LU_OPEN,                       // last used values of ppen dialog
   PROF_LU_RIGHTMARGIN,                // last used values for right margin
   PROF_LU_FONTSIZE,                   // last used values for font size
   PROF_IN_SHRNKSTRINGS,               // last used shrink string for inline
   PROF_OUT_SHRNKSTRINGS,              // last used shrink string for outline
   PROF_SET_USEROPTS,                  // last used user options
   PROF_VERSION,                       // version of profile
   PROF_LU_FONTFACENAME,               // last used values for font facenames
   PROF_LU_TRNOTESTRING,               // version2: trnote string abbreviation
   PROF_EXTFONTS,                      // extended non-monospaced fonts
   PROF_EOF = 999                      // end of profile file marker
} PROFDATATYPE;

// profile value-supplied flags
BOOL  afValue[PROF_EOF];               // TRUE = value has been supplied

// profile block header
typedef struct _PROFILEHEAD
{
   USHORT usType;                      // current profile data type
   USHORT usSize;                      // size of current profile data
} PROFILEHEAD, *PPROFILEHEAD;

/**********************************************************************/
/* Default fonts to be used in extended font dialog                   */
/**********************************************************************/
static byte FontExt2[] = {
0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x22,0x41,0x72,0x69,0x61,
0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,
0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,
0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,
0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,
0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,
0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x04,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x01,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x02,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x22,0x41,0x72,0x69,0x61,
0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,
0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,
0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,0xc0,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,
0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xc0,0xc0,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,
0x80,0xec,0x08,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,
0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,
0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,0xc0,0x00,
0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,
0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,
0x80,0xec,0x08,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,
0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,
0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,
0x00,0x22,0x41,0x72,0x69,0x61,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x54,0x00,0x00,0x00,0x0b,0x00,0x80,0xec,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x22,0x41,0x72,0x69,0x61,
0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


SHORT EQFBQueryProfName( PSZ pszProfName );
BOOL   EQFBIsProfileOK ( VOID );
VOID   EQFBCheckEditor ( VOID );
VOID  EQFBCheckAssignedResKey( VOID );
// Add for R012027 start
void OtmWriteSCKeyProfile();
void OtmReadSCKeyProfile();
SHORT OtmQuerySCKeyProfName(PSZ pszProfName);
// Add end

// help  function to add missing functions to the keytable
void EQFBAddFunction
(
  USHORT        newFunction,           // function to add to key table
  PKEYPROFTABLE pKeyTable,             // key table being modified
  PKEYPROFTABLE pDefKeyTable           // default key table (source for new key table entry)
)
{
  PKEYPROFTABLE pKey = NULL;
  PKEYPROFTABLE pDefKey = NULL;

  // check if table contains function already
  pKey = pKeyTable;
  while ( (pKey->Function != LAST_FUNC) && (pKey->Function != newFunction) ) pKey++;

  // add function if not contained in list
  if ( pKey->Function != newFunction )
  {
    // find corresponding entry in default table
    pDefKey = pDefKeyTable;
    while ( (pDefKey->Function != LAST_FUNC) && (pDefKey->Function != newFunction) ) pDefKey++;

    // find empty entry in table
    pKey = pKeyTable;
    while ( (pKey->Function != LAST_FUNC) && (pKey->Function != 0) ) pKey++;

    // if default entry exists...
    if ( pDefKey && (pKey->Function == 0) )
    {
      // insert new entry into the table
      memcpy( pKey, pDefKey, sizeof(KEYPROFTABLE) );
    } /* endif */
  } /* endif */
} /* end of function EQFBAddFunction */




//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBReadProfile
//-----------------------------------------------------------------------------
// Function call:     EQFBReadProfile()
//-----------------------------------------------------------------------------
// Description:       Load profile data
//-----------------------------------------------------------------------------
// Parameters:        none
//-----------------------------------------------------------------------------
// Returncode type:   USHORT usRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_OPENFILE   - error opening file
//                    ERR_READFILE   - error reading file
//                    ERR_NOMEMORY   - memory allocation error
//-----------------------------------------------------------------------------
// Side effects:      All profile related data areas are filled
//-----------------------------------------------------------------------------
// Function flow:     -read profile file and fill related storage areas
//                    -do default initialization for all storage areas
//                        which are not filled yet
//-----------------------------------------------------------------------------

SHORT EQFBReadProfile( VOID )
{
   CHAR        szProfName[MAX_EQF_PATH]; // buffer for profile file name
   PVOID       pData = NULL;           // ptr to current profile data
   PROFILEHEAD ProfHead;               // profile block header
   SHORT       sRC = 0;                // function return code
   USHORT      usOpenAction;           // action performed by DosOpen
   USHORT      usDosRC;                // return code of DosXXX calls
   ULONG       ulBytesRead;            // # of bytes read from file
   HFILE       hProfile = (HFILE) NULL;// handle of profile file
   USHORT      usCurSize = 0;          // current size of profile data buffer
   BOOL            fOK;                // ok flag set by UtlAlloc
   VIOFONTCELLSIZE  vioDefFontCellSize;// default cell size
   PVIOFONTCELLSIZE  pvioFontSize;     // pointer to font size arrays
   USHORT      i;                      // index
   CHAR        chFaceName[LF_FACESIZE];      // font name
   USHORT      usProfVersion = 0;          // version of profile
   CHARFORMAT2* paszFontExtSpecs;
   USHORT*      pusRightMargin = get_usRightMargin();
   OPENDATA*    pOpenData = get_OpenData();
   TEXTTYPETABLE* pDefTextTypeTable = get_DefTextTypeTable();
   TEXTTYPETABLE* pTextTypeTable = get_TextTypeTable();
   KEYPROFTABLE* pDefKeyTable = get_DefKeyTable();
   KEYPROFTABLE* pKeyTable = get_KeyTable();
   CHAR* paszFontFacesGlobal = get_aszFontFacesGlobal();
   USEROPT* pEQFBUserOpt = get_EQFBUserOpt();

   // no profile values processed yet
   memset( afValue, NULC, sizeof(afValue) );

   sRC = EQFBQueryProfName( szProfName );        // get the profile name

   // try to open profile file
   if ( !sRC )
   {
      usDosRC = UtlOpen( szProfName,
                         &hProfile,
                         &usOpenAction, 0L,
                         FILE_NORMAL,
                         FILE_OPEN,
                         OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                         0L,
                         FALSE );
      if ( usDosRC )
      {
         hProfile = (HFILE) NULL;
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* init user option settings                                       */
   /*******************************************************************/
   memset(pEQFBUserOpt, NULC, sizeof( USEROPT ));
   memset( paszFontFacesGlobal, NULC, MAX_DIF_DOC * LF_FACESIZE );

   // read and process profile data until end of profile reached
   BOOL bReadKeys = FALSE;                         // Add for R012027
   ProfHead.usType = 0;
   while ( !sRC && hProfile && (ProfHead.usType != PROF_EOF) )
   {
      // read profile block header
      usDosRC = UtlReadL( hProfile,
                         &ProfHead,
                         sizeof(ProfHead),
                         &ulBytesRead,
                         FALSE );
      sRC = ( usDosRC ) ? ERR_READFILE : 0;

      // get / check buffer for profile data
      if ( !sRC && (ProfHead.usType != PROF_EOF) )
      {
         if ( ProfHead.usSize > usCurSize )
         {
            fOK = UtlAlloc((PVOID *) &pData,
                            (LONG) usCurSize,
                            (LONG) max( ProfHead.usSize, MIN_ALLOC ),
                            ERROR_STORAGE );
            if ( fOK )
            {
               usCurSize = max( ProfHead.usSize, MIN_ALLOC );
            }
            else
            {
               sRC = ERR_NOMEMORY;
            } /* endif */
         } /* endif */
      } /* endif */

      // read profile block data
      if ( !sRC && (ProfHead.usType != PROF_EOF) )
      {
         usDosRC = UtlReadL( hProfile,
                            pData,
                            ProfHead.usSize,
                            &ulBytesRead,
                            FALSE );
         sRC = ( usDosRC ) ? ERR_READFILE : 0;
      } /* endif */

      // process profile block
      if ( !sRC && (ProfHead.usType != PROF_EOF) )
      {
         switch ( ProfHead.usType )
         {

            case PROF_VERSION:
               usProfVersion = (USHORT) *((PUSHORT)pData);
               afValue[ProfHead.usType] = TRUE;
               break;

            case PROF_KEYS:            // key assignment information
               bReadKeys = TRUE;                    // Add for R012027
               memcpy( pKeyTable, pData, min( ProfHead.usSize, sizeof(KEYPROFTABLE) * (LAST_FUNC+1)) );

               // older tables will have a wrong end indicator which is now a valid function code
               // so we change the function code of the last entry in the table to LAST_FUNC to ensure that
               // the entry is not misinterpreted as a valid entry
               {
                 int iLastEntry = (ProfHead.usSize / sizeof(KEYPROFTABLE)) - 1;
                 pKeyTable[iLastEntry].Function = LAST_FUNC;
               }

               // ensure that table has an end element
               pKeyTable = get_KeyTable();
               (pKeyTable+LAST_FUNC)->Function = LAST_FUNC;

               // add any missing new functions to the key table to allow key assignment
               EQFBAddFunction( CFIND_FUNC, pKeyTable, pDefKeyTable );
               EQFBAddFunction( SEGPROP_FUNC, pKeyTable, pDefKeyTable );
               EQFBAddFunction( DICTPFXDOWN_FUNC, pKeyTable, pDefKeyTable );
               EQFBAddFunction( DICTPFXUP_FUNC, pKeyTable, pDefKeyTable );
               EQFBAddFunction( TSEGNEXT_EXACT_FUNC, pKeyTable, pDefKeyTable );
               EQFBAddFunction( TSEGNEXT_FUZZY_FUNC, pKeyTable, pDefKeyTable );
               EQFBAddFunction( TSEGNEXT_NONE_FUNC, pKeyTable, pDefKeyTable );
               EQFBAddFunction( TSEGNEXT_MT_FUNC, pKeyTable, pDefKeyTable );
               EQFBAddFunction( TSEGNEXT_GLOBAL_FUNC, pKeyTable, pDefKeyTable );

               afValue[ProfHead.usType] = TRUE;
               break;

            case PROF_FONTCOLS:        // font/color information
               memcpy( pTextTypeTable, pDefTextTypeTable, sizeof(TEXTTYPETABLE) * MAXCOLOUR);
               memcpy(
                 pTextTypeTable, pData,
                       min( ProfHead.usSize, sizeof(TEXTTYPETABLE) * MAXCOLOUR) );
               afValue[ProfHead.usType] = TRUE;
               break;

            case PROF_LU_OPEN:         // last used values of open dialog
               memset( pOpenData, NULC, sizeof(OPENDATA) );
               memcpy( pOpenData, pData,
                       min( ProfHead.usSize, sizeof(OPENDATA) ) );
               afValue[ProfHead.usType] = TRUE;
               break;

            case PROF_LU_RIGHTMARGIN:  // last used values of right margins
               *pusRightMargin = (USHORT) *((PUSHORT)pData);
               afValue[ProfHead.usType] = TRUE;
               break;

            case PROF_LU_FONTSIZE:     // last used values for font sizes
			   pvioFontSize = get_vioFontSize();
               memset( pvioFontSize, NULC, sizeof(VIOFONTCELLSIZE) * MAX_DIF_DOC );
               memcpy( pvioFontSize, pData,
                       min( ProfHead.usSize, sizeof(VIOFONTCELLSIZE) * MAX_DIF_DOC) );

               EQFBGetDefCellSize( &vioDefFontCellSize, chFaceName );

               // if something went wrong use default one
               for ( i=0; i<MAX_DIF_DOC ;i++)
               {
                  if ( pvioFontSize->cx == 0 || pvioFontSize->cy == 0
                       || pvioFontSize->cx >= 50 || pvioFontSize->cy >= 50 )
                  {
                     pvioFontSize->cx = vioDefFontCellSize.cx;
                     pvioFontSize->cy = vioDefFontCellSize.cy;
                  } /* endif */
                  pvioFontSize++;           // point to next candidate
               } /* endfor */
               afValue[ProfHead.usType] = TRUE;
               break;
            case  PROF_IN_SHRNKSTRINGS:
               memcpy( pEQFBUserOpt->szInTagAbbr, pData, MAXSHRKLEN+1);
               afValue[ProfHead.usType] = TRUE;
               break;
            case  PROF_OUT_SHRNKSTRINGS:
               memcpy( pEQFBUserOpt->szOutTagAbbr, pData, MAXSHRKLEN+1);
               afValue[ProfHead.usType] = TRUE;
               break;
            case  PROF_SET_USEROPTS:
               memcpy( pEQFBUserOpt, pData, sizeof( USEROPT));
               afValue[ProfHead.usType] = TRUE;
               /*******************************************************/
               /* do some consistency checking                        */
               /*******************************************************/
               if ( !pEQFBUserOpt->sRMargin  )
               {
                 pEQFBUserOpt->sRMargin = RMARGIN_DEFAULT;
               } /* endif */
               if ( !pEQFBUserOpt->sFocusLine )
               {
                 pEQFBUserOpt->sFocusLine = FOCUSLINE;
               } /* endif */
               pEQFBUserOpt->fNoCUASel = 0;
               break;

            case PROF_LU_FONTFACENAME:     // last used values for fontfacenames
               memcpy( paszFontFacesGlobal, pData,
                       min( ProfHead.usSize, MAX_DIF_DOC * LF_FACESIZE ) );
               afValue[ProfHead.usType] = TRUE;
               EQFBGetDefCellSize( &vioDefFontCellSize, chFaceName );
               // if something went wrong use default one
               /*******************************************************/
               /* check whether specified facename is available on machine */
               /*******************************************************/
               for ( i=0; i<MAX_DIF_DOC ;i++)
               {
                  if (!*paszFontFacesGlobal)
                  {
                     strcpy(paszFontFacesGlobal, chFaceName);
                  } /* endif */
				  paszFontFacesGlobal += LF_FACESIZE;
               } /* endfor */
              break;
            case PROF_LU_TRNOTESTRING:
              memcpy( pEQFBUserOpt->chTRNoteAbbr, pData,
                      sizeof(pEQFBUserOpt->chTRNoteAbbr));
              afValue[ProfHead.usType] = TRUE;
              break;

            case PROF_EXTFONTS:
			  paszFontExtSpecs = get_aszFontExtSpecs();
              memcpy( paszFontExtSpecs, pData,
                      MAXEXTTYPE * sizeof( CHARFORMAT2));
              afValue[ProfHead.usType] = TRUE;
              break;
            default:
              break;                  // ignore unknown profile blocks

         } /* endswitch */
      } /* endif */
   } /* endif */

   // Add for R012027 start
   if (bReadKeys)
   {
       OtmReadSCKeyProfile();
   }
   // Add end

   // close any open profile file
   if ( hProfile )
   {
      UtlClose( hProfile, FALSE );
   } /* endif */

   /*******************************************************************/
   /* if profile corrupted, force use of defaults                     */
   /*******************************************************************/
   if (!EQFBIsProfileOK() )
   {
      i = PROF_KEYS;
      while (i < PROF_EOF)
      {
        afValue[i] = FALSE;
        i++;
      } /* endwhile */
   } /* endif */

   // initialize (= set to default values) all profile block types not
   // contained in the profile
   ProfHead.usType = 0;
   while ( !sRC && (ProfHead.usType < PROF_EOF))
   {
      if ( !afValue[ProfHead.usType] ) // if no data was supplied in profile
      {
         switch ( ProfHead.usType )    // set default values for this type
         {
            case PROF_KEYS:
               memcpy( pKeyTable, pDefKeyTable, sizeof( KEYPROFTABLE) * (LAST_FUNC+1));
               break;
            case PROF_FONTCOLS:
               memcpy( pTextTypeTable, pDefTextTypeTable, sizeof(TEXTTYPETABLE) * MAXCOLOUR);
               break;
            case PROF_LU_OPEN:
               pOpenData->fTroja  = TRUE;
               pOpenData->fSource = FALSE;
               strcpy( pOpenData->szDocPattern, DEFAULT_PATTERN );
               strcpy( pOpenData->szFilePattern, DEFAULT_PATTERN );
               pOpenData->szFileName[0] = EOS;
               pOpenData->szDocName[0] = EOS;
               break;
            case PROF_LU_RIGHTMARGIN:  // last used values of right margins
               *pusRightMargin = RMARGIN_DEFAULT;
               break;
            case PROF_LU_FONTSIZE:     // last used values for size of windows
            case PROF_LU_FONTFACENAME: // last used values for fontfacenames
               EQFBGetDefCellSize( &vioDefFontCellSize, chFaceName );
			   pvioFontSize = get_vioFontSize();
			   paszFontFacesGlobal = get_aszFontFacesGlobal();
               for ( i=0; i<MAX_DIF_DOC ;i++)
               {
                  *pvioFontSize = vioDefFontCellSize;   // set default cellsize
				  pvioFontSize++;
                  strcpy(paszFontFacesGlobal, chFaceName);
				  paszFontFacesGlobal += LF_FACESIZE;
               } /* endfor */

               break;
             case  PROF_IN_SHRNKSTRINGS:
               strcpy(pEQFBUserOpt->szInTagAbbr,SHRINKIN_SEGDATA);
               break;
             case  PROF_OUT_SHRNKSTRINGS:
               strcpy(pEQFBUserOpt->szOutTagAbbr,SHRINKOUT_SEGDATA);
               break;
             case PROF_LU_TRNOTESTRING:
               strcpy( pEQFBUserOpt->chTRNoteAbbr, TRNOTE_SEGDATA);
               break;

             case PROF_EXTFONTS:
                 /*****************************************************/
                 /* FontExt2 is currently bigger than aszFontExtSpecs */
                 /* since it contains already the ITM charformats     */
                 /*****************************************************/
          			 paszFontExtSpecs = get_aszFontExtSpecs();
                 memcpy( paszFontExtSpecs, FontExt2, MAXEXTTYPE * sizeof( CHARFORMAT2));
               break;

             case  PROF_SET_USEROPTS:
               pEQFBUserOpt->fSrcUnChg = TRUE;     // source unchanged
               pEQFBUserOpt->fFuzzyMsg = TRUE;     // displ. fuzzy unch. message
               pEQFBUserOpt->fAutoStop = TRUE;     // displ. source unch. message
               pEQFBUserOpt->DispTM = DISP_PROTECTED; //dispstyle in TM window
               pEQFBUserOpt->UserOptFlags.bConvSOSI = FALSE;    // do not insert SOSI in SaveSeg
               pEQFBUserOpt->fAutoRepl = TRUE;     // automatic replace in fuzzy
               pEQFBUserOpt->fCUABksp = TRUE;
               pEQFBUserOpt->fLineWrap = TRUE;
               pEQFBUserOpt->fTagCheck = FALSE;
               pEQFBUserOpt->fCrsInsert = FALSE;
               pEQFBUserOpt->fInsProposal = FALSE;
               pEQFBUserOpt->fFullSeg = TRUE;        // display complete tm prop
               pEQFBUserOpt->DispTrans = DISP_PROTECTED;
               pEQFBUserOpt->fInitCrsMode = FALSE;
               pEQFBUserOpt->sRMargin = RMARGIN_DEFAULT;
//               pEQFBUserOpt->fIgnCRLF = TRUE;
               pEQFBUserOpt->sFocusLine = FOCUSLINE;
               pEQFBUserOpt->fSrcPropWnd = FALSE;
               pEQFBUserOpt->fNumProp = FALSE;
               pEQFBUserOpt->fOriginProp = FALSE;
               pEQFBUserOpt->fDateOfProp = FALSE;
               pEQFBUserOpt->fAddInfoDic = FALSE;
               pEQFBUserOpt->fLkupSingleOfCompounds = 0;
               pEQFBUserOpt->fUseLatestMatch = FALSE;
               pEQFBUserOpt->fAllDictTerms = FALSE;
               pEQFBUserOpt->fExactContextTMMatch = TRUE;
               pEQFBUserOpt->fSegBound = FALSE;
               pEQFBUserOpt->bSegmentBoundary = 0x20;

               pEQFBUserOpt->UserOptFlags.bVisibleSpace = FALSE;
               pEQFBUserOpt->bVisibleLineFeed = 0x20;
               pEQFBUserOpt->bVisibleBlank = 0x20;

               pEQFBUserOpt->fDispDictName = FALSE;
               pEQFBUserOpt->UserOptFlags.bDispMTAlways = TRUE;
               pEQFBUserOpt->UserOptFlags.bBackSave = FALSE;
               pEQFBUserOpt->UserOptFlags.bBackSubst = FALSE;
               pEQFBUserOpt->sMinuteTilNextSave = 20;
               pEQFBUserOpt->UserOptFlags.bAutoSpellCheck = FALSE;
               pEQFBUserOpt->UserOptFlags.bBidiLogicDisplay = TRUE;
               pEQFBUserOpt->fNoCUASel = 0;
               pEQFBUserOpt->fDispMemName = FALSE;
               pEQFBUserOpt->fDispMemIndicator = FALSE;
               break;
         } /* endswitch */
      } /* endif */
      ProfHead.usType++;
   } /* endwhile */

   if ( pData )
   {
      UtlAlloc((PVOID *) &pData, 0L, 0L, NOMSG );
   } /* endif */
   EQFBCheckAssignedResKey();
   EQFBCheckEditor();
   return( sRC );
}

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBWriteProfile
//-----------------------------------------------------------------------------
// Function call:     EQFBWriteProfile()
//-----------------------------------------------------------------------------
// Description:       Write profile data
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//-----------------------------------------------------------------------------
// Returncode type:   USHORT usRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_OPENFILE   - error opening file
//                    ERR_WRITEFILE  - error writing file
//-----------------------------------------------------------------------------
// Side effects:      All profile related data areas are written to the
//                     profile file
//-----------------------------------------------------------------------------
// Function flow:     - get profile name
//                    - open profile file for output
//                    - while not all profile data processed
//                       -supply size and ptr of profile data
//                       -write profile data to file
//                      endwhile
//                    - close profile file
//-----------------------------------------------------------------------------

SHORT EQFBWriteProfile( PTBDOCUMENT pDoc )
{
   CHAR        szProfName[MAX_EQF_PATH]; // buffer for profile file name
   PROFILEHEAD ProfHead;               // profile block header
   HFILE       hProfile = (HFILE) NULL;// handle of profile file
   USHORT      usOpenAction;           // action performed by DosOpen
   USHORT      usDosRC;                // return code of DosXXX calls
   USHORT      usType;                 // current profile data type
   ULONG       ulBytesWritten;         // # of bytes written to file
   PVOID       pData;                  // ptr to current profile data
   SHORT       sRC = 0;                // function return code
   USHORT      usVersion;
   CHARFORMAT2* paszFontExtSpecs;
   USHORT* pusRightMargin = get_usRightMargin();
   VIOFONTCELLSIZE* pvioFontSize = get_vioFontSize();
   USEROPT* pEQFBUserOpt = get_EQFBUserOpt();

   sRC = EQFBQueryProfName( szProfName ); // get the profile name

   // open profile file for output
   if ( !sRC )
   {
      usDosRC = UtlOpen( szProfName,
                         &hProfile,
                         &usOpenAction, 0L,
                         FILE_NORMAL,
                         FILE_OPEN | FILE_CREATE,
                         OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                         0L,
                         FALSE );
      if ( usDosRC )
      {
         hProfile = (HFILE) NULL;
         sRC = ERR_OPENFILE;
      } /* endif */
   } /* endif */

   BOOL bWriteKeys = FALSE;                  // Add for R012027
   // write all profile data types
   usType = 0;                         // start with first profile data type
   while ( !sRC && (usType <= PROF_EOF) )   // while not all profile data processed
   {
      // set default values (to trap notimplemented profile parts)
      memset( &ProfHead, 0, sizeof(ProfHead) );
      pData  = NULL;

      // supply size and pointer of profile data
      switch ( usType )
      {
         case PROF_KEYS:
            pData = (PVOID) get_KeyTable();
            ProfHead.usType = usType;
            ProfHead.usSize = sizeof(KEYPROFTABLE) * (LAST_FUNC+1);
            bWriteKeys = TRUE;
            break;
         case PROF_FONTCOLS:
            pData = (PVOID) get_TextTypeTable();
            ProfHead.usType = usType;
            ProfHead.usSize = sizeof(TEXTTYPETABLE) * MAXCOLOUR;
            break;
         case PROF_LU_OPEN:
            pData = (PVOID) get_OpenData();
            ProfHead.usType = usType;
            ProfHead.usSize = sizeof(OPENDATA);
            break;
          case PROF_LU_RIGHTMARGIN:
            if (pDoc && pDoc->fAutoLineWrap )
            {
				*pusRightMargin = AUTOSIZE;
            } /* endif */
            pData = pusRightMargin;
            ProfHead.usType = usType;
            ProfHead.usSize = sizeof(USHORT);
            break;
         case PROF_LU_FONTSIZE:
            pData = pvioFontSize;
            ProfHead.usType = usType;
            ProfHead.usSize = sizeof(VIOFONTCELLSIZE) * MAX_DIF_DOC;
            break;
         case  PROF_IN_SHRNKSTRINGS:
            pData = pEQFBUserOpt->szInTagAbbr;
            ProfHead.usType = usType;
            ProfHead.usSize = MAXSHRKLEN+1;
            break;
         case  PROF_OUT_SHRNKSTRINGS:
            pData = pEQFBUserOpt->szOutTagAbbr;
            ProfHead.usType = usType;
            ProfHead.usSize = MAXSHRKLEN+1;
            break;
         case PROF_SET_USEROPTS:
            pData = pEQFBUserOpt;
            ProfHead.usType = usType;
            ProfHead.usSize = sizeof(USEROPT);
            break;
          case PROF_LU_FONTFACENAME:
            pData = (PVOID) get_aszFontFacesGlobal();
            ProfHead.usType = usType;
            ProfHead.usSize = MAX_DIF_DOC * LF_FACESIZE;
            break;
          case PROF_VERSION:
            usVersion = PROF_CURVERSION;
            pData = &usVersion;
            ProfHead.usType = usType;
            ProfHead.usSize = sizeof(USHORT);
            break;
          case PROF_LU_TRNOTESTRING:
            pData = pEQFBUserOpt->chTRNoteAbbr;
            ProfHead.usType = usType;
            ProfHead.usSize = sizeof(pEQFBUserOpt->chTRNoteAbbr);
            break;
         case PROF_EOF:
            ProfHead.usType = usType;
            ProfHead.usSize = 0;       // no data area for end-of-file marker
            pData  = NULL;
            break;

         case PROF_EXTFONTS:
            ProfHead.usType = usType;
            ProfHead.usSize = MAXEXTTYPE * sizeof(CHARFORMAT2);
            paszFontExtSpecs = get_aszFontExtSpecs();
            pData = paszFontExtSpecs;
            break;

      } /* endswitch */

      // write profile data to file
      if ( ProfHead.usType )
      {
         usDosRC = UtlWriteL( hProfile,
                             &ProfHead,
                             sizeof(ProfHead),
                             &ulBytesWritten,
                             FALSE );
         sRC = ( usDosRC ) ? ERR_WRITEFILE : 0;
      } /* endif */
      if ( !sRC && ProfHead.usSize )
      {
         usDosRC = UtlWriteL( hProfile,
                             pData,
                             (ULONG)ProfHead.usSize,
                             &ulBytesWritten,
                             FALSE );
         sRC = ( usDosRC ) ? ERR_WRITEFILE : 0;
      } /* endif */

      // continue with next profile data type
      usType++;

   } /* endwhile */

   // Add for R012027 start
   if (bWriteKeys)
   {
       OtmWriteSCKeyProfile();
   }
   // Add end

   // close profile file
   if ( hProfile )
   {
      UtlClose( hProfile, FALSE );
   } /* endif */

   return( sRC );
}

//-----------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------
// Function name:     EQFBQueryProfName
//-----------------------------------------------------------------------------
// Function call:     _
//-----------------------------------------------------------------------------
// Description:       Build translation processor profile name
//-----------------------------------------------------------------------------
// Parameters:        PSZ pszProfName - buffer for created profile name
//-----------------------------------------------------------------------------
// Returncode type:   USHORT usRC
//-----------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_OPENFILE   - error opening file
//                    ERR_WRITEFILE  - error writing file
//-----------------------------------------------------------------------------
// Prerequesits:      System properties are loaded
//-----------------------------------------------------------------------------
// Function flow:     create fully qualified profile filename
//-----------------------------------------------------------------------------

SHORT EQFBQueryProfName( PSZ pszProfName )
{
   SHORT  sRC = 0;                     // function return code
   PPROPSYSTEM pSysProp = get_pSysProp();

   sprintf( pszProfName, "%s\\%s\\%s",
            pSysProp->PropHead.szPath,
            pSysProp->szPropertyPath,
            TPRO_PROFILE );

   return( sRC );
}

BOOL EQFBIsProfileOK
(
  VOID
)
{
  USHORT        usJ = 0;
  BOOL          fOK = TRUE;
  PKEYPROFTABLE pKeyTemp;             // temp pointer

  pKeyTemp = get_KeyTable();                // pointer to keytable

  while ((pKeyTemp->Function <= LAST_FUNC ) && (usJ < LAST_FUNC ))
  {
     usJ++;
     pKeyTemp++;
  } /* endwhile */
  if (pKeyTemp->Function > LAST_FUNC )
  {
    fOK = FALSE;
  }
  else
  {
    fOK = TRUE;
  } /* endif */

  return(fOK);
}

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBCheckEditor
//-----------------------------------------------------------------------------
// Function call:     EQFBCheckEditor
//-----------------------------------------------------------------------------
// Description:
//-----------------------------------------------------------------------------
// Parameters:        -
//-----------------------------------------------------------------------------
// Returncode type:   -
//-----------------------------------------------------------------------------
// Returncodes:       void
//-----------------------------------------------------------------------------
// Function flow:
//-----------------------------------------------------------------------------
VOID   EQFBCheckEditor
(
   VOID
)
{
  USHORT    usJ = 0;
  USHORT    usI = 0;
  KEYPROFTABLE* pDefKeyTable = get_DefKeyTable();
  KEYPROFTABLE* pKeyTable = get_KeyTable();

  /********************************************************************/
  /* copy bEditor indicator from DefKeyTable in active KeyTable       */
  /********************************************************************/

  while (usJ < LAST_FUNC)
  {
    usI = 0;
    while ( usI < LAST_FUNC )
    {
      if ( pKeyTable->Function == pDefKeyTable->Function )
      {
        pKeyTable->bEditor = pDefKeyTable->bEditor;
        usI = LAST_FUNC;
      }
      else
      {
        usI++;
		pDefKeyTable++;
      } /* endif */
    } /*endwhile */
    usJ++;
	pKeyTable++;
  } /* endwhile */

  return;
}
//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBCheckAssignedResKey
//-----------------------------------------------------------------------------
// Function call:     EQFBCheckAssignedResKey
//-----------------------------------------------------------------------------
// Description:
//-----------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//-----------------------------------------------------------------------------
// Returncode type:   -
//-----------------------------------------------------------------------------
// Returncodes:       void
//-----------------------------------------------------------------------------
// Function flow:
//-----------------------------------------------------------------------------
VOID   EQFBCheckAssignedResKey
(
   VOID
)
{
  UCHAR     ucCurCode;
  UCHAR     ucCurState;
  SHORT     sCurFunction = 0;
  SHORT     sRc = 0;
  USHORT    usAssignStatus = NOT_ASSIGNABLE;
  BOOL      fFound = FALSE;
  USHORT    usI = 0;
  USHORT    usJ = 0;
  RESKEYTABLE* pResKeyTab;
  
  /********************************************************************/
  /* check each reserved key                                          */
  /********************************************************************/
  pResKeyTab = get_ResKeyTab();
  while (!sRc && (sCurFunction != -1) )
  {
    ucCurCode = pResKeyTab->ucCode;
    ucCurState = pResKeyTab->ucState;
    sCurFunction = pResKeyTab->Function;
    usAssignStatus = pResKeyTab->usAssignStatus;
    fFound = FALSE;
    if (sCurFunction && (usAssignStatus == ASSIGNABLE))
    {
      KEYPROFTABLE* pKeyTable = get_KeyTable();
      usJ = 0;
      while (!fFound && (usJ < LAST_FUNC))
      {
        if ((pKeyTable->ucCode == ucCurCode )
             && (pKeyTable->ucState == ucCurState ) )
        {
          pResKeyTab->usAssignStatus = ASSIGNED_TO_OTHER;
          fFound = TRUE;
        } /* endif */
        usJ++;
        pKeyTable++;
      } /* endwhile */
    } /* endif */
    usI++;
	pResKeyTab++;
  } /* endwhile */

  usI = 0;
  sCurFunction = 0;

  pResKeyTab = get_ResKeyTab();
  while (!sRc && (sCurFunction != -1) )
  {
    ucCurCode = pResKeyTab->ucCode;
    ucCurState = pResKeyTab->ucState;
    sCurFunction = pResKeyTab->Function;
    usAssignStatus = pResKeyTab->usAssignStatus;
    fFound = FALSE;
    if (sCurFunction && (usAssignStatus == NOT_ASSIGNABLE))
    {
      KEYPROFTABLE* pKeyTable = get_KeyTable();
      usJ = 0;
      while (!fFound && (usJ < LAST_FUNC))
      {
        if ((pKeyTable->ucCode == ucCurCode )
             && (pKeyTable->ucState == ucCurState ) )
        {
          fFound = TRUE;
          pKeyTable->ucCode = 0;
          pKeyTable->ucState = 0;
        } /* endif */
        usJ++;
        pKeyTable++;
      } /* endwhile */
    } /* endif */
    usI++;
	pResKeyTab++;
  } /* endwhile */

  return;
}

// Add for R012027 start
void OtmWriteSCKeyProfile()
{
    CHAR    szProfName[MAX_EQF_PATH];   // buffer for profile file name
    SHORT   sRC = 0;                    // function return code
    USHORT  usDosRC;                    // return code of DosXXX calls
    HFILE   hProfile = (HFILE) NULL;    // handle of profile file
    USHORT  usOpenAction;               // action performed by DosOpen
    ULONG   ulBytesWritten;             // # of bytes written to file

    // Query the file name
    sRC = OtmQuerySCKeyProfName(szProfName); // get the profile name

    if (sRC)
    {
        return;
    }

    // write profile data to file
    SPECCHARKEYVEC* pSpecCharKeyVec = GetSpecCharKeyVec();

    if ((*pSpecCharKeyVec).empty())
    {
        UtlDelete(szProfName, FALSE, FALSE);
        return;
    }

    // open profile file for output
    usDosRC = UtlOpen(szProfName, &hProfile, &usOpenAction, 0L, FILE_NORMAL, FILE_OPEN | FILE_CREATE,
                      OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, FALSE);
    if (usDosRC)
    {
        hProfile = (HFILE) NULL;
        sRC = ERR_OPENFILE;
    }

    if (sRC)
    {
        return;
    }

    // Write length first
    size_t nLen = (*pSpecCharKeyVec).size();
    usDosRC = UtlWriteL(hProfile, &nLen, (ULONG)sizeof(size_t), &ulBytesWritten, FALSE);
    sRC = ( usDosRC ) ? ERR_WRITEFILE : 0;

    if (sRC)
    {
        UtlClose(hProfile, FALSE);
        return;
    }

    for (size_t iInx = 0; iInx < nLen; iInx++)
    {
        usDosRC = UtlWriteL(hProfile, &(*pSpecCharKeyVec)[iInx], (ULONG)sizeof((*pSpecCharKeyVec)[iInx]), &ulBytesWritten, FALSE);
        sRC = ( usDosRC ) ? ERR_WRITEFILE : 0;
    }

    // close profile file
    if (hProfile)
    {
        UtlClose(hProfile, FALSE);
    }
}

void OtmReadSCKeyProfile()
{
    CHAR    szProfName[MAX_EQF_PATH];   // buffer for profile file name
    SHORT   sRC = 0;                    // function return code
    USHORT  usDosRC;                    // return code of DosXXX calls
    HFILE   hProfile = (HFILE) NULL;    // handle of profile file
    USHORT  usOpenAction;               // action performed by DosOpen
    ULONG   ulBytesRead;                // # of bytes read from file

    // Query the file name
    sRC = OtmQuerySCKeyProfName(szProfName); // get the profile name

    if (sRC)
    {
        return;
    }

    // open profile file for read
    usDosRC = UtlOpen(szProfName, &hProfile, &usOpenAction, 0L, FILE_NORMAL, FILE_OPEN,
                      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, 0L, FALSE);
    if (usDosRC)
    {
        hProfile = (HFILE) NULL;
        sRC = ERR_OPENFILE;
    }

    if (sRC)
    {
        return;
    }

    // read profile data from file
    ClearSpecCharKeyVec();
    SPECCHARKEYVEC* pSpecCharKeyVec = GetSpecCharKeyVec();

    // Write length first
    size_t nLen = 0;
    usDosRC = UtlReadL(hProfile, &nLen, sizeof(size_t), &ulBytesRead, FALSE);
    sRC = (usDosRC) ? ERR_READFILE : 0;

    if (sRC)
    {
        return;
    }

    // get / check buffer for profile data
    for (size_t iInx = 0; iInx < nLen; iInx++)
    {
        SPECCHARKEY oneSCKey;
        usDosRC = UtlReadL(hProfile, &oneSCKey, (ULONG)sizeof(oneSCKey), &ulBytesRead, FALSE);
        (*pSpecCharKeyVec).push_back(oneSCKey);
        sRC = (usDosRC) ? ERR_READFILE : 0;
        if (sRC)
        {
            break;
        }
    }

    BackupSpecCharKeyVec();

    // close profile file
    if (hProfile)
    {
        UtlClose(hProfile, FALSE);
    }
}

SHORT OtmQuerySCKeyProfName(PSZ pszProfName)
{
   SHORT  sRC = 0;                     // function return code
   PPROPSYSTEM pSysProp = get_pSysProp();

   sprintf( pszProfName, "%s\\%s\\%s",
            pSysProp->PropHead.szPath,
            pSysProp->szPropertyPath,
            SPEC_CHAR_KEY_PROFILE);

   return (sRC);
}
// Add end

VOID WriteMTLog( PACTSEGLOGENH2 pSegLog, PSZ_W pszSegSource, PSZ pszPropSourceDoc, PSZ_W pszMetaData, PPEDATABUFFER pPEData )
{
  WriteToMTLog( hMTLog, pSegLog, pszSegSource, pszPropSourceDoc, pszMetaData, pPEData );
}

