/*! \file
	Description: Defines for Global Find and Replace (GFR) function

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

// logging functions and defines
#ifdef _DEBUG
  static FILE *hfLog = NULL;
  #define LOGSTART() { char p[60]; UtlMakeEQFPath( p, NULC, SYSTEM_PATH, NULL ); strcat( p, "\\LOGS" ); UtlMkDir( p, 0, FALSE ); strcat( p, "\\GLOBALFR.LOG" ); hfLog = fopen( p, "a" ); }
  #define LOGEND() { if ( hfLog ) fclose( hfLog ); }
  #define LOGWRITESTRING( s ) { if ( hfLog ) fprintf( hfLog, "%s\n", s ); }
  #define LOGWRITESTRINGANDSTRING( s1, s2 ) { if ( hfLog ) fprintf( hfLog, "%s%s\n", s1, s2 ); }
  #define LOGWRITESTRINGANDINT( s, i ) { if ( hfLog ) fprintf( hfLog, "%s%ld\n", s, i ); }

#else
  #define LOGSTART()
  #define LOGEND()
  #define LOGWRITESTRING( s )
  #define LOGWRITESTRINGANDSTRING( s1, s2 )
  #define LOGWRITESTRINGANDINT( s, i )
#endif

// activate this define to show debug info in text area
#ifdef _DEBUG
//  #define SHOWDEBUG_INFO
#endif


/**********************************************************************/
/* various sizes and other constant values                            */
/**********************************************************************/
// dummy character for "NONE" wildcard
#define NONE_WILDCARD L'\xFFFF'

// text to be shown for NONE wildcard
static CHAR_W szNoneWildcard[] = L"NONE";

// list of characters allowed for single and multiple substitution wildcards
// the first character is the NONE dummy character
static CHAR_W szWildCards[] = L"\xFFFF!§$^%&=?*+#~";

// current character to be used for single and multiple substitution
static CHAR_W chMultSubst;
static CHAR_W chSingleSubst;

// default wildcard characters
#define DEFAULT_MULT_WILDCARD   L'*'
#define DEFAULT_SINGLE_WILDCARD L'?'

// strings for the status line
#define ERROR_MEMORY_STRING "search aborted, insufficient memory"
#define INFO_SEARCHING      "searching..."
#define INFO_REPLACING      "replacing..."
#define INFO_PREPARING      "preparing replace..."
#define INFO_SEARCHDONE     "search complete, found %u occurences of search string"
#define INFO_REPLACEDONE    "replacing complete, %u occurences of %u found search strings have been replaced"
#define INFO_PREPAREDONE    "prepare replace complete, %u occurences of %u found search strings can be replaced"
#define INFO_SEARCHSTOPPED  "search has been stopped, found %u occurences of search string"
#define INFO_REPLACESTOPPED "replacing has been stopped, %u occurences of %u found search strings have been replaced"
#define INFO_PREPARESTOPPED "prepare replace has been stopped, %u occurences of %u found search strings can be replaced"


// display column modes
typedef enum _DISPLAY_COLUMNS_MODE { UNDEFINED_COLUMN_MODE = 0, ONLY_SOURCE_COLUMN_MODE, ONLY_TARGET_COLUMN_MODE, SOURCE_AND_TARGET_COLUMN_MODE } DISPLAY_COLUMNS_MODE;

// fixed width of first two columns
#define DOCNUM_COL_WIDTH 0x42
#define SEGNUM_COL_WIDTH 0x42

// ID of our timer
#define FOLFINDTIMER 9876

// result list export formats
#define RESULTLIST_EXPORT_PLAINTEXT  1
#define RESULTLIST_EXPORT_XML        2

// filter for result list export formats (same sequence as result export format IDs required)
#define RESULTLIST_EXPORT_FORMAT_FILTERS "Plain Text (*.TXT)\0*.TXT\0XML (*.XML)\0*.XML\0\0\0"

// filter for batch find & replace list export and import format
#define RBATCHLIST_EXPORT_FORMAT_FILTERS "Comma-separated values (*.CSV)\0*.CSV\0\0\0"

// defines to be used with Global Find&Change (GFC) sSearchIn member of the FOLFINDLASTUSED structure:
#define GFR_SEARCH_IN_TARGET 0
#define GFR_SEARCH_IN_SOURCE 1
#define GFR_SEARCH_IN_BOTH   2

// name of the file containing the contents of the last used batch list
#define LASTUSEDBATCHLIST "BatchList.LastUsed"

// max number of displayed lines per entry
#define MAX_LINES_PER_ITEM 100

// text draw types
typedef enum _DRAWTYPE { NORMALTEXT_DRAWTYPE, FOUND_DRAWTYPE, TOBECHANGED_DRAWTYPE, CHANGED_DRAWTYPE, CHANGETO_DRAWTYPE, CHANGEDTO_DRAWTYPE, ADDSEGMENTTEXT_DRAWTYPE } DRAWTYPE;

