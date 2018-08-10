/*! \file
	Description: EQF Folder Handler Global Find and Replace (GFR) function
               Batch list maintenance tab related code

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


// global data for subclassing and listview subitem editing
HWND hEdit = NULL;        // handle of currently active edit contro
WNDPROC LVOldProc;        // list view control original windows procedire
WNDPROC EOldProc;         // edit control original windows procedire
int iItem;                // clicked item
int iSubItem;             // clicked subitem
HWND hwndBatchListTab;    // handle of batch list tab window
CHAR_W szEditText[100];     // buffer for edited text
CHAR_W szOrgClipBoardText[2400]; // buffer for original clipboard text

// Dialog procesure for batch list maintenance tab
INT_PTR CALLBACK GFR_BATCHLIST_DLGPROC
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

        // Anchor IDA 
        pIda = (PFOLFINDDATA)PVOIDFROMMP2(mp2);
        ANCHORDLGIDA( hwnd, pIda );

        // create batch list columns
        HWND hwndList = GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW );
        {
          LV_COLUMN LvCol;

          ListView_SetExtendedListViewStyle( hwndList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );


          memset( &LvCol, 0, sizeof(LvCol) );
          LvCol.mask= LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;    
          LvCol.pszText ="#";                        
          LvCol.cx = 10; // dummy width, actual widt is set during WM_SIZE processing
          SendMessage (hwndList, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol ); 

          LvCol.pszText ="Search string in TARGET segment";                        
          SendMessage (hwndList, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol ); 

          LvCol.pszText ="Search string in SOURCE segment";                        
          SendMessage (hwndList, LVM_INSERTCOLUMN, 2, (LPARAM)&LvCol ); 

          LvCol.pszText ="Replacement string in TARGET";                        
          SendMessage (hwndList, LVM_INSERTCOLUMN, 3, (LPARAM)&LvCol ); 
        }

        // subclass list view control and set global variables
        hEdit = NULL;        
        LVOldProc	= (WNDPROC)SetWindowLong( hwndList, GWL_WNDPROC, (LONG)GFR_ListViewProc );
        hwndBatchListTab = hwnd;

        // load last used batch list (when available)
        UtlMakeEQFPath( pIda->szNameBuffer, NULC, PROPERTY_PATH, NULL );
        strcat( pIda->szNameBuffer, "\\" );
        strcat( pIda->szNameBuffer, LASTUSEDBATCHLIST );
        if ( UtlFileExist( pIda->szNameBuffer ) )
        {
          GFR_ImportBatchList( pIda, hwnd, pIda->szNameBuffer );
          HWND hwndListView = GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW );

          // show last used batch list in list view control
          ListView_DeleteAllItems( hwndListView );
          for( int i = 0; i < pIda->iBatchListUsed; i++ )
          {
            GFR_AddToBatchListView( hwndListView, i, pIda->ppBatchList[i] );
          } /* endfor */
        } /* endif */

        GFR_SetApplyBatchListCheckBoxState( pIda );

        GFR_UpdateBatchListButtons( pIda, hwnd );

        return MRFROMSHORT(TRUE);
      }
      break;

    case WM_COMMAND:
      {
        PFOLFINDDATA pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
        switch ( WMCOMMANDID( mp1, mp2 ) )
        {
          case ID_FOLFIND_BATCHLIST_UP_PB: 
            {
              HWND hwndListView = GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW );
              int iSelItem = GFR_BatchListGetSelected( hwndListView );
              if ( (iSelItem != -1) && (iSelItem != 0) && (pIda->iBatchListUsed > 1 ) )
              {
                PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[iSelItem];
                PFOLFINDBATCHLISTENTRY pPrevEntry = pIda->ppBatchList[iSelItem-1];

                // switch entries
                pIda->ppBatchList[iSelItem] = pPrevEntry;
                pIda->ppBatchList[iSelItem-1] = pEntry;

                // update list view
                GFR_UpdateBatchListViewEntry( hwndListView, iSelItem - 1, pEntry  );
                GFR_UpdateBatchListViewEntry( hwndListView, iSelItem, pPrevEntry  );

                // change selection
                //SETFOCUSHWND( hwndListView );
                ListView_SetItemState( hwndListView, iSelItem, 0, LVIS_SELECTED );
                ListView_SetItemState( hwndListView, iSelItem - 1, LVIS_SELECTED, LVIS_SELECTED );
              } /* endif */
            }
            break;

          case ID_FOLFIND_BATCHLIST_EDIT_PB:
          case ID_FOLFIND_BATCHLIST_ADD_PB:
            {
              HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

              LV_ITEMW lvi;
              BOOL fAdd = WMCOMMANDID( mp1, mp2 ) == ID_FOLFIND_BATCHLIST_ADD_PB;
              HWND hwndListView = GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW );
              int iSelItem = GFR_BatchListGetSelected( hwndListView );
              if ( (iSelItem == -1) && !fAdd ) return( 0 );

              memset( &(pIda->BatchEntryData), 0, sizeof( pIda->BatchEntryData) );
              BOOL fOK = TRUE;
              pIda->BatchEntryData.fAddEntry = fAdd;
              if ( !fAdd )
              {
                // get entry data pointer
                memset( &lvi, 0, sizeof(lvi) );
                lvi.mask       = LVIF_PARAM;
                lvi.iItem      = iSelItem;
                lvi.iSubItem   = 0;
                SendMessageW( hwndListView, LVM_GETITEMW, 0, (LPARAM)(LV_ITEM FAR *) &lvi);

                // fill data area
                if ( lvi.lParam != 0 )
                {
                  PFOLFINDBATCHLISTENTRY pEntry = (PFOLFINDBATCHLISTENTRY)lvi.lParam;
                  wcscpy( pIda->BatchEntryData.szTargetFind, (PSZ_W)(((PBYTE)pEntry) + pEntry->iTargetFindOffs) );
                  wcscpy( pIda->BatchEntryData.szSourceFind, (PSZ_W)(((PBYTE)pEntry) + pEntry->iSourceFindOffs) );
                  wcscpy( pIda->BatchEntryData.szTargetChange, (PSZ_W)(((PBYTE)pEntry) + pEntry->iTargetChangeOffs) );
                } /* endif */
              } /* endif */

              DIALOGBOX( hwnd, GFR_BATCHLIST_ENTRY_DLGPROC, hResMod, ID_FOLFIND_BATCHLIST_ENTRY_DLG, &pIda->BatchEntryData, fOK );

              // process new or changed batch list entry
              if ( pIda->BatchEntryData.fOK )
              {
                PFOLFINDBATCHLISTENTRY pEntry = GFR_CreateBatchListEntry( &pIda->BatchEntryData );
                if ( pEntry != NULL )
                {
                  if ( fAdd )
                  {
                    // add a new entry to our list view
                    if ( iSelItem == -1 ) iSelItem = 0;
                    int iNewItem = GFR_AddToBatchListView( hwndListView, iSelItem, pEntry );
                    GFR_BatchListRenumber( hwnd, iNewItem );

                    // insert entry into our internal table
                    GFR_AddBatchListEntry( pIda, pEntry, iNewItem );

                    GFR_UpdateBatchListButtons( pIda, hwnd );
                    GFR_SetApplyBatchListCheckBoxState( pIda );
                  }
                  else
                  {
                    // update displayed data
                    GFR_UpdateBatchListViewEntry( hwndListView, iSelItem, pEntry );

                    // update entry pointer in our internal list
                    PFOLFINDBATCHLISTENTRY pOldEntry = pIda->ppBatchList[iSelItem];
                    UtlAlloc( (PVOID *)&pOldEntry, 0, 0, NOMSG );
                    pIda->ppBatchList[iSelItem] = pEntry;

                    GFR_SetApplyBatchListCheckBoxState( pIda );

                  } /* endif */
                } /* endif */
              } /* endif */
            }
            break;

          case ID_FOLFIND_BATCHLIST_DEL_PB:
            {
              HWND hwndListView = GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW );
              int iSelItem = GFR_BatchListGetSelected( hwndListView );
              if ( (iSelItem != -1) && (pIda->iBatchListUsed != 0 ) )
              {
                PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[iSelItem];

                // remove entry from our pointer array
                pIda->iBatchListUsed -= 1;
                UtlAlloc( (PVOID *)&pEntry, 0, 0, NOMSG );
                for( int i = iSelItem; i < pIda->iBatchListUsed; i++ )
                {
                  pIda->ppBatchList[i] = pIda->ppBatchList[i+1];
                } /* endfor */
                pIda->ppBatchList[pIda->iBatchListUsed] = NULL;

                // remove listview item
                ListView_DeleteItem( hwndListView, iSelItem );

                // renumber list view items
                GFR_BatchListRenumber( hwnd, iSelItem );

                // select next or previous item
                int iNewSelItem = iSelItem;
                if ( (iNewSelItem + 1) >= pIda->iBatchListUsed )
                {
                  iNewSelItem -= 1;
                } /* endif */
                if ( iNewSelItem != -1 ) ListView_SetItemState( hwndListView, iNewSelItem, LVIS_SELECTED, LVIS_SELECTED );

                GFR_UpdateBatchListButtons( pIda, hwnd );
                GFR_SetApplyBatchListCheckBoxState( pIda );

              } /* endif */
            }
            break;

          case ID_FOLFIND_BATCHLIST_DOWN_PB:
            {
              HWND hwndListView = GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW );
              int iSelItem = GFR_BatchListGetSelected( hwndListView );
              if ( (iSelItem != -1) && ((iSelItem + 1) != pIda->iBatchListUsed) && (pIda->iBatchListUsed > 1 ) )
              {
                PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[iSelItem];
                PFOLFINDBATCHLISTENTRY pNextEntry = pIda->ppBatchList[iSelItem+1];

                // switch entries
                pIda->ppBatchList[iSelItem] = pNextEntry;
                pIda->ppBatchList[iSelItem+1] = pEntry;

                // update list view
                GFR_UpdateBatchListViewEntry( hwndListView, iSelItem + 1, pEntry  );
                GFR_UpdateBatchListViewEntry( hwndListView, iSelItem, pNextEntry  );

                // change selection
                //SETFOCUSHWND( hwndListView );
                ListView_SetItemState( hwndListView, iSelItem, 0, LVIS_SELECTED );
                ListView_SetItemState( hwndListView, iSelItem + 1, LVIS_SELECTED, LVIS_SELECTED );
              } /* endif */
            }
            break;

          case ID_FOLFIND_BATCHLIST_IMPORT_PB:
            {
                OPENFILENAME OpenStruct;
                memset( &OpenStruct, 0, sizeof(OpenStruct) );
                OpenStruct.lStructSize = sizeof(OpenStruct);
                OpenStruct.hwndOwner = hwnd;
                OpenStruct.lpstrFile = pIda->pLastUsed->szBatchListName;
                OpenStruct.nMaxFile = sizeof(pIda->pLastUsed->szBatchListName)-1;
                OpenStruct.lpstrFilter        = RBATCHLIST_EXPORT_FORMAT_FILTERS;
                OpenStruct.nFilterIndex       = 0;
                OpenStruct.lpstrFileTitle = NULL;
                OpenStruct.nMaxFileTitle = 0;
                OpenStruct.lpstrTitle = "Import a Batch Find & Replace List";
                OpenStruct.lpstrDefExt = "CSV";
                OpenStruct.lpstrInitialDir = pIda->pLastUsed->szBatchListDir;
                OpenStruct.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
                if ( GetOpenFileName( &OpenStruct ) )
                {
                  if ( GFR_ImportBatchList( pIda, hwnd, pIda->pLastUsed->szBatchListName ) )
                  {
                    HWND hwndListView = GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW );

                    // show new batch list in list view control
                    ListView_DeleteAllItems( hwndListView );
                    for( int i = 0; i < pIda->iBatchListUsed; i++ )
                    {
                      GFR_AddToBatchListView( hwndListView, i, pIda->ppBatchList[i] );
                    } /* endfor */
                    SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)"Batch find & replace list has been imported" );

                  }
                  strcpy( pIda->pLastUsed->szBatchListDir, pIda->pLastUsed->szBatchListName );
                  strcpy( pIda->pLastUsed->szBatchListName , UtlSplitFnameFromPath( pIda->pLastUsed->szBatchListDir ) );
                  GFR_UpdateBatchListButtons( pIda, hwnd );
                  GFR_SetApplyBatchListCheckBoxState( pIda );

                } /* endif */

            }
            break;

          case ID_FOLFIND_BATCHLIST_EXPORT_PB:
            {
              OPENFILENAME OpenFileName;

              memset( &OpenFileName, 0, sizeof(OpenFileName) );
              OpenFileName.lStructSize        = sizeof(OpenFileName);
              OpenFileName.hwndOwner          = hwnd;
              OpenFileName.lpstrFile          = pIda->pLastUsed->szBatchListName;
              OpenFileName.nMaxFile           = sizeof(pIda->pLastUsed->szBatchListName);
              OpenFileName.lpstrFileTitle     = NULL;
              OpenFileName.nMaxFileTitle      = 0;  
              OpenFileName.lpstrFilter        = RBATCHLIST_EXPORT_FORMAT_FILTERS;
              OpenFileName.nFilterIndex       = 0;
              OpenFileName.lpstrInitialDir    = pIda->pLastUsed->szBatchListDir;
              OpenFileName.lpstrTitle         = "Export a Batch Find & Replace List";
              OpenFileName.Flags              = OFN_ENABLESIZING | OFN_EXPLORER | OFN_LONGNAMES | OFN_NODEREFERENCELINKS | OFN_NOTESTFILECREATE | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
              OpenFileName.nFileOffset        = 0;
              OpenFileName.nFileExtension     = 0;
              OpenFileName.lpstrDefExt        = "CSV";
              OpenFileName.lCustData          = 0L;
              OpenFileName.lpfnHook           = NULL;
              OpenFileName.lpTemplateName     = NULL;

              fOK = (GetSaveFileName( &OpenFileName ) != 0 );
              if ( fOK )
              {
                GFR_ExportBatchList( pIda, hwnd, pIda->pLastUsed->szBatchListName );
                SendMessage( pIda->hStatus, SB_SETTEXT, 2, (LPARAM)"Batch find & replace list has been exported" );

                strcpy( pIda->pLastUsed->szBatchListDir, pIda->pLastUsed->szBatchListName );
                strcpy( pIda->pLastUsed->szBatchListName , UtlSplitFnameFromPath( pIda->pLastUsed->szBatchListDir ) );
              } /* endif */
            }
            break;

        } /* endswitch */
      } 
      return 0;

    case WM_NOTIFY:
      {
          pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );
          LPNMHDR pnmhdr = (LPNMHDR)mp2;
          switch( pnmhdr->code )
          {
           case LVN_ENDLABELEDIT:
              {
			          LVITEMW LvItem;
			          LV_DISPINFOW *dispinfo = (LV_DISPINFOW*)mp2;
			          LvItem.iItem = dispinfo->item.iItem;
			          LvItem.iSubItem = dispinfo->item.iSubItem;
			          LvItem.pszText = dispinfo->item.pszText;
			          SendDlgItemMessage( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW, LVM_SETITEMTEXTW, (WPARAM)LvItem.iItem, (LPARAM)&LvItem); // put new text

                // update our internal batch list with the new data
                PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[dispinfo->item.iItem];
                memset( &pIda->BatchEntryData, 0, sizeof(pIda->BatchEntryData) );
                switch( dispinfo->item.iSubItem )
                {
                  case 1: 
                    wcscpy( pIda->BatchEntryData.szTargetFind, dispinfo->item.pszText );
                    wcscpy( pIda->BatchEntryData.szSourceFind, (PSZ_W)(((PBYTE)pEntry) + pEntry->iSourceFindOffs) );
                    wcscpy( pIda->BatchEntryData.szTargetChange, (PSZ_W)(((PBYTE)pEntry) + pEntry->iTargetChangeOffs) );
                    break;
                  case 2: 
                    wcscpy( pIda->BatchEntryData.szTargetFind, (PSZ_W)(((PBYTE)pEntry) + pEntry->iTargetFindOffs) );
                    wcscpy( pIda->BatchEntryData.szSourceFind, dispinfo->item.pszText );
                    wcscpy( pIda->BatchEntryData.szTargetChange, (PSZ_W)(((PBYTE)pEntry) + pEntry->iTargetChangeOffs) );
                    break;
                  case 3: 
                    wcscpy( pIda->BatchEntryData.szTargetFind, (PSZ_W)(((PBYTE)pEntry) + pEntry->iTargetFindOffs) );
                    wcscpy( pIda->BatchEntryData.szSourceFind, (PSZ_W)(((PBYTE)pEntry) + pEntry->iSourceFindOffs) );
                    wcscpy( pIda->BatchEntryData.szTargetChange, dispinfo->item.pszText );
                    break;
                } /* endswitch */

                PFOLFINDBATCHLISTENTRY pNewEntry = GFR_CreateBatchListEntry( &pIda->BatchEntryData );
                if ( pNewEntry )
                {
                  // update pointer array
                  pIda->ppBatchList[dispinfo->item.iItem] = pNewEntry;
                  UtlAlloc( (PVOID *)&pEntry, 0, 0, NOMSG );

                  // update lParam of item
                  LvItem.mask       = LVIF_PARAM;
                  LvItem.iItem      = iItem;
                  LvItem.iSubItem   = 0;
                  LvItem.lParam     = (LPARAM)pNewEntry;
                  SendDlgItemMessage( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW, LVM_SETITEMW, 0, (LPARAM)(LV_ITEM FAR *) &LvItem);
                  GFR_SetApplyBatchListCheckBoxState( pIda );
                } /* endif */

		          }
          		break;
	
            case LVN_ITEMCHANGED:
              {
                LPNMLISTVIEW pnmv = (LPNMLISTVIEW)mp2; 
                if ( pnmv->uChanged & LVIF_STATE ) 
                {
                  GFR_UpdateBatchListButtons( pIda , hwnd );
                } /* endif */
              }
              break;
          } /* endswitch */
      } /* endif */
      return 0;

    case WM_CLOSE:
      {
        PFOLFINDDATA pIda = ACCESSDLGIDA( hwnd, PFOLFINDDATA );

        // save current batch list as last used value
        UtlMakeEQFPath( pIda->szNameBuffer, NULC, PROPERTY_PATH, NULL );
        strcat( pIda->szNameBuffer, "\\" );
        strcat( pIda->szNameBuffer, LASTUSEDBATCHLIST );
        DeleteFile( pIda->szNameBuffer );
        GFR_ExportBatchList( pIda, hwnd, pIda->szNameBuffer );
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
        LONG    lBorderSize  = WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER);
        LONG    cxAvail = sWidth - (2 * lBorderSize);
        RECT    rect;

        // we re-position/re-size all dialog controls ...
        HDWP hdwp = BeginDeferWindowPos( 11 );

        // get height of button below the tree view control
        GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_IMPORT_PB ), &rect );
        MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
        LONG lImportButtonHeight = rect.bottom - rect.top;

        // get width of button right from tree view control
        GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_EDIT_PB ), &rect );
        MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
        LONG lEditButtonWidth = rect.right - rect.left;
        LONG lEditButtonHeigth = rect.bottom - rect.top;

        // compute new rectangle of tree view control
        RECT rcTreeView;
        GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW), &rcTreeView );
        MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rcTreeView, 2 );
        rcTreeView.bottom = sHeight - (2 * lBorderSize) - rcTreeView.top - lImportButtonHeight - 6; 
        rcTreeView.right = cxAvail - rcTreeView.left - lEditButtonWidth - 20;

        // repositiom import and export button
        GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_IMPORT_PB ), &rect );
        MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
        hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_IMPORT_PB ),
                               HWND_TOP, ((rcTreeView.right - rcTreeView.left ) / 2) + rcTreeView.left - 10 - (rect.right - rect.left), rcTreeView.bottom + 4, 
                               0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );

        GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_EXPORT_PB ), &rect );
        MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
        hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_EXPORT_PB ),
                               HWND_TOP, ((rcTreeView.right - rcTreeView.left ) / 2) + rcTreeView.left + 10, rcTreeView.bottom + 5, 
                               0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );

        // resize batch list tree view control
        hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW ),
                              HWND_TOP, 0, 0,
                              rcTreeView.right - rcTreeView.left,
                              rcTreeView.bottom - rcTreeView.top,
                              SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

        // reposition batch list edit buttons
        {
          LONG lYPos = ((rcTreeView.bottom - rcTreeView.top) / 2) + rcTreeView.top - (lEditButtonHeigth  / 2) - 2 * (lEditButtonHeigth + 10);
          LONG lXPos = rcTreeView.right + 10; 
          GetWindowRect( GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_UP_PB), &rect );
          MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_UP_PB ),
                                  HWND_TOP, lXPos, lYPos, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
          lYPos += lEditButtonHeigth + 10;
          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_ADD_PB ),
                                  HWND_TOP, lXPos, lYPos, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
          lYPos += lEditButtonHeigth + 10;
          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_EDIT_PB ),
                                  HWND_TOP, lXPos, lYPos, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
          lYPos += lEditButtonHeigth + 10;
          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_DEL_PB ),
                                  HWND_TOP, lXPos, lYPos, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
          lYPos += lEditButtonHeigth + 10;
          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_DOWN_PB ),
                                  HWND_TOP, lXPos, lYPos, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
        }
        // do actual dialog control re-positioning
        if ( hdwp != NULL )
        {
          EndDeferWindowPos( hdwp );
        } /* endif */

        // adjust batch list column widths
        ResizeBatchListColumns( hwnd, pIda );
     } /* endif */
     break;
  } /* endswitch */
  return WinDefDlgProc( hwnd, msg, mp1, mp2 );
} /* end GFR_BATCHLIST_DLGPROC */


