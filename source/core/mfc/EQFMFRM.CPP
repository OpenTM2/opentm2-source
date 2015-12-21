/*! \file
	Description: Interface of the CMainFrame class

	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqfmfc.h"
#include "EQFD.h"
#include "EQF.ID"
#include "EQFB.ID"
#include "eqfstart.id"

/**********************************************************************/
/* private prototypes                                                 */
/**********************************************************************/
#ifndef CPPTEST
extern "C"
{
#endif
/**********************************************************************/
/* ensure correct packing of our structures                           */
/**********************************************************************/
#pragma pack( push, TM2StructPacking, 1 )
  #define INCL_EQF_EDITORAPI        // for WM_EQF_SHOWHTML message
  #include "eqf.h"
  #include "eqfstart.h"
  #include "eqfutmdi.h"
#pragma pack( pop, TM2StructPacking )

__declspec(dllexport)
  VOID    TwbWinInitCS( CREATESTRUCT * pCS );
#ifndef CPPTEST
}
#endif

#include <dde.h>                     // DDE defines
#include "eqfmFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
        //{{AFX_MSG_MAP(CMainFrame)
        ON_MESSAGE( WM_MDIGETACTIVE, OnMDIGetActive )
        ON_MESSAGE( WM_MDIRESTORE,   OnMDIRestore )
        ON_MESSAGE( WM_MDIMAXIMIZE,  OnMDIMaximize )
        ON_MESSAGE( WM_MDISETMENU,   OnMDISetMenu )
        ON_WM_PARENTNOTIFY()
        ON_WM_CREATE()
        ON_WM_CLOSE()
        ON_WM_INITMENUPOPUP()
        ON_MESSAGE( WM_QUERYENDSESSION,       On_WM_QUERYENDSESSION)
        ON_MESSAGE( WM_ENDSESSION,            On_WM_ENDSESSION)
        ON_MESSAGE( WM_EQF_INITIALIZE,        OnEQFInit )
        ON_MESSAGE( WM_MDICREATE,             OnMDICreate )
        ON_MESSAGE( WM_MDIACTIVATE,           OnMDIActivate )
        ON_MESSAGE( WM_EQF_INITMENU,          OnEQFInitMenu )

        ON_MESSAGE( WM_EQF_SHUTDOWN,          On_WM_EQF_SHUTDOWN)
        ON_MESSAGE( WM_EQFN_SHUTDOWNCANCELED, On_WM_EQFN_SHUTDOWNCANCELED)
        ON_MESSAGE( WM_EQFN_OBJECTREMOVED,    On_WM_EQFN_OBJECTREMOVED)
        ON_MESSAGE( WM_EQF_ACTIVATEINSTANCE,  On_WM_EQF_ACTIVATEINSTANCE )
        ON_MESSAGE( WM_DDE_INITIATE,          On_WM_DDE_INITIATE)
        ON_MESSAGE( WM_DESTROY,               On_WM_DESTROY )

        ON_MESSAGE( WM_COPYDATA,  On_WM_COPYDATA )

        ON_MESSAGE( HM_HELPSUBITEM_NOT_FOUND, On_HM_HELPSUBITEM_NOT_FOUND )
        ON_MESSAGE( WM_EQF_SHOWHTML,          On_WM_EQF_SHOWHTML )
        ON_MESSAGE( WM_MDITILE,               OnMDITile )
        ON_MESSAGE( WM_MDICASCADE,            OnMDICascade)
        ON_MESSAGE( WM_EQF_MDIMINALL,             OnMDIMinall)
        ON_MESSAGE( WM_EQF_MDIRESTOREALL,         OnMDIRestoreall)
        //}}AFX_MSG_MAP

END_MESSAGE_MAP()

