/*! \file
	Description: EQF Folder Handler Global Find and Replace (GFR) function
               Result Area Painting related code

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TM               // Translation Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file
#include "eqfdde.h"               // batch mode definitions
#include "eqffol00.h"             // our .h stuff
#include "eqffol.id"              // our ID file
#include "eqftmi.h"               // TM internal definitions
#include "EQFHLOG.H"            // Translation Processor priv. include file
#include "eqflp.h"
#include <windowsx.h>
#include <commctrl.h>

#include <process.h>              /* _beginthread, _endthread */
  #include <direct.h>

#include "eqfutmdi.h"           // MDI utilities
#include "richedit.h"           // MDI utilities

#include "OtmProposal.h"
#include "core\memory\MemoryFactory.h"
#include "cxmlwriter.h"

#include "eqfgfr.h"




BOOL GFR_ResizeColumns
( 
  HWND        hwnd,                    // dialog window handle
  PFOLFINDDATA pIda                     // dialog IDA
)
{
  HWND hwndList = GetDlgItem( hwnd, ID_FOLFIND_RESULT_LISTBOX );
  RECT rect;

  GetWindowRect( hwndList, &rect );

  switch ( pIda->CurDisplayMode )
  {
    case SOURCE_AND_TARGET_COLUMN_MODE:
      // evenly use available space for source and target column
      pIda->aiListViewColWidth[2] = 
      pIda->aiListViewColWidth[3] = (rect.right - rect.left - DOCNUM_COL_WIDTH - SEGNUM_COL_WIDTH)/2;  
      break;

    case ONLY_SOURCE_COLUMN_MODE:
    case ONLY_TARGET_COLUMN_MODE:
      // use all available space for source/target column
      pIda->aiListViewColWidth[2] = rect.right - rect.left - DOCNUM_COL_WIDTH - SEGNUM_COL_WIDTH;           // use remaining width for this column
      break;
  } /* endswich */

  // force a repaint of the header and the result list
  SETTEXT( hwnd, ID_FOLFIND_HEADER_TEXT, "y" );
  GFR_ForceRefreshOfItemHeights( pIda );
  return( TRUE );
}


BOOL GFR_AdjustResultColumns
( 
  HWND        hwnd,                    // dialog window handle
  PFOLFINDDATA pIda                     // dialog IDA
)
{
  DISPLAY_COLUMNS_MODE NewDisplayMode;
  HWND hwndList = GetDlgItem( hwnd, ID_FOLFIND_RESULT_LISTBOX );
  RECT rect;

  GetWindowRect( hwndList, &rect );

  if ( QUERYCHECK( hwnd, ID_FOLFIND_SEARCHBOTH_RB ) || QUERYCHECK( hwnd, ID_FOLFIND_SHOWSOURCEANDTARGET_CHK ) )
  {
    NewDisplayMode = SOURCE_AND_TARGET_COLUMN_MODE;
  }
  else if ( QUERYCHECK( hwnd, ID_FOLFIND_SEARCHSOURCE_RB ) )
  {
    NewDisplayMode = ONLY_SOURCE_COLUMN_MODE;
  }
  else
  {
    NewDisplayMode = ONLY_TARGET_COLUMN_MODE;
  }

  // create first three columns if not done yet
  if ( pIda->CurDisplayMode == UNDEFINED_COLUMN_MODE )
  {
    pIda->aiListViewColWidth[0] = DOCNUM_COL_WIDTH;
    pIda->aiListViewColWidth[1] = SEGNUM_COL_WIDTH;
    pIda->aiListViewColWidth[2] = rect.right - rect.left - DOCNUM_COL_WIDTH - SEGNUM_COL_WIDTH;           // use remaining width for this column
    pIda->CurDisplayMode = ONLY_TARGET_COLUMN_MODE;

  }

  // do we have to change the columns currently displayed?
  if ( pIda->CurDisplayMode != NewDisplayMode )
  {
    if ( NewDisplayMode == SOURCE_AND_TARGET_COLUMN_MODE )
    {
      // compute available width per column
      LONG lWidth = (rect.right - rect.left - DOCNUM_COL_WIDTH - SEGNUM_COL_WIDTH) / 2; 

      pIda->aiListViewColWidth[2] = lWidth;
      pIda->aiListViewColWidth[3] = lWidth;
    }
    else
    {
      // remove fouth column if necessary
      if ( pIda->CurDisplayMode == SOURCE_AND_TARGET_COLUMN_MODE )
      {
        pIda->aiListViewColWidth[2] = rect.right - rect.left - DOCNUM_COL_WIDTH - SEGNUM_COL_WIDTH;           // use remaining width for this column
      }
    }
    pIda->CurDisplayMode = NewDisplayMode;

    // force a repaint of the header and the result list
    SETTEXT( hwnd, ID_FOLFIND_HEADER_TEXT, "z" );
    GFR_ForceRefreshOfItemHeights( pIda );
  }
  return( TRUE );
}


void DrawItemColumn( PFOLFINDDATA pIda, HDC hdc, int iColumn, PSZ_W pszString, LPRECT prcClip, PFOLFINDRESULTENTRY pEntry, PFOLFINDRESULTPOS pFoundPos, int iFoundEntries, BOOL fTarget, int iBaseColorIndex );

//
//  FUNCTION:   AddListViewItems(HWND)
//
//  PURPOSE:    Adds the initial items to the listview control.