BOOL ResizeBatchListColumns
( 
  HWND        hwnd,                    // dialog window handle
  PFOLFINDDATA pIda                     // dialog IDA
)
{
  HWND hwndList = GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW );
  LV_COLUMN LvCol;
  RECT rect;

  GetWindowRect( hwndList, &rect );
  int iAvail = rect.right - rect.left;
  int iAveCharWidth = 0;

  // get average character width
  {
    TEXTMETRIC tm;
    HDC hdc = GetDC( hwndList );
    GetTextMetrics( hdc, &tm );
    //ReleaseDC( hwndList, hdc );
    iAveCharWidth = tm.tmAveCharWidth;
  }

  // compute new width of columns
  pIda->aiBatchListColWidth[0] = iAveCharWidth * 5;
  iAvail -= pIda->aiBatchListColWidth[0];
  pIda->aiBatchListColWidth[1] = iAvail / 3;
  iAvail -= pIda->aiBatchListColWidth[1];
  pIda->aiBatchListColWidth[2] = pIda->aiBatchListColWidth[1];
  iAvail -= pIda->aiBatchListColWidth[2];
  pIda->aiBatchListColWidth[3] = iAvail;

  // set column widths
  for( int i = 0; i < 3; i++ )
  {
    memset( &LvCol, 0, sizeof(LvCol) );
    LvCol.mask = LVCF_WIDTH | LVCF_SUBITEM;    
    LvCol.cx = pIda->aiBatchListColWidth[i];               
    SendMessage( hwndList, LVM_SETCOLUMN, i, (LPARAM)&LvCol ); 
  } /* endfor */

  // use remaining space for the last column
  ListView_SetColumnWidth( hwndList, 3, LVSCW_AUTOSIZE_USEHEADER );

  return( TRUE );
}



