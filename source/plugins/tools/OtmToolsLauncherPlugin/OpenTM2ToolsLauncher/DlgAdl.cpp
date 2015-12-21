/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgAdl.h"
#include "afxdialogex.h"
#include "Commons.h"
#include <string>
#include <fstream>
// CDlgAdl dialog

IMPLEMENT_DYNAMIC(CDlgAdl, CDialog)

CDlgAdl::CDlgAdl(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgAdl::IDD, pParent)
{

}

CDlgAdl::~CDlgAdl()
{
}

void CDlgAdl::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ADL_CHECK_DIC, m_check_dict);
    DDX_Control(pDX, IDC_CHECK_ADL_FOL, m_check_fol);
    DDX_Control(pDX, IDC_CHECK_ADL_MEM, m_check_mem);
    DDX_Control(pDX, IDC_ADL_EDIT_DICT, m_edit_dict);
    DDX_Control(pDX, IDC_ADL_EDIT_FOL, m_edit_fol);
   /* DDX_Control(pDX, IDC_BUTTON_ADL_DICT, m_button_dict_search);
    DDX_Control(pDX, IDC_BUTTON_ADL_FOL, m_button_fol_search);
    DDX_Control(pDX, IDC_BUTTON_ADL_MEM, m_button_mem_search);*/
    DDX_Control(pDX, IDC_BUTTON_ADL_RUN, m_button_run);
    DDX_Control(pDX, IDC_LIST_ADL_LOG, m_list_log);
    DDX_Control(pDX, IDC_ADL_EDIT_MEM, m_edit_mem);
}


BEGIN_MESSAGE_MAP(CDlgAdl, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_ADL_RUN, &CDlgAdl::OnBnClickedButtonAdlRun)
    /*ON_BN_CLICKED(IDC_BUTTON_ADL_MEM, &CDlgAdl::OnBnClickedButtonAdlMem)
    ON_BN_CLICKED(IDC_BUTTON_ADL_FOL, &CDlgAdl::OnBnClickedButtonAdlFol)
    ON_BN_CLICKED(IDC_BUTTON_ADL_DICT, &CDlgAdl::OnBnClickedButtonAdlDict)*/
    ON_BN_CLICKED(IDC_CHECK_ADL_FOL, &CDlgAdl::OnClickedCheckAdlFol)
    ON_BN_CLICKED(IDC_CHECK_ADL_MEM, &CDlgAdl::OnClickedCheckAdlMem)
    ON_BN_CLICKED(IDC_ADL_CHECK_DIC, &CDlgAdl::OnClickedAdlCheckDic)
END_MESSAGE_MAP()


// CDlgAdl message handlers


void CDlgAdl::OnBnClickedButtonAdlRun()
{
    char szBuf[512+1] = {'\0'};
    
    int memcnt = m_edit_mem.GetLine(0,szBuf,512);
    if(m_check_mem.GetCheck() && memcnt==0)
    {
        MessageBox("please input /MEM parameter");
        return;
    }
	szBuf[memcnt]='\0';
    std::string mem(szBuf);

    
    int folcnt = m_edit_fol.GetLine(0,szBuf,512);
    if(m_check_fol.GetCheck() && folcnt==0)
    {
        MessageBox("please input /FOL parameter");
        return;
    }
	szBuf[folcnt]='\0';
    std::string fol(szBuf);

  
    int dictcnt = m_edit_dict.GetLine(0,szBuf,512);
    if(m_check_dict.GetCheck() && dictcnt==0)
    {
        MessageBox("please input /DICT parameter");
        return;
    }
	szBuf[dictcnt]='\0';
    std::string dict(szBuf);

    if(!m_check_mem.GetCheck() &&
       !m_check_fol.GetCheck() &&
       !m_check_dict.GetCheck() )
    {
         MessageBox("please select at least one");
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
    cmd += "\\Win\\OtmAdl.exe ";

    if(memcnt!=0)
        cmd += "/MEM="+mem;

    if(folcnt!=0)
        cmd+=" /FOL="+fol;

    if(dictcnt!=0)
        cmd+=" /DIC="+dict;

    // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);

}


//void CDlgAdl::OnBnClickedButtonAdlMem()
//{
//    CString res = Commons::searchDialog(_T("MEM Files (*.MEM)|*.MEM|All Files (*.*)|*.*||"));
//    if(!res.IsEmpty())
//    {
//        m_edit_mem.SetWindowText( Commons::getFileNameFromPath(res) );
//
//    }
//}


//void CDlgAdl::OnBnClickedButtonAdlFol()
//{
//    CString res = Commons::searchDialog(_T("FOL Files (*.F00)|*.F00|All Files (*.*)|*.*||"));
//    if(!res.IsEmpty())
//        m_edit_fol.SetWindowText( Commons::getFileNameFromPath(res) );
//}


//void CDlgAdl::OnBnClickedButtonAdlDict()
//{
//    CString res = Commons::searchDialog(_T("DIC Files (*.PRO)|*.PRO|All Files (*.*)|*.*||"));
//    if(!res.IsEmpty())
//        m_edit_dict.SetWindowText( Commons::getFileNameFromPath(res) );
//}


BOOL CDlgAdl::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  Add extra initialization here
    if(!m_check_mem.GetCheck())
       m_edit_mem.EnableWindow(FALSE);

    if(!m_check_fol.GetCheck())
       m_edit_fol.EnableWindow(FALSE);

    if(!m_check_dict.GetCheck())
       m_edit_dict.EnableWindow(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgAdl::OnClickedCheckAdlFol()
{
   m_edit_fol.EnableWindow(m_check_fol.GetCheck());
}


void CDlgAdl::OnClickedCheckAdlMem()
{
    m_edit_mem.EnableWindow(m_check_mem.GetCheck());
}


void CDlgAdl::OnClickedAdlCheckDic()
{
    m_edit_dict.EnableWindow(m_check_dict.GetCheck());
}


void CDlgAdl::OnOK()
{
    // TODO: Add your specialized code here and/or call the base class

    //CDialog::OnOK();
}
