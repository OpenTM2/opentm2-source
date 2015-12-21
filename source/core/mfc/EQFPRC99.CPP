/*! \file
	Description: EQF Folder Handler

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqfmfc.h"

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
  #include "eqffol00.h"
  #include "EQFHLOG.H"            // defines for history log processing
  #include "eqfcolw.id"                // column width IDs
#include "eqfstart.h"

#pragma pack( pop, TM2StructPacking )

#ifndef CPPTEST
}
#endif


#include "eqfclbox.h"   // base class for Column listbox
#include "eqfPRC99.h"
#include "eqfcfrm.h"    // child frame base class
#include "eqfd.h"

/**********************************************************************/
/* our class definition file                                          */
/**********************************************************************/

/////////////////////////////////////////////////////////////////////////////
// CGenProcView

IMPLEMENT_DYNCREATE(CGenProcView, CView)

BEGIN_MESSAGE_MAP(CGenProcView, CView)
        //{{AFX_MSG_MAP(CGenProcView)
        ON_WM_LBUTTONDBLCLK()
        ON_WM_CREATE()
        ON_MESSAGE( WM_EQF_TERMINATE,      OnEQFTerminate )

        ON_MESSAGE( WM_ACTIVATE,           On_WM_ACTIVATE )
        ON_MESSAGE( WM_CLOSE,              On_WM_CLOSE )
        ON_MESSAGE( WM_DESTROY,            On_WM_DESTROY )
        ON_MESSAGE( WM_EQFN_CREATED,       On_WM_EQFN_CREATED    )
        ON_MESSAGE( WM_EQFN_DELETED,       On_WM_EQFN_DELETED  )
        ON_MESSAGE( WM_EQFN_TASKDONE,      On_WM_EQFN_TASKDONE )
        ON_MESSAGE( WM_EQF_ABOUTTODELETE,  On_WM_EQF_ABOUTTODELETE  )
        ON_MESSAGE( WM_EQF_COMMAND,        On_WM_EQF_COMMAND )
        ON_MESSAGE( WM_EQF_DDE_REQUEST,    On_WM_EQF_DDE_REQUEST )
        ON_MESSAGE( WM_SETTEXT,            On_WM_SETTEXT )
        ON_MESSAGE( WM_EQF_INITIALIZE,     On_WM_EQF_INITIALIZE )
        ON_MESSAGE( WM_EQF_INITMENU,       On_WM_EQF_INITMENU )
        ON_MESSAGE( WM_EQF_PROCESSTASK,    On_WM_EQF_PROCESSTASK )
        ON_MESSAGE( WM_EQF_QUERYPROPSIZE,  On_WM_EQF_QUERYPROPSIZE )
        ON_MESSAGE( WM_EQF_SETFOCUS,       On_WM_EQF_SETFOCUS )
        ON_MESSAGE( WM_EQF_SHUTDOWN,       On_WM_EQF_SHUTDOWN )
        ON_MESSAGE( WM_EQF_SLIDER_SETTEXT, On_WM_EQF_SLIDER_SETTEXT )
        ON_MESSAGE( WM_EQF_UPDATESLIDER,   On_WM_EQF_UPDATESLIDER )
        ON_MESSAGE( WM_ERASEBKGND,         On_WM_ERASEBKGND )
        ON_MESSAGE( WM_ICONERASEBKGND,     On_WM_ICONERASEBKGND )
        ON_MESSAGE( WM_INITMENU,           On_WM_INITMENU )
        ON_MESSAGE( WM_INITMENUPOPUP,      On_WM_INITMENUPOPUP )
        ON_MESSAGE( WM_KEYDOWN,            On_WM_KEYDOWN )
        ON_MESSAGE( WM_MDIACTIVATE,        On_WM_MDIACTIVATE )
        ON_MESSAGE( WM_PAINT,              On_WM_PAINT )
        ON_MESSAGE( WM_QUERYDRAGICON,      On_WM_QUERYDRAGICON )
        ON_MESSAGE( WM_SETFOCUS,           On_WM_SETFOCUS )
        ON_MESSAGE( WM_SIZE,               On_WM_SIZE )
        ON_MESSAGE( WM_TIMER,              On_WM_TIMER )
        //}}AFX_MSG_MAP
        ON_COMMAND_RANGE( PID_WIND_MI_TOP, PID_WIND_MI_BOT, OnWndActivate )
        ON_COMMAND_RANGE( ID_TWBM_AAB, ID_TWBM_AAB_LAST, On_WM_COMMAND )
        ON_UPDATE_COMMAND_UI_RANGE(ID_TWBM_AAB , ID_TWBM_AAB_LAST, OnToolbarActivate)
END_MESSAGE_MAP()


int CGenProcView::OnCreate(LPCREATESTRUCT cs)
{
  PVOID p = cs->lpCreateParams;
  cs->lpCreateParams = GetMDIStruct();
  (GENERICPROCESSWP)(m_hWnd, WM_CREATE, NULL, (LPARAM) cs );
  cs->lpszClass = GetMDIStruct()->szClass;
  cs->lpCreateParams = p;
  return CView::OnCreate(cs);
}


LRESULT CGenProcView::OnEQFTerminate( WPARAM mp1, LPARAM mp2)
{
  /**************************************************************/
  /* Call instance callback function to fill the column         */
  /* listbox                                                    */
  /**************************************************************/
  LRESULT mResult = GENERICPROCESSWP( m_hWnd, WM_EQF_TERMINATE, mp1, mp2 );
  /****************************************************************/
  /* Terminate only if callback function has returned FALSE       */
  /* (= continue termination)                                     */
  /****************************************************************/
  if ( mResult == FALSE )
  {
    ((CMDIChildWnd*)GetParent())->MDIDestroy();
  } /* endif */
  return (LRESULT)TRUE;
}

