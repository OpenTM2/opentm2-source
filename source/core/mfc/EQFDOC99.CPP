/*! \file
	Description: EQF Document  Handler

	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqfmfc.h"

#ifndef CPPTEST
extern "C"
{
#endif
  #define INCL_EQF_EDITORAPI
  #define INCL_EQF_TP
  #define INCL_EQF_ASD
  #define INCL_EQF_TM
  #define INCL_EQF_TAGTABLE         // tag table and format functions
  #define INCL_EQF_FOLDER           // folder list and document list functions
  #define INCL_EQF_ANALYSIS            // Analysis functions

/**********************************************************************/
/* ensure correct packing of our structures                           */
/**********************************************************************/
#pragma pack( push, TM2StructPacking, 1 )

  #include "eqf.h"                  // General .H for EQF
  #include "eqfstart.id"            // IDs for EQFSTARR resource
  #include "eqfdde.h"               // batch mode definitions
  #include "eqfdoc00.h"             // for FLL_OBJECT_IND
  #include "eqfstart.h"
  #include "eqfb.id"
  #include "eqfbdlg.id"

#pragma pack( pop, TM2StructPacking )

#ifndef CPPTEST
}
#endif


#include "eqfclbox.h"   // base class for Column listbox
#include "eqfDOC99.h"
#include "eqffnt99.h"
#include "eqfcfrm.h"                   // base class for child frames
#include <dde.h>                       // DDE defines
#include "eqfmfrm.h"                   // main frame base class

#include "eqfd.h"

#ifndef CPPTEST
extern "C" {
#endif
VOID EQFBOnDrop
(
  PDOCUMENT_IDA pIda,
  LONG          lxPos,
  LONG          lyPos,
  PSZ_W         pData,
  BOOL          fMove
);
#ifndef CPPTEST
}
#endif

/**********************************************************************/
/* our class definition file                                          */
/**********************************************************************/

#define TM_PREVIEW "TranslationManager-Preview"
LRESULT APIENTRY EQFSHOWHTML( HWND, WINMSG, WPARAM, LPARAM );

/////////////////////////////////////////////////////////////////////////////
// CGenTEnvView

CLIPFORMAT CGenTEnvView::m_cfObjectDescriptor=NULL;

IMPLEMENT_DYNCREATE(CGenTEnvView, CView)

BEGIN_MESSAGE_MAP(CGenTEnvView, CView)
        //{{AFX_MSG_MAP(CGenTEnvView)
        ON_WM_LBUTTONDBLCLK()
        ON_WM_CREATE()
        ON_WM_CLOSE()
        ON_MESSAGE( WM_INITMENUPOPUP, OnInitMenuPopup )

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
        ON_MESSAGE( WM_EQF_PROOF,                     On_WM_EQF_PROOF)
        ON_MESSAGE( WM_EQF_MISSPELLED,                On_WM_EQF_MISSPELLED)
        ON_MESSAGE( WM_EQF_AUTOTRANS,                 On_WM_EQF_AUTOTRANS)
        ON_MESSAGE( WM_EQF_DDE_REQUEST,               On_WM_EQF_DDE_REQUEST)
        ON_MESSAGE( WM_EQF_DDE_ANSWER,                On_WM_EQF_DDE_ANSWER)
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
        ON_MESSAGE( WM_EQF_SHOWHTML,                  On_WM_EQF_SHOWHTML )
        ON_MESSAGE( WM_EQF_IS_TENV ,                  On_WM_EQF_IS_TENV )
        ON_MESSAGE( WM_EQF_DRAGDROP,                  On_WM_EQF_DRAGDROP )
//      ON_MESSAGE( EQFM_DOC_IS_SAVED,                On_EQFM_DOC_IS_SAVED )
//      ON_MESSAGE( EQFM_DOC_IS_LOADED,               On_EQFM_DOC_IS_LOADED )
//      ON_MESSAGE( EQFM_DOC_STATUS,                  On_EQFM_DOC_STATUS )
//      ON_MESSAGE( EQFM_DOC_IS_XLATED,               On_EQFM_DOC_IS_XLATED )
//      ON_MESSAGE( EQFM_DOC_IS_ULATED,               On_EQFM_DOC_IS_ULATED )
        ON_COMMAND( PID_SYS_CLOSE,                    On_PID_SYS_CLOSE )
        ON_COMMAND( PID_SYS_MOVE,                     On_PID_SYS_MOVE  )
        ON_COMMAND( PID_SYS_SIZE,                     On_PID_SYS_SIZE  )
        ON_COMMAND( PID_WIND_MI_MINALL,               OnMDIMinAll )
        ON_COMMAND( PID_WIND_MI_RESTOREALL,           OnMDIRestore )
        ON_COMMAND( IDM_FONTEXT,                      OnFontExtDlg )
        ON_MESSAGE( WM_EQF_UPDATESTATUSBAR_TEXT,      UpdateStatusBar )
        //}}AFX_MSG_MAP

        ON_COMMAND_RANGE( ID_TP_MAIN_WINDOW, IDM_LASTDOC_ENVIRONMENT, OnAllCommand )
        ON_UPDATE_COMMAND_UI_RANGE(ID_TP_MAIN_WINDOW, IDM_LASTDOC_ENVIRONMENT, OnToolbarActivate)
        ON_UPDATE_COMMAND_UI( ID_INDICATOR_CAPS, OnUpdateStatusIns )
        ON_UPDATE_COMMAND_UI( ID_INDICATOR_NUM, OnUpdateStatusSegNum )
        ON_UPDATE_COMMAND_UI( ID_INDICATOR_SCRL, OnUpdateStatusLinNum )
