/*! \file
	Description: EQF Folder Handler Global Find and Replace (GFR) function

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
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

#include "eqfgfr.h"             // golbal find and replace defines

// color text names (in same order as base entry defines plus subtext defines)
PSZ pszGFRTextTypes[] = {"Normal text", "Found text", "To be changed text", "Changed text", "Change-to text", "Changed-to text", "Additional segment data text", 
  "Normal text when entry is selected", "Found text when entry is selected", "To be changed text when entry is selected", "Changed text when entry is selected", 
  "Change-to text when entry is selected", "Changed-to text when entry is selected", "Additional segment data text when entry is selected", 
  "Normal text when entry has the input focus", "Found text when entry has the input focus", "To be changed text when entry has the input focus", "Changed text when entry has the input focus", 
  "Change-to text when entry has the input focus", "Changed-to text when entry has the input focus", "Additional segment data text when entry has the input focus", 
  NULL };

// default foreground and background colors (in same order as texttype names)
//                                   Normal            Found             To be changed     Changed           Change-to         Changed-to        Add.segment data
COLORREF aclrGFRDefaultForeground[] = { RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),       RGB(80,80,80),      // normal element
                                     RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),       RGB(80,80,80),      // selected element 
                                     RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),       RGB(80,80,80) };    // focus element 
COLORREF aclrGFRDefaultBackground[] = { RGB(255,255,255), RGB(150,150,255), RGB(255,150,150), RGB(255,80,80),   RGB(150,255,150), RGB(100,255,100), RGB(255,255,255),   // normal element
                                     RGB(230,230,230), RGB(150,150,255), RGB(255,150,150), RGB(255,80,80),   RGB(150,255,150), RGB(100,255,100), RGB(230,230,230),   // selected element 
                                     RGB(230,230,255), RGB(150,150,255), RGB(255,150,150), RGB(255,80,80),   RGB(150,255,150), RGB(100,255,100), RGB(230,230,255) }; // focus element 



// table containing find expression operators (has to be terminated with an empty element)
FOLFINDOPERATOR FolFindOperators[] =
{
  { L"AND",         FF_AND_OP },
  { L"OR",          FF_OR_OP },
  { L"NOT",         FF_NOT_OP },
  { L"",            FF_EMPTY_OP }
};

// prototypes
void GFR_PrepareSearchForBatchEntry( PFOLFINDDATA pIda, PFOLFINDBATCHLISTENTRY pEntry );

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     FolGlobFindChange
//+----------------------------------------------------------------------------+
BOOL FolGlobFindChange
(
  PSZ              pszFolObjName,      // folder object name
  BOOL             fFromFolderList,    // TRUE = called from folder list
  BOOL             fMultipleObjs       // TRUE = called with a list of folder object names
)
{
  BOOL        fOK = TRUE;              // internal O.K. flag
  PFOLFINDDATA pIda;                    // ptr to IDA of dialog
  HWND        hwndFolFindDlg;          // handle of global-find-and-change dialog

  /********************************************************************/
  /* Allocate IDA of Global Find and Change dialog                    */
  /********************************************************************/
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof(FOLFINDDATA), ERROR_STORAGE );

  /********************************************************************/
  /* Fill-in IDA fields                                               */
  /********************************************************************/
  if ( fOK )
  {
    pIda->fFromFolderList = fFromFolderList;
    pIda->fMultipleObjs = fMultipleObjs;
    if ( pIda->fMultipleObjs )
    {
      pIda->pszObjList = pszFolObjName;
      pIda->pszActiveFolder = pIda->pszObjList;
      strcpy( pIda->szSubFolObjName, pIda->pszActiveFolder );
    }
    else
    {
      strcpy( pIda->szSubFolObjName, pszFolObjName );
    } /* endif */

    if ( FolIsSubFolderObject( pIda->szSubFolObjName ) )
    {
      // convert pszFolObjName to folder object name by cutting the
      // subfolder part and property part from the name
      strcpy( pIda->szFolObjName, pszFolObjName );
      UtlSplitFnameFromPath( pIda->szFolObjName );
      UtlSplitFnameFromPath( pIda->szFolObjName );
    }
    else
    {
      // pszFolObjName contains the folder object name already
      strcpy( pIda->szFolObjName, pIda->szSubFolObjName );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Start Global Find and Change dialog                              */
  /********************************************************************/
  if ( fOK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      hwndFolFindDlg = CreateMDIDialogParam( hResMod,
                                             MAKEINTRESOURCE(ID_FOLFIND_DLG),
                                             (HWND)UtlQueryULong( QL_TWBCLIENT ),
                                             (FARPROC)FolFindDlgProc,
                                             MP2FROMP(pIda), TRUE,
                                             (HPOINTER) UtlQueryULong(QL_DICTENTRYDISPICO)); //hiconDICTDISP );

  } /* endif */

  return( fOK );
} /* end of function FolGlobFindChange */


MRESULT FolFindPassToAllTabs( PFOLFINDDATA pIda, WINMSG msg, WPARAM mp1, LPARAM mp2 )
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure

  mp1; mp2; 

    // pass command to all tabs
  int iItem = 0;
  while ( pIda->hwndPages[iItem] )
  {
    PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ iItem ], DWL_DLGPROC );
    mResult = pfnWp( pIda->hwndPages[iItem], msg, 0, 0L);
    iItem++;
  } /* endwhile */

  return( mResult );
}

MRESULT EXPENTRY FolFindDlgProc( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 )
{
   MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
   LONG     lTabCtrl;
   PFOLFINDDATA pIda;
   BOOL     fOK = TRUE;

   switch ( msg )
   {
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_FOLFIND_DLG, mp2 ); break;

      case WM_INITDLG:
        {
          BOOL fLastUsedPositionAvailable = FALSE;
          pIda = (PFOLFINDDATA)PVOIDFROMMP2(mp2);
          ANCHORDLGIDA( hwnd, pIda );
          pIda->hwnd = hwnd;

          // get dialog position and size
          WinQueryWindowPos( hwnd, &pIda->swpDlg );

          // build unique dialog object name
          sprintf( pIda->szObjName, "FOLFIND:%s", pIda->szFolObjName );

          // register dialog
          EqfRegisterObject( pIda->szObjName, hwnd, clsFOLFIND );
          pIda->fRegistered = TRUE;

          UtlRegisterModelessDlg( hwnd );

          // get last used values
          if ( fOK ) fOK = GFR_GetLastUsedValues( pIda );

          // add folder name(s) to the title bar
          strcpy( pIda->szStatusLine, "Global Find and Replace" );   
          if ( pIda->pszObjList )
          {
            // add list of folders
            PSZ pszFolObjName = pIda->pszObjList;
            strcat( pIda->szStatusLine, " [folders: " );
            int iRoomLeft = min( sizeof(pIda->szStatusLine), 160 ) - strlen(pIda->szStatusLine) - 6;
            BOOL fFirstFolder = TRUE;
            while ( iRoomLeft && *pszFolObjName )
            {
              if ( fFirstFolder )
              {
                fFirstFolder = FALSE;
              }
              else
              {
                // insert separator
                strcat( pIda->szStatusLine, ", " );
                iRoomLeft = (iRoomLeft > 2) ? (iRoomLeft - 2) : 0;
              } /* endif */

              SubFolObjectNameToName( pszFolObjName, pIda->szAlias );

              int iFolderNameLen = strlen( pIda->szAlias );
              if ( iFolderNameLen > iRoomLeft )
              {
                strcat( pIda->szStatusLine, "..." );
                iRoomLeft = 0;
              }
              else
              {
                strcat( pIda->szStatusLine, pIda->szAlias );
                iRoomLeft -= iFolderNameLen;
              }
              pszFolObjName += strlen(pszFolObjName) + 1;
            } /*endwhile */
            strcat( pIda->szStatusLine, "]" );
          }
          else
          {
            SubFolObjectNameToName( pIda->szFolObjName, pIda->szAlias  );
            sprintf( pIda->szStatusLine + strlen(pIda->szStatusLine), " [folder: %s]", pIda->szAlias );
          } /* endif */
          SetWindowText( hwnd, pIda->szStatusLine );

          // start our search thread
          if ( fOK ) GlobalFindStartThread( pIda );

          // create status bar
          if ( fOK )
          {
            int statwidths[] = {300, 400, -1};

            pIda->hStatus = CreateWindowEx( 0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
                                            hwnd, (HMENU)ID_FOLFIND_STATUSBAR, GetModuleHandle(NULL), NULL);

            SendMessage( pIda->hStatus, SB_SETPARTS, 3, (LPARAM)statwidths);

            SendMessage( pIda->hStatus, SB_SETTEXT, 0, (LPARAM)"0 entries of 0 selected");
            ShowWindow( pIda->hStatus, SW_SHOW);
          }

          // Create invisible listbox for document names                
          if ( fOK )
          {
            pIda->hwndLB = WinCreateWindow( hwnd, WC_LISTBOX, "", 0L, 0, 0, 0, 0, hwnd, HWND_TOP, 1, NULL, NULL );
          } /* endif */

          // load our tab dialogs
          if ( fOK ) fOK = GFR_PropertySheetLoad( hwnd, pIda );

          // fill color data area with default values
          pIda->ColorData.hwndOwner = hwnd;
          pIda->ColorData.sID = 0;
          strcpy( pIda->ColorData.szTitle, "Set Global Find & Replace Colors" );
          int i = 0;
          while( pszGFRTextTypes[i] != NULL ) 
          {
            strcpy( pIda->ColorData.ColorSetting[i].szElement, pszGFRTextTypes[i] );
            pIda->ColorData.ColorSetting[i].cForeground = aclrGFRDefaultForeground[i];
            pIda->ColorData.ColorSetting[i].cDefaultForeground = aclrGFRDefaultForeground[i];
            pIda->ColorData.ColorSetting[i].cBackground = aclrGFRDefaultBackground[i];
            pIda->ColorData.ColorSetting[i].cDefaultBackground = aclrGFRDefaultBackground[i];
            i++;
          } /* endwhile */

          // set any user defined colors
          if ( pIda->pLastUsed && (pIda->pLastUsed->aclrBackground[0] != 0) && (pIda->pLastUsed->aclrForeground[0] != 0) )
          {
              int i = 0;
              while( pIda->ColorData.ColorSetting[i].szElement[0] != EOS )
              {
                pIda->ColorData.ColorSetting[i].cForeground = pIda->pLastUsed->aclrForeground[i];
                pIda->ColorData.ColorSetting[i].cBackground = pIda->pLastUsed->aclrBackground[i];
                i++;
              } /* endwhile */
          } /* endif */

          // get handles and sizes of dialog controls
          if ( fOK )
          {
            int i = 0;

            // get handles and sizes at bottom of dialog window
            pIda->hwndButton[0] = GetDlgItem( hwnd, ID_FOLFIND_FIND_PB );
            pIda->hwndButton[1] = GetDlgItem( hwnd, ID_FOLFIND_CHANGE_PB );
            pIda->hwndButton[2] = GetDlgItem( hwnd, ID_FOLFIND_STOP_PB );
            pIda->hwndButton[3] = GetDlgItem( hwnd, ID_FOLFIND_OPEN_PB );
            pIda->hwndButton[4] = GetDlgItem( hwnd, ID_FOLFIND_CANCEL_PB );
            pIda->hwndButton[5] = GetDlgItem( hwnd, ID_FOLFIND_HELP_PB );
            for ( i = 0; i < NUM_OF_GFR_BUTTONS; i++ )
            {
              RECT rect;
              GetWindowRect( pIda->hwndButton[i], &rect );
              pIda->sButtonWidth[i] = (SHORT)(rect.right - rect.left);
              if ( i == 0 ) pIda->sButtonHeight = (SHORT)(rect.bottom - rect.top);
            } /* endfor */
          } /* endif */

          // use remembered size and position (if any available)
          if ( fOK )
          {
            SWP swpButton;

            if ( pIda->pLastUsed->swpFolFindSizePos.cx != 0 )
            {
              EQFSWP2SWP( pIda->pLastUsed->swpFolFindSizePos, pIda->swpDlg );
              fLastUsedPositionAvailable = TRUE;
            }
            else
            {
              memset( &(pIda->swpDlg), 0, sizeof(pIda->swpDlg) );
              memset( &pIda->pLastUsed->swpFolFindSizePos, 0, sizeof(pIda->pLastUsed->swpFolFindSizePos) );
              WinQueryWindowPos( hwnd, &(pIda->swpDlg) );
            } /* endif */
          }

          // prepare font information
          {
            // this code has been borrowed from function SetCtrlFont in file EQFOSWIN.C
            HFONT   hFontDlg;

            hFontDlg = (HFONT)SendMessage( hwnd, WM_GETFONT, 0, 0L );
            if ( hFontDlg != NULL )
            {
              if ( GetObject( hFontDlg, sizeof(LOGFONT), (LPSTR) &(pIda->lf) ) )
              {
                pIda->lf.lfCharSet  = (UCHAR)GetCharSet();
                if (pIda->lf.lfHeight > 0 )
                {
                  pIda->lf.lfHeight -=  SHEIGHTINCTRL;
                }
                else
                {
                  pIda->lf.lfWeight = FW_NORMAL;
                } /* endif */
                pIda->lf.lfOutPrecision = OUT_TT_PRECIS;
                strcpy( pIda->lf.lfFaceName, "Arial" );
              }
            }
          }

          // apply last used font info when available
          if ( pIda->pLastUsed->lfFaceName[0] != 0 )
          {
            // get last used font settings
            memset( &(pIda->lf), 0, sizeof(pIda->lf) );
            pIda->lf.lfHeight = pIda->pLastUsed->lfHeight;
            pIda->lf.lfWidth = pIda->pLastUsed->lfWidth;
            pIda->lf.lfEscapement = pIda->pLastUsed->lfEscapement;
            pIda->lf.lfOrientation = pIda->pLastUsed->lfOrientation;
            pIda->lf.lfWeight = pIda->pLastUsed->lfWeight;
            pIda->lf.lfItalic = pIda->pLastUsed->lfItalic;
            pIda->lf.lfUnderline = pIda->pLastUsed->lfUnderline;
            pIda->lf.lfStrikeOut = pIda->pLastUsed->lfStrikeOut;
            pIda->lf.lfCharSet = pIda->pLastUsed->lfCharSet;
            pIda->lf.lfOutPrecision = pIda->pLastUsed->lfOutPrecision;
            pIda->lf.lfClipPrecision = pIda->pLastUsed->lfClipPrecision;
            pIda->lf.lfQuality = pIda->pLastUsed->lfQuality;
            pIda->lf.lfPitchAndFamily = pIda->pLastUsed->lfPitchAndFamily;
            strcpy( pIda->lf.lfFaceName, pIda->pLastUsed->lfFaceName );
          } /* endif */

          // create font to be used for result area
          pIda->hFontControl = CreateFontIndirect( &(pIda->lf) );


          // Center dialog within TWB
          if ( fOK && !fLastUsedPositionAvailable )
          {
            SWP swpTWB;
            WinQueryWindowPos( (HWND)UtlQueryULong( QL_TWBCLIENT ), &swpTWB );
            if ( (pIda->swpDlg.x > 0) && ((pIda->swpDlg.x + pIda->swpDlg.cx) < swpTWB.cx) )
            {
              pIda->swpDlg.x = (swpTWB.cx - pIda->swpDlg.cx) / 2;
            } /* endif */
            if ( (pIda->swpDlg.y > 0) && ((pIda->swpDlg.y + pIda->swpDlg.cy) < swpTWB.cy) )
            {
              pIda->swpDlg.y = (swpTWB.cy - pIda->swpDlg.cy) / 2;
            } /* endif */
          } /* endif */

          // postpone sizing of dialog window
          if ( fOK ) WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(INITDLG_TASK), 0L );

          // destroy window in case of errors
          if ( !fOK )
          {
            WinDestroyWindow( hwnd );
            return 0;
          } /* endif */
        }
        return 0;
        break;

    case WM_EQF_PROCESSTASK:
      {
        PFOLFINDDATA pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
        if ( (pIda != NULL) && (!pIda->fTerminate) )
        {
          switch ( SHORT1FROMMP1(mp1) )
          {
            case INITDLG_TASK :
              {
                HWND hwndFrame = GetParent( hwnd );

                // use position and size of result area for minimum dialog width
                {
                  RECT rect;

                  GetWindowRect( hwnd, &rect );
                  pIda->iMinDialogWidth = rect.right - rect.left;
                  pIda->iMinDialogHeight = rect.bottom - rect.top;
                }

                // ensure that dialog size does not fall below our minimum size
                // GQ 2015/08/20 disabled code below to fix P402963
                //if ( fOK )
                //{
                //  if ( pIda->swpDlg.cx < pIda->iMinDialogWidth )
                //  {
                //    pIda->swpDlg.cx = pIda->iMinDialogWidth;
                //  } /* endif */

                //  if ( pIda->swpDlg.cy < pIda->iMinDialogHeight )
                //  {
                //    pIda->swpDlg.cy = pIda->iMinDialogHeight;
                //  } /* endif */
                //}


                // ensure that dialog is in visible screen area
                if ( fOK )
                {
                  // the following computation is based on old OS/2 windows coordiantes (0,0 = lower left corner)

                  int cxScreen = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
                  int cyScreen = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

                  if ( (pIda->swpDlg.cy + pIda->swpDlg.y) > cyScreen )
                  {
                    pIda->swpDlg.y = (SHORT)(cyScreen - pIda->swpDlg.cy);
                  }
                  if ( (pIda->swpDlg.cx + pIda->swpDlg.x) > cxScreen )
                  {
                    pIda->swpDlg.x = (SHORT)(cxScreen - pIda->swpDlg.cx);
                  }

                } /* endif */

                WinSetWindowPos( hwndFrame, HWND_TOP, pIda->swpDlg.x, pIda->swpDlg.y, pIda->swpDlg.cx, pIda->swpDlg.cy, EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_SHOW | EQF_SWP_ACTIVATE );
                PostMessage( pIda->hwndPages[0], WM_EQF_PROCESSTASK, INITDLG_TASK, 0 );
              }
              break;
            case SIZEDLG_TASK :
              {
                HWND hwndFrame = GetParent( hwnd );
                WinSetWindowPos( hwndFrame, HWND_TOP, pIda->swpLastValidSizePos.x, pIda->swpLastValidSizePos.y, 
                  (pIda->swpLastValidSizePos.cx < pIda->iMinDialogWidth) ? pIda->iMinDialogWidth : pIda->swpLastValidSizePos.cx, 
                  (pIda->swpLastValidSizePos.cy < pIda->iMinDialogHeight) ? pIda->iMinDialogHeight : pIda->swpLastValidSizePos.cy, EQF_SWP_SIZE | EQF_SWP_MOVE );
              }
              break;
          } /* endswitch */
        } /* endif */
      }
      break;

      //case WM_COMMAND:
      //   mResult = AnaPropCommand( hwndDlg, mp1, mp2 );
      //   break;

    case WM_NOTIFY:
        {
          PFOLFINDDATA pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
          switch(((LPNMHDR)mp2)->code)
          {
            case TCN_SELCHANGING: 
            {
              TC_ITEM Item;
              pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA);
              lTabCtrl = TabCtrl_GetCurSel( pIda->hwndTabCtrl );
              memset( &Item, 0, sizeof(Item) );
              Item.mask = TCIF_PARAM;
              TabCtrl_GetItem( pIda->hwndTabCtrl, lTabCtrl, &Item );
              lTabCtrl = Item.lParam;
              ShowWindow( pIda->hwndPages[ lTabCtrl ], SW_HIDE );
              return FALSE; // allow change of selection
            }
            case TCN_SELCHANGE: 
            {
              TC_ITEM Item;
              pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA);
              lTabCtrl = TabCtrl_GetCurSel( pIda->hwndTabCtrl );
              memset( &Item, 0, sizeof(Item) );
              Item.mask = TCIF_PARAM;
              TabCtrl_GetItem( pIda->hwndTabCtrl, lTabCtrl, &Item );
              lTabCtrl = Item.lParam;
              ShowWindow( pIda->hwndPages[ lTabCtrl ], SW_SHOW );
              if ( lTabCtrl == 0 )
              {
                // interactive find&replace tab is shown
                ENABLECTRL( hwnd, ID_FOLFIND_FIND_PB, !pIda->fSearching  );
                ENABLECTRL( hwnd, ID_FOLFIND_CHANGE_PB, !pIda->fSearching  );
                ENABLECTRL( hwnd, ID_FOLFIND_STOP_PB, pIda->fSearching  );
                GFR_SetEnableStateResultListButtons( pIda );
              }
              else if ( lTabCtrl == 1 )
              {
                // batch list maintenance tab is shown
                ENABLECTRL( hwnd, ID_FOLFIND_FIND_PB, FALSE );
                ENABLECTRL( hwnd, ID_FOLFIND_CHANGE_PB, FALSE );
                ENABLECTRL( hwnd, ID_FOLFIND_OPEN_PB, FALSE );
                ENABLECTRL( hwnd, ID_FOLFIND_STOP_PB, FALSE );
              }
              return TRUE;
            }
          }
          return TRUE;          
        }
        break;

    case WM_CLOSE:
      {
        PFOLFINDDATA pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
        pIda->fTerminate = TRUE;
        FolFindCloseDoc( pIda, TRUE );

        // pass command to all tabs
        FolFindPassToAllTabs( pIda, WM_CLOSE, 0, 0 );

        // copy current color setting to last used values
        {
            int i = 0;
            while( pIda->ColorData.ColorSetting[i].szElement[0] != EOS )
            {
              pIda->pLastUsed->aclrForeground[i] = pIda->ColorData.ColorSetting[i].cForeground;
              pIda->pLastUsed->aclrBackground[i] = pIda->ColorData.ColorSetting[i].cBackground;
              i++;
            } /* endwhile */
        } 

        // save last used values
        {
          UtlSaveWindowPos( GetParent( hwnd ), &(pIda->pLastUsed->swpFolFindSizePos) );
          GFR_SaveLastUsedValues( pIda );
        }
        if ( pIda->fRegistered )
        {
          EqfRemoveObject( TWBFORCE, hwnd );
          pIda->fRegistered = FALSE;
        } /* endif */
        SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ), WM_MDIDESTROY, MP1FROMHWND(hwnd), 0L ) ;
        return 0;
      }
      break;

  case WM_GETMINMAXINFO:
    {
      MINMAXINFO FAR *lpMinMax = (MINMAXINFO *)PVOIDFROMMP2(mp2);
      pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
      lpMinMax->ptMinTrackSize.x = pIda->iMinDialogWidth;
      lpMinMax->ptMinTrackSize.y = pIda->iMinDialogHeight;
    }

   case WM_SIZE :
     pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
     // resize inner window only if normal sizing request...
     if ( (pIda != NULL) && ((mp1 == SIZENORMAL) || (mp1 == SIZEFULLSCREEN)) )
     {
       SHORT   sWidth  = LOWORD( mp2 );      // new width of dialog
       SHORT   sHeight = HIWORD( mp2 );      // new height of dialog
       LONG lStatusLineHeight = 0;   // height of status line
       LONG   lBorderSize  = WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER);
       LONG   cxAvail = sWidth - (2 * lBorderSize);


       // try to avoid windows smaller as our minimum size
       //int iMinX = pIda->iMinDialogWidth;
       //int iMinY = pIda->iMinDialogHeight;
       //if ( (sWidth < pIda->iMinDialogWidth) || (sHeight < pIda->iMinDialogHeight) )
       //{
       //  // resize dialog to its last valid dialog position and size
       //  WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SIZEDLG_TASK), 0L );
       //  return( 0 );
       //}

       // save current size and position as last valid window size and position
       UtlSaveWindowPos( GetParent( hwnd ), &(pIda->swpLastValidSizePos) );

       // get height of status bar
       {
         RECT rcStatus;
         GetWindowRect( pIda->hStatus, &rcStatus );
         lStatusLineHeight = rcStatus.bottom - rcStatus.top;
       }


       // re-arrange controls
       {
         LONG  lXPos;                  // current x (horizontal) position
         LONG  lYPos;                  // current y (vertical) position (starting from bottom to top!)

         int i;
         {
           // we re-position/re-size all dialog controls ...
           HDWP hdwp = BeginDeferWindowPos( 11 );

           // re-arrange status bar
           SendMessage( pIda->hStatus, WM_SIZE, 0, 0);

           // re-arrange pushbuttons
           {
             LONG  lGap;                   // gap between pushbuttons
             LONG  lTotGaps;               // total size of gaps between pushbuttons
             LONG  lTotSize = 0;           // total width of pushbuttons
             LONG  lCorrect;               // correction value

             for ( i = 0; i < NUM_OF_GFR_BUTTONS; i++ )
             {
               lTotSize += pIda->sButtonWidth[i];
             } /* endfor */

             lTotGaps = (cxAvail > lTotSize) ? (cxAvail - lTotSize) : 0;
             lGap = lTotGaps / NUM_OF_GFR_BUTTONS;
             lCorrect = (cxAvail > lTotSize) ? ((cxAvail - (lGap * 6) - lTotSize) / 2) : 0;
             lXPos    = pIda->sBorderSize;
             lYPos    = sHeight - pIda->sBorderSize - pIda->sButtonHeight - lStatusLineHeight - 4;
             for ( i = 0; (i < NUM_OF_GFR_BUTTONS) && (hdwp != NULL); i++ )
             {
               lXPos += i ? (pIda->sButtonWidth[i-1] + lGap) : ((lGap / 2) + lCorrect);
               hdwp = DeferWindowPos( hdwp, pIda->hwndButton[i], HWND_TOP, lXPos, lYPos, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             } /* endfor */
           }
           // use remaining room left in dialog for the tab control
           {
             RECT rect;
             GetWindowRect( pIda->hwndTabCtrl, &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, pIda->hwndTabCtrl, HWND_TOP, 0, 0, cxAvail, lYPos - 4 - rect.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
           }

           // do actual dialog control re-positioning
           if ( hdwp != NULL )
           {
             EndDeferWindowPos( hdwp );
           } /* endif */

           // adjust status bar 
           {
             int iWidths[3];

             iWidths[0] = cxAvail / 3;
             iWidths[1] = iWidths[0];
             iWidths[2] = -1;
             SendMessage( pIda->hStatus, SB_SETPARTS, 3, (LPARAM)iWidths );
           }

           // resize the dialogs of our tab pages
           {
             RECT rect;
             int i = 0;
             GetClientRect( pIda->hwndTabCtrl, &rect );
             TabCtrl_AdjustRect( pIda->hwndTabCtrl, FALSE, &rect );
             rect.top += 20;
             MapWindowPoints( pIda->hwndTabCtrl, hwnd, (POINT *) &rect, 2 );
             while( pIda->hwndPages[i] )
             {
               SetWindowPos( pIda->hwndPages[i], HWND_TOP, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER ); 
               i++;
             }
           }
         }
       }
     } /* endif */
     break;

  case WM_COMMAND:
    {
      PFOLFINDDATA pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case ID_FOLFIND_HELP_PB:
          UtlInvokeHelp();
          break;

        case ID_FOLFIND_FIND_PB:
        case ID_FOLFIND_CHANGE_PB:
        case ID_FOLFIND_OPEN_PB:
        case ID_FOLFIND_STOP_PB:
          // pass command message to interactive find&replace tab
          {
            PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[0], DWL_DLGPROC );
            mResult = pfnWp( pIda->hwndPages[0], WM_COMMAND, mp1, mp2 );
          }
          break;

        case DID_CANCEL:
          // do not close the dialog when the on-spot editing in the batch list is active, just end on-spot editing instead
          if ( GFR_IsOnSpotEditingActive() ) 
          {
            GFR_EndOnSpotEditing();
          }
          else
          {
            pIda->fTerminate = TRUE;
            WinPostMsg( hwnd, WM_CLOSE, 0L, 0 );
          }
          break;

        case ID_FOLFIND_CANCEL_PB:
          pIda->fTerminate = TRUE;
          WinPostMsg( hwnd, WM_CLOSE, 0L, 0 );
          break;
       } /* endswitch */
     } 
     break;

    case WM_EQF_INITMENU:
    case WM_INITMENU:
    case WM_INITMENUPOPUP:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

    case WM_EQF_TOOLBAR_ENABLED:
      break;


    case WM_DESTROY:
      pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
      if (pIda)
      {
        // stop our thread
        pIda->fStopThread = TRUE;

        // wait until thread has terminated
        {
          int i = 10;
          while ( (i != 0) && pIda->fThreadIsRunning )
          {
            Sleep( 100 );
            i--;
          }
        }

        if ( pIda->hwndLB ) WinDestroyWindow( pIda->hwndLB );

        if ( pIda->hFontControl ) DeleteObject( pIda->hFontControl );

        if ( pIda->fRegistered )
        {
          EqfRemoveObject( TWBFORCE, hwnd );
          pIda->fRegistered = FALSE;
        } /* endif */

        if ( pIda->pLastUsed ) UtlAlloc ((PVOID *)&(pIda->pLastUsed), 0L, 0L, NOMSG);
        UtlAlloc ((PVOID *)&pIda, 0L, 0L, NOMSG);
      } /* endif */
      LOGWRITESTRING( "*** Ending Dialog ***" );
      LOGEND();
      UtlUnregisterModelessDlg( hwnd );
      break;

      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */

   return mResult;
} /* end of FolFindDlgProc */

