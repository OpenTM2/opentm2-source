//+----------------------------------------------------------------------------+
//|  EQFCLBOX.H - EQF Column listbox class                                     |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+
//|  Entry Points:                                                             |
//+----------------------------------------------------------------------------+
//|  Externals:                                                                |
//+----------------------------------------------------------------------------+
//|  Internals:                                                                |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//+----------------------------------------------------------------------------+
//|  To be done / known limitations / caveats:                                 |
//|                                                                            |
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
// $Revision: 1.2 $ ----------- 29 Mar 2007
// GQ: - added OnExit prototype
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
// $Revision: 1.4 $ ----------- 8 Oct 2004
// --RJ: accessibility: CListViewEx: add OnSysKeyDown function
// 
//
// $Revision: 1.3 $ ----------- 29 Sep 2004
// --RJ: accessibility: add De/Select pushbutton to view dialog
//
//
// $Revision: 1.2 $ ----------- 28 Sep 2004
// GQ: - added OnHelp methods to dialog classes
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
// $Revision: 1.2 $ ----------- 5 Nov 2003
// GQ: - changed size of view column arrays from 21 to MAX_DEFINEDCOLUMNS
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
// $Revision: 1.3 $ ----------- 17 Mar 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
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
// $Revision: 1.2 $ ----------- 20 Nov 2002
// GQ: - added OnMTList method
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.5 $ ----------- 26 Oct 2001
// CV (10/26/01)
// -- OnHeaderchanged(), new function prototype
//
//
// $Revision: 1.4 $ ----------- 16 Oct 2001
// R005316 Keep column layout as defines by the user added
//
//
// $Revision: 1.3 $ ----------- 9 Oct 2001
// GQ: Added sMultSelPopUpId to member variables of ClistViewEx class
//
//
// $Revision: 1.2 $ ----------- 11 Sep 2001
// increased general field index to 21 (details view) (R06520)
//
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.9 $ ----------- 5 Jun 2001
// GQ: Added view shrink and view hide methods
//
//
// $Revision: 1.8 $ ----------- 28 May 2001
// GQ: Added prototype for SetItemText and CheckFilterMatch methods
//
//
// $Revision: 1.7 $ ----------- 6 Nov 2000
// -- add OnConfDrives, OnConnectResources
//
//
// $Revision: 1.6 $ ----------- 18 Jul 2000
// - added CFiltDlg class declaration and declared handlers for filter messages
//
//
// $Revision: 1.5 $ ----------- 22 May 2000
// - added CSortDlg class declaration
//
//
// $Revision: 1.4 $ ----------- 4 May 2000
// -- add ClearCLBox for deleting everything in list
// - some modifications for TQM
//
//
// $Revision: 1.3 $ ----------- 10 Jan 2000
// - added system properties handling
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFCLBOX.HV_   1.6   08 Feb 1999 16:15:52   BUILD  $
 *
 * $Log:   K:\DATA\EQFCLBOX.HV_  $
 *
 *    Rev 1.6   08 Feb 1999 16:15:52   BUILD
 * -- image list support
 *
 *    Rev 1.5   01 Feb 1999 09:47:16   BUILD
 * -- add functions for PrintList
 *
 *    Rev 1.4   11 Jan 1999 11:16:08   BUILD
 * No change.
 *
 *    Rev 1.3   11 Jan 1999 10:47:38   BUILD
 * -- add QueryView, SetView, QuerySortView, SetSortView functions
 *
 *    Rev 1.2   07 Dec 1998 10:28:14   BUILD
 * -- add SetSelection, SetTopIndex
 *
 *    Rev 1.1   09 Nov 1998 09:35:22   BUILD
 * -- add support for more than 300 files
 *
 *    Rev 1.0   26 Oct 1998 19:15:04   BUILD
 * Initial revision.
 *
*/
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

        // Generated message map functions
        //{{AFX_MSG(CDetailView)
        afx_msg void OnDblclkAvail();
        afx_msg void OnDblclkSelect();
        afx_msg void OnSelect();
        afx_msg void OnDeselect();
        virtual BOOL OnInitDialog();
        afx_msg void OnClear();
        afx_msg void OnHelp();
        afx_msg void OnView();
        afx_msg void OnCancel();
        afx_msg LRESULT OnDefID( WPARAM mp1, LPARAM mp2);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
public:
        CHAR   ColumnHeader[MAX_DEFINEDCOLUMNS][80];   // Buffer for column header texts
        USHORT usColumnWidth[MAX_DEFINEDCOLUMNS];      //   ...   width
        SHORT  asDetailsView[MAX_DEFINEDCOLUMNS];
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

