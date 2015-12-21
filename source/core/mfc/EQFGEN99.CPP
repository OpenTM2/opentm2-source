/*! EQFGen99.C - EQF Generic    Handler                                        
	Copyright (c) 1990-2015, International Business Machines Corporation and others. All rights reserved.
*/

#include "eqfmfc.h"
#include "core\PluginManager\OtmPlugin.h"
#include "core\PluginManager\PluginManager.h"

#ifndef CPPTEST
extern "C"
{
#endif
#define INCL_EQF_TAGTABLE         // tag table and format functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_ANALYSIS            // Analysis functions
#define INCL_EQF_EDITORAPI        // for WM_EQF_SHOWHTML message

/**********************************************************************/
/* ensure correct packing of our structures                           */
/**********************************************************************/
#pragma pack( push, TM2StructPacking, 1 )

#include "eqf.h"                  // General .H for EQF
#include "eqfstart.id"            // IDs for EQFSTARR resource
#include "eqfdde.h"               // batch mode definitions

#include "eqffll00.h"             // for FLL_OBJECT_IND
#include "eqffol00.h"             // for FOL_SIZE_IND
#include "EQFHLOG.H"              // defines for history log processing
#include "eqfcolw.id"             // column width IDs
#include "eqfstart.h"

//------------------------------------------------------------------------------
//  Helptable include files                                                     
//     - ID files for dialogs and windows                                       
//     - help ID file                                                           
//     - message ID file                                                        
//     - help table                                                             
//------------------------------------------------------------------------------
#include "eqfutils.id"
#include "eqfdoc01.id"
#include "eqfiana1.id"
#include "eqfmem.id"
#include "eqffol.id"
#include "eqffll.id"
#include "eqftag00.id"                 // tag handler IDs
#include <EQFTIMP.ID>
#include <EQFTEXP.ID>
#include <EQFLNG00.ID>
#include "eqfdic00.id"
#include "eqfutclb.id"
#include "EQFDIMP.ID"
#include "EQFDEX.ID"
#include "EQFTIMP.ID"
#include "EQFTEXP.ID"
#include "EQFCNT01.ID"
#include "EQFFILT.ID"
#include "EQFQDPR.ID"
#include "EQFTwb.ID"                   // IDs of Twb dialogs
#include "EQFB.ID"                     // IDs of Translation Processor window
#include "EQFITMD.ID"                  // IDs of ITM dialogs and windows
#include "EQFBDLG.ID"                  // IDs of Translation Processor dialogs
#include "EQFDPROP.ID"                 // IDs of Dictionary Dialogs - 1 -
#include "EQFDDLG.ID"                  // IDs of Dictionary Dialogs - 2 -
#include "EQFRDICS.ID"                 // IDs for remote dictionary
#include "EQFLIST.ID"                  // IDs of list handler windows and dialogs
#include "EQFTMM.ID"                   // IDs of TMM window
#include "EQFTMFUN.ID"                 // IDs of TMM dialogs
#include "eqfrpt.id"                 // IDs report handler dialog
#include "eqfmt.id"                    // IDs of mt properties

#undef _WPTMIF                         // we don't care about WP I/F
#include "eqfhelp.id"                  // help resource IDs
#ifdef _TQM
  #include "eqftqm.id"                   // TQM control IDs
  #include "eqftqm.hid"                  // TQM help IDs
  #include "eqftqm.htb"                          // first part of help tables
  #include "eqfmsg.htb"                  // message help table
#else
  #include "eqfhlp1.h"                   // first part of help tables
  #include "eqfmsg.htb"                  // message help table
#endif


#pragma pack( pop, TM2StructPacking )

#ifndef CPPTEST
}
#endif
// cv
#include <afxadv.h>

#include "eqfclbox.h"   // base class for Column listbox
#include "eqfd.h"
#include "eqfcfrm.h"    // child frame base class
#include "eqfGen99.h"
#include "eqfrpt99.h"   // report handler specifics
#include <dde.h>        // DDE defines
#include "eqfmfrm.h"    // main frame base class
#include "eqfweb.h"    // main frame base class
#include "EQFMUPRP.H"  // markup table property dialog class
#include <windows.h>
#include <mshtmhst.h>
#include <urlmon.h>
#include <tchar.h>
#include <comdef.h>
#include <objbase.h>
#pragma comment(lib, "urlmon.lib")


int BSTRToLocal(LPTSTR pLocal, BSTR pWide, DWORD dwChars);
int LocalToBSTR(BSTR pWide, LPTSTR pLocal, DWORD dwChars);




/**********************************************************************/
/* our class definition file                                          */
/**********************************************************************/

/////////////////////////////////////////////////////////////////////////////
// CGenListView

IMPLEMENT_DYNCREATE(CGenListView, CListViewEx)

BEGIN_MESSAGE_MAP(CGenListView, CListViewEx)
//{{AFX_MSG_MAP(CGenListView)
ON_WM_LBUTTONDBLCLK()
ON_WM_KEYUP()
ON_WM_CREATE()
//      ON_WM_CLOSE()
ON_WM_TIMER()
ON_MESSAGE( WM_CLOSE,         On_WM_CLOSE   )
ON_MESSAGE( WM_DESTROY,       On_WM_DESTROY )
ON_MESSAGE( WM_EQF_MDIACTIVATE, On_WM_EQF_MDIACTIVATE )
ON_MESSAGE( WM_INITMENU,      On_WM_INITMENU )
ON_MESSAGE( WM_INITMENUPOPUP, On_WM_INITMENUPOPUP )

ON_COMMAND( PID_FILE_MI_EBUS, OnEBus )
ON_COMMAND( PID_FILE_MI_EBUS1, OnEBus1 )
ON_COMMAND( PID_FILE_MI_EBUS2, OnEBus2 )

