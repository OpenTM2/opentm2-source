/*! \file
	Description: Private include file of word count handler
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

//--- stack size for wordcount stack
#define WCT_STACKSIZE 8192

// third state of fOrg Boolean
#define TMMATCHES_STATE 2

// fourth state of fOrg Boolean
#define DUPLICATE_STATE 3

// fith state of fOrg Boolean
#define FUZZYMATCH_STATE 4

//--- count return codes                                                 /*@1204A*/
#define ITEM_LOCKED     ERR_COUNT_BASE + 1 //flag that item id locked    /*@1204A*/
#define FILE_FOUND      ERR_COUNT_BASE + 2 //file found for counting     /*@1204A*/
#define FILE_NOT_FOUND  ERR_COUNT_BASE + 3 //no file found for counting  /*@1204A*/
#define FILE_LOCKED     ERR_COUNT_BASE + 4 //file used by another process/*@1204A*/
#define FILE_IN_ERROR   ERR_COUNT_BASE + 5 // file env. could not be activated
#define FILE_NOINFO     ERR_COUNT_BASE + 6 // file contains no match info

#define CNT_OUTPUT_EXTENSION "CNT"               //extension for output file

#define HEADER            0         //defines the types of lines
#define FILE_COUNT_LINE   1         //for wordcount
#define ERROR_LINE        2         //output file
#define SUMMARY_LINE      3         //output file
#define LOCKED_LINE       4         //output file
#define NOINFO_LINE       5         //output file

#define EQUAL_LINE_LONG  "=========================================================================================="
#define MINUS_LINE_LONG  "------------------------------------------------------------------------------------------"
#define EQUAL_LINE_SHORT "=========================================================================="
#define MINUS_LINE_SHORT "--------------------------------------------------------------------------"

#define EQUAL_LINE_TMMATCH "==========================================================================================="
#define MINUS_LINE_TMMATCH "-------------------------------------------------------------------------------------------"
#define EQUAL_LINE_TMREPLMATCH "================================================================================================================="
#define MINUS_LINE_TMREPLMATCH "-----------------------------------------------------------------------------------------------------------------"


#define SUMMARY_EQUALS "========="

#define MINUS_LINE_DUPLICATE         "------------------------------------------------------------------------------------------------------------------"
#define EQUAL_LINE_DUPLICATE         "=================================================================================================================="

#define MINUS_LINE_MEMINFODUPLICATE         "--------------------------------------------------------------------------------------------------------------------------------------------------"
#define EQUAL_LINE_MEMINFODUPLICATE         "=================================================================================================================================================="

#define STR_CNT00_FILE_DUPLICATE_GML "Documents                           | Translated | Untranslated  | Duplicate words | To be translated  |  Markup |"

#define STR_CNT00_FILE_DUPLICATE_MEMINFO_GML1 "Documents                          | Duplicate    |     95-99% fuzzy |     75-94% fuzzy  |     0-74% fuzzy |     Total        |    Exact Matches |"
#define STR_CNT00_FILE_DUPLICATE_MEMINFO_GML2 "                                   | words        |     match words  |     match words   |     match words |     untranslated |    (100% match   |"
#define STR_CNT00_FILE_DUPLICATE_MEMINFO_GML3 "                                   | (repetitions)|                  |                   |                 |     words        |    words         |"

#define MINUS_LINE_FUZZYMATCH         "-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------"
#define EQUAL_LINE_FUZZYMATCH         "================================================================================================================================================================================================="
#define STR_CNT00_FILE_FUZZYMATCH_GML "Documents                               |             Total |          Class-0 |          Class-1 |          Class-2 |          Class-3 |          Class-4 |          Class-5 |        Class-6up |"

//                                   123456789012345678 12345678901234567 12345678901234567 12345678901234567 1234567


//positions for count instance window
#define COUNT_X                60
#define COUNT_Y                60
#define COUNT_ORIG_CX         600
#define COUNT_TRAN_CX         700
#define COUNT_CY              400

