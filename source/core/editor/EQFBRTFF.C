
//+----------------------------------------------------------------------------+
//|EQFBRTFF.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:   R.Jornitz                                                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:   This module contains a set of routines which                 |
//|               interface to the Presentation Manager                        |
//|               It uses standard VIO-Calls to process the screen             |
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_TM               // general Transl. Memory functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFTPI.H"               // Translation Processor priv. include file
#include <eqfdoc00.h>

#include "EQFB.ID"                     // Translation Processor IDs

USHORT EQFBTCheckPos( PTBDOCUMENT );     // check position of new tag
BOOL EQFBTInitTransRTF( PTBDOCUMENT, PULONG );   // init translation environment
VOID  EQFBFuncDispOrgRTF ( PTBDOCUMENT  pDoc) ;





void EQFBWndProc_CommandRTF
(
  HWND hwnd,
  WPARAM mp1,
  LPARAM mp2
)
{
    PTBDOCUMENT pDoc;                  // pointer to document
    USHORT      usMatch;
    BOOL        fUpdate = TRUE;        // force screen update
    BOOL        fFound = FALSE;
    USHORT      usCmdId = SHORT1FROMMP1(mp1);

    mp2;                               // get rid of compiler warning

    pDoc = ACCESSWNDIDA( hwnd, PTBDOCUMENT );

    switch ( usCmdId )
    {
      case IDM_FIND:
        EQFBFuncRTFFunc( pDoc, FIND_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_CFIND:
        EQFBFuncRTFFunc( pDoc, CFIND_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_OPEN:
        EQFBFuncRTFFunc( pDoc, OPEN_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_FONTCOL:
        EQFBFuncRTFFunc( pDoc, FONTS_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_KEYS:
        EQFBFuncRTFFunc( pDoc, KEYS_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_ENTRY:
        EQFBFuncRTFFunc( pDoc, ENTRYSEN_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_COMMAND:
        EQFBFuncRTFFunc( pDoc, COMMAND_FUNC, hwnd, mp1, mp2 );
        fUpdate = FALSE;               // no refresh necessary any more
        break;

      case IDM_SETTINGS:
        EQFBFuncRTFFunc( pDoc, SETTINGS_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_SAVE:
        EQFBFuncRTFFunc( pDoc, SAVE_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_PRINT:
        EQFBFuncRTFFunc( pDoc, PRINT_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_QUIT:      // Quit current document - same behaviour as CLOSE
        switch ( pDoc->docType )
        {
           case SSOURCE_DOC:
              WinShowWindow( pDoc->hwndFrame , FALSE );
              SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                              WM_EQF_SETFOCUS,
                              0, MP2FROMP( pDoc->twin->hwndFrame ));
              break;
           case STARGET_DOC:
              EQFBQuit (hwnd);                       // quit source doc
              fUpdate = FALSE;
              break;
           case OTHER_DOC:
              pDoc = EQFBFuncQuit( pDoc );           // quit document
              break;
           case TRNOTE_DOC:
              WinPostMsg( hwnd, WM_CLOSE, 0, 0L );
              break;
        } /* endswitch */
        break;

      case IDM_REIMPORT:
        EQFBFuncReImportDoc( pDoc );
        break;

      case IDM_FILE:                       // save target
        WinPostMsg( hwnd, WM_CLOSE, 0, 0L );
        fUpdate = FALSE;
        break;

      case IDM_UNDO:
        EQFBFuncRTFFunc( pDoc, UNDO_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_REDO:
        EQFBFuncRTFFunc( pDoc, REDO_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_UNMARK:
        EQFBFuncRTFFunc( pDoc, MARKCLEAR_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_FINDMARK:
        EQFBFuncRTFFunc( pDoc, MARKFIND_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_GOTOLINE:
        EQFBFuncRTFFunc( pDoc, GOTO_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_GOTOSEG:
        EQFBFuncRTFFunc( pDoc, GOTOSEGMENT_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_TOCGOTO:
        EQFBFuncRTFFunc( pDoc, TOCGOTO_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_GOUPDSEGMENT:
        EQFBFuncRTFFunc( pDoc, GOUPDSEGMENT_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_QUERYLINE:
        EQFBFuncRTFFunc( pDoc, QUERYLINE_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_TOP   :
        pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
        EQFBFuncTopDoc( pDoc );        // move to the top of the document
        EQFBGotoSegRTF( pDoc,
                        pDoc->TBCursor.ulSegNum,     // pos. at this segment
                        0 );
        break;

      case IDM_BOTTOM:   // is menue nec??
        pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
        EQFBFuncBottomDoc( pDoc );           // move to the bottom of the document
        EQFBGotoSegRTF( pDoc,
                        pDoc->TBCursor.ulSegNum,     // pos. at this segment
                        0 );
        break;

      case IDM_SOS   :
        pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
        EQFBFuncStartSeg ( pDoc );           // move to the start of segment
        EQFBGotoSegRTF( pDoc,
                        pDoc->TBCursor.ulSegNum,     // pos. at this segment
                        0 );
        break;

      case IDM_EOS   :
        EQFBFuncEndSeg ( pDoc );             // move to the end of segment
        break;

      case IDM_SPLIT:
        EQFBFuncRTFFunc( pDoc, SPLITLINE_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_JOIN:
        EQFBFuncRTFFunc( pDoc, JOINLINE_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_CYCLE:
        EQFBFuncRTFFunc( pDoc, NEXTDOC_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_CUT:                       // Copy to clipboard and cut
        EQFBFuncRTFFunc( pDoc, CUT_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_COPY:                      // Copy to clipboard
        EQFBFuncRTFFunc( pDoc, COPY_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_CLEAR:
        EQFBFuncRTFFunc( pDoc, MARKDELETE_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_PASTE:                     // paste from clipboard
        EQFBFuncRTFFunc( pDoc, PASTE_FUNC, hwnd, mp1, mp2 );
        break;

      case IDM_COPYPROPBLOCK:             // copy proposal block
        EQFBFuncRTFFunc( pDoc, PROPMARKCOPY_FUNC, hwnd, mp1, mp2 );
        break;

     case IDM_TRANSSEG:                   // translate the segment
        EQFBTransRTF( pDoc, POS_TOBEORDONE );         // pos at translated ones, too
        break;
     case IDM_TRANSNEW:                   // position at the next untransl.
        EQFBTransRTF( pDoc, POS_TOBE );         
        break;
     case IDM_TRANSNEW_EXACT:             // position at the next untransl. with EXACT matches
        EQFBTransRTF( pDoc, POS_TOBE_EXACT );             
        break;
     case IDM_TRANSNEW_FUZZY:             // position at the next untransl. with FUZZY matches
        EQFBTransRTF( pDoc, POS_TOBE_FUZZY );             
        break;
     case IDM_TRANSNEW_NONE:              // position at the next untransl. with NO matches
        EQFBTransRTF( pDoc, POS_TOBE_NONE );              
        break;
     case IDM_TRANSNEW_MT:                // position at the next untransl. with MT matches
        EQFBTransRTF( pDoc, POS_TOBE_MT );              
        break;
     case IDM_TRANSNEW_GLOBAL:            // position at the next untransl. with GLOBAL MEMORY matches
        EQFBTransRTF( pDoc, POS_TOBE_GLOBAL );              
        break;
     case IDM_UNTRANS:                    // untranslate the active segment
        EQFBFuncRTFFunc( pDoc, UNTRANS_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_DICTLOOK:                   // activate dictionary lookup
        EQFBFuncRTFFunc( pDoc, DICTLOOK_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_EDITTERM:
        EQFBFuncRTFFunc( pDoc, EDITTERM_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_ADDABBREV:                  // add abbreviation to abbrev.list
        EQFBFuncRTFFunc( pDoc, ADDABBREV_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_EDITABBREV:                 // edit abbreviations
        EQFBEditAbbrev( pDoc );
        break;
     case IDM_EDITADD:                     // edit addenda terms
        EQFBFuncRTFFunc( pDoc, EDITADD_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_TRANSWND:                   // activate translation window
        EQFBFuncRTFFunc( pDoc, ACTTRANS_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_DICTWND:                    // activate dictionary window
        EQFBFuncRTFFunc( pDoc, ACTDIC_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_TMWND:                      // activate translation mem window
        EQFBFuncRTFFunc( pDoc, ACTPROP_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_DISPORG:                    // display the original synchronized
        EQFBFuncDispOrgRTF( pDoc );
        break;
     case IDM_SRCPROPWND:                 // display the original synchronized
        EQFBFuncDispSrcProp (pDoc);
        break;
     case IDM_SEGPROPWND:                 // display the segment properties window
        EQFBFuncDispSegProp (pDoc);
        break;
     case IDM_GOTO:                       // goto the active segment
        EQFBFuncRTFFunc( pDoc, GOTOSEG_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_JOINSEG:                    // join segments
        EQFBFuncRTFFunc( pDoc, JOINSEG_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_SPLITSEG:                   // split segments
        EQFBFuncRTFFunc( pDoc, SPLITSEG_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_MARKSEG:                    // mark active segment
        pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
        EQFBUpdateTBCursor( pDoc );
        EQFBMark( pDoc );
        break;
     case IDM_GOTOMARK:                   // goto marked segment
        EQFBFindMark( pDoc );
        break;
     case IDM_CLEARMARK:                  // clear any available mark
        pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
        EQFBUpdateTBCursor( pDoc );
        EQFBClearMark( pDoc );
        break;
      case IDM_POSTEDIT:                   // switch to postediting mode
        EQFBSetPostEditRTF( pDoc );
        break;
     case IDM_AUTOTRANS:                  // switch to automatic translation
        EQFBFuncRTFFunc( pDoc, AUTOTRANS_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_SHOWTRANS:                  // switch to automatic translation
        EQFBFuncRTFFunc( pDoc, SHOWTRANS_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_VISIBLESPACE:
        EQFBFuncRTFFunc( pDoc, VISIBLESPACE_FUNC, hwnd, mp1, mp2 );
//        InvalidateRect( pDoc->hwndRichEdit, NULL, FALSE );
        break;
     case IDM_PROOFSEG:                   // spellcheck of current segment
        EQFBFuncRTFFunc( pDoc, SPELLSEG_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_PROOFALL:                   // spellcheck for file
        EQFBFuncRTFFunc( pDoc, SPELLFILE_FUNC, hwnd, mp1, mp2 );
        break;

     case IDM_AUTOSPELL:                   // autospellcheck on/off
        EQFBFuncRTFFunc( pDoc, SPELLAUTO_FUNC, hwnd, mp1, mp2 );
        break;
     case IDM_NEXTMISSPELL:                 // goto next misspelled
        EQFBFuncRTFFunc( pDoc, NEXTMISSPELLED_FUNC, hwnd, mp1, mp2 );
        break;

     case IDM_PROTECTED:                  // switch to protected mode
        EQFBChangeStyle( pDoc,  DISP_PROTECTED);
        SendMessage( pDoc->hwndClient, WM_EQF_FONTCHANGED, 0L, 0L );
        break;
     case IDM_HIDE:                       // switch to hided mode
        EQFBChangeStyle( pDoc,  DISP_HIDE);
        SendMessage( pDoc->hwndClient, WM_EQF_FONTCHANGED, 0L, 0L );
        break;
     case IDM_UNPROTECTED:                // switch to unprotected mode
        EQFBChangeStyle( pDoc,  DISP_UNPROTECTED);
        SendMessage( pDoc->hwndClient, WM_EQF_FONTCHANGED, 0L, 0L );
        break;
     case IDM_SHRINK:                     // switch to shrink style
        EQFBChangeStyle( pDoc,  DISP_SHRINK);
        SendMessage( pDoc->hwndClient, WM_EQF_FONTCHANGED, 0L, 0L );
        break;
     case IDM_COMPACT:                    // switch to compact style
        EQFBChangeStyle( pDoc,  DISP_COMPACT);
        SendMessage( pDoc->hwndClient, WM_EQF_FONTCHANGED, 0L, 0L );
        break;
     case IDM_SHORTEN:                    // switch to shorten style
        EQFBChangeStyle( pDoc,  DISP_SHORTEN);
        SendMessage( pDoc->hwndClient, WM_EQF_FONTCHANGED, 0L, 0L );
        break;
     case IDM_WYSIWYG:                    // switch to WYSIWYG style
        EQFBChangeStyle( pDoc,  DISP_WYSIWYG);
        SendMessage( pDoc->hwndClient, WM_EQF_FONTCHANGED, 0L, 0L );
        break;
     case IDM_SC_HORZ:
       EQFBSetResetFrameCtrls( pDoc, hwnd, FCF_HORZSCROLL );
       break;
     case IDM_SC_VERT:
       EQFBSetResetFrameCtrls( pDoc, hwnd, FCF_VERTSCROLL );
       break;
     case IDM_SC_TITLE:
       EQFBSetResetFrameCtrls( pDoc, hwnd, (FCF_TITLEBAR | FCF_SYSMENU) );
       break;

     case IDM_SC_STATUS:
       {
//       BOOL fShowWindow = !WinIsWindowVisible( pDoc->hStatusBarWnd );
//       BYTE bStatus = ((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType];
//       if ( !fShowWindow )
//       {
//         WinShowWindow( pDoc->hStatusBarWnd, FALSE );
//         bStatus &= ~TP_WND_STATUSBAR;
//       }
//       else
//       {
//         bStatus |= TP_WND_STATUSBAR;
//       } /* endif */
//       /*************************************************************/
//       /* store settings for next load                              */
//       /*************************************************************/
//       ((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType] = bStatus;
//       /********************************************************************/
//       /* force the update of the window ...                               */
//       /********************************************************************/
//       {
//         RECT  rc;
//         GetClientRect(hwnd, &rc);
//         PostMessage( hwnd, WM_SIZE,
//                      0, MAKELPARAM( rc.right, rc.bottom ));
//       }
       }
       break;

     case IDM_SC_RULER:
       {
         BOOL fShowWindow = !WinIsWindowVisible( pDoc->hRulerWnd );
         BYTE bStatus = ((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType];
         if ( !fShowWindow )
         {
           WinShowWindow( pDoc->hRulerWnd, FALSE );
           bStatus &= ~TP_WND_RULER;
           pDoc->ulRulerSize = 0;
         }
         else
         {
           WinShowWindow( pDoc->hRulerWnd, TRUE );
           bStatus |= TP_WND_RULER;
           pDoc->ulRulerSize = pDoc->cy;
         } /* endif */
         /*************************************************************/
         /* store settings for next load                              */
         /*************************************************************/
         ((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType] = bStatus;
         /********************************************************************/
         /* force the update of the window ...                               */
         /********************************************************************/
         {
           RECT  rc;
           GetClientRect(hwnd, &rc);
           PostMessage( hwnd, WM_SIZE,
                        0, MAKELPARAM( rc.right, rc.bottom ));
         }
       }
       break;

     case PID_SYS_CLOSE:
       WinPostMsg( hwnd, WM_SYSCOMMAND, SC_CLOSE, NULL );
       break;
     case PID_SYS_SIZE:
       WinSendMsg( hwnd, WM_SYSCOMMAND, SC_SIZE, NULL );
       break;
     case PID_SYS_MOVE:
       WinSendMsg( hwnd, WM_SYSCOMMAND, SC_MOVE, NULL );
       break;
     case IDM_HOTPOPUP:
       {
         POINT  lppt;
         SHORT  sPopUpId;
         GetCaretPos( &lppt );
         /********************************************************/
         /* find correct popup ...                               */
         /********************************************************/
         switch ( pDoc->docType )
         {
           case TRNOTE_DOC:       // TRNOTE document
             sPopUpId = ID_TPRO_POPUP_TRNOTE;
             break;
           case OTHER_DOC:        // other documents besides ....
             sPopUpId = ID_TPRO_POPUP_OTHER;
             break;
           case SSOURCE_DOC:      // source segmented document
             sPopUpId = ID_TPRO_POPUP_SRC;
             break;
           case STARGET_DOC:      // target segmented document
             sPopUpId = ID_TPRO_POPUP_TRANS;
             break;
           case SERVPROP_DOC:     // proposal window
             sPopUpId = ID_SRV_POPUP_TM;
             break;
           case SERVDICT_DOC:     // dictionary window
             sPopUpId = ID_SRV_POPUP_DCT;
             break;
           case SERVSOURCE_DOC:   // source window for proposals
             sPopUpId = ID_SRV_POPUP_SRCP;
             break;
           default:
             sPopUpId = 0;
             break;
         } /* endswitch */
       }
       break;

     default:
        usMatch = usCmdId;

        if ( (usMatch >= IDM_DICTCPY)  && (usMatch < (IDM_DICTCPY+26)) )
        {
          pDoc->usChar = usMatch - IDM_DICTCPY + CHARACTER_A ;
          EQFBGetDictMatch( pDoc );
        }
        else if ( (usMatch >= IDM_DICTCPY+26) && (usMatch <= (IDM_DICTCPY+51)) )
        {
          /************************************************************/
          /* force for IDM_DICTCPY+27 usChar = A and so on            */
          /************************************************************/
          pDoc->usChar = usMatch -(IDM_DICTCPY+26) + 'A' ;
          EQFBGetDictMatch( pDoc );
        }
        else if ( (usMatch >= IDM_PROPCPY)  && (usMatch <= IDM_PROPCPY+10) )
        {
          pDoc->usChar = (usMatch - IDM_PROPCPY) + CHARACTER_0;
          EQFBGetPropMatch( pDoc );
        }
        else if ( (usMatch >= IDM_OTHERWND) && (usMatch <= IDM_LASTOTHER) )
        {
          while ( !fFound )
          {
            pDoc = pDoc->next;
            if (((pDoc->docType == OTHER_DOC) || (pDoc->docType == TRNOTE_DOC))
                 && ( pDoc->usWndId == usMatch )  )
            {
               fFound = TRUE;
            } /* endif */
          } /* endwhile */
          /********************************************************/
          /* user wants to go to this document                    */
          /********************************************************/
          pDoc->Redraw |= REDRAW_ALL;
                                      // activate document window
          WinShowWindow( pDoc->hwndFrame, TRUE );

          WinSetActiveWindow( HWND_DESKTOP, pDoc->hwndFrame );
        }
        else if ( (usMatch >= IDM_DOC_ENVIRONMENT) &&
                  (usMatch <= IDM_LASTDOC_ENVIRONMENT) )
        {
          PSZ pOutDocObjName = NULL;

          if ( UtlAlloc( (PVOID *)&pOutDocObjName, 0L, (LONG)MAX_LONGPATH, NOMSG ) )
          {
            USHORT usRc;
            usRc = EQF_XDOCNUM( (PSTEQFGEN)pDoc->pstEQFGen, (USHORT)(usMatch-IDM_DOC_ENVIRONMENT),
                                       pOutDocObjName );
            if ( !usRc && *pOutDocObjName )
            {
              usRc = EQFBTenvStart( pDoc, pOutDocObjName, (PSTEQFGEN)pDoc->pstEQFGen );
            } /* endif */
            UtlAlloc( (PVOID *)&pOutDocObjName, 0L, 0L, NOMSG );
          } /* endif */
        }
        else
        {
          /************************************************************/
          /* Should not happen under OS/2, but under Windows we have  */
          /* to pass those messages to the default procedure....      */
          /************************************************************/
          DefWindowProc( hwnd, WM_COMMAND, mp1, mp2 );
        } /* endif */
        break;
    } /* command switch */

} /* EQFBWndProc_CommandRTF  */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBTrans                                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBTrans( PTBDOCUMENT );                                |
//+----------------------------------------------------------------------------+
//|Description:       This function will send (if necessary) the last          |
//|                   segment to the TM and activates the next one             |
//|                   dependent on the mode.                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT         pointer to document instance data    |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:      sSource and sTarget will be positioned                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     - check if user moved out of active segment              |
//|                     or not (EQFBTCheckPos),                                |
//|                   - reset post edit mode                                   |
//|                   i.e. 'normal' operation (just pressed Ctrl-Enter)        |
//|                   case XLATESEG:                                           |
//|                     - set translation tag                                  |
//|                     - save segment (translation together with              |
//|                       source) in TM                                        |
//|                     - find next segment to be translated or inform         |
//|                       WorkBench                                            |
//|                       if found set it current and send it to Services      |
//|                       ( FOREGROUND)                                        |
//|                       Find next and send it to services (BACKGROUND)       |
//|                   case MOVE          -- user moved unexpected              |
//|                     - inform user if he had changed something;             |
//|                       let him decide what to do                            |
//|                       Save or ignore changes depending on user decision    |
//|                     - find source segment of new position;                 |
//|                       send it to services;                                 |
//|                       activate it                                          |
//|                   case FIRST                                               |
//|                     - find next segment to be translated                   |
//|                         (call EQFBTInitTrans)                              |
//|                       if found set it current and send it to Services      |
//|                         (FOREGROUND)                                       |
//|                       Find next and send it to services (BACKGROUND)       |
//|                                                                            |
//+----------------------------------------------------------------------------+
VOID EQFBTransRTF
(
  PTBDOCUMENT pDoc,                       // pointer to document ida
  USHORT      usCond                      // start condition
)
{
   ULONG     ulSegNum;                    // segment number to start with
   USHORT    usResult;                    // return value from UtlError
   BOOL      fOK = TRUE;                  // success indicator
   PTBSEGMENT pSeg;                       // pointer to segment
   PTBSEGMENT pSourceSeg;                 // pointer to source segment
   USHORT    usStartPos;                  // start position
   PSZ_W     pData;                       // pointer to data
   HWND      hwndTemp;

   if (pDoc->ulWorkSeg)
    EQFBGetWorkSegRTF( pDoc, pDoc->ulWorkSeg );

   usStartPos = EQFBTCheckPos( pDoc );    // find start condition
   hwndTemp = pDoc->hwndFrame;

//   pDoc->EQFBFlags.PostEdit = FALSE;      // reset post edit mode /* @39D */
   pDoc->EQFBFlags.AutoMode  = FALSE;     // reset automatic mode

//   EQFBFuncMarkClear ( pDoc );            //clear old mark

   switch ( usStartPos )
   {
      case CHECKPOS_FIRST:
         pDoc->EQFBFlags.PostEdit = FALSE;      // reset post edit  /* @39A */
         if (EQFBTInitTransRTF ( pDoc, &ulSegNum )) // find where to start
         {
            EQFBDoNextTwoRTF ( pDoc, &ulSegNum, usCond ); // do the next two segments
         }  /* endif */
         break;

      case CHECKPOS_POSTEDIT:
         if ( pDoc->EQFBFlags.workchng )
         {
            fOK = EQFBSaveSeg( pDoc );
         }
         else
         {
            EQFBWorkSegOut( pDoc );
         } /* endif */
         if ( usCond == POS_TOBEORDONE )
         {
            usCond = POS_CURSOR;                 // position at cursor
         } /* endif */
         if ( fOK )
         {
           pDoc->EQFBFlags.PostEdit = FALSE;      // reset post edit/* @39A */
           ulSegNum = pDoc->TBCursor.ulSegNum;   // set segment number
           EQFBDoNextTwoRTF ( pDoc,
                           &ulSegNum, usCond );  // do the next two segments
         } /* endif */
         break;


      case CHECKPOS_XLATESEG:
         if (EQFBSaveSeg( pDoc))                 // save current segment
         {
            pDoc->EQFBFlags.PostEdit = FALSE;    // reset post edit
                                                 // store start address
            ulSegNum = pDoc->TBCursor.ulSegNum + 1;
            EQFBDoNextTwoRTF ( pDoc,
                            &ulSegNum , usCond  );// do the next two segments
         } /* endif */
         break;

      case CHECKPOS_MOVE:
         if ( usCond == POS_TOBEORDONE )
         {
            usCond = POS_CURSOR;                // position at cursor
         } /* endif */

         // issue warning determining if user wants to save or not
         usResult = MBID_NO;                    // ignore segment if nothing ch.
                                                // something is changed
         if ( pDoc->pSaveSegW )
            pDoc->EQFBFlags.workchng = (USHORT)(UTF16strcmp( pDoc->pSaveSegW, pDoc->pEQFBWorkSegmentW ) != 0);

         if ( pDoc->fFuzzyCopied || pDoc->EQFBFlags.workchng )
         {
            usResult = UtlError( TB_CHANGESEGMENT, MB_YESNOCANCEL,
                                 0, NULL, EQF_QUERY);

            pDoc = ACCESSWNDIDA( hwndTemp, PTBDOCUMENT );
         } /* endif */

         if ( pDoc )
         {
           switch ( usResult )
           {
             case MBID_YES:
                if (EQFBSaveSeg( pDoc ))         // save current segment
                {
                   ulSegNum = (pDoc->TBCursor).ulSegNum; // store start addr
                   EQFBDoNextTwoRTF ( pDoc,                 // do the next two
                                   &ulSegNum , usCond  );// untranslated ?
                } /* endif */
                break;
             case MBID_NO:                            // ignore changes
                pSeg = EQFBGetSegW(pDoc, pDoc->tbActSeg.ulSegNum); // get seg
                                                      // get old status
                pSeg->qStatus = pDoc->tbActSeg.qStatus;
                pSeg->SegFlags.Current = FALSE;
                pSeg->SegFlags.Expanded = FALSE;
                if ( pSeg->qStatus != QF_XLATED)
                {
                  pSeg->SegFlags.Typed = FALSE;

                  pSeg->SegFlags.Copied = FALSE;
                  pSeg->usModWords = 0;
                  memset(&pSeg->CountFlag, 0, sizeof( pSeg->CountFlag));
                }
                if ( pDoc->pSaveSegW )
                {
                  pSeg->pDataW = pDoc->pSaveSegW;
                } /* endif */
                // if untranslate active,  use original segment
                // and copy it as source of translation
                if ( pSeg->SegFlags.UnTrans )    // untranslate active
                {
                   pSourceSeg = EQFBGetSegW(pDoc->twin, pDoc->tbActSeg.ulSegNum);

                   if ( UtlAlloc( (PVOID *)&pData, 0L,
                                (LONG) max((pSourceSeg->usLength+1)*sizeof(CHAR_W), MIN_ALLOC),
                                 ERROR_STORAGE) )
                   {
                      UtlAlloc( (PVOID *) &(pSeg->pDataW) , 0L, 0L, NOMSG );
                      pSeg->pDataW = pData;
                      memcpy( (PBYTE)pData,
                              (PBYTE) pSourceSeg->pDataW,
                               pSourceSeg->usLength * sizeof(CHAR_W));
                      *(pData + pSourceSeg->usLength) = EOS;
                   } /* endif */
                   pSeg->SegFlags.UnTrans = FALSE;         // reset untrans flag
                } /* endif */
                EQFBSetWorkSegRTF( pDoc, pDoc->tbActSeg.ulSegNum, pSeg->pDataW );

                pDoc->EQFBFlags.workchng = FALSE;     // no change in work seg
                SendMessage( pDoc->hwndRichEdit, EM_SETMODIFY, 0L, 0L );
                EQFBCompSeg( pSeg );

                pDoc->pSaveSegW = NULL;                 // reset save seg
                ulSegNum = pDoc->TBCursor.ulSegNum;    // store start address
                EQFBDoNextTwoRTF ( pDoc,
                                &ulSegNum , usCond  ); // do the next two segs
                break;
             default:                                  // set back to active seg
                break;

           } /* endswitch */
         } /* endif */
         break;
      default:
         WinAlarm( HWND_DESKTOP, WA_ERROR);  // should not occur but listen...
         break;
   } /* endswitch */

   pDoc = ACCESSWNDIDA( hwndTemp, PTBDOCUMENT );
   if ( pDoc )
   {
     pDoc->Redraw |= REDRAW_ALL;               // indicate to redraw everything
   } /* endif */

   return ;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBTInitTrans                                           |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBTInitTrans( PTBDOCUMENT, PUSHORT );                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:         This function will initially position the starget      |
//|                     and the ssource file                                   |
//|                     It will position it either on the CURRENT tag or       |
//|                     on the first NONE translated tag.                      |
//|                     If none is found user and MAT Tools will be informed   |
//|                     that document is already translated.                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT       - pointer to document instance         |
//|                   PUSHORT           - segment number where to start with   |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE :  start of segment found                           |
//|                   FALSE:  document already translated                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:         SOURCE_DOC and TARGET_DOC will be positioned          |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     - Scan for the current tag starting from the top of      |
//|                     the document                                           |
//|                   - if not found scan for the first to be translated       |
//|                     segment                                                |
//|                   - if no untranslated segment found inform user and       |
//|                     MAT Tools that document is already translated and      |
//|                     init the active segment structure                      |
//|                   - if segment found set segment number of active segment .|
//|                                                                            |
//+----------------------------------------------------------------------------+
BOOL EQFBTInitTransRTF
(
   PTBDOCUMENT  pDoc,                     // pointer to document instance
   PULONG  pulSegNum )                    // pointer to segment number
{
   BOOL fOK = TRUE;                       // success indicator
   PTBSEGMENT  pSeg;                      // pointer to segment struct.
   ULONG       ulSegNum;                  // segment number

   ulSegNum = 1;
   pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);        // reset to begin

   while ( pSeg && !pSeg->SegFlags.Current)
   {
      ulSegNum++;                         // point to next segment
      pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
   } /* endwhile */

   if ( !pSeg        )                      // not found - end of table reached
   {
      ulSegNum = 1;                         // reset to begin
      pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
      while ( pSeg && pSeg->qStatus != QF_TOBE && pSeg->qStatus != QF_ATTR)
      {
         ulSegNum ++;
         pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
      } /* endwhile */
   } /* endif */

   if ( pSeg && pSeg->pDataW )               // start point found
   {
      *pulSegNum = pSeg->ulSegNum;          // set return position
      pSeg->SegFlags.Current = FALSE;       // reset current flag
      if ( pSeg->qStatus == QF_CURRENT )    // get rid of current....
      {
         pSeg->qStatus = QF_TOBE;           // it's to be
      } /* endif */
      if ( pDoc->hwndRichEdit )
      {
        BYTE b = pDoc->pDispFileRTF->bRTFFill;
        pDoc->pDispFileRTF->bRTFFill = RTF_FILL;
        EQFBSetWorkSegRTF( pDoc, ulSegNum, pSeg->pDataW );
        pDoc->pDispFileRTF->bRTFFill = b;
      }
   }
   else                                         // document translated
   {
      if ( !pDoc->fXlated )                     // set translated
      {
         EQFBDocIsTranslated( pDoc );
         *pulSegNum = 1;                        // set return position
         fOK = FALSE;                           // do not proceed with proc.
      }
      else
      {
         *pulSegNum = pDoc->TBCursor.ulSegNum;  // activate the cursor pos
      } /* endif */
   } /* endif */


    return ( fOK );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBTCheckPos                                            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBTCheckPos( PTBDOCUMENT );                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       This function will determine which of the following      |
//|                   states is true:                                          |
//|                     - first call to TM (no previously segment              |
//|                       available)                                           |
//|                     - segment normally translated                          |
//|                     - user changed to another segment                      |
//|                     - in postedit mode                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        USHORT  type of user action                              |
//|                                CHECKPOS_XLATESEG normal user action        |
//|                                CHECKPOS_MOVE     user moved                |
//|                                CHECKPOS_FIRST    first call                |
//|                                CHECKPOS_POSTEDIT postedit mode             |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT -- pointer to document structure             |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       type of user action                                      |
//|                        CHECKPOS_XLATESEG normal user action                |
//|                        CHECKPOS_MOVE     user moved                        |
//|                        CHECKPOS_FIRST    first call                        |
//|                        CHECKPOS_POSTEDIT postedit mode                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:       - if postedit flag on then                             |
//|                          set output postedit                               |
//|                       elsif first invocation then                          |
//|                          set output to first call to TM                    |
//|                       elsif segnum of active segment != current segnum     |
//|                          set output to 'move'                              |
//|                       endif                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
USHORT EQFBTCheckPos
(
   PTBDOCUMENT   pDoc                     // pointer to document struct
)
{
   USHORT usResult = CHECKPOS_XLATESEG;      // segment normally translated

   EQFBUpdateTBCursor( pDoc );
   if ( pDoc->EQFBFlags.PostEdit )
   {
      usResult = CHECKPOS_POSTEDIT;          // come from post edit mode
   }
   else
   {
      if ( pDoc->tbActSeg.ulSegNum == 0)         // first invocation
      {
         usResult = CHECKPOS_FIRST;
      }
      else
      {
         if ( pDoc->tbActSeg.ulSegNum != (pDoc->TBCursor).ulSegNum )
         {
            usResult = CHECKPOS_MOVE;
         } /* endif */
      } /* endif */
   } /* endif */

   return( usResult );
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
VOID EQFBFindGotoSegRTF
(
  PTBDOCUMENT  pDoc,                   // pointer to document instance
  ULONG  ulSegNum,                     // segment number to position at
  USHORT usSegOffset,                  // segment offset to position at
  USHORT usLen                         // length of match
)
{
    PEQFBBLOCK  pBlock = (PEQFBBLOCK) pDoc->pBlockMark;

    //goto segment , position TBRowOffset and TBCursor
    if (usSegOffset + usLen > 1 )
    {
      EQFBGotoMarkSegRTF( pDoc, ulSegNum, usSegOffset,
                          ulSegNum, (USHORT) (usSegOffset + usLen - 1));
    }
    else
    {
      EQFBGotoMarkSegRTF( pDoc, ulSegNum, usSegOffset, ulSegNum, usSegOffset);
    } /* endif */
    // show selection, even if we do not have the focus ...
    SendMessage( pDoc->hwndRichEdit, EM_HIDESELECTION, 0L, 0L );

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
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBSetPostEditRTF                                       |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBSetPostEdit( PTBDOCUMENT );                          |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       go from first draft editing into post                    |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT       document instance data                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:      pDoc->EQFBFlags.PostEdit will be set                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     - if we have to save last active segment then            |
//|                       save it                                              |
//|                     else      --  (if no segment to save)                  |
//|                      - if active segment exists then                       |
//|                          set current segment qstatus                       |
//|                        endif                                               |
//|                      - write workbuffer out                                |
//|                     endif                                                  |
//|                   - if already in post editing mode then                   |
//|                       switch back                                          |
//|                     else:                                                  |
//|                       if okay so far then                                  |
//|                         set post edit flag                                 |
//|                         load new segment into work buffer                  |
//|                         get rid of proposal and dictionary window          |
//|                       endif                                                |
//|                     endif                                                  |
//+----------------------------------------------------------------------------+
VOID EQFBSetPostEditRTF( PTBDOCUMENT pDoc )
{
   BOOL fOK = TRUE;                          // success indicator
   PTBSEGMENT  pSeg;                         // pointer to segment
   ULONG       ulSegNum = 1;
   ULONG       ulNewSegNum = 0;
   USHORT      usNewSegOffset = 0;
   CHARRANGE   chRange;

   SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
   pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
   EQFBUpdateTBCursor( pDoc );

   if (pDoc->ulWorkSeg)
      EQFBGetWorkSegRTF(pDoc, pDoc->ulWorkSeg );

   if (pDoc->tbActSeg.ulSegNum)
   {
     ulSegNum = pDoc->tbActSeg.ulSegNum;
   }
   if ( pDoc->EQFBFlags.workchng )           // check if something changed ??
   {
     //force cursor to be in actseg for old editor routines
     ulNewSegNum = pDoc->TBCursor.ulSegNum;
     usNewSegOffset = pDoc->TBCursor.usSegOffset;
     pDoc->TBCursor.ulSegNum = pDoc->ulWorkSeg;
     pDoc->TBCursor.usSegOffset = 0;
     fOK = EQFBSaveSeg ( pDoc );                // save the last active segment
     pDoc->TBCursor.ulSegNum = ulNewSegNum;
     pDoc->TBCursor.usSegOffset = usNewSegOffset;
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
         EQFBGotoSegRTF( pDoc, pDoc->TBCursor.ulSegNum, 0);
      } /* endif */
      EQFBWorkSegOut( pDoc );                // else reset the worksegment info
   } /* endif */

   if ( fOK )
   {
      if ( pDoc->EQFBFlags.PostEdit )           // if in post edit switch back
      {
          pDoc->EQFBFlags.PostEdit = FALSE;
          ulSegNum = pDoc->TBCursor.ulSegNum;
          EQFBDoNextTwoRTF ( pDoc, &ulSegNum,
                          POS_TOBEORDONE ); // do the next two segments
      }
      else                                      // go into post edit mode
      {
          pDoc->tbActSeg.ulSegNum = 0;        // reset act segment number
          pDoc->EQFBFlags.PostEdit = TRUE;    // set post edit mode
                                              // get rid of prop/dict wnd
          EQFCLEAR (EQFF_NOPROPWND | EQFF_NODICTWND | EQFF_NOSEGPROPWND);
          pDoc->ulWorkSeg = 0;                // force a load of ...
          EQFBWorkSegIn( pDoc );              // ... current segment
          pDoc->EQFBFlags.EndOfSeg = FALSE;   // reset EndOfSeg flag
      } /* endif */
   } /* endif */
   EQFBDisplayFileNewRTF( pDoc );
   pSeg = EQFBGetSegW( pDoc, ulSegNum );
   EQFBSetWorkSegRTF( pDoc, ulSegNum, pSeg->pDataW );
   SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBFuncDispOrg                                          |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBFuncDispOrg(PTBDOCUMENT)                             |
//+----------------------------------------------------------------------------+
//|Description:       display the original                                     |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc ptr to document instance                |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if in target document                                    |
//|                     if not visible synchronize if first                    |
//|                     show and activate source window                        |
//|                   else                                                     |
//|                     beep                                                   |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

VOID  EQFBFuncDispOrgRTF
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
  /********************************************************************/
  /* Load original document if not yet done                           */
  /********************************************************************/
  PTBDOCUMENT pTempDoc = pDoc->next;

  while ( pTempDoc->docType != SSOURCE_DOC && pTempDoc != pDoc)
  {
    pTempDoc = pTempDoc->next;
  } /* endwhile */
  if ( (pTempDoc->docType == SSOURCE_DOC) && !pTempDoc->pDispFileRTF->pHeader )
  {
    SETCURSOR(SPTR_WAIT);
    EQFBDispFileRTF( pTempDoc );
    SETCURSOR(SPTR_ARROW);
  } /* endif */

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
    BringWindowToTop( pDoc->twin->hwndFrame );
    WinSetActiveWindow (HWND_DESKTOP, pDoc->twin->hwndFrame );
    PostMessage( ((PSTEQFGEN) pDoc->pstEQFGen)->hwndTWBS, WM_EQF_SETFOCUS,
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
    WinSetActiveWindow( HWND_DESKTOP, pDoc->hwndFrame );
    PostMessage( ((PSTEQFGEN) pDoc->pstEQFGen)->hwndTWBS, WM_EQF_SETFOCUS,
                   0, MP2FROMHWND( pDoc->hwndFrame ));

  } /* endif */
}


VOID EQFBUndoRTF( PTBDOCUMENT pDoc )
{
  if ( SendMessage( pDoc->hwndRichEdit, EM_CANUNDO, 0, 0L ) )
  {
    SendMessage( pDoc->hwndRichEdit, EM_UNDO, 0, 0L );
  }
  else
  {
    EQFBFuncNothing( pDoc );
  } /* endif */
}



VOID EQFBRedoRTF( PTBDOCUMENT pDoc )
{
  if ( SendMessage( pDoc->hwndRichEdit, EM_CANREDO, 0, 0L ) )
  {
    SendMessage( pDoc->hwndRichEdit, EM_REDO, 0, 0L );
  }
  else
  {
    EQFBFuncNothing( pDoc );
  } /* endif */

}


/********************************************************************/
/* Get the selected area and convert it into TPRO coordinates       */
/********************************************************************/
VOID EQFBGetSelBlockRTF( PTBDOCUMENT pDoc )
{
  CHARRANGE   chRange;
  TBROWOFFSET TBCursor;

  PEQFBBLOCK  pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;      // block structure
  memset( pstBlock, 0, sizeof( EQFBBLOCK ) );

  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
  if ( chRange.cpMin < chRange.cpMax )
  {
    pstBlock->pDoc = pDoc;
    EQFBGetSegFromCaretRTF( pDoc, &TBCursor, chRange.cpMin );
    pstBlock->ulSegNum = TBCursor.ulSegNum;
    pstBlock->usStart  = TBCursor.usSegOffset;
    EQFBGetSegFromCaretRTF( pDoc, &TBCursor, chRange.cpMax - 1 );
    pstBlock->ulEndSegNum = TBCursor.ulSegNum;
    pstBlock->usEnd    = TBCursor.usSegOffset;
  } /* endif */
}
