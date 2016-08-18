//+----------------------------------------------------------------------------+
//|  EQFUTPRI.H - Private header file for general EQF utilities                |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
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
// $Revision: 1.2 $ ----------- 3 Sep 2008
// GQ: - used correct define for maximum number of TM DBs (MAX_NUM_OF_READONLY_MDB)
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
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
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
// $Revision: 1.2 $ ----------- 6 Feb 2002
// GQ: - added subfolder path fields to RENAMEDLGIDA
//
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.5 $ ----------- 19 Feb 2001
// -- PRINTDATA: add field fAnsiConv for OEM to Ansi conversion
//
//
// $Revision: 1.4 $ ----------- 7 Feb 2000
// - modifications for long name support in rename dialog
//
//
// $Revision: 1.3 $ ----------- 31 Jan 2000
// - modifications for long name support in rename dialog
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFUTPRI.HV_   1.5   22 Nov 1999 18:07:40   BUILD  $
 *
 * $Log:   K:\DATA\EQFUTPRI.HV_  $
 *
 *    Rev 1.5   22 Nov 1999 18:07:40   BUILD
 * - enlarged name fields in rename dialog IDA to support long names
 *
 *    Rev 1.4   12 Nov 1997 13:54:28   BUILD
 * - added some variables to rename dialog IDA
 *
 *    Rev 1.3   30 Sep 1997 10:31:30   BUILD
 * - finished object rename dialog
 *
 *    Rev 1.2   15 Sep 1997 10:43:18   BUILD
 * - A.57k: Added defines for the rename object dialog
 *
 *    Rev 1.1   10 Jan 1996 19:34:40   BUILD
 * - added stuff for yes-to-all dialog
 *
 *    Rev 1.0   09 Jan 1996 09:48:40   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

typedef struct _PRINTDATA
{
   HDC hdcPrinter;                     // printer device context
   HPS hpsPrinter;                     // printer presentation space
   SIZEL sizel;
   SHORT cxChar;                       // size of average char
   SHORT cyChar;
   USHORT usLine;                      // current output line
   USHORT usLineSpace;                 // vert space used for one line
   USHORT usLinesPerPage;              // number of lines which fit on one page
   FILE   *hFile;                      // handle for printing via files
   BOOL   fAnsiConv;                   // oem conversion on/off flag
} PRINTDATA, *PPRINTDATA;

typedef struct _YESTOALL_IDA
{
   PSZ        pszTitle;                // title for message box
   PSZ        pszText;                 // message box text
   SHORT      sID;                     // ID to use for message box
   USHORT     usStyle;                 // style flags for message box
} YESTOALL_IDA, *PYESTOALL_IDA;

INT_PTR CALLBACK UTLYESTOALLDLG
(
   HWND   hwnd,                        // handle of window
   WINMSG msg,                         // type of message
   WPARAM mp1,                         // first message parameter
   LPARAM mp2                          // second message parameter
);

VOID   UtlReplString( SHORT sID, PSZ pszString );

/**********************************************************************/
/* Private data for the rename object dialog                          */
/**********************************************************************/
INT_PTR CALLBACK UTLRENAMEDLG
(
   HWND   hwnd,                        // handle of window
   WINMSG msg,                         // type of message
   WPARAM mp1,                         // first message parameter
   LPARAM mp2                          // second message parameter
);

typedef struct _RENAMEDLGIDA
{
   OBJNAME    szObject;                // buffer for object name
   OBJNAME    szNewObject;             // buffer for new object name
   CLASSES    ObjectClass;             // class of object being renamed
   UTLCHECKOBJTYPES ObjectType;        // type of object being renamed
   SHORT      sID;                     // ID to use for dialog
   CHAR       szBuffer[8096];          // general purpose buffer
   CHAR       szFolObj[MAX_EQF_PATH];  // buffer for folder object name
   CHAR       szFolProp[MAX_EQF_PATH]; // buffer for folder property file name
   CHAR       szDocObj[MAX_EQF_PATH];  // buffer for document object name
   CHAR       szDocProp[MAX_EQF_PATH]; // buffer for document property file name
   CHAR       szOldName[MAX_LONGFILESPEC]; // buffer for old name
   CHAR       szOldShortName[MAX_FILESPEC]; // buffer for old short name
   CHAR       szNewName[MAX_LONGFILESPEC]; // buffer for new name
   CHAR       szNewShortName[MAX_FILESPEC]; // buffer for new short name
   CHAR       szOldPath[MAX_EQF_PATH]; // buffer for old path name
   CHAR       szNewPath[MAX_EQF_PATH]; // buffer for new path name
   PROPFOLDER stFolProp;               // buffer for folder properties
   PROPDOCUMENT stDocProp;             // buffer for document properties
   PROPDICTIONARY stDictProp;          // buffer for dictionary properties
   CHAR       MemTbl[MAX_NUM_OF_READONLY_MDB*MAX_FILESPEC];  // buffer for TM list
   CHAR       DicTbl[NUM_OF_FOLDER_DICS*MAX_FILESPEC];   // buffer for dict list
   char szLongMemory[MAX_LONGFILESPEC];// long name of folder's memory db
   char szLongName[MAX_LONGFILESPEC];  // buffer for long names
   CHAR       szSubFolSearchPath[MAX_EQF_PATH]; // subfolder search path
   CHAR       szSubFolObj[MAX_EQF_PATH];// subfolder object name
   CHAR       szPropDir[MAX_EQF_PATH]; // name of property directory
} RENAMEDLGIDA, *PRENAMEDLGIDA;


typedef struct _ADDUSERDLGIDA
{
   OBJNAME    szObject;                // buffer for object name
   OBJNAME    szUserObject;             // buffer for new object name
   SHORT      sID;                     // ID to use for dialog
   CHAR       szBuffer[8096];          // general purpose buffer
} ADDUSERDLGIDA, *PADDUSERDLGIDA;

INT_PTR CALLBACK UTLADDUSERDLG
(
   HWND   hwnd,                        // handle of window
   WINMSG msg,                         // type of message
   WPARAM mp1,                         // first message parameter
   LPARAM mp2                          // second message parameter
);

typedef struct _REMOVEUSERIDA
{
   OBJNAME    szObject;                // buffer for object name
   OBJNAME    szUserObject;                // buffer for object name
   SHORT      sID;                     // ID to be used for dialog
   HWND    hwndLB; 
   CHAR       szBuffer[512];              // buffer for various tasks
} REMOVEUSERIDA, *PREMOVEUSERIDA;

INT_PTR CALLBACK UTLREMOVEUSERDLG
(
   HWND   hwnd,                        // handle of window
   WINMSG msg,                         // type of message
   WPARAM mp1,                         // first message parameter
   LPARAM mp2                          // second message parameter
);


typedef struct _LISTUSERIDA
{
   OBJNAME    szObject;                // buffer for object name
   SHORT      sID;                     // ID to be used for dialog
   HWND    hwndLB;
   CHAR       szBuffer[512];           // buffer for various tasks
} LISTUSERIDA, *PLISTUSERIDA;

INT_PTR CALLBACK UTLLISTUSERDLG
(
   HWND   hwnd,                        // handle of window
   WINMSG msg,                         // type of message
   WPARAM mp1,                         // first message parameter
   LPARAM mp2                          // second message parameter
);
//  End of EQFUTPRI.H
