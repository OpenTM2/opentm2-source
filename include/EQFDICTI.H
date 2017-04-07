//+----------------------------------------------------------------------------+
//|  EQFDICTI.H - Private header file for the EQF Dictionary Handler           |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.2 $ ----------- 12 Apr 2010
// GQ: - added szImportDir and ulImportDirLength to DIMPIDA
// 
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.2 $ ----------- 15 Nov 2009
// GQ: - increased MAXI_SIZE to fix S618014 Error when importing SGM dictionary
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
// $Revision: 1.2 $ ----------- 25 Nov 2005
// GQ: - added szToExternalFileText and fYesToAll to DEXIDA
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.2 $ ----------- 19 Jul 2005
// GQ: - added hwndToCombo to DIMPIDA
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
// $Revision: 1.2 $ ----------- 3 Mar 2004
// GQ: - added fSkipped flag to DEXIDA
//     - added ulSkipped and apszField to DORGIDA
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
// $Revision: 1.2 $ ----------- 29 Jul 2002
// --RJ: R07197: add ulOemCP, ulAnsiCP to DEXIDA and DIMPIDA;
//      this CP is used during dict export and import
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.11 $ ----------- 17 Dec 2001
// GQ: - Added buffers for the names of the model dictionaries
//
//
// $Revision: 1.10 $ ----------- 7 Dec 2001
// GQ: - Add lpucNewDataSize to DEXIDA
//
//
// $Revision: 1.9 $ ----------- 27 Nov 2001
// Dictionary description export/import
//
//
// $Revision: 1.8 $ ----------- 26 Nov 2001
// GQ: - Added pszList and pszActFile to DIMPIDA
//
//
// $Revision: 1.7 $ ----------- 9 Nov 2001
// GQ: Added Dictionary list parameter to DictionaryOrganize and DictionaryExport
//
//
// $Revision: 1.6 $ ----------- 22 Oct 2001
// -- RJ: move DorgInit() prototype
//
//
// $Revision: 1.5 $ ----------- 19 Oct 2001
// --RJ: unicode enabling in import/export dict structures
//
//
// $Revision: 1.4 $ ----------- 9 Oct 2001
// GQ: Added SELDICTINFO structure
//
//
// $Revision: 1.3 $ ----------- 13 Sep 2001
// long description added in PDEXIDA
//
//
// $Revision: 1.2 $ ----------- 3 Sep 2001
// GQ: Moved prototype of DicGetCheckPassword to EQF.H
//
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.9 $ ----------- 9 Aug 2001
// Changed TrojaResultBuf to TrojaResultBufLong
//
//
//
// $Revision: 1.8 $ ----------- 28 May 2001
// GQ: Moved DictionaryImport and DictionaryOrganize to EQF.H
//
//
//
// $Revision: 1.7 $ ----------- 19 Feb 2001
// -- Thai: add fAnsiConv to DEXIDA
//
//
// $Revision: 1.6 $ ----------- 13 Oct 2000
// - added usExpFormat to DEXIDA
// - added usImpFormat to DIMPIDA
//
//
// $Revision: 1.5 $ ----------- 18 Sep 2000
// - fixed PTM KBT0847: Dictionary import fails when path is too long
//
//
// $Revision: 1.4 $ ----------- 30 Aug 2000
// -- added necessary definitons for dictionary property sheets
//
//
// $Revision: 1.3 $ ----------- 10 Jan 2000
// - support long dictionary names
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFDICTI.HV_   1.5   14 Jan 1998 14:24:20   BUILD  $
 *
 * $Log:   J:\DATA\EQFDICTI.HV_  $
 *
 *    Rev 1.5   14 Jan 1998 14:24:20   BUILD
 * - changed MAX_PATH to MAX_PATH144 or MAX_LONGPATH
 *
 *    Rev 1.4   16 May 1997 15:16:06   BUILD
 * - implementation of work item A.44: new API functions (here: defines for
 *   batch dictionary exporet)
 *
 *    Rev 1.3   16 Dec 1996 09:39:30   BUILD
 * - added LAN drive list ot DICCREATEIDA and removed obsolete server stuff
 *
 *    Rev 1.2   09 Sep 1996 11:04:28   BUILD
 * - added new define NEWFOLIMP for dictionary property dialog called by
 *   dictionary import via folder import
 *
 *    Rev 1.1   13 Jan 1996 13:59:50   BUILD
 * - added prototype for function DicLoadFieldNames
 * - added fAutLookup flag to MAPENTRY structure and CODETAGS structure
 * - added AUTLOOKUP token (string)
 *
 *    Rev 1.0   09 Jan 1996 09:50:08   BUILD
 * Initial revision.
*/
#define  MIN_DICT_SPACE    50000   //min. disk space needed for dict creation
#define  HPS_HEADLINE_cy   25
#define  DICT_PROPERTIES_NAME "EQFDICT.PRP" // name of dictionary list props
#define  EXT_OF_DICPROP ".PRO"     // extension of dictionary properties
#define  COMPLETED_TASK   USER_TASK + 1

//#define MAX_FIELDS_FOR_DICT 38    // max number of fields per dictionary
                                  // (is dictated by entryfield limit in
                                  //  dictionary edit)
// GQ 2017/01/19 there is no restriction for the number of fields in a dialog anymore, so we could increase the max field value
#define MAX_FIELDS_FOR_DICT 100    // max number of fields per dictionary 


//new dialog flag to show if entry structure given via sgml or not
#define PROPS             1
#define NEW               2
#define REM_DEL           3
#define LOCAL_DEL         4
#define NEWFOLIMP         5

