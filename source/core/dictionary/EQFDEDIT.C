//+----------------------------------------------------------------------------+
//|EQFDEDIT.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2015, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:   G. Queck (QSoft)                                                  |
//|          based on code by Sotec                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|          Edit of a dictionary entry                                        |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
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
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#include <eqf.h>                  // General Translation Manager include file

#undef _WPTMIF                         // we don't care about WP I/F
#include "eqfhelp.id"                  // help resource IDs
#include "eqfhlp1.h"                   // first part of help tables
#include "eqfmsg.htb"                          // message help table


#include "eqfdasdi.h"
#include "OtmDictionaryIF.H"
#include "eqflp.h"                // Defines for generic list handlers
#define EQFLIST_C                 // force initialization of global data
#include <EQFLISTI.H>             // private list handler defines
#include "EQFDEDIT.H"             // internal edit dialog header file
#include "EQFDDLG.ID"             // dialog IDs
#include <time.h>

MRESULT VScroll( HWND hwnd, LONG  lId, SHORT sNotification, LONG lScrollPos );
MRESULT EditChar( HWND, SHORT );
MRESULT EditControl( HWND, SHORT, SHORT );
MRESULT EditCommand( HWND, SHORT, SHORT );
MRESULT EditClose( HWND hwnd, WPARAM mp1, LPARAM mp2 );
BOOL    EditGetEntry( PEDITIDA pIda );
BOOL    EditCalcDlgControls( PEDITIDA pIda );

LRESULT CALLBACK EditSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

extern HELPSUBTABLE hlpsubtblEditEntryDlg[];

static BOOL CheckLanguages ( PEDITIDA pIda );
static BOOL ReplaceWithUnicodeField
( 
  HWND hwndDlg,                        // window handle of dialog window
  int  iEntryFieldID                   // ID of entry field
);


/*******************************************************************************
*
*       function:       EditEntryDlgProc
*
*******************************************************************************/

INT_PTR CALLBACK EditEntryDlgProc (HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
  MRESULT    mResult = FALSE;
  PEDITIDA   pIda;          // the dialog structure contains static information
  // used at many locations in the module

  switch (msg)
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( EDIT_ENTRY_DLG, mp2 ); break;


    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblEditEntryDlg[0] );
      mResult = TRUE;  // message processed
      break;


    case WM_INITDLG:
      pIda = (PEDITIDA )mp2;
      UtlAlloc( (PVOID *)&pIda->pStringBuffer, 0L, (LONG)MAX_ALLOC, ERROR_STORAGE );
      if ( !pIda->pStringBuffer )
      {
        WinDismissDlg( hwnd, FALSE );
      }
      else
      {
        ANCHORDLGIDA( hwnd, pIda );
        pIda->hwndDlg = hwnd;


        {
          USHORT usDummy;
          UtlGetLANUserIDW( pIda->szUserIDW, &usDummy, FALSE );
        }
        ReplaceWithUnicodeField( hwnd, EDIT_ENTRY_EDIT_HEADWORD );

        // create font to be used for the controls
        {
          // this code has been borrowed from function SetCtrlFont in file EQFOSWIN.C
          HFONT   hFontDlg;

          hFontDlg = (HFONT)SendMessage( hwnd, WM_GETFONT, 0, 0L );
          if ( hFontDlg != NULL )
          {
            LOGFONT lFont;
            if ( GetObject( hFontDlg, sizeof(LOGFONT), (LPSTR) &lFont ))
            {
              lFont.lfCharSet  = (UCHAR)GetCharSet();
              if (lFont.lfHeight > 0 )
              {
                lFont.lfHeight -=  SHEIGHTINCTRL;
              }
              else
              {
                lFont.lfWeight = FW_NORMAL;
              } /* endif */
              lFont.lfOutPrecision = OUT_TT_PRECIS;
              pIda->hFontControl = CreateFontIndirect( &lFont );
            }
          }
        }

        if ( EditCalcDlgControls( pIda ) )
        {
          HMODULE hResMod;
          hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
          LOADSTRING( (HAB)UtlQueryULong( QL_HAB), hResMod, SID_EDIT_ENTRY_TEMPLATE, pIda->szTemplatePattern );

          InitEdit (hwnd, pIda);
          if (pIda->pControls) // first control
            SETFOCUSHWND( pIda->hwndHeadword );
          pIda->fDestroy = FALSE; // set by WM_CLOSE to TRUE to avoid multiple // destroys
          FillDictCombo (pIda);
          SETTEXTLIMIT( hwnd, EDIT_ENTRY_EDIT_HEADWORD, MAX_TERM_LEN );
          if ( pIda->hFontControl != NULL ) SendDlgItemMessage( hwnd, EDIT_ENTRY_EDIT_HEADWORD, WM_SETFONT, (WPARAM)pIda->hFontControl, MP2FROMSHORT(TRUE) );
          pIda->hwndSelection = pIda->hwndHeadword;
          WinStartTimer( WinQueryAnchorBlock(hwnd), 
                         hwnd,
                         100,       // timer ID
                         500L );    // timeout time in milliseconds
        }
        else
        {
          WinDismissDlg( hwnd, FALSE );
        } /* endif */
      } /* endif */
      break;


    case WM_ACTIVATE:
      if (mp1)
      {
        pIda = ACCESSDLGIDA( hwnd, PEDITIDA );
        ENABLECTRLHWND( pIda->hwndPaste, CheckClipbrdData( hwnd ) );
      } /* endif */
      mResult = WinDefDlgProc (hwnd, msg, mp1, mp2);
      break;

    case WM_VSCROLL:
      {
        LONG lScrollbarID = GetDlgCtrlID( (HWND)mp2 );
        mResult = VScroll( hwnd, lScrollbarID, LOWORD(mp1),
                           HIWORD(mp1) );
      }
      return( mResult );

    case WM_EQF_SETFOCUS:
      SETFOCUSHWND( (HWND)mp1 );
      return 0;


    case DM_GETDEFID:
      if ( GetKeyState(VK_TAB) & 0x8000  )
      {
        SHORT sTest;
        sTest = 0;
      } /* endif */
      break;


      /******************************************************************/
      /* On WM_TIMER update the clipboard buttons Copy and Paste        */
      /******************************************************************/
    case WM_TIMER:
      pIda = ACCESSDLGIDA( hwnd, PEDITIDA );
      CheckSelection ( pIda );
      break;

    case WM_COMMAND:
      EditCommand( hwnd, WMCOMMANDID( mp1, mp2 ), WMCOMMANDCMD( mp1, mp2 ) );
      break;

    case WM_EQF_CLOSE:
      EditClose( hwnd, mp1, mp2 );
      return 0;

    case WM_DESTROY:
      pIda = ACCESSDLGIDA( hwnd, PEDITIDA );
      if ( pIda )
      {
        if ( pIda->hFontControl ) DeleteObject( pIda->hFontControl );
        UtlAlloc( (PVOID *)&pIda->pStringBuffer, 0L, 0L, NOMSG );
      } /* endif */
      break;

    default:
      mResult = WinDefDlgProc (hwnd, msg, mp1, mp2);
  }
  return( mResult );

} /* end 'EntryEditDlgProc */

MRESULT VScroll( HWND hwnd, LONG lId, SHORT sNotification, LONG lScrollPos )
{
  PEDITIDA      pIda;                 // edit dialog ida
  PCONTROL      pControl;             // ptr for processing od controls
  PCONTROL      pLastControl;         // ptr to last edit control
  PCONTROL      pFirstControl;        // ptr to first (visible) control
  LONG          lYMove = 0;           // number of pels for move of controls
  LONG          lCurPos;              // old/current slider position

  if ( lId == EDIT_ENTRY_SCROLL_ENTRY )
  {
    pIda = ACCESSDLGIDA( hwnd, PEDITIDA );

    switch ( sNotification )           // case scroll command ...
    {
      case SB_LINEUP:
      case SB_LINEDOWN:
      case SB_PAGEUP:
      case SB_PAGEDOWN:
        switch ( sNotification )
        {
          case SB_LINEUP:
            pControl = GetFirstVisControl( pIda );
            if ( pControl && (pControl != pIda->pControls) )
            {
              pControl--; // get previous control
              lYMove = pControl->swpStatic.cy +
                       pIda->sCtrlDistance;
              lScrollPos = pControl - pIda->pControls + 1;
            }
            else
            {
              lYMove = 0; // nothing to scroll/move
            } /* endif */
            break;
          case SB_PAGEUP:
            pFirstControl =
            pControl      = GetFirstVisControl( pIda );
            lYMove = 0;
            while ( pControl &&
                    (pControl != pIda->pControls) &&
                    (pFirstControl->swpStatic.y + lYMove >
                     pIda->swpGrBox.y ) )
            {
              pControl--; // get previous control
              lYMove += pControl->swpStatic.cy +
                        pIda->sCtrlDistance;
            } /* endwhile */
            lScrollPos = pControl - pIda->pControls + 1;
            break;
          case SB_LINEDOWN:
            pControl = GetFirstVisControl( pIda );
            pLastControl = pIda->pControls +
                           (pIda->usNumControls - 1);
            if ( pControl && (pControl < pLastControl) )
            {
              lYMove = - pControl->swpStatic.cy -
                       pIda->sCtrlDistance;
              lScrollPos = pControl - pIda->pControls + 2;
            }
            else
            {
              lYMove = 0;
            } /* endif */
            break;
          case SB_PAGEDOWN:
            pControl = GetFirstVisControl( pIda );
            pLastControl = pIda->pControls +
                           (pIda->usNumControls - 1);
            lYMove = 0;
            while ( pControl &&
                    (pControl < pLastControl) &&
                    !(pControl->swpStatic.fs & EQF_SWP_HIDE) )
            {
              lYMove -= pControl->swpStatic.cy +
                        pIda->sCtrlDistance;
              pControl++;
            } /* endif */
            lScrollPos = pControl - pIda->pControls + 1;
            break;
        } /* endswitch */

        if ( lYMove )
        {
          SetScrollPos( GetDlgItem( hwnd, EDIT_ENTRY_SCROLL_ENTRY), SB_CTL,
                        lScrollPos, TRUE );
          MoveControls( pIda, lYMove );
        } /* endif */
        break;

      case SB_THUMBPOSITION:
        if ( (lScrollPos >= 1) && (lScrollPos <= pIda->usNumControls) )
        {
          lScrollPos--;              // convert to index

          pControl = GetFirstVisControl( pIda );
          lCurPos = pControl - pIda->pControls;

          // if not already at the given position ...
          if ( lCurPos != lScrollPos )
          {
            // ... compute amount of pixels to move controls
            lYMove = 0;
            if ( lCurPos < lScrollPos )
            {
              pLastControl = pIda->pControls + lScrollPos;
              while ( pControl < pLastControl )
              {
                lYMove -= pControl->swpStatic.cy +
                          pIda->sCtrlDistance;
                pControl++; // get next control
              } /* endwhile */
            }
            else
            {
              pLastControl = pIda->pControls + lScrollPos;
              while ( pControl > pLastControl )
              {
                pControl--; // get previous control
                lYMove += pControl->swpStatic.cy +
                          pIda->sCtrlDistance;
              } /* endwhile */
            } /* endif */
            SetScrollPos( GetDlgItem( hwnd, EDIT_ENTRY_SCROLL_ENTRY), SB_CTL,
                          lScrollPos + 1, TRUE );
            MoveControls( pIda, lYMove );

          } /* endif */
        } /* endif */

        break;
      default:
        break;
    } /* endswitch */
  } /* endif */
  return( FALSE );
}


VOID MoveControls( PEDITIDA pIda, LONG  lYMove )
{
  PCONTROL      pControl;             // ptr to currently tested control
  PCONTROL      pLastControl;         // ptr to last edit control
  PSWP          pSwp;                 // ptr into SWP buffer
  USHORT        usControls;           // number of controls requiring
                                      // processing by PM
  BOOL          fOldHidden;          // set if control was hidden
  BOOL          fNewHidden;           // set if control is now hidden
  RECT          rc;                   // rectangle being erased

  pControl     = pIda->pControls;
  pLastControl = pIda->pControls + (pIda->usNumControls - 1);
  usControls   = 0;
  pSwp         = pIda->pSWPs;
  pIda->usNumVisControls = 0;

  LOCKWINDOWUPDATE( pIda->hwndDlg );

  while ( pControl <= pLastControl )
  {
    // correct controls Y position
    pControl->swpStatic.y = (SHORT)(pControl->swpStatic.y + lYMove);
    pControl->swpEntry.y  = (SHORT)(pControl->swpEntry.y  + lYMove);

    // check visibility state

    fOldHidden = (pControl->swpEntry.fs & EQF_SWP_HIDE) != 0; // save old state

    if ( (pControl->swpStatic.y <= pIda->swpGrBox.y ) ||
         (pControl->swpStatic.y + pControl->swpStatic.cy >=
          pIda->swpGrBox.cy + pIda->swpGrBox.y) )
    {
      pControl->swpStatic.fs =
      pControl->swpEntry.fs  = EQF_SWP_MOVE | EQF_SWP_HIDE;
      fNewHidden = TRUE;
    }
    else
    {
      pControl->swpStatic.fs =
      pControl->swpEntry.fs  = EQF_SWP_MOVE | EQF_SWP_SHOW;
      pIda->usNumVisControls++;
      fNewHidden = FALSE;
    } /* endif */

    if ( fNewHidden != fOldHidden )
    {
      ShowWindow( pControl->hwndStatic, ( fNewHidden ) ? SW_HIDE : SW_SHOWNA );
      ShowWindow( pControl->hwndEntry, ( fNewHidden ) ? SW_HIDE : SW_SHOWNA );
    } /* endif */

    if ( !fNewHidden )
    {
      MoveWindow( pControl->hwndStatic,
                  pControl->swpStatic.x,
                  pControl->swpStatic.y,
                  pControl->swpStatic.cx,
                  pControl->swpStatic.cy, TRUE );
      MoveWindow( pControl->hwndEntry,
                  pControl->swpEntry.x,
                  pControl->swpEntry.y,
                  pControl->swpEntry.cx,
                  pControl->swpEntry.cy, TRUE );
    } /* endif */

    pControl++;
  } /* endwhile */

  LOCKWINDOWUPDATE( NULL );
  rc.left   = pIda->swpGrBox.x + 2;
  rc.right  = pIda->swpGrBox.x + pIda->swpGrBox.cx - 4;
  rc.top    = pIda->swpGrBox.y + 2;
  rc.bottom = pIda->swpGrBox.y + pIda->swpGrBox.cy - 2;
  InvalidateRect( pIda->hwndDlg, &rc, TRUE );
  UpdateWindow( pIda->hwndDlg );
} /* end of MoveControls */



PCONTROL GetFirstVisControl ( PEDITIDA pIda )
{
  PCONTROL      pControl;                // ptr to currently tested control

  pControl     = pIda->pControls;
  while ( (pControl <= pIda->pLastControl) &&
          (SWP_FLAG(pControl->swpStatic) & EQF_SWP_HIDE) )
  {
    pControl++;
  } /* endwhile */

  return( (pControl > pIda->pLastControl) ? NULL : pControl );
} /* endof GetFirstVisControl */


PCONTROL GetLastVisControl ( PEDITIDA pIda )
{
  PCONTROL      pControl;                // ptr to currently tested control

  pControl     = GetFirstVisControl( pIda );

  while ( (pControl <= pIda->pLastControl) &&
          !(SWP_FLAG(pControl->swpStatic) & EQF_SWP_HIDE) )
  {
    pControl++;
  } /* endwhile */

  if ( pControl != pIda->pControls )
  {
    pControl--;
  } /* endif */

  return( pControl );
} /* endof GetLastVisControl */


MRESULT EditChar( HWND hwnd, SHORT sVirtualKey )
{
  PCONTROL  pstControl;               // ptr to edit control structures
  HWND      hwndNextFocus;            // next focus to set on (entryfields)
  HWND      hwndTemp;                 // buffer for window handles
  PEDITIDA  pIda;                   // the dialog IDA
  BOOL      fPassThru = TRUE;         // TRUE = pass message to WinDefDlgProc

  pIda = ACCESSDLGIDA( hwnd, PEDITIDA );

  switch ( sVirtualKey )
  {
    case VK_TAB:
    case VK_ENTER:
    case VK_DOWN:
      hwndNextFocus = GETFOCUS();
      pstControl =  ControlFromHwnd (pIda, hwndNextFocus);
      if ( pstControl )
      {
        // one of our entry fields / MLEs has the focus
        if ( pstControl == pIda->pLastControl )
        {
          hwndNextFocus = WinWindowFromID( hwnd,
                                           EDIT_ENTRY_PUBO_NEXT_TEMPL );
          if ( !WinIsWindowEnabled( hwndNextFocus ) )
          {
            hwndNextFocus = WinWindowFromID( hwnd,
                                             EDIT_ENTRY_PUBO_ADD_TEMPL );
          } /* endif */
        }
        else
        {
          hwndNextFocus = (pstControl+1)->hwndEntry;

          if ( (pstControl+1)->swpStatic.fs & EQF_SWP_HIDE )
          {
            VScroll( hwnd, EDIT_ENTRY_SCROLL_ENTRY, SB_PAGEDOWN, 0 );
          } /* endif */
        } /* endif */
        SETFOCUSHWND( hwndNextFocus );
        fPassThru = FALSE;
      }
      else if ( hwndNextFocus == pIda->hwndHeadword )
      {
        pstControl = GetFirstVisControl( pIda );
        SETFOCUSHWND( pstControl->hwndEntry );
        fPassThru = FALSE;
      } /* endif */
      break;

    case VK_UP:
    case VK_BACKTAB:
      hwndNextFocus = GETFOCUS();
      pstControl =  ControlFromHwnd (pIda, hwndNextFocus);
      if ( pstControl)
      {
        if ( pstControl == pIda->pControls )
        {
          // focus is already on first control
          hwndNextFocus = pIda->hwndHeadword;
        }
        else
        {
          hwndNextFocus = (pstControl-1)->hwndEntry;
          if ( SWP_FLAG(((pstControl-1)->swpStatic)) & EQF_SWP_HIDE )
          {
            VScroll( hwnd, EDIT_ENTRY_SCROLL_ENTRY, SB_PAGEUP, 0 );
          } /* endif */
        } /* endif */
        SETFOCUSHWND( hwndNextFocus );
        fPassThru = FALSE;
      }
      else
      {
        hwndTemp = WinWindowFromID( hwnd, EDIT_ENTRY_PUBO_NEXT_TEMPL );
        if ( !WinIsWindowEnabled( hwndTemp ) )
        {
          hwndTemp = WinWindowFromID( hwnd, EDIT_ENTRY_PUBO_ADD_TEMPL );
        } /* endif */
        if ( hwndNextFocus == hwndTemp )
        {
          pstControl = GetLastVisControl( pIda );
          SETFOCUSHWND( pstControl->hwndEntry );
          fPassThru = FALSE;
        } /* endif */
      } /* endif */
      break;
  } /* endswitch VK */

  return( MRFROMSHORT(fPassThru) );
}

MRESULT EditControl
(
HWND   hwnd,                        // dialog handle
SHORT  sId,                         // id in action
SHORT  sNotification                // notification
)
{
  PEDITIDA   pIda;                    // the dialog IDA
  SHORT      sItem;                   // index of active listbox/combobox item
  PPROPDICTIONARY pDictProps;         // pointer to dictionary properties
  CHAR       szDate[20];              // buffer for current date string
  LONG       lTime;                   // date/time as long value
  PCONTROL   pstControl;              // ptr for controls processing
  HWND       hwndControl;             // handle of PM control


  pIda = ACCESSDLGIDA( hwnd, PEDITIDA );

  if ( pIda != NULL )
  {
    switch (sId)
    {
      case EDIT_ENTRY_EDIT_HEADWORD:
        switch (sNotification)
        {
          case EN_SETFOCUS:
            pIda->hwndLastField = WinWindowFromID( hwnd, sId );
            pIda->hwndSelection = WinWindowFromID( hwnd, sId );
            CheckSelection ( pIda );
            ENABLECTRLHWND( pIda->hwndPaste, CheckClipbrdData( hwnd ) );
            break;

          case EN_KILLFOCUS:
            pIda->hwndSelection = NULLHANDLE;
            ENABLECTRLHWND( pIda->hwndPaste, CheckClipbrdData( hwnd ) );

            ClearIME( hwnd );
            break;

          case EN_CHANGE:
            if ( !pIda->fChangedByProgram )
            {
              EditChangeCheck( pIda );
              if ( !pIda->fDataChanged )
              {
                pIda->fDataChanged = TRUE;
                if ( pIda->pEditCB->UpdateField.fInDict )
                {
                  UtlTime( &lTime );
                  UtlLongToDateString( lTime, szDate, sizeof(szDate) );

                  pstControl = pIda->pControls;
                  while ( (pstControl <= pIda->pLastControl) &&
                          (pstControl->usFieldNo !=
                           pIda->pEditCB->UpdateField.usField) )
                  {
                    pstControl++;
                  } /* endwhile */

                  if ( pstControl <= pIda->pLastControl )
                  {
                    SETTEXTHWND( pstControl->hwndEntry, szDate );
                  } /* endif */
                } /* endif */
              } /* endif */
            } /* endif */
            break;

        } /* endswitch NotifyCode */
        break;

      case EDIT_ENTRY_COMBO_TARGET_DICT:
        switch (sNotification)
        {
          case CBN_EFCHANGE:
            // get target dictionary
            QUERYTEXT( hwnd, EDIT_ENTRY_COMBO_TARGET_DICT, pIda->szTempBuf );
            sItem = CBSEARCHITEM( hwnd, EDIT_ENTRY_COMBO_TARGET_DICT,
                                  pIda->szTempBuf );
            pIda->hSaveDict = (HDCB) CBQUERYITEMHANDLE( hwnd,
                                                        EDIT_ENTRY_COMBO_TARGET_DICT, sItem );

            AsdRetPropPtr( NULL, pIda->hSaveDict, &pDictProps );

            if ( pDictProps->fProtected || pDictProps->fCopyRight )
            {
              ENABLECTRL( hwnd, EDIT_ENTRY_PUBO_SAVE, FALSE );
            }
            else
            {
              ENABLECTRL( hwnd, EDIT_ENTRY_PUBO_SAVE, TRUE );
            } /* endif */
            break;

          case CBN_SELCHANGE:
            // get target dictionary
            sItem = CBQUERYSELECTION( hwnd, EDIT_ENTRY_COMBO_TARGET_DICT );
            if ( sItem != LIT_NONE )
            {
              pIda->hSaveDict = (HDCB) CBQUERYITEMHANDLE( hwnd,
                                                          EDIT_ENTRY_COMBO_TARGET_DICT, sItem );

              AsdRetPropPtr( NULL, pIda->hSaveDict, &pDictProps );

              if ( pDictProps->fProtected || pDictProps->fCopyRight )
              {
                ENABLECTRL( hwnd, EDIT_ENTRY_PUBO_SAVE, FALSE );
              }
              else
              {
                ENABLECTRL( hwnd, EDIT_ENTRY_PUBO_SAVE, TRUE );
              } /* endif */
            } /* endif */
            break;

        } /* endswitch NotifyCode */
        break;

      default:
        if ( sId >= EDIT_ENTRY_FIRST_EF_ID )
        {
          switch (sNotification)
          {
            case EN_CHANGE:

              if ( !pIda->fChangedByProgram )
              {
                EditChangeCheck( pIda );
                /**********************************************/
                /* Handle term data change                    */
                /**********************************************/
                if ( !pIda->fDataChanged )
                {
                  pIda->fDataChanged = TRUE;
                  if ( pIda->pEditCB->UpdateField.fInDict )
                  {
                    UtlTime( &lTime );
                    UtlLongToDateString( lTime, szDate, sizeof(szDate) );

                    pstControl = pIda->pControls;
                    while ( (pstControl <= pIda->pLastControl) &&
                            (pstControl->usFieldNo !=
                             pIda->pEditCB->UpdateField.usField) )
                    {
                      pstControl++;
                    } /* endwhile */

                    if ( pstControl <= pIda->pLastControl )
                    {
                      SETTEXTHWND( pstControl->hwndEntry, szDate );
                    } /* endif */
                  } /* endif */
                } /* endif */

                /**********************************************/
                /* Handle target data change                  */
                /**********************************************/
                if ( !pIda->fTargChanged &&
                     pIda->pEditCB->TargUpdateField.fInDict )
                {
                  /********************************************/
                  /* Get control data of field which has been */
                  /* changed                                  */
                  /********************************************/
                  hwndControl = WinWindowFromID( hwnd, sId );
                  pstControl = pIda->pControls;
                  while ( (pstControl <= pIda->pLastControl) &&
                          (pstControl->hwndEntry != hwndControl) )
                  {
                    pstControl++;
                  } /* endwhile */

                  /********************************************/
                  /* If this control is on target level, set  */
                  /* target level update date field data      */
                  /********************************************/
                  if ( (pstControl <= pIda->pLastControl) &&
                       (pstControl->usLevel == 4) )
                  {
                    pIda->fTargChanged = TRUE;
                    UtlTime( &lTime );
                    UtlLongToDateString( lTime, szDate,
                                         sizeof(szDate) );

                    pstControl = pIda->pControls;
                    while ( (pstControl <= pIda->pLastControl) &&
                            (pstControl->usFieldNo !=
                             pIda->pEditCB->TargUpdateField.usField) )
                    {
                      pstControl++;
                    } /* endwhile */

                    if ( pstControl <= pIda->pLastControl )
                    {
                      SETTEXTHWND( pstControl->hwndEntry, szDate );
                    } /* endif */
                  } /* endif */
                } /* endif */
              } /* endif */
              break;

            case EN_SETFOCUS:
              pIda->hwndLastField = WinWindowFromID( hwnd, sId );
              pIda->hwndSelection = WinWindowFromID( hwnd, sId );
              CheckSelection ( pIda );
              ENABLECTRLHWND( pIda->hwndPaste, CheckClipbrdData( hwnd ) );
              break;

            case EN_KILLFOCUS:
              pIda->hwndSelection = NULLHANDLE;
              ENABLECTRLHWND( pIda->hwndPaste, CheckClipbrdData( hwnd ) );
              break;
          } /* endswitch NotifyCode */
          break;
        } /* endif */
    } /* endswitch ControlID */
  } /* endif pIda */
  return( 0 );
}

MRESULT EditCommand
(
HWND hwnd,
SHORT sId,                          // id of button
SHORT sNotification                 // notification type
)
{
  PEDITIDA   pIda;                    // the dialog data structure
  BOOL       fOK;                     // internal OK flag
  SHORT      sItem;                   // index of active listbox/combobox item
  PPROPDICTIONARY pDictProps;         // pointer to dictionary properties


  pIda = ACCESSDLGIDA( hwnd, PEDITIDA );
  if ( pIda != NULL )
  {
    switch (sId)
    {
      case EDIT_ENTRY_PUBO_HELP:
        UtlInvokeHelp();
        break;
      case EDIT_ENTRY_PUBO_SAVE:
        if ( pIda->fDisabled )
        {
          WinAlarm( HWND_DESKTOP, WA_ERROR );
        }
        else
        {
          // get target dictionary
          QUERYTEXT( hwnd, EDIT_ENTRY_COMBO_TARGET_DICT, pIda->szTempBuf );
          sItem = CBSEARCHITEM( hwnd, EDIT_ENTRY_COMBO_TARGET_DICT,
                                pIda->szTempBuf );
          pIda->hSaveDict = (HDCB) CBQUERYITEMHANDLE( hwnd,
                                                      EDIT_ENTRY_COMBO_TARGET_DICT, sItem );

          AsdRetPropPtr( NULL, pIda->hSaveDict, &pDictProps );

          if ( pDictProps->fProtected || pDictProps->fCopyRight )
          {
            UtlError( ERROR_EDITDICT_PROTECTED,
                      MB_CANCEL, 0, NULL, EQF_ERROR );
          }
          else
          {
            // save dictionary entry
            pIda->fDisabled = TRUE;
            fOK = SaveEntry (hwnd, pIda);

            if ( fOK)
            {
              pIda->fDataChanged = FALSE;
              WinPostMsg (pIda->pLUPCB->hwndParent,
                          pIda->pLUPCB->usNotifyMsg,
                          (WPARAM)hwnd,
                          MP2FROM2SHORT( DLG_CANCELED, EDIT_ENTRY_DLG) );
              WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, MP2FROMSHORT(TRUE) );
            }
            else
            {
              pIda->fDisabled = FALSE;
            } /* endif */
          } /* endif */
        } /* endif */
        return 0;

      case EDIT_ENTRY_PUBO_NEXT_TEMPL:
        if ( pIda->fDisabled )
        {
          WinAlarm( HWND_DESKTOP, WA_ERROR );
        }
        else
        {
          pIda->fDisabled = TRUE;
          WinTextToData (pIda);
          pIda->usTemplate += 1;
          if ( pIda->usTemplate > pIda->usMaxTempl )
          {
            pIda->usTemplate = 1;
          } /* endif */
          DataToWinText (pIda);
          pIda->fDisabled = FALSE;
        } /* endif */
        break;

      case EDIT_ENTRY_PUBO_PREV_TEMPL:
        if ( pIda->fDisabled )
        {
          WinAlarm( HWND_DESKTOP, WA_ERROR );
        }
        else
        {
          pIda->fDisabled = TRUE;
          WinTextToData (pIda);
          pIda->usTemplate -= 1;
          if ( pIda->usTemplate == 0 )
          {
            pIda->usTemplate = pIda->usMaxTempl;
          } /* endif */
          DataToWinText (pIda);
          pIda->fDisabled = FALSE;
        } /* endif */
        break;

      case EDIT_ENTRY_PUBO_DEL_ENTRY:
        if ( pIda->fDisabled )
        {
          WinAlarm( HWND_DESKTOP, WA_ERROR );
        }
        else
        {
          AsdRetPropPtr( NULL, pIda->hEditHandle, &pDictProps );

          if ( pDictProps->fProtected || pDictProps->fCopyRight )
          {
            UtlError( ERROR_EDITDICT_PROTECTED,
                      MB_CANCEL, 0, NULL, EQF_ERROR );
          }
          else
          {
            pIda->fDisabled = TRUE;
            DeleteEntry (pIda);
            pIda->fDisabled = FALSE;
          } /* endif */
        } /* endif */
        return 0;

      case EDIT_ENTRY_PUBO_DEL_TEMPL:
        if ( pIda->fDisabled )
        {
          WinAlarm( HWND_DESKTOP, WA_ERROR );
        }
        else
        {
          EditChangeCheck( pIda );
          pIda->fDisabled = TRUE;
          DeleteTemplate (pIda);
          if ( !pIda->fDataChanged )
          {
            pIda->fDataChanged = TRUE;
            /*****************************************************/
            /* Update of term date field has already been done   */
            /* in function DeleteTemplate                        */
            /*****************************************************/
          } /* endif */
          pIda->fDisabled = FALSE;
        } /* endif */
        return 0;

      case EDIT_ENTRY_PUBO_ADD_TEMPL:
        if ( pIda->fDisabled )
        {
          WinAlarm( HWND_DESKTOP, WA_ERROR );
        }
        else
        {
          EditChangeCheck( pIda );
          pIda->fDisabled = TRUE;
          AddTemplate( pIda, FALSE );
          if ( !pIda->fDataChanged )
          {
            pIda->fDataChanged = TRUE;
            /*****************************************************/
            /* Update of term date field has already been done   */
            /* in function AddTemplate                           */
            /*****************************************************/
          } /* endif */
          pIda->fDisabled = FALSE;
        } /* endif */
        return 0;

      case EDIT_ENTRY_PUBO_COPY_TEMPL:
        if ( pIda->fDisabled )
        {
          WinAlarm( HWND_DESKTOP, WA_ERROR );
        }
        else
        {
          EditChangeCheck( pIda );
          pIda->fDisabled = TRUE;
          AddTemplate( pIda, TRUE );
          if ( !pIda->fDataChanged )
          {
            pIda->fDataChanged = TRUE;
            /*****************************************************/
            /* Update of term date field has already been done   */
            /* in function AddTemplate                           */
            /*****************************************************/
          } /* endif */
          pIda->fDisabled = FALSE;
        } /* endif */
        return 0;

      case DID_CANCEL:
      case EDIT_ENTRY_PUBO_CANCEL:
        if ( pIda->fDisabled )
        {
          WinAlarm( HWND_DESKTOP, WA_ERROR );
        }
        else
        {
          pIda->fDisabled = TRUE;
          WinPostMsg (pIda->pLUPCB->hwndParent,
                      pIda->pLUPCB->usNotifyMsg,
                      (WPARAM)hwnd,
                      MP2FROM2SHORT (DLG_CANCELED, EDIT_ENTRY_DLG) );
          WinPostMsg (hwnd, WM_EQF_CLOSE, MP1FROMSHORT(TRUE), NULL);
        } /* endif */
        return 0;

      case EDIT_ENTRY_PUBO_PASTE:
        if ( pIda->fDisabled )
        {
          WinAlarm( HWND_DESKTOP, WA_ERROR );
        }
        else
        {
          ClipbrdToEntry( pIda, pIda->hwndLastField );
          CheckSelection( pIda );
        } /* endif */
        return 0;                              // query focus of current EF

      case EDIT_ENTRY_PUBO_COPY:
        if ( pIda->fDisabled )
        {
          WinAlarm( HWND_DESKTOP, WA_ERROR );
        }
        else
        {
          EntryToClipbrd( pIda, pIda->hwndLastField );
        } /* endif */
        return 0;                              // query focus of current EF


      case EDIT_ENTRY_EDIT_HEADWORD:
      case EDIT_ENTRY_COMBO_TARGET_DICT:
        EditControl( hwnd, sId, sNotification );
        break;

      default:
        if ( sId >= EDIT_ENTRY_FIRST_EF_ID )
          EditControl( hwnd, sId, sNotification );
        break;

    } /* endswitch ControlID */
  } /* endif pIda */
  return( MRFROMSHORT( TRUE ) );
}

