/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgTmxSplitSeg.h"
#include "afxdialogex.h"
#include "Commons.h"

std::string TYPE[3] = {"E", "W", "A"};
// CDlgTmxSplitSeg dialog

IMPLEMENT_DYNAMIC(CDlgTmxSplitSeg, CDialog)

CDlgTmxSplitSeg::CDlgTmxSplitSeg(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgTmxSplitSeg::IDD, pParent)
{

}

CDlgTmxSplitSeg::~CDlgTmxSplitSeg()
{
}

void CDlgTmxSplitSeg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //  DDX_Control(pDX, IDC_BUTTON_TMSPLITSEG_INMEM, m_check_inmem);
    DDX_Control(pDX, IDC_BUTTON_TMSSPLITSEG_OUTMEM, m_button_outmem);
    DDX_Control(pDX, IDC_BUTTON_TMSPLITSEG_INMEM, m_button_inmem);
    DDX_Control(pDX, IDC_COMBO_TMXSPLITSEG_LOGS, m_combo_logs);
    DDX_Control(pDX, IDC_EDIT_TMXSPLITSEG_LOG, m_edit_log);
    DDX_Control(pDX, IDC_EDIT_TMXSPLITSEG_OUTMEM, m_edit_outmem);
    DDX_Control(pDX, IDC_TMXSPLITSEG_CHECK_LOG, m_check_log);
    DDX_Control(pDX, IDC_TMXSPLITSEG_CHECK_LOGS, m_check_logs);
    DDX_Control(pDX, IDC_TMXSPLITSEG_CHECK_OUTPUTMEM, m_check_outputmem);
    DDX_Control(pDX, IDC_TMXSPLITSEG_EDIT_INMEM, m_edit_inmem);
}


BEGIN_MESSAGE_MAP(CDlgTmxSplitSeg, CDialog)

    ON_BN_CLICKED(IDC_BUTTON_TMSPLITSEG_INMEM, &CDlgTmxSplitSeg::OnBnClickedButtonTmsplitsegInmem)
    ON_BN_CLICKED(IDC_BUTTON_TMSSPLITSEG_OUTMEM, &CDlgTmxSplitSeg::OnBnClickedButtonTmssplitsegOutmem)
    ON_BN_CLICKED(IDC_TMXSPLITSEG_BUTTON_RUN, &CDlgTmxSplitSeg::OnBnClickedTmxsplitsegButtonRun)
    ON_BN_CLICKED(IDC_TMXSPLITSEG_CHECK_OUTPUTMEM, &CDlgTmxSplitSeg::OnBnClickedTmxsplitsegCheckOutputmem)
    ON_BN_CLICKED(IDC_TMXSPLITSEG_CHECK_LOGS, &CDlgTmxSplitSeg::OnBnClickedTmxsplitsegCheckLogs)
    ON_BN_CLICKED(IDC_TMXSPLITSEG_CHECK_LOG, &CDlgTmxSplitSeg::OnBnClickedTmxsplitsegCheckLog)
END_MESSAGE_MAP()


// CDlgTmxSplitSeg message handlers



BOOL CDlgTmxSplitSeg::OnInitDialog()
{
    CDialog::OnInitDialog();
    //ASCII, ANSI, UTF16 or TMX
    m_combo_logs.InsertString(0,"E");
    m_combo_logs.InsertString(1,"W");
    m_combo_logs.InsertString(2,"A");

    m_edit_outmem.EnableWindow(m_check_outputmem.GetCheck());
    m_edit_log.EnableWindow(m_check_log.GetCheck());
    m_combo_logs.EnableWindow(m_check_logs.GetCheck());
    return TRUE; 
}


void CDlgTmxSplitSeg::OnBnClickedButtonTmsplitsegInmem()
{
    CString res = Commons::searchDialog(_T("TMX MEM Files (*.TMX)|*.TMX|All Files (*.*)|*.*||"));
    if(!res.IsEmpty())
    {
        m_edit_inmem.SetWindowText( res );

    }
}


void CDlgTmxSplitSeg::OnBnClickedButtonTmssplitsegOutmem()
{
    CString res = Commons::searchDialog(_T("TMX MEM Files (*.TMX)|*.TMX|All Files (*.*)|*.*||"));
    if(!res.IsEmpty())
    {
        m_edit_outmem.SetWindowText( res );

    }
}


void CDlgTmxSplitSeg::OnBnClickedTmxsplitsegButtonRun()
{
     char szBuf[512+1] = {'\0'};
    
    int inmemcnt = m_edit_inmem.GetLine(0,szBuf,512);
    if(inmemcnt==0)
    {
        MessageBox("please input inputmem parameter");
        return;
    }
    szBuf[inmemcnt]='\0';
    std::string inmem(szBuf);

    int outmemcnt = m_edit_outmem.GetLine(0,szBuf,512);
    if(m_check_outputmem.GetCheck() && outmemcnt==0)
    {
        MessageBox("please input outputmem parameter");
        return;
    }
    szBuf[outmemcnt]='\0';
    std::string outmem(szBuf);

    int logscnt = m_combo_logs.GetCurSel();
    if(m_check_logs.GetCheck() && logscnt==-1)
    {
        MessageBox("please input /LOGS parameter");
        return;
    }

    int logcnt = m_edit_log.GetLine(0,szBuf,512);
    if(m_check_log.GetCheck() && logcnt==0)
    {
        MessageBox("please input /MArkup parameter");
        return;
    }    
    szBuf[logcnt]='\0';
    std::string log(szBuf);

    std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
    {
        MessageBox("Cant' get OpenTM2 installed path");
        return;
    }  
    std::string opentm2path(szBuf);

    std::string cmd(opentm2path);
    cmd += "\\Win\\OtmTmxSplitSegments.exe ";
    cmd +="  "+inmem; 
    if(m_check_outputmem.GetCheck())
         cmd += "  "+outmem;
    if(m_check_logs.GetCheck())
        cmd += "  /LOGS="+TYPE[logscnt];
    if(m_check_log.GetCheck())
        cmd +="  /log="+log;

     // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}


void CDlgTmxSplitSeg::OnBnClickedTmxsplitsegCheckOutputmem()
{
     m_edit_outmem.EnableWindow(m_check_outputmem.GetCheck());
}


void CDlgTmxSplitSeg::OnBnClickedTmxsplitsegCheckLogs()
{
     m_combo_logs.EnableWindow(m_check_logs.GetCheck());
}


void CDlgTmxSplitSeg::OnBnClickedTmxsplitsegCheckLog()
{
    m_edit_log.EnableWindow(m_check_log.GetCheck());
}


void CDlgTmxSplitSeg::OnOK()
{
    // TODO: Add your specialized code here and/or call the base class

   // CDialog::OnOK();
}


//void CDlgTmxSplitSeg::OnCancel()
//{
//    // TODO: Add your specialized code here and/or call the base class
//
//   // CDialog::OnCancel();
//}
