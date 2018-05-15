//+----------------------------------------------------------------------------+
//|EQFLISTI.H                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author: Gerhard Queck (QSoft)                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Private include file for list handler.                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
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
// $Revision: 1.3 $ ----------- 25 Sep 2001
// -- RJ: Move LstBufWrite to EQFUtfil.c
//
//
// $Revision: 1.2 $ ----------- 18 Sep 2001
// -- RJ: unicode enabling
//
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.4 $ ----------- 14 Feb 2001
// - added format parameter to LstReadSgmlList and LstWriteSgmlList
//
//
// $Revision: 1.3 $ ----------- 22 Sep 2000
// added new bitmap
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFLISTI.HV_   1.5   16 Feb 1998 07:37:10   BUILD  $
 *
 * $Log:   J:\DATA\EQFLISTI.HV_  $
 *
 *    Rev 1.5   16 Feb 1998 07:37:10   BUILD
 * - Win32: added bitmap handles for gray-backgrounded bitmaps
 *
 *    Rev 1.4   19 Jan 1998 11:11:52   BUILD
 * - corrected bit fields for OS/2 compile
 *
 *    Rev 1.3   14 Jan 1998 15:56:30   BUILD
 * - use type short for bit fields
 *
 *    Rev 1.2   26 Feb 1997 20:12:56   BUILD
 * -- Compiler defines for _POE22, _TKT21, and NEWTCSTUFF eliminated
 *
 *    Rev 1.1   10 Jan 1996 19:32:02   BUILD
 * - added defines for addenda term list
 *
 *    Rev 1.0   09 Jan 1996 09:46:34   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

/**********************************************************************/
/* Internal Error Identifiers of the List Handler                     */
/**********************************************************************/
typedef enum _LISTHANDLER_INT_ERRORS
{
  START_ID = ID_LIST_WINDOW,

  // LstWorkProcessMarks: process state is in an undefined state
  LSTWORKPROCESSMARKS_PROCSTATE


} LISTHANDLER_INT_ERRORS;

/**********************************************************************/
/* Several constants used within the list handler                     */
/**********************************************************************/
#define LISTNAMESIZE       80          // max. length of the name of a list type
#define LISTPROCPATTERN    "EQFLP*.DLL"// pattern for user list processors
#define LISTPROCENTRYPOINT "LISTPROC"  // entry point for list processors


#define  EXT_OF_DICPROP ".PRO"     // extension of dictionary properties

#define USER_FORMAT_EXIT   "EQFLSTUS.DLL" // name of export/import user exit
#define USER_EXPORTINIT    "EXPINIT"      // name of export init entry point
#define USER_EXPORTTERM    "EXPTERM"      // name of export a term entry point
#define USER_EXPORTEND     "EXPEND"       // name of export end entry point
#define USER_IMPORTINIT    "IMPINIT"      // name of import init entry point
#define USER_IMPORTTERM    "IMPTERM"      // name of import a term entry point
#define USER_IMPORTEND     "IMPEND"       // name of import end entry point

#define IO_BUFFER_SIZE     16000       // size of input buffers
#define TOK_BUFFER_SIZE    16000       // size of token buffers
#define CONTEXT_TABLE_SIZE 16000       // size of a single context table
#define CONTEXT_LIST_SIZE     20       // number of entries in a context list
#define TERM_TABLE_SIZE    16000       // size of a single term table
#define PTR_LIST_ENTRIES      20       // one-time allocation unit for pointer
                                       // lists in number of entries
#define MAX_PRINT_LINE       512       // max length of a single print line
#define TERM_BUFFER_SIZE    4000       // incremental size for term buffers
#define DELETE_MARK        'D'         // flag in SGML file for delete mark
#define EXCL_MARK          'X'         // flag in SGML file for exclusion mark
#define DICT_MARK          'A'         // flag in SGML file for dictionary mark
#define SELECT_MARK        'S'         // flag in SGML file for selected terms
#define TOP_MARK           'T'         // flag in SGML file for top of list term
#define CONTEXT_MARK       'C'         // flag in SGML file for add context
#define END_TAG_CHAR       '>'         // end delimiter for tags
#define ATTR_VALUE_DELIMITER '='       // value delimiter in SGML attributes
#define MAX_EXCLLIST_SIZE  0xFFF0      // max size for a noise/exclusion list
#define LIST_TEMP_NAME     "TEMPLIST.$$$"        // temporary name for list files

