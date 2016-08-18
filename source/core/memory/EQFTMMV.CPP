/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TP               // public translation processor definitions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFTPI.H"               // private translation processor definitions
#include "EQFTMM.H"               // TMM include file  .
#include "EQFTMM.id"                   // TMM dialog include file



//Globals
static TBSEGMENT  tb2NewLineSegment = {  "\n\n", 2, QF_XLATED ,0, NULL,{0}, 0, {0}, 0,0,0, 0L, NULL, L"\n\n", NULL};

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TMMVISWNDPROC
//------------------------------------------------------------------------------
// Function call:     window procedure
//------------------------------------------------------------------------------
// Description:       window proc for visual itm documents
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd   window handle
//                    USHORT msg  message number
//                    MPARAM mp1  message parameter1
//                    MPARAM mp2  message parameter2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       from default window procedure
//------------------------------------------------------------------------------
// Function flow:     dependant on message do case processing
//                    WM_CREATE: do nothing
//                    WM_ERASEBACKGROUND
//                      get the background cleared in the system default color
//                    WM_PAINT
//                      repaint the visual document
//                    WM_SIZE
//                      size the visual document window
//                    WM_CHAR
//                      call routine which handles all WM_CHAR msg
//                    WM_BUTTON1DOWN
//                      set cursor according to mouseclick
//                    WM_ACTIVATE
//                      activate either src or tgt document
//                    WM_BUTTON1UP
//                      release mouse button1
//                    WM_BUTTON1DBCLK
//                      mark current segment on mouse doubleclick
//                    WM_BUTTON2DBLCLK
//                      unmark segment on mousebutton 2 doubleclick
//                    WM_HSCROLL
//                      scroll horizontally
//                    WM_VSCROLL
//                      scroll vertically
//                    WM_CLOSE
//                      free all allocated areas
//                      force a close of workbench
//                    default:
//                      pass on the message to the default window procedure..
//                  endswitch
//                  return mResult
//------------------------------------------------------------------------------
MRESULT APIENTRY TMMVISWNDPROC
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT mResult = FALSE;           // window proc return value
  PTMMVISDOC pVisDoc = ACCESSWNDIDA( hwnd, PTMMVISDOC );

  switch( msg )
  {
    case WM_CREATE:
      break;


    case WM_SYSCOMMAND:
      mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
      break;

    case WM_COMMAND:
      WinPostMsg( ((PTMMIDA)pVisDoc->pTMMIda)->hwnd, WM_COMMAND, mp1, mp2 );
      break;

    case WM_PAINT:
    {
      PTBDOCUMENT pDoc = pVisDoc->pDoc;
      PAINTSTRUCT ps;
      HDC    hdc;

      hdc = BeginPaint(hwnd, &ps );
      pDoc->Redraw |= REDRAW_ALL; // enforce repaint....
      EQFBRefreshScreen( pDoc );  // refresh the screen
      EndPaint(hwnd, &ps);
      mResult = (LRESULT) FALSE;
    }
    break;

  case WM_EQF_FONTCHANGED:
    {
      PTBDOCUMENT pDoc = pVisDoc->pDoc;
      PVIOFONTCELLSIZE pVioFont;                 // pointer to font

      {
        RECT  rect;
        HDC    hdc = GetDC( pDoc->hwndClient );
        GetClientRect( pDoc->hwndClient, &rect );
        FillRect( hdc, &rect, (HBRUSH)GetStockObject( WHITE_BRUSH ));
        ReleaseDC (pDoc->hwndClient, hdc);
      }
      pVioFont = get_vioFontSize();
      EQFBSetNewCellSize( pDoc, (pVioFont + pDoc->docType)->cx,
                          (pVioFont + pDoc->docType)->cy );
    }
    break;


    case WM_SIZE:
     if ( pVisDoc != NULL )
     {
      PTBDOCUMENT pDoc = pVisDoc->pDoc;
      PTBROWOFFSET pTBRow;
      ULONG        ulOldRow;

      if (pDoc)
      {
        pDoc->xClient = SHORT1FROMMP2(mp2) ;
        pDoc->yClient = SHORT2FROMMP2(mp2) ;

        EQFBVioSetNewDocSize( pDoc );


        pTBRow = pDoc->TBRowOffset+1 + pDoc->lCursorRow; // get first element
        if ( pTBRow->ulSegNum )     // document already loaded?
        {
          ULONG ulPos;
          if ( pDoc->ulMaxSeg > 32000 )
          {
            ulOldRow = pDoc->ulMaxSeg / 2;
            ulPos = (pTBRow->ulSegNum  - 1 ) / 2;
          }
          else
          {
            ulOldRow = max(pDoc->ulMaxSeg - 2, 2);
            ulPos = pTBRow->ulSegNum - 1;
          } /* endif */

          SetScrollRange( pDoc->hwndFrame, SB_VERT, 0, ulOldRow, FALSE);
          SetScrollPos( pDoc->hwndFrame, SB_VERT, ulPos, TRUE );

          SetScrollRange( pDoc->hwndFrame, SB_HORZ, 0, 255, FALSE);

	      if (pDoc->fTARight)
	      {
			if (pDoc-> lSideScroll < 255)
			{
				SetScrollPos(pDoc->hwndFrame, SB_HORZ,255 - pDoc->lSideScroll, TRUE );
			}
			else
			{
				SetScrollPos(pDoc->hwndFrame, SB_HORZ,0, TRUE );
			}
		  }
	      else
	      {
			  SetScrollPos(pDoc->hwndFrame, SB_HORZ,pDoc->lSideScroll, TRUE );
          }
        } /* endif */

        EQFBScreenCursor( pDoc );      // position cursor and slider
      } /* endif */
    }
    break;
    case WM_CLOSE:
      ANCHORWNDIDA( hwnd, NULL );
      mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );
      break;

    case WM_WINDOWPOSCHANGING:
	  /************************************************************/
	  /* adjust positions/sizes of window to match our display    */
	  /* characteristics (character size chosen)                  */
	  /************************************************************/
	  if ( pVisDoc != NULL )
	  {
		WINDOWPOS FAR * pSWP = (WINDOWPOS FAR *) mp2;
		ULONG  ulDelta;
		PTBDOCUMENT pDoc = NULL;
		LONG   ulTemp = 0;
        pDoc = pVisDoc->pDoc;
		if ( pDoc && pDoc->cx && pDoc->cy)
		{
           // maximized window no sizable border
		   ulDelta = 2*WinQuerySysValue(HWND_DESKTOP,SM_CXFIXEDFRAME)- 2;
		   if ( GetWindowLong( pDoc->hwndFrame, GWL_STYLE ) & WS_VSCROLL )
		   {
			 ulDelta += WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL );
		   } /* endif */
		   pSWP->cx = ulDelta  +
						((pSWP->cx - ulDelta) / pDoc->cx ) * pDoc->cx;

		   /*******************************************************/
		   /* horizontal size ...                                 */
		   /*******************************************************/
		   ulDelta = 2*WinQuerySysValue(HWND_DESKTOP,SM_CYFIXEDFRAME);
		   if ( GetWindowLong( pDoc->hwndFrame, GWL_STYLE ) & WS_HSCROLL )
		   {
			 ulDelta += WinQuerySysValue(HWND_DESKTOP, SV_CYHSCROLL );
		   } /* endif */
		   if ( GetWindowLong( pDoc->hwndFrame, GWL_STYLE ) & WS_CAPTION )
		   {
			 ulDelta += WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
		   } /* endif */

		   pSWP->cy = ulDelta +
					  ((pSWP->cy - ulDelta) / pDoc->cy ) * pDoc->cy;
		   ulTemp = pSWP->cy;
		} /* endif */
	  }
      break;
  case WM_KEYDOWN:
    /***********************************************************/
    /* ignore CTRL key if it comes from an extended key...     */
    /***********************************************************/
    if ( mp2 & 0x020000000 )
    {
      /*********************************************************/
      /* ignore the request                                    */
      /*********************************************************/
      break;
    }
    else
    {
      /***********************************************************/
      /* check if help key is pressed -> in all other cases fall */
      /* through to normal processing...                         */
      /***********************************************************/
      if ( mp1 == VK_F1 )
      {
        /**************************************************************/
        /* Trigger help by posting HM_HELPSUBITEM_NOT_FOUND to TWB    */
        /**************************************************************/
        PostMessage( (HWND)UtlQueryULong( QL_TWBFRAME ),
                     HM_HELPSUBITEM_NOT_FOUND,
                     0,
                     MP2FROM2SHORT( ID_TMM_WINDOW, ID_TMM_WINDOW ));
        break;
      } /* endif */
    } /* endif */
  case WM_SYSCHAR:
  case WM_SYSKEYDOWN:
  case WM_CHAR:           // determine character and pass it to editor
    mResult = HandleTMMWMChar( hwnd, mp1, mp2, msg );
    break;

        case WM_BUTTON1DOWN:    // Position cursor to pointer
            /**********************************************************/
            /* use button1down ONLY if window has focus ...           */
            /**********************************************************/
            if ( GETFOCUS() == hwnd)
            {
              /************************************************************/
              /* get rid of any selection                                 */
              /************************************************************/
              PTBDOCUMENT pDoc = pVisDoc->pDoc;
              if ( pDoc->pBlockMark && ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc )
              {
                memset( pDoc->pBlockMark, 0, sizeof( EQFBBLOCK ));
                pDoc->Redraw |= REDRAW_ALL;
                EQFBRefreshScreen( pDoc );
              } /* endif */
              if ( EQFBMousePosition( pDoc, msg, mp1, mp2 ) )
              {
                SETCAPTURE( hwnd );            // capture the mouse
              } /* endif */
            }
            else
            {
              // inform the window of focus change...
                SendMessage( ((PTMMIDA)pVisDoc->pTMMIda)->hwnd,
                              WM_EQF_SETFOCUS,
                              0, MP2FROMP( hwnd ));
            } /* endif */

            break;

          case WM_MOUSEMOVE:      // If button 1 down, send mark block to editor
           {
            HWND hwndCapt = GETCAPTURE;
            if ( hwndCapt == hwnd )
            {
              EQFBMousePosition( pVisDoc->pDoc, msg, mp1, mp2 );
            }
            else
            {
              /********************************************************/
              /* if nothing captured ignore the reset of WinSetCapture*/
              /********************************************************/
              if ( hwndCapt )
              {
                RELEASECAPTURE;               // release the mouse
              } /* endif */
            } /* endif */
            mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
           }
           break;


     case WM_ACTIVATE:
      {
        PTBDOCUMENT pDoc = pVisDoc->pDoc;
        if ( (SHORT1FROMMP1(mp1)) )
        {
          ((PTMMIDA)pVisDoc->pTMMIda)->usActWin = pDoc->docType;
        } /* endif */
        mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
      }
      break;
     case WM_BUTTON1UP:      // Send
       {
         PTBDOCUMENT pDoc = pVisDoc->pDoc;
         if (pDoc->EQFBFlags.PostEdit &&
                (pDoc->ulWorkSeg != pDoc->TBCursor.ulSegNum) )
         {
           EQFBWorkSegOut(pDoc);
           EQFBWorkSegIn(pDoc);
         } /* endif */
         RELEASECAPTURE;                       // release the mouse
         mResult =  WinDefWindowProc ( hwnd, msg, mp1, mp2 );
       }
       break;
     case WM_BUTTON1DBLCLK:          // mark segment if in valid area
       {
         PTBDOCUMENT pDoc = pVisDoc->pDoc;
         if ( pDoc->TBCursor.ulSegNum % 2 )
         {
           if ( pDoc->twin->pBlockMark )
           {
             // clear mark in inactive window
             EQFBFuncMarkClear ( pDoc->twin );
           } /* endif */

           if (pDoc->lCursorRow==(SHORT) min(pDoc->mouseRow,pDoc->lScrnRows-1)
                 &&  pDoc->lCursorCol == (SHORT) pDoc->mouseCol )
           {
             EQFBFuncMarkSegment( pDoc );
             EQFBScreenData( pDoc );                   // display screen
           } /* endif */
         } /* endif */
         mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
       }
       break;

     case WM_BUTTON2DBLCLK:  // Send un-mark to editor
       {
         PTBDOCUMENT pDoc = pVisDoc->pDoc;
         EQFBFuncMarkClear( pDoc );
         mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
       }
       break;
    case WM_HSCROLL:
      {
         PTBDOCUMENT pDoc = pVisDoc->pDoc;
         USHORT  usSBType, usThumbPosition;
         usSBType        = (USHORT)LOWORD(mp1);
         usThumbPosition = (USHORT)HIWORD(mp1);
         EQFBHScroll ( pDoc, usSBType, usThumbPosition );
      }
      break;

    case WM_VSCROLL:
      {
        PTBDOCUMENT pDoc = pVisDoc->pDoc;
        USHORT  usSBType, usThumbPosition;
        usSBType =        (USHORT)LOWORD(mp1);
        usThumbPosition = (USHORT)HIWORD(mp1);
        EQFBVScroll ( pDoc, usSBType, usThumbPosition );
      }
      break;

    case WM_KILLFOCUS:
      HideCaret( hwnd );
      DestroyCaret();
      break;

    case WM_SETFOCUS:
      {
         PTBDOCUMENT pDoc = pVisDoc->pDoc;

         ((PTMMIDA)pVisDoc->pTMMIda)->hwndFocus = hwnd;

         WinSendMsg( pDoc->next->hwndFrame, WM_NCACTIVATE, FALSE, NULL );
         WinSendMsg( hwnd, WM_NCACTIVATE, TRUE, NULL );
         CreateCaret(hwnd, (HBITMAP)NULL,
                     pDoc->vioCurShapes[pDoc->usCursorType].cx,
                     pDoc->vioCurShapes[pDoc->usCursorType].cEnd);
         SetCaretPos( pDoc->lCursorCol * pDoc->cx,pDoc->lCursorRow * pDoc->cy);
         ShowCaret( hwnd );
         ((PTMMIDA)pVisDoc->pTMMIda)->usActWin = pDoc->docType;

         /************************************************************/
		 /* get rid of any selection (if in CUA mode )               */
		 /************************************************************/
		 if ( pDoc->pBlockMark && ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc )
		 {
			   memset( pDoc->pBlockMark, 0, sizeof( EQFBBLOCK ));
			   pDoc->Redraw |= REDRAW_ALL;        // force redraw the screen
			   EQFBRefreshScreen( pDoc );
		 } /* endif */

		 /********************************************************/
		 /* display IME conversion window as hot-spot conversion */
		 /* window at cursor place                               */
		 /********************************************************/
		 if ( pDoc->hlfIME )
		 {
			 ImeMoveConvertWin(pDoc, hwnd,
							   (SHORT)(pDoc->lCursorCol * pDoc->cx),
							   (SHORT)(pDoc->lCursorRow * pDoc->cy) );

			 ImeSetFont( pDoc, hwnd, &pDoc->lf );
		 } /* endif */

       }
       break;

        case WM_MOUSEACTIVATE:
          if ( (hwnd != GetFocus()))
          {
            WinSetFocus( HWND_DESKTOP, hwnd );
          } /* endif */
          mResult = MA_ACTIVATE;
          break;

    default:
        mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );
        break;
  } /* switch */

  return mResult ;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     HandleTMMWMChar
