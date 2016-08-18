//+----------------------------------------------------------------------------+
//|EQFTMM99.H    translation memory maintenance handler                        |
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
// $Revision: 1.3 $ ----------- 18 Jul 2000
// - added handling for filter messages
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFTMM99.HV_   1.2   18 Jan 1999 10:26:14   BUILD  $
 *
 * $Log:   K:\DATA\EQFTMM99.HV_  $
 *
 *    Rev 1.2   18 Jan 1999 10:26:14   BUILD
 * -- enable OnSize/OnMove processing
 *
 *    Rev 1.1   11 Jan 1999 10:43:58   BUILD
 * -- cleanup
 *
 *    Rev 1.0   02 Nov 1998 11:23:50   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

class CGenTMMDoc : public CEQFDoc
{
protected: // create from serialization only
        CGenTMMDoc();
        DECLARE_DYNCREATE(CGenTMMDoc)

// Attributes
public:

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CGenTMMDoc)
        public:
        virtual BOOL OnNewDocument();
        virtual void Serialize(CArchive& ar);
        //}}AFX_VIRTUAL
        PSZ QuerySelFolderName();


// Implementation
public:
        virtual ~CGenTMMDoc();
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
        //{{AFX_MSG(CGenTMMDoc)
                // NOTE - the ClassWizard will add and remove member functions here.
                //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////


class CGenTMMView : public CView
{
protected: // create from serialization only
        CGenTMMView();
        DECLARE_DYNCREATE(CGenTMMView)

// Attributes
public:
        CGenTMMDoc* GetDocument();

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CGenTMMView)
        public:
        virtual void OnDraw(CDC* pDC);  // overridden to draw this view
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
        protected:
        virtual void OnInitialUpdate(); // called first time after construct
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CGenTMMView();

        MDICREATESTRUCT    MDICreate;
        VOID SetMDIStruct( MDICREATESTRUCT * p1 )
                     { memcpy(&MDICreate, p1, sizeof(MDICREATESTRUCT)); return;};
        MDICREATESTRUCT* GetMDIStruct() { return &MDICreate; };

        BOOL             fTEnvView()    { return FALSE;};

        MRESULT TwbInitMenu( HWND hwnd, WPARAM mp1, LPARAM mp2 );