// macro to create dictionary property name out of dictionary name
#define PROPNAME( pszBuffer, pszDict )                       \
{                                                            \
   UtlMakeEQFPath( pszBuffer, NULC, SYSTEM_PATH, NULL );     \
   strcat( pszBuffer, "\\" );                                \
   Utlstrccpy( pszBuffer + strlen(pszBuffer), pszDict, '.' );\
   strcat( pszBuffer, EXT_OF_DICPROP );                      \
}

// macro to create fully qualified dictionary name
#define DICTNAME( pszBuffer, chDrive, pszDict )              \
{                                                            \
   UtlMakeEQFPath( pszBuffer, chDrive, DIC_PATH, NULL );     \
   strcat( pszBuffer, "\\" );                                \
   Utlstrccpy( pszBuffer + strlen(pszBuffer), pszDict, '.' );\
   strcat( pszBuffer, EXT_OF_DIC );                          \
}

//Dictionary structure for properties passed to new dict dialog
typedef struct _DICTPARMS
{
   USHORT   usType;
   PSZ      pszData;
} DICTPARMS, *PDICTPARMS;

//Dictionary delete structure
typedef struct _DICTDEL
{
   USHORT   usType;
   PSZ      pszName;
} DICTDEL, *PDICTDEL;

//DictionaryHandler Instance Data Area
typedef struct _DICTIONARYIDA
{
    IDA_HEAD     IdaHead;              // common header of instance area
    HWND         hLBox;                // list box wnd handle
    CHAR         szColumnTitles[256];  // column titles
    SHORT  sLastUsedViewList[MAX_VIEW];// last used view list
    SHORT  sDetailsViewList[MAX_VIEW]; // user's details view list
    SHORT  sSortList[MAX_VIEW];        // user's sort criteria list
    HDCB   hDict[NUM_OF_FOLDER_DICS];  // handles for asd plus index file(s)
    HUCB   hUser[NUM_OF_FOLDER_DICS];  // ASD user handles(max 10)
    CHAR   szLookupDictName[NUM_OF_FOLDER_DICS][MAX_PATH144]; //symbol name of dicts
    HLUPCB hLookup[NUM_OF_FOLDER_DICS];   // ASD lookup handle
    USHORT       usLUPID[NUM_OF_FOLDER_DICS];   // ASD lookup ID
    RECTL        rclDispPos;           //ptr to display window position
    USHORT       usDictNum;            //amount of open dictionaries
    CHAR   szLongName[MAX_LONGFILESPEC]; // buffer for dict long names

} DICTIONARYIDA, *PDICTIONARYIDA;

static PFNWP pfnwpOldFrameProc;        // address of old frame proc
MRESULT EXPENTRY DicFrameSubProc ( HWND, USHORT, WPARAM, LPARAM );

MRESULT APIENTRY DICTIONARYHANDLERWP( HWND, WINMSG, WPARAM, LPARAM);
MRESULT APIENTRY DICTIONARYWP( HWND, WINMSG, WPARAM, LPARAM);

// module EQFDEX.C
VOID   DictionaryExport( HWND, PSZ, PVOID );

// module EQFDORG.C
VOID   DictionaryOrganize( HWND, PSZ, PVOID );

// module EQFDIC01.C
VOID   DictionaryProp( HWND, PSZ );
VOID   DictionaryOpen( HWND, PSZ, PDICTIONARYIDA );
MRESULT DictPropControl( HWND, SHORT, SHORT );

// module EQFDIC03.C
BOOL   DictionaryNew( HWND, PDICTPARMS );

// module EQFRDICS.c
VOID   DictionaryRemote( HWND );

// module EQFDIC00.C
BOOL   DicMakeDictListItem ( HWND, PSZ, HPROP, PSZ );
VOID DictListItemGreyedOutTest ( HWND, BOOL, PSZ );

// index to elements of dictionary list items
typedef enum
{
  DIC_OBJECT_IND,
  DIC_NAME_IND,
  DIC_DESCR_IND,
  DIC_DRIVE_IND,
  DIC_SERVER_IND,
  DIC_USERID_IND,
  DIC_SOURCELANG_IND
} DIC_ITEMINDX;

#define MAX_NUM_DICTS      99          // max. number of dict concurrently open

#define MAXI_SIZE    0xFFFF // 32000               //64000  0x8000  maximum size
#define MAX_USER     20               // max number of user-defined entry fields

#define TIMER               1          // timer id
#define TIMEOUT           500          // timeout time in miliseconds

#define DICT_SECTION        1          // dict and /dict tags
#define MAPTABLE_SECTION    2          // maptable section
#define HEADER_SECTION      4          // header section
#define ENTRY_SECTION       8          // dict entry section

#define NO_DISPLAY 0                   // not displayed at all
#define FIRST_DISPLAY 1                // displayed on first panel
#define SECOND_DISPLAY 2               // displayed on second panel
#define THIRD_DISPLAY 3                // displayed on third panel

#define RW_STATUS  1                   // read/write status
#define RO_STATUS  2                   // read only status
#define NA_STATUS  3                   // no access

#define AUTLOOKUP  "01"                // automatic lookup values
#define DISPLEVEL  "0123"              // display level values
#define ENTRYTYPE  "12"                // entry type values
#define STATUS     "123"               // status values
#define VITAL      "01"                // vital values

