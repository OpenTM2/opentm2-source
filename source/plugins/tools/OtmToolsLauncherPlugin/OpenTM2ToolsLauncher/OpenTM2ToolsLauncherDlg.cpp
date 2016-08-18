/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "stdafx.h"
#include "OpenTM2ToolsLauncher.h"
#include "OpenTM2ToolsLauncherDlg.h"
#include "afxdialogex.h"
#include <fstream>
#include "Commons.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// COpenTM2ToolsLauncherDlg dialog




COpenTM2ToolsLauncherDlg::COpenTM2ToolsLauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(COpenTM2ToolsLauncherDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void COpenTM2ToolsLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_TOOLLAUNCHER, m_Tab);
}

BEGIN_MESSAGE_MAP(COpenTM2ToolsLauncherDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDHELP, &COpenTM2ToolsLauncherDlg::OnBnClickedHelp)
    ON_BN_CLICKED(IDEXIT, &COpenTM2ToolsLauncherDlg::OnBnClickedExit)
END_MESSAGE_MAP()


// COpenTM2ToolsLauncherDlg message handlers

BOOL COpenTM2ToolsLauncherDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
  
    // read from history to fill selected and create tab
    // Add first tab page 
    mToolLauncher.Create(IDD_DLG_TOOLLAUNCHER, this);
    m_Tab.AddPage(_T("Launcher"),&mToolLauncher);
    mToolLauncher.doInitial();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void COpenTM2ToolsLauncherDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COpenTM2ToolsLauncherDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
        m_Tab.RedrawWindow();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR COpenTM2ToolsLauncherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void COpenTM2ToolsLauncherDlg::saveHistory()
{
     char szBuf[512+1] = {'\0'};
     std::string otmpath = Commons::getOpenTM2InstallPath(szBuf,512);
     std::string hist = otmpath+ "\\property\\selected_exes.history";
     std::ofstream fout(hist);
     for(int i=0; i<m_Tab.m_tabtitles.size(); i++)
     {
         if(m_Tab.m_tabtitles[i] != std::string("Launcher"))
             fout<<m_Tab.m_tabtitles[i]<<std::endl;
     }
     fout.close();
}

void COpenTM2ToolsLauncherDlg::OnClose()
{
     saveHistory();
     CDialogEx::OnClose();
}


void COpenTM2ToolsLauncherDlg::OnBnClickedHelp()
{
    MessageBox("Not implemented yet.");
}


void COpenTM2ToolsLauncherDlg::OnBnClickedExit()
{
    saveHistory();
    this->DestroyWindow();
}
