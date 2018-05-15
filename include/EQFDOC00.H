/*! EQFDOC00.H - Internal header file for the EQF Document Handler
	Copyright (c) 1990-2015, International Business Machines Corporation and others. All rights reserved.
*/

#ifndef INCL_EQFDOC00
#define INCL_EQFDOC00

#define  ERROR_MSG_HANDLED      11111  // set dummy return code

/*------------------------------------------------------------------------------
* Definitions
*-----------------------------------------------------------------------------*/
// some size values
#define REQ_STACKSIZE   ((USHORT)1024*48)       // stacksize for threads
#define PBUFLEN         sizeof (STEQFPCMD)      // size of pipe cmd/rsp. struct
#define PCMDLEN         (PBUFLEN - EQF_SEGLEN*2)// size of pipe cmd/rsp. struct
#define MAX_DICTBUFF_SIZE       (1024*10)       // size of formatted dict.buffer


#define GETDICT_ENTRIES       1        // fill dictionary entries
#define INSERT_PROPOSAL       2        // insert the proposals
#define INSERT_DICTIONARY     3        // insert the dictionary
#define DICTIONARY_LOOKUP     4        // lookup a dictionary term
#define SHOW_PROPDICT         5        // show/hide proposal/dictionary window
#define ACT_PROPDICT          6        // activate proposal/dictionary window
#define ACT_SOURCEPROP        7        // activate source of proposal window
#define DICTIONARY_EDIT       8        // edit a dictionary term


#define  ID_TIME              23       // timer id
#define  TIMER_LPEX           80       // wait for LPEX load

#define MAX_DICT_TERMS        300

#define FUZZY_PROP            32       // fuzzy proposal
#define EXACT_PROP            16       // exact proposal
#define MACHINE_TRANS_PENDING 8        // machine trans. proposal pending
#define REPLACE_PROP          4        // replace string..
#define FUZZY_REPLACE_PROP    2        // fuzzy replace string..
#define MACHINE_TRANS_PROP    1        // machine trans. proposal
#define XLIFF_PROP            64       // XLIFF proposal
#define GLOBMEM_TRANS_PROP    128      // global memory proposal
#define GLOBMEMSTAR_TRANS_PROP 256     // global memory with star proposal
#define GLOBMEMFUZZY_TRANS_PROP 512    // global memory fuzzy proposal

#define TWBS         "TWBS"            // service class
#define TWBSRTF      "TWBSRTF"         // service class

#define TWBS_W       L"TWBS"           // service class
#define TWBSRTF_W    L"TWBSRTF"        // service class


#define EQFXLATE_START  1              // setup translate doc struct
#define EQFXLATE_DOC    2              // translate document
#define EQFXLATE_UNSEGM 3              // unsegment document

/**********************************************************************/
/* defines for autolookup returns...                                  */
/**********************************************************************/
#define DICT_DATA_PROCESSED    1       // dictionary data processed in auto look
#define DICT_IN_USE            2       // dictionary in use during auto lookup

