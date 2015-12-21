/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


// CDlgTmxSplitSeg dialog

class CDlgTmxSplitSeg : public CDialog
{
	DECLARE_DYNAMIC(CDlgTmxSplitSeg)

public:
	CDlgTmxSplitSeg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgTmxSplitSeg();

// Dialog Data
	enum { IDD = IDD_DLG_TMXSPLITSEGMENTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedButtonTmsplitsegInmem();
    afx_msg void OnBnClickedButtonTmssplitsegOutmem();
    afx_msg void OnBnClickedTmxsplitsegButtonRun();
    afx_msg void OnBnClickedTmxsplitsegCheckOutputmem();
    afx_msg void OnBnClickedTmxsplitsegCheckLogs();
    afx_msg void OnBnClickedTmxsplitsegCheckLog();
//    CButton m_check_inmem;
    CButton m_button_outmem;
    CButton m_button_inmem;
    CComboBox m_combo_logs;
    CEdit m_edit_log;
    CEdit m_edit_outmem;
    CButton m_check_log;
    CButton m_check_logs;
    CButton m_check_outputmem;
    CEdit m_edit_inmem;
    virtual void OnOK();
//    virtual void OnCancel();
};