#define AUTLOOKUPW  L"01"              // automatic lookup values
#define DISPLEVELW  L"0123"            // display level values
#define ENTRYTYPEW  L"12"              // entry type values
#define STATUSW     L"123"             // status values
#define VITALW      L"01"              // vital values

#define  SLIDER_INCR             101  // slider increment
#define  ASCII_FLAG              1    // Ascii flag converted to char.
#define  DRIVE_NAME              12   // Special drive name
#define  EQF_IDENTIFICATION    "EQF"  // EQF identification
#define  YES_STR               "YES"  // value string in dict header
#define  YES_STRW              L"YES"
#define  MAX_STACK               35   // deepness of stack
#define  STRINGLEN               40   // length of static text in std. window
#define  TXTLEN                 180   // length of merge text in merge dialog
#define  BUFFERLEN              256   // max length of work buffer and head term
#define  EMPTY_STRING           ""    // empty string
#define  SYSFIELD                -1   // system dict entry field value
#define  REPLACE                 2
#define  COPY                    3
#define  DIMP                   "Dictionary import"
#define IMPORTPATH              "\\IMPORT"

// dictionary export and import formats
#define DICT_FORMAT_SGML_ANSI     1
#define DICT_FORMAT_SGML_ASCII    2
#define DICT_FORMAT_SGML_UNICODE  3
#define DICT_FORMAT_XML_UTF8      4


// filter for dictionary formats (same sequence as memory export and import formats required)
#define DICT_FORMAT_FILTERS "SGML ANSI\0*.*\0SGML ASCII\0*.*\0SGML UTF-16\0*.*\0XML UTF-8\0*.*\0\0\0"


typedef struct _STACK
{
    SHORT    sCurrent;                 // current entry
    SHORT    sStack[ MAX_STACK ];      // deepness of stack
} STACK , *PSTACK;

typedef struct _MAPENTRY
{
    SHORT    sTokenId;          // token id
    SHORT    sId;               // indicator for user-created or system field
    USHORT   usDisplay;
    USHORT   usEntryType;
    USHORT   usStatus;
    BOOL     fVital;
    BOOL     fAutLookup;
    USHORT   usLevel;
    PSZ_W    pData;             // pointer to data
} MAPENTRY , *PMAPENTRY;

typedef enum _DIMPTASK
{
   EQFDIMP_READ,                    // read entry
   EQFDIMP_WRITE,                   // write entry
   EQFDIMP_PROCESS,                 // process entry
   EQFDIMP_CLOSE,                   // close processing
   EQFIMP_MERGE,
   EQFIMP_CLOSE,
   EQFIMP_GETWORD
} DIMPTASK;

typedef struct _MEM
{
   USHORT  usAvail;                 // available memory in # of w's
   USHORT  usUsed;                  // used memoryin # of w's
   CHAR_W  chBuffer[ MAXI_SIZE ];    // buffer area
} MEM, * PMEM;

// structure containing information on selected dictionaries for later
// processing
typedef struct _SELDICTINFO
{
  SHORT       sItem;                   // item index of dictionary in listbox
  OBJNAME     szObjName;               // object name of selected dictionary
  CHAR        szName[MAX_LONGFILESPEC];// name of dictionary
} SELDICTINFO, *PSELDICTINFO;

#if defined(DICTPROP)
PROPDICTIONARY SgmlProp = { {"","", 0, 0},  //PropHead
                           "","", {0},     //Descr, slang, tlang(s)
                           FALSE,          //copyright
                           "", "",         //DictPath, IndexPath
                            0, 1,
                           { CLR_BLACK, CLR_WHITE, "System Proportional"},
                           { CLR_RED, CLR_WHITE, "System Proportional" },
                           {0}, FALSE, 0l,  //profentries, protected, password
                           "" };
#else
   extern PROPDICTIONARY SgmlProp;
#endif