/*------------------------------------------------------------------------------
* typedefs
*-----------------------------------------------------------------------------*/
typedef struct _STEQFSAB                        // Send Ahead Buffer
{

  BOOL          fbInUse;                        // indicator: buffer used
  ULONG         ulParm1;                        // parameter 1
  USHORT        usRC;                           // return code
  PCHAR_W       pucSourceSeg;                   // src.segment buffer
  USHORT        usLevel    [EQF_NPROP_TGTS];    // tgt. match levels
  USHORT        usNumExactBytes[EQF_NPROP_TGTS];// number of bytes matching
  USHORT        usMachineTrans[EQF_NPROP_TGTS];  // machine translation match
  EQF_BOOL      fEqualSource[EQF_NPROP_TGTS];   // source of proposal eq source
//TM_ADDRESS    stTMAddr   [EQF_NPROP_TGTS];    // TM address of matches
  USHORT        usPropCount;                    // no. of matches found
  PCHAR_W       pucTargetSegs;                  // tgt.segments buffer pointer
  USHORT        usDictCount;                    // no. of dict.src.words
  USHORT        usMaxDictlen;                   // longest dict.term (src/tgt)
  PCHAR_W       pucDictWords;                   // dict.words buffer
  BOOL          fDAReady;                       // dict. data ready
  BOOL          fMTReady;                       // MT    data ready
  PCHAR_W       pucMTSeg;                       // ptr to segment buffer
  PCHAR_W       pucPropsSegs;                   // source prop segments buffer
  CHAR          szFName[EQF_NPROP_TGTS][MAX_LONGFILESPEC];   //file name
  LONG          lDate[EQF_NPROP_TGTS];           // date proposal was translated
  LONG          ulKey[EQF_NPROP_TGTS];          // record number of proposal
  USHORT        usTargetNum[EQF_NPROP_TGTS];     // target within key
  USHORT        usDBIndex[EQF_NPROP_TGTS];      // index of TM proposal belongs to
  ULONG         ulSegNum[EQF_NPROP_TGTS];       // segment number of proposal
  USHORT        fsAvailFlags;         // indicate if other exact or fuzzy props are avail
  USHORT        usFuzzyPercents[EQF_NPROP_TGTS]; // Fuzzy percentage per proposal
  PSZ_W         pszSortPropsSeg[EQF_NPROP_TGTS];
  PSZ_W         pszSortTargetSeg[EQF_NPROP_TGTS];
  PSZ_W         pszContext;                      // ptr to context or NULL if none
  EQF_BOOL      fInvisible[EQF_NPROP_TGTS];      // TRUE if propfuzzyness < DISPLAYFUZZLEVEL
  CHAR          szUniqueKey[EQF_NPROP_TGTS][MAX_LONGFILESPEC];   // keys of proposals
  PVOID         pvMetaData;                      // ptr to metadata or NULL if none
  // fields for additional data of memory proposals
  PCHAR_W       pucPropAddData;                  // buffer for additional data of memory segments
  PSZ_W         pszSortPropsData[EQF_NPROP_TGTS];// ptr to additional data of memory segments
  PSZ_W         pszAddData;                      // ptr to additional data of current segment or NULL if none
} STEQFSAB;


typedef struct _STEQFSRCDCT                     // Source buffer
{
  CHAR_W ucbSourceSeg[ EQF_SRCLEN ];
  CHAR_W ucbDictWords[ EQF_DICTLEN];
  CHAR_W ucbMTSeg    [ EQF_SRCLEN ];            // MT proposal area
  CHAR_W szContext   [ EQF_SRCLEN ];            // segment context area
} STEQFSRCDCT;

typedef STEQFSAB        far *PSTEQFSAB;
typedef STEQFSRCDCT     far *PSTEQFSRCDCT;

typedef struct _STEQFDUMP
{
   CHAR_W  chProp[ EQF_TGTLEN ];              // buffer for proposals
   CHAR_W  chDict[ EQF_DICTLEN ];             // buffer for dictionary entries
} STEQFDUMP, * PSTEQFDUMP;


typedef struct twbsdevice
{
  /********************************************************************/
  /* TBDocument must be first ..... any others to follow              */
  /********************************************************************/
   TBDOCUMENT  tbDoc;                  // document structure
   USHORT  usType;                     // type of window
   PVOID   pDoc;                       // pointer to document
} TWBSDEVICE, * PTWBSDEVICE;


typedef struct stHDW
{
   ULONG   ulSegNum;                   // number of segment
   PSZ_W   pData;                      // pointer to HDW data
} STHDW, * PSTHDW;


// move this into document ida
typedef struct _stdictword
{
   USHORT usNum;                                 // number of dictionary
   USHORT usOffset;                              // offset
   USHORT usType;                                // type of word - hdw/target
   USHORT usLength;                              // length of word
   USHORT usDispLen;                             // length needed in display
   CHAR_W chStylePrefix;                         // style prefix or zero if none
} STDICTWORD, * PSTDICTWORD;

typedef struct _startstopprop
{
  USHORT usStartSeg;                             // start segment
  USHORT usLastSeg;                              // last segment
} STARTSTOPPROP, * PSTARTSTOPPROP;

