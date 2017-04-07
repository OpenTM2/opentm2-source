//+----------------------------------------------------------------------------+
//|EQFDDISP.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2017, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:              G. Queck (QSoft)                                       |
//+----------------------------------------------------------------------------+
//|Description:         Dialog procedure to display the found term             |
//|                                                                            |
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
#define INCL_EQF_FILT             // dictionary filter functions
#define INCL_EQF_PRINT            // print functions
#include <eqf.h>                  // General Translation Manager include file

#include <time.h>
#include "EQFDASDI.H"             // internal ASD services header file
#include "OtmDictionaryIF.H"
#include "eqfddisp.h"             // internal header file of display dialog
#include "eqfddlg.id"             // dialog IDs
#include "eqfutmdi.h"           // MDI utilities

#define DISP_NAME_SIZE 15


MRESULT EXPENTRY SpinSubclassProc( HWND, WINMSG, WPARAM, LPARAM );
BOOL ReplaceWithUnicodeField
( 
  HWND hwndDlg,                        // window handle of dialog window
  int  iEntryFieldID                   // ID of entry field
);

static FARPROC pfnOrgSpinProc;         // original spinbutton procedure

#define PLACE_FOR_PUBOS 30
        // space for pushbuttons at bottom of dialog window
        // used in WM_ADJUSTWINDOWPOS

/*******************************************************************************
*
*       function:       DISPENTRYDLGPROC
*
*******************************************************************************/

INT_PTR CALLBACK DISPENTRYDLGPROC( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
  PDISPIDA pIda;  // data from lookup thread and static information
                            // for this dialog
  USHORT      usControlID;             // used in WM_CONTROL and WM_COMMAND
  HWND        hwndMLE;                 // handle of the MLE containing the entry
  HPS         hps;                     // used to query character dimensions
  BOOL        fOK;                     // internal OK flag
  PRECTL      prectl;                  // pointer for rectangle processing
  LONG        cxAvail;                 // available x size of dialog
  USHORT      usRC;                    // WinDlgBox return code
  PFOUNDINFO  pDictInfo;               // ptr to info of selected dictionary
  USHORT      usI;                     // loop index
  PUCB        pUCB;                    // pointer to user control block
  SWP         swp;                     // buffer for current window positions
                                       // variables for pushbutton
  ULONG       ulGap;                   //   positioning
  ULONG       ulTotGaps;
  ULONG       ulXPos;
  ULONG       ulTotSize;
  ULONG       ulCorrect;
  CHAR        szPanelNumber[10];       // buffer for panel numbers
  PSZ         pszPanelNumber;          // points to panel number
  PSZ         pszPanelText[4];         // panel text pointers

  switch (msg)
  {
     case WM_EQF_QUERYID : HANDLEQUERYID( DISP_ENTRY_DLG, mp2 ); break;

     case WM_INITDLG:
        fOK = TRUE;
        pIda = (PDISPIDA) mp2;
        pIda->hwnd = hwnd;
        ANCHORDLGIDA( hwnd, pIda );
        pIda->fsStatus = 0;

         if ( fOK )
         {
            PSZ_W pszTemp;
            ULONG ulMsgLen;
			HMODULE hResMod;
			hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            UtlLoadStringW( (HAB)UtlQueryULong( QL_HAB ),
                           hResMod, SID_DISP_ENTRY_TITLE, pIda->szTemp,
                           pIda->szUniBuffer,  sizeof(pIda->szUniBuffer)/ sizeof(CHAR_W) );

            pszTemp = pIda->szUniBuffer + UTF16strlenCHAR(pIda->szUniBuffer);

            /**********************************************************/
            /* check if term contains special encoding - if so don't  */
            /* display it at the titlebar                             */
            /* titlebar example "Lookup Entry: high level design"     */
            /**********************************************************/
            {
              BOOL  fShiftOut = FALSE;
              PCHAR_W p = pIda->ucTerm;
              CHAR_W  c;
              while ( ((c = *p++)!= NULC) && !fShiftOut )
              {
                if ( (c == SO) && ( *p != SO ) )
                {
                  fShiftOut = TRUE;
                  break;
                } /* endif */
              } /* endwhile */
              if ( !fShiftOut )
              {
                UTF16strcat( pIda->szUniBuffer, pIda->ucTerm);
              } /* endif */
            }

            // convert term to ANSI
        //    if ( GetCharSet() != THAI_CHARSET )
        //    {
        //      OEMTOANSI( pszTemp );
        //    }
            SETTEXTHWNDW( hwnd, pIda->szUniBuffer );

            ReplaceWithUnicodeField( hwnd, DISP_ENTRY_MLE_ENTRY );

            hwndMLE = WinWindowFromID (hwnd, DISP_ENTRY_MLE_ENTRY);

            pUCB = (PUCB)pIda->pLUPCB->hUCB;

            SetCtrlFnt (hwnd, GetCharSet(), DISP_ENTRY_MLE_ENTRY, 0 );

            WinPostMsg( pIda->pLUPCB->hwndParent,
                        pIda->pLUPCB->usNotifyMsg,
                        MP1FROMHWND(hwnd),
                        MP2FROM2SHORT( DLG_SHOWN, DISP_ENTRY_DLG) );

            SetFixedPitchMLEFont( hwndMLE, pUCB->fDBCS | GetCharSet(), &pIda->usMLECharWidth );

            DicDspRefreshDictSpin( hwnd, pIda );

            /**********************************************************/
            /* load panel strings                                     */
            /**********************************************************/
            LOADSTRING( (HAB)UtlQueryULong( QL_HAB ),
                        hResMod, SID_DISP_PANEL_TEXT,
                        pIda->szTemp );
            for ( usI = 0; usI < 3; usI++ )
            {
              itoa( usI + 1, szPanelNumber, 10 );
              pszPanelNumber = szPanelNumber;
              DosInsMessage( &pszPanelNumber, 1, pIda->szTemp,
                           strlen( pIda->szTemp ) + 1,
                           pIda->szPanelText[usI],
                           sizeof(pIda->szPanelText[usI]),
                           &ulMsgLen );
              pIda->szPanelText[usI][ulMsgLen] = EOS;
              pszPanelText[usI] = pIda->szPanelText[usI];
            } /* endfor */
            pszPanelText[3] = NULL;

            /**********************************************************/
            /* fill panel spin button                                 */
            /**********************************************************/
            CBINSERTITEM( hwnd, DISP_ENTRY_LEVEL_SPIN, pszPanelText[0] );
            CBINSERTITEM( hwnd, DISP_ENTRY_LEVEL_SPIN, pszPanelText[1] );
            CBINSERTITEM( hwnd, DISP_ENTRY_LEVEL_SPIN, pszPanelText[2] );
            CBSELECTITEM( hwnd, DISP_ENTRY_LEVEL_SPIN, 0 );

            hps = GETPS( hwnd );
            GetCharXY (hps, &(pIda->usCharWidth), &(pIda->usCharHeight));
            RELEASEPS( hwnd, hps );

            /**********************************************************/
            /* Compute minimal dialog size                            */
            /**********************************************************/
            pIda->usTitleHeight = (SHORT) WinQuerySysValue (HWND_DESKTOP, SV_CYTITLEBAR);
            pIda->usBorderSize  = (SHORT) WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER);
            pIda->usScreenXSize = (SHORT) WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);
            pIda->usScreenYSize = (SHORT) WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);

            WinQueryWindowPos( WinWindowFromID( hwnd, DISP_ENTRY_PUBO_COPY ),
                               &pIda->swpButt[0] );
            WinQueryWindowPos( WinWindowFromID( hwnd, DISP_ENTRY_PUBO_EDIT ),
                               &pIda->swpButt[1] );
            WinQueryWindowPos( WinWindowFromID( hwnd, DISP_ENTRY_PUBO_PRINT ),
                               &pIda->swpButt[2] );
            WinQueryWindowPos( WinWindowFromID( hwnd, DISP_ENTRY_PUBO_CANCEL ),
                               &pIda->swpButt[3] );
            WinQueryWindowPos( WinWindowFromID( hwnd,
                               DISP_ENTRY_PUBO_HELP ),
                               &pIda->swpButt[4] );

            pIda->MinX = pIda->usBorderSize * 2;
            for ( usI = 0; usI < 5; usI++ )
            {
              pIda->MinX = (SHORT)(pIda->MinX + pIda->swpButt[usI].cx + 2);
            } /* endfor */
            pIda->MinY = pIda->usTitleHeight +
                         (2 * pIda->usBorderSize) +
                         (6 * pIda->usCharHeight );

            /**********************************************************/
            /* Set intial size and position of dialog                 */
            /**********************************************************/
            prectl = &(pIda->rclDisp);             // for easy access ..

            if ( !PRECTL_XRIGHT(prectl) && !PRECTL_XLEFT(prectl)  &&
                 !PRECTL_YTOP(prectl)   && !PRECTL_YBOTTOM(prectl) )
            {
               // set default size and position
               swp.x  =  100;
               swp.cx = max( (SHORT)(pIda->usScreenXSize / 3), pIda->MinX );
               swp.y  =  100;
               swp.cy = max( (SHORT)(pIda->usScreenYSize / 4), pIda->MinY );
            }
            else
            {
              swp.x  = (SHORT)PRECTL_XLEFT(prectl);
              swp.cx = max( (SHORT)(PRECTL_XRIGHT(prectl) - PRECTL_XLEFT(prectl)), pIda->MinX );
              swp.y  = (SHORT)PRECTL_YBOTTOM(prectl);
              swp.cy = max( (SHORT)(PRECTL_YTOP(prectl) - PRECTL_YBOTTOM(prectl)), pIda->MinY );
            } /* endif */

             if ( !(pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE)  )
            {
              sprintf( pIda->szObjName, "DISPLAY: %ld", (LONG)hwnd );
              EqfRegisterObject( pIda->szObjName, hwnd, clsDICTDISP );
              pIda->fRegistered = TRUE;
            } /* endif */

            WinSetWindowPos( hwnd, HWND_TOP,
                             swp.x, swp.y, swp.cx, swp.cy,
                             EQF_SWP_MOVE | EQF_SWP_SIZE |
                             EQF_SWP_SHOW | EQF_SWP_ACTIVATE );
         } /* endif */

         /*************************************************************/
         /* Disable copy button                                       */
         /*************************************************************/
