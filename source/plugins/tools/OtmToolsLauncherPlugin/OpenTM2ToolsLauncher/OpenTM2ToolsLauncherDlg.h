/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
// OpenTM2ToolsLauncherDlg.h : header file
//

#pragma once
#include "TabCtrlOwn.h"
#include "DlgToolLauncher.h"

// COpenTM2ToolsLauncherDlg dialog
class COpenTM2ToolsLauncherDlg : public CDialogEx
{
// Construction
public:
	COpenTM2ToolsLauncherDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_OPENTM2TOOLSLAUNCHER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    CTabCtrlOwn  m_Tab;
    CDlgToolLauncher mToolLauncher;
    afx_msg void OnClose();
    afx_msg void OnBnClickedHelp();
    afx_msg void OnBnClickedExit();
private:
    void saveHistory();
};