typedef struct _TWBSDISPSTRUCT
{
  STDICTWORD stDictWord[ MAX_DICT_TERMS ];    // maximum dict. terms
  BOOL       fStatusLine[ MAX_DICT_TERMS ];   // status of line /TRUE = HDW
  STEQFSAB   stEQFExt;                        // sab used for extract
  CHAR_W     szDictBuffer[MAX_DICTBUFF_SIZE]; // space for dict window
  CHAR_W     szPrefix[10000];                 //  space for prefixes
  PSZ_W      pszPrefix[10];                   // pointer to starts
  CHAR_W     chInsertSource[ 22000 ];         // buffer for insert source (must be öarge enough to contain 10 segments!)
  CHAR       szNoDictEntry[128] ;             // display buffer for no dict ent.
  CHAR_W     chNormSegment[ EQF_SEGLEN ];     // array for normalized segment
  PSTEQFSAB  pstEQFSabDict;                   // ptr to current SAB element
  STARTSTOPPROP StartStopProp[ 10 ];             // proposal identification
  USHORT     usLCSModWords;                   // num of diff. words to best src-of-prop
  USHORT     usIndexInEqfSAB[10];             // index of visible prop in stEQFSab struct
                                              // (stEQFSAB contains invisible proposals too!)
} TWBSDISPSTRUCT, * PTWBSDISPSTRUCT;

  typedef EQF_BOOL (*PFN_EQFMTINIT)( PSZ, PSZ, USHORT, USHORT, PSZ );
  typedef EQF_BOOL (*PFN_EQFMTTRANS)( PSZ, USHORT, PSZ, PSZ, PSZ );
  typedef EQF_BOOL (*PFN_EQFMTCLOSE)( PSZ );
  typedef EQF_BOOL (*PFN_EQF_XSTART)( PSZ, PSZ, PSZ, PSTEQFGEN);
  typedef EQF_BOOL (*PFN_EQF_XSTOP)( PSZ, PSTEQFGEN,  BOOL);
  typedef EQF_BOOL (*PFN_EQF_XSTARTEX)( PSZ, PSZ*, PSTEQFGEN);

struct add_docu_ida{
    CHAR     szDocFormat[MAX_FILESPEC];        // format / Tag Table
    CHAR     szDocMemory[MAX_LONGFILESPEC];    // Translation Memory
    CHAR     szDocSourceLang[MAX_LANG_LENGTH]; // source language
    CHAR     szDocTargetLang[MAX_LANG_LENGTH]; // target language
    CHAR     szDocLongName[MAX_LONGFILESPEC];  // document long name
    CHAR     szAlias[MAX_LONGFILESPEC];        // document alias name
    CHAR     szShortAlias[MAX_FILESPEC];       // short alias name
    CHAR     szEditor[MAX_FILESPEC];           // document editor
    OBJNAME  szFolderObjName;                  // object  folder
    char aDicts[NUM_OF_FOLDER_DICS][MAX_LONGFILESPEC];  // folder's list of dictionaries
    char aMemDBs[MAX_NUM_OF_READONLY_MDB][MAX_LONGFILESPEC];  // folder's list of mems
    HPROP        hpropFolder;             //handel of folder properties
    CHAR          szGlobalMemOptFile[MAX_LONGFILESPEC]; // name of global memory option file
};
typedef struct add_docu_ida ADDDOC_IDA, *PADDDOC_IDA;