// Dialog procesure for batch list entry dialog
INT_PTR CALLBACK GFR_BATCHLIST_ENTRY_DLGPROC
(
   HWND hwnd,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  PBATCHLISTENTRYDATA pData = NULL;    // ptr to batch list entry data

  switch (msg)
  {

    case WM_INITDLG:
      {
        // Anchor data area
        pData = (PBATCHLISTENTRYDATA)PVOIDFROMMP2(mp2);
        ANCHORDLGIDA( hwnd, pData );

        HWND hwndInsertBehind = HWND_TOP;

        // replace resource file created controls with Unicode enabled counterpart
        hwndInsertBehind = GFR_ReplaceWithUnicodeEditControl( hwnd, ID_FOLFIND_BATCHLIST_TARGETFIND_EF, hwndInsertBehind );
        hwndInsertBehind = GFR_ReplaceWithUnicodeEditControl( hwnd, ID_FOLFIND_BATCHLIST_SOURCEFIND_EF, hwndInsertBehind );
        hwndInsertBehind = GFR_ReplaceWithUnicodeEditControl( hwnd, ID_FOLFIND_BATCHLIST_TARGETCHANGE_EF, hwndInsertBehind );

        // set entry field text limits
        SETTEXTLIMIT( hwnd, ID_FOLFIND_BATCHLIST_TARGETFIND_EF, MAX_SEGMENT_SIZE );
        SETTEXTLIMIT( hwnd, ID_FOLFIND_BATCHLIST_SOURCEFIND_EF, MAX_SEGMENT_SIZE );
        SETTEXTLIMIT( hwnd, ID_FOLFIND_BATCHLIST_TARGETCHANGE_EF, MAX_SEGMENT_SIZE );

        // fill entry fields if this is no new entry
        if ( !pData->fAddEntry )
        {
          SETTEXTW( hwnd, ID_FOLFIND_BATCHLIST_TARGETFIND_EF, pData->szTargetFind);
          SETTEXTW( hwnd, ID_FOLFIND_BATCHLIST_SOURCEFIND_EF, pData->szSourceFind );
          SETTEXTW( hwnd, ID_FOLFIND_BATCHLIST_TARGETCHANGE_EF, pData->szTargetChange );
        }

        // change title and OK button text for existing entries
        // fill entry fields if this is no new entry
        if ( !pData->fAddEntry )
        {
          SETTEXT( hwnd, ID_FOLFIND_BATCHLIST_OK_PB, "Change");
          SETTEXTHWND( hwnd, "Change Batch Find & Replace List Entry");
        }

        pData->fOK = FALSE;

        return MRFROMSHORT(TRUE);
      }
      break;

    case WM_COMMAND:
      {
        PBATCHLISTENTRYDATA pData = ACCESSDLGIDA( hwnd, PBATCHLISTENTRYDATA );
        switch ( WMCOMMANDID( mp1, mp2 ) )
        {
          case ID_FOLFIND_BATCHLIST_OK_PB: 
            QUERYTEXTW( hwnd, ID_FOLFIND_BATCHLIST_TARGETFIND_EF, pData->szTargetFind );
            QUERYTEXTW( hwnd, ID_FOLFIND_BATCHLIST_SOURCEFIND_EF, pData->szSourceFind );
            QUERYTEXTW( hwnd, ID_FOLFIND_BATCHLIST_TARGETCHANGE_EF, pData->szTargetChange );
            pData->fOK = TRUE;
            POSTCLOSE( hwnd, TRUE );
            break;
          case ID_FOLFIND_BATCHLIST_CANCEL_PB:
            pData->fOK = FALSE;
            POSTCLOSE( hwnd, FALSE );
            break;
        } /* endswitch */
      } 
      return 0;

    case WM_CLOSE:
      DISMISSDLG( hwnd, SHORT1FROMMP2(mp2) );
      return 0;
      break;
  } /* endswitch */
  return WinDefDlgProc( hwnd, msg, mp1, mp2 );
} /* end GFR_BATCHLIST_ENTRY_DLGPROC */

