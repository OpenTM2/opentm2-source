/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgXliff2Exp.h"
#include "afxdialogex.h"
#include "Commons.h"

// CDlgXliff2Exp dialog

IMPLEMENT_DYNAMIC(CDlgXliff2Exp, CDialogEx)

CDlgXliff2Exp::CDlgXliff2Exp(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgXliff2Exp::IDD, pParent)
{

}

CDlgXliff2Exp::~CDlgXliff2Exp()
{
}

void CDlgXliff2Exp::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_XLIFF2EXP_EDIT_IN, m_edit_in);
	DDX_Control(pDX, IDC_XLIFF2EXP_EDIT_OUT, m_edit_outfile);
	DDX_Control(pDX, IDC_CHECK_XLIFF2EXP_OUTFILE, m_check_outfile);
}


BEGIN_MESSAGE_MAP(CDlgXliff2Exp, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_XLIFF2EXP_INFILE, &CDlgXliff2Exp::OnBnClickedButtonXliff2expInfile)
	ON_BN_CLICKED(IDC_BUTTON_XLIFF2EXP_OUTFILE, &CDlgXliff2Exp::OnBnClickedButtonXliff2expOutfile)
	ON_BN_CLICKED(IDC_XLIFF2EXP_BUTTON_RUN, &CDlgXliff2Exp::OnBnClickedXliff2expButtonRun)
	ON_BN_CLICKED(IDC_CHECK_XLIFF2EXP_OUTFILE, &CDlgXliff2Exp::OnBnClickedCheckXliff2expOutfile)
END_MESSAGE_MAP()


// CDlgXliff2Exp message handlers


void CDlgXliff2Exp::OnBnClickedButtonXliff2expInfile()
{
	CString res = Commons::searchDialog("");
    if(!res.IsEmpty())
    {
        m_edit_in.SetWindowText( res );

    }
}


void CDlgXliff2Exp::OnBnClickedButtonXliff2expOutfile()
{
	CString res = Commons::searchDialog(_T("EXP MEM Files (*.EXP)|*.EXP|All Files (*.*)|*.*||"));
    if(!res.IsEmpty())
    {
        m_edit_outfile.SetWindowText( res );

    }
}


void CDlgXliff2Exp::OnBnClickedXliff2expButtonRun()
{
	 char szBuf[512+1] = {'\0'};
    
    int infilecnt = m_edit_in.GetLine(0,szBuf,512);
    if(infilecnt==0)
    {
        MessageBox("please input /INPUTMEM parameter");
        return;
    }
	szBuf[infilecnt]='\0';
    std::string infile(szBuf);

    int outfilecnt = m_edit_outfile.GetLine(0,szBuf,512);
    if(m_check_outfile.GetCheck() && outfilecnt==0)
    {
        MessageBox("please input /OUTPUTMEM parameter");
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
    cmd += "\\Win\\OtmXliff2Exp.exe ";

    cmd += "  "+infile;
    if(m_check_outfile.GetCheck() && outfilecnt!=0)
        cmd += "  "+outfile;

    // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}


void CDlgXliff2Exp::OnBnClickedCheckXliff2expOutfile()
{
	m_edit_outfile.EnableWindow(m_check_outfile.GetCheck());
}


BOOL CDlgXliff2Exp::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_edit_outfile.EnableWindow(m_check_outfile.GetCheck());

	return TRUE;

}


void CDlgXliff2Exp::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}
