/*! \file
	Description: Defines for Proof Read Import window

	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/


#include "OtmProofReadEntry.h"
#include "OtmProofReadList.h"

// default extension of proof read list files
#define EXT_OF_PROOFREADLIST "PRXML"

//// display column modes
//typedef enum _DISPLAY_COLUMNS_MODE { UNDEFINED_COLUMN_MODE = 0, ONLY_SOURCE_COLUMN_MODE, ONLY_TARGET_COLUMN_MODE, SOURCE_AND_TARGET_COLUMN_MODE } DISPLAY_COLUMNS_MODE;

// fixed width of first two columns
#define DOCNUM_COL_WIDTH 0x42
#define SEGNUM_COL_WIDTH 0x42

// text draw types
typedef enum _DRAWTYPE { NORMALTEXT_DRAWTYPE, CHANGED_DRAWTYPE, INSERTED_DRAWTYPE } DRAWTYPE;


// number of pushbuttons at the bottom of the window
#define NUM_OF_PRI_BUTTONS 2

// list of different text types

// base color index for the type of entry
#define PRI_MODIFIED_ENTRY     0            /* text in modified entry */
#define PRI_PROCESSED_ENTRY    9            /* text in modified entry */
#define PRI_UNCHANGED_ENTRY   18            /* text in unchanged entry */

// additional offset for the state of the entry (the value is the additional offset into the color table)
#define PRI_NORMAL_ENTRY     0              /* text in normal entry */
#define PRI_SELECTED_ENTRY   3              /* text in selected entry */  
#define PRI_FOCUS_ENTRY      6              /* text in entry with input focus */



// subtext defines (added to the main entry type gives the index into the color and name table)
#define PRI_NORMAL_TEXT         0
#define PRI_CHANGED_TEXT        1
#define PRI_INSERTED_TEXT       2

// structure containing the last used values of the proof read document import dialog
typedef struct _PROOFREADIMPORTLASTUSED
{
  EQF_SWP   swpSizePos;                         // dialog size and position
  COLORREF  aclrForeground[40];                 // array with user defined foreround colors
  COLORREF  aclrBackground[40];                 // array with user defined background colors
  BOOL      fShowUnchanged;                     // show unchanged entries flag
  BOOL      fShowModified;                      // show modified entries flag
  BOOL      fShowProcessed;                     // show processed entries flag
  BOOL      fProcessDoc;                        // process document flag
  BOOL      fProcessMem;                        // process memory flag
  CHAR      szSaveDir[MAX_PATH];                // last used direcory to save or load lists
  BOOL      fShowDifferences;                   // show differences flag
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
  //
  CHAR      szFiller[5980];                     // room for future enhancements
} PROOFREADIMPORTLASTUSED, *PPROOFREADIMPORTLASTUSED;

// fixed width of first two columns
#define CHECKBOX_COL_WIDTH 0x20
#define DOCNUM_COL_WIDTH 0x42
#define SEGNUM_COL_WIDTH 0x42

// tasks for proof read import dialog
#define INITDLG_TASK 400
#define ENABLE_TASK  401
#define REFRESH_TASK 402
#define UPDATEPROCESSPB_TASK 403
#define REFRESHITEM_TASK 404
#define SETFOCUS_TASK 405

typedef enum _CHANGETYPE 
{ 
  FOUND_TYPE,                // entry for a found string
  CHANGETO_TYPE,             // entry for a found string which can be changed to the change-to string
  CHANGED_TYPE,              // entry for a found string which has been changed to the change-to string by the user
  AUTOCHANGED_TYPE           // entry for a found string which has been changed automatically (the offsets of following entries have to been adjusted because of the changed string)
} CHANGETYPE;

// entry for the changes list
typedef struct _PROOFREADCHANGE
{
  USHORT usOffs;             // offset within segment
  USHORT usLen;              // length of changed string
  USHORT usType;
} PROOFREADCHANGE, *PPROOFREADCHANGE;

// maximum number of displayed changes within the segment text
#define MAX_PROOFREADCHANGES 40

// changes list
typedef struct _PROOFREADCHANGELIST
{
  int  iUsed;                // number of entries used in this list
  PROOFREADCHANGE aChanges[MAX_PROOFREADCHANGES]; // table of changes
} PROOFREADCHANGELIST, *PPROOFREADCHANGELIST;

