//+----------------------------------------------------------------------------+
//|  EQFLOGO.C - Product Info Panels                                           |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:            G. Queck (QSoft)                                         |
//+----------------------------------------------------------------------------+
//|Description:       Product Info panels                                      |
//+----------------------------------------------------------------------------+
//|Entry Points:      main                   Main program of MAT               |
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
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
// $Revision: 1.4 $ ----------- 27 Oct 2004
// GQ: - fixed P020744: TQM doens't show Product Information and returns error
//       by using the hResMod handle instead of loading the resource DLLs of TM when displaying the logo
// 
// 
// $Revision: 1.3 $ ----------- 26 Oct 2004
// GQ: - TQM: do not load resource for logo display
// 
// 
// $Revision: 1.2 $ ----------- 7 Sep 2004
// --RJ: accessibility: support display of logo in high contrast mode
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
// $Revision: 1.3 $ ----------- 26 Feb 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
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
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFLOGO.CV_   1.10   16 Dec 1998 16:21:42   BUILD  $
 *
 * $Log:   K:\DATA\EQFLOGO.CV_  $
 *
 *    Rev 1.10   16 Dec 1998 16:21:42   BUILD
 * add support for small screen resolution
 *
 *    Rev 1.9   16 Dec 1998 14:10:30   BUILD
 * center logos logo
 *
 *    Rev 1.8   07 Dec 1998 11:39:24   BUILD
 * rework for logos splash: Title not displayed
 *
 *    Rev 1.7   15 Oct 1998 19:53:42   BUILD
 * - support LOGOS bitmap
 *
 *    Rev 1.6   28 Jan 1998 11:05:00   BUILD
 * - fixed problem "logo corrupted on low-res screens"
 *
 *    Rev 1.5   26 Jan 1998 11:07:40   BUILD
 * - use correct background color for text string in Win32 environment
 *
 *    Rev 1.4   14 Jan 1998 15:48:28   BUILD
 * - migrated to Win32bit
 *
 *    Rev 1.3   18 Mar 1996 16:38:00   BUILD
 * - fixed PTM KWT0183: reshape/resize logo to look a little bit more fancy
 *
 *    Rev 1.1   24 Jan 1996 09:58:10   BUILD
 * - corrected handling of resource module handle
 *
 *    Rev 1.0   09 Jan 1996 09:08:50   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
#include <eqf.h>             // General Translation Manager include file
#include <eqfstart.h>
#include <eqfstart.id>

#include <time.h>                      // C library functions: time

typedef struct _LOGODLGDATA
{
  LONG   LogoDisplayTime;              // logo display time
  USHORT usType;                       // type of logo (EQFSTART, EQFITM, ...)
  UCHAR  ucRelease;                    // intermediate release
} LOGODLGDATA, *PLOGODLGDATA;