BOOL GFR_PropertySheetLoad( HWND hwnd, PFOLFINDDATA pIda )
{
  TC_ITEM   TabCtrlItem;
  HWND      hwndTabCtrl;
  int i = 0;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  RECT rect;
  HWND hwndTabWindow;

  // remember adress of user area
  hwndTabCtrl = GetDlgItem( hwnd, ID_FOLFIND_TABCTRL );
  pIda->hwndTabCtrl = hwndTabCtrl;
  GetClientRect( hwndTabCtrl, &rect );
  TabCtrl_AdjustRect( hwndTabCtrl, FALSE, &rect );

  // leave some additional space at top
  rect.top += 20;
  MapWindowPoints( hwndTabCtrl, hwnd, (POINT *) &rect, 2 );


  TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM;

  // create the interactive find and Replace TAB and load the associated dialog
  TabCtrlItem.pszText = "Interactive Find && Replace";
  i = 0;
  TabCtrlItem.lParam = i;
  SendMessage(hwndTabCtrl, TCM_INSERTITEM, i, (LPARAM)&TabCtrlItem);
  hwndTabWindow = pIda->hwndPages[i] = CreateDialogParam( hResMod, MAKEINTRESOURCE( ID_FOLFIND_INTERACTIVE_TAB ), hwnd, GFR_INTERACTIVE_DLGPROC, (LPARAM)pIda ); 
  SetWindowPos( pIda->hwndPages[i], HWND_TOP, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
  SetFocus( pIda->hwndPages[i] );
  UtlRegisterModelessDlg( pIda->hwndPages[i] );

  // create the batch list maintenance TAB and load the associated dialog
  TabCtrlItem.pszText = "Batch List Maintenance";
  i = 1;
  TabCtrlItem.lParam = i;
  SendMessage(hwndTabCtrl, TCM_INSERTITEM, i, (LPARAM)&TabCtrlItem);
  hwndTabWindow = pIda->hwndPages[i] = CreateDialogParam( hResMod, MAKEINTRESOURCE( ID_FOLFIND_BATCHLIST_TAB ), hwnd, GFR_BATCHLIST_DLGPROC, (LPARAM)pIda ); 
  SetWindowPos( pIda->hwndPages[i], HWND_TOP, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
  SetFocus( pIda->hwndPages[i] );
  UtlRegisterModelessDlg( pIda->hwndPages[i] );

  // hide all dialog pages but the first one
  i = 1;
  while ( pIda->hwndPages[i] )
  {
    ShowWindow( pIda->hwndPages[i], SW_HIDE );
    i++;
  } /* endwhile */

  return( TRUE );
}




// Dialog procesure for interactive find&change
INT_PTR CALLBACK GFR_INTERACTIVE_DLGPROC
(
   HWND hwnd,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  PFOLFINDDATA pIda = NULL;             // ptr to IDA of dialog
  BOOL        fOK;                     // internal O.K. flag

  switch (msg)
  {

    case WM_INITDLG:
      {
        fOK = TRUE;

        LOGSTART();
        LOGWRITESTRING( "*** Init Dialog ***" );

        // Anchor IDA 
        pIda = (PFOLFINDDATA)PVOIDFROMMP2(mp2);
        ANCHORDLGIDA( hwnd, pIda );

        // replace find and replace combobox by their Unicode enabled counterpart
        GFR_ReplaceWithUnicodeComboBox( hwnd, ID_FOLFIND_FINDSOURCE_EF );
        GFR_ReplaceWithUnicodeComboBox( hwnd, ID_FOLFIND_CHANGE_EF );
        GFR_ReplaceWithUnicodeComboBox( hwnd, ID_FOLFIND_FIND_EF );

        // correct tab order of controls (is out of order after replacing the comboboxes...)
        HWND hwndInsertBehind = HWND_TOP;
        HWND hwndNext = GetDlgItem( hwnd, ID_FOLFIND_FIND_EF );
        SetWindowPos( hwndNext, hwndInsertBehind, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE );
        hwndInsertBehind = hwndNext;
        hwndNext = GetDlgItem( hwnd, ID_FOLFIND_CHANGE_EF );
        SetWindowPos( hwndNext, hwndInsertBehind, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE );
        hwndInsertBehind = hwndNext;
        hwndNext = GetDlgItem( hwnd, ID_FOLFIND_FINDSOURCE_CHK );
        SetWindowPos( hwndNext, hwndInsertBehind, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE );
        hwndInsertBehind = hwndNext;
        hwndNext = GetDlgItem( hwnd, ID_FOLFIND_FINDSOURCE_EF );
        SetWindowPos( hwndNext, hwndInsertBehind, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE );
        hwndInsertBehind = hwndNext;
        hwndNext = GetDlgItem( hwnd, ID_FOLFIND_APPLYBATCH_CHK );
        SetWindowPos( hwndNext, hwndInsertBehind, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE );


        pIda->hResultListBox = GetDlgItem( hwnd, ID_FOLFIND_RESULT_LISTBOX );
        if ( pIda->hFontControl != NULL ) SendMessage( pIda->hResultListBox, WM_SETFONT, (WPARAM)pIda->hFontControl, MP2FROMSHORT(TRUE) );

        // Set initial state of dialog controls
        if ( fOK )
        {
          ENABLECTRL( pIda->hwnd, ID_FOLFIND_OPEN_PB,    FALSE );
          ENABLECTRL( hwnd, ID_FOLFIND_FINDNEXT_PB,      FALSE );
          ENABLECTRL( hwnd, ID_FOLFIND_APPLY_PB,         FALSE );
          ENABLECTRL( pIda->hwnd, ID_FOLFIND_FIND_PB,    FALSE );
          ENABLECTRL( pIda->hwnd, ID_FOLFIND_CHANGE_PB,  FALSE );
          ENABLECTRL( hwnd, ID_FOLFIND_EXPORT_PB,        FALSE );
          ENABLECTRL( hwnd, ID_FOLFIND_APPLYCHANGE_PB,   FALSE );
          ENABLECTRL( hwnd, ID_FOLFIND_APPLYNEXT_PB,     FALSE );
          ENABLECTRL( hwnd, ID_FOLFIND_PREV_PB,          FALSE );
          ENABLECTRL( hwnd, ID_FOLFIND_NEXT_PB,          FALSE );
          ENABLECTRL( pIda->hwnd, ID_FOLFIND_STOP_PB,    FALSE );

          SETCHECK_TRUE( hwnd, ID_FOLFIND_IGNORE_RB );

          CBSETTEXTLIMIT( hwnd, ID_FOLFIND_FIND_EF, MAX_FINDCHANGE_LEN );
          CBSETTEXTLIMIT( hwnd, ID_FOLFIND_CHANGE_EF, MAX_FINDCHANGE_LEN );
          CBSETTEXTLIMIT( hwnd, ID_FOLFIND_FINDSOURCE_EF, MAX_FINDCHANGE_LEN );

          // preset wildcard fields with the defaults
          pIda->chWildCardMultChar = NONE_WILDCARD;
          pIda->chWildCardSingleChar = NONE_WILDCARD;
        } /* endif */

         // apply last used values
         if ( fOK )
         {
            SETCHECK_FALSE( hwnd, ID_FOLFIND_RESPECT_RB);
            SETCHECK_FALSE( hwnd, ID_FOLFIND_IGNORE_RB );

            if ( pIda->pLastUsed->fFolFindCaseRespect )
            {
              SETCHECK_TRUE( hwnd, ID_FOLFIND_RESPECT_RB );
            }
            else
            {
              SETCHECK_TRUE( hwnd, ID_FOLFIND_IGNORE_RB );
            } /* endif */
            SETCHECK( hwnd, ID_FOLFIND_MEMUPD_CHK, pIda->pLastUsed->fFolFindUpdateTM );
            SETCHECK( hwnd, ID_FOLFIND_TRANSLTEXT_CHK, pIda->pLastUsed->fFolFindTranslTextOnly );
            SETCHECK( hwnd, ID_FOLFIND_WHOLEWORD_CHK, pIda->pLastUsed->fFolFindWholeWordsOnly );
            SETCHECK( hwnd, ID_FOLFIND_FINDSOURCE_CHK, pIda->pLastUsed->fAndFindInSource );
            SETCHECK( hwnd, ID_FOLFIND_RESPECTLF_CHK, pIda->pLastUsed->fRespectLineFeeds );
            SETCHECK( hwnd, ID_FOLFIND_PRESERVECASE_CHK, pIda->pLastUsed->fPreserveCase );

            // Fill history in comboboxes
            GFR_RefreshComboboxHistory( hwnd, ID_FOLFIND_FIND_EF, pIda, pIda->pLastUsed->szFolFind, pIda->pLastUsed->szFindList );
            GFR_RefreshComboboxHistory( hwnd, ID_FOLFIND_CHANGE_EF, pIda, pIda->pLastUsed->szFolChangeTo, pIda->pLastUsed->szReplaceList );
            GFR_RefreshComboboxHistory( hwnd, ID_FOLFIND_FINDSOURCE_EF, pIda, pIda->pLastUsed->szAndFindInSource, pIda->pLastUsed->szAddFindList );

            // Fill "display segments before/after" combbox
            for( int i = 0; i <= 2; i++ )
            {
              char szNum[3];
              itoa( i, szNum, 10 );
              int iItem = CBINSERTITEMEND( hwnd, ID_FOLFIND_SEGSAROUND_CB, szNum );
              if ( pIda->pLastUsed->sShowBeforeAfter == i )
              {
                CBSELECTITEM( hwnd, ID_FOLFIND_SEGSAROUND_CB, iItem );
              }
            } /* endfor */

            // set other last-used values
            {
              switch( pIda->pLastUsed->sSearchIn )
              {
                case GFR_SEARCH_IN_SOURCE:
                  SETCHECK_TRUE( hwnd, ID_FOLFIND_SEARCHSOURCE_RB );
                  pIda->fSearchInSource = TRUE;
                  break;
                case GFR_SEARCH_IN_BOTH:
                  SETCHECK_TRUE( hwnd, ID_FOLFIND_SEARCHBOTH_RB );
                  pIda->fSearchInTarget = TRUE;
                  pIda->fSearchInSource = TRUE;
                  break;
                case GFR_SEARCH_IN_TARGET:
                default:
                  SETCHECK_TRUE( hwnd, ID_FOLFIND_SEARCHTARGET_RB );
                  pIda->fSearchInTarget = TRUE;
                  break;
              }

              // get last used wildcard characters
              if ( pIda->pLastUsed->chWildCardSingleChar ) pIda->chWildCardSingleChar = pIda->pLastUsed->chWildCardSingleChar;
              if ( pIda->pLastUsed->chWildCardMultChar )   pIda->chWildCardMultChar   = pIda->pLastUsed->chWildCardMultChar;
            }

            SETCHECK( hwnd, ID_FOLFIND_CONFCHANGES_CHK, pIda->pLastUsed->fFolFindConfirm );
            FolFindControl( hwnd, ID_FOLFIND_CONFCHANGES_CHK, 0 );

            // remember orinial size of options-and-documents group box and results group box
            GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_OPTDOCS_GB ), &(pIda->rcOrgOptsAndDocsGB) );
            MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&(pIda->rcOrgOptsAndDocsGB), 2 );
            GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_TEXT_GB ), &(pIda->rcOrgResultGB) );
            MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&(pIda->rcOrgResultGB), 2 );
         } /* endif */

        // fill wildcard character fields
        if ( fOK )
        {
          CHAR_W szTemp[20];
          PSZ_W pszWildCard = szWildCards;

          while ( *pszWildCard )
          {
            SHORT sItem;

            if ( *pszWildCard == NONE_WILDCARD )
            {
              wcscpy( szTemp, szNoneWildcard );
            }
            else
            {
              szTemp[0] = *pszWildCard;
              szTemp[1] = 0;
            } /* endif */

            sItem = CBINSERTITEMENDW( hwnd, ID_FOLFIND_WILDCARDSINGLE_CB , szTemp );
            if ( *pszWildCard == pIda->chWildCardSingleChar ) CBSELECTITEM( hwnd, ID_FOLFIND_WILDCARDSINGLE_CB, sItem );
            CBINSERTITEMENDW( hwnd, ID_FOLFIND_WILDCARDMULT_CB , szTemp );
            if ( *pszWildCard == pIda->chWildCardMultChar ) CBSELECTITEM( hwnd, ID_FOLFIND_WILDCARDMULT_CB, sItem );

            // next wildcard character
            pszWildCard++;
          } /*endwhile */
        } /* endif */

        if ( fOK )
        {
          GFR_AdjustResultColumns( hwnd, pIda );
        } /* endif */

        if ( fOK ) fOK = GFR_PrepFolderSearch( hwnd, pIda );

        return MRFROMSHORT(FALSE);
      }
      break;

    case WM_EQF_PROCESSTASK:
      {
        PFOLFINDDATA pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
        if ( (pIda != NULL) && (!pIda->fTerminate) )
        {
          switch ( SHORT1FROMMP1(mp1) )
          {
            case INITDLG_TASK:
              // postpone setting of the input focus
              {
                HWND hwndOldFocus = (HWND)PostMessage( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SETFOCUS_TASK), (LPARAM)GetDlgItem( hwnd, ID_FOLFIND_FINDSOURCE_EF ) );
                hwndOldFocus = (HWND)PostMessage( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SETFOCUS_TASK), (LPARAM)GetDlgItem( hwnd, ID_FOLFIND_CHANGE_EF  ) );
                hwndOldFocus = (HWND)PostMessage( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SETFOCUS_TASK), (LPARAM)GetDlgItem( hwnd, ID_FOLFIND_FIND_EF ) );
              }
              break;
            case SETFOCUS_TASK:
              SetFocus( (HWND)mp2 );
              break;
          } /* endswitch */
        } /* endif */
      }
      break;

    case WM_COMMAND:
      {
        PFOLFINDDATA pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
        switch ( WMCOMMANDID( mp1, mp2 ) )
        {
          case ID_FOLFIND_HELP_PB:
            UtlInvokeHelp();
            break;

          case DID_CANCEL:
            pIda->fTerminate = TRUE;
            WinPostMsg( hwnd, WM_CLOSE, 0L, 0 );
            break;

          case ID_FOLFIND_COLLAPSE_PB:
            {
              RECT rcCurrent;
              int iChangeInHeight = 0;

              // toggle collapse flag
              pIda->fCollapsed = !pIda->fCollapsed;

              // hide or show options and document list
              int iShowCmd =  pIda->fCollapsed ? SW_HIDE : SW_SHOW ;
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_WILDCARD_GB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_WILDCARDSINGLE_TEXT ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_WILDCARDSINGLE_CB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_WILDCARDMULT_TEXT ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_WILDCARDMULT_CB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_PARAMETERS_GB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_MEMUPD_CHK ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_CONFCHANGES_CHK ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_TRANSLTEXT_CHK ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_WHOLEWORD_CHK ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_PRESERVECASE_CHK ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_CASE_GB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_RESPECT_RB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_IGNORE_RB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_SEARCHIN_GB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_SEARCHTARGET_RB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_SEARCHSOURCE_RB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_SEARCHBOTH_RB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_DISPLAY_GB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_SHOWSOURCEANDTARGET_CHK ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_RESPECTLF_CHK ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_SHOW_GB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_COLOR_PB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_FONT_PB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_SEGSBEFORE_TEXT ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_SEGSAROUND_CB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_DOCS_GB ), iShowCmd );
              ShowWindow( GetDlgItem( hwnd, ID_FOLFIND_DOCS_LB ), iShowCmd );

              // re-size options-and-documents groupbox
              GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_OPTDOCS_GB ), &rcCurrent );
              MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rcCurrent, 2 );
              int iNewHeight = pIda->fCollapsed ? 16 : (pIda->rcOrgOptsAndDocsGB.bottom - pIda->rcOrgOptsAndDocsGB.top);
              iChangeInHeight = rcCurrent.bottom - rcCurrent.top - iNewHeight;
              SetWindowPos( GetDlgItem( hwnd, ID_FOLFIND_OPTDOCS_GB ), HWND_TOP, 0, 0, rcCurrent.right - rcCurrent.left, iNewHeight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

              // re-size result area
              GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_TEXT_GB ), &rcCurrent );
              MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rcCurrent, 2 );
              SetWindowPos( GetDlgItem( hwnd, ID_FOLFIND_TEXT_GB ), HWND_TOP, rcCurrent.left, rcCurrent.top - iChangeInHeight, 
                            rcCurrent.right - rcCurrent.left, rcCurrent.bottom - rcCurrent.top + iChangeInHeight, SWP_NOACTIVATE | SWP_NOZORDER );
              GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_HEADER_TEXT ), &rcCurrent );
              MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rcCurrent, 2 );
              SetWindowPos( GetDlgItem( hwnd, ID_FOLFIND_HEADER_TEXT ), HWND_TOP, rcCurrent.left, rcCurrent.top - iChangeInHeight, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
              GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_RESULT_LISTBOX ), &rcCurrent );
              MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rcCurrent, 2 );
              SetWindowPos( GetDlgItem( hwnd, ID_FOLFIND_RESULT_LISTBOX ), HWND_TOP, rcCurrent.left, rcCurrent.top - iChangeInHeight, 
                            rcCurrent.right - rcCurrent.left, rcCurrent.bottom - rcCurrent.top + iChangeInHeight, SWP_NOACTIVATE | SWP_NOZORDER );

              // change collapse button text
              SetWindowText( GetDlgItem( hwnd, ID_FOLFIND_COLLAPSE_PB ), pIda->fCollapsed ? "+" : "-" );
            }
            break;

          case ID_FOLFIND_EXPORT_PB:
            {
              if ( GFR_ExportResultList( hwnd, pIda ) == 0 )
              {
                SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)"List of search results has been exported" );
              }
            }
            break;

          case ID_FOLFIND_PREV_PB:
            {
              int iSelItem = QUERYSELECTIONHWND( pIda->hResultListBox );
              if ( (iSelItem != -1) && (iSelItem != 0) )
              {
                int iTopIndex = SendMessage( pIda->hResultListBox, LB_GETTOPINDEX, 0, 0 );
                if ( iTopIndex != 0 ) SendMessage( pIda->hResultListBox, LB_SETTOPINDEX, (WPARAM)(iTopIndex - 1), 0 );
                SendMessage( pIda->hResultListBox, LB_SETCURSEL, iSelItem - 1, 0 );
                GFR_SetEnableStateResultListButtons( pIda );
                GFR_UpdateSelectionStatus( pIda );
              } /* endif */
            }
            break;

          case ID_FOLFIND_NEXT_PB:
            {
              int iSelItem = QUERYSELECTIONHWND( pIda->hResultListBox );
              int iNumOfItems = QUERYITEMCOUNTHWND( pIda->hResultListBox );
              if ( (iSelItem != -1) && ((iSelItem + 1) < iNumOfItems) )
              {
                int iTopIndex = SendMessage( pIda->hResultListBox, LB_GETTOPINDEX, 0, 0 );
                if ( (iTopIndex + 1) < iNumOfItems ) SendMessage( pIda->hResultListBox, LB_SETTOPINDEX, (WPARAM)(iTopIndex + 1), 0 );
                SendMessage( pIda->hResultListBox, LB_SETCURSEL, iSelItem + 1, 0 );
                GFR_SetEnableStateResultListButtons( pIda );
                GFR_UpdateSelectionStatus( pIda );
              } /* endif */
            }
            break;

          case ID_FOLFIND_APPLYCHANGE_PB:
          case ID_FOLFIND_APPLYNEXT_PB:
            {
              int iSelItem = -1;
              PFOLFINDRESULTENTRY pEntry = NULL;
              BOOL fChangesDone = FALSE;

              // get currently selected listbox entry 
              iSelItem = QUERYSELECTIONHWND( pIda->hResultListBox );
              if ( iSelItem == -1 ) break;

              // get result entry pointer from the selected item
              pEntry = (PFOLFINDRESULTENTRY) QUERYITEMHANDLEHWND( pIda->hResultListBox, iSelItem );
              if ( pEntry == NULL ) break;


              // load the document if is not currently loaded
              if ( strcmp( pEntry->szDocObjName, pIda->szCurDocObjName ) != 0 )
              {
                FolFindCloseDoc( pIda, TRUE );
                strcpy( pIda->szFolObjName, pEntry->szDocObjName );
                strcpy( pIda->szCurDoc, UtlSplitFnameFromPath( pIda->szFolObjName ) );
                if ( !GFR_OpenDoc( pIda, TRUE, FALSE, FALSE ) ) break;
              }

              // perform all prepared changes and adjust type of change entry
              {
                int iOffsDelta = 0;              // delta between old offset in result entry and new offset because of changes in the text
                PFOLFINDRESULTPOS pPos = (PFOLFINDRESULTPOS)((PBYTE)pEntry + pEntry->iResultPosOffs);

                for( int i = 0; i < pEntry->iUsedEntries; i++, pPos++ )
                {
                  if ( pPos->Type == CHANGETO_TYPE )
                  {
                    PSZ_W pszChangeTo = (PSZ_W)((PBYTE)(pEntry->pszChangeToBuffer) + pPos->usChangeOffs);
                    int iChangeToLen = wcslen( pszChangeTo);
                    USHORT usFoundOffs = (USHORT)((int)pPos->usOffs + iOffsDelta);
                    USHORT usOffs = (USHORT)((int)pPos->usOffs + iOffsDelta) + pPos->usLen;
                    if ( GFR_ReplaceTextInSegment( pIda, pIda->pTargetDoc, pEntry->ulSegNum, usFoundOffs, pPos->usLen, pszChangeTo, &usOffs ) )
                    {
                      pPos->fPreserveCase = pIda->fPreserveCase;
                      pPos->Type = CHANGED_TYPE;
                      fChangesDone = TRUE;
                      iOffsDelta += iChangeToLen - (int)pPos->usLen;
                    }
                  } /* endif */
                } /* endfor */

                // force a repaint of the item
                if ( fChangesDone )
                {
                  RECT rc;
                  SendMessage( pIda->hResultListBox, LB_GETITEMRECT, iSelItem, (LPARAM)&rc );
                  InvalidateRect( pIda->hResultListBox, &rc, FALSE );
                }

                // disable apply button as current entry has been processed
                ENABLECTRL( hwnd, ID_FOLFIND_APPLYCHANGE_PB, FALSE );
                ENABLECTRL( hwnd, ID_FOLFIND_APPLYNEXT_PB, FALSE );
                if ( LOWORD(mp1) == ID_FOLFIND_APPLYNEXT_PB )
                {
                  PostMessage( hwnd, WM_COMMAND, ID_FOLFIND_NEXT_PB, 0 );
                } /* endif */
              }

            }
            break;

          case ID_FOLFIND_COLOR_PB:
            {
              if ( UtlColorChooserDlg( &(pIda->ColorData) ) == 0 )
              {
                // refresh result list
                InvalidateRect( GetDlgItem( hwnd, ID_FOLFIND_RESULT_LISTBOX ), NULL, TRUE );
              } /* endif */
            } 
            break;

          case ID_FOLFIND_FONT_PB:
            {
              CHOOSEFONT ChooseFontData;

              memset( &ChooseFontData, 0, sizeof(ChooseFontData) );
              ChooseFontData.lStructSize = sizeof(ChooseFontData);
              ChooseFontData.hwndOwner = hwnd;
              ChooseFontData.Flags = CF_FORCEFONTEXIST | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
              ChooseFontData.lpLogFont = &(pIda->lf);

              if ( ChooseFont( &ChooseFontData ) )
              {
                // free any previously created font
                if ( pIda->hFontControl ) DeleteObject( pIda->hFontControl );

                // create new font to be used
                pIda->hFontControl = CreateFontIndirect( &(pIda->lf) );

                // update last used values
                if ( pIda->pLastUsed )
                {
                  pIda->pLastUsed->lfHeight = pIda->lf.lfHeight;
                  pIda->pLastUsed->lfWidth = pIda->lf.lfWidth;
                  pIda->pLastUsed->lfEscapement = pIda->lf.lfEscapement;
                  pIda->pLastUsed->lfOrientation = pIda->lf.lfOrientation;
                  pIda->pLastUsed->lfWeight = pIda->lf.lfWeight;
                  pIda->pLastUsed->lfItalic = pIda->lf.lfItalic;
                  pIda->pLastUsed->lfUnderline = pIda->lf.lfUnderline;
                  pIda->pLastUsed->lfStrikeOut = pIda->lf.lfStrikeOut;
                  pIda->pLastUsed->lfCharSet = pIda->lf.lfCharSet;
                  pIda->pLastUsed->lfOutPrecision = pIda->lf.lfOutPrecision;
                  pIda->pLastUsed->lfClipPrecision = pIda->lf.lfClipPrecision;
                  pIda->pLastUsed->lfQuality = pIda->lf.lfQuality;
                  pIda->pLastUsed->lfPitchAndFamily = pIda->lf.lfPitchAndFamily;
                  strcpy( pIda->pLastUsed->lfFaceName, pIda->lf.lfFaceName );
                } /* endif */

                // refresh result list
                InvalidateRect( pIda->hResultListBox, NULL, TRUE );
              } /* endif */
            }
            break;



          case ID_FOLFIND_STOP_PB:
            // stop any search currrently active
            if ( pIda->fSearching )
            {
              pIda->fStopSearch = TRUE;

              // give thread some time for termination and retry stop command
              Sleep( 100 );
              PostMessage( hwnd, WM_COMMAND, MP1FROMSHORT(ID_FOLFIND_STOP_PB), NULL );
              break;
            }
            else
            {
              // enable find and replace buttons, disable stop button
              GFR_PreProcessFindString( pIda->szBuffer, &pIda->fLogicExpression, pIda->szExpression );
              GFR_CheckAndPrepareExpression( pIda->szExpression, pIda->fCaseIgnore );

              if ( pIda->szBuffer[0] != EOS )
              {
                ENABLECTRL( pIda->hwnd, ID_FOLFIND_FIND_PB, TRUE );
                ENABLECTRL( pIda->hwnd, ID_FOLFIND_CHANGE_PB, TRUE );
              } /* endif */ 
              ENABLECTRL( pIda->hwnd, ID_FOLFIND_STOP_PB, FALSE );

            } /* endif */
            break;

          case ID_FOLFIND_FIND_PB:
          case ID_FOLFIND_CHANGE_PB:
            fOK = TRUE;

            // return immediately when a search operation is running
            if ( pIda->fSearching )
            {
              return( 0 );
            } /* endif */

            // delete result list and clear result list control
            GFR_ClearResultList( pIda );
            DELETEALLHWND( pIda->hResultListBox );
            GFR_UpdateSelectionStatus( pIda );

            ENABLECTRL( pIda->hwnd, ID_FOLFIND_EXPORT_PB, FALSE );
            ENABLECTRL( pIda->hwnd, ID_FOLFIND_CHANGE_PB, FALSE );
            GFR_SetEnableStateResultListButtons( pIda );


            // Get user settings
            pIda->fCaseIgnore = QUERYCHECK( hwnd, ID_FOLFIND_IGNORE_RB );
            pIda->fConfirm    = QUERYCHECK( hwnd, ID_FOLFIND_CONFCHANGES_CHK );
            pIda->fUpdateTM   = QUERYCHECK( hwnd, ID_FOLFIND_MEMUPD_CHK );
            pIda->fTranslTextOnly = QUERYCHECK( hwnd, ID_FOLFIND_TRANSLTEXT_CHK );
            pIda->fFind       = (WMCOMMANDID( mp1, mp2 ) == ID_FOLFIND_FIND_PB);
            pIda->fWholeWordsOnly = QUERYCHECK( hwnd, ID_FOLFIND_WHOLEWORD_CHK );
            pIda->pLastUsed->fAndFindInSource= pIda->fAndFindInSource = QUERYCHECK( hwnd, ID_FOLFIND_FINDSOURCE_CHK );
            pIda->pLastUsed->fApplyBatchList = pIda->fApplyBatchList = QUERYCHECK( hwnd, ID_FOLFIND_APPLYBATCH_CHK );
            pIda->pLastUsed->fRespectLineFeeds = pIda->fRespectLineFeeds = QUERYCHECK( hwnd, ID_FOLFIND_RESPECTLF_CHK );
            if ( QUERYCHECK( hwnd, ID_FOLFIND_SHOWSOURCEANDTARGET_CHK ) )
            {
              pIda->pLastUsed->fShowSource = TRUE;
              pIda->pLastUsed->fShowTarget = TRUE;
            }
            if ( QUERYCHECK( hwnd, ID_FOLFIND_SEARCHBOTH_RB) )
            {
              pIda->fSearchInTarget = TRUE;
              pIda->fSearchInSource = TRUE;
              pIda->pLastUsed->sSearchIn = GFR_SEARCH_IN_BOTH;
            }
            else if ( QUERYCHECK( hwnd, ID_FOLFIND_SEARCHSOURCE_RB) )
            {
              pIda->fSearchInTarget = FALSE;
              pIda->fSearchInSource = TRUE;
              pIda->pLastUsed->sSearchIn = GFR_SEARCH_IN_SOURCE;
            }
            else 
            {
              pIda->fSearchInTarget = TRUE;
              pIda->fSearchInSource = FALSE;
              pIda->pLastUsed->sSearchIn = GFR_SEARCH_IN_TARGET;
            }
            GFR_AdjustResultColumns( hwnd, (PFOLFINDDATA)pIda );

            {
              SHORT sItem = 0;

              // preset with defaults
              pIda->chWildCardMultChar = NONE_WILDCARD;
              pIda->chWildCardSingleChar = NONE_WILDCARD;

              // get user characters
              sItem = CBQUERYSELECTION( hwnd, ID_FOLFIND_WILDCARDSINGLE_CB );
              if ( sItem >= 0 ) pIda->chWildCardSingleChar = szWildCards[sItem];
              sItem = CBQUERYSELECTION( hwnd, ID_FOLFIND_WILDCARDMULT_CB );
              if ( sItem >= 0 ) pIda->chWildCardMultChar = szWildCards[sItem];

              // check that different wildcard characters have been chosen
              if ( (pIda->chWildCardMultChar != NONE_WILDCARD)   &&
                   (pIda->chWildCardSingleChar != NONE_WILDCARD) &&
                   (pIda->chWildCardMultChar == pIda->chWildCardSingleChar) )
              {
                UtlError( ERROR_SAME_WILDCARDS, MB_CANCEL, 0, NULL, EQF_ERROR );
                SETFOCUS( hwnd, ID_FOLFIND_WILDCARDSINGLE_CB );
                break;
              } /* endif */

              // fill our static fields
              chMultSubst = pIda->chWildCardMultChar;
              chSingleSubst = pIda->chWildCardSingleChar;

            }

            // get number of segments to be displayed before/after segment containing the found string
            {
              char szNum[3];
              int iSelected = 0;
              CBQUERYSELECTEDITEMTEXT( iSelected, hwnd, ID_FOLFIND_SEGSAROUND_CB, szNum );
              int iNum = atol( szNum );
              pIda->pLastUsed->sShowBeforeAfter = (SHORT)iNum;
            } 


            // activate first batch list entry when requested or use data from our entry fields
            if ( pIda->fApplyBatchList )
            {
              pIda->iCurBatchEntry = GFR_GetNextNonEmptyBatchListEntry( pIda, 0 );
              if ( pIda->iCurBatchEntry < pIda->iBatchListUsed )
              {
                PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[pIda->iCurBatchEntry];
                GFR_PrepareSearchForBatchEntry( pIda, pEntry );
              }
              else
              {
                UtlError( ERROR_INVALID_BATCHLIST, MB_CANCEL, 0, NULL, EQF_ERROR );
                SETFOCUS( hwnd, ID_FOLFIND_APPLYBATCH_CHK );
                break;
              }
            }
            else
            {
              // Get text from the entry fields
              QUERYTEXTW( hwnd, ID_FOLFIND_FIND_EF, pIda->szFind );
              QUERYTEXTW( hwnd, ID_FOLFIND_CHANGE_EF, pIda->szChangeTo );
              QUERYTEXTW( hwnd, ID_FOLFIND_FINDSOURCE_EF, pIda->szAndFindInSource);

              // set/prepare the find and replace fields 
              if ( !GFR_SetSearchFields( pIda, hwnd, pIda->szFind, pIda->szChangeTo, pIda->szAndFindInSource, TRUE ) )
              {
                // do not start search or replace 
                break;
              } /* endif */
            } /* endif */

            /**************************************************************/
            /* Save last used values                                      */
            /**************************************************************/
            {
              pIda->pLastUsed->fFolFindCaseRespect = (EQF_BOOL)(!pIda->fCaseIgnore);
              pIda->pLastUsed->fFolFindConfirm     = (EQF_BOOL)pIda->fConfirm;
              pIda->pLastUsed->fFolFindUpdateTM    = (EQF_BOOL)pIda->fUpdateTM;
              pIda->pLastUsed->fFolFindTranslTextOnly = (EQF_BOOL)pIda->fTranslTextOnly;
              pIda->pLastUsed->fFolFindWholeWordsOnly = (EQF_BOOL)pIda->fWholeWordsOnly;
              pIda->pLastUsed->chWildCardMultChar = pIda->chWildCardMultChar;
              pIda->pLastUsed->chWildCardSingleChar = pIda->chWildCardSingleChar;

              if ( pIda->fSearchInTarget && pIda->fSearchInSource )
              {
                pIda->pLastUsed->sSearchIn = 2;
              }
              else if ( pIda->fSearchInSource )
              {
                pIda->pLastUsed->sSearchIn = 1;
              }
              else
              {
                pIda->pLastUsed->sSearchIn = 0;
              }

              // Update Search and replace History
              if ( !pIda->fApplyBatchList )
              {
                GFR_UpdHistory( pIda->szFind, pIda->pLastUsed->szFolFind, &pIda->pLastUsed->szFindList[0] );
                GFR_UpdHistory( pIda->szChangeTo, pIda->pLastUsed->szFolChangeTo, &pIda->pLastUsed->szReplaceList[0] );
                GFR_UpdHistory( pIda->szAndFindInSource, pIda->pLastUsed->szAndFindInSource, &pIda->pLastUsed->szAddFindList[0] );

                GFR_RefreshComboboxHistory( hwnd, ID_FOLFIND_FIND_EF, pIda, pIda->pLastUsed->szFolFind, pIda->pLastUsed->szFindList );
                GFR_RefreshComboboxHistory( hwnd, ID_FOLFIND_CHANGE_EF, pIda, pIda->pLastUsed->szFolChangeTo, pIda->pLastUsed->szReplaceList );
                GFR_RefreshComboboxHistory( hwnd, ID_FOLFIND_FINDSOURCE_EF, pIda, pIda->pLastUsed->szAndFindInSource, pIda->pLastUsed->szAddFindList );
              } /* endif */
            }

            pIda->usHits = 0;
            pIda->usChanges = 0;
            pIda->fStartSearch = TRUE; // tell thread to start searching
            SetTimer( hwnd, FOLFINDTIMER, 200, NULL );

            // disable find and replace buttons, enable stop button
            ENABLECTRL( pIda->hwnd, ID_FOLFIND_FIND_PB, FALSE );
            ENABLECTRL( pIda->hwnd, ID_FOLFIND_CHANGE_PB, FALSE );
            ENABLECTRL( pIda->hwnd, ID_FOLFIND_STOP_PB, TRUE );

            break;

            case ID_FOLFIND_OPEN_PB :
            {
              POPENANDPOS pOpen;
              int iSelItem = QUERYSELECTIONHWND( pIda->hResultListBox );
              if ( iSelItem != -1 )
              {
                // close any open document
                FolFindCloseDoc( pIda, TRUE );

                if ( UtlAlloc( (PVOID *)&pOpen, 0L, (LONG)sizeof(OPENANDPOS), ERROR_STORAGE ) )
                {
                  CHAR   szDocFullPath[MAX_EQF_PATH];
                  FILEFINDBUF stResultBuf;            // DOS file find struct
                  USHORT     usCount = 1;             // number of files requested
                  HDIR       hDirHandle = HDIR_CREATE;// DosFind routine handle

                  pIda->fDocOpenActive = TRUE;

                  // get document and folder info for selected entry
                  PFOLFINDRESULTENTRY pEntry = (PFOLFINDRESULTENTRY)QUERYITEMHANDLEHWND( pIda->hResultListBox, iSelItem );

                  if ( pEntry != NULL )
                  {
                    // Get last update date to check later on for document changes
                    strcpy( pOpen->szDocName, pEntry->szDocObjName );
                    UtlSplitFnameFromPath( pOpen->szDocName );
                    UtlMakeEQFPath( szDocFullPath, pOpen->szDocName[0], DIRSEGTARGETDOC_PATH, UtlGetFnameFromPath(pOpen->szDocName) );
                    strcat( szDocFullPath, BACKSLASH_STR );
                    strcat( szDocFullPath, UtlGetFnameFromPath( pIda->szCurDocObjName  ));
                    UtlFindFirst( szDocFullPath, &hDirHandle, FILE_NORMAL, &stResultBuf, sizeof(stResultBuf), &usCount, 0L, 0);
                    // close search file handle
                    if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );
                    FileTimeToDosDateTime( &stResultBuf.ftLastWriteTime, (LPWORD)&pIda->DocDate, (LPWORD)&pIda->DocTime );

                    // Setup open-and-pos structure and pass request to document handler 
                    strcpy( pOpen->szDocName, pEntry->szDocObjName );
                    pOpen->ulSeg = pEntry->ulSegNum;
                    pOpen->usOffs= pIda->usFoundOffs;
                    pOpen->usLen = (USHORT)UTF16strlenCHAR(pIda->szFindUpr);

                    // use first found string in target as searched string
                    PFOLFINDRESULTPOS pPos = (PFOLFINDRESULTPOS)((PBYTE)pEntry + pEntry->iResultPosOffs);
                    int i = 0;
                    while( (i < pEntry->iUsedEntries ) && !pPos->fTarget )
                    {
                      pPos++; i++;
                    } /* endwhile */
                    if ( i < pEntry->iUsedEntries )
                    {
                      PSZ_W pszTarget = (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[TARGET_TEXT]);
                      UTF16strncpy( pOpen->chFind, pszTarget + pPos->usOffs, pPos->usLen );
                      pOpen->usOffs= pPos->usOffs;
                      pOpen->chFind[pPos->usLen] = 0;
                    } /* endwhile */

                    EqfPost2Handler( DOCUMENTHANDLER, WM_EQF_PROCESSTASK, MP1FROMSHORT(OPEN_AND_POSITION_TASK), MP2FROMP(pOpen) );
                  } /* endif */
                } /* endif */
              } /* endif */
            }
            break;

          case ID_FOLFIND_CONFCHANGES_CHK:
          case ID_FOLFIND_PRESERVECASE_CHK:
          case ID_FOLFIND_FIND_EF:
          case ID_FOLFIND_CHANGE_EF:
          case ID_FOLFIND_FINDSOURCE_EF:
          case ID_FOLFIND_FINDSOURCE_CHK:
          case ID_FOLFIND_APPLYBATCH_CHK:
          case ID_FOLFIND_SEARCHBOTH_RB:
          case ID_FOLFIND_SEARCHTARGET_RB:
          case ID_FOLFIND_SEARCHSOURCE_RB:
          case ID_FOLFIND_RESPECTLF_CHK:
          case ID_FOLFIND_SEGSAROUND_CB:
            FolFindControl( hwnd, WMCOMMANDID( mp1, mp2 ), HIWORD( mp1 ));
            break;

          case ID_FOLFIND_SHOWSOURCEANDTARGET_CHK:
            GFR_AdjustResultColumns( hwnd, (PFOLFINDDATA)pIda );
            break;

          case ID_FOLFIND_RESULT_LISTBOX:
            if ( HIWORD(mp1) == LBN_SELCHANGE )
            {
              // handle the selection change in the result listbox
              GFR_SetEnableStateResultListButtons( pIda );
              GFR_UpdateSelectionStatus( pIda );
            } /* endif */
            break;
        } /* endswitch */
        return 0;
      }

    case WM_CLOSE:
      {
        PFOLFINDDATA pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );

        if ( pIda->fUseThaiFont )
        {
          DelCtrlFont(hwnd, ID_FOLFIND_CHANGE_EF );
          DelCtrlFont(hwnd, ID_FOLFIND_FIND_EF );
        } /* endif */
        return 0;
      }
      break;

   case WM_SIZE :
     // resize inner window only if normal sizing request...
     pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );


     if ( (pIda != NULL) && ((mp1 == SIZENORMAL) || (mp1 == SIZEFULLSCREEN)) )
     {
       SHORT   sWidth  = LOWORD( mp2 );      // new width of dialog
       SHORT   sHeight = HIWORD( mp2 );      // new height of dialog
       LONG   lBorderSize  = WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER);
       LONG   cxAvail = sWidth - (2 * lBorderSize);

       // re-arrange controls
       {
         LONG  yTextStartPos;          // vertical start position for text area

         {
           // we re-position/re-size all dialog controls ...
           HDWP hdwp = BeginDeferWindowPos( 11 );

           // resize find and replace entry fields
           {
             RECT rect;
             LONG lHalfWidth = cxAvail / 2;
             LONG lLastPos = 0;

             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_FIND_EF ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_FIND_EF ),
                                    HWND_TOP, 0, 0,
                                    (lHalfWidth > (rect.left + 3)) ? (lHalfWidth - rect.left - 3) : 0,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_CHANGE_EF ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_CHANGE_EF ),
                                    HWND_TOP, 0, 0,
                                    (lHalfWidth > (rect.left + 3)) ? (lHalfWidth - rect.left - 3) : 0,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_FINDSOURCE_CHK ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_FINDSOURCE_CHK ),
                                    HWND_TOP, lHalfWidth + 10, rect.top, 
                                    0, 0,
                                    SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             lLastPos = rect.right;

             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_FINDSOURCE_EF ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_FINDSOURCE_EF ),
                                    HWND_TOP, lLastPos + 10, rect.top, 
                                    (cxAvail - lLastPos - 10) > 0 ? (cxAvail - lLastPos - 10) : 0,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOZORDER );

             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_APPLYBATCH_CHK ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_APPLYBATCH_CHK ),
                                    HWND_TOP, lHalfWidth + 10, rect.top, 
                                    0, 0,
                                    SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );

             
           }

           // resize options-and-documents groupbox
           {
             RECT rect;

             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_OPTDOCS_GB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_OPTDOCS_GB ),
                                    HWND_TOP, 0, 0,
                                    (cxAvail > (rect.left + 3)) ? (cxAvail - rect.left - 3) : 0,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
           }

           // resize documents listbox and groupbox
           {
             RECT rect;

             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_DOCS_GB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_DOCS_GB ),
                                    HWND_TOP, 0, 0,
                                    (cxAvail > (rect.left + 6)) ? (cxAvail - rect.left - 6) : 0,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
             yTextStartPos = rect.bottom + 2;

             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_DOCS_LB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_DOCS_LB ),
                                    HWND_TOP, 0, 0,
                                    (cxAvail > (rect.left + 8)) ? (cxAvail - rect.left - 8) : 0,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
           }

           // resize and reposition result area
           {
             RECT rect;
             LONG lResultButtonTop;
             LONG lGroupBoxBottom = 0;
             LONG lGroupBoxWidth = 0;
             LONG lGroupBoxLeft = 0;

             // resize text groupbox
             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_TEXT_GB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             rect.bottom = sHeight - pIda->sBorderSize - 4;
             lGroupBoxWidth = cxAvail - (2 * pIda->sBorderSize);
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_TEXT_GB ),
                                    HWND_TOP,
                                    0, 0,
                                    lGroupBoxWidth,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
             lGroupBoxBottom = rect.bottom;
             lGroupBoxLeft = rect.left;
             

             // adjust position of result area buttons
             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_PREV_PB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             lResultButtonTop = rect.top;
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_PREV_PB ),
                                    HWND_TOP,
                                    rect.left, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0,
                                    SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_EXPORT_PB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             lResultButtonTop = rect.top;
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_EXPORT_PB ),
                                    HWND_TOP,
                                    rect.left, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0,
                                    SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_APPLYCHANGE_PB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             lResultButtonTop = rect.top;
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_APPLYCHANGE_PB ),
                                    HWND_TOP,
                                    rect.left, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0,
                                    SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_APPLYNEXT_PB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             lResultButtonTop = rect.top;
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_APPLYNEXT_PB ),
                                    HWND_TOP,
                                    rect.left, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0,
                                    SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_NEXT_PB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             lResultButtonTop = rect.top;
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_NEXT_PB ),
                                    HWND_TOP,
                                    rect.left, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0,
                                    SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );


             // resize result control
             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_HEADER_TEXT ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_HEADER_TEXT ),
                                    HWND_TOP,
                                    0, 0,
                                    lGroupBoxWidth - 2 * (rect.left - lGroupBoxLeft ),
                                    lResultButtonTop - rect.top - 8,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
             GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_RESULT_LISTBOX ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );

             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_RESULT_LISTBOX ),
                                    HWND_TOP,
                                    0, 0,
                                    lGroupBoxWidth - 2 * (rect.left - lGroupBoxLeft ),
                                    lResultButtonTop - rect.top - 8,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
           }

           // do actual dialog control re-positioning
           if ( hdwp != NULL )
           {
             EndDeferWindowPos( hdwp );
           } /* endif */

           // adjust result control columns
           GFR_ResizeColumns( hwnd, pIda );
         }
       }
     } /* endif */
     break;

   case WM_TIMER:
     pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
     if ( !pIda->fProcessingTimerMessage )
     {
       int iDisplayUpTo = 0;          // display result entries up to this entry
       BOOL fFirstResultEntry = FALSE;

       pIda->fProcessingTimerMessage = TRUE;

       // check thread status and decide which entries are to be displayed
       if ( !pIda->fSearching )       // search operation complete?
       {
         KillTimer( hwnd, FOLFINDTIMER );   // we don't need the timer anymore
         iDisplayUpTo = pIda->iResultListUsed; // we can display all result entries

         // enable/disable pushbuttons
         GFR_PreProcessFindString( pIda->szBuffer, &pIda->fLogicExpression, pIda->szExpression );
         GFR_CheckAndPrepareExpression( pIda->szExpression, pIda->fCaseIgnore );
         ENABLECTRL( pIda->hwnd, ID_FOLFIND_FIND_PB, (pIda->szBuffer[0] != EOS) );
         ENABLECTRL( pIda->hwnd, ID_FOLFIND_CHANGE_PB, (pIda->szBuffer[0] != EOS) );
         ENABLECTRL( pIda->hwnd, ID_FOLFIND_STOP_PB, FALSE );
       }
       else
       {
         iDisplayUpTo = pIda->iResultListUsed - 1; // do not display last entry as this may be changed by the search thread
       }

       fFirstResultEntry = (pIda->iResultsDisplayed == 0) && (iDisplayUpTo != 0);

      // show available result entries
       while ( (pIda->ppResultList != NULL) && (pIda->iResultsDisplayed < iDisplayUpTo) )
       {
         PFOLFINDRESULTENTRY pCurEntry = pIda->ppResultList[pIda->iResultsDisplayed];
         INSERTITEMENDHWND( pIda->hResultListBox, pCurEntry );
         pIda->iResultsDisplayed++;
         GFR_UpdateSelectionStatus( pIda );
       } /* endwhile */

       if ( fFirstResultEntry )
       {
         SELECTITEMHWND( pIda->hResultListBox, 0 );
       } /* endif */

       if ( !pIda->fSearching )
       {
         GFR_SetEnableStateResultListButtons( pIda );
       }
       pIda->fProcessingTimerMessage = FALSE;
     }
     break;

   case WM_DRAWITEM:                // draw a listbox/combobox item
      {
        LPDRAWITEMSTRUCT lpDisp = (LPDRAWITEMSTRUCT)mp2;
        pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
        GFR_DrawItem( pIda, lpDisp );
        return( TRUE );
      }
      break;

  case WM_MEASUREITEM:
    {
      pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
      LPMEASUREITEMSTRUCT pMeasureItem = (LPMEASUREITEMSTRUCT)mp2; 
      GFR_MeasureItem( pIda, pMeasureItem );
      return( TRUE );
    }
    break;    

  } /* endswitch */


  return WinDefDlgProc( hwnd, msg, mp1, mp2 );

} /* end GFR_INTERACTIVE_DLGPROC */



/**********************************************************************/
/* Handling for WM_CONTROL message                                    */
/**********************************************************************/
MRESULT FolFindControl
(
HWND   hwnd,                        // dialog handle
SHORT  sId,                         // id in action
SHORT  sNotification                // notification
)
{
  PFOLFINDDATA pIda;                    // ptr to IDA of dialog
  MRESULT mResult = MRFROMSHORT(FALSE);

  pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
  switch ( sId )
  {
    case ID_FOLFIND_CONFCHANGES_CHK:
      if ( QUERYCHECK( hwnd, ID_FOLFIND_CONFCHANGES_CHK ) )
      {
        SETTEXT( pIda->hwnd, ID_FOLFIND_CHANGE_PB, "Prepare &Replace" );
      }
      else
      {
        SETTEXT( pIda->hwnd, ID_FOLFIND_CHANGE_PB, "&Replace" );
      } /* endif */
      break;

    case ID_FOLFIND_SEARCHBOTH_RB:
      SETTEXT( hwnd, ID_FOLFIND_FIND_TEXT, "Find string [in SRC+TGT]" ); 
      ENABLECTRL( hwnd, ID_FOLFIND_CHANGE_EF, !QUERYCHECK( hwnd, ID_FOLFIND_APPLYBATCH_CHK ) );
      ENABLECTRL( pIda->hwnd, ID_FOLFIND_CHANGE_PB, TRUE );
      break;

    case ID_FOLFIND_SEARCHTARGET_RB:
      SETTEXT( hwnd, ID_FOLFIND_FIND_TEXT, "Find string [in TARGET]" ); 
      ENABLECTRL( hwnd, ID_FOLFIND_CHANGE_EF, !QUERYCHECK( hwnd, ID_FOLFIND_APPLYBATCH_CHK ) );
      ENABLECTRL( pIda->hwnd, ID_FOLFIND_CHANGE_PB, TRUE );
      break;

    case ID_FOLFIND_SEARCHSOURCE_RB:
      SETTEXT( hwnd, ID_FOLFIND_FIND_TEXT, "Find string [in SOURCE]" ); 
      ENABLECTRL( hwnd, ID_FOLFIND_CHANGE_EF, FALSE );
      ENABLECTRL( pIda->hwnd, ID_FOLFIND_CHANGE_PB, FALSE );
      break;

    case ID_FOLFIND_APPLYBATCH_CHK:
      GFR_EnableOrDisableEntryFields( hwnd );
      break;

    case ID_FOLFIND_FINDSOURCE_CHK:
      ENABLECTRL( hwnd, ID_FOLFIND_FINDSOURCE_EF, QUERYCHECK( hwnd, ID_FOLFIND_FINDSOURCE_CHK ) );
      break;

    case ID_FOLFIND_FIND_EF:
      if ( (sNotification == CBN_EDITCHANGE) || (sNotification == CBN_SELCHANGE) )
      {
        // get current find text
        if ( sNotification == CBN_EDITCHANGE )
        {
          QUERYTEXTW( hwnd, ID_FOLFIND_FIND_EF, pIda->szBuffer );
        }
        else // sNotification == CBN_SELCHANGE
        {
          SHORT sItem = CBQUERYSELECTION( hwnd, ID_FOLFIND_FIND_EF );
          CBQUERYITEMTEXTW( hwnd, ID_FOLFIND_FIND_EF, sItem, pIda->szBuffer );
        } /* endif */

        // process find string
//        UtlStripBlanksW( pIda->szBuffer );
        GFR_PreProcessFindString( pIda->szBuffer, &pIda->fLogicExpression, pIda->szExpression );
        GFR_CheckAndPrepareExpression( pIda->szExpression, pIda->fCaseIgnore );

        if ( pIda->szBuffer[0] == EOS )
        {
          ENABLECTRL( pIda->hwnd, ID_FOLFIND_FIND_PB, FALSE );
          ENABLECTRL( pIda->hwnd, ID_FOLFIND_CHANGE_PB, FALSE );
        }
        else if ( !pIda->fSearching && !pIda->fStartSearch )
        {
          ENABLECTRL( pIda->hwnd, ID_FOLFIND_FIND_PB, TRUE );
          SETDEFAULTPB_TRUE( pIda->hwnd, ID_FOLFIND_FIND_PB );
          ENABLECTRL( pIda->hwnd, ID_FOLFIND_CHANGE_PB, TRUE );
        } /* endif */
      } /* endif */
      break;

    case ID_FOLFIND_RESPECTLF_CHK:
      {
        BOOL fNewState = QUERYCHECK( hwnd, ID_FOLFIND_RESPECTLF_CHK );
        if ( pIda->fRespectLineFeeds != fNewState )
        {
          pIda->pLastUsed->fRespectLineFeeds = pIda->fRespectLineFeeds = fNewState;

          // force a repaint of the result list
          GFR_ForceRefreshOfItemHeights( pIda );
        } /* endif */
      }
      break;
      
    case ID_FOLFIND_SEGSAROUND_CB:
      {
        char szNum[10];
        int sItem;
        CBQUERYSELECTEDITEMTEXT( sItem, hwnd, ID_FOLFIND_SEGSAROUND_CB, szNum );
        SHORT sNewValue = (SHORT)atol( szNum );
        if ( sNewValue != pIda->pLastUsed->sShowBeforeAfter )
        {
          pIda->pLastUsed->sShowBeforeAfter = sNewValue;

          // force a repaint of the result list
          GFR_ForceRefreshOfItemHeights( pIda );
        } /* endif */
      }
      break;

    case ID_FOLFIND_PRESERVECASE_CHK:
      BOOL fNewState = QUERYCHECK( hwnd, ID_FOLFIND_PRESERVECASE_CHK );
      if ( pIda->fPreserveCase != fNewState )
      {
        pIda->pLastUsed->fPreserveCase = pIda->fPreserveCase = fNewState;

        // force a repaint of the result list
        GFR_ForceRefreshOfItemHeights( pIda );
      } /* endif */
      break;

  } /* endswitch sID */
  return( mResult );
} /* end of function FolFindControl */