/**********************************************************************/
/* Instance data area (IDA) of proof read window                      */
/**********************************************************************/
#pragma pack(4)
typedef struct _PROOFREADDATA
{
  std::vector<std::string> *pvProofReadXML;// vector with fully qualified names of proof read XML files
  HWND        hwnd;                    // handle of dialog window
  CHAR        szObjName[MAX_EQF_PATH]; // buffer for dialog object name
  CHAR        szStatusLine[2048];      // buffer for status line text
  HWND        hStatus;                 // handle of status bar
  HWND        hResultListBox;          // handle of the result list box
  SWP         swpDlg;                  // dialog size and position
  HWND        hwndButton[NUM_OF_PRI_BUTTONS];// handles of pushbuttons
  SHORT       sButtonWidth[NUM_OF_PRI_BUTTONS];  // widths of pushbuttons
  SHORT       sButtonHeight;           // height of first pushbutton
  int         iMinDialogWidth;         // minimum width of dialog window
  int         iMinDialogHeight;        // minimum heigth of dialog window
  SHORT       sBorderSize;             // X size of window border
  COLORCHOOSERDATA ColorData;
  CHAR        szFolLongName[MAX_PATH]; // long name of folder
  CHAR        szFolShortName[MAX_PATH];// short name of folder
  CHAR        szDocLongName[MAX_PATH];       // long name of current document
  CHAR        szDocShortName[MAX_FILESPEC];  // short name of current document
  CHAR        szDocObjName[MAX_EQF_PATH]; // buffer for document object names
  CHAR        szDocMemory[MAX_LONGFILESPEC]; // buffer for document TM name
  CHAR        szDocFormat[MAX_FILESPEC]; // buffer for name of document markup table
  CHAR        szDocSrcLng[MAX_LANG_LENGTH]; // buffer for document source lang
  CHAR        szDocTgtLng[MAX_LANG_LENGTH]; // buffer for document target lang
  CHAR        szAlias[MAX_LONGFILESPEC];  // alias for current document
  CHAR        szShortAlias[MAX_FILESPEC]; // short alias for current document
  PPROOFREADIMPORTLASTUSED pLastUsed;                    // points to area with the last used values
  PTBDOCUMENT pTargetDoc;              // ptr to loaded target document
  PTBDOCUMENT pSourceDoc;              // ptr to loaded source document
  BOOL        fTMUpdRequired;          // TRUE = TM update us required
  OtmMemory   *pMem;                   // open TM object
  CHAR        szNameBuffer[MAX_LONGFILESPEC*2];  // name buffer
  OBJNAME     szFolObjName;            // object name of folder
  ULONG       ulSegNum;                // number of active segment
  CHAR_W      szBuffer[1024];          // general purpose buffer
  HFONT hFontControl;
  int         aiListViewColWidth[10];            // current width of list view columns
  BOOL        fCollapsed;                       // TRUE = options region of dialog is in collapsed state
  CHAR_W      szColTextBuffer[MAX_SEGMENT_SIZE*8]; // buffer for the complete text of a column
  OtmProofReadList *pList;               // current list of proof read entries
  BOOL        fShowUnchanged;            // show unchanged entries flag
  BOOL        fShowModified;             // show modified entries flag
  BOOL        fShowProcessed;            // show processed entries flag
  BOOL        fProcessDoc;               // TRUE = update document
  BOOL        fProcessMem;                  // TRUE = update memory
  CHAR        szListFileDir[MAX_PATH];      // buffer for list file directory for save and load
  CHAR        szListFileName[MAX_PATH];     // buffer for list file name for save and load
  char        szCurTagTable[MAX_PATH];      // name of active markup, empty if no markup has been aciviated yet
  PLOADEDTABLE pTagTable;                   // tag table for current markup
  BYTE        bDiffInputBufer[30000L];      // input buffer for EQFBFindDiff function
  BYTE        bDiffTokenBuffer[10000];      // token buffer for EQFBFindDiff function
  CHAR        szCurLanguage[MAX_LANG_LENGTH]; // current language 
  SHORT       sLangID;                      // language ID for current language
  ULONG       ulOEMCP;                      // OEM CP for current language
  BOOL        fShowDifferences;             // show differences flag
  int         iProcessRuns;                 // number of auto-processings donw
  LOGFONT     lf;                           // settings of currently used font
} PROOFREADDATA, *PPROOFREADDATA;

BOOL PRI_PropertySheetLoad( HWND hwnd, PPROOFREADDATA pIda );
BOOL PRI_SaveLastUsedValues( PPROOFREADDATA pIda );
BOOL PRI_GetLastUsedValues( PPROOFREADDATA pIda );

MRESULT EXPENTRY ProofReadDlgProc( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );

BOOL ProofReadWindow( std::vector<std::string> *pvXMLFiles, HINSTANCE hDLL );

BOOL ProofReadProcessList( OtmProofReadList *pListIn, BOOL fProcessDocIn, BOOL fProcessMemIn, HWND hwndDialog, int iNumOfRuns  );
