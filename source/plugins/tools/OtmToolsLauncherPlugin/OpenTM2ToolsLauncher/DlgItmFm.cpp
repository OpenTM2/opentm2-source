/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
// DlgItmFm.cpp : implementation file
//

#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgItmFm.h"
#include "afxdialogex.h"
#include "Commons.h"

// CDlgItmFm dialog

IMPLEMENT_DYNAMIC(CDlgItmFm, CDialog)

CDlgItmFm::CDlgItmFm(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgItmFm::IDD, pParent)
{

}

CDlgItmFm::~CDlgItmFm()
{
}

void CDlgItmFm::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CITMFM_EDIT_IN, m_edit_in);
    DDX_Control(pDX, IDC_CITMFM_EDIT_OUT, m_edit_out);
}


BEGIN_MESSAGE_MAP(CDlgItmFm, CDialog)
    ON_BN_CLICKED(IDC_CITMFM_BUTTON_RUN, &CDlgItmFm::OnBnClickedCitmfmButtonRun)
END_MESSAGE_MAP()



void CDlgItmFm::OnBnClickedCitmfmButtonRun()
{
    char szBuf[512+1] = {'\0'};
    
    int incnt = m_edit_in.GetLine(0,szBuf,512);
    if(incnt==0)
    {
        MessageBox("please input /IN parameter");
        return;
    }
	szBuf[incnt]='\0';
    std::string in(szBuf);

    int outcnt = m_edit_out.GetLine(0,szBuf,512);
    if(outcnt==0)
    {
        MessageBox("please input /OUT parameter");
        return;
    }
	szBuf[outcnt]='\0';
    std::string out(szBuf);

    std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
    {
        MessageBox("Cant' get OpenTM2 installed path");
        return;
    }  
    std::string opentm2path(szBuf);

    std::string cmd(opentm2path);
    cmd += "\\Win\\OtmCreateITMFromMemory.exe ";
    cmd +=" /IN="+in; 
    cmd +=" /OUT="+out; 

    // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}




void CDlgItmFm::OnOK()
{
    // TODO: Add your specialized code here and/or call the base class

    //CDialog::OnOK();
}
