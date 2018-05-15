//+----------------------------------------------------------------------------+
//| EQFFSRCH.CPP                                                               |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Fuzzy Search Function                                         |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//

#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TM               // Translation Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file
#include "eqfdde.h"               // batch mode definitions
#include "eqffol00.h"             // our .h stuff
#include "eqffol.id"              // our ID file
#include "eqftmi.h"               // TM internal definitions
#include "EQFHLOG.H"            // Translation Processor priv. include file
#include "eqflp.h"

#include "eqfutmdi.h"           // MDI utilities
#include "richedit.h"           // MDI utilities

#include "OtmProposal.h"
#include "core\memory\MemoryFactory.h"
#include "cxmlwriter.h"
#include "core\utilities\LogWriter.h"


// activate this define to show debug info in text area
#ifdef _DEBUG
//  #define SHOWDEBUG_INFO 
#endif

// activate the following define to active logging
//#define FUZZYSEGMENTSEARCHLOGGING


/**********************************************************************/
/* various sizes and other constant values                            */
/**********************************************************************/
#define MLE_BUFFER_SIZE    8096        // size of buffer for MLE text

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

// number of pushbuttons in push button row at the bottom of the dialog window
#define NUM_OF_FUZZYSEARCH_PB 8

// array of custom colors used by fuzzy search
static COLORREF CustomColors[16];

/**********************************************************************/
/* Tasks during find and change operations                            */
/**********************************************************************/
typedef enum _FINDTASK
{
  OPEN_DOC_TASK,
  SEARCH_FUZZY_TASK,
  APPLY_CHANGE_TASK,
  CLOSE_DOC_TASK,
  NEXT_DOC_TASK,
  STOP_TASK,
  INITDLG_TASK,
  SETFOCUS_TASK,
  RESIZE_DIALOG_TASK
} FINDTASK;


// structure for list of changes in the strings
typedef struct _FS_STARTSTOP
{
  USHORT           usStart;            // start offset 
  USHORT           usStop;             // stop offset 
  USHORT           usType;             // type of entry
} FS_STARTSTOP, *PFS_STARTSTOP;


// max. number of find locations per segment
#define MAX_FINDENTRIES 10

// structure containing a found fuzzy match
typedef struct _FOUNDFUZZYMATCH
{
  int         iWords;                            // number of words in segment source (w/o tags)
  int         iDiffWords;                        // number of different words (segment vs source w/o tags)
  int         iFolderID;                         // ID of document containing the segment
  int         iDocumentID;                       // ID of document containing the segment
  ULONG       ulSegNum;                          // segment number 
  PSZ_W       pszSegment;                        // segment in document (w/ inline tags)
  PSZ_W       pszSource;                         // source of fuzzy match (w/ inline tags)
  PSZ_W       pszTarget;                         // target of fuzzy match (w/ inline tags)
  PFS_STARTSTOP   pSegmentChanges;               // change list for segment data       
  PFS_STARTSTOP   pProposalChanges;              // change list for proposal data
  BOOL        fUsed;                             // match has been used flag
} FOUNDFUZZYMATCH, *PFOUNDFUZZYMATCH;

// list of different text types
// (the enum value Is used as index into the color and name tables - so pls do not change the order or remove entries)
typedef enum _FSTEXTTYPES
{
  FS_NORMAL_TEXT,                      // normal text
  FS_NORMAL_INSERTED_TEXT,             // inserted text
  FS_NORMAL_MODIFIED_TEXT,             // modified text 
  FS_SELECTED_TEXT,                    // normal text when item is selected
  FS_SELECTED_INSERTED_TEXT,           // inserted text when item is selected
  FS_SELECTED_MODIFIED_TEXT,           // inserted text when item is selected
  FS_FOCUS_TEXT,                       // normal text when item has the input focus
  FS_FOCUS_INSERTED_TEXT,              // inserted text when item has the input focus
  FS_FOCUS_MODIFIED_TEXT,              // modified text when item has the input focus
  FS_OPENED_TEXT,                      // normal text when item has beed opened
  FS_OPENED_INSERTED_TEXT,             // inserted text when item has been opened
  FS_OPENED_MODIFIED_TEXT,             // modified text when item has been opened    
  FS_UNUSED1_TEXT,                     // for future enhancements - currently no used
  FS_UNUSED2_TEXT,                     // for future enhancements - currently no used
  FS_UNUSED3_TEXT,                     // for future enhancements - currently no used
  FS_UNUSED4_TEXT,                     // for future enhancements - currently no used
  FS_UNUSED5_TEXT,                     // for future enhancements - currently no used
  FS_UNUSED6_TEXT                      // for future enhancements - currently no used
} FSTEXTTYPES;

// text types name (in same order as the FSTEXTTYPES enumeration
PSZ pszTextTypes[] = {"Normal text", "Inserted text", "Modified text", "Normal text when element is selected", "Inserted text when element is selected", "Modified text when element is selected", 
  "Normal text when element has the input focus", "Inserted text when element has the input focus", "Modified text when element has the input focus", "Normal text when element has been opened", 
  "Inserted text when element has been opened", "Modified text when element has been opened", NULL };

// default foreground and background colors (in same order as the FSTEXTTYPES enumeration)
//                                   Normal            Inserted          Modified
COLORREF aclrDefaultForeground[] = { RGB(0,0,0),       RGB(0,0,200),     RGB(0,50,50),      // normal element
                                     RGB(0,0,0),       RGB(0,0,200),     RGB(0,50,50),      // selected element 
                                     RGB(0,0,0),       RGB(0,0,200),     RGB(0,50,50),      // focus element 
                                     RGB(0,0,0),       RGB(0,0,200),     RGB(0,50,50) };    // opened element
COLORREF aclrDefaultBackground[] = { RGB(255,255,255), RGB(255,255,255), RGB(255,255,255),  // normal element
                                     RGB(190,190,190), RGB(190,190,190), RGB(190,190,190),  // selected element 
                                     RGB(250,250,5),   RGB(250,250,5),   RGB(250,250,5),    // focus element 
                                     RGB(140,240,140), RGB(140,240,140), RGB(140,240,140) };// opened element


//
// NameList functions and defines
//

// sorted table data
typedef struct _SORTEDTABLEENTRY
{
  int id;
  int offs;
} SORTEDTABLEENTRY, *PSORTEDTABLEENTRY;

// namelist structure
typedef struct _NAMELIST
{
  int nextId;
  int iNameSpaceUsed;
  int iNameSpaceSize;
  PSZ pNameSpace;
  int iSortedTableSize;
  int iSortedTableUsed;
  PSORTEDTABLEENTRY pSortedTable;
  int iOffsTableSize;
  int *pOffsTable;
} NAMELIST, *PNAMELIST;

PNAMELIST NameList_Create();
void NameList_Destroy( PNAMELIST pList );
PSZ NameList_GetName(  PNAMELIST pList, int id );
int NameList_AddName(  PNAMELIST pList, PSZ pszName );
int NameList_FindName(  PNAMELIST pList, PSZ pszName );
int NameList_AddToNamespace(  PNAMELIST pList, PSZ pszName );
int NameList_AddToSortedTable(  PNAMELIST pList, int id, int offs );
int NameList_AddToOffsTable(  PNAMELIST pList, int id, int offs );

//
// StringBuffer functions and defines
//
typedef PVOID STRINGBUFFERHANDLE;
STRINGBUFFERHANDLE StringBuffer_Create();
PSZ_W StringBuffer_AddString( STRINGBUFFERHANDLE hBuffer, PSZ_W pszString );
PBYTE StringBuffer_AddData( STRINGBUFFERHANDLE hBuffer, PBYTE pbData, LONG lDataLen );
void StringBuffer_Destroy( STRINGBUFFERHANDLE hBuffer );



/**********************************************************************/
/* Instance data area (IDA) for find/change dialog                    */
/**********************************************************************/
#pragma pack(4)


typedef struct _FSEARCHIDAX
{
  HWND        hwnd;                    // handle of dialog window
  HWND        hwndErrMsg;              // parent for error messages
  OBJNAME     szObjName;               // object name of dialog
  BOOL        fRegistered;             // 'dialog has been registered' flag
  BOOL        fFromFolderList;         // 'dialog called from folder list' flag
  BOOL        fFind;                   // TRUE = we are in find mode
  BOOL        fTerminate;              // TRUE = dialog is being terminated
  FINDTASK    CurTask;                 // currently performed task
  ULONG       ulSegNum;                // number of active segment
  ULONG       ulAddSegNum;                       // segment number in additional table
  ULONG       ulActiveTable;                     // current search table
  USHORT      usOffs;                  // position within current segment
  USHORT      usFoundOffs;             // position of found string within segment
  USHORT      usLine;                  // current line number within document
  HWND        hwndLB;                  // handle of listbox for document names
  USHORT      usHits;                  // number of hits found
  USHORT      usChanges;               // number of strings changed
  OtmMemory   *pMem;                       // open TM object
  PSZ         pTMIn;                   // input space for translation memory
  PSZ         pTMOut;                  // output space for translation memory
  BOOL        fTMUpdRequired;          // TRUE = TM update us required
  BOOL        fDocOpenActive;                    // TRUE if DocOpen has been activated
  BOOL        fSetFocusToResult;                 // TRUE force input focus to result listbox
  BOOL        fUseThaiFont;                      // use Thai font for our MLEs
  BOOL        fSearching;                        // TRUE = we are in search or change mode
  BOOL        fTranslTextOnly;                   // TRUE = search translatable text only
  SHORT       sBorderSize;                       // size of dialog border
  HWND        hwndButton[10];                    // handles of pushbuttons
  SHORT       sButtonWidth[10];                  // widths of pushbuttons
  SHORT       sButtonHeight;                     // height of first pushbutton
  SWP         swpDlg;                            // dialog size and position
  LONG        lMinWidth;                         // minimum dialog width
  LONG        lMinHeight;                        // minimum dialog height
  BOOL        fInitCompleted;                    // TRUE = dialog initalization has been completed
  ULONG       ulActFindSeg;                      // segment number of actual match
  ULONG       ulLastFindSeg;                     // segment number of last match
  CHAR_W      chWildCardSingleChar;              // wildcard for single characters
  CHAR_W      chWildCardMultChar;                // wildcard for multiple characters
  BOOL        fMultipleObjs;                     // TRUE: we are searchin in more than one folder
  PSZ         pszObjList;                        // points to list of objects being searched
  BOOL        fStopSearch;                       // TRUE = stop current search
  BOOL        fStoppedByUser;                    // TRUE = search stopped by user
  BOOL        fStoppedByError;                   // TRUE = search stopped by error message
  BOOL        fWithMarks;                        // TRUE = mark differencs in result list and exported list
 
  int         iSearchClass;                      // class of currently searched fuzzy matches
  int         iSearchMode;                       // currently selected search mode
  LONG        lItemHeight;                       // height of listbox items

  // fields containing info and data of currently active folder
  PSZ         pszActiveFolder;                   // currently searched folder in folder object list
  CHAR        szSubFolObjName[MAX_EQF_PATH];     // buffer for subfolder/folder object name
  OBJNAME     szFolObjName;                      // object name of folder (when processing a single folder)
  OBJNAME     szCurFolObjName;                   // object name of current folder
  CHAR        szFolLongName[MAX_LONGFILESPEC];   // long name of current folder
  OBJNAME     szLastFolObjName;                  // object name of previous folder

  // fields containing info and data of the current document
  CHAR        szAlias[MAX_LONGFILESPEC];         // alias for current document
  CHAR        szShortAlias[MAX_FILESPEC];        // short alias for current document
  CHAR        szDocObjName[MAX_EQF_PATH];        // buffer for document object names
  unsigned    DocTime;                           // Update time of STARGET document
  unsigned    DocDate;                           // Update date of STARGET document
  CHAR        szCurDoc[MAX_FILESPEC];            // name of currently processed document
  ULONG       ulSourceOemCP;                     // source OEM codepage
  ULONG       ulTargetOemCP;                     // source OEM codepage
  CHAR        szLongName[MAX_LONGFILESPEC];      // long name of current document
  CHAR        szDocMemory[MAX_LONGFILESPEC];     // buffer for document TM name
  CHAR        szDocShortMemory[MAX_FILESPEC];    // buffer for document TM name
  CHAR        szDocFormat[MAX_FILESPEC];         // buffer for name of document markup table
  CHAR        szDocSrcLng[MAX_LANG_LENGTH];      // buffer for document source lang
  CHAR        szDocTgtLng[MAX_LANG_LENGTH];      // buffer for document target lang
  PTBDOCUMENT pTBDoc;                            // ptr to loaded document
  SHORT       sTgtLangID;                        // morph ID of document target language
  SHORT       sSrcLangID;                        // morph ID of document source language
  SHORT       sDocCount;                         // number of documents in documents listbox

  // fields for the list of found fuzzy matches
  ULONG       ulNumOfMatches;                    // number of found matches
  ULONG       ulMatchTableSize;                  // size of found matches table (number of entries)
  PFOUNDFUZZYMATCH pMatchList;                   // list of found matches
  FOUNDFUZZYMATCH CurMatch;                      // buffer for current match
  PNAMELIST    pNameList;                        // name list for markup table and document names  
  PVOID        pvStringBuffer;                   // string buffer for segment data 
  int          iMaxExtent;                       // extent of drawn text


  // fields for memory lookup
  OtmMemory  *pFolMem[MAX_NUM_OF_FOLDER_MDB+1]; // array with folder memory DB objects
  CHAR   szFolMemNames[MAX_NUM_OF_FOLDER_MDB+1][MAX_LONGFILESPEC]; // name of folder memories 
  OtmMemory  *pDocMem;                           // document memory object
  CHAR   szDocMemName[MAX_LONGFILESPEC];         // name document memory
  CHAR   szNewDocMem[MAX_LONGFILESPEC];          // name of next document memory
  int    iNumOfFolMems;                          // number of folder memories
  BOOL   fStopAtFirstExact;                      // stop at first exact match flag from folder properties

  // buffer areas
  FS_STARTSTOP SegmentChangesList[1024];         // source difference list
  FS_STARTSTOP ProposalChangesList[1024];        // target difference list
  CHAR_W      szTempSeg[MAX_SEGMENT_SIZE];       // buffer for current segment
  CHAR_W      szTempProp[MAX_SEGMENT_SIZE];      // buffer for current proposal source segment
  CHAR        chTokBufStartStop[MAX_SEGMENT_SIZE*4];// token buffer for start/stop table
  CHAR        szBuffer[3000];                    // general purpose buffer
  CHAR_W      szBufferW[3000];                   // general purpose buffer
  CHAR        szNameBuffer[MAX_LONGFILESPEC];    // name buffer
  CHAR_W      szBestMatchSource[MAX_SEGMENT_SIZE];// buffer for source of best mathc
  CHAR_W      szBestMatchTarget[MAX_SEGMENT_SIZE];// buffer for taregt of best mathc
  CHAR        szExportFileName[MAX_LONGFILESPEC]; // file name for the export of the result list

  // logging
  LogWriter   *pLogWriter;                        // log writer object

  // current color settings
  COLORCHOOSERDATA ColorData;

  LOGFONT lf; // current font settings
  HFONT hFontControl;                             // currently activa draw font

} FSEARCHIDAX, *PFSEARCHIDAX;


/**********************************************************************/
/* Prototype section                                                  */
/**********************************************************************/
MRESULT EXPENTRY FSearchDlgProc( HWND, WINMSG, WPARAM, LPARAM );
MRESULT FSearchControl( HWND, SHORT, SHORT );
VOID FS_FreeDoc( PFSEARCHIDAX pIda, PVOID pvDoc );
BOOL FS_SearchFuzzy( PTBDOCUMENT pDoc, PFSEARCHIDAX pIda );

BOOL FS_CloseDoc
(
  PFSEARCHIDAX pIda,
  BOOL        fFreeDoc
);
BOOL FS_CheckForChangedDoc
(
  PFSEARCHIDAX pIda,
  PBOOL       pfRefreshed              // document-has-been-refreshed flag
);

BOOL FS_LoadDoc
(
  PFSEARCHIDAX pIda,
  BOOL fCheckLock,                     // check for locking
  BOOL fStartSearch,                   // start searching
  BOOL fContinueNext                   // continue with nect doc flag
);
USHORT FS_FillDocListbox
(
  HWND        hwnd,                    // dialog window handle
  PFSEARCHIDAX pIda                     // dialog IDA
);
SHORT FolNLFCmp
(
  PSZ_W   pData,
  PSZ_W   pSearch,
  PUSHORT pusLen
);

void FSearch_SetColor
(
  HWND        hwndMLE,                 // handle of rich edit control
  int         iStart,                  // start pos for colored area
  int         iEnd,                    // end pos of colored area
  COLORREF    colorText,               // text color
  COLORREF    colorBackground          // background color
);

BOOL FS_PrepFolderSearch( PFSEARCHIDAX pIda );
void FS_ClearResults( PFSEARCHIDAX pIda );
void FS_InitResults( PFSEARCHIDAX pIda );
int FS_AddToResultList( PFSEARCHIDAX pIda, PFOUNDFUZZYMATCH pMatch );
int CntGetProposalClass( PSZ_W pszSegment, PSZ_W pszProposal, PLOADEDTABLE pTable, PBYTE pbBufer, PBYTE pbTokBuf, SHORT sLangID, ULONG ulOemCP, PUSHORT pusWords );
void FS_ShowResult( PFSEARCHIDAX pIda, int iMatch );
void FS_MeasureItem( HWND hwnd, LONG lParam );
BOOL FS_DrawItem( PFSEARCHIDAX pIda, LONG lParam );
int FS_ComputeItemHeight( PFSEARCHIDAX pIda, HWND hwndDlg, int iControlID );
void FS_OpenDocInTenv( PFSEARCHIDAX pIda, PFOUNDFUZZYMATCH pMatch );
static SHORT FS_CloseFolderMemory( PFSEARCHIDAX pIda );
int FS_GetProposalDiffs( PSZ_W pszSegment, PSZ_W pszProposal, PLOADEDTABLE pTable, PBYTE pbBuffer, PBYTE pbTokBuf, SHORT sLangID, ULONG ulOemCP, 
                         PFS_STARTSTOP pSegmentChangesList, PLONG plSegmentChangesListLen, PFS_STARTSTOP pProposalChangesList, PLONG plProposalChangesListLen,
                         PBOOL fBelowThreshold );
