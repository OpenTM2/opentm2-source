/*! \file
	Description: Functions replacing OS/2 functions in Windows environment

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include <time.h>
#include <locale.h>
#include "eqf.h"

#include <tchar.h>

// temporary defines to compile without DBCS LIB
USHORT dbcscp = 0;

/**********************************************************************/
/* substitute WinQueryWindow call                                     */
/**********************************************************************/
HWND APIENTRY
WinQueryWindow
(
  HWND  hwnd,                          // Window handle
  SHORT sCmd,                          // window looked for
  BOOL  fLock                          // window to be locked
)
{
  HWND hwndResult;                     // resulting window handle

  fLock;                               // avoid compiler warnings


  switch ( sCmd )
  {
    case QW_PARENT:
      /****************************************************************/
      /* get parent handle - set window handle to NULL in exceptional */
      /* cases..                                                      */
      /****************************************************************/
      if ( hwnd )
      {
        hwndResult = GetParent( hwnd );
        if (!hwndResult)
        {
          hwndResult = hwnd;
        } /* endif */
      }
      else
      {
        hwndResult = hwnd;
      } /* endif */
      break;
    case GW_OWNER:
    case GW_HWNDFIRST:
    case GW_HWNDLAST:
    case GW_HWNDNEXT:
    case GW_HWNDPREV:
      hwndResult = GetWindow( hwnd, sCmd );
      break;
    default:
      hwndResult = (HWND) NULL;
  } /* endswitch */

  return hwndResult;
}

/**********************************************************************/
/* WinQueryWindowRect - determine the window positions for the        */
/* specified window...                                                */
/**********************************************************************/
BOOL    APIENTRY WinQueryWindowRect
(
  HWND hwnd,
  PRECTL prclDest
)
{
  RECT  rectDest;
  RECT  rectScreen;

//GetWindowRect( hwnd, &rectDest );
  GetClientRect( hwnd, &rectDest );
  GetWindowRect( GetDesktopWindow(), &rectScreen );

  /********************************************************************/
  /* adjust screen rectangle, if window overlaps at bottom...         */
  /********************************************************************/
  rectScreen.bottom = max(rectDest.bottom, rectScreen.bottom);

  /********************************************************************/
  /* adjust window positions to be in OS/2 PM coordinates..           */
  /********************************************************************/
  PRECTL_XLEFT(prclDest) = rectDest.left;
  PRECTL_XRIGHT(prclDest)  = rectDest.right;

  PRECTL_YBOTTOM(prclDest) = rectScreen.bottom - rectDest.bottom - rectDest.top ;
  PRECTL_YTOP(prclDest)    = rectScreen.bottom - rectDest.top;
  return( TRUE );
}

