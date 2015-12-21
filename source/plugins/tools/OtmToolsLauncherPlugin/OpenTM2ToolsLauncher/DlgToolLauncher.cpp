/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "DlgToolLauncher.h"
#include "afxdialogex.h"
#include <string>
#include <fstream>
#include "TabCtrlOwn.h"
#include "Commons.h"

// CDlgToolLauncher dialog
static const int gLength = 16;
char*  gExes[gLength] = {
"OtmAdl",
"OtmChangeFxp",
"OtmChkCalc",
"OtmExp2Tmx",
"OtmCreateITMFromMemory",
"OtmMtEval",
"OtmTmxSplitSegments",
"OtmRemoveTags",
"OtmShowFxp",
"OtmTmx2Exp",
"OtmTmxSource2Text",
"OtmXliff2Exp",
"TM2OTMMigrator",
"OtmGetToolInfo",
"OtmGetReportData",
"OtmMemoryTool"
};


IMPLEMENT_DYNAMIC(CDlgToolLauncher, CDialog)

CDlgToolLauncher::CDlgToolLauncher(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgToolLauncher::IDD, pParent)
{

}

CDlgToolLauncher::~CDlgToolLauncher()
{
}

void CDlgToolLauncher::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_TOTAL, m_ListTotalExes);
    DDX_Control(pDX, IDC_LIST_SELECTED, m_ListSelectedExes);
    DDX_Control(pDX, IDC_BUTTON_OPENTM2SCRIPTERGUI, m_button_opentm2scripter);
    DDX_Control(pDX, IDC_BUTTON_REMOVEALL, m_button_removeall);
    DDX_Control(pDX, IDC_BUTTON_SELECALL, m_button_selectall);
    DDX_Control(pDX, IDC_BUTTON_SELECT, m_button_select);
    DDX_Control(pDX, IDC_BUTTON_UNSELECT, m_button_unselected);
    DDX_Control(pDX, IDC_BUTTONOTMITM, m_button_itm);
}


BEGIN_MESSAGE_MAP(CDlgToolLauncher, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_SELECT, &CDlgToolLauncher::OnBnClickedButtonSelect)
    ON_BN_CLICKED(IDC_BUTTON_UNSELECT, &CDlgToolLauncher::OnBnClickedButtonUnselect)
    ON_BN_CLICKED(IDC_BUTTON_SELECALL, &CDlgToolLauncher::OnBnClickedButtonSelecall)
    ON_BN_CLICKED(IDC_BUTTON_REMOVEALL, &CDlgToolLauncher::OnBnClickedButtonRemoveall)
    ON_BN_CLICKED(IDC_BUTTON_OPENTM2SCRIPTERGUI, &CDlgToolLauncher::OnBnClickedButtonOpentm2scriptergui)
    ON_BN_CLICKED(IDC_BUTTONOTMITM, &CDlgToolLauncher::OnBnClickedButtonotmitm)
    ON_LBN_DBLCLK(IDC_LIST_TOTAL, &CDlgToolLauncher::OnDblclkListTotal)
    ON_LBN_DBLCLK(IDC_LIST_SELECTED, &CDlgToolLauncher::OnDblclkListSelected)
END_MESSAGE_MAP()


// CDlgToolLauncher message handlers

CTabCtrlOwn* CDlgToolLauncher::getTabCtrl()
{
     return (CTabCtrlOwn*)this->GetParent();
}

void CDlgToolLauncher::doInitial()
{
    char szBuf[512+1] = {'\0'};
    std::string otmpath = Commons::getOpenTM2InstallPath(szBuf,512);
    std::string hist = otmpath+ "\\property\\selected_exes.history";
    std::ifstream fin(hist);
    std::string line;
    int idx = 0;
    while(getline(fin,line))
    {
        m_ListSelectedExes.InsertString(idx++,line.c_str());
        addPage(line);
    }
    fin.close();

    for(int i=0; i<gLength; i++)
    {
        // only not in selected then add it here
        if(m_ListSelectedExes.FindString(0,gExes[i])== LB_ERR)
            m_ListTotalExes.AddString(gExes[i]);
    }
}