//
//  FUNCTION:   GFR_DrawItem(const LPDRAWITEMSTRUCT)
//
//  PURPOSE:    Draws one item in the listview control.
//
//  PARAMETERS:
//      lpDrawItem - Pointer to the information needed to draw the item.  The
//                   item number is in the itemID member.
//

// handle WM_DRAWITEM message for our ownerdrawn listbox
void GFR_DrawItem( PFOLFINDDATA pIda, LPDRAWITEMSTRUCT lpDrawItem )
{
  WCHAR szString[256];
  UINT uFirstColWidth;
  RECT rcClip;
  int iBaseColorIndex = 0;

  HFONT hFontOld = NULL;
  
  if ( pIda->hFontControl ) hFontOld = (HFONT)SelectObject( lpDrawItem->hDC, pIda->hFontControl );

  if ( lpDrawItem->hwndItem == GetDlgItem( pIda->hwndPages[0], ID_FOLFIND_HEADER_TEXT ) )
  {
    PSZ_W pszText;

    // only handle normal draw operations
    if ( (lpDrawItem->itemAction != ODA_FOCUS) && (lpDrawItem->itemAction != ODA_SELECT) && (lpDrawItem->itemAction != ODA_DRAWENTIRE) ) return;
   
    DWORD dwBackColor = pIda->ColorData.ColorSetting[iBaseColorIndex].cBackground;
    SetBkColor( lpDrawItem->hDC, dwBackColor );
    SetTextColor( lpDrawItem->hDC, pIda->ColorData.ColorSetting[iBaseColorIndex].cForeground );

    // erase rectangle
    {
      HBRUSH hBrush = CreateSolidBrush( dwBackColor );
      FillRect( lpDrawItem->hDC, &(lpDrawItem->rcItem), hBrush ); 
      DeleteObject( hBrush );
    }

    // Set up the new clipping rect for the first column text and draw it
    rcClip.left = lpDrawItem->rcItem.left;
    rcClip.right = lpDrawItem->rcItem.left + pIda->aiListViewColWidth[0];
    rcClip.top = lpDrawItem->rcItem.top;
    rcClip.bottom = lpDrawItem->rcItem.bottom;
    GFR_DrawFrame( lpDrawItem->hDC, &rcClip );
    pszText = L"Document";
    ExtTextOutW( lpDrawItem->hDC, rcClip.left + 2, rcClip.top + 1, ETO_CLIPPED, &rcClip, pszText, wcslen(pszText), NULL);

    // draw segment number column
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + pIda->aiListViewColWidth[1];
    GFR_DrawFrame( lpDrawItem->hDC, &rcClip );
    pszText = L"Segment";
    ExtTextOutW( lpDrawItem->hDC, rcClip.left + 2, rcClip.top + 1, ETO_CLIPPED, &rcClip, pszText, wcslen(pszText), NULL);

    // draw source or target column header
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + pIda->aiListViewColWidth[2];
    GFR_DrawFrame( lpDrawItem->hDC, &rcClip );
    pszText = (pIda->CurDisplayMode == ONLY_TARGET_COLUMN_MODE ) ? L"Target" : L"Source";
    ExtTextOutW( lpDrawItem->hDC, rcClip.left + 2, rcClip.top + 1, ETO_CLIPPED, &rcClip, pszText, wcslen(pszText), NULL);

    // target column (only when 4 columns are displayed)
    if ( pIda->CurDisplayMode == SOURCE_AND_TARGET_COLUMN_MODE )
    {
      rcClip.left = rcClip.right;
      rcClip.right = rcClip.left + pIda->aiListViewColWidth[3];
      GFR_DrawFrame( lpDrawItem->hDC, &rcClip );
      pszText = L"Target";
      ExtTextOutW( lpDrawItem->hDC, rcClip.left + 2, rcClip.top + 1, ETO_CLIPPED, &rcClip, pszText, wcslen(pszText), NULL);
    }
  }
  else
  {
    // If there are no list box items, skip this message. 
    if (lpDrawItem->itemID == -1) return; 

    // only handle normal draw operations
    if ( (lpDrawItem->itemAction != ODA_FOCUS) && (lpDrawItem->itemAction != ODA_SELECT) && (lpDrawItem->itemAction != ODA_DRAWENTIRE) ) return;

    iBaseColorIndex = GFR_NORMAL_ENTRY;
    if ( lpDrawItem->itemState & ODS_FOCUS ) 
    { 
      iBaseColorIndex = GFR_FOCUS_ENTRY;
    } 
    else if ( lpDrawItem->itemState & ODS_SELECTED ) 
    { 
      iBaseColorIndex = GFR_SELECTED_ENTRY;
    } 
    SetBkColor( lpDrawItem->hDC, pIda->ColorData.ColorSetting[iBaseColorIndex].cBackground );

    //// erase rectangle
    //{
    //  HBRUSH hBrush = CreateSolidBrush( dwBackColor );
    //  FillRect( lpDrawItem->hDC, &(lpDrawItem->rcItem), hBrush ); 
    //  DeleteObject( hBrush );
    //}

    // get thre result entry for the painted item
    PFOLFINDRESULTENTRY pEntry = (PFOLFINDRESULTENTRY) QUERYITEMHANDLEHWND( pIda->hResultListBox, lpDrawItem->itemID );

    // Calculate the width of the first column.
    uFirstColWidth = pIda->aiListViewColWidth[0];

    // Set up the new clipping rect for the first column text and draw it
    //ListView_GetSubItemRect( pIda->hListView, lpDrawItem->itemID, 0, LVIR_BOUNDS, &rcClip);
    rcClip.left = lpDrawItem->rcItem.left;
    rcClip.right = lpDrawItem->rcItem.left + pIda->aiListViewColWidth[0];
    rcClip.top = lpDrawItem->rcItem.top;
    rcClip.bottom = lpDrawItem->rcItem.bottom;
    swprintf( szString, L"%ld", pEntry->iDocIndex + 1 );
    DrawItemColumn( pIda, lpDrawItem->hDC, 0, szString, &rcClip, pEntry, NULL, 0, FALSE, iBaseColorIndex );

    // draw segment number column
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + pIda->aiListViewColWidth[1];
    swprintf( szString, L"%lu", pEntry->ulSegNum );
    DrawItemColumn( pIda, lpDrawItem->hDC, 1, szString, &rcClip, pEntry, NULL, 0, FALSE, iBaseColorIndex );

    // draw source or target text
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + pIda->aiListViewColWidth[2];
    //swprintf( szString, L"ClipRect Left=%ld Top=%ld Right=%ld Bottom=%ld", rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );

    PSZ_W pszData = NULL;
    BOOL fTarget = FALSE;
    PFOLFINDRESULTPOS pFoundPos = (PFOLFINDRESULTPOS)((PBYTE)pEntry + pEntry->iResultPosOffs );
    switch ( pIda->CurDisplayMode )
    {
      case SOURCE_AND_TARGET_COLUMN_MODE:
      case ONLY_SOURCE_COLUMN_MODE:
        pszData = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[SOURCE_TEXT]);
        fTarget = FALSE;
        break;

      case ONLY_TARGET_COLUMN_MODE:
        pszData = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[TARGET_TEXT]);
        fTarget = TRUE;
        break;
    } /* endswich */
    //  FillRect ( lpDrawItem->hDC, &rcClip, (HBRUSH) GetSysColor( COLOR_HIGHLIGHT ) );
    if ( pszData != NULL ) DrawItemColumn( pIda, lpDrawItem->hDC, 2, pszData, &rcClip, pEntry, pFoundPos, pEntry->iUsedEntries, fTarget, iBaseColorIndex );


    // target column (only when 4 columns are displayed)
    if ( pIda->CurDisplayMode == SOURCE_AND_TARGET_COLUMN_MODE )
    {
      rcClip.left = rcClip.right;
      rcClip.right = rcClip.left + pIda->aiListViewColWidth[3];
      //swprintf( szString, L"ClipRect Left=%ld Top=%ld Right=%ld Bottom=%ld", rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
      pszData = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[TARGET_TEXT]);
      FillRect ( lpDrawItem->hDC, &rcClip, (HBRUSH) GetSysColor( COLOR_GRADIENTINACTIVECAPTION ) );
      DrawItemColumn( pIda, lpDrawItem->hDC, 3, pszData, &rcClip, pEntry, pFoundPos, pEntry->iUsedEntries, TRUE, iBaseColorIndex );
    }

    // draw focus rectangle if the item the focus
    if (lpDrawItem->itemState & ODS_FOCUS) 
    { 
        DrawFocusRect( lpDrawItem->hDC, &(lpDrawItem->rcItem) ); 
    } 
  } /* endif */

  if ( hFontOld ) SelectObject( lpDrawItem->hDC, hFontOld );

  return; 
} 

