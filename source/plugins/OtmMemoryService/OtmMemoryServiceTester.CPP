//+----------------------------------------------------------------------------+
//| OpenTM2Scripter.C                                                          |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Batch interface using the TM API                              |
//|                                                                            |
//|  Test tool for the OpenTM2 API                                             |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//

#include <time.h>
#include <stdarg.h>
#include <direct.h>
#include "EQF.H"
#include "OTMFUNC.H"
#include "EQFFUNCI.H"
#include "EQFTM.H" //for Translation Memory Wild Cards
#include <windows.h>
#include "core\PluginManager\PluginManager.h"



// name of default script file
#define DEFAULT_SCRIPT "OpenTM2.SCR"

// several constants used in the following code
#define TRUE  1
#define FALSE 0
#define NULC '\0'
#define WRONG_OPT 255

// some macros ...
#define SKIPSPACE(p)  while (*p == ' '  || *p == 9) p++
#define SKIPCOMMA(p)  if (*p == ',' ) p++

char szMsg[8096];                       // buffer for TranslationManager messages
char szScriptFile[1024];                // name of script file
char szLine[2048];                      // buffer for a single line of the script
char szTempLine[2048];                  // temporary buffer for a single line of the script
char szTempScriptFile[100];             // temporary script file
long lOption;                           // buffer for option parameters
char chNullParm = NULC;                 // empty character parameter
EXTFOLPROP FolProps;                    // buffer for folder properties
// buffer for single folder property values, the largest value will be the list of RO-Memories can reach MAX_NUM_OF_READONLY_MDB * MAX_LONGFILESPEC
char szFolPropValue[MAX_NUM_OF_READONLY_MDB*MAX_LONGFILESPEC]; 


// enumeration of symbolic command IDs
typedef enum _COMMANDID
{
    INTCOMPWC_ID,
    INTFOR_ID,
    INTENDFOR_ID,
    INTCOMPBINARY_ID,
    INTINCREMENT_ID,
    INTDECREMENT_ID,
    INTSAVERETURNCODE_ID,
    INTADD_ID,
    INTSUB_ID,
    INTTESTCASE_ID,
    INTEND_ID,
    INTTESTRESULT_ID,
	INTTESTVALUE_ID,
    INTDELETEFILE_ID,
    INTCOPYFILE_ID,
    INTSETVALUE_ID,
    INTINCLUDE_ID,
    INTECHO_ID,
    INTIF_ID,
    INTELSEIF_ID,
    INTELSE_ID,
    INTENDIF_ID,
    INTDEFINE_ID,
    INTGOTO_ID,
    INTGOTOCOND_ID,
    INTMARKER_ID,  //For marker with 'marker'-command
    EQFCREATEMEM_ID,
    EQFCREATEFOLDER_ID,
    EQFEXPORTFOLDER_ID,
    EQFEXPORTFOLDERFP_ID,
    EQFCOUNTWORDS_ID,
    EQFIMPORTDICT_ID,
    EQFEXPORTDICT_ID,
    EQFIMPORTDOC_ID,
    EQFEXPORTDOC_ID,
    EQFIMPORTMEM_ID,
    EQFEXPORTMEM_ID,
    EQFANALYZEDOC_ID,
    EQFDELETEMEM_ID,
    EQFIMPORTFOLDER_ID,
    EQFIMPORTFOLDERFP_ID,
    EQFDELETEFOLDER_ID,
    EQFDELETEDOC_ID,
    EQFORGANIZEMEM_ID,
    EQFCHANGEFOLPROPS_ID,
    EQFCREATESUBFOLDER_ID,
    EQFARCHIVETM_ID,
    EQFCREATECONTROLLEDFOLDER_ID,
    EQFCHANGEMFLAG_ID,
    EQFCREATECNTREPORT_ID,
    EQFSETSYSLANGUAGE_ID,
    EQFGETSYSLANGUAGE_ID,
    EQFLOADSEGFILE_ID,
    EQFBUILDSEGDOCNAME_ID,
    EQFGETSEGW_ID,
    EQFGETSOURCELINE_ID,
    EQFGETSEGMENTNUMBER_ID,
    SLEEP_ID,
    EQFEXPORTFOLDERFPAS_ID,
    EQFCLEANMEMORY_ID,
    EQFCREATECOUNTREPORT_ID,
    EQFRENAME_ID,
    EQFANALYZEDOCEX_ID,
    EQFPROCESSNOMATCH_ID,
    EQFOPENDOC_ID,
    EQFCHANGEFOLPROPSEX_ID,
    EQFDELETEMTLOG_ID,
  EQFGETSHORTNAME_ID,
  EQFREMOVEDOCS_ID,
  EQFRESTOREDOCS_ID,
  EQFPROCESSNOMATCHEX_ID,
  EQFADDCTIDLIST_ID,
  EQFEXPORTSEGS_ID,
  EQFGETFOLDERPROP_ID,
  EQFFOLDEREXISTS_ID,
  EQFMEMORYEXISTS_ID,
  EQFDICTIONARYEXISTS_ID,
  EQFDOCUMENTEXISTS_ID,
  EQFCREATEITM_ID,
  EQFCOUNTWORDWSINSTRING_ID,
  EQFCHECKSPELLING_ID,
  EQFREDUCETOSTEMFORM_ID,
  EQFCLEARMTFLAG_ID,
  EQFFILTERNOMATCHFILE_ID,
  EQFDELETEDICT_ID,
  EQFGETVERSIONEX_ID,
  EQFGETFOLDERPROPEX_ID,
  EQFOPENDOCBYTRACK_ID,
  EQFIMPORTMEMEX_ID,
  EQFADDMATCHSEGID_ID,
  EQFCREATECOUNTREPORTEX_ID,
  EQFIMPORTFOLDERAS_ID,
  EQFSEARCHFUZZYSEGMENTS_ID
} COMMANDID;

typedef enum _LOGLEVEL
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_NO
}LOGLEVEL;

//List maximum for Wildcard paths
#define WCPATH_LIST_MAX 1024

// structure for saving wildcardpaths
typedef struct _WCPATH
{
    char path[WCPATH_LIST_MAX];
    struct _WCPATH *next;
}WCPATH, *PWCPATH;

// structure for saving markers
typedef struct _MARKER
{
    char name[51];
    USHORT ln;
    fpos_t tmpScript;
    struct _MARKER *next;
} MARKER, *PMARKER;

//Structure for saving constants
typedef struct _CONSTA
{
    CHAR name[51];
    CHAR value[512];
    struct _CONSTA *next;
} CONSTA,*PCONSTA;

//Structure for saving variables
typedef union _Value
{
    int    iVal;
    char*  pszVal;
}Value;

typedef enum _Type
{
    INTEGER,
    POINTERSTR
}Type;

typedef struct _VARIABLE
{
    CHAR  name[51];
    Value value;
    Type  type;
    struct _VARIABLE *next;
} VARIABLE,*PVARIABLE;

//Structure for integerlist (Required for if-handling)
typedef struct _IFLIST{
    struct _IFLIST *next;
    int id;
    int lastCond;
} IFLIST,*PIFLIST;

typedef struct _FORLIST{
    struct _FORLIST *next;
    int id;
    char action;
    int actionvalue;
    char varname[51];
} FORLIST,*PFORLIST;

//Structure for filelist (includes)
typedef struct _INCLIST{
    struct _INCLIST *next;
    FILE *filepointer;
    char path[1024];
    char name[128];
} INCLIST,*PINCLIST;

// structure for script commands
typedef struct _COMMAND
{
    char   szCommand[40];            // command string
    COMMANDID ID;                    // symbolic command ID
    int    iNumOfParms;                // number of paramters
    char   szParmFlags[100];        // parameter flags
    char   szWCFlags[100];            // wildcard flags
} COMMAND, *PCOMMAND;

// defines for parameter flags
#define NO_PARM            '0'
#define OPTION_PARM        '1'
#define CHARACTER_PARM    '2'
#define STRING_PARM        '3'
#define FLOAT_PARM        '4'
#define LONG_PARM        '5'
#define SHORT_PARM        '6'
#define BOOL_PARM        '7'

//szWCFlags :
// '1' - absolute file path lists - e.g. EQFIMPORTDOC ,list_of_files,...
// '2' - absolute file paths without lists - not used yet
// '3' - relative file path lists - e.g. EQFDELETEDOC ,list_of_documents,...
// '4' - relative folder names - e.g. EQFDELETEFOLDER folder
// '5' - relative memory names - e.g. EQFDELETEMEM
// '6' - relative list of read-only-memories
// - ATTENTION - USE THIS TYPE ONLY WITH TYPE '7' -
// e.g. : EQFCREATEFOLDER ..., list_of_dictionaries, ... , LIST_OF_MEMORIES,...
// '7' - relative list of dictionaries
// - ATTENTION - USE THIS TYPE ONLY WITH TYPE '6' -
// e.g. : EQFCREATEFOLDER ..., LIST_OF_DICTIONARIES, ... , list_of_memories,...
// table with valid commands (last entry must be an empty one)

COMMAND aCommands[] =
{ // comand name        ID                   parms   parameter flags
    { "COMPAREWORDCOUNT",            INTCOMPWC_ID,                     2, "330000000000000000", "000000000000000000"},
    { "ENDFOR",                     INTENDFOR_ID,                      0, "000000000000000000", "000000000000000000"},
    { "FOR",                        INTFOR_ID,                         4, "353300000000000000", "000000000000000000"},
    { "COMPAREBINARY",              INTCOMPBINARY_ID,                  2, "330000000000000000", "000000000000000000"},
    { "INC",                        INTINCREMENT_ID,                   1, "300000000000000000", "000000000000000000"},
    { "DEC",                        INTDECREMENT_ID,                   1, "300000000000000000", "000000000000000000"},
    { "ADD",                        INTADD_ID,                         2, "350000000000000000", "000000000000000000"},
    { "SUB",                        INTSUB_ID,                         2, "350000000000000000", "000000000000000000"},
    { "TESTCASE",                   INTTESTCASE_ID,                    2, "330000000000000000", "000000000000000000"},
    { "END",                        INTEND_ID,                         0, "000000000000000000", "000000000000000000"},
    { "TESTRESULT",                 INTTESTRESULT_ID,                  2, "530000000000000000", "000000000000000000"},
	{ "TESTVALUE",                  INTTESTVALUE_ID,                   2, "330000000000000000", "000000000000000000"},
    { "DELETEFILE",                 INTDELETEFILE_ID,                  1, "300000000000000000", "000000000000000000"},
    { "COPYFILE",                   INTCOPYFILE_ID,                    3, "331000000000000000", "000000000000000000"},
    { "SAVERETURNCODE",             INTSAVERETURNCODE_ID,              1, "300000000000000000", "000000000000000000"},
    { "SETVALUE",                   INTSETVALUE_ID,                    1, "500000000000000000", "000000000000000000"},
    { "INCLUDE",                    INTINCLUDE_ID,                     1, "300000000000000000", "000000000000000000"},
    { "ECHO",                       INTECHO_ID,                        1, "300000000000000000", "000000000000000000"},
    { "IF",                         INTIF_ID,                          1, "300000000000000000", "000000000000000000"},
    { "ELSEIF",                     INTELSEIF_ID,                      1, "300000000000000000", "000000000000000000"},
    { "ELSE",                       INTELSE_ID,                        0, "000000000000000000", "000000000000000000"},
    { "ENDIF",                        INTENDIF_ID,                     0, "000000000000000000", "000000000000000000"},
    { "DEFINE",                        INTDEFINE_ID,                     2, "330000000000000000", "000000000000000000"},
    { "GOTO",                        INTGOTO_ID,                         1, "300000000000000000", "000000000000000000"},
    { "GOTOCOND",                    INTGOTOCOND_ID,                     2, "330000000000000000", "000000000000000000"},
    { "MARKER",                        INTMARKER_ID,                     1, "300000000000000000", "000000000000000000"},
    { "#",                            INTMARKER_ID,                     1, "300000000000000000", "000000000000000000"}, //COPY OF MARKER
    { "EQFCREATEMEM",                EQFCREATEMEM_ID,                 5, "332310000000000000", "000000000000000000"},
    { "EQFCREATEFOLDER",            EQFCREATEFOLDER_ID,                11, "332333333330000000", "000000700060000000"},
    { "EQFEXPORTFOLDER",            EQFEXPORTFOLDER_ID,                 5, "321330000000000000", "000300000000000000"},
    { "EQFEXPORTFOLDERFP",            EQFEXPORTFOLDERFP_ID,             5, "331330000000000000", "000300000000000000"},
    { "EQFCOUNTWORDS",                EQFCOUNTWORDS_ID,                 4, "331300000000000000", "030000000000000000"},
    { "EQFIMPORTDICT",                EQFIMPORTDICT_ID,                 4, "333100000000000000", "000000000000000000"},
    { "EQFEXPORTDICT",                EQFEXPORTDICT_ID,                 3, "313000000000000000", "000000000000000000"},
    { "EQFIMPORTDOC",                EQFIMPORTDOC_ID,                11, "333333333310000000", "010000000000000000"},
    { "EQFEXPORTDOC",                EQFEXPORTDOC_ID,                 4, "333100000000000000", "030000000000000000"},
    { "EQFIMPORTMEM",                EQFIMPORTMEM_ID,                 3, "331000000000000000", "020000000000000000"},
    { "EQFEXPORTMEM",                EQFEXPORTMEM_ID,                 3, "331000000000000000", "000000000000000000"},
    { "EQFANALYZEDOC",                EQFANALYZEDOC_ID,                 4, "333100000000000000", "030000000000000000"},
    { "EQFDELETEMEM",                EQFDELETEMEM_ID,                 1, "300000000000000000", "500000000000000000"},
    { "EQFIMPORTFOLDER",            EQFIMPORTFOLDER_ID,                 4, "322100000000000000", "000000000000000000"},
    { "EQFIMPORTFOLDERFP",            EQFIMPORTFOLDERFP_ID,             4, "332100000000000000", "000000000000000000"},
    { "EQFDELETEFOLDER",            EQFDELETEFOLDER_ID,                 1, "300000000000000000", "400000000000000000"},
    { "EQFDELETEDOC",                EQFDELETEDOC_ID,                 2, "330000000000000000", "030000000000000000"},
    { "EQFORGANIZEMEM",                EQFORGANIZEMEM_ID,                 1, "300000000000000000", "500000000000000000"},
    { "EQFARCHIVETM",                    EQFARCHIVETM_ID,                 5, "332310000000000000", "003000000000000000"},
    { "EQFCHANGEFOLPROPS",            EQFCHANGEFOLPROPS_ID,             5, "323330000000000000", "000080000000000000"},
    { "EQFCREATESUBFOLDER",            EQFCREATESUBFOLDER_ID,             10, "333333333300000000", "000000000000000000"},
    { "EQFCHANGEMFLAG",                EQFCHANGEMFLAG_ID,                 2, "310000000000000000", "500000000000000000"},
    { "EQFCREATECONTROLLEDFOLDER",    EQFCREATECONTROLLEDFOLDER_ID,    24, "332333333333333333333333", "000000700060000000000000"},
    { "EQFCREATECNTREPORT",     EQFCREATECNTREPORT_ID,   84, "23331333331131314444444444444444444444444444444444444444444444444444444444446654311", "00300000000000000000000000000000000000000000000000000000000000000000000000000000000" },
    { "EQFSETSYSLANGUAGE",      EQFSETSYSLANGUAGE_ID,    1, "3000000000000", "000000000000000000"},
    { "EQFGETSYSLANGUAGE",      EQFGETSYSLANGUAGE_ID,    0, "0000000000000", "000000000000000000"},
    { "EQFLOADSEGFILE",         EQFLOADSEGFILE_ID,       1, "3000000000000", "000000000000000000"},
    { "EQFBUILDSEGDOCNAME",     EQFBUILDSEGDOCNAME_ID,   3, "3370000000000", "000000000000000000"},
    { "EQFGETSEGW",             EQFGETSEGW_ID,           1, "5000000000000", "000000000000000000"},
    { "EQFGETSOURCELINE",       EQFGETSOURCELINE_ID,     1, "5000000000000", "000000000000000000"},
    { "EQFGETSEGMENTNUMBER",    EQFGETSEGMENTNUMBER_ID,  2, "5500000000000", "000000000000000000"},
    { "SLEEP",                  SLEEP_ID,                1, "6000000000000", "000000000000000000"},
    { "EQFEXPORTFOLDERFPAS",    EQFEXPORTFOLDERFPAS_ID,  7, "3331333000000000000", "000030000000000000"},
    { "EQFCLEANMEMORY",         EQFCLEANMEMORY_ID,       4, "3331000000000000000", "000000000000000000"},
    { "EQFCREATECOUNTREPORT",   EQFCREATECOUNTREPORT_ID, 7, "333333100000000000", "030000000000000000"},
    { "EQFRENAME",              EQFRENAME_ID,            4, "333100000000000000", "000000000000000000"},
    { "EQFANALYZEDOCEX",        EQFANALYZEDOCEX_ID,      6, "333331000000000000", "030000000000000000"},
    { "EQFPROCESSNOMATCH",      EQFPROCESSNOMATCH_ID,    8, "333333310000000000", "000000000000000000"},
    { "EQFOPENDOC",             EQFOPENDOC_ID,           4, "335500000000000000", "000000000000000000"},
    { "EQFCHANGEFOLPROPSEX",    EQFCHANGEFOLPROPSEX_ID,  9, "323333333000000000", "000076000000000000"},
    { "EQFDELETEMTLOG",         EQFDELETEMTLOG_ID,       1, "300000000000000000", "400000000000000000"},
    { "EQFGETSHORTNAME",        EQFGETSHORTNAME_ID,      2, "330000000000000000", "000000000000000000"},
    { "EQFREMOVEDOCS",          EQFREMOVEDOCS_ID,        2, "330000000000000000", "000000000000000000"},
    { "EQFRESTOREDOCS",         EQFRESTOREDOCS_ID,       1, "300000000000000000", "000000000000000000"},
    { "EQFPROCESSNOMATCHEX",    EQFPROCESSNOMATCHEX_ID, 10, "333333333100000000", "000000000000000000"},
    { "EQFADDCTIDLIST",         EQFADDCTIDLIST_ID,       2, "330000000000000000", "000000000000000000"},
    { "EQFEXPORTSEGS",          EQFEXPORTSEGS_ID,        5, "333310000000000000", "000000000000000000"},
    { "EQFGETFOLDERPROP",       EQFGETFOLDERPROP_ID,     2, "330000000000000000", "000000000000000000"},
    { "EQFFOLDEREXISTS",        EQFFOLDEREXISTS_ID,      1, "300000000000000000", "000000000000000000"},
    { "EQFMEMORYEXISTS",        EQFMEMORYEXISTS_ID,      1, "300000000000000000", "000000000000000000"},
    { "EQFDICTIONARYEXISTS",    EQFDICTIONARYEXISTS_ID,  1, "300000000000000000", "000000000000000000"},
    { "EQFDOCUMENTEXISTS",      EQFDOCUMENTEXISTS_ID,    2, "330000000000000000", "000000000000000000"},
    { "EQFCREATEITM",           EQFCREATEITM_ID,         9, "333333331000000000", "000000000000000000"},
    { "EQFCOUNTWORDSINSTRING",  EQFCOUNTWORDWSINSTRING_ID, 3, "333000000000000000", "000000000000000000"},
    { "EQFCHECKSPELLING",       EQFCHECKSPELLING_ID,     5, "333310000000000000", "000000000000000000"},
    { "EQFREDUCETOSTEMFORM",    EQFREDUCETOSTEMFORM_ID,  5, "333310000000000000", "000000000000000000"},
    { "EQFCLEARMTFLAG",         EQFCLEARMTFLAG_ID,       2, "330000000000000000", "000000000000000000"},
    { "EQFFILTERNOMATCHFILE",   EQFFILTERNOMATCHFILE_ID, 7, "333333100000000000", "000000000000000000"},
    { "EQFDELETEDICT",          EQFDELETEDICT_ID,        1, "300000000000000000", "700000000000000000"},
    { "EQFGETVERSIONEX",        EQFGETVERSIONEX_ID,      1, "000000000000000000", "000000000000000000"},
    { "EQFGETFOLDERPROPEX",     EQFGETFOLDERPROPEX_ID,   3, "333000000000000000", "000000000000000000"},
    { "EQFOPENDOCBYTRACK",      EQFOPENDOCBYTRACK_ID,    2, "330000000000000000", "000000000000000000"},
    { "EQFIMPORTMEMEX",         EQFIMPORTMEMEX_ID,       7, "333333100000000000", "020000000000000000"},
    { "EQFADDMATCHSEGID",       EQFADDMATCHSEGID_ID,     4, "333100000000000",    "000000000000000000"},
    { "EQFCREATECOUNTREPORTEX", EQFCREATECOUNTREPORTEX_ID, 10, "333333333100000000", "030000000000000000"},
    { "EQFIMPORTFOLDERAS",      EQFIMPORTFOLDERAS_ID,    5, "332310000000000000", "000000000000000000"},
    { "EQFSEARCHFUZZYSEGMENTS", EQFSEARCHFUZZYSEGMENTS_ID, 6, "333331000000000000", "030000000000000000"},
    { "",                       (COMMANDID)0,            0, "000000000000000000", "000000000000000000"}
};

// structure for option values
typedef struct _OPTION
{
    char   szOption[40];                 // option string
    long   lOption;                      // value of option
} OPTION, *POPTION;

// table with valid options (last entry must be an empty one)
OPTION aOptions[] =
{
    { "LOCAL_OPT",                   LOCAL_OPT},
    { "SHARED_OPT",                  SHARED_OPT},
    { "TMMATCH_OPT",                 TMMATCH_OPT},
    { "ADDTOMEM_OPT",                ADDTOMEM_OPT},
    { "ADJUSTREFERENCES_OPT",        ADJUSTREFERENCES_OPT },
    { "AUTOSUBST_OPT",               AUTOSUBST_OPT},
    { "UNTRANSLATED_OPT",            UNTRANSLATED_OPT},
    { "AUTOLAST_OPT",                AUTOLAST_OPT},
    { "AUTOJOIN_OPT",                AUTOJOIN_OPT},
    { "AUTOCONTEXT_OPT",             AUTOCONTEXT_OPT},
    { "REDUNDCOUNT_OPT",             REDUNDCOUNT_OPT},
    { "WITHDICT_OPT",                WITHDICT_OPT},
    { "WITHMEM_OPT",                 WITHMEM_OPT},
    { "DELETE_OPT",                  DELETE_OPT},
    { "WITHREADONLYMEM_OPT",         WITHREADONLYMEM_OPT},
    { "WITHDOCMEM_OPT",              WITHDOCMEM_OPT},
    { "SOURCE_OPT",                  SOURCE_OPT},
    { "TARGET_OPT",                  TARGET_OPT},
    { "SNOMATCH_OPT",                SNOMATCH_OPT},
    { "IGNORE_OPT",                  IGNORE_OPT},
    { "REPLACE_OPT",                 REPLACE_OPT},
    { "COMBINE_OPT",                 COMBINE_OPT},
    { "EXTERNAL_OPT",                EXTERNAL_OPT},
    { "INTERNAL_OPT",                INTERNAL_OPT},
    { "OVERWRITE_OPT",               OVERWRITE_OPT},
    { "IGNOREPATH_OPT",              IGNOREPATH_OPT},
    { "USEASFOLDERTM_OPT",           USEASFOLDERTM_OPT},
    { "CLEAR_MMOPT",                 CLEAR_MMOPT},
    { "SET_MMOPT",                   SET_MMOPT},
    { "NOANA_TYP",                   NOANA_TYP},
    { "NOTM_TYP",                    NOTM_TYP},
    { "PREPARE_TYP",                 PREPARE_TYP},
    { "PLAUS_OPT",                   PLAUS_OPT },        // Plausibilty check
    { "LOST_OPT",                    LOST_OPT },         // Lost Data: Force new shipment
    { "LIST_OPT",                    LIST_OPT },         // List of Documents
    { "BASE_TYP",                    BASE_TYP },
    { "FACT_TYP",                    FACT_TYP },
    { "SUM_TYP",                     SUM_TYP },
    { "TRUE",                        TRUE },
    { "FALSE",                       FALSE },
    { "ASCII_OPT",                   ASCII_OPT },
    { "ANSI_OPT",                    ANSI_OPT },
    { "UTF16_OPT",                   UTF16_OPT },
    { "WRONG_OPT",                   WRONG_OPT },
    { "SETMFLAG_OPT",                SETMFLAG_OPT },
    { "SOURCESOURCEMEM_OPT",         SOURCESOURCEMEM_OPT },
    { "ADJUSTLEADWS_OPT",            ADJUSTLEADWS_OPT },
    { "ADJUSTTRAILWS_OPT",           ADJUSTTRAILWS_OPT },
    { "RESPECTCRLF_OPT",             RESPECTCRLF_OPT },
    { "TMX_OPT",                     TMX_OPT },
    { "TMX_UTF16_OPT",               TMX_UTF16_OPT },
    { "TMX_UTF8_OPT",                TMX_UTF8_OPT },
    { "VALFORMAT_XML_OPT",           VALFORMAT_XML_OPT },
    { "VALFORMAT_HTML_OPT",          VALFORMAT_HTML_OPT },
    { "VALFORMAT_DOC_OPT",           VALFORMAT_DOC_OPT },
    { "VALFORMAT_ODT_OPT",           VALFORMAT_ODT_OPT },
    { "VALFORMAT_COMBINE_OPT",       VALFORMAT_COMBINE_OPT },
    { "VALFORMAT_PROTSEGS_OPT",      VALFORMAT_PROTSEGS_OPT },
    { "CLEANMEM_INTERNAL_MEMORY_OPT", CLEANMEM_INTERNAL_MEMORY_OPT },
    { "CLEANMEM_EXTERNAL_MEMORY_OPT", CLEANMEM_EXTERNAL_MEMORY_OPT },
    { "CLEANMEM_COMPLETE_IN_ONE_CALL_OPT", CLEANMEM_COMPLETE_IN_ONE_CALL_OPT },
    { "CLEANMEM_LOGGING_OPT",        CLEANMEM_LOGGING_OPT },
    { "CLEANMEM_BESTMATCH_OPT",      CLEANMEM_BESTMATCH_OPT },
    { "TEXT_OUTPUT_OPT",             TEXT_OUTPUT_OPT },
    { "XML_OUTPUT_OPT",              XML_OUTPUT_OPT },
    { "HTML_OUTPUT_OPT",             HTML_OUTPUT_OPT },
    { "DUPLICATE_OPT",               DUPLICATE_OPT },
    { "CLEANMEM_MERGE_OPT",          CLEANMEM_MERGE_OPT },
    { "MASTERFOLDER_OPT",            MASTERFOLDER_OPT },
    { "XLIFF_OPT",                   XLIFF_OPT },
    { "NOBLANKATSEGEND_OPT",         NOBLANKATSEGEND_OPT },
    { "NOSUBSTIFIDENTICAL_OPT",      NOSUBSTIFIDENTICAL_OPT },
    { "CLEANMEM_KEEP_DUPS_OPT",      CLEANMEM_KEEP_DUPS_OPT },
    { "DUPLMEMMATCH_OPT",            DUPMEMMATCH_OPT },
    { "PROTECTXMPSCREEN_OPT",        PROTECTXMPSCREEN_OPT },
    { "CLEANRTF_OPT",                CLEANRTF_OPT },
    { "NOMARKUP_UPDATE_OPT",         NOMARKUP_UPDATE_OPT },
    { "SENDTOMT_OPT",                SENDTOMT_OPT },
    { "STOPATFIRSTEXACT_OPT",        STOPATFIRSTEXACT_OPT},
    { "IGNORECOMMENTED_OPT",         IGNORECOMMENTED_OPT},
    { "INCURLYBRACE_OPT",            INCURLYBRACE_OPT },
    { "NOSUBDIRSEARCH_OPT",          NOSUBDIRSEARCH_OPT },
    { "PLAINXML_OPT",                PLAINXML_OPT },
    { "PROTECTXMP_OPT",              PROTECTXMP_OPT },
    { "PROTECTMSGNUM_OPT",           PROTECTMSGNUM_OPT },
    { "PROTECTMETA_OPT",             PROTECTMETA_OPT }, 
    { "PROTECTSCREEN_OPT",           PROTECTSCREEN_OPT },
    { "PROTECTCODEBLOCK_OPT",        PROTECTCODEBLOCK_OPT },
    { "DXT_UTF8_OPT",                DXT_UTF8_OPT },
    { "WITHRELATIVEPATH_OPT",        WITHRELATIVEPATH_OPT },
    { "WITHOUTRELATIVEPATH_OPT",     WITHOUTRELATIVEPATH_OPT },
    { "OPENTM2FORMAT_OPT",           OPENTM2FORMAT_OPT },
    { "WITHTRACKID_OPT",             WITHTRACKID_OPT },
    { "FORCENEWMATCHID_OPT",         FORCENEWMATCHID_OPT },
    { "CANCEL_UNKNOWN_MARKUP_OPT",   CANCEL_UNKNOWN_MARKUP_OPT },
    { "SKIP_UNKNOWN_MARKUP_OPT",     SKIP_UNKNOWN_MARKUP_OPT },
    { "GENERIC_UNKNOWN_MARKUP_OPT",  GENERIC_UNKNOWN_MARKUP_OPT },
    { "",                            0L}
};