ON_MESSAGE( WM_EQF_INITIALIZE,                On_WM_EQF_INITIALIZE)
ON_MESSAGE( WM_EQF_TERMINATE,                 On_WM_EQF_TERMINATE)
ON_MESSAGE( WM_EQF_SHUTDOWN,                  On_WM_EQF_SHUTDOWN)
ON_MESSAGE( WM_EQFN_SHUTDOWNCANCELED,         On_WM_EQFN_SHUTDOWNCANCELED)
ON_MESSAGE( WM_EQF_QUERYSYSTEMPATH,           On_WM_EQF_QUERYSYSTEMPATH)
ON_MESSAGE( WM_EQF_QUERYSYSPROPPATH,          On_WM_EQF_QUERYSYSPROPPATH)
ON_MESSAGE( WM_EQF_ACTIVATEINSTANCE,          On_WM_EQF_ACTIVATEINSTANCE)
ON_MESSAGE( WM_EQF_QUERYMENUTBL,              On_WM_EQF_QUERYMENUTBL)
ON_MESSAGE( WM_EQF_OPEN,                      On_WM_EQF_OPEN)
ON_MESSAGE( WM_EQF_CREATE,                    On_WM_EQF_CREATE)
ON_MESSAGE( WM_EQF_DELETE,                    On_WM_EQF_DELETE)
ON_MESSAGE( WM_EQF_ABOUTTODELETE,             On_WM_EQF_ABOUTTODELETE)
ON_MESSAGE( WM_EQFN_CREATED,                  On_WM_EQFN_CREATED)
ON_MESSAGE( WM_EQFN_DELETED,                  On_WM_EQFN_DELETED)
ON_MESSAGE( WM_EQFN_PROPERTIESCHANGED,        On_WM_EQFN_PROPERTIESCHANGED)
ON_MESSAGE( WM_EQF_INSERTNAMES,               On_WM_EQF_INSERTNAMES)
ON_MESSAGE( WM_EQF_QUERYSELECTEDNAMES,        On_WM_EQF_QUERYSELECTEDNAMES)
ON_MESSAGE( WM_EQF_COMMAND,                   On_WM_EQF_COMMAND)
ON_MESSAGE( WM_EQF_QUERYHANDLER,              On_WM_EQF_QUERYHANDLER)
ON_MESSAGE( WM_EQF_REMOVEHANDLER,             On_WM_EQF_REMOVEHANDLER)
ON_MESSAGE( WM_EQF_REMOVEOBJECT,              On_WM_EQF_REMOVEOBJECT)
ON_MESSAGE( WM_EQF_QUERYOBJECT,               On_WM_EQF_QUERYOBJECT)
ON_MESSAGE( WM_EQF_QUERYOBJECTNAME,           On_WM_EQF_QUERYOBJECTNAME)
ON_MESSAGE( WM_EQF_QUERYOBJECTCLASS,          On_WM_EQF_QUERYOBJECTCLASS)
ON_MESSAGE( WM_EQF_QUERYOBJECTSTATUS,         On_WM_EQF_QUERYOBJECTSTATUS)
ON_MESSAGE( WM_EQF_SETOBJECTSTATUS,           On_WM_EQF_SETOBJECTSTATUS)
ON_MESSAGE( WM_EQF_QUERYOBJECTCOUNT,          On_WM_EQF_QUERYOBJECTCOUNT)
ON_MESSAGE( WM_EQF_GETOBJECTLIST,             On_WM_EQF_GETOBJECTLIST)
ON_MESSAGE( WM_EQF_QUERYHANDLERCOUNT,         On_WM_EQF_QUERYHANDLERCOUNT)
ON_MESSAGE( WM_EQF_SETSYMBOL,                 On_WM_EQF_SETSYMBOL)
ON_MESSAGE( WM_EQF_QUERYSYMBOL,               On_WM_EQF_QUERYSYMBOL)
ON_MESSAGE( WM_EQF_REMOVESYMBOL,              On_WM_EQF_REMOVESYMBOL)
ON_MESSAGE( WM_EQFN_OBJECTREMOVED,            On_WM_EQFN_OBJECTREMOVED)
ON_MESSAGE( WM_EQF_QUERYSYSTEMPROPHND,        On_WM_EQF_QUERYSYSTEMPROPHND)
ON_MESSAGE( WM_EQF_OPENPROPERTIES,            On_WM_EQF_OPENPROPERTIES)
ON_MESSAGE( WM_EQF_CREATEPROPERTIES,          On_WM_EQF_CREATEPROPERTIES)
ON_MESSAGE( WM_EQF_DELETEPROPERTIES,          On_WM_EQF_DELETEPROPERTIES)
ON_MESSAGE( WM_EQF_CLOSEPROPERTIES,           On_WM_EQF_CLOSEPROPERTIES)
ON_MESSAGE( WM_EQF_GETALLPROPERTIES,          On_WM_EQF_GETALLPROPERTIES)
ON_MESSAGE( WM_EQF_PUTALLPROPERTIES,          On_WM_EQF_PUTALLPROPERTIES)
ON_MESSAGE( WM_EQF_SAVEPROPERTIES,            On_WM_EQF_SAVEPROPERTIES)
ON_MESSAGE( WM_EQF_QUERYPROPSIZE,             On_WM_EQF_QUERYPROPSIZE)
ON_MESSAGE( WM_EQF_REGISTER,                  On_WM_EQF_REGISTER)
ON_MESSAGE( WM_EQF_REINIT,                    On_WM_EQF_REINIT)
ON_MESSAGE( WM_EQF_LSTREGISTER,               On_WM_EQF_LSTREGISTER)
ON_MESSAGE( WM_EQF_LSTEVENT,                  On_WM_EQF_LSTEVENT)
ON_MESSAGE( WM_EQF_LSTQRYLOCK,                On_WM_EQF_LSTQRYLOCK)
ON_MESSAGE( WM_EQF_LSTSETLOCK,                On_WM_EQF_LSTSETLOCK)
ON_MESSAGE( WM_EQF_REFRESH,                   On_WM_EQF_REFRESH)
ON_MESSAGE( WM_EQF_SETFOCUS,                  On_WM_EQF_SETFOCUS)
ON_MESSAGE( LM_EQF_QUERYVIEWLIST,             On_LM_EQF_QUERYVIEWLIST)
ON_MESSAGE( LM_EQF_SETVIEWLIST,               On_LM_EQF_SETVIEWLIST)
ON_MESSAGE( LM_EQF_SETSORTLIST,               On_LM_EQF_SETSORTLIST)
ON_MESSAGE( LM_EQF_QUERYSORTLIST,             On_LM_EQF_QUERYSORTLIST)
ON_MESSAGE( LM_EQF_SETFILTER,                 On_LM_EQF_SETFILTER)
ON_MESSAGE( LM_EQF_QUERYFILTER,               On_LM_EQF_QUERYFILTER)
ON_MESSAGE( WM_EQF_NEXTSTEP,                  On_WM_EQF_NEXTSTEP)
ON_MESSAGE( WM_EQF_PROCESSTASK,               On_WM_EQF_PROCESSTASK)
ON_MESSAGE( WM_EQFN_TASKDONE,                 On_WM_EQFN_TASKDONE)
ON_MESSAGE( WM_EQF_ABOUTTOREMOVEDRIVE,        On_WM_EQF_ABOUTTOREMOVEDRIVE)
ON_MESSAGE( WM_EQF_SLIDER_POSITION,           On_WM_EQF_SLIDER_POSITION)
ON_MESSAGE( WM_EQF_SLIDER_SETTEXT,            On_WM_EQF_SLIDER_SETTEXT)
ON_MESSAGE( WM_EQF_MEMLOAD_START,             On_WM_EQF_MEMLOAD_START)
ON_MESSAGE( WM_EQF_MEMLOAD_PROCESS,           On_WM_EQF_MEMLOAD_PROCESS)
ON_MESSAGE( WM_EQF_MEMLOAD_END,               On_WM_EQF_MEMLOAD_END)
ON_MESSAGE( WM_EQF_MEMFILL_LISTBOX,           On_WM_EQF_MEMFILL_LISTBOX)
ON_MESSAGE( WM_EQF_MEMEXPORT_START,           On_WM_EQF_MEMEXPORT_START)
ON_MESSAGE( WM_EQF_MEMEXPORT_PROCESS,         On_WM_EQF_MEMEXPORT_PROCESS)
ON_MESSAGE( WM_EQF_MEMEXPORT_END,             On_WM_EQF_MEMEXPORT_END)
ON_MESSAGE( WM_EQF_MEMORGANIZE_START,         On_WM_EQF_MEMORGANIZE_START)
ON_MESSAGE( WM_EQF_MEMORGANIZE_PROCESS,       On_WM_EQF_MEMORGANIZE_PROCESS)
ON_MESSAGE( WM_EQF_MEMORGANIZE_END,           On_WM_EQF_MEMORGANIZE_END)
ON_MESSAGE( WM_EQF_MEMMERGE_START,            On_WM_EQF_MEMMERGE_START)
ON_MESSAGE( WM_EQF_MEMMERGE_PROCESS,          On_WM_EQF_MEMMERGE_PROCESS)
ON_MESSAGE( WM_EQF_MEMMERGE_END,              On_WM_EQF_MEMMERGE_END)
ON_MESSAGE( WM_EQF_QUERY,                     On_WM_EQF_QUERY)
ON_MESSAGE( WM_EQF_FONTCHANGED,               On_WM_EQF_FONTCHANGED)
ON_MESSAGE( WM_EQF_MEMORGANIZE_ACTIVATE,      On_WM_EQF_MEMORGANIZE_ACTIVATE)
ON_MESSAGE( WM_DRIVEBUTTON_INIT,              On_WM_DRIVEBUTTON_INIT)
ON_MESSAGE( WM_DRIVEBUTTON_SELECT,            On_WM_DRIVEBUTTON_SELECT)
ON_MESSAGE( WM_EQF_DDE_REQUEST,               On_WM_EQF_DDE_REQUEST)
ON_MESSAGE( WM_EQF_DDE_ANSWER,                On_WM_EQF_DDE_ANSWER)
ON_MESSAGE( LM_EQF_SETITEMSTATE,              On_LM_EQF_SETITEMSTATE)
ON_MESSAGE( LM_EQF_QUERYITEMSTATE,            On_LM_EQF_QUERYITEMSTATE)
ON_MESSAGE( LM_EQF_INSERTITEMSTATE,           On_LM_EQF_INSERTITEMSTATE)
ON_MESSAGE( WM_EQFN_DRIVEREMOVED,             On_WM_EQFN_DRIVEREMOVED)
ON_MESSAGE( WM_EQFN_DRIVEADDED,               On_WM_EQFN_DRIVEADDED)
ON_MESSAGE( WM_EQF_COLCHANGED,                On_WM_EQF_COLCHANGED)
ON_MESSAGE( WM_EQF_WD_MAIN_NOTIFY,            On_WM_EQF_WD_MAIN_NOTIFY)
ON_MESSAGE( WM_EQF_INITMENU,                  On_WM_EQF_INITMENU)
ON_MESSAGE( WM_EQF_BUILDITEMTEXT,             On_WM_EQF_BUILDITEMTEXT)
ON_MESSAGE( WM_EQF_CREATELISTWINDOW,          On_WM_EQF_CREATELISTWINDOW)
ON_MESSAGE( WM_EQF_UPDATESLIDER,              On_WM_EQF_UPDATESLIDER)
ON_MESSAGE( WM_EQF_SHOWPOPUP,                 On_WM_EQF_SHOWPOPUP)
ON_MESSAGE( EQF_CD_BM_CLICK,                  On_EQF_CD_BM_CLICK)
ON_MESSAGE( WM_EQF_CHECK_REM_PROPS,           On_WM_EQF_CHECK_REM_PROPS)
ON_MESSAGE( WM_EQF_FILTSETDICT,               On_WM_EQF_FILTSETDICT)
ON_MESSAGE( WM_EQF_QUERYID,                   On_WM_EQF_QUERYID)
ON_MESSAGE( WM_EQF_CHANGEOBJECTNAME,          On_WM_EQF_CHANGEOBJECTNAME)
ON_MESSAGE( WM_EQF_CLOSE,                     On_WM_EQF_CLOSE)
ON_MESSAGE( LM_EQF_GETITEMFROMPOINT,          On_LM_EQF_GETITEMFROMPOINT)
ON_MESSAGE( LM_EQF_REFRESH,                   On_LM_EQF_REFRESH)

