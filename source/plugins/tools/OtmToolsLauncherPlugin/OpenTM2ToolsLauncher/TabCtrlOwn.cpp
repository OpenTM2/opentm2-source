/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "TabCtrlOwn.h"

BEGIN_MESSAGE_MAP(CTabCtrlOwn, CTabCtrl)
    ON_NOTIFY_REFLECT (TCN_SELCHANGE, OnSelChange)
    ON_WM_SIZE()
END_MESSAGE_MAP()

void CTabCtrlOwn::OnSelChange (NMHDR* pNMHDR, LRESULT* pResult) 
{
    int nIndex = GetCurSel ();
    if (nIndex == -1)
        return;

    for(std::size_t i=0; i<m_tabs.size(); i++)
    {
        if(i == nIndex)
            m_tabs[i]->ShowWindow(SW_SHOW);
        else
            m_tabs[i]->ShowWindow(SW_HIDE);
    }
	*pResult = 0;
}

int CTabCtrlOwn::AddPage (LPCTSTR pszTitle, CDialog* pDialog) 
{
    if(pDialog == NULL)
        return -1;

    TC_ITEM item;
    item.mask = TCIF_TEXT;
    item.pszText = (LPTSTR) pszTitle;
    int nIndex = GetItemCount();

    if (InsertItem (nIndex, &item) == -1)
         return -1;

    m_tabs.push_back(pDialog);
    m_tabtitles.push_back(std::string(pszTitle));

    pDialog->SetParent (this);

    CRect rect; 
    GetClientRect (&rect);
    if (rect.Width () > 0 && rect.Height () > 0)
        ResizeDialog (nIndex, rect.Width (), rect.Height ());

    rect.top +=22;
    rect.bottom -=1;
    rect.left +=1;
    rect.right -=1;

    pDialog->MoveWindow(&rect);

    if (nIndex == 0)
        pDialog->ShowWindow (SW_SHOW);

	return nIndex;
}

BOOL CTabCtrlOwn::DelPage(LPCTSTR pszTitle)
{
    if(pszTitle == NULL)
        return FALSE;
    int i;
    for(i=0; i<m_tabtitles.size(); i++)
    {
        if(m_tabtitles[i] == pszTitle )
            break;
    }
    if(i!=GetItemCount())
    {
        BOOL bSuc = DeleteItem(i);
        if(bSuc)
        {
            m_tabs[i]->DestroyWindow();
            m_tabs.erase(m_tabs.begin()+i);
            m_tabtitles.erase(m_tabtitles.begin()+i);
            if(i>0)
            {
                m_tabs[/*i-1*/0]->ShowWindow(SW_SHOW);
                SetCurSel(/*i-1*/0);
                this->RedrawWindow();
            }
        }
    }
    return FALSE;
}

CDialog* CTabCtrlOwn::GetDialogPage(int nIndex)
{
    if(nIndex<0 || nIndex>=m_tabs.size())
        return NULL;

     return m_tabs[nIndex];
}

void CTabCtrlOwn::ResizeDialog (int nIndex, int cx, int cy) 
{
    if (nIndex != -1) 
    {
        CDialog* pDialog = m_tabs[nIndex];

        if (pDialog != NULL)
        {
            CRect rect;
            GetItemRect (nIndex, &rect);

            int x, y, nWidth, nHeight;
            DWORD dwStyle = GetStyle ();

            int nTabHeight = rect.Height () * GetRowCount ();
            x = 4;
            y = (dwStyle & TCS_BOTTOM) ? 4 : nTabHeight + 4;
            nWidth = cx - 8;
            nHeight = cy - nTabHeight - 8;

            pDialog->SetWindowPos (NULL, x, y, nWidth, nHeight, SWP_NOZORDER);
        }
    }
}


void CTabCtrlOwn::OnSize(UINT nType, int cx, int cy)
{
    CTabCtrl::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here
    for(int i=0; i<m_tabs.size(); i++)
    {
        ResizeDialog(i,cx, cy);
    }
}