#define EXPORT_NTL_EXT    ".NTX"       // extension of exported new term list
#define EXPORT_FTL_EXT    ".FTX"       // extension of exported found term list
#define EXPORT_EXCL_EXT   ".EXX"       // extension of exported exclusion list
#define EXPORT_NOISE_EXT  ".NOX"       // extension of exported noise list
#define EXT_OF_EXTERNAL_LIST ".LST"    // extension of external lists


/**********************************************************************/
/* Format of strings stored in the column listbox of the list handler */
/* instance window                                                    */
/**********************************************************************/
#define  LST_ITEM_FORMAT       "%s\x15%s\x15%s%\x15%ld\x15%ld\x15%ld\x15"
typedef enum               //    ^     ^     ^       ^      ^      ^
{                          //    |     |     |       |      |      |
  LST_OBJECT_IND,          //----+     |     |       |      |      |
  LST_NAME_IND,            //----------+     |       |      |      |
  LST_DRIVE_IND,           //----------------+       |      |      |
  LST_SIZE_IND,            //------------------------+      |      |
  LST_UPD_DATE_IND,        //-------------------------------+      |
  LST_UPD_DATETIME_IND     //--------------------------------------+
} LST_ITEMINDX;

/**********************************************************************/
/* Types for built-in lists                                           */
/**********************************************************************/
typedef enum _LISTTYPES
{
  UNKNOWN_TYPE,                        // list type is unknown
  NTL_TYPE,                            // new terms list
  FTL_TYPE,                            // found terms list
  EXCL_TYPE,                           // exclusion list
  NOISE_TYPE,                          // noise list
  ABR_TYPE,                            // abbreviation list
  ADD_TYPE                             // addenda term list
} LISTTYPES;

/**********************************************************************/
/* System names of built-in list types                                */
/**********************************************************************/
#define NTL_LIST_NAME          "NTLLIST"
#define FTL_LIST_NAME          "FTLLIST"
#define EXCLUSION_LIST_NAME    "EXCLLIST"
#define NOISE_LIST_NAME        "NOISELIST"
#define ABR_LIST_NAME          "ABBRLIST"
#define ADD_LIST_NAME          "ADDLIST"

/**********************************************************************/
/* Type of List Processor Entry Points                                */
/**********************************************************************/
typedef USHORT (EXPENTRY *PLISTPROC)( PVOID *, USHORT, HWND, PSZ );

/**********************************************************************/
/* Structure of List Processor Table Elements                         */
/**********************************************************************/
typedef struct _LISTPROCTABLEENTRY
{
  PLISTPROC   pListProc;               // processor for this type of list
  PVOID       pListData;               // ptr to list processor private data
  CHAR        szListType[LISTNAMESIZE];// system name of this type of list
  CHAR        szListName[LISTNAMESIZE];// user name of this type of list
  HMODULE     hModule;                 // handle of list processor module (DLL)
  USHORT   ausCommands[LST_LAST_CMD+2];// buffer for list processor commands
} LISTPROCTABLEENTRY, *PLISTPROCTABLEENTRY;

/**********************************************************************/
/* List handler instance data area                                    */
/**********************************************************************/
typedef struct _LSTIDA
{
  IDA_HEAD     IdaHead;                // common header of instance area
  OBJNAME      szPropName;             // object name for properties
  HWND         hLBox;                  // list box wnd handle
  BOOL         fFirstActivation;       // TRUE = is activated the first time
  PSZ          *apszListTypes;         // array with list type names
  LISTTYPES    Type;                   // type of list
  CHAR         szBuffer[4096];         // multi-purpose buffer
} LSTIDA, *PLSTIDA;

