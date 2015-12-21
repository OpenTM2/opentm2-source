/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgChangeFxp.h"
#include "afxdialogex.h"
#include "Commons.h"

// CDlgChangeFxp dialog

static std::string TYPE[3] = {"MASTER","CHILD","STANDARD"};

IMPLEMENT_DYNAMIC(CDlgChangeFxp, CDialog)

CDlgChangeFxp::CDlgChangeFxp(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgChangeFxp::IDD, pParent)
{

}

CDlgChangeFxp::~CDlgChangeFxp()
{
}

void CDlgChangeFxp::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_FOLDERFILE, m_btn_folderfile);
    DDX_Control(pDX, IDC_CHANGEFXP_CHECK_PW, m_check_pw);
    DDX_Control(pDX, IDC_CHANGEFXP_CHECK_SHIP, m_check_ship);
    DDX_Control(pDX, IDC_CHANGEFXP_CHECK_TYPE, m_check_type);
    DDX_Control(pDX, IDC_CHANGEFXP_EDIT_FOLDERFILE, m_edit_folderfile);
    DDX_Control(pDX, IDC_CHANGEFXP_EDIT_PW, m_edit_pw);
    DDX_Control(pDX, IDC_CHANgeFXP_EDIT_SHIP, m_edit_ship);
    DDX_Control(pDX, IDC_CHANGEFXPCOMBO_TYPE, m_combo_type);
}


BEGIN_MESSAGE_MAP(CDlgChangeFxp, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_FOLDERFILE, &CDlgChangeFxp::OnBnClickedButtonFolderfile)
    ON_BN_CLICKED(IDC_CHANGEFXP_CHECK_TYPE, &CDlgChangeFxp::OnBnClickedChangefxpCheckType)
    ON_BN_CLICKED(IDC_CHANGEFXP_CHECK_PW, &CDlgChangeFxp::OnBnClickedChangefxpCheckPw)
    ON_BN_CLICKED(IDC_CHANGEFXP_CHECK_SHIP, &CDlgChangeFxp::OnBnClickedChangefxpCheckShip)
    ON_BN_CLICKED(IDC_CHANGEFXP_BUTTON_RUN, &CDlgChangeFxp::OnBnClickedChangefxpButtonRun)
END_MESSAGE_MAP()


// CDlgChangeFxp message handlers


void CDlgChangeFxp::OnBnClickedButtonFolderfile()
{
    CString res = Commons::searchDialog(_T("FXP Files (*.FXP)|*.FXP|All Files (*.*)|*.*||"));
    if(!res.IsEmpty())
        m_edit_folderfile.SetWindowText( res );
}


void CDlgChangeFxp::OnBnClickedChangefxpCheckType()
{
    m_combo_type.EnableWindow(m_check_type.GetCheck());
}


void CDlgChangeFxp::OnBnClickedChangefxpCheckPw()
{
    m_edit_pw.EnableWindow(m_check_pw.GetCheck());
}


void CDlgChangeFxp::OnBnClickedChangefxpCheckShip()
{
    m_edit_ship.EnableWindow(m_check_ship.GetCheck());
}


BOOL CDlgChangeFxp::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_combo_type.InsertString(0,"MASTER");
    m_combo_type.InsertString(1,"CHILD");
    m_combo_type.InsertString(2,"STANDARD");

    m_edit_pw.EnableWindow(m_check_pw.GetCheck());
    m_edit_ship.EnableWindow(m_check_ship.GetCheck());
    m_combo_type.EnableWindow(m_check_type.GetCheck());

    return TRUE; 

}


void CDlgChangeFxp::OnBnClickedChangefxpButtonRun()
{
    char szBuf[512+1] = {'\0'};
    
    int ffcnt = m_edit_folderfile.GetLine(0,szBuf,512);
    if(ffcnt==0)
    {
        MessageBox("please input folderfile parameter");
        return;
    }
	szBuf[ffcnt]='\0';
    std::string folderfile(szBuf); 

    int typecnt = m_combo_type.GetCurSel();
    if(m_check_type.GetCheck() && typecnt==-1)
    {
        MessageBox("please input /TYPE parameter");
        return;
    }
	szBuf[typecnt]='\0';
    std::string type(szBuf);

    int pwcnt = m_edit_pw.GetLine(0,szBuf,512);
    if(m_check_pw.GetCheck() && pwcnt==0)
    {
        MessageBox("please input /PW parameter");
        return;
    }
	szBuf[pwcnt]='\0';
    std::string pw(szBuf);

    int shipcnt = m_edit_ship.GetLine(0,szBuf,512);
    if(m_check_ship.GetCheck() && shipcnt==0)
    {
        MessageBox("please input /SHIP parameter");
        return;
    }
	szBuf[shipcnt]='\0';
    std::string ship(szBuf);

     std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
    {
        MessageBox("Cant' get OpenTM2 installed path");
        return;
    }   
    std::string opentm2path(szBuf);

    std::string cmd(opentm2path);
    cmd += "\\Win\\OtmChangeFxp.exe ";

    cmd += folderfile; 

    if(typecnt!=-1 && m_check_type.GetCheck())
        cmd+=" /TYPE="+TYPE[m_combo_type.GetCurSel()];

    if(pwcnt!=0)
        cmd += " /PW="+pw;

    if(shipcnt!=0)
        cmd += " /SHIP="+ship;

     // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}


void CDlgChangeFxp::OnOK()
{
    // TODO: Add your specialized code here and/or call the base class

    //CDialog::OnOK();
}
