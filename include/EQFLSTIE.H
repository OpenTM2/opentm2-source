//+----------------------------------------------------------------------------+
//|EQFLSTIE.H                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:   G. Queck (QSoft)                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Private header file of the list export/import functions        |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
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
// $Revision: 1.3 $ ----------- 14 Feb 2001
// - added format field to export and import IDA
// 
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFLSTIE.HV_   1.0   09 Jan 1996 09:46:44   BUILD  $
 *
 * $Log:   J:\DATA\EQFLSTIE.HV_  $
 *
 *    Rev 1.0   09 Jan 1996 09:46:44   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

/**********************************************************************/
/* Definition of export dialog IDA                                    */
/**********************************************************************/
typedef struct _LSTEXPIDA
{
  USHORT usListType;                   // type of list beeing processed
  USHORT usFormatID;                   // ID of currently selected format RB
  CHAR   szListName[MAX_EQF_PATH];     // buffer for fully qualified list name
  CHAR   szPropName[MAX_EQF_PATH];     // list handler properties name
  CHAR   szExportDrive[MAX_DRIVE];     // buffer for export drive
  CHAR   szExportPath[CCHMAXPATH];     // buffer for export path
  CHAR   szExportName[CCHMAXPATH];     // buffer for fully qualified export name
  CHAR   szString[1024];               // multi purose string buffer
  CONTROLSIDA Controls;                // IDA for dialog utilities
  BOOL   fInitInProgress;              // initialization-is-in-progress flag
  USHORT usFormat;                     // format of export (ANSI,ASCII,UNICODE,...)
} LSTEXPIDA, *PLSTEXPIDA;

/**********************************************************************/
/* Definition of import dialog IDA                                    */
/**********************************************************************/
typedef struct _LSTIMPIDA
{
  USHORT usListType;                   // type of list beeing processed
  USHORT usFormatID;                   // ID of currently selected format RB
  CHAR   szListType[MAX_EQF_PATH];     // list type system name
  CHAR   szListName[MAX_EQF_PATH];     // buffer for list name
  CHAR   szListPath[MAX_EQF_PATH];     // buffer for fully qualified list name
  CHAR   szPropName[MAX_EQF_PATH];     // list handler properties name
  CHAR   szImportDrive[MAX_DRIVE];     // buffer for import drive
  CHAR   szImportPath[CCHMAXPATH];     // buffer for import path
  CHAR   szImportName[CCHMAXPATH];     // buffer for fully qualified import name
  CHAR   szString[1024];               // multi purose string buffer
  CONTROLSIDA Controls;                // IDA for dialog utilities
  BOOL      fInitInProgress;           // initialization-is-in-progress flag
  USHORT usFormat;                     // format of import (ANSI,ASCII,UNICODE,...)
} LSTIMPIDA, *PLSTIMPIDA;

/**********************************************************************/
/* Prototypes of internal functions                                   */
/**********************************************************************/
static MRESULT LstExpInit( HWND hwnd, WPARAM mp1, LPARAM mp2 );
static MRESULT LstExpControl( HWND, SHORT, SHORT );
static MRESULT LstExpCommand( HWND, SHORT, SHORT );
static MRESULT LstExpClose( HWND hwnd, WPARAM mp1, LPARAM mp2 );
static MRESULT LstImpInit( HWND hwnd, WPARAM mp1, LPARAM mp2 );
static MRESULT LstImpCommand( HWND, SHORT, SHORT );
       USHORT  LstImportList( PSZ, USHORT, USHORT, PSZ, USHORT );
       USHORT  LstExportList( PSZ, USHORT, USHORT, PSZ, USHORT );
       MRESULT LstImpControl( HWND hwndDlg, SHORT sId, SHORT sNotification );