/**********************************************************************/
/* Definition for pointer lists                                       */
/**********************************************************************/
typedef struct _PTRLIST
{
  USHORT usMaxEntries;                 // max. number of entries in list
  USHORT usEntries;                    // number of entries in list
  PVOID  Ptr[1];                       // pointer array ( variable length )
} PTRLIST, *PPTRLIST;


/**********************************************************************/
/* Entry of the context table                                         */
/* The array of context pointers is stored in the area behind this    */
/* structure.                                                         */
/**********************************************************************/
typedef struct _CONTEXTTABLE
{
  USHORT usSize;                       // size of context table
  USHORT usFirst;                      // first context number in table
  USHORT usLast;                       // last context number in table
  struct _CONTEXTTABLE *pNextTable;    // ptr to next context table
} CONTEXTTABLE, *PCONTEXTTABLE;

/**********************************************************************/
/* Context ID list for a term                                         */
/**********************************************************************/
typedef struct _CONTEXTLIST
{
  USHORT usSize;                       // size of context list
  USHORT usUsed;                       // used size of context list
  USHORT ausContextID[1];              // begin of context ID array
} CONTEXTLIST, *PCONTEXTLIST;

/**********************************************************************/
/* Flag values for fMark in Flags bit field in structure below and    */
/* as current view value in WWL dialog                                */
/**********************************************************************/
#define NOTMARKED_FLAG    0
#define DELMARK_FLAG      1
#define EXCLMARK_FLAG     2
#define DICTMARK_FLAG     3
#define VIEWALL_FLAG      4            // view all terms

/**********************************************************************/
/* Definition for term list entries of NT or FT term lists            */
/**********************************************************************/
typedef struct _TERM
{
  PSZ_W        pszName;                // name of term
  ULONG        ulFrequency;            // term frequency
  PSZ_W        pszTranslation;         // translation for term
  PSZ          pszDestination;         // exclusion list or dictionary
  PSZ_W        pszContext;             // context if changed by user
  PCONTEXTLIST pContextList;           // pointer to context list of term
  USHORT usFlags;                      // term process flags
  struct _Flags                        // term processing flags
  {
    unsigned short fMark        : 3;     // type of mark
    unsigned short fSelected    : 1;     // term is selected flags
    unsigned short fTop         : 1;     // term is first visible listbox item
    unsigned short fContext     : 1;     // term context has been selected
    unsigned short fDeleted     : 1;     // term is deleted
  } Flags;
} TERM, *PTERM;

/**********************************************************************/
/* Term table layout                                                  */
/**********************************************************************/
typedef struct _TERMTABLE
{
  USHORT usSize;                       // size of term table in bytes
  USHORT usUsedEntries;                // number of used entries in table
  USHORT usMaxEntries;                 // max. number of entries in table
  struct _TERMTABLE *pNextTable;       // ptr to next term table
} TERMTABLE, *PTERMTABLE;

/**********************************************************************/
/* List Header Structure                                              */
/**********************************************************************/
typedef struct _LISTHEADER
{
  PPTRLIST   DocumentList;             // list of text names (document names)
  PPTRLIST   DictionaryList;           // list of dictionaries
  PPTRLIST   OutDictionaryList;        // list of output dictionaries
  PPTRLIST   ExclListList;             // list of exclusion lists
  PPTRLIST   ExclDictList;             // list of exclusion dictionaries
  PSZ_W      pszCreateDate;            // ptr to list create date
  PSZ_W      pszCreateTime;            // ptr to list create time
} LISTHEADER, *PLISTHEADER;