// object types for rename function
OPTION aRenameObjects[] =
{
    { "RENAME_FOLDER",            RENAME_FOLDER },
    { "RENAME_MEMORY",            RENAME_MEMORY },
    { "RENAME_DICTIONARY",        RENAME_DICTIONARY },
    { "",                         0L}
};

// object types for short name function
OPTION aShortNameObjects[] =
{ { "FOLDER_OBJ",            FOLDER_OBJ },
  { "MEMORY_OBJ",            MEMORY_OBJ },
  { "DICT_OBJ",              DICT_OBJ },
  { "DOCUMENT_OBJ",          DOCUMENT_OBJ },
  { "",                      0L}};

// reports
OPTION aReports[] =
{
    { "HISTORY_REP",            HISTORY_REP },
    { "COUNTING_REP",           COUNTING_REP },
    { "CALCULATING_REP",        CALCULATING_REP },
    { "PREANALYSIS_REP",        PREANALYSIS_REP },
    { "REDUNDANCY_REP",         REDUNDANCY_REP },
    { "REDUNDANCYSEGMENT_REP",  REDUNDANCYSEGMENT_REP },
    { "",                            0L}
};

// report types
OPTION aReportTypes[] =
{
    { "BRIEF_SORTBYDATE_REPTYPE",        BRIEF_SORTBYDATE_REPTYPE },
    { "BRIEF_SORTBYDOC_REPTYPE",         BRIEF_SORTBYDOC_REPTYPE },
    { "DETAIL_REPTYPE",                  DETAIL_REPTYPE },
    { "WITHTOTALS_REPTYPE",              WITHTOTALS_REPTYPE },
    { "WITHOUTTOTALS_REPTYPE",           WITHOUTTOTALS_REPTYPE },
    { "BASE_REPTYPE",                    BASE_REPTYPE },
    { "BASE_SUMMARY_REPTYPE",            BASE_SUMMARY_REPTYPE },
    { "BASE_SUMMARY_FACTSHEET_REPTYPE",  BASE_SUMMARY_FACTSHEET_REPTYPE },
    { "SUMMARY_FACTSHEET_REPTYPE",       SUMMARY_FACTSHEET_REPTYPE },
    { "FACTSHEET_REPTYPE",               FACTSHEET_REPTYPE },
    { "",                                0L}
};

// fuzzy search modes
OPTION aFuzzySearchModes[] =
{
  { "UPTOSELECTEDCLASS_MODE",          UPTOSELECTEDCLASS_MODE },
  { "SELECTEDCLASSANDHIGHER_MODE",     SELECTEDCLASSANDHIGHER_MODE },
  { "ONLYSELECTEDCLASS_MODE",          ONLYSELECTEDCLASS_MODE },
  { "",                                0L}
};

// fuzzy search classes
OPTION aFuzzySearchClasses[] =
{
  { "0",                               0 },
  { "1",                               1 },
  { "2",                               2 },
  { "3",                               3 },
  { "4",                               4 },
  { "5",                               5 },
  { "6",                               6 },
  { "",                                0L}
};

// token array and number of current tokens
char *apTokens[100];
int  iTokens;

//Flag for deleting Tempscript, after excecuting
char deleteTempScriptFlag = 0;


typedef struct _FUNCTEST_OUTPUT
{
    CHAR     szBuffer[1024];
    BOOL     fProcessID;
    BOOL     fDateTime;
    LOGLEVEL logLevel;
    FILE     *hfLog;
    char     szLogFile[1024];    // name of log file
} FUNCTEST_OUTPUT, *PFUNCTEST_OUTPUT;

// output log , if the logLevel is higher than the setted logLevel with /loglevel=
void OutputLog(LOGLEVEL logLevel, PFUNCTEST_OUTPUT Out, const char* pformat,...);

void Out_RC( PFUNCTEST_OUTPUT pOut, PSZ pszFunction, USHORT usRC, HSESSION hSession);
void Out_String( PFUNCTEST_OUTPUT pOut, const char* pszString,... );
void Out_Buffer( PFUNCTEST_OUTPUT pOut );



void GetDateTime( PSZ pszBuffer );
USHORT GetOption( PSZ pszOption, POPTION pOpt );

char szLastSegDocName[256] = "";
char szShortName[20] = "";
PARSSEGMENTW Segment = {0};
char szBuffer[8096];
char szVersionInfo[1024+1];
unsigned long ulWords = 0;
unsigned long ulTags = 0;

// OpenTM2 installed path
char gOpenTM2Path[512+1]={0};

#include "dbghelp.h"

// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
                                                                        CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                                                                        CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                                                        CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
                                                                        );

LONG TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionINFO );

// function prototypes
int PreProcessParms( PCOMMAND pCmd );
void PreProcessScript(FILE* hfScript, FILE* targetScript,PMARKER pfirst_maker, PFUNCTEST_OUTPUT Out, PCONSTA* lastConsta);
void SaveMarker( PMARKER pfirst_marker, FILE* targetScript, PFUNCTEST_OUTPUT Out, USHORT sCurLine, char* markername);
void GetBasePath(char *destString, char* sourceString);

//function prototypes for constant handling
void SaveConsta(PSZ name, PSZ value,PCONSTA *lastConst);
int ReplaceConstants(PSZ input,PSZ output, PCONSTA *lastConsta, PFUNCTEST_OUTPUT Out);


//function prototypes for variable handling
int SaveValueToVar(PSZ name, void* pValue, PVARIABLE *lastVariable, Type type=INTEGER);
int ReplaceVariables(PSZ input,PSZ output, PVARIABLE *lastVariable, PFUNCTEST_OUTPUT Out);
int IncVar(PSZ name, PVARIABLE *lastVariable,PFUNCTEST_OUTPUT Out);
int DecVar(PSZ name, PVARIABLE *lastVariable,PFUNCTEST_OUTPUT Out);
int AddToVar(char* name,  int toadd, PVARIABLE *lastVariable, PFUNCTEST_OUTPUT Out);
USHORT deleteFile(const char* pFilePath);
USHORT copyFile(const char* pSourceFilePath, const char* pTagetFilePath, LONG lOptions );

//function prototypes for wildcard handling
USHORT EscapeBackslash(char *pString);
int IsWildcard( char * TempLine);
int IsMatch(char *pattern, char *string);
void GetFolderPath(char *FolderName, char *targetString, PFUNCTEST_OUTPUT Out);
void AddToList(PWCPATH p_first_wc_path, char *sName);
void ClearList(PWCPATH p_first_wc_path);
int GetWildCardList(PWCPATH p_first_wc_path, PCOMMAND pCmd, PFUNCTEST_OUTPUT Out);
int GetFolderNames(PWCPATH p_first_wc_path, char *FolderName, PFUNCTEST_OUTPUT Out);
int GetMemoryNames(PWCPATH p_first_wc_path, char *MemoryName, PFUNCTEST_OUTPUT Out);
int GetFileNames(PWCPATH p_first_wc_path, char *FolderName, char *FilePath, PFUNCTEST_OUTPUT Out);
int GetDictionaryNames(PWCPATH p_first_wc_path, char *DictionaryName, PFUNCTEST_OUTPUT Out);
int GetFilePaths(PWCPATH p_first_wc_path, char *FileName, PFUNCTEST_OUTPUT Out);

//function prototypes for checking if-conditions
//char CheckSingleCondition(char*);
//char ParseExp(char*);

// used to parse checking if-conditions
int   expressionValue(char**);
int   termValue(char**);
char* factorValue(char**); 
char* skipSpace(char*);
int   isDigits(const char*);
int   cmpIgnoreQuotes(char*,char*);


//function for printing a Description for the parameters (help)
void Out_Help(PFUNCTEST_OUTPUT Out);

// get OpenTM2 installed path
char* getOpenTM2InstallPath(char* pResOut, int length);
const char* getGlobalOpenTM2Path();
// trim characters from string
char* trim(const char* pSrc,const char tCh, char* pRes, int len);
// execute a command
int executeCommand(const char* pszCmd);