/**********************************************************************/
/* Tasks during find and change operations                            */
/**********************************************************************/
typedef enum _FINDTASK
{
  OPEN_DOC_TASK,
  FIND_NEXT_TASK,
  APPLY_CHANGE_TASK,
  CLOSE_DOC_TASK,
  NEXT_DOC_TASK,
  STOP_TASK,
  INITDLG_TASK,
  SIZEDLG_TASK,
  WAIT_TASK,
  START_SEARCH_TASK,
  SETFOCUS_TASK,
} FINDTASK;

// IDs for operands and operators in find expressions
typedef enum _FF_OPERATOR
{
  FF_EMPTY_OP,
  FF_STRING_OP,
  FF_AND_OP,
  FF_OR_OP,
  FF_NOT_OP
} FF_OPERATOR;

// structure for the definition of find expression operators
typedef struct _FOLFINDOPERATOR
{
  CHAR_W           szName[40];         // operator name
  FF_OPERATOR      ID;                 // ID of operator
} FOLFINDOPERATOR, *PFOLFINDOPERATOR;


// max. number of find locations per segment
#define MAX_FINDENTRIES 10

// structure to collect find locations in active segment
typedef struct _FOLFIND_FINDLIST
{
  int         iEntries;                          // number of used entries
  int         aiStartPos[MAX_FINDENTRIES+1];     // array with start positions
  int         aiEndPos[MAX_FINDENTRIES +1];      // array with end positions
} FOLFIND_FINDLIST, *PFOLFIND_FINDLIST;

typedef enum _FINDPOSTYPE 
{ 
  FOUND_TYPE,                // entry for a found string
  CHANGETO_TYPE,             // entry for a found string which can be changed to the change-to string
  CHANGED_TYPE,              // entry for a found string which has been changed to the change-to string by the user
  AUTOCHANGED_TYPE           // entry for a found string which has been changed automatically (the offsets of following entries have to been adjusted because of the changed string)
} FINDPOSTYPE;

// entry for the position of a found text within a segment
typedef struct _FOLFINDRESULTPOS
{
  BOOL   fTarget;            // TRUE = found string in target, FALSE = found string in source
  BOOL   fPreserveCase;      // state of the preserve state flag from the time the entry was changed
  FINDPOSTYPE Type;          // type of entry
  USHORT usOffs;             // offset within segment
  USHORT usLen;              // length of found string
  USHORT usChangeOffs;       // offset of change-to string in changed texts (only for CHANGETO_TYPE, CHANGED_TYPE, and AUTOCHANGED_TYPE)
} FOLFINDRESULTPOS, *PFOLFINDRESULTPOS;

// enumeration for the text string table in the result entry structure
enum RESULTTEXTID 
{ 
  SOURCE_TEXT = 0,           // source text of the segment containing the found string
  TARGET_TEXT,               // target text of the segment containing the found string
  PREV_SEG_SOURCE_TEXT0,     // source text of the previous segment 
  PREV_SEG_SOURCE_TEXT1,     // source text of the segment before the previous segment  
  PREV_SEG_TARGET_TEXT0,     // target text of the previous segment  
  PREV_SEG_TARGET_TEXT1,     // target text of the sgment before the previous segment  
  NEXT_SEG_SOURCE_TEXT0,     // source text of the next segment 
  NEXT_SEG_SOURCE_TEXT1,     // source text of the segment following the next segment  
  NEXT_SEG_TARGET_TEXT0,     // target text of the next segment   
  NEXT_SEG_TARGET_TEXT1,     // target text of the segment following the next segment  
  MAX_NUM_OF_TEXTS           // end of enumeration identifier 
};
// variable length entry for search results, the fixed part of the structure
// is followed by the source and target stringsand iAllocatedEntries FOLFINDRESULTPOS entries
typedef struct _FOLFINDRESULTENTRY
{
  int    iSize;              // size of entry (in number of bytes)
  int    iDocIndex;          // index of document
  ULONG  ulSegNum;           // segment number
  BOOL   fTranslated;        // TRUE = segment has been translated
  OBJNAME szDocObjName;      // document object name
  int    iTextLen[MAX_NUM_OF_TEXTS]; // length of the various texts  (in number of bytes)
  int    iTextOffs[MAX_NUM_OF_TEXTS]; // offset of the various texts  (relative to the begin of the FOLFINDRESULTENTRY structure)
  int    iResultPosOffs;     // offset of the list of FOLFINDRESULTPOS entries (relative to the begin of the FOLFINDRESULTENTRY structure)
  int    iAllocatedEntries;  // number of allocated FOLFINDRESULTPOS entries 
  int    iUsedEntries;       // number of used FOLFINDRESULTPOS entries
  int    iSourceLineBreaks[MAX_LINES_PER_ITEM]; // list of linebreak offsets within source text
  int    iTargetLineBreaks[MAX_LINES_PER_ITEM]; // list of linebreak offsets within source text
  int    iChangeToBufferUsed; // number of used bytes in change-to string buffer
  int    iChangeToBufferSize; // number of used bytes in change-to string buffer
  PSZ_W  pszChangeToBuffer;   // pointer to buffer with change-to strings
} FOLFINDRESULTENTRY, *PFOLFINDRESULTENTRY;