//       if ( fOK )
//       {
//         ENABLECTRL( hwnd, DISP_ENTRY_PUBO_COPY, FALSE );
//       } /* endif */

         if ( !fOK )
         {
            WinPostMsg (hwnd, WM_CLOSE, NULL, NULL);
         }
         else if ( !(pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE)  )
         {
           UtlRegisterModelessDlg( hwnd );
           LupRegisterDisplayDlg( pIda->pLUPCB, hwnd );
         } /* endif */

         return( MRFROMSHORT(TRUE) );

    case WM_GETMINMAXINFO:
      {
        MINMAXINFO FAR *lpMinMax;      // ptr to min/max info structure

        lpMinMax = (MINMAXINFO *)PVOIDFROMMP2(mp2);
        pIda = ACCESSDLGIDA( hwnd, PDISPIDA );
        lpMinMax->ptMinTrackSize.x = pIda->MinX;
        lpMinMax->ptMinTrackSize.y = pIda->MinY;
      }
      break;

      case WM_SIZE :
         // resize inner window only if normal sizing request...
         pIda = ACCESSDLGIDA( hwnd, PDISPIDA );
         if ( (pIda != NULL) &&
              ((mp1 == SIZENORMAL) || (mp1 == SIZEFULLSCREEN)) )
         {
           SHORT   sWidth  = LOWORD( mp2 );      // new width of dialog
           SHORT   sHeight = HIWORD( mp2 );      // new height of dialog

           pIda = ACCESSDLGIDA( hwnd, PDISPIDA );

           pIda->usBorderSize  = (SHORT) WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER);

           cxAvail = sWidth - (2 * pIda->usBorderSize);

           /***********************************************************/
           /* We have to resize the entry field and to rearrange the  */
           /* push buttons at the bootom of the dialog                */
           /***********************************************************/
           GetRelativeWindowPos( WinWindowFromID( hwnd, DISP_ENTRY_MLE_ENTRY ),
                                 &swp );
           MoveWindow( WinWindowFromID (hwnd, DISP_ENTRY_MLE_ENTRY),
                       pIda->usBorderSize + 1,
                       2 * pIda->usCharHeight,
                       cxAvail - 2,
                       sHeight - pIda->usTitleHeight
                               - (2 * pIda->usBorderSize)
                               - pIda->swpButt[0].cy  // place for pushbutton
                               - pIda->usCharHeight,  // area
                       TRUE );
           /***********************************************************/
           /* Post notification message so that new dialog position   */
           /* may be stored in property files and set new dlg         */
           /* position in LUP control structure                       */
           /***********************************************************/
           {
             SWP   swp;

             WinPostMsg (pIda->pLUPCB->hwndParent,
                        pIda->pLUPCB->usNotifyMsg,
                        MP1FROMHWND( hwnd ),
                        MP2FROM2SHORT( DLG_POSITIONED, DISP_ENTRY_DLG ) );

             WinQueryWindowPos( hwnd, &swp );
             RECTL_XLEFT(pIda->pLUPCB->rclDisp)   = swp.x;
             RECTL_XRIGHT(pIda->pLUPCB->rclDisp)  = swp.x + swp.cx;
             RECTL_YBOTTOM(pIda->pLUPCB->rclDisp) = swp.y;
             RECTL_YTOP(pIda->pLUPCB->rclDisp)    = swp.y + swp.cy;
           }

           /***********************************************************/
           /* Adjust positions of pushbuttons                         */
           /***********************************************************/
           ulTotSize = 0;
           for ( usI = 0; usI < 5; usI++ )
           {
             ulTotSize += pIda->swpButt[usI].cx;
           } /* endfor */

           if ( cxAvail > (LONG)ulTotSize )
           {
             ulTotGaps = cxAvail - ulTotSize;
           }
           else
           {
             ulTotGaps = 0;
           } /* endif */
           ulGap     = ulTotGaps / 5;
           if ( cxAvail > (LONG)ulTotSize )
           {
             ulCorrect = (cxAvail - (ulGap * 5) - ulTotSize) / 2;
           }
           else
           {
             ulCorrect = 0;
           } /* endif */
           ulXPos    = pIda->usBorderSize;

           {
             HDWP hdwp = BeginDeferWindowPos( 5 );

             for ( usI = 0; (usI < 5) && (hdwp != NULL); usI++ )
             {
               if ( usI )
               {
                 ulXPos += pIda->swpButt[usI-1].cx + ulGap;
               }
               else
               {
                 ulXPos += (ulGap / 2) + ulCorrect;
               } /* endif */
               hdwp = DeferWindowPos( hdwp, pIda->swpButt[usI].hwnd,
                                      HWND_TOP,
                                      ulXPos,
                                      sHeight - pIda->usBorderSize - pIda->swpButt[usI].cy,
                                      pIda->swpButt[usI].cx,
                                      pIda->swpButt[usI].cy,
                                      SWP_NOACTIVATE | SWP_NOSIZE |
                                      SWP_NOZORDER );
             } /* endfor */
             if ( hdwp != NULL )
             {
               EndDeferWindowPos( hdwp );
             } /* endif */
           }

           DicDspFillMLE( hwnd, pIda );

         } /* endif */
         break;

       case WM_MOVE :
         pIda = ACCESSDLGIDA( hwnd, PDISPIDA );
         /***********************************************************/
         /* Post notification message so that new dialog position   */
         /* may be stored in property files and set new dlg         */
         /* position in LUP control structure                       */
         /***********************************************************/
         {
           SWP   swp;

           WinPostMsg (pIda->pLUPCB->hwndParent,
                      pIda->pLUPCB->usNotifyMsg,
                      MP1FROMHWND( hwnd ),
                      MP2FROM2SHORT( DLG_POSITIONED, DISP_ENTRY_DLG ) );

           WinQueryWindowPos( hwnd, &swp );
           RECTL_XLEFT(pIda->pLUPCB->rclDisp)   = swp.x;
           RECTL_XRIGHT(pIda->pLUPCB->rclDisp)  = swp.x + swp.cx;
           RECTL_YBOTTOM(pIda->pLUPCB->rclDisp) = swp.y;
           RECTL_YTOP(pIda->pLUPCB->rclDisp)    = swp.y + swp.cy;
         }
         break;
    case WM_EQF_COMMAND:
    case WM_COMMAND:
       usControlID   = WMCOMMANDID( mp1, mp2 );
       pIda = ACCESSDLGIDA( hwnd, PDISPIDA );
       switch (usControlID)
       {
          case DISP_ENTRY_PUBO_HELP:
            UtlInvokeHelp();
            break;
          case DISP_ENTRY_PUBO_EDIT:
             pDictInfo = DicDspGetSelectedDict( hwnd, pIda );
             if ( pDictInfo )
             {
				HMODULE hResMod;
				hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                usRC = SearchAndEdit( hwnd, pIda->pLUPCB,
                                      pIda->ucTerm,
                                      hResMod,
                                      pDictInfo->hDCB );
                if ( usRC == DID_ERROR )      // Failed edit dialog
                {
                   // do nothing
                }
                else if ( usRC == TRUE )      // Has dictionary been changed?
                {
                   // Yes! ==> time to update our entry display
                   fOK = DicDspRefreshDictInfo( pIda );
                   DicDspRefreshDictSpin( hwnd, pIda );

                   if ( fOK && pIda->usFoundInfos )
                   {
                      // force a MLE refresh

                      WinPostMsg( hwnd, WM_EQF_COMMAND,
                                  MP1FROMSHORT( DISP_ENTRY_LEVEL_SPIN ),
                                  MP2FROM2SHORT( 0, SPBN_CHANGE ) );
                   }
                   else
                   {
                      // remove dialog
                      WinPostMsg( hwnd, WM_CLOSE, 0L, 0L );
                   } /* endif */
                } /* endif */
             } /* endif */
             return 0;

          case DISP_ENTRY_PUBO_PRINT:
             PrintMLE( hwnd, pIda );
             return 0;

          case DID_CANCEL:
          case DISP_ENTRY_PUBO_CANCEL:
             WinPostMsg (hwnd, WM_CLOSE, NULL, NULL);
             return 0;

          case DISP_ENTRY_PUBO_COPY:
              {
                DWORD Selection;
                Selection = SendDlgItemMessage( hwnd, DISP_ENTRY_MLE_ENTRY,
                                                EM_GETSEL, 0, 0L );
                if ( LOWORD(Selection) != HIWORD(Selection) )
                {
                  SendDlgItemMessage( hwnd, DISP_ENTRY_MLE_ENTRY, WM_COPY, 0, 0L );
                  WinPostMsg (hwnd, WM_CLOSE, NULL, NULL);
                } /* endif */
              }
              return 0;

          /************************************************************/
          /* here we process the notification messages which are     */
          /* under OS/2 processed in the WM_CONTROL case ...         */
          /************************************************************/
          case DISP_ENTRY_LEVEL_SPIN:
          case DISP_ENTRY_DICT_SPIN:
          case DISP_ENTRY_MLE_ENTRY :
             DicDspControl( hwnd, WMCOMMANDID( mp1, mp2 ),
                            WMCOMMANDCMD( mp1, mp2 ) );
             break;
       } /* endswitch */
       break;

    case WM_CLOSE:
       pIda = ACCESSDLGIDA( hwnd, PDISPIDA );

       DelCtrlFont (hwnd, DISP_ENTRY_MLE_ENTRY);

       if ( pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE  )
       {
         WinDismissDlg( hwnd, FALSE );
       }
       else
       {
         SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ), WM_MDIDESTROY,
                      MP1FROMHWND(hwnd), 0L ) ;
       } /* endif */
       return 0;

    case WM_DESTROY:
         pIda = ACCESSDLGIDA( hwnd, PDISPIDA );
         if (pIda)
         {
           if ( !(pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE)  )
           {
             UtlUnregisterModelessDlg( hwnd );
             LupUnregisterDisplayDlg( pIda->pLUPCB, hwnd );
           } /* endif */
           if ( pIda->fRegistered )
           {
               EqfRemoveObject(TWBFORCE, hwnd);
               pIda->fRegistered = FALSE;
           } /* endif */

           if ( WinIsWindow( (HAB)UtlQueryULong( QL_HAB ),
                pIda->pLUPCB->hwndParent ) )
           {
              WinPostMsg( pIda->pLUPCB->hwndParent,
                          pIda->pLUPCB->usNotifyMsg,
                          MP1FROMHWND(hwnd),
                          MP2FROM2SHORT( DLG_TERM_NORM, DISP_ENTRY_DLG ) );

              // if hwndCall is visible (= display has been called from the
              // lookup dialog directly) activate lookup else activate parent
              // window
              if ( !(pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE)  )
              {
                /******************************************************/
                /* Only activate calling window if it is NOT a        */
                /* window of our workbench (e.g. is called by XLATE   */
                /* or LPEX editor)                                    */
                /* To check if it is a workbench window we follow     */
                /* the parent chain up to the workbench window or the */
                /* desktop window (whichever comes first)             */
                /******************************************************/
                HWND hwndTWB     = (HWND)UtlQueryULong( QL_TWBCLIENT );
                HWND hwndTemp    = pIda->hwndCall;
                while ( (hwndTemp != NULLHANDLE)  &&
                        (hwndTemp != hwndTWB) )
                {
                  hwndTemp = GETPARENT( hwndTemp );
                } /* endwhile */
                if ( hwndTemp != hwndTWB )
                {
                  HWND hwndActivate = WinIsWindowVisible( pIda->hwndCall ) ?
                                      pIda->hwndCall :
                                      pIda->pLUPCB->hwndParent;

                  SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                               WM_MDIACTIVATE, MP1FROMHWND(hwndActivate), 0L );
                } /* endif */
              } /* endif */
           } /* endif */

           for ( usI = 0; usI < pIda->usFoundInfos; usI++ )
           {
              QLDBDestroyTree( &(pIda->FoundInfo[usI].hLDBTree) );
           } /* endfor */

           if ( pIda->hFilter )
           {
             FiltClose( pIda->hFilter );
           } /* endif */
           UtlAlloc( (PVOID *)&pIda->pucMLEBuffer, 0L, 0L, NOMSG );
           UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG);
         } /* endif */
         return 0;

    }
    return WinDefDlgProc (hwnd, msg, mp1, mp2);

} /* end DISPENTRYDLGPROC */

