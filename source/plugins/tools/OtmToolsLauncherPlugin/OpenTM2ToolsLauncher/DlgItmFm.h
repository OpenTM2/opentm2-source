/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


// CDlgItmFm dialog

class CDlgItmFm : public CDialog
{
	DECLARE_DYNAMIC(CDlgItmFm)

public:
	CDlgItmFm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgItmFm();

// Dialog Data
	enum { IDD = IDD_DLG_CARETEITMFROMMEMORY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CEdit m_edit_in;
    CEdit m_edit_out;
    afx_msg void OnBnClickedCitmfmButtonRun();

    virtual void OnOK();
};