typedef struct _DIMPIDA
{
 HWND         hwnd;                       // process window handle
 CHAR         szDictName[MAX_LONGPATH];   // dictionary name (PropName)
 CHAR         szShortName[MAX_FNAME];     // dictionary short name
 CHAR         szLongName[MAX_LONGPATH];   // dictionary long name
 IDA_HEAD     IdaHead;                    // common header of instance area
 CHAR         szFolderDicts[NUM_OF_FOLDER_DICS][MAX_LONGPATH]; //folder dicts
 CHAR         szInvokingHandler[MAX_EQF_PATH]; //import folder dicts handler name
 USHORT       usDictNum;                  // number of folder import dicts
 CONTROLSIDA  Controls;                   // ptr to controls ida
 CHAR         szNewDictName[MAX_LONGPATH];// newly specified dictionary name
 CHAR         szPropPath[MAX_LONGPATH];   // name of profile of imported file
 BOOL         fAscii;                     // TRUE = ASCII else EQF internal format
 BOOL         fSavedFormat;               // TRUE = ASCII else EQF internal format
 HWND         DictLBhwnd;                 // dict list hwnd
 HWND         hwndPathEf;                 // handle to Path entry field
 HWND         hwndDirLb;                  // handle to directories listbox
 HWND         hwndFilesLb;                // handle to Files listbox
 HWND         hwndNewNameLB;              // handle to new dict name listbox
 HWND         hwndSourceLangLB;           // handle of invisible source lang lb
 FILEFINDBUF  FolderResultBuf;            // DOS file find struct
// FILEFINDBUF  TrojaResultBuf;             // DOS file find struct
 LONGFILEFIND TrojaResultBufLong;
 CHAR         szDrive[MAX_DRIVE];         // drive letter with :
 CHAR         chSavedDrive;               // saved drive letter
 CHAR         chDrive;                    // current target drive
 CHAR         szPath[MAX_LONGPATH];       // directory incl. trailing
 CHAR         szSavedPath[MAX_LONGPATH];  // saved path
 CHAR         szName[MAX_LONGPATH];       // filename
 CHAR         szExt[MAX_FEXT];            // file extension with leading dot
 CHAR         szPatternName[MAX_FILESPEC]; // active pattern name
 CHAR         szPatternExt[MAX_FEXT];     // active pattern extension
 CHAR         szFindPath[MAX_LONGPATH];   // path for FillFileDirList utility
 CHAR         szPathContent[MAX_LONGPATH];// content of entry Path entryfield
 CHAR         szString[MAX_LONGPATH];     // string buffer
 CHAR         szSymbol[MAX_LONGPATH];     // string buffer
 CHAR         szDummy[MAX_LONGPATH];      // string buffer
 CHAR         szDriveList[27];            // Drive list
 CHAR         szTitle[60];                // title bar text of analysis window
 HPROP        hDictListProp;              // handle of dict list properties
 PPROPSYSTEM  pSysProp;                   // ptr to system properties
 HFILE        hFile;                      // Handle of file to be loaded
 HFILE        hOut;                       // Handle of output file for testing
 ULONG        ulSize;                     // size of file to be loaded
 ULONG        ulFilePosition;             // position in input file
 DIMPTASK     DimpTask;                   // currently active task
 USHORT       usKilledTask;               // interrupted task
 BOOL         fKill;                      // general kill flag
 BOOL         fNextDict;                  // next dict during folder import
 BOOL         fAll;                       // all data processed
 BOOL         fFree;                      // free symbol or not
 BOOL         fShutdown;                  // shutdown message issued
 BOOL         fValid;                     // valid tag or not
 BOOL         fImpMerge;                  // folder import merge or not
 BOOL         fStop;                      // stop processing and release memory alloc.
 BOOL         fMerge;                     // merge two dicts
 BOOL         fRemote;                    // flag if dict is remote
 BOOL         fNotOk;                     // something wrong so cancel
 BOOL         fNotCompleted;              // process interrupted
 BOOL         fNoError;                   // display superfluous maptable data once
 BOOL         fNoImp;                     // display that entry exists only once
 BOOL         fIsSgml;                    // Sgml dict file or not
 USHORT       usDictLeft;                 // number of import folder dicts left
 PBYTE        pInputFileBlock;            // pointer to mem block in input file
 ULONG        ulUsed;                     // used size of memory
 USHORT       usFlags;                    // dict merge entries flags
 USHORT       usMaxTokens;                // max. number of tokens
 PBYTE        pEntry;                     // poibter to dictionary entry
 PSZ          pTokenBlock;                // pointer to mem block in token list
 PSZ          pszMsgError[5];             // QDAM error array
 STACK        Stack;                      // stack
 PSZ_W        pRest;                      // ptr to not processed rest
 CHAR         szImportDictTxt[MAX_LONGPATH];  // buffer for importing text string
 CHAR         szInDictTxt[1024];          // buffer for entry group box
 CHAR         szEntryTxt[STRINGLEN];      // buffer for entry
 HWND         hwndImportDictTxt;          // text with importing string
 HWND         hwndInDictTxt;              // text with new imported dict
 HWND         hwndEntryTxt;               // text with entry
 HWND         hwndSlider;                 // Slider Window
 LONG         lPelsPerLine;               // pels per line (used for form)
 LONG         lSliderPos;                 // slider position
 LONG         lFileSlider;                // new slider pos after new dimpread
 USHORT       usMapNext;                  // current pos in maptable
 USHORT       usMapAvail;                 // how much space in maptable available
 HDCB         hDict1;                     // handles for asd plus index file(s)
 HDCB         hDict2;                     // handles for asd plus index file(s)
 HUCB         hUser;                      // ASD user handle
 CHAR_W       ucTermBuf[MAX_TERM_LEN];    // term buffer
 CHAR_W       ucErrorTerm[MAX_TERM_LEN];   // for error messages
 PVOID        pTagTable;                  // pointer to tagtable
 SHORT        sOrgToken[ LAST_TAG ];      // max. number of tokens allowed
 SHORT        sSkipToken;                 // end token
 SHORT        sSkipStack;                 // stack position
 PMEM         pMem;                       // pointer to buffer area
 PPROPDICTIONARY pAsdDictProp;            // pointer to asd dict properties
 PPROPDICTIONARY pSgmlProp;               // pointer to sgml dict properties
 PMAPENTRY    pMapEntry;                  // structure with tag information
 PTOKENENTRY  pToken;                     // current active token
 PVOID        phTree;                     // tree of new entry
 USHORT       ausNoOfFields[QLDB_MAX_LEVELS]; // number of fields per level
 USHORT       usAddNodesOffset;           // node offset
 ULONG        ulMaxEntries;               // total number of entries in asd
 ULONG        ulCurrentEntry;             // current number of entries in dict
 ULONG        ulTermNum;                  // term number
 ULONG        ulDataLen;                  // dict data length
 CHAR_W       chWorkBuffer[BUFFERLEN];    // temporary working area
 USHORT       usActiveLevel;              // current level of entry
 USHORT       usSubTree;                  // current subtree level
 USHORT       usHomLevel;                 // start of hom level
 USHORT       usSenseLevel;               // start of sense level
 USHORT       usTargetLevel;              // start of target level
 USHORT       usLevel;                    // level of entry field
 SHORT        sId;                        // user field number
 SHORT        sSection;                   // dict section
 BOOL         fCopyRight;                 // dict copyrighted
 CHAR         szSourceLang[MAX_LANG_LENGTH]; // dictionary source language
 CHAR_W       szDateW[20];                // buffer for current date string
 HWND         HWNDImpCmd;                 // dict imp or cmd line hwnd
 USHORT       usDDERc;                    // error return ( normal/dde )
 PDICIMPEXP   pDicImpExp;                 //command line dict import structure
 USHORT       usImpMode;                  // mode for import
 BOOL         fFirstRead;                 // TRUE = this is the first read access
 PBYTE        pConvBuffer;                // for unicode/AScii conv. in DimpREad
 ULONG        ulUsedInConv;               // bytes in pConvBuffer
 PSZ          pszList;                    // points to list of import files or NULL
 PSZ          pszActFile;                 // points to active file in list of import files or NULL
 CHAR         szLongDesc[MAX_LONG_DESCRIPTION];
 ULONG        ulOEMCP;
 ULONG        ulAnsiCP;
 HWND         hwndToCombo;                  // handle of "to dictionary" combobox in standard open dialog
 CHAR szImportDir[MAX_EQF_PATH];         // import directory name
 ULONG ulImportDirLength;                // length of import directory name
 CHAR_W       szErrorMsgParm[MAX_PATH];  // buffer for the parameter of error messages
 BOOL         fIgnoreFieldNotInDictError;// TRUE = ignore fields which are not defined in the dictionary

} DIMPIDA, *PDIMPIDA;

