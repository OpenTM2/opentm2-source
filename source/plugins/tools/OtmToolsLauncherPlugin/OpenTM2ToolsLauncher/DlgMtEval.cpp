/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/


#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgMtEval.h"
#include "afxdialogex.h"
#include "Commons.h"

// CDlgMtEval dialog

IMPLEMENT_DYNAMIC(CDlgMtEval, CDialog)

CDlgMtEval::CDlgMtEval(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMtEval::IDD, pParent)
{

}

CDlgMtEval::~CDlgMtEval()
{
}

void CDlgMtEval::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHECK_MTEVAL_OUT, m_check_outfile);
    DDX_Control(pDX, IDC_CHECK_MTEVAL_DETAILS, m_check_details);
    DDX_Control(pDX, IDC_MTEVAL_EDIT_FSP, m_edit_filespec);
    DDX_Control(pDX, IDC_MTEVAL_EDIT_OUTFILE, m_edit_outfile);
}


BEGIN_MESSAGE_MAP(CDlgMtEval, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_MTEVAL_FILESPEC, &CDlgMtEval::OnBnClickedButtonMtevalFilespec)
    ON_BN_CLICKED(IDC_BUTTON_MTEVAL_OUTFILE, &CDlgMtEval::OnBnClickedButtonMtevalOutfile)
    ON_BN_CLICKED(IDC_CHECK_MTEVAL_OUT, &CDlgMtEval::OnBnClickedCheckMtevalOut)
    ON_BN_CLICKED(IDC_BUTTON_MTEVAL_RUN, &CDlgMtEval::OnBnClickedButtonMtevalRun)
END_MESSAGE_MAP()


// CDlgMtEval message handlers


void CDlgMtEval::OnBnClickedButtonMtevalFilespec()
{
    CString res = Commons::searchFolder();
    if(!res.IsEmpty())
        m_edit_filespec.SetWindowText( res );
}


void CDlgMtEval::OnBnClickedButtonMtevalOutfile()
{
    CString res = Commons::searchDialog("");
    if(!res.IsEmpty())
        m_edit_outfile.SetWindowText( res );
}


BOOL CDlgMtEval::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_edit_outfile.EnableWindow(m_check_outfile.GetCheck());

    return TRUE; 
}


void CDlgMtEval::OnBnClickedCheckMtevalOut()
{
    m_edit_outfile.EnableWindow(m_check_outfile.GetCheck());
}


void CDlgMtEval::OnBnClickedButtonMtevalRun()
{
     char szBuf[512+1] = {'\0'};
    
    int fldcnt = m_edit_filespec.GetLine(0,szBuf,512);
    if(fldcnt==0)
    {
        MessageBox("please input filespec parameter");
        return;
    }
	szBuf[fldcnt]='\0';
    std::string filespec(szBuf);

    int outfilecnt = m_edit_outfile.GetLine(0,szBuf,512);
    if(m_check_outfile.GetCheck() && outfilecnt==0)
    {
        MessageBox("please input outfile.xml parameter");
        return;
    }
	szBuf[outfilecnt]='\0';
    std::string outfile(szBuf);

    std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
    {
        MessageBox("Cant' get OpenTM2 installed path");
        return;
    }  
    std::string opentm2path(szBuf);

    std::string cmd(opentm2path);
    cmd += "\\Win\\OtmMtEval.exe ";

    cmd += " "+filespec;
    if(m_check_outfile.GetCheck() && outfilecnt!=0)
        cmd += " "+outfile;

    if(m_check_details.GetCheck())
        cmd+= " /details";

    // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}


void CDlgMtEval::OnOK()
{
    // TODO: Add your specialized code here and/or call the base class

    //CDialog::OnOK();
}