// renumber items in the batch list starting at the given item
void GFR_BatchListRenumber( HWND hwnd, int iItem )
{
  CHAR szNumber[10];
  HWND hwndList = GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW );
  int iNumOfItems = ListView_GetItemCount( hwndList );
  while ( iItem < iNumOfItems  )
  {
    ltoa( iItem + 1, szNumber, 10 );
    ListView_SetItemText( hwndList, iItem, 0, szNumber );
    iItem++;
  } /* endwhile */     
}

// create new batch list entry with data from batch list entry data structure
PFOLFINDBATCHLISTENTRY GFR_CreateBatchListEntry( PBATCHLISTENTRYDATA pData )
{
  PFOLFINDBATCHLISTENTRY pNewEntry = NULL;

  // compute size of newor updated entry
  int iTargetFindLen = (wcslen( pData->szTargetFind ) + 1 ) * sizeof(CHAR_W); 
  int iSourceFindLen = (wcslen( pData->szSourceFind ) + 1 ) * sizeof(CHAR_W); 
  int iTargetChangeLen = (wcslen( pData->szTargetChange ) + 1 ) * sizeof(CHAR_W); 
  int iNewLen = sizeof(FOLFINDBATCHLISTENTRY) + iTargetFindLen + iSourceFindLen + iTargetChangeLen;

  // allocate new entry
  if ( !UtlAlloc( (PVOID *)&pNewEntry, 0, iNewLen, ERROR_STORAGE ) ) return( NULL );

  // fill new entry
  pNewEntry->iSize = iNewLen;
  pNewEntry->iTargetFindLen = iTargetFindLen;
  pNewEntry->iTargetFindOffs = sizeof(FOLFINDBATCHLISTENTRY);
  wcscpy( (PSZ_W)(((PBYTE)pNewEntry) + pNewEntry->iTargetFindOffs), pData->szTargetFind );
  pNewEntry->iSourceFindLen = iSourceFindLen;
  pNewEntry->iSourceFindOffs = pNewEntry->iTargetFindOffs + pNewEntry->iTargetFindLen;
  wcscpy( (PSZ_W)(((PBYTE)pNewEntry) + pNewEntry->iSourceFindOffs), pData->szSourceFind );
  pNewEntry->iTargetChangeLen = iTargetChangeLen;
  pNewEntry->iTargetChangeOffs = pNewEntry->iSourceFindOffs + pNewEntry->iSourceFindLen;
  wcscpy( (PSZ_W)(((PBYTE)pNewEntry) + pNewEntry->iTargetChangeOffs), pData->szTargetChange );

  return( pNewEntry );
}

