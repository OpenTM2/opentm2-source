//+----------------------------------------------------------------------------+
//|EQFCFRM.H     Interface for the CChildFrame class                           |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
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
// $Revision: 1.7 $ ----------- 18 Jun 2001
// -- RJ: fix problem in maximizing MDI Dialogs
//
//
// $Revision: 1.6 $ ----------- 28 May 2001
// GQ: Added handling for LM_EQF_SETITEMTEXT message
//
//
// $Revision: 1.5 $ ----------- 20 Dec 2000
// - added handling for EQFN_DELETEDNAME message
//
//
// $Revision: 1.4 $ ----------- 6 Nov 2000
// -- add OnCreateClient to correctly support splitter window
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
 * $Header:   K:\DATA\EQFCFRM.HV_   1.4   15 Feb 1999 08:11:16   BUILD  $
 *
 * $Log:   K:\DATA\EQFCFRM.HV_  $
 *
 *    Rev 1.4   15 Feb 1999 08:11:16   BUILD
 * -- get rid of unused functions
 *
 *    Rev 1.3   18 Jan 1999 10:26:32   BUILD
 * -- move OnMDIMinAll to correct place
 *
 *    Rev 1.2   11 Jan 1999 10:43:38   BUILD
 * -- add windows pulldown handling...
 *
 *    Rev 1.1   07 Dec 1998 10:28:06   BUILD
 * -- support restore saved size
 *
 *    Rev 1.0   26 Oct 1998 19:11:38   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#if !defined(AFX_CHILDFRM_H__2EDD418B_CCC9_11D1_8162_00AA0026430D__INCLUDED_)
#define AFX_CHILDFRM_H__2EDD418B_CCC9_11D1_8162_00AA0026430D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CChildFrame : public CMDIChildWnd
{
        DECLARE_DYNCREATE(CChildFrame)
public:
        CChildFrame();

// Attributes
public:

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CChildFrame)
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CChildFrame();
        virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif
        VOID    OnToggleWindow( UINT mp1 );
        MRESULT OnWindowsPopup( WPARAM mp1, LPARAM mp2 );

        void    OnMDIMinAll();
        void    OnMDIRestore();
        CSplitterWnd m_wndSplitter;
