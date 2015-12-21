/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgMigrator.h"
#include "afxdialogex.h"
#include "Commons.h"

// CDlgMigrator dialog

IMPLEMENT_DYNAMIC(CDlgMigrator, CDialogEx)

CDlgMigrator::CDlgMigrator(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgMigrator::IDD, pParent)
{

}

CDlgMigrator::~CDlgMigrator()
{
}

void CDlgMigrator::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_MIG_ALL, m_check_all);
	DDX_Control(pDX, IDC_CHECK_MIG_ANA, m_check_ana);
	DDX_Control(pDX, IDC_CHECK_MIG_CALCPROFILE, m_check_calcprofile);
	DDX_Control(pDX, IDC_CHECK_MIG_DICT, m_check_dict);
	DDX_Control(pDX, IDC_CHECK_MIG_FOLDER, m_check_folder);
	DDX_Control(pDX, IDC_CHECK_MIG_MEM, m_check_mem);
	DDX_Control(pDX, IDC_CHECK_MIG_SETTING, m_check_settings);
	DDX_Control(pDX, IDC_CHECK_MIG_SHMEM, m_check_shmem);
}


BEGIN_MESSAGE_MAP(CDlgMigrator, CDialogEx)
//	ON_BN_CLICKED(IDC_CHECK_MIG_MEM, &CDlgMigrator::OnBnClickedCheckMigMem)
//	ON_BN_CLICKED(IDC_CHECK_MIG_FOLDER, &CDlgMigrator::OnBnClickedCheckMigFolder)
//	ON_BN_CLICKED(IDC_CHECK_MIG_SETTING, &CDlgMigrator::OnBnClickedCheckMigSetting)
//	ON_BN_CLICKED(IDC_CHECK_MIG_ANA, &CDlgMigrator::OnBnClickedCheckMigAna)
//	ON_BN_CLICKED(IDC_CHECK_MIG_SHMEM, &CDlgMigrator::OnBnClickedCheckMigShmem)
	ON_BN_CLICKED(IDC_MIG_BUTTON_RUN, &CDlgMigrator::OnBnClickedMigButtonRun)
END_MESSAGE_MAP()


// CDlgMigrator message handlers


void CDlgMigrator::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}




void CDlgMigrator::OnBnClickedMigButtonRun()
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
    cmd += "\\Win\\TM2OTMMigrator.exe ";

	std::string cmdold(cmd);

	if(m_check_mem.GetCheck())
		cmd += "  /Mem";

	if(m_check_shmem.GetCheck())
		cmd += "  /SharedMem";

	if(m_check_folder.GetCheck())
		cmd += "  /Folder";
    
    
	if(m_check_dict.GetCheck())
		cmd += "  /Dict";

	if(m_check_settings.GetCheck())
		cmd += "  /Settings";

	if(m_check_calcprofile.GetCheck())
		cmd += "  /CalcProfile";

	if(m_check_ana.GetCheck())
		cmd += "  /AnalysisProfile";

	if(m_check_all.GetCheck())
		cmd += "  /All";

	if(cmdold == cmd)
	{
		MessageBox("Please select at least one parameter");
		return;
	}
    // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}
