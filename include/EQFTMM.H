/*! EQFTMM.H
	Copyright (c) 1990-2015, International Business Machines Corporation and others. All rights reserved.
*/

#include "OtmProposal.h"
#include "core\PluginManager\OtmMemory.h"
#include "core\memory\MemoryFactory.h"
#include "vector"


#define TMM_VISCLASS  "TMMVisClass"    // class name
#define TMM_VISCLASSW L"TMMVisClass"   // class name (as wide character)
#define TMM_STATUSBAR "TMMStatusBar"   // class name
#define MSG_FIELD                 4     // units for message in status bar
#define DOC_FIELD                 6     // units for docname in status bar
#define DATE_FIELD                4     // units for date in status bar
#define NUM_FIELD                 0     // units for loaded segs in status bar
#define CUR_SEG_FIELD             5     // units for cur segnum in status bar
#define PERC_FIELD                4     // units for percentage in status bar
#define TMM_RESOURCE_DLL "EQFTMMR"
#define ADD_SEGMENT USER_TASK + 1
#define GET_SEGMENT USER_TASK + 2

#define ID_U5                     0x1000

#define ID_U5_MESSAGEBOX          0x3000
#define ID_U5_DLG                 0x3005

#define ID_U6_BOK                 0x3010
#define ID_U6_BCANCEL             0x3015
#define ID_U6_BHELP               0x3020
#define ID_U6_HELP                0x3022
#define ID_U6_T2                  0x3025
#define ID_U6_EF2                 0x3030
#define ID_U5_GET_TM              0x3035

#define EQF_TM_MAINT_RES          "EQFTMFUN"
#define EQF_TM_PW                 "TMMaint"
#define TM_PASSWORD               "ABMAINT"
#define TM_PW_LAST_TRY            "XXXXXXX"
#define PW_LAST_TRY               3
#define TM_PW_LENGTH              7
#define START_YEAR                1992
#define MAX_MONTH_LEN             15
#define NINETEEN                  1900
#define SLASH                     '/'
#define HYPHEN                    '-'

#define MAX_SEGS_PER_LOAD         99999 /*32000*/

/**********************************************************************/
/* types for access to out internal tables                            */
/**********************************************************************/
typedef enum _TMMTABLE
{
  TMM_TAGTABLES,                       // tag tables contained in TM
  TMM_LANGUAGES,                       // languages used by TM
  TMM_DOCUMENTS,                       // documents referenced by TM
  TMM_LONGNAMES                        // document long names referenced by TM
} TMMTABLE;

// table index value if no string is available
#define NOSTRINGINDEX -1

/**********************************************************************/
/* macro to update statusbar                                          */
/**********************************************************************/
#define UPDSTATUSBAR( p ) INVALIDATERECT( (p)->hStatusBarWnd, NULL, FALSE )

typedef enum _MONTH_CODE
{
  JANUARY,
  FEBRUARY,
  MARCH,
  APRIL,
  MAY,
  JUNE,
  JULY,
  AUGUST,
  SEPTEMBER,
  OCTOBER,
  NOVEMBER,
  DECEMBER,
  MAXENTRY
} MONTH_CODE;

typedef struct _NEWTM_ADDRESS
{
  ULONG       ulKey;                   // TM key to get
  USHORT      usTarget;                // Target within record
} NEWTM_ADDRESS, *PNEWTM_ADDRESS;


