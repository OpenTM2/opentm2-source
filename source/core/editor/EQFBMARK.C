//+----------------------------------------------------------------------------+
//|EQFBMARK.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:                                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       This file contains all functions concerned with marking  |
//|                   a block                                                  |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|  EQFBFuncMarkBlock()   - mark a block                                      |
//|  EQFBFuncMarkSegment() - mark a segment                                    |
//|  EQFBFuncMarkClear()   - unmark any marked block                           |
//|  EQFBFuncMarkDelete()  - delete any marked block                           |
//|  EQFBFuncMarkCopy()    - insert a copy of any marked block                 |
//|  EQFBFuncMarkMove()    - move any marked block                             |
//|  EQFBFuncMarkFind()    - find the first marked block                       |
//|  EQFBFuncMarkLeftCUA   - mark block using CUA keys (shift cursor left)     |
//|  EQFBFuncMarkRightCUA  - mark block using CUA keys (shift cursor right)    |
//|  EQFBFuncMarkUpCUA     - mark block using CUA keys (shift cursor up)       |
//|  EQFBFuncMarkDownCUA   - mark block using CUA keys (shift cursor down)     |
//+----------------------------------------------------------------------------+
#define INCL_BASE
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // public EDITOR API functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqfdoc00.h"             // private document handler include file
#include "EQFTPI.H"                    // Translation Processor priv. include file

extern CHAR chDictLookup[];            // our character word/text table...

static VOID EQFBFuncMarkCopyMove( PTBDOCUMENT, BOOL);  // move/copy a mark
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkBLock                                        |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkBlock(PTBDOCUMENT)                           |
//+----------------------------------------------------------------------------+
//|Description:       mark a block within a segment                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc    pointer to document instance         |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if blockmark exist and isnot in current document         |
//|                     display error message                                  |
//|                   else                                                     |
//|                     if no mark yet                                         |
//|                       set blockmarkfields                                  |
//|                     else if mark in other segment                          |
//|                       clear old blockmark                                  |
//|                       set blockmarkfields                                  |
//|                     else (block in this segment )                          |
//|                      adjust blockmark fields                               |
//|                     adjust end of blockmark pointer if                     |
//|                     current char is DBCS                                   |
//+----------------------------------------------------------------------------+

 void EQFBFuncMarkBlock
 (
   PTBDOCUMENT pDoc                     //pointer to Doc instance
 )
 {
   PEQFBBLOCK  pstBlock;                // pointer to block struct

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;
   /* If a mark already exists it must be in the current document */
   if (pstBlock->pDoc != NULL && pstBlock->pDoc != pDoc)
   {
      UtlError( TB_MARKSET, MB_CANCEL, 0, NULL, EQF_WARNING);
   }
   else if (pstBlock->pDoc == NULL)           // no mark yet
   {
      pstBlock->pDoc     = pDoc;
      pstBlock->ulSegNum = pDoc->TBCursor.ulSegNum;
      pstBlock->usStart  = pDoc->TBCursor.usSegOffset;
      pstBlock->usEnd    = pDoc->TBCursor.usSegOffset;
      pstBlock->ulEndSegNum = pDoc->TBCursor.ulSegNum;
      /******************************************************************/
      /* adjust pstBlock->usEnd if current char is DBCS                 */
      /******************************************************************/
      pDoc->Redraw |= REDRAW_ALL;
   }
   else if ( pstBlock->ulSegNum != pDoc->TBCursor.ulSegNum )
   {
     if (pDoc->TBCursor.ulSegNum <  pstBlock->ulSegNum )
     {
       /***************************************************************/
       /* adjust start of blockmark                                   */
       /***************************************************************/
       pstBlock->ulSegNum = pDoc->TBCursor.ulSegNum;
       pstBlock->usStart  = pDoc->TBCursor.usSegOffset;
     }
     else
     {
       /***************************************************************/
       /* Cursor is right of Blockmark                                */
       /***************************************************************/
       pstBlock->ulEndSegNum = pDoc->TBCursor.ulSegNum;
       pstBlock->usEnd  = pDoc->TBCursor.usSegOffset;
     } /* endif */
     pDoc->Redraw |= REDRAW_ALL;
   }
   else       // cursor in already marked segment
   {                                          // adjust area to mark
      if ( pstBlock->usStart > pDoc->TBCursor.usSegOffset )
      {
         EQFBDBCS2ND(pDoc,FALSE);             //correct to the left if nec
         pstBlock->usStart = pDoc->TBCursor.usSegOffset;
         pstBlock->ulSegNum = pDoc->TBCursor.ulSegNum;
      }
      else
      {
        if ((pDoc->TBCursor.usSegOffset == pstBlock->usStart)
             && (pstBlock->usStart < pstBlock->usEnd)    )
        {
          pstBlock->usStart++;
        }
        else
        {
          LONG lPos = EQFBCurSegFromCursor( pDoc );
          if (lPos <= 0 )
          {
            pstBlock->usEnd = pDoc->TBCursor.usSegOffset-1;
            pstBlock->ulEndSegNum = pDoc->TBCursor.ulSegNum;
          }
          else
          {
            // after the call to EQFBCurSegFromCursor we are one off... (in the case of RichEdit)
            if (pDoc->hwndRichEdit && pDoc->TBCursor.usSegOffset)
                pDoc->TBCursor.usSegOffset--;

            EQFBDBCS2ND(pDoc,TRUE);              //correct to the right if nec

            pstBlock->usEnd   = pDoc->TBCursor.usSegOffset;
            pstBlock->ulEndSegNum = pDoc->TBCursor.ulSegNum;
          } /* endif */
        } /* endif */
      } /* endif */
      pDoc->Redraw |= REDRAW_ALL;
   } /* endif */
   pDoc->lDBCSCursorCol = pDoc->lCursorCol;
   return;
 }

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkSegment                                      |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkSegment(PTBDOCUMENT)                         |
//+----------------------------------------------------------------------------+
//|Description:       mark current segment                                     |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc                                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if mark exists but not in current doc:                   |
//|                     display error message                                  |
//|                   else  if mark in another segment                         |
//|                            display error message                           |
//|                         else set blockmark fields                          |
//+----------------------------------------------------------------------------+

