//+----------------------------------------------------------------------------+
//| BatchUtil.C                                                                |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 2012-2017, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Batch interface using the TM API                              |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//

#include <time.h>
#include "EQF.H"
#include "EQFSERNO.H"
#include "OTMFUNC.H"

#define MAXBATCH  20                        // max no of batches in 1 task
#define MAXMAND   10                        // max no of mandatory batches
#define MAX_DDE_FILES   2000                // max number of files in a list

#define LISTINDICATOR '@'
#define LISTSTART     '('                   // indicator for list start
#define LISTEND       ')'                   // indicator for list end
#define YES_STRING    "YES"                 // the yes string
#define NO_STRING     "NO"                  // the no  string

CHAR szTaskList[] = "/TASKLIST=";           // TASKLIST parameter
CHAR szShortTaskList[] = "/TAS=";           // short version of TASKLIST parameter
CHAR szQuiet[] = "/QUIET";                  // quiet parameter
CHAR szQuietNomsg[] = "/QUIET=NOMSG";       // quiet parameter/ nomsg mode
int  iQuietNomsgLen = 12;                   // length of quiet parameter/ nomsg mode
CHAR szNoErrorStop[] = "/NOERRORSTOP";      // no error stop parameter
CHAR szShortNoErrorStop[] = "/NOE";         // no error stop parameter (short version)

#define QUIET_NOMSG 2      

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
  BATCH_ROMEM,   
  BATCH_SHIPMENT,
  BATCH_TASKLIST,
  BATCH_VERSION,
  BATCH_TMAIL,
  BATCH_TRANSLATOR, 
  BATCH_VAL,
  BATCH_FORMAT,
  BATCH_ADJUST,
  BATCH_NEW,
  BATCH_SEGMENT,
  BATCH_LINE,
  BATCH_SEARCH,
  BATCH_TRACKID,                       // Doc open: TVT track ID
  BATCH_IMPORTAS,                      // folder import: import-as parameter
  BATCH_END                            // end of list indicator
} BATCHCMD;

typedef enum _BATCHTASK
{
  TASK_FLDCRT,                         // create a folder
  TASK_FLDEXP,                         // export a folder
  TASK_FLDIMP,                         // import a folder
  TASK_WORDCNT,                        // wordcount a folder
  TASK_DICIMP,                         // dictionary import
  TASK_DICEXP,                         // dictionary export
  TASK_DOCIMP,                         // document import
  TASK_DOCEXP,                         // document export
  TASK_MEMIMP,                         // import translation memory
  TASK_MEMEXP,                         // export translation memory
  TASK_MEMCRT,                         // create translation memory
  TASK_ANALYSIS,                       // analyse docs or folder
  TASK_MEMDEL,                         // delete translation memory
  TASK_MEMORG,                         // organize translation memory
  TASK_FLDDEL,                         // delete folder
  TASK_DOCDEL,                         // delete document
  TASK_CNTRPT,                         // wordcount a folder
  TASK_ARCHTM,                         // build archive TM
  TASK_RENAME,                         // rename an obect 
  TASK_DOCOPE,                         // open a document
  TASK_REMOVEDOCS,                     // remove a group of documents
  TASK_RESTOREDOCS,                    // restore a group of documents
  TASK_FLDPROP,                        // change folder properties
  TASK_MARKUPCRT,                      // create a markup table (TBL)     2-19-14
  TASK_CONNECT,
  TASK_DISCONNECT,
  TASK_END
} BATCHTASK;

typedef struct _BATCHLIST              // list of batches
{
  BATCHCMD   BatchCmd;
} BATCHLIST, *PBATCHLIST;

typedef struct _BATCHPARAMETER
{
  LONG        lOptions;                          // options to be used for the function
  CHAR        szName[MAX_LONGFILESPEC];          // buffer for folder/memory/dictionary name
  CHAR        szDescr[MAX_DESCRIPTION];          // description
  CHAR        szMarkup[MAX_FNAME];               // markup table name
  CHAR        szEdit[MAX_FNAME];                 // editor name
  CHAR        szSourceLang[MAX_LANG_LENGTH];     // source language 
  CHAR        szTargetLang[MAX_LANG_LENGTH];     // target language
  CHAR        szConversion[MAX_DESCRIPTION];     // conversion  
  CHAR        szAlias[MAX_LONGFILESPEC];         // buffer for document name alias
  CHAR        szMemory[MAX_LONGFILESPEC];        // buffer for memory name
  CHAR        szPassword[MAX_FNAME];             // folder password
  CHAR        szPath[MAX_LONGFILESPEC];          // buffer for fully qualified file names and paths
  CHAR        szShipment[MAX_LONGFILESPEC];      // buffer for shipment
  CHAR        szVersion[MAX_LONGFILESPEC];       // buffer for version
  CHAR        szTranslator[MAX_LONGFILESPEC];    // buffer for translator name
  CHAR        szTranslatorMail[MAX_LONGFILESPEC];// buffer for translator email address
  CHAR        szProfile[MAX_LONGFILESPEC];       // buffer for profile name
  CHAR        szTaskList[MAX_LONGFILESPEC];      // buffer for task list name
  CHAR        szImportAs[MAX_LONGFILESPEC];      // buffer for import as folder name
  PSZ         pszDicList;                        // list of dictionary names
  PSZ         pszDocList;                        // list of document names
  PSZ         pszMemList;                        // list of memory names
  CHAR        chToDrive;                         // target drive
  CHAR        chFromDrive;                       // source drive
  LONG        lAddOptions1;                      // additional options 1
  LONG        lAddOptions2;                      // additional options 2
  LONG        lAddOptions3;                      // additional options 3
  BOOL        aParmUsed[BATCH_END];              // flag array for specified parameters
  BOOL        fSwitch1;                          // switch 1 value
  BOOL        fSwitch2;                          // switch 2 value
  BOOL        fSwitch3;                          // switch 3 value
  BOOL        fDescription;                      // description-has-been-specified flag
  BOOL        fMemory;                           // memory-has-been-specified flag
  BOOL        fDicList;                          // dictionary-list-has-been-specified flag
  BOOL        fTargetLang;                       // target-language-has-been-specified flag
  BOOL        fMemList;                          // memory-list-has-been-specified flag
  BOOL        fShipment;                         // shipment-number-has-been-specified flag
  BOOL        fProfile;                          // profile-name-has-been-specified flag
} BATCHPARAMETER, *PBATCHPARAMETER;

typedef struct _BATCHDATA
{
  CHAR        szCmdLine[2048];                   // buffer for command line
  CHAR        szOrgCmdLine[2048];                // buffer for original command line
  USHORT      usRC;                              // function return code
  HWND        hwndErrMsg;                        // window handle for error messages
  PBATCHLIST  pBatchList;                        // Batchlist of this task
  PBATCHLIST  pMandList;                         // list of mandatory batches of task
  BATCHTASK   Task;                              // task of this instance
  USHORT      usQuietMode;                       // processs quiet mode (TRUE / FALSE / QUIET_NOMSG)
  BOOL        fQuietprocessed;                   // TRUE = quiet option has been processed
  HSESSION    hSession;                          // API session handle
  CHAR        szMsgBuffer[8096];                 // buffer for error messages
  CHAR        szTaskList[MAX_LONGFILESPEC];      // buffer for task list file name
  CHAR        szPathBuffer[1024];                // buffer for path names
  CHAR_W      szUTF16Buffer[MAX_LONGFILESPEC];   // buffer for UTF16 strings
} BATCHDATA, *PBATCHDATA;

typedef struct _TASKLIST
{
  BATCHTASK TaskCmd;                   // task identifier
  CHAR szDesc[15];                     // description of item
  USHORT (*function)( PBATCHDATA );    // function performing the task
} TASKLIST, *PTASKLIST;

typedef struct _CMDLIST
{
  BATCHCMD BatchCmd;                   // identifier
  CHAR    szDesc[ 25 ];                // full name of item
  CHAR    szShortCut[ 25 ];            // shortcut of item
} CMDLIST, *PCMDLIST;

typedef enum _BATCHPARMTYPE
{
  OPTION_PARMTYPE,                               // check agaist list of options, place result in Parm.lOption 
  ADDOPTION_PARMTYPE,                            // check agaist list of options, place result in Parm.lAddOption1/2/3 
  NAME_PARMTYPE,                                 // get name, store result in pvValue
  NAMELIST_PARMTYPE,                             // get list of names, store result in pvValue 
  LETTER_PARMTYPE,                               // get single character/drive, store result in pvValue
  OVERWRITE_PARMTYPE,                            // get overwrite flag, store result in Parm.lOption 
  SWITCH_PARMTYPE,                               // get single switch 
  LONG_PARMTYPE,                                 // get long number
  DUMMY_PARMTYPE                                 // marks end of list 
} BATCHPARMTYPE;

typedef struct _BATCHPARM
{
  BATCHCMD      Cmd;                             // Batch command; e.g. /FLD=
  BATCHPARMTYPE Type;                            // type of parameter processing
  PVOID         pvValue;                         // value/target: for NAME/LETTER,NAMELIST: target for value
                                                 //               for OPTION/ADDOPTION: list of valid options
  LONG          lMaxLength;                      // max length: for NAME/LETTER: max number of characters
                                                 //             for ADDOPTION: number of option field to use as target
                                                 //             for NAMELIST: 1 = allow empty lists
  PBOOL         pfUsed;                          // pointer to "parameter has been used" flag or NULL if not required
} BATCHPARM, *PBATCHPARM;

typedef struct _BATCHOPTION
{
  CHAR     szName[20];                // option name
  LONG     lValue;                    // value for option
} BATCHOPTION, *PBATCHOPTION;

// prototypes
int main( int argc, char *argv[], char *envp[] );
USHORT BatchNotImplemented( PBATCHDATA pData );
USHORT BatchAnalysis( PBATCHDATA pData );
USHORT BatchCreateFolder( PBATCHDATA pData );
USHORT BatchRename( PBATCHDATA pData );
USHORT BatchExportDoc( PBATCHDATA pData );  
USHORT BatchImportDoc( PBATCHDATA pData );
USHORT BatchExportFolder( PBATCHDATA pData );
USHORT BatchImportFolder( PBATCHDATA pData );
USHORT BatchDeleteFolder( PBATCHDATA pData );
USHORT BatchCountWords( PBATCHDATA pData );
USHORT BatchDeleteDoc( PBATCHDATA pData );
USHORT BatchExportMem( PBATCHDATA pData );
USHORT BatchDeleteMem( PBATCHDATA pData );
USHORT BatchCreateMem( PBATCHDATA pData );
USHORT BatchImportMem( PBATCHDATA pData );
USHORT BatchOrganizeMem( PBATCHDATA pData );
USHORT BatchExportDict( PBATCHDATA pData );
USHORT BatchImportDict( PBATCHDATA pData );
USHORT BatchArchiveMem( PBATCHDATA pData );
USHORT BatchCountReport( PBATCHDATA pData );
USHORT BatchOpenDoc( PBATCHDATA pData );
USHORT BatchCheckCommand( PBATCHDATA pData );
USHORT BatchOverwriteParm( PBATCHDATA pData, PSZ pStart);
USHORT BatchLongParm( PBATCHDATA pData, PSZ pStart, PLONG plValue, BATCHCMD cmd ); 
USHORT BatchStringParm( PBATCHDATA pData, PSZ pStart, PSZ pszName, int iMaxLen, BATCHCMD cmd  );
USHORT BatchDriveParm( PBATCHDATA pData, PSZ pStart, CHAR *pchDrive, BATCHCMD cmd );
USHORT BatchOptions( PBATCHDATA pData, PSZ pStart, PBATCHOPTION pOptions, PLONG plOption, BATCHCMD cmd );
USHORT BatchNameList( PBATCHDATA pData, PSZ pStart, PSZ *ppList, BATCHCMD cmd, LONG lAllowEmptyLists );
void   BatchHandleAPIError( PBATCHDATA pData, USHORT usRC );
VOID   BatchBothStripDuplSlash( PBATCHDATA, PSZ, PSZ);
BATCHTASK BatchValidateTask( PBATCHDATA pData );
BATCHCMD BatchValidateToken( PSZ *ppToken, PBATCHPARM pParmInfo  );
USHORT BatchGetParameters( PBATCHDATA pData, PBATCHPARM pParmInfo );
USHORT BatchCleanParms( PBATCHDATA pData );
USHORT BatchCheckMandParms( PBATCHDATA pData, BATCHCMD *pMandParms );
VOID   BatchMakeTaskList( PSZ pszBuffer );
VOID   BatchMakeOptionList( PBATCHOPTION pOption, PSZ pszBuffer );
VOID   BatchMakeParmList( PBATCHPARM pParm, PSZ pszBuffer );
USHORT BatchGetQuietMode( PBATCHDATA pData );
USHORT BatchRemoveDocs( PBATCHDATA pData );
USHORT BatchRestoreDocs( PBATCHDATA pData );
USHORT BatchChangeFolProps( PBATCHDATA pData );
USHORT BatchCreateMarkup( PBATCHDATA pData );
USHORT BatchConnectSharedMem(PBATCHDATA pData);
USHORT BatchDisconnectSharedMem(PBATCHDATA pData);