//defines for colums in counted listbox
#define DOCNAME           0
#define SHORTNAME         1
#define TOBETRANSLATED    2
#define TRANSLATED        3
#define TOTAL             4
#define MARKUP            5
#define NL                6
#define MACHMATCH         7
#define NOMATCH           8
#define REPL              9
#define FUZZYREPL        10

#define EQFCOUNTMOD "EQFCOUNTMOD"

// output format of report
typedef enum
{
  ASCII_FORMAT,
  HTML_FORMAT,
  XML_FORMAT
} REPORT_FORMAT;


typedef struct _cnt                 //count structure, used for data exchange
{                                   //between count handler, -instance, -dialog
   OBJNAME szFolderObjName;         //folder object name
   CHAR    szFolderName[MAX_FILESPEC];//folder name
   CHAR    szLongFolderName[MAX_LONGFILESPEC];// long folder name
   OBJNAME szCountInstanceObjName;  //count instance object name
   OBJNAME szObjNameBuffer;         //primary buffer for an object name
   HWND    hwndCountHandlerLB;         //handle of LB containig files to be counted
   /*******************************************************************/
   /* The following BOOL fOrg is a 5state BOOL:                       */
   /* TRUE            = count original                                */
   /* FALSE           = count translation                             */
   /* TMMATCHES_STATE = count/display TM match info                   */
   /* DUPLICATE_STATE = count duplicates                              */
   /* FUZZYMATCH_STATE = count fuzzy matches                          */
   /*******************************************************************/
   BOOL    fOrg;                    //status of original RB
   BOOL    fTran;                   //status of original RB
   BOOL    fTranslated;             //status of already translated CB
   BOOL    fToBeTran;               //status of to be translated CB
   BOOL    fTotal;                  //status of total CB
   BOOL    fScreen;                 //status of screen CB
   BOOL    fFile;                   //status of file CB
   BOOL    fPrint;                  //status of print CB
   BOOL    fReplMatch;              //status of replace match checkbox
   CHAR    szOutputFile[MAX_LONGPATH+MAX_FILESPEC]; //filename for output file
   CHAR    szDriveLetter[MAX_DRIVE];//drive letter for error message
   CHAR    szTitle[35];             //title text of instance window
   char    szFormat[MAX_FILESPEC];  // folder's documents format table
   char    szMemory[MAX_LONGFILESPEC];// folder's translation memory
   SHORT   sSrcLanguage;            // source language
   SHORT   sTgtLanguage;            // target language
   CHAR    szFileObjName[MAX_LONGPATH]; //@DOE
   CHAR    szFileName[MAX_FILESPEC];//@DOE
   CHAR    szSourceLang[MAX_LANG_LENGTH]; // document source language
   CHAR    szTargetLang[MAX_LANG_LENGTH]; // document target language
   CHAR    szBuffer[4096];           // general purpose buffer
   USHORT  usFillMode;               // folder or selected documents only...
   BOOL    fBatch;                  // TRUE = we are in batch mode
   HWND    hwndErrMsg;              // window handle to be used for error messages
   PDDEWRDCNT pDDEWrdCnt;           // ptr to batch data structure
   USHORT  usNoOfDocs;              // number of documents
   PSZ     pDocNameBuffer;          // document name buffer
   PSZ     pszCurDoc;               // current document in document name buffer
   OBJNAME szParentObjName;         // parent (sub)folder object name
   CHAR    szDisplayName[MAX_LONGFILESPEC];// folder/subfolder name for display
   ULONG   ulSrcOemCP;
   ULONG   ulTgtOemCP;
   BOOL    fXMLOutput;              // write output in XML format instead of TEXT
   REPORT_FORMAT OutputFormat;      // final format of output file
   BOOL    fInclMemMatch;           // include memory match info to duplicate word count
   USHORT  usFormat;                // selected output format
} CNT, * PCNT;

