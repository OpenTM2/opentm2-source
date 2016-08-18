/*! \file
	Description: This module contains all the code associated with writing to the screen.

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_MORPH
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS
#include <eqf.h>                       // General Translation Manager include file

#include <eqfdoc00.h>
#include "EQFTPI.H"                    // Translation Processor priv. include file
#include <eqfentity.h>


/* Declare the private routines in this module. */

 VOID EQFBStartOfRow
                   (
                      PTBDOCUMENT  pDoc,              // pointer to doc ida
                      PTBROWOFFSET pTBRow,            // value of segment/offset
                      LONG         lStartRow         // starting row
                   );
static COLOUR EQFBSegCol (PTBSEGMENT,USHORT,
                          USHORT, DISPSTYLE);          // get color of segment

static VOID EQFBScreenMarkSeg ( PTBDOCUMENT pDoc );   // find segment to mark

static VOID EQFBSetScrnUpdate ( PTBDOCUMENT pDoc,     // pointer to document ida
                                ULONG       ulRow,    // start row
                                ULONG       ulOffset, // offset within row
                                LONG        lLength);// length

#define MARKCHANGE     4711                                // indicate change of disp
#define CHAR_COMPACT '@'

static
BOOL EQFBCheckForBlockMark
(
   PTBDOCUMENT  pDoc,                     // pointer to document ida
   PEQFBBLOCK   pstBlock,                 // block struct
   ULONG        ulSegNum,                // segment to be tested
   SHORT        sSegOffs                 // offset in seg to be tested
);

