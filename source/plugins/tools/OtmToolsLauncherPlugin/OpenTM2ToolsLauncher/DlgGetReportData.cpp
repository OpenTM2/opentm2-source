/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgGetReportData.h"
#include "afxdialogex.h"
#include "Commons.h"

// CDlgGetReportData dialog

IMPLEMENT_DYNAMIC(CDlgGetReportData, CDialogEx)

CDlgGetReportData::CDlgGetReportData(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgGetReportData::IDD, pParent)
{

}

CDlgGetReportData::~CDlgGetReportData()
{
}

void CDlgGetReportData::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Control(pDX, IDC_BUTTON_GETRPD_INFILE, m_edit_folderpkg);
	//  DDX_Control(pDX, IDC_GETRPD_BUTTON_RUN, m_btn_folderpkg);
	DDX_Control(pDX, IDC_BUTTON_GETRPD_INFILE, m_btn_folderpkg);
	DDX_Control(pDX, IDC_GETRPD_EDIT_IN, m_edit_folderpkg);
}


BEGIN_MESSAGE_MAP(CDlgGetReportData, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_GETRPD_INFILE, &CDlgGetReportData::OnBnClickedButtonGetrpdInfile)
	ON_BN_CLICKED(IDC_GETRPD_BUTTON_RUN, &CDlgGetReportData::OnBnClickedGetrpdButtonRun)
END_MESSAGE_MAP()


// CDlgGetReportData message handlers


void CDlgGetReportData::OnBnClickedButtonGetrpdInfile()
{
	CString res = Commons::searchDialog(_T("FXP  Files (*.FXP)|*.FXP|All Files (*.*)|*.*||"));
    if(!res.IsEmpty())
    {
        m_edit_folderpkg.SetWindowText( res );

    }
}


void CDlgGetReportData::OnBnClickedGetrpdButtonRun()
{
	char szBuf[512+1] = {'\0'};
    
    int infilecnt = m_edit_folderpkg.GetLine(0,szBuf,512);
    if(infilecnt==0)
    {
        MessageBox("please input FolderPakage parameter");
        return;
    }
	szBuf[infilecnt]='\0';
    std::string infile(szBuf);

	std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
    {
        MessageBox("Cant' get OpenTM2 installed path");
        return;
    }  
    std::string opentm2path(szBuf);

    std::string cmd(opentm2path);
    cmd += "\\Win\\OtmGetReportData.exe ";

    cmd += "  "+infile;
    

    // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}


void CDlgGetReportData::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}
