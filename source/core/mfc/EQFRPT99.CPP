/*! \file
	Description: EQF Report Handler

	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqfmfc.h"
#include "eqfrpt.id"

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
  #include "EQFHLOG.H"            // defines for history log processing
  #include "eqfcolw.id"                // column width IDs
  #include "eqfrpt00.h"             // report handler stuff
  #include "eqfstart.h"
  #include "eqfrpt.h"

#pragma pack( pop, TM2StructPacking )

#ifndef CPPTEST
}
#endif


#include "eqfclbox.h"   // base class for Column listbox
#include "eqfd.h"
#include "eqfrpt99.h"   // report handler specifics
#include "eqfcfrm.h"    // child frame base class
#include <dde.h>        // DDE defines
#include "eqfmfrm.h"    // main frame base class

/**********************************************************************/
/* our class definition file                                          */
/**********************************************************************/

/////////////////////////////////////////////////////////////////////////////
// CRPTProcView

IMPLEMENT_DYNCREATE(CRPTProcView, CView)

BEGIN_MESSAGE_MAP(CRPTProcView, CView)
        //{{AFX_MSG_MAP(CRPTProcView)
       //         ON_WM_LBUTTONDBLCLK() //ON_WM_LBUTTONDBLCLK()

                ON_WM_CREATE()
                ON_WM_SIZE()
                ON_WM_MOVE()

                ON_MESSAGE( WM_CLOSE,         OnClose )
                ON_MESSAGE( WM_EQF_INITIALIZE,OnEQFInit )
                ON_MESSAGE( WM_EQF_SETFOCUS,  On_WM_EQF_SETFOCUS )
                ON_MESSAGE( WM_EQF_TERMINATE, OnEQFTerminate )

                ON_MESSAGE( WM_DESTROY,       On_WM_DESTROY )
                ON_MESSAGE( WM_EQF_INITMENU,  On_WM_EQF_INITMENU )
                ON_MESSAGE( WM_INITMENU,      On_WM_INITMENU )
                ON_MESSAGE( WM_INITMENUPOPUP, On_WM_INITMENUPOPUP )
                ON_MESSAGE( WM_EQF_PROCESSTASK,On_WM_EQF_PROCESSTASK )
                ON_MESSAGE( WM_TIMER,         On_WM_TIMER )

                ON_COMMAND(PID_WIND_MI_CASCADE ,       OnMDICascade )
                ON_COMMAND(PID_WIND_MI_TILE ,          OnMDITile )
                ON_COMMAND(PID_WIND_MI_MINALL,         OnMDIMinAll )
                ON_COMMAND(PID_WIND_MI_RESTOREALL,     OnMDIRestore )
                ON_COMMAND(PID_VIEW_TPRO_TOOLBAR,      OnViewTPROToolBar)
                ON_COMMAND(PID_VIEW_MI_STATUSBAR,      OnViewStatusBar)
                ON_COMMAND(PID_VIEW_TWB_TOOLBAR,       OnViewTWBToolBar)

                ON_LBN_DBLCLK(DID_CALCRPT_OUT_LB,      OnDocSelect )
                //}}AFX_MSG_MAP
                ON_COMMAND_RANGE( PID_WIND_MI_TOP, PID_WIND_MI_BOT, OnWndActivate )
                ON_COMMAND_RANGE( ID_TWBM_AAB, ID_TWBM_AAB_LAST, On_WM_COMMAND )

                ON_UPDATE_COMMAND_UI_RANGE(ID_TWBM_AAB , ID_TWBM_AAB_LAST, OnToolbarActivate)

END_MESSAGE_MAP()


int CRPTProcView::OnCreate(LPCREATESTRUCT cs)
{
  /********************************************************************/
  /* call standard initialisation                                     */
  /********************************************************************/
  pIda->pfnCallBack( &pIda->CommArea , m_hWnd, WM_CREATE, NULL,
                     (LPARAM) pIda->pvUserData  );

  return CView::OnCreate(cs);
}