static HMODULE  hmodBitmaps;           // resource module containing bitmaps
static HMODULE  hLogoResMod;           // resource module with dlgs

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TwbShowLogo            Show the MAT logo dialog          |
//+----------------------------------------------------------------------------+
//|Function call:     TwbShowLogo( LONG lLogoDisplayTime, HWND hwnd );         |
//+----------------------------------------------------------------------------+
//|Description:       Starts the display of the MAT logo dialog window         |
//+----------------------------------------------------------------------------+
//|Input parameter:   LONG   LogoDisplayTime    logo display time              |
//|                   HWND   hwnd               parent window handle           |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     load resource module                                     |
//|                   show MAT logo dialog by calling WinDlgBox                |
//|                   free resource module                                     |
//+----------------------------------------------------------------------------+
VOID TwbShowLogo
(
  LONG   LogoDisplayTime,              // logo display time
  HWND   hwnd,                         // parent window handle
  PSZ    pszEqfResFile,                // pointer to resource file name
  USHORT usType,                       // type of logo (EQFSTART, EQFITM, ...)
  UCHAR  ucRelease                     // intermediate release
)
{
   INT_PTR   sRC = 0;
   LOGODLGDATA  LogoDlgData;

   // load generic resource for windows ...
#ifdef _TQM
   hLogoResMod = hResMod;
   hmodBitmaps = hResMod;
#else
   DosLoadModule( NULL, NULLHANDLE, pszEqfResFile, &hLogoResMod );
   DosLoadModule( NULL, NULLHANDLE, EQFLOGOR_DLL, &hmodBitmaps );
#endif

   /*******************************************************************/
   /* provide data in structure to be passed on to dialog             */
   /*******************************************************************/
   LogoDlgData.LogoDisplayTime = LogoDisplayTime;
   LogoDlgData.usType = usType;
   LogoDlgData.ucRelease = ucRelease;

   DIALOGBOX( hwnd, TwbLogoDlgBox, hLogoResMod, ID_LOGO_DLG, &LogoDlgData, sRC );

#ifndef _TQM
   DosFreeModule ( hmodBitmaps );
   DosFreeModule ( hLogoResMod );
#endif

} /* end of function TwbShowLogo */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TwbLogoDlgBox          MAT logo dialog procedure         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function call:     TwbLogoDlgBox( HWND hwnd, USHORT msg, MPARAM mp1,        |
//|                                  MPARAM mp2 )                              |
//+----------------------------------------------------------------------------+
//|Description:       Window procedure for the MAT logo dialog window.         |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwnd     handle of window                         |
//|                   USHORT msg      type of message                          |
//|                   MPARAM mp1      first message parameter                  |
//|                   MPARAM mp2      second message parameter                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       depends on message type                                  |
//|                   normal return codes are:                                 |
//|                   TRUE  = message has been processed                       |
//|                   FALSE = message has not been processed                   |
//+----------------------------------------------------------------------------+
//|Function flow:     switch message                                           |
//|                     case WM_INITDLG:                                       |
//|                       allocate and anchor dialog IDA                       |
//|                       load logo bitmap                                     |
//|                       set start time an current developer bitmap           |
//|                       start timer                                          |
//|                     case WM_COMMAND:                                       |
//|                       switch command ID                                    |
//|                         case DID_CANCEL:                                   |
//|                         case OK button:                                    |
//|                           post WM_CLOSE to end the dialog                  |
//|                       endswitch                                            |
//|                     case WM_TIMER:                                         |
//|                       if timer period is the SPECIAL_TIME value then       |
//|                         increment bitmap number                            |
//|                         draw current developer bitmap                      |
//|                       else                                                 |
//|                         post WM_CLOSE to end the dialog                    |
//|                       endif                                                |
//|                     case WM_CLOSE:                                         |
//|                       stop timer                                           |
//|                       dismiss the dialog                                   |
//|                     case WM_DESTROY:                                       |
//|                       free dialog IDA                                      |
//|                     case WM_PAINT:                                         |
//|                       let default dialog procedure paint the dialog        |
//|                       compute inner window rectangle                       |
//|                       draw IBM logo                                        |
//|                       draw current developer bitmap                        |
//|                     default:                                               |
//|                       pass message to default dialog procedure             |
//|                   endswitch                                                |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK TwbLogoDlgBox
(
  HWND hwnd,
  WINMSG message,
  WPARAM mp1,
  LPARAM mp2
)
{
  HPS         hps;                     // Handle to Presentation Space
  RECTL       rect;                    // Rectangle
  RECTL       rectDraw;                // Rectangle for draw operations
  BOOL        fOK;                     // internal OK flag
  PLOGOIDA    pIda;                    // pointer to logo window IDA
  LONG        lWidth = 0L;             // width of physical screen
  LONG        lHeight = 0L;            // height of physical screen
  MRESULT     mResult = FALSE;         // result of message processing
  BITMAP             Bitmap;           // info of currently processed bitmap
  PAINTSTRUCT ps;                        // pointer to paint struct
  POINTL      pt;                      // presentation space coordinates
  USHORT      usI;                     // loop index
  LONG        cxChar, cyChar;          // average character width and height
  LONG        cxDlgBorder, cyDlgBorder;// width and height of dialog border
  LONG        cxDlg, cyDlg;            // width and height of dialog window
  PLOGODLGDATA pLogoDlgData;           // pointer to passed data
  BOOL        fLogos;

  cxDlg = 0L;
  cyDlg = 0L;
  cyChar = 0L;
  cxChar = 0L;
  memset(&rect, 0, sizeof(rect));

  switch (message)
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_LOGO_DLG, mp2 ); break;

    case WM_INITDLG :
      /****************************************************************/
      /* get pointer to passed data                                   */
      /****************************************************************/
      pLogoDlgData = (PLOGODLGDATA) mp2;
      /****************************************************************/
      /* Allocate IDA                                                 */
      /****************************************************************/
      fOK = UtlAlloc( (PVOID *) &pIda, 0L, (LONG)sizeof(LOGOIDA), ERROR_STORAGE );
      ANCHORDLGIDA( hwnd, pIda );

      /****************************************************************/
      /* Load logo strings                                            */
      /****************************************************************/
      {
            LOADSTRING( NULLHANDLE, hLogoResMod, SID_LOGO_TITLELINE, pIda->szTitle );
            LOADSTRING( NULLHANDLE, hLogoResMod, SID_LOGO_COPYRIGHT1, pIda->szCopyRight[0] );
            LOADSTRING( NULLHANDLE, hLogoResMod, SID_LOGO_COPYRIGHT2, pIda->szCopyRight[1] );
            LOADSTRING( NULLHANDLE, hLogoResMod, SID_LOGO_COPYRIGHT3, pIda->szCopyRight[2] );
            LOADSTRING( NULLHANDLE, hLogoResMod, SID_LOGO_COPYRIGHT4, pIda->szCopyRight[3] );
            LOADSTRING( NULLHANDLE, hLogoResMod, SID_LOGO_COPYRIGHT5, pIda->szCopyRight[4] );
            LOADSTRING( NULLHANDLE, hLogoResMod, SID_LOGO_REVISION, pIda->szRevision );
            fLogos = FALSE;
       }

      /****************************************************************/
      /* add subsequent revision level if any passed...               */
      /****************************************************************/
      if (pLogoDlgData->ucRelease )
      {
        UCHAR ucData[2];
        ucData[0] = pLogoDlgData->ucRelease;
        ucData[1] = EOS;
        strcat(pIda->szRevision, (PSZ)ucData );
      } /* endif */

      /****************************************************************/
      /* Load logo bitmap                                             */
      /****************************************************************/
      if ( fOK )
      {
        hps = GETPS( hwnd );

        lWidth = WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);
        lHeight = WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);

        /* check which logo bitmap can be used (or none)                   */
        pIda->hbmLogo = NULLHANDLE;
        if (lWidth < 1024L)
        {
          /***********************************************************/
          /* load bitmap for VGA displays                            */
          /***********************************************************/
          if ( fLogos )
          {
             pIda->hbmLogo = LoadBitmap( hmodBitmaps,
                   MAKEINTRESOURCE(ID_EQF_BMP_LOGOSLOGO ) );
             pIda->hbmEqfLogo = LoadBitmap( hmodBitmaps,
                   MAKEINTRESOURCE(ID_EQF_BMP_LOGOSLOGO1) );
          }
          else
          {
            pIda->hbmLogo = LoadBitmap( hmodBitmaps,
                                        MAKEINTRESOURCE(ID_EQF_BMP_IBMLOGO_SMALL ) );
            pIda->hbmEqfLogo = LoadBitmap( hmodBitmaps,
                                      MAKEINTRESOURCE(ID_EQF_BMP_EQFLOGO_SMALL) );
          } /* endif */
        }
        else
        {
          /***********************************************************/
          /* load bitmap for 8514/A, XGA and PS/55 displays          */
          /***********************************************************/
          {
            pIda->hbmLogo = LoadBitmap( hmodBitmaps,
                  MAKEINTRESOURCE(ID_EQF_BMP_IBMLOGO_LARGE ) );
            pIda->hbmEqfLogo = LoadBitmap( hmodBitmaps,
                  MAKEINTRESOURCE(ID_EQF_BMP_EQFLOGO_LARGE) );
           }
        } /* endif */
        RELEASEPS( hwnd, hps );
      } /* endif */

      /****************************************************************/
      /* Get text and bitmap dimensions                               */
      /****************************************************************/
      if ( fOK )
      {
        LONG cx, cy;

        pIda->cxChar = cxChar = UtlQueryULong( QL_AVECHARWIDTH );
        pIda->cyChar = cyChar = UtlQueryULong( QL_CHARHEIGHT );
        hps = GETPS(hwnd);
        GetObject( pIda->hbmLogo, sizeof(Bitmap), (LPSTR) &Bitmap);
        pIda->sizeLogoBmp.cx = Bitmap.bmWidth;
        pIda->sizeLogoBmp.cy = Bitmap.bmHeight;

        GetObject( pIda->hbmEqfLogo, sizeof(Bitmap), (LPSTR) &Bitmap);
        pIda->sizeEqfLogoBmp.cx = Bitmap.bmWidth;
        pIda->sizeEqfLogoBmp.cy = Bitmap.bmHeight;
        /**************************************************************/
        /* in case of LOGOS we don't have a second bitmap, so adjust  */
        /* only for the 2 chars...                                    */
        /**************************************************************/
        if ( fLogos )
        {
          pIda->sizeEqfLogoBmp.cx = 2 * cxChar;
          pIda->sizeEqfLogoBmp.cy = 9 * cyChar;
        } /* endif */

        pIda->usMaxWidth = 0;
        for ( usI = 0; usI < 5; usI++ )
        {
          RECTL_YBOTTOM(rect) = RECTL_XLEFT(rect) = 0;
          RECTL_YTOP(rect)   = lHeight;
          RECTL_XRIGHT(rect) = lWidth;
          TEXTSIZE( hps, pIda->szCopyRight[usI], cx, cy );
          pIda->usMaxWidth = max( pIda->usMaxWidth, (USHORT)cx );
        } /* endfor */
        TEXTSIZE( hps, pIda->szRevision, cx, cy );
        pIda->usMaxWidth = max( pIda->usMaxWidth, (USHORT)cx );

        RECTL_YBOTTOM(rect) = 0;
        RECTL_YTOP(rect)    = cy;
        RECTL_XLEFT(rect)   = 0;
        RECTL_XRIGHT(rect)  = cx;
        pIda->cyChar = cyChar = cy; // correct line height value

        RELEASEPS( hwnd, hps );
      } /* endif */

      /****************************************************************/
      /* Setup drawing rectangles                                     */
      /****************************************************************/
      if ( fOK )
      {
        /**************************************************************/
        /* General dialog layout:                                     */
        /*     +--------------------------------------+               */
        /*     |     +-----------------------+        |               */
        /*     |     |                       |        |               */
        /*     |     |       LogoBmp         |        |               */
        /*     |     |                       |        |               */
        /*     |     +-----------------------+        |               */
        /*     |     ---------title line------        |               */
        /*     | +------------------+                 |               */
        /*     | |                  |    - copy -     |               */
        /*     | |                  |    - right-     |               */
        /*     | |                  |    --- 1 --     |               */
        /*     | |    EqfLogoBmp    |    --- - --     |               */
        /*     | |                  |    --- 5 --     |               */
        /*     | |                  |    +-------+    |               */
        /*     | |                  |    |  OK   |    |               */
        /*     | +------------------+    +-------+    |               */
        /*     +--------------------------------------+               */
        /**************************************************************/
        cxDlgBorder = WinQuerySysValue( HWND_DESKTOP, SV_CXDLGFRAME );
        cyDlgBorder = WinQuerySysValue( HWND_DESKTOP, SV_CYDLGFRAME );

        /**************************************************************/
        /* Compute width of dialog window                             */
        /*   which is the maximum of: logo width + space + border,    */
        /*                            title width + space + border,   */
        /*                            width of lower area + border    */
        /**************************************************************/
        cxDlg = (2L * cxDlgBorder) + (6L * cxChar) +
                pIda->sizeEqfLogoBmp.cx + 1L +
                pIda->usMaxWidth;
        cxDlg = max( cxDlg, ((RECTL_XRIGHT(rect) - RECTL_XLEFT(rect)) + (2 * cxChar) + (2 * cxDlgBorder)) );
        cxDlg = max( cxDlg, (pIda->sizeLogoBmp.cx + (2 * cxChar) + (2 * cxDlgBorder)) );

        /**************************************************************/
        /* Setup logo bitmap rectangle and                            */
        /* strech logo bitmap if bitmap is smaller than the title line*/
        /**************************************************************/
        if ( (RECTL_XRIGHT(rect) - RECTL_XLEFT(rect)) > pIda->sizeLogoBmp.cx )
        {
          pIda->sizeLogoBmp.cy = pIda->sizeLogoBmp.cy *
                                 (RECTL_XRIGHT(rect) - RECTL_XLEFT(rect)) /
                                 pIda->sizeLogoBmp.cx;
          pIda->sizeLogoBmp.cx = RECTL_XRIGHT(rect) - RECTL_XLEFT(rect);
        } /* endif */

        // no logo in OpenTM
        pIda->sizeLogoBmp.cy = pIda->sizeLogoBmp.cx = 0;

        RECTL_XLEFT(pIda->rectLogo)   = (cxDlg - pIda->sizeLogoBmp.cx) >> 1;
        RECTL_XRIGHT(pIda->rectLogo)  = RECTL_XLEFT(pIda->rectLogo) +
                                 pIda->sizeLogoBmp.cx;

        /**************************************************************/
        /* Setup title text rectangle                                 */
        /**************************************************************/
        RECTL_XLEFT(pIda->rectTitle)   = cxDlgBorder + cxChar;
        RECTL_XRIGHT(pIda->rectTitle)  = RECTL_XLEFT(pIda->rectTitle) + cxDlg -
                                  (2 * cxDlgBorder) - (2 * cxChar);

        /**************************************************************/
        /* Setup EQF Logo bitmap rectangle                            */
        /**************************************************************/
        RECTL_XLEFT(pIda->rectEqfLogo)   = cxDlgBorder + (2L * cxChar);
        RECTL_XRIGHT(pIda->rectEqfLogo)  = RECTL_XLEFT(pIda->rectEqfLogo) +
                                    pIda->sizeEqfLogoBmp.cx;

        /**************************************************************/
        /* Setup Copyright text rectangle (first line only)           */
        /**************************************************************/
        RECTL_XLEFT(pIda->rectCopyRight)   = RECTL_XRIGHT(pIda->rectEqfLogo) + 1L;
        if ( !fLogos )
        {
          RECTL_XRIGHT(pIda->rectCopyRight)  = RECTL_XLEFT(pIda->rectCopyRight) +
                                               pIda->usMaxWidth +( 4 * cxChar );
        }
        else
        {
          /************************************************************/
          /* center in middle of screen                               */
          /************************************************************/
          RECTL_XRIGHT(pIda->rectCopyRight)  = cxDlg - 4 * cxChar ;
        } /* endif */

        /**************************************************************/
        /* Set Y coordiantes of rectangles                            */
        /**************************************************************/
        RECTL_YTOP(pIda->rectLogo) = cyDlgBorder + cyChar +
                            WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR );
        RECTL_YBOTTOM(pIda->rectLogo) = RECTL_YTOP(pIda->rectLogo) +
                                 pIda->sizeLogoBmp.cy;
        if ( !fLogos )
        {
          RECTL_YTOP(pIda->rectTitle) = RECTL_YBOTTOM(pIda->rectLogo) + cyChar;
          RECTL_YBOTTOM(pIda->rectTitle) = RECTL_YTOP(pIda->rectTitle) + cyChar;

        }
        else
        {
          RECTL_YTOP(pIda->rectTitle) = RECTL_YBOTTOM(pIda->rectLogo);
          RECTL_YBOTTOM(pIda->rectTitle) = RECTL_YTOP(pIda->rectTitle);

        } /* endif */
        RECTL_YTOP(pIda->rectEqfLogo) = RECTL_YBOTTOM(pIda->rectTitle) + (2 * cyChar);
        RECTL_YBOTTOM(pIda->rectEqfLogo) = RECTL_YTOP(pIda->rectEqfLogo) +
                                    pIda->sizeEqfLogoBmp.cy;

        RECTL_YTOP(pIda->rectCopyRight) = RECTL_YTOP(pIda->rectEqfLogo);
        RECTL_YBOTTOM(pIda->rectCopyRight) = RECTL_YTOP(pIda->rectCopyRight) -
                                      cyChar;

        cyDlg = RECTL_YBOTTOM(pIda->rectEqfLogo) +
                cyChar +
                WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR ) +
                (2 * cyDlgBorder);
      } /* endif */

      if ( fOK )
      {
         pIda->usTime = (USHORT) pLogoDlgData->LogoDisplayTime;
         if ( pIda->usTime )
         {
           WinStartTimer( (HAB) UtlQueryULong( QL_HAB ), hwnd, 1, pIda->usTime );
         }
      } /* endif */

      /****************************************************************/
      /* end dialog in case of errors                                 */
      /****************************************************************/
      if ( fOK )
      {

        /**************************************************************/
        /* Center dialog within desktop                               */
        /**************************************************************/
        lWidth = WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);
        lHeight = WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);


        WinSetWindowPos( hwnd, (HWND)HWND_TOP,
                         (SHORT)((lWidth - cxDlg) >> 1),
                         (SHORT)((lHeight - cyDlg) >> 1),
                         (SHORT)cxDlg,
                         (SHORT)cyDlg,
                         (USHORT)(EQF_SWP_MOVE | EQF_SWP_SIZE) );

        WinPostMsg( hwnd, WM_EQF_INITIALIZE, MP1FROMSHORT( 0 ), MP2FROMSHORT( 0 ) );
      }
      else
      {
        WinDismissDlg( hwnd, FALSE );
      } /* endif */

      // NOTE: Under OS/2 we set the focus, therefore TRUE as return
      //       value is okay
      //       under Windows we let Win3.1 do it, therefore TRUE is
      //       also okay...
      mResult = MRFROMSHORT(TRUE);
      break;

    case  WM_EQF_INITIALIZE :
      pIda = ACCESSDLGIDA( hwnd, PLOGOIDA );

      /**************************************************************/
      /* Set pushbutton position                                    */
      /**************************************************************/
      {
        RECT rect;

        GetWindowRect( WinWindowFromID( hwnd, PID_PB_OK ), &rect );
        pIda->swpOkPB.cx = (SHORT)(rect.right - rect.left);
        pIda->swpOkPB.cy = (SHORT)(rect.bottom - rect.top);
        pIda->swpOkPB.x  = ((SHORT)(RECTL_XRIGHT(pIda->rectCopyRight) -
                           (SHORT)RECTL_XLEFT(pIda->rectCopyRight)  -
                           pIda->swpOkPB.cx) >> 1) +
                           (SHORT)RECTL_XRIGHT(pIda->rectEqfLogo);
        pIda->swpOkPB.y  = (SHORT)RECTL_YBOTTOM(pIda->rectEqfLogo)- pIda->swpOkPB.cy;

        MoveWindow( WinWindowFromID( hwnd, PID_PB_OK ),
                    pIda->swpOkPB.x, pIda->swpOkPB.y,
                    pIda->swpOkPB.cx, pIda->swpOkPB.cy,
                    TRUE );
        UpdateWindow( WinWindowFromID( hwnd, PID_PB_OK ) );
      }
      break;

    case  WM_COMMAND :
      switch (WMCOMMANDID( mp1, mp2 ))
      {
        case  DID_CANCEL:
        case  PID_PB_OK :
          WinPostMsg( hwnd, WM_CLOSE, 0L, 0L );
          mResult = MRFROMSHORT(TRUE);
          break;
      } /* endswitch */
      break;

    case  WM_TIMER :
      WinPostMsg( hwnd, WM_CLOSE, 0L, 0L );
      break;

    case  WM_CLOSE :
      WinStopTimer( WinQueryAnchorBlock(hwnd), hwnd, 1 );
      WinDismissDlg(hwnd, FALSE);
      break;

    case  WM_PAINT :
      {
      /****************************************************************/
      /* Access IDA                                                   */
      /****************************************************************/
      DWORD  dwRGB_WINDOW = GetSysColor(COLOR_WINDOW);
      DWORD  dwRGB_WINDOWTEXT = GetSysColor(COLOR_WINDOWTEXT);
      pIda = ACCESSDLGIDA( hwnd, PLOGOIDA );

      /****************************************************************/
      /* Let default procedure paint the dialog window                */
      /****************************************************************/
      WinDefDlgProc(hwnd, message, mp1, mp2);

      /****************************************************************/
      /* Get dialog window presentation space                         */
      /****************************************************************/
      hps = BeginPaint(hwnd, &ps );

      /****************************************************************/
      /* Draw IBM logo                                                */
      /****************************************************************/
      //WinDrawBitmap( hps, pIda->hbmLogo, NULL, (PPOINTL)&(pIda->rectLogo),
      //               CLR_PALEGRAY,
      //               GetSysColor(COLOR_BTNFACE),
      //               DBM_STRETCH | DBM_IMAGEATTRS);

      /****************************************************************/
      /* Draw EQF logo                                                */
      /****************************************************************/
      pt.x = RECTL_XLEFT(pIda->rectEqfLogo);
      pt.y = RECTL_YTOP(pIda->rectEqfLogo);
      pt.x = RECTL_XLEFT(pIda->rectEqfLogo);
      WinDrawBitmap( hps, pIda->hbmEqfLogo, NULL, &pt,
                     CLR_PALEGRAY, CLR_BACKGROUND,
                     DBM_IMAGEATTRS );

      // under Win32 we use the BTNFACE color as background as
      // this color is also used for the dialog background
         SetBkColor( hps, GetSysColor(COLOR_BTNFACE) );
      /****************************************************************/
      /* Draw title line                                              */
      /****************************************************************/
      {
				WinDrawText( hps, -1, pIda->szTitle, &pIda->rectTitle, dwRGB_WINDOWTEXT, dwRGB_WINDOW, DT_CENTER );
      } /* end block */


      /****************************************************************/
      /* Draw copyright lines                                         */
      /****************************************************************/
      memcpy( &rectDraw, &pIda->rectCopyRight, sizeof(RECTL) );
      if (!UtlIsHighContrast())
      {
		  for ( usI = 0; usI < 5; usI++ )
		  {
			WinDrawText( hps, -1, pIda->szCopyRight[usI],
						 &rectDraw,
						  0, 0, DT_CENTER );
			RECTL_YBOTTOM(rectDraw) += (pIda->cyChar * 3 / 2);
			RECTL_YTOP(rectDraw)    = RECTL_YBOTTOM(rectDraw) - pIda->cyChar;
		  } /* endfor */
		  WinDrawText( hps, -1, pIda->szRevision,
					   &rectDraw,
						0, 0, DT_CENTER );
      }
      else
      {
          for ( usI = 0; usI < 5; usI++ )
		  {
			WinDrawText( hps, -1, pIda->szCopyRight[usI],
						 &rectDraw,
						  dwRGB_WINDOWTEXT, dwRGB_WINDOW, DT_CENTER );
			RECTL_YBOTTOM(rectDraw) += (pIda->cyChar * 3 / 2);
			RECTL_YTOP(rectDraw)    = RECTL_YBOTTOM(rectDraw) - pIda->cyChar;
		  } /* endfor */
		  WinDrawText( hps, -1, pIda->szRevision,
					   &rectDraw,
						dwRGB_WINDOWTEXT, dwRGB_WINDOW, DT_CENTER );
      }
      /****************************************************************/
      /* Release presentation space                                   */
      /****************************************************************/
      EndPaint(hwnd, &ps);
      }
      break;

   case WM_DESTROY:
      pIda = ACCESSDLGIDA( hwnd, PLOGOIDA );
      if ( pIda )
      {
        if ( pIda->hbmLogo )
        {
            DeleteObject( pIda->hbmLogo );
        } /* endif */
        if ( pIda->hbmEqfLogo )
        {
            DeleteObject( pIda->hbmEqfLogo );
        } /* endif */
        UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
      } /* endif */
      break;

    default  :
      mResult = WinDefDlgProc(hwnd, message, mp1, mp2 );
  } /* endswitch */

  return ( mResult );
} /* end of function TwbLogoDlgBox */