MRESULT DicDspControl
(
   HWND   hwnd,                        // dialog handle
   SHORT  sId,                         // id in action
   SHORT  sNotification                // notification
)
{
  PDISPIDA pIda;                       // lookup dialog IDA

  pIda = ACCESSDLGIDA( hwnd, PDISPIDA );

  switch ( sId )
  {
    case DISP_ENTRY_LEVEL_SPIN:
       switch ( sNotification )
       {
          case CBN_SELCHANGE:
             if ( !DicDspFillMLE( hwnd, pIda ) )
             {
                WinPostMsg (hwnd, WM_CLOSE, NULL, NULL);
             } /* endif */
             break;
       } /* endswitch */
       break;

    case DISP_ENTRY_DICT_SPIN:
       switch ( sNotification )
       {
          case CBN_SELCHANGE:
             if ( !DicDspFillMLE( hwnd, pIda ) )
             {
                WinPostMsg (hwnd, WM_CLOSE, NULL, NULL);
             } /* endif */
             break;
       } /* endswitch */

//  case DISP_ENTRY_MLE_ENTRY :
//    switch ( sNotification )
//    {
//       case EN_KILLFOCUS:
//          ENABLECTRL( hwnd, DISP_ENTRY_PUBO_COPY, FALSE );
//          break;
//
//       case EN_SETFOCUS:
//          ENABLECTRL( hwnd, DISP_ENTRY_PUBO_COPY, TRUE );
//          break;
//    } /* endswitch NotifyCode */
      break;

  } /* endswitch */
   return ( 0 );
} /* end of DicDspControl */