void EQFBGetStrikeULine (CHARFORMAT2 *, PSHORT, PSHORT, PSHORT );

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBScreenSegment
//------------------------------------------------------------------------------
// Function call:     EQFBScreenSegment( PTBDOCUMENT,USHORT,PTBROWOFFSET,
//                                        USHORT)
//------------------------------------------------------------------------------
// Description:       write complete line   to the screen and all segments
//                    following in this line
//                    the line is padded with blanks if nec
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc,         pointer to doc ida
//                    USHORT   usRow,           row where to start display
//                    PTBROWOFFSET pRowOffset   pointer to row table
//                    USHORT   usOffset         offset where to start
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - get ptr to segment
//                    - while not end of line
//                        - get state of current char
//                        - if update of line necessary:
//                             - call EQFBSysScrnText if nec.
//                             - reset length
//                        - remember last state
//                        - action according to state of char
//                    - if still s.thing in buffer, call EQFBSysScrnText
//                    - pad rest with zeros
//
//------------------------------------------------------------------------------
static   CHAR_W  szText[ MAX_SEGMENT_SIZE ]; // screen row
 void EQFBScreenSegment
 (
    PTBDOCUMENT pDoc,                        // pointer to doc ida
    LONG   lRow,                          // row where to start display
    PTBROWOFFSET pRowOffset,                 // pointer to row table
    ULONG        ulOffset                        // offset where to start
 )
 {
   ULONG  ulSegNum;                          // segment to be used
   SHORT  sSegOffs;                          // index in segment data
   ULONG  ulColOffset   = 0;                 // column offset
   USHORT usLen         = 0;                 // length of text filled
   PSZ_W  pData;                             // pointer to data
   BOOL   fNewLine = FALSE;                  // new line indicator
   COLOUR colour = COLOUR_TOBE;              // color to be used
   USHORT usState = UNPROTECTED_CHAR;        // status of character
   USHORT usLastState = UNPROTECTED_CHAR;    // status of last character
   USHORT usPrevState = UNPROTECTED_CHAR;    // prev. state of character
   PTBSEGMENT  pSeg;                         // pointer to segment table
   PTBSEGMENT  pTempSeg;                     // temp pointer to segm table
   ULONG  ulMaxDisp;                         // maximal free space in line
   DISPSTYLE   DispStyle;                    // display style
   BYTE        bMark = 0;                    // mark or any other spec. active
   PEQFBBLOCK  pstBlock;                         // block struct
   BOOL        fDBCS1ST = FALSE;                 // true if char is DBCS_1st
   USHORT      usHLState = NO_HIGHLIGHT;
   USHORT      usPrevHLState = NO_HIGHLIGHT;
   USHORT      usLastHLState = NO_HIGHLIGHT;
   ULONG       ulLenDisplayed = 0;
   USEROPT*    pEQFBUserOpt = get_EQFBUserOpt();

   ulSegNum = pRowOffset->ulSegNum;          // get data where to start with
   sSegOffs = pRowOffset->usSegOffset;

   pSeg = EQFBGetVisSeg(pDoc, &ulSegNum );
   pData = ( pSeg ) ? pSeg->pDataW : NULL;    // set data pointer


   fNewLine = (pData == NULL);            // set break criteria for while loop
 // store dispstyle and set it to protected if unprotected to get tags coloured
   DispStyle =  pDoc->DispStyle;
   pDoc->DispStyle = (DispStyle==DISP_UNPROTECTED) ? DISP_PROTECTED : DispStyle;

   /*******************************************************************/
   /* init string to be displayed for bidi                            */
   /*******************************************************************/
   if ( IS_RTL(pDoc) && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay)
   {
     memset( &szText[0], 0, sizeof( szText ) );
   } /* endif */

 // set startoption for mark
   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;                  // get pointer to block mark
   if (  pstBlock )
   {

     bMark |= (EQFBCheckForBlockMark(pDoc, pstBlock,
                                     ulSegNum, sSegOffs)) ? DISP_MARK : 0;

     while ( ! fNewLine )
     {
       /***************************************************************/
       /* if previously character is DBCS_First display state is      */
       /* not changed                                                 */
       /***************************************************************/
       if ( ! fDBCS1ST )
       {
         usState = EQFBCharType( pDoc, pSeg, sSegOffs );

         usHLState = EQFBGetHLType( pSeg, sSegOffs);
         if ((usLastState == usState) && (usHLState != usLastHLState ))
         {
           /***********************************************************/
           /* highlighting is not allowed to change inside of tagging */
           /***********************************************************/
           if ((usLastState == COMPACT_CHAR ) ||
               (usLastState == TRNOTE_CHAR ) ||
               (usLastState == SHORTEN_CHAR) )
           {
              usHLState = usLastHLState;         // keep old style
           } /* endif */
         } /* endif */
       } /* endif */


        if ( (usState != usLastState ) ||
             (usHLState !=usLastHLState) )               // update of line necessary
        {
           if ( usLastState == COMPACT_CHAR )
           {
             ASCII2Unicode( pEQFBUserOpt->szInTagAbbr, szText+usLen, pDoc->ulOemCodePage );
             usLen = (USHORT)(usLen + UTF16strlenCHAR (szText+usLen));

           } /* endif */
           if ( usLastState == TRNOTE_CHAR )
           {
             ASCII2Unicode( pEQFBUserOpt->chTRNoteAbbr, szText+usLen, pDoc->ulOemCodePage );
             usLen = (USHORT)(usLen + UTF16strlenCHAR (szText+usLen));
           } /* endif */

           if ( usLastState == SHORTEN_CHAR )
           {
             UTF16strcpy(szText + usLen, SHORTEN_SEGDATA );
             usLen = (USHORT)(usLen + UTF16strlenCHAR (SHORTEN_SEGDATA));
           } /* endif */

           if ( ulOffset < usLen )
           {
              usLastState = (usLastState==MARKCHANGE) ? usPrevState:usLastState;
              usLastHLState = (usLastState==MARKCHANGE) ? usPrevHLState:usLastHLState;
              bMark |= ( usLastHLState == MISSPELLED_HIGHLIGHT ) ? DISP_MISSPELLED : 0;
              colour = EQFBSegCol(pSeg, usLastState, usLastHLState, DispStyle );
              if (  (usLen  > ulOffset ) && ( pDoc->lScrnCols > (LONG)ulColOffset ))
              {  // use szText and convert to ASCII to get usLen in bytes qqqqq
                 ulMaxDisp = min( usLen-ulOffset, pDoc->lScrnCols-ulColOffset);
              }
              else
              {
                // should not occur -- STOP display!!
                ulMaxDisp = 0;
                fNewLine = TRUE;
              }
             /*****************************************************************/
             /* does line start with 2nd DBCS char? (due to sidescroll?)      */
             /* if so, display a blank as 1st byte of line                    */
             /* if line ends with DBCS 1st char, display blank instead        */
             /*****************************************************************/
              if ( IS_RTL(pDoc) && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay )
              {
                int i;
                for ( i=0; i<(LONG)ulOffset; i++ )
                {
                  if ( szText[i] == EOS )
                  {
                    szText[i] = BLANK;
                  } /* endif */
                } /* endfor */
              } /* endif */

              bMark |= (pSeg->SegFlags.NoReorder) ? DISP_NOREORDER : 0;

              if (ulMaxDisp)
              {
                EQFBSysScrnText( pDoc, lRow, ulColOffset, szText + ulOffset,
                               ulMaxDisp, colour, bMark, szText, &ulLenDisplayed );
		      }

              if ( IS_RTL(pDoc) && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay )
              {
                memset( &szText[0], 0, sizeof( szText ) );
              } /* endif */

              EQFBSetScrnUpdate( pDoc, lRow, ulColOffset, ulMaxDisp );

              ulColOffset = ulColOffset + ulMaxDisp;             // start for next display
              ulOffset = 0;
              bMark = 0;
           }
           else
           {
              ulOffset = ulOffset - usLen;
           } /* endif */
           usLen = 0;                                // reset length
        } /* endif */
        usLastState = usState;                       // remember last state
        usLastHLState = usHLState;

        switch ( usLastState )
        {
          case PROTECTED_CHAR:                 // put character into disp string
          case UNPROTECTED_CHAR:
             if ((pDoc->docType != SERVDICT_DOC) &&
                  pEQFBUserOpt->UserOptFlags.bVisibleSpace &&
                  (*(pData + sSegOffs) == ' ' ))
             {
               szText[usLen ++] = (pDoc->chVisibleBlank) ? pDoc->chVisibleBlank : ' ';
             }
             else
             {
               szText[usLen ++] = *(pData + sSegOffs) ;      // not at end of string
             } /* endif */
             if ( pDoc == pstBlock->pDoc)
             {                                         // segment number the same
                bMark |= (EQFBCheckForBlockMark(pDoc, pstBlock,
                                                ulSegNum, sSegOffs)) ? DISP_MARK : 0;

                if ( (pstBlock->ulSegNum == ulSegNum)
                      && ( sSegOffs == (SHORT) pstBlock->usStart-1 ))
                {
                   usPrevState = usLastState;
                   usLastState = MARKCHANGE;
                   usPrevHLState = usLastHLState;
                }
                else if ( (pstBlock->ulEndSegNum == ulSegNum) &&
                          (sSegOffs == (SHORT) pstBlock->usEnd))
                {
                   usPrevState = usLastState;
                   usLastState = MARKCHANGE;
                   usPrevHLState = usLastHLState;
                }
                else
                {
                  /****************************************************/
                  /* donothing                                        */
                  /****************************************************/
                } /* endif */
             } /* endif */
                                              // set end of line indication
             //fNewLine = ( usLen > usOffset + pDoc->sScrnCols - ulColOffset );
             fNewLine = ( usLen + ulColOffset > ulOffset + pDoc->lScrnCols );
             /*********************************************************/
             /* fill up szText with rest of string for correct        */
             /* handling of BIDI stuff                                */
             /*********************************************************/
             if ( fNewLine && IS_RTL(pDoc) && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay)
             {
               if ( pSeg->pDataW[sSegOffs] )
               {
                 UTF16strcpy( &szText[usLen], pSeg->pDataW + sSegOffs + 1 );
               } /* endif */
             } /* endif */
             break;
           case LINEBREAK_CHAR:                    // write out this line
             if ( (pDoc->docType != SERVDICT_DOC ) &&
                   pEQFBUserOpt->UserOptFlags.bVisibleSpace &&
                   (*(pData + sSegOffs) == '\n' ))
             {
               szText[usLen ++] = pDoc->chVisibleLF;
             } /* endif */
             fNewLine = TRUE;
             break;
           case ENDOFSEG_CHAR:                  // indicate end of segment
             if (pDoc->EQFBFlags.PostEdit && pDoc->pUserSettings->fSegBound )
             {
               szText[usLen++] =  (pDoc->chSegBound) ? pDoc->chSegBound : ' ';
             } /* endif */
             ulSegNum++;                       // point to next one
             pTempSeg = EQFBGetVisSeg( pDoc, &ulSegNum);
             if ( !pTempSeg || ! (pTempSeg->pDataW) )
             {                                 // eof reached ???
                fNewLine = TRUE;
             }
             else
             {
                pSeg = pTempSeg;               // pointer to segment table
                sSegOffs = -1;                 // display start of next segment
                fDBCS1ST = FALSE;              // current char cannot be 2nd
                pData = pSeg->pDataW;
             } /* endif */
             bMark |= DISP_SEGMENT_START;
             break;
          case COMPACT_CHAR:
          case HIDDEN_CHAR:                    // skip this character
             break;
          default:
             break;
        } /* endswitch */
        sSegOffs ++;                           // point to next character

     } /* endwhile */

     if ( usLen > 0 || usLastState == COMPACT_CHAR)                           // still something in buffer
     {
        if ( usLastState == COMPACT_CHAR )
        {
          ASCII2Unicode( pEQFBUserOpt->szInTagAbbr, szText+usLen, pDoc->ulOemCodePage);
          usLen = (USHORT)(usLen + UTF16strlenCHAR (szText+usLen));
        } /* endif */
        if ( usLastState == TRNOTE_CHAR )
        {
          ASCII2Unicode( pEQFBUserOpt->chTRNoteAbbr, szText+usLen, pDoc->ulOemCodePage);
          usLen = (USHORT)(usLen + UTF16strlenCHAR (szText+usLen));
        } /* endif */
        if ( usLastState == SHORTEN_CHAR )
        {
          UTF16strcpy(szText + usLen, SHORTEN_SEGDATA );
          usLen = (USHORT)(usLen + UTF16strlenCHAR (SHORTEN_SEGDATA));
        } /* endif */

        if ( ulOffset < usLen )
        {
           colour = EQFBSegCol(pSeg, usLastState, usLastHLState, DispStyle );
           bMark |= ( usLastHLState == MISSPELLED_HIGHLIGHT ) ? DISP_MISSPELLED : 0;

           if (  (usLen > ulOffset ) && ( pDoc->lScrnCols > (LONG)ulColOffset ))
           {

            ulMaxDisp = min( usLen-ulOffset, pDoc->lScrnCols-ulColOffset);

           }
           else
           {
             // should not occur -- STOP display!!
             ulMaxDisp = 0;
             fNewLine = TRUE;
           }

           bMark |= (EQFBCheckForBlockMark(pDoc, pstBlock,
                                           ulSegNum, sSegOffs)) ? DISP_MARK : 0;
           /*****************************************************************/
           /* does line start with 2nd DBCS char? (due to sidescroll?)      */
           /* if so, display a blank as 1st byte of line                    */
           /* if line ends with DBCS 1st char, display blank instead        */
           /*****************************************************************/

           if ( IS_RTL(pDoc) && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay )
           {
             int i;
             for ( i=0; i<(LONG)ulOffset; i++ )
             {
               if ( szText[i] == EOS )
               {
                 szText[i] = BLANK;
               } /* endif */
             } /* endfor */
           } /* endif */
           bMark |= (pSeg->SegFlags.NoReorder) ? DISP_NOREORDER : 0;
           if (ulMaxDisp)
		   {
             EQFBSysScrnText( pDoc, lRow, ulColOffset, szText + ulOffset,
                            ulMaxDisp, colour, bMark, szText, &ulLenDisplayed );
		   }

           if ( IS_RTL(pDoc) && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay )
           {
             memset( &szText[0], 0, sizeof( szText ) );
           } /* endif */
                                                 // set update region
           EQFBSetScrnUpdate( pDoc, lRow, ulColOffset, ulMaxDisp );

           ulColOffset = ulColOffset + ulMaxDisp;             // start for next display
           ulOffset = 0;
           bMark = 0;
        }
        else
        {
           ulOffset = ulOffset - usLen;
        } /* endif */
        usLen = 0;                                // reset length
     } /* endif */

     if (fNewLine && pDoc->EQFBFlags.PostEdit
              && pDoc->pUserSettings->fSegBound
              && pSeg)
     {
       usState = EQFBCharType(pDoc, pSeg, sSegOffs);
       if ((usState == ENDOFSEG_CHAR ) )
       {
         szText[usLen++] =  (pDoc->chSegBound) ? pDoc->chSegBound : ' ';
         if (ulOffset < usLen )
         {
           colour = EQFBSegCol(pSeg, usState, NO_HIGHLIGHT, DispStyle );
           bMark |= ( usLastHLState == MISSPELLED_HIGHLIGHT ) ? DISP_MISSPELLED : 0;
           if (  (usLen > ulOffset ) && ( pDoc->lScrnCols > (LONG)ulColOffset ))
           {
             ulMaxDisp = min( usLen-ulOffset, pDoc->lScrnCols-ulColOffset);
           }
           else
           {
             // should not occur -- STOP display!!
             ulMaxDisp = 0;
             fNewLine = TRUE;
           }

          // usMaxDisp = min( usLen-usOffset, pDoc->sScrnCols-ulColOffset);

           if ( IS_RTL( pDoc ) && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay)
           {
             int i;
             for ( i=0; i<(LONG)ulOffset; i++ )
             {
               if ( szText[i] == EOS )
               {
                 szText[i] = BLANK;
               } /* endif */
             } /* endfor */
           } /* endif */
           bMark |= (pSeg->SegFlags.NoReorder) ? DISP_NOREORDER : 0;

           if (ulMaxDisp)
           {
             EQFBSysScrnText( pDoc, lRow, ulColOffset, szText + ulOffset,
                            ulMaxDisp, colour, bMark, szText, &ulLenDisplayed );
	       }

           if ( IS_RTL(pDoc) && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay )
           {
             memset( &szText[0], 0, sizeof( szText ) );
           } /* endif */
                                                 // set update region
           EQFBSetScrnUpdate( pDoc, lRow, ulColOffset, ulMaxDisp );

           ulColOffset = ulColOffset + ulMaxDisp;             // start for next display
           bMark = 0;
         } /* endif */

       } /* endif */
     } /* endif */

     if ( (LONG)ulColOffset < pDoc->lScrnCols )     // pad rest with zeros
     {
       colour = EQFBSegCol(pSeg, usLastState, usLastHLState, DispStyle );
       if (pSeg)
       {
                   bMark |= (pSeg->SegFlags.NoReorder) ? DISP_NOREORDER : 0;
       }

       EQFBSysScrnChar( pDoc, lRow, ulColOffset,
                        (pDoc->lScrnCols - ulColOffset),
                        colour, bMark );
       EQFBSetScrnUpdate( pDoc, lRow, ulColOffset,
                          (pDoc->lScrnCols-ulColOffset));

     } /* endif */
   } /* endif */

   pDoc->DispStyle = DispStyle;           // reset to original display style
   return;
 }


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBScreenData
//------------------------------------------------------------------------------
// Function call:     EQFBScreenData(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       write the requested area to the screen
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDOc   ptr to documnet instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - get the row to draw
//                    - write workline to the screen
//                    - Next redraw the lines below the current line,
//                       if they have been changed.
//                    - Finally redraw the area above the current line.
//                    - rest redraw indicator
//
//------------------------------------------------------------------------------
 VOID EQFBScreenData
 (
    PTBDOCUMENT pDoc                      // pointer to document instance
 )
 {
   LONG    lI;                              // index
   LONG    lRow;                            // screen row
   USHORT  usLastrow;                     // last row on the screen
   LONG    lOffset;                      // offset on screen
   COLOUR  color;                         // color to be chosen

   if ( pDoc->hwndRichEdit )
   {
     /*****************************************************************/
     /* Screen display handled differently in new editor              */
     /*****************************************************************/
     return;
   } /* endif */


   /*******************************************************************/
   /* force a refill for Bidi screen shadow                           */
   /*******************************************************************/
   if ( pDoc->Redraw >= REDRAW_ALL && pDoc->pBidiStruct  )
   {
     pDoc->pBidiStruct->fRedrawScreen = TRUE;
   } /* endif */
   lRow     = pDoc->lCursorRow;
   usLastrow = (USHORT)(pDoc->lScrnRows-1);
   lOffset  = pDoc->lSideScroll;

   if ( pDoc->Redraw <= REDRAW_LINE )
   {
      EQFBSysScrnUpdate ( pDoc, FALSE );     // disable screen update
   } /* endif */

   // if workline changed  write it out
     EQFBScreenSegment( pDoc,
                        lRow,
                        &(pDoc->TBRowOffset[lRow+1]),       // cursor segment
                        lOffset);



   /* Next redraw the lines below the current line, if they have */
   /* been changed.                                              */
   if (pDoc->Redraw >= REDRAW_BELOW)
   {
      lI = (lRow+1);
      while (lI <=  usLastrow && (pDoc->TBRowOffset[lI+1]).ulSegNum > 0)
      {
         EQFBScreenSegment( pDoc,
                            lI,
                            &(pDoc->TBRowOffset[lI+1]),  // cursor segment
                            lOffset);
         lI++ ;                                          // point to next row
      } /* endwhile */

      /* Pad to the bottom with blank lines. */
      usLastrow++;                                    // fill partial row too
      color = COLOUR_NOP;                             // default background

      for (; lI < (SHORT) usLastrow; lI++)
         EQFBSysScrnChar(pDoc, lI, 0, pDoc->lScrnCols, color, 0 );
   }

   /* Finally redraw the area above the current line. */

   if (pDoc->Redraw >= REDRAW_ALL)
   {
      lI = (lRow-1);
      while (lI >= 0 && (pDoc->TBRowOffset[lI+1]).ulSegNum > 0 )
      {
         EQFBScreenSegment( pDoc,
                            lI,
                            &(pDoc->TBRowOffset[lI+1]),  // cursor segment
                            lOffset);
         lI-- ;                                          // point to next row
      } /* endwhile */

      /* Pad to the top of the screen with blank lines. */
      color = COLOUR_NOP;                             // default background
      for (; lI >= 0; lI--)
         EQFBSysScrnChar(pDoc, lI, 0, pDoc->lScrnCols, color, 0 );
   }



   if ( pDoc->Redraw <= REDRAW_LINE )
   {
      EQFBSysScrnUpdate ( pDoc, TRUE  );     // enable screen update
   } /* endif */
   pDoc->Redraw = REDRAW_NONE;            // The data area is now up to date.
                                          // show the buffer
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBScreenCursorType
//------------------------------------------------------------------------------
// Function call:     EQFBScreenCursorType(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       set an insert or replace cursor
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc    pointer to document instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     issue a call to the low level system routine
//                      (EQGBSysScrnCurShape)
//------------------------------------------------------------------------------

 void EQFBScreenCursorType
 (
   PTBDOCUMENT  pDoc
 )

 {
   USHORT usCursor;

   usCursor = (USHORT)((pDoc->EQFBFlags.inserting) ? CURSOR_INSERT : CURSOR_REPLACE);

   usCursor = (USHORT)((pDoc->EQFBFlags.EndOfSeg) ? CURSOR_SEGMENT : usCursor);


   if ( pDoc->usCursorType != usCursor )
   {
      pDoc->usCursorType = usCursor;                  // save cursor type

      /* Size the cursor */
      EQFBSysScrnCurShape( pDoc, (CURSOR)usCursor );
   } /* endif */
   return;
 }



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBScrnLinesFromSeg
//------------------------------------------------------------------------------
// Function call:     EQFBScrnLinesFromSeg(PTBDOCUMENT,USHORT,
//                        USHORT,PTBROWOFFSET)
//------------------------------------------------------------------------------
// Description:       fill segment table TBRow from current position
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc,           pointer to doc ida
//                    USHORT       usStartRow,     starting row
//                    USHORT       usNumRows,      number of rows
//                    PTBROWOFFSET pTBStartSeg     starting segment
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     fill starting with the starting row
//                      the segment numbers and offsets for
//                      the requested number of rows
//                    - get next segment
//                    - fill first row
//                    - while not at end of table and
//                        still not number of rows
//                            - scan for a line break in cur segment
//                    - if not found increase segment and try again
//                    - call EQFBFillPrevTB if nec
//
//------------------------------------------------------------------------------
 VOID EQFBScrnLinesFromSeg
 (
    PTBDOCUMENT  pDoc,                       // pointer to doc ida
    LONG         lStartRow,                 // starting row
    LONG         lNumRows,                  // number of rows
    PTBROWOFFSET pTBStartSeg                 // starting segment
 )
 {
   if ( !pDoc->hwndRichEdit )
   {
     SHORT  sSegOffs;                          // index in segment data
     PSZ_W  pData;                             // pointer to data

     PTBSEGMENT pSeg;                          // pointer to segment
     PTBROWOFFSET  pTBRow;                     // pointer to row structure
     BOOL          fLineBreak = FALSE;         // is a linebreak pending
     BOOL          fPrevRow = FALSE;           // fill line info for prev row
     ULONG         ulStartSegNum;              // segment number


     ulStartSegNum = pTBStartSeg->ulSegNum;    // start segment
     lStartRow++;                             // 1st table row= sCursorRow 0
     pSeg = EQFBGetVisSeg(pDoc, &ulStartSegNum); // pointer to segment
     sSegOffs = (SHORT) pTBStartSeg->usSegOffset;   // start offset

     if ( pSeg )
     {
        if ( sSegOffs > (SHORT) pSeg->usLength )         // consistency check
        {
           ulStartSegNum++;                       // next segment
           pSeg = EQFBGetVisSeg(pDoc, &ulStartSegNum); // pointer to segment
           sSegOffs = 0;                          // start at begin
        } /* endif */
        pData = ( pSeg ) ? pSeg->pDataW : NULL; // set data pointer

        if ( lStartRow == 1)                     // skip first row for the moment
        {
           fPrevRow = TRUE;
        } /* endif */

        lStartRow = min( lStartRow, MAX_SCREENLINES );
        lNumRows = min( lNumRows, MAX_SCREENLINES - lStartRow );
        pTBRow = pDoc->TBRowOffset + lStartRow + 1;   // first element to fill
        lNumRows++;                              // table contains seg/off for
                                                  // the line scrnrow+1

        memset ( pTBRow, 0, lNumRows * sizeof(TBROWOFFSET)) ;


        EQFBStartOfRow (pDoc, pTBStartSeg, lStartRow-1 ); // fill first row


        // while not at end of table and still not number of rows
        //    scan for a line break in current segment
        //    if not found increase segment and try again
        //

        while ( lNumRows > 0 && pData )
        {
           // check if line break is pending and we are not pointing to end of string
           if ( fLineBreak  &&  *(pData + sSegOffs))
           {
             pTBRow->usSegOffset = sSegOffs;   // offset
             pTBRow->ulSegNum = ulStartSegNum; // segment
             pTBRow++;                         // point to next row entry
             lNumRows --;
             fLineBreak = FALSE;               // init linebreak
           } /* endif */

           switch ( *(pData + sSegOffs) )
           {
              case LF:                             // line break found
              case SOFTLF_CHAR:
                /********************************************************/
                /* linefeed can be hidden if compact style              */
                /* if it is not hidden, then set linebreak indicator    */
                /********************************************************/
                 if ( EQFBCharType( pDoc, pSeg, sSegOffs )
                                    == LINEBREAK_CHAR )
                 {
                   fLineBreak = TRUE;
                 } /* endif */
                 break;
              case EOS:                             // end of segment
                 ulStartSegNum ++;                   // point to next segment
                 sSegOffs = -1;                      // and init data
                                                     // will be incremented at end
                                                     // of switch to start at 0
                 pSeg = EQFBGetVisSeg(pDoc, &ulStartSegNum); // pointer to segment
                 pData = ( pSeg ) ? pSeg->pDataW : NULL; // set data pointer
                 break;
              default :
                 break;
           } /* endswitch */

           sSegOffs ++;                             // point to next character
        } /* endwhile */

        if ( fPrevRow )
        {
           EQFBFillPrevTBRow( pDoc, 1 ); // pointer to row offset table
        } /* endif */
     }
     else
     {                                   // init rowoffset area
         memset( pDoc->TBRowOffset, 0 , sizeof(TBROWOFFSET) * MAX_SCREENLINES );
     } /* endif */
   } /* endif */
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBStartOfRow
//------------------------------------------------------------------------------
// Function call:     EQFBStartOfRow(PTBDOCUMENT,PTBROWOFFSET,USHORT)
//------------------------------------------------------------------------------
// Description:       fill current TBRow with Segment number/offset
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc,        pointer to doc ida
//                    PTBROWOFFSET pTBRow,      value of segment/offset
//                    SHORT        sStartRow    starting row
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - get the data for the starting segment and offset
//                         from TBROW element of the start row
//                     - loop until start of row found, i.e. until \n or
//                           first segment reached
//------------------------------------------------------------------------------

 VOID EQFBStartOfRow
 (
    PTBDOCUMENT  pDoc,                       // pointer to doc ida
    PTBROWOFFSET pTBRow,                     // value of segment/offset
    LONG         lStartRow                   // starting row
 )
 {
   SHORT  sSegOffset;                        // index in segment data
   PSZ_W  pData;                             // pointer to data

   PTBSEGMENT pSeg;                          // pointer to segment
   PTBROWOFFSET  pTBRowTbl;                  // pointer to row structure
   ULONG         ulSegNum;                   // current segment
   BOOL          fFound = FALSE;             // not found yet

   ulSegNum = pTBRow->ulSegNum;              // segment number
   sSegOffset = (SHORT) pTBRow->usSegOffset; // start offset

   lStartRow++;                             // table versus screen offset
   pTBRowTbl = pDoc->TBRowOffset + lStartRow;  // starting row


   pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);    // pointer to segment
   pData = ( pSeg ) ? pSeg->pDataW : NULL; // set data pointer

   // loop until start of row found, i.e. until \n or first segment reached
 if ( pData )
 {
   do
   {
      pTBRowTbl->ulSegNum = ulSegNum;        // store current value
      pTBRowTbl->usSegOffset = sSegOffset;

      sSegOffset --;                         // point to prev. character
      if ( sSegOffset < 0)
      {
         if ( ulSegNum > 1)
         {
            ulSegNum--;
            pSeg = EQFBGetPrevVisSeg(pDoc, &ulSegNum);    // pointer to segment
            if ( pSeg && pSeg->pDataW )
            {
               pData = pSeg->pDataW;                   // get pointer to data
               sSegOffset = (SHORT) UTF16strlenCHAR( pData );
               if ( sSegOffset > 0 )
               {
                  sSegOffset--;
               } /* endif */
            }
            else
            {
               fFound = TRUE;
            } /* endif */
         }
         else
         {
            pTBRowTbl->ulSegNum = 1;         // start of table reached
            pTBRowTbl->usSegOffset = 0;
            fFound = TRUE;
         } /* endif */
      } /* endif */

      if ( (sSegOffset>=0) &&
          ((*(pData+sSegOffset) == '\n')||(*(pData+sSegOffset) == SOFTLF_CHAR) ) )
      {
        /**************************************************************/
        /* if display style compact and linebreak is hidden,          */
        /* do not set indicator                                       */
        /**************************************************************/
        if ( EQFBCharType( pDoc, pSeg, sSegOffset )
                           == LINEBREAK_CHAR )
        {
          fFound = TRUE;
        } /* endif */
      } /* endif */
   } while ( !fFound  ); /* enddo */
 } /* endif */

}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFillPrevTBRow
//------------------------------------------------------------------------------
// Function call:     EQFBFillPrevTBRow(PTBDOCUMENT,USHORT)
//------------------------------------------------------------------------------
// Description:       fill TBRow backwards
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc,
//                    SHORT        sStartRow
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Side effects:      Note that the one of the current row we already have
//                    detected but nevertheless we fill it again
//------------------------------------------------------------------------------
// Function flow:     while previous segments available and
//                     rows to do
//                     scan for a linebreak in current segment
//                     and fill TBRow accordingly
//------------------------------------------------------------------------------

 VOID EQFBFillPrevTBRow
 (
    PTBDOCUMENT  pDoc,                       // pointer to doc ida
    LONG         lStartRow                   // starting row
 )
 {
   SHORT         sSegOffs = 0;               // index in segment data
   PSZ_W         pData;                      // pointer to data

   PTBSEGMENT    pSeg = NULL;                // pointer to segment
   PTBROWOFFSET  pTBRow = NULL;              // pointer to row structure
   TBROWOFFSET   TBCurRow;                   // current row
   ULONG         ulSegNum = 0L;              // current segment
   BOOL          fFirst = TRUE;              // first call

   lStartRow++;                              // table versus screen offset

   pData = NULL;                             // init data pointer

   while ( !pData && lStartRow > 0 )         // check if at end
   {
      pTBRow = pDoc->TBRowOffset + lStartRow;  // starting row
      ulSegNum = pTBRow->ulSegNum;              // segment number
      if ( ulSegNum )
      {
         sSegOffs = (SHORT) pTBRow->usSegOffset;// start offset
         pSeg = EQFBGetVisSeg(pDoc, &ulSegNum); // pointer to segment
         pData = ( pSeg ) ? pSeg->pDataW : NULL; // set data pointer
      } /* endif */
      if ( !pData )                          // start one earlier - end reached
      {
         lStartRow --;
      } /* endif */
   } /* endwhile */

   if ( lStartRow > 0 )
   {
     memset ( (pDoc->TBRowOffset), 0, lStartRow * sizeof(TBROWOFFSET)) ;
   } /* endif */

   // while previous segments available and rows to do
   //    scan for a line break in current segment
   //
   // Note that the one of the current row we already have detected but
   // nevertheless we fill it again

   lStartRow++;                                // since we go backward and
                                               // have to do one more
   while ( lStartRow > 0 && pData )           // change this to Do ..While
   {
      if (!fFirst &&
         (( *(pData+sSegOffs) == '\n') ||( *(pData+sSegOffs) == SOFTLF_CHAR) ) )
      {
        /**************************************************************/
        /* if display style compact, linebreak can be hidden          */
        /**************************************************************/
        if ( EQFBCharType( pDoc, pSeg, sSegOffs )
                           == LINEBREAK_CHAR )
        {
          memcpy(pTBRow,&TBCurRow,sizeof(TBROWOFFSET));   // fill in entry data
          lStartRow--;
		  // P018287: lStartRow was incremented at the begin of the loop, hence we cannot decrease
		  // pTBRow if lStartRow == 0, otherwise we point outside of pDoc->TBRowOffset
		  // and we would blow up if someone uses pTBRow afterwards
		  if (lStartRow > 0)
		  {
			  pTBRow --;                             // point to previous entry
		  }
        } /* endif */
      } /* endif */
      fFirst = FALSE ;                          // not the first time
      TBCurRow.usSegOffset = (USHORT) sSegOffs;
      TBCurRow.ulSegNum    = ulSegNum;

      sSegOffs --;                             // point to prev character
      if ( sSegOffs <  0)                      // get data for previous segment
      {
         if (ulSegNum > 1 )                     // still not at the beginning
         {
           ulSegNum--;
           pSeg = EQFBGetPrevVisSeg( pDoc, &ulSegNum);  // pointer to segment
           pData = ( pSeg ) ? pSeg->pDataW : NULL;          // set data pointer
           if ( pData )
           {
             sSegOffs = (SHORT) UTF16strlenCHAR(pData);            // length of segment
             sSegOffs = (sSegOffs > 0) ? (sSegOffs-1) : 0;
           }
           else
           {
             pTBRow->ulSegNum = 1;                // fill in initial data for
             pTBRow->usSegOffset = 0;             // first segment
           } /* endif */
         }
         else
         {
           pTBRow->ulSegNum = 1;                // fill in initial data for
           pTBRow->usSegOffset = 0;             // first segment
           pData = NULL;                        // reset data
         } /* endif */
      } /* endif */

   } /* endwhile */

}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBSegCol
//------------------------------------------------------------------------------
// Function call:     EQFBSegCol(PTBSEGMENT,USHORT,USHORT, DISPSTYLE)
//------------------------------------------------------------------------------
// Description:       get the active color of act. segment depending on status
//------------------------------------------------------------------------------
// Parameters:        PTBSEGMENT  pSeg,       active segment
//                    USHORT      usState,    currently active state
//                    USHORT      usHLState,  highlighting state
//                    DISPSTYLE   DispStyle   currently active disp Style
//------------------------------------------------------------------------------
// Returncode type:   COLOUR
//------------------------------------------------------------------------------
// Returncodes:       the active color
//------------------------------------------------------------------------------
// Function flow:     if ptr to segment is not 0
//                      switch (status of segment)
//                        set color according to this state
//------------------------------------------------------------------------------

static
COLOUR EQFBSegCol
(
   PTBSEGMENT  pSeg,                         // active segment
   USHORT      usState,                      // currently active state
   USHORT      usHLState,                    // highlighting state
   DISPSTYLE   DispStyle                     // currently active disp Style
)
{
   COLOUR   color = COLOUR_TOBE;                           // color to be used
   USHORT   usOriginalState = 0;

   usOriginalState = usState;
   if ( (DispStyle == DISP_SHRINK) || (DispStyle == DISP_COMPACT)
        || (DispStyle == DISP_SHORTEN)   )
   {
     DispStyle = DISP_PROTECTED;                 // to get it correctly displayed
     usState = (usState == COMPACT_CHAR) ? PROTECTED_CHAR : usState;
   } /* endif */

   if (usState == ENDOFSEG_CHAR )
   {
     usState = PROTECTED_CHAR;
   } /* endif */

   if (usState == TRNOTE_CHAR )
   {
     usState = PROTECTED_CHAR;
   } /* endif */

   if (usState == SHORTEN_CHAR )
   {
     usState = PROTECTED_CHAR;
   } /* endif */

   if ( pSeg )
   {
     switch (pSeg->qStatus & 0x0FF)
     {
       case QF_TOBE:
       case QF_ATTR:
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_TOBE;
          break;
       case QF_XLATED:                         // already translated
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_XLATED : COLOUR_XLATED;

          if (usState != PROTECTED_CHAR &&
              ( pSeg->SegFlags.Typed || pSeg->SegFlags.Copied ) )
          {
            if ( pSeg->SegFlags.Typed )
            {
              if ( pSeg->SegFlags.Copied )
              {
                color = COLOUR_TMODPROPOSAL;
              }
              else
              {
                color = COLOUR_TFROMSCRATCH;
              } /* endif */
            }
            else
            {
              color = COLOUR_TCOPYPROPOSAL;
            } /* endif */
          } /* endif */
          break;
       case QF_NOP:                            // nop, tag
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_NOP : COLOUR_NOP;
          break;
       case QF_CURRENT:                         // current segment
          if ( DispStyle == DISP_PROTECTED )
          {
             color = (usState==PROTECTED_CHAR) ? COLOUR_P_ACTIVE : COLOUR_ACTIVE;
          }
          else
          {
             color = (usState==PROTECTED_CHAR)
                          ? COLOUR_ACTIVE_TAG : COLOUR_ACTIVE;
          } /* endif */
          break;
       case QF_MACHPROPOSAL:                       // machine proposal
         color = (usState == PROTECTED_CHAR) ? COLOUR_MACHINE_MATCH_PROT : COLOUR_MACHINE_MATCH_NORMAL;
          break;
       case QF_FUZZYPROPOSAL:                       // fuzzy proposal
         color = (usState == PROTECTED_CHAR) ? COLOUR_FUZZY_MATCH_PROT : COLOUR_FUZZY_MATCH_NORMAL ;
          break;
       case QF_PROP0TEXT:                       // proposal 0
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_TOBE;
          break;
       case QF_PROP0PREFIX:                     // proposal 0 prefix
          color = COLOUR_SRV_PROP0PREFIX;
          break;
       case QF_PROPNTEXT:                       // proposal n
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_XLATED : COLOUR_XLATED;
          break;
       case QF_PROPNPREFIX:                     // proposal n prefix
          color = COLOUR_SRV_PROPNPREFIX;
          break;
       case QF_DICTHEAD:                        // colour of dictionary header
          color = COLOUR_SRV_DICTHEAD;
          break;
       case QF_DICTTRANS:                       // colour of dict. translation
          color = COLOUR_SRV_DICTTRANS;
          break;
       case QF_DICTPREFIX:                      // colour of dict prefix
          color = COLOUR_SRV_DICTPREFIX;
          break;
       case QF_DICTADDINFO:                     // colour of additional dict info
          color = COLOUR_SRV_DICTADDINFO;
          break;
       case QF_PROPSRCEQU:                      // proposal source equal text
          color = COLOUR_SRV_PROPSRCEQU;
          break;
       case QF_PROPSRCUNEQU:                    // proposal source unequal text
          color = COLOUR_SRV_PROPSRCUNEQU;
          break;
       case QF_PROPSRCDEL:                      // proposal source deleted text
          color = COLOUR_SRV_PROPSRCDEL;
          break;
       case QF_PROPSRCINS:                      // proposal source inserted text
          color = COLOUR_SRV_PROPSRCINS;
          break;

       case QF_DICTINDIC:                       // colour of dict prefix
          color = COLOUR_DICTINDIC;
          break;

       case QF_DICTSTYLEPREF:                   // colour of dictionary style indicator "preferred term"
          color = COLOUR_DICTSTYLEPREF;
          break;
       case QF_DICTSTYLENOT:                    // colour of dictionary style indicator "not allowedterm"
          color = COLOUR_DICTSTYLENOT;
          break;

        case QF_ANCHOR_1 :
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_ANCHOR_1;
          break;
        case QF_ANCHOR_2 :
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_ANCHOR_2;
          break;
        case QF_ANCHOR_3 :
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_ANCHOR_3;
          break;
        case QF_VALID_01 :
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_VALID_01;
          break;
        case QF_VALID_10 :
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_VALID_10;
          break;
        case QF_VALID_11_1 :
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_VALID_11_1;
          break;
        case QF_VALID_11_2 :
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_VALID_11_2;
          break;
        case QF_VALID_11_3 :
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_VALID_11_3;
          break;
        case QF_CROSSED_OUT :
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_CROSSED_OUT;
          break;
        case  QF_NOP_ANCHOR_1:
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_NOP : COLOUR_NOP_ANCHOR_1;
          break;
        case  QF_NOP_ANCHOR_2:
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_NOP : COLOUR_NOP_ANCHOR_2;
          break;
        case  QF_NOP_ANCHOR_3:
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_NOP : COLOUR_NOP_ANCHOR_3;
          break;
        case  QF_VISACT:
//        color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_VISACT;
          if ( DispStyle == DISP_PROTECTED )
          {
             color = (usState==PROTECTED_CHAR) ? COLOUR_P_ACTIVE : COLOUR_VISACT;
          }
          else
          {
             color = (usState==PROTECTED_CHAR)
                          ? COLOUR_ACTIVE_TAG : COLOUR_VISACT;
          } /* endif */
          break;
        case  QF_OVERCROSS:
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_TOBE : COLOUR_OVERCROSS;       // VisITM: overcross user anchor
          break;
        case QF_CROSSED_OUT_NOP :
          color = (usState == PROTECTED_CHAR) ? COLOUR_P_NOP : COLOUR_CROSSED_OUT;
          break;

        case QF_TRNOTE_L1_1:
          color = COLOUR_TRNOTE_11;
          break;
        case QF_TRNOTE_L1_2:
          color = COLOUR_TRNOTE_12;
          break;
        case QF_TRNOTE_L2:
          color = COLOUR_TRNOTE_2;
          break;

        case QF_OWN_COLOR:
          /************************************************************/
          /* high order byte contains color                           */
          /************************************************************/
          {
            PTEXTTYPETABLE pTextType;
            pTextType = get_TextTypeTable() + COLOUR_OWN_SETTING;
            color = COLOUR_OWN_SETTING;
            pTextType->sFGColor = (pSeg->qStatus & 0x0F00 ) >> 8;
            pTextType->sBGColor = (pSeg->qStatus & 0xF000 ) >> 12;
          }
          break;
     } /* endswitch */
   } /* endif */

   switch ( usHLState )
   {
     case TAG_HIGHLIGHT:
       color = COLOUR_UNMATCHTAG;
       break;
     default:
        break;
   } /* endswitch */

   if (usOriginalState == TRNOTE_CHAR)
   {
      color = COLOUR_STANDARD_TRNOTE;
   }

   return (color);
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBPhysCursorFromSeg
//------------------------------------------------------------------------------
// Function call:     EQFBPhysCursorFromSeg(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       prepare physical cursor position (sCursorRow,sCursorCol)
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     use the TBRowOffset table and the passed segment
//                    number/offset to find  CursorCol/Row
//                    - check if given SegNum starts 'in' TBRowOffset
//                                 and 'in' window
//                    - loop til line of TBRowOffset starts
//                           with segnum < TBCursor.Segnum
//                    - check if we've gone too far, since requested
//                           segment might start within a row.
//                    - get segment and offset
//                    - while not at starting segment
//                        - update usColumn/lRow according to cur char
//                    - while not at correct offset
//                        - update Col/row according to current char
//                    - set cursor row ( starts with 0)
//                    - set return column
//                    - scroll screen if necessary
//
//------------------------------------------------------------------------------
 VOID EQFBPhysCursorFromSeg
 (
    PTBDOCUMENT  pDoc                        // pointer to doc ida
 )
 {
    PTBROWOFFSET pTBTemp;                    // pointer to screen row offset
    PSZ_W        pData;                      // pointer to data
    USHORT       usRow = 1;                  // cursor row
    ULONG        ulColumn = 0;               // cursor column
    ULONG        ulSegNum;                   // segment number
    SHORT        sSegOffs;                   // segment offset
    PTBSEGMENT   pSeg;                       // pointer to segment
    LONG         lHelpCursor;                //help var for cursor value
    PTBROWOFFSET pTBCursor;                  // cursor in segnum/offset
    ULONG        ulActLine;

    pTBTemp = pDoc->TBRowOffset;             // init index through table
    pTBCursor = &(pDoc->TBCursor);           // use cursor settings in doc

    //check if given SegNum starts 'in' TBRowOffset and 'in' window
    //loop til line of TBRowOffset starts with segnum < TBCursor.Segnum
    if (!pDoc->hwndRichEdit  )
    {
      while ((pTBCursor->ulSegNum) <= ((pDoc->TBRowOffset+1)->ulSegNum)
                 && (pDoc->TBRowOffset)->ulSegNum != 0)
      {
         EQFBFuncScrollDown(pDoc,FALSE);
      } /* endwhile */

      while ( ((pDoc->TBRowOffset+usRow)->ulSegNum > 0 )
              && pTBCursor->ulSegNum > (pDoc->TBRowOffset + usRow)->ulSegNum )
      {
         usRow++;                              // point to next one
      } /* endwhile */

      // if (pDoc->TBRowOffset+usRow) ->ulSegNum = 0,pTBCursor->ulSegNum
      // is not on Screen, and not in TBRowOffset!!
      // program is causing a prompt in this case

      // check if we've gone too far, since requested segment might start
      // within a row.
      if ( (pDoc->TBRowOffset+usRow)->ulSegNum == 0 ||
           (pDoc->TBRowOffset+usRow)->ulSegNum >= pTBCursor->ulSegNum)
      {
         usRow --;
      } /* endif */
      usRow = ( usRow == 0 ) ? 1 : usRow ;

      ulSegNum = (pDoc->TBRowOffset+usRow)->ulSegNum;   // get segment and offset
      sSegOffs = (pDoc->TBRowOffset+usRow)->usSegOffset;


      pSeg = EQFBGetVisSeg( pDoc, &ulSegNum );
      pData = ( pSeg ) ? pSeg->pDataW : NULL; // set data pointer

      // find starting segment
      while ( ((ulSegNum <  pTBCursor->ulSegNum)
              || (ulSegNum == pTBCursor->ulSegNum
                  && (sSegOffs <  (SHORT) pTBCursor->usSegOffset))) && pData)
      {
         switch ( EQFBCharType( pDoc, pSeg, sSegOffs ))
         {
           case LINEBREAK_CHAR:
              ulColumn = 0;                  // reset column
              usRow ++;                      // set row change
              break;
            case COMPACT_CHAR:
            case TRNOTE_CHAR:
           case PROTECTED_CHAR:
           case UNPROTECTED_CHAR:
           case SHORTEN_CHAR:
              ulColumn++;

              break;
            case ENDOFSEG_CHAR:                  // indicate end of segment
              if (pDoc->EQFBFlags.PostEdit && pDoc->pUserSettings->fSegBound )
              {
                if (sSegOffs &&
                     (EQFBCharType(pDoc, pSeg, (SHORT)(sSegOffs-1))!= LINEBREAK_CHAR))
                {
                  ulColumn++;

                } /* endif */
              } /* endif */
              ulSegNum++;                    // point to new segment
              sSegOffs = -1;                 // start of next segment
              pSeg = EQFBGetVisSeg( pDoc, &ulSegNum );
              pData = ( pSeg ) ? pSeg->pDataW : NULL; // set data pointer
              break;
           case HIDDEN_CHAR:                 // skip this character
              break;
           default:
              break;
         } /* endswitch */
         sSegOffs ++;                        // point to next character

      } /* endwhile */


      pDoc->lCursorRow = usRow -1;           // set cursor row ( starts with 0)
      pDoc->lCursorCol = ulColumn - pDoc->lSideScroll; // set return column

      //check if sCursorRow is on screen and in TBRowOffset
      if (pDoc->lCursorRow > (SHORT) pDoc->lScrnRows-1)
      {
         lHelpCursor = pDoc->lCursorRow;              //EQFBFuncSCrollUp needs sCursor to
                                                //be within screen(i.e. EQFBLineDown)
         pDoc->lCursorRow = pDoc->lScrnRows-1;       //simulate cursor in last screenline

         while (lHelpCursor >  (pDoc->lScrnRows-1))
         {
            EQFBFuncScrollUp(pDoc,FALSE);       //no update of TBCursor
            lHelpCursor --;
         } /* endwhile */
      } /* endif */


      // check if cursor is in visible area
      if (  EQFBRelCurPos(  pDoc, pDoc->lCursorCol,  pDoc->lScrnCols  ) )
      {
          LONG  lPos = EQFBGetAbsPos( pDoc, pDoc->lCursorCol  );
         ulColumn = ((lPos -  pDoc->lScrnCols*pDoc->cx) / pDoc->cx) + 3;
         pDoc->lSideScroll = pDoc->lSideScroll + ulColumn;      // shift screen a little bit
         pDoc->lCursorCol  = pDoc->lCursorCol  - ulColumn;
         pDoc->Redraw |= REDRAW_ALL;         // force a screen update
      }
      else
      if ( ( pDoc->lCursorCol >=  pDoc->lScrnCols))
      {
         ulColumn = pDoc->lCursorCol - pDoc->lScrnCols + 3;
         pDoc->lSideScroll = pDoc->lSideScroll + ulColumn;      // shift screen a little bit
         pDoc->lCursorCol  = pDoc->lCursorCol  - ulColumn;
         pDoc->Redraw |= REDRAW_ALL;         // force a screen update
      }
      else if (pDoc->lCursorCol < 0 )
      {
         pDoc->lSideScroll = pDoc->lSideScroll + pDoc->lCursorCol;// shift screen a little bit
         pDoc->lCursorCol  = 0;
         pDoc->Redraw |= REDRAW_ALL;         // force a screen update
      } /* endif */


      ulActLine = EQFBQueryActLine(pDoc,
                                   (pDoc->TBCursor).ulSegNum,
                                   (pDoc->TBCursor).usSegOffset );
      pDoc->ulVScroll = ulActLine - pDoc->lCursorRow;


    } /* endif */

   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBCurSegFromCursor
//------------------------------------------------------------------------------
// Function call:     EQFBCurSegFromCursor(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       get segment number/offset from cursorrow/column
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc  ptr to document instance
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       negative if cursor right of text
//------------------------------------------------------------------------------
// Function flow:     get cursor position
//                    get segment number and offset from TBRowOffset
//                      ( of the CursorRow) ( we know that the physical
//                      cursor row is the index into the TBRowOffset table)
//                    while offset in line < given cursor column
//                         according to char type: update offset in line,
//                                                  SegNum and SegOffset
//                    set TBCursor with current segment number /offset
//------------------------------------------------------------------------------
 LONG EQFBCurSegFromCursor
 (
    PTBDOCUMENT  pDoc                        // pointer to doc ida
 )
 {
    PTBROWOFFSET pTBTemp;                    // pointer to screen row offset
    ULONG        ulSegNum;                   // segment for cursor
    SHORT        sSegOffset;                 // offset where to start
    ULONG        ulColumn;                   // offset from start of line
    USHORT       usOffset = 0;               // offset within screen row
    BOOL         fFound = FALSE;             // not found until now
    PSZ_W        pData = NULL;               // pointer to data
    PTBSEGMENT   pSeg;                       // pointer to segment

   if ( pDoc->hwndRichEdit )
   {
     if ( pDoc->pDispFileRTF->fTBCursorReCalc )
     {
       CHARRANGE chRange;
       SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
       EQFBGetSegFromCaretRTF( pDoc, &pDoc->TBCursor, chRange.cpMax );
       pDoc->pDispFileRTF->fTBCursorReCalc = FALSE;
     } /* endif */
     /*****************************************************************/
     /* if cursor at end of segment, 0 should be returned             */
     /* HOW can I check that?                                         */
     /*****************************************************************/
     return 1;
   } /* endif */

    pTBTemp = pDoc->TBRowOffset+1+pDoc->lCursorRow; // get TB struct from cursor

    ulSegNum = pTBTemp->ulSegNum;            // segment number
    sSegOffset = pTBTemp->usSegOffset;       // segment offset where to start

    ulColumn = pDoc->lCursorCol + pDoc->lSideScroll;  // position within line
    pSeg = EQFBGetVisSeg( pDoc, &ulSegNum );
    pData = ( pSeg ) ? pSeg->pDataW : NULL; // set data pointer

   if ( pData && ulSegNum )
   {
     while ( (usOffset <= ulColumn) && ! fFound )
     {
        switch ( EQFBCharType( pDoc, pSeg, sSegOffset ))
        {
          case LINEBREAK_CHAR:                 // end of segment reached
             fFound = TRUE;
             if ( ! *pData  )                  // special case for empty segm.
             {
                sSegOffset = 0;
                ulColumn = 1;
             } /* endif */
             break;
           case COMPACT_CHAR:
           case TRNOTE_CHAR:
          case PROTECTED_CHAR:
          case UNPROTECTED_CHAR:
          case SHORTEN_CHAR:
             usOffset++;                       // next character found

             break;
          case ENDOFSEG_CHAR:                  // indicate end of segment
           if (pDoc->EQFBFlags.PostEdit && pDoc->pUserSettings->fSegBound )
           {
             usOffset++;
           } /* endif */

           if (usOffset <= ulColumn )
           {
             ulSegNum++;                       // point to new segment
             sSegOffset = -1;                  // start of next segment
                                               // will increase at end of loop
             pSeg = EQFBGetVisSeg( pDoc, &ulSegNum );
             pData = ( pSeg ) ? pSeg->pDataW : NULL; // set data pointer
             if ( ! pData )                    // EOF reached
             {
               fFound = TRUE;
               /*******************************************************/
               /* set cursor one row higher ...                       */
               /*******************************************************/
               if ( pDoc->lCursorRow )
               {
                 pDoc->lCursorRow --;
               } /* endif */
               pTBTemp = pDoc->TBRowOffset+1+pDoc->lCursorRow;
               ulSegNum = pTBTemp->ulSegNum;   // segment number
               sSegOffset = pTBTemp->usSegOffset; // segment offset to start
             } /* endif */
           } /* endif */
             break;
          case HIDDEN_CHAR:                    // skip this character
             break;
          default:
             break;
        } /* endswitch */
        sSegOffset ++;                        // point to next character
     } /* endwhile */

     // get rid of blanks at end of line....
//     if ( sSegOffset >= 0 )
//     {
//      sSegOffset --;
//     } /* endif */
     if ( sSegOffset > 0 )
     {
      sSegOffset --;
     } /* endif */
   }
   else
   {
      ulSegNum = 0;                              // init segment number
      sSegOffset = 0;
   } /* endif */

   (pDoc->TBCursor).ulSegNum = ulSegNum;            // current cursor segment
   (pDoc->TBCursor).usSegOffset = sSegOffset;       // offset within segment
   return (usOffset - ulColumn);
 }

//-------------------------------------------------------------------
// void EQFBScreenCursor(PTBDOCUMENT) - position the real cursor
//                                    and the slider
//
// Note: Line length is set to 255 characters by default
//
//-------------------------------------------------------------------

 VOID EQFBScreenCursor( PTBDOCUMENT pDoc)   // active document
 {
    ULONG  ulDelta = 0;
    USHORT usScrollPos;

   EQFBSysScrnCurPos( pDoc, pDoc->lCursorRow, pDoc->lCursorCol);

   if ( (LONG)pDoc->ulMaxLine < pDoc->lScrnRows )
   {
      if ( pDoc->TBRowOffset[0].ulSegNum )
      {
         ulDelta++;
      } /* endif */
      if ( pDoc->TBRowOffset[pDoc->lScrnRows].ulSegNum )
      {
         ulDelta++;
      } /* endif */
   } /* endif */
                               // Set horizontal scroll slider
   /*******************************************************************/
   /* set horizontal scroll slider                                    */
   /*******************************************************************/
   usScrollPos = (USHORT)min( pDoc->lSideScroll, 254 );

   if (pDoc->fTARight)
   {
		if (usScrollPos < 255)
		{
			SetScrollPos(pDoc->hwndFrame, SB_HORZ,255 - usScrollPos, TRUE );
		}
		else
		{
			SetScrollPos(pDoc->hwndFrame, SB_HORZ,0, TRUE );
		}
   }
   else
   {
		SetScrollPos(pDoc->hwndFrame, SB_HORZ,usScrollPos, TRUE );
   }

   ulDelta = pDoc->TBCursor.ulSegNum ;
   if ( ulDelta )
   {
     ulDelta--;                          // 0 based
   } /* endif */
   /*******************************************************************/
   /* adjust for begin of file                                        */
   /*******************************************************************/
   if ( (pDoc->lCursorRow == 0) && (pDoc->TBRowOffset[0].ulSegNum == 0) )
   {
     ulDelta = 0;
   }
   else if ( !ulDelta )
   {
     if ( pDoc->lCursorRow || (pDoc->TBRowOffset[0].ulSegNum == 1) )
     {
       ulDelta++;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* adjust for EOF                                                  */
   /*                                                                 */
   /* if file contains only 2 segments we have to provide for         */
   /* scrolling purposes an extra one....                             */
   /*******************************************************************/
   if ( ulDelta && pDoc->ulMaxSeg < 4 )
   {
     ulDelta ++;
   } /* endif */

   /*******************************************************************/
   /* if something left over in file but position already at end      */
   /* adjust it....                                                   */
   /*******************************************************************/
   if ( pDoc->TBRowOffset[pDoc->lCursorRow+2].ulSegNum )
   {
     if ( ulDelta >= pDoc->ulMaxSeg - 2 )
     {
       ulDelta = max( pDoc->ulMaxSeg-2, 2) -1 ;
     } /* endif */
   }
   else
   {
     ulDelta = max( pDoc->ulMaxSeg-2, 2);
   } /* endif */
   if ((pDoc->ulMaxSeg) > MAX_VSCROLLRANGE )              // for big docs
   {
     //ulDelta = ulDelta >> 1;
     double dbMaxSeg = (double)pDoc->ulMaxSeg;
     double dbRange = (double)MAX_VSCROLLRANGE;
     double dbValue = (double)ulDelta * dbRange / dbMaxSeg;
     ulDelta = (ULONG)dbValue;
   } /* endif */
   SetScrollPos(pDoc->hwndFrame, SB_VERT, ulDelta, TRUE);

   // entity handling
   if ( UtlQueryUShort( QS_ENTITYPROCESSING ) && pDoc->lEntity )
   {
     PSTEQFGEN pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen;
     PDOCUMENT_IDA  pDocIda = (PDOCUMENT_IDA)pstEQFGen->pDoc;

     pDoc->szNewStatusLine[0] = EOS;

     if ( isEntityMarkup( pDocIda->szDocFormat ) )
     {
       // get word under cursor
       PTBSEGMENT pSeg;
       BOOL fFound;
       int iOffs = pDoc->TBCursor.usSegOffset;

       EQFBCurSegFromCursor( pDoc );
       pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );

       // look for entity
       while ( (iOffs != 0) && (pSeg->pDataW[iOffs] != L'&') && (iswalnum(pSeg->pDataW[iOffs]) || (pSeg->pDataW[iOffs] != L';'))  )
       {
         iOffs--;
       } /*endwhile */
       if ( (pSeg->pDataW[iOffs] == L'&') )
       {
         int iEnd = iOffs;
         // find end of entity
         while ( (pSeg->pDataW[iEnd] != L';') && (pSeg->pDataW[iEnd] != L' ') && pSeg->pDataW[iEnd] )
         {
           iEnd++;
         } /*endwhile */
         if ( (pSeg->pDataW[iOffs] == L'&') )
         {
          int iLen = iEnd - iOffs - 1;
          int iMaxLen = (sizeof(pDoc->szCurWord) / sizeof(CHAR_W)) - 1;
          if ( iLen > iMaxLen ) iLen = iMaxLen;
          wcsncpy( pDoc->szCurWord, pSeg->pDataW + iOffs + 1, iLen );
          pDoc->szCurWord[iLen] = 0;
          pDoc->szCurValue[0] = 0;

          fFound = GetEntityValue( pDoc->lEntity, pDoc->szCurWord, pDoc->TBCursor.ulSegNum,
                                   UtlGetFnameFromPath( pDoc->szDocName ), pDoc->szCurValue );

          sprintf( pDoc->szNewStatusLine, "&%S=%S", pDoc->szCurWord, pDoc->szCurValue );
         } /* endif */
       } /* endif */
     } /* endif */

     if ( strcmp( pDoc->szCurStatusLine, pDoc->szNewStatusLine ) != 0 )
     {
       strcpy( pDoc->szCurStatusLine, pDoc->szNewStatusLine );
       SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS, WM_EQF_UPDATESTATUSBAR_TEXT, 0, (LONG)(pDoc->szCurStatusLine) );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* update status bar                                               */
   /*******************************************************************/
   STATUSBAR( pDoc );

   /*******************************************************************/
   /* update ruler bar                                                */
   /*******************************************************************/
   RULER( pDoc );
   return;
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBSysScrnUpdate
//------------------------------------------------------------------------------
// Function call:     EQFBSysScrnUpdate(PTBDOCUMENT,BOOL)
//------------------------------------------------------------------------------
// Description:       enable or disable screen update
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//                    BOOL fUpdate   flag to indicate whether update allowed
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     Wet status of attribute
//                    if already reversed then undo it
//                    read char from string, do the attribute
//                          manipulation and write it
//                    (for the passed number of characters)
//                     ** in our case it is a macro only **
//------------------------------------------------------------------------------

void  EQFBSysScrnUpdate
(
  PTBDOCUMENT pDoc,
  BOOL        fUpdate                     // update allowed or not
)
{
  if (fUpdate)
  {
   UpdateWindow(pDoc->hwndClient);
  }
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBSetScrnUpdate
//------------------------------------------------------------------------------
// Function call:     EQFBSetScrnUpdate(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       set sreen update region
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc   pointer to document instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     set start and end of the area to be invalidated for
//                    the next display
//------------------------------------------------------------------------------
static
VOID EQFBSetScrnUpdate
(
   PTBDOCUMENT  pDoc,                     // pointer to document ida
   ULONG        ulRow,                    // start row
   ULONG        ulOffset,                 // offset within row
   LONG         lLength                  // length
)
{
   ULONG  ulStart;

   ulStart = ulRow * pDoc->usMaxScrnCols + ulOffset;
                        // set update region
   if ( pDoc->usScrnUpdStart > 0 )
   {
      pDoc->usScrnUpdStart = (USHORT)min( pDoc->usScrnUpdStart, ulStart );
   }
   else
   {
      pDoc->usScrnUpdStart = (USHORT) ulStart;
   } /* endif */

   pDoc->usScrnUpdEnd = (USHORT)max( pDoc->usScrnUpdEnd, ulStart + lLength );

}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBCheckForBlockMark
//------------------------------------------------------------------------------
// Function call:     EQFBCheckForBlockMark
//------------------------------------------------------------------------------
// Description:
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc   pointer to document instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------
static
BOOL EQFBCheckForBlockMark
(
   PTBDOCUMENT  pDoc,                     // pointer to document ida
   PEQFBBLOCK   pstBlock,                                  // block struct
   ULONG        ulSegNum,
   SHORT        sSegOffs                  // offset
)
{

  BOOL         fMark = FALSE;
     if (pDoc == pstBlock->pDoc )
     {
       if ((ulSegNum == pstBlock->ulSegNum) &&
           (ulSegNum == pstBlock->ulEndSegNum) &&
           ( (SHORT)pstBlock->usStart <= sSegOffs) &&
           ( sSegOffs <= (SHORT) pstBlock->usEnd)   )
       {
         fMark = TRUE;
       }
       else if ( (ulSegNum == pstBlock->ulSegNum) &&
                 (ulSegNum < pstBlock->ulEndSegNum) &&
                 ((SHORT)pstBlock->usStart <= sSegOffs)  )
       {
         fMark = TRUE;
       }
       else if ((pstBlock->ulSegNum < ulSegNum)      &&
                (ulSegNum == pstBlock->ulEndSegNum) &&
                ( (SHORT)pstBlock->usEnd >= sSegOffs)   )
       {
         fMark = TRUE;
       }
       else if ( ( pstBlock->ulSegNum < ulSegNum) &&
                 ( ulSegNum < pstBlock->ulEndSegNum ))
       {
         fMark = TRUE;
       }
       else
       {
         fMark = FALSE;
       } /* endif */
     } /* endif */

  return (fMark);
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBGetHLType
//------------------------------------------------------------------------------
// Function call:     EQFBGetHLType
//------------------------------------------------------------------------------
// Description:
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc   pointer to document instance
//                    PTBSEGMENT  pSeg
//                    SHORT       sSegOffs
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------
USHORT
EQFBGetHLType
(
    PTBSEGMENT    pSeg,
    SHORT         sSegOffs
)
{
  USHORT         usHLState = NO_HIGHLIGHT;
  PSTARTSTOP     pstCurrent;

  if (pSeg->pusHLType)
  {
     // look for position in start/stop table
     pstCurrent = (PSTARTSTOP) pSeg->pusHLType;
     while ( (pstCurrent->usType != 0) && (sSegOffs > (SHORT)pstCurrent->usStop) )
     {
        pstCurrent++;
     } /* endwhile */

     if ((pstCurrent != 0) && (sSegOffs >= pstCurrent->usStart))
     {
        usHLState = pstCurrent->usType;
     }
     else
     {
        usHLState = NO_HIGHLIGHT;
     } /* endif */
  } /* endif */
  return (usHLState);
} /* end of EQFBGetHLType */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBSegColRTF
//------------------------------------------------------------------------------
// Function call:     EQFBSegColRTF(............................TYLE)
//------------------------------------------------------------------------------
// Description:       get the active charformat            depending on status
//------------------------------------------------------------------------------
// Parameters:        PTBSEGMENT  pSeg,       active segment
//                    USHORT      usState,    currently active state
//------------------------------------------------------------------------------
// Returncode type:   CHARFORMAT2
//------------------------------------------------------------------------------
// Returncodes:       the active charformat
//------------------------------------------------------------------------------
// Function flow:     if ptr to segment is not 0
//                      switch (status of segment)
//------------------------------------------------------------------------------

PSZ_W  EQFBSegColRTF
(
   CHARFORMAT2 *pNewCharFormat,
   PTBSEGMENT   pSeg,                         // active segment
   USHORT       usState,                      // currently active state
   USHORT       usHLState,                    // highlighting state
   DISPSTYLE    DispStyle,                    // currently active disp Style
   BOOL         fPostEdit,                       // TRUE if Postedit
   PDISPFILERTF pDispFileRTF
)
{
   PSZ_W        pRTFFont = NULL;
   USHORT       usFontIndex = 0;
   CHARFORMAT2* pFormatTbl1;
   CHARFORMAT2* pFormatTbl2;
   SHORT        sStrikeOut = 0;
   SHORT        sUnderline = UNDERLINE_NOSPEC;
   SHORT        sStyle = STYLE_STANDARD;
   USHORT       usOriginalState = 0;
   CHARFORMAT2* paszFontExtSpecs = get_aszFontExtSpecs();
   usOriginalState = usState;

   if ( (DispStyle == DISP_SHRINK) || (DispStyle == DISP_COMPACT)
        || (DispStyle == DISP_SHORTEN)   )
   {
     DispStyle = DISP_PROTECTED;                 // to get it correctly displayed
     usState = (usState == COMPACT_CHAR) ? PROTECTED_CHAR : usState;
   } /* endif */
   if ((usState == ENDOFSEG_CHAR) || (usState == TRNOTE_CHAR) ||
             (usState == SHORTEN_CHAR) )
   {
     usState = PROTECTED_CHAR;
   } /* endif */
   if ( pSeg )
   {
     switch (pSeg->qStatus & 0x0FF)
     {
       case QF_TOBE:
       case QF_ATTR:
       case QF_PROP0TEXT:                       // proposal 0
          pFormatTbl1 = paszFontExtSpecs + EXT_T_SOURCE;
          pFormatTbl2 = paszFontExtSpecs + EXT_T_LAYOUT;
          if (usState == PROTECTED_CHAR )
          {                            //COLOUR_P_TOBE
            EQFBGetStrikeULine(pFormatTbl2,
                               &sStrikeOut, &sUnderline, &sStyle);

            EQFBSetCharFormat2 (pNewCharFormat,
                  pFormatTbl2->szFaceName,
                  pFormatTbl2->bCharSet,
                  pFormatTbl2->bPitchAndFamily,
                  pFormatTbl2->yHeight,
                  sStyle,
                  pFormatTbl2->crTextColor,
                  pFormatTbl1->crBackColor,
                  sUnderline,
                  sStrikeOut );
            usFontIndex = PROTECT_IN_SOURCE;
          }
          else
          {                                            //COLOUR_TOBE
            memcpy (pNewCharFormat, pFormatTbl1, sizeof(CHARFORMAT2));

	        usFontIndex = EXT_T_SOURCE;
          } /* endif */

          break;
       case QF_XLATED:                         // already translated
       case QF_PROPNTEXT:                       // proposal n
          pFormatTbl1 = paszFontExtSpecs + EXT_T_TRANSLATION;
          pFormatTbl2 = paszFontExtSpecs + EXT_T_LAYOUT;
          if (usState == PROTECTED_CHAR )
          {                            //COLOUR_P_XLATED
            EQFBGetStrikeULine(pFormatTbl2,
                               &sStrikeOut, &sUnderline, &sStyle);

            EQFBSetCharFormat2 (pNewCharFormat,
                  pFormatTbl2->szFaceName,
                  pFormatTbl2->bCharSet,
                  pFormatTbl2->bPitchAndFamily,
                  pFormatTbl2->yHeight,
                  sStyle,
                  pFormatTbl2->crTextColor,
                  pFormatTbl1->crBackColor,
                  sUnderline,
                  sStrikeOut );
            usFontIndex = PROTECT_IN_TRANSLATION;
          }
          else
          {                                            //COLOUR_XLATED

            memcpy (pNewCharFormat, pFormatTbl1, sizeof(CHARFORMAT2));
            usFontIndex = EXT_T_TRANSLATION;
          } /* endif */

          if (pSeg->qStatus == QF_XLATED && fPostEdit &&
              (usState != PROTECTED_CHAR) &&
              ( pSeg->SegFlags.Typed || pSeg->SegFlags.Copied ) )
          {
            if ( pSeg->SegFlags.Typed )
            {
              if ( pSeg->SegFlags.Copied )
              {
                pFormatTbl1 = paszFontExtSpecs + EXT_R_MODIFIED;
                usFontIndex = EXT_R_MODIFIED;
              }
              else
              {
                pFormatTbl1 = paszFontExtSpecs + EXT_R_SCRATCH;
                usFontIndex = EXT_R_SCRATCH;
              } /* endif */
            }
            else
            {
              pFormatTbl1 = paszFontExtSpecs + EXT_R_COPIED;
                usFontIndex = EXT_R_COPIED;
            } /* endif */
            memcpy (pNewCharFormat, pFormatTbl1, sizeof(CHARFORMAT2));
          } /* endif */
          break;
       case QF_NOP:                            // nop, tag
          if (usState == PROTECTED_CHAR )
          {                            //COLOUR_P_TOBE
            memcpy (pNewCharFormat, paszFontExtSpecs + EXT_T_LAYOUT,
                     sizeof(CHARFORMAT2));

            usFontIndex = EXT_T_LAYOUT;
          }
          else
          {                                            //COLOUR_TOBE

            memcpy (pNewCharFormat, paszFontExtSpecs + EXT_T_SOURCE,
                      sizeof(CHARFORMAT2));

	        usFontIndex = EXT_T_SOURCE;
          } /* endif */
          break;
        case QF_CURRENT:                         // current segment
          pFormatTbl1 = paszFontExtSpecs + EXT_T_ACTIVE;
          pFormatTbl2 = paszFontExtSpecs + EXT_T_LAYOUT;

          if (usState == PROTECTED_CHAR )
          {
            EQFBGetStrikeULine(pFormatTbl2,
                               &sStrikeOut, &sUnderline, &sStyle);

            EQFBSetCharFormat2 (pNewCharFormat,
                  pFormatTbl2->szFaceName,
                  pFormatTbl2->bCharSet,
                  pFormatTbl2->bPitchAndFamily,
                  pFormatTbl2->yHeight,
                  sStyle,
                  pFormatTbl2->crTextColor,
                  pFormatTbl1->crBackColor,
                  sUnderline,
                  sStrikeOut );
            usFontIndex = PROTECT_IN_CURRENT;
          }
          else
          {
            memcpy (pNewCharFormat, pFormatTbl1, sizeof(CHARFORMAT2));

			usFontIndex = EXT_T_ACTIVE;
          } /* endif */

          break;
       case QF_PROP0PREFIX:                     // proposal 0 prefix
       case QF_PROPNPREFIX:                     // proposal n prefix

          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_M_PREFIX,
                    sizeof(CHARFORMAT2));

	      usFontIndex = EXT_M_PREFIX;
          break;
       case QF_DICTHEAD:                        // colour of dictionary header

          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_D_HEADWORD,
                    sizeof(CHARFORMAT2));

	      usFontIndex = EXT_D_HEADWORD;
          break;
       case QF_DICTTRANS:                       // colour of dict. translation

          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_D_TRANSLATION,
                    sizeof(CHARFORMAT2));

	      usFontIndex = EXT_D_TRANSLATION;
          break;
       case QF_DICTPREFIX:                      // colour of dict prefix

          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_D_PREFIX,
                    sizeof(CHARFORMAT2));

	      usFontIndex = EXT_D_PREFIX;
          break;
       case QF_DICTADDINFO:                     // colour of additional dict info

          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_D_ADDITIONINFO,
                    sizeof(CHARFORMAT2));

	      usFontIndex = EXT_D_ADDITIONINFO;
          break;
       case QF_PROPSRCEQU:                      // proposal source equal text

          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_T_TRANSLATION,
                    sizeof(CHARFORMAT2));

	      usFontIndex = EXT_T_TRANSLATION;
          break;
       case QF_PROPSRCUNEQU:                    // proposal source modified

          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_M_PMODIFIED,
                    sizeof(CHARFORMAT2));

	      usFontIndex = EXT_M_PMODIFIED;
          break;
       case QF_PROPSRCDEL:                      // proposal source deleted text

          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_M_PDELETED,
                    sizeof(CHARFORMAT2));

	      usFontIndex = EXT_M_PDELETED;
          break;
       case QF_PROPSRCINS:                      // proposal source inserted text

          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_M_PINSERTED,
                    sizeof(CHARFORMAT2));
          usFontIndex = EXT_M_PINSERTED;

          break;

       case QF_DICTINDIC:                       // colour of dict prefix

          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_D_ADDITIONINFO,
                    sizeof(CHARFORMAT2));

	      usFontIndex = EXT_D_ADDITIONINFO;
          break;

        case QF_TRNOTE_L1_1:
        case QF_TRNOTE_L1_2:
        case QF_TRNOTE_L2:

          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_T_TRNOTE,
                    sizeof(CHARFORMAT2));

	      usFontIndex = EXT_T_TRNOTE;
          break;

        case QF_OWN_COLOR:

          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_T_SOURCE,
                    sizeof(CHARFORMAT2));

          usFontIndex = EXT_T_SOURCE;
          break;

       case QF_DICTSTYLEPREF:
          pFormatTbl1 = paszFontExtSpecs + EXT_D_TRANSLATION;
          EQFBGetStrikeULine( pFormatTbl1, &sStrikeOut, &sUnderline, &sStyle);
          EQFBSetCharFormat2( pNewCharFormat, pFormatTbl1->szFaceName, pFormatTbl1->bCharSet, pFormatTbl1->bPitchAndFamily,
                              pFormatTbl1->yHeight, sStyle,
                              COLORRGBTABLE[ COL_BLACK ], COLORRGBTABLE[ COL_LIGHTGREEN ],
                              sUnderline, sStrikeOut );
          usFontIndex = EXT_T_SOURCE;
          break;
       case QF_DICTSTYLENOT:
          pFormatTbl1 = paszFontExtSpecs + EXT_T_SOURCE;
          EQFBGetStrikeULine( pFormatTbl1, &sStrikeOut, &sUnderline, &sStyle);
          EQFBSetCharFormat2( pNewCharFormat, pFormatTbl1->szFaceName, pFormatTbl1->bCharSet, pFormatTbl1->bPitchAndFamily,
                              pFormatTbl1->yHeight, sStyle,
                              COLORRGBTABLE[ COL_BLACK ], COLORRGBTABLE[ COL_LIGHTRED ],
                              sUnderline, sStrikeOut );
          usFontIndex = EXT_T_SOURCE;
          break;

        default:
          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_T_SOURCE,
                    sizeof(CHARFORMAT2));

          usFontIndex = EXT_T_SOURCE;
          break;
     } /* endswitch */
   } /* endif */

   switch ( usHLState )
   {
     case TAG_HIGHLIGHT:
          memcpy (pNewCharFormat, paszFontExtSpecs + EXT_R_LAYOUTCHANGES,
                    sizeof(CHARFORMAT2));
          usFontIndex = EXT_R_LAYOUTCHANGES;
       break;
     default:
        break;
   } /* endswitch */
    if (usOriginalState == TRNOTE_CHAR )
    {
      memcpy (pNewCharFormat, paszFontExtSpecs + EXT_T_RTFEDIT_TRNOTE,
                        sizeof(CHARFORMAT2));
      usFontIndex = EXT_T_RTFEDIT_TRNOTE;
    } /* endif */

   if ( pDispFileRTF )
   {
     pRTFFont = ASCII2Unicode( pDispFileRTF->pCharFormat[usFontIndex],
                           &pDispFileRTF->chRTFFormatW[0], 0L );
//     pRTFFont = pDispFileRTF->pCharFormat[usFontIndex];
   }
   return pRTFFont;
}