BOOL IsLineSplitChar( CHAR_W c ) 
{
  return( iswspace( c) || (c == L'-') || (c == L'/') || (c == L'.') || (c == L',') );

}

// get text height in number of lines
int GetTextHeight( HDC hdc, int iAvail, PSZ_W pszString, int iLen )
{
  SIZE size;
  int iLines = 0;

  do 
  {
    // check if text fits into available space
    GetTextExtentPoint32W( hdc, pszString, iLen, &size );

    // if there is not enough room truncate text until it fits
    if ( size.cx > iAvail )
    {
      int i = iLen - 1;
      BOOL fTextFits = FALSE;
      do
      {
        // find preceeding whitespace char
        while ( (i > 0) && !IsLineSplitChar( pszString[i] ) ) i--;

        // split text at found position and test if it fits
        if ( i != 0 ) 
        {
          GetTextExtentPoint32W( hdc, pszString, i, &size );
          if ( size.cx > iAvail )
          {
            // try a shorter string
            i--;
          }
          else
          {
            fTextFits = TRUE;
          }
        }
      } while ( (i != 0) && !fTextFits );

      if ( fTextFits )
      {
        // go to next line
        iLines++;

        // continue with remaining text
        pszString += i;
        iLen -= i;
      }
      else
      {
        // we have to clip the text
        iLines++;
        iLen = 0;
      }
    }
    else
    {
      // text fits into clipping rectangle
      iLines++;
      iLen = 0;
    } /* endif */
  } while ( iLen != 0 );
  return( iLines );
}