int main
(
    int argc,
    char *argv[],
    char *envp[]
)
{
    HSESSION hSession = 0L;
    HPARSSEGFILE hLoadedFile = 0L;
    USHORT   usRC = 0;
    int fOK = TRUE;
    int i; //counting variable
    FILE *hfScript = NULL;               // script file handle
    FILE *hfLog = NULL;                  // log file handle
    PCOMMAND pCmd;                       // ptr to currently active command
    short sCurLine = 0;                  // current line in input file
    short sBraces = 0;                   // counter for braces
    FUNCTEST_OUTPUT Out;
    MARKER first_marker; // first marker
    FILE *tmpScript; //Filepointer for tmp script file
    PMARKER tmpmarker; // Marker pointer
    PCONSTA lastConsta=NULL; //Pointer to last Element of constants list
    PVARIABLE lastVariable=NULL;
    WCPATH first_wc_path;
    PWCPATH tmp_wc_path;
    envp;

    memset( &Out, 0, sizeof(Out) );

    // set default log level as LOG_ERROR if no set
    Out.logLevel = LOG_INFO;

    //init first_wc_path
    first_wc_path.next = NULL;
    first_wc_path.path[0] = NULC;
    
    //init first markers name
    first_marker.name[0]=NULC;
    first_marker.next=0;

    // install exception handler
    SetUnhandledExceptionFilter( (LPTOP_LEVEL_EXCEPTION_FILTER)TopLevelFilter );

    // show logo
    Out_String( &Out, "OpenTM2Scripter - The OpenTM2 API test program\n\n" );

    // skip program name
    argc--;
    argv++;

    // process arguments
    szScriptFile[0] = '\0';

     while ( argc )
      {
        PSZ pszParm = argv[0];
        while(*pszParm == ' ') 
            ++pszParm;

        if ( (*pszParm == '-') || (*pszParm == '/') )
        {
          if ( _strnicmp( pszParm+1, "logfile=", 8 ) == 0 )
          {
            strcpy( Out.szLogFile, pszParm+9 );
          }
          else if ( _strnicmp( pszParm+1, "constant=", 9 ) == 0 )
          {
            for(i=10;*(pszParm+i)!=',';i++){}
            *(pszParm+i)=0;
            SaveConsta(pszParm+10,pszParm+11+strlen(pszParm+10),&lastConsta);
          }
          else if ( _strnicmp( pszParm+1, "log=", 4 ) == 0 )
          {
            strcpy( Out.szLogFile, pszParm+5 );
          }
          else if(_strnicmp( pszParm+1, "loglevel=", 9) == 0)
          {   
              const int LENGTH=20;
              char logLevel[LENGTH+1]={0};
              strncpy(logLevel,pszParm+10, LENGTH);
              if(_strnicmp(logLevel,"DEBUG",5) == 0)
                  Out.logLevel = LOG_DEBUG;
              else if(_strnicmp(logLevel,"INFO",4) == 0)
                  Out.logLevel = LOG_INFO;
              else if(_strnicmp(logLevel,"WARNING",7) == 0) 
                  Out.logLevel = LOG_WARNING;
              else if(_strnicmp(logLevel,"ERROR",5) == 0) 
                  Out.logLevel = LOG_ERROR;
              else if(_strnicmp(logLevel,"NO",2) == 0) 
                  Out.logLevel = LOG_NO;
          }
          else if ( _strnicmp( pszParm+1, "INFO=", 5 ) == 0 )
          {
            if ( strchr( pszParm+6, 'D') || strchr( pszParm+6, 'd') )
            {
              Out.fDateTime = TRUE;
            } /* endif */
            if ( strchr( pszParm+6, 'P') || strchr( pszParm+6, 'p') )
            {
              Out.fProcessID = TRUE;
            } /* endif */
          }else if( _strnicmp( pszParm+1, "HELP", 4) == 0){
              Out_Help(&Out);
              fOK = FALSE;
          }else if( _strnicmp( pszParm+1, "delTemp", 7) == 0){
              deleteTempScriptFlag = 1;
          }
          else{
              OutputLog(LOG_WARNING,&Out,"WARNING==> unknown option \'%s\' is ignored\n", pszParm);
          } /* endif */
        }
        else if ( szScriptFile[0] == '\0' )
        {
          strcpy( szScriptFile, pszParm );
        }
        else
        {
          OutputLog(LOG_WARNING,&Out,"WARNING==> superfluos command line parameter \'%s\' is ignored\n", pszParm);
        } /* endif */
        argc--;
        argv++;
      } /* endwhile */

    // use first parameter as script file name or use default if none specified
    if ( fOK && szScriptFile[0] != '\0' )
    {
         OutputLog(LOG_DEBUG, &Out, "using specified script file %s\n", szScriptFile );
    }
    else if(fOK)
    {
        Out_Help(&Out);
        strcpy( szScriptFile, DEFAULT_SCRIPT );
        OutputLog( LOG_DEBUG, &Out, "using default script file %s\n", szScriptFile );
    } /* endif */

    // open log file (if any)
    if ( fOK && (Out.szLogFile[0] != '\0'))
    {
        Out.hfLog = fopen( Out.szLogFile, "w" );
        if ( Out.hfLog  == NULL )
        {
            OutputLog(LOG_WARNING, &Out, "Warning==> Open of log file %s failed\n ", Out.szLogFile/*szLogFile*/ );
        } 
        else
            hfLog = Out.hfLog;
    } /* endif */

    // open script file
    if ( fOK )
    {
        hfScript = fopen( szScriptFile, "r" );
        if ( hfScript == NULL )
        {
            OutputLog( LOG_ERROR, &Out, "ERROR ==> Open of script file %s failed, program aborted\n ", szScriptFile );
            fOK = FALSE;
        } /* endif */
    } /* endif */

    // start the batch session
    if ( fOK )
    {
        //    Out_TimeStamp( &Out, "EqfStartSession" );
        usRC = EqfStartSession( &hSession );
        if ( usRC  )
        {
            Out_RC( &Out, "EqfStartSession", usRC,hSession);
            fOK = FALSE;
        }
        else
        {
            OutputLog( LOG_INFO,&Out, "INFO ==> Eqf session started successfully\n");
        } /* endif */
    } /* endif */
  
  
    //Preprocessing Scriptfile
    if(fOK){

        OutputLog( LOG_DEBUG,&Out, "\n------------------------------------\nstart preprocessing script using \n(default_temp_script.script)\n" );
        //open temporary scfriptfile
        szTempScriptFile[0]='\0';
        sprintf(szTempScriptFile,"temp_script_%d.script",GetCurrentProcessId());
        tmpScript = fopen( szTempScriptFile, "w+" );
        PreProcessScript(hfScript, tmpScript, &first_marker, &Out, &lastConsta);
        //Set pointer to the beginning of the file
        fseek(tmpScript, 0L, SEEK_SET);
        //set tmp-Script as hfScript for processing it
        hfScript = tmpScript;
        OutputLog( LOG_DEBUG,&Out, "finish preprocessing script\n------------------------------------\n\n");
    }

  // loop through script file
    while ( fOK && !feof(hfScript) )
    {
        // get next line
        memset( szTempLine, 0, sizeof(szTempLine) );
        memset( szLine, 0, sizeof(szLine) );
        fgets( szTempLine, sizeof(szTempLine), hfScript );
        sCurLine++;
        // replace all variables in current line with saved Value
        if(ReplaceVariables( szTempLine, szLine , &lastVariable, &Out))
            szLine[0]=0;
        // process the line
        if ( szLine[0] != '\0' )
        {
            OutputLog(LOG_DEBUG, &Out, szLine);

            // remove trailing LF
            {
                int i = strlen(szLine);
                if ( szLine[i-1] == '\n' )
                {
                    szLine[i-1] = '\0';
                } /* endif */
            }

            // process the line
            if ( (szLine[0] == '*') || (szLine[0] == NULC) )
            {
                // skip comments and empty lines
            }
            else
            {
                // split line into tokens
                {
                    PSZ pszCurPos;        // current position within line

                    // initialize token array
                    memset( apTokens, 0, sizeof(apTokens) );
                    iTokens = 0;

                    // get first value as command token
                    pszCurPos = szLine;
                    SKIPSPACE( pszCurPos );      // skip leading white space
                    if ( *pszCurPos != NULC )
                    {
                        apTokens[iTokens++] = pszCurPos;
                        while ( (*pszCurPos != NULC) && (*pszCurPos != ' ') )
                        {
                            pszCurPos++;
                        } /* endwhile */
                        if ( *pszCurPos != NULC )
                        {
                            *pszCurPos = NULC;
                            pszCurPos++;
                        } /* endif */
                    } /* endif */

                    // scan remaining line for tokens
                    while ( *pszCurPos != NULC )
                    {
                        SKIPSPACE( pszCurPos );      // skip leading white space

                        // handle current token
                        if ( *pszCurPos == '\"' )
                        {
                            // remember token start
                            pszCurPos++;
                            apTokens[iTokens++] = pszCurPos;

                            // look for end of token
                            while ( (*pszCurPos != '\"') && (*pszCurPos != NULC) )
                            {
                                pszCurPos++;
                            } /* endwhile */

                            // terminate token and skip any delimiters
                            if ( *pszCurPos != NULC )
                            {
                                *pszCurPos = NULC;       // terminate token
                                pszCurPos++;             // skip string delimiter
                                SKIPSPACE( pszCurPos );  // skip any whitespace
                                SKIPCOMMA( pszCurPos );  // skip any delimiting comma
                            } /* endif */
                        }
                        else if ( *pszCurPos == '(' )
                        {
                            // remember token start
                            pszCurPos++;
                            apTokens[iTokens++] = pszCurPos;

                            // look for end of token
                            while ( (*pszCurPos != ')' || sBraces > 0) && (*pszCurPos != NULC) )
                            {
                                // remember open braces, prevent from early matches

                                if (*pszCurPos == '(') sBraces++;

                                if (*pszCurPos == ')') sBraces--;

                                pszCurPos++;
                            } /* endwhile */

                            // terminate token and skip any delimiters
                            if ( *pszCurPos != NULC )
                            {
                                *pszCurPos = NULC;       // terminate token
                                pszCurPos++;             // skip delimiter
                                SKIPSPACE( pszCurPos );  // skip any whitespace
                                SKIPCOMMA( pszCurPos );  // skip any delimiting comma
                            } /* endif */
                        }
                        else
                        {
                            // remember token start
                            apTokens[iTokens++] = pszCurPos;

                            // look for end of token
                            while ( (*pszCurPos != ',') && (*pszCurPos != NULC) )
                            {
                                pszCurPos++;
                            } /* endwhile */

                            // terminate token and skip any delimiters
                            if ( *pszCurPos != NULC )
                            {
                                *pszCurPos = NULC;       // terminate token
                                pszCurPos++;             // continue with next one
                            } /* endif */
                        } /* endif */
                    } /* endwhile */
                }

                // check first token against commands
                {
                    pCmd = aCommands;
                    while ( (pCmd->szCommand[0] != NULC) && (_stricmp( pCmd->szCommand, apTokens[0] ) != 0) )
                    {
                        pCmd++;
                    } /* endwhile */

                    if ( pCmd->szCommand[0] == NULC )
                    {
                        OutputLog(LOG_ERROR, &Out, "\tERROR ==>%s in line %d is no valid command, line is ignored.\n",apTokens[0], sCurLine);
                    } /* endif */
                }

                // handle valid commands
                if ( pCmd->szCommand[0] != NULC )
                {
                    switch ( pCmd->ID )
                    {
                        case INTCOMPWC_ID:
                        {
                            if ( PreProcessParms( pCmd ) )
                            {
                                FILE* tmpFilePointer1;
                                FILE* tmpFilePointer2;
                                char tmpLine1[1024];
                                char tmpLine2[1024];
                                char tmpLine3[1024];
                                char lastLine1[1024];
                                char lastLine2[1024];
                                int iLineCounter = 0;
                                int iSame = 1;
                                
                                //Look wheather the Params are there or not, if not give ERROR
                                if(apTokens[1] != NULL && apTokens[2] != NULL){
                                    
                                    //Try to Open the Files
                                    tmpFilePointer1=fopen( apTokens[1], "rb");                                    
                                    tmpFilePointer2=fopen( apTokens[2], "rb");

                                    //Set Memory for the Last Lines
                                    memset( lastLine1, 0, sizeof(lastLine1) );
                                    memset( lastLine2, 0, sizeof(lastLine2) );
                                    
                                    // Look wheather File1 and File2 was successfully opened or not, if not give ERROR
                                    if(tmpFilePointer1 != NULL && tmpFilePointer2 != NULL)
                                    {
                                        //Go through the Lines of the wordcount file
                                        while(!feof(tmpFilePointer1) && !feof(tmpFilePointer2))
                                        {
                                            //Clear Memory and get Lines
                                            memset( tmpLine1, 0, sizeof(tmpLine1) );
                                            memset( tmpLine2, 0, sizeof(tmpLine2) );
                                            fgets( tmpLine1, sizeof(tmpLine1), tmpFilePointer1 );
                                            fgets( tmpLine2, sizeof(tmpLine2), tmpFilePointer2 );

                                            //Inc Counter
                                            iLineCounter++;

                                            //Do Something if the Lines are Different
                                            if (_stricmp(tmpLine1,tmpLine2) != 0){
                                                sprintf(tmpLine3,"\t---------------------------------------\n\tDifference in Line %d:\n", iLineCounter);
                                                OutputLog(LOG_DEBUG,&Out,tmpLine3);
                                                if(_stricmp(lastLine1, lastLine2) ==0){
                                                    OutputLog(LOG_DEBUG,&Out,"\tLast Line: %s", lastLine1);
                                                }
                                                
                                                sprintf(lastLine1,"\tFile %s: \t%s", apTokens[1], tmpLine1);
                                                sprintf(lastLine2,"\tFile %s: \t%s", apTokens[2], tmpLine2);
                                                 OutputLog(LOG_DEBUG,&Out,lastLine1);
                                                 OutputLog(LOG_DEBUG,&Out,lastLine2);
                                                 OutputLog(LOG_DEBUG,&Out,"\t---------------------------------------\n");
                                                iSame = 0;
                                            }
                                            strcpy(lastLine1, tmpLine1);
                                            strcpy(lastLine2, tmpLine2);
                                        }/* endwhile */

                                        if(feof(tmpFilePointer1) && feof(tmpFilePointer2) && iSame)
                                        {
                                             //sprintf(Out.szBuffer,"\tINFO ==> %s returned: TRUE\n", apTokens[0]);
                                             OutputLog(LOG_INFO,&Out,"\tINFO ==> %s returned: TRUE\n", apTokens[0]);
                                        }else{
                                             //sprintf(Out.szBuffer,"\tINFO ==> %s returned: FALSE\n", apTokens[0]);
                                             OutputLog(LOG_INFO,&Out,"\tINFO ==> %s returned: FALSE\n", apTokens[0]);
                                        }

                                    }else{
                                        if(tmpFilePointer1 == NULL) 
                                            OutputLog(LOG_ERROR, &Out, "\tERROR ==> Unable to COMPARE, because it was not possible to open File %s.\n", apTokens[1] );
                                        
                                        if(tmpFilePointer2 == NULL) 
                                            OutputLog(LOG_ERROR, &Out, "\tERROR ==> Unable to COMPARE, because it was not possible to open File %s.\n", apTokens[2] );
                                    }                                
                                }else{
                                    if(apTokens[1] == NULL ) 
                                        OutputLog( LOG_ERROR, &Out, "\tERROR ==> Unable to COMPARE, because first Parameter(FilePath) was missing \n");
                                    
                                    if(apTokens[2] == NULL ) 
                                        OutputLog( LOG_ERROR, &Out, "\tERROR ==> Unable to COMPARE, because second Parameter(FilePath) was missing \n");                                    
                                }
                            }
                            break;
                        }
                        case INTCOMPBINARY_ID:
                        {
                            if ( PreProcessParms( pCmd ) )
                            {
                                if(apTokens[1] != NULL && apTokens[2] != NULL)
                                {
                                    FILE* tmpFilePointer1;
                                    FILE* tmpFilePointer2;
                                    char tmpChar1;
                                    char tmpChar2;

                                    tmpFilePointer1=fopen( apTokens[1], "rb");
                                    tmpFilePointer2=fopen( apTokens[2], "rb");

                                    // Look wheather File1 and File2 was successfully opened or not, if not give ERROR
                                    if(tmpFilePointer1 != NULL && tmpFilePointer2 != NULL)
                                    {
                                        fread(&tmpChar1, sizeof(char), 1, tmpFilePointer1);
                                        fread(&tmpChar2, sizeof(char), 1, tmpFilePointer2);
    
                                        while(tmpChar1 == tmpChar2 && !feof(tmpFilePointer1) && !feof(tmpFilePointer2))
                                        {
                                            fread(&tmpChar1, sizeof(char), 1, tmpFilePointer1);
                                            fread(&tmpChar2, sizeof(char), 1, tmpFilePointer2);
                                        }
    
                                        if(feof(tmpFilePointer1) && feof(tmpFilePointer2))
                                        {
                                            OutputLog(LOG_INFO, &Out, "\tINFO ==> %s returned: TRUE\n", apTokens[0] );
                                            usRC = 0;
                                        }else{
                                            OutputLog( LOG_INFO, &Out, "\tINFO ==> %s returned: FALSE\n", apTokens[0] );
                                            usRC = 1;
                                        }
    
                                    }else{
                                        if(tmpFilePointer1 == NULL) 
                                            OutputLog( LOG_ERROR, &Out, "\tERROR ==> Unable to COMPAREBINARY, because it was not possible to open File %s.\n", apTokens[1] );
                                        
                                        if(tmpFilePointer2 == NULL)
                                            OutputLog( LOG_ERROR, &Out, "\tERROR ==> Unable to COMPAREBINARY, because it was not possible to open File %s.\n", apTokens[2] );
                                       
                                        usRC = 1;
                                    }

                                    // close file
                                    if(tmpFilePointer1!=NULL)
                                        fclose(tmpFilePointer1);

                                    if(tmpFilePointer2!=NULL)
                                        fclose(tmpFilePointer2);

                                }else{
                                    if(apTokens[1] == NULL) OutputLog(LOG_ERROR, &Out, "\tERROR ==> Unable to COMPARE, because first Parameter(FilePath) was missing \n");
                                    if(apTokens[2] == NULL) OutputLog(LOG_ERROR, &Out, "\tERROR ==> Unable to COMPARE, because second Parameter(FilePath) was missing \n");                                    
                                }

                            }//end if(Pre...)
                        }
                        break;

                        case INTDEFINE_ID:
                        case INTMARKER_ID:
                        {
                            //do Nothing
                        }
                        break;
                        
                        case INTINCREMENT_ID:
                        {
                            if ( PreProcessParms( pCmd ) )
                            {
                                if (apTokens[1]!=NULL){
                                    IncVar(apTokens[1],&lastVariable,&Out);    
                                }
                                else{
                                      OutputLog(LOG_ERROR, &Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored");
                                }
                            }
                        }
                        break;

                        case INTDECREMENT_ID:
                        {
                            if ( PreProcessParms( pCmd ) )
                            {
                                if (apTokens[1]!=NULL){
                                    DecVar(apTokens[1],&lastVariable,&Out);
                                }
                                else{
                                      OutputLog(LOG_ERROR, &Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored");
                                }
                            }
                        }
                        break;

                        case INTADD_ID:
                        {
                            if ( PreProcessParms( pCmd ) )
                            {
                                if (apTokens[1]!=NULL&&apTokens[2]!=NULL){
                                    AddToVar(apTokens[1],atoi(apTokens[2]),&lastVariable,&Out);
                                }
                                else{
                                    if(Out.logLevel <= LOG_ERROR)
                                      OutputLog(LOG_ERROR,&Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored");
                                }
                            }
                        }
                        break;

                        case INTSUB_ID:
                        {
                            if ( PreProcessParms( pCmd ) )
                            {
                                if (apTokens[1]!=NULL&&apTokens[2]!=NULL){
                                    AddToVar(apTokens[1],atoi(apTokens[2])*(-1),&lastVariable,&Out);
                                }
                                else{
                                      OutputLog(LOG_ERROR, &Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored");
                                }
                            }
                        }
                        break;

                        case INTDELETEFILE_ID:
                        {
                            if ( PreProcessParms( pCmd ) )
                            {
                                 if (apTokens[1]!=NULL)
                                 {
                                   usRC = deleteFile(apTokens[1]);
                                   if(usRC != 0)
                                       OutputLog(LOG_ERROR, &Out,"\tERROR ==> %s can't be deleted, return %d\n",apTokens[1], usRC);
                                   else
                                       OutputLog(LOG_INFO, &Out,"\tINFO ==> %s be deleted successfully, return %d\n",apTokens[1], usRC);
                                 }
                                else
                                {
                                      OutputLog(LOG_ERROR, &Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored");
                                }
                            }
                        }
                        break;

                        case INTCOPYFILE_ID:
                        {
                            if ( PreProcessParms( pCmd ) )
                            {
                                 if ( (apTokens[1]!=NULL) && (apTokens[2]!=NULL) )
                                 {
                                   usRC = copyFile(apTokens[1], apTokens[2], lOption );
                                   if(usRC != 0)
                                       OutputLog(LOG_ERROR, &Out,"\tERROR ==> %s can't be copied to %s, return code is %d\n",apTokens[1], apTokens[2], usRC);
                                   else
                                       OutputLog(LOG_INFO, &Out,"\tINFO ==> %s copied successfully to %s\n",apTokens[1], apTokens[2] );
                                 }
                                else
                                 {
                                      OutputLog(LOG_ERROR, &Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored\n");
                                 }
                            }
                        }
                        break;
					    
						// For P403200 Begin
						case INTTESTVALUE_ID:
						{
							if ( PreProcessParms( pCmd ) )
							{
								if ( (apTokens[1]!=NULL) && (apTokens[2]!=NULL) )
								{
									 int res = strcmp(apTokens[1], apTokens[2]);
									 usRC = (res==0?1:0);
                                     OutputLog(LOG_INFO, &Out,"\tINFO ==> %s compare with %s euqal %d\n",apTokens[1], apTokens[2],res );
									 
								}
								else
								{
									 OutputLog(LOG_ERROR, &Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored\n");
								}
							}
						}
						break;
						// For P403200 End
                        
						case INTSAVERETURNCODE_ID:
                        {
                            if ( PreProcessParms( pCmd ) )
                            {
                                if (apTokens[1]!=NULL){
                                    int iRes = usRC;
                                    SaveValueToVar(apTokens[1],&iRes,&lastVariable);
                                }
                                else{
                                      OutputLog(LOG_ERROR, &Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored");
                                }
                            }
                        }
                        break;

                        case INTSETVALUE_ID:
                        {
                            if ( PreProcessParms( pCmd ) )
                            {
                                if (apTokens[1]!=NULL){

									if(isDigits(apTokens[2])){
                                       int setVal = atoi(apTokens[2]);
                                       SaveValueToVar(apTokens[1],&setVal,&lastVariable);
									} else{
										// For P403200 Begin
										PSZ pszValue = (PSZ)malloc( strlen(apTokens[2])+1);
                                        if(pszValue != NULL){
											strncpy( pszValue, apTokens[2],strlen(apTokens[2])+1 );
                                            SaveValueToVar(apTokens[1],pszValue,&lastVariable,POINTERSTR);
										}
										// For P403200 End
									}

                                }
                                else{
                                      OutputLog(LOG_ERROR,&Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored");
                                }
                            }
                        }
                        break;

                        case INTECHO_ID :
                            if ( PreProcessParms( pCmd ) )
                            {
                                Out_String(&Out, "%s\n", apTokens[1]);
                            }
                            break;

                        case INTGOTO_ID :
                        {
                            if ( PreProcessParms( pCmd ) )
                            {
                                if (apTokens[1]!=NULL){
                                    tmpmarker=&first_marker;
                                    //Go threw marker list until marker is found
                                    while (tmpmarker->next!=NULL && strcmp(tmpmarker->name,apTokens[1])!=0 ){
                                        tmpmarker=tmpmarker->next;
                                    }
                                    //Go to the marker
                                    if (tmpmarker!=NULL && strcmp(tmpmarker->name,apTokens[1])==0 ){
                                        fsetpos(hfScript,&tmpmarker->tmpScript);
                                        sCurLine= tmpmarker->ln;
                                    }
                                    else{
                                        //ERROR: Marker not found
                                        OutputLog( LOG_ERROR, &Out, "\tERROR ==> Marker '%s' not found.\n", apTokens[1] );
                                    }
                                }
                                else{
                                      OutputLog(LOG_ERROR,&Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored");
                                }
                            } /* endif */
                        }
                        break;

                        case INTGOTOCOND_ID:
                        {
                            if ( PreProcessParms( pCmd) )
                            {
                                if (apTokens[2]!=NULL){
									char* expv1 = apTokens[1]+1;
									char* expv = apTokens[1];
                                    if ( (*apTokens[1]=='!' && expressionValue(&expv1)==0) || (*apTokens[1]!='!' && expressionValue(&expv) ) )
                                    {
                                        if(apTokens[1][0] ==  '!') 
                                        {
                                            apTokens[1]++;
                                            OutputLog( LOG_DEBUG, &Out, "\tINFO ==> Same as If ( Condition )\n\tINFO ==> If-Condition is false (Negotiated Condition is true) Go to marker '%s'\n", apTokens[2]);
                                        }else{
                                            OutputLog( LOG_DEBUG, &Out, "\tINFO ==> Condition is true. Go to marker '%s'\n", apTokens[2]);                                    
                                        }
                                        
                                        tmpmarker=&first_marker;
                                        //Go threw marker list until marker is found
                                        while (tmpmarker->next!=NULL && strcmp(tmpmarker->name,apTokens[2])!=0 ){
                                            tmpmarker=tmpmarker->next;
                                        }
                                        //Go to the marker
                                        if (tmpmarker!=NULL && strcmp(tmpmarker->name,apTokens[2])==0 ){
                                            fsetpos(hfScript,&tmpmarker->tmpScript);
                                            sCurLine= tmpmarker->ln;
                                        }
                                        else{
                                            //ERROR: Marker not found
                                            OutputLog( LOG_ERROR, &Out, "\tERROR ==> Marker '%s' not found.\n", apTokens[2] );
                                        }
                                    }
                                    else{
                                        if(apTokens[1][0] ==  '!') 
                                        {
                                            apTokens[1]++;
                                            if(Out.logLevel <= LOG_INFO)
                                              OutputLog( LOG_INFO, &Out, "\tINFO ==> Same as If ( Condition )\n\tINFO ==> If-Condition is true (Negotiated Condition is false) Go to marker '%s'\n", apTokens[2]);
                                        }else{
                                              OutputLog(LOG_INFO, &Out, "\tINFO ==> Condition is false.\n" );
                                        }
                                    }
                                } 
                                else{
                                    if(Out.logLevel <= LOG_ERROR)
                                      OutputLog(LOG_ERROR,&Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored");
                                }
                            }
                        }
                        break;

                        case EQFCREATEMEM_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                usRC = EqfCreateMem( hSession,               // Eqf session handle
                                        apTokens[1],            // name of new Translation Memory
                                        apTokens[2],            // description for new Translation Memory or NULL
                                        *(apTokens[3]),         // target drive for new Translation Memory
                                        apTokens[4],            // Translation Memory source language
                                        lOption   );            // type of new Translation Memory
                                Out_RC( &Out, "EqfCreateMem", usRC, hSession);
                            } /* endif */
                        }
                        break;

                        case EQFCREATEFOLDER_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    usRC = EqfCreateFolder( hSession,            // Eqf session handle
                                            apTokens[1],         // name of folder
                                            apTokens[2],         // folder description or NULL
                                            *(apTokens[3]),      // folder target drive
                                            apTokens[4],         // folder Translation Memory
                                            apTokens[5],         // folder markup
                                            apTokens[6],         // folder editor
                                            tmp_wc_path->path,         // list of dictionaries or NULL
                                            apTokens[8],         // folder source language
                                            apTokens[9],         // folder target language
                                            apTokens[10],        // conversion
                                            tmp_wc_path->next->path );      // read-only TMs
                                    Out_RC( &Out, "EqfCreateFolder", usRC, hSession );
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFEXPORTFOLDER_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        USHORT usProgress = 0;
                                        do
                                        {
                                            usRC = EqfExportFolder( hSession,        // session handle
                                                    apTokens[1],     // name of folder
                                                    *(apTokens[2]),  // folder target drive
                                                    lOption,         // options for the folder export or 0L
                                                    tmp_wc_path->path,     // list of documents or NULL
                                                    apTokens[5] );   // export description or NULL
                                            if ( hfLog )
                                            {
                                                USHORT usNewProgress;
                                                EqfGetProgress( hSession, &usNewProgress );
                                                if ( usNewProgress != usProgress )
                                                {
                                                    usProgress = usNewProgress;
                                                    OutputLog(LOG_DEBUG, &Out, "\tProgress=%u\n", usProgress);
                                                } /* endif */
                                            } /* endif */
                                        } while ( usRC == CONTINUE_RC ); /* enddo */

                                        Out_RC( &Out, "EqfExportFolder", usRC, hSession );

                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFEXPORTFOLDERFP_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        USHORT usProgress = 0;
                                        do
                                        {
                                            usRC = EqfExportFolderFP( hSession,      // session handle
                                                    apTokens[1],     // name of folder
                                                    apTokens[2],     // folder target path
                                                    lOption,         // options for the folder export or 0L
                                                    tmp_wc_path->path,     // list of documents or NULL
                                                    apTokens[5] );   // export description or NULL
                                            if ( hfLog )
                                            {
                                                USHORT usNewProgress;
                                                EqfGetProgress( hSession, &usNewProgress );
                                                if ( usNewProgress != usProgress )
                                                {
                                                    usProgress = usNewProgress;
                                                    OutputLog(LOG_DEBUG, &Out, "\tProgress=%u\n", usProgress);
                                                } /* endif */
                                            } /* endif */
                                        } while ( usRC == CONTINUE_RC ); /* enddo */

                                        Out_RC( &Out, "EqfExportFolderFP", usRC, hSession );

                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }                        
                        break;

                        case EQFEXPORTFOLDERFPAS_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        USHORT usProgress = 0;
                                        do
                                        {
                                            usRC = EqfExportFolderFPas( hSession,      // session handle
                                                    apTokens[1],     // name of folder
                                                    apTokens[2],     // folder target path
                                                    apTokens[3],      // Export As name
                                                    lOption,         // options for the folder export or 0L
                                                    tmp_wc_path->path,     // list of documents or NULL
                                                    apTokens[6],     // description
                                                    apTokens[7]      // new name for folder memory
                                            );   // export description or NULL
                                            if ( hfLog )
                                            {
                                                USHORT usNewProgress;
                                                EqfGetProgress( hSession, &usNewProgress );
                                                if ( usNewProgress != usProgress )
                                                {
                                                    usProgress = usNewProgress;
                                                    OutputLog(LOG_DEBUG, &Out, "\tProgress=%u\n", usProgress);
                                                } /* endif */
                                            } /* endif */
                                        } while ( usRC == CONTINUE_RC ); /* enddo */

                                        Out_RC( &Out, "EqfExportFolderFPas", usRC, hSession );

                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFCOUNTWORDS_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                do
                                {
                                    usRC = EqfCountWords(   hSession,        // Eqf session handle
                                            apTokens[1],     // name of folder
                                            apTokens[2],     // list of documents or NULL
                                            lOption,         // options for the word count
                                            apTokens[4] );   // fully qualified output file
                                } while ( usRC == CONTINUE_RC ); /* enddo */

                                Out_RC( &Out, "EqfCountWordsFolder", usRC, hSession );

                            } /* endif */
                        }
                        break;

                        case EQFIMPORTDICT_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                    do
                                    {
                                        usRC = EqfImportDict( hSession,          // Eqf session handle
                                                apTokens[1],       // fully qualified name of input file
                                                apTokens[2],       // name of dictionary
                                                apTokens[3],       // password of dictionary
                                                lOption );         // dictionary import options
                                    } while ( usRC == CONTINUE_RC ); /* enddo */

                                    Out_RC( &Out, "EqfImportDict", usRC, hSession );

                            } /* endif */
                        }
                        break;

                        case EQFEXPORTDICT_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                do
                                {
                                    usRC = EqfExportDict( hSession,          // Eqf session handle
                                            apTokens[1],       // name of dictionary
                                            lOption,           // dictionary export options or 0L
                                            apTokens[3] );     // fully qualified name of output file
                                } while ( usRC == CONTINUE_RC ); /* enddo */

                                Out_RC( &Out, "EqfExportDict", usRC, hSession );

                            } /* endif */
                        }
                        break;

                        case EQFIMPORTDOC_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        do
                                        {
                                            usRC = EqfImportDoc(  hSession,          // Eqf session handle
                                                    apTokens[1],       // name of folder receiving the documents
                                                    tmp_wc_path->path,       // list of input files (documents)
                                                    apTokens[3],       // document Translation Memory or NULL
                                                    apTokens[4],       // document markup or NULL
                                                    apTokens[5],       // document editor or NULL
                                                    apTokens[6],       // document source language or NULL
                                                    apTokens[7],       // document target language or NULL
                                                      apTokens[8],       // alias for document name or NULL
                                                      apTokens[9],       // start path
                                                      apTokens[10],      // conversion
                                                      lOption );         // document import options or 0L
                                        } while ( usRC == CONTINUE_RC ); /* enddo */

                                        Out_RC( &Out, "EqfImportDoc", usRC,hSession );
                                        
                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFEXPORTDOC_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                //if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    //while(tmp_wc_path->next != NULL) 
                                    {
                                        do
                                        {
                                            usRC = EqfExportDoc( hSession,           // Eqf session handle
                                                    apTokens[1],     // name of folder
                                                    apTokens[2],//tmp_wc_path->path,     // list of documents or NULL
                                                    apTokens[3],     // start path
                                                    lOption );       // options for document export
                                        } while ( usRC == CONTINUE_RC ); /* enddo */

                                        Out_RC( &Out, "EqfExportDoc", usRC, hSession );
                                        
                                        //Shifting Pointer
                                       // tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFIMPORTMEM_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                USHORT usProgress = 0;
                                do
                                {
                                    usRC = EqfImportMem( hSession,           // Eqf session handle
                                            apTokens[1],     // name of Translation Memory
                                            apTokens[2],     // fully qualified name of input file
                                            lOption );       // options for Translation Memory import
                                    if ( hfLog )
                                    {
                                        USHORT usNewProgress;
                                        EqfGetProgress( hSession, &usNewProgress );
                                        if ( usNewProgress != usProgress )
                                        {
                                            usProgress = usNewProgress;
                                            OutputLog(LOG_DEBUG, &Out, "\tProgress=%u\n", usProgress);
                                        } /* endif */
                                    } /* endif */
                                } while ( usRC == CONTINUE_RC ); /* enddo */

                                Out_RC( &Out, "EqfImportMem", usRC, hSession );

                            } /* endif */
                        }                        
                        break;
                        
                        case EQFIMPORTMEMEX_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                USHORT usProgress = 0;
                                do
                                {
                                    usRC = EqfImportMemEx( hSession,           // Eqf session handle
                                            apTokens[1],     // name of Translation Memory
                                            apTokens[2],     // fully qualified name of input file
                                            apTokens[3],     // translation memory ID
                                            apTokens[4],     // ID for the origin of the translation memory
                                            apTokens[5],     // unused
                                            apTokens[6],     // unused
                                            lOption );       // options for Translation Memory import
                                    if ( hfLog )
                                    {
                                        USHORT usNewProgress;
                                        EqfGetProgress( hSession, &usNewProgress );
                                        if ( usNewProgress != usProgress )
                                        {
                                            usProgress = usNewProgress;
                                            OutputLog(LOG_DEBUG, &Out, "\tProgress=%u\n", usProgress);
                                        } /* endif */
                                    } /* endif */
                                } while ( usRC == CONTINUE_RC ); /* enddo */

                                Out_RC( &Out, "EqfImportMemEx", usRC, hSession );

                            } /* endif */
                        }                        
                        break;
                        
                        case EQFADDMATCHSEGID_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                do
                                {
                                    usRC = EqfAddMatchSegID( hSession,           // Eqf session handle
                                            apTokens[1],     // name of Translation Memory
                                            apTokens[2],     // translation memory ID
                                            apTokens[3],     // ID for the origin of the translation memory
                                            lOption );       // options for Translation Memory import
                                } while ( usRC == CONTINUE_RC ); /* enddo */

                                Out_RC( &Out, "EqfAddMatchSegID", usRC, hSession );

                            } /* endif */
                        }                        
                        break;
                        
                        case EQFEXPORTMEM_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                USHORT usProgress = 0;
                                do
                                {
                                    usRC = EqfExportMem( hSession,           // Eqf session handle
                                            apTokens[1],     // name of Translation Memory
                                            apTokens[2],     // fully qualified name of output file
                                            lOption );       // options for Translation Memory export
                                    if ( hfLog )
                                    {
                                        USHORT usNewProgress;
                                        EqfGetProgress( hSession, &usNewProgress );
                                        if ( usNewProgress != usProgress )
                                        {
                                            usProgress = usNewProgress;
                                            OutputLog(LOG_DEBUG, &Out, "\tProgress=%u\n", usProgress);
                                        } /* endif */
                                    } /* endif */
                                } while ( usRC == CONTINUE_RC ); /* enddo */

                                Out_RC( &Out, "EqfExportMem", usRC,hSession );

                            } /* endif */
                        }
                        break;

                        case EQFANALYZEDOC_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        USHORT usProgress = 0;
                                        do
                                        {
                                            usRC = EqfAnalyzeDoc( hSession,          // Eqf session handle
                                                    apTokens[1],     // name of folder
                                                    tmp_wc_path->path,     // list with document names
                                                    apTokens[3],     // Translation Memory
                                                    lOption );       // options for analysis
                                            if ( hfLog )
                                            {
                                                USHORT usNewProgress;
                                                EqfGetProgress( hSession, &usNewProgress );
                                                if ( usNewProgress != usProgress )
                                                {
                                                    usProgress = usNewProgress;
                                                    OutputLog(LOG_DEBUG, &Out, "\tProgress=%u\n", usProgress);
                                                } /* endif */
                                            } /* endif */
                                        } while ( usRC == CONTINUE_RC ); /* enddo */

                                        Out_RC( &Out, "EqfAnalyzeDoc", usRC,hSession );

                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFANALYZEDOCEX_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        do
                                        {
                                            usRC = EqfAnalyzeDocEx( hSession,          // Eqf session handle
                                                    apTokens[1],     // name of folder
                                                    tmp_wc_path->path,     // list with document names
                                                    apTokens[3],     // Translation Memory
                                                    apTokens[4],     // analysis profile
                                                    apTokens[5],     // reserved for future enhancement
                                                    lOption );       // options for analysis
                                        } while ( usRC == CONTINUE_RC ); /* enddo */

                                        Out_RC( &Out, "EqfAnalyzeDocEx", usRC ,hSession);
                                    
                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFDELETEMEM_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        usRC = EqfDeleteMem( hSession,           // Eqf session handle
                                                tmp_wc_path->path );      // Translation Memory being deleted

                                        Out_RC( &Out, "EqfDeleteMem", usRC ,hSession);

                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFIMPORTFOLDER_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                USHORT usProgress = 0;
                                do
                                {
                                    usRC = EqfImportFolder( hSession,        // Eqf session handle
                                            apTokens[1],     // name of folder
                                            *(apTokens[2]),  // drive containing the imported folder
                                            *(apTokens[3]),  // target drive for folder
                                            lOption );       // options for the folder import
                                    if ( hfLog )
                                    {
                                        USHORT usNewProgress;
                                        EqfGetProgress( hSession, &usNewProgress );
                                        if ( usNewProgress != usProgress )
                                        {
                                            usProgress = usNewProgress;
                                            OutputLog(LOG_DEBUG, &Out, "\tProgress=%u\n", usProgress);
                                        } /* endif */
                                    } /* endif */
                                } while ( usRC == CONTINUE_RC ); /* enddo */

                                Out_RC( &Out, "EqfImportFolder", usRC,hSession );

                            } /* endif */
                        }
                        break;

                        case EQFIMPORTFOLDERFP_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                USHORT usProgress = 0;
                                do
                                {
                                    usRC = EqfImportFolderFP( hSession,      // Eqf session handle
                                            apTokens[1],     // name of folder
                                            apTokens[2],     // path containing the imported folder
                                            *(apTokens[3]),  // target drive for folder
                                            lOption );       // options for the folder import
                                    if ( hfLog )
                                    {
                                        USHORT usNewProgress;
                                        EqfGetProgress( hSession, &usNewProgress );
                                        if ( usNewProgress != usProgress )
                                        {
                                            usProgress = usNewProgress;
                                            OutputLog(LOG_DEBUG, &Out, "\tProgress=%u\n", usProgress);
                                        } /* endif */
                                    } /* endif */
                                } while ( usRC == CONTINUE_RC ); /* enddo */

                                Out_RC( &Out, "EqfImportFolderFP", usRC,hSession );

                            } /* endif */
                        }
                        break;

                        case EQFIMPORTFOLDERAS_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                USHORT usProgress = 0;
                                do
                                {
                                    usRC = EqfImportFolderAs( hSession,      // Eqf session handle
                                            apTokens[1],     // name of folder
                                            apTokens[2],     // path containing the imported folder
                                            *(apTokens[3]),  // target drive for folder
                                            apTokens[4],     // new folder name
                                            lOption );       // options for the folder import
                                    if ( hfLog )
                                    {
                                        USHORT usNewProgress;
                                        EqfGetProgress( hSession, &usNewProgress );
                                        if ( usNewProgress != usProgress )
                                        {
                                            usProgress = usNewProgress;
                                            OutputLog(LOG_DEBUG, &Out, "\tProgress=%u\n", usProgress);
                                        } /* endif */
                                    } /* endif */
                                } while ( usRC == CONTINUE_RC ); /* enddo */

                                Out_RC( &Out, "EqfImportFolderAs", usRC,hSession );

                            } /* endif */
                        }
                        break;

                        case EQFDELETEFOLDER_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        usRC = EqfDeleteFolder( hSession,        // Eqf session handle
                                                tmp_wc_path->path );   // name of folder

                                        Out_RC( &Out, "EqfDeleteFolder", usRC,hSession );
                                        
                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFDELETEDOC_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        usRC = EqfDeleteDoc( hSession,           // Eqf session handle
                                                apTokens[1],        // name of folder
                                                tmp_wc_path->path );      // list of documents being deleted
                                        Out_RC( &Out, "EqfDeleteDoc", usRC ,hSession);
                                        
                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFORGANIZEMEM_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        do
                                        {
                                            usRC = EqfOrganizeMem( hSession,           // Eqf session handle
                                                    tmp_wc_path->path );      // Translation Memory being deleted
                                        } while ( usRC == CONTINUE_RC ); /* enddo */
                                        Out_RC( &Out, "EqfOrganizeMem", usRC ,hSession);
                                    
                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFARCHIVETM_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        // avoid trap for empty drive letter parameter
                                        CHAR szDrive = (apTokens[2] == NULL) ? NULC : *(apTokens[2]);

                                        do
                                        {
                                          // call API function
                                            usRC = EqfArchiveTM( hSession,    // Eqf session handle
                                                    apTokens[1], // folder name
                                                    szDrive, // target drive
                                                    tmp_wc_path->path, // documents
                                                    apTokens[4], // TM name
                                                    lOption );   // options
                                        } while ( usRC == CONTINUE_RC ); /* enddo */
                                        Out_RC( &Out, "EqfArchiveMem", usRC,hSession );
                                    
                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFCHANGEFOLPROPS_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        do
                                        {
                                            usRC = EqfChangeFolProps( hSession,    // Eqf session handle
                                                    apTokens[1], // folder name
                                                    *(apTokens[2]), // target drive
                                                    apTokens[3], // target language
                                                    apTokens[4], // memory name
                                                    tmp_wc_path->path);// dictionary name
                                        } while ( usRC == CONTINUE_RC ); /* enddo */
                                        Out_RC( &Out, "EqfChangeFolProps", usRC ,hSession);
                                        
                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFCHANGEFOLPROPSEX_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    do
                                    {
                                      // GQ 2016/06/10: only change values which really have been specified, 
                                      //                we have to find a way how to distinguish unused parameters from intentionally left blank parameters in the script...
                                      PSZ pszDictionary = ( tmp_wc_path->path[0] != EOS ) ? tmp_wc_path->path : NULL;
                                      PSZ pszROMem = ( tmp_wc_path->next->path[0] != EOS ) ? tmp_wc_path->next->path : NULL;

                                      usRC = EqfChangeFolPropsEx( hSession,    // Eqf session handle
                                                apTokens[1], // folder name
                                                *(apTokens[2]), // target drive
                                                apTokens[3], // target language
                                                apTokens[4], // memory name
                                                pszDictionary, // dictionary name
                                                pszROMem, // read-only memory name
                                                apTokens[7], // description
                                                apTokens[8], // profile name
                                                apTokens[9]); // unused 2

                                    } while ( usRC == CONTINUE_RC ); /* enddo */
                                    Out_RC( &Out, "EqfChangeFolPropsEx", usRC,hSession );
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFDELETEMTLOG_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        usRC = EqfDeleteMTLog( hSession,    // Eqf session handle
                                                tmp_wc_path->path ); // folder name
                                        Out_RC( &Out, "EqfDeleteMTLog", usRC, hSession );

                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

          case EQFGETSHORTNAME_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {
                  USHORT usObject = GetOption( apTokens[1], aShortNameObjects );

                  usRC = EqfGetShortName( hSession, usObject, apTokens[2], szShortName ); 
                  if ( !usRC )
                  {
                    OutputLog(LOG_INFO,&Out,szShortName);

                    if(apTokens[3]!=NULL && apTokens[3][0]!='\0')
                        SaveValueToVar(apTokens[3],szShortName,&lastVariable,POINTERSTR);

                  }                      
                  Out_RC( &Out, "EqfShortName", usRC ,hSession);
                } /* endif */
              }
              break;

            case EQFREMOVEDOCS_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {
                  usRC = EqfRemoveDocs( hSession, apTokens[1], apTokens[2] ); 
                  Out_RC( &Out, "EqfRemoveDocs", usRC ,hSession);
                } /* endif */
              }
              break;


            case EQFRESTOREDOCS_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {
                  usRC = EqfRestoreDocs( hSession, apTokens[1] ); 
                  Out_RC( &Out, "EqfRestoreDocs", usRC,hSession );
                } /* endif */
              }
              break;



                        case EQFCREATESUBFOLDER_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                do
                                {
                                    usRC = EqfCreateSubFolder( hSession,    // Eqf session handle
                                            apTokens[1], // folder name
                                            apTokens[2], // subfolder anme
                                            apTokens[3], // memory name
                                            apTokens[4], // markup
                                            apTokens[5], // sourcelanguage
                                            apTokens[6], // target language
                                            apTokens[7], // Editor
                                            apTokens[8], // Conversion
                                            apTokens[9], // Translator
                                            apTokens[10]);// Translator Mail
                                } while ( usRC == CONTINUE_RC ); /* enddo */

                                Out_RC( &Out, "EqfCreateSubFolder", usRC ,hSession);
                            } /* endif */
                        }
                        break;

                        case EQFCREATECONTROLLEDFOLDER_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    do
                                    {
                                        usRC = EqfCreateControlledFolder( hSession,    // Eqf session handle
                                                apTokens[1], // folder name
                                                apTokens[2], // description
                                                *(apTokens[3]), // target drive
                                                apTokens[4], // Translation Memory
                                                apTokens[5], // Markup
                                                apTokens[6], // Editor
                                                tmp_wc_path->path, // Dictionaries
                                                apTokens[8], // Source language
                                                apTokens[9], // Target language
                                                apTokens[10],// Conversion
                                                tmp_wc_path->next->path, // Read only Memories
                                                apTokens[12], // Password
                                                apTokens[13], // Project Coordinator Name
                                                apTokens[14], // Proj. Coord. Mail
                                                apTokens[15], // Translator Name
                                                apTokens[16], // Translator Mail
                                                apTokens[17], // Product Name
                                                apTokens[18], // Product Family
                                                apTokens[19], // Similar Product
                                                apTokens[20], // Product Dictionary
                                                apTokens[21], // Product Memory
                                                apTokens[22], // Previous Version
                                                apTokens[23], // Version
                                                apTokens[24]); // Shipment Number
                                    } while ( usRC == CONTINUE_RC ); /* enddo */

                                    Out_RC( &Out, "EqfCreateControlledFolder", usRC, hSession );
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFCHANGEMFLAG_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        do
                                        {
                                            usRC = EqfChangeMFlag( hSession,    // Eqf session handle
                                                    tmp_wc_path->path, // folder name
                                                    lOption );   // options
                                        } while ( usRC == CONTINUE_RC ); /* enddo */

                                        Out_RC( &Out, "EqfChangeMFlag", usRC,hSession );

                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

            case EQFCREATEITM_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {
                  do
                  {
                    usRC = EqfCreateITM( hSession,    // Eqf session handle
                                         apTokens[1], // name of prev. created TM
                                         apTokens[2], // file pairs
                                         apTokens[3], // markup
                                         apTokens[4], // name of external ITM
                                         apTokens[5], // source language
                                         apTokens[6], // target language
                                         apTokens[7], // source start path
                                         apTokens[8], // target start path
                                         lOption );   // options
                  } while ( usRC == CONTINUE_RC ); /* enddo */
                  Out_RC( &Out, "EqfCreateITM", usRC,hSession );
                } /* endif */
              }
              break;




                        case EQFCREATECNTREPORT_ID:
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {

                            } /* endif */
                        }
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        do
                                        {
                                            REPORTTYPE        ReportType = {NULL, 0L, NULL};
                                            REPORTSETTINGS    RepSettings = {NULL, 0, 0, NULL, 0, NULL, 0};
                                            FACTSHEET     FactSheet[10][3];
                                            FINALFACTORS  FinalFactors = {0L, 0L, NULL};

                                            ReportType.pszReport         = apTokens[4];        // report name
                                            ReportType.lRepType          = atol(apTokens[5]);  // report type
                                            ReportType.pszDescription    = apTokens[6];        // report description

                                            RepSettings.pszCountType     = apTokens[10];       // count type
                                            RepSettings.bShow            = atoi(apTokens[11]); // show categories
                                            RepSettings.bSummary         = atoi(apTokens[12]); // show summary
                                            RepSettings.pszRepLayout     = apTokens[13];       // report layout
                                            RepSettings.bShrink          = atoi(apTokens[14]); // automatic shrink
                                            RepSettings.pszStatisticType = apTokens[15];       // statistics type
                                            RepSettings.bExProposal      = atoi(apTokens[16]); // existing proposals

                                            FactSheet[0][0].lComplexity  = (float)atof(apTokens[17]);  // Analyze.Auto.Simple
                                            FactSheet[0][1].lComplexity  = (float)atof(apTokens[18]);  // Analyze.Auto.Medium
                                            FactSheet[0][2].lComplexity  = (float)atof(apTokens[19]);  // Analyze.Auto.Complex
                                            FactSheet[1][0].lComplexity  = (float)atof(apTokens[20]);  // Analyze.Post.Simple
                                            FactSheet[1][1].lComplexity  = (float)atof(apTokens[21]);  // Analyze.Post.Medium
                                            FactSheet[1][2].lComplexity  = (float)atof(apTokens[22]);  // Analyze.Post.Complex
                                            FactSheet[2][0].lComplexity  = (float)atof(apTokens[23]);  // Analyze.Edit.Simple
                                            FactSheet[2][1].lComplexity  = (float)atof(apTokens[24]);  // Analyze.Edit.Medium
                                            FactSheet[2][2].lComplexity  = (float)atof(apTokens[25]);  // Analyze.Edit.Complex
                                            FactSheet[3][0].lComplexity  = (float)atof(apTokens[26]);  // Edit.Exact.Simple
                                            FactSheet[3][1].lComplexity  = (float)atof(apTokens[27]);  // Edit.Exact.Medium
                                            FactSheet[3][2].lComplexity  = (float)atof(apTokens[28]);  // Edit.Exact.Complex
                                            FactSheet[4][0].lComplexity  = (float)atof(apTokens[29]);  // Edit.Replace.Simple
                                            FactSheet[4][1].lComplexity  = (float)atof(apTokens[30]);  // Edit.Replace.Medium
                                            FactSheet[4][2].lComplexity  = (float)atof(apTokens[31]);  // Edit.Replace.Complex
                                            FactSheet[5][0].lComplexity  = (float)atof(apTokens[32]);  // Fuzzy.MatchesLess70.Simple
                                            FactSheet[5][1].lComplexity  = (float)atof(apTokens[33]);  // Fuzzy.MatchesLess70.Medium
                                            FactSheet[5][2].lComplexity  = (float)atof(apTokens[34]);  // Fuzzy.MatchesLess70.Complex
                                            FactSheet[6][0].lComplexity  = (float)atof(apTokens[35]);  // Fuzzy.MatchesLess90.Simple
                                            FactSheet[6][1].lComplexity  = (float)atof(apTokens[36]);  // Fuzzy.MatchesLess90.Medium
                                            FactSheet[6][2].lComplexity  = (float)atof(apTokens[37]);  // Fuzzy.MatchesLess90.Complex
                                            FactSheet[7][0].lComplexity  = (float)atof(apTokens[38]);  // Fuzzy.MatchesGreater90.Simple
                                            FactSheet[7][1].lComplexity  = (float)atof(apTokens[39]);  // Fuzzy.MatchesGreater90.Medium
                                            FactSheet[7][2].lComplexity  = (float)atof(apTokens[40]);  // Fuzzy.MatchesGreater90.Complex
                                            FactSheet[8][0].lComplexity  = (float)atof(apTokens[41]);  // MachineMatches.Simple
                                            FactSheet[8][1].lComplexity  = (float)atof(apTokens[42]);  // MachineMatches.Medium
                                            FactSheet[8][2].lComplexity  = (float)atof(apTokens[43]);  // MachineMatches.Complex
                                            FactSheet[9][0].lComplexity  = (float)atof(apTokens[44]);  // ManuallyTranslated.Simple
                                            FactSheet[9][1].lComplexity  = (float)atof(apTokens[45]);  // ManuallyTranslated.Medium
                                            FactSheet[9][2].lComplexity  = (float)atof(apTokens[46]);  // ManuallyTranslated.Complex
                                            FactSheet[0][0].lPayFactor   = (float)atof(apTokens[47]);  // Analyze.Auto.Simple
                                            FactSheet[0][1].lPayFactor   = (float)atof(apTokens[48]);  // Analyze.Auto.Medium
                                            FactSheet[0][2].lPayFactor   = (float)atof(apTokens[49]);  // Analyze.Auto.Complex
                                            FactSheet[1][0].lPayFactor   = (float)atof(apTokens[50]);  // Analyze.Post.Simple
                                            FactSheet[1][1].lPayFactor   = (float)atof(apTokens[51]);  // Analyze.Post.Medium
                                            FactSheet[1][2].lPayFactor   = (float)atof(apTokens[52]);  // Analyze.Post.Complex
                                            FactSheet[2][0].lPayFactor   = (float)atof(apTokens[53]);  // Analyze.Edit.Simple
                                            FactSheet[2][1].lPayFactor   = (float)atof(apTokens[54]);  // Analyze.Edit.Medium
                                            FactSheet[2][2].lPayFactor   = (float)atof(apTokens[55]);  // Analyze.Edit.Complex
                                            FactSheet[3][0].lPayFactor   = (float)atof(apTokens[56]);  // Edit.Exact.Simple
                                            FactSheet[3][1].lPayFactor   = (float)atof(apTokens[57]);  // Edit.Exact.Medium
                                            FactSheet[3][2].lPayFactor   = (float)atof(apTokens[58]);  // Edit.Exact.Complex
                                            FactSheet[4][0].lPayFactor   = (float)atof(apTokens[59]);  // Edit.Replace.Simple
                                            FactSheet[4][1].lPayFactor   = (float)atof(apTokens[60]);  // Edit.Replace.Medium
                                            FactSheet[4][2].lPayFactor   = (float)atof(apTokens[61]);  // Edit.Replace.Complex
                                            FactSheet[5][0].lPayFactor   = (float)atof(apTokens[62]);  // Fuzzy.MatchesLess70.Simple
                                            FactSheet[5][1].lPayFactor   = (float)atof(apTokens[63]);  // Fuzzy.MatchesLess70.Medium
                                            FactSheet[5][2].lPayFactor   = (float)atof(apTokens[64]);  // Fuzzy.MatchesLess70.Complex
                                            FactSheet[6][0].lPayFactor   = (float)atof(apTokens[65]);  // Fuzzy.MatchesLess90.Simple
                                            FactSheet[6][1].lPayFactor   = (float)atof(apTokens[66]);  // Fuzzy.MatchesLess90.Medium
                                            FactSheet[6][2].lPayFactor   = (float)atof(apTokens[67]);  // Fuzzy.MatchesLess90.Complex
                                            FactSheet[7][0].lPayFactor   = (float)atof(apTokens[68]);  // Fuzzy.MatchesGreater90.Simple
                                            FactSheet[7][1].lPayFactor   = (float)atof(apTokens[69]);  // Fuzzy.MatchesGreater90.Medium
                                            FactSheet[7][2].lPayFactor   = (float)atof(apTokens[70]);  // Fuzzy.MatchesGreater90.Complex
                                            FactSheet[8][0].lPayFactor   = (float)atof(apTokens[71]);  // MachineMatches.Simple
                                            FactSheet[8][1].lPayFactor   = (float)atof(apTokens[72]);  // MachineMatches.Medium
                                            FactSheet[8][2].lPayFactor   = (float)atof(apTokens[73]);  // MachineMatches.Complex
                                            FactSheet[9][0].lPayFactor   = (float)atof(apTokens[74]);  // ManuallyTranslated.Simple
                                            FactSheet[9][1].lPayFactor   = (float)atof(apTokens[75]);  // ManuallyTranslated.Medium
                                            FactSheet[9][2].lPayFactor   = (float)atof(apTokens[76]);  // ManuallyTranslated.Complex

                                            FinalFactors.lUnit           = atol(apTokens[79]);         // pay standard
                                            FinalFactors.lCurrFactor     = (float)atof(apTokens[80]);  // currency factor
                                            FinalFactors.pszLocalCurrency = apTokens[81];              // local currency

                                            usRC = EqfCreateCntReport( hSession,                    // Eqf session handle
                                                    *(apTokens[1]),              // drive letter
                                                    apTokens[2],                 // folder name
                                                    tmp_wc_path->path,                 // document list or NULL for all docs in folder
                                                    &ReportType,
                                                    apTokens[7],                 // outfile name
                                                    apTokens[8],                 // outfile type
                                                    apTokens[9],                 // profile name
                                                    &RepSettings,
                                                    &FactSheet[0][0],
                                                    (USHORT)atoi(apTokens[77]),  // column
                                                    (USHORT)atoi(apTokens[78]),  // category
                                                    &FinalFactors,
                                                    atol(apTokens[82]),          // security option                                                  apTokens[10],      // conversion
                                                    (BOOL)atoi(apTokens[83]) );  // shipment type
                                        } while ( usRC == CONTINUE_RC ); /* enddo */
                                        Out_RC( &Out, "EqfCreateCountingRpt", usRC ,hSession);
                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFSETSYSLANGUAGE_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                usRC = EqfSetSysLanguage( hSession,          // Eqf session handle
                                        apTokens[1] );          // name of language to be set
                                Out_RC( &Out, "EqfSetSysLanguage", usRC,hSession );
                            } /* endif */
                        }
                        break;

                        case EQFGETSYSLANGUAGE_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                CHAR chText[256];
                                memset( &chText[0], 0, sizeof( chText ));
                                usRC = EqfGetSysLanguage( hSession,          // Eqf session handle
                                        chText );          // name of system language

                                //sprintf( szMsg, "\tINFO ==> EqfGetSysLanguage returned %u SystemLanguage: >%s<\n", usRC, chText );
                                OutputLog(LOG_INFO, &Out, "\tINFO ==> EqfGetSysLanguage returned %u SystemLanguage: >%s<\n", usRC, chText );

                            } /* endif */
                        }
                        break;

                        case EQFLOADSEGFILE_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                PSZ pszDocName = apTokens[1];

                                // free any file loaded by a previous call
                                if ( hLoadedFile ) { EqfFreeSegFile( hLoadedFile ); hLoadedFile = NULL; }

                                if ( _stricmp( pszDocName, "&segdocname" ) == 0 )
                                {
                                    pszDocName = szLastSegDocName;
                                } /* endif */

                                usRC = EqfLoadSegFile( hSession, pszDocName, &hLoadedFile );
                                Out_RC( &Out, "EqfLoadSegFile", usRC ,hSession);
                            } /* endif */
                        }
                        break;

                        case EQFBUILDSEGDOCNAME_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                szLastSegDocName[0] = '\0';
                                usRC = EqfBuildSegDocName( hSession, apTokens[1], apTokens[2],
                                        (USHORT)atoi(apTokens[3]), szLastSegDocName );

                                if(usRC==0 && apTokens[4]!=NULL && apTokens[4][0]!='\0')
                                    SaveValueToVar(apTokens[4],szLastSegDocName,&lastVariable,POINTERSTR);

                                OutputLog(LOG_INFO, &Out, "\tINFO ==> EqfBuildSegDocName returned %u, DocPathName is %s\n", usRC,szLastSegDocName );

                            } /* endif */
                        }
                        break;

                        case EQFGETSEGW_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                if ( !hLoadedFile )
                                {
                                    OutputLog(LOG_ERROR, &Out, "ERROR ==> Could not run EqfGetSegW, no segmented file is loaded\n" );
                                }
                                else
                                {
                                    LONG lSegNum = atol(apTokens[1]);
                                    memset( &Segment, 0, sizeof(Segment) );
                                    usRC = EqfGetSegW( hLoadedFile, lSegNum, &Segment );
                                    WideCharToMultiByte( CP_OEMCP, 0, Segment.szData,
                                            -1, szBuffer, sizeof(szBuffer), NULL, NULL );

                                    OutputLog( LOG_INFO, &Out, "\tINFO ==> EqfGetSegW returned %u, Text of Segment %ld is >%s<\n", usRC, lSegNum, szBuffer);

                                } /* endif */
                            } /* endif */
                        }
                        break;

                        case EQFGETSOURCELINE_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                if ( !hLoadedFile )
                                {
                                    OutputLog(LOG_ERROR, &Out,"ERROR ==> Could not run EqfGetSourceLine, no segmented file is loaded\n");
                                }
                                else
                                {
                                    LONG lSegNum = atol(apTokens[1]);
                                    LONG lStartLine = 0;
                                    LONG lEndLine = 0;
                                    usRC = EqfGetSourceLine( hLoadedFile, lSegNum, &lStartLine, &lEndLine );

                                    OutputLog(LOG_INFO, &Out, "\tINFO ==> EqfGetSourceLine returned %u, start/end line for Segment %ld is %ld/%ld\n",
                                              usRC, lSegNum, lStartLine, lEndLine);
                                } /* endif */
                            } /* endif */
                        }
                        break;

                        case EQFGETSEGMENTNUMBER_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                if ( !hLoadedFile )
                                {
                                    OutputLog(LOG_ERROR, &Out, "\tERROR ==> Could not run EqfGetSegmentNumber, no segmented file is loaded\n");
                                }
                                else
                                {
                                    LONG lSegNum = 0;
                                    LONG lLine = atol(apTokens[1]);
                                    LONG lColumn = atol(apTokens[2]);
                                    usRC = EqfGetSegmentNumber( hLoadedFile, lLine, lColumn, &lSegNum );
                                    OutputLog( LOG_INFO, &Out, "\tINFO ==> EqfGetSegmentNumber returned %u, segment number of segment at position %ld:%ld is %ld\n",
                                                usRC, lLine, lColumn, lSegNum);
                                } /* endif */
                            } /* endif */
                        }
                        break;

                        case EQFCLEANMEMORY_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                USHORT usProgress = 0;
                                do
                                {
                                    usRC = EqfCleanMemory(    hSession,      // session handle
                                            apTokens[1],     // name of folder
                                            apTokens[2],     // input memory
                                            apTokens[3],     // output memory
                                            lOption );       // options
                                    if ( hfLog )
                                    {
                                        USHORT usNewProgress;
                                        EqfGetProgress( hSession, &usNewProgress );
                                        if ( usNewProgress != usProgress )
                                        {
                                            usProgress = usNewProgress;
                                            OutputLog(LOG_DEBUG, &Out, "\tProgress=%u\n", usProgress);
                                        } /* endif */
                                    } /* endif */
                                } while ( usRC == CONTINUE_RC ); /* enddo */
                                Out_RC( &Out, "EqfCleanMemory", usRC,hSession );
                            
                            } /* endif */
                        }
                        break;

                        case EQFCREATECOUNTREPORT_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        USHORT usReport = GetOption( apTokens[4], aReports );
                                        USHORT usType   = GetOption( apTokens[5], aReportTypes );


                                        usRC = EqfCreateCountReport( hSession,     // Eqf session handle
                                                apTokens[1],  // name of folder containing the documents
                                                tmp_wc_path->path,  // list of documents being counted
                                                apTokens[3],  // fully qualified name of output file
                                                usReport,     // ID of report being created
                                                usType,       // type of report being created
                                                apTokens[6],  // name of profile
                                                lOption );    // options

                                        Out_RC( &Out, "EqfCreateCountReport", usRC ,hSession);
                                        
                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        case EQFCREATECOUNTREPORTEX_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        USHORT usReport = GetOption( apTokens[4], aReports );
                                        USHORT usType   = GetOption( apTokens[5], aReportTypes );


                                        usRC = EqfCreateCountReportEx( hSession,     // Eqf session handle
                                                apTokens[1],  // name of folder containing the documents
                                                tmp_wc_path->path,  // list of documents being counted
                                                apTokens[3],  // fully qualified name of output file
                                                usReport,     // ID of report being created
                                                usType,       // type of report being created
                                                apTokens[6],  // name of profile
                                                apTokens[7],  // shipment
                                                apTokens[8],  // unused
                                                apTokens[9],  // unused
                                                lOption );    // options

                                        Out_RC( &Out, "EqfCreateCountReportEx", usRC ,hSession);
                                        
                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;


                        case EQFRENAME_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                USHORT usObject = GetOption( apTokens[1], aRenameObjects );

                                usRC = EqfRename( hSession,     // Eqf session handle
                                        usObject,     // type of pbject being renamed
                                        apTokens[2],  // name of existing object
                                        apTokens[3],  // new name for object
                                        lOption );    // options

                                Out_RC( &Out, "EqfRename", usRC ,hSession);

                            } /* endif */
                        }
                        break;

                        case EQFPROCESSNOMATCH_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                do
                                {
                                    usRC = EqfProcessNomatch( hSession,     // Eqf session handle
                                            apTokens[1],  // fully qualified name of the SNOMATCh file, folder name or search path
                                            apTokens[2],  // name of the internal input memory
                                            apTokens[3],  // name of internal output memory (is created if it does not exist)
                                            apTokens[4],  // fully qualified file name of the memory match count report (text format)
                                            apTokens[5],  // fully qualified file name of the memory match count report (XML format)
                                            apTokens[6],  // fully qualified file name of the duplicate word count report (text format)
                                            apTokens[7],  // fully qualified file name of the duplicate word count report (XML format)
                                            lOption );    // options
                                } while ( usRC == CONTINUE_RC ); /* enddo */

                                Out_RC( &Out, "EqfProcessNomatch", usRC,hSession );

                            } /* endif */
                        }
                        break;

          case EQFPROCESSNOMATCHEX_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {

                  do
                  {
                    usRC = EqfProcessNomatchEx( hSession,     // Eqf session handle
                                      apTokens[1],  // fully qualified name of the SNOMATCh file, folder name or search path
                                      apTokens[2],  // name of the internal input memory
                                      apTokens[3],  // name of internal output memory (is created if it does not exist)
                                      apTokens[4],  // fully qualified file name of the memory match count report (text format)
                                      apTokens[5],  // fully qualified file name of the memory match count report (XML format)
                                      apTokens[6],  // fully qualified file name of the duplicate word count report (text format)
                                      apTokens[7],  // fully qualified file name of the duplicate word count report (XML format)
                                      apTokens[8],  // fully qualified file name of the output nomatch file (XML format)
                                      apTokens[9],  // fully qualified file name of the output nomatch file (EXP format)
                                      lOption );    // options
                  } while ( usRC == CONTINUE_RC ); /* enddo */

                  Out_RC( &Out, "EqfProcessNomatchEx", usRC ,hSession);

                } /* endif */
              }
              break;




                        case EQFOPENDOC_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                ULONG ulSegNum = atol(apTokens[3]);
                                ULONG ulLine = atol(apTokens[4]);

                                usRC = EqfOpenDoc( hSession, apTokens[1], apTokens[2], ulSegNum, ulLine );

                                OutputLog( LOG_INFO, &Out, "\tInfo==>EqfOpenDoc returned %u\n", usRC );
                            } /* endif */
                        }
                        break;

            case EQFADDCTIDLIST_ID :
            {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {
                  usRC = EqfAddCTIDList( hSession, apTokens[1], apTokens[2] );

                  OutputLog(LOG_INFO, &Out, "\tInfo==>EqfAddCTIDList returned %u\n", usRC );

                } /* endif */
            }
            break;

            case EQFEXPORTSEGS_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {
                  do
                  {
                    usRC = EqfExportSegs( hSession,     // Eqf session handle
                                      apTokens[1],  // folder name 
                                      apTokens[2],  // list with document names or NULL
                                      apTokens[3],  // file containing start/stop tag list
                                      apTokens[4],  // name of output file
                                      lOption );    // options
                  } while ( usRC == CONTINUE_RC ); /* enddo */

                  Out_RC( &Out, "EqfExportSegs", usRC,hSession );

                } /* endif */
              }
              break;

              case EQFFOLDEREXISTS_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {
                  usRC = EqfFolderExists( hSession,     // Eqf session handle
                                          apTokens[1] );// folder name 
                  Out_RC( &Out, "EqfFolderExists", usRC,hSession );
                } /* endif */
              }
              break;

            case EQFMEMORYEXISTS_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {
                  usRC = EqfMemoryExists( hSession,     // Eqf session handle
                                          apTokens[1] );// memory name 
                  Out_RC( &Out, "EqfMemoryExists", usRC ,hSession);

                } /* endif */
              }
              break;

            case EQFDICTIONARYEXISTS_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {
                  usRC = EqfDictionaryExists( hSession,     // Eqf session handle
                                              apTokens[1] );// dictionary name 
                  Out_RC( &Out, "EqfDictionaryExists", usRC,hSession );
                } /* endif */
              }
              break;

            case EQFDOCUMENTEXISTS_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {
                  usRC = EqfDocumentExists( hSession,     // Eqf session handle
                                              apTokens[1],// folder name 
                                              apTokens[2] );// document name 
                  Out_RC( &Out, "EqfDocumentExists", usRC,hSession );
                } /* endif */
              }
              break;


            case EQFGETFOLDERPROP_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {

                  usRC = EqfGetFolderProp( hSession,     // Eqf session handle
                                      apTokens[1],    // folder name 
                                      &FolProps );        // buffer for property data
                  if ( usRC == 0 )
                  {
                    int i = 0;
                    FILE *hfOut = NULL;
                    PSZ pszOutFile = apTokens[2];
                    if ( (pszOutFile != NULL) && (*pszOutFile != '\0') )
                    {
                      hfOut = fopen( pszOutFile, "w" );
                    } /* endif */                       
                    if ( hfOut != NULL ) 
                        fprintf( hfOut, "Properties of folder %s\n", apTokens[1] ); 
                    else
                        OutputLog( LOG_DEBUG, &Out, "\n" );

                    sprintf( szMsg, "Drive=%c\n", FolProps.chDrive );
                    if ( hfOut != NULL ) 
                        fputs( szMsg, hfOut ); 
                    else
                        OutputLog( LOG_DEBUG, &Out, szMsg );

                    sprintf( szMsg, "TargetLang=%s\n", FolProps.szTargetLang );
                    if ( hfOut != NULL )
                        fputs( szMsg, hfOut );
                    else
                        OutputLog(LOG_DEBUG, &Out, szMsg );

                    sprintf( szMsg, "R/W-Memory=%s\n", FolProps.szRWMemory );
                    if ( hfOut != NULL ) 
                        fputs( szMsg, hfOut );
                    else
                        OutputLog(LOG_DEBUG, &Out, szMsg );

                    strcpy( szMsg, "R/O-Memories=" );
                    i = 0;
                    while ( (i < MAX_NUM_OF_READONLY_MDB) && (FolProps.szROMemTbl[i][0] != EOS) )
                    {
                      if ( i != 0 ) strcat( szMsg, "," );
                      strcat( szMsg, FolProps.szROMemTbl[i] );
                      i++;
                    } /* endwhile */                       
                    strcat( szMsg, "\n" );
                    if ( hfOut != NULL ) 
                        fputs( szMsg, hfOut ); 
                    else
                        OutputLog( LOG_DEBUG, &Out, szMsg );

                    strcpy( szMsg, "Dictionaries=" );
                    i = 0;
                    while ( (i < NUM_OF_FOLDER_DICS) && (FolProps.szDicTbl[i][0] != EOS) )
                    {
                      if ( i != 0 ) strcat( szMsg, "," );
                      strcat( szMsg, FolProps.szDicTbl[i] );
                      i++;
                    } /* endwhile */                       
                    strcat( szMsg, "\n" );
                    if ( hfOut != NULL ) 
                        fputs( szMsg, hfOut ); 
                    else
                        OutputLog(LOG_DEBUG, &Out, szMsg );

                    if ( hfOut != NULL ) fclose( hfOut );
                  } /* endif */                     
                  Out_RC( &Out, "EqfGetFolderProp", usRC ,hSession);
                } /* endif */
              }
              break;

            case EQFGETFOLDERPROPEX_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {

                  usRC = EqfGetFolderPropEx( hSession,     // Eqf session handle
                                             apTokens[1],  // folder name 
                                             apTokens[2],  // key for requested value
                                             szFolPropValue, // buffer for returned value
                                             sizeof(szFolPropValue) );
                  if ( usRC == 0 )
                  {
                    if ( (apTokens[3] != NULL) && (apTokens[3][0] != '\0') )
                    {
                      // as variables only save the pointer to the string but not its value we have to make a copy of the value string
                      PSZ pszValue = (PSZ)malloc( max( strlen( szFolPropValue ), 10 ) );
                      strcpy( pszValue, szFolPropValue );
                      SaveValueToVar( apTokens[3], pszValue, &lastVariable, POINTERSTR );
                    }
                  } /* endif */                     
                  Out_RC( &Out, "EqfGetFolderPropEx", usRC ,hSession);
                } /* endif */
              }
              break;

            case EQFOPENDOCBYTRACK_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {

                  usRC = EqfOpenDocByTrack( hSession,     // Eqf session handle
                                             apTokens[1],  // folder name 
                                             apTokens[2] );// track ID
                  Out_RC( &Out, "EqfOpenDocByTrack", usRC ,hSession);
                } /* endif */
              }
              break;

            case EQFCOUNTWORDWSINSTRING_ID :
              {
                // preprocess parameters
                if ( PreProcessParms( pCmd ) )
                {
                  //unsigned long ulWords = 0;
                  //unsigned long ulTags = 0;
                  PSZ_W pszUTF16String = NULL;
                  size_t size = 0;

                  if ( apTokens[3] != NULL )
                  {
                    size = strlen(apTokens[3]) + 20;
                    pszUTF16String = (PSZ_W)malloc( sizeof(CHAR_W) * size );

                    MultiByteToWideChar( CP_OEMCP, 0, apTokens[3], -1, pszUTF16String, size );
                    usRC = EqfCountWordsInString( hSession,     // Eqf session handle
                                                  apTokens[1],    // markup name
                                                  apTokens[2],    // language
                                                  pszUTF16String,    // text being counted
                                                  &ulWords, &ulTags ); 

                    if(apTokens[4]!=NULL && apTokens[4][0]!='\0')
                        SaveValueToVar(apTokens[4],&ulWords,&lastVariable);

                    if(apTokens[5]!=NULL && apTokens[5][0]!='\0')
                        SaveValueToVar(apTokens[5],&ulTags,&lastVariable);

                    free( pszUTF16String );
                  }
                  Out_RC( &Out, "EqfCountWordsInString", usRC ,hSession);
                  if ( usRC == 0 )
                  {
                    OutputLog( LOG_DEBUG, &Out, "String contains %lu words and %lu Tags\n", ulWords, ulTags );
                  } /* endif */                     
                } /* endif */
              }
              break;


                  case EQFCHECKSPELLING_ID :
                    {
                      // preprocess parameters
                      if ( PreProcessParms( pCmd ) )
                      {
                        usRC = EqfCheckSpelling( hSession,     // Eqf session handle
                                                 apTokens[1],    // language
                                                 apTokens[2],    // in terms
                                                 apTokens[3],    // in file
                                                 apTokens[4],    // report file
                                                 lOption );
                  
                        Out_RC( &Out, "EqfCheckSpelling", usRC ,hSession);
                      } /* endif */
                    }
                    break;


                  case EQFREDUCETOSTEMFORM_ID :
                    {
                      // preprocess parameters
                      if ( PreProcessParms( pCmd ) )
                      {
                        usRC = EqfReduceToStemForm( hSession,     // Eqf session handle
                                                    apTokens[1],    // language
                                                    apTokens[2],    // in terms
                                                    apTokens[3],    // in file
                                                    apTokens[4],  // report file
                                                    lOption );
                  
                        Out_RC( &Out, "EqfReduceToStemForm", usRC ,hSession);
                      } /* endif */
                    }
                    break;

                    case EQFFILTERNOMATCHFILE_ID :
                        {
                          // preprocess parameters
                          if ( PreProcessParms( pCmd ) )
                          {
                            lOption |= CLEANMEM_COMPLETE_IN_ONE_CALL_OPT; // always complete in one call
                            usRC = EqfFilterNoMatchFile( hSession,     // Eqf session handle
                                                         apTokens[1],  // input nomatch file (XML format)
                                                         apTokens[2],  // global memory option file
                                                         apTokens[3],  // Memory for filtering
                                                         apTokens[4],  // OUT: filtered nomatch file (XML format)  
                                                         apTokens[5],  // OUT: filtered nomatch file (EXP format)  
                                                         apTokens[6],  // OUT: Word count report
                                                         lOption );    // options
                  
                            Out_RC( &Out, "EqfFilterNoMatchFile", usRC ,hSession);
                          } /* endif */
                        }
                        break;

                        case SLEEP_ID :
                        {
                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {
                                int iTime = atoi(apTokens[1]);
                                Sleep( iTime );

                                OutputLog( LOG_INFO, &Out,  "\tINFO ==> Sleeping %ld miliseconds...\n", iTime );
                            } /* endif */
                        }
                        break;

                        case EQFDELETEDICT_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        usRC = EqfDeleteDict( hSession, tmp_wc_path->path );     

                                        Out_RC( &Out, "EqfDeleteDict", usRC ,hSession);

                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                                else
                                {
                                  // call API call without wildcard support
                                  usRC = EqfDeleteDict( hSession, apTokens[1] );
                                  Out_RC( &Out, "EqfDeleteDict", usRC ,hSession);
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;
                        
                        case EQFGETVERSIONEX_ID :
                        {
                            if ( PreProcessParms( pCmd ) )
                            {  
                               char szVersion[512+1]={'\0'};
                               usRC = EqfGetVersionEx(szVersion,512);

                               if(usRC==0)
                               {
                                  if(apTokens[1]!=NULL)
                                      SaveValueToVar(apTokens[1],szVersion,&lastVariable,POINTERSTR);

                                  OutputLog( LOG_INFO, &Out,  "\tINFO ==>OTM Version     %s\n", szVersion );
                               }
                            }
                        }
                        break;

                        case EQFSEARCHFUZZYSEGMENTS_ID :
                        {
                            //setting the tmp_wc_path on the first Wildcard in the List
                            tmp_wc_path = &first_wc_path;

                            // preprocess parameters
                            if ( PreProcessParms( pCmd ) )
                            {    
                                int iMode = GetOption( apTokens[4], aFuzzySearchModes);
                                int iClass = GetOption( apTokens[5], aFuzzySearchClasses );

                                //Return Value 1 means that everything there was no ERROR
                                //Return Value 0 shouldn't happen here, because its only for 
                                //Calls which should never include a Wildcard
                                //Other Return Values mean that the path is wrong, or nothing was found, so we skip it
                                if( GetWildCardList(tmp_wc_path, pCmd, &Out) == 1)
                                {
                                    while(tmp_wc_path->next != NULL) 
                                    {
                                        usRC = EqfSearchFuzzySegments( hSession,          // Eqf session handle
                                                apTokens[1],     // name of folder
                                                tmp_wc_path->path,     // list with document names
                                                apTokens[3],     // output file name
                                                iMode,
                                                iClass,
                                                lOption );       // options for fuzzy segment search

                                        Out_RC( &Out, "EqfSearchFuzzySegments", usRC,hSession );

                                        //Shifting Pointer
                                        tmp_wc_path = tmp_wc_path->next;
                                    }     
                                }
                            }
                            //Clearing List of Paths
                            ClearList(&first_wc_path);
                        }
                        break;

                        default :
                            OutputLog( LOG_DEBUG, &Out, szMsg);
                            break;
                    } /* endswitch */
                } /* endif */
            } /* endif */

        } /* endif */
    } /* endwhile */

    // cleanup
    if ( hLoadedFile ) EqfFreeSegFile( hLoadedFile );
    if ( hSession != 0L )
    {
        //    Out_TimeStamp( &Out, "EqfEndSession" );
        EqfEndSession( hSession );
    }

    // create html based log
    if(Out.hfLog!= NULL && Out.szLogFile[0]!='\0')
    {
        char szCmd[1024+1] = {'\0'};

		char* pJavaHome = getenv("JAVA_HOME");
		if(pJavaHome == NULL)
		{
			OutputLog( LOG_ERROR, &Out, "JAVA_HOME not configured in your system\n");
		}
		else 
		{
			strcpy(szCmd,pJavaHome);
			strcat(szCmd,"\\bin\\");
			strcat(szCmd,"java -jar ");
			strcat(szCmd, getGlobalOpenTM2Path());
			strcat(szCmd,"OpenTM2ScripterGUI\\OpenTM2ScripterGUI.jar -report ");
			if(strchr(Out.szLogFile,'\\') == NULL)
			{
				char szCurDir[512+1] = {'\0'};
				getcwd(szCurDir,512);
				strcat(szCmd,szCurDir);
				strcat(szCmd,"\\");
			}
			strcat(szCmd,Out.szLogFile);
			int nRC = executeCommand(szCmd);
			if(nRC==0)
				OutputLog( LOG_INFO, &Out, "Html report created successfully\n");
		}
    }

    if ( hfScript != NULL ) fclose( hfScript );
    if ( hfLog != NULL )    fclose( hfLog );
 
    // Delete TempScript if Flag is set
    if ( deleteTempScriptFlag == 1){
        if(hfScript != NULL)
        {
            DeleteFile(szTempScriptFile);
            OutputLog(LOG_DEBUG, &Out, "\nDefault_temp_script.script was deleted\n");
        }
    }
    return( (int)usRC );
} /* end of main */