/**********************************************************************/
/* For TM match counting the count values below are re-used as        */
/* follows:                                                           */
/* ulWToBeTran     = untranslated words (= no proposal)               */
/* ulWTransMod     = words of fuzzy matches                           */
/* ulWTransCopied  = words of exact-exact matches                     */
/* ulWTransScratch = words of exact matches (one proposal)            */
/* ulWMarkup       = words of exact matches (more than one proposal)  */
/* ulWTotal        = total number of words                            */
/* ulWTransSource  = replace match words                              */
/**********************************************************************/

// max number of fuzzy columns
#define MAX_FUZZY_COLS  5

// max number of class columns
#define MAX_CLASS_COLS  7

typedef struct _LCOUNT
{
   ULONG     ulWToBeTran;            //number of words to be translated
   ULONG     ulWTransMod;            //number of translated words
   ULONG     ulWTransCopied;         //number of translated words
   ULONG     ulWTransScratch;        //number of translated words
   ULONG     ulWMarkup;              //number of markup words
   ULONG     ulWTotal;               //number of total words in file
   ULONG     ulWTransSource;         //number of transl. words in source file
   ULONG     ulFuzzyRepl;            //number of words in fuzzy replace matches
   ULONG     ulMachineMatch;         //number of machine matchs
   ULONG     ulWTran;                //number of words to be translated (only for duplicate word count)
   ULONG     ulWDuplicate;           //duplicate wct only: number of duplicate words (only for duplicate word count)
   ULONG     ulWUntranslated;        //duplicate wct only: number of untranslated words 
   // for duplicate word count with memory match info
   ULONG     ulWFuzzy[MAX_FUZZY_COLS]; // columns with fuzzy matches
   // for fuzzy match word count 
   ULONG     ulWClass[MAX_CLASS_COLS]; // number of words in fuzzy matches per class
   ULONG     ulSClass[MAX_CLASS_COLS]; // number of segments in fuzzy matches per class
} LCOUNT, *PLCOUNT;

