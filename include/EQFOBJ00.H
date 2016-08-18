//+----------------------------------------------------------------------------+
//|  EQFOBJ00.H - Internal header file for the EQF Object Manager              |
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
 * $Header:   K:\DATA\EQFOBJ00.HV_   1.1   08 Nov 1999 13:56:10   BUILD  $
 *
 * $Log:   K:\DATA\EQFOBJ00.HV_  $
 * 
 *    Rev 1.1   08 Nov 1999 13:56:10   BUILD
 * - changed type of message paramater for function SendAll to WINMSG
 *
 *    Rev 1.0   09 Jan 1996 09:46:58   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#ifndef INCL_EQFOBJ00
#define INCL_EQFOBJ00

#define HANDLERENTRIES  50
#define OBJECTENTRIES  300
#define SYMBOLENTRIES  600


/* Object Table Entry */
struct objm_tblentry {
     EQF_ATOM     aName;                /* objects name as atom              */
     HWND         hWnd;                 /* objects window handle             */
     USHORT       usClassID;            /* objects class id                  */
     USHORT       fStatus;              /* objects status flags              */
};
typedef struct objm_tblentry OBJENTRY, *POBJENTRY;


/* Object Manager Object Table */
typedef struct _OBJTBL {
     PLUG         plug;                 /* Chaining convention               */
     USHORT       usMax;                /* Maximum entries                   */
     USHORT       usUsed;               /* Used entries                      */
     POBJENTRY    pObjEntry;            /* 1st entry in the table of objects */
} OBJTBL, *POBJTBL;


/* Object Manager Instance Area */
typedef struct _OBJM_IDA {
     IDA_HEAD     IdaHead;              /* Common header of instance area    */
     HATOMTBL     hAtomTbl;             /* EQF atom table for objects        */
     HATOMTBL     hAtomSymTbl;          /* EQF atom table for symbols        */
     POBJTBL      pHndlrTbl;            /* Table of active EQF handlers      */
     POBJTBL      pObjTbl;              /* Table of active EQF objects       */
     POBJTBL      pSymbolTbl;           /* Table of Symbols for APPL dep. Use*/
} OBJM_IDA, *POBJM_IDA;

typedef struct _QUERYOBJ
{
  SHORT            sClass;
  SHORT            sFlags;
  PSZ              pBuffer;
} QUERYOBJ, *PQUERYOBJ;

typedef struct _QUERYOBJNAME
{
  HWND             hwnd;
  PSZ              pBuffer;
} QUERYOBJNAME, *PQUERYOBJNAME;


typedef struct _SETOBJSTATUS
{
  HWND             hwnd;
  SHORT            sMask;
  SHORT            sStatus;
} SETOBJSTATUS, *PSETOBJSTATUS;

typedef struct _GETOBJLIST
{
  SHORT        sClass;
  SHORT        sCount;
  PVOID        pBuffer;
} GETOBJLIST, *PGETOBJLIST;

POBJENTRY MakeObjEntry( POBJTBL pt);
VOID      RemoveObjEntry( POBJTBL pt, EQF_ATOM at);
int       TerminateObjects( POBJM_IDA pIda, USHORT flg);
int       RemoveObject( POBJM_IDA pIda, POBJENTRY pe, USHORT flg);
int       TerminateHandlers( POBJM_IDA pIda, USHORT flg);
int       RemoveHandler( POBJM_IDA pIda, POBJENTRY pe, USHORT flg);
POBJENTRY FindObjByAtom( POBJTBL pt, EQF_ATOM atom);
POBJENTRY FindObjByHwnd( POBJTBL pt, HWND hwnd);
POBJENTRY FindObjByClass( POBJTBL pt, USHORT cls);
POBJENTRY FindObjByClassFlags( POBJTBL pt, USHORT cls, USHORT flgs);
POBJENTRY FindObjByAtomClass( POBJTBL pt, EQF_ATOM atom, USHORT cls);
USHORT    CountObjByClass( POBJTBL pt, USHORT cls);
USHORT    LoadObjList( POBJTBL pt, USHORT cls, POBJLST ptab, register int i);
USHORT    SendAll( POBJTBL pt, CLASSES objClass, WINMSG message, \
                   WPARAM mp1, LPARAM mp2);

#endif