//+----------------------------------------------------------------------------+
//| EditClose               - Close Dictionary Edit Dialog Window              |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Check if data has changed, cleanup, dismiss dialog                      |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HWND      hwnd                IN      window handle of edit dialog      |
//|    MPARAM    mp1                 IN      first parameter of WM_CLOSE msg   |
//|                                          TRUE  = check if data has changed |
//|                                          FALSE = close dialog w/o check    |
//|    MPARAM    mp2                 IN      second parameter of WM_CLOSE msg  |
//|                                          this value is retuned to the      |
//|                                          function calling the edit dialog  |
//|                                          via WinDismiss function           |
//|                                          TRUE  = dictionary has been       |
//|                                                  changed                   |
//|                                          FALSE = no changes done           |
//|                                          Note: If mp1 = TRUE and user      |
//|                                                has changed data and saves  |
//|                                                these changes, WinDismiss   |
//|                                                will return TRUE in any case|
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    MRESULT  mResult     message return code; is always FALSE               |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
MRESULT EditClose( HWND hwnd, WPARAM mp1, LPARAM mp2 )
{
  PEDITIDA   pIda;            // edit dialog IDA
  BOOL       fSaved;                  // data has been saved flag
  USHORT     usMBCode;                // message box return code
  BOOL       fClose = TRUE;           // 'close the dialog' flag
  PSZ        pszLastDict;             // ptr to name of last dictionary
  PTERM      pTerm;                   // ptr to current term
  SHORT      sItem;                   // listbox item index
  PPROPDICTIONARY pDictProps;         // pointer to dictionary properties

  fSaved = SHORT1FROMMP2(mp2);   // get initial data save flag

  pIda = ACCESSDLGIDA( hwnd, PEDITIDA );

  if (pIda && !pIda->fDestroy)
  {
    pIda->fDestroy = TRUE;
    pIda->fsStatus &= ~OTHER_DICT;

    if ( SHORT1FROMMP1(mp1)    &&     // if change-data check requested and
         pIda->fDataChanged )       // data has actually been changed ...
    {
      /*************************************************************/
      /* Get state of currently selected save-to dictionary        */
      /*************************************************************/
      QUERYTEXT( hwnd, EDIT_ENTRY_COMBO_TARGET_DICT, pIda->szTempBuf );
      sItem = CBSEARCHITEM( hwnd, EDIT_ENTRY_COMBO_TARGET_DICT,
                            pIda->szTempBuf );
      pIda->hSaveDict = (HDCB) CBQUERYITEMHANDLE( hwnd,
                                                  EDIT_ENTRY_COMBO_TARGET_DICT, sItem );

      AsdRetPropPtr( NULL, pIda->hSaveDict, &pDictProps );

      if ( pDictProps->fProtected || pDictProps->fCopyRight )
      {
      }
      else
      {
        pIda->fDisabled = FALSE;
        usMBCode = UtlError( WD_SAVE_ON_CLOSE,
                             MB_YESNOCANCEL | MB_WARNING | MB_DEFBUTTON1,
                             0, NULL, EQF_QUERY );
        switch ( usMBCode )
        {
          case MBID_YES :
            pIda->fDestroy = FALSE;
            pIda->fDisabled  = FALSE;
            fClose = FALSE;         // do not close the dialog
            WinPostMsg( hwnd, WM_COMMAND,
                        MP1FROMSHORT( EDIT_ENTRY_PUBO_SAVE), NULL );
            break;
          case MBID_CANCEL :
            pIda->fDestroy = FALSE;
            pIda->fDisabled  = FALSE;
            fClose = FALSE;         // do not close the dialog
            break;
          case MBID_NO :
          default:
            // continue closing dialog
            pIda->fDisabled = TRUE;
            break;
        } /* endswitch */
      } /* endif */
    } /* endif */

    if ( fClose && pIda->fList2Dict )
    {
      if ( fSaved )
      {
        /**************************************************************/
        /* mark current term as processed                             */
        /**************************************************************/
        pTerm = *(pIda->ppTermList);
        pTerm->Flags.fDeleted = TRUE;
        pIda->fListChanged    = TRUE;
      } /* endif */

      /************************************************************/
      /* Get name of dictionary which is active for this edit     */
      /************************************************************/
      pTerm = *(pIda->ppTermList);
      pszLastDict = pTerm->pszDestination;

      /************************************************************/
      /* Make the next term the current one                       */
      /************************************************************/
      pIda->ppTermList++;
      pIda->usTerms--;
      if ( pIda->usTerms )
      {
        pTerm = *(pIda->ppTermList);
      } /* endif */

      /**************************************************************/
      /* in case of CANCEL and more terms in list:                  */
      /*   let user decide if he/she wants to process more terms    */
      /**************************************************************/
      if ( !fSaved && pIda->usTerms )
      {
        usMBCode = UtlError( QUERY_EDITDICT_CONTINUE,
                             MB_YESNO | MB_WARNING | MB_DEFBUTTON1,
                             0, NULL, EQF_QUERY );
      }
      else
      {
        usMBCode = MBID_YES;
      } /* endif */

      /**************************************************************/
      /* Process next term if user wants to continue and list       */
      /* contains more terms                                        */
      /**************************************************************/
      if ( (usMBCode == MBID_YES) && pIda->usTerms )
      {
        /************************************************************/
        /* Check if target for new term is the current dictionary   */
        /************************************************************/
        if ( strcmp( pTerm->pszDestination, pszLastDict ) == 0 )
        {
          /**********************************************************/
          /* stay in  edit dialog and display the new term          */
          /**********************************************************/
          fClose           = FALSE;
          pIda->fDisabled  = FALSE;
          pIda->fDestroy   = FALSE;

          UTF16strcpy( pIda->szOrgHeadword, pTerm->pszName );
          UTF16strcpy( pIda->szHeadword, pTerm->pszName );

          EditGetEntry( pIda );

          DataToWinText( pIda );

        }
        else
        {
          fSaved = TRUE;             // allow continuation of term processing
        } /* endif */
      } /* endif */
    } /* endif */

    WinSetFocus( HWND_DESKTOP, hwnd );

    if ( fClose )
    {
      WinPostMsg (pIda->pLUPCB->hwndParent,
                  pIda->pLUPCB->usNotifyMsg,
                  (WPARAM)hwnd,
                  MP2FROM2SHORT( DLG_TERM_NORM, EDIT_ENTRY_DLG ) );

      UtlAlloc( (PVOID *)&pIda->pControls, 0L, 0L, NOMSG);
      UtlAlloc( (PVOID *)&pIda->pSWPs, 0L, 0L, NOMSG);
      if ( pIda->hLdbTree )  QLDBDestroyTree( &pIda->hLdbTree );

      // the following two statements force the focus to be on our dialog;
      // if the dialog hasn't the focus, no window gains the focus after
      // the dialog has been dismissed
      pIda->fDisabled = FALSE;
      WinSetFocus( HWND_DESKTOP, hwnd );


      /* 5@KIT0855A */
      WinStopTimer( WinQueryAnchorBlock(hwnd),
                    hwnd,
                    100 );

//      DelCtrlFont (hwnd, EDIT_ENTRY_EDIT_HEADWORD);

      WinDismissDlg( hwnd, fSaved );
    } /* endif */
  } /* endif  pIda*/
  return( 0 );
}

/*******************************************************************************
*
*       function: SaveEntry
*
*******************************************************************************/