typedef struct _TMIDA
{
  CHAR_W      szSearchString[MAX_SEGMENT_SIZE + 2];
  CHAR_W      szOrgSearchString[MAX_SEGMENT_SIZE + 2];
  CHAR        szSourceLng[MAX_LANG_LENGTH];
  CHAR        szTargetLng[MAX_LANG_LENGTH];
  CHAR_W      szSourceSeg[MAX_SEGMENT_SIZE];
  CHAR_W      szTargetSeg[MAX_SEGMENT_SIZE];
  ULONG       ulSeg2Load;
  CHAR        szDoc2Search[MAX_LONGFILESPEC];
  CHAR        szDoc2SearchUpr[MAX_LONGFILESPEC]; // uppercased search document
  BOOL        fDocListGiven;            // TRUE if user selects files from list
  CHAR        *pszDoc2SearchUprList;     // Ptr to dynamically allocated X15 list
  //USHORT      usDifInPtr;
 // USHORT      usDifAllocated;
  SHORT       sCurCluster;
  ULONG       ulSegNum;                 // segment number form TM
  ULONG       ulSearchSegNum;           // segment number to be searched
  USHORT      usTranslationFlag;        // buffer for translation flag
  ULONG       ulRunNum;                 // number of curretnly transfered segment
  ULONG       ulCurTMKey;               // key of current segment
  ULONG       ulExtracted;
  LONG        lPercent;
  //TMX_EXT_IN_W  TMIn;
  //TMX_EXT_OUT_W TMOut;
  PNEWTM_ADDRESS pAddr;
  ULONG       ulAddrAlloc;
  ULONG       ulPrevPos;
  ULONG       ulAddrCount;
  BOOL        fPrevious;
  NEWTM_ADDRESS  NextSegAddr;           // Address of next segment
  CHAR        szTagTable[MAX_EQF_PATH];
  CHAR        szSegMarkup[MAX_EQF_PATH];// markup of current segment
  CHAR        szFileName[MAX_LONGFILESPEC];
  CHAR        szMemShortName[MAX_FILESPEC]; // short name of TM
  CHAR        szMemLongName[MAX_LONGFILESPEC]; // long name of TM
  SHORT_FN    szSourceDoc;
  LONG_FN     szLongName;
  LONG        lDate;
  LONG        lBufDate;                // date currently used in szBufDate
  CHAR        szBufDate[30];           // buffer for date strings
  SERVERNAME  szServer;
//  HTM         hTMMaint;
  OtmMemory   *pMemory;

  CHAR        szPassword[MAX_FNAME];
  USHORT      usPwTry;
  LONG        lCurrentDate;
  LONG        lStartDate;
  LONG        lEndDate;
  USHORT      usStartMonth;
  USHORT      usEndMonth;
  USHORT      usStartYear;
  USHORT      usEndYear;
  BOOL        fCase;        // respect = TRUE, ignore = FALSE
  BOOL        fLookup;      // lookup in source = TRUE, in target = FALSE
  BOOL        fExact;       // lookup exact matches = TRUE, other stuff = FALSE
  BOOL        fMoreTrans;   // get only matches that contain more than one trans
  BOOL        fMPlus;       // include M-flagged segments
  BOOL        fMMinus;      // include segments w/o M-flags
  BOOL        fDate;
  BOOL        fDoc;
  BOOL        fFound;
  BOOL        fNorm;
  BOOL        fStop;
  BOOL        fRun;
  SHORT       sLanguages;             // number of languages in TM
  PSZ         (*ppLanguages)[];       // table of TM languages ptr
  SHORT       sTagTables;             // number of tagtables in TM
  PSZ         (*ppTagTables)[];       // table of TM tagtables ptr
  SHORT       sDocuments;             // number of documents in TM
  PSZ         (*ppDocuments)[];        // table of TM documents ptr
  SHORT       sLongNames;             // number of long document names in TM
  PSZ         (*ppLongNames)[];       // table of TM long document names ptr
  PPOOL       pPool;                   // strings for the tables above
  CHAR        szBuffer[1024];          // general purpose buffer
  USHORT      usPMSegs;                // number of surrounding segments to display
  UINT        uiErrCode;               // error code
  CHAR_W      szSourceSegBuf[MAX_SEGMENT_SIZE]; // temporary segment buffer
  CHAR_W      szTargetSegBuf[MAX_SEGMENT_SIZE]; // temporary segment buffer
  BOOL        fSkipThisGroup;          // for more than one translations mode: skip this group
  //TMX_EXT_IN_W  TMInTemp;                // buffer for look-ahead in more-than-one-translation mode
  //TMX_EXT_OUT_W TMOutTemp;               // buffer for look-ahead in more-than-one-translation mode
  CHAR_W      szContext[MAX_SEGMENT_SIZE];
  ULONG       ulSystemPrefCP;           // OEM/ASCII CP of system pref.lang
  CHAR_W      szBufferW[MAX_SEGMENT_SIZE+MAX_SEGMENT_SIZE];
  CHAR        szProposalKey[50];      // buffer for key of current proposal
  CHAR_W      szPrevSourceSeg[MAX_SEGMENT_SIZE]; // source text of previous segment
  CHAR_W      szPrevTargetSeg[MAX_SEGMENT_SIZE]; // target text of previous segment
  char        szCurrentKey[20];         // buffer for current memory position
  CHAR_W      szTempSourceSeg[MAX_SEGMENT_SIZE]; // buffer for temporary source or target text
  CHAR_W      szLastSourceSeg[MAX_SEGMENT_SIZE]; // source text of last processed segment
} TMIDA, * PTMIDA;