void
EQFBGetStrikeULine
(    CHARFORMAT2 * pFormatTbl,
     PSHORT        psStrikeOut,
     PSHORT        psUnderline,
     PSHORT        psStyle
)
{
  if ((pFormatTbl->dwMask  & CFM_STRIKEOUT)
      && (pFormatTbl->dwEffects & CFE_STRIKEOUT)  )
  {
    *psStrikeOut = STRIKEOUT;
  } /* endif */
  if (pFormatTbl->dwMask & CFM_UNDERLINETYPE)
  {
    if (pFormatTbl->bUnderlineType == CFU_UNDERLINENONE )
    {
      *psUnderline = UNDERLINE_NONE;
    }
    else
    {
      *psUnderline = UNDERLINE_YES;
    } /* endif */
  } /* endif */
  if (pFormatTbl->dwMask & CFM_ITALIC &&
      pFormatTbl->dwEffects & CFE_ITALIC )
  {
    if (pFormatTbl->dwMask & CFM_BOLD &&
        pFormatTbl->dwEffects & CFE_BOLD )
    {
      *psStyle = STYLE_ITALBOLD;
    }
    else
    {
      *psStyle = STYLE_ITALIC;
    } /* endif */
  }
  else
  {
    if (pFormatTbl->dwMask & CFM_BOLD &&
        pFormatTbl->dwEffects & CFE_BOLD )
    {
      //bold
      *psStyle = STYLE_BOLD;
    }
    else
    {
      *psStyle = STYLE_STANDARD;
    } /* endif */
  } /* endif */

 return;
}
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBSegColRTF
//------------------------------------------------------------------------------
// Function call:     EQFBSegColRTF(............................TYLE)
//------------------------------------------------------------------------------
// Description:       get the active charformat            depending on status
//------------------------------------------------------------------------------
// Parameters:        PTBSEGMENT  pSeg,       active segment
//                    USHORT      usState,    currently active state
//------------------------------------------------------------------------------
// Returncode type:   CHARFORMAT2
//------------------------------------------------------------------------------
// Returncodes:       the active charformat
//------------------------------------------------------------------------------
// Function flow:     if ptr to segment is not 0
//                      switch (status of segment)
//------------------------------------------------------------------------------

