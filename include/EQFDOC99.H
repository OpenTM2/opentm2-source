//+----------------------------------------------------------------------------+
//|EQFDOC99.H    document handler for MFC                                      |
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
// $Revision: 1.6 $ ----------- 17 Apr 2001
// -- RJ: KBT01032: post message only if window still exist...
//
//
// $Revision: 1.5 $ ----------- 8 Jan 2001
// -- add updatestatusbar function
//
//
// $Revision: 1.4 $ ----------- 16 Oct 2000
// -- support statusbar
//
//
// $Revision: 1.3 $ ----------- 21 Jun 2000
// -- add OnFontExtDlg
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFDOC99.HV_   1.7   11 Nov 1999 15:21:50   BUILD  $
 *
 * $Log:   K:\DATA\EQFDOC99.HV_  $
 *
 *    Rev 1.7   11 Nov 1999 15:21:50   BUILD
 * deleted hex 00
 *
 *    Rev 1.6   17 May 1999 13:43:46   BUILD
 * -- add support for drag/drop
 *
 *    Rev 1.5   15 Feb 1999 08:11:26   BUILD
 * -- get rid of commented code sections
 *
 *    Rev 1.4   21 Jan 1999 10:17:26   BUILD
 * -- add initial sizing routine to restore last used size
 * -- add EQFM_ messages
 *
 *    Rev 1.3   12 Jan 1999 12:58:52   BUILD
 * -- allow for Windows minimize and restore
 *
 *    Rev 1.2   11 Jan 1999 10:46:58   BUILD
 * -- add ToolbarEnable
 *
 *    Rev 1.1   07 Dec 1998 10:38:04   BUILD
 * -- add On_PID_SYS_CLOSE, On_PID_SYS_MOVE, On_PID_SYS_SIZE
 *
 *    Rev 1.0   26 Oct 1998 19:13:02   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
/**********************************************************************/
/* Use this macro for all messages send directly to our generic list  */
/* window procedure in EQFHNDLR.C                                     */
/**********************************************************************/
#define ON_MSG_MACRO( B )   \
   afx_msg LRESULT On_##B( WPARAM mp1, LPARAM mp2 )           \
          { return DOCUMENTWP( m_hWnd, B, mp1, mp2 );}


class CGenTEnvDoc : public CEQFDoc
{
protected: // create from serialization only
        CGenTEnvDoc();
        DECLARE_DYNCREATE(CGenTEnvDoc)

// Attributes
public:

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CGenTEnvDoc)
        public:
        virtual BOOL OnNewDocument();
        virtual void Serialize(CArchive& ar);
        //}}AFX_VIRTUAL
        PSZ QuerySelFolderName();


// Implementation
public:
        virtual ~CGenTEnvDoc();
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
        //{{AFX_MSG(CGenTEnvDoc)
                // NOTE - the ClassWizard will add and remove member functions here.
                //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

class CRectItem;

class CGenTEnvView : public CView
{
protected: // create from serialization only
        CGenTEnvView();
        DECLARE_DYNCREATE(CGenTEnvView)

// Attributes
public:
        CGenTEnvDoc* GetDocument();

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CGenTEnvView)
        public:
        virtual void OnDraw(CDC* pDC);  // overridden to draw this view
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
        protected:
        virtual void OnInitialUpdate(); // called first time after construct
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CGenTEnvView();

        MDICREATESTRUCT    MDICreate;
        VOID SetMDIStruct( MDICREATESTRUCT * p1 )
                     { memcpy(&MDICreate, p1, sizeof(MDICREATESTRUCT)); return;};
        MDICREATESTRUCT* GetMDIStruct() { return &MDICreate; };
        PDOCUMENT_IDA    GetpIda()      { return pIda; };
        BOOL             fTEnvView()    { return TRUE; };

        LRESULT TwbInitMenu( HWND hwnd, WPARAM mp1, LPARAM mp2 );

        BOOL             OnDrop ( COleDataObject* pDataObject,
                                  DROPEFFECT dropEffect, CPoint point );

        DROPEFFECT   OnDragEnter( COleDataObject* pDataObject,
                                  DWORD grfKeyState, CPoint point);

        DROPEFFECT   OnDragOver(  COleDataObject*,
                                  DWORD grfKeyState, CPoint point );

        void         OnDragLeave();

        BOOL         GetObjectInfo(COleDataObject* pDataObject,
                                   CSize* pSize, CSize* pOffset);

        void         ClientToDoc(CRect& rect);
        void         ClientToDoc(CSize& size);
        void         ClientToDoc(CPoint& point);

        void         DocToClient(CRect& rect);
        void         DocToClient(CSize& size);
        void         DocToClient(CPoint& point);