/**********************************************************************/
/* IDA for Work With Lists Dialog                                     */
/**********************************************************************/
typedef struct _WWLDLGIDA
{
   BOOL       fDisabled;               // TRUE = dialog should behave as disabled
   HWND       hwnd;                    // dialog window handle
   HDC        hdc;                     // memory device context
   HPS        hps;                     // handle of our memory presentation space
   HBITMAP    hbmDict;                 // handle of dictionary bitmap
   HBITMAP    hbmExcl;                 // handle of exclusion list bitmap
   HBITMAP    hbmDel;                  // handle of delete bitmap
   HBITMAP    hbmNoMark;               // handle of ni mark bitmap
   HBITMAP    hbmDictG;                // handle of dictionary bitmap
   HBITMAP    hbmExclG;                // handle of exclusion list bitmap
   HBITMAP    hbmDelG;                 // handle of delete bitmap
   HBITMAP    hbmNoMarkG;              // handle of ni mark bitmap
   HBITMAP    hbmAllG;              // handle of ni mark bitmap
   USHORT     usListType;              // type of list being processed
   CHAR       szListType[MAX_EQF_PATH];// buffer for list type name
   CHAR       szListPath[MAX_EQF_PATH];// fully qualified list path
   CHAR       szListName[MAX_FNAME];   // name of list
   CHAR       szBuffer[CCHMAXPATH];    // general purpose buffer
   CHAR       szBuffer2[CCHMAXPATH];   // general purpose buffer
   CHAR_W     szUnicodeBuffer[2048];   // gen.purpose unicode buffer
   CHAR       szNotMarkedText[MAX_FNAME];// buffer for not-marked icon text
   CHAR       szAllMarkedText[MAX_FNAME];// buffer for all-marks  icon text
   USHORT     usView;                  // currently selected view
   PPOOL      pPool;                   // pointer to string pool of list
   PTERMTABLE pTermTable;              // ptr to term table(s)
   PCONTEXTTABLE pContextTable;        // ptr to context table(s)
   LISTHEADER ListHeader;              // list header data
   PSZ        pszExclList;             // ptr to name of current exclusion list
                                       // (string is stored in the string pool!)
   PSZ        pszDictionary;           // ptr to name of current dictionary
                                       // (string is stored in the string pool!)
   PFNWP      pButtonProc;             // adress of PM's button window procedure
   BOOL       fListChanged;            // true if list has been changed
   HWND       hwndDictLB;              // handle of dummy dictionary listbox
   //----------- Data of Mark for dictionary dialog -----------------
   SHORT      sMarkItem;               // currently processed item
   PTERM      pMarkTerm;               // currently processed term
   CHAR       szMarkBuffer[2048];      // general purpose buffer
   HPOINTER   hptrArrow;               // handle of arrow mouse pointer
   HPOINTER   hptrWait;                // handle of hourglass mouse pointer
} WWLDLGIDA, *PWWLDLGIDA;

/**********************************************************************/
/* IDA for List Edit Dialog                                           */
/**********************************************************************/
typedef struct _LISTEDITIDA
{
   HWND       hwnd;                    // dialog window handle
   USHORT     usListType;              // type of list being processed
   SHORT      sLangID;                 // language for abbreviation list
   CHAR       szListPath[MAX_EQF_PATH];// fully qualified list path
   CHAR       szListName[MAX_EQF_PATH];// name of current list
   CHAR       szSaveToName[MAX_FNAME]; // name of save to list
   CHAR       szBuffer[CCHMAXPATH];    // general purpose buffer
   CHAR       szBuffer2[CCHMAXPATH];   // general purpose buffer
} LISTEDITIDA, *PLISTEDITIDA;

/**********************************************************************/
/* Prototypes for internal list handler functions                     */
/**********************************************************************/
MRESULT ListCallBack
(
  PLISTCOMMAREA    pCommArea,
  HWND             hwnd,
  WINMSG           message,
  WPARAM           mp1,
  LPARAM           mp2
);
MRESULT LstInstCREATE( HWND );
MRESULT LstInstTERMINATE( HWND, WPARAM );
MRESULT LstInstINITIALIZE( HWND );
MRESULT LstInstINSERTNAMES( HWND, WPARAM, LPARAM );
MRESULT LstInstINITMENU( HWND, WPARAM, LPARAM );
MRESULT LstInstCOMMAND( HWND, USHORT, WPARAM, LPARAM );
INT_PTR CALLBACK LSTFRAMESUBPROC( HWND, USHORT, WPARAM, LPARAM );
VOID    LstSetDefaultPos( PSWP );
USHORT  EXPENTRY LSTNEWTERMSPROCESSOR( PVOID *, USHORT, HWND, PSZ );
USHORT  EXPENTRY LSTFOUNDTERMSPROCESSOR( PVOID *, USHORT, HWND, PSZ );
USHORT  EXPENTRY LSTEXCLISTPROCESSOR( PVOID *, USHORT, HWND, PSZ );
USHORT  EXPENTRY LSTNOISELISTPROCESSOR( PVOID *, USHORT, HWND, PSZ );
USHORT  LstBuildListboxItem ( PSZ, PSZ, PFDATE, PFTIME, ULONG );

