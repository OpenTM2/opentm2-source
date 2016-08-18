//+----------------------------------------------------------------------------+
//|  EQFUTMDI.C - EQF Modeless MDI Dialog                                      |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+
//|  Entry Points:                                                             |
//+----------------------------------------------------------------------------+
//|  Externals:                                                                |
//+----------------------------------------------------------------------------+
//|  Internals:                                                                |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//+----------------------------------------------------------------------------+
//|  To be done / known limitations / caveats:                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.1 $ ----------- 30 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.3 $ ----------- 17 Mar 2003
// --RJ: remove redundant IsUnicodeSystem, since only UnicodeSystems are supported any more
// 
//
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
//
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.3 $ ----------- 4 Sep 2002
// --RJ: R07197: use installed ANSI-CP for display of MDI windows
//
//
// $Revision: 1.2 $ ----------- 30 Jul 2002
// --RJ: add cp for conversion ; delete Non-WIn32-bit branches
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.4 $ ----------- 17 Dec 2001
// RJ: Add code to use correct DefWindowProc/DefWindowProcW
//
//
// $Revision: 1.3 $ ----------- 7 Dec 2001
// RJ: fix problem in Global Find&Change
//
//
// $Revision: 1.2 $ ----------- 3 Sep 2001
// -- RJ: get rid of warnings
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
// $Revision: 1.5 $ ----------- 18 Jun 2001
// GQ: Prevent MDI dialogs from maximizing (MFC trap if a non-MFC MDI child window
//     is maximized)
//
//
// $Revision: 1.4 $ ----------- 19 Feb 2001
// -- fix problem in returning font in WM_GETFONT
//
//
// $Revision: 1.3 $ ----------- 21 Jun 2000
// - Win32: use Widnows default GUI font for controls in MDI dialogs
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFUTMDI.CV_   1.3   26 Jan 1998 11:26:34   BUILD  $
 *
 * $Log:   J:\DATA\EQFUTMDI.CV_  $
 *
 *    Rev 1.3   26 Jan 1998 11:26:34   BUILD
 * - corrected dialog template scanning in Win32 environment
 *
 *    Rev 1.2   19 Jan 1998 11:03:02   BUILD
 * - corrected handling of WM_MDIACTIVATE message under Win32
 * - use a dialog IDA instead of several words in class/window extra bytes
 *   area (area overflows under Win95 when using 32bit compiler)
 *
 *    Rev 1.1   14 Jan 1998 15:26:52   BUILD
 * - migration to 32bit Windows SDK
 *
 *
 *    Rev 1.0   09 Jan 1996 09:16:58   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
#include <eqf.h>

#include <memory.h>
#include "eqfutmdi.h"


#define DEFMDICHILDPROC( hDlg, uMsg, wParam, lParam ) \
   DefMDIChildProcW( hDlg, uMsg, wParam, lParam )

// global definitions

char szMDIDialogClass[] = WC_EQFMDIDLG;

// Helper function to align a pointer on DWORD boundaries


PVOID DWORDAlign ( PVOID lpIn)
{
    ULONG ul;

    ul = (ULONG) lpIn;
    ul +=3;
    ul >>=2;
    ul <<=2;
    return (PVOID) ul;
}

//---------------------------------------------------------------------------
//  BOOL FAR PASCAL RegisterMDIDialog( HINSTANCE hInstance )
//
//  Description:
//     Registers the "MDIDialog" class.
//
//  Parameters:
//     HINSTANCE hInstance
//        handle to instance
//
//---------------------------------------------------------------------------

BOOL FAR PASCAL RegisterMDIDialog( HINSTANCE hInstance )
{
   WNDCLASS  wc ;
   BOOL      fOK = TRUE;
   USHORT    usRC = NO_ERROR;

   // register MDIDialog window class
   wc.style =         CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc =   MDIDialogProc ;
   wc.cbClsExtra =    0 ;
//old:   wc.cbWndExtra =    DLGWINDOWEXTRA + GWW_MDIDIALOGEXTRABYTES ;
   wc.cbWndExtra =    DLGWINDOWEXTRA + sizeof(PVOID);
   wc.hInstance =     hInstance ;
//   wc.hIcon =         (HPOINTER) UtlQueryULong(QL_FOLICON); //hiconFOL;
   wc.hIcon =         LoadIcon( NULL, IDI_APPLICATION );
   wc.hCursor =       LoadCursor( NULL, IDC_ARROW ) ;
   wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1) ;
   wc.lpszMenuName =  NULL ;
   wc.lpszClassName = szMDIDialogClass ;

   if ( RegisterClass( &wc ) == 0 )
   {
     usRC = (USHORT)GetLastError();
     fOK = FALSE;
   } /* endif */
   return( fOK );
} // end of RegisterMDIDialog()

