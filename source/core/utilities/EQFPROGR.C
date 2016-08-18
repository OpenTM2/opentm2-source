/*! \file
	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/
#include <eqf.h>
#include <custcntl.h>
#include "eqfprogr.h"
#include "eqfprogi.h"

//BOOL PASCAL RegisterProgressControl(HINSTANCE hInstance)
BOOL RegisterProgressControl(HINSTANCE hInstance)
{
  static BOOL     fRegistered=FALSE;
  WNDCLASS        wc;

  if (!fRegistered)
  {
    wc.lpfnWndProc   = ProgressBarWndProc;
    wc.cbClsExtra    = CBCLASSEXTRA;
    wc.cbWndExtra    = CBWINDOWEXTRA;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0L;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = PROGRESSCONTROL;
    wc.style         = CS_GLOBALCLASS | CS_VREDRAW | CS_HREDRAW;

    fRegistered=RegisterClass(&wc);
  }

  return fRegistered;
}


LRESULT CALLBACK ProgressBarWndProc
(
  HWND hWnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam
)
{
    LPCREATESTRUCT  lpCreate;
    PPROGRESSBAR    pPB;
    PAINTSTRUCT     ps;
    RECT            rc1, rc2, rcT;
    WORD            dx, dy, x, y;
    WORD            iRange, iPos;
    char            ach[30];
    LONG            cx, cy;
    BOOL            bVert;

    pPB=(PPROGRESSBAR)GetWindowLong(hWnd, GWL_PROGRESSBARHMEM);

    switch(uMsg)
        {
        case WM_NCCREATE:
            UtlAlloc( (PVOID *) &pPB, 0L, (LONG)CBPROGRESSBAR, NOMSG );

            if ( pPB == NULL )
                return 0L;
            SetWindowLong(hWnd, GWL_PROGRESSBARHMEM, (LONG)pPB );
            return TRUE;

        case WM_CREATE:
            // Set Style Defaults
            lpCreate = (LPCREATESTRUCT)lParam;
            pPB->dwStyle  = lpCreate->style;

            if((PBS_VERTICAL & pPB->dwStyle) &&
              (PBS_HORIZONTAL & pPB->dwStyle))
                pPB->dwStyle &= ~PBS_HORIZONTAL;
            pPB->rgb = RGB(0,0,255);
            return TRUE;

        case WM_NCDESTROY:
            UtlAlloc( (PVOID *) &pPB, 0L, 0L, NOMSG );
            break;

        case PB_SETRANGE:
            pPB->iRange = LOWORD(wParam);
            InvalidateRect(hWnd,NULL,FALSE);
            UpdateWindow(hWnd);
            return FALSE;

        case PB_SETPOS:
            pPB->iPos = LOWORD(wParam);
            InvalidateRect(hWnd,NULL,FALSE);
            UpdateWindow(hWnd);
            return FALSE;

        case PB_SETCOLOR:
            pPB->rgb = lParam;
            InvalidateRect(hWnd,NULL,FALSE);
            UpdateWindow(hWnd);
            return FALSE;

        case WM_SETFOCUS:
            hWnd = hWnd;
            break;


        case WM_GETDLGCODE:
            return( DLGC_WANTALLKEYS );
            break;

        case WM_KEYDOWN:
            if ( wParam == VK_F1 )
            {
              /**************************************************************/
              /* Post message to the process window                         */
              /**************************************************************/
              PostMessage( GetParent( hWnd ), WM_KEYDOWN, wParam, lParam );
            } /* endif */
            break;

        case WM_PAINT:
            BeginPaint(hWnd, &ps);
            if(PBS_VERTICAL & pPB->dwStyle)
                bVert = TRUE;
            else
                bVert = FALSE;

            GetClientRect(hWnd, &rcT);
            rc1 = rcT;
            if(PBS_CHISELED & pPB->dwStyle)
                InflateRect(&rc1, -2, -2);
            else
                InflateRect(&rc1, -1,-1);
            rc2 = rc1;

            iRange = pPB->iRange;
            iPos =   pPB->iPos;

            if(iRange <= 0)
                iRange = 1;

            if(iPos > iRange)
                iPos = iRange;

            dx = (USHORT)rc1.right;
            dy = (USHORT)rc1.bottom;

            x  = (WORD)((DWORD)iPos * dx / iRange) + 1;
            y  = (WORD)((DWORD)iPos * dy / iRange) - 1;

            if(PBS_PERCENTAGE & pPB->dwStyle)
                wsprintf(ach,"%3d%%",
                    (WORD)((DWORD)iPos * 100 / iRange));
            else
                lstrcpy(ach,"");

            TEXTSIZE( ps.hdc, ach, cx, cy );

            if(bVert)
                {
                rc1.top -= y;
                rc2.bottom -= y;
                }
            else
                {
                rc1.right = x;
                rc2.left  = x;
                }

            SetBkColor(ps.hdc, pPB->rgb);
            SetTextColor(ps.hdc, RGB(192,192,192));
            ExtTextOut(ps.hdc,
                (dx-cx)/2,(dy-cy)/2,
                ETO_OPAQUE | ETO_CLIPPED,
                &rc1, ach,lstrlen(ach),NULL);

            SetBkColor(ps.hdc, RGB(192,192,192));
            SetTextColor(ps.hdc, pPB->rgb);
            ExtTextOut(ps.hdc,
                (dx-cx)/2,(dy-cy)/2,
                ETO_OPAQUE | ETO_CLIPPED,
                &rc2, ach,lstrlen(ach),NULL);

            if(PBS_CHISELED & pPB->dwStyle)
                {
                SetBkColor(ps.hdc, RGB(255,255,255));

                // top of rect.
                rc2.left = 1;
                rc2.bottom = 2;
                rc2.top = 1;
                rc2.right = rc2.right + 2;
                ExtTextOut(ps.hdc,
                    1,1, ETO_OPAQUE | ETO_CLIPPED,
                    &rc2, "",0,NULL);

                // left of rect.
                rc2.left = 1;
                rc2.top = 1;
                rc2.right = 2;
                rc2.bottom = rcT.bottom;
                ExtTextOut(ps.hdc,
                    1,1, ETO_OPAQUE | ETO_CLIPPED,
                    &rc2, "",0,NULL);

                SetBkColor(ps.hdc, RGB(128,128,128));
                // right of rect.
                rc2.left = rcT.right - 2;
                rc2.top = 1;
                rc2.right = rcT.right;
                ExtTextOut(ps.hdc,
                    1,1, ETO_OPAQUE | ETO_CLIPPED,
                    &rc2, "",0,NULL);

                // bottom of rect.
                rc2.left = 1;
                rc2.top = rcT.bottom - 2;
                rc2.bottom = rcT.bottom;
                rc2.right = rcT.right;
                ExtTextOut(ps.hdc,
                    1,1, ETO_OPAQUE | ETO_CLIPPED,
                    &rc2, "",0,NULL);
                }
            FrameRect(ps.hdc, &rcT, (HBRUSH) GetStockObject(BLACK_BRUSH));

            EndPaint(hWnd,(LPPAINTSTRUCT)&ps);
            return 0L;
        default:
            return DefWindowProc(hWnd,uMsg,wParam,lParam);
        }
    return 0L;
}


