//+----------------------------------------------------------------------------+
//|  EQFCLBOX.H - EQF Column listbox class                                     |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#include "eqf.id"

typedef struct _CLBOXCOLDATA             // data of a single column
{
   USHORT       usId;                  // id of item
   USHORT       usWidth;               // width of column in characters!
   CLBDATATYPE  DataType;              // type of column
   USHORT       usAdjust;              // adjustment
} CLBOXCOLDATA, *PCLBOXCOLDATA;


class CListViewEx : public CListView
{
        DECLARE_DYNCREATE(CListViewEx)

// Construction
public:
        CListViewEx();

// Attributes
protected:
        BOOL m_bFullRowSel;

public:
        BOOL SetFullRowSel(BOOL bFillRowSel);
        BOOL GetFullRowSel();

        BOOL m_bClientWidthSel;

// Overrides
protected:
        virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CListViewEx)
        public:
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
        protected:
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CListViewEx();
        PSZ      GetColumnHeader()  { return &ColumnHeader[0][0]; };
        PUSHORT  GetColumnWidth()   { return &usColumnWidth[0]; };
        PSHORT   GetCurrentView()   { return asCurView; };
        PSHORT   GetDetailsView()   { return asDetailsView; };
        PSHORT   GetSortView()      { return asSortView; };
        VOID     SetDetailsView( PSHORT psDetail )
                  { memcpy( asDetailsView, psDetail, sizeof(asDetailsView)); };
        VOID     SetSortView( PSHORT psSort )
                  { memcpy( asSortView, psSort, sizeof(asSortView)); };
        VOID     SetCurrentView( PSHORT psCurrent )
                  { memcpy( asCurView, psCurrent, sizeof(asCurView)); };
        VOID     Sort();
        VOID     Filter();
        LRESULT  QueryView( WPARAM mp1, LPARAM mp2 );
        LRESULT  SetView( WPARAM mp1, LPARAM mp2 );
        VOID     QuerySortView( PSHORT psSort )
                  { memcpy( psSort, asSortView, sizeof(asSortView)); };
        LRESULT  QueryFilter( PCLBFILTER pFilter )
                  { memcpy( pFilter, &m_Filter, sizeof(m_Filter)); return( 0 );};
        LRESULT  SetFilter( PCLBFILTER pFilter )
                  { memcpy( &m_Filter, pFilter, sizeof(m_Filter)); return( 0 );};
        VOID     ClearCLBox();
        VOID     UpdateCLBox();
        BOOL     CheckFilterMatch( PSZ pszItemData );
        VOID     DeleteRow( PSZ pszBuffer );
        BOOL     DeleteItem( int iItem );
        BOOL     SetItemData( int iItem, LONG lData );
        LONG     GetItemData( int iItem );
        int      FindRow( PSZ pszBuffer );
        SHORT    GetClassId()       { return sClassId; };
        VOID     SetClassId( SHORT sClass )       { sClassId = sClass; };
        int      QuerySelection( );
        BOOL     SetSelection( int iItem );
        BOOL     SetTopIndex( int iItem );
        BOOL     SetItemState( int, USHORT );
        USHORT   GetItemState( int );
        SHORT    GetSelectedItems( SHORT sMaxItems, LPINT lpnItems );
        void     FillColumnHeaders( PCLBOXCOLDATA pColData );

        void     OnDetailsDlg();
        void     OnSortDlg();
        void     OnFilterDlg();
        void     OnAll();
        void     OnNamesView();
        void     OnDetailsView();

        void     OnShrinkPath();
        void     OnHidePath();
        void     OnShowPath();
        void     PrepareNameColumn( int iColumn, PSZ pszColText, PSZ pszBuffer );

        VOID     SelectAll( );
        VOID     DeSelectAll( );

//        VOID     OnConfDrives();
        VOID     OnTagTable();
        VOID     OnMTList();
        VOID     OnLangList();
        VOID     OnExclList();
        VOID     OnNewTerms();
        VOID     OnFoundTerms();
        VOID     OnArchiveTM();
        VOID     OnCreateReport();
        VOID     OnWordCount();
        VOID     OnPrintList();
        VOID     OnSysProps();
        VOID     OnExit();
        VOID     OnConfDrives();
        VOID     OnPluginManager();
        VOID     OnAutoVersionUp();
        VOID     OnConnectResources();

        PGENLISTINSTIDA pIda;