void CDlgToolLauncher::addPage(std::string title)
{
    CTabCtrlOwn* pTab = getTabCtrl();
    if(pTab == NULL || title.empty())
        return;

    if(title==std::string("OtmAdl") )
    {
        m_Adl.Create(IDD_DLG_ADL, this);
        pTab->AddPage("OtmAdl",&m_Adl);
    }
    else if(title==std::string("OtmExp2Tmx") )
    {
        m_Exp2Tmx.Create(IDD_DLG_EXP2TMX, this);
        pTab->AddPage("OtmExp2Tmx",&m_Exp2Tmx);
    }
    else if(title==std::string("OtmChangeFxp") )
    {
        m_ChangeFxp.Create(IDD_DLG_CHANGEFXP, this);
        pTab->AddPage("OtmChangeFxp",&m_ChangeFxp);
    }
    else if(title==std::string("OtmChkCalc") )
    {
        m_Chk.Create(IDD_DLG_CHKCALC, this);
        pTab->AddPage("OtmChkCalc",&m_Chk);
    }
    else if(title==std::string("OtmCreateITMFromMemory") )
    {
        m_ItmFm.Create(IDD_DLG_CARETEITMFROMMEMORY, this);
        pTab->AddPage("OtmCreateITMFromMemory",&m_ItmFm);
    }
    else if(title==std::string("OtmMtEval") )
    {
        m_MtEval.Create(IDD_DLG_MTEVAL, this);
        pTab->AddPage("OtmMtEval",&m_MtEval);
    }
    else if(title==std::string("OtmTmxSplitSegments") )
    {
        m_TmxSplitSeg.Create(IDD_DLG_TMXSPLITSEGMENTS, this);
        pTab->AddPage("OtmTmxSplitSegments",&m_TmxSplitSeg);
    }
	else if(title==std::string("OtmRemoveTags") )
    {
        m_RemoveTags.Create(IDD_DLG_REMOVETAGS, this);
        pTab->AddPage("OtmRemoveTags",&m_RemoveTags);
    }
	else if(title==std::string("OtmShowFxp") )
    {
        m_ShowFxp.Create(IDD_DLG_SHOWFXP, this);
        pTab->AddPage("OtmShowFxp",&m_ShowFxp);
    }
	else if(title==std::string("OtmTmx2Exp") )
    {
        m_Tmx2Exp.Create(IDD_DLG_TMX2EXP, this);
        pTab->AddPage("OtmTmx2Exp",&m_Tmx2Exp);
    }
	else if(title==std::string("OtmTmxSource2Text") )
    {
        m_Tmx2Text.Create(IDD_DLG_TMX2TEXT, this);
        pTab->AddPage("OtmTmxSource2Text",&m_Tmx2Text);
    }
	else if(title==std::string("OtmXliff2Exp") )
    {
        m_Xliff2Exp.Create(IDD_DLG_XLIFF2EXP, this);
        pTab->AddPage("OtmXliff2Exp",&m_Xliff2Exp);
    }
	else if(title==std::string("TM2OTMMigrator") )
    {
        m_Migrator.Create(IDD_DLG_MIGRATOR, this);
        pTab->AddPage("TM2OTMMigrator",&m_Migrator);
    }
	else if(title==std::string("OtmGetToolInfo") )
    {
        m_GetToolInfo.Create(IDD_DLG_GETTOOLINFO, this);
        pTab->AddPage("OtmGetToolInfo",&m_GetToolInfo);
    }
	else if(title==std::string("OtmGetReportData") )
    {
        m_GetRpd.Create(IDD_DLG_GETREPORTDATA, this);
        pTab->AddPage("OtmGetReportData",&m_GetRpd);
    }
	else if(title==std::string("OtmMemoryTool") )
    {
        m_MemTool.Create(IDD_DLG_MEMTOOL, this);
        pTab->AddPage("OtmMemoryTool",&m_MemTool);
    }

}

void CDlgToolLauncher::delPage(std::string title)
{
    CTabCtrlOwn* pTab = getTabCtrl();
    if(pTab == NULL || title.empty())
        return;

    pTab->DelPage(title.c_str());
}

void CDlgToolLauncher::OnBnClickedButtonSelect()
{
    addTools();
}


void CDlgToolLauncher::OnBnClickedButtonUnselect()
{
    removeTools();
}


void CDlgToolLauncher::OnBnClickedButtonSelecall()
{
   
    int total = m_ListTotalExes.GetCount();
    for(int i=0; i<total; i++)
    {
        m_ListTotalExes.SetSel(i);
    }
}

void CDlgToolLauncher::OnBnClickedButtonRemoveall()
{
   
    int total = m_ListSelectedExes.GetCount();
    for(int i=0; i<total; i++)
    {
        m_ListSelectedExes.SetSel(i);
    }
}


void CDlgToolLauncher::OnBnClickedButtonOpentm2scriptergui()
{
    char szBuf[512+1]={'\0'};
    std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
        return;

    char szOldCwd[512+1] = {'\0'};
    GetCurrentDirectory(512,szOldCwd);
    std::string cwd;
    cwd += path;
    cwd += "\\OpenTM2ScripterGUI";
    SetCurrentDirectory(cwd.c_str());

    std::string cmd;
    cmd += "java -jar ";
    cmd += "OpenTM2ScripterGUI.jar";
    Commons::executeCommand(cmd,"");

    SetCurrentDirectory(szOldCwd);
}


void CDlgToolLauncher::OnBnClickedButtonotmitm()
{
    char szBuf[512+1]={'\0'};
    std::string path = Commons::getOpenTM2InstallPath(szBuf,512);
    if(path.empty())
        return;

    std::string itm;
    itm += path;
    itm += "\\win\\OtmItm.exe";
    Commons::executeCommand(itm,"");
}



void CDlgToolLauncher::OnDblclkListTotal()
{
    addTools();
}

void CDlgToolLauncher::OnDblclkListSelected()
{
    removeTools();
}

void CDlgToolLauncher::addTools()
{
    int tools[50];
    int selCnt =  m_ListTotalExes.GetSelItems( 50, tools );
    if(selCnt == LB_ERR)
    {
        MessageBox("Please select the tool(s) you want to add");
        return;
    }
    
    std::vector<CString> textVec;

    for(int i=0; i<selCnt; i++)
    {
        CString text;
        int selIdx = tools[i];
        m_ListTotalExes.GetText(selIdx, text);
        textVec.push_back(text);
        m_ListSelectedExes.InsertString(m_ListSelectedExes.GetCount(),text);
        //m_ListTotalExes.DeleteString(selIdx);
        addPage(text.GetString());
    }

    int i=0;
    while(i<m_ListTotalExes.GetCount())
    {
        CString text;
        m_ListTotalExes.GetText(i, text);
        
        int j=0;
        for(; j<textVec.size(); j++)
            if(text==textVec[j])
            {
                m_ListTotalExes.DeleteString(i);
                break;
            }

        if(j == textVec.size())
        {
            i++;
        }
    }

}

void CDlgToolLauncher::removeTools()
{

    int tools[50];
    int selCnt =  m_ListSelectedExes.GetSelItems( 50, tools );
    if(selCnt == LB_ERR)
    {
        MessageBox("Please select the tool(s) you want to unselect");
        return;
    }

    std::vector<CString> textVec;

    for(int i=0; i<selCnt; i++)
    {
        CString text;
        int selIdx = tools[i];
        m_ListSelectedExes.GetText(selIdx, text);
        textVec.push_back(text);
        m_ListTotalExes.InsertString(m_ListTotalExes.GetCount(),text);
        //m_ListSelectedExes.DeleteString(selIdx);
        delPage(text.GetString());
    }

    
    int i=0;
    while(i<m_ListSelectedExes.GetCount())
    {
        CString text;
        m_ListSelectedExes.GetText(i, text);
        
        int j=0;
        for(; j<textVec.size(); j++)
            if(text==textVec[j])
            {
                m_ListSelectedExes.DeleteString(i);
                break;
            }

        if(j == textVec.size())
        {
            i++;
        }
    }
}