VOID EQFBFuncMarkSegment
(
  PTBDOCUMENT pDoc                     //pointer to Doc instance
)
{
   PTBSEGMENT pSeg;                    // pointer to segment
   PEQFBBLOCK  pstBlock;                         // pointer to block struct

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

   /* If a mark already exists it must be in the current document */
   if ( pstBlock->pDoc != NULL && pstBlock->pDoc != pDoc)
   {
      UtlError( TB_MARKSET, MB_CANCEL, 0, NULL, EQF_WARNING);
   }
   else if ( pstBlock->pDoc == pDoc &&
                  pstBlock->ulSegNum != pDoc->TBCursor.ulSegNum )
   {                                          // mark in another segment
      UtlError( TB_MARKOUTSIDESEG, MB_CANCEL, 0, NULL, EQF_WARNING);
   }
   else
   {                                          // adjust area to mark
      pstBlock->pDoc     = pDoc;
      pstBlock->ulSegNum = pDoc->TBCursor.ulSegNum;
      pstBlock->usStart  = 0;
      pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );
      pstBlock->usEnd    = (USHORT) UTF16strlenCHAR( pSeg->pDataW ) ;
      pstBlock->ulEndSegNum = pDoc->TBCursor.ulSegNum;
      pDoc->Redraw |= REDRAW_ALL;
   } /* endif */

   return;
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkClear                                        |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkClear                                        |
//+----------------------------------------------------------------------------+
//|Description:       clear a previously marked block                          |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc  pointer to document instance           |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if blockmark fields are set:                             |
//|                     reinit them                                            |
//+----------------------------------------------------------------------------+


 void EQFBFuncMarkClear
 (
   PTBDOCUMENT pDoc                     //pointer to Doc instance
 )
 {
   PEQFBBLOCK  pstBlock;                // pointer to block struct
   PTBDOCUMENT pTempDoc;                // temp.ptr to doc with mark

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

   if ( (pstBlock && pstBlock->pDoc != NULL) && (pstBlock->ulSegNum != 0) )
   {
      pTempDoc = pstBlock->pDoc;           //point to doc with mark
      pstBlock->pDoc->Redraw |= REDRAW_ALL;
      memset( pstBlock, 0, sizeof( EQFBBLOCK ));
      EQFBScreenData( pTempDoc );          // display screen
   } /* endif */

   return;
 }

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkDelete                                       |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkDelete(PTBDOCUMENT)                          |
//+----------------------------------------------------------------------------+
//|Description:       delete a previously marked block                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDOc       pointer to document instance      |
//+----------------------------------------------------------------------------+
//|Function flow:     if reflow is allowed:                                    |
//|                     if mark exists and is incurrent segment                |
//|                       if mark contains protected characters                |
//|                         inform user and let him decide                     |
//|                       if ok so far                                         |
//|                         update undo buffer                                 |
//|                         delete marked area                                 |
//|                         call EQFBUpdateChangedSeg                          |
//|                         clear Mark                                         |
//|                     else                                                   |
//|                       beep                                                 |
//|                     endif                                                  |
//|                   else                                                     |
//|                     beep                                                   |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+


 void EQFBFuncMarkDelete
 (
   PTBDOCUMENT pDoc                     //pointer to Doc instance
 )
 {
   PEQFBBLOCK  pstBlock;                // pointer to block struct
   BOOL        fProtected = FALSE;      // no protected characters
   USHORT      usCnt;                   // character count
   USHORT      usResult;                         // return code from UtlError
   BOOL        fReflow = pDoc->EQFBFlags.Reflow;

   pstBlock =(PEQFBBLOCK)pDoc->pBlockMark;
   if (!pDoc->EQFBFlags.Reflow)
   {
	   if ( pstBlock->pDoc != NULL
	              && (pstBlock->ulSegNum == pDoc->ulWorkSeg)
	              && (pstBlock->ulEndSegNum == pDoc->ulWorkSeg )
	      )
	   {
		   fReflow = TRUE;        // if no LF found, reflow allowed
	      // check if mark contains a linefeed
	      usCnt = pstBlock->usStart;
	      while (usCnt <= pstBlock->usEnd && fReflow )
	      {
			  if (EQFBCharType(pDoc,pDoc->pTBSeg,usCnt)== LINEBREAK_CHAR)
			  {
				  fReflow = FALSE;
		      }
		      usCnt++;
	      } /* endwhile */
       }
   }

   if (fReflow)          //del   only if reflow allowed
   {
     // GQ: check last character of selected area, adjust if necessary to avoid accidently deleting the linbreak at the end of the 
     // selected block (fix for P023077)
     if ( (pstBlock->usEnd > pstBlock->usStart) && (pstBlock->ulSegNum == pstBlock->ulEndSegNum) ) 
     {  
			  unsigned int uiType = EQFBCharType(pDoc,pDoc->pTBSeg,pstBlock->usEnd);
			  if ( uiType == LINEBREAK_CHAR)
        {
          pstBlock->usEnd -= 1; 
        } /* endif */
     }

      if ( pstBlock->pDoc != NULL
           && (pstBlock->ulSegNum == pDoc->ulWorkSeg)
           && (pstBlock->ulEndSegNum == pDoc->ulWorkSeg )
           )
      {
         // check if mark contains protected characters
         usCnt =  pstBlock->usStart;
         while (usCnt <= pstBlock->usEnd && !fProtected )
         {
            switch ( EQFBCharType(pDoc,pDoc->pTBSeg,usCnt)  )
            {
               case PROTECTED_CHAR:
               case HIDDEN_CHAR:
               case COMPACT_CHAR:
                  fProtected = TRUE;
                  break;
               default:
                  usCnt ++;                 // try with next character
                  break;
            } /* endswitch */
         } /* endwhile */

        // protected character in word -- inform user and let him decide
        if ( fProtected )
        {
           // have to set pstBlock->pDoc to NULL to avoid to come into this loop again
           // WinMessageBox (aka UtlError) dispatches all messages in the queue under Win2k/WinXP
           PTBDOCUMENT pTempDoc = pstBlock->pDoc;
		   pstBlock->pDoc = NULL;
           usResult = UtlError( TB_DELPROTECT,
                                MB_YESNO | MB_DEFBUTTON2, 0, NULL, EQF_QUERY);
           fProtected = (usResult != MBID_YES);
		   pstBlock->pDoc = pTempDoc;
        } /* endif */


        if ( !fProtected)
        {
          if ( pDoc->pUndoSegW )
          {
             UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);//update undo buffer
             pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
             pDoc->fUndoState = FALSE;                    // not a typing func
          } /* endif */
          // if mark is until end of string add an additional EOS character
          // to work string to avoid problems in EQFBWorkLeft
          if ( ! *(pDoc->pEQFBWorkSegmentW+pstBlock->usEnd) )
          {
             *(pDoc->pEQFBWorkSegmentW+pstBlock->usEnd + 1) = EOS;
          } /* endif */

          EQFBWorkLeft( pDoc, pstBlock->usStart,
                        (USHORT)(pstBlock->usEnd - pstBlock->usStart+1) );

          // add an additional blank if we got an empty string to
          // allow positioning -- only necessary in STANDARD case
          if ( ! *(pDoc->pEQFBWorkSegmentW)  && !pDoc->hwndRichEdit )
          {
             *(pDoc->pEQFBWorkSegmentW) = BLANK;
             *(pDoc->pEQFBWorkSegmentW+1) = EOS;
          } /* endif */

          usCnt = pstBlock->usStart;
          if (usCnt && ( usCnt > UTF16strlenCHAR(pDoc->pEQFBWorkSegmentW)) )
          {
            usCnt = (SHORT) UTF16strlenCHAR(pDoc->pEQFBWorkSegmentW);
          } /* endif */
          pDoc->TBCursor.usSegOffset = usCnt;
          pDoc->TBCursor.ulSegNum = pstBlock->ulSegNum;

//        if (pstBlock->usStart )
//        {
//          pDoc->TBCursor.usSegOffset--;
//        } /* endif */

          EQFBPhysCursorFromSeg( pDoc );             // set cursor at new offset
          EQFBUpdateChangedSeg(pDoc);
          EQFBFuncMarkClear( pDoc );                // clear the marked area
          EQFBScrnLinesFromSeg ( pDoc,              // pointer to doc ida
                                 0,                 // starting row
                                 pDoc->lScrnRows,  // number of rows
                                                    // starting segment
                                 (pDoc->TBRowOffset+1));

          pDoc->Redraw |= REDRAW_ALL;               // force repaint of screen
        } /* endif */
      }
      else
      {
         EQFBFuncNothing( pDoc );           // do a beep
      } /* endif */
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // issue a beep if on no reflow
   } /* endif */

   return;
 }

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkCopy                                         |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkCopy(PTBDOCUMENT)                            |
//+----------------------------------------------------------------------------+
//|Description:       copy previously marked block to current cursor position  |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc      pointer to doc instance            |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if mark exists                                           |
//|                     call EQFBFuncMarkCopyMove                              |
//|                   else                                                     |
//|                     beep                                                   |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+


 void EQFBFuncMarkCopy
 (
   PTBDOCUMENT pDoc                     //pointer to Doc instance
 )

 {
   PEQFBBLOCK  pstBlock;                         // pointer to block struct

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

    if ( pstBlock->pDoc != NULL )
    {
       EQFBFuncMarkCopyMove ( pDoc, FALSE );    // do only a copy
    }
    else
    {
       EQFBFuncNothing( pDoc );
    } /* endif */
    return;
 }

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkMove                                         |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkMove(PTBDOCUMENT)                            |
//+----------------------------------------------------------------------------+
//|Description:       move a previously marked block to the cursor posiiton    |
//|                   only supported within a segment                          |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc   pointer to document instance          |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if mark exists and within same segment                   |
//|                     call EQFBFuncMarkCopyMove                              |
//+----------------------------------------------------------------------------+


 void EQFBFuncMarkMove
 (
   PTBDOCUMENT pDoc                     //pointer to Doc instance
 )
 {
   PEQFBBLOCK  pstBlock;                         // pointer to block struct

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

    if ( pstBlock->pDoc != NULL
          && pstBlock->ulSegNum == pDoc->ulWorkSeg )
    {
       if ( pstBlock->ulSegNum != pDoc->TBCursor.ulSegNum )
       {
          UtlError( TB_MARKMOVENOTVALID , MB_CANCEL, 0, NULL, EQF_WARNING);

       }
       else
       {
         EQFBFuncMarkCopyMove ( pDoc, TRUE );
       } /* endif */
    } /* endif */
    return;
 }

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkLeftCUA                                      |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkLeftCUA                                      |
//+----------------------------------------------------------------------------+
//|Description:       mark area using cursor keys                              |
//|                   (according to the CUA guidelines)                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc   pointer to doc instance               |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     move cursor left                                         |
//|                   mark the block                                           |
//+----------------------------------------------------------------------------+