// our function parameter structure
static BATCHPARAMETER Parm = { 0 };


static CMDLIST EQFCmdList[ BATCH_END + 1 ] =
{
  {BATCH_TASK,         "/TASK=",        "/TA="     },     // task
  {BATCH_FLD,          "/FLD=",         "/FLD="    },     // folder name
  {BATCH_FILES,        "/FILES=",       "/FI="     },     // input file or list of files
  {BATCH_OUT,          "/OUT=",         "/OUT="    },     // name of output file
  {BATCH_TYPE,         "/TYPE=",        "/TY="     },     // type of selection
  {BATCH_MARKUP,       "/MARKUP=",      "/MA="     },     // markup language
  {BATCH_FROMDRIVE,    "/FROMDRIVE=",   "/FR="     },     // from which drive
  {BATCH_TODRIVE,      "/TODRIVE=",     "/TO="     },     // to which drive
  {BATCH_FROMPATH,     "/FROMPATH=",    "/FP="     },     // from which path
  {BATCH_TOPATH,       "/TOPATH=",      "/TP="     },     // to which path
  {BATCH_EDIT,         "/EDIT=",        "/ED="   },       // name of editor
  {BATCH_SRCLNG,       "/SRCLNG=",      "/SR=" },         // source language
  {BATCH_TGTLNG,       "/TGTLNG=",      "/TG=" },         // target language
  {BATCH_DESC,         "/DESC=",        "/DE="   },       // description of a file
  {BATCH_NAME,         "/NAME=",        "/NA="   },       // name of a file
  {BATCH_MEM,          "/MEM=",         "/ME="    },      // translation memory
  {BATCH_DICT,         "/DICT=",        "/DI="   },       // name of dictionary
  {BATCH_FILT,         "/FILT=",        "/FILT="   },     // name of filter
  {BATCH_OPTIONS,      "/OPTIONS=",     "/OP=" },         // options
  {BATCH_EXCLLIST,     "/EXCLLIST=",    "/EXCLL="  },     // exclusion list
  {BATCH_EXCLDICT,     "/EXCLDICT=",    "/EXCLD="  },     // exclusion dictionary
  {BATCH_OVERWRITE,    "/OVERWRITE=",   "/OV=" },         // overwrite existing files
  {BATCH_SGMLMEM,      "/SGMLMEM=",     "/SG=" },         // name of the SGML output file
  {BATCH_NOANA,        "/NOANA=",       "/NOA="   },      // NO analysis
  {BATCH_NOTMDB,       "/NOTMDB=",      "/NOT=" },        // NO translation memory database
  {BATCH_NOCONF,       "/NOCONF=",      "/NOC=" },        // NO confirmation message at end
  {BATCH_LEVEL ,       "/LEVEL=",       "/LE="  },        // level of match
  {BATCH_CURDIR,       "/$CD=",         "/$CD=", },       // current drive and directory
  {BATCH_PASSWORD,     "/PASSWORD=",    "/PA=" },         // password for dictionary
  {BATCH_ALIAS,        "/ALIAS=",       "/AL=" },         // alias
  {BATCH_REPORT,       "/REPORT=",      "/RE=" },         // Counting Report
  {BATCH_PROFILE,      "/PROFILE=",     "/PR=" },         // Counting Profile
  {BATCH_STARTPATH,    "/STARTPATH=",   "/ST=" },         // Start path
  {BATCH_CONVERSION,   "/CONV=",        "/CO=" },         // Conversion
  {BATCH_ROMEM,        "/ROMEM=",       "/RO=" },         // Read only memories
  {BATCH_SHIPMENT,     "/SHIPMENT=",    "/SH=" },         // shipment
  {BATCH_TASKLIST,     "/TASKLIST=",    "/TAS=" },        // task list
  {BATCH_VERSION,      "/VERSION=",     "/VE=" },         // version  
  {BATCH_TMAIL,        "/TMAIL=",       "/TM=" },         // translators mail address
  {BATCH_TRANSLATOR,   "/TRANSLATOR=",  "/TR=" },         // translator
  {BATCH_VAL,          "/VAL=",         "/VA=" },         // validation document options
  {BATCH_FORMAT,       "/FORMAT=",      "/FO=" },         // format for counting report
  {BATCH_ADJUST,       "/ADJUST=",      "/ADJ=" },        // adjust references
  {BATCH_NEW,          "/NEW=",         "/NEW=" },        // new name
  {BATCH_SEGMENT,      "/SEGMENT=",     "/SE=" },         // segment number
  {BATCH_LINE,         "/LINE=",        "/LI=" },         // line number
  {BATCH_SEARCH,       "/SEARCH=",      "/SE=" },         // search string
  {BATCH_TRACKID,      "/TRACK=",       "/TRK=" },        // TVT track ID 
  {BATCH_IMPORTAS,     "/IMPORTAS=",    "/AS=" },         // import as folder name
  {BATCH_END,          "",              ""       }        // end of list indicator
                } ;

static TASKLIST EQFTaskList[TASK_END+1] =
{
 {TASK_FLDCRT,     "FLDCRT",  BatchCreateFolder },         //create a folder
 {TASK_FLDEXP,     "FLDEXP",  BatchExportFolder },         //export a folder
 {TASK_FLDIMP,     "FLDIMP",  BatchImportFolder },         //import a folder
 {TASK_WORDCNT,    "WORDCNT", BatchCountWords },           //wordcount a folder
 {TASK_DICIMP,     "DICIMP",  BatchImportDict },           //dictionary import
 {TASK_DICEXP,     "DICEXP",  BatchExportDict },
 {TASK_DOCIMP,     "DOCIMP",  BatchImportDoc },            //document import
 {TASK_DOCEXP,     "DOCEXP",  BatchExportDoc },            //document export
 {TASK_MEMIMP,     "MEMIMP",  BatchImportMem },            //import translation memory
 {TASK_MEMEXP,     "MEMEXP",  BatchExportMem },            //export translation memory
 {TASK_MEMCRT,     "MEMCRT",  BatchCreateMem },            //create translation memory
 {TASK_ANALYSIS,   "ANALYSIS",BatchAnalysis },             //analyse docs or folder
 {TASK_MEMDEL,     "MEMDEL",  BatchDeleteMem },            //delete translation memory
 {TASK_MEMORG,     "MEMORG",  BatchOrganizeMem },          // organize memory
 {TASK_FLDDEL,     "FLDDEL",  BatchDeleteFolder },
 {TASK_DOCDEL,     "DOCDEL",  BatchDeleteDoc },
 {TASK_CNTRPT,     "CNTRPT",  BatchCountReport },          //counting report
 {TASK_ARCHTM,     "ARCHTM",  BatchArchiveMem },
 {TASK_RENAME,     "RENAME",  BatchRename },
 {TASK_DOCOPE,     "DOCOPEN", BatchOpenDoc },
 {TASK_REMOVEDOCS, "REMOVEDOCS", BatchRemoveDocs },
 {TASK_RESTOREDOCS,"RESTOREDOCS", BatchRestoreDocs },
 {TASK_FLDPROP,    "FLDPROP",   BatchChangeFolProps },         
 {TASK_MARKUPCRT,  "MARKUPCRT", BatchCreateMarkup },         //Create markup table (TBL)  2-19-14
 {TASK_CONNECT,    "CONNECT",   BatchConnectSharedMem},
 {TASK_DISCONNECT, "DISCONNECT", BatchDisconnectSharedMem},
 {TASK_END,        "",        BatchNotImplemented}
} ;


// option lists

BATCHOPTION AnalysisOptions[] =
{                       
  { "TMMATCH",       TMMATCH_OPT },
  { "ADDTOMEM",      ADDTOMEM_OPT },
  { "AUTO",          AUTOSUBST_OPT },
  { "UNTRANSLATED",  UNTRANSLATED_OPT },
  { "AUTOLAST",      AUTOLAST_OPT },
  { "AUTOJOIN",      AUTOJOIN_OPT },
  { "AUTOCONTEXT",   AUTOCONTEXT_OPT },
  { "REDUNDCOUNT",   REDUNDCOUNT_OPT },
  { "ADJUSTLEADWS",  ADJUSTLEADWS_OPT },
  { "ADJUSTTRAILWS", ADJUSTTRAILWS_OPT },
  { "RESPECTCRLF",   RESPECTCRLF_OPT },
  { "IGNOREPATH",    IGNOREPATH_OPT },
  { "STOPATFIRSTEXACT", STOPATFIRSTEXACT_OPT },
  { "PROTECTXMPSCREEN", PROTECTXMPSCREEN_OPT },
  { "IGNORECOMMENTED", IGNORECOMMENTED_OPT },
  { "SENDTOMT",       SENDTOMT_OPT },
  { "PROTECTXMP",     PROTECTXMP_OPT },
  { "PROTECTMSGNUM",  PROTECTMSGNUM_OPT },
  { "PROTECTMETA",    PROTECTMETA_OPT }, 
  { "PROTECTSCREEN",  PROTECTSCREEN_OPT },
  { "PROTECTCODEBLOCK",PROTECTCODEBLOCK_OPT },
  { "", 0 } };
  
BATCHOPTION DocExportOptions[] =
{                       
  { "TARGET",              TARGET_OPT},
  { "SOURCE",              SOURCE_OPT },
  { "SNOMATCH",            SNOMATCH_OPT },
  { "VALDOC",              VALFORMAT_XML_OPT },
  { "PLAINXML",            PLAINXML_OPT },
  { "WITHRELATIVEPATH",    WITHRELATIVEPATH_OPT },
  { "WITHOUTRELATIVEPATH", WITHOUTRELATIVEPATH_OPT },
  { "OPENTM2FORMAT",       OPENTM2FORMAT_OPT },
  { "WITHTRACKID",         WITHTRACKID_OPT },
  { "", 0 } };

BATCHOPTION DocExportValOptions[] =
{                       
  { "XML",           VALFORMAT_XML_OPT },
  { "HTML",          VALFORMAT_HTML_OPT },
  { "DOC",           VALFORMAT_DOC_OPT },
  { "DOCX",          VALFORMAT_DOCX_OPT },
  { "ODT",           VALFORMAT_ODT_OPT },
  { "COMBINE",       VALFORMAT_COMBINE_OPT },
  { "PROTSEGS",      VALFORMAT_PROTSEGS_OPT },
  { "", 0 } };

BATCHOPTION WordCountOptions[] =
{                       
  { "TARGET",        TARGET_OPT},
  { "SOURCE",        SOURCE_OPT },
  { "TMMATCH",       TMMATCH_OPT },
  { "DUPLICATE",     DUPLICATE_OPT },
  { "DUPMEMMATCH",   DUPMEMMATCH_OPT },
  { "SEPREPLMATCH",  SEPERATEREPLMATCH_OPT },
  { "FUZZYMATCH",    FUZZYMATCH_OPT },
  { "", 0 } };

BATCHOPTION CountReportReportOptions[] =
{                       
  { "HISTORY",        HISTORY_REP },
  { "COUNTING",       COUNTING_REP },
  { "CALCULATING",    CALCULATING_REP },
  { "PREANALYSIS",    PREANALYSIS_REP },
  { "REDUNDANCY",     REDUNDANCY_REP },
  { "REDUNDANCYSEGMENT", REDUNDANCYSEGMENT_REP },
  { "", 0 } };

BATCHOPTION CountReportTypeOptions[] =
{                       
  { "DATE",           BRIEF_SORTBYDATE_REPTYPE },
  { "BRIEF",          BRIEF_SORTBYDOC_REPTYPE },
  { "DETAIL",         DETAIL_REPTYPE },
  { "WITH_TOTALS",    WITHTOTALS_REPTYPE },
  { "WITHOUT_TOTALS", WITHOUTTOTALS_REPTYPE },
  { "BASE",           BASE_REPTYPE },
  { "BASE_SUMMARY",   BASE_SUMMARY_REPTYPE },
  { "BASE_SUMMARY_FACT", BASE_SUMMARY_FACTSHEET_REPTYPE },
  { "SUMMARY_FACT",   SUMMARY_FACTSHEET_REPTYPE },
  { "FACT",           FACTSHEET_REPTYPE },
  { "", 0 } };

BATCHOPTION CountReportFormatOptions[] =
{                       
  { "ASCII",        TEXT_OUTPUT_OPT },
  { "XML",          XML_OUTPUT_OPT },
  { "HTML",         HTML_OUTPUT_OPT },
  { "", 0 } };

BATCHOPTION FolderImportOptions[] =
{       
  { "DICT",        WITHDICT_OPT },
  { "MEM",         WITHMEM_OPT },
  { "XLIFF",       XLIFF_OPT },
  { "", 0 } };