int FS_DrawDifferences( PFSEARCHIDAX pIda, HDC hdc, PSZ_W pszText, PFS_STARTSTOP pStartStop, DWORD dwBackColor, FSTEXTTYPES TextTypeNormal, FSTEXTTYPES TextTypeInserted, FSTEXTTYPES TextTypeModified );
BOOL FS_ExportResults( PFSEARCHIDAX pIda );
void FS_SelectSearchMode( HWND hwnd, int iMode );

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolFuzzySearch                                        |
//+----------------------------------------------------------------------------+
BOOL FolFuzzySearch
(
  PSZ              pszFolObjName,      // folder object name
  BOOL             fFromFolderList,    // TRUE = called from folder list
  BOOL             fMultipleObjs       // TRUE = called with a list of folder object names
)
{
  BOOL        fOK = TRUE;              // internal O.K. flag
  PFSEARCHIDAX pIda;                    // ptr to IDA of dialog
  HWND        hwndFSearchDlg;          // handle of global-find-and-change dialog

  // Allocate IDA of fuzzy search dialog
  size_t iSize = sizeof(FSEARCHIDAX);
  pIda = (PFSEARCHIDAX)malloc( iSize );
  if ( pIda != NULL )
  {
    memset( pIda, 0, iSize );
  }
  else
  {
    UtlError( ERROR_NOT_ENOUGH_MEMORY, MB_CANCEL, 0, NULL, EQF_ERROR );
    fOK = FALSE;
  } /* end */     

  // Fill-in IDA fields                                               
  if ( fOK )
  {
    pIda->fFromFolderList = fFromFolderList;
    pIda->fMultipleObjs = fMultipleObjs;
    if ( pIda->fMultipleObjs )
    {
      pIda->pszObjList = pszFolObjName;
      pIda->pszActiveFolder = pIda->pszObjList;
      strcpy( pIda->szSubFolObjName, pIda->pszActiveFolder );
    }
    else
    {
      strcpy( pIda->szSubFolObjName, pszFolObjName );
    } /* endif */

    if ( FolIsSubFolderObject( pIda->szSubFolObjName ) )
    {
      // convert pszFolObjName to folder object name by cutting the
      // subfolder part and property part from the name
      strcpy( pIda->szFolObjName, pszFolObjName );
      UtlSplitFnameFromPath( pIda->szFolObjName );
      UtlSplitFnameFromPath( pIda->szFolObjName );
    }
    else
    {
      // pszFolObjName contains the folder object name already
      strcpy( pIda->szFolObjName, pIda->szSubFolObjName );
    } /* endif */
  } /* endif */

  // Start fuzzy search dialog
  if ( fOK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      hwndFSearchDlg = CreateMDIDialogParam( hResMod, MAKEINTRESOURCE(ID_FUZZYSEARCH_DLG),
        (HWND)UtlQueryULong( QL_TWBCLIENT ), (FARPROC)FSearchDlgProc, MP2FROMP(pIda), TRUE,
        (HPOINTER) UtlQueryULong(QL_DICTENTRYDISPICO)); //hiconDICTDISP );

  } /* endif */

  return( fOK );
} /* end of function FolFuzzySearch */




//
//
//   FSearchDlgProc
//
//
//