void  EQFBFuncMarkLeftCUA
(
  PTBDOCUMENT pDoc
)
{
   PEQFBBLOCK  pstBlock;                // pointer to block struct

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

   pDoc->EQFBFlags.InSelection = TRUE;
   EQFBFuncLeft( pDoc );               // move cursor to the left
   pDoc->EQFBFlags.InSelection = FALSE;

   EQFBFuncMarkBlock(pDoc);               // adjust the mark
   // fix anomaly if we toggle marking direction
   if ((pstBlock->usStart == pstBlock->usEnd) && (pstBlock->usStart > pDoc->TBCursor.usSegOffset) )
   {
     pstBlock->usStart= pstBlock->usEnd = pDoc->TBCursor.usSegOffset;
   }
   return;
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkRightCUA                                     |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkRightCUA(PTBDOCUMENT)                        |
//+----------------------------------------------------------------------------+
//|Description:       mark area using cursor keys                              |
//|                   (according to the CUA guidelines)                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc   pointer to doc instance               |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     mark the block                                           |
//|                   move the cursor right                                    |
//+----------------------------------------------------------------------------+

void  EQFBFuncMarkRightCUA
(
  PTBDOCUMENT pDoc
)
{
   PEQFBBLOCK  pstBlock;                         // pointer to block struct
   LONG     lPos;

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

   EQFBFuncMarkBlock(pDoc);              // mark the block
   pDoc->EQFBFlags.InSelection = TRUE;
   EQFBFuncRight( pDoc );              // move cursor to the right
   lPos = EQFBCurSegFromCursor( pDoc );

   if (lPos <= 0 && pDoc->pUserSettings->fCUABksp &&
      ((pDoc->TBRowOffset+pDoc->lCursorRow+2)->ulSegNum > 0 )    )
   {
     /*****************************************************************/
     /* goto first pos in next line if not at end of file             */
     /*****************************************************************/
     EQFBFuncDown(pDoc);
     EQFBFuncStartLine(pDoc);
   } /* endif */
   pDoc->EQFBFlags.InSelection = FALSE;
   return;
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkUpCUA                                        |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkUpCUA(PTBDOCUMENT)                           |
//+----------------------------------------------------------------------------+
//|Description:       mark area using cursor keys                              |
//|                   (according to the CUA guidelines)                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc   pointer to doc instance               |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     move the cursor up one line                              |
//|                   mark the area                                            |
//+----------------------------------------------------------------------------+

void  EQFBFuncMarkUpCUA
(
  PTBDOCUMENT pDoc
)
{
  /********************************************************************/
  /* add startup for blockmark (if nothing active right now)          */
  /********************************************************************/
   if (! ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc)
   {
     EQFBFuncMarkBlock( pDoc );          // mark the block
   }
   pDoc->EQFBFlags.InSelection = TRUE;
   EQFBFuncUp( pDoc );                 // move cursor up
   pDoc->EQFBFlags.InSelection = FALSE;
   EQFBFuncSegMarkBlock( pDoc );          // mark the block
   return;
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkDownCUA                                      |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkDownCUA(PTBDOCUMENT)                         |
//+----------------------------------------------------------------------------+
//|Description:       mark area using cursor keys                              |
//|                   (according to the CUA guidelines)                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc   pointer to doc instance               |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     move the cursor down one line                            |
//|                   mark the area                                            |
//+----------------------------------------------------------------------------+

void  EQFBFuncMarkDownCUA
(
  PTBDOCUMENT pDoc
)
{
  /********************************************************************/
  /* add startup for blockmark (if nothing active right now)          */
  /********************************************************************/
   if (! ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc)
   {
     EQFBFuncMarkBlock( pDoc );          // mark the block
   }
   pDoc->EQFBFlags.InSelection = TRUE;
   EQFBFuncDown( pDoc );               // move cursor down
   pDoc->EQFBFlags.InSelection = FALSE;
   EQFBFuncSegMarkBlock( pDoc );          // mark the block
   return;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkCopyMove                                     |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkCopyMove(PTBDOCUMENT,BOOL)                   |
//+----------------------------------------------------------------------------+
//|Description:       copy or move a marked block to the current cursor pos    |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc   pointer to doc instance               |
//|                   BOOL   fMove  TRUE if move marked block, FALSE if copy   |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     get ptr to marked area                                   |
//|                   get length of worksegment                                |
//|                   update undo buffer                                       |
//|                   if new legnth would be too long:                         |
//|                     display error message                                  |
//|                   else if typing not allowed: do nothing, beep             |
//|                        else - copy marked area in allocated space          |
//|                             - if right of text pad out with zeros          |
//|                             - copy/move marked area to destination         |
//|                             - call EQFBUpdateChangedSeg                    |
//|                             - reset Blockmarkfields                        |
//+----------------------------------------------------------------------------+
static
VOID   EQFBFuncMarkCopyMove
(
   PTBDOCUMENT   pDoc ,                // pointer to document ida
   BOOL          fMove                 // move or copy
)
{
   PTBSEGMENT  pSegMark;               // pointer to segment data
   USHORT  usLength = 0;               // length of area to copy
   LONG    lPos;                       // position offset
   SHORT   sLen;                       // length of the work segment
   PSZ_W   pData;                      // pointer to data
   PSZ_W   pMark;                      // pointer to marked data
   BOOL    fOK  = TRUE;                // success indicator
   PEQFBBLOCK  pstBlock;               // pointer to block struct
   ULONG   ulCurSeg;
   PTBSEGMENT  pSeg;

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;


   lPos = EQFBCurSegFromCursor( pDoc );
   lPos--;                             // adjust position (CR )
   if ( pDoc->pUndoSegW )
   {                                   //update undo buffer
      UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
      pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
      pDoc->fUndoState = FALSE;        // not a typing func
   } /* endif */
                                       // get pointer to marked segment
   pSegMark = EQFBGetSegW( pstBlock->pDoc, pstBlock->ulSegNum );

   if (pstBlock->ulSegNum == pstBlock->ulEndSegNum )
   {
     usLength = pstBlock->usEnd - pstBlock->usStart + 1;
   }
   else
   {
     if (fMove )
     {
       EQFBFuncNothing(pDoc);
       fOK = FALSE;
     }
     else
     {
        ulCurSeg = pstBlock->ulSegNum;
        pSeg = EQFBGetSegW( pstBlock->pDoc, ulCurSeg );
        if (pSeg )
        {
          usLength = pSeg->usLength - pstBlock->usStart + 1;
        } /* endif */
        ulCurSeg++;
        while (ulCurSeg < pstBlock->ulEndSegNum )
        {
          pSeg = EQFBGetSegW( pstBlock->pDoc, ulCurSeg );
          if (pSeg )
          {
            usLength = usLength + pSeg->usLength;
          } /* endif */
          ulCurSeg++;
        } /* endwhile */
        usLength = usLength + pstBlock->usEnd;
     } /* endif */
   } /* endif */

   sLen = (SHORT)UTF16strlenCHAR( pDoc->pEQFBWorkSegmentW );   // length of work segment
   if ( !fOK || (usLength - lPos + sLen >= MAX_SEGMENT_SIZE ))
   {
      UtlError( TB_TOOLONG, MB_CANCEL, 0, NULL, EQF_ERROR);
   }
   else                // copy data and check if blanks to be padded
   {

      if ( ! EQFBFuncCharIn( pDoc ))
      {
         EQFBFuncNothing( pDoc );               // beep
      }
      else
      {
         if ( UtlAlloc((PVOID *) &pMark, 0L,
                        (LONG) max((usLength + 1),MIN_ALLOC), ERROR_STORAGE) )
         {
           pSeg = EQFBGetSegW( pstBlock->pDoc, pstBlock->ulSegNum );
           if (pstBlock->ulSegNum == pstBlock->ulEndSegNum )
           {
             memcpy( pMark, pSeg->pDataW + pstBlock->usStart, usLength);
           }
           else
           {
             PSZ_W     pCurMark;
             memcpy( pMark, pSeg->pDataW + pstBlock->usStart,
                     (pSeg->usLength - pstBlock->usStart + 1)*sizeof(CHAR_W));
             pCurMark = pMark + pSeg->usLength - pstBlock->usStart;
             ulCurSeg = pstBlock->ulSegNum + 1;
             while (ulCurSeg < pstBlock->ulEndSegNum )
             {
               pSeg = EQFBGetSegW(pstBlock->pDoc, ulCurSeg);
               if (pSeg )
               {
                 memcpy(pCurMark, pSeg->pDataW, pSeg->usLength * sizeof(CHAR_W));
                 pCurMark += pSeg->usLength;
               } /* endif */
               ulCurSeg++;
             } /* endwhile */
             pSeg = EQFBGetSegW(pstBlock->pDoc, ulCurSeg);
             if (pSeg )
             {
               memcpy(pCurMark, pSeg->pDataW, pstBlock->usEnd * sizeof(CHAR_W));
             } /* endif */
           } /* endif */

           *(pMark+usLength) = '\0';
           if ( usLength && (pMark[usLength-1] == '\0') )
           {
             usLength --;
           } /* endif */

           if ( fMove )
           {
              if (! ((pDoc->TBCursor.usSegOffset
                                              < pstBlock->usStart)
                       || (pDoc->TBCursor.usSegOffset > pstBlock->usEnd)))
              {
                 UtlError( TB_MARKMOVENOTVALID , MB_CANCEL, 0,
                           NULL, EQF_WARNING);
                 return;                              // skip markmove sequence
              }
              else
              {
                 // get rid of marked area
                 pData = pSegMark->pDataW + pstBlock->usStart;
                 memmove(pData, pData + usLength,
                         (sLen - pstBlock->usStart + 1)*sizeof(CHAR_W));
                                                      // adjust cursor pos
                 if ( pDoc->TBCursor.usSegOffset > pstBlock->usEnd )
                 {
                    pDoc->TBCursor.usSegOffset = pDoc->TBCursor.usSegOffset - usLength;
                 } /* endif */
                 EQFBCompSeg( pDoc->pTBSeg );                //update length of segment
                 EQFBScrnLinesFromSeg
                    ( pDoc,                         // pointer to doc ida
                      0,                            // starting row
                      pDoc->lScrnRows,             // number of rows
                      (pDoc->TBRowOffset+1));       // starting segment

                 EQFBPhysCursorFromSeg ( pDoc );    // update physical curspos
                 pDoc->lDBCSCursorCol = pDoc->lCursorCol;
                 lPos = EQFBCurSegFromCursor( pDoc );
                 lPos--;                             // adjust position (CR )
                 sLen = (SHORT)UTF16strlenCHAR( pDoc->pEQFBWorkSegmentW );   // len of work seg
              } /* endif */
           } /* endif */



           if ( lPos < 0 )
           {
              EQFBFuncPad ( pDoc, lPos);             // pad with blanks
              sLen = (SHORT) UTF16strlenCHAR( pDoc->pEQFBWorkSegmentW );
           } /* endif */
                                              // get some free space
           lPos = pDoc->TBCursor.usSegOffset;
           memmove( pDoc->pEQFBWorkSegmentW + lPos + usLength,
                    pDoc->pEQFBWorkSegmentW + lPos ,
                    (sLen - lPos + 1)*sizeof(CHAR_W));

           memcpy(  pDoc->pEQFBWorkSegmentW + lPos, pMark, usLength*sizeof(CHAR_W));

           UtlAlloc((PVOID *) &pMark, 0L, 0L, NOMSG);   // free space

           EQFBUpdateChangedSeg(pDoc);         // force update of seg info
                                               // force update of screens
           pstBlock->pDoc->Redraw |= REDRAW_ALL;
           pDoc->Redraw |= REDRAW_ALL;

           pstBlock->pDoc = pDoc;          // update marked area
           pstBlock->ulSegNum = pDoc->ulWorkSeg;
           pstBlock->usStart  = (USHORT)lPos;
           pstBlock->usEnd    = pstBlock->usStart + usLength - 1;
           pstBlock->ulEndSegNum = pDoc->ulWorkSeg;  // TO BE CHECKED!!
         } /* endif */
      } /* endif */
   } /* endif */
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkFind                                         |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkFind(PTBDOCUMENT)                            |
//+----------------------------------------------------------------------------+
//|Description:       find a marked block                                      |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT                                              |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if blockmark exists:                                     |
//|                     goto marked segment                                    |
//+----------------------------------------------------------------------------+

 void EQFBFuncMarkFind
 (
   PTBDOCUMENT pDoc                             //pointer to Doc instance
 )
 {
    PTBDOCUMENT  pTempDoc;
    PEQFBBLOCK  pstBlock;                         // pointer to block struct

    pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

   if (pstBlock->pDoc != NULL)
   {
      EQFBGotoSeg( pstBlock->pDoc, pstBlock->ulSegNum,
                   pstBlock->usStart );
      pTempDoc = pDoc;
      while ( pDoc->next != pstBlock->pDoc )
      {
         pDoc = pDoc->next;               // point to next document
         if ( pDoc == pTempDoc )          // just in case ..( to avoid loop..)
         {
            break;
         } /* endif */
      } /* endwhile */
      EQFBFuncNextDoc( pDoc );            // let it do the dirty work
   } /* endif */
   return;
 }

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkNextWord                                     |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkNextWord(PTBDOCUMENT)                        |
//+----------------------------------------------------------------------------+
//|Description:       mark next word too                                       |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc                                         |
//+----------------------------------------------------------------------------+
//|Function flow:     if codepage is not DBCS                                  |
//|                     if blockmark in another doc, utlerror                  |
//|                     else                                                   |
//|                       if no mark exists                                    |
//|                         set start of mark at starting cursor position      |
//|                       endif                                                |
//|                       EQFBFuncNextWord                                     |
//|                       EQFBFuncLeft                                         |
//|                       EQFBFuncMarkRightCUA                                 |
//|                     endif                                                  |
//|                   else                                                     |
//|                     EQFBFuncMarkRightCUA                                   |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

 VOID EQFBFuncMarkNextWord
 (
   PTBDOCUMENT pDoc
 )
 {
   PEQFBBLOCK  pstBlock;               //pointer to block struct
   BOOL        fReduceBlockMark = TRUE;
   PTBSEGMENT  pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );
   PSZ_W       pData;
   USHORT      usOffset;

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;
   if (!IsDBCS_CP(pDoc->ulOemCodePage) )
   {
      /****************************************************************/
      /* if blockmark in another doc, do error handling here          */
      /* (otherwise it is twice, because EQFBFuncMarkBlock is         */
      /* called twice )                                               */
      /****************************************************************/
      if (pstBlock->pDoc != NULL && pstBlock->pDoc != pDoc)
      {
         UtlError( TB_MARKSET, MB_CANCEL, 0, NULL, EQF_WARNING);
      }
      else
      {
         /*****************************************************************/
         /* if no mark exists, start with current cursor position         */
         /* if mark is in another segment, start new mark                 */
         /*****************************************************************/
         if ( pstBlock->pDoc == NULL
              || (pDoc->TBCursor.ulSegNum < pstBlock->ulSegNum)
              || ( pDoc->TBCursor.ulSegNum > pstBlock->ulEndSegNum) )
         {
                                          //if no mark exists, start with
           EQFBFuncMarkBlock(pDoc);              //currrent cursor position
         } /* endif */

         pDoc->EQFBFlags.InSelection = TRUE;
         usOffset = pDoc->TBCursor.usSegOffset;

         if (pstBlock->ulEndSegNum == pDoc->TBCursor.ulSegNum )
         {
           if ( ((pstBlock->usEnd == usOffset)
                    || (pstBlock->usEnd + 1 == usOffset)) )
           {
             fReduceBlockMark = FALSE;
           }
           else
           {
             //check if at end of line
             if (usOffset > pstBlock->usEnd )
             {
               pData = pSeg->pDataW;
               do
               {
                 usOffset--;
               } while ((*(pData + usOffset ) == BLANK)
                         && (usOffset > pstBlock->usEnd) ); /* enddo */
               if ( (*(pData + usOffset) == '\n') || (*(pData + usOffset) == SOFTLF_CHAR) )
               {
                 fReduceBlockMark = FALSE;
               } /* endif */
             } /* endif */
           } /* endif */
         } /* endif */
         EQFBFuncNextWord(pDoc);
         if ( (pDoc->lCursorCol + pDoc->lSideScroll) > 0)
         {
           if ( fReduceBlockMark )
           {
             //reduce blockmark from beginning if currently skipped
             //word has been marked already
             if ((pDoc->TBCursor.usSegOffset > pstBlock->usEnd )
                  && (pDoc->TBCursor.ulSegNum == pstBlock->ulEndSegNum ))
             {
               pstBlock->usStart = pstBlock->usEnd;
               pstBlock->usEnd = pDoc->TBCursor.usSegOffset;
               pstBlock->ulSegNum = pDoc->TBCursor.ulSegNum;
             }
             else
             {
               pstBlock->usStart = pDoc->TBCursor.usSegOffset;
               pstBlock->ulSegNum = pDoc->TBCursor.ulSegNum;
             } /* endif */
             pDoc->Redraw |= REDRAW_ALL;
           }
           else
           {
             EQFBFuncLeft(pDoc);
             EQFBFuncMarkRightCUA(pDoc);
           } /* endif */
         }
         else
         {
           /**********************************************************/
           /* we are at begin of line                                */
           /**********************************************************/
           EQFBFuncUp(pDoc);
           //RJ P018931EQFBFuncEndLine(pDoc);
           EQFBFuncDoEndLine(pDoc, TRUE);
           if (fReduceBlockMark )
           {
             //reduce blockmark from beginning if currently skipped
             //word has been marked already
             if ((pDoc->TBCursor.usSegOffset > pstBlock->usEnd )
                  && (pDoc->TBCursor.ulSegNum == pstBlock->ulEndSegNum ))
             {
               pstBlock->usStart = pstBlock->usEnd;
               pstBlock->usEnd = pDoc->TBCursor.usSegOffset;
               pstBlock->ulSegNum = pDoc->TBCursor.ulSegNum;
             }
             else
             {
               pstBlock->usStart = pDoc->TBCursor.usSegOffset;
               pstBlock->ulSegNum = pDoc->TBCursor.ulSegNum;
             } /* endif */
             pDoc->Redraw |= REDRAW_ALL;
             pDoc->EQFBFlags.InSelection = TRUE;
             EQFBFuncDown(pDoc);
             EQFBFuncStartLine(pDoc);
           }
           else
           {
             EQFBFuncMarkBlock(pDoc);              // mark the block
             pDoc->EQFBFlags.InSelection = TRUE;
             EQFBFuncRight( pDoc );              // move cursor to the right

             if ((pDoc->TBRowOffset+pDoc->lCursorRow+2)->ulSegNum > 0 )
             {
               /*****************************************************************/
               /* goto first pos in next line if not at end of file             */
               /*****************************************************************/
               EQFBFuncDown(pDoc);
               EQFBFuncStartLine(pDoc);
             } /* endif */
           } /* endif */
         } /* endif */
         pDoc->EQFBFlags.InSelection = FALSE;
      } /* endif */
   }
   else
   {
     EQFBFuncMarkRightCUA(pDoc);
   } /* endif */
   return;
 } /* end of function EQFBFuncMarkNextWord */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkPrevWord                                     |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkPrevWord(PTBDOCUMENT)                        |
//+----------------------------------------------------------------------------+
//|Description:       mark previous word too                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc                                         |
//+----------------------------------------------------------------------------+
//|Function flow:     if codepage is not DBCS                                  |
//|                     if blockmark in another doc, utlerror                  |
//|                     else                                                   |
//|                       if no mark exists                                    |
//|                         set start of mark at starting cursor position      |
//|                       endif                                                |
//|                       EQFBFuncPrevWord                                     |
//|                       EQFBFuncMarkBlock                                    |
//|                     endif                                                  |
//|                   else                                                     |
//|                     EQFBFuncMarkLeftCUA                                    |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

 VOID EQFBFuncMarkPrevWord
 (
   PTBDOCUMENT pDoc
 )
 {
   PEQFBBLOCK  pstBlock;               //pointer to block struct

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

   if ( !IsDBCS_CP(pDoc->ulOemCodePage) )
   {
      /****************************************************************/
      /* if blockmark in another doc, do error handling here          */
      /* (otherwise it is twice, because EQFBFuncMarkBlock is         */
      /* called twice )                                               */
      /****************************************************************/
      if (pstBlock->pDoc != NULL && pstBlock->pDoc != pDoc)
      {
         UtlError( TB_MARKSET, MB_CANCEL, 0, NULL, EQF_WARNING);
      }
      else
      {
         if ( pstBlock->pDoc == NULL
              || pstBlock->ulSegNum != pDoc->TBCursor.ulSegNum )
         {
                                          //if no mark exists, start with
           EQFBFuncMarkBlock(pDoc);       //currrent cursor position
         } /* endif */
         pDoc->EQFBFlags.InSelection = TRUE;
         EQFBFuncPrevWord(pDoc);
         if ( pstBlock->usStart < pDoc->TBCursor.usSegOffset )
         {
//            pDoc->TBCursor.usSegOffset--;
            if ( (pDoc->lCursorCol + pDoc->lSideScroll) > 0)
            {
              EQFBFuncLeft(pDoc);
            }
            else
            {
              EQFBFuncUp(pDoc);
              EQFBFuncEndLine(pDoc);
            } /* endif */
            EQFBDBCS2ND(pDoc,FALSE);             //correct to the left if nec
         } /* endif */
         if ((pstBlock->usStart == pDoc->TBCursor.usSegOffset) &&
             (pstBlock->ulSegNum == pDoc->TBCursor.ulSegNum)  )
         {
           if (!( (pDoc->TBCursor.usSegOffset == 0) &&
                  (pDoc->TBCursor.ulSegNum == 1)) )
           {
             /*********************************************************/
             /* clear only if not at start of file                    */
             /*********************************************************/
             EQFBFuncMarkClear(pDoc);         // back at start of blockmark
           } /* endif */
         }
         else
         {
           EQFBFuncMarkBlock(pDoc);
         } /* endif */
         pDoc->EQFBFlags.InSelection = FALSE;
      } /* endif */
   }
   else
   {
     EQFBFuncMarkLeftCUA(pDoc);
   } /* endif */

   return;
 } /* end of function EQFBFuncMarkPrevWord */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncWordDel   - delete current word                  |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncWordDel(PTBDOCUMENT);                            |
//+----------------------------------------------------------------------------+
//|Description:       delete the word the cursor is currently on               |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT   pDoc                                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if DBCS code page                                        |
//|                      beep                                                  |
//|                   else                                                     |
//|                    get cur.Segnum and SegOffset,ptr to segment             |
//|                    if fOK                                                  |
//|                     find word begin/end                                    |
//|                     trailing blanks will be part of the word               |
//|                     if not right of text and if cursor on a word           |
//|                      with protected chars, or on a blank, or word          |
//|                      delimiter, do nothing/inform user                     |
//|                     if still ok                                            |
//|                       update undo buffer                                   |
//|                       delete word                                          |
//|                       set cursor to begin of deleted word                  |
//|                       recalc segment length etc.                           |
//|                     endif                                                  |
//|                     remove blockmark, if in same segment                   |
//|                    endif                                                   |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

VOID EQFBFuncWordDel
(
  PTBDOCUMENT pDoc                       //pointer to Doc instance
)
 {
   USHORT usWordStart = 0;                  //start/end of current word
   USHORT usWordEnd = 0;
   USHORT usCnt;                            //counter for while loop
   PSZ_W   pData;                           // pointer to data
   USHORT  usResult;                        // return value from MsgBox
   PTBSEGMENT pSeg;                         // pointer to segment data
   BOOL   fOK = TRUE;                       //FALSE if cur char is DBCS
   BOOL   fFound = FALSE;
   CHAR_W c;


   /*******************************************************************/
   /* if dbcs, function only beeps                                    */
   /*******************************************************************/
   if ( IsDBCS_CP(pDoc->ulOemCodePage) )
   {
     EQFBFuncNothing(pDoc);
   }
   else
   {
      EQFBCurSegFromCursor(pDoc);
      pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );
      if ( fOK && pSeg )
      {
         PDOCUMENT_IDA  pIdaDoc;
         SHORT          sLanguageId;
         ULONG          ulOemCP = 0L;

         if (pDoc->pstEQFGen)
         {
             pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
             sLanguageId = pIdaDoc->sSrcLanguage;
             ulOemCP = pIdaDoc->ulSrcOemCP;
         }
         else
         {
             sLanguageId = pDoc->sSrcLanguage;
             ulOemCP = pDoc->ulOemCodePage;
         }

         // retrieve all words - even TF_NOLOOKUP and other noise

         fFound = EQFBFindWord(pSeg->pDataW,
                               sLanguageId,
                               pDoc->TBCursor.usSegOffset,
                               &usWordStart,
                               &usWordEnd, ulOemCP, TRUE);
       }
       if ( fOK && fFound )
       {
		 // P016169: LF /punctuation at word end will be cut off the word
		 if (EQFBCharType(pDoc, pDoc->pTBSeg, usWordEnd) == LINEBREAK_CHAR)
		 {
			 usWordEnd --;
	     }
	     c = *(pSeg->pDataW + usWordEnd);
	     if (iswpunct(c) && !iswspace(c))
	     {
			 usWordEnd--;
	     }
         // trailing blanks will be treated as part of the word and deleted, too.
         pData = pSeg->pDataW + usWordEnd + 1;      // pointer to data
         while ( *pData == BLANK )
         {
            pData++;
            usWordEnd++;
         } /* endwhile */

         //if not right of text and if cursor on a word with protected chars,
         // or on a blank or other word delimiter, do nothing
         if (usWordStart <= usWordEnd)
         {
            //check whether word is protected
            usCnt = usWordStart;
            while (usCnt <= usWordEnd &&
                    EQFBCharType(pDoc,pDoc->pTBSeg,usCnt) == UNPROTECTED_CHAR)
            {
               usCnt ++;
            } /* endwhile */

            // protected character in word -- inform user and let him decide
            if ( usCnt <= usWordEnd)
            {
               usResult = UtlError( TB_DELPROTECT,
                                    MB_YESNO | MB_DEFBUTTON2, 0, NULL, EQF_QUERY);
               if (usResult == MBID_YES)
               {
                  usCnt = usWordEnd + 1;
               } /* endif */
            } /* endif */

            if (usCnt > usWordEnd)
            {
               if ( pDoc->pUndoSegW )
               {                                      //update undo buffer
                 UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
                 pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
                 pDoc->fUndoState = FALSE;            // not a typing func
               } /* endif */
               EQFBWorkLeft(pDoc,usWordStart,(USHORT)(usWordEnd - usWordStart+1));
               // set cursor at begin of deleted word
               pDoc->TBCursor.usSegOffset = usWordStart;

               EQFBPhysCursorFromSeg( pDoc );            // set cursor at new offset
               EQFBUpdateChangedSeg (pDoc);
               pDoc->Redraw |= REDRAW_LINE;
            } /* endif */
         }
         else
         {
            EQFBFuncNothing(pDoc);
         } /* endif */

           // remove the block mark if in the same segment
           EQFBFuncResetMarkInSeg( pDoc );
      } /* endif */
   } /* endif */
   return;
 }
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncWordMark   - mark current word                   |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncWordMark(PTBDOCUMENT);                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description  :     mark the word the cursor is currently on                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT       pointer to document instance data      |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if DBCS code page                                        |
//|                      beep                                                  |
//|                   else                                                     |
//|                     if fOK                                                 |
//|                        get TBCursor.SegNum and SegOffset                   |
//|                        get begin/end of current word                       |
//|                        if begin < end                                      |
//|                          if mark in other doc exists                       |
//|                            error message                                   |
//|                          else                                              |
//|                            delete old blockmark (if it exists)             |
//|                            set blackmark to current word                   |
//|                          endif                                             |
//|                        else                                                |
//|                          beep                                              |
//|                        endif                                               |
//|                     endif                                                  |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

 VOID EQFBFuncWordMark
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   PEQFBBLOCK  pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;     // block structure
   USHORT      usWordEnd = 0;                   //start/end of current word
   USHORT      usWordStart = 0;
   BOOL        fOK = TRUE;                      //FALSE if cur char is DBCS
   BOOL        fFound = FALSE;
   PTBSEGMENT  pSeg = NULL;
   DISPSTYLE   DispStyle;
   CHAR_W      c;

   /*******************************************************************/
   /* if dbcs, function only beeps                                    */
   /*******************************************************************/
   if ( IsDBCS_CP(pDoc->ulOemCodePage) )
   {
     EQFBFuncNothing(pDoc);
   }
   else
   {
      EQFBCurSegFromCursor(pDoc);

      pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );
      if ( fOK && pSeg )
      {
         PDOCUMENT_IDA  pIdaDoc;
         SHORT          sLanguageId;
         ULONG          ulOemCP = 0L;
         if (pDoc->pstEQFGen)
         {
              pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
              sLanguageId = pIdaDoc->sSrcLanguage;
              ulOemCP = pIdaDoc->ulSrcOemCP;
         }
         else
         {
              sLanguageId = pDoc->sSrcLanguage;
              ulOemCP = pDoc->ulOemCodePage;
         }

         fFound = EQFBFindWord(pSeg->pDataW,
                                sLanguageId,
                                pDoc->TBCursor.usSegOffset,
                                &usWordStart,
                                &usWordEnd, ulOemCP, FALSE);
      }
      if ( fOK && fFound )
      {//RJ: P016169: cut off ending punctuation from word!
		  c = *(pSeg->pDataW + usWordEnd);
		  if (iswpunct(c) && !iswspace(c))
		  {
		  	 usWordEnd--;
	      }
          // skip protected characters ...
          DispStyle = pDoc->DispStyle;
          pDoc->DispStyle = DISP_PROTECTED;
          while ( usWordStart <= usWordEnd )
          {
            if ( EQFBCharType(pDoc,pDoc->pTBSeg,usWordStart) != UNPROTECTED_CHAR)
            {
              usWordStart++;
            }
            else
            {
              break;
            } /* endif */
          } /* endwhile */
          while ( usWordEnd > usWordStart )
          {
              if ( EQFBCharType(pDoc,pDoc->pTBSeg,usWordEnd) != UNPROTECTED_CHAR)
              {
                usWordEnd--;
              }
              else
              {
                break;
              } /* endif */
          } /* endwhile */

          pDoc->DispStyle = DispStyle;

         if (usWordStart <= usWordEnd)
         {
            pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;
           // If a mark already exists it must be in the current document
           if (pstBlock->pDoc != NULL && pstBlock->pDoc != pDoc)
           {
              UtlError( TB_MARKSET, MB_CANCEL, 0, NULL, EQF_WARNING);
           }
           else
           {
             if (pstBlock->pDoc != NULL)           // no mark yet ??
             {
                pstBlock->pDoc->Redraw |= REDRAW_ALL;    // force repaint
                EQFBFuncMarkClear(pDoc);
             } /* endif */
             // now no blockmark in current doc ( pstBlock->pDoc == NULL)
             pstBlock->pDoc = pDoc;
             pstBlock->ulSegNum = pDoc->TBCursor.ulSegNum;
             pstBlock->usStart = usWordStart;
             pstBlock->usEnd = usWordEnd;
             pstBlock->ulEndSegNum = pDoc->TBCursor.ulSegNum;
             pDoc->Redraw |= REDRAW_LINE;
           } /* endif */
         }
         else
         {
            EQFBFuncNothing(pDoc);
         } /* endif */
      } /* endif */
   } /* endif */

   return;
 }
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncDelTilTag                                        |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncDelTilTag(PTBDOCUMENT)                           |
//+----------------------------------------------------------------------------+
//|Description:       delete until next tag (only in current segment)          |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc    pointer to document instance         |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if reflow allowed:                                       |
//|                     - while not end of segment or char is protected        |
//|                           increase counter                                 |
//|                     - update undo buffer                                   |
//|                     - delete the given no. of chars                        |
//|                     - update segment specifications                        |
//|                   else beep                                                |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

 VOID EQFBFuncDelTilTag
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   BOOL   fOK = TRUE;                     // for end of while loop
   SHORT  sNoBytes = 0;                   // number of bytes to be deleted
   USHORT usSegOffset;                    //
   USHORT usType_Char;                    // type of char (from EQFBCharType)
   PSZ_W  pData;
   BOOL   fTripleNec = FALSE;
   BOOL   fReflow = TRUE;

   // P018311: delete allowec if specified area contains no LF

      EQFBCurSegFromCursor (pDoc);           //get current SegNum + SegOffset
      usSegOffset = pDoc->TBCursor.usSegOffset;
      pData = pDoc->pEQFBWorkSegmentW + usSegOffset;
      // loop til end of area-to-be-deleted
      // (either a tag (hidden or protected) or end of segment)
      while (fOK && fReflow)
      {
       usType_Char = EQFBCharType(pDoc,pDoc->pTBSeg,usSegOffset);
       switch ( usType_Char )
       {
          case COMPACT_CHAR:
          case HIDDEN_CHAR:
          case PROTECTED_CHAR:
             fOK = FALSE;
             break;
          case LINEBREAK_CHAR:
             if (!pDoc->EQFBFlags.Reflow && (*pData != SOFTLF_CHAR))
             {
			   fReflow = FALSE;
		     }
		     else
		     {
			   pData ++;
               sNoBytes ++;
               usSegOffset ++;
             } /* endif */

             break;
          default:
             if (*pData == '\0')
             {
                fOK = FALSE;
             }
             else
             {
               pData ++;
               sNoBytes ++;
               usSegOffset ++;
             } /* endif */
             break;
       } /* endswitch */
      } /* endwhile */
      //if cursor is on a tag, beep
      if (usSegOffset == pDoc->TBCursor.usSegOffset)
      {
         WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if nothing deleted
      }
      else if (!fReflow)
      {
         UtlError( TB_LFCHANGE_NOT_ALLOWED, MB_CANCEL, 0, NULL, EQF_WARNING);
      }
      else
      {
         if ( pDoc->pUndoSegW )
         {                                      //update undo buffer
           UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
           pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
           pDoc->fUndoState = FALSE;            // not a typing func
         } /* endif */
         //don't del \n if previous to \0
         if (*pData =='\0')
         {
            if (EQFBCharType(pDoc,pDoc->pTBSeg,(USHORT)(usSegOffset-1)) == LINEBREAK_CHAR)
            {
               sNoBytes --;
            }                                                 /* @KIT966A */
            else                                              /* @KIT966A */
            {                                                 /* @KIT966A */
              fTripleNec = TRUE;                              /* @KIT966A */
            } /* endif */
         } /* endif */

         EQFBWorkLeft(pDoc,pDoc->TBCursor.usSegOffset,sNoBytes);
         // if segment is empty add a blank.
         if ( *(pDoc->pEQFBWorkSegmentW) == EOS )
         {
           pData = pDoc->pEQFBWorkSegmentW;
           *pData++ = BLANK;
           *pData   = EOS;
           fTripleNec = FALSE;                                /* @KIT966A */
         } /* endif */

         EQFBUpdateChangedSeg(pDoc);

         if ( fTripleNec )                                    /* @KIT966A */
         {                                                    /* @KIT966A */
           EQFBFuncLeft(pDoc);                                /* @KIT966A */
           pDoc->EQFBFlags.EndOfSeg = TRUE;                   /* @KIT966A */
         } /* endif */                                        /* @KIT966A */

         // reset any old block mark in same segment
         EQFBFuncResetMarkInSeg( pDoc );
      } /* endif */

   return;
 }

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncResetMarkInSeg                                   |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncResetMarkInSeg(PTBDOCUMENT)                      |
//+----------------------------------------------------------------------------+
//|Description:       reset mark if in active segment                          |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc   pointer to document instance          |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if mark is in current document and segment:              |
//|                     reset it and force a repaint                           |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