// add a batch list entry to the batch list table
BOOL GFR_AddBatchListEntry( PFOLFINDDATA pIda, PFOLFINDBATCHLISTENTRY pEntry, int iPos )
{
  // enlarge pointer array of batch list when necessary
  if ( (pIda->iBatchListUsed + 1) > pIda->iBatchListSize )
  {
    int iNewSize = pIda->iBatchListSize + 50;
    int iOldLen = pIda->iBatchListSize * sizeof(PFOLFINDBATCHLISTENTRY);
    int iNewLen = iNewSize * sizeof(PFOLFINDBATCHLISTENTRY);
    if ( !UtlAlloc( (PVOID *)&(pIda->ppBatchList), iOldLen, iNewLen, ERROR_STORAGE ) ) return( FALSE );
    pIda->iBatchListSize = iNewSize;
  }

  // verify given position
  if ( (iPos < 0) || (iPos > pIda->iBatchListUsed) )iPos = pIda->iBatchListUsed;

  // make room at given position
  for( int i = (pIda->iBatchListUsed - 1); i >= iPos; i-- )
  {
    pIda->ppBatchList[i+1] = pIda->ppBatchList[i];
  }

  // add pointer of new entry at given position
  pIda->ppBatchList[iPos] = pEntry;

  pIda->iBatchListUsed += 1;

  return( TRUE );
}