MRESULT EXPENTRY FSearchDlgProc
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  PFSEARCHIDAX pIda = NULL;             // ptr to IDA of dialog
  BOOL        fOK;                     // internal O.K. flag
  BOOL        fStopProcess;            // stop current process flag


  switch (msg)
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_FUZZYSEARCH_DLG, mp2 ); break;

    case WM_INITDLG:
      {
        BOOL fLastUsedPositionAvailable = FALSE;

        fOK = TRUE;



        /**************************************************************/
        /* Anchor IDA and register dialog                             */
        /**************************************************************/
        if ( fOK )
        {
          pIda = (PFSEARCHIDAX)PVOIDFROMMP2(mp2);

          // get dialog position and size
          WinQueryWindowPos( hwnd, &pIda->swpDlg );
          ANCHORDLGIDA( hwnd, pIda );
          pIda->hwndErrMsg = pIda->hwnd = hwnd;

          // build unique dialog object name
          sprintf( pIda->szObjName, "FUZZYSEARCH:%s", pIda->szFolObjName );

          // register dialog
          EqfRegisterObject( pIda->szObjName, hwnd, clsFUZZYSEARCH );
          pIda->fRegistered = TRUE;

          UtlRegisterModelessDlg( hwnd );

#ifdef FUZZYSEGMENTSEARCHLOGGING
          LogWriter::registerLogFile( "FuzzySearch", "Fuzzy segment search processing log" );
          pIda->pLogWriter = new LogWriter();
          pIda->pLogWriter->write( "Info: processing WM_INITDLG" );
#else
          pIda->pLogWriter = NULL;
#endif

        } /* endif */


        // fill class combobox
        {
          PSZ pszClassNames[] = { "Class-0 (0 words difference)", "Class-1 (1 word difference)", "Class-2 (2 words difference)",
                                  "Class-3 (3 words difference)", "Class-4 (4 words difference)", "Class-5 (5 words difference)", 
                                  "Class-6 (6 or more words difference)", NULL };

          int i = 0;
          while ( pszClassNames[i] != NULL )
          {
            int iItem = CBINSERTITEMEND( hwnd, ID_FUZZYSEARCH_CLASS_CB, pszClassNames[i] );
            CBSETITEMHANDLE( hwnd, ID_FUZZYSEARCH_CLASS_CB, iItem, i );
            i++;
          } /* endwhile */             
          CBSELECTITEM( hwnd, ID_FUZZYSEARCH_CLASS_CB, 0 );
        }

        // fill mode combobox
        {
          PSZ pszModeNames[] = { "With selected class or higher", "Having the selected class only", "Up to and including the selected class", NULL };
          int aiModeValues[] = { SELECTEDCLASSANDHIGHER_MODE, ONLYSELECTEDCLASS_MODE, UPTOSELECTEDCLASS_MODE, 0 };

          int i = 0;
          while ( pszModeNames[i] != NULL )
          {
            int iItem = CBINSERTITEMEND( hwnd, ID_FUZZYSEARCH_MODE_CB, pszModeNames[i] );
            CBSETITEMHANDLE( hwnd, ID_FUZZYSEARCH_MODE_CB, iItem, aiModeValues[i] );
            i++;
          } /* endwhile */             
          CBSELECTITEM( hwnd, ID_FUZZYSEARCH_MODE_CB, 0 );
        }
        
        // fill color data area with default values
        pIda->ColorData.hwndOwner = hwnd;
        pIda->ColorData.sID = 0;
        strcpy( pIda->ColorData.szTitle, "Set Fuzzy Segment Search Colors" );
        int i = 0;
        while( pszTextTypes[i] != NULL ) 
        {
          strcpy( pIda->ColorData.ColorSetting[i].szElement, pszTextTypes[i] );
          pIda->ColorData.ColorSetting[i].cForeground = aclrDefaultForeground[i];
          pIda->ColorData.ColorSetting[i].cDefaultForeground = aclrDefaultForeground[i];
          pIda->ColorData.ColorSetting[i].cBackground = aclrDefaultBackground[i];
          pIda->ColorData.ColorSetting[i].cDefaultBackground = aclrDefaultBackground[i];
          i++;
        } /* endwhile */

        /**************************************************************/
        /* Set initial state of dialog controls                       */
        /**************************************************************/
        if ( fOK )
        {
          ENABLECTRL( hwnd, ID_FUZZYSEARCH_OPEN_PB,          FALSE );
          ENABLECTRL( hwnd, ID_FUZZYSEARCH_EXPORT_PB,          FALSE );
        } /* endif */

          // prepare font information
        if ( fOK )
        {
          // this code has been borrowed from function SetCtrlFont in file EQFOSWIN.C
          HFONT   hFontDlg;

          hFontDlg = (HFONT)SendMessage( hwnd, WM_GETFONT, 0, 0L );
          if ( hFontDlg != NULL )
          {
            if ( GetObject( hFontDlg, sizeof(LOGFONT), (LPSTR) &(pIda->lf) ) )
            {
              pIda->lf.lfCharSet  = (UCHAR)GetCharSet();
              if (pIda->lf.lfHeight > 0 )
              {
                pIda->lf.lfHeight -=  SHEIGHTINCTRL;
              }
              else
              {
                pIda->lf.lfWeight = FW_NORMAL;
              } /* endif */
              pIda->lf.lfOutPrecision = OUT_TT_PRECIS;
              strcpy( pIda->lf.lfFaceName, "Arial" );
            }
          }
        }

         if ( fOK )
         {
          EQFINFO     ErrorInfo;       // error code of property handler calls
          PPROPFOLDERLIST pFllProp;    // ptr to folder list properties
          PVOID       hFllProp;        // handle of folder list properties
          OBJNAME     szFllObjName;    // buffer for folder list object name

          UtlMakeEQFPath( szFllObjName, NULC, SYSTEM_PATH, NULL );
          strcat( szFllObjName, BACKSLASH_STR );
          strcat( szFllObjName, DEFAULT_FOLDERLIST_NAME );
          hFllProp = OpenProperties( szFllObjName, NULL, PROP_ACCESS_READ, &ErrorInfo );
          if ( hFllProp )
          {
            pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFllProp );

            // use remembered size and position (if any available
            {
              SWP swpMLE, swpButton, swpDispOptions;
              RECT rect;
              LONG lMinSize;

              if ( pFllProp->swpFSearchSizePos.cx != 0 )
              {
                EQFSWP2SWP( pFllProp->swpFSearchSizePos, pIda->swpDlg );
                fLastUsedPositionAvailable = TRUE;
              }
              else
              {
                memset( &(pIda->swpDlg), 0, sizeof(pIda->swpDlg) );
                memset( &pFllProp->swpFSearchSizePos, 0, sizeof(pFllProp->swpFSearchSizePos) );
                WinQueryWindowPos( hwnd, &(pIda->swpDlg) );
              } /* endif */

              WinQueryWindowPos( GetDlgItem( hwnd, ID_FUZZYSEARCH_FIND_PB ), &swpButton );
              WinQueryWindowPos( GetDlgItem( hwnd, ID_FUZZYSEARCH_RESULT_LB ), &swpMLE );

              GetWindowRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_DOCS_GB ), &rect );
              MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );

              WinQueryWindowPos( GetDlgItem( hwnd, ID_FUZZYSEARCH_DISPLAYOPT_GB ), &swpDispOptions );

              // ensure dialog size falls not below its minimum
              pIda->lMinWidth = lMinSize = swpDispOptions.x + swpDispOptions.cx + 5;
              if ( pIda->swpDlg.cx < lMinSize )
              {
                pIda->swpDlg.cx = (SHORT)lMinSize;
              } /* endif */

              pIda->lMinHeight = lMinSize = rect.bottom + swpMLE.cy + swpButton.cx + 10;
              if ( pIda->swpDlg.cy < lMinSize )
              {
                pIda->swpDlg.cy = (SHORT)lMinSize;
              } /* endif */

            }

            // select last used values
            FS_SelectSearchMode( hwnd, pFllProp->iFSLastUsedMode );
            CBSELECTITEM( hwnd, ID_FUZZYSEARCH_CLASS_CB, pFllProp->iFSLastUsedClass );
            strcpy( pIda->szExportFileName, pFllProp->szFSLastExportFile );
            if ( pFllProp->fFSWithMarks ) SETCHECK_TRUE( hwnd, ID_FUZZYSEARCH_SHOWDIFF_CHK );

            // copy any last used color settings
            if ( pFllProp->aclrFSLastUsedBackground[0] != pFllProp->aclrFSLastUsedForeground[0] )
            {
              int iMaxEntries = sizeof(pFllProp->aclrFSLastUsedBackground)/sizeof(COLORREF);
              for( int i = 0; i < iMaxEntries; i++ )
              {
                pIda->ColorData.ColorSetting[i].cForeground = pFllProp->aclrFSLastUsedForeground[i];
                pIda->ColorData.ColorSetting[i].cBackground = pFllProp->aclrFSLastUsedBackground[i];
              } /* endfor */
            } /* endif */

            if ( pFllProp->szFSFontFaceName[0] != 0 )
            {
              // get last used font settings
              memset( &(pIda->lf), 0, sizeof(pIda->lf) );
              pIda->lf.lfHeight = pFllProp->lFSFontHeight;
              pIda->lf.lfWidth = pFllProp->lFSFontWidth;
              pIda->lf.lfEscapement = pFllProp->lFSFontEscapement;
              pIda->lf.lfOrientation = pFllProp->lFSFontOrientation;
              pIda->lf.lfWeight = pFllProp->lFSFontWeight;
              pIda->lf.lfItalic = pFllProp->bFSFontItalic;
              pIda->lf.lfUnderline = pFllProp->bFSFontUnderline;
              pIda->lf.lfStrikeOut = pFllProp->bFSFontStrikeOut;
              pIda->lf.lfCharSet = pFllProp->bFSFontCharSet;
              pIda->lf.lfOutPrecision = pFllProp->bFSFontOutPrecision;
              pIda->lf.lfClipPrecision = pFllProp->bFSFontClipPrecision;
              pIda->lf.lfQuality = pFllProp->bFSFontQuality;
              pIda->lf.lfPitchAndFamily = pFllProp->bFSFontPitchAndFamily;
              strcpy( pIda->lf.lfFaceName, pFllProp->szFSFontFaceName );
            } /* endif */


            CloseProperties( hFllProp, PROP_QUIT, &ErrorInfo );
          } /* endif */
        } /* endif */


        // Create invisible listbox for document names                
        if ( fOK ) pIda->hwndLB = WinCreateWindow( hwnd, WC_LISTBOX, "", 0L, 0, 0, 0, 0, hwnd, HWND_TOP, 1, NULL, NULL );


        // get handles and sizes of dialog controls
        if ( fOK )
        {
          int i = 0;

          // get handles and sizes at bottom of dialog window
          pIda->hwndButton[0] = GetDlgItem( hwnd, ID_FUZZYSEARCH_FIND_PB );
          pIda->hwndButton[1] = GetDlgItem( hwnd, ID_FUZZYSEARCH_STOP_PB );
          pIda->hwndButton[2] = GetDlgItem( hwnd, ID_FUZZYSEARCH_OPEN_PB );
          pIda->hwndButton[3] = GetDlgItem( hwnd, ID_FUZZYSEARCH_EXPORT_PB );
          pIda->hwndButton[4] = GetDlgItem( hwnd, ID_FUZZYSEARCH_COLOR_PB );
          pIda->hwndButton[5] = GetDlgItem( hwnd, ID_FUZZYSEARCH_FONT_PB );
          pIda->hwndButton[6] = GetDlgItem( hwnd, ID_FUZZYSEARCH_CANCEL_PB );
          pIda->hwndButton[7] = GetDlgItem( hwnd, ID_FUZZYSEARCH_HELP_PB );
          for ( i = 0; i < NUM_OF_FUZZYSEARCH_PB; i++ )
          {
            RECT rect;
            GetWindowRect( pIda->hwndButton[i], &rect );
            pIda->sButtonWidth[i] = (SHORT)(rect.right - rect.left);
            if ( i == 0 ) pIda->sButtonHeight = (SHORT)(rect.bottom - rect.top);
          } /* endfor */
        } /* endif */

        if ( fOK )
        {
          ENABLECTRL( hwnd, ID_FUZZYSEARCH_STOP_PB, FALSE );
          ENABLECTRL( hwnd, ID_FUZZYSEARCH_OPEN_PB, FALSE );
          ENABLECTRL( hwnd, ID_FUZZYSEARCH_EXPORT_PB, FALSE );
        } /* endif */

        // create font to be used for result area
        pIda->hFontControl = CreateFontIndirect( &(pIda->lf) );

        // Show the dialog window                                     
        if ( fOK )
        {
          SWP  swpTWB;

          // Ensure that dialog is not outside of the TWB
          UtlKeepInTWB( &pIda->swpDlg );

          // Center dialog within TWB
          if ( !fLastUsedPositionAvailable )
          {
            WinQueryWindowPos( (HWND)UtlQueryULong( QL_TWBCLIENT ), &swpTWB );
            if ( (pIda->swpDlg.x > 0) && ((pIda->swpDlg.x + pIda->swpDlg.cx) < swpTWB.cx) )
            {
              pIda->swpDlg.x = (swpTWB.cx - pIda->swpDlg.cx) / 2;
            } /* endif */
            if ( (pIda->swpDlg.y > 0) && ((pIda->swpDlg.y + pIda->swpDlg.cy) < swpTWB.cy) )
            {
              pIda->swpDlg.y = (swpTWB.cy - pIda->swpDlg.cy) / 2;
            } /* endif */
          } /* endif */

          // postpone sizing of dialog window
          WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(INITDLG_TASK), 0L );
        }
        if ( !fOK )
        {
          WinDestroyWindow( hwnd );
          return 0;
        } /* endif */
      }

      fOK = FS_PrepFolderSearch( pIda );

      return MRFROMSHORT(TRUE);


 //case WM_SETFOCUS:
 //   {
 //     pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );
 //     if ( pIda != NULL )
 //     {
 //       if ( pIda->fSetFocusToResult )
 //       {
 //         pIda->fSetFocusToResult = FALSE;
 //         SETFOCUS( hwnd, ID_FUZZYSEARCH_RESULT_LB );
 //         //WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SETFOCUS_TASK), 0L );
 //         return(TRUE);
 //       } /* endif */;                 
 //     } /* endif */
 //   }
 //   return WinDefDlgProc( hwnd, msg, mp1, mp2 );
 //   break;

    case WM_COMMAND:
      pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case ID_FUZZYSEARCH_MODE_CB:
          {
            SHORT sCommand = WMCOMMANDCMD( mp1, mp2 );
            if ( sCommand == CBN_SETFOCUS )
            {
              if ( pIda->pLogWriter ) pIda->pLogWriter->write( "Info: Input focus is on mode combo box" );
              if ( pIda->fSetFocusToResult )
              {
                if ( pIda->pLogWriter ) pIda->pLogWriter->write( "Info: Posting SETFOCUS_TASK in order to pass focus to result listbox" );
                pIda->fSetFocusToResult = FALSE;
                //SETFOCUS( hwnd, ID_FUZZYSEARCH_RESULT_LB );
                WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SETFOCUS_TASK), 0L );
                return(TRUE);
              } /* endif */;                 
            } /* endif */
          }
          break;

        case ID_FUZZYSEARCH_SHOWDIFF_CHK:
          {
            BOOL fNewState = QUERYCHECK( hwnd, ID_FUZZYSEARCH_SHOWDIFF_CHK );
            if ( fNewState != pIda->fWithMarks )
            {
              pIda->fWithMarks = fNewState;
              InvalidateRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_RESULT_LB ), NULL, FALSE ); 
            } /* endif */
          }
          break;

        case ID_FUZZYSEARCH_RESULT_LB:
          {
            SHORT sCommand = WMCOMMANDCMD( mp1, mp2 );
            if ( sCommand == LN_ENTER )
            {
              SHORT sSelected = QUERYSELECTION( hwnd, ID_FUZZYSEARCH_RESULT_LB );
              if ( sSelected >= 0 )
              {
                int iMatch = QUERYITEMHANDLE( hwnd, ID_FUZZYSEARCH_RESULT_LB, sSelected );
                FS_OpenDocInTenv( pIda, &(pIda->pMatchList[iMatch]) );
                sSelected += 1;
                if ( sSelected < QUERYITEMCOUNT( hwnd, ID_FUZZYSEARCH_RESULT_LB ) )
                {
                  SELECTITEM( hwnd, ID_FUZZYSEARCH_RESULT_LB, sSelected );
                  WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SETFOCUS_TASK), 0L );
                } /* endif */                   
              } /* endif */               
            }
            else if ( sCommand == LBN_SELCHANGE )
            {
              SHORT sItem = QUERYSELECTION( hwnd, ID_FUZZYSEARCH_RESULT_LB );
              ENABLECTRL( hwnd, ID_FUZZYSEARCH_OPEN_PB, (sItem >= 0) );
            } /* endif */               
          }
          break;

        case ID_FUZZYSEARCH_HELP_PB:
          UtlInvokeHelp();
          break;

        case DID_CANCEL:
        case ID_FUZZYSEARCH_CANCEL_PB:
          pIda->fTerminate = TRUE;
          WinPostMsg( hwnd, WM_CLOSE, 0L, 0 );
          break;

        case ID_FUZZYSEARCH_STOP_PB:
          pIda->fStopSearch = TRUE;
          pIda->fStoppedByUser = TRUE;
          break;

        case ID_FUZZYSEARCH_EXPORT_PB:
          if ( pIda->fSearching )
          {
            break;
          } /* endif */
          FS_ExportResults( pIda );
          break;

        case ID_FUZZYSEARCH_COLOR_PB:
          {
            if ( UtlColorChooserDlg( &(pIda->ColorData) ) == 0 )
            {
              // refresh result list
              InvalidateRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_RESULT_LB ), NULL, TRUE );
            } /* endif */
          } 
          break;

        case ID_FUZZYSEARCH_FONT_PB:
          {
            CHOOSEFONT ChooseFontData;

            memset( &ChooseFontData, 0, sizeof(ChooseFontData) );
            ChooseFontData.lStructSize = sizeof(ChooseFontData);
            ChooseFontData.hwndOwner = hwnd;
            ChooseFontData.Flags = CF_FORCEFONTEXIST | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
            ChooseFontData.lpLogFont = &(pIda->lf);

            if ( ChooseFont( &ChooseFontData ) )
            {
              // free any previously created font
              if ( pIda->hFontControl ) DeleteObject( pIda->hFontControl );

              // create new font to be used
              pIda->hFontControl = CreateFontIndirect( &(pIda->lf) );

              // compute new item height
              pIda->lItemHeight = FS_ComputeItemHeight( pIda, hwnd, ID_FUZZYSEARCH_RESULT_LB );
              SendDlgItemMessage( hwnd, ID_FUZZYSEARCH_RESULT_LB, LB_SETITEMHEIGHT, 0, pIda->lItemHeight );

              // refresh result list
              InvalidateRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_RESULT_LB ), NULL, TRUE );
            } /* endif */
          }
          break;

        case ID_FUZZYSEARCH_FIND_PB:
          fOK = TRUE;
 
          // leave search if we are already searching
          if ( pIda->fSearching )
          {
            break;
          } /* endif */

          if ( pIda->pLogWriter ) pIda->pLogWriter->write( "Info: Starting search..." );

          // compute listbox item height if not done yet
          if ( pIda->lItemHeight == 0 )
          {
            pIda->lItemHeight = FS_ComputeItemHeight( pIda, hwnd, ID_FUZZYSEARCH_RESULT_LB );

          } /* endif */             

          // get currently selected search class
          {
            int iSelectedItem = CBQUERYSELECTION( hwnd, ID_FUZZYSEARCH_CLASS_CB );
            pIda->iSearchClass = CBQUERYITEMHANDLE( hwnd, ID_FUZZYSEARCH_CLASS_CB, iSelectedItem );
          }

          // get currently selected search mode
          {
            int iSelectedItem = CBQUERYSELECTION( hwnd, ID_FUZZYSEARCH_MODE_CB );
            pIda->iSearchMode = CBQUERYITEMHANDLE( hwnd, ID_FUZZYSEARCH_MODE_CB, iSelectedItem );
          }

          pIda->fWithMarks = QUERYCHECK( hwnd, ID_FUZZYSEARCH_SHOWDIFF_CHK );

          FS_ClearResults( pIda );
          FS_InitResults( pIda );
          pIda->iMaxExtent = 0;

          pIda->fSearching = TRUE;
          pIda->fStopSearch = FALSE;
          pIda->fStoppedByUser = FALSE; 
          pIda->fStoppedByError = FALSE; 
          ENABLECTRL( hwnd, ID_FUZZYSEARCH_FIND_PB, FALSE );
          ENABLECTRL( hwnd, ID_FUZZYSEARCH_OPEN_PB, FALSE );
          ENABLECTRL( hwnd, ID_FUZZYSEARCH_EXPORT_PB, FALSE );
          ENABLECTRL( hwnd, ID_FUZZYSEARCH_STOP_PB, TRUE );
          SETTEXT( hwnd, ID_FUZZYSEARCH_STATUS_TEXT, "Starting search..." );
          pIda->sDocCount =  QUERYITEMCOUNT( hwnd, ID_FUZZYSEARCH_DOCS_LB );
          WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(NEXT_DOC_TASK), 0L );
          break;

        case ID_FUZZYSEARCH_OPEN_PB :
          {
            SHORT sSelected = QUERYSELECTION( hwnd, ID_FUZZYSEARCH_RESULT_LB );
            if ( sSelected >= 0 )
            {
              int iMatch = QUERYITEMHANDLE( hwnd, ID_FUZZYSEARCH_RESULT_LB, sSelected );

              SETFOCUS( hwnd, ID_FUZZYSEARCH_RESULT_LB );
              pIda->fSetFocusToResult = TRUE;
              FS_OpenDocInTenv( pIda, &(pIda->pMatchList[iMatch]) );
              sSelected += 1;
              if ( sSelected < QUERYITEMCOUNT( hwnd, ID_FUZZYSEARCH_RESULT_LB ) )
              {
                SELECTITEM( hwnd, ID_FUZZYSEARCH_RESULT_LB, sSelected );
                //WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SETFOCUS_TASK), 0L );
              } /* endif */                   
            } /* endif */               
          }
          break;

      } /* endswitch */
      return 0;

    case WM_EQF_PROCESSTASK:
      pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );
      fOK = TRUE;
      if ( (pIda != NULL) && (!pIda->fTerminate) )
      {
        switch ( SHORT1FROMMP1(mp1) )
        {
          case INITDLG_TASK :
            {
              HWND hwndFrame = GetParent( hwnd );
              WinSetWindowPos( hwndFrame, HWND_TOP, pIda->swpDlg.x, pIda->swpDlg.y, pIda->swpDlg.cx, pIda->swpDlg.cy,
                               EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_SHOW | EQF_SWP_ACTIVATE );
              pIda->fInitCompleted = TRUE;
            }
            break;

          case OPEN_DOC_TASK :
            if ( pIda->pLogWriter ) pIda->pLogWriter->write( "Info: Processing task: OPEN_DOC_TASK" );
            fOK = FS_LoadDoc( pIda, TRUE, TRUE, TRUE );
            break;

          case SEARCH_FUZZY_TASK :
            // search fuzzy matches
            {
              BOOL fDocIsDone = FALSE;
              if ( pIda->pLogWriter ) pIda->pLogWriter->write( "Info: Processing task: SEARCH_FUZZY_TASK" );
              fDocIsDone = FS_SearchFuzzy( pIda->pTBDoc, pIda );

              UtlDispatch();
              pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );
              if ( (pIda != NULL) && !pIda->fTerminate )
              {
                if ( fDocIsDone || pIda->fStopSearch )
                {
                  WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(CLOSE_DOC_TASK), 0L );
                }
                else
                {
                  WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SEARCH_FUZZY_TASK), 0L );
                } /* endif */                   
              } /* endif */
            }
            break;


          case CLOSE_DOC_TASK :
            /**********************************************************/
            /* Close current doc (and save document if necessary)     */
            /**********************************************************/
            if ( pIda->pLogWriter ) pIda->pLogWriter->write( "Info: Processing task: CLOSE_DOC_TASK" );

            fStopProcess = FS_CloseDoc( pIda, TRUE );

            if ( fStopProcess || pIda->fStopSearch )
            {
              WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(STOP_TASK), 0L );
            }
            else
            {
              UtlDispatch();
              pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );
              if ( (pIda != NULL) && !pIda->fTerminate )
              {
                DELETEITEM( hwnd, ID_FUZZYSEARCH_DOCS_LB, 0 );
                WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(NEXT_DOC_TASK), 0L );
              } /* endif */
            } /* endif */
            break;

          case NEXT_DOC_TASK :
            {
              SHORT sDocsInLB = 0;

              /**********************************************************/
              /* Get next document from listbox                         */
              /**********************************************************/
              if ( pIda->pLogWriter ) pIda->pLogWriter->write( "Info: Processing task: NEXT_DOC_TASK" );
              sDocsInLB = QUERYITEMCOUNT( hwnd, ID_FUZZYSEARCH_DOCS_LB );
              if ( sDocsInLB > 0 )
              {
                SHORT sNextTask; 
                /********************************************************/
                /* make next document the active one                    */
                /********************************************************/
                {
                  SHORT sIndex = (SHORT)QUERYITEMHANDLE( hwnd, ID_FUZZYSEARCH_DOCS_LB, 0 );
                  if ( sIndex >= 0 )
                  {
                    QUERYITEMTEXTHWND( pIda->hwndLB, sIndex, pIda->szCurDoc );
                    if ( pIda->pLogWriter ) pIda->pLogWriter->writef( "Info: Next document is %s", pIda->szCurDoc  );
                    SELECTITEM( hwnd, ID_FUZZYSEARCH_DOCS_LB, 0 );
                    if ( pIda->fMultipleObjs )
                    {
                      // get folder object name for this document
                      PSZ pszFolder = (PSZ)QUERYITEMHANDLEHWND( pIda->hwndLB, sIndex );
                      if ( pszFolder )
                      {
                        strcpy( pIda->szCurFolObjName, pszFolder );
                      } /* endif */
                    }
                    else
                    {
                      strcpy( pIda->szCurFolObjName, pIda->szFolObjName );
                    } /* endif */
                    sNextTask = OPEN_DOC_TASK;
                  }
                  else
                  {
                    // skip folder speperator
                    DELETEITEM( hwnd, ID_FUZZYSEARCH_DOCS_LB, 0 );
                    sNextTask = NEXT_DOC_TASK;
                  } /* endif */

                }
                SELECTITEM( hwnd, ID_FUZZYSEARCH_DOCS_LB, 0 );
                sprintf( pIda->szBuffer, "Searching document %d of %d ...", pIda->sDocCount - sDocsInLB + 1, pIda->sDocCount );
                SETTEXT( hwnd, ID_FUZZYSEARCH_STATUS_TEXT, pIda->szBuffer );

                /********************************************************/
                /* Start next step                                      */
                /********************************************************/
                UtlDispatch();
                pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );
                if ( (pIda != NULL) && !pIda->fTerminate )
                {
                  WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(sNextTask), 0L );
                } /* endif */
              }
              else
              {
                /********************************************************/
                /* no more documents in listbox                         */
                /********************************************************/

                if ( pIda->pLogWriter ) pIda->pLogWriter->write( "Info: No more documents in listbox" );
                /********************************************************/
                /* Inform user about end of search                      */
                /********************************************************/
                if ( pIda->ulNumOfMatches == 0)
                {
                  UtlErrorHwndW( INFO_FS_DOC_NO_HIT, MB_OK, 0, NULL, EQF_INFO, hwnd, TRUE );
                }
                else
                {
                  CHAR szCount[20];
                  PSZ pszParm = szCount;
                  sprintf( szCount, "%lu", pIda->ulNumOfMatches );
                  UtlErrorHwnd( INFO_FS_SEARCH_COMPLETE, MB_OK, 1, &pszParm, EQF_INFO, hwnd );
                  SELECTITEM( hwnd, ID_FUZZYSEARCH_RESULT_LB, 0 );  
                  SETFOCUS( hwnd, ID_FUZZYSEARCH_RESULT_LB );
                } /* endif */                 

                /********************************************************/
                /* continue with stop operation task                    */
                /********************************************************/
                UtlDispatch();
                pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );
                if ( (pIda != NULL) && !pIda->fTerminate )
                {
                  WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(STOP_TASK), 0L );
                } /* endif */
              } /* endif */
            }
            break;

          case STOP_TASK :
            if ( pIda->pLogWriter ) pIda->pLogWriter->write( "Info: Processing task: STOP_TASK" );
            pIda->fSearching = FALSE;


            if ( pIda->fStoppedByUser )
            {
              sprintf( pIda->szBuffer, "Search stopped by user, found %lu segments with fuzzy matches up to stop", pIda->ulNumOfMatches );
              SETTEXT( hwnd, ID_FUZZYSEARCH_STATUS_TEXT, pIda->szBuffer );
            } 
            else if ( pIda->fStoppedByError )
            {
              sprintf( pIda->szBuffer, "Search stopped by error, found %lu segments with fuzzy matches up to error", pIda->ulNumOfMatches );
              SETTEXT( hwnd, ID_FUZZYSEARCH_STATUS_TEXT, pIda->szBuffer );
            } 
            else 
            {
              sprintf( pIda->szBuffer, "Search complete, found %lu segments with fuzzy matches", pIda->ulNumOfMatches );
              SETTEXT( hwnd, ID_FUZZYSEARCH_STATUS_TEXT, pIda->szBuffer );
            } /* endif */               

            ENABLECTRL( hwnd, ID_FUZZYSEARCH_STOP_PB, FALSE );
            ENABLECTRL( hwnd, ID_FUZZYSEARCH_FIND_PB, TRUE );
            DELETEALL( hwnd, ID_FUZZYSEARCH_DOCS_LB );
            FS_FillDocListbox( hwnd, pIda );
            UtlSetHorzScrollingForLB(GetDlgItem(hwnd, ID_FUZZYSEARCH_DOCS_LB));
            if ( pIda->iNumOfFolMems != 0)
            {
              FS_CloseFolderMemory( pIda );
            } /*endif */
            pIda->szLastFolObjName[0] = EOS;              // no folder (memory) is active anymore
            if ( pIda->pDocMem != NULL )
            {
              MemoryFactory *pFactory = MemoryFactory::getInstance();
              pFactory->closeMemory( pIda->pDocMem );
              pIda->pDocMem = NULL;
            } /* endif */               
            if ( pIda->ulNumOfMatches != 0 )
            {
              ENABLECTRL( hwnd, ID_FUZZYSEARCH_EXPORT_PB, TRUE );
            } /* endif */
            pIda->usHits = 0;
            pIda->usChanges = 0;
            break;

          case SETFOCUS_TASK :
            if ( pIda->pLogWriter ) pIda->pLogWriter->write( "Info: Processing task: SETFOCUS_TASK" );
            SETFOCUS( hwnd, ID_FUZZYSEARCH_RESULT_LB );
            PostMessage( hwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem( hwnd, ID_FUZZYSEARCH_RESULT_LB ), TRUE );
            break;

          case RESIZE_DIALOG_TASK:
            {
             SHORT sWidth = SHORT1FROMMP2(mp2);
             SHORT sHeight = SHORT2FROMMP2(mp2);

             HWND hwndFrame = GetParent( hwnd );
             WinSetWindowPos( hwndFrame, HWND_TOP, 0, 0, sWidth, sHeight, EQF_SWP_SIZE );
            }
            break;

        } /* endswitch */
      } /* endif */
      return 0;
      break;

    case WM_CLOSE:
      pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );
      pIda->fTerminate = TRUE;
      FS_CloseDoc( pIda, TRUE );
       
      // save current size and position
      {
        EQFINFO     ErrorInfo;       // error code of property handler calls
        PPROPFOLDERLIST pFllProp;    // ptr to folder list properties
        PVOID       hFllProp;        // handle of folder list properties
        OBJNAME     szFllObjName;    // buffer for folder list object name

        UtlMakeEQFPath( szFllObjName, NULC, SYSTEM_PATH, NULL );
        strcat( szFllObjName, BACKSLASH_STR );
        strcat( szFllObjName, DEFAULT_FOLDERLIST_NAME );
        hFllProp = OpenProperties( szFllObjName, NULL, PROP_ACCESS_READ,
                                   &ErrorInfo );
        if ( hFllProp )
        {
          if ( SetPropAccess( hFllProp, PROP_ACCESS_WRITE) )
          {
            pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFllProp );

            UtlSaveWindowPos( GetParent( hwnd ), &(pFllProp->swpFSearchSizePos) );

            // save last used values
            pFllProp->fFSWithMarks = QUERYCHECK( hwnd, ID_FUZZYSEARCH_SHOWDIFF_CHK );
            pFllProp->iFSLastUsedClass = CBQUERYSELECTION( hwnd, ID_FUZZYSEARCH_CLASS_CB );
            int iSelected = CBQUERYSELECTION( hwnd, ID_FUZZYSEARCH_MODE_CB );
            if ( iSelected != LIT_NONE ) pFllProp->iFSLastUsedMode = CBQUERYITEMHANDLE( hwnd, ID_FUZZYSEARCH_MODE_CB, iSelected );
            strcpy( pFllProp->szFSLastExportFile, pIda->szExportFileName );

            // save last used color settings
            int iMaxEntries = sizeof(pFllProp->aclrFSLastUsedBackground)/sizeof(COLORREF);
            for( int i = 0; i < iMaxEntries; i++ )
            {
              pFllProp->aclrFSLastUsedForeground[i] = pIda->ColorData.ColorSetting[i].cForeground;
              pFllProp->aclrFSLastUsedBackground[i] = pIda->ColorData.ColorSetting[i].cBackground;
            } /* endfor */

            // save font information
            pFllProp->lFSFontHeight = pIda->lf.lfHeight;
            pFllProp->lFSFontWidth = pIda->lf.lfWidth;
            pFllProp->lFSFontEscapement = pIda->lf.lfEscapement;
            pFllProp->lFSFontOrientation = pIda->lf.lfOrientation;
            pFllProp->lFSFontWeight = pIda->lf.lfWeight;
            pFllProp->bFSFontItalic = pIda->lf.lfItalic;
            pFllProp->bFSFontUnderline = pIda->lf.lfUnderline;
            pFllProp->bFSFontStrikeOut = pIda->lf.lfStrikeOut;
            pFllProp->bFSFontCharSet = pIda->lf.lfCharSet;
            pFllProp->bFSFontOutPrecision = pIda->lf.lfOutPrecision;
            pFllProp->bFSFontClipPrecision = pIda->lf.lfClipPrecision;
            pFllProp->bFSFontQuality = pIda->lf.lfQuality;
            pFllProp->bFSFontPitchAndFamily = pIda->lf.lfPitchAndFamily;
            strcpy( pFllProp->szFSFontFaceName, pIda->lf.lfFaceName );

            SaveProperties( hFllProp, &ErrorInfo);
          } /* endif */
          CloseProperties( hFllProp, PROP_QUIT, &ErrorInfo );
        } /* endif */
      }
      if ( pIda->fRegistered )
      {
        EqfRemoveObject( TWBFORCE, hwnd );
        pIda->fRegistered = FALSE;
      } /* endif */
      SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ), WM_MDIDESTROY,
                   MP1FROMHWND(hwnd), 0L ) ;
      return 0;


    case WM_EQF_INITMENU:
    case WM_INITMENU:
    case WM_INITMENUPOPUP:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

    case WM_EQF_TOOLBAR_ENABLED:
      /****************************************************************/
      /* disable all toolbars...                                      */
      /****************************************************************/
      break;

    case WM_DESTROY:
      pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );
      if (pIda)
      {
        if ( pIda->hFontControl ) DeleteObject( pIda->hFontControl );
        if ( pIda->hwndLB ) WinDestroyWindow( pIda->hwndLB );
        if ( pIda->fRegistered )
        {
          EqfRemoveObject( TWBFORCE, hwnd );
          pIda->fRegistered = FALSE;
        } /* endif */

        if ( pIda->pLogWriter ) 
        {
          pIda->pLogWriter->write( "Info: Ending fuzzy sement search" );
          pIda->pLogWriter->close();
          delete pIda->pLogWriter;
          pIda->pLogWriter = NULL;
        } /* endif */

        free( pIda );
        pIda = NULL;
      } /* endif */
      UtlUnregisterModelessDlg( hwnd );
      break;

   case WM_SIZE :
     pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );

     // resize inner window only if normal sizing request...
     if ( (pIda != NULL) && ((mp1 == SIZENORMAL) || (mp1 == SIZEFULLSCREEN)) )
     {
       SHORT   sWidth  = LOWORD( mp2 );      // new width of dialog
       SHORT   sHeight = HIWORD( mp2 );      // new height of dialog

       LONG   lBorderSize  = WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER);

       LONG   cxAvail = sWidth - (2 * lBorderSize);

       // re-arrange controls
       {
         LONG  lTotSize = 0;           // total width of pushbuttons
         LONG  lTotGaps;               // total size of gaps between pushbuttons
         LONG  lGap;                   // gap between pushbuttons
         LONG  lCorrect;               // correction value
         LONG  lXPos;                  // current x position
         LONG  yTextStartPos;          // vertical start position for text area
         LONG  lLeftBorder;            // border to leave on left side
         LONG  lGroupBoxWidth;         // width of group box

         int i;
         {
           // we re-position/re-size all dialog controls ...
           HDWP hdwp = BeginDeferWindowPos( 11 );

           // re-arrange pushbuttons
           for ( i = 0; i < NUM_OF_FUZZYSEARCH_PB; i++ )
           {
             lTotSize += pIda->sButtonWidth[i];
           } /* endfor */

           lTotGaps = (cxAvail > lTotSize) ? (cxAvail - lTotSize) : 0;
           lGap = lTotGaps / NUM_OF_FUZZYSEARCH_PB;
           lCorrect = (cxAvail > lTotSize) ? ((cxAvail - (lGap * NUM_OF_FUZZYSEARCH_PB) - lTotSize) / 2) : 0;
           lXPos    = pIda->sBorderSize;
           for ( i = 0; (i < NUM_OF_FUZZYSEARCH_PB) && (hdwp != NULL); i++ )
           {
             lXPos += i ? (pIda->sButtonWidth[i-1] + lGap) : ((lGap / 2) + lCorrect);
             hdwp = DeferWindowPos( hdwp, pIda->hwndButton[i], HWND_TOP, lXPos,
                                    sHeight - pIda->sBorderSize - pIda->sButtonHeight,
                                    pIda->sButtonWidth[i], pIda->sButtonHeight,
                                    SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
           } /* endfor */


           // resize documents listbox and groupbox
           {
             RECT rect;

             GetWindowRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_OPTIONS_GB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             lLeftBorder = rect.left;
             lGroupBoxWidth = (cxAvail > (rect.left + 3)) ? (cxAvail - rect.left - 3) : 0;
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FUZZYSEARCH_OPTIONS_GB ),
                                    HWND_TOP, 0, 0,
                                    lGroupBoxWidth,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

             GetWindowRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_DOCS_GB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             lLeftBorder = rect.left;
             lGroupBoxWidth = (cxAvail > (rect.left + 3)) ? (cxAvail - rect.left - 3) : 0;
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FUZZYSEARCH_DOCS_GB ),
                                    HWND_TOP, 0, 0,
                                    lGroupBoxWidth,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
             yTextStartPos = rect.bottom + 2;

             GetWindowRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_DOCS_LB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             rect.left = lLeftBorder + 4;
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FUZZYSEARCH_DOCS_LB ),
                                    HWND_TOP, rect.left, rect.top,
                                    lGroupBoxWidth - 8,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOZORDER );
           }

           // set y position for result listbox
           {
             RECT rect;

             GetWindowRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_DOCS_LB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             yTextStartPos = rect.bottom + 2;
           }

           // resize result area
           {
             RECT rect;
             LONG lYDistance;

             // get vertical displacement of text MLE to text groupbox
             GetWindowRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_RESULT_LB ), &rect );
             lYDistance = rect.top;
             GetWindowRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_RESULT_GB ), &rect );
             lYDistance -= rect.top;

             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );

             // resize and reposition result groupbox
             rect.top = yTextStartPos;
             rect.bottom = sHeight - pIda->sBorderSize - pIda->sButtonHeight - 6;
             rect.left = lLeftBorder;
             rect.right = lLeftBorder + lGroupBoxWidth;

             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FUZZYSEARCH_RESULT_GB ),
                                    HWND_TOP,
                                    rect.left, rect.top,
                                    rect.right - rect.left,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOZORDER );

             // resize and reposition result listbox
             rect.top    += lYDistance;
             rect.bottom -= 4 + pIda->sButtonHeight;
             rect.left   += 4;
             rect.right  -= 4;
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FUZZYSEARCH_RESULT_LB ),
                                    HWND_TOP,
                                    rect.left, rect.top,
                                    rect.right - rect.left,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOZORDER );

             // reposition status field
             {
               RECT rectStatus;
               GetWindowRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_STATUS_TEXT ), &rectStatus );
               MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rectStatus, 2 );

              hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FUZZYSEARCH_STATUS_TEXT ),
                                     HWND_TOP,
                                     rect.left + 10, rect.bottom + 4, 
                                     rect.right - rect.left - 20,
                                     rectStatus.bottom - rectStatus.top,
                                     SWP_NOACTIVATE | SWP_NOZORDER );
             }

             // resize find and display options groupbox
             GetWindowRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_DISPLAYOPT_GB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             rect.right = lLeftBorder + lGroupBoxWidth - 4;
             if ( rect.right <= rect.left )
             {
               rect.right = rect.left + 1;
             } /* endif */
             lXPos = rect.right;
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FUZZYSEARCH_DISPLAYOPT_GB ),
                                    HWND_TOP,
                                    rect.left, rect.top,
                                    rect.right - rect.left,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOZORDER );

             // resize find and display options groupbox
             GetWindowRect( GetDlgItem( hwnd, ID_FUZZYSEARCH_SHOWDIFF_CHK ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             rect.right = lXPos - 4;
             if ( rect.right <= rect.left )
             {
               rect.right = rect.left + 1;
             } /* endif */
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_FUZZYSEARCH_SHOWDIFF_CHK ),
                                    HWND_TOP,
                                    rect.left, rect.top,
                                    rect.right - rect.left,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOZORDER );



           }

           // do actual dialog control re-positioning
           if ( hdwp != NULL )
           {
             EndDeferWindowPos( hdwp );
           } /* endif */
         }
       }

       // restore dialog to minimum width and height when it is getting too small
       // GQ 2017(08/04: disabled the code below as it tends to create endless dialog resizing loops...
       //if ( pIda->fInitCompleted )
       //{
       //  SHORT sMinWidth = (SHORT)pIda->lMinWidth;
       //  SHORT sMinHeight = (SHORT)pIda->lMinHeight;
       //  BOOL fResize = FALSE;
       //  if ( (sWidth + 10) < sMinWidth )
       //  {
       //    sWidth = sMinWidth;
       //    fResize = TRUE;
       //  }
       //  if ( (sHeight + 10) < sMinHeight )
       //  {
       //    fResize = TRUE;
       //    sHeight = sMinHeight;
       //  } 
       //  if ( fResize )
       //  {
       //    WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(RESIZE_DIALOG_TASK), MP2FROM2SHORT( sWidth, sHeight ) );
       //  } /* endif */
       //} /* endif */
     } /* endif */
     break;

   case WM_MEASUREITEM:
     FS_MeasureItem( hwnd, mp2 );
     return( 0 );
     break;

   case WM_DRAWITEM:
     pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );
     return ( FS_DrawItem( pIda, mp2 ) );
     break;


  } /* endswitch */

  return WinDefDlgProc( hwnd, msg, mp1, mp2 );

} /* end FSearchDlgProc */