void GFR_SetTextAndBackgroundStyle( PFOLFINDDATA pIda, HDC hdc, RECT *prc, DRAWTYPE DrawType, int iBaseColorIndex )
{
  int iColorIndex = 0;

  prc; 

  switch( DrawType )
  {
    case NORMALTEXT_DRAWTYPE:     iColorIndex = iBaseColorIndex + GFR_NORMAL_TEXT; break;
    case FOUND_DRAWTYPE:          iColorIndex = iBaseColorIndex + GFR_FOUND_TEXT; break;
    case TOBECHANGED_DRAWTYPE:    iColorIndex = iBaseColorIndex + GFR_TOBECHANGED_TEXT; break;
    case CHANGED_DRAWTYPE:        iColorIndex = iBaseColorIndex + GFR_CHANGED_TEXT; break;
    case CHANGETO_DRAWTYPE:       iColorIndex = iBaseColorIndex + GFR_CHANGETO_TEXT; break;
    case CHANGEDTO_DRAWTYPE:      iColorIndex = iBaseColorIndex + GFR_CHANGEDTO_TEXT; break;
    case ADDSEGMENTTEXT_DRAWTYPE: iColorIndex = iBaseColorIndex + GFR_ADDSEGMENTDATA_TEXT; break;
  } /* endswitch */

  SetTextColor( hdc, pIda->ColorData.ColorSetting[iColorIndex].cForeground );
  SetBkColor( hdc, pIda->ColorData.ColorSetting[iColorIndex].cBackground );

  return;
}

// draw text and handle any linebreaks within the text
void GFR_DrawMultiLineText( PFOLFINDDATA pIda, HDC hdc, PRECT prcClip, PPOINT pPt, PSZ_W pszString, int iLen, int iLineHeight, int iBaseColorIndex,  DRAWTYPE DrawType )
{
  if ( pIda->fRespectLineFeeds )
  {
    // copy the string to our buffer and use the text in the buffer
    wcsncpy( pIda->szColTextBuffer, pszString, iLen );
    pIda->szColTextBuffer[iLen] = 0;
    pszString = pIda->szColTextBuffer;

    // split text at line feeds and draw the resulting text lines
    PSZ_W pszLineFeed = NULL;
    do
    {
      pszLineFeed = wcschr( pszString, L'\n' );
      if ( pszLineFeed != NULL )
      {
        // draw current line of text
        int iLineLen = pszLineFeed - pszString;
        if ( iLineLen > 0 ) GFR_DrawText( pIda, hdc, prcClip, pPt, pszString, iLineLen, iLineHeight, iBaseColorIndex, DrawType );

        // go to next line
        pPt->x = prcClip->left + 2;
        pPt->y += iLineHeight;

        // process the rest of the text
        pszString = pszLineFeed + 1;
        iLen -= iLineLen + 1;

        if ( *pszString == 0 ) iLen = 0;
      }
      else
      {
        // draw remaining text
        if ( iLen > 0 ) GFR_DrawText( pIda, hdc, prcClip, pPt, pszString, iLen, iLineHeight, iBaseColorIndex, DrawType );
      } /* endif */
    } while ( (pszLineFeed != NULL) && (iLen != 0) ); /* endwhile */
  }
  else
  {
    // copy string to our buffer and replace LFs with blank before drawing the text
    wcsncpy( pIda->szColTextBuffer, pszString, iLen );
    pIda->szColTextBuffer[iLen] = 0;
    for( int i = 0; i < iLen; i++ )
    {
      if ( (pIda->szColTextBuffer[i] == L'\n') || (pIda->szColTextBuffer[i] == L'\r') ) pIda->szColTextBuffer[i] = L' ';
    } /* endfor */

    GFR_DrawText( pIda, hdc, prcClip, pPt, pIda->szColTextBuffer, iLen, iLineHeight, iBaseColorIndex, DrawType );
  } /* endif */
}

void GFR_DrawText( PFOLFINDDATA pIda, HDC hdc, PRECT prcClip, PPOINT pPt, PSZ_W pszString, int iLen, int iLineHeight, int iBaseColorIndex,  DRAWTYPE DrawType )
{
  SIZE size;
  RECT rc;

  do 
  {
    // check if text fits into available space
    GetTextExtentPoint32W( hdc, pszString, iLen, &size );
    rc.top = pPt->y;
    rc.bottom = pPt->y + size.cy;
    rc.left = pPt->x;
    rc.right = pPt->x + size.cx;

    // if there is not enough room truncate text until it fits
    if ( rc.right > prcClip->right )
    {
      int i = iLen - 1;
      BOOL fTextFits = FALSE;
      do
      {
        // find preceeding whitespace char
        while ( (i > 0) && !IsLineSplitChar( pszString[i] ) ) i--;

        // split text at found position and test if it fits
        if ( i != 0 ) 
        {
          GetTextExtentPoint32W( hdc, pszString, i, &size );
          if ( (pPt->x + size.cx) > prcClip->right )
          {
            // try a shorter string
            i--;
          }
          else
          {
            fTextFits = TRUE;
          }
        }
      } while ( (i != 0) && !fTextFits );

      if ( fTextFits )
      {
        // draw fitting part of text
        rc.top = pPt->y;
        rc.bottom = pPt->y + size.cy;
        rc.left = pPt->x;
        rc.right = pPt->x + size.cx;

        GFR_SetTextAndBackgroundStyle( pIda, hdc, &rc, DrawType, iBaseColorIndex );
        ExtTextOutW( hdc, pPt->x, pPt->y, (DrawType == NORMALTEXT_DRAWTYPE ) ? ETO_CLIPPED : (ETO_CLIPPED | ETO_OPAQUE), &rc, pszString, i, NULL);
        // apply strike through
        if ( DrawType == CHANGED_DRAWTYPE )
        {
          int iXPos = rc.top + ((rc.bottom - rc.top) / 2) + 1;
          MoveToEx( hdc, rc.left, iXPos, (LPPOINT) NULL); 
          LineTo( hdc, rc.right, iXPos );
        } /* endif */

        // go to next line
        pPt->x = prcClip->left + 2;
        pPt->y += iLineHeight;

        // continue with remaining text
        pszString += i;
        iLen -= i;
      }
      else if ( size.cx < (prcClip->right - prcClip->left - 2) )
      {
        // draw text in next line
        // go to next line
        pPt->x = prcClip->left + 2;
        pPt->y += iLineHeight;
      }
      else
      {
        // draw text but clip it 
        rc.top = pPt->y;
        rc.bottom = pPt->y + size.cy;
        rc.left = pPt->x;
        rc.right = pPt->x + size.cx;
        if ( rc.right > prcClip->right )  rc.right = prcClip->right;
        GFR_SetTextAndBackgroundStyle( pIda, hdc, &rc, DrawType, iBaseColorIndex );
        ExtTextOutW( hdc, pPt->x, pPt->y, (DrawType == NORMALTEXT_DRAWTYPE ) ? ETO_CLIPPED : (ETO_CLIPPED | ETO_OPAQUE), &rc, pszString, iLen, NULL);
        // apply strike through
        if ( DrawType == CHANGED_DRAWTYPE )
        {
          int iXPos = rc.top + ((rc.bottom - rc.top) / 2) + 1;
          MoveToEx( hdc, rc.left, iXPos, (LPPOINT) NULL); 
          LineTo( hdc, rc.right, iXPos );
        } /* endif */

        // go to next line
        pPt->x = prcClip->left + 2;
        pPt->y += iLineHeight;

        // continue with remaining text
        iLen = 0;
      }
    }
    else
    {
      // draw remaining text
      GFR_SetTextAndBackgroundStyle( pIda, hdc, &rc, DrawType, iBaseColorIndex );
      ExtTextOutW( hdc, pPt->x, pPt->y, (DrawType == NORMALTEXT_DRAWTYPE ) ? ETO_CLIPPED : (ETO_CLIPPED | ETO_OPAQUE), &rc, pszString, iLen, NULL);
      // apply strike through
      if ( DrawType == CHANGED_DRAWTYPE )
      {
        int iXPos = rc.top + ((rc.bottom - rc.top) / 2) + 1;
        MoveToEx( hdc, rc.left, iXPos, (LPPOINT) NULL); 
        LineTo( hdc, rc.right, iXPos );
      } /* endif */
      pPt->x += size.cx;
      iLen = 0;
    } /* endif */
  } while ( iLen != 0 );
}