END_MESSAGE_MAP()

LRESULT CGenTEnvView::UpdateStatusBar( WPARAM mp1, LPARAM mp2 )
{
	mp1;
    CChildFrame  *pChildFrame = (CChildFrame *)(GetParent());
    CMainFrame   *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();
    pFrameWnd->OnSetStatusBar( 0, (PSZ)mp2 );
    return 0;
}

void CGenTEnvView::OnUpdateStatusIns(CCmdUI *pCmdUI)
{

  PSTEQFGEN   pstEQFGen = this->pIda->pstEQFGen;
  PTBDOCUMENT pDoc = ACCESSWNDIDA( pstEQFGen->hwndEditorTgt, PTBDOCUMENT );

  pCmdUI->Enable();

  if ( pDoc )
  {
    if ( pDoc->hwndRichEdit )
  {
      if ( pDoc->EQFBFlags.inserting )
    {
        pCmdUI->SetText( pDoc->chInsStatusText );
    }
      else
    {
        CHAR  chString[40];
        LOADSTRING( NULL, AfxGetResourceHandle(), ID_INDICATOR_OVR, chString );
        pCmdUI->SetText( chString );
    } /* endif */
  }
    else
  {
      if ( pDoc->usCursorType == CURSOR_REPLACE )
    {
        CHAR  chString[40];
        LOADSTRING( NULL, AfxGetResourceHandle(), ID_INDICATOR_OVR, chString );
        pCmdUI->SetText( chString );
    }
      else
    {
        pCmdUI->SetText( pDoc->chInsStatusText );
    } /* endif */
  } /* endif */
  } /* endif */
}

void CGenTEnvView::OnUpdateStatusSegNum(CCmdUI *pCmdUI)
{
  PSTEQFGEN   pstEQFGen = this->pIda->pstEQFGen;
  PTBDOCUMENT pDoc = ACCESSWNDIDA( pstEQFGen->hwndEditorTgt, PTBDOCUMENT );
  CHAR        chBuffer[30];

  if ( pDoc )
  {
    sprintf( chBuffer, "%s %3.3d", pDoc->chSegStatusText, pDoc->TBCursor.ulSegNum );
    pCmdUI->Enable();
    pCmdUI->SetText( chBuffer );
  }
}

void CGenTEnvView::OnUpdateStatusLinNum(CCmdUI *pCmdUI)
{
  PSTEQFGEN   pstEQFGen = this->pIda->pstEQFGen;
  PTBDOCUMENT pDoc = ACCESSWNDIDA( pstEQFGen->hwndEditorTgt, PTBDOCUMENT );
  CHAR        chBuffer[30];

  if ( pDoc )
  {
    sprintf( chBuffer, "%s %3.3ld", pDoc->chLineStatusText,
             (ULONG)(pDoc->ulVScroll+pDoc->lCursorRow ));
    pCmdUI->Enable();
    pCmdUI->SetText( chBuffer );
  }
}






int CGenTEnvView::OnCreate(LPCREATESTRUCT cs)
{
  int iRet = CView::OnCreate(cs);
  pIda = ACCESSWNDIDA( m_hWnd, PDOCUMENT_IDA );
  /********************************************************************/
  /* register drop target                                             */
  /********************************************************************/
  m_dropTarget.Register( this );
  return iRet;
}


BOOL CGenTEnvView::GetObjectInfo(COleDataObject* pDataObject,
                                 CSize* pSize, CSize* pOffset)
{
  ASSERT(pSize != NULL);

  // get object descriptor data
  HGLOBAL hObjDesc = pDataObject->GetGlobalData(m_cfObjectDescriptor);
  if (hObjDesc == NULL)
  {
    if (pOffset != NULL)
    {
      *pOffset = CSize(0, 0);          // fill in defaults instead
    } /* endif */

    *pSize = CSize(0, 0);
    return FALSE;
  }
  ASSERT(hObjDesc != NULL);

  // otherwise, got CF_OBJECTDESCRIPTOR ok.  Lock it down and extract size.
  LPOBJECTDESCRIPTOR pObjDesc = (LPOBJECTDESCRIPTOR)GlobalLock(hObjDesc);
  ASSERT(pObjDesc != NULL);
  pSize->cx = (int)pObjDesc->sizel.cx;
  pSize->cy = (int)pObjDesc->sizel.cy;
  if (pOffset != NULL)
  {
    pOffset->cx = (int)pObjDesc->pointl.x;
    pOffset->cy = (int)pObjDesc->pointl.y;
  }
  GlobalUnlock(hObjDesc);
  GlobalFree(hObjDesc);

  // successfully retrieved pSize & pOffset info
  return TRUE;
}

