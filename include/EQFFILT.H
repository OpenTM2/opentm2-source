//+----------------------------------------------------------------------------+
//|EQFFILT.H                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:         G. Queck (QSoft)                                            |
//+----------------------------------------------------------------------------+
//|Description:    Include file for filter functions                           |
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
// $Revision: 1.2 $ ----------- 17 Mar 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
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
 * $Header:   J:\DATA\EQFFILT.HV_   1.1   14 Jan 1998 16:18:50   BUILD  $
 *
 * $Log:   J:\DATA\EQFFILT.HV_  $
 *
 *    Rev 1.1   14 Jan 1998 16:18:50   BUILD
 * - migration to Win32 environment
 *
 *    Rev 1.0   09 Jan 1996 09:45:58   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

//filter 'handle' (a pointer to the filter control block)
typedef PVOID HFILTER;

VOID DictionaryFilter( HDCB hDCB, PSZ pFiltName, HWND hwndDlg );

USHORT FiltOpen
(
  PSZ      pszFiltName,                // Name of filter (w/o path info)
  HDCB     hDCB,                       // handle of dictionary being worked on
  HFILTER *phFilter                    // address of the filter handle buffer
);

USHORT FiltClose
(
  HFILTER hFilter                        // handle of filter
);


USHORT FiltWork
(
  HFILTER      hFilter,                // filter handler
  PVOID        hLDBTree,               // handle of LDB tree
  PVOID        *phNewLDBTree           // handle of result tree or NULL if no
                                       // result tree is required
);

USHORT FiltWork2
(
  HFILTER      hFilter,                // filter handler
  PVOID        hLDBTree,               // handle of LDB tree
  PVOID        *phNewLDBTree,          // handle of result tree or NULL if no
                                       // result tree is required
  HDCB         hDCB                    // handle of dictionary containing entry
);

USHORT FiltCheckFields
(
  HFILTER      hFilter,                // filter handler
  PPROPDICTIONARY pDictProp,           // properties of dictionary
  BOOL        fMsg                     // show-message-box flag
);

PSZ FiltCheckSelFields
(
  HFILTER      hFilter,                // filter handle
  PSZ          pszFields               // ptr to field list
);

BOOL IsFilterMessage( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );
