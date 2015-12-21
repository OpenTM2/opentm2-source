//+----------------------------------------------------------------------------+
//|EQFGEN99.H    List window handler for MFC                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

/**********************************************************************/
/* Use this macro for all messages send directly to our generic list  */
/* window procedure in EQFHNDLR.C                                     */
/**********************************************************************/
#define ON_MSG_MACRO( B )   \
   afx_msg LRESULT On_##B( WPARAM mp1, LPARAM mp2 )           \
          { return GENERICLISTWP( m_hWnd, B, mp1, mp2 ); }


class CGenListDoc : public CEQFDoc
{
protected: // create from serialization only
        CGenListDoc();
        DECLARE_DYNCREATE(CGenListDoc)

// Attributes
public:

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CGenListDoc)
        public:
        virtual BOOL OnNewDocument();
        virtual void Serialize(CArchive& ar);
        //}}AFX_VIRTUAL
        PSZ QuerySelFolderName();


// Implementation
public:
        virtual ~CGenListDoc();
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
        //{{AFX_MSG(CGenListDoc)
                // NOTE - the ClassWizard will add and remove member functions here.
                //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////


class CGenListView : public CListViewEx
{
protected: // create from serialization only
        CGenListView();
        DECLARE_DYNCREATE(CGenListView)

// Attributes
public:
        CGenListDoc* GetDocument();
        UINT m_uiFormat;

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CGenListView)
        public:
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
        // cv
        virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
        //
        protected:
        virtual void OnInitialUpdate(); // called first time after construct
        // cv
        virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
        //
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CGenListView();
        void GenLoadFolderNames ( CListCtrl *, PSZ );
        VOID FillColumnHeader( );


        MDICREATESTRUCT    MDICreate;
        VOID SetMDIStruct( MDICREATESTRUCT * p1 )
                     { memcpy(&MDICreate, p1, sizeof(MDICREATESTRUCT)); return;};
        MDICREATESTRUCT* GetMDIStruct() { return &MDICreate; };
        BOOL             fTEnvView()    { return FALSE; };
        VOID               SetChildFrame( CChildFrame * p ) { pChildFrame = p; }

#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:

  // cv
  COleDropTarget m_DropTarget;
  //

