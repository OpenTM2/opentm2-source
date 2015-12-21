//+----------------------------------------------------------------------------+
//|EQFDDE.H                                                                    |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+


#ifndef EQFDDE_INCLUDED

#define  MAX_CMDLINELEN 4096           // max length of cmdline
#define  CURDIR_NAME    "/$CD="        // name of Current directory option
#define MAX_FULLPATH  255              // max length of fully qualified paths

/* Maximum string sizes for various names and text */
#define MAX_DDEAPPL_SZ    64            // Arbritrary max string size
#define MAX_DDETOPIC_SZ   64            // not defined by protocol
#define MAX_DDEITEM_SZ    64            // "   "

/* Default DDE communications parameters */

/* This DDE Applications Name */
#define  CV_DDEAPPNAME "TranslationManager"
#define  CV_DDETOPIC   "Batch Mode API"
#define  CV_DDEITEM    "Batch Mode API"

typedef struct _DDERETURN
{
  USHORT  usRc;                        // error code
  CHAR chMsgBuf[256];                  // message text
} DDERETURN, *PDDERETURN;

/**********************************************************************/
/* defines for DDE                                                    */
/**********************************************************************/
#define  MAX_MSG_SZ    300                  // max size of text string

#define MAXALLBATCH  50                     // max no of batches possible
#define MAXBATCH  20                        // max no of batches in 1 task
#define MAXMAND   10                        // max no of mandatory batches
#define DELSLASH      '/'                   // delimiter for cmdline strings
#define DELSLASH_STR  "/"                   // delimiter for cmdline strings
#define LISTINDICATOR '@'
#define LISTSTART     '('                   // indicator for list start
#define LISTEND       ')'                   // indicator for list end
#define YES_STRING    "YES"                 // the yes string
#define NO_STRING     "NO"                  // the no  string
#define TARGET_STRING "TARGET"              // the TARGET string
#define SOURCE_STRING "SOURCE"              // the SOURCE string
#define REPLACE_STRING "REPLACE"            // the replace string
#define IGNORE_STRING  "IGNORE"             // the ignore string
#define COMBINE_STRING "COMBINE"            // the combine string
#define LISTSTART_STRING "("
#define LISTEND_STRING   ")"
#define COMMA_STRING     ","
#define SNOMATCH_STRING "SNOMATCH"        // the SNOMATCH string
#define TMMATCH_STRING "TMMATCH"          // the TMMATCH string

// strings for counting report's

  #define BATCH_RPT_REPORT_1          "HISTORY"
  #define BATCH_RPT_REPORT_2          "COUNTING"
  #define BATCH_RPT_REPORT_3          "CALCULATING"
  #define BATCH_RPT_REPORT_4          "PRE-ANALYSIS"
  #define BATCH_RPT_REPORT_5          "REDUNDANCY"
  #define BATCH_RPT_REPORT_6          "REDSEGLIST"

// strings for counting report's  options

  #define BATCH_RPT_OPTIONS_1         "DATE"
  #define BATCH_RPT_OPTIONS_2         "BRIEF"
  #define BATCH_RPT_OPTIONS_3         "DETAIL"
  #define BATCH_RPT_OPTIONS_4         "WITH_TOTALS"
  #define BATCH_RPT_OPTIONS_5         "WITHOUT_TOTALS"
  #define BATCH_RPT_OPTIONS_6         "BASE"
  #define BATCH_RPT_OPTIONS_7         "BASE_SUMMARY"
  #define BATCH_RPT_OPTIONS_8         "BASE_SUMMARY_FACT"
  #define BATCH_RPT_OPTIONS_9         "SUMMARY_FACT"
  #define BATCH_RPT_OPTIONS_10        "FACT"


#define DDE_COMBINE 0
#define DDE_IGNORE  1
#define DDE_REPLACE 2

