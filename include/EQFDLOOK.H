//+----------------------------------------------------------------------------+
//|  EQFDLOOK.H  - Internal header file for dictionary lookup dialog           |
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
// $Revision: 1.2 $ ----------- 19 Oct 2001
// --RJ: unicode enabling in LOOKIDA
// 
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.3 $ ----------- 10 Jan 2000
// - enlarged term buffer of lookup dialog IDA
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFDLOOK.HV_   1.1   10 Jan 1996 19:30:04   BUILD  $
 *
 * $Log:   J:\DATA\EQFDLOOK.HV_  $
 *
 *    Rev 1.1   10 Jan 1996 19:30:04   BUILD
 * - enhanced wildcard/compound search
 *
 *    Rev 1.0   09 Jan 1996 09:49:28   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define DLUP_TERMBUF_SIZE 32000  /* size of term buffer */
#define MAX_TERMS         60
// max terms for wildcard search
#define MAX_TERMS_WILD   300

typedef enum _LOOKUP_TYPES
{
   LOOK_HW,                            // lookup headword
   LOOK_NEIGHB,                        // lookup neighborhood
   LOOK_COMP_WORD,                     // lookup compound words
   LOOK_SYNONYMS,                      // lookup synonyms
   LOOK_REL_TERM,                      // lookup related terms
   LOOK_ABBREV,                        // lookup abbreviations
   LOOK_HIDD_TERM,                     // lookup hidden terms
   LOOK_SEQUENTIAL,                    // lookup terms in sequence
   END_LOOK
} LOOKUP_TYPES;

typedef struct _LOOKIDA
{
   PLUPCB     pLUPCB;                  // ptr to lookup control block
   HWND       hwnd;
   HWND       hwndFocus;               // window which should receive the focus
   CHAR_W     ucbTerm[MAX_TERM_LEN + 1];     // term being looked-up
   USHORT     usLookType;
   PBYTE      pbThreadStack;
   USHORT     usLookAction;
   HMODULE    hmod;
   CHAR_W     ucbTermBuf[4096];        // buffer for terms and dictionary names
   BOOL       fLookupDlgShown;         // lookup dialog shown flag
   HWND       hwndPrevFocus;           // window which had the focus before
                                       // the lookup dialog showed up
   BOOL       fCalledWithTerm;         // TRUE if dialog was called wit a term
                                       // for lookup (usually this is the case
                                       // for lookup in the translation
                                       // environment)
   BOOL       fRegistered;             // TRUE if dialog has been registered
                                       // using EqfRegisterObject
   OBJNAME    szObjName;               // name used for EqfRegisterObject
   ULONG      ulTermNum;               // term number returned by Asd calls
   ULONG      ulDataLen;               // data length returned by Asd calls
   HFILTER    hFilter;                 // handle of open filters
   CHAR       szFilter[MAX_FNAME];     // buffer for filter name
   PSZ_W      pucData;                 // buffer for term data
   USHORT     usDataSize;              // size of term data area in numb of char_w's
   BOOL       fDisabled;               // dlg is disabled flag
   PSZ_W      pucTermListBuffer;       // buffer for term lists
   USHORT     usLBItems;               // max. number of visible items in listbox
   BOOL       fChangedBySystem;        // entry-field-is-changed-by-system flag
   BOOL       fAtTop;                  // at-top-of-terms flag
   BOOL       fAtBottom;               // at_bottom-of-terms flag
   LONG       lUpdTime;                // last update time of dictionary
   BOOL           fWaitCursor;                     // TRUE = wait cursor is shown
} LOOKIDA, *PLOOKIDA;
