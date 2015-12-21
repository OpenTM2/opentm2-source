/*! \file
	Description: EQF Folder handler

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqfmfc.h"

#ifndef CPPTEST
extern "C"
{
#endif
  #define INCL_EQF_FOLDER              // folder list and document list functions
  #define INCL_EQF_EDITORAPI        // for WM_EQF_SHOWHTML message


/**********************************************************************/
/* ensure correct packing of our structures                           */
/**********************************************************************/
#pragma pack( push, TM2StructPacking, 1 )

  #include "eqf.h"                     // General .H for EQF
  #include "eqfstart.id"               // IDs for EQFSTARR resource
  #include "eqfutmdi.h"                // MDI for dialogs
#include "EQFB.ID"                     // IDs of Translation Processor window

#include "eqfdde.h"               // batch mode definitions
#include "eqfstart.h"
#include "eqffll00.h"             // Folder List Handler defines
#include "eqffol00.h"             // Folder handler defines

//cv
#include "eqfutdlg.h"
#include "eqfdoc01.h"

#pragma pack( pop, TM2StructPacking )

#ifndef CPPTEST
}
#endif

#include "eqfclbox.h"                  // base class for Column listbox
#include "eqfcfrm.h"                   // child frame base class
#include "eqfgen99.h"                  // generic listwindow
#include "eqffll99.h"                  // folder list handler specifics
//#include "eqfmfrm.h"                   // base class for MDI Childframes
#include <dde.h>                       // DDE defines
#include "eqfmfrm.h"                   // main frame base class

#include "eqfd.h"

/**********************************************************************/
/* Build up the new Folder/SubFolder tree                             */
/* lParam items in the Tree are set as follows:                       */
/*   ROOT_ITEM                                                        */
/*   FOLDER_ITEM                                                      */
/*   SUBFOLDER_ITEM                                                   */
/**********************************************************************/
#define ROOT_ITEM         1
#define FOLDER_ITEM       2
#define SUBFOLDER_ITEM    3


void  SetFolderTitle( CHAR *, CWnd * ) ;

/////////////////////////////////////////////////////////////////////////////
// CFLLProcDoc

IMPLEMENT_DYNCREATE(CFLLProcDoc, CEQFDoc)

BEGIN_MESSAGE_MAP(CFLLProcDoc, CEQFDoc)
        //{{AFX_MSG_MAP(CFLLProcDoc)
                // NOTE - the ClassWizard will add and remove mapping macros here.
                //    DO NOT EDIT what you see in these blocks of generated code!

        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFLLProcDoc construction/destruction

CFLLProcDoc::CFLLProcDoc()
{

  fUserFlag = FALSE;

  m_hMyMenu = ::LoadMenu( AfxGetResourceHandle(),
                          MAKEINTRESOURCE(ID_TWBM_WINDOW));
  UtlSetULong( QL_TREEVIEWMENU, (ULONG)m_hMyMenu );
}

CFLLProcDoc::~CFLLProcDoc()
{
  DestroyMenu( m_hMyMenu );
}

BOOL CFLLProcDoc::OnNewDocument()
{
  if (!CEQFDoc::OnNewDocument())
          return FALSE;

  SetTitle( (PSZ) ((CFLLListView*)pActView)->pIda->CommArea.szTitle );

  EqfRegisterObject( GetObjName(), (pActView->m_hWnd),
                     ((CFLLListView*)pActView)->pIda->CommArea.sListObjClass );

  return TRUE;
}


HMENU CFLLProcDoc::GetDefaultMenu()
{
  return m_hMyMenu;
}

/////////////////////////////////////////////////////////////////////////////
// CFLLProcDoc helper functions



/////////////////////////////////////////////////////////////////////////////
// CFLLProcDoc serialization

void CFLLProcDoc::Serialize(CArchive& ar)
{
        if (ar.IsStoring())
        {
                // TODO: add storing code here
        }
        else
        {
                // TODO: add loading code here
        }
}

/////////////////////////////////////////////////////////////////////////////
// CFLLProcDoc diagnostics

#ifdef _DEBUG
void CFLLProcDoc::AssertValid() const
{
        CEQFDoc::AssertValid();
}

void CFLLProcDoc::Dump(CDumpContext& dc) const
{
        CEQFDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFLLProcDoc commands

#define IDC_LEFTTREEVIEW 0xe900   // magic number - first view in splitter window

/////////////////////////////////////////////////////////////////////////////
// CFLLListView

IMPLEMENT_DYNCREATE(CFLLListView, CTreeView)

BEGIN_MESSAGE_MAP(CFLLListView, CTreeView)
        //{{AFX_MSG_MAP(CFLLListView)
        ON_WM_CREATE()
        ON_WM_RBUTTONDOWN()
        ON_COMMAND(PID_UTILS_MI_TAGTABLE,      CListViewEx::OnTagTable)
        ON_COMMAND(PID_UTILS_MI_LNGUPDATE,     CListViewEx::OnLangList)
        ON_COMMAND(PID_UTILS_MI_EXCLUSION,     CListViewEx::OnExclList)
        ON_COMMAND(PID_UTILS_MI_NEWTERMS,      CListViewEx::OnNewTerms)
        ON_COMMAND(PID_UTILS_MI_FOUNDTERMS,    CListViewEx::OnFoundTerms)
        ON_COMMAND(PID_FILE_MI_SELECTALL,      CListViewEx::SelectAll)
        ON_COMMAND(PID_FILE_MI_DESELECTALL,    CListViewEx::DeSelectAll)
        ON_COMMAND(PID_FILE_MI_SYSPROP,        CListViewEx::OnSysProps)
        ON_COMMAND(PID_FILE_MI_EXIT,           CListViewEx::OnExit)
        ON_MESSAGE( WM_EQF_MDIACTIVATE, On_WM_EQF_MDIACTIVATE )
        ON_MESSAGE( WM_INITMENUPOPUP,  OnInitMenuPopup )
        ON_MESSAGE( WM_EQF_INITIALIZE, OnFillFolderTree)
        ON_MESSAGE( WM_EQFN_DELETEDNAME,   OnEQFNDeletedName )
        ON_MESSAGE( WM_EQFN_DELETED,   OnEQFNDeleted )
        ON_MESSAGE( WM_EQFN_CREATED,   OnEQFNNew )
        ON_MESSAGE( WM_EQFN_DRIVEADDED,  OnDriveChanged)
        ON_MESSAGE( WM_EQFN_DRIVEREMOVED,  OnDriveChanged)
        ON_NOTIFY(TVN_SELCHANGING, IDC_LEFTTREEVIEW, ShowNotification)
        ON_NOTIFY(TVN_SELCHANGED,  IDC_LEFTTREEVIEW, OnSelchanged)
        ON_NOTIFY(TVN_GETDISPINFO, IDC_LEFTTREEVIEW, ShowNotification)
        ON_NOTIFY(TVN_SETDISPINFO, IDC_LEFTTREEVIEW, ShowNotification)
        ON_NOTIFY(TVN_ITEMEXPANDING, IDC_LEFTTREEVIEW, ShowNotification)
        ON_NOTIFY(TVN_ITEMEXPANDED, IDC_LEFTTREEVIEW, ShowNotification)
        ON_NOTIFY(TVN_BEGINDRAG, IDC_LEFTTREEVIEW, ShowNotification)
        ON_NOTIFY(TVN_KEYDOWN, IDC_LEFTTREEVIEW, OnKeyDown)
        ON_NOTIFY(TVN_ENDLABELEDIT, IDC_LEFTTREEVIEW, OnRenameFolder)
        ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_LEFTTREEVIEW, OnRenameFolderBegin)


        ON_COMMAND(PID_WIND_MI_CASCADE ,       OnMDICascade )
        ON_COMMAND(PID_WIND_MI_TILE ,          OnMDITile )
        ON_COMMAND(PID_WIND_MI_MINALL,         OnMDIMinAll )
        ON_COMMAND(PID_WIND_MI_RESTOREALL,     OnMDIRestore )
        ON_COMMAND(PID_VIEW_MI_SORT,           CListViewEx::OnSortDlg)      // use base ones
        ON_COMMAND(PID_VIEW_MI_ALL,            CListViewEx::OnAll)          // use base ones
        ON_COMMAND(PID_VIEW_MI_SOME,           CListViewEx::OnFilterDlg)    // use base ones
        ON_COMMAND(PID_VIEW_MI_DETAILSDLG,     OnDetailsDlg)
        ON_COMMAND(PID_VIEW_MI_NAMES,          CListViewEx::OnNamesView)    // use base ones
        ON_COMMAND(PID_VIEW_MI_DETAILS,        CListViewEx::OnDetailsView)  // use base ones

        ON_COMMAND(PID_UTILS_MI_TAGTABLE,      CListViewEx::OnTagTable)     // use base ones
        ON_COMMAND(PID_UTILS_MI_LNGUPDATE,     CListViewEx::OnLangList)     // use base ones
        ON_COMMAND(PID_UTILS_MI_EXCLUSION,     CListViewEx::OnExclList)     // use base ones
        ON_COMMAND(PID_UTILS_MI_NEWTERMS,      CListViewEx::OnNewTerms)     // use base ones
        ON_COMMAND(PID_UTILS_MI_FOUNDTERMS,    CListViewEx::OnFoundTerms)   // use base ones
        ON_COMMAND(PID_UTILS_MI_DRIVES,        CListViewEx::OnConfDrives)   // usebase ones
        ON_COMMAND(PID_UTILS_MI_PLGINMGR,      CListViewEx::OnPluginManager)  // new start
        ON_COMMAND(PID_UTILS_MI_ATOVERUP,      CListViewEx::OnAutoVersionUp)  // new end
        ON_COMMAND(PID_UTILS_MI_CONNECT,       CListViewEx::OnConnectResources)   // usebase ones
        ON_COMMAND(PID_FILE_MI_PRINTLIST,      CListViewEx::OnPrintList)
        ON_COMMAND(PID_FILE_MI_SELECTALL,      CListViewEx::SelectAll)
        ON_COMMAND(PID_FILE_MI_DESELECTALL,    CListViewEx::DeSelectAll)
        ON_COMMAND(PID_FILE_MI_SYSPROP,        CListViewEx::OnSysProps)
        ON_COMMAND(PID_FILE_MI_EXIT,           CListViewEx::OnExit)
        ON_COMMAND(PID_FILE_MI_EBUS,           OnEBus )
        ON_COMMAND(PID_FILE_MI_EBUS1,          OnEBus1 )
        ON_COMMAND(PID_FILE_MI_EBUS2,          OnEBus2 )

        ON_MESSAGE( WM_EQF_TERMINATE,          On_WM_EQF_TERMINATE)
        ON_MESSAGE( WM_EQF_INSERTNAMES,        On_WM_EQF_INSERTNAMES )
        ON_MESSAGE( LB_GETCOUNT,               OnGetCount )
        ON_MESSAGE( LM_EQF_QUERYITEMSTATE,     OnQueryItemState )
        ON_MESSAGE( LB_GETTEXT,                OnGetText )
        ON_MESSAGE( LB_GETCURSEL,              OnGetCurSel )
        ON_MESSAGE( WM_EQF_SHOWHTML,           On_WM_EQF_SHOWHTML )

        //}}AFX_MSG_MAP

        ON_COMMAND(ID_FILE_PRINT, CTreeView::OnFilePrint)
        ON_COMMAND(ID_FILE_PRINT_DIRECT, CTreeView::OnFilePrint)
        ON_COMMAND(ID_FILE_PRINT_PREVIEW, CTreeView::OnFilePrintPreview)

       ON_COMMAND_RANGE( PID_WIND_MI_TOP, PID_WIND_MI_BOT, OnWndActivate )
       ON_COMMAND_RANGE( ID_TWBM_AAB, ID_TWBM_AAB_LAST, On_WM_COMMAND )
       ON_UPDATE_COMMAND_UI( ID_INDICATOR_CAPS, OnUpdateStatusTextCaps )
       ON_UPDATE_COMMAND_UI_RANGE(ID_TWBM_AAB , ID_TWBM_AAB_LAST, OnToolbarActivate)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFLLListView

#define EQFFOLDER        "EqfFolder"

/**********************************************************************/
/* On_WM_EQF_SHOWHTML:  pass on request to base function              */
/**********************************************************************/
LRESULT CFLLListView::On_WM_EQF_SHOWHTML( WPARAM mp1, LPARAM mp2 )
{
  /********************************************************************/
  /* let RightPaneView do the dirty work -- it is a GenListView wnd.. */
  /********************************************************************/
  return ((CFLLRightPaneView *)((CSplitterWnd*)GetParent())->GetPane(0,1))->SendMessage(WM_EQF_SHOWHTML, mp1, mp2 );
}


/**********************************************************************/
/* GetCurSel: Returns the hItem of the selected item - not the index  */
/**********************************************************************/
LRESULT CFLLListView::OnGetCurSel( WPARAM mp1, LPARAM mp2 )
{
  mp1; mp2;
  return (LONG) GetTreeCtrl().GetSelectedItem();
}

LRESULT CFLLListView::OnGetCount( WPARAM mp1, LPARAM mp2 )
{
  mp1; mp2;
  return GetTreeCtrl().GetCount();
}

/**********************************************************************/
/* OnQueryItemState:  returns the state of the folder item            */
/* -- we always return TRUE in the case of the folder tree            */
/**********************************************************************/
LRESULT CFLLListView::OnQueryItemState( WPARAM mp1, LPARAM mp2 )
{
  mp1; mp2;
  return TRUE;
}

/**********************************************************************/
/* Get text from item #i                                              */
/*   -- mp1: hItem                                                    */
/*   -- mp2: ptr to buffer                                            */
/**********************************************************************/
LRESULT CFLLListView::OnGetText( WPARAM mp1, LPARAM mp2 )
{
  HTREEITEM hItem = (HTREEITEM) mp1;
  TV_ITEM tvItem;
  /********************************************************************/
  /* get selected item and pass objectname as mp2                     */
  /********************************************************************/
  memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
  tvItem.mask = TVIF_TEXT | TVIF_PARAM;
  tvItem.pszText = pIda->CommArea.szBuffer;
  tvItem.cchTextMax = sizeof( pIda->CommArea.szBuffer );
  tvItem.hItem = hItem;
  GetTreeCtrl().GetItem( &tvItem );

  // for subfolders we have to go up the chain...
  if ( LOWORD(tvItem.lParam) == SUBFOLDER_ITEM )
  {
     HTREEITEM hParent = hItem;
     do
     {
       hParent = GetTreeCtrl().GetParentItem( hParent );
       memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
       tvItem.mask = TVIF_TEXT | TVIF_PARAM;
       tvItem.pszText = pIda->CommArea.szBuffer;
       tvItem.cchTextMax = sizeof(pIda->CommArea.szBuffer);
       tvItem.hItem = hParent;
       GetTreeCtrl().GetItem( &tvItem );
     } while ( LOWORD(tvItem.lParam) == SUBFOLDER_ITEM );
  } /* endif */

  BOOL fIsNew = FALSE;
  PSZ pszObjName = (PSZ) pIda->CommArea.szBuffer;
  CHAR szNewShortName[ MAX_FILESPEC ];
  ANSITOOEM( pszObjName );
  ObjLongToShortName( pszObjName, szNewShortName, FOLDER_OBJECT, &fIsNew );
  UtlMakeEQFPath( pszObjName, NULC, SYSTEM_PATH, NULL );
  strcat( pszObjName, BACKSLASH_STR   );
  strcat( pszObjName, szNewShortName  );
  strcat( pszObjName, EXT_FOLDER_MAIN );

  /********************************************************************/
  /* Build folder item name will be returned in CommArea.szBuffer     */
  /********************************************************************/
  pIda->pfnCallBack( &pIda->CommArea , m_hWnd, WM_EQF_BUILDITEMTEXT,
                     0, (LONG) pszObjName );
  if ( mp2 !=  (LONG)&pIda->CommArea.szBuffer[0] )
  {
    strcpy( (PSZ) mp2, pIda->CommArea.szBuffer );
  } /* endif */
  return strlen( (PSZ) mp2 );

}