BOOL CGenTEnvView::OnDrop ( COleDataObject* pDataObject,
                            DROPEFFECT dropEffect, CPoint point )
{
  /********************************************************************/
  /* clean up focus rect                                              */
  /********************************************************************/
  ASSERT_VALID(this);
  OnDragLeave();

  // offset point as appropriate for dragging
  GetObjectInfo(pDataObject, &m_dragSize, &m_dragOffset);
  CClientDC dc(NULL);
  dc.HIMETRICtoDP(&m_dragSize);
  dc.HIMETRICtoDP(&m_dragOffset);
  point -= m_dragOffset;

  // if move within the view
  ClientToDoc(point);

  /********************************************************************/
  /* paste of item                                                    */
  /********************************************************************/
  BeginWaitCursor();

  /********************************************************************/
  /* use clipboard data if not doing a drag/drop                      */
  /********************************************************************/
  COleDataObject clipboardData;
  if ( pDataObject == NULL )
  {
    clipboardData.AttachClipboard();
    pDataObject = &clipboardData;
  } /* endif */

  if ( pDataObject->IsDataAvailable( CF_TEXT ) )
  {
    HGLOBAL hObj = pDataObject->GetGlobalData(CF_TEXT);
    if ( hObj )
    {
      PSZ_W p = (PSZ_W) GlobalLock( hObj );

      RECT rect;
      POINT pt;
      POINT pt1;
      pt.x = point.x;
      pt.y = point.y;
      ::ClientToScreen( pIda->hwnd, &pt );
      ::GetClientRect( pIda->pstEQFGen->hwndEditorTgt, &rect );
      pt1.x = rect.left; pt1.y = rect.top;
      ::ClientToScreen( pIda->pstEQFGen->hwndEditorTgt, &pt1 );

      EQFBOnDrop(pIda, pt.x - pt1.x,
                       pt.y - pt1.y, p,
                       (dropEffect == DROPEFFECT_MOVE) );

    //EQFBOnDrop( pIda, point.x, point.y, p, (dropEffect == DROPEFFECT_MOVE) );
      GlobalUnlock( hObj );
      GlobalFree( hObj );
    } /* endif */
  } /* endif */

  EndWaitCursor();
  return TRUE;
}

DROPEFFECT CGenTEnvView::OnDragEnter( COleDataObject* pDataObject,
                                      DWORD grfKeyState, CPoint point)
{
  ASSERT(m_prevDropEffect == DROPEFFECT_NONE);

  if ( !COleClientItem::CanCreateFromData( pDataObject) )
  {
    return DROPEFFECT_NONE;
  } /* endif */

  GetObjectInfo(pDataObject, &m_dragSize, &m_dragOffset);
  CClientDC dc(NULL);
  dc.HIMETRICtoDP(&m_dragSize);
  dc.HIMETRICtoDP(&m_dragOffset);

  return OnDragOver(pDataObject, grfKeyState, point);
}


