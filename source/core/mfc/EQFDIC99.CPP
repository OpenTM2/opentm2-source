/*! \file
	Description: EQF Dictionary Lookup handler

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqfmfc.h"

#ifndef CPPTEST
extern "C"
{
#endif
  #define INCL_EQF_TAGTABLE            // tag table and format functions
  #define INCL_EQF_FOLDER              // folder list and document list functions
  #define INCL_EQF_ANALYSIS            // Analysis functions
  #define INCL_EQF_ASD                 // dictionary access functions (Asd...)
  #define INCL_EQF_EDITORAPI        // for WM_EQF_SHOWHTML message


/**********************************************************************/
/* ensure correct packing of our structures                           */
/**********************************************************************/
#pragma pack( push, TM2StructPacking, 1 )

  #include "eqf.h"                     // General .H for EQF
  #include "eqfstart.id"               // IDs for EQFSTARR resource
  #include "eqfutmdi.h"                // MDI for dialogs

#include "eqfstart.h"

#pragma pack( pop, TM2StructPacking )

#ifndef CPPTEST
}
#endif


#include "eqfclbox.h"                  // base class for Column listbox
#include "eqfdic99.h"                  // dictionary handler specifics
#include "eqfcfrm.h"                   // base class for MDI Childframes

#include "eqfd.h"

/**********************************************************************/
/* our class definition file                                          */
/**********************************************************************/

/////////////////////////////////////////////////////////////////////////////
// CDICProcView

IMPLEMENT_DYNCREATE(CDICProcView, CView)

BEGIN_MESSAGE_MAP(CDICProcView, CView)
        //{{AFX_MSG_MAP(CDICProcView)
        ON_WM_LBUTTONDBLCLK()
        ON_WM_CREATE()
        ON_WM_CLOSE()
        ON_WM_SIZE()
        ON_WM_CHAR()
        ON_WM_GETDLGCODE()
        ON_WM_ERASEBKGND()
        ON_WM_INITMENU()
        ON_MESSAGE( WM_SETTEXT,       On_WM_SETTEXT )
        ON_MESSAGE( WM_INITMENUPOPUP, OnInitMenuPopup )
		ON_MESSAGE( WM_SYSCOMMAND,    On_WM_SysCommand)
        ON_MESSAGE( WM_TIMER,           On_WM_TIMER )
        ON_COMMAND( LOOKUP_PUBO_CANCEL, OnPuboCancel )
        ON_COMMAND( LOOKUP_PUBO_OK,     OnPuboOK )
        ON_COMMAND( LOOKUP_PUBO_EDIT,   OnPuboEdit )
        ON_UPDATE_COMMAND_UI(SC_MAXIMIZE, OnMaximizeButton)
        //}}AFX_MSG_MAP
        ON_COMMAND_RANGE( LOOKUP_DLG, LOOKUP_DLG+50, OnAllCommand )
END_MESSAGE_MAP()


/**********************************************************************/
/* create window using our own window proc...                         */
/**********************************************************************/
int CDICProcView::OnCreate(LPCREATESTRUCT cs)
{
  SetCreateStruct( cs );
  PVOID l = cs->lpCreateParams;
  cs->lpCreateParams = (PVOID) GetMDIStruct();
  if (CWnd::OnCreate(cs) == -1)
    return -1;


  // if ok, wire in the current document
  ASSERT(m_pDocument == NULL);
  CCreateContext* pContext = (CCreateContext*)l;

  // A view should be created in a given context!
  if (pContext != NULL && pContext->m_pCurrentDoc != NULL)
  {
    pContext->m_pCurrentDoc->AddView(this);
    ASSERT(m_pDocument != NULL);
  }

  return 0;
}

/********************************************************************/
/* disable maximize button                                          */
/********************************************************************/
void CDICProcView::OnMaximizeButton(CCmdUI* pCmdUI)
{
  pCmdUI->Enable( FALSE );
}