/**********************************************************************/
/* force insert of foldernames -- pass it on                          */
/**********************************************************************/
LRESULT CFLLListView::On_WM_EQF_INSERTNAMES( WPARAM mp1, LPARAM mp2 )
{
  mp2;
  LRESULT mResult = 0;
  if ( pIda )
  {
    HWND       hwndTargetLB = HWNDFROMMP1(mp1);
    BOOL       fCombo;


    ISCOMBOBOX( hwndTargetLB, fCombo );


    HTREEITEM hItem = GetTreeCtrl().GetRootItem();

    // find current name in childs of hItem
    hItem = GetTreeCtrl().GetNextItem( hItem, TVGN_CHILD );

    while ( hItem )
    {
      TV_ITEM tvItem;
      memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
      tvItem.mask = TVIF_TEXT | TVIF_PARAM;
      tvItem.pszText = pIda->CommArea.szBuffer;
      tvItem.cchTextMax = sizeof( pIda->CommArea.szBuffer );
      tvItem.hItem = hItem;
      GetTreeCtrl().GetItem( &tvItem );

      // for subfolders we have to go up the chain...
      if ( LOWORD(tvItem.lParam) == SUBFOLDER_ITEM )
      {
         HTREEITEM hParent = hItem;
         do
         {
           hParent = GetTreeCtrl().GetParentItem( hParent );
           memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
           tvItem.mask = TVIF_TEXT | TVIF_PARAM;
           tvItem.pszText = pIda->CommArea.szBuffer;
           tvItem.cchTextMax = sizeof(pIda->CommArea.szBuffer);
           tvItem.hItem = hParent;
           GetTreeCtrl().GetItem( &tvItem );
         } while ( LOWORD(tvItem.lParam) == SUBFOLDER_ITEM );
      } /* endif */

      if ( tvItem.pszText[0] )
      {
        if ( fCombo )
        {
          CBINSERTITEMHWND( hwndTargetLB, tvItem.pszText );
        }
        else
        {
          INSERTITEMHWND( hwndTargetLB, tvItem.pszText );
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* move to next item ...                                        */
      /****************************************************************/
      hItem = GetTreeCtrl().GetNextItem( hItem, TVGN_NEXT );
    } /* endwhile */

    if ( fCombo )
    {
      mResult = MRFROMSHORT( CBQUERYITEMCOUNTHWND( hwndTargetLB ) );
    }
    else
    {
      mResult = MRFROMSHORT( QUERYITEMCOUNTHWND( hwndTargetLB ) );
    } /* endif */
  } /* endif */

  return( mResult );
}

/**********************************************************************/
/* Window is activated/deactivated                                    */
/**********************************************************************/
VOID CFLLListView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView )
{
  CView::OnActivateView( bActivate, pActivateView, pDeactiveView );
  if ( bActivate )
  {
    HWND hwnd = EqfQueryObject( NULL, clsFOLDERLIST, OBJ_ACTIVE | OBJ_BUSY );
    if ( !hwnd )
    {
       EqfActivateInstance( m_hWnd, bActivate );
    } /* endif */
  }
}

/**********************************************************************/
/* TWB is shutdown - store any positions if necessary                 */
/**********************************************************************/
LRESULT CFLLListView::On_WM_EQF_TERMINATE( WPARAM mp1, LPARAM mp2 )
{
  EQF_SWP	Tstswp;
  CFLLListView     * pWnd;
  pWnd = (CFLLListView *)(((CSplitterWnd *)GetParent())->GetPane(0,0));

  UtlSaveWindowPos(pWnd->m_hWnd, &Tstswp );
  {
	PPROPSYSTEM pPropSys = GetSystemPropPtr();
    pPropSys->sLeftPaneWidth = Tstswp.cx;
  }
  UtlSaveWindowPos( pIda->hFrame, &(pIda->CommArea.swpSizePos) );
  return pIda->pfnCallBack( &pIda->CommArea , m_hWnd, WM_EQF_TERMINATE, mp1, mp2 );
}


VOID CFLLListView::OnMDIMinAll()
{
    ((CChildFrame *)GetParent()->GetParent())->OnMDIMinAll();
}

VOID CFLLListView::OnMDIRestore()
{
    ((CChildFrame *)GetParent()->GetParent())->OnMDIRestore();
};

/**********************************************************************/
/* create window using our own window proc...                         */
/**********************************************************************/
int CFLLListView::OnCreate(LPCREATESTRUCT cs)
{
  LRESULT mResult = pIda->pfnCallBack( &pIda->CommArea , m_hWnd, WM_CREATE, NULL,
                                       (LPARAM) pIda->pvUserData  );
  // return asap if no window should be created
  if ( (SHORT)mResult == DO_NOT_CREATE )
  {
      return( -1 );
  } /* endif */

  /****************************************************************/
  /* Set flags for handling of WM_EQFN_ messages                  */
  /****************************************************************/
  PSHORT   psMsg = pIda->CommArea.asMsgsWanted;

  while ( *psMsg != 0 )
  {
      switch ( *psMsg )
      {
      case WM_EQFN_PROPERTIESCHANGED :
          pIda->fWantsPropMsg = TRUE;
          break;
      case WM_EQF_QUERYSELECTEDNAMES :
          pIda->fWantsSelectedNamesMsg = TRUE;
          break;
      case WM_EQFN_DELETED :
          pIda->fWantsDelMsg = TRUE;
          break;
      case WM_EQFN_CREATED :
          pIda->fWantsCreateMsg = TRUE;
          break;
      } /* endswitch */
      psMsg++;
  } /* endwhile */

// check if we need multiple selections
  if ( !pIda->CommArea.fMultipleSel )
  {
      cs->style |= LVS_SINGLESEL;
  } /* endif */
  cs->lpszClass = GetMDIStruct()->szClass;
  return CView::OnCreate(cs);
}



/**********************************************************************/
/* Test activation                                                    */
/**********************************************************************/
LRESULT CFLLListView::On_WM_EQF_MDIACTIVATE( WPARAM mp1, LPARAM mp2 )
{
    if ( mp1 )
    {
        /********************************************************************/
        /* send activation request                                          */
        /********************************************************************/
        GENERICLISTWP( m_hWnd, WM_MDIACTIVATE, 0L, MP2FROMHWND(pIda->CommArea.hwndLB) );

        EqfActivateInstance( m_hWnd, TRUE );
    }
    else
    {
        /******************************************************************/
        /* mp2 contains handle of window to be deactivated                */
        /******************************************************************/
        CChildFrame *pWnd = (CChildFrame *) mp2;
        if ( mp2 )
        {
          CView*   pActView = pWnd->GetActiveView();
          GENERICLISTWP( m_hWnd, WM_MDIACTIVATE, 0L, MP2FROMHWND( pActView->m_hWnd ));
          EqfActivateInstance( m_hWnd, FALSE );
        } /* endif */
    } /* endif */
    return 0;
}

VOID CFLLListView::OnWndActivate( UINT mp1 )
{
  CChildFrame  *pChildFrame = (CChildFrame *)(GetParent()->GetParent());
  pChildFrame->OnToggleWindow( mp1 );
}


VOID CFLLListView::OnDetailsDlg( )
{
  ((CFLLRightPaneView *)((CSplitterWnd*)GetParent())->GetPane(0,1))->OnDetailsDlgWrapper();
}


void CFLLListView::OnToolbarActivate(CCmdUI* pCmdUI)
{
  UINT   usID = pCmdUI->m_nID;
  BOOL   fEnable = FALSE;

  /********************************************************************/
  /* disable selection if rootitem selected                           */
  /********************************************************************/
  HTREEITEM hTreeItem = GetTreeCtrl().GetSelectedItem();
  TV_ITEM tvItem;
  memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
  tvItem.mask = TVIF_PARAM;
  tvItem.hItem = hTreeItem;
  GetTreeCtrl().GetItem( &tvItem );
  /********************************************************************/
  /* enable selection for all active objects                          */
  /********************************************************************/
  if ( (usID >= ID_TWBM_AAB_TOOLPLUGINS) && (usID < ID_TWBM_AAB_LAST) )
  {
    // enable all tool plugin menu items
    fEnable = TRUE;
  }
  else if ( (usID >= PID_WIND_MI_TOP) && ( usID <= PID_WIND_MI_BOT) )
  {
    // enable all window menu items
    fEnable = TRUE;
  }
  else if ( (LOWORD(tvItem.lParam) == ROOT_ITEM) )
  {
    // root item -- nothing should be enabled
    switch ( usID )
    {
      case PID_UTILS_MI_TAGTABLE:
      case PID_UTILS_MI_LNGUPDATE:
      case PID_UTILS_MI_PLGINMGR:  // new start
      case PID_UTILS_MI_ATOVERUP:  // new end
      case PID_UTILS_MI_DRIVES:
      case PID_UTILS_MI_EXCLUSION:
      case PID_UTILS_MI_NEWTERMS:
      case PID_UTILS_MI_FOUNDTERMS:
      case PID_UTILS_MI_ABBR:
      case PID_WIND_MI_MINALL:
      case PID_WIND_MI_RESTOREALL:
      case PID_WIND_MI_TILE:
      case PID_WIND_MI_CASCADE:
      case PID_FILE_MI_EBUS:
      case PID_FILE_MI_EBUS1:
      case PID_FILE_MI_EBUS2:
      case PID_FILE_MI_SYSPROP:
      case PID_FILE_MI_EXIT:
      case PID_FILE_MI_IMPORT:
      case PID_VIEW_TWB_TOOLBAR:
      case PID_VIEW_TPRO_TOOLBAR:
      case PID_VIEW_MI_STATUSBAR:
      case PID_FILE_MI_NEW:
      case PID_UTILS_MI_DELEXPMAT:
      case PID_UTILS_MI_MT:
          /****************************************************************/
          /* enable above items all the time..                            */
          /****************************************************************/
          fEnable = TRUE;
          break;
    } /* endswitch */
  }
  else
  {
      switch ( usID )
      {
      case PID_UTILS_MI_TAGTABLE:
      case PID_UTILS_MI_LNGUPDATE:
      case PID_UTILS_MI_PLGINMGR:  // new start
      case PID_UTILS_MI_ATOVERUP:  // new end
      case PID_UTILS_MI_DRIVES:
      case PID_UTILS_MI_EXCLUSION:
      case PID_UTILS_MI_NEWTERMS:
      case PID_UTILS_MI_FOUNDTERMS:
      case PID_UTILS_MI_ABBR:
      case PID_WIND_MI_MINALL:
      case PID_WIND_MI_RESTOREALL:
      case PID_WIND_MI_TILE:
      case PID_WIND_MI_CASCADE:
      case PID_FILE_MI_EBUS:
      case PID_FILE_MI_EBUS1:
      case PID_FILE_MI_EBUS2:
      case PID_FILE_MI_SYSPROP:
      case PID_FILE_MI_EXIT:
      case PID_VIEW_TWB_TOOLBAR:
      case PID_VIEW_TPRO_TOOLBAR:
      case PID_VIEW_MI_STATUSBAR:
      case PID_FILE_MI_NEW:
      case PID_VIEW_MI_NAMES:
      case PID_VIEW_MI_DETAILSDLG:
      case PID_VIEW_MI_DETAILS:
      case PID_UTILS_MI_DELEXPMAT:
      case PID_FILE_MI_PRINTLIST:
      case PID_UTILS_MI_MT:
          /****************************************************************/
          /* enable above items all the time..                            */
          /****************************************************************/
          fEnable = TRUE;
          break;
      case PID_FILE_MI_IMPORT:
          if ( (LOWORD(tvItem.lParam) == SUBFOLDER_ITEM) )
          {
            fEnable = FALSE;
          }
          else
          {
            fEnable = ( EqfQueryObject( NULL, clsFOLDERIMP, 0) == NULLHANDLE );
          } /* endif */
          break;
      case PID_FILE_MI_RENAME:
          if ( (LOWORD(tvItem.lParam) == SUBFOLDER_ITEM) )
          {
            fEnable = FALSE;
          }
          else
          {
            fEnable = (hTreeItem != NULL);
          } /* endif */
          break;
      case PID_FILE_MI_OPEN:
      case PID_FILE_MI_PROPERTIES:
      case PID_FILE_MI_HTMLPROPS:
      case PID_UTILS_MI_REPORT:
      case PID_FILE_MI_DELETE:
          fEnable = (hTreeItem != NULL);
          break;
      case PID_FILE_MI_EXPORT:
          if ( (LOWORD(tvItem.lParam) == SUBFOLDER_ITEM) )
          {
            fEnable = FALSE;
          }
          else
          {
            fEnable = (hTreeItem != NULL);
            if ( fEnable )
            {
               fEnable =  (EqfQueryObject( NULL, clsFOLDEREXP, 0) == NULLHANDLE);
            } /* endif */
          } /* endif */
          break;
      case PID_FILE_MI_ANALYZE:
      case PID_UTILS_MI_COUNT:
      case PID_UTILS_MI_ARCHTM:
      case PID_UTILS_MI_EXPORTSEGS:
      case PID_FILE_MI_FINDCHANGE:
      case PID_FILE_MI_FUZZYSEARCH:
      case PID_UTILS_MI_DELETEDOCS:
          fEnable = (hTreeItem != NULL);
          if ( fEnable )
          {
            fEnable = FolderNotEmpty();
          } /* endif */
          break;

      case PID_UTILS_MI_RESTOREDOCS:
          fEnable = (hTreeItem != NULL);
          if ( fEnable )
          {
            fEnable = IsRemovedDirNotEmpty();
          } /* endif */
          break;




      default:
          break;
      } /* endswitch */
  } /* endif */
  pCmdUI->Enable( fEnable );
}

void CFLLListView::OnUpdateStatusTextCaps(CCmdUI *pCmdUI)
{

    pCmdUI->SetText( "" );

    CChildFrame  *pChildFrame = (CChildFrame *)(GetParent()->GetParent());
    CMainFrame   *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();

    USHORT usI = 1;
    UINT usJ = GetTreeCtrl().GetCount();
    CHAR  chText[50];
    CHAR  szString[40];
    if ( usJ == 0 )
    {
      LOADSTRING( NULL, AfxGetResourceHandle(), SID_STATUSBAR_OBJ_TEXT_NOSEL, szString );
      sprintf( chText, szString, usI );
    }
    else
    {
      LOADSTRING( NULL, AfxGetResourceHandle(), SID_STATUSBAR_OBJ_TEXT_SEL, szString );
      sprintf( chText, szString, usI, usJ );
    } /* endif */
    pFrameWnd->OnSetStatusBar( 0, chText );
}

BOOL CFLLListView::FolderNotEmpty()
{
  BOOL fEnable = FALSE;
  HTREEITEM hTreeItem = GetTreeCtrl().GetSelectedItem();
  TV_ITEM tvItem;
  /********************************************************************/
  /* get selected item and pass objectname as mp2                     */
  /********************************************************************/
  memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
  tvItem.mask = TVIF_TEXT | TVIF_PARAM;
  tvItem.pszText = pIda->CommArea.szBuffer;
  tvItem.cchTextMax = sizeof( pIda->CommArea.szBuffer );
  tvItem.hItem = hTreeItem;
  GetTreeCtrl().GetItem( &tvItem );

  // for subfolders we have to go up the chain...
  if ( LOWORD(tvItem.lParam) == SUBFOLDER_ITEM )
  {
     HTREEITEM hParent = hTreeItem;
     do
     {
       hParent = GetTreeCtrl().GetParentItem( hParent );
       memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
       tvItem.mask = TVIF_TEXT | TVIF_PARAM;
       tvItem.pszText = pIda->CommArea.szBuffer;
       tvItem.cchTextMax = sizeof(pIda->CommArea.szBuffer);
       tvItem.hItem = hParent;
       GetTreeCtrl().GetItem( &tvItem );
     } while ( LOWORD(tvItem.lParam) == SUBFOLDER_ITEM );
  } /* endif */


  PSZ pszObjName = (PSZ) pIda->CommArea.szBuffer;
  OBJNAME   szDocPath;
  strcpy( (PSZ)szDocPath, pszObjName );
  ANSITOOEM( (PSZ)szDocPath );
  SubFolNameToObjectName( (PSZ)szDocPath, pszObjName );


  /**********************************************************/
  /* Check if folder contains documents before enabling     */
  /* document based menu items                              */
  /**********************************************************/
  PSZ       pszDocPath = (PSZ) &szDocPath;
  USHORT    usCount = 1;
  FILEFINDBUF ResultBuf;
  USHORT    usRC;
  HDIR      hDirHandle = HDIR_CREATE;

  // setup path to folder documents
  strcpy( pszDocPath, pszObjName );
  strcat( pszDocPath, BACKSLASH_STR );
  UtlQueryString( QST_SOURCEDIR, pszDocPath + strlen(pszDocPath),
                  MAX_FILESPEC );
  strcat( pszDocPath, BACKSLASH_STR );
  strcat( pszDocPath, DEFAULT_PATTERN );

  // look for documents
  usRC = UtlFindFirst( pszDocPath, &hDirHandle, FILE_NORMAL,
                       &ResultBuf, sizeof( ResultBuf),
                       &usCount, 0L, FALSE );
  if ( usRC == NO_ERROR )
  {
      // close search file handle
      UtlFindClose( hDirHandle, FALSE );
      fEnable = (usCount > 0);
  }
  else
  {
      fEnable = FALSE;
  }
  return fEnable;
}

BOOL CFLLListView::IsRemovedDirNotEmpty()
{
  BOOL fEnable = FALSE;
  HTREEITEM hTreeItem = GetTreeCtrl().GetSelectedItem();
  TV_ITEM tvItem;
  /********************************************************************/
  /* get selected item and pass objectname as mp2                     */
  /********************************************************************/
  memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
  tvItem.mask = TVIF_TEXT | TVIF_PARAM;
  tvItem.pszText = pIda->CommArea.szBuffer;
  tvItem.cchTextMax = sizeof( pIda->CommArea.szBuffer );
  tvItem.hItem = hTreeItem;
  GetTreeCtrl().GetItem( &tvItem );

  // for subfolders we have to go up the chain...
  if ( LOWORD(tvItem.lParam) == SUBFOLDER_ITEM )
  {
     HTREEITEM hParent = hTreeItem;
     do
     {
       hParent = GetTreeCtrl().GetParentItem( hParent );
       memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
       tvItem.mask = TVIF_TEXT | TVIF_PARAM;
       tvItem.pszText = pIda->CommArea.szBuffer;
       tvItem.cchTextMax = sizeof(pIda->CommArea.szBuffer);
       tvItem.hItem = hParent;
       GetTreeCtrl().GetItem( &tvItem );
     } while ( LOWORD(tvItem.lParam) == SUBFOLDER_ITEM );
  } /* endif */


  PSZ pszObjName = (PSZ) pIda->CommArea.szBuffer;
  OBJNAME   szDocPath;
  strcpy( (PSZ)szDocPath, pszObjName );
  ANSITOOEM( (PSZ)szDocPath );
  SubFolNameToObjectName( (PSZ)szDocPath, pszObjName );


  /* Check if folder contains removed documents */
  PSZ       pszDocPath = (PSZ) &szDocPath;
  USHORT    usCount = 1;
  FILEFINDBUF ResultBuf;
  USHORT    usRC;
  HDIR      hDirHandle = HDIR_CREATE;

  // setup path to removed folder documents
  strcpy( pszDocPath, pszObjName );
  strcat( pszDocPath, BACKSLASH_STR );
  UtlQueryString( QST_REMOVEDDOCDIR, pszDocPath + strlen(pszDocPath), MAX_FILESPEC );
  strcat( pszDocPath, BACKSLASH_STR );
  UtlQueryString( QST_PROPDIR, pszDocPath + strlen(pszDocPath), MAX_FILESPEC );
  strcat( pszDocPath, BACKSLASH_STR );
  strcat( pszDocPath, DEFAULT_PATTERN );

  // look for removed documents
  usRC = UtlFindFirst( pszDocPath, &hDirHandle, FILE_NORMAL, &ResultBuf, sizeof( ResultBuf), &usCount, 0L, FALSE );
  if ( usRC == NO_ERROR )
  {
      // close search file handle
      UtlFindClose( hDirHandle, FALSE );
      fEnable = (usCount > 0);
  }
  else
  {
      fEnable = FALSE;
  }
  return fEnable;
}

void CFLLListView::OnRButtonDown(UINT nFlags, CPoint point)
{
  nFlags;
  RECT rc;
  CTreeCtrl& TreeCtrl=GetTreeCtrl();
  GetClientRect( (LPRECT)&rc);

  if (PtInRect ((LPRECT)&rc, point))
  {
    TVHITTESTINFO HitTestInfo;

    HitTestInfo.pt = point;
    HTREEITEM nHitItem = TreeCtrl.HitTest(&HitTestInfo);

    if ( nHitItem )
    {
      TV_ITEM tvItem;
      memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
      tvItem.mask = TVIF_PARAM;
      tvItem.hItem = nHitItem;
      GetTreeCtrl().GetItem( &tvItem );

      if ( (LOWORD(tvItem.lParam) == ROOT_ITEM) )
      {
        HandlePopupMenu( this->m_hWnd, point, ID_FLLTREE_ROOT_POPUP );
      }
      else
      {
        BOOL fIsSubFolder;

        if ( nHitItem && (HitTestInfo.flags & TVHT_ONITEM) )
        {
          if ( !(TreeCtrl.GetItemState(nHitItem, TVIS_SELECTED) & TVIS_SELECTED) )
          {
            TreeCtrl.Select( nHitItem, TVGN_CARET );
          } /* endif */
        } /* endif */

        // check if item is a subfolder
        {
          TV_ITEM tvItem;
          memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
          tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
          tvItem.hItem = nHitItem;
          TreeCtrl.GetItem( &tvItem );
          fIsSubFolder = (LOWORD(tvItem.lParam) == SUBFOLDER_ITEM);
        }

        if ( fIsSubFolder )
        {
          HandlePopupMenu( this->m_hWnd, point, ID_FLLTREE_SUBFOL_POPUP );
        }
        else
        {
          HandlePopupMenu( this->m_hWnd, point, pIda->CommArea.sPopupMenuID );
        } /* endif */
      } /* endif */
    }
    else
    {
      HandlePopupMenu( this->m_hWnd, point, pIda->CommArea.sNoSelPopupMenuID );
    } /* endif */
  } /* endif */

}


void CFLLListView::ShowNotification( LPNMHDR lpHdr, LRESULT* plResult )
{
  lpHdr; plResult;
}

void CFLLListView::OnKeyDown( LPNMHDR lpHdr, LRESULT* plResult )
{
  plResult;
  NMTVKEYDOWN * pKeyDown = (NMTVKEYDOWN *) lpHdr;
  switch ( pKeyDown->wVKey )
  {
	case VK_F1:
	{
       	  /**************************************************************/
		  /* process help ...                                           */
		  /**************************************************************/
		   CChildFrame  *pChildFrame = (CChildFrame *)(GetParent()->GetParent());
           CMainFrame   *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();

		  pFrameWnd->PostMessage( HM_HELPSUBITEM_NOT_FOUND, 0,
								  MP2FROM2SHORT( 0, 0 ) );
    }
	break;
    case VK_DELETE:
      /****************************************************************/
      /* delete the selected item                                     */
      /****************************************************************/
      {
        BOOL fIsSubFolder = FALSE;
        HTREEITEM hTreeItem = GetTreeCtrl().GetSelectedItem();

        // get selected item
        GetItemObjName( hTreeItem, pIda->CommArea.szBuffer, &fIsSubFolder );
        PSZ pszObjName = (PSZ) pIda->CommArea.szBuffer;                                                               //

        // call folder delete and message handling
        CFLLRightPaneView *pWnd = (CFLLRightPaneView *)(((CSplitterWnd *)GetParent())->GetPane(0,1));
        EqfRemoveObject( TWBFORCE, pWnd->m_hWnd);
        if (!EqfSend2Handler( FOLDERHANDLER, WM_EQF_DELETE, MP1FROMSHORT(m_hWnd),
                              MP2FROMP(pszObjName)))
        {
          GetTreeCtrl().DeleteItem( hTreeItem );
        }
        else
        {
          EqfRegisterObject( pszObjName, pWnd->m_hWnd, pWnd->GetIda()->CommArea.sListObjClass );
        } /* endif */

      }
      break;
    case VK_F10:
      { UINT nFlags = 0;
        RECT rc;
        CPoint point = /* 0 */ CPoint(0,20);
        BOOL   fShift;

        fShift = GetKeyState(VK_SHIFT) >> 15 ;
        if (fShift)
        {
           GetClientRect( (LPRECT)&rc);
	       if (PtInRect ((LPRECT)&rc, point))
               CFLLListView::OnRButtonDown( nFlags, point);
        }
      }
      break;
    case VK_TAB:
    case VK_BACKTAB:
      ((CSplitterWnd*)GetParent())->ActivateNext();
      break;
    default:
      break;
  } /* endswitch */
}

/**********************************************************************/
/* Determine if you are on an editable label - the root label is not  */
/* editable...                                                        */
/**********************************************************************/
VOID CFLLListView::OnRenameFolderBegin( LPNMHDR lpHdr, LRESULT* plResult )
{
  plResult; lpHdr;
  HTREEITEM hTreeItem = GetTreeCtrl().GetSelectedItem();
  TV_ITEM tvItem;
  memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
  tvItem.mask = TVIF_PARAM;
  tvItem.hItem = hTreeItem;
  GetTreeCtrl().GetItem( &tvItem );
  /********************************************************************/
  /* donot allow edit of ROOT_ITEM...                                 */
  /********************************************************************/
  if ( (LOWORD(tvItem.lParam) == ROOT_ITEM) )
  {
    TreeView_EndEditLabelNow( GetTreeCtrl().m_hWnd, TRUE );
  } /* endif */
  return;
}



/**********************************************************************/
/* Finish renaming a folder                                           */
/**********************************************************************/
void CFLLListView::OnRenameFolder( LPNMHDR lpHdr, LRESULT* plResult )
{
  plResult;
  BOOL fOK = TRUE;
  NMTVDISPINFO *pDispInfo = (NMTVDISPINFO *) lpHdr;
  TVITEM * ptvNewItem = &pDispInfo->item;
  if ( ptvNewItem->pszText )
  {
     CHAR szNewShortName[ MAX_FILESPEC ];
     CHAR szOldName[ MAX_LONGFILESPEC ];
     HTREEITEM hTreeItem = GetTreeCtrl().GetSelectedItem();
     TV_ITEM tvItem;
     memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
     tvItem.mask = TVIF_TEXT;
     tvItem.pszText = szOldName;
     tvItem.cchTextMax = sizeof( szOldName );
     tvItem.hItem = hTreeItem;
     GetTreeCtrl().GetItem( &tvItem );

     if ( (LOWORD(tvItem.lParam) == SUBFOLDER_ITEM) )
     {
       PSZ pszObjName = (PSZ) pIda->CommArea.szObjName;
       strcpy( pszObjName, ptvNewItem->pszText );
       ANSITOOEM( pszObjName );
       UtlStripBlanks( pszObjName );

       // check name syntax
       if ( (*pszObjName == EOS) || !UtlCheckLongName( pszObjName ) )
       {
         OEMTOANSI( pszObjName );
         UtlErrorHwnd( ERROR_INV_LONGNAME, MB_CANCEL, 1, &pszObjName, EQF_ERROR, m_hWnd );
         fOK = FALSE;
       } /* endif */

       // change subfolder name in subfolder properties
       if ( fOK )
       {
         CHAR chBuffer[ MAX_LONGFILESPEC ];
         TV_ITEM tvTestItem;
         HTREEITEM hParent = hTreeItem;

         // construct full subfolder name
         memset( (PVOID)&tvTestItem, 0, sizeof( tvTestItem ));

         // get name of current subfolder
         CString strFullName = "";
         memset( (PVOID)&tvTestItem, 0, sizeof( tvTestItem ));
         tvTestItem.mask = TVIF_TEXT;
         tvTestItem.pszText = chBuffer;
         tvTestItem.cchTextMax = sizeof( chBuffer );
         tvTestItem.hItem = hTreeItem;
         GetTreeCtrl().GetItem( &tvTestItem );
         strFullName = chBuffer;

         // prepend names of parent (sub)folders
         do
         {
           hParent = GetTreeCtrl().GetParentItem( hParent );

           memset( (PVOID)&tvTestItem, 0, sizeof( tvTestItem ));
           tvTestItem.mask = TVIF_TEXT;
           tvTestItem.pszText = chBuffer;
           tvTestItem.cchTextMax = sizeof( chBuffer );
           tvTestItem.hItem = hParent;
           GetTreeCtrl().GetItem( &tvTestItem );

           {
             CString strTemp = chBuffer;
             strTemp += "\\";
             strTemp += strFullName;
             strFullName = strTemp;
           }
         } while ( LOWORD(tvTestItem.lParam) == SUBFOLDER_ITEM );

         // check if new name is already in use
         if ( fOK  )
         {
           OBJNAME szObjName;
           strcpy ( (PSZ)chBuffer, strFullName );

           PSZ pszOldName = UtlSplitFnameFromPath( (PSZ)chBuffer );
           if ( stricmp( pszOldName, pszObjName ) != 0 )
           {
             strcat( (PSZ)chBuffer, BACKSLASH_STR );
             strcat( (PSZ)chBuffer, pszObjName );

             // use SubFolNameToObjectName function to check if such a subfolder exists
             if ( SubFolNameToObjectName( chBuffer, (PSZ)szObjName ) )
             {
               OEMTOANSI( pszObjName );
               UtlError( ERROR_SUBFOLDER_EXISTS, MB_CANCEL, 1, &pszObjName, EQF_ERROR );
               fOK = FALSE;
             } /* endif */
           } /* endif */
         } /* endif */

         // load the property file and change the subfolder name
         if ( fOK )
         {
           PPROPFOLDER pPropSubFolder = NULL;
           OBJNAME szObjName;
           strcpy ( (PSZ)chBuffer, strFullName );
           SubFolNameToObjectName( (PSZ)chBuffer, (PSZ)szObjName );
           ULONG ulBytes = 0;
           fOK = UtlLoadFileL( (PSZ)szObjName, (PVOID *)&pPropSubFolder, &ulBytes, FALSE, TRUE );
           if ( fOK )
           {
             strcpy( pPropSubFolder->szLongName, pszObjName );
             UtlWriteFileL( (PSZ)szObjName, ulBytes, pPropSubFolder, TRUE );
             UtlAlloc( (PVOID *)&pPropSubFolder, 0L, 0L, NOMSG );
           } /* endif */
         } /* endif */
       } /* endif */
     }
     else
     {
       // prepare folder rename
       PSZ pszObjName = (PSZ) pIda->CommArea.szObjName;
       strcpy( pszObjName, ptvNewItem->pszText );
       ANSITOOEM( pszObjName );
       UtlStripBlanks( pszObjName );

       // check object name syntax
       if ( (*pszObjName == EOS) || !UtlCheckLongName( pszObjName ) )
       {
         OEMTOANSI( pszObjName );
         UtlErrorHwnd( ERROR_INV_LONGNAME, MB_CANCEL, 1, &pszObjName, EQF_ERROR, m_hWnd );
         fOK = FALSE;
       } /* endif */

       // existence check
       if ( fOK )
       {
         BOOL fIsNew = FALSE;
         ObjLongToShortName( pszObjName, szNewShortName, FOLDER_OBJECT, &fIsNew );

         if ( !fIsNew )
         {
           OEMTOANSI( pszObjName );
           UtlErrorHwnd( ERROR_NEWFOLDER_EXISTS, MB_CANCEL, 1, &pszObjName,
                         EQF_ERROR, m_hWnd );
           fOK = FALSE;
         } /* endif */
       } /* endif */


       // let the handler for the object class do the actual renaming ...
       if ( fOK )
       {
         BOOL fIsNew = FALSE;
         CHAR szOldShortName[ MAX_FILESPEC ];
         ANSITOOEM( szOldName );
         ObjLongToShortName( szOldName, szOldShortName, FOLDER_OBJECT, &fIsNew );


         // setup old and new name parameter
         UtlMakeEQFPath( pszObjName, NULC, SYSTEM_PATH, NULL );
         sprintf( pIda->CommArea.szBuffer,"%s\\%s%s%c%s%c%s" ,
                  pszObjName, szOldShortName, EXT_FOLDER_MAIN,
                  X15, szNewShortName,
                  X15, ptvNewItem->pszText );
         ANSITOOEM( pIda->CommArea.szBuffer );
         fOK = (BOOL)EqfSend2Handler( FOLDERHANDLER,
                                      WM_EQF_PROCESSTASK,
                                      MP1FROMSHORT(RENAMEOBJECT_TASK),
                                      MP2FROMP(pIda->CommArea.szBuffer) );
       } /* endif */
     } /* endif */

     if ( fOK )
     {
       /***************************************************************/
       /* update the tree  -- use settings as above..                 */
       /***************************************************************/
       strcpy( szOldName, ptvNewItem->pszText );
       GetTreeCtrl().SetItem( &tvItem );
     } /* endif */
  }
  else
  {
    /******************************************************************/
    /* editing canceled                                               */
    /******************************************************************/
  } /* endif */
}


void CFLLListView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
   pResult;
   NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
   HTREEITEM hItem = pNMTreeView->itemNew.hItem;
   LONG lParam = pNMTreeView->itemNew.lParam;
   switch ( LOWORD( lParam ) )
   {
     case ROOT_ITEM:
       /**************************************************************/
       /* fill folder list                                           */
       /**************************************************************/
       ((CFLLRightPaneView *)((CSplitterWnd*)GetParent())->GetPane(0,1))->FillRightFolderListPane();
       break;
     case FOLDER_ITEM:
       /**************************************************************/
       /* open document list for folder                              */
       /**************************************************************/
       if ( hItem)
       {
         CHAR chBuffer[ MAX_LONGFILESPEC ];
         TV_ITEM tvItem;
         memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
         tvItem.mask = TVIF_TEXT;
         tvItem.pszText = chBuffer;
         tvItem.cchTextMax = sizeof( chBuffer );
         tvItem.hItem = hItem;
         GetTreeCtrl().GetItem( &tvItem );

         SetFolderTitle( chBuffer, GetParent() ) ;

         ((CFLLRightPaneView *)((CSplitterWnd*)GetParent())->GetPane(0,1))->FillRightFolderPane( chBuffer );
       } /* endif */
       break;
     case SUBFOLDER_ITEM:
       /**************************************************************/
       /* open document list for subfolder                           */
       /**************************************************************/
       if ( hItem)
       {
         CHAR chBuffer[ MAX_LONGFILESPEC ];
         TV_ITEM tvItem;
         HTREEITEM hParent = hItem;

         // construct full subfolder name
         memset( (PVOID)&tvItem, 0, sizeof( tvItem ));

         // get name of current subfolder
         CString strFullName = "";
         memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
         tvItem.mask = TVIF_TEXT;
         tvItem.pszText = chBuffer;
         tvItem.cchTextMax = sizeof( chBuffer );
         tvItem.hItem = hItem;
         GetTreeCtrl().GetItem( &tvItem );
         strFullName = chBuffer;

         // prepend names of parent (sub)folders
         do
         {
           hParent = GetTreeCtrl().GetParentItem( hParent );

           memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
           tvItem.mask = TVIF_TEXT;
           tvItem.pszText = chBuffer;
           tvItem.cchTextMax = sizeof( chBuffer );
           tvItem.hItem = hParent;
           GetTreeCtrl().GetItem( &tvItem );

           {
             CString strTemp = chBuffer;
             strTemp += "\\";
             strTemp += strFullName;
             strFullName = strTemp;
           }
         } while ( LOWORD(tvItem.lParam) == SUBFOLDER_ITEM );

         strcpy( chBuffer, strFullName );

         SetFolderTitle( chBuffer, GetParent() ) ;

         ((CFLLRightPaneView *)((CSplitterWnd*)GetParent())->GetPane(0,1))->FillRightFolderPane( chBuffer );
       } /* endif */
       break;
     default:
       break;
   }
}