// variable length entry for batch change lists, the fixed part of the structure
// is followed by the search in target string, the search in source string and the replacement string
typedef struct _FOLFINDBATCHLISTENTRY
{
  int    iSize;              // size of entry (in number of bytes)
  int    iTargetFindLen;     // length of the target find string (in number of bytes)
  int    iTargetFindOffs;    // offset of the target find string (relative to the begin of the FOLFINDBATCHLISTENTRY structure)
  int    iSourceFindLen;     // length of the source find string (in number of bytes)
  int    iSourceFindOffs;    // offset of the source find string (relative to the begin of the FOLFINDBATCHLISTENTRY structure)
  int    iTargetChangeLen;   // length of the target change string (in number of bytes)
  int    iTargetChangeOffs;  // offset of the target change string (byte offset in change-to string buffer)
} FOLFINDBATCHLISTENTRY, *PFOLFINDBATCHLISTENTRY;


// structure containing the last used values of the global find and replace dialog
typedef struct _FOLFINDLASTUSED
{
   SHORT     sSearchIn;                          // "search text in" identifier (0 = target, 1 = source, 2 = both)
   EQF_BOOL  fShowSource;                        // state of "when found in TARGET, show SOURCE" checkbox
   EQF_BOOL  fShowTarget;                        // state of "when found in SOURCE, show TARGET" checkbox
   SHORT     sShowBeforeAfter;                   // number of segments to show before and after found text
   USHORT    usResultListExpMode;                // result list export mode
   CHAR      szResultListName[MAX_LONGPATH];     // name used for result list
   CHAR      szResultListDir[MAX_LONGPATH];      // directory the result list is exported to
   EQF_BOOL  fFolFindTranslTextOnly;             // translatable text only checkbox
   EQF_SWP   swpFolFindSizePos;                  // dialog size and position
   CHAR_W    szFolFind[MAX_FINDCHANGE_LEN+1];    // string being looked for
   CHAR_W    szFolChangeTo[MAX_FINDCHANGE_LEN+1];// new value for string
   EQF_BOOL  fFolFindWholeWordsOnly;             // LU: find whole words only
   CHAR_W    chWildCardSingleChar;               // wildcard for single characters
   CHAR_W    chWildCardMultChar;                 // wildcard for multiple characters
   FINDNAME  szFindList[MAX_SEARCH_HIST];        // List of Last used find values in global find and change
   FINDNAME  szReplaceList[MAX_SEARCH_HIST];     // List of Last used replace values
   EQF_BOOL  fFolFindConfirm;                    // 'Confirm changes' checkbox flag
   EQF_BOOL  fFolFindUpdateTM;                   // 'Update translation memory' checkbox flag
   EQF_BOOL  fFolFindCaseRespect;                // 'Respect case' radiobutton state
   CHAR      szBatchListName[MAX_LONGPATH];      // name used for batch list
   CHAR      szBatchListDir[MAX_LONGPATH];       // directory the batch list is imported from or exported to
   BOOL      fAndFindInSource;                   // TRUE = search additional string in source
   FINDNAME  szAddFindList[MAX_SEARCH_HIST];     // List of Last used replace values
   CHAR_W    szAndFindInSource[MAX_FINDCHANGE_LEN+1];// last used "and find in source" string
   BOOL      fApplyBatchList;                    // true = apply a batch find/replace list
   BOOL      fRespectLineFeeds;                  // TRUE = break lines at line feeds, FALSE = ignore line feeds and show text as a single block
   COLORREF  aclrForeground[40];                 // array with user defined foreround colors
   COLORREF  aclrBackground[40];                 // array with user defined background colors
  // info on last used font
  LONG lfHeight;
  LONG lfWidth;
  LONG lfEscapement;
  LONG lfOrientation;
  LONG lfWeight;
  BYTE lfItalic;
  BYTE lfUnderline;
  BYTE lfStrikeOut;
  BYTE lfCharSet;
  BYTE lfOutPrecision;
  BYTE lfClipPrecision;
  BYTE lfQuality;
  BYTE lfPitchAndFamily;
  TCHAR lfFaceName[40];
  BOOL      fPreserveCase;                      // true = preserve case of first character
  CHAR      szFiller[6360];                     // room for future enhancements
} FOLFINDLASTUSED, *PFOLFINDLASTUSED;

// structure passed to/from batch list result dialog
typedef struct _BATCHLISTENTRYDATA
{
  BOOL        fAddEntry;                        // TRUE = this entry is added, FALSE = an existing entry is being worked on
  BOOL        fOK;                              // TRUE = entry is OK, FALSE = dialog was cancelled
  CHAR_W      szTargetFind[MAX_SEGMENT_SIZE+1]; // find in TARGET string
  CHAR_W      szSourceFind[MAX_SEGMENT_SIZE+1]; // find in SOURCE string
  CHAR_W      szTargetChange[MAX_SEGMENT_SIZE+1]; // change to in target string
} BATCHLISTENTRYDATA, *PBATCHLISTENTRYDATA;