//------------------------------------------------------------------------
//  HWND FAR PASCAL CreateMDIDialogParam( HINSTANCE hInstance,
//                                        LPCSTR lpTemplateName,
//                                        HWND hClientWnd,
//                                        FARPROC lpDialogProc,
//                                        LPARAM lParam )
//
//  Description:
//     This function creates an MDI child of the "MDIDialog"
//     class.
//
//  Parameters:
//     HINSTANCE  hInstance
//        handle to instance
//
//     LPCSTR lpTemplateName
//        pointer to template name
//
//     HWND hClientWnd
//        handle to MDI client window
//
//     DLGPROC lpDialogProc
//        pointer to dialog procedure
//
//     LPARAM lParam
//        dword parameter passed in WM_INITDIALOG to dialog procedure
//
//------------------------------------------------------------------------

HWND FAR PASCAL CreateMDIDialogParam
(
  HINSTANCE hInstance,
  LPCSTR lpTemplateName,
  HWND hClientWnd,
  FARPROC lpDialogProc,
  LPARAM lParam,
  BOOL    fDialogFrame,
  HICON   hIcon
)
{
   HDC                      hDC ;
   HFONT                    hFont, hOldFont ;
   HGLOBAL                  hRes ;
   HMENU                    hMenu = NULL ;
   HRSRC                    hDlgRes ;
   HWND                     hDlg ;
   LOGFONT                  lf ;
   LPSTR                    lpDlgRes ;
   MDICREATESTRUCT          MDIcs ;
   MDIDIALOGINFO            MDIdi ;
   RECT                     rcWnd ;
   TEXTMETRIC               tm ;
   WORD                     wUnitsX, wUnitsY ;

   // local variables used when parsing the dialog template

   int                      dtX, dtY, dtCX, dtCY ;
   char                     dtMenuName[ MAXLEN_MENUNAME ],
                            dtClassName[ MAXLEN_CLASSNAME ],
                            dtCaptionText[ MAXLEN_CAPTIONTEXT ],
                            dtTypeFace[ MAXLEN_TYPEFACE ] ;
   BYTE                     dtItemCount ;
   DWORD                    dtStyle ;
   WORD                     dtPointSize ;


   hClientWnd;
   hDlgRes = FindResource( hInstance, lpTemplateName, RT_DIALOG ) ;
   hRes = LoadResource( hInstance, hDlgRes ) ;
   lpDlgRes = (LPSTR) LockResource( hRes ) ;

   // get style, item count, and initial position, size
   {
     LPDLGTEMPLATE pDlgTemplate = (LPDLGTEMPLATE)lpDlgRes;
     dtStyle = pDlgTemplate->style;
     dtItemCount = (BYTE)pDlgTemplate->cdit;
     dtX = pDlgTemplate->x;
     dtY = pDlgTemplate->y;;
     dtCX = pDlgTemplate->cx;
     dtCY = pDlgTemplate->cy;
     lpDlgRes += sizeof(DLGTEMPLATE);
   }
   // get menu name and load it if specified
   // Convert unicode menu name or use ordinal
   {
     LPCWSTR pName = (LPCWSTR)lpDlgRes;

     if ( *pName == 0 )
     {
       // no menu specified
       hMenu = NULL;
       lpDlgRes += sizeof(WORD);
     }
     else if ( *pName == 0xFFFF)
     {
       // menu ordinal specified
       pName++;
       hMenu = LoadMenu( hInstance, MAKEINTRESOURCE( *pName ) );
       pName++;
       lpDlgRes = (LPSTR)pName;
     }
     else
     {
       // convert unicode menu name to multy byte string and load menu
       BOOL fDefaultChar;
       WideCharToMultiByte( CP_ACP,               // use default Ansi code page
                           WC_COMPOSITECHECK,    // combine composite chars
                           pName,                // ptr to unicode string
                           -1,                   // compute length
                           dtMenuName,           // buffer for result string
                           sizeof(dtMenuName),   // size of result buffer
                           NULL,                 // use system default char
                           &fDefaultChar );      // default has been used flag
       while ( *pName != 0)
       {
         pName++;
       }
       pName++;
       lpDlgRes = (LPSTR)pName;
     } /* endif */
   }

   // get class name
   // Convert unicode class name
   {
     LPCWSTR pName = (LPCWSTR)lpDlgRes;

     if ( *pName == 0xFFFF)
     {
      // ordinal value of predefined window class is used
        pName++;
        *pName++; // where to store ordinal value ???
        lpDlgRes = (LPSTR)pName;
     }
     else
     {
       BOOL fDefaultChar;
       WideCharToMultiByte( CP_ACP,               // use default Ansi code page
                            WC_COMPOSITECHECK,    // combine composite chars
                            pName,                // ptr to unicode string
                            -1,                   // compute length
                            dtClassName,          // buffer for result string
                            sizeof(dtClassName),  // size of result buffer
                            NULL,                 // use system default char
                            &fDefaultChar );      // default has been used flag
        while ( *pName != 0) pName++;
        pName++;
        lpDlgRes = (LPSTR)pName;
     } /* endif */
   }

   // get caption text
   // Convert unicode caption text
   {
     LPCWSTR pName = (LPCWSTR)lpDlgRes;
     BOOL fDefaultChar;
     WideCharToMultiByte( CP_ACP,               // use default Ansi code page
                          WC_COMPOSITECHECK,    // combine composite chars
                          pName,                // ptr to unicode string
                          -1,                   // compute length
                          dtCaptionText,        // buffer for result string
                          sizeof(dtCaptionText),// size of result buffer
                          NULL,                 // use system default char
                          &fDefaultChar );      // default has been used flag
      while ( *pName != 0)
      {
        pName++;
      }
      pName++;
      lpDlgRes = (LPSTR)pName;
   }

   hDC = GetDC( NULL ) ;

   if (dtStyle & DS_SETFONT)
   {
      // get point size
      WORD FAR *pPointSize = (WORD FAR *)lpDlgRes;
      dtPointSize = *pPointSize++;
      lpDlgRes = (LPSTR)pPointSize;
      // get face name
      {
        LPCWSTR pName = (LPCWSTR)lpDlgRes;
        BOOL fDefaultChar;
        WideCharToMultiByte( CP_ACP,               // use default Ansi code page
                             WC_COMPOSITECHECK,    // combine composite chars
                             pName,                // ptr to unicode string
                             -1,                   // compute length
                             dtTypeFace,           // buffer for result string
                             sizeof(dtTypeFace),   // size of result buffer
                             NULL,                 // use system default char
                             &fDefaultChar );      // default has been used flag
        while ( *pName != 0) pName++;
        pName++;
        lpDlgRes = (LPSTR)pName;
      }
      memset( &lf, 0, sizeof( LOGFONT ) ) ;
      lstrcpy( lf.lfFaceName, dtTypeFace ) ;
      lf.lfHeight = -MulDiv( dtPointSize,
                             GetDeviceCaps( hDC, LOGPIXELSY ),
                             72 )  ;
      lf.lfWeight = FW_BOLD ;
      // use correct character set in case of DBCS

      if ( IsDBCS_CP((ULONG)GetCodePage( ANSI_CP )) )
      {
        lf.lfCharSet = (BYTE)GetCharSet();
        lf.lfOutPrecision = OUT_TT_PRECIS;
      } /* endif */
      hFont = CreateFontIndirect( &lf ) ;
   }
   else
      hFont = NULL ;

   // compute width and height of font

   hOldFont = (HFONT)
      SelectObject( hDC,
                    (hFont == NULL) ? GetStockObject( SYSTEM_FONT ) : hFont ) ;
   GetTextMetrics( hDC, &tm ) ;
   {
     DWORD cx, cy;
     TEXTSIZE( hDC, WIDTHSTRING, cx, cy );
     wUnitsX = (USHORT)(cx / 26);
   }

   wUnitsX = (wUnitsX + 1) / 2 ;
   wUnitsY = (USHORT)tm.tmHeight ;
   SelectObject( hDC, hOldFont ) ;

   ReleaseDC( NULL, hDC ) ;

   rcWnd.left = 0 ;
   rcWnd.top = 0 ;
   rcWnd.right = DLGTOCLIENTX( dtCX, wUnitsX ) ;
   rcWnd.bottom = DLGTOCLIENTY( dtCY, wUnitsY ) ;
   AdjustWindowRect( &rcWnd, dtStyle, FALSE ) ;

   // create the MDI dialog window

   MDIdi.hFont =        hFont ;
   MDIdi.hMenu =        hMenu ;
   MDIdi.lpDialogProc = (WNDPROC) lpDialogProc ;
   MDIdi.lpDlgItems =   lpDlgRes ;
   MDIdi.wUnitsX =      wUnitsX ;
   MDIdi.wUnitsY =      wUnitsY ;
   MDIdi.dtItemCount =  (int) dtItemCount ;
   MDIdi.lParam =       lParam ;
   MDIdi.fDialogFrame = fDialogFrame;
   MDIdi.hIcon        = hIcon;

   dtStyle &= WS_MDIALLOWED ;

   MDIcs.szClass =      szMDIDialogClass ;
   MDIcs.szTitle =      dtCaptionText ;
   MDIcs.hOwner =       (HAB)UtlQueryULong( QL_HAB );
   MDIcs.x =            CW_USEDEFAULT ;
   MDIcs.y =            CW_USEDEFAULT ;
   MDIcs.cx =           rcWnd.right - rcWnd.left ;
// MDIcs.cy =           rcWnd.bottom - rcWnd.top ;
   MDIcs.cy =           rcWnd.bottom - rcWnd.top + (2 * GetSystemMetrics( SM_CYFRAME ));
   if ( fDialogFrame )
   {
     MDIcs.style =        (dtStyle | WS_MDIDLGCHILD ) &  ~WS_MAXIMIZEBOX ;
   }
   else
   {
     MDIcs.style =        (dtStyle | WS_MDICHILD ) &  ~WS_MAXIMIZEBOX ;
   } /* endif */
   MDIcs.lParam =       (LPARAM) (LPMDIDIALOGINFO) &MDIdi ;

   hDlg =
      (HWND) SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                                  WM_MDICREATE, 0,
                                  (LPARAM) (LPMDICREATESTRUCT) &MDIcs ) ;

   UnlockResource( hRes ) ;
   FreeResource( hRes ) ;

   return ( hDlg ) ;

} // end of CreateMDIDialogParam()