//+----------------------------------------------------------------------------+
//| DicDspFillMLE           - Fills the entry MLE with data of dictionry data  |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    see EQFDDISP.H                                                          |
//+----------------------------------------------------------------------------+
BOOL DicDspFillMLE
(
   HWND      hwnd,                     // handle of display dialog window
   PDISPIDA  pIda                      // ptr to display dialog IDA
)
{
   PFOUNDINFO pFoundInfo;              // pointer to FoundInfo structure of
                                       //   current dictionary
   BOOL       fOK;                      // internal OK flag, returned to caller
   PUCB       pUCB;                    // pointer to user control block
   USHORT      usI;                    // general loop index
   USHORT      usDisplayLevel;         // current display level

   // get current display level number
   SHORT sItem;
   CBQUERYSELECTEDITEMTEXT( sItem, hwnd, DISP_ENTRY_LEVEL_SPIN, pIda->szTemp );
   for ( usI = 0; usI < 3; usI++ )
   {
     if ( strcmp( pIda->szTemp, pIda->szPanelText[usI] ) == 0 )
     {
       break;
     } /* endif */
   } /* endfor */
   usDisplayLevel = ( usI < 3 ) ? usI + 1 : 1;

   // get FoundInfo structure for this dictionary
   pFoundInfo = DicDspGetSelectedDict( hwnd, pIda );

   // check if pFoundInfo is OK
   fOK = ( pFoundInfo != NULL );

   // fill MLE
   if ( fOK )
   {
      pUCB = (PUCB)pIda->pLUPCB->hUCB;
      fOK = FillMLE( WinWindowFromID( hwnd, DISP_ENTRY_MLE_ENTRY ),
                     pIda->usMLECharWidth,
                     pIda->pucMLEBuffer,
                     pIda->usMLEBufSize,
                     pFoundInfo->hDCB,
                     pFoundInfo->hLDBTree,
                     usDisplayLevel);

   } /* endif */

   return( fOK );
}

//+----------------------------------------------------------------------------+
//| FillMLE                 - Fills the entry MLE with data of dictionry data  |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    see EQFDDISP.H                                                          |
//+----------------------------------------------------------------------------+
BOOL FillMLE
(
   HWND      hwndMLE,                  // handle of MLE
   USHORT    usMLECharWidth,           // character width of MLE
   PSZ_W     pucMLEBuffer,             // buffer for MLE data
   USHORT    usBufSize,                // size of MLE buffer
   HDCB      hDCB,                     // dictionary handle
   PVOID     hLDBTree,                 // LDB tree handle
   USHORT    usDisplayLevel          // display level
)
{
   PDCB      pDCB;                   // dictionary control block pointer
   BOOL      fOK = TRUE;                     // internal OK flag, returned to caller
   PSZ_W     pucEntry;                   // ptr to field data
   USHORT    usDataLen;                   // length of field or MLE data
   PPROFENTRY pProfEntry;              // ptr to current profile entry
   PSZ_W      pucDisplMem;                // ptr for MLE buffer processing
   SWP        swpMLE;                  // size and position of MLE
   ULONG     ulMLEWidth;              // width of MLE in character units
   BOOL       fFirstTime;              // flag for calls to BuildLine function
   USHORT         usLdbRc;             // return code of LDB functions
   USHORT         usLevel;             // level of current node
   USHORT         usField;             // number of currently processed field
   USHORT         usI;                 // general loop index
                                                                /* 3@KIT0899A */
   USHORT         usFreeSpace;         // free space left in buffer
   USHORT         usLineLen;           // length of cuurent data line
   CHAR_W         chBufW[DICTENTRYLENGTH];
   ULONG          ulOemCP = 0L;

   ulOemCP = GetLangOEMCP(NULL);
   /*******************************************************************/
   /* Convert dictionary handle to control block pointer              */
   /*******************************************************************/
   pDCB = (PDCB)hDCB;

   /*******************************************************************/
   /* get current MLE width in character units                        */
   /*******************************************************************/
   WinQueryWindowPos( hwndMLE, &swpMLE );
   ulMLEWidth = swpMLE.cx -
                (2 * WinQuerySysValue( HWND_DESKTOP, SV_CXBORDER )) -
                     WinQuerySysValue( HWND_DESKTOP, SV_CXVSCROLL );
   ulMLEWidth = ( usMLECharWidth ) ? ulMLEWidth / usMLECharWidth :
                0;
   ulMLEWidth -= 1;                   // leave room for one characters

   ENABLEUPDATEHWND_FALSE( hwndMLE );

   SETTEXTHWND( hwndMLE, EMPTY_STRING );

   /*******************************************************************/
   /* fill MLE buffer -- usBufSize is in number of CHAR_Ws            */
   /*******************************************************************/
   UTF16memset( pucMLEBuffer, ' ', usBufSize );
   pucDisplMem = pucMLEBuffer;
   usFreeSpace =  usBufSize - usMLECharWidth - 4;               /* 1@KIT0899A */

   /****************************************************************/
   /* Reset tree positions                                         */
   /****************************************************************/
   QLDBResetTreePositions( hLDBTree );

   /*****************************************************************/
   /* Get current (=first node)                                     */
   /*****************************************************************/
   usLdbRc = QLDBCurrNode( hLDBTree, pDCB->apszFields, &usLevel );

   /*****************************************************************/
   /* Process nodes while no error occurs and tree has more nodes   */
   /*****************************************************************/
   while ( usFreeSpace &&                                       /* 1@KIT0899A */
           fOK && (usLdbRc == QLDB_NO_ERROR) &&                 /* 2@KIT0899C */
           (usLevel != QLDB_END_OF_TREE) )
   {
     /***************************************************************/
     /* Add fields of current node to MLE buffer                    */
     /***************************************************************/
     usI = 0;
     while ( usFreeSpace &&                                     /* 1@KIT0899A */
             fOK && (usI < pDCB->ausNoOfFields[usLevel-1]) )    /* 1@KIT0899C */
     {
       usField = pDCB->ausFirstField[usLevel-1] + usI;
       pProfEntry = pDCB->Prop.ProfEntry + usField;
       if ( pDCB->apszFields[usI]    &&
            *(pDCB->apszFields[usI]) &&
            pProfEntry->usDisplay   &&
            (pProfEntry->usDisplay <= usDisplayLevel) &&
            strcmp( pProfEntry->chSystName, HEADWORD_SYST_NAME ) )
       {
         /***********************************************************/
         /* Write field data                                        */
         /***********************************************************/
         if ( fOK )
         {
           pucEntry = pDCB->apszFields[usI];
           usDataLen = (USHORT) UTF16strlenCHAR( pucEntry );
           fFirstTime = TRUE;
           ASCII2UnicodeBuf( pProfEntry->chUserName, chBufW,
                             sizeof(chBufW)/ sizeof(CHAR_W), ulOemCP);
           while( usFreeSpace &&                                 /* 1@KIT0899A */
                  BuildLine( pucDisplMem, ulMLEWidth,           /* 1@KIT0899C */
                             usFreeSpace,
                             chBufW,
                             &fFirstTime,
                             &pucEntry, &usDataLen /* fisDBCS1*/ ) )
           {
              usLineLen = (USHORT) UTF16strlenCHAR( pucDisplMem );                /* 10@KIT0899A */
              if ( usFreeSpace > (usLineLen + 1) )
              {
                usFreeSpace -= (usLineLen + 1);
              }
              else
              {
                usFreeSpace = 0;
              } /* endif */
              pucDisplMem += usLineLen;
              *(pucDisplMem++) = CR;
              *(pucDisplMem++) = LF;
           } /* endwhile */
         } /* endif */
       } /* endif */
       usI++;
     } /* endwhile */

     /***************************************************************/
     /* Get next node of node tree                                  */
     /***************************************************************/
     usLdbRc = QLDBNextNode( hLDBTree, pDCB->apszFields, &usLevel );
   } /* endwhile */

   *(pucDisplMem--) = 0;
   usDataLen = (USHORT) (pucDisplMem - pucMLEBuffer );
 //  if ( GetCharSet() != THAI_CHARSET )
 //  {
 //     OEMTOANSI( pucMLEBuffer );
 //  }
   SETTEXTHWNDW( hwndMLE, pucMLEBuffer );
   ENABLEUPDATEHWND_TRUE ( hwndMLE);

   return( fOK );
}

