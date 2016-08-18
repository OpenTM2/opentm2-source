/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgMemoryTool.h"
#include "afxdialogex.h"
#include "DlgToolLauncher.h"


std::string TASKS[] = {"deleteMtProposal", "reverseMemory", "deleteIdentical", "ChgProposalMeta"};
// CDlgMemoryTool dialog

IMPLEMENT_DYNAMIC(CDlgMemoryTool, CDialogEx)

CDlgMemoryTool::CDlgMemoryTool(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgMemoryTool::IDD, pParent)
{

}

CDlgMemoryTool::~CDlgMemoryTool()
{
}

void CDlgMemoryTool::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MEMTOOLCOMBO_TASK, m_combo_task);
	//DDX_Control(pDX, IDC_MEMTOOL_EDIT_TOMARKUP, m_edit_tomarkup);
	//DDX_Control(pDX, IDC_MEMTOOL_EDIT_TOLANG, m_edit_tolang);
	DDX_Control(pDX, IDC_MEMTOOL_EDIT_REV, m_edit_rev);
	DDX_Control(pDX, IDC_MEMTOOL_EDIT_OUT, m_edit_out);
	DDX_Control(pDX, IDC_MEMTOOL_EDIT_MEM, m_edit_mem);
	//DDX_Control(pDX, IDC_MEMTOOL_EDIT_FROMMARKUP, m_edit_frommarkup);
	//DDX_Control(pDX, IDC_MEMTOOL_EDIT_FROMLANG, m_edit_fromlang);
	DDX_Control(pDX, IDC_MEMTOOL_EDIT_DOC, m_edit_doc);
	DDX_Control(pDX, IDC_MEMTOOL_EDIT_DATE, m_edit_date);
	DDX_Control(pDX, IDC_MEMTOOL_CHECK_TYPE, m_check_type);
	DDX_Control(pDX, IDC_MEMTOOL_CHECK_TOMAKRUP, m_check_tomarkup);
	DDX_Control(pDX, IDC_MEMTOOL_CHECK_TOLANG, m_check_tolang);
	DDX_Control(pDX, IDC_MEMTOOL_CHECK_SET, m_check_set);
	DDX_Control(pDX, IDC_MEMTOOL_CHECK_FROMMARKUP, m_check_frommarkup);
	DDX_Control(pDX, IDC_MEMTOOL_CHECK_FROMLANG, m_check_fromlang);
	DDX_Control(pDX, IDC_MEMTOOL_CHECK_DOC, m_check_doc);
	DDX_Control(pDX, IDC_MEMTOOL_CHECK_DATE, m_check_date);
	DDX_Control(pDX, IDC_MEMTOOL_CHECK_CLEAR, m_check_clear);
	DDX_Control(pDX, IDC_MEMTOOL_COMBO_FROMLANG, m_combo_fromlang);
	DDX_Control(pDX, IDC_MEMTOOL_COMBO_FROMMARKUP, m_combo_frommarkup);
	DDX_Control(pDX, IDC_MEMTOOL_COMBO_TOLANG, m_combo_tolang);
	//  DDX_Control(pDX, IDC_MEMTOOL_COMBO_TOMARKUP, m_combo_tomarpup);
	DDX_Control(pDX, IDC_MEMTOOL_COMBO_TOMARKUP, m_combo_tomarkup);
}


BEGIN_MESSAGE_MAP(CDlgMemoryTool, CDialogEx)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_MEMTOOL_BUTTON_RUN, &CDlgMemoryTool::OnBnClickedMemtoolButtonRun)
	ON_CBN_SELCHANGE(IDC_MEMTOOLCOMBO_TASK, &CDlgMemoryTool::OnSelchangeMemtoolcomboTask)
	ON_BN_CLICKED(IDC_MEMTOOL_CHECK_FROMMARKUP, &CDlgMemoryTool::OnBnClickedMemtoolCheckFrommarkup)
	ON_BN_CLICKED(IDC_MEMTOOL_CHECK_TOMAKRUP, &CDlgMemoryTool::OnBnClickedMemtoolCheckTomakrup)
	ON_BN_CLICKED(IDC_MEMTOOL_CHECK_FROMLANG, &CDlgMemoryTool::OnBnClickedMemtoolCheckFromlang)
	ON_BN_CLICKED(IDC_MEMTOOL_CHECK_TOLANG, &CDlgMemoryTool::OnBnClickedMemtoolCheckTolang)
	ON_BN_CLICKED(IDC_MEMTOOL_CHECK_DATE, &CDlgMemoryTool::OnBnClickedMemtoolCheckDate)
	ON_BN_CLICKED(IDC_MEMTOOL_CHECK_DOC, &CDlgMemoryTool::OnBnClickedMemtoolCheckDoc)
	ON_BN_CLICKED(IDC_MEMTOOL_CHECK_CLEAR, &CDlgMemoryTool::OnBnClickedMemtoolCheckClear)
	ON_BN_CLICKED(IDC_MEMTOOL_CHECK_SET, &CDlgMemoryTool::OnBnClickedMemtoolCheckSet)
	ON_CBN_SELCHANGE(IDC_MEMTOOL_COMBO_TOMARKUP, &CDlgMemoryTool::OnCbnSelchangeMemtoolComboTomarkup)