#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
        //{{AFX_MSG(CGenTMMView)
        afx_msg int  OnCreate( LPCREATESTRUCT cs);
        afx_msg VOID OnClose();
        afx_msg VOID OnSize( UINT mp1, int cx, int cy );
        afx_msg VOID OnMove( int cx, int cy );
        afx_msg VOID OnMDICascade()
                { ((CMDIChildWnd *)GetParent())->GetMDIFrame()->MDICascade(); };

        afx_msg VOID OnMDITile()
                { ((CMDIChildWnd *)GetParent())->GetMDIFrame()->MDITile(); };

        afx_msg VOID OnMDIMinAll();

        afx_msg VOID OnMDIRestore();


  /********************************************************************/
  /* all of our EQF special defined messages                          */
  /********************************************************************/
  afx_msg MRESULT On_WM_EQF_INITIALIZE( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_INITIALIZE, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_TERMINATE( WPARAM mp1, LPARAM mp2 )
          { LONG lResult = ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_TERMINATE, mp1, mp2 );
            PostMessage(WM_CLOSE, NULL, NULL);      // force close of view..
            return lResult; }
  afx_msg MRESULT On_WM_EQF_SHUTDOWN( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_SHUTDOWN, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQFN_SHUTDOWNCANCELED( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQFN_SHUTDOWNCANCELED, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYSYSTEMPATH( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYSYSTEMPATH, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYSYSPROPPATH( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYSYSPROPPATH, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_ACTIVATEINSTANCE( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_ACTIVATEINSTANCE, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYMENUTBL( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYMENUTBL, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_OPEN( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_OPEN, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_CREATE( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_CREATE, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_DELETE( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_DELETE, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_ABOUTTODELETE( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_ABOUTTODELETE, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQFN_CREATED( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQFN_CREATED, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQFN_DELETED( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQFN_DELETED, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQFN_PROPERTIESCHANGED( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQFN_PROPERTIESCHANGED, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_INSERTNAMES( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_INSERTNAMES, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYSELECTEDNAMES( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYSELECTEDNAMES, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_COMMAND( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_COMMAND, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYHANDLER( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYHANDLER, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_REMOVEHANDLER( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_REMOVEHANDLER, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_REMOVEOBJECT( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_REMOVEOBJECT, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYOBJECT( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYOBJECT, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYOBJECTNAME( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYOBJECTNAME, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYOBJECTCLASS( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYOBJECTCLASS, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYOBJECTSTATUS( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYOBJECTSTATUS, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_SETOBJECTSTATUS( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_SETOBJECTSTATUS, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYOBJECTCOUNT( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYOBJECTCOUNT, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_GETOBJECTLIST( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_GETOBJECTLIST, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYHANDLERCOUNT( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYHANDLERCOUNT, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_SETSYMBOL( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_SETSYMBOL, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYSYMBOL( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYSYMBOL, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_REMOVESYMBOL( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_REMOVESYMBOL, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQFN_OBJECTREMOVED( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQFN_OBJECTREMOVED, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYSYSTEMPROPHND( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYSYSTEMPROPHND, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_OPENPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_OPENPROPERTIES, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_CREATEPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_CREATEPROPERTIES, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_DELETEPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_DELETEPROPERTIES, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_CLOSEPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_CLOSEPROPERTIES, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_GETALLPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_GETALLPROPERTIES, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_PUTALLPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_PUTALLPROPERTIES, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_SAVEPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_SAVEPROPERTIES, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYPROPSIZE( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYPROPSIZE, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_REGISTER( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_REGISTER, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_REINIT( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_REINIT, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_LSTREGISTER( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_LSTREGISTER, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_LSTEVENT( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_LSTEVENT, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_LSTQRYLOCK( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_LSTQRYLOCK, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_LSTSETLOCK( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_LSTSETLOCK, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_REFRESH( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_REFRESH, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_SETFOCUS( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_SETFOCUS, mp1, mp2 );}
  afx_msg MRESULT On_LM_EQF_QUERYVIEWLIST( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, LM_EQF_QUERYVIEWLIST, mp1, mp2 );}
  afx_msg MRESULT On_LM_EQF_SETVIEWLIST( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, LM_EQF_SETVIEWLIST, mp1, mp2 );}
  afx_msg MRESULT On_LM_EQF_SETSORTLIST( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, LM_EQF_SETSORTLIST, mp1, mp2 );}
  afx_msg MRESULT On_LM_EQF_QUERYSORTLIST( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, LM_EQF_QUERYSORTLIST, mp1, mp2 );}
  afx_msg MRESULT On_LM_EQF_SETFILTER( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, LM_EQF_SETFILTER, mp1, mp2 );}
  afx_msg MRESULT On_LM_EQF_QUERYFILTER( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, LM_EQF_QUERYFILTER, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_NEXTSTEP( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_NEXTSTEP, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_PROCESSTASK( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_PROCESSTASK, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQFN_TASKDONE( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQFN_TASKDONE, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_ABOUTTOREMOVEDRIVE( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_ABOUTTOREMOVEDRIVE, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_SLIDER_POSITION( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_SLIDER_POSITION, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_SLIDER_SETTEXT( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_SLIDER_SETTEXT, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMLOAD_START( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMLOAD_START, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMLOAD_PROCESS( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMLOAD_PROCESS, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMLOAD_END( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMLOAD_END, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMFILL_LISTBOX( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMFILL_LISTBOX, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMEXPORT_START( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMEXPORT_START, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMEXPORT_PROCESS( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMEXPORT_PROCESS, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMEXPORT_END( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMEXPORT_END, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMORGANIZE_START( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMORGANIZE_START, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMORGANIZE_PROCESS( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMORGANIZE_PROCESS, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMORGANIZE_END( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMORGANIZE_END, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMMERGE_START( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMMERGE_START, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMMERGE_PROCESS( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMMERGE_PROCESS, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMMERGE_END( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMMERGE_END, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERY( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERY, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_FONTCHANGED( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_FONTCHANGED, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_COM_INITIALIZE( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_COM_INITIALIZE, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_COM_EXIT_COMPLETE( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_COM_EXIT_COMPLETE, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MEMORGANIZE_ACTIVATE( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MEMORGANIZE_ACTIVATE, mp1, mp2 );}
  afx_msg MRESULT On_WM_DRIVEBUTTON_INIT( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_DRIVEBUTTON_INIT, mp1, mp2 );}
  afx_msg MRESULT On_WM_DRIVEBUTTON_SELECT( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_DRIVEBUTTON_SELECT, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_PROOF( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_PROOF, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_MISSPELLED( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_MISSPELLED, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_AUTOTRANS( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_AUTOTRANS, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_DDE_REQUEST( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_DDE_REQUEST, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_DDE_ANSWER( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_DDE_ANSWER, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQFN_DRIVEREMOVED( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQFN_DRIVEREMOVED, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQFN_DRIVEADDED( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQFN_DRIVEADDED, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_COLCHANGED( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_COLCHANGED, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_WD_MAIN_NOTIFY( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_WD_MAIN_NOTIFY, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_INITMENU( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_INITMENU, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_BUILDITEMTEXT( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_BUILDITEMTEXT, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_CREATELISTWINDOW( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_CREATELISTWINDOW, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_UPDATESLIDER( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_UPDATESLIDER, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_SHOWPOPUP( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_SHOWPOPUP, mp1, mp2 );}
  afx_msg MRESULT On_EQF_CD_BM_CLICK( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, EQF_CD_BM_CLICK, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_CHECK_REM_PROPS( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_CHECK_REM_PROPS, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_FILTSETDICT( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_FILTSETDICT, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_QUERYID( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_QUERYID, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_CHANGEOBJECTNAME( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_CHANGEOBJECTNAME, mp1, mp2 );}
  afx_msg MRESULT On_WM_EQF_CLOSE( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_EQF_CLOSE, mp1, mp2 );}
  afx_msg MRESULT On_LM_EQF_GETITEMFROMPOINT( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, LM_EQF_GETITEMFROMPOINT, mp1, mp2 );}
  afx_msg MRESULT On_LM_EQF_REFRESH( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, LM_EQF_REFRESH, mp1, mp2 );}

  afx_msg MRESULT On_WM_EQF_SHOWHTML( WPARAM mp1, LPARAM mp2 );

  afx_msg MRESULT On_WM_EQF_IS_TENV( WPARAM mp1, LPARAM mp2 )
          { mp1; mp2;return TRUE; }

  afx_msg MRESULT OnInitMenu( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_INITMENU, mp1, mp2 );}
  afx_msg MRESULT OnInitMenuPopup( WPARAM mp1, LPARAM mp2 );


        //}}AFX_MSG
        afx_msg void OnAllCommand( UINT mp1 )
          {  ((PFNWP)(TMMWNDPROC))( m_hWnd, WM_COMMAND, mp1, NULL );}

        DECLARE_MESSAGE_MAP()


public:

protected:
};

#ifndef _DEBUG  // debug version in cpp file
inline CGenTMMDoc* CGenTMMView::GetDocument()
   { return (CGenTMMDoc*)m_pDocument; }
#endif


/////////////////////////////////////////////////////////////////////////////

