/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgTmx2Text.h"
#include "afxdialogex.h"
#include "Commons.h"

// CDlgTmx2Text dialog

IMPLEMENT_DYNAMIC(CDlgTmx2Text, CDialogEx)

CDlgTmx2Text::CDlgTmx2Text(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgTmx2Text::IDD, pParent)
{

}

CDlgTmx2Text::~CDlgTmx2Text()
{
}

void CDlgTmx2Text::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TMX2TEXT_EDIT_IN, m_edit_in);
	DDX_Control(pDX, IDC_TMX2TEXT_EDIT_OUT, m_edit_out);
	DDX_Control(pDX, IDC_CHECK_TMX2TEXT_OUTFILE, m_check_outfile);
}


BEGIN_MESSAGE_MAP(CDlgTmx2Text, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_TMX2TEXT_OUTFILE, &CDlgTmx2Text::OnBnClickedButtonTmx2textOutfile)
	ON_BN_CLICKED(IDC_BUTTON_TMX2TEXT_INFILE, &CDlgTmx2Text::OnBnClickedButtonTmx2textInfile)
	ON_BN_CLICKED(IDC_TMX2TEXT_BUTTON_RUN, &CDlgTmx2Text::OnBnClickedTmx2textButtonRun)
	ON_BN_CLICKED(IDC_CHECK_TMX2TEXT_OUTFILE, &CDlgTmx2Text::OnBnClickedCheckTmx2textOutfile)
END_MESSAGE_MAP()


// CDlgTmx2Text message handlers


BOOL CDlgTmx2Text::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_edit_out.EnableWindow(m_check_outfile.GetCheck());
	return TRUE;  
	
}


void CDlgTmx2Text::OnBnClickedButtonTmx2textOutfile()
{
    CString res = Commons::searchDialog("");
    if(!res.IsEmpty())
    {
        m_edit_out.SetWindowText( res );

    }
}


void CDlgTmx2Text::OnBnClickedButtonTmx2textInfile()
{
	 CString res = Commons::searchDialog(_T("TMX MEM Files (*.TMX)|*.TMX|All Files (*.*)|*.*||"));
    if(!res.IsEmpty())
    {
        m_edit_in.SetWindowText( res );

    }
}


void CDlgTmx2Text::OnBnClickedTmx2textButtonRun()
{
	 char szBuf[512+1] = {'\0'};
    
    int infilecnt = m_edit_in.GetLine(0,szBuf,512);
    if(infilecnt==0)
    {
        MessageBox("please input infile parameter");
        return;
    }
	szBuf[infilecnt]='\0';
    std::string infile(szBuf);

    int outfilecnt = m_edit_out.GetLine(0,szBuf,512);
    if(m_check_outfile.GetCheck() && outfilecnt==0)
    {
        MessageBox("please input outfile parameter");
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
    cmd += "\\Win\\OtmTmxSource2Text.exe ";

    cmd += "  "+infile;
    if(m_check_outfile.GetCheck() && outfilecnt!=0)
        cmd += "  "+outfile;

    // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}


void CDlgTmx2Text::OnBnClickedCheckTmx2textOutfile()
{
	m_edit_out.EnableWindow(m_check_outfile.GetCheck());
}


void CDlgTmx2Text::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}