// Generated message map functions
protected:
  //{{AFX_MSG(CGenListView)
  afx_msg VOID OnDeleted(SHORT sClass, PSZ pObjData);
  afx_msg VOID OnCreated(SHORT sClass, PSZ pObjData);

  afx_msg VOID OnPropertiesChanged( SHORT sClassId, PSZ pData)
          { CListViewEx::OnPropertiesChanged( sClassId, (LPARAM)pData ); }

  afx_msg VOID OnLButtonDblClk(UINT uFlags, CPoint Point );

  afx_msg void OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags);

  afx_msg int  OnCreate( LPCREATESTRUCT cs);

  afx_msg LRESULT On_WM_CLOSE( WPARAM mp1, LPARAM mp2 );
  afx_msg void OnEBus();
  afx_msg void OnEBus1();
  afx_msg void OnEBus2();

  afx_msg void OnTimer( UINT uTimer )
          { pIda->pfnCallBack( &pIda->CommArea, m_hWnd, WM_TIMER,
                               uTimer, NULL ); };



  /********************************************************************/
  /* all of our EQF special defined messages                          */
  /********************************************************************/
  afx_msg LRESULT On_WM_EQF_INITIALIZE( WPARAM mp1, LPARAM mp2 )
          { return pIda->pfnCallBack( &pIda->CommArea ,
                                      m_hWnd, WM_EQF_INITIALIZE, mp1, mp2 );}

  ON_MSG_MACRO( WM_DESTROY )
  ON_MSG_MACRO( WM_EQF_TERMINATE )
  ON_MSG_MACRO( WM_EQF_SHUTDOWN )
  ON_MSG_MACRO( WM_EQFN_SHUTDOWNCANCELED )
  ON_MSG_MACRO( WM_EQF_QUERYSYSTEMPATH )
  ON_MSG_MACRO( WM_EQF_QUERYSYSPROPPATH )
  ON_MSG_MACRO( WM_EQF_ACTIVATEINSTANCE )
  ON_MSG_MACRO( WM_EQF_QUERYMENUTBL )
  ON_MSG_MACRO( WM_EQF_OPEN )
  ON_MSG_MACRO( WM_EQF_CREATE )
  ON_MSG_MACRO( WM_EQF_DELETE )
  ON_MSG_MACRO( WM_EQF_ABOUTTODELETE )
  ON_MSG_MACRO( WM_EQFN_CREATED )
  ON_MSG_MACRO( WM_EQFN_DELETED )
  ON_MSG_MACRO( WM_EQFN_DELETEDNAME )
  ON_MSG_MACRO( WM_EQFN_PROPERTIESCHANGED )
  ON_MSG_MACRO( WM_EQF_INSERTNAMES )
  ON_MSG_MACRO( WM_EQF_QUERYSELECTEDNAMES )
  ON_MSG_MACRO( WM_EQF_COMMAND )
  ON_MSG_MACRO( WM_EQF_QUERYHANDLER )
  ON_MSG_MACRO( WM_EQF_REMOVEHANDLER )
  ON_MSG_MACRO( WM_EQF_REMOVEOBJECT )
  ON_MSG_MACRO( WM_EQF_QUERYOBJECT )
  ON_MSG_MACRO( WM_EQF_QUERYOBJECTNAME )
  ON_MSG_MACRO( WM_EQF_QUERYOBJECTCLASS )
  ON_MSG_MACRO( WM_EQF_QUERYOBJECTSTATUS )
  ON_MSG_MACRO( WM_EQF_SETOBJECTSTATUS )
  ON_MSG_MACRO( WM_EQF_QUERYOBJECTCOUNT )
  ON_MSG_MACRO( WM_EQF_GETOBJECTLIST )
  ON_MSG_MACRO( WM_EQF_QUERYHANDLERCOUNT )
  ON_MSG_MACRO( WM_EQF_SETSYMBOL )
  ON_MSG_MACRO( WM_EQF_QUERYSYMBOL )
  ON_MSG_MACRO( WM_EQF_REMOVESYMBOL )
  ON_MSG_MACRO( WM_EQFN_OBJECTREMOVED )
  ON_MSG_MACRO( WM_EQF_QUERYSYSTEMPROPHND )
  ON_MSG_MACRO( WM_EQF_OPENPROPERTIES )
  ON_MSG_MACRO( WM_EQF_CREATEPROPERTIES )
  ON_MSG_MACRO( WM_EQF_DELETEPROPERTIES )
  ON_MSG_MACRO( WM_EQF_CLOSEPROPERTIES )
  ON_MSG_MACRO( WM_EQF_GETALLPROPERTIES )
  ON_MSG_MACRO( WM_EQF_PUTALLPROPERTIES )
  ON_MSG_MACRO( WM_EQF_SAVEPROPERTIES )
  ON_MSG_MACRO( WM_EQF_QUERYPROPSIZE )
  ON_MSG_MACRO( WM_EQF_REGISTER )
  ON_MSG_MACRO( WM_EQF_REINIT )
  ON_MSG_MACRO( WM_EQF_LSTREGISTER )
  ON_MSG_MACRO( WM_EQF_LSTEVENT )
  ON_MSG_MACRO( WM_EQF_LSTQRYLOCK )
  ON_MSG_MACRO( WM_EQF_LSTSETLOCK )
  ON_MSG_MACRO( WM_EQF_REFRESH )
  ON_MSG_MACRO( WM_EQF_SETFOCUS )
  ON_MSG_MACRO( WM_EQF_NEXTSTEP )
  ON_MSG_MACRO( WM_EQF_PROCESSTASK )
  ON_MSG_MACRO( WM_EQFN_TASKDONE )
  ON_MSG_MACRO( WM_EQF_ABOUTTOREMOVEDRIVE )
  ON_MSG_MACRO( WM_EQF_SLIDER_POSITION )
  ON_MSG_MACRO( WM_EQF_SLIDER_SETTEXT )
  ON_MSG_MACRO( WM_EQF_MEMLOAD_START )
  ON_MSG_MACRO( WM_EQF_MEMLOAD_PROCESS )
  ON_MSG_MACRO( WM_EQF_MEMLOAD_END )
  ON_MSG_MACRO( WM_EQF_MEMFILL_LISTBOX )
  ON_MSG_MACRO( WM_EQF_MEMEXPORT_START )
  ON_MSG_MACRO( WM_EQF_MEMEXPORT_PROCESS )
  ON_MSG_MACRO( WM_EQF_MEMEXPORT_END )
  ON_MSG_MACRO( WM_EQF_MEMORGANIZE_START )
  ON_MSG_MACRO( WM_EQF_MEMORGANIZE_PROCESS )
  ON_MSG_MACRO( WM_EQF_MEMORGANIZE_END )
  ON_MSG_MACRO( WM_EQF_MEMMERGE_START )
  ON_MSG_MACRO( WM_EQF_MEMMERGE_PROCESS )
  ON_MSG_MACRO( WM_EQF_MEMMERGE_END )
  ON_MSG_MACRO( WM_EQF_QUERY )
  ON_MSG_MACRO( WM_EQF_FONTCHANGED )
  ON_MSG_MACRO( WM_EQF_MEMORGANIZE_ACTIVATE )
  ON_MSG_MACRO( WM_DRIVEBUTTON_INIT )
  ON_MSG_MACRO( WM_DRIVEBUTTON_SELECT )
  ON_MSG_MACRO( WM_EQF_DDE_REQUEST )
  ON_MSG_MACRO( WM_EQF_DDE_ANSWER )

  ON_MSG_MACRO( WM_EQFN_DRIVEREMOVED )
  ON_MSG_MACRO( WM_EQFN_DRIVEADDED )
  ON_MSG_MACRO( WM_EQF_COLCHANGED )
  ON_MSG_MACRO( WM_EQF_WD_MAIN_NOTIFY )
  ON_MSG_MACRO( WM_EQF_INITMENU )
  ON_MSG_MACRO( WM_EQF_BUILDITEMTEXT )
  ON_MSG_MACRO( WM_EQF_CREATELISTWINDOW )
  ON_MSG_MACRO( WM_EQF_UPDATESLIDER )
  ON_MSG_MACRO( WM_EQF_SHOWPOPUP )
  ON_MSG_MACRO( EQF_CD_BM_CLICK )
  ON_MSG_MACRO( WM_EQF_CHECK_REM_PROPS )
  ON_MSG_MACRO( WM_EQF_FILTSETDICT )
  ON_MSG_MACRO( WM_EQF_QUERYID )
  ON_MSG_MACRO( WM_EQF_CHANGEOBJECTNAME )
  ON_MSG_MACRO( WM_EQF_CLOSE )
  ON_MSG_MACRO( LM_EQF_GETITEMFROMPOINT )
  ON_MSG_MACRO( LM_EQF_REFRESH )
  ON_MSG_MACRO( WM_INITMENU )

  afx_msg LRESULT On_WM_EQF_SHOWHTML( WPARAM mp1, LPARAM mp2 );

  afx_msg LRESULT On_WM_EQF_TAGTABLEPROPS( WPARAM mp1, LPARAM mp2 );


  afx_msg void  OnViewTWBToolBar();
  afx_msg void  OnViewTPROToolBar();
  afx_msg void  OnViewStatusBar();

  afx_msg void  OnToolbarActivate(CCmdUI* pCmdUI);

  //cv
  afx_msg void OnFileCopy();
  afx_msg void OnFileCut();
  afx_msg void OnFilePaste();
  afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
  //

  afx_msg LRESULT On_WM_INITMENUPOPUP( WPARAM mp1, LPARAM mp2 );

  afx_msg LRESULT On_WM_EQF_MDIACTIVATE( WPARAM mp1, LPARAM mp2 );
  afx_msg VOID OnMDICascade()
          { ((CMDIChildWnd *)GetParent())->GetMDIFrame()->MDICascade(); };

  afx_msg VOID OnMDITile()
          { ((CMDIChildWnd *)GetParent())->GetMDIFrame()->MDITile(); };

  afx_msg VOID OnMDIMinAll();

  afx_msg VOID OnMDIRestore();

  /********************************************************************/
  /* special handling for our listbox control                         */
  /********************************************************************/

  afx_msg LRESULT On_LM_EQF_QUERYVIEWLIST( WPARAM mp1, LPARAM mp2 )
          {  return QueryView( mp1, mp2 ); }
  afx_msg LRESULT On_LM_EQF_SETVIEWLIST( WPARAM mp1, LPARAM mp2 )
          {  return SetView( mp1, mp2 ); }
  afx_msg LRESULT On_LM_EQF_QUERYSORTLIST( WPARAM mp1, LPARAM mp2 )
          { mp1; QuerySortView( (PSHORT)mp2 ); return( 0 ); }
  afx_msg LRESULT On_LM_EQF_SETSORTLIST( WPARAM mp1, LPARAM mp2 )
          { mp1; SetSortView( (PSHORT)mp2 ); return( 0 ); }
  afx_msg LRESULT On_LM_EQF_QUERYFILTER( WPARAM mp1, LPARAM mp2 )
          { mp1; return QueryFilter( (PCLBFILTER)mp2 ); }
  afx_msg LRESULT On_LM_EQF_SETFILTER( WPARAM mp1, LPARAM mp2 )
          { mp1; return SetFilter( (PCLBFILTER)mp2 ); }
  afx_msg LRESULT On_LM_EQF_SETITEMSTATE( WPARAM mp1, LPARAM mp2 )
          {  return SetItemState( (SHORT) mp1, (USHORT) mp2 ); }
  afx_msg LRESULT On_LM_EQF_QUERYITEMSTATE( WPARAM mp1, LPARAM mp2 )
          { mp2; return GetItemState( (SHORT) mp1 ); }

  ON_MSG_MACRO( LM_EQF_INSERTITEMSTATE )

  afx_msg LRESULT OnEqfQueryItemText( WPARAM mp1, LPARAM mp2 );


  afx_msg LRESULT OnResetContent( WPARAM mp1, LPARAM mp2 )
          { mp1; mp2; return GetListCtrl().DeleteAllItems(); }

  afx_msg LRESULT OnAddString( WPARAM mp1, LPARAM mp2 );

  afx_msg LRESULT OnInsertString( WPARAM mp1, LPARAM mp2 )
          { return OnAddString( mp1, mp2 ); }