// clear the batch list table
BOOL GFR_ClearBatchList( PFOLFINDDATA pIda )
{
  for( int i = 0; i < pIda->iBatchListUsed; i++ )
  {
    PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[i]; 
    UtlAlloc( (PVOID *)&pEntry, 0, 0, NOMSG );
    pIda->ppBatchList[i] = NULL;
  } /* endfor */
  pIda->iBatchListUsed = 0;
  return( TRUE );
}

// get the (first) selected item of a listview control
int GFR_BatchListGetSelected( HWND hwndListView )
{
  return( ListView_GetNextItem( hwndListView, -1, LVNI_SELECTED ) );
}

// add a batch list entry to the list view
int GFR_AddToBatchListView( HWND hwndListView, int iPos, PFOLFINDBATCHLISTENTRY pEntry )
{
  LVITEMW lvi;

  CHAR_W szNumber[10];

  _ltow(iPos + 1, szNumber, 10 );

  memset( &lvi, 0, sizeof(lvi) );
  lvi.mask       = LVIF_TEXT | LVIF_PARAM;
  lvi.iItem      = iPos;
  lvi.iSubItem   = 0;
  lvi.pszText    = szNumber;
  lvi.cchTextMax = wcslen(szNumber);
  lvi.lParam     = (LPARAM)pEntry;

  int iItem = SendMessageW( hwndListView, LVM_INSERTITEMW, 0, (LPARAM)(LV_ITEM FAR *) &lvi);

  GFR_UpdateBatchListViewEntry( hwndListView, iItem, pEntry );

  return( iItem );
}

// update an entry ion the batch list view
int GFR_UpdateBatchListViewEntry( HWND hwndListView, int iItem, PFOLFINDBATCHLISTENTRY pEntry )
{
  LVITEMW lvi;

  CHAR_W szNumber[10];

  _ltow(iItem + 1, szNumber, 10 );

  lvi.mask       = LVIF_TEXT | LVIF_PARAM;
  lvi.iItem      = iItem;
  lvi.iSubItem   = 0;
  lvi.pszText    = szNumber;
  lvi.cchTextMax = wcslen(szNumber);
  lvi.lParam     = (LPARAM)pEntry;
  SendMessageW( hwndListView, LVM_SETITEMW, 0, (LPARAM)(LV_ITEM FAR *) &lvi);

  lvi.mask       = LVIF_TEXT;
  lvi.iSubItem   = 1;
  lvi.pszText    = (PSZ_W)(((PBYTE)pEntry) + pEntry->iTargetFindOffs); 
  lvi.cchTextMax = wcslen(lvi.pszText);
  SendMessageW( hwndListView, LVM_SETITEMW, 0, (LPARAM)(LV_ITEM FAR *) &lvi);

  lvi.iSubItem   = 2;
  lvi.pszText    = (PSZ_W)(((PBYTE)pEntry) + pEntry->iSourceFindOffs);
  lvi.cchTextMax = wcslen(lvi.pszText);
  SendMessageW( hwndListView, LVM_SETITEMW, 0, (LPARAM)(LV_ITEM FAR *) &lvi);

  lvi.iSubItem   = 3;
  lvi.pszText    = (PSZ_W)(((PBYTE)pEntry) + pEntry->iTargetChangeOffs);
  lvi.cchTextMax = wcslen(lvi.pszText);
  SendMessageW( hwndListView, LVM_SETITEMW, 0, (LPARAM)(LV_ITEM FAR *) &lvi);

  return( iItem );
}

// update enable state of batch list buttons
void GFR_UpdateBatchListButtons( PFOLFINDDATA pIda , HWND hwnd )
{
  int iSelItem = GFR_BatchListGetSelected( GetDlgItem( hwnd, ID_FOLFIND_BATCHLIST_LISTVIEW ) );

  if ( pIda->iBatchListUsed == 0 )
  {
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_UP_PB, FALSE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_ADD_PB, TRUE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_EDIT_PB, FALSE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_DEL_PB, FALSE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_DOWN_PB, FALSE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_EXPORT_PB, FALSE );
  }
  else if ( iSelItem == -1 )
  {
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_UP_PB, FALSE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_ADD_PB, TRUE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_EDIT_PB, FALSE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_DEL_PB, FALSE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_DOWN_PB, FALSE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_EXPORT_PB, TRUE );
  }
  else 
  {
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_ADD_PB, TRUE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_EDIT_PB, TRUE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_DEL_PB, TRUE );
    ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_EXPORT_PB, TRUE );
    if ( (iSelItem == 0) || (pIda->iBatchListUsed == 1) )
    {
      ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_UP_PB, FALSE );
    }
    else
    {
      ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_UP_PB, TRUE );
    } /* endif */

    if ( (iSelItem + 1) < pIda->iBatchListUsed )
    {
      ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_DOWN_PB, TRUE );
    }
    else
    {
      ENABLECTRL( hwnd,  ID_FOLFIND_BATCHLIST_DOWN_PB, FALSE );
    } /* endif */
  }
}