struct docu_ida {
    IDA_HEAD     IdaHead;                 //common header of instance area
    PPROPEDIT    ppropEdit;               //pointer to editor properties
    HPROP        hpropEdit;               //handle of editor properties
    HPROP        hpropFolder;             //handel of folder properties
    PPROPDOCUMENT ppropDoc;               //pointer to document properties
    HWND         hwndEditorSrcParent;     //parent of editor source window
    HWND         hwndEditorTgtParent;     //parent of editor target window
    HWND         hwndEditorSrcOwner;     //owner  of editor source window
    HWND         hwndEditorTgtOwner;     //owner  of editor target window
    HWND         hwndEditorTgt;           //handle of editor target window
    HWND         hwnd;                    //client handle
    HWND         hwndActiveFolder;        //handle of active folder
    OBJNAME      szFolderObjName;         //object name of target folder
    OBJNAME      szEditObjName;           //object name of editor properties
    CHAR         szFolderName[MAX_FILESPEC]; //name of folder
    CHAR         szDocName[MAX_FILESPEC];    //document name
    CHAR         szSource[MAX_PATH144];      //full qualified source document
    CHAR         szSegSource[MAX_PATH144];   //full qualified segm. source document
    CHAR         szSegTarget[MAX_PATH144];   //full qualified segm. target document
    CHAR         szMemory[EQF_MAX_TM_BASES+1][MAX_LONGFILESPEC]; //full qualified memory
    CHAR         szDicts[NUM_OF_FOLDER_DICS+1][MAX_LONGFILESPEC];//list of folder dicts.
    //the next two are used in BuildGenericStructure
    CHAR         szDicPath[MAX_PATH144];     //buffer dictionary path
    CHAR         szString[NUM_OF_FOLDER_DICS * MAX_FILESPEC]; //string buffer
    CHAR         szBuf[3*MAX_LONGPATH];       //buffer
    CHAR         szEditDll[MAX_FILESPEC]; // editor dll name
    PFN_EQF_XSTART pfnEQF_XStart;         // start and end function for edit
    PFN_EQF_XSTOP  pfnEQF_XStop;
    PSTEQFGEN      pstEQFGen;             //ptr to requ. handlers generic struc.
    PSTEQFSRCDCT   pstEQFSrcDct;          // dictionary data
    PCHAR_W        pucEQFTgt;             // buffer for target
    STEQFSAB      stEQFSab [EQF_NSAB];    // send ahead buffer (control)
    PSTEQFSAB     pstEQFQDA;              // queue command for DA structure
    STEQFSAB      stEQFQCmd;              // queue command structure
    BOOL          fSemDAProc;             // da ready - waiting for work
    HWND          hwndSource;             // HWND of prop.source
    HWND          hwndProposals;          // handle of proposals window
    HWND          hwndDictionary;         // handle of dictionary window
    TWBSDEVICE    tbDevDictionary;        // device contexts for dictionary
    TWBSDEVICE    tbDevProposal;          // device contexts for proposal
    TWBSDEVICE    tbDevSource;            // device contexts for source of props
    HUCB          hUCB;                   // user control block handle
    HDCB          hDCB;                   // dictionary control block handle
    HLUPCB        hLUPCB;                 // Lookup services control block
    PVOID         pvMemLookup;            // pointer to data area for memory lokup
    PVOID         pvUnused;               // unused pointer 
    PVOID         pvUnused2[EQF_MAX_TM_BASES+1]; // former memory database handle array
    unsigned short fRunTM:         1;      // TM running
    unsigned short fRunDA:         1;      // DA running
    unsigned short fForeground:    1;      // indicator for fg.seg present
    unsigned short fRunMT;                 // MT is running
    PVOID         pStackDA;               // DA thread stack
    USHORT        usEI, usFI;             // entry/foreground index
    STHDW         stHDW[ MAX_DICT_TERMS ];// array for headword cross reference
    USHORT        usDAError;              // dictionary access error
    CHAR_W        szMsgBuffer[EQF_NAMELEN];// message buffer
    ULONG         fsConfig;               // currently displayed windows
    SHORT         sSrcLanguage;           // source language
    SHORT         sTgtLanguage;           // language of target
    USHORT        ausDummy[5];            // dummy
    BOOL          fDocLocked;             // document locked
    BOOL          fDictLocked;            // dictionary locked
    BOOL          fDAInsert;              // insert dictionary entries pending
    TID           tidDA;                  // thread ID for dictioanry thread
    /******************************************************************/
    /* now the MT defines are following                               */
    /******************************************************************/
    TID           tidMT;                  // MT process
    BOOL          fSemMTProc;             // mt ready - waiting for work
    PSTEQFSAB     pstEQFQMT;              // queue command for MT structure
    USHORT        usSrcLang;              // source language identifier
    USHORT        usTgtLang;              // target language identifier
    CHAR_W        szMTBuffer[EQF_NAMELEN];// MT message buffer
    PFN_EQFMTINIT pfnEQFMTInit;
    PFN_EQFMTTRANS pfnEQFMTTrans;
    PFN_EQFMTCLOSE pfnEQFMTClose;
    CHAR          szMTDll[MAX_FILESPEC];  // MT dll name
    PVOID         pStackMT;               // MT stack
    BOOL          fMTInsert;              // MT translation pending
    BOOL          fNoMTProp;              // MT should not be inserted if avail
    HWND          hwndFocusWnd;           // handle of the focus window...
    PUSEROPT      pUserSettings;          // user settings...
    /******************************************************************/
    /* the settings for the proposal area                             */
    /******************************************************************/
    CHAR_W        chStartFlag[2];         // the '['
    CHAR_W        chEndFlag[2];           //     ']'
    CHAR_W        chFFlag[2];             //     'F'
    CHAR_W        chRFlag[2];             //     'R'
    CHAR_W        chMFlag[2];                    //     'M' of the TM prop wnd.
    /******************************************************************/
    /* New stuff by XQG for shutting down 'under-cover' analysis      */
    /******************************************************************/
    BOOL          fAnalysisIsRunning;    // analysis-is-up-and-running flag
    PEQF_BOOL     pfKillAnalysis;        // pointer to ta input area
    /******************************************************************/
    /* Memory, format and languages of document returned by           */
    /* DocQueryInfo call                                              */
    /******************************************************************/
    CHAR          szDocFormat[MAX_FILESPEC];        // format / Tag Table
    CHAR          szDocMemory[MAX_LONGFILESPEC];    // Translation Memory
    CHAR          szDocSourceLang[MAX_LANG_LENGTH]; // source language
    CHAR          szDocTargetLang[MAX_LANG_LENGTH]; // target language
    TWBSDISPSTRUCT stTWBS;                          // twbs struct ...
    HMENU         hTPROMenu;                        // handle of TPRO Menu
    HMENU         hTPROWndMenu;                     // handle of TPRO Wnd. Menu
    HWND          hWaitDlg;                         // wait dialog of analysis
    POPENANDPOS   pOpenAndPos;                   // pointer to open and pos struct..
    PVOID         pTMBestMatch;                            // ptr to struct to merge props
    /******************************************************************/
    /* Document long name, alias and editor as returned by            */
    /* DocQueryInfo2 call                                             */
    /******************************************************************/
    CHAR          szDocLongName[MAX_LONGFILESPEC];// document long name
    CHAR          szAlias[MAX_LONGFILESPEC];     // document alias name
    CHAR          szShortAlias[MAX_FILESPEC];    // short alias name
    CHAR          szEditor[MAX_FILESPEC];        // document editor
    ULONG         ulSegFileDate;       // date of last save of seg target
    PPOOL          pDocNamePool;       // list of selected objects..
    PSZ            *apszDocNames;      // ptr to ptr array with document names
    USHORT         usDocNamesAllocated;// space for number of documents allocated
    USHORT         usDocNamesUsed;     // space for number of documents allocated
    PFN_EQF_XSTARTEX pfnEQF_XStartEx;  // start and end function for edit
    BOOL           fNewDictAccess;     // new dictionary access
    BOOL           fEQFM_DOC_IS_SAVED; // in processing of EQFM_DOC_IS_SAVED msg
    CHAR         szLongFolderName[MAX_LONGFILESPEC];       // long name of folder
    CHAR_W         chDictPrefixes[ALL_LETTERS + 1];
    SHORT          sDictPrefixPage ;   // PgUp/PgDn in dict window
    USHORT         usEditor;           // Displ. control to be used
    BOOL           fTWBSPopup;         // proposal window popup
    // values of last used segment in editor API calls
    ULONG          ulLastSegNum;         // segment number
    BOOL           fLastSegMTMatch;      // MT match flag
    CHAR           szLastSegDoc[MAX_LONGFILESPEC]; // document name
    CHAR           szLastSegMarkup[MAX_FILESPEC];  // markup name
    BOOL           fSTargetInUnicode;    // TRUE if starget file is unicode
    ULONG          ulTgtOemCP;           // CP of sTgtLanguage/szDocTargetLang
    ULONG          ulSrcOemCP;           // CP of sSrcLanguage/szDocSourceLang
    BOOL           fStopAtFirstExact;    // stop at first exact match flag from folder properties
    CHAR           szGlobalMemOptFile[MAX_LONGFILESPEC]; // name of global memory option file
    PVOID          pvGlobalMemOptFile;   // loaded global memory option file
};
typedef struct docu_ida DOCUMENT_IDA, *PDOCUMENT_IDA;

