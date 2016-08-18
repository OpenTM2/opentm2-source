/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


// CDlgShowFxp dialog

class CDlgShowFxp : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgShowFxp)

public:
	CDlgShowFxp(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgShowFxp();

// Dialog Data
	enum { IDD = IDD_DLG_SHOWFXP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_edit_folder;
	CButton m_btn_search;
	CButton m_check_detail;
	afx_msg void OnBnClickedButtonShowfxpRun();
	afx_msg void OnBnClickedButtonShowfxpFilespec();
	virtual void OnOK();
};