ON_MESSAGE( LB_RESETCONTENT,           OnResetContent )
ON_MESSAGE( LB_ADDSTRING,              OnAddString )
ON_MESSAGE( LB_INSERTSTRING,           OnInsertString )
ON_MESSAGE( LB_DELETESTRING,           OnDeleteString )
ON_MESSAGE( LB_SETITEMDATA,            OnSetItemData )
ON_MESSAGE( LB_GETCURSEL,              OnQuerySelection )
ON_MESSAGE( LB_SETCURSEL,              OnSetSelection )
ON_MESSAGE( LB_SETSEL,                 OnSetMultSel )
ON_MESSAGE( LB_GETITEMDATA,            OnQueryItemHandle )
ON_MESSAGE( LB_GETTEXT,                OnQueryItemText )
ON_MESSAGE( LB_FINDSTRING,             OnFindString )
ON_MESSAGE( LB_FINDSTRINGEXACT,        OnFindString )
ON_MESSAGE( LB_GETCOUNT,               OnGetCount )
ON_MESSAGE( LB_GETSELCOUNT,            OnGetSelCount )
ON_MESSAGE( LB_GETSELITEMS,            OnGetSelItems )
ON_MESSAGE( LB_SETTOPINDEX,            OnSetTopIndex )
ON_MESSAGE( LM_EQF_QUERYITEMTEXT,      OnEqfQueryItemText )
//      ON_MESSAGE( LM_EQF_QUERYITEMSTATE,     OnQueryItemState )
//      ON_MESSAGE( LM_EQF_QUERYVIEWLIST,      OnQueryViewList )

//cv
ON_COMMAND(PID_FILE_MI_COPY, OnFileCopy)
ON_COMMAND(PID_FILE_MI_CUT, OnFileCut)
ON_COMMAND(PID_FILE_MI_PASTE, OnFilePaste)

