/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


// CDlgMtEval dialog

class CDlgMtEval : public CDialog
{
	DECLARE_DYNAMIC(CDlgMtEval)

public:
	CDlgMtEval(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMtEval();

// Dialog Data
	enum { IDD = IDD_DLG_MTEVAL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonMtevalFilespec();
    afx_msg void OnBnClickedButtonMtevalOutfile();
    CButton m_check_outfile;
    CButton m_check_details;
    CEdit m_edit_filespec;
    CEdit m_edit_outfile;
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedCheckMtevalOut();
    afx_msg void OnBnClickedButtonMtevalRun();
    virtual void OnOK();
};