/**********************************************************************/
/* none of the following strings is allowed to be longer than         */
/* MAX_STRINGLEN:                                                     */
/* YES_STRING, NO_STRING, TARGET_STRING, SOURCE_STRING, REPLACE_STRING*/
/* IGNORE_STRING, COMBINE_STRING                                      */
/**********************************************************************/
#define MAX_STRINGLEN   20

#define      ID_DDEINST_WINDOW 12           // id of instance window
#define      WC_EQF_DDE    "EQF_DDE"


/**********************************************************************/
/* typedef's for DDE                                                  */
/**********************************************************************/
/**********************************************************************/
/* if more than 30 BATCHES, change MAXALLBATCH                        */
/**********************************************************************/
typedef  enum _BATCHCMD
{
  BATCH_TASK,                          //task to be done
  BATCH_FLD,                           //filename of a folder
  BATCH_FILES,                         //file name or list of files
  BATCH_OUT,                           //output file name
  BATCH_TYPE,                          //type of selection
  BATCH_MARKUP,                        //name of selected markup table
  BATCH_FROMDRIVE,                     //name of drive obj copied from
  BATCH_TODRIVE,                       //name of drive obj copied to
  BATCH_FROMPATH,                      //name of path obj copied from
  BATCH_TOPATH,                        //name of Path obj copied to
  BATCH_EDIT,                          //name of editor
  BATCH_SRCLNG,                        //name of source language
  BATCH_TGTLNG,                        //name of target language
  BATCH_DESC,                          //description string
  BATCH_NAME,                          //name of obj to be created
  BATCH_MEM,                           // translation memory
  BATCH_DICT,                          //dictionary or list of dictionaries
  BATCH_FILT,                          //filter
  BATCH_OPTIONS,                       //options for different tasks
  BATCH_EXCLLIST,                      //exclusion list
  BATCH_EXCLDICT,                      //exclusion dictionary
  BATCH_OVERWRITE,                     // overwrite existing files
  BATCH_SGMLMEM,                       // name of the SGML output file
  BATCH_NOANA,                         // NO analysis
  BATCH_NOTMDB,                        // NO translation memory database
  BATCH_NOCONF,                        // NO confirmation message at end
  BATCH_LEVEL ,                        // level of match
  BATCH_CURDIR,                        // current drive and directory
  BATCH_PASSWORD,                      // password for dictionary
  BATCH_ALIAS,                         // alias name
  BATCH_REPORT,                        // WORDCOUNT Report
  BATCH_PROFILE,                       // WORDCOUNT Profile for options
  BATCH_STARTPATH,                     // Doc Import/Export: start path
  BATCH_CONVERSION,                    // FolCreate and Doc Import/export: conversion
  BATCH_FORMAT,                        // reports: format for output file
  BATCH_SEGMENT,                       // open doc: segment number
// ------- insert new /xxx keywords before this line ---------
  BATCH_SERVER,                        // server for remote resources
  BATCH_VISUAL,                        // Visual required
  BATCH_PREPARE,                       // prepare alignment and save it
  BATCH_REVMEM,                        // reversed translation memory
  BATCH_REVALL,                        // select all matches or not
  BATCH_CLEAR,                         // clear all [m] flags
  BATCH_SET,                           // set all [m] flags
  BATCH_YES,                           // YES for clear and/or set
  BATCH_NO,                            // NO for clear and/or set
  TMT_FROMMU,                          // TMT: from markup
  TMT_TOMU,                            // TMT: to markup
  TMT_FROMLANG,                        // TMT: from target language
  TMT_TOLANG,                          // TMT: to target language
  TMT_DOC,                             // TMT: searched document
  TMT_DATE,                            // TMT: new segment date
  TMT_MTLIST,                          // TMT: name of list containing the entries for the MT flag list
  BATCH_END                            // end of list indicator
} BATCHCMD;