ON_MESSAGE( WM_EQF_SHOWHTML,           On_WM_EQF_SHOWHTML )
ON_MESSAGE( WM_EQF_TAGTABLEPROPS,      On_WM_EQF_TAGTABLEPROPS)
ON_COMMAND(PID_VIEW_MI_SORT,           CListViewEx::OnSortDlg)      // use base ones
ON_COMMAND(PID_VIEW_MI_ALL,            CListViewEx::OnAll)          // use base ones
ON_COMMAND(PID_VIEW_MI_SOME,           CListViewEx::OnFilterDlg)    // use base ones
ON_COMMAND(PID_VIEW_MI_DETAILSDLG,     CListViewEx::OnDetailsDlg)   // use base ones
ON_COMMAND(PID_VIEW_MI_NAMES,          CListViewEx::OnNamesView)    // use base ones
ON_COMMAND(PID_VIEW_MI_DETAILS,        CListViewEx::OnDetailsView)  // use base ones

ON_COMMAND(PID_VIEW_MI_SHRINKPATH,     CListViewEx::OnShrinkPath)   // use base ones
ON_COMMAND(PID_VIEW_MI_HIDEPATH,       CListViewEx::OnHidePath)     // use base ones
ON_COMMAND(PID_VIEW_MI_SHOWPATH,       CListViewEx::OnShowPath)     // use base ones

ON_COMMAND(PID_UTILS_MI_TAGTABLE,      CListViewEx::OnTagTable)     // use base ones
ON_COMMAND(PID_UTILS_MI_MT,            CListViewEx::OnMTList)       // use base ones
ON_COMMAND(PID_UTILS_MI_LNGUPDATE,     CListViewEx::OnLangList)     // use base ones
ON_COMMAND(PID_UTILS_MI_EXCLUSION,     CListViewEx::OnExclList)     // use base ones
ON_COMMAND(PID_UTILS_MI_NEWTERMS,      CListViewEx::OnNewTerms)     // use base ones
ON_COMMAND(PID_UTILS_MI_FOUNDTERMS,    CListViewEx::OnFoundTerms)   // use base ones
ON_COMMAND(PID_UTILS_MI_PLGINMGR,      CListViewEx::OnPluginManager)
ON_COMMAND(PID_UTILS_MI_ATOVERUP,      CListViewEx::OnAutoVersionUp)
ON_COMMAND(PID_UTILS_MI_DRIVES,        CListViewEx::OnConfDrives)   // usebase ones
ON_COMMAND(PID_UTILS_MI_CONNECT,       CListViewEx::OnConnectResources)   // usebase ones
ON_COMMAND(PID_FILE_MI_PRINTLIST,      CListViewEx::OnPrintList)
ON_COMMAND(PID_FILE_MI_SELECTALL,      CListViewEx::SelectAll)
ON_COMMAND(PID_FILE_MI_DESELECTALL,    CListViewEx::DeSelectAll)
ON_COMMAND(PID_FILE_MI_SYSPROP,        CListViewEx::OnSysProps)
ON_COMMAND(PID_FILE_MI_EXIT,           CListViewEx::OnExit)

ON_COMMAND(PID_TQM_MI_LOGON,          OnTQMLogon)
ON_COMMAND(PID_TQM_MI_LOGOFF,         OnTQMLogoff)
ON_COMMAND(PID_TQM_MI_USER,           OnTQMUser)
ON_COMMAND(PID_TQM_MI_EVALUATION,     OnTQMEvaluation)
ON_COMMAND(PID_TQM_MI_VENDOR,         OnTQMVendor)
ON_COMMAND(PID_TQM_MI_REPORTS,        OnTQMReports)
ON_COMMAND(PID_TQM_MI_SETTINGS,       OnTQMSettings)
ON_COMMAND(PID_TQM_MI_PROJECTS,       OnTQMProjects)
ON_COMMAND(PID_TQM_MI_ARCHIVE,        OnTQMArchive)

ON_COMMAND(PID_WIND_MI_CASCADE ,       OnMDICascade )
ON_COMMAND(PID_WIND_MI_TILE ,          OnMDITile )
ON_COMMAND(PID_WIND_MI_MINALL,         OnMDIMinAll )
ON_COMMAND(PID_WIND_MI_RESTOREALL,     OnMDIRestore )

ON_COMMAND(PID_VIEW_TPRO_TOOLBAR,      OnViewTPROToolBar)
ON_COMMAND(PID_VIEW_MI_STATUSBAR,      OnViewStatusBar)
ON_COMMAND(PID_VIEW_TWB_TOOLBAR,       OnViewTWBToolBar)
ON_NOTIFY_REFLECT(HDN_ITEMCLICK, OnHeaderClick)

// cv
ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBegindrag)
//
//}}AFX_MSG_MAP
ON_COMMAND_RANGE( PID_WIND_MI_TOP, PID_WIND_MI_BOT, OnWndActivate )
ON_COMMAND_RANGE( ID_TWBM_AAB, ID_TWBM_AAB_LAST, OnWMCOMMAND )
ON_UPDATE_COMMAND_UI_RANGE(ID_TWBM_AAB , ID_TWBM_AAB_LAST, OnToolbarActivate)
ON_UPDATE_COMMAND_UI( ID_INDICATOR_CAPS, OnUpdateStatusTextCaps )
END_MESSAGE_MAP()

// cv
void CGenListView::OnFileCopy()
{
    //pIda->CommArea.fUserFlag = m_uiFormat;
    pIda->pfnCallBack( &pIda->CommArea ,
                       m_hWnd, WM_EQF_COMMAND, PID_FILE_MI_COPY, FALSE);
}

void CGenListView::OnFileCut()
{
    //pIda->CommArea.fUserFlag = m_uiFormat;
    pIda->pfnCallBack( &pIda->CommArea ,
                       m_hWnd, WM_EQF_COMMAND, PID_FILE_MI_CUT, FALSE);
}

void CGenListView::OnFilePaste()
{
    //pIda->CommArea.fUserFlag = m_uiFormat;
    pIda->pfnCallBack( &pIda->CommArea ,
                       m_hWnd, WM_EQF_COMMAND, PID_FILE_MI_PASTE, FALSE);
}


void CGenListView::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	pNMHDR;
    // TODO: Add your control notification handler code here
    CString szItem;
    DROPEFFECT dwDropEffect;
    COleDropSource *pDropSource = new COleDropSource;
    COleDataSource *pDataSource = new COleDataSource;

    // first export the documents before dragging them
    pIda->pfnCallBack( &pIda->CommArea ,
                       m_hWnd, WM_EQF_COMMAND, PID_FILE_MI_COPY, FALSE);

    dwDropEffect = pDataSource->DoDragDrop(DROPEFFECT_MOVE|DROPEFFECT_COPY, NULL, pDropSource);

    delete pDropSource;
    delete pDataSource;

    *pResult = 0;
}


