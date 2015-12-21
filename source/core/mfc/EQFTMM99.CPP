/*! \file
	Description: EQF TranslationMemory maintanance handler

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
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_TM               // general Transl. Memory functions
  #define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
  #define INCL_EQF_EDITORAPI        // editor API
  #define INCL_EQF_TP               // public translation processor definitions

/**********************************************************************/
/* ensure correct packing of our structures                           */
/**********************************************************************/
#pragma pack( push, TM2StructPacking, 1 )

  #include "eqf.h"                     // General .H for EQF

  #undef   INIT_TABLES
  #include "EQFTPI.H"               // private translation processor definitions

  #define  INIT_TABLES  1
  #include "EQFTMM.H"              // TMM include file
  #include "EQFTMM.id"             // TMM dialog include file
  #include "eqfb.id"

#pragma pack( pop, TM2StructPacking )

#ifndef CPPTEST
}
#endif


#include "eqfclbox.h"   // base class for Column listbox
#include "eqfTMM99.h"
#include "eqfcfrm.h"    // case class for child frames
#include "eqfd.h"

/**********************************************************************/
/* our class definition file                                          */
/**********************************************************************/

MRESULT APIENTRY EQFSHOWHTML( HWND, WINMSG, WPARAM, LPARAM );

/////////////////////////////////////////////////////////////////////////////
// CGenTMMView

IMPLEMENT_DYNCREATE(CGenTMMView, CView)

BEGIN_MESSAGE_MAP(CGenTMMView, CView)
        //{{AFX_MSG_MAP(CGenTMMView)
        ON_WM_LBUTTONDBLCLK()
        ON_WM_CREATE()
        ON_WM_SIZE()
        ON_WM_MOVE()
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
        ON_MESSAGE( WM_EQF_COM_INITIALIZE,            On_WM_EQF_COM_INITIALIZE)
        ON_MESSAGE( WM_EQF_COM_EXIT_COMPLETE,         On_WM_EQF_COM_EXIT_COMPLETE)
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
        ON_MESSAGE( WM_EQF_IS_TENV ,                  On_WM_EQF_IS_TENV )

        ON_COMMAND(PID_WIND_MI_CASCADE ,       OnMDICascade )
        ON_COMMAND(PID_WIND_MI_TILE ,          OnMDITile )
        ON_COMMAND(PID_WIND_MI_MINALL,         OnMDIMinAll )
        ON_COMMAND(PID_WIND_MI_RESTOREALL,     OnMDIRestore )
        //}}AFX_MSG_MAP
        ON_COMMAND_RANGE( ID_TMM_WINDOW, ID_TMM_LASTAAB, OnAllCommand )
END_MESSAGE_MAP()


int CGenTMMView::OnCreate(LPCREATESTRUCT cs)
{
  PVOID p = cs->lpCreateParams;
  cs->lpCreateParams = GetMDIStruct();
  (TMMWNDPROC)(m_hWnd, WM_CREATE, NULL, (LPARAM) cs );
  cs->lpszClass = GetMDIStruct()->szClass;
  cs->lpCreateParams = p;
  return CView::OnCreate(cs);
}

VOID CGenTMMView::OnMDIMinAll()
{
  ((CChildFrame *)GetParent())->OnMDIMinAll();
}

VOID CGenTMMView::OnMDIRestore()
{
  ((CChildFrame *)GetParent())->OnMDIRestore();
};



VOID CGenTMMView::OnSize(WPARAM mp1, int cx, int cy )
{
  (TMMWNDPROC)(m_hWnd, WM_SIZE, mp1, MAKELONG( cx, cy ));
  CView::OnSize( mp1, cx, cy );
  return;
}

VOID CGenTMMView::OnMove(int cx, int cy )
{
  CView::OnMove( cx, cy );
  return;
}