VOID CRPTProcView::OnSize(WPARAM mp1, int cx, int cy )
{
  mp1; cx; cy;

  PRPT pRPT = (PRPT) pIda->CommArea.pUserIDA;

  if ( pRPT && (pRPT->usReport == COMBINED_REPORT) )
  {
    // adjust size of our innner window
     CRect rect;
     GetClientRect( &rect );
     CWnd *lb = GetDlgItem( DID_CALCRPT_OUT_LB );
     if ( lb ) lb->SetWindowPos( &wndTop, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER |  SWP_SHOWWINDOW );
  }
  else
  {
    ((CEQFDApp *)AfxGetApp())->SizeShowWnd( this->m_hWnd, pCWeb );
  } /* endif */

  UtlSaveWindowPos( pIda->hFrame, &(pIda->CommArea.swpSizePos) );
}


void CRPTProcView::OnMove(int cx, int cy)
{
  cx; cy;
  UtlSaveWindowPos( pIda->hFrame, &(pIda->CommArea.swpSizePos) );
}

void CRPTProcView::OnDraw( CDC* pDC )
{
  pDC;
  /********************************************************************/
  /* force refresh                                                    */
  /********************************************************************/
  ((CEQFDApp *)AfxGetApp())->RefreshShowWnd( pCWeb );
}

// set focus to HTML control (if any)
MRESULT CRPTProcView::On_WM_EQF_SETFOCUS( WPARAM mp1, LPARAM mp2 )
{
   mp1; mp2;
  if ( this->pIda )
  {
    PRPT pRPT = (PRPT) this->pIda->CommArea.pUserIDA;

    if ( pRPT && !pRPT->fRptLB )
    {
      if ( this->pCWeb )
      {
        CWnd *pCwnd = (CWnd *)this->pCWeb;
        pCwnd->SetFocus();
      } /* endif */
    } /* endif */
  } /* endif */
  return( 0L );
}


LRESULT CRPTProcView::OnEQFInit( WPARAM mp1, LPARAM mp2)
{
  /**************************************************************/
  /* Call instance callback function                            */
  /**************************************************************/
  HWND hwnd = GetParent()->m_hWnd;
  ANCHORWNDIDA( hwnd, pIda );
  pIda->pfnCallBack( &pIda->CommArea , hwnd, WM_EQF_INITIALIZE, mp1, mp2 );
  if (IsWindow( m_hWnd ))
  {
    ((CEQFDoc*)GetDocument())->SetTitle( (PSZ) pIda->CommArea.szTitle );
  } /* endif */

  return (LRESULT)TRUE;
}

LRESULT CRPTProcView::On_WM_EQF_PROCESSTASK( WPARAM mp1, LPARAM mp2)
{
  MRESULT mResult = TRUE;

  // check if task is to be processed here if not pass to callback function
  if ( mp1 == OPEN_AND_POSITION_TASK )
  {
    // navigate to document containing the report
    CHAR chHTMLFile[MAX_EQF_PATH];
    PRPT pRPT = (PRPT) pIda->CommArea.pUserIDA;
    sprintf( chHTMLFile, "%s\\RPT.HTML", pRPT->szFolderObjName );

    // to display the new file we destroy the web control and create a new one
    ((CEQFDApp *)AfxGetApp())->DestroyShowWnd( pCWeb );
    pCWeb = NULL;

    UtlDispatch();
    Sleep( 200 );
    UtlDispatch();

    pCWeb = ((CEQFDApp *)AfxGetApp())->CreateShowWnd( this->m_hWnd, chHTMLFile );

    UtlDispatch();
    Sleep( 200 );
    UtlDispatch();


    ((CEQFDApp *)AfxGetApp())->RefreshShowWnd( pCWeb );

    UtlDispatch();
  }
  else
  {
    pIda->pfnCallBack( &pIda->CommArea , m_hWnd, WM_EQF_PROCESSTASK, mp1, mp2 );
  } /* endif */
  return mResult;
}