//          { return InsertRow( (PSZ) mp2, (SHORT) mp1); }

  afx_msg LRESULT OnDeleteString( WPARAM mp1, LPARAM mp2 )
          { mp2; return DeleteItem( (SHORT) mp1 ); }

  afx_msg LRESULT OnSetItemData( WPARAM mp1, LPARAM mp2 )
          { return SetItemData( (SHORT) mp1, (LONG) mp2 ); }

  afx_msg LRESULT OnFindString( WPARAM mp1, LPARAM mp2 )
          { mp1; return FindRow( (PSZ) mp2 ); }

  afx_msg LRESULT OnQueryItemHandle( WPARAM mp1, LPARAM mp2 )
          { mp2; return GetItemData( (SHORT) mp1 ); }

  afx_msg LRESULT OnQuerySelection( WPARAM mp1, LPARAM mp2 )
          { mp1; mp2; return QuerySelection(); }

  afx_msg LRESULT OnSetSelection(WPARAM mp1, LPARAM mp2 )
          { mp2; return ((SetSelection((SHORT) mp1)) ? 0 : LB_ERR); }

  afx_msg LRESULT OnSetMultSel(WPARAM mp1, LPARAM mp2 )
          { mp1; return ((SetSelection((SHORT) LOWORD(mp2))) ? 0 : LB_ERR); }

  afx_msg LRESULT OnQueryItemText( WPARAM mp1, LPARAM mp2 );
  afx_msg LRESULT OnQueryItemState( WPARAM mp1, LPARAM mp2 )
          { mp2;return GetItemState( (SHORT) mp1 ); }

  afx_msg VOID    OnQueryViewList( WPARAM mp1, LPARAM mp2 );

  afx_msg LRESULT OnGetCount( WPARAM mp1, LPARAM mp2)
          { mp1; mp2; return  GetListCtrl().GetItemCount(); }

  afx_msg LRESULT OnGetSelCount( WPARAM mp1, LPARAM mp2 )
          { mp1; mp2; return GetListCtrl().GetSelectedCount(); }

  afx_msg LRESULT OnGetSelItems( WPARAM mp1, LPARAM mp2 )
          { return GetSelectedItems( (SHORT) mp1, (LPINT) mp2 ); }

  afx_msg LRESULT OnSetTopIndex(WPARAM mp1, LPARAM mp2 )
          { mp2; return ((SetTopIndex((SHORT) mp1)) ? 0 : LB_ERR); }

  afx_msg VOID    OnTQMLogon();
  afx_msg VOID    OnTQMLogoff();
  afx_msg VOID    OnTQMUser();
  afx_msg VOID    OnTQMEvaluation();
  afx_msg VOID    OnTQMVendor();
  afx_msg VOID    OnTQMReports();
  afx_msg VOID    OnTQMSettings();
  afx_msg VOID    OnTQMProjects();
  afx_msg VOID    OnTQMArchive();
  afx_msg void    OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult);

  afx_msg VOID    A4( WPARAM mp1, LPARAM mp2 );
  //}}AFX_MSG
  afx_msg VOID    OnWndActivate( UINT mp1 );
  afx_msg void    OnWMCOMMAND( UINT mp1 );

  DECLARE_MESSAGE_MAP()

  void OnUpdateStatusText(CCmdUI *pCmdUI);
  void OnUpdateStatusTextCaps(CCmdUI *pCmdUI);

// view type helpers
public:

// flags and states
protected:
    BOOL fInitialUpdate;
    CChildFrame* pChildFrame;
};

#ifndef _DEBUG  // debug version in cpp file
inline CGenListDoc* CGenListView::GetDocument()
   { return (CGenListDoc*)m_pDocument; }
#endif

inline VOID  CGenListView::OnQueryViewList( WPARAM mp1, LPARAM mp2 )
   {
     switch ( mp1 )
     {
       case CURRENT_VIEW:
       default:
         memcpy( (PVOID)mp2, asCurView, sizeof( asCurView ));
         break;
       case DETAILS_VIEW:
         memcpy( (PVOID)mp2, asDetailsView, sizeof( asDetailsView ));
         break;
     };
   }

#undef ON_MSG_MACRO
/////////////////////////////////////////////////////////////////////////////