END_MESSAGE_MAP()


// CDlgMemoryTool message handlers


void CDlgMemoryTool::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}


BOOL CDlgMemoryTool::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect rc;
    GetClientRect(&rc);

	CRect parRect;
	CTabCtrl* pTab = ((CDlgToolLauncher*)this->GetParent())->getTabCtrl();
	pTab->GetClientRect(&parRect);

	
	SCROLLINFO si;  
    si.cbSize = sizeof(SCROLLINFO); 
    si.fMask = SIF_ALL;
    si.nMin = 0;
    si.nMax = rc.bottom-rc.top;
	si.nPage =  si.nMax-40;
    si.nPos = 0;
    SetScrollInfo(SB_VERT, &si); 


	Commons::getMarkuptables(m_Markups);
	for(std::size_t i=0; i<m_Markups.size(); i++)
	{
		m_combo_tomarkup.InsertString(i,m_Markups[i].c_str() );
		m_combo_frommarkup.InsertString(i,m_Markups[i].c_str() );
	}
	//Commons::getSourceLanguage(m_SrcLangs);
	//Commons::getTargetLanguage(m_TgtLangs);
	Commons::getLanguages(m_Langs);
	for(std::size_t i=0; i<m_Langs.size(); i++)
	{
		m_combo_fromlang.InsertString(i,m_Langs[i].c_str() );
		m_combo_tolang.InsertString(i,m_Langs[i].c_str() );
	}

    // windows init
	m_combo_tomarkup.EnableWindow(FALSE);
	m_combo_tolang.EnableWindow(FALSE);
	//m_edit_rev.EnableWindow(FALSE);
	//m_edit_out.EnableWindow(FALSE);
	m_combo_frommarkup.EnableWindow(FALSE);
	m_combo_fromlang.EnableWindow(FALSE);
	m_edit_doc.EnableWindow(FALSE);
	m_edit_date.EnableWindow(FALSE);

	for(int i=0; i<4; i++)
		m_combo_task.InsertString(i,TASKS[i].c_str());

	m_combo_task.SetCurSel(3);

	setStatusWithTask(TASKS[3]);



	return TRUE;  // return TRUE unless you set the focus to a control
}