//  Set selected folder name in title text of the folder list      
void  SetFolderTitle( CHAR * szFolder, CWnd * pWnd ) 
{
   // CFLLProcDoc : public CEQFDoc : public CDocument  GetTitle() and SetTitle() must be used to update the title

  CHAR      szCurTitle[512] ;       
  CHAR      szBaseTitle[512] ;       
  CString   strTitle;
  CMDIChildWnd  *pChild ;
  CDocument *pDocument ;

  pChild = (CMDIChildWnd *)pWnd->GetParent();
  pDocument = pChild->GetActiveDocument();
  strTitle = pDocument->GetTitle();
  strcpy( szCurTitle, strTitle ) ;
  LOADSTRING( NULLHANDLE, AfxGetResourceHandle(), SID_FLLI_TITLE, szBaseTitle );
  if ( ! strncmp( szCurTitle, szBaseTitle, strlen(szBaseTitle) ) ) {
     sprintf( &szCurTitle[strlen(szBaseTitle)], "     (Selected:  %s)", szFolder );
     pDocument->SetTitle(szCurTitle);
  }
}


// Get the object name for the given tree view item
int CFLLListView::GetItemObjName( HTREEITEM hItem, PSZ pszObjName, PBOOL pfIsSubFolder )
{
  BOOL fSubFolder = FALSE;
  TV_ITEM tvItem;
  CString strFullName;
  CString strOldName;

  // get current item
  memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
  tvItem.mask = TVIF_TEXT;
  tvItem.pszText = m_Buffer;
  tvItem.cchTextMax = sizeof( m_Buffer );
  tvItem.hItem = hItem;
  GetTreeCtrl().GetItem( &tvItem );

  // for subfolders we have to go up the chain...
  if ( LOWORD(tvItem.lParam) == SUBFOLDER_ITEM )
  {
     fSubFolder = TRUE;
     CString strTemp;
     strFullName = m_Buffer;
     HTREEITEM hParent = hItem;
     do
     {
       hParent = GetTreeCtrl().GetParentItem( hParent );
       memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
       tvItem.mask = TVIF_TEXT | TVIF_PARAM;
       tvItem.pszText = m_Buffer;
       tvItem.cchTextMax = sizeof(m_Buffer);
       tvItem.hItem = hParent;
       GetTreeCtrl().GetItem( &tvItem );
       strTemp = m_Buffer;
       strTemp += "\\";
       strTemp += strFullName;
       strFullName = strTemp;
     } while ( LOWORD(tvItem.lParam) == SUBFOLDER_ITEM );
     strcpy( m_Buffer, strFullName );
  } /* endif */
  strOldName = m_Buffer;
  ANSITOOEM( m_Buffer );
  if ( !SubFolNameToObjectName( m_Buffer, m_Buffer ) )
  {
    // try with old (= not converted) name
    strcpy( m_Buffer, strOldName );
    SubFolNameToObjectName( m_Buffer, m_Buffer );
  }
  if ( pszObjName ) strcpy( pszObjName, m_Buffer );
  if ( pfIsSubFolder ) *pfIsSubFolder = fSubFolder;
  return( strlen( m_Buffer ) );
}