//
//  FUNCTION:   DrawItemColumn(HDC, LPTSTR, LPRECT)
//
//  PURPOSE:    Draws the text for one of the columns in the list view.
//
//  PARAMETERS:
//      hdc     - Handle of the DC to draw the text into.
//      lpsz    - String to draw.
//      prcClip - Rectangle to clip the string to.
//

void DrawItemColumn( PFOLFINDDATA pIda, HDC hdc, int iColumn, PSZ_W pszString, LPRECT prcClip, PFOLFINDRESULTENTRY pEntry, PFOLFINDRESULTPOS pFoundPos, int iFoundEntries, BOOL fTarget, int iBaseColorIndex )
{
  int iCurPos = 0; // current position within text
  int iLineHeight = 0;
  int iOffsDelta = 0;              // delta between old offset in result entry and new offset because of changes in the text

  // get text line height
  {
    TEXTMETRIC tm;
    GetTextMetrics( hdc, &tm );
    iLineHeight = tm.tmHeight + 4;
  }
  // get text length
  int iRemaining = wcslen( pszString );

  SetTextColor( hdc, pIda->ColorData.ColorSetting[iBaseColorIndex].cForeground );
  SetBkColor( hdc, pIda->ColorData.ColorSetting[iBaseColorIndex].cBackground );

  // erase rectangle
  {
    HBRUSH hBrush = CreateSolidBrush( pIda->ColorData.ColorSetting[iBaseColorIndex].cBackground );
    FillRect( hdc, prcClip, hBrush ); 
    DeleteObject( hBrush );
  }

  // draw grid lines
  GFR_DrawFrame( hdc, prcClip );

  // start output at upper left corner and update reference point
  POINT pt;
  pt.x = prcClip->left + 2;
  pt.y = prcClip->top + 1; 


  // draw any segment data before the segment containing the found string
  if ( (iColumn >= 2) && (pIda->pLastUsed->sShowBeforeAfter == 2) )
  {
    PSZ_W pszText = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[ fTarget ? PREV_SEG_TARGET_TEXT1 : PREV_SEG_SOURCE_TEXT1 ] );
    GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pszText, wcslen(pszText), iLineHeight, iBaseColorIndex, ADDSEGMENTTEXT_DRAWTYPE );
  } /* endif */

  if ( (iColumn >= 2)  && (pIda->pLastUsed->sShowBeforeAfter >= 1) )
  {
    PSZ_W pszText = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[ fTarget ? PREV_SEG_TARGET_TEXT0 : PREV_SEG_SOURCE_TEXT0 ] );
    GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pszText, wcslen(pszText), iLineHeight, iBaseColorIndex, ADDSEGMENTTEXT_DRAWTYPE );
  } /* endif */

  // find next next active entry in found pos array 
  if ( pFoundPos != NULL )
  {
    while ( (iFoundEntries != 0) && (pFoundPos->fTarget != fTarget) )
    {
      pFoundPos++; iFoundEntries--;
    }
    if ( iFoundEntries == NULL ) pFoundPos = NULL;
  }
  else
  {
    // draw single string centered within the provided rectangle
    SIZE size;

    GetTextExtentPoint32W( hdc, pszString, iRemaining, &size );
    int iPos = (prcClip->right - prcClip->left - size.cx - 4) / 2;
    ExtTextOutW( hdc, pt.x + iPos, pt.y, ETO_CLIPPED, prcClip, pszString, iRemaining, NULL);
    return;
  } /* endif */
  
  while ( iRemaining )
  {
    // outut of current text
    if ( pFoundPos == NULL )
    {
      // output of remaining text
      GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pszString + iCurPos, iRemaining, iLineHeight, iBaseColorIndex, NORMALTEXT_DRAWTYPE );
      iRemaining = 0;
    }
    else
    {
      // output text up to found position

      // Attention:
      // for Type == AUTOCHANGED_TYPE entries the usOffs is the offset in the changed string and not in the not-changed string
      int iCurOffs = (int)pFoundPos->usOffs - iOffsDelta;

      int iOutputLen = iCurOffs - iCurPos;
      if ( iOutputLen != 0 )
      {
        GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pszString + iCurPos, iOutputLen, iLineHeight, iBaseColorIndex, NORMALTEXT_DRAWTYPE );
        iRemaining -= iOutputLen;
        iCurPos += iOutputLen;
      }

      // output of found string
      switch ( pFoundPos->Type )
      {
        case CHANGETO_TYPE:
          {
            PSZ_W pszChangeTo = (PSZ_W)((PBYTE)(pEntry->pszChangeToBuffer) + pFoundPos->usChangeOffs);

            GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pszString + iCurPos, (int)pFoundPos->usLen, iLineHeight, iBaseColorIndex, TOBECHANGED_DRAWTYPE );
            if ( *pszChangeTo != 0 ) 
            {
              // apply any preserve case option
              wcscpy( pIda->szChangeToModified, pszChangeTo );
              pIda->szChangeToModified[0] = GFR_AdjustCase( pszString[iCurPos], *pszChangeTo, pIda->fPreserveCase );

              GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pIda->szChangeToModified, wcslen(pIda->szChangeToModified), iLineHeight, iBaseColorIndex, CHANGETO_DRAWTYPE );
            } /* endif */
          }
          break;

        case CHANGED_TYPE:
          {
            PSZ_W pszChangeTo = (PSZ_W)((PBYTE)(pEntry->pszChangeToBuffer) + pFoundPos->usChangeOffs);
            GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pszString + iCurPos, (int)pFoundPos->usLen, iLineHeight, iBaseColorIndex, CHANGED_DRAWTYPE );
            if ( *pszChangeTo != 0 ) 
            {
              // apply any preserve case option
              wcscpy( pIda->szChangeToModified, pszChangeTo );
              pIda->szChangeToModified[0] = GFR_AdjustCase( pszString[iCurPos], *pszChangeTo, pFoundPos->fPreserveCase );

              GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pIda->szChangeToModified, wcslen(pIda->szChangeToModified), iLineHeight, iBaseColorIndex, CHANGEDTO_DRAWTYPE );
            } /* endif */
          }
          break;

        case AUTOCHANGED_TYPE:
          {
            PSZ_W pszChangeTo = (PSZ_W)((PBYTE)(pEntry->pszChangeToBuffer) + pFoundPos->usChangeOffs);
            GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pszString + iCurPos, (int)pFoundPos->usLen, iLineHeight, iBaseColorIndex, CHANGED_DRAWTYPE );
            if ( *pszChangeTo != 0 ) GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pszChangeTo, wcslen(pszChangeTo), iLineHeight, iBaseColorIndex, CHANGEDTO_DRAWTYPE );
            iOffsDelta += (wcslen(pszChangeTo) - (int)pFoundPos->usLen);
          }
          break;

        case FOUND_TYPE:
        default:
          GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pszString + iCurPos, (int)pFoundPos->usLen, iLineHeight, iBaseColorIndex, FOUND_DRAWTYPE );
          break;
      } /* endswitch */

      iRemaining -= (int)pFoundPos->usLen;
      iCurPos += (int)pFoundPos->usLen;

      // go to next relevant foundpos entry 
      if ( pFoundPos != NULL )
      {
        pFoundPos++; iFoundEntries--;
        while ( (iFoundEntries != 0) && (pFoundPos->fTarget != fTarget) )
        {
          pFoundPos++; iFoundEntries--;
        }
        if ( iFoundEntries == NULL ) pFoundPos = NULL;
      }
    }
  }

  // draw any segment data following the segment containing the found string
  if ( (iColumn >= 2)  && (pIda->pLastUsed->sShowBeforeAfter >= 1) )
  {
    PSZ_W pszText = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[ fTarget ? NEXT_SEG_TARGET_TEXT0 : NEXT_SEG_SOURCE_TEXT0 ] );
    GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pszText, wcslen(pszText), iLineHeight, iBaseColorIndex, ADDSEGMENTTEXT_DRAWTYPE );
  } /* endif */

  if ( (iColumn >= 2)  && (pIda->pLastUsed->sShowBeforeAfter == 2) )
  {
    PSZ_W pszText = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[ fTarget ? NEXT_SEG_TARGET_TEXT1 : NEXT_SEG_SOURCE_TEXT1 ] );
    GFR_DrawMultiLineText( pIda, hdc, prcClip, &pt, pszText, wcslen(pszText), iLineHeight, iBaseColorIndex, ADDSEGMENTTEXT_DRAWTYPE );
  } /* endif */


}

