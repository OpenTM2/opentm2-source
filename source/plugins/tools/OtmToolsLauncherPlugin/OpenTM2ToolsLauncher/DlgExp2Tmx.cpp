/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgExp2Tmx.h"
#include "afxdialogex.h"
#include "Commons.h"
#include <fstream>

static std::string INMODE[3] = {"UTF16","ASCII","ANSI"};
static std::string OUTMODE[2] = {"UTF8","UTF16"};
// CDlgExp2Tmx dialog

IMPLEMENT_DYNAMIC(CDlgExp2Tmx, CDialog)

CDlgExp2Tmx::CDlgExp2Tmx(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgExp2Tmx::IDD, pParent)
{

}

CDlgExp2Tmx::~CDlgExp2Tmx()
{
}

void CDlgExp2Tmx::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_OUTMEM, m_button_outmem_search);
	DDX_Control(pDX, IDC_BUTTON_INMEM, m_button_inmem_search);
	DDX_Control(pDX, IDC_EXP2TMX_BUTTON_RUN, m_button_run);
	DDX_Control(pDX, IDC_EXP2TMX_CHECK_INMODE, m_check_inmode);
	DDX_Control(pDX, IDC_EXP2TMX_COMBO_OUTMODE, m_combo_outmode);
	DDX_Control(pDX, IDC_EXP2TMX_EDIT_INMEM, m_edit_inmem);
	DDX_Control(pDX, IDC_EXP2TMXCOMBO_INMODE, m_combo_inmode);
	//  DDX_Control(pDX, IDC_EXP2TMX_LIST_LOG, m_list_log);
	DDX_Control(pDX, IDC_EXP2TMX_EDIT_OUTMEM, m_edit_outmem);
	DDX_Control(pDX, IDC_EXP2TMX_CHECK_OUTMEM, m_check_outmem);
	DDX_Control(pDX, IDC_EXP2TMX_CHECK_OUTMODE, m_check_outmode);
	DDX_Control(pDX, IDC_EXP2TMX_CHECK_NOCRLF, m_chec_nocrlf);
}


BEGIN_MESSAGE_MAP(CDlgExp2Tmx, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_INMEM, &CDlgExp2Tmx::OnBnClickedButtonInmem)
    ON_BN_CLICKED(IDC_BUTTON_OUTMEM, &CDlgExp2Tmx::OnBnClickedButtonOutmem)
    ON_BN_CLICKED(IDC_EXP2TMX_BUTTON_RUN, &CDlgExp2Tmx::OnBnClickedExp2tmxButtonRun)
    ON_BN_CLICKED(IDC_EXP2TMX_CHECK_OUTMEM, &CDlgExp2Tmx::OnBnClickedExp2tmxCheckOutmem)
    ON_BN_CLICKED(IDC_EXP2TMX_CHECK_INMODE, &CDlgExp2Tmx::OnBnClickedExp2tmxCheckInmode)
    ON_BN_CLICKED(IDC_EXP2TMX_CHECK_OUTMODE, &CDlgExp2Tmx::OnBnClickedExp2tmxCheckOutmode)
END_MESSAGE_MAP()


// CDlgExp2Tmx message handlers


BOOL CDlgExp2Tmx::OnInitDialog()
{
    CDialog::OnInitDialog();

    //UTF16, ASCII, ANSI
    m_combo_inmode.InsertString(0,"UTF16");
    m_combo_inmode.InsertString(1,"ASCII");
    m_combo_inmode.InsertString(2,"ANSI");
    //UTF8 or UTF16
    m_combo_outmode.InsertString(0,"UTF8");
    m_combo_outmode.InsertString(1,"UTF16");

    m_edit_outmem.EnableWindow(m_check_outmem.GetCheck());
    m_combo_inmode.EnableWindow(m_check_inmode.GetCheck());
    m_combo_outmode.EnableWindow(m_check_outmode.GetCheck());

	m_chec_nocrlf.SetActiveWindow();

    return TRUE;  
}


void CDlgExp2Tmx::OnBnClickedButtonInmem()
{
    CString res = Commons::searchDialog(_T("EXP MEM Files (*.EXP)|*.EXP|All Files (*.*)|*.*||"));
    if(!res.IsEmpty())
    {
        m_edit_inmem.SetWindowText( res );

    }
}


void CDlgExp2Tmx::OnBnClickedButtonOutmem()
{
    CString res = Commons::searchDialog(_T("TMX MEM Files (*.TMX)|*.TMX|All Files (*.*)|*.*||"));
    if(!res.IsEmpty())
    {
        m_edit_outmem.SetWindowText( res );

    }
}


void CDlgExp2Tmx::OnBnClickedExp2tmxButtonRun()
{
    char szBuf[512+1] = {'\0'};
    
    int memcnt = m_edit_inmem.GetLine(0,szBuf,512);
    if(memcnt==0)
    {
        MessageBox("please input /INPUTMEM parameter");
        return;
    }
	szBuf[memcnt]='\0';
    std::string inmem(szBuf);

    
    int outmemcnt = m_edit_outmem.GetLine(0,szBuf,512);
    if(m_check_outmem.GetCheck() && outmemcnt==0)
    {
        MessageBox("please input /OUTPUT parameter");
        return;
    }
	szBuf[outmemcnt]='\0';
    std::string outmem(szBuf);

  
    int incnt = m_combo_inmode.GetCurSel();
    if(m_check_inmode.GetCheck() && incnt==-1)
    {
        MessageBox("please input /INMODE parameter");
        return;
    }

    int outcnt = m_combo_outmode.GetCurSel();
    if(m_check_outmode.GetCheck() && outcnt==-1)
    {
        MessageBox("please input /OUTMODE parameter");
        return;
    }


    std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
    {
        MessageBox("Cant' get OpenTM2 installed path");
        return;
    }   
    std::string opentm2path(szBuf);

    std::string cmd(opentm2path);
    cmd += "\\Win\\OtmExp2Tmx.exe ";
    cmd +="/INPUTMEM="+inmem; 

    if(outmemcnt!=0)
        cmd += " /OUTPUTMEM="+outmem;

    if(incnt!=-1 && m_check_inmode.GetCheck())
        cmd+=" /INMODE="+INMODE[m_combo_inmode.GetCurSel()];

    if(outcnt!=-1 && m_check_outmode.GetCheck())
        cmd+=" /OUTMODE="+OUTMODE[m_combo_outmode.GetCurSel()];

	if(m_chec_nocrlf.GetCheck())
	{
		cmd += " /NOCRLF";
	}
    // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}


void CDlgExp2Tmx::OnBnClickedExp2tmxCheckOutmem()
{
    m_edit_outmem.EnableWindow(m_check_outmem.GetCheck());
}


void CDlgExp2Tmx::OnBnClickedExp2tmxCheckInmode()
{
    m_combo_inmode.EnableWindow(m_check_inmode.GetCheck());
}


void CDlgExp2Tmx::OnBnClickedExp2tmxCheckOutmode()
{
    m_combo_outmode.EnableWindow(m_check_outmode.GetCheck());
}


void CDlgExp2Tmx::OnOK()
{
    // TODO: Add your specialized code here and/or call the base class

    //CDialog::OnOK();
}