// preprocess the parameters of a non-DDE function call
// (the parameter pointers have been already stored in the apTokens array)
int PreProcessParms( PCOMMAND pCmd )
{
    int fOK = TRUE;        // function return code
    int iParm = 0;        // index of currently processed parameter

    while ( fOK && (iParm < pCmd->iNumOfParms) )
    {
        switch ( pCmd->szParmFlags[iParm] )
        {
        case OPTION_PARM :
        {
            PSZ pszOptionString = apTokens[iParm+1];
            lOption = 0L;
            if ( pszOptionString != NULL )
            {
                SKIPSPACE( pszOptionString );
                while ( *pszOptionString )
                {
                    PSZ pszOptionStart = pszOptionString;

                    SKIPSPACE( pszOptionString );

                        // isolate next option
                    while ( isalpha(*pszOptionString) || isdigit(*pszOptionString) ||
                            (*pszOptionString == '_') )
                    {
                        pszOptionString++;
                    } /* endwhile */
                    if ( *pszOptionString != NULC )
                    {
                        *pszOptionString = NULC;
                        pszOptionString++;
                        SKIPSPACE( pszOptionString );
                        if ( *pszOptionString == '+' ) pszOptionString++;
                        SKIPSPACE( pszOptionString );
                    } /* endif */

                    // check the option
                    if ( *pszOptionStart != NULC )
                    {
                        POPTION pOpt = aOptions;
                        while ( (pOpt->szOption[0] != NULC) &&
                                (_stricmp( pOpt->szOption, pszOptionStart ) != 0) )
                        {
                            pOpt++;
                        } /* endwhile */
                        if ( pOpt->szOption[0] != NULC )
                        {
                            lOption |= pOpt->lOption;
                        }
                        else
                        {
                            printf("ERROR ==> Unkown or invalid options %s detected, line is ignored.\n",
                                    pszOptionStart );
                            fOK = FALSE;
                        } /* endif */
                    } /* endif */
                } /* endwhile */
            } /* endif */
        }
        break;

        case CHARACTER_PARM :
            if ( apTokens[iParm+1] == NULL )
            {
                apTokens[iParm+1] = &chNullParm;
            } /* endif */
            break;

        case STRING_PARM :
            // change empty string parm to NULL
            if ( apTokens[iParm+1] == NULL )
            {
                // nothing to do here
            }
            else if ( *(apTokens[iParm+1]) == NULC )
            {
                apTokens[iParm+1] = NULL;
            } /* endif */
            else if ( !strcmp(apTokens[iParm+1], "NULL" ))
            {
                apTokens[iParm+1] = NULL;
            } /* endif */
            break;

        case FLOAT_PARM :
            // nothing to be checked here. atof() is applied at apropriate places
            break;

        case LONG_PARM :
            // nothing to be checked here. atol() is applied at apropriate places
            break;

        case SHORT_PARM :
            // nothing to be checked here. atoi() is applied at apropriate places
            break;

        case BOOL_PARM :
            // nothing to be checked here. atoi() is applied at apropriate places
            break;

        default :
            break;
        } /* endswitch */
        iParm++;
    } /* endwhile */
    return( fOK );
} /* end of function PreProcessParms */