void GFR_UpdateSelectionStatus
(
  PFOLFINDDATA      pIda
)
{
  int iItems = QUERYITEMCOUNTHWND( pIda->hResultListBox );
  int iSelected = QUERYSELECTIONHWND( pIda->hResultListBox );
  char szString[40];
  if ( iSelected != -1 )
  {
    sprintf( szString, "item %ld of %ld selected", iSelected + 1, iItems );
  }
  else
  {
    sprintf( szString, "no item of %ld selected", iItems );
  } /* endif */
  SendMessage( pIda->hStatus, SB_SETTEXT, 0, (LPARAM)szString);
}

// build complete text string (incl. delete and changed text) for given column in column text buffer of pIda
int GFR_BuildFullColumnText( PFOLFINDDATA pIda, PFOLFINDRESULTENTRY pEntry, BOOL fTarget )
{
  int iFillPos = 0;
  PSZ_W pszText = NULL;
  int iFoundEntries = pEntry->iUsedEntries;
  int iCurPos = 0; // current position within text
  int iOffsDelta = 0; // offset delta between position in original string and string after applying the change

  if ( pIda->pLastUsed->sShowBeforeAfter == 2 )
  {
    pszText = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[ fTarget ? PREV_SEG_TARGET_TEXT1 : PREV_SEG_SOURCE_TEXT1 ] );
    wcscpy( pIda->szColTextBuffer + iFillPos, pszText );
    iFillPos += wcslen( pszText );
  } /* endif */

  if ( pIda->pLastUsed->sShowBeforeAfter >= 1 )
  {
    pszText = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[ fTarget ? PREV_SEG_TARGET_TEXT0 : PREV_SEG_SOURCE_TEXT0 ] );
    wcscpy( pIda->szColTextBuffer + iFillPos, pszText );
    iFillPos += wcslen( pszText );
  } /* endif */

  PFOLFINDRESULTPOS pFoundPos = (PFOLFINDRESULTPOS)((PBYTE)pEntry + pEntry->iResultPosOffs );
  pszText = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[ fTarget ? TARGET_TEXT : SOURCE_TEXT ] );
  int iRemaining = wcslen( pszText );

  // find next next active entry in found pos array 
  if ( pFoundPos != NULL )
  {
    while ( (iFoundEntries != 0) && (pFoundPos->fTarget != fTarget) ) 
    {
      pFoundPos++; 
      iFoundEntries--;
    }
    if ( iFoundEntries == NULL ) pFoundPos = NULL;
  } /* endif */
  
  while ( iRemaining )
  {
    // outut of current text
    if ( pFoundPos == NULL )
    {
      // output of remaining text
      wcsncpy( pIda->szColTextBuffer + iFillPos, pszText + iCurPos, iRemaining );
      iFillPos += iRemaining;
      iRemaining = 0;
    }
    else
    {
      // output text up to found position

      // Attention:
      // for Type == AUTOCHANGED_TYPE entries the usOffs is the offset in the changed string and not in the original string
      int iCurOffs = (int)pFoundPos->usOffs - iOffsDelta;

      int iOutputLen = iCurOffs - iCurPos;
      if ( iOutputLen != 0 )
      {
        wcsncpy( pIda->szColTextBuffer + iFillPos, pszText + iCurPos, iOutputLen );
        iFillPos += iOutputLen;
        iRemaining -= iOutputLen;
        iCurPos += iOutputLen;
      }

      // output of found string
      wcsncpy( pIda->szColTextBuffer + iFillPos, pszText + iCurPos, (int)pFoundPos->usLen );
      iFillPos += (int)pFoundPos->usLen;
      if ( (pFoundPos->Type == CHANGETO_TYPE) || (pFoundPos->Type == CHANGED_TYPE)|| (pFoundPos->Type == AUTOCHANGED_TYPE) )
      {
        PSZ_W pszChangeTo = (PSZ_W)((PBYTE)(pEntry->pszChangeToBuffer) + pFoundPos->usChangeOffs);
        if ( *pszChangeTo != 0 ) 
        {
          int iChangeToLen = wcslen(pszChangeTo);
          if ( pFoundPos->Type == AUTOCHANGED_TYPE ) iOffsDelta += (iChangeToLen - (int)pFoundPos->usLen);
          wcsncpy( pIda->szColTextBuffer + iFillPos, pszChangeTo, iChangeToLen );
          iFillPos += iChangeToLen;
        }
      } /* endif */
      iRemaining -= (int)pFoundPos->usLen;
      iCurPos += (int)pFoundPos->usLen;

      // go to next relevant foundpos entry 
      if ( pFoundPos != NULL )
      {
        pFoundPos++; iFoundEntries--;
        while ( (iFoundEntries != 0) && (pFoundPos->fTarget != fTarget) )
        {
          pFoundPos++; 
          iFoundEntries--;
        }
        if ( iFoundEntries == NULL ) pFoundPos = NULL;
      }
    }
  }

  if ( pIda->pLastUsed->sShowBeforeAfter >= 1 )
  {
    pszText = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[ fTarget ? NEXT_SEG_TARGET_TEXT0 : NEXT_SEG_SOURCE_TEXT0 ] );
    wcscpy( pIda->szColTextBuffer + iFillPos, pszText );
    iFillPos += wcslen( pszText );
  } /* endif */

  if ( pIda->pLastUsed->sShowBeforeAfter == 2 )
  {
    pszText = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[ fTarget ? NEXT_SEG_TARGET_TEXT1 : NEXT_SEG_SOURCE_TEXT1 ] );
    wcscpy( pIda->szColTextBuffer + iFillPos, pszText );
    iFillPos += wcslen( pszText );
  } /* endif */

  pIda->szColTextBuffer[iFillPos] = 0;

  return( iFillPos );
}