/**********************************************************************/
/* Handle a WM_COMMAND message (menu selection) by sending a          */
/* WM_EQF_FLLTREE_COMMAND message to the folder handler               */
/**********************************************************************/
void CFLLListView::On_WM_COMMAND ( UINT mp1 )
{
  /********************************************************************/
  /* get selected item and pass objectname as mp2                     */
  /********************************************************************/
  BOOL fIsSubFolder = FALSE;
  HTREEITEM hTreeItem = GetTreeCtrl().GetSelectedItem();

  GetItemObjName( hTreeItem, pIda->CommArea.szBuffer, &fIsSubFolder );
  PSZ pszObjName = (PSZ) pIda->CommArea.szBuffer;                                                               //

  /********************************************************************/
  /* handle commands which update tree                                */
  /********************************************************************/
  CFLLRightPaneView *pWnd = (CFLLRightPaneView *)(((CSplitterWnd *)GetParent())->GetPane(0,1));
  switch ( SHORT1FROMMP1(mp1) )
  {
    case PID_FILE_MI_NEW:
     (*pIda->pfnCallBack)( &pIda->CommArea, m_hWnd,
                          WM_EQF_FLLTREE_COMMAND, mp1, MP2FROMP( pszObjName ) );
      break;
    case PID_FILE_MI_RENAME:
      (*pIda->pfnCallBack)( &pIda->CommArea, m_hWnd,
                            WM_EQF_FLLTREE_COMMAND, mp1, MP2FROMP( pszObjName ) );
      /****************************************************************/
      /* update tree                                                  */
      /****************************************************************/
      break;
    case PID_FILE_MI_DELETE:
      if ( fIsSubFolder )
      {
        SubFolderDelete( pszObjName, FALSE );
      }
      else
      {
        EqfRemoveObject( TWBFORCE, pWnd->m_hWnd);
        if (!EqfSend2Handler( FOLDERHANDLER, WM_EQF_DELETE, MP1FROMSHORT(m_hWnd),
                              MP2FROMP(pszObjName)))
        {
          GetTreeCtrl().DeleteItem( hTreeItem );
        }
        else
        {
          EqfRegisterObject( pszObjName, pWnd->m_hWnd, pWnd->GetIda()->CommArea.sListObjClass );
        } /* endif */
      } /* endif */
      break;

    case PID_FILE_MI_PROPERTIES:
      if ( fIsSubFolder )
      {
        FolSubFolderProperties( pszObjName );
      }
      else
      {

        CFLLRightPaneView *pWnd = (CFLLRightPaneView *)(((CSplitterWnd *)GetParent())->GetPane(0,1));
        EqfRemoveObject( TWBFORCE, pWnd->m_hWnd);
        EqfActivateInstance( m_hWnd, TRUE );
        (*pIda->pfnCallBack)( &pIda->CommArea, m_hWnd,
                               WM_EQF_FLLTREE_COMMAND, mp1, MP2FROMP( pszObjName ) );
        EqfRegisterObject( pszObjName, pWnd->m_hWnd, pWnd->GetIda()->CommArea.sListObjClass );

        // refresh document list of folder
        {
          CHAR szFolName[MAX_LONGFILESPEC];

          SubFolObjectNameToName( pszObjName, szFolName );
          ((CFLLRightPaneView *)((CSplitterWnd*)GetParent())->GetPane(0,1))->FillRightFolderPane( szFolName );
        }

      } /* endif */
      break;

    case PID_FILE_MI_HTMLPROPS:
      /****************************************************************/
      /* deregister folder object and run request ...                 */
      /****************************************************************/
      if ( fIsSubFolder )
      {
        BOOL  fOK = TRUE;
        fOK = SubFolderPropsToHtml( pszObjName, m_hWnd, TRUE);
      }
      else
      {
        CFLLRightPaneView *pWnd = (CFLLRightPaneView *)(((CSplitterWnd *)GetParent())->GetPane(0,1));
        EqfRemoveObject( TWBFORCE, pWnd->m_hWnd);
        EqfActivateInstance( m_hWnd, TRUE );
        (*pIda->pfnCallBack)( &pIda->CommArea, m_hWnd,
                               WM_EQF_FLLTREE_COMMAND, mp1, MP2FROMP( pszObjName ) );
        EqfRegisterObject( pszObjName, pWnd->m_hWnd, pWnd->GetIda()->CommArea.sListObjClass );

      }
      break;

    case PID_UTILS_MI_DELETEDOCS:
    case PID_UTILS_MI_RESTOREDOCS:
      {
        // same as default case but without removing folder object to allow updates of the document list 
        CFLLRightPaneView *pWnd = (CFLLRightPaneView *)(((CSplitterWnd *)GetParent())->GetPane(0,1));
        EqfActivateInstance( m_hWnd, TRUE );
        (*pIda->pfnCallBack)( &pIda->CommArea, m_hWnd, WM_EQF_FLLTREE_COMMAND, mp1, MP2FROMP( pszObjName ) );
      }
      break;

    default:
      {
        PGENLISTINSTIDA pTestIda = NULL;              // for test purposes only
        CFLLRightPaneView *pWnd = (CFLLRightPaneView *)(((CSplitterWnd *)GetParent())->GetPane(0,1));
        pTestIda = pWnd->GetIda();
        EqfRemoveObject( TWBFORCE, pWnd->m_hWnd);
        EqfActivateInstance( m_hWnd, TRUE );
        (*pIda->pfnCallBack)( &pIda->CommArea, m_hWnd,
                               WM_EQF_FLLTREE_COMMAND, mp1, MP2FROMP( pszObjName ) );
        pTestIda = pWnd->GetIda();
        EqfRegisterObject( pszObjName, pWnd->m_hWnd, pWnd->GetIda()->CommArea.sListObjClass );
      }
      break;
  } /* endswitch */
}

CFLLListView::CFLLListView()
{
  pIda = NULL;
}

CFLLListView::~CFLLListView()
{
}

BOOL CFLLListView::PreCreateWindow(CREATESTRUCT& cs)
{
  CEQFDApp * myApp = (CEQFDApp *)AfxGetApp();
  MDICREATESTRUCT *pMDI = myApp->GetMDIStruct();
  SetMDIStruct( pMDI );

  UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(GENLISTINSTIDA), ERROR_STORAGE );
  if ( pIda )
  {
      PLISTCREATEPARMS pCreateParms;
      pCreateParms = (PLISTCREATEPARMS)PVOIDFROMMP2(pMDI->lParam);
      pIda->pfnCallBack = pCreateParms->pfnCallBack;
      pIda->pvUserData  = pCreateParms->pvUserData;
      pIda->fRestart    = pCreateParms->fRestart;
      strcpy( (PSZ)pIda->CommArea.szObjName, pMDI->szTitle );
  }
  else
  {
      return FALSE;
  } /* endif */

  cs.style |= LVS_SHOWSELALWAYS | LVS_REPORT | LVS_SORTASCENDING;
  cs.lpszName = pMDI->szTitle;

  return CTreeView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CFLLListView Darstellung

void CFLLListView::OnDraw(CDC* pDC)
{
  pDC;
}


/////////////////////////////////////////////////////////////////////////////
// CFLLListView Drucken

BOOL CFLLListView::OnPreparePrinting(CPrintInfo* pInfo)
{
        return DoPreparePrinting(pInfo);
}

void CFLLListView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{

}

void CFLLListView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{

}

void CFLLListView::OnInitialUpdate()
{
  CTreeView::OnInitialUpdate();

  pIda->hFrame = GetParentFrame()->m_hWnd;       // store frame window
  pIda->CommArea.hwndLB = m_hWnd;                // store listbox

  ULONG ul = ::GetWindowLong( m_hWnd, GWL_ID );
  pIda->CommArea.sListboxID = (SHORT) ul;
  ANCHORWNDIDA( m_hWnd, pIda );


  // set the icon
  GetParentFrame()->SetIcon( pIda->CommArea.hIcon, FALSE );
  SetWindowText( (PSZ)pIda->CommArea.szTitle );


  PostMessage( WM_EQF_INITIALIZE, NULL, (LONG)pIda );

  //--- set process window postion and size ---
  WinSetWindowPos( pIda->hFrame, HWND_TOP,
          pIda->CommArea.swpSizePos.x,  pIda->CommArea.swpSizePos.y,
          pIda->CommArea.swpSizePos.cx,
          pIda->CommArea.swpSizePos.cy,
          (USHORT)(pIda->CommArea.swpSizePos.fs | EQF_SWP_ACTIVATE |
          EQF_SWP_SHOW | EQF_SWP_SIZE | EQF_SWP_MOVE));

  EqfActivateInstance( m_hWnd, TRUE );

  //cv
  m_TreeDropTarget.Register(this);
  //
}


/**********************************************************************/
/* Folder deleted -- update tree                                      */
/**********************************************************************/
LRESULT CFLLListView::OnEQFNDeletedName( WPARAM mp1, LPARAM mp2 )
{
  if ( (mp1 == clsFOLDER) && mp2 )
  {
    PSZ pszFolder = (PSZ)mp2;
    HTREEITEM hItem = GetTreeCtrl().GetRootItem();

    // find current name in childs of hItem
    hItem = GetTreeCtrl().GetNextItem( hItem, TVGN_CHILD );
    while ( hItem )
    {
      // if names match use this item else continue with next one
      if ( GetTreeCtrl().GetItemText( hItem ).Compare( pszFolder ) == 0 )
      {
        break;
      }
      else
      {
        hItem = GetTreeCtrl().GetNextItem( hItem, TVGN_NEXT );
      } /* endif */
    } /* endwhile */

    // if item found remove it
    if ( hItem )
    {
      GetTreeCtrl().DeleteItem( hItem );
    } /* endif */
  }
  else  if ( (mp1 == clsDOCUMENT) && mp2 )
  {
    // mp2 in EQFN_DELETEDNAME is the name of the folder/subfolder deleted!
    // search correct position

    HTREEITEM hItem = GetTreeCtrl().GetRootItem();

    PSZ pszNextPart = (PSZ)mp2;
    PSZ pszEnd = strchr( pszNextPart, BACKSLASH );
    do
    {
      // isolate next part of folder/subfolder chain
      if ( pszEnd ) *pszEnd = EOS;

      // find current name in childs of hItem
      hItem = GetTreeCtrl().GetNextItem( hItem, TVGN_CHILD );
      while ( hItem )
      {
        // if names match use this item else continue with next one
        if ( GetTreeCtrl().GetItemText( hItem ).Compare( pszNextPart ) == 0 )
        {
          break;
        }
        else
        {
          hItem = GetTreeCtrl().GetNextItem( hItem, TVGN_NEXT );
        } /* endif */
      } /* endwhile */

      // if item found go one level deeper if necessary
      if ( hItem && pszEnd )
      {
        *pszEnd = BACKSLASH;
        pszNextPart = pszEnd + 1;
        pszEnd = strchr( pszNextPart, BACKSLASH );
      }
      else
      {
        pszNextPart = NULL;
      } /* endif */
    } while ( hItem && pszNextPart );

    if ( hItem )
    {
       GetTreeCtrl().DeleteItem( hItem );
    } /* endif */
  } /* endif */
  return 0;
}

/**********************************************************************/
/* Folder deleted -- update folders listed in right pane view         */
/**********************************************************************/
LRESULT CFLLListView::OnEQFNDeleted( WPARAM mp1, LPARAM mp2 )
{
  if ( (mp1 == clsFOLDER) && mp2 )
  {
    CFLLRightPaneView *pWnd = (CFLLRightPaneView *)(((CSplitterWnd *)GetParent())->GetPane(0,1));
    if ( pWnd->pIda->CommArea.sListObjClass == clsFOLDERLIST )
    {
      pWnd->DeleteRow( (PSZ)mp2 );
    } /* endif */
  } /* endif */
  return 0;
}



HTREEITEM CFLLListView::GetItemByName(HTREEITEM hItem, PSZ pszItemName)
{
    // If hItem is NULL, start search from root item.
    if (hItem == NULL)
                hItem = GetTreeCtrl().GetNextItem( TVGN_ROOT, 0 );

    while (hItem != NULL)
    {
        CHAR  chName[MAX_LONGFILESPEC+1];
        TV_ITEM item;

        item.hItem = hItem;
        item.mask = TVIF_TEXT | TVIF_CHILDREN;
        item.pszText = chName;
        item.cchTextMax = sizeof( chName )-1;
        GetTreeCtrl().GetItem( &item );

        // Did we find it?
        if (stricmp(chName, pszItemName) == 0)
            return hItem;

        // Check whether we have child items.
        if (item.cChildren)
        {
            // Recursively traverse child items.
            HTREEITEM hItemFound, hItemChild;

            hItemChild = GetTreeCtrl().GetNextItem(hItem, TVGN_CHILD);
            hItemFound = GetItemByName(hItemChild, pszItemName);

            // Did we find it?
            if (hItemFound != NULL)
                return hItemFound;
        }

        // Go to next sibling item.
        hItem = GetTreeCtrl().GetNextItem(hItem, TVGN_NEXT);
    }

    // Not found.
    return NULL;
}




