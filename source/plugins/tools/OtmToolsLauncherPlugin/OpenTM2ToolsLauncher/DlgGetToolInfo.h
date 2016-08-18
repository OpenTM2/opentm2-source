#pragma once


// CDlgGetToolInfo dialog

class CDlgGetToolInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgGetToolInfo)

public:
	CDlgGetToolInfo(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgGetToolInfo();

// Dialog Data
	enum { IDD = IDD_DLG_GETTOOLINFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	CButton m_check_cmd;
	CButton m_check_ui;
	afx_msg void OnBnClickedCheckGettoolinfoUi();
	afx_msg void OnBnClickedCheckGettoolinfoCmd();
	afx_msg void OnBnClickedGettoolinfoButtonRun();
};
