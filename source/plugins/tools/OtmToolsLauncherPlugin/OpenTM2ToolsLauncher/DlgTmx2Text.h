/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


// CDlgTmx2Text dialog

class CDlgTmx2Text : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTmx2Text)

public:
	CDlgTmx2Text(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgTmx2Text();

// Dialog Data
	enum { IDD = IDD_DLG_TMX2TEXT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_edit_in;
	CEdit m_edit_out;
	CButton m_check_outfile;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonTmx2textOutfile();
	afx_msg void OnBnClickedButtonTmx2textInfile();
	afx_msg void OnBnClickedTmx2textButtonRun();
	afx_msg void OnBnClickedCheckTmx2textOutfile();
	virtual void OnOK();
};
