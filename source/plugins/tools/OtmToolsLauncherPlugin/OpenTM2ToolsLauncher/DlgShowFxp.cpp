/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgShowFxp.h"
#include "afxdialogex.h"
#include "Commons.h"

// CDlgShowFxp dialog

IMPLEMENT_DYNAMIC(CDlgShowFxp, CDialogEx)

CDlgShowFxp::CDlgShowFxp(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgShowFxp::IDD, pParent)
{

}

CDlgShowFxp::~CDlgShowFxp()
{
}

void CDlgShowFxp::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SHOWFXP_EDIT_FSP, m_edit_folder);
	DDX_Control(pDX, IDC_BUTTON_SHOWFXP_FILESPEC, m_btn_search);
	DDX_Control(pDX, IDC_CHECK_SHOWFXP_DETAILS, m_check_detail);
}


BEGIN_MESSAGE_MAP(CDlgShowFxp, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SHOWFXP_RUN, &CDlgShowFxp::OnBnClickedButtonShowfxpRun)
	ON_BN_CLICKED(IDC_BUTTON_SHOWFXP_FILESPEC, &CDlgShowFxp::OnBnClickedButtonShowfxpFilespec)
END_MESSAGE_MAP()


// CDlgShowFxp message handlers


void CDlgShowFxp::OnBnClickedButtonShowfxpRun()
{
	char szBuf[512+1] = {'\0'};
    
    int fldcnt = m_edit_folder.GetLine(0,szBuf,512);
    if(fldcnt==0)
    {
        MessageBox("please input folderfile parameter");
        return;
    }
	szBuf[fldcnt]='\0';
    std::string folder(szBuf);
	
	std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
    {
        MessageBox("Cant' get OpenTM2 installed path");
        return;
    }  
    std::string opentm2path(szBuf);

    std::string cmd(opentm2path);
    cmd += "\\Win\\OtmShowFxp.exe  ";

	cmd += folder;

    if(m_check_detail.GetCheck())
        cmd+= "  /DETAILS";

    // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}


void CDlgShowFxp::OnBnClickedButtonShowfxpFilespec()
{
	CString res = Commons::searchDialog("");
    if(!res.IsEmpty())
        m_edit_folder.SetWindowText( res );
}


void CDlgShowFxp::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}
