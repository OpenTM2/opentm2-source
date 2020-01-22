/*! \file
	Description: This module contains the routines that implement the editor's functions.
	Each editor function is implemented as a separate routine.
 
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file


#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFB.ID"                // Translation Processor IDs
// Add for R012027 start
#include "SpecialChardlg.h"
// Add end

#define IND_ENDOFSTRING  0             // rc from EQFBLineCheck: \0 found
#define IND_LINEFEED     1             // rc from EQFBLineCheck: \n found
#define IND_SOFTLF       2             // rc from EQFBLineCheck: one \a found

 static VOID EQFBReflow ( PTBDOCUMENT, BOOL); // reflow text via split/insert..
 static VOID EQFBFuncDeleteCharDo ( PTBDOCUMENT, BOOL );
 static VOID EQFBFuncJoinLineDo( PTBDOCUMENT pDoc, BOOL fInsertBlank );
 static USHORT EQFBLineCheck(PTBDOCUMENT);      //same seg in remainder of line?

 static VOID EQFBFuncScrollIncLeft ( PTBDOCUMENT pDoc,   // scroll screen right
                                     SHORT sScroll);     //  x pos
 static VOID EQFBFuncScrollIncRight ( PTBDOCUMENT pDoc,  // scroll screen left
                                      SHORT sDelta);     //  x pos



//#define SPACE_INCREMENT 20           // make 20 spaces
#define SPACE_INCREMENT 0

#define SIDE_DELTA      10           // scroll if end reach 10 characters



#if defined(TPLOGGING)
 #define TPLOGFILE "\\TP.LOG"
 FILE *hTPLog = NULL;
 #define TPLOG()                \
    if ( hTPLog )               \
       fprintf( hTPLog, "TPRO: %s, %d \n",__FILE__, __LINE__ );    \
    else                         \
       DosBeep( 1200, 200 );
#else
 #define TPLOG()
#endif

#if defined(MEASURETIME)
   SEL    selGlobSeg, selLocalSeg;  // selectors returned by DosGetInfoSeg
   GINFOSEG FAR * pInfoSeg;
   ULONG   ulLastTime;
   ULONG   ulInitDBCS = 0L;
   ULONG   ulLoadResource = 0L;
   ULONG   ulLoadFile = 0L;
   ULONG   ulGetSeg = 0L;

#define INITTIME()                    \
{                                     \
   ulLastTime = pInfoSeg->msecs;      \
   ulInitDBCS = 0L;                   \
   ulLoadResource = 0L;               \
   ulLoadFile = 0L;                   \
   ulGetSeg = 0L;                     \
}
// get delta time and store it in supplied variable; reset last time
#define GETTIME( ulTime )                       \
{                                               \
   ulTime += pInfoSeg->msecs - ulLastTime;      \
   ulLastTime = pInfoSeg->msecs;                \
}

#endif

/********************************************************************/
/* get rid of selection during cursor movement if ShiftKey not      */
/* pressed                                                          */
/********************************************************************/
#define RESETSELECTION( pDoc ) \
  if ( (pDoc->ucState & ~ST_SHIFT) && !pDoc->EQFBFlags.InSelection ) \
     EQFBFuncMarkClear( pDoc );
//////////////////////////////////////////////////////////////////////////////
//
// Note: Each EQFBFunc routine is also responsible for setting the
//       EQFBRedraw bits to indicate how much of the screen will need
//       updating.
//       Each EQFBFunc routine is responsible to update TBCursor
//       (Segment Number and Offset of cursor pos)
//


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name: EQFBFuncNothing    -  do nothing
//------------------------------------------------------------------------------
// Function call:     EQFBFuncNothing( PTBDOCUMENT );
//------------------------------------------------------------------------------
// Description:       This function will do nothing - it only issues a beep
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     issue a beep
//------------------------------------------------------------------------------

 VOID EQFBFuncNothing
 (
     PTBDOCUMENT pDoc                         //pointer to doc instance
 )

 {
   /* This is easy enough! */
    pDoc;                                    // get rid of compiler warnings
    WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
   return;
 }



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name: EQFBFuncCharIn - determine if character input allowed
//------------------------------------------------------------------------------
// Function call:     EQFBFuncCharIn( PTBDOCUMENT );
//------------------------------------------------------------------------------
// Description:       checks whether character input is allowed at current
//                    position
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc     pointer to document instance data
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE : input allowed
//                    FALSE: no input allowed
//------------------------------------------------------------------------------
// Function flow:      - check if on protected character,
//                     - if so check if either we are in END_OF_SEG mode or on
//                       first character of inline tag and insert mode is on.
//                       in such a case return true
//------------------------------------------------------------------------------
BOOL EQFBFuncCharIn
(
   PTBDOCUMENT   pDoc                     // pointer to document ida
)
{
   SHORT sLen;                            // length
   BOOL  fNoInput = FALSE;                // success indicator
   USHORT usType_Char;                    // return value of EQFBCharType
   BOOL fTypeIns = FALSE;                        // true if insert before tag
   BOOL  fNoBeep = FALSE;
   USHORT usStatus;

   // check if cursor is already on next segment ( just the case if prev.
   // segment is empty
   usStatus = EQFBCurrentState(pDoc);
   if (usStatus == STARGET )
   {
     sLen = pDoc->pTBSeg->usLength;

     /*****************************************************************/
     /* if insert, and cursor is on 1st char of next seg in same line,*/
     /* and previous seg is the active one, then insert character at  */
     /* end of active segment                                         */
     /*****************************************************************/
     if (sLen && pDoc->EQFBFlags.inserting &&
          (pDoc->TBCursor.ulSegNum == pDoc->ulWorkSeg + 1)  &&
          (pDoc->TBCursor.usSegOffset == 0 ) &&
          ( *(pDoc->pEQFBWorkSegmentW+sLen-1) != LF)  )
     {
       (pDoc->TBCursor).usSegOffset  = sLen - 1;           // end of segment
       pDoc->TBCursor.ulSegNum = pDoc->ulWorkSeg;

       EQFBCheckEndOfSeg(pDoc,pDoc->pTBSeg,pDoc->TBCursor.usSegOffset);
       if (!(pDoc->EQFBFlags.EndOfSeg ))
       {
         fNoInput = TRUE;
       }
       else
       {
         EQFBPhysCursorFromSeg(pDoc);
         pDoc->lDBCSCursorCol = pDoc->lCursorCol;
       } /* endif */
     }
     else
     {
       fNoBeep = TRUE;                 // charin not allowed, but do not BEEP
       fNoInput = TRUE;
     } /* endif */
   }
   else
   {
     if ( pDoc->TBCursor.ulSegNum != pDoc->ulWorkSeg )
     {
        // make space for one character and adjust length
        UTF16strcat( pDoc->pEQFBWorkSegmentW,L" ");
        EQFBCompSeg( pDoc->pTBSeg );
        pDoc->TBCursor.ulSegNum = pDoc->ulWorkSeg ;
        pDoc->Redraw |= REDRAW_LINE;                         // redraw the line
     }
     else
     {
        sLen = pDoc->TBCursor.usSegOffset;
        usType_Char = EQFBCharType(pDoc,pDoc->pTBSeg,sLen);
        fNoInput = (( usType_Char == PROTECTED_CHAR)
                    || (usType_Char == COMPACT_CHAR)
                    || (usType_Char == TRNOTE_CHAR));
        if ( fNoInput )
        {
           fNoInput = ! pDoc->EQFBFlags.EndOfSeg;

           // check if we are on first char of inline tag and in insert mode
           if ( fNoInput )
           {
              fNoInput= ! pDoc->EQFBFlags.inserting;
              if ( sLen > 0 && (pDoc->EQFBFlags.inserting))
              {
                 sLen = sLen - 1;
                 usType_Char = EQFBCharType(pDoc,pDoc->pTBSeg,sLen);
                 switch ( usType_Char )
                 {
					 case LINEBREAK_CHAR:        //P016804: check whether LF is PROTECTED!!
					   fTypeIns = EQFBIsLFProtected(pDoc->pTBSeg, sLen );

					   if (fTypeIns)
					   {
                         UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusBPET) ,0L ,0L , NOMSG);
                         if (pDoc->pTBSeg->pusHLType )
                         {
                            pDoc->pTBSeg->SegFlags.Spellchecked = FALSE;
                            if (!pDoc->fAutoSpellCheck )
                            {
                              UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusHLType),0L,0L,NOMSG);
                            } /* endif */
                          } /* endif */
                       } /* endif */
                       break;
                    case UNPROTECTED_CHAR:            // force reparse
                       UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusBPET) ,0L ,0L , NOMSG);
                       if (pDoc->pTBSeg->pusHLType )
                       {
                         pDoc->pTBSeg->SegFlags.Spellchecked = FALSE;
                         if (!pDoc->fAutoSpellCheck )
                         {
                           UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusHLType),0L,0L,NOMSG);
                         } /* endif */
                       } /* endif */
                       fTypeIns = TRUE;
                       break;
                    case PROTECTED_CHAR:
                       fTypeIns = EQFBDiffTag(pDoc,pDoc->ulWorkSeg,(USHORT)(sLen+1));
                       break;
                     case COMPACT_CHAR:
                     case TRNOTE_CHAR:
                       fTypeIns = FALSE;
                       break;
                    default:
                       break;
                 } /* endswitch */
                 fNoInput= ! (pDoc->EQFBFlags.inserting && fTypeIns);
              } /* endif */
           } /* endif */
        } /* endif */
     } /* endif */
   } /* endif */
   if ( ! fNoInput )                                           /* @KIT920A */
   {                                                           /* @KIT920A */
     pDoc->usNumTyped++;                                       /* @KIT920A */
   } /* endif */                                               /* @KIT920A */
   return ! fNoInput;
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name: EQFBFuncCharacter - type an ASCII character
//------------------------------------------------------------------------------
// Function call:    EQFBFuncCharacter( PTBDOCUMENT );
//------------------------------------------------------------------------------
// Description:      Enters the character into the current line in the current
//                   editing mode (either Insert or Replace or triple-mode).
//------------------------------------------------------------------------------
// Parameters:       PTBDOCUMENT    pointer to document instance data
//------------------------------------------------------------------------------
// Returncode type:  VOID
//------------------------------------------------------------------------------
// Function flow:    - get ptr to segment and position in segment
//                   - if char input is not allowed, beep
//                     else
//                          - If in Insert mode and not beyond the end,
//                              shift characters to the right to make room.
//                          - If beyond the end of the line pad out with blanks
//                          - Add the new character
//                          - if outside of right margin do a line wrap
//                          - move the cursor right
//                     endif
//------------------------------------------------------------------------------
 VOID EQFBFuncCharacter
 (
   PTBDOCUMENT pDoc
 )
 {

   LONG     lPos;                          // length of delta
   SHORT    sLen;                          // length of string
   PSZ_W    pData;                         // pointer to data
   BOOL     fOK = TRUE;                    // success indicator
   PTBSEGMENT pSeg;                        // pointer to segment
   PTBROWOFFSET  pTBRow;                   // pointer to row/offset
   BOOL     fAlloc = FALSE;                // true if Utlalloc nec
   SHORT    sBytesinChar = 1;              // 1 if SBCS, 2 if DBCS
   USHORT   usLocat = 0;                   //where to make space
   USHORT   ABSLOCAT_NOTUSED = 0xFFFF;     // value for "not used" condition
   USHORT   usAbsLocat = ABSLOCAT_NOTUSED; // abs offset in segment
   USHORT   usType;                        //return from EQFBCharType
   DISPSTYLE DispStyle;                    // adjust DispStyle to Expanded
   USHORT   usCurType = UNPROTECTED_CHAR;  //return from EQFBCharTYpe
   USHORT   usSegOffset;
   USHORT   usCType;

   pSeg = pDoc->pTBSeg;
   lPos = EQFBCurSegFromCursor( pDoc );
   lPos--;                                // adjust position (CR )

   if ( (! pDoc->usChar) || pDoc->fImeStartComposition )
   {
     /*****************************************************************/
     /* do nothing                                                    */
     /*****************************************************************/
   }
   else if ( ! EQFBFuncCharIn( pDoc ) )
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // issue a beep if trying on R/o doc
   }
   else
   {
     /*****************************************************************/
     /* if no chars prev.typed, update UndoBuffer (fUndoState=FALSE ) */
     /* if already chars typed in, do not update (fUndoState = TRUE ) */
     /*****************************************************************/
      if ( pDoc->pUndoSegW && !pDoc->fUndoState)
      {                                      //update undo buffer
         UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
         pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
         pDoc->fUndoState = TRUE;
      } /* endif */

      /****************************************************************/
      /* if selection in current segment delete it...                 */
      /****************************************************************/
      if ( EQFBFuncDelSel( pDoc ) )
      {
        lPos = EQFBCurSegFromCursor( pDoc );
        lPos--;                                // adjust position (CR )
      } /* endif */

/**********************************************************************/
/*      If not beyond the end ...                                     */
/**********************************************************************/
      if ( lPos >= 0 )
      {
//       If in Insert mode, shift characters to the right to make room.
//       and pad with blanks as dummies
         if (pDoc->EQFBFlags.inserting || pDoc->EQFBFlags.EndOfSeg)
         {
            // make space either for SBCS character or DBCS character
            if ( pDoc->EQFBFlags.EndOfSeg)   //if triple mode,
            {                                // do not move cur. char
               DispStyle = (pSeg->SegFlags.Expanded) ?
                            DISP_PROTECTED : (pDoc->DispStyle);
               usSegOffset = pDoc->TBCursor.usSegOffset;
               if ( (DispStyle == DISP_COMPACT) &&
                ((usCurType= EQFBCharType(pDoc,pSeg,usSegOffset))
                                  == COMPACT_CHAR) )
               {
                  usType = HIDDEN_CHAR;
                  while (usType == HIDDEN_CHAR  )
                  {
                    usSegOffset++;
                    usType = EQFBCharType(pDoc,pSeg, usSegOffset);
                  } /* endwhile */
                  usLocat = usSegOffset - pDoc->TBCursor.usSegOffset;
               }
               else
               {
                 usCType = SBCS;
                 usLocat = 1;
               } /* endif */
            } /* endif */
            usAbsLocat = pDoc->TBCursor.usSegOffset + usLocat;
            EQFBWorkRight(pDoc, usAbsLocat, sBytesinChar);
            /**********************************************************/
            /* init for security the 'new' space                      */
            /**********************************************************/
            pDoc->pEQFBWorkSegmentW[usAbsLocat] = BLANK;

//            if ( sBytesinChar == 2 )
//            {
//               pDoc->EQFBWorkSegment[usAbsLocat+1] = BLANK;
//            } /* endif */
            pSeg->usLength = pSeg->usLength + sBytesinChar;           // increase length

            // prohibit that the inserted chars ( if endofseg &lastchar prot.)
            // re automatically protected too!

            if (pDoc->EQFBFlags.EndOfSeg )
            {
              fAlloc=( (usCurType == PROTECTED_CHAR) || (usCurType == COMPACT_CHAR));
            } /* endif */

            if (fAlloc)
            {
               UtlAlloc((PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG); // free old seg table

               if (pSeg->pusHLType )
               {
                 pSeg->SegFlags.Spellchecked = FALSE;
                 if (!pDoc->fAutoSpellCheck )
                 {
                   UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusHLType),0L,0L,NOMSG);
                 } /* endif */
               }
            }
            else
            {
               //reparse segment table either for 1 or two characters (SBCS/DBCS)
               EQFBReparse(pDoc, pSeg,pDoc->TBCursor.usSegOffset,sBytesinChar);
            } /* endif */

            // adjust row/offset table
            //  +1 since sCursorRow starts with 0
            //  +1 since next line and following are affected
            pTBRow = pDoc->TBRowOffset + pDoc->lCursorRow + 2;

            while ( pTBRow->ulSegNum == pDoc->TBCursor.ulSegNum )
            {
               pTBRow->usSegOffset = pTBRow->usSegOffset + sBytesinChar; // adjust segment offset

               pTBRow++;                           // pointer to new segment
            } /* endwhile */

            if ( pDoc->EQFBFlags.EndOfSeg )        // end of segment reached.
            {
               EQFBFuncRight( pDoc );              // move into free space
            } /* endif */
            pDoc->Redraw |= REDRAW_LINE;           // redraw the line
         } /* endif */
      } /* endif */