static UINT indicators[] =
{
        ID_SEPARATOR,           // status line indicator
        ID_INDICATOR_CAPS,
        ID_INDICATOR_NUM,
        ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction



CMainFrame::CMainFrame()
{
  // TODO: add member initialization code here
  fTENVMaximized = FALSE;
}

CMainFrame::~CMainFrame()
{
}

MRESULT CMainFrame::On_HM_HELPSUBITEM_NOT_FOUND( WPARAM mp1, LPARAM mp2 )
{
  return ((PFNWP)(TwbMainWP))( MDIGetActive( NULL )->m_hWnd,
                                HM_HELPSUBITEM_NOT_FOUND, mp1, mp2 );
}


MRESULT CMainFrame::OnMDIGetActive( WPARAM mp1, LPARAM mp2 )
{
  mp1;
  CMDIChildWnd * pWnd;
  pWnd = MDIGetActive( (PBOOL) mp2 );

  return (MRESULT) pWnd->m_hWnd;
}

MRESULT CMainFrame::OnMDIMaximize( WPARAM mp1, LPARAM mp2 )
{
  mp1; mp2;
  CMDIChildWnd * pWnd;

  pWnd = MDIGetActive( NULL );
  pWnd->MDIMaximize();

  return (MRESULT) 0;
}



MRESULT CMainFrame::OnMDIRestore( WPARAM mp1, LPARAM mp2 )
{
  mp2;
  if ( !mp1)
  {
    CMDIChildWnd * pWnd;
    pWnd = MDIGetActive( NULL );
    MDIRestore( pWnd );
  }
  else
  {
    CWnd * pWnd ;
    pWnd = FromHandle((HWND)mp1);
    pWnd = pWnd->GetParent();
    MDIRestore( pWnd );
  }
  return (MRESULT) 0;
}

MRESULT CMainFrame::OnMDISetMenu( WPARAM mp1, LPARAM mp2 )
{
  return (MRESULT)
  ::SendMessage( m_hWndMDIClient, WM_MDISETMENU, mp1, mp2 );
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  USHORT       usHideToolBars;

  if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
          return -1;

  if (!m_wndTWBToolBar.CreateEx(this, TBSTYLE_FLAT,
           WS_CHILD|WS_VISIBLE|CBRS_TOP| CBRS_GRIPPER | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
           CRect(0,0,0,0), ID_TWBM_WINDOW) ||
      !m_wndTWBToolBar.LoadToolBar(ID_TWBM_WINDOW))
  {
          TRACE0("Failed to create toolbar\n");
          return -1;      // fail to create
  }


  if (!m_wndTPROToolBar.CreateEx(this, TBSTYLE_FLAT,
           WS_CHILD|WS_VISIBLE|CBRS_TOP| CBRS_GRIPPER | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
           CRect(0,0,0,0), ID_TP_MAIN_WINDOW) ||
          !m_wndTPROToolBar.LoadToolBar(ID_TP_MAIN_WINDOW))
  {
          TRACE0("Failed to create toolbar\n");
          return -1;      // fail to create
  }

  if (!m_wndStatusBar.Create(this) ||
          !m_wndStatusBar.SetIndicators(indicators,
            sizeof(indicators)/sizeof(UINT)))
  {
          TRACE0("Failed to create status bar\n");
          return -1;                   // fail to create
  }
  else
  {
    UINT nId, nStyle;
    INT nWidth;
    m_wndStatusBar.GetPaneInfo( 2, nId, nStyle, nWidth );
    nWidth = 60;  //KBT1072: segnumber 65345 not displ.correctly
    m_wndStatusBar.SetPaneInfo( 2, nId, nStyle, nWidth );

    m_wndStatusBar.GetPaneInfo( 3, nId, nStyle, nWidth );
    nWidth = 54;
    m_wndStatusBar.SetPaneInfo( 3, nId, nStyle, nWidth );
  }

  CString s;
  s.LoadString( SID_TWB_TOOLBAR );
  m_wndTWBToolBar.SetWindowText( s );
  s.LoadString( SID_TPRO_TOOLBAR );
  m_wndTPROToolBar.SetWindowText( s );
  m_wndTWBToolBar.EnableDocking(CBRS_ALIGN_ANY);
  m_wndTPROToolBar.EnableDocking(CBRS_ALIGN_ANY);
  EnableDocking(CBRS_ALIGN_ANY);

  DockControlBar(&m_wndTWBToolBar);
  DockControlBar(&m_wndTPROToolBar);

  LoadBarState("EqfStart");
  m_wndTWBToolBar.SetBarStyle(m_wndTWBToolBar.GetBarStyle() |
          CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
  m_wndTPROToolBar.SetBarStyle(m_wndTPROToolBar.GetBarStyle() |
          CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

  usHideToolBars = UtlQueryUShort( QS_HIDETOOLBARS );
  if ( (usHideToolBars==1) || (usHideToolBars==3) ) 
     m_wndTWBToolBar.ShowWindow( SW_HIDE );
  else
     m_wndTWBToolBar.ShowWindow( SW_SHOW );
  if ( (usHideToolBars==2) || (usHideToolBars==3) ) 
     m_wndTPROToolBar.ShowWindow( SW_HIDE );
  else
     m_wndTPROToolBar.ShowWindow( SW_SHOW );
	  
  return 0;
}


void CMainFrame::OnCheckToolBar()
{
  /********************************************************************/
  /* update menu accordingly                                          */
  /********************************************************************/
  CMenu        *pPopupMenu = GetMenu();
  /***************************************************************/
  /* get View pulldown                                           */
  /***************************************************************/
  BOOL         fMaximized;
  MDIGetActive( &fMaximized);

  USHORT usID = ( fMaximized ) ? PID_TWBM_SM_VIEW + 1 : PID_TWBM_SM_VIEW;
  pPopupMenu = pPopupMenu->GetSubMenu( usID );
  if ( pPopupMenu )
  {
#ifndef _TQM
    pPopupMenu->CheckMenuItem( PID_VIEW_TWB_TOOLBAR, MF_BYCOMMAND |
             ((m_wndTWBToolBar.IsWindowVisible()) ? MF_CHECKED : MF_UNCHECKED));
    pPopupMenu->CheckMenuItem( PID_VIEW_TPRO_TOOLBAR, MF_BYCOMMAND |
             ((m_wndTPROToolBar.IsWindowVisible()) ? MF_CHECKED : MF_UNCHECKED));
#endif
    pPopupMenu->CheckMenuItem( PID_VIEW_MI_STATUSBAR , MF_BYCOMMAND |
             ((m_wndStatusBar.IsWindowVisible()) ? MF_CHECKED : MF_UNCHECKED));
  } /* endif */

}

void CMainFrame::OnToggleToolBar( USHORT usID )
{
  CFrameWnd *  pFrameWnd;
  PPROPSYSTEM  pPropSys;
  USHORT       usHideToolBars;
  switch ( usID )
  {
    case PID_VIEW_TWB_TOOLBAR:
      pFrameWnd = m_wndTWBToolBar.GetDockingFrame();
      usHideToolBars = UtlQueryUShort( QS_HIDETOOLBARS );
      if (m_wndTWBToolBar.IsWindowVisible())
      {
        if ( pFrameWnd != this)
        {
          pFrameWnd->ShowWindow( SW_HIDE );
        } /* endif */
        m_wndTWBToolBar.ShowWindow( SW_HIDE );
        if ( usHideToolBars == 0 ) 
           usHideToolBars = 1 ;
        if ( usHideToolBars == 2 ) 
           usHideToolBars = 3 ;
      }
      else
      {
        m_wndTWBToolBar.ShowWindow( SW_SHOW );
        if ( pFrameWnd != this )
        {
          CRect rect;
          m_wndTWBToolBar.GetWindowRect( &rect );
          pFrameWnd->SetWindowPos( &wndTop,
                                   rect.left, rect.top, rect.right, rect.bottom,
                                   SWP_SHOWWINDOW );
        } /* endif */
        if ( usHideToolBars == 1 ) 
           usHideToolBars = 0 ;
        if ( usHideToolBars == 3 ) 
           usHideToolBars = 2 ;
      }
      UtlSetUShort( QS_HIDETOOLBARS, usHideToolBars );
      pPropSys = GetSystemPropPtr();
      pPropSys->usHideToolBars = usHideToolBars ;
      break;
    case PID_VIEW_TPRO_TOOLBAR:
      pFrameWnd = m_wndTPROToolBar.GetDockingFrame();
      usHideToolBars = UtlQueryUShort( QS_HIDETOOLBARS );
      if (m_wndTPROToolBar.IsWindowVisible())
      {
        if ( pFrameWnd != this)
        {
          pFrameWnd->ShowWindow( SW_HIDE );
        } /* endif */
        m_wndTPROToolBar.ShowWindow( SW_HIDE );
        if ( usHideToolBars == 0 ) 
           usHideToolBars = 2 ;
        if ( usHideToolBars == 1 ) 
           usHideToolBars = 3 ;
      }
      else
      {
        m_wndTPROToolBar.ShowWindow( SW_SHOW );
        if ( pFrameWnd != this )
        {
          CRect rect;
          m_wndTPROToolBar.GetWindowRect( &rect );
          pFrameWnd->SetWindowPos( &wndTop,
                                   rect.left, rect.top, rect.right, rect.bottom,
                                   SWP_SHOWWINDOW );
        } /* endif */
        if ( usHideToolBars == 2 ) 
           usHideToolBars = 0 ;
        if ( usHideToolBars == 3 ) 
           usHideToolBars = 1 ;
      }
      UtlSetUShort( QS_HIDETOOLBARS, usHideToolBars );
      pPropSys = GetSystemPropPtr();
      pPropSys->usHideToolBars = usHideToolBars ;
      break;
    case PID_VIEW_MI_STATUSBAR :
      pFrameWnd = m_wndStatusBar.GetDockingFrame();
      if (m_wndStatusBar.IsWindowVisible())
      {
        if ( pFrameWnd != this)
        {
          pFrameWnd->ShowWindow( SW_HIDE );
        } /* endif */
        m_wndStatusBar.ShowWindow( SW_HIDE );
      }
      else
      {
        m_wndStatusBar.ShowWindow( SW_SHOW );
        if ( pFrameWnd != this )
        {
          CRect rect;
          m_wndStatusBar.GetWindowRect( &rect );
          pFrameWnd->SetWindowPos( &wndTop,
                                   rect.left, rect.top, rect.right, rect.bottom,
                                   SWP_SHOWWINDOW );
        } /* endif */
      }
      break;
  } /* endswitch */
  RecalcLayout();
}


VOID CMainFrame::OnClose()
{
  /****************************************************************/
  /* save toolbar states                                          */
  /****************************************************************/
  SaveBarState("EqfStart");

  /********************************************************************/
  /* save our positions                                               */
  /********************************************************************/
  CRect rect;
  WINDOWPLACEMENT WindowPlacement;
  GetWindowPlacement( &WindowPlacement );
  rect = WindowPlacement.rcNormalPosition;

  CWinApp * pApp = AfxGetApp();
  pApp->WriteProfileInt( EQFWORKBENCH, "RectLeft", (int)rect.left );
  pApp->WriteProfileInt( EQFWORKBENCH, "RectTop", (int)rect.top );
  pApp->WriteProfileInt( EQFWORKBENCH, "RectRight", (int)rect.right );
  pApp->WriteProfileInt( EQFWORKBENCH, "RectBottom", (int)rect.bottom );
  pApp->WriteProfileInt( EQFWORKBENCH, "SWFlags", (int)WindowPlacement.showCmd);
  pApp->WriteProfileInt( EQFWORKBENCH, "MinX", (int) WindowPlacement.ptMinPosition.x );
  pApp->WriteProfileInt( EQFWORKBENCH, "MinY", (int) WindowPlacement.ptMinPosition.y );
  pApp->WriteProfileInt( EQFWORKBENCH, "MaxX", (int) WindowPlacement.ptMaxPosition.x );
  pApp->WriteProfileInt( EQFWORKBENCH, "MaxY", (int) WindowPlacement.ptMaxPosition.y );

  TwbMainWP( m_hWnd, WM_CLOSE, NULL, NULL );
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  // TODO: Modify the Window class or styles here by modifying
  //  the CREATESTRUCT cs
  TwbWinInitCS( &cs );

  return CMDIFrameWnd::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
        CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
        CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


MRESULT CMainFrame::OnMDICreate( WPARAM mp1, LPARAM mp2)
{
  mp1;
  HWND hwnd = NULL;
  MDICREATESTRUCT *pMDICreate = (MDICREATESTRUCT*) mp2;

  if (pMDICreate )
  {
    CEQFDApp * myApp = (CEQFDApp *)AfxGetApp();
    if ( strcmp( pMDICreate->szClass, GENERICLIST ) == 0 )
    {
      if ( strncmp( pMDICreate->szTitle, REPORTHANDLER, strlen(REPORTHANDLER)) == 0 )
      {
        /**************************************************************/
        /* create an HTML window for the report handler               */
        /**************************************************************/
        hwnd = myApp->CreateList( pMDICreate, 5 );
      }
      else
      {
        // start separate folder/subfolder handler or use normal mode
        {
          PPROPSYSTEM pPropSys = GetSystemPropPtr();
          if ( pPropSys->fUseIELikeListWindows &&
               (strncmp( &pMDICreate->szTitle[7], "FLIST", 5 ) == 0) )
          {
            hwnd = myApp->CreateList( pMDICreate, 7 );
            EqfIELookAndFeel(TRUE);     // set IE Look and Feel
          }
          else
          {
            hwnd = myApp->CreateList( pMDICreate, 1 );
          } /* endif */
        }
      } /* endif */
    }
    else
    if ( strcmp( pMDICreate->szClass, GENERICPROCESS ) == 0 )
    {
      hwnd = myApp->CreateList( pMDICreate, 2 );
    }
    else
    if ( strcmp( pMDICreate->szClass, DOCUMENT ) == 0 )
    {
      hwnd = myApp->CreateList( pMDICreate, 3 );
    }
    else
    if ( strcmp( pMDICreate->szClass, TMM_CLASS ) == 0 )
    {
      hwnd = myApp->CreateList( pMDICreate, 4 );
    }
    else
    if ( strcmp( pMDICreate->szClass, WC_EQFMDIDLG ) == 0 )
    {
      // restore current MDI window if maximized or the MFC frame work will trap...
      BOOL         fMaximized;
      CMDIChildWnd* pChildWnd = MDIGetActive( &fMaximized);
      if ( fMaximized )
      {
        pChildWnd->MDIRestore();
      } /* endif */

      hwnd = myApp->CreateList( pMDICreate, 6 );
    }
    else
    {
      char chText[200];
      sprintf( chText, "@@ not yet implemented: %s", pMDICreate->szClass );
      AfxMessageBox( chText );
    } /* endif */
  } /* endif */
  return (MRESULT)hwnd;
}

MRESULT CMainFrame::OnMDIActivate( WPARAM mp1, LPARAM mp2)
{
  mp2;
  HWND hwndChild = (HWND)mp1;
  CWnd * pCWnd = FromHandle( hwndChild );
  MDIActivate( pCWnd->GetParent() );
  return (MRESULT)TRUE;

}

MRESULT CMainFrame::OnMDITile( WPARAM mp1, LPARAM mp2)
{
  mp1; mp2;
  MDITile(MDITILE_HORIZONTAL);
  return (MRESULT)TRUE;

}

MRESULT CMainFrame::OnMDICascade( WPARAM mp1, LPARAM mp2)
{
  mp1; mp2;
  MDICascade();

  return (MRESULT)TRUE;

}

MRESULT CMainFrame::OnMDIMinall(WPARAM mp1, LPARAM mp2 )
{
  HWND hwndFrame =  (HWND) UtlQueryULong(QL_TWBFRAME );

  PTWBMAIN_IDA  pIda = ACCESSWNDIDA( hwndFrame, PTWBMAIN_IDA );
  mp1; mp2;
  if ( pIda )
  {
    POBJLST  pObject = pIda->hwndsInMenuWindows;
    USHORT   usI     = pIda->usInMenuWindows;
    HWND     hwndObj;

    for ( ; usI; usI--, pObject++ )
    {
       if ( IDFROMWINDOW( pObject->hwnd ) == FID_CLIENT )
       {
           // a standard window ...
           hwndObj = GETPARENT( pObject->hwnd );
       }
       else
       {
           // a dialog window ...
           hwndObj = pObject->hwnd;
       } /* endif */

       if ( (pObject->usClassID == clsFOLDER) || (pObject->usClassID == clsFOLDERLIST) )
       {
         PPROPSYSTEM pPropSys = GetSystemPropPtr();
         if ( pPropSys->fUseIELikeListWindows )
         {
           hwndObj = GETPARENT( hwndObj );
         }
       }


       CWnd * pWnd = FromHandle(hwndObj)->GetParent();

       pWnd->ShowWindow( SW_MINIMIZE );
    } /* endfor */
  } /* endif */

  return (MRESULT)TRUE;
}


MRESULT CMainFrame:: On_WM_COPYDATA( WPARAM mp1, LPARAM mp2)
{
  //::MessageBox( NULL, "receive message", "TranslationManager", MB_CANCEL );
  PCOPYDATASTRUCT	pMyCDS = (PCOPYDATASTRUCT) mp2;
  // used to notify read queue for shared translation memory webservice based
  if(pMyCDS->dwData==0)
  {
      (USHORT)EqfSend2Handler( MEMORYHANDLER, WM_EQF_READINQUE, mp1,MP2FROMP(pMyCDS->lpData));
  }
  else if(pMyCDS->dwData==1)
  {  // refresh memory list when created memory not from OpenTM2 GUI
     (USHORT)EqfSend2Handler( MEMORYHANDLER, WM_EQF_ADDMEMTOLIST, mp1,MP2FROMP(pMyCDS->lpData)); 
  }
   else if(pMyCDS->dwData==2)
  {  // refresh memory list when created memory not from OpenTM2 GUI
     (USHORT)EqfSend2Handler( MEMORYHANDLER, WM_EQF_REMOVEMEMFROMLIST, mp1,MP2FROMP(pMyCDS->lpData)); 
  }
  return (MRESULT)TRUE;
}


MRESULT CMainFrame::OnMDIRestoreall( WPARAM mp1, LPARAM mp2 )
{
  mp2; mp1;

  CMDIChildWnd * pWnd;
  CMDIChildWnd * pStartWnd;
  CMDIFrameWnd *pFrameWnd;


  pStartWnd = MDIGetActive( NULL );
  pWnd = MDIGetActive( NULL );
  pFrameWnd = pStartWnd->GetMDIFrame();

   do
    {
       pWnd->MDIRestore();
       pFrameWnd->MDINext();
       pWnd = MDIGetActive();
    } while ( pWnd && pWnd != pStartWnd ); /* enddo */

  return (MRESULT) 0;
}


MRESULT CMainFrame::OnEQFInit( WPARAM mp1, LPARAM mp2)
{
  mp1; mp2;
  return (MRESULT)TRUE;
}

VOID CMainFrame::OnInitMenuPopup( CMenu *pMenu, UINT uIndex, BOOL fSysMenu )
{
  BOOL bMaximized;
  CMDIChildWnd * pMDIChild = MDIGetActive( &bMaximized );
  if ( pMDIChild )
  {
    HMENU hMenu = pMenu->GetSafeHmenu();
    ::SendMessage( pMDIChild->m_hWnd,
                   WM_INITMENUPOPUP,
                   (WPARAM) hMenu,
                   MAKELPARAM( uIndex, fSysMenu ) );
#ifdef _TQM
  }
  else
  {
    HMENU hMenu = pMenu->GetSafeHmenu();
    // send to TQM list handler to handle initmenupopup
    UtlMenuDisableAll( this->m_hWnd, uIndex );
    EqfSend2Handler( TQMLISTHANDLER, WM_EQF_COMMAND, (WPARAM)WM_INITMENU,
                     MAKELPARAM( uIndex, fSysMenu ) );
#endif
  } /* endif */
  return;
}


MRESULT CMainFrame::OnEQFInitMenu( WPARAM mp1, LPARAM mp2 )
{
  ::SendMessage( GetActiveView()->m_hWnd, WM_EQF_INITMENU, mp1, mp2 );
  return 0;
}


void CMainFrame::OnParentNotify( UINT message, LPARAM mp2 )
{
  if ( message == WM_RBUTTONDOWN )
  {
    CHAR chBuffer[80];
    POINT pt;
    pt.x = LOWORD( mp2 );
    pt.y = HIWORD( mp2 );

    ClientToScreen( &pt );
    CWnd * pWnd = WindowFromPoint( pt );
    ::GetClassName( pWnd->m_hWnd, chBuffer, sizeof( chBuffer ) );
//    if ( strcmp( chBuffer, "MDIClient") == 0 )
//    {
//      // insert the generic popup here
//      int i = 0;
//    } /* endif */
  }
  else
  {
    CWnd::OnParentNotify( message, mp2 );
  } /* endif */
  return;
}

/**********************************************************************/
/* update statusbar fields                                            */
/**********************************************************************/
VOID CMainFrame::OnSetStatusBar( USHORT usId, PSZ pData  )
{
  m_wndStatusBar.SetPaneText( usId, pData, TRUE);
}