typedef struct _DORGIDA
{
 IDA_HEAD     IdaHead;                    // common header of instance area
 CHAR         szDictName[MAX_FNAME];      // dictionary name
 CHAR         szString[MAX_LONGPATH];     // string buffer
 CHAR         szFilePath[MAX_LONGPATH];   // string buffer
 CHAR         szPath[MAX_LONGPATH];       // string buffer
 CHAR         szOriginalProps[MAX_EQF_PATH];  // Full prop file name
 CHAR         szOriginalAsd[MAX_EQF_PATH];    // Full asd file name
 CHAR         szOriginalAsi[MAX_EQF_PATH];    // Full asi file name
 CHAR         szOrganizedProps[MAX_EQF_PATH]; // organized prop file name
 CHAR         szOrganizedAsd[MAX_EQF_PATH];   // organized asd file name
 CHAR         szOrganizedAsi[MAX_EQF_PATH];   // organized asi file name
 CHAR         szCopyAsd[MAX_EQF_PATH];    // copy asd file name
 CHAR         szCopyAsi[MAX_EQF_PATH];    // copy asi file name
 CHAR         szTitle[60];                // title bar text of standard window
 CHAR         szEntryGB[STRINGLEN];       // buffer for group box title
 CHAR         szEntryTxt[STRINGLEN];      // buffer for entry text
 HWND         DictLBhwnd;                 // dict list hwnd
 HWND         hwndEntryGB;                // handle of entry group box
 HWND         hwndEntryTxt;               // handle of entry text
 HWND         hwndSlider;                 // handle of Slider Window
 LONG         lPelsPerLine;               // pels per line (used for form)
 LONG         lSliderPos;                 // slider position
 HFILE        hFile;                      // Handle of file to be loaded
 FILEFINDBUF  ResultBuf;                  // DOS file find struct
 HDCB         hDict1;                     // handles for asd plus index file(s)
 HDCB         hDict2;                     // handles for asd plus index file(s)
 HUCB         hUser;                      // ASD user handle
 BOOL         fFree;                      // dict not involved in another process
 BOOL         fKill;                      // general kill flag
 BOOL         fNoOrgError;                // org success indication
 PSZ          pszMsgError[5];             // QDAM error array
 CHAR_W       ucTermBuf[MAX_TERM_LEN];    // term buffer
 CHAR         ucAnsiTerm[MAX_TERM_LEN];   // ANSI converted term
 ULONG        ulDataLen;                  // length of term data record
 PSZ_W        pucDictData;                // term's associated data
 ULONG        ulMaxEntries;               // total number of entries in asd
 ULONG        ulCurrentEntry;             // current number of entry
 USHORT       ausNoOfFields[QLDB_MAX_LEVELS];  // number of fields per level
 BOOL         fRemote;                    // flag for remote dict or not
 BOOL         fNoMsg;                     // shows if terminated msg should appear
 CHAR         szServer[MAX_SERVER_NAME];  // server name
 CHAR         szUserid[MAX_USERID];       // user id
 CHAR         szSourceLang[MAX_LANG_LENGTH]; // dict source lang
 CHAR         chRemPrimDrive;             // primary remote drive
 CHAR         szLongName[MAX_LONGFILESPEC]; // dictionary long name
 CHAR         ucASCIITermBuf[MAX_TERM_LEN]; // term buffer if old dict with ASCII terms
 ULONG        ulASCIIDataLen;               // number of chars in ASCIITermBuf
 PSZ          pucASCIIDictData;             // term's ass. data if in ASCII
 USHORT       usVersion;                    // 2 is Version2 Unicode, 0 older ones
 PSELDICTINFO pDictList;                    // list of dictionaries being organized
 PSELDICTINFO pActiveDict;                  // points to active entry in list
 ULONG        ulTotalTerms;                 // number of organized terms
 ULONG        ulOemCP;
 ULONG        ulSkipped;                    // number of corrupted entries being skipped
 PSZ_W        apszField[MAX_PROF_ENTRIES];
}DORGIDA, * PDORGIDA;