LONG TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionINFO )
{
    LONG retval = EXCEPTION_CONTINUE_SEARCH;
    LPCTSTR szResult = NULL;

    // firstly see if dbghelp.dll is around and has the function we need
    // look next to the EXE first, as the one in System32 might be old
    // (e.g. Windows 2000)
    HMODULE hDll = NULL;
    char szDbgHelpPath[_MAX_PATH];
    static char szScratch [2048];
    static char szMessage [2048];

    if (GetModuleFileName( NULL, szDbgHelpPath, _MAX_PATH ))
    {
        char *pSlash = strrchr( szDbgHelpPath, '\\' );
        if (pSlash)
        {
            strcpy( pSlash+1, "DBGHELP.DLL" );
            hDll = LoadLibrary( szDbgHelpPath );
        }
    }

    if (hDll==NULL)
    {
        // load any version we can
        hDll = LoadLibrary( "DBGHELP.DLL" );
    }

    if (hDll)
    {
        MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)GetProcAddress( hDll, "MiniDumpWriteDump" );
        if (pDump)
        {
            char szDumpPath[_MAX_PATH];

            // work out a good place for the dump file
            UtlMakeEQFPath( szDumpPath, NULC, SYSTEM_PATH, NULL );
            strcat( szDumpPath, "\\OpenTM2Scripter.dmp" );

            {
                // create the file
                HANDLE hFile = CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL );

                if (hFile!=INVALID_HANDLE_VALUE)
                {
                    MINIDUMP_EXCEPTION_INFORMATION ExINFO;
                    BOOL bOK = TRUE;

                    ExINFO.ThreadId = GetCurrentThreadId();
                    ExINFO.ExceptionPointers = pExceptionINFO;
                    ExINFO.ClientPointers = FALSE;

                    // write the dump
                    bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExINFO, NULL, NULL );
                    if (bOK)
                    {
                        sprintf( szScratch, "Please send the file '%s' to the TranslationManager development team for problem evaluation.", szDumpPath  );
                        szResult = szScratch;
                        retval = EXCEPTION_EXECUTE_HANDLER;
                    }
                    else
                    {
                        sprintf( szScratch, "No dump created (Failed to save dump file to '%s' (ERROR %d))", szDumpPath, GetLastError() );
                        szResult = szScratch;
                    }
                    CloseHandle(hFile);
                }
                else
                {
                    sprintf( szScratch, "No dump created (Failed to create dump file '%s' (ERROR %d))", szDumpPath, GetLastError() );
                    szResult = szScratch;
                }
            }
        }
        else
        {
            szResult = "No dump created (DBGHELP.DLL too old)";
        }
    }
    else
    {
        szResult = "No dump created (DBGHELP.DLL not found)";
    }

    if (szResult)
    {
        strcpy( szMessage, "EQF9999: A fatal ERROR has been encountered.\n\nOpenTM2Scripter ended abnormally.\n\n" );
        strcat( szMessage, szResult );
        MessageBox( NULL, szMessage, "Abnormal program termination", MB_OK );
    } /* endif */

    return retval;
}

// output functions

/* Output log if the logLevel is equal or higher than setted by /loglevel
 * @param  logLevel -> which loglevel to used(LOGLEVEL)
 * @param  pOut     -> structure used to store buffer and file pointer(PFUNCTEST_OUTPUT)
 * @param  pformat  -> information format to print(const char*)
 */
void OutputLog(LOGLEVEL logLevel, PFUNCTEST_OUTPUT pOut, const char* pformat,...)
{
    va_list args;

    if(logLevel<pOut->logLevel)
        return;

    // process variable arguments
    va_start(args, pformat);
    memset(pOut->szBuffer,0,sizeof(pOut->szBuffer));
    vsnprintf(pOut->szBuffer,sizeof(pOut->szBuffer)/sizeof(CHAR)-1*sizeof(CHAR),pformat,args);
    va_end(args);

    Out_Buffer( pOut );
}

/* Output result, if ERROR happen, also output message
 * @param  pOut       -> structure used to store buffer and file pointer(PFUNCTEST_OUTPUT)
 * @param  pszFunction-> name of the API name(PSZ)
 * @param  usRC       -> result code(USHORT)
 * @param  hSession   -> pointer to HSESSION(HSESSION)
 */
void Out_RC( PFUNCTEST_OUTPUT pOut, PSZ pszFunction, USHORT usRC,HSESSION hSession )
{
    if(pOut == NULL)
        return;

    if(usRC==0)
    {
        OutputLog(LOG_INFO, pOut,"\tINFO ==>%s returned %u\n", pszFunction, usRC );
    }
    else
    {
        PSZ pszSource = NULL;
        USHORT usRes = 0;

        if(LOG_ERROR < pOut->logLevel)
            return;

        OutputLog(LOG_ERROR, pOut, "\tERROR ==>%s returned %u\n", pszFunction, usRC );
        
        //output error message
        if(hSession == NULL)
            return;

        EqfGetLastError( hSession, &usRes, szMsg, sizeof(szMsg) );
        OutputLog(LOG_ERROR, pOut, "\tMessage %u\n", usRes  );

        // write message text line by line to output
        pszSource = szMsg;
        do
        {
            PSZ pszTarget = pOut->szBuffer;
            *pszTarget++ = '\t';
            while ( (*pszSource != '\n') && *pszSource ) *pszTarget++ =* pszSource++;
            *pszTarget++ = '\n';
            *pszTarget = '\0';
            Out_Buffer( pOut );
            if ( *pszSource == '\n') pszSource++;
        } while ( *pszSource );
    }//end else
}

/* Output information for the program, such as help information
 * @param  pOut     -> structure used to store buffer and file pointer(PFUNCTEST_OUTPUT)
 * @param  pformat  -> information format to print(const char*)
 */
void Out_String( PFUNCTEST_OUTPUT pOut, const char* pformat,... )
{
    va_list args;

    if(pOut==NULL || pformat==NULL)
        return;

    // process variable arguments
    va_start(args, pformat);
    memset(pOut->szBuffer,0,sizeof(pOut->szBuffer));
    vsnprintf(pOut->szBuffer,sizeof(pOut->szBuffer)/sizeof(CHAR)-1*sizeof(CHAR),pformat,args);
    va_end(args);

    Out_Buffer( pOut );
}

/* Output strings in buffer to file handle or stdout
 * @param  pOut     -> structure used to store buffer and file pointer(PFUNCTEST_OUTPUT)
 */
void Out_Buffer( PFUNCTEST_OUTPUT pOut )
{
    if ( pOut->szLogFile[0]!='\0')
    { 
        if(pOut->hfLog == NULL)
            pOut->hfLog = fopen( pOut->szLogFile, "w" );
    }

    if ( pOut->fDateTime )
    {
        CHAR szDateTime[20];
        GetDateTime( szDateTime );

        if ( pOut->hfLog  )
        {
            fprintf( pOut->hfLog, szDateTime  );
        }
 
        printf( szDateTime );
    }

    if ( pOut->fProcessID )
    {
        DWORD dwProcessID = GetCurrentProcessId();
        if ( pOut->hfLog  )
        {
            fprintf( pOut->hfLog, "%5ld ", dwProcessID );
        }

        printf( "%5ld ", dwProcessID );
    }

    if ( pOut->hfLog  )
    {
        fprintf( pOut->hfLog, "%s", pOut->szBuffer );
        fflush(pOut->hfLog);
    }
    {    
        printf( "%s", pOut->szBuffer );
        fflush(stdout);
    }
    
    pOut->szBuffer[0] = NULC;
}

/* Output help information
 * @param  pOut     -> structure used to store buffer and file pointer(PFUNCTEST_OUTPUT)
 */
void Out_Help(PFUNCTEST_OUTPUT Out){
    Out_String( Out, "No Parameters Specified -> Help Mode active\n-----------------------------------------\n\n");
    Out_String( Out, "Possible Parameters:\n\n");
    
    Out_String( Out, "Scriptfile: \tPath to the script, which should be excecuted\n");
    Out_String( Out, "\t\tif no script is specified OpenTM2.SCR is used\n");
    Out_String( Out, "\t\tSyntax: Only the name, no special syntax\n");
    Out_String( Out, "\t\tExample: Script.txt or ..\\anyFolder\\anyScript.txt\n\n");
    
    Out_String( Out, "Logfile: \tPath to the file in which the log should be written\n");
    Out_String( Out, "\t\tSyntax: /log= or /logfile= and then the path\\name\n");
    Out_String( Out, "\t\tExample: /logfile=log.txt or /log=..\\anyFolder\\anyLog.txt\n\n");

    Out_String( Out, "LogLevel: \tLog level for this this script\n");
    Out_String( Out, "\t\tSyntax: /loglevel=[DEBUG | INFO | WARNING | ERROR | NO]\n");
    Out_String( Out, "\t\t        ERROR is the hightest priority, log only output\n");
    Out_String( Out, "\t\t        if its priority is equal or higher than /loglevel setted\n");
    Out_String( Out, "\t\tExample: /logLevel=WARNING, WARNING and ERROR log outputed\n\n");

    Out_String( Out, "Constants: \tConstants which should be attached to the script\n");
    Out_String( Out, "\t\tSyntax: /constant=constant_Name,constant_Value\n");
    Out_String( Out, "\t\tfor example /constant=my_foldername, AnyRandomFoldername\n\n");
    
    Out_String( Out, "INFO: \t\tFlags for Datetime or ProcessID Output\n");
    Out_String( Out, "\t\tSyntax: /INFO=d for Datetime\n");
    Out_String( Out, "\t\tSyntax: /INFO=p for ProcessID\n\n");
    
    Out_String( Out, "Help: \t\tGives this Help Output, other Parameters will be ignored\n");
    Out_String( Out, "\t\tSyntax: /HELP\n\n");
    
    Out_String( Out, "DelTemp: \tFlag for deleting the TempScript after excecution\n");
    Out_String( Out, "\t\tSyntax: /delTemp\n\n");

    Out_String( Out, "Examplecall: OpenTM2Scripter.exe script.scr /log=log.txt /loglevel=WARNING /Info=d /Info=p /constant=any_constant,any_value\n");
    Out_String( Out, "-----------------------------------------\n\n");
}

void GetDateTime( PSZ pszBuffer )
{
    CHAR szDate[12], szDate2[12], szTime[12];

    _strtime( szTime );
    _strdate( szDate );

    // strdat formats the date in the form mm/dd/yy  so we have to exchange the fields
    szDate2[0] = szDate[6];
    szDate2[1] = szDate[7];
    szDate2[2] = szDate[2];
    szDate2[3] = szDate[0];
    szDate2[4] = szDate[1];
    szDate2[5] = szDate[5];
    szDate2[6] = szDate[3];
    szDate2[7] = szDate[4];
    szDate2[8] = '\0';

    sprintf( pszBuffer, "%s %s ", szDate2, szTime );
}