//     If beyond the end of the line pad out with blank
      if ( lPos < 0)
      {
         // in case of DBCS character we have to make space for one additional character
         if (sBytesinChar == 2)
         {
             lPos --;
         }

         /*************************************************************/
         /* if line ends with protected chars check whether next      */
         /* line starts with another protected tag or unprotected     */
         /*************************************************************/
         fOK = EQFBDiffProtectTag(pDoc,pDoc->ulWorkSeg,
                                 pDoc->TBCursor.usSegOffset);
         if ( fOK )
         {
                                                        // get rid of CRLF
           sLen = (USHORT)UTF16strlenCHAR( pDoc->pEQFBWorkSegmentW );   // length of work segment
           // make some more space free
           if ( sLen - lPos + SPACE_INCREMENT < MAX_SEGMENT_SIZE )
           {
              lPos -= SPACE_INCREMENT;
           } /* endif */
           if ( sLen - lPos < MAX_SEGMENT_SIZE )     // enough space ??
           {
              EQFBFuncPad ( pDoc, lPos);             // pad with blanks
                                                     // fill new pos in next line
              EQFBScrnLinesFromSeg ( pDoc,               // pointer to doc ida
                                     pDoc->lCursorRow,   // starting row
                                     (pDoc->lScrnRows -
                                        pDoc->lCursorRow),// number of rows
                                     &(pDoc->TBCursor)); // seg num
           }
           else
           {
                                                     // issue error message
              UtlError( TB_TOOLONG, MB_CANCEL, 0, NULL, EQF_ERROR);
              fOK = FALSE;
           } /* endif */
         }
         else
         {
           WinAlarm( HWND_DESKTOP, WA_WARNING ); // issue a beep
         } /* endif */
      } /* endif */

      if ( fOK )
      {
        // Add the new character and move the cursor right
        if ( (pDoc->EQFBFlags.inserting || pDoc->EQFBFlags.EndOfSeg) && (usAbsLocat != ABSLOCAT_NOTUSED) )
        {
          // GQ: use the position of the newly inserted blank if we are doing and insert
          // an absolute location is available
          pData = &(pDoc->pEQFBWorkSegmentW[usAbsLocat]);
        }
        else
        {
          pData = &(pDoc->pEQFBWorkSegmentW[pDoc->TBCursor.usSegOffset]);
        } /* endif */
        *pData = pDoc->usChar;                     // set character

         EQFBSysDispRestOfLine( pDoc );               // display it

         if (pDoc->usChar == pDoc->usTagEnd)            // force reparse
         {
           UtlAlloc((PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG); // free old segment table

           if (pSeg->pusHLType )
           {
             pSeg->SegFlags.Spellchecked = FALSE;
             if (!pDoc->fAutoSpellCheck )
             {
               UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusHLType),0L,0L,NOMSG);
             } /* endif */
           } /* endif */
           pDoc->Redraw |= REDRAW_ALL;                    // redraw all
         } /* endif */
         pDoc->EQFBFlags.EndOfSeg = (*(pData+sBytesinChar) == '\0');
                                                    // set EndOfSeg flag
         if ( ! pDoc->EQFBFlags.EndOfSeg )
         {
           EQFBFuncRight( pDoc );
         }
         else
         {
//          if ( IS_RTL( pDoc ))
//          {
//            pDoc->EQFBFlags.inserting = TRUE;
//            EQFBFuncRight( pDoc );
//          }
//          else
            {
              EQFBScreenCursorType( pDoc );
            }
         } /* endif */

         if ( !pDoc->hwndRichEdit )
         {  BOOL fReflowAllowed = FALSE;

            if (pDoc->EQFBFlags.Reflow ||
                 (!pDoc->EQFBFlags.Reflow && pDoc->fAutoLineWrap))
            {  // P018278: allow SoftLF in MRI files too!
				fReflowAllowed = TRUE;
		    }

           if ( pDoc->fLineWrap && fReflowAllowed &&
               ( (pDoc->lCursorCol+pDoc->lSideScroll >= pDoc->sRMargin) ||
                  EQFBRelCurPos(  pDoc, pDoc->lCursorCol+pDoc->lSideScroll, pDoc->sRMargin  ) ||
                 (pDoc->usNumTyped >= CHARACTERS_TYPED ) ) )

           {
              EQFBFuncDoLineWrap(pDoc);
              pDoc->usNumTyped = 0;                               /* @KIT920A */
              /**********************************************************/
              /* set pData again, TBCursor.usSegOffset may be changed   */
              /* during linewrap -- ensure, that EndOfSeg is reset, if  */
              /* due to linewrap we have new conditions...              */
              /**********************************************************/
              pData = &(pDoc->pEQFBWorkSegmentW[pDoc->TBCursor.usSegOffset]);
              if ( pDoc->EQFBFlags.EndOfSeg )
              {
                pDoc->EQFBFlags.EndOfSeg = (*(pData+sBytesinChar) == '\0');
                                                        // set EndOfSeg flag
                EQFBScreenCursorType( pDoc );
              } /* endif */
           } /* endif */
         } /* endif */


         pDoc->EQFBFlags.workchng = TRUE;
         pSeg->SegFlags.Typed = TRUE;
         pSeg->CountFlag.PropChanged = TRUE;

		 MTLogStartEditing( pDoc );

         pDoc->ActSegLog.usNumTyped ++;
         /*************************************************************/
         /* force reparse of autospellcheck                           */
         /*************************************************************/
         if (pDoc->usChar == BLANK )
         {
           pSeg->SegFlags.Spellchecked = FALSE;
           if (pDoc->fAutoSpellCheck )
           {
             EQFBWorkThreadTask ( pDoc, THREAD_SPELLSEGMENT );
           } /* endif */
         } /* endif */
      } /* endif */
   } /* endif */
   return;
 }