/**********************************************************************/
/* Handling for WM_CONTROL message                                    */
/**********************************************************************/
MRESULT FSearchControl
(
  HWND   hwnd,                        // dialog handle
  SHORT  sId,                         // id in action
  SHORT  sNotification                // notification
)
{
  PFSEARCHIDAX pIda;                    // ptr to IDA of dialog
  MRESULT mResult = MRFROMSHORT(FALSE);

  sNotification; sId;

  pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );
  //switch ( sId )
  //{
  //} /* endswitch sID */
  return( mResult );
} /* end of function FSearchControl */


// remove LFs in segment data
void FS_RemoveLF( PSZ_W pszData, ULONG ulOemCP )
{
  PSZ_W pszTemp = pszData;
  PSZ_W pszOut = pszData;
  while ( *pszTemp != EOS)
  {
    if ( *pszTemp == LF )
    {
      if ( !IsDBCS_CP( ulOemCP ) )
      {
        *pszOut++ = BLANK;
      }
      else
      {
        if ( ! EQFIsDBCSChar( pszTemp[1], ulOemCP) )
        {
          *pszOut++ = BLANK;
        } /* endif */
      } /* endif */
    }
    else
    { 
      *pszOut++ = *pszTemp;
    } /* endif */
    pszTemp++;
  } /* endwhile */
  *pszOut = 0;
} /* end of function FS_RemoveLF */

//
//  function FS_CloseFolderMemory
//
//  close the memory DBs associated with the folder
//
static SHORT FS_CloseFolderMemory( PFSEARCHIDAX pIda )
{
  SHORT       sRc = 0;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  int i = 0;
  while ( i < pIda->iNumOfFolMems )
  {
    pFactory->closeMemory( pIda->pFolMem[i] );
    i++;
  } /*endwhile */

  if ( pIda->pDocMem != NULL )
  {
    pFactory->closeMemory( pIda->pDocMem );
  } /* endif */

  memset( &(pIda->pFolMem), 0, sizeof(pIda->pFolMem) );
  memset( &(pIda->pDocMem), 0, sizeof(pIda->pDocMem) );

  pIda->iNumOfFolMems = 0;

  return( sRc );
} /* end of function FS_CloseFolderMemory */

//
//  function FS_OpenDocMemory
//
//  open document memory if not open yet
//
static SHORT FS_OpenDocMemory
( 
  PFSEARCHIDAX     pIda,                 // control block
  PSZ         pszDocMem                // name of document memory
)
{
  SHORT       sRc = 0;
  MemoryFactory *pFactory = MemoryFactory::getInstance();


  // ignore memory if is the same as the folder memory
  if ( strcmp( pIda->szFolMemNames[0], pszDocMem ) == 0 )
  {
    pszDocMem = "";
  } /* endif */

  if ( pIda->pDocMem != NULL ) // already a document memory active ?
  {
    if ( strcmp( pIda->szDocMemName, pszDocMem ) == 0 )
    {
      // same memory, so we are done
      return( 0 );
    } /* endif */

    // new name is different , de-activate current document memory
    pFactory->closeMemory( pIda->pDocMem );
    pIda->pDocMem = NULL;
  } /* endif */

  // open document memory
  if ( pszDocMem[0] )
  {
    int iRC = 0;


    strcpy( pIda->szDocMemName, pszDocMem );

    pIda->pDocMem = pFactory->openMemory( NULL, pIda->szDocMemName, NONEXCLUSIVE, &iRC );

    if ( pIda->pDocMem == NULL)
    {
      PSZ pszParm = pIda->szDocMemName;
      UtlErrorHwnd( ERROR_TM_NOT_EXIST, MB_CANCEL, 1, &pszParm, EQF_ERROR, pIda->hwndErrMsg );
      sRc = ERROR_TM_NOT_EXIST;
    }
  } /* endif */

  return( sRc );
} /* end of function FS_OpenDocMemory */

//
//  function FS_OpenFolderMemory
//
//  open the memory DBs associated with the folder
//
static SHORT FS_OpenFolderMemory
( 
  PFSEARCHIDAX     pIda                  // control block
)
{
  SHORT       sRc = 0;
  HPROP       hProp;                   // handle to document properties
  ULONG       ulErrorInfo;             // error indicator from PRHA
  int         iNumOfMems = 0;
  BOOL        fOK = TRUE;
  MemoryFactory *pFactory = MemoryFactory::getInstance();


  // get list of folder memories
  hProp = OpenProperties( pIda->szSubFolObjName, NULL, PROP_ACCESS_READ, &ulErrorInfo );
  if ( hProp )
  {
    PPROPFOLDER pProp = (PPROPFOLDER)MakePropPtrFromHnd( hProp );

    // add folder main memory to list
    if ( pProp->szLongMemory[0]!= EOS )
    {
      strcpy( pIda->szFolMemNames[iNumOfMems], pProp->szLongMemory );
    }
    else
    {
      strcpy( pIda->szFolMemNames[iNumOfMems], pProp->szMemory );
    } /* endif */
    iNumOfMems++;

    // add folder R/O memories to the list
    if ( pProp->aLongMemTbl[0][0] != EOS )
    {
      int i = 0;
      while ( (i < MAX_NUM_OF_READONLY_MDB) && (pProp->aLongMemTbl[i][0] != EOS) )
      {
        strcpy( pIda->szFolMemNames[iNumOfMems], pProp->aLongMemTbl[i] );
        iNumOfMems++;
        i++;
      } /*endwhile */
    }
    else
    {
      PSZ pszToken;
      PSZ pszBuffer = (PSZ)(pIda->szBuffer);

      //get first R/O memory from list
      strcpy( pszBuffer, pProp->MemTbl );
      pszToken = strtok( pszBuffer, X15_STR );
      while ( (pszToken != NULL) && fOK )
      {
        strcpy( pIda->szFolMemNames[iNumOfMems], pszToken );
        iNumOfMems++;
        pszToken = strtok( NULL, X15_STR );
      } /* endwhile */
    } /* endif */

    // open the folder memories
    {
      int i = 0;
      int iRC = 0;
      while ( !sRc && (i < iNumOfMems) )
      {
        pIda->pFolMem[i] = pFactory->openMemory( NULL, pIda->szFolMemNames[i], NONEXCLUSIVE, &iRC );
        if ( pIda->pFolMem[i] == NULL )
        {
          PSZ pszParm = pIda->szFolMemNames[i];
          UtlErrorHwnd( ERROR_TM_NOT_EXIST, MB_CANCEL, 1, &pszParm, EQF_ERROR, pIda->hwndErrMsg );
          sRc = ERROR_TM_NOT_EXIST;
        }
        i++;
      } /* endwhile */         
    }

    // get stop at first exact... flag from properties
    pIda->fStopAtFirstExact = pProp->fStopAtFirstExact;

    CloseProperties( hProp, PROP_QUIT, &ulErrorInfo );
  } /* endif */

  pIda->iNumOfFolMems = iNumOfMems;

  return( sRc );
} /* end of function FS_OpenFolderMemory */


// lookup segment in a single memory
USHORT FS_LookupInMemory( PFSEARCHIDAX pIda, PTBSEGMENT pSeg, OtmMemory *pMem, std::vector<OtmProposal *> &vResults )
{
  USHORT      usMatch = 0;
  OtmProposal SearchKey;
  
  SearchKey.setDocShortName( pIda->szCurDoc );
  SearchKey.setDocName( pIda->szLongName );
  SearchKey.setSegmentNum( pSeg->ulSegNum );
  SearchKey.setSource( pSeg->pDataW );
  SearchKey.setMarkup( pIda->szDocFormat );
  SearchKey.setSourceLanguage( pIda->szDocSrcLng);
  SearchKey.setTargetLanguage( pIda->szDocTgtLng);

  pMem->searchProposal( SearchKey, vResults, GET_EXACT_AND_FUZZY );

  if ( OtmProposal::getNumOfProposals( vResults ) != 0 )
  {
    usMatch = (USHORT)vResults[0]->getFuzziness();
  } /* endif */

  return( usMatch );
} /* end of function FS_LookupInMemory */

// lookup segment in all folder and document memories
USHORT FS_LookupInAllMems( PFSEARCHIDAX pIda, PTBSEGMENT pSeg )
{
  USHORT      usMatch = 0;
  int         iMemIndex = 0;

  OtmProposal Result;

  std::vector<OtmProposal *> vResults;

  vResults.push_back( &Result );

  if ( pIda->pDocMem != NULL )
  {
    // get match for document memory 
    usMatch = FS_LookupInMemory( pIda, pSeg, pIda->pDocMem, vResults );
    if ( usMatch != 0 )
    {
      vResults[0]->getSource( pIda->szBestMatchSource, sizeof(pIda->szBestMatchSource)/sizeof(CHAR_W) );
      vResults[0]->getTarget( pIda->szBestMatchTarget, sizeof(pIda->szBestMatchTarget)/sizeof(CHAR_W) );
    } /* endif */       

    // skip first folder memory
    iMemIndex = 1;
  } /* endif */

  // get matches for folder memories
  while ( iMemIndex < pIda->iNumOfFolMems )
  {
    USHORT usNewMatch = 0;

    OtmProposal::clearAllProposals( vResults );
    usNewMatch = FS_LookupInMemory ( pIda, pSeg, pIda->pFolMem[iMemIndex], vResults );
    if ( usNewMatch > usMatch )
    {
      // get copy of better match 
      vResults[0]->getSource( pIda->szBestMatchSource, sizeof(pIda->szBestMatchSource)/sizeof(CHAR_W) );
      vResults[0]->getTarget( pIda->szBestMatchTarget, sizeof(pIda->szBestMatchTarget)/sizeof(CHAR_W) );
      usMatch = usNewMatch;
    } /* endif */       
    iMemIndex++;
  } /*endwhile */
  return( usMatch );
} /* end of function FS_LookupInAllMems */

