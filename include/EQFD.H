//+----------------------------------------------------------------------------+
//|EQFD.H   main header file for the EQFD application                          |
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
// $Revision: 1.2 $ ----------- 22 Nov 2006
// GQ: - increased size of CEQFDApp::chTitle to 300 for the display of long names
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
// $Revision: 1.3 $ ----------- 29 Oct 2004
// --RJ: P020773: support OnEBus2
// 
//
// $Revision: 1.2 $ ----------- 8 Oct 2004
// GQ: - changes for compile for TQM
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
// $Revision: 1.2 $ ----------- 16 Jan 2004
// GQ: - Added ShowWndNavigate method
//
//
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
//
//
// $Revision: 1.2 $ ----------- 20 Oct 2003
// -- RJ: P018362: rename m_hTQMSem to m_hTM_TQMSem for checking if TM is still running
//
//
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
//
//
// $Revision: 1.2 $ ----------- 18 Jul 2003
// GQ: - Added ExceptionHandler method
//
//
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
//
//
// $Revision: 1.3 $ ----------- 26 Feb 2003
// -- RJ: .NET adaptation
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
// $Revision: 1.4 $ ----------- 11 Aug 2000
// - TQM: added handle for semaphore file
//
//
// $Revision: 1.3 $ ----------- 4 May 2000
// - added handling for TQM menu items
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFD.HV_   1.6   05 Jul 1999 10:14:58   BUILD  $
 *
 * $Log:   K:\DATA\EQFD.HV_  $
 *
 *    Rev 1.6   05 Jul 1999 10:14:58   BUILD
 * -- add short-key handling in TM/Wi
 *
 *    Rev 1.5   29 Mar 1999 15:37:36   BUILD
 * added _EBUS
 *
 *    Rev 1.4   20 Jan 1999 18:38:16   BUILD
 * included EQFRCMFC.H
 *
 *    Rev 1.3   18 Jan 1999 15:14:46   BUILD
 * -- add SetFontSize member function
 *
 *    Rev 1.2   07 Dec 1998 10:43:36   BUILD
 * -- cleanup startup code for MFC
 *
 *    Rev 1.1   09 Nov 1998 09:35:36   BUILD
 * -- add support for HelpIndex
 *
 *    Rev 1.0   26 Oct 1998 19:11:12   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#if !defined(AFX_EQFD_H__2EDD4185_CCC9_11D1_8162_00AA0026430D__INCLUDED_)
#define AFX_EQFD_H__2EDD4185_CCC9_11D1_8162_00AA0026430D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
        #error include 'stdafx.h' before including this file for PCH
#endif


#include "eqfrcmfc.h"


BOOL CALLBACK ChildEnumProc ( HWND hwnd, LPARAM lIndex );
/////////////////////////////////////////////////////////////////////////////
// CEQFDApp:
// See EQFD.cpp for the implementation of this class
//

class CEQFDApp : public CWinApp
{
private:
	static LONG WINAPI TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo );
public:
        CEQFDApp();
        HWND    CreateList    ( MDICREATESTRUCT *pMDICreate, SHORT sTempl);
        HWND    CreateProcess ( MDICREATESTRUCT *pMDICreate);

        PVOID   CreateShowWnd ( HWND hwnd, PSZ pTextFile );
        VOID    DestroyShowWnd( PVOID pCWeb );
        VOID    ShowWndNavigate( PVOID pCWeb, PSZ pszFile );
        VOID    SizeShowWnd   ( HWND hwndParent, PVOID pCWeb );
        VOID    RefreshShowWnd( PVOID pCWeb );
        VOID    SetFontSize   ( PVOID pCWeb, LONG lSel );
#ifdef _TQM
        VOID    PrintHTML( HWND hwndParent, PSZ pszHtmlFile );
#endif
        VOID    SetObjTitle( PSZ pObj ) { strcpy( chTitle, pObj ); };
        PSZ     GetObjTitle( )          { return chTitle; };
        CHAR chTitle[300];
        MDICREATESTRUCT* GetMDIStruct() { return pMDI; };
        MDICREATESTRUCT *pMDI;

        BOOL fReportHandler;

        HANDLE m_hTM_TQMSem;


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CEQFDApp)
        public:
        virtual BOOL InitInstance();
        virtual int  ExitInstance();
        virtual BOOL AddAvailTemplates();
        virtual BOOL PreTranslateMessage( MSG * pMsg );
        //}}AFX_VIRTUAL

// Implementation

        //{{AFX_MSG(CEQFDApp)
        afx_msg void OnAppAbout();
        afx_msg void OnHelpIndex();
        afx_msg void OnEBus();
        afx_msg void OnEBus1();
        afx_msg void OnEBus2();
        afx_msg void OnHelpForHelp();
        afx_msg void OnHelp();
        afx_msg void OnContextHelp();
        afx_msg void OnHelpUsing();

        afx_msg void OnTQMLogon();
        afx_msg void OnTQMLogoff();
        afx_msg void OnTQMUser();
        afx_msg void OnTQMEvaluation();
        afx_msg void OnTQMVendor();
        afx_msg void OnTQMReports();
        afx_msg void OnTQMSettings();
        afx_msg void OnTQMProjects();
        afx_msg void OnTQMArchive();

                // NOTE - the ClassWizard will add and remove member functions here.
                //    DO NOT EDIT what you see in these blocks of generated code !
        afx_msg HWND OnProcessTask(WPARAM mp1, LPARAM mp2);
        afx_msg VOID A3( WPARAM mp1, LPARAM mp2 );
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EQFD_H__2EDD4185_CCC9_11D1_8162_00AA0026430D__INCLUDED_)