VOID FolFindFreeDoc
(
  PVOID pvDoc,
  PVOID pvSourceDoc
)
{
  PTBDOCUMENT     pDoc = (PTBDOCUMENT)pvDoc;
  PTBDOCUMENT     pSourceDoc = (PTBDOCUMENT)pvSourceDoc;

  if ( pSourceDoc != NULL )
  {
   SegFileFreeDoc( (PVOID *)&pSourceDoc );
  }

  if ( pDoc == NULL )
  {
    return;
  } /* endif */

  if (pDoc->pQFTagTable)
  {
    TAFreeTagTable( (PLOADEDTABLE)pDoc->pQFTagTable );
    pDoc->pQFTagTable = NULL;
  } /* endif */
  if (pDoc->pDocTagTable)
  {
    TAFreeTagTable( (PLOADEDTABLE)pDoc->pDocTagTable );
    pDoc->pDocTagTable = NULL;
  } /* endif */

  SegFileFreeDoc( (PVOID *)&pDoc );
} /* end of function FolFindFreeDoc */

BOOL FolFindMatch
(
  PTBDOCUMENT pDoc,                  //ptr to target document
  PTBDOCUMENT pSourceDoc,            //ptr to source document
  PFOLFINDDATA pIda
)
{
  BOOL fFound    = FALSE;              // found-a-match flag
  BOOL fDone     = FALSE;              // we-are-through flag
  ULONG  ulLen;                        // length of chFindTarget
  PSZ_W  pData;
  SHORT sRc;                          //return value
  PTBSEGMENT pSeg;                    //ptr to segment
  CHAR_W c;                             //temporarily hold *pData
  ulLen    = UTF16strlenCHAR(pIda->szFindUpr);
  sRc = 0;


  // while no match fFound and not at end of document
  while ( !fFound && !fDone )
  {
    pSeg = EQFBGetSegW( pDoc, pIda->ulSegNum );

    if ( pSeg == NULL)
    {
      /****************************************************************/
      /* End of document reached                                      */
      /****************************************************************/
      fDone = TRUE;
    }
    else if ( pSeg->pDataW == NULL )
    {
      /****************************************************************/
      /* End of document reached???                                   */
      /****************************************************************/
      fDone = TRUE;
    }
    else if ( pSeg->SegFlags.Joined && !pSeg->SegFlags.JoinStart )
    {
      /****************************************************************/
      /* Segment has been chained to a previous segment so ignore it  */
      /****************************************************************/
      if ( pIda->ulSegNum < pDoc->ulMaxSeg - 1 )
      {
        pIda->ulSegNum++;
        pIda->usOffs = 0;
      }
      else
      {
        fDone = TRUE;               // no more segments to follow
      } /* endif */
    }
    else if ( (pSeg->qStatus == QF_NOP) && pIda->fTranslTextOnly )
    {
      /****************************************************************/
      /* Segment with non-translatable data, ignore segment but       */
      /* count LFs to keep line number up-to-date                     */
      /****************************************************************/
      pData  = pSeg->pDataW;

      while ( (c = *pData) != NULC )
      {
        if ( c == LF )
        {
          pIda->usLine++;
        } /* endif */
        pData++;
      } /* endwhile */


      if ( pIda->ulSegNum < pDoc->ulMaxSeg - 1 )
      {
        pIda->ulSegNum++;
        pIda->usOffs = 0;
        if ( pIda->fApplyBatchList )
        {
          pIda->iCurBatchEntry = GFR_GetNextNonEmptyBatchListEntry( pIda, 0 );
          if ( pIda->iCurBatchEntry < pIda->iBatchListUsed )
          {
            PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[pIda->iCurBatchEntry];
            GFR_PrepareSearchForBatchEntry( pIda, pEntry );
          } /* endif */
        } /* endif */
      }
      else
      {
        fDone = TRUE;               // no more segments to follow
      } /* endif */
    }
    else                            // normal data segments
    {
      if ( pIda->fCurrentSearchInSource )
      {
        pSeg = EQFBGetSegW( pSourceDoc, pIda->ulSegNum );
      }

      UTF16strcpy( pIda->szTempSeg, pSeg->pDataW );
      GFR_PrepareTempSeg( pIda, pIda->szTempSeg, pSeg->qStatus == QF_NOP );

      // prepare search pattern
      UTF16strcpy( pIda->szSearchPattern, pIda->szFindUpr );
      FolFind_PreparePattern( pIda->szSearchPattern );

      // apply search expression on current segment..
      {
        PSZ_W pszExpression = pIda->szExpression;
        USHORT usMatchPos = 0;         // position of found string
        USHORT usMatchLen = 0;         // length of found string
        BOOL fOK = TRUE;

        memcpy( &pIda->LastFindList, &pIda->ActFindList, sizeof(pIda->ActFindList) );
        pIda->ulLastFindSeg = pIda->ulActFindSeg;
        memset( &pIda->ActFindList, 0, sizeof(pIda->ActFindList) );
        fOK = GFR_EvaluateExpression( &pszExpression, &fFound, pIda->szTempSeg, pIda->usOffs, &usMatchPos, &usMatchLen,
                                          pIda->fWholeWordsOnly, &pIda->ActFindList );

        // ignore match if it the same as last time in order to avoid endless loops..
        if ( fFound )
        {
          pIda->ulActFindSeg = pIda->ulSegNum;

          if ( (pIda->ulActFindSeg == pIda->ulLastFindSeg) &&
               (memcmp( &pIda->LastFindList, &pIda->ActFindList, sizeof(pIda->ActFindList) ) == 0) )
          {
            fFound = FALSE;
          } /* endif */
        } /* endif */

        if ( !fOK )
        {
          // TODO: error handling
        } /* endif */

        // count linefeeds up to segment end or up to found string
        {
          USHORT usEndOffs, usI;

          // set end offset for linefeed handling
          usEndOffs = fFound ? usMatchPos : (USHORT)UTF16strlenCHAR( pIda->szTempSeg + pIda->usOffs );

          // count linefeeds
          for( usI = pIda->usOffs; usI < usEndOffs; usI++ )
          {
            if ( pSeg->pDataW[usI] == LF ) pIda->usLine++;
          } /* endfor */
        }

        // handle match
        if ( fFound )
        {
          if ( pIda->fAndFindInSource )
          {
            // check if source segment contains the string being searched for

            // prepare search pattern
            UTF16strcpy( pIda->szSearchPattern, pIda->szAndFindInSourceUpr );
            FolFind_PreparePattern( pIda->szSearchPattern );

            PTBSEGMENT pSourceSeg = EQFBGetSegW( pSourceDoc, pIda->ulSegNum );

            UTF16strcpy( pIda->szAddSourceTempSeg, pSourceSeg->pDataW );

            GFR_PrepareTempSeg( pIda, pIda->szAddSourceTempSeg, pSourceSeg->qStatus == QF_NOP );
            PSZ_W pszExpression = pIda->szAndFindInExpression;
            USHORT usTempMatchPos = 0;         // position of found string
            USHORT usTempMatchLen = 0;         // length of found string
            BOOL fOK = TRUE;

            fOK = GFR_EvaluateExpression( &pszExpression, &fFound, pIda->szAddSourceTempSeg, pIda->usOffs, &usTempMatchPos, &usTempMatchLen, pIda->fWholeWordsOnly, NULL );
          }

          if ( fFound )
          {
            fDone  = TRUE;
            pIda->usFoundOffs = usMatchPos;
            pIda->usFindLen   = usMatchLen;
            pIda->usOffs      = usMatchPos + usMatchLen;
          }
        } /* endif */
      }

      if ( !fFound )
      {
        // search target if requested and not done yet
        if ( pIda->fCurrentSearchInSource && pIda->fSearchInTarget )
        {
          pIda->fCurrentSearchInSource = FALSE;
          pIda->usOffs = 0;
        }
        else
        {
          // goto next valid batch list entry when in batch list mode
          if ( pIda->fApplyBatchList ) pIda->iCurBatchEntry = GFR_GetNextNonEmptyBatchListEntry( pIda, pIda->iCurBatchEntry + 1 );

          // use batch list entry if available
          if ( pIda->fApplyBatchList && (pIda->iCurBatchEntry < pIda->iBatchListUsed) )
          {
            // in batch list mode stay in the current segment and repeat the find operation with the next entry
            pIda->usOffs = 0;
            memset( &pIda->LastFindList, 0, sizeof(pIda->LastFindList) ); // clear last find list
            PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[pIda->iCurBatchEntry];
            GFR_PrepareSearchForBatchEntry( pIda, pEntry );
          }
          // check if next segment available
          else if ( pIda->ulSegNum < pDoc->ulMaxSeg - 1 )
          {
            pIda->ulSegNum++;
            pIda->usOffs = 0;
            if ( pIda->fApplyBatchList )
            {
              pIda->iCurBatchEntry = GFR_GetNextNonEmptyBatchListEntry( pIda, 0 );
              if ( pIda->iCurBatchEntry < pIda->iBatchListUsed )
              {
                PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[pIda->iCurBatchEntry];
                GFR_PrepareSearchForBatchEntry( pIda, pEntry );
              } /* endif */
            } /* endif */
            memset( &pIda->LastFindList, 0, sizeof(pIda->LastFindList) ); // clear last find list

            // get location for next search operation
            pIda->fCurrentSearchInSource = pIda->fSearchInSource;
          }
          else
          {
            fDone = TRUE;               // no more segments to follow
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

  } /*endwhile*/

  return fFound;
}


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     FolTMOpen
//+----------------------------------------------------------------------------+
// Function call:     FolTMOpen( PITMIDA );
//+----------------------------------------------------------------------------+
// Description:       This function will open the specified translation
//                    memory and allocate the necessary structures.
//+----------------------------------------------------------------------------+
// Parameters:        PITMIDA              pointer to ida
//+----------------------------------------------------------------------------+
// Returncode type:   USHORT
//+----------------------------------------------------------------------------+
// Returncodes:       ERROR_MSG_HANDLED    error message already handled
//                    NO_ERROR             everything went okay
//+----------------------------------------------------------------------------+
// Side effects:      Specified Translation Memory will be opened
//+----------------------------------------------------------------------------+
// Function flow:     Allocate the buffers necessary for Input and Output,
//                    if not okay then
//                      set usRc to ERROR_MSG_HANDLED
//                    endif
//                    if okay so far
//                       open TM in NONEXCLUSIVE mode and allow for error handl
//                       set usRc to ERROR_MSG_HANDLED
//                    endif
//                    return
//+----------------------------------------------------------------------------+
static USHORT FolTMOpen
(
PFOLFINDDATA pIda
)
{
  USHORT        usRc = NO_ERROR;
  PSZ           pTemp;                 // pointer to TM Name
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  if ( usRc == NO_ERROR )
  {
    if ( !pFactory->exists( NULL, pIda->szDocMemory ) )
    {
      PSZ pszErrParm[2];
      pszErrParm[0] = pIda->szDocMemory;

      // folder or document memory???
      FolQueryInfo2( pIda->szFolObjName, pIda->szNameBuffer, NULL, NULL, NULL, NULL, TRUE );
      if ( strcmp( pIda->szNameBuffer, pIda->szDocMemory ) == 0 )
      {
        usRc = ERROR_TM_NOT_EXIST;
        UtlError( usRc, MB_CANCEL, 1, pszErrParm, EQF_ERROR );
      }
      else
      {
        usRc = ERROR_TM_FOR_DOC_NOT_EXIST;
        pszErrParm[1] = pIda->szLongName;
        UtlError( usRc, MB_CANCEL, 2, pszErrParm, EQF_ERROR );
      } /* endif */
    }
    else
    {
      int iRC = 0;
      pIda->pMem = pFactory->openMemory( NULL, pIda->szDocMemory, NONEXCLUSIVE, &iRC );

      if ( pIda->pMem == NULL )
      {
        usRc = (USHORT)iRC;
        switch ( usRc )
        {
          case FILE_MIGHT_BE_CORRUPTED:
          case VERSION_MISMATCH:
          case CORRUPT_VERSION_MISMATCH:
            pTemp = pIda->szDocMemory;
            UtlError( ITM_TM_NEEDS_ORGANIZE, MB_CANCEL, 1, &pTemp, EQF_ERROR );
            break;
          default:
            pFactory->showLastError( NULL, pIda->szDocMemory, NULL, pIda->hwnd );
            break;
        } /* endswitch */
      } /* endif */
    } /* endif */
  } /* endif */
  return usRc;
}


static USHORT FolTmClose
(
  PFOLFINDDATA  pIda
)

{
  USHORT        usRc = NO_ERROR;

  if ( pIda->pMem != NULL )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    pFactory->closeMemory( pIda->pMem );
    pIda->pMem = NULL;
  } /* endif */

  return usRc;
} // end FolTmClose

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     FolTmReplace
//+----------------------------------------------------------------------------+
// Function call:     FolTMReplace( PFOLFINDDATA, USHORT, PSZ, PSZ );
//+----------------------------------------------------------------------------+
// Description:       replace the segment in the translation memory
//+----------------------------------------------------------------------------+
// Parameters:        PDOCUMENT_IDA        pointer to document ida
//                    PSTEQFSAB            send ahead buffer with segment
//+----------------------------------------------------------------------------+
// Returncode type:   BOOL
//+----------------------------------------------------------------------------+
// Returncodes:       FALSE                error message from TM
//                    TRUE                 everything went okay
//+----------------------------------------------------------------------------+
static BOOL FolTmReplace
(
PFOLFINDDATA  pIda,
PSZ_W       pSrcSeg,
PSZ_W       pTgtSeg
)
{
  BOOL          fOK = TRUE;
  OtmProposal Prop;

  Prop.setSource( pSrcSeg );
  Prop.setTarget( pTgtSeg );

  if ( pIda->szShortAlias[0] != EOS )
  {
    Prop.setDocShortName( pIda->szShortAlias );
    Prop.setDocName( pIda->szAlias );
  }
  else
  {
    Prop.setDocShortName( UtlGetFnameFromPath( pIda->szCurDocObjName ) );
    Prop.setDocName( pIda->szLongName );
  } /* endif */

  Prop.setSegmentNum( pIda->ulSegNum );
  Prop.setMarkup( pIda->szDocFormat );
  Prop.setSourceLanguage( pIda->szDocSrcLng );
  Prop.setTargetLanguage( pIda->szDocTgtLng );

  // handle any segment context
  if ( pIda->pTargetDoc->pfnGetSegContext )
  {
    PTBSEGMENT pPrevSeg = EQFBGetSegW( pIda->pSourceDoc, pIda->ulSegNum - 1 );
    PTBSEGMENT pNextSeg = EQFBGetSegW( pIda->pSourceDoc, pIda->ulSegNum + 1 );
    PSZ_W pszPrevSegData = ( pPrevSeg ) ? pPrevSeg->pDataW : NULL;
    PSZ_W pszNextSegData = ( pNextSeg ) ? pNextSeg->pDataW : NULL;

    (pIda->pTargetDoc->pfnGetSegContext)( pSrcSeg, pszPrevSegData,
                                          pszNextSegData,
                                          pIda->szBuffer,
                                          (LONG)pIda->pSourceDoc,
                                          pIda->ulSegNum );
    Prop.setContext( pIda->szBuffer );
  } /* endif */

  if ( pIda->pMem != NULL ) pIda->pMem->putProposal( Prop );

  return fOK;
} // end FolTmReplace


BOOL FolFindUpdateTM
(
PFOLFINDDATA      pIda                // IDA of FolFind dialog
)
{
  BOOL fOK = TRUE;                     // function return code

  /********************************************************************/
  /* Initialization                                                   */
  /********************************************************************/
  pIda->pMem = NULL;

  /********************************************************************/
  /* Get information from document                                    */
  /********************************************************************/
  if ( fOK )
  {
    fOK = ( DocQueryInfo2( pIda->szCurDocObjName,
                           pIda->szDocMemory,
                           pIda->szDocFormat,
                           pIda->szDocSrcLng,
                           pIda->szDocTgtLng,
                           pIda->szLongName,
                           pIda->szAlias,
                           NULL,
                           TRUE ) == NO_ERROR );
    if ( fOK )
    {
      if ( UtlIsLongFileName( pIda->szAlias ) )
      {
        UtlLongToShortName( pIda->szAlias, pIda->szShortAlias );
      }
      else
      {
        strcpy( pIda->szShortAlias, pIda->szAlias );
        pIda->szAlias[0] = EOS;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Open translation memory                                          */
  /********************************************************************/
  if ( fOK )
  {
    fOK = (FolTMOpen( pIda ) == NO_ERROR );
  } /* endif */

  /********************************************************************/
  /* loop over all segments and update TM if translated segments have */
  /* been changed                                                     */
  /********************************************************************/
  if ( fOK )
  {
    PTBSEGMENT pSourceSeg;             // ptr to source segment
    PTBSEGMENT pTargetSeg;             // ptr to target segment
    ULONG ulSegNum;                    //

    ulSegNum = pIda->ulSegNum;         // get a copy of actual segment number

    pIda->ulSegNum = 1;
    while ( fOK && (pIda->ulSegNum <= (pIda->pTargetDoc->ulMaxSeg - 1) )  )
    {
      pSourceSeg = EQFBGetSegW( pIda->pSourceDoc, pIda->ulSegNum );
      pTargetSeg = EQFBGetSegW( pIda->pTargetDoc, pIda->ulSegNum );
      if ( (pTargetSeg->qStatus == QF_XLATED) && pTargetSeg->SegFlags.Changed )
      {
        fOK = FolTmReplace( pIda, pSourceSeg->pDataW, pTargetSeg->pDataW );
      } /* endif */
      pIda->ulSegNum++;
    } /* endwhile */

    // restore segment number
    pIda->ulSegNum = ulSegNum;
  } /* endif */

  /********************************************************************/
  /* Close translation memory                                         */
  /********************************************************************/
  if ( pIda->pMem != NULL )
  {
    FolTmClose( pIda );
  } /* endif */

  return( fOK );
} /* end of function FolFindUpdateTM */

BOOL FolFindCloseDoc
(
PFOLFINDDATA pIda,
BOOL        fFreeDoc
)
{
  BOOL fStopProcess = FALSE;

  if ( pIda->pTargetDoc )
  {
    if ( pIda->pTargetDoc->flags.changed  )
    {
//      CHAR   szDocFullPath[MAX_EQF_PATH];
      SHORT  sRc;
      USHORT usCPConv = 0;

      /******************************************************/
      /* Update TM if required                              */
      /******************************************************/
      if ( pIda->fTMUpdRequired )
      {
        if ( !FolFindUpdateTM( pIda ) )
        {
          fStopProcess = TRUE;
        } /* endif */
      } /* endif */

      pIda->pTargetDoc->docType = STARGET_DOC; // enable hist log processing
      sRc = EQFBFileWriteEx( pIda->pTargetDoc->szDocName, pIda->pTargetDoc, DOCSAVE_LOGTASK2, usCPConv );
      if ( sRc )
      {
        PSZ    pszParm = UtlGetFnameFromPath( pIda->szCurDocObjName );
        if ( pIda->szLongName[0] != EOS ) pszParm = pIda->szLongName;
        if ( UtlError( ERROR_FOLFIND_SAVE_SEGFILE, MB_OKCANCEL,
                       1, &pszParm, EQF_QUERY ) != MBID_OK )
        {
          fStopProcess = TRUE;
        } /* endif */
      }
      else
      {
        // set document touch date
        HPROP           hPropDocument;     // handle to document properties
        PPROPDOCUMENT   pPropDocument;     // pointer to document properties
        ULONG           ulErrorInfo;       // error indicator from PRHA

        //UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0],
        //                SYSTEM_PATH,
        //                UtlGetFnameFromPath(pIda->szFolObjName) );
        //strcat( szDocFullPath, BACKSLASH_STR );
        //strcat( szDocFullPath, pIda->szCurDoc );

        if ( (hPropDocument = OpenProperties( pIda->szCurDocObjName, NULL,
                                             PROP_ACCESS_READ,
                                             &ulErrorInfo))!= NULL)
        {
          pPropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hPropDocument );
          if ( SetPropAccess( hPropDocument, PROP_ACCESS_WRITE) )
          {
            UtlTime( (PLONG)&pPropDocument->ulTouched );
            SaveProperties( hPropDocument, &ulErrorInfo );
            ResetPropAccess( hPropDocument, PROP_ACCESS_WRITE);
          } /* endif */
          CloseProperties( hPropDocument, PROP_FILE, &ulErrorInfo);
        } /* endif */
      } /* endif */
    } /* endif */

    /********************************************************/
    /* Unlock document  (only if it has not been unlocked   */
    /* fo access by document open)                          */
    /********************************************************/
    if ( !pIda->fDocOpenActive )
    {
      //CHAR   szDocFullPath[MAX_EQF_PATH];
      //SHORT  sRc;

      //UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0],
      //                SYSTEM_PATH,
      //                UtlGetFnameFromPath(pIda->szFolObjName) );
      ////append document name to path
      //strcat( szDocFullPath, BACKSLASH_STR );
      //strcat( szDocFullPath, pIda->szCurDoc );

      //sRc = (SHORT)WinSendMsg( EqfQueryObjectManager(),
      //                         WM_EQF_REMOVESYMBOL,
      //                         NULL,
      //                         MP2FROMP( pIda->szCurDocObjName ) );
    }

    /******************************************************************/
    /* Free document area if requested                                */
    /******************************************************************/
    if ( fFreeDoc )
    {
      if ( pIda->pTargetDoc->pQFTagTable != NULL )
      {
        TAFreeTagTable( (PLOADEDTABLE)pIda->pTargetDoc->pQFTagTable );
      } /* endif */
      FolFindFreeDoc( pIda->pTargetDoc, pIda->pSourceDoc  );
      pIda->pTargetDoc = NULL;
      pIda->pSourceDoc = NULL;
    } /* endif */
  } /* endif */

  // clear object name of current document as no document is loaded anymore
  pIda->szCurDocObjName[0] = EOS;

  return( fStopProcess );
}

/**********************************************************************/
/* Check if document has been unlocked for document open and, if      */
/* this has been done, if it can be accessed and locked again         */
/* if document has been change dit is reloaded into memory            */
/**********************************************************************/
BOOL FolCheckForChangedDoc
(
PFOLFINDDATA pIda,
PBOOL       pfRefreshed              // document-has-been-refreshed flag
)
{
  BOOL fOK = TRUE;

  *pfRefreshed = FALSE;                // default value is not-refreshed

  if ( pIda->fDocOpenActive )
  {
    /******************************************************************/
    /* Check if document is still locked                              */
    /******************************************************************/
    //CHAR   szDocFullPath[MAX_EQF_PATH];
    SHORT  sRc;

    //UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0],
    //                SYSTEM_PATH,
    //                UtlGetFnameFromPath(pIda->szFolObjName) );
    ////append document name to path
    //strcat( szDocFullPath, BACKSLASH_STR );
    //strcat( szDocFullPath, pIda->szCurDoc );

    sRc = QUERYSYMBOL( pIda->szCurDocObjName );
    if ( sRc == -1 )
    {
      WORD wDate, wTime;
      CHAR   szDocFullPath[MAX_EQF_PATH];
      FILEFINDBUF stResultBuf;            // DOS file find struct
      USHORT     usCount = 1;             // number of files requested
      HDIR       hDirHandle = HDIR_CREATE;// DosFind routine handle

      // document is not in use ==> lock this document
      //SETSYMBOL( szDocFullPath );
      pIda->fDocOpenActive = FALSE;

      /*******************************************************/
      /* Get last update date to check for document changes  */
      /*******************************************************/
      UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0],
                      DIRSEGTARGETDOC_PATH,
                      UtlGetFnameFromPath(pIda->szFolObjName) );
      strcat( szDocFullPath, BACKSLASH_STR );
      strcat( szDocFullPath, UtlGetFnameFromPath( pIda->szCurDocObjName) );
      UtlFindFirst( szDocFullPath, &hDirHandle, FILE_NORMAL,
                    &stResultBuf, sizeof(stResultBuf),
                    &usCount, 0L, 0);
      // close search file handle
      if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

      FileTimeToDosDateTime( &stResultBuf.ftLastWriteTime,
                             (LPWORD)&wDate,
                             (LPWORD)&wTime );
      if ( (pIda->DocTime != wTime) ||
           (pIda->DocDate != wDate) )
      {
        /*************************************************************/
        /* Re-load of document required                              */
        /*************************************************************/
        *pfRefreshed = TRUE;
        FolFindFreeDoc( pIda->pTargetDoc, pIda->pSourceDoc );
        pIda->pTargetDoc  = NULL;
        pIda->pSourceDoc  = NULL;
        fOK = GFR_OpenDoc( pIda, FALSE, FALSE, FALSE );
        pIda->usOffs   = 0;           // restart at begin of current segment
        pIda->fCurrentSearchInSource = pIda->fSearchInSource;
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* Document still used by TPRO or another process               */
      /****************************************************************/
      PSZ    pszErrParm = UtlGetFnameFromPath( pIda->szCurDocObjName );
      if ( pIda->szLongName[0] != EOS ) pszErrParm = pIda->szLongName;
      UtlError( ERROR_FOLFIND_DOC_STILLINUSE,
                MB_CANCEL, 1,
                &pszErrParm, EQF_QUERY );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  return( fOK );
}

// mark a document in the document listbox as unusable
VOID MarkDocumentAsUnusable
(
  PFOLFINDDATA pIda,
  PSZ  pszErrorText
)
{
  QUERYITEMTEXT( pIda->hwndPages[0], ID_FOLFIND_DOCS_LB, pIda->iCurrentDocumentListboxItem, pIda->szNameBuffer );
  pIda->szNameBuffer[6] = '!';
  sprintf( pIda->szNameBuffer+strlen(pIda->szNameBuffer), " [%s]", pszErrorText );
  SETITEMTEXT( pIda->hwndPages[0], ID_FOLFIND_DOCS_LB, (SHORT)pIda->iCurrentDocumentListboxItem, pIda->szNameBuffer );
  SETITEMHANDLE( pIda->hwndPages[0], ID_FOLFIND_DOCS_LB, pIda->iCurrentDocumentListboxItem, -1 );
}

BOOL GFR_OpenDoc
(
PFOLFINDDATA pIda,
BOOL fCheckLock,                     // check for locking
BOOL fStartSearch,                   // start searching
BOOL fContinueNext                   // continue with nect doc flag
)
{
  BOOL fStopProcess = FALSE;
  BOOL fLocked = FALSE;
  BOOL fOK = TRUE;
  CHAR   szDocTgtLng[MAX_LANG_LENGTH];
  CHAR   szDocSrcLng[MAX_LANG_LENGTH];

  fContinueNext;

  /**********************************************************/
  /* Check if document is locked                            */
  /**********************************************************/
  if ( !fCheckLock )
  {
    fLocked = TRUE;
  }
  else
  {
    SHORT  sRc;

    // get folder long name
    SubFolObjectNameToName( pIda->szFolObjName, pIda->szFolLongName );

    // setup document object name
    UtlMakeEQFPath( pIda->szCurDocObjName, pIda->szFolObjName[0], SYSTEM_PATH, UtlGetFnameFromPath(pIda->szFolObjName) );
    strcat( pIda->szCurDocObjName, BACKSLASH_STR );
    strcat( pIda->szCurDocObjName, pIda->szCurDoc );

    // get document long name and markup
    pIda->szLongName[0] = EOS;
    DocQueryInfo2( pIda->szCurDocObjName, NULL, pIda->szDocFormat, szDocSrcLng, szDocTgtLng, pIda->szLongName, NULL, NULL, FALSE );

    sRc = QUERYSYMBOL( pIda->szCurDocObjName );
    if ( sRc == -1 )
    {
      //// document is not in use ==> lock this document
      //SETSYMBOL( pIda->szCurDocObjName );
      //fLocked = TRUE;
    }
    else
    {
      MarkDocumentAsUnusable( pIda, "In use" );
      fOK = FALSE;
    } /* endif */
  }

  /**********************************************************/
  /* allocate TBDOC structure                               */
  /**********************************************************/
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pIda->pTargetDoc, 0L, (LONG)sizeof(TBDOCUMENT), NOMSG );
    if ( fOK ) fOK = UtlAlloc( (PVOID *)&pIda->pSourceDoc, 0L, (LONG)sizeof(TBDOCUMENT), NOMSG );
    if ( !fOK )
    {
      pIda->usThreadError = ERROR_NOT_ENOUGH_MEMORY;
      fStopProcess = TRUE;
    } /* endif */
  } /* endif */

  /**********************************************************/
  /* load tag table                                         */
  /**********************************************************/
  if ( fOK )
  {
    SHORT  sRc;

    sRc = TALoadTagTable( DEFAULT_QFTAG_TABLE, (PLOADEDTABLE *)&pIda->pTargetDoc->pQFTagTable, TRUE, TRUE ); 
    fOK = (sRc == NO_ERROR);
    if ( fOK ) pIda->pSourceDoc->pQFTagTable = pIda->pTargetDoc->pQFTagTable;
  } /* endif */

  // load document markup table
  if ( fOK )
  {
    SHORT  sRc;
    sRc = TALoadTagTableExHwnd( pIda->szDocFormat, (PLOADEDTABLE *)&pIda->pTargetDoc->pDocTagTable, FALSE, TALOADPROTTABLEFUNC | TALOADUSEREXIT, TRUE, NULLHANDLE ); 
    fOK = (sRc == NO_ERROR);
    if ( fOK ) pIda->pSourceDoc->pDocTagTable = pIda->pTargetDoc->pDocTagTable;
  } /* endif */

  /**********************************************************/
  /* load the target file                                    */
  /**********************************************************/
  if ( fOK )
  {
    CHAR   szDocFullPath[MAX_EQF_PATH];
    SHORT  sRc;

    /********************************************************/
    /* build fully qualified name of segemented target doc  */
    /********************************************************/
    UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0], DIRSEGTARGETDOC_PATH, UtlGetFnameFromPath(pIda->szFolObjName) );
    //append document name to path
    strcat( szDocFullPath, BACKSLASH_STR );
    strcat( szDocFullPath, pIda->szCurDoc );

    // check if segmented target file exists, ignore document if file does not exist                               
    if ( UtlFileExist( szDocFullPath ) )
    {
      strcpy( pIda->pTargetDoc->szDocName, szDocFullPath );
      strcpy( pIda->pTargetDoc->szDocLongName, pIda->szLongName );

      // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to TgtLang
      pIda->pTargetDoc->ulOemCodePage = GetLangCodePage(OEM_CP, szDocTgtLng);
      pIda->pTargetDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, szDocTgtLng);
      MorphGetLanguageID( szDocTgtLng, &(pIda->sTgtLangID) );


      pIda->pTargetDoc->docType = STARGET_DOC; // enable hist log processing

      sRc = EQFBFileRead( szDocFullPath, pIda->pTargetDoc );

      if ( sRc != NO_ERROR )
      {
        // mark document as not readable
        MarkDocumentAsUnusable( pIda, "Read error" );
        fOK = FALSE;
      } /* endif */
    }
    else
    {
      MarkDocumentAsUnusable( pIda, "Not analyzed" );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /**********************************************************/
  /* load the source file                                   */
  /**********************************************************/
  if ( fOK )
  {
    CHAR   szDocFullPath[MAX_EQF_PATH];
    SHORT  sRc;

    UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0], DIRSEGSOURCEDOC_PATH, UtlGetFnameFromPath(pIda->szFolObjName) );
    strcat( szDocFullPath, BACKSLASH_STR );
    strcat( szDocFullPath, pIda->szCurDoc );

    // check if segmented source file exists, ignore document if file does not exist                               
    if ( UtlFileExist( szDocFullPath ) )
    {
      strcpy( pIda->pSourceDoc->szDocName, szDocFullPath );
      strcpy( pIda->pSourceDoc->szDocLongName, pIda->szLongName );

      // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to SrcLang
      pIda->pSourceDoc->ulOemCodePage = GetLangCodePage(OEM_CP, szDocSrcLng);
      pIda->pSourceDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, szDocSrcLng);
      MorphGetLanguageID( szDocSrcLng, &(pIda->sSrcLangID) );


      pIda->pTargetDoc->docType = SSOURCE_DOC; // disable hist log processing

      sRc = EQFBFileRead( szDocFullPath, pIda->pSourceDoc );

      if ( sRc != NO_ERROR )
      {
        // mark document as not readable
        MarkDocumentAsUnusable( pIda, "Read error" );
        fOK = FALSE;
      } /* endif */
    }
    else
    {
      MarkDocumentAsUnusable( pIda, "Not analyzed" );
      fOK = FALSE;
    } /* endif */
  } /* endif */


  /********************************************************/
  /* Unlock locked documents                              */
  /********************************************************/
  if ( !fOK && fLocked )
  {
    CHAR   szDocFullPath[MAX_EQF_PATH];

    UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0],
                    SYSTEM_PATH,
                    UtlGetFnameFromPath(pIda->szFolObjName) );
    //append document name to path
    strcat( szDocFullPath, BACKSLASH_STR );
    strcat( szDocFullPath, pIda->szCurDoc );

    //sRc = REMOVESYMBOL( szDocFullPath );
  } /* endif */

  /**********************************************************/
  /* start search at begin of document                      */
  /**********************************************************/
  if ( fOK )
  {
    pIda->fTMUpdRequired = FALSE;
    if ( fStartSearch )
    {
      GFR_ResetSearchPosition( pIda );
    } /* endif */
  }
  else
  {
    /********************************************************/
    /* Cleanup partially loaded document                    */
    /********************************************************/
    if ( pIda->pTargetDoc || pIda->pSourceDoc )
    {
      FolFindFreeDoc( pIda->pTargetDoc, pIda->pSourceDoc );
      pIda->pTargetDoc  = NULL;
      pIda->pSourceDoc  = NULL;
    } /* endif */
  } /* endif */
  return( fOK );
}