// search next 20 segments for fuzzy matches
BOOL FS_SearchFuzzy
(
  PTBDOCUMENT pDoc,                  //ptr to doc instance
  PFSEARCHIDAX pIda
)
{
  BOOL fDone     = FALSE;              // we-are-through flag
  SHORT sRc;                            //return value
  PTBSEGMENT pSeg;                      //ptr to segment
  int iSegmentsToCheck = 20;            // number of segments to check

  sRc = 0;


  // while no match fFound and not at end of document
  while ( !fDone && (iSegmentsToCheck > 0) && !pIda->fStopSearch )
  {
    iSegmentsToCheck--;

    pSeg = EQFBGetFromBothTables( pDoc, &(pIda->ulSegNum), &(pIda->ulAddSegNum), &(pIda->ulActiveTable));

    if ( (pSeg == NULL) || (pSeg->pDataW == NULL) )
    {
      /* End of document reached                                      */
      fDone = TRUE;
    }
    else if ( pSeg->SegFlags.NoCount || pSeg->SegFlags.Joined || ((pSeg->qStatus != QF_ATTR) && (pSeg->qStatus != QF_TOBE) && (pSeg->qStatus != QF_CURRENT)) )
    {
      // ignore segment
    }
    else                            // normal data segments
    {
      // lookup segment in memory databases
      USHORT usMatch = FS_LookupInAllMems( pIda, pSeg );

      if ( (usMatch > 0) && (usMatch < 100) )
      {
        LONG lSegmentChangesListLen = 0;
        LONG lProposalChangesListLen = 0;
        int iClass = 0;
        USHORT usWords = 0;

        UTF16strcpy( pIda->szTempSeg, pSeg->pDataW );
        FS_RemoveLF( pIda->szTempSeg, pIda->ulSourceOemCP );

        UTF16strcpy( pIda->szTempProp, pIda->szBestMatchSource );
        FS_RemoveLF( pIda->szTempProp, pIda->ulSourceOemCP );

        iClass = CntGetProposalClass( pIda->szTempSeg, pIda->szTempProp, (PLOADEDTABLE)pDoc->pDocTagTable, 
                                            pDoc->pInBuf, pDoc->pTokBuf, pIda->sSrcLangID, pIda->ulSourceOemCP, &usWords );
        if ( ( (pIda->iSearchMode == UPTOSELECTEDCLASS_MODE ) && (iClass <= pIda->iSearchClass) ) ||
             ( (pIda->iSearchMode == ONLYSELECTEDCLASS_MODE ) && (iClass == pIda->iSearchClass) ) ||
             ( (pIda->iSearchMode == SELECTEDCLASSANDHIGHER_MODE ) && (iClass >= pIda->iSearchClass) ) )
        {
          int iMatchIndex = 0;
          BOOL fBelowThreshold = FALSE;

          // get differences between segment and proposal
          FS_GetProposalDiffs( pIda->szTempSeg, pIda->szTempProp, (PLOADEDTABLE)pDoc->pDocTagTable, 
                                            pDoc->pInBuf, pDoc->pTokBuf, pIda->sSrcLangID, pIda->ulSourceOemCP,
                                           (PFS_STARTSTOP)&(pIda->SegmentChangesList), &lSegmentChangesListLen, (PFS_STARTSTOP)&(pIda->ProposalChangesList), 
                                           &lProposalChangesListLen, &fBelowThreshold );

          // fill-in match info
          if ( !fBelowThreshold )
          {
            memset( &(pIda->CurMatch),0, sizeof(pIda->CurMatch) );
            pIda->CurMatch.iWords = (int)usWords;
            pIda->CurMatch.iDiffWords = iClass;
            pIda->CurMatch.iDocumentID = NameList_AddName( pIda->pNameList, pIda->szLongName );
            pIda->CurMatch.iFolderID = NameList_AddName( pIda->pNameList, pIda->szFolLongName );
            pIda->CurMatch.ulSegNum = pSeg->ulSegNum;
            pIda->CurMatch.pszSegment = StringBuffer_AddString( pIda->pvStringBuffer, pIda->szTempSeg );
            pIda->CurMatch.pszSource =  StringBuffer_AddString( pIda->pvStringBuffer, pIda->szTempProp );
            UTF16strcpy( pIda->szTempSeg, pIda->szBestMatchTarget );
            FS_RemoveLF( pIda->szTempSeg, pIda->ulTargetOemCP );
            pIda->CurMatch.pszTarget =  StringBuffer_AddString( pIda->pvStringBuffer, pIda->szTempSeg );

            // store token list in string buffer
            pIda->CurMatch.pSegmentChanges = (PFS_STARTSTOP)StringBuffer_AddData( pIda->pvStringBuffer, (PBYTE)&(pIda->SegmentChangesList), lSegmentChangesListLen );
            pIda->CurMatch.pProposalChanges= (PFS_STARTSTOP)StringBuffer_AddData( pIda->pvStringBuffer, (PBYTE)&(pIda->ProposalChangesList), lProposalChangesListLen );

            // add fuzzy match to result list
            iMatchIndex = FS_AddToResultList( pIda, &(pIda->CurMatch) );

            // show fuzzy match in result listbox
            if ( pIda->hwnd != HWND_FUNCIF ) FS_ShowResult( pIda, iMatchIndex );
          } /* endif */             
        } /* endif */           
      } /* endif */         
    } /* endif */
  } /*endwhile*/

  return fDone;
}


VOID FS_FreeDoc( PFSEARCHIDAX pIda, PVOID pvDoc )
{
  PTBDOCUMENT     pDoc = (PTBDOCUMENT)pvDoc;

  pIda;

  if ( pDoc == NULL )
  {
    return;
  } /* endif */

  if (pDoc->pQFTagTable)
  {
    TAFreeTagTable( (PLOADEDTABLE)pDoc->pQFTagTable );
    pDoc->pQFTagTable = NULL;
  } /* endif */
  if (pDoc->pDocTagTable)
  {
    TAFreeTagTable( (PLOADEDTABLE)pDoc->pDocTagTable );
    pDoc->pDocTagTable = NULL;
  } /* endif */

  SegFileFreeDoc( (PVOID *)&pDoc );
} /* end of function FS_FreeDoc */



BOOL FS_CloseDoc
(
  PFSEARCHIDAX pIda,
  BOOL        fFreeDoc
)
{
  BOOL fStopProcess = FALSE;

  if ( pIda->pTBDoc )
  {
    if ( pIda->pTBDoc->flags.changed && !pIda->fDocOpenActive )
    {
      CHAR   szDocFullPath[MAX_EQF_PATH];
      SHORT  sRc;
      USHORT usCPConv = 0;

      /********************************************************/
      /* build fully qualified name of segemented target doc  */
      /********************************************************/
      UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0],
                      DIRSEGTARGETDOC_PATH,
                      UtlGetFnameFromPath(pIda->szFolObjName) );
      //append document name to path
      strcat( szDocFullPath, BACKSLASH_STR );
      strcat( szDocFullPath, pIda->szCurDoc );

      pIda->pTBDoc->docType = STARGET_DOC; // enable hist log processing
      sRc = EQFBFileWriteEx( szDocFullPath, pIda->pTBDoc,
                             DOCSAVE_LOGTASK2, usCPConv );
      if ( sRc )
      {
        PSZ    pszParm = pIda->szCurDoc;
        if ( pIda->szLongName[0] != EOS ) pszParm = pIda->szLongName;
        if ( UtlErrorHwnd( ERROR_FOLFIND_SAVE_SEGFILE, MB_OKCANCEL,
                       1, &pszParm, EQF_QUERY, pIda->hwnd  ) != MBID_OK )
        {
          fStopProcess = TRUE;
        } /* endif */
      }
      else
      {
        // set document touch date
        HPROP           hPropDocument;     // handle to document properties
        PPROPDOCUMENT   pPropDocument;     // pointer to document properties
        ULONG           ulErrorInfo;       // error indicator from PRHA

        UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0],
                        SYSTEM_PATH,
                        UtlGetFnameFromPath(pIda->szFolObjName) );
        strcat( szDocFullPath, BACKSLASH_STR );
        strcat( szDocFullPath, pIda->szCurDoc );

        if ( (hPropDocument = OpenProperties( szDocFullPath, NULL,
                                             PROP_ACCESS_READ,
                                             &ulErrorInfo))!= NULL)
        {
          pPropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hPropDocument );
          if ( SetPropAccess( hPropDocument, PROP_ACCESS_WRITE) )
          {
            UtlTime( (PLONG)&pPropDocument->ulTouched );
            SaveProperties( hPropDocument, &ulErrorInfo );
            ResetPropAccess( hPropDocument, PROP_ACCESS_WRITE);
          } /* endif */
          CloseProperties( hPropDocument, PROP_FILE, &ulErrorInfo);
        } /* endif */
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* Free document area if requested                                */
    /******************************************************************/
    if ( fFreeDoc )
    {
      if ( pIda->pTBDoc->pQFTagTable != NULL )
      {
        TAFreeTagTable( (PLOADEDTABLE)pIda->pTBDoc->pQFTagTable );
      } /* endif */
      FS_FreeDoc( pIda,  pIda->pTBDoc );
      pIda->pTBDoc = NULL;
    } /* endif */
  } /* endif */

  return( fStopProcess );
}

/**********************************************************************/
/* Check if document has been unlocked for document open and, if      */
/* this has been done, if it can be accessed and locked again         */
/* if document has been change dit is reloaded into memory            */
/**********************************************************************/
BOOL FS_CheckForChangedDoc
(
  PFSEARCHIDAX pIda,
  PBOOL       pfRefreshed              // document-has-been-refreshed flag
)
{
  BOOL fOK = TRUE;

  *pfRefreshed = FALSE;                // default value is not-refreshed

  if ( pIda->fDocOpenActive )
  {
    /******************************************************************/
    /* Check if document is still locked                              */
    /******************************************************************/
    CHAR   szDocFullPath[MAX_EQF_PATH];
    SHORT  sRc;

    UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0],
                    SYSTEM_PATH,
                    UtlGetFnameFromPath(pIda->szFolObjName) );
    //append document name to path
    strcat( szDocFullPath, BACKSLASH_STR );
    strcat( szDocFullPath, pIda->szCurDoc );

    sRc = QUERYSYMBOL( szDocFullPath );
    if ( sRc == -1 )
    {
      WORD wDate, wTime;
      CHAR   szDocFullPath[MAX_EQF_PATH];
      FILEFINDBUF stResultBuf;            // DOS file find struct
      USHORT     usCount = 1;             // number of files requested
      HDIR       hDirHandle = HDIR_CREATE;// DosFind routine handle

      // document is not in use ==> lock this document
//      SETSYMBOL( szDocFullPath );
      pIda->fDocOpenActive = FALSE;

      /*******************************************************/
      /* Get last update date to check for document changes  */
      /*******************************************************/
      UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0],
                      DIRSEGTARGETDOC_PATH,
                      UtlGetFnameFromPath(pIda->szFolObjName) );
      strcat( szDocFullPath, BACKSLASH_STR );
      strcat( szDocFullPath, pIda->szCurDoc );
      UtlFindFirst( szDocFullPath, &hDirHandle, FILE_NORMAL,
                    &stResultBuf, sizeof(stResultBuf),
                    &usCount, 0L, 0);
      // close search file handle
      if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

      FileTimeToDosDateTime( &stResultBuf.ftLastWriteTime,
                             (LPWORD)&wDate,
                             (LPWORD)&wTime );
      if ( (pIda->DocTime != wTime) ||
           (pIda->DocDate != wDate) )
      {
        /*************************************************************/
        /* Re-load of document required                              */
        /*************************************************************/
        *pfRefreshed = TRUE;
        FS_FreeDoc( pIda, pIda->pTBDoc );
        pIda->pTBDoc  = NULL;
        fOK = FS_LoadDoc( pIda, FALSE, FALSE, FALSE );
        pIda->usOffs   = 0;           // restart at begin of current segment
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* Document still used by TPRO or another process               */
      /****************************************************************/
      PSZ    pszErrParm = pIda->szCurDoc;
      if ( pIda->szLongName[0] != EOS ) pszErrParm = pIda->szLongName;
      UtlError( ERROR_FOLFIND_DOC_STILLINUSE,
                MB_CANCEL, 1,
                &pszErrParm, EQF_QUERY );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  return( fOK );
}


// Fill visible document listbox with document long names
USHORT FS_FillDocListbox
(
  HWND        hwnd,                    // dialog window handle
  PFSEARCHIDAX pIda                     // dialog IDA
)
{
  SHORT sItemCount = QUERYITEMCOUNTHWND( pIda->hwndLB );
  SHORT sItem = 0;
  PSZ pszLastFolder = NULL;
  PSZ pszCurFolder =  pIda->szFolObjName;

  while ( sItem < sItemCount )
  {
    SHORT sInsertItem;           // index of inserted item
    CHAR  szShortName[MAX_FILESPEC];

    // get the document name
    QUERYITEMTEXTHWND( pIda->hwndLB, sItem, szShortName );
    if ( pIda->fMultipleObjs ) pszCurFolder = (PSZ)QUERYITEMHANDLEHWND( pIda->hwndLB, sItem );


    // build document object name
    UtlMakeEQFPath( pIda->szDocObjName, pszCurFolder[0], SYSTEM_PATH, UtlGetFnameFromPath(pszCurFolder) );
    strcat( pIda->szDocObjName, BACKSLASH_STR );
    strcat( pIda->szDocObjName, szShortName );

    // add folder seperator when folder changes
    if ( pIda->fMultipleObjs && (pszLastFolder != pszCurFolder) )
    {
      SubFolObjectNameToName( pszCurFolder, pIda->szLongName );
      sprintf( pIda->szNameBuffer, "*** %s ***", pIda->szLongName );
      sInsertItem = INSERTITEM( hwnd, ID_FUZZYSEARCH_DOCS_LB, pIda->szNameBuffer );
      if ( sInsertItem >= 0 ) SETITEMHANDLE( hwnd, ID_FUZZYSEARCH_DOCS_LB, sInsertItem, -1 );
      pszLastFolder = pszCurFolder;
    } /* endif */

    // get the long name for this document
    pIda->szLongName[0] = EOS;
    DocQueryInfo2( pIda->szDocObjName, NULL, NULL, NULL, NULL, pIda->szLongName, NULL, NULL, TRUE );

    // add long name to long name listbox
    if ( pIda->fMultipleObjs )
    {
      if ( pIda->szLongName[0] != EOS )
      {
        OEMTOANSI( pIda->szLongName );
        sInsertItem = INSERTITEMEND( hwnd, ID_FUZZYSEARCH_DOCS_LB, pIda->szLongName );
      }
      else
      {
        sInsertItem = INSERTITEMEND( hwnd, ID_FUZZYSEARCH_DOCS_LB, szShortName );
      } /* endif */
    }
    else
    {
      if ( pIda->szLongName[0] != EOS )
      {
        OEMTOANSI( pIda->szLongName );
        sInsertItem = INSERTITEM( hwnd, ID_FUZZYSEARCH_DOCS_LB, pIda->szLongName );
      }
      else
      {
        sInsertItem = INSERTITEM( hwnd, ID_FUZZYSEARCH_DOCS_LB, szShortName );
      } /* endif */
    } /* endif */

    // set item handle to index of document in invisible document LB
    if ( sInsertItem >= 0 )
    {
      SETITEMHANDLE( hwnd, ID_FUZZYSEARCH_DOCS_LB, sInsertItem, sItem );
    } /* endif */
    sItem++;
  } /* endwhile */
  return( NO_ERROR );
} /* end of function FS_FillDocListbox */




//
// set the color of a text area in the rich edit control
void FSearch_SetColor
(
  HWND        hwndMLE,                 // handle of rich edit control
  int         iStart,                  // start pos for colored area
  int         iEnd,                    // end pos of colored area
  COLORREF    colorText,               // text color
  COLORREF    colorBackground          // background color
)
{
  CHARFORMAT2 Format;

  MLESETSELHWND( hwndMLE, iStart, iEnd );
  memset( &Format, 0, sizeof(Format) );
  Format.cbSize = sizeof(Format);
  Format.dwMask = CFM_COLOR | CFM_BACKCOLOR;
  Format.crTextColor = colorText;
  Format.crBackColor = colorBackground;
  SendMessage( hwndMLE, EM_SETCHARFORMAT, SCF_SELECTION, MP2FROMP(&Format) );
} /* end of function FSearch_SetColor */


// prepare search for a folder
BOOL FS_PrepFolderSearch
(
  PFSEARCHIDAX pIda
)
{
  BOOL fOK = TRUE;

  /* Get folder language                                        */
  if ( fOK)
  {
      EQFINFO     ErrorInfo;       // error code of property handler calls
      PVOID  hFolProp;
      PPROPFOLDER pFolProp;
      CHAR    szSysDrive[MAX_DRIVE];
      CHAR   chFolderDrive;

      // Use Sytem drive to retrieve folder properties
      UtlQueryString( QST_PRIMARYDRIVE, szSysDrive, sizeof(szSysDrive) );

      // save original folder drive
      chFolderDrive = pIda->szFolObjName[0];
      pIda->szFolObjName[0]=szSysDrive[0];

      hFolProp = OpenProperties( pIda->szFolObjName, NULL, PROP_ACCESS_READ, &ErrorInfo );
      if ( hFolProp )
      {
        pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
        pIda->fUseThaiFont = FALSE;
        if ( (stricmp(pFolProp->szSourceLang, THAI_STR) == 0) || (stricmp(pFolProp->szTargetLang, THAI_STR) == 0) )
        {
        } /* endif */
        CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
      } /* endif */
      // restore folder drive
      pIda->szFolObjName[0] = chFolderDrive;
  }

  /* Load document names                                        */
  if ( fOK )
  {
    if ( pIda->fMultipleObjs )
    {
      HWND hwndTemp = WinCreateWindow( pIda->hwnd, WC_LISTBOX, "", 0L, 0, 0, 0, 0, pIda->hwnd, HWND_TOP, 2, NULL, NULL );
      PSZ pszFolder = pIda->pszObjList;

      // for all folders do...
      while ( *pszFolder )
      {
        int i = 0;
        int docs = 0;

        // fill temp listbox with documents of folder
        DELETEALLHWND( hwndTemp );
        EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND(hwndTemp), MP2FROMP(pszFolder) );

        // copy documents to document listbox and set item handle to folder object name
        docs = QUERYITEMCOUNTHWND( hwndTemp );
        i = 0;
        while ( i < docs )
        {
          int inserted = 0;
          QUERYITEMTEXTHWND( hwndTemp, i, pIda->szNameBuffer );
          inserted = INSERTITEMENDHWND( pIda->hwndLB, pIda->szNameBuffer );
          if ( inserted >= 0 ) SETITEMHANDLEHWND( pIda->hwndLB, inserted, (LONG)pszFolder );
          i++;
        } /*endwhile */

        // next folder
        pszFolder += strlen(pszFolder) + 1;
      } /*endwhile */
      WinDestroyWindow( hwndTemp );
    }
    else if ( pIda->fFromFolderList )
    {
      EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                        MP1FROMHWND(pIda->hwndLB),
                        MP2FROMP(pIda->szSubFolObjName) );
    }
    else
    {
      EqfSend2Handler( FOLDERHANDLER, WM_EQF_QUERYSELECTEDNAMES,
                        MP1FROMHWND(pIda->hwndLB),
                        MP2FROMP(pIda->szSubFolObjName) );
    } /* endif */
    DELETEALL( pIda->hwnd, ID_FUZZYSEARCH_DOCS_LB );
    FS_FillDocListbox( pIda->hwnd, pIda );
  } /* endif */

  /* Set document listbox horizontal screen width */
  if ( fOK )
  {
    HWND hwndLB = GetDlgItem( pIda->hwnd, ID_FUZZYSEARCH_DOCS_LB);
    UtlSetHorzScrollingForLB(hwndLB);
  }

  return( fOK );
}

//
// add match to table
//
BOOL FS_AddMatch( PFSEARCHIDAX pIda, PFOUNDFUZZYMATCH pNewMatch )
{
  BOOL        fOK = TRUE;

  // make room if table is full
  if ( pIda->ulNumOfMatches >= pIda->ulMatchTableSize )
  {
    fOK = UtlAlloc( (PVOID *)&(pIda->pMatchList), pIda->ulMatchTableSize * sizeof(FOUNDFUZZYMATCH), sizeof(FOUNDFUZZYMATCH) * (pIda->ulMatchTableSize + 100), ERROR_STORAGE );
    if ( fOK )
    {
      pIda->ulMatchTableSize += 100;
    } /* endif */       
  } /* endif */     

  // add new match
  if ( fOK )
  {
    memcpy( pIda->pMatchList + pIda->ulNumOfMatches, pNewMatch, sizeof(FOUNDFUZZYMATCH) );
    pIda->ulNumOfMatches++;
  } /* endif */     

  return( fOK );
} /* FS_AddMatch */

//
// NameList functions
//
PNAMELIST NameList_Create()
{ 
  PNAMELIST pList = NULL;
  pList = (PNAMELIST)malloc( sizeof(NAMELIST) );
  if ( pList != NULL )
  {
    memset( pList, 0, sizeof(NAMELIST) );
  } /* endif */     
  return( pList );
}

void NameList_Destroy( PNAMELIST pList  )
{ 
  if ( pList != NULL )
  {
    if ( pList->pNameSpace != NULL ) free( pList->pNameSpace );
    if ( pList->pSortedTable != NULL ) free( pList->pSortedTable );
    free( pList );
  } /* endif */     
}

PSZ NameList_GetName( PNAMELIST pList, int id )
{
  if ( id < pList->nextId )
  {
    return( pList->pNameSpace + pList->pOffsTable[id] );
  }
  else
  {
    return( NULL );
  } /* endif */
}

int NameList_AddName( PNAMELIST pList, PSZ pszName )
{
  int offs = 0;
  int id = NameList_FindName( pList, pszName );
  if ( id < 0 )
  { 
    // get new ID
    id = pList->nextId;
    pList->nextId += 1;

    offs = NameList_AddToNamespace( pList, pszName );

    NameList_AddToSortedTable( pList, id, offs );
    NameList_AddToOffsTable( pList, id, offs );
  } /* endif */

  return( id );
}

