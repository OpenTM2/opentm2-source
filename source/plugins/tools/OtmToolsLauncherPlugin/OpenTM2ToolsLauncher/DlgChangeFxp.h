/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


// CDlgChangeFxp dialog

class CDlgChangeFxp : public CDialog
{
	DECLARE_DYNAMIC(CDlgChangeFxp)

public:
	CDlgChangeFxp(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgChangeFxp();

// Dialog Data
	enum { IDD = IDD_DLG_CHANGEFXP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CButton m_btn_folderfile;
    CButton m_check_pw;
    CButton m_check_ship;
    CButton m_check_type;
    CEdit m_edit_folderfile;
    CEdit m_edit_pw;
    CEdit m_edit_ship;
    CComboBox m_combo_type;
    afx_msg void OnBnClickedButtonFolderfile();
    afx_msg void OnBnClickedChangefxpCheckType();
    afx_msg void OnBnClickedChangefxpCheckPw();
    afx_msg void OnBnClickedChangefxpCheckShip();
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedChangefxpButtonRun();
    virtual void OnOK();
};