//------------------------------------------------------------------------------
// Function call:     HandleTMMWMChar(hwnd, mp1, mp2)
//------------------------------------------------------------------------------
// Description:       handles WM_CHAR msg send to the visdoc windows
//------------------------------------------------------------------------------
// Parameters:        HWND        hwnd,
//                    MPARAM      mp1,
//                    MPARAM      mp2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:   switch (key):
//                    ctrl-shift-left: mark previous word
//                    ctrl-shift-right: mark next word
//                    ctrl-insert: copy to clipboard
//                    ctrl-home: go to top of document
//                    ctrl-end: go to end of document
//                    ctrl-left: go to previous word
//                    ctrl-right: goto next word
//                    ctrl-enter: activate next segment saving previously
//                                active
//                    ctrl-delete: truncate line
//                    ctrl-t: activate other window
//                    shift-insert: copy from clipboard
//                    shift-left: mark block moving left
//                    shift-right: mark block moving right
//                    shift-up: mark block moving up
//                    shift-down: mark block moving down
//                    shift-backtab: do backtab
//                    shift-character: enter capital letter
//                    shilft-delete: truncate segment
//                    alt-j: join line
//                    alt-s: split line
//                    alt-n: load next lot of segments into editor
//                    alt-u: unmark
//                    up-arrow: move up a line
//                    down-arrow: move down a line
//                    right-arrow: move one char to right
//                    left-arrow: move one char to left
//                    home: move to beginning to line
//                    end: move to end of line
//                    page up: move one page up
//                    page down: move one page down
//                    tab: tab right
//                    delete: if block marked delete block
//                            if positioned on character delete character
//                    backspace: move back one position deleting character
//                    insert: toggle between insert/replace mode
//                    space: empty character
//                    F3: call wm_close and leave editor
//                    F4:save and return to initial tm edit dialog
//                    enter: move to next line
//                    character: enter small letters
//
//                   if !fDefault
//                     update screen
//------------------------------------------------------------------------------
MRESULT HandleTMMWMChar
(
  HWND hwnd,                                       // window handle
  WPARAM mp1,
  LPARAM mp2,
  WINMSG msg
)
{
  MRESULT mResult = MRFROMSHORT(FALSE);// indicate message is processed
  PTBDOCUMENT pTBDoc;                  // pointer to device data
  BOOL             fDefault = FALSE;   // default processing required
  PTMMIDA          pTMMIda;
  USHORT           usRc = NO_ERROR;
  USHORT           usResult = MBID_YES;
  UCHAR            ucCode = NULC;      // key code
  UCHAR            ucState = NULC;     // shift state of key
  BOOL             fUpdate = TRUE;     // update of windows required flag
  BOOL             fDBCSOK = TRUE;
  USHORT           usCode;             // key code

  PTMMVISDOC pVisDoc = ACCESSWNDIDA( hwnd, PTMMVISDOC );

  pTBDoc = pVisDoc->pDoc;


   switch ( msg )
   {
     case WM_KEYDOWN:
       /********************************************************************/
       /* ignore CTRL key if it comes from an extended key...              */
       /********************************************************************/
       if ( mp2 & 0x020000000 )
       {
         ucState = 0;
       }
       else
       {
         fDefault = !EQFBKeyState( SHORTFROMMP1(mp1), &ucCode, &ucState );
         if ( fDefault && (ucCode == '+') )
         {
           fDefault = FALSE;
         } /* endif */
       } /* endif */
       break;
     case WM_SYSKEYDOWN:
       /***************************************************************/
       /* for some reasons F10 arrives as a SYSKEY                    */
       /***************************************************************/
       if (!(mp2 & 0x20000000) )
       {
         fDefault = !EQFBKeyState( SHORTFROMMP1(mp1), &ucCode, &ucState );
       }
       else
       {
         fDefault = !EQFBKeyState( SHORTFROMMP1(mp1), &ucCode, &ucState );
         ucState |= ST_ALT;
       } /* endif */
       break;
     case WM_CHAR:
       ucCode = (UCHAR) LOWORD( mp1 );
       usCode = LOWORD( mp1 );
       ucState = 0;
       fDefault = FALSE;
       if ( (usCode == VK_ENTER )  ||
            (usCode == VK_TAB )    ||
            (usCode == VK_ESC )    ||
            (usCode == VK_BACKSPACE) )
       {
         ucState |= ST_VK;
         if ( (ucCode == VK_TAB) && (GetKeyState(VK_SHIFT) & 0x8000) )
         {
           /**********************************************************/
           /* ignore shift status and set backtab                    */
           /**********************************************************/
           ucCode = VK_BACKTAB;
           ucState  &= ~ST_SHIFT;
         } /* endif */
       } /* endif*/
       /****************************************************************/
       /* refine for dictionary proposal copy, ctrl-+ and ctrl-backspace */
       /* make sure that key is not picked up twice, as character to be*/
       /* typed in also!!                                              */
       /****************************************************************/
       if ( GetKeyState (VK_CTRL) & 0x8000 )
       {
         if ( (usCode <= 'Z' - 'A' + 1) || (usCode == 127)
              || (usCode == 28) || (usCode == 29) )
         {
           fDefault = TRUE;
         } /* endif */
       } /*endif*/
       break;

     case WM_SYSCHAR:
       if ( mp2 & 0x20000000 )
       {
         fDefault = FALSE;
         ucState = ST_ALT;
         ucCode = (UCHAR) LOWORD( mp1 );

         if ( (ucCode == VK_ENTER )  ||
              (ucCode == VK_TAB )    ||
              (ucCode == VK_ESC )    ||
              (ucCode == VK_BACKSPACE) )
         {
           ucState |= ST_VK;
           if ( (ucCode == VK_TAB) && (GetKeyState(VK_SHIFT) & 0x8000) )
           {
             /**********************************************************/
             /* ignore shift status and set backtab                    */
             /**********************************************************/
             ucCode = VK_BACKTAB;
             ucState  &= ~ST_SHIFT;
           } /* endif */
         } /* endif*/
       } /* endif */
       break;
     case WM_SYSDEADCHAR:
     case WM_DEADCHAR:
       ucState = ucCode = 0;
       break;
     default:
       MessageBeep( (UINT) -1 );
       ucState = ucCode = 0;
       break;
   } /* endswitch */

  if ( !fDefault )
  {
    if ( pTBDoc->ulOemCodePage && (msg == WM_CHAR))
    {
        pTBDoc->usChar      = (USHORT)mp1;

    }
    else
    {
       pTBDoc->usChar      = (USHORT)mp1;
       pTBDoc->usDBCS2Char = 0;
       if ((pTBDoc->ucState & ST_CTRL) && (pTBDoc->ucState & ST_SHIFT)
            && ('A' <= pTBDoc->usChar) && (pTBDoc->usChar <= 'Z') )
       {
         pTBDoc->usChar = pTBDoc->usChar + 'a' - 'A';
       } /* endif */
       fDBCSOK = TRUE;
    } /* endif */

  } /* endif */

  if ( !fDefault && fDBCSOK )
  {
    PFUNCTIONTABLE pFuncTab = get_FuncTab();
    if ( ucState & ST_CTRL )  // ctrl key pressed
    {
      if ( ucState & ST_SHIFT )
      {
        switch ( ucCode  )
        {
          case VK_LEFT:
            if ( pTBDoc->twin->pBlockMark )
            {
              // clear mark in inactive window
              EQFBFuncMarkClear ( pTBDoc->twin );
            } /* endif */
            if ( pTBDoc->docType == SSOURCE_DOC )
            {
              EQFBFuncMarkPrevWord( pTBDoc );
            }
            else
            {
              void (*function)( PTBDOCUMENT );
              function = (pFuncTab + MARKPREV_FUNC)->function;
              (*function)(pTBDoc);
            }
            break;
          case VK_RIGHT:
            if ( pTBDoc->twin->pBlockMark )
            {
              // clear mark in inactive window
              EQFBFuncMarkClear ( pTBDoc->twin );
            } /* endif */
            if ( pTBDoc->docType == SSOURCE_DOC )
            {
              EQFBFuncMarkNextWord( pTBDoc );
            }
            else
            {
               void (*function)( PTBDOCUMENT );
               function = (pFuncTab + MARKNEXT_FUNC)->function;
              (*function)(pTBDoc);
            }
            break;
          default :
            fDefault = TRUE;
            break;
        } /* endswitch */
      }
      else
      {
        if ( ucState & ST_VK )  // virtual key valid
        {
          switch ( ucCode )
          {
            case VK_INSERT:
              // Ctrl + Insert copy to clipboard
              EQFBFuncCopyToClip( pTBDoc );
              break;
            case VK_HOME:
              EQFBFuncTopDoc( pTBDoc );
              break;
            case VK_END:
              EQFBFuncBottomDoc( pTBDoc );
              break;
            case VK_LEFT:
              if ( pTBDoc->docType == SSOURCE_DOC)
              {
                EQFBFuncPrevWord( pTBDoc );
              }
              else
              {
                  void (*function)( PTBDOCUMENT );
                  function = (pFuncTab + PREVWORD_FUNC)->function;
                 (*function)(pTBDoc);
              }
              break;
            case VK_RIGHT:
              if ( pTBDoc->docType == SSOURCE_DOC )
              {
                  EQFBFuncNextWord( pTBDoc );
              }
              else
              {
                  void (*function)( PTBDOCUMENT );
                  function = (pFuncTab + NEXTWORD_FUNC)->function;
                  (*function)(pTBDoc);
              }
              break;
            case VK_ENTER:
              //activate next segment, store changes if needed
              pTMMIda = (PTMMIDA)pVisDoc->pTMMIda;
              if ( pTBDoc->tbActSeg.ulSegNum == (pTBDoc->TBCursor).ulSegNum )
              {
                if ( (pTBDoc->tbActSeg).ulSegNum+2
                      < pTBDoc->ulMaxSeg )
                {
                  //activate next segment only if there still are segments
                  usRc = (USHORT)!ActivateSegment( pTMMIda, (USHORT)((pTBDoc->tbActSeg).ulSegNum+2) );
                }
                else
                {
                  //keep last segment active
                  usRc = (USHORT)!ActivateSegment( pTMMIda, pTBDoc->tbActSeg.ulSegNum );
                } /* endif */
              }
              else
              {
                if ( pTBDoc->EQFBFlags.workchng )
                {
                  usResult = UtlError( TB_CHANGESEGMENT, MB_YESNOCANCEL,
                                       0, NULL, EQF_QUERY);
                }
                else
                {
                  usResult = MBID_YES;
                } /* endif */

                if ( usResult == MBID_YES )
                {
                  if ( pTBDoc->TBCursor.ulSegNum < pTBDoc->ulMaxSeg )
                  {
                    if ( (pTBDoc->TBCursor.ulSegNum % 2) == 0 )
                    {
                      // get rid of activating empty segments
                      pTBDoc->TBCursor.ulSegNum+=1;
                      if ( pTBDoc->TBCursor.ulSegNum == pTBDoc->ulMaxSeg )
                      {
                        pTBDoc->TBCursor.ulSegNum-=2;
                      } /* endif */
                    } /* endif */
                    usRc = (USHORT)!ActivateSegment( pTMMIda, pTBDoc->TBCursor.ulSegNum );
                  }
                  else
                  {
                    //keep last segment active
                    usRc = (USHORT)!ActivateSegment( pTMMIda, pTBDoc->tbActSeg.ulSegNum );
                  } /* endif */
                }
                else if ( usResult == MBID_NO )
                {
                  if ( pTBDoc->TBCursor.ulSegNum < pTBDoc->ulMaxSeg )
                  {
                    if ( (pTBDoc->TBCursor.ulSegNum % 2) == 0 )
                    {
                      // get rid of activating empty segments
                      pTBDoc->TBCursor.ulSegNum+=1;
                      if ( pTBDoc->TBCursor.ulSegNum == pTBDoc->ulMaxSeg )
                      {
                        pTBDoc->TBCursor.ulSegNum-=2;
                      } /* endif */
                    } /* endif */
                    usRc = (USHORT)!UnchangedSegment( pTMMIda, pTBDoc->TBCursor.ulSegNum );
                  }
                  else
                  {
                    //keep last segment active
                    usRc = (USHORT)!UnchangedSegment( pTMMIda, pTBDoc->tbActSeg.ulSegNum );
                  } /* endif */
                } /* endif */
              } /* endif */

              if ( usRc )
              {
                //tm error msg so quit editor
                WinPostMsg( pTMMIda->hwnd, WM_CLOSE, NULL, NULL );
              }
              else
              {
                EQFBRefreshScreen( pTBDoc ); // refresh translation
                EQFBRefreshScreen( pTBDoc->twin );   // refresh source

                //reset qstatus for subsequent pulldown functions
                pTMMIda->TBSourceDoc.tbActSeg.qStatus = QF_XLATED;
                pTMMIda->TBTargetDoc.tbActSeg.qStatus = QF_XLATED;
              } /* endif */
              break;
            case  VK_DELETE:
              // Ctrl + Del = delete until end of line
              if ( pTBDoc->docType == STARGET_DOC )
              {
                if ( pTBDoc->TBCursor.ulSegNum == pTBDoc->tbActSeg.ulSegNum )
                {
                  EQFBFuncTruncate( pTBDoc );
                }
                else
                {
                  WinAlarm( HWND_DESKTOP, WA_WARNING );
                  fUpdate = FALSE;
                } /* endif */
              }
              else
              {
                WinAlarm( HWND_DESKTOP, WA_WARNING );
                fUpdate = FALSE;
              } /* endif */
              break;
            default:
              fDefault = TRUE;
              break;
          } /* endswitch */
        }
        else
        {
          switch ( ucCode )
          {
            case  't':
            case  'T':
              pTMMIda = (PTMMIDA)pVisDoc->pTMMIda;
              WinPostMsg( pTMMIda->hwnd, WM_COMMAND,
                          MP1FROMSHORT( ID_TMM_TOGGLEDOC ), NULL );
              break;
            case  'j':
            case  'J':
              pTMMIda = (PTMMIDA)pVisDoc->pTMMIda;
              WinPostMsg( pTMMIda->hwnd, WM_COMMAND,
                          MP1FROMSHORT( ID_TMM_JOIN ),
                          NULL );
              break;
            case  'S':
            case  's':
              pTMMIda = (PTMMIDA)pVisDoc->pTMMIda;
              WinPostMsg( pTMMIda->hwnd, WM_COMMAND,
                          MP1FROMSHORT( ID_TMM_SPLIT ),
                          NULL );
              break;
            case  'N':
            case  'n':
              pTMMIda = (PTMMIDA)pVisDoc->pTMMIda;
              if ( !pTMMIda->pTMIda->fRun && !pTMMIda->pTMIda->fStop )
              {
                WinPostMsg( pTMMIda->hwnd, WM_COMMAND,
                            MP1FROMSHORT( ID_TMM_NEXT ),
                            NULL );
              } /* endif */
              break;
            case  'P':
            case  'p':
              pTMMIda = (PTMMIDA)pVisDoc->pTMMIda;
              if ( !pTMMIda->pTMIda->fRun &&
                   (pTMMIda->pTMIda->ulPrevPos > 0) )
              {
                WinPostMsg( pTMMIda->hwnd, WM_COMMAND,
                            MP1FROMSHORT( ID_TMM_PREVIOUS ),
                            NULL );
              } /* endif */
              break;
            case  'm':
            case  'M':
              // unmark in both windows
              EQFBFuncMarkClear( pTBDoc );
              EQFBFuncMarkClear( pTBDoc->twin );
              break;
            case  'd':
            case  'D':
              pTMMIda = (PTMMIDA)pVisDoc->pTMMIda;
              if ( !pTMMIda->pTMIda->fRun &&
                   (pTBDoc->TBCursor.ulSegNum == pTBDoc->tbActSeg.ulSegNum ) )
              {
                WinPostMsg( pTMMIda->hwnd, WM_COMMAND,
                            MP1FROMSHORT( ID_TMM_DEL_CUR ),
                            NULL );
              }
              else
              {
                // beep when not allowed
                WinAlarm( HWND_DESKTOP, WA_WARNING );
                fUpdate = FALSE;
              } /* endif */
              break;
            case  '-':
              pTMMIda = (PTMMIDA)pVisDoc->pTMMIda;
              if ( !pTMMIda->pTMIda->fRun &&
                   (pTBDoc->TBCursor.ulSegNum == pTBDoc->tbActSeg.ulSegNum ) )
              {
                WinPostMsg( pTMMIda->hwnd, WM_COMMAND,
                            MP1FROMSHORT( ID_TMM_DELM_CUR ),
                            NULL );
              }
              else
              {
                // beep when not allowed
                WinAlarm( HWND_DESKTOP, WA_WARNING );
                fUpdate = FALSE;
              } /* endif */
              break;
            case  '+':
              pTMMIda = (PTMMIDA)pVisDoc->pTMMIda;
              if ( !pTMMIda->pTMIda->fRun &&
                   (pTBDoc->TBCursor.ulSegNum == pTBDoc->tbActSeg.ulSegNum ) )
              {
                WinPostMsg( pTMMIda->hwnd, WM_COMMAND,
                            MP1FROMSHORT( ID_TMM_SETM_CUR ),
                            NULL );
              }
              else
              {
                // beep when not allowed
                WinAlarm( HWND_DESKTOP, WA_WARNING );
                fUpdate = FALSE;
              } /* endif */
              fDefault = FALSE;
              break;
           default:
              fDefault = TRUE;
              break;
          } /* endswitch */
        } /* endif */
      } /* endif */
    }
    else if ((ucState & ST_SHIFT) && (ucState & ST_ALT ))
    {
		switch ( ucCode)
		{
			case VK_HOME:
			    if ( pTBDoc->twin->pBlockMark )
				{
				  // clear mark in inactive window
				  EQFBFuncMarkClear ( pTBDoc->twin );
				} /* endif */
				if ( pTBDoc->docType == SSOURCE_DOC )
				{
					EQFBFuncMarkSegStartCUA( pTBDoc );
				}
				else
				{
					void (*function)( PTBDOCUMENT );
					function = (pFuncTab + MARKSEGSTART_FUNC)->function;
					(*function)(pTBDoc);
                }
			   break;
			case VK_END:
			   if ( pTBDoc->twin->pBlockMark )
				{
				  // clear mark in inactive window
				  EQFBFuncMarkClear ( pTBDoc->twin );
				} /* endif */
				if ( pTBDoc->docType == SSOURCE_DOC )
				{
					EQFBFuncMarkSegEndCUA( pTBDoc );
				}
				else
				{
					void (*function)( PTBDOCUMENT );
					function = (pFuncTab + MARKSEGEND_FUNC)->function;
					(*function)(pTBDoc);
                }
			   break;
			default:
			   break;
	    } /* endswitch */
    }
    else if ( ucState & ST_SHIFT )  // shift key pressed
    {
      if ( ucState & ST_VK )  // virtual key valid
      {
        switch ( ucCode )
        {
           case VK_LEFT:
             if ( pTBDoc->twin->pBlockMark )
             {
               // clear mark in inactive window
               EQFBFuncMarkClear ( pTBDoc->twin );
             } /* endif */
             if ( pTBDoc->docType == SSOURCE_DOC )
             {
                 EQFBFuncMarkLeftCUA( pTBDoc );
             }
             else
             {
                 void (*function)( PTBDOCUMENT );
                 function = (pFuncTab + MARKLEFT_FUNC)->function;
                 (*function)(pTBDoc);
             }
             break;
           case VK_RIGHT:
             if ( pTBDoc->twin->pBlockMark )
             {
               // clear mark in inactive window
               EQFBFuncMarkClear ( pTBDoc->twin );
             } /* endif */
             if ( pTBDoc->docType == SSOURCE_DOC )
             {
                 EQFBFuncMarkRightCUA( pTBDoc );
             }
             else
             {
                void (*function)( PTBDOCUMENT );
                function = (pFuncTab + MARKRIGHT_FUNC)->function;
                (*function)(pTBDoc);
             }
             break;
           case VK_UP:                             //goto previous connection
             if ( pTBDoc->twin->pBlockMark )
             {
               // clear mark in inactive window
               EQFBFuncMarkClear ( pTBDoc->twin );
             } /* endif */
             EQFBFuncMarkUpCUA( pTBDoc );
             break;
           case VK_DOWN:                           //goto next connection
             if ( pTBDoc->twin->pBlockMark )
             {
               // clear mark in inactive window
               EQFBFuncMarkClear ( pTBDoc->twin );
             } /* endif */
             EQFBFuncMarkDownCUA( pTBDoc );
             break;
           case VK_BACKTAB:
             EQFBFuncBacktab( pTBDoc );
             break;
           case  VK_INSERT:
             // Shift + Insert Copy from Clipboard
             if ( pTBDoc->docType == STARGET_DOC )
             {
               EQFBFuncPasteFromClip( pTBDoc );
             }
             else
             {
               WinAlarm( HWND_DESKTOP, WA_WARNING );
             } /* endif */
             break;
           case  VK_DELETE:
             // Shift + Delete Cut to Clipboard
             if ( pTBDoc->docType == STARGET_DOC )
             {
               EQFBFuncCutToClip( pTBDoc );
             }
             else
             {
               WinAlarm( HWND_DESKTOP, WA_WARNING );
             } /* endif */
             break;
           case VK_SHIFT:                // do nothing
           case VK_ESC:                  // do nothing
             break;
           case VK_HOME:
			    if ( pTBDoc->twin->pBlockMark )
				{
				  // clear mark in inactive window
				  EQFBFuncMarkClear ( pTBDoc->twin );
				} /* endif */
				if ( pTBDoc->docType == SSOURCE_DOC )
				{
					EQFBFuncMarkStartCUA( pTBDoc );
				}
				else
				{
					void (*function)( PTBDOCUMENT );
					function = (pFuncTab + MARKSTART_FUNC)->function;
					(*function)(pTBDoc);
                }
			   break;
			case VK_END:
			   if ( pTBDoc->twin->pBlockMark )
				{
				  // clear mark in inactive window
				  EQFBFuncMarkClear ( pTBDoc->twin );
				} /* endif */
				if ( pTBDoc->docType == SSOURCE_DOC )
				{
					EQFBFuncMarkEndCUA( pTBDoc );
				}
				else
				{
					void (*function)( PTBDOCUMENT );
					function = (pFuncTab + MARKEND_FUNC)->function;
					(*function)(pTBDoc);
                }
			   break;
        } /* endswitch */
      }
      else
      {
        //character input by user
        if ( pTBDoc->docType == STARGET_DOC )
        {
          if ( pTBDoc->tbActSeg.ulSegNum == (pTBDoc->TBCursor).ulSegNum )
          {
            EQFBFuncCharacter( pTBDoc );
          }  /* endif */
        }
        else
        {
          fDefault = TRUE;
        } /* endif */
      } /* endif */
    }
    else if ( ucState & ST_ALT )  // alt key pressed
    {
      if ( ucState & ST_VK )  // virtual key valid
      {
        switch ( ucCode )
        {
          case  VK_DELETE:
            // Ctrl + Del = delete until end of line
            if ( pTBDoc->docType == STARGET_DOC )
            {
              if ( pTBDoc->TBCursor.ulSegNum == pTBDoc->tbActSeg.ulSegNum )
              {
                EQFBFuncTruncSeg( pTBDoc );
              }
              else
              {
                WinAlarm( HWND_DESKTOP, WA_WARNING );
                fUpdate = FALSE;
              } /* endif */
            }
            else
            {
              WinAlarm( HWND_DESKTOP, WA_WARNING );
              fUpdate = FALSE;
            } /* endif */
            break;
          default:
            fDefault = TRUE;
            break;
        } /* endswitch */
      }
      else
      {
        switch ( ucCode )
        {
          default:
            fDefault = TRUE;
            break;
        } /* endswitch */
      } /* endif */
    }
    else                                   // no control key
    {
      if ( ucState & ST_VK )  // virtual key valid
      {
        switch ( ucCode  )
        {
          case VK_UP:
            EQFBFuncUp( pTBDoc );
            break;
          case VK_DOWN:
            EQFBFuncDown( pTBDoc );
            break;
          case VK_LEFT:
            if ( pTBDoc->docType == SSOURCE_DOC )
            {
              EQFBFuncLeft( pTBDoc );
            }
            else
            {
              void (*function)( PTBDOCUMENT );
              function = (pFuncTab + LEFT_FUNC)->function;
              (*function)(pTBDoc);
            }
            break;
          case VK_RIGHT:
            if ( pTBDoc->docType == SSOURCE_DOC )
            {
              EQFBFuncRight( pTBDoc );
            }
            else
            {
            //activate BidiLRSwap action in FuncTab
              void (*function)( PTBDOCUMENT );
              function = (pFuncTab + RIGHT_FUNC)->function;
              (*function)(pTBDoc);
            }
            break;
          case VK_HOME:
            EQFBFuncStartLine( pTBDoc );
            break;
          case VK_END:
            EQFBFuncEndLine( pTBDoc );
            break;
          case VK_PAGEUP:
            EQFBFuncPageUp( pTBDoc );
            break;
          case VK_PAGEDOWN:
            EQFBFuncPageDown( pTBDoc );
            break;
          case VK_TAB:
            EQFBFuncTab( pTBDoc );
            break;
          case VK_DELETE:
            {
              PEQFBBLOCK pstBlock;

              if ( pTBDoc->docType == STARGET_DOC )
              {
                pstBlock = (PEQFBBLOCK)pTBDoc->pBlockMark;
                if ( pstBlock->pDoc != NULL )
                {
                  EQFBFuncMarkDelete( pTBDoc );
                }
                else
                {
                  EQFBFuncDeleteChar( pTBDoc );
                } /* endif */
              }
              else
              {
                WinAlarm( HWND_DESKTOP, WA_WARNING );
              } /* endif */
            }
            break;
          case VK_BACKSPACE:
            if ( (pTBDoc->docType == STARGET_DOC) &&
                 ( pTBDoc->TBCursor.ulSegNum == pTBDoc->tbActSeg.ulSegNum ))
            {
              EQFBFuncBackspace( pTBDoc );
            }
            else
            {
              WinAlarm( HWND_DESKTOP, WA_WARNING );
            } /* endif */
            break;
          case VK_INSERT:
            EQFBFuncInsToggle( pTBDoc );
            break;
          case VK_SPACE:
            if ( pTBDoc->docType == STARGET_DOC )
            {
              if ( pTBDoc->tbActSeg.ulSegNum == (pTBDoc->TBCursor).ulSegNum )
              {
                EQFBFuncCharacter( pTBDoc );
              } /* endif */
            } /* endif */
            break;
          case  VK_F4:
            pTMMIda = (PTMMIDA)pVisDoc->pTMMIda;
            WinPostMsg( pTMMIda->hwnd, WM_CLOSE, NULL, NULL );
            break;
          case  VK_F5:
            //save and quit
            pTMMIda = (PTMMIDA)pVisDoc->pTMMIda;
            WinPostMsg( pTMMIda->hwnd, WM_COMMAND,
                        MP1FROMSHORT( ID_TMM_SAVE ),
                        NULL );
            break;
          case VK_ENTER:
            //move down a line in window
            EQFBFuncNextLine( pTBDoc);
            break;
          case VK_ESC:                  // do nothing
            break;
          case VK_F10:
            pTMMIda = (PTMMIDA)pVisDoc->pTMMIda;
            WinPostMsg( pTMMIda->hwndFrame, WM_CHAR, mp1, mp2 );
            break;
        } /* endswitch */
      }
      else
      {
        //character input by user
        if ( pTBDoc->docType == STARGET_DOC )
        {
          if ( pTBDoc->tbActSeg.ulSegNum == (pTBDoc->TBCursor).ulSegNum )
          {
            EQFBFuncCharacter( pTBDoc );
          } /* endif */
        }
        else
        {
          fDefault = TRUE;
        } /* endif */
      } /* endswitch */
    } /* endif */
  } /* endif */

  if ( fDefault )
  {
    mResult = WinDefWindowProc( hwnd, msg, mp1, mp2 );
  }
  else if ( fUpdate )
  {
    EQFBScreenData( pTBDoc );          // display screen
    EQFBScreenData( pTBDoc->twin );    // display screen
    EQFBScreenCursor( pTBDoc );        // update cursor and sliders
    EQFBScreenCursor( pTBDoc->twin );  // update cursor and sliders
  } /* endif */

  return (mResult);
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     SaveSegment
//------------------------------------------------------------------------------
// Function call:     SaveSegment( pTMMIda, pfSaved );
//------------------------------------------------------------------------------
// Description:       save any changes made to translation memory
//------------------------------------------------------------------------------
// Input parameter:   PTMMIDA pTMMIda
// Parameters:        PBOOL   pfSaved
//------------------------------------------------------------------------------
// Output parameter:
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE         succesful
//                    FALSE        error
//------------------------------------------------------------------------------
// Function flow:     if changed flag is set
//                      do worksegout for both docs
//                      get src seg (important for find & change)
//                      remove trailing LFs from source string
//                      remove trailing LFs from target string
//                      if target string is empty after LFs have been removed
//                        add none tag
//                        synchronize src and tgt tags
//                      update segment in translation memory
//                    else
//                      nothing needs to be saved in translation memory
//                      do worksegout for both docs
//------------------------------------------------------------------------------
BOOL SaveSegment
(
  PTMMIDA pTMMIda,                               // pointer to main structure
  PBOOL   pfSaved
)
{
  ULONG  ulSegNum;                             // segnment number
  ULONG  ulPos;
  ULONG  ulLen, ulI;                           // counter
  PTMEXTRAS pTMParms;                          // tm paramters
  PSZ_W pSrcString = NULL;                       // temp buffer
  PSZ_W pTgtString = NULL;                       // temp buffer
  BOOL fOK = TRUE;                             // success indicator
  CHAR_W ch;                                     // character
  PSZ_W pData;                                   // pointer to data string
  PTBDOCUMENT pTargetDoc;                      // pointer to target doc structure
  PTBSEGMENT pSrcSeg;                          // pointer to source segment structure
  PTBSEGMENT pTgtSeg;
  BOOL       fEmptySave = TRUE;

  pTargetDoc = &pTMMIda->TBTargetDoc;

  if ( pTargetDoc->EQFBFlags.workchng )
  {
    //free working segment
    // must be done twice (for changed and not changed)
    // because workchng is set to FALSE in EQFBWorkSegOut
    EQFBWorkSegOut( pTargetDoc );
    EQFBWorkSegOut( pTargetDoc->twin );

    //get source doc segment - this is especially needed for find and change
    //when all action is performed on target doc and pTBSeg is not updated
    //on source doc side
    fOK = GetAndStrip( &pSrcSeg, &pSrcString,
                       pTargetDoc->twin, pTargetDoc->ulWorkSeg );

    if ( fOK )
    {
      fOK = GetAndStrip( &pTgtSeg, &pTgtString,
                         pTargetDoc, pTargetDoc->ulWorkSeg );
    } /* endif */

    if ( fOK )
    {
      if ( pTgtString[0] == EOS )
      {
        pTargetDoc->pEQFBWorkSegmentW[0] = EOS;

        //check if tags are empty ..
        fEmptySave = EQFBDoEmptySegCheck( pTargetDoc );    // check if segment is empty

        if ( fEmptySave )
        {
          *pfSaved = TRUE;
          //update target strings
          memcpy( pTargetDoc->pTBSeg->pDataW, pTargetDoc->pEQFBWorkSegmentW,
                  UTF16strlenBYTE(EMPTY_TAG)+2 );
          pTargetDoc->pTBSeg->usLength = (USHORT)(UTF16strlenCHAR(EMPTY_TAG) + 1);
          pTargetDoc->pTBSeg->qStatus = QF_NOP;
          UTF16strcpy( pTgtString, pTargetDoc->pTBSeg->pDataW );

          //update source string
          UTF16strcpy( pTargetDoc->twin->pTBSeg->pDataW, pSrcString );
          pTargetDoc->twin->pTBSeg->usLength = (USHORT)
                     ( UTF16strlenCHAR(pTargetDoc->twin->pTBSeg->pDataW)+1);
          //calculate no of linefeeds in source seg to add to target
          //for synchronization
          pData = pSrcString;
          ulLen = 0;
          while ( (ch = *pData) != NULC )
          {
            if ( ch == '\n' )
            {
              ulLen++;
            } /* endif */
            pData ++;
          } /* endwhile */

          //if required add linefeeds to target after the none flag to align docs
          for ( ulI=0; ulI<ulLen; ulI++ )
          {
            ulPos = UTF16strlenCHAR( pTargetDoc->pTBSeg->pDataW );
            *(pTargetDoc->pTBSeg->pDataW+ulPos) = LF;
            *(pTargetDoc->pTBSeg->pDataW+ulPos+1) = EOS;
            pTargetDoc->pTBSeg->usLength++;
          } /* endfor */

          pTargetDoc->EQFBFlags.workchng = TRUE;
          pTargetDoc->Redraw |= REDRAW_ALL;          // redraw the screen
          pTargetDoc->pTBSeg->SegFlags.Typed = TRUE;

          //mode readjust screen start positions
          EQFBScrnLinesFromSeg ( pTargetDoc,        // pointer to doc ida
                                 0,                 // starting row
                                 pTargetDoc->lScrnRows,  // number of rows
                                 (pTargetDoc->TBRowOffset+1)); // starting segment
        }
        else
        {
          *pfSaved = FALSE;
        } /* endif */
      } /* endif */
    } /* endif */

    //update translation memory
    if ( fOK && fEmptySave )
    {
      //get position in parameters structure
      ulSegNum = (pTargetDoc->ulWorkSeg - 1) / 2;
      pTMParms = pTMMIda->pTMParms + ulSegNum;

      //update segment from tm
      fOK  = TMMReplace( pTMMIda->pTMIda,
                         pSrcString,         //source segment
                         pTgtString,         //target segment
                         pTMMIda->pTMIda->szFileName,  //tm name
                         pTMMIda->pTMIda->pMemory,    //tm handle
                         pTMParms );         //seg no & doc name
    } /* endif */
  }
  else
  {
    //free working segment
    EQFBWorkSegOut( pTargetDoc );
    EQFBWorkSegOut( pTargetDoc->twin );
  } /* endif */

  if ( pSrcString )
    UtlAlloc( (PVOID *)&(pSrcString), 0L, 0L, NOMSG );
  if ( pTgtString )
    UtlAlloc( (PVOID *)&(pTgtString), 0L, 0L, NOMSG );

  return( fOK );

} /* end of function SaveSegment */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     DeleteSegments
//------------------------------------------------------------------------------
// Function call:     DeleteSegments( pTMMIda )
//------------------------------------------------------------------------------
// Description:       delete a segment from the translation memory
//------------------------------------------------------------------------------
// Input parameter:   PTMMIDA pTMMIda
// Parameters:
//------------------------------------------------------------------------------
// Output parameter:
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       0 - NO_ERROR, all others are errors
//------------------------------------------------------------------------------
// Function flow:       do worksegout for both docs
//                      get src seg
//                      remove trailing LFs from source string
//                      remove trailing LFs from target string
//                      delete segment in translation memory
//------------------------------------------------------------------------------

BOOL DeleteSegments
(
  PTMMIDA pTMMIda                          // pointer to main structure
)
{
  ULONG  ulSegNum;                             // segnment number
  PTMEXTRAS pTMParms;                          // tm paramters
  PSZ_W  pSrcString = NULL;                      // temp buffer
  PSZ_W  pTgtString = NULL;                      // temp buffer
  PTBDOCUMENT pTargetDoc;                      // pointer to target doc structure
  PTBSEGMENT pSrcSeg;                          // pointer to source segment structure
  PTBSEGMENT pTgtSeg;                          // pointer to target segment structure
  BOOL   fOK = TRUE;

  pTargetDoc = &pTMMIda->TBTargetDoc;

  //get source doc segment - this is especially needed for find and change
  //when all action is performed on target doc and pTBSeg is not updated
  //on source doc side
  fOK = GetAndStrip( &pSrcSeg, &pSrcString,
                     pTargetDoc->twin, pTargetDoc->ulWorkSeg );

  if ( fOK )
  {
    fOK = GetAndStrip( &pTgtSeg, &pTgtString,
                       pTargetDoc, pTargetDoc->ulWorkSeg );
  } /* endif */

  if ( fOK )
  {
    //update or delete from translation memory
    if ( pTMMIda->fDelSeg )
    {
      //get position in parameters structure
      ulSegNum = (pTargetDoc->ulWorkSeg - 1) / 2;
      pTMParms = pTMMIda->pTMParms + ulSegNum;

      //delete segment from tm
      fOK = TMMDelete( pTMMIda->pTMIda,
                        pSrcString,                   //source segment
                        pTgtString,                //target segment
                        pTMMIda->pTMIda->szFileName,   //tm name
                        pTMMIda->pTMIda->pMemory,     //tm handle
                        pTMParms );                   //seg no & doc name
    }/* endif */
  } /* endif */

  if ( pSrcString )
    UtlAlloc( (PVOID *)&(pSrcString), 0L, 0L, NOMSG );
  if ( pTgtString )
    UtlAlloc( (PVOID *)&(pTgtString), 0L, 0L, NOMSG );


  return( fOK );

} /* end of function DeleteSegments */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     SubtractLineNum
//------------------------------------------------------------------------------
// Function call:     SubtractLineNum( pTBDoc, pData )
//------------------------------------------------------------------------------
// Description:       set usmaxline back if LF is encountered in string
//------------------------------------------------------------------------------
// Input parameter:   PTBDOCUMENT pTBDoc
// Parameters:        PSZ pData
//------------------------------------------------------------------------------
// Output parameter:
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       VOID
//------------------------------------------------------------------------------
// Function flow:     if an LF is encountered in the string
//                    set back usMaxLine
//------------------------------------------------------------------------------
VOID  SubtractLineNum
(
  PTBDOCUMENT pTBDoc,              // pointer to document
  PSZ_W         pData                // pointer to data
)
{
  CHAR_W  ch;

  while ( (ch = *pData) != NULC )
  {
    if ( ch == '\n')
    {
      pTBDoc->ulMaxLine--;
    } /* endif */
    pData ++;
  } /* endwhile */
} /* end of function SubtractLineNum */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UpdateSegTable
//------------------------------------------------------------------------------
// Function call:     UpdateSegTable( pDoc )
//------------------------------------------------------------------------------
// Description:       update segment table when a segment is deleted
//------------------------------------------------------------------------------
// Input parameter:   PTBDOCUMENT pTBDoc
// Parameters:
//------------------------------------------------------------------------------
// Output parameter:
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       VOID
//------------------------------------------------------------------------------
// Function flow:     get segment to delete
//                    check for linefeeds and adjust usmaxline
//                    free old segment in segment table
//                    move up all subsequent segments two positions in segment
//                      table
//                    get last double spacing and free this too
//                    update segment table counters to reflect the deletions
//                    if there is only one segment left ypu can assume that
//                      it is the initial dummy one so delete this too
//                      free all remaining segment table memory
//------------------------------------------------------------------------------
VOID UpdateSegTable
(
  PTBDOCUMENT pDoc                     // pointer to document ida
)
{
  PTBSEGMENT pOldSeg = NULL;                     // pointer to segment
  PTBSEGMENT pNewSeg = NULL;                     // pointer to segment
  PTBSEGMENT pSeg = NULL;                        // pointer to segment
  ULONG  ulSegNum = pDoc->tbActSeg.ulSegNum;     // seg num of seg to be deleted
  ULONG  ulOldSeg;                               // seg num of lastgot seg
  ULONG  ulJ;                                    // for loop counter
  PTBSEGMENTTABLE pSegTable;                     // pointer to segment table
  ULONG  ulSegTables;                            // no of seg tables to process

  //get segment that is to be deleted
  pOldSeg = EQFBGetSegW( pDoc, ulSegNum );

  //check for linefeeds and subtract from usmaxline - also for the deleted
  //double spacing
  SubtractLineNum( pDoc, pOldSeg->pDataW );
  SubtractLineNum( pDoc, tb2NewLineSegment.pDataW );

  //free segment before updating segment table
  if ( pOldSeg->pDataW )
  {
    UtlAlloc( (PVOID *)&pOldSeg->pDataW, 0L, 0L, NOMSG );
  } /* endif */
   if ( pOldSeg->pData )
    {
      UtlAlloc( (PVOID *)&pOldSeg->pData, 0L, 0L, NOMSG );
  } /* endif */
  if ( pOldSeg->pusBPET )
  {
    UtlAlloc( (PVOID *)&pOldSeg->pusBPET, 0L, 0L, NOMSG );
  } /* endif */
  if (pOldSeg->pusHLType) UtlAlloc((PVOID *)&(pOldSeg->pusHLType),0L,0L,NOMSG);

  //start at seg num of segment to be deleted, break off at maxseg-2 - the
  //deleted segment and the double spacing segment, jump omitting the double
  //spacing segment
  for ( ulJ = ulSegNum; ulJ < pDoc->ulMaxSeg-2; ulJ+=2 )
  {
    //get next text segment
    pNewSeg = EQFBGetSegW( pDoc, (ulJ+2) );

    //remember old seg no
    ulOldSeg = pOldSeg->ulSegNum;

    //copy segment one up from its original position
    memcpy( pOldSeg, pNewSeg, sizeof( TBSEGMENT ) );

    //change seg numbers
    pOldSeg->ulSegNum = ulOldSeg;

    //switch pointers for next round
    pOldSeg = pNewSeg;
  } /* endfor */

  //update psegtables so that the last two entries are erased
  pSegTable = pDoc->pSegTables;
  ulSegTables = pDoc->ulSegTables;

  //find last table
  while ( ulSegTables && ( pOldSeg->ulSegNum >= pSegTable->ulSegments) )
  {
     pOldSeg->ulSegNum -= pSegTable->ulSegments;
     ulSegTables--;
     pSegTable++;
  } /* endwhile */

  //get last one (double spacing) and free it
  pSeg = pSegTable->pSegments + pSegTable->ulSegments-1 ;
  if ( pSeg->pDataW )
    UtlAlloc( (PVOID *)&pSeg->pDataW, 0L, 0L, NOMSG );
  if ( pSeg->pData )
    UtlAlloc( (PVOID *)&pSeg->pData, 0L, 0L, NOMSG );
  if ( pSeg->pusBPET )
    UtlAlloc( (PVOID *)&pSeg->pusBPET, 0L, 0L, NOMSG );
  if (pSeg->pusHLType) UtlAlloc((PVOID *)&(pSeg->pusHLType),0L,0L,NOMSG);
  if (pSeg->pContext) UtlAlloc((PVOID *)&(pSeg->pContext),0L,0L,NOMSG);

  if ( pSegTable->ulSegments )
  {
    pSegTable->ulSegments -= 2;        //take account of both deletes

    //adjust maxseg
    pDoc->ulMaxSeg -= 2;

    //if nothing left in segtable -- get rid of of initial entry and then
    //the table
    if ( pSegTable->ulSegments == 1 )
    {
      //nothing left in segtable -- get rid of of initial entry and then
      //the table
      pSeg = pSegTable->pSegments + pSegTable->ulSegments-1 ;
      if ( pSeg->pData )
        UtlAlloc( (PVOID *)&pSeg->pData, 0L, 0L, NOMSG );
      if ( pSeg->pDataW )
        UtlAlloc( (PVOID *)&pSeg->pDataW, 0L, 0L, NOMSG );
      if ( pSeg->pusBPET )
        UtlAlloc( (PVOID *)&pSeg->pusBPET, 0L, 0L, NOMSG );
      if (pSeg->pusHLType) UtlAlloc((PVOID *)&(pSeg->pusHLType),0L,0L,NOMSG);

      if (pSeg->pContext) UtlAlloc((PVOID *)&(pSeg->pContext),0L,0L,NOMSG);

      pSegTable->ulSegments --;
      pDoc->ulSegTables--;
      pDoc->ulMaxSeg --;
      UtlAlloc( (PVOID *)&pDoc->pSegTables, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */
} /* end of function UpdateSegTable */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TMMReplace( pSourceSeg, pTargetSeg, pTMName, hTM,
//                                pTMParms )
//------------------------------------------------------------------------------
// Function call:     TMMReplace(
//                             PSZ pSourceSeg,          //source segment
//                             PSZ pTargetSeg,          //target segment
//                             PSZ pTMName,             //tm name
//                             HTM hTM,                 //tm handle
//                             PTMEXTRAS pTMParms       //seg no & doc name
//------------------------------------------------------------------------------
// Description:       replace the segment in the translation memory
//------------------------------------------------------------------------------
// Parameters:                 PSZ pSourceSeg,          //source segment
//                             PSZ pTargetSeg,          //target segment
//                             PSZ pTMName,             //tm name
//                             HTM hTM,                 //tm handle
//                             PTMEXTRAS pTMParms       //seg no & doc name
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       all others           error message from TM
//                    NO_ERROR - 0         everything went okay
//------------------------------------------------------------------------------
// Function flow:     allocate replace structures
//                    setup prefix for IN
//                    setup segment
//                    call TmReplace for doing the Replace call
//                    return returncode
//------------------------------------------------------------------------------
BOOL  TMMReplace
(
  PTMIDA pTMIda,          // TMM ida
  PSZ_W pSourceSeg,          //source segment
  PSZ_W pTargetSeg,          //target segment
  PSZ pTMName,             //tm name
  OtmMemory *pMemory,      // memory
  PTMEXTRAS pTMParms      //seg no & doc name
)
{
  USHORT usRc;
  BOOL             fOK;

  OtmProposal *pProposal = new OtmProposal();

  if ( fOK )
  {
    if ( IsDBCS_CP(pTMIda->ulSystemPrefCP) )
    {
      ULONG i;
      i = UTF16strlenCHAR( pSourceSeg ) + 1;
      EQFBUtlConvertSOSI( pTMIda->szBufferW, &i, pSourceSeg, DELETE_SOSI, pTMIda->ulSystemPrefCP );
      TMMCorrectLFCount( pTMIda->szBufferW, pTMParms->sSourceLF );
      pProposal->setSource( pTMIda->szBufferW );


      i = UTF16strlenCHAR( pTargetSeg ) + 1;
      EQFBUtlConvertSOSI( pTMIda->szBufferW, &i, pTargetSeg, DELETE_SOSI, pTMIda->ulSystemPrefCP );
      TMMCorrectLFCount( pTMIda->szBufferW , pTMParms->sTargetLF );
      pProposal->setTarget( pTMIda->szBufferW );

    }
    else
    {
      UTF16strcpy( pTMIda->szBufferW, pSourceSeg );
      TMMCorrectLFCount( pTMIda->szBufferW, pTMParms->sSourceLF );
      pProposal->setSource( pTMIda->szBufferW );
      UTF16strcpy(  pTMIda->szBufferW, pTargetSeg);
      TMMCorrectLFCount( pTMIda->szBufferW , pTMParms->sTargetLF );
      pProposal->setTarget( pTMIda->szBufferW );
    } /* endif */

    // correct number of LFs at end of segment data (alignment
    // functions may have tampered with them)
    TMMGetNameForInd( pTMIda, pTMParms->sSourceDocInd, pTMIda->szBuffer, TMM_DOCUMENTS );
    pProposal->setDocShortName( pTMIda->szBuffer );
    TMMGetNameForInd( pTMIda, pTMParms->sLongNameInd, pTMIda->szBuffer, TMM_LONGNAMES );
    pProposal->setDocName( pTMIda->szBuffer );
    TMMGetNameForInd( pTMIda, pTMParms->sTagTableInd, pTMIda->szBuffer, TMM_TAGTABLES );
    pProposal->setMarkup( pTMIda->szBuffer );
    pProposal->setSourceLanguage( pTMIda->szSourceLng );
    TMMGetNameForInd( pTMIda, pTMParms->sTargetLangInd, pTMIda->szBuffer, TMM_LANGUAGES );
    pProposal->setTargetLanguage( pTMIda->szBuffer );
    pProposal->setSegmentNum( pTMParms->ulTmSegNum );
    switch ( pTMParms->usTranslationFlag )
    {
      case  TRANSLFLAG_NORMAL: pProposal->setType( OtmProposal::eptManual ); break;
      case  TRANSLFLAG_GLOBMEM: pProposal->setType( OtmProposal::eptGlobalMemory ); break;
      case  TRANSLFLAG_MACHINE: pProposal->setType( OtmProposal::eptMachine ); break;
      default: pProposal->setType( OtmProposal::eptManual ); break;
    } /* endswitch */
    if ( pTMParms->pszContext ) 
    {
      pProposal->setContext( pTMParms->pszContext );
    } /* endif */

    usRc = (USHORT)pMemory->putProposal( *pProposal );
    if ( usRc != NO_ERROR )
    {
      TMM_Error( usRc, UtlGetFnameFromPath(pTMName), EMPTY_STRING );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  delete( pProposal );

  return( fOK );
} /* end of function TMMReplace */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TMMDelete( pSourceSeg, pTargetSeg, pTMName, hTM,
//                               pTMParms )
//------------------------------------------------------------------------------
// Function call:     TMMDelete(
//                             PSZ pSourceSeg,          //source segment
//                             PSZ pTargetSeg,          //target segment
//                             PSZ pTMName,             //tm name
//                             HTM hTM,                 //tm handle
//                             PTMEXTRAS pTMParms       //seg no & doc name
//------------------------------------------------------------------------------
// Description:       delete segment from translation memory
//------------------------------------------------------------------------------
// Parameters:                 PSZ pSourceSeg,          //source segment
//                             PSZ pTargetSeg,          //target segment
//                             PSZ pTMName,             //tm name
//                             HTM hTM,                 //tm handle
//                             PTMEXTRAS pTMParms       //seg no & doc name
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       all others           error message from TM
//                    NO_ERROR - 0         everything went okay
//------------------------------------------------------------------------------
// Function flow:     allocate replace structures
//                    setup segment
//                    call TmDelete for doing the delete call; set return code
//                    return returncode
//------------------------------------------------------------------------------
BOOL  TMMDelete
(
  PTMIDA pTMIda,
  PSZ_W pSourceSeg,          //source segment
  PSZ_W pTargetSeg,          //target segment
  PSZ pTMName,             //tm name
  OtmMemory *pMemory,      // memory
  PTMEXTRAS pTMParms       //seg no & doc name
)
{
  USHORT         usRc;
  BOOL           fOK = TRUE;

  OtmProposal *pProposal = new OtmProposal();

  if ( fOK )
  {
    UTF16strcpy( pTMIda->szBufferW, pSourceSeg );
    TMMCorrectLFCount( pTMIda->szBufferW, pTMParms->sSourceLF );
    pProposal->setSource( pTMIda->szBufferW );
    UTF16strcpy(  pTMIda->szBufferW, pTargetSeg);
    TMMCorrectLFCount( pTMIda->szBufferW , pTMParms->sTargetLF );
    pProposal->setTarget( pTMIda->szBufferW );

    TMMGetNameForInd( pTMIda, pTMParms->sSourceDocInd, pTMIda->szBuffer, TMM_DOCUMENTS );
    pProposal->setDocShortName( pTMIda->szBuffer );
    TMMGetNameForInd( pTMIda, pTMParms->sLongNameInd, pTMIda->szBuffer, TMM_LONGNAMES );
    pProposal->setDocName( pTMIda->szBuffer );
    TMMGetNameForInd( pTMIda, pTMParms->sTagTableInd, pTMIda->szBuffer, TMM_TAGTABLES );
    pProposal->setMarkup( pTMIda->szBuffer );
    pProposal->setSourceLanguage( pTMIda->szSourceLng );
    TMMGetNameForInd( pTMIda, pTMParms->sTargetLangInd, pTMIda->szBuffer, TMM_LANGUAGES );
    pProposal->setTargetLanguage( pTMIda->szBuffer );
    pProposal->setSegmentNum( pTMParms->ulTmSegNum );
    switch ( pTMParms->usTranslationFlag )
    {
      case  TRANSLFLAG_NORMAL: pProposal->setType( OtmProposal::eptManual ); break;
      case  TRANSLFLAG_GLOBMEM: pProposal->setType( OtmProposal::eptGlobalMemory ); break;
      case  TRANSLFLAG_MACHINE: pProposal->setType( OtmProposal::eptMachine ); break;
      default: pProposal->setType( OtmProposal::eptManual ); break;
    } /* endswitch */

    if ( pTMParms->pszContext ) 
    {
      pProposal->setContext( pTMParms->pszContext );
    } /* endif */

    usRc = pMemory->deleteProposal( *pProposal );
    if ( usRc == SEG_NOT_FOUND )
    {
      UtlError( ERROR_TMM_DELERR, MB_CANCEL, 0, NULL, EQF_ERROR );
      fOK = FALSE;
    }
    else if ( usRc != NO_ERROR )
    {
      TMM_Error( usRc, UtlGetFnameFromPath(pTMName), EMPTY_STRING );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* free allocated resources                                         */
  /********************************************************************/
  delete( pProposal );

  return( fOK );

} /* end of function TMMDelete */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TM_Error
//------------------------------------------------------------------------------
// Function call:     TMM_ERROR( usErr, pMemName, pServer );
//------------------------------------------------------------------------------
// Description:       map TM error to EQF error msg number
//------------------------------------------------------------------------------
// Parameters:        USHORT usErr  - error code returned by tm function
//                    PSZ pMemName  - pointer to tm name
//                    PSZ pServer   - pointer to server name
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       VOID
//------------------------------------------------------------------------------
// Function flow:     map the TM error to EQF error
//                    fill message buffer with transl.memory name/server name
//                    issue message
//------------------------------------------------------------------------------
VOID TMM_Error
(
  USHORT usErr,                        //transl. memory error code
  PSZ pMemName,                        //tm name
  PSZ pServer                          //server name
)
{
  USHORT usErrID;                      // error id to be set
  BOOL fMemName = TRUE;                // mem file name or server name
  PSZ pszMsgTable[2];                  // error msg parameter list

  switch (usErr)
  {
    case DISK_FULL:
         usErrID = ERROR_MEM_DISK_FULL;
         break;
    case DB_FULL:
         usErrID = ERROR_MEM_DB_FULL;
         break;
    case SEG_NOT_FOUND:
         usErrID = ERROR_TMM_DELERR;
         break;
    case TM_FILE_NOT_FOUND:
         usErrID = EQFS_TM_NOT_FOUND;
         break;
    case FILE_ALREADY_OPEN:
         usErrID = ERROR_MEM_IN_EXCL_USE;
         break;
    case CORRUPTION_FLAG_ON:
    case FILE_MIGHT_BE_CORRUPTED:
         usErrID = ERROR_MEM_MIGHT_BE_CORRUPTED;
         break;
    case VERSION_MISMATCH:
    case CORRUPT_VERSION_MISMATCH:
         usErrID = ERROR_MEM_VERSION_MISMATCH;
         break;
    case NOT_REPLACED_OLD_SEGMENT:
         usErrID = ERROR_MEM_NOT_REPLACED;
         break;
    default:
          usErrID = ERROR_MEM_UNDEFINED;
          break;
  }
  /********************************************************************/
  /* display tm name                                                  */
  /********************************************************************/
  if ( fMemName )
  {
    CHAR szRC[20];                     // string representation of usErr
    CHAR szMemName[MAX_FNAME];         // buffer for TM name

    Utlstrccpy( szMemName, pMemName, DOT );
    itoa( usErr, szRC, 10 );
    pszMsgTable[0] = szMemName;
    pszMsgTable[1] = szRC;
    UtlError( usErrID, MB_CANCEL, 2, pszMsgTable, EQF_ERROR );
  }
  else
  {
    /******************************************************************/
    /* display server name                                            */
    /******************************************************************/
    CHAR ch[2];
    ch[0] = pMemName[0];
    ch[1] = EOS;
    pszMsgTable[0] = ch;
    pszMsgTable[1] = pServer;
    UtlError( usErrID, MB_CANCEL, 2, pszMsgTable, EQF_ERROR );
  } /* endif */
} /* end of function TMMError */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TMEditWork
//------------------------------------------------------------------------------
// Function call:     TMEditWork ( pDoc)
//------------------------------------------------------------------------------
// Description:       calls the save segment function for find and change
//------------------------------------------------------------------------------
// Input parameter:   PTBDOCUMENT pDoc
// Parameters:
//------------------------------------------------------------------------------
// Output parameter:
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       BOOL - true/false
//------------------------------------------------------------------------------
// Function flow:     if the postedit flag is set - e.g. for find and change
//                      all the SaveSegment function for the TM Edit
//                      function and not the editor save function
//------------------------------------------------------------------------------
BOOL APIENTRY TMEditWork
(
  PTBDOCUMENT pDoc      //pointer to main structure
)
{
  BOOL fOK = TRUE;                     //success indicator
  BOOL fDummy;

  if ( pDoc->EQFBFlags.PostEdit && pDoc->TBCursor.ulSegNum != pDoc->ulWorkSeg)
  {
    fOK = SaveSegment( (PTMMIDA)pDoc->pTMMaint, &fDummy );
  } /* endif */

  return( fOK );

} /* end of function TMEditWork */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     DeleteAllSegments
//------------------------------------------------------------------------------
// Function call:     DeleteAllSegments( pTMMIda )
//------------------------------------------------------------------------------
// Description:       delete all segment in the segment table(s)
//------------------------------------------------------------------------------
// Input parameter:   PTMMIDA pTMMIda
// Parameters:
//------------------------------------------------------------------------------
// Output parameter:
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       USHORT - NO_ERROR (0) shows success
//------------------------------------------------------------------------------
// Function flow:     move through the segment table(s) beginning at the first
//                    segment and calling up the delete segment function
//                    omit the double spacing segments
//------------------------------------------------------------------------------
BOOL DeleteAllSegments
(
  PTMMIDA pTMMIda                          // pointer to main structure
)
{
  PTBDOCUMENT pTargetDoc;              //pointer to target doc structure
  BOOL        fOK = TRUE;
  USHORT      ulSegNum = 1;

  pTMMIda->fDelSeg = TRUE;   //set del flag
  pTargetDoc = &pTMMIda->TBTargetDoc;
  while ( fOK && (ulSegNum < pTargetDoc->ulMaxSeg) )
  {
    pTargetDoc->ulWorkSeg = ulSegNum;
    fOK = DeleteSegments( pTMMIda );
    ulSegNum += 2;
  } /* endwhile */

  return( fOK );

} /* end of function DeleteAllSegments */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     GetAndStrip
//------------------------------------------------------------------------------
// Function call:     fOK = GetAndStrip( ppTBSeg, ppString,
//                          pTBDoc, ulWorkSeg );
//------------------------------------------------------------------------------
// Description:       gets a segment and strips LFs
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       fOK
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------
BOOL GetAndStrip
(
   PTBSEGMENT *    ppTBSeg,
   PSZ_W *         ppString,
   PTBDOCUMENT     pTBDoc,
   ULONG           ulWorkSeg
)
{
   ULONG           ulPos = 0;
   BOOL            fOK = TRUE;

   *ppTBSeg = EQFBGetSegW( pTBDoc, ulWorkSeg );

   //write source pdata to structure for replace
   ulPos = UTF16strlenCHAR( (*ppTBSeg)->pDataW );
   fOK = UtlAlloc( (PVOID *)ppString, 0L,
                   max( (ULONG)MIN_ALLOC, (ULONG)(ulPos + 1) * sizeof(CHAR_W)),
                   ERROR_STORAGE );

   if ( fOK )
   {
     UTF16strcpy( *ppString, (*ppTBSeg)->pDataW );
     //remove any trailing linefeeds added for alignment purposes
     while ( (ulPos != 0) && (*(*ppString + ulPos - 1) == LF) )
     {
       ulPos--;
     } /* endwhile */
     *(*ppString + ulPos) = EOS;
   } /* endif */

   return( fOK );

} /* end of function GetAndStrip */

