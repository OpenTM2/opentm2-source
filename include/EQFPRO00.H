//+----------------------------------------------------------------------------+
//|  EQFPRO00.H - Internal header file for the EQF Property Handler            |
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
// $Revision: 1.2 $ ----------- 11 Apr 2006
// GQ: - added fOverWrite flag to PROPMSGPARM structure
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
// $Revision: 1.2 $ ----------- 18 Mar 2004
// GQ: - added fOverwriteExisting flag to CreatePropFile function
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
 * $Header:   J:\DATA\EQFPRO00.HV_   1.0   09 Jan 1996 09:47:02   BUILD  $
 *
 * $Log:   J:\DATA\EQFPRO00.HV_  $
 * 
 *    Rev 1.0   09 Jan 1996 09:47:02   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#ifndef INCL_EQFPRO00
#define INCL_EQFPRO00

/* Handles per acquired block */
#define PROPHWND_ENTRIES      30

/* Properties Control Block */
struct prop_cntl {
     PLUG    Plug;                      // Foreward / backward chain
     PSZ     pszFname;                  // Properties file name (full path)
     ULONG   lFlgs;                     // processing flags: WRITE mode etc.
     USHORT  usFsize;                   // File Size
     USHORT  usUser;                    // properties use count
     PPROPHEAD pHead;                   // ptr tp properties heading data
};
typedef struct prop_cntl PROPCNTL, *PPROPCNTL;

/* Properties Handle */
struct prop_hnd {
     PLUG      Plug;                    // Foreward / backward chain
     PVOID     pHndID;                  // Identifier for handle
     ULONG     lFlgs;                   // Processing flags: WRITE mode etc.
     PPROPCNTL pCntl;                   // Properties control buffer
};
typedef struct prop_hnd PROPHND, far *PPROPHND;

/* PropertyHandler Instance Area */
struct prop_ida {
     IDA_HEAD     IdaHead;              // Common header of instance area
     PPROPHND     hSystem;              // Handle to EQF system properties
     PEQFINFO     pErrorInfo;           // Error bucket for error infos.

     PPLUG        ChainBlkHnd;          // Chain of blocks used for handles
     PPROPHND     TopUsedHnd;           // 1st used entry in table of hnds.
     PPROPHND     TopFreeHnd;           // 1st free entry in table of hdns.
     PPROPCNTL    TopCntl;              // Top entry of properties cntls
     PPROPCNTL    LastCntl;             // Last entry of properties cntls
};
typedef struct prop_ida PROP_IDA, far *PPROP_IDA;

/* Property Handler message structure */
typedef struct _PROPMSGPARM
{
     HPROP     hObject;
     PSZ       pszName;
     PSZ       pszPath;
     BYTE     *pBuffer;
     USHORT    fFlg;
     BYTE      tmpName[MAX_EQF_PATH];  // use when pszName must must be split
     PEQFINFO  pErrorInfo;             // supplied error info pointer
     BOOL      fOverWrite;             // TRUE = overwrite existing prop file when creating
} PROPMSGPARM, far *PPROPMSGPARM;


/* Export window proc */
MRESULT   APIENTRY PropertyHandlerWP( HWND, WINMSG, WPARAM, LPARAM);

PPROPHND  MakePropHnd( PPROP_IDA pIda);
SHORT     FreePropHnd( PPROP_IDA pIda, PPROPHND hprop);
PPROPHND  FindPropHnd( PPROPHND ptop, PPROPHND hprop);
PPROPCNTL LoadPropFile( PPROP_IDA pIda, PSZ pName, PSZ pPath, USHORT Acc);
PPROPCNTL CreatePropFile( PPROP_IDA pIda, PSZ pName, PSZ pPath, USHORT Class, BOOL fOverwriteExisting );
VOID      DropPropFile( PPROP_IDA pIda, PPROPCNTL pcntl);
SHORT     DeletePropFile( PPROP_IDA pIda, PSZ pszName, PSZ pszPath);
PPROPCNTL FindPropCntl( PPROPCNTL ptop, PSZ pszName, PSZ pszPath);
USHORT    GetSysProp( PPROP_IDA pIda);
USHORT    PutItAway( PPROPCNTL pcntl);
SHORT     LoadPropMsg( PPROPMSGPARM mp, PPROPHND hprop, PSZ name, PSZ path, \
                       USHORT flg, PEQFINFO pErrorInfo, BYTE *buffer);

#endif
