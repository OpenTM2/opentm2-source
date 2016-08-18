//+----------------------------------------------------------------------------+
//|  EQFPROGI.H - Header file for progress bar                                 |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
// 
// $CMVC 
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.1 $ ----------- 31 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
// 
// 
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
// 
// 
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
// 
// 
// 
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
// 
/*
 * $Header:   J:\DATA\EQFPROGI.HV_   1.1   14 Jan 1998 15:26:22   BUILD  $
 *
 * $Log:   J:\DATA\EQFPROGI.HV_  $
 * 
 *    Rev 1.1   14 Jan 1998 15:26:22   BUILD
 * - hide CtlType dialog prototypes under Win32
 * 
 *    Rev 1.0   09 Jan 1996 09:51:18   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
typedef struct tagPROGRESSBAR
    {
    DWORD       dwStyle;        //Control Styles
    WORD        iPos;           //Current Position
    WORD        iRange;         //Max Range
    COLORREF    rgb;
    } PROGRESSBAR;

typedef PROGRESSBAR  *PPROGRESSBAR;
typedef PROGRESSBAR FAR *LPPROGRESSBAR;

#define CBPROGRESSBAR sizeof(PROGRESSBAR)

// Offsets to use with GetWindowLong
#define GWL_PROGRESSBARHMEM    0

// Extra bytes for the window if the size of a local handle.
#define CBWINDOWEXTRA       sizeof(PVOID)
// Extra Class bytes.
#define CBCLASSEXTRA        0

LRESULT CALLBACK ProgressBarWndProc(HWND, WINMSG, WPARAM, LPARAM);

HANDLE  CALLBACK HCtlInfo(void);
WORD    CALLBACK WTranslateCtlFlags(DWORD, LPSTR, WORD);
#ifndef WIN32BIT
BOOL    CALLBACK FShowCtlStyleDlg(HWND, HANDLE, LPFNSTRTOID,
    LPFNIDTOSTR);
BOOL    CALLBACK FCtlStyleDlgProc(HWND, WORD, WORD, LONG);
#endif
