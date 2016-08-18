/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


// CDlgGetReportData dialog

class CDlgGetReportData : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgGetReportData)

public:
	CDlgGetReportData(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgGetReportData();

// Dialog Data
	enum { IDD = IDD_DLG_GETREPORTDATA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	CButton m_edit_folderpkg;
//	CButton m_btn_folderpkg;
	afx_msg void OnBnClickedButtonGetrpdInfile();
	afx_msg void OnBnClickedGetrpdButtonRun();
	CButton m_btn_folderpkg;
	CEdit m_edit_folderpkg;
	virtual void OnOK();
};