/*******************************************************************************
*
*       function:  PrintMLE
*
*******************************************************************************/

VOID PrintMLE (HWND hwnd, PDISPIDA pIda )
{
   PFOUNDINFO pFoundInfo;              // pointer to FoundInfo structure of
                                       //   current dictionary
   PDCB       pDCB = NULL;             // dictionary control block pointer
   BOOL       fOK = TRUE;              // internal OK flag, returned to caller
   PSZ_W      pucEntry;                // ptr to field data
   USHORT     usDataLen;               // length of field or MLE data
   PPROFENTRY pProfEntry;              // ptr to current profile entry
   PSZ_W      pucDisplMem;                // ptr for MLE buffer processing
   HPRINT        hPrint = NULLHANDLE;  // print handle
   CHAR_W         chPrintLine[81];     // buffer for printing
   BOOL           fFirstTime;          // flag for calls to BuildLine function
   USHORT         usLdbRc;             // return code of LDB functions
   USHORT         usLevel;             // level of current node
   USHORT         usField;             // number of currently processed field
   USHORT         usI;                 // general loop index
   PUCB           pUCB;                    // pointer to user control block
   USHORT         usDisplayLevel;         // current display level
   CHAR_W         chTempW[MAX_LONGFILESPEC];
   PSZ_W          pTempW;
   ULONG          ulOemCP = 0L;


   // get current display level number
   SHORT sItem;
   HMODULE hResMod;
   hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
   CBQUERYSELECTEDITEMTEXT( sItem, hwnd, DISP_ENTRY_LEVEL_SPIN, pIda->szTemp );

   ulOemCP = GetLangOEMCP(NULL);
   for ( usI = 0; usI < 3; usI++ )
   {
     if ( strcmp( pIda->szTemp, pIda->szPanelText[usI] ) == 0 )
     {
       break;
     } /* endif */
   } /* endfor */
   usDisplayLevel = ( usI < 3 ) ? usI + 1 : 1;


   // get FoundInfo structure for this dictionary
   pFoundInfo = DicDspGetSelectedDict( hwnd, pIda );

   // check if pFoundInfo is OK
   fOK = ( pFoundInfo != NULL );

   // open print device context
   if ( fOK )
   {
      ULONG  ulLen;                 // length of generated line
      PSZ_W    pszEntry;                 // ptr to dict entry name

      UtlLoadStringW( (HAB)UtlQueryULong( QL_HAB ),
                           hResMod, SID_DISP_DICT_PRTJOB, pIda->szTemp,
                           pIda->szUniBuffer,  sizeof(pIda->szUniBuffer) / sizeof(CHAR_W));

      pszEntry = pIda->ucTerm;
      DosInsMessageW( &pszEntry, 1, pIda->szUniBuffer, UTF16strlenCHAR(pIda->szUniBuffer),
                     chPrintLine, (SHORT)(sizeof(chPrintLine) - 1),
                     &ulLen );
      chPrintLine[ulLen] = EOS;
      fOK = UtlPrintOpenW( &hPrint, chPrintLine, hwnd );
   } /* endif */

   // print dictionary name
   if ( fOK )
   {
      UtlLoadStringW( WinQueryAnchorBlock (hwnd),
                     hResMod, SID_DISP_DICT_TEXT, pIda->szTemp,
                     pIda->szUniBuffer, sizeof(pIda->szUniBuffer)/ sizeof(CHAR_W) );
      pDCB = (PDCB)pFoundInfo->hDCB;

      fFirstTime = TRUE;
      //pucTemp  = pFoundInfo->szDictName;
      usDataLen = (USHORT) strlen(pFoundInfo->szDictName);

      ASCII2UnicodeBuf( pFoundInfo->szDictName, chTempW,
                        sizeof(chTempW)/ sizeof(CHAR_W),ulOemCP);

      pUCB = (PUCB)pIda->pLUPCB->hUCB;

    // if ( GetCharSet() != THAI_CHARSET )
    //{
    //   ANSITOOEM( pIda->szBuffer );
    //  }
      pTempW = chTempW;
      BuildLine( chPrintLine,
                 sizeof(chPrintLine) - 2,
                 sizeof(chPrintLine) - 2,
                 pIda->szUniBuffer,
                 &fFirstTime,
                 &pTempW,
                 &usDataLen /*pUCB->fisDBCS1*/ );

      UTF16strcat( chPrintLine, L"\n" );
      fOK = UtlPrintLineW( hPrint, chPrintLine );
      if ( fOK )
      {
        UTF16strcpy( chPrintLine, L"\n" );
        fOK = UtlPrintLineW( hPrint, chPrintLine );
      } /* endif */
   } /* endif */

   // print fields of dictionary entry
   if ( fOK )
   {
      UTF16memset( pIda->pucMLEBuffer, ' ', pIda->usMLEBufSize );
      pucDisplMem = pIda->pucMLEBuffer;

      /****************************************************************/
      /* Reset tree positions                                         */
      /****************************************************************/
      QLDBResetTreePositions( pFoundInfo->hLDBTree );

      /*****************************************************************/
      /* Get current (=first node)                                     */
      /*****************************************************************/
      usLdbRc = QLDBCurrNode( pFoundInfo->hLDBTree,
                              pDCB->apszFields, &usLevel );

      /*****************************************************************/
      /* Process nodes while no error occurs and tree has more nodes   */
      /*****************************************************************/
      while ( fOK &&
              (usLdbRc == QLDB_NO_ERROR) &&
              (usLevel != QLDB_END_OF_TREE) )
      {
        /***************************************************************/
        /* Print fields of current node                                */
        /***************************************************************/
        usI = 0;
        while ( fOK && (usI < pDCB->ausNoOfFields[usLevel-1]) )
        {
          usField = pDCB->ausFirstField[usLevel-1] + usI;
          pProfEntry = pDCB->Prop.ProfEntry + usField;
          if ( pDCB->apszFields[usI]    &&
               *(pDCB->apszFields[usI]) &&
               pProfEntry->usDisplay   &&
               (pProfEntry->usDisplay <= usDisplayLevel) )
          {
            /***********************************************************/
            /* Write field data                                        */
            /***********************************************************/
            if ( fOK )
            {
              pucEntry = pDCB->apszFields[usI];
              usDataLen = (USHORT) UTF16strlenCHAR( pucEntry );
              fFirstTime = TRUE;
              ASCII2UnicodeBuf( pProfEntry->chUserName,
                                chTempW, sizeof(chTempW)/sizeof(CHAR_W), ulOemCP);
              while( BuildLine( chPrintLine,
                                sizeof(chPrintLine) - 2,
                                sizeof(chPrintLine) - 2,
                                chTempW,
                                &fFirstTime,
                                &pucEntry, &usDataLen /* pUCB->fisDBCS1*/) )

              {
                 UTF16strcat( chPrintLine, L"\n" );
                 fOK = UtlPrintLineW( hPrint, chPrintLine );

                 //--- add empty line after headword ---
                 if ( strcmp( pProfEntry->chSystName,
                              HEADWORD_SYST_NAME ) == 0 )
                 {
                    UTF16strcpy( chPrintLine, L"\n" );
                    fOK = UtlPrintLineW( hPrint, chPrintLine );
                 } /* endif */
              } /* endwhile */
            } /* endif */
          } /* endif */
          usI++;
        } /* endwhile */

        /***************************************************************/
        /* Get next node of node tree                                  */
        /***************************************************************/
        usLdbRc = QLDBNextNode( pFoundInfo->hLDBTree, pDCB->apszFields,
                                &usLevel );
      } /* endwhile */
   } /* endif */

   if ( hPrint )         UtlPrintClose( hPrint );
}

