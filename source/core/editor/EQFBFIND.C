//+----------------------------------------------------------------------------+
//|EQFBFIND.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:                                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: This file contains all routines concerned with finding a       |
//|             selected word                                                  |
//+----------------------------------------------------------------------------+
#define INCL_EQF_MORPH            // morph settings for Arabic/Hebrew detection
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // public EDITOR API functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFBDLG.ID"             // dialog control IDs

static SHORT EQFBFindMatchBack ( PTBDOCUMENT, PFINDDATA); //find match backwards
static SHORT EQFBChangeMatch ( PTBDOCUMENT,PFINDDATA); //change match
BOOL EQFBInRange (PTBDOCUMENT, PFINDDATA, ULONG);    //check if in change range

MRESULT EQFBFindInit( HWND, WPARAM, LPARAM );
MRESULT EQFBFindCommand( HWND, WPARAM, LPARAM );
VOID    EQFBFindFill( HWND, PFINDDATA );          // fill find data struct
VOID    EQFBFindButtons( PTBDOCUMENT, HWND, BOOL);// enable/disable buttons

static SHORT EQFBInProtected
(
    PTBDOCUMENT pDoc,
    PFINDDATA   pFindData
);
static
VOID EQFBFindInPrevSeg
(
    PFINDDATA   pFindData,
    PTBDOCUMENT pDoc,
    PULONG      pulSegNum,
    PUSHORT     pusSegOffset,
    PBOOL       pfWrapped
);

static
VOID EQFBBlanksAndLFSubst
(
    PTBDOCUMENT  pDoc,
    PSZ_W        pTextStart,
    ULONG        ulSegNum,
    PTBSEGMENT   pSeg
);


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFindMatch                                            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFindMatch(PTBDOCUMENT,PFINDDATA)                     |
//+----------------------------------------------------------------------------+
//|Description:       find the next occurence of the specified text            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc,               ptr to doc instance_     |
//|                   PFINDDATA pFindData             ptr to FindData          |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:               0    - next occurence found                      |
//|                   WARN_NOMATCH - no match found                            |
//|                   WARN_OUTRANGE - if called from change, match is found    |
//|                                   but out of range                         |
//+----------------------------------------------------------------------------+
//|Prerequesits:      TBCURSOR.ulSegNum and usSegOffset point to the start     |
//|                      position                                              |
//+----------------------------------------------------------------------------+
//|Function flow:     if backward searching , EQFBFindMatchBack                |
//|                   else:  - init local variables                            |
//|                          - if not 1st call: skip previously found match    |
//|                          - while no match found and next segment available |
//|                             -get ptr to text                               |
//|                             - make uppercase if Ignorecase                 |
//|                             - while not at end of segment                  |
//|                                  or no match found                         |
//|                               - Scan for 1st letter of the target string   |
//|                               -if no match:set segment end indicator       |
//|                                else: if complete match: set found indic.   |
//|                                      else: increment offset                |
//|                                               or set segment end           |
//|                             - if segend: increment ulSegNum                |
//|                                          or set 'No match found'           |
//|                          -If a match is found ensure cur pos is visible    |
//|                             on the screen.                                 |
//|                           else set rc = 'No match found'                   |
//|                                                                            |
//+----------------------------------------------------------------------------+