MRESULT CRPTProcView::OnClose( WPARAM mp1, LPARAM mp2 )
{
  MRESULT mResult;

  /********************************************************************/
  /* free our Web control...                                          */
  /********************************************************************/
  ((CEQFDApp *)AfxGetApp())->DestroyShowWnd( pCWeb );
  pCWeb = NULL;

  /********************************************************************/
  /* close our  HTML control file                                     */
  /********************************************************************/
  RptOutputToHTMLControlClose( (PRPT) pIda->CommArea.pUserIDA );

  mResult = pIda->pfnCallBack( &pIda->CommArea ,
                               m_hWnd, WM_CLOSE, mp1, mp2 );

  EqfRemoveObject( TWBFORCE, m_hWnd);
  return mResult;
}

LRESULT CRPTProcView::OnEQFTerminate( WPARAM mp1, LPARAM mp2)
{
  /**************************************************************/
  /* Call instance callback function to fill the column         */
  /* listbox                                                                */
  /**************************************************************/
  MRESULT mResult;
  mResult = pIda->pfnCallBack( &pIda->CommArea , m_hWnd, WM_EQF_TERMINATE,
                               mp1, mp2 );

  /********************************************************************/
  /* close our  HTML control file                                     */
  /********************************************************************/
  RptOutputToHTMLControlClose( (PRPT) pIda->CommArea.pUserIDA );
  /********************************************************************/
  /* free our Web control...                                          */
  /********************************************************************/
  ((CEQFDApp *)AfxGetApp())->DestroyShowWnd( pCWeb );
  /****************************************************************/
  /* Terminate only if callback function has returned FALSE       */
  /* (= continue termination)                                     */
  /****************************************************************/
  if ( mResult == FALSE )
  {
      EqfActivateInstance( m_hWnd, FALSE );
    ((CMDIChildWnd*)GetParent())->MDIDestroy();
  } /* endif */
  return mResult;
}

/**********************************************************************/
/* force as default option an open on double click                    */
/**********************************************************************/
//VOID CRPTProcView::OnLButtonDblClk(UINT uFlags, CPoint Point )
//{
//   uFlags; Point;
//   SHORT      sItem = QUERYSELECTIONHWND( pIda->CommArea.hwndLB );
//
//    if ( (sItem != LIT_NONE) && (pIda->CommArea.sDefaultAction != 0) )
//    {
//       (*pIda->pfnCallBack)( &pIda->CommArea,
//                                  m_hWnd,
//                                  WM_EQF_COMMAND,
//                                  MP1FROMSHORT(pIda->CommArea.sDefaultAction),
//                                  MP2FROMP(NULL) );
//    } /* endif */
//    return;
//}

void CRPTProcView::OnDocSelect( )
{
	SHORT      sItem = QUERYSELECTIONHWND( pIda->CommArea.hwndLB );

    if ( (sItem != LIT_NONE) && (pIda->CommArea.sDefaultAction != 0) )
    {
		(*pIda->pfnCallBack)( &pIda->CommArea,
                                  m_hWnd,
                                  WM_EQF_COMMAND,
                                  MP1FROMSHORT(pIda->CommArea.sDefaultAction),
                                  MP2FROMP(NULL) );
    } /* endif */


}

void CRPTProcView::OnWndActivate( UINT mp1 )
{
  CChildFrame  *pChildFrame = (CChildFrame *)GetParent();
  pChildFrame->OnToggleWindow( mp1 );
}

void CRPTProcView::OnViewTWBToolBar()
{
    CChildFrame  *pChildFrame = (CChildFrame *)GetParent();
    CMainFrame  *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();
    pFrameWnd->OnToggleToolBar( PID_VIEW_TWB_TOOLBAR );
};

