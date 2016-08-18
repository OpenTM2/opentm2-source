//+----------------------------------------------------------------------------+
//|EQFMFRM.H     Interface of the CMainFrame class                             |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.2 $ ----------- 18 Oct 2004
// --RJ: P017482: support tile / cascade, minall, restoreall
// 
//
// $Revision: 1.1 $ ----------- 31 Aug 2004
//  -- New Release TM6.0.6!!
//
//
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
//
//
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
//
//
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
//
//
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
//
//
// $Revision: 1.2 $ ----------- 24 Feb 2003
// -- RJ: .NET adaptation
//
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.7 $ ----------- 18 Jun 2001
// -- RJ: add support for WM_QUERYENDSESSION and WM_ENDSESSION to allow shutdown
//
//
// $Revision: 1.6 $ ----------- 20 Mar 2001
// -- RJ: KBT1007: support mdimaximize message
//
//
// $Revision: 1.5 $ ----------- 19 Feb 2001
// -- add WM_EQF_SHOWHTML
//
//
// $Revision: 1.4 $ ----------- 16 Oct 2000
// -- update statusbar field
//
//
// $Revision: 1.3 $ ----------- 10 Jan 2000
// -- allow for generic popup
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFMFRM.HV_   1.5   01 Feb 1999 09:43:06   BUILD  $
 *
 * $Log:   K:\DATA\EQFMFRM.HV_  $
 *
 *    Rev 1.5   01 Feb 1999 09:43:06   BUILD
 * -- add dynamic recreation of closed TPRO and TWB toolbars..
 *
 *    Rev 1.4   18 Jan 1999 10:24:24   BUILD
 * -- add controlbar class for CEQFObjectBar
 *
 *    Rev 1.3   11 Jan 1999 10:37:08   BUILD
 * -- dynamic activation from toolbar items
 * -- add missing handling for Close processing
 *
 *    Rev 1.2   07 Dec 1998 10:38:18   BUILD
 * -- support restore saved size
 *
 *    Rev 1.1   09 Nov 1998 09:36:42   BUILD
 * -- add A1 as dummy function
 *
 *    Rev 1.0   26 Oct 1998 19:12:28   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#if !defined(AFX_MAINFRM_H__2EDD4189_CCC9_11D1_8162_00AA0026430D__INCLUDED_)
#define AFX_MAINFRM_H__2EDD4189_CCC9_11D1_8162_00AA0026430D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define EQFWORKBENCH        "EqfStart-WorkBench"

/**********************************************************************/
/* Use this macro for all messages send directly to the main          */
/* window procedure in EQFMAIN.C                                      */
/**********************************************************************/
#define ON_MSG_MACRO( B )   \
   afx_msg MRESULT On_##B( WPARAM mp1, LPARAM mp2 )           \
          { return TwbMainWP( m_hWnd, B, mp1, mp2 ); }


class CMainFrame : public CMDIFrameWnd
{
        DECLARE_DYNAMIC(CMainFrame)
public:
        CMainFrame();

// Attributes
public:
  BOOL fTENVMaximized;

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CMainFrame)
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CMainFrame();
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif
        void OnToggleToolBar( USHORT usId );
        void OnCheckToolBar();
        void OnSetStatusBar( USHORT usId, PSZ pData );

protected:  // control bar embedded members
        CStatusBar    m_wndStatusBar;
        CToolBar      m_wndTWBToolBar;
        CToolBar      m_wndTPROToolBar;


// Generated message map functions
protected:
        //{{AFX_MSG(CMainFrame)
        afx_msg VOID A2(WPARAM mp1, LPARAM mp2 );
        afx_msg MRESULT OnMDIGetActive(WPARAM mp1, LPARAM mp2 );
        afx_msg MRESULT OnMDIRestore(WPARAM mp1, LPARAM mp2 );
        afx_msg MRESULT OnMDIMaximize(WPARAM mp1, LPARAM mp2 );
        afx_msg LONG OnMDISetMenu(WPARAM mp1, LPARAM mp2 );
        afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
        afx_msg VOID OnClose();
        afx_msg MRESULT OnMDICreate( WPARAM mp1, LPARAM mp2);
        afx_msg MRESULT OnMDIActivate( WPARAM mp1, LPARAM mp2);
        afx_msg MRESULT OnEQFInit( WPARAM mp1, LPARAM mp2);
        afx_msg MRESULT OnEQFInitMenu( WPARAM mp1, LPARAM mp2 );
        afx_msg VOID OnInitMenuPopup( CMenu *pMenu, UINT uIndex, BOOL fSysMenu ) ;
        afx_msg void OnParentNotify( UINT message, LPARAM mp2 );
        afx_msg LONG On_HM_HELPSUBITEM_NOT_FOUND( WPARAM mp1, LPARAM mp2);

        afx_msg MRESULT OnMDITile( WPARAM mp1, LPARAM mp2);
        afx_msg MRESULT OnMDICascade( WPARAM mp1, LPARAM mp2);
		afx_msg MRESULT OnMDIMinall( WPARAM mp1, LPARAM mp2);
		afx_msg MRESULT OnMDIRestoreall( WPARAM mp1, LPARAM mp2);

		afx_msg MRESULT On_WM_COPYDATA( WPARAM mp1, LPARAM mp2);

        ON_MSG_MACRO( WM_QUERYENDSESSION )
        ON_MSG_MACRO( WM_ENDSESSION )
        ON_MSG_MACRO( WM_DESTROY )
        ON_MSG_MACRO( WM_EQF_TERMINATE )
        ON_MSG_MACRO( WM_EQF_SHUTDOWN )
        ON_MSG_MACRO( WM_EQF_SHOWHTML )
        ON_MSG_MACRO( WM_EQFN_SHUTDOWNCANCELED )
        ON_MSG_MACRO( WM_EQFN_OBJECTREMOVED )
        ON_MSG_MACRO( WM_EQF_ACTIVATEINSTANCE )
        ON_MSG_MACRO( WM_DDE_INITIATE )

        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#undef ON_MSG_MACRO

#endif // !defined(AFX_MAINFRM_H__2EDD4189_CCC9_11D1_8162_00AA0026430D__INCLUDED_)