//------------------------------------------------------------------------
//  BOOL NEAR CreateMDIDialogChildren( HWND hDlg, LPSTR lpDlgItems,
//                                     int dtItemCount, HFONT hFont,
//                                     WORD wUnitsX, WORD wUnitsY )
//
//  Description:
//     This function is used during the WM_CREATE of the MDIDialogProc().
//
//  Parameters:
//     HWND hDlg
//        handle to MDI dialog window
//
//     LPSTR lpDlgItems
//        pointer to dialog items
//
//     int dtItemCount
//        count of dialog items
//
//     HFONT hFont
//        handle to dialog font
//
//     WORD wUnitsX
//        size of dialog units (X)
//
//     WORD wUnitsY
//        size of dialog units (Y)
//
//------------------------------------------------------------------------

BOOL NEAR CreateMDIDialogChildren( HWND hDlg, LPSTR lpDlgItems,
                                   int dtItemCount, HFONT hFont,
                                   WORD wUnitsX, WORD wUnitsY )
{
   int      nCtl ;
   HWND     hCtl ;
   LPDLGITEMTEMPLATE pItemTemplate;

   // local vars used when parsing the dialog template

   int      dtID, dtX, dtY, dtCX, dtCY ;
   char     dtClassName[ MAXLEN_CLASSNAME ],
            dtCaptionText[ MAXLEN_CAPTIONTEXT ] ;
   BYTE     dtInfoSize ;
   DWORD    dtStyle ;

   hFont;
   for (nCtl = 0; nCtl < dtItemCount; nCtl++)
   {
      lpDlgItems = (LPSTR) DWORDAlign( lpDlgItems );
      pItemTemplate = (LPDLGITEMTEMPLATE)lpDlgItems;
      dtX = pItemTemplate->x;
      dtY = pItemTemplate->y;
      dtCX = pItemTemplate->cx;
      dtCY = pItemTemplate->cy;
      dtID = pItemTemplate->id;
      dtStyle = pItemTemplate->style;
      lpDlgItems += sizeof(DLGITEMTEMPLATE);

      // get class name
      {
        LPCWSTR pName = (LPCWSTR)lpDlgItems;

        if ( *pName == 0xFFFF)
        {
          pName++;
          switch ( *pName )
          {
             case 0x80:
                lstrcpy( dtClassName, "BUTTON" ) ;
                break ;

             case 0x81:
                lstrcpy( dtClassName, "EDIT" ) ;
                break ;

             case 0x82:
                lstrcpy( dtClassName, "STATIC" ) ;
                break ;

             case 0x83:
                lstrcpy( dtClassName, "LISTBOX" ) ;
                break ;

             case 0x84:
                lstrcpy( dtClassName, "SCROLLBAR" ) ;
                break ;

             case 0x85:
                lstrcpy( dtClassName, "COMBOBOX" ) ;
                break ;
          }
          pName++;
          lpDlgItems = (LPSTR)pName;
        }
        else
        {
          // convert unicode menu name to multy byte string and load menu
          BOOL fDefaultChar;
          WideCharToMultiByte( CP_ACP,               // use default Ansi code page
                              WC_COMPOSITECHECK,    // combine composite chars
                              pName,                // ptr to unicode string
                              -1,                   // compute length
                              dtClassName,           // buffer for result string
                              sizeof(dtClassName),   // size of result buffer
                              NULL,                 // use system default char
                              &fDefaultChar );      // default has been used flag
          while ( *pName != 0) pName++;
          pName++;
          lpDlgItems = (LPSTR)pName;
        } /* endif */
      }



      // get caption text
      {
        LPCWSTR pName = (LPCWSTR)lpDlgItems;

        BOOL fDefaultChar;
        WideCharToMultiByte( CP_ACP,              // use default Ansi code page
                            WC_COMPOSITECHECK,    // combine composite chars
                            pName,                // ptr to unicode string
                            -1,                   // compute length
                            dtCaptionText,        // buffer for result string
                            sizeof(dtCaptionText),// size of result buffer
                            NULL,                 // use system default char
                            &fDefaultChar );      // default has been used flag
        while ( *pName != 0) pName++;
        pName++;
        lpDlgItems = (LPSTR)pName;
      }

	  {
        WORD *pInfoSize = (WORD *)lpDlgItems;
        dtInfoSize = (BYTE)*pInfoSize++;
		lpDlgItems = (LPSTR)pInfoSize;
      }
      hCtl = CreateWindowEx( WS_EX_NOPARENTNOTIFY,
                             dtClassName, dtCaptionText, dtStyle,
                             DLGTOCLIENTX( dtX, wUnitsX ),
                             DLGTOCLIENTY( dtY, wUnitsY ),
                             DLGTOCLIENTX( dtCX, wUnitsX ),
                             DLGTOCLIENTY( dtCY, wUnitsY ),
                             hDlg, (HMENU) dtID, GETINSTANCE( hDlg ),
                             (dtInfoSize == 0) ? NULL :
                                                 (VOID FAR *) lpDlgItems ) ;

//    SendMessage( hCtl, WM_SETFONT,
//                 WPARAM) ((hFont == NULL) ? GetStockObject( DEFAULT_GUI_FONT ) :
//                        hFont),
//                 LPARAM) FALSE ) ;

      SendMessage( hCtl, WM_SETFONT,
                   (WPARAM)GetStockObject( DEFAULT_GUI_FONT ),
                   (LPARAM) FALSE ) ;


      // bounce over child info structure

      lpDlgItems += dtInfoSize ;
   }
   return ( TRUE ) ;
} // end of CreateMDIDialogChildren()

