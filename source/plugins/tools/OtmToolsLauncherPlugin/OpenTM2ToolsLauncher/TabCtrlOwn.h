/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#pragma once


#include <afxtempl.h>
#include <vector>
#include <string>
class CTabCtrlOwn : public CTabCtrl 
{
public:
    int  AddPage (LPCTSTR pszTitle, CDialog* pDialog);
    BOOL DelPage (LPCTSTR pszTitle);
    CDialog* GetDialogPage(int nIndex);
    void ResizeDialog (int nIndex, int cx, int cy) ;

    std::vector<std::string> m_tabtitles;
    std::vector<CDialog*> m_tabs;

protected:
    afx_msg void OnSelChange (NMHDR* pNMHDR, LRESULT* pResult);
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
};


