/*! EQFPROGR.H
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
	Module: EQFPROGR.H
	Purpose: Public include file for Progress Bar Custom Control.
*/

#define PROGRESSCONTROL "ProgressBar"

#ifndef PBS_VERTICAL         // are we using MFC??
  #define PBS_VERTICAL        0x0004L
#endif
#ifndef PBS_HORIZONTAL
  #define PBS_HORIZONTAL      0x0002L
#endif
#define PBS_PERCENTAGE      0x0010L
#define PBS_CHISELED        0x0008L

#define PB_SETRANGE         WM_USER+1
#define PB_SETPOS           WM_USER+2
#define PB_SETCOLOR         WM_USER+3

__declspec(dllexport)
BOOL RegisterProgressControl( HINSTANCE hInstance);
//BOOL    PASCAL   RegisterProgressControl( HAB hInstance);

