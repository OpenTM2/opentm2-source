/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgTmx2Exp.h"
#include "afxdialogex.h"
#include "Commons.h"
#include "DlgToolLauncher.h"

// CDlgTmx2Exp dialog

IMPLEMENT_DYNAMIC(CDlgTmx2Exp, CDialogEx)

CDlgTmx2Exp::CDlgTmx2Exp(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgTmx2Exp::IDD, pParent)
{

}

CDlgTmx2Exp::~CDlgTmx2Exp()
{
}

void CDlgTmx2Exp::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Control(pDX, IDC_TMX2EXP_EDIT_INMEM, m_edir_inmem);
	DDX_Control(pDX, IDC_TMX2EXP_EDIT_OUTMEM, m_edit_outmem);
	//  DDX_Control(pDX, IDC_TMX2EXPCOMBO_INMODE, m_combo_inmode);
	DDX_Control(pDX, IDC_TMX2EXP_EDIT_MARKUP, m_edit_markup);
	DDX_Control(pDX, IDC_TMX2EXP_CHECK_CLEANTF, m_check_clearntf);
	DDX_Control(pDX, IDC_TMX2EXP_CHECK_INCLUDEBRACE, m_check_includebrace);
	DDX_Control(pDX, IDC_tMX2EXP_CHECK_MARKUP, m_check_markup);
	DDX_Control(pDX, IDC_TMX2EXP_CHECK_OUTMEM, m_check_outmem);
	DDX_Control(pDX, IDC_TMX2EXP_CHECK_OUTMODE, m_check_outmode);
	DDX_Control(pDX, IDC_TMX2EXP_CHECK_SS, m_check_ss);
	DDX_Control(pDX, IDC_TMX2EXPCOMBO_OUTMODE, m_combo_outmode);
	DDX_Control(pDX, IDC_TMX2EXP_EDIT_INMEM, m_edit_inmem);
}


BEGIN_MESSAGE_MAP(CDlgTmx2Exp, CDialogEx)
//	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_TMX2EXP_CHECK_SS, &CDlgTmx2Exp::OnBnClickedTmx2expCheckSs)
	ON_BN_CLICKED(IDC_TMX2EXP_CHECK_INCLUDEBRACE, &CDlgTmx2Exp::OnBnClickedTmx2expCheckIncludebrace)
	ON_BN_CLICKED(IDC_TMX2EXP_CHECK_CLEANTF, &CDlgTmx2Exp::OnBnClickedTmx2expCheckCleantf)
	ON_BN_CLICKED(IDC_tMX2EXP_CHECK_MARKUP, &CDlgTmx2Exp::OnBnClickedtmx2expCheckMarkup)
	ON_BN_CLICKED(IDC_TMX2EXP_CHECK_OUTMODE, &CDlgTmx2Exp::OnBnClickedTmx2expCheckOutmode)
	ON_BN_CLICKED(IDC_TMX2EXP_CHECK_OUTMEM, &CDlgTmx2Exp::OnBnClickedTmx2expCheckOutmem)
	ON_BN_CLICKED(IDC_TMX2EXP_BUTTON_RUN, &CDlgTmx2Exp::OnBnClickedTmx2expButtonRun)
	ON_BN_CLICKED(IDC_TMX2EXP_BUTTON_INMEM, &CDlgTmx2Exp::OnBnClickedTmx2expButtonInmem)
END_MESSAGE_MAP()



BOOL CDlgTmx2Exp::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    m_combo_outmode.InsertString(0,"UTF16");
    m_combo_outmode.InsertString(1,"ASCII");
    m_combo_outmode.InsertString(2,"ANSI");

	m_edit_outmem.EnableWindow(m_check_outmem.GetCheck());
	m_combo_outmode.EnableWindow(m_check_outmode.GetCheck());
	m_edit_markup.EnableWindow(m_check_markup.GetCheck());


	//CRect rc;
    //GetClientRect(&rc);

	//CRect parRect;
	//CTabCtrl* pTab = ((CDlgToolLauncher*)this->GetParent())->getTabCtrl();
	//pTab->GetClientRect(&parRect);

	// TODO: change tracks size 
	//this->SetScrollRange(SB_VERT,0,(rc.bottom-parRect.bottom-40));

	return TRUE;  
}