typedef struct _stFuzzyOrder
{
   USHORT   usIndex;
   USHORT   usFuzzyPercents;
   USHORT   usLevel;
   USHORT   usNumExactBytes;
   USHORT   usMachineTrans;
   EQF_BOOL fEqualSource;
   CHAR     szFName[MAX_LONGFILESPEC];
   LONG     lDate;
   LONG     ulKey;
   USHORT   usTargetNum;
   USHORT   usDBIndex;
   ULONG    ulSegNum;
   PSZ_W      pszPropsSeg;         // ptrs to src-of-props
   PSZ_W      pszTargetSeg;        // ptrs to props
   EQF_BOOL   fInvisible;          // TRUE if not displayed in mem.wnd
   PSZ_W      pszAddData;          // ptrs to segment additional data
} STFUZZYORDER, * PSTFUZZYORDER;

/*-----------------------------------------------------------------------------*
* Function prototypes
*-----------------------------------------------------------------------------*/
// *** EQF_SUBS ***
__declspec(dllexport) USHORT  EQF_Allocate    (BOOL);
__declspec(dllexport) VOID    EQFDA           (PVOID);
__declspec(dllexport) VOID    EQFMT           (PVOID);

// prototypes for extended access to services


__declspec(dllexport) USHORT EQFXINIT (PSTEQFGEN pstEQFGen, PSZ *apszTlm, PSZ *apszLDict);
__declspec(dllexport) USHORT EQFXTRANSSEG ( PSTEQFGEN pstEQFGen, PSZ pszSrc, USHORT usSegNum, EQF_BOOL fForeground, USHORT fsFlags, PSHORT psExact );
__declspec(dllexport) USHORT EQFXCLEAR ( PSTEQFGEN pstEQFGen, USHORT fsFlags );
__declspec(dllexport) USHORT EQFXCLOSE ( PSTEQFGEN pstEQFGen, EQF_BOOL fShutdown );
__declspec(dllexport) USHORT EQFXGETPROP ( PSTEQFGEN pstEQFGen, USHORT usNum, PSZ pszBuffer, PUSHORT pusLevel );
__declspec(dllexport) USHORT EQFXSAVESEG ( PSTEQFGEN pstEQFGen, PSZ pszSrc, PSZ pszTgt, USHORT usSegNum );
__declspec(dllexport) PSZ    EQFXERRINS ( PSTEQFGEN pstEQFGen );
__declspec(dllexport) PSZ_W  EQFXERRINSW( PSTEQFGEN pstEQFGen );
__declspec(dllexport) PSZ    EQFXERRMSG ( PSTEQFGEN pstEQFGen );
__declspec(dllexport) USHORT EQFXDELSEG ( PSTEQFGEN pstEQFGen, PSZ pszSrc,PSZ pszTgt, USHORT usSegNum );
__declspec(dllexport) USHORT EQFXGETSEGNUM ( PSTEQFGEN pstEQFGen, PULONG pulSegNum );
__declspec(dllexport) USHORT EQFXEXTSEG ( PSTEQFGEN pstEQFGen, PSZ pszBuffer, USHORT usNum );
__declspec(dllexport) USHORT EQFXDICTLOOK ( PSTEQFGEN pstEQFGen, PSZ pszSegBuf, PSZ pszWord, USHORT usPos, EQF_BOOL fSource );
__declspec(dllexport) USHORT EQFXDICTEDIT ( PSTEQFGEN pstEQFGen, PSZ pszWord );
__declspec(dllexport) USHORT EQFXGETDICT ( PSTEQFGEN pstEQFGen, USHORT usNum, PSZ pszBuffer);
__declspec(dllexport) USHORT EQFXGETSOURCE ( PSTEQFGEN pstEQFGen, USHORT usAction );
__declspec(dllexport) USHORT EQFXDUMPSEG (PSTEQFGEN pstEQFGen, PSZ pszSrc,PSZ pszFileName,USHORT usSegNum, USHORT usFlags,PCHAR pchProp,  PCHAR pchDict );
__declspec(dllexport) USHORT EQFXPROOF ( PSTEQFGEN, PSZ, PCHAR, PUSHORT);
__declspec(dllexport) USHORT EQFXPROOFAID ( PSTEQFGEN, PSZ, PCHAR, PUSHORT);
__declspec(dllexport) USHORT EQFXPROOFADD ( PSTEQFGEN, PSZ );
__declspec(dllexport) USHORT EQFXADDABBREV ( PSTEQFGEN, PSZ );