// Fill visible document listbox with document long names
USHORT GFR_FillDocListbox
(
HWND        hwnd,                    // dialog window handle
PFOLFINDDATA pIda                     // dialog IDA
)
{
  SHORT sItemCount = QUERYITEMCOUNTHWND( pIda->hwndLB );
  SHORT sItem = 0;
  PSZ pszLastFolder = NULL;
  PSZ pszCurFolder =  pIda->szFolObjName;
  HWND hwndDocLB = GetDlgItem( hwnd, ID_FOLFIND_DOCS_LB );

  while ( sItem < sItemCount )
  {
    SHORT sInsertItem;           // index of inserted item
    CHAR  szShortName[MAX_FILESPEC];

    // get the document name
    QUERYITEMTEXTHWND( pIda->hwndLB, sItem, szShortName );
    if ( pIda->fMultipleObjs ) pszCurFolder = (PSZ)QUERYITEMHANDLEHWND( pIda->hwndLB, sItem );


    // build document object name
    UtlMakeEQFPath( pIda->szDocObjName, pszCurFolder[0], SYSTEM_PATH, UtlGetFnameFromPath(pszCurFolder) );
    strcat( pIda->szDocObjName, BACKSLASH_STR );
    strcat( pIda->szDocObjName, szShortName );

    // add folder seperator when folder changes
    if ( pIda->fMultipleObjs && (pszLastFolder != pszCurFolder) )
    {
      SubFolObjectNameToName( pszCurFolder, pIda->szLongName );
      sprintf( pIda->szNameBuffer, "*** %s ***", pIda->szLongName );
      sInsertItem = INSERTITEMHWND( hwndDocLB, pIda->szNameBuffer );
      if ( sInsertItem >= 0 ) SETITEMHANDLEHWND( hwndDocLB, sInsertItem, -1 );
      pszLastFolder = pszCurFolder;
    } /* endif */

    // get the long name for this document
    pIda->szLongName[0] = EOS;
    DocQueryInfo2( pIda->szDocObjName, NULL, NULL, NULL, NULL, pIda->szLongName, NULL, NULL, TRUE );

    // add long name to long name listbox
    if ( pIda->szLongName[0] != EOS )
    {
      OEMTOANSI( pIda->szLongName );
      sprintf( pIda->szNameBuffer, "%4.4d    %s", sItem + 1, pIda->szLongName );
    }
    else
    {
      sprintf( pIda->szNameBuffer, "%4.4d    %s", sItem + 1, szShortName );
    } /* endif */
    sInsertItem = INSERTITEMENDHWND( hwndDocLB, pIda->szNameBuffer );

    // set item handle to index of document in invisible document LB
    if ( sInsertItem >= 0 )
    {
      SETITEMHANDLEHWND( hwndDocLB, sInsertItem, sItem );
    } /* endif */

    sItem++;
  } /* endwhile */
  return( NO_ERROR );
} /* end of function GFR_FillDocListbox */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     GFR_NLFCmp
//+----------------------------------------------------------------------------+
// Function call:     GFR_NLFCmp
//+----------------------------------------------------------------------------+
// Description:       compare w/o regard of lfs
//+----------------------------------------------------------------------------+
// Parameters:        PSZ         pData
//                    PSZ         pSearch
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:               0    - strings are equal up to blanks /lfs
//                    1 / -1       - not equal
//+----------------------------------------------------------------------------+
// Function flow:
//
//+----------------------------------------------------------------------------+
SHORT GFR_NLFCmp
(
  PSZ_W   pData,
  PSZ_W   pSearch,
  PUSHORT pusLen
)
{
  SHORT  sRc = 0;                               // init strings are equal
  USHORT usMatchLen = 0;

  CHAR_W   c, d;
  while ( ((d = *pSearch) != NULC) && ((c = *pData) != NULC) )
  {
    if (c == d )
    {
      pData++;
      pSearch++;
      usMatchLen++;
    }
    else if ( c == LF )
    {
      pData ++;
      usMatchLen++;
      if (d == BLANK )
      {
        pSearch++;
      } /* endif */
    }
    else
    {
      sRc = c-d;
      break;
    } /* endif */
  } /* endwhile */

  if (*pSearch )
  {
    sRc = -1;
  } /* endif */

  if ( pusLen )
  {
    *pusLen = usMatchLen;
  } /* endif */

  return sRc;
}

 VOID
 GFR_UpdHistory
 (
   PSZ_W pszSource,              //  pIda->szFind,
   PSZ_W pszTarget,              // pFllProp->szFolFind,
   PFINDNAME pHistList           // pFllProp->szFindList[0] );
  )
  {
    int iFound=-1;
    int i;
      UTF16strcpy( pszTarget, pszSource );

      // is insertion necessary ????
      for (i=0;i<MAX_SEARCH_HIST;i++)
      {
        if (!UTF16strcmp(pszSource,pHistList[i] )) iFound = i;
      }// end for

      if (iFound>=0)
      {
        for (i=iFound;i<(MAX_SEARCH_HIST-1);i++)
        {
          UTF16strcpy(pHistList[i] , pHistList[i+1]);
        }// end for
      } // end iFound

      for (i=(MAX_SEARCH_HIST-1);i>=1;i--)
      {
        UTF16strcpy(pHistList[i] , pHistList[i-1]);
      }// end for

      UTF16strcpy(pHistList[0] , pszSource);

      return;
  }

//
// Function GFR_MaskProtectedText
//
// Masks protected text within the segment with 0x2622 (unicode symbol for radio-activity)
//

#define FOLFINDMASKCHAR 0x2622

BOOL GFR_MaskProtectedText
(
  PSZ_W       pszSegment,              // ptr to segment data
  PLOADEDTABLE pDocTagTable,           // ptr to loaded tag table for document
  ULONG       ulOemCodePage,           // OEM codepage of document
  BOOL        fProtected               // TRUE = segment is a protected one
)
{
  BOOL fOK = TRUE;                     // function return code

  if ( fProtected )
  {
    // mask complete segment...
    PSZ_W pszTemp = pszSegment;
    while ( *pszTemp )
    {
      if ( *pszTemp != LF )
      {
        *pszTemp = FOLFINDMASKCHAR;
      } /* endif */
      pszTemp++;
    } /* endwhile */
  }
  else
  {
    PBYTE  pStartStop = NULL;
    USHORT usRC = 0;
    int          iIterations = 0;
    USHORT       usAddEntries = 0;
    PTOKENENTRY  pTokenList = NULL;

    do
    {
      // (re)allocate token buffer
      LONG lOldSize = (usAddEntries * sizeof(TOKENENTRY));
      LONG lNewSize = ((usAddEntries+128) * sizeof(TOKENENTRY));

      if (UtlAlloc((PVOID *) &pTokenList, lOldSize, lNewSize, NOMSG) )
      {
        usAddEntries += 128;
        iIterations++;
      }
      else
      {
        iIterations = 14;    // force end of loop
      } /* endif */

      // (re)try tokenization
      if (iIterations < 14 )
      {
        usRC = TACreateProtectTableW( pszSegment, pDocTagTable, 0, (PTOKENENTRY)pTokenList,
                                      (USHORT)lNewSize, (PSTARTSTOP *)&pStartStop,
                                      pDocTagTable->pfnProtTable,
                                      pDocTagTable->pfnProtTableW,
                                      ulOemCodePage);
      } /* endif */
    } while ((iIterations < 14) && (usRC == EQFRS_AREA_TOO_SMALL));

    // 'hide' protected text (i.e. replace text with masking character)
    if ( !usRC )
    {
      PSTARTSTOP pstCurrent = (PSTARTSTOP) pStartStop;

      while ( (pstCurrent->usType != 0) && !usRC )
      {
        if ( pstCurrent->usType == UNPROTECTED_CHAR )
        {
          // nothing to do
        }
        else
        {
          int i;
          for( i = pstCurrent->usStart; i <= pstCurrent->usStop; i++ )
            pszSegment[i] = FOLFINDMASKCHAR;
        } /* endif */
        pstCurrent++;
      } /* endwhile */
    }
    else
    {
      fOK = FALSE;                     // function failed
    } /* endif */

    // free protect table
    if ( pStartStop ) UtlAlloc( (PVOID *)&pStartStop, 0L, 0L, NOMSG );
    if ( pTokenList ) UtlAlloc( (PVOID *)&pTokenList, 0L, 0L, NOMSG );
  } /* endif */

  return( fOK );
} /* end of function GFR_MaskProtectedText */

