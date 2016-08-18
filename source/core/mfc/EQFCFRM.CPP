/*! \file
	Description: Interface of the CChildFrame class

	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqfmfc.h"
#include "EQFD.h"

#ifndef CPPTEST
extern "C"
{
#endif
/**********************************************************************/
/* ensure correct packing of our structures                           */
/**********************************************************************/
#pragma pack( push, TM2StructPacking, 1 )
  #define INCL_EQF_EDITORAPI        // for WM_EQF_SHOWHTML message
  #include "eqf.h"                     // General .H for EQF
  #include "eqfstart.h"             // Help support


#pragma pack( pop, TM2StructPacking )

#ifndef CPPTEST
}
#endif

#include <dde.h>                     // DDE defines
#include "eqfmfrm.h"
#include "eqfcFrm.h"
#include "eqfclbox.h"   // base class for Column listbox
#include "eqfgen99.h"
#include "eqffll99.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
        //{{AFX_MSG_MAP(CChildFrame)
        ON_WM_MDIACTIVATE()
        ON_MESSAGE( WM_CLOSE,         On_WM_CLOSE )
        ON_WM_SIZE()
        ON_WM_SYSCOMMAND()
        ON_WM_MOVE()
        ON_MESSAGE( WM_INITMENU,      OnInitMenu )
        ON_MESSAGE( WM_INITMENUPOPUP, OnInitMenuPopup )
//        ON_MESSAGE( WM_SYSCHAR,       On_WM_SYSCHAR     )
//        ON_MESSAGE( WM_SYSKEYDOWN,    On_WM_SYSKEYDOWN  )
//        ON_MESSAGE( WM_SYSDEADCHAR,   On_WM_SYSDEADCHAR )
        ON_MESSAGE( WM_TIMER,         On_WM_TIMER )

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
        ON_MESSAGE( LM_EQF_SETITEMSTATE,              On_LM_EQF_SETITEMSTATE)
        ON_MESSAGE( LM_EQF_QUERYITEMSTATE,            On_LM_EQF_QUERYITEMSTATE)
        ON_MESSAGE( LM_EQF_SETITEMTEXT,               On_LM_EQF_SETITEMTEXT)
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

        ON_MESSAGE( WM_EQF_SHOWHTML,                  On_WM_EQF_SHOWHTML )

        ON_MESSAGE( LB_RESETCONTENT,                  On_LB_RESETCONTENT)
        ON_MESSAGE( LB_ADDSTRING,                     On_LB_ADDSTRING)
        ON_MESSAGE( LB_GETCURSEL,                     On_LB_GETCURSEL)
        ON_MESSAGE( LB_GETITEMDATA,                   On_LB_GETITEMDATA)
        ON_MESSAGE( LB_GETTEXT,                       On_LB_GETTEXT)
        ON_MESSAGE( LB_FINDSTRING,                    On_LB_FINDSTRING)
        ON_MESSAGE( LB_GETCOUNT,                      On_LB_GETCOUNT)
        ON_MESSAGE( LB_GETSELCOUNT,                   On_LB_GETSELCOUNT)

        ON_COMMAND(PID_WIND_MI_CASCADE ,              OnCascade )
        ON_COMMAND(PID_WIND_MI_TILE ,                 OnTile )
        ON_COMMAND(PID_WIND_MI_MINALL,                OnMinAll )
        ON_COMMAND(PID_WIND_MI_RESTOREALL,            OnRestore )

        //}}AFX_MSG_MAP
        ON_COMMAND_RANGE( PID_WIND_MI_TOP, PID_WIND_MI_BOT, OnWndActivate )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
  // add member initialization code here
  hObjFocus  = NULL;
  usObjClass = 0;

}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::OnCreateClient( LPCREATESTRUCT lpcs,
                                 CCreateContext* pContext)
{
   BOOL fOK = TRUE;
   const char * p = ((CRuntimeClass *)(pContext->m_pNewViewClass))->m_lpszClassName;

#ifndef _TQM
   if (strcmp( p, CFLLPROCVIEW ) == 0 )
   {
     /********************************************************************/
     /* create a splitter view for our new folder list window            */
     /********************************************************************/
     if ( m_wndSplitter.CreateStatic( this, 1,2 ) )
     {
	   int         cx = 200;
	   PPROPSYSTEM pPropSys = GetSystemPropPtr();
	   if (pPropSys->sLeftPaneWidth)			// avoid setting cx=0
	       cx = pPropSys->sLeftPaneWidth;

       if (!m_wndSplitter.CreateView( 0,0, pContext->m_pNewViewClass,
                                     CSize(cx, cx), pContext ) ||
           !m_wndSplitter.CreateView( 0,1, RUNTIME_CLASS( CFLLRightPaneView ),
                                     CSize(100, 100), pContext  ) )
       {
         m_wndSplitter.DestroyWindow();
         fOK = FALSE;
       }
       SetActiveView((CView*)m_wndSplitter.GetPane(0,1));
     }
     else
     {
       fOK = FALSE;
     }
   }
   else
#endif
     fOK = CFrameWnd::OnCreateClient( lpcs, pContext );

   return fOK;
}