//------------------------------------------------------------------------------
// Function name: EQFBDBCS2ND - check if on 2nd DBCS and adjust sCursorCol
//------------------------------------------------------------------------------
// Function call:    EQFBDBCS2ND( pDoc, fDirection );
//------------------------------------------------------------------------------
// Description:      if code page is DBCS and cursor is on 2nd DBCS,
//                   adjust sCursorCol
//------------------------------------------------------------------------------
// Parameters:       PTBDOCUMENT    pointer to document instance data
//                   BOOL           direction : TRUE = RIGHT, FALSE = LEFT
//                                  (correction of cursor in this direction)
//------------------------------------------------------------------------------
// Returncode type:  BOOL
//------------------------------------------------------------------------------
// Returncodes:      FALSE  if no adjustment
//                   TRUE   if sCursorCol is adjusted
//------------------------------------------------------------------------------
// Side effects:     pDoc->sCursorCol may be changed, TBCursor.SegNum and
//                   TBCursor.SegOffset are changed accordingly
//------------------------------------------------------------------------------
// Function flow:    if DBCS codepage is defined then
//                      begin
//                      if current char is 2nd of a dbcs-char then
//                               begin set return value TRUE
//                               adjust sCursorCol and TBCursor.SegOffset
//                                 according to given direction
//                      endif
//                   endif
//------------------------------------------------------------------------------

 BOOL EQFBDBCS2ND
 (
    PTBDOCUMENT pDoc,                      //pointer to Doc instance
    BOOL        fDirection
 )
 {
   BOOL       fOK = FALSE;                  // nothing changed

   if (!pDoc->hwndRichEdit)
   {
	   if ((pDoc->pUserSettings && pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay)
	     || IsDBCS_CP(pDoc->ulOemCodePage))
       {  // here in BIDI IBM is displayed as MBI
		   if ( pDoc->pArabicStruct && pDoc->pArabicStruct->plCaretPos)
		   {
			 PLONG plCaretPos = pDoc->pArabicStruct->plCaretPos + pDoc->lCursorRow * (pDoc->lScrnCols+1);
			 LONG  lCol = pDoc->lCursorCol;

			 if (fDirection)
			 {
			   while ( (lCol > 0) && plCaretPos[lCol ] == plCaretPos[lCol-1])
			   {
				 fOK = TRUE;
				 pDoc->TBCursor.usSegOffset++;
				 lCol = ++(pDoc->lCursorCol);
				 EQFBCurSegFromCursor( pDoc );     // find new cursor segment
			   }
			 }
			 else                                //if correction to the left
			 {
			   while ( (lCol > 0) && plCaretPos[ lCol ] == plCaretPos[lCol-1])
			   {
				 fOK = TRUE;
				 lCol = --(pDoc->lCursorCol);          //move to correct column
				 pDoc->TBCursor.usSegOffset--;    //TBCursor.ulSegNum cannot
			   } /* endif */
			 }
		   }
		   else
		   {  // no special handling nec; RJ040306
			   // here add stuff if in Bidi IBM is displayed as "IBM"
	       }
      }
   } /* endif */

   return(fOK);
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncRight - move the cursor 1 character to the right
//------------------------------------------------------------------------------
// Function call:     EQFBFuncRight( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     move the cursor 1 character to the right
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     - if at the right end of window then
//                        scroll
//                      else
//                        move cursor 1 position to the right and update
//                        TBCursor (Seg Number and Offset of cursor pos)
//                      endif
//                    - check if work segment changed
//------------------------------------------------------------------------------

 VOID EQFBFuncRight
 (
    PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
    /******************************************************************/
    /* reset selection if shiftkey not set                            */
    /******************************************************************/
    RESETSELECTION( pDoc );

    if ( pDoc->hwndRichEdit )
    {
      EQFBFuncRightRTF( pDoc );
      return;
    } /* endif */


   if (pDoc->lCursorCol >= pDoc->lScrnCols-1)
   {
      EQFBFuncScrollIncLeft( pDoc, SIDE_DELTA );
   }
   else  if (EQFBRelCurPos(  pDoc, pDoc->lCursorCol, pDoc->lScrnCols-1  ) )
   {
       EQFBFuncScrollIncLeft( pDoc, SIDE_DELTA );
   }
   else
   {
      ++pDoc->lCursorCol;
      EQFBCurSegFromCursor( pDoc );            //find new cursor segment
      EQFBDBCS2ND( pDoc, TRUE);                //adjust Col for DBCS
      pDoc->lDBCSCursorCol = pDoc->lCursorCol; //set to actual Col
   } /* endif */

   EQFBWorkSegCheck( pDoc );                   //check if segment changed
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncLeft - move the cursor 1 character to the left
//------------------------------------------------------------------------------
// Function call:     EQFBFuncLeft( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     move the cursor 1 character to the left
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - if at the left end of window then
//                        scroll (if possible)
//                      else
//                        move cursor 1 position to the left
//                        update TBCursor (Seg Number and Offset)
//                      endif
//                    - call EQFBWorkSegCheck
//
//------------------------------------------------------------------------------

 VOID EQFBFuncLeft
 (
 PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 {
    /******************************************************************/
    /* reset selection if shiftkey not set                            */
    /******************************************************************/
    RESETSELECTION( pDoc );

    if ( pDoc->hwndRichEdit )
    {
      EQFBFuncLeftRTF( pDoc );
      return;
    } /* endif */



   if (pDoc->lCursorCol <= 0)
   {
      EQFBFuncScrollIncRight( pDoc, SIDE_DELTA );

      /****************************************************************/
      /* if CUA, in 1st column and not already at top of doc, ...     */
      /****************************************************************/
      if (pDoc->pUserSettings->fCUABksp && pDoc->EQFBFlags.Reflow
          && (pDoc->lCursorCol + pDoc->lSideScroll == 0) )
      {
        if (pDoc->lCursorRow == 0 )
        {
          if (EQFBLineUp(pDoc) == 0 )
          {
            EQFBFuncUp(pDoc);
            EQFBFuncEndLine(pDoc);
          } /* endif */
        }
        else
        {
           EQFBFuncUp(pDoc);
           EQFBFuncEndLine(pDoc);
        } /* endif */
      }
   }
   else
   {
      --(pDoc->lCursorCol);             // move one left
      EQFBCurSegFromCursor(pDoc);       // find new cursor segment
      EQFBDBCS2ND( pDoc, FALSE);        // adjust Col for DBCS

      pDoc->lDBCSCursorCol = pDoc->lCursorCol;  //set to actual Col
   }
   EQFBWorkSegCheck( pDoc );            //check if segment changed

   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncUp   - move the cursor one line up
//------------------------------------------------------------------------------
// Function call:     EQFBFuncUp( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     move the cursor 1 line up
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - if in 1st line of window then
//                        scroll down
//                      else if no next line then
//                        do nothing
//                      else
//                        move cursor 1 line up
//                        update TBCursor (Seg No.and Offset)
//                      endif
//                    - call EQFBWorkSegCheck
//
//------------------------------------------------------------------------------

 VOID EQFBFuncUp
 (
 PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 {
    PTBROWOFFSET pTBRow;         // pointer to row structure

    /******************************************************************/
    /* reset selection if shiftkey not set                            */
    /******************************************************************/
    RESETSELECTION( pDoc );


   if (pDoc->lCursorRow <= 0)           // first row is sCursorRow 0
   {
      EQFBFuncScrollDown( pDoc ,TRUE);
   }
   else
   {

      pTBRow = pDoc->TBRowOffset+pDoc->lCursorRow ; // screen - row offset

      if ( pTBRow->ulSegNum == 0)
      {
//      return code for warning may be set here
      }
      else
      {
          --pDoc->lCursorRow;
          pDoc->lCursorCol = pDoc->lDBCSCursorCol; //correct Col if previous
                                                   // DBCS correction
          EQFBCurSegFromCursor( pDoc );            // find new cursor segment
          EQFBDBCS2ND( pDoc, FALSE);                // adjust Col for DBCS
      } /* endif */

   }
   EQFBWorkSegCheck( pDoc );                       //check if segment changed
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncDown   - move the cursor one line down
//------------------------------------------------------------------------------
// Function call:     EQFBFuncDown( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     move the cursor 1 line down
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - if segment number of actual segment >0 then
//                        if in last line of window then
//                          scroll
//                        else if no next line then
//                          do nothing
//                        else
//                          move cursor 1 line down
//                          update TBCursor (Seg No. and Offset of cursor pos )
//                        endif
//                    - call EQFBWorkSegCheck
//
//------------------------------------------------------------------------------

 VOID EQFBFuncDown
 (
 PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 {
   PTBROWOFFSET pTBRow;         // pointer to row structure

   /******************************************************************/
   /* reset selection if shiftkey not set                            */
   /******************************************************************/
   RESETSELECTION( pDoc );


   if ( (pDoc->TBRowOffset+1+pDoc->lCursorRow)->ulSegNum > 0 )
   {
     if (pDoc->lCursorRow >= pDoc->lScrnRows-1)
     {
        EQFBFuncScrollUp( pDoc,TRUE );
     }
     else
     {

      pTBRow = pDoc->TBRowOffset+pDoc->lCursorRow+2 ;  // screen - row table offset
      if ( pTBRow->ulSegNum == 0)
      {
//      return code for warning may be set here
      }
      else
      {
          ++pDoc->lCursorRow;
          pDoc->lCursorCol = pDoc->lDBCSCursorCol;//correct Col if previous
                                                  // DBCS correction
          EQFBCurSegFromCursor( pDoc );           // find new cursor segment
          EQFBDBCS2ND( pDoc, FALSE);             // adjust Col for DBCS
      } /* endif */

     } /* endif */
     EQFBWorkSegCheck( pDoc );        //check if segment changed
   } /* endif */

   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncStartLine   - move the cursor to start of line
//------------------------------------------------------------------------------
// Function call:     EQFBFuncStartLine( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     move the cursor to start of line
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - while not at 1st pos of line:
//                        move cursor left and scroll if necessary
//                      endwhile
//                    - update TBCursor (Seg Number and Offset of cursor pos)
//                    - call EQFBWorkSegCheck
//
//------------------------------------------------------------------------------

 VOID EQFBFuncStartLine
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   if ( pDoc->hwndRichEdit )
   {
     EQFBFuncStartLineRTF(pDoc);
   }
   else
   {
     ULONG   ulPos;

     /******************************************************************/
     /* reset selection if shiftkey not set                            */
     /******************************************************************/
     RESETSELECTION( pDoc );

     ulPos = pDoc->lCursorCol + pDoc->lSideScroll;

     while (ulPos-- > 0)
     {

        if (pDoc->lCursorCol <= 0)
        {
           if (pDoc->lSideScroll > 0)
           {
             --pDoc->lSideScroll;
           }
        }
        else
        {
           --(pDoc->lCursorCol);       // move one left
        }
     } /*endwhile*/

     pDoc->lDBCSCursorCol = pDoc->lCursorCol;
     pDoc->Redraw |= REDRAW_ALL;
     EQFBCurSegFromCursor( pDoc );        // find new cursor segment
     EQFBWorkSegCheck( pDoc );            //check if segment changed
   } /* endif */
   return;
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncStartSeg    - move the cursor to start of segment
//------------------------------------------------------------------------------
// Function call:     EQFBFuncStartSeg( PTBDOCUMENT )
//
//------------------------------------------------------------------------------
// Description  :     move the cursor to start of the segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     - set ptr to segment,
//                    - set TBCursor.usSegOffset to begin of segment
//                    - get physical cursor positions of startseg
//                    - while char is protected and not at end of string
//                        move cursor right
//                      endwhile
//                    - if cursor at end of segment and next
//                      segment in same line then
//                         set EndOfSeg- flag for triple-Mode
//                         update screen cursor type
//                      endif
//
//------------------------------------------------------------------------------

 VOID EQFBFuncStartSeg
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   PTBSEGMENT  pSeg;                   // pointer to segment
   PSZ_W  pData;                       // pointer to data
   BOOL   fEndofProtected = FALSE;     // true if at end of protected
   USHORT usType;
   USHORT usSegOffset;                 //temp segment offset
   LONG   lPos = 0;                    // line position

   /******************************************************************/
   /* reset selection if shiftkey not set                            */
   /******************************************************************/
   RESETSELECTION( pDoc );


   (pDoc->TBCursor).usSegOffset  = 0;  // start at the beginning
//   pSeg = EQFBGetSeg( pDoc, pDoc->TBCursor.ulSegNum );
   pSeg = EQFBGetVisSeg( pDoc, &pDoc->TBCursor.ulSegNum ); // check if visible


   EQFBPhysCursorFromSeg ( pDoc );        // pointer to doc ida
                                          //loop while char is protected
   usType = UNPROTECTED_CHAR;                    // init usType

   while ( !fEndofProtected )
   {
     usType = EQFBCharType(pDoc,pSeg,
                                pDoc->TBCursor.usSegOffset);
     usSegOffset = pDoc->TBCursor.usSegOffset + 1;
     switch ( usType )
     {
       case  COMPACT_CHAR:
       case PROTECTED_CHAR:
       case SHORTEN_CHAR:
         if ( *(pSeg->pDataW + usSegOffset))
         {
            EQFBFuncRight(pDoc);
            if ( pDoc->TBCursor.usSegOffset < usSegOffset )
            {
                fEndofProtected = TRUE;  // something is wrong ... leave loop
                // RJ: QFA-seg without transl. data +compact P018728 ??
                // make fix only for compact -
                // for security do changes only for COMPACT
                if (usType== COMPACT_CHAR)
                {
                  EQFBFuncLeft(pDoc);
			    }
            }
         }
         else
         {
            fEndofProtected = TRUE;       // get out of the while loop..
         } /* endif */
         break;

       case HIDDEN_CHAR:
         pDoc->TBCursor.usSegOffset ++;
         break;

       case  LINEBREAK_CHAR:
         /***************************************************************/
         /* if next char exists in segm., set cursor to next line       */
         /* if next char protected, go on, else stop loop               */
         /***************************************************************/
         if (*(pSeg->pDataW + usSegOffset )  )
         {
           if ( EQFBCharType(pDoc,pSeg, usSegOffset)
                             != PROTECTED_CHAR )
           {
             fEndofProtected = TRUE;
           } /* endif */
           EQFBFuncDown(pDoc);
           EQFBFuncStartLine(pDoc);
         }
         else
         {
           fEndofProtected = TRUE;
         } /* endif */
         break;
       default :
         fEndofProtected = TRUE;
         break;
     } /* endswitch */
   } /* endwhile */

   if ( (usType == PROTECTED_CHAR) ||
        (usType == COMPACT_CHAR) || (usType == SHORTEN_CHAR) )
   {
     EQFBCheckEndOfSeg(pDoc,pSeg,pDoc->TBCursor.usSegOffset);
     if ( usType == COMPACT_CHAR &&
          !(pDoc->EQFBFlags.EndOfSeg)  )
     {
       EQFBFuncRight(pDoc);
     } /* endif */
   }
   else
   {
     /*****************************************************************/
     /* this is default at start of segment                           */
     /*****************************************************************/
      pDoc->EQFBFlags.EndOfSeg = FALSE;
      EQFBScreenCursorType( pDoc );
   } /* endif */

   /*******************************************************************/
   /* check how many characters belong to this segment                */
   /*******************************************************************/
   pData = pSeg->pDataW + pDoc->TBCursor.usSegOffset ;

   if (IsDBCS_CP(pDoc->ulOemCodePage))
   {
	   CHAR_W c;
		while( ((c =*pData) != NULC) && (*pData != LF))
		{
			if (EQFIsDBCSChar(c, pDoc->ulOemCodePage))
			{
				lPos += 2;
		    }
		    else
		    {
				lPos ++;
		    }
			pData++;
	    } /* endwhile */
   }
   else
   {
      while ( *pData && (*pData != LF) )
      {
        pData++;
        lPos++;
      } /* endwhile */
   }
   /*******************************************************************/
   /* if we are too far on the right hand border, set sCursorCol in   */
   /* such a way, that segment starts at column 10 (SIDE_DELTA        */
   /*******************************************************************/
   if ( ((pDoc->lCursorCol + SIDE_DELTA) > pDoc->lScrnCols)
         && (lPos > SIDE_DELTA) )
   {
     lPos  = min( lPos + pDoc->lCursorCol + 1 - pDoc->lScrnCols,
                   (pDoc->lCursorCol - SIDE_DELTA) ) ;
     pDoc->lSideScroll = pDoc->lSideScroll + lPos;
     pDoc->lCursorCol  = pDoc->lCursorCol  - lPos;
   } /* endif */


   pDoc->lDBCSCursorCol = pDoc->lCursorCol;

   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncEndSeg    - move the cursor to end of segment
//------------------------------------------------------------------------------
// Function call:     EQFBFuncEndSeg( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :      move the cursor to end of the segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     - set ptr to segment,
//                    - set TBCursor.usSegOffset to end of segment
//                    - get physical cursor positions of endseg
//                    - if cursor at end of segment and next
//                       segment in same line then
//                        set EndOfSeg- flag for triple-Mode
//                        update scrn cursor type
//                      endif
//
//------------------------------------------------------------------------------
 VOID EQFBFuncEndSeg
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   PTBSEGMENT pSeg;                       // segment pointer
   PSZ_W  pData;                          // pointer to data
   USHORT usSegOffset;
   USHORT usType;
   DISPSTYLE DispStyle;

   /******************************************************************/
   /* reset selection if shiftkey not set                            */
   /******************************************************************/
   RESETSELECTION( pDoc );


   if (pDoc->hwndRichEdit && pDoc->pDispFileRTF )
   {
     pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
   }


   EQFBCurSegFromCursor( pDoc );          // set cursor segment

   pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );

   (pDoc->TBCursor).usSegOffset  = pSeg->usLength - 1; // end of segment
                                          // get pointer to data
   /*******************************************************************/
   /* if clause:                                                      */
   /* adjust pDoc->TBCursor.usSegOffset if on compact-hidden chars    */
   /* else:                                                           */
   /* adjust pDOc->TBCursor.usSegOffset to LF + ending blanks         */
   /*******************************************************************/

   DispStyle = (pSeg->SegFlags.Expanded) ?
                 DISP_PROTECTED : (pDoc->DispStyle);
   usSegOffset = (pDoc->TBCursor).usSegOffset;
   if ( (DispStyle == DISP_COMPACT) &&
       ((usType= EQFBCharType(pDoc,pSeg,usSegOffset)) == HIDDEN_CHAR) )
   {
     usType = HIDDEN_CHAR;
     while (usType == HIDDEN_CHAR && (usSegOffset > 0 ))
     {
       usSegOffset--;
       usType = EQFBCharType(pDoc,pSeg, usSegOffset);
     } /* endwhile */
     pDoc->TBCursor.usSegOffset = usSegOffset;
   }
   else
   {
     pData = (pSeg->pDataW + pDoc->TBCursor.usSegOffset);
     if ( *pData == LF)                   // skip newline
     {
        pData--;
     } /* endif */
     while ( *pData == BLANK )              // skip blanks
     {
        pData--;
        pDoc->TBCursor.usSegOffset--;
     } /* endwhile */


   } /* endif */
   EQFBPhysCursorFromSeg ( pDoc );                // pointer to doc ida
                                          // set EndOfSeg Flag
   pData = (pSeg->pDataW + pDoc->TBCursor.usSegOffset);
   if (*pData != LF)
   {
      EQFBCheckEndOfSeg(pDoc,pSeg,pDoc->TBCursor.usSegOffset);
//      pDoc->EQFBFlags.EndOfSeg = (*(pData+1) == EOS); // set EndOfSeg flag
      EQFBDBCS2ND(pDoc,FALSE);            //adjust CursorCol if 2nd DBCSbyte
      EQFBScreenCursorType( pDoc );
   } /* endif */
   pDoc->lDBCSCursorCol = pDoc->lCursorCol;

   if (pDoc->hwndRichEdit)
   {
     // Avoid sending enSelChanged message
     BYTE b = pDoc->pDispFileRTF->bRTFFill;
     pDoc->pDispFileRTF->bRTFFill = RTF_FILL;

     EQFBGotoSegRTF( pDoc,
                     pDoc->TBCursor.ulSegNum,     // pos. at this segment
                     (USHORT)(pDoc->TBCursor.usSegOffset+1) );
     pDoc->pDispFileRTF->bRTFFill = b;
   } /* endif */



   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncEndLine   - move the cursor to end of line
//------------------------------------------------------------------------------
// Function call:     EQFBFuncEndLine( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     move the cursor to end of line
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     call EQFBFuncDoEndLine
//------------------------------------------------------------------------------

 VOID EQFBFuncEndLine
 (
   PTBDOCUMENT pDoc                              //pointer to Doc instance
 )
 {
   /*******************************************************************/
   /* goto previous segment if *pData[0] == LF                        */
   /*******************************************************************/
   if ( pDoc->hwndRichEdit )
   {
     EQFBFuncEndLineRTF(pDoc);
   }
   else
   {
     EQFBFuncDoEndLine(pDoc, FALSE);
   }
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncDoEndLine   - move the cursor to end of line
//------------------------------------------------------------------------------
// Function call:     EQFBFuncDoEndLine( PTBDOCUMENT,
//                                       BOOL );
//------------------------------------------------------------------------------
// Description  :     move the cursor to end of line
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - if cursor already right of text then
//                        move cursor to the left
//                      else
//                        while not end of line
//                          while not end of segment or end of line
//                            increase counter
//                            if end of segment and not end of line then
//                              get next segment
//                            endif
//                           endwhile
//                           move cursor counter-many times to the right
//                            (scroll if necessary)
//                        endwhile
//                      endif
//                    - update TBCursor (Seg Number and Offset of cursor pos)
//                    - call EQFBWorkSegCheck
//
//------------------------------------------------------------------------------

 VOID EQFBFuncDoEndLine
 (
   PTBDOCUMENT pDoc,                             //pointer to Doc instance
   BOOL        fFromJoinLine
 )

 {
   PSZ_W pData;                           // pointer to data
   ULONG ulSegNum;                        // segment to be used
   PTBSEGMENT pSeg;                       // pointer to segment
   PTBROWOFFSET pTBTemp;                  //temp pointer


   EQFBCurSegFromCursor( pDoc );
   /*********************************************************************/
   /* check segnum/segoffset of start of next line (in TBRowOffset)     */
   /* decrease segoffset (if possible) or segnum to get end of this line*/
   /*********************************************************************/
   pTBTemp = pDoc->TBRowOffset + pDoc->lCursorRow + 2;
   if ( pTBTemp->usSegOffset )
   {
	 pDoc->TBCursor.usSegOffset = (pTBTemp->usSegOffset>0)? (pTBTemp->usSegOffset - 1) : 0;
     pDoc->TBCursor.ulSegNum = pTBTemp->ulSegNum;
     pSeg = EQFBGetSegW(pDoc,pDoc->TBCursor.ulSegNum);
   }
   else
   {
     ulSegNum = pTBTemp->ulSegNum;
     if ( ulSegNum )
     {
       ulSegNum--;
       pSeg = EQFBGetPrevVisSeg(pDoc,&ulSegNum);
       pDoc->TBCursor.usSegOffset = (pSeg->usLength > 0) ? (pSeg->usLength - 1) : 0;
       pDoc->TBCursor.ulSegNum = ulSegNum;
     }
     else
     {
        ulSegNum = pDoc->ulMaxSeg - 1;
        pSeg = EQFBGetVisSeg( pDoc, &ulSegNum );    // check if segment is visible
        if ( !pSeg )
        {
           ulSegNum = pDoc->ulMaxSeg - 1;
           pSeg = EQFBGetPrevVisSeg( pDoc, &ulSegNum ); // find last visible segment
        } /* endif */

        (pDoc->TBCursor).ulSegNum = ulSegNum;              // cursor row
                                                           // cursor offset (skip CRLF)
        (pDoc->TBCursor).usSegOffset = (pSeg->usLength > 0) ? (pSeg->usLength - 1) : 0;
     } /* endif */
   } /* endif */
   pData = (pSeg->pDataW + pDoc->TBCursor.usSegOffset);
   if ( *pData == LF)                   // skip newline
   {
     if (pDoc->TBCursor.usSegOffset )
     {
       pData--;
     }
     else           // LF is *pData[0]!, get previous seg (KBT0268)
     {
       if (!fFromJoinLine )
       {
         ulSegNum = pDoc->TBCursor.ulSegNum;
         ulSegNum--;
         if (ulSegNum )
         {
           pSeg = EQFBGetPrevVisSeg(pDoc,&ulSegNum);
           pDoc->TBCursor.usSegOffset = (pSeg->usLength > 0) ? (pSeg->usLength - 1) : 0;
           pDoc->TBCursor.ulSegNum = ulSegNum;
           pData = (pSeg->pDataW + pDoc->TBCursor.usSegOffset);
         } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */
   if (*pData == BLANK )
   {
     /*****************************************************************/
     /* skip blanks                                                   */
     /*****************************************************************/
     while ( *pData == BLANK )
     {
        pData--;
        pDoc->TBCursor.usSegOffset--;
     } /* endwhile */
     pData++;
     pDoc->TBCursor.usSegOffset++;
   } /* endif */

   EQFBPhysCursorFromSeg ( pDoc );                // pointer to doc ida
                                          // set EndOfSeg Flag
   if (pSeg )
   {
     pData = (pSeg->pDataW + pDoc->TBCursor.usSegOffset);
   } /* endif */
   if (*pData != LF)
   {
      pDoc->EQFBFlags.EndOfSeg = (*(pData+1) == EOS); // set EndOfSeg flag
   } /* endif */
   EQFBDBCS2ND(pDoc,FALSE);            //adjust CursorCol if 2nd DBCSbyte
   EQFBScreenCursorType( pDoc );

   pDoc->lDBCSCursorCol = pDoc->lCursorCol;

   if (  EQFBRelCurPos(  pDoc, pDoc->lCursorCol, pDoc->lScrnCols-1  ) )
   {
	  ULONG ulColumn = 0;
      LONG  lPos = EQFBGetAbsPos( pDoc, pDoc->lCursorCol  );
      ulColumn = ((lPos -  pDoc->lScrnCols*pDoc->cx) / pDoc->cx) + 3;
      pDoc->lSideScroll = pDoc->lSideScroll + ulColumn;      // shift screen a little bit
      pDoc->lCursorCol  = pDoc->lCursorCol  - ulColumn;
      EQFBCurSegFromCursor( pDoc );         // find new cursor segment
      EQFBDBCS2ND ( pDoc, TRUE);            // adjust lCursorCol if 2ndDB
   }
   pDoc->Redraw |= REDRAW_ALL;
   EQFBWorkSegCheck( pDoc );            //check if segment changed

   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncTab   - move the cursor to next tab stop
//------------------------------------------------------------------------------
// Function call:     EQFBFuncTab( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     move the cursor to next tab stop
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     - set counter how many bytes to the next tabstop
//                    - if DBCS codepage is defined then
//                         adjust sCursorCol / sSideScroll
//                         get TBCursor.SegNum + Offset from cur. cursor
//                         set DBCSCursorCol to new CursorCol
//                         call EQFBDBCS2ND (correct to the left if nec)
//                      else
//                         while counter is >0
//                            EQFBFuncRight and decrease counter
//                         endwhile
//                      endif
//
//------------------------------------------------------------------------------

 VOID EQFBFuncTab
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 {
   int   iI;                              // index
   LONG  lPos;                            // current position

   pDoc->lCursorCol = pDoc->lDBCSCursorCol; //set to DBCSCursorCol
   lPos = pDoc->lCursorCol + pDoc->lSideScroll;

   iI = configTabStops - lPos%configTabStops;
   if (IsDBCS_CP(pDoc->ulOemCodePage))
   {
      if (pDoc->lCursorCol+iI >= pDoc->lScrnCols-1)
      {
        if ( (pDoc->lSideScroll + iI + pDoc->lScrnCols)
                                    <= MAX_SEGMENT_SIZE )
        {
            pDoc->lSideScroll = pDoc->lSideScroll + iI;           // adjust scroll if nec
            pDoc->Redraw |= REDRAW_ALL;        // set redraw status;
        } /* endif */
      }
      else
      {
         pDoc->lCursorCol = (SHORT)(pDoc->lCursorCol + iI);
      } /* endif */
      EQFBCurSegFromCursor(pDoc);           // set TBCursor.SegNum+SegOffset
      pDoc->lDBCSCursorCol = pDoc->lCursorCol;  //nec for rekursive calls
      EQFBDBCS2ND(pDoc,FALSE);              // adjust to the left if nec
   }
   else
   {
      while (iI-- > 0)
         EQFBFuncRight( pDoc );
   } /* endif */


   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncBacktab   - move the cursor to previous tab stop
//------------------------------------------------------------------------------
// Function call:     EQFBFuncBackTab( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :      move the cursor to previous tab stop
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - set counter how many bytes to the next tabstop
//                    - if DBCS codepage is defined then
//                         adjust sCursorCol / sSideScroll
//                         get TBCursor.SegNum +SegOffset from current cursor
//                         set DBCSCursorCol to new CursorCol
//                         call EQFBDBCS2ND (correct to the left if nec)
//                      else
//                         while counter is >0
//                            EQFBFuncLeft and decrease counter
//                         endwhile
//                      endif
//
//------------------------------------------------------------------------------
 VOID EQFBFuncBacktab
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   LONG  lI;                              // index
   LONG  lPos;                            // current position

   pDoc->lCursorCol = pDoc->lDBCSCursorCol; //set to DBCSCursorCol
   lPos = pDoc->lCursorCol + pDoc->lSideScroll;
   if ((lI = lPos%configTabStops) == 0)
      lI = configTabStops;
   while ( lI-- > 0 )
   {
     /*****************************************************************/
     /* do EQFBFuncLeft, but DBCS adjustment outside of loop          */
     /*****************************************************************/
     if (pDoc->lCursorCol <= 0)
     {
        EQFBFuncScrollIncRight( pDoc, SIDE_DELTA );
     }
     else
     {
        --(pDoc->lCursorCol);               // move one left
        EQFBCurSegFromCursor(pDoc);         // find new cursor segment
     }
   } /* endwhile */
   EQFBDBCS2ND( pDoc, FALSE);               // adjust Col for DBCS
   pDoc->lDBCSCursorCol = pDoc->lCursorCol; //set to actual Col
   EQFBWorkSegCheck( pDoc );                //check if segment changed
   return;
 }


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncDeleteChar   - delete current character
//------------------------------------------------------------------------------
// Function call:     EQFBFuncDeleteChar( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     delete current  character
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - call EQFBFuncDeleteCharDo to do the dirty work with
//                      parameter FALSE (we are not coming from BACKSPACE)
//
//------------------------------------------------------------------------------

 VOID EQFBFuncDeleteChar
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   EQFBFuncDeleteCharDo( pDoc, FALSE );
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncDeleteCharDo   - delete current character
//------------------------------------------------------------------------------
// Function call:     EQFBFuncDeleteCharDo( PTBDOCUMENT, BOOL );
//
//------------------------------------------------------------------------------
// Description  :     delete current  character, skip trailing Blanks if not
//                    coming from BACKSPACE
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//                    BOOL              are we coming from BACKSPACE?
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - if right of text then
//                        do nothing
//                      else if current char is UnProtected then
//                        delete char
//                        call EQFBUpdateChangedSeg
//                      else
//                        do beep
//                      endif
//
//------------------------------------------------------------------------------
 static
 VOID EQFBFuncDeleteCharDo
 (
   PTBDOCUMENT pDoc,                             //pointer to Doc instance
   BOOL        fBackSpace
 )
 {
   USHORT usPos;
   LONG   lPos;                          // length of delta
   SHORT sDellen = 1;                      // 1 for SBCS, 2 for DBCS
                                           //how many bytes to delete
   CHAR_W usChar;
   BOOL   fEndOfLine = FALSE;              // true if beyond end of line


  /********************************************************************/
  /* delete selection ...                                             */
  /********************************************************************/
  if ( !EQFBFuncDelSel( pDoc ))
  {
    USHORT usStatus = EQFBCurrentState(pDoc);
    if (usStatus == STARGET )
    {
      WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
    }
    else if ( pDoc->ulWorkSeg != pDoc->TBCursor.ulSegNum )
    {
      /****************************************************************/
      /* we are outside our active worksegment                        */
      /****************************************************************/
      WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
    }
    else
    {
      lPos = EQFBCurSegFromCursor( pDoc );
      lPos--;                                      // adjust position (CR )
      /******************************************************************/
      /* check whether cursor is positioned on ending blanks            */
      /******************************************************************/
      if (pDoc->pUserSettings->fCUABksp && pDoc->EQFBFlags.Reflow && !fBackSpace)
      {
        usPos = pDoc->TBCursor.usSegOffset;
        usChar = *(pDoc->pEQFBWorkSegmentW + usPos);

        while ((usChar == BLANK) && (usPos < pDoc->pTBSeg->usLength) )
        {
          usPos++;
          usChar = *(pDoc->pEQFBWorkSegmentW + usPos);
        } /* endwhile */
        if ((usChar == LF) || (usChar == SOFTLF_CHAR))
        {
          fEndOfLine = TRUE;
        } /* endif */
      }
      if ((lPos >= 0 ) && !fEndOfLine )
      {
         if ( EQFBCharType(pDoc,pDoc->pTBSeg,pDoc->TBCursor.usSegOffset) ==
                                                     UNPROTECTED_CHAR )
         {
        /*****************************************************************/
        /* if no chars prev.typed, update UndoBuffer (fUndoState=FALSE ) */
        /* if already chars typed in, do not update (fUndoState = TRUE ) */
        /* ( typing means typing or deleting a character)                */
        /*****************************************************************/
            if ( pDoc->pUndoSegW && !pDoc->fUndoState)
            {                                      //update undo buffer
               UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
               pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
               pDoc->fUndoState = TRUE;
            } /* endif */

            usPos = pDoc->TBCursor.usSegOffset;
            /************************************************************/
            /* KBT0339: if in front of and after char is SoftLF, delete */
            /* one softLF too                                           */
            /************************************************************/
            if (pDoc->fAutoLineWrap && pDoc->fLineWrap )
            {
              usChar = *(pDoc->pEQFBWorkSegmentW + usPos+1);
              if ( usPos && (usChar == SOFTLF_CHAR)
                   && (usChar == *(pDoc->pEQFBWorkSegmentW + usPos - 1) ))
              {
                sDellen++;           // delete SoftLF following char too
                pDoc->Redraw |= REDRAW_ALL;                    // redraw all
              } /* endif */
            } /* endif */
            EQFBWorkLeft(pDoc,usPos, sDellen);            // change string and flag
            pDoc->pTBSeg->usLength = pDoc->pTBSeg->usLength - sDellen;                      // set new size
            EQFBReparse(pDoc, pDoc->pTBSeg, usPos, (SHORT)-sDellen);           //reparse segment table
            pDoc->Redraw |= REDRAW_LINE;            // set redraw status;

            EQFBCurSegFromCursor( pDoc );           // get new cursor pos
            EQFBWorkSegCheck( pDoc );
                                                    // update TBRowOffset table
            EQFBScrnLinesFromSeg ( pDoc,            // pointer to doc ida
                             pDoc->lCursorRow,      // starting row
                             (SHORT)(pDoc->lScrnRows -
                                 pDoc->lCursorRow),  // number of rows
                             &(pDoc->TBCursor) );   // seg num start row

            if ( *(pDoc->pEQFBWorkSegmentW+usPos) != '\n' )
            {
               EQFBSysDispRestOfLine( pDoc );
            } /* endif */

            // remove the block mark if in the same segment
            EQFBFuncResetMarkInSeg( pDoc );
            pDoc->pTBSeg->SegFlags.Typed = TRUE;
            pDoc->pTBSeg->CountFlag.PropChanged = TRUE;
            MTLogStartEditing( pDoc );
            pDoc->ActSegLog.usNumTyped ++;

         }
         else
         {
            WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
         } /* endif */
      }
      else
      {
        /****************************************************************/
        /* sPos < 0 ( cursor at or beyond end of line )                 */
        /****************************************************************/
        if (pDoc->pUserSettings->fCUABksp && pDoc->EQFBFlags.Reflow && !fBackSpace)
        {
          /**************************************************************/
          /* check if segment contains lf after some blanks             */
          /**************************************************************/
          usPos = pDoc->TBCursor.usSegOffset;
          usChar = *(pDoc->pEQFBWorkSegmentW + usPos);

          while ((usChar == BLANK) && (usPos < pDoc->pTBSeg->usLength) )
          {
            usPos++;
            usChar = *(pDoc->pEQFBWorkSegmentW + usPos);
          } /* endwhile */
          /**************************************************************/
          /* delete linefeed                                            */
          /**************************************************************/
          if (((usChar == LF) || (usChar == SOFTLF_CHAR)) &&
              !( (pDoc->TBCursor.ulSegNum == pDoc->ulMaxSeg - 1) &&
                 (pDoc->TBCursor.usSegOffset == pDoc->pTBSeg->usLength -1)) )
          {
            EQFBFuncDoEndLine(pDoc, TRUE);
            EQFBFuncJoinLineDo(pDoc,FALSE);
            usPos = pDoc->TBCursor.usSegOffset;
            usChar = *(pDoc->pEQFBWorkSegmentW + usPos);
            sDellen = (USHORT)UTF16strlenCHAR(pDoc->pEQFBWorkSegmentW);
            if ((usChar == BLANK) && (usPos < UTF16strlenCHAR(pDoc->pEQFBWorkSegmentW)-1) )
            {
              EQFBFuncRight(pDoc);
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */
   return;
 }
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBUpdateChangedSeg  - update changed segment
//------------------------------------------------------------------------------
// Function call:     EQFBUpdateChangedSeg( PTBDOCUMENT );
//------------------------------------------------------------------------------
// Description  :     update changed segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - update length of segment
//                    - set TBCursor
//                    - update TBRowOffsettable
//                    - set pSeg->SegFlags.Typed = TRUE;
//------------------------------------------------------------------------------

 VOID EQFBUpdateChangedSeg
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   pDoc->EQFBFlags.workchng = TRUE;                        // something changed in workseg
   if (pDoc->pTBSeg )
   {
     EQFBCompSeg( pDoc->pTBSeg );                          //update length of segment
   } /* endif */

   /*****************************************************************/
   /* update cursor position and TBRow table where needed           */
   /*****************************************************************/
   if ( !pDoc->hwndRichEdit )
   {
     EQFBCurSegFromCursor( pDoc );                 // get new cursor pos
                                                   // update TBRowOffset table
     EQFBScrnLinesFromSeg ( pDoc,                  // pointer to doc ida
                            pDoc->lCursorRow,      // starting row
                            (pDoc->lScrnRows -
                                pDoc->lCursorRow), // number of rows
                            &(pDoc->TBCursor) );   // seg num start row
     pDoc->Redraw |= REDRAW_BELOW;                 // redraw below the line
   }
   else
   {
     EQFBSetWorkSegRTF( pDoc, pDoc->ulWorkSeg, pDoc->pEQFBWorkSegmentW );
   } /* endif */

   if (pDoc->pTBSeg )
   {
     pDoc->pTBSeg->SegFlags.Typed = TRUE;                  // something modified
   } /* endif */

   MTLogStartEditing( pDoc );

   pDoc->ActSegLog.usNumTyped ++;

   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncBackspace   - move left and delete a character
//------------------------------------------------------------------------------
// Function call:     EQFBFuncBackspace( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     move left and delete a character
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if at right of text then
//                      call EQFBFuncPad
//                    endif
//                    if not at left end of line and left end of segment then
//                      call EQFBFuncLeft
//                      call EQFBFuncDeleteChar
//                    endif
//
//------------------------------------------------------------------------------

 VOID EQFBFuncBackspace
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 { LONG  lPos;
   LONG  lLen;                                   // delta position
   USHORT usSegOffset;
   USHORT usChar;                         // character

   /****************************************************************/
   /* if selection in current segment delete it...                 */
   /****************************************************************/
   if ( !EQFBFuncDelSel( pDoc )  )
   {
     lPos = pDoc->lCursorCol + pDoc->lSideScroll;

     lLen = EQFBCurSegFromCursor( pDoc );       // get current TBCursor
     if ( lLen < 0 )
     {
        EQFBFuncPad ( pDoc, lLen);       // pad with blanks
     } /* endif */

     if ( pDoc->TBCursor.usSegOffset > 0 )
     {
        if (lPos > 0 )
        {
          /*****************************************************************/
          /* if no chars prev.typed, update UndoBuffer (fUndoState=FALSE ) */
          /* if already chars typed in, do not update (fUndoState = TRUE ) */
          /* ( typing means typing or deleting a character)                */
          /*****************************************************************/
           if ( pDoc->pUndoSegW && !pDoc->fUndoState)
           {                                      //update undo buffer
              UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
              pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
              pDoc->fUndoState = TRUE;
           } /* endif */
           EQFBFuncLeft( pDoc );
           EQFBFuncDeleteCharDo (pDoc, TRUE );
        }
        else
        {
          if ( pDoc->pUserSettings->fCUABksp && pDoc->EQFBFlags.Reflow )
          {
             /**************************************************************/
             /* allow backspace across line boundery                       */
             /* check if segment contains a linefeed at a previous position*/
             /**************************************************************/
             usSegOffset = pDoc->TBCursor.usSegOffset;
             usChar = *(pDoc->pEQFBWorkSegmentW + usSegOffset);
             while ( ((usChar != LF) && ( usChar != SOFTLF_CHAR)) && usSegOffset > 0)
             {
               usSegOffset --;
               usChar = *(pDoc->pEQFBWorkSegmentW + usSegOffset);
             } /* endwhile */
             /**************************************************************/
             /* delete linefeed                                            */
             /**************************************************************/
             if ( (usChar == LF) || (usChar == SOFTLF_CHAR) )
             {
               EQFBFuncUp(pDoc);
               EQFBFuncDoEndLine(pDoc, TRUE);
               EQFBFuncJoinLineDo(pDoc,FALSE);
               /*********************************************************/
               /* do not position cursor on the blank                   */
               /*********************************************************/
               usSegOffset = pDoc->TBCursor.usSegOffset;
               usChar = *(pDoc->pEQFBWorkSegmentW + usSegOffset);
               if ( usChar == BLANK )
               {
                 EQFBFuncRight (pDoc);
               } /* endif */
//             /************************************************************/
//             /* NO! because in DBCS there may be no blank inserted!!!!   */
//             /* reset fUndoState to avoid that DeleteChar updates        */
//             /* the Undo buffer again                                    */
//             /************************************************************/
//             pDoc->fUndoState = TRUE;
//
//             EQFBFuncDeleteChar(pDoc);     //delete blank inserted by Joinline
             } /* endif */

          } /*endif*/
        } /* endif */
     } /* endif */
   } /* endif */

   return;
 }


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncTruncate   - truncate current line
//------------------------------------------------------------------------------
// Function call:     EQFBFuncTruncate( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     truncate current line
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if not right of text then
//                      if current char is protected then
//                        beep
//                      else
//                        display msg if part to be deleted contains tags
//                        if user wants to delete then
//                          while not at end of line or end of segment
//                            delete char in worksegment
//                          endwhile
//                          call EQFBUpdateChangedSeg
//                          reset any old block mark in same segment
//                        endif
//                      endif
//
//------------------------------------------------------------------------------

 VOID EQFBFuncTruncate
 (
 PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 { USHORT usPos;
   USHORT usChar;                      // character
   PSZ_W  pData;                       // pointer to data
   LONG   lPos;                        // length of delta
   BOOL fProtect = FALSE;              // false if no protected chars
                                       // to be deleted
   USHORT usResult = FALSE;            // return from UtlError
   SHORT  sBytesInChar = 1;            // default for SBCS


   if (pDoc->hwndRichEdit )
   {
     EQFBCompSeg(pDoc->pTBSeg);
   } /* endif */
lPos = EQFBCurSegFromCursor( pDoc );
   lPos--;                                // adjust position (CR )
   if (lPos>=0)
   {
       if ( pDoc->TBCursor.usSegOffset > 0 &&
           EQFBCharType(pDoc,pDoc->pTBSeg,pDoc->TBCursor.usSegOffset)
                        == PROTECTED_CHAR &&
           EQFBCharType(pDoc,pDoc->pTBSeg,(USHORT)(pDoc->TBCursor.usSegOffset-1))
                        == PROTECTED_CHAR )
       {  //deleting half of a tag is not allowed
          WinAlarm( HWND_DESKTOP, WA_WARNING ); // issue a beep if trying on R/o doc
       }
       else
       {
		  USHORT fEndOfSeg = FALSE;

          EQFBWorkSegCheck( pDoc );              //check if segment changed
          // remove SoftLF if available
          if (pDoc->fLineWrap && pDoc->fAutoLineWrap )
          {
            USHORT  i = pDoc->pTBSeg->usLength;
            USHORT  usSegOffset = pDoc->TBCursor.usSegOffset;
            EQFBGotoSeg(pDoc, pDoc->TBCursor.ulSegNum, 0);
            EQFBBufRemoveSoftLF(pDoc->hwndRichEdit, pDoc->pEQFBWorkSegmentW, &i, &usSegOffset);
			EQFBGotoSeg(pDoc, pDoc->TBCursor.ulSegNum, usSegOffset);
            EQFBCompSeg( pDoc->pTBSeg );                          //update length of segment
          }
		  pData = pDoc->pEQFBWorkSegmentW + pDoc->TBCursor.usSegOffset;
                                                //get current char
          usChar = *pData;

          // loop til min(end of line, end of segment)
          // check for protected chars, if so, ask user whether to
          // delete or not
          usPos = pDoc->TBCursor.usSegOffset;
          while (usChar != '\0' && usChar != '\n')
          {
             if (EQFBCharType(pDoc,pDoc->pTBSeg,usPos) != UNPROTECTED_CHAR)
             {
                fProtect = TRUE;
             } /* endif */
             usPos ++;
             usChar = * (pDoc->pEQFBWorkSegmentW + usPos);
          } /* endwhile */
          if (fProtect)
          {
             usResult = UtlError( TB_DELPROTECT, MB_YESNOCANCEL | MB_DEFBUTTON3,
                                        0, NULL, EQF_QUERY);
             fProtect = (usResult != MBID_YES);    // yes is:delete tag
          } /* endif */
          if (!fProtect)
          {
             if ( pDoc->pUndoSegW )
             {                                      //update undo buffer
                UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
                pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
                pDoc->fUndoState = FALSE;            // not a typing function
             } /* endif */
             EQFBWorkLeft(pDoc,pDoc->TBCursor.usSegOffset,
                           (USHORT)(usPos - pDoc->TBCursor.usSegOffset));
             // if segment is empty add a blank.
             pData = pDoc->pEQFBWorkSegmentW;
             if ( *pData == EOS )
             {
               *pData++ = BLANK;
               *pData   = EOS;

               EQFBUpdateChangedSeg(pDoc);
             }
             else
             {
               /*******************************************************/
               /* move cursor back into segment (if at last line of   */
               /* segment )                                           */
               /*******************************************************/
               if ( pDoc->TBCursor.usSegOffset && (usChar != LF))
               {
                 pDoc->TBCursor.usSegOffset --;
                 if ( pDoc->lCursorCol > 0 )
                 {
                   pDoc->lCursorCol --;
                 }
                 else
                 {
                   EQFBPhysCursorFromSeg(pDoc); //set sCursorCol/Row
                 } /* endif */
                 EQFBUpdateChangedSeg(pDoc);      // and update length etc.
                 /******************************************************/
                 /* if adjusting Col for DBCS was done                 */
                 /******************************************************/
                 if (EQFBDBCS2ND( pDoc, FALSE))
                 {
                    sBytesInChar = 2;
                 } /* endif */
                 pData = pDoc->pEQFBWorkSegmentW + pDoc->TBCursor.usSegOffset;
                 pDoc->EQFBFlags.EndOfSeg =
                    (*(pData+sBytesInChar) == '\0'); // set EndOfSeg flag
				 fEndOfSeg = pDoc->EQFBFlags.EndOfSeg;
                 EQFBScreenCursorType( pDoc );
               }
               else
               {
                 EQFBUpdateChangedSeg(pDoc);      // and update length etc.
               } /* endif */
             } /* endif */


             // reset any old block mark in same segment
             EQFBFuncResetMarkInSeg( pDoc );
          }
          else
          /************************************************************/
          /* it is protected - check if user selected to delete every-*/
          /* thing except the tags, i.e. the MBID_NO selection.       */
          /************************************************************/
          if ( usResult == MBID_NO )
          {
            USHORT  usActPos;

            usActPos = usPos = pDoc->TBCursor.usSegOffset;
            usChar = *(pDoc->pEQFBWorkSegmentW + usPos );
            while ((usChar != EOS) && (usChar != LF) )
            {
              if (EQFBCharType(pDoc,pDoc->pTBSeg,usPos) != UNPROTECTED_CHAR)
              {
                /******************************************************/
                /* character in tag - will be stored..                */
                /******************************************************/
                *(pDoc->pEQFBWorkSegmentW + usActPos) = (CHAR) usChar;
                usActPos++;
              } /* endif */
              usPos ++;
              usChar = * (pDoc->pEQFBWorkSegmentW + usPos);
            } /* endwhile */
            UTF16strcpy( pDoc->pEQFBWorkSegmentW + usActPos,
                    pDoc->pEQFBWorkSegmentW + usPos );

            /**********************************************************/
            /* update segment values...                               */
            /**********************************************************/
            EQFBUpdateChangedSeg( pDoc );
            EQFBFuncResetMarkInSeg( pDoc );
		  }
		  else
		  {
            /**********************************************************/
            /* update segment values...                               */
            /**********************************************************/
            EQFBUpdateChangedSeg( pDoc );
            EQFBFuncResetMarkInSeg( pDoc );
          } /* endif */

          // insert softlf if necessary...
          if (pDoc->fLineWrap & pDoc->fAutoLineWrap )
          {
             /********************************************************/
             /* insert SoftLFs where needed                          */
             /********************************************************/
             LONG      lEndCol;
             USHORT   usSegOffset = pDoc->TBCursor.usSegOffset;
             EQFBGotoSeg(pDoc, pDoc->TBCursor.ulSegNum, 0);
			 EQFBBufAddSoftLF(pDoc, pDoc->pTBSeg, pDoc->pEQFBWorkSegmentW,
                              pDoc->lCursorCol + pDoc->lSideScroll,
                              &lEndCol,
                              &usSegOffset);
             EQFBGotoSeg(pDoc, pDoc->TBCursor.ulSegNum, usSegOffset);
			 EQFBUpdateChangedSeg(pDoc);            // force update of seg info
			 pDoc->Redraw |= REDRAW_ALL;                 // redraw all
			 // set cursor type
			 pDoc->EQFBFlags.EndOfSeg = fEndOfSeg;
             EQFBScreenCursorType( pDoc );
          } /* endif */
       } /* endif */

   } /* endif */
   return;
 }
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncTruncSeg   - truncate current segment
//------------------------------------------------------------------------------
// Function call:     EQFBFuncTruncSeg( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     truncate current segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if reflow is allowed then
//                      if current char is protected then
//                        beep
//                      else
//                        - display msg if user tries to delete tags
//                        - if user wants to delete then
//                           - while not at end of segment
//                               delete char in worksegment
//                             endwhile
//                           - if linebreak further on in segment,
//                             don't throw it away
//                           - call EQFBUpdateChangedSeg
//                           - set EndOfSeg-Flag if necessary
//                          endif
//                      endif
//                    endif
//
//------------------------------------------------------------------------------

 VOID EQFBFuncTruncSeg
 (
 PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 { USHORT usPos;
   USHORT usChar;                         // character
   PSZ_W  pData;                          // pointer to data
   BOOL   fLinebreak = FALSE;             // true if lf will be deleted
   USHORT usResult = FALSE;               // return from UtlErr etc.
   BOOL   fProtect = FALSE;               // true if protected chars
                                          // to be deleted
   SHORT sBytesinChar = 1;                // 1 = SBCS, 2 = DBCS
   BOOL   fReflow = TRUE;
   BOOL   fHardLF = FALSE;                // no hard LF yet detected in seg


  if ( pDoc->TBCursor.usSegOffset > 0 &&
	  EQFBCharType(pDoc,pDoc->pTBSeg,pDoc->TBCursor.usSegOffset)
				   == PROTECTED_CHAR &&
	  EQFBCharType(pDoc,pDoc->pTBSeg,(USHORT)(pDoc->TBCursor.usSegOffset-1))
				   == PROTECTED_CHAR )
  {  //deleting half of a tag is not allowed
	 WinAlarm( HWND_DESKTOP, WA_WARNING ); // issue a beep if trying on R/o doc
  }
  else
  {
	  EQFBCurSegFromCursor( pDoc );        //set TBCursor
	  EQFBWorkSegCheck( pDoc );            //check if segment changed
	  pData = pDoc->pEQFBWorkSegmentW + pDoc->TBCursor.usSegOffset;

	  usPos = pDoc->TBCursor.usSegOffset;
	  usChar = * (pDoc->pEQFBWorkSegmentW + usPos);
	  while (usChar != '\0')
	  {
		 usResult = EQFBCharType(pDoc,pDoc->pTBSeg,usPos);
		 switch (usResult)
		 {
		   case LINEBREAK_CHAR:
			  fLinebreak = TRUE;
			  if (usChar != SOFTLF_CHAR)
			  {
				  fHardLF = TRUE;
		      }
			  break;
		   case  COMPACT_CHAR:
		   case PROTECTED_CHAR:
			  fProtect = TRUE;
			  break;
		   case HIDDEN_CHAR:
			  fProtect = TRUE;
			  break;
		   default:
			  break;
		 } /* endswitch */
		 usPos ++;
		 usChar = * (pDoc->pEQFBWorkSegmentW + usPos);
	  } /* endwhile */
	  if (!pDoc->EQFBFlags.Reflow && fHardLF)
	  {
		  UtlError( TB_LFCHANGE_NOT_ALLOWED, MB_CANCEL, 0, NULL, EQF_WARNING);
		  fReflow = FALSE;
      }
      if (fReflow)
      {
		  if (fProtect)
		  {
			 usResult = UtlError( TB_DELPROTECT, MB_YESNOCANCEL | MB_DEFBUTTON3,
										  0, NULL, EQF_QUERY);
			 fProtect = (usResult != MBID_YES);    // yes is:delete tag
		  } /* endif */
		  if (!fProtect)
		  {
			 if ( pDoc->pUndoSegW )
			 {                                      //update undo buffer
				UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
				pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
				pDoc->fUndoState = FALSE;            // not a typing function
			 } /* endif */
			 if (fLinebreak && fHardLF)
			 {
			   * pData = '\n';
			   *(pData+1) = '\0';
			 }
			 else
			 {
				*pData = '\0';
				/*******************************************************/
				/* move cursor back into segment                       */
				/*******************************************************/
				if ( pDoc->TBCursor.usSegOffset )
				{
				  pDoc->TBCursor.usSegOffset --;
				  if ( pDoc->lCursorCol > 0 )
				  {
					pDoc->lCursorCol --;
				  }
				  else
				  {
					EQFBPhysCursorFromSeg(pDoc); //set lCursorCol/Row
				  } /* endif */
				} /* endif */
			 } /* endif */

			 // if segment is empty add a blank.
			 if ( *(pDoc->pEQFBWorkSegmentW) == EOS )
			 {
			   pData = pDoc->pEQFBWorkSegmentW;
			   *pData++ = BLANK;
			   *pData   = EOS;
			   EQFBUpdateChangedSeg(pDoc);              //and update length etc.
			 }
			 else
			 {
														// update TBRowOffset table
			   EQFBUpdateChangedSeg(pDoc);              //and update length etc.

												  // set EndOfSeg Flag
			   pData = &(pDoc->pEQFBWorkSegmentW[pDoc->TBCursor.usSegOffset]);
			   if (*pData != '\n')
			   {
				  // if adjusting Col for DBCS was done
				  if (EQFBDBCS2ND( pDoc, FALSE))
				  {
					 sBytesinChar = 2;
				  } /* endif */
				  pDoc->EQFBFlags.EndOfSeg =
					 (*(pData+sBytesinChar) == '\0'); // set EndOfSeg flag
				  EQFBScreenCursorType( pDoc );
			   } /* endif */
			 } /* endif */
			 pDoc->lDBCSCursorCol = pDoc->lCursorCol;
			 // reset any old block mark in same segment
			 EQFBFuncResetMarkInSeg( pDoc );
		   }
		   else
		  /************************************************************/
		  /* it is protected - check if user selected to delete every-*/
		  /* thing except the tags, i.e. the MBID_NO selection.       */
		  /************************************************************/
		  if ( usResult == MBID_NO )
		  {
			USHORT  usActPos;
			BOOL    fLineBreakPending = FALSE;

			usActPos = usPos = pDoc->TBCursor.usSegOffset;
			usChar = *(pDoc->pEQFBWorkSegmentW + usPos );
			while (usChar != EOS)
			{
			  if (EQFBCharType(pDoc,pDoc->pTBSeg,usPos) != UNPROTECTED_CHAR)
			  {
				/******************************************************/
				/* line feed returns != UNPROTECTED_CHAR, too.        */
				/******************************************************/
				if ( usChar == LF )
				{
				  fLineBreakPending = TRUE;
				}
				else
				{
				  if ( fLineBreakPending )
				  {
					*(pDoc->pEQFBWorkSegmentW + usActPos) = LF;
					usActPos++;
					fLineBreakPending = FALSE;
				  } /* endif */
				  /******************************************************/
				  /* character in tag - will be stored..                */
				  /******************************************************/
				  *(pDoc->pEQFBWorkSegmentW + usActPos) = (CHAR) usChar;
				  usActPos++;
				} /* endif */
			  } /* endif */
			  usPos ++;
			  usChar = * (pDoc->pEQFBWorkSegmentW + usPos);
			} /* endwhile */

			/**********************************************************/
			/* avoid joining from lines ....                          */
			/**********************************************************/
			if ( fLinebreak && fHardLF)
			{
			  *(pDoc->pEQFBWorkSegmentW + usActPos) = LF;
			  usActPos++;
			} /* endif */

			*(pDoc->pEQFBWorkSegmentW + usActPos) = EOS;

			/**********************************************************/
			/* update segment values...                               */
			/**********************************************************/
			EQFBUpdateChangedSeg( pDoc );
			EQFBFuncResetMarkInSeg( pDoc );
		  } /* endif */
	  } /* endif */
  } /* endif */
  return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncDeleteLine   - delete line
//------------------------------------------------------------------------------
// Function call:     EQFBFuncDeleteLine( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     delete line (if totally filled with active segment)
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if reflowing allowed then
//                      if line starts with active segment then
//                        if segment fills total line and segment consists
//                         of more than 1 line then
//                           goto start of line
//                           delete this line in worksegment
//                        else
//                           beep
//                        endif
//                      else
//                        beep
//                      endif
//                    endif
//
//------------------------------------------------------------------------------

 VOID EQFBFuncDeleteLine
 (
   PTBDOCUMENT pDoc                        //pointer to Doc instance
 )

 {
   PTBROWOFFSET pTBTemp;                  // pointer to screen row offset
   USHORT usChar;
   PSZ_W  pData;                          //pointer to data
   USHORT usLineFeed = 0;                 //indicator for 2nd line feed
   USHORT usNumber = 0;                   // no. of pos. to shift left
   BOOL fProtect = FALSE;                 // no protected chars to be deleted
   USHORT usResult = FALSE;               //return from UtlErr
   USHORT  usCursorPos;                   // temporary cursor position


   if (pDoc->EQFBFlags.Reflow)            //trunc. only if feflow allowed
   {
       EQFBCurSegFromCursor(pDoc);
       EQFBWorkSegCheck( pDoc );              //check if segment changed

       pTBTemp = pDoc->TBRowOffset+1+pDoc->lCursorRow; // get TB struct from cursor

                         //if line starts with act.segment and
                         //   same seg in remainder of line
       if ((pDoc->TBCursor.ulSegNum == pTBTemp->ulSegNum)
             && (EQFBLineCheck(pDoc) != IND_ENDOFSTRING))
       {
                                             //check if segment consists of
                                             //more than 1 line
          pData = pDoc->pEQFBWorkSegmentW;
                                             //scan til 2nd \n
          while ((usChar = *pData++) != '\0' && usLineFeed != 2 )
          {
             if (usChar =='\n')
             {
                usLineFeed++;
             } /* endif */
          } /* endwhile */

          pTBTemp++;                             //segment not only this line...
          if ( usLineFeed == 2 || (pDoc->TBCursor.ulSegNum == pTBTemp->ulSegNum)  )
          {                                  // delete line
             usCursorPos = pDoc->TBCursor.usSegOffset;   // store old cursor pos
             EQFBFuncStartLine(pDoc);
             pData = pDoc->pEQFBWorkSegmentW + pDoc->TBCursor.usSegOffset;
                                                   //get current char
             while ( *pData++ != '\n' )
             {
                if (EQFBCharType(pDoc,pDoc->pTBSeg,
                   (USHORT)(pDoc->TBCursor.usSegOffset + usNumber)) != UNPROTECTED_CHAR)
                {
                   fProtect = TRUE;
                } /* endif */
                usNumber++;
             } // endwhile
             if (fProtect)
             {
                usResult = UtlError( TB_DELPROTECT,
                            MB_YESNO | MB_DEFBUTTON2, 0, NULL, EQF_QUERY);
                fProtect = (usResult != MBID_YES);    // yes is:delete tag

                // if no delete takes place - position at old cusor pos.
                if ( fProtect )
                {
                                                  // restore old cursor pos
                  pDoc->TBCursor.usSegOffset = usCursorPos;
                  EQFBPhysCursorFromSeg( pDoc );  // set phys.cursor at new offs
                } /* endif */
             } /* endif */
             if (!fProtect)
             {
                if ( pDoc->pUndoSegW )
                {                                  //update undo buffer
                   UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
                   pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
                   pDoc->fUndoState = FALSE;       // not a typing function
                } /* endif */
                usNumber++;                        // delete \n too
                                                   //delete line in worksegment
                EQFBWorkLeft(pDoc,pDoc->TBCursor.usSegOffset, usNumber);

                EQFBUpdateChangedSeg(pDoc);
                // reset any old block mark in same segment
                EQFBFuncResetMarkInSeg( pDoc );
             } /* endif */
          }
          else
          {
             WinAlarm( HWND_DESKTOP, WA_WARNING );          // issue a beep
          } /* endif */
       }
       else
       {
         WinAlarm( HWND_DESKTOP, WA_WARNING );          // issue a beep

       } /* endif */
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // issue a beep if trying on R/o doc
   } /* endif */
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncNextLine   - move to start of next line
//------------------------------------------------------------------------------
// Function call:     EQFBFuncNextLine( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     move to start of next line
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     call EQFBFuncDown
//                    call EQFBFuncStartLine
//
//------------------------------------------------------------------------------

 VOID EQFBFuncNextLine
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 {

   EQFBFuncDown( pDoc );
   EQFBFuncStartLine( pDoc );
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncInsertLine - insert a new blank line
//------------------------------------------------------------------------------
// Function call:     EQFBFuncInsertLine( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     insert a new blank line
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if reflowing allowed then
//                      update Undo-buffer for later Undo
//                      insert a new blank line by calling EQFBReflow
//                      and position cursor by calling EQFBFuncNextLine
//                    else
//                      beep
//                    endif
//
//------------------------------------------------------------------------------

 VOID EQFBFuncInsertLine
 (
 PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 {

   if (pDoc->EQFBFlags.Reflow)             // reflow = TRUE if reflowing allowed
   {
      if ( pDoc->pUndoSegW )
      {
         UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);  //update undo buffer
         pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
         pDoc->fUndoState = FALSE;                      // not a typing function
      } /* endif */
      EQFBReflow(pDoc,TRUE);
      EQFBFuncNextLine(pDoc);
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // issue a beep if trying on R/o doc
   } /*endif*/
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncSplitLine - split current line at the cursor
//------------------------------------------------------------------------------
// Function call:     EQFBFuncSplitLine( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     split current line at the cursor
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:      - if current character is protected then
//                         beep
//                       else
//                         update undo buffer,
//                         split line by calling EQFBReflow
//                       endif
//------------------------------------------------------------------------------
 VOID EQFBFuncSplitLine
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
    BOOL fSplit= TRUE;                    // FALSE if split not allowed


   if ( EQFBCharType(pDoc,pDoc->pTBSeg,pDoc->TBCursor.usSegOffset) ==
                                               PROTECTED_CHAR )

   {
      if (pDoc->TBCursor.usSegOffset > 0)
      {
         if ( EQFBCharType(pDoc,pDoc->pTBSeg,(USHORT)((pDoc->TBCursor.usSegOffset)-1)) ==
                                               PROTECTED_CHAR )
         {
            fSplit = FALSE;
         }
      }
      else
      {
         /*************************************************************/
         /* KBT610: allow if seg starts with inlinetage               */
         /*************************************************************/
         fSplit = TRUE;
      } /* endif */
   } /* endif */

   if (!fSplit)
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // issue a beep if trying on R/o doc
   }
   else
   {
      if ( pDoc->pUndoSegW )
      {
         UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);  //update undo buffer
         pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
         pDoc->fUndoState = FALSE;                      // not a typing function
      } /* endif */
      EQFBReflow(pDoc,FALSE);
   } /* endif */
   return;
 }

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBReflow - reflow line
//------------------------------------------------------------------------------
// Function call:     EQFBReflow( PTBDOCUMENT, BOOL );
//
//------------------------------------------------------------------------------
// Description  :     split current line at the cursor or insert new line
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     if reflowing allowed then
//                      set TBCursor
//                      if right of text then
//                         move til end of line
//                      endif
//                      if INSERT then
//                         loop til end of segment or end of line
//                      endif
//                      insert a LINEMARK ( \n)
//                      call EQFBUpdateChangedSeg
//                      set flags for redraw and changed
//                    endif
//
//------------------------------------------------------------------------------
static
 VOID EQFBReflow
 (
   PTBDOCUMENT pDoc,                      //pointer to Doc instance
   BOOL fInsert                           //true if Insert,FALSE if SPLIT
 )
 {
   USHORT usChar;                         // character
   PSZ_W  pData;                          // pointer to data
   LONG   lLen;                           //return from EQFBCurSegFromCursor

   USHORT usCount;                        //counts how many blanks to skip
                                          //during split

  if (pDoc->EQFBFlags.Reflow)             // reflow = TRUE if reflowing allowed
  {
     lLen = EQFBCurSegFromCursor( pDoc );   //set TBCursor

     if ( lLen < 0 )                        //if cursor right of text,move left
     {                                      //til end of text
        while ( lLen++ < 0)
           EQFBFuncLeft( pDoc );
     } /*endif*/


       pData = pDoc->pEQFBWorkSegmentW + pDoc->TBCursor.usSegOffset;
       usChar = *pData;

//// if fInsert scan for a linebreak in segment or end of segment

     if (fInsert)
     {
       while (usChar != '\n' && (usChar != '\0') )
       {
           ++pData;                           //goto next position
           usChar = *pData;
       } /* endwhile */

     } /* endif */


     memmove(pData+1,pData,UTF16strlenBYTE(pData)+sizeof(CHAR_W));   // make space for newline
     *pData = '\n';                            // insert newline

     if (!fInsert)                             // if split, remove leading blank
     {
        usCount = 1;
        while (*(pData + usCount) == BLANK )   // skip leading blanks next line
        {
           usCount ++;
        } /* endwhile */
        UTF16strcpy(pData+1,pData+usCount);
     }
     else
     {
     } /* endif */
     EQFBUpdateChangedSeg(pDoc);
     pDoc->Redraw |= REDRAW_ALL;

     // get rid of any old block mark available
     EQFBFuncResetMarkInSeg( pDoc );

   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // issue a beep if trying on R/o doc
   } /* endif */
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncJoinLine - join next line to the current
//------------------------------------------------------------------------------
// Function call:     EQFBFuncJoinLine( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     join next line to the current
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - if reflowing allowed then
//                        if no other segment follows in this line then
//                           - update undo buffer
//                           - goto end of line
//                           - Strip trailing blanks from the current line
//                           - Skip leading blanks on the next line
//                           - join the two lines with one blank in between
//                           - if there is a blank between 2 DBCS chars then
//                              delete the blank
//                             endif
//                           - call EQFBUpdateChangedSeg
//                        endif
//                      endif
//
//------------------------------------------------------------------------------
 VOID EQFBFuncJoinLine
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   EQFBFuncJoinLineDo( pDoc, TRUE );
 }
 
 VOID EQFBFuncJoinLineDo( PTBDOCUMENT pDoc, BOOL fInsertBlank )
 {
   PSZ_W   pData;                         // pointer to data
   USHORT  usChar;
   USHORT  usCount;                       // counts blanks
   BOOL    fColumnBased = TRUE;           // indicator if found tag is column based
   USHORT  usRc;

   if (pDoc->EQFBFlags.Reflow)             // reflow = TRUE if reflowing allowed
   {
      usRc = EQFBLineCheck(pDoc);                //same seg in remainder of line?
      if ( usRc != IND_ENDOFSTRING )
      {
        if ( pDoc->pUndoSegW )
        {
           UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);  //update undo buffer
           pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
           pDoc->fUndoState = FALSE;                      // not a typing function
        } /* endif */
        EQFBFuncDoEndLine(pDoc, TRUE);            // KBT413: FuncDoEndLine

        pData = pDoc->pEQFBWorkSegmentW + pDoc->TBCursor.usSegOffset;
        usChar = *pData;
        usCount = 0;                             // scroll to CRLF
        while (*(pData+usCount) == BLANK )
        {
            usCount++;
        } /* endwhile */
        fColumnBased = TACheckColumnPos ( pData+usCount+1,
                                          pDoc->pDocTagTable,
                                          (PTOKENENTRY) pDoc->pTokBuf,
                                          TOK_BUFFER_SIZE,
                                          pDoc->pfnUserExit );

        if ( !fColumnBased )
        {
          if ( fInsertBlank )               // do this only for normal line joines
          {
            UTF16strcpy(pData,pData+usCount);//skip ending blanks
          } /* endif */
                                           //skip leading blanks in next line
          usCount = 0;                     // count til next char != BLANK
                                           // for pData+usCount is source in strcpy
          if (  (usChar == SOFTLF_CHAR) || !fInsertBlank )
          {
            usCount = 1;  // remove (soft) LF char
          }
          else
          {
                                    // set pData to destination for strcpy
            if (*(pData-1) == '\r')
            {
               *(pData-1) = BLANK;
               usCount++;                    // increase usCount,start BLANKS checking
            }                                // at next position
            else
            { 
               *pData = BLANK;               //insert a blank
               pData++;
            } /* endif */
          } /* endif */

          if ( fInsertBlank )               // do this only for normal line joines
          {
            while (*(pData+usCount) == BLANK)  // skip leading blanks in next line
            {
               usCount++;
            } /* endwhile */
          } /* endif */

          UTF16strcpy(pData,pData+usCount);
          if (pDoc->TBCursor.usSegOffset == 0 )
          {
            /**********************************************************/
            /* donot insert a blank at begin of segment               */
            /**********************************************************/
            pData = pDoc->pEQFBWorkSegmentW;
            if (*pData == BLANK )
            {
              UTF16strcpy(pData, pData +1);
            } /* endif */
          }
          else if (IsDBCS_CP(pDoc->ulOemCodePage))
          {
            // if there is a blank between 2 DBCS chars, delete the blank
            if (EQFIsDBCSChar( pDoc->pEQFBWorkSegmentW[pDoc->TBCursor.usSegOffset-1], pDoc->ulOemCodePage) &&
                EQFIsDBCSChar( pDoc->pEQFBWorkSegmentW[pDoc->TBCursor.usSegOffset+1], pDoc->ulOemCodePage))
            {
              pData = pDoc->pEQFBWorkSegmentW + pDoc->TBCursor.usSegOffset;
              if (*pData == BLANK )
              {
                UTF16strcpy(pData, pData +1);
              } /* endif */
            } /* endif */
          } /* endif */

          EQFBUpdateChangedSeg(pDoc);

          pDoc->Redraw |= REDRAW_ALL;

          // reset any old block mark in same segment
          EQFBFuncResetMarkInSeg( pDoc );
        }
        else
        {
          /************************************************************/
          /* go back to old position: EQFBFuncEndLine has been done   */
          /************************************************************/
          EQFBGotoSeg(pDoc,pDoc->ulWorkSeg,pDoc->usUndoSegOff);
        } /* endif */

      } /* endif */
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // issue a beep if trying on R/o doc
   } /* endif */


   return;
 }
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBLineCheck - check if same seg in remainder of line
//------------------------------------------------------------------------------
// Function call:     EQFBLineCheck( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     check if same seg in remainder of line
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       IND_LINEFEED  other seg in line
//                    IND_ENDOFSTRING - other seg follows in line
//                    IND_SOFTLF - soft linefeeds ends this line
//------------------------------------------------------------------------------
// Function flow:     if not right of text
//                      loop til end of line or end of segment
//                      if end of segment comes first,
//                             set return = FALSE;
//                      endif
//                    endif
//
//------------------------------------------------------------------------------
 static
USHORT EQFBLineCheck
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
{
   USHORT usChar;
   PTBSEGMENT  pSeg;                     // pointer to segment
   PSZ_W   pData;                                // pointer to error data
   USHORT  usNext = BLANK;
   USHORT  usPrevious = BLANK;
   LONG    lLen;

   USHORT  usRc = IND_ENDOFSTRING;


   lLen = EQFBCurSegFromCursor(pDoc);
   if ( lLen < 0)
   {
     EQFBFuncPad( pDoc, lLen ); // pad with blanks...
   } /* endif */

   pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );

   pData = pSeg->pDataW + pDoc->TBCursor.usSegOffset;
   usChar = *pData;
   usNext = *(pData+1);
   if (pDoc->TBCursor.usSegOffset )
   {
     usPrevious = *(pData-1);
   } /* endif */
   //loop til end of line or end  of segment

   while (usChar != '\n' && usChar != '\0'
              && !ISLF(usChar, usNext, usPrevious) )
   {
     pData++;
     usPrevious = usChar;
     usChar = usNext;
     usNext = *(pData+1);
   } /* endwhile */

   switch ( usChar )
   {
     case '\n':
       usRc = IND_LINEFEED;
       break;
     case '\a':
       usRc = IND_SOFTLF;
       break;
     default:
     case '\0':
       usRc = IND_ENDOFSTRING;
       break;
   } /* endswitch */
   return(usRc);
}
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBCursorScrollUp
//------------------------------------------------------------------------------
// Function call:     EQFBCursorScrollUp(PTBDOCUMENT);
//------------------------------------------------------------------------------
// Description:       scroll screen up one line
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     only for scroll up key - with one parameter
//                    call EQFBFuncScrollUp(pDoc,TRUE)
//------------------------------------------------------------------------------

 VOID EQFBCursorScrollUp
 (
   PTBDOCUMENT pDoc                        //pointer to Doc instance
 )
 {

  EQFBFuncScrollUp(pDoc,TRUE);       //scroll and set TBCursor
   return;
 }
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncScrollUp
//------------------------------------------------------------------------------
// Function call:     EQFBFuncScrollUp(PTBDOCUMENT,BOOL);
//------------------------------------------------------------------------------
// Description:       scroll screen up one line
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc    ptr to doc instance data
//                    BOOL sTBCursor      indicator whether to update TBCursor
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     scroll screen up one line
//                    reset TBRow table
//                    set TBCursor if fTBCursor is true
//------------------------------------------------------------------------------

 VOID EQFBFuncScrollUp
 (
   PTBDOCUMENT pDoc,                       //pointer to Doc instance
   BOOL fTBCursor                          //if true: recalculate TBCursor
 )

 {
    LONG         lRow;                    // row where to start
    PTBROWOFFSET pTBRow;                  // pointer to row/offset

    if (EQFBLineDown( pDoc ) == 0)
    {
      // scroll TBRowOffset  structure one line up and fill last line
      memmove(pDoc->TBRowOffset, (pDoc->TBRowOffset+1 ),
               sizeof( TBROWOFFSET ) * (pDoc->lScrnRows+1) );

      lRow = pDoc->lScrnRows;           // temporary row variable

      EQFBScrnLinesFromSeg ( pDoc,
                             (lRow - 1),
                             1,
                             (pDoc->TBRowOffset+lRow) );
      pDoc->ulVScroll ++;
      if (fTBCursor)
      {
         pDoc->lCursorCol = pDoc->lDBCSCursorCol;
         EQFBCurSegFromCursor( pDoc );    // find new cursor segment
         EQFBDBCS2ND ( pDoc, FALSE);       // adjust lCursorCol if 2ndDBCS
      } /* endif */
      pDoc->Redraw |= REDRAW_ALL;
   }
   else
   {
      pTBRow = pDoc->TBRowOffset+pDoc->lCursorRow+2 ;  // screen - row table offset
      if ( pTBRow->ulSegNum > 0)
      {
         ++pDoc->lCursorRow;
         pDoc->lCursorCol = pDoc->lDBCSCursorCol;
         EQFBCurSegFromCursor( pDoc );           // find new cursor segment
         EQFBDBCS2ND ( pDoc, FALSE);            // adjust lCursorCol if 2ndDBCS
      }
   } /* endif */

   if ( fTBCursor )                                                 /* @39A */
   {                                                                /* @39A */
     EQFBWorkSegCheck( pDoc );                 //check if segment changed
   } /* endif */                                                    /* @39A */
   return;
 }
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBCursorScrollDown
//------------------------------------------------------------------------------
// Function call:     EQFBCursorScrollDown(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       scroll screen down one line
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     only for ScrollDown key, with one parameter
//                    call EQFBFUncScrollDown(pDoc,TRUE);
//------------------------------------------------------------------------------

 VOID EQFBCursorScrollDown
 (
   PTBDOCUMENT pDoc                        //pointer to DOc instance
 )
 {

    EQFBFuncScrollDown(pDoc,TRUE);  // scroll and set TBCUrsor
   return;
 }
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncScrollDown
//------------------------------------------------------------------------------
// Function call:     EQFBFuncScrollDown(PTBDOCUMENT,BOOL);
//------------------------------------------------------------------------------
// Description:       scroll screen dddown one line
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc    ptr to doc instance data
//                    BOOL fTBCursor      indicator whether to update TBCursor
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     scroll screen down one line
//                    reset TBRow table
//                    set TBCursor if fTBCursor is true
//------------------------------------------------------------------------------

 VOID EQFBFuncScrollDown
 (
   PTBDOCUMENT pDoc,                       //pointer to DOc instance
   BOOL fTBCursor                          //if true: recalculate TBCursor
 )
 {
   PTBROWOFFSET pTBRow;                   // row offset pointer

   if (EQFBLineUp( pDoc ) == 0)
   {
      // scroll TBRowOffset  structure one line down and fill top row
      memmove((pDoc->TBRowOffset+1), (pDoc->TBRowOffset),
               sizeof( TBROWOFFSET ) * (pDoc->lScrnRows+1) );

      EQFBFillPrevTBRow( pDoc,  1 );   // pointer to row offset
      pDoc->ulVScroll --;

      if (fTBCursor)
      {
         pDoc->lCursorCol = pDoc->lDBCSCursorCol;
         EQFBCurSegFromCursor( pDoc );    // find new cursor segment
         EQFBDBCS2ND ( pDoc, FALSE);       // adjust lCursorCol if 2ndDBCS
      } /* endif */

      pDoc->Redraw |= REDRAW_ALL;
   }
   else
   {
      pTBRow = pDoc->TBRowOffset+pDoc->lCursorRow ; // screen - row offset

      if ( pTBRow->ulSegNum > 0)
      {
          --pDoc->lCursorRow;
          pDoc->lCursorCol = pDoc->lDBCSCursorCol;
          EQFBCurSegFromCursor( pDoc );            // find new cursor segment
          EQFBDBCS2ND ( pDoc, FALSE);       // adjust lCursorCol if 2ndDBCS
      } /* endif */
   }
   if ( fTBCursor )                                                 /* @39A */
   {                                                                /* @39A */
     EQFBWorkSegCheck( pDoc );                 //check if segment changed
   } /* endif */                                                    /* @39A */
   return;
 }
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncScrollLeft
//------------------------------------------------------------------------------
// Function call:     EQFBFuncScrollLeft(PTBDOCUMENT);
//------------------------------------------------------------------------------
// Description:       scroll screen left one position
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     call EQFBFuncScrollIncLeft(pDoc,1)
//------------------------------------------------------------------------------

 VOID EQFBFuncScrollLeft
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

{

   EQFBFuncScrollIncLeft( pDoc, 1 );       // shift only one
   return;
 }
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncScrollIncLeft
//------------------------------------------------------------------------------
// Function call:     EQFBFuncScrollIncLeft(PTBDOCUMENT,SHORT);
//------------------------------------------------------------------------------
// Description:       scroll screen left SIDE_DELTA positions
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc     ptr to doc instance
//                    SHORT   sSideDelta   number of scroll positions
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if not at maximum of segment size
//                      scroll screen left SIDE_DELTa positions
//                      set TBCursor
//------------------------------------------------------------------------------
 static
 VOID EQFBFuncScrollIncLeft
 (
   PTBDOCUMENT pDoc,                      //pointer to Doc instance
   SHORT       sSideDelta                 // number of scroll positions
 )

 {
   PTBSEGMENT  pSeg;                      //pointer to segment struct
   USHORT usChar;

   if (pDoc->lSideScroll + sSideDelta <= MAX_SEGMENT_SIZE - pDoc->lScrnCols)
   {
      pDoc->lSideScroll = pDoc->lSideScroll + sSideDelta;
      pDoc->lCursorCol -=( sSideDelta - 1);// adjust cursor position

      EQFBCurSegFromCursor( pDoc );         // find new cursor segment
      EQFBDBCS2ND ( pDoc, TRUE);            // adjust lCursorCol if 2ndDBCS

      pDoc->Redraw |= REDRAW_ALL;
   }
   else if (pDoc->lCursorCol < pDoc->lScrnCols-1)
   {
     /*****************************************************************/
     /* if DBCS codepage & cur. char DBCS_1ST, move 2 bytes right     */
     /* it is assumed that cursor cannot be on a 2nd DBCS byte        */
     /*****************************************************************/
     if (IsDBCS_CP(pDoc->ulOemCodePage))                  //if DBCS codepage
     {
       pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );
       usChar = *(pSeg->pDataW + pDoc->TBCursor.usSegOffset);
       if (EQFIsDBCSChar(usChar, pDoc->ulOemCodePage) )
       {
         pDoc->lCursorCol++;
       } /* endif */
     } /* endif */
      ++pDoc->lCursorCol;
      EQFBCurSegFromCursor( pDoc );       // find new cursor segment
      EQFBDBCS2ND ( pDoc, TRUE);          // adjust lCursorCol if 2ndDBCS

   }
   else
   {
      UtlError( TB_TOOLONG, MB_CANCEL, 0, NULL, EQF_ERROR);
   } /* endif */
   pDoc->lDBCSCursorCol = pDoc->lCursorCol;   //set to actual Col
   EQFBWorkSegCheck( pDoc );               //check if segment changed
   return;
 }
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncScrollRight
//------------------------------------------------------------------------------
// Function call:     EQFBFuncScrollRight(PTBDOCUMENT);
//------------------------------------------------------------------------------
// Description:       scroll screen right one position
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc         ptr to doc instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     call EQFBFuncScrollIncRight (pDoc,1)
//------------------------------------------------------------------------------

 VOID EQFBFuncScrollRight
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 {

   EQFBFuncScrollIncRight( pDoc , 1 );    // scroll one position to the right
   return;
 }
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncScrollIncRight
//------------------------------------------------------------------------------
// Function call:     EQFBFuncScrollIncRight(PTBDOCUMENT,SHORT)
//------------------------------------------------------------------------------
// Description:       scroll screen right given number of positions
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT     pDOc  ptr to doc instance
//                    SHORT           sSideDelta number of pos to scroll
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if not at left end:
//                      scroll screen rigth passed number of positions
//                      reset TBCursor
//------------------------------------------------------------------------------
 static
 VOID EQFBFuncScrollIncRight
 (
   PTBDOCUMENT pDoc,                      // pointer to Doc instance
   SHORT       sSideDelta                 // number of positions to scroll
 )
 {
   LONG  lSideScroll;

   if ( pDoc->lSideScroll > 0 )
   {
      lSideScroll = min( pDoc->lSideScroll, sSideDelta );
      pDoc->lSideScroll = pDoc->lSideScroll - lSideScroll ;
      pDoc->lCursorCol +=( lSideScroll - 1);// adjust cursor position
      EQFBCurSegFromCursor( pDoc );       // find new cursor segment
      EQFBDBCS2ND ( pDoc, FALSE);         // adjust lCursorCol if 2ndDBCS
      pDoc->Redraw |= REDRAW_ALL;
   }
   else if ( pDoc->lCursorCol > 0 )
   {
      lSideScroll = min( pDoc->lCursorCol, sSideDelta );
      pDoc->lCursorCol = pDoc->lCursorCol - lSideScroll;
      EQFBCurSegFromCursor( pDoc );       // find new cursor segment
      EQFBDBCS2ND ( pDoc, FALSE);         // adjust CursorCol if 2ndDBCS
   } /* endif */

   pDoc->lDBCSCursorCol = pDoc->lCursorCol;   //set to actual Col

   EQFBWorkSegCheck( pDoc );              //check if segment changed
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncPageUp
//------------------------------------------------------------------------------
// Function call:     EQFBFUncPageUp (PTBDOCUMENT);
//------------------------------------------------------------------------------
// Description:       scroll up one page
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc    ptr to doc instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     call EQFBFuncScrollDown ScrnRows many times
//                    if we are now on the top line, force it to the first
//                     line of the data area
//                    set TBCursor
//------------------------------------------------------------------------------

 VOID EQFBFuncPageUp
 (
   PTBDOCUMENT pDoc                       //pointer to DOc instance
 )

 {
   int i;

   for (i = 0; i < pDoc->lScrnRows-1; i++)
   {
      EQFBFuncScrollDown( pDoc,FALSE );
   } /* endfor */

//    If we are now on the top line, force it to the first
//    line of the data area.

   if ( (pDoc->TBRowOffset + 1)->ulSegNum == 0)
   {
      EQFBFuncTopDoc( pDoc );
   } /* endif */

   EQFBCurSegFromCursor( pDoc );    // find new cursor segment
   EQFBDBCS2ND ( pDoc, FALSE);          // adjust CursorCol if 2ndDBCS
   pDoc->Redraw |= REDRAW_ALL;
   EQFBWorkSegCheck(pDoc);                                            /* @39A */
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncPageDown
//------------------------------------------------------------------------------
// Function call:     EQFBFUncPageDown(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       scroll down one page
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc    ptr to doc instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     call EQFBFuncScrollUp   ScrnRows many times
//                    if we are now on the lastline, force it to the end
//                     line of the data area
//                    set TBCursor
//------------------------------------------------------------------------------

 VOID EQFBFuncPageDown
 (
 PTBDOCUMENT pDoc                               //pointer to Doc instance
 )

 {
   SHORT i;

   for (i = 0; i < pDoc->lScrnRows-1; i++)
      EQFBFuncScrollUp( pDoc ,FALSE);

//    If we are now on the last line, force it to the end
//    line of the data area.
   if ((pDoc->TBRowOffset + pDoc->lScrnRows)->ulSegNum == 0)
   {
      EQFBFuncBottomDoc( pDoc );
   } /* endif */
   EQFBCurSegFromCursor( pDoc );    // find new cursor segment
   EQFBDBCS2ND ( pDoc, FALSE);          // adjust CursorCol if 2ndDBCS
   EQFBWorkSegCheck(pDoc);
                                                                      /* @39A */
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncInsToggle
//------------------------------------------------------------------------------
// Function call:     EQFBFuncInsToggle(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       toggle insert/replace mode
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc   pointer to document instance data
//------------------------------------------------------------------------------
// Function flow:     toggle insert/replace  indicator
//                    set screen cursor type
//------------------------------------------------------------------------------

 VOID EQFBFuncInsToggle
 (
   PTBDOCUMENT pDoc                       //pointer to DOc instance
 )
 {
    pDoc->EQFBFlags.inserting = !pDoc->EQFBFlags.inserting;
    EQFBScreenCursorType( pDoc );
    return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncUndo
//------------------------------------------------------------------------------
// Function call:     EQFBFuncUndo(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       undo changes to current segment in parts
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT         pointer to doc instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if something changed and backup exists
//                      if temp storage can be allocated
//                        fill temp storage with old worksegment
//                        store Cursor.SegOffset
//                        copy undobuffer back in worksegment
//                        set TBCursor.usSEgOffset to position of Undo
//                        fill Undobuffer with content of temp storage
//                      else
//                        copy Undobuffer in worksegment
//                      endif
//                      set state that it is not a typing function
//                      recalc cursorcol and row
//                      actualize sDBCSCursorCol
//                      set workseg changed flag
//                      update length of segment
//                      update TBRowOFfset table
//                      force update
//                      reset old block mark
//                    endif
//------------------------------------------------------------------------------

 VOID EQFBFuncUndo
 (
    PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   PSZ_W  pTemp;                           //buffer for consecutive Undo
   USHORT usTempSegOff;                    //Temp.storing TBCursor.SegOffset

   if ( (pDoc->EQFBFlags.workchng || pDoc->fFuzzyCopied) && pDoc->pUndoSegW )
   {

      /****************************************************************/
      /* allow toggle of content of WorkSeg with pUndoSeg             */
      /****************************************************************/

      UtlAlloc((PVOID *)&pTemp,
                    0L, (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W),ERROR_STORAGE);
      if ( pTemp )
      {
        UTF16strcpy(pTemp,pDoc->pEQFBWorkSegmentW);     // fill pTemp to interchange
        usTempSegOff = pDoc->TBCursor.usSegOffset;
        UTF16strcpy( pDoc->pEQFBWorkSegmentW, pDoc->pUndoSegW );
        pDoc->TBCursor.usSegOffset = pDoc->usUndoSegOff;
        UTF16strcpy(pDoc->pUndoSegW, pTemp);       // contents of WorkSeg + pUndoSeg
        pDoc->usUndoSegOff = usTempSegOff;
        UtlAlloc((PVOID *)&pTemp, 0L, 0L, NOMSG);     // free space
      }
      else
      {
        UTF16strcpy( pDoc->pEQFBWorkSegmentW, pDoc->pUndoSegW );
        pDoc->TBCursor.usSegOffset = pDoc->usUndoSegOff;
      } /* endif */
      pDoc->fUndoState = FALSE;              // not a typing function
      EQFBCompSeg( pDoc->pTBSeg );                          //update length of segment
                                                 //goto old position
      EQFBGotoSeg(pDoc,pDoc->ulWorkSeg,pDoc->TBCursor.usSegOffset),

//    pDoc->sSideScroll = 0;
//    EQFBPhysCursorFromSeg( pDoc );         // set cursor at new offset
//    pDoc->sDBCSCursorCol = pDoc->sCursorCol;   //set to actual Col
/**********************************************************************/
/* code from EQFBUPdatChangedSeg,but with diff. EQFBScrnLInesFromSeg  */
/* and without EQFBCurSegFromCursor                                   */
/**********************************************************************/
//    pDoc->EQFBFlags.workchng = TRUE;              // sth changed in workseg
//    pSeg = EQFBGetSeg( pDoc, pDoc->ulWorkSeg );   // pointer to segment
//    EQFBCompSeg( pSeg );                          //update length of segment
//                                                  // update TBRowOffset table
//    EQFBScrnLinesFromSeg ( pDoc,                  // pointer to doc ida
//                           0,                     // starting row
//                           pDoc->sScrnRows,      // number of rows
//                                                  // starting segment
//                           (pDoc->TBRowOffset+1));
      pDoc->EQFBFlags.workchng = TRUE;              // sth changed in workseg
      // reset any old block mark in same segment
      EQFBFuncResetMarkInSeg( pDoc );
      pDoc->pTBSeg->SegFlags.Typed = TRUE;

      MTLogStartEditing( pDoc );     

      pDoc->ActSegLog.usNumTyped ++;

   } /* endif */
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncEscape
//------------------------------------------------------------------------------
// Function call:     EQFBFuncEscape(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       accept any character as an ASCII key
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     beep ; function not possible yet
//------------------------------------------------------------------------------

 VOID EQFBFuncEscape
 (
    PTBDOCUMENT pDoc                       //pointer to Doc instance
 )

 {
   pDoc;                                  // get rid of compiler warning
   WinAlarm( HWND_DESKTOP, WA_ERROR);

   return;
 }




//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncDispOrg
//------------------------------------------------------------------------------
// Function call:     EQFBFuncDispOrg(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       display the original
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc ptr to document instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if in target document
//                      if not visible synchronize if first
//                      show and activate source window
//                    else
//                      beep
//                    endif
//------------------------------------------------------------------------------

VOID  EQFBFuncDispOrg
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
  /********************************************************************/
  /* function is only active if in TARGET_DOCUMENT                    */
  /********************************************************************/
  if ( pDoc->docType == STARGET_DOC )
  {
    if ( pDoc->tbActSeg.ulSegNum  &&
           ! WinIsWindowVisible( pDoc->twin->hwndFrame ) )
    {
       EQFBGotoSeg( pDoc->twin,
                    pDoc->tbActSeg.ulSegNum,
                    0 );              // position source document
    } /* endif */
    WinShowWindow( pDoc->twin->hwndFrame, TRUE );
    SendMessage( ((PSTEQFGEN) pDoc->pstEQFGen)->hwndTWBS, WM_EQF_SETFOCUS,
                   0, MP2FROMHWND( pDoc->twin->hwndFrame ));
  }
  else
  {
    while ( pDoc->docType != SSOURCE_DOC)
    {
      pDoc = pDoc->next;
    } /* endwhile */
    /********************************************************/
    /* user wants to go to this document                    */
    /********************************************************/
    pDoc->Redraw |= REDRAW_ALL;
                                // activate original window
    WinShowWindow( pDoc->hwndFrame, TRUE );
    SendMessage( ((PSTEQFGEN) pDoc->pstEQFGen)->hwndTWBS, WM_EQF_SETFOCUS,
                   0, MP2FROMHWND( pDoc->hwndFrame ));
  } /* endif */
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncPad
//------------------------------------------------------------------------------
// Function call:     EQFBFuncPad(PTBDOCUMENT,
//------------------------------------------------------------------------------
// Description:       pad current line with blanks
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pointer to document instance data
//                    SHORT   number of blanks to be padded
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     move rest of segment to the right passed number of pos
//                    fill active segment from cursor pos with blanks
//                    set new segment length
//                    free segment table
//                    calculate new cursor segment number /offset
//------------------------------------------------------------------------------

VOID EQFBFuncPad
(
    PTBDOCUMENT  pDoc,                             // pointer to document ida
    LONG         lPos                              // number of blanks
)
{
   PSZ_W pData;                                    // pointer to data segment
   PSZ_W pEnd;                                     // pointer to end
   LONG i;

   pData = pDoc->pEQFBWorkSegmentW+pDoc->TBCursor.usSegOffset;
   pEnd = pDoc->pEQFBWorkSegmentW + UTF16strlenCHAR( pDoc->pEQFBWorkSegmentW );

   memmove( pData - lPos, pData, (pEnd - pData + 1)*sizeof(CHAR_W));
   //memset( pData, BLANK, -sPos * sizeof(CHAR_W) );                  // pad with zeros
   for ( i = 0; i < -lPos; i ++)
   {
     *pData++ = ' ';
   } /* endfor */


   pDoc->pTBSeg->usLength = (USHORT)(pDoc->pTBSeg->usLength - lPos );       // set new segment length
   UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusBPET) ,0L ,0L , NOMSG);     // free segment table

   if (pDoc->pTBSeg->pusHLType )
   {
     pDoc->pTBSeg->SegFlags.Spellchecked = FALSE;
     if (!pDoc->fAutoSpellCheck )
     {
       UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusHLType),0L,0L,NOMSG);
     } /* endif */
   }
   EQFBCurSegFromCursor( pDoc );                   // get new cursor pos
   return;
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncDispSrcProp
//------------------------------------------------------------------------------
// Function call:     EQFBFuncDispSrcProp(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       display the source of the proposals
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc ptr to document instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------

VOID  EQFBFuncDispSrcProp
(
    PTBDOCUMENT  pDoc                  // pointer to document ida
)
{
  pDoc;                                // avoid compiler warnings
  /********************************************************************/
  /* call function of services                                        */
  /********************************************************************/
  if (!pDoc->EQFBFlags.PostEdit)
  {
    EQFGETSOURCE( EQF_ACTIVATE );
  }

} /* end of function EQFBFuncDispSrcProp  */

// display segment properties window
VOID  EQFBFuncDispSegProp
(
    PTBDOCUMENT  pDoc                  // pointer to document ida
)
{
  if (!pDoc->EQFBFlags.PostEdit)
  {
    // ensure that the passed document is the STARGET_DOC!
    PTBDOCUMENT pTargetDoc = pDoc;
    while ( pTargetDoc->docType != STARGET_DOC )
    {
      if ( pTargetDoc->next )
      {
        pTargetDoc = pTargetDoc->next;
        if ( pTargetDoc == pDoc )
        {
          break;   // avoid wrap-around
        } /* endif */
      }
      else
      {
        break; // no next document
      } /* endif */
    } /*endwhile */

    MDStartDialog( pTargetDoc );

    // refresh with data from current segment
    {
      PTBSEGMENT pSeg = EQFBGetSegW( pTargetDoc, pTargetDoc->tbActSeg.ulSegNum );
      if ( pSeg )
      {
        PSZ_W pszContext = EQFBGetContext( pTargetDoc, pSeg, pTargetDoc->tbActSeg.ulSegNum );
        pTargetDoc->szContextBuffer[0] = 0;

        if ( (pszContext != NULL) && (*pszContext != 0) && (pTargetDoc->pfnFormatContext != NULL) )
        {
          (pDoc->pfnFormatContext)( pszContext, pTargetDoc->szContextBuffer );
        } /* endif */             
        MDRefreshMetadata( pTargetDoc, pSeg, pTargetDoc->szContextBuffer );
      } /* endif */
    }
  }

} /* end of function EQFBFuncDispSrcProp  */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncDelSel
//------------------------------------------------------------------------------
// Function call:     EQFBFuncDelSel(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       delete the selection
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc ptr to document instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------
BOOL  EQFBFuncDelSel
(
    PTBDOCUMENT  pDoc                  // pointer to document ida
)
{
  BOOL fDel = FALSE;

    PEQFBBLOCK  pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;        // pointer to block struct

    // redo blockmark in current segment at character input
    if ( pstBlock->pDoc == pDoc &&
         (pstBlock->ulSegNum == pDoc->ulWorkSeg) &&
         (pstBlock->ulEndSegNum == pDoc->ulWorkSeg) )
    {
      fDel = TRUE;
      EQFBFuncMarkDelete( pstBlock->pDoc );
      pstBlock->pDoc = NULL;                           // reset block mark
    }
    else
    {
      /****************************************************************/
      /* if blockmark is in current doc, donot delete one char        */
      /****************************************************************/
      if (pstBlock->pDoc == pDoc )
      {
        fDel = TRUE;                   //donot delete a char
      } /* endif */
      EQFBFuncResetMarkInSeg( pDoc );
    } /* endif */

  return fDel;
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncExactStartSeg - move the cursor to start of seg
//------------------------------------------------------------------------------
// Function call:     EQFBFuncExactStartSeg( PTBDOCUMENT )
//
//------------------------------------------------------------------------------
// Description  :     move the cursor to start of the segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     - set ptr to segment,
//                    - set TBCursor.usSegOffset to begin of segment
//                    - get physical cursor positions of startseg
//                    - if cursor at end of segment and next
//                      segment in same line then
//                         set EndOfSeg- flag for triple-Mode
//                         update screen cursor type
//                      endif
//
//------------------------------------------------------------------------------

 VOID EQFBFuncExactStartSeg
 (
   PTBDOCUMENT pDoc                       //pointer to Doc instance
 )
 {
   PTBSEGMENT  pSeg;                   // pointer to segment
   PSZ_W  pData;                       // pointer to data
   LONG   lPos = 0;                    // line position

   /******************************************************************/
   /* reset selection if shiftkey not set                            */
   /******************************************************************/
   RESETSELECTION( pDoc );

   (pDoc->TBCursor).usSegOffset  = 0;  // start at the beginning
   pSeg = EQFBGetVisSeg( pDoc, &pDoc->TBCursor.ulSegNum ); // check if visible


   EQFBPhysCursorFromSeg ( pDoc );        // pointer to doc ida
                                          //loop while char is protected

   pDoc->EQFBFlags.EndOfSeg = FALSE;
   EQFBScreenCursorType( pDoc );

   /*******************************************************************/
   /* check how many characters belong to this segment                */
   /*******************************************************************/
   pData = pSeg->pDataW + pDoc->TBCursor.usSegOffset ;
   if (IsDBCS_CP(pDoc->ulOemCodePage))
   {
  	   CHAR_W c;
  		while( ((c =*pData) != NULC) && (*pData != LF))
  		{
  			if (EQFIsDBCSChar(c, pDoc->ulOemCodePage))
  			{
  				lPos += 2;
  		    }
  		    else
  		    {
  				lPos ++;
  		    }
  			pData++;
  	    } /* endwhile */
   }
   else
   {
        while ( *pData && (*pData != LF) )
        {
          pData++;
          lPos++;
        } /* endwhile */
   }
   /*******************************************************************/
   /* if we are too far on the right hand border, set sCursorCol in   */
   /* such a way, that segment starts at column 10 (SIDE_DELTA        */
   /*******************************************************************/
   if ( ((pDoc->lCursorCol + SIDE_DELTA) > pDoc->lScrnCols)
         && (lPos > SIDE_DELTA) )
   {
     lPos  = min( lPos + pDoc->lCursorCol + 1 - pDoc->lScrnCols,
                   (pDoc->lCursorCol - SIDE_DELTA) ) ;
     pDoc->lSideScroll = pDoc->lSideScroll + lPos;
     pDoc->lCursorCol  = pDoc->lCursorCol  - lPos;
   } /* endif */


   pDoc->lDBCSCursorCol = pDoc->lCursorCol;

   return;
 }



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncRedo
//------------------------------------------------------------------------------
// Function call:     EQFBFuncRedo(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       redo changes to current segment in parts
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT         pointer to doc instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------

VOID EQFBFuncRedo
(
   PTBDOCUMENT pDoc                       //pointer to Doc instance
)
{

  if ( pDoc->hwndRichEdit )
  {
    EQFBRedoRTF( pDoc );
  }
  else
  {
    EQFBFuncNothing( pDoc );
  } /* endif */
  return;
}

// Compare cursor position with absolut position
BOOL EQFBRelCurPos(  PTBDOCUMENT pDoc, LONG lCol, LONG lAbsPos  )
{
	if ((pDoc->pUserSettings && pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay)
	     || IsDBCS_CP(pDoc->ulOemCodePage))
    {  // here in BIDI IBM is displayed as MBI
		if ( pDoc->pArabicStruct && pDoc->pArabicStruct->plCaretPos)
		{
		   PLONG plCaretPos = pDoc->pArabicStruct->plCaretPos + pDoc->lCursorRow * (pDoc->lScrnCols+1);
		   return  ( (lCol <= pDoc->lScrnCols)&& (plCaretPos[lCol] >= lAbsPos * pDoc->cx) );
		}
		else
		{
		   return FALSE;
	   }
    }
    else
    {
		return FALSE;
    }
}

// return the absolute position (i.e. depending on the characters/DBCS stuff displayed)
LONG EQFBGetAbsPos(  PTBDOCUMENT pDoc, LONG  lCol  )
{
	if ((pDoc->pUserSettings && pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay)
	     || IsDBCS_CP(pDoc->ulOemCodePage))
	{  // here in BIDI IBM is displayed as MBI
		if ( pDoc->pArabicStruct && pDoc->pArabicStruct->plCaretPos)
		{
		   PLONG plCaretPos = pDoc->pArabicStruct->plCaretPos + pDoc->lCursorRow * (pDoc->lScrnCols+1);
		   return  plCaretPos[lCol] ;
		}
		else
		{
		   return (pDoc->cx * lCol);
	   }
    }
    else
    {
		return (pDoc->cx * lCol);
    }
}

// Add for R012027 start
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncInsertChar - insert a character
//------------------------------------------------------------------------------
// Function call:     EQFBFuncInsertChar( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     insert a new character
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if reflowing allowed then
//                      update Undo-buffer for later Undo
//                      insert a character by calling EQFBReflow
//                    else
//                      beep
//                    endif
//
//------------------------------------------------------------------------------
VOID EQFBFuncInsertChar
(
    PTBDOCUMENT pDoc                       //pointer to Doc instance
)
{
    SPECCHARW specCharItm;

    memset(specCharItm.wstrChar,      0x00, sizeof(specCharItm.wstrChar));
    memset(specCharItm.wstrCharUni,   0x00, sizeof(specCharItm.wstrCharUni));
    memset(specCharItm.wstrCharName,  0x00, sizeof(specCharItm.wstrCharName));
    memset(specCharItm.wstrCharKey,   0x00, sizeof(specCharItm.wstrCharKey));

    QueryInsSpecCharW((UCHAR) pDoc->ucCode, pDoc->ucState, &specCharItm);

    if ((NULL == specCharItm.wstrChar) || (wcslen(specCharItm.wstrChar) == 0))
    {
        return;
    }

    wchar_t * wstrInsChar = specCharItm.wstrChar;
    if (pDoc->hwndRichEdit)
    {
        EQFBReplaceSelRTF(pDoc, specCharItm.wstrChar);
    }
    else
    {
        while ( (pDoc->usChar = *wstrInsChar) != NULC)  // insert word in active seg
        {                                // simulate as of keystrokes
            EQFBFuncCharacter( pDoc );     // therefore don't need cursor

            pDoc->ActSegLog.usNumTyped --;       // correction of eqfbfuncchar.

            wstrInsChar++;                       // status, etc....
            // toggle into insert mode at begin of inline tag
            if ( ! EQFBFuncCharIn(pDoc))
            {
                pDoc->EQFBFlags.inserting = TRUE;
            }
        }
    }
}
// Add end