//------------------------------------------------------------------------
//  LRESULT FAR PASCAL MDIDialogProc( HWND hDlg, UINT uMsg,
//                                    WPARAM wParam, LPARAM lParam )
//
//  Description:
//     This is the MDI dialog procedure.
//
//  Parameters:
//     HWND hDlg
//        handle to MDI dialog window
//
//     UINT uMsg
//        current message
//
//     WPARAM wParam
//        word parameter
//
//     LPARAM lParam
//        dword parameter
//
//------------------------------------------------------------------------

LRESULT FAR PASCAL MDIDialogProc( HWND hDlg, UINT uMsg,
                                  WPARAM wParam, LPARAM lParam )
{
   switch (uMsg)
   {
      case WM_CREATE:
      {
         int              nPos ;
         LPMDIDIALOGINFO  lpMDIdi ;
         LPMDIDIALOGINFO  lpDlgIda;

         HMENU hSysMenu;         // handle of system menu

         lParam = (LPARAM) ((LPCREATESTRUCT) lParam) -> lpCreateParams ;
         lpMDIdi =
            (LPMDIDIALOGINFO) ((LPMDICREATESTRUCT) lParam) -> lParam ;

//old:         SetWindowLong( hDlg, DLGWINDOWEXTRA + GWL_MDIDIALOGPROC,
//old:                        (LONG) lpMDIdi -> lpDialogProc ) ;
//old:         SetWindowWord( hDlg, DLGWINDOWEXTRA + GWW_MDIDIALOGHFONT,
//old:                        (WORD) lpMDIdi -> hFont ) ;
//old:         SetWindowWord( hDlg, DLGWINDOWEXTRA + GWW_MDIDIALOGHMENU,
//old:                        (WORD) lpMDIdi -> hMenu ) ;
//old:         SetWindowWord( hDlg, DLGWINDOWEXTRA + GWW_MDIDIALOGHICON,
//old:                        (WORD) lpMDIdi -> hIcon ) ;

         lpDlgIda = (LPMDIDIALOGINFO) malloc( sizeof(MDIDIALOGINFO) );
         memcpy( lpDlgIda, lpMDIdi, sizeof(MDIDIALOGINFO) );
         SetWindowLong( hDlg, DLGWINDOWEXTRA, (LONG)lpDlgIda ) ;


         /*************************************************************/
         /* Set extended style if MDI child window should have a      */
         /* dialog border                                             */
         /*************************************************************/
         if ( lpMDIdi->fDialogFrame )
         {
           LONG lStyle = GetWindowLong( hDlg, GWL_STYLE );

           /***********************************************************/
           /* remove minimize and maximize buttons from titlebar      */
           /***********************************************************/
           lStyle &= ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX );
           SetWindowLong( hDlg, GWL_STYLE, lStyle );

           /***********************************************************/
           /* Set modal dialog border                                 */
           /***********************************************************/
           lStyle = GetWindowLong( hDlg, GWL_EXSTYLE );
           lStyle |= WS_EX_DLGMODALFRAME;
           SetWindowLong( hDlg, GWL_EXSTYLE, lStyle );

           /***********************************************************/
           /* Disable system menu items not used for dialogs          */
           /***********************************************************/
           hSysMenu = GetSystemMenu( hDlg, FALSE );
           if ( hSysMenu != NULL )
           {
             EnableMenuItem( hSysMenu, SC_RESTORE,  MF_GRAYED );
    //         EnableMenuItem( hSysMenu, SC_SIZE,     MF_GRAYED );
             EnableMenuItem( hSysMenu, SC_MINIMIZE, MF_GRAYED );
             EnableMenuItem( hSysMenu, SC_MAXIMIZE, MF_GRAYED );
           } /* endif */
         } /* endif */


         // Get the "Window" sub-menu handle

         // NOTE: A _VERY_ big assumption is made here... I assume
         // that the menu is an MDI type of menu and that the position
         // of the Window sub-menu is the second to last item in the menu.

         //                                 nPos
         //                                  \/
         // +----------+----------+------+--------+------+
         // | MENUITEM | MENUITEM | ...  | WINDOW | HELP |
         // +----------+----------+------+--------+------+

         if (NULL != lpMDIdi -> hMenu)
         {
            if ((nPos = GetMenuItemCount( lpMDIdi -> hMenu ) - 2) < 0)
               nPos = 0 ;
//old:            SetWindowWord( hDlg, DLGWINDOWEXTRA + GWW_MDIDIALOGHWNDMENU,
//old:                           (WORD) GetSubMenu( lpMDIdi -> hMenu, nPos ) ) ;
         }

         // send font notification

         lpMDIdi -> lpDialogProc( hDlg, WM_SETFONT,
                                  (WPARAM) ((lpMDIdi -> hFont == NULL) ?
                                              GetStockObject( SYSTEM_FONT ) :
                                              lpMDIdi -> hFont),
                                  (LPARAM) FALSE ) ;

         // create dialog children

         CreateMDIDialogChildren( hDlg, lpMDIdi -> lpDlgItems,
                                  lpMDIdi -> dtItemCount,
                                  lpMDIdi -> hFont,
                                  lpMDIdi -> wUnitsX,
                                  lpMDIdi -> wUnitsY ) ;

         // send WM_INITDIALOG notification

         lpMDIdi -> lpDialogProc( hDlg, WM_INITDIALOG, NULL,
                                  lpMDIdi -> lParam ) ;

         return ( 0 ) ;
      }

      case WM_MDIACTIVATE:
      {
         HMENU  hMenu = NULL;
         HMENU  hWindowMenu = NULL;
         HWND   hFrameWnd ;

         LPMDIDIALOGINFO  lpDlgIda = (LPMDIDIALOGINFO)GetWindowLong( hDlg, DLGWINDOWEXTRA );

         // inform frame window about focus change
         EqfActivateInstance( hDlg, wParam );

         // Get the MDI dialog's menu information
         if ( lpDlgIda != NULL )
         {
           int              nPos ;

           hMenu = lpDlgIda->hMenu;
           if ((nPos = GetMenuItemCount( hMenu ) - 2) < 0)
             nPos = 0 ;
           hWindowMenu = GetSubMenu( hMenu, nPos );
//old:           hMenu = (HMENU) GetWindowWord( hDlg, DLGWINDOWEXTRA + GWW_MDIDIALOGHMENU ) ;
//old:          hWindowMenu = (HMENU) GetWindowWord( hDlg, DLGWINDOWEXTRA + GWW_MDIDIALOGHWNDMENU ) ;
         }

         // Get the MDI frame window handle...
         hFrameWnd = (HWND)UtlQueryULong( QL_TWBFRAME );

         if ( hDlg == (HWND)lParam )
         {
            PostMessage( hFrameWnd, WM_COMMAND, IDM_MENUCHANGE,
                         MAKELONG( hMenu, hWindowMenu ) ) ;
         }
         if (FALSE == wParam)
            PostMessage( hFrameWnd, WM_COMMAND, IDM_MENUCHANGE, 0L ) ;

         /*************************************************************/
         /* Notify dialog of activation/deactivation                  */
         /*************************************************************/
         if ( lpDlgIda != NULL )
         {
//           FARPROC  lpDialogProc ;
           WNDPROC  lpDialogProc ;

//old:           lpDialogProc = (FARPROC) GetWindowLong( hDlg, DLGWINDOWEXTRA +
//old:                                                         GWL_MDIDIALOGPROC ) ;
           lpDialogProc = lpDlgIda->lpDialogProc;
           if (NULL != lpDialogProc)
           {
             WPARAM wp;

             wp = (FALSE == wParam) ? WA_INACTIVE : WA_ACTIVE;
             lpDialogProc( hDlg, WM_ACTIVATE, wp, 0L );
           } /* endif */
         }

         return 0 ;
      }

      case STM_SETICON:
      {
         HICON  hOldIcon ;

         hOldIcon = (HICON) GET_PROP( hDlg, ATOM_HICON ) ;
         SET_PROP( hDlg, ATOM_HICON, (HANDLE)wParam ) ;
         if (IsIconic( hDlg ))
            UpdateWindow( hDlg ) ;
         return ( (LRESULT) MAKELONG( hOldIcon, 0 ) ) ;
      }

      case STM_GETICON:
         return( (LRESULT) MAKELONG( GET_PROP( hDlg, ATOM_HICON ), 0 ) ) ;

      case WM_QUERYDRAGICON:
        {
          LPMDIDIALOGINFO  lpDlgIda = (LPMDIDIALOGINFO)GetWindowLong( hDlg, DLGWINDOWEXTRA );
          HICON hIcon = lpDlgIda->hIcon;
//old:          HICON hIcon = (HICON)GetWindowWord( hDlg,
//old:                                       DLGWINDOWEXTRA + GWW_MDIDIALOGHICON );
          return( (MRESULT)hIcon );
        }
        break;

    case WM_PAINT:
      {
        PAINTSTRUCT ps;
        LPMDIDIALOGINFO  lpDlgIda = (LPMDIDIALOGINFO)GetWindowLong( hDlg, DLGWINDOWEXTRA );

        BeginPaint( hDlg, &ps );
        if ( (lpDlgIda != NULL) && IsIconic( hDlg ) )
        {
          HICON hIcon = lpDlgIda->hIcon;
//old:          HICON hIcon = (HICON)GetWindowWord( hDlg,
//old:                                       DLGWINDOWEXTRA + GWW_MDIDIALOGHICON );
          SendMessage( hDlg, WM_ICONERASEBKGND, (WPARAM)ps.hdc, 0L );
          DrawIcon( ps.hdc, 0, 0, hIcon );
        } /* endif */
        EndPaint( hDlg, &ps );
      }
      break;

    case WM_ERASEBKGND:
      if ( IsIconic(hDlg) )
        return TRUE;
      else
        break;

    case WM_ICONERASEBKGND:
      {
        POINT ptTemp;
        POINT pt = { 0, 0 };
        LONG lResult;
        HWND hwndClient = (HWND)UtlQueryULong( QL_TWBCLIENT );

        // adjust the hdc for the MDI client window and
        // have the MDI client erase the icon background
        // with the background bitmap or color
        ClientToScreen( hDlg, &pt );
        ScreenToClient( hwndClient, &pt );
        OffsetViewportOrgEx( (HDC)wParam, -pt.x, -pt.y, &ptTemp );
        OffsetClipRgn( (HDC)wParam, pt.x, pt.y );

        // note reverse polarity in return values from WM_ERASEBKGND
        // and WM_ICONERASEBKGND
        lResult = !SendMessage( hwndClient, WM_ERASEBKGND, wParam, lParam );

        // restore viewport extents and clipping offset
        OffsetViewportOrgEx( (HDC)wParam, pt.x, pt.y, &ptTemp );
        OffsetClipRgn( (HDC)wParam, pt.x, pt.y );

        return lResult;
      }
      break;

      case WM_CLOSE:
      {
         LPMDIDIALOGINFO  lpDlgIda = (LPMDIDIALOGINFO)GetWindowLong( hDlg, DLGWINDOWEXTRA );
//         FARPROC  lpDialogProc ;
         WNDPROC  lpDialogProc ;

//old:         lpDialogProc = (FARPROC) GetWindowLong( hDlg, DLGWINDOWEXTRA +
//old:                                                       GWL_MDIDIALOGPROC ) ;
         if ( lpDlgIda != NULL )
         {
           lpDialogProc = lpDlgIda->lpDialogProc;
         }
         else
         {
           lpDialogProc = NULL;
         } /* endif */

         SendMessage( GetParent( hDlg ), WM_MDIRESTORE, (WPARAM) hDlg, 0L ) ;

         if (NULL != lpDialogProc)
            lpDialogProc( hDlg, WM_CLOSE, (WPARAM)0, (LPARAM) NULL ) ;
            return ( 0L ) ;
      }

      case WM_SYSCOMMAND:
         // 'eat' the maximize syscommand as the MFC performs GPF when maximizing
         // a non-MFC MDI child window ...
         if ( (wParam & 0xFFF0) == SC_MAXIMIZE )
         {
           return ( 1L );
         }
         else
         {
           return ( DEFMDICHILDPROC( hDlg, uMsg, wParam, lParam ) ) ;
         } /* endif */
         break;
      case WM_MENUCHAR:
      case WM_CHILDACTIVATE:
         return ( DEFMDICHILDPROC( hDlg, uMsg, wParam, lParam ) ) ;
      case WM_SETTEXT:
         DEFWINDOWPROC( hDlg, uMsg, wParam, lParam );
         return ( DEFMDICHILDPROC( hDlg, uMsg, wParam, lParam ) ) ;

      case WM_GETMINMAXINFO:
      case WM_SIZE:
      case WM_MOVE:
      {
//         FARPROC  lpDialogProc ;
         WNDPROC  lpDialogProc ;
         LRESULT  lResult ;
         LPMDIDIALOGINFO  lpDlgIda = (LPMDIDIALOGINFO)GetWindowLong( hDlg, DLGWINDOWEXTRA );

         lResult = DEFMDICHILDPROC( hDlg, uMsg, wParam, lParam ) ;
//old:         lpDialogProc = (FARPROC) GetWindowLong( hDlg, DLGWINDOWEXTRA +
//old:                                                       GWL_MDIDIALOGPROC ) ;
         if ( lpDlgIda != NULL )
         {
           lpDialogProc = lpDlgIda->lpDialogProc;
           if (lpDialogProc != NULL)
              lpDialogProc( hDlg, uMsg, wParam, lParam ) ;
         }
         return ( lResult ) ;
      }

      case WM_NCDESTROY:
      {
         LPMDIDIALOGINFO  lpDlgIda = (LPMDIDIALOGINFO)GetWindowLong( hDlg, DLGWINDOWEXTRA );
//         FARPROC  lpDialogProc ;
         WNDPROC  lpDialogProc ;
         HFONT    hFont ;
         HMENU    hMenu ;
         HWND     hFrameWnd ;

         // Get the MDI dialog's menu information
         if ( lpDlgIda != NULL )
         {
           hMenu = lpDlgIda->hMenu;
         }
         else
         {
           hMenu = NULL;
         } /* endif */

//old:            (HMENU) GetWindowWord( hDlg, DLGWINDOWEXTRA +
//old:                                         GWW_MDIDIALOGHMENU ) ;

         // Get the MDI frame window handle...
         hFrameWnd = (HWND)UtlQueryULong( QL_TWBFRAME );

         if ( lpDlgIda != NULL )
         {
           lpDialogProc = lpDlgIda->lpDialogProc;
         }
         else
         {
           lpDialogProc = NULL;
         } /* endif */
//old:         lpDialogProc = (FARPROC) GetWindowLong( hDlg, DLGWINDOWEXTRA +
//old:                                                       GWL_MDIDIALOGPROC ) ;
         if (NULL != lpDialogProc)
            lpDialogProc( hDlg, uMsg, wParam, lParam ) ;
         hFont =
           lpDlgIda->hFont;
//old:            (HFONT) GetWindowWord( hDlg, DLGWINDOWEXTRA + GWW_MDIDIALOGHFONT ) ;
         if (hFont != NULL)
            DeleteObject( hFont ) ;

         // delete the MDI dialog's menu

         if (NULL != hMenu)
         {
            HWND  hFrameWnd ;

            // HACK!  Need to reset menu information before
            // destroying the child window.

            // Get the MDI frame window handle...

            // NOTE: An assumption is made that the parent of the MDI
            // dialog is the MDI client which has the MDI frame as its
            // parent.

            hFrameWnd = GetParent( GetParent( hDlg ) ) ;

            // Send a notification to _FORCE_ the frame window's
            // not be the current child's menu.

            SendMessage( hFrameWnd, WM_COMMAND, IDM_MENUCHANGE, 0L ) ;
            DestroyMenu( hMenu ) ;
         }

         // If property for dialog icon was added, remove it.

         if (NULL != GET_PROP( hDlg, ATOM_HICON ))
            REMOVE_PROP( hDlg, ATOM_HICON ) ;

         // finally: destroy our IDA are
         free( lpDlgIda );

         return ( DefDlgProc( hDlg, uMsg, wParam, lParam ) ) ;
      }

      case WM_KEYDOWN:
      case WM_CHAR:
      {
         BOOL     fResult = FALSE ;
         LPMDIDIALOGINFO  lpDlgIda = (LPMDIDIALOGINFO)GetWindowLong( hDlg, DLGWINDOWEXTRA );
//         FARPROC  lpDialogProc ;
         WNDPROC  lpDialogProc ;

//old:         lpDialogProc = (FARPROC) GetWindowLong( hDlg, DLGWINDOWEXTRA +
//old:                                                       GWL_MDIDIALOGPROC ) ;
         if ( lpDlgIda != NULL )
         {
           lpDialogProc = lpDlgIda->lpDialogProc;
         }
         else
         {
           lpDialogProc = NULL;
         } /* endif */
         if (lpDialogProc != NULL)
         {
            fResult = lpDialogProc( hDlg, uMsg, wParam, lParam ) ;
            if (!fResult)
               return ( DefDlgProc( hDlg, uMsg, wParam, lParam ) ) ;
            else
               return ( (LRESULT) fResult ) ;
         }
         else
            return( DefDlgProc( hDlg, uMsg, wParam, lParam ) ) ;
      }
      break;

    case WM_GETFONT:
      {
         LPMDIDIALOGINFO  lpDlgIda = (LPMDIDIALOGINFO)GetWindowLong( hDlg, DLGWINDOWEXTRA );
         HFONT hFont = NULL;
         if ( lpDlgIda != NULL )
         {
           hFont = lpDlgIda->hFont;
         } /* endif */
//old:   hFont = (HFONT) GetWindowWord( hDlg, DLGWINDOWEXTRA + GWW_MDIDIALOGHFONT ) ;
        return (MRESULT)hFont;
      }
      break;

      default:
      {
         BOOL     fResult = FALSE ;
         LPMDIDIALOGINFO  lpDlgIda = (LPMDIDIALOGINFO)GetWindowLong( hDlg, DLGWINDOWEXTRA );
//         FARPROC  lpDialogProc ;
         WNDPROC  lpDialogProc ;

//old:         lpDialogProc = (FARPROC) GetWindowLong( hDlg, DLGWINDOWEXTRA +
//old:                                                       GWL_MDIDIALOGPROC ) ;
         if ( lpDlgIda != NULL )
         {
           lpDialogProc = lpDlgIda->lpDialogProc;
         }
         else
         {
           lpDialogProc = NULL;
         } /* endif */
         if (lpDialogProc != NULL)
         {
            fResult = lpDialogProc( hDlg, uMsg, wParam, lParam ) ;
            if (!fResult)
               return ( DefDlgProc( hDlg, uMsg, wParam, lParam ) ) ;
            else
               return ( (LRESULT) fResult ) ;
         }
         else
            return( DefDlgProc( hDlg, uMsg, wParam, lParam ) ) ;
      }
   }
   return ( DEFMDICHILDPROC( hDlg, uMsg, wParam, lParam ) ) ;

} // end of MDIDialogProc()