/**********************************************************************/
/* Folder new     -- update tree                                      */
/**********************************************************************/
LRESULT CFLLListView::OnEQFNNew( WPARAM mp1, LPARAM mp2 )
{
  if ( mp1 == clsFOLDER )
  {
    HTREEITEM hTreeItemRoot = GetTreeCtrl().GetRootItem();
    if (hTreeItemRoot && mp2)
    {
      /****************************************************************/
      /* insert folder name                                           */
      /****************************************************************/
      CHAR  chName[MAX_LONGFILESPEC];
      CHAR  chSelName[MAX_LONGFILESPEC];
      HTREEITEM hTreeItem;
      TV_INSERTSTRUCT TreeCtrlItem;
      TreeCtrlItem.hInsertAfter = TVI_SORT;
      TreeCtrlItem.hParent = hTreeItemRoot;
      TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
      TreeCtrlItem.item.iSelectedImage = 3;
      TreeCtrlItem.item.iImage = 2;

      SubFolObjectNameToName( (PSZ)mp2, chName );
      OEMTOANSI( chName );

      /****************************************************************/
      /* check if this item is already in the list -- skip it, we got */
      /* notification for item already inserted...                    */
      /****************************************************************/
      chSelName[0] = EOS;
      hTreeItem = GetTreeCtrl().GetSelectedItem();
      if ( hTreeItem )
      {
        TV_ITEM tvItem;
        memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
        tvItem.mask = TVIF_TEXT | TVIF_PARAM;
        tvItem.pszText = chSelName;
        tvItem.cchTextMax = sizeof( chSelName );
        tvItem.hItem = hTreeItem;
        GetTreeCtrl().GetItem( &tvItem );
      } /* endif */
      if ( chSelName[0] && strcmp( chSelName, chName ) == 0 )
      {
        /**************************************************************/
        /* skip already inserted item                                 */
        /**************************************************************/
      }
      else
      {
        TreeCtrlItem.item.pszText = (PSZ) &chName[0];
        TreeCtrlItem.item.lParam = MAKELONG(FOLDER_ITEM, 0);
        hTreeItem = GetTreeCtrl().InsertItem(&TreeCtrlItem);

        AddSubFolders( hTreeItem, (PSZ)mp2 );

        /****************************************************************/
        /* select newly inserted item                                   */
        /****************************************************************/
        GetTreeCtrl().SelectItem( hTreeItem );
        ((CFLLRightPaneView *)((CSplitterWnd*)GetParent())->GetPane(0,1))->FillRightFolderPane( chName );
      } /* endif */

    } /* endif */
  }
  else if ( mp1 == clsDOCUMENT )
  {
    HTREEITEM hTreeItemRoot = GetTreeCtrl().GetRootItem();
    if (hTreeItemRoot && mp2)
    {
      BOOL fSubFolder = FolIsSubFolderObject( (PSZ)mp2 );
      if ( fSubFolder )
      {
        CHAR  chName[MAX_LONGFILESPEC];
        CHAR  chSelName[MAX_LONGFILESPEC];
        HTREEITEM hItem;

        SubFolObjectNameToName( (PSZ)mp2, chName );
        OEMTOANSI( chName );

        // search correct insert position
        hItem = GetTreeCtrl().GetRootItem();

        PSZ pszNextPart = chName;
        PSZ pszEnd = strchr( pszNextPart, BACKSLASH );
        do
        {
          // isolate next part of folder/subfolder chain
          if ( pszEnd ) *pszEnd = EOS;

          // find current name in childs of hItem
          hItem = GetTreeCtrl().GetNextItem( hItem, TVGN_CHILD );
          while ( hItem )
          {
            // get item
            TV_ITEM tvItem;
            memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
            tvItem.mask = TVIF_TEXT | TVIF_PARAM;
            tvItem.pszText = chSelName;
            tvItem.cchTextMax = sizeof( chSelName );
            tvItem.hItem = hItem;
            GetTreeCtrl().GetItem( &tvItem );

            // if names match use this item else continue with next one
            if ( strcmp( chSelName, pszNextPart ) == 0 )
            {
              break;
            }
            else
            {
              hItem = GetTreeCtrl().GetNextItem( hItem, TVGN_NEXT );
            } /* endif */
          } /* endwhile */

          // if item found go one level deeper if necessary
          if ( hItem && pszEnd )
          {
            pszNextPart = pszEnd + 1;
            pszEnd = strchr( pszNextPart, BACKSLASH );
          } /* endif */
        } while ( hItem && pszEnd );

        // insert new subfolder if we have the correct insert position
        if ( hItem )
        {
          TV_INSERTSTRUCT TreeCtrlItem;
          TreeCtrlItem.hInsertAfter = TVI_SORT;
          TreeCtrlItem.hParent = hItem;
          TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
          TreeCtrlItem.item.iSelectedImage = 3;
          TreeCtrlItem.item.iImage = 2;
          TreeCtrlItem.item.lParam = MAKELONG( SUBFOLDER_ITEM, 0 );
          TreeCtrlItem.item.pszText = pszNextPart;
          TreeCtrlItem.item.cchTextMax = strlen(pszNextPart) + 1;

          hItem = GetTreeCtrl().InsertItem(&TreeCtrlItem);
          GetTreeCtrl().Invalidate();
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */
  return 0;
}

LRESULT CFLLListView::OnDriveChanged(WPARAM mp1, LPARAM mp2)
{
	mp1; mp2;
    return OnFillFolderTree(MP1FROMSHORT(0),MP2FROMP(pIda));
}

/**********************************************************************/
/* Build up the new Folder/SubFolder tree                             */
/* lParam items in the Tree are set as follows:                       */
/*   ROOT_ITEM                                                        */
/*   FOLDER_ITEM                                                      */
/*   SUBFOLDER_ITEM                                                   */
/**********************************************************************/

LRESULT CFLLListView::OnFillFolderTree( WPARAM mp1, LPARAM mp2 )
{
  mp1;
  if ( mp2 )
  {
    HWND hwndLB = NULLHANDLE;
    PSZ  pBuffer;
    /********************************************************************/
    /* mp2 contains the ida for the folder list ...                     */
    /********************************************************************/
    pIda = (PGENLISTINSTIDA) mp2;
    pBuffer = pIda->CommArea.szBuffer;
    /********************************************************************/
    /* fill in the images                                               */
    /********************************************************************/
    if ( m_ImageList.GetSafeHandle() )
    {
       m_ImageList.DeleteImageList();
    } /* endif */
    GetTreeCtrl().DeleteItem( TVI_ROOT );

    m_ImageList.Create(16, 16, TRUE, 1, 4 );
    // root
    m_ImageList.Add( LoadIcon( AfxGetResourceHandle(),
                               MAKEINTRESOURCE( TWB_ICON ) )  );
    m_ImageList.Add( LoadIcon( AfxGetResourceHandle(),
                               MAKEINTRESOURCE( TWB_ICON ) )  );
    // folder
    m_ImageList.Add( LoadIcon( AfxGetResourceHandle(),
                               MAKEINTRESOURCE( FLL_ICON ) )  );
    m_ImageList.Add( LoadIcon( AfxGetResourceHandle(),
                               MAKEINTRESOURCE( FLL_OPENED_ICON ) )  );

    TreeView_SetImageList( GetTreeCtrl(), m_ImageList, TVSIL_NORMAL );

    /********************************************************************/
    /* Set styles for display                                           */
    /********************************************************************/
    LONG l = GetWindowLong( GetTreeCtrl().m_hWnd, GWL_STYLE );
    l |= TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_LINESATROOT |
         TVS_EDITLABELS;
    SetWindowLong( GetTreeCtrl().m_hWnd, GWL_STYLE, l );

    /********************************************************************/
    /* Fill our listbox with the names of all folders                   */
    /********************************************************************/
    USHORT usNum, usItemCount;
    hwndLB = WinCreateWindow( m_hWnd, WC_LISTBOX, "", 0L, 0, 0, 0, 0,
                              m_hWnd, HWND_TOP, 1, NULL, NULL );

    FllLoadFolderNames( hwndLB, pBuffer );
    /********************************************************************/
    /* set new popup menu                                               */
    /********************************************************************/
    pIda->CommArea.sPopupMenuID = ID_FLLTREE_POPUP;

    usItemCount = QUERYITEMCOUNTHWND( hwndLB );

    /********************************************************************/
    /* insert Root element                                              */
    /********************************************************************/
    TV_INSERTSTRUCT TreeCtrlItem;
    TreeCtrlItem.hInsertAfter = TVI_SORT;
    TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    TreeCtrlItem.item.iSelectedImage = 1;
    TreeCtrlItem.item.iImage = 0;
    TreeCtrlItem.hParent = TVI_ROOT;
    char chRootText[40];
    LOADSTRING( NULLHANDLE, AfxGetResourceHandle(), SID_FLLI_ROOT, chRootText );
    TreeCtrlItem.item.pszText = chRootText;

    TreeCtrlItem.item.lParam = MAKELONG(ROOT_ITEM, 0);
    HTREEITEM hTreeItemRoot = GetTreeCtrl().InsertItem(&TreeCtrlItem);
    HTREEITEM hTreeLastUsedFolder = NULL;
    HTREEITEM hTreeItem1;
    CString strValue = AfxGetApp()->GetProfileString( EQFFOLDER, "FolderRecent", NULL);

    TreeCtrlItem.item.iSelectedImage = 3;
    TreeCtrlItem.item.iImage = 2;
    for ( usNum=0; usNum < usItemCount; usNum ++ )
    {
      QUERYITEMTEXTHWND( hwndLB, usNum, pBuffer );

      TreeCtrlItem.hParent = hTreeItemRoot;
      TreeCtrlItem.item.pszText = UtlParseX15 (pBuffer, 1);
      TreeCtrlItem.item.lParam = MAKELONG(FOLDER_ITEM, 0);
      hTreeItem1 = GetTreeCtrl().InsertItem(&TreeCtrlItem);

      // insert any subfolders of current folder
      UtlParseX15( pBuffer, 0 );
      AddSubFolders( hTreeItem1, pBuffer );

      if ( !strValue.IsEmpty() && strValue.Compare(TreeCtrlItem.item.pszText) == 0 )
      {
        hTreeLastUsedFolder = hTreeItem1;
      } /* endif */
     } /* endfor */
     /*******************************************************************/
     /* expand and select root or the last used folder                  */
     /*******************************************************************/
     HTREEITEM hTreeItem = GetTreeCtrl().GetRootItem();
     if (hTreeItem)
     {
       GetTreeCtrl().Expand( hTreeItem, TVE_EXPAND );
       if ( !hTreeLastUsedFolder )
       {
         GetTreeCtrl().SelectItem( hTreeItem );
         ((CFLLRightPaneView *)((CSplitterWnd*)GetParent())->GetPane(0,1))->FillRightFolderListPane();
       }
       else
       {
         GetTreeCtrl().SelectItem( hTreeLastUsedFolder );
         ((CFLLRightPaneView *)((CSplitterWnd*)GetParent())->GetPane(0,1))->FillRightFolderPane( (char *)LPCTSTR( strValue ) );
       } /* endif */
     }

    if ( hwndLB )
       ::WinDestroyWindow( hwndLB );
  } /* endif */
  return 0;
}


/**********************************************************************/
/* try to select specified folder and open it                         */
/**********************************************************************/
void CFLLListView::OpenFolder( PSZ pFolder )
{
#ifdef OLDCODE
  HTREEITEM hTreeItem = GetTreeCtrl().GetRootItem();
  /********************************************************************/
  /* find first item                                                  */
  /********************************************************************/
  if (hTreeItem)
  {
    hTreeItem = GetTreeCtrl().GetNextItem( hTreeItem, TVGN_CHILD );
  } /* endif */

  while ( hTreeItem )
  {
    if ( GetTreeCtrl().GetItemText( hTreeItem ).Compare( pFolder ) == 0 )
    {
      GetTreeCtrl().SelectItem( hTreeItem );
      ((CFLLRightPaneView *)((CSplitterWnd*)GetParent())->GetPane(0,1))->FillRightFolderPane( pFolder );
      break;
    }
    else
    {
      hTreeItem = GetTreeCtrl().GetNextSiblingItem( hTreeItem );
    } /* endif */
  } /* endwhile */
#else

    // search correct position
    HTREEITEM hItem = GetTreeCtrl().GetRootItem();

    PSZ pszNextPart = pFolder;
    PSZ pszEnd = strchr( pszNextPart, BACKSLASH );
    do
    {
      // isolate next part of folder/subfolder chain
      if ( pszEnd ) *pszEnd = EOS;

      // find current name in childs of hItem
      hItem = GetTreeCtrl().GetNextItem( hItem, TVGN_CHILD );
      while ( hItem )
      {
        // if names match use this item else continue with next one
        if ( GetTreeCtrl().GetItemText( hItem ).Compare( pszNextPart ) == 0 )
        {
          break;
        }
        else
        {
          hItem = GetTreeCtrl().GetNextItem( hItem, TVGN_NEXT );
        } /* endif */
      } /* endwhile */

      // if item found go one level deeper if necessary
      if ( hItem && pszEnd )
      {
        *pszEnd = BACKSLASH;
        pszNextPart = pszEnd + 1;
        pszEnd = strchr( pszNextPart, BACKSLASH );
      }
      else
      {
        pszNextPart = NULL;
      } /* endif */
    } while ( hItem && pszNextPart );

    if ( hItem )
    {
      GetTreeCtrl().SelectItem( hItem );
      ((CFLLRightPaneView *)((CSplitterWnd*)GetParent())->GetPane(0,1))->FillRightFolderPane( pFolder );
    } /* endif */


#endif

}

#define UtlMenuDisableItem( sItemID )  \
    SETAABITEM( ::GetMenu((HWND)UtlQueryULong( QL_TWBFRAME )), sItemID, FALSE)
/**********************************************************************/
/* let parent view do the menu preparation                            */
/**********************************************************************/
LRESULT CFLLListView::OnInitMenuPopup( WPARAM mp1, LPARAM mp2 )
{
  LRESULT mResult = FALSE;
  /********************************************************************/
  /* disable selection if rootitem selected                           */
  /********************************************************************/
  HTREEITEM hTreeItem = GetTreeCtrl().GetSelectedItem();
  TV_ITEM tvItem;
  memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
  tvItem.mask = TVIF_PARAM;
  tvItem.hItem = hTreeItem;
  GetTreeCtrl().GetItem( &tvItem );
  /********************************************************************/
  /* enable selection for all active objects                          */
  /********************************************************************/
  if ( (LOWORD(tvItem.lParam) == ROOT_ITEM) )
  {
    // root item -- nothing should be enabled
    UtlMenuDisableItem( PID_FILE_MI_NEW );
    UtlMenuDisableItem( PID_FILE_MI_DELETE );
    UtlMenuDisableItem( PID_FILE_MI_IMPORT );
    UtlMenuDisableItem( PID_FILE_MI_SYSPROP);
    UtlMenuDisableItem( PID_FILE_MI_EXIT);
    UtlMenuDisableItem( PID_UTILS_MI_MT );
    UtlMenuDisableItem( PID_FILE_MI_PRINTLIST );
    UtlMenuDisableItem( PID_VIEW_MI_NAMES );
    UtlMenuDisableItem( PID_VIEW_MI_DETAILSDLG );
    UtlMenuDisableItem( PID_VIEW_MI_DETAILS );
    UtlMenuDisableItem( PID_VIEW_MI_TOOLBAR );
    UtlMenuDisableItem( PID_UTILS_MI_TAGTABLE );
    UtlMenuDisableItem( PID_UTILS_MI_LNGUPDATE );
    UtlMenuDisableItem( PID_UTILS_MI_PLGINMGR );  // new start
    UtlMenuDisableItem( PID_UTILS_MI_ATOVERUP );  // new end
    UtlMenuDisableItem( PID_UTILS_MI_DRIVES );
    UtlMenuDisableItem( PID_TERMLISTS_POPUP );
    UtlMenuDisableItem( PID_UTILS_MI_EXCLUSION );
    UtlMenuDisableItem( PID_UTILS_MI_NEWTERMS );
    UtlMenuDisableItem( PID_UTILS_MI_FOUNDTERMS );
    UtlMenuDisableItem( PID_UTILS_MI_ABBR );
    UtlMenuDisableItem( PID_FILE_MI_RENAME    );
    UtlMenuDisableItem( PID_UTILS_MI_REPORT );
    UtlMenuDisableItem( PID_UTILS_MI_ARCHTM );
    UtlMenuDisableItem( PID_UTILS_MI_EXPORTSEGS );
    UtlMenuDisableItem( PID_UTILS_MI_DELETEDOCS );
    UtlMenuDisableItem( PID_UTILS_MI_RESTOREDOCS );

    /********************************************************************/
    /* the windows pulldown is the 5th menu item                        */
    /********************************************************************/
    UINT         usID =  mp2;
    CChildFrame  *pChildFrame = (CChildFrame *)(GetParent()->GetParent());
    CMainFrame   *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();
    BOOL         fMaximized;
    CMDIChildWnd *pChildWnd = pFrameWnd->MDIGetActive( &fMaximized);
    pChildWnd;		// avoid compiler warnings
    usID = ( fMaximized ) ? usID -1 : usID;

    if ( usID == PID_TWBM_SM_WINDOWS)
    {
      pChildFrame->OnWindowsPopup( mp1, mp2 );
    } /* endif */
  }
  else
  {
    /********************************************************************/
    /* the windows pulldown is the 5th menu item                        */
    /********************************************************************/
    LRESULT      mResult = FALSE;
    UINT         usID =  mp2;
    CChildFrame  *pChildFrame = (CChildFrame *)(GetParent()->GetParent());
    CMainFrame   *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();
    BOOL         fMaximized;
    CMDIChildWnd *pChildWnd = pFrameWnd->MDIGetActive( &fMaximized);
    pChildWnd;

    usID = ( fMaximized ) ? usID -1 : usID;

    switch ( usID )
    {
      case 0xFFFF:
          /****************************************************************/
          /* we were at the first sub-menu in maximized mode -- do nothing*/
          /* We have to do this here, because in MDI IsZoomed is not      */
          /* working correctly....                                        */
          /****************************************************************/
          break;
      case PID_TWBM_SM_HELP:
          // do nothing; leave help and windows items active ...

          // GQ 2015/10/13: disable some of the help items until out help system is working again
          UtlMenuDisableItem( PID_HELP_FOR_HELP );
          UtlMenuDisableItem( PID_HELP_MI_INDEX );


          break;
      case PID_TWBM_SM_WINDOWS:
          // use base function to build the windows pulldown ...
          mResult = pChildFrame->OnWindowsPopup( mp1, mp2 );
          break;
      case PID_TWBM_SM_VIEW:
          pFrameWnd->OnCheckToolBar();
          mResult = pIda->pfnCallBack( &pIda->CommArea ,
                                       m_hWnd, WM_EQF_INITMENU, mp2, 0 );
          break;

      default:
          mResult = pIda->pfnCallBack( &pIda->CommArea ,
                                       m_hWnd, WM_EQF_INITMENU, mp2, 0 );
          //--- ensure that some of the items are enabled anyway ---
          UtlMenuEnableItem( PID_UTILS_MI_TAGTABLE );
          UtlMenuEnableItem( PID_UTILS_MI_LNGUPDATE );
          UtlMenuEnableItem( PID_UTILS_MI_PLGINMGR );  // new start
          UtlMenuEnableItem( PID_UTILS_MI_ATOVERUP );  // new end
          UtlMenuEnableItem( PID_UTILS_MI_DRIVES );
          UtlMenuEnableItem( PID_TERMLISTS_POPUP );
          UtlMenuEnableItem( PID_UTILS_MI_EXCLUSION );
          UtlMenuEnableItem( PID_UTILS_MI_NEWTERMS );
          UtlMenuEnableItem( PID_UTILS_MI_FOUNDTERMS );
          UtlMenuEnableItem( PID_UTILS_MI_ABBR );
          UtlMenuEnableItem( PID_FILE_MI_SYSPROP );
          UtlMenuDisableItem( PID_FILE_MI_EXIT);
          UtlMenuEnableItem( PID_UTILS_MI_MT );
          break;
    } /* endswitch */

    /********************************************************************/
    /* take care of specialities of the tree                            */
    /********************************************************************/

    // check if selected item is a subfolder
    TV_ITEM tvItem;
    memset( (PVOID)&tvItem, 0, sizeof( tvItem ));
    tvItem.mask = TVIF_PARAM;
    tvItem.hItem = hTreeItem;
    GetTreeCtrl().GetItem( &tvItem );
    BOOL fSubFolder = LOWORD(tvItem.lParam) == SUBFOLDER_ITEM;


    if ( FolderNotEmpty() )
    {
      UtlMenuEnableItem( PID_UTILS_MI_REPORT );
      UtlMenuEnableItem( PID_FILE_MI_ANALYZE );
      UtlMenuEnableItem( PID_UTILS_MI_COUNT );
      UtlMenuEnableItem( PID_UTILS_MI_ARCHTM );
      UtlMenuEnableItem( PID_UTILS_MI_EXPORTSEGS );
      UtlMenuEnableItem( PID_UTILS_MI_DELETEDOCS );
      UtlMenuEnableItem( PID_FILE_MI_FINDCHANGE );
      UtlMenuEnableItem( PID_FILE_MI_FUZZYSEARCH );
      UtlMenuEnableItem( PID_UTILS_MI_MT     );
    } /* endif */

    if ( IsRemovedDirNotEmpty() )
    {
      UtlMenuEnableItem( PID_UTILS_MI_RESTOREDOCS );
    } /* endif */

    UtlMenuEnableItem( PID_FILE_MI_PROPERTIES );
    UtlMenuEnableItem( PID_FILE_MI_HTMLPROPS );
    if ( fSubFolder )
    {
      UtlMenuDisableItem( PID_FILE_MI_EXPORT    );
      UtlMenuDisableItem( PID_FILE_MI_IMPORT    );
      UtlMenuDisableItem( PID_FILE_MI_RENAME    );
    }
    else
    {
      UtlMenuEnableItem( PID_FILE_MI_EXPORT    );
      UtlMenuEnableItem( PID_FILE_MI_RENAME    );
    } /* endif */
    UtlMenuDisableItem( PID_FILE_MI_PRINTLIST );
    UtlMenuDisableItem( PID_VIEW_MI_NAMES );
    UtlMenuDisableItem( PID_VIEW_MI_DETAILSDLG );
    UtlMenuDisableItem( PID_VIEW_MI_DETAILS );
  }
  return mResult;
}


/**********************************************************************/
/* add all subfolders of given folder to tree control                 */
/**********************************************************************/

typedef struct _SUBFOLTABLE
{
  CHAR      szName[MAX_LONGFILESPEC];// name of subfolder
  ULONG     ulID;                    // ID of subfolder
  ULONG     ulParentID;              // ID of parent subfolder
  BOOL      fInserted;               // subfolder has been inserted flag
  HTREEITEM hItem;                   // tree item handle of this item
} SUBFOLTABLE, *PSUBFOLTABLE;

int iSubFolCompare( const void *pElem1, const void *pElem2 )
{
  return( (int)((PSUBFOLTABLE)pElem1)->ulParentID - (int)((PSUBFOLTABLE)pElem2)->ulParentID );
};


int CFLLListView::AddSubFolders
(
  HTREEITEM hFolItem,                  // item handle of parent folder
  PSZ   pszFolObjName                  // object name of parent folder
)
{

  PSUBFOLTABLE pSubFolTable = NULL;
  HWND hwndLB = NULLHANDLE;
  char *pszBuffer = new char[2048 ];

  // Fill our listbox with object names of all subfolder of current folder
  hwndLB = WinCreateWindow( m_hWnd, WC_LISTBOX, "", 0L, 0, 0, 0, 0,
                            m_hWnd, HWND_TOP, 1, NULL, NULL );

  // load all subfolder names with parentship info
  USHORT usSubFolders = FolLoadSubFolderNames( pszFolObjName, hwndLB,
                                               LOADSUBFOLNAMES_PARENTINFO |
                                               LOADSUBFOLNAMES_ALLSUBFOLDERS,
                                               pszBuffer );

  // allocate our table for subfolder parentship information
  if ( usSubFolders )
  {
    UtlAlloc( (PVOID *)&pSubFolTable, 0L, sizeof(SUBFOLTABLE)*usSubFolders, ERROR_STORAGE );
  } /* endif */

  // fill parentship table
  if ( pSubFolTable )
  {
    // copy all listbox items to table
    for ( USHORT usNum=0; usNum < usSubFolders; usNum ++ )
    {
      QUERYITEMTEXTHWND( hwndLB, usNum, pszBuffer );

      strcpy( pSubFolTable[usNum].szName, UtlParseX15( pszBuffer, 1 ) );
      pSubFolTable[usNum].ulID = atol( UtlParseX15( pszBuffer, 2 ) );
      pSubFolTable[usNum].ulParentID = atol( UtlParseX15( pszBuffer, 3 ) );
    } /* endfor */

    // sort all items
    qsort( pSubFolTable, usSubFolders, sizeof(SUBFOLTABLE), iSubFolCompare );
  } /* endif */

  // add all subfolders to tree control
  if ( pSubFolTable )
  {
    BOOL      fAllInserted = FALSE;    // all subfolders have been inserted flag

    do
    {
      TV_INSERTSTRUCT TreeCtrlItem;

      fAllInserted = TRUE;          // assume all subfolders are inserted

      for ( USHORT usSubFol = 0; usSubFol < usSubFolders; usSubFol++ )
      {
        if ( !pSubFolTable[usSubFol].fInserted )
        {
          TreeCtrlItem.hParent = NULLHANDLE;

          // search parent handle in subfolder table
          if ( pSubFolTable[usSubFol].ulParentID == 0 )
          {
            // use main folder handle
            TreeCtrlItem.hParent = hFolItem;
          }
          else
          {
            // look for parent in table
            USHORT usJ;
            for ( usJ = 0; usJ < usSubFolders; usJ++ )
            {
              if ( pSubFolTable[usJ].ulID == pSubFolTable[usSubFol].ulParentID )
              {
                if ( pSubFolTable[usJ].fInserted )
                {
                  // use handle of inserted item
                  TreeCtrlItem.hParent = pSubFolTable[usJ].hItem;
                }
                else
                {
                  // we have to retry later on ...
                  fAllInserted = FALSE;
                } /* endif */
                break;
              } /* endif */
              if ( usJ == usSubFolders )
              {
                // parent subfolder does not exist, attach subfolder to main folder
                TreeCtrlItem.hParent = hFolItem;
              } /* endif */
            } /* endif */
          } /* endif */

          // insert item
          if ( TreeCtrlItem.hParent != NULLHANDLE )
          {
            TreeCtrlItem.hInsertAfter = TVI_SORT;
            TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
            TreeCtrlItem.item.iSelectedImage = 3;
            TreeCtrlItem.item.iImage = 2;

            // search parent handle in subfolder table
            TreeCtrlItem.item.pszText = pSubFolTable[usSubFol].szName;
            TreeCtrlItem.item.lParam = MAKELONG( SUBFOLDER_ITEM, 0 );
            pSubFolTable[usSubFol].hItem = GetTreeCtrl().InsertItem(&TreeCtrlItem);
            pSubFolTable[usSubFol].fInserted = TRUE;
          } /* endif */
        } /* endif */
      } /* endfor */
    } while ( !fAllInserted );

  } /* endif */


  if ( hwndLB ) ::WinDestroyWindow( hwndLB );

  delete pszBuffer;
  return( usSubFolders );
}

/////////////////////////////////////////////////////////////////////////////
// CFLLListView Diagnose

#ifdef _DEBUG
void CFLLListView::AssertValid() const
{
        CTreeView::AssertValid();
}

void CFLLListView::Dump(CDumpContext& dc) const
{
        CTreeView::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFLLListView




/////////////////////////////////////////////////////////////////////////////
// CFLLRightPaneView

IMPLEMENT_DYNCREATE(CFLLRightPaneView, CGenListView)

BEGIN_MESSAGE_MAP(CFLLRightPaneView, CGenListView)
        //{{AFX_MSG_MAP(CFLLRightPaneView)
        ON_WM_LBUTTONDBLCLK()
//@@        ON_MESSAGE( WM_INITMENU,      OnInitMenu )
        ON_MESSAGE( WM_INITMENUPOPUP, OnInitMenuPopup )
        ON_MESSAGE( WM_EQF_OPEN,        On_WM_EQF_OPEN)
        ON_MESSAGE( WM_EQF_SHOWHTML,   CGenListView::On_WM_EQF_SHOWHTML )
        ON_MESSAGE( WM_EQF_MDIACTIVATE, On_WM_EQF_MDIACTIVATE )
        ON_COMMAND(PID_VIEW_MI_SORT,           CListViewEx::OnSortDlg)      // use base ones
        ON_COMMAND(PID_VIEW_MI_ALL,            CListViewEx::OnAll)          // use base ones
        ON_COMMAND(PID_VIEW_MI_SOME,           CListViewEx::OnFilterDlg)    // use base ones
        ON_COMMAND(PID_VIEW_MI_DETAILSDLG,     OnDetailsDlg)
        ON_COMMAND(PID_VIEW_MI_NAMES,          CListViewEx::OnNamesView)    // use base ones
        ON_COMMAND(PID_VIEW_MI_DETAILS,        CListViewEx::OnDetailsView)  // use base ones

        ON_COMMAND(PID_UTILS_MI_TAGTABLE,      CListViewEx::OnTagTable)     // use base ones
        ON_COMMAND(PID_UTILS_MI_LNGUPDATE,     CListViewEx::OnLangList)     // use base ones
        ON_COMMAND(PID_UTILS_MI_EXCLUSION,     CListViewEx::OnExclList)     // use base ones
        ON_COMMAND(PID_UTILS_MI_NEWTERMS,      CListViewEx::OnNewTerms)     // use base ones
        ON_COMMAND(PID_UTILS_MI_FOUNDTERMS,    CListViewEx::OnFoundTerms)   // use base ones
        ON_COMMAND(PID_UTILS_MI_PLGINMGR,      CListViewEx::OnPluginManager)  // new start
        ON_COMMAND(PID_UTILS_MI_ATOVERUP,      CListViewEx::OnAutoVersionUp)  // new end
        ON_COMMAND(PID_UTILS_MI_DRIVES,        OnConfDrives)   // usebase ones
        ON_COMMAND(PID_UTILS_MI_CONNECT,       OnConnectResources)   // usebase ones
        ON_COMMAND(PID_FILE_MI_PRINTLIST,      CListViewEx::OnPrintList)
        ON_COMMAND(PID_FILE_MI_SELECTALL,      CListViewEx::SelectAll)
        ON_COMMAND(PID_FILE_MI_DESELECTALL,    CListViewEx::DeSelectAll)
        ON_COMMAND(PID_FILE_MI_SYSPROP,        CListViewEx::OnSysProps)
        ON_COMMAND(PID_FILE_MI_EXIT,           CListViewEx::OnExit)
        ON_COMMAND(PID_FILE_MI_EBUS,           OnEBus )
        ON_COMMAND(PID_FILE_MI_EBUS1,          OnEBus1 )
        ON_COMMAND(PID_FILE_MI_EBUS2,          OnEBus2 )
        ON_COMMAND(PID_VIEW_MI_SHRINKPATH,     CListViewEx::OnShrinkPath)   // use base ones
        ON_COMMAND(PID_VIEW_MI_HIDEPATH,       CListViewEx::OnHidePath)     // use base ones
        ON_COMMAND(PID_VIEW_MI_SHOWPATH,       CListViewEx::OnShowPath)     // use base ones

        ON_MESSAGE( WM_EQF_TERMINATE,          On_WM_EQF_TERMINATE)
        ON_COMMAND(ID_FILE_PRINT, CGenListView::OnFilePrint)
        ON_COMMAND(ID_FILE_PRINT_DIRECT, CGenListView::OnFilePrint)
        ON_COMMAND(ID_FILE_PRINT_PREVIEW, CGenListView::OnFilePrintPreview)

        //}}AFX_MSG_MAP

		ON_COMMAND_RANGE( ID_TWBM_AAB, ID_TWBM_AAB_LAST, On_WM_COMMAND )
        ON_COMMAND_RANGE( PID_WIND_MI_TOP, PID_WIND_MI_BOT, OnWndActivate )
        ON_COMMAND_RANGE( ID_TWBM_AAB, ID_TWBM_AAB_LAST, On_WM_COMMAND )
        ON_UPDATE_COMMAND_UI_RANGE(ID_TWBM_AAB , ID_TWBM_AAB_LAST, CGenListView::OnToolbarActivate)
//       ON_UPDATE_COMMAND_UI_RANGE(ID_TWBM_AAB , ID_TWBM_AAB_LAST, OnToolbarActivate)



END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFLLRightPaneView

LRESULT CFLLRightPaneView::On_WM_EQF_TERMINATE( WPARAM mp1, LPARAM mp2 )
{
  this->SaveViewLists();
  return GENERICLISTWP( m_hWnd, WM_EQF_TERMINATE, mp1, mp2 );
}


CFLLRightPaneView::CFLLRightPaneView()
{
  pIda = NULL;

}

CFLLRightPaneView::~CFLLRightPaneView()
{
}

BOOL CFLLRightPaneView::PreCreateWindow(CREATESTRUCT& cs)
{
    UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(GENLISTINSTIDA), ERROR_STORAGE );
    return CGenListView::PreCreateWindow(cs);
}


LRESULT CFLLRightPaneView::On_WM_EQF_MDIACTIVATE( WPARAM mp1, LPARAM mp2 )
{
    if ( mp1 )
    {
        /********************************************************************/
        /* send activation request                                          */
        /********************************************************************/
        GENERICLISTWP( m_hWnd, WM_MDIACTIVATE, 0L, MP2FROMHWND(pIda->CommArea.hwndLB) );

        EqfActivateInstance( m_hWnd, TRUE );
    }
    else
    {
        /******************************************************************/
        /* mp2 contains handle of window to be deactivated                */
        /******************************************************************/
        CChildFrame *pWnd = (CChildFrame *) mp2;
        if ( mp2 )
        {
          CView*   pActView = pWnd->GetActiveView();
          GENERICLISTWP( m_hWnd, WM_MDIACTIVATE, 0L, MP2FROMHWND( pActView->m_hWnd ));
          EqfActivateInstance( m_hWnd, FALSE );
        } /* endif */
    } /* endif */
    return 0;
}



void CFLLRightPaneView::OnWndActivate( UINT mp1)
{
  CChildFrame  *pChildFrame = (CChildFrame *)(GetParent()->GetParent());
  pChildFrame->OnToggleWindow( mp1 );
}


VOID CFLLRightPaneView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView )
{
  CView::OnActivateView( bActivate, pActivateView, pDeactiveView );
  if ( bActivate )
  {
    HWND hwnd = EqfQueryObject( NULL, clsFOLDER, OBJ_ACTIVE | OBJ_BUSY );
    if ( !hwnd )
    {
       EqfActivateInstance( m_hWnd, bActivate );
    } /* endif */
  }
}

VOID CFLLRightPaneView::OnDetailsDlgWrapper( )
{
  OnDetailsDlg();
}

// save the current view lists to the approbriate property file
void CFLLRightPaneView::SaveViewLists()
{
  PLISTCOMMAREA pCommArea = &pIda->CommArea;

  CListViewEx::QueryView( CURRENT_VIEW, (LONG)&pCommArea->asCurView[0] );
  CListViewEx::QueryView( DETAILS_VIEW, (LONG)&pCommArea->asDetailsView[0] );
  CListViewEx::QueryView( SORT_VIEW,    (LONG)&pCommArea->asSortList[0] );

  if ( strcmp( UtlGetFnameFromPath( (PSZ)pCommArea->szObjName ), DEFAULT_FOLDERLIST_NAME ) == 0 )
  {
    // update folder list properties
    PPROPFOLDERLIST   pProp;
    EQFINFO           ErrorInfo;
    HPROP             hProp;

    /**************************************************************/
    /* Open properties                                            */
    /**************************************************************/
    hProp = OpenProperties( (PSZ)pCommArea->szObjName, NULL,
                            PROP_ACCESS_READ, &ErrorInfo );
    if ( hProp )
    {
      /************************************************************/
      /* Save current view lists and window position              */
      /************************************************************/
      if ( SetPropAccess( hProp, PROP_ACCESS_WRITE ) )
      {

        USHORT usWidth[MAX_VIEW+1];

       (BOOL)WinSendMsg( pCommArea->hwndLB,
                                  WM_EQF_GETCOLUMNWIDTH,
                                  (WPARAM)usWidth,
                                  NULL );

        pProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hProp);

        memcpy( pProp->sLastUsedViewList, pCommArea->asCurView,
                sizeof(pProp->sLastUsedViewList) );

        memcpy( (pProp->sLastUsedViewWidth), (usWidth),
               sizeof(pProp->sLastUsedViewWidth) );

        memcpy( pProp->sDetailsViewList, pCommArea->asDetailsView,
                sizeof(pProp->sDetailsViewList) );
        memcpy( pProp->sSortList, pCommArea->asSortList,
                sizeof(pProp->sSortList) );
        memcpy( &pProp->Filter, &pCommArea->Filter,
                sizeof(pProp->Filter) );
        SaveProperties( hProp, &ErrorInfo );
      } /* endif */
      CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
    } /* endif */
  }
  else if ( FolIsSubFolderObject( (PSZ)pCommArea->szObjName ) )
  {
    ULONG ulLen;
    PPROPFOLDER pProp = NULL;            // loaded property file

    if ( UtlLoadFileL( (PSZ)pCommArea->szObjName , (PVOID *)&pProp, &ulLen, FALSE, FALSE ) )
    {

       USHORT usWidth[MAX_VIEW+1];

       (BOOL)WinSendMsg( pCommArea->hwndLB,
                                  WM_EQF_GETCOLUMNWIDTH,
                                  (WPARAM)usWidth,
                                  NULL );

      memcpy( pProp->sLastUsedViewList, pCommArea->asCurView,
              sizeof(pProp->sLastUsedViewList) );

      memcpy( (pProp->sLastUsedViewWidth), (usWidth),
              sizeof(pProp->sLastUsedViewWidth) );

      memcpy( pProp->sDetailsViewList, pCommArea->asDetailsView,
              sizeof(pProp->sDetailsViewList) );
      memcpy( pProp->sSortList, pCommArea->asSortList,
              sizeof(pProp->sSortList) );
      memcpy( &pProp->Filter, &pCommArea->Filter,
              sizeof(pProp->Filter) );
      UtlWriteFileL( (PSZ)pCommArea->szObjName, ulLen, pProp, FALSE );
    } /* endif */
    if ( pProp ) UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
  }
  else
  {
    PPROPFOLDER   pProp;
    EQFINFO       ErrorInfo;
    HPROP         hProp;

    /**************************************************************/
    /* Replace drive letter in object name with drive letter      */
    /* of primary drive in order to build property name           */
    /**************************************************************/
    UtlQueryString( QST_PRIMARYDRIVE, pCommArea->szBuffer,
                    sizeof(pCommArea->szBuffer) );
    strcpy( pCommArea->szBuffer + 1, (PSZ)pCommArea->szObjName + 1);
    /**************************************************************/
    /* Open properties                                            */
    /**************************************************************/
    hProp = OpenProperties( pCommArea->szBuffer, NULL,
                            PROP_ACCESS_READ, &ErrorInfo );
    if ( hProp )
    {
      /************************************************************/
      /* Save current view lists and window position              */
      /************************************************************/
      if ( SetPropAccess( hProp, PROP_ACCESS_WRITE ) )
      {

        USHORT usWidth[MAX_DEFINEDCOLUMNS];

       (BOOL)WinSendMsg( pCommArea->hwndLB,
                                  WM_EQF_GETCOLUMNWIDTH,
                                  (WPARAM)usWidth,
                                  NULL );

        pProp = (PPROPFOLDER)MakePropPtrFromHnd( hProp);

        memcpy( pProp->sLastUsedViewList, pCommArea->asCurView,
                sizeof(pProp->sLastUsedViewList) );

        memcpy( (pProp->sLastUsedViewWidth), (usWidth),
                 sizeof(pProp->sLastUsedViewWidth) ) ;

        memcpy( pProp->sDetailsViewList, pCommArea->asDetailsView,
                sizeof(pProp->sDetailsViewList) );
        memcpy( pProp->sSortList, pCommArea->asSortList,
                sizeof(pProp->sSortList) );
        memcpy( &pProp->Filter, &pCommArea->Filter,
                sizeof(pProp->Filter) );
        SaveProperties( hProp, &ErrorInfo );
      } /* endif */
      CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
    } /* endif */
  } /* endif */
}

VOID CFLLRightPaneView::OnDetailsDlg(  )
{
  CListViewEx::OnDetailsDlg();
  this->SaveViewLists();
}


void CFLLRightPaneView::On_WM_COMMAND ( UINT mp1 )
{
  switch ( SHORT1FROMMP1(mp1))
  {
    case PID_FILE_MI_OPEN:
      if ( pIda->CommArea.sListObjClass == clsFOLDERLIST )
      {
          On_WM_EQF_OPEN( mp1, 0L );
      }
      else
      {
        // check if selected item is a subfolder
        SHORT sItem = QUERYSELECTIONHWND( pIda->CommArea.hwndLB );
        if ( sItem != LIT_NONE )
        {
          ::SendMessage( pIda->CommArea.hwndLB, LM_EQF_QUERYITEMTEXT,
                         sItem, (LPARAM) &pIda->CommArea.szBuffer[0] );

          // get parameter and search in folderlist...
          if ( FolIsSubFolderItem( pIda->CommArea.szBuffer ) )
          {
            OBJNAME ObjName;

            UtlParseX15( pIda->CommArea.szBuffer, 0 );
            strcpy( (PSZ)ObjName, pIda->CommArea.szBuffer );
            SubFolObjectNameToName( (PSZ)ObjName, pIda->CommArea.szBuffer );
            ((CFLLListView *)((CSplitterWnd*)GetParent())->GetPane(0,0))->OpenFolder( pIda->CommArea.szBuffer );
          }
          else
          {
            CGenListView::OnWMCOMMAND( mp1 );
          } /* endif */
        }
        else
        {
          CGenListView::OnWMCOMMAND( mp1 );
        } /* endif */
      } /* endif */
      break;
    case PID_VIEW_MI_HIDEPATH:
      CGenListView::OnWMCOMMAND( mp1 );
      break;
    //cv
    case PID_FILE_MI_COPY:
    case PID_FILE_MI_CUT:
    case PID_FILE_MI_PASTE:
        //CGenListView::SendMessage(WM_COMMAND,mp1,mp2);
        //break;
    default:
      CGenListView::OnWMCOMMAND( mp1 );
      break;
  } /* endswitch */
}
/////////////////////////////////////////////////////////////////////////////
// CFLLRightPaneView Darstellung

void CFLLRightPaneView::OnDraw(CDC* pDC)
{
	pDC;
}


/////////////////////////////////////////////////////////////////////////////
// CFLLRightPaneView Drucken

BOOL CFLLRightPaneView::OnPreparePrinting(CPrintInfo* pInfo)
{
        return DoPreparePrinting(pInfo);
}

void CFLLRightPaneView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{

}

void CFLLRightPaneView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{

}

void CFLLRightPaneView::OnInitialUpdate()
{
  pChildFrame = (CChildFrame *)GetParentFrame(); // store MDI parent ...
  CGenListView::OnInitialUpdate();
}

void CFLLRightPaneView::FillRightFolderPane( PSZ pFolder )
{
  // save folder name (will be destroyed during EqfRemoveObject as
  // folder name is stored in pIda->CommArea->szBuffer
  CString strFolder = pFolder;

  /********************************************************************/
  /* remove object and clear listbox                                  */
  /********************************************************************/
  if ( m_hWnd )
  {
    EqfRemoveObject( TWBFORCE, m_hWnd);
  } /* endif */
  ClearCLBox();

  // restore original folder name
  strcpy( pFolder, strFolder );

  if ( pIda )
  {
    PSZ pszObjName = (PSZ) pIda->CommArea.szObjName;
    memset(pIda, 0, sizeof(PGENLISTINSTIDA) );
    pIda->pfnCallBack = FolderCallBack;       //pCreateParms->pfnCallBack;
    pIda->pvUserData  = NULL;                 //pCreateParms->pvUserData;
    pIda->fRestart    = FALSE;                             //pCreateParms->fRestart;

    AfxGetApp()->WriteProfileString( EQFFOLDER, "FolderRecent", pFolder);

    SetFolderTitle( pFolder, GetParent() ) ;

    ANSITOOEM( pFolder );
    SubFolNameToObjectName( pFolder, pszObjName );

    pIda->pfnCallBack( &pIda->CommArea, m_hWnd, WM_EQF_COLUMNLIST,
                       0L, (LPARAM) &pIda->CommArea );


    pIda->hFrame = GetParentFrame()->m_hWnd;       // store frame window
    pIda->CommArea.hwndLB = m_hWnd;                // store listbox

    ULONG ul = ::GetWindowLong( m_hWnd, GWL_ID );

    pIda->CommArea.sListboxID = (SHORT) ul;

    ANCHORWNDIDA( m_hWnd, pIda );

    sPopUpId = pIda->CommArea.sPopupMenuID;
    sNoSelPopUpId = pIda->CommArea.sNoSelPopupMenuID;
    sMultSelPopUpId = pIda->CommArea.sMultPopupMenuID;

    /******************************************************************/
    /* insert the column headers                                      */
    /******************************************************************/
    CLBCTLDATA * pCtrl     = pIda->CommArea.pColData;
    CLBCOLDATA * pColTable = pCtrl->pColData;

    memcpy( asCurView, pCtrl->psLastUsedViewList, sizeof( asCurView ));
    memcpy( asDetailsView, pCtrl->psDetailsViewList, sizeof( asDetailsView ));
    memcpy( asNameView, pCtrl->psNameViewList, sizeof( asNameView ));
    memset( ColumnHeader, 0, sizeof(ColumnHeader) );

    if ( pIda->CommArea.hIcon )
    {
      if ( m_ImageList.GetSafeHandle() )
      {
         m_ImageList.DeleteImageList();
      } /* endif */

      m_ImageList.Create(16, 16, TRUE, 1, 4 );
      m_ImageList.Add( pIda->CommArea.hIcon );
    } /* endif */

    for (int i=0; (i <= MAX_DEFINEDCOLUMNS) && (pColTable+i)->pszTitle; i++ )
    {
        memcpy( ColumnHeader[i], (pColTable+i)->pszTitle,
                sizeof( ColumnHeader[i] ));


        usColumnWidth[i] = (pColTable+i)->usWidth;
        switch ( (pColTable+i)->usFormat )
        {
        case DT_LEFT:
            usColumnAdjust[i] = LVCFMT_LEFT;
            break;
        case DT_RIGHT:
            usColumnAdjust[i] = LVCFMT_RIGHT;
            break;
        default:
            usColumnAdjust[i] = LVCFMT_LEFT;
            break;
        } /* endswitch */
        usColumnType[i] = (USHORT)(pColTable+i)->DataType;
    } /* endfor */
    //usColumnWidth[1] += 2;
    InsertTitle();
    /****************************************************************/
    /* Set flags for handling of WM_EQFN_ messages                  */
    /****************************************************************/
    PSHORT   psMsg = pIda->CommArea.asMsgsWanted;

    while ( *psMsg != 0 )
    {
        switch ( *psMsg )
        {
        case WM_EQFN_PROPERTIESCHANGED :
            pIda->fWantsPropMsg = TRUE;
            break;
        case WM_EQF_QUERYSELECTEDNAMES :
            pIda->fWantsSelectedNamesMsg = TRUE;
            break;
        case WM_EQFN_DELETED :
            pIda->fWantsDelMsg = TRUE;
            break;
        case WM_EQFN_CREATED :
            pIda->fWantsCreateMsg = TRUE;
            break;
        } /* endswitch */
        psMsg++;
    } /* endwhile */
    /******************************************************************/
    /* insert items and register object                               */
    /******************************************************************/
    pIda->pfnCallBack( &pIda->CommArea, m_hWnd, WM_EQF_INITIALIZE,
                       (WPARAM) pIda->CommArea.hwndLB, NULL );

    /******************************************************************/
    /* support multi selection                                        */
    /******************************************************************/
    ULONG ulStyle = ::GetWindowLong( m_hWnd, GWL_STYLE );
    ulStyle &= ~LVS_SINGLESEL;
    ::SetWindowLong( m_hWnd, GWL_STYLE, ulStyle );

    EqfRegisterObject( pszObjName, m_hWnd, pIda->CommArea.sListObjClass );
    if ( ::SendMessage( pIda->CommArea.hwndLB, LB_GETCOUNT, NULL, NULL) > 0 )
    {
      ::SendMessage( pIda->CommArea.hwndLB, LB_SETCURSEL, 0, NULL );
    } /* endif */
//@@    EqfActivateInstance( m_hWnd, TRUE );
  }
}

void CFLLRightPaneView::FillRightFolderListPane( )
{

  /********************************************************************/
  /* remove object and clear listbox                                  */
  /********************************************************************/
  if ( m_hWnd )
  {
    EqfRemoveObject( TWBFORCE, m_hWnd);
  } /* endif */
  ClearCLBox();
  if ( pIda )
  {
    PSZ pszObjName = (PSZ)pIda->CommArea.szObjName;
    pIda->pfnCallBack = FolderListCallBack; //pCreateParms->pfnCallBack;
    pIda->pvUserData  = NULL;                 //pCreateParms->pvUserData;
    pIda->fRestart    = FALSE;                //pCreateParms->fRestart;

    UtlMakeEQFPath( pszObjName, NULC, SYSTEM_PATH, NULL );
    strcat( pszObjName, BACKSLASH_STR );
    strcat( pszObjName, DEFAULT_FOLDERLIST_NAME );

    pIda->pfnCallBack( &pIda->CommArea, m_hWnd, WM_EQF_COLUMNLIST,
                       0L, (LPARAM) &pIda->CommArea );

    pIda->hFrame = GetParentFrame()->m_hWnd;       // store frame window
    pIda->CommArea.hwndLB = m_hWnd;                          // store listbox

    ULONG ul = ::GetWindowLong( m_hWnd, GWL_ID );

    pIda->CommArea.sListboxID = (SHORT) ul;

    ANCHORWNDIDA( m_hWnd, pIda );

    sPopUpId = pIda->CommArea.sPopupMenuID;
    sNoSelPopUpId = pIda->CommArea.sNoSelPopupMenuID;
    sMultSelPopUpId = pIda->CommArea.sMultPopupMenuID;

    /******************************************************************/
    /* insert the column headers                                      */
    /******************************************************************/
    CLBCTLDATA * pCtrl     = pIda->CommArea.pColData;
    CLBCOLDATA * pColTable = pCtrl->pColData;

    memcpy( asCurView, pCtrl->psLastUsedViewList, sizeof( asCurView ));
    memcpy( asDetailsView, pCtrl->psDetailsViewList, sizeof( asDetailsView ));
    memcpy( asNameView, pCtrl->psNameViewList, sizeof( asNameView ));
    memset( ColumnHeader, 0, sizeof(ColumnHeader) );

    if ( pIda->CommArea.hIcon )
    {
      if ( m_ImageList.GetSafeHandle() )
      {
         m_ImageList.DeleteImageList();
      } /* endif */

      m_ImageList.Create(16, 16, TRUE, 1, 4 );
      m_ImageList.Add( pIda->CommArea.hIcon );
    } /* endif */

    for (int i=0; (i<20) && (pColTable+i)->pszTitle; i++ )
    {
        memcpy( ColumnHeader[i], (pColTable+i)->pszTitle,
                sizeof( ColumnHeader[i] ));


        usColumnWidth[i] = (pColTable+i)->usWidth;
        switch ( (pColTable+i)->usFormat )
        {
        case DT_LEFT:
            usColumnAdjust[i] = LVCFMT_LEFT;
            break;
        case DT_RIGHT:
            usColumnAdjust[i] = LVCFMT_RIGHT;
            break;
        default:
            usColumnAdjust[i] = LVCFMT_LEFT;
            break;
        } /* endswitch */
        usColumnType[i] = (USHORT)(pColTable+i)->DataType;
    } /* endfor */
    //usColumnWidth[1] += 2;

    InsertTitle();
    pIda->pfnCallBack( &pIda->CommArea, m_hWnd, WM_EQF_INITIALIZE,
                       (WPARAM) pIda->CommArea.hwndLB, NULL );

    /******************************************************************/
    /* support multi selection                                        */
    /******************************************************************/
    if ( !pIda->CommArea.fMultipleSel )
    {
    ULONG ulStyle = ::GetWindowLong( m_hWnd, GWL_STYLE );
    ulStyle |= LVS_SINGLESEL;
    ::SetWindowLong( m_hWnd, GWL_STYLE, ulStyle );
    } /* endif */

    // Register new object
    EqfRegisterObject( pszObjName, m_hWnd, pIda->CommArea.sListObjClass );

    EqfActivateInstance( m_hWnd, TRUE );
  }
}


/**********************************************************************/
/* force as default option an open on double click                    */
/* Differentiate between opening a folder and opening a file          */
/**********************************************************************/
LRESULT CFLLRightPaneView::On_WM_EQF_OPEN(WPARAM mp1, LPARAM mp2 )
{
  if ( pIda->CommArea.sListObjClass == clsFOLDERLIST )
  {
    /******************************************************************/
    /* get the folder name, open the folder and position tree         */
    /******************************************************************/
    SHORT      sItem = QUERYSELECTIONHWND( pIda->CommArea.hwndLB );
    if ( sItem != LIT_NONE )
    {
      PSZ pFolder;
      ::SendMessage( pIda->CommArea.hwndLB, LM_EQF_QUERYITEMTEXT,
                     sItem, (LPARAM) &pIda->CommArea.szBuffer[0] );
      // get parameter and search in folderlist...
      pFolder = UtlParseX15 (pIda->CommArea.szBuffer, 1);
      ((CFLLListView *)((CSplitterWnd*)GetParent())->GetPane(0,0))->OpenFolder( pFolder );
    }

  }
  else
  {
    /******************************************************************/
    /* pass on to generic handler                                     */
    /******************************************************************/
    CGenListView::On_WM_EQF_OPEN( mp1, mp2 );
  } /* endif */
  return 0;
}
/**********************************************************************/
/* force as default option an open on double click                    */
/* Differentiate between opening a folder and opening a file          */
/**********************************************************************/
VOID CFLLRightPaneView::OnLButtonDblClk(UINT uFlags, CPoint Point )
{
  if ( pIda->CommArea.sListObjClass == clsFOLDERLIST )
  {
    /******************************************************************/
    /* get the folder name, open the folder and position tree         */
    /******************************************************************/
    SHORT      sItem = QUERYSELECTIONHWND( pIda->CommArea.hwndLB );
    if ( sItem != LIT_NONE )
    {
      PSZ pFolder;
      ::SendMessage( pIda->CommArea.hwndLB, LM_EQF_QUERYITEMTEXT,
                     sItem, (LPARAM) &pIda->CommArea.szBuffer[0] );
      // get parameter and search in folderlist...
      pFolder = UtlParseX15 (pIda->CommArea.szBuffer, 1);
      ((CFLLListView *)((CSplitterWnd*)GetParent())->GetPane(0,0))->OpenFolder( pFolder );
    }

  }
  else
  {
    /******************************************************************/
    /* pass on to generic handler                                     */
    /******************************************************************/
    CGenListView::OnLButtonDblClk( uFlags, Point );
  } /* endif */
  return;
}


LRESULT CFLLRightPaneView::OnInitMenuPopup( WPARAM mp1, LPARAM mp2 )
{
  /********************************************************************/
  /* the windows pulldown is the 5th menu item                        */
  /********************************************************************/
  LRESULT      mResult = FALSE;
  UINT         usID = mp2;
  CChildFrame  *pChildFrame = (CChildFrame *)(GetParent()->GetParent());
  CMainFrame   *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();
  BOOL         fMaximized;
  CMDIChildWnd *pChildWnd = pFrameWnd->MDIGetActive( &fMaximized);
  pChildWnd;

  usID = ( fMaximized ) ? usID -1 : usID;

  switch ( usID )
  {
    case 0xFFFF:
        /****************************************************************/
        /* we were at the first sub-menu in maximized mode -- do nothing*/
        /* We have to do this here, because in MDI IsZoomed is not      */
        /* working correctly....                                        */
        /****************************************************************/
        break;
    case PID_TWBM_SM_HELP:
        // do nothing; leave help and windows items active ...

        // GQ 2015/10/13: disable some of the help items until out help system is working again
        UtlMenuDisableItem( PID_HELP_FOR_HELP );
        UtlMenuDisableItem( PID_HELP_MI_INDEX );


        break;
    case PID_TWBM_SM_WINDOWS:
        // use base function to build the windows pulldown ...
        mResult = pChildFrame->OnWindowsPopup( mp1, mp2 );
        break;
    case PID_TWBM_SM_VIEW:
        pFrameWnd->OnCheckToolBar();
        mResult = pIda->pfnCallBack( &pIda->CommArea ,
                                     m_hWnd, WM_EQF_INITMENU, mp2, 0 );
        break;

    default:
        mResult = pIda->pfnCallBack( &pIda->CommArea ,
                                     m_hWnd, WM_EQF_INITMENU, mp2, 0 );
        //--- ensure that some of the items are enabled anyway ---
        UtlMenuEnableItem( PID_FILE_MI_DESELECTALL );
        UtlMenuEnableItem( PID_FILE_MI_SELECTALL );
        UtlMenuEnableItem( PID_UTILS_MI_TAGTABLE );
        UtlMenuEnableItem( PID_UTILS_MI_LNGUPDATE );
        UtlMenuEnableItem( PID_UTILS_MI_PLGINMGR );
        UtlMenuEnableItem( PID_UTILS_MI_ATOVERUP );
        UtlMenuEnableItem( PID_UTILS_MI_DRIVES );
        UtlMenuEnableItem( PID_TERMLISTS_POPUP );
        UtlMenuEnableItem( PID_UTILS_MI_EXCLUSION );
        UtlMenuEnableItem( PID_UTILS_MI_NEWTERMS );
        UtlMenuEnableItem( PID_UTILS_MI_FOUNDTERMS );
        UtlMenuEnableItem( PID_UTILS_MI_ABBR );
        UtlMenuEnableItem( PID_FILE_MI_SYSPROP );
        UtlMenuEnableItem( PID_FILE_MI_EXIT );
        UtlMenuEnableItem( PID_UTILS_MI_MT );
        break;
  } /* endswitch */
  return mResult;
}


/////////////////////////////////////////////////////////////////////////////
// CFLLRightPaneView Diagnose

#ifdef _DEBUG
void CFLLRightPaneView::AssertValid() const
{
        CGenListView::AssertValid();
}

void CFLLRightPaneView::Dump(CDumpContext& dc) const
{
        CGenListView::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFLLRightPaneView


/////////////////////////////////////////////////////////////////////////////
// CTreeDropTarget

// cv
#define RECT_BORDER 10


/********************************************************************
OnDragEnter()
  Called when the user drags the object in Tree control.
********************************************************************/
DROPEFFECT CTreeDropTarget::OnDragEnter( CWnd* pWnd, COleDataObject* pDataObject,
  DWORD dwKeyState, CPoint point
)
{
	pDataObject; point;
    //CTreeCtrl *pTreeCtrl = (CTreeCtrl*)pWnd;
    CTreeCtrl& tree = ((CFLLListView*)pWnd)->GetTreeCtrl();

    // store source folder
    m_hItemFolder = tree.GetSelectedItem();

  DROPEFFECT dropeffectRet = DROPEFFECT_COPY;
  if ( (dwKeyState & MK_SHIFT) == MK_SHIFT)
    dropeffectRet = DROPEFFECT_MOVE;
  return dropeffectRet;
}


/********************************************************************
OnDragOver()
  Called when the user drags the object over Tree control.
********************************************************************/
DROPEFFECT CTreeDropTarget::OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point )
{
  pDataObject;

  DROPEFFECT dropeffectRet = DROPEFFECT_COPY;
  if ( (dwKeyState & MK_SHIFT) == MK_SHIFT)
    dropeffectRet = DROPEFFECT_MOVE;
  // Expand and highlight the item under the mouse and
  CTreeCtrl *pTreeCtrl = (CTreeCtrl *)pWnd;
  HTREEITEM hTItem = pTreeCtrl->HitTest(point);
  if ( hTItem != NULL )
  {
        // check if target folder == source folder
        if (hTItem == m_hItemFolder)
        {
          dropeffectRet = DROPEFFECT_NONE;
        }
        else
        {
        //pTreeCtrl->Expand(hTItem, TVE_EXPAND);
    pTreeCtrl->SelectDropTarget(hTItem);
        }
  }

  // Scroll Tree control depending on mouse position
  CRect rectClient;
  pWnd->GetClientRect(&rectClient);
  pWnd->ClientToScreen(rectClient);
  pWnd->ClientToScreen(&point);
  int nScrollDir = -1;
  if ( point.y >= rectClient.bottom - RECT_BORDER)
    nScrollDir = SB_LINEDOWN;
  else
  if ( (point.y <= rectClient.top + RECT_BORDER) )
    nScrollDir = SB_LINEUP;


  if ( nScrollDir != -1 )
  {
    int nScrollPos = pWnd->GetScrollPos(SB_VERT);
    WPARAM wParam = MAKELONG(nScrollDir, nScrollPos);
    pWnd->SendMessage(WM_VSCROLL, wParam);
  }

  nScrollDir = -1;
  if ( point.x <= rectClient.left + RECT_BORDER )
    nScrollDir = SB_LINELEFT;
  else
  if ( point.x >= rectClient.right - RECT_BORDER)
    nScrollDir = SB_LINERIGHT;

  if ( nScrollDir != -1 )
  {
    int nScrollPos = pWnd->GetScrollPos(SB_VERT);
    WPARAM wParam = MAKELONG(nScrollDir, nScrollPos);
    pWnd->SendMessage(WM_HSCROLL, wParam);
  }

  return dropeffectRet;
}

/********************************************************************
OnDragLeave()
  Called when the user drags the object out of Tree control.
********************************************************************/
void CTreeDropTarget::OnDragLeave( CWnd* pWnd )
{
  // Remove Highlighting
  pWnd->SendMessage(TVM_SELECTITEM, TVGN_DROPHILITE,0);

}


/********************************************************************
OnDrop()
  Called when the user drops the object in the  Tree control.
********************************************************************/
BOOL CTreeDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
    DROPEFFECT dropEffect, CPoint point)
{
	pDataObject;
  // add files to selected folder
    UINT uiFormat = UtlQueryUShort(QS_CLIPBOARDFORMAT);
    if (IsClipboardFormatAvailable(uiFormat))
    {
      CTreeCtrl *pTreeCtrl = (CTreeCtrl *)pWnd;
      HTREEITEM hTItem = pTreeCtrl->HitTest(point);
      if ( hTItem != NULL )
        {
            /*
            typedef struct _DocProp
            {
                char szLongName[MAX_LONGFILESPEC];
                char szName[MAX_FILESPEC];
            } DOCPROP,*PDOCPROP;

            typedef struct _DocList
            {
              short sBytes;
              short sAction;
              short sCount;
              char szFolder[MAX_FULLPATH];
              DOCPROP aDoc[1];
            } DOCLIST,*PDOCLIST;
            */

            PDOCLIST pList;
            CHAR szDocObjName[MAX_LONGFILESPEC];

            int ret;
            PDOCIMPIDA pImpIda;
            BOOL isSubFolder;
            CHAR szFolder[MAX_LONGFILESPEC];


            ((CFLLListView*)pWnd)->GetItemObjName(hTItem,szFolder,&isSubFolder);

            {
                HGLOBAL hMem;
                LPVOID lpBuffer;
                INT i,cbSize;
                CHAR * pszExt;
                CHAR szDrive[MAX_DRIVE];
                USHORT usRC;

                // get clipboard data
                OpenClipboard(NULL);
                hMem = GetClipboardData(uiFormat);
                lpBuffer = GlobalLock(hMem);
                GlobalUnlock(hMem);
                cbSize = ((PDOCLIST)lpBuffer)->lBytes;
                UtlAlloc( (PVOID *) &pList, 0L, cbSize, NOMSG );
                memcpy(pList,lpBuffer,cbSize);
                CloseClipboard();
                EmptyClipboard();

                UtlAlloc( (PVOID*)&pImpIda,0L,sizeof(DOCIMPIDA),NOMSG);

                pImpIda->fYesToAll = FALSE;
                strcpy( pImpIda->szParentObjName,szFolder);
                if (isSubFolder)
                {
                    ULONG ulFolderID = FolGetSubFolderIdFromObjName(szFolder);
                    pImpIda->ulParentID = ulFolderID;
                    UtlSplitFnameFromPath( szFolder );
                    UtlSplitFnameFromPath( szFolder );

                }
                if (strcmp(szFolder,pList->szFolder) == 0)
                    pImpIda->fYesToAll = TRUE;

                strcpy((CHAR *)pImpIda->szTargetFolder, szFolder);
                strcpy( (CHAR *)pImpIda->szToFolder, UtlGetFnameFromPath( (CHAR *)pImpIda->szTargetFolder ) );
                strcpy( (CHAR *)pImpIda->szFromFolder,UtlGetFnameFromPath(pList->szFolder));

                if (dropEffect == DROPEFFECT_MOVE)
                    pList->sAction = PID_FILE_MI_CUT;

                UtlQueryString( QST_PRIMARYDRIVE, szDrive, MAX_DRIVE );

                // loop throuh the document list
                for (i=0;i<pList->sCount;i++)
                {
                    BOOL fDeleted = FALSE;

                    strcpy( (CHAR *)pImpIda->szString,pList->aDoc[i].szName);
                    strcpy( (CHAR *)pImpIda->stFs.szDrive,szDrive);
                    Utlstrccpy( pImpIda->stFs.szName, pList->aDoc[i].szName, DOT );
                    pszExt = strchr( pList->aDoc[i].szName, DOT );
                    if ( pszExt != NULL )
                    {
                        strcpy( (CHAR *)pImpIda->stFs.szExt, pszExt );
                    }
                    else
                    {
                        pImpIda->stFs.szExt[0] = EOS;
                    }

                    // we have to do the delete first, if the document is moved
                    // between subfolders of the same folder
                    if ( strcmp( pList->szFolder, (CHAR *)pImpIda->szTargetFolder ) == 0 )
                    {
                      if (pList->sAction == PID_FILE_MI_CUT)
                      {
                          strcpy(szDocObjName,pList->szFolder);
                          strcat(szDocObjName,BACKSLASH_STR);
                          strcat(szDocObjName,pList->aDoc[i].szName);
                          DocumentDelete((CHAR *)szDocObjName,FALSE,&usRC);
                          fDeleted = TRUE;
                      } /* endif */
                    } /* endif */

                    // import document int internal format
                    ret = DocImpInternal(pImpIda);
                    if (ret != LOAD_OK)
                    {
                        UtlError( ERROR_EXPORT_INT, MB_CANCEL, 0, NULL, EQF_ERROR );
                        break;
                    }
                    else
                    {
                      // delete package file
                      CHAR szPackage[MAX_EQF_PATH];
                      UtlMakeEQFPath( szPackage, pImpIda->stFs.szDrive[0], EXPORT_PATH, NULL );
                      strcat( szPackage, BACKSLASH_STR );
                      strcat( szPackage, pImpIda->szFromFolder );
                      strcat( szPackage, BACKSLASH_STR );
                      strcat( szPackage, pList->aDoc[i].szName );
                      DocImpInternalDeletePackage( szPackage );
                    } /* endif */

                    // delete source document if not yet deleted and CUT operation
                    if ( !fDeleted )
                    {
                        if (pList->sAction == PID_FILE_MI_CUT)
                        {
                          BOOL fIsNew = FALSE;
                          CHAR szDocShortName[MAX_FILESPEC];

                          // build correct document short name (may be different from short name in target folder)
                          FolLongToShortDocName( pList->szFolder, pList->aDoc[i].szLongName, szDocShortName, &fIsNew );

                          // delete document
                          strcpy(szDocObjName,pList->szFolder);
                          strcat(szDocObjName,BACKSLASH_STR);
                          strcat(szDocObjName,szDocShortName);
                          DocumentDelete(szDocObjName,FALSE,&usRC);
                      } /* endif */
                    } /* endif */


                    // refresh document view
                    strcpy(szDocObjName,(CHAR *)pImpIda->szTargetFolder);
                    strcat(szDocObjName,BACKSLASH_STR);
                    strcat(szDocObjName,pList->aDoc[i].szName);
                    EqfSend2AllHandlers ( WM_EQFN_CREATED,
                                          MP1FROMSHORT( clsDOCUMENT ),
                                          MP2FROMP( szDocObjName ));

                }
                // free mem
                UtlAlloc( (PVOID *)&pImpIda,0L,0L,NOMSG);
                UtlAlloc( (PVOID *)&pList, 0L, 0L, NOMSG );
            }
        }
    }
  // Remove highlighting
  pWnd->SendMessage(TVM_SELECTITEM, TVGN_DROPHILITE,0);
  return TRUE;

}

// CTreeDropTarget end
/////////////////////////////////////////////////////////////////////////////
//