int NameList_FindName( PNAMELIST pList, PSZ pszName )
{
  int low = 0;
  int high = pList->iSortedTableUsed;
  int result = -1;
  int test = 1;
  if ( pList->iSortedTableUsed != 0 )
  {
    do
    {
      PSZ pszEntry = NULL;    

      test = (low + high) / 2;
      pszEntry = pList->pNameSpace + pList->pSortedTable[test].offs;    
      result = strcmp( pszName, pszEntry );
      if (result > 0 )
      {
        low = test + 1;
      } 
      else if ( result < 0 )
      {
        high = test - 1;
      } /* endif */
    } while ( (result != 0) && (low <= high) );
  } /* endif */
  return( (result == 0) ? pList->pSortedTable[test].id : -1 );
}

int NameList_AddToNamespace( PNAMELIST pList, char *pszName )
{
  int len = strlen( pszName ) + 1;
  int offs = -1;

  if ( (pList->iNameSpaceUsed + len) > pList->iNameSpaceSize )
  {
    pList->iNameSpaceSize += 4096;
    pList->pNameSpace = (char *)realloc( pList->pNameSpace, pList->iNameSpaceSize );
  } /* endif */

  if ( pList->pNameSpace != NULL )
  {
    offs = pList->iNameSpaceUsed;
    strcpy( pList->pNameSpace + offs, pszName );
    pList->iNameSpaceUsed += len;
  } /* endif */

  return( offs );
}

int NameList_AddToSortedTable( PNAMELIST pList, int id, int offs )
{
  int rc = 0;
  char *pszNew = pList->pNameSpace + offs;

  // enlarge table if necessary
  if ( pList->iSortedTableSize <= (pList->iSortedTableUsed + 1) )
  {
    int newLen = 0;
    int oldLen = pList->iSortedTableSize * sizeof(SORTEDTABLEENTRY);
    pList->iSortedTableSize += 1024;
    newLen = pList->iSortedTableSize * sizeof(SORTEDTABLEENTRY);
    pList->pSortedTable = (PSORTEDTABLEENTRY)realloc( pList->pSortedTable, newLen );
    memset( (BYTE *)pList->pSortedTable + oldLen, 0, newLen - oldLen );
  } /* endif */

  // add entry
  if ( pList->pSortedTable != NULL )
  {
    // find insert location
    int result = 0;
    int i = 0;
    while ( (i < pList->iSortedTableUsed) && (result <= 0) )
    {
      char *pszEntry = pList->pNameSpace + pList->pSortedTable[i].offs;
      result = strcmp( pszEntry, pszNew );
      if ( result <= 0 )
      {
        i++;
      } /* endif */
    } /*endwhile */
    
    // make room for new entry
    if ( i < pList->iSortedTableUsed )
    {
      int j;
      for( j = pList->iSortedTableUsed - 1; j >= i; j-- )
      {
        pList->pSortedTable[j+1].id   =  pList->pSortedTable[j].id;
        pList->pSortedTable[j+1].offs =  pList->pSortedTable[j].offs;
      } /* endfor */
    } /*endwhile */

    // add new entry
    pList->pSortedTable[i].offs = offs;
    pList->pSortedTable[i].id   = id;
    pList->iSortedTableUsed++;
  } /* endif */

  return( rc );
}

int NameList_AddToOffsTable( PNAMELIST pList, int id, int offs )
{
  int rc = 0;

  // enlarge table if necessary
  if ( pList->iOffsTableSize <= id )
  {
    int newLen = 0;
    int oldLen = pList->iOffsTableSize * sizeof(int);
    pList->iOffsTableSize += 1024;
    newLen = pList->iOffsTableSize * sizeof(int);
    pList->pOffsTable = (int *)realloc( pList->pOffsTable, newLen );
    memset( (BYTE *)pList->pOffsTable + oldLen, 0, newLen - oldLen );
  } /* endif */

  // add entry
  if ( pList->pOffsTable != NULL )
  {
    pList->pOffsTable[id] = offs;
  } /* endif */

  return( rc );
}

// 
// StringBuffer functions
//

typedef struct _STRINGBUFFER
{
  int         iFreeSpace;                        // number of free CHAR_Ws in buffer area
  int         iUsedSpace;                        // number of used CHAR_Ws in buffer area
  struct _STRINGBUFFER *pPrev;                   // previous string buffer
} STRINGBUFFER, *PSTRINGBUFFER;

#define STRINGBLOCKSIZE 32000

STRINGBUFFERHANDLE StringBuffer_Create()
{
  PSTRINGBUFFER *ppBuffer = (PSTRINGBUFFER *)malloc( sizeof(PVOID) );
  *ppBuffer = NULL;
  return( (PVOID)ppBuffer );
}


PSZ_W StringBuffer_AddString( STRINGBUFFERHANDLE hBuffer, PSZ_W pszString )
{
  int iLen = (wcslen( pszString ) + 1) * sizeof(CHAR_W);
  PSZ_W pszReturn = (PSZ_W)StringBuffer_AddData( hBuffer, (PBYTE)pszString, iLen );
  return( pszReturn);
}

PBYTE StringBuffer_AddData( STRINGBUFFERHANDLE hBuffer, PBYTE pbData, LONG lDataLen )
{
  PSTRINGBUFFER pBuffer = *((PSTRINGBUFFER *)hBuffer);
  PBYTE pbNewData = NULL;

  // always work in CHAR_Ws
  int iLen = (lDataLen + 1 ) / sizeof(CHAR_W);


  // add a new buffer if current is full
  if ( (pBuffer == NULL) || (pBuffer->iFreeSpace < iLen) )
  {
    int iAllocLen = (STRINGBLOCKSIZE * sizeof(CHAR_W)) + sizeof(STRINGBUFFER); 
    PSTRINGBUFFER pNewBlock = (PSTRINGBUFFER)malloc( iAllocLen ); 
    if ( pNewBlock != NULL )
    {
      memset( pNewBlock, 0, iAllocLen );
      pNewBlock->iFreeSpace = STRINGBLOCKSIZE;
      pNewBlock->iUsedSpace = 0;
      pNewBlock->pPrev = pBuffer;
      pBuffer = pNewBlock;
      *((PSTRINGBUFFER *)hBuffer) = pBuffer;
    } /* endif */       
  } /* endif */     

  // add data to current buffer
  if ( pBuffer != NULL )
  {
    pbNewData = (PBYTE)(((PSZ_W)(pBuffer+1)) + pBuffer->iUsedSpace);
    memcpy( pbNewData, pbData, lDataLen );
    pBuffer->iUsedSpace += iLen; 
    pBuffer->iFreeSpace -= iLen; 
  } /* endif */     

  return( pbNewData );
}

void StringBuffer_Destroy( STRINGBUFFERHANDLE hBuffer )
{
  PSTRINGBUFFER pBuffer = *((PSTRINGBUFFER *)hBuffer);

  while ( pBuffer != NULL )
  {
    PSTRINGBUFFER pPrevBuffer = pBuffer->pPrev;
    free( pBuffer );
    pBuffer = pPrevBuffer;
  } /* endwhile */     
  *((PSTRINGBUFFER *)hBuffer) = NULL;
}


BOOL FS_LoadDoc
(
  PFSEARCHIDAX pIda,
  BOOL fCheckLock,                     // check for locking
  BOOL fStartSearch,                   // start searching
  BOOL fContinueNext                   // continue with nect doc flag
)
{
  BOOL fStopProcess = FALSE;
  BOOL fLocked = FALSE;
  BOOL fOK = TRUE;

  /**********************************************************/
  /* Check if document is locked                            */
  /**********************************************************/
  if ( !fCheckLock )
  {
    fLocked = TRUE;
  }
  else
  {
    CHAR   szDocFullPath[MAX_EQF_PATH];
    SHORT  sRc;

    // get folder long name
    SubFolObjectNameToName( pIda->szCurFolObjName, pIda->szFolLongName );

    // setup document object name
    UtlMakeEQFPath( szDocFullPath, pIda->szCurFolObjName[0], SYSTEM_PATH, UtlGetFnameFromPath(pIda->szCurFolObjName) );
    strcat( szDocFullPath, BACKSLASH_STR );
    strcat( szDocFullPath, pIda->szCurDoc );

    // get document memory, long name, and markup
    pIda->szLongName[0] = EOS;
    DocQueryInfo2( szDocFullPath, pIda->szDocMemory, pIda->szDocFormat, pIda->szDocSrcLng, pIda->szDocTgtLng, pIda->szLongName, NULL, NULL, FALSE );

    sRc = QUERYSYMBOL( szDocFullPath );
    if ( sRc == -1 )
    {
      // document is not in use ==> lock this document
//      SETSYMBOL( szDocFullPath );
      fLocked = TRUE;
    }
    else
    {
      USHORT usMBRC;
      PSZ    pszErrParm = pIda->szCurDoc;

      if ( pIda->szLongName[0] != EOS ) pszErrParm = pIda->szLongName;
      usMBRC = UtlErrorHwnd( ERROR_FOLFIND_DOC_INUSE, MB_YESNO, 1, &pszErrParm, EQF_QUERY, pIda->hwnd );
      if ( usMBRC == MBID_NO )
      {
        fStopProcess = TRUE;
      } /* endif */
      fOK = FALSE;
    } /* endif */
  }

  // open folder memories when folder changes
  if ( fOK )
  {
    // open folder memories when folder changes
    if ( strcmp( pIda->szLastFolObjName, pIda->szCurFolObjName ) != 0 )
    {
      // close any open folder memories
      if ( pIda->szLastFolObjName[0] != EOS )
      {
        FS_CloseFolderMemory( pIda );
      } /* endif */         

      // open memories of new folder
      fOK = (FS_OpenFolderMemory( pIda ) == 0);                 // control block
      if ( !fOK )
      {
        fStopProcess = TRUE;
      }
      else
      {
        strcpy( pIda->szLastFolObjName, pIda->szCurFolObjName );
      } /* endif */         
    } /* endif */       
  } /* endif */     



  /**********************************************************/
  /* allocate TBDOC structure                               */
  /**********************************************************/
  if ( fOK )
  {
    fOK = UtlAllocHwnd( (PVOID *)&pIda->pTBDoc, 0L, (LONG)sizeof(TBDOCUMENT), ERROR_STORAGE, pIda->hwnd );
    if ( !fOK )
    {
      fStopProcess = TRUE;
    } /* endif */
  } /* endif */

  /**********************************************************/
  /* load tag table                                         */
  /**********************************************************/
  if ( fOK )
  {
    SHORT  sRc;

    sRc = TALoadTagTable( DEFAULT_QFTAG_TABLE, (PLOADEDTABLE *)&pIda->pTBDoc->pQFTagTable, TRUE, (pIda->hwnd != HWND_FUNCIF) );       
    fOK = (sRc == NO_ERROR);
  } /* endif */

  // load document markup table
  if ( fOK )
  {
    SHORT  sRc;

    sRc = TALoadTagTableExHwnd( pIda->szDocFormat,
                                (PLOADEDTABLE *)&pIda->pTBDoc->pDocTagTable, FALSE, TALOADPROTTABLEFUNC | TALOADUSEREXIT,
                                TRUE, pIda->hwnd  ); 
    fOK = (sRc == NO_ERROR);
  } /* endif */

  /**********************************************************/
  /* load the file                                          */
  /**********************************************************/
  if ( fOK )
  {
    CHAR   szDocFullPath[MAX_EQF_PATH];
    SHORT  sRc;

    /********************************************************/
    /* build fully qualified name of segemented target doc  */
    /********************************************************/
    UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0],
                    DIRSEGTARGETDOC_PATH,
                    UtlGetFnameFromPath(pIda->szFolObjName) );
    //append document name to path
    strcat( szDocFullPath, BACKSLASH_STR );
    strcat( szDocFullPath, pIda->szCurDoc );

    /********************************************************/
    /* check if segmented target file exists, issue warning */
    /* if file does not exist                               */
    /********************************************************/
    if ( UtlFileExist( szDocFullPath ) )
    {
      USHORT       usMBRC;
      PTBDOCUMENT  pDoc;
      PSZ    pszErrParm = pIda->szCurDoc;
      if ( pIda->szLongName[0] != EOS ) pszErrParm = pIda->szLongName;
      strcpy( pIda->pTBDoc->szDocName, szDocFullPath );
      strcpy( pIda->pTBDoc->szDocLongName, pIda->szLongName );

      // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to TgtLang
      pIda->ulSourceOemCP = GetLangCodePage(OEM_CP, pIda->szDocSrcLng);
      pIda->ulTargetOemCP = pIda->pTBDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pIda->szDocTgtLng);
      pIda->pTBDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pIda->szDocTgtLng);
      MorphGetLanguageID( pIda->szDocTgtLng, &(pIda->sTgtLangID) );
      MorphGetLanguageID( pIda->szDocSrcLng, &(pIda->sSrcLangID) );


      pIda->pTBDoc->docType = STARGET_DOC; // enable hist log processing

      sRc = EQFBFileRead( szDocFullPath, pIda->pTBDoc );
      if (sRc >= 0 )
      {
        pDoc = pIda->pTBDoc;
      } /* endif */

      if ( sRc != NO_ERROR )
      {
        usMBRC = UtlErrorHwnd( ERROR_FOLFIND_DOCLOAD, MB_OKCANCEL, 1, &pszErrParm, EQF_QUERY, pIda->hwnd );
        if ( usMBRC != MBID_OK )
        {
          fStopProcess = TRUE;
        } /* endif */
      } /* endif */
    }
    else
    {
      USHORT usMBRC;

      PSZ    pszErrParm = pIda->szCurDoc;
      if ( pIda->szLongName[0] != EOS ) pszErrParm = pIda->szLongName;

      usMBRC = UtlErrorHwnd( ERROR_FS_DOC_NOT_ANALYZED, MB_OKCANCEL,
                             1, &pszErrParm, EQF_QUERY, pIda->hwnd );
      if ( usMBRC != MBID_OK )
      {
        fStopProcess = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************/
  /* Unlock locked documents                              */
  /********************************************************/
  if ( !fOK && fLocked )
  {
    CHAR   szDocFullPath[MAX_EQF_PATH];
    SHORT  sRc;

    UtlMakeEQFPath( szDocFullPath, pIda->szFolObjName[0],
                    SYSTEM_PATH,
                    UtlGetFnameFromPath(pIda->szFolObjName) );
    //append document name to path
    strcat( szDocFullPath, BACKSLASH_STR );
    strcat( szDocFullPath, pIda->szCurDoc );

    sRc = REMOVESYMBOL( szDocFullPath );
  } /* endif */


  // open document memory
  if ( fOK && (pIda->szDocMemory[0] != EOS) )
  {
    SHORT sRC = FS_OpenDocMemory( pIda, pIda->szDocMemory );
    if ( sRC != 0 )
    {
      fStopProcess = TRUE;
      fOK = FALSE;
    } /* endif */       
  } /* endif */     
  
  /**********************************************************/
  /* start search at begin of document                      */
  /**********************************************************/
  if ( fStopProcess )
  {
    if ( pIda->hwnd != HWND_FUNCIF ) WinPostMsg( pIda->hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(STOP_TASK), 0L );
  }
  else if ( fOK )
  {
    pIda->fTMUpdRequired = FALSE;
    pIda->ulSegNum = 1;
    pIda->usOffs   = 0;
    pIda->usLine   = 1;
    pIda->ulAddSegNum = 1;
    pIda->ulActiveTable = STANDARDTABLE;

    if ( fStartSearch )
    {
      UtlDispatch();
      pIda = ACCESSDLGIDA( pIda->hwnd, PFSEARCHIDAX );
      if ( (pIda != NULL) && !pIda->fTerminate )
      {
        WinPostMsg( pIda->hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SEARCH_FUZZY_TASK), 0L );
      } /* endif */
    } /* endif */
  }
  else
  {
    /********************************************************/
    /* Cleanup partially loaded document                    */
    /********************************************************/
    if ( pIda->pTBDoc )
    {
      FS_FreeDoc( pIda, pIda->pTBDoc );
      pIda->pTBDoc = NULL;
    } /* endif */

    /********************************************************/
    /* error while loading document, continue with          */
    /* next one                                             */
    /********************************************************/
    if ( fContinueNext )
    {
      UtlDispatch();
      pIda = ACCESSDLGIDA( pIda->hwnd, PFSEARCHIDAX );
      if ( (pIda != NULL) && !pIda->fTerminate )
      {
        DELETEITEM( pIda->hwnd, ID_FUZZYSEARCH_DOCS_LB, 0 );
        WinPostMsg( pIda->hwnd, WM_EQF_PROCESSTASK,
                    MP1FROMSHORT(NEXT_DOC_TASK), 0L );
      } /* endif */
    } /* endif */
  } /* endif */
  return( fOK );
} /* end of function FS_LoadDoc */

// copy a fuzzy token list to a FS_STARTSTOP list and combine tokens of the same type
int FS_CopyToStartStopList( PFUZZYTOK pToken, PFS_STARTSTOP pStartStop )
{
  int iEntries = 0;                       // number of entries in start stop list
  USHORT usLastType = 0;

  // process tokens of source list
  while ( pToken->ulHash ) 
  {
    if ( (iEntries != 0) && ((USHORT)pToken->sType == usLastType) )
    {
      // update end position of previous entry
      pStartStop[iEntries-1].usStop = pToken->usStop;
    }
    else
    {
      // add new entry
      pStartStop[iEntries].usStart = pToken->usStart;
      pStartStop[iEntries].usStop = pToken->usStop;
      pStartStop[iEntries].usType = usLastType = (USHORT)pToken->sType;
      iEntries++;
    } /* endif */       
    pToken++;
  }

  // terminate start stop list
  pStartStop[iEntries].usStart = 0;
  pStartStop[iEntries].usStop = 0;
  pStartStop[iEntries].usType = 0;
  iEntries++;

  return( iEntries * sizeof(FS_STARTSTOP) );
}

