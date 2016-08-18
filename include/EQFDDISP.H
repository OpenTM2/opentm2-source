//+----------------------------------------------------------------------------+
//|  EQFDDISP.H  - Internal header file for dictionary display dialog          |
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
// $Revision: 1.3 $ ----------- 17 Mar 2003
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
// $Revision: 1.2 $ ----------- 19 Oct 2001
// --RJ: enable DISPIDA for Unicode
//
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.3 $ ----------- 10 Jan 2000
// - added long dictionary name support
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFDDISP.HV_   1.0   09 Jan 1996 09:45:30   BUILD  $
 *
 * $Log:   J:\DATA\EQFDDISP.HV_  $
 *
 *    Rev 1.0   09 Jan 1996 09:45:30   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define MAX_DISP_LEVEL   3
#define MIN_DISP_LEVEL   1
#define SPIN_YPOS_CHARS    1.8     // calculated from window top side
#define SPIN_WIDTH_CHARS   15
#define DICTNAME_POS_CHARS 15
#define DOUBLE_WIDTH_ONE_LINE 14  // code for printer (EPSON_CODE)

#define COURIER_8PTS       3
#define NORMAL_STYLE       0

#define MAX_TITLE_LEN  100    // maximum length of title in DLookup DLg

typedef struct _DISPLAY_INFO
{
   PUCHAR      pucCode;
   UCHAR       ucbDisplName [DICTENTRYLENGTH];
   USHORT      usDispLevel;
} DISPINFO, *PSTDISPINFO;

//
// structure containing info on dictionary where entry was found and
// the data of the entry.
//
typedef struct _FOUNDINFO
{
   HDCB             hDCB;              // dictionary handle
   CHAR             szDictName[MAX_LONGFILESPEC]; // dictionary name (w/o path+ext)
   PVOID            hLDBTree;          // LDB tree handle
} FOUNDINFO, *PFOUNDINFO;

typedef struct _DISPIDA
{
   HWND     hwnd;                      // handle of our dialog window
   PUCHAR   pucReply;
   PUCHAR   pucTerm;
   PUCHAR   pucaMLEText [ MAX_DISP_LEVEL + 1 ];
   PVOID    pvFirstTok;
   USHORT   usServHandle;
   USHORT   usData;
   USHORT   usaMLETextLen [ MAX_DISP_LEVEL + 1 ];
   USHORT   usLookAction;
   HMODULE  hmod;
   BOOL     fDestroy;
   BOOL     fEdit;
   USHORT   usCharHeight;
   USHORT   usCharWidth;
   USHORT   usTitleHeight;
   USHORT   usBorderSize;
   USHORT   usScreenXSize;
   USHORT   usScreenYSize;
   USHORT   fsStatus;
   HDCB     hEditHandle;               // handle of dictioanry containing entry
   PLUPCB   pLUPCB;                    // ptr to lookup control block
   CHAR_W   ucTerm[MAX_TERM_LEN + 1];     // term being looked-up
   CHAR_W   ucbTermBuf[MAX_TERM_LEN + 1]; // term buffer
   PUCHAR   pucSigRec;
   HWND     hwndCall;                  // handle of calling window
   CHAR_W   szUniBuffer[4096];            // multi purpose buffer
   USHORT   usAvailDicts;              // number of available dictionaries
   HDCB     ahDCBAvail[MAX_DICTS+1];   // array of available dictionary handles
   USHORT   usFoundInfos;              // number of entries in found info table
   FOUNDINFO FoundInfo[MAX_DICTS+1];   // array of info on found terms
   PSZ_W     pucMLEBuffer;              // ptr to buffer for MLE data
   USHORT    usMLEBufSize;              // current size of MLE buffer
   USHORT    usMLECharWidth;            // width of a character in the MLE
   BOOL      fRegistered;             // TRUE if dialog has been registered
                                       // using EqfRegisterObject
   OBJNAME    szObjName;               // name used for EqfRegisterObject
   PSZ_W      apszField[MAX_PROF_ENTRIES];// buffer for field data pointers
   SWP        swpButt[5];              // size and positions of pushbuttons
   CHAR       szPanelText[3][80];      // buffer for panel spinbutton text
   HFILTER    hFilter;                 // handle of open filters
   CHAR       szFilter[MAX_FNAME];     // buffer for filter name
   RECTL      rclDisp;                 // initial window size and position
   LONG       lTimeStamp;              // dialog creation time
   SHORT      MinX;                    // minimum X size of dialog
   SHORT      MinY;                    // minimum Y size of dialog
   CHAR       szDictNameTemp[MAX_LONGFILESPEC]; // temp buffer for dict names
   CHAR       szTemp[MAX_TITLE_LEN];     // temp buffer for ASCII strings
} DISPIDA, *PDISPIDA;





INT_PTR CALLBACK   DispEntryDlgProc       (HWND, WINMSG, WPARAM, LPARAM);
VOID               PrintMLE               (HWND, PDISPIDA );
VOID               MLEToClipbrd           (HWND, USHORT);
MRESULT EXPENTRY   FrameWinProc           (HWND, WINMSG, WPARAM, LPARAM);
VOID               CheckAndSetPos         (HWND, PDISPIDA);
USHORT             GetPointer             (HWND);
PSTDISPINFO        QueryDispInfo          (PSTDISPINFO, PUCHAR, USHORT);

//+----------------------------------------------------------------------------+
//| DicDspGetSelectedDict   - get ptr to info structure of selected dictionary |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Get current value of dictionary spinbutton and search dictionary in     |
//|    table of available dictionaries.                                        |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HWND      hwnd                IN      handle of display dialog window   |
//|    PDISPIDA  pIda                IN      ptr to display dialog IDA         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    PFOUNDINFO  pDictInfo       ptr to info structure of selected dictionary|
//|                                or NULL in case of errors                   |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
PFOUNDINFO DicDspGetSelectedDict
(
   HWND      hwnd,                     // handle of display dialog window
   PDISPIDA  pIda                      // ptr to display dialog IDA
);

//+----------------------------------------------------------------------------+
//| DicDspRefreshDictSpin   - refresh dictionaries displayed in dict spinbutton|
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Use list of FoundInfos to update dictionary names displayed in the      |
//|    table of available dictionaries.                                        |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HWND      hwnd                IN      handle of display dialog window   |
//|    PDISPIDA  pIda                IN      ptr to display dialog IDA         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    Nothing                                                                 |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
VOID DicDspRefreshDictSpin
(
   HWND      hwnd,                     // handle of display dialog window
   PDISPIDA  pIda                      // ptr to display dialog IDA
);

//+----------------------------------------------------------------------------+
//| DicDspRefreshDictInfo   - refresh dictionary info table in IDA             |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    The dictionary info table in the IDA will be updated as follows:        |
//|    - if the selected term is found in a dictionary which is not in the     |
//|      the current FoundInfo table, a new dictionary info entry is added to  |
//|      the table.                                                            |
//|    - if the selected term is not found in a dictionary but there is a      |
//|      entry for this dictionary in the FoundInfo table, the entry will be   |
//|      removed.                                                              |
//|    - if the selected term is found in a dictionary and there is already    |
//|      a FoundInfo entry for this dictionary, the term data in the dictionary|
//|      info table entry will be replaced with the new value.                 |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PDISPIDA  pIda                IN      ptr to display dialog IDA         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    BOOL      fOK                 TRUE  = refresh successful                |
//|                                  FALSE = refresh failed (dictionary or     |
//|                                          memory allocation error)          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    FoundInfo table in IDA is updated.                                      |
//+----------------------------------------------------------------------------+
BOOL DicDspRefreshDictInfo
(
   PDISPIDA  pIda                      // ptr to display dialog IDA
);


//+----------------------------------------------------------------------------+
//| DicDspFillMLE           - Fills the entry MLE with data of dictionry data  |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Fills the entry MLE with the fields of the dictionary entry of the      |
//|    currently selected dictionary and the currently selected display        |
//|    level.                                                                  |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    HWND      hwnd                IN      handle of display dialog window   |
//|    PDISPIDA  pIda                IN      ptr to display dialog IDA         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    BOOL     fOK                          function successful flag          |
//|                                          TRUE  = MLE has been filled       |
//|                                          FALSE = error while filling MLE   |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
BOOL DicDspFillMLE
(
   HWND      hwnd,                     // handle of display dialog window
   PDISPIDA  pIda                      // ptr to display dialog IDA
);

BOOL GetRelativeWindowPos( HWND hwndControl, SWP FAR *pSwp );

MRESULT DicDspControl( HWND, SHORT, SHORT );