VOID EQFBFuncResetMarkInSeg
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
   PEQFBBLOCK  pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;        // pointer to block struct

   EQFBFuncMarkClear( pDoc );

   // redo blockmark in current segment at character input
   if ( pstBlock->pDoc == pDoc && (pstBlock->ulSegNum == pDoc->ulWorkSeg) )
   {
      pstBlock->pDoc = NULL;                       // reset block mark
      pDoc->Redraw |= REDRAW_ALL;                  // force redraw the screen
   } /* endif */
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncNextWord   - move the cursor to next word        |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncNextWord( PTBDOCUMENT );                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description  :     move the cursor to next word                             |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT       pointer to document instance data      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if DBCS code page                                        |
//|                      beep                                                  |
//|                   else                                                     |
//|                     if not right of text then                              |
//|                       -if already at end of segment and                    |
//|                        next segment follows in this line,                  |
//|                         goto begin of next segment                         |
//|                        else                                                |
//|                         while cursor not at end of line and                |
//|                          character at cursor position <> Blank             |
//|                          and not end of segment                            |
//|                           call EQFBFuncRight                               |
//|                         endwhile                                           |
//|                         while cursor not at end of line and                |
//|                          character at cursor position == Blank             |
//|                          and not at end of segment                         |
//|                           call EQFBFuncRight                               |
//|                         endwhile                                           |
//|                         if at end of segment and not on a linefeed         |
//|                           EQFBFuncRight                                    |
//|                         endif                                              |
//|                        endif                                               |
//|                     endif                                                  |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

 VOID EQFBFuncNextWord
 (
 PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 {
   LONG       lLen;                            // delta position
   USHORT     usChar;                          // character
   PSZ_W      pData;                           // pointer to data
   ULONG      ulSegNum;                        // segment to be used
   PTBSEGMENT pTBSeg;                          // ptr to segment
   USHORT     usNext = BLANK;                  // next character
   USHORT     usPrevious = BLANK;              // previous character
   USHORT     usType_Char;


   /*******************************************************************/
   /* if dbcs, function only beeps                                    */
   /*******************************************************************/
   lLen = EQFBCurSegFromCursor( pDoc );
   //if right of text goto start of next line
   if ( lLen <= 0 )
   {
      /*************************************************************/
      /* go to next line if not at end of document                 */
      /*************************************************************/
      if ( (pDoc->TBRowOffset+pDoc->lCursorRow+2)->ulSegNum > 0 )
      {
        EQFBFuncDown( pDoc );
        EQFBFuncStartLine( pDoc );
      } /* endif */
   }
   else
   {
      ulSegNum = pDoc->TBCursor.ulSegNum;
      pTBSeg = EQFBGetVisSeg(pDoc, &ulSegNum);  // point to start of table
      pData = pTBSeg->pDataW+pDoc->TBCursor.usSegOffset;
      usChar = *pData;
      usNext = *(pData + 1);
      if (pDoc->TBCursor.usSegOffset)
      {
        usPrevious = *(pData - 1);
      } /* endif */
      /*************************************************************/
      /* if already at end of segment, and if next segment follows */
      /* in this line, go on to begin of next segment              */
      /* if not at end of segment, goto next word as usual         */
      /*************************************************************/
      if ( pDoc->TBCursor.usSegOffset == (pTBSeg->usLength-1) )
      {
         if ( usChar != '\n')
         {
           EQFBFuncRight(pDoc);
         } /* endif */
      }
      else
      {
        usType_Char = EQFBCharType(pDoc, pTBSeg, pDoc->TBCursor.usSegOffset);
        /*************************************************************/
        /* loop til min(end of word or end of segment, or            */
        /* change of UNPROTECTED to any kind of PROTECTED/Linebreak  */
        /* or back                                                   */
        /*************************************************************/
        while (usChar != BLANK && !ISLF(usChar, usNext, usPrevious)
              && pDoc->TBCursor.usSegOffset < (pTBSeg->usLength -1)
              && (usType_Char == EQFBCharType(pDoc, pTBSeg,
                                             pDoc->TBCursor.usSegOffset)))
        {
           // remember last segment number and lst offset
           ULONG  ulLastSegNum = pDoc->TBCursor.ulSegNum;
           USHORT usLastOffset = pDoc->TBCursor.usSegOffset;

           EQFBFuncRight(pDoc);

           // break out of loop if something failed and the cursor hasn't been moved
           if ( (ulLastSegNum == pDoc->TBCursor.ulSegNum) && (usLastOffset == pDoc->TBCursor.usSegOffset) )
           {
             break;
           } /* endif */

           ulSegNum = pDoc->TBCursor.ulSegNum;
           pTBSeg = EQFBGetVisSeg(pDoc, &ulSegNum);  // point to start of table
           pData = pTBSeg->pDataW+pDoc->TBCursor.usSegOffset;
           usPrevious = usChar;
           usChar = *pData;
           usNext = *(pData+1);
        } /* endwhile */

        while (!ISLF(usChar, usNext, usPrevious) && usChar == BLANK
               && pDoc->TBCursor.usSegOffset < pTBSeg->usLength )
        {
           // remember last segment number and lst offset
           ULONG  ulLastSegNum = pDoc->TBCursor.ulSegNum;
           USHORT usLastOffset = pDoc->TBCursor.usSegOffset;

           EQFBFuncRight(pDoc);

           // break out of loop if something failed and the cursor hasn't been moved
           if ( (ulLastSegNum == pDoc->TBCursor.ulSegNum) && (usLastOffset == pDoc->TBCursor.usSegOffset) )
           {
             break;
           } /* endif */

           ulSegNum = pDoc->TBCursor.ulSegNum;
           pTBSeg = EQFBGetVisSeg(pDoc, &ulSegNum);  // point to start of table
           pData = pTBSeg->pDataW+pDoc->TBCursor.usSegOffset;
           usPrevious = usChar;
           usChar = *pData;
           usNext = *(pData+1);
           /***********************************************************/
           /* if at end of segment goto 1st pos of next segment       */
           /* or 1 position after last character of line              */
           /***********************************************************/
           if ( ISLF(usChar, usNext, usPrevious) )
           {
              if ( (pDoc->TBRowOffset+pDoc->lCursorRow+2)->ulSegNum > 0 )
              {
                EQFBFuncDown( pDoc );
                EQFBFuncStartLine( pDoc );
                ulSegNum = pDoc->TBCursor.ulSegNum;
                pTBSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
                pData = pTBSeg->pDataW+pDoc->TBCursor.usSegOffset;
                usPrevious = BLANK;
                usChar = *pData;
                usNext = *(pData+1);
              } /* endif */
           } /* endif */
        } /* endwhile */
        /***********************************************************/
        /* if at end of segment goto 1st pos of next segment       */
        /* or 1 position after last character of line              */
        /***********************************************************/
        if ( ISLF(usChar, usNext, usPrevious) )
        {
           if ( (pDoc->TBRowOffset+pDoc->lCursorRow+2)->ulSegNum > 0 )
           {
             EQFBFuncDown( pDoc );
             EQFBFuncStartLine( pDoc );
           } /* endif */
        } /* endif */

      } /* endif */
   } /* endif */
   return;
 }

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncPrevWord   - move the cursor to previous word    |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncPrevWord( PTBDOCUMENT );                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description  :     move the cursor to previous word                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT       pointer to document instance data      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     - if codepage is DBCS                                    |
//|                       beep                                                 |
//|                     else                                                   |
//|                       if right of text then                                |
//|                         move to end of line                                |
//|                       endif                                                |
//|                       if not right of line and not in 1st col              |
//|                         if at begin of segment                             |
//|                          EQFBFuncLeft (goto last char of prev segment)     |
//|                         else                                               |
//|                           loop til end of previous word                    |
//|                            (blank, begin of line and begin of segment      |
//|                             mark the begin of a word)                      |
//|                            (do not skip segment boundery)                  |
//|                         endif                                              |
//|                       endif                                                |
//|                     endif                                                  |
//+----------------------------------------------------------------------------+

 VOID EQFBFuncPrevWord
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 { int pos;
   LONG  lLen;                            // delta position

   PSZ_W   pData;                           // pointer to data
   ULONG   ulSegNum;                        // segment to be used
   PTBSEGMENT pTBSeg;                       // ptr to segment
   BOOL    fShift;                          // true if something done in loop
   USHORT  usType_Char;                     // type returned from EQFBCharType
   USHORT  usStartType_Char;                // type at start of loop

   /*******************************************************************/
   /* if dbcs, function only beeps                                    */
   /*******************************************************************/
   if ( IsDBCS_CP(pDoc->ulOemCodePage) )
   {
     EQFBFuncNothing(pDoc);
   }
   else
   {
      lLen = EQFBCurSegFromCursor( pDoc );
      //if right of text move first to end of line
      if (lLen < 0)
      {
         EQFBFuncEndLine( pDoc );
         lLen = 0;
      } /* endif */

      pos = pDoc->lCursorCol + pDoc->lSideScroll;
      if ( pos == 0 )
      {
        if ( ((pDoc->TBCursor).ulSegNum == 1 )&&
             ((pDoc->TBCursor).usSegOffset == 0)  )
        {
          /************************************************************/
          /* at top of document, do nothing                           */
          /************************************************************/
        }
        else
        {
          EQFBFuncUp(pDoc);
          EQFBFuncEndLine(pDoc);
          pos = pDoc->lCursorCol + pDoc->lSideScroll;
          lLen = EQFBCurSegFromCursor( pDoc );
        } /* endif */
      } /* endif */

      fShift = FALSE;
      if (lLen >=0 && pos >0)
      {
        CHAR_W c;
        /**************************************************************/
        /* go one character to the left..                             */
        /**************************************************************/
        EQFBFuncLeft(pDoc);
        ulSegNum = pDoc->TBCursor.ulSegNum;
        pTBSeg = EQFBGetVisSeg(pDoc, &ulSegNum);  // point to start of table
        pData = pTBSeg->pDataW+pDoc->TBCursor.usSegOffset;
        /**************************************************************/
        /* skip all blanks ..                                         */
        /**************************************************************/
        while ( ((c=*pData) == BLANK) || (c==LF) )
        {
          if ( (pDoc->lCursorCol + pDoc->lSideScroll) > 0)
          {
            EQFBFuncLeft(pDoc);
          }
          else
          {
            /**********************************************************/
            /* we are at begin of line                                */
            /**********************************************************/
            if ( ((pDoc->TBCursor).ulSegNum == 1 )&&
                 ((pDoc->TBCursor).usSegOffset == 0)  )
            {
              /************************************************************/
              /* at top of document, do nothing                           */
              /************************************************************/
              break;
            }
            else
            {
              EQFBFuncUp(pDoc);
              EQFBFuncEndLine(pDoc);
            } /* endif */
          } /* endif */
          ulSegNum = pDoc->TBCursor.ulSegNum;
          pTBSeg = EQFBGetVisSeg(pDoc, &ulSegNum);  // point to start of table
          pData = pTBSeg->pDataW+pDoc->TBCursor.usSegOffset;
        } /* endwhile */
        /**************************************************************/
        /* now skip all previous characters until we find the blank   */
        /* or start of a line                                         */
        /**************************************************************/
        usStartType_Char = EQFBCharType(pDoc, pTBSeg, pDoc->TBCursor.usSegOffset);
        usType_Char = usStartType_Char;
        while ( (*pData != BLANK ) && (usStartType_Char == usType_Char) )
        {
          if ( (pDoc->lCursorCol + pDoc->lSideScroll) > 0)
          {
            EQFBFuncLeft(pDoc);
            ulSegNum = pDoc->TBCursor.ulSegNum;
            pTBSeg = EQFBGetVisSeg(pDoc, &ulSegNum);  // point to start of table
            pData = pTBSeg->pDataW+pDoc->TBCursor.usSegOffset;
            usType_Char = EQFBCharType(pDoc, pTBSeg, pDoc->TBCursor.usSegOffset);
            if ( (*pData == BLANK ) || (usType_Char != usStartType_Char ) )
            {
              /********************************************************/
              /* we've gone one too far                               */
              /********************************************************/
              EQFBFuncRight( pDoc );
              break;
            } /* endif */
          }
          else
          {
            break;
          } /* endif */
        } /* endwhile */
      } /* endif */
   } /* endif */
   return;
 }


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkStartCUA                                     |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkStartCUA    (PTBDOCUMENT)                    |
//+----------------------------------------------------------------------------+
//|Description:       mark segment from current position to start              |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc                                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if mark exists but not in current doc:                   |
//|                     display error message                                  |
//|                   else  if mark in another segment                         |
//|                            display error message                           |
//|                         else set blockmark fields                          |
//+----------------------------------------------------------------------------+

VOID EQFBFuncMarkStartCUA
(
  PTBDOCUMENT pDoc                     //pointer to Doc instance
)
{

   PEQFBBLOCK  pstBlock;                         // pointer to block struct

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

   /* If a mark already exists it must be in the current document */
   if ( pstBlock->pDoc != NULL && pstBlock->pDoc != pDoc)
   {
      UtlError( TB_MARKSET, MB_CANCEL, 0, NULL, EQF_WARNING);
   }
   else
   {
     TBROWOFFSET TBCursor;
     LONG    lSideScroll, lCursorCol;

     TBCursor    = pDoc->TBCursor;
     lSideScroll = pDoc->lSideScroll;
     lCursorCol  = pDoc->lCursorCol;

     if ( pstBlock->pDoc == NULL
          || pstBlock->ulSegNum != pDoc->TBCursor.ulSegNum )
     {
                                      //if no mark exists, start with
       EQFBFuncMarkBlock(pDoc);       //currrent cursor position
     } /* endif */
     pDoc->EQFBFlags.InSelection = TRUE;
     EQFBFuncStartLine(pDoc);
     if ( pDoc->TBCursor.ulSegNum != TBCursor.ulSegNum )
     {
       pDoc->TBCursor = TBCursor;
       pDoc->lSideScroll = lSideScroll;
       pDoc->lCursorCol  = lCursorCol;
//       EQFBFuncStartSeg( pDoc );
       EQFBFuncExactStartSeg( pDoc );
     } /* endif */
     pDoc->EQFBFlags.InSelection = FALSE;
     EQFBFuncMarkBlock(pDoc);
   } /* endif */

   return;
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkEndCUA                                       |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkEndCUA      (PTBDOCUMENT)                    |
//+----------------------------------------------------------------------------+
//|Description:       mark segment from current position to start              |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc                                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if mark exists but not in current doc:                   |
//|                     display error message                                  |
//|                   else  if mark in another segment                         |
//|                            display error message                           |
//|                         else set blockmark fields                          |
//+----------------------------------------------------------------------------+

VOID EQFBFuncMarkEndCUA
(
  PTBDOCUMENT pDoc                     //pointer to Doc instance
)
{

   PEQFBBLOCK  pstBlock;                         // pointer to block struct

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

   /* If a mark already exists it must be in the current document */
   if ( pstBlock->pDoc != NULL && pstBlock->pDoc != pDoc)
   {
      UtlError( TB_MARKSET, MB_CANCEL, 0, NULL, EQF_WARNING);
   }
   else
   {
     TBROWOFFSET TBCursor;
     LONG  lSideScroll, lCursorCol;

     TBCursor    = pDoc->TBCursor;
     lSideScroll = pDoc->lSideScroll;
     lCursorCol  = pDoc->lCursorCol;

     if ( pstBlock->pDoc == NULL
          || pstBlock->ulSegNum != pDoc->TBCursor.ulSegNum )
     {
                                      //if no mark exists, start with
       EQFBFuncMarkBlock(pDoc);       //currrent cursor position
     } /* endif */
     pDoc->EQFBFlags.InSelection = TRUE;
     EQFBFuncEndLine(pDoc);
     if ( TBCursor.ulSegNum != pDoc->TBCursor.ulSegNum )
     {
       pDoc->TBCursor = TBCursor;
       pDoc->lSideScroll = lSideScroll;
       pDoc->lCursorCol  = lCursorCol;
     if (pDoc->hwndRichEdit)
     {
         // Avoid sending enSelChanged message
     BYTE b = pDoc->pDispFileRTF->bRTFFill;
     pDoc->pDispFileRTF->bRTFFill = RTF_FILL;
     EQFBGotoSegRTF( pDoc, TBCursor.ulSegNum, TBCursor.usSegOffset );
     pDoc->pDispFileRTF->bRTFFill = b;
     }
       EQFBFuncEndSeg( pDoc );
     } /* endif */
     pDoc->EQFBFlags.InSelection = FALSE;
     EQFBFuncMarkBlock(pDoc);
   } /* endif */

   return;
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncCaps     - toggle the initial character of word  |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncCaps(PTBDOCUMENT);                               |
//+----------------------------------------------------------------------------+
//|Description:       toggle the first character of the word the cursor is     |
//|                   currently on...                                          |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT   pDoc                                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if DBCS code page                                        |
//|                      beep                                                  |
//|                   else                                                     |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

VOID EQFBFuncCaps
(
  PTBDOCUMENT pDoc                       //pointer to Doc instance
)
 {
   USHORT usWordStart;                       //start/end of current word
   USHORT usWordEnd;
   BOOL   fFound = FALSE;
   PDOCUMENT_IDA  pIdaDoc;
   SHORT          sLanguageId;
   ULONG          ulOemCP = 0L;

   /*******************************************************************/
   /* if dbcs, function only beeps                                    */
   /*******************************************************************/
   if (IsDBCS_CP(pDoc->ulOemCodePage) )
   {
     EQFBFuncNothing(pDoc);
   }
   else
   {
     /*****************************************************************/
     /* allow toggling of first character if cursor is at a blank, or */
     /* a period, etc. following to the word                          */
     /* This is usually the case if copying a dictionary entry....    */
     /*****************************************************************/
     BOOL fAtNoLookUp;
     UCHAR chOEM[2];

     chOEM[0] = (UCHAR)pDoc->pEQFBWorkSegmentW[ pDoc->TBCursor.usSegOffset ];
     chOEM[1] = EOS;

     AnsiToOem( (LPCSTR)chOEM, (LPSTR)chOEM );
     fAtNoLookUp = ((!chDictLookup[ chOEM[0] ] || (chOEM[0]==LF)) && pDoc->TBCursor.usSegOffset);
     if ( fAtNoLookUp )
     {
       pDoc->TBCursor.usSegOffset--;
     } /* endif */
     if (pDoc->pstEQFGen)
     {
       pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
       sLanguageId = pIdaDoc->sSrcLanguage;
       ulOemCP = pIdaDoc->ulSrcOemCP;
     }
     else
     {
         sLanguageId = pDoc->sSrcLanguage;
         ulOemCP = pDoc->ulOemCodePage;

     }
     fFound = EQFBFindWord(pDoc->pEQFBWorkSegmentW,
                            sLanguageId,
                            pDoc->TBCursor.usSegOffset,
                            &usWordStart,
                            &usWordEnd, ulOemCP, FALSE);

     if ( fFound && fAtNoLookUp )
     {
       /***************************************************************/
       /* restore old position...                                     */
       /***************************************************************/
       pDoc->TBCursor.usSegOffset++;
     } /* endif */

     //if not right of text and if cursor on a word with protected chars,
     // or on a blank or other word delimiter, do nothing
     if (fFound && usWordStart <= usWordEnd)
     {
       // skip protected characters ...
       while ( usWordStart <= usWordEnd )
       {
         if ( EQFBCharType(pDoc,pDoc->pTBSeg,usWordStart) != UNPROTECTED_CHAR)
         {
           usWordStart++;
         }
         else
         {
           break;
         } /* endif */
       } /* endwhile */
       if ( usWordStart > usWordEnd )
       {
         /********************************************************/
         /* ignore request ( and beep ) ...                      */
         /********************************************************/
         EQFBFuncNothing( pDoc );
       }
       else
       {
         /********************************************************/
         /* change the character using the UtlLower/UtlUpper     */
         /* function supporting NLS characters....               */
         /********************************************************/
         CHAR_W chText[2];
         CHAR_W c;

         if ( pDoc->pUndoSegW )
         {                                      //update undo buffer
           UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
           pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
           pDoc->fUndoState = FALSE;            // not a typing func
         } /* endif */

         chText[0] = c = pDoc->pEQFBWorkSegmentW[ usWordStart ];
         chText[1] = EOS;

         UtlUpperW( chText );
         if ( chText[0] == c )
         {
           /******************************************************/
           /* it was already upper, so lower it...               */
           /******************************************************/
           UtlLowerW( chText );
         } /* endif */
         pDoc->pEQFBWorkSegmentW[ usWordStart ] = chText[ 0 ];
         pDoc->EQFBFlags.workchng = TRUE;     // segment changed...
         pDoc->Redraw |= REDRAW_LINE;
       } /* endif */
     }
     else
     {
        EQFBFuncNothing(pDoc);
     } /* endif */
   } /* endif */
   return;
 }


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkSegStartCUA                                  |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkSegStartCUA    (PTBDOCUMENT)                 |
//+----------------------------------------------------------------------------+
//|Description:       mark segment from current position to start              |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc                                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if mark exists but not in current doc:                   |
//|                     display error message                                  |
//|                   else  if mark in another segment                         |
//|                            display error message                           |
//|                         else set blockmark fields                          |
//+----------------------------------------------------------------------------+

VOID EQFBFuncMarkSegStartCUA
(
  PTBDOCUMENT pDoc                     //pointer to Doc instance
)
{

   PEQFBBLOCK  pstBlock;                         // pointer to block struct

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

   /* If a mark already exists it must be in the current document */
   if ( pstBlock->pDoc != NULL && pstBlock->pDoc != pDoc)
   {
      UtlError( TB_MARKSET, MB_CANCEL, 0, NULL, EQF_WARNING);
   }
   else
   {
     TBROWOFFSET TBCursor;
     LONG   lSideScroll, lCursorCol;

     TBCursor    = pDoc->TBCursor;
     lSideScroll = pDoc->lSideScroll;
     lCursorCol  = pDoc->lCursorCol;

     if ( pstBlock->pDoc == NULL
          || pstBlock->ulSegNum != pDoc->TBCursor.ulSegNum )
     {
                                      //if no mark exists, start with
       EQFBFuncMarkBlock(pDoc);       //currrent cursor position
     } /* endif */
     pDoc->EQFBFlags.InSelection = TRUE;
     EQFBFuncExactStartSeg(pDoc);
     pDoc->EQFBFlags.InSelection = FALSE;
     EQFBFuncMarkBlock(pDoc);
   } /* endif */

   return;
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncMarkSegEndCUA                                    |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncMarkSegEndCUA   (PTBDOCUMENT)                    |
//+----------------------------------------------------------------------------+
//|Description:       mark segment from current position to start              |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc                                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if mark exists but not in current doc:                   |
//|                     display error message                                  |
//|                   else  if mark in another segment                         |
//|                            display error message                           |
//|                         else set blockmark fields                          |
//+----------------------------------------------------------------------------+

VOID EQFBFuncMarkSegEndCUA
(
  PTBDOCUMENT pDoc                     //pointer to Doc instance
)
{

   PEQFBBLOCK  pstBlock;                         // pointer to block struct

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

   /* If a mark already exists it must be in the current document */
   if ( pstBlock->pDoc != NULL && pstBlock->pDoc != pDoc)
   {
      UtlError( TB_MARKSET, MB_CANCEL, 0, NULL, EQF_WARNING);
   }
   else
   {
     TBROWOFFSET TBCursor;
     LONG  lSideScroll, lCursorCol;

     TBCursor    = pDoc->TBCursor;
     lSideScroll = pDoc->lSideScroll;
     lCursorCol  = pDoc->lCursorCol;

     if ( pstBlock->pDoc == NULL
          || pstBlock->ulSegNum != pDoc->TBCursor.ulSegNum )
     {
                                      //if no mark exists, start with
       EQFBFuncMarkBlock(pDoc);       //currrent cursor position
     } /* endif */
     pDoc->EQFBFlags.InSelection = TRUE;
     EQFBFuncEndSeg(pDoc);
     pDoc->EQFBFlags.InSelection = FALSE;
     EQFBFuncMarkBlock(pDoc);
   } /* endif */

   return;
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncSegMarkBLock                                     |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncSegMarkBlock(PTBDOCUMENT)                        |
//+----------------------------------------------------------------------------+
//|Description:       mark a block within a segment, do not clear mark if      |
//|                   crossing segment border                                  |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc    pointer to document instance         |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if blockmark exist and isnot in current document         |
//|                     display error message                                  |
//|                   else                                                     |
//|                     if no mark yet                                         |
//|                       set blockmarkfields                                  |
//|                     else if cursor in other segment                        |
//|                       set blockmarkfields at start/end of segment          |
//|                     else (block in this segment )                          |
//|                      adjust blockmark fields                               |
//|                     adjust end of blockmark pointer if                     |
//|                     current char is DBCS                                   |
//+----------------------------------------------------------------------------+

 void EQFBFuncSegMarkBlock
 (
   PTBDOCUMENT pDoc                     //pointer to Doc instance
 )
 {
   PEQFBBLOCK  pstBlock;                // pointer to block struct
//   USHORT      usChar;                  //to store current char
//   PTBSEGMENT  pSeg;                    // pointer to segment

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;
   /* If a mark already exists it must be in the current document */
   if (pstBlock->pDoc != NULL && pstBlock->pDoc != pDoc)
   {
      UtlError( TB_MARKSET, MB_CANCEL, 0, NULL, EQF_WARNING);
   }
   else if (pstBlock->pDoc == NULL)           // no mark yet
   {
      pstBlock->pDoc     = pDoc;
      pstBlock->ulSegNum = pDoc->TBCursor.ulSegNum;
      pstBlock->usStart  = pDoc->TBCursor.usSegOffset;
      pstBlock->usEnd    = pDoc->TBCursor.usSegOffset;
      pstBlock->ulEndSegNum = pDoc->TBCursor.ulSegNum;
      /******************************************************************/
      /* adjust pstBlock->usEnd if current char is DBCS                 */
      /******************************************************************/
//      if (IsDBCS_CP(pDoc->ulOemCodePage))                  //if DBCS codepage
//      {
//        pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );
//        usChar = *(pSeg->pDataW + pDoc->TBCursor.usSegOffset);
//        if ( isdbcs1(usChar) == DBCS_1ST )
//        {
//          pstBlock->usEnd ++;
//        } /* endif */
//      } /* endif */
      pDoc->Redraw |= REDRAW_ALL;
   }
   else if ( pstBlock->ulSegNum != pDoc->TBCursor.ulSegNum )
   {
     if (pstBlock->ulSegNum > pDoc->TBCursor.ulSegNum )
     {
       pstBlock->ulSegNum = pDoc->TBCursor.ulSegNum;
       pstBlock->usStart = pDoc->TBCursor.usSegOffset;
     }
     else
     {
       pstBlock->ulEndSegNum = pDoc->TBCursor.ulSegNum;
       pstBlock->usEnd = pDoc->TBCursor.usSegOffset;
     } /* endif */
     pDoc->Redraw |= REDRAW_ALL;
   }
   else
   {                                          // adjust area to mark
      if ( pstBlock->usStart > pDoc->TBCursor.usSegOffset )
      {
         EQFBDBCS2ND(pDoc,FALSE);             //correct to the left if nec
         pstBlock->usStart = pDoc->TBCursor.usSegOffset;
      }
      else
      {
         if (pstBlock->ulEndSegNum == pDoc->TBCursor.ulSegNum )
         {
           EQFBDBCS2ND(pDoc,TRUE);              //correct to the right if nec
           pstBlock->usEnd   = pDoc->TBCursor.usSegOffset;
         } /* endif */

      } /* endif */
      pDoc->Redraw |= REDRAW_ALL;
   } /* endif */
   /******************************************************************/
   /* adjust pstBlock->usEnd if current char is DBCS                 */
   /******************************************************************/
//   if ( _dbcs_cp == DBCS_CP &&
//                    pstBlock->usEnd == pDoc->TBCursor.usSegOffset)
//   {
//      pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );
//      usChar = *(pSeg->pDataW + pDoc->TBCursor.usSegOffset);
//      if ( isdbcs1(usChar) == DBCS_1ST )
//      {
//        pstBlock->usEnd ++;
//      } /* endif */
//   } /* endif */
   pDoc->lDBCSCursorCol = pDoc->lCursorCol;
   return;
 }