// get the value for an option
USHORT GetOption( PSZ pszOption, POPTION pOpt )
{
    USHORT usOption = 0;

    while ( (pOpt->szOption[0] != NULC) && (_stricmp( pOpt->szOption, pszOption ) != 0) )
    {
        pOpt++;
    } /* endwhile */

    if ( pOpt->szOption[0] != NULC )
    {
        usOption = (USHORT)pOpt->lOption;
    } /* endif */

    return( usOption );
}

/*
 *Preprocessing the Script to a new Script, handles markers, const, if etc.
 *@param hfScript source file / Script file from user as FILE*
 *@param targetScript targetScript as FILE*
 *@pfirst_marker pointer to the first marker as PMARKER
 *@Out as PFUNCTEST_OUTPUT
 *@lastConsta pointer to the pointer to latest processed constant (to NULL at the programm start) as PCONSTA*
 */
void PreProcessScript(FILE* hfScript, FILE* targetScript,PMARKER pfirst_marker, PFUNCTEST_OUTPUT Out, PCONSTA* lastConsta)
{
    int i;

    char buffer[10],markername[50];
    USHORT sCurLine = 0;
    PCOMMAND pCmd;                       // ptr to currently active command
    //Variables for include-handling
    PINCLIST firstinc=0,tmpinc=0;
    FILE *includeScript=0;
    char includepath[1024], szIncludeFile[1024];
    //variables for if handling
    int ifid=0; //Counter for if handling
    PIFLIST firstif=0,tmpif=0;
    //variables for FOR handling
    int forid=0; //Counter for FOR handling
    PFORLIST firstfor=0, tmpfor=0;

    // for TestCase, TestResult
    int fOK = 1;
    const int MAXCASENAMELEN = 512;
    int fTestCase = 0;
    int iTestResult = 0;
    char* pszTcName = NULL;
    char* pszTcDes  = NULL;
    char* pszBuf    = NULL;

    //save path for include
    strcpy(includepath,szScriptFile);
    GetBasePath(includepath,includepath);
    // loop through script file

    pszTcName = (char*)malloc((MAXCASENAMELEN+1)*sizeof(char));
    pszTcDes  = (char*)malloc((MAXCASENAMELEN*2+1)*sizeof(char));
    pszBuf    = (char*)malloc((MAXCASENAMELEN+1)*sizeof(char));
    if(pszTcName==NULL || pszTcDes==NULL || pszBuf==NULL)
    {
        fOK = 0;
    }

    while ( fOK && !feof(hfScript) )
    {
        // get next line
        memset( szTempLine, 0, sizeof(szTempLine) );
        memset( szLine, 0, sizeof(szLine) );
        fgets( szTempLine, sizeof(szTempLine), hfScript );
        
        // replace all constants in current line with saved String
        if(ReplaceConstants( szTempLine, szLine , lastConsta, Out))
            szLine[0]=0;
        
        // process the line
        if ( szLine[0] != '\0' )
        {
            int iStop = 0;

            // remove trailing LF
            {
                int i = strlen(szLine);
                if ( szLine[i-1] == '\n' )
                {
                    szLine[i-1] = '\0';
                } /* endif */
            }

            //remove Spaces and Tabs
            {
                int j = 0;
                while(szLine[j] == ' ' || szLine[j] == 9) j++;
                if( szLine[j] == NULC ) iStop = 1; 
            }
            
            // process the line
            if ( (szLine[0] == NULC) || iStop )
            {
                // skip comments and empty lines
            }
            else if(szLine[0] == '*')
            {
                // print the Comments in the tempScript, that they are shown later
                sCurLine++;
                fprintf(targetScript, "%s\n", szLine);
            }
            else
            {
                sCurLine++;
                // split line into tokens
                {
                    PSZ pszCurPos;                 // current position within line

                    // initialize token array
                    memset( apTokens, 0, sizeof(apTokens) );
                    iTokens = 0;

                    // get first value as command token
                    pszCurPos = szLine;
                    SKIPSPACE( pszCurPos );      // skip leading white space
                    if ( *pszCurPos != NULC )
                    {
                        apTokens[iTokens++] = pszCurPos;
                        while ( (*pszCurPos != NULC) && (*pszCurPos != ' ') )
                        {
                            pszCurPos++;
                        } /* endwhile */
                        if ( *pszCurPos != NULC )
                        {
                            *pszCurPos = NULC;
                            pszCurPos++;
                        } /* endif */
                    } /* endif */

                    // scan remaining line for tokens
                    while ( *pszCurPos != NULC )
                    {
                        SKIPSPACE( pszCurPos );      // skip leading white space

                        // handle current token
                        if ( *pszCurPos == '\"' )
                        {
                            // remember token start
                            // keep the \" in the tokens
                            //pszCurPos++;
                            apTokens[iTokens++] = pszCurPos++;

                            // look for end of token
                            while ( (*pszCurPos != '\"') && (*pszCurPos != NULC) )
                            {
                                pszCurPos++;
                            } /* endwhile */

                            // terminate token and skip any delimiters
                            if ( *pszCurPos != NULC )
                            {
                                pszCurPos++;             // keep the \" in the apTokens
                                char tempCh = *pszCurPos;
                                *pszCurPos = NULC;       // terminate token
                                if(tempCh == ',')
                                    pszCurPos++;         // move to next, this parameter end
                                else
                                {
                                   pszCurPos++; 
                                   SKIPSPACE( pszCurPos );  // skip any whitespace
                                   SKIPCOMMA( pszCurPos );  // skip any delimiting comma
                                }
                            } /* endif */
                        }
                        else
                        {
                            // remember token start
                            apTokens[iTokens++] = pszCurPos;

                            // look for end of token
                            while ( (*pszCurPos != ',') && (*pszCurPos != NULC) )
                            {
                                pszCurPos++;
                            } /* endwhile */

                            // terminate token and skip any delimiters
                            if ( *pszCurPos != NULC )
                            {
                                *pszCurPos = NULC;       // terminate token
                                pszCurPos++;             // continue with next one
                            } /* endif */
                        } /* endif */
                    } /* endwhile */
                }

                // check first token against commands
                {
                    pCmd = aCommands;
                    while ( (pCmd->szCommand[0] != NULC) && (_stricmp( pCmd->szCommand, apTokens[0] ) != 0) )
                    {
                        pCmd++;
                    } /* endwhile */

                    if ( pCmd->szCommand[0] == NULC )
                    {
                        fprintf(targetScript, "%s", apTokens[0]);
                        for (i=1;i<iTokens;i++)
                        {
                            fprintf( targetScript, " %s", apTokens[i]);
                            if( i != iTokens-1 ) fprintf(targetScript, ",");
                        }
                        fprintf( targetScript, "\n");
                    } /* endif */
                }

                // handle valild commands
                if ( pCmd->szCommand[0] != NULC )
                {
                    switch ( pCmd->ID )
                    {

                    case INTFOR_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            if (apTokens[1]!=NULL&&apTokens[2]!=NULL&&apTokens[4]!=NULL)
                            {
                                char* tmpchar;
                                forid++;
                                tmpfor=firstfor;
                                //create new element in FOR list if theres no elemtent
                                if (firstfor==0)
                                {
                                    tmpfor=(PFORLIST)malloc(sizeof(FORLIST));
                                    firstfor=tmpfor;
                                }
                                //create new element at the end of the list
                                else
                                {
                                    while (tmpfor->next!=0) tmpfor=tmpfor->next;
                                    tmpfor->next=(PFORLIST)malloc(sizeof(FORLIST));
                                    tmpfor=tmpfor->next;
                                }
                                //Set standart values in created for list element
                                tmpfor->next=0;
                                tmpfor->id=forid;
                                strcpy(tmpfor->varname,apTokens[1]);
    
                                tmpchar=apTokens[4];
                                while (*tmpchar!=NULC&&*tmpchar!=' ')
                                {
                                    tmpchar++;
                                }
                                if (*tmpchar!=NULC)
                                {
                                    *tmpchar=NULC;
                                    tmpchar++;
                                }
    
                                if ( strcmp(apTokens[4],"INC")==0 ) tmpfor->action=1;
                                else if ( strcmp(apTokens[4],"DEC")==0 ) tmpfor->action=2;
                                else if ( strcmp(apTokens[4],"ADD")==0 ) tmpfor->action=3;
                                else if ( strcmp(apTokens[4],"SUB")==0 ) tmpfor->action=4;
                                else
                                {
                                    OutputLog( LOG_WARNING, Out,"\tWARNING==> Invalid action '%s' in for command changed to INC\n",apTokens[4]);
                                    tmpfor->action=1;
                                }
    
                                if (tmpfor->action>=3){
                                    tmpfor->actionvalue=atoi(tmpchar);
                                }
    
                                //Set the start value for variable
                                fprintf( targetScript, "SETVALUE %s, %s\n", apTokens[1], apTokens[2]);
                                //Set beginning marker
                                strcpy(markername,"for_marker_begin_");
                                _itoa(tmpfor->id,buffer,10);
                                strcat(markername,buffer);
                                SaveMarker(pfirst_marker,targetScript,Out,sCurLine,markername);
                                fprintf( targetScript, "MARKER for_marker_begin_%d\n", tmpfor->id);
                                //replace FOR Command with GOTOCOND Command
                                fprintf( targetScript, "GOTOCOND !%s, for_marker_end_%d\n", apTokens[3],tmpfor->id);
                                if (apTokens[3]==NULL)
                                {
                                    OutputLog(LOG_WARNING, Out,"\tWARNING==> No condition setted for FOR loop. Loop block will never be reached\n");
                                }
                            }
                            else
                            {
                                OutputLog(LOG_ERROR,Out,"\tERROR ==> Not enougth parameters for FOR command. Variable name, start value and action are required\n");
                            }
                        }
                    }
                    break;

                    case INTENDFOR_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            if (firstfor!=NULL)
                            {
                                //Select Commant for counting
                                if (tmpfor->action==1) fprintf( targetScript, "INC %s\n",tmpfor->varname);
                                if (tmpfor->action==2) fprintf( targetScript, "DEC %s\n",tmpfor->varname);
                                if (tmpfor->action==3) fprintf( targetScript, "ADD %s, %d\n",tmpfor->varname,tmpfor->actionvalue);
                                if (tmpfor->action==4) fprintf( targetScript, "SUB %s, %d\n",tmpfor->varname,tmpfor->actionvalue);
                                //GOTO to the beginning of the for block
                                fprintf( targetScript, "GOTO for_marker_begin_%d\n",tmpfor->id);
                                //create markername
                                strcpy(markername,"for_marker_end_");
                                _itoa(tmpfor->id,buffer,10);
                                strcat(markername,buffer);
                                //create endmarker for the forblock
                                SaveMarker(pfirst_marker,targetScript,Out,sCurLine,markername);
                                fprintf( targetScript, "MARKER for_marker_end_%d\n",tmpfor->id);
                                //Delete closed for block from the for list
                                tmpfor=firstfor;
                                if (tmpfor->next==0)
                                {
                                    free(tmpfor);
                                    firstfor=0;
                                }
                                else
                                {
                                    while (tmpfor->next->next!=0) tmpfor=tmpfor->next;
                                    free(tmpfor->next);
                                    tmpfor->next=0;
                                }
                            }
                            else
                            {
                                OutputLog(LOG_ERROR, Out,"\tERROR ==> There was no FOR open before ENDFOR command. ENDFOR is ignored\n");
                            }
                        }
                    }
                    break;

                    case INTINCLUDE_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            if (apTokens[1]!=NULL)
                            {
                                //read the current include basepath in szIncludeFile (relative to the scripter)
                                strcpy(szIncludeFile,includepath);
                                //put include token at the end of the basepath
                                strcat(szIncludeFile,apTokens[1]);
                                includeScript = fopen( szIncludeFile, "r" );  //Try to open included file for read
                                //Catch ERRORs by open the file
                                if ( includeScript == NULL )
                                {   
                                    OutputLog(LOG_ERROR, Out,"\tERROR ==> Open of included script '%s' file failed, include is ignored\n",szIncludeFile);
                                } /* endif */
                                //Set Variables for comming processing of includes file
                                else
                                {
                                    //Save position of current Sciptfile in INCLUDE LIST
                                    OutputLog(LOG_INFO,Out,"\tINFO ==> include script file '%s'\n",szIncludeFile);
                                    fprintf(targetScript, "\n*----------->Included from Script %s<--------------*\n", apTokens[1]);
                                    if (firstinc==0)
                                    {
                                        firstinc=(PINCLIST)malloc(sizeof(INCLIST));
                                        tmpinc=firstinc;
                                    }
                                    else
                                    {
                                        tmpinc=firstinc;
                                         while (tmpinc->next!=0) tmpinc=tmpinc->next;
                                         tmpinc->next=(PINCLIST)malloc(sizeof(INCLIST));
                                         tmpinc=tmpinc->next;
                                    }
                                    tmpinc->filepointer=hfScript;
                                    tmpinc->next=0;
                                    strcpy(tmpinc->name, apTokens[1]);
                                    strcpy(tmpinc->path,includepath);
                                    //Set processed Skript hfScript=includeScript --> include Script will be processed
                                    hfScript=includeScript;
                                    //change basepath for include to basepath of the current hfScript/includeScript
                                    strcpy(includepath,szIncludeFile);
                                    GetBasePath(includepath,includepath);
                                }
                            }
                            else
                            {
                                    OutputLog(LOG_ERROR, Out,"\tERROR ==> Not enought parameter for INCLUDE command. Path to file is required\n");
                            }
                        }
                    }
                    break;

                    case INTIF_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            ifid++;
                            tmpif=firstif;
                            //create new element in if list if theres no elemtent
                            if (firstif==0)
                            {
                                tmpif=(PIFLIST)malloc(sizeof(IFLIST));
                                firstif=tmpif;
                            }
                            //create new element at the end of the list
                            else
                            {
                                while (tmpif->next!=0) tmpif=tmpif->next;
                                tmpif->next=(PIFLIST)malloc(sizeof(IFLIST));
                                tmpif=tmpif->next;
                            }
                            //Set standart values in created if list element
                            tmpif->next=0;
                            tmpif->id=ifid;
                            tmpif->lastCond=0;
                            //replace the if in the targetScript with an GOTOCOND command
                            fprintf( targetScript, "GOTOCOND !%s, if_marker_%d_%d\n", apTokens[1], tmpif->id, tmpif->lastCond);
                            if (apTokens[1]==NULL)
                            {
                                OutputLog(LOG_WARNING, Out,"\tWARNING ==> No condition setted for if command. If block will never be reached.\n");
                            }
                        }
                    }
                    break;

                    case INTELSEIF_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            //set GOTO comand in targetScript for jumping to the end of the if block when if condition
                            //or earlier elseif condition was true
                            if (firstif!=NULL)
                            {
                                fprintf( targetScript, "GOTO if_marker_end_%d\n",tmpif->id);
                                //create markername
                                strcpy(markername,"if_marker_");
                                _itoa(tmpif->id,buffer,10);
                                strcat(markername,buffer);
                                strcat(markername,"_");
                                _itoa(tmpif->lastCond,buffer,10);
                                strcat(markername,buffer);
                                //create marker as junmpingpoint when all earlier conditions was false
                                SaveMarker(pfirst_marker,targetScript,Out,sCurLine,markername);
                                fprintf( targetScript, "MARKER if_marker_%d_%d\n",tmpif->id, tmpif->lastCond);
                                tmpif->lastCond++;
                                //Replace elseif in targetScript with an GOTOCOND command
                                fprintf( targetScript, "GOTOCOND !%s, if_marker_%d_%d\n", apTokens[1], tmpif->id, tmpif->lastCond);
                                if (apTokens[1]==NULL)
                                {
                                    OutputLog(LOG_WARNING, Out,"\tWARNING ==> No condition setted for ELSEIF command. elseif block will never be reached.\n");
                                }
                            }
                            else
                            {
                                OutputLog(LOG_ERROR, Out,"\tERROR ==> There was no if opened before ELSEIF command. ELSEIF is ignored\n");
                            }
                        }
                    }
                    break;

                    case INTELSE_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            if (firstif!=NULL)
                            {
                                //set GOTO comand in targetScript for jumping to the end of the if block when if condition
                                //or earlier elseif condition was true
                                fprintf( targetScript, "GOTO if_marker_end_%d\n",tmpif->id);
                                //create markername
                                strcpy(markername,"if_marker_");
                                _itoa(tmpif->id,buffer,10);
                                strcat(markername,buffer);
                                strcat(markername,"_");
                                _itoa(tmpif->lastCond,buffer,10);
                                strcat(markername,buffer);
                                //create marker as junmpingpoint when all earlier conditions was false
                                SaveMarker(pfirst_marker,targetScript,Out,sCurLine,markername);
                                tmpif->lastCond++;
                                fprintf( targetScript, "MARKER if_marker_%d_%d\n",tmpif->id, tmpif->lastCond);
                            }
                            else
                            {
                                 OutputLog(LOG_ERROR, Out,"\tERROR ==> There was no if opened before ELSEIF command. ELSEIF is ignored\n");
                            }
                        }
                    }
                    break;

                    case INTENDIF_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            if (firstif!=NULL)
                            {
                                //create markername
                                strcpy(markername,"if_marker_");
                                _itoa(tmpif->id,buffer,10);
                                strcat(markername,buffer);
                                strcat(markername,"_");
                                _itoa(tmpif->lastCond,buffer,10);
                                strcat(markername,buffer);
                                //create marker as junmpingpoint when all earlier conditions was false and no else case exist
                                SaveMarker(pfirst_marker,targetScript,Out,sCurLine,markername);
                                fprintf( targetScript, "MARKER if_marker_%d_%d\n",tmpif->id, tmpif->lastCond);
                                //create markername
                                strcpy(markername,"if_marker_end_");
                                _itoa(tmpif->id,buffer,10);
                                strcat(markername,buffer);
                                //creade end marker for the if block
                                SaveMarker(pfirst_marker,targetScript,Out,sCurLine,markername);
                                fprintf( targetScript, "MARKER if_marker_end_%d\n",tmpif->id);
                                //Delete closed if block from the if list
                                tmpif=firstif;
                                if (tmpif->next==0)
                                {
                                    free(tmpif);
                                    firstif=0;
                                }
                                else
                                {
                                    while (tmpif->next->next!=0) tmpif=tmpif->next;
                                    free(tmpif->next);
                                    tmpif->next=0;
                                }
                            }
                            else
                            {
                                   OutputLog(LOG_ERROR,Out,"\tERROR ==> There was no if opened before ENDIF command. ENDIF is ignored\n");
                            }
                        }
                    }
                    break;

                    case INTDEFINE_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            if (apTokens[1]!=NULL&&apTokens[2]!=NULL)
                            {
                                SaveConsta(apTokens[1],apTokens[2],lastConsta);
                                OutputLog( LOG_DEBUG, Out, "\tsave constant '%s'\n", apTokens[1] );
                                fprintf( targetScript, "%s %s,%s\n", apTokens[0], apTokens[1], apTokens[2]);
                            }
                            else
                            {
                                OutputLog(LOG_ERROR, Out,"\tERROR ==> Not enought parameter for DEFINE command. name and value are required\n");
                            }
                        }
                    }
                    break;

                    case INTGOTO_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            if (apTokens[1]==NULL)
                            {
                                OutputLog(LOG_ERROR,Out,"\tERROR ==> Not enought parameter for GOTO command. Destination is required\n");
                            }
                            else
                            {
                                fprintf( targetScript, "%s %s\n", apTokens[0], apTokens[1]);    
                            }
                        }
                    }
                    break;

                    case INTGOTOCOND_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            if (apTokens[2]!=NULL)
                            {
                                fprintf( targetScript, "%s %s,%s\n", apTokens[0], apTokens[1],apTokens[2]);
                                if (apTokens[1]==NULL)
                                {
                                    OutputLog(LOG_WARNING,Out,"\tWARNING ==> No condition setted for GOTOCOND command. Condition will never be true.\n");
                                }
                            }
                            else
                            {
                                OutputLog(LOG_ERROR,Out,"\tERROR ==> Not enought parameter for GOTOCOND command. Destination is required\n");
                            }
                        }
                    }
                    break;

                    case INTMARKER_ID :
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            if(apTokens[1] != NULL)
                            {
                                //Save Marker
                                SaveMarker(pfirst_marker,targetScript,Out,sCurLine,apTokens[1]);
                                fprintf( targetScript, "%s %s\n", apTokens[0],apTokens[1]);
                            }
                            else
                            {
                                 OutputLog(LOG_ERROR, Out, "\tERROR ==> unable to save marker: No name parameter setted\n" );
                            }
                        } /* endif */
                    }
                    break;

                    case INTTESTCASE_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            if (apTokens[1]!=NULL && apTokens[2]!=NULL)
                            {
                                if(fTestCase==0)
                                {   
                                    pszTcName[MAXCASENAMELEN]='\0';
                                    pszTcDes[MAXCASENAMELEN*2]='\0';

                                    // test case name without ""
                                    trim(apTokens[1], '"', pszTcName,MAXCASENAMELEN);

                                    // test case description
                                    trim(apTokens[2], '"', pszTcDes,MAXCASENAMELEN);

                                    fprintf(targetScript, "ECHO %s[%s]=>Begin\n", pszTcName,pszTcDes);
                                    fTestCase = 1;
                                    
                                }
                                else
                                {
                                    OutputLog(LOG_ERROR, Out,"\tERROR ==> TestCase can't be nested.");
                                }
                            }
                            else
                            {
                                OutputLog(LOG_ERROR, Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored");
                            }
                        }
                    }
                    break;

                    case INTEND_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {
                            if(fTestCase==1)
                            {
                                strcpy(markername,pszTcName);
                                strcat(markername,"_END");
                                SaveMarker(pfirst_marker,targetScript,Out,sCurLine,markername);
                                fprintf( targetScript, "MARKER %s\n",markername);
                                sCurLine++;

                                fprintf(targetScript, "ECHO %s[%s]=>Finish\n", pszTcName,pszTcDes);
                                fTestCase = 0;
                                iTestResult = 0;
                            }
                            else
                            {
                                OutputLog(LOG_ERROR, Out,"\tERROR ==> END should be matched with TESTCASE.");
                            }
                        }
                    }
                    break;

                    case INTTESTRESULT_ID:
                    {
                        if ( PreProcessParms( pCmd ) )
                        {       
                            if (fTestCase==1 && apTokens[1]!=NULL && apTokens[2]!=NULL)
                            {
                                pszBuf[MAXCASENAMELEN] = '\0';

                                fprintf(targetScript, "SAVERETURNCODE result\n");
                                sCurLine++;

                                sprintf(markername,"ML%d_END",sCurLine+3);
                                iTestResult++;

                                trim(apTokens[1],'"', pszBuf, MAXCASENAMELEN);
                                fprintf( targetScript, "GOTOCOND $result$==%s, %s\n", pszBuf,markername);
                                sCurLine++;

                                trim(apTokens[2],'"', pszBuf, MAXCASENAMELEN);
                                fprintf(targetScript,  "ECHO \"TESTRESULT=>%s\"\n",pszBuf);
                                sCurLine++;

                                fprintf(targetScript,"GOTO %s_END\n",pszTcName);
                                sCurLine++;
                                SaveMarker(pfirst_marker,targetScript,Out,sCurLine,markername);
                                fprintf( targetScript, "MARKER %s\n",markername);
                            }
                            else
                            {
                                if(fTestCase==1)
                                    OutputLog(LOG_ERROR, Out,"\tERROR ==> Not enougth parameters for this command. Line is ignored");
                                else
                                    OutputLog(LOG_ERROR, Out,"\tERROR ==> TestResult key word only could used in TESTCASE body.");
                            }
                        }
                    }
                    break;

                    default :
                    {
                        fprintf(targetScript, "%s", apTokens[0]);
                        for (i=1;i<iTokens;i++)
                        {
                            fprintf( targetScript, " %s", apTokens[i]);
                            if( i != iTokens-1 ) fprintf(targetScript, ",");
                        }
                        fprintf( targetScript, "\n");
                    }
                    break;

                    } /* endswitch */

                } /* endif */
            } /* endif */
        } /* endif */
        
        //Hande the end of included script files
        while ( feof(hfScript) && firstinc!=0)
        {
            tmpinc=firstinc;
            //Delete the first element in include list if there is only one element
            if (tmpinc->next==0)
            {
                fclose(hfScript);
                //reload hfScript position and includepath from before the include
                OutputLog( LOG_INFO,Out, "\tINFO ==> File successfully included\n");
                fprintf(targetScript, "*-----------> end of Script %s<--------------*\n\n", tmpinc->name);
                hfScript=tmpinc->filepointer;
                strcpy(includepath,tmpinc->path);
                free(tmpinc);
                firstinc=0;
            }
            //Delete the last element in include list
            else
            {
                while (tmpinc->next->next!=0) tmpinc=tmpinc->next;
                fclose(hfScript);
                //reload hfScript position and includepath from before the include
                fprintf(targetScript, "*-----------> end of Script %s<--------------*\n\n", tmpinc->name);
                OutputLog( LOG_INFO, Out, "\tINFO ==> File successfully included\n");
                hfScript=tmpinc->next->filepointer;
                strcpy(includepath,tmpinc->next->path);
                free(tmpinc->next);
                tmpinc->next=0;
            }
        }/* endwhile */
        
    } /* endwhile */

    //release resources
    if(pszTcName != NULL)
        free(pszTcName);

    if(pszTcDes != NULL)
        free(pszTcDes);

    if(pszBuf!=NULL)
        free(pszBuf);
}

USHORT EscapeBackslash(char *pString) {
    char tempString[1024];
    USHORT iCount=0;
    USHORT sShift = 0;
    USHORT sLast = 0;

    while(pString[iCount] != NULC)
    {
        if(pString[iCount] == '\\')
        {
            tempString[iCount+sShift] = pString[iCount];
            sShift+=1;
            sLast = iCount;
        }
        tempString[iCount+sShift] = pString[iCount];
        iCount++;
    }
    tempString[iCount+sShift]=NULC;
    strcpy(pString,tempString);
    return sLast;
}

/*
 * function returns the path to a folder from a givin path to a file
 * Example: C:/Documents and Settings/test.txt --> C:/Documents and Settings/
 * Example: test.txt --> NULC
 * Example: myfiles\folder\test.txt --> myfiles\folder\
 * @param destString pointer to the fist character of the destination string as char*
 * @param sourceString pointer to the first character of the source string as char*
 */
void GetBasePath(char *destString, char* sourceString) {
    short iCount=0;
    short sLast=0;
    //finde the last / oder \ in sourceString
    while(sourceString[iCount] != NULC)
    {
        if(sourceString[iCount] == '\\'||sourceString[iCount] == '/') sLast = iCount;
        iCount++;
    }
    //replace character after last / or \ with NULC
    //raplace first character with NULC if there is no / oder \  //
    if (sLast==0) sLast--;
    sourceString[sLast+1]=NULC;
    strcpy(destString,sourceString);
}

