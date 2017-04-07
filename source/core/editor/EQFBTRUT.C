/*! \file
	Description: EQF Translation Utilities

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_BASE
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_MORPH
#include <eqf.h>                  // General Translation Manager include file
#include "eqfdoc00.h"                  // private document handler include file
/**********************************************************************/
/* The next include is necessary due to inconsistent or               */
/* not existing interfaces ...                                        */
/**********************************************************************/
//#include <eqftai.h>               // private text analysis include file

//#include "eqfdoc00.h"             // private document handler include file

#include "EQFTPI.H"               // Translation Processor priv. include file

#define MSGBOXDATALEN     30        // data len of segment to be displ.

#define CHECKPOS_XLATESEG 0         // same position
#define CHECKPOS_MOVE   1           // user movement
#define CHECKPOS_XLATED 2           // already translated
#define CHECKPOS_FIRST  3           // fist call to tm
#define CHECKPOS_POSTEDIT 4         // post edit mode was active



#define  EXACT_MATCH    1           // exact match


#ifdef TRANTIME
  static  ULONG  ulTime = 0L;
  static  ULONG  ulSave = 0L;
  static  ULONG  ulStart;
  static  ULONG  ulEnd;
  static  ULONG  ulBegin;
  static  ULONG  ulDelta = 0L;
  FILE    *fStream;
#endif

static VOID EQFBDelSeg ( PTBDOCUMENT, ULONG ); // delete segment from TM

static BOOL EQFBJoinSegData ( PTBDOCUMENT, ULONG, ULONG ); // join segment data

static BOOL EQFBSplitSegData ( PTBDOCUMENT, ULONG, ULONG, USHORT, USHORT );

static PTBSEGMENT EQFBFindNextAutoSource( PTBDOCUMENT, PULONG, PVOID * ); // find next source special