typedef struct _MONTHTABLE
{
   USHORT  usMonthNum;
   CHAR    szMonthName[MAX_MONTH_LEN];
} MONTHTABLE, *PMONTHTABLE;

typedef struct _TMEXTRAS
{
  ULONG      ulTmSegNum;               // tm segment number
  SHORT      sSourceDocInd;            // index of short document name
  SHORT      sLongNameInd;             // index of long document name
  LONG       lDate;
  SHORT      sTargetLangInd;           // index of target language
  SHORT      sTagTableInd;             // index of tag table name
  BYTE       sSourceLF;                // number of LFs at end of source segm
  BYTE       sTargetLF;                // number of LFs at end of target segm
  USHORT     usTranslationFlag;        // Translation flag
  CHAR       szKey[50];                // internal proposal key
  PSZ_W      pszContext;               // segment context or NULL if none
} TMEXTRAS, * PTMEXTRAS;

typedef struct _TMMVISDOC
{
  PVOID pTMMIda;                       // pointer to itm ida ...
  QSTATUS qVisActState;                // stores old state of VISACT segment
  PTBDOCUMENT pDoc;                    // pointer to active document
} TMMVISDOC, * PTMMVISDOC;

typedef struct _TMMIDA
{
  HWND  hwnd;                          // tm maintain window handle
  HWND  hwndFrame;
  BOOL  fKill;                         // success indicator
  BOOL  fUpdate;
  CHAR  chTranslMemory[MAX_EQF_PATH];  // translation memory
  CHAR        szMessage[30];           // status for status bar( running... )
  TMMVISDOC   stVisDocTgt;             // structure containing target vis info
  TMMVISDOC   stVisDocSrc;             // structure containing source vis info
  TBDOCUMENT  TBSourceDoc;             // source document struct
  TBDOCUMENT  TBTargetDoc;             // target document struct
  PBUFCB      pBufCB;                  // pointer to control block
  HWND        hStatusBarWnd;           // status bar
  HAB         hab;                     // PM anchor block handle
  BOOL        fHorizontal;             // true if visdoc windows horizontally
  BOOL        fDelSeg;                 // true if segment is to be deleted
  ULONG       ulSrcSegNum;             // current segment no
  ULONG       ulTgtSegNum;             // current segment no
  USHORT      usActWin;                // active window
  PTEXTTYPETABLE pColorTable;          // table with color settings
  CHAR        chTMFname[MAX_EQF_PATH]; // translation memory
  PTMEXTRAS   pTMParms ;               // pointer to TM seg no & doc name
  PTMIDA      pTMIda;                  // passed structure
  HWND        hwndFocus;               // window having the input focus
  HMENU       hwndMenu;                // handle of our AAB
  HMENU       hwndWndMenu;             // handle of our windows submenu
  // buffers for text strings (prompts) used in status bar
  CHAR        szStatusPrompt[30];      // prompt for status
  CHAR        szDocPrompt[30];         // prompt for document
  CHAR        szDatePrompt[30];        // prompt for date
  CHAR        szCurPrompt[30];         // prompt for current segment
  CHAR        szTotalPrompt[30];       // prompt for total segments
  CHAR        szComplPrompt[30];       // prompt for completed percentage
  CHAR        szWorkState[30];         // message for work state
  CHAR        szDeleteState[30];       // message for delete state
  CHAR        szQueryState[30];        // message for query state
  CHAR        szSearchState[30];       // message for search state
  CHAR        szBuffer[1024];          // general purpose buffer
  CHAR        szDocName[MAX_LONGFILESPEC]; // buffer for document name
  SHORT       sOrgFocusLine;           // original editor focus line
} TMMIDA, * PTMMIDA;

VOID    LastSegDel( HWND, PTMMIDA );
VOID    CheckBlock( PTBDOCUMENT, ULONG );
VOID    BackToQuery( PTMMIDA, HWND );
VOID    NextPrevious( PTMMIDA, HWND, BOOL, BOOL );
BOOL    GetAndStrip( PTBSEGMENT *, PSZ_W *, PTBDOCUMENT, ULONG );
BOOL    TmmStartHandler ( HAB, PSZ, PFNWP );
__declspec(dllexport)
MRESULT APIENTRY TMMWNDPROC( HWND, WINMSG, WPARAM, LPARAM );
MRESULT APIENTRY TMMVISWNDPROC( HWND, WINMSG, WPARAM, LPARAM );
MRESULT APIENTRY TMMSTATUSBARWNDPROC( HWND, WINMSG, WPARAM, LPARAM );
static VOID TMMFillStatusBar( HWND, HPS  );
VOID    SetupUtils( HAB );
BOOL    DoVisualTMM( HAB, HWND, PTMMIDA );
VOID    TMMDocInit( PTMMIDA );
BOOL    TMMDocStart( HAB, HWND, PTMMVISDOC );
VOID    FreeTMMDoc( PTBDOCUMENT );
VOID    TMMSetWindowPos( PTMMIDA pIda, SHORT cx, SHORT cy );
VOID    TMMTile( PTMMIDA, BOOL );
VOID    TMMFuncQuit( PTMMIDA );
void    TMMCommand( HWND, WPARAM );
MRESULT HandleTMMWMChar( HWND, WPARAM, LPARAM, WINMSG );
BOOL    AddSegment( PTBDOCUMENT, PSZ_W, ULONG, PULONG, SHORT );
BOOL    AddSourceAndTranslation( PTMMIDA );
VOID    AdjustLines( PTBDOCUMENT, PSZ_W, PUSHORT, SHORT );
VOID    AdjustLinesInSegment( PTBDOCUMENT, SHORT );
VOID    SetLineNumber( PTBDOCUMENT, PSZ_W );
static  VOID EQFDispWindow( PTBDOCUMENT );
BOOL    ActivateSegment( PTMMIDA, ULONG );
BOOL    UnchangedSegment( PTMMIDA, ULONG );

BOOL    SaveSegment( PTMMIDA, PBOOL );
VOID    AddLFAndUpdate( PTBDOCUMENT );
VOID    DelTrailingLF( PTBDOCUMENT );
BOOL    TMMReplace( PTMIDA, PSZ_W, PSZ_W, PSZ, OtmMemory *, PTMEXTRAS );
BOOL    TMMDelete( PTMIDA, PSZ_W, PSZ_W, PSZ, OtmMemory *, PTMEXTRAS );
VOID    UpdateSegTable( PTBDOCUMENT );
VOID    SubtractLineNum( PTBDOCUMENT, PSZ_W );
VOID    TMM_Error( USHORT, PSZ, PSZ );
BOOL    APIENTRY TMEditWork ( PTBDOCUMENT );
BOOL    FreeForNext( PTMMIDA );
BOOL    TMMDocNext( PTMMVISDOC );
BOOL    DeleteSegments( PTMMIDA );
BOOL    DeleteAllSegments( PTMMIDA );
BOOL    setMFlag(PTMMIDA, PTBDOCUMENT, USHORT );
/**********************************************************************/
/* function prototypes from query                                     */
/**********************************************************************/
BOOL    GetQuerySettings( HWND, PTMIDA );
BOOL    CompareSearch2Seg( PTMIDA, PSZ_W, PSZ_W );
BOOL    ExactCompare( PSZ_W, PSZ_W );
BOOL    WildCompare( PSZ, PSZ );
BOOL    WildCompareW( PSZ_W, PSZ_W );

BOOL    GetTmProp2Ida( PTMIDA );

__declspec(dllexport) BOOL    StartTMFunct( HWND, PTMIDA );
VOID    UtlSetString( SHORT sID, PSZ pszString );
VOID    SetupDateDlg( HWND, PTMIDA );
VOID    SetupQueryDlg( HWND, PTMIDA );
BOOL    QueryDateSetting( HWND, PTMIDA );
BOOL    QueryDocSetting( HWND, PTMIDA );
__declspec(dllexport) BOOL    QueryTMEntry( PTMIDA );
BOOL    GetAndCompareSegment( PTMIDA, OtmProposal &prop );
__declspec(dllexport) VOID    FreeTMIda( PTMIDA );
__declspec(dllexport) VOID    FreeTMIdaForSave( PTMIDA );
VOID    FreeListPSZ( PTMIDA );
VOID    CutCR_LF( PSZ_W, BOOL, ULONG );
VOID    AsterixCR_LF( PSZ_W );
BOOL    AddToList( PTMIDA, PSZ, TM_ADDRESS );
VOID    SetTransDate( PSZ, LONG );
VOID  PASCAL FAR TMCleanUp( USHORT );

MRESULT EXPENTRY TMPASSWORDDLG( HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) INT_PTR CALLBACK TMQUERYDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK TMSPECDATEDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK TMSPECDOCDLG( HWND, WINMSG, WPARAM, LPARAM );
MRESULT EXPENTRY EDITENTRYDLG( HWND, WINMSG, WPARAM, LPARAM );
MRESULT QueryWM_CONTROL( HWND, SHORT, SHORT );
MRESULT QueryWM_COMMAND( HWND, SHORT, SHORT );
MRESULT SpecDateWM_CONTROL( HWND, SHORT, SHORT );
MRESULT SpecDateWM_COMMAND( HWND, SHORT, SHORT );

VOID SetHorzScrollingForLB(HWND);

BOOL TMMGetListFromTM
(
  PTMIDA      pTMIda,                  // ptr ot TMM Ida
  TMMTABLE    Table                    // ID of table
);
__declspec(dllexport) SHORT TMMFindInTable
(
  PTMIDA      pTMIda,                  // ptr ot TMM Ida
  PSZ         pszName,                 // name being looked up
  TMMTABLE    Table                    // ID of table
);
__declspec(dllexport) BOOL TMMGetNameForInd
(
  PTMIDA      pTMIda,                  // ptr ot TMM Ida
  SHORT       sIndex,                  // index of name
  PSZ         pszName,                 // buffer for name
  TMMTABLE    Table                    // ID of table
);
__declspec(dllexport) VOID TMMCorrectLFCount
(
  PSZ_W         pszString,               // points to string to be processed
  SHORT       sLFCount                 // number of LFs at end of data required
);
__declspec(dllexport) LONG TMMCountLFAtEnd
(
  PSZ_W         pszString                // points to string to be processed
);