void CDlgMemoryTool::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO scrollinfo;  
    GetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  

    switch (nSBCode)  
    {  
    case SB_BOTTOM:  
        ScrollWindow(0,(scrollinfo.nPos-scrollinfo.nMax)*10);  
        scrollinfo.nPos = scrollinfo.nMax;  
        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
        break;  

    case SB_TOP:  
        ScrollWindow(0,(scrollinfo.nPos-scrollinfo.nMin)*10);  
        scrollinfo.nPos = scrollinfo.nMin;  
        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
        break;  

    case SB_LINEUP:  
        scrollinfo.nPos -= 1;  
        if(scrollinfo.nPos < scrollinfo.nMin)
        {  
            scrollinfo.nPos = scrollinfo.nMin;  
            break;  
        }  
        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
        ScrollWindow(0,10);  
        break;

    case SB_LINEDOWN:  
        scrollinfo.nPos += 1;  
		if (scrollinfo.nPos+scrollinfo.nPage>scrollinfo.nMax)  
        {  
            scrollinfo.nPos = scrollinfo.nMax;  
            break;  
        }  
        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
        ScrollWindow(0,-10);  
        break;  

    case SB_PAGEUP:  
        scrollinfo.nPos -= 5;  
        if (scrollinfo.nPos < scrollinfo.nMin)
        {  
            scrollinfo.nPos = scrollinfo.nMin;  
            break;  
        }  
        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
        ScrollWindow(0,10*5);  
        break;  

    case SB_PAGEDOWN:  
        scrollinfo.nPos += 5;  
        if (scrollinfo.nPos+scrollinfo.nPage>scrollinfo.nMax)  
        {  
            scrollinfo.nPos = scrollinfo.nMax;  
            break;  
        }  
        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
        ScrollWindow(0,-10*5);  
        break;  

    case SB_ENDSCROLL:      
    case SB_THUMBPOSITION:  
        break;  

    case SB_THUMBTRACK:  
        ScrollWindow(0,(scrollinfo.nPos-nPos)*10);  
        scrollinfo.nPos = nPos;  
        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
        break;  
    }
	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CDlgMemoryTool::disableAll()
{
	m_combo_tomarkup.EnableWindow(FALSE);
	m_combo_tolang.EnableWindow(FALSE);
	m_edit_rev.EnableWindow(FALSE);
	m_edit_out.EnableWindow(FALSE);
	m_combo_frommarkup.EnableWindow(FALSE);
	m_combo_fromlang.EnableWindow(FALSE);
	m_edit_doc.EnableWindow(FALSE);
	m_edit_date.EnableWindow(FALSE);
	m_check_tomarkup.EnableWindow(FALSE);
	m_check_tolang.EnableWindow(FALSE);
	m_check_set.EnableWindow(FALSE);
	m_check_frommarkup.EnableWindow(FALSE);
	m_check_fromlang.EnableWindow(FALSE);
	m_check_doc.EnableWindow(FALSE);
	m_check_date.EnableWindow(FALSE);
	m_check_clear.EnableWindow(FALSE);

	m_check_tomarkup.SetCheck(FALSE);
	m_check_tolang.SetCheck(FALSE);
	m_check_set.SetCheck(FALSE);
	m_check_frommarkup.SetCheck(FALSE);
	m_check_fromlang.SetCheck(FALSE);
	m_check_doc.SetCheck(FALSE);
	m_check_date.SetCheck(FALSE);
	m_check_clear.SetCheck(FALSE);

	/*m_edit_tomarkup.SetWindowText("");
	m_edit_tolang.SetWindowText("");*/
	m_edit_rev.SetWindowText("");
	m_edit_out.SetWindowText("");
	//m_edit_frommarkup.SetWindowText("");
	//m_edit_fromlang.SetWindowText("");
	m_edit_doc.SetWindowText("");
	m_edit_date.SetWindowText("");
}


void CDlgMemoryTool::setStatusWithTask(const std::string& task)
{
	disableAll();

	if(task=="deleteMtProposal")
	{

	}
	else if(task=="reverseMemory")
	{
		m_edit_rev.EnableWindow(TRUE);
	}
	else if(task=="deleteIdentical")
	{
		m_edit_out.EnableWindow(TRUE);
	}
	else if(task=="ChgProposalMeta")
	{
		m_check_tomarkup.EnableWindow(TRUE);
		m_check_tolang.EnableWindow(TRUE);
		m_check_set.EnableWindow(TRUE);
		m_check_frommarkup.EnableWindow(TRUE);
		m_check_fromlang.EnableWindow(TRUE);
		m_check_doc.EnableWindow(TRUE);
		m_check_date.EnableWindow(TRUE);
		m_check_clear.EnableWindow(TRUE);
	}
}