/**********************************************************************/
/* do handling of SysCommand messages -- eat them for SC_MAXIMIZE req.*/
/**********************************************************************/
MRESULT CDICProcView::On_WM_SysCommand( UINT nID, LPARAM lParam )
{
  // indicate that we took care of the SysCommand
  MRESULT mResult = TRUE;

  if ( (nID & SC_MAXIMIZE) == SC_MAXIMIZE	)
  {
	  // ignore maximize request -- and double click on Titlebar, which is SC_MAXIMIZE | 0x0002

    // GQ: allow maximize!
	  mResult = FALSE;

  }
  else
  {
	  // should not happen, if we come through EQFCFRM.CPP ...
	  CView::OnSysCommand( nID, lParam );
	  mResult = FALSE;
  }
  return mResult;
}


VOID CDICProcView::OnSize(WPARAM mp1, int cx, int cy )
{
  CView::OnSize(mp1, cx, cy);
  ((PFNWP)(MDIDialogProc))( m_hWnd, WM_SIZE, mp1, MP2FROM2SHORT( cx, cy ));
  return;
}

void CDICProcView::OnInitMenu( CMenu *pMenu )
{
  /********************************************************************/
  /* disable maximize option                                          */
  /********************************************************************/
  pMenu->EnableMenuItem( SC_MAXIMIZE, MF_GRAYED | MF_BYCOMMAND);
}

LRESULT CDICProcView::On_WM_SETTEXT( WPARAM mp1, LPARAM mp2 )
{
	mp1;
    SetTitle((PSZ)mp2);
    return 0;
}


