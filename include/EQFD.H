//+----------------------------------------------------------------------------+
//|EQFD.H   main header file for the EQFD application                          |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
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
        afx_msg void OnTechGuide();
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
