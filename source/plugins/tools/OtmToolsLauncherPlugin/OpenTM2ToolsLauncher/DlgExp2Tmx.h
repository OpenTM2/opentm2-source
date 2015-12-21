/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once

#include <string>

// CDlgExp2Tmx dialog

class CDlgExp2Tmx : public CDialog
{
	DECLARE_DYNAMIC(CDlgExp2Tmx)

public:
	CDlgExp2Tmx(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgExp2Tmx();

// Dialog Data
	enum { IDD = IDD_DLG_EXP2TMX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CButton m_button_outmem_search;
    CButton m_button_inmem_search;
    CButton m_button_run;
    CButton m_check_inmode;
    CComboBox m_combo_outmode;
    CEdit m_edit_inmem;
    CComboBox m_combo_inmode;
//    CListBox m_list_log;
    CEdit m_edit_outmem;
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedButtonInmem();
    afx_msg void OnBnClickedButtonOutmem();
    afx_msg void OnBnClickedExp2tmxButtonRun();
    afx_msg void OnBnClickedExp2tmxButtonHelp();
    afx_msg void OnBnClickedExp2tmxCheckOutmem();
    afx_msg void OnBnClickedExp2tmxCheckInmode();
    afx_msg void OnBnClickedExp2tmxCheckOutmode();
    CButton m_check_outmem;
    CButton m_check_outmode;
    virtual void OnOK();
	CButton m_chec_nocrlf;
};