DROPEFFECT CGenListView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	pDataObject; point;
    // TODO: Add your specialized code here and/or call the base class
    //if (pDataObject->IsDataAvailable(m_uiFormat))
    if (IsClipboardFormatAvailable(m_uiFormat))
    {
        if ((dwKeyState & MK_SHIFT) == MK_SHIFT)
            return DROPEFFECT_MOVE;
        else
            return DROPEFFECT_COPY;
    }
    return DROPEFFECT_NONE;

    //return CListView::OnDragOver(pDataObject, dwKeyState, point);
}

BOOL CGenListView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
    // TODO: Add your specialized code here and/or call the base class
    CString s;
    pChildFrame->MDIActivate();
    if (IsClipboardFormatAvailable(m_uiFormat))
    {
        short sAction;
        if (dropEffect == DROPEFFECT_COPY)
            sAction = PID_FILE_MI_COPY;
        else
            sAction = PID_FILE_MI_CUT;

        pIda->pfnCallBack( &pIda->CommArea ,
                           m_hWnd, WM_EQF_COMMAND, PID_FILE_MI_PASTE, sAction);

        return TRUE;
    }
    return CListView::OnDrop(pDataObject, dropEffect, point);
}
//


void CGenListView::OnUpdateStatusTextCaps(CCmdUI *pCmdUI)
{

    pCmdUI->SetText( "" );

    CMainFrame   *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();

    CListCtrl& ListCtrl = GetListCtrl();
    UINT usI = ListCtrl.GetItemCount();
    UINT usJ = ListCtrl.GetSelectedCount();
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
      sprintf( chText, szString, usJ, usI );
    } /* endif */
    pFrameWnd->OnSetStatusBar( 0, chText );
}
void CGenListView::OnViewTWBToolBar()
{
    CMainFrame  *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();
    pFrameWnd->OnToggleToolBar( PID_VIEW_TWB_TOOLBAR );
};

void CGenListView::OnViewTPROToolBar()
{
    CMainFrame  *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();
    pFrameWnd->OnToggleToolBar( PID_VIEW_TPRO_TOOLBAR );
};

void CGenListView::OnViewStatusBar()
{
    CMainFrame  *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();
    pFrameWnd->OnToggleToolBar( PID_VIEW_MI_STATUSBAR );
};

VOID CGenListView::OnWndActivate( UINT mp1 )
{
    pChildFrame->OnToggleWindow( mp1 );
}

VOID CGenListView::OnWMCOMMAND( UINT mp1 )
{
  short sCommand = SHORT1FROMMP1(mp1);
  if ( (sCommand >= ID_TWBM_AAB_TOOLPLUGINS) && (sCommand  < ID_TWBM_AAB_LAST) )
  {
    PluginManager *pPluginManager = PluginManager::getInstance();
    pPluginManager->processToolCommand( sCommand );
  }
  else
  { 
    GENERICLISTWP( m_hWnd, WM_COMMAND, mp1, NULL ); 
  }
}


void CGenListView::OnToolbarActivate(CCmdUI* pCmdUI)
{
    UINT usID = pCmdUI->m_nID;
    BOOL   fEnable = FALSE;
    /********************************************************************/
    /* enable selection for all active objects                          */
    /********************************************************************/
    if ( (usID >= ID_TWBM_AAB_TOOLPLUGINS) && (usID  < ID_TWBM_AAB_LAST) )
    {
        fEnable = TRUE;
    }
    else if ( (usID >= PID_WIND_MI_TOP) && ( usID <= PID_WIND_MI_BOT) )
    {
        fEnable = TRUE;
    }
    else
    {
        switch ( usID )
        {
        case PID_UTILS_MI_TAGTABLE:
        case PID_UTILS_MI_LNGUPDATE:
        case PID_UTILS_MI_PLGINMGR:
        case PID_UTILS_MI_ATOVERUP:
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
        case PID_UTILS_MI_MT:

            /****************************************************************/
            /* enable above items all the time..                            */
            /****************************************************************/
            fEnable = TRUE;
            break;
        case PID_VIEW_TWB_TOOLBAR:
        case PID_VIEW_TPRO_TOOLBAR:
        case PID_VIEW_MI_STATUSBAR:
            /****************************************************************/
            /* enable above items all the time..                            */
            /****************************************************************/
            fEnable = TRUE;
            break;
        case PID_FILE_MI_COPY:
        case PID_FILE_MI_CUT:
        case PID_FILE_MI_PASTE:
            if (!m_uiFormat)
                break;
            //pIda->CommArea.fUserFlag = m_uiFormat;
        default:
            fEnable = pIda->pfnCallBack( &pIda->CommArea ,
                                         m_hWnd, WM_EQF_TOOLBAR_ENABLED, usID, FALSE);
            break;
        } /* endswitch */
    } /* endif */
    pCmdUI->Enable( fEnable );
}

LRESULT CGenListView::On_WM_INITMENUPOPUP( WPARAM mp1, LPARAM mp2 )
{
    LRESULT      LRESULT = FALSE;
    UINT         usID = (USHORT) mp2;
    CMainFrame   *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();
    BOOL         fMaximized;
  	CMDIChildWnd *pChildWnd = pFrameWnd->MDIGetActive( &fMaximized);
  	pChildWnd;  // avoid compiler warnings
    usID = ( fMaximized ) ? usID -1 : usID;

    switch ( usID )
    {
    case (UINT)-1:
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
        LRESULT = pChildFrame->OnWindowsPopup( mp1, mp2 );
        break;
    case PID_TWBM_SM_VIEW:
#ifndef _TQM
        pFrameWnd->OnCheckToolBar();
#endif
        LRESULT = pIda->pfnCallBack( &pIda->CommArea ,
                                     m_hWnd, WM_EQF_INITMENU, mp2, 0 );
        break;
    case PID_FILE_MI_COPY:
    case PID_FILE_MI_CUT:
    case PID_FILE_MI_PASTE:
        if (!m_uiFormat)
            break;
        //pIda->CommArea.fUserFlag = m_uiFormat;

    default:
        LRESULT = pIda->pfnCallBack( &pIda->CommArea ,
                                     m_hWnd, WM_EQF_INITMENU, mp2, 0 );
        //--- ensure that some of the items are enabled anyway ---
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
    return LRESULT;
}

VOID CGenListView::OnMDIMinAll( )
{
        pChildFrame->OnMDIMinAll();
}

VOID CGenListView::OnMDIRestore()
{
        pChildFrame->OnMDIRestore();
};

LRESULT CGenListView::On_WM_EQF_MDIACTIVATE( WPARAM mp1, LPARAM mp2 )
{
    if ( mp1 )
    {
        /********************************************************************/
        /* send activation request                                          */
        /********************************************************************/
        GENERICLISTWP( m_hWnd, WM_MDIACTIVATE, 0L, MP2FROMHWND(pIda->CommArea.hwndLB) );
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
        } /* endif */
    } /* endif */
    return 0;
}


