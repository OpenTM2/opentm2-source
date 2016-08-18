/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
// OpenTM2ToolsLauncher.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COpenTM2ToolsLauncherApp:
// See OpenTM2ToolsLauncher.cpp for the implementation of this class
//

class COpenTM2ToolsLauncherApp : public CWinApp
{
public:
	COpenTM2ToolsLauncherApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern COpenTM2ToolsLauncherApp theApp;