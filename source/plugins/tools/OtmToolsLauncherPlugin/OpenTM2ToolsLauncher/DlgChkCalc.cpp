/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgChkCalc.h"
#include "afxdialogex.h"
#include "Commons.h"

// CDlgChkCalc dialog

IMPLEMENT_DYNAMIC(CDlgChkCalc, CDialogEx)

CDlgChkCalc::CDlgChkCalc(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgChkCalc::IDD, pParent)
{

}

CDlgChkCalc::~CDlgChkCalc()
{
}

void CDlgChkCalc::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_CHK_FLD, m_button_fld);
    DDX_Control(pDX, IDC_BUTTON_CHK_FXP, m_button_fxp);
    DDX_Control(pDX, IDC_BUTTON_CHK_HLOG, m_button_hlog);
    //  DDX_Control(pDX, IDC_BUTTON_CHKCALC_RUN, m_chk_run);
    DDX_Control(pDX, IDC_CHECK_CHK_ALL, m_check_all);
    DDX_Control(pDX, IDC_CHECK_CHK_FLD, m_check_fld);
    DDX_Control(pDX, IDC_CHECK_CHK_FXP, m_check_fxp);
    DDX_Control(pDX, IDC_CHECK_CHK_HLOG, m_check_hlog);
    DDX_Control(pDX, IDC_CHK_EDIT_FLD, m_edit_fld);
    DDX_Control(pDX, IDC_CHK_EDIT_FXP, m_edit_fxp);
    DDX_Control(pDX, IDC_CHK_EDIT_HLOG, m_edit_hlog);
}


BEGIN_MESSAGE_MAP(CDlgChkCalc, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_CHK_FLD, &CDlgChkCalc::OnBnClickedButtonChkFld)
    ON_BN_CLICKED(IDC_BUTTON_CHK_FXP, &CDlgChkCalc::OnBnClickedButtonChkFxp)
    ON_BN_CLICKED(IDC_BUTTON_CHK_HLOG, &CDlgChkCalc::OnBnClickedButtonChkHlog)
    ON_BN_CLICKED(IDC_BUTTON_CHKCALC_RUN, &CDlgChkCalc::OnBnClickedButtonChkcalcRun)
    ON_BN_CLICKED(IDC_CHECK_CHK_FLD, &CDlgChkCalc::OnBnClickedCheckChkFld)
    ON_BN_CLICKED(IDC_CHECK_CHK_FXP, &CDlgChkCalc::OnBnClickedCheckChkFxp)
    ON_BN_CLICKED(IDC_CHECK_CHK_HLOG, &CDlgChkCalc::OnBnClickedCheckChkHlog)
    ON_BN_CLICKED(IDC_CHECK_CHK_ALL, &CDlgChkCalc::OnBnClickedCheckChkAll)
END_MESSAGE_MAP()


// CDlgChkCalc message handlers


void CDlgChkCalc::OnBnClickedButtonChkFld()
{
    CString res = Commons::searchFolder();
    if(!res.IsEmpty())
        m_edit_fld.SetWindowText( Commons::getFileNameFromPath(res) );
}


void CDlgChkCalc::OnBnClickedButtonChkFxp()
{
    CString res = Commons::searchDialog(_T("FXP Files (*.FXP)|*.FXP|All Files (*.*)|*.*||"));
    if(!res.IsEmpty())
        m_edit_fxp.SetWindowText( res );
}


void CDlgChkCalc::OnBnClickedButtonChkHlog()
{
    // TODO: decide the filter
   CString res = Commons::searchDialog(_T("LOG Files (*.*)|*.*|All Files (*.*)|*.*||"));
    if(!res.IsEmpty())
        m_edit_hlog.SetWindowText( res );
}


void CDlgChkCalc::OnBnClickedButtonChkcalcRun()
{
    char szBuf[512+1] = {'\0'};
    
    int fldcnt = m_edit_fld.GetLine(0,szBuf,512);
    if(m_check_fld.GetCheck() && fldcnt==0)
    {
        MessageBox("please input /FLD parameter");
        return;
    }
	szBuf[fldcnt]='\0';
    std::string fld(szBuf);

    int fxpcnt = m_edit_fxp.GetLine(0,szBuf,512);
    if(m_check_fxp.GetCheck() && fxpcnt==0)
    {
        MessageBox("please input /FXP parameter");
        return;
    }
    szBuf[fxpcnt]='\0';
    std::string fxp(szBuf);

    int hlogcnt = m_edit_hlog.GetLine(0,szBuf,512);
    if(m_check_hlog.GetCheck() && hlogcnt==0)
    {
        MessageBox("please input /HLOG parameter");
        return;
    }
	szBuf[hlogcnt]='\0';
    std::string hlog(szBuf);

    
    if(fldcnt==0 && fxpcnt==0 && hlogcnt==0 && !m_check_all.GetCheck())
    {
        MessageBox("please select at least one parameter");
        return;
    }

    std::string parameters;

    if(m_check_all.GetCheck())
    {
        parameters += " /ALL";
    }
    else
    {
        if(m_check_fld.GetCheck() && fldcnt!=0)
            parameters += " /FLD="+fld;

        if(m_check_fxp.GetCheck() && fxpcnt != 0)
            parameters += " /FXP="+fxp;

        if(m_check_hlog.GetCheck() && hlogcnt != 0)
            parameters += " /HLOG="+hlog;
    }

    std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
    {
        MessageBox("Cant' get OpenTM2 installed path");
        return;
    }   
    std::string opentm2path(szBuf);

    std::string cmd(opentm2path);
    cmd += "\\Win\\OtmChkCalc.exe ";

    cmd += parameters;

    // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}

void CDlgChkCalc::OnBnClickedCheckChkFld()
{
    m_check_all.EnableWindow(!m_check_fld.GetCheck());
    m_check_fxp.EnableWindow(!m_check_fld.GetCheck());
    m_check_hlog.EnableWindow(!m_check_fld.GetCheck());
    m_edit_fld.EnableWindow(m_check_fld.GetCheck());
}


void CDlgChkCalc::OnBnClickedCheckChkFxp()
{
    m_check_all.EnableWindow(!m_check_fxp.GetCheck());
    m_check_fld.EnableWindow(!m_check_fxp.GetCheck());
    m_check_hlog.EnableWindow(!m_check_fxp.GetCheck() );
    m_edit_fxp.EnableWindow(m_check_fxp.GetCheck());
}


void CDlgChkCalc::OnBnClickedCheckChkHlog()
{
    m_check_all.EnableWindow(!m_check_hlog.GetCheck());
    m_check_fld.EnableWindow(!m_check_hlog.GetCheck());
    m_check_fxp.EnableWindow(!m_check_hlog.GetCheck());
    m_edit_hlog.EnableWindow(m_check_hlog.GetCheck());
}


void CDlgChkCalc::OnBnClickedCheckChkAll()
{

    m_check_fld.EnableWindow(!m_check_all.GetCheck());
    m_check_fxp.EnableWindow(!m_check_all.GetCheck());
    m_check_hlog.EnableWindow(!m_check_all.GetCheck());
}


BOOL CDlgChkCalc::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_edit_fld.EnableWindow(m_check_fld.GetCheck());
    m_edit_fxp.EnableWindow(m_check_fxp.GetCheck());
    m_edit_hlog.EnableWindow(m_check_hlog.GetCheck());

    return TRUE;  
}


void CDlgChkCalc::OnOK()
{
    // TODO: Add your specialized code here and/or call the base class

    //CDialogEx::OnOK();
}
