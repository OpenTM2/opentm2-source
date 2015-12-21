/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once
#include "Commons.h"

// CDlgMemoryTool dialog

class CDlgMemoryTool : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMemoryTool)

public:
	CDlgMemoryTool(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMemoryTool();

// Dialog Data
	enum { IDD = IDD_DLG_MEMTOOL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedMemtoolButtonRun();
	afx_msg void OnSelchangeMemtoolcomboTask();

private:
	void setStatusWithTask(const std::string& task);
	void disableAll();
public:
	std::vector<std::string> m_SrcLangs;
	std::vector<std::string> m_TgtLangs;
	std::vector<std::string> m_Langs;
	std::vector<std::string> m_Markups;

	CComboBox m_combo_task;
	//CEdit m_edit_tomarkup;
	//CEdit m_edit_tolang;
	CEdit m_edit_rev;
	CEdit m_edit_out;
	CEdit m_edit_mem;
	//CEdit m_edit_frommarkup;
	//CEdit m_edit_fromlang;
	CEdit m_edit_doc;
	CEdit m_edit_date;
	CButton m_check_type;
	CButton m_check_tomarkup;
	CButton m_check_tolang;
	CButton m_check_set;
	CButton m_check_frommarkup;
	CButton m_check_fromlang;
	CButton m_check_doc;
	CButton m_check_date;
	CButton m_check_clear;
	afx_msg void OnBnClickedMemtoolCheckFrommarkup();
	afx_msg void OnBnClickedMemtoolCheckTomakrup();
	afx_msg void OnBnClickedMemtoolCheckFromlang();
	afx_msg void OnBnClickedMemtoolCheckTolang();
	afx_msg void OnBnClickedMemtoolCheckDate();
	afx_msg void OnBnClickedMemtoolCheckDoc();
	afx_msg void OnBnClickedMemtoolCheckClear();
	afx_msg void OnBnClickedMemtoolCheckSet();
	afx_msg void OnCbnSelchangeMemtoolComboTomarkup();
	CComboBox m_combo_fromlang;
	CComboBox m_combo_frommarkup;
	CComboBox m_combo_tolang;
//	CComboBox m_combo_tomarpup;
	CComboBox m_combo_tomarkup;
};