#ifdef _DEBUG
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:
        static LPCTSTR MakeShortString(CDC* pDC, LPCTSTR lpszLong, int nColumnLen, int nOffset);
        void RepaintSelectedItems();
        VOID InsertColumnItems( int iInsertedItem, PSZ pszBuffer );
        BOOL CLBPrintList();

// Implementation - client area width
        int m_cxClient;

// Implementation - state icon width
        int m_cxStateImageOffset;
        afx_msg LRESULT OnSetImageList(WPARAM wParam, LPARAM lParam);

// Implementation - list view colors
        COLORREF m_clrText;
        COLORREF m_clrTextBk;
        COLORREF m_clrBkgnd;
        afx_msg LRESULT OnSetTextColor(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnSetTextBkColor(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnSetBkColor(WPARAM wParam, LPARAM lParam);

// Implementation - columns selected on different views
        SHORT  asCurView[ MAX_VIEW+1 ];
        SHORT  asDetailsView[ MAX_VIEW+1 ];
        SHORT  asSortView[ MAX_VIEW+1 ];
        SHORT  asNameView[ MAX_VIEW+1 ];
        CHAR   ColumnHeader[MAX_DEFINEDCOLUMNS][80];   // Buffer for column header texts
        USHORT usColumnWidth[MAX_DEFINEDCOLUMNS];      //   ...   width
        USHORT usColumnType[MAX_DEFINEDCOLUMNS];       //       ... type
        USHORT usColumnAdjust[MAX_DEFINEDCOLUMNS];
        SHORT  sPopUpId;               // Popup Id with selection
        SHORT  sMultSelPopUpId;        // Popup Id with selection of more than one item
        SHORT  sNoSelPopUpId;          // popup id for w/o selection
        SHORT  sClassId;               // display class
        SHORT  sPropClass;             // property class of object
        CLBFILTER m_Filter;            // currently active filter
        CImageList m_ImageList;

        void   InsertTitle();
        int    InsertRow( PSZ p, int iItem = 0, SHORT sImage = 0 );

        char   chItemBuffer[256];

// Generated message map functions
protected:
        //{{AFX_MSG(CListViewEx)
        afx_msg void OnMove(int cx, int cy);

        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg void OnPaint();
        afx_msg void OnSetFocus(CWnd* pOldWnd);
        afx_msg void OnKillFocus(CWnd* pNewWnd);
        afx_msg void OnDestroy();
     //   afx_msg void OnKeyDown( UINT mp1, UINT cx, UINT cy );
        afx_msg void OnSysKeyDown( UINT mp1, UINT cx, UINT cy );
        afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
        afx_msg LRESULT OnDeleted( WPARAM wParam, LPARAM lParam );
        afx_msg LRESULT OnInsertNames(WPARAM mp1, LPARAM mp2);
        afx_msg LRESULT OnGetColumnWidth(WPARAM mp1, LPARAM mp2);
        afx_msg LRESULT OnCreated( WPARAM mp1, LPARAM mp2);
        afx_msg LRESULT OnSetItemText( WPARAM mp1, LPARAM mp2);
        afx_msg LRESULT OnPropertiesChanged( WPARAM mp1, LPARAM mp2);
        afx_msg SHORT OnSelectedNames( HWND hwndTargetLB );

        afx_msg void  OnHeaderClick(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
        //cv
        afx_msg void  OnHeaderchanged(NMHDR* pNMHDR, LRESULT* pResult);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CDetailView dialog

class CDetailView : public CDialog
{
// Construction
public:
        CDetailView(CWnd* pParent = NULL);   // standard constructor
        ~CDetailView();                      // standard destructor

// Dialog Data
        //{{AFX_DATA(CDetailView)
        enum { IDD = ID_SELVIEW_DLG };
                // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CDetailView)
        protected:
        //}}AFX_VIRTUAL

// Implementation
protected:
        // remove the element from selected list and add it to the available list
        void RemoveFromSelected( CListBox *pLBoxAvail, CListBox *pLBoxSelected, int i );

        // get the name for the defaults file
        void GetDefaultsFileName( int iPopupID, PSZ pszName );

        // refill available and selected listbox using given viewlist
        void CDetailView::RefillListboxes( PSHORT sViewList );


        // Generated message map functions
        //{{AFX_MSG(CDetailView)
        afx_msg void OnDblclkAvail();
        afx_msg void OnDblclkSelect();
        afx_msg void OnSelect();
        afx_msg void OnDeselect();
        afx_msg void OnUp();
        afx_msg void OnDown();
        afx_msg void OnSaveDefault();
        afx_msg void OnLoadDefault();

        virtual BOOL OnInitDialog();
        afx_msg void OnClear();
        afx_msg void OnHelp();
        afx_msg void OnView();
        afx_msg void OnCancel();
        afx_msg LRESULT OnDefID( WPARAM mp1, LPARAM mp2);
        afx_msg BOOL PreTranslateMessage( MSG* pMsg );
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
public:
        CToolTipCtrl* m_pToolTip;                      // tool tip control
        CHAR   ColumnHeader[MAX_DEFINEDCOLUMNS][80];   // Buffer for column header texts
        USHORT usColumnWidth[MAX_DEFINEDCOLUMNS];      //   ...   width
        SHORT  asDetailsView[MAX_DEFINEDCOLUMNS];
        SHORT  sPopupID;                               // ID of popup dialog for this window (is used to get the type of window)
};



/////////////////////////////////////////////////////////////////////////////
// CSortDlg dialog

class CSortDlg : public CDialog
{
// Construction
public:
        CSortDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
        //{{AFX_DATA(CSortDlg)
        enum { IDD = ID_SORT_DLG };
                // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CSortDlg)
        protected:
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CSortDlg)
        virtual BOOL OnInitDialog();
        afx_msg void OnSort();
        afx_msg void OnHelp();
        afx_msg void OnCancel();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
public:
        CHAR   ColumnHeader[MAX_DEFINEDCOLUMNS][80];   // Buffer for column header texts
        USHORT usColumnWidth[MAX_DEFINEDCOLUMNS];      //   ...   width
        SHORT  asSortView[ MAX_DEFINEDCOLUMNS];
};


/////////////////////////////////////////////////////////////////////////////
// CFilterDlg dialog

class CFilterDlg : public CDialog
{
// Construction
public:
        CFilterDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
        //{{AFX_DATA(CSortDlg)
        enum { IDD = ID_FILT2_DLG };
                // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CSortDlg)
        protected:
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CSortDlg)
        virtual BOOL OnInitDialog();
        afx_msg void OnFilter();
        afx_msg void OnHelp();
        afx_msg void OnCancel();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
public:
        CHAR   ColumnHeader[MAX_DEFINEDCOLUMNS][80];   // Buffer for column header texts
        USHORT usColumnWidth[MAX_DEFINEDCOLUMNS];      //   ...   width
        CHAR   szBuffer[1024];
        CLBFILTER Filter;
};




class CEQFDoc : public CDocument
{
protected: // create from serialization only
        CEQFDoc();
        DECLARE_DYNCREATE(CEQFDoc)

// Attributes
public:

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CEQFDoc)
        public:
        virtual BOOL OnNewDocument();
        virtual void Serialize(CArchive& ar);
        //}}AFX_VIRTUAL
        VOID OnDetailsDlg();
        VOID OnNamesView();
        VOID OnDetailsView();
        VOID OnConfDrives();
        VOID OnConnectResources();
        VOID OnTagTable();
        VOID OnMTList();
        VOID OnLangList();
        VOID OnExclList();
        VOID OnNewTerms();
        VOID OnFoundTerms();
        VOID OnArchiveTM();
        VOID OnCreateReport();
        VOID OnWordCount();
        VOID OnPrintList();


// Implementation
public:
        virtual ~CEQFDoc();
        virtual BOOL CanCloseFrame( CFrameWnd * pFrame );
        VOID     SetObjName( PSZ pBuffer ) { strObjName = pBuffer; };
        PSZ      GetObjName()              { return (PSZ) LPCTSTR(strObjName); };
        LRESULT TwbInitMenu( HWND hwnd, WPARAM mp1, LPARAM mp2 );


#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:
        CString strObjName;             // object name;
        char chBuffer[265];
        BOOL fUserFlag;
        BOOL fNoClose;
        CView* pActView;

// Generated message map functions
protected:
        //{{AFX_MSG(CEQFDoc)
                // NOTE - the ClassWizard will add and remove member functions here.
                //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