USHORT SaveEntry (HWND hwnd, PEDITIDA  pIda)
{
  ULONG ulDataLen = 0;               // length of the LDB record
  USHORT usRC = 0;                    // return code for ASD-call
  USHORT usLdbRC;                     // return code of QLDB functions
  USHORT usSaveRC = TRUE;             // return code for this function
  PSZ_W  pucRecord = NULL;            // pointer to LDB record data
  USHORT usMBCode;                     // message box return code
  BOOL   fOtherDict = FALSE;           // TRUE = entry is saved to another dictionary
  USHORT usMergeFlags;                 // merge type flags
  PPROPDICTIONARY pSaveProp;           // ptr to profile of save dict
  PPROPDICTIONARY pEditProp;           // ptr to profile of edit dict
  USHORT          usI;                 // general loop index
  BOOL   fSaveEntry = TRUE;           // return from asking user if data are different
  BOOL   fOK = TRUE;                  // internal OK flag

  fOK = WinTextToData( pIda );

  if ( fOK )
  {
    /******************************************************/
    /* Check if struct of save-to-dict differs            */
    /* and get structure of the dict                      */
    /******************************************************/
    AsdRetPropPtr( NULL, pIda->hSaveDict, &pSaveProp );
    AsdRetPropPtr( NULL, pIda->hEditHandle, &pEditProp );

    pIda->fsStatus &= ~STRUCT_DIFFERENT;
    if ( pIda->hSaveDict != pIda->hEditHandle )
    {
      if ( pSaveProp->usLength == pEditProp->usLength )
      {
        usI = 0;
        while ( (usI < pSaveProp->usLength) &&
                (strcmp( pSaveProp->ProfEntry[usI].chSystName,
                         pEditProp->ProfEntry[usI].chSystName ) == 0) )
        {
          usI++;
        } /* endwhile */
        if ( usI < pSaveProp->usLength )
        {
          pIda->fsStatus |= STRUCT_DIFFERENT;
        } /* endif */
      }
      else
      {
        pIda->fsStatus |= STRUCT_DIFFERENT;
      } /* endif */
    } /* endif */

    if ( pIda->fsStatus & STRUCT_DIFFERENT )
    {
      fOtherDict = TRUE;
      usMBCode = ASKUSER( WD_STRUCT_DIFFERENT );
      if ( usMBCode != MBID_YES )
      {
        fSaveEntry = FALSE;
        fOK = FALSE;
      }  /* endif */
    } /* endif */

    /****************************************************************/
    /* Set merge flags                                              */
    /****************************************************************/
    if (fSaveEntry)
    {
      if ( pIda->fNewEntry )                                 /* 11@KIT1148A */
      {
        /************************************************************/
        /* A new entry will be written to the dictionary, use       */
        /* merge entry with user prompt option to allow merge if    */
        /* while editing the new entry another user has added the   */
        /* same term already.                                       */
        /************************************************************/
        usMergeFlags = MERGE_ADD | MERGE_USERPROMPT | MERGE_SOURCE_EDIT |
                       MERGE_NOPROMT_CHECKBOX;
      }
      else if ( (pIda->hSaveDict == pIda->hEditHandle) &&
                (UTF16strcmp( pIda->szOrgHeadword, pIda->szHeadword ) == 0) )
      {
        /************************************************************/
        /* An edited entry is rewritten to disk, so replace the     */
        /* existing entry without user notification                 */
        /************************************************************/
        usMergeFlags = MERGE_REPLACE | MERGE_NOUSERPROMPT;
      }
      else
      {
        /************************************************************/
        /* Leave it up to user to handle collisions                 */
        /************************************************************/
        usMergeFlags = MERGE_ADD | MERGE_USERPROMPT | MERGE_SOURCE_EDIT |
                       MERGE_NOPROMT_CHECKBOX;
      } /* endif */
    } /* endif */

    /******************************************************/
    /* Combine nodes with same data (not required as      */
    /* the function to join same nodes is also called     */
    /* by QLDBRecordToTree)                               */
    /******************************************************/

    /******************************************************/
    /* encode the data for saving in an ASD               */
    /******************************************************/
    if ( fSaveEntry )
    {
      // ulDataLen in Numb of charw's
      usLdbRC = QLDBTreeToRecord( pIda->hLdbTree, &pucRecord, &ulDataLen );
    } /* endif */

    /****************************************************************/
    /* Save entry in dictionary using AsdMerge function             */
    /****************************************************************/
    if ( fSaveEntry )
    {
      usRC = AsdMergeEntry( pIda->pLUPCB->hUCB,
                            pIda->hEditHandle, pIda->szHeadword,
                            ulDataLen, pucRecord,
                            pIda->hSaveDict, &usMergeFlags );
    } /* endif */

    if (usRC == LX_RC_OK_ASD)
    {
      /**************************************************************/
      /* Report changes to dictionary services                      */
      /**************************************************************/
      AsdEntryChange( pIda->pLUPCB->hUCB,
                      pIda->hSaveDict,
                      pIda->szHeadword );
    }
    else if ( usRC != LX_MAX_ERR )
    {
      AsdRCHandling( usRC, pIda->hSaveDict,                  /* 2@KIT1082C */
                     NULL, NULL, pIda->szHeadword, hwnd );
      usSaveRC = FALSE;
    }
    else
    {
      usSaveRC = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( pucRecord ) UtlAlloc( (PVOID *)&pucRecord, 0L, 0L, NOMSG );

  return((USHORT)(usSaveRC && fOK));
}

/*******************************************************************************
*
*       function: ClipbrdToEntry  (PASTE)
*
*******************************************************************************/

VOID ClipbrdToEntry (PEDITIDA  pIda, HWND hwndEntry)
{
  PCONTROL  pstControl;
  ULONG    ulSuccess;

  if (pIda && ((ulSuccess = CheckClipbrdData (pIda->hwndDlg))!= 0 ))  // check if data in clipbrd
  {
    ulSuccess = 0;
    if ( (pstControl = ControlFromHwnd (pIda, hwndEntry)) != NULL )
    {
      SendMessage( hwndEntry, WM_PASTE, 0, 0L );
      ulSuccess = TRUE;
    }
    else if (hwndEntry == pIda->hwndHeadword)
    {
      WinSendMsg (hwndEntry, WM_PASTE, 0, 0L );
      ulSuccess = TRUE;
    }
    else
    {
      NOTIFYUSER( WD_NO_EF_SPECIFIED);
      return;
    }
    if (!ulSuccess)
      NOTIFYUSER( WD_PASTE_NO_SUCCESS);
  }
  else
    NOTIFYUSER( WD_CLIPBOARD_EMPTY);
}

/*******************************************************************************
*
*       function: EntryToClipbrd  (COPY)
*
*******************************************************************************/

VOID EntryToClipbrd (PEDITIDA  pIda, HWND hwndEntry)
{
  PCONTROL    pstControl;
  ULONG       ulSuccess = 0;

  if ( (pstControl = ControlFromHwnd (pIda, hwndEntry)) != NULL )
  {

    SendMessage( hwndEntry, WM_COPY, 0, 0L );
    ulSuccess = CheckClipbrdData (hwndEntry);     //check if data in clipbrd

  }
  else if (hwndEntry == pIda->hwndHeadword)
  {

    SendMessage( hwndEntry, WM_COPY, 0, 0L );

    ulSuccess = CheckClipbrdData (hwndEntry);  // check if data in clipbrd
  }
  else
  {
    NOTIFYUSER( WD_NO_EF_SPECIFIED);
    return;
  }
  if (!ulSuccess)
    NOTIFYUSER( WD_NO_SELECTION);
  else
    ENABLECTRLHWND( pIda->hwndPaste, TRUE );
}

/*******************************************************************************
*
*       function: CheckClipbrdData
*
*******************************************************************************/

ULONG CheckClipbrdData( HWND hwndEntry )
{
  ULONG  ulDataLen = 0;

  if ( OpenClipboard( hwndEntry ) )
  {
    HANDLE hClipboardData;

    hClipboardData = GetClipboardData( CF_TEXT );
    if ( hClipboardData != NULL )
    {
      ulDataLen = GlobalSize( hClipboardData );
    } /* endif */
    CloseClipboard();
  } /* endif */

  return ulDataLen;
}

/*******************************************************************************
*
*       function:       InitEdit
*
*******************************************************************************/

VOID InitEdit (HWND hwnd, PEDITIDA  pIda )
{
  BOOL       fOK = TRUE;

  fOK = EditGetEntry( pIda );

  if ( fOK )
  {
    fOK = ( CreateControls( pIda ) != NULL);
  } /* endif */

  if ( fOK )
  {
    AsdRetPropPtr( NULL, pIda->hEditHandle, &pIda->pEditProp );
    if ( pIda->pEditProp->fProtected || pIda->pEditProp->fCopyRight )
    {
      ENABLECTRL( hwnd, EDIT_ENTRY_PUBO_DEL_ENTRY, FALSE );
    } /* endif */

    pIda->hSaveDict = pIda->hEditHandle;


    if ( pIda->usNumControls <= pIda->usNumVisControls )
    {
      SetScrollRange( GetDlgItem( hwnd, EDIT_ENTRY_SCROLL_ENTRY), SB_CTL,
                      1, 1, TRUE );
    }
    else
    {
      SetScrollRange( GetDlgItem( hwnd, EDIT_ENTRY_SCROLL_ENTRY), SB_CTL,
                      1, pIda->usNumControls, TRUE );
      SetScrollPos( GetDlgItem( hwnd, EDIT_ENTRY_SCROLL_ENTRY), SB_CTL,
                    1, TRUE );
    } /* endif */

    pIda->usScrollWidth = (SHORT) WinQuerySysValue (HWND_DESKTOP, SV_CXVSCROLL);
    pIda->usMaxShown = 0;
    ENABLECTRLHWND( pIda->hwndPaste, CheckClipbrdData( hwnd ) );

    WinPostMsg (pIda->pLUPCB->hwndParent,
                pIda->pLUPCB->usNotifyMsg,
                (WPARAM)hwnd,
                MP2FROM2SHORT( DLG_SHOWN, EDIT_ENTRY_DLG ) );

    SETTEXTLIMITHWND( pIda->hwndHeadword, (ENTRYFIELD_SIZE - 1) );

    UtlDispatch();

    pIda = ACCESSDLGIDA( hwnd, PEDITIDA );
    if ( pIda != NULL )
    {
      DataToWinText( pIda );
    } /* endif */

  }
  else // Creation of controls failed
  {
    WinPostMsg (hwnd, WM_EQF_CLOSE, NULL, NULL);
  }
  return;
}


/*******************************************************************************
*
*       function: DataToWinText
*
*******************************************************************************/

BOOL DataToWinText (PEDITIDA  pIda)
{
  PCONTROL   pstControl;
  PSZ_W      pucData;
  HWND       hwndEntry;                // handle of current control
  USHORT     usLdbRC = QLDB_NO_ERROR;       // return code of QLDB functions
  USHORT     usLevel;                  // level returned by QLDBNextTemplate
  USHORT     usTemplate;               // number of current template

  pIda->fChangedByProgram = TRUE;

  /********************************************************************/
  /* Position to active template (pIda->usTemplate)                   */
  /********************************************************************/
  QLDBResetTreePositions( pIda->hLdbTree );
  QLDBCurrTemplate( pIda->hLdbTree, pIda->apszFields );
  for ( usTemplate = 1;
      usTemplate < pIda->usTemplate;
      usTemplate++ )
  {
    QLDBNextTemplate( pIda->hLdbTree, pIda->apszFields, &usLevel );
  } /* endfor */

  /********************************************************************/
  /* Loop over controls and fill-in data from template                */
  /********************************************************************/
  pstControl = pIda->pControls;          // first control

  while ( pstControl <= pIda->pLastControl )
  {
    hwndEntry = pstControl->hwndEntry;
    pucData   = pIda->apszFields[pstControl->usFieldNo];

    if ( ((ULONG)pucData) % 2 ) {     /* if ptr is odd, then move to fix SetWindowText bug */
       wcscpy( pIda->szFieldBuffer, pucData ) ;
       pucData = pIda->szFieldBuffer ;
    }
    SETTEXTHWNDW( hwndEntry, pucData );

    pstControl++;
  } /* endwhile */

  // set template number text
  UpdateTemplNumber( pIda, pIda->usTemplate );

  {
    HWND hwnd = pIda->hwndDlg ;

    UtlDispatch();

    pIda = ACCESSDLGIDA( hwnd, PEDITIDA );
    if ( pIda != NULL )
    {
      pIda->fChangedByProgram = FALSE;
      pIda->fTargChanged      = FALSE;     // no changes in target data yet
    } /* endif */
  }


  return( usLdbRC == QLDB_NO_ERROR );
}


/*******************************************************************************
*
*       function: WinTextToData  // this function recognizes multiple
*                                // senses and targets
*******************************************************************************/

BOOL WinTextToData (PEDITIDA  pIda)

{
  PCONTROL pstControl;
  HWND       hwndEntry;
  ULONG       ulBytesExported;
  PSZ_W      pucField;                 // pointer to data of current field
  USHORT     usLevel;                  // level returned by QLDBNextTemplate
  USHORT     usTemplate;               // number of current template
  USHORT     usLdbRC;                  // return code of QLDB functions

  /********************************************************************/
  /* Reset string buffer pointer                                      */
  /********************************************************************/
  pucField = pIda->pStringBuffer;
  /********************************************************************/
  /* Position to active template (pIda->usTemplate)                   */
  /********************************************************************/
  QLDBResetTreePositions( pIda->hLdbTree );
  QLDBCurrTemplate( pIda->hLdbTree, pIda->apszFields );
  for ( usTemplate = 1;
      usTemplate < pIda->usTemplate;
      usTemplate++ )
  {
    QLDBNextTemplate( pIda->hLdbTree, pIda->apszFields, &usLevel );
  } /* endfor */

  QUERYTEXTHWNDW( pIda->hwndHeadword, pIda->szFieldBuffer,MLE_SIZE  );

  UtlStripBlanksW( pIda->szFieldBuffer );
  UTF16strcpy( pIda->szHeadword, pIda->szFieldBuffer );
  UTF16strcpy( pucField, pIda->szFieldBuffer );
  pIda->apszFields[pIda->usHeadwordField] = pucField;
  pucField += UTF16strlenCHAR(pucField) + 1;

  /********************************************************************/
  /* Loop over controls and set template data                         */
  /********************************************************************/
  pstControl = pIda->pControls;          // first control

  while ( pstControl <= pIda->pLastControl )
  {
    hwndEntry = pstControl->hwndEntry;

    ulBytesExported = (ULONG) QUERYTEXTHWNDW( hwndEntry, pIda->szFieldBuffer, MLE_SIZE );

    pIda->szFieldBuffer[ulBytesExported] = EOS;

    UTF16strcpy( pucField, pIda->szFieldBuffer );
    pIda->apszFields[pstControl->usFieldNo] = pucField;
    pucField += UTF16strlenCHAR(pucField) + 1;

    pstControl++;
  } /* endwhile */

  /********************************************************************/
  /* Update template data                                             */
  /********************************************************************/
  usLdbRC = QLDBUpdateCurrTemplate( pIda->hLdbTree, pIda->apszFields );

  return( usLdbRC == QLDB_NO_ERROR );
}

/*******************************************************************************
*
*       function:       DeleteEntry
*
*******************************************************************************/

VOID DeleteEntry (PEDITIDA  pIda)
{
  USHORT  usRC;
  PSZ_W   pucStringBuf;
  PSZ_W   apszErrParm[2];

  if (UtlAlloc ((PVOID *)&pucStringBuf, 0L, (LONG)ENTRYFIELD_SIZE * sizeof(CHAR_W), ERROR_STORAGE))
  {
    QUERYTEXTHWNDW( pIda->hwndHeadword, pucStringBuf, ENTRYFIELD_SIZE );
    pucStringBuf [ENTRYFIELD_SIZE - 1] = EOS;
    UtlStripBlanksW( pucStringBuf );

    if ( UTF16strcmp( pIda->szHeadword, pucStringBuf ) )
    {
      apszErrParm[0] = pIda->szHeadword;
      apszErrParm[1] = pucStringBuf;
      UtlErrorW( ERROR_HEADWORD_CHANGED, MB_CANCEL, 2, apszErrParm, EQF_ERROR, TRUE  );
    }
    else
    {
      if ( ASKUSER(WD_DELETE_ENTRY) == MBID_YES)
      {
        usRC = AsdDelEntry( pIda->pLUPCB->hUCB,
                            pIda->hEditHandle,
                            pucStringBuf );
        if ( usRC != LX_RC_OK_ASD )
        {
          AsdRCHandling( usRC, pIda->hEditHandle, NULL, NULL, NULL, NULLHANDLE );
        }
        else
        {
          WinPostMsg (pIda->hwndDlg, WM_EQF_CLOSE, NULL, MP2FROMSHORT(TRUE) );
        } /* endif */
      } /* endif */
    } /* endif */

    UtlAlloc ((PVOID *)&pucStringBuf, 0L, 0L, NOMSG);
  } /* endif */
}

/*******************************************************************************
*
*       function:       DeleteTemplate
*
*******************************************************************************/

VOID DeleteTemplate (PEDITIDA  pIda)
{
  USHORT     usLevel;                  // level returned by QLDBNextTemplate
  USHORT     usTemplate;               // number of current template
  USHORT     usLdbRC;                  // return code of QLDB functions
  CHAR_W     szDate[20];               // buffer for current date string
  LONG       lTime;                    // date/time as long value

  /********************************************************************/
  /* Position to active template (here: node)                         */
  /********************************************************************/
  usLdbRC = QLDBResetTreePositions( pIda->hLdbTree );
  usLdbRC = QLDBMoveToNode( pIda->hLdbTree, QLDB_CHILD_NODE,
                            pIda->apszFields, &usLevel );
  for ( usTemplate = 1;
      usTemplate < pIda->usTemplate;
      usTemplate++ )
  {
    usLdbRC = QLDBMoveToNode( pIda->hLdbTree, QLDB_NEXT_NODE_ON_SAME_LEVEL,
                              pIda->apszFields, &usLevel );
  } /* endfor */

  /********************************************************************/
  /* Delete active template                                           */
  /********************************************************************/
  usLdbRC = QLDBDestroySubtree( &pIda->hLdbTree );

  /********************************************************************/
  /* Update template numbers                                          */
  /********************************************************************/
  if ( pIda->usTemplate == pIda->usMaxTempl )
  {
    pIda->usTemplate--;
  } /* endif */
  pIda->usMaxTempl--;
  if ( pIda->usMaxTempl == 1 )
  {
    ENABLECTRL( pIda->hwndDlg, EDIT_ENTRY_PUBO_DEL_TEMPL, FALSE );
  } /* endif */

  /********************************************************************/
  /* Update term update data field if this is the first change        */
  /********************************************************************/
  if ( !pIda->fDataChanged && pIda->pEditCB->UpdateField.fInDict )
  {
    UtlTime( &lTime );
    UtlLongToDateStringW( lTime, szDate, sizeof(szDate)/ sizeof(CHAR_W) );

    QLDBCurrTemplate( pIda->hLdbTree, pIda->apszFields );

    pIda->apszFields[pIda->pEditCB->UpdateField.usField] = szDate;

    usLdbRC = QLDBUpdateCurrTemplate( pIda->hLdbTree, pIda->apszFields );
  } /* endif */

  /********************************************************************/
  /* Update term update author field if this is the first change      */
  /********************************************************************/
  if ( !pIda->fDataChanged && pIda->pEditCB->AuthorOfUpdateField.fInDict &&
       (pIda->szUserIDW[0] != EOS)  )
  {
    QLDBCurrTemplate( pIda->hLdbTree, pIda->apszFields );

    pIda->apszFields[pIda->pEditCB->AuthorOfUpdateField.usField] =
    pIda->szUserIDW;

    usLdbRC = QLDBUpdateCurrTemplate( pIda->hLdbTree, pIda->apszFields );
  } /* endif */

  /********************************************************************/
  /* Display new template                                             */
  /********************************************************************/
  DataToWinText( pIda );
}

/*******************************************************************************
*
*       function:    AddTemplate
*
*******************************************************************************/

VOID AddTemplate
(
PEDITIDA  pIda,                   // dialog IDA
BOOL      fCopy                     // copy template flag, TRUE = copy data
)
{
  USHORT     usLdbRC;                  // return code of QLDB functions
  USHORT     usField;                  // field index
  CHAR_W     szDate[20];               // buffer for current date string
  LONG       lTime;                    // date/time as long value

  /********************************************************************/
  /* Save current template                                            */
  /*   This will also position to the currently active template       */
  /********************************************************************/
  WinTextToData( pIda );

  /********************************************************************/
  /* Get active template                                              */
  /********************************************************************/
  usLdbRC = QLDBCurrTemplate( pIda->hLdbTree, pIda->apszFields );

  /********************************************************************/
  /* Clear field pointers if copy is off                              */
  /********************************************************************/
  if ( !fCopy )
  {
    for ( usField = 0; usField < pIda->pEditProp->usLength; usField++ )
    {
      pIda->apszFields[usField] = EMPTY_STRINGW;
    } /* endfor */
  } /* endif */

  /********************************************************************/
  /* Set automatic term and template date fields                      */
  /********************************************************************/
  if ( pIda->pEditCB->UpdateField.fInDict ||
       pIda->pEditCB->TargCreateDateField.fInDict ||
       pIda->pEditCB->TargUpdateField.fInDict )
  {
    UtlTime( &lTime );
    UtlLongToDateStringW( lTime, szDate, sizeof(szDate)/sizeof(CHAR_W) );

    if ( pIda->pEditCB->UpdateField.fInDict )
    {
      pIda->apszFields[pIda->pEditCB->UpdateField.usField] = szDate;
    } /* endif */

    if ( pIda->pEditCB->TargCreateDateField.fInDict )
    {
      pIda->apszFields[pIda->pEditCB->TargCreateDateField.usField] = szDate;
    } /* endif */

    if ( pIda->pEditCB->TargUpdateField.fInDict )
    {
      pIda->apszFields[pIda->pEditCB->TargUpdateField.usField] = szDate;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Set automatic term and template author fields                    */
  /********************************************************************/
  if ( ( pIda->pEditCB->TransAuthorField.fInDict ||
         pIda->pEditCB->AuthorOfUpdateField.fInDict ||
         pIda->pEditCB->TransAuthorOfUpdateField.fInDict ) &&
       (pIda->szUserIDW[0] != EOS) )
  {
    if ( pIda->pEditCB->TransAuthorField.fInDict )
    {
      pIda->apszFields[pIda->pEditCB->TransAuthorField.usField] =
      pIda->szUserIDW;
    } /* endif */

    if ( pIda->pEditCB->AuthorOfUpdateField.fInDict )
    {
      pIda->apszFields[pIda->pEditCB->AuthorOfUpdateField.usField] =
      pIda->szUserIDW;
    } /* endif */

    if ( pIda->pEditCB->TransAuthorOfUpdateField.fInDict )
    {
      pIda->apszFields[pIda->pEditCB->TransAuthorOfUpdateField.usField] =
      pIda->szUserIDW;
    } /* endif */
  } /* endif */


  /********************************************************************/
  /* Add template                                                     */
  /********************************************************************/
  usLdbRC = QLDBAddSubtree( pIda->hLdbTree, 2,
                            pIda->apszFields + pIda->ausFirstField[1] );

  /********************************************************************/
  /* Update template numbers                                          */
  /********************************************************************/
  pIda->usMaxTempl++;
  pIda->usTemplate = pIda->usMaxTempl;
  if ( pIda->usMaxTempl > 1 )
  {
    ENABLECTRL( pIda->hwndDlg, EDIT_ENTRY_PUBO_DEL_TEMPL, TRUE );
  } /* endif */

  /********************************************************************/
  /* Display new template                                             */
  /********************************************************************/
  DataToWinText( pIda );

}

/*******************************************************************************
*
*       function: CheckSelection
*
*******************************************************************************/

VOID CheckSelection (PEDITIDA  pIda)
{
  ULONG      ulSelection = 0L;
  PCONTROL pstControl;
  USHORT     usSelection = 0;
  /* 2@KIT0855A */
  HWND       hwndFocus;


  hwndFocus = GETFOCUS();
  if ( (pstControl = ControlFromHwnd( pIda, hwndFocus ) ) != NULL ) /* 1@KIT0855C */
  {
    if ((ulSelection = (ULONG)WinSendMsg( hwndFocus, EM_GETSEL, NULL, NULL))!= 0)
      usSelection = SHORT1FROMMP2(ulSelection) - SHORT2FROMMP2(ulSelection);
  }
  else if ( hwndFocus == pIda->hwndHeadword)                    /* 1@KIT0855C */
  {
    /* 1@KIT0855C */
    ulSelection = (ULONG)WinSendMsg ( hwndFocus, EM_GETSEL, NULL, NULL);
    if ( ulSelection != 0L )
      usSelection = SHORT1FROMMP2(ulSelection) - SHORT2FROMMP2(ulSelection);
  }
  else  // no valid entryfield
  {
    return;
  }

  if (usSelection)
    WinEnableWindow (pIda->hwndCopy, TRUE);
  else
    WinEnableWindow (pIda->hwndCopy, FALSE);
}


VOID UpdateTemplNumber( PEDITIDA  pIda, USHORT usTemplate )
{
  CHAR       cDataBuf[80];           // buffer for template number string
  CHAR       cCurTempl[10];          // number of current template
  CHAR       cMaxTempl[10];          // number of max. template
  PCHAR      apcParms[2];            // parameter poiter array
  PCONTROL   pstControl;              // ptr to control for current node
  ULONG      ulStyle;                 // buffer for static control style

  ULONG     ulMsgLen;                // length of created message


  //
  // update template number static
  //
  pIda->usTemplate = usTemplate;
  itoa( pIda->usTemplate, cCurTempl, 10 );
  itoa( pIda->usMaxTempl, cMaxTempl, 10 );
  apcParms[0] = cCurTempl;
  apcParms[1] = cMaxTempl;
  DosInsMessage( apcParms, 2,
                 pIda->szTemplatePattern, strlen(pIda->szTemplatePattern),
                 cDataBuf, sizeof(cDataBuf) - 1,
                 &ulMsgLen );
  cDataBuf[ulMsgLen] = EOS;
  SETTEXT( pIda->hwndDlg, EDIT_ENTRY_STEXT_TEMPL, cDataBuf );

  //
  // protect/unprotect headword level fields depending on current template
  //
  pstControl = pIda->pControls;          // first control
  while ( (pstControl <= pIda->pLastControl) && (pstControl->usLevel == 1) )
  {
    if ( pstControl->fsStatus & READ_ONLY )
    {
      // nothing to do: read-only fields do not require any change
    }
    else
    {
      if ( pIda->usTemplate == 1 )
      {
        // clear read-only mode for control

          WinSendMsg( pstControl->hwndEntry, EM_SETREADONLY,
                      MP1FROMSHORT(FALSE), 0L );
      }
      else
      {
        // set read-only mode for control

          WinSendMsg( pstControl->hwndEntry, EM_SETREADONLY,
                      MP1FROMSHORT(TRUE), 0L );

        ulStyle = WinQueryWindowULong( pstControl->hwndStatic, QWL_STYLE );

      } /* endif */
    } /* endif control is read-only */
    pstControl++;
  } /* endwhile */

} /* end of UpdateTemplNumber */


/*******************************************************************************
*
*       function:       CreateControls
*
*******************************************************************************/

PCONTROL CreateControls (PEDITIDA  pIda )
{
  PCONTROL    pControl;
  PPROFENTRY  pProfEntry;
  BOOL        fOK = TRUE;             // internal OK flag
  USHORT      usField;                // field number index
  ULONG       ulEntryStyle, ulStaticStyle; // window styles
  LONG        lLastYPos;              // Y position of last control
  LONG        lEndOfArea;             // End of visible area (Y value)
  USHORT      usCurrentID;            // ID of currently processed control
  USHORT      sCtrlHeight;            // height of current control
  HWND        hwndInsertBehind;       // Z-Order placement handle
  SWP         swpDialog;              // size and position of edit dialog
  HFONT       hFont;                  // font to be used for new controls
  CHAR_W      szUTF16Buffer[DICTENTRYLENGTH]; // buffer for OEM->ANSI conv.
  BOOL        fAllVisible = TRUE;     // all-controls-are-visible flag
  ULONG       ulOemCP= 0L;

  hwndInsertBehind = pIda->hwndHeadword;         // insert controls behind headword EF


  /********************************************************************/
  /* Use font of headword entry field for all controls                */
  /********************************************************************/
  hFont = (HFONT)SendMessage( pIda->hwndHeadword, WM_GETFONT, 0, 0L );


  WinQueryWindowPos( pIda->hwndDlg, &swpDialog );

  if ( fOK )
  {
    //--- get number of controls to be created ---
    pIda->usNumControls = 0;
    pProfEntry  = pIda->pEditProp->ProfEntry;
    for ( usField = 0; usField < pIda->pEditProp->usLength; usField++ )
    {
      if ( pProfEntry->usStatus != NA_STATUS)
      {
        if ( strcmp( pProfEntry->chSystName, HEADWORD_SYST_NAME ) )
        {
          pIda->usNumControls++;
        }
        else
        {
          pIda->usHeadwordField = usField;
        } /* endif */
      } /* endif */
      pProfEntry++;
    } /* endfor */

    //--- allocate storage for control array and SWP structures ---
    pIda->usNumVisControls = 0;
    fOK = UtlAlloc((PVOID *) &pIda->pControls, 0L,
                    (LONG) (pIda->usNumControls * sizeof(CONTROL)),
                    ERROR_STORAGE );
    if ( fOK )
    {
      fOK = UtlAlloc((PVOID *) &pIda->pSWPs, 0L,
                      (LONG) (pIda->usNumControls * sizeof(SWP) * 2),
                      ERROR_STORAGE );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    /******************************************************************/
    /* Get groupbox rectangle in Windows coordinates                  */
    /******************************************************************/
    {
      RECT rc;
      POINT pt;

      GetWindowRect( pIda->hwndGrBox, &rc );

      pt.x = rc.left;
      pt.y = rc.top;
      ScreenToClient( pIda->hwndDlg, &pt );

      pIda->swpGrBox.cx = (SHORT)(rc.right - rc.left);
      pIda->swpGrBox.cy = (SHORT)(rc.bottom - rc.top);
      pIda->swpGrBox.x  = (SHORT)pt.x;
      pIda->swpGrBox.y  = (SHORT)pt.y;
    }

    pProfEntry  = pIda->pEditProp->ProfEntry;
    pControl    = pIda->pControls;
    lLastYPos   = pIda->swpGrBox.y + pIda->sCharHeight;
    lEndOfArea  = pIda->swpGrBox.y + pIda->swpGrBox.cy; //  - pIda->sCharHeight;
    usCurrentID = EDIT_ENTRY_FIRST_EF_ID;
    for ( usField = 0; (usField < pIda->pEditProp->usLength && fOK); usField++ )
    {
      if ( (pProfEntry->usStatus != NA_STATUS) &&
           (usField != pIda->usHeadwordField ) )
      {
        ulStaticStyle = SS_LEFT;

        if ( pProfEntry->usStatus == RO_STATUS )
        {
          pControl->fsStatus = READ_ONLY;
        } /* endif */

        //--- set basic size of control and static text field ---
        sCtrlHeight = (pProfEntry->usEntryFieldType == TYPE_LARGE) ?
                      (MLE_HEIGHT_CHARS * pIda->sCharHeight) :
                      pIda->sCharHeight + pIda->sHalfChar;
        pControl->swpEntry.cy =
        pControl->swpStatic.cy = sCtrlHeight;
        pControl->swpEntry.y  =
        pControl->swpStatic.y = (SHORT)lLastYPos;
        lLastYPos += pIda->sCtrlDistance + pControl->swpStatic.cy;
        pControl->usControlID = usCurrentID++;
        pControl->swpEntry.x  = (SHORT)(EF_POS_CHARS * pIda->sCharWidth +
                                pIda->swpGrBox.x + pIda->lStaticEnlargement);
        pControl->swpEntry.cx = pIda->swpGrBox.cx -
                                pControl->swpEntry.x - 5;
        pControl->swpStatic.x    = STATIC_POS_CHARS * pIda->sCharWidth +
                                   pIda->swpGrBox.x;
        pControl->swpStatic.cx   = (SHORT)((STATIC_WIDTH_CHARS * pIda->sCharWidth) +
                                   pIda->lStaticEnlargement);


        //--- set control type specific styles ---
        if ( pProfEntry->usEntryFieldType == TYPE_LARGE )
        {
//         ulEntryStyle  = ES_MULTILINE | ES_WANTRETURN | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_BORDER;
          ulEntryStyle  = ES_MULTILINE | ES_WANTRETURN                  | ES_AUTOVSCROLL | WS_BORDER;
          if ( pProfEntry->usStatus == RO_STATUS )
          {
            ulEntryStyle |= ES_READONLY;
          }
          else
          {
            ulEntryStyle |= WS_TABSTOP;
          } /* endif */
          ulStaticStyle = SS_LEFT;
        }
        else
        {
          ulEntryStyle  = ES_LEFT | WS_BORDER | ES_AUTOHSCROLL;
          if (pProfEntry->usStatus == RO_STATUS)
          {
            ulEntryStyle |= ES_READONLY;
          }
          else
          {
            ulEntryStyle |= WS_TABSTOP;
          } /* endif */
          ulStaticStyle = SS_LEFTNOWORDWRAP;
        } /* endif */

        //-- set visible flag if control is in visible part of groupbox ---
        if ( (pControl->swpStatic.y + pControl->swpStatic.cy) < lEndOfArea )
        {
          ulStaticStyle |= WS_VISIBLE;
          ulEntryStyle  |= WS_VISIBLE;
          pControl->swpEntry.fs = pControl->swpStatic.fs = EQF_SWP_SHOW;
          pIda->usNumVisControls++;
        }
        else
        {
          pControl->swpEntry.fs =
          pControl->swpStatic.fs = EQF_SWP_HIDE;
          fAllVisible = FALSE;
        } /* endif */

        MultiByteToWideChar( CP_ACP, 0, pProfEntry->chUserName, -1, szUTF16Buffer, sizeof(szUTF16Buffer)/sizeof(CHAR_W) );
        pControl->hwndStatic = CreateWindowExW( 0, L"STATIC",
                                             szUTF16Buffer,
                                             WS_CHILD | ulStaticStyle,
                                             pControl->swpStatic.x,
                                             pControl->swpStatic.y,
                                             pControl->swpStatic.cx,
                                             pControl->swpStatic.cy,
                                             pIda->hwndDlg,
                                             NULL,
                                             (HINSTANCE)UtlQueryULong( QL_HAB ),
                                             NULL);


        if ( pControl->hwndStatic )
        {
          if ( pIda->hFontControl != NULL ) SendMessage( pControl->hwndStatic, WM_SETFONT, (WPARAM)pIda->hFontControl, MP2FROMSHORT(TRUE) );
          /************************************************************/
          /* Correct z-order                                          */
          /************************************************************/
          SetWindowPos( pControl->hwndStatic, hwndInsertBehind, 0, 0, 0, 0,
                        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );
        } /* endif */
        hwndInsertBehind =
        pControl->swpStatic.hwnd = pControl->hwndStatic;
        if ( (pControl->swpStatic.hwnd != NULL) &&
             (pControl->fsStatus == READ_ONLY) )
        {
          ENABLECTRLHWND( pControl->swpStatic.hwnd, FALSE );
        } /* endif */

        pControl->hwndEntry = CreateWindowExW( 0, L"Edit",
                                              L"",
                                              WS_CHILD | ulEntryStyle,
                                              pControl->swpEntry.x,
                                              pControl->swpEntry.y,
                                              pControl->swpEntry.cx,
                                              pControl->swpEntry.cy,
                                              pIda->hwndDlg,
                                              (HMENU)((LONG)pControl->usControlID),
                                              (HINSTANCE)UtlQueryULong( QL_HAB ),
                                              NULL );

        if ( pControl->hwndEntry )
        {
          if ( pIda->hFontControl != NULL ) SendMessage( pControl->hwndEntry, WM_SETFONT, (WPARAM)pIda->hFontControl, MP2FROMSHORT(TRUE) );
          /************************************************************/
          /* Correct z-order                                          */
          /************************************************************/
          SetWindowPos( pControl->hwndEntry, hwndInsertBehind, 0, 0, 0, 0,
                        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );

          /************************************************************/
          /* Subclass edit control for keyboard navigation            */
          /************************************************************/
          SetWindowSubclass( pControl->hwndEntry, EditSubclassProc, 0, 0);
        } /* endif */
        hwndInsertBehind =
        pControl->swpEntry.hwnd = pControl->hwndEntry;

        if ( !pControl->hwndEntry || !pControl->hwndStatic )
        {
          // creation of windows failed
          fOK = FALSE;
        }
        else
        {
          // windows were created, set textlimits
          if ( pProfEntry->usEntryFieldType != TYPE_LARGE )
          {
            SETTEXTLIMITHWND( pControl->hwndEntry, (ENTRYFIELD_SIZE - 1) );
          } /* endif */

          /********************************************************************/
          /* Set fonts                                                        */
          /********************************************************************/
          //SendMessage( pControl->hwndEntry,  WM_SETFONT, (WPARAM)hFont, FALSE );
          SendMessage( pControl->hwndStatic, WM_SETFONT, (WPARAM)hFont, FALSE );

          /***********************************************************/
          /* Grey out static fields for readonly fields              */
          /***********************************************************/
          if ( pControl->fsStatus == READ_ONLY )
          {
            ENABLECTRLHWND( pControl->swpStatic.hwnd, FALSE );
          } /* endif */
        } /* endif */

        pControl->usLevel    = pProfEntry->usLevel;
        pControl->usFieldNo  = usField;


        {
          HWND hwnd = pIda->hwndDlg ;

          UtlDispatch();               // wait until 'traffic' has ended

          pIda = ACCESSDLGIDA( hwnd, PEDITIDA );
          if ( pIda == NULL )
          {
            fOK = FALSE;               // trigger end-of-loop condition
          } /* endif */
        }

        pControl++;
      } /* endif */

      if ( fOK && pProfEntry && (usField == pIda->usHeadwordField)  )
      {
        ulOemCP = GetLangOEMCP(NULL);
        ASCII2UnicodeBuf( pProfEntry->chUserName,
                          pIda->szHeadWordUserNameW,
                          sizeof(pIda->szHeadWordUserNameW)/sizeof(CHAR_W),
                          ulOemCP);
        pIda->pucHeadwordName = pIda->szHeadWordUserNameW;
      } /* endif */

      pProfEntry++;

    } /* endfor */
  } /* endif */

  if ( fOK )
  {
    pIda->pLastControl = pIda->pControls + (pIda->usNumControls -  1);
  } /* endif */

  //
  // cleanup
  //
  if ( !fOK )
  {
    pControl = pIda->pControls;
    while (pControl && pIda->usNumControls )
    {
      WinDestroyWindow( pControl->hwndEntry);
      WinDestroyWindow( pControl->hwndStatic);
      pControl++;
      pIda->usNumControls--;
    } /* endwhile */

    UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR );
    if ( pIda->pControls )
    {
      UtlAlloc((PVOID *) &pIda->pControls, 0L, 0L, NOMSG );
    } /* endif */
    if ( pIda->pSWPs )
    {
      UtlAlloc((PVOID *) &pIda->pSWPs, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Hide scrollbar if all controls are visible                       */
  /********************************************************************/
  if ( fAllVisible )
  {
    HIDECONTROL( pIda->hwndDlg, EDIT_ENTRY_SCROLL_ENTRY );
  } /* endif */

  return( pIda->pControls );
} /* end of CreateControls */

/*******************************************************************************
*
*       function        GetCharXY
*
*******************************************************************************/

VOID GetCharXY (HPS hps, PUSHORT puscxChar, PUSHORT puscyChar)
{
  FONTMETRICS fm;

  GpiQueryFontMetrics (hps, (LONG) sizeof fm, &fm);

  *puscxChar = (SHORT) fm.tmAveCharWidth;
  *puscyChar = (SHORT) (fm.tmHeight + fm.tmExternalLeading);

}

/*******************************************************************************
*
*       function        ControlFromHwnd
*
*******************************************************************************/

PCONTROL ControlFromHwnd( PEDITIDA pIda, HWND hwnd )
{
  PCONTROL pControl;

  // search for a control with a specified handle

  pControl = pIda->pControls;
  while ( pControl && (pControl <= pIda->pLastControl) )
  {
    if ( (pControl->hwndEntry == hwnd) || (pControl->hwndStatic == hwnd) )
    {
      return pControl;
    }
    pControl++;
  } /* endwhile */

  return NULL;
}

/*******************************************************************************
*
*       function        FillDictCombo
*
*******************************************************************************/

VOID FillDictCombo (PEDITIDA  pIda )
{
  HWND   hwndCombo;
  HDCB   ahDCB[MAX_DICTS+1];          // list of associated dictionaries
  USHORT usNumOfDicts;                // number of dictionaries in association
  USHORT usI;                         // loop index
  SHORT  sItem;                       // index of inserted listbox item

  // get list of associated dictionaries
  AsdRetDictList( pIda->pLUPCB->hDCB, ahDCB, &usNumOfDicts );

  // fill the combo box of the edit dialog with all target dictionaries
  hwndCombo = WinWindowFromID (pIda->hwndDlg, EDIT_ENTRY_COMBO_TARGET_DICT);

  for ( usI=0; usI<usNumOfDicts; usI++)
  {
    AsdQueryDictName( ahDCB[usI], pIda->szTempBuf );
    OEMTOANSI( pIda->szTempBuf );
    sItem = CBINSERTITEMHWND( hwndCombo, pIda->szTempBuf );
    CBSETITEMHANDLEHWND( hwndCombo, sItem, ahDCB[usI] );
  } /* endfor */

  AsdQueryDictName( pIda->hEditHandle, pIda->szTempBuf );

  OEMTOANSI( pIda->szTempBuf );

  {
    SHORT         sItem;              // combobox item index

    sItem = CBSEARCHITEMHWND( hwndCombo, pIda->szTempBuf );
    if ( sItem != LIT_NONE )
    {
      CBSELECTITEMHWND( hwndCombo, sItem );
    } /* endif */
  }

}

/*******************************************************************************
*
*       function:  SearchAndEdit
*
*******************************************************************************/

USHORT SearchAndEdit
(
HWND    hwnd,
PLUPCB  pLUPCB,
PSZ_W   pucTerm,
HMODULE hmod,
HDCB    hdcbEdit                    // handle of dictionary to be searched
// for or NULL if hDCB of lookup
// control block is to be used
)
{
  PEDITIDA   pIda     = NULL;          // IDA of dictioanry edit dialog
  BOOL       fOK      = TRUE;          // OK flag for program control
  INT_PTR    iEditRC = DID_ERROR;      // edit dialog return code
  USHORT     usRC;
  HDCB       hLockedDict;              // handle of locked dictionary

  /********************************************************************/
  /* Allocate IDA                                                     */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof(EDITIDA), ERROR_STORAGE );
  } /* endif */

  /********************************************************************/
  /* Fill IDA fields                                                  */
  /********************************************************************/
  if ( fOK )
  {
    pIda->hmod      = hmod;
    pIda->pLUPCB    = pLUPCB;
    pIda->hwndFocus = GETFOCUS();
    pIda->hwndCall  = hwnd; // the display Dialog..
    UTF16strcpy( pIda->szOrgHeadword, pucTerm );
    UTF16strcpy( pIda->szHeadword, pucTerm );
    pIda->hEditHandle = ( hdcbEdit ) ? hdcbEdit : pLUPCB->hDCB;
  } /* endif */

  /********************************************************************/
  /* Activate edit dialog                                             */
  /********************************************************************/
  if ( fOK )
  {
    /****************************************************************/
    /* lock the entry                                               */
    /****************************************************************/
    hLockedDict = pIda->hEditHandle;
    usRC = AsdLockEntry( hLockedDict, pucTerm, TRUE );
    if ( usRC != LX_RC_OK_ASD )
    {
      /* 2@KIT1082C */
      AsdRCHandling( usRC, pIda->hEditHandle, NULL, NULL, pucTerm, NULLHANDLE );
      fOK = FALSE;
    }
    else
    {
	  HMODULE hResMod;
	  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      if ( pLUPCB->ulFlags & LUP_MODALDLG_MODE )
      {
        DIALOGBOXW( hwnd, EditEntryDlgProc,
                   hResMod, EDIT_ENTRY_DLG, pIda, iEditRC );
      }
      else
      {
        DIALOGBOXW( pLUPCB->hwndParent, EditEntryDlgProc,
                   hResMod, EDIT_ENTRY_DLG, pIda, iEditRC );
      } /* endif */
      if ( iEditRC == DID_ERROR )      // Failed WinDlgBox call?
      {
        UtlError( WD_START_EDIT, MB_CANCEL, 0, NULL, EQF_ERROR );
      } /* endif */
      /**************************************************************/
      /* free previously locked entry                               */
      /**************************************************************/
      AsdLockEntry( hLockedDict, pucTerm, FALSE );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( pIda )                    UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );

  return( (USHORT)iEditRC );
}

//+----------------------------------------------------------------------------+
//| EditChangeCheck         - check if change of data is possible              |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    If nothing has been changed yet and the edit dictionary is read-only,   |
//|    a warning message is issued.                                            |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//|    if change flag is false                                                 |
//|       if dictionary is protected                                           |
//|          issue change warning                                              |
//|       endif;                                                               |
//|    endif;                                                                  |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PEDITIDA  pIda                IN      pointer to edit IDA               |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        TRUE (always)                                      |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    pIda must point to a valid IDA.                                         |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT EditChangeCheck
(
PEDITIDA pIda                       // pointer to edit dialog IDA
)
{
  PPROPDICTIONARY pDictProps;         // pointer to dictionary properties
  HDCB            hSaveDict;          // handle of 'save to' dict
  SHORT           sItem;              // listbox item index

  if ( !pIda->fDataChanged )          // if nothing has been changed yet ...
  {
    QUERYTEXT( pIda->hwndDlg, EDIT_ENTRY_COMBO_TARGET_DICT, pIda->szTempBuf );
    sItem = CBSEARCHITEM( pIda->hwndDlg, EDIT_ENTRY_COMBO_TARGET_DICT,
                          pIda->szTempBuf );
    hSaveDict = (HDCB) CBQUERYITEMHANDLE( pIda->hwndDlg,
                                          EDIT_ENTRY_COMBO_TARGET_DICT, sItem );

    AsdRetPropPtr( NULL, hSaveDict, &pDictProps );

    if ( pDictProps->fProtected || pDictProps->fCopyRight )
    {
      UtlError( WARNING_EDITDICT_PROTECTED, MB_OK, 0, NULL, EQF_ERROR );
    } /* endif */
  } /* endif */
  return( TRUE );
} /* end of EditChangeCheck */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     List2DictInterface                                       |
//+----------------------------------------------------------------------------+
//|Function call:     List2DictInterface( HWND hwndCaller,                     |
//|                                       PTERM *ppTermList,                   |
//|                                       USHORT usTerms );                    |
//+----------------------------------------------------------------------------+
//|Description:       Process a list of terms marked for dictionaries.         |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND    hwndCaller        caller's window handle         |
//|                   PTERM   *ppTermList       ptr to dictionary term list    |
//|                   USHORT  usTerms           number of terms in list        |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE      list has been changed                          |
//|                   FALSE     no changes have been done                      |
//+----------------------------------------------------------------------------+
//|Function flow:     begin ASD session (AsdBegin)                             |
//|                   allocate edit dialog IDA                                 |
//|                   get current focus window                                 |
//|                   loop through term table until complete or cancelled      |
//|                     check if target dictionary of term is locked           |
//|                     lock target dictionary of term                         |
//|                     open the dictionary (AsdOpen)                          |
//|                     begin lookup session (LupBegin)                        |
//|                     fill edit dialog IDA fields                            |
//|                     activate edit dialog                                   |
//|                     end lookup session and close dictionary                |
//|                     unlock dictionary                                      |
//|                     save IDA values and clear IDA                          |
//|                   free edit dialog IDA                                     |
//|                   end Asd session (AsdEnd)                                 |
//+----------------------------------------------------------------------------+
BOOL List2DictInterface
(
HWND    hwndCaller,                  // caller's window handle
PTERM   *ppTermList,                 // ptr to dictionary term list
USHORT  usTerms,                     // number of terms in list
PUSHORT pusRC                        // pointer to caller's return code
)
{
  PEDITIDA   pIda     = NULL;          // IDA of dictionary edit dialog
  BOOL       fOK = TRUE;               // OK flag for program control
  INT_PTR    iEditRC = TRUE;           // edit dialog return code
  HUCB       hUser;                    // user control block handle
  HLUPCB     hLupCB;                   // lookup control block handle
  USHORT     usRC;                     // return code of called functions
  CHAR       szDictName[MAX_EQF_PATH]; // buffer for dictionary name
  PSZ        pszDictName = szDictName; // pointer to dictionary name area
  PTERM      pTerm;                    // pointer to data of a term
  USHORT     usErrDict;                // number of erraneous dictionary
  HWND       hwndFocus = NULL;         // focus window
  BOOL       fListChanged = FALSE;     // TRUE = list has been changed
  BOOL       fLocked = FALSE;          // TRUE = dictionary has been locked

  /********************************************************************/
  /* Begin ASD session                                                */
  /********************************************************************/
  hUser = NULL;
  usRC = AsdBegin( 2, &hUser );
  fOK = ( usRC == LX_RC_OK_ASD );

  /********************************************************************/
  /* Allocate IDA                                                     */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof(EDITIDA), ERROR_STORAGE );
  } /* endif */

  /******************************************************************/
  /* Fill IDA fields                                                */
  /******************************************************************/
  if ( fOK )
  {
    hwndFocus = GETFOCUS();
  } /* endif */

  /********************************************************************/
  /* Loop through term table until canceled or all terms have been    */
  /* processed                                                        */
  /********************************************************************/
  while ( fOK && usTerms && (iEditRC == TRUE) )
  {
    pTerm        = *ppTermList;
    hLupCB       = NULL;

    /******************************************************************/
    /* Check if dictionary is currently locked, issue error message   */
    /* or lock dictionary                                             */
    /******************************************************************/
    UtlMakeEQFPath( szDictName, NULC, SYSTEM_PATH, (PSZ) NULP );
    strcat( szDictName, BACKSLASH_STR );
    strcat( szDictName, pTerm->pszDestination );
    strcat( szDictName, EXT_OF_DICTPROP );
    if ( (SHORT) WinSendMsg( EqfQueryObjectManager(),
                             WM_EQF_QUERYSYMBOL,
                             NULL,
                             MP2FROMP( szDictName )) != -1 )
    {
      /****************************************************************/
      /* Dictionary is locked: issue approbriate message              */
      /****************************************************************/
      UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, &(pTerm->pszDestination),
                EQF_ERROR );
      fOK = FALSE;
    }
    else
    {
      /****************************************************************/
      /* lock dictionary                                              */
      /****************************************************************/
      WinSendMsg( EqfQueryObjectManager(),
                  WM_EQF_SETSYMBOL,
                  NULL,
                  MP2FROMP( szDictName ));
      fLocked = TRUE;
    } /* endif */

    /******************************************************************/
    /* Build dictionary name                                          */
    /******************************************************************/
    if ( fOK )
    {
      UtlMakeEQFPath( szDictName, NULC, PROPERTY_PATH, NULL );
      strcat( szDictName, BACKSLASH_STR );
      strcat( szDictName, pTerm->pszDestination );
      strcat( szDictName, EXT_OF_DICTPROP );
    } /* endif */

    /******************************************************************/
    /* open the target dictionary for the list terms                  */
    /******************************************************************/
    if ( fOK )
    {
      usRC = AsdOpen( hUser,           // user ctl block handle
                      ASD_GUARDED,     // open flags
                      1,               // nr of dict in pszDictName
                      &pszDictName,    // dictionary name array
                      &pIda->hEditHandle,// dict ctl block handle
                      &usErrDict );    // number of failing dict
      if ( usRC != LX_RC_OK_ASD )
      {
        /* 1@KIT1082C */
        AsdRCHandling( usRC, NULL, pTerm->pszDestination, NULL, NULL, NULLHANDLE );
        fOK = FALSE;
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* begin lookup session                                           */
    /******************************************************************/
    if ( fOK )
    {
      USHORT usLupID;

      LupBegin( hUser,                 // Asd services user control block
                pIda->hEditHandle,     // Asd services dictionary control block
                hwndCaller,            // parent handle for dialogs and messages
                WM_EQF_WD_MAIN_NOTIFY, // message used for notifications
                NULL,                  // size/position of display dialog
                NULL,                  // size/position of edit dialog or NULL
                &hLupCB,               // Lookup services control block
                &usLupID );            // Lookup ID
    } /* endif */

    /******************************************************************/
    /* Fill IDA fields                                                */
    /******************************************************************/
    if ( fOK )
    {
	  HMODULE hResMod;
	  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      pIda->pLUPCB    = (PLUPCB)hLupCB;
      UTF16strcpy( pIda->szOrgHeadword, pTerm->pszName );
      UTF16strcpy( pIda->szHeadword, pTerm->pszName );
      pIda->hmod      = hResMod;
      pIda->hwndFocus = hwndFocus;
      pIda->hwndCall  = hwndCaller;      // the work with lists dialog
      pIda->fList2Dict= TRUE;
      pIda->usTerms   = usTerms;
      pIda->ppTermList = ppTermList;
      pIda->fListChanged = fListChanged;
    } /* endif */

    /********************************************************************/
    /* Activate edit dialog                                             */
    /********************************************************************/
    if ( fOK )
    {
      /****************************************************************/
      /* lock the entry                                               */
      /****************************************************************/
      usRC = AsdLockEntry( pIda->hEditHandle, pTerm->pszName, TRUE );
      if ( usRC != LX_RC_OK_ASD )
      {
        /* 1@KIT1082C */
        AsdRCHandling( usRC, pIda->hEditHandle, NULL, NULL, pTerm->pszName, NULLHANDLE );
        fOK = FALSE;
      }
      else
      {
		HMODULE hResMod;
		hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        DIALOGBOXW( hwndCaller, EditEntryDlgProc,
                    hResMod, EDIT_ENTRY_DLG, pIda, iEditRC );

        if ( iEditRC == DID_ERROR )      // Failed WinDlgBox call?
        {
          UtlError( WD_START_EDIT, MB_CANCEL, 0, NULL, EQF_ERROR );
        } /* endif */
        /**************************************************************/
        /* free previously locked entry                               */
        /**************************************************************/
        AsdLockEntry( pIda->hEditHandle, pTerm->pszName, FALSE );
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* Close lookup services and dictionary                           */
    /******************************************************************/
    if ( hLupCB )            LupEnd( hLupCB );
    if ( pIda->hEditHandle ) AsdClose( hUser, pIda->hEditHandle );

    /******************************************************************/
    /* unlock dictionary                                              */
    /******************************************************************/
    if ( fLocked )
    {
      UtlMakeEQFPath( szDictName, NULC, SYSTEM_PATH, (PSZ) NULP );
      strcat( szDictName, BACKSLASH_STR );
      strcat( szDictName, pTerm->pszDestination );
      strcat( szDictName, EXT_OF_DICTPROP );
      WinSendMsg( EqfQueryObjectManager(),
                  WM_EQF_REMOVESYMBOL,
                  NULL,
                  MP2FROMP( pszDictName ));
      fLocked = FALSE;
    } /* endif */

    /******************************************************************/
    /* Save IDA values and clear IDA                                  */
    /******************************************************************/
    if ( fOK )
    {
      usTerms    = pIda->usTerms ;
      ppTermList = pIda->ppTermList;
      fListChanged = pIda->fListChanged;
      memset( pIda, 0, sizeof(EDITIDA) );
    } /* endif */
  } /* endwhile */


  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( pIda )                    UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
  if ( hUser )                   AsdEnd( hUser );

  if ( !fOK )
  {
    *pusRC = ERROR_INVALID_FUNCTION; // dummy return code to indicate failures
  } /* endif */

  return( fListChanged );
} /* end of function List2DictInterface */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EditGetEntry                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EditGetEntry( pIda );                                    |
//+----------------------------------------------------------------------------+
//|Description:       Reads the entry with the name pIda->szHeadword from the  |
//|                   dictionary and converts it into a LDB tree. If no entry  |
//|                   with the given name is found, an empty LDB tree is       |
//|                   created.                                                 |
//|                   If list-to-dictionary mode is active the term data       |
//|                   (translation and context) is added to the LDB tree.      |
//+----------------------------------------------------------------------------+
//|Input parameter:   PEDITIDA     pIda    pointer to dictionary edit IDA      |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE      function completed successfully                |
//|                   FALSE     errors occurred                                |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      - The following IDA fields are changed:                  |
//|                     pEditHandle  (receives the dictionary of the match)    |
//|                     szHeadword   (receives the headword found)             |
//|                     pEditProp    (receives pointer to dictionary profile)  |
//|                     hLdbTree     (receives address of LDB tree)            |
//|                     usTemplate   (receives number of current template)     |
//|                     usMaxTempl   (receives maximum template number)        |
//|                   - The dictionary name static is filled with the          |
//|                     dictionary name                                        |
//|                   - the initial state of the dialog pushbuttons is set     |
//|                   - the headword is displayed in the headword entry field  |
//+----------------------------------------------------------------------------+
//|Function flow:     call AsdFndMatch to read the dictionary entry            |
//|                   if found get entry data                                  |
//|                   get dictionary handle for terms not found                |
//|                   get pointer to dictionary properties                     |
//|                   get field numbers for translation and context field      |
//|                   prepare an empty template for add template or create tree|
//|                   add translation and/or context data to template          |
//|                   if entry does not exist                                  |
//|                     create LDB tree from prepared template                 |
//|                   else                                                     |
//|                     convert entry data to LDB tree                         |
//|                     flatten LDB tree                                       |
//|                     if list->dictionary mode                               |
//|                       add prepared template to ldb tree                    |
//|                     set current template and maximum template to 1         |
//|                       set current template to last template                |
//|                     else                                                   |
//|                       set current template to first template               |
//|                     evaluate number of templates                           |
//|                     if list->dictionary mode and term data has been added  |
//|                       position to last template                            |
//|                   set enabled state of delete pushbutton                   |
//|                   display dictionary name                                  |
//|                   display headword                                         |
//|                   free any entry data                                      |
//+----------------------------------------------------------------------------+
BOOL EditGetEntry( PEDITIDA pIda )
{
  PSZ_W      pucData  = NULL;          // pointer to found dictionary (term) data
  ULONG      ulTermNum;                // term number
  ULONG      ulDataLen;                // length of term data record
  USHORT     usRC;                     // return code from ASD-calls
  USHORT     usLdbRC;                  // return code of LDB calls
  USHORT     usI;                      // general loop index
  HDCB       ahDCB[MAX_DICTS+1];       // list of associated dictionaries
  USHORT     usNumOfDicts;             // number of dictionaries in association
  USHORT     usLevel;                  // level returned by QLDBNextTemplate
  PTERM      pTerm = NULL;             // ptr to term data
  CHAR_W     szDateW[20];               // buffer for current date string
  LONG       lTime;                    // date/time as long value
  PSZ_W      pszNextTrans = NULL;      // ptr to next translation (List2Dict mode)
  USHORT     usAddedTemplates = 0;     // added templates
  BOOL       fAnsiConv = TRUE;

  /********************************************************************/
  /* Find entry                                                       */
  /********************************************************************/
  usRC = AsdFndMatch( pIda->szHeadword,
                      pIda->hEditHandle,
                      pIda->pLUPCB->hUCB,
                      pIda->szHeadword,
                      &ulTermNum,
                      &ulDataLen,            // numb of char_w's
                      &pIda->hEditHandle );

  /***********************************************************/
  /* Allocate area for dictionary entry data                 */
  /***********************************************************/
  if ( usRC == LX_RC_OK_ASD )
  {
    pIda->fNewEntry   = FALSE;
    UtlAlloc( (PVOID *)&pucData, 0L,
              max( (LONG)MIN_ALLOC, ulDataLen * sizeof(CHAR_W)),
              ERROR_STORAGE );
    if ( !pucData )
    {
      usRC = LX_MEM_ALLOC_ASD;
    } /* endif */
  } /* endif */

  /***********************************************************/
  /* Get dictionary entry                                    */
  /***********************************************************/
  if ( usRC == LX_RC_OK_ASD )
  {
    usRC = AsdRetEntryW( pIda->hEditHandle,
                        pIda->pLUPCB->hUCB,
                        pIda->szHeadword,
                        &ulTermNum,
                        pucData,
                        &ulDataLen,
                        &pIda->hEditHandle );
  } /* endif */

  /********************************************************************/
  /* Get dictionary handle for terms not found                        */
  /********************************************************************/
  if ( usRC == LX_WRD_NT_FND_ASD )
  {
    /******************************************************************/
    /* for associated dictionaries use first 'real' dictionary        */
    /******************************************************************/
    AsdRetDictList( pIda->pLUPCB->hDCB, ahDCB, &usNumOfDicts );
    pIda->hEditHandle = ahDCB[0];

    pIda->fNewEntry   = TRUE;
    usRC = LX_RC_OK_ASD;
  }
  else
  {
    /******************************************************************/
    /* Use current headword as org headword                           */
    /******************************************************************/
    UTF16strcpy( pIda->szOrgHeadword, pIda->szHeadword );
  } /* endif */

  /********************************************************************/
  /* Get field numbers for translation and context field if running in*/
  /* list-to-dictionary mode                                          */
  /********************************************************************/
  if ( usRC == LX_RC_OK_ASD )
  {
    pIda->pEditCB = (PDCB)pIda->hEditHandle;

    AsdRetPropPtr( pIda->pLUPCB->hUCB, pIda->hEditHandle, &pIda->pEditProp );
    QLDBFillFieldTables( pIda->pEditProp, pIda->ausNoOfFields,
                         pIda->ausFirstField );

    if ( pIda->fList2Dict  )
    {
      /****************************************************************/
      /* Address term data                                            */
      /****************************************************************/
      pTerm = *(pIda->ppTermList);
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* prepare template for add or create of LDB tree                   */
  /********************************************************************/
  if ( usRC == LX_RC_OK_ASD )
  {
    /**************************************************************/
    /* Clear template data                                        */
    /**************************************************************/
    for ( usI = 0; usI < pIda->pEditProp->usLength; usI++ )
    {
      pIda->apszFields[usI] = EMPTY_STRINGW;
    } /* endfor */

    /******************************************************************/
    /* add term data if running in list->dict mode                    */
    /******************************************************************/
    if ( pIda->fList2Dict )
    {
      /***************************************************************/
      /* add first term translation (if any)                         */
      /***************************************************************/
      if ( pIda->pEditCB->TransField.fInDict && pTerm->pszTranslation)
      {
        UTF16strncpy( pIda->ucbTermBuf, pTerm->pszTranslation,
                 (sizeof(pIda->ucbTermBuf)/sizeof(CHAR_W)-1) );
        pIda->ucbTermBuf[sizeof(pIda->ucbTermBuf)/sizeof(CHAR_W) - 1] = EOS;
        pszNextTrans = UTF16strchr( pIda->ucbTermBuf, SEMICOLON );
        if ( pszNextTrans )
        {
          *pszNextTrans = EOS;
          pszNextTrans++;
        } /* endif */
        pIda->apszFields[pIda->pEditCB->TransField.usField] = pIda->ucbTermBuf;
      }
      else
      {
        pszNextTrans = NULL;
      } /* endif */

      /***************************************************************/
      /* add term context (if any)                                   */
      /***************************************************************/
      if ( pIda->pEditCB->ContextField.fInDict && pTerm->pszContext
           && pTerm->Flags.fContext )
      {
        pIda->apszFields[pIda->pEditCB->ContextField.usField] = pTerm->pszContext;
      } /* endif */
    } /* endif */
  } /* endif */

  /***********************************************************/
  /* Convert entry to LDB tree                               */
  /***********************************************************/
  if ( usRC == LX_RC_OK_ASD )
  {
    if ( pIda->fNewEntry )
    {
      /***************************************************************/
      /* Set creation and update fields (if any)                     */
      /***************************************************************/
      if ( pIda->pEditCB->CreateDateField.fInDict     ||
           pIda->pEditCB->UpdateField.fInDict         ||
           pIda->pEditCB->TargCreateDateField.fInDict ||
           pIda->pEditCB->TargUpdateField.fInDict )
      {
        UtlTime( &lTime );
        UtlLongToDateStringW( lTime, szDateW, sizeof(szDateW)/sizeof(CHAR_W) );

        if ( pIda->pEditCB->CreateDateField.fInDict )
        {
          pIda->apszFields[pIda->pEditCB->CreateDateField.usField] = szDateW;
        } /* endif */

        if ( pIda->pEditCB->UpdateField.fInDict )
        {
          pIda->apszFields[pIda->pEditCB->UpdateField.usField] = szDateW;
        } /* endif */

        if ( pIda->pEditCB->TargCreateDateField.fInDict )
        {
          pIda->apszFields[pIda->pEditCB->TargCreateDateField.usField] = szDateW;
        } /* endif */

        if ( pIda->pEditCB->TargUpdateField.fInDict )
        {
          pIda->apszFields[pIda->pEditCB->TargUpdateField.usField] = szDateW;
        } /* endif */
      } /* endif */

      /***************************************************************/
      /* Set creation and update author fields (if any)              */
      /***************************************************************/
      if ( (pIda->pEditCB->AuthorField.fInDict     ||
            pIda->pEditCB->AuthorOfUpdateField.fInDict         ||
            pIda->pEditCB->TransAuthorField.fInDict ||
            pIda->pEditCB->TransAuthorOfUpdateField.fInDict ) &&
           (pIda->szUserIDW[0] != EOS) )
      {
        if ( pIda->pEditCB->AuthorField.fInDict )
        {
          pIda->apszFields[pIda->pEditCB->AuthorField.usField] =
          pIda->szUserIDW;
        } /* endif */

        if ( pIda->pEditCB->AuthorOfUpdateField.fInDict )
        {
          pIda->apszFields[pIda->pEditCB->AuthorOfUpdateField.usField] =
          pIda->szUserIDW;
        } /* endif */

        if ( pIda->pEditCB->TransAuthorField.fInDict )
        {
          pIda->apszFields[pIda->pEditCB->TransAuthorField.usField] =
          pIda->szUserIDW;
        } /* endif */

        if ( pIda->pEditCB->TransAuthorOfUpdateField.fInDict )
        {
          pIda->apszFields[pIda->pEditCB->TransAuthorOfUpdateField.usField] =
          pIda->szUserIDW;
        } /* endif */
      } /* endif */

      /***************************************************************/
      /* Create a new LDB tree                                       */
      /***************************************************************/
      pIda->hLdbTree = NULL;
      usLdbRC = QLDBCreateTree( pIda->ausNoOfFields, pIda->apszFields,
                                &pIda->hLdbTree );

      if ( usLdbRC != QLDB_NO_ERROR )
      {
        UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR );
        usRC = LX_MEM_ALLOC_ASD;
      }
      else
      {
        usRC = LX_RC_OK_ASD;
      } /* endif */
    }
    else
    {
      /**************************************************************/
      /* Entry exists                                               */
      /**************************************************************/

      /***************************************************************/
      /* Convert record to tree                                      */
      /***************************************************************/
      pIda->hLdbTree = NULL;
      usLdbRC = QLDBRecordToTree( pIda->ausNoOfFields, pucData,
                                  ulDataLen, &pIda->hLdbTree );

      /***************************************************************/
      /* Flatten tree                                                */
      /***************************************************************/
      if ( usLdbRC == QLDB_NO_ERROR )
      {
        usLdbRC = QLDBFlattenTree( &pIda->hLdbTree );
      } /* endif */

      /***************************************************************/
      /* Add new template if in list->dictionary mode and term data  */
      /* can be added                                                */
      /***************************************************************/
      if ( pIda->fList2Dict && ( pIda->pEditCB->TransField.fInDict ||
                                 pIda->pEditCB->ContextField.fInDict ) )
      {
        usLdbRC = QLDBAddSubtree( pIda->hLdbTree, 2,
                                  pIda->apszFields + pIda->ausFirstField[1] );
        usAddedTemplates++;
      } /* endif */

      /***************************************************************/
      /* Handle errors from LDB processing                           */
      /***************************************************************/
      if ( usLdbRC != QLDB_NO_ERROR )
      {
        switch ( usLdbRC )
        {
          case QLDB_NO_MEMORY :
            UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR );
            usRC = LX_MEM_ALLOC_ASD;
            break;
          default :
            UtlError( ERROR_ASDENTRY_CORRUPT, MB_CANCEL, 0, NULL, EQF_ERROR );
            usRC = LX_UNEXPECTED_ASD;
            break;
        } /* endswitch */
      } /* endif */

    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Add any pending translations from list2dict interface            */
  /********************************************************************/
  if ( usRC == LX_RC_OK_ASD )
  {
    while ( (pszNextTrans != NULL) && (*pszNextTrans != EOS) && (usRC == LX_RC_OK_ASD) )
    {
      // skip leading whitespace
      while ( *pszNextTrans == SPACE )
      {
        pszNextTrans++;
      } /* endwhile */

      // process translation if not empty
      if ( *pszNextTrans != EOS )
      {
        // prepare translation
        pIda->apszFields[pIda->pEditCB->TransField.usField] = pszNextTrans;
        pszNextTrans = UTF16strchr( pszNextTrans, SEMICOLON );
        if ( pszNextTrans )
        {
          *pszNextTrans = EOS;
          pszNextTrans++;
        } /* endif */

        // add new template
        usLdbRC = QLDBAddSubtree( pIda->hLdbTree, 2,
                                  pIda->apszFields + pIda->ausFirstField[1] );
        usAddedTemplates++;

        // Handle errors from LDB processing
        if ( usLdbRC != QLDB_NO_ERROR )
        {
          switch ( usLdbRC )
          {
            case QLDB_NO_MEMORY :
              usRC = LX_MEM_ALLOC_ASD;
              break;
            default :
              usRC = LX_UNEXPECTED_ASD;
              break;
          } /* endswitch */
        } /* endif */
      } /* endif */
    } /* endwhile */
  } /* endif */

  /***************************************************************/
  /* get number of templates                                     */
  /***************************************************************/
  if ( usRC == LX_RC_OK_ASD )
  {
    pIda->usTemplate = 1;
    pIda->usMaxTempl = 0;
    QLDBResetTreePositions( pIda->hLdbTree );
    do
    {
      pIda->usMaxTempl++;
      QLDBNextTemplate( pIda->hLdbTree, pIda->apszFields, &usLevel );
    } while ( usLevel != QLDB_END_OF_TREE ); /* enddo */
    QLDBResetTreePositions( pIda->hLdbTree );
  } /* endif */

  /***************************************************************/
  /* position to first added template if running in              */
  /* list->dictionary mode and term data has been added          */
  /***************************************************************/
  if ( pIda->fList2Dict &&
       ( pIda->pEditCB->TransField.fInDict ||
         pIda->pEditCB->ContextField.fInDict ) &&
       usAddedTemplates )
  {
    if ( usAddedTemplates < pIda->usMaxTempl )
    {
      pIda->usTemplate = pIda->usMaxTempl - usAddedTemplates + 1;
    } /* endif */
  } /* endif */

  /***********************************************************/
  /* handle errors                                           */
  /***********************************************************/
  switch (usRC)
  {
    case LX_RC_OK_ASD:
      break;

    default:
      AsdRCHandling( usRC, pIda->hEditHandle, NULL, NULL, pIda->szHeadword, NULLHANDLE );
      break;
  } /* endswitch */

  fAnsiConv = CheckLanguages ( pIda );

  /********************************************************************/
  /* Prepare display of entry                                         */
  /********************************************************************/
  if ( usRC == LX_RC_OK_ASD )
  {
    if ( pIda->fNewEntry )
    {
      // for a new entry hide the dictionary name static
      WinShowWindow( WinWindowFromID( pIda->hwndDlg, EDIT_ENTRY_STEXT_DICT ), FALSE );
      // and disable the delete pushbutton
      ENABLECTRL( pIda->hwndDlg, EDIT_ENTRY_PUBO_DEL_ENTRY, FALSE );
      // and the delete template pushbutton also
      ENABLECTRL( pIda->hwndDlg, EDIT_ENTRY_PUBO_DEL_TEMPL, FALSE );
    }
    else
    {
      AsdQueryDictName( pIda->hEditHandle, pIda->szTempBuf );

      CONDOEMTOANSI( fAnsiConv, pIda->szTempBuf);

      WinSetWindowText( WinWindowFromID( pIda->hwndDlg,
                                         EDIT_ENTRY_STEXT_DICTNAME ),
                        pIda->szTempBuf );
    } /* endif */

    pIda->fDataChanged = FALSE;

    if (  pIda->usMaxTempl == 1 )
    {
      ENABLECTRL( pIda->hwndDlg, EDIT_ENTRY_PUBO_DEL_TEMPL, FALSE );
    } /* endif */

    pIda->fChangedByProgram = TRUE;


    SETTEXTHWNDW( pIda->hwndHeadword, pIda->szHeadword );

    pIda->fChangedByProgram = FALSE;
  if ( pIda->pucHeadwordName )
    {
    SETTEXTW( pIda->hwndDlg, EDIT_ENTRY_STEXT_HEADWORD, pIda->pucHeadwordName );
  }
   //GQTEST WinSendMsg (pIda->hwndHeadword, EM_SETSEL, NULL, NULL);
  } /* endif */

  /********************************************************************/
  /* cleanup                                                          */
  /********************************************************************/
  if ( pucData )            UtlAlloc( (PVOID *)&pucData, 0L, 0L, FALSE );

  return( (usRC == LX_RC_OK_ASD) ? TRUE : FALSE );
} /* end of function EditGetEntry */

/**********************************************************************/
/* Local macro to reposition a dialog control                         */
/**********************************************************************/
#define MOVECONTROL( hwnd, id, cxDelta ) \
{ \
   WinQueryWindowPos( WinWindowFromID( hwnd, id ), &swp ); \
   swp.x = (SHORT)(swp.x + cxDelta); \
   WinSetWindowPos( WinWindowFromID( hwnd, id ), HWND_TOP, swp.x, \
                    swp.y, swp.cx, swp.cy, EQF_SWP_MOVE );   \
}

/**********************************************************************/
/* Calculate and set size and positions of dialog controls            */
/**********************************************************************/
BOOL    EditCalcDlgControls( PEDITIDA pIda )
{
  BOOL             fOK = TRUE;         // function return code
  HPS              hpsWindow;          // window presentation space handle
  LONG             cxStatic;           // max x size of field name static
  SWP              swpDialog;          // size and position of edit dialog
  SHORT            cxScreen, cyScreen; // size of physical screen
  SWP              swp;                // general size and position buffer
  ULONG            ulTermNum;          // term number
  ULONG            ulDataLen;          // length of term data record
  USHORT           usRC;               // return code from ASD-calls
  SHORT            sPBOffset;          // offset for pushbuttons
  PPROFENTRY       pProfEntry;         // ptr to dictionary profile entry
  USHORT           usField;            // index for dictionary profile processing
  HDCB             ahDCB[MAX_DICTS+1]; // list of associated dictionaries
  USHORT           usNumOfDicts;       // number of dictionaries in association
  HFONT            hFont, hOldFont;    // buffer for font handles

  /********************************************************************/
  /* Get dictionary entry (in order to get the dictionary to be used  */
  /* for the edit dialog)                                             */
  /********************************************************************/
  usRC = AsdFndMatch( pIda->szHeadword,
                      pIda->hEditHandle,
                      pIda->pLUPCB->hUCB,
                      pIda->szHeadword,
                      &ulTermNum,
                      &ulDataLen,                   //numb of char_w's
                      &pIda->hEditHandle );
  fOK = (usRC == LX_RC_OK_ASD );

  /********************************************************************/
  /* Get dictionary handle for terms not found                        */
  /********************************************************************/
  if ( usRC == LX_WRD_NT_FND_ASD )
  {
    /******************************************************************/
    /* for associated dictionaries use first 'real' dictionary        */
    /******************************************************************/
    AsdRetDictList( pIda->pLUPCB->hDCB, ahDCB, &usNumOfDicts );
    pIda->hEditHandle = ahDCB[0];
    fOK = TRUE;
  } /* endif */

  /********************************************************************/
  /* Get pointer to dictionary properties                             */
  /********************************************************************/
  if ( fOK )
  {
    usRC = AsdRetPropPtr( NULL, pIda->hEditHandle, &pIda->pEditProp );
    fOK = (usRC == LX_RC_OK_ASD );
  } /* endif */

  /********************************************************************/
  /* Query font metrics info, window handles and other stuff          */
  /********************************************************************/
  if ( fOK )
  {
    pIda->hwndGrBox = WinWindowFromID( pIda->hwndDlg, EDE_GB_EF_ID);
    WinQueryWindowPos( pIda->hwndGrBox, &(pIda->swpGrBox));

    pIda->hab           = WinQueryAnchorBlock( pIda->hwndDlg );
    pIda->hwndHeadword  = WinWindowFromID (pIda->hwndDlg, EDIT_ENTRY_EDIT_HEADWORD);
    pIda->hwndPaste     = WinWindowFromID (pIda->hwndDlg, EDIT_ENTRY_PUBO_PASTE);
    pIda->hwndCopy      = WinWindowFromID (pIda->hwndDlg, EDIT_ENTRY_PUBO_COPY);

    hpsWindow = GETPS( pIda->hwndHeadword );
    hFont = (HFONT)SendMessage( pIda->hwndHeadword, WM_GETFONT, 0, 0L );
    hOldFont = (HFONT) SelectObject( hpsWindow, hFont );
    GetCharXY( hpsWindow, (PUSHORT)(&pIda->sCharWidth), (PUSHORT)(&pIda->sCharHeight) );
    SelectObject( hpsWindow, hOldFont );
    RELEASEPS( pIda->hwndHeadword, hpsWindow );

    pIda->sCtrlDistance = pIda->sCharHeight / 4;  // Y distance between controls
    pIda->usBorderSize  = (USHORT) WinQuerySysValue (HWND_DESKTOP, SV_CXBORDER);
    pIda->sQuartChar    = pIda->sCharHeight / 4;
    pIda->sHalfChar     = pIda->sCharHeight / 2;
  } /* endif */

  /*******************************************************************/
  /* Get width of field name static                                  */
  /*******************************************************************/
  hpsWindow = GETPS( pIda->hwndHeadword );
  hFont = (HFONT)SendMessage( pIda->hwndHeadword, WM_GETFONT, 0, 0L );
  hOldFont = (HFONT) SelectObject( hpsWindow, hFont );
  pProfEntry  = pIda->pEditProp->ProfEntry;
  cxStatic    = 0L;
  for ( usField = 0; usField < pIda->pEditProp->usLength; usField++ )
  {
    LONG lCX, lCY;

    TEXTSIZE( hpsWindow, pProfEntry->chUserName, lCX, lCY );
    cxStatic = max( cxStatic, (lCX+2L) );
    pProfEntry++;
  } /* endfor */
  SelectObject( hpsWindow, hOldFont );
  RELEASEPS( pIda->hwndHeadword, hpsWindow );

  /*******************************************************************/
  /* Compute static enlargement value                                */
  /*******************************************************************/
  if ( (STATIC_WIDTH_CHARS * pIda->sCharWidth) >= (SHORT)cxStatic )
  {
    pIda->lStaticEnlargement = 0;
  }
  else
  {
    pIda->lStaticEnlargement = cxStatic -
                               (STATIC_WIDTH_CHARS * pIda->sCharWidth);
  } /* endif */

  /*******************************************************************/
  /* restrict enlargement value to desktop size                      */
  /*******************************************************************/
  {
    WinQueryWindowPos( pIda->hwndDlg, &swpDialog );
    cxScreen = (SHORT)WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    cyScreen = (SHORT)WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );
    if ( (swpDialog.cx + pIda->lStaticEnlargement) > cxScreen )
    {
      pIda->lStaticEnlargement = max( 0, (cxScreen - swpDialog.cx) );
    } /* endif */
    pIda->swpGrBox.cx = (SHORT)(pIda->swpGrBox.cx + pIda->lStaticEnlargement);
  }

  /********************************************************************/
  /* Resize/Reposition dialog window                                  */
  /********************************************************************/
  if ( pIda->lStaticEnlargement != 0 )
  {
    swpDialog.cx = (SHORT)(swpDialog.cx + pIda->lStaticEnlargement);
    WinSetWindowPos( pIda->hwndDlg, HWND_TOP,
                     (SHORT)((cxScreen - swpDialog.cx) >> 1),
                     (SHORT)((cyScreen - swpDialog.cy) >> 1),
                     (SHORT)swpDialog.cx,
                     (SHORT)swpDialog.cy,
                     EQF_SWP_MOVE | EQF_SWP_SIZE );
  } /* endif */

  /********************************************************************/
  /* Resize/reposition dialog controls (only if default dialog width  */
  /* had to be changed)                                               */
  /********************************************************************/
  if ( pIda->lStaticEnlargement != 0 )
  {
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_SCROLL_ENTRY,
                 pIda->lStaticEnlargement );
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_PUBO_PREV_TEMPL,
                 pIda->lStaticEnlargement );
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_PUBO_NEXT_TEMPL,
                 pIda->lStaticEnlargement );
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_PUBO_DEL_TEMPL,
                 pIda->lStaticEnlargement );
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_PUBO_ADD_TEMPL,
                 pIda->lStaticEnlargement );
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_PUBO_PASTE,
                 pIda->lStaticEnlargement );
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_PUBO_COPY,
                 pIda->lStaticEnlargement );
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_PUBO_COPY_TEMPL,
                 pIda->lStaticEnlargement );
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_GRBOX_TEMPLATE,
                 pIda->lStaticEnlargement );
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_GRBOX_CLIPBOARD,
                 pIda->lStaticEnlargement );
    WinSetWindowPos( pIda->hwndGrBox, HWND_TOP,
                     pIda->swpGrBox.x, pIda->swpGrBox.y,
                     pIda->swpGrBox.cx, pIda->swpGrBox.cy, EQF_SWP_SIZE );

    /******************************************************************/
    /* Re-align pushbuttons at bottom of dialog window                */
    /******************************************************************/
    sPBOffset =  (SHORT)(pIda->lStaticEnlargement / 4);
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_PUBO_SAVE,
                 (sPBOffset >> 1) );
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_PUBO_DEL_ENTRY,
                 (sPBOffset >> 1) + sPBOffset );
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_PUBO_CANCEL,
                 (sPBOffset >> 1) + (2 * sPBOffset) );
    MOVECONTROL( pIda->hwndDlg, EDIT_ENTRY_PUBO_HELP,
                 (sPBOffset >> 1) + (3 * sPBOffset) );
  } /* endif */

  return( fOK );
} /* end of function EditCalcDlgControls */