//
// function GFR_MatchPattern
// prepare search pattern for pattern matching
//
// logic extracted from UtlMatchStringW
//
BOOL GFR_MatchPattern
(
  PSZ_W        pszMatch,               // string being compared
  PSZ_W        pszPattern,             // pattern for pattern matching
  PUSHORT      pusMatchlen,            // length of matching string
  BOOL        fWholeWordsOnly          // whole-words-only search flag
)
{
  BOOL       fMatch = TRUE;            // function return code: match flag
  PSZ_W      pszMatch2, pszMatch3;     // ptr for multiple substitution checks
  BOOL       fFound;                   // string found flag
  USHORT     usMatchLen = 0;           // length of matching string

  // Initialize compare result
  fMatch = TRUE;

  if ( !*pszMatch && !*pszPattern )
  {
    // Empty strings do always match ...
  }
  else
  {
    while ( fMatch && *pszPattern )
    {
      // skip LFs in text, treat as blank if a blank is at current position in pattern
      if ( *pszMatch == LF )
      {
        *pszMatch++;
        usMatchLen++;
        if (*pszPattern == BLANK )
        {
          pszPattern++;
        } /* endif */
      }
      else
      {
        if ( *pszPattern == chSingleSubst )
        {
          if ( !*pszMatch )
          {
            fMatch = FALSE; // no match as match string ends
          }
          else
          {
            pszMatch++;             // test next character
            usMatchLen++;
            pszPattern++;
          } /* endif */
        }
        else if ( *pszPattern == chMultSubst )
        {
          ++pszPattern;
          if ( *pszPattern )
          {
            PSZ_W pszMatchStart = pszMatch;

            // compare strings until next * or end of string found in second string
            // fFound is true if match found
            fFound = FALSE;
            while ( !fFound && fMatch )
            {
              // find character in first string matching first character after replacement symbol in second symbol                            */
              pszMatch = UTF16strchr( pszMatch, *pszPattern);

              // in whole word search: stop when word delimiter has been detected
              if ( fWholeWordsOnly && pszMatch )
              {
                PSZ_W pszTest = pszMatchStart;
                while ( (pszTest < pszMatch) && !GFR_isWordDelimiter(*pszTest) )
                {
                  pszTest++;
                } /*endwhile */
                if ( (pszTest != pszMatch))
                {
                  // word delimtiers contained in skipped characters, stop match
                  pszMatch = NULL;
                } /* endif */
              } /* endif */

              if (pszMatch == (PSZ_W)NULL)
                fMatch = FALSE; // strings do not match
              else
              {
                // check for matching of characters until next replacement character or end of
                // pattern string
                pszMatch3 = pszPattern;
                pszMatch2 = pszMatch;
                while (!fFound && fMatch )
                {
                  ++pszMatch2;                   /* first characters */
                  ++pszMatch3;                   /* already checked  */
                  if ( *pszMatch3 == EOS )
                  {
                                            /* substring has matched */
                    fFound = TRUE;
                    pszMatch = pszMatch2;        /* set pointers after */
                    pszPattern = pszMatch3;       /* last matching chars*/
                  }
                  else if (*pszMatch2 == EOS)  /* end of string      */
                    fMatch = FALSE; // strings do not match
                  else if (*pszMatch3 != chSingleSubst &&
                            *pszMatch2 != *pszMatch3)
                  {
                    ++pszMatch;     /*retry next char in first string*/
                    break;
                  }
                } /* endwhile */
              }
            } /* endwhile */

            // adjust match len
            if ( fFound )
            {
              usMatchLen = usMatchLen + (USHORT)(pszMatch - pszMatchStart);
            } /* endif */
          }
          else
          {
            // pattern string ends with multiple substitution character, so add all following characters
            // up to next word delimiter (?) to matching string
            while ( *pszMatch && !GFR_isWordDelimiter(*pszMatch) )
            {
              pszMatch++;
              usMatchLen++;
            } /*endwhile */
          } /* endif */
        }
        else
        {
          if (*pszMatch != *pszPattern) /* characters not equal */
          {
            fMatch = FALSE; // strings do not match
          }
          else
          {
            pszMatch++;             // test next character
            pszPattern++;
            usMatchLen++;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* end while (!fMatch && *pszPattern) */
  } /* endif */

  *pusMatchlen = usMatchLen;         // return length of matching string

  return( fMatch );
} /* end of function GFR_MatchPattern */

//
// function FolFind_PreparePattern
//
// prepare search pattern for pattern matching
//
// code extracted from UtlMatchStringW
//
void FolFind_PreparePattern
(
  PSZ_W       pszPattern               // pattern string being prepared
)
{
  PSZ_W       pszSource, pszTarget;    // ptr for pattern string pre-processing
  BOOL        fChanged = FALSE;        // pattern-has-been changed flag

  // convert substrings like ??*?? to ????* and compress multiple * to single *
  do
  {
    pszSource = pszTarget = pszPattern;
    fChanged = FALSE;
    while ( *pszSource )
    {
      while ( *pszSource && (*pszSource != chMultSubst) ) *pszTarget++ = *pszSource++;

      if ( *pszSource )
      {
        // combine multiple substitution characters
        while ( *(pszSource+1) == chMultSubst )
        {
          fChanged = TRUE;
          pszSource++;
        } /* endif */

        // Shift single substitution characters before multiple substitution characters
        if ( *(pszSource+1) == chSingleSubst )
        {
          fChanged = TRUE;

          // replace * with ?
          pszSource++;
          *pszTarget++ = chSingleSubst;

          // replace last ? with *
          while ( *(pszSource+1) == chSingleSubst )
          {
            *pszTarget++ = *pszSource++;
          } /* endwhile */

          pszSource++;
          *pszTarget++ = chMultSubst;

          if ( *(pszSource+1) != chMultSubst )
          {
            *pszTarget++ = *pszSource++;
          } /* endif */
        }
        else
        {
          *pszTarget++ = *pszSource++;
        } /* endif */
      } /* endif */
    } /* endwhile */
    *pszTarget = 0;
  } while ( fChanged );
  return;
} /* end of function FolFind_PreparePattern */

//
// function GFR_isWordDelimiter
//
// Check if given character may be a word delimiter
//
BOOL GFR_isWordDelimiter
(
  CHAR_W      chTest                   // character being tested
)
{
  BOOL        fIsWordDelimiter = FALSE;// function return code

  switch ( chTest )
  {
    case L'.' :
    case L'!' :
    case L'?' :
    case L',' :
    case L';' :
    case L':' :
    case L'/' :
    case L'\\' :
    case L'\n' :
    case L'\r' :
    case L' ' :
    case L'(' :
    case L')' :
    case L'\"' :
    case L'\'' :
    case L'+' :
    case L'=' :
    case L'[' :
    case L']' :
    case L'{' :
    case L'}' :
    case L'<' :
    case L'>' :
        fIsWordDelimiter = TRUE;
        break;
    default:
        fIsWordDelimiter = FALSE;
        break;
  } /*endswitch */
  return( fIsWordDelimiter );
} /* end of function GFR_isWordDelimiter */

//
// function GFR_PreProcessFindString
//
// Pre-process specified find string and check if logical operators have been used
//
BOOL GFR_PreProcessFindString
(
  PSZ_W       pszFind,                 // specified find string (normally in uppercased form)
  PBOOL       pfLogicExpression,       // pointer to fLogicExpression flag, TRUE = logic expression
                                       //   has been used in find string
  PSZ_W       pszExpressionBuffer      // pointer to buffer for preprocessed expression
)
{
  BOOL        fOK = TRUE;              // function return code
  PSZ_W       pszCurPos;               // ptr for find string processing
  PSZ_W       pszExpression;           // ptr for expression processing
  BOOL        fQuotesUsed = FALSE;     // TRUE = quotes used within expression
  enum
  {
    FF_NO_MODE,                        // no mode is active
    FF_STRING_MODE                      // currently adding string elements
  } Mode;                              // current operation mode

  // prepare preprocesing
  *pfLogicExpression = FALSE;
  pszCurPos =  pszFind;
  pszExpression = pszExpressionBuffer;
  Mode = FF_NO_MODE;

  // process until end of find string...
  while ( *pszCurPos )
  {
    PSZ_W   pszStartToken = pszCurPos;

    // skip any whitespace up to start of next token
    while ( *pszStartToken  && iswspace(*pszStartToken) ) pszStartToken++;

    // handle token
    if ( *pszStartToken == L'\"' )
    {
      // handle string enclosed in double quotes
      BOOL fComplete = FALSE;

      fQuotesUsed = TRUE;

      // ingore any whitespace prefixeing string
      pszCurPos = pszStartToken;

      // start new string if none is active
      if ( Mode != FF_STRING_MODE )
      {
        *pszExpression++ = FF_STRING_OP;
        Mode = FF_STRING_MODE;
      } /* endif */

      // add anything up to closing doublequote as text element
      pszCurPos++;
      while ( *pszCurPos && !fComplete )
      {
        if ( *pszCurPos == L'\"' )
        {
          // handle end of string or duplicated double quote
          pszCurPos++;
          if ( *pszCurPos == L'\"' )
          {
            // treat as single double quote
            *pszExpression++ = *pszCurPos++;
          }
          else
          {
            // end quoted string
            fComplete = TRUE;
          } /* endif */
        }
        else
        {
          *pszExpression++ = *pszCurPos++;
        } /* endif */
      } /*endwhile */
    }
    else
    {
      PSZ_W pszEnd;                    // end of token
      CHAR_W chTemp;
      PFOLFINDOPERATOR pOp = FolFindOperators;

      // find end of token and terminate it
      pszEnd = pszStartToken;
      while ( *pszEnd && !iswspace(*pszEnd) ) pszEnd++;
      chTemp = *pszEnd;
      *pszEnd = 0;

      // check token against list of operators
      while ( pOp->szName[0] )
      {
        if ( UTF16stricmp( pszStartToken, pOp->szName ) == 0 )
        {
           break;                     // leave while loop
        } /* endif */
        pOp++;                       // try next one
      } /*endwhile */

      // handle token or string
      if ( pOp->szName[0] )
      {
        // complete any active string
        if ( Mode == FF_STRING_MODE )
        {
          *pszExpression++ = 0;
          Mode = FF_NO_MODE;
        } /* endif */

        // add operator ID to expression
        *pszExpression++ = (wchar_t)pOp->ID;
        *pfLogicExpression = TRUE;
      }
      else
      {
        // seems to be normal string..

        // include preceeding whitespace if a string is already active
        if ( Mode == FF_STRING_MODE ) pszStartToken = pszCurPos;

        // start new string if none is active
        if ( Mode != FF_STRING_MODE )
        {
          *pszExpression++ = FF_STRING_OP;
          Mode = FF_STRING_MODE;
        } /* endif */

        // add string token to string expression
        while ( *pszStartToken ) *pszExpression++ = *pszStartToken++;
      } /* endif */

      // restore token end delimiter
      *pszEnd = chTemp;

      // continue with next character following the token
      pszCurPos = pszEnd;
    } /* endif */
  } /*endwhile */

  // terminate expression list
  if ( Mode == FF_STRING_MODE )
  {
    *pszExpression++ = 0;              // terminate string
  } /* endif */
  *pszExpression = 0;                  // terminate expression list

  // if no logic expression has been used, use original search string as string expression
  if ( fOK && !*pfLogicExpression && !fQuotesUsed )
  {
    pszExpression = pszExpressionBuffer;
    pszCurPos =  pszFind;

    *pszExpression++ = FF_STRING_OP;

    while ( *pszCurPos )
    {
      if ( *pszCurPos == L'\"' )
      {
        pszCurPos++;
        while ( *pszCurPos && *pszCurPos != L'\"' )
        {
          *pszExpression++ = *pszCurPos++;
        } /*endwhile */
        if ( *pszCurPos ) pszCurPos++;
      }
      else
      {
        *pszExpression++ = *pszCurPos++;
      } /* endif */
    } /*endwhile */

    *pszExpression++ = 0;              // terminate string
    *pszExpression = 0;                // terminate expression list
  } /* endif */

  return( fOK );

} /* end of function GFR_PreProcessFindString */

//
// function GFR_SearchString
//
// Search the givern pattern in the string data
//
BOOL GFR_SearchString
(
  PSZ_W       pszData,                 // segment data being searched
  PSZ_W       pszPattern,              // string or pattern being seached for
  USHORT      usStartOffs,             // start offset to begin search
  PUSHORT     pusFoundPos,             // ptr to buffer for position of matching string
  PUSHORT     pusFoundLen,             // ptr to buffer for length of matching string
  BOOL        fWholeWordsOnly          // TRUE = search whole words only
)
{
  BOOL        fFound = FALSE;          // function return code
  PSZ_W       pData = pszData + usStartOffs;  // current position within input data
  USHORT      usOffs = usStartOffs;    // offset of found match
  USHORT      usLen  = 0;              // length of found match

  while ( !fFound && *pData )
  {
    CHAR_W chChar = pszPattern[0];

    if ( fWholeWordsOnly )
    {
      // find next start of a word and check first character of the word
      CHAR_W chLastChar = ( usOffs == 0 ) ? L' ' : pData[-1];

      // find next word delimiter
      while ( (*pData != EOS) && !GFR_isWordDelimiter(chLastChar) ) chLastChar = *pData++;

      // skip multiple word delimiters
      while ( (*pData != EOS) && GFR_isWordDelimiter(*pData) ) *pData++;
    }
    else
    {
      // Scan for the 1st letter of the target string (don't use strchr, it is too slow)
      if ( (chChar == chMultSubst) || (chChar == chSingleSubst) )
      {
        // no search necessary
      }
      else
      {
        CHAR_W c;

        while (((c = *pData) != NULC) && ((chChar != c) != NULC))  pData++;
      } /* endif */
    } /* endif */

    if ( *pData )
    {
      // then check for complete match
      if ( GFR_MatchPattern( pData, pszPattern, &usLen, fWholeWordsOnly ) )
      {
        fFound = TRUE;

        // when looking for whole words only, check if found string is delimited by a word delimiter
        if ( fWholeWordsOnly )
        {
          PSZ_W  pszWordEnd = pData + usLen;
          if ( (*pszWordEnd == EOS) || GFR_isWordDelimiter(*pszWordEnd) )
          {
            // ok, we can use this match
          }
          else
          {
            // reset found flag, skip all characters up to next word
            fFound = FALSE;
            usOffs = (USHORT)(pszWordEnd - pszData);
            while ( (*pszWordEnd != EOS) && !GFR_isWordDelimiter(*pszWordEnd) )
            {
              pszWordEnd++;
              usOffs++;
            } /*endwhile */
            pData = pszData + usOffs;
          } /* endif */
        } /* endif */

        if ( fFound )
        {
          usOffs = (USHORT)(pData - pszData);
        } /* endif */
      }
      else         // no match; go on if possible
      {
        if ( *pData ) pData++;         // goto next char
        usOffs = (USHORT)(pData - pszData);
      } /* endif */
    } /* endif */
  } /* endwhile */

  if ( fFound)
  {
    *pusFoundPos = usOffs;
    *pusFoundLen = usLen;
  } /* endif */

  return( fFound );
} /* end of function GFR_SearchString */

//
// function GFR_EvaluateExpression
//
// Apply a search expression on the current segment
//
BOOL GFR_EvaluateExpression
(
  PSZ_W       *ppszExpression,         // expression evaluation pointer
  PBOOL       pfResult,                // ptr to buffer fo evaluation result
  PSZ_W       pszSegment,              // ptr to segment data
  USHORT      usStartOffs,             // start offset to begin search
  PUSHORT     pusMatchPos,             // offset of matching string
  PUSHORT     pusMatchLen,             // length of matching string
  BOOL        fWholeWordsOnly,         // whole-words-only search flag
  PFOLFIND_FINDLIST pFindList          // list of find locations in current segment
)
{
  BOOL fResult = FALSE;

  // get result of first expression
  BOOL fOK = GFR_EvaluateSingleExpression( ppszExpression, &fResult, pszSegment, usStartOffs, pusMatchPos, pusMatchLen, fWholeWordsOnly, pFindList );

  // restart at begin of segment
  //usStartOffs = 0;

  // process any remaining part of expression
  while ( fOK && **ppszExpression )
  {
    CHAR_W chCurOp = **ppszExpression;
    *ppszExpression += 1;

    switch ( chCurOp )
    {
      case FF_AND_OP :
      case FF_OR_OP :
        {
          USHORT usMatchPos = 0;
          USHORT usMatchLen = 0;
          BOOL   fResult2 = FALSE;
          fOK = GFR_EvaluateSingleExpression( ppszExpression, &fResult2, pszSegment, usStartOffs, &usMatchPos, &usMatchLen, fWholeWordsOnly, pFindList );
          if ( fOK )
          {
            // use new match position of no one set yet
            if ( !*pusMatchPos && !*pusMatchLen )
            {
              *pusMatchPos = usMatchPos;
              *pusMatchLen = usMatchLen;
            } /* endif */

            if ( chCurOp == FF_AND_OP )
            {
              fResult = fResult && fResult2;
            }
            else
            {
              fResult = fResult || fResult2;
            } /* endif */
          } /* endif */
        }
        break;
      default:
        // error in expression
        fOK = FALSE;
        break;
    } /*endswitch */

  } /*endwhile */
  *pfResult = fResult;
  return( fOK );
} /* end of function GFR_EvaluateExpression */

//
// function GFR_EvaluateSingleExpression
//
// Evaluate a single expression (search string and negation)
//
BOOL GFR_EvaluateSingleExpression
(
  PSZ_W       *ppszExpression,         // expression evaluation pointer
  PBOOL       pfResult,                // ptr to buffer fo evaluation result
  PSZ_W       pszSegment,              // ptr to segment data
  USHORT      usStartOffs,             // start offset to begin search
  PUSHORT     pusMatchPos,             // offset of matching string
  PUSHORT     pusMatchLen,             // length of matching string
  BOOL        fWholeWordsOnly,         // whole-words-only search flag
  PFOLFIND_FINDLIST pFindList          // list of find locations in current segment
)
{
  BOOL        fOK = TRUE;              // function return code
  CHAR_W      chOperatorID;            // ID of current exression operator

  // get current operator and go to next one
  chOperatorID = **ppszExpression;
  *ppszExpression += 1;

  // handle current operator
  switch ( chOperatorID  )
  {
    case FF_STRING_OP:
      // get match state of current string
      *pfResult = GFR_SearchString( pszSegment, *ppszExpression, usStartOffs, pusMatchPos, pusMatchLen, fWholeWordsOnly );
      // skip expression string
      *ppszExpression += UTF16strlenCHAR( *ppszExpression ) + 1;
      // remember match position
      if ( (pFindList != NULL) && (pFindList->iEntries < MAX_FINDENTRIES) ) 
      {
        int i = pFindList->iEntries;
        pFindList->aiStartPos[i] = *pusMatchPos;
        pFindList->aiEndPos[i]   = *pusMatchPos + *pusMatchLen;
        pFindList->iEntries      = i + 1;
      } /* endif */
      break;
    case FF_NOT_OP:
      fOK = GFR_EvaluateSingleExpression( ppszExpression, pfResult, pszSegment, usStartOffs, pusMatchPos, pusMatchLen, fWholeWordsOnly, pFindList );
      *pfResult = !*pfResult;
      break;
    case FF_OR_OP:
    case FF_AND_OP:
    case FF_EMPTY_OP:
    default:
      // something seems to be wrong....
      fOK = FALSE;
      break;
  } /*endswitch */
  return( fOK );
} /* end of function GFR_EvaluateSingleExpression */

//
// function GFR_CheckAndPrepareExpression
//
// Apply a search expression on the current segment
//
BOOL GFR_CheckAndPrepareExpression
(
  PSZ_W       pszExpression,           // expresion being prepared and checked
  BOOL        fCaseIgnore              // case inore flag (for uppercasing of search strings
)
{
  BOOL fOK = TRUE;                     // function return code
  CHAR_W chLastOp = FF_EMPTY_OP;       // buffer for last expression operator

  // check syntax of search expression
  {
    PSZ_W  pszTemp = pszExpression;

    while ( fOK && *pszTemp )
    {
      CHAR_W chCurOp = *pszTemp++;

      switch ( chCurOp )
      {
        case FF_STRING_OP:
          // can appear anythere, just skip string part
          pszTemp += UTF16strlenCHAR( pszTemp ) + 1;
          break;
        case FF_NOT_OP:
          // has to be followed by another NOT or a string
          if ( (*pszTemp != FF_NOT_OP) && (*pszTemp != FF_STRING_OP) )
          {
            fOK = FALSE;
          } /* endif */
          break;
        case FF_OR_OP:
        case FF_AND_OP:
          // must be prefixed by a string operator and has to be followed by NOT or string
          if ( (chLastOp != FF_STRING_OP) ||
               ((*pszTemp != FF_NOT_OP) && (*pszTemp != FF_STRING_OP)) )
          {
            fOK = FALSE;
          } /* endif */
          break;
        case FF_EMPTY_OP:
        default:
          // should nether be contained in expression
          fOK = FALSE;
          break;
      } /*endswitch */
      chLastOp = chCurOp;
    } /*endwhile */
  }

  // do string expression pre-processing
  if ( fOK )
  {
    PSZ_W  pszSource = pszExpression;
    PSZ_W  pszTarget = pszExpression;

    while ( *pszSource )
    {
      if ( *pszSource == FF_STRING_OP )
      {
        // copy string operator
        *pszTarget++ = *pszSource++;

        // set to uppercase for case-insensitive search
        if ( fCaseIgnore )
        {
          UtlUpperW( pszSource );
        } /* endif */

        // preprocess wildcards
        FolFind_PreparePattern( pszSource );

        // copy to target expression
        while ( *pszSource ) *pszTarget++ = *pszSource++;
        *pszTarget++ = *pszSource++;        // copy end delimiter
      }
      else
      {
        // copy any other operator
        *pszTarget++ = *pszSource++;
      } /* endif */
    } /*endwhile */
    *pszTarget = 0;
  } /* endif */
  return( fOK );
} /* end of function GFR_CheckAndPrepareExpression */

//
// set the color of a text area in the rich edit control
void GFR_SetColor
(
  HWND        hwndMLE,                 // handle of rich edit control
  int         iStart,                  // start pos for colored area
  int         iEnd,                    // end pos of colored area
  COLORREF    colorText,               // text color
  COLORREF    colorBackground          // background color
)
{
  CHARFORMAT2 Format;

  MLESETSELHWND( hwndMLE, iStart, iEnd );
  memset( &Format, 0, sizeof(Format) );
  Format.cbSize = sizeof(Format);
  Format.dwMask = CFM_COLOR | CFM_BACKCOLOR;
  Format.crTextColor = colorText;
  Format.crBackColor = colorBackground;
  SendMessage( hwndMLE, EM_SETCHARFORMAT, SCF_SELECTION, MP2FROMP(&Format) );
} /* end of function GFR_SetColor */


// prepare search for a folder
BOOL GFR_PrepFolderSearch
(
  HWND hwnd,
  PFOLFINDDATA pIda
)
{
  BOOL fOK = TRUE;
  BOOL fNewUseThaiFont = FALSE;

  /* Get folder language                                        */
  if ( fOK)
  {
      EQFINFO     ErrorInfo;       // error code of property handler calls
      PVOID  hFolProp;
      PPROPFOLDER pFolProp;
      CHAR    szSysDrive[MAX_DRIVE];
      CHAR   chFolderDrive;

      // Use Sytem drive to retrieve folder properties
      UtlQueryString( QST_PRIMARYDRIVE, szSysDrive, sizeof(szSysDrive) );

      // save original folder drive
      chFolderDrive = pIda->szFolObjName[0];
      pIda->szFolObjName[0]=szSysDrive[0];

      hFolProp = OpenProperties( pIda->szFolObjName, NULL, PROP_ACCESS_READ, &ErrorInfo );
      if ( hFolProp )
      {
        pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
        pIda->fUseThaiFont = FALSE;
        if ( (stricmp(pFolProp->szSourceLang, THAI_STR) == 0) || (stricmp(pFolProp->szTargetLang, THAI_STR) == 0) )
        {
        } /* endif */
        CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
      } /* endif */
      // restore folder drive
      pIda->szFolObjName[0] = chFolderDrive;
  }

  if ( fOK )
  {
    if ( pIda->fUseThaiFont != fNewUseThaiFont )
    {
      if ( fNewUseThaiFont )  // src or tgt lng is Thai
      {
        SetCtrlFnt( hwnd, THAI_CHARSET, ID_FOLFIND_CHANGE_EF, ID_FOLFIND_FIND_EF );
      }
      else
      {
        if ( pIda->fUseThaiFont )
        {
          DelCtrlFont( hwnd, ID_FOLFIND_CHANGE_EF );
          DelCtrlFont( hwnd, ID_FOLFIND_FIND_EF );
        } /* endif */
      } /* endif */
      pIda->fUseThaiFont = fNewUseThaiFont;
    } /* endif */
  } /* endif */

  /* Load document names                                        */
  if ( fOK )
  {
    if ( pIda->fMultipleObjs )
    {
      HWND hwndTemp = WinCreateWindow( pIda->hwnd, WC_LISTBOX, "", 0L, 0, 0, 0, 0, pIda->hwnd, HWND_TOP, 2, NULL, NULL );
      PSZ pszFolder = pIda->pszObjList;

      // for all folders do...
      while ( *pszFolder )
      {
        int i = 0;
        int docs = 0;

        // fill temp listbox with documents of folder
        DELETEALLHWND( hwndTemp );
        EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND(hwndTemp), MP2FROMP(pszFolder) );

        // copy documents to document listbox and set item handle to folder object name
        docs = QUERYITEMCOUNTHWND( hwndTemp );
        i = 0;
        while ( i < docs )
        {
          int inserted = 0;
          QUERYITEMTEXTHWND( hwndTemp, i, pIda->szNameBuffer );
          inserted = INSERTITEMENDHWND( pIda->hwndLB, pIda->szNameBuffer );
          if ( inserted >= 0 ) SETITEMHANDLEHWND( pIda->hwndLB, inserted, (LONG)pszFolder );
          i++;
        } /*endwhile */

        // next folder
        pszFolder += strlen(pszFolder) + 1;
      } /*endwhile */
      WinDestroyWindow( hwndTemp );
    }
    else if ( pIda->fFromFolderList )
    {
      EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                        MP1FROMHWND(pIda->hwndLB),
                        MP2FROMP(pIda->szSubFolObjName) );
    }
    else
    {
      EqfSend2Handler( FOLDERHANDLER, WM_EQF_QUERYSELECTEDNAMES,
                        MP1FROMHWND(pIda->hwndLB),
                        MP2FROMP(pIda->szSubFolObjName) );
    } /* endif */
    DELETEALL( hwnd, ID_FOLFIND_DOCS_LB );
    GFR_FillDocListbox( hwnd, pIda );
  } /* endif */

  /* Set document listbox horizontal screen width */
  if ( fOK )
  {
    HWND hwndLB = GetDlgItem( pIda->hwnd, ID_FOLFIND_DOCS_LB);
    UtlSetHorzScrollingForLB(hwndLB);
  }

  return( fOK );
}

/*! \brief Replace a commbobox with its Unicode enabled version
  \param hwndDlg handle of dialog window
  \param iComboBoxID numeric identifier of the control
  \returns TRUE when successful, or FALSE when the new control could not be created
*/
BOOL GFR_ReplaceWithUnicodeComboBox
( 
  HWND hwndDlg,                        // window handle of dialog window
  int  iComboBoxID                     // ID of combobox
)
{
  WINDOWPLACEMENT Placement;
  WINDOWINFO Info;

  // get values from window being replaced
  HWND hwndCombo = GetDlgItem( hwndDlg, iComboBoxID );
  GetWindowPlacement( hwndCombo, &Placement );
  GetWindowInfo( hwndCombo, &Info );

  // destroy the window
  DestroyWindow( hwndCombo );

  // create a new one
  hwndCombo = CreateWindowExW( 0, L"ComboBox", L"", Info.dwStyle | CBS_AUTOHSCROLL, 
                               Placement.rcNormalPosition.left, Placement.rcNormalPosition.top,
                               Placement.rcNormalPosition.right - Placement.rcNormalPosition.left,
                               UtlQueryULong( QL_PELSPERLINE ) * 10,
                               hwndDlg, (HMENU)iComboBoxID, (HINSTANCE)(HAB)UtlQueryULong( QL_HAB ), 0 );

  if ( hwndCombo != NULL )
  {
    SetWindowLong( hwndCombo, GWL_ID, iComboBoxID );
    SetWindowPos( hwndCombo, HWND_TOP, 0, 0, Placement.rcNormalPosition.right - Placement.rcNormalPosition.left,
                  UtlQueryULong( QL_PELSPERLINE ) * 10, SWP_NOACTIVATE | SWP_NOMOVE );
  }

  return( hwndCombo != NULL );
}



BOOL GFR_ClearResultList( PFOLFINDDATA pIda )
{
  if ( pIda->ppResultList != NULL )
  {
    for( int i = 0; i < pIda->iResultListUsed; i++ )
    {
      if ( pIda->ppResultList[i]->pszChangeToBuffer != NULL )
      {
        UtlAlloc( (PVOID *)&(pIda->ppResultList[i]->pszChangeToBuffer), 0, 0, NOMSG );
      }
      UtlAlloc( (PVOID *)&(pIda->ppResultList[i]), 0, 0, NOMSG );
    }
    pIda->iResultListUsed = 0;
    pIda->iResultsDisplayed = 0;
  }
  return( TRUE );
}

// add a found position to the foundpos list of an result entry
BOOL GFR_AddFindPosToResultList( PFOLFINDDATA pIda, PFOLFINDRESULTENTRY *ppEntry, BOOL fTarget, USHORT usOffs, USHORT  usLen, FINDPOSTYPE Type, USHORT usChangeToOffs )
{
  PFOLFINDRESULTENTRY pEntry = NULL; 
  PFOLFINDRESULTPOS pFoundPos = NULL;

  // enlarge the result entry if necessary
  if ( (*ppEntry)->iAllocatedEntries < ((*ppEntry)->iUsedEntries + 1) )
  {
    int iOldLen = (*ppEntry)->iSize;
    int iNewLen = (*ppEntry)->iSize + (sizeof(FOLFINDRESULTPOS) * 10); 
    if ( !UtlAlloc( (PVOID *)ppEntry, iOldLen, iNewLen, NOMSG ) )
    {
      pIda->usThreadError = ERROR_NOT_ENOUGH_MEMORY;
      strcpy( pIda->szStatusLine, ERROR_MEMORY_STRING );
      return( FALSE );
    }
    (*ppEntry)->iSize = iNewLen;
    (*ppEntry)->iAllocatedEntries += 10;
  }

  // find correct position for the insertion of the new find position (based on the offset)
  pEntry = *ppEntry; 
  pFoundPos = (PFOLFINDRESULTPOS)(((PBYTE)pEntry) + pEntry->iResultPosOffs);
  int iPos = 0;
  while( (iPos < pEntry->iUsedEntries) && (pFoundPos->usOffs < usOffs) )
  {
    iPos++;
    pFoundPos++;
  }

  // make room for new entry if we are not at the end of the list
  if ( iPos < pEntry->iUsedEntries )
  {
    memmove( pFoundPos + 1, pFoundPos, sizeof(FOLFINDRESULTPOS) * (pEntry->iUsedEntries - iPos) );
  }

  // add find position information
  {
    pFoundPos->fTarget = fTarget;
    pFoundPos->usOffs = usOffs;
    pFoundPos->usLen = usLen;
    pFoundPos->Type = Type;
    if ( Type != FOUND_TYPE ) pFoundPos->usChangeOffs = usChangeToOffs;
    pEntry->iUsedEntries += 1;
  }

  return( TRUE );
}


// add a result from a find operation to the result list
BOOL GFR_AddFindResultToList
( 
  PFOLFINDDATA pIda,                     // dialog IDA
  int         iDocIndex,                // index of document
  PSZ         pszDocObjName,            // document object name
  BOOL        fTarget,                  // TRUE = found in target, FALSE = found in source
  ULONG       ulSegNum,                 // number of segment
  BOOL        fTranslated,              // TRUE = segment is translated
  PSZ_W       apszText[MAX_NUM_OF_TEXTS],// table containing the text strings for this entry
  USHORT      usOffs,                   // offset within segment
  USHORT      usLen                     // length of found string
)
{
  FINDPOSTYPE Type;                     // type of new foundpos entry
  USHORT      usChangeToOffs = 0;       // offset of change-to string within the entry
  BOOL        fOK = TRUE;

  // prepare type of foundpos entry
  if ( !pIda->fFind && fTarget && fTranslated )
  {
    Type = pIda->fConfirm ? CHANGETO_TYPE : AUTOCHANGED_TYPE;
  }
  else
  {
    Type = FOUND_TYPE;
  } /* endif */

  // create a new result entry or add to current one?
  PFOLFINDRESULTENTRY pCurEntry = (pIda->iResultListUsed != 0 ) ? pIda->ppResultList[pIda->iResultListUsed-1] : NULL;

  if ( (pCurEntry != NULL ) && (pCurEntry->iDocIndex == iDocIndex) && (pCurEntry->ulSegNum == ulSegNum ) )
  {
    // we can use the existing entry
  }
  else
  {
    // create a new entry to the end of the result list
    PFOLFINDRESULTENTRY pNewEntry = NULL;
    int aiTextLen[MAX_NUM_OF_TEXTS]; 
    int iTotalTextLen = 0;

    for( int i = 0; i < MAX_NUM_OF_TEXTS; i++ )
    {
      if ( (apszText[i] == NULL) || (apszText[i][0] == 0) )
      {
        aiTextLen[i] = 0;
      }
      else
      {
        aiTextLen[i] = (wcslen(apszText[i]) + 1) * sizeof(CHAR_W);
        iTotalTextLen += aiTextLen[i]; 
      }
    }

    int iLen = sizeof(FOLFINDRESULTENTRY) + iTotalTextLen + (sizeof(FOLFINDRESULTPOS) * 10);
    if ( !UtlAlloc( (PVOID *)&pNewEntry, 0, iLen, NOMSG ) )
    {
      pIda->usThreadError = ERROR_NOT_ENOUGH_MEMORY;
      strcpy( pIda->szStatusLine, ERROR_MEMORY_STRING );
      return( FALSE );
    }

    // fill fixed part of the new entry and add strings
    memset( pNewEntry, 0, iLen );
    pNewEntry->iSize = iLen;
    pNewEntry->fTranslated = fTranslated;
    pNewEntry->iAllocatedEntries = 10;
    pNewEntry->iUsedEntries = 0;
    pNewEntry->iDocIndex = iDocIndex;
    pNewEntry->iChangeToBufferSize = 0;
    pNewEntry->iChangeToBufferUsed = 0;
    pNewEntry->pszChangeToBuffer = NULL;
    strcpy( pNewEntry->szDocObjName, pszDocObjName );
    pNewEntry->ulSegNum = ulSegNum;
    int iCurOffs = sizeof(FOLFINDRESULTENTRY);
    for( int i = 0; i < MAX_NUM_OF_TEXTS; i++ )
    {
      pNewEntry->iTextLen[i] = aiTextLen[i];
      pNewEntry->iTextOffs[i] = iCurOffs;
      if ( aiTextLen[i] != 0 ) wcscpy( (PSZ_W)((PBYTE)pNewEntry + iCurOffs), apszText[i] );
      iCurOffs += aiTextLen[i];
    }
    pNewEntry->iResultPosOffs = iCurOffs;

    // alloc/re-alloc result table if necessary
    if ( pIda->iResultListSize < (pIda->iResultListUsed + 1) )
    {
      int iOldLen = pIda->iResultListSize * sizeof(PFOLFINDRESULTENTRY); /* yes, this is correct, we allocate a list of pointers...*/  
      int iNewLen = (pIda->iResultListSize + 1000) * sizeof(PFOLFINDRESULTENTRY); 
      if ( !UtlAlloc( (PVOID *)&(pIda->ppResultList), iOldLen, iNewLen, NOMSG ) )
      {
        pIda->usThreadError = ERROR_NOT_ENOUGH_MEMORY;
        strcpy( pIda->szStatusLine, ERROR_MEMORY_STRING );
        return( FALSE );
      }
      pIda->iResultListSize += 1000;
    }

    // add the new entry to the list
    pIda->ppResultList[pIda->iResultListUsed] = pNewEntry;
    pIda->iResultListUsed += 1;
    pCurEntry = pNewEntry;
  }

  // add any change-to string to the record
  if ( Type != FOUND_TYPE )
  {
    int iChangeToLen = (wcslen(pIda->szChangeTo) + 1 ) * sizeof(CHAR_W);

    // enlarge current change-to string buffer when necessary
    if ( (pCurEntry->iChangeToBufferUsed + iChangeToLen) >= pCurEntry->iChangeToBufferSize )
    {
      int iOldLen = pCurEntry->iChangeToBufferSize;
      int iNewLen = pCurEntry->iChangeToBufferSize + (4096 * sizeof(CHAR_W)); 
      if ( !UtlAlloc( (PVOID *)&(pCurEntry->pszChangeToBuffer), iOldLen, iNewLen, NOMSG ) )
      {
        pIda->usThreadError = ERROR_NOT_ENOUGH_MEMORY;
        strcpy( pIda->szStatusLine, ERROR_MEMORY_STRING );
        return( FALSE );
      }
      pCurEntry->iChangeToBufferSize = iNewLen;
    }

    // add new change-to string
    usChangeToOffs = (USHORT)(pCurEntry->iChangeToBufferUsed );
    wcscpy( (PSZ_W)((PBYTE)(pCurEntry->pszChangeToBuffer) + pCurEntry->iChangeToBufferUsed), pIda->szChangeTo );
    pCurEntry->iChangeToBufferUsed += iChangeToLen;
  }

  // add found position to the record
  fOK = GFR_AddFindPosToResultList( pIda, &pCurEntry, fTarget, usOffs, usLen, Type, usChangeToOffs );

  // update entry pointer in result list (pointer may have been changed by the previous operations)
  pIda->ppResultList[pIda->iResultListUsed-1] = pCurEntry;

  return( TRUE );
}

// reset document search position
VOID  GFR_ResetSearchPosition
( 
  PFOLFINDDATA pIda
)
{
  pIda->ulSegNum = 1;
  pIda->usOffs   = 0;
  pIda->usLine   = 1;
  pIda->fCurrentSearchInSource = pIda->fSearchInSource;
  pIda->iCurBatchEntry = 0;
  if ( pIda->fApplyBatchList )
  {
    pIda->iCurBatchEntry = GFR_GetNextNonEmptyBatchListEntry( pIda, 0 );
    if ( pIda->iCurBatchEntry < pIda->iBatchListUsed )
    {
      PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[pIda->iCurBatchEntry];
      GFR_PrepareSearchForBatchEntry( pIda, pEntry );
    } /* endif */
  } /* endif */
}


// get next or previous segment from a document
PTBSEGMENT GetAdjacentSegment( PTBDOCUMENT pDoc, ULONG ulSegNum, BOOL fNext, int iNumOfSegs )
{
  PTBSEGMENT pSeg = NULL;

  if ( fNext )
  {
    // get following segment
    while( iNumOfSegs != 0 )
    {
      if ( ulSegNum >= pDoc->ulMaxSeg - 1 )
      {
        return( NULL );               // no more segments to follow
      } /* endif */

      ulSegNum++;
      pSeg = EQFBGetSegW( pDoc, ulSegNum );
      if ( (pSeg != NULL) && pSeg->SegFlags.Joined && !pSeg->SegFlags.JoinStart )
      {
        // ignore joined segments
      }
      else
      {
        iNumOfSegs--;
      }
    } /* endwhile */
  }
  else
  {
    // get previous segment
    while( iNumOfSegs != 0 )
    {
      if ( ulSegNum == 1 )
      {
        return( NULL );               // no more previous segments 
      } /* endif */

      ulSegNum--;
      pSeg = EQFBGetSegW( pDoc, ulSegNum );
      if ( (pSeg != NULL) && pSeg->SegFlags.Joined && !pSeg->SegFlags.JoinStart )
      {
        // ignore joined segments
      }
      else
      {
        iNumOfSegs--;
      }
    } /* endwhile */
  }
  return( pSeg );
}

USHORT GlobalFindStartThread
(
  PFOLFINDDATA pIda                     // dialog IDA
)
{
    _beginthread( GFR_SearchThread, 0, (void *)pIda );
    return( NO_ERROR );

} /* end of function GlobalFindStartThread */


void GFR_SearchThread
(
  PVOID   pvIda                     // dialog IDA
)
{
  PFOLFINDDATA pIda = (PFOLFINDDATA)pvIda;

  pIda->fThreadIsRunning = TRUE;
  pIda->ThreadTask = WAIT_TASK;

  while ( !pIda->fStopThread )
  {
    if ( pIda->fStopSearch )
    {
      // close any openend document
      if ( pIda->ThreadTask == FIND_NEXT_TASK )
      {
         FolFindCloseDoc( pIda, TRUE );
      }
      pIda->ThreadTask = WAIT_TASK;
      pIda->fStopSearch = FALSE;
      pIda->fSearching = FALSE;

      if ( pIda->fFind ) 
      {
        sprintf( pIda->szStatusLine, INFO_SEARCHSTOPPED, pIda->usHits);
        SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)pIda->szStatusLine );
      }
      else if ( pIda->fConfirm )
      {
        sprintf( pIda->szStatusLine, INFO_PREPARESTOPPED, pIda->usMayBeChanged, pIda->usHits);
        SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)pIda->szStatusLine );
      }
      else
      {
        sprintf( pIda->szStatusLine, INFO_REPLACESTOPPED, pIda->usChanges, pIda->usHits );
        SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)pIda->szStatusLine );
      } /* endif */

    }
    else
    {
      switch ( pIda->ThreadTask )
      {
        case WAIT_TASK:
          if ( pIda->fStartSearch )
          {
            pIda->fStartSearch = FALSE;
            pIda->fSearching = TRUE;
            pIda->ThreadTask = START_SEARCH_TASK;
          }
          else
          {
            Sleep( 50 ); // sleep 50 milli seconds
          }
          break;

        case START_SEARCH_TASK:
          pIda->usHits = 0;
          pIda->usChanges = 0;
          pIda->usMayBeChanged = 0;
          pIda->iCurrentDocumentListboxItem = -1;
          pIda->iCurDocInd = -1;
          pIda->ThreadTask = NEXT_DOC_TASK;
          GFR_ClearResultList( pIda );
          if ( pIda->fFind ) 
          {
            SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)INFO_SEARCHING );
          }
          else if ( pIda->fConfirm )
          {
            SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)INFO_PREPARING );
          }
          else
          {
            SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)INFO_REPLACING );
          } /* endif */
          break;

        case NEXT_DOC_TASK:
          {
            pIda->iCurrentDocumentListboxItem += 1;
            int iEntries = QUERYITEMCOUNT( pIda->hwndPages[0], ID_FOLFIND_DOCS_LB );
            if ( iEntries < pIda->iCurrentDocumentListboxItem )
            {
              // no more documents 
              pIda->ThreadTask = WAIT_TASK;
              pIda->fSearching = FALSE;
              if ( pIda->fFind ) 
              {
                sprintf( pIda->szStatusLine, INFO_SEARCHDONE, pIda->usHits);
                SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)pIda->szStatusLine );
              }
              else if ( pIda->fConfirm )
              {
                sprintf( pIda->szStatusLine, INFO_PREPAREDONE, pIda->usMayBeChanged,  pIda->usHits);
                SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)pIda->szStatusLine );
              }
              else
              {
                sprintf( pIda->szStatusLine, INFO_REPLACEDONE, pIda->usChanges, pIda->usHits );
                SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)pIda->szStatusLine );
              } /* endif */
            }
            else
            {
              int iIndex = QUERYITEMHANDLE( pIda->hwndPages[0], ID_FOLFIND_DOCS_LB, pIda->iCurrentDocumentListboxItem );
              if ( iIndex >= 0 )
              {
                pIda->iCurDocInd = iIndex;
                QUERYITEMTEXTHWND( pIda->hwndLB, iIndex, pIda->szCurDoc );
                SELECTITEM( pIda->hwndPages[0], ID_FOLFIND_DOCS_LB, pIda->iCurrentDocumentListboxItem );
                if ( pIda->fMultipleObjs )
                {
                  // get folder object name for this document
                  PSZ pszFolder = (PSZ)QUERYITEMHANDLEHWND( pIda->hwndLB, iIndex );
                  if ( pszFolder )
                  {
                    strcpy( pIda->szFolObjName, pszFolder );
                  } /* endif */
                } /* endif */
                pIda->ThreadTask = OPEN_DOC_TASK;
              }
              else
              {
                // skip folder speperator
                pIda->ThreadTask = NEXT_DOC_TASK;
              } /* endif */
            } /* endif */
          }
          break;

        case OPEN_DOC_TASK:
          // close any already loaded document
          if ( pIda->szCurDocObjName[0] != EOS ) 
          {
            FolFindCloseDoc( pIda, TRUE );
          } /* endif */

          // open the document
          if ( GFR_OpenDoc( pIda, TRUE, TRUE, TRUE ) )
          {
            // start seaching in loaded document
            pIda->ThreadTask = FIND_NEXT_TASK;
            GFR_ResetSearchPosition( pIda );
          }
          else
          {
            // skip this document
            pIda->ThreadTask = NEXT_DOC_TASK;
          }
          break;

        case FIND_NEXT_TASK:
          if ( FolFindMatch( pIda->pTargetDoc, pIda->pSourceDoc, pIda ) )
          {
            PSZ_W apszTexts[MAX_NUM_OF_TEXTS];

            // provide source and target text for current segment and the surrounding segments
            memset( apszTexts, 0, sizeof(apszTexts) );
            for( int i = 0; i < MAX_NUM_OF_TEXTS; i++ )
            {
              PTBSEGMENT pSeg = NULL;
              switch( i )
              {
                case SOURCE_TEXT:
                  pSeg = EQFBGetSegW( pIda->pSourceDoc, pIda->ulSegNum );
                  break;

                case TARGET_TEXT:
                  pSeg = EQFBGetSegW( pIda->pTargetDoc, pIda->ulSegNum );
                  break;

                case PREV_SEG_SOURCE_TEXT0:
                  pSeg = GetAdjacentSegment( pIda->pSourceDoc, pIda->ulSegNum, FALSE, 1 );
                  break;

                case PREV_SEG_SOURCE_TEXT1:
                  pSeg = GetAdjacentSegment( pIda->pSourceDoc, pIda->ulSegNum, FALSE, 2 );
                  break;

                case PREV_SEG_TARGET_TEXT0:
                  pSeg = GetAdjacentSegment( pIda->pTargetDoc, pIda->ulSegNum, FALSE, 1 );
                  break;

                case PREV_SEG_TARGET_TEXT1:
                  pSeg = GetAdjacentSegment( pIda->pTargetDoc, pIda->ulSegNum, FALSE, 2 );
                  break;

                case NEXT_SEG_SOURCE_TEXT0:
                  pSeg = GetAdjacentSegment( pIda->pSourceDoc, pIda->ulSegNum, TRUE, 1 );
                  break;

                case NEXT_SEG_SOURCE_TEXT1:
                  pSeg = GetAdjacentSegment( pIda->pSourceDoc, pIda->ulSegNum, TRUE, 2 );
                  break;

                case NEXT_SEG_TARGET_TEXT0:
                  pSeg = GetAdjacentSegment( pIda->pTargetDoc, pIda->ulSegNum, TRUE, 1 );
                  break;

                case NEXT_SEG_TARGET_TEXT1:
                  pSeg = GetAdjacentSegment( pIda->pTargetDoc, pIda->ulSegNum, TRUE, 2 );
                  break;
              }
              apszTexts[i] = ( pSeg == NULL ) ? NULL : pSeg->pDataW;
            }

            PTBSEGMENT pTargetSeg = EQFBGetSegW( pIda->pTargetDoc, pIda->ulSegNum );
            pIda->usHits++;
            if ( !pIda->fFind && pIda->fConfirm && !pIda->fCurrentSearchInSource && (pTargetSeg->qStatus == QF_XLATED) )
            {
              pIda->usMayBeChanged++;
            } /* endif */

            if ( !GFR_AddFindResultToList( pIda, pIda->iCurDocInd, pIda->szCurDocObjName, !pIda->fCurrentSearchInSource, pIda->ulSegNum, pTargetSeg->qStatus == QF_XLATED, 
                                       apszTexts, pIda->usFoundOffs, pIda->usFindLen ) )
            { 
              SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)ERROR_MEMORY_STRING );
              pIda->ThreadTask = STOP_TASK;
            }
            else if ( !pIda->fFind && !pIda->fConfirm && !pIda->fCurrentSearchInSource && (pTargetSeg->qStatus == QF_XLATED) )
            {
              // perform chang w/o user interaction
              pIda->ThreadTask = APPLY_CHANGE_TASK;
            }
            else
            {
              // continue searching
              pIda->ThreadTask = FIND_NEXT_TASK;
            } /* endif */
          }
          else
          {
            // no more hits in current document
            pIda->ThreadTask = CLOSE_DOC_TASK;
#ifdef _DEBUG
            if ( pIda->iResultListUsed != 0 )
            {
              CHAR szLogFile[MAX_LONGPATH];
              FILE *hfLogFile = NULL;

              UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
              strcat( szLogFile, "\\FolFindResultList.txt" );

              hfLogFile = fopen( szLogFile, "ab" );
              if ( hfLogFile != NULL )
              {
                fputs( UNICODEFILEPREFIX, hfLogFile );
                fwprintf( hfLogFile, L"======= results for search string %s =========\r\n", pIda->szFind );
                for( int i = 0; i < pIda->iResultListUsed; i++ )
                {
                  PFOLFINDRESULTENTRY pEntry = pIda->ppResultList[i];
                  fwprintf( hfLogFile, L"Entry=%ld, DocInd=%ld, SegNum=%lu, NumOfEntries=%ld\r\n", i, pEntry->iDocIndex, pEntry->ulSegNum, pEntry->iUsedEntries );
                  fwprintf( hfLogFile, L"Source=%s\r\n", (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[SOURCE_TEXT]) );
                  fwprintf( hfLogFile, L"Target=%s\r\n", (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[TARGET_TEXT]) );
                  PFOLFINDRESULTPOS pResultPos = (PFOLFINDRESULTPOS)((PBYTE)pEntry + pEntry->iResultPosOffs);
                  for( int j = 0; j < pEntry->iUsedEntries; j++ )
                  {
                    fwprintf( hfLogFile, L"   Result=%ld, usOffs=%u, usLen=%u, fTarget=%s\r\n", j, pResultPos->usOffs, pResultPos->usLen, pResultPos->fTarget ? L"TRUE" : L"FALSE" );
                    pResultPos++;
                  }
                } 
                fclose( hfLogFile );
              }
            }
#endif
          }
          break;

        case APPLY_CHANGE_TASK :
          // Change current hit
          {
            GFR_ReplaceTextInSegment( pIda, pIda->pTargetDoc, pIda->ulSegNum, pIda->usFoundOffs, pIda->usFindLen, pIda->szChangeTo, &(pIda->usOffs) );
          }

          // Continue with search
          pIda->ThreadTask = FIND_NEXT_TASK;
          break;

        case CLOSE_DOC_TASK :
          {
            // Close current doc (and save document if necessary)     
            BOOL fStopProcess = FolFindCloseDoc( pIda, TRUE );
            pIda->ThreadTask = fStopProcess ? STOP_TASK : NEXT_DOC_TASK;
          }
          break;

        case STOP_TASK :
          pIda->fSearching = FALSE;
          pIda->ThreadTask = WAIT_TASK;
          break;
      } /* endswitch */
    }
  } /* endwhile */

  // do any cleanup


  pIda->fThreadIsRunning = FALSE;

  //_endthread();
  return;
} /* end of function DocScanDirThread */


