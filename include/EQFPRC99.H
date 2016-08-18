//+----------------------------------------------------------------------------+
//|EQFPRC99.H    Generic process window for MFC                                |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
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
// $Revision: 1.2 $ ----------- 26 Nov 2001
// GQ: - Added On_WM_SETTEXT method
//
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFPRC99.HV_   1.1   01 Feb 1999 09:44:04   BUILD  $
 *
 * $Log:   K:\DATA\EQFPRC99.HV_  $
 *
 *    Rev 1.1   01 Feb 1999 09:44:04   BUILD
 * -- cleanup of superfluous functions
 *
 *    Rev 1.0   11 Jan 1999 11:17:06   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
/**********************************************************************/
/* Use this macro for all messages send directly to our generic list  */
/* window procedure in EQFHNDLR.C                                     */
/**********************************************************************/
#define ON_MSG_MACRO( B )   \
   afx_msg LRESULT On_##B( WPARAM mp1, LPARAM mp2 )           \
          { return GENERICPROCESSWP( m_hWnd, B, mp1, mp2 ); }


class CGenProcDoc : public CEQFDoc
{
protected: // create from serialization only
        CGenProcDoc();
        DECLARE_DYNCREATE(CGenProcDoc)

// Attributes
public:

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CGenProcDoc)
        public:
        virtual BOOL OnNewDocument();
        virtual void Serialize(CArchive& ar);
        //}}AFX_VIRTUAL
        PSZ QuerySelFolderName();


// Implementation
public:
        virtual ~CGenProcDoc();
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
        //{{AFX_MSG(CGenProcDoc)
                // NOTE - the ClassWizard will add and remove member functions here.
                //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////


class CGenProcView : public CView
{
protected: // create from serialization only
        CGenProcView();
        DECLARE_DYNCREATE(CGenProcView)

// Attributes
public:
        CGenProcDoc* GetDocument();

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CGenProcView)
        public:
        virtual void OnDraw(CDC* pDC);  // overridden to draw this view
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
        protected:
        virtual void OnInitialUpdate(); // called first time after construct
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CGenProcView();

        MDICREATESTRUCT    MDICreate;
        VOID SetMDIStruct( MDICREATESTRUCT * p1 )
                     { memcpy(&MDICreate, p1, sizeof(MDICREATESTRUCT)); return;};
        MDICREATESTRUCT* GetMDIStruct() { return &MDICreate; };

        PSZ GetListTitle()   { return (PSZ)&pIda->CommArea.szTitle; };
        LRESULT TwbInitMenu( HWND hwnd, WPARAM mp1, LPARAM mp2 );

#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
        //{{AFX_MSG(CGenProcView)
        afx_msg int  OnCreate( LPCREATESTRUCT cs);
        afx_msg LRESULT OnEQFTerminate( WPARAM mp1, LPARAM mp2);
        afx_msg LRESULT OnInitMenuPopup( WPARAM mp1, LPARAM mp2 );

        afx_msg void OnToolbarActivate(CCmdUI* pCmdUI);
        afx_msg LRESULT On_WM_INITMENUPOPUP( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT On_WM_SETTEXT( WPARAM mp1, LPARAM mp2 );

        ON_MSG_MACRO( WM_ACTIVATE )
        ON_MSG_MACRO( WM_CLOSE )

        ON_MSG_MACRO( WM_DESTROY )
        ON_MSG_MACRO( WM_EQFN_CREATED  )
        ON_MSG_MACRO( WM_EQFN_DELETED  )
        ON_MSG_MACRO( WM_EQFN_TASKDONE )
        ON_MSG_MACRO( WM_EQF_ABOUTTODELETE )
        ON_MSG_MACRO( WM_EQF_COMMAND )
        ON_MSG_MACRO( WM_EQF_DDE_REQUEST )
        ON_MSG_MACRO( WM_EQF_INITIALIZE)
        ON_MSG_MACRO( WM_EQF_INITMENU)
        ON_MSG_MACRO( WM_EQF_PROCESSTASK )
        ON_MSG_MACRO( WM_EQF_QUERYPROPSIZE )
        ON_MSG_MACRO( WM_EQF_SETFOCUS )
        ON_MSG_MACRO( WM_EQF_SHUTDOWN )
        ON_MSG_MACRO( WM_EQF_SLIDER_SETTEXT )
        ON_MSG_MACRO( WM_EQF_UPDATESLIDER )
        ON_MSG_MACRO( WM_ERASEBKGND )
        ON_MSG_MACRO( WM_ICONERASEBKGND )
        ON_MSG_MACRO( WM_INITMENU )
        ON_MSG_MACRO( WM_KEYDOWN )
        ON_MSG_MACRO( WM_MDIACTIVATE )
        ON_MSG_MACRO( WM_PAINT )
        ON_MSG_MACRO( WM_QUERYDRAGICON )
        ON_MSG_MACRO( WM_SETFOCUS )
        ON_MSG_MACRO( WM_SIZE )
        ON_MSG_MACRO( WM_TIMER )

        //}}AFX_MSG

        afx_msg void  OnWndActivate( UINT mp1 );
        afx_msg void  On_WM_COMMAND( UINT mp1 ) { GENERICPROCESSWP( m_hWnd, WM_COMMAND, mp1, NULL ); }

        DECLARE_MESSAGE_MAP()


// view type helpers
public:
        PGENPROCESSINSTIDA  pIda;                   // pointer to generic list

// flags and states
protected:
};

#ifndef _DEBUG  // debug version in cpp file
inline CGenProcDoc* CGenProcView::GetDocument()
   { return (CGenProcDoc*)m_pDocument; }
#endif

#undef ON_MSG_MACRO

/////////////////////////////////////////////////////////////////////////////