INT_PTR CALLBACK LSTWORKWITHLISTSDLG ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK LSTEDITLISTSDLG ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK LSTMARKDICTDLG ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK LSTEXPORTDLG ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK LSTIMPORTDLG ( HWND, WINMSG, WPARAM, LPARAM );
USHORT  LstAddListProcessor ( HWND, PLSTIDA, HMODULE, PLISTPROC );
INT_PTR CALLBACK LSTBITMAPBUTTONWP( HWND, WINMSG, WPARAM, LPARAM );
VOID    LstWorkFillTermsLB( PWWLDLGIDA pIda );
USHORT  LstWorkProcessMarks ( PWWLDLGIDA pIda );
VOID    LstWorkSetSelFlags( PWWLDLGIDA pIda );
VOID    LstWorkClearSelFlags( PWWLDLGIDA pIda );

/**********************************************************************/
/* Procedures in EQFLSTUT.C                                           */
/**********************************************************************/
BOOL    LstWriteExclList( PSZ pszListName, PSZ_W pszTermBuf );
USHORT  LstGetListTypeFromName( PSZ pszListName );
USHORT  LstRefreshListsLB( PLSTIDA pIda, BOOL fSaveSelection );
USHORT  LstWriteSGMLList( PSZ, PLISTHEADER, PTERMTABLE, PCONTEXTTABLE, BOOL, LISTTYPES, USHORT );
USHORT  LstAddPtrToList( PPTRLIST *, PVOID );
BOOL    LstPrintList( PSZ pszListPath, USHORT usListType );
USHORT  LstAddContextIDToList( PCONTEXTLIST *ppContextList, USHORT usID );
USHORT  LstReadSGMLList( PSZ, PLISTHEADER, PTERMTABLE *, PCONTEXTTABLE *, PPOOL *, BOOL, LISTTYPES, USHORT );
USHORT  LstAddContext( PCONTEXTTABLE *ppContextTable, USHORT usID, PSZ_W pszContext );
PSZ_W     LstGetContext( PCONTEXTTABLE pContextTable, USHORT usID );
BOOL    LstSetContext( PCONTEXTTABLE pContextTable, USHORT usID, PSZ_W pszNewContext );
VOID    LstDestroyContextTable( PCONTEXTTABLE pContextTable );
USHORT  LstAddTerm( PTERMTABLE *ppTermTable, PTERM pTerm );
VOID    LstDestroyTermTable( PTERMTABLE pTermTable );
BOOL    LstDeleteList( PSZ pszList );

USHORT LstInsertListNames
(
   LISTTYPES  Type,                    // type of list being listed
   BOOL       fDetailed,               // TRUE = detailed list
   HWND       hwndLB,                  // handle of listbox being filled
   PSZ        pszBuffer,               // pointer to buffer for listbox item text
   BOOL       fDisableUpdate           // disable-listbox-update flag
);


__declspec(dllexport) BOOL List2DictInterface
(
  HWND hwndCaller,
  PTERM *ppTermList,
  USHORT usTerms,
  PUSHORT pusRC                        // pointer to caller's return code
);

MRESULT LstWWLControl
(
   HWND   hwndDlg,                     // dialog handle
   SHORT  sId,                         // id in action
   SHORT  sNotification                // notification
);