// number of pushbuttons at the bottom of the global find&replace window
#define NUM_OF_GFR_BUTTONS 6

// list of different text types
// main entry types (the value is the base index into the color and name table)
#define GFR_NORMAL_ENTRY     0              /* text in normal entry */
#define GFR_SELECTED_ENTRY   7              /* text in selected entry */  
#define GFR_FOCUS_ENTRY     14              /* text in entry with input focus */

// subtext defines (added to the main entry type gives the index into the color and name table)
#define GFR_NORMAL_TEXT         0
#define GFR_FOUND_TEXT          1
#define GFR_TOBECHANGED_TEXT    2
#define GFR_CHANGED_TEXT        3
#define GFR_CHANGETO_TEXT       4
#define GFR_CHANGEDTO_TEXT      5
#define GFR_ADDSEGMENTDATA_TEXT 6 

/**********************************************************************/
/* Instance data area (IDA) for find/change dialog                    */
/**********************************************************************/
// GQ: ensure that all elements of the data area are aligned on 4 byte boundaries (without it some Windows functions fail to access the data)
#pragma pack(4)
typedef struct _FOLFINDDATA
{
  HWND        hwnd;                    // handle of dialog window
  OBJNAME     szObjName;               // object name of dialog
  OBJNAME     szFolObjName;            // object name of folder
  BOOL        fRegistered;             // 'dialog has been registered' flag
  BOOL        fFromFolderList;         // 'dialog called from folder list' flag
  BOOL        fFind;                   // TRUE = we are in find mode
  BOOL        fTerminate;              // TRUE = dialog is being terminated
  BOOL        fCaseIgnore;             // TRUE = ignore case
  BOOL        fConfirm;                // TRUE = confirm on changes
  BOOL        fUpdateTM;               // TRUE = update folder memory
  CHAR_W      szFind[MAX_FINDCHANGE_LEN+1];     // string being looked for
  CHAR_W      szFindUpr[MAX_FINDCHANGE_LEN+1];  // uppercase version of szFind
  USHORT      usFindLen;               // length of found string
  CHAR_W      szChangeTo[MAX_FINDCHANGE_LEN+1]; // new value for string
  CHAR_W      szBuffer[1024];          // general purpose buffer
  FINDTASK    CurTask;                 // currently performed task
  int         iCurDocInd;              // index of current document (invisible document listbox)
  int         iCurrentDocumentListboxItem;       // index of current document in visible document listbox
  CHAR        szCurDoc[MAX_FILESPEC];  // name of currently processed document
  PTBDOCUMENT pTargetDoc;              // ptr to loaded target document
  PTBDOCUMENT pSourceDoc;              // ptr to loaded source document
  ULONG       ulSegNum;                // number of active segment
  USHORT      usOffs;                  // position within current segment
  USHORT      usFoundOffs;             // position of found string within segment
  USHORT      usLine;                  // current line number within document
  CHAR_W      szTempSeg[MAX_SEGMENT_SIZE];       // buffer for current segment
  HWND        hwndLB;                  // handle of listbox for document names
  USHORT      usHits;                  // number of hits found
  USHORT      usChanges;               // number of strings changed
  USHORT      usMayBeChanged;          // number of found strings which can be changed
  CHAR        szDocMemory[MAX_LONGFILESPEC]; // buffer for document TM name
  CHAR        szDocFormat[MAX_FILESPEC]; // buffer for name of document markup table
  CHAR        szDocSrcLng[MAX_LANG_LENGTH]; // buffer for document source lang
  CHAR        szDocTgtLng[MAX_LANG_LENGTH]; // buffer for document target lang
  OtmMemory   *pMem;                   // open TM object
  BOOL        fTMUpdRequired;          // TRUE = TM update us required

  unsigned    DocTime;                 // Update time of STARGET document
  unsigned    DocDate;                 // Update date of STARGET document

  BOOL        fDocOpenActive;          // TRUE if DocOpen has been activated
  CHAR        szLongName[MAX_LONGFILESPEC];  // long name of current document
  CHAR        szFolLongName[MAX_LONGFILESPEC];  // long name of current folder
  CHAR        szAlias[MAX_LONGFILESPEC];  // alias for current document
  CHAR        szShortAlias[MAX_FILESPEC]; // short alias for current document
  CHAR        szDocObjName[MAX_EQF_PATH]; // buffer for document object names
  BOOL        fUseThaiFont;               // use Thai font for our MLEs
  BOOL        fTranslTextOnly;            // TRUE = search translatable text only
  CHAR        chTokBufStartStop[MAX_SEGMENT_SIZE*4];// token buffer for start/stop table
  SHORT       sBorderSize;                       // size of dialog border
  HWND        hwndButton[NUM_OF_GFR_BUTTONS];    // handles of pushbuttons
  SHORT       sButtonWidth[NUM_OF_GFR_BUTTONS];  // widths of pushbuttons
  SHORT       sButtonHeight;                     // height of first pushbutton
  CHAR_W      szFindOld[MAX_FINDCHANGE_LEN+1];   // previous find string
  SWP         swpDlg;                            // dialog size and position
  CHAR        szSubFolObjName[MAX_EQF_PATH];     // buffer for subfolder/folder object name
  CHAR_W      szSearchPattern[MAX_FINDCHANGE_LEN+1]; // prepared search string for pattern matching
  BOOL        fWholeWordsOnly;                   // search whole words only
  BOOL        fLogicExpression;                  // TRUE = find string uses logical expression
  CHAR_W      szExpression[2*MAX_FINDCHANGE_LEN];// buffer for find expression
  FOLFIND_FINDLIST ActFindList;                  // list of find locations in current segment
  FOLFIND_FINDLIST LastFindList;                 // list of find locations in current segment of previous find
  ULONG       ulActFindSeg;                      // segment number of actual match
  ULONG       ulLastFindSeg;                     // segment number of last match
  CHAR_W      chWildCardSingleChar;              // wildcard for single characters
  CHAR_W      chWildCardMultChar;                // wildcard for multiple characters
  SHORT       sTgtLangID;                        // morph ID of document target language
  SHORT       sSrcLangID;                        // morph ID of document source language
  BOOL        fMultipleObjs;                     // TRUE: we are searchin in more than one folder
  PSZ         pszObjList;                        // points to list of objects being searched
  PSZ         pszActiveFolder;                   // currently searched folder in folder object list
  CHAR        szNameBuffer[MAX_LONGFILESPEC*2];  // name buffer
  DISPLAY_COLUMNS_MODE CurDisplayMode;           // current display mode
  BOOL        fSearchInTarget;                   // search in target document (user setting)
  BOOL        fSearchInSource;                   // search in source document (user setting)
  BOOL        fCurrentSearchInSource;            // TRUE = use source document for current search , FALSE = use target document for current search
  FINDTASK    ThreadTask;                        // currently performed task
  BOOL        fProcessingTimerMessage;           // TRUE = currently processing WM_TIMER message
  HWND        hStatus;                           // handle of status bar
  HWND        hResultListBox;                    // handle of the result list box
  int         aiListViewColWidth[10];            // current width of list view columns
  int         aiBatchListColWidth[10];           // current width of batch list view columns
  // list of results
  int iResultListSize;                           // size of result list in number of entries
  int iResultListUsed;                           // number of used entries in result list
  PFOLFINDRESULTENTRY *ppResultList;              // pointer to a list of PFOLFINDRESULTENTRY pointers
  int iResultsDisplayed;                         // number of result entries already shown in result area
  CHAR        szResultListFile[MAX_LONGPATH];     // fully qualified file name of result list

  // batch change list info
  int iBatchListSize;                            // size of batch list in number of entries
  int iBatchListUsed;                            // number of entries in batch list
  PFOLFINDBATCHLISTENTRY *ppBatchList;           // pointer to a list of PFOLFINDBATCHLISTENTRY pointers

  // thread communication and data area
  BOOL        fThreadIsRunning;                  // TRUE = Thread is running
  BOOL        fStopThread;                       // TRUE = thread should stop and terminate itself
  BOOL        fThreadIsActive;                   // TRUE = thread is searching, FALSE = thread is waiting for work
  BOOL        fStopSearch;                       // TRUE = stop current search (set by main window code)
  BOOL        fStartSearch;                      // TRUE = start a new search (set by main window code, reset by thread) 
  BOOL        fSearching;                        // TRUE = a search operation is performed by the thread
  CHAR        szStatusLine[1024];                // status information                       
  USHORT      usThreadError;                     // error number for errors detected by the thread

  PFOLFINDLASTUSED pLastUsed;                    // points to area with the last used values

  HWND        hwndTabCtrl;                       // handle of tab control
  HWND        hwndPages[5];                      // handle of tab dialogs 

  BATCHLISTENTRYDATA BatchEntryData;             // buffer for batchlist entry data
  BOOL        fAndFindInSource;                // TRUE = search additional string in source
  CHAR_W      szAndFindInSource[MAX_FINDCHANGE_LEN+1];   // additional string being looked for in source
  CHAR_W      szAndFindInSourceUpr[MAX_FINDCHANGE_LEN+1];  // uppercase version of szAndFindInSource
  CHAR_W      szAddSourceTempSeg[MAX_SEGMENT_SIZE]; // buffer for additional search in source segment
  BOOL        fAndFindInSourceLogicExpression;  // TRUE = "and find in source" string uses logical expression
  CHAR_W      szAndFindInExpression[2*MAX_FINDCHANGE_LEN];// buffer for find expression
  BOOL        fApplyBatchList;                  // true = apply a batch find/replace list
  int         iCurBatchEntry;                   // number of currently active entry in batch find/replace list
  OBJNAME     szCurDocObjName;                  // object name of currently processed document
  BOOL        fRespectLineFeeds;                // TRUE = break lines at line feeds, FALSE = ignore line feeds and show text as a single block
  int         iMinDialogWidth;                  // minimum width of dialog window
  int         iMinDialogHeight;                 // minimum heigth of dialog window
  BOOL        fCollapsed;                       // TRUE = options region of dilaog is in collapsed state
  SWP         swpLastValidSizePos;              // last valid dialog size and position
  RECT        rcOrgOptsAndDocsGB;               // original size and position of options and document groupbox
  RECT        rcOrgResultGB;                    // original size and position of results groupbox
  CHAR_W      szColTextBuffer[MAX_SEGMENT_SIZE*8]; // buffer for the complete text of a column
  CHAR_W      szChangeToModified[MAX_FINDCHANGE_LEN+1];   // copy of change to string with adjusted start character

  // current color settings
  COLORCHOOSERDATA ColorData;
  
  // font to be used for result area and othere controls
  HFONT hFontControl;
  LOGFONT     lf;                               // settings of currently used font
  BOOL      fPreserveCase;                      // true = preserve case of first character
} FOLFINDDATA, *PFOLFINDDATA;