typedef enum _DDETASK
{
  TASK_FLDCRT,                         //create a folder
  TASK_FLDEXP,                         //export a folder
  TASK_FLDIMP,                         //import a folder
  TASK_WORDCNT,                        //wordcount a folder
  TASK_XLATE,                          //import,translate,export doc
  TASK_DICIMP,                         //dictionary import
  TASK_DICEXP,                         //dictionary export
  TASK_DOCIMP,                         //document import
  TASK_DOCEXP,                         //document export
  TASK_MEMIMP,                         //import translation memory
  TASK_MEMEXP,                         //export translation memory
  TASK_MEMCRT,                         //create translation memory
  TASK_LOOKUP,                         //lookup a sentence
  TASK_ANALYSIS,                       //analyse docs or folder
  TASK_LSTCRT,                         //create a list
  TASK_STATUS,                         //return object status
  TASK_DOCOPEN,                        //open a document
  TASK_MEMDEL,                         //delete translation memory
  TASK_MEMORG,                         //organize translation memory
  TASK_FLDDEL,                         // delete folder
  TASK_DOCDEL,                         // delete document
  TASK_CNTRPT,                         //wordcount a folder
  TASK_ARCHTM,                         //build archive TM
  TASK_END
} DDETASK;

typedef struct _BATCHLIST              // list of batches
{
  BATCHCMD   BatchCmd;
} BATCHLIST, *PBATCHLIST ;


typedef struct _DDECLIENT              // for each instance, i.e.client
{
  HWND hwndInstance;                   // instance handle
  HWND hwndDDEClient;                  // handle of DDE client
  IDA_HEAD  IdaHead;                   // common part of dde client instance
  CHAR szObjName[MAX_DDETOPIC_SZ];     // object name of instance object
  CHAR szTopic[MAX_DDETOPIC_SZ];       // Conversation topic type
  CHAR szItem[MAX_DDEITEM_SZ];         // Conversation item type
  CHAR szCmdLine[MAX_CMDLINELEN];      // uppercases command line
  CHAR szOrgCmdLine[MAX_CMDLINELEN];   // original command line
  PSZ  pszActCmd;                      // active command
  DDETASK  Task;                       // task of this instance
  PBATCHLIST pBatchList;               // Batchlist of this task
  PBATCHLIST pMandList;                // list of mandatory batches of task
  PVOID pTaskIda;                      // ptr to function for this task
  CHAR       szHandler[25];            // TM/2 handler for cur task
  DDERETURN  DDEReturn;                // return structure

  HWND hwndErrMsg;                     // handle of DDE client
} DDECLIENT, *PDDECLIENT;

typedef struct _TASKLIST
{
  DDETASK TaskCmd;                     // task identifier
  CHAR szDesc[15];                     // description of item
  BOOL (*function)( PDDECLIENT );      // special function for the task
  CHAR szHandler[25];                  // handler who cares about cmd
  BATCHLIST BatchList[MAXBATCH];       // parameter list for this task
  BATCHLIST MandList[MAXMAND];         // list of mandatory parameters
} TASKLIST, *PTASKLIST;


typedef struct _CMDLIST
{
  BATCHCMD BatchCmd;                   // identifier
  CHAR    szDesc[ 15 ];                // full name of item
  CHAR    szShortCut[ 10 ];            // shortcut of item
} CMDLIST, *PCMDLIST;



/**********************************************************************/
/* structure for DDE batch handler                                    */
/**********************************************************************/
typedef struct _DDEBATCH
{
  IDA_HEAD  IdaHead;                   //common part of dde server handler
} DDEBATCH, *PDDEBATCH;