void CGenProcView::OnWndActivate( UINT mp1 )
{
  CChildFrame  *pChildFrame = (CChildFrame *)GetParent();
  pChildFrame->OnToggleWindow( mp1 );
}



void CGenProcView::OnToolbarActivate(CCmdUI* pCmdUI)
{
  UINT   usID = pCmdUI->m_nID;
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
      case PID_FILE_MI_SYSPROP:
      case PID_FILE_MI_EXIT:
      case PID_UTILS_MI_MT:
        /****************************************************************/
        /* enable above items all the time..                            */
        /****************************************************************/
        fEnable = TRUE;
        break;
      default:
        fEnable = pIda->pfnCallBack( &pIda->CommArea ,
                                     m_hWnd, WM_EQF_TOOLBAR_ENABLED, usID, FALSE);
        break;
    } /* endswitch */
  } /* endif */
  pCmdUI->Enable( fEnable );
}

LRESULT CGenProcView::On_WM_INITMENUPOPUP( WPARAM mp1, LPARAM mp2 )
{
  LRESULT      mResult = FALSE;
  UINT         usID = mp2;
  CChildFrame  *pChildFrame = (CChildFrame *)GetParent();
  CMDIFrameWnd *pFrameWnd   = pChildFrame->GetMDIFrame();
  BOOL         fMaximized;
  CMDIChildWnd *pChildWnd = pFrameWnd->MDIGetActive( &fMaximized);
  pChildWnd;  // avoid compiler warnings
  usID = ( fMaximized ) ? usID -1 : usID;

  switch ( usID )
  {
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

    default:
       mResult = pIda->pfnCallBack( &pIda->CommArea ,
                                    m_hWnd, WM_EQF_INITMENU, mp2, 0 );
       //--- ensure that some of the items are enabled anyway ---
       UtlMenuEnableItem( PID_UTILS_MI_TAGTABLE );
       UtlMenuEnableItem( PID_UTILS_MI_LNGUPDATE );
       UtlMenuEnableItem( PID_UTILS_MI_PLGINMGR);
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

LRESULT CGenProcView::On_WM_SETTEXT( WPARAM mp1, LPARAM mp2 )
{
  mp1;
  ((CEQFDoc*)GetDocument())->SetTitle( (PSZ)mp2 );
  return 0;
}



/////////////////////////////////////////////////////////////////////////////
// CGenProcView construction/destruction

CGenProcView::CGenProcView()
{
  pIda = NULL;      // init pIda
}

CGenProcView::~CGenProcView()
{
}

void CGenProcView::OnDraw( CDC* pDC )
{
  pDC;
}

BOOL CGenProcView::PreCreateWindow(CREATESTRUCT& cs)
{
  CEQFDApp * myApp = (CEQFDApp *)AfxGetApp();
  MDICREATESTRUCT *pMDI = myApp->GetMDIStruct();
  SetMDIStruct( pMDI );
  cs.lpszName = pMDI->szTitle;

  return CView::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CGenProcView initialization

void CGenProcView::OnInitialUpdate()
{
  CView::OnInitialUpdate();

  pIda = ACCESSWNDIDA( m_hWnd, PGENPROCESSINSTIDA );
  pIda->hFrame = GetParentFrame()->m_hWnd;       // store frame window

  SetWindowText( (PSZ)pIda->CommArea.szTitle );
  ((CEQFDoc*)GetDocument())->SetTitle( (PSZ) pIda->CommArea.szTitle );
  // set the icon
  GetParentFrame()->SetIcon( pIda->CommArea.hIcon, FALSE );

  //--- set process window postion and size ---
  if ( !pIda->fVisible )
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

}

/////////////////////////////////////////////////////////////////////////////
// CGenProcView diagnostics

#ifdef _DEBUG
void CGenProcView::AssertValid() const
{
        CView::AssertValid();
}

void CGenProcView::Dump(CDumpContext& dc) const
{
        CView::Dump(dc);
}

CGenProcDoc* CGenProcView::GetDocument() // non-debug version is inline
{
        ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGenProcDoc)));
        return (CGenProcDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CGenProcView message handlers


/////////////////////////////////////////////////////////////////////////////
// CGenProcDoc

IMPLEMENT_DYNCREATE(CGenProcDoc, CEQFDoc)

BEGIN_MESSAGE_MAP(CGenProcDoc, CEQFDoc)
        //{{AFX_MSG_MAP(CGenProcDoc)
                // NOTE - the ClassWizard will add and remove mapping macros here.
                //    DO NOT EDIT what you see in these blocks of generated code!

        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGenProcDoc construction/destruction

CGenProcDoc::CGenProcDoc()
{

  fUserFlag = FALSE;

}

CGenProcDoc::~CGenProcDoc()
{
}

BOOL CGenProcDoc::OnNewDocument()
{
  if (!CEQFDoc::OnNewDocument())
          return FALSE;

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGenProcDoc helper functions



/////////////////////////////////////////////////////////////////////////////
// CGenProcDoc serialization

void CGenProcDoc::Serialize(CArchive& ar)
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
// CGenProcDoc diagnostics

#ifdef _DEBUG
void CGenProcDoc::AssertValid() const
{
        CEQFDoc::AssertValid();
}

void CGenProcDoc::Dump(CDumpContext& dc) const
{
        CEQFDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGenProcDoc commands