// process a single line from an imported batch list
BOOL ProcessImportedBatchListLine( PFOLFINDDATA pIda, PSZ_W pszLine, PBATCHLISTENTRYDATA pData )
{
  memset( pData, 0, sizeof(BATCHLISTENTRYDATA) );
  int iColumn = 0;

  while( (iColumn < 3 ) && (*pszLine != 0) )
  {
    while( iswspace( *pszLine ) ) pszLine++; // skip any whitespace

    PSZ_W pszStart = pszLine;

    if ( *pszLine == L'\"' )
    {
      // process text enclosed in quotes
      pszLine++; 
      PSZ_W pszOutPos = pszStart;
      BOOL fEndOfString = FALSE;
      do
      {
        if ( (*pszLine == 0) || (*pszLine == L'\n') || (*pszLine == L'\r') )
        {
          fEndOfString = TRUE;
        }
        else if ( *pszLine == L'\"' )
        {
          if ( *(pszLine+1) == L'\"'  )
          {
            // doubled double-quote detected
            *pszOutPos++ = *pszLine++;
            pszLine++;
          }
          else
          {
            // double quote marks the end of the string
            fEndOfString = TRUE;
          } /* endif */
        }
        else
        {
          // normal character within the string
            *pszOutPos++ = *pszLine++;
        } /* endif */
      } while( !fEndOfString );
      *pszOutPos = 0;

      if ( *pszLine == L'\"' ) 
      {
        pszLine++;
        while( iswspace( *pszLine ) ) pszLine++; // skip any whitespace
        if ( *pszLine == L',' ) pszLine++;
      }
    }
    else
    {
      // find next comma
      while( (*pszLine != 0) && (*pszLine != L',') && (*pszLine != L'\n') && (*pszLine != L'\r') ) pszLine++;
      if ( *pszLine != 0 ) 
      {
        *pszLine = 0;
        pszLine++;
      } /* endif */
    } /* endif */

    // handle any data found
    switch( iColumn )
    {
      case 0: wcscpy( pData->szTargetFind, pszStart ); break;
      case 1: wcscpy( pData->szSourceFind, pszStart ); break;
      case 2: wcscpy( pData->szTargetChange, pszStart ); break;
    } /* endswitch */
    iColumn++;
  } /* endwhile */

  // add entry to batch list if ok
  if ( (pData->szTargetFind[0] != 0) || (pData->szSourceFind[0] != 0) )
  {
    PFOLFINDBATCHLISTENTRY pNewEntry = GFR_CreateBatchListEntry( pData );
    if ( pNewEntry )
    {
      GFR_AddBatchListEntry( pIda, pNewEntry, pIda->iBatchListUsed );
    } /* endif */
  } /* endif */
  return( TRUE );
} /* end of ProcessImportedBatchListLine */

// import a batch list
BOOL GFR_ImportBatchList( PFOLFINDDATA pIda, HWND hwnd, PSZ pszListFile )
{

  FILE *hfList = fopen( pszListFile, "rb" );
  if ( hfList == NULL )
  {
    UtlErrorHwnd( FILE_NOT_EXISTS, MB_CANCEL, 1, &pszListFile, EQF_ERROR, hwnd );
    return( FALSE );
  } /* endif */

  // clear existing batch list
  GFR_ClearBatchList( pIda );

  // read first two bytes and check for UTF16 BOM
  fread( pIda->szNameBuffer, 2, 1, hfList );

  if ( memcmp( pIda->szNameBuffer, UNICODEFILEPREFIX, 2 ) == 0 )
  {
    // import unicode list
    while( !feof( hfList ) )
    {
      pIda->szBuffer[0] = 0;
      fgetws( pIda->szBuffer, sizeof(pIda->szBuffer)/ sizeof(CHAR_W), hfList );
      ProcessImportedBatchListLine( pIda, pIda->szBuffer, &pIda->BatchEntryData );
    } /* endwhile */
  }
  else
  {
    // import ANSI list
    fclose( hfList );
    hfList = fopen( pszListFile, "r" );
    if ( hfList == NULL )
    {
      UtlErrorHwnd( FILE_NOT_EXISTS, MB_CANCEL, 1, &pszListFile, EQF_ERROR, hwnd );
      return( FALSE );
    } /* endif */

    while( !feof( hfList ) )
    {
      pIda->szNameBuffer[0] = EOS;
      fgets( pIda->szNameBuffer, sizeof(pIda->szNameBuffer), hfList );
      int iChars = MultiByteToWideChar( CP_ACP, 0, pIda->szNameBuffer, strlen(pIda->szNameBuffer), pIda->szBuffer, sizeof(pIda->szBuffer) / sizeof(CHAR_W) );
      pIda->szBuffer[iChars] = 0;
      ProcessImportedBatchListLine( pIda, pIda->szBuffer, &pIda->BatchEntryData );
    } /* endwhile */
  }
  fclose( hfList );

  return( TRUE );
}

// write a string to the output file, enclose string in quotes if it contains a comma
BOOL WriteStringToExportedBatchList( FILE *hfList, PSZ_W pszString )
{
  PSZ_W pszComma = wcschr( pszString, L',' );
  PSZ_W pszQuote = wcschr( pszString, L'\"' );

  // strip any linefeed from string
  int iLen = wcslen(pszString);
  if ( iLen && pszString[iLen-1] == L'\n' ) iLen--;
  if ( iLen && pszString[iLen-1] == L'\r' ) iLen--;

  if ( (pszComma != NULL ) || (pszQuote != NULL) )
  {
    // enclose string in double-quotes and double any double-quote contained in the string
    fwrite( L"\"", sizeof(CHAR_W), 1, hfList );
    PSZ_W pszPos = pszString;
    while ( *pszPos != 0 )
    {
      if ( *pszPos == L'\"' )
      {
        fwrite( L"\"", sizeof(CHAR_W), 1, hfList );
      } /* endif */
      fwrite( pszPos++, sizeof(CHAR_W), 1, hfList );
    } /* endwhile */
    fwrite( L"\"", sizeof(CHAR_W), 1, hfList );
  }
  else
  {
    // write string as-is
    fwrite( pszString, sizeof(CHAR_W), iLen, hfList );
  } /* endif */

  return( TRUE );
}

// export a batch list
BOOL GFR_ExportBatchList( PFOLFINDDATA pIda, HWND hwnd, PSZ pszListFile )
{
  FILE *hfList = fopen( pszListFile, "wb" );
  if ( hfList == NULL )
  {
    UtlErrorHwnd( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszListFile, EQF_ERROR, hwnd );
    return( FALSE );
  } /* endif */

  // write BOM 
  fwrite( UNICODEFILEPREFIX, strlen(UNICODEFILEPREFIX), 1, hfList );

  // write batch list entries
  for( int i = 0; i < pIda->iBatchListUsed; i++ )
  {
    PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[i];

    WriteStringToExportedBatchList( hfList, (PSZ_W)(((PBYTE)pEntry) + pEntry->iTargetFindOffs) );
    fwrite( L",", sizeof(CHAR_W), 1, hfList );
    WriteStringToExportedBatchList( hfList, (PSZ_W)(((PBYTE)pEntry) + pEntry->iSourceFindOffs) );
    fwrite( L",", sizeof(CHAR_W), 1, hfList );
    WriteStringToExportedBatchList( hfList, (PSZ_W)(((PBYTE)pEntry) + pEntry->iTargetChangeOffs) );
    fwrite( L"\r\n", sizeof(CHAR_W), 2, hfList );
  }
  
  fclose( hfList );

  return( TRUE );
}