typedef struct _cntinstance_ida        //ida of count instance window procedure
{
   IDA_HEAD IdaHead;                   //common part of count instance ida
   PCNT   pCnt;                        //pointer to count structure
   PSZ    pStack;                      // pointer to stack
   CHAR   chTagTableName[MAX_EQF_PATH];// tag table name
   CHAR   chQFTagTable[MAX_EQF_PATH];  // internal QF tag table name
   CHAR   szCountFile[MAX_LONGPATH+MAX_FILESPEC]; //full path to actual file to be counted
   CHAR   szCountFileName[MAX_FILESPEC]; //name of actual file to be counted
   HWND   hwndCountLB;                 //handle of count listbox
   CHAR   szFolderInfo[50];            //folder info text  Folder : xxxxxxx
   HWND   hwndFolderInfo;              //handle of folder info text
   CHAR   szColDocuments[50];          //column header documents
   CHAR   szColToBe[50];               //column header untranslated
   CHAR   szColTran[50];               //column header translated
   CHAR   szColNL[50];                 //column header NL........
   CHAR   szColMarkup[50];             //column header Markup
   CHAR   szColTotal[50];              //column header total
   CHAR   szColMachMatch[50];          //column machine matches
   CHAR   szCounting[50];              //contains text: counting...
   CHAR   szError[50];                 //contains text Error
   CHAR   szNoInfo[50];                //contains text no match info
   CHAR   szTotalText[50];             //contains text Total
   CHAR   szColNoMatch[50];            //contains column header No match
   CHAR   szColReplMatch[50];          //contains column header Replace
   CHAR   szColFuzzyReplMatch[50];     //contains column header FuzzyReplace
   LCOUNT lCountFile;                  // result for one file
   LCOUNT lCountSum;                   // summary
   CHAR   szBuffer[8200];              //buffer to create string for output file
   CHAR   szLine[500];                 //buffer for colum listbox
   CHAR   szFileLine[500];             //buffer for output file lines
   SHORT  sCountNum;                   //number of items (documents) to be counted
   SHORT  sIndexItem;                  //index of item (document) of count listbox
   HFILE  hfFileHandle;                //DOS file handle of output file
   BOOL   fClose;                      //close flag for output file
   LONG   lPelsPerLine;                //pels per line of count instance window
   USHORT usRc;                        //error indicator for word count utility
   CHAR   szErrorTextBuffer[50];       //contains text: !!!! Error counting file
   CHAR   szStringBuffer[1500];        //dummy string buffer
   BOOL   fKill;                       // thread is requested to be killed
   BOOL   fThreadActive;               // thread is active
   USHORT usStatus;                    // current status
   TBDOCUMENT  TBTgtDoc;               // document structure target
   TBDOCUMENT  TBSrcDoc;               // document structure source
   CHAR   szMsgBuf[4096];              // message buffer for help
   BOOL   fHelpInitActive;             // help instance initialization is active
   CHAR   szLocked[20];                //contains text loacked //@DOE
   BOOL   fUseSourceLng;               // use source language if tgt not avail..
   HWND   hwndDocLB;                   // listbox with document short names
   CHAR   szLongName[MAX_LONGFILESPEC];// buffer for document long name
   HWND   hwndLB;                      // handle of listbox (same as ColumnListbox)
   BOOL   fTimerProcessing;            // currently processing the timer...
   CHAR   szShortenedName[MAX_LONGFILESPEC]; // long file name with abbreviation
   BOOL   fFolderLocked;               // folder-has-been-locked flag
   CHAR   szMemPath[MAX_EQF_PATH];     // path name of temporary memory
   HTM    htm;                         // handle of temporary memory
   CHAR   szEscapeBuffer[1500];        // buffer to escape strings for XML output
   BOOL    fWithMatchInfo;             // TRUE = include memory match information
   CHAR   szDocMemName[MAX_LONGFILESPEC]; // name document memory
   CHAR   szNewDocMem[MAX_LONGFILESPEC]; // name of next document memory
   CHAR   szDocMemPath[MAX_LONGFILESPEC]; // path of document memory
   int    iNumOfFolMems;               // number of folder memories
   BOOL   fStopAtFirstExact;           // stop at first exact match flag from folder properties
   PVOID  pvMemLookupData;             // ptr to memory lookup data area
   CHAR_W szSegmentSource[MAX_SEGMENT_SIZE+1]; // buffer for segment source
} CNTIDA, *PCNTIDA;

typedef struct _handler_ida            //ida of count handler
{
   IDA_HEAD IdaHead;                   //common part of count instance ida
   CHAR     szBuffer[MAX_LONGPATH +    //string fuffer for instance name
               sizeof( COUNTHANDLER)];
} HANDLER_IDA, *PHANDLER_IDA;

static SHORT sOrgGMLView[]  = { DOCNAME, TOBETRANSLATED, MARKUP, TOTAL,
                                CLBLISTEND };
static SHORT sTranGMLView[] = { DOCNAME, TRANSLATED, TOBETRANSLATED, NL,
                                MARKUP, CLBLISTEND };
static SHORT sTMMatchGMLView[] = { DOCNAME, TOBETRANSLATED, TRANSLATED,
                                   TOTAL, MARKUP, NL, MACHMATCH, NOMATCH, CLBLISTEND };
static SHORT sTMReplMatchGMLView[] = { DOCNAME, TOBETRANSLATED, TRANSLATED,
                                       TOTAL, MARKUP, NL, MACHMATCH, NOMATCH, REPL, FUZZYREPL, CLBLISTEND };
static SHORT sDuplicateGMLView[] = { DOCNAME, 2, 3, 4, 5, 6, CLBLISTEND };

static SHORT sDuplicateMemInfoGMLView[] = { DOCNAME, 2, 3, 4, 5, 6, 7, CLBLISTEND };

static SHORT sFuzzyMatchGMLView[] = { DOCNAME, 2, 3, 4, 5, 6, 7, 8, 9, CLBLISTEND };