BOOL DisplayEntry
(
   PLUPCB  pLUPCB,                     // lookup control block pointer
   PSZ_W   pucTerm,                    // term bein displayed
   HMODULE hmod,                       // resource module handle
   HWND    hwndCall,                   // caller's window handle
   PSZ     pszFilter                   // name of selected filter or empty string
)
{
  PDISPIDA      pIda = NULL;           // display dialog IDA
  BOOL          fOK = TRUE;            // internal OK flag
  USHORT        usAsdRC;               // return code of ASD functions
  USHORT        usI;                   // loop index
  POBJLST       pObjList;              // pointer for object list
  POBJLST       pObject;               // pointer for object list processing
  LONG          lLastTime = 0L;        // timestamp of newest display dialog
  SWP           swpDisp;               // size/position of newest disp. dlg

  memset (&swpDisp, 0, sizeof(swpDisp));
  /********************************************************************/
  /* Check if a dictionary display window for the same term and the   */
  /* same lookup control block is active somethere ...                */
  /********************************************************************/
  if ( (usI = EqfQueryObjectCount( clsDICTDISP ) ) != 0 )
  {
     UtlAlloc( (PVOID *)&pObjList, 0L, (LONG) ((usI * sizeof( *pObjList)) + 20),
               ERROR_STORAGE );
     if ( pObjList )
     {
        usI = EqfGetObjectList( clsDICTDISP, usI, pObjList);
        pObject = pObjList;
        for( ; usI; usI--,pObject++ )
        {
          /************************************************************/
          /* Access IDA of dictionary display object                  */
          /************************************************************/
          pIda = ACCESSDLGIDA( pObject->hwnd, PDISPIDA );

          if ( pIda )
          {
             if ( (pIda->pLUPCB == pLUPCB) &&
                  !(pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE)  )
             {
               if ( UTF16strcmp( pIda->ucTerm, pucTerm ) == 0 )
               {
                 /*******************************************************/
                 /* Dictionary display for current term and current     */
                 /* lookup control block is already active. Set focus   */
                 /* to existing window, free objectlist and return      */
                 /* from function                                       */
                 /*******************************************************/

                 SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                              WM_MDIACTIVATE, MP1FROMHWND(pIda->hwnd), 0L );

                 UtlAlloc( (PVOID *)&pObjList, 0L, 0L, NOMSG );
                 return( TRUE );
               }
               else
               {
                 /*****************************************************/
                 /* Another term of same lookup session               */
                 /*    If this dialog is newer than the other dlgs so */
                 /*    far, get dialog rectangle                      */
                 /*****************************************************/
                 if ( pIda->lTimeStamp > lLastTime )
                 {
                   lLastTime = pIda->lTimeStamp;
                   WinQueryWindowPos( pIda->hwnd, &swpDisp );
                 } /* endif */
               } /* endif */
             } /* endif */
          } /* endif */
        } /* endfor */
        UtlAlloc( (PVOID *)&pObjList, 0L, 0L, NOMSG );
        pIda = NULL;
     } /* endif */
  } /* endif */


  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(DISPIDA), ERROR_STORAGE );
  if ( fOK )
  {
     fOK = UtlAlloc( (PVOID *)&pIda->pucMLEBuffer, 0L, (LONG) MAX_ALLOC * sizeof(CHAR_W),
                     ERROR_STORAGE );
     pIda->usMLEBufSize = MAX_ALLOC;
  } /* endif */

  //
  // fill fields in IDA
  //
  if ( fOK )
  {
     UTF16strcpy( pIda->ucTerm, pucTerm );
     memcpy( &pIda->rclDisp, &pLUPCB->rclDisp, sizeof(pIda->rclDisp) );
     UtlTime( &pIda->lTimeStamp );

     /*****************************************************************/
     /* Use size/position of existing display dialogs of same         */
     /* lookup group                                                  */
     /*****************************************************************/
     if ( lLastTime != 0L )
     {
       /***************************************************************/
       /* Use rectangle of newest display dialog but shift dialog     */
       /* a bit (= titlebar height) to the left and to the bottom     */
       /***************************************************************/
       lLastTime        = WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR );
       swpDisp.x       = swpDisp.x       + (SHORT)lLastTime;
       swpDisp.y       = swpDisp.y       - (SHORT)lLastTime;
       RECTL_XLEFT(pIda->rclDisp)   = swpDisp.x;
       RECTL_XRIGHT(pIda->rclDisp)  = swpDisp.x + swpDisp.cx;
       RECTL_YBOTTOM(pIda->rclDisp) = swpDisp.y;
       RECTL_YTOP(pIda->rclDisp)    = swpDisp.y + swpDisp.cy;
     } /* endif */

     pIda->pLUPCB   = pLUPCB;
     pIda->hmod     = hmod;
     pIda->hwndCall = hwndCall;
     strcpy( pIda->szFilter, pszFilter );
     if ( pIda->szFilter[0] != EOS )
     {
       usAsdRC = FiltOpen( pIda->szFilter,
                        pIda->pLUPCB->hDCB,
                        &pIda->hFilter );
       if ( usAsdRC != NO_ERROR )
       {
         UtlError( ERROR_DLUP_LOAD_FILTER,
                   MB_CANCEL, 1, &pszFilter, EQF_ERROR );
         fOK = FALSE;
       } /* endif */
     }
     else
     {
       pIda->hFilter = NULL;
     } /* endif */
  } /* endif */

  //
  // get list of available dictionaries
  //
  if ( fOK )
  {
     usAsdRC = AsdRetDictList( pLUPCB->hDCB,
                               pIda->ahDCBAvail,
                               &pIda->usAvailDicts );
     fOK = ( usAsdRC == LX_RC_OK_ASD );
  } /* endif */

  //
  // get all dictionaries with matching terms and setup FoundInfo structure
  // in IDA
  //
  if ( fOK )
  {
     fOK = DicDspRefreshDictInfo( pIda );
  } /* endif */

  if ( fOK )
  {
    HWND           hwndDlg;            // handle of loaded dialog
	HMODULE hResMod;
	hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    if ( pLUPCB->ulFlags & LUP_MODALDLG_MODE  )
    {
       DIALOGBOX( hwndCall, DISPENTRYDLGPROC, hResMod,
                  DISP_ENTRY_DLG, pIda, fOK );
    }
    else
    {
      if ( pLUPCB->lpfnDispProc == NULL )
      {
        pLUPCB->lpfnDispProc = MakeProcInstance( (FARPROC)(DISPENTRYDLGPROC),
                                                 (HAB)UtlQueryULong( QL_HAB ) );
      } /* endif */
       hwndDlg = CreateMDIDialogParam( hResMod,
                                       MAKEINTRESOURCE(DISP_ENTRY_DLG),
                                       (HWND)UtlQueryULong( QL_TWBCLIENT ),
                                       pLUPCB->lpfnDispProc,
                                       MP2FROMP(pIda), FALSE,
                                       (HPOINTER) UtlQueryULong(QL_DICTENTRYDISPICO)); //hiconDICTDISP );

       if ( hwndDlg == NULLHANDLE )
       {
          UtlError (WD_START_DISPLAY, MB_CANCEL, 0, NULL, EQF_ERROR );
          fOK = FALSE;
       } /* endif */
    } /* endif */
   } /* endif */

   return( fOK );
}

//+----------------------------------------------------------------------------+
//| DicDspGetSelectedDict   - get ptr to info structure of selected dictionary |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    see EQFDISP.H                                                           |
//+----------------------------------------------------------------------------+
PFOUNDINFO DicDspGetSelectedDict
(
   HWND      hwnd,                     // handle of display dialog window
   PDISPIDA  pIda                      // ptr to display dialog IDA
)
{
   PFOUNDINFO  pDictInfo = NULL;       // ptr to info of currently selected dictionary
   USHORT      usI;                    // general loop index

   // get name of selected dictionary
   SHORT sItem;
   CBQUERYSELECTEDITEMTEXT( sItem, hwnd, DISP_ENTRY_DICT_SPIN, pIda->szDictNameTemp );

   if ( pIda->szDictNameTemp[0] != EOS )
   {
      // get FoundInfo structure for this dictionary
      usI = 0;
      pDictInfo = pIda->FoundInfo;
      while ( (usI < pIda->usFoundInfos) &&
              (strcmp( pDictInfo->szDictName, pIda->szDictNameTemp) != 0) )
      {
         usI++;
         pDictInfo++;
      } /* endwhile */

      // check if pDictInfo is OK
      if (usI >= pIda->usFoundInfos)
      {
         // internal error: no info for current dictionary ...
         pDictInfo = NULL;
      } /* endif */
   } /* endif */

   return( pDictInfo );

} /* endof DicDspGetActiveDict */