MRESULT CGenTMMView::OnInitMenuPopup( WPARAM mp1, LPARAM mp2 )
{
  /********************************************************************/
  /* the windows pulldown is the 5th menu item                        */
  /********************************************************************/
  MRESULT      mResult = FALSE;
  UINT         usID = (USHORT) mp2;
  CChildFrame  *pChildFrame = (CChildFrame *)GetParent();
  CMDIFrameWnd *pFrameWnd   = pChildFrame->GetMDIFrame();
  BOOL         fMaximized;
  CMDIChildWnd *pChildWnd = pFrameWnd->MDIGetActive( &fMaximized);
  pChildWnd;  // avoid compiler warnings
  usID = ( fMaximized ) ? usID -1 : usID;

  switch ( usID )
  {
    case 5:      // Help
      // do nothing; leave help and windows items active ...
      break;
    case 4:      // Windows pulldown
      // use base function to build the windows pulldown ...
      mResult = pChildFrame->OnWindowsPopup( mp1, mp2 );
      break;

    default:
       mResult = (TMMWNDPROC)(m_hWnd, WM_EQF_INITMENU, mp1, usID );
       //--- ensure that some of the items are enabled anyway ---
       UtlMenuEnableItem( PID_UTILS_MI_TAGTABLE );
       UtlMenuEnableItem( PID_UTILS_MI_LNGUPDATE );
       UtlMenuEnableItem( PID_UTILS_MI_DRIVES );
       UtlMenuEnableItem( PID_UTILS_MI_PLGINMGR );
       UtlMenuEnableItem( PID_UTILS_MI_ATOVERUP );
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


void CGenTMMView::OnDraw( CDC* pDC )
{
  pDC;
}

void CGenTMMView::OnClose()
{
  ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_CLOSE, NULL, NULL );
  ((CMDIChildWnd*) GetParent())->MDIDestroy();
}


/////////////////////////////////////////////////////////////////////////////
// CGenTMMView construction/destruction

CGenTMMView::CGenTMMView()
{
}

CGenTMMView::~CGenTMMView()
{
}

BOOL CGenTMMView::PreCreateWindow(CREATESTRUCT& cs)
{

  CEQFDApp * myApp = (CEQFDApp *)AfxGetApp();
  MDICREATESTRUCT *pMDI = myApp->GetMDIStruct();
  SetMDIStruct( pMDI );
  cs.lpszName = pMDI->szTitle;

  return CView::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CGenTMMView initialization

void CGenTMMView::OnInitialUpdate()
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
// CGenTMMView diagnostics

#ifdef _DEBUG
void CGenTMMView::AssertValid() const
{
        CView::AssertValid();
}

void CGenTMMView::Dump(CDumpContext& dc) const
{
        CView::Dump(dc);
}

CGenTMMDoc* CGenTMMView::GetDocument() // non-debug version is inline
{
        ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGenTMMDoc)));
        return (CGenTMMDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CGenTMMView message handlers


/////////////////////////////////////////////////////////////////////////////
// CGenTMMDoc

IMPLEMENT_DYNCREATE(CGenTMMDoc, CEQFDoc)

BEGIN_MESSAGE_MAP(CGenTMMDoc, CEQFDoc)
        //{{AFX_MSG_MAP(CGenTMMDoc)
                // NOTE - the ClassWizard will add and remove mapping macros here.
                //    DO NOT EDIT what you see in these blocks of generated code!

        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGenTMMDoc construction/destruction

CGenTMMDoc::CGenTMMDoc()
{

}

CGenTMMDoc::~CGenTMMDoc()
{
}

BOOL CGenTMMDoc::OnNewDocument()
{
  if (!CEQFDoc::OnNewDocument())
          return FALSE;

  SetTitle( (char*)((CGenTMMView*)pActView)->GetMDIStruct()->szTitle );

  return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// CGenTMMDoc helper functions



/////////////////////////////////////////////////////////////////////////////
// CGenTMMDoc serialization

void CGenTMMDoc::Serialize(CArchive& ar)
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
// CGenTMMDoc diagnostics

#ifdef _DEBUG
void CGenTMMDoc::AssertValid() const
{
        CEQFDoc::AssertValid();
}

void CGenTMMDoc::Dump(CDumpContext& dc) const
{
        CEQFDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGenTMMDoc commands