void CDlgMemoryTool::OnBnClickedMemtoolButtonRun()
{
	char szBuf[512+1]={'\0'};

	std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
    {
        MessageBox("Cant' get OpenTM2 installed path");
        return;
    }   
    std::string opentm2path(szBuf);

	std::string task = TASKS[m_combo_task.GetCurSel()];

    std::string cmd(opentm2path);
    cmd += "\\Win\\OtmMemoryTool.exe ";
    cmd +=" /TASK="+task; 

	int memcnt = m_edit_mem.GetLine(0,szBuf,512);
    if(memcnt==0)
    {
        MessageBox("please input /Mem parameter");
        return;
    }
	szBuf[memcnt]='\0';
    std::string mem(szBuf);

	cmd+=" /Mem="+mem;

	if(task == "deleteMtProposal")
	{
		;
	}
	else if(task == "reverseMemory")
	{
		int revcnt = m_edit_rev.GetLine(0,szBuf,512);
        if(revcnt==0)
        {
            MessageBox("please input /Rev parameter");
            return;
        }
		szBuf[revcnt]='\0';
        std::string rev(szBuf);

		cmd+=" /REV="+rev;
	}
	else if(task == "deleteIdentical")
	{
		int outcnt = m_edit_out.GetLine(0,szBuf,512);
        if(outcnt==0)
        {
            MessageBox("please input /OUT parameter");
            return;
        }
		szBuf[outcnt]='\0';
        std::string outmem(szBuf);

		cmd+=" /OUT="+outmem;
	}
	else if(task == "ChgProposalMeta")
	{
		//
		int fromlangSel = m_combo_fromlang.GetCurSel();
		if(fromlangSel==-1 && m_check_fromlang.GetCheck())
		{
			MessageBox("please select /FROMLANG parameter");
            return;
		}

		if(m_check_fromlang.GetCheck() && fromlangSel!=-1)
		{
			cmd += " /FROMLANG="+m_Langs[fromlangSel];
		}

		//
		int tolangSel = m_combo_tolang.GetCurSel();
		if(tolangSel==-1 && m_check_tolang.GetCheck())
		{
			MessageBox("please select /TOLANG parameter");
            return;
		}

		if(m_check_tolang.GetCheck() && tolangSel!=-1)
		{
			cmd += " /TOLANG="+m_Langs[tolangSel];
		}
		
		//
		int frommarkupSel = m_combo_frommarkup.GetCurSel();
		if(frommarkupSel==-1 && m_check_frommarkup.GetCheck())
		{
			MessageBox("please select /FROMMARKUP parameter");
            return;
		}

		if(m_check_frommarkup.GetCheck() && frommarkupSel!=-1)
		{
			cmd += " /FROMMARKUP="+m_Markups[frommarkupSel];
		}

		//
		int tomarkupSel = m_combo_tomarkup.GetCurSel();
		if(tomarkupSel==-1 && m_check_tomarkup.GetCheck())
		{
			MessageBox("please select /TOMARKUP parameter");
            return;
		}

		if(m_check_tomarkup.GetCheck() && tomarkupSel!=-1)
		{
			cmd += " /TOMARKUP="+m_Markups[tomarkupSel];
		}

		//
		int datecnt = m_edit_date.GetLine(0,szBuf,512);
		if(datecnt==0 && m_check_date.GetCheck())
		{
			MessageBox("please input /DATE parameter");
            return;
		}
		szBuf[datecnt]='\0';
		if(m_check_date.GetCheck())
		{
		    std::string date(szBuf);
			cmd += " /DATE="+date;
		}


		//
		int doccnt = m_edit_doc.GetLine(0,szBuf,512);
		if(doccnt==0 && m_check_doc.GetCheck())
		{
			MessageBox("please input /DOC parameter");
            return;
		}
		szBuf[doccnt]='\0';
		if(m_check_doc.GetCheck())
		{
		    std::string doc(szBuf);
			cmd += " /DOC="+doc;
		}

		//
		if(m_check_clear.GetCheck())
		{
			cmd += " /CLEAR";
		}
		else if(m_check_set.GetCheck())
		{
			cmd += " /SET";
		}
		
	}

	if(m_check_type.GetCheck())
	{
		cmd += " /TYPE=noconf";
	}

	// execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);
}


void CDlgMemoryTool::OnSelchangeMemtoolcomboTask()
{
	setStatusWithTask(TASKS[m_combo_task.GetCurSel()]);
}


void CDlgMemoryTool::OnBnClickedMemtoolCheckFrommarkup()
{
	m_combo_frommarkup.EnableWindow(m_check_frommarkup.GetCheck());
}


void CDlgMemoryTool::OnBnClickedMemtoolCheckTomakrup()
{
	m_combo_tomarkup.EnableWindow(m_check_tomarkup.GetCheck());
}


void CDlgMemoryTool::OnBnClickedMemtoolCheckFromlang()
{
	m_combo_fromlang.EnableWindow(m_check_fromlang.GetCheck());
}


void CDlgMemoryTool::OnBnClickedMemtoolCheckTolang()
{
	m_combo_tolang.EnableWindow(m_check_tolang.GetCheck());
}


void CDlgMemoryTool::OnBnClickedMemtoolCheckDate()
{
		m_edit_date.EnableWindow(m_check_date.GetCheck());
}


void CDlgMemoryTool::OnBnClickedMemtoolCheckDoc()
{
		m_edit_doc.EnableWindow(m_check_doc.GetCheck());
}


void CDlgMemoryTool::OnBnClickedMemtoolCheckClear()
{
	if(m_check_clear.GetCheck())
	    m_check_set.SetCheck(FALSE);
}


void CDlgMemoryTool::OnBnClickedMemtoolCheckSet()
{
	if(m_check_set.GetCheck())
	    m_check_clear.SetCheck(FALSE);
}


void CDlgMemoryTool::OnCbnSelchangeMemtoolComboTomarkup()
{
	// TODO: Add your control notification handler code here
}