DROPEFFECT CGenTEnvView::OnDragOver(  COleDataObject*,
                                      DWORD grfKeyState, CPoint point )
{
  point -= m_dragOffset;               // adjust target rect by original cursor offset


  DROPEFFECT de = DROPEFFECT_NONE;
  /********************************************************************/
  /* check if over translation window and active segment              */
  /********************************************************************/
  if ( EQFOverTransWindow( pIda, point.x, point.y ))
  {
    // check for force link
    if ((grfKeyState & (MK_CONTROL|MK_SHIFT)) == (MK_CONTROL|MK_SHIFT))
    {
      de = DROPEFFECT_NONE;        // we don't support linking ..
    }
    // check for force copy
    else if ((grfKeyState & MK_CONTROL) == MK_CONTROL)
    {
      de = DROPEFFECT_COPY;
    }
    // check for force move
    else if ((grfKeyState & MK_ALT) == MK_ALT)
    {
      de = DROPEFFECT_MOVE;
    }
    // default -- recommended action is move
    else
    {
      de = DROPEFFECT_MOVE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* chek                                                             */
  /********************************************************************/

  if (point == m_dragPoint)
    return de;

  // otherwise, cursor has moved -- need to update the drag feedback
  CClientDC dc(this);
  if (m_prevDropEffect != DROPEFFECT_NONE)
  {
    // erase previous focus rect
    dc.DrawFocusRect(CRect(m_dragPoint, m_dragSize));
  }
  m_prevDropEffect = de;
  if (m_prevDropEffect != DROPEFFECT_NONE)
  {
    m_dragPoint = point;
    dc.DrawFocusRect(CRect(point, m_dragSize));
  }

  return de;

}


void CGenTEnvView::OnDragLeave()
{
  CClientDC dc(this);
  if (m_prevDropEffect != DROPEFFECT_NONE)
  {
    dc.DrawFocusRect(CRect(m_dragPoint,m_dragSize)); // erase previous focus rect
    m_prevDropEffect = DROPEFFECT_NONE;
  } /* endif */
}


void CGenTEnvView::ClientToDoc(CRect& rect)
{
  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.DPtoLP(&rect); // convert device rect to logical rect
}

void CGenTEnvView::ClientToDoc(CSize& size)
{
  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.DPtoLP(&size); // convert device rect to logical rect
  size.cx = abs(size.cx);
  size.cy = abs(size.cy);
}

void CGenTEnvView::ClientToDoc(CPoint& point)
{
  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.DPtoLP(&point); // convert device point to logical point
}


void CGenTEnvView::DocToClient(CRect& rect)
{
  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.LPtoDP(&rect); // convert logical rect to device rect
  rect.NormalizeRect();
}


void CGenTEnvView::DocToClient(CSize& size)
{
  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.LPtoDP(&size); // convert logical size to device size
  size.cx = abs(size.cx);
  size.cy = abs(size.cy);
}

void CGenTEnvView::DocToClient(CPoint& point)
{
  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.LPtoDP(&point); // convert logical point to device point
}


VOID CGenTEnvView::OnMDIMinAll()
{
  ((CChildFrame *)GetParent())->OnMDIMinAll();
}

VOID CGenTEnvView::OnMDIRestore()
{
  ((CChildFrame *)GetParent())->OnMDIRestore();
};

LRESULT CGenTEnvView::OnInitMenuPopup( WPARAM mp1, LPARAM mp2 )
{
  /********************************************************************/
  /* the windows pulldown is the 5th menu item                        */
  /********************************************************************/
  LRESULT      mResult = FALSE;
  UINT         usID = mp2;
  CChildFrame  *pChildFrame = (CChildFrame *)GetParent();
  CMDIFrameWnd *pFrameWnd   = pChildFrame->GetMDIFrame();
  BOOL         fMaximized;
  CMDIChildWnd *pChildWnd = pFrameWnd->MDIGetActive( &fMaximized);
  pChildWnd; // avoid compiler warnings
  usID = ( fMaximized ) ? usID -1 : usID;

  switch ( usID )
  {
    case IDM_HELP:      // Help
      // do nothing; leave help and windows items active ...
      break;
    case IDM_WINDOW_MENU:      // Windows pulldown
      // use base function to build the windows pulldown ...
      mResult = pChildFrame->OnWindowsPopup( mp1, mp2 );
      break;

    default:
//       mResult = pIda->pfnCallBack( &pIda->CommArea ,
//                                    m_hWnd, WM_EQF_INITMENU, mp2, 0 );
       mResult = (DOCUMENTWP)(m_hWnd, WM_EQF_INITMENU, mp1, mp2);
//falsch       mResult = (DOCUMENTWP)(m_hWnd, WM_EQF_INITMENU, mp2, 0);
       //--- ensure that some of the items are enabled anyway ---
       UtlMenuEnableItem( PID_UTILS_MI_TAGTABLE );
       UtlMenuEnableItem( PID_UTILS_MI_LNGUPDATE );
       UtlMenuEnableItem( PID_UTILS_MI_DRIVES );
       UtlMenuEnableItem( PID_UTILS_MI_PLGINMGR );  // new start
       UtlMenuEnableItem( PID_UTILS_MI_ATOVERUP );  // new end
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




VOID CGenTEnvView::OnSize(WPARAM mp1, int cx, int cy )
{
  mp1; cx; cy;
  return;
}


void CGenTEnvView::OnDraw( CDC* pDC )
{
  pDC;
}

void CGenTEnvView::OnClose()
{
  CWnd::OnClose();
  /********************************************************************/
  /* free any pending dictionary lookup dialogs                       */
  /* This has to be done here - cleanup sequence under MFC different  */
  /* under standard                                                   */
  /********************************************************************/
  pIda = ACCESSWNDIDA( m_hWnd, PDOCUMENT_IDA );

  if ( pIda && pIda->hLUPCB )
  {
    CWnd   * pWnd;
    HWND   hwnd = LupGetLookupHandle( pIda->hLUPCB );
    PHWND  paHwnd;
    USHORT usDisplayDlgs = 0;

    if ( hwnd )
    {
      /****************************************************************/
      /* Terminate the lookup dialog and any displayed edit dialogs   */
      /****************************************************************/
      if ( WinIsWindow( (HAB)UtlQueryULong( QL_HAB ), hwnd ) )
      {
        pWnd = FromHandle( hwnd );
        ((CMDIChildWnd*)pWnd->GetParent())->MDIDestroy();
      } /* endif */
    } /* endif */

	usDisplayDlgs = LupGetDispHandles( pIda->hLUPCB, &paHwnd );
	for ( int i = 0; i < usDisplayDlgs; i++)
	{
		hwnd = paHwnd[i];
		if ( WinIsWindow( (HAB)UtlQueryULong( QL_HAB ), hwnd ) )
		{
		  pWnd = FromHandle( hwnd );
		  ((CMDIChildWnd*)pWnd->GetParent())->MDIDestroy();
		} /* endif */
	} /* endfor */
  } /* endif */

  if (::IsWindow(m_hWnd) )
    ((CMDIChildWnd*) GetParent())->MDIDestroy();
}


void CGenTEnvView::OnToolbarActivate(CCmdUI* pCmdUI)
{
  UINT usNum = pCmdUI->m_nID;
  HWND   hwnd = ::GetFocus();
  if ( hwnd )
  {
    pCmdUI->Enable( EQFBEnableToolbarItem( hwnd, (USHORT)usNum ) );
  } /* endif */
}

LRESULT CGenTEnvView::On_WM_EQF_SHOWHTML( WPARAM mp1, LPARAM mp2 )
{

  EQFSHOWDLG ShowDlg;
  strcpy(ShowDlg.chTitle, (PSZ) mp1 );
  strcpy(ShowDlg.chFileName, (PSZ) mp2 );
  ShowDlg.DoModal();
  return (FALSE);
}


void CGenTEnvView::OnFontExtDlg()
{

  CFontExtDlg FontExtDlg;
  PTBDOCUMENT pDoc;
  PSTEQFGEN   pstEQFGen = this->pIda->pstEQFGen;

  pDoc = ACCESSWNDIDA( pstEQFGen->hwndEditorTgt, PTBDOCUMENT );
  FontExtDlg.SetHwnd(this->pIda->pstEQFGen->hwndEditorTgt);

  switch ( FontExtDlg.DoModal() )
  {
    case IDOK:
      EQFBResetAllFonts(pDoc);
      break;
    default:
      break;
  } /* endswitch */
  return;
}



LRESULT CGenTEnvView::On_WM_EQF_DRAGDROP( WPARAM mp1, LPARAM mp2 )
{
  mp1;
  /********************************************************************/
  /* mp2 is pointer to document struct                                */
  /********************************************************************/
  PTBDOCUMENT pDoc = (PTBDOCUMENT) mp2;
  GLOBALHANDLE hGlobal = EQFBFuncCopyToHGlob( pDoc );
  if ( hGlobal )
  {
    BOOL fMove = FALSE;         // move not supported
    COleDataSource DragDropSrc;
        DWORD  dwEffects;
    DragDropSrc.CacheGlobalData( CF_TEXT, hGlobal );
    if ( pDoc->docType == STARGET_DOC )
    {
      PEQFBBLOCK  pBlock = (PEQFBBLOCK) pDoc->pBlockMark;

      if ( (pBlock->ulSegNum == pDoc->ulWorkSeg) &&
             (pBlock->ulEndSegNum == pDoc->ulWorkSeg) )
      {
        fMove = TRUE;
      }
      else if ( pDoc->EQFBFlags.PostEdit  )
      {
        if ( (pBlock->ulSegNum == pDoc->pTBSeg->ulSegNum) &&
               (pBlock->ulEndSegNum == pDoc->pTBSeg->ulSegNum) )
        {
          fMove = (pDoc->pTBSeg->qStatus == QF_XLATED);
        }

      } /* endif */
    } /* endif */
    dwEffects = DragDropSrc.DoDragDrop( (fMove) ? (DROPEFFECT_COPY|DROPEFFECT_MOVE) : DROPEFFECT_COPY);

  } /* endif */
  return (TRUE);
}


/////////////////////////////////////////////////////////////////////////////
// CGenTEnvView construction/destruction

CGenTEnvView::CGenTEnvView()
{
  pIda = NULL;      // init pIda
  m_prevDropEffect = DROPEFFECT_NONE;
  m_pSelection     = NULL;
  m_bInDrag        = FALSE;
  m_dragOffset     = NULL;
  OleInitialize(NULL);                   //not done in eqfdApp any more
  if ( m_cfObjectDescriptor == NULL )
  {
    m_cfObjectDescriptor = (CLIPFORMAT)::RegisterClipboardFormat(_T("Object Descriptor"));
  } /* endif */
}

CGenTEnvView::~CGenTEnvView()
{
  OleUninitialize();
}

BOOL CGenTEnvView::PreCreateWindow(CREATESTRUCT& cs)
{

  CEQFDApp * myApp = (CEQFDApp *)AfxGetApp();
  MDICREATESTRUCT *pMDI = myApp->GetMDIStruct();
  SetMDIStruct( pMDI );

  cs.lpszClass = DOCUMENT;

  return CView::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CGenTEnvView initialization

void CGenTEnvView::OnInitialUpdate()
{

  CView::OnInitialUpdate();

  RECTL           Rectl;                   // rectangle structure
  WinQueryWindowRect( EqfQueryTwbClient(), &Rectl );

  //set window position and maximize client window
  WinSetWindowPos( GetParentFrame()->m_hWnd, HWND_TOP,
                   (SHORT) RECTL_XLEFT(Rectl),
                   (SHORT) RECTL_YBOTTOM(Rectl),
                   (SHORT) (RECTL_XRIGHT(Rectl) - RECTL_XLEFT(Rectl)),
                   (SHORT) (RECTL_YTOP(Rectl) - RECTL_YBOTTOM(Rectl)),
                   EQF_SWP_ACTIVATE | EQF_SWP_SHOW | EQF_SWP_SIZE |
                   EQF_SWP_MAXIMIZE | EQF_SWP_MOVE );

}


/////////////////////////////////////////////////////////////////////////////
// CGenTEnvView diagnostics

#ifdef _DEBUG
void CGenTEnvView::AssertValid() const
{
        CView::AssertValid();
}

void CGenTEnvView::Dump(CDumpContext& dc) const
{
        CView::Dump(dc);
}

CGenTEnvDoc* CGenTEnvView::GetDocument() // non-debug version is inline
{
        ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGenTEnvDoc)));
        return (CGenTEnvDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CGenTEnvView message handlers


/////////////////////////////////////////////////////////////////////////////
// CGenTEnvDoc

IMPLEMENT_DYNCREATE(CGenTEnvDoc, CEQFDoc)

BEGIN_MESSAGE_MAP(CGenTEnvDoc, CEQFDoc)
        //{{AFX_MSG_MAP(CGenTEnvDoc)
                // NOTE - the ClassWizard will add and remove mapping macros here.
                //    DO NOT EDIT what you see in these blocks of generated code!

        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGenTEnvDoc construction/destruction

CGenTEnvDoc::CGenTEnvDoc()
{

  fUserFlag = FALSE;

}

CGenTEnvDoc::~CGenTEnvDoc()
{
}

BOOL CGenTEnvDoc::OnNewDocument()
{
  if (!CEQFDoc::OnNewDocument())
          return FALSE;

  SetDocWindowText( ((CGenTEnvView*)pActView)->GetpIda()  , NULL,
                    (char*)((CGenTEnvView*)pActView)->GetMDIStruct()->szTitle );
  SetTitle( ((CGenTEnvView*)pActView)->GetpIda()->szBuf );

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGenTEnvDoc helper functions



/////////////////////////////////////////////////////////////////////////////
// CGenTEnvDoc serialization

void CGenTEnvDoc::Serialize(CArchive& ar)
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
// CGenTEnvDoc diagnostics

#ifdef _DEBUG
void CGenTEnvDoc::AssertValid() const
{
        CEQFDoc::AssertValid();
}

void CGenTEnvDoc::Dump(CDumpContext& dc) const
{
        CEQFDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGenTEnvDoc commands



/////////////////////////////////////////////////////////////////////////////
// EQFSHOWDLG dialog


EQFSHOWDLG::EQFSHOWDLG(CWnd* pParent /*=NULL*/)
        : CDialog(EQFSHOWDLG::IDD, pParent)
{
        //{{AFX_DATA_INIT(EQFSHOWDLG)
                // NOTE: the ClassWizard will add member initialization here
        //}}AFX_DATA_INIT

}


BEGIN_MESSAGE_MAP(EQFSHOWDLG, CDialog)
        //{{AFX_MSG_MAP(EQFSHOWDLG)
        ON_WM_SIZE()
        ON_WM_DESTROY()
        ON_MESSAGE( WM_EQF_INITIALIZE,                On_WM_EQF_INITIALIZE)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EQFSHOWDLG message handlers

BOOL EQFSHOWDLG::OnInitDialog()
{
  /********************************************************************/
  /* Dialog initialisation                                            */
  /********************************************************************/
  CDialog::OnInitDialog();
  ShowWindow( SW_HIDE );

  SetWindowText( chTitle );

  pShowWnd = ((CEQFDApp *)AfxGetApp())->CreateShowWnd( this->m_hWnd, chFileName );

  if ( !pShowWnd )
  {
    CDialog::OnCancel();
  }
  else
  {
    PostMessage( WM_EQF_INITIALIZE, NULL, NULL );
  } /* endif */

  return TRUE;
}

LRESULT EQFSHOWDLG::On_WM_EQF_INITIALIZE( WPARAM mp1, LPARAM mp2 )
{
  mp1; mp2;
  /********************************************************************/
  /* restore old window positions                                     */
  /********************************************************************/
  CRect rect;
  CWinApp *pApp = AfxGetApp();
  rect.left     = pApp->GetProfileInt( TM_PREVIEW, "RectLeft"  , 0 );
  rect.top      = pApp->GetProfileInt( TM_PREVIEW, "RectTop"   , 0 );
  rect.right    = pApp->GetProfileInt( TM_PREVIEW, "RectRight" , 0 );
  rect.bottom   = pApp->GetProfileInt( TM_PREVIEW, "RectBottom", 0 );

  if ( rect.right && rect.bottom && rect.right < 2000 && rect.bottom < 2000 )
  {
    SetWindowPos( &wndTop, rect.left,  rect.top,
                           rect.right - rect.left, rect.bottom - rect.top,
                           SWP_SHOWWINDOW );
  } /* endif */
  return 0;
}

void EQFSHOWDLG::OnSize(UINT nType, int cx, int cy)
{
  nType; cx; cy;
  ((CEQFDApp *)AfxGetApp())->SizeShowWnd( this->m_hWnd, pShowWnd );

}

void EQFSHOWDLG::OnCancel()
{
  /********************************************************************/
  /* save our positions                                               */
  /********************************************************************/
  CRect rect;
  GetWindowRect( &rect );
  CWinApp * pApp = AfxGetApp();
  pApp->WriteProfileInt( TM_PREVIEW, "RectLeft", (int)rect.left );
  pApp->WriteProfileInt( TM_PREVIEW, "RectTop", (int)rect.top );
  pApp->WriteProfileInt( TM_PREVIEW, "RectRight", (int)rect.right );
  pApp->WriteProfileInt( TM_PREVIEW, "RectBottom", (int)rect.bottom );

  CDialog::OnCancel();
}




/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CRectItem, COleClientItem, 0)

CRectItem::CRectItem(COleDocument* pContainer) : COleClientItem(pContainer) ,
        m_ptPos(10, -10), m_sizeContent(0,0), m_sizeIcon(0,0),
        m_sizeContentExtent(0, 0), m_sizeIconExtent(0, 0)
{
}

CRectItem::CRectItem() : m_ptPos(10, -10), m_sizeContent(0,0), m_sizeIcon(0,0),
        m_sizeContentExtent(0, 0), m_sizeIconExtent(0, 0)
{
}

CRectItem::~CRectItem()
{
}

void CRectItem::Invalidate(CView* pNotThisView)
{
        GetDocument()->UpdateAllViews(pNotThisView, 0, this);
}

/////////////////////////////////////////////////////////////////////////////

CSize CRectItem::GetSize()
{
  DVASPECT dv = GetDrawAspect();
  if (dv == DVASPECT_ICON)
    return m_sizeIcon;
  else
    return m_sizeContent;
}

CSize CRectItem::GetBaseSize()
{
  DVASPECT dv = GetDrawAspect();
  if (dv == DVASPECT_ICON)
    return m_sizeIconExtent;
  else
    return m_sizeContentExtent;
}

void CRectItem::SetSize(CSize size)
{
  DVASPECT dv = GetDrawAspect();
  if (dv == DVASPECT_ICON)
    m_sizeIcon = size;
  else
    m_sizeContent = size;
}

void CRectItem::SetBaseSize(CSize size)
{
  DVASPECT dv = GetDrawAspect();
  if (dv == DVASPECT_ICON)
    m_sizeIconExtent = size;
  else
    m_sizeContentExtent = size;
}

void CRectItem::SetRect(CRect& rect)
{
  m_ptPos = rect.TopLeft();
  SetSize(rect.Size());
}

BOOL CRectItem::UpdateExtent()
{
  // get size in pixels
  CSize size;
  if (!GetCachedExtent(&size))
    return FALSE;       // blank
  Invalidate();   // invalidate the old size/position
  CSize sizeBase = GetBaseSize();
  if (size == sizeBase) // no change
    return FALSE;
  // if new object (i.e. m_extent is empty) setup position
  if (sizeBase == CSize(0,0))
  {
    // convert to document coords
    CSize sizeNew(MulDiv(size.cx, 10, 254), - MulDiv(size.cy, 10, 254));
    SetSize(sizeNew);
  }
  else
  {
    if (!IsInPlaceActive() && size != sizeBase)
    {
      // data changed and not inplace, so scale up rect as well
      CSize sizeCur = GetSize();
      sizeCur.cx = MulDiv(sizeCur.cx, size.cx, sizeBase.cx);
      sizeCur.cy = - MulDiv(-sizeCur.cy, size.cy, sizeBase.cy);
      SetSize(sizeCur);
    }
  }
  SetBaseSize(size);
  Invalidate();   // as well as the new size/position
  return TRUE;
}

BOOL CRectItem::OnChangeItemPosition(const CRect& rectPos)
{
  CGenTEnvView* pView = GetActiveView();
  if (pView == NULL)
    return FALSE;
  ASSERT_VALID(pView);

  CRect rc = rectPos;
  pView->ClientToDoc(rc);

  if (rc != GetRect())
  {
    // invalidate old item
    Invalidate();
    // update to new rectangle
    SetRect(rc);

    GetDocument()->SetModifiedFlag();
    CSize sizeExtent;
    GetCachedExtent(&sizeExtent);
    SetBaseSize(sizeExtent);

    // and invalidate new
    Invalidate();
  }
  return COleClientItem::OnChangeItemPosition(rectPos);
}

void CRectItem::OnActivate()
{
  // allow only one inplace active item per frame
//  CGenTEnvView* pView = GetActiveView();
//@@ASSERT_VALID(pView);
//@@COleClientItem* pItem = GetDocument()->GetInPlaceActiveItem(pView);
//@@if (pItem != NULL && pItem != this)
//@@        pItem->Close();
//@@
//@@COleClientItem::OnActivate();
//@@
//@@// set selection to an item when it becomes active
//@@pView->SetSelection(this);
}

void CRectItem::OnDeactivateUI(BOOL bUndoable)
{
  COleClientItem::OnDeactivateUI(bUndoable);

  // hide the object if it is not an outside-in object
  DWORD dwMisc = 0;
  m_lpObject->GetMiscStatus(GetDrawAspect(), &dwMisc);
  if (dwMisc & OLEMISC_INSIDEOUT)
    DoVerb(OLEIVERB_HIDE, NULL);
}

void CRectItem::OnChange(OLE_NOTIFICATION nCode, DWORD dwParam)
{
  COleClientItem::OnChange(nCode, dwParam);
  switch(nCode)
  {
    case OLE_CHANGED:
            UpdateExtent();
            Invalidate();
            break;
    case OLE_CHANGED_ASPECT:
    case OLE_CHANGED_STATE:
            Invalidate();
            break;
  }
}

void CRectItem::OnGetItemPosition(CRect& rPosition)
{
  ASSERT_VALID(this);

  if (GetSize() == CSize(0,0))
          UpdateExtent();

  // copy m_rect, which is in document coordinates
  rPosition = GetRect();
  CGenTEnvView* pView = GetActiveView();
  ASSERT_VALID(pView);
  pView->DocToClient(rPosition);
}

void CRectItem::Move(CRect &rc)
{
  // invalidate old rect
  Invalidate();
  // invalidate new
  SetRect(rc);
  Invalidate();

  // update item rect when in-place active
  if (IsInPlaceActive())
          SetItemRects();
}

void CRectItem::Serialize(CArchive& ar)
{
  CRect rect;

  // IMPORTANT: when using "easy" serialize -- call base class FIRST!
  //  (not strictly necessary, but a good idea)
  COleClientItem::Serialize(ar);

  // now store/retrieve data specific to CRectItem
  if (ar.IsStoring())
  {
          WORD w = 0x5500;        // magic value
          ar << w << m_ptPos << m_sizeIcon << m_sizeIconExtent <<
                  m_sizeContent << m_sizeContentExtent;
  }
  else
  {
    WORD w;
    ar >> w >> m_ptPos >> m_sizeIcon >> m_sizeIconExtent >>
            m_sizeContent >> m_sizeContentExtent;
    if (w != 0x5500)
    {
      TRACE0("Bad magic number in front of an item wnd\n");
      AfxThrowArchiveException(CArchiveException::generic);
    }
  }
}

void CRectItem::ResetSize()
{
  ASSERT_VALID(this);
  Invalidate();
  SetBaseSize(CSize(0, 0));
  UpdateExtent();
}

// OnGetClipboardData is used by CopyToClipboard and DoDragDrop
COleDataSource* CRectItem::OnGetClipboardData(
        BOOL bIncludeLink, LPPOINT lpOffset, LPSIZE lpSize)
{
  ASSERT_VALID(this);

  COleDataSource* pDataSource = new COleDataSource;
  TRY
  {
    GetNativeClipboardData(pDataSource);
    GetClipboardData(pDataSource, bIncludeLink, lpOffset, lpSize);
  }
  CATCH_ALL(e)
  {
    delete pDataSource;
    THROW_LAST();
  }
  END_CATCH_ALL

  ASSERT_VALID(pDataSource);
  return pDataSource;
}

void CRectItem::GetNativeClipboardData(COleDataSource* pDataSource)
{
  pDataSource;
  ASSERT_VALID(this);
  ASSERT_VALID(GetDocument());

//@@// Create a shared file and associate a CArchive with it
//@@CSharedFile file;
//@@CArchive ar(&file, CArchive::store);
//@@
//@@// Serialize selected objects to the archive
//@@Serialize(ar);
//@@ar.Close();
//@@pDataSource->CacheGlobalData(CGenTEnvDoc::m_cfPrivate, file.Detach());
}

/////////////////////////////////////////////////////////////////////////////