static CHAR_W chSeg1[MSGBOXDATALEN + 1];  // start of segment 1
static CHAR_W chSeg2[MSGBOXDATALEN + 1];  // start of segment 2


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBSplitSeg - split one segment
//------------------------------------------------------------------------------
// Function call:     EQFBSplitSeg( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       split previously joined segments
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT     pointer to document instance data
//
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
VOID
EQFBSplitSeg
(
  PTBDOCUMENT pDoc
)
{
   PSZ_W      pSeg[3];                   // pointer array to segments
   PTBSEGMENT pTBSeg;                    // pointer to segment
   PTBSEGMENT pTBFirstSeg;               // pointer to first segment
   PTBSEGMENT pTBFirstSegTwin;                   // ptr to 1st twin seg
   PTBSEGMENT pTBSegTwin;
   ULONG      ulJ;                       // index
   USHORT     usMBId;                    // return code from UtlError
   ULONG      ulSegNum;                  // segment number
   BOOL       fOK = TRUE;                // success indicator
   SHORT      sRc;                       // return code from DocSave
   PEQFBBLOCK pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;     // block structure
   USHORT     usLenSeg1Twin;             // len of 1st twin seg w/o joined
   USHORT     usLenSeg2Twin;             // len of twin( seg2+..+lastsegjoined)
   USHORT     usLength;                   // length of segment

   if (pDoc->EQFBFlags.Reflow)            //trunc. only if reflow allowed
   {
     if (pDoc->hwndRichEdit )
     {
        pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
        EQFBUpdateTBCursor( pDoc );
        EQFBGetWorkSegRTF(pDoc, pDoc->ulWorkSeg );
     } /* endif */
      pTBFirstSeg = pTBSeg = EQFBGetSegW(pDoc, pDoc->tbActSeg.ulSegNum);
      if ( pDoc->tbActSeg.ulSegNum != pDoc->TBCursor.ulSegNum )
      {
         UtlError( TB_NOTINACTSEG, MB_CANCEL, 0, NULL, EQF_WARNING);
      }
      else if (!pTBSeg->SegFlags.JoinStart )
      {
         UtlError( TB_NOSEGJOINED, MB_CANCEL, 0, NULL, EQF_WARNING);
      }
      else
      {                                     // length of source
         pTBFirstSegTwin = EQFBGetSegW(pDoc->twin, pDoc->tbActSeg.ulSegNum);

         /*************************************************************/
         /*calc original length of this segment when it was not joined*/
         /*************************************************************/

         usLenSeg1Twin = (USHORT)UTF16strlenCHAR(pTBFirstSegTwin->pDataW);

         // scan thru joined segments and subtract the length of the last joined segment
         ulJ = pDoc->tbActSeg.ulSegNum + 1;
         pTBSegTwin = EQFBGetSegW(pDoc->twin, ulJ);
         usLenSeg2Twin = 0;
         while (pTBSegTwin && pTBSegTwin->SegFlags.Joined)
         {
           usLenSeg2Twin = (USHORT)UTF16strlenCHAR(pTBSegTwin->pDataW);
           ulJ++;
           pTBSegTwin = EQFBGetSegW(pDoc->twin, ulJ);
         } /* endwhile */

         usLenSeg1Twin = usLenSeg1Twin - usLenSeg2Twin;

         /*************************************************************/
         /* fill strings to display message                           */
         /*************************************************************/
         usLength = min(MSGBOXDATALEN, usLenSeg1Twin);
         UTF16strncpy( chSeg1, pTBFirstSegTwin->pDataW, usLength);
         chSeg1[ usLength ] = EOS;   // set end of string
         if ( UTF16strlenCHAR(chSeg1) >= MSGBOXDATALEN )
         {                                // fill last 3 chars with '...'
            UTF16strcpy(chSeg1+MSGBOXDATALEN-4,L"...");
         } /* endif */

         pSeg[0] = chSeg1;
                                          // limit second segment
         usLength = min(MSGBOXDATALEN, usLenSeg2Twin);
         UTF16strncpy( chSeg2, pTBFirstSegTwin->pDataW+usLenSeg1Twin, usLength);
         chSeg2[ usLength ] = EOS;   // set end of string
         if ( UTF16strlenCHAR(chSeg2) >= MSGBOXDATALEN )
         {                                // fill last 3 chars with '...'
            UTF16strcpy(chSeg2+MSGBOXDATALEN-4, L"...");
         } /* endif */
         pSeg[1] = chSeg2;

         // display message for Split segment message box
         usMBId = UtlErrorW( TB_JOINEDSEG, MB_YESNO | MB_DEFBUTTON2,
                            2, &pSeg[0], EQF_QUERY, TRUE);
         if ( usMBId == MBID_YES )                 // split the segments
         {

            EQFBWorkSegOut( pDoc );              // save the current segment
            if ( EQFBSplitSegData( pDoc, pDoc->tbActSeg.ulSegNum, (ULONG)(ulJ-1),
                                    usLenSeg1Twin, usLenSeg2Twin ))
            {
                                                 // save source seg file
               sRc = EQFBDocSave( pDoc->twin, pDoc->twin->szDocName, TRUE);
               if ( !sRc )
               {
                  EQFBDocSave( pDoc, pDoc->szDocName, TRUE);// save tgt seg file
               } /* endif */
            } /* endif */
            EQFCLEAR( 0 );                       // clear the service buf
            ulSegNum = pDoc->tbActSeg.ulSegNum;
            // remove any remaining block mark
            if ( pstBlock->pDoc == pDoc &&
                   pstBlock->ulSegNum == ulSegNum )
            {
               pstBlock->pDoc = NULL;             // reset block mark
            } /* endif */
            /**********************************************************/
            /* activate first segment                                 */
            /**********************************************************/
            fOK = EQFBSendNextSource( pDoc,        // pointer to document
                                      &ulSegNum,   // ptr to new segment
                                      TRUE,        // foreground mode
                                      POS_CURSOR); // position at cursor

            if ( fOK )
            {
              EQFBActivateSegm( pDoc, ulSegNum ); // activate the segment
            } /* endif */
            if (pDoc->hwndRichEdit )
            {
              USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
              EQFBSetSplitStartSegRTF(pDoc, pDoc->tbActSeg.ulSegNum );
              EQFBSetSplittedSegRTF(pDoc, pDoc->tbActSeg.ulSegNum + 1 );
              if ( pEQFBUserOpt->sFocusLine )
              {
                ULONG ulLine = (ULONG) SendMessage( pDoc->hwndRichEdit,
                                                    EM_EXLINEFROMCHAR, 0, -1 );
                ULONG ulFirstLine = (ULONG) SendMessage(
                                                pDoc->hwndRichEdit,
                                                EM_GETFIRSTVISIBLELINE, 0,0);
                LONG lDiff = ulLine - ulFirstLine - pEQFBUserOpt->sFocusLine + 1;
                SendMessage( pDoc->hwndRichEdit, EM_LINESCROLL, 0, lDiff );
              } /* endif */

            } /* endif */
         } /* endif */
      } /* endif */
   } /* endif */
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBJOINSEG - join segments
//------------------------------------------------------------------------------
// Function call:     EQFBJoinSeg( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       join 2 segments
//                                           allowed
//                       combinations       combinations
//
//                     ATTRTODO / ATTRTODO       x
//                     ATTRTODO / TODO           x
//                     ATTRTODO / NOP            x
//                     ATTRTODO / XLATED         x
//                     TODO     / TODO           x
//                     TODO     / NOP            x
//                     TODO     / XLATED         x
//                     NOP      / NOP
//                     NOP      / XLATED         x
//                     XLATED   / XLATED         x
//
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:      Files will be saved
//
//------------------------------------------------------------------------------
// Function flow:     -if not in active window then
//                       issue error message
//                     else:
//                      - if combined segment would be larger as
//                           allowed maximum then
//                             display error message and cancel request
//                             (maximum is the length of the work segment)
//                        else
//                        - display msgbox with the two segments to be joined
//                          if Yes then
//                            - call delete 'old' segments
//                            - update seg. identifications(EQFBJoinSegData)
//                              if OK then
//                                save documents
//                                send 'new' source to services
//                                acitvate joined segment
//                              endif
//                          endif
//                        endif
//                      endif
//
//------------------------------------------------------------------------------
VOID
EQFBJoinSeg
(
  PTBDOCUMENT pDoc
)
{
   PSZ_W      pSeg[3];                    // pointer array to segments
   PTBSEGMENT pTBStartSeg;                // pointer to segment
   PTBSEGMENT pTBJoinedSeg;               // segment to be joined
   ULONG      ulSegNum;                   // segment number
   BOOL       fOK = TRUE;                 // success indicator
   USHORT     usMBId;                     // return code from UtlError
   USHORT     usLength;                   // length of the joined segments
   PSTEQFGEN  pstEQFGen;                  // pointer to generic structure
   SHORT      sRc;                        // return code from DocSave
   PEQFBBLOCK pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;// block structure
   BOOL       fChanged;                   // temporary seg.changed flag

   if (pDoc->EQFBFlags.Reflow)            //trunc. only if reflow allowed
   {
     if (pDoc->hwndRichEdit )
     {
        pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
        EQFBUpdateTBCursor( pDoc );
        EQFBGetWorkSegRTF(pDoc, pDoc->ulWorkSeg );
     } /* endif */
      ulSegNum = pDoc->tbActSeg.ulSegNum;
                                             // if not in active segment issue msg
      if ( ulSegNum != pDoc->TBCursor.ulSegNum )
      {
         UtlError( TB_NOTINACTSEG, MB_CANCEL, 0, NULL, EQF_WARNING);
      }
      else
      {
                                             // get first segment to be joined
          pTBStartSeg = EQFBGetSegW(pDoc, ulSegNum);
                                             // get the second one
          ulSegNum++;
          pTBJoinedSeg = EQFBGetVisSeg( pDoc, &ulSegNum );
          if (!pTBJoinedSeg || pTBJoinedSeg->ulSegNum == 0) //actseg is last seg
          {
             WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if no next segment
             fOK = FALSE;
          }
          else
          {
                                             // check for length of joined segment
             pTBJoinedSeg = EQFBGetSegW(pDoc, ulSegNum );  // get real len(Compact!!)
             usLength = (USHORT)(UTF16strlenCHAR(pTBStartSeg->pDataW)
                           + UTF16strlenCHAR(pTBJoinedSeg->pDataW) + 1);

             if ( usLength >= MAX_SEGMENT_SIZE )
             {
               UtlError( TB_JOINEDSEGTOOLONG, MB_CANCEL, 0, NULL, EQF_WARNING);
               fOK = FALSE;
             }
             else
             {                                  // display message what to join
                                                // limit first segment
               PSZ_W pTemp = pTBStartSeg->pDataW;
               while ( (*pTemp == BLANK) || (*pTemp == LF) )
               {
                 pTemp++;
               } /* endwhile */
               UTF16strncpy( chSeg1, pTemp, MSGBOXDATALEN);
               chSeg1[ MSGBOXDATALEN ] = EOS;   // set end of string
               if ( UTF16strlenCHAR(chSeg1) >= MSGBOXDATALEN )
               {                             // fill last three chars with ï...ï
                  UTF16strcpy(chSeg1+MSGBOXDATALEN-4,L"...");
               } /* endif */
               pSeg[0] = chSeg1;
                                             // limit second segment
               pTemp = pTBJoinedSeg->pDataW;
               while ( (*pTemp == BLANK) || (*pTemp == LF) )
               {
                 pTemp++;
               } /* endwhile */
               UTF16strncpy( chSeg2, pTemp, MSGBOXDATALEN);
               chSeg2[ MSGBOXDATALEN ] = EOS;   // set end of string
               if ( UTF16strlenCHAR(chSeg2) >= MSGBOXDATALEN )
               {                             // fill last three chars with ï...ï
                  UTF16strcpy(chSeg2+MSGBOXDATALEN-4,L"...");
               } /* endif */
               pSeg[1] = chSeg2;

               usMBId = UtlErrorW( TB_JOINSEG, MB_YESNO | MB_DEFBUTTON2,
                                  2, &pSeg[0], EQF_QUERY, TRUE);
               if ( usMBId == MBID_YES )        // user wants to join ???
               {
                  fChanged = pDoc->EQFBFlags.workchng; // save workseg changed
                  EQFBWorkSegOut( pDoc );       // reset current active segment
                                                //  .. delete segments in memory
                  EQFBDelSeg( pDoc, pDoc->tbActSeg.ulSegNum );
                  EQFBDelSeg( pDoc, ulSegNum );
                  pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
                  WinPostMsg( pstEQFGen->hwndTWBS, EQFM_DOC_IS_ULATED,
                              NULL, NULL);
                                                // update the segm. identifications
                  if ( EQFBJoinSegData(pDoc,pDoc->tbActSeg.ulSegNum,ulSegNum) )
                  {
                                                // save source seg file
                     pDoc->twin->ulWorkSeg = pDoc->tbActSeg.ulSegNum;
                     sRc = EQFBDocSave( pDoc->twin, pDoc->twin->szDocName, TRUE);
                     if ( !sRc )
                     {
                       /***********************************************/
                       /* set ulWorkSeg because EQFBDocSave traps     */
                       /* if ulWorkSeg = 0                            */
                       /***********************************************/
                       pDoc->ulWorkSeg = pDoc->tbActSeg.ulSegNum;
                       EQFBDocSave( pDoc, pDoc->szDocName, TRUE);// save tgt seg file
                     } /* endif */
                  } /* endif */
                  pDoc->EQFBFlags.workchng = (USHORT)fChanged; // restore workseg flag
                  EQFCLEAR( 0 );                // clear the internal service buffer
                                                // activate the current segment
                  ulSegNum = pDoc->tbActSeg.ulSegNum;
                  // remove any remaining block mark
                  if ( pstBlock->pDoc == pDoc &&
                         pstBlock->ulSegNum == ulSegNum )
                  {
                     pstBlock->pDoc = NULL;             // reset block mark
                  } /* endif */

                  fOK = EQFBSendNextSource( pDoc,      // pointer to document
                                            &ulSegNum, // pointer to new segment
                                            TRUE,      // send in foreground mode
                                            POS_CURSOR); // untransl. only

                  // store cursor position intermediately
                  usLength = pDoc->TBCursor.usSegOffset;
                  if ( fOK )
                  {
                     EQFBActivateSegm( pDoc, ulSegNum );// activate the current segment
                  } /* endif */
                  // reset cursor position to old position
                  pDoc->TBCursor.usSegOffset = usLength;
                  if (!pDoc->hwndRichEdit )
                  {
                    EQFBPhysCursorFromSeg(pDoc);          //position at SegOffset
                    pDoc->lDBCSCursorCol =
                                     pDoc->lCursorCol;     //store in DBCS column
                  }
                  else
                  {
                    EQFBSetJoinStartSegRTF(pDoc, pDoc->tbActSeg.ulSegNum );
                    EQFBSetJoinedSegRTF(pDoc, pTBJoinedSeg->ulSegNum );
                  } /* endif */
               } /* endif */
             } /* endif */
          } /* endif */
      } /* endif */
   } /* endif */
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBDelSeg - delete a segment from the translation memory
//------------------------------------------------------------------------------
// Function call:     EQFBDelSeg( PTBDOCUMENT, ULONG  );
//
//------------------------------------------------------------------------------
// Description:       delete a segment from the translation memory
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance
//                    ULONG             segment number
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     -  get the segment data  ( source )
//                    -  get the translation data
//                    -  issue a delete request
//                       (don't care if it is translated or
//                       not in the current segment)
//
//------------------------------------------------------------------------------

static VOID
EQFBDelSeg
(
   PTBDOCUMENT pDoc,                            // pointer to Document ida
   ULONG       ulSegNum                         // segment number
)
{
   PTBSEGMENT  pSegSource;                      // pointer to segment
   PTBSEGMENT  pSegTarget;                      // pointer to segment

   pSegTarget = EQFBGetSegW( pDoc, ulSegNum );
   pSegSource = EQFBGetSegW( pDoc->twin, ulSegNum );
   if ( pSegTarget && pSegSource)
   {
      EQFDELSEGW( pSegSource->pDataW, pSegTarget->pDataW, ulSegNum );
   } /* endif */

   return;
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBJoinSegData - join two segments
//------------------------------------------------------------------------------
// Function call:     EQFBJoinSegData( PTBDOCUMENT, USHORT, USHORT );
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
EQFBJoinSegData
(
   PTBDOCUMENT pDoc,                      // pointer to Document ida
   ULONG     ulSegNumStart,               // segment number
   ULONG     ulSegNum                     // segment number
)
{
   PTBSEGMENT pTBStartSrc;                // pointer to segment
   PTBSEGMENT pTBJoinedSrc;               // segment to be joined
   PTBSEGMENT pTBStartTgt;                // pointer to segment
   PTBSEGMENT pTBJoinedTgt;               // segment to be joined
   PSZ_W      pDataSrc;                   // pointer to data
   PSZ_W      pDataTgt = NULL;            // pointer to data
   USHORT     usLength;                   // length of the segment
   USHORT     usJoinLength;               // length of to be joined segment
   USHORT     usTgtLength;                // length of the tgt.segment
   USHORT     usTgtJoinLength;            // length of to be joined tgt segment
   BOOL       fOK;                        // success indicator
   ULONG      ulSrcWords = 0L;
   ULONG      ulSrcMarkUp = 0L;
   SHORT      sRc;
   PDOCUMENT_IDA  pIdaDoc;

                                          // join the data in the source file
   pTBStartSrc  = EQFBGetSegW( pDoc->twin, ulSegNumStart );
   pTBJoinedSrc = EQFBGetSegW( pDoc->twin, ulSegNum );
   pTBStartTgt  = EQFBGetSegW( pDoc, ulSegNumStart );
   pTBJoinedTgt = EQFBGetSegW( pDoc, ulSegNum );
   usJoinLength = (USHORT)UTF16strlenCHAR(pTBJoinedSrc->pDataW);
   usLength     = (USHORT)UTF16strlenCHAR(pTBStartSrc->pDataW) + usJoinLength + 1;

   usTgtJoinLength = (USHORT)UTF16strlenCHAR(pTBJoinedTgt->pDataW);
   usTgtLength     = (USHORT)UTF16strlenCHAR(pTBStartTgt->pDataW) + usTgtJoinLength + 1;

   fOK = UtlAlloc( (PVOID *) &pDataSrc, 0L,
                   (LONG) (max( usLength, MIN_ALLOC)) * sizeof(CHAR_W), ERROR_STORAGE);
   if ( fOK )
   {
      fOK = UtlAlloc((PVOID *) &pDataTgt, 0L,
                      (LONG) (max( usTgtLength, MIN_ALLOC )) * sizeof(CHAR_W) , ERROR_STORAGE);
   } /* endif */

   if ( fOK )
   {
      UTF16strcpy( pDataSrc,pTBStartSrc->pDataW);  // get source data for seg start
      UTF16strcat( pDataSrc,pTBJoinedSrc->pDataW); // ...  for seg joined

      UTF16strcpy( pDataTgt,pTBStartTgt->pDataW);  // get target data for seg start
      UTF16strcat( pDataTgt,pTBJoinedTgt->pDataW); // ...  for seg joined

                                          // free the old start seg
      UtlAlloc((PVOID *) &(pTBStartSrc->pDataW), 0L, 0L, NOMSG);
      pTBStartSrc->pDataW = pDataSrc;

      UtlAlloc((PVOID *) &(pTBStartTgt->pDataW), 0L, 0L, NOMSG);
      pTBStartTgt->pDataW = pDataTgt;
                                          // update the segm. identifications
      pTBStartSrc->SegFlags.JoinStart = TRUE;
      pTBJoinedSrc->SegFlags.Joined = TRUE;

      pTBStartTgt->SegFlags.JoinStart = TRUE;
      pTBJoinedTgt->SegFlags.Joined = TRUE;
                                          // reset the status info of the segm.
///      pTBStartTgt->qStatus = pTBStartSrc->qStatus;
      pTBStartTgt->qStatus = QF_TOBE;
      pTBJoinedTgt->qStatus = pTBJoinedSrc->qStatus;

      // GQ: reset count flags as segment is not translated yet
      pTBStartTgt->CountFlag.AnalAutoSubst = FALSE;
      pTBStartTgt->CountFlag.EditAutoSubst = FALSE;

                                          // or with mark identification
      pTBStartTgt->SegFlags.Marked |= pTBJoinedTgt->SegFlags.Marked;


      EQFBCompSeg( pTBStartTgt );         // reparse source and tgt segment
      EQFBCompSeg( pTBStartSrc );


      pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
      sRc = EQFBWordCntPerSeg(
                        (PLOADEDTABLE)pDoc->pDocTagTable,
                        (PTOKENENTRY) pDoc->pTokBuf,
                        pTBStartSrc->pDataW,
                        pIdaDoc->sSrcLanguage,
                        &ulSrcWords, &ulSrcMarkUp, pDoc->twin->ulOemCodePage);

      pTBStartTgt->usSrcWords = (USHORT) ulSrcWords;

      pDoc->flags.changed = TRUE;         // target document changed..
      pDoc->twin->flags.changed = TRUE;   // source document changed..

      // check if the tobe joined segment was already joined
      // if so split the sources and reset the status JoinStart

      if ( pTBJoinedSrc->SegFlags.JoinStart ) // split the prev. joined segs
      {
         do                                  // at least one joined seg avail
         {
            ulSegNum++;                      // point to next segment
            pTBStartSrc  = EQFBGetSegW( pDoc->twin, ulSegNum );
            if ( pTBStartSrc && pTBStartSrc->SegFlags.Joined )
            {
               usJoinLength = (USHORT)(usJoinLength
                                - UTF16strlenCHAR( pTBStartSrc->pDataW ));
               pTBStartTgt  = EQFBGetSegW( pDoc, ulSegNum );
               usTgtJoinLength = (USHORT)(usTgtJoinLength
                                  - UTF16strlenCHAR( pTBStartTgt->pDataW ));
            } /* endif */
         } while ( pTBStartSrc && pTBStartSrc->SegFlags.Joined ); /* enddo */

         *(pTBJoinedSrc->pDataW + usJoinLength) = EOS ;
         *(pTBJoinedTgt->pDataW + usTgtJoinLength) = EOS ;
         pTBJoinedSrc->SegFlags.JoinStart = FALSE;
         pTBJoinedTgt->SegFlags.JoinStart = FALSE;
         EQFBCompSeg( pTBJoinedSrc );        // reparse source segment
         EQFBCompSeg( pTBJoinedTgt );        // reparse source segment
         // set modified flag (just in case something was changed)...
         pTBJoinedTgt->SegFlags.Typed  = TRUE ;
         pTBStartTgt->SegFlags.Typed  = TRUE ;
      } /* endif */
   } /* endif */

   return ( fOK );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBSplitSegData - split joined segments
//------------------------------------------------------------------------------
// Function call:     EQFBSplitSegData( PTBDOCUMENT, ULONG, ULONG,
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
//                    After Split:        
//                      SegNum   Joined data         Flag
//                        Seg1   Seg1+Seg2         JoinStart
//                        Seg2   Seg2              Joined
//                        Seg3   Seg3                 -
//
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
//                        delete the segment from the TM
//                        indicate that file is now untranslated
//                        reset the status flags for last attached
//                        (joined) segment
//                        copy the changed data into source and
//                         target segment
//                        reparse the source and target segment
//                        indicate that source and target document
//                         are changed and update the segment status
//                      endif
//
//------------------------------------------------------------------------------

static BOOL
EQFBSplitSegData
(
   PTBDOCUMENT pDoc,                      // pointer to Document ida
   ULONG       ulSegNum,                  // segment number
   ULONG       ulLastSegJoined,           // segment number of last joined segm
   USHORT      usLenSeg1Twin,             // len of new seg 1 twin
   USHORT      usLenSeg2Twin              // len of remaining joined twin segs
)
{
   PTBSEGMENT pTBStartSrc;                // pointer to segment
   PTBSEGMENT pTBStartTgt;                // pointer to segment
   PTBSEGMENT pTBStartSrc2;               // pointer to joined segment
   PTBSEGMENT pTBStartTgt2;               // pointer to joined segment
   PSZ_W      pDataSrc;                   // pointer to data
   PSZ_W      pDataSrc2;                  // pointer to data
   PSZ_W      pDataTgt;                   // pointer to data
   PSZ_W      pDataTgt2;                  // pointer to data
   BOOL       fOK = TRUE;                 // success indicator
   PSTEQFGEN  pstEQFGen;                  // pointer to generic structure
   USHORT     usLenSeg1TwinW;
   USHORT     usLenSeg2TwinW;
   USHORT     usMinAllocW;
   ULONG      ulEndSeg;                   // last segment of joined segment list

   // GQ 2015/03/13: New approach split segments from the end of the list


                                          // split the data in the source file
   pTBStartSrc = EQFBGetSegW( pDoc->twin, ulSegNum );
   pTBStartTgt = EQFBGetSegW( pDoc, ulSegNum );
   pTBStartSrc2 = EQFBGetSegW( pDoc->twin, ulLastSegJoined );
   pTBStartTgt2 = EQFBGetSegW( pDoc, ulLastSegJoined );

   usMinAllocW = MIN_ALLOC * sizeof(CHAR_W);
   usLenSeg1TwinW = (usLenSeg1Twin + 1) * sizeof(CHAR_W);
   usLenSeg2TwinW = (usLenSeg2Twin + 1) * sizeof(CHAR_W);

   if ( UtlAlloc((PVOID *) &pDataSrc2, 0L,
                  (LONG) max((usLenSeg2TwinW), usMinAllocW), ERROR_STORAGE)
     && UtlAlloc((PVOID *) &pDataSrc, 0L,
                  (LONG) max((usLenSeg1TwinW), usMinAllocW), ERROR_STORAGE)
     && UtlAlloc((PVOID *) &pDataTgt2, 0L,
                  (LONG) max((usLenSeg2TwinW), usMinAllocW), ERROR_STORAGE)
     && UtlAlloc((PVOID *) &pDataTgt, 0L,
                  (LONG) max((usLenSeg1TwinW), usMinAllocW), ERROR_STORAGE) )
   {
      EQFBDelSeg(pDoc, ulSegNum);           //del seg in the memory
      pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
      WinPostMsg( pstEQFGen->hwndTWBS, EQFM_DOC_IS_ULATED, NULL, NULL);

      memcpy( pDataSrc2,pTBStartSrc->pDataW+usLenSeg1Twin, usLenSeg2TwinW);
      *(pDataSrc2+usLenSeg2Twin) = EOS;
      memcpy( pDataTgt2,pDataSrc2, usLenSeg2TwinW+1);

      memcpy( pDataSrc,pTBStartSrc->pDataW, usLenSeg1TwinW);
      *(pDataSrc+usLenSeg1Twin) = EOS;
      memcpy( pDataTgt,pDataSrc, usLenSeg1TwinW+1);

      UtlAlloc((PVOID *) &(pTBStartSrc2->pDataW), 0L, 0L, NOMSG);
      pTBStartSrc2->pDataW = pDataSrc2;
      UtlAlloc((PVOID *) &(pTBStartTgt2->pDataW), 0L, 0L, NOMSG);
      pTBStartTgt2->pDataW = pDataTgt2;


      UtlAlloc((PVOID *) &(pTBStartSrc->pDataW), 0L, 0L, NOMSG);
      pTBStartSrc->pDataW = pDataSrc;
      UtlAlloc((PVOID *) &(pTBStartTgt->pDataW), 0L, 0L, NOMSG);
      pTBStartTgt->pDataW = pDataTgt;

      // if the last segment of the joined segment list is splitted, remove joined segments indicator completely
      if ( ulLastSegJoined == (ulSegNum + 1) )
      {
        pTBStartSrc->SegFlags.JoinStart = FALSE;
        pTBStartTgt->SegFlags.JoinStart = FALSE;
      }

      pTBStartSrc2->SegFlags.Joined = FALSE;
      pTBStartTgt2->SegFlags.Joined = FALSE;

      /****************************************************************/
      /* set status info back                                         */
      /****************************************************************/
      pTBStartTgt->qStatus = pTBStartSrc->qStatus;

      pTBStartTgt->SegFlags.Current = TRUE;

      EQFBCompSeg( pTBStartTgt );         // reparse source and tgt segment
      EQFBCompSeg( pTBStartSrc );

      EQFBCompSeg( pTBStartTgt2 );
      EQFBCompSeg( pTBStartSrc2 );

      // count source words of splitted segment
      {
        ULONG ulSrcWords  = 0L;
        ULONG ulSrcMarkUp = 0L;
        USHORT usRc;
        PDOCUMENT_IDA  pIdaDoc;

        pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
        usRc = EQFBWordCntPerSeg(
                        (PLOADEDTABLE)pDoc->pDocTagTable,
                        (PTOKENENTRY) pDoc->pTokBuf,
                        pTBStartSrc->pDataW,
                        pIdaDoc->sSrcLanguage,
                        &ulSrcWords, &ulSrcMarkUp, pDoc->twin->ulOemCodePage);

        if (!usRc)
        {
          pTBStartTgt->usSrcWords = (USHORT) ulSrcWords;
        } /* endif */
      }
      pTBStartTgt->usTgtWords = 0;

      pTBStartTgt2->SegFlags.Typed  = FALSE ;
      pTBStartTgt->SegFlags.Typed  = FALSE ;

      pDoc->flags.changed = TRUE;         // target document changed..
      pDoc->twin->flags.changed = TRUE;   // source document changed..

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
// Function name:     EQFBTUnTrans - untranslate a prev. translated segment
//------------------------------------------------------------------------------
// Function call:     EQFBTUnTrans( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:         delete the segment in the TM and copy the
//                      original into the segment and look it up again
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     - delete the segment in the TM (if an exact match exist )
//                    - copy the original segment
//                    - look it up again (w/o additional dictionary lookup)
//
//------------------------------------------------------------------------------

VOID
EQFBTUnTrans
(
   PTBDOCUMENT pDoc                       // pointer to Document ida
)
{
   PTBSEGMENT pSeg;                       // pointer to segment
   USHORT     usRc;                       // return code
   USHORT     usMatchFound;               // found match
   PSZ_W      pData;                      // pointer to data
   PSTEQFGEN  pstEQFGen;                  // pointer to generic structure
   USHORT     usWorkLen;                  // length of work segment

   if ( pDoc->tbActSeg.qStatus != QF_XLATED )
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING );  // nothing to do (not translated)
   }
   else
   {
     PSZ_W pszContext;

      //  delete the segment in the TM (if already translated)
      EQFBDelSeg( pDoc, pDoc->ulWorkSeg) ;
      // and mark that document is not translated any more
      pDoc->fXlated = FALSE;
      pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
      WinPostMsg( pstEQFGen->hwndTWBS, EQFM_DOC_IS_ULATED, NULL, NULL);

      //  copy the original segment
      pSeg = EQFBGetSegW( pDoc->twin, pDoc->ulWorkSeg );
      UTF16strcpy(pDoc->pEQFBWorkSegmentW, pSeg->pDataW);       // copy segment
      EQFBCompSeg( pDoc->pTBSeg );                        // force a recompute

      pDoc->TBCursor.usSegOffset = 0;                   // set segment offset
      EQFBGotoSeg( pDoc, pDoc->ulWorkSeg, 0);
      pDoc->Redraw |= REDRAW_ALL;                       // redraw the screen
//    pDoc->EQFBFlags.workchng = TRUE;                  // change to workseg
      pDoc->EQFBFlags.workchng = FALSE;                 // no change to workseg
      pDoc->flags.changed = TRUE;                       // target doc changed..

      // get rid of translated indication - set attribute of source sentence
      pDoc->tbActSeg.qStatus = pSeg->qStatus;
      pDoc->pTBSeg->SegFlags.UnTrans = TRUE;
      pDoc->tbActSeg.SegFlags.UnTrans = TRUE;
      pDoc->pTBSeg->SegFlags.Typed = FALSE;
      pDoc->pTBSeg->SegFlags.Copied = FALSE;
      pDoc->pTBSeg->usTgtWords = 0;
      pDoc->tbActSeg.usTgtWords = 0;

      // clear meta data buffer
      pDoc->szMetaData[0] = 0;

      MTLogUndoProposalCopy( pDoc );
	  
      // reset existing proposal flags in MT log
      memset( &(pDoc->ActSegLog.PropTypeExists), 0, sizeof(pDoc->ActSegLog.PropTypeExists) );

      //  look it up again (w/o additional dictionary lookup)
      pszContext = EQFBGetContext( pDoc, pSeg, pSeg->ulSegNum );
      usRc = EQFTRANSSEG3W(pSeg->pDataW, pszContext, pSeg->pvMetadata, // pointer to seg data
                           pSeg->ulSegNum,  // segment number
                           TRUE,                    // mode of operation
                           EQFF_NOAUTODICT,
                           (PSHORT)&usMatchFound);

      // refill proposal match flags
      if ( usRc == EQFRC_OK ) EQFBGetMatchFlags( pDoc );

     if (usRc != EQFRC_OK )
     {
        PSZ pErr = EQFERRINS();           // get error message
        UtlError( EQFERRID(), MB_CANCEL, 1, &pErr, EQF_ERROR );
     }
     else
     {
       /***************************************************************/
       /* copy original into savesegment and free old contents...     */
       /***************************************************************/
       usWorkLen = (USHORT)max( MIN_ALLOC,
                        UTF16strlenBYTE( pSeg->pDataW )+ sizeof(CHAR_W));

       usRc = (USHORT)!UtlAlloc((PVOID *) &pData, 0L, (LONG) usWorkLen, TRUE );
       if ( pData )
       {
         UtlAlloc((PVOID *) &pDoc->pSaveSegW, 0L, 0L, FALSE );
         pDoc->pSaveSegW = pData;
         UTF16strcpy( pDoc->pSaveSegW, pSeg->pDataW );
       } /* endif */

	   // in case of richedit: enforce set segment
	   if (pDoc->hwndRichEdit)
	   {
	     EQFBSetWorkSegRTF( pDoc, pDoc->ulWorkSeg, pDoc->pEQFBWorkSegmentW );
	   }
     } /* endif */
   } /* endif */

   return;
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFindScratch - find next seg. transl. from scratch
//------------------------------------------------------------------------------
// Function call:     EQFBFindScratch (PTBDOCUMENT);
//
//------------------------------------------------------------------------------
// Description:        This function will find the next segment which is
//                     translated from scratch
//
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:      -- start at the current segment and scan forward
//                        until the next segment with s=1 is found
//                        (SegFlags.Typed = TRUE and SegFlags.Copied = FALSE)
//                     -- if none is available wrap around and scan until
//                        the position you left
//                     -- if none found then
//                         issue warning and stay with the cursor
//                        else
//                         goto found  segment
//
//------------------------------------------------------------------------------
VOID
EQFBFindScratch
(
  PTBDOCUMENT pDoc                      // pointer to document ida
)
{
   PTBSEGMENT  pSeg;                      // pointer to segment
   PTBSEGMENT  pSegStart;                 // pointer to start segment
   ULONG       ulSegNum;                  // segment number

   ulSegNum = pDoc->TBCursor.ulSegNum + 1; // start search at next segment
   pSeg = EQFBGetVisSeg(pDoc, &ulSegNum); // get segm
   pSegStart = pSeg;
   while ( pSeg && ! (pSeg->SegFlags.Typed && !pSeg->SegFlags.Copied) )
   {
      ulSegNum++;                         // point to next segment
      pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
   } /* endwhile */

   // not found - end of table reached
   if ( !pSeg || !(pSeg->SegFlags.Typed && !pSeg->SegFlags.Copied) )
   {
     ulSegNum = 1;
     pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);   // reset to begin again
     while ( pSeg && (pSeg != pSegStart) &&
              ! (pSeg->SegFlags.Typed && !pSeg->SegFlags.Copied))
     {
        ulSegNum++;                       // point to next segment
        pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
     } /* endwhile */
   } /* endif */

   if ( pSeg && (pSeg->SegFlags.Typed && !pSeg->SegFlags.Copied) )                 // start point found
   {
      if ( pSeg->ulSegNum == pDoc->tbActSeg.ulSegNum )
      {
         EQFBGotoActSegment( pDoc );
      }
      else
      {
         pDoc->TBCursor.ulSegNum = pSeg->ulSegNum; // set segment number
         pDoc->TBCursor.usSegOffset = 0;           // set segment offset
         EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum, 0);
      } /* endif */
   }
   else                                         // No segment available
   {
     /*****************************************************************/
     /* No segment available which is translated from scratch         */
     /*****************************************************************/
      UtlError( TB_NOSEGSCRATCH, MB_CANCEL, 0, NULL, EQF_WARNING);
   } /* endif */
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFindCopy    - find next seg. which is prop.copy
//------------------------------------------------------------------------------
// Function call:     EQFBFindCopy    (PTBDOCUMENT);
//
//------------------------------------------------------------------------------
// Description:        This function will find the next segment which is
//                     as copy of a found proposal
//
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:      -- start at the current segment and scan forward
//                        until the next segment with s=3 is found
//                        (SegFlags.Typed = FALSE &  SegFlags.Copied = TRUE )
//                     -- if none is available wrap around and scan until
//                        the position you left
//                     -- if none found then
//                         issue warning and stay with the cursor
//                        else
//                         goto found  segment
//
//------------------------------------------------------------------------------
VOID
EQFBFindCopy
(
  PTBDOCUMENT pDoc                      // pointer to document ida
)
{
   PTBSEGMENT  pSeg;                      // pointer to segment
   PTBSEGMENT  pSegStart;                 // pointer to start segment
   ULONG       ulSegNum;                  // segment number

   ulSegNum = pDoc->TBCursor.ulSegNum + 1; // start search at next segment
   pSeg = EQFBGetVisSeg(pDoc, &ulSegNum); // get segm
   pSegStart = pSeg;
   while ( pSeg && ! (!pSeg->SegFlags.Typed && pSeg->SegFlags.Copied) )
   {
      ulSegNum++;                         // point to next segment
      pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
   } /* endwhile */

   // not found - end of table reached
   if ( !pSeg || !(!pSeg->SegFlags.Typed && pSeg->SegFlags.Copied) )
   {
     ulSegNum = 1;
     pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);   // reset to begin again
     while ( pSeg && (pSeg != pSegStart) &&
              ! (!pSeg->SegFlags.Typed && pSeg->SegFlags.Copied))
     {
        ulSegNum++;                       // point to next segment
        pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
     } /* endwhile */
   } /* endif */

   if ( pSeg && (!pSeg->SegFlags.Typed && pSeg->SegFlags.Copied) )                 // start point found
   {
      if ( pSeg->ulSegNum == pDoc->tbActSeg.ulSegNum )
      {
         EQFBGotoActSegment( pDoc );
      }
      else
      {
         pDoc->TBCursor.ulSegNum = pSeg->ulSegNum; // set segment number
         pDoc->TBCursor.usSegOffset = 0;           // set segment offset
         EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum, 0);
      } /* endif */
   }
   else                                         // No segment available
   {
     /*****************************************************************/
     /* No segment available which is translated from scratch         */
     /*****************************************************************/
      UtlError( TB_NOSEGSCRATCH, MB_CANCEL, 0, NULL, EQF_WARNING);
   } /* endif */
}
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFindCpyTyped- find next seg. copied & typed in
//------------------------------------------------------------------------------
// Function call:     EQFBFindCpyTyped (PTBDOCUMENT);
//
//------------------------------------------------------------------------------
// Description:        This function will find the next segment which is
//                     a copy of a proposal and typed in something
//
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:      -- start at the current segment and scan forward
//                        until the next segment with s=2 is found
//                        (SegFlags.Typed = TRUE and SegFlags.Copied = TRUE )
//                     -- if none is available wrap around and scan until
//                        the position you left
//                     -- if none found then
//                         issue warning and stay with the cursor
//                        else
//                         goto found  segment
//
//------------------------------------------------------------------------------
VOID
EQFBFindCpyTyped
(
  PTBDOCUMENT pDoc                      // pointer to document ida
)
{
   PTBSEGMENT  pSeg;                      // pointer to segment
   PTBSEGMENT  pSegStart;                 // pointer to start segment
   ULONG       ulSegNum;                  // segment number

   ulSegNum = pDoc->TBCursor.ulSegNum + 1; // start search at next segment
   pSeg = EQFBGetVisSeg(pDoc, &ulSegNum); // get segm
   pSegStart = pSeg;
   while ( pSeg && ! (pSeg->SegFlags.Typed && pSeg->SegFlags.Copied) )
   {
      ulSegNum++;                         // point to next segment
      pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
   } /* endwhile */

   // not found - end of table reached
   if ( !pSeg || !(pSeg->SegFlags.Typed && pSeg->SegFlags.Copied) )
   {
     ulSegNum = 1;
     pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);   // reset to begin again
     while ( pSeg && (pSeg != pSegStart) &&
              ! (pSeg->SegFlags.Typed && pSeg->SegFlags.Copied))
     {
        ulSegNum++;                       // point to next segment
        pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
     } /* endwhile */
   } /* endif */

   if ( pSeg && (pSeg->SegFlags.Typed && pSeg->SegFlags.Copied) )                 // start point found
   {
      if ( pSeg->ulSegNum == pDoc->tbActSeg.ulSegNum )
      {
         EQFBGotoActSegment( pDoc );
      }
      else
      {
         pDoc->TBCursor.ulSegNum = pSeg->ulSegNum; // set segment number
         pDoc->TBCursor.usSegOffset = 0;           // set segment offset
         EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum, 0);
      } /* endif */
   }
   else                                         // No segment available
   {
     /*****************************************************************/
     /* No segment available which is translated from scratch         */
     /*****************************************************************/
      UtlError( TB_NOSEGSCRATCH, MB_CANCEL, 0, NULL, EQF_WARNING);
   } /* endif */
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBAutoTranslate
//------------------------------------------------------------------------------
// Function call:     EQFBAutoTranslate( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       do an automatic translation starting at
//                    the current cursor position
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT      document ida
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     - store current active segment number
//                    - set flag whether user requested w/o stop
//                    - if postedit mode or already automatic started
//                          issue a beep
//                          set fOK = FALSE
//                      else if current segment is not the active segment
//                           if automatic stop
//                              issue error message, stop request (fOK = FALSE)
//                           else set cursor to active segment
//                    - if OK so far
//                        if Save Seg exists
//                          if nothing changed and no Fuzzy match copied
//                             try to get exact match
//                             if found
//                               save segment into TM with exact match
//                             else
//                               if automatic stop
//                                 issue warning, stop auto.translation
//                               else
//                                 reset fOK, reset old segment status
//                                 write worksegment out
//                               endif
//                             endif
//                          else
//                            save segment
//                          endif
//                        endif
//                      endif
//                      if OK so far
//                        set automatic mode and call EQFBFuncAutoTrans
//
//------------------------------------------------------------------------------
VOID EQFBAutoTranslate
(
   PTBDOCUMENT  pDoc                            // pointer to doc. instance
)
{
   BOOL       fOK = TRUE;                       // success indicator
   ULONG      ulSegNum;                          // segment number
   PTBSEGMENT pSeg;
   BOOL       fPropMode = pDoc->pUserSettings->fInsProposal;
   USHORT     usRc;
   USHORT     usMatchFound;            // number of matches found


   pDoc->ulAutoSegNum = (pDoc->tbActSeg).ulSegNum;  // nec for stop if !fAutoStop

   ulSegNum = (pDoc->TBCursor).ulSegNum;         // get segment number
   if ( pDoc->EQFBFlags.AutoMode || pDoc->EQFBFlags.PostEdit)
   {
      EQFBFuncNothing( pDoc );                   // issue a beep
      fOK = FALSE;                               // do not proceed with autotr.
   }
   else if ( ulSegNum != pDoc->tbActSeg.ulSegNum )
   {
     if ( pDoc->pUserSettings->fAutoStop )
     {
       UtlError( TB_NOTINACTSEG, MB_CANCEL, 0, NULL, EQF_WARNING);
       fOK = FALSE;                               // do not proceed with autotr.
     }
     else
     {
       /***************************************************************/
       /* set cursor into active segment and then start               */
       /***************************************************************/
       ulSegNum = pDoc->tbActSeg.ulSegNum;
       EQFBGotoSeg( pDoc, ulSegNum, 0);
     } /* endif */
   } /* endif */

   if ( fOK )
   {
	 if (pDoc->fAutoSpellCheck)
	 {
	   USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
	   EQFBFuncSpellAuto(pDoc);   // turn off auto-spellcheck
	   //remember original setting to restore it at end of edit-auto-subst
	   pEQFBUserOpt->UserOptFlags.bAutoSpellCheck = TRUE;
     }
     pDoc->pUserSettings->fInsProposal= FALSE;  // no cursor insert in auto
     if ( pDoc->pSaveSegW )                        // segment active
     {
                                                  // check if something changed
        if ( ! (pDoc->EQFBFlags.workchng || pDoc->fFuzzyCopied))
        {
           PSZ_W pszContext;
           USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
           // try to get exact match
           pSeg = EQFBGetSegW(pDoc,ulSegNum);
           pszContext = EQFBGetContext( pDoc, pSeg, ulSegNum );
           usRc = EQFTRANSSEG3W( pSeg->pDataW, pszContext, pSeg->pvMetadata,
                                ulSegNum,
                                TRUE,           // foreground translation
                                               // automatic mode, i.e. no dict
                               (USHORT)((pEQFBUserOpt->fExactContextTMMatch ? EQFF_EXACTCONTEXT : 0) |
                                        (pEQFBUserOpt->fUseLatestMatch ? 0 : EQFF_ONLYONEEXACT)),
                               (PSHORT)&usMatchFound);

           if ( usRc == EQFRC_OK )
           {
            USHORT usState = EqfGetPropState((PSTEQFGEN)pDoc->pstEQFGen, 1 );
 
            if ( (usState & GLOBMEM_TRANS_PROP) || 
                 (usState & GLOBMEMSTAR_TRANS_PROP) ||
                 (usState & MACHINE_TRANS_PROP ) )
            {
              // ignore machine translation proposals or global memory proposals
              fOK = FALSE;
            }
            else
            {
              // copy the proposal if good enough
              fOK = EQFBCopyPropMatch( pDoc, EXACT_MATCH, EQUAL_EQUAL, FALSE, TRUE );
            }

             if (pDoc->hwndRichEdit )
             {
               USHORT  usOffset = 0;

               EQFBSetWorkSegRTF( pDoc, ulSegNum, pDoc->pEQFBWorkSegmentW );
               EQFBGotoSeg( pDoc, ulSegNum, usOffset );
             } /* endif */
             if ( fOK )
             {                    // save the segment into transl. memory
               fOK = EQFBSaveSeg( pDoc );                  /* @KIT948M */
               if ( fOK )                                  /* @KIT948A */
               {                                           /* @KIT948A */
                 pSeg->SegFlags.Copied = TRUE;             /* @KIT948M */
                 pSeg->SegFlags.Typed = FALSE;             /* @KIT948M */
                 pSeg->CountFlag.PropChanged = FALSE;
                 pSeg->CountFlag.EditAutoSubst = TRUE;
                 pSeg->usModWords = 0;

                 memset(&(pDoc->ActSegLog.PropTypeCopied), 0, sizeof( pDoc->ActSegLog.PropTypeCopied) );
                 pDoc->ActSegLog.PropTypeCopied.Exact = TRUE;    /* @KIT948M */

               }                                           /* @KIT948A */
               else                                        /* @KIT948A */
               {                                           /* @KIT948A */
                 EQFBFuncAutoStop( pDoc );                 /* @KIT948A */
                 UtlError( TB_AUTOSTOP, MB_OK,             /* @KIT948A */
                           0, NULL, EQF_WARNING);          /* @KIT948A */
               } /* endif */                               /* @KIT948A */
             }
             else
             {
               if ( pDoc->pUserSettings->fAutoStop)
               {
                  EQFBFuncAutoStop( pDoc );
                  UtlError( TB_AUTOSTOP, MB_OK, 0, NULL, EQF_WARNING);
               }
               else          //continue if Automatic without stop
               {
                 fOK = TRUE;                // reset indication
                                            // reset old status
                 pSeg->qStatus = pDoc->tbActSeg.qStatus;
                 pSeg->SegFlags.Current = FALSE;
                 pSeg->usModWords = 0;
                 EQFBWorkSegOut( pDoc );    // make space for next segment...
               } /* endif */
             } /* endif */
           }
           else
           {
             EQFBFuncAutoStop( pDoc );
             fOK = FALSE;
             UtlError( TB_AUTOSTOP, MB_OK, 0, NULL, EQF_WARNING);
           } /* endif */
        }
        else       // we changed the segment -- save it first than start trans
        {
           fOK = EQFBSaveSeg( pDoc );
        } /* endif */
     } /* endif */

     pDoc->pUserSettings->fInsProposal = (EQF_BOOL)fPropMode;
   } /* endif */


   if ( fOK )
   {
      pDoc->fXlated = FALSE;                     // set translated back
      pDoc->EQFBFlags.AutoMode  = TRUE;          // in automatic mode
      EQFBFuncAutoTrans( pDoc );                 // issue call to auto trans
   } /* endif */
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncAutoTrans
//------------------------------------------------------------------------------
// Function call:     EQFBFuncAutoTrans( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       do an automatic translation with the
//                    current segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT      document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:      Posts message to call it recursively
//
//------------------------------------------------------------------------------
// Function flow:      if not AutoTrans mode on then
//                       skip the rest
//                     else
//                       - if automatic stop
//                            find next untranslated segment
//                            (EQFBFindNExtSource)
//                         else
//                            find next untranslated segment
//                            (EQFBFindNextAutoSource)
//                       - if available then
//                           send it in foreground mode to services
//                          -if RC == exact match found then
//                              paste it into TARGET_DOC,
//                              save it in translation memory
//                              set SegFlags
//                           else
//                              if automatic stop
//                                EQFBFuncAutoSTop
//                                inform user that no exact match was found
//                              else
//                                reset fOK to TRUE
//                                set flag that not all segments are translated
//                                get old status and write worksegment out
//                              endif
//                           endif
//                         else
//                           reset autoMOde
//                           if Automatic stop
//                             Doc is translated (EQFBDocIsTranslated)
//                           else
//                             check whether document is translated
//                             if not all translated
//                               activate 'old' active segment
//                             else
//                               Doc is Translated (EQFBDocIsTranslated)
//                             endif
//                           endif
//                         endif
//                         if (fOK so far)
//                         - do cursor update
//                         - allow other users to process
//                         - post message to proceed with next segment
//                         endif
//                     endif
//------------------------------------------------------------------------------
VOID EQFBFuncAutoTrans
(
   PTBDOCUMENT  pDoc                            // pointer to doc. instance
)
{
   BOOL       fOK = TRUE;                       // success indicator
   USHORT     usRc;                             // return value
   ULONG      ulSegNum;                         // segment number
   PTBSEGMENT pSeg;                             // pointer to source segment
   USHORT  usMatchFound;                         // number of Matches found
   BOOL    fEndReached;                          //TRUE if EOF reached
   HWND    hwndTemp;
   BOOL    fPropMode = pDoc->pUserSettings->fInsProposal;
   PVOID     pvMetaData;

   if ( pDoc->EQFBFlags.AutoMode )
   {
      pDoc->pUserSettings->fInsProposal= FALSE;  // no cursor insert in auto
      if (pDoc->fAutoSpellCheck)
	  {
		   USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
	  	   EQFBFuncSpellAuto(pDoc);   // turn off auto-spellcheck
	  	   //remember original setting to restore it at end of edit-auto-subst
	  	   pEQFBUserOpt->UserOptFlags.bAutoSpellCheck = TRUE;
      }
      ulSegNum = (pDoc->TBCursor).ulSegNum;      // get segment number
      if ( pDoc->pUserSettings->fAutoStop )
      {
        pSeg = EQFBFindNextSource( pDoc, &ulSegNum, POS_TOBE,&fEndReached, TRUE, &pvMetaData );
      }
      else
      {
        pSeg = EQFBFindNextAutoSource( pDoc, &ulSegNum, &pvMetaData );
      } /* endif */
      /****************************************************************/
      /* get reference to target document segment and all its settings*/
      /****************************************************************/
      if ( pSeg )
      {
        pSeg = EQFBGetSegW(pDoc,ulSegNum);
        fOK = (pSeg != NULL);
      }
      else
      {
        fOK = FALSE;
      } /* endif */
      if ( fOK )
      {
         PSZ_W pszContext;
		 USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
         EQFBActivateSegm( pDoc, ulSegNum);  // activate the current segment
         EQFBScreenData( pDoc );          // force screen update

#ifdef TRANTIME
         ulStart = pGlobInfoSeg->msecs;
#endif
         pszContext = EQFBGetContext( pDoc, pSeg, pSeg->ulSegNum );
         usRc = EQFTRANSSEG3W( pSeg->pDataW, pszContext, pvMetaData,
                            ulSegNum,
                            TRUE,           // foreground translation
                                            // automatic mode, i.e. no dict
                            (USHORT)(EQFF_NODICTWND | EQFF_NOAUTODICT | EQFF_NOPROPWND |
                                     (pEQFBUserOpt->fExactContextTMMatch ? EQFF_EXACTCONTEXT : 0) |
                                     (pEQFBUserOpt->fUseLatestMatch ? 0 : EQFF_ONLYONEEXACT)),
                            (PSHORT)&usMatchFound);

#ifdef TRANTIME
      ulEnd = pGlobInfoSeg->msecs;
      ulTime += (ulEnd - ulStart);
#endif

         if ( usRc == EQFRC_OK )
         {
            USHORT usState = EqfGetPropState((PSTEQFGEN)pDoc->pstEQFGen, 1 );
 
            if ( (usState & GLOBMEM_TRANS_PROP) || 
                 (usState & GLOBMEMSTAR_TRANS_PROP) ||
                 (usState & MACHINE_TRANS_PROP ) )
            {
              // ignore machine translation proposals or global memory proposals
              fOK = FALSE;
            }
            else
            {
              // copy the proposal if good enough
              fOK = EQFBCopyPropMatch( pDoc, EXACT_MATCH, EQUAL_EQUAL, FALSE, TRUE );
            }

            /**********************************************************/
            /* reset flagging which props exist and copied;           */
            /* this is only interesting if not in autosubst           */
            /**********************************************************/
            memset(&(pSeg->CountFlag),0,sizeof(pSeg->CountFlag) );
            if ( fOK )
            {                    // save the segment into transl. memory
               fOK = EQFBSaveSeg( pDoc );                  /* @KIT948M */
               if ( fOK )                                  /* @KIT948A */
               {                                           /* @KIT948A */
                 pSeg->SegFlags.Copied = TRUE;             /* @KIT948M */
                 pSeg->SegFlags.Typed = FALSE;             /* @KIT948M */
                 pSeg->CountFlag.PropChanged = FALSE;
                 pSeg->CountFlag.EditAutoSubst = TRUE;
                 pSeg->usModWords = 0;


                 memset(&(pDoc->ActSegLog.PropTypeCopied), 0, sizeof( pDoc->ActSegLog.PropTypeCopied) );
                 pDoc->ActSegLog.PropTypeCopied.Exact = TRUE;    /* @KIT948M */

               }                                           /* @KIT948A */
               else                                        /* @KIT948A */
               {                                           /* @KIT948A */
                 EQFBFuncAutoStop( pDoc );                 /* @KIT948A */
                 UtlError( TB_AUTOSTOP, MB_OK,             /* @KIT948A */
                           0, NULL, EQF_WARNING);          /* @KIT948A */
               } /* endif */                               /* @KIT948A */
            }
            else
            {
              if ( pDoc->pUserSettings->fAutoStop )
              {
                 EQFBFuncAutoStop( pDoc );
                 UtlError( TB_AUTOSTOP, MB_OK, 0, NULL, EQF_WARNING);
              }
              else        // continue if automatic without stop
              {
                fOK = TRUE;
                                           // get old status
                pSeg->qStatus = pDoc->tbActSeg.qStatus;
                pSeg->SegFlags.Current = FALSE;
                pSeg->usModWords = 0;           //qual.of best prop not valid here
                EQFBWorkSegOut( pDoc );    // make space for next segment...
                // force redisplay of segment -- we need it in different colour
                if ( pDoc->hwndRichEdit )
                {
                  EQFBSetWorkSegRTF( pDoc, ulSegNum, pDoc->pEQFBWorkSegmentW );
                } /* endif */
              } /* endif */
            } /* endif */
         }
         else
         {
            EQFBFuncAutoStop( pDoc );
            fOK = FALSE;
            UtlError( TB_AUTOSTOP, MB_OK, 0, NULL, EQF_WARNING);
         } /* endif */
      }
      else
      {
		USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
        pDoc->EQFBFlags.AutoMode  = FALSE;       // reset auto mode
        if (pEQFBUserOpt->UserOptFlags.bAutoSpellCheck)
		{
		   EQFBFuncSpellAuto(pDoc);   // turn auto-spellcheck on
        }
        if ( pDoc->pUserSettings->fAutoStop )
        {
          EQFBDocIsTranslated( pDoc );   // document completely translated
        }
        else
        {
          /************************************************************/
          /* check whether document is translated                     */
          /************************************************************/
          ulSegNum = 1;
          pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
          while ( pSeg && pSeg->pDataW && pSeg->qStatus != QF_TOBE
                                      && pSeg->qStatus != QF_ATTR )
          {
             ulSegNum++;                            // point to next segment
             pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
          } /* endwhile */

          /************************************************************/
          /* if not all translated,activate 'old' active segment again*/
          /************************************************************/
          if ( pSeg )
          {
            EQFBGotoSeg(pDoc,pDoc->ulAutoSegNum, 0);
            ulSegNum = (pDoc->TBCursor).ulSegNum; // store start addr
            EQFBDoNextTwo ( pDoc,                 // do the next two
                          &ulSegNum , POS_CURSOR  );//
            EQFBScreenData( pDoc );          // force screen update

          }
          else
          {
             EQFBDocIsTranslated( pDoc );   // document completely translated
          } /* endif */
        } /* endif */
      } /* endif */
      /****************************************************************/
      /* reset the active cursor mode...                              */
      /****************************************************************/
      pDoc->pUserSettings->fInsProposal = (EQF_BOOL)fPropMode;

      EQFBScreenCursor( pDoc );        // position cursor and slider
      if ( fOK )
      {
         hwndTemp = pDoc->hwndFrame;
         UtlDispatch();                            // dispatch waiting messages
         pDoc = ACCESSWNDIDA( hwndTemp, PTBDOCUMENT );
         // post message to procede with Automatic Translation
         if ( pDoc )
         {
           WinPostMsg( pDoc->hwndClient, WM_EQF_AUTOTRANS, NULL, NULL );
         } /* endif */
      } /* endif */
   } /* endif */

}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncAutoStop
//------------------------------------------------------------------------------
// Function call:     EQFBFuncAutoStop( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       Do handling if automatic translation stops
//                    due to no exact match, error or user request
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT      document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - get segment and reset segment status
//                    - store old data
//                    - activate the next two segments
//
//------------------------------------------------------------------------------
VOID EQFBFuncAutoStop
(
   PTBDOCUMENT  pDoc                            // pointer to doc. instance
)
{
   ULONG      ulSegNum;                         // segment number
   PTBSEGMENT pSeg;                             // pointer to source segment
   USEROPT* pEQFBUserOpt = get_EQFBUserOpt();

   pDoc->EQFBFlags.AutoMode  = FALSE;     // reset auto mode
   // active segment number might be reset by correct EQFBSaveSeg
   // but any other info is still in tbActSeg structure
   if ( ! pDoc->tbActSeg.ulSegNum )
   {
      pDoc->tbActSeg.ulSegNum = (pDoc->TBCursor).ulSegNum;
   } /* endif */

   pSeg = EQFBGetSegW(pDoc, pDoc->tbActSeg.ulSegNum); // get seg
   pSeg->qStatus = pDoc->tbActSeg.qStatus; // reset status
   pSeg->SegFlags.Current = FALSE;

   pSeg->usModWords = 0;
   // something to restore, otherwise request to this function after correct
   // save
   if ( pDoc->pSaveSegW )
   {
      pSeg->pDataW = pDoc->pSaveSegW;
   } /* endif */
   EQFBCompSeg( pSeg );

   EQFCLEAR( 0 );                      // clear internal buffers

   pDoc->pSaveSegW = NULL;                 // reset save seg
   ulSegNum = pDoc->tbActSeg.ulSegNum;    // get current position
   EQFBDoNextTwo( pDoc,
                  &ulSegNum, TRUE); // already posit.
   if (pEQFBUserOpt->UserOptFlags.bAutoSpellCheck)
   {
	   EQFBFuncSpellAuto(pDoc);   // turn auto-spellcheck on
   }

}
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFindNextAutoSource
//------------------------------------------------------------------------------
// Function call:     EQFBFindNextAutoSource(PTBDOCUMENT, PUSHORT);
//
//------------------------------------------------------------------------------
// Description:       find the next (untranslated) segment
//                    special if automatic translation without stop
//                    stops if wrapped around and comes to usAutoSegNUm
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT -- pointer to document structure
//                    PUSHORT     -- the segment number where to start from
//
//------------------------------------------------------------------------------
// Returncode type:   PTBSEGMENT
//------------------------------------------------------------------------------
// Returncodes:       pointer to source segment structure
//                    NULL  if no matchin segment found
//------------------------------------------------------------------------------
// Side effects:      On output pulSegNum will contain the active
//                    segment number
//
//------------------------------------------------------------------------------
// Function flow:     - scan source doc and find next segment
//                      (either tobe or attribute)
//                      starting from the current position
//                    - if not found in the forward scan do a wrap around
//                       and scan until segment is found or at
//                       usAutoSegNum
//                    - get pointer to it
//                      if none is available return a NULL
//
//------------------------------------------------------------------------------

static PTBSEGMENT
EQFBFindNextAutoSource
(
  PTBDOCUMENT pDoc,     // pointer to document structure
  PULONG   pulSegNum,   // pointer to segment number
  PVOID    *ppvMetaData // ptr to callers metadata pointer
)
{
   BOOL   fFound = FALSE;                 // no match found
   PTBSEGMENT  pSeg;                      // pointer to segment
   ULONG       ulSegNum;                  // segment number


   ulSegNum = *pulSegNum;                        // store segment number temporarily
   ulSegNum ++;
   pSeg = EQFBGetVisSeg(pDoc, &ulSegNum); // get seg

//   pSegStart = pSeg;

   while ( pSeg && pSeg->pDataW && ! fFound && ulSegNum != pDoc->ulAutoSegNum)
   {
      switch ( pSeg->qStatus)
      {
        case QF_ATTR:                           // attribute
        case QF_TOBE:                           // to be translated
           fFound = TRUE;
           break;
        default :
           break;
      } /* endswitch */
      if ( !fFound)
      {
         ulSegNum++;                            // point to next segment
         pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
      } /* endif */
   } /* endwhile */
   /*******************************************************************/
   /* if already at start segment of automatic, stop                  */
   /* because no next segment is found                                */
   /*******************************************************************/
   if ( ulSegNum == pDoc->ulAutoSegNum )
   {
     pSeg = NULL;
   }
   else
   {
      if ( !fFound )                              // not found,i.e. end table
      {
         ulSegNum = 1;                              // reset to first segment
         pSeg = EQFBGetVisSeg(pDoc, &ulSegNum) ;    // get first seg
         while ( pSeg && ulSegNum != pDoc->ulAutoSegNum && ! fFound)
         {
            switch ( pSeg->qStatus)
            {
              case QF_ATTR:                           // attribute
              case QF_TOBE:                           // to be translated
                 fFound = TRUE;
                 break;
              default:
                 break;
            } /* endswitch */
            if ( !fFound)
            {
               ulSegNum++;                            // point to next segment
               pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
            } /* endif */
         } /* endwhile */
         if ( !fFound )                         // set to NULL if not found
         {
           pSeg = NULL;
         } /* endif */
      } /* endif */
   } /* endif */

   *pulSegNum = ulSegNum;          // set segnum for return

   if ( pSeg ) *ppvMetaData = pSeg->pvMetadata;

   return ( pSeg );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBEmpty
//------------------------------------------------------------------------------
// Function call:     EQFBEmpty( PTBDOCUMENT );
//------------------------------------------------------------------------------
// Description:       Delete a segment
//                    The text of the segment is replaced by the :NONE tag
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   pointer to document instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     check if in active segment (i.e. in target document,
//                    not in postedit, cursor in active segment)
//                    if yes then
//                       display warning: do you really want...?
//                       if yes then
//                          copy :NONE tag in the Worksegment
//                          in case of last segment add a LF
//                          set flags, re-calc segment length etc.
//                          call EQFBTrans(save seg & activate next
//                       else do nothing
//                       endif
//                    endif
//------------------------------------------------------------------------------
VOID
EQFBEmpty
(
   PTBDOCUMENT pDoc          // pointer to document instance
)
{
   USHORT usResult;                             //return from UtlError
   PSZ_W  pData;                                //pointer to worksegment
   PEQFBBLOCK pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;      // pointer to block struct
   BOOL       fLFfound = FALSE;                 // true if LF found in seg

   if (pDoc->EQFBFlags.PostEdit )
   {
       WinAlarm( HWND_DESKTOP, WA_WARNING );    // beep if not correct mode
   }
   else
   {
      pData = pDoc->pEQFBWorkSegmentW;
      UTF16strncpy( chSeg1, pDoc->pTBSeg->pDataW, MSGBOXDATALEN);
      chSeg1[ MSGBOXDATALEN ] = EOS;            // set end of string
      if ( UTF16strlenCHAR(chSeg1) >= MSGBOXDATALEN )
      {                                         //fill last 3 chars:ï...ï
         UTF16strcpy(chSeg1+MSGBOXDATALEN-4,L"...");
      } /* endif */
      pData = chSeg1;
      usResult = UtlErrorW( TB_SEGDELETE, MB_YESNO | MB_DEFBUTTON2, 1, &pData,
                                  EQF_QUERY, TRUE );
      if (usResult == MBID_YES)                   //yes = want to delete
      {
        /**************************************************************/
        /* check whether segment contains a LF                        */
        /**************************************************************/
        pData = pDoc->pEQFBWorkSegmentW;                          /* @KIT975A */
        while ( (*pData) && !fLFfound)                          /* @KIT975A */
        {                                                       /* @KIT975A */
          if ( *pData == LF )                                   /* @KIT975A */
          {                                                     /* @KIT975A */
            fLFfound = TRUE;                                    /* @KIT975A */
          }                                                     /* @KIT975A */
          else                                                  /* @KIT975A */
          {                                                     /* @KIT975A */
            pData ++;                                           /* @KIT975A */
          } /* endif */                                         /* @KIT975A */
        } /* endwhile */                                        /* @KIT975A */
        /**************************************************************/
        /* if no LF in seg copy empty-tag, else leave one LF in seg   */
        /**************************************************************/
        if ( fLFfound )                         // set linefeed
        {                                                       /* @KIT975A */
          pDoc->pEQFBWorkSegmentW[0] = LF;                        /* @KIT975A */
          pDoc->pEQFBWorkSegmentW[1] = EOS;                       /* @KIT975A */
        }                                                       /* @KIT975A */
        else                                    // set empty tag
        {                                                       /* @KIT975A */
           UTF16strcpy(pDoc->pEQFBWorkSegmentW,EMPTY_TAG);        /* @KIT975M */
        } /* endif */                                           /* @KIT975A */
         // in case of last segment add a LF
         /*************************************************************/
         /* this is no more nec   6 lines : @KIT975D                  */
         /*************************************************************/
//       usSeg = pDoc->ulWorkSeg + 1;
//       pSeg2 = EQFBGetVisSeg( pDoc, &usSeg );    // check if segment is visible
//       if ( !pSeg2 )
//       {
//          strcat(pDoc->EQFBWorkSegment,chLF);   // add line feed
//       } /* endif */
                                                                /* @KIT975A */
         EQFBCompSeg( pDoc->pTBSeg );             //update length of segment
         EQFBFuncStartSeg( pDoc );                // set cursor at seg.begin
         EQFBUpdateChangedSeg(pDoc);              //update segment ident.
         pDoc->pTBSeg->SegFlags.Copied = FALSE;                     /* @EAC */
         #ifdef _WINDOWS
           EQFBScreenData( pDoc );                // display screen
         #endif
         EQFBTrans(pDoc,POS_TOBEORDONE);          // save seg, activate next
         pDoc->Redraw |= REDRAW_ALL;              // indicate to update all of screen

         // redo blockmark in current segment at character input
         if ( pstBlock->pDoc == pDoc &&
               pstBlock->ulSegNum == pDoc->ulWorkSeg )
         {
            pstBlock->pDoc = NULL;                 // reset block mark
         } /* endif */
      }
      else
      {
         // do nothing
      } /* endif */
   } /* endif */
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBMark - (BookMark) mark segment
//------------------------------------------------------------------------------
// Function call:     EQFBMark( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:        This function will mark the cursor segment
//
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     -Sets the Mark- flag in the cursor segment
//                    - displays a message that segment is now bookmarked
//
//------------------------------------------------------------------------------
VOID
EQFBMark
(
  PTBDOCUMENT pDoc                      // pointer to document ida
)
{
   PTBSEGMENT pSeg;                     // pointer to segment
   PSZ_W      pData;                    // ptr to 1st 30 chars of seg

   pSeg = EQFBGetSegW(pDoc, pDoc->TBCursor.ulSegNum); // get segm
   pSeg->SegFlags.Marked = TRUE;
   pDoc->EQFBFlags.MarkedSeg = TRUE;
   pDoc->flags.changed = TRUE;          // target document changed..
                                        //display info message
   UTF16strncpy( chSeg1, pSeg->pDataW, MSGBOXDATALEN);
   chSeg1[ MSGBOXDATALEN ] = EOS;   // set end of string
   if ( UTF16strlenCHAR(chSeg1) >= MSGBOXDATALEN )
   {                                // fill last three chars with ï...ï
      UTF16strcpy(chSeg1+MSGBOXDATALEN-4,L"...");
   } /* endif */

   pData = chSeg1;
   UtlErrorW( TB_MARKSEG, MB_OK, 1, &pData, EQF_INFO, TRUE);
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBClearMark - clear the mark in the current segment
//------------------------------------------------------------------------------
// Function call:     EQFBClearMark( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:        This function will clear any mark in the cursor segment
//
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:      - check if the Mark- flag is set in the cursor segment
//                       if mark is not set then
//                         issue a warning
//                       else
//                         display message that segment now will be cleared
//                         clear mark flag
//                         set generic document flag depending if another mark
//                           is available or not
//                       endif
//------------------------------------------------------------------------------
VOID
EQFBClearMark
(
  PTBDOCUMENT pDoc                      // pointer to document ida
)
{
   PTBSEGMENT pTBSeg;                   // pointer to segment
   ULONG      ulSegNum;                 // segment number
   PSZ_W pData;                         // ptr to 1st 30 chars of seg

   pTBSeg = EQFBGetSegW(pDoc, pDoc->TBCursor.ulSegNum); // get segm
   if ( pTBSeg->SegFlags.Marked )
   {
      pTBSeg->SegFlags.Marked = FALSE;  // reset marked flag
      pDoc->flags.changed = TRUE;          // target document changed..
                                        //display info message
      UTF16strncpy( chSeg1, pTBSeg->pDataW, MSGBOXDATALEN);
      chSeg1[ MSGBOXDATALEN ] = EOS;   // set end of string
      if ( UTF16strlenCHAR(chSeg1) >= MSGBOXDATALEN )
      {                                // fill last three chars with ï...ï
         UTF16strcpy(chSeg1+MSGBOXDATALEN-4,L"...");
      } /* endif */
      pData = chSeg1;
      UtlErrorW( TB_CLEARMARK, MB_OK, 1, &pData, EQF_INFO, TRUE);

      ulSegNum = 1;
      pTBSeg = EQFBGetVisSeg(pDoc, &ulSegNum);  // point to start of table

      while ( pTBSeg && ! pTBSeg->SegFlags.Marked)
      {
         ulSegNum++;                        // point to next segment
         pTBSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
      } /* endwhile */
      pDoc->EQFBFlags.MarkedSeg = (USHORT)(pTBSeg != NULL ) ;

   }
   else
   {
      UtlError( TB_NOSEGCLEAR, MB_CANCEL, 0, NULL, EQF_WARNING);
   } /* endif */
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFindMark - find next marked segment and activate it
//------------------------------------------------------------------------------
// Function call:     EQFBFindMark( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:        This function will find the next marked segment
//
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:      -- start at the current segment and scan forward
//                        until the next marked segment is found.
//                     -- if none is available wrap around and scan until
//                        the position you left
//                     -- if none found then
//                         issue warning and stay with the cursor
//                        else
//                         goto marked segment
//
//------------------------------------------------------------------------------
VOID
EQFBFindMark
(
  PTBDOCUMENT pDoc                      // pointer to document ida
)
{
   PTBSEGMENT  pSeg;                      // pointer to segment
   PTBSEGMENT  pSegStart;                 // pointer to start segment
   ULONG       ulSegNum;                  // segment number

   ulSegNum = pDoc->TBCursor.ulSegNum + 1; // start search at next segment
   pSeg = EQFBGetVisSeg(pDoc, &ulSegNum); // get segm
   pSegStart = pSeg;
   while ( pSeg && ! pSeg->SegFlags.Marked )
   {
      ulSegNum++;                         // point to next segment
      pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
   } /* endwhile */

   if ( !pSeg || !pSeg->SegFlags.Marked )   // not found - end of table reached
   {
     ulSegNum = 1;
     pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);   // reset to begin again
     while ( pSeg && pSeg != pSegStart && ! pSeg->SegFlags.Marked)
     {
        ulSegNum++;                       // point to next segment
        pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
     } /* endwhile */
   } /* endif */

   if ( pSeg && pSeg->SegFlags.Marked )                 // start point found
   {
     if (!pDoc->hwndRichEdit )
     {
      if ( pSeg->ulSegNum == pDoc->tbActSeg.ulSegNum )
      {
         EQFBGotoActSegment( pDoc );
      }
      else
      {
         pDoc->TBCursor.ulSegNum = pSeg->ulSegNum; // set segment number
         pDoc->TBCursor.usSegOffset = 0;           // set segment offset
         EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum, 0);
      } /* endif */
     }
     else
     {
         pDoc->TBCursor.ulSegNum = pSeg->ulSegNum; // set segment number
         pDoc->TBCursor.usSegOffset = 0;           // set segment offset

                 EQFBGotoSegRTF( pDoc,
                       pDoc->TBCursor.ulSegNum,     // pos. at this seg
                       pDoc->TBCursor.usSegOffset );
     } /* endif */
   }
   else                                         // No marked segment available
   {
      UtlError( TB_NOSEGMARK, MB_CANCEL, 0, NULL, EQF_WARNING);
   } /* endif */


}
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBSetPostEdit
//------------------------------------------------------------------------------
// Function call:     EQFBSetPostEdit( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       go from first draft editing into post
//                    editing mode
//                    Any change in a workline/segment will issue
//                    a Save segment request at leaving time of the
//                    segment
//                    Any other selection on the translate pulldown
//                    will reset the flag and deactivate the checkmark.
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:      pDoc->EQFBFlags.PostEdit will be set
//
//------------------------------------------------------------------------------
// Function flow:     - if we have to save last active segment then
//                        save it
//                      else      --  (if no segment to save)
//                       - if active segment exists then
//                           set current segment qstatus
//                         endif
//                       - write workbuffer out
//                      endif
//                    - if already in post editing mode then
//                        switch back
//                      else:
//                        if okay so far then
//                          set post edit flag
//                          load new segment into work buffer
//                          get rid of proposal and dictionary window
//                        endif
//                      endif
//                    - force repaint of the screen
//------------------------------------------------------------------------------
VOID EQFBSetPostEdit( PTBDOCUMENT pDoc )
{
   BOOL fOK = TRUE;                          // success indicator
   PTBSEGMENT  pSeg;                         // pointer to segment
   ULONG       ulSegNum;

   /*******************************************************************/
   /* allow for change to postedit in new RichEdit control            */
   /*******************************************************************/
   if ( pDoc->hwndRichEdit )
   {
     EQFBSetPostEditRTF( pDoc );
     return;
   } /* endif */

   if ( pDoc->EQFBFlags.workchng )           // check if something changed ??
   {
      fOK = EQFBSaveSeg ( pDoc );            // save the last active segment
   }
   else
   {
      if ( pDoc->tbActSeg.ulSegNum)
      {
         pSeg = EQFBGetSegW( pDoc, pDoc->tbActSeg.ulSegNum );
                                             // get old status of segment
         pSeg->qStatus          = pDoc->tbActSeg.qStatus;
         pSeg->SegFlags.Current = FALSE;
         pSeg->SegFlags.Expanded = FALSE;
         EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum, 0);
         UtlAlloc((PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG);       // free old segment table
      } /* endif */
      EQFBWorkSegOut( pDoc );                // else reset the worksegment info
   } /* endif */

   if ( fOK )
   {
      if ( pDoc->EQFBFlags.PostEdit )           // if in post edit switch back
      {
          pDoc->EQFBFlags.PostEdit = FALSE;
          ulSegNum = pDoc->TBCursor.ulSegNum;
          EQFBDoNextTwo ( pDoc, &ulSegNum,
                          POS_TOBEORDONE ); // do the next two segments
      }
      else                                      // go into post edit mode
      {
          pDoc->tbActSeg.ulSegNum = 0;        // reset act segment number
          pDoc->EQFBFlags.PostEdit = TRUE;    // set post edit mode
                                              // get rid of prop/dict wnd
          EQFCLEAR (EQFF_NOPROPWND | EQFF_NODICTWND | EQFF_NOSEGPROPWND );
          pDoc->ulWorkSeg = 0;                // force a load of ...
          EQFBWorkSegIn( pDoc );              // ... current segment
          pDoc->EQFBFlags.EndOfSeg = FALSE;   // reset EndOfSeg flag
          EQFBScreenCursorType( pDoc );       // and set cursor correctly
      } /* endif */
      /****************************************************************/
      /* if segment boundary sign is added or deleted, autolinewrap   */
      /* must be recalculated                                         */
      /****************************************************************/
      if (pDoc->pUserSettings->fSegBound &&
          pDoc->fLineWrap && pDoc->fAutoLineWrap )
      {
        EQFBFuncMarginAct(pDoc);
        EQFBFuncMarginAct(pDoc);
        if (pDoc->fAutoSpellCheck && pDoc->pvSpellData )
		{ // force that thread recalcs pusHLType of screen
		   PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
		   pSpellData->TBFirstLine.ulSegNum = 0;
		   pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
	    }
      } /* endif */

   } /* endif */
   pDoc->Redraw |= REDRAW_ALL;
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBTempPostEdit  - used in spellcheck dialog
//------------------------------------------------------------------------------
// Function call:     EQFBTempPostEdit( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       go from first draft editing into post
//                    editing mode, similar to normal post edit mode, but
//                    let proposal and dictionary window stay on screen
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT      document ida
//
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE  if successful
//                    FALSE if user didn't want to save or error happened
//
//------------------------------------------------------------------------------
// Side effects:      pDoc->EQFBFlags.PostEdit will be set
//
//------------------------------------------------------------------------------
// Function flow:     - if we have to save last active segment: save it
//                      else:(if no segment to save)
//                       - if active segment exists then
//                           set current segment qstatus
//                         endif
//                       -  write workbuffer out
//                    - if already in post editing mode then
//                        switch back
//                      else:
//                        if okay so far then
//                          set post edit flag
//                          load new segment into work buffer
//                          DO NOT get rid of proposal and dictionary
//                              window
//                        endif
//                      endif
//
//------------------------------------------------------------------------------
BOOL EQFBTempPostEdit( PTBDOCUMENT pDoc )
{
   BOOL fOK = TRUE;                          // success indicator
   PTBSEGMENT  pSeg;                         // pointer to segment
   ULONG       ulSegNum;                     // segment number

   if ( pDoc->EQFBFlags.workchng )           // check if something changed ??
   {
      fOK = EQFBSaveSeg ( pDoc );            // save the last active segment
   }
   else
   {
      if ( pDoc->tbActSeg.ulSegNum)
      {
         pSeg = EQFBGetSegW( pDoc, pDoc->tbActSeg.ulSegNum );
                                             // get old status of segment
         pSeg->qStatus          = pDoc->tbActSeg.qStatus;
         pSeg->SegFlags.Current = FALSE;
      } /* endif */
      EQFBWorkSegOut( pDoc );                // else reset the worksegment info
   } /* endif */

   if ( pDoc->EQFBFlags.PostEdit )           // if in post edit switch back
   {
       pDoc->EQFBFlags.PostEdit = FALSE;
       ulSegNum = pDoc->TBCursor.ulSegNum;
       EQFBDoNextTwo ( pDoc, &ulSegNum,
                       POS_TOBEORDONE ); // do the next two segments
   }
   else
   {
      if ( fOK )
      {
         pDoc->tbActSeg.ulSegNum = 0;        // reset act segment number
         pDoc->EQFBFlags.PostEdit = TRUE;    // set post edit mode
                                             // get rid of prop/dict wnd
         pDoc->ulWorkSeg = 0;                // force a load of ...
      } /* endif */
      EQFBWorkSegIn( pDoc );                 // ... current segment
   } /* endif */
   return ( fOK );
}