//+----------------------------------------------------------------------------+
//| DicDspRefreshDictSpin   - refresh dictionaries displayed in dict spinbutton|
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    see EQFDISP.H                                                           |
//+----------------------------------------------------------------------------+
VOID DicDspRefreshDictSpin
(
   HWND      hwnd,                     // handle of display dialog window
   PDISPIDA  pIda                      // ptr to display dialog IDA
)
{
   PSZ        pszArray[MAX_DICTS+1];   // array with dictionary names
   USHORT     usI;                     // loop index

   for ( usI = 0; usI < pIda->usFoundInfos; usI++ )
   {
      pszArray[usI] = pIda->FoundInfo[usI].szDictName;
      CBINSERTITEM( hwnd, DISP_ENTRY_DICT_SPIN, pIda->FoundInfo[usI].szDictName );
   } /* endfor */

   CBSELECTITEM( hwnd, DISP_ENTRY_DICT_SPIN, 0 );
} /* endof DicDspRefreshDictSpin */

//+----------------------------------------------------------------------------+
//| DicDspRefreshDictInfo   - refresh dictionary info table in IDA             |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    see EQFDISP.H                                                           |
//+----------------------------------------------------------------------------+
BOOL DicDspRefreshDictInfo
(
   PDISPIDA  pIda                      // ptr to display dialog IDA
)
{
  BOOL          fOK = TRUE;            // internal OK flag
  ULONG         ulTermNum;             // number of term being displayed
  ULONG         ulDataLen;             // length of term data
  HDCB          hEditHandle;           // handle of dictioanry containing term
  USHORT        usAsdRC;               // return code of ASD functions
  USHORT        usI, usJ;              // loop index
  PFOUNDINFO    pFoundInfo;            // ptr for found info table processing
  USHORT        usLdbRC;               // return code of LDB functions
  PSZ_W         pucData = NULL;        // pointer to record data of entry
  PDCB          pDCB = NULL;           // ptr to dictionary control block
  BOOL          fCopyRighted = TRUE;   // TRUE = all dict are copyrighted
  PVOID         hLDBTree = NULL;       // LDB tree handle
  PVOID         hFilterLDBTree;        // LDB tree handle

   //
   // loop through available dictionaries and update FoundInfo table in IDA
   //
   usI = 0;
   while ( fOK && (usI < pIda->usAvailDicts) )
   {
      // lookup term in dictionary
      usAsdRC = AsdFndMatch( pIda->ucTerm,
                             pIda->ahDCBAvail[usI],
                             pIda->pLUPCB->hUCB,
                             pIda->ucbTermBuf,
                             &ulTermNum,
                             &ulDataLen,       // numb of char_w's
                             &hEditHandle );

      if (usAsdRC == LX_WRD_NT_FND_ASD)
      {
         usAsdRC = AsdFndEquivW( pIda->ucTerm,
                                 pIda->ahDCBAvail[usI],
                                 pIda->pLUPCB->hUCB,
                                 pIda->ucbTermBuf,
                                 &ulTermNum,
                                 &ulDataLen,
                                 &hEditHandle );
      } /* endif */

      if ( usAsdRC == LX_RC_OK_ASD )           // if found ...
      {
         /*************************************************************/
         /* Allocate buffer for entry data                            */
         /*************************************************************/
         if ( fOK )
         {
            fOK = UtlAlloc( (PVOID *)&pucData, 0L,
                             sizeof(CHAR_W) * (max( (LONG)MIN_ALLOC, ulDataLen)),
                            ERROR_STORAGE );
         } /* endif */

         /*************************************************************/
         /* Get data of dictionary entry                              */
         /*************************************************************/
         if ( fOK )
         {
            usAsdRC = AsdRetEntryW( pIda->ahDCBAvail[usI],
                                   pIda->pLUPCB->hUCB,
                                   pIda->ucbTermBuf,
                                   &ulTermNum,
                                   pucData,
                                   &ulDataLen,
                                   &pIda->hEditHandle );
            fOK = ( usAsdRC == LX_RC_OK_ASD );
         } /* endif */

         /*************************************************************/
         /* Create LDB tree                                           */
         /*************************************************************/
         if ( fOK )
         {
           pDCB = (PDCB)pIda->ahDCBAvail[usI];
           hLDBTree = NULL;
           usLdbRC = QLDBRecordToTree( pDCB->ausNoOfFields,
                                       pucData, ulDataLen,
                                       &hLDBTree );
           fOK = ( usLdbRC == NO_ERROR );
         } /* endif */

         /*************************************************************/
         /* Discard dictionary data                                   */
         /*************************************************************/
         if ( pucData )
         {
           UtlAlloc( (PVOID *)&pucData, 0L, 0L, NOMSG );
         } /* endif */

         /*************************************************************/
         /* Apply any filter                                          */
         /*************************************************************/
         if ( fOK && pIda->hFilter )
         {
           if ( FiltWork( pIda->hFilter,
                          hLDBTree,
                          &hFilterLDBTree ) == NO_ERROR )
           {
             QLDBDestroyTree( &hLDBTree );
             hLDBTree = hFilterLDBTree;
             hFilterLDBTree = NULL;
           }
           else
           {
             usAsdRC = LX_WRD_NT_FND_ASD;
             QLDBDestroyTree( &hLDBTree );
           } /* endif */
         } /* endif */
      } /* endif */

      if ( usAsdRC == LX_RC_OK_ASD )           // if found ...
      {
         // add new entry or change existing one
         usJ = 0;
         while ( (usJ < pIda->usFoundInfos) &&
                 (pIda->FoundInfo[usJ].hDCB != pIda->ahDCBAvail[usI]) )
         {
            usJ++;
         } /* endwhile */

         if ( usJ < pIda->usFoundInfos )     // if there is a entry ..
         {
            //
            // ... use existing entry
            //
            pFoundInfo = pIda->FoundInfo + usJ;
         }
         else
         {
            //
            // ... create new FoundInfo entry
            //

            // setup pointer to current FoundInfo entry
            pucData = NULL;
            pFoundInfo = pIda->FoundInfo + pIda->usFoundInfos;
            memset( pFoundInfo, 0, sizeof(FOUNDINFO) );

            // get dictionary handle
            pFoundInfo->hDCB = pIda->ahDCBAvail[usI];

            // get dictionary name without path and extension
            AsdQueryDictName( pFoundInfo->hDCB, pFoundInfo->szDictName );

            // increment number of filled FoundInfo structures
            pIda->usFoundInfos++;
         } /* endif */

         if ( pFoundInfo->hLDBTree )
         {
           QLDBDestroyTree( &(pFoundInfo->hLDBTree) );
         } /* endif */
         pFoundInfo->hLDBTree = hLDBTree;
         if ( !pDCB->Prop.fCopyRight )
         {
           fCopyRighted = FALSE;
         } /* endif */
      }
      else                             // = if term was not found in dictionary
      {
         // remove any FoundInfo table entry for this dictionary
         usJ = 0;
         while ( (usJ < pIda->usFoundInfos) &&
                 (pIda->FoundInfo[usJ].hDCB != pIda->ahDCBAvail[usI]) )
         {
            usJ++;
         } /* endwhile */

         if ( usJ < pIda->usFoundInfos )    // if dictionary entry found ...
         {
            // remove the obsolete entry
            QLDBDestroyTree( &(pIda->FoundInfo[usJ].hLDBTree) );
            pIda->usFoundInfos--;
            if ( usJ != pIda->usFoundInfos )    // not the last entry ???
            {
               // more dictionaries following the current one
               memmove( pIda->FoundInfo + usJ, pIda->FoundInfo + usJ + 1,
                        sizeof( FOUNDINFO) * (pIda->usFoundInfos - usJ) );
            } /* endif */
         } /* endif */
      } /* endif */
      usI++;
   } /* endwhile */

   return( fOK );
} /* end of DicDspRefreshDictInfo */