/*------------------------------------------------------------------------------
* EQF Msg Commands
*-----------------------------------------------------------------------------*/
#define WM_EQFCMD                WM_EQF_END        // start for this
#define WM_EQFCMD_INIT           WM_EQFCMD+ 0      // initialize TWBS session
#define WM_EQFCMD_CLEAR          WM_EQFCMD+ 1      // clear buffers and queues
#define WM_EQFCMD_CLOSE          WM_EQFCMD+ 2      // close TWBS session
#define WM_EQFCMD_GETPROP        WM_EQFCMD+ 3      // get TM proposal
#define WM_EQFCMD_GETDICT        WM_EQFCMD+ 4      // get dictionary match
#define WM_EQFCMD_DICTLOOK       WM_EQFCMD+ 5      // invoke dict.lookup dialog
#define WM_EQFCMD_SAVESEG        WM_EQFCMD+ 6      // save segment to TM
#define WM_EQFCMD_TRANSSEG       WM_EQFCMD+ 7      // translate a segment
#define WM_EQFCMD_DELSEG         WM_EQFCMD+ 8      // delete a segment from TM
#define WM_EQFCMD_EXTSEG         WM_EQFCMD+ 9      // extract segment from TM
#define WM_EQFCMD_DUMPSEG        WM_EQFCMD+10      // dump segment from TM