SHORT EQFBFindMatch
(
PTBDOCUMENT pDoc,                  //ptr to doc instance
PFINDDATA pFindData                // ptr to FindData
)
{
  int    found;
  USHORT usLen;                       //length of chFindTarget
  USHORT usLen2;                       //length of chReplace
  PSZ_W  pData;
  SHORT  sRc;                         //return value
  PSZ_W  pText = NULL;                //ptr to text
  BOOL   fSegend;                     //TRUE if end of segment, else FALSE
  ULONG  ulSegNum;                    //store TBCUrsor
  USHORT usSegOffset;                 //store TBCursor
  USHORT usWrapCount = 0 ;            //Count # of times restart at segment 1
  PTBSEGMENT pSeg;                    //ptr to segment
  CHAR_W chChar;                      //temporarily store current char
  CHAR_W c;                           //temporarily hold *pData
  USHORT usMatchLen;                  //length of matches string

  // copy, otherwise chFindTarget is empty
  UTF16strcpy( pFindData->chFindTarget, pFindData->chFind );
  // set document, otherwise it's empty
  if(pFindData->pDoc == NULL)
	  pFindData->pDoc = pDoc;

  if (!pFindData->fForward)                  //separate routine if backward
  {
    sRc = EQFBFindMatchBack(pDoc,pFindData);
  }
  else
  {
    usMatchLen = usLen = (USHORT) UTF16strlenCHAR(pFindData->chFindTarget);
    ulSegNum = pFindData->ulSegNumBegin;
    usSegOffset = pFindData->usSegOffsetBegin;
    sRc = 0;
    found = FALSE;
    fSegend = FALSE;

    if (pFindData->usFirstCall == CH_CHNGEFIND )  // skip previously found match
    {
      if (pFindData->fChange )
      {                                 //skip previously changed word
        usLen2 = (USHORT) UTF16strlenBYTE(pFindData->chReplace);
        usSegOffset = usSegOffset + usLen2;

        if ((ulSegNum == pFindData->ulFirstSegNum) &&
            (usSegOffset <= pFindData->usFirstSegOffset) )
        {
          /***********************************************************/
          /* adjust usFirstSegOffset nec due to replace in segment   */
          /* ( chFindTarget with length usLen has been deleted,      */
          /* chReplace with length usLen2 has been added )           */
          /***********************************************************/
          pFindData->usFirstSegOffset = pFindData->usFirstSegOffset
                                        + usLen2 - usLen;
        } /* endif */

      }
      else
      {                                // skip previuosly found match
        usSegOffset = usSegOffset + usLen;
      } /* endif */
    } /* endif */
    if ( pFindData->usFirstCall == CH_MATCHOUTRANGE )
    {
      usSegOffset = usSegOffset + usLen;
    } /* endif */

    if ((pFindData->usFirstCall == CH_CHNGEFIND) ||
        (pFindData->usFirstCall == CH_MATCHOUTRANGE )  )
    {
      if ((ulSegNum == pFindData->ulFirstSegNum) &&
          (usSegOffset == pFindData->usFirstSegOffset) )
      {
        sRc = WARN_NOMATCH;
      } /* endif */
    } /* endif */

    /*****************************************************************/
    /* get country info in case of SBCS                              */
    /*****************************************************************/
	
    if (pFindData->fIgnoreCase )
    {
      UtlUpperW(pFindData->chFindTarget );
    } /* endif */

    //while no match found and next segment available
    while (!found && sRc == 0)
    {
      pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
      if (pSeg && pSeg->pDataW)
      {
        fSegend = FALSE;
        // If we doing a non-exact case match the current line is
        // copied into the work line and uppercased.
        if (UtlAlloc((PVOID *)&pText, 0L,
                     (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W),ERROR_STORAGE))
        {
           EQFBBlanksAndLFSubst ( pDoc, pText, ulSegNum, pSeg );

          if ((ulSegNum == pFindData->ulFirstSegNum)
              && (usSegOffset < pFindData->usFirstSegOffset))
          {
            // stops search where the search started
            *(pText+pFindData->usFirstSegOffset) = EOS;
          } /* endif */

          if (pFindData->fIgnoreCase)
          {
            UtlUpperW( pText );
          } /* endif */
        }
        else
        {
          sRc = WARN_NOMATCH;
        } /* endif */
      }
      else
      {
        //sRc = WARN_NOMATCH;
        fSegend = TRUE;     // force wrap-around (P019858)
      } /* endif */
      //if forward:while not at end of segment or no match found
      if ( !sRc)                         // if pText ok...
      {
        while (!found && !fSegend)
        {
          /******************************************************/
          /* Scan for the 1st letter of the target string       */
          /* don't use strchr, it is too slow                   */
          /******************************************************/
          chChar = (CHAR_W) *pFindData->chFindTarget;
          pData = pText+usSegOffset;
          while (((c=*pData) != NULC) && (chChar != c) )
          {
            pData ++;
          } /* endwhile */

          if (!(*pData) )
          {
            fSegend = TRUE;
            usSegOffset = (USHORT)(pData-pText);
          }
          else
          {
            // then check for complete match
            if (StrNLFCmp(pData, pFindData->chFindTarget) == 0)
            {
              found = TRUE;
              usSegOffset = (USHORT)(pData - pText);

              // GQ 20170926 Adjust usMatchLen, the segment may contain softline feeds which are not contained in usLen yet
              for( USHORT usI = 0; usI < usLen; usI++ )
              {
                if ( pData[usI] == SOFTLF_CHAR )
                {
                  usMatchLen++;
                } /* endif */
              } /* endfor */
            }
            else         // no match; go on if possible
            {
              usSegOffset = (USHORT)(pData - pText);
              chChar = (CHAR_W) *(pText+usSegOffset); //goto next char
              if (chChar != '\0')                   //adjusted for mixed
              {
                usSegOffset = usSegOffset + 1;
              }
              else
              {
                fSegend = TRUE;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endwhile */

        if (fSegend)
        {                            // free space
          if(pText) UtlAlloc((PVOID *)&pText, 0L, 0L,NOMSG);
          //check if next segment available
          if (ulSegNum < pFindData->ulFirstSegNum )
          {
            ulSegNum++;
            usSegOffset = 0;
            if ((ulSegNum == pFindData->ulFirstSegNum) &&
                (usSegOffset == pFindData->usFirstSegOffset) )
            {
              sRc = WARN_NOMATCH;
            } /* endif */
          }
          else
          {
            if ((ulSegNum == pFindData->ulFirstSegNum ) &&
                (usSegOffset == pFindData->usFirstSegOffset ) )
            {
              // segend has been set to position usFirsSegOffset
              sRc = WARN_NOMATCH;
            }
            else
            {
              if (ulSegNum < pFindData->pDoc->ulMaxSeg - 1 )
              {
                ulSegNum ++;
                usSegOffset = 0;
              }
              else
              {
                ulSegNum = 1;
                usSegOffset = 0;
                usWrapCount++;
                if ( ( (ulSegNum == pFindData->ulFirstSegNum) &&
                       (usSegOffset == pFindData->usFirstSegOffset) ) ||
                     ( usWrapCount > 1 ) ) 
                {
                  sRc = WARN_NOMATCH;
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */

      } /* endif */
    } /*endwhile*/

    /* We arrive here either having found a match or not. */
    if (!found)
    {
      sRc = WARN_NOMATCH;
    }

    /* If a match is found ensure the current position is visible */
    /* on the screen.                                             */
    else
    {
      if ( pFindData->fChange )
      {
        /**************************************************************/
        /* if change is selected, position only if in change range    */
        /* and if the range is currenttoend.                          */
        /* If "segment only" is selected and the match is not in range*/
        /* (which is equal to no match found) the find/change stops   */
        /**************************************************************/
        if ( !EQFBInRange(pDoc,pFindData,ulSegNum) )
        {
          if ( pFindData->usRange == CHANGE_CURRENTTOEND )
          {
            sRc = WARN_OUTRANGE;
            pFindData ->usFirstCall = CH_MATCHOUTRANGE;
          }
          else
          {
            sRc = WARN_NOMATCH;
          } /* endif */
        }
        else
        {
          pFindData->usFirstCall =  CH_CHNGEFIND;
          //goto segment , position TBRowOffset and TBCursor
          EQFBFindGotoSeg(pDoc,ulSegNum,usSegOffset,usMatchLen);
        } /* endif */
        /**************************************************************/
        /* set begin position for consecutive calls                   */
        /**************************************************************/
        pFindData->ulSegNumBegin = ulSegNum;
        pFindData->usSegOffsetBegin = usSegOffset;
      }
      else
      {
        pFindData->usFirstCall =  CH_CHNGEFIND;
        EQFBFindGotoSeg(pDoc,ulSegNum,usSegOffset,usMatchLen);
        sRc = 0;
      } /* endif */
    } /* endif */
  } /* endif */
  
  return sRc;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFindMatchBack                                        |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFindMatchBack(PTBDOCUMENT,PFINDDATA)                 |
//+----------------------------------------------------------------------------+
//|Description:       find backwards next occurence of specified string        |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT       pointer to document instance data      |
//|                   PFINDDATA         ptr to pFindData                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0    next occurence found                                |
//|                   WARN_NOMATCH - no match found                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:      TBCURSOR.ulSegNum and usSegOffset point to the start     |
//|                      position                                              |
//+----------------------------------------------------------------------------+
//|Function flow:     - init local variables                                   |
//|                   - if not 1st call: skip previously found match           |
//|                   - while no match found and next segment available        |
//|                      -get ptr to text                                      |
//|                      - make uppercase if Ignorecase                        |
//|                      - while not at end of segment or no match found       |
//|                        - Scan for the 1st letter of target string          |
//|                        -if no match:set segment end indicator              |
//|                         else: if complete match:                           |
//|                                   set found indicator                      |
//|                               else: increment offset                       |
//|                                     or set segment end                     |
//|                      - if segend: increment ulSegNum                       |
//|                                   or set 'No match found'                  |
//|                   -If a match is found ensure cur pos is visible           |
//|                    on the screen.                                          |
//|                    else set rc = 'No match found'                          |
//|                                                                            |
//+----------------------------------------------------------------------------+

SHORT EQFBFindMatchBack
(
PTBDOCUMENT pDoc,                  //ptr to doc instance
PFINDDATA pFindData                // ptr to FindData
)
{
  int   found;
  CHAR_W chChar;                      //1st char of chFindTarget
  USHORT usLen;                       //length of chFindTarget
  BOOL   fFirstFound;                 //true if nmatcvh for 1st char found
  PSZ_W  pData;
  SHORT  sRc;                         //return value
  PSZ_W  pText = NULL;                //ptr to text
  BOOL   fSegend;                     //TRUE if end of segment, else FALSE
  ULONG  ulSegNum;                    //store TBCUrsor
  USHORT usSegOffset;                 //store TBCursor
  PTBSEGMENT pSeg;                    //ptr to segment
  PSZ_W  pDataPrev;                   //ptr to pos of previous'firstfound'
  PSZ_W  pTextStart = NULL;
  BOOL   fAddStart = FALSE;
  BOOL   fWrapped = FALSE;
  USHORT  usLen2;
  USHORT  usNextAfterFirstSegOffs = 1;  // is 2 if usFirstSegOffset == DBCS1st
  USHORT usMatchLen;                  //length of match in segment data
 
  pSeg = EQFBGetVisSeg(pDoc, &(pFindData->ulFirstSegNum));

  usMatchLen = usLen = (USHORT) UTF16strlenCHAR(pFindData->chFindTarget);
  ulSegNum = pFindData->ulSegNumBegin;                         /* @X3C */
  usSegOffset = pFindData->usSegOffsetBegin;                   /* @X3C */
  sRc = 0;
  found = FALSE;
  fSegend = FALSE;
  pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
 

  if ( (pFindData->fChange)
       && (pFindData->usFirstCall == CH_CHNGEFIND)
       && (ulSegNum == pFindData->ulFirstSegNum )
       && (usSegOffset <=pFindData->usFirstSegOffset ) )
  { 
    usLen2 = (USHORT) UTF16strlenCHAR(pFindData->chReplace);
    pFindData->usFirstSegOffset = pFindData->usFirstSegOffset
                                  + usLen2 - usLen;
	
  } /* endif */
  if ((ulSegNum == pFindData->ulFirstSegNum ) &&
      (usSegOffset == pFindData->usFirstSegOffset+usNextAfterFirstSegOffs ))
  {
    sRc = WARN_NOMATCH;
  } /* endif */

//      if (!pFindData->usFirstCall )          // skip previously found match
  if ((pFindData->usFirstCall == CH_CHNGEFIND)
      || (pFindData->usFirstCall == CH_MATCHOUTRANGE))
  {

    if (usSegOffset != 0 )
    {
      usSegOffset --;
    }
    else
    {
      EQFBFindInPrevSeg(pFindData, pDoc,&ulSegNum,
                         &usSegOffset, &fWrapped );
	  
    } /* endif */
  }
  else
  {// if at first call cursor is already on searched word, display it as 1st match
    if ((pFindData->usFirstSegOffset == 0) && (pFindData->usFirstCall != CH_FINDFIRST))
    {
      EQFBFindInPrevSeg(pFindData, pDoc,&ulSegNum,
                         &usSegOffset, &fWrapped );
    } /* endif */
  } /* endif */

  /*****************************************************************/
  /* get country info in case of SBCS                              */
  /*****************************************************************/
  if (pFindData->fIgnoreCase)
  {
      UtlUpperW( pFindData->chFindTarget );
  } /* endif */
  //while no match found and next segment available
  while (!found && ! sRc )
  {
    pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
    if (pSeg && pSeg->pDataW)
    {
      fSegend = FALSE;
      // If we doing a non-exact case match the current line is
      // copied into the work line and uppercased.
      if (UtlAlloc((PVOID *)&pTextStart, 0L,
                   (LONG)MAX_SEGMENT_SIZE*sizeof(CHAR_W),ERROR_STORAGE))
      {
        pText = pTextStart;
        EQFBBlanksAndLFSubst ( pDoc, pTextStart, ulSegNum, pSeg );

        if (pFindData->fIgnoreCase)
        {
            UtlUpperW( pText );
        } /* endif */
        fAddStart = FALSE;
        if (ulSegNum == pFindData->ulFirstSegNum )
        {
          if ( (pFindData->usSegOffsetBegin > pFindData->usFirstSegOffset)
               || (pFindData->ulSegNumBegin > pFindData->ulFirstSegNum)
               ||  fWrapped )
          {
            pText = pText + (pFindData->usFirstSegOffset)+usNextAfterFirstSegOffs;
            fAddStart = TRUE;                      // needed if match found
            /*******************************************************/
            /* if already one match has been found in this segment */
            /* and it is the starting/ending segment, check only   */
            /* the rest of this segment now                        */
            /*******************************************************/
            if ( (pFindData->ulSegNumBegin == pFindData->ulFirstSegNum )
                 && ( pFindData->usSegOffsetBegin >
                      pFindData->usFirstSegOffset+usNextAfterFirstSegOffs ) )
            {
              usSegOffset = pFindData->usSegOffsetBegin
                            - pFindData->usFirstSegOffset
                            - usNextAfterFirstSegOffs - 1;
            }
            else
            {
              usSegOffset = (USHORT) UTF16strlenCHAR(pText);
            } /* endif */
          } /* endif */
        } /* endif */
      }
      else
      {
        sRc = WARN_NOMATCH;
      } /* endif */
    }
    else
    {
      sRc = WARN_NOMATCH;
    } /* endif */
    //if Forward = FALSE:while not begin of segment or no match found
    if ( ! sRc)
    {
      while (!found && !fSegend)
      {
        /***********************************************************/
        /* scan for 1st letter of target string, but forward in-   */
        /* stead of backwards because of DBCS enabling             */
        /***********************************************************/
//         usChar = *(pFindData->chFindTarget);
        chChar = (CHAR_W) *pFindData->chFindTarget;
        pData = pText;
        fFirstFound = FALSE;
        pDataPrev = NULL;
        while ( !fSegend && !fFirstFound)
        {
//           if ( *pData == (CHAR) usChar )
          if ( (CHAR_W) (*pData) == chChar )
          {
            pDataPrev = pData;           //store pos if this match
          } /* endif */
          if ( pData >= pText+usSegOffset )  //end of loop reached ??
          {
            if ( pDataPrev )
            {
              pData = pDataPrev;
              fFirstFound = TRUE;
            }
            else
            {
              fSegend = TRUE;
            } /* endif */
          }
          else                              //else goto next char
          {
            pData++;
          } /* endif */
        } /* endwhile */
        if (fFirstFound)
        {
          //matching char found
          if (StrNLFCmp(pData, pFindData->chFindTarget) == 0)
          {
            found = TRUE;
            usSegOffset = (USHORT) (pData - pText);

            // GQ 20170926 Adjust usMatchLen, the segment may contain softline feeds which are not contained in usLen yet
            for( USHORT usI = 0; usI < usLen; usI++ )
            {
              if ( pData[usI] == SOFTLF_CHAR )
              {
                usMatchLen++;
              } /* endif */
            } /* endfor */

          }
          else         // no match; go on if possible
          {
            usSegOffset = (USHORT)(pData - pText);
            if (usSegOffset != 0)
            {
              usSegOffset--;
            }
            else
            {
              fSegend = TRUE;
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endwhile */

      if (fSegend)
      {                            // free space
        //check if next segment available
        if (pTextStart && (ulSegNum == pFindData->ulFirstSegNum) &&
            (pText == (pTextStart +
                       (pFindData->usFirstSegOffset+usNextAfterFirstSegOffs)))  )
        {
          sRc = WARN_NOMATCH;
        }
        else
        {
          EQFBFindInPrevSeg(pFindData, pDoc,&ulSegNum,
                         &usSegOffset, &fWrapped );
          if ((ulSegNum == pFindData->ulFirstSegNum ) &&
              (usSegOffset ==
               pFindData->usFirstSegOffset+usNextAfterFirstSegOffs ))
          {
            sRc = WARN_NOMATCH;        // not finally!!
          } /* endif */
        } /* endif */
        UtlAlloc((PVOID *)&pTextStart, 0L, 0L,NOMSG); // free spacr
      } /* endif */

    } /* endif */
  } /*endwhile*/

  /* We arrive here either having found a match or not. */
  if (!found)
  {
    sRc = WARN_NOMATCH;
  }

  /* If a match is found ensure the current position is visible */
  /* on the screen, but if change: only if in change range     */
  else
  {
    if ( pFindData->fChange )
    {
      /**************************************************************/
      /* if change is selected, position only if in change range    */
      /* and if the range is currenttoend.                          */
      /* If "segment only" is selected and the match is not in range*/
      /* (which is equal to no match found) the find/change stops   */
      /**************************************************************/
      if ( !EQFBInRange(pDoc,pFindData,ulSegNum) )
      {
        if ( pFindData->usRange == CHANGE_CURRENTTOEND )
        {
          sRc = WARN_OUTRANGE;
          pFindData ->usFirstCall = CH_MATCHOUTRANGE;
        }
        else
        {
          sRc = WARN_NOMATCH;
        } /* endif */
      }
      else
      {
        pFindData->usFirstCall =  CH_CHNGEFIND;
        if (fAddStart && (ulSegNum == pFindData->ulFirstSegNum))
        {
          usSegOffset += (pFindData->usFirstSegOffset
                          + usNextAfterFirstSegOffs);
        } /* endif */
        //goto segment , position TBRowOffset and TBCursor
        EQFBFindGotoSeg(pDoc,ulSegNum,usSegOffset,usMatchLen);
      } /* endif */
      /**************************************************************/
      /* set begin position for consecutive calls                   */
      /**************************************************************/
      pFindData->ulSegNumBegin = ulSegNum;
      if (fAddStart && (ulSegNum == pFindData->ulFirstSegNum) )
      {
        pFindData->usSegOffsetBegin = usSegOffset
                                      - pFindData->usFirstSegOffset
                                      - usNextAfterFirstSegOffs;
      }
      else
      {
        pFindData->usSegOffsetBegin = usSegOffset;
      } /* endif */
    }
    else
    {
      pFindData->usFirstCall =  CH_CHNGEFIND;

      if (fAddStart && (ulSegNum == pFindData->ulFirstSegNum))
      {
        usSegOffset += (pFindData->usFirstSegOffset + usNextAfterFirstSegOffs);
      } /* endif */
      EQFBFindGotoSeg(pDoc,ulSegNum,usSegOffset,usMatchLen);
      sRc = 0;
    } /* endif */

  }  /* endif */
  return sRc;
}

static
VOID EQFBFindInPrevSeg
(
    PFINDDATA   pFindData,
    PTBDOCUMENT pDoc,
    PULONG      pulSegNum,
    PUSHORT     pusSegOffset,
    PBOOL       pfWrapped
)
{
  ULONG       ulSegNum = *pulSegNum;
  PTBSEGMENT  pSeg;
  
  if (ulSegNum > 1)
  {
    ulSegNum--;
    pSeg = EQFBGetPrevVisSeg(pDoc, &ulSegNum);
    *pusSegOffset = (USHORT)UTF16strlenCHAR(pSeg->pDataW);
  }
  else
  {
	     ulSegNum = pFindData->pDoc->ulMaxSeg-1;
         pSeg = EQFBGetPrevVisSeg(pDoc, &ulSegNum);
         *pusSegOffset = (USHORT)UTF16strlenCHAR(pSeg->pDataW);
         *pfWrapped = TRUE;
   
  } /* endif */
 
  *pulSegNum = ulSegNum;
  return;
}


static
VOID EQFBBlanksAndLFSubst
(
    PTBDOCUMENT  pDoc,
    PSZ_W        pTextStart,
    ULONG        ulSegNum,
    PTBSEGMENT   pSeg
)
{
    BOOL   fWorkchng;                   // store EQFBFlags.workchng
    BOOL   fTyped;                      // store SegFlags.Typed
    PSZ_W p1 = pSeg->pDataW;
    PSZ_W p2 = pSeg->pDataW;
    CHAR_W c;

    if ( !IsDBCS_CP(pDoc->ulOemCodePage))
    {
      /******************************************************/
      /* if we are dealing with the work seg, get rid of    */
      /* pending blanks                                     */
      /******************************************************/
      if ( ulSegNum == pDoc->ulWorkSeg )
      {
        while ( (c = *p1 = *p2) != NULC )
        {
          if ( c == LF )
          {
            *p1 = BLANK;
            while ( *p1 == BLANK )
            {
              p1--;
            } /* endwhile */
            p1++;
            *p1 = LF;
          } /* endif */
          p1++;
          p2++;
        } /* endwhile */
        *p1 = EOS;
        fWorkchng = pDoc->EQFBFlags.workchng;

        // P016416: if SegFlags.Typed is changed to TRUE, color of segment can change to
        // "Translated as MODIFIED proposal" during POstedit - reviewing
        fTyped = pDoc->pTBSeg->SegFlags.Typed;  // P016416:
        EQFBUpdateChangedSeg(pDoc);      //update segment ids
        pDoc->EQFBFlags.workchng = (USHORT) fWorkchng;
        pDoc->pTBSeg->SegFlags.Typed = (USHORT) fTyped;
      } /* endif */
    }
    /******************************************************/
    /* substitute LF through blanks                       */
    /******************************************************/
    p1 = pTextStart;
    p2 = pSeg->pDataW;
    while ( (c = *p1 = *p2) != NULC )
    {
        // GQ 2017/09/25 Leave SOFT LF as-is
        // if ( (c == LF) || (c == SOFTLF_CHAR) )
        if ( c == LF ) 
        {
          if (!IsDBCS_CP(pDoc->ulOemCodePage) )           //for SBCS only
          {
            *p1 = BLANK;
          }
          else
          {
            /**************************************************/
            /* do not insert a blank, if a DBCS string is fol.*/
            /**************************************************/
            if (! EQFIsDBCSChar(*(p2+1), pDoc->ulOemCodePage)  )
            {
              *p1 = BLANK;
            }
            else
            {
              /************************************************/
              /* nothing to do -- LFs treated in StrNLFCmp    */
              /************************************************/
            } /* endif */
          } /* endif */
        } /* endif */
        p1++;
        p2++;
    } /* endwhile */
    *p1 = EOS;

    return;

}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBCmdFind                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBCmdFind(PTBDOCUMENT,PFINDDATA)                       |
//+----------------------------------------------------------------------------+
//|Description:       find next occurence of some text                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc       ptr to doc instance               |
//|                   PFINDDATA   pFindData  ptr to data for find/change       |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0    success                                             |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFBFindMatch                                       |
//|                   if no match found issue warning                          |
//+----------------------------------------------------------------------------+

SHORT EQFBCmdFind
(
PTBDOCUMENT pDoc,           //ptr to doc instance
PFINDDATA pFindData         // finddata structure
)

{
  pFindData->ulSegNumBegin = pDoc->TBCursor.ulSegNum;
  pFindData->usSegOffsetBegin = pDoc->TBCursor.usSegOffset;

  pFindData->fChange = FALSE;              //set FIND
  // Find the next match from the current position.  The FindMatch
  // call places the cursor in the data if match found
  /*******************************************************************/
  /* Find the next match from the current position. The FindMatch    */
  /* call places the cursor in the data if match found.              */
  /*******************************************************************/

  if (EQFBFindMatch(pDoc,pFindData) != 0 )
  {
    UtlError( TB_NOMATCH, MB_CANCEL, 0, NULL, EQF_WARNING);
  }


  return 0;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBCmdChange                                            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBCmdChange(PTBDOCUMENT,PFINDDATA)                     |
//+----------------------------------------------------------------------------+
//|Description:       change one piece of text into another                    |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc   ptr to document instance              |
//|                   PFINDDATA   pFIndData ptr to find/change structure       |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0 success (includes target not found)                    |
//+----------------------------------------------------------------------------+
//|Function flow:     - set change indicator                                   |
//|                   -if confirm on changes                                   |
//|                       - if not first call                                  |
//|                            if change is allowed in this range              |
//|                               call EQFBChangeMatch                         |
//|                       - call EQFBFIndMatch                                 |
//|                       - if no match issue warning                          |
//|                    else - set firstcall on true                            |
//|                         - call EQFBFindMatch                               |
//|                         - if no match issue warning                        |
//|                           else set found indicator                         |
//|                         - while match found and in Range                   |
//|                              (where change is allowed)                     |
//|                                 - change match                             |
//|                                 - if next match found,                     |
//|                                     set indicator                          |
//|                                   else set error message,                  |
//|                                      ( no match found)                     |
//|                        (issue warning if too long)                         |
//|                                                                            |
//+----------------------------------------------------------------------------+

SHORT EQFBCmdChange
(
PTBDOCUMENT pDoc,                         //ptr to doc instance
PFINDDATA   pFindData                     // finddata structure
)

{
  BOOL   fFound = FALSE;
  SHORT  sRc = 0;                                // return code
  SHORT  sOk = 0;                                //return from EQFBChangematch
                                                 //(!=0 if too long)

  pFindData->fChange = TRUE;              //set CHANGE
  if (pFindData->fConfirmChanges)         //if confirm = TRUE;
  {
//     if (!pFindData->usFirstCall)          //TBCursor is positioned
    if (pFindData->usFirstCall != CH_FINDFIRST)          //TBCursor is positioned
    {
      if (EQFBInRange(pDoc,pFindData,pDoc->TBCursor.ulSegNum))
      {
        EQFBChangeMatch(pDoc,pFindData);
      } /* endif */
    } /* endif */
    if (pFindData->usFirstCall != CH_CHNGEONLY)   //CH_CHNGEONLY is set if
    {
      //called from  spellcheck
      //if first call , init begin segnum and segoffset
      pFindData->ulSegNumBegin = pDoc->TBCursor.ulSegNum;
      pFindData->usSegOffsetBegin = pDoc->TBCursor.usSegOffset;
      sRc = EQFBFindMatch(pDoc,pFindData);

      if ( !sRc ) sRc = EQFBInProtected(pDoc, pFindData );

      while ( sRc == WARN_OUTRANGE )
      {
        sRc = EQFBFindMatch(pDoc,pFindData);
        if ( !sRc ) sRc = EQFBInProtected(pDoc, pFindData );
      } /* endwhile */
    } /* endif */

    if (sRc == WARN_NOMATCH)
    {
      /***************************************************************/
      /* make sure that after this warning 'find' is nec, an imme-   */
      /* diate change is not allowed (because it does nonsense)      */
      /***************************************************************/
      pFindData->usFirstCall = CH_FINDFIRST; //disable 2nd changing last match
      EQFBScreenData( pDoc );               // update screen first
      UtlError( TB_NOMATCH, MB_CANCEL, 0, NULL, EQF_WARNING);
    } /* endif */
  }
  else            //no confirm on changes
  {
    pFindData->usFirstCall = CH_FINDFIRST;                 // start findmatch at current pos
    //if first call , init begin segnum and segoffset
    pFindData->ulSegNumBegin = pDoc->TBCursor.ulSegNum;
    pFindData->usSegOffsetBegin = pDoc->TBCursor.usSegOffset;
    /******************************************************************/
    /* while match found  and ChangeMatch- rc is ok, find match       */
    /*  which is in Change-Range and change it                        */
    /* (i.e. do not leave loop at 1st out-of-range -match if          */
    /* currenttoend is specified, but leave loop at 1st out-of-range  */
    /* match if segment only is specified )                           */
    /* (Change-rc is ok if = 0)                                       */
    /******************************************************************/
    while ( sRc != WARN_NOMATCH && (sOk==0))
    {
      sRc = EQFBFindMatch(pDoc,pFindData);
      if ( !sRc ) sRc = EQFBInProtected(pDoc, pFindData );
      while ( sRc == WARN_OUTRANGE )
      {
        sRc = EQFBFindMatch(pDoc,pFindData);
        if ( !sRc ) sRc = EQFBInProtected(pDoc, pFindData );
      } /* endwhile */
      /****************************************************************/
      /* now either sRc = 0(=match found) or no match found           */
      /****************************************************************/
      if ( sRc == 0 )                       //change match found
      {
        fFound = TRUE;                      //set for "no further match..
        sOk = EQFBChangeMatch(pDoc,pFindData);
      }
      else                                    // no match found
      {
        EQFBScreenData( pDoc );                  // update screen first
        if ( fFound )                         //no further match found
        {
          UtlError( TB_NOFURTHERMATCH, MB_CANCEL, 0, NULL, EQF_WARNING);
        }
        else                                  //no match found
        {
          UtlError( TB_NOMATCH, MB_CANCEL, 0, NULL, EQF_WARNING);
        } /* endif */
      } /* endif */
    } /* endwhile */
  } /* endif */

  return sRc;
}

// set WARN_OUTRANGE to force that subsequent EQFBFindMatch does
// not recalc. pFindData->usFirstSegOffset if no replace took place
static
SHORT EQFBInProtected
(
    PTBDOCUMENT pDoc,
    PFINDDATA   pFindData
)
{   PTBSEGMENT pSeg;
    USHORT     usType;
    SHORT      sRc = 0;
    pSeg = EQFBGetSegW(pDoc, pFindData->ulSegNumBegin);

    usType = EQFBCharType(pDoc, pSeg, pFindData->usSegOffsetBegin );
    if (  (usType == PROTECTED_CHAR) || (usType == COMPACT_CHAR))
    {
      sRc = WARN_OUTRANGE;  // don't stop at such strings ...
      pFindData ->usFirstCall = CH_MATCHOUTRANGE;
    } /* endif */
    return (sRc);
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBChangeMatch                                          |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBChangeMatch(PTBDOCUMENT,PFINDDATA)                   |
//+----------------------------------------------------------------------------+
//|Description:       change the match found                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc  pointer to document instance data      |
//|                   PFINDDATA pFindData pointer to find/change data          |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:            0         else                                      |
//|                   WARN_TOOLONG - changed string would be too long          |
//+----------------------------------------------------------------------------+
//|Prerequesits:      TBCURSOR.ulSegNum and usSegOffset point to the start     |
//|                      position                                              |
//+----------------------------------------------------------------------------+
//|Function flow:     if character is protected, : beep                        |
//|                   else check worksegment                                   |
//|                        check if segment would not be too long              |
//|                        replace the match                                   |
//|                        call EQFBUpdateChangedSeg                           |
//|                                                                            |
//+----------------------------------------------------------------------------+

SHORT EQFBChangeMatch
(
PTBDOCUMENT pDoc,                  //ptr to doc instance
PFINDDATA pFindData                // ptr to FindData
)
{
  ULONG ulTlen;                            //length of chFindTarget
  ULONG ulRlen;                            //length of usReplace
  SHORT  sRc = 0;                           // return code
  PSZ_W  pSegData;                               // pointer to segment data
  USHORT usType;
  PTBSEGMENT pSeg;

  pSeg = EQFBGetSegW(pDoc,pDoc->TBCursor.ulSegNum);
  usType = EQFBCharType(pDoc,pSeg,
                        pDoc->TBCursor.usSegOffset);
  if (  (usType == PROTECTED_CHAR) || (usType == COMPACT_CHAR))
  {
    WinAlarm( HWND_DESKTOP, WA_WARNING ); // issue a beep if trying on R/o doc
    pFindData ->usFirstCall = CH_MATCHOUTRANGE;
  }
  else
  {
    if (pDoc->pTMMaint != NULL )        // if from TMEdit: avoid EQFBSaveSeg
    {
      if (pDoc->EQFBFlags.PostEdit &&
          (pDoc->ulWorkSeg != pDoc->TBCursor.ulSegNum)  )
      {
        EQFBWorkSegOut(pDoc);
        EQFBWorkSegIn(pDoc);
      } /* endif */
    }
    else
    {
      EQFBWorkSegCheck( pDoc );                 //check if segment changed
    } /* endif */
    ulTlen = UTF16strlenCHAR(pFindData->chFindTarget);
    ulRlen = UTF16strlenCHAR(pFindData->chReplace);
    pSegData = pDoc->pEQFBWorkSegmentW;

    if (UTF16strlenCHAR(pSegData) - ulTlen + ulRlen >= MAX_SEGMENT_SIZE)
    {
      UtlError( TB_TOOLONG, MB_CANCEL, 0, NULL, EQF_WARNING);
      sRc = WARN_TOOLONG;
    }
    else
    {
      if (IsDBCS_CP(pDoc->ulOemCodePage) )
      {
        USHORT i = 0;
        PSZ_W  pStart = pSegData + pDoc->TBCursor.usSegOffset;
        ULONG ulLFOffset = 0L;
        while (i < ulTlen )
        {
          if ( (*pStart == SOFTLF_CHAR || *pStart == LF) &&
               (pFindData->chFindTarget[i] != BLANK) )
          {
            ulLFOffset ++;
            pStart++;
            pDoc->Redraw |= REDRAW_BELOW;      // we get rid of an LF
            if ( pDoc->pTBSeg )
            {
              UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusBPET) ,0L ,0L , NOMSG);
              if (pDoc->pTBSeg->pusHLType )
              {
                pDoc->pTBSeg->SegFlags.Spellchecked = FALSE;
                UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusHLType) ,0L ,0L , NOMSG);
              } /* endif */
            } /* endif */
          }
          else
          {
            i++;
            pStart++;
          } /* endif */
        } /* endwhile */
        ulTlen += ulLFOffset;
      } /* endif */

      EQFBWorkLeft(pDoc, pDoc->TBCursor.usSegOffset, (USHORT)ulTlen);
      EQFBWorkRight(pDoc, pDoc->TBCursor.usSegOffset, (USHORT)ulRlen);
      UTF16strncpy( pSegData+(pDoc->TBCursor.usSegOffset),
               pFindData->chReplace, (USHORT) ulRlen );
      pDoc->Redraw |= REDRAW_LINE;
      EQFBUpdateChangedSeg(pDoc);      //update segment ids
      // reset any old block mark in same segment
      EQFBFuncResetMarkInSeg( pDoc );
      if (pDoc->pTMMaint != NULL )    // for TMEdit only:
      {
        pSeg->SegFlags.Marked = TRUE; // set bookmark to changed segment
      } /* endif */

    } /* endif */
  } /*endif*/
  return sRc;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBInRange                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBInRange(PTBDOCUMENT,PFINDDATA,USHORT)                |
//+----------------------------------------------------------------------------+
//|Description:       test if in range for changing                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc    ptr to document instance data        |
//|                   PFINDDATA  pFindData ptr to pFindData                    |
//|                   USHORT segment number to be tested                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    if in the range for changing                     |
//|                   FALSE   do not change, not in range                      |
//+----------------------------------------------------------------------------+
//|Function flow:     - check pFindData->usRange                               |
//|                       case CURRENTTOEND: if translated: ok                 |
//|                       case SEGMENTONLY: if translated and                  |
//|                                             workseg = actseg               |
//|                                             ok                             |
//|                       default : false                                      |
//+----------------------------------------------------------------------------+

BOOL EQFBInRange
(
  PTBDOCUMENT pDoc,           //ptr to doc instance
  PFINDDATA   pFindData,      // finddata structure
  ULONG       ulSegNum
)

{
  BOOL fInRange = FALSE;     // TRUE if in range
  PTBSEGMENT  pSeg;          // pointer to active seg

  switch ( pFindData->usRange )
  {
  case CHANGE_CURRENTTOEND:                     //only possib. in postedit
    pSeg =  EQFBGetSegW( pDoc, ulSegNum );       // get current segment
    if (pSeg->qStatus == QF_XLATED)
    {
      fInRange = TRUE;
    } /*endif*/
    break;
  case CHANGE_SEGMENTONLY:
    pSeg =  EQFBGetSegW( pDoc,ulSegNum );              // get current segment
    if ( !pDoc->EQFBFlags.PostEdit )             //only in active segment
    {
      if ( ulSegNum == pDoc->tbActSeg.ulSegNum )
      {
        fInRange = TRUE;
      } /* endif */
    }
    else                                        // in postedit
    {
      if ( (ulSegNum == pFindData->ulStartSegNum )
           && (pSeg->qStatus == QF_XLATED) )
      {
        fInRange = TRUE;
      } /*endif*/
    } /* endif */
    break;
  case CHANGE_MARKONLY:
    fInRange = TRUE;    //just for now, not correct for later
    break;
  default :
    fInRange = FALSE;
    break;
  } /* endswitch */

  return fInRange;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFindGotoSeg                                          |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFindGotoSeg( PTBDOCUMENT, USHORT, USHORT, USHORT );  |
//+----------------------------------------------------------------------------+
//|Description:       call EQFBGotoSeg and temp.change dispstyle               |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT  pointer to document instance                |
//|                   USHORT       segment to be found                         |
//|                   USHORT       segment offset to position at               |
//|                   USHORT       length of what we found                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
VOID EQFBFindGotoSeg
(
  PTBDOCUMENT  pDoc,                   // pointer to document instance
  ULONG  ulSegNum,                     // segment number to position at
  USHORT usSegOffset,                  // segment offset to position at
  USHORT usLen                         // length of match
)
{
  DISPSTYLE DispStyle;
  if (!pDoc->hwndRichEdit )
  {
    PEQFBBLOCK  pBlock = (PEQFBBLOCK) pDoc->pBlockMark;

    DispStyle = pDoc->DispStyle;
    pDoc->DispStyle = (pDoc->DispStyle == DISP_PROTECTED) ?
                      DISP_UNPROTECTED : pDoc->DispStyle ;

    pDoc->Redraw |= REDRAW_LINE;

    //goto segment , position TBRowOffset and TBCursor
    EQFBGotoSeg(pDoc,ulSegNum,usSegOffset);

    if ((pDoc->docType == SSOURCE_DOC) || (pDoc->docType == STARGET_DOC))
    {
      PTBDOCUMENT pDocTwin;
      pDocTwin = pDoc->twin;
      pDocTwin->Redraw |= REDRAW_LINE;
      EQFBGotoSeg(pDocTwin, ulSegNum, 0);
      if (pDocTwin->Redraw != REDRAW_NONE )
      {
        EQFBScreenData(pDocTwin);
        EQFBScreenCursor(pDocTwin);
      } /* endif */
    } /* endif */
    /****************************************************************/
    /* we will mark match as selected, so that we can see it more   */
    /* visible                                                      */
    /****************************************************************/
    pBlock->pDoc       = pDoc;
    pBlock->ulSegNum   = ulSegNum;
    pBlock->usStart    = usSegOffset;
    if (usSegOffset + usLen > 1 )
    {
      pBlock->usEnd = usSegOffset + usLen - 1;
    }
    else
    {
      pBlock->usEnd = 0;
    } /* endif */
    pBlock->ulEndSegNum = ulSegNum;
    pDoc->DispStyle = DispStyle;
  }
  else
  {
    EQFBFindGotoSegRTF( pDoc, ulSegNum, usSegOffset, usLen );
  } /* endif */

  return;
} /* end of function EQFBFindGotoSeg */

/*////////////////////////////////////////////////////////////////////////////
:H2.EQFBFINDDLGPROC - dialog procedure for find dialog
*/
// Description:
//    Accepts find text, change text and find options using a dialog panel.
//
//   Flow (message driven):
//       case WM_INITDLG:
//         call EQFBFindInit to initialize the dialog controls;
//       case WM_COMMAND
//         call EQFBFindCommand to handle user commands;
//       case WM_CLOSE
//         call EQFBFindClose to end the dialog;
//
// Arguments:
//  mp2 of WM_INITDLG msg = PFINDDATA pFindData ptr to data structure containing
//                          last used values.
//
// Returns:
//  USHORT usRC      0              - dialog was cancelled or closed
//                   FIND           - user pressed 'Find' button
//                   CHANGEFIND     - user pressed 'Change then find' button
//                   CHANGEALL      - user pressed 'Change all' button
//                   FINDALL        - user pressed 'Find all' button
//
// Prereqs:
//   None
//
// SideEffects:
//   if usRC <> 0 is returned, the FINDDATA structure contains the selections
//   made by the user
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK EQFBFINDDLGPROC
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure

  switch ( msg )
  {
  case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_FIND_DLG, mp2 ); break;

  case WM_INITDLG:
    SETWINDOWID( hwndDlg, ID_TB_FIND_DLG );
    mResult = DIALOGINITRETURN( EQFBFindInit( hwndDlg, mp1, mp2 ));
    break;

  case WM_COMMAND:
    mResult = EQFBFindCommand( hwndDlg, mp1, mp2 );
    break;

  case WM_CLOSE:
    mResult = EQFBFindClose( hwndDlg, mp1, mp2 );
    break;

  default:
    mResult = FALSE;
    break;
  } /* endswitch */

  return mResult;
} /* end of EQFBFindDlgProc */

MRESULT APIENTRY EQFBFINDDLGPROCMODELESS
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure

  switch ( msg )
  {
  case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_FIND_DLG, mp2 ); break;

  case WM_INITDLG:
    SETWINDOWID( hwndDlg, ID_TB_FIND_DLG );
    mResult = DIALOGINITRETURN( EQFBFindInit( hwndDlg, mp1, mp2 ));
    /*******************************************************************/
    /* register modeless dialog                                        */
    /*******************************************************************/
    UtlRegisterModelessDlg( hwndDlg );
    break;

  case WM_COMMAND:
    mResult = EQFBFindCommand( hwndDlg, mp1, mp2 );
    break;

  case WM_CLOSE:
    mResult = EQFBFindCloseModeless( hwndDlg, mp1, mp2 );
    break;

 case WM_ACTIVATE:
    if ( mp1 )
    {
      PFINDDATA  pFindData;                    // pointer to find data
      PTBDOCUMENT  pDoc;                       // pointer to document
      pFindData = ACCESSDLGIDA(hwndDlg, PFINDDATA );
      pDoc = pFindData->pDoc;
      if (!pDoc->hwndRichEdit )
      {
        pFindData->usFirstCall = CH_FINDFIRST;   // we dont know where cursor is
      } /* endif */
      pFindData->ulFirstSegNum = pFindData->pDoc->TBCursor.ulSegNum;
      pFindData->usFirstSegOffset = pFindData->pDoc->TBCursor.usSegOffset;

      // reset any old block mark in same segment
      EQFBFuncResetMarkInSeg( pDoc );
      EQFBFindButtons( pDoc, hwndDlg, TRUE );
      /***********************************************************/
      /* bring parent of find dialog on top...                   */
      /***********************************************************/
      BringWindowToTop( pDoc->hwndFrame );
      BringWindowToTop( hwndDlg );
      WinPostMsg( hwndDlg, WM_EQF_SETFOCUS,
                  0, MP2FROMHWND( WinWindowFromID(hwndDlg, ID_TB_FIND_FIND_EF)));

    } /* endif */
    mResult = FALSE;
    break;
  case WM_EQF_SETFOCUS:
    /*************************************************************/
    /* set focus to entry field and select it to ease-up working */
    /* after reactivation...                                     */
    /*************************************************************/
    SETFOCUS( hwndDlg, ID_TB_FIND_FIND_EF );
    SETEFSEL( hwndDlg, ID_TB_FIND_FIND_EF, 0, -1 );
    break;
  case WM_DESTROY:
    UtlUnregisterModelessDlg( hwndDlg );
    break;

  default:
    mResult = FALSE;
    break;
  } /* endswitch */

  return mResult;
} /* end of EQFBFindDlgProcModeless */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFindInit - initialization for find dialog
*/
// Description:
//    Initialize all dialog controls and allocate required memory.
//
//   Flow:
//      - allocate and anchor dialog IDA;
//      - fill find and change text from last used values
//      - set radio buttons accordingly
//
// Arguments:
//  mp2 of WM_INITDLG msg = PFINDDATA pFindData ptr to data structure containing
//                          last used values.
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - ptr to IDA is stored in dialog word QWL_USER
//
// External references:
//   UtlAlloc
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBFindInit
(
HWND    hwndDlg,                    // handle of dialog window
WPARAM  mp1,                        // first parameter of WM_INITDLG
LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  MRESULT     mResult = FALSE;        // result of message processing
  PFINDDATA   pFindData;              // pointer to ida for find init
  PTBDOCUMENT pDoc;                   // pointer to document
  USHORT      usId;                   // user id
  PTBSEGMENT  pSeg;
  BOOL        fAnsiToOEM = TRUE;
  USHORT      xPos = 100;              // intial dialog position 
  USHORT      yPos = 50;
  mp1 = mp1;                          // suppress 'unreferenced parameter' msg

  pFindData = (PFINDDATA) mp2;
  if ( pFindData )
  {
    EQFINFO     ErrorInfo;              // error code of property handler calls
    PPROPFOLDERLIST pFllProp = NULL;    // ptr to folder list properties
    PVOID       hFllProp;               // handle of folder list properties
    OBJNAME     szFllObjName;           // buffer for folder list object name

    ANCHORDLGIDA( hwndDlg, pFindData );
    pDoc = pFindData->pDoc ;            // point to active document
    fAnsiToOEM = CheckForAnsiConv(pDoc);

    //
    // open folder list properties
    //
    UtlMakeEQFPath( (PSZ)szFllObjName, NULC, SYSTEM_PATH, NULL );
    strcat( (PSZ)szFllObjName, BACKSLASH_STR );
    strcat( (PSZ)szFllObjName, DEFAULT_FOLDERLIST_NAME );
    hFllProp = OpenProperties( (PSZ) szFllObjName, NULL, PROP_ACCESS_READ,
                               &ErrorInfo );
    if ( hFllProp )
    {
      pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFllProp );

    }                                  //end if

    // preset Find Entry field and select it
    SETTEXTLIMIT( hwndDlg, ID_TB_FIND_FIND_EF, MAX_FINDCHANGE_LEN );
    //
    // Fill ID_TB_FIND_FIND_EF
    //

    if ( pFindData->chFind[0] != EOS )
    {
      CBDELETEALL( hwndDlg, ID_TB_FIND_FIND_EF );
      CBINSERTITEMENDW( hwndDlg,ID_TB_FIND_FIND_EF , pFindData->chFind );
      CBSELECTITEM (hwndDlg, ID_TB_FIND_FIND_EF , 0 );
    } /* endif */
    //
    // Insert History into ID_TB_FIND_FIND_EF
    //
    if (pFllProp)
    {
      int i=0;

      while (i < MAX_SEARCH_HIST && pFllProp->szFindList[i][0] != EOS)
      {
        SHORT sItem = CBSEARCHITEMW(hwndDlg, ID_TB_FIND_FIND_EF ,
                                   pFllProp->szFindList[i]);

        if (sItem == LIT_NONE)
        {
          CBINSERTITEMENDW( hwndDlg,ID_TB_FIND_FIND_EF , pFllProp->szFindList[i]);
        }
        i++;
      } /* endwhile */

      // get last use dialog position (if available)
      if ( pFllProp->swpTEnvFindSizePos.x || pFllProp->swpTEnvFindSizePos.y )
      {
        // get client rectange of our parent window
        RECT rectParent;
        GetClientRect( pFindData->pDoc->hwndClient, &rectParent );

        // use saved values only if valid
        if ( ((pFllProp->swpTEnvFindSizePos.x + pFllProp->swpTEnvFindSizePos.cx) < rectParent.right) && 
             ((pFllProp->swpTEnvFindSizePos.y + 20) > rectParent.bottom) )
        {
          xPos = pFllProp->swpTEnvFindSizePos.x;
          yPos = pFllProp->swpTEnvFindSizePos.y;
        } /* endif */
      } /* endif */
    } /* endif */

    // preset Change Entry field and select it
    SETTEXTLIMIT( hwndDlg, ID_TB_FIND_CHANGE_EF, MAX_FINDCHANGE_LEN );
    if ( (pFindData->pDoc->docType == STARGET_DOC)
        || (pFindData->pDoc->docType == VISTGT_DOC)
        || (pFindData->pDoc->docType == VISSRC_DOC) )
    {
      //
      // Fill  ID_TB_FIND_CHANGE_EF
      //

      if ( pFindData->chReplace[0] != EOS )
      {
        CBDELETEALL( hwndDlg, ID_TB_FIND_CHANGE_EF );
        CBINSERTITEMENDW( hwndDlg,ID_TB_FIND_CHANGE_EF , pFindData->chReplace );
        CBSELECTITEM (hwndDlg, ID_TB_FIND_CHANGE_EF  , 0 );
      } /* endif */

      //
      // Insert History into ID_TB_FIND_CHANGE_EF
      //

      if (pFllProp)
      {
        int i=0;

        while (i < MAX_SEARCH_HIST && pFllProp->szReplaceList[i][0] != EOS)
        {
          SHORT sItem = CBSEARCHITEMW(hwndDlg, ID_TB_FIND_CHANGE_EF ,
                                     pFllProp->szReplaceList[i]);

          if (sItem == LIT_NONE)
          {
            CBINSERTITEMENDW( hwndDlg,ID_TB_FIND_CHANGE_EF , pFllProp->szReplaceList[i]);
          }
          i++;
        }// end while
      }// end if
    }
    else
    {
      /**************************************************************/
      /* disable change entry field if not in target document       */
      /**************************************************************/
      ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_EF, FALSE );
      ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_TXT,FALSE );
    } /* endif */

    SetCtrlFnt (hwndDlg, pFindData->pDoc->lf.lfCharSet,
                ID_TB_FIND_CHANGE_EF, ID_TB_FIND_FIND_EF );

    // set init for 1st call
    pFindData->usFirstCall = CH_FINDFIRST;
    if ( IS_RTL_ARABIC( pFindData->pDoc ))
    {
    pFindData->fIgnoreCase = FALSE;     // for Arabic, ignorecase -search should be handled as respect-case
    }
    usId = (pFindData->fIgnoreCase) ? ID_TB_FIND_IGNORE_RB :
           ID_TB_FIND_RESPECT_RB;
    CLICK( hwndDlg, usId );
    usId = (pFindData->fForward) ? ID_TB_FIND_FORWARD_RB :
           ID_TB_FIND_BACKWARD_RB;
    CLICK( hwndDlg, usId );

    SETCHECK_TRUE( hwndDlg, ID_TB_FIND_CONFCHANGES_CHK);

    // if not in post-edit disable CURRENTTOEND and MARKEDBLOCK
    pDoc = pFindData->pDoc ;            // point to active document

    pFindData->ulFirstSegNum = pDoc->TBCursor.ulSegNum;
    pFindData->usFirstSegOffset = pDoc->TBCursor.usSegOffset;

    if ( IS_RTL_ARABIC( pFindData->pDoc ))
    {
     ENABLECTRL( hwndDlg, ID_TB_FIND_IGNORE_RB, FALSE );  // Arabic: only respect-case is allowed
    }
    if ( ! pDoc->EQFBFlags.PostEdit )
    {
      ENABLECTRL( hwndDlg, ID_TB_FIND_CURPOSTOEND_RB, FALSE );
      ENABLECTRL( hwndDlg, ID_TB_FIND_BLOCK_RB, FALSE );

      if ( pDoc->TBCursor.ulSegNum != pDoc->tbActSeg.ulSegNum )
      {
        ENABLECTRL( hwndDlg, ID_TB_FIND_ELEMENT_RB, FALSE );
        ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_PB, FALSE );
        ENABLECTRL( hwndDlg, ID_TB_FIND_CONFCHANGES_CHK, FALSE );
      }
      else
      {
        CLICK( hwndDlg, ID_TB_FIND_ELEMENT_RB );
      } /* endif */
    }
    else            // if post edit mode
    {
      CLICK( hwndDlg, ID_TB_FIND_CURPOSTOEND_RB );
      pSeg = EQFBGetSegW(pDoc,pDoc->TBCursor.ulSegNum);
      /*****************************************************************/
      /* if segment not translated, 'change segment only' is disabled  */
      /*****************************************************************/
      if ( pSeg->qStatus != QF_XLATED )
      {
        ENABLECTRL( hwndDlg, ID_TB_FIND_ELEMENT_RB, FALSE);
      } /* endif */
    } /* endif */

    /****************************************************************/
    /* store the position of an already marked block -- we have to  */
    /* use it temporarily to highlight items...                     */
    /****************************************************************/
    memcpy( &pFindData->MarkedBlock, pDoc->pBlockMark,
            sizeof( pFindData->MarkedBlock ));


    if (hFllProp)
    {
      CloseProperties( hFllProp, PROP_QUIT, &ErrorInfo );
    }

  } /* endif */

  // position the dialog window
  WinSetWindowPos( hwndDlg, HWND_TOP, xPos, yPos, 0, 0, EQF_SWP_MOVE );

  return( mResult );
} /* end of EQFBFindInit */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFindCommand - process WM_COMMAND messages of find dialog
*/
// Description:
//    Handle WM_COMMAND messages (= pressing of pushbuttons) of
//    find dialog panel.
//
//   Flow (message driven):
//      case 'Find' pushbutton:
//         update callers FINDDATA structure with actual values;
//         post a WM_CLOSE message to dialog, mp1 = FIND;
//      case 'Change' pushbutton:
//         update callers FINDDATA structure with actual values;
//         post a WM_CLOSE messagge to dialog, mp1 = CHANGE;
//      case CANCEL pushbutton or DID_CANCEL (= ESCAPE key):
//         post a WM_CLOSE messgae to dialog, mp1 = 0;
//
// Arguments:
//   SHORT1FROMMP(mp1) = ID of control sending the WM_COMMAND message
//
// Returns:
//  MRESULT(TRUE)  = command is processed
//
// Prereqs:
//   None
//
// SideEffects:
//   - callers FINDDATA structure is updated if required
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBFindCommand
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);     // TRUE = command is processed
  PFINDDATA  pFindData;                    // pointer to find data
  PTBDOCUMENT  pDoc;                       // pointer to document
  HWND       hwndFocus;                    // focus control

  mp2;
  pFindData = ACCESSDLGIDA(hwndDlg, PFINDDATA );
  pDoc = pFindData->pDoc;

  switch ( WMCOMMANDID( mp1, mp2 ) )
  {
  case ID_TB_FIND_HELP_PB:
    mResult = UtlInvokeHelp();
    break;
  case ID_TB_FIND_FIND_PB:              // find selected value
    EQFBFindFill ( hwndDlg, pFindData);// get data from dialog
    hwndFocus = GETFOCUS();
    EQFBFindButtons( pDoc, hwndDlg, FALSE );
    EQFBCmdFind( pDoc, pFindData );
    EQFBScreenData( pDoc );            // display screen
    EQFBScreenCursor( pDoc );          // update cursor and sliders
    UtlDispatch();
    EQFBFindButtons( pDoc, hwndDlg, TRUE );
    SETFOCUSHWND(hwndFocus);
    if (pDoc->fAutoSpellCheck && pDoc->pvSpellData )
	{ // force that thread recalcs pusHLType
	    PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
	    pSpellData->TBFirstLine.ulSegNum = 0;
	    pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
	}
    break;
  case ID_TB_FIND_CHANGE_PB:            // change then find
    EQFBFindFill ( hwndDlg, pFindData);      // get data from dialog
    hwndFocus = GETFOCUS();
    EQFBFindButtons( pDoc, hwndDlg, FALSE );
    EQFBCmdChange(pDoc,pFindData );
    EQFBScreenData( pDoc );
    EQFBScreenCursor( pDoc);
    UtlDispatch();
    EQFBFindButtons( pDoc, hwndDlg, TRUE );
    SETFOCUSHWND(hwndFocus);
    if (pDoc->fAutoSpellCheck && pDoc->pvSpellData )
	{ // force that thread recalcs pusHLType
	    PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
	    pSpellData->TBFirstLine.ulSegNum = 0;
	    pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
	}
    break;

  case ID_TB_FIND_CANCEL_PB:
  case DID_CANCEL:
    EQFBFindFill ( hwndDlg, pFindData);      // get data from dialog
    POSTCLOSE( hwndDlg, FALSE );
    break;

  case ID_TB_FIND_CHANGE_EF:
  case ID_TB_FIND_FIND_EF:
    if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
    {
      ClearIME( hwndDlg );
    } /* endif */
    break;

  default:
    mResult = FALSE;
    break;
  } /* endswitch */

  return( mResult );
} /* end of EQFBFindCommand */


VOID EQFBFindButtons
(
PTBDOCUMENT pDoc,                      // pointer to document
HWND hwndDlg,                          // dialog handle
BOOL fEnable                           // state of the buttons
)
{
  PTBSEGMENT pSeg;
  //toggle arrow /hour glass
  SETCURSOR( (fEnable) ? SPTR_ARROW : SPTR_WAIT );
  ENABLECTRL( hwndDlg, ID_TB_FIND_FIND_PB, fEnable );
  ENABLECTRL( hwndDlg, ID_TB_FIND_CANCEL_PB, fEnable );
  ENABLECTRL( hwndDlg, ID_TB_FIND_HELP_PB, fEnable );
  if ( (pDoc->docType == VISSRC_DOC )
       || ((!pDoc->EQFBFlags.PostEdit)
           && ( pDoc->TBCursor.ulSegNum != pDoc->tbActSeg.ulSegNum )))
  {
    ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_PB, FALSE );
    ENABLECTRL( hwndDlg, ID_TB_FIND_CURPOSTOEND_RB, FALSE );
    ENABLECTRL( hwndDlg, ID_TB_FIND_ELEMENT_RB, FALSE );
    ENABLECTRL( hwndDlg, ID_TB_FIND_CONFCHANGES_CHK, FALSE );
  }
  else
  {
    /******************************************************************/
    /* set buttons etc if postedit or in active segment               */
    /******************************************************************/
    ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_PB, fEnable );
    ENABLECTRL( hwndDlg, ID_TB_FIND_ELEMENT_RB, fEnable );
    ENABLECTRL( hwndDlg, ID_TB_FIND_CONFCHANGES_CHK, fEnable );
  } /* endif */
  if ( pDoc->EQFBFlags.PostEdit)
  {
    pSeg = EQFBGetSegW(pDoc,pDoc->TBCursor.ulSegNum);
    /*****************************************************************/
    /* if segment not translated, 'change segment only' is disabled  */
    /*****************************************************************/
    if ( pSeg->qStatus != QF_XLATED )
    {
      ENABLECTRL( hwndDlg, ID_TB_FIND_ELEMENT_RB, FALSE);
      ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_PB, FALSE );
    } /* endif */
  } /* endif */
  if (pDoc->docType == VISSRC_DOC )
  {
    ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_PB, FALSE );
    ENABLECTRL( hwndDlg, ID_TB_FIND_ELEMENT_RB, FALSE );
    ENABLECTRL( hwndDlg, ID_TB_FIND_CONFCHANGES_CHK, FALSE );
    ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_EF, FALSE );
    ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_TXT,FALSE );
  } /* endif */
  if (pDoc->docType == VISTGT_DOC)
  {
    if ( pDoc->TBCursor.ulSegNum == pDoc->tbActSeg.ulSegNum )
    {
      ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_PB, fEnable );
      ENABLECTRL( hwndDlg, ID_TB_FIND_ELEMENT_RB, fEnable );
      ENABLECTRL( hwndDlg, ID_TB_FIND_CONFCHANGES_CHK, fEnable );
      ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_EF, fEnable );
      ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_TXT,fEnable );
    }
    else
    {
      ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_PB, FALSE );
      ENABLECTRL( hwndDlg, ID_TB_FIND_ELEMENT_RB, FALSE );
      ENABLECTRL( hwndDlg, ID_TB_FIND_CONFCHANGES_CHK, FALSE );
      ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_EF, FALSE );
      ENABLECTRL( hwndDlg, ID_TB_FIND_CHANGE_TXT,FALSE );
    } /* endif */
  } /* endif */
}

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFindFill - fill the FindData structure with currently selected values
*/
// Description:
//    fill the FindData structure with currently selected values
//
//   Flow :
//    get selection of case
//    get selection of direction
//    get range of changes
//    get input from Find field
//    get input from Change field
//
//
// Arguments:
//   HWND            window handle of find dialog
//   PFINDDATA       pointer to find data structure
//
// Return:
//   VOID
//
// Prereqs:
//   None
//
// SideEffects:
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
VOID EQFBFindFill
(
HWND hwndDlg,                       // handle of dialog window
PFINDDATA  pFindData                // pointer to find data structure
)
{
  BOOL   fStatus;                       // last status of radio button
  BOOL   fFindFirst = FALSE;            // not first call yet
  CHAR_W chTextW[MAX_FINDCHANGE_LEN+1];

  //    get selection of case
  fStatus = pFindData->fIgnoreCase;
  pFindData->fIgnoreCase =
  (BOOL) QUERYCHECK( hwndDlg, ID_TB_FIND_IGNORE_RB );
  fFindFirst |= ( fStatus != pFindData->fIgnoreCase);

  //    get selection of direction
  pFindData->fForward =
  (BOOL) QUERYCHECK( hwndDlg, ID_TB_FIND_FORWARD_RB );

  //    get range of changes
  if (  (BOOL) QUERYCHECK( hwndDlg, ID_TB_FIND_CURPOSTOEND_RB ))
  {
    pFindData->usRange = CHANGE_CURRENTTOEND;
  }
  else
  {
    pFindData->usRange = CHANGE_SEGMENTONLY ;
  } /* endif */
  //set segNum at start
  pFindData->ulStartSegNum = pFindData->pDoc->TBCursor.ulSegNum;
  //    get confirmation flag
  pFindData->fConfirmChanges =
  (BOOL) QUERYCHECK( hwndDlg, ID_TB_FIND_CONFCHANGES_CHK );
  //   get input from Change field if in STARGET DOC
  if ( (pFindData->pDoc->docType == STARGET_DOC) ||
       (pFindData->pDoc->docType == VISSRC_DOC)  ||
       (pFindData->pDoc->docType == VISTGT_DOC)  )
  {
    QUERYTEXTW( hwndDlg, ID_TB_FIND_CHANGE_EF, chTextW );

    fFindFirst |= ( UTF16strcmp( pFindData->chReplace, chTextW ) != 0);
    UTF16strcpy( pFindData->chReplace, chTextW );
  }
  //    get input from Find field
  QUERYTEXTW( hwndDlg, ID_TB_FIND_FIND_EF, chTextW );

  fFindFirst |= ( UTF16strcmp( pFindData->chFind, chTextW ) != 0);
  UTF16strcpy( pFindData->chFind, chTextW );


  /**************************************************************/
  /* Save last used values                                      */
  /**************************************************************/
  {
    EQFINFO     ErrorInfo;              // error code of property handler calls
    PPROPFOLDERLIST pFllProp = NULL;    // ptr to folder list properties
    PVOID       hFllProp;               // handle of folder list properties
    OBJNAME     szFllObjName;           // buffer for folder list object name

    UtlMakeEQFPath( (PSZ)szFllObjName, NULC, SYSTEM_PATH, NULL );
    strcat( (PSZ) szFllObjName, BACKSLASH_STR );
    strcat( (PSZ) szFllObjName, DEFAULT_FOLDERLIST_NAME );
    hFllProp = OpenProperties( (PSZ) szFllObjName, NULL, PROP_ACCESS_READ,
                               &ErrorInfo );
    if ( hFllProp )
    {
      if ( SetPropAccess( hFllProp, PROP_ACCESS_WRITE) )
      {
        pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFllProp );

        //
        // Update Search History
        //

        if (pFllProp)
        {

          int iFound=-1;
          int i;

          // is insertion necessary ????
          for (i=0;i<MAX_SEARCH_HIST;i++)
          {
            if (!UTF16strcmp(pFindData->chFind,pFllProp->szFindList[i] )) iFound = i;

          }// end for


          if (iFound>=0)
          {
            for (i=iFound;i<(MAX_SEARCH_HIST-1);i++)
            {
              UTF16strcpy(pFllProp->szFindList[i] , pFllProp->szFindList[i+1]);
            }// end for

          } // end iFound

          for (i=(MAX_SEARCH_HIST-1);i>=1;i--)
          {
            UTF16strcpy(pFllProp->szFindList[i] , pFllProp->szFindList[i-1]);
          }// end for

          UTF16strcpy(pFllProp->szFindList[0] , pFindData->chFind);

        } // end if


        //
        // Update Replace history
        //

        if (pFllProp)
        {

          int iFound=-1;
          int i;

          // is insertion necessary ????
          for (i=0;i<MAX_SEARCH_HIST;i++)
          {
            if (!UTF16strcmp(pFindData->chReplace,pFllProp->szReplaceList[i] )) iFound = i;

          }// end for


          if (iFound>=0)
          {
            for (i=iFound;i<(MAX_SEARCH_HIST-1);i++)
            {
              UTF16strcpy(pFllProp->szReplaceList[i] , pFllProp->szReplaceList[i+1]);
            }// end for

          } // end iFound

          for (i=(MAX_SEARCH_HIST-1);i>=1;i--)
          {
            UTF16strcpy(pFllProp->szReplaceList[i] , pFllProp->szReplaceList[i-1]);
          }// end for

          UTF16strcpy(pFllProp->szReplaceList[0] , pFindData->chReplace);

        } // end if


        SaveProperties( hFllProp, &ErrorInfo);
      } /* endif */

      //
      // Fill ID_TB_FIND_FIND_EF
      //

      if ( pFindData->chFind[0] != EOS )
      {
        CBDELETEALL( hwndDlg, ID_TB_FIND_FIND_EF );
        CBINSERTITEMENDW( hwndDlg,ID_TB_FIND_FIND_EF , pFindData->chFind );
        CBSELECTITEM (hwndDlg, ID_TB_FIND_FIND_EF , 0 );
      } /* endif */


      //
      // Insert History into ID_TB_FIND_FIND_EF
      //

      if (pFllProp)
      {
        int i=0;

        while (i < MAX_SEARCH_HIST && pFllProp->szFindList[i][0] != EOS)
        {
          SHORT sItem = CBSEARCHITEMW(hwndDlg, ID_TB_FIND_FIND_EF ,
                                     pFllProp->szFindList[i]);

          if (sItem == LIT_NONE)
          {
            CBINSERTITEMENDW( hwndDlg,ID_TB_FIND_FIND_EF , pFllProp->szFindList[i]);
          }
          i++;

        }// end while

      }// end if


      //
      // Fill  ID_TB_FIND_CHANGE_EF
      //

      if ( pFindData->chReplace[0] != EOS )
      {
        CBDELETEALL( hwndDlg, ID_TB_FIND_CHANGE_EF );
        CBINSERTITEMENDW( hwndDlg,ID_TB_FIND_CHANGE_EF , pFindData->chReplace );
        CBSELECTITEM (hwndDlg, ID_TB_FIND_CHANGE_EF  , 0 );
      } /* endif */


      //
      // Insert History into ID_TB_FIND_CHANGE_EF
      //

      if (pFllProp)
      {
        int i=0;

        while (i < MAX_SEARCH_HIST && pFllProp->szReplaceList[i][0] != EOS)
        {
          SHORT sItem = CBSEARCHITEMW(hwndDlg, ID_TB_FIND_CHANGE_EF ,
                                     pFllProp->szReplaceList[i]);

          if (sItem == LIT_NONE)
          {
            CBINSERTITEMENDW( hwndDlg,ID_TB_FIND_CHANGE_EF , pFllProp->szReplaceList[i]);
          }
          i++;

        }// end while

      }// end if

      CloseProperties( hFllProp, PROP_QUIT, &ErrorInfo );
    } /* endif */
  } /* endif */


  if ( fFindFirst )                            // reset to first find
  {
    pFindData->usFirstCall = CH_FINDFIRST;    // 1 if 1st call
    pFindData->ulFirstSegNum = pFindData->pDoc->TBCursor.ulSegNum;
    pFindData->usFirstSegOffset = pFindData->pDoc->TBCursor.usSegOffset;
  } /* endif */
  // copy find string
  UTF16strcpy( pFindData->chFindTarget, pFindData->chFind );
}