//+----------------------------------------------------------------------------+
//| SetFixedPitchMLEFont    - Set the MLE font to a monospaced font            |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Set the font of the MLE to a font with the same character size as the   |
//|    default system font, but change the typeface of the font to             |
//|    'System Monospaced'.                                                    |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HWND      hwndMLE             IN      handle of MLE                     |
//|    BOOL      fDBCS               IN      DBCS flag                         |
//|    PUSHORT   pusMLECharWidth     IN      ptr to MLE character width field  |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    BOOL                                                                    |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
BOOL SetFixedPitchMLEFont
(
   HWND      hwndMLE,                  // handle of MLE
   BOOL      fDBCS,                    // TRUE = we are running in DBCS environment
   PUSHORT   pusMLECharWidth           // ptr to buffer for MLE character width
)
{
   BOOL          fOK = TRUE;           // internal OK flag
   HFONT    hFont;                     // handle of created font
   HFONT    hOldFont = NULL;
   TEXTMETRIC tm;                    // filled text metrics
   HDC      hdc;                     // device context

   hdc = GetDC( hwndMLE );

   if ( !fDBCS )
   {
     hFont = (HFONT) GetStockObject( ANSI_FIXED_FONT );

     hOldFont = (HFONT) SelectObject( hdc, hFont );
   } /* endif */

   GetTextMetrics( hdc, &tm );

   if ( !fDBCS )
   {
     SelectObject( hdc, hOldFont );
   } /* endif */

   ReleaseDC( hwndMLE, hdc );

   *pusMLECharWidth     = (USHORT)tm.tmAveCharWidth;

   if ( !fDBCS )
   {
     SendMessage( hwndMLE, WM_SETFONT,
                  (WPARAM)GetStockObject( ANSI_FIXED_FONT ),
                  MP2FROMSHORT(FALSE) );
   } /* endif */
   return( fOK );
} /* endof SetFixedPitchMLEFont */


BOOL BuildLine
(
   PSZ_W    pszLineBuf,                // ptr to output buffer
   ULONG    ulMaxColumns,              // max number of character columns
   USHORT   usLineSize,                // size (width) of line
   PSZ_W    pszFieldName,              // name of entry field
   PBOOL    pfFirstTime,               // TRUE if called for the first time
   PSZ_W    *ppData,                   // ptr to entry data pointer
   PUSHORT  pusDataLen                 // length of data area
 //  BOOL     fisDBCS1[]                 // DBCS character flag array-deleted!
)
{
   USHORT   usPos;                     // current position in output buffer
   BOOL     fMoreData = FALSE;         // flag returned to caller
   BOOL     fLineFull = FALSE;         // changes to TRUE if line has been filled
   PSZ_W    pucOut;                    // pointer into output (line) buffer
   PSZ_W    pucIn;                     // pointer into input data
   USHORT   usDataLen;                 // length of remaining data
   PSZ_W    pucInLastSpace;            // position of last space in input
   PSZ_W    pucOutLastSpace = NULL;    // position of last space in output
   USHORT   usDataLenLastSpace = 0;    // data length at last space
   CHAR_W   ucCurrent;                 // buffer for current character
   USHORT   usCol = 1;                 // current output column

   usDataLen = *pusDataLen;
   pucInLastSpace = NULL;              // no space position remembered yet

   if ( usDataLen )
   {
     BOOL fShiftOut = FALSE;          // not in ShiftOut mode for WP control
      fMoreData = TRUE;

      // setup first part of string
      if ( *pfFirstTime )
      {
         // add field name to buffer
         usPos = (USHORT)max( UTF16strlenCHAR(pszFieldName), DISP_NAME_SIZE ) + 1;
         UTF16memset( pszLineBuf, ' ', (USHORT)(usPos + 1));
         memcpy( pszLineBuf, pszFieldName, UTF16strlenBYTE(pszFieldName) );

         pszLineBuf[usPos++] = ':';
         pszLineBuf[usPos++] = ' ';
         *pfFirstTime = FALSE;
         usCol = usPos;
      }
      else
      {
         // blank out room occupied by field name
         UTF16memset( pszLineBuf, ' ', DISP_NAME_SIZE + 4 );
         usCol = usPos = DISP_NAME_SIZE + 3;
      } /* endif */

      // fill-in data of field
      pucOut = pszLineBuf + usPos;
      pucIn  = *ppData;
      while ( usDataLen && !fLineFull )
      {
        ucCurrent = *(pucOut++) = *(pucIn++);

        if ( ucCurrent == '\r')
        {
           usDataLen--;             // decrement number of data bytes
           pucOut--;                  // remove last character
           fLineFull = TRUE;          // set line complete flag
        }
        else
        {
           usDataLen--;
           usPos++;
           usCol++;

           // DBCS characters cover two display positions...
           if ( EQFIsDBCSChar( ucCurrent, 850 ) )
           {
             usPos++;
             usCol++;
             pucOutLastSpace = pucOut - 1;

           } /* endif */

           if ( fShiftOut )
           {
             /*****************************************************/
             /* in ShiftOut mode every character is encoded as    */
             /* three bytes...                                    */
             /*****************************************************/
             if ( usDataLen && (usPos < usLineSize) )
             {
              *pucOut++ = *pucIn++;
              usDataLen--;
              usPos++;
             } /* endif */

             if ( usDataLen && (usPos < usLineSize) )
             {
              *pucOut++ = *pucIn++;
              usDataLen--;
              usPos++;
             } /* endif */
           
           } /* endif */

           if ( (usPos >= usLineSize ) ||
                (usCol >= ulMaxColumns) )
           {
              fLineFull = TRUE;       // set line complete flag
              if ( pucInLastSpace )   // space position available?
              {
                 // position to last space character
                 pucIn  = pucInLastSpace;
                 pucOut = pucOutLastSpace;
                 usDataLen = usDataLenLastSpace;
              } /* endif */
           }
           else
           {
              if ( ucCurrent == ' ' )
              {
                 pucInLastSpace     = pucIn - 1;
                 pucOutLastSpace    = pucOut - 1;
                 usDataLenLastSpace = usDataLen + 1;
              } /* endif */
           } /* endif */

        } /* endif */
      } /* endwhile */

      // remove any whitespace characters at beginning of next line
      while ( usDataLen &&
              ((*pucIn == '\n') || (*pucIn == '\r') || (*pucIn == ' ')))
      {
         pucIn++;
         usDataLen--;
      } /* endif */

      // terminate current line
      *pucOut = EOS;

      // set data pointer and caller's usDataLen
      *ppData = pucIn;
      *pusDataLen = usDataLen;
   } /* endif */

   return( fMoreData );
} /* end of BuildLine */


/**********************************************************************/
/* Get the window position in coordinates relative to the parent      */
/* window                                                             */
/**********************************************************************/
BOOL GetRelativeWindowPos( HWND hwndControl, SWP FAR *pSwp )
{
  HWND             hwndParent;         // parent window of control
  POINT            pt;                 // point for coordinate conversions
  RECT             rect;               // buffer for control rectangle

  hwndParent = GetParent( hwndControl );

  /********************************************************************/
  /* Get control rectangle in screen coordinates                      */
  /********************************************************************/
  GetWindowRect( hwndControl, &rect );

  /********************************************************************/
  /* Convert control position to relative coordinates                 */
  /********************************************************************/
  pt.x = rect.left;
  pt.y = rect.top;
  ScreenToClient( hwndParent, &pt );

  /********************************************************************/
  /* Fill caller's size and position structure                        */
  /********************************************************************/
  pSwp->x = (SHORT)pt.x;
  pSwp->y = (SHORT)pt.y;
  pSwp->cx = (SHORT)(rect.right - rect.left);
  pSwp->cy = (SHORT)(rect.bottom - rect.top);

  return( TRUE );
} /* end of function GetRelativeWindowPos */

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
                               UtlQueryULong( QL_PELSPERLINE ) * 10,
                               hwndDlg, (HMENU)iEntryFieldID, (HINSTANCE)(HAB)UtlQueryULong( QL_HAB ), 0 );

  if ( hwndEntryField != NULL )
  {
    SetWindowLong( hwndEntryField, GWL_ID, iEntryFieldID );
    SetWindowPos( hwndEntryField, HWND_TOP, 0, 0, Placement.rcNormalPosition.right - Placement.rcNormalPosition.left,
                  UtlQueryULong( QL_PELSPERLINE ) * 10, SWP_NOACTIVATE | SWP_NOMOVE );
  }

  return( hwndEntryField != NULL );
}
