/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgGetToolInfo.h"
#include "afxdialogex.h"
#include "Commons.h"

// CDlgGetToolInfo dialog

IMPLEMENT_DYNAMIC(CDlgGetToolInfo, CDialogEx)

CDlgGetToolInfo::CDlgGetToolInfo(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgGetToolInfo::IDD, pParent)
{

}

CDlgGetToolInfo::~CDlgGetToolInfo()
{
}

void CDlgGetToolInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_GETTOOLINFO_CMD, m_check_cmd);
	DDX_Control(pDX, IDC_CHECK_GETTOOLINFO_UI, m_check_ui);
}


BEGIN_MESSAGE_MAP(CDlgGetToolInfo, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_GETTOOLINFO_UI, &CDlgGetToolInfo::OnBnClickedCheckGettoolinfoUi)
	ON_BN_CLICKED(IDC_CHECK_GETTOOLINFO_CMD, &CDlgGetToolInfo::OnBnClickedCheckGettoolinfoCmd)
	ON_BN_CLICKED(IDC_GETTOOLINFO_BUTTON_RUN, &CDlgGetToolInfo::OnBnClickedGettoolinfoButtonRun)
END_MESSAGE_MAP()


// CDlgGetToolInfo message handlers


BOOL CDlgGetToolInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	
	m_check_cmd.SetCheck(TRUE);

	return TRUE;  

}


void CDlgGetToolInfo::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}


void CDlgGetToolInfo::OnBnClickedCheckGettoolinfoUi()
{
		m_check_cmd.SetCheck(!m_check_ui.GetCheck());

}


void CDlgGetToolInfo::OnBnClickedCheckGettoolinfoCmd()
{
	m_check_ui.SetCheck(!m_check_cmd.GetCheck());
}


void CDlgGetToolInfo::OnBnClickedGettoolinfoButtonRun()
{
	char szBuf[512+1]={'\0'};
    std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
    {
        MessageBox("Cant' get OpenTM2 installed path");
        return;
    }  
    std::string opentm2path(szBuf);

    std::string cmd(opentm2path);
    cmd += "\\Win\\OtmGetToolInfo.exe ";

	if(m_check_cmd.GetCheck())
		cmd += "  /CMD";
	else if(m_check_ui.GetCheck())
		cmd += "  /UI";

	// execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);

}