void CGenListView::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags)
{
	nRepCnt; nFlags;
    if ( (GetKeyState( VK_SHIFT ) & 0x8000) || (GetKeyState( VK_CTRL ) & 0x8000))
    {
        /******************************************************************/
        /* pass message on                                                */
        /******************************************************************/
        Default();
    }
    else
    {
        SHORT sParentID;
        CMainFrame  *pFrameWnd;
        switch ( nChar )
        {
        case VK_DELETE:
            /******************************************************************/
            /* delete key pressed...                                          */
            /******************************************************************/
            PostMessage( WM_COMMAND, MP1FROMSHORT (PID_FILE_MI_DELETE), 0L);
            break;
        case VK_F1:
            /**************************************************************/
            /* process help ...                                           */
            /**************************************************************/
            SendMessage( WM_EQF_QUERYID, 0, MP2FROMP(&sParentID) );
            pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();
            pFrameWnd->PostMessage( HM_HELPSUBITEM_NOT_FOUND, 0,
                                    MP2FROM2SHORT( sParentID, 0 ) );
            break;
        default:
            Default();
            break;
        } /* endswitch */
    } /* endif */

}

int CGenListView::OnCreate(LPCREATESTRUCT cs)
{
    LRESULT LRESULT = pIda->pfnCallBack( &pIda->CommArea , m_hWnd, WM_CREATE, NULL,
                                         (LPARAM) pIda->pvUserData  );

    // return asap if no window should be created
    if ( (SHORT)LRESULT == DO_NOT_CREATE )
    {
        return( -1 );
    } /* endif */

    //CMenu *pMenu = GetMenu();
    //UtlSetULong( QL_TWBMENU, (ULONG)pMenu->m_hMenu );

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

    pChildFrame = (CChildFrame *)GetParent(); // store MDI parent ...
    return CListViewEx::OnCreate(cs);
}

LRESULT CGenListView::On_WM_CLOSE( WPARAM mp1, LPARAM mp2 )
{
	mp1;
    LRESULT lResult = FALSE;
    if ( !pIda->CommArea.fNoClose )
    {
        lResult = ((PFNWP)(GENERICLISTWP))( m_hWnd, WM_CLOSE, NULL, mp2 );
        if ( lResult != TRUE )
        {
          ((CMDIChildWnd*) GetParent())->MDIDestroy();
        } /* endif */
    }
    else
    {
        lResult = TRUE;
    } /* endif */
    return lResult;
}

LRESULT CGenListView::OnQueryItemText( WPARAM mp1, LPARAM mp2 )
{
  PSZ pszItemText = NULL;
  PSZ pszTarget   = (PSZ)mp2;
  LRESULT lResult = 0;

  if ( (SHORT)mp1 < GetListCtrl().GetItemCount() )
  {
    pszItemText = ((PSZ) GetListCtrl().GetItemData( (SHORT) mp1 ))+4;
    strcpy( pszTarget, pszItemText );
    lResult = (LRESULT)strlen(pszItemText);
  }
  else
  {
    lResult = LB_ERR;
  } /* endif */

  return( lResult );
}

LRESULT CGenListView::OnEqfQueryItemText( WPARAM mp1, LPARAM mp2 )
{
  PSZ pszItemText = NULL;
  PSZ pszTarget   = (PSZ)mp2;
  LRESULT lResult = 0;

  if ( (SHORT)mp1 < GetListCtrl().GetItemCount() )
  {
    pszItemText = ((PSZ) GetListCtrl().GetItemData( (SHORT) mp1 ))+4;
    strcpy( pszTarget, pszItemText );
    lResult = (LRESULT)strlen(pszItemText);
  }
  else
  {
    lResult = LB_ERR;
  } /* endif */

  return( lResult );
}




LRESULT CGenListView::OnAddString( WPARAM mp1, LPARAM mp2 )
{
    int iImage = 0;
    if ( pIda && pIda->CommArea.sListObjClass == clsFOLDER )
    {
        BOOL fSubFolder = FALSE;

        /******************************************************************/
        /* we are dealing with a document list                            */
        /******************************************************************/
        PSZ pszBuffer = (PSZ) mp2;

#ifndef _TQM
        fSubFolder = FolIsSubFolderItem( pszBuffer );
#endif

        if ( fSubFolder )
        {
          // use folder icon for entry
          //sImage = m_ImageList.Add( pIda->CommArea.hIcon );
          iImage = m_ImageList.Add( LoadIcon( AfxGetResourceHandle(),
                                    MAKEINTRESOURCE( FLL_ICON ) )  );
        }
        else
        {
          /*********************************************************************/
          /* Use SHGetFileInfo call to set correct icon image                  */
          /*********************************************************************/
          PSZ pData = UtlParseX15( pszBuffer, 1 );

          LONG lRc;
          SHFILEINFO FAR sfi;
          memset( &sfi, 0, sizeof( sfi ));
          lRc = SHGetFileInfo( pData,
                               FILE_ATTRIBUTE_OFFLINE,
                               &sfi,
                               sizeof( sfi ),
                               SHGFI_USEFILEATTRIBUTES |
                               SHGFI_ICON | //SHGFI_ICONLOCATION | SHGFI_SMALLICON |
                               SHGFI_SYSICONINDEX
                             );
          /******************************************************************/
          /* we have more than one column -- therefore add split character  */
          /* (0x15) again                                                   */
          /******************************************************************/
          *(pData+strlen(pData)) = X15;
          if ( lRc != 0L )
          {
              /***************************************************************/
              /* we found the image list, so add it to our images...         */
              /***************************************************************/
              iImage = m_ImageList.Add( sfi.hIcon );
              DestroyIcon( sfi.hIcon );
          } /* endif */
        } /* endif */
    } /* endif */

    return InsertRow( (PSZ) mp2, (SHORT) mp1, (SHORT)iImage );

}


/////////////////////////////////////////////////////////////////////////////
// CGenListView construction/destruction

CGenListView::CGenListView()
{
    pIda = NULL;
    fInitialUpdate = FALSE;
    pChildFrame = NULL;
}

CGenListView::~CGenListView()
{
    m_ImageList.DeleteImageList();
}

BOOL CGenListView::PreCreateWindow(CREATESTRUCT& cs)
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


    m_uiFormat = UtlQueryUShort(QS_CLIPBOARDFORMAT);

    return CListViewEx::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CGenListView initialization

