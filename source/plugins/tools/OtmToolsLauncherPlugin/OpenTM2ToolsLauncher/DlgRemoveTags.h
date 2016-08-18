/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once
#include <vector>

// CDlgRemoveTags dialog

class CDlgRemoveTags : public CDialog
{
	DECLARE_DYNAMIC(CDlgRemoveTags)

public:
	CDlgRemoveTags(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgRemoveTags();

// Dialog Data
	enum { IDD = IDD_DLG_REMOVETAGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	CEdit m_inmem;
//	CEdit m_markup;
	CEdit m_edit_inmem;
	//CEdit m_edit_markup;
	CEdit m_edit_outmem;
	CComboBox m_combo_inmode;
	CComboBox m_combo_outmode;
	CButton m_btn_inmem_search;
	CButton m_btn_outmem_search;
	virtual BOOL OnInitDialog();
	CButton m_check_markup;
	afx_msg void OnClickedRemoetagsCheckMarkup();
	afx_msg void OnClickedButtonInmem();
	afx_msg void OnBnClickedButtonOutmem();
	afx_msg void OnBnClickedRemovetagsButtonRun();
	virtual void OnOK();
	CComboBox m_combo_markup;
	std::vector<std::string> m_Markups;
};
