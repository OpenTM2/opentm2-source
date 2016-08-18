//+----------------------------------------------------------------------------+
//|  EQFUTCLB.H - Private header file for the Column Listbox Control (CLB)     |
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
// $Revision: 1.3 $ ----------- 26 Feb 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
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
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.3 $ ----------- 22 May 2000
// - worked on sort items function
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFUTCLB.HV_   1.1   12 Nov 1997 13:37:00   BUILD  $
 *
 * $Log:   J:\DATA\EQFUTCLB.HV_  $
 *
 *    Rev 1.1   12 Nov 1997 13:37:00   BUILD
 * - added protptype for function CLBPrintList
 *
 *    Rev 1.0   09 Jan 1996 09:48:28   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define CLB_EXTRABYTES  16             // number of extra bytes in CLB control
#define USER_OFFSET        0           // offset of user ptr in extra bytes
#define IDA_OFFSET         4           // offset of IDA ptr in extra bytes
#define CLBWORKSIZE     4096           // size of internal work area(s)
#define CLB_START_COMMENT  '['         // start of column header comment
#define CLB_END_COMMENT    ']'         // end of column header comment
#define CLB_WIDTH_FACTOR  1.0          // factor used for computation of
                                       //   character widths
#define LISTBOX_SUPER_CLASS "EqfListbox" // class name of listbox super class

typedef struct _CLBIDA
{
   PSZ      pszTitle;                  // ptr to title string
   HWND     hwndCLB;                   // handle of our own window
   HWND     hwndLB;                    // handle of listbox window
   HWND     hwndOwner;                 // handle of owner window
   PCLBCTLDATA pCtlData;               // column listbox control data
   USHORT   usViewListType;            // type of active view list
   //--- area for default font characteristics ---
   LONG     lCharHeight;               // heigth of character in default font
   LONG     lLineHeight;               // heigth of a line in default font
   LONG     lAveCharWidth;             // average width of character in deffont
   LONG     lMaxCharWidth;             // max width of character in default font
   //--- area for work pointers and work data areas
   PSHORT   psCurrentViewList;         // currently active view list
   CHAR     szWork[CLBWORKSIZE];       // general work area
   PSZ      *pDataPtr;                 // ptr to column data pointer table
   USHORT   usViewListSize;            // computed max size of a view list
   ULONG    ulOldWidth;                // old display width of listbox items
   ULONG    ulDisplayWidth;            // display width of listbox items
   LONG     lCurrentXLeft;             // current start position of visible area
   CHAR     szBuffer[512];             // general purpose buffer
   //--- area(s) for listbox items ---
   USHORT   usNoOfItems;               // no of items displayed in listbox
   USHORT   usTotalNoOfItems;          // total number of items in table
   USHORT   usItemTableSize;           // allocated size of item table
   PSZ      *pItemTable;               // table containing item text strings
   LONG     lNumberFonts;              // number of found fonts
   LOGFONT  lf;                      // logical font structure
 } CLBIDA, *PCLBIDA;

MRESULT EXPENTRY CLBWindowProc ( HWND, WINMSG, WPARAM, LPARAM );
MRESULT CLBCreate( HWND hwnd, PCLBCTLDATA pCtlData, LPARAM mp2 );
VOID    CLBDrawTitle( HDC hdc, PRECT prcl, PCLBIDA pIda );
VOID    CLBDrawItem( HDC hdc, PRECT prcl, PCLBIDA pIda, SHORT sItem );

INT_PTR CALLBACK SelectViewDlg ( HWND, WINMSG, WPARAM, LPARAM );
VOID    CLBCopyViewList( PSHORT psTarget, PSHORT psSource, USHORT usMaxItems );
ULONG   CLBQueryItemWidth( PCLBIDA pIda );
BOOL    CLBActivateViewList( PCLBIDA, PSHORT, USHORT );
BOOL    SetFixedFont( HWND hwnd, PCLBIDA pIda);
VOID    SelectViewDlgControl ( HWND hwnd, SHORT  sId, SHORT  sNotification );

BOOL CLBPrintList( PCLBIDA );