//void CDlgTmx2Exp::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//{
//	SCROLLINFO scrollinfo;  
//    GetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
//
//    switch (nSBCode)  
//    {  
//    case SB_BOTTOM:  
//        ScrollWindow(0,(scrollinfo.nPos-scrollinfo.nMax)*10);  
//        scrollinfo.nPos = scrollinfo.nMax;  
//        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
//        break;  
//
//    case SB_TOP:  
//        ScrollWindow(0,(scrollinfo.nPos-scrollinfo.nMin)*10);  
//        scrollinfo.nPos = scrollinfo.nMin;  
//        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
//        break;  
//
//    case SB_LINEUP:  
//        scrollinfo.nPos -= 1;  
//        if(scrollinfo.nPos < scrollinfo.nMin)
//        {  
//            scrollinfo.nPos = scrollinfo.nMin;  
//            break;  
//        }  
//        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
//        ScrollWindow(0,10);  
//        break;
//
//    case SB_LINEDOWN:  
//        scrollinfo.nPos += 1;  
//        if (scrollinfo.nPos>scrollinfo.nMax)  
//        {  
//            scrollinfo.nPos = scrollinfo.nMax;  
//            break;  
//        }  
//        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
//        ScrollWindow(0,-10);  
//        break;  
//
//    case SB_PAGEUP:  
//        scrollinfo.nPos -= 5;  
//        if (scrollinfo.nPos < scrollinfo.nMin)
//        {  
//            scrollinfo.nPos = scrollinfo.nMin;  
//            break;  
//        }  
//        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
//        ScrollWindow(0,10*5);  
//        break;  
//
//    case SB_PAGEDOWN:  
//        scrollinfo.nPos += 5;  
//        if (scrollinfo.nPos>scrollinfo.nMax)  
//        {  
//            scrollinfo.nPos = scrollinfo.nMax;  
//            break;  
//        }  
//        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
//        ScrollWindow(0,-10*5);  
//        break;  
//
//    case SB_ENDSCROLL:      
//    case SB_THUMBPOSITION:  
//        break;  
//
//    case SB_THUMBTRACK:  
//        ScrollWindow(0,(scrollinfo.nPos-nPos)*10);  
//        scrollinfo.nPos = nPos;  
//        SetScrollInfo(SB_VERT,&scrollinfo,SIF_ALL);  
//        break;  
//    }
//	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
//}



void CDlgTmx2Exp::OnBnClickedTmx2expCheckSs()
{
	// TODO: Add your control notification handler code here
}


void CDlgTmx2Exp::OnBnClickedTmx2expCheckIncludebrace()
{
	// TODO: Add your control notification handler code here
}


void CDlgTmx2Exp::OnBnClickedTmx2expCheckCleantf()
{
	// TODO: Add your control notification handler code here
}


void CDlgTmx2Exp::OnBnClickedtmx2expCheckMarkup()
{
	// TODO: Add your control notification handler code here
	m_edit_markup.EnableWindow(m_check_markup.GetCheck());
}


void CDlgTmx2Exp::OnBnClickedTmx2expCheckOutmode()
{
	// TODO: Add your control notification handler code here
	m_combo_outmode.EnableWindow(m_check_outmode.GetCheck());
}


void CDlgTmx2Exp::OnBnClickedTmx2expCheckOutmem()
{
	// TODO: Add your control notification handler code here
	m_edit_outmem.EnableWindow(m_check_outmem.GetCheck());
}


void CDlgTmx2Exp::OnBnClickedTmx2expButtonRun()
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
        MessageBox("please input /OUTPUTMEM parameter");
        return;
    }
	szBuf[outmemcnt]='\0';
    std::string outmem(szBuf);

	int outmodecnt = m_combo_outmode.GetCurSel();
    if(m_check_outmode.GetCheck() && outmodecnt==-1)
    {
        MessageBox("please input /OUTMODE parameter");
        return;
    }


	int markupcnt = m_edit_markup.GetLine(0,szBuf,512);
    if(m_check_markup.GetCheck() && markupcnt==0)
    {
        MessageBox("please input /Markup parameter");
        return;
    }
	szBuf[markupcnt]='\0';
	std::string markup(szBuf);

	std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
    {
        MessageBox("Cant' get OpenTM2 installed path");
        return;
    }   
    std::string opentm2path(szBuf);

    std::string cmd(opentm2path);
    cmd += "\\Win\\OtmTmx2Exp.exe ";
    cmd +=" /INPUTMEM="+inmem; 

	if(outmemcnt!=0 && m_check_outmem.GetCheck())
        cmd += " /OUTPUTMEM="+outmem;

	static std::string OUTMODE[3] = {"UTF16","ASCII","ANSI"};
    if(m_check_outmode.GetCheck())
        cmd+=" /OUTMODE="+OUTMODE[m_combo_outmode.GetCurSel()];

	if(markupcnt!=0 && m_check_markup.GetCheck())
        cmd += " /MArkup="+markup;

	if(m_check_clearntf.GetCheck())
		cmd+= " /CLEANRTF";

	if(m_check_includebrace.GetCheck())
		cmd += " /INCURLYBRACE";

	if(m_check_ss.GetCheck())
		cmd+= " /SOURCESOURCE";

    // execute command and fill log
    CRect mainRect;
    this->GetDlgItem(IDC_LIST_LOG)->GetWindowRect(&mainRect);
    Commons::runAndFillLog(mainRect,this,IDC_LIST_LOG,cmd,opentm2path);


}


void CDlgTmx2Exp::OnBnClickedTmx2expButtonInmem()
{
    CString res = Commons::searchDialog(_T("TMX MEM Files (*.TMX)|*.TMX|All Files (*.*)|*.*||"));
    if(!res.IsEmpty())
    {
        m_edit_inmem.SetWindowText( res );
    }
}


void CDlgTmx2Exp::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}