/**********************************************************************/
/* WinQueryWindowPos - determine window position and style flags      */
/* for the specified window                                           */
/**********************************************************************/
BOOL APIENTRY
WinQueryWindowPos
(
  HWND  hwnd,                          // Window handle
  PSWP  pSwp                           // size and position structure
)
{
  BOOL fOK;                            // success indicator
  WINDOWPLACEMENT  wp;                 // window placement structure
  HWND hwndPar = GetParent( hwnd );    // handle of parent
  RECT rectDest;                       // rectangle

  /********************************************************************/
  /* get rectangel of parent - use desktop if no parent handle set    */
  /********************************************************************/
  if (! hwndPar )
  {
    hwndPar = GetDesktopWindow();
  } /* endif */
//  GetClientRect( hwndPar, &rectDest );
  GetWindowRect( hwndPar, &rectDest );

  wp.length = sizeof( wp );
  fOK = GetWindowPlacement( hwnd, &wp );


  /********************************************************************/
  /* return the values ....                                           */
  /********************************************************************/
  if ( fOK )
  {
    switch ( wp.showCmd )
    {
      case SW_MINIMIZE :
      case SW_SHOWMINIMIZED :
        pSwp->fs      = EQF_SWP_MINIMIZE;
        break;
      case SW_MAXIMIZE :
        pSwp->fs      = EQF_SWP_MAXIMIZE;
        wp.rcNormalPosition = rectDest;
        break;
      default :
        pSwp->fs = 0;
        break;
    } /* endswitch */
    pSwp->fs      |= EQF_SWP_SHOW;
    pSwp->fs      |= EQF_SWP_SIZE;

    pSwp->cy      = (SHORT)(wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
    pSwp->cx      = (SHORT)(wp.rcNormalPosition.right - wp.rcNormalPosition.left);
    pSwp->y       = (SHORT)(rectDest.bottom - rectDest.top - wp.rcNormalPosition.bottom);
    if (pSwp->y < 0 ) pSwp->y = 0;
    pSwp->x       = (SHORT)(wp.rcNormalPosition.left);
    pSwp->hwnd    = hwnd;
    pSwp->hwndInsertBehind = (HWND) NULL;   // not set yet
  } /* endif */

  return fOK;
}

/**********************************************************************/
/* WinSetWindowPos - set window positions depending on PM style of    */
/*  operation (bottom to top)                                         */
/* Tobe done: check on flags    ....                                  */
/**********************************************************************/
BOOL APIENTRY
WinSetWindowPos
(
  HWND hwnd,                           // window handle
  HWND hwndAfter,                      // insert behind
  LONG x ,
  LONG y,
  LONG cx,
  LONG cy,
  USHORT fsFlags                       // PM display flags
)
{
  BOOL  fOK = TRUE;                    // success indicator
  HWND hwndPar = GetParent( hwnd );    // handle of parent
  RECT  rectDest;                      // rectangle
  USHORT fsWinFlags = 0;               // window flags
  LONG   cxFullScreen, cyFullScreen;    // fullscreen window size...

  if (! hwndPar )
  {
    hwndPar = GetDesktopWindow();
  } /* endif */

  /********************************************************************/
  /* if no SIZING requested, we have to get the correct cy value      */
  /********************************************************************/
  if ( !( fsFlags & EQF_SWP_SIZE) )
  {
    GetWindowRect( hwnd, &rectDest );
    cy = (rectDest.bottom - rectDest.top);
  } /* endif */
  /********************************************************************/
  /* get rectangle positions of parent                                */
  /********************************************************************/
  GetWindowRect( hwndPar, &rectDest );
  rectDest.bottom = rectDest.bottom - y - cy - rectDest.top;

  /********************************************************************/
  /* change the flags to fit windows behaviour                        */
  /********************************************************************/
  fsWinFlags |= (fsFlags & EQF_SWP_SIZE) ? 0 : SWP_NOSIZE;
  fsWinFlags |= (fsFlags & EQF_SWP_MOVE) ? 0 : SWP_NOMOVE;
  fsWinFlags |= (fsFlags & EQF_SWP_ZORDER) ? 0 : SWP_NOZORDER;
  fsWinFlags |= (fsFlags & EQF_SWP_SHOW) ? SWP_SHOWWINDOW : 0;
  fsWinFlags |= (fsFlags & EQF_SWP_HIDE) ? SWP_HIDEWINDOW : 0;
  fsWinFlags |= (fsFlags & EQF_SWP_NOREDRAW) ? SWP_NOREDRAW : 0;
//  fsWinFlags |= (fsFlags & EQF_SWP_NOADJUST) ?
  fsWinFlags |= (fsFlags & EQF_SWP_ACTIVATE) ? 0 : SWP_NOACTIVATE;
//  fsWinFlags |= (fsFlags & EQF_SWP_DEACTIVATE) ?
//  fsWinFlags |= (fsFlags & EQF_SWP_EXTSTATECHANGE) ?
//  fsWinFlags |= (fsFlags & EQF_SWP_FOCUSACTIVATE) ?
//  fsWinFlags |= (fsFlags & EQF_SWP_FOCUSDEACTIVATE) ?

  /********************************************************************/
  /* consistency check ...                                            */
  /********************************************************************/
  cxFullScreen = GetSystemMetrics( SM_CXSCREEN );   // full
  cyFullScreen = GetSystemMetrics( SM_CYSCREEN );  // full

  x = ( x < 0 ) ? 0 : x;
  x = ( x > cxFullScreen) ?  cxFullScreen : x;
  cx = ( cx < 0 ) ? 0 : cx;
  cx = ( (x+cx) > cxFullScreen) ? (cxFullScreen - x) : cx;

  y = rectDest.bottom;
  y = ( y < 0 ) ? 0 : y;
  y = ( y > cyFullScreen) ?  cyFullScreen : y;
  cy = ( cy < 0 ) ? 0 : cy;
  cy = ( (y+cy) > cyFullScreen) ? (cyFullScreen - y) : cy;



  /********************************************************************/
  /* check for minimize or maximize                                   */
  /********************************************************************/
  if ( (fsFlags & EQF_SWP_MINIMIZE) || (fsFlags & EQF_SWP_MAXIMIZE) ||
       (fsFlags & EQF_SWP_RESTORE) )
  {
    /******************************************************************/
    /* only set internal positions in such a special case             */
    /* disable SW_SHOWWINDOW flag, i.e. hide this window for a moment */
    /******************************************************************/
    fsWinFlags &= ~SWP_SHOWWINDOW;
    fsWinFlags |= SWP_HIDEWINDOW;
    fOK = SetWindowPos( hwnd, hwndAfter, x, y, cx, cy, fsWinFlags );

    if (fsFlags & EQF_SWP_MINIMIZE)
    {
      fsWinFlags = SW_SHOWMINIMIZED;
    }
    else if (fsFlags & EQF_SWP_MAXIMIZE)
    {
      fsWinFlags = SW_SHOWMAXIMIZED;
    }
    else      // EQF_SWP_RESTORE
    {
      fsWinFlags = SW_RESTORE;
    } /* endif */
    ShowWindow (hwnd,  fsWinFlags);
  }
  else
  {
    /******************************************************************/
    /* position window in a normal manour.....                        */
    /******************************************************************/
    fOK = SetWindowPos( hwnd, hwndAfter, x, y, cx, cy, fsWinFlags );
  } /* endif */
  return fOK;
}

//------------------------------------------------------------------------------
// Function name:     WinDrawBitmap
//------------------------------------------------------------------------------
BOOL  APIENTRY WinDrawBitmap
(
  HDC     hdc,
  HBITMAP hbm,
  PRECTL  pwrcSrc,
  PPOINTL pptlDst,
  LONG    clrFore,
  LONG    clrBack,
  USHORT  fs
)
{
  BOOL    fOK = TRUE;                  // success indicator
  BITMAP  bm;
  HBITMAP hbmDefault, hbmMono;
  HDC     hdcMem, hdcMono;
  HBRUSH  hbr, hbrOld;
  POINT   ptSize, ptOrg;
  DWORD   rgbShadow;
  DWORD   crBack, crText;
  COLORREF clrForeSave, clrBackSave;   // foreground and background colors
  DWORD   dwFlags;
  PRECTL  prectDest;                   // destination rectangle for DBM_STRETCH

  pwrcSrc;
  memset(&clrForeSave, 0, sizeof(clrForeSave));
  memset(&clrBackSave, 0, sizeof(clrBackSave));
  hdcMem = CreateCompatibleDC( hdc );
  SelectObject( hdcMem, hbm );
  SetMapMode( hdcMem, GetMapMode( hdc ) );

  /********************************************************************/
  /* get the bitmap dimensions                                        */
  /********************************************************************/
  GetObject( hbm, sizeof( BITMAP ), (LPSTR) &bm );
  ptSize.x = bm.bmWidth;
  ptSize.y = bm.bmHeight;
  DPtoLP( hdc, &ptSize, 1 );

  ptOrg.x = 0;
  ptOrg.y = 0;
  DPtoLP( hdcMem, &ptOrg, 1 );

  /********************************************************************/
  /* translate drawing flags                                          */
  /********************************************************************/
  dwFlags = SRCCOPY;
//  if ( fs & DBM_INVERT )
//  {
//    dwFlags |= DSTINVERT;
//  } /* endif */


  /********************************************************************/
  /* save any colors for resetting them later ...                     */
  /********************************************************************/
  if (!( fs & DBM_IMAGEATTRS) )
  {
    clrForeSave = GetTextColor( hdc );
    clrBackSave = GetBkColor( hdc );
    /******************************************************************/
    /* set  colors                                                    */
    /******************************************************************/
    SetTextColor( hdc, clrFore );
    SetBkColor( hdc, clrBack );
  } /* endif */

  /********************************************************************/
  /* according to the fs value either use BitBlt or StretchBlt        */
  /********************************************************************/
  if ( fs & DBM_STRETCH )
  {
    prectDest = (PRECTL)pptlDst;

    fOK = StretchBlt( hdc,
                      (SHORT) PRECTL_XLEFT(prectDest),
                      (SHORT) PRECTL_YTOP(prectDest),
                      (SHORT) (PRECTL_XRIGHT(prectDest) - PRECTL_XLEFT(prectDest)),
                      (SHORT) (PRECTL_YBOTTOM(prectDest) - PRECTL_YTOP(prectDest)),
                      hdcMem,
                      ptOrg.x,
                      ptOrg.y,
                      ptSize.x,
                      ptSize.y,
                      dwFlags );
  }
  else
  {
    fOK = BitBlt( hdc, (SHORT) pptlDst->x, (SHORT) pptlDst->y,
                  ptSize.x, ptSize.y, hdcMem, ptOrg.x, ptOrg.y, dwFlags );

    if ( fs & DBM_INVERT )
    {
      PatBlt( hdc, (SHORT) pptlDst->x+1, (SHORT) pptlDst->y+1,
              ptSize.x-2, ptSize.y-2, DSTINVERT);
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* restore any changed colors                                       */
  /********************************************************************/
  if (!(fs & DBM_IMAGEATTRS))
  {
    SetTextColor( hdc, clrForeSave );
    SetBkColor( hdc, clrBackSave );
  } /* endif */


  /********************************************************************/
  /* if half tone required ....                                       */
  /********************************************************************/
  if ( fs & DBM_HALFTONE )
  {
    hdcMono = CreateCompatibleDC( hdc );
    if (hdcMono)
    {
      hbmMono = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
      hbmDefault = (HBITMAP) SelectObject(hdcMono, hbmMono);

      rgbShadow  = GetSysColor(COLOR_BTNSHADOW);
      hbr = CreateSolidBrush(rgbShadow);

      if (hbr)
      {
        hbrOld = (HBRUSH) SelectObject(hdc, hbr);
        if (hbrOld)
        {
          // initalize whole area with 1's
          PatBlt(hdcMono, 0, 0, ptSize.x, ptSize.y, WHITENESS);

          // create mask based on color bitmap
          // convert this to 1's
          SetBkColor(hdcMem, GetSysColor(COLOR_BTNFACE));
          BitBlt(hdcMono, 0, 0, ptSize.x, ptSize.y,
                 hdcMem, 0, 0, SRCCOPY);
          // convert this to 1's
          SetBkColor(hdcMem, GetSysColor(COLOR_BTNHIGHLIGHT));
          // OR in the new 1's
          BitBlt(hdcMono, 0, 0,  ptSize.x, ptSize.y,
                 hdcMem, 0, 0, SRCPAINT);

          SelectObject(hdc, hbrOld);

          crText = SetTextColor(hdc, 0L);       // 0's in mono -> 0 (for ROP)
          crBack = SetBkColor(hdc, 0x00FFFFFF); // 1's in mono -> 1
          // draw the shadow color where we have 0's in the mask
          // let the black border remain
          BitBlt(hdc, (SHORT)pptlDst->x+1, (SHORT)pptlDst->y+1,
                 ptSize.x-2, ptSize.y-2, hdcMono, 1, 1, 0x00B8074A);
          SetTextColor( hdc, crText );
          SetBkColor( hdc, crBack );
        }
        DeleteObject(hbr);
      } /* endif */

      // free resources...
      if (hbmDefault)
        SelectObject(hdcMono, hbmDefault);
      DeleteDC(hdcMono);
    } /* endif */
  } /* endif */


  DeleteDC( hdcMem );

  return ( fOK );
} /* end of function DrawBitmap */

/********************************************************************/
/* determine Color                                                  */
/********************************************************************/
COLORREF UtlGetColorref( LONG lColor )
{
  COLORREF rgbColor;

  /********************************************************************/
  /* map the OS/2 system colors to Windows RGB colors...              */
  /********************************************************************/
  if ( lColor <= -10L )
  {
    switch ( lColor )
    {
      case SYSCLR_SCROLLBAR:
        rgbColor = GetSysColor(COLOR_SCROLLBAR);
        break;
      case SYSCLR_BACKGROUND:
        rgbColor = GetSysColor(COLOR_BACKGROUND);
        break;
      case SYSCLR_ACTIVETITLE:
        rgbColor = GetSysColor(COLOR_ACTIVECAPTION);
        break;
      case SYSCLR_INACTIVETITLE:
        rgbColor = GetSysColor(COLOR_INACTIVECAPTION);
        break;
      case SYSCLR_MENU:
        rgbColor = GetSysColor(COLOR_MENU);
        break;
      case SYSCLR_WINDOW:
        rgbColor = GetSysColor(COLOR_WINDOW);
        break;
      case SYSCLR_WINDOWFRAME:
        rgbColor = GetSysColor(COLOR_WINDOWFRAME);
        break;
      case SYSCLR_MENUTEXT:
        rgbColor = GetSysColor(COLOR_MENUTEXT);
        break;
      case SYSCLR_WINDOWTEXT:
        rgbColor = GetSysColor(COLOR_WINDOWTEXT);
        break;
      case SYSCLR_TITLETEXT:
        rgbColor = GetSysColor(COLOR_CAPTIONTEXT);
        break;
      case SYSCLR_ACTIVEBORDER:
        rgbColor = GetSysColor(COLOR_ACTIVEBORDER);
        break;
      case SYSCLR_INACTIVEBORDER:
        rgbColor = GetSysColor(COLOR_INACTIVEBORDER);
        break;
      case SYSCLR_APPWORKSPACE:
        rgbColor = GetSysColor(COLOR_APPWORKSPACE);
        break;
      case SYSCLR_HILITEFOREGROUND:
        rgbColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
        break;
      case SYSCLR_HILITEBACKGROUND:
        rgbColor = GetSysColor(COLOR_HIGHLIGHT);
        break;
      default:
        rgbColor = 0L;
        break;
    } /* endswitch */
  }
  else
  {
    rgbColor = lColor;
  } /* endif */

  return rgbColor;
}

/*
 * DriveType
 *
 * Purpose:
 *  Augments the Windows API GetDriveType with a call to the CD-ROM
 *  extensions to determine if a drive is a floppy, hard disk, CD-ROM,
 *  RAM-drive, or networked  drive.
 *
 * Note: This will work perfectly under Windows NT
 *
 * Parameters:
 *  iDrive          UINT containing the zero-based drive index
 *
 * Return Value:
 *  UINT            One of the following values describing the drive:
 *                  DRIVE_FLOPPY, DRIVE_HARD, DRIVE_CDROM, DRIVE_RAM,
 *                  DRIVE_NETWORK.
 */

USHORT EqfDriveType(USHORT iDrive)
{
    USHORT   iType;

    //Validate possible drive indices
    if (0 > iDrive  || 25 < iDrive)
        return 0xFFFF;

        {
      CHAR szRoot[4] = "A:\\";
      szRoot[0] = (CHAR)('A' + iDrive);
      iType=(USHORT)GetDriveType(szRoot);
        }
    /*
     * Under Windows NT, GetDriveType returns complete information
     * not provided under Windows 3.x which we now get through other
     * means.
     */
    return iType;
}


SHORT LBSetItemTextW( HWND hwndLB, SHORT index, PSZ_W pText)
{

  LONG             lItemData;          // buffer for item data
    BOOL             fSelected = FALSE;  // item-is-selected flag
    SHORT            sCurSelection;      // index of currently selected item
    SHORT            sSelItems;          // number of selected items in listbox
    BOOL             fMultipleSel = TRUE;       // TRUE for multiple selection listbox

    /********************************************************************/
    /* Get item data                                                    */
    /********************************************************************/
    lItemData = SendMessage( hwndLB, LB_GETITEMDATA, index, 0L );

    /********************************************************************/
    /* Check if our item is selected                                    */
    /********************************************************************/
    sSelItems = (SHORT)SendMessage( hwndLB, LB_GETSELCOUNT, 0, 0L );
    if ( sSelItems == LB_ERR )
    {
      /******************************************************************/
      /* This seems to be a single selection listbox                    */
      /******************************************************************/
      fMultipleSel = FALSE;
      sCurSelection = (SHORT)SendMessage( hwndLB, LB_GETCURSEL, 0, 0L );
      fSelected = (sCurSelection == index );
    }
    else if ( sSelItems != 0 )
    {
      int *pIndexArray;                  // array with index of selected items

      /******************************************************************/
      /* This seems to be a multiple selection listbox                  */
      /******************************************************************/
      fMultipleSel = TRUE;

      /******************************************************************/
      /* Allocate buffer for selected items array                       */
      /******************************************************************/
      if ( UtlAlloc( (PVOID *) &pIndexArray, 0L,
                     max( MIN_ALLOC, (LONG)(sizeof(int)*sSelItems) ),
                     NOMSG ) )
      {
        SHORT i;

        SendMessage( hwndLB, LB_GETSELITEMS, (WPARAM)sSelItems,
                     MP2FROMP(pIndexArray) );
        for ( i = 0; i < sSelItems; i++ )
        {
          if ( pIndexArray[i] == index )
          {
            fSelected = TRUE;
            break;
          } /* endif */
        } /* endfor */
        UtlAlloc( (PVOID *) &pIndexArray, 0L, 0L, NOMSG );
      } /* endif */
    } /* endif */

    /********************************************************************/
    /* inhibit redraw of listbox contents                               */
    /********************************************************************/
    SendMessage( hwndLB, WM_SETREDRAW, FALSE, 0L );

    /********************************************************************/
    /* Insert new string, set item data and remove old string           */
    /********************************************************************/
    //SendMessageW( hwndLB, LB_INSERTSTRING, (WPARAM)index, (LPARAM) pText);
    INSERTITEMHWNDW( hwndLB, index, pText);
    SendMessage( hwndLB, LB_SETITEMDATA, index, lItemData );
    SendMessage( hwndLB, LB_DELETESTRING, (WPARAM)(index+1), 0L );

    /********************************************************************/
    /* Select item if required                                          */
    /********************************************************************/
    if ( fSelected )
    {
      if ( fMultipleSel )
      {
        SendMessage( hwndLB, LB_SETSEL, (WPARAM)TRUE, MAKELPARAM( index, 0 ) );
      }
      else
      {
        SendMessage( hwndLB, LB_SETCURSEL, MP1FROMSHORT(index), 0L );
      } /* endif */
    } /* endif */

    /********************************************************************/
    /* Enable redraw of listbox and invalidate changed item             */
    /* (LB_GETITEMRECT returns LB_ERR ...)                              */
    /********************************************************************/
    SendMessage( hwndLB, WM_SETREDRAW, TRUE, 0L );

    InvalidateRect( hwndLB, NULL, FALSE );
    UpdateWindow( hwndLB );

    return( 0 );
  } /* end of function LBSetItemTextW */


/**********************************************************************/
/* Function to simulate LM_SETITEMTEXT message                        */
/**********************************************************************/
SHORT LBSetItemText( HWND hwndLB, SHORT index, PSZ pText )
{
  LONG             lItemData;          // buffer for item data
  BOOL             fSelected = FALSE;  // item-is-selected flag
  SHORT            sCurSelection;      // index of currently selected item
  SHORT            sSelItems;          // number of selected items in listbox
  BOOL             fMultipleSel = TRUE;   // TRUE for multiple selection listbox


  /********************************************************************/
  /* Get item data                                                    */
  /********************************************************************/
  lItemData = SendMessage( hwndLB, LB_GETITEMDATA, index, 0L );

  /********************************************************************/
  /* Check if our item is selected                                    */
  /********************************************************************/
  sSelItems = (SHORT)SendMessage( hwndLB, LB_GETSELCOUNT, 0, 0L );
  if ( sSelItems == LB_ERR )
  {
    /******************************************************************/
    /* This seems to be a single selection listbox                    */
    /******************************************************************/
    fMultipleSel = FALSE;
    sCurSelection = (SHORT)SendMessage( hwndLB, LB_GETCURSEL, 0, 0L );
    fSelected = (sCurSelection == index );
  }
  else if ( sSelItems != 0 )
  {
    int *pIndexArray;                  // array with index of selected items

    /******************************************************************/
    /* This seems to be a multiple selection listbox                  */
    /******************************************************************/
    fMultipleSel = TRUE;

    /******************************************************************/
    /* Allocate buffer for selected items array                       */
    /******************************************************************/
    if ( UtlAlloc( (PVOID *) &pIndexArray, 0L,
                   max( MIN_ALLOC, (LONG)(sizeof(int)*sSelItems) ),
                   NOMSG ) )
    {
      SHORT i;

      SendMessage( hwndLB, LB_GETSELITEMS, (WPARAM)sSelItems,
                   MP2FROMP(pIndexArray) );
      for ( i = 0; i < sSelItems; i++ )
      {
        if ( pIndexArray[i] == index )
        {
          fSelected = TRUE;
          break;
        } /* endif */
      } /* endfor */
      UtlAlloc( (PVOID *) &pIndexArray, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* inhibit redraw of listbox contents                               */
  /********************************************************************/
  SendMessage( hwndLB, WM_SETREDRAW, FALSE, 0L );

  /********************************************************************/
  /* Insert new string, set item data and remove old string           */
  /********************************************************************/
  SendMessage( hwndLB, LB_INSERTSTRING, (WPARAM)index, (LPARAM) pText);                                 \
  SendMessage( hwndLB, LB_SETITEMDATA, index, lItemData );
  SendMessage( hwndLB, LB_DELETESTRING, (WPARAM)(index+1), 0L );

  /********************************************************************/
  /* Select item if required                                          */
  /********************************************************************/
  if ( fSelected )
  {
    if ( fMultipleSel )
    {
      SendMessage( hwndLB, LB_SETSEL, (WPARAM)TRUE, MAKELPARAM( index, 0 ) );
    }
    else
    {
      SendMessage( hwndLB, LB_SETCURSEL, MP1FROMSHORT(index), 0L );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Enable redraw of listbox and invalidate changed item             */
  /* (LB_GETITEMRECT returns LB_ERR ...)                              */
  /********************************************************************/
  SendMessage( hwndLB, WM_SETREDRAW, TRUE, 0L );
//  SendMessage( hwndLB, LB_GETITEMRECT, MP1FROMSHORT(index),
//               MP2FROMP(&rcItem) );
  InvalidateRect( hwndLB, NULL, FALSE );
  UpdateWindow( hwndLB );

  return( 0 );
} /* end of function LBSetItemText */


/**********************************************************************/
/* function to simulate initdbcs and set the codepage                 */
/**********************************************************************/
unsigned int _dbcs_cp;    // global variable...
unsigned int _IsUnicodeSystem; // global variable

VOID InitUnicode( VOID )
{
    OSVERSIONINFO Osv ;

    Osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;

    if(!GetVersionEx(&Osv))
    {
    _IsUnicodeSystem = (unsigned int) FALSE;
    }
    else
    {
      _IsUnicodeSystem = (unsigned int) (Osv.dwPlatformId == VER_PLATFORM_WIN32_NT) ;
    }
    if (!_IsUnicodeSystem)
    {
       WinMessageBox( HWND_DESKTOP, QUERYACTIVEWINDOW(),
                                  "Your operating system is not supported any more by TranslationManager.",
                                  "Error",
                                  4713,
                                  MB_OK | MB_INFORMATION |
                        MB_DEFBUTTON1 | MB_MOVEABLE | MB_APPLMODAL );

       exit(1);
    }
}

BOOL IsUnicodeSystem()
{
  return _IsUnicodeSystem;
}


VOID initdbcs( VOID )
{
  unsigned int cp;

  cp = GetOEMCP();                     // Gets OEM Codepage
  switch(cp)
  {
//    case 874:                          // Thai  -- in TP5.5.2 under Unicode not necessary anymore
    case 932:                          // Japanese
    case 943:                          // Japanese2
    case 936:                          // Simplified Chinese
    case 949:                          // Korean
    case 950:                          // Chinese (Traditional)
    case 1351:                         //
      _dbcs_cp = DBCS_CP;
      break;
    default :
      _dbcs_cp = NON_DBCS_CP;
      break;
  } /* endswitch */

  return;
}


/**********************************************************************/
/* UserDefaultLangIsAPLang                                            */
/**********************************************************************/
BOOL UserDefaultLangIsAPLang()
{

  LANGID LangId;

  LangId = GetUserDefaultLangID();

  switch (LangId)
  {
    case 0x0404:        // Chinese
    case 0x0804:
    case 0x0c04:
    case 0x1004:
    case 0x0411:        // Japanese
    case 0x0412:        // Korean
    case 0x0812:
   // case 0x041e:        // Thai-- in TP5.5.2 under Unicode not necessary anymore
        return TRUE;
        break;
    default :
        return FALSE;
        break;
  }
}

/**********************************************************************/
/* IsAPLang                                                           */
/**********************************************************************/
BOOL IsAPLang(char *pszLang)
{
   short sI;
   char *szLangArray[5] = { "Chinese(simpl.)", "Chinese(trad.)", "Japanese", "Korean", "" };

   if (!pszLang)
      return FALSE;

   for (sI = 0; strlen(szLangArray[sI]) > 0; sI ++)
   {
      if (!strcmp(szLangArray[sI], pszLang))
         return TRUE;
   }

   return FALSE;
}

/**********************************************************************/
/*function to decide whether a character is SBCS, DBCS_1St, DBCS_2ND  */
/* FUNCTION NAME:  getctype                                           */
/**********************************************************************/
// NOT NEEDED if Non-Unicodesystem is NOT supported!! RJ: 020903
//unsigned int getctype(const char *string, unsigned int n) {
//
//   unsigned int i;
//
//   for (i = 0;
//        i < n;
//        (isdbcs1(string[i])) ? i += 2 : ++i);
//
//   if (i == n)
//      return isdbcs1(string[n]);
//   else
//      return DBCS_2ND;
//}


/**********************************************************************/
/* Character conversion tables                                        */
/*                                                                    */
/* If DBCS codepage is active OemToAnsi and AnsiToOem functions do    */
/* not converted anything - therefore to allow for German character   */
/**********************************************************************/
//const UCHAR chAnsiToOem850[256] =
//{ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
// 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
// 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
// 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
// 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
// 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
// 96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
//112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
//128,129, 44,159, 44, 95,253,252,136, 37, 83, 60, 79,141,142,143,
//144, 96, 39, 34, 34,149, 45, 95,152,153,115, 62,111,157,158, 89,
// 32,173,189,156,207,190,221,245,249,184,166,174,170,240,169,238,
//248,241,253,252,239,230,244,250,247,251,167,175,172,171,243,168,
//183,181,182,199,142,143,146,128,212,144,210,211,222,214,215,216,
//209,165,227,224,226,229,153,158,157,235,233,234,154,237,232,225,
//133,160,131,198,132,134,145,135,138,130,136,137,141,161,140,139,
//208,164,149,162,147,228,148,246,155,151,163,150,129,236,231,152 };
//
//const UCHAR chOem850ToAnsi[256] =
//{ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,164,
// 16, 17, 18, 19,182,167, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
// 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
// 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
// 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
// 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
// 96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
//112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
//199,252,233,226,228,224,229,231,234,235,232,239,238,236,196,197,
//201,230,198,244,246,242,251,249,255,214,220,248,163,216,215,131,
//225,237,243,250,241,209,170,186,191,174,172,189,188,161,171,187,
// 95, 95, 95,166,166,193,194,192,169,166,166, 43, 43,162,165, 43,
// 43, 45, 45, 43, 45, 43,227,195, 43, 43, 45, 45,166, 45, 43,164,
//240,208,202,203,200,105,205,206,207, 43, 43, 95, 95,166,204, 95,
//211,223,212,210,245,213,181,254,222,218,219,217,253,221,175,180,
//173,177, 95,190,182,167,247,184,176,168,183,185,179,178, 95, 32 };

/**********************************************************************/
/* convert input OEM data into Ansi data                              */
/**********************************************************************/
VOID Oem850ToAnsiBuff
(
  PSZ pInData,
  PSZ pOutData,
  USHORT usLen
)
{
  if ( usLen )
  {
    while ( usLen-- )
    {
      *pOutData++ =  *pInData++ ;
//    *pOutData++ =  chOem850ToAnsi[ *pInData++ ];
    } /* endwhile */
  } /* endif */
  return;
}

/**********************************************************************/
/* convert input Ansi data into Oem850 data                           */
/**********************************************************************/
VOID AnsiToOem850Buff
(
  PSZ pInData,
  PSZ pOutData,
  USHORT usLen
)
{
  if ( usLen )
  {
    while ( usLen-- )
    {
      *pOutData++ =   *pInData++;
//    *pOutData++ =   chAnsiToOem850[ *pInData++ ];
    } /* endwhile */
  } /* endif */
  return;
}

//------------------------------------------------------------------------------
// Function name:     SetCtrlFnt
//------------------------------------------------------------------------------
// Function call:     SetCtrlFnt(HWND, USHORT, USHORT, USHORT)
//------------------------------------------------------------------------------
// Description:       set font to display dbcs chars in dialog entryfields
//------------------------------------------------------------------------------
// Parameters:        HWND    hwndDLg        dialog handle
//                    USHORT  lfCharSet      current char set
//                    USHORT  usCtrlId       1st Ctrl that must be changed
//                    USHORT  usCtrlId       2nd Ctrl that must be changed
//------------------------------------------------------------------------------
// Returncode type:   void
//------------------------------------------------------------------------------
// Function flow:     if dbcs
//                       get font of current dialog
//                       get LOGFONT object
//                       adjust characterset and height
//                       create that font
//                       for all given dialog controls:
//                         adjust the font of the dialog control
//------------------------------------------------------------------------------

VOID SetCtrlFnt
(
   HWND    hwndDlg,                       // handle of dialog window
   USHORT  lfCharSet,
   USHORT  usCtrl1stId,
   USHORT  usCtrl2ndId
)
{
      LOGFONT lFont;
      HFONT   hFontDlg;

//      if ( _dbcs_cp == DBCS_CP )
//      {
        if (( hFontDlg = (HFONT) SendMessage( hwndDlg, WM_GETFONT, 0, 0L ))!= NULL)
        {
          if ( GetObject( hFontDlg, sizeof(LOGFONT), (LPSTR) &lFont ))
          {
            lFont.lfCharSet  = (UCHAR)lfCharSet;
            if (lFont.lfHeight > 0 )
            {
              lFont.lfHeight -=  SHEIGHTINCTRL;
            }
            else
            {
//            lFont.lfHeight += SHEIGHTINCTRL;
              lFont.lfWeight = FW_NORMAL;
            } /* endif */
            lFont.lfOutPrecision = OUT_TT_PRECIS;
            if ((hFontDlg = CreateFontIndirect( &lFont ))!= NULL)
            {
              if (usCtrl1stId)
              {
                SendDlgItemMessage( hwndDlg, usCtrl1stId,
                                    WM_SETFONT, (WPARAM)hFontDlg,
                                    MP2FROMSHORT(TRUE) );
              }
              if (usCtrl2ndId)
              {
                SendDlgItemMessage( hwndDlg, usCtrl2ndId,
                                    WM_SETFONT, (WPARAM)hFontDlg,
                                    MP2FROMSHORT(TRUE) );
              }
            }
          }
        }
//      }
   return;
} /* end of SetCtrlFnt */

VOID SetCtrlFntHwnd
(
   HWND    hwnd,                       // handle of dialog window
   USHORT  lfCharSet
)
{
   LOGFONT lFont;
   HFONT   hFontDlg;

   if (( hFontDlg = (HFONT) SendMessage( hwnd, WM_GETFONT, 0, 0L ))!= NULL)
   {
     if ( GetObject( hFontDlg, sizeof(LOGFONT), (LPSTR) &lFont ))
     {
       lFont.lfCharSet  = (UCHAR)lfCharSet;
       if (lFont.lfHeight > 0 )
       {
         lFont.lfHeight -=  SHEIGHTINCTRL;
       }
       else
       {
         lFont.lfWeight = FW_NORMAL;
       } /* endif */
       lFont.lfOutPrecision = OUT_TT_PRECIS;
       if ((hFontDlg = CreateFontIndirect( &lFont ))!= NULL)
       {
         SendMessage( hwnd,
                      WM_SETFONT, (WPARAM)hFontDlg,
                      MP2FROMSHORT(TRUE) );
       }
     }
   }
   return;
} /* end of SetCtrlFntHwnd */

//------------------------------------------------------------------------------
// Function name:     DelCtrlFont
//------------------------------------------------------------------------------
// Function call:     DelCtrlFOnt(HWND, USHORT)
//------------------------------------------------------------------------------
// Description:       delete font created for the given dialog control
//------------------------------------------------------------------------------
// Parameters:        HWND     hwndDlg
//                    USHORT   id of dialog control
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     if dbcs
//                      get handle of dialog control
//                      get font of that dialog control
//                      delete that font object
//------------------------------------------------------------------------------

VOID DelCtrlFont
(
   HWND    hwndDlg,                       // handle of dialog window
   USHORT  usCtrlId
)
{
   HFONT   hFontCtrl;
   HWND    hwndCtrl;

   //---delete the font for EFs
//   if ( _dbcs_cp == DBCS_CP )
//   {
     // get font used in the entryfield and delete it
     hwndCtrl =  GetDlgItem(hwndDlg, usCtrlId);
     if (( hFontCtrl = (HFONT) SendMessage( hwndCtrl, WM_GETFONT, 0, 0L ))!= NULL)
       DeleteObject(hFontCtrl);
     /*****************************************************************/
     /* get rid of all currently entered stuff in the ime             */
     /*****************************************************************/
#ifndef _TQM
     {
       HIMC  hImc;
       BOOL  fOpenStatus;
       hImc = ImmGetContext( hwndDlg );
       if ( hImc  )
       {
     fOpenStatus = ImmGetOpenStatus(hImc);
     ImmSetOpenStatus( hImc, FALSE );
     if (fOpenStatus)
     {
        ImmSetOpenStatus(hImc, TRUE);
     }
         ImmReleaseContext( hwndDlg, hImc );
       } /* endif */
     }
#endif
//   }
   return;
} /* end of DelCtrlFont */

VOID DelCtrlFontHwnd
(
   HWND    hwndCtrl                       // handle of dialog window
)
{
   HFONT   hFontCtrl;

   // get font used in the listcontrol and delete it
   if (( hFontCtrl = (HFONT) SendMessage( hwndCtrl, WM_GETFONT, 0, 0L ))!= NULL)
     DeleteObject(hFontCtrl);
   return;
} /* end of DelCtrlFontHwnd */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ClearIME
//------------------------------------------------------------------------------
// Function call:     ClearIME(HWND)
//------------------------------------------------------------------------------
// Description:       Clear IME buffer
//------------------------------------------------------------------------------
// Parameters:        HWND     hwndDlg
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     if IME context
//                      free IME context
//------------------------------------------------------------------------------

VOID ClearIME
(
   HWND    hwndDlg                        // handle of dialog window
)
{
  // get rid of all currently entered stuff in the ime
#ifndef _TQM
  HIMC  hImc;
  hImc = ImmGetContext( hwndDlg );
  if ( hImc  )
  {
    ImmNotifyIME( hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, (ULONG)NULL );
    ImmReleaseContext( hwndDlg, hImc );
  } /* endif */
#endif
  return;
} /* end of ClearIME */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     GetCharSet
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       return the characterset currently in use
//------------------------------------------------------------------------------
// Parameters:        none
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       character set id
//------------------------------------------------------------------------------
// Function flow:     get code page
//                    set character according to codepage
//                    character set is 0 by default ( if not dbcs etc.)
//------------------------------------------------------------------------------


USHORT GetCharSet ( VOID )
{
  USHORT   usCharSet = 0;                 // define of characterset
  DWORD         dwCodePage;
  CHARSETINFO  cs;
  CHAR         cp[10];

  GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, cp, sizeof(cp));
  dwCodePage = _ttol (cp);
  if (TranslateCharsetInfo ( (DWORD *) dwCodePage, &cs, TCI_SRCCODEPAGE))
  {
          usCharSet = (BYTE) cs.ciCharset;
  }

//cp = GetKBCodePage();
//
//if ( _dbcs_cp == DBCS_CP )
//{
//  switch(cp)
//  {
//    case 874:                          // Thai
//    case 932:                          // Japanese
//      usCharSet = SHIFTJIS_CHARSET;
//      break;
//    case 949:                          // Korean
//      usCharSet = HANGEUL_CHARSET;
//      break;
//    case 936:                          // Simplified Chinese
//    case 950:                          // Chinese (Traditional)
//      usCharSet = CHINESEBIG5_CHARSET;
//      break;
//    case 1351:                         //
//    default :
//      usCharSet = SHIFTJIS_CHARSET;
//      break;
//  } /* endswitch */
//}
//else
//{
//  switch ( cp )
//  {
//    case 852  :
//      usCharSet = EASTEUROPE_CHARSET;
//      break;
//    case 855  :
//    case 866  :
//      usCharSet = RUSSIAN_CHARSET;
//      break;
//    case 869  :
//    case 737  :
//      usCharSet = GREEK_CHARSET;
//      break;
//    case 857  :
//      usCharSet = TURKISH_CHARSET;
//      break;
//    case 862  :
//      usCharSet = HEBREW_CHARSET;
//      break;
//    case 864  :
//    case 708  :
//    case 709  :
//    case 710  :
//    case 720  :
//      usCharSet = ARABIC_CHARSET;
//      break;
//        case 874:
//                usCharSet = THAI_CHARSET;
//    case 915  :
//    case 813  :
//    default:
//      break;
//  } /* endswitch */
//
//} /* endif */
   return (usCharSet);
} /* end of GetCharSet */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ConvertAscBin
//------------------------------------------------------------------------------
// Function call:     bHex = ConvertAscBin( bByte1, bByte2 );
//------------------------------------------------------------------------------
// Description:       this function will convert two hex digits into a meaning
//                    ful byte.
//------------------------------------------------------------------------------
// Parameters:        BYTE   bByte1      first byte
//                    BYTE   bByte2      second byte
//------------------------------------------------------------------------------
// Returncode type:   BYTE
//------------------------------------------------------------------------------
// Returncodes:       value of the hex digits
//------------------------------------------------------------------------------
// Prerequesits:      bByte1 and bByte2 are valid hex digits...
//------------------------------------------------------------------------------
// Function flow:     use first character byte and convert it into a value
//                    shift the value 4 bits and add the converted second value
//------------------------------------------------------------------------------
BYTE OsWinConvertAscBin
(
  BYTE  bByte1,                        // first byte
  BYTE  bByte2                         // second byte
)
{
  BYTE  bValue;                        // return value

  /********************************************************************/
  /* get value of first byte and convert it into number               */
  /********************************************************************/
  if ( (bByte1 >= '0') && (bByte1 <= '9') )
  {
    bValue = bByte1 -'0';
  }
  else
  {
    bValue = bByte1 -'a' + 10;
  } /* endif */

  bValue <<= 4;                        // shift value for bytes


  /********************************************************************/
  /* get value of second byte and convert it into number              */
  /********************************************************************/
  if ( (bByte2 >= '0') && (bByte2 <= '9') )
  {
    bValue += bByte2 -'0';
  }
  else
  {
    bValue += bByte2 -'a' + 10;
  } /* endif */

  return bValue;
} /* end of function ConvertAscBin */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ConvertBinAsc
//------------------------------------------------------------------------------
// Function call:     ConvertBinAsc( bByte, chTempBuf );
//------------------------------------------------------------------------------
// Description:       convert byte into two byte ascii value
//------------------------------------------------------------------------------
// Parameters:        BYTE   bByte   byte to be converted
//------------------------------------------------------------------------------
// Returncode type:   PSZ
//------------------------------------------------------------------------------
// Returncodes:       pointer to filled buffer
//------------------------------------------------------------------------------
// Function flow:     get first half byte and convert it into a ascii hex numb.
//                    get 2nd half byte and convert it into a ascii hex numb
//                    return pointer to filled buffer
//------------------------------------------------------------------------------
PSZ     OsWinConvertBinAsc
(
  BYTE bByte,                          // value to be converted
  PSZ  pAscii                          // pointer to ascii value
)
{
  BYTE bTemp;                          // temporary storage

  /********************************************************************/
  /* get first half ( shift rest out )                                */
  /********************************************************************/
  bTemp = bByte >> 4;

  if ( bTemp <= 9 )
  {
    bTemp += '0';
  }
  else
  {
    bTemp += 'a' - 10;
  } /* endif */

  *pAscii = bTemp;

  /********************************************************************/
  /* get second half ..                                               */
  /********************************************************************/
  bTemp = ( bByte & 0x0F );

  if ( bTemp <= 9 )
  {
    bTemp += '0';
  }
  else
  {
    bTemp += 'a' - 10;
  } /* endif */

  *(pAscii+1) = bTemp;

  return pAscii;
} /* end of function ConvertBinAsc */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFToWPConv
//------------------------------------------------------------------------------
// Function call:     EQFToWPConv( pIn, pOut, 200 );
//------------------------------------------------------------------------------
// Description:       Convert special WP representation of Greek and other
//                    national characters into WP Word strings (i.e. 2 byte
//                    strings)
//------------------------------------------------------------------------------
// Parameters:        PUCHAR pucData  - input string (EQF format)
//                    PBYTE  pwzData  - output (WP format)
//                    USHORT usLen    - length of provided output buffer
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       length of filled output buffer
//------------------------------------------------------------------------------
// Function flow:     loop thru input string and convert it to a double byte
//                    (WP understandable) string.
//                    Take special care for SO and SI characters
//------------------------------------------------------------------------------
USHORT EQFToWPConv
(
  PUCHAR pucData,
  PBYTE  pwzData,
  USHORT usLen
)
{

  PBYTE pStart = pwzData;

  pucData; usLen;
  // function kept without code until total WP is deleted!! (03/02/25)
  return((USHORT)( pwzData - pStart + 2 ));
} /* end of function EQFToWPConv */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     WPToEQFConv
//------------------------------------------------------------------------------
// Function call:     WPToEQFConv( pIn, pOut, 200 );
//------------------------------------------------------------------------------
// Description:       Convert WordPerfect strings (i.e. 2 byte strings) into
//                    EQF representation
//------------------------------------------------------------------------------
// Parameters:        PUCHAR pucData   input string (WP 2 byte character)
//                    PBYTE  pwzData,  output string (EQF single byte)
//                    USHORT usLen     length of provided buffer
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       length of filled buffer
//------------------------------------------------------------------------------
// Function flow:     loop thru input string and convert it from 2 byte WP
//                    string into an EQF understandable string.
//                    Take special care for WP Encoded HRTs
//------------------------------------------------------------------------------
USHORT WPToEQFConv
(
  PUCHAR pucData,
  PBYTE  pwzData,
  USHORT usLen
)
{
   PBYTE pStart = pwzData;
   pucData; usLen;

 // function kept without code until total WP is deleted!! (03/02/25)
  return( (USHORT)(pwzData - pStart + 1));
} /* end of function WPToEQFConv */

/**********************************************************************/
/* Substition for OS/2 PM WinRegisterWindow function                  */
/**********************************************************************/
BOOL APIENTRY WinRegisterClass( HINSTANCE hInstance, PSZ pszClass,
                       WNDPROC lpfnWindowProc, LONG lStyle,
                       SHORT sExtraBytes )
{
  WNDCLASS  wc;

  lStyle;
  wc.style = (UINT) NULL;             /* Class style(s).                    */
  wc.lpfnWndProc = lpfnWindowProc;    /* Function to retrieve messages for  */
                                      /* windows of this class.             */
  wc.cbClsExtra = 0;                  /* No per-class extra data.           */
  wc.cbWndExtra = sExtraBytes;        /* per-window extra data.          */
  wc.hInstance = (HINSTANCE)hInstance;   /* Application that owns the class.   */
  wc.hIcon = LoadIcon((HINSTANCE) NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor((HINSTANCE) NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName =  pszClass;   /* Name of menu resource in .RC file. */
  wc.lpszClassName = pszClass; /* Name used in call to CreateWindow. */

  /* Register the window class and return success/failure code. */

  return ( RegisterClass( &wc ) != 0 );
}

/**********************************************************************/
/* Substition for OS/2 PM WinSetMultWindowPos function                */
/**********************************************************************/
BOOL APIENTRY WinSetMultWindowPos( HINSTANCE hInstance, PSWP pSwp, USHORT usWindows )
{
  BOOL fOK = TRUE;                     // success indicator
  hInstance;
  while ( usWindows && fOK)
  {
    fOK = WinSetWindowPos( pSwp->hwnd, pSwp->hwndInsertBehind,
                           pSwp->x, pSwp->y, pSwp->cx, pSwp->cy,
                           pSwp->fs );
    pSwp++;
    usWindows--;
  } /* endwhile */

  return fOK;
}

/**********************************************************************/
/* Substition for OS/2 DosGetDateTime function                        */
/**********************************************************************/
USHORT APIENTRY DosGetDateTime( PDATETIME pDateTime )
{
  time_t           lCurTime;           // buffer for current date and time
  struct tm        *pCurTime;          // ptr to C date/time structure

  // Get date and time
  time( &lCurTime );                   // get current date/time
  pCurTime = localtime( &lCurTime );   // convert to date/time structure

  // Fill callers date and time structure
  pDateTime->hours      = (UCHAR)pCurTime->tm_hour;
  pDateTime->minutes    = (UCHAR)pCurTime->tm_min;
  pDateTime->seconds    = (UCHAR)pCurTime->tm_sec;
  pDateTime->hundredths = 0;
  pDateTime->day        = (UCHAR)pCurTime->tm_mday;
  pDateTime->month      = (UCHAR)pCurTime->tm_mon + 1;
  pDateTime->year       = (USHORT)(pCurTime->tm_year + 1900);
  pDateTime->timezone   = 0;
  pDateTime->weekday    = (UCHAR)pCurTime->tm_wday;

  return( NO_ERROR );
}

SHORT APIENTRY WinDrawText
(
  HPS hps,
  SHORT cchText,
  PSZ lpchText,
  PRECTL prcl,
  LONG clrFore,
  LONG clrBack,
  USHORT rgfCmd
)
{
  RECT             rect;               // Windows rectangle

  /********************************************************************/
  /* Copy rectangle data                                              */
  /********************************************************************/
  rect.left   = (int)PRECTL_XLEFT(prcl);
  rect.right  = (int)PRECTL_XRIGHT(prcl);
  rect.top    = (int)PRECTL_YTOP(prcl);
  rect.bottom = (int)PRECTL_YBOTTOM(prcl);

  /********************************************************************/
  /* set fore and background if not default required                  */
  /********************************************************************/
  if ( clrFore != 0L )
  {
    SetTextColor(hps, UtlGetColorref(clrFore));
  } /* endif */
  if ( clrBack )
  {
    SetBkColor(hps,UtlGetColorref(clrBack));
  } /* endif */

  /********************************************************************/
  /* Use DrawText to draw text                                        */
  /********************************************************************/
  return ( (SHORT)DrawText( hps, lpchText, cchText, (RECT FAR *)&rect, rgfCmd ) );
}

/**********************************************************************/
/* Windows version of WinMapDlgPoints function                        */
/**********************************************************************/
BOOL  APIENTRY WinMapDlgPoints
(
  HWND hwndDlg,
  PPOINTL prgwptl,
  USHORT cwpt,
  BOOL fCalcWindowCoords
)
{
  RECT   rect;                         // Windows rectangle structure
  BOOL fOK = TRUE;                     // success indicator

  if ( fCalcWindowCoords )
  {
    /******************************************************************/
    /* Map to window units (pixels)                                   */
    /******************************************************************/
    while ( cwpt )
    {
      /****************************************************************/
      /* Map one point                                                */
      /****************************************************************/
      rect.right  = (int)prgwptl->x;
      rect.top    = (int)prgwptl->y;
      rect.left   = 0;
      rect.bottom = 0;
      MapDialogRect( hwndDlg, &rect );
      prgwptl->x   = (LONG)rect.right;
      prgwptl->y   = (LONG)rect.top;

      /****************************************************************/
      /* Continue with next point                                     */
      /****************************************************************/
      cwpt--;
      prgwptl++;
    } /* endwhile */
  }
  else
  {
    fOK = FALSE;
  } /* endif */
  return ( fOK );
} /* end of function WinMapDlgpoints */

HWND APIENTRY WinCreateWindow
(
  HWND hwndParent,
  PSZ pszClass,
  PSZ pszName,
  ULONG flStyle,
  LONG x, LONG y, LONG cx, LONG cy,
  HWND hwndOwner,
  HWND hwndInsertBehind,
  USHORT id,
  PVOID pCtlData,
  PVOID pPresParams
)
{
  HWND             hwnd;               // handle of new window
  hwndOwner;
  pPresParams;

  hwnd = CreateWindow(  pszClass,
                       pszName,
                       flStyle,
                       x, y, cx, cy,
                       hwndParent,
                       (HMENU)NULL,
                       (HINSTANCE)UtlQueryULong( QL_HAB ),
                        pCtlData );
  if ( hwnd != (HWND)NULL )
  {
    /******************************************************************/
    /* Set window Z-Order                                             */
    /******************************************************************/
    SetWindowPos( hwnd, hwndInsertBehind, 0, 0, 0, 0,
                  SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );

    /******************************************************************/
    /* Set window ID                                                  */
    /******************************************************************/
    SETWINDOWID( hwnd, id );
  } /* endif */

  return ( hwnd );
} /* end of function WinCreateWindow */

/**********************************************************************/
/* GetNextSelection: function to simulate the processing of           */
/*                   multiple selections in listboxes with            */
/*                   the LS_MULTIPLESEL style under OS/2 PM           */
/**********************************************************************/
SHORT GetNextSelection( HWND hwndLB, SHORT sPos )
{
  SHORT            sSelItems;          // number of selected items
  int              sNextItem;          // index of next selected item
  int              i;                  // loop index
  static int       sItems[30];         // local buffer for selected items
  int             *psItems;            // pointer to temporary item buffer

  /********************************************************************/
  /* Get number of selected items, LB_ERR indicates that the listbox  */
  /* is a single selection listbox!                                   */
  /********************************************************************/
  sSelItems = (SHORT)SendMessage( hwndLB, LB_GETSELCOUNT, 0, 0L );

  /********************************************************************/
  /* Retrieve index of selected items                                 */
  /********************************************************************/
  if ( sSelItems == LB_ERR )
  {
    /******************************************************************/
    /* That's a single selection listbox!                             */
    /* Get current selection and check if it is in front of the       */
    /* position requested                                             */
    /******************************************************************/
    sNextItem = (SHORT)SendMessage( hwndLB, LB_GETCURSEL, 0, 0L );
    if ( (sNextItem == LB_ERR) || (sNextItem <= sPos)  )
    {
      sNextItem = LIT_NONE;
    } /* endif */
  }
  else if ( sSelItems <= 30 )
  {
    /******************************************************************/
    /* Get selected items into our local buffer                       */
    /******************************************************************/
    SendMessage( hwndLB, LB_GETSELITEMS, sSelItems, MP2FROMP(sItems) );

    /******************************************************************/
    /* Search selected items for an item which is after the start     */
    /* position                                                       */
    /******************************************************************/
    sNextItem = LIT_NONE;              // assume no item will be found
    for ( i = 0 ; i < sSelItems; i++ )
    {
      if ( sItems[i] > sPos )
      {
        sNextItem = sItems[i];
        break;
      } /* endif */
    } /* endfor */
  }
  else
  {
    /******************************************************************/
    /* Allocate a temporary buffer for the selected items             */
    /******************************************************************/
    psItems = NULL;
    sNextItem = LIT_NONE;              // assume no item will be found
    UtlAlloc( (PVOID *) &psItems, 0L, (LONG)(sizeof(int) * sSelItems), ERROR_STORAGE );

    /******************************************************************/
    /* Get selected items into our temporary buffer                   */
    /******************************************************************/
    if ( psItems != NULL )
    {
      SendMessage( hwndLB, LB_GETSELITEMS, sSelItems, MP2FROMP(psItems) );
    } /* endif */

    /******************************************************************/
    /* Search selected items for an item which is after the start     */
    /* position                                                       */
    /******************************************************************/
    if ( psItems != NULL )
    {
      sNextItem = LIT_NONE;              // assume no item will be found
      for ( i = 0 ; i < sSelItems; i++ )
      {
        if ( psItems[i] > sPos )
        {
          sNextItem = psItems[i];
          break;
        } /* endif */
      } /* endfor */
    } /* endif */

    /******************************************************************/
    /* Free temporary item buffer                                     */
    /******************************************************************/
    if ( psItems != NULL )
    {
      UtlAlloc( (PVOID *) &psItems, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  return( (SHORT)sNextItem );
}


/**********************************************************************/
/* EQFAnsiToOem: do ANSI/ASCII conversion based on the currently      */
/*               selected codepages (based on Regional Settings)      */
/**********************************************************************/
VOID
EQFAnsiToOem( PSZ pIn, PSZ pOut )
{
  USHORT usInCP, usOutCP;

  usInCP = (USHORT) GetCodePage( ANSI_CP );
  usOutCP = (USHORT) GetCodePage( OEM_CP );

  // code page conversion required???
  if ( usInCP != usOutCP )
  {
    EQFCPAnsiToOem( usInCP, pIn, usOutCP, pOut );
  }
  else
  {
    // copy data without conversion
    if ( pIn != pOut )
    {
      strcpy( pOut, pIn );
    } /* endif */
  } /* endif */
}

/**********************************************************************/
/* EQFCPAnsiToOem: do ANSI/ASCII conversion based on the passed code  */
/*                 pages                                              */
/**********************************************************************/
VOID
EQFCPAnsiToOem( USHORT usInCP, PSZ pIn, USHORT usOutCP, PSZ pOut )
{
  USHORT usRc;
  PUCHAR pTable;

  usInCP;
  usRc = UtlQueryCharTableEx( ANSI_TO_ASCII_TABLE, &pTable, usOutCP );

  if ((usRc == NO_ERROR) && pTable)
  {
    BYTE c;
    while ( (c = *pIn++ ) != NULC)
    {
            *pOut++ = pTable[ c ];
    }
    *pOut = EOS;
  }
  else
  {
    // standard conversion
    AnsiToOem( pIn, pOut );
  }

}

/**********************************************************************/
/* EQFAnsiToOemBuff: do ASCII/ANSI conversion based on the currently  */
/*               selected codepages (based on Regional Settings)      */
/**********************************************************************/
VOID
EQFAnsiToOemBuff( PSZ pIn, PSZ pOut, USHORT usLen )
{
  BYTE c;
  c = pIn[usLen];
  pIn[usLen] = EOS;
  EQFAnsiToOem( pIn, pOut );
  pIn[usLen] = c;
}

/**********************************************************************/
/* EQFCPAnsiToOemBuff: do ASCII/ANSI conversion based on the passed   */
/*                 codepages                                          */
/**********************************************************************/
VOID
EQFCPAnsiToOemBuff( USHORT usInCP, PSZ pIn, USHORT usOutCP, PSZ pOut, USHORT usLen )
{
  BYTE c;
  c = pIn[usLen];
  pIn[usLen] = EOS;
  EQFCPAnsiToOem( usInCP, pIn, usOutCP, pOut );
  pIn[usLen] = c;
}

VOID
EQFCPAnsiToOemBuffL( USHORT usInCP, PSZ pIn, USHORT usOutCP, PSZ pOut, ULONG ulLen )
{
  BYTE c;
  c = pIn[ulLen];
  pIn[ulLen] = EOS;
  EQFCPAnsiToOem( usInCP, pIn, usOutCP, pOut );
  pIn[ulLen] = c;
}



/**********************************************************************/
/* EQFOemToAnsi: do ASCII/ANSI conversion based on the currently      */
/*               selected codepages (based on Regional Settings)      */
/**********************************************************************/

VOID
EQFOemToAnsi( PSZ pIn, PSZ pOut )
{
  USHORT usInCP, usOutCP;

  usInCP = (USHORT) GetCodePage( OEM_CP );
  usOutCP = (USHORT) GetCodePage( ANSI_CP );

  // code page conversion required???
  if ( usInCP != usOutCP )
  {
    EQFCPOemToAnsi( usInCP, pIn, usOutCP, pOut );
  }
  else
  {
    // copy data without conversion
    if ( pIn != pOut )
    {
      strcpy( pOut, pIn );
    } /* endif */
  } /* endif */
}

/**********************************************************************/
/* EQFCPOemToAnsi: do ASCII/ANSI conversion based on the passed code  */
/*                 pages                                              */
/**********************************************************************/
VOID
EQFCPOemToAnsi( USHORT usInCP, PSZ pIn, USHORT usOutCP, PSZ pOut )
{
  USHORT usRc;
  PUCHAR pTable;
  usOutCP;

  usRc = UtlQueryCharTableEx( ASCII_TO_ANSI_TABLE, &pTable, usInCP );

  if ((usRc == NO_ERROR) && pTable)
  {
    // use our conversion routines
    BYTE c;
    while ( (c = *pIn++) != NULC)
    {
            *pOut++ = pTable[ c ];
    }
    *pOut = EOS;
  }
  else
  {
    // standard conversion
    OemToAnsi( pIn, pOut );
  }


}

/**********************************************************************/
/* EQFOemToAnsiBuff: do ASCII/ANSI conversion based on the currently  */
/*               selected codepages (based on Regional Settings)      */
/**********************************************************************/
VOID
EQFOemToAnsiBuff( PSZ pIn, PSZ pOut, USHORT usLen )
{
  BYTE c;
  c = pIn[usLen];
  pIn[usLen] = EOS;
  EQFOemToAnsi( pIn, pOut );
  pIn[usLen] = c;
}

/**********************************************************************/
/* EQFCPOemToAnsiBuff: do ASCII/ANSI conversion based on the passed   */
/*                 codepages                                          */
/**********************************************************************/
VOID
EQFCPOemToAnsiBuff( USHORT usInCP, PSZ pIn, USHORT usOutCP, PSZ pOut, USHORT usLen )
{
  BYTE c;
  c = pIn[usLen];
  pIn[usLen] = EOS;
  EQFCPOemToAnsi( usInCP, pIn, usOutCP, pOut );
  pIn[usLen] = c;
}

/**********************************************************************/
/* Get Codepage appropriate for the selected source and target langs  */
/**********************************************************************/
// should not be used in System-independent handling!
ULONG GetCodePage( USHORT usType )
{
  CHAR   cp[10];
  ULONG  ulReturnCP = 0L;

  switch ( usType )
  {
    case OEM_CP:
      GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTCODEPAGE, cp, sizeof(cp));
      ulReturnCP = (ULONG)_ttol (cp);
      if (ulReturnCP == 720L)
      {
        ulReturnCP = 864L;
      }
      else if ((ulReturnCP == 737L) && (GetOEMCP() == 869) && (GetKBCodePage() == 869 ) )
      {
        // fix for sev1 Greek: Win NT problem (01/09/23)
        ulReturnCP = 869L;
      }
      break;
    case ANSI_CP:
    default:
      GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, cp, sizeof(cp));
      ulReturnCP = (ULONG) _ttol (&cp[0]);
      break;
  } /* endswitch */

  return (ulReturnCP);
}

////////////////////////////////////////////////////////////////
// Get Codepage appropriate for given language ( not OS-dependent!)
///////////////////////////////////////////////////////////////////
ULONG GetLangCodePage( USHORT usType, PSZ pLanguage )
{
  ULONG  ulReturnCP = 0L;

  switch ( usType )
  {
    case OEM_CP:
         // default if nothing else is avail.
         ulReturnCP = GetLangOEMCP(pLanguage);
         break;
    case ANSI_CP:
    default:
         ulReturnCP = GetLangAnsiCP(pLanguage);
         break;
  } /* endswitch */

  return (ulReturnCP);
}


/**********************************************************************/
/* allow to fill the rectangle in the selected color                  */
/**********************************************************************/
void ERASERECT(HDC hdc, const RECT* pRect, LONG bg)
{
	HBRUSH hbr = CreateSolidBrush(UtlGetColorref(bg));
	HBRUSH hbrOld = (HBRUSH) SelectObject(hdc, hbr);
	FillRect(hdc, pRect, hbr);
	SelectObject(hdc, hbrOld);
	DeleteObject(hbr);
}