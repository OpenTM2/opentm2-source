//+----------------------------------------------------------------------------+
//|EQFFLL99.h    Folder and Subfolder list support                             |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#define CFLLPROCVIEW "CFLLListView"


class CFLLProcDoc : public CEQFDoc
{
protected: // create from serialization only
        CFLLProcDoc();
        DECLARE_DYNCREATE(CFLLProcDoc)

// Attributes
public:
  HMENU m_hMyMenu;

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CFLLProcDoc)
        public:
        virtual BOOL OnNewDocument();
        virtual void Serialize(CArchive& ar);
        virtual HMENU GetDefaultMenu();
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CFLLProcDoc();
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
        //{{AFX_MSG(CFLLProcDoc)
                // NOTE - the ClassWizard will add and remove member functions here.
                //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};


// cv
/////////////////////////////////////////////////////////////////////////////
// CTreeDropTarget

class CTreeDropTarget :public COleDropTarget
{
// Overrides
    public:
        virtual DROPEFFECT OnDragEnter( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );
        virtual DROPEFFECT OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );
        virtual void OnDragLeave( CWnd* pWnd );
        virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);

// Attributes
    public:
      HTREEITEM m_hItemFolder;

};


class CFLLListView : public CTreeView
{
protected:
        CFLLListView();
        DECLARE_DYNCREATE(CFLLListView)

// Attribute
public:
//        CFLLProcView *pParentView;

       // cv
        CTreeDropTarget m_TreeDropTarget;
        //

// Operationen
public:

        //{{AFX_VIRTUAL(CFLLListView)
        public:
        virtual void OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView );
        virtual void OnDraw(CDC* pDC);
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
        protected:
        virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
        virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
        virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
        virtual void OnInitialUpdate();

        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CFLLListView();
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif
        HTREEITEM GetItemByName(HTREEITEM hItem, PSZ pszItemName);


        void OpenFolder( PSZ pFolder );
        BOOL FolderNotEmpty();
        BOOL IsRemovedDirNotEmpty();
        int  AddSubFolders( HTREEITEM hFolItem, PSZ pszFolObjName );

        CImageList m_ImageList;
        MDICREATESTRUCT    MDICreate;
        VOID SetMDIStruct( MDICREATESTRUCT * p1 )
                     { memcpy(&MDICreate, p1, sizeof(MDICREATESTRUCT)); return;};
        MDICREATESTRUCT* GetMDIStruct() { return &MDICreate; };
        PGENLISTINSTIDA pIda;

        char m_Buffer[512];            // general purpose buffer
        int GetItemObjName( HTREEITEM hItem, PSZ pszObjName, PBOOL pfIsSubFolder = NULL );

protected:


protected:
        //{{AFX_MSG(CFLLListView)
        afx_msg int  OnCreate( LPCREATESTRUCT cs);
        afx_msg void OnKeyDown( LPNMHDR lpHdr, LRESULT* plResult );
        afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
        afx_msg void ShowNotification( LPNMHDR lpHdr, LRESULT* plResult );
        afx_msg LRESULT On_WM_EQF_MDIACTIVATE( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT OnEQFNDeleted( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT OnEQFNDeletedName( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT OnEQFNNew( WPARAM mp1, LPARAM mp2 );

        afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnRenameFolder( NMHDR* pNMHDR, LRESULT* plResult );
        afx_msg void OnRenameFolderBegin( NMHDR* pNMHDR, LRESULT* plResult );

        afx_msg VOID OnToolbarActivate(CCmdUI* pCmdUI);
        afx_msg void OnUpdateStatusTextCaps(CCmdUI *pCmdUI);
        afx_msg LRESULT OnInitMenuPopup( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT OnInitMenu( WPARAM mp1, LPARAM mp2 );

        afx_msg VOID OnMDICascade()
                { CChildFrame  *pChildFrame = (CChildFrame *)(GetParent()->GetParent());
                  CMDIFrameWnd *pFrameWnd   = (CMDIFrameWnd*)pChildFrame->GetMDIFrame();
                  pFrameWnd->MDICascade();
                };


        afx_msg VOID OnMDITile()
                { CChildFrame  *pChildFrame = (CChildFrame *)(GetParent()->GetParent());
                  CMDIFrameWnd *pFrameWnd   = (CMDIFrameWnd*)pChildFrame->GetMDIFrame();
                  pFrameWnd->MDITile();
                };



        afx_msg VOID OnMDIMinAll();

        afx_msg VOID OnMDIRestore();
        afx_msg VOID OnDetailsDlg( );

        afx_msg LRESULT On_WM_EQF_TERMINATE( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT On_WM_EQF_INSERTNAMES( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT On_WM_EQF_SHOWHTML( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT OnGetCount( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT OnQueryItemState( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT OnGetText( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT OnGetCurSel( WPARAM mp1, LPARAM mp2 );

        afx_msg VOID    OnEBus()  { StartBrowser("home"); } // Start the homepage
        afx_msg VOID    OnEBus1() { StartBrowser("docu"); } // Start the Online Docu
        afx_msg VOID    OnTechGuide() { StartBrowser("techguide"); } // Start the Online Docu
        afx_msg VOID    OnEBus2() { StartBrowser("docuHTML"); } // Start the Online Docu
        afx_msg LRESULT OnFillFolderTree( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT OnDriveChanged(WPARAM mp1, LPARAM mp2);
        //}}AFX_MSG
        afx_msg VOID On_WM_COMMAND ( UINT mp1 );
        afx_msg VOID OnWndActivate ( UINT mp1 );

        DECLARE_MESSAGE_MAP()
};

//#ifndef _DEBUG  // debug version in cpp file
//inline CFLLRightPaneDoc* CFLLRightPaneView::GetDocument()
//   { return (CFLLRightPaneDoc*)m_pDocument; }
//#endif


class CFLLRightPaneView : public CGenListView
{
protected:
        CFLLRightPaneView();
        DECLARE_DYNCREATE(CFLLRightPaneView)

// Attribute
public:
//        CFLLProcView *pParentView;

// Operationen
public:

        //{{AFX_VIRTUAL(CFLLRightPaneView)
        public:
        virtual void OnDraw(CDC* pDC);
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
        protected:
        virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
        virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
        virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
        virtual void OnInitialUpdate();
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CFLLRightPaneView();
        virtual void OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView );
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif
        void FillRightFolderPane( PSZ pFolder );
        void FillRightFolderListPane();
//        void SaveParentView( CFLLProcView *pView );
        PGENLISTINSTIDA GetIda() { return pIda; }
        VOID OnDetailsDlgWrapper( );
        void SaveViewLists();

protected:


protected:
        //{{AFX_MSG(CFLLRightPaneView)
        afx_msg VOID OnLButtonDblClk(UINT uFlags, CPoint Point );
        afx_msg LRESULT On_WM_EQF_MDIACTIVATE( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT OnInitMenuPopup( WPARAM mp1, LPARAM mp2 );
        afx_msg LRESULT On_WM_EQF_OPEN( WPARAM mp1, LPARAM mp2 );

        afx_msg VOID OnDetailsDlg( );
        afx_msg LRESULT On_WM_EQF_TERMINATE( WPARAM mp1, LPARAM mp2 ) ;
        afx_msg VOID OnEBus()  { StartBrowser("home"); }  // Start the homepage
        afx_msg VOID OnEBus1() { StartBrowser("docu"); }  // Start the Online Docu
        afx_msg VOID OnTechGuide() { StartBrowser("techguide"); }  // Start the Online Docu
        afx_msg VOID OnEBus2() { StartBrowser("docuHTML"); }  // Start the Online Docu

        //}}AFX_MSG
        afx_msg VOID On_WM_COMMAND ( UINT i );
        afx_msg VOID OnWndActivate( UINT mp1 ) ;


        DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