#define WM_EQFCMD_PROOF          WM_EQFCMD+11      // proofread a segment
#define WM_EQFCMD_PROOFADD       WM_EQFCMD+12      // add word to addenda
#define WM_EQFCMD_PROOFAID       WM_EQFCMD+13      // get word help for a word
#define WM_EQFCMD_GETSOURCE      WM_EQFCMD+14      // display source of proposal
#define WM_EQFCMD_DICTEDIT       WM_EQFCMD+15      // invoke dict.edit dialog
#define WM_EQFCMD_ADDABBREV      WM_EQFCMD+16      // add an abbrevation ..
#define WM_EQFCMD_DELPROPOSAL    WM_EQFCMD+17      // delete a proposal ..
#define WM_EQFCMD_XDOCACT        WM_EQFCMD+18      // activate a new document settings
#define WM_EQFCMD_XDOCADD        WM_EQFCMD+19      // add a new document to ring
#define WM_EQFCMD_XDOCREMOVE     WM_EQFCMD+20      // remove a document from ring
#define WM_EQFCMD_XDOCNEXT       WM_EQFCMD+21      // iterate to next document in chain
#define WM_EQFCMD_XDOCNUM        WM_EQFCMD+22      // return i-th document (i=0..n)
#define WM_EQFCMD_XDOCINLIST     WM_EQFCMD+23      // check if specified doc part of list
#define WM_EQFCMD_WORDCNTPERSEG  WM_EQFCMD+24      // count words in segment
#define WM_EQFCMD_GETSEGNUM      WM_EQFCMD+25      // get segment number