LRESULT CALLBACK EditSubclassProc( HWND hwnd, UINT msg, WPARAM mp1, LPARAM mp2, UINT_PTR uIdSubclass, DWORD_PTR dwRefData )
{
  MRESULT mResult;

  switch ( msg )
  {
    case WM_GETDLGCODE:
      return( DefSubclassProc( hwnd, msg, mp1, mp2 ) | DLGC_WANTTAB );

      /******************************************************************/
      /* Process TAB and BACKTAB using the EditChar function            */
      /******************************************************************/
    case WM_KEYDOWN:
      if ( (mp1 == VK_TAB) ||
           (mp1 == VK_UP)  ||
           (mp1 == VK_DOWN) )
      {
        LONG  lStyle;                  // style of edit control
        HWND  hwndDlg;                 // handle if dialog window

        hwndDlg = GetParent( hwnd );   // get dialog handle

        /**************************************************************/
        /* Convert SHIFT+TAB to BACKTAB                               */
        /**************************************************************/
        if ( (mp1 == VK_TAB) && (GetKeyState(VK_SHIFT) & 0x8000) )
        {
          mp1 = VK_BACKTAB;
        } /* endif */

        /**************************************************************/
        /* Get style of entry field                                   */
        /**************************************************************/
        lStyle = GetWindowLong( hwnd, GWL_STYLE );

        /**************************************************************/
        /* For multiline entry fields pass up and down on to          */
        /* control otherwise activate EditChar to process the key     */
        /**************************************************************/
        if ( ((mp1 == VK_UP) || (mp1 == VK_DOWN)) && (lStyle & ES_MULTILINE) )
        {
          return DefSubclassProc( hwnd, msg, mp1, mp2 );
        }
        else
        {
          EditChar( hwndDlg, (SHORT)mp1 );
        } /* endif */
        return( 0L );
      } /* endif */
      break;
  } /* endswitch */
  return DefSubclassProc( hwnd, msg, mp1, mp2 );
} /* end of function EditSubclassProc */