VOID  EQFBSegColRTFInit
(
   CHARFORMAT2 *pNewCharFormat,
   USHORT      usI
)
{
   CHARFORMAT2* pFormatTbl1;
   CHARFORMAT2* pFormatTbl2;
   SHORT        sStrikeOut = 0;
   SHORT        sUnderline = UNDERLINE_NOSPEC;
   SHORT        sStyle = STYLE_STANDARD;
   BOOL         fIsHighContrast = UtlIsHighContrast();
   CHARFORMAT2* paszFontExtSpecs = get_aszFontExtSpecs();

   switch (usI)
   {
     case PROTECT_IN_SOURCE:
        pFormatTbl1 = paszFontExtSpecs + EXT_T_SOURCE;
        pFormatTbl2 = paszFontExtSpecs + EXT_T_LAYOUT;
        EQFBGetStrikeULine(pFormatTbl2,
                           &sStrikeOut, &sUnderline, &sStyle);
        if (fIsHighContrast)
        {
			pFormatTbl2->crTextColor = GetSysColor(COLOR_GRAYTEXT);
			pFormatTbl1->crBackColor = GetSysColor(COLOR_WINDOW);
	    }
        EQFBSetCharFormat2 (pNewCharFormat,
              pFormatTbl2->szFaceName,
              pFormatTbl2->bCharSet,
              pFormatTbl2->bPitchAndFamily,
              pFormatTbl2->yHeight,
              sStyle,
              pFormatTbl2->crTextColor,
              pFormatTbl1->crBackColor,
              sUnderline,
              sStrikeOut );
        break;
     case PROTECT_IN_TRANSLATION:
        pFormatTbl1 = paszFontExtSpecs + EXT_T_TRANSLATION;
        pFormatTbl2 = paszFontExtSpecs + EXT_T_LAYOUT;
        EQFBGetStrikeULine(pFormatTbl2,
                           &sStrikeOut, &sUnderline, &sStyle);
        if (fIsHighContrast)
        {
			pFormatTbl2->crTextColor = GetSysColor(COLOR_GRAYTEXT);  // green
			pFormatTbl1->crBackColor = GetSysColor(COLOR_WINDOW);
	    }
        EQFBSetCharFormat2 (pNewCharFormat,
              pFormatTbl2->szFaceName,
              pFormatTbl2->bCharSet,
              pFormatTbl2->bPitchAndFamily,
              pFormatTbl2->yHeight,
              sStyle,
              pFormatTbl2->crTextColor,
              pFormatTbl1->crBackColor,
              sUnderline,
              sStrikeOut );
        break;
     case PROTECT_IN_CURRENT:
        pFormatTbl1 = paszFontExtSpecs + EXT_T_ACTIVE;
        pFormatTbl2 = paszFontExtSpecs + EXT_T_LAYOUT;
        if (fIsHighContrast)
        {
			pFormatTbl2->crTextColor = GetSysColor(COLOR_GRAYTEXT);  // lila
			pFormatTbl1->crBackColor = GetSysColor(COLOR_WINDOW);
	    }
        EQFBGetStrikeULine(pFormatTbl2,
                           &sStrikeOut, &sUnderline, &sStyle);
        EQFBSetCharFormat2 (pNewCharFormat,
              pFormatTbl2->szFaceName,
              pFormatTbl2->bCharSet,
              pFormatTbl2->bPitchAndFamily,
              pFormatTbl2->yHeight,
              sStyle,
              pFormatTbl2->crTextColor,
              pFormatTbl1->crBackColor,
              sUnderline,
              sStrikeOut );
        break;
      default:
        memcpy(pNewCharFormat, paszFontExtSpecs + usI, sizeof( CHARFORMAT2 ));
		if (fIsHighContrast)
	    {
			pNewCharFormat->crTextColor = GetSysColor(COLOR_WINDOWTEXT);
			pNewCharFormat->crBackColor = GetSysColor(COLOR_WINDOW);
	    }
        break;
   } /* endswitch */

   return ;
}