void CRPTProcView::OnViewTPROToolBar()
{
    CChildFrame  *pChildFrame = (CChildFrame *)GetParent();
    CMainFrame  *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();
    pFrameWnd->OnToggleToolBar( PID_VIEW_TPRO_TOOLBAR );
};

void CRPTProcView::OnViewStatusBar()
{
    CChildFrame  *pChildFrame = (CChildFrame *)GetParent();
    CMainFrame  *pFrameWnd   = (CMainFrame  *)pChildFrame->GetMDIFrame();
    pFrameWnd->OnToggleToolBar( PID_VIEW_MI_STATUSBAR );
};

void CRPTProcView::OnToolbarActivate(CCmdUI* pCmdUI)
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
      case PID_FILE_MI_SYSPROP:
      case PID_FILE_MI_EXIT:
      case PID_UTILS_MI_MT :
      case PID_FILE_MI_OPEN:
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




MRESULT CRPTProcView::On_WM_INITMENUPOPUP( WPARAM mp1, LPARAM mp2 )
{
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
      mResult = pChildFrame->OnWindowsPopup( mp1, mp2 );
      break;

    default:
       mResult = pIda->pfnCallBack( &pIda->CommArea ,
                                    m_hWnd, WM_EQF_INITMENU, mp2, 0 );
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
       UtlMenuEnableItem( PID_FILE_MI_OPEN );
       UtlMenuEnableItem( PID_WIND_MI_MINALL );
       UtlMenuEnableItem( PID_WIND_MI_RESTOREALL );
       UtlMenuEnableItem( PID_WIND_MI_TILE );
       UtlMenuEnableItem( PID_WIND_MI_CASCADE );
       break;
  } /* endswitch */
  return mResult;
}

VOID CRPTProcView::OnMDIMinAll()
{
  ((CChildFrame *)GetParent())->OnMDIMinAll();
}

VOID CRPTProcView::OnMDIRestore()
{
  ((CChildFrame *)GetParent())->OnMDIRestore();
};




/////////////////////////////////////////////////////////////////////////////
// CRPTProcView construction/destruction

CRPTProcView::CRPTProcView()
{
  pIda = NULL;       // init pIda
  pCWeb = NULL;
}

CRPTProcView::~CRPTProcView()
{
}

BOOL CRPTProcView::PreCreateWindow(CREATESTRUCT& cs)
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

  cs.style |= LVS_SHOWSELALWAYS | LVS_REPORT;

  return CView::PreCreateWindow(cs);
}




/////////////////////////////////////////////////////////////////////////////
// CRPTProcView initialization
/////////////////////////////////////////////////////////////////////////////