//Function Prototypes

HWND hwndCreateSlider ( HWND, USHORT );
VOID DicBatchImp( HWND, PVOID );
VOID DicBatchExp( HWND, PVOID );

#define  MODELDICT             "-Master-"

typedef struct _DICCREATEIDA
{
   BOOL     fNewDict;                  // TRUE = create dictionary dialog
                                       // FALSE = dictionary property dialog
   CHAR     szName[MAX_FNAME];         // dictionary name
   CHAR     szDicName[MAX_EQF_PATH];   // ASD dict path
   CHAR     szPropName[MAX_EQF_PATH];  // property name (without property dir)
   CHAR     szDrive[MAX_DRIVE];        // asd dict drive letter
   CHAR     chDrive;                   // current target drive
   CHAR     chRemPrimDrive;            // primary drive on LAN
   PROPDICTIONARY Prop;                // copy of dictionary properties
   PROPDICTIONARY PropCopy;            // copy of dictionary properties
   PPROPSYSTEM    pSysProp;            // ptr to system properties
   PDICTPARMS     pDictParms;          // dial parameters
   HPROP          hDicProp;            // handle of dictionary properties
   CHAR           szBuffer1[MAX_LONGPATH]; // buffer for object names
   CHAR           szBuffer2[MAX_LONGPATH]; // buffer for object names
   HWND           hwndDictLB;          // dict listbox handle
   CHAR           szLANDriveList[MAX_DRIVELIST];  // array of LAN drives
   CHAR           szServer[MAX_SERVER_NAME]; //server name
   CHAR           szDriveList[MAX_DRIVELIST];//array of drives (local or remote)
   PPROFENTRY     pEntry;              // pointer to current profile entry
   PROFENTRY      NewEntry;            // pointer to added/changed entry
   BOOL           fChangeName;         // either add or change name dialog
   BOOL           fProtected;          // dict r/o or r/w
   HDCB           hDict;               // handles for asd plus index file(s)
   HUCB           hUser;               // ASD user handle

   PSZ            pszMsgError[5];      // QDAM error string array
   ULONG          ulPassword;          // password
   USHORT         usUserPriv;          // lan adm user id
   CHAR           szLanUserID[MAX_USERID];   // user id
   BOOL           fProtectionChanged;  // protection-of-dict-has-changed flag
   CHAR           szLongName[MAX_LONGFILESPEC]; // buffer for dict long name
   HWND           hwndPages[20];      // array with property pages
   HWND           hwndTabCtrl;        //tabs
   CHAR           szModelDictLongName[MAX_LONGPATH];       // model dict long name
   CHAR           szModelDictShortName[MAX_FILESPEC];      // model dict short name
} DICCREATEIDA, *PDICCREATEIDA;

INT_PTR CALLBACK DICCREATEDICTIONARYDLG ( HWND, WINMSG, WPARAM, LPARAM );
MRESULT DictCreateControl( HWND, SHORT, SHORT );
MRESULT DictCreateCommand( HWND, SHORT, SHORT );
MRESULT SetEntryStructControl( HWND, SHORT, SHORT );
MRESULT SetEntryStructCommand( HWND, SHORT, SHORT );

MRESULT APIENTRY DICTIONARYPROPERTYDLG ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK DICTIONARYPROPERTYDLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK DICTPROP_GENERAL_DLGPROC
(
 HWND hwnd,                       //handle of dialog window
 WINMSG msg,
 WPARAM mp1,
 LPARAM mp2
 );
 INT_PTR CALLBACK DICTPROP_OPTIONS_DLGPROC
 (
 HWND hwnd,                       // handle of dialog window
 WINMSG msg,
 WPARAM mp1,
 LPARAM mp2
 );


INT_PTR CALLBACK SETENTRYSTRUCTDLG ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK DICMODELDLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport)
INT_PTR CALLBACK PCH_DLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport)
INT_PTR CALLBACK DICT1PASSWORDDLG ( HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport)
INT_PTR CALLBACK DICT2PASSWORDDLG ( HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport)
VOID DicFillEntryLB( HWND, PDICCREATEIDA, USHORT );
static VOID DeleteDriveIcons( HWND, PSZ );

MRESULT PCH_InitDlg ( HWND, WPARAM, LPARAM );
MRESULT PCH_Command ( HWND, WPARAM, LPARAM, PDICCREATEIDA );


#define  MIN_DICT_SPACE    50000   //min. disk space needed for dict creation
#define  HPS_HEADLINE_cy   25
#define  DICT_PROPERTIES_NAME "EQFDICT.PRP" // name of dictionary list props
#define  EXT_OF_DICPROP ".PRO"     // extension of dictionary properties
#define  COMPLETED_TASK   USER_TASK + 1

//new dialog flag to show if entry structure given via sgml or not
#define PROPS             1
#define NEW               2
#define REM_DEL           3
#define LOCAL_DEL         4

#define  SGML_EXT                ".SGM" // default ext for sgml files
#define  TAGLENGTH               100    // max length of tag info in sgml file
#define  BUFFER_LENGTH           400    // max length of output buffer for write
#define  ID_TOKEN                "ID="  // id token attribute
#define  AUTH_TOKEN              "STATUS="       // status token attribute
#define  DISPLAY_TOKEN           "DISPLEVEL="    // display token attribute
#define  ENTRYTYPE_TOKEN         "ENTRYTYPE="    // entry type token attribute
#define  VITAL_TOKEN             "VITAL"         // entry obligatory
#define  AUTLOOKUP_TOKEN         "AUTLOOKUP"     // display in aut. lookup