/**********************************************************************/
/* structure which holds info for the task document import +export    */
/**********************************************************************/
typedef struct _DOCIMPEXP
{
  HWND hwndOwner;                          // handle of instance window
  CHAR chFldName[MAX_LONGFILESPEC];        // foldername
  CHAR chCurDir[ MAX_PATH144 ];            // current drive with directory
  CHAR chListName[MAX_FULLPATH];           // list filename specified
  BOOL fOverwrite;                         // overwrite existing files  KIT1288c
  USHORT usActFile;                        // active file to be worked on
  USHORT usFileNums;                       // number of files in filelist
  PSZ *ppFileArray;                        // ptr to file array
  BOOL fInternal;                          // TRUE if internal format(exp)
  BOOL fSource;                            // TRUE if source exported
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                          // return structure
  PSZ   *ppCurDirArray;                          // list of current directories
  CHAR szMem[MAX_LONGFILESPEC];            // name of translation memory
  CHAR szFormat[MAX_FNAME];                // name of markup/format table
  CHAR szEdit[MAX_FNAME];                  // name of editor
  LANGUAGE     szSourceLang;               // source language
  LANGUAGE     szTargetLang;               // target language
  CHAR szAlias[MAX_LONGFILESPEC];          // alias for document
  BOOL fSNOMATCH;                          // export SNOMATCH file
  HWND hwndErrMsg;                         // HWND to be used for error messages
  CHAR szStartPath[MAX_LONGFILESPEC];      // buffer for start path (if specified)
  CHAR szConversion[MAX_DESCRIPTION];      // buffer for document export conversion
  PSZ  pszFileNameBuffer;                  // ptr to buffer for file names
  CHAR szBuffer[MAX_LONGPATH];             // general purpose buffer
  ULONG  ulSubFolderID;                    // ID of subfolder/folder
  // fields for the export in validation format
  BOOL  fValFormat;                        // TRUE = export document(s) in validation format
  BOOL  fValCombine;                       // TRUE = combine documents into a single output file
  BOOL  fValProtSegs;                      // TRUE = include protected segments
  LONG  lValOutputFormat;                  // selected output format
  LONG  lValExportHandle;                  // handle of validation format export
  LONG  lOptions;                          // specified import/export options
  BOOL  fPlainXML;                         // TRUE = export document(s) in plain XML format
  BOOL  fWithTrackID;                      // TRUE = export document(s) with TVT tracking IDs
} DOCIMPEXP, *PDOCIMPEXP;