BATCHOPTION FolderExportOptions[] =
{       
  { "DICT",        WITHDICT_OPT },
  { "MEM",         WITHMEM_OPT },
  { "ROMEM",       WITHREADONLYMEM_OPT },
  { "DOCMEM",      WITHDOCMEM_OPT },
  { "DELETE",      DELETE_OPT },
  { "MASTERFOLDER",MASTERFOLDER_OPT },
  { "XLIFF",       XLIFF_OPT },
  { "NOREDUND",    WO_REDUNDANCY_DATA_OPT },
  { "", 0 } };

BATCHOPTION MemoryCreateOptions[] =
{                       
  { "LOCAL",       LOCAL_OPT },
  { "SHARED",      SHARED_OPT },
  { "", 0 } };

BATCHOPTION MemoryImportOptions[] =
{                       
  { "ASCII",        ASCII_OPT},
  { "ANSI",         ANSI_OPT},
  { "UTF16",        UTF16_OPT},
  { "TMX",          TMX_OPT},
  { "TMXTRADOS",    TMX_OPT | CLEANRTF_OPT},
  { "CANCELBADMARKUP",  CANCEL_UNKNOWN_MARKUP_OPT},
  { "SKIPBADMARKUP",    SKIP_UNKNOWN_MARKUP_OPT},
  { "GENERICBADMARKUP", GENERIC_UNKNOWN_MARKUP_OPT},
  { "", 0 } };

BATCHOPTION MemoryExportOptions[] =
{                       
  { "ASCII",        ASCII_OPT},
  { "ANSI",         ANSI_OPT},
  { "UTF16",        UTF16_OPT},
  { "TMXUTF16",     TMX_UTF16_OPT},
  { "TMXUTF8",      TMX_UTF8_OPT},
  { "", 0 } };

BATCHOPTION DictionaryImportOptions[] =
{                       
  { "ASCII",        ASCII_OPT},
  { "ANSI",         ANSI_OPT},
  { "UTF16",        UTF16_OPT},
  { "IGNORE",       IGNORE_OPT},
  { "REPLACE",      REPLACE_OPT},
  { "COMBINE",      COMBINE_OPT},
  { "DXTUTF8",      DXT_UTF8_OPT},
  { "", 0 } };

BATCHOPTION DictionaryExportOptions[] =
{                       
  { "ASCII",        ASCII_OPT},
  { "ANSI",         ANSI_OPT},
  { "UTF16",        UTF16_OPT},
  { "DXTUTF8",      DXT_UTF8_OPT},
  { "", 0 } };

BATCHOPTION ArchiveMemOptions[] =
{                       
  { "USEASFOLDERTM",   USEASFOLDERTM_OPT },
  { "SOURCESOURCEMEM", SOURCESOURCEMEM_OPT },
  { "SETMFLAG",        SETMFLAG_OPT },
  { "", 0 } };

  // parameter handling lists