void WriteSegmentWithMarks( PFOLFINDDATA pIda, CXmlWriter *xw, PSZ_W pszText, PFOLFINDRESULTPOS pFoundPos, int iFoundEntries, BOOL fTarget )
{
  int iCurPos = 0; // current position within text

  // get text length
  int iRemaining = wcslen( pszText );

  // find next next active entry in found pos array 
  if ( pFoundPos != NULL )
  {
    while ( (iFoundEntries != 0) && (pFoundPos->fTarget != fTarget) )
    {
      pFoundPos++; iFoundEntries--;
    }
    if ( iFoundEntries == NULL ) pFoundPos = NULL;
  }
  
  while ( iRemaining )
  {
    // outut of current text
    if ( pFoundPos == NULL )
    {
      // output of remaining text
      xw->WriteString( pszText + iCurPos );
      iRemaining = 0;
    }
    else
    {
      // output text up to found position
      int iOutputLen = (int)pFoundPos->usOffs - iCurPos;
      if ( iOutputLen != 0 )
      {
        wcsncpy( pIda->szBuffer, pszText + iCurPos, iOutputLen );
        pIda->szBuffer[iOutputLen] = 0;
        xw->WriteString( pIda->szBuffer );
        iRemaining -= iOutputLen;
        iCurPos += iOutputLen;
      }

      // output of found string
      xw->WriteRaw( L"<hp1>" );
      wcsncpy( pIda->szBuffer, pszText + iCurPos, (int)pFoundPos->usLen );
      pIda->szBuffer[pFoundPos->usLen] = 0;
      xw->WriteString( pIda->szBuffer );
      xw->WriteRaw( L"</hp1>" );
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
        if ( (iFoundEntries == NULL) || (pFoundPos->usLen == 0) ) pFoundPos = NULL;
      }
    }
  }
}