VOID CChildFrame::OnMDIMinAll( )
{
  CMDIFrameWnd *pFrameWnd = GetMDIFrame();

  PTWBMAIN_IDA  pIda = ACCESSWNDIDA( pFrameWnd->m_hWnd, PTWBMAIN_IDA );
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

  return;

//  CMDIChildWnd *pChildWnd = pFrameWnd->MDIGetActive();
//  CMDIChildWnd *pTempWnd;
//  pTempWnd = pChildWnd;
//  do
//  {
//    /******************************************************************/
//    /* MDI behaves in the way, that minimizing a window will toggle   */
//    /* activation..., but if minimized, active window will stay the   */
//    /* same..                                                         */
//    /******************************************************************/
//     pChildWnd->ShowWindow( SW_MINIMIZE );
//     CMDIChildWnd *pChildWnd1 = pFrameWnd->MDIGetActive();
//     if ( pChildWnd == pChildWnd1 )
//     {
//       pFrameWnd->MDINext();
//       pChildWnd = pFrameWnd->MDIGetActive();
//     }
//     else
//     {
//       pChildWnd = pChildWnd1;
//     } /* endif */
//  } while ( pChildWnd && pTempWnd != pChildWnd ); /* enddo */
}


VOID CChildFrame::OnMDIRestore()
{
  CMDIFrameWnd *pFrameWnd = GetMDIFrame();
  CMDIChildWnd *pChildWnd = pFrameWnd->MDIGetActive();
  CMDIChildWnd *pTempWnd;
  pTempWnd = pChildWnd;
  do
  {
     pChildWnd->MDIRestore();
     pFrameWnd->MDINext();
     pChildWnd = pFrameWnd->MDIGetActive();
  } while ( pChildWnd && pTempWnd != pChildWnd ); /* enddo */
 };

VOID CChildFrame::OnToggleWindow( UINT mp1 )
{
  CMDIFrameWnd *pFrameWnd = GetMDIFrame();
  PTWBMAIN_IDA  pIda = ACCESSWNDIDA( pFrameWnd->m_hWnd, PTWBMAIN_IDA );
  USHORT        usI;
  POBJLST       pObject;             // pointer for object list processing
  /*****************************************************/
  /* Get index into window list                        */
  /* Be sure to enclose PID_WIND_MI_NEXT in () as      */
  /* the value of PID_WIND_MI_NEXT is of the form      */
  /* basevalue + offset!                               */
  /*****************************************************/
  if ( mp1 >= (PID_WIND_MI_NEXT ))
  {
    usI = (USHORT)(mp1 - (PID_WIND_MI_NEXT));

    /*****************************************************/
    /* Get window handle                                 */
    /*****************************************************/
    pObject = pIda->hwndsInMenuWindows + usI;

    /******************************************************************/
    /* activate new MDI window                                        */
    /******************************************************************/
    CWnd * pWndActiveOld = pFrameWnd->MDIGetActive();
    pFrameWnd->MDIActivate( FromHandle( pObject->hwnd ) );
    CWnd * pWndActive = pFrameWnd->MDIGetActive() ;
    if ( pWndActive == pWndActiveOld )
    {
      /****************************************************************/
      /* we came from a client window - so try with frame again       */
      /****************************************************************/
      pFrameWnd->MDIActivate( FromHandle( ::GetParent(pObject->hwnd) ) );
      pWndActive = pFrameWnd->MDIGetActive();
      if ( pWndActive == pWndActiveOld )
      {
        /****************************************************************/
        /* we came from a client window - so try with frame again       */
        /****************************************************************/
        pFrameWnd->MDIActivate( FromHandle( ::GetParent(::GetParent(pObject->hwnd)) ) );
        pWndActive = pFrameWnd->MDIGetActive();
      }
    } /* endif */

    /********************************************************************/
    /* restore window                                                   */
    /********************************************************************/
    if ( pWndActive->IsIconic() )
    {
      pFrameWnd->MDIRestore( pWndActive );
    } /* endif */
  }
  else
  {
  } /* endif */
}