// *** EQF_REQ/EQF_REQF ***
__declspec(dllexport) VOID    EQFR_Init       (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_Clear      (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_ClearDictUpd  (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_Close      (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_GetProp    (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_GetDict    (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_DictLook   (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_DictEdit   (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_SaveSeg    (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_TransSeg   (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_DelSeg     (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_GetSegNum  (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_ExtSeg     (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_DumpSeg     (PDOCUMENT_IDA);
__declspec(dllexport) VOID    EQFR_ProofSeg    (PDOCUMENT_IDA);    // proof read segment
__declspec(dllexport) VOID    EQFR_ProofAdd    (PDOCUMENT_IDA);    // add to addenda
__declspec(dllexport) VOID    EQFR_ProofAid    (PDOCUMENT_IDA);    // get  help for misspelled word
__declspec(dllexport) VOID    EQFR_GetSource   (PDOCUMENT_IDA);    // display source of proposal wind.
__declspec(dllexport) VOID    EQFRepaint       (PDOCUMENT_IDA);    // repaint windows ....
__declspec(dllexport) VOID    EQFR_AddAbbrev   (PDOCUMENT_IDA);    // add to abbreviation dictionary
__declspec(dllexport) VOID    EQFR_XDocAct     (PDOCUMENT_IDA);    // activate specified doc with TM and Dict
__declspec(dllexport) VOID    EQFR_XDocAdd     (PDOCUMENT_IDA);    // add specified doc to ring
__declspec(dllexport) VOID    EQFR_XDocRemove  (PDOCUMENT_IDA);    // remove specified doc from ring
__declspec(dllexport) VOID    EQFR_XDocNext    (PDOCUMENT_IDA);    // iterate to next document in ring
__declspec(dllexport) VOID    EQFR_XDocNum     (PDOCUMENT_IDA);    // return i-th document (i=0..n)
__declspec(dllexport) VOID    EQFR_XDocInList  (PDOCUMENT_IDA);    // check if document in list
__declspec(dllexport) VOID    EQFR_WordCntPerSeg(PDOCUMENT_IDA);   // count words in segment


// *** EQF_TM ***
__declspec(dllexport) VOID    EQFTM      (PDOCUMENT_IDA, USHORT, PSTEQFSAB);
__declspec(dllexport) VOID    EQFDAInit ( PDOCUMENT_IDA pDoc );
__declspec(dllexport) VOID    EQFDAEntries( PDOCUMENT_IDA pDoc );
__declspec(dllexport) VOID    EQFDAClose ( PDOCUMENT_IDA pDoc );


/* Export window proc */
__declspec(dllexport) MRESULT APIENTRY DOCUMENTHANDLERWP( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport) MRESULT APIENTRY DOCUMENTWP( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport) MRESULT APIENTRY TWBSERVICEWP( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport) MRESULT APIENTRY TWBSERVICEWPRTF( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport) MRESULT APIENTRY TWBSERVICEWPRTFCTRL( HWND, WINMSG, WPARAM, LPARAM);

__declspec(dllexport) BOOL InitMFE( PDOCUMENT_IDA );
__declspec(dllexport) BOOL CheckResources( PDOCUMENT_IDA );

__declspec(dllexport) VOID    EQFMTInit       (PDOCUMENT_IDA);

__declspec(dllexport) BOOL   DocLoad ( PSZ   pFolder, PSZ   pInFile );
__declspec(dllexport) BOOL   DocUnSegm ( PSZ   pFolder, PSZ   pInFile, PSZ pOutFile );
__declspec(dllexport) MRESULT DocXCreateInst ( HWND hwnd, PSZ  pszObjName );

__declspec(dllexport) BOOL EQFOverTransWindow ( PDOCUMENT_IDA, LONG, LONG );


#define TOP_OF_LIST     0
#define BOTTOM_OF_LIST  1
__declspec(dllexport) BOOL AddDocToList ( PDOCUMENT_IDA, PSZ, USHORT );
__declspec(dllexport) BOOL QueryDocInList ( PDOCUMENT_IDA, PSZ, PUSHORT );
__declspec(dllexport) VOID RemoveDocFromList( PDOCUMENT_IDA, PSZ );
__declspec(dllexport) BOOL FindNextDocInList ( PDOCUMENT_IDA, PSZ, PSZ );
__declspec(dllexport) BOOL FindDocInList ( PDOCUMENT_IDA, USHORT, PSZ );
__declspec(dllexport) BOOL BuildGenericStructure ( PDOCUMENT_IDA, PSTEQFGEN, PSZ *, PSZ *, BOOL );
__declspec(dllexport) USHORT  ActivateASD     ( PDOCUMENT_IDA );
__declspec(dllexport) VOID SetDocWindowText ( PDOCUMENT_IDA, HWND, PSZ );

__declspec(dllexport) BOOL SetDictAccess ( PSZ );           // set dictionary access
__declspec(dllexport) VOID ReleaseDictAccess ( PSZ );       // release dictionary access

__declspec(dllexport) USHORT CreateTWBSInst ( HWND  hwnd );
__declspec(dllexport) USHORT FreeTWBSInst ( HWND  hwnd );
__declspec(dllexport) VOID CleanUp ( HWND );                                     // free resources

__declspec(dllexport) HKL    SwitchKeyboardForSpecLangs( PDOCUMENT_IDA pIda );

__declspec(dllexport) USHORT GetLangID( PDOCUMENT_IDA pIda );

#endif

__declspec(dllexport) USHORT EqfGetPropState (PSTEQFGEN pstEqfGen, USHORT usProp);
__declspec(dllexport) USHORT CalcPercent(USHORT usDifferent, USHORT usTotal);
__declspec(dllexport) VOID EQFBDocSetCodePage( PTBDOCUMENT pNewdoc, PDOCUMENT_IDA pIda);
__declspec(dllexport) USHORT   EqfGetPropFuzzyness(PSTEQFGEN pstEqfGen, PULONG pulFuzzyness);
//   End of EQFDOC00.H
//ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß
