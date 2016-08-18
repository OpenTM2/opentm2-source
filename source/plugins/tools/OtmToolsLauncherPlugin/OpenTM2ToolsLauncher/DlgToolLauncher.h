/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once

#include <vector>
#include <string>
#include "DlgAdl.h"
#include "TabCtrlOwn.h"
#include "DlgExp2Tmx.h"
#include "DlgChangeFxp.h"
#include "DlgChkCalc.h"
#include "DlgItmFm.h"
#include "DlgMtEval.h"
#include "DlgTmxSplitSeg.h"
#include "DlgRemoveTags.h"
#include "DlgShowFxp.h"
#include "DlgTmx2Exp.h"
#include "DlgTmx2Text.h"
#include "DlgXliff2Exp.h"
#include "DlgMigrator.h"
#include "DlgGetToolInfo.h"
#include "DlgGetReportData.h"
#include "DlgMemoryTool.h"

// CDlgToolLauncher dialog
class CDlgToolLauncher : public CDialog
{
	DECLARE_DYNAMIC(CDlgToolLauncher)
private:
    std::vector<std::string> mSelcted;
public:
	CDlgToolLauncher(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgToolLauncher();

    void addPage(std::string title);
    void delPage(std::string title);
    void doInitial();
// Dialog Data
	enum { IDD = IDD_DLG_TOOLLAUNCHER };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
    CListBox m_ListTotalExes;
    CListBox m_ListSelectedExes;
    afx_msg void OnBnClickedButtonSelect();
    afx_msg void OnBnClickedButtonUnselect();
    afx_msg void OnBnClickedButtonSelecall();
    afx_msg void OnBnClickedButtonRemoveall();
    afx_msg void OnBnClickedButtonOpentm2scriptergui();
    afx_msg void OnBnClickedButtonotmitm();
    CButton m_button_opentm2scripter;
    CButton m_button_remove_selected;
    CButton m_button_removeall;
    CButton m_button_selectall;
    CButton m_button_select;
    CButton m_button_unselectall;
    CButton m_button_unselected;
    CButton m_button_itm;

    //
    CDlgAdl       m_Adl;
    CDlgExp2Tmx   m_Exp2Tmx;
    CDlgChangeFxp m_ChangeFxp;
    CDlgChkCalc   m_Chk;
    CDlgItmFm     m_ItmFm;
    CDlgMtEval    m_MtEval;
    CDlgTmxSplitSeg    m_TmxSplitSeg;
	CDlgRemoveTags     m_RemoveTags;
	CDlgShowFxp     m_ShowFxp;
	CDlgTmx2Exp     m_Tmx2Exp;
	CDlgTmx2Text   m_Tmx2Text;
	CDlgXliff2Exp  m_Xliff2Exp;
	CDlgMigrator   m_Migrator;
	CDlgGetToolInfo m_GetToolInfo;
	CDlgGetReportData m_GetRpd;
	CDlgMemoryTool    m_MemTool;

private:
    
    void addTools();
    void removeTools();
public:
	CTabCtrlOwn* getTabCtrl(void);
    afx_msg void OnDblclkListTotal();
    afx_msg void OnDblclkListSelected();
};