MRESULT CChildFrame::OnWindowsPopup( WPARAM mp1, LPARAM mp2 )
{
  mp1;
  MRESULT mResult = FALSE;

  /***************************************************************/
  /* find separator and add all available MDI windows            */
  /* add checkmark on active one.                                */
  /***************************************************************/
  CMDIFrameWnd *pFrameWnd = GetMDIFrame();
  CMDIChildWnd *pChildWnd = pFrameWnd->MDIGetActive();
  CMenu        *pPopupMenu = pFrameWnd->GetMenu();

  /***************************************************************/
  /* get Windows pulldown                                        */
  /***************************************************************/
  pPopupMenu = pPopupMenu->GetSubMenu( mp2 );
  TwbInitMenuMFC( pPopupMenu->m_hMenu,
                  (LONG) pFrameWnd->m_hWnd ,
                  (LONG) pChildWnd->m_hWnd );
  return mResult;
}


MRESULT CChildFrame::On_WM_CLOSE ( WPARAM mp1, LPARAM mp2 )
{
  MRESULT mResult = FALSE;
  if (IsWindow( GetActiveView()->m_hWnd ))
  {
    mResult = ::SendMessage( GetActiveView()->m_hWnd, WM_CLOSE, mp1, mp2 );
  } /* endif */

  return mResult;
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{

  CEQFDApp * myApp = (CEQFDApp *)AfxGetApp();
  MDICREATESTRUCT *pMDI = myApp->GetMDIStruct();

  cs.x  = pMDI->x;
  cs.y  = pMDI->y;
  cs.cx = pMDI->cx;
  cs.cy = pMDI->cy;
  cs.lpszClass = pMDI->szClass;
  cs.lpszName  = pMDI->szTitle;

  return CMDIChildWnd::PreCreateWindow(cs);
}

void CChildFrame::OnMDIDestroy()
{
//  CMDIChildWnd::OnMDIDestroy();

}


void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActWnd, CWnd* pDeActWnd)
{
  /********************************************************************/
  /* for TranslationEnvironment restore MDI windows if not previously */
  /* maximized                                                        */
  /* if loosing the focus we have to store the currently active state */
  /********************************************************************/
  static BOOL fMaximizeWindow = FALSE;

  CMainFrame * pFrame = (CMainFrame *)GetMDIFrame();
  if ( !bActivate )
  {
    if ( GetActiveView()->SendMessage(WM_EQF_IS_TENV, NULL, NULL ))
    {
      pFrame->MDIGetActive( &pFrame->fTENVMaximized );
      if ( pFrame->fTENVMaximized && !fMaximizeWindow )
      {
        MDIRestore();
      } /* endif */
    }
    else
    {
      pFrame->MDIGetActive( &fMaximizeWindow );
    } /* endif */
  } /* endif */

  CMDIChildWnd::OnMDIActivate(bActivate, pActWnd, pDeActWnd);

  /********************************************************************/
  /* The following has to be done to ensure that only list views will */
  /* handle the correct activation/deactivation sequence              */
  /********************************************************************/
  if ( bActivate )
  {
    CView * pView = ((CChildFrame  *) pActWnd)->GetActiveView();
    pView->SendMessage( WM_EQF_MDIACTIVATE, (WPARAM)1L, (LPARAM)pView->m_hWnd );
  } /* endif */



  /********************************************************************/
  /* if bactive and TENV and maximized before -- maximize it again    */
  /********************************************************************/
  if ( bActivate && pFrame->fTENVMaximized
       && GetActiveView()->SendMessage(WM_EQF_IS_TENV, NULL, NULL ) )
  {
    MDIMaximize();
  } /* endif */
}

void CChildFrame::OnSysCommand( UINT nID, LPARAM lParam )
{
  CView * pView = GetActiveView();
  // take care of maximizing and double click on Titlebar, which is SC_MAXIMIZE | 0x0002 ...
  if (pView && ((nID & SC_MAXIMIZE) == SC_MAXIMIZE))
  {
     if (!::SendMessage( pView->m_hWnd, WM_SYSCOMMAND, nID, lParam))
	 {
		 CMDIChildWnd::OnSysCommand( nID, lParam );
	 };
  }
  else
  {
	  CMDIChildWnd::OnSysCommand( nID, lParam );
  }
}


VOID CChildFrame::OnSize(UINT mp1, int cx, int cy)
{
  CView * pView = GetActiveView();
  if (pView)
     ::SendMessage( pView->m_hWnd, WM_SIZE, mp1, MAKELONG( cx, cy ));
  CMDIChildWnd::OnSize( mp1, cx, cy );
}

VOID CChildFrame::OnMove(int cx, int cy)
{
  CView * pView = GetActiveView();
  if (pView)
     ::SendMessage( pView->m_hWnd, WM_MOVE, cx, cy );
  CMDIChildWnd::OnMove( cx, cy );
}



/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
        CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
        CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