// Checks whether a String contains a Wildcard or not
int IsWildcard(char *TempLine){
    int iCount;

    if(TempLine == NULL) return 0;
    
    iCount = strlen(TempLine)-1;
    
    // Run from the right side, until the first Backslash appears
    while(TempLine[iCount] != '\\'){
        if(TempLine[iCount] == '*' || TempLine[iCount] == '?' ) return 1;
        iCount--;
    }
    return 0;
}


void SaveMarker(PMARKER pfirst_marker, FILE* targetScript, PFUNCTEST_OUTPUT Out, USHORT sCurLine, char* markername){
    //create a copy of pfirst_marker in tmpmarker
    PMARKER tmpmarker=pfirst_marker;
    //Go to the end of the markerlist or to the marker with the same markername
    if ( strcmp(tmpmarker->name,"")!=0 )
    {
        while (tmpmarker->next!=NULL && strcmp(tmpmarker->name,markername)!=0 ) tmpmarker=tmpmarker->next;
    } /* endif */
    //if Marker with same name already exists ignore the deklaration an show the warning
    if( strcmp(tmpmarker->name,markername)==0 ){
        OutputLog( LOG_WARNING, Out, "\tWARNING ==> marker '%s' already exists\n", markername );
        //create new Element in markerlist
    }else{
        OutputLog( LOG_DEBUG,Out, "\tsave marker '%s'\n", markername );
        if ( strcmp(pfirst_marker->name,"")!=0 ){
            tmpmarker->next=(PMARKER)malloc(sizeof(MARKER));
            tmpmarker=tmpmarker->next;
        }/* endif */
        //Save markername, linenumber and a copy of current targetScript filepointer in the new element
        strcpy(tmpmarker->name,markername);
        tmpmarker->next=NULL;
        tmpmarker->ln=sCurLine;
        fgetpos(targetScript,&tmpmarker->tmpScript);
    }/* endif */
}

/* Function to Replace all Constants in a line
 * @param input InputLine as char*(PSZ)
 * @param value OutputLine as char*(PSZ)
 * @param *lastConsta Pointer to the latest created Constant(PCONSTA)
 * @param Out textbuffer for text output(PFUNCTEST_OUTPUT)
 * @return 0 Everything worked fine
 * @return 1 Replacing Constant makes the line to long.
 * @return 2 The Constant does not exist.
 * @return 3 No Constants defined.
 */
int ReplaceConstants(PSZ input,PSZ output, PCONSTA *lastConsta, PFUNCTEST_OUTPUT Out)
{
    {
        int length=2048;
        PCONSTA tC;
        CHAR temp[102];
        PSZ temp2;
        DWORD dwProcessID;
        while(*input!=0){
            while(*input!='%'&&*input!=0)
            {
                dwProcessID = GetCurrentProcessId();
                length--;
                if(length>0)  *(output++)=*(input++);
                else
                {
                    OutputLog(LOG_WARNING, Out, "Replacing Variables make Line to long. \n");
                    return 1;
                }

            }
            if ( _strnicmp( input, "%procid%", 8 ) == 0 && *input!=0)
            {
                input += 8;
                length-=8;
                if(length>0){
                    dwProcessID = GetCurrentProcessId();
                    sprintf( output, "%8.8X", dwProcessID );
                    output += strlen(output);}
                else
                {
                    OutputLog( LOG_WARNING, Out, "Replacing constants make Line to long(max 2048 chars). ERROR while replacing procid.\n");
                    return 1;
                }
            }
            else
            if (*input!=0)
            {
                input++;
                temp2=temp;
                while(*input!='%'&&*input!=0)
                {
                    *temp2++=*input++;
                }
                *temp2=0;
                input++;

                tC=*lastConsta;
                if(tC==NULL)
                {
                    OutputLog( LOG_WARNING,Out, "No constants defined.\n");
                    return 3;
                }
                while(tC!=NULL&&_stricmp(tC->name,temp))
                    {
                       tC=tC->next;
                    }
                if(tC!=NULL)
                {
                    length-=strlen(tC->value);
                    if(length>=0)
                    {
                    strcpy(output,tC->value);
                    output+=strlen(tC->value);
                    }
                    else
                    {   
                        OutputLog( LOG_WARNING, Out, "Replacing constants make Line to long. ERROR while replacing %s.\n", temp);
                        return 1;
                    }
                }
                else
                {
                    OutputLog( LOG_WARNING,Out, "constant '%s' not defined\n", temp);
                    return 2;
                }
            }
        }
        return 0;
    }
}

/* Function to save a Constant in the dynamic single linked List
 * @param name Name(max. 50 chars) of Constant as char*(PSZ)
 * @param value Text(max. 511 chars) of Constant as char*(PSZ)
 * @param *lastConsta Pointer to the latest created Constant(PCONSTA)
 */
void SaveConsta(PSZ name, PSZ value, PCONSTA *lastConsta)
{
    PCONSTA newConsta=(PCONSTA)malloc(sizeof(CONSTA));
    strcpy(newConsta->name,name);
    strcpy(newConsta->value,value);
    newConsta->next=*lastConsta;
    *lastConsta=newConsta;
}

/*
 * Save an integer to a variable, when no variable with the given name exists a new one is created.
 * @return 0 Variable found
 * @return 1 Variable not found and a new variable created
 */
int SaveValueToVar(char* name, void* pValue, PVARIABLE *lastVariable, Type type)
{
    PVARIABLE tV;
    tV=*lastVariable;

    if(tV==NULL)
    {
        tV=(PVARIABLE)malloc(sizeof(VARIABLE));
        strcpy(tV->name,name);
        tV->type = type;

        if(tV->type==INTEGER)
            tV->value.iVal = *((int*)pValue);
        else if(tV->type==POINTERSTR)
            tV->value.pszVal = (char*)pValue;

        tV->next=*lastVariable;
        *lastVariable=tV;
        return 1;
    }
    else
    {
        while(tV->next!=NULL&&_stricmp(tV->name,name))
        {
           tV=tV->next;
        }
        if(!_stricmp(tV->name,name))
        {
            if(tV->type==INTEGER)
                tV->value.iVal = *((int*)pValue);
            else if(tV->type==POINTERSTR)
                tV->value.pszVal = (char*)pValue;

            return 0;
        }
        else
        {
            tV=(PVARIABLE)malloc(sizeof(VARIABLE));
            strcpy(tV->name,name);
            tV->type = type;

            if(tV->type==INTEGER)
                tV->value.iVal = *((int*)pValue);
            else if(tV->type==POINTERSTR)
                tV->value.pszVal = (char*)pValue;

            tV->next=*lastVariable;
            *lastVariable=tV;
            return 1;
        }
    }
}

/*
 * Increments a variable, when no variable with the given name exists a new one is created.
 * @return 0 Variable found
 * @return 1 Variable not found an new variable created and initialized
 */
int IncVar(char* name, PVARIABLE *lastVariable, PFUNCTEST_OUTPUT Out)
{
    PVARIABLE tV;
    tV=*lastVariable;
    if(tV==NULL)
    {
        tV=(PVARIABLE)malloc(sizeof(VARIABLE));
        strcpy(tV->name,name);
        tV->value.iVal=1;
        tV->type = INTEGER;
        tV->next=*lastVariable;
        *lastVariable=tV;
        OutputLog(LOG_INFO, Out,"\tINFO ==> New variable: %s = 1\n",name);
        return 0;
    }
    else
    {
        while(tV->next!=NULL&&_stricmp(tV->name,name))
        {
            tV=tV->next;
        }

        if(!_stricmp(tV->name,name))
        {
            if(tV->type == INTEGER)
                tV->value.iVal++;
            return 0;
        }
        else
        {
            tV=(PVARIABLE)malloc(sizeof(VARIABLE));
            strcpy(tV->name,name);
            tV->type = INTEGER;
            tV->value.iVal=1;
            tV->next=*lastVariable;
            *lastVariable=tV;
            OutputLog(LOG_INFO,Out,"\tINFO ==> New variable: %s = 1\n",name);
            return 0;
        }
    }
}

/*
 * Decrements a variable, when no variable with the given name exists a new one is created.
 * @return 0 Variable found
 * @return 1 Variable not found an new variable created and initialized
 */
int DecVar(char* name, PVARIABLE *lastVariable, PFUNCTEST_OUTPUT Out)
{
    PVARIABLE tV;
    tV=*lastVariable;
    if(tV==NULL)
    {
        tV=(PVARIABLE)malloc(sizeof(VARIABLE));
        strcpy(tV->name,name);
        tV->type = INTEGER;
        tV->value.iVal=1;
        tV->next=*lastVariable;
        *lastVariable=tV;
        OutputLog(LOG_INFO,Out,"\tINFO ==> New variable: %s = 1\n",name);
        return 0;
    }
    else
    {
        while(tV->next!=NULL&&_stricmp(tV->name,name))
        {
            tV=tV->next;
        }

        if(!_stricmp(tV->name,name))
        {
            if(tV->type == INTEGER)
                tV->value.iVal--;
            return 0;
        }
        else
        {
            tV=(PVARIABLE)malloc(sizeof(VARIABLE));
            strcpy(tV->name,name);
            tV->type = INTEGER;
            tV->value.iVal=1;
            tV->next=*lastVariable;
            *lastVariable=tV;
            OutputLog(LOG_INFO,Out,"\tINFO ==> New variable: %s = 1\n",name);
            return 0;
        }
    }
}

/*
 * Add a integer to a variable, when no variable with the given name exists a new one is created.
 * @param name pointer to first character of the variables name as char*
 * @param toadd number that will be added to the variable as integer
 * @param lastVariable pointer to the pointer to the last variable as PVARIABLE*
 * @param Out as PFUNCTEST_OUTPUT
 * @return 0 Variable found and toadd value was added
 * @return 1 Variable not found an new variable created and initialized with toadd value
 */
int AddToVar(char* name, int toadd, PVARIABLE *lastVariable, PFUNCTEST_OUTPUT Out)
{
    PVARIABLE tV;
    tV=*lastVariable;
    if(tV==NULL)
    {
        tV=(PVARIABLE)malloc(sizeof(VARIABLE));
        strcpy(tV->name,name);
        tV->type = INTEGER;
        tV->value.iVal=toadd;
        tV->next=*lastVariable;
        *lastVariable=tV;
        OutputLog(LOG_INFO,Out,"\tINFO ==> New variable: %s \n", name);
        return 0;
    }
    else
    {
        while(tV->next!=NULL&&_stricmp(tV->name,name))
        {
            tV=tV->next;
        }
        if(!_stricmp(tV->name,name))
        {
            if(tV->type == INTEGER)
                tV->value.iVal += toadd;
            return 0;
        }
        else
        {
            tV=(PVARIABLE)malloc(sizeof(VARIABLE));
            strcpy(tV->name,name);
            tV->type = INTEGER;
            tV->value.iVal=toadd;
            tV->next=*lastVariable;
            *lastVariable=tV;
            OutputLog(LOG_INFO,Out,"\tINFO ==> New variable: %s \n", name);
            return 0;
        }
    }
}

/* Function to Replace all Variables in a line
 * @param input InputLine as char*(PSZ)
 * @param value OutputLine as char*(PSZ)
 * @param *lastVariable Pointer to the latest created variable(PVARIABLE)
 * @param Out textbuffer for text output(PFUNCTEST_OUTPUT)
 * @return 0 Everything worked fine
 * @return 1 Variable not found.
 */
int ReplaceVariables(PSZ input,PSZ output, PVARIABLE *lastVariable, PFUNCTEST_OUTPUT Out)
{
    PVARIABLE tV;
    CHAR temp[102];
    PSZ temp2;
    PSZ temp3;
    
    temp3 = input;

    while(*input!=NULC){
        while(*input!='$'&&*input!=NULC)
        {
            *(output++)=*(input++);
        }
        if (*input!=NULC)
        {
            input++;
            temp2=temp;
            while(*input!='$'&&*input!=NULC)
            {
                *temp2++=*input++;
            }
            *temp2=NULC;
            input++;

            tV=*lastVariable;
            if(tV==NULL)
            {
                OutputLog(LOG_ERROR,Out, "\tERROR ==> variable '%s' not defined\n", temp);
                return 1;
            }
            while(tV->next!=NULL&&_stricmp(tV->name,temp))
            {
                tV=tV->next;
            }
            if(!_stricmp(tV->name,temp))
            {
                if(tV->type == INTEGER)
                    sprintf(output, "%d",tV->value.iVal);
                else
                    sprintf(output, "%s",tV->value.pszVal);

                output+=strlen(output);
            }
            else
            {
                OutputLog(LOG_ERROR, Out, "%s", temp3);
                OutputLog( LOG_ERROR,Out, "\tERROR ==> variable '%s' not defined, line not proccessed\n", temp);
                return 1;
            }
        }
    }
    return 0;
}

/* Function which handles the Wildcards
 * When there are Parameter which could include Wildcards
 * It makes a List starting with p_first_wc_path out of it
 * @param p_first_wc_path a pointer to the first list element (PWCPATH)
 * @param pCmd a Copy of the actuall Command (PCOMMAND)
 * @param Out a pointer to the Out-Element for writing Output (PFUNCTEST_OUTPUT)
 * @return 0 It was no Wildcard-Variable found in szWCFlags[...] -> wrong Call
 * @return 1 Everything went fine
 * @return 2 No Wildcard was found
 * @return 3 Wildcard list was too long
 */
int GetWildCardList(PWCPATH p_first_wc_path, PCOMMAND pCmd, PFUNCTEST_OUTPUT Out)
{
    int iParm = 0;
    
    while( iParm < pCmd->iNumOfParms )
    {
        switch(pCmd->szWCFlags[iParm])
        {
    
            case '1': 
                {
                    //Absolute Path-Paramater-List wich could include Wildcards
                    //Check if it has a Wildcard, if not only write the path in the List
                    if(IsWildcard(apTokens[iParm+1])==1)
                    {
                        //Pointer for parting it into Tokens
                        char *tmp_char_pointer;
                        char *act_word;
                        //temporary List for the new List of Paths 
                        char sList[WCPATH_LIST_MAX];
                        int WildcardReturnValue;
                        int iListCounter = 0;
                        PWCPATH tmp_wc_path;
                        
                        // saving start adresses in tmp_char_pointer and act_word
                        tmp_char_pointer = apTokens[iParm+1];
                        act_word = apTokens[iParm+1];
                        //Clear sList
                        sList[0] = NULC;
                        
                        //Go through the whole Line
                        while(*tmp_char_pointer != NULC)
                        {
                            //If there is a ',' or the Command ends at the next Letter, we can use the Token
                            if(*tmp_char_pointer == ',' || *(tmp_char_pointer+1) == NULC)
                            {
                                // if ',' set a NULC on this place
                                if( *(tmp_char_pointer) == ',' ) *tmp_char_pointer = NULC;
                                
                                //Get the Wildcards and write the Returnvalue in WildcardReturnValue
                                WildcardReturnValue = GetFilePaths(p_first_wc_path, act_word, Out);
                                
                                // 1 - everything went fine, if not, end the function and return the ERROR
                                if(WildcardReturnValue != 1) return WildcardReturnValue;
                                
                                // Go trough the path_list and write it Commaseperated in sList
                                tmp_wc_path = p_first_wc_path;
                                while(tmp_wc_path->next != NULL)
                                {
                                    strcat(sList, tmp_wc_path->path);
                                    strcat(sList, ",");
                                    iListCounter += strlen(tmp_wc_path->path)+1;
                                    
                                    tmp_wc_path = tmp_wc_path->next;
                                }
                                //Clear the Listagain and set act_word on the next word
                                ClearList(p_first_wc_path);
                                act_word = tmp_char_pointer+1;
                                SKIPSPACE(act_word);
                            }
                            SKIPSPACE(tmp_char_pointer);
                            tmp_char_pointer++;
                            
                            //Get an ERROR if the List gets too Long
                            if(iListCounter > WCPATH_LIST_MAX){
                                OutputLog(LOG_ERROR, Out, "\tERROR ==> %s can not be Processed, the Path with the replaced Wildcard is too long.\n", apTokens[iParm+1]);
                                return 3;
                            }
                        }/* End of while *tmp_char_pointer != NULC */
                        sList[strlen(sList)-1] = NULC;

                        AddToList(p_first_wc_path, sList);
                        return 1;
                    }else{
                        // No Wildcard so write only the one Path in the list and create a new element
                        AddToList(p_first_wc_path, apTokens[iParm+1]);
                        return 1;
                    }/* endif ISWILDCARD */
                }/* End of Case '1' */
                break;
            
            case '2':
                //Single, Absolute File-Paramater wich could include Wildcards
                //Check if it has a Wildcard, only write the path in the List
                if(IsWildcard(apTokens[iParm+1])==1)
                {
                    return GetMemoryNames(p_first_wc_path, apTokens[iParm+1], Out);
                }else{
                    // No Wildcard so write only the one Path in the list and create a new element
                    AddToList(p_first_wc_path, apTokens[iParm+1]);
                    return 1;
                }/* endif ISWILDCARD */
                break;
            case '3':
                //Relative List of File-Paramaters which could include Wildcards
                //Check if it has a Wildcard, only write the path in the List
                if(IsWildcard(apTokens[iParm+1])==1)
                {
                        //Pointer for parting it into Tokens
                        char *tmp_char_pointer;
                        char *act_word;
                        //temporary List for the new List of Paths 
                        char sList[WCPATH_LIST_MAX];
                        int WildcardReturnValue;
                        int iListCounter = 0;
                        PWCPATH tmp_wc_path;
                        
                        // saving start adresses in tmp_char_pointer and act_word
                        tmp_char_pointer = apTokens[iParm+1];
                        act_word = apTokens[iParm+1];
                        //Clear sList
                        sList[0] = NULC;
                        
                        while(*tmp_char_pointer != NULC)
                        {
                            if(*tmp_char_pointer == ',' || *(tmp_char_pointer+1) == NULC)
                            {
                                // if ',' set a NULC on this place
                                if( *(tmp_char_pointer) == ',' ) *tmp_char_pointer = NULC;

                                WildcardReturnValue = GetFileNames(p_first_wc_path, apTokens[1], act_word, Out);
                                if(WildcardReturnValue != 1) return WildcardReturnValue;
                                
                                // 1 - everything went fine, if not, end the function and return the ERROR
                                if(WildcardReturnValue != 1) return WildcardReturnValue;
                                
                                // Go trough the path_list and write it Commaseperated in sList
                                tmp_wc_path = p_first_wc_path;
                                while(tmp_wc_path->next != NULL)
                                {
                                    strcat(sList, tmp_wc_path->path);
                                    strcat(sList, ",");
                                    iListCounter += strlen(tmp_wc_path->path)+1;
                                    
                                    tmp_wc_path = tmp_wc_path->next;
                                }
                                //Clear the Listagain and set act_word on the next word
                                ClearList(p_first_wc_path);
                                act_word = tmp_char_pointer+1;
                                SKIPSPACE(act_word);
                            }

                            tmp_char_pointer++;
                            SKIPSPACE(tmp_char_pointer);
                            if(iListCounter > WCPATH_LIST_MAX){
                                OutputLog(LOG_ERROR, Out, "\tERROR ==> %s can not be Processed, the Path with the replaced Wildcard is too long.\n", apTokens[iParm+1]);
                                return 3;
                            }
                        }
                        sList[strlen(sList)-1] = NULC;

                        AddToList(p_first_wc_path, sList);
                        return 1;
                }else{
                    AddToList(p_first_wc_path, apTokens[iParm+1]);
                    return 1;
                }
                break;
                
            case '4': 
                //Single, relative Folder-Paramater wich could include Wildcards
                //Check if it has a Wildcard, only write the path in the List
                if(IsWildcard(apTokens[iParm+1])==1)
                {
                    return GetFolderNames(p_first_wc_path, apTokens[iParm+1], Out);
                }else{
                    AddToList(p_first_wc_path, apTokens[iParm+1]);
                    return 1;
                }
                break;
                
            case '5':
                //Single, relative Memory-Paramater wich could include Wildcards
                //Check if it has a Wildcard, only write the path in the List
                if(IsWildcard(apTokens[iParm+1])==1)
                {
                    return GetMemoryNames(p_first_wc_path, apTokens[iParm+1], Out);
                }else{
                    AddToList(p_first_wc_path, apTokens[iParm+1]);
                    return 1;
                }
                break;

            case '6':
            {
                //Relative List of Memory-Paramater which could include Wildcards
                //Check if it has a Wildcard, only write the path in the List
                if(IsWildcard(apTokens[iParm+1])==1)
                {
                        //Pointer for parting it into Tokens
                        char *tmp_char_pointer;
                        char *act_word;
                        //temporary List for the new List of Paths 
                        char sList[WCPATH_LIST_MAX];
                        int WildcardReturnValue;
                        int iListCounter = 0;
                        PWCPATH tmp_wc_path;

                        // saving start adresses in tmp_char_pointer and act_word
                        tmp_char_pointer = apTokens[iParm+1];
                        act_word = apTokens[iParm+1];
                        //Clear sList
                        sList[0] = NULC;
                        
                        while(*tmp_char_pointer != NULC)
                        {
                            if(*tmp_char_pointer == ' ' || *(tmp_char_pointer+1) == NULC)
                            {
                                // if ',' set a NULC on this place
                                if( *(tmp_char_pointer) == ' ' ) *tmp_char_pointer = NULC;
                                
                                WildcardReturnValue = GetMemoryNames(p_first_wc_path->next, act_word, Out);
                                
                                // 1 - everything went fine, if not, end the function and return the ERROR
                                if(WildcardReturnValue != 1) return WildcardReturnValue;
                                
                                // Go trough the path_list and write it Commaseperated in sList
                                tmp_wc_path = p_first_wc_path->next;
                                while(tmp_wc_path->next != NULL)
                                {
                                    strcat(sList, tmp_wc_path->path);
                                    strcat(sList, " ");
                                    iListCounter += strlen(tmp_wc_path->path)+1;
                                    
                                    tmp_wc_path = tmp_wc_path->next;
                                }
                                
                                //Clear the Listagain and set act_word on the next word
                                ClearList(p_first_wc_path->next);
                                act_word = tmp_char_pointer+1;
                                SKIPSPACE(act_word);
                            }
                            tmp_char_pointer++;
                            if(iListCounter > WCPATH_LIST_MAX){
                                OutputLog(LOG_ERROR, Out, "\tERROR ==> %s can not be Processed, the Path with the replaced Wildcard is too long.\n", apTokens[iParm+1]);
                                return 3;
                            }
                        }

                        AddToList(p_first_wc_path, sList);
                        return 1;
                }else{
                    AddToList(p_first_wc_path, apTokens[iParm+1]);
                    return 1;
                }
            }
                break;
                
            case '7':
                //Relative List of Dictionary-Paramater which could include Wildcards
                //Check if it has a Wildcard, only write the path in the List
                if(IsWildcard(apTokens[iParm+1])==1)
                {
                        //Pointer for parting it into Tokens
                        char *tmp_char_pointer;
                        char *act_word;
                        //temporary List for the new List of Paths 
                        char sList[WCPATH_LIST_MAX];
                        int WildcardReturnValue;
                        int iListCounter = 0;
                        PWCPATH tmp_wc_path;
                        
                        // saving start adresses in tmp_char_pointer and act_word
                        tmp_char_pointer = apTokens[iParm+1];
                        act_word = apTokens[iParm+1];
                        //Clear sList
                        sList[0] = NULC;
                 
                        while(*tmp_char_pointer != NULC)
                        {
                            if(*tmp_char_pointer == ' ' || *(tmp_char_pointer+1) == NULC)
                            {
                                // if ',' set a NULC on this place
                                if( *(tmp_char_pointer) == ' ' ) *tmp_char_pointer = NULC;
                                
                                WildcardReturnValue = GetDictionaryNames(p_first_wc_path, act_word, Out);
                                
                                // 1 - everything went fine, if not, end the function and return the ERROR
                                if(WildcardReturnValue != 1) return WildcardReturnValue;
                                
                                // Go trough the path_list and write it Commaseperated in sList
                                tmp_wc_path = p_first_wc_path;
                                while(tmp_wc_path->next != NULL)
                                {
                                    strcat(sList, tmp_wc_path->path);
                                    strcat(sList, ",");
                                    iListCounter += strlen(tmp_wc_path->path)+1;
                                    
                                    tmp_wc_path = tmp_wc_path->next;
                                }
                                //Clear the Listagain and set act_word on the next word
                                ClearList(p_first_wc_path);
                                act_word = tmp_char_pointer+1;
                                SKIPSPACE(act_word);
                            }
                            tmp_char_pointer++;
                            if(iListCounter > WCPATH_LIST_MAX){
                                OutputLog(LOG_ERROR, Out, "\tERROR ==> %s can not be Processed, the Path with the replaced Wildcard is too long.\n", apTokens[iParm+1]);
                                return 3;
                            }
                        }
                        
                        AddToList(p_first_wc_path, sList);
                }else{
                    AddToList(p_first_wc_path, apTokens[iParm+1]);
                }
                break;
                
            case '8':
                //Relative List of Dictionary-Paramater which could include Wildcards
                //Check if it has a Wildcard, only write the path in the List
                if(IsWildcard(apTokens[iParm+1])==1)
                {
                        //Pointer for parting it into Tokens
                        char *tmp_char_pointer;
                        char *act_word;
                        //temporary List for the new List of Paths 
                        char sList[WCPATH_LIST_MAX];
                        int WildcardReturnValue;
                        int iListCounter = 0;
                        PWCPATH tmp_wc_path;
                            
                        // saving start adresses in tmp_char_pointer and act_word
                        tmp_char_pointer = apTokens[iParm+1];
                        act_word = apTokens[iParm+1];
                        //Clear sList
                        sList[0] = NULC;
                 
                        while(*tmp_char_pointer != NULC)
                        {
                            if(*tmp_char_pointer == ' ' || *(tmp_char_pointer+1) == NULC)
                            {
                                // if ',' set a NULC on this place
                                if( *(tmp_char_pointer) == ' ' ) *tmp_char_pointer = NULC;
                                
                                WildcardReturnValue = GetDictionaryNames(p_first_wc_path, act_word, Out);
                                
                                // 1 - everything went fine, if not, end the function and return the ERROR
                                if(WildcardReturnValue != 1) return WildcardReturnValue;
                                
                                // Go trough the path_list and write it Commaseperated in sList
                                tmp_wc_path = p_first_wc_path;
                                while(tmp_wc_path->next != NULL)
                                {
                                    strcat(sList, tmp_wc_path->path);
                                    strcat(sList, ",");
                                    iListCounter += strlen(tmp_wc_path->path)+1;
                                    
                                    tmp_wc_path = tmp_wc_path->next;
                                }
                                //Clear the Listagain and set act_word on the next word
                                ClearList(p_first_wc_path);
                                act_word = tmp_char_pointer+1;
                                SKIPSPACE(act_word);
                            }
                            tmp_char_pointer++;
                            if(iListCounter > WCPATH_LIST_MAX){
                                OutputLog(LOG_ERROR,Out, "\tERROR ==> %s can not be Processed, the Path with the replaced Wildcard is too long.\n", apTokens[iParm+1]);
                                return 3;
                            }
                        }
                        
                        AddToList(p_first_wc_path, sList);
                        return 1;
                }else{
                    AddToList(p_first_wc_path, apTokens[iParm+1]);
                    return 1;
                }
                break;
            default: 
                break;
        } /* endswitch */
        iParm++;
    } /* endwhile */
    return 0;
}