/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFindClose - process WM_CLOSE messages of find dialog
*/
// Description:
//    Handle WM_CLOSE messages (= dialog termination requests) of
//    find dialog panel.
//
//   Flow:
//      dismiss dialog
//
// Arguments:
//   SHORT1FROMMP(mp1) = type to be returned using WinDismissDlg
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - dialog is removed
//   - caller receives type specifed in mp1
//
// External references:
//   UtlAlloc
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBFindClose
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT mResult = FALSE;
  PFINDDATA  pFindData;                    // pointer to find data
  PTBDOCUMENT  pDoc;                       // pointer to document

  pFindData = ACCESSDLGIDA(hwndDlg, PFINDDATA );
  pDoc = pFindData->pDoc;

  mp1 = mp1;                          // supress 'unreferenced parameter' msg
  mp2 = mp2;                          // supress 'unreferenced parameter' msg

  /****************************************************************/
  /* restore the position of the block and force repain of screen */
  /* -- we might have to update some areas...                     */
  /****************************************************************/
  memcpy( pDoc->pBlockMark, &pFindData->MarkedBlock,
          sizeof( pFindData->MarkedBlock ));

  pDoc->Redraw = REDRAW_ALL;

  DelCtrlFont (hwndDlg, ID_TB_FIND_CHANGE_EF);
  //--- get rid off dialog ---
  DISMISSDLG( hwndDlg, TRUE );
  pFindData->hwndFindDlg = NULLHANDLE;
  pFindData->pDoc = NULL;

  return( mResult );
} /* end of EQFBFindClose */


