//+----------------------------------------------------------------------------+
//|EQFWCNT.H                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:      R.Jornitz                                                      |
//+----------------------------------------------------------------------------+
//|Description: Public include file of word count handler                      |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+

__declspec(dllexport) INT_PTR CALLBACK  WORDCOUNTDLGPROC( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport) MRESULT WCntListCallBack( PLISTCOMMAREA, HWND, WINMSG, WPARAM, LPARAM);