/**********************************************************************/
/* Prototype section                                                  */
/**********************************************************************/
MRESULT EXPENTRY FolFindDlgProc( HWND, WINMSG, WPARAM, LPARAM );
MRESULT FolFindControl( HWND, SHORT, SHORT );
VOID FolFindFreeDoc( PVOID pvTargetDoc, PVOID pvSourceDoc );
BOOL FolFindMatch( PTBDOCUMENT pDoc, PTBDOCUMENT pSourceDoc, PFOLFINDDATA pIda );
static USHORT FolTMOpen ( PFOLFINDDATA pIda );
static USHORT FolTmClose( PFOLFINDDATA  pIda );
static BOOL FolTmReplace( PFOLFINDDATA  pIda, PSZ_W pSrcSeg, PSZ_W pTgtSeg );
BOOL FolFindUpdateTM( PFOLFINDDATA pIda );
BOOL FolFindCloseDoc( PFOLFINDDATA pIda, BOOL fFreeDoc );
BOOL FolCheckForChangedDoc
(
  PFOLFINDDATA pIda,
  PBOOL       pfRefreshed              // document-has-been-refreshed flag
);
BOOL GFR_OpenDoc
(
  PFOLFINDDATA pIda,
  BOOL fCheckLock,                     // check for locking
  BOOL fStartSearch,                   // start searching
  BOOL fContinueNext                   // continue with nect doc flag
);
USHORT GFR_FillDocListbox
(
  HWND        hwnd,                    // dialog window handle
  PFOLFINDDATA pIda                     // dialog IDA
);
SHORT GFR_NLFCmp
(
  PSZ_W   pData,
  PSZ_W   pSearch,
  PUSHORT pusLen
);