#define  ID_TOKENW                L"ID="  // id token attribute
#define  AUTH_TOKENW              L"STATUS="       // status token attribute
#define  DISPLAY_TOKENW           L"DISPLEVEL="    // display token attribute
#define  ENTRYTYPE_TOKENW         L"ENTRYTYPE="    // entry type token attribute
#define  VITAL_TOKENW             L"VITAL"         // entry obligatory
#define  AUTLOOKUP_TOKENW         L"AUTLOOKUP"     // display in aut. lookup

#define  MAXNODES                500    // max. number of nodes in entry struct
#define  LINELIMIT               50     // max. number of chars in line
#define  INDENT                  2      // number of spaces to indent
#define  DEXP                    "Dictionary Export"
#define  XCOORDINATE_STATICTXT   5      // x-coordinate pos of export statics
#define  XCOORDINATE_ENTRY       8      // x-coordinate pos of entry in std win
#define  YCOORDINATE_EXPORTTXT   3
#define  YCOORDINATE_SLIDER      7
#define  YCOORDINATE_ENTRY       11
#define  YCOORDINATE_ENTRYGB     12
#define  WIDTH_STATICTXT         10
#define  WIDTH_ENTRY             15
#define  HEIGHT_SLIDER           5
#define  HEIGHT_ENTRYGB          4
#define  CODELENGTH              2

#define  ZERO_LEVEL               0    // top node level
#define  FIRST_LEVEL              1    // entry level
#define  SECOND_LEVEL             2    // hom level
#define  THIRD_LEVEL              3    // sense level
#define  FOURTH_LEVEL             4    // target level

#define  TOP_NODE                'a'  // top most node
#define  FIRST                   'b'  // entry level
#define  SECOND                  'c'  // hom level
#define  THIRD                   'd'  // sense level
#define  FOURTH                  'e'  // target level


// import/ export formats (sequence must be same as in filter list for saveas dialog!)
typedef struct _CODETAGS
{
   SHORT   sFieldNumber;      //number of corresponding dictionary field or
                              // -1 for level entries
   PSZ     pStartTag;         //start tag
   PSZ     pEndTag;           //end tag
   PSZ     pSysName;          //system field name
   PSZ     pUserName;         //user selected field name
   SHORT   sLevel;            //entry level (entry,hom,sense,target)
   USHORT  usDisplay;         //display level
   USHORT  usEntryType;       //sle or mle
   USHORT  usStatus;          //authorization
   BOOL    fVital;            //obligatory field or not
   BOOL    fAutLookup;        //use field in automatic lookup flag
} CODETAGS, * PCODETAGS;

typedef struct _CODETAGSW
{
   SHORT   sFieldNumber;      //number of corresponding dictionary field or
                              // -1 for level entries
   PSZ_W     pStartTag;         //start tag
   PSZ_W     pEndTag;           //end tag
   PSZ       pSysName;          //system field name
   PSZ       pUserName;         //user selected field name
   SHORT   sLevel;            //entry level (entry,hom,sense,target)
   USHORT  usDisplay;         //display level
   USHORT  usEntryType;       //sle or mle
   USHORT  usStatus;          //authorization
   BOOL    fVital;            //obligatory field or not
   BOOL    fAutLookup;        //use field in automatic lookup flag
} CODETAGSW, * PCODETAGSW;

typedef enum _DEXTASK
{
   PREPARE,                         // prepare memory areas
   INITIALIZE,                      // set up conversion table
   MAPTABLE,                        // write maptable
   GET_ENTRY,                       // get next ASD entry
   EXPORT_ENTRY,                    // pack data in sgml syntax
   CLOSE_PROCESS                    // write end dict processing
} DEXTASK;

