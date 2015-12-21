//+----------------------------------------------------------------------------+
//|EQFDIC99.h    Dictionary Lookup                                             |
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
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.4 $ ----------- 18 Jun 2001
// -- RJ: fix problem in maximizing MDI Dialogs
//
//
// $Revision: 1.3 $ ----------- 20 Mar 2001
// -- RJ: KBT0735: intercept WM_SETTEXT to correctly display titlebar in MDI windows
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFDIC99.HV_   1.3   15 Feb 1999 08:07:48   BUILD  $
 *
 * $Log:   K:\DATA\EQFDIC99.HV_  $
 *
 *    Rev 1.3   15 Feb 1999 08:07:48   BUILD
 * -- get rid of comments
 *
 *    Rev 1.2   11 Jan 1999 10:32:48   BUILD
 * -- store create struct
 *
 *    Rev 1.1   07 Dec 1998 10:43:28   BUILD
 * -- add dictionary list support
 *
 *    Rev 1.0   16 Nov 1998 16:56:40   BUILD
 * Initial revision.
 *
*/
//+----------------------------------------------------------------------------+

#include "eqfddlg.id"             // dialog IDs


class CDICProcDoc : public CEQFDoc
{
protected: // create from serialization only
        CDICProcDoc();
        DECLARE_DYNCREATE(CDICProcDoc)

// Attributes
public:

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CDICProcDoc)
        public:
        virtual BOOL OnNewDocument();
        virtual void Serialize(CArchive& ar);
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CDICProcDoc();
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
        //{{AFX_MSG(CDICProcDoc)
                // NOTE - the ClassWizard will add and remove member functions here.
                //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////


class CDICProcView : public CView
{
protected: // create from serialization only
        CDICProcView();
        DECLARE_DYNCREATE(CDICProcView)

// Attributes
public:
        CDICProcDoc* GetDocument();

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CDICProcView)
        public:
        virtual void OnDraw(CDC* pDC);  // overridden to draw this view
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
        protected:
        virtual void OnInitialUpdate(); // called first time after construct
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CDICProcView();
        VOID SetMDIStruct( MDICREATESTRUCT * p1 )
                     { memcpy(&MDICreate, p1, sizeof(MDICREATESTRUCT)); return;};
        MDICREATESTRUCT* GetMDIStruct() { return &MDICreate; };
        VOID SetTitle( PSZ p) { strcpy( (PSZ)&MDICreate.szTitle[0], p ); };
        VOID SetCreateStruct( LPCREATESTRUCT p1 )
                     { memcpy(&cs, p1, sizeof(CREATESTRUCT)); return;};
        LPCREATESTRUCT GetCreateStruct() { return &cs; };

        LRESULT TwbInitMenu( HWND hwnd, WPARAM mp1, LPARAM mp2 );

#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:
        MDICREATESTRUCT    MDICreate;
        CREATESTRUCT       cs;

// Generated message map functions
protected:
        //{{AFX_MSG(CDICProcView)
        afx_msg int     OnCreate( LPCREATESTRUCT cs);
        afx_msg VOID    OnSize( UINT mp1, int cx, int cy );
        afx_msg VOID    OnInitMenu( CMenu *pMenu );
        afx_msg LRESULT OnInitMenuPopup( WPARAM mp1, LPARAM mp2 );
        afx_msg VOID    OnChar( UINT mp1, UINT cx, UINT cy );
        afx_msg UINT    OnGetDlgCode();
        afx_msg VOID    OnKeyDown( int mp1, int cx, int cy );
        afx_msg VOID    OnClose();
        afx_msg LRESULT On_WM_SETTEXT( WPARAM mp1, LPARAM mp2 );
        afx_msg BOOL    OnEraseBackground( CDC* pDC );
        afx_msg LRESULT On_WM_TIMER( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(MDIDialogProc))( m_hWnd, WM_TIMER, mp1, mp2 ); };
        afx_msg VOID    OnProcessTask( WPARAM mp1, LPARAM mp2 );
        afx_msg void OnPuboCancel()
          { ((PFNWP)(MDIDialogProc))( m_hWnd, WM_COMMAND, LOOKUP_PUBO_CANCEL, NULL );}
        afx_msg void OnPuboOK()
          { ((PFNWP)(MDIDialogProc))( m_hWnd, WM_COMMAND, LOOKUP_PUBO_OK, NULL);}
        afx_msg void OnPuboEdit()
          { ((PFNWP)(MDIDialogProc))( m_hWnd, WM_COMMAND, LOOKUP_PUBO_EDIT, NULL );}
        afx_msg void OnMaximizeButton(CCmdUI* pCmdUI);
        afx_msg LRESULT On_WM_SysCommand( UINT nID, LPARAM lParam );
        //}}AFX_MSG
        afx_msg void OnAllCommand( UINT mp1)
                { ((PFNWP)(MDIDialogProc))( m_hWnd, WM_COMMAND, mp1, NULL );}

        DECLARE_MESSAGE_MAP()


// view type helpers
public:

// flags and states
protected:
};

#ifndef _DEBUG  // debug version in cpp file
inline CDICProcDoc* CDICProcView::GetDocument()
   { return (CDICProcDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