VOID
 GFR_UpdHistory
 (
   PSZ_W pszSource,              //  pIda->szFind,
   PSZ_W pszTarget,              // pFllProp->szFolFind,
   PFINDNAME pHistList           // pFllProp->szFindList[0] );
  );
BOOL GFR_MaskProtectedText
(
  PSZ_W       pszSegment,              // ptr to segment data
  PLOADEDTABLE pDocTagTable,           // ptr to loaded tag table for document
  ULONG       ulOemCodePage,           // OEM codepage of document
  BOOL        fProtected               // TRUE = segment is a protected one
);
void FolFind_PreparePattern
(
  PSZ_W       pszPattern               // pattern string being prepared
);
BOOL GFR_MatchPattern
(
  PSZ_W        pszMatch,               // string being compared
  PSZ_W        pszPattern,             // pattern for pattern matching
  PUSHORT      pusMatchlen,            // length of matching string
  BOOL        fWholeWordsOnly          // whole-words-only search flag
);
BOOL GFR_isWordDelimiter
(
  CHAR_W      chTest                   // character being tested
);
BOOL GFR_PreProcessFindString
(
  PSZ_W       pszFind,                 // specified find string (normally in uppercased form)
  PBOOL       pfLogicExpression,       // pointer to fLogicExpression flag, TRUE = logic expression
                                       //   has been used in find string
  PSZ_W       pszExpressionBuffer      // pointer to buffer for preprocessed expression
);
BOOL GFR_SearchString
(
  PSZ_W       pszData,                 // segment data being searched
  PSZ_W       pszPattern,              // string or pattern being seached for
  USHORT      usStartOffs,             // start offset to begin search
  PUSHORT     pusFoundPos,             // ptr to buffer for position of matching string
  PUSHORT     pusFoundLen,             // ptr to buffer for length of matching string
  BOOL        fWholeWordsOnly          // TRUE = search whole words only
);
BOOL GFR_EvaluateExpression
(
  PSZ_W       *ppszExpression,         // expression evaluation pointer
  PBOOL       pfResult,                // ptr to buffer fo evaluation result
  PSZ_W       pszSegment,              // ptr to segment data
  USHORT      usStartOffs,             // start offset to begin search
  PUSHORT     pusMatchPos,             // offset of matching string
  PUSHORT     pusMatchLen,             // length of matching string
  BOOL        fWholeWordsOnly,         // whole-words-only search flag
  PFOLFIND_FINDLIST pFindList          // list of find locations in current segment
 );