//declare table for colum listbox
static CLBCOLDATA ColTable[] =

{ { "",   16, AUTOWIDTHTEXT_DATA, DT_LEFT  },//names of counted files
  { "",   12, AUTOWIDTHTEXT_DATA, DT_LEFT  },   //short name of counted files
  { "",   17, AUTOWIDTHTEXT_DATA, DT_RIGHT },   //untranslated words
  { "",   13, AUTOWIDTHTEXT_DATA, DT_RIGHT },   //translated words
  { "",    7, AUTOWIDTHTEXT_DATA, DT_RIGHT },   //total number of words
  { "",   12, AUTOWIDTHTEXT_DATA, DT_RIGHT },   //markup
  { "",   12, AUTOWIDTHTEXT_DATA, DT_RIGHT },   //NL words
  { "",   12, AUTOWIDTHTEXT_DATA, DT_RIGHT },   //for TMMatches only: mach match
  { "",   12, AUTOWIDTHTEXT_DATA, DT_RIGHT },   //for TMMatches only: no match
  { "",   12, AUTOWIDTHTEXT_DATA, DT_RIGHT },   //for TMMatches only: replace
  { "",   12, AUTOWIDTHTEXT_DATA, DT_RIGHT },   //for TMMatches only: fuzzyrepl
  { NULL, 0,  TEXT_DATA,          0      } };

static CLBCTLDATA CntCLBData =
{  sizeof(CLBCTLDATA),                 // overall size of structure
   11,                                 // we have 11 data columns
   1,                                  // one character space between columns
   SYSCLR_WINDOWSTATICTEXT,            // paint title in color of static text
   SYSCLR_WINDOW,                      // background is normal window background
   SYSCLR_WINDOWTEXT,                  // paint item in color of window text
   SYSCLR_WINDOW,                      // background is normal window background
   X15,                                // use X15 character as data seperator
   sOrgGMLView,                        // set current (= last used) view list
   sOrgGMLView,                        // set default view list
   sOrgGMLView,                        // set user set details view list
   sOrgGMLView,                        // set view list for 'name' view option
   sOrgGMLView,                        // set sort criteria list
   ColTable };                         // set address of column definition table


typedef struct _ida                          //instance area for dialog
{
   PCNT        pCnt;                         //pointer to count structure
   HPROP       hpropFolder;                  //handle of folder properties
   PPROPFOLDER ppropFolder;                  //pointer to folder properties
   CHAR        szFolderPropName[MAX_PATH144+MAX_FILESPEC];
                                             //name of folder properies
   CHAR        szDrives[MAX_DRIVELIST];      //list of available drives
   CHAR        szSysPath[MAX_EQF_PATH];      //MAT system path
   CHAR        szWorkString[1000];           //string buffer
   CHAR        szGMLText[10];                //string for word GML
} IDA, *PIDA;

/**********************************************************************/
/* status process can be ....                                         */
/**********************************************************************/
#define CNT_COUNTING          1
#define CNT_ERROR             2
#define CNT_START             3
#define CNT_STOP              4
#define CNT_FINISHED          5

#define CNT_TIMER_ID       3           // timer id
#define CNT_TIMER_VAL      200L        // process new file each half sec.

/**********************************************************************/
/* internal QF tag table - remember: drive identifier will be reset   */
/* to system drive later on...                                        */
/**********************************************************************/
#define CNT_QFTAGTABLE "C:\\EQF\\TABLE\\QFTAGS.IBL"

BOOL CNTInit ( PCNTIDA pCNTIda );

USHORT CntFuncPrepCount
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  LONG        lOptions,                // options for word count
  PSZ         pszOutFile               // name of output file
);
__declspec(dllexport) USHORT CntFuncCountProcess
(
  PFCTDATA    pData                    // ptr to function call interface data
);

__declspec(dllexport) USHORT   CntWriteToFile( PCNTIDA, HWND, USHORT );