static BOOL CheckLanguages
(
        PEDITIDA pIda
)
{
        BOOL fAnsiConv;

  if (stricmp(pIda->pEditProp->szSourceLang, THAI_STR ) == 0)
  {
          fAnsiConv = FALSE;
  }
  else if ( GetCharSet() == THAI_CHARSET )
  {
          fAnsiConv = FALSE;
  }
  else
  {
          fAnsiConv = TRUE;
  }

  return (fAnsiConv);
}  /* end of function CheckLanguages */

BOOL ReplaceWithUnicodeField
( 
  HWND hwndDlg,                        // window handle of dialog window
  int  iEntryFieldID                   // ID of entry field
)
{
  WINDOWPLACEMENT Placement;
  WINDOWINFO Info;

  // get values from window being replaced
  HWND hwndEntryField = GetDlgItem( hwndDlg, iEntryFieldID );
  GetWindowPlacement( hwndEntryField, &Placement );
  GetWindowInfo( hwndEntryField, &Info );

  // destroy the window
  DestroyWindow( hwndEntryField );

  // create a new one
  hwndEntryField = CreateWindowExW( 0, L"Edit", L"", Info.dwStyle, 
                               Placement.rcNormalPosition.left, Placement.rcNormalPosition.top,
                               Placement.rcNormalPosition.right - Placement.rcNormalPosition.left,
                               Placement.rcNormalPosition.bottom - Placement.rcNormalPosition.top,
                               hwndDlg, (HMENU)iEntryFieldID, (HINSTANCE)(HAB)UtlQueryULong( QL_HAB ), 0 );

  if ( hwndEntryField != NULL )
  {
    SetWindowLong( hwndEntryField, GWL_ID, iEntryFieldID );
    SetWindowPos( hwndEntryField, HWND_TOP, 0, 0, Placement.rcNormalPosition.right - Placement.rcNormalPosition.left,
      Placement.rcNormalPosition.bottom - Placement.rcNormalPosition.top, SWP_NOACTIVATE | SWP_NOMOVE );
  }

  return( hwndEntryField != NULL );
}