// listview sub item edit control subclass procedure
long _stdcall EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){
		case WM_KILLFOCUS:
		{
      if ( hEdit != NULL ) // get current text only when the edit control is available
      {
			  LV_DISPINFOW lvDispinfo;
			  ZeroMemory(&lvDispinfo,sizeof(LV_DISPINFO));
			  lvDispinfo.hdr.hwndFrom = hwnd;
			  lvDispinfo.hdr.idFrom = GetDlgCtrlID(hwnd);
			  lvDispinfo.hdr.code = LVN_ENDLABELEDIT;
			  lvDispinfo.item.mask = LVIF_TEXT;
			  lvDispinfo.item.iItem = iItem;
			  lvDispinfo.item.iSubItem = iSubItem;
        lvDispinfo.item.pszText = NULL;
        SendMessageW( hwnd, WM_GETTEXT, sizeof(szEditText) / sizeof(CHAR_W), (LPARAM)szEditText );
			  lvDispinfo.item.pszText = szEditText;
			  lvDispinfo.item.cchTextMax = wcslen(szEditText);
			  SendMessage( hwndBatchListTab, WM_NOTIFY, (WPARAM)ID_FOLFIND_BATCHLIST_LISTVIEW, (LPARAM)&lvDispinfo); //the LV ID and the LVs Parent window's HWND
        hEdit = NULL;
  			DestroyWindow(hwnd);
      }
			break;
		}
	}

	return CallWindowProcW(EOldProc, hwnd, message, wParam, lParam);
}

// listview subclass procedure
long _stdcall GFR_ListViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){
		case WM_LBUTTONDBLCLK:
		{
      CallWindowProc(LVOldProc, hwnd, message, wParam, lParam);
			if (hEdit != NULL){ SendMessageW( hEdit, WM_KILLFOCUS, 0, 0 );};
			LVHITTESTINFO itemclicked;
			long x, y;
			x = (long)LOWORD(lParam);
			y = (long)HIWORD(lParam);
			itemclicked.pt.x = x;
			itemclicked.pt.y = y;
			int lResult = ListView_SubItemHitTest(hwnd,&itemclicked);
			if ( (lResult != -1) && (itemclicked.iSubItem != 0) )
      {
				RECT subitemrect;
        LV_ITEMW lvi;
				ListView_GetSubItemRect(hwnd,itemclicked.iItem,itemclicked.iSubItem,LVIR_BOUNDS,&subitemrect);
				int iHeight = subitemrect.bottom - subitemrect.top;
				int iWidth = subitemrect.right - subitemrect.left;
        memset( &lvi, 0, sizeof(lvi) );
        lvi.iItem = itemclicked.iItem;
        lvi.iSubItem = itemclicked.iSubItem;
        lvi.pszText = szEditText;
        lvi.cchTextMax = sizeof(szEditText) / sizeof(CHAR_W);
        HFONT hFont = (HFONT)SendMessage( hwnd, WM_GETFONT, 0, 0L );
        SendMessage( hwnd, LVM_GETITEMTEXTW, (WPARAM)itemclicked.iItem, (LPARAM)&lvi );
        RECT rcLV;
        GetWindowRect( hwnd, &rcLV );
        MapWindowPoints( HWND_DESKTOP, GetParent( hwnd ), (LPPOINT)&rcLV, 2 );
				hEdit = CreateWindowExW( WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_WANTRETURN, 
                                 rcLV.left + subitemrect.left, rcLV.top + subitemrect.top, iWidth, 1.5 * iHeight, GetParent( hwnd ), (HMENU)4711, (HINSTANCE)(HAB)UtlQueryULong( QL_HAB ), NULL);
        if ( hFont != NULL ) SendMessage( hEdit, WM_SETFONT, (WPARAM)hFont, 0L ); 
        SetWindowTextW( hEdit, szEditText );
        SendMessage( hEdit, EM_SETSEL, (WPARAM)0, (LPARAM)-1 );
				SetFocus(hEdit);
				EOldProc = (WNDPROC)SetWindowLongW(hEdit, GWL_WNDPROC, (LONG)EditProc);
				iItem = itemclicked.iItem;
				iSubItem = itemclicked.iSubItem;
			}
			return 0;
			break;
		}
	}
	return CallWindowProc(LVOldProc, hwnd, message, wParam, lParam);
}

// replace a resource file created edit control with its Unicode enabled counterpart
HWND GFR_ReplaceWithUnicodeEditControl( HWND hwndDlg, int iID, HWND hwndInsertBehind )
{
  WINDOWPLACEMENT Placement;
  HWND hwndEF   = GetDlgItem( hwndDlg, iID );
  HFONT   hFont;

  GetWindowPlacement( hwndEF, &Placement );
  hFont = (HFONT)SendMessage( hwndEF, WM_GETFONT, 0, 0L );
  DestroyWindow( hwndEF );
  hwndEF = CreateWindowExW( 0, L"Edit", L"", 
                            ES_LEFT | WS_BORDER | WS_TABSTOP | WS_VISIBLE | ES_AUTOHSCROLL| WS_CHILD,
                            Placement.rcNormalPosition.left, Placement.rcNormalPosition.top, 
                            Placement.rcNormalPosition.right - Placement.rcNormalPosition.left, 
                            Placement.rcNormalPosition.bottom - Placement.rcNormalPosition.top, 
                            hwndDlg, (HMENU)iID, (HINSTANCE)(HAB)UtlQueryULong( QL_HAB ), 0 );
  SetWindowLong( hwndEF, GWL_ID, (LONG)iID );
  SetWindowPos( hwndEF, hwndInsertBehind, 0, 0, Placement.rcNormalPosition.right - Placement.rcNormalPosition.left, 
                            Placement.rcNormalPosition.bottom - Placement.rcNormalPosition.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW );
  if ( hFont != NULL ) SendMessage( hwndEF, WM_SETFONT, (WPARAM)hFont, 0L ); 
  return( hwndEF );
}

// test if on-spot editing in the batch list is active
BOOL GFR_IsOnSpotEditingActive()
{
  return( hEdit != NULL );
}

// end on-spot editing of a batch list entry
void GFR_EndOnSpotEditing()
{
  HWND hwndTemp = hEdit;
  hEdit = NULL;
  DestroyWindow( hwndTemp );
}
