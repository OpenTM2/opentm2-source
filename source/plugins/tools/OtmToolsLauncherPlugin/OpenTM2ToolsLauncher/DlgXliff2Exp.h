/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


// CDlgXliff2Exp dialog

class CDlgXliff2Exp : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgXliff2Exp)

public:
	CDlgXliff2Exp(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgXliff2Exp();

// Dialog Data
	enum { IDD = IDD_DLG_XLIFF2EXP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_edit_in;
	CEdit m_edit_outfile;
	CButton m_check_outfile;
	afx_msg void OnBnClickedButtonXliff2expInfile();
	afx_msg void OnBnClickedButtonXliff2expOutfile();
	afx_msg void OnBnClickedXliff2expButtonRun();
	afx_msg void OnBnClickedCheckXliff2expOutfile();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
