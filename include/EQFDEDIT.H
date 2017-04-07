//+----------------------------------------------------------------------------+
//|  EQFDEDIT.H  - Internal header file for dictionary edit dialog             |
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
// --RJ: Unicode enabling EDITIDA
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
 * $Header:   J:\DATA\EQFDEDIT.HV_   1.3   18 Mar 1996 16:27:16   BUILD  $
 *
 * $Log:   J:\DATA\EQFDEDIT.HV_  $
 *
 *    Rev 1.3   18 Mar 1996 16:27:16   BUILD
 * - added szUserID field to edit dialog IDA
 *
 *    Rev 1.1   04 Mar 1996 10:31:52   BUILD
 * - enlarged ucbTermBuf field of edit dialog IDA
 *
 *    Rev 1.0   09 Jan 1996 09:45:34   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define ASKUSER( usMsg ) \
   UtlError( usMsg, MB_YESNO | MB_WARNING | MB_DEFBUTTON2, 0, NULL, EQF_QUERY)

#define NOTIFYUSER( usMsg ) \
  UtlError( usMsg, MB_OK, 0, NULL, EQF_WARNING )

#define READ_ONLY         0x0020
#define BLANK ' '
#define STRUCT_DIFFERENT   0x1000
#define OTHER_DICT         0x0800
#define EF_POS_CHARS           18
// #define EF_WIDTH_CHARS      40
#define MLE_HEIGHT_CHARS        3
// #define SLE_HEIGHT_CHARS   1.5
#define STATIC_POS_CHARS        2
#define STATIC_WIDTH_CHARS     15
#define NOT_IN_DICT        0xFFFF      // field-not-in-dict identifier
#define MLE_SIZE           (4096L) // 2 * effective text size, regards CR CR LF
#define ENTRYFIELD_SIZE    128

typedef struct _XXCONTROL
{
   HWND            hwndStatic;
   HWND            hwndEntry;
   PUCHAR          pucTypeOfEntry;
   USHORT          usControlID;
   USHORT          fsStatus;
   USHORT          usFieldNo;          // number (index) of corresponding field
   USHORT          usLevel;            // level of field
   SWP             swpEntry;
   SWP             swpStatic;
} CONTROL, *PCONTROL;



typedef struct _EDITIDA
{
   HWND            hwndDlg;
   HWND            hwndFocus;
   HWND            hwndCall;
   HWND            hwndSelection;
   HWND            hwndLastField;
   HWND            hwndHeadword;
   HWND            hwndCopy;
   HWND            hwndPaste;
   CHAR_W          szOrgHeadword[MAX_TERM_LEN + 1]; // buffer for headword
   CHAR_W          szHeadword[MAX_TERM_LEN + 1]; // buffer for headword
   PSZ_W           pucHeadwordName;
   PSZ_W           pucTermData;
   USHORT          usTermDataLen;
   USHORT          usSaveToDictHandle;
   HWND            hwndGrBox;
   SWP             swpGrBox;
   HAB             hab;
   USHORT          usScrollWidth;
   USHORT          usBorderSize;      // X size of window border
   USHORT          usTemplate;
   USHORT          usPOS;
   USHORT          usMaxTempl;         // current maximum template value
   USHORT          usSense;
   USHORT          usTarget;
   USHORT          usDictIndex;
   USHORT          usSpinValue;        // current spin button value
   BOOL            fNewEntry;
   USHORT          usMaxShown;
   USHORT          usAddNodesOffset;
   HMODULE         hmod;
   SHORT           sLastPos;
   SHORT           sLastControlPos;
   BOOL            fNoSave;
   PUCHAR          pucAddNodes;
   PUCHAR          pucFirstAddNodes;
   USHORT          fsStatus;
   BOOL            fDestroy;
   PSWP            pswpFirst;
   BOOL            fNoSetFocus;
   PCONTROL      pstFocusControl;
   BOOL            fNoQueryFocus;
   BOOL            fKeyDown;
   PSZ_W           pucOldTermData;
   USHORT          usOldTermDataLen;
   PLUPCB          pLUPCB;             // ptr to lookup control block
   HDCB            hEditHandle;        // handle of dictionary containing entry
   PPROPDICTIONARY pEditProp;          // ptr to profile of edited dictionary
   PDCB      pEditCB;                  // ptr to CB of edited dictionary
   HDCB            hSaveDict;          // handle of save dictionary
   PUCHAR          pucSigRec;          // signature record data
   CHAR_W          ucbTermBuf[4096];   // buffer for terms
   BOOL            fDataChanged;       // 'entry data has been changed' flag
   BOOL            fTargChanged;       // 'target data has been changed' flag
   BOOL            fDisabled;          // 'dialog is disabled' flag
   BOOL            fChangedByProgram;  // 'entry field change by program' flag
   //-------------------------
   LONG       lStaticEnlargement;      // enlargment value of statics
   SHORT      sCharWidth;              // avarage character width
   SHORT      sCharHeight;             // character height
   SHORT      sCtrlDistance;           // Y distance of controls
   SHORT      sQuartChar;              // 1/4 of char height
   SHORT      sHalfChar;               // 1/2 of char height
   USHORT     usNumControls;           // number of controls
   USHORT     usNumVisControls;        // number of visible controls
   PCONTROL   pControls;               // ptr to control structure array
   PCONTROL   pLastControl;            // ptr to last control in array
   PSWP       pSWPs;                   // ptr to SWP buffer for
                                       //     WinSetMultWindowPos function

   CHAR       szTempBuf[4096];         // multi-purpose buffer: only dictnames!!
   PVOID      hLdbTree;                // handle of QLDB tree
   PSZ_W      apszFields[MAX_PROF_ENTRIES];   // field data pointers
   USHORT     ausNoOfFields[QLDB_MAX_LEVELS]; // array with no of fields/level
   USHORT     ausFirstField[QLDB_MAX_LEVELS]; // array with first field/level
   CHAR_W     szFieldBuffer[MLE_SIZE+1];         // buffer for field data
   PCHAR_W    pStringBuffer;           // string buffer
   USHORT     usHeadwordField;         // number of headword field in profile
   //--------- for list to dictionary interface -------
   BOOL       fList2Dict;              // TRUE = list to dict mode
   PTERM      *ppTermList;             // pointer into term list
   USHORT     usTerms;                 // number of terms waiting for processing
   BOOL       fListChanged;            // terms list has been changed flag
   CHAR       szTemplatePattern[256];  // pattern for template text
   CHAR_W     szUserIDW[MAX_USERID];    // user ID of a logged-on user
   CHAR_W     szHeadWordUserNameW[DICTENTRYLENGTH];  // if username is different to chSystName
   HFONT      hFontControl;             // font to be used for dialog controls
} EDITIDA, *PEDITIDA;


   VOID               AddTemplate            (PEDITIDA ,BOOL);
   ULONG              CheckClipbrdData       (HWND);
   PPROFENTRY         CheckName              (PEDITIDA , USHORT);
   VOID               CheckSelection         (PEDITIDA );
   VOID               ClipbrdToEntry         (PEDITIDA , HWND);
   PCONTROL           ControlFromHwnd        (PEDITIDA, HWND);
   PCONTROL           CreateControls        (PEDITIDA);
   BOOL               DataToWinText          (PEDITIDA );
   VOID               DeleteEntry            (PEDITIDA );
   VOID               DeletePOS              (PEDITIDA );
   VOID               DeleteSense            (PEDITIDA );
   VOID               DeleteTarget           (PEDITIDA );
   VOID               DeleteTemplate         (PEDITIDA );
   VOID               DisplayDAMessage       (HWND, USHORT );
   INT_PTR CALLBACK   EditEntryDlgProc       (HWND, WINMSG, WPARAM, LPARAM);
   VOID               EntryToClipbrd         (PEDITIDA, HWND );
   VOID               FillDictCombo          (PEDITIDA );
   PCONTROL           GetFirstVisControl     (PEDITIDA);
   PCONTROL           GetLastVisControl      (PEDITIDA);
   VOID               InitEdit               (HWND, PEDITIDA );
   PCONTROL           IsNodeDisplayed        (PEDITIDA , USHORT, PUSHORT);
   BOOL               NextTemplate           (PEDITIDA );
   BOOL               PreviousTemplate       (PEDITIDA );
   USHORT             SaveEntry              (HWND, PEDITIDA );
   VOID               UpdateTemplNumber      ( PEDITIDA  pstDlg, USHORT usTemplate );
   VOID               UpdateWinText          (PEDITIDA );
   BOOL               WinTextToData          (PEDITIDA );
   VOID               MoveControls           ( PEDITIDA, LONG );

//+----------------------------------------------------------------------------+
//| EditChangeCheck         - check if change of data is possible              |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    If nothing has been changed yet and the edit dictionary is read-only,   |
//|    a warning message is issued.                                            |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PEDITIDA  pIda                IN      pointer to edit IDA               |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    USHORT   usRC        TRUE (always)                                      |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    pIda must point to a valid IDA.                                         |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
USHORT EditChangeCheck
(
   PEDITIDA pIda                       // pointer to edit dialog IDA
);