// (re)fill table containing line break offsets within given text and provided rectangle
int GFR_GetTextHeight( PFOLFINDDATA pIda, HWND hwndControl, HDC hdc, PFOLFINDRESULTENTRY pEntry, BOOL fTarget )
{
  TEXTMETRIC tm;
  GetTextMetrics( hdc, &tm );
  int iLineHeight = tm.tmHeight + 2;

  // get complete text to be displayed
  int iLength = GFR_BuildFullColumnText( pIda, pEntry, fTarget );

  // replace any LF with blank if we are displaying flow text
  if ( !pIda->fRespectLineFeeds )
  {
    for ( int i = 0; i < iLength; i++ )
    {
      if ( pIda->szColTextBuffer[i] == L'\n' ) pIda->szColTextBuffer[i] = L' ';
    }
  } /* endif */

  // get height of text using the DrawText function
  PSZ_W pszText = pIda->szColTextBuffer;
  RECT rc;
  rc.top = 0;
  rc.left = 0;
  rc.right = pIda->aiListViewColWidth[2];
  rc.bottom = iLineHeight;
  int iRes = DrawTextW( hdc, pszText, wcslen(pszText), &rc, DT_WORDBREAK | DT_CALCRECT );
  return( rc.bottom + 4 );
}

int GFR_MeasureItem( PFOLFINDDATA pIda, LPMEASUREITEMSTRUCT pMeasureItem )
{
  HDC hdc = GetDC(NULL);
  TEXTMETRIC tm;
  GetTextMetrics( hdc, &tm );
  PFOLFINDRESULTENTRY pEntry = (PFOLFINDRESULTENTRY)pMeasureItem->itemData;
  if ( pEntry == NULL ) return( 0 );
  int iMaxHeight = 0;
  if ( pIda->CurDisplayMode == SOURCE_AND_TARGET_COLUMN_MODE )
  {
    int iSourceHeight = GFR_GetTextHeight( pIda, pIda->hResultListBox, hdc, pEntry, FALSE );
    int iTargetHeight = GFR_GetTextHeight( pIda, pIda->hResultListBox, hdc, pEntry, TRUE );
    iMaxHeight = max( iSourceHeight, iTargetHeight );
  }
  else if ( pIda->CurDisplayMode == ONLY_SOURCE_COLUMN_MODE )
  {
    iMaxHeight = GFR_GetTextHeight( pIda, pIda->hResultListBox, hdc, pEntry, FALSE );
  }
  else
  {
    iMaxHeight = GFR_GetTextHeight( pIda, pIda->hResultListBox, hdc, pEntry, TRUE );
  } /* endif */
  ReleaseDC( NULL, hdc );
  pMeasureItem->itemHeight = iMaxHeight;

  return( 0 );
}

// force a refresh of the height of the listbox items
int GFR_ForceRefreshOfItemHeights( PFOLFINDDATA pIda )
{
  // GQ: The following code did not send WM_MEASUREITEM messages for the listbox items
	//RECT rc;
 // GetWindowRect( pIda->hResultListBox, &rc );
	//WINDOWPOS wp;
	//wp.hwnd = pIda->hResultListBox;
 // wp.cx = rc.right - rc.left;
 // wp.cy = rc.bottom - rc.top;
	//wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
	//SendMessage( pIda->hResultListBox, WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp );

  // GQ: Also this code did not work, it did not send WM_MEASUREITEM messages for the listbox items
  int iItemCount = QUERYITEMCOUNTHWND( pIda->hResultListBox );
  for ( int i = 0; i < iItemCount; i++ )
  {
    PFOLFINDRESULTENTRY pEntry = (PFOLFINDRESULTENTRY)QUERYITEMHANDLEHWND( pIda->hResultListBox, i );
    SETITEMHANDLEHWND( pIda->hResultListBox, i, pEntry );
  }

  // now we are using a brute force approach: delete all items, refill the listbox, retore current top item and current selected item and allow redraw...
  SendMessage( pIda->hResultListBox, WM_SETREDRAW, FALSE, 0 );
  int iSelItem = SendMessage( pIda->hResultListBox, LB_GETCURSEL, 0, 0 );
  int iTopIndex = SendMessage( pIda->hResultListBox, LB_GETTOPINDEX, 0, 0 );
  SendMessage( pIda->hResultListBox, LB_RESETCONTENT, 0, 0 );
  for( int i = 0; i < pIda->iResultsDisplayed; i++ )
  {
    PFOLFINDRESULTENTRY pCurEntry = pIda->ppResultList[i];
    INSERTITEMENDHWND( pIda->hResultListBox, pCurEntry );
  } /* endfor */
  if ( iTopIndex != -1 ) SendMessage( pIda->hResultListBox, LB_SETTOPINDEX, iTopIndex, 0 );
  if ( iSelItem != -1 ) SendMessage( pIda->hResultListBox, LB_SETCURSEL, iSelItem, 0 );
  SendMessage( pIda->hResultListBox, WM_SETREDRAW, TRUE, 0 );
  InvalidateRect( pIda->hResultListBox, NULL, TRUE );
  return( 0 );
} 

// draw a frame
int  GFR_DrawFrame( HDC hdc, LPRECT prc )
{
  MoveToEx( hdc, prc->left, prc->top, NULL );
  LineTo( hdc, prc->right, prc->top );
  LineTo( hdc, prc->right, prc->bottom );
  LineTo( hdc, prc->left, prc->bottom );
  LineTo( hdc, prc->left, prc->top );
  return( 0 );
}