typedef struct _DEXIDA
{
   IDA_HEAD     IdaHead;                      // common header of instance area
   HWND         hwnd;                         // handle of list window
   CHAR         szDictName[MAX_LONGPATH];     // dictionary name
   CHAR         szShortName[MAX_FNAME];      // dictionary short name
   CHAR         szSgmlDictName[MAX_LONGPATH]; // newly specified dictionary name
   CHAR         szFilePath[MAX_LONGPATH];     // Full file name + path
   CONTROLSIDA  ControlsIda;
   BOOL         fAscii;                  // TRUE = external format
   HWND         hwndDirLb;               // handle to directories listbox
   HWND         hwndNewNameSle;          // handle to new dict name sle
   BOOL         fKill;                   // stop processing flag
   BOOL         fLast;
   BOOL         fFree;
   BOOL         fHalfCompleted;
   BOOL         fShutdown;
   CHAR         szDrive[MAX_DRIVE];         // drive letter with :
   CHAR         chSavedDrive;               // saved drive letter
   CHAR         szSavedPath[MAX_LONGPATH];  // saved path
   CHAR         szPath[MAX_LONGPATH];       // directory incl. trailing
   CHAR         szName[MAX_LONGFILESPEC];   // filename
   CHAR         szExt[MAX_FEXT];            // file extension with leading dot
   CHAR         szPatternName[MAX_FNAME];   // active pattern name
   CHAR         szPatternExt[MAX_FEXT];     // active pattern extension
   CHAR         szFindPath[MAX_LONGPATH];   // path for FillFileDirList utility
   CHAR         szPathContent[MAX_LONGPATH];  // content of entry Path entryfield
   CHAR         szString[MAX_LONGPATH];       // string buffer
   CHAR         szDummy[MAX_LONGPATH];        // string buffer
   CHAR         szDriveList[MAX_DRIVELIST];   // Drive list
   FILEFINDBUF  ResultBuf;               // DOS file find struct
   DEXTASK      DexTask;                 // currently active task
   HFILE        hFile;                   // Handle of input dictionary file
   HFILE        hOut;                    // Handle of SGML dictionary file
   HPROP        hDictListProp;           // handle of dict list properties
   HDCB         hDict;                   // handles for asd plus index file(s)
   HUCB         hUser;                   // ASD user handle
   CHAR         szTitle[512];            // title bar text of window
   ULONG        ulSize;                  // size of file to be loaded
   PBYTE          pBuffer;                 // pointer to memory area
   PSZ_W          pTagBuffer;              // pointer to memory area for tag info
   PSZ_W          pTag;                    // pointer to current pos in tag mem area
   PSZ_W          pWriteBuffer;            // pointer to current pos in buffer for write
   PBYTE          pCurrent;                // current position in buffer mem area
   PBYTE          pBufferEnd;              // end position in buffer mem area
   PSZ          pszMsgError[5];          // qdam error message array
   BOOL         fSpecialCase;            // no further errors dispalyed
   BOOL         fComplete;               // something went very wrong
   CHAR         szExportDictTxt[STRINGLEN]; // buffer for text string
   CHAR         szEntryTxt[STRINGLEN];   // buffer for text string
   CHAR         szEntryStaticTxt[STRINGLEN];     // buffer for text string
   HWND         DictLBhwnd;              // dict list box handle
   HWND         hwndExportDictTxt;       // text with dict for
   HWND         hwndEntryTxt;            // text with new imported dict
   HWND         hwndEntryStaticTxt;      // text with new imported dict
   HWND         hwndSlider;              // Slider Window
   LONG         lSliderPos;              // indicator of current slider position
   PSZ_W        pucData;                 // ptr with dict data
   PSZ_W        pucNewData;              // ptr with dict data and ctln's
   PUCHAR       pucSigRec;
   CHAR_W       ucTermBuf[MAX_TERM_LEN]; // term buffer
   //UCHAR        ucAnsiTerm[MAX_TERM_LEN];// ANSI converted term
   ULONG        ulMaxDictEntries;        // total number of dict entries
   ULONG        ulTermNum;               // term number in asd dict
   ULONG        ulDataLen;               // length of term data record
   SHORT        sCurrentLevel;           // current level in entry tree struct
   PTOKENENTRY  pToken;                  // current active token
   PPROPDICTIONARY pProp;                // pointer to dict properties
   USHORT       usMaxNodes;              // max number of nodes in entry struct
   USHORT       usKilledTask;            // which task discontinued?
   CODETAGSW     CodeTags[MAXNODES];      // structure with tag and field names
   BOOL         fCopyRight;
   BOOL         fReadOnly;
   CHAR         szSourceLang[MAX_LANG_LENGTH];             // dictionary source language
   // array containing the number of dictionary fields per level
   USHORT       ausNoOfFields[QLDB_MAX_LEVELS];
   // array containing the number of the first field of a level
   USHORT       ausFirstFieldOfLevel[QLDB_MAX_LEVELS];
   // Buffer for the data pointers of a node
   PSZ_W        apszField[MAX_PROF_ENTRIES];
   // Field-number-to-code-tag array. Index is field number, data is index
   // into code tag table
   USHORT       ausFieldToCode[MAX_PROF_ENTRIES];
   // level-number-to-code-tag array. Index is level number, data is index
   // into code tag table
   USHORT       ausLevelToCode[QLDB_MAX_LEVELS];
   HWND         hwndError;             // hwnd for error messages
   USHORT       usDDERc;               // error return ( normal/dde )
   PDDEDICEXP   pDicExp;               // command line dict export structure
   USHORT       usExpFormat;           // SGML format to use for export
   BOOL         fAnsiConv;
   CHAR         szLongDesc[MAX_LONG_DESCRIPTION];
   PSZ          pASCIIBuffer;
 PSELDICTINFO pDictList;                    // list of dictionaries being exported
 PSELDICTINFO pActiveDict;                  // points to active entry in list
   LONG         lpucNewDataSize;            // size of buffer pointed to by pucNewData
   ULONG        ulOemCP;
   ULONG        ulAnsiCP;
   BOOL         fSkipped;             // TRUE = corrupted entries have been skipped during export
   CHAR         szToExternalFileText[256]; // buffer for process window text
   BOOL         fYesToAll;                 // yes to all flag
}DEXIDA, * PDEXIDA;

// ************* Function Prototypes ************************************

VOID DICTIONARYEXPORT( HWND, PSZ );

BOOL DicGetCheckPassword               // compute a password value or check
                                       // the given password against password
                                       // value
                                       // return value for fCheckPassword = TRUE:
                                       //   TRUE  = password matchs password value
                                       //   FALSE = password does not match
                                       // return value for fCheckPassword = FALSE:
                                       //   TRUE  = password has correct syntax
                                       //   FALSE = password is invalid

(
  PSZ              pszPassword,        // password string
  PULONG           pulPassword,        // computed password value
  BOOL             fCheckPassword      // flag: TRUE  = check given password
                                       //       FALSE = compute password value
);

BOOL DicLoadFieldNames( PPROPDICTIONARY );
BOOL DorgInit( HWND, PDORGIDA );