BOOL GFR_EvaluateSingleExpression
(
  PSZ_W       *ppszExpression,         // expression evaluation pointer
  PBOOL       pfResult,                // ptr to buffer fo evaluation result
  PSZ_W       pszSegment,              // ptr to segment data
  USHORT      usStartOffs,             // start offset to begin search
  PUSHORT     pusMatchPos,             // offset of matching string
  PUSHORT     pusMatchLen,             // length of matching string
  BOOL        fWholeWordsOnly,         // whole-words-only search flag
  PFOLFIND_FINDLIST pFindList          // list of find locations in current segment
);
BOOL GFR_CheckAndPrepareExpression
(
  PSZ_W       pszExpression,           // expresion being prepared and checked
  BOOL        fCaseIgnore              // case ignore flag (for uppercasing of search strings)
);
void GFR_SetColor
(
  HWND        hwndMLE,                 // handle of rich edit control
  int         iStart,                  // start pos for colored area
  int         iEnd,                    // end pos of colored area
  COLORREF    colorText,               // text color
  COLORREF    colorBackground          // background color
);

BOOL GFR_PrepFolderSearch( HWND hwnd, PFOLFINDDATA pIda );

/*! \brief Replace a commbobox with its Unicode enabled version
  \param hwndDlg handle of dialog window
  \param iComboBoxID numeric identifier of the control
  \returns TRUE when successful, or FALSE when the new control could not be created
*/
BOOL GFR_ReplaceWithUnicodeComboBox
( 
  HWND hwndDlg,                        // window handle of dialog window
  int  iComboBoxID                     // ID of combobox
);

/*! \brief Resize columns in the result control
  \param hwnd handle of dialog window
  \param pIda pointer to global find and replcace instance data area
  \returns TRUE 
*/
BOOL GFR_ResizeColumns
( 
  HWND        hwnd,                    // dialog window handle
  PFOLFINDDATA pIda                     // dialog IDA
);

/*! \brief Adjust column layout in the result control
  \param hwnd handle of dialog window
  \param pIda pointer to global find and replcace instance data area
  \returns TRUE 
*/
BOOL GFR_AdjustResultColumns
( 
  HWND        hwnd,                    // dialog window handle
  PFOLFINDDATA pIda                     // dialog IDA
);

/*! \brief Clear a result list
  Free all result entries of the list and set the used count to zero
  \param pIda pointer to global find and replace instance data area
  \returns TRUE
*/
BOOL GFR_ClearResultList( PFOLFINDDATA pIda );

/*! \brief Clear a result list
  Free all result entries of the list and set the used count to zero
  \param pIda pointer to global find and replace instance data area
  \returns TRUE
*/
BOOL GFR_GetLastUsedValues( PFOLFINDDATA pIda );


/*! \brief Add a single find result to the find list
  \param pIda pointer to global find and replace instance data area
  \param iDocIndex index of document containg the find result
  \param pszDocObjName document object name
  \param fTarget TRUE = result found in target, FALSE = result found in source
  \param ulSegNum number of the segment containing the find result
  \param fTranslated TRUE = segment is translated
  \param pszText array containing the text string pointers
  \param usOffs offset of found text within segment
  \param usLen length of found string
  \returns TRUE if successful 
*/
BOOL GFR_AddFindResultToList
( 
  PFOLFINDDATA pIda,                     // dialog IDA
  int         iDocIndex,                // index of document
  PSZ         pszDocObjName,            // document object name
  BOOL        fTarget,                  // TRUE = found in target, FALSE = found in source
  ULONG       ulSegNum,                 // number of segment
  BOOL        fTranslated,              // TRUE = segment is translated
  PSZ_W       pszText[MAX_NUM_OF_TEXTS],// table containing the text strings for this entry
  USHORT      usOffs,                   // offset within segment
  USHORT      usLen                     // length of found string
);

/*! \brief Thread doing the actual searching
  \param pvIda pointer to global find and replace instance data area
  \returns TRUE
*/
void GFR_SearchThread
(
  PVOID pvIda                     // dialog IDA
);

/*! \brief Start the search thread
  \param pIda pointer to global find and replace instance data area
  \returns TRUE
*/
USHORT GlobalFindStartThread
(
  PFOLFINDDATA pIda                     // dialog IDA
);

/*! \brief Reset the search position
  \param pIda pointer to global find and replace instance data area
  \returns TRUE
*/
VOID  GFR_ResetSearchPosition
( 
  PFOLFINDDATA pIda
);

// draw single text string and break text exceeding the right side boundary
void GFR_DrawText( PFOLFINDDATA pIda, HDC hdc, PRECT prcClip, PPOINT pPt, PSZ_W pszString, int iLen, int iLineHeight, int iColorBaseIndex, DRAWTYPE DrawType );

// draw multi line text
void GFR_DrawMultiLineText( PFOLFINDDATA pIda, HDC hdc, PRECT prcClip, PPOINT pPt, PSZ_W pszString, int iLen, int iLineHeight, int iColorBaseIndex, DRAWTYPE DrawType );

