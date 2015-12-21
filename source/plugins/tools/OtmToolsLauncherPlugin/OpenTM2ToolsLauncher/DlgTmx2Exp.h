/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


// CDlgTmx2Exp dialog

class CDlgTmx2Exp : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTmx2Exp)

public:
	CDlgTmx2Exp(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgTmx2Exp();

// Dialog Data
	enum { IDD = IDD_DLG_TMX2EXP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
//	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
//	afx_msg void OnSize(UINT nType, int cx, int cy);

//	afx_msg void OnSize(UINT nType, int cx, int cy);
//	CEdit m_edir_inmem;
	CEdit m_edit_outmem;
//	CComboBox m_combo_inmode;
	CEdit m_edit_markup;
	afx_msg void OnBnClickedTmx2expCheckSs();
	afx_msg void OnBnClickedTmx2expCheckIncludebrace();
	afx_msg void OnBnClickedTmx2expCheckCleantf();
	afx_msg void OnBnClickedtmx2expCheckMarkup();
	afx_msg void OnBnClickedTmx2expCheckOutmode();
	afx_msg void OnBnClickedTmx2expCheckOutmem();
	afx_msg void OnBnClickedTmx2expButtonRun();
	afx_msg void OnBnClickedTmx2expButtonInmem();
	CButton m_check_clearntf;
	CButton m_check_includebrace;
	CButton m_check_markup;
	CButton m_check_outmem;
	CButton m_check_outmode;
	CButton m_check_ss;
	CComboBox m_combo_outmode;
	CEdit m_edit_inmem;
	virtual void OnOK();
};