#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:
   BOOL m_bInDrag;
   CRectItem* m_pSelection;
   COleDropTarget  m_dropTarget;
   CPoint          m_dragPoint;
   CSize           m_dragSize;
   CSize           m_dragOffset;
   DROPEFFECT      m_prevDropEffect;
   static CLIPFORMAT m_cfObjectDescriptor;

// Generated message map functions
protected:
  //{{AFX_MSG(CGenTEnvView)
  afx_msg int  OnCreate( LPCREATESTRUCT cs);
  afx_msg VOID OnClose();
  afx_msg VOID OnSize( UINT mp1, int cx, int cy );
  afx_msg LRESULT OnInitMenuPopup( WPARAM mp1, LPARAM mp2 );
  /********************************************************************/
  /* all of our EQF special defined messages                          */
  /********************************************************************/
  afx_msg LRESULT On_A( WPARAM mp1, LPARAM mp2 );

  ON_MSG_MACRO(  WM_EQF_INITIALIZE )

  afx_msg LRESULT On_WM_EQF_TERMINATE( WPARAM mp1, LPARAM mp2 )
          { LONG lResult = FALSE;
            if (::IsWindow(m_hWnd) )
            {
              lResult = ((PFNWP)(DOCUMENTWP))( m_hWnd, WM_EQF_TERMINATE, mp1, mp2 );
              if (::IsWindow(m_hWnd) )
                PostMessage(WM_CLOSE, NULL, NULL);      // force close of view..
            }
            return lResult; }

  ON_MSG_MACRO(  WM_EQF_SHUTDOWN )

  ON_MSG_MACRO(  WM_EQFN_SHUTDOWNCANCELED )

  ON_MSG_MACRO(  WM_EQF_QUERYSYSTEMPATH )

  ON_MSG_MACRO(  WM_EQF_QUERYSYSPROPPATH )

  ON_MSG_MACRO(  WM_EQF_ACTIVATEINSTANCE )

  ON_MSG_MACRO(  WM_EQF_QUERYMENUTBL )

  ON_MSG_MACRO(  WM_EQF_OPEN )

  ON_MSG_MACRO(  WM_EQF_CREATE )
  ON_MSG_MACRO(  WM_EQF_DELETE )
  ON_MSG_MACRO(  WM_EQF_ABOUTTODELETE )
  ON_MSG_MACRO(  WM_EQFN_CREATED )
  ON_MSG_MACRO(  WM_EQFN_DELETED )
  ON_MSG_MACRO(  WM_EQFN_PROPERTIESCHANGED )
  ON_MSG_MACRO(  WM_EQF_INSERTNAMES )
  ON_MSG_MACRO(  WM_EQF_QUERYSELECTEDNAMES )
  ON_MSG_MACRO(  WM_EQF_COMMAND )
  ON_MSG_MACRO(  WM_EQF_QUERYHANDLER )
  ON_MSG_MACRO(  WM_EQF_REMOVEHANDLER )
  ON_MSG_MACRO(  WM_EQF_REMOVEOBJECT )
  ON_MSG_MACRO(  WM_EQF_QUERYOBJECT )
  ON_MSG_MACRO(  WM_EQF_QUERYOBJECTNAME )
  ON_MSG_MACRO(  WM_EQF_QUERYOBJECTCLASS )
  ON_MSG_MACRO(  WM_EQF_QUERYOBJECTSTATUS )
  ON_MSG_MACRO(  WM_EQF_SETOBJECTSTATUS )
  ON_MSG_MACRO(  WM_EQF_QUERYOBJECTCOUNT )
  ON_MSG_MACRO(  WM_EQF_GETOBJECTLIST )
  ON_MSG_MACRO(  WM_EQF_QUERYHANDLERCOUNT )
  ON_MSG_MACRO(  WM_EQF_SETSYMBOL )
  ON_MSG_MACRO(  WM_EQF_QUERYSYMBOL )
  ON_MSG_MACRO(  WM_EQF_REMOVESYMBOL )
  ON_MSG_MACRO(  WM_EQFN_OBJECTREMOVED )
  ON_MSG_MACRO(  WM_EQF_QUERYSYSTEMPROPHND )
  ON_MSG_MACRO(  WM_EQF_OPENPROPERTIES )
  ON_MSG_MACRO(  WM_EQF_CREATEPROPERTIES )
  ON_MSG_MACRO(  WM_EQF_DELETEPROPERTIES )
  ON_MSG_MACRO(  WM_EQF_CLOSEPROPERTIES )
  ON_MSG_MACRO(  WM_EQF_GETALLPROPERTIES )
  ON_MSG_MACRO(  WM_EQF_PUTALLPROPERTIES )
  ON_MSG_MACRO(  WM_EQF_SAVEPROPERTIES )
  ON_MSG_MACRO(  WM_EQF_QUERYPROPSIZE )
  ON_MSG_MACRO(  WM_EQF_REGISTER )
  ON_MSG_MACRO(  WM_EQF_REINIT )
  ON_MSG_MACRO(  WM_EQF_LSTREGISTER )
  ON_MSG_MACRO(  WM_EQF_LSTEVENT )
  ON_MSG_MACRO(  WM_EQF_LSTQRYLOCK )
  ON_MSG_MACRO(  WM_EQF_LSTSETLOCK )
  ON_MSG_MACRO(  WM_EQF_REFRESH    )
  ON_MSG_MACRO(  WM_EQF_SETFOCUS   )
  ON_MSG_MACRO(  LM_EQF_QUERYVIEWLIST )
  ON_MSG_MACRO(  LM_EQF_SETVIEWLIST )
  ON_MSG_MACRO(  LM_EQF_SETSORTLIST )
  ON_MSG_MACRO(  LM_EQF_QUERYSORTLIST )
  ON_MSG_MACRO(  WM_EQF_NEXTSTEP )
  ON_MSG_MACRO(  WM_EQF_PROCESSTASK )
  ON_MSG_MACRO(  WM_EQFN_TASKDONE )
  ON_MSG_MACRO(  WM_EQF_ABOUTTOREMOVEDRIVE )
  ON_MSG_MACRO(  WM_EQF_SLIDER_POSITION )
  ON_MSG_MACRO(  WM_EQF_SLIDER_SETTEXT )
  ON_MSG_MACRO(  WM_EQF_MEMLOAD_START )
  ON_MSG_MACRO(  WM_EQF_MEMLOAD_PROCESS )
  ON_MSG_MACRO(  WM_EQF_MEMLOAD_END )
  ON_MSG_MACRO(  WM_EQF_MEMFILL_LISTBOX )
  ON_MSG_MACRO(  WM_EQF_MEMEXPORT_START )
  ON_MSG_MACRO(  WM_EQF_MEMEXPORT_PROCESS )
  ON_MSG_MACRO(  WM_EQF_MEMEXPORT_END )
  ON_MSG_MACRO(  WM_EQF_MEMORGANIZE_START )
  ON_MSG_MACRO(  WM_EQF_MEMORGANIZE_PROCESS )
  ON_MSG_MACRO(  WM_EQF_MEMORGANIZE_END )
  ON_MSG_MACRO(  WM_EQF_MEMMERGE_START )
  ON_MSG_MACRO(  WM_EQF_MEMMERGE_PROCESS )
  ON_MSG_MACRO(  WM_EQF_MEMMERGE_END )
  ON_MSG_MACRO(  WM_EQF_QUERY )
  ON_MSG_MACRO(  WM_EQF_FONTCHANGED )
  ON_MSG_MACRO(  WM_EQF_MEMORGANIZE_ACTIVATE )
  ON_MSG_MACRO(  WM_DRIVEBUTTON_INIT )
  ON_MSG_MACRO(  WM_DRIVEBUTTON_SELECT )
  ON_MSG_MACRO(  WM_EQF_PROOF )
  ON_MSG_MACRO(  WM_EQF_MISSPELLED )
  ON_MSG_MACRO(  WM_EQF_AUTOTRANS )
  ON_MSG_MACRO(  WM_EQF_DDE_REQUEST )
  ON_MSG_MACRO(  WM_EQF_DDE_ANSWER )
  ON_MSG_MACRO(  WM_EQFN_DRIVEREMOVED )
  ON_MSG_MACRO(  WM_EQFN_DRIVEADDED )
  ON_MSG_MACRO(  WM_EQF_COLCHANGED )
  ON_MSG_MACRO(  WM_EQF_WD_MAIN_NOTIFY )
  ON_MSG_MACRO(  WM_EQF_INITMENU )
  ON_MSG_MACRO(  WM_EQF_BUILDITEMTEXT )
  ON_MSG_MACRO(  WM_EQF_CREATELISTWINDOW )
  ON_MSG_MACRO(  WM_EQF_UPDATESLIDER )
  ON_MSG_MACRO(  WM_EQF_SHOWPOPUP )
  ON_MSG_MACRO(  EQF_CD_BM_CLICK )
  ON_MSG_MACRO(  WM_EQF_CHECK_REM_PROPS )
  ON_MSG_MACRO(  WM_EQF_FILTSETDICT )
  ON_MSG_MACRO(  WM_EQF_QUERYID )
  ON_MSG_MACRO(  WM_EQF_CHANGEOBJECTNAME )
  ON_MSG_MACRO(  WM_EQF_CLOSE )
  ON_MSG_MACRO(  LM_EQF_GETITEMFROMPOINT )
  ON_MSG_MACRO(  LM_EQF_REFRESH )
  ON_MSG_MACRO(  EQFM_DOC_IS_SAVED )
  ON_MSG_MACRO(  EQFM_DOC_IS_LOADED )
  ON_MSG_MACRO(  EQFM_DOC_STATUS )
  ON_MSG_MACRO(  EQFM_DOC_IS_XLATED )
  ON_MSG_MACRO(  EQFM_DOC_IS_ULATED )

  afx_msg LRESULT On_WM_EQF_SHOWHTML( WPARAM mp1, LPARAM mp2 );

  afx_msg LRESULT On_WM_EQF_DRAGDROP( WPARAM mp1, LPARAM mp2 );

  afx_msg LRESULT On_WM_EQF_IS_TENV( WPARAM mp1, LPARAM mp2 )
          { mp1; mp2; return TRUE; }

  afx_msg LRESULT OnInitMenu( WPARAM mp1, LPARAM mp2 )
          { return ((PFNWP)(DOCUMENTWP))( m_hWnd, WM_INITMENU, mp1, mp2 );}

  afx_msg void OnAllCommand( UINT mp1)
          {  ((PFNWP)(DOCUMENTWP))( m_hWnd, WM_COMMAND, mp1, NULL );}
  afx_msg void    OnToolbarActivate(CCmdUI* pCmdUI);

  afx_msg void OnMDIMinAll();
  afx_msg void OnMDIRestore();

  afx_msg void On_PID_SYS_CLOSE()
          { ((PFNWP)(DOCUMENTWP))( m_hWnd, WM_COMMAND, PID_SYS_CLOSE, NULL );}
  afx_msg void On_PID_SYS_SIZE()
          { ((PFNWP)(DOCUMENTWP))( m_hWnd, WM_COMMAND, PID_SYS_SIZE, NULL );}
  afx_msg void On_PID_SYS_MOVE()
          { ((PFNWP)(DOCUMENTWP))( m_hWnd, WM_COMMAND, PID_SYS_MOVE, NULL );}

  afx_msg void OnFontExtDlg();

  afx_msg LRESULT UpdateStatusBar( WPARAM mp1, LPARAM mp2 );
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

  void OnUpdateStatusIns(CCmdUI *pCmdUI);
  void OnUpdateStatusSegNum(CCmdUI *pCmdUI);
  void OnUpdateStatusLinNum(CCmdUI *pCmdUI);