/**********************************************************************/
/* structure which holds info for the task dictionary import + export */
/**********************************************************************/
typedef struct _DICIMPEXP
{
  HWND hwndOwner;                          // handle of instance window
  CHAR chDictName[MAX_LONGFILESPEC];              // dictionary name (target)
  CHAR chCurDir[ MAX_PATH144 ];            // current drive with directory
  CHAR chListName[MAX_PATH144+MAX_FILESPEC];// dictionary to be imported
  USHORT usOption;                         // 1= combine, 2=replace, 3=ignore
  CHAR chDicPassWord[MAX_FNAME];           // password for dictionary
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurrency of each spec
  DDERETURN  DDEReturn;                    // return structure
  PSZ   *ppCurDirArray;                          // list of current directories
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DICIMPEXP, *PDICIMPEXP;

/**********************************************************************/
/* structure which holds info for the task "folder create"            */
/**********************************************************************/
typedef struct _DDEFOLCRT
{
  HWND hwndOwner;                          // handle of instance window
  CHAR szName[MAX_LONGFILESPEC];           // folder name
  CHAR szDescr[MAX_DESCRIPTION];           // folder description
  CHAR chToDrive;                          // target drive of folder
  CHAR szMem[MAX_LONGFILESPEC];            // name of translation memory
  CHAR szFormat[MAX_FNAME];                // name of markup/format table
  CHAR szEdit[MAX_FNAME];                  // name of editor
  CHAR szDicts[NUM_OF_FOLDER_DICS][MAX_LONGFILESPEC]; // folder's list of dicts
  LANGUAGE     szSourceLang;               // source language
  LANGUAGE     szTargetLang;               // target language
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                    // return structure
  char MemTbl[MAX_NUM_OF_READONLY_MDB][MAX_LONGFILESPEC];  // folder's list of mems
  HWND hwndErrMsg;                         // HWND to be used for error messages
  CHAR       szConversion[MAX_DESCRIPTION];// document export conversion
} DDEFOLCRT, *PDDEFOLCRT;

/**********************************************************************/
/* structure which holds info for the task "folder export"            */
/**********************************************************************/
typedef struct _DDEFOLEXP
{
  HWND   hwndOwner;                    // handle of instance window
  CHAR   szFolder[MAX_LONGFILESPEC];   // folder name
  PSZ    pszNote;                      // points to buffer containing note
  CHAR   chToDrive;                    // target drive of folder export
  BOOL   fToPath;                      // TRUE = target path given
  CHAR   chToPath[MAX_FULLPATH];       // target path of folder export
  USHORT usFileNums;                   // number of files in filelist
  PSZ    *ppFileArray;                 // ptr to file array
  BOOL   fWithDict;                    // TRUE = include dictionaries
  BOOL   fWithMem;                     // TRUE = include Translation Memory
  BOOL   fDelete;                      // TRUE = delete folder after export
  BOOL   fOverWrite;                   // TRUE = overwrite existing packages
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                // return structure
  BOOL   fWithROMem;                   // TRUE = include ReadOnly Memory during export
  CHAR   chListName[MAX_FULLPATH];     // list filename specified
  PSZ   *ppCurDirArray;                // list of current directories
  BOOL   fWithDocMem;                  // TRUE = include document Memory during export
  HWND hwndErrMsg;                     // HWND to be used for error messages
  BOOL   fAsMasterFolder;              // TRUE = export folder as master folder
} DDEFOLEXP, *PDDEFOLEXP;

/**********************************************************************/
/* structure which holds info for the task "folder import"            */
/**********************************************************************/
typedef struct _DDEFOLIMP
{
  HWND   hwndOwner;                    // handle of instance window
  CHAR   szFolder[MAX_LONGFILESPEC];   // folder name
  CHAR   chFromDrive;                  // source drive for folder import
  BOOL   fFromPath;                    // TRUE = source path given
  CHAR   chFromPath[MAX_FULLPATH];     // source path for folder import
  CHAR   chToDrive;                    // target drive for imported folder
  BOOL   fWithDict;                    // TRUE = include dictionaries
  BOOL   fWithMem;                     // TRUE = include Translation Memory
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                // return structure
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DDEFOLIMP, *PDDEFOLIMP;

/**********************************************************************/
/* structure which holds info for the task "wordcount"                */
/**********************************************************************/
typedef struct _DDEWRDCNT
{
  HWND   hwndOwner;                    // handle of instance window
  CHAR   szFolder[MAX_LONGFILESPEC];   // folder name
  USHORT usFileNums;                   // number of files in filelist
  PSZ    *ppFileArray;                 // ptr to file array
  CHAR   szOutFile[CCHMAXPATH+1];      // folder name
  BOOL   fOverWrite;                   // TRUE = overwrite any existing file
  BOOL   fTarget;                      // TRUE = count target document
  BOOL   fTMMatch;                     // TRUE = count TM matches
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                // return structure
  CHAR chListName[MAX_PATH144+MAX_FILESPEC];  // list filename specified
  PSZ   *ppCurDirArray;                // list of current directories
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DDEWRDCNT, *PDDEWRDCNT;



/**********************************************************************/
/* structure which holds info for the task "CountingReport"           */
/**********************************************************************/
typedef struct _DDECNTRPT
{
  HWND   hwndOwner;                    // handle of instance window
  CHAR   szFolder[MAX_LONGFILESPEC];   // folder name
  USHORT usFileNums;                   // number of files in filelist
  USHORT usFilesInFolder;              // number of files in folder
  PSZ    *ppFileArray;                 // ptr to file array
  CHAR   szOutFile[CCHMAXPATH+1];      // folder name
  BOOL   fOverWrite;                   // TRUE = overwrite any existing file
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                // return structure
  CHAR chListName[MAX_PATH144+MAX_FILESPEC];  // list filename specified
  PSZ   *ppCurDirArray;                // list of current directories
  HWND hwndErrMsg;                     // HWND to be used for error messages

  USHORT  usReport;                    // Counting report options
  USHORT  usOptions;                   //       -"-
  CHAR    szProfile[40];               // Profile name for remaining options
  CHAR    szFormat[40];                // report output format 
} DDECNTRPT, *PDDECNTRPT;


/**********************************************************************/
/* structure which holds info for the task "analysis"                 */
/**********************************************************************/
typedef struct _DDEANALYSIS
{
  HWND   hwndOwner;                    // handle of instance window
  CHAR   szFolder[MAX_LONGFILESPEC];   // folder name
  USHORT usFileNums;                   // number of files in filelist
  PSZ    *ppFileArray;                 // ptr to file array
  USHORT usMemNums;                    // number of memories filelist
  PSZ    *ppMemArray;                  // ptr to memory array
  BOOL   fOverWrite;                   // TRUE = overwrite any existing file
  BOOL   fAddToMem;                    // TRUE = add segs found to folder TM
  BOOL   fReplExactMatchs;             // TRUE = replace exaxt matches
  BOOL   fUNSEG;                       // TRUE = create UNSEG file
  BOOL   fUseLatestMatch;              // TRUE = use latest match if more than one
  BOOL   fAutoJoin;                    // TRUE = test for joined segments option
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                // return structure
  CHAR chListName[MAX_PATH144+MAX_FILESPEC];  // list filename specified
  PSZ   *ppCurDirArray;                // list of current directories
  BOOL   fExactContextTMMatch;         // TRUE = ExactContextTMMatch selected
  BOOL   fTMMatch;                     // TRUE = collect TM match info
  HWND hwndErrMsg;                         // HWND to be used for error messages
  BOOL   fRedundCount;                 // TRUE = prepare redundancy report
  BOOL   fLeadingWS;                   // TRUE= adjust leading WS
  BOOL   fTrailingWS;                  // TRUE= adjust trailing WS
  BOOL   fRespectCRLF;                 // TRUE = respect CRLF in segment
} DDEANALYSIS, *PDDEANALYSIS;

/**********************************************************************/
/* structure which holds info for the task "translation memory create"*/
/**********************************************************************/
typedef struct _DDEMEMCRT
{
  HWND hwndOwner;                          // handle of instance window
  CHAR szName[MAX_LONGFILESPEC];           // TM name
  CHAR szDescr[MAX_MEM_DESCRIPTION];       // TM description
  CHAR chToDrive;                          // target drive of folder
  BOOL fShared;                            // TRUE = shared TM
  CHAR szFormat[MAX_FILESPEC];             // name of markup/format table
  LANGUAGE     szSourceLang;               // source language
  LANGUAGE     szTargetLang;               // target language
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                    // return structure
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DDEMEMCRT, *PDDEMEMCRT;

/**********************************************************************/
/* structure which holds info for the task "translation memory export"*/
/**********************************************************************/
typedef struct _DDEMEMEXP
{
  HWND    hwndOwner;                   // handle of instance window
  CHAR    szName[MAX_LONGFILESPEC];    // TM name
  CHAR    szOutFile[CCHMAXPATH+1];     // output file name
  CHAR    szCurDir[CCHMAXPATH+1];      // current/active directory
  BOOL    fOverWrite;                  // TRUE = overwrite existing files
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                    // return structure
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DDEMEMEXP, *PDDEMEMEXP;

/**********************************************************************/
/* structure which holds info for the task "translation memory import"*/
/**********************************************************************/
typedef struct _DDEMEMIMP
{
  HWND    hwndOwner;                   // handle of instance window
  CHAR    szName[MAX_LONGFILESPEC];    // TM name
  CHAR    szInFile[CCHMAXPATH+1];      // input file name
  CHAR    szCurDir[CCHMAXPATH+1];      // current/active directory
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                    // return structure
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DDEMEMIMP, *PDDEMEMIMP;

/**********************************************************************/
/* structure which holds info for the task "document open"            */
/**********************************************************************/
typedef struct _DDEDOCOPEN
{
  HWND hwndOwner;                          // handle of instance window
  CHAR szFolder[MAX_LONGFILESPEC];         // folder name
  CHAR szName[MAX_LONGFILESPEC];           // document name
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                    // return structure
  HWND hwndErrMsg;                         // HWND to be used for error messages
  LONG  lSegNum;                           // segment number to position to
} DDEDOCOPEN, *PDDEDOCOPEN;

/**********************************************************************/
/* structure which holds info for the task "translation memory delete"*/
/**********************************************************************/
typedef struct _DDEMEMDEL
{
  HWND    hwndOwner;                   // handle of instance window
  CHAR    szName[MAX_LONGFILESPEC];    // TM name
  CHAR    szCurDir[CCHMAXPATH+1];      // current/active directory
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                    // return structure
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DDEMEMDEL, *PDDEMEMDEL;

/************************************************************************/
/* structure which holds info for the task "translation memory organize"*/
/************************************************************************/
typedef struct _DDEMEMORG
{
  HWND    hwndOwner;                   // handle of instance window
  CHAR    szName[MAX_LONGFILESPEC];    // TM name
  CHAR    szCurDir[CCHMAXPATH+1];      // current/active directory
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                    // return structure
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DDEMEMORG, *PDDEMEMORG;

/**********************************************************************/
/* structure which holds info for the task "dictionary export"        */
/**********************************************************************/
typedef struct _DDEDICEXP
{
  HWND        hwndOwner;               // handle of instance window
  CHAR        szName[MAX_LONGFILESPEC];// dictionary name
  CHAR        szOutFile[CCHMAXPATH+1]; // output file name
  PSZ         *ppCurDirArray;          // list of current directories
  BOOL        fOverWrite;              // TRUE = overwrite existing files
  USHORT      usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN   DDEReturn;               // return structure
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DDEDICEXP, *PDDEDICEXP;

/**********************************************************************/
/* structure which holds info for the task "document delete"          */
/**********************************************************************/
typedef struct _DDEDOCDEL
{
  HWND        hwndOwner;               // handle of instance window
  CHAR        szFldName[MAX_LONGFILESPEC]; // foldername
  CHAR        szCurDir[ MAX_PATH144 ]; // current drive with directory
  CHAR        szListName[MAX_FULLPATH];// list filename specified
  BOOL        fOverwrite;              // overwrite existing files
  USHORT      usActFile;               // active file to be worked on
  USHORT      usFileNums;              // number of files in filelist
  PSZ         *ppFileArray;            // ptr to file array
  USHORT      usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN   DDEReturn;               // return structure
  PSZ         *ppCurDirArray;          // list of current directories
  CHAR        szName[MAX_LONGFILESPEC];// buffer for document name
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DDEDOCDEL, *PDDEDOCDEL;

/**********************************************************************/
/* structure which holds info for the task "folder delete"            */
/**********************************************************************/
typedef struct _DDEFOLDEL
{
  HWND        hwndOwner;               // handle of instance window
  CHAR        szFolder[MAX_LONGFILESPEC];   // folder name
  OBJNAME     szObjName;               // buffer for folder object name
  USHORT      usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN   DDEReturn;               // return structure
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DDEFOLDEL, *PDDEFOLDEL;


/**********************************************************************/
/* structure which holds info for the task "archive TM"               */
/**********************************************************************/
typedef struct _DDEARCHTM
{
  HWND   hwndOwner;                    // handle of instance window
  CHAR   szFolder[MAX_LONGFILESPEC];   // folder name
  CHAR   szMemName[MAX_LONGFILESPEC];  // TM name
  USHORT usFileNums;                   // number of files in filelist
  PSZ    *ppFileArray;                 // ptr to file array
  CHAR chListName[MAX_FULLPATH];       // list filename specified
  PSZ   *ppCurDirArray;                // list of current directories
  BOOL   fOverWrite;                   // TRUE = overwrite/delete output TM
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                // return structure
  HWND hwndErrMsg;                     // HWND to be used for error messages
} DDEARCHTM, *PDDEARCHTM;

/**********************************************************************/
/* structure which holds info for the task "BATCHIMPORT_TASK"         */
/**********************************************************************/
typedef struct _BATCHIMPORTPARMS
{
  PSZ         pszObject;               // name of object being imported
  HWND        hwndErrMsg;              // handle of batch object window
} BATCHIMPORTPARMS, *PBATCHIMPORTPARMS;

/**********************************************************************/
/* function prototypes for DDE handler                                */
/**********************************************************************/

HGLOBAL MakeDDEDataSeg(USHORT, PSZ, PVOID, USHORT, USHORT );

static BOOL EQFFunctionCmdLine (PDDECLIENT );
BOOL ListOfFiles ( PDDECLIENT, PSZ **, PSZ , PSZ **);
static BATCHCMD ValidateToken ( PSZ  *ppToken, PBATCHLIST  pBatchList );
static BOOL CheckBatchOccurrence (PDDECLIENT, PUSHORT);
static DDETASK ValidateTask ( PDDECLIENT pDDEClient );

BOOL ResolveMultFileNames(PSZ, PSZ, PSZ *); // resolve single/multiple subst.
BOOL ResolveMultFileNamesEx(PSZ, PSZ, PSZ *, BOOL ); // resolve single/multiple subst.
BOOL ResolveLongMultFileNames(PSZ, PSZ, PSZ *, ULONG ); // resolve single/multiple
BOOL DDEDocLoadWork ( PDOCIMPEXP,PSZ); // do the dirty work
BOOL DDEDocUnLoadWork(PDOCIMPEXP, PSZ, PSZ, PSZ ); // do the dirty work


MRESULT APIENTRY DDEINSTWNDPROC( HWND, WINMSG, WPARAM, LPARAM );

/**********************************************************************/
/* list of functions; one for each task which is implemented          */
/**********************************************************************/
BOOL  EQFNot(PDDECLIENT);                  // all not implemented funcs
BOOL  EQFDocImp(PDDECLIENT);               // document import
BOOL  EQFDocExp(PDDECLIENT);               // document export
BOOL  EQFDicImp(PDDECLIENT);               // dictionary import
BOOL  DDEFolderCreateParms(PDDECLIENT);    // prepare folder create parms
BOOL  DDEFolderImportParms(PDDECLIENT);    // prepare folder import parms
BOOL  DDEFolderExportParms(PDDECLIENT);    // prepare folder export parms
BOOL  DDEWordCountParms(PDDECLIENT);       // prepare wordcount parms
BOOL  DDECountReportParms(PDDECLIENT);     // prepare wordcount parms
BOOL  DDEAnalysisParms(PDDECLIENT);        // prepare analysis parms
BOOL  DDEMemCrtParms(PDDECLIENT);          // prepare memory create parms
BOOL  DDEMemExpParms(PDDECLIENT);          // prepare memory export parms
BOOL  DDEMemImpParms(PDDECLIENT);          // prepare memory import parms
BOOL  DDEDocOpenParms(PDDECLIENT);         // prepare document open
BOOL  DDEDocLoad(PDOCIMPEXP);
BOOL  DDEDocUnLoad(PDOCIMPEXP);
BOOL  DDEMemDelParms(PDDECLIENT);          // prepare memory export parms
BOOL  DDEMemOrgParms(PDDECLIENT);          // prepare memory organize parms

BOOL  DDEFolDelParms(PDDECLIENT);          // prepare folder delete parms
BOOL  DDEDocDelParms(PDDECLIENT);          // prepare document delete parms
BOOL  DDEDicExpParms(PDDECLIENT);          // prepare dictionary export parms

BOOL  DDEArchTMParms(PDDECLIENT);          // prepare archive TM function

USHORT TABatchArchTM(HWND hwndParent, PDDEARCHTM pArchTM);

#define EQFDDE_INCLUDED
#endif
