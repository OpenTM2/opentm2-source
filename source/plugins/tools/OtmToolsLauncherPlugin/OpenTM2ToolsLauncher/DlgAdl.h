/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


// CDlgAdl dialog

class CDlgAdl : public CDialog
{
	DECLARE_DYNAMIC(CDlgAdl)

public:
	CDlgAdl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgAdl();

// Dialog Data
	enum { IDD = IDD_DLG_ADL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonAdlRun();
    CButton m_check_dict;
    CButton m_check_fol;
    CButton m_check_mem;
    CEdit m_edit_dict;
    CEdit m_edit_fol;
    //CButton m_button_dict_search;
    //CButton m_button_mem_search;
    //CButton m_button_fol_search;
    CButton m_button_run;
    CListBox m_list_log;
//    afx_msg void OnBnClickedButtonAdlMem();
//    afx_msg void OnBnClickedButtonAdlFol();
//    afx_msg void OnBnClickedButtonAdlDict();
    virtual BOOL OnInitDialog();
    afx_msg void OnClickedCheckAdlFol();
    afx_msg void OnClickedCheckAdlMem();
    afx_msg void OnClickedAdlCheckDic();
    CEdit m_edit_mem;
    virtual void OnOK();
};
