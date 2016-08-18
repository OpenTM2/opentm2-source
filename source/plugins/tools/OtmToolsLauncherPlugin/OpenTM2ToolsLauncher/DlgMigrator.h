/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


// CDlgMigrator dialog

class CDlgMigrator : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMigrator)

public:
	CDlgMigrator(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMigrator();

// Dialog Data
	enum { IDD = IDD_DLG_MIGRATOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
//	afx_msg void OnBnClickedCheckMigMem();
//	afx_msg void OnBnClickedCheckMigFolder();
//	afx_msg void OnBnClickedCheckMigSetting();
//	afx_msg void OnBnClickedCheckMigAna();
//	afx_msg void OnBnClickedCheckMigShmem();
	afx_msg void OnBnClickedMigButtonRun();
	CButton m_check_all;
	CButton m_check_ana;
	CButton m_check_calcprofile;
	CButton m_check_dict;
	CButton m_check_folder;
	CButton m_check_mem;
	CButton m_check_settings;
	CButton m_check_shmem;
};