MRESULT CDICProcView::OnInitMenuPopup( WPARAM mp1, LPARAM mp2 )
{
  /********************************************************************/
  /* the windows pulldown is the 5th menu item                        */
  /********************************************************************/
  MRESULT      mResult = FALSE;
  UINT         usID = mp2;
  CChildFrame  *pChildFrame = (CChildFrame *)GetParent();
  CMDIFrameWnd *pFrameWnd   = pChildFrame->GetMDIFrame();
  BOOL         fMaximized;
  CMDIChildWnd *pChildWnd = pFrameWnd->MDIGetActive( &fMaximized);
  pChildWnd;  // avoid compiler warnings
  usID = ( fMaximized ) ? usID -1 : usID;

  switch ( usID )
  {
    case PID_TWBM_SM_HELP :      // Help
      // do nothing; leave help and windows items active ...

      // GQ 2015/10/13: disable some of the help items until out help system is working again
      UtlMenuDisableItem( PID_HELP_FOR_HELP );
      UtlMenuDisableItem( PID_HELP_MI_INDEX );


      break;
    case PID_TWBM_SM_WINDOWS:      // Windows pulldown
      // use base function to build the windows pulldown ...
      mResult = pChildFrame->OnWindowsPopup( mp1, mp2 );
      break;

    default:
       mResult = (MDIDialogProc)(m_hWnd, WM_EQF_INITMENU, mp2, 0 );
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



VOID CDICProcView::OnChar(UINT mp1, UINT n, UINT m )
{
  ((PFNWP)(MDIDialogProc))( m_hWnd, WM_CHAR, mp1, MP2FROM2SHORT( n, m ));
  return;
}

//UINT CDICProcView::OnGetDlgCode( WPARAM mp1, LPARAM mp2 )
UINT CDICProcView::OnGetDlgCode()
{
  return ((PFNWP)(MDIDialogProc))( m_hWnd, WM_GETDLGCODE, NULL, NULL );
//  return ((PFNWP)(MDIDialogProc))( m_hWnd, WM_GETDLGCODE, mp1, mp2 );
}

VOID CDICProcView::OnKeyDown(int mp1, int n, int m )
{
  switch ( mp1 )
  {
    case VK_TAB:
      {
        HWND hwndCombo = ::GetParent( m_hWnd );      // get combobox handle
        ::SetFocus( ::GetNextDlgTabItem( m_hWnd, hwndCombo, FALSE ));
      }
      break;
    default:
      ((PFNWP)(MDIDialogProc))( m_hWnd, WM_KEYDOWN, mp1, MP2FROM2SHORT( n, m ));
      break;
  } /* endswitch */
  return;
}


void CDICProcView::OnDraw( CDC* pDC )
{
  pDC;
  /********************************************************************/
  /* force refresh                                                    */
  /********************************************************************/
  ((PFNWP)(MDIDialogProc))( m_hWnd, WM_PAINT, NULL, NULL );
  return;
}


BOOL CDICProcView::OnEraseBackground( CDC* pDC )
{
  pDC;
  /********************************************************************/
  /* force refresh                                                    */
  /********************************************************************/
  ((PFNWP)(MDIDialogProc))( m_hWnd, WM_ERASEBKGND, NULL, NULL );
  return FALSE;
}

void CDICProcView::OnClose()
{
  ((PFNWP)(MDIDialogProc))( m_hWnd, WM_CLOSE, NULL, NULL );
  CWnd::OnClose();
  ((CMDIChildWnd*) GetParent())->MDIDestroy();
}





/////////////////////////////////////////////////////////////////////////////
// CDICProcView construction/destruction

CDICProcView::CDICProcView()
{
}

CDICProcView::~CDICProcView()
{
}

BOOL CDICProcView::PreCreateWindow(CREATESTRUCT& cs)
{

  CEQFDApp * myApp = (CEQFDApp *)AfxGetApp();
  MDICREATESTRUCT *pMDI = myApp->GetMDIStruct();
  SetMDIStruct( pMDI );

  cs.lpszClass = pMDI->szClass;
  return CView::PreCreateWindow(cs);
}
void CDICProcView::OnInitialUpdate()
{
    CView::OnInitialUpdate();
}


/////////////////////////////////////////////////////////////////////////////
// CDICProcView initialization


VOID  CDICProcView::OnProcessTask(WPARAM mp1, LPARAM mp2)
{
  ((PFNWP)(MDIDialogProc))( m_hWnd, WM_EQF_PROCESSTASK, mp1, mp2 );
  return ;
}



/////////////////////////////////////////////////////////////////////////////
// CDICProcView diagnostics

#ifdef _DEBUG
void CDICProcView::AssertValid() const
{
        CView::AssertValid();
}

void CDICProcView::Dump(CDumpContext& dc) const
{
        CView::Dump(dc);
}

CDICProcDoc* CDICProcView::GetDocument() // non-debug version is inline
{
        ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDICProcDoc)));
        return (CDICProcDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CDICProcView message handlers


/////////////////////////////////////////////////////////////////////////////
// CDICProcDoc

IMPLEMENT_DYNCREATE(CDICProcDoc, CEQFDoc)

BEGIN_MESSAGE_MAP(CDICProcDoc, CEQFDoc)
        //{{AFX_MSG_MAP(CDICProcDoc)
                // NOTE - the ClassWizard will add and remove mapping macros here.
                //    DO NOT EDIT what you see in these blocks of generated code!

        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDICProcDoc construction/destruction

CDICProcDoc::CDICProcDoc()
{

  fUserFlag = FALSE;

}

CDICProcDoc::~CDICProcDoc()
{
}

BOOL CDICProcDoc::OnNewDocument()
{
  if (!CEQFDoc::OnNewDocument())
          return FALSE;

  SetTitle( ((CDICProcView*)pActView)->GetMDIStruct()->szTitle );

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDICProcDoc helper functions



/////////////////////////////////////////////////////////////////////////////
// CDICProcDoc serialization

void CDICProcDoc::Serialize(CArchive& ar)
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
// CDICProcDoc diagnostics

#ifdef _DEBUG
void CDICProcDoc::AssertValid() const
{
        CEQFDoc::AssertValid();
}

void CDICProcDoc::Dump(CDumpContext& dc) const
{
        CEQFDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDICProcDoc commands