MRESULT EQFBFindCloseModeless
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT mResult = FALSE;
  PFINDDATA  pFindData;                    // pointer to find data
  PTBDOCUMENT  pDoc;                       // pointer to document

  pFindData = ACCESSDLGIDA(hwndDlg, PFINDDATA );
  pDoc = pFindData->pDoc;

  mp1 = mp1;                          // supress 'unreferenced parameter' msg
  mp2 = mp2;                          // supress 'unreferenced parameter' msg

  /****************************************************************/
  /* restore the position of the block and force repain of screen */
  /* -- we might have to update some areas...                     */
  /****************************************************************/
  if (pDoc )
  {
    if (pDoc->pBlockMark )
    {
      memcpy( pDoc->pBlockMark, &pFindData->MarkedBlock,
              sizeof( pFindData->MarkedBlock ));
    } /* endif */

    pDoc->Redraw = REDRAW_ALL;
  } /* endif */

  DelCtrlFont (hwndDlg, ID_TB_FIND_CHANGE_EF);
  WinSetActiveWindow( HWND_DESKTOP, pDoc->hwndFrame );

  if ( pFindData->hwndFindDlg )
  {
    HWND hwndTemp = pFindData->hwndFindDlg;

    // save current find dialog position
    { 
      EQFINFO     ErrorInfo;       // error code of property handler calls
      PPROPFOLDERLIST pFllProp;    // ptr to folder list properties
      PVOID       hFllProp;        // handle of folder list properties
      OBJNAME     szFllObjName;    // buffer for folder list object name

      UtlMakeEQFPath( szFllObjName, NULC, SYSTEM_PATH, NULL );
      strcat( szFllObjName, BACKSLASH_STR );
      strcat( szFllObjName, DEFAULT_FOLDERLIST_NAME );
      hFllProp = OpenProperties( szFllObjName, NULL, PROP_ACCESS_READ,
                                  &ErrorInfo );
      if ( hFllProp )
      {
        if ( SetPropAccess( hFllProp, PROP_ACCESS_WRITE) )
        {
          pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFllProp );
          UtlSaveWindowPos( pFindData->hwndFindDlg, &(pFllProp->swpTEnvFindSizePos) );
          SaveProperties( hFllProp, &ErrorInfo);
        } /* endif */
        CloseProperties( hFllProp, PROP_QUIT, &ErrorInfo );
      } /* endif */
    }

    /*****************************************************************/
    /* WinDestroyWindow forces change of active window, hence forces */
    /* show/hide sequence of find dialog, which is unnecessary...    */
    /*****************************************************************/
    pFindData->hwndFindDlg = NULLHANDLE;
    WinDestroyWindow( hwndTemp );
  } /* endif */
  pFindData->pDoc = NULL;


  return( mResult );
} /* end of EQFBFindClose */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     StrNLFCmp                                                |
//+----------------------------------------------------------------------------+
//|Function call:     StrNLFCmp                                                |
//+----------------------------------------------------------------------------+
//|Description:       compare w/o regard of lfs                                |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ         pData                                        |
//|                   PSZ         pSearch                                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:               0    - strings are equal up to blanks /lfs       |
//|                   1 / -1       - not equal                                 |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
SHORT StrNLFCmp
(
PSZ_W   pData,
PSZ_W   pSearch
)
{
  SHORT  sRc = 0;                               // init strings are equal
  CHAR_W c, d;
  while (((d = *pSearch) != NULC) && ((c = *pData) != NULC) )
  {
    if (c == d )
    {
      pData ++;
      pSearch ++;
    }
    else if ( c == SOFTLF_CHAR )
    {
      pData ++;
    }
    else if ( c == LF )
    {
      pData ++;
      if (d == BLANK )
      {
        pSearch ++;
      } /* endif */
    }
    else
    {
      sRc = c-d;
      break;
    } /* endif */
  } /* endwhile */
  if (*pSearch )
  {
    sRc = -1;
  } /* endif */
  return sRc;
}

BOOL CheckForAnsiConv
(
        PTBDOCUMENT pDoc
)
{
        BOOL  fAnsiToOEM = TRUE;

        if (pDoc->lf.lfCharSet == THAI_CHARSET )
        {
            fAnsiToOEM = FALSE; // in these cases ANsiToOEM / OEMToANsi will produce wrong results!
        }
        return fAnsiToOEM;
}