// export TM using the standard save as dialog
int GFR_ExportResultList( HWND hwndParent, PFOLFINDDATA pIda )
{
  BOOL fOK = TRUE;
  BOOL fDone = TRUE;
  BOOL fExtAdded = FALSE;

  OPENFILENAME OpenFileName;

  memset( &OpenFileName, 0, sizeof(OpenFileName) );
  OpenFileName.lStructSize        = sizeof(OpenFileName);
  OpenFileName.hwndOwner          = hwndParent;
  OpenFileName.hInstance          = NULLHANDLE;
  OpenFileName.lpstrFilter        = RESULTLIST_EXPORT_FORMAT_FILTERS;
  OpenFileName.lpstrCustomFilter  = NULL;
  OpenFileName.nMaxCustFilter     = 0;
  OpenFileName.nFilterIndex       = ( pIda->pLastUsed->usResultListExpMode ) ? pIda->pLastUsed->usResultListExpMode : RESULTLIST_EXPORT_PLAINTEXT;

  strcpy( pIda->pLastUsed->szResultListName, ( pIda->pszObjList != NULL ) ? "ResultList" : pIda->szFolLongName );
  strcpy( pIda->szResultListFile, pIda->pLastUsed->szResultListName );
  OpenFileName.lpstrFile          = pIda->szResultListFile;
  OpenFileName.nMaxFile           = sizeof(pIda->szResultListFile);
  OpenFileName.lpstrFileTitle     = NULL;
  OpenFileName.nMaxFileTitle      = 0;
  if ( pIda->pLastUsed->szResultListDir[0] == EOS ) UtlMakeEQFPath( pIda->szNameBuffer, NULC, SYSTEM_PATH, NULL );
  OpenFileName.lpstrInitialDir    = pIda->pLastUsed->szResultListDir;
  OpenFileName.lpstrTitle         = "Export find results to";
  OpenFileName.Flags              = OFN_ENABLESIZING | OFN_EXPLORER | OFN_LONGNAMES | OFN_NODEREFERENCELINKS | OFN_NOTESTFILECREATE | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
  OpenFileName.nFileOffset        = 0;
  OpenFileName.nFileExtension     = 0;
  OpenFileName.lpstrDefExt        = NULL;   // no default extension, we have to set the extension depending of the format
  OpenFileName.lCustData          = 0L;
  OpenFileName.lpfnHook           = NULL;
  OpenFileName.lpTemplateName     = NULL;

  do
  {
    fOK = (GetSaveFileName( &OpenFileName ) != 0 );

    if ( fOK )
    {
      pIda->pLastUsed->usResultListExpMode = (USHORT)OpenFileName.nFilterIndex;
      fExtAdded = FALSE;

      // set export file extension if none specified by user
      {
        // find file name within full path
        PSZ pszFileName = strrchr( pIda->szResultListFile, '\\' );
        if ( pszFileName != NULL )
        {
          pszFileName++;               
        }
        else
        {
          pszFileName = pIda->szResultListFile;
        } /* endif */

        // set default extension if none specified
        if ( strchr( pszFileName, '.' ) == NULL )
        {
          // append file extension depending on export format
          switch ( pIda->pLastUsed->usResultListExpMode )
          {
            case RESULTLIST_EXPORT_PLAINTEXT:
              strcat( pIda->szResultListFile, ".TXT" );
                break;
            default:
              strcat( pIda->szResultListFile, ".XML" );
                break;
          } /*endswitch */
          fExtAdded = TRUE;
        } /* endif */
      }
    } /* endif */

    // get overwrite confirmation (has to be done outside the dialog as the file extension
    // is not available within the SaveAs dialog
    fDone = TRUE;
    if ( fOK && UtlFileExist(pIda->szResultListFile)  )
    {
      PSZ pszParms[2]; 

      pszParms[0] = pIda->szResultListFile;
      if ( UtlError( FILE_EXISTS, MB_OKCANCEL | MB_DEFBUTTON2, 1, pszParms, EQF_QUERY ) != MBID_OK )
      {
        PSZ pszName; 

        fDone = FALSE;                   // redo SaveAs dialog

        strcpy ( pIda->pLastUsed->szResultListDir, pIda->szResultListFile );
        pszName = UtlSplitFnameFromPath( pIda->pLastUsed->szResultListDir );
        if ( fExtAdded )
        {
          Utlstrccpy( pIda->pLastUsed->szResultListName, pszName, DOT );
        }
        else
        {
          strcpy( pIda->pLastUsed->szResultListName, pszName );
        } /* endif */
      } /* endif */
    } /* endif */
  } while ( !fDone );

  // write the xml version of the result list
  if ( fOK )
  {  
    if ( pIda->pLastUsed->usResultListExpMode == RESULTLIST_EXPORT_PLAINTEXT )
    {
      FILE *hfOut = fopen( pIda->szResultListFile, "wb" );
      if ( hfOut == NULL )
      {
        PSZ pszParm = pIda->szResultListFile;
        UtlErrorHwnd( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszParm, EQF_ERROR, hwndParent );
        return( 1 );
      } /* endif */

      // write BOM 
      fwrite( UNICODEFILEPREFIX, strlen(UNICODEFILEPREFIX), 1, hfOut );

      // write header info
      fwprintf( hfOut, L"Result List of Global Find&Replace\r\n", 0 );
      fwprintf( hfOut, L"==================================\r\n", 0 );
      if ( pIda->iResultListUsed != 0 )
      {
        for( int i = 0; i < pIda->iResultListUsed; i++ )
        {
          PFOLFINDRESULTENTRY pEntry = pIda->ppResultList[i];
          fwprintf( hfOut, L"Entry              = %ld\r\n", i + 1 );
          strcpy( pIda->szNameBuffer, pEntry->szDocObjName );
          UtlSplitFnameFromPath( pIda->szNameBuffer );
          SubFolObjectNameToName( pIda->szNameBuffer, pIda->szNameBuffer );
          fwprintf( hfOut, L"Folder             = %S\r\n", pIda->szNameBuffer );
          strcpy( pIda->szNameBuffer, UtlGetFnameFromPath( pEntry->szDocObjName ) ); // preset buffer with short name
          DocQueryInfo2( pEntry->szDocObjName, NULL, NULL, NULL, NULL, pIda->szNameBuffer, NULL, NULL, FALSE ); // try to get the long name
          fwprintf( hfOut, L"Document           = %S\r\n", pIda->szNameBuffer );
          fwprintf( hfOut, L"Segment Number     = %lu\r\n", pEntry->ulSegNum );
          fwprintf( hfOut, L"Source Text\r\n%s\r\n", (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[SOURCE_TEXT]) ); 
          fwprintf( hfOut, L"Target Text\r\n%s\r\n\r\n", (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[TARGET_TEXT]) ); 
        } 
      } /* endif */
      fclose( hfOut );
    }
    else
    {
      CXmlWriter *xw = new CXmlWriter( pIda->szResultListFile );
      xw->Formatting = CXmlWriter::Indented;
      xw->Encoding = CXmlWriter::UTF8;
      xw->Indention = 2;
      if ( xw->WriteStartDocument() )
      {
        xw->WriteStartElement( "FindResults" );
        // xw->WriteStylesheet( VALXMLTOHTML_TRANSONLY_STYLESHEET );
        xw->WriteStartElement( "Header" );
        xw->WriteElementString( L"Searched", pIda->szFind );
        xw->WriteEndElement(); // "Header"
        if ( pIda->iResultListUsed != 0 )
        {
          xw->WriteStartElement( "ResultList" );
          for( int i = 0; i < pIda->iResultListUsed; i++ )
          {
            PFOLFINDRESULTENTRY pEntry = pIda->ppResultList[i];
            xw->WriteStartElement( "Result" );

            xw->WriteStartAttribute( "Entry" );
            xw->WriteInt( i );
            xw->WriteEndAttribute(); // "Entry"

            strcpy( pIda->szNameBuffer, pEntry->szDocObjName );
            UtlSplitFnameFromPath( pIda->szNameBuffer );
            SubFolObjectNameToName( pIda->szNameBuffer, pIda->szNameBuffer );
            xw->WriteAttributeString( "Folder", pIda->szNameBuffer );

            strcpy( pIda->szNameBuffer, UtlGetFnameFromPath( pEntry->szDocObjName ) ); // preset buffer with short name
            DocQueryInfo2( pEntry->szDocObjName, NULL, NULL, NULL, NULL, pIda->szNameBuffer, NULL, NULL, FALSE ); // try to get the long name
            xw->WriteAttributeString( "Document", pIda->szNameBuffer );

            xw->WriteStartAttribute( "DocIndex" );
            xw->WriteInt( pEntry->iDocIndex + 1 );
            xw->WriteEndAttribute(); // "DocIndex"

            xw->WriteStartAttribute( "SegNum" );
            xw->WriteInt( pEntry->ulSegNum );
            xw->WriteEndAttribute(); // "SegNum"

            xw->WriteStartElement( "Source" );
            WriteSegmentWithMarks( pIda, xw, (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[SOURCE_TEXT]), (PFOLFINDRESULTPOS)((PBYTE)pEntry + pEntry->iResultPosOffs), pEntry->iUsedEntries, FALSE );
            xw->WriteEndElement(); // "Source" 

            xw->WriteStartElement( "Target" );
            WriteSegmentWithMarks( pIda, xw, (PSZ_W)((PBYTE)pEntry + pEntry->iTextOffs[TARGET_TEXT]), (PFOLFINDRESULTPOS)((PBYTE)pEntry + pEntry->iResultPosOffs), pEntry->iUsedEntries, TRUE );
            xw->WriteEndElement(); // "Target" 
            xw->WriteEndElement(); // "Result"
          } 
          xw->WriteEndElement(); // "ResultList"
        } /* endif */
        xw->WriteEndElement(); // "FindResults"
        xw->WriteEndDocument();
        xw->Close();
        delete xw;
      }
      else
      {
        PSZ pszParm = pIda->szResultListFile;
        UtlErrorHwnd( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszParm, EQF_ERROR, pIda->hwnd );
        fOK = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */

  return( fOK ? 0 : 1 );
} /* end of function GFR_ExportResultList */




BOOL GFR_GetLastUsedValues( PFOLFINDDATA pIda )
{
  PFOLFINDLASTUSED pLastUsed = NULL;
  BOOL fOK = TRUE;

  // build name of our file containing the last used values
  CHAR szLastUsedValues[MAX_LONGPATH];
  UtlMakeEQFPath( szLastUsedValues, NULC, PROPERTY_PATH, NULL );
  strcat( szLastUsedValues, "\\GlobFindLastUsedValues.DAT" );

  // allocate last values area
  fOK = UtlAlloc( (PVOID *)&pLastUsed, 0, sizeof(FOLFINDLASTUSED), ERROR_STORAGE );
  if ( !fOK ) return( FALSE );

  // load existing file, use values from folder list properties if load fails
  FILE *hfLastUsed = fopen( szLastUsedValues, "rb" );

  if ( hfLastUsed == NULL )
  {
    // get last used values from folder list property file
    EQFINFO     ErrorInfo;       // error code of property handler calls
    PPROPFOLDERLIST pFllProp = NULL;// ptr to folder list properties
    PVOID       hFllProp;        // handle of folder list properties
    OBJNAME     szFllObjName;    // buffer for folder list object name

    UtlMakeEQFPath( szFllObjName, NULC, SYSTEM_PATH, NULL );
    strcat( szFllObjName, BACKSLASH_STR );
    strcat( szFllObjName, DEFAULT_FOLDERLIST_NAME );
    hFllProp = OpenProperties( szFllObjName, NULL, PROP_ACCESS_READ, &ErrorInfo );
    if ( hFllProp ) pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFllProp );
    if ( pFllProp )
    {
      int i=0;

      pLastUsed->fFolFindCaseRespect = pFllProp->fFolFindCaseRespect;
      pLastUsed->fFolFindUpdateTM = pFllProp->fFolFindUpdateTM;
      pLastUsed->fFolFindTranslTextOnly = pFllProp->fFolFindTranslTextOnly;
      pLastUsed->fFolFindWholeWordsOnly = pFllProp->fFolFindWholeWordsOnly;
      pLastUsed->fFolFindConfirm = pFllProp->fFolFindConfirm;
      pLastUsed->chWildCardSingleChar = pFllProp->chWildCardSingleChar;
      pLastUsed->chWildCardMultChar = pFllProp->chWildCardMultChar;
      wcscpy( pLastUsed->szFolFind, pFllProp->szFolFind );
      wcscpy( pLastUsed->szFolChangeTo, pFllProp->szFolChangeTo );
      while (i < MAX_SEARCH_HIST && pFllProp->szFindList[i][0] != EOS)
      {
        wcscpy( pLastUsed->szFindList[i], pFllProp->szFindList[i] );
        i++;
      }// end while
      i=0;
      while (i < MAX_SEARCH_HIST && pFllProp->szReplaceList[i][0] != EOS)
      {
        wcscpy( pLastUsed->szReplaceList[i], pFllProp->szReplaceList[i] );
        i++;
        i++;
      }// end while
      memcpy( &(pLastUsed->swpFolFindSizePos), &(pFllProp->swpFolFindSizePos), sizeof(pLastUsed->swpFolFindSizePos) );
    }
  }
  else
  {
    // load last used values (don't care if loaded data is shorter than our last used values area)
    fread( pLastUsed, 1, sizeof(FOLFINDLASTUSED), hfLastUsed );
    fclose( hfLastUsed );
  }

  // anchor last used values in IDA
  pIda->pLastUsed = pLastUsed;

  return( TRUE );
}

BOOL GFR_SaveLastUsedValues( PFOLFINDDATA pIda )
{
  // build name of our file containing the last used values
  CHAR szLastUsedValues[MAX_LONGPATH];
  UtlMakeEQFPath( szLastUsedValues, NULC, PROPERTY_PATH, NULL );
  strcat( szLastUsedValues, "\\GlobFindLastUsedValues.DAT" );

  UtlWriteFile( szLastUsedValues, sizeof(FOLFINDLASTUSED), (PVOID)pIda->pLastUsed, FALSE );

  return( TRUE );
}


// prepare temporary segment data for searching
BOOL GFR_PrepareTempSeg( PFOLFINDDATA pIda, PSZ_W pszTempSeg, BOOL fProtected )
{
  // set to uppercase if case-insenstive search is requrested
  if ( pIda->fCaseIgnore )
  {
    UtlUpperW( pszTempSeg );
  } /* endif */

  // blank-out not translatable text if requested
  if ( pIda->fTranslTextOnly )
  {
    GFR_MaskProtectedText( pszTempSeg, (PLOADEDTABLE)pIda->pTargetDoc->pDocTagTable, pIda->pTargetDoc->ulOemCodePage, fProtected );
  } /* endif */

  // replace LF in temp segment by blank (except when followed by
  // DBCS characters)
  {
    PSZ_W pszTemp = pszTempSeg;
    while ( *pszTemp != EOS)
    {
      if ( *pszTemp == LF )
      {
        if ( !IsDBCS_CP(pIda->pTargetDoc->ulOemCodePage) )
        {
          *pszTemp = BLANK;
        }
        else
        {
          if ( ! EQFIsDBCSChar( pszTemp[1], pIda->pTargetDoc->ulOemCodePage) )
          {
            *pszTemp = BLANK;
          }
          else
          {
            // ignore LF - will be handled by GFR_NLFCmp/GFR_MatchPattern
          } /* endif */
        } /* endif */
      } /* endif */
      pszTemp++;
    } /* endwhile */
  }
  return( TRUE );
} /* end of GFR_PrepareTempSeg*/

// refresh the history in a search field combobox
BOOL GFR_RefreshComboboxHistory( HWND hwnd, int iID, PFOLFINDDATA pIda, PSZ_W pszFieldValue, PFINDNAME pHistoryList )
{
  pIda;

  if ( *pszFieldValue != 0 )
  {
    // Insert changed history into ID_FOLFIND_FIND_EF
    CBDELETEALL( hwnd, iID );
    CBINSERTITEMENDW( hwnd, iID, pszFieldValue );
    CBSELECTITEM (hwnd, iID, 0 );
  } /* endif */

  {
    int i = 0;

    while ( (i < MAX_SEARCH_HIST) && (pHistoryList[0] != 0) )
    {
      SHORT sItem = CBSEARCHITEMW( hwnd, iID, pHistoryList );
      if (sItem == LIT_NONE)
      {
        CBINSERTITEMENDW( hwnd, iID, pHistoryList );
      }
      i++;
      pHistoryList++;
    }// end while
  }// end if
  return( TRUE );
} /* end of GFR_RefreshComboboxHistory*/

// set and prepare the find and replace fields for the next search operation
BOOL GFR_SetSearchFields( PFOLFINDDATA pIda, HWND hwnd, PSZ_W pszFind, PSZ_W pszChangeTo, PSZ_W pszAndFindInSource, BOOL fErrorHandling )
{
  BOOL fOK = TRUE;

  if ( pszFind != (PSZ_W)&pIda->szFind ) wcscpy( pIda->szFind, pszFind );
  wcscpy( pIda->szFindUpr, pszFind );
  if ( pIda->fCaseIgnore ) UtlUpperW( pIda->szFindUpr );

  // pre-process specified find string and check if logical operators have been used
  GFR_PreProcessFindString( pIda->szFindUpr, &pIda->fLogicExpression, pIda->szExpression );

  // check search expression and do any preprocessing of search strings (uppercasing, simplify wildcards)
  if ( !GFR_CheckAndPrepareExpression( pIda->szExpression, pIda->fCaseIgnore ) )
  {
    if ( fErrorHandling )
    {
      // error in search expression, show message and do not start search
      UtlError( ERROR_INV_EXPRESSION, MB_CANCEL, 0, NULL, EQF_ERROR );
      SETFOCUS( hwnd, ID_FOLFIND_FIND_EF );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( pszChangeTo != (PSZ_W)&pIda->szChangeTo ) wcscpy( pIda->szChangeTo, pszChangeTo );

  if ( pIda->fAndFindInSource )
  {
    if ( pszAndFindInSource != (PSZ_W)&pIda->szAndFindInSource ) wcscpy( pIda->szAndFindInSource, pszAndFindInSource );
    wcscpy( pIda->szAndFindInSourceUpr, pIda->szAndFindInSource );
    if ( pIda->fCaseIgnore ) UtlUpperW( pIda->szAndFindInSourceUpr );
    GFR_PreProcessFindString( pIda->szAndFindInSourceUpr, &pIda->fAndFindInSourceLogicExpression, pIda->szAndFindInExpression );
    if ( !GFR_CheckAndPrepareExpression( pIda->szAndFindInExpression, pIda->fCaseIgnore ) )
    {
      if ( fOK && fErrorHandling )
      {
        // error in search expression, show message and do not start search
        UtlError( ERROR_INV_EXPRESSION, MB_CANCEL, 0, NULL, EQF_ERROR );
        SETFOCUS( hwnd, ID_FOLFIND_FINDSOURCE_EF );
        fOK = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */
  return( fOK );
} /* end of GFR_SetSearchFields */

// get the next non-empty entry from the batch list
int GFR_GetNextNonEmptyBatchListEntry( PFOLFINDDATA pIda, int iStartEntry )
{
  int iCurEntry = iStartEntry;
  while( (iCurEntry < pIda->iBatchListUsed) && (pIda->ppBatchList[iCurEntry]->iTargetFindLen <= 2) && (pIda->ppBatchList[iCurEntry]->iSourceFindLen <= 2) )
  {
    iCurEntry++;
  } /* endwhile */
  return( iCurEntry );
}

// set enable state of "apply batch list" checkbox
BOOL GFR_SetApplyBatchListCheckBoxState( PFOLFINDDATA pIda )
{
  BOOL fEnable = TRUE;
  if ( pIda->iBatchListUsed == 0 )
  {
    fEnable = FALSE;
  }
  else
  {
    int iValidEntry = GFR_GetNextNonEmptyBatchListEntry( pIda, 0 );
    fEnable = (iValidEntry < pIda->iBatchListUsed);
  } /* endif */

  ENABLECTRL( pIda->hwndPages[0], ID_FOLFIND_APPLYBATCH_CHK, fEnable );

  if ( !fEnable )
  {
    SETCHECK_FALSE( pIda->hwndPages[0], ID_FOLFIND_APPLYBATCH_CHK );
    GFR_EnableOrDisableEntryFields( pIda->hwndPages[0] );
  }

  return( fEnable );
}

// enable or disable the search entry fields depending on state of "apply batch list" checkbox
VOID GFR_EnableOrDisableEntryFields( HWND hwnd )
{
  BOOL fApplyBatch = QUERYCHECK( hwnd, ID_FOLFIND_APPLYBATCH_CHK );
  ENABLECTRL( hwnd, ID_FOLFIND_FIND_EF, !fApplyBatch );

  BOOL fFindInSourceOnly = QUERYCHECK( hwnd, ID_FOLFIND_SEARCHSOURCE_RB );
  ENABLECTRL( hwnd, ID_FOLFIND_CHANGE_EF, !fApplyBatch && !fFindInSourceOnly );

  ENABLECTRL( hwnd, ID_FOLFIND_FINDSOURCE_CHK, !fApplyBatch );

  BOOL fFindSource = QUERYCHECK( hwnd, ID_FOLFIND_FINDSOURCE_CHK );
  ENABLECTRL( hwnd, ID_FOLFIND_FINDSOURCE_EF, !fApplyBatch && fFindSource );
}

// set enabled state of the result list buttons and the the open button
VOID GFR_SetEnableStateResultListButtons( PFOLFINDDATA pIda )
{
  HWND hwnd = pIda->hwndPages[0];   // get window handle of the interactive find tab

  int iSelItem = QUERYSELECTIONHWND( pIda->hResultListBox );
  int iNumOfItems = QUERYITEMCOUNTHWND( pIda->hResultListBox );
  if ( iSelItem != -1 )
  {
    PFOLFINDRESULTENTRY pEntry = (PFOLFINDRESULTENTRY)QUERYITEMHANDLEHWND( pIda->hResultListBox, iSelItem );;
    if ( pEntry != NULL )
    {
      PFOLFINDRESULTPOS pResultPos = (PFOLFINDRESULTPOS)((PBYTE)pEntry + pEntry->iResultPosOffs);
      int i = 0;
      while( (i < pEntry->iUsedEntries) && (pResultPos->Type != CHANGETO_TYPE) )
      {
        pResultPos++; i++;
      } /* endwhile */
      ENABLECTRL( hwnd, ID_FOLFIND_APPLYCHANGE_PB, (pEntry->fTranslated && (i < pEntry->iUsedEntries) ) );
      ENABLECTRL( hwnd, ID_FOLFIND_APPLYNEXT_PB, (pEntry->fTranslated && (i < pEntry->iUsedEntries) ) );
      ENABLECTRL( pIda->hwnd, ID_FOLFIND_OPEN_PB, !pIda->fSearching );
    }
    else
    {
      ENABLECTRL( hwnd, ID_FOLFIND_APPLYCHANGE_PB, FALSE );
      ENABLECTRL( hwnd, ID_FOLFIND_APPLYNEXT_PB, FALSE );
      ENABLECTRL( pIda->hwnd, ID_FOLFIND_OPEN_PB, FALSE );
    } /* endif */

    ENABLECTRL( hwnd, ID_FOLFIND_PREV_PB, (iSelItem != 0) );
    ENABLECTRL( hwnd, ID_FOLFIND_NEXT_PB, (iSelItem + 1) < iNumOfItems );
  }
  else
  {
    // no item is selected
    ENABLECTRL( hwnd, ID_FOLFIND_APPLYCHANGE_PB, FALSE );
    ENABLECTRL( hwnd, ID_FOLFIND_APPLYNEXT_PB, FALSE );
    ENABLECTRL( hwnd, ID_FOLFIND_NEXT_PB, FALSE );
    ENABLECTRL( hwnd, ID_FOLFIND_PREV_PB, FALSE );
    ENABLECTRL( pIda->hwnd, ID_FOLFIND_OPEN_PB, FALSE );
  } /* endif */
  ENABLECTRL( hwnd, ID_FOLFIND_EXPORT_PB, (iNumOfItems != 0) && !pIda->fSearching );
}

// do a single change in the segment text
BOOL GFR_ReplaceTextInSegment( PFOLFINDDATA pIda, PTBDOCUMENT pDoc, ULONG ulSegNum, USHORT usFoundOffs, USHORT usFindLen, PSZ_W pszChangeTo, USHORT *pusOffs )
{
  PTBSEGMENT pSeg = EQFBGetSegW( pDoc, ulSegNum );
  ULONG ulDataLen   = UTF16strlenCHAR( pSeg->pDataW );
  ULONG ulChangeLen = UTF16strlenCHAR( pszChangeTo );
  ULONG ulNewLen = ulDataLen + ulChangeLen - pIda->usFindLen + 1;
  BOOL fOK = TRUE;

  if ( (ulNewLen) >= MAX_SEGMENT_SIZE )
  {
    UtlError( TB_TOOLONG, MB_CANCEL, 0, NULL, EQF_WARNING);
    fOK = FALSE;
  }
  else
  {
    // Allocate segment data buffer for changed segment and change segment data 
    PSZ_W pNewData;

    if ( UtlAlloc( (PVOID *)&pNewData, 0L, (ulNewLen) * sizeof(CHAR_W), ERROR_STORAGE ) )
    {
      memcpy( pNewData, pSeg->pDataW, usFoundOffs * sizeof(CHAR_W) );
      memcpy( pNewData + usFoundOffs, pszChangeTo, ulChangeLen * sizeof(CHAR_W) );
      pNewData[usFoundOffs] = GFR_AdjustCase( pSeg->pDataW[usFoundOffs], pszChangeTo[0], pIda->fPreserveCase );
      memcpy( pNewData + usFoundOffs + ulChangeLen, pSeg->pDataW + usFoundOffs + usFindLen, (ulDataLen - usFoundOffs - usFindLen + 1) * sizeof(CHAR_W));

      UtlAlloc( (PVOID *)&pSeg->pDataW, 0L, 0L, NOMSG );
      pSeg->pDataW = pNewData;
      pSeg->usLength = (USHORT)ulNewLen;
      pSeg->SegFlags.Changed = TRUE; // segment has been changed

      // re-compute target words
      {
        ULONG ulTgtWords = 0;
        ULONG ulTgtMarkUp = 0;

        EQFBWordCntPerSeg( (PLOADEDTABLE)pDoc->pDocTagTable, (PTOKENENTRY) pDoc->pTokBuf,
                            pSeg->pDataW, pIda->sTgtLangID, &ulTgtWords, &ulTgtMarkUp, pDoc->ulOemCodePage );

        pSeg->usTgtWords = (USHORT) ulTgtWords;
      }

      *pusOffs = (USHORT)(*pusOffs + ulChangeLen - usFindLen);
      pIda->usChanges++;
      pDoc->flags.changed = TRUE;
      if ( pIda->fUpdateTM && (pSeg->qStatus == QF_XLATED) )
      {
        pIda->fTMUpdRequired = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */
  return( fOK );
} /* end of GFR_ReplaceTextInSegment */

// Set the search in source/target entries for current batch list entry
void GFR_PrepareSearchForBatchEntry( PFOLFINDDATA pIda, PFOLFINDBATCHLISTENTRY pEntry )
{
  pIda->fSearchInSource = FALSE;
  pIda->fSearchInTarget = FALSE;
  pIda->fAndFindInSource = FALSE;

  if ( pEntry->iTargetFindLen > 2 )
  {
    pIda->fSearchInTarget = TRUE;
    pIda->fAndFindInSource = (pEntry->iSourceFindLen > 2);
  }
  else if ( pEntry->iSourceFindLen > 2 )
  {
    pIda->fSearchInSource = TRUE;
  } /* endif */

  // use source or target string as search string depending on search flag
  if ( pIda->fSearchInTarget )
  {
    GFR_SetSearchFields( pIda, pIda->hwndPages[0], (PSZ_W)((PBYTE)pEntry + pEntry->iTargetFindOffs), (PSZ_W)((PBYTE)pEntry + pEntry->iTargetChangeOffs), (PSZ_W)((PBYTE)pEntry + pEntry->iSourceFindOffs), FALSE );
  }
  else
  {
    GFR_SetSearchFields( pIda, pIda->hwndPages[0], (PSZ_W)((PBYTE)pEntry + pEntry->iSourceFindOffs), (PSZ_W)((PBYTE)pEntry + pEntry->iTargetChangeOffs), (PSZ_W)((PBYTE)pEntry + pEntry->iTargetFindOffs), FALSE );
  } /* endif */
  pIda->fCurrentSearchInSource = pIda->fSearchInSource;
} /* end of function GFR_PrepareSearchForBatchEntry */

// adjust the case of the first character of the replace string
CHAR_W GFR_AdjustCase( CHAR_W chOrgChar, CHAR_W chNewChar, BOOL fPreserveCase )
{
  CHAR_W chResult = chNewChar;

  if ( fPreserveCase )
  {
    CHAR_W szTemp[2];
    szTemp[0] = 0;
    szTemp[1] = 0;

    BOOL fOrgIsUpper = iswupper( chOrgChar );
    BOOL fNewIsUpper = iswupper( chNewChar );
    if ( fOrgIsUpper != fNewIsUpper )
    {
      if ( fOrgIsUpper )
      {
        szTemp[0] = chNewChar;
        CharUpperW( szTemp );
        chResult = szTemp[0];
      }
      else
      {
        szTemp[0] = chNewChar;
        CharLowerW( szTemp );
        chResult = szTemp[0];
      } /* endif */
    } /* endif */
  } /* endif */

  return( chResult );
} /* end of function GFR_PrepareSearchForBatchEntry */