/* Function which handles the Wildcards for absolute File Paths
 * It makes a List starting with p_first_wc_path out of it
 * @param p_first_wc_path a pointer to the first list element (PWCPATH)
 * @param FileName pointer on the actual File Path/Pattern (e.g. C:\\*)
 * @param Out a pointer to the Out-Element for writing Output (PFUNCTEST_OUTPUT)
 * @return 1 Everything went fine
 * @return 2 No Wildcard was found
 */
int GetFilePaths(PWCPATH p_first_wc_path, char *FileName, PFUNCTEST_OUTPUT Out)
{
    WIN32_FIND_DATA wfd;
    HANDLE handle;
    char help[128];
    char sPath[128];
    short sPos;
    
    strcpy(help, FileName);
    strcpy(sPath, FileName);

    //Escaping the Backslashes (\ -> \\) and get Position from last Backslash
    sPos = EscapeBackslash(help);

    handle = FindFirstFile(help, &wfd);
    
    if((int)handle != -1)
    {
        
        if( strcmp(wfd.cFileName, ".") == 0) {
            FindNextFile(handle , &wfd);
        }
        if( strcmp(wfd.cFileName, "..") == 0) { 
            FindNextFile(handle, &wfd);
        }
        //Delete String after Last Backslash(we need the Path)
        sPath[sPos+1] = NULC;
        
        do{
            // write Path + File Name in help, if a File was found, else Write NULC in help
            if(wfd.cFileName != NULC)
            {
                strcpy(help, sPath);
                strcat(help, wfd.cFileName);
            }else{
                help[0] = NULC;
                OutputLog(LOG_WARNING, Out, "\tWARNING ==> No file was found for Wildcard in Line: \n%s\n", szLine);
            }

            AddToList(p_first_wc_path, help);
            OutputLog(LOG_INFO, Out, "\tINFO ==>File Wildcard was replaced with %s\n", help);
        
        }while(FindNextFile(handle , &wfd) != 0);
        
        FindClose(handle);
        
        return 1;
    }else{
        OutputLog(LOG_INFO,Out, "\tINFO ==> Nothing was found for Wildcard %s\n", help);
        FindClose(handle);
        return 2;
    }/* endif handle =! -1 */    
}

/* Function which handles the Wildcards for File Paths
 * It makes a List starting with p_first_wc_path out of it
 * @param p_first_wc_path a pointer to the first list element (PWCPATH)
 * @param FileName pointer on the actual File Name including Wildcard (e.g. Doc*.txt)
 * @param Out a pointer to the Out-Element for writing Output (PFUNCTEST_OUTPUT)
 * @return 1 Everything went fine
 * @return 2 No Wildcard was found
 */
int GetFileNames(PWCPATH p_first_wc_path, char *FolderName, char *FilePath, PFUNCTEST_OUTPUT Out){
    char sFolderPath[128];
    WIN32_FIND_DATA wfd;
    HANDLE FileHandle;
    PROPDOCUMENT new_prop;
      FILE* fp;
    char path[1024];
    int iContinue = 1;

    if(FolderName != NULL)
    {
        GetFolderPath(FolderName, sFolderPath, Out);
        strcat(sFolderPath, "\\PROPERTY\\");
        strcpy(path, sFolderPath);
        strcat(path, "*");
        
        FileHandle = FindFirstFile(path, &wfd);
        if((int)FileHandle != -1)
        {
            if( strcmp(wfd.cFileName, ".") == 0) {
                if(FindNextFile(FileHandle , &wfd) == (BOOL) NULL) iContinue = 0;
            }
            if( strcmp(wfd.cFileName, "..") == 0 && iContinue) { 
                if(FindNextFile(FileHandle , &wfd) == (BOOL) NULL) iContinue = 0;
            }
    
            if(iContinue == 1)
            {
                do{
                    if( strcmp(wfd.cFileName, "HISTLOG.DAT") == 0){
                        if(FindNextFile(FileHandle , &wfd) ==(BOOL) NULL) iContinue = 0;
                    }
                    if(iContinue == 1)
                    {
                        strcpy(path, sFolderPath);
                        strcat(path, wfd.cFileName);
                
                        fp=fopen(path, "rb");
                        fread(&new_prop, sizeof(new_prop), 1, fp);
                        fclose( fp );
                        
            
                        if(IsMatch(new_prop.szLongName, FilePath)){
                            //write it in the List
                            AddToList(p_first_wc_path, new_prop.szLongName);
                            OutputLog(LOG_INFO, Out, "\tINFO ==>File Wildcard was replaced with %s\n", new_prop.szLongName);
                        }
                    }
                }while(FindNextFile(FileHandle, &wfd) != (BOOL) NULL && iContinue);
            }/* endif iContinue ==! */
    
            FindClose(FileHandle);        
            
            if(p_first_wc_path->next != NULL) {
                return 1;
            }else{
                OutputLog(LOG_ERROR, Out, "\tERROR ==> No Matching File was found for %s\n", FilePath);
                return 2; 
            }
        }else{
            OutputLog(LOG_ERROR, Out, "\tERROR ==> Folder %s was not found.\n", FolderName);
            return 2;
        }
    }else{
         OutputLog(LOG_ERROR, Out, "\tERROR ==> Foldername as mandatory parameter is missing.\n");
        return 2;
    }
}/*end of funtion GetFileNames*/

/* Function which writes the real Path to a Folder from the Name
 * it writes it in targetString
 * @param FolderName (e.g. SHOWMEHTML)
 * @param targetString in which we write the target Pattern
 */
void GetFolderPath(char *FolderName, char *targetString, PFUNCTEST_OUTPUT Out)
{
    WIN32_FIND_DATA wfd;
    HANDLE FolderHandle=INVALID_HANDLE_VALUE ;
    PROPFOLDER new_prop;
    FILE* fp;
    char path[1024+1];
    path[1024]='\0';
    if(getGlobalOpenTM2Path()!=NULL)
    { 
        strncpy(path,getGlobalOpenTM2Path(),1024);
        strncat(path,"PROPERTY\\",1024-strlen(getGlobalOpenTM2Path()));
        strncat(path,"*.F*",1024-strlen(path));
        FolderHandle = FindFirstFile(path, &wfd);
    }
    
    if(FolderHandle != INVALID_HANDLE_VALUE)
    {
        do{
            char* pTemp = strrchr(path,'\\');
            if(pTemp != NULL) 
                *(++pTemp)='\0';

            strcat(path, wfd.cFileName);
            fp=fopen(path, "rb");
            fread(&new_prop, sizeof(new_prop), 1, fp);
            fclose( fp );
        }while((FindNextFile(FolderHandle, &wfd) !=(BOOL) NULL) && _stricmp(new_prop.szLongName, FolderName) );
        
        FindClose(FolderHandle);
        
        if( _stricmp(new_prop.szLongName, FolderName) == 0){
            
            strcpy(path, new_prop.PropHead.szPath);
            strcat(path, "\\");
            strcat(path, new_prop.PropHead.szName);
            strcpy(targetString, path);
        }else{
            *targetString = NULC;
        }
    }else{
        OutputLog(LOG_ERROR, Out, "\tERROR ==> Folder %s was not found\n", FolderName);
    }
}

/* Function which handles the Wildcards for absolute Folders
 * It makes a List starting with p_first_wc_path out of it
 * @param p_first_wc_path a pointer to the first list element (PWCPATH)
 * @param FoderName pointer on the actual Folder Pattern (e.g. SH*HTML)
 * @param Out a pointer to the Out-Element for writing Output (PFUNCTEST_OUTPUT)
 * @return 1 Everything went fine
 * @return 2 No Wildcard was found
 */
int GetFolderNames(PWCPATH p_first_wc_path, char *FolderName, PFUNCTEST_OUTPUT Out){
    WIN32_FIND_DATA wfd;
    HANDLE FolderHandle = INVALID_HANDLE_VALUE;
    PROPFOLDER new_prop;
    FILE* fp;
    char path[1024+1];
    path[1024]='\0';
    if(getGlobalOpenTM2Path()!=NULL)
    { 
        strncpy(path,getGlobalOpenTM2Path(),1024);
        strncat(path,"PROPERTY\\",1024-strlen(getGlobalOpenTM2Path()));
        strncat(path,"*.F*",1024-strlen(path));
        FolderHandle = FindFirstFile(path, &wfd);
    }

    if(FolderHandle != INVALID_HANDLE_VALUE)
    {
        do{
            char* pTemp = strrchr(path,'\\');
            if(pTemp != NULL) 
                *(++pTemp)='\0';

            strcat(path, wfd.cFileName);
            fp=fopen(path, "rb");
            fread(&new_prop, sizeof(new_prop), 1, fp);
            fclose( fp );
            
            if(IsMatch(new_prop.szLongName, FolderName)){
                //write it in the List
                AddToList(p_first_wc_path, new_prop.szLongName);
                OutputLog(LOG_INFO, Out, "\tINFO ==>Folder Wildcard was replaced with %s\n", new_prop.szLongName);
            }
        }while(FindNextFile(FolderHandle, &wfd) !=(BOOL) NULL);
        
        FindClose(FolderHandle);
        
        if(p_first_wc_path->next != NULL) {
            return 1;
        }else{
            OutputLog(LOG_ERROR, Out, "\tERROR ==> No matching Folder was found for %s\n", FolderName);
            return 2;
        }
    }else{
        OutputLog(LOG_ERROR, Out, "\tERROR ==> No Folder was found in your OTM-Installation at WC-Call: %s\n", FolderName);
        return 2;
    }
}

/* Function which handles the Wildcards for Memory Names
 * It makes a List starting with p_first_wc_path out of it
 * @param p_first_wc_path a pointer to the first list element (PWCPATH)
 * @param MemoryName pointer on the actual MemoryNamePattern(e.g. Test*Mem)
 * @param Out a pointer to the Out-Element for writing Output (PFUNCTEST_OUTPUT)
 * @return 1 Everything went fine
 * @return 2 No Wildcard was found
 */
int GetMemoryNames(PWCPATH p_first_wc_path, char *MemoryName, PFUNCTEST_OUTPUT Out){
    WIN32_FIND_DATA wfd;
    HANDLE FolderHandle=INVALID_HANDLE_VALUE;
    PROP_NTM new_prop;
    FILE* fp;
    char path[1024+1];
    path[1024]='\0';
     if(getGlobalOpenTM2Path()!=NULL)
    { 
        strncpy(path,getGlobalOpenTM2Path(),1024);
        strncat(path,"PROPERTY\\",1024-strlen(getGlobalOpenTM2Path()));
        strncat(path,"*.MEM",1024-strlen(path));
        FolderHandle = FindFirstFile(path, &wfd);
    }

    if(FolderHandle != INVALID_HANDLE_VALUE)
    {
        do{
            char* pTemp = strrchr(path,'\\');
            if(pTemp != NULL) 
                *(++pTemp)='\0';
            strcat(path, wfd.cFileName);

            fp=fopen(path, "rb");
            fread(&new_prop, sizeof(new_prop), 1, fp);
            fclose( fp );
            
            if(IsMatch(new_prop.szLongName, MemoryName)){
                //write it in the List
                AddToList(p_first_wc_path, new_prop.szLongName);
                OutputLog(LOG_INFO,Out, "\tINFO ==>Memory Wildcard was replaced with %s\n", new_prop.szLongName);
            }
        }while(FindNextFile(FolderHandle, &wfd) != (BOOL) NULL);
        
        FindClose(FolderHandle);
        
        if(p_first_wc_path->next != NULL) {
            return 1;
        }else{
            OutputLog(LOG_ERROR, Out, "\tERROR ==> No matching Memory was found for %s\n", MemoryName);
            return 2;
        }
    }else{
        OutputLog(LOG_ERROR,Out, "\tERROR ==> No Folder was found in your OTM-Installation at WC-Call: %s\n", MemoryName);
        return 2;
    }
}

/* Function which handles the Wildcards for absolute Dictionary Names
 * It makes a List starting with p_first_wc_path out of it
 * @param p_first_wc_path a pointer to the first list element (PWCPATH)
 * @param DictionaryName pointer to the actual DictionaryNamePattern (e.g. SH*HTMLDIC)
 * @param Out a pointer to the Out-Element for writing Output (PFUNCTEST_OUTPUT)
 * @return 1 Everything went fine
 * @return 2 No Wildcard was found
 */
int GetDictionaryNames(PWCPATH p_first_wc_path, char *DictionaryName, PFUNCTEST_OUTPUT Out){
    //getting List with the Dictionarynames of the the Projekt
    WIN32_FIND_DATA wfd;
    HANDLE FolderHandle=INVALID_HANDLE_VALUE;
    PROPDICTIONARY new_prop;
    FILE* fp;
    char path[1024+1];
    path[1024]='\0';
    if(getGlobalOpenTM2Path()!=NULL)
    { 
        strncpy(path,getGlobalOpenTM2Path(),1024);
        strncat(path,"PROPERTY\\",1024-strlen(getGlobalOpenTM2Path()));
        strncat(path,"*.PRO",1024-strlen(path));
        FolderHandle = FindFirstFile(path, &wfd);
    }

    if(FolderHandle != INVALID_HANDLE_VALUE)
    {
        do{
            char* pTemp = strrchr(path,'\\');
            if(pTemp != NULL) 
                *(++pTemp)='\0';
            strcat(path, wfd.cFileName);

            fp=fopen(path, "rb");
            fread(&new_prop, sizeof(new_prop), 1, fp);
            fclose( fp );
            

            if(IsMatch(new_prop.szLongName, DictionaryName)){
                //write it in the List
                AddToList(p_first_wc_path, new_prop.szLongName);
                OutputLog(LOG_INFO, Out, "\tINFO ==>Dictionary Wildcard was replaced with %s\n", new_prop.szLongName);
            }
        }while(FindNextFile(FolderHandle, &wfd) !=(BOOL)  NULL);
        
        FindClose(FolderHandle);
        
        if(p_first_wc_path->next != NULL) {
            return 1;
        }else{
            OutputLog(LOG_ERROR,Out, "\tERROR ==> No matching Dictionary was found for %s\n", DictionaryName);
            return 2;
        }
    }else{
        OutputLog(LOG_WARNING,Out, "\tWARNING ==>  No Matching Dictionary was found for %s\n", DictionaryName);
        return 2;
    }
}

/* Checks if the Pattern matches with the String
 * @param string -> pointer to the string (e.g. Test1.doc)
 * @param pattern -> pointer to the pattern with the Wildcard (e.g. Test*.doc)
 * @return 1 when string and pattern match
 * @return 0 when string and pattern don't match
 */
int IsMatch(char *string, char *pattern){
        // delete Spaces and Tabs on the end of the pattern
        {
            int iCount = strlen(pattern)-1;
            while(pattern[iCount] == ' ' || pattern[iCount] == 9)
                pattern[iCount] = NULC;
                iCount--;
        }
    
        while(*string){
            switch(*pattern){
                case '*':
                    do {
                        ++pattern;
                    } while(*pattern == '*');

                    if(!*pattern){
                        return(1);
                    }

                    while(*string != '\0'){
                        if(*pattern != *string) string++;
                        else break;

                    }
                    if(IsMatch(string++,pattern++)== 1)
                        return(1);

                    return(0);

                case '?':
                    if(IsMatch(++string, ++pattern) == 1)
                        return (1);

                    return(0);
                default :
                    while(*pattern != '\0'){
                        if(*pattern == '*'){
                            if(IsMatch(string, pattern) == 1)
                                return (1);
                        }
                        
                        if(*pattern == '?'){
                            if(IsMatch(string,pattern) == 1)
                                return (1);
                        }
                        
                        if(_strnicmp(string, pattern,1) != 0){
                            return(0);
                        }
                        pattern++;
                        string++;
                    }
                    if(*(pattern-1) == '*') return (1);
                    if(*string == NULC) return (1);
                    return 0;
            }// end switch
        }// end while
        while (*pattern == '*') ++pattern;
        return !*pattern;
}

/* Clears the whole List of WC_Paths
 * ->Set List begin on NULL/NULC again
 * the other Elements will be deleted(free memory)
 * @param  p_first_wc_path -> pointer to the first Element of the List (PWCPATH)
 */
void ClearList(PWCPATH p_first_wc_path){
    PWCPATH tmp_wc_path;
    PWCPATH next_wc_path;
    
    tmp_wc_path = p_first_wc_path;
    
    //Only do Something when there is a List
    if(tmp_wc_path->next != NULL){
        next_wc_path = tmp_wc_path->next;

        tmp_wc_path->path[0] = NULC;
        tmp_wc_path->next = NULL;
        
        tmp_wc_path = next_wc_path;
        next_wc_path = next_wc_path -> next;
        
        while(next_wc_path != NULL)
        {
            free( tmp_wc_path );
            tmp_wc_path = next_wc_path;
            next_wc_path = next_wc_path->next;
        }
        
        free( tmp_wc_path );
        
    }else{
        next_wc_path = NULL;
        tmp_wc_path -> path[0] = NULC;
    }
}

/* Adds a Element to the List of WC_PATHS
 * and mallocs a new Element
 * @param  p_first_wc_path -> pointer to the first Element of the List (PWCPATH)
 */
void AddToList(PWCPATH p_first_wc_path, char *sName){
    PWCPATH tmp_wc_path;
    
    // print the Command line in Path List
    tmp_wc_path = p_first_wc_path;
    
    //going to end of wc_path_list
    while(tmp_wc_path->next != NULL) 
    {
        tmp_wc_path = tmp_wc_path->next;
    }
    
    if(sName == NULL){
        strcpy(tmp_wc_path->path, "");
    }else{
        strcpy(tmp_wc_path->path , sName);        
    }
    
    //writing name in last, empty element and create a new element
    tmp_wc_path->next = (PWCPATH)malloc(sizeof(WCPATH));
    tmp_wc_path = tmp_wc_path->next;
    tmp_wc_path->next = NULL;
}

USHORT deleteFile(const char* pFilePath)
{
    if(pFilePath == NULL)
        return 0;

     BOOL bRes =  DeleteFile(TEXT(pFilePath));
     if(!bRes)
     {
         return( (USHORT)GetLastError() );
     }
    return 0;
}

USHORT copyFile(const char* pSourceFilePath, const char* pTargetFilePath, LONG lOption  )
{
    if( (pSourceFilePath == NULL) || (pTargetFilePath == NULL) )
        return 0;

    BOOL bRes = CopyFile( pSourceFilePath, pTargetFilePath, (lOption & OVERWRITE_OPT) == 0 );
     if(!bRes)
     {
         return( (USHORT)GetLastError() );
     }
    return 0;
}

char* getOpenTM2InstallPath(char* pResOut, int length)
{
    if(pResOut == NULL || length<=0)
        return NULL;

    memset(pResOut, 0x00, length*sizeof(pResOut[0]));

    HKEY hKey = NULL;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        HKEY hSubKey = NULL;
        if (RegOpenKeyExA(hKey, "OpenTM2" , 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
        {
            DWORD dwType = REG_SZ;
            DWORD iSize = length*sizeof(pResOut[0]);
            int iSuccess = RegQueryValueExA(hSubKey, "", 0, &dwType, (LPBYTE)pResOut, &iSize);
            if (iSuccess == ERROR_SUCCESS)
            {
                dwType = REG_SZ;
                iSize = length*sizeof(pResOut[0]);
                strcat(pResOut, "\\");
            }
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

    if (pResOut[0] == 0)
    {
        return NULL;
    }
    
    return pResOut;
}

const char* getGlobalOpenTM2Path()
{
    if(gOpenTM2Path[0]==0)
    {
        return getOpenTM2InstallPath(gOpenTM2Path, 512);
    }
    return gOpenTM2Path;
}

char* trim(const char* pSrc,const char tCh, char* pRes, int len)
{
    if(pSrc==NULL || pRes==NULL)
        return NULL;

    const char* pFront = pSrc;
    while(*pFront!='\0' && *pFront==tCh)
        ++pFront;

    if(*pFront =='\0')
        return NULL;

    const char* pBack = pSrc+strlen(pSrc)-1;
    while(pBack != pFront && *pBack==tCh)
        --pBack;

    while(pFront<= pBack && len>0)
    {    
        *pRes++=*pFront++;
        --len;
    }

    *pRes = '\0';

    return pRes;
}


int executeCommand(const char* pszCmd)
{
    DWORD nRC = NO_ERROR;

    STARTUPINFO si;
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.hStdOutput = NULL;
    si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi;

    if (CreateProcess(NULL, (LPSTR)pszCmd, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
    {
        WaitForSingleObject(pi.hProcess, INFINITE);

        if (!GetExitCodeProcess(pi.hProcess, &nRC))
        {
            nRC = GetLastError();
        }
    }
    else
    {
        nRC = GetLastError();
    }
    
    return nRC;
}
int expressionValue(char** expIn)
{
	char* exp = *expIn;
	exp = skipSpace(exp);
	int val = termValue(&exp);
	exp = skipSpace(exp);
    
	while( strlen(exp)>0 && (*exp=='&'||*exp=='|') )
	{
		char op = *exp++;
        if(strlen(exp)==0)
			break;

        int nextVal = termValue(&exp);
        if (op == '&')
            val = val&&nextVal;
        else
            val = val||nextVal;
        exp = skipSpace(exp);
	}
	*expIn = exp;
	return val;
}

int termValue(char** expIn) 
{
	int bRes = -1;
	char* exp = *expIn;
	char* nextVal = NULL;
    exp = skipSpace(exp);
    char* val = factorValue(&exp);
    exp = skipSpace(exp);
    
	bRes = atoi(val);

    while ( strlen(exp)>0 &&
		    ( *exp=='>' ||  *exp=='<'|| *exp=='=' || *exp=='!') ) 
	{
        	
        char op = *exp++;
            
        if (op == '>')
		{
            nextVal = factorValue(&exp);
			*expIn = exp;
			if( !isDigits(val) &&
				!isDigits(nextVal))
			{
				bRes = (cmpIgnoreQuotes(val,nextVal)>0);
			}
			else
			{
				bRes = (atoi(val)>atoi(nextVal));
			}
        }
        else if(op=='<')
		{
            nextVal = factorValue(&exp);
			*expIn = exp;
			if( !isDigits(val) &&
				!isDigits(nextVal))
			{
				bRes = (cmpIgnoreQuotes(val,nextVal)<0);
			}
			else
			{
				bRes = (atoi(val)<atoi(nextVal));
			}
        }
        else if(op=='=')
		{
			exp++;
            nextVal = factorValue(&exp);
			*expIn = exp;
			if( !isDigits(val) &&
				!isDigits(nextVal))
			{
				bRes = (cmpIgnoreQuotes(val,nextVal)==0);
			}
			else
			{
				bRes = (atoi(val)==atoi(nextVal));
			}
        }
        else if(op=='!')
		{
			exp++;
            nextVal = factorValue(&exp);
			*expIn = exp;
			if( !isDigits(val) &&
				!isDigits(nextVal))
			{
				bRes = (cmpIgnoreQuotes(val,nextVal)!=0);
			}
			else
			{
			    bRes = (atoi(val)!=atoi(nextVal));
			}
        }
        
		if(bRes != -1)
		{
			if(val != NULL)
			{
				free(val);
				val = NULL;
			}
			if(nextVal != NULL)
			{
				free(nextVal);
				nextVal = NULL;
			}
			return bRes;
		}

		exp = skipSpace(exp);
    }
	*expIn = exp;
	if(val != NULL)
	    free(val);
	return bRes;
}


char* factorValue(char** expIn) 
{
	char* exp = *expIn;
    exp = skipSpace(exp);
    char ch = *exp++;
    
    if( isdigit(ch) || isalpha(ch) || ch=='"' ) 
	{
		int bracecnt = 0;
		int idx = 0;
		PSZ pszValue = (PSZ)malloc( strlen(exp)+2);
		if(pszValue == NULL)
		{
			*expIn = exp;
			return NULL;
		}

		pszValue[idx++] = ch;

        char tch = *exp;
        while(  strlen(exp)>0 &&
        		tch!='=' &&
        		tch!='!'&&
        		tch!='<'&&
        		tch!='>'&&
        		tch!='&'&&
        		tch!='|' &&
        		tch!=' ') 
		{
        	if(tch=='(')
        		bracecnt++;
        	else if(tch==')')
			{
        		bracecnt--;
        		if(bracecnt<0)
				{
        		    exp++;
        			break;
        		}
        	}
			pszValue[idx++] = tch;
        	exp++;
        	if(strlen(exp) == 0)
        		break;
        	tch = *exp;
        }
        
		*expIn = exp;
        pszValue[idx] = '\0';
		return pszValue;
    } 
    else if(ch=='(') 
	{
        int val = expressionValue(&exp);
        exp = skipSpace(exp);
        *expIn = exp;

        if (strlen(exp)>0 && *exp!=')' )
            return NULL;
        else if( strlen(exp)>0 && *exp==')')
		{
            exp++;
			*expIn = exp;
		}

		PSZ pValue = (PSZ)malloc(30);
		if(pValue == NULL)
		{
			return NULL;
		}
		return itoa(val,pValue,10);
    }
	return NULL;
}

char* skipSpace(char* exp)
{
	while(*exp!='\0' && *exp==' ')
		exp++;

	return exp;
}

int isDigits(const char* pIn)
{
	int res = 1;
	const char* pTemp = pIn;
	// skip begin space and symbol
	while( (*pTemp==' ' || 
		    *pTemp=='+' ||
			*pTemp=='-') && *pTemp!='\0')
		pTemp++;
	if(*pTemp=='\0'|| *pTemp=='.')
		res = 0;

	// process normal
	int dotcnts = 0;
	while(*pTemp!='\0' && res!=0)
	{
		if( *pTemp=='.' || (*pTemp>='0'&&*pTemp<='9') )
		{
			// only one dot allowed for double or float numbers
			if(*pTemp=='.')
			{
				dotcnts++;
				if(dotcnts>1)
                    res = 0;
			}

			pTemp++;
		}
		else
		{
			res = 0;
		}
	}
	return res;
}

int cmpIgnoreQuotes(char* pOrg,char* pTgt)
{
	if(pOrg==NULL || pTgt==NULL)
		return 0;
    
	if(*pTgt=='"' && *pOrg!='"')
	{
	    PSZ pszValue = (PSZ)malloc( strlen(pTgt)+1);
		if(pszValue != NULL)
		{
			pTgt++;
			strncpy(pszValue,pTgt,strlen(pTgt)-1);
			*(pszValue+strlen(pTgt)-1) = '\0';
			int bres = strcmp(pOrg,pszValue);
			free(pszValue);
			return bres;
		}
	}
	else
	{
		return strcmp(pOrg,pTgt);
	}
  return 0;
}