public:
        PDOCUMENT_IDA  pIda;                   // pointer to generic list
protected:
};

#ifndef _DEBUG  // debug version in cpp file
inline CGenTEnvDoc* CGenTEnvView::GetDocument()
   { return (CGenTEnvDoc*)m_pDocument; }
#endif



/////////////////////////////////////////////////////////////////////////////
// EQFSHOWDLG dialog

class EQFSHOWDLG : public CDialog
{
// Construction
public:
        EQFSHOWDLG(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
        //{{AFX_DATA(EQFSHOWDLG)
        enum { IDD = ID_TB_SHOW_DLG };
                // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(EQFSHOWDLG)
        protected:
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(EQFSHOWDLG)
        virtual BOOL OnInitDialog();
        afx_msg void OnCancel();
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg LRESULT On_WM_EQF_INITIALIZE(WPARAM mp1, LPARAM mp2);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
public:
        char chFileName[64];        // file name to be loaded
        char chTitle[128];          // title text of show window
        PVOID pShowWnd;
};




class CRectItem : public COleClientItem
{
        DECLARE_SERIAL(CRectItem)
        CRectItem();

public:
        CRectItem(COleDocument* pContainer);
        ~CRectItem();

// Attributes
        CPoint m_ptPos;             //position in document
        CSize GetSize();            //current size
        void SetSize(CSize size);
        CSize GetBaseSize();        //base size
        void SetBaseSize(CSize size);
        CRect GetRect()             //current rect
                { return CRect(m_ptPos, GetSize()); }
        void SetRect(CRect& rect);