void CGenListView::OnInitialUpdate()
{
  if ( !fInitialUpdate )
  {
    //cv
    if (pIda->CommArea.sListObjClass == clsFOLDER && m_uiFormat)
        m_DropTarget.Register(this);
    else if (pIda->CommArea.sListObjClass != clsFOLDERLIST)
        m_uiFormat = 0;
    //
    fInitialUpdate = TRUE;
    CListViewEx::OnInitialUpdate();
    {
      strcpy( (PSZ)pIda->CommArea.szBuffer, (PSZ)pIda->CommArea.szTitle );
      if ( pIda->CommArea.pColData->pFilter )
      {
        strcat( (PSZ)pIda->CommArea.szBuffer, " [Some]" );
      } /* endif */
      SetWindowText( (PSZ)pIda->CommArea.szBuffer );
    }

    pIda->hFrame = GetParentFrame()->m_hWnd;       // store frame window
    pIda->CommArea.hwndLB = m_hWnd;                          // store listbox

    ULONG ul = ::GetWindowLong( m_hWnd, GWL_ID );

    pIda->CommArea.sListboxID = (SHORT) ul;

    ANCHORWNDIDA( m_hWnd, pIda );

    sPopUpId = pIda->CommArea.sPopupMenuID;
    sNoSelPopUpId = pIda->CommArea.sNoSelPopupMenuID;
    sMultSelPopUpId = pIda->CommArea.sMultPopupMenuID;
    /********************************************************************/
    /* disable close option                                             */
    /********************************************************************/
    {
        CMenu * pMenu;         // handle of system menu
        pMenu = GetParentFrame()->GetSystemMenu( FALSE );
        if ( pMenu != NULL )
        {
          if ( pIda->CommArea.fNoClose )
          {
            pMenu->EnableMenuItem( SC_CLOSE, MF_GRAYED );
          }
          else
          {
            pMenu->EnableMenuItem( SC_CLOSE, MF_ENABLED );
          } /* endif */
        } /* endif */
    } /* endif */

    /******************************************************************/
    /* insert the column headers                                      */
    /******************************************************************/
    CLBCTLDATA * pCtrl     = pIda->CommArea.pColData;
    CLBCOLDATA * pColTable = pCtrl->pColData;

    memcpy( asCurView, pCtrl->psLastUsedViewList, sizeof( asCurView ));
    memcpy( asDetailsView, pCtrl->psDetailsViewList, sizeof( asDetailsView ));
    memcpy( asNameView, pCtrl->psNameViewList, sizeof( asNameView ));
    memcpy( asSortView, pCtrl->psSortList, sizeof( asSortView ));
    if ( pCtrl->pFilter )
    {
      memcpy( &m_Filter, pCtrl->pFilter, sizeof( m_Filter ));
    }
    else
    {
      memset( &m_Filter, 0, sizeof( m_Filter ));
    } /* endif */
    if ( pIda->CommArea.hIcon )
    {
        m_ImageList.Create(16, 16, TRUE, 1, 4 );
        m_ImageList.Add( pIda->CommArea.hIcon );
    } /* endif */

    for (int i=0; (i <= MAX_DEFINEDCOLUMNS) && (pColTable+i)->pszTitle; i++ )
    {
        memcpy( ColumnHeader[i], (pColTable+i)->pszTitle, sizeof( ColumnHeader[i] ));
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
    // set the icon
    GetParentFrame()->SetIcon( pIda->CommArea.hIcon, FALSE );

    /****************************************************************/
    /* Mask minimize flag if not restarted by TWB                   */
    /****************************************************************/
    if ( !pIda->fRestart )
    {
        pIda->CommArea.swpSizePos.fs &= ~EQF_SWP_MINIMIZE;
    } /* endif */
    //--- set list window position and size ---
    if ( pIda->CommArea.fInvisible )
    {
        WinSetWindowPos( pIda->hFrame, HWND_TOP,
                         pIda->CommArea.swpSizePos.x,  pIda->CommArea.swpSizePos.y,
                         pIda->CommArea.swpSizePos.cx, pIda->CommArea.swpSizePos.cy,
                         (USHORT)(pIda->CommArea.swpSizePos.fs |
                                  EQF_SWP_SIZE | EQF_SWP_MOVE));
    }
    else
    {
        WinSetWindowPos( pIda->hFrame, HWND_TOP,
                         pIda->CommArea.swpSizePos.x,  pIda->CommArea.swpSizePos.y,
                         pIda->CommArea.swpSizePos.cx, pIda->CommArea.swpSizePos.cy,
                         (USHORT)(pIda->CommArea.swpSizePos.fs | EQF_SWP_ACTIVATE |
                                  EQF_SWP_SHOW | EQF_SWP_SIZE | EQF_SWP_MOVE));
    }
  } /* endif */
}


/////////////////////////////////////////////////////////////////////////////
// CGenListView diagnostics

#ifdef _DEBUG
void CGenListView::AssertValid() const
{
    CListViewEx::AssertValid();
}

void CGenListView::Dump(CDumpContext& dc) const
{
    CListViewEx::Dump(dc);
}

CGenListDoc* CGenListView::GetDocument() // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGenListDoc)));
    return(CGenListDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CGenListView message handlers

VOID CGenListView::OnDeleted(SHORT sClassId, PSZ pObject)
{
	sClassId;
    /******************************************************************/
    /* delete entry in listbox                                        */
    /******************************************************************/
    DeleteRow( pObject);
    return;
}


/**********************************************************************/
/* force as default option an open on double click                    */
/**********************************************************************/
VOID CGenListView::OnLButtonDblClk(UINT uFlags, CPoint Point )
{
	uFlags; Point;
    SHORT      sItem = QUERYSELECTIONHWND( pIda->CommArea.hwndLB );

    if ( (sItem != LIT_NONE) && (pIda->CommArea.sDefaultAction != 0) )
    {
        if ( WinSendMsg( pIda->CommArea.hwndLB, LM_EQF_QUERYITEMSTATE,
                         MP1FROMSHORT(sItem), 0L ) )
        {
            (*pIda->pfnCallBack)( &pIda->CommArea,
                                  m_hWnd,
                                  WM_EQF_COMMAND,
                                  MP1FROMSHORT(pIda->CommArea.sDefaultAction),
                                  MP2FROMP(NULL) );
        } /* endif */
    } /* endif */
    return;
}



void CGenListView::OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	pNMHDR;
        // TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

    *pResult = 0;
}


/////////////////////////////////////////////////////////////////////////////
// CGenListDoc

IMPLEMENT_DYNCREATE(CGenListDoc, CEQFDoc)

BEGIN_MESSAGE_MAP(CGenListDoc, CEQFDoc)
//{{AFX_MSG_MAP(CGenListDoc)
// NOTE - the ClassWizard will add and remove mapping macros here.
//    DO NOT EDIT what you see in these blocks of generated code!

//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGenListDoc construction/destruction

CGenListDoc::CGenListDoc()
{

    fUserFlag = FALSE;

}

CGenListDoc::~CGenListDoc()
{
}

BOOL CGenListDoc::OnNewDocument()
{
    if (!CEQFDoc::OnNewDocument())
        return FALSE;

    PLISTCOMMAREA pCommArea = &(((CListViewEx*)pActView)->pIda->CommArea);

    strcpy( (PSZ)pCommArea->szBuffer, (PSZ)pCommArea->szTitle );
    if ( (pCommArea->Filter.asColumn[0] != 0) || (pCommArea->Filter.asOperator[0] != 0) )
    {
      strcat( (PSZ)pCommArea->szBuffer, " [Some]" );
    } /* endif */
    SetTitle( (PSZ)pCommArea->szBuffer );

    EqfRegisterObject( GetObjName(), (pActView->m_hWnd),
                       ((CListViewEx*)pActView)->pIda->CommArea.sListObjClass );

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGenListDoc helper functions



/////////////////////////////////////////////////////////////////////////////
// CGenListDoc serialization

void CGenListDoc::Serialize(CArchive& ar)
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
// CGenListDoc diagnostics

#ifdef _DEBUG
void CGenListDoc::AssertValid() const
{
    CEQFDoc::AssertValid();
}

void CGenListDoc::Dump(CDumpContext& dc) const
{
    CEQFDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGenListDoc commands



// Start the homepage
// ------------------

void CGenListView::OnEBus()
{


    StartBrowser("home");

}




// Start the Online Docu
// ---------------------


void CGenListView::OnEBus1()
{


    StartBrowser("docu");

}// OnEBus1

void CGenListView::OnEBus2()
{


    StartBrowser("docu");

}// OnEBus2




LRESULT CGenListView::On_WM_EQF_SHOWHTML( WPARAM mp1, LPARAM mp2 )
{
  mp1;

  HRESULT   hr = E_OUTOFMEMORY;
  IMonikerPtr pmk;
  #define ARRSIZE 200
  OLECHAR   pchHTML[ARRSIZE];
  SHOWHTMLDIALOGFN* pfnShowHTMLDialog = NULL;
  HINSTANCE hinstMSHTML = NULL;

  TCHAR chHTMLFILE[200];


  lstrcpy(chHTMLFILE, (PSZ) mp2 );

  LocalToBSTR(pchHTML, chHTMLFILE , ARRSIZE);

  hinstMSHTML = LoadLibrary(TEXT("MSHTML.DLL"));

  if (NULL != hinstMSHTML)
  {
      pfnShowHTMLDialog = (SHOWHTMLDIALOGFN*)
                          GetProcAddress(hinstMSHTML, TEXT("ShowHTMLDialog"));
  }

  if (NULL != pfnShowHTMLDialog)
      hr = CreateURLMoniker(NULL, pchHTML, &pmk);

  if (SUCCEEDED(hr))
  {


      hr = pfnShowHTMLDialog(EqfQueryTwbClient(),       // HWND        hwndParent,
                             pmk,        // IMoniker *  pMk,
                             NULL,       // VARIANT *   pvarArgIn,
                             NULL,       // options
                             NULL        // VARIANT *   pvarArgOut,
                            );
  }

  if (hinstMSHTML)
      FreeLibrary(hinstMSHTML);



    return(FALSE);
} //On_WM_EQF_SHOWHTML

// start markup table properties dialog
LRESULT CGenListView::On_WM_EQF_TAGTABLEPROPS( WPARAM mp1, LPARAM mp2 )
{
  mp1;
  CMUPropSheet MuPropDlg; 

  if ( mp2 ) MuPropDlg.SetTagTableObjName( (PSZ)mp2 );
  MuPropDlg.m_psh.dwFlags |= PSH_HASHELP;
  MuPropDlg.DoModal();
  return(TRUE);
} //On_WM_EQF_TAGTABLEPROPS


/**************************************************************************

   BSTRToLocal()

**************************************************************************/

int BSTRToLocal(LPTSTR pLocal, BSTR pWide, DWORD dwChars)
{
    *pLocal = 0;

#ifdef UNICODE
    lstrcpyn(pLocal, pWide, dwChars);
#else
    WideCharToMultiByte( CP_ACP,
                         0,
                         pWide,
                         -1,
                         pLocal,
                         dwChars,
                         NULL,
                         NULL);
#endif

    return lstrlen(pLocal);
}

/**************************************************************************

   LocalToBSTR()

**************************************************************************/

int LocalToBSTR(BSTR pWide, LPTSTR pLocal, DWORD dwChars)
{
    *pWide = 0;

#ifdef UNICODE
    lstrcpyn(pWide, pLocal, dwChars);
#else
    MultiByteToWideChar( CP_ACP,
                         0,
                         pLocal,
                         -1,
                         pWide,
                         dwChars);
#endif

    return lstrlenW(pWide);
}

VOID CGenListView::OnTQMLogon()
{
  EqfSend2Handler( TQMLISTHANDLER, WM_EQF_COMMAND,
                   MP1FROMSHORT(PID_TQM_MI_LOGON), 0L );
  return;
}

VOID CGenListView::OnTQMLogoff()
{
  EqfSend2Handler( TQMLISTHANDLER, WM_EQF_COMMAND,
                   MP1FROMSHORT(PID_TQM_MI_LOGOFF), 0L );
  return;
}

VOID CGenListView::OnTQMUser()
{
  EqfSend2Handler( TQMLISTHANDLER, WM_EQF_COMMAND,
                   MP1FROMSHORT(PID_TQM_MI_USER), 0L );
  return;
}

VOID CGenListView::OnTQMEvaluation()
{
  EqfSend2Handler( TQMLISTHANDLER, WM_EQF_COMMAND,
                   MP1FROMSHORT(PID_TQM_MI_EVALUATION), 0L );
  return;
}

VOID CGenListView::OnTQMVendor()
{
  EqfSend2Handler( TQMLISTHANDLER, WM_EQF_COMMAND,
                   MP1FROMSHORT(PID_TQM_MI_VENDOR), 0L );
  return;
}

VOID CGenListView::OnTQMReports()
{
  EqfSend2Handler( TQMLISTHANDLER, WM_EQF_COMMAND,
                   MP1FROMSHORT(PID_TQM_MI_REPORTS), 0L );
  return;
}

VOID CGenListView::OnTQMSettings()
{
  EqfSend2Handler( TQMLISTHANDLER, WM_EQF_COMMAND,
                   MP1FROMSHORT(PID_TQM_MI_SETTINGS), 0L );
  return;
}

VOID CGenListView::OnTQMProjects()
{
  EqfSend2Handler( TQMLISTHANDLER, WM_EQF_COMMAND,
                   MP1FROMSHORT(PID_TQM_MI_PROJECTS), 0L );
  return;
}

VOID CGenListView::OnTQMArchive()
{
  EqfSend2Handler( TQMLISTHANDLER, WM_EQF_COMMAND,
                   MP1FROMSHORT(PID_TQM_MI_ARCHIVE), 0L );
  return;
}