// get the class of a proposal
int FS_GetProposalDiffs( PSZ_W pszSegment, PSZ_W pszProposal, PLOADEDTABLE pTable, PBYTE pbBuffer, PBYTE pbTokBuf, SHORT sLangID, ULONG ulOemCP, 
                           PFS_STARTSTOP pSegmentChangesList, PLONG plSegmentChangesListLen, PFS_STARTSTOP pProposalChangesList, PLONG plProposalChangesListLen,
                           PBOOL pfBelowThreshold )
{
  PFUZZYTOK    pFuzzyTgt = NULL;
  PFUZZYTOK    pFuzzyTok = NULL;       // returned token list
  USHORT       usDiff = 0;             // number of differences
  USHORT       usWords = 0;            // number of words/tokens
  BOOL         fOK = TRUE; 

  // fast exit if one or both strings are empty...
  if ( (*pszSegment == EOS) || (*pszProposal == EOS) )
  {
    return( -1 );
  } /*   endif */

  // call function to evaluate the differences
  if ( fOK )
  {
    fOK = EQFBFindDiffEx( pTable, pbBuffer, pbTokBuf, pszSegment, pszProposal, sLangID, (PVOID *)&pFuzzyTok, (PVOID *)&pFuzzyTgt, ulOemCP );
  } /* endif */

  if ( fOK )
  {

    EQFBCountDiff( pFuzzyTok, &usWords, &usDiff );

    if ( pfBelowThreshold != NULL )
    {
      USHORT usFuzzy = 0;
      USHORT usFuzzyThreshold = 0;

      // compute fuzziness
      usFuzzy = (usWords != 0) ? ((usWords - usDiff) * 100 / usWords) : 100;

      // check if fuzziness is below fuzziness threshold
	    if ( usWords > 15 )
	    {
		    usFuzzyThreshold = (USHORT)(UtlQueryULong( QL_LARGEFUZZLEVEL ) / 100);
	    }
	    else if ( usWords > 4 )
	    {
  		  usFuzzyThreshold = (USHORT)(UtlQueryULong( QL_MEDIUMFUZZLEVEL ) / 100);
	    }
	    else
	    {
		    usFuzzyThreshold = (USHORT)(UtlQueryULong( QL_SMALLFUZZLEVEL ) / 100);
	    } /* endif */

      *pfBelowThreshold = (usFuzzy <= usFuzzyThreshold);
    } /* endif */       

    
  } /* endif */

  // if requested copy fuzzy token lists to caller's buffer
  if ( pSegmentChangesList != NULL ) *plSegmentChangesListLen = FS_CopyToStartStopList( pFuzzyTgt, pSegmentChangesList );
  if ( pProposalChangesList != NULL ) *plProposalChangesListLen = FS_CopyToStartStopList( pFuzzyTok, pProposalChangesList );

  // free allocated buffers and lists
  if ( pFuzzyTgt ) UtlAlloc( (PVOID *)&pFuzzyTgt, 0L, 0L, NOMSG );
  if ( pFuzzyTok ) UtlAlloc( (PVOID *)&pFuzzyTok, 0L, 0L, NOMSG );

  return  ( fOK ? usDiff : -1 );

} /* end of function FS_GetProposalDiffs */

// clear any search results
void FS_ClearResults
(
  PFSEARCHIDAX pIda
)
{
  if ( pIda->pMatchList != NULL )
  {
    UtlAlloc( (PVOID *)&(pIda->pMatchList), 0, 0, NOMSG );
    pIda->ulNumOfMatches = 0;
    pIda->ulMatchTableSize = 0;
  } /* endif */     
  if ( pIda->pNameList != NULL )
  {
    NameList_Destroy( pIda->pNameList );
    pIda->pNameList = NULL;
  } /* endif */     
  if ( pIda->pvStringBuffer != NULL )
  {
    StringBuffer_Destroy( pIda->pvStringBuffer );
    pIda->pvStringBuffer = NULL;
  } /* endif */     
}

// initialize search results
void FS_InitResults
(
  PFSEARCHIDAX pIda
)
{
  pIda->ulNumOfMatches = 0;
  pIda->ulMatchTableSize = 0;
  pIda->pNameList = NameList_Create();
  pIda->pvStringBuffer = StringBuffer_Create( );
  if ( pIda->hwnd != HWND_FUNCIF ) DELETEALL( pIda->hwnd, ID_FUZZYSEARCH_RESULT_LB );
}

// add a match to the result list
int FS_AddToResultList( PFSEARCHIDAX pIda, PFOUNDFUZZYMATCH pMatch )
{
  // enlarge result table when necessary
  if ( pIda->ulNumOfMatches >= pIda->ulMatchTableSize  )
  {
    ULONG ulIncrement = 128;
    ULONG ulOldLen = pIda->ulMatchTableSize * sizeof(FOUNDFUZZYMATCH); 
    ULONG ulNewLen = (pIda->ulMatchTableSize + ulIncrement) * sizeof(FOUNDFUZZYMATCH); 
  
    if ( UtlAlloc( (PVOID *)&(pIda->pMatchList), ulOldLen, ulNewLen, ERROR_STORAGE ) )
    {
       pIda->ulMatchTableSize += ulIncrement; 
    }
    else
    {
      return( -1 );
    } /* endif */       
  } /* endif */     

  // add new entry
  memcpy( &(pIda->pMatchList[pIda->ulNumOfMatches]), pMatch, sizeof(FOUNDFUZZYMATCH) );

  return( pIda->ulNumOfMatches++ );
}

// show a fuzzy match in the result listbox
void FS_ShowResult( PFSEARCHIDAX pIda, int iMatch )
{
  int iItem = 0;
  PFOUNDFUZZYMATCH pResult = pIda->pMatchList + iMatch;
  sprintf( pIda->szBuffer, "======%s:%s(%lu)======", NameList_GetName( pIda->pNameList, pResult->iFolderID ), 
           NameList_GetName( pIda->pNameList, pResult->iDocumentID ), pResult->ulSegNum );
  iItem = INSERTITEMEND( pIda->hwnd, ID_FUZZYSEARCH_RESULT_LB, pIda->szBuffer );
  SETITEMHANDLE( pIda->hwnd, ID_FUZZYSEARCH_RESULT_LB, iItem, iMatch );
}

// handle WM_MEASUREITEM message for our ownerdrawn listbox
void FS_MeasureItem( HWND hwnd, LONG lParam )
{
  LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam; 

  PFSEARCHIDAX pIda = ACCESSDLGIDA( hwnd, PFSEARCHIDAX );

  if ( pIda != NULL )
  {
    if ( pIda->lItemHeight != 0 )
    {
      lpmis->itemHeight = pIda->lItemHeight;
    }
    else
    {
      lpmis->itemHeight = 60; 
    } /* endif */     
  }
  else
  {
    lpmis->itemHeight = 60; 
  } /* endif */     
}


// output text with difference coloring
int FS_DrawDifferences( PFSEARCHIDAX pIda, HDC hdc, PSZ_W pszText, PFS_STARTSTOP pStartStop,  FSTEXTTYPES TextTypeNormal, FSTEXTTYPES TextTypeInserted, FSTEXTTYPES TextTypeModified )
{
  POINT pt;

  while ( pStartStop->usType != 0 )
  {
    switch ( pStartStop->usType )
    {
      case 'D' :
        break;

      case 'I' :
        if ( pIda->fWithMarks )
        {
          SetTextColor( hdc, pIda->ColorData.ColorSetting[TextTypeInserted].cForeground );
          SetBkColor( hdc, pIda->ColorData.ColorSetting[TextTypeInserted].cBackground );

        }
        else
        {
          SetTextColor( hdc, pIda->ColorData.ColorSetting[TextTypeNormal].cForeground );
          SetBkColor( hdc, pIda->ColorData.ColorSetting[TextTypeNormal].cBackground );
        } /* endif */
        TextOutW( hdc, 1, 1, pszText + pStartStop->usStart, pStartStop->usStop - pStartStop->usStart + 1 ); 						
        break;

      case 'M' :
        if ( pIda->fWithMarks )
        {
          SetTextColor( hdc, pIda->ColorData.ColorSetting[TextTypeModified].cForeground );
          SetBkColor( hdc, pIda->ColorData.ColorSetting[TextTypeModified].cBackground );
        }
        else
        {
          SetTextColor( hdc, pIda->ColorData.ColorSetting[TextTypeNormal].cForeground );
          SetBkColor( hdc, pIda->ColorData.ColorSetting[TextTypeNormal].cBackground );
        } /* endif */
        TextOutW( hdc, 1, 1, pszText + pStartStop->usStart, pStartStop->usStop - pStartStop->usStart + 1 ); 						
        break;

      default:
        SetTextColor( hdc, pIda->ColorData.ColorSetting[TextTypeNormal].cForeground );
        SetBkColor( hdc, pIda->ColorData.ColorSetting[TextTypeNormal].cBackground );
        TextOutW( hdc, 1, 1, pszText + pStartStop->usStart, pStartStop->usStop - pStartStop->usStart + 1 ); 						
        break;
    } /* endswitch */       

    pStartStop++;
  } /* endwhile */             

  SetTextColor( hdc, pIda->ColorData.ColorSetting[TextTypeNormal].cForeground );
  SetBkColor( hdc, pIda->ColorData.ColorSetting[TextTypeNormal].cBackground );

  GetCurrentPositionEx( hdc, &pt );

  return( pt.x );
}

// compute listbox item height
int FS_ComputeItemHeight( PFSEARCHIDAX pIda, HWND hwndDlg, int iControlID )
{
  HFONT hFont = 0;
  int iHeight = 0;
  HWND hwndLB = GetDlgItem( hwndDlg, iControlID );
  HDC hDC = GetDC( hwndLB );
  if ( hDC != NULLHANDLE )
  {
    TEXTMETRIC tm;
    if ( pIda->hFontControl ) 
    {
      hFont = pIda->hFontControl;
    }
    else
    {
      hFont = (HFONT)SendMessage( hwndLB, WM_GETFONT, 0, 0 );
    } /* endif */
    HFONT hFontOld = (HFONT)SelectObject( hDC, hFont );
    GetTextMetrics( hDC, &tm);
    SelectObject( hDC, hFontOld );
    iHeight = 4 * (tm.tmHeight + tm.tmExternalLeading + 1);
    ReleaseDC( hwndDlg, hDC );
  } /* endif */     
  return( iHeight );
}

// handle WM_DRAWITEM message for our ownerdrawn listbox
BOOL FS_DrawItem( PFSEARCHIDAX pIda, LONG lParam )
{
  TEXTMETRIC tm; 
  int y; 
  LPDRAWITEMSTRUCT lpdis; 

  lpdis = (LPDRAWITEMSTRUCT) lParam; 

  // If there are no list box items, skip this message. 
  if (lpdis->itemID == -1) 
  { 
      return( TRUE ); 
  } 

  HFONT hFontOld = NULL;
  
  if ( pIda->hFontControl ) hFontOld = (HFONT)SelectObject( lpdis->hDC, pIda->hFontControl );

  // Draw the bitmap and text for the list box item. Draw a 
  // rectangle around the bitmap if it is selected. 

  switch ( lpdis->itemAction ) 
  { 
    case ODA_FOCUS:
    case ODA_SELECT: 
    case ODA_DRAWENTIRE: 
      {
        // Display the bitmap associated with the item. 

        //hbmpPicture =(HBITMAP)SendMessage(lpdis->hwndItem, 
        //    LB_GETITEMDATA, lpdis->itemID, (LPARAM) 0); 

        //hdcMem = CreateCompatibleDC(lpdis->hDC); 
        //hbmpOld = SelectObject(hdcMem, hbmpPicture); 

        //BitBlt(lpdis->hDC, 
        //    lpdis->rcItem.left, lpdis->rcItem.top, 
        //    lpdis->rcItem.right - lpdis->rcItem.left, 
        //    lpdis->rcItem.bottom - lpdis->rcItem.top, 
        //    hdcMem, 0, 0, SRCCOPY); 

        // Display the text associated with the item. 
        int iMatch = SendMessage( lpdis->hwndItem, LB_GETITEMDATA, lpdis->itemID , 0L );
        if ( iMatch >= 0 )
        {
          PFOUNDFUZZYMATCH pMatch = &(pIda->pMatchList[iMatch]);
          POINT pt;
          LONG xPos;
          int extent = 0;
          BOOL fRefreshScrollExtent = FALSE;
          FSTEXTTYPES TextTypeNormal;
          FSTEXTTYPES TextTypeInserted;
          FSTEXTTYPES TextTypeModified;

          // set text types depending on item state
          if ( lpdis->itemState & ODS_SELECTED ) 
          {
            if ( lpdis->itemState & ODS_FOCUS ) 
            {
              TextTypeNormal = FS_FOCUS_TEXT;
              TextTypeInserted = FS_FOCUS_INSERTED_TEXT;
              TextTypeModified = FS_FOCUS_MODIFIED_TEXT;
            }
            else
            {
              TextTypeNormal = FS_SELECTED_TEXT;
              TextTypeInserted = FS_SELECTED_INSERTED_TEXT;
              TextTypeModified = FS_SELECTED_MODIFIED_TEXT;
            }
          }              
          else if ( pMatch->fUsed ) 
          {
            TextTypeNormal = FS_OPENED_TEXT;
            TextTypeInserted = FS_OPENED_INSERTED_TEXT;
            TextTypeModified = FS_OPENED_MODIFIED_TEXT;
          }
          else
          {
            TextTypeNormal = FS_NORMAL_TEXT;
            TextTypeInserted = FS_NORMAL_INSERTED_TEXT;
            TextTypeModified = FS_NORMAL_MODIFIED_TEXT;
          } /* endif */             

          SetTextColor( lpdis->hDC, pIda->ColorData.ColorSetting[TextTypeNormal].cForeground );
          SetBkColor( lpdis->hDC, pIda->ColorData.ColorSetting[TextTypeNormal].cBackground );

          // erase rectangle
          {
            HBRUSH hBrush = CreateSolidBrush( pIda->ColorData.ColorSetting[TextTypeNormal].cBackground );
            FillRect( lpdis->hDC, &(lpdis->rcItem), hBrush ); 
            DeleteObject( hBrush );
          }

          GetTextMetrics( lpdis->hDC, &tm ); 

          y = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;

          SetTextAlign( lpdis->hDC, TA_UPDATECP );

          // draw a single line
          MoveToEx( lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.bottom - 1, NULL );
          LineTo( lpdis->hDC, lpdis->rcItem.right, lpdis->rcItem.bottom - 1 );
          
          //MoveToEx( lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, NULL );
          //LineTo( lpdis->hDC, lpdis->rcItem.right - 2, lpdis->rcItem.top + 1 );
          //LineTo( lpdis->hDC, lpdis->rcItem.right - 2, lpdis->rcItem.bottom - 1 );
          //LineTo( lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.bottom - 1 );
          //LineTo( lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1 );

          // show folder and document info
          MoveToEx( lpdis->hDC, lpdis->rcItem.left + 4, lpdis->rcItem.top + 4, NULL );
          sprintf( pIda->szBuffer, "Folder: %s   Class: %ld   Word count: %ld   Document: %s(%lu)", NameList_GetName( pIda->pNameList, pMatch->iFolderID ), 
            pMatch->iDiffWords, pMatch->iWords, NameList_GetName( pIda->pNameList, pMatch->iDocumentID ), pMatch->ulSegNum );
          TextOut( lpdis->hDC, lpdis->rcItem.left + 4, lpdis->rcItem.top + 4, pIda->szBuffer, strlen(pIda->szBuffer) ); 						

          // show segment and proposal prefix
          MoveToEx( lpdis->hDC, lpdis->rcItem.left + 4, lpdis->rcItem.top + 4 + (tm.tmHeight + 4), NULL );
          TextOut( lpdis->hDC, 6, 22, "Segment: ", 9 ); 		
          GetCurrentPositionEx( lpdis->hDC, &pt );
          xPos = pt.x;

          MoveToEx( lpdis->hDC, lpdis->rcItem.left + 4, lpdis->rcItem.top + 4 + 2*(tm.tmHeight + 4), NULL );
          TextOut( lpdis->hDC, 1, 1, "Proposal: ", 10 ); 						
          GetCurrentPositionEx( lpdis->hDC, &pt );
          if ( pt.x > xPos ) xPos = pt.x;

          // show segment and proposal text
          MoveToEx( lpdis->hDC, xPos + 10, lpdis->rcItem.top + 4 + 1 * (tm.tmHeight + 4), NULL );
          extent = FS_DrawDifferences( pIda, lpdis->hDC, pMatch->pszSegment, pMatch->pSegmentChanges, TextTypeNormal, TextTypeInserted, TextTypeModified );

          if ( extent > pIda->iMaxExtent )
          {
            pIda->iMaxExtent = extent;
            fRefreshScrollExtent = TRUE;
          } /* endif */             

          MoveToEx( lpdis->hDC, xPos + 10, lpdis->rcItem.top + 4 + 2*(tm.tmHeight + 4), NULL );
          extent = FS_DrawDifferences( pIda, lpdis->hDC, pMatch->pszSource, pMatch->pProposalChanges, TextTypeNormal, TextTypeInserted, TextTypeModified );

          if ( extent > pIda->iMaxExtent )
          {
            pIda->iMaxExtent = extent;
            fRefreshScrollExtent = TRUE;
          } /* endif */             

          //SelectObject( hdcMem, hbmpOld ); 
          //DeleteDC( hdcMem ); 

          // has the item the focus
          if (lpdis->itemState & ODS_FOCUS) 
          { 
              // Draw a rectangle around item to indicate the focus 
              DrawFocusRect( lpdis->hDC, &(lpdis->rcItem) ); 
          } 

          // set listbox hoizontal scroll extent when required
          if ( fRefreshScrollExtent )
          {
            SendMessage( lpdis->hwndItem, LB_SETHORIZONTALEXTENT, pIda->iMaxExtent + GetSystemMetrics(SM_CXVSCROLL), 0);
          } /* endif */             
        } /* endif */             
      }
      break; 
  } 

  if ( hFontOld ) SelectObject( lpdis->hDC, hFontOld );

  return TRUE; 
} 

// open current fuzzy match in translation environment
void FS_OpenDocInTenv( PFSEARCHIDAX pIda, PFOUNDFUZZYMATCH pMatch )
{
  POPENANDPOS pOpen;

  if ( UtlAlloc( (PVOID *)&pOpen, 0L, (LONG)sizeof(OPENANDPOS), ERROR_STORAGE ) )
  {
    CHAR szDocShortName[MAX_FILESPEC];
    BOOL fIsNew = FALSE;

    pIda->fDocOpenActive = TRUE;
    SubFolNameToObjectName( NameList_GetName( pIda->pNameList, pMatch->iFolderID ), pOpen->szDocName );
    FolLongToShortDocName( pOpen->szDocName, NameList_GetName( pIda->pNameList, pMatch->iDocumentID ), szDocShortName, &fIsNew );
    strcat( pOpen->szDocName, BACKSLASH_STR );
    strcat( pOpen->szDocName, szDocShortName );
    pOpen->ulSeg = pMatch->ulSegNum;
    pOpen->usOffs= 0;
    pOpen->usLen = 0;
    pOpen->chFind[0] = 0;
    pMatch->fUsed = TRUE;

    EqfPost2Handler( DOCUMENTHANDLER, WM_EQF_PROCESSTASK, MP1FROMSHORT(OPEN_AND_POSITION_TASK), MP2FROMP(pOpen) );
  } /* endif */
}

void FSWriteSegmentWithDiffs( PFSEARCHIDAX pIda, CXmlWriter *xw, PSZ_W pszText, PFS_STARTSTOP pStartStop )
{
  int iLen = 0;
  xw->WriteString( "" );      // ensure that the start tag is terminated properly before writing text using WriteRaw()

  while ( pStartStop->usType != 0 )
  {
    switch ( pStartStop->usType )
    {
      case 'D' :
        break;

      case 'I' :
        if ( pIda->fWithMarks ) xw->WriteRaw( L"<hp1>" );
        iLen = (int)(pStartStop->usStop - pStartStop->usStart + 1);
        wcsncpy( pIda->szBufferW, pszText + pStartStop->usStart, iLen );
        pIda->szBufferW[iLen] = 0;
        xw->WriteString( pIda->szBufferW );
        if ( pIda->fWithMarks ) xw->WriteRaw( L"</hp1>" );
        break;

      case 'M' :
        if ( pIda->fWithMarks ) xw->WriteRaw( L"<hp2>" );
        iLen = (int)(pStartStop->usStop - pStartStop->usStart + 1);
        wcsncpy( pIda->szBufferW, pszText + pStartStop->usStart, iLen );
        pIda->szBufferW[iLen] = 0;
        xw->WriteString( pIda->szBufferW );
        if ( pIda->fWithMarks ) xw->WriteRaw( L"</hp2>" );
        break;

      default:
        iLen = (int)(pStartStop->usStop - pStartStop->usStart + 1);
        wcsncpy( pIda->szBufferW, pszText + pStartStop->usStart, iLen );
        pIda->szBufferW[iLen] = 0;
        xw->WriteString( pIda->szBufferW );
        break;
    } /* endswitch */       

    pStartStop++;
  } /* endwhile */             
}