        CGenTEnvDoc* GetDocument()
                { return (CGenTEnvDoc*)COleClientItem::GetDocument(); }
        CGenTEnvView* GetActiveView()
                { return (CGenTEnvView*)COleClientItem::GetActiveView(); }

// Operations
        void Dirty()
                { GetDocument()->SetModifiedFlag(); }
        void Invalidate(CView* pNotThisView = NULL);
        BOOL UpdateExtent();
        void Move(CRect &rc);
        void ResetSize();
        void GetNativeClipboardData(COleDataSource *pDataSource);

        virtual void Serialize(CArchive& ar); // from CObject - public to call directly

// Overridables
protected:
        virtual void OnChange(OLE_NOTIFICATION wNotification, DWORD dwParam);
        virtual BOOL OnChangeItemPosition(const CRect& rectPos);
        virtual void OnActivate();
        virtual COleDataSource* OnGetClipboardData(BOOL bIncludeLink,
                LPPOINT lpOffset, LPSIZE lpSize);
        virtual void OnDeactivateUI(BOOL bUndoable);

public:
        virtual void OnGetItemPosition(CRect& rPosition);
protected:
        CSize m_sizeIcon;           //current size
        CSize m_sizeContent;        //current size
        CSize m_sizeIconExtent;     //default size
        CSize m_sizeContentExtent;  //default size
};




#undef ON_MSG_MACRO
/////////////////////////////////////////////////////////////////////////////



