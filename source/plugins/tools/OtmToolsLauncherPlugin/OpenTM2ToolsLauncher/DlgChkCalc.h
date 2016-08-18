/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


// CDlgChkCalc dialog

class CDlgChkCalc : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgChkCalc)

public:
	CDlgChkCalc(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgChkCalc();

// Dialog Data
	enum { IDD = IDD_DLG_CHKCALC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CButton m_button_fld;
    CButton m_button_fxp;
    CButton m_button_hlog;
//    CButton m_chk_run;
    CButton m_check_all;
    CButton m_check_fld;
    CButton m_check_fxp;
    CButton m_check_hlog;
    CEdit m_edit_fld;
    CEdit m_edit_fxp;
    CEdit m_edit_hlog;
    afx_msg void OnBnClickedButtonChkFld();
    afx_msg void OnBnClickedButtonChkFxp();
    afx_msg void OnBnClickedButtonChkHlog();
    afx_msg void OnBnClickedButtonChkcalcRun();
    afx_msg void OnBnClickedCheckChkFld();
    afx_msg void OnBnClickedCheckChkFxp();
    afx_msg void OnBnClickedCheckChkHlog();
    afx_msg void OnBnClickedCheckChkAll();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
};
