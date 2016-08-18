//+----------------------------------------------------------------------------+
//|EQFRPT99.h    Report handler                                                |
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
// $Revision: 1.2 $ ----------- 14 Oct 2004
// --RJ: P015098: add OnDocSelect()
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
// $Revision: 1.2 $ ----------- 2 Feb 2004
// GQ: - added WM_EQF_PROCESSTASK message handler
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
// $Revision: 1.3 $ ----------- 12 Mar 2003
// GQ: - added handling for WM_EQF_SETFOCUS message
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
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFRPT99.HV_   1.4   29 Mar 1999 15:36:22   BUILD  $
 *
 * $Log:   K:\DATA\EQFRPT99.HV_  $
 *
 *    Rev 1.4   29 Mar 1999 15:36:22   BUILD
 * added OnLButtonDblClk
 *
 *    Rev 1.3   18 Jan 1999 15:15:16   BUILD
 * -- add OnMove support
 *
 *    Rev 1.2   18 Jan 1999 10:26:42   BUILD
 * -- add Windows pulldown handling
 *
 *    Rev 1.1   15 Dec 1998 11:48:34   BUILD
 * -- adjust to use HTML control
 *
 *    Rev 1.0   26 Oct 1998 19:10:52   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
/**********************************************************************/
/* Use this macro for all messages send directly to our generic list  */
/* window procedure in EQFHNDLR.C                                     */
/**********************************************************************/
#define ON_MSG_MACRO( B )   \
   afx_msg LRESULT On_##B( WPARAM mp1, LPARAM mp2 )           \
          { return pIda->pfnCallBack( &pIda->CommArea, m_hWnd, B, mp1, mp2 ); }


class CRPTProcDoc : public CEQFDoc
{
protected: // create from serialization only
        CRPTProcDoc();
        DECLARE_DYNCREATE(CRPTProcDoc)



// Attributes
public:

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CRPTProcDoc)
        public:
        virtual BOOL OnNewDocument();
        virtual void Serialize(CArchive& ar);
        //}}AFX_VIRTUAL
        PSZ QuerySelFolderName();


// Implementation
public:
        virtual ~CRPTProcDoc();
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
        //{{AFX_MSG(CRPTProcDoc)
                // NOTE - the ClassWizard will add and remove member functions here.
                //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_MSG


        DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////


class CRPTProcView : public CView
{
protected: // create from serialization only
        CRPTProcView();
        DECLARE_DYNCREATE(CRPTProcView)

// Attributes
public:
        CRPTProcDoc* GetDocument();

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CRPTProcView)
        public:
        virtual void OnDraw(CDC* pDC);  // overridden to draw this view
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
        protected:
        virtual void OnInitialUpdate(); // called first time after construct
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CRPTProcView();

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
        //{{AFX_MSG(CRPTProcView)
        afx_msg int     OnCreate      ( LPCREATESTRUCT cs);
        afx_msg LRESULT OnClose       ( WPARAM mp1, LPARAM mp2 );
        afx_msg VOID    OnSize        ( UINT mp1, int cx, int cy );
        afx_msg VOID    OnMove        ( int cx, int cy );
        afx_msg LRESULT OnEQFInit     ( WPARAM mp1, LPARAM mp2);
        afx_msg LRESULT OnEQFTerminate( WPARAM mp1, LPARAM mp2);

        ON_MSG_MACRO( WM_DESTROY )
        ON_MSG_MACRO( WM_TIMER )
        ON_MSG_MACRO( WM_INITMENU )
        ON_MSG_MACRO( WM_EQF_INITMENU )

        afx_msg void  OnViewTWBToolBar();
        afx_msg void  OnViewTPROToolBar();
        afx_msg void  OnViewStatusBar();

        afx_msg void OnToolbarActivate(CCmdUI* pCmdUI);
        afx_msg LRESULT On_WM_INITMENUPOPUP( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT On_WM_EQF_PROCESSTASK( WPARAM mp1, LPARAM mp2 );

        afx_msg MRESULT On_WM_EQF_SETFOCUS( WPARAM mp1, LPARAM mp2 );

        afx_msg VOID OnMDICascade()
                { ((CMDIChildWnd *)GetParent())->GetMDIFrame()->MDICascade(); };

        afx_msg VOID OnMDITile()
                { ((CMDIChildWnd *)GetParent())->GetMDIFrame()->MDITile(); };

        afx_msg VOID OnMDIMinAll();


        afx_msg VOID OnMDIRestore();

//        afx_msg VOID OnLButtonDblClk(UINT uFlags, CPoint Point );

        //}}AFX_MSG
        afx_msg void OnWndActivate( UINT mp1);
        afx_msg void OnDocSelect();
        afx_msg void On_WM_COMMAND( UINT mp1) {  pIda->pfnCallBack( &pIda->CommArea, m_hWnd, WM_COMMAND, mp1, NULL ); }
        DECLARE_MESSAGE_MAP()


// view type helpers
public:
        PGENLISTINSTIDA pIda;
        PVOID  pCWeb;
// flags and states
protected:
};

#ifndef _DEBUG  // debug version in cpp file
inline CRPTProcDoc* CRPTProcView::GetDocument()
   { return (CRPTProcDoc*)m_pDocument; }
#endif

#undef ON_MSG_MACRO
/////////////////////////////////////////////////////////////////////////////