/*! \brief Draw a single line in the result listview control
  \param pIda pointer to global find and replace instance data area
  \param lpDrawItem structure containing the draw information
  \returns TRUE
*/
void GFR_DrawItem( PFOLFINDDATA pIda, LPDRAWITEMSTRUCT lpDrawItem );

/*! \brief Update the selected items part in the status bar
  \param pIda pointer to global find and replace instance data area
  \returns TRUE
*/
void GFR_UpdateSelectionStatus(   PFOLFINDDATA pIda );

BOOL ResizeBatchListColumns
( 
  HWND        hwnd,                    // dialog window handle
  PFOLFINDDATA pIda                     // dialog IDA
);

INT_PTR CALLBACK GFR_INTERACTIVE_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
INT_PTR CALLBACK GFR_BATCHLIST_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
INT_PTR CALLBACK GFR_BATCHLIST_ENTRY_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
BOOL GFR_PropertySheetLoad( HWND hwnd, PFOLFINDDATA pIda );
BOOL GFR_SaveLastUsedValues( PFOLFINDDATA pIda );
void GFR_BatchListRenumber( HWND hwnd, int iItem );
PFOLFINDBATCHLISTENTRY GFR_CreateBatchListEntry( PBATCHLISTENTRYDATA pData );
// add a batch list entry to the batch list table
BOOL GFR_AddBatchListEntry( PFOLFINDDATA pIda, PFOLFINDBATCHLISTENTRY pEntry, int iPos ); 
// clear the batch list table
BOOL GFR_ClearBatchList( PFOLFINDDATA pIda );
// get the (first) selected item of a listview control
int GFR_BatchListGetSelected( HWND hwndListView );
// add a batch list entry to the list view
int GFR_AddToBatchListView( HWND hwndListView, int iPos, PFOLFINDBATCHLISTENTRY pEntry );
// update an entry ion the batch list view
int GFR_UpdateBatchListViewEntry( HWND hwndListView, int iItem, PFOLFINDBATCHLISTENTRY pEntry );
// update enable state of batch list buttons
void GFR_UpdateBatchListButtons( PFOLFINDDATA pIda , HWND hwnd );
// import a batch list
BOOL GFR_ImportBatchList( PFOLFINDDATA pIda, HWND hwnd, PSZ pszListFile );
// export a batch list
BOOL GFR_ExportBatchList( PFOLFINDDATA pIda, HWND hwnd, PSZ pszListFile );
// export TM using the standard save as dialog
int GFR_ExportResultList( HWND hwndParent, PFOLFINDDATA pIda );

long _stdcall GFR_ListViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// prepare temporary segment data for searching
BOOL GFR_PrepareTempSeg( PFOLFINDDATA pIda, PSZ_W pszTempSeg, BOOL fProtected );

// refresh the history in a search field combobox
BOOL GFR_RefreshComboboxHistory( HWND hwnd, int iID, PFOLFINDDATA pIda, PSZ_W pszFieldValue, PFINDNAME pHistoryList );

// set and prepare the find and replace fields for the next search operation
BOOL GFR_SetSearchFields( PFOLFINDDATA pIda, HWND hwnd, PSZ_W pszFind, PSZ_W pszChangeTo, PSZ_W pszAndFindInSource, BOOL fErrorHandling );

// get the next non-empty entry from the batch list
int GFR_GetNextNonEmptyBatchListEntry( PFOLFINDDATA pIda, int iStartEntry );

// set enable state of "apply batch list" checkbox
BOOL GFR_SetApplyBatchListCheckBoxState( PFOLFINDDATA pIda );

// enable or disable the search entry fields depending on state of "apply batch list" checkbox
VOID GFR_EnableOrDisableEntryFields( HWND hwnd );

// set enabled state of result list buttons and the open button
VOID GFR_SetEnableStateResultListButtons( PFOLFINDDATA pIda );

// do a single change in the segment text
BOOL GFR_ReplaceTextInSegment( PFOLFINDDATA pIda, PTBDOCUMENT pDoc, ULONG ulSegNum, USHORT usOffs, USHORT usFindLen, PSZ_W pszChangeTo, USHORT *pusOffs );

// measure a item (handler for WM_MEASUREITEM message)
int GFR_MeasureItem( PFOLFINDDATA pIda, LPMEASUREITEMSTRUCT pMeasureItem );

// force a refresh of the height of the listbox items
int GFR_ForceRefreshOfItemHeights( PFOLFINDDATA pIda );

// draw a frame
int  GFR_DrawFrame( HDC hdc, LPRECT prc );

// replace a resource file created edit control with its Unicode enabled counterpart
HWND GFR_ReplaceWithUnicodeEditControl( HWND hwndDlg, int iID, HWND hwndInsertBehind );

// test if on-spot editing in the batch list is active
BOOL GFR_IsOnSpotEditingActive();

// end on-spot editing of a batch list entry
void GFR_EndOnSpotEditing();

// adjust the case of the first character of the replace string
CHAR_W GFR_AdjustCase( CHAR_W chOrgChar, CHAR_W chNewChar, BOOL fPreserveCase );