// Generated message map functions
protected:


  //{{AFX_MSG(CChildFrame)

  afx_msg void    OnCascade()     { GetMDIFrame()->MDICascade(); };
  afx_msg void    OnTile()        { GetMDIFrame()->MDITile(); };
  afx_msg void    OnMinAll()      { OnMDIMinAll();  };
  afx_msg void    OnRestore()     { OnMDIRestore(); };


  afx_msg void OnMDIActivate(BOOL fAct, CWnd* pActWnd, CWnd* pDeActWnd );

  afx_msg void OnMDIDestroy( );
  afx_msg MRESULT On_WM_CLOSE( WPARAM mp1, LPARAM mp2 );
  afx_msg MRESULT OnInitMenu( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_INITMENU, mp1, mp2 ); }

  afx_msg MRESULT OnInitMenuPopup( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_INITMENUPOPUP, mp1, mp2 ); }
  /********************************************************************/
  /* all of our EQF special defined messages                          */
  /********************************************************************/
  afx_msg MRESULT On_WM_TIMER( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_TIMER, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_INITIALIZE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_INITIALIZE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_TERMINATE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_TERMINATE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_SHUTDOWN( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_SHUTDOWN, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQFN_SHUTDOWNCANCELED( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQFN_SHUTDOWNCANCELED, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYSYSTEMPATH( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYSYSTEMPATH, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYSYSPROPPATH( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYSYSPROPPATH, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_ACTIVATEINSTANCE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_ACTIVATEINSTANCE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYMENUTBL( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYMENUTBL, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_OPEN( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_OPEN, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_CREATE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_CREATE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_DELETE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_DELETE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_ABOUTTODELETE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_ABOUTTODELETE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQFN_CREATED( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQFN_CREATED, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQFN_DELETED( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQFN_DELETED, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQFN_DELETEDNAME( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQFN_DELETEDNAME, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQFN_PROPERTIESCHANGED( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQFN_PROPERTIESCHANGED, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_INSERTNAMES( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_INSERTNAMES, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYSELECTEDNAMES( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYSELECTEDNAMES, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_COMMAND( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_COMMAND, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYHANDLER( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYHANDLER, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_REMOVEHANDLER( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_REMOVEHANDLER, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_REMOVEOBJECT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_REMOVEOBJECT, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYOBJECT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYOBJECT, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYOBJECTNAME( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYOBJECTNAME, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYOBJECTCLASS( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYOBJECTCLASS, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYOBJECTSTATUS( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYOBJECTSTATUS, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_SETOBJECTSTATUS( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_SETOBJECTSTATUS, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYOBJECTCOUNT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYOBJECTCOUNT, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_GETOBJECTLIST( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_GETOBJECTLIST, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYHANDLERCOUNT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYHANDLERCOUNT, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_SETSYMBOL( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_SETSYMBOL, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYSYMBOL( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYSYMBOL, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_REMOVESYMBOL( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_REMOVESYMBOL, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQFN_OBJECTREMOVED( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQFN_OBJECTREMOVED, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYSYSTEMPROPHND( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYSYSTEMPROPHND, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_OPENPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_OPENPROPERTIES, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_CREATEPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_CREATEPROPERTIES, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_DELETEPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_DELETEPROPERTIES, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_CLOSEPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_CLOSEPROPERTIES, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_GETALLPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_GETALLPROPERTIES, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_PUTALLPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_PUTALLPROPERTIES, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_SAVEPROPERTIES( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_SAVEPROPERTIES, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYPROPSIZE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYPROPSIZE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_REGISTER( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_REGISTER, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_REINIT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_REINIT, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_LSTREGISTER( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_LSTREGISTER, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_LSTEVENT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_LSTEVENT, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_LSTQRYLOCK( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_LSTQRYLOCK, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_LSTSETLOCK( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_LSTSETLOCK, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_REFRESH( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_REFRESH, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_SETFOCUS( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_SETFOCUS, mp1, mp2 ); }
  afx_msg MRESULT On_LM_EQF_QUERYVIEWLIST( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LM_EQF_QUERYVIEWLIST, mp1, mp2 ); }
  afx_msg MRESULT On_LM_EQF_SETVIEWLIST( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LM_EQF_SETVIEWLIST, mp1, mp2 ); }
  afx_msg MRESULT On_LM_EQF_SETSORTLIST( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LM_EQF_SETSORTLIST, mp1, mp2 ); }
  afx_msg MRESULT On_LM_EQF_QUERYSORTLIST( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LM_EQF_QUERYSORTLIST, mp1, mp2 ); }
  afx_msg MRESULT On_LM_EQF_SETFILTER( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LM_EQF_SETFILTER, mp1, mp2 ); }
  afx_msg MRESULT On_LM_EQF_QUERYFILTER( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LM_EQF_QUERYFILTER, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_NEXTSTEP( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_NEXTSTEP, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_PROCESSTASK( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_PROCESSTASK, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQFN_TASKDONE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQFN_TASKDONE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_ABOUTTOREMOVEDRIVE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_ABOUTTOREMOVEDRIVE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_SLIDER_POSITION( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_SLIDER_POSITION, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_SLIDER_SETTEXT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_SLIDER_SETTEXT, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMLOAD_START( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMLOAD_START, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMLOAD_PROCESS( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMLOAD_PROCESS, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMLOAD_END( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMLOAD_END, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMFILL_LISTBOX( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMFILL_LISTBOX, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMEXPORT_START( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMEXPORT_START, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMEXPORT_PROCESS( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMEXPORT_PROCESS, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMEXPORT_END( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMEXPORT_END, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMORGANIZE_START( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMORGANIZE_START, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMORGANIZE_PROCESS( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMORGANIZE_PROCESS, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMORGANIZE_END( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMORGANIZE_END, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMMERGE_START( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMMERGE_START, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMMERGE_PROCESS( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMMERGE_PROCESS, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMMERGE_END( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMMERGE_END, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERY( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERY, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_FONTCHANGED( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_FONTCHANGED, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_COM_INITIALIZE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_COM_INITIALIZE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_COM_EXIT_COMPLETE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_COM_EXIT_COMPLETE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MEMORGANIZE_ACTIVATE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MEMORGANIZE_ACTIVATE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_DRIVEBUTTON_INIT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_DRIVEBUTTON_INIT, mp1, mp2 ); }
  afx_msg MRESULT On_WM_DRIVEBUTTON_SELECT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_DRIVEBUTTON_SELECT, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_PROOF( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_PROOF, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_MISSPELLED( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_MISSPELLED, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_AUTOTRANS( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_AUTOTRANS, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_DDE_REQUEST( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_DDE_REQUEST, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_DDE_ANSWER( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_DDE_ANSWER, mp1, mp2 ); }
  afx_msg MRESULT On_LM_EQF_SETITEMSTATE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LM_EQF_SETITEMSTATE, mp1, mp2 ); }
  afx_msg MRESULT On_LM_EQF_QUERYITEMSTATE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LM_EQF_QUERYITEMSTATE, mp1, mp2 ); }
  afx_msg MRESULT On_LM_EQF_SETITEMTEXT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LM_EQF_SETITEMTEXT, mp1, mp2 ); }
  afx_msg MRESULT On_LM_EQF_INSERTITEMSTATE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LM_EQF_INSERTITEMSTATE, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQFN_DRIVEREMOVED( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQFN_DRIVEREMOVED, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQFN_DRIVEADDED( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQFN_DRIVEADDED, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_COLCHANGED( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_COLCHANGED, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_WD_MAIN_NOTIFY( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_WD_MAIN_NOTIFY, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_INITMENU( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_INITMENU, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_BUILDITEMTEXT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_BUILDITEMTEXT, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_CREATELISTWINDOW( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_CREATELISTWINDOW, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_UPDATESLIDER( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_UPDATESLIDER, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_SHOWPOPUP( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_SHOWPOPUP, mp1, mp2 ); }
  afx_msg MRESULT On_EQF_CD_BM_CLICK( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( EQF_CD_BM_CLICK, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_CHECK_REM_PROPS( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_CHECK_REM_PROPS, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_FILTSETDICT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_FILTSETDICT, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_QUERYID( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_QUERYID, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_CHANGEOBJECTNAME( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_CHANGEOBJECTNAME, mp1, mp2 ); }
  afx_msg MRESULT On_WM_EQF_CLOSE( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_CLOSE, mp1, mp2 ); }
  afx_msg MRESULT On_LM_EQF_GETITEMFROMPOINT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LM_EQF_GETITEMFROMPOINT, mp1, mp2 ); }
  afx_msg MRESULT On_LM_EQF_REFRESH( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LM_EQF_REFRESH, mp1, mp2 ); }

  afx_msg MRESULT On_WM_EQF_SHOWHTML( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( WM_EQF_SHOWHTML, mp1, mp2 ); }

//@@afx_msg MRESULT On_WM_EQF_IS_TENV( WPARAM mp1, LPARAM mp2 )
//@@          { return GetActiveView()->SendMessage( WM_EQF_IS_TENV, mp1, mp2 ); }

  /********************************************************************/
  /* the standard listbox message we have to deal with...             */
  /********************************************************************/
  afx_msg MRESULT On_LB_RESETCONTENT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LB_RESETCONTENT, mp1, mp2 ); }
  afx_msg MRESULT On_LB_ADDSTRING( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LB_ADDSTRING, mp1, mp2 ); }
  afx_msg MRESULT On_LB_GETCURSEL( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LB_GETCURSEL, mp1, mp2 ); }
  afx_msg MRESULT On_LB_GETITEMDATA( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LB_GETITEMDATA, mp1, mp2 ); }
  afx_msg MRESULT On_LB_GETTEXT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LB_GETTEXT, mp1, mp2 ); }
  afx_msg MRESULT On_LB_FINDSTRING( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LB_FINDSTRING, mp1, mp2 ); }
  afx_msg MRESULT On_LB_GETCOUNT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LB_GETCOUNT, mp1, mp2 ); }
  afx_msg MRESULT On_LB_GETSELCOUNT( WPARAM mp1, LPARAM mp2 )
          { return GetActiveView()->SendMessage( LB_GETSELCOUNT, mp1, mp2 ); }


  afx_msg VOID OnSize( UINT mp1,int cx, int cy);
  afx_msg void OnSysCommand( UINT nID, LPARAM lParam );
  afx_msg VOID OnMove( int cx, int cy);



  //}}AFX_MSG
  afx_msg void    OnWndActivate( UINT mp1 ) { OnToggleWindow( mp1 );}

  DECLARE_MESSAGE_MAP()
  VOID OnActivateInstance( HWND mpHwnd, BOOL fActivate );


  HWND hObjFocus;
  USHORT usObjClass;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__2EDD418B_CCC9_11D1_8162_00AA0026430D__INCLUDED_)