BOOL FS_WriteResultsToFile( PFSEARCHIDAX pIda, PSZ pszFileName )
{
  CXmlWriter *xw = new CXmlWriter( pszFileName );
  xw->Formatting = CXmlWriter::Indented;
  xw->Encoding = CXmlWriter::UTF8;
  xw->Indention = 2;
  if ( xw->WriteStartDocument() )
  {
    xw->WriteStartElement( "FuzzySegmentSearchResults" );
    // xw->WriteStylesheet( VALXMLTOHTML_TRANSONLY_STYLESHEET );
    xw->WriteStartElement( "Header" );
    xw->WriteElementString( "Folder", pIda->szFolLongName );
    xw->WriteStartElement( "NumOfDocs" );
    xw->WriteInt( pIda->sDocCount );
    xw->WriteEndElement(); // "NumOfDocs"
    if ( pIda->iSearchMode == UPTOSELECTEDCLASS_MODE )
    {
      sprintf( pIda->szBuffer, "Up to and including class %ld", pIda->iSearchClass );
    }
    else if ( pIda->iSearchMode == ONLYSELECTEDCLASS_MODE )
    {
      sprintf( pIda->szBuffer, "Only class %ld", pIda->iSearchClass );
    }
    else
    {
      sprintf( pIda->szBuffer, "Class %ld and higher", pIda->iSearchClass );
    } /* endif */
    xw->WriteElementString( "Filter", pIda->szBuffer );
    xw->WriteEndElement(); // "Header"
    if ( pIda->ulNumOfMatches != 0 )
    {
      xw->WriteStartElement( "SegmentList" );
      for( ULONG ulI = 0; ulI < pIda->ulNumOfMatches; ulI++ )
      {
        PFOUNDFUZZYMATCH pMatch = pIda->pMatchList + ulI;
        pMatch->iDiffWords;
        xw->WriteStartElement( "Segment" );
        xw->WriteStartAttribute( "Class" );
        xw->WriteInt( pMatch->iDiffWords );
        xw->WriteEndAttribute(); // "Class"
        xw->WriteStartAttribute( "WordCount" );
        xw->WriteInt( pMatch->iWords );
        xw->WriteEndAttribute(); // "WordCount"
        xw->WriteAttributeString( "Document", NameList_GetName( pIda->pNameList, pMatch->iDocumentID ) );
        xw->WriteEndAttribute(); // "Class"
        xw->WriteStartAttribute( "SegmentNum" );
        xw->WriteInt( (int)pMatch->ulSegNum);
        xw->WriteEndAttribute(); // "SegmentNum"
        xw->WriteStartElement( "Source" );
        FSWriteSegmentWithDiffs( pIda, xw, pMatch->pszSegment, pMatch->pSegmentChanges );
        xw->WriteEndElement(); // "Source" 

        xw->WriteStartElement( "ProposalSource" );
        FSWriteSegmentWithDiffs( pIda, xw, pMatch->pszSource, pMatch->pProposalChanges );
        xw->WriteEndElement(); // "ProposalSource" 
        xw->WriteEndElement(); //  "Segment"
      } /* endfor */
      xw->WriteEndElement(); // "SegmentList"
    } /* endif */
    xw->WriteEndElement(); // "FuzzySegmentSearchResults"
    xw->WriteEndDocument();
    xw->Close();
    delete xw;
  }  
  return( TRUE );
}

BOOL FS_ExportResults( PFSEARCHIDAX pIda )
{
  BOOL fOK = TRUE;
  OPENFILENAME OpenFileName;

  memset( &OpenFileName, 0, sizeof(OpenFileName) );
  OpenFileName.lStructSize        = sizeof(OpenFileName);
  OpenFileName.hwndOwner          = pIda->hwnd;
  OpenFileName.hInstance          = NULLHANDLE;
  OpenFileName.lpstrFilter        = "XML\0*.XML\0\0\0";
  OpenFileName.lpstrCustomFilter  = NULL;
  OpenFileName.nMaxCustFilter     = 0;
  OpenFileName.nFilterIndex       = 0;
  OpenFileName.lpstrFile          = pIda->szExportFileName;
  OpenFileName.nMaxFile           = sizeof(pIda->szExportFileName);
  OpenFileName.lpstrFileTitle     = NULL;
  OpenFileName.nMaxFileTitle      = 0;
  OpenFileName.lpstrInitialDir    = NULL;
  OpenFileName.lpstrTitle         = "Export current result list";
  OpenFileName.Flags              = OFN_ENABLESIZING | OFN_EXPLORER | OFN_LONGNAMES | OFN_NODEREFERENCELINKS |
                                    OFN_NOTESTFILECREATE | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
  OpenFileName.nFileOffset        = 0;
  OpenFileName.nFileExtension     = 0;
  OpenFileName.lpstrDefExt        = "XML";
  OpenFileName.lCustData          = 0L;
  OpenFileName.lpfnHook           = NULL;
  OpenFileName.lpTemplateName     = NULL;

  fOK = GetSaveFileName( &OpenFileName ) != 0;
  if ( fOK )
  {
    fOK = FS_WriteResultsToFile( pIda, pIda->szExportFileName );
    if ( fOK )
    {
      PSZ pszParm = pIda->szExportFileName;
      UtlErrorHwnd( INFO_FS_EXPORT_COMPLETE, MB_OK, 1, &pszParm, EQF_INFO, pIda->hwnd );
    } /* endif */
  } /* endif */
  return( fOK );
} /* end of */

// helper functions to check specified parameters
BOOL FolFuncCheckFolderName
( 
  PSZ pszFolderName,         // IN: folder name
  PUSHORT pusRC,             // IN: pointer to caller's return code field (can be NULL when no return code is needed)
  PSZ     pszFolderShortName // IN: pointer to caller's buffer for the folder short name (can be NULL when no short name is needed)
)
{
  BOOL        fOK = TRUE;                // O.K. flag
  USHORT      usRC = 0;                  // error return code    
  CHAR        szShortName[MAX_FILESPEC]; // bufer for folder short name

  szShortName[0] = EOS;
  if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
  {
    fOK = FALSE;
    usRC = TA_MANDFOLDER;
    UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
  }
  else
  {
    BOOL fIsNew = FALSE;
    ObjLongToShortName( pszFolderName, szShortName, FOLDER_OBJECT, &fIsNew);
    if ( fIsNew )
    {
      fOK = FALSE;
      PSZ pszParm = pszFolderName;
      usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( pusRC != NULL ) *pusRC = usRC;
  if ( pszFolderShortName != NULL ) strcpy( pszFolderShortName, szShortName );
  
  return( fOK );
} /* endif */

BOOL FolFuncBuildFolderObjName
( 
  PSZ pszFolderShortName,    // IN: folder short name
  PSZ pszFlderObjName        // IN: caller's buffer for the folder object name
)
{
  BOOL        fOK = TRUE;                // O.K. flag
    
  // build folder object name and fill IDA fields
  if ( fOK )
  {
    UtlMakeEQFPath( pszFlderObjName, NULC, SYSTEM_PATH, NULL );
    strcat( pszFlderObjName, BACKSLASH_STR );
    strcat( pszFlderObjName, pszFolderShortName );
    strcat( pszFlderObjName, EXT_FOLDER_MAIN );
  } /* endif */

  // get folder drive to complete the folder object name
  if ( fOK )
  {
    EQFINFO     ErrorInfo;           // error code of property handler calls
    PPROPFOLDER pFolProp;            // ptr to folder properties
    PVOID       hFolProp;            // handle of folder properties

    hFolProp = OpenProperties( pszFlderObjName, NULL, PROP_ACCESS_READ, &ErrorInfo);
    if ( hFolProp )
    {
      pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
      pszFlderObjName[0] = pFolProp->chDrive;
      CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of FolFuncBuildFolderObjName */

BOOL FolFuncCheckDocumentList
( 
  PSZ pszFolderObjName,      // IN: folder object name
  PSZ pszDocuments,          // IN: list of documents (when empty all documents of the folder are added to the document name buffer)
  PUSHORT pusRC,             // IN: pointer to caller's return code field (can be NULL when no return code is needed)
  PSZ     *ppDocNameBuffer,  // IN: pointer to pointer for the document name buffer, this buffer is allocated automatically
  PULONG  pulNumOfDocs       // IN: pointer to caller's number of documents variable
)
{
  LONG lBufferSize = 0;
  LONG lBufferUsed = 0;
  PSZ pDocNameBuffer = NULL;
  USHORT usRC = 0;
  BOOL        fOK = TRUE;                // O.K. flag
  ULONG ulDocuments = 0;

  *ppDocNameBuffer = NULL;

  // check if documents exist
  if ( fOK && (pszDocuments != NULL) && (*pszDocuments != EOS))
  {
    PSZ    pszTemp = pszDocuments;    // ptr for document list processing
    PSZ    pszDocNameStart;           // ptr for document list processing
    CHAR   chTemp;                    // buffer for current character

    // isolate current document name
    pszDocNameStart = pszDocuments;
    while ( fOK && (*pszDocNameStart != EOS) )
    {
      BOOL fIsNew = FALSE;

      // isolate current document name
      {
        // skip leading whitespace and seperators
        while ( (*pszDocNameStart == ' ') || (*pszDocNameStart == COMMA) )
        {
          pszDocNameStart++;
        } /* endwhile */

        // find end of document name
        if ( *pszDocNameStart == DOUBLEQUOTE)
        {
          pszDocNameStart += 1;
          pszTemp = pszDocNameStart;
          while ( *pszTemp && (*pszTemp != DOUBLEQUOTE) )
          {
            pszTemp++;
          } /* endwhile */
          chTemp = *pszTemp;
          *pszTemp = EOS;
        }
        else
        {
          pszTemp = pszDocNameStart;
          while ( *pszTemp && (*pszTemp != COMMA) )
          {
            pszTemp++;
          } /* endwhile */
          chTemp = *pszTemp;
          *pszTemp = EOS;
        } /* endif */
      }

      if ( *pszDocNameStart != EOS)
      {
        CHAR szDocShortName[MAX_FILESPEC];

        FolLongToShortDocName( pszFolderObjName, pszDocNameStart, szDocShortName, &fIsNew );

        if ( fIsNew )
        {
          fOK = FALSE;
          PSZ pszParm = pszDocNameStart;
          usRC = ERROR_TA_SOURCEFILE;
          UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        }
        else
        {
          // add document short name to document name buffer
          LONG lAddLen = strlen(szDocShortName) + 1;
          if ( lBufferSize < (lBufferUsed + lAddLen + 1) )
          {
            UtlAllocHwnd( (PVOID *)&pDocNameBuffer, lBufferSize,
                          lBufferSize + 8096L, ERROR_STORAGE, HWND_FUNCIF );
            lBufferSize += 8096L;
          } /* endif */

          if ( pDocNameBuffer != NULL )
          {
            strcpy( pDocNameBuffer + lBufferUsed, szDocShortName );
            lBufferUsed += lAddLen;
            ulDocuments++;
          } /* endif */
        } /* endif */
      } /* endif */

      // next document name
      if ( chTemp == DOUBLEQUOTE )
      {
        *pszTemp = chTemp;
        pszDocNameStart = pszTemp + 1;
      }
      else
      {
        *pszTemp = chTemp;
        pszDocNameStart = pszTemp;
      } /* endif */
    } /* endwhile */
    if ( pDocNameBuffer != NULL )
    {
      pDocNameBuffer[lBufferUsed] = EOS; // terminate document name buffer
    } /* endif */
  }
  else
  {
    // build list of all folder documents
    ulDocuments = LoadDocumentNames( pszFolderObjName, HWND_FUNCIF, LOADDOCNAMES_INCLSUBFOLDERS, (PSZ)&pDocNameBuffer );
  } /* endif */

  if ( pusRC != NULL ) *pusRC = usRC;
  if ( pulNumOfDocs != NULL ) *pulNumOfDocs = ulDocuments;
  if ( ppDocNameBuffer != NULL ) *ppDocNameBuffer = pDocNameBuffer;

  return( fOK );
} /* end of FolFuncCheckDocumentList */

// check the specified output file name
BOOL FolFuncCheckOutputFile( PUSHORT pusRC, PSZ pszOutputFile, LONG lOptions )
{
  USHORT usRC = 0;
  BOOL        fOK = TRUE;                // O.K. flag

  if ( (pszOutputFile == NULL) || (*pszOutputFile == EOS) )
  {
    fOK = FALSE;
    usRC = ERROR_FS_MISSING_OUTPUT_NAME;
    UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
  } 
  else if ( !(lOptions & OVERWRITE_OPT) && UtlFileExist( pszOutputFile ) )
  {
    fOK = FALSE;
    PSZ pszParm = pszOutputFile;
    usRC = ERROR_FILE_EXISTS_ALREADY_BATCH;
    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
  } /* endif */

  if ( pusRC != NULL ) *pusRC = usRC;
  return( fOK );
}

// check the specified search class
BOOL FolFuncCheckClass( PUSHORT pusRC, int iClass )
{
  USHORT usRC = 0;
  BOOL        fOK = TRUE;                // O.K. flag

  if ( (iClass < 0) || (iClass > 6) )
  {
    CHAR szClass[10];
    PSZ pszParm = szClass;
    ltoa( iClass, szClass, 10 );
    fOK = FALSE;
    usRC = ERROR_FS_INVALID_CLASS;
    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
  } /* endif */

  if ( pusRC != NULL ) *pusRC = usRC;
  return( fOK );
}

// check the specified search class
BOOL FolFuncCheckMode( PUSHORT pusRC, int iMode )
{
  USHORT usRC = 0;
  BOOL        fOK = TRUE;                // O.K. flag

  if ( (iMode != UPTOSELECTEDCLASS_MODE) && (iMode != SELECTEDCLASSANDHIGHER_MODE) && (iMode != ONLYSELECTEDCLASS_MODE) )
  {
    CHAR szMode[10];
    PSZ pszParm = szMode;
    ltoa( iMode, szMode, 10 );
    fOK = FALSE;
    usRC = ERROR_FS_INVALID_MODE;
    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
  } /* endif */

  if ( pusRC != NULL ) *pusRC = usRC;
  return( fOK );
}


__declspec(dllexport)
USHORT FolFuncFuzzySearch
(
  PSZ              pszFolderName,      // name of folder
  PSZ              pszDocuments,       // list of documents or NULL 
  PSZ              pszOutputFile,      // fully qualified name of output file
  int              iSearchMode,        // search mode
  int              iClass,             // searched class
  LONG             lOptions            // processing options
)
{
  BOOL        fOK = TRUE;              // internal O.K. flag
  PFSEARCHIDAX pIda;                    // ptr to IDA of dialog
  HWND        hwndFSearchDlg;          // handle of global-find-and-change dialog
  USHORT usRC = 0;
  PSZ         pDocNameBuffer = NULL;
  ULONG       ulNumOfDocs = 0;
  
  // Allocate IDA of fuzzy search funtions
  size_t iSize = sizeof(FSEARCHIDAX);
  pIda = (PFSEARCHIDAX)malloc( iSize );
  if ( pIda != NULL )
  {
    memset( pIda, 0, iSize );
  }
  else
  {
    UtlErrorHwnd( ERROR_NOT_ENOUGH_MEMORY, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    fOK = FALSE;
  } /* end */     

   PFSEARCHIDAX pIdaX = (PFSEARCHIDAX)pIda; 



  // check provided parameters
  if ( fOK ) fOK = FolFuncCheckFolderName( pszFolderName, &usRC, pIda->szSubFolObjName );
  if ( fOK ) fOK = FolFuncBuildFolderObjName( pIda->szSubFolObjName, pIda->szFolObjName );
  if ( fOK ) fOK = FolFuncCheckDocumentList( pIda->szFolObjName, pszDocuments, &usRC, &pDocNameBuffer, &ulNumOfDocs );
  if ( fOK ) fOK = FolFuncCheckOutputFile( &usRC, pszOutputFile, lOptions );
  if ( fOK ) fOK = FolFuncCheckMode( &usRC, iSearchMode );
  if ( fOK ) fOK = FolFuncCheckClass( &usRC, iClass );

  // Fill-in IDA fields                                               
  if ( fOK )
  {
    pIda->fFromFolderList = FALSE;
    pIda->fMultipleObjs = FALSE;
    strcpy( pIda->szSubFolObjName, pIda->szFolObjName );

    if ( FolIsSubFolderObject( pIda->szSubFolObjName ) )
    {
      // convert pszFolObjName to folder object name by cutting the
      // subfolder part and property part from the name
      strcpy( pIda->szFolObjName, pIda->szSubFolObjName );
      UtlSplitFnameFromPath( pIda->szFolObjName );
      UtlSplitFnameFromPath( pIda->szFolObjName );
    }
    else
    {
      // pszFolObjName contains the folder object name already
      strcpy( pIda->szFolObjName, pIda->szSubFolObjName );
    } /* endif */

    pIda->iSearchClass = iClass;
    pIda->iSearchMode = iSearchMode;
    strcpy( pIda->szExportFileName, pszOutputFile );
    pIda->fWithMarks = lOptions & MARKDIFFERENCES_OPT;
    pIda->hwnd = HWND_FUNCIF;
    pIda->sDocCount = (SHORT)ulNumOfDocs;
  } /* endif */

  // loop over all documents and search for fuzzy segments
  if ( fOK )
  {
    PSZ pszCurDoc = pDocNameBuffer;
    strcpy( pIda->szCurFolObjName, pIda->szFolObjName );
    FS_InitResults( pIda );

    while ( fOK && (*pszCurDoc != EOS) )
    {
      BOOL fDocIsDone = FALSE;
      strcpy( pIda->szCurDoc, pszCurDoc );
      pIda->fStopSearch = FALSE;

      fOK = FS_LoadDoc( pIda, TRUE, FALSE, FALSE );

      while( fOK && !fDocIsDone )
      {
        fDocIsDone = FS_SearchFuzzy( pIda->pTBDoc, pIda );
      } /* endwhile */

      FS_CloseDoc( pIda, TRUE );

      // next document
      pszCurDoc += strlen(pszCurDoc) + 1;
    } /* endwhile */
  } /* endif */

  // export found fuzzy matches
  if ( fOK )
  {
    fOK = FS_WriteResultsToFile( pIda, pszOutputFile );
  } /* endif */

  // cleanup
  if ( pIda ) FS_ClearResults( pIda );
  if ( pDocNameBuffer ) UtlAlloc( (PVOID *)&pDocNameBuffer, 0, 0, NOMSG );;
  if ( pIda ) free( pIda );

  return( usRC );
} /* end of function FolFuncFuzzySearch */

// select a specific search mode in the search mode drop down list
void FS_SelectSearchMode( HWND hwnd, int iMode )
{
  int iItems = CBQUERYITEMCOUNT( hwnd, ID_FUZZYSEARCH_MODE_CB );
  for( int i = 0; i < iItems; i++ )
  {
    int iItemMode = CBQUERYITEMHANDLE( hwnd, ID_FUZZYSEARCH_MODE_CB, i );
    if ( iItemMode == iMode )
    {
      CBSELECTITEM( hwnd, ID_FUZZYSEARCH_MODE_CB, i );
      break;
    } /* endif */
  } /* endfor */
}