void CRPTProcView::OnInitialUpdate()
{

  CView::OnInitialUpdate();

  pIda->hFrame = GetParentFrame()->m_hWnd;       // store frame window


  // set the icon
  GetParentFrame()->SetIcon( pIda->CommArea.hIcon, FALSE );
  SetWindowText( (PSZ)pIda->CommArea.szTitle );

  //--- set process window postion and size ---
    WinSetWindowPos( pIda->hFrame, HWND_TOP,
            pIda->CommArea.swpSizePos.x,  pIda->CommArea.swpSizePos.y,
            pIda->CommArea.swpSizePos.cx,
            pIda->CommArea.swpSizePos.cy,
            (USHORT)(pIda->CommArea.swpSizePos.fs | EQF_SWP_ACTIVATE |
            EQF_SWP_SHOW | EQF_SWP_SIZE | EQF_SWP_MOVE));

   /*******************************************************************/
   /* get pointer to RPT and create a stream for our HTML output contr*/
   /*******************************************************************/
   PRPT pRPT = (PRPT) pIda->CommArea.pUserIDA;


   if (pRPT->usReport == COMBINED_REPORT)
   {
     CRect rect;
     GetClientRect( &rect );
     pRPT->hwndLB =
     pIda->CommArea.hwndLB =
                  WinCreateWindow( this->m_hWnd, WC_LISTBOX, "",
                          WS_VISIBLE | WS_CHILD | LBS_STANDARD | LBS_NOTIFY,
                          0, 0,
                          rect.right - rect.left,
                          rect.bottom - rect.top,
                          this->m_hWnd, HWND_TOP,
                          DID_CALCRPT_OUT_LB,
                          NULL, NULL );

     pRPT->fRptLB = true;


   }
   else
   {


     /*******************************************************************/
     /* create file for Reporting                                       */
     /*******************************************************************/

     RptOutputToHTMLControlOpen( pRPT );

    CHAR chHTMLFile[MAX_EQF_PATH];
//     sprintf( chHTMLFile, "%s\\RPT.HTML", pRPT->szFolderObjName );
    sprintf( chHTMLFile, "%s\\TEMP.HTML", pRPT->szFolderObjName );

    // create temporary dummy file
    {
      FILE *hf = fopen( chHTMLFile, "w" );
      fprintf( hf, "%s", "<html>\n<body>\n<p><b>preparing report...</b></p>\n</body>\n</html>" );
      fclose( hf );
    }

     pCWeb = ((CEQFDApp *)AfxGetApp())->CreateShowWnd( this->m_hWnd, chHTMLFile );
     UtlDispatch();
     Sleep( 200 );
     UtlDispatch();

     ((CEQFDApp *)AfxGetApp())->RefreshShowWnd( pCWeb );

     this->PostMessage( WM_EQF_SETFOCUS, 0L, 0L );

     if ( !pCWeb )
     {
        RptOutputToHTMLControlClose( pRPT );
     } /* endif */

   } /* end if */


}


/////////////////////////////////////////////////////////////////////////////
// CRPTProcView diagnostics

#ifdef _DEBUG
void CRPTProcView::AssertValid() const
{
        CView::AssertValid();
}

void CRPTProcView::Dump(CDumpContext& dc) const
{
        CView::Dump(dc);
}

CRPTProcDoc* CRPTProcView::GetDocument() // non-debug version is inline
{
        ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRPTProcDoc)));
        return (CRPTProcDoc*)m_pDocument;
}
#endif //_DEBUG



/////////////////////////////////////////////////////////////////////////////
// CRPTProcDoc

IMPLEMENT_DYNCREATE(CRPTProcDoc, CEQFDoc)

BEGIN_MESSAGE_MAP(CRPTProcDoc, CEQFDoc)
        //{{AFX_MSG_MAP(CRPTProcDoc)
                // NOTE - the ClassWizard will add and remove mapping macros here.
                //    DO NOT EDIT what you see in these blocks of generated code!

        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRPTProcDoc construction/destruction

CRPTProcDoc::CRPTProcDoc()
{

  fUserFlag = FALSE;

}

CRPTProcDoc::~CRPTProcDoc()
{
}

BOOL CRPTProcDoc::OnNewDocument()
{
  if (!CEQFDoc::OnNewDocument())
          return FALSE;

  SetTitle( ((CRPTProcView*)pActView)->GetMDIStruct()->szTitle );

  EqfRegisterObject( GetObjName(), (pActView->m_hWnd),
                     ((CRPTProcView*)pActView)->pIda->CommArea.sListObjClass );

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRPTProcDoc helper functions



/////////////////////////////////////////////////////////////////////////////
// CRPTProcDoc serialization

void CRPTProcDoc::Serialize(CArchive& ar)
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
// CRPTProcDoc diagnostics

#ifdef _DEBUG
void CRPTProcDoc::AssertValid() const
{
        CEQFDoc::AssertValid();
}

void CRPTProcDoc::Dump(CDumpContext& dc) const
{
        CEQFDoc::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CGenListView message handlers






/////////////////////////////////////////////////////////////////////////////
// CRPTProcDoc
