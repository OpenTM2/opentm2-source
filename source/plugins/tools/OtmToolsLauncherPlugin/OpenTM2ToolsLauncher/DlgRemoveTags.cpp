/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgRemoveTags.h"
#include "afxdialogex.h"
#include "Commons.h"

// CDlgRemoveTags dialog
char* MODES[] = {"UTF16", "ASCII", "ANSI","TMX", "INTERNAL"};
IMPLEMENT_DYNAMIC(CDlgRemoveTags, CDialog)

CDlgRemoveTags::CDlgRemoveTags(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgRemoveTags::IDD, pParent)
{

}

CDlgRemoveTags::~CDlgRemoveTags()
{
}

void CDlgRemoveTags::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//  DDX_Control(pDX, IDC_REMOVETAGS_EDIT_INMEM, m_inmem);
	//  DDX_Control(pDX, IDC_REMOVETAGS_EDIT_MARKUP, m_markup);
	DDX_Control(pDX, IDC_REMOVETAGS_EDIT_INMEM, m_edit_inmem);
	//DDX_Control(pDX, IDC_REMOVETAGS_EDIT_MARKUP, m_edit_markup);
	DDX_Control(pDX, IDC_REMOVETAGS_EDIT_OUTMEM, m_edit_outmem);
	DDX_Control(pDX, IDC_REMOVETAGSCOMBO_INMODE, m_combo_inmode);
	DDX_Control(pDX, IDC_RMEOVETAGS_COMBO_OUTMODE, m_combo_outmode);
	DDX_Control(pDX, IDC_BUTTON_INMEM, m_btn_inmem_search);
	DDX_Control(pDX, IDC_BUTTON_OUTMEM, m_btn_outmem_search);
	DDX_Control(pDX, IDC_REMOETAGS_CHECK_MARKUP, m_check_markup);
	DDX_Control(pDX, IDC_RMEOVETAGS_COMBO_MARKUP, m_combo_markup);
}


BEGIN_MESSAGE_MAP(CDlgRemoveTags, CDialog)
	ON_BN_CLICKED(IDC_REMOETAGS_CHECK_MARKUP, &CDlgRemoveTags::OnClickedRemoetagsCheckMarkup)
	ON_BN_CLICKED(IDC_BUTTON_INMEM, &CDlgRemoveTags::OnClickedButtonInmem)
	ON_BN_CLICKED(IDC_BUTTON_OUTMEM, &CDlgRemoveTags::OnBnClickedButtonOutmem)
	ON_BN_CLICKED(IDC_REMOVETAGS_BUTTON_RUN, &CDlgRemoveTags::OnBnClickedRemovetagsButtonRun)
END_MESSAGE_MAP()


// CDlgRemoveTags message handlers


BOOL CDlgRemoveTags::OnInitDialog()
{
	CDialog::OnInitDialog();

	//UTF16, ASCII, ANSI
    m_combo_inmode.InsertString(0,"UTF16");
    m_combo_inmode.InsertString(1,"ASCII");
    m_combo_inmode.InsertString(2,"ANSI");
	m_combo_inmode.InsertString(3,"TMX");

    //UTF8 or UTF16
    m_combo_outmode.InsertString(0,"UTF16");
    m_combo_outmode.InsertString(1,"ASCII");
    m_combo_outmode.InsertString(2,"ANSI");
	m_combo_outmode.InsertString(3,"TMX");
	m_combo_outmode.InsertString(4,"INTERNAL");

	Commons::getMarkuptables(m_Markups);
	for(std::size_t i=0; i<m_Markups.size(); i++)
	{
		m_combo_markup.InsertString(i,m_Markups[i].c_str() );
	}
    m_combo_markup.EnableWindow(m_check_markup.GetCheck());

	return TRUE; 

}


void CDlgRemoveTags::OnClickedRemoetagsCheckMarkup()
{
	  m_combo_markup.EnableWindow(m_check_markup.GetCheck());
}


void CDlgRemoveTags::OnClickedButtonInmem()
{
	 CString res = Commons::searchDialog("");
    if(!res.IsEmpty())
    {
        m_edit_inmem.SetWindowText( res );

    }
}


void CDlgRemoveTags::OnBnClickedButtonOutmem()
{
	CString res = Commons::searchDialog("");
    if(!res.IsEmpty())
    {
        m_edit_outmem.SetWindowText( res );

    }
}


void CDlgRemoveTags::OnBnClickedRemovetagsButtonRun()
{
	char szBuf[512+1] = {'\0'};
    
    int memcnt = m_edit_inmem.GetLine(0,szBuf,512);
    if(memcnt==0)
    {
        MessageBox("please input /INMEM parameter");
        return;
    }
	szBuf[memcnt]='\0';
    std::string inmem(szBuf);

    
    int outmemcnt = m_edit_outmem.GetLine(0,szBuf,512);
    if(outmemcnt==0)
    {
        MessageBox("please input /OUTMEM parameter");
        return;
    }
	szBuf[outmemcnt]='\0';
    std::string outmem(szBuf);

	int incnt = m_combo_inmode.GetCurSel();
    if(incnt==-1)
    {
        MessageBox("please select /INTYPE parameter");
        return;
    }
	std::string inmode = MODES[incnt];

	int outcnt = m_combo_outmode.GetCurSel();
    if(outcnt==-1)
    {
        MessageBox("please select /OUTTYPE parameter");
        return;
    }
	std::string outmode = MODES[outcnt];

	int markupsel = m_combo_markup.GetCurSel();
    if(m_check_markup.GetCheck() && markupsel==-1)
    {
        MessageBox("please select /MArkup parameter");
        return;
    }
	std::string markup(m_Markups[markupsel]);

	std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
    {
        MessageBox("Cant' get OpenTM2 installed path");
        return;
    }   
    std::string opentm2path(szBuf);

    std::string cmd(opentm2path);
    cmd += "\\Win\\OtmRemoveTags.exe ";
    cmd +=" /INMEM="+inmem; 
	cmd +=" /OUTMEM="+outmem; 
	cmd +=" /INTYPE="+inmode; 
	cmd +=" /OUTTYPE="+outmode; 

	if(markupsel!=-1 && m_check_markup.GetCheck())
	{
		cmd += " /MArkup="+markup;
	}

	
    // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}


void CDlgRemoveTags::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialog::OnOK();
}