BATCHPARM FolderCreateParms[] =
{
  { BATCH_NAME,       NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_DESC,       NAME_PARMTYPE,      Parm.szDescr,           MAX_DESCRIPTION, NULL },
  { BATCH_MEM,        NAMELIST_PARMTYPE,  &(Parm.pszMemList),     0, NULL },
  { BATCH_TODRIVE,    LETTER_PARMTYPE,    &(Parm.chToDrive),      1, NULL }, 
  { BATCH_MARKUP,     NAME_PARMTYPE,      Parm.szMarkup,          MAX_FNAME, NULL },
  { BATCH_EDIT,       NAME_PARMTYPE,      Parm.szEdit,            MAX_FNAME, NULL },
  { BATCH_DICT,       NAMELIST_PARMTYPE,  &(Parm.pszDicList),     0, NULL },
  { BATCH_SRCLNG,     NAME_PARMTYPE,      Parm.szSourceLang,      MAX_LANG_LENGTH, NULL }, 
  { BATCH_TGTLNG,     NAME_PARMTYPE,      Parm.szTargetLang,      MAX_LANG_LENGTH, NULL },
//  { BATCH_CONVERSION, NAME_PARMTYPE,      Parm.szConversion,      MAX_DESCRIPTION, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM AnalysisParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_FILES,      NAMELIST_PARMTYPE,  &(Parm.pszDocList),     0, NULL },
  { BATCH_OPTIONS,    OPTION_PARMTYPE,    AnalysisOptions,        0, NULL }, 
  { BATCH_MEM,        NAMELIST_PARMTYPE,  &(Parm.pszMemList),     0, NULL },
  { BATCH_PROFILE,    NAME_PARMTYPE,      Parm.szProfile,         40, NULL },
  { BATCH_OVERWRITE,  OVERWRITE_PARMTYPE, NULL,                   0, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM DocExportParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_FILES,      NAMELIST_PARMTYPE,  &(Parm.pszDocList),     0, NULL },
  { BATCH_STARTPATH,  NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_OPTIONS,    OPTION_PARMTYPE,    DocExportOptions,       0, NULL }, 
  { BATCH_VAL,        ADDOPTION_PARMTYPE, DocExportValOptions,    1, NULL }, 
  { BATCH_OVERWRITE,  OVERWRITE_PARMTYPE, NULL,                   0, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM DocImportParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_FILES,      NAMELIST_PARMTYPE,  &(Parm.pszDocList),     0, NULL },
  { BATCH_STARTPATH,  NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_ALIAS,      NAME_PARMTYPE,      Parm.szAlias,           MAX_LONGFILESPEC, NULL },
  { BATCH_OVERWRITE,  OVERWRITE_PARMTYPE, NULL,                   0, NULL },
  { BATCH_MARKUP,     NAME_PARMTYPE,      Parm.szMarkup,          MAX_FNAME, NULL },
  { BATCH_EDIT,       NAME_PARMTYPE,      Parm.szEdit,            MAX_FNAME, NULL },
  { BATCH_DICT,       NAMELIST_PARMTYPE,  &(Parm.pszDicList),     0, NULL },
//  { BATCH_CONVERSION, NAME_PARMTYPE,      Parm.szConversion,      MAX_DESCRIPTION, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM DocDeleteParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_FILES,      NAMELIST_PARMTYPE,  &(Parm.pszDocList),     0, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM FolderImportParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_FILES,      NAMELIST_PARMTYPE,  &(Parm.pszDocList),     0, NULL },
  { BATCH_FROMPATH,   NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_FROMDRIVE,  LETTER_PARMTYPE,    &(Parm.chFromDrive),    1, NULL },
  { BATCH_OPTIONS,    OPTION_PARMTYPE,    FolderImportOptions,    0, NULL }, 
  { BATCH_TODRIVE,    LETTER_PARMTYPE,    &(Parm.chToDrive),      1, NULL },
  { BATCH_EDIT,       NAME_PARMTYPE,      Parm.szEdit,            MAX_FNAME, NULL },
  { BATCH_MARKUP,     NAME_PARMTYPE,      Parm.szMarkup,          MAX_FNAME, NULL },
  { BATCH_OVERWRITE,  OVERWRITE_PARMTYPE, NULL,                   0, NULL },
//  { BATCH_CONVERSION, NAME_PARMTYPE,      Parm.szConversion,      MAX_DESCRIPTION, NULL },
  { BATCH_IMPORTAS,   NAME_PARMTYPE,      Parm.szImportAs,        MAX_LONGFILESPEC, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM FolderExportParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_FILES,      NAMELIST_PARMTYPE,  &(Parm.pszDocList),     0, NULL },
  { BATCH_TODRIVE,    LETTER_PARMTYPE,    &(Parm.chToDrive),      1, NULL },
  { BATCH_TOPATH,     NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_OPTIONS,    OPTION_PARMTYPE,    FolderExportOptions,    0, NULL }, 
  { BATCH_OVERWRITE,  OVERWRITE_PARMTYPE, NULL,                   0, NULL },
  { BATCH_DESC,       NAME_PARMTYPE,      Parm.szDescr,           MAX_DESCRIPTION, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM FolderDeleteParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM MemoryCreateParms[] =
{
  { BATCH_NAME,       NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_DESC,       NAME_PARMTYPE,      Parm.szDescr,           MAX_DESCRIPTION, NULL },
  { BATCH_TODRIVE,    LETTER_PARMTYPE,    &(Parm.chToDrive),      1, NULL }, 
  { BATCH_SRCLNG,     NAME_PARMTYPE,      Parm.szSourceLang,      MAX_LANG_LENGTH, NULL }, 
  { BATCH_TYPE,       OPTION_PARMTYPE,    MemoryCreateOptions,    0, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM MemoryDeleteParms[] =
{
  { BATCH_MEM,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM MemoryOrganizeParms[] =
{
  { BATCH_MEM,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM MemoryExportParms[] =
{
  { BATCH_MEM,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_OUT,        NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_TYPE,       OPTION_PARMTYPE,    MemoryExportOptions,    0, NULL },
  { BATCH_OVERWRITE,  OVERWRITE_PARMTYPE, NULL,                   0, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM MemoryImportParms[] =
{
  { BATCH_MEM,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_FILES,      NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_TYPE,       OPTION_PARMTYPE,    MemoryImportOptions,    0, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM WordCountParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_OUT,        NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_FILES,      NAMELIST_PARMTYPE,  &(Parm.pszDocList),     0, NULL },
  { BATCH_OVERWRITE,  OVERWRITE_PARMTYPE, NULL,                   0, NULL },
  { BATCH_OPTIONS,    OPTION_PARMTYPE,    WordCountOptions,       0, NULL },
  { BATCH_FORMAT,     ADDOPTION_PARMTYPE, CountReportFormatOptions, 3, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM CountReportParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_OUT,        NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_FILES,      NAMELIST_PARMTYPE,  &(Parm.pszDocList),     0, NULL },
  { BATCH_OVERWRITE,  OVERWRITE_PARMTYPE, NULL,                   0, NULL },
  { BATCH_REPORT,     ADDOPTION_PARMTYPE, CountReportReportOptions, 1, NULL },
  { BATCH_TYPE,       ADDOPTION_PARMTYPE, CountReportTypeOptions, 2, NULL },
  { BATCH_FORMAT,     ADDOPTION_PARMTYPE, CountReportFormatOptions, 3, NULL },
  { BATCH_PROFILE,    NAME_PARMTYPE,      Parm.szProfile,         MAX_LONGFILESPEC, NULL },
  { BATCH_SHIPMENT,   NAME_PARMTYPE,      Parm.szShipment,        MAX_LONGFILESPEC, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM TaskListParms[] =
{
  { BATCH_TASKLIST,   NAME_PARMTYPE,      Parm.szTaskList,        MAX_LONGFILESPEC, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM CreateControlledFolderParms[] =
{
  { BATCH_NAME,       NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_DESC,       NAME_PARMTYPE,      Parm.szDescr,           MAX_DESCRIPTION, NULL },
  { BATCH_MEM,        NAME_PARMTYPE,      Parm.szMemory,          MAX_LONGFILESPEC, NULL },
  { BATCH_TODRIVE,    LETTER_PARMTYPE,    &(Parm.chToDrive),      1, NULL }, 
  { BATCH_MARKUP,     NAME_PARMTYPE,      Parm.szMarkup,          MAX_FNAME, NULL },
  { BATCH_EDIT,       NAME_PARMTYPE,      Parm.szEdit,            MAX_FNAME, NULL },
  { BATCH_DICT,       NAMELIST_PARMTYPE,  &(Parm.pszDicList),     0, NULL },
  { BATCH_SRCLNG,     NAME_PARMTYPE,      Parm.szSourceLang,      MAX_LANG_LENGTH, NULL }, 
  { BATCH_TGTLNG,     NAME_PARMTYPE,      Parm.szTargetLang,      MAX_LANG_LENGTH, NULL },
  { BATCH_ROMEM,      NAMELIST_PARMTYPE,  &(Parm.pszMemList),     0, NULL },
  { BATCH_PASSWORD,   NAME_PARMTYPE,      Parm.szPassword,        MAX_FNAME, NULL },
  { BATCH_SHIPMENT,   NAME_PARMTYPE,      Parm.szShipment,        MAX_LONGFILESPEC, NULL },
  { BATCH_VERSION,    NAME_PARMTYPE,      Parm.szVersion,         MAX_LONGFILESPEC, NULL },
  { BATCH_TRANSLATOR, NAME_PARMTYPE,      Parm.szTranslator,      MAX_LONGFILESPEC, NULL },
  { BATCH_TMAIL,      NAME_PARMTYPE,      Parm.szTranslatorMail,  MAX_LONGFILESPEC, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM ArchiveMemoryParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_FILES,      NAMELIST_PARMTYPE,  &(Parm.pszDocList),     0, NULL },
  { BATCH_MEM,        NAME_PARMTYPE,      Parm.szMemory,          MAX_LONGFILESPEC, NULL },
  { BATCH_OVERWRITE,  OVERWRITE_PARMTYPE, NULL,                   0, NULL },
  { BATCH_OPTIONS,    OPTION_PARMTYPE,    ArchiveMemOptions,      0, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM DictionaryImportParms[] =
{
  { BATCH_DICT,       NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_FILES,      NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_OPTIONS,    OPTION_PARMTYPE,    DictionaryImportOptions,0, NULL },
  { BATCH_PASSWORD,   NAME_PARMTYPE,      Parm.szPassword,        MAX_FNAME, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM DictionaryExportParms[] =
{
  { BATCH_DICT,       NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_OUT,        NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_PASSWORD,   NAME_PARMTYPE,      Parm.szPassword,        MAX_FNAME, NULL },
  { BATCH_OPTIONS,    OPTION_PARMTYPE,    DictionaryExportOptions,0, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM RenameParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_MEM,        NAME_PARMTYPE,      Parm.szMemory,          MAX_LONGFILESPEC, NULL },
  { BATCH_DICT,       NAME_PARMTYPE,      Parm.szAlias,           MAX_LONGFILESPEC, NULL },
  { BATCH_NEW,        NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_ADJUST,     SWITCH_PARMTYPE,    NULL,                   1, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM DocOpenParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_NAME,       NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_SEGMENT,    LONG_PARMTYPE,      &(Parm.lAddOptions1),   0, NULL },
  { BATCH_LINE,       LONG_PARMTYPE,      &(Parm.lAddOptions2),   0, NULL },
  { BATCH_SEARCH,     NAME_PARMTYPE,      Parm.szAlias,           MAX_LONGFILESPEC, NULL },
  { BATCH_TRACKID,    NAME_PARMTYPE,      Parm.szShipment,        MAX_LONGFILESPEC, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM RemoveDocsParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_NAME,       NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM RestoreDocsParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM FolderChangePropsParms[] =
{
  { BATCH_FLD,        NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_DESC,       NAME_PARMTYPE,      Parm.szDescr,           MAX_DESCRIPTION, &(Parm.fDescription) },
  { BATCH_MEM,        NAME_PARMTYPE,      Parm.szMemory,          MAX_LONGFILESPEC, &(Parm.fMemory) },
  { BATCH_DICT,       NAMELIST_PARMTYPE,  &(Parm.pszDicList),     1, &(Parm.fDicList) },
  { BATCH_TGTLNG,     NAME_PARMTYPE,      Parm.szTargetLang,      MAX_LANG_LENGTH, &(Parm.fTargetLang) },
  { BATCH_ROMEM,      NAMELIST_PARMTYPE,  &(Parm.pszMemList),     1, &(Parm.fMemList) },
  { BATCH_SHIPMENT,   NAME_PARMTYPE,      Parm.szShipment,        sizeof(Parm.szShipment), &(Parm.fShipment) },
  { BATCH_PROFILE,    NAME_PARMTYPE,      Parm.szProfile,         sizeof(Parm.szProfile), &(Parm.fProfile) },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };

BATCHPARM MarkupCreateParms[] =                                        /* 2-19-14 */
{
  { BATCH_NAME,       NAME_PARMTYPE,      Parm.szName,            MAX_LONGFILESPEC, NULL },
  { BATCH_OUT,        NAME_PARMTYPE,      Parm.szPath,            MAX_LONGFILESPEC, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };


BATCHPARM ConnectSharedMemParms[] =                                   
{ 
  { BATCH_FROMDRIVE,  LETTER_PARMTYPE,    &(Parm.chFromDrive),      1, NULL }, 
  { BATCH_MEM,        NAME_PARMTYPE,      Parm.szMemory,          MAX_LONGFILESPEC, NULL },
  { BATCH_END,        DUMMY_PARMTYPE,     NULL,                   0, NULL } };


//+----------------------------------------------------------------------------+
//| main                                                                       |
//+----------------------------------------------------------------------------+

static void showHelp();

int main
( 
  int argc, 
  char *argv[],
  char *envp[] 
)
{
  USHORT   usRC = 0;
  PBATCHDATA pData = NULL;
  BOOL fErrorShown = FALSE;

  envp;

  if(argc==2 && stricmp(argv[1],"-h")==0 )
  {
      showHelp();
      return 0;
  }

  // allocate our data area
  if ( !UtlAllocHwnd( (PVOID *)&pData, 0, sizeof(BATCHDATA), NOMSG, HWND_FUNCIF ) )
  {
     MessageBox( NULL, "Memory allocation error, program stopped", "OtmBatch Error", MB_CANCEL | MB_ICONSTOP );
     fErrorShown = TRUE;
  }

  // ensure that \EQF\TABLE is in PATH environment variable 
  if ( !usRC )
  {
    char *pathVar = getenv( "PATH" );
    char szTableDir[20];

    strcpy( pData->szMsgBuffer, pathVar );
    strupr( pData->szMsgBuffer );
    if ( strstr( pData->szMsgBuffer, "\\OTM\\TABLE" ) == NULL )
    {
      strcpy( pData->szMsgBuffer, "PATH=" );
      strcat( pData->szMsgBuffer, pathVar );
      strcat( pData->szMsgBuffer, ";" );
      GetProfileString( "EqfStart", "Drive", "", szTableDir, sizeof( szTableDir ));
      strcpy( szTableDir + 1, ":\\OTM\\TABLE" );
      strcat( pData->szMsgBuffer, szTableDir );
      _putenv( pData->szMsgBuffer );
    } /* endif */
    pData->szMsgBuffer[0] = 0 ;
  } 

  // initialize our API session
  if ( !usRC )
  {
    USHORT usProgID = 0;
    pData->hwndErrMsg = HWND_FUNCIF;             // use func IF message box
    usRC = EqfStartSession( &(pData->hSession) );
    usProgID = UtlQueryUShort( QS_PROGRAMID ); 
    UtlSetUShort( QS_PROGRAMID, BATCHUTIL_PROGID );
    usProgID = UtlQueryUShort( QS_PROGRAMID );
    usProgID = UtlQueryUShort( QS_PROGRAMID );
  } /* endif */


  if ( !usRC )
  {
    argc--; argv++;
    pData->szCmdLine[0] = EOS;
    int iCmd = 0;
    while ( argc )
    { 
      //pre-process command parameter to skip '/' in value
      {
          int assignFound = 0;
          int quoteFound = 0;
          for(size_t i=0; i<strlen(argv[0]); i++)
          {
              if(assignFound==0)
              {
                  if(argv[0][i]=='=')
                      assignFound=1;
              }
              else if(quoteFound==0)
              {
                  if(argv[0][i]=='"')
                      quoteFound=1;
                  else if(argv[0][i]=='/')
                  {
                      pData->szCmdLine[iCmd++]='"';
                      pData->szCmdLine[iCmd++]=argv[0][i];
                      pData->szCmdLine[iCmd++]='"';
                      continue;
                  }
              }
              else
              {
                  if(argv[0][i]=='"')
                      quoteFound=0;
              }
              pData->szCmdLine[iCmd++]=argv[0][i];
          }//end for

          if ( argc > 1 )
              pData->szCmdLine[iCmd++] = ' ';
      }
      // end pre-process command line arguments to skip '/'

      //strcat( pData->szCmdLine, argv[0] );
      //if ( argc > 1 ) strcat( pData->szCmdLine, " " );

      argc--;
      argv++;
    } /*endwhile */
    
    // add a precaution
    if(iCmd >= sizeof(pData->szCmdLine)/sizeof(pData->szCmdLine[0]))
    {
        printf("The characters of this command exceed 2048\n");
        return 0;
    }
    pData->szCmdLine[iCmd] = EOS;
    // end add a precaution

  } /* endif */

  // if a task list is specified process this tasklist otherwise handle single task
  if ( !usRC )
  {
    PSZ pszTaskList = NULL;
    BOOL fNoErrorStop = FALSE;

    // look for tasklist parameter
    strcpy( pData->szOrgCmdLine, pData->szCmdLine );
    strupr( pData->szCmdLine );
    pszTaskList = strstr( pData->szCmdLine, szTaskList );
    if ( pszTaskList )
    {
      pszTaskList += strlen(szTaskList);
    }
    else
    {
      pszTaskList = strstr( pData->szCmdLine, szShortTaskList );
      if ( pszTaskList )
      {
        pszTaskList += strlen(szShortTaskList);
      } /* endif */
    } /* endif */

    // get quiet option
    BatchGetQuietMode( pData );

    // handle tasklist or single task
    if ( pszTaskList )
    {
      FILE *hfTaskList = NULL;
      PSZ pszName = NULL;

      // get noerrorstop option
      fNoErrorStop = strstr( pData->szCmdLine, szNoErrorStop ) == NULL;
      if ( !fNoErrorStop)
      {
        fNoErrorStop = strstr( pData->szCmdLine, szShortNoErrorStop ) == NULL;
      } /* endif */

      // extract task list name
      pszName = pData->szTaskList;
      while ( *pszTaskList && (*pszTaskList != ' ') && (*pszTaskList != '/') )
      {
        *pszName++ = *pszTaskList++;
      } /*endwhile */
      *pszName = EOS;

      // open list file and process file
      hfTaskList = fopen( pData->szTaskList, "r" );
      if ( hfTaskList)
      {
        fgets( pData->szCmdLine, sizeof(pData->szCmdLine) - 1, hfTaskList );
        while ( !usRC && !feof(hfTaskList) )
        {
          int iLen = strlen(pData->szCmdLine);
          if ( iLen && pData->szCmdLine[iLen-1] == '\n' ) pData->szCmdLine[iLen-1] = EOS;

          if ( pData->szCmdLine[0] != '*' )
          {
            strcpy( pData->szOrgCmdLine, pData->szCmdLine );
            strupr( pData->szCmdLine );
            usRC = BatchCheckCommand( pData );
          } /* endif */

          if ( usRC && fNoErrorStop )
          {
            usRC = 0;
          } /* endif */

          if ( !usRC )
          {
            fgets( pData->szCmdLine, sizeof(pData->szCmdLine) - 1, hfTaskList );
          } /* endif */
        } /*endwhile */
        fclose( hfTaskList );
      }
      else
      {
        sprintf( pData->szMsgBuffer, "Failed to open tasklist \"%s\"", pData->szTaskList );
        usRC = 15;
        if ( pData->usQuietMode == QUIET_NOMSG )
        {
          printf( "%s\n", pData->szMsgBuffer );
        }
        else if ( pData->usQuietMode == FALSE  )
        {
          MessageBox( NULL, pData->szMsgBuffer, "OtmBatch Error", MB_CANCEL | MB_ICONSTOP );
        } /* endif */
        fErrorShown = TRUE;
      } /* endif */
    }
    else
    {
      // process single task
      usRC = BatchCheckCommand( pData );
    } /* endif */
  } /* endif */

  if ( pData )
  {
    if ( fErrorShown )
    {
      // nothing todo
    }
    else if ( usRC )
    {
      USHORT usLastRC = 0;
      EqfGetLastError( pData->hSession, &usLastRC, pData->szMsgBuffer, sizeof(pData->szMsgBuffer) );
      if ( pData->szMsgBuffer[0] )
      {
        if ( pData->usQuietMode == QUIET_NOMSG )
        {
          printf( "%s\n", pData->szMsgBuffer );
        }
        else if ( pData->usQuietMode == FALSE  )
        {
          MessageBox( NULL, pData->szMsgBuffer, "OtmBatch Error", MB_CANCEL | MB_ICONSTOP );
        } /* endif */
      }
      else
      {
        if ( pData->usQuietMode == QUIET_NOMSG )
        {
          printf( "Unknown error while processing the command\n"  );
        }
        else if ( pData->usQuietMode == FALSE  )
        {
          MessageBox( NULL, "Unknown error while processing the command", "OtmBatch Error", MB_CANCEL | MB_ICONSTOP );
        } /* endif */
      } /* endif */
      if ( usLastRC != 0 )
      {
        usRC = usLastRC;
      } /* endif */
    }
    else 
    {
      // show completion message
      if ( pData->usQuietMode == QUIET_NOMSG )
      {
        printf( "OtmBatch processing completed successfully\n"  );
      }
      else if ( pData->usQuietMode == FALSE  )
      {
        MessageBox( NULL, "OtmBatch processing completed successfully", "BatchUtil Info", MB_OK );
      } /* endif */
    } /* endif */

    if ( pData->hSession ) EqfEndSession( pData->hSession );
  } /* endif */

  if ( pData->usQuietMode == QUIET_NOMSG )
  {
    printf( "OtmBatch return code is %u\n", usRC );
  }

  return( (int)usRC );
} /* end of function main */

USHORT BatchGetQuietMode
(
  PBATCHDATA pData               // ptr to client ida
)
{
  // get quiet option
  {
    PSZ pszQuiet = strstr( pData->szCmdLine, szQuiet );
    int iLen = 0;

    if ( pszQuiet )
    {
      if ( strnicmp( pszQuiet, szQuietNomsg, iQuietNomsgLen ) == 0 )
      {
        pData->usQuietMode = QUIET_NOMSG;
        iLen = iQuietNomsgLen;
      }
      else
      {
        pData->usQuietMode = TRUE;
        iLen = strlen(szQuiet);
      } /* endif */

      // remove option from commandline
      memmove( pszQuiet, pszQuiet + iLen, strlen(pszQuiet) - iLen + 1 );

      // update original commandline as well
      pszQuiet = pData->szOrgCmdLine + (pszQuiet - pData->szCmdLine);
      memmove( pszQuiet, pszQuiet + iLen, strlen(pszQuiet) - iLen + 1 );
    }
    else
    {
      pData->usQuietMode = FALSE;
    } /* endif */
  }
  pData->fQuietprocessed = TRUE;
  return( 0 );
}

USHORT BatchCheckCommand
(
  PBATCHDATA pData               // ptr to client ida
)
{
  USHORT   usRC = 0;

  USHORT (*function)( PBATCHDATA );    // and function to be processed

  /* append '/' to allow for easier processing of last token.     */
  strcat( pData->szCmdLine, "/" );
  pData->usRC = 0;

  // get/remove quiet option
  if ( !pData->fQuietprocessed )BatchGetQuietMode( pData );

  // suppress any leading blanks
  {
    PSZ pszSource = pData->szCmdLine;
    PSZ pszTarget = pData->szCmdLine;
    while ( *pszSource == ' ' ) pszSource++;
    while ( *pszSource != EOS ) *pszTarget++ = *pszSource++;
    *pszTarget = EOS;
    pszSource = pszTarget = pData->szOrgCmdLine;
    while ( *pszSource == ' ' ) pszSource++;
    while ( *pszSource != EOS ) *pszTarget++ = *pszSource++;
    *pszTarget = EOS;
  }

  /* tokenize and analyze it ...                                    */
  pData->Task = BatchValidateTask( pData );
//  pData->pBatchList = EQFTaskList[pData->Task].BatchList;
//  pData->pMandList = EQFTaskList[pData->Task].MandList;
  if ( pData->Task == TASK_END )
  {
    usRC = pData->usRC;
  }
  else
  {
    function = EQFTaskList[pData->Task].function;
    usRC = (*function)(pData);                //execute the function
  } /* endif */

  return( usRC );
} /* end of function CheckCmdLine */

BATCHCMD BatchValidateToken
(
  PSZ  *ppToken,                     // token to be analysed
  PBATCHPARM pParmInfo               // pointer to cmd list of task
)
{
  BOOL fFound = FALSE;              // success indicator
  PSZ  pData = *ppToken;
  PSZ  pszTempDesc;                  //description of temp batchcmd
  BATCHCMD curParm = BATCH_TASK;    // currently tested task

  while ( !fFound && curParm != BATCH_END)
  {
    pszTempDesc = EQFCmdList[curParm].szDesc;
    if ( !strncmp(pszTempDesc, pData, strlen(pszTempDesc) ))
    {
      fFound = TRUE;
      *ppToken += strlen( pszTempDesc );   // return data area of token
    }
    else
    {
      pszTempDesc = EQFCmdList[curParm].szShortCut;
      if ( !strncmp(pszTempDesc, pData, strlen(pszTempDesc) ))
      {
        fFound = TRUE;
        *ppToken += strlen( pszTempDesc );   // return data area of token
      }
      else
      {
        curParm = pParmInfo->Cmd;
        pParmInfo++;
      } /* endif */
    } /* endif */
  } /* endwhile */

  return( curParm );
} /* end of function BatchValidateToken */


BOOL ListOfFiles
(
  PBATCHDATA   pData,          //ptr to DDE instance structure
  PSZ **pppCurDirIndex,             // pointer to curdir array
  PSZ pFile,                        // name of list file
  PSZ  **pppListIndex               // pointer to listindex array
)
{
  PSZ  pszData;                          // pointer to data
  BOOL fOK = TRUE;                     // success indicator
  ULONG   ulSize = 0;                      // size of the file
  USHORT usChar;                       // character to be checked
  CHAR   szBuf[MAX_LONGPATH+MAX_FILESPEC]; // temp buffer
  CHAR   szOutBuf[ MAX_EQF_PATH ];

  pData;                         // get rid of compiler warnings
  /********************************************************************/
  /* ensure, that the list  file is fully qualified -- if not do it   */
  /********************************************************************/
  pszData = UtlGetFnameFromPath( pFile );
  if ( !pszData && (pppCurDirIndex != NULL))
  {
    fOK = UtlInsertCurDir(pFile, pppCurDirIndex, szOutBuf);
    if ( fOK )
    {
      pFile = szOutBuf;
      strcpy( szBuf, pFile );
    } /* endif */
  }
  else
  {
    strcpy( szBuf, pFile );
  } /* endif */

  /********************************************************************/
  /* load the file - limited to files up to 64k                       */
  /********************************************************************/
  if ( fOK )
  {
    pszData = NULL;
    fOK = UtlLoadFileL( szBuf, (PVOID *)&pszData, &ulSize, FALSE, FALSE );
  } /* endif */

  if ( fOK )
  {
    /******************************************************************/
    /* get rid of the file ending symbols ....                        */
    /******************************************************************/
    if ( pszData[ulSize-1] == EOFCHAR )
    {
      ulSize--;
    } /* endif */
    usChar = pszData[ulSize-1];
    while ( usChar ==  LF || usChar == CR || usChar == BLANK)
    {
      ulSize--;
      usChar = pszData[ulSize-1];
    } /* endwhile */

    pszData[ulSize] = EOS;

    fOK = UtlValidateList( pszData, pppListIndex ,MAX_DDE_FILES );
  } /* endif */

  return ( fOK );
} /* end of function ListOfFiles */

// dummy for not implemented functions
USHORT BatchNotImplemented( PBATCHDATA pData )
{
  pData;
  return( 999 );
}

// batch analysis
USHORT BatchAnalysis
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, AnalysisParms );

  // check whether all mandatory parameters have beeb provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_END };

    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call analysis function
  if ( !usRC )
  {
    PSZ pszProfile = (Parm.szProfile[0] != EOS ) ? Parm.szProfile : NULL;
    do
    {
      usRC = EqfAnalyzeDocEx( pData->hSession, Parm.szName, Parm.pszDocList, Parm.pszMemList, pszProfile, NULL, Parm.lOptions );
    } while ( usRC == CONTINUE_RC );
    if ( usRC )
    {
      BatchHandleAPIError( pData, usRC );
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );


  return( usRC );
} /* end of function BatchAnalysis*/

USHORT BatchCreateFolder 
(
  PBATCHDATA pData
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, FolderCreateParms );

  // check whether all mandatory parameters are provided mandatory are: name, drive, memory, markup, editor, 
  // source language, target language
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_NAME, BATCH_MEM, BATCH_MARKUP, BATCH_SRCLNG, BATCH_TGTLNG, BATCH_END };

    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call create folder function
  if ( !usRC )
  {
    if ( Parm.szEdit[0] == EOS ) strcpy( Parm.szEdit, "Standard" );
    usRC = EqfCreateFolder( pData->hSession, Parm.szName, Parm.szDescr, Parm.chToDrive, Parm.pszMemList, Parm.szMarkup,
                            Parm.szEdit, Parm.pszDicList, Parm.szSourceLang, Parm.szTargetLang, Parm.szConversion,
                            NULL );
    if ( usRC )
    {
      BatchHandleAPIError( pData, usRC );
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );

} /* end of function BatchCreateFolder */

USHORT BatchExportDoc  
(
  PBATCHDATA pData
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, DocExportParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_FILES, BATCH_END };

    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call document export function
  if ( !usRC )
  {
    // prepare options
    if ( Parm.lOptions & VALFORMAT_XML_OPT )
    {
      Parm.lOptions &= ~VALFORMAT_XML_OPT;
      Parm.lOptions |= Parm.lAddOptions1;
    } /* endif */
    if ( Parm.lOptions & PLAINXML_OPT )
    {
      Parm.lOptions |= (Parm.lAddOptions1 & VALFORMAT_COMBINE_OPT);
    } /* endif */


    do
    {
      usRC = EqfExportDoc( pData->hSession, Parm.szName, Parm.pszDocList, Parm.szPath, Parm.lOptions );
    } while ( usRC == CONTINUE_RC );

    if ( usRC )
    {
      BatchHandleAPIError( pData, usRC );
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );

} /* end of function BatchExportDoc */

// batch document import
USHORT BatchImportDoc
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, DocImportParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_FILES, BATCH_END };

    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call document import function
  if ( !usRC )
  {
    do
    {
      usRC = EqfImportDoc( pData->hSession, Parm.szName, Parm.pszDocList, Parm.pszMemList, Parm.szMarkup,
                          Parm.szEdit, Parm.szSourceLang, Parm.szTargetLang, Parm.szAlias, Parm.szPath,
                          Parm.szConversion, Parm.lOptions );
    } while ( usRC == CONTINUE_RC );
    if ( usRC )
    {
      BatchHandleAPIError( pData, usRC );
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );


  return( usRC );
} /* end of function BatchImportDoc */

// batch folder export
USHORT BatchExportFolder
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, FolderExportParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_TODRIVE, BATCH_END };

    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call export folder function
  if ( !usRC )
  {
    // prepare path information
    if ( Parm.szPath[0] != '\0' )
    {
      sprintf( pData->szPathBuffer, "%c:%s", Parm.chToDrive, Parm.szPath );
    }
    else
    {
      sprintf( pData->szPathBuffer, "%c:\\OTM\\EXPORT", Parm.chToDrive );
    } /* endif */

    // do the export
    do
    {
      usRC = EqfExportFolderFP( pData->hSession, Parm.szName, pData->szPathBuffer, Parm.lOptions, Parm.pszDocList,
                              Parm.szDescr );
    } while ( usRC == CONTINUE_RC );

    if ( usRC )
    {
      BatchHandleAPIError( pData, usRC );
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchExportFolder */

// batch folder import
USHORT BatchImportFolder
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, FolderImportParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_FROMDRIVE, BATCH_END };

    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call folder import function
  if ( !usRC )
  {
    if ( Parm.szPath[0] )
    {
      sprintf( pData->szPathBuffer, "%c:%s", Parm.chFromDrive, Parm.szPath );
      do
      {
        if ( Parm.szImportAs[0] )
        {
          usRC = EqfImportFolderAs( pData->hSession, Parm.szName, pData->szPathBuffer, Parm.chToDrive, Parm.szImportAs, Parm.lOptions );
        }
        else
        {
          usRC = EqfImportFolderFP( pData->hSession, Parm.szName, pData->szPathBuffer, Parm.chToDrive, Parm.lOptions );
        } /* endif */
      } while ( usRC == CONTINUE_RC );
    }
    else
    {
      do
      {
        usRC = EqfImportFolder( pData->hSession, Parm.szName, Parm.chFromDrive, Parm.chToDrive, Parm.lOptions );
      } while ( usRC == CONTINUE_RC );
    } /* endif */

    if ( usRC )
    {
      BatchHandleAPIError( pData, usRC );
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchImportFolder */

// batch folder delete
USHORT BatchDeleteFolder
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, FolderDeleteParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_END };

    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call delete folder function
  if ( !usRC )
  {
    usRC = EqfDeleteFolder( pData->hSession, Parm.szName );

    if ( usRC )
    {
      BatchHandleAPIError( pData, usRC );
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchDeleteFolder */

// batch count words
USHORT BatchCountWords
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, WordCountParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_OUT, BATCH_END };

    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call count words function
  if ( !usRC )
  {
    do
    {
      usRC = EqfCountWords( pData->hSession, Parm.szName, Parm.pszDocList, Parm.lOptions | Parm.lAddOptions3, Parm.szPath );
    } while ( usRC == CONTINUE_RC );

    if ( usRC )
    {
      BatchHandleAPIError( pData, usRC );
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchCountWords */

// batch document delete
USHORT BatchDeleteDoc
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, DocDeleteParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_FILES, BATCH_END };
    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call document delete function
  if ( !usRC )
  {
    usRC = EqfDeleteDoc( pData->hSession, Parm.szName, Parm.pszDocList );
    if ( usRC ) BatchHandleAPIError( pData, usRC );
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchDeleteDoc */

// batch memory export
USHORT BatchExportMem
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, MemoryExportParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_MEM, BATCH_OUT, BATCH_END };
    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call memory export function
  if ( !usRC )
  {
    do
    {
      usRC = EqfExportMem( pData->hSession, Parm.szName, Parm.szPath, Parm.lOptions );
    } while ( usRC == CONTINUE_RC );

    if ( usRC ) BatchHandleAPIError( pData, usRC );
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchExportMem */

// batch memory delete
USHORT BatchDeleteMem
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, MemoryDeleteParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_MEM, BATCH_END };
    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call delete memory function
  if ( !usRC )
  {
    usRC = EqfDeleteMem( pData->hSession, Parm.szName );

    if ( usRC ) BatchHandleAPIError( pData, usRC );
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchDeleteMem */

// batch memory create
USHORT BatchCreateMem
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, MemoryCreateParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_NAME, BATCH_TODRIVE, BATCH_SRCLNG, BATCH_END };
    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call memor create function
  if ( !usRC )
  {
    usRC = EqfCreateMem( pData->hSession, Parm.szName, Parm.szDescr, Parm.chToDrive, Parm.szSourceLang, Parm.lOptions );
  
    if ( usRC ) BatchHandleAPIError( pData, usRC );
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchCreateMem */


// batch memory import
USHORT BatchImportMem
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, MemoryImportParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_MEM, BATCH_FILES, BATCH_END };
    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call memory import function
  if ( !usRC )
  {
    do
    {
      usRC = EqfImportMem( pData->hSession, Parm.szName, Parm.szPath, Parm.lOptions );
    } while ( usRC == CONTINUE_RC );
    if ( usRC ) BatchHandleAPIError( pData, usRC );
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchImportMem */

// batch memory organize
USHORT BatchOrganizeMem
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, MemoryOrganizeParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_MEM, BATCH_END };
    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call memory organize function
  if ( !usRC )
  {
    do
    {
      usRC = EqfOrganizeMem( pData->hSession, Parm.szName );
    } while ( usRC == CONTINUE_RC );
    if ( usRC ) BatchHandleAPIError( pData, usRC );
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchOrganizeMem */

// batch dictionary export
USHORT BatchExportDict
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, DictionaryExportParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_DICT, BATCH_OUT, BATCH_END };
    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call dictionary export function
  if ( !usRC )
  {
    do
    {
      usRC = EqfExportDict( pData->hSession, Parm.szName, Parm.lOptions, Parm.szPath );
    } while ( usRC == CONTINUE_RC );
    if ( usRC ) BatchHandleAPIError( pData, usRC );
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchExportDict */

// batch dictionary import
USHORT BatchImportDict
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, DictionaryImportParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_DICT, BATCH_FILES, BATCH_END };
    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call dictionary import function
  if ( !usRC )
  {
    do
    {
      usRC = EqfImportDict( pData->hSession, Parm.szPath, Parm.szName, Parm.szPassword, Parm.lOptions );
    } while ( usRC == CONTINUE_RC );

    if ( usRC ) BatchHandleAPIError( pData, usRC );
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchImportDict */

// batch dictionary import
USHORT BatchArchiveMem
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, ArchiveMemoryParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_MEM, BATCH_END };
    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call archive memory function
  if ( !usRC )
  {
    do
    {
      usRC = EqfArchiveTM( pData->hSession, Parm.szName, Parm.chToDrive, Parm.pszDocList, Parm.szMemory, Parm.lOptions );
    } while ( usRC == CONTINUE_RC );

    if ( usRC ) BatchHandleAPIError( pData, usRC );
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchArchiveMem */


// batch create counting report
USHORT BatchCountReport
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, CountReportParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_OUT, BATCH_REPORT, BATCH_TYPE, BATCH_END };
    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call archive memory function
  if ( !usRC )
  {        
    do
    {
      usRC = EqfCreateCountReportEx( pData->hSession, Parm.szName, Parm.pszDocList, Parm.szPath, (USHORT)Parm.lAddOptions1,
        (USHORT)Parm.lAddOptions2, Parm.szProfile, Parm.szShipment, NULL, NULL, Parm.lOptions | Parm.lAddOptions3 );
    } while ( usRC == CONTINUE_RC );
    if ( usRC ) BatchHandleAPIError( pData, usRC );
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchCountReport */

// batch rename
USHORT BatchRename
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;
  LONG lOptions = 0;


  usRC = BatchGetParameters( pData, RenameParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_NEW, BATCH_END };

    usRC = BatchCheckMandParms( pData, MandParms );

    // in addition exactly one of BATCH_FLD, BATCH_MEM or BATCH_DICT has to be specified
    if ( !usRC )
    {
      if ( !Parm.aParmUsed[BATCH_FLD] && !Parm.aParmUsed[BATCH_MEM] && !Parm.aParmUsed[BATCH_DICT] )
      {
        usRC = pData->usRC = DDE_MANDPARAMISSING;
        {
          PSZ pszParm = "/FLD, /MEM or /DICT";
          UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  // call delete folder function
  if ( !usRC )
  {
    USHORT usRename = 0;

    // get type of rename
    if ( Parm.szName[0] != EOS )
    {
      usRename = RENAME_FOLDER;
    } 
    else if ( Parm.szMemory[0] != EOS )
    {
      if ( usRename != 0)
      {
        PSZ pszParm = "/MEM (together with /FLD)";
        usRC = pData->usRC = DDE_WRONGBATCH;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
      }
      else
      {
        usRename = RENAME_MEMORY;
        strcpy( Parm.szName, Parm.szMemory );
      } /* endif */
    }
    else if ( Parm.szAlias[0] != EOS )
    {
      if ( usRename != 0)
      {
        PSZ pszParm = (usRename == RENAME_FOLDER) ? "/DICT (together with /FLD)" : "/DICT (together with /MEM)";
        usRC = pData->usRC = DDE_WRONGBATCH;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
      }
      else
      {
        usRename = RENAME_DICTIONARY;
        strcpy( Parm.szName, Parm.szAlias );
      } /* endif */
    }
    else
    {
      usRC = pData->usRC = DDE_MANDPARAMISSING;
      {
        PSZ pszParm = "/FLD, /MEM or /DICT";
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
      } /* endif */
    } /* endif */

    // adjust is only valid fpr DICT or TM rename
    if ( !usRC )
    {
      if ( Parm.fSwitch1 && (usRename == RENAME_FOLDER))
      {
        PSZ pszParm = "/ADJUST (together with /FLD)";
        usRC = pData->usRC = DDE_WRONGBATCH;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
      } /* endif */
    } /* endif */

    if ( !usRC )
    {
      if ( Parm.fSwitch1 )
      {
        lOptions = ADJUSTREFERENCES_OPT;
      } /* endif */
    } /* endif */

    if ( !usRC )
    {
      usRC = EqfRename( pData->hSession, usRename, Parm.szName, Parm.szPath, lOptions  );

      if ( usRC )
      {
        BatchHandleAPIError( pData, usRC );
      } /* endif */
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchDeleteFolder */


// batch open document
USHORT BatchOpenDoc
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;
  BOOL   fWithTrackID = FALSE ;

  usRC = BatchGetParameters( pData, DocOpenParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    if ( Parm.aParmUsed[BATCH_TRACKID] ) {
       fWithTrackID = TRUE ;
       BATCHCMD MandParms[] = { BATCH_FLD, BATCH_TRACKID, BATCH_END };
       usRC = BatchCheckMandParms( pData, MandParms );
    } else {
       BATCHCMD MandParms[] = { BATCH_FLD, BATCH_NAME, BATCH_END };
       usRC = BatchCheckMandParms( pData, MandParms );
    }

  } /* endif */

  // call document open function
  if ( !usRC )
  {
    // if OTM minimized, restore it firstly
    HWND hotmwnd = FindWindow( TWBMAIN ,NULL);
	if(hotmwnd != NULL && IsIconic(hotmwnd))
	{
		PostMessage( hotmwnd, WM_SYSCOMMAND, WPARAM(SC_RESTORE), 0L );
	}

    
    if ( fWithTrackID ) {
       usRC = EqfOpenDocByTrack( pData->hSession, Parm.szName, Parm.szShipment );
    } else {
       // convert any search string to UTF-16
       if ( Parm.szAlias[0] != EOS )
       {
         memset( pData->szUTF16Buffer, 0, sizeof(pData->szUTF16Buffer) );
         MultiByteToWideChar( CP_ACP, 0, Parm.szAlias, -1, pData->szUTF16Buffer, MAX_LONGFILESPEC ); 
       } /* end */       

       usRC = EqfOpenDocEx( pData->hSession, Parm.szName, Parm.szPath, Parm.lAddOptions1, Parm.lAddOptions2, pData->szUTF16Buffer );
    }

    if ( usRC )
    {
      BatchHandleAPIError( pData, usRC );
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchOpenDoc */


// batch remove documents
USHORT BatchRemoveDocs
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, RemoveDocsParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_NAME, BATCH_END };

    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call remove documents function
  if ( !usRC )
  {
    usRC = EqfRemoveDocs( pData->hSession, Parm.szName, Parm.szPath );

    if ( usRC )
    {
      BatchHandleAPIError( pData, usRC );
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchRemoveDocs */


// batch restore documents
USHORT BatchRestoreDocs
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, RestoreDocsParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_END };

    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call remove documents function
  if ( !usRC )
  {
    usRC = EqfRestoreDocs( pData->hSession, Parm.szName );

    if ( usRC )
    {
      BatchHandleAPIError( pData, usRC );
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchRestoreDocs */




// batch change folder properties
USHORT BatchChangeFolProps
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, FolderChangePropsParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_FLD, BATCH_END };

    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call change folder propertiesfunction
  if ( !usRC )
  {
    usRC = EqfChangeFolPropsEx( pData->hSession, Parm.szName, NULC, 
                                Parm.fTargetLang ? Parm.szTargetLang : NULL, 
                                Parm.fMemory ? Parm.szMemory : NULL,
                                Parm.fDicList ? Parm.pszDicList : NULL, 
                                Parm.fMemList ? Parm.pszMemList : NULL, 
                                Parm.fDescription ? Parm.szDescr : NULL, 
                                Parm.fProfile ? Parm.szProfile : NULL, 
                                Parm.fShipment ? Parm.szShipment : NULL);

    if ( usRC )
    {
      BatchHandleAPIError( pData, usRC );
    } /* endif */
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchDeleteFolder */

// batch markup table create                    2-19-14
USHORT BatchCreateMarkup
(
  PBATCHDATA pData 
)
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, MarkupCreateParms );

  // check whether all mandatory parameters have been provided 
  if ( !usRC )
  {
    BATCHCMD MandParms[] = { BATCH_NAME, BATCH_OUT, BATCH_END };
    usRC = BatchCheckMandParms( pData, MandParms );
  } /* endif */

  // call markup table create function
  if ( !usRC )
  {
    usRC = EqfCreateMarkup( pData->hSession, Parm.szName, Parm.szPath );
  
    if ( usRC ) BatchHandleAPIError( pData, usRC );
  } /* endif */

  BatchCleanParms( pData );

  return( usRC );
} /* end of function BatchCreateMarkup */




//
// Utilities
//
USHORT BatchCheckMandParms
(
  PBATCHDATA pData,
  BATCHCMD *pMandParms
)
{
  USHORT usRC = 0;

  while ( !usRC && (*pMandParms != BATCH_END) )
  {
    if ( ! Parm.aParmUsed[*pMandParms] )
    {
      usRC = pData->usRC = DDE_MANDPARAMISSING;
      {
        PSZ pszParm = EQFCmdList[*pMandParms].szDesc;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
      } /* endif */
    }
    else
    {
      pMandParms++;
    } /* endif */
  } /*endwhile */
  return( usRC );
} /* end of function BatchCheckMandParms */

USHORT BatchCleanParms 
(
  PBATCHDATA pData
)
{
  pData;

  if ( Parm.pszDicList ) UtlAlloc( (PVOID *)&(Parm.pszDicList), 0L, 0L, NOMSG );
  if ( Parm.pszDocList ) UtlAlloc( (PVOID *)&(Parm.pszDocList), 0L, 0L, NOMSG );
  if ( Parm.pszMemList ) UtlAlloc( (PVOID *)&(Parm.pszMemList), 0L, 0L, NOMSG );
  memset( &Parm, 0, sizeof(Parm) );

  return( 0 );

} /* end of function BatchCleanParms */

USHORT BatchGetParameters 
(
  PBATCHDATA pData,
  PBATCHPARM pParmInfo
)
{
  USHORT usRC = 0;
  PSZ   pStart;                        // pointer to start of current token
  PSZ   pActive;                       // pointer to active position
  CHAR  c;
  BATCHCMD  BatchCmd = (BATCHCMD)0;                  // active batch command

  memset( &Parm, 0, sizeof(BATCHPARM) );

  if ( !usRC )
  {
    pActive = pStart = pData->szCmdLine;
    while ( ((c=*pActive)!= NULC) && !usRC )
    {
      switch ( c )
      {
        case '/':
          if (*(pActive+1)== '/')
          {
             pActive += 2;                       //skip duplicated slash
          }
          else
          {
            if ( pStart != pActive )
            {
              PSZ pszBlankAtParmEnd = NULL;

              *pActive = EOS;                    //just temporary ...

              // supress any blanks at the end of the parameter 
              {
                int iLastCharPos = strlen(pStart) - 1;

                if ( pStart[iLastCharPos] == ' ' )
                {
                  PSZ pszTemp = pStart + iLastCharPos;
                  while ( *pszTemp == ' ' )
                  {
                    pszBlankAtParmEnd = pszTemp;
                    pszTemp--;
                  } /*endwhile */

                  *pszBlankAtParmEnd = EOS;
                } /* endif */
              }

              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = BatchValidateToken( &pStart, pParmInfo );

              Parm.aParmUsed[BatchCmd] = TRUE;
              
              if ( BatchCmd == BATCH_TASK )
              {
                // ignore task, has been evaluated already
              }
              else if ( (pStart != pActive) || (BatchCmd == BATCH_DESC) || (BatchCmd == BATCH_ROMEM) || (BatchCmd == BATCH_DICT) )
              {
                PBATCHPARM pCurrentParm = pParmInfo;

                BatchBothStripDuplSlash(pData, pStart, pActive);

                while ( (pCurrentParm->Type != DUMMY_PARMTYPE) && (pCurrentParm->Cmd != BatchCmd) )
                {
                  pCurrentParm++;
                } /*endwhile */

                if ( pCurrentParm->Type != DUMMY_PARMTYPE)
                {
                  if ( pCurrentParm->pfUsed != NULL ) *(pCurrentParm->pfUsed) = TRUE;

                  switch ( pCurrentParm->Type )
                  {
                    case OPTION_PARMTYPE:
                        usRC = BatchOptions( pData, pStart, (PBATCHOPTION)pCurrentParm->pvValue, &(Parm.lOptions), BatchCmd ); 
                        break;
                    case ADDOPTION_PARMTYPE:
                      {
                        PLONG plOption;
                        switch( pCurrentParm->lMaxLength )
                        {
                          case 1 : plOption = &(Parm.lAddOptions1); break;
                          case 2 : plOption = &(Parm.lAddOptions2); break;
                          case 3 : plOption = &(Parm.lAddOptions3); break;
                          default: plOption = &(Parm.lOptions);  break;
                        } /*endswitch */
                        usRC = BatchOptions( pData, pStart, (PBATCHOPTION)pCurrentParm->pvValue, plOption, BatchCmd ); 
                      }
                      break;
                    case NAME_PARMTYPE:
                        usRC = BatchStringParm( pData, pStart, (PSZ)pCurrentParm->pvValue, pCurrentParm->lMaxLength, BatchCmd );
                        break;
                    case NAMELIST_PARMTYPE:
                      usRC = BatchNameList( pData, pStart, (PSZ *)pCurrentParm->pvValue, BatchCmd, pCurrentParm->lMaxLength ); 
                        break;
                    case LETTER_PARMTYPE:
                      usRC = BatchDriveParm( pData, pStart, (CHAR *)pCurrentParm->pvValue, BatchCmd ); 
                      break;
                    case OVERWRITE_PARMTYPE:
                      usRC = BatchOverwriteParm( pData, pStart ); 
                      break;
                    case  LONG_PARMTYPE:
                      usRC = BatchLongParm( pData, pStart, (LONG *)pCurrentParm->pvValue, BatchCmd ); 
                      break;

                    case   SWITCH_PARMTYPE:
                      {
                        BOOL fValue = FALSE;

                        if ( (stricmp( pStart, "YES" ) == 0 ) || (stricmp( pStart, "Y" ) == 0 ) )
                        {
                          fValue = TRUE;
                        } /* endif */

                        switch( pCurrentParm->lMaxLength )
                        {
                          case 1 : Parm.fSwitch1 = fValue; break;
                          case 2 : Parm.fSwitch2 = fValue; break;
                          case 3 : Parm.fSwitch2 = fValue; break;
                        } /*endswitch */
                      }
                      break;
                    default:
                        break;
                  } /*endswitch */
                }
                else if ( BatchCmd == BATCH_TASK )
                {
                  // has already been processed
                }
                else
                {
                  usRC = pData->usRC = BATCH_WRONGPARM;
                  {
                    PSZ pszParms[2];
                    pszParms[0] = pStart;
                    pszParms[1] = pData->szMsgBuffer;
                    BatchMakeParmList( pParmInfo, pData->szMsgBuffer );
                    UtlErrorHwnd( pData->usRC, MB_CANCEL, 2, pszParms, EQF_ERROR, pData->hwndErrMsg );
                  } /* endif */
                } /* endif */
              }
              else
              {
                 usRC = pData->usRC = DDE_WRONGCMDLINE;
                  {
                    PSZ pszParm = pStart;
                    UtlErrorHwnd( pData->usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
                  } /* endif */
              } /* endif */
              *pActive = '/';                  //reset EOS to slash
              if ( pszBlankAtParmEnd ) *pszBlankAtParmEnd = ' ';
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              pActive++;
            } /* endif */
          } /* endif */
          break;

        case DOUBLEQUOTE:
          pActive++;
          while ( *pActive && (*pActive != DOUBLEQUOTE) ) pActive++;
          if ( *pActive == DOUBLEQUOTE ) pActive++;
          break;

        default :
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */
  } /* endif */

  return( usRC );

} /* end of function BatchGetParameters */



// parameter handling utilities
USHORT BatchOverwriteParm( PBATCHDATA pData, PSZ pStart )
{
  USHORT usRC = 0;

  if ( !(stricmp(pStart, YES_STRING)) )
  {
    Parm.lOptions |= OVERWRITE_OPT;
  }
  else if ( (stricmp(pStart, NO_STRING)) != 0 )
  {
    PSZ pszParm;

    usRC = pData->usRC = DDE_WRONGCMDLINE;
    pszParm = EQFCmdList[BATCH_OVERWRITE].szDesc;
    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
  } /* endif */
  return( usRC );
} /* end of function BatchOverwriteParm */

USHORT BatchLongParm( PBATCHDATA pData, PSZ pStart, PLONG plValue, BATCHCMD cmd )
{
  USHORT usRC = 0;

  // test  if only digits have been used
  PSZ pszTemp = pStart;
  while( isdigit(*pszTemp) ) pszTemp++;

  if ( *pszTemp == EOS )
  {
    *plValue = atol( pStart );
  }
  else 
  {
    PSZ pszParm;

    usRC = pData->usRC = DDE_WRONGCMDLINE;
    pszParm = EQFCmdList[cmd].szDesc;
    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
  } /* endif */
  return( usRC );
} /* end of function BatchLongParm */


USHORT BatchStringParm( PBATCHDATA pData, PSZ pStart, PSZ pszName, int iMaxLen, BATCHCMD cmd )
{
  USHORT usRC = 0;
  int iLen = strlen( pStart );

  if ( iLen < iMaxLen )
  {
    // get string from original command line
    int iOffs = pStart - pData->szCmdLine;
    char chTemp = pData->szOrgCmdLine[iOffs+iLen];
    pData->szOrgCmdLine[iOffs+iLen] = EOS;

    strcpy( pszName, pData->szOrgCmdLine + iOffs );

    pData->szOrgCmdLine[iOffs+iLen] = chTemp;
  }
  else
  {
    usRC = pData->usRC = DDE_WRONGCMDLINE;
    {
      PSZ pszParm = EQFCmdList[cmd].szDesc;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
    } /* endif */
  } /* endif */
  return( usRC );
} /* end of function BatchOverwriteParm */

USHORT BatchOptions( PBATCHDATA pData, PSZ pStart, PBATCHOPTION pOptions, PLONG plOption,  BATCHCMD cmd  )
{
  USHORT usRC = 0;
  PSZ   *ppListIndex = NULL;

  if ( UtlValidateList( pStart, &ppListIndex ,MAX_DDE_FILES) )
  {
    USHORT usI = 0;
    while ( ppListIndex[usI] && !usRC )
    {
      PBATCHOPTION pOption = pOptions;
      BOOL fFound = FALSE;
      while ( !fFound && pOption->szName[0] )
      {
        if ( stricmp( ppListIndex[usI], pOption->szName ) == 0 )
        {
          fFound = TRUE;
          *plOption |= pOption->lValue;
        }
        else
        {
          pOption++;
        } /* endif */
      } /*endwhile */
      if ( !fFound )
      {
        usRC = pData->usRC = BATCH_WRONGVALUE;
        {
          PSZ pszParms[3];
          BatchMakeOptionList( pOptions, pData->szMsgBuffer );
          pszParms[0] = ppListIndex[usI];
          pszParms[1] = EQFCmdList[cmd].szDesc;
          pszParms[2] = pData->szMsgBuffer;
          UtlErrorHwnd( usRC, MB_CANCEL, 3, pszParms, EQF_ERROR, pData->hwndErrMsg );
        } /* endif */
      } /* endif */
      usI++;
    } /* endwhile */

    UtlAlloc( (PVOID *)&ppListIndex, 0L, 0L, NOMSG );
  }
  else
  {
    usRC = pData->usRC = DDE_ERRFILELIST;
    {
      PSZ pszParm = EQFCmdList[cmd].szDesc;
      UtlErrorHwnd( pData->usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function BatchOptions */

USHORT BatchNameList( PBATCHDATA pData, PSZ pStart, PSZ *ppList, BATCHCMD cmd, LONG lAllowEmptyLists )
{
  USHORT usRC = 0;

  lAllowEmptyLists;

  *ppList = NULL;

  if ( *pStart == LISTINDICATOR )
  {
    // load list of files
    FILE *hfList = NULL;

    //ULONG ulLen = 0;
    //if ( UtlLoadFileL( pStart + 1, (PVOID *)ppList, &ulLen, FALSE, FALSE ) ) 
    //{
    hfList = fopen( pStart + 1, "rb" );
    if ( hfList != NULL )
    {
      int iLen = _filelength( fileno(hfList) ) + 10;

      if ( UtlAllocHwnd( (PVOID *)ppList, 0, iLen+10, ERROR_STORAGE, pData->hwndErrMsg ) )
      {
        fread( *ppList, 1, iLen, hfList );
      }
      else
      {
        usRC = pData->usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
      fclose( hfList );
    }
    else
    {
      PSZ pszParm = EQFCmdList[cmd].szDesc;

      usRC = pData->usRC = DDE_ERRFILELIST;
      UtlErrorHwnd( pData->usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
    } /* endif */
  }
  else
  {
    // make a working copy of the list 
    int iLen = strlen(pStart);
    if ( UtlAllocHwnd( (PVOID *)ppList, 0, iLen+20, ERROR_STORAGE, pData->hwndErrMsg ) )
    {
      // get string from original command line
      int iOffs = pStart - pData->szCmdLine;
      char chTemp = pData->szOrgCmdLine[iOffs+iLen];
      pData->szOrgCmdLine[iOffs+iLen] = EOS;
      strcpy( *ppList, pData->szOrgCmdLine + iOffs );
      pData->szOrgCmdLine[iOffs+iLen] = chTemp;
    }
    else
    {
      usRC = pData->usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // convert list into a comma seperated list, keep double quotes if necessary
  if ( !usRC )
  {
    PSZ pszSource = *ppList;
    PSZ pszTarget = *ppList;
    int iDocuments = 0;
    char chListEndDelimiter = EOS;        

    // skip leading whitespace
    while ( isspace(*pszSource) ) pszSource++;

    // skip any list delimiter
    if ( *pszSource == '(' ) 
    {
      chListEndDelimiter = ')';
      pszSource++;
    }
    else if ( *pszSource == '[' )
    {
      chListEndDelimiter = ']';
      pszSource++;
    }
    else if ( *pszSource == '{' )
    {
      chListEndDelimiter = '}';
      pszSource++;
    }

    // while not end of list
    while ( *pszSource && (*pszSource != chListEndDelimiter) && (*pszSource != 0x1a) )
    {
      // skip leading whitespace
      while ( isspace(*pszSource) ) pszSource++;

      if ( *pszSource && (*pszSource != chListEndDelimiter) && (*pszSource != 0x1a) )
      {
        // add name delimiter if not the first document in list
        if ( iDocuments != 0 )
        {
          *pszTarget++ = ',';
        } /* endif */
      } /* endif */

      // copy document name
      if ( *pszSource == '\"' )
      {
        // check if name contains a comma
        PSZ pszTemp = pszSource + 1;
        BOOL fDoubleQuotesNeeded = FALSE;
        while ( !fDoubleQuotesNeeded && *pszTemp && (*pszTemp != '\"') )
        {
          if ( *pszTemp++ == ',' ) fDoubleQuotesNeeded = TRUE;
        } /*endwhile */

        // copy name up to closing doublequote
        if ( fDoubleQuotesNeeded ) *pszTarget++ = *pszSource;
        pszSource++;
        while ( *pszSource && (*pszSource != '\"') )
        {
          *pszTarget++ = *pszSource++;
        } /*endwhile */

        // skip ending doublequote
        if ( *pszSource == '\"' ) 
        {
          if ( fDoubleQuotesNeeded ) *pszTarget++ = *pszSource;
          pszSource++;
        } /* endif */           

        // skip whitespace
        while ( isspace(*pszSource) ) pszSource++;

        // skip end delimiter  
        if ( *pszSource == ',' ) pszSource++;

        iDocuments++;
      }
      else
      {
        // copy name up to comma, linefeed or list end delimiter
        while ( *pszSource && (*pszSource != ',') && (*pszSource != '\n') && (*pszSource != chListEndDelimiter) )
        {
          *pszTarget++ = *pszSource++;
        } /*endwhile */

        // remove any trailing whitespace of document name
        while ( (pszTarget > *ppList) && isspace( *(pszTarget - 1)) )
        {
          pszTarget--;
        } /*endwhile */

        // skip end delimiter  
        if ( *pszSource == ',' ) pszSource++;

        iDocuments++;
      } /* endif */

      // skip trailing whitespace
      while ( isspace(*pszSource) ) pszSource++;
    } /*endwhile */

    // terminate list
    *pszTarget = EOS;
  } /* endif */

  return( usRC );
} /* end of function BatchNameList */


void BatchHandleAPIError( PBATCHDATA pData, USHORT usRC )
{
  pData; usRC;
}

static BOOL BatchStripDuplSlash
(
   PSZ    pStart
)
{
  PSZ    pIn, pOut;
  CHAR   c;
  BOOL   fStripped = FALSE;

  pIn = pOut = pStart;
  while ( (c=*pOut++ = *pIn++) != NULC )
  {
    if ( (c == '/') && ((*pIn ) == '/') )
    {
      pIn++;                           // skip dupl. slash
      fStripped = TRUE;
    } /* endif */
  } /* endwhile */

  return(fStripped);
} /* end of function BatchStripDuplSlash*/

VOID BatchBothStripDuplSlash
(
   PBATCHDATA  pData,
   PSZ         pStart,
   PSZ         pActive
)
{
   if (BatchStripDuplSlash(pStart))
   {
     PSZ     pszOrgStart = pData->szOrgCmdLine + (pStart - pData->szCmdLine);
     PSZ     pszOrgActive = pData->szOrgCmdLine + (pActive - pData->szCmdLine);
     CHAR    c = *pszOrgActive;

     *pszOrgActive = EOS;             // temporary
     BatchStripDuplSlash( pszOrgStart);
     *pszOrgActive = c;
   } /* endif */
  return;
} /* end of function BatchBothStripDuplSlash*/

BATCHTASK BatchValidateTask
(
  PBATCHDATA pData             //ptr to client ida
)
{
  PTASKLIST  pEQFTaskList;          //ptr to list with tasks / batchlists
  BOOL  fFound = FALSE;             //success indicator
  PSZ   pszTask;                    //temp ptr to cmdline
  PSZ   pTaskStr;                   //ptr where task found in cmdline
  BATCHTASK  TaskCmd = (BATCHTASK)0;           //task identifier to be returned
  USHORT  usLen;
  USHORT usRC = 0;

  pEQFTaskList = EQFTaskList;

  /* find task of this request                                        */
  pszTask = EQFCmdList[BATCH_TASK].szDesc;
  pTaskStr = strstr( pData->szCmdLine, pszTask );
  if ( !pTaskStr )
  {
    pszTask = EQFCmdList[BATCH_TASK].szShortCut;
    pTaskStr = strstr( pData->szCmdLine, pszTask );
  } /* endif */
  if ( pTaskStr )
  {
    pszTask = pTaskStr + strlen(pszTask);
    while ( !fFound && ( pEQFTaskList->TaskCmd != TASK_END) )
    {
      usLen = (USHORT)strlen(pEQFTaskList->szDesc);
      if ( (!strncmp( pEQFTaskList->szDesc, pszTask, usLen)) && ( (pszTask[usLen] == '/') || (pszTask[usLen] == ' '))  )
      {
        fFound = TRUE;
        TaskCmd = pEQFTaskList->TaskCmd;
      }
      else
      {
        pEQFTaskList ++;
      } /* endif */
    } /* endwhile */

    if ( !fFound )
    {
      PSZ pszParm = pData->szMsgBuffer;
      usRC = pData->usRC = BATCH_WRONGTASK;
      BatchMakeTaskList( pData->szMsgBuffer );
      UtlErrorHwnd( pData->usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );

      TaskCmd = TASK_END;
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* error no task specified                                        */
    /******************************************************************/
    PSZ pszParm = pData->szMsgBuffer;
    pData->usRC = BATCH_NOTASK;
    BatchMakeTaskList( pData->szMsgBuffer );
    UtlErrorHwnd( pData->usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
    TaskCmd = TASK_END;
  } /* endif */
  return (TaskCmd);
} /* end of function BatchValidateTask */

// make list of known tasks
VOID BatchMakeTaskList( PSZ pszBuffer )
{
  PTASKLIST pTask = EQFTaskList;

  *pszBuffer = EOS;
  while ( pTask->TaskCmd != TASK_END )
  {
    strcat( pszBuffer, pTask->szDesc );
    pTask++;
    if ( pTask->TaskCmd != TASK_END )
      strcat( pszBuffer, ", " );
  } /*endwhile */
} /* end of function BatchMakeTaskList */

// make list of allowed options
VOID BatchMakeOptionList( PBATCHOPTION pOption, PSZ pszBuffer )
{
  *pszBuffer = EOS;
  while ( pOption->szName[0] )
  {
    strcat( pszBuffer, pOption->szName );
    pOption++;
    if ( pOption->szName[0] )
      strcat( pszBuffer, ", " );
  } /*endwhile */
} /* end of function BatchMakeOptionList */

// make list of allowed parameters
VOID BatchMakeParmList( PBATCHPARM pParm, PSZ pszBuffer )
{
  *pszBuffer = EOS;
  while ( pParm->Type != DUMMY_PARMTYPE )
  {
    strcat( pszBuffer, EQFCmdList[pParm->Cmd].szDesc );
    pParm++;
    if ( pParm->Type != DUMMY_PARMTYPE )
      strcat( pszBuffer, ", " );
  } /*endwhile */
} /* end of function BatchMakeOptionList */

USHORT BatchDriveParm( PBATCHDATA pData, PSZ pStart, CHAR *pchDrive, BATCHCMD cmd )
{
  USHORT usRC = 0;

  if ( strlen( pStart ) == 1 )
  {
    *pchDrive = (CHAR)toupper(*pStart);
  }
  else
  {
    usRC = pData->usRC = DDE_WRONGCMDLINE;
    {
      PSZ pszParm = EQFCmdList[cmd].szDesc;
      UtlErrorHwnd( pData->usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
    } /* endif */
  } /* endif */
  return( usRC );
} /* end of function BatchDriveParm */

USHORT BatchConnectSharedMem(PBATCHDATA pData )
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, ConnectSharedMemParms );
  if ( !usRC )
  {
    usRC = EqfConnectSharedMem( pData->hSession,Parm.chFromDrive,Parm.szMemory);
    if ( usRC ) BatchHandleAPIError( pData, usRC );
  }

  BatchCleanParms( pData );

  return( usRC );
} 

USHORT BatchDisconnectSharedMem( PBATCHDATA pData )
{
  USHORT usRC = 0;

  usRC = BatchGetParameters( pData, ConnectSharedMemParms );
  if ( !usRC )
  {
    usRC = EqfDisconnectSharedMem( pData->hSession,Parm.chFromDrive,Parm.szMemory);
    if ( usRC ) BatchHandleAPIError( pData, usRC );
  }

  BatchCleanParms( pData );

  return( usRC );
} 

void showHelp()
{
    printf( "OtmCmd.EXE       : OpenTM2 command line\n" );
    printf( "Version          : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright        : %s\n",STR_COPYRIGHT );
    printf( "Purpose          : Execute OpenTM2 from command line\n" );
    printf( "Syntax format    : \n");
    printf("                    OtmCmd /TASK=Analyse /FLD=fold [/Files=document(s) name] [/Options=[ADDTOMEM|AUTO|AUTOCONTEXT|AUTOLAST|AUTOJOIN|TMMATCH|UNTRANSLATED|ADJUSTLEADWS|ADJUSTTRAILWS|PROTECTXMPSCREEN|RESPECTCRLF]] [/MEM=memdb]\n" );
    printf("\n");
    printf( "                     or\n");
    
    printf("                    OtmCmd /TASK=DOCEXP /FLD=fold /Files=doc(s) [/OPtions=[TARGET|SOURCE|SNOMATCH]] [/OVerwrite=[Yes|No]] [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");
    
    printf("                    OtmCmd /TASK=DOCIMP /FLD=fold /Files=doc(s) [/STartpath=startpath] [/ALIAS=alias] [/OVerwrite=[Yes|No]] [/QUIET] [/EDit=editor] [/MArkup=markup] \n");
    printf("\n");
    printf( "                     or\n");
    
    printf("                    OtmCmd /TASK=DOCDEL /FLD=fold /Files=doc(s) [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");
    
    printf("                    OtmCmd /TASK=FLDCRT /Name=folder [/DEsc=desc] [/TOdrive=drive] /MEm=memdb /MArkup=markuptable [/EDit=editor] /SRclng=source /TGtlng=target [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");
    
    printf("                    OtmCmd /TASK=FLDCRT /Name=folder [/DEsc=desc] [/TOdrive=drive] /MEm=memdb /MArkup=markuptable [/EDit=editor] [/Dict=dict] /SRclng=source /TGtlng=target [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");
    
    printf("                    OtmCmd /TASK=FLDEXP /FLD=folder [/FRomdrive=drive] [/FromPath=path] /TOdrive=drive [/Files=doc(s)] [/OPtions=[DICT|MEM|ROMEM|DOCMEM|DELETE]] [/OVerwrite=[Yes|No]] [/QUIET]\n");
    printf( "                   \n or\n");
    
    printf("                    OtmCmd /TASK=FLDIMP /FLD=folder /FRomdrive=drive [/FromPath=path] [/TOdrive=drive] [/OPtions=[DICT|MEM]] [/EDit=editor] [/QUIET] [/MArkup=markup] [/IMPORTAS=newname]\n");
    printf("\n");
    printf( "                     or\n");
    
    printf("                    OtmCmd /TASK=FLDDEL /FLD=folder [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");
    
    printf("                    OtmCmd /TASK=MEMCRT /NAme=memdb [/DEsc=desc] [/TYpe=[LOCAL|SHARED]] /TOdrive=drive /SRclng=source [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");

    printf("                    OtmCmd /TASK=MEMDEL MEm=memdb [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");

    printf("                    OtmCmd /TASK=MEMEXP /OUT=mem_file [/TYpe=EXTERNAL] /MEm=memdb [/OVerwrite=[Yes|No]] [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");

    printf("                    OtmCmd /TASK=MEMIMP /OUT=mem_file [/TYpe=EXTERNAL] /MEm=memdb [/OVerwrite=[Yes|No]] [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");

    printf("                    OtmCmd /TASK=DICEXP /DIct=dic_name /OUT=mem_file[/OVerwrite=[Yes|No]] [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");

    printf("                    OtmCmd /TASK=DICIMP /FIles=dict_file /DIct=dict_name [/OPtions=[COMBINE|REPLACE|IGNORE]] [/PAssword=password] [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");

    printf("                    OtmCmd /TASK=ARCHTM /FLD=folder [/FIles=doc(s)] /MEm=memdb [/OVerwrite=[Yes|No]] [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");

    printf("                    OtmCmd /TASK=MEMORG /MEm=memdb [/OVerwrite=[Yes|No]] [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");

    printf("                    OtmCmd /TASK=DOCOPEN /FLD=folder /NAme=document_name [/SEgment=segnumber] [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");

    printf("                    OtmCmd /TASK=WORDCNT /FLD=folder [FIles=doc[s]] /OUT=cnt_name [/OVerwrite=[Yes|No]] [/OPtions=[SOURCE|TARGET|TMMATCH]] [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");

    printf("                    OtmCmd /TASK=CNTRPT /FLD=folder [FIles=doc[s]]  /OUT=output_name /REport=[HISTORY|COUNTING|CALCULATING|PREANALYSIS|REDUNDANCY|REDSEGLIST] /TYpe=[DATE|BRIEF|DETAIL|WITH_TOTALS|WITHOUT_TOTALS|BASE|BASE_SUMMARY|BASE_SUMMARY_FACT|SUMMARY_FACT|FACT][/OVerwrite=[Yes|No]] [/PRofile=profile][/FOrmat=[XML|ASCII|HTML]] [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");

	printf("                    OtmCmd /TASK=CONNECT|DISCONNECT /FRomdrive=drive /MEm=memdb  [/QUIET]\n");
    printf("\n");
    printf( "                     or\n");

    printf( "Options and parameters:\n" );
    printf( "    /TASK         specifies what want to do\n" );
    printf( "    /FLD          the name of a folder that contains the documents\n" );
    printf( "    /FIles        name of document\n" );
    printf( "    /MEm          translation Memory or a list of Translation Memory databases \n" );
    printf( "    /STartpath    the drive, directory, or both where exported is to be placed.\n" );
    printf( "    /ALIAS        an alias name for the document to be processed\n" );
    printf( "    /EDit         Specifies the editor to be used\n" );
    printf( "    /MArkup       Specifies the markup table to be used\n" );
    printf( "    /IMPORTAS     Specifies the new name of the folder after the import\n" );
    printf( "    /DEsc         specifies the description of the new object\n" );
    printf( "    /TOdrive      specifies the drive where the new object is to be placed\n" );
    printf( "    /DIct         specifies the name of a dictionary to be associated with\n" );
    printf( "    /SRclng       the source language\n" );
    printf( "    /TGtlng       the target language\n" );
    printf( "    /TYpe         specifies the object type\n" );
    printf( "    /OVerwrite    Specifies if an existing object will be overwritten \n" );
    printf( "    /QUIET        If you specify this parameter, you are not prompted with any message window. \n" );
}
